#-------------------------------------------------------------------------------
#   z80_decoder.py
#   Generate huge switch/case Z80 instruction decoder.
#   See: 
#       http://www.z80.info/decoding.htm
#       http://www.righto.com/2014/10/how-z80s-registers-are-implemented-down.html
#       http://www.z80.info/zip/z80-documented.pdf
#       https://www.omnimaga.org/asm-language/bit-n-(hl)-flags/5/?wap2
#-------------------------------------------------------------------------------
import sys

# tab-width for generated code
TabWidth = 2

# the output path
OutPath = '../chips/_z80_decoder.h'

# the target file handle
Out = None

# 8-bit register table, the 'HL' entry is for instructions that use
# (HL), (IX+d) and (IY+d)
r = [ 'B', 'C', 'D', 'E', 'H', 'L', 'HL', 'A' ]

# 16-bit register table, with SP
rp = [ 'BC', 'DE', 'HL', 'SP' ]

# 16-bit register table, with AF (only used for PUSH/POP)
rp2 = [ 'BC', 'DE', 'HL', 'FA' ]

# condition-code table (for conditional jumps etc)
cond = [
    '!(_G_F()&Z80_ZF)',  # NZ
    '(_G_F()&Z80_ZF)',   # Z
    '!(_G_F()&Z80_CF)',   # NC
    '(_G_F()&Z80_CF)',    # C
    '!(_G_F()&Z80_PF)',   # PO
    '(_G_F()&Z80_PF)',    # PE
    '!(_G_F()&Z80_SF)',   # P
    '(_G_F()&Z80_SF)'     # M
]

# the same as 'human readable' flags for comments
cond_cmt = [ 'NZ', 'Z', 'NC', 'C', 'PO', 'PE', 'P', 'M' ]

# 8-bit ALU instructions command names
alu_cmt = [ 'ADD', 'ADC', 'SUB', 'SBC', 'AND', 'XOR', 'OR', 'CP' ]

# rot and shift instruction command names
rot_cmt = [ 'RLC', 'RRC', 'RL', 'RR', 'SLA', 'SRA', 'SLL', 'SRL' ]

# an 'opcode' wraps the instruction byte, human-readable asm mnemonics,
# and the source code which implements the instruction
class opcode :
    def __init__(self, op) :
        self.byte = op
        self.cmt = None
        self.src = None

indent = 0

#-------------------------------------------------------------------------------
#   output a src line
#
def l(s) :
    Out.write(tab()+s+'\n')

#-------------------------------------------------------------------------------
# instruction fetch machine cycle (M1):
#              T1   T2   T3   T4
#    --------+----+----+----+----+
#    CLK     |--**|--**|--**|--**|
#    A15-A0  |   PC    | REFRESH |
#    MREQ    |   *|****|  **|**  |
#    RD      |   *|****|    |    |
#    WAIT    |    | -- |    |    |
#    M1      |****|****|    |    |
#    D7-D0   |    |   X|    |    |
#    RFSH    |    |    |****|****|
#
# memory-read machine cycle:
#              T1   T2   T3
#    --------+----+----+----+
#    CLK     |--**|--**|--**|
#    A15-A0  |   MEM ADDR   |
#    MREQ    |   *|****|**  |
#    RD      |   *|****|**  |
#    WR      |    |    |    |
#    D7-D0   |    |    | X  |
#    WAIT    |    | -- |    |
#
# memory-write machine cycle:
#              T1   T2   T3
#    --------+----+----+----+
#    CLK     |--**|--**|--**|
#    A15-A0  |   MEM ADDR   |
#    MREQ    |   *|****|**  |
#    RD      |    |    |    |
#    WR      |    |  **|**  |
#    D7-D0   |   X|XXXX|XXXX|
#    WAIT    |    | -- |    |
#
# input machine cycle:
#              T1   T2   TW   T3
#    --------+----+----+----+----+
#    CLK     |--**|--**|--**|--**|
#    A15-A0  |     PORT ADDR     |
#    IORQ    |    |****|****|**  |
#    RD      |    |****|****|**  |
#    WR      |    |    |    |    |
#    D7-D0   |    |    |    | X  |
#    WAIT    |    |    | -- |    |
#
#   NOTE: the implementation moves the activation of the IORQ|RD
#   pins from T2 to TW, so that the pins will only be active 
#   for one tick (assuming no wait states)
#
# output machine cycle:
#              T1   T2   TW   T3
#    --------+----+----+----+----+
#    CLK     |--**|--**|--**|--**|
#    A15-A0  |     PORT ADDR     |
#    IORQ    |    |****|****|**  |
#    RD      |    |    |    |    |
#    WR      |    |****|****|**  |
#    D7-D0   |  XX|XXXX|XXXX|XXXX|
#    WAIT    |    |    | -- |    |
#
#    NOTE: the IORQ|WR pins will already be switched off at the beginning
#    of TW, so that IO devices don't need to do double work.
#

#-------------------------------------------------------------------------------
# write source header
#
def write_header() :
    l('uint32_t z80_exec(z80_t* cpu, uint32_t num_ticks) {')
    l('  uint64_t r0 = cpu->bc_de_hl_fa;')
    l('  uint64_t r1 = cpu->wz_ix_iy_sp;')
    l('  uint64_t r2 = cpu->im_ir_pc_bits;')
    l('  uint64_t r3 = cpu->bc_de_hl_fa_;')
    l('  uint64_t ws = _z80_map_regs(r0, r1, r2);')
    l('  uint64_t map_bits = r2 & _BITS_MAP_REGS;')
    l('  uint64_t pins = cpu->pins;')
    l('  const uint64_t trap_addr = cpu->trap_addr;')
    l('  const z80_tick_t tick = cpu->tick;')
    l('  void* ud = cpu->user_data;')
    l('  int trap_id = -1;')
    l('  uint32_t ticks = 0;')
    l('  uint8_t op, d8;')
    l('  uint16_t addr, d16;')
    l('  uint16_t pc = _G_PC();')
    l('  do {')
    l('    _OFF(Z80_INT);')
    l('    /* delay-enable interrupt flags */')
    l('    if (r2 & _BIT_EI) {')
    l('      r2 &= ~_BIT_EI;')
    l('      r2 |= (_BIT_IFF1 | _BIT_IFF2);')
    l('    }')
    l('    _FETCH(op)')
    l('    if (op == 0xED) {')
    l('      map_bits &= ~(_BIT_USE_IX|_BIT_USE_IY);')
    l('    }')
    l('    if (map_bits != (r2 & _BITS_MAP_REGS)) {')
    l('      const uint64_t old_map_bits = r2 & _BITS_MAP_REGS;')
    l('      r0 = _z80_flush_r0(ws, r0, old_map_bits);')
    l('      r1 = _z80_flush_r1(ws, r1, old_map_bits);')
    l('      r2 = (r2 & ~_BITS_MAP_REGS) | map_bits;')
    l('      ws = _z80_map_regs(r0, r1, r2);')
    l('    }')
    l('    switch (op) {')

#-------------------------------------------------------------------------------
# write the end of the main switch-case
#
def write_op_post():
    l('      default: break;')
    l('    }')

#-------------------------------------------------------------------------------
# write source footer
#
def write_footer() :
    l('    map_bits &= ~(_BIT_USE_IX|_BIT_USE_IY);')
    l('    if (trap_addr != 0xFFFFFFFFFFFFFFFF) {')
    l('      uint64_t ta = trap_addr;')
    l('      for (int i = 0; i < Z80_MAX_NUM_TRAPS; i++) {')
    l('        ta >>= 16;')
    l('        if (((ta & 0xFFFF) == pc) && (pc != 0xFFFF)) {')
    l('          trap_id = i;')
    l('          break;')
    l('        }')
    l('      }')
    l('    }')
    l('  } while ((ticks < num_ticks) && (trap_id < 0));')
    l('  _S_PC(pc);')
    l('  r0 = _z80_flush_r0(ws, r0, r2);')
    l('  r1 = _z80_flush_r1(ws, r1, r2);')
    l('  r2 = (r2 & ~_BITS_MAP_REGS) | map_bits;')
    l('  cpu->bc_de_hl_fa = r0;')
    l('  cpu->wz_ix_iy_sp = r1;')
    l('  cpu->im_ir_pc_bits = r2;')
    l('  cpu->bc_de_hl_fa_ = r3;')
    l('  cpu->pins = pins;')
    l('  cpu->trap_id = trap_id;')
    l('  return ticks;')
    l('}')

#-------------------------------------------------------------------------------
#   Generate code for checking and handling an interrupt request at
#   the end of an instruction:
#
#   An interrupt response cycle looks a lot like an opcode fetch machine cycle.
#
#   NOTE: The INT pint is normally checked at the start of the last time-cycle
#   of an instruction. The emulator moves this check to the start
#   of the next time cycle (so it will check right *after* the last instruction
#   time-cycle.
#
#              T1   T2   TW   TW   T3   T4
#    --------+----+----+----+----+----+----+
#    CLK     |--**|--**|--**|--**|--**|--**|
#    INT    *|*   |    |    |    |    |    |
#    A15-A0  |        PC         |  RFRSH  |
#    M1      |****|****|****|****|    |    |
#    MREG    |    |    |    |    |  **|**  |
#    IORQ    |    |    |  **|****|    |    |
#    RD      |    |    |    |    |    |    |
#    WR      |    |    |    |    |    |    |
#    D7-D0   |    |    |    |  XX|XX..|    |
#    WAIT    |    |    |    | -- |    |    |
#    RFSH    |    |    |    |    |****|****|
#
#   Interrupt controllers must check for the M1|IORQ pin combination 
#   being set (with neither the RD nor WR being set, and the highest 
#   priority interrupt controller must place the interrupt vector on the
#   data bus.
#
#   INT MODE 1: 13 cycles
#   INT MODE 2: 19 cycles
#
def write_interrupt_handling():
    l('    if (((pins & (Z80_INT|Z80_BUSREQ))==Z80_INT) && (r2 & _BIT_IFF1)) {')
    l('      r2 &= ~(_BIT_IFF1|_BIT_IFF2);')
    l('      if (pins & Z80_HALT) {')
    l('        pins &= ~Z80_HALT;')
    l('        pc++;')
    l('      }')
    l('      _ON(Z80_M1|Z80_IORQ);')
    l('      _SA(pc);')
    l('      _TW(4);')
    l('      const uint8_t int_vec = _GD();')
    l('      _OFF(Z80_M1|Z80_IORQ);')
    l('      _BUMPR();')
    l('      _T(2);')
    l('      switch (_G_IM()) {')
    l('        case 0:')
    l('          break;')
    l('        case 1:')
    l('          {')
    l('            uint16_t sp = _G_SP();')
    l('            _MW(--sp,pc>>8);')
    l('            _MW(--sp,pc);')
    l('            _S_SP(sp);')
    l('            pc = 0x0038;')
    l('            _S_WZ(pc);')
    l('          }')
    l('          break;')
    l('        case 2:')
    l('          {')
    l('            uint16_t sp = _G_SP();')
    l('            _MW(--sp,pc>>8);')
    l('            _MW(--sp,pc);')
    l('            _S_SP(sp);')
    l('            addr = (_G_I()<<8) | (int_vec & 0xFE);')
    l('            uint8_t z,w;')
    l('            _MR(addr++,z);')
    l('            _MR(addr,w);')
    l('            pc = (w<<8)|z;')
    l('            _S_WZ(pc);')
    l('          }')
    l('          break;')
    l('      }')
    l('    }')

#-------------------------------------------------------------------------------
# Write the ED extended instruction block.
#
def write_ed_ops():
    l('case 0xED: {')
    inc_indent()
    l('_FETCH(op);')
    l('switch(op) {')
    inc_indent()
    for i in range(0, 256):
        write_op(enc_ed_op(i))
    l('default: break;');
    dec_indent()
    l('}')
    dec_indent()
    l('}')
    l('break;')

#-------------------------------------------------------------------------------
# Write the CB extended instruction block as 'hand-decoded' ops
#
def write_cb_ops():
    l('case 0xCB: {')
    inc_indent()
    l('/* special handling for undocumented DD/FD+CB double prefix instructions,')
    l(' these always load the value from memory (IX+d),')
    l(' and write the value back, even for normal')
    l(' "register" instructions')
    l(' see: http://www.baltazarstudios.com/files/ddcb.html')
    l('*/')
    l('/* load the d offset for indexed instructions */')
    l('int8_t d;')
    l('if (_IDX()) { _IMM8(d); } else { d=0; }')
    l('/* fetch opcode without memory refresh and incrementint R */')
    l('_FETCH_CB(op);')
    l('const uint8_t x = op>>6;')
    l('const uint8_t y = (op>>3)&7;')
    l('const uint8_t z = op&7;')
    l('const int rz = (7-z)<<3;')
    l('/* load the operand (for indexed ops, always from memory!) */')
    l('if ((z == 6) || _IDX()) {')
    l('  _T(1);')
    l('  addr = _G_HL();')
    l('  if (_IDX()) {')
    l('    _T(1);')
    l('    addr += d;')
    l('    _S_WZ(addr);')
    l('  }')
    l('  _MR(addr,d8);')
    l('}')
    l('else {')
    l('  /* simple non-indexed, non-(HL): load register value */')
    l('  d8 = _G8(ws,rz);')
    l('}')
    l('uint8_t f = _G_F();')
    l('uint8_t r;')
    l('switch (x) {')
    l('  case 0:')
    l('     /* rot/shift */')
    l('     switch (y) {')
    l('       case 0: /*RLC*/ r=d8<<1|d8>>7; f=_z80_szp[r]|(d8>>7&Z80_CF); break;')
    l('       case 1: /*RRC*/ r=d8>>1|d8<<7; f=_z80_szp[r]|(d8&Z80_CF); break;')
    l('       case 2: /*RL */ r=d8<<1|(f&Z80_CF); f=_z80_szp[r]|(d8>>7&Z80_CF); break;')
    l('       case 3: /*RR */ r=d8>>1|((f&Z80_CF)<<7); f=_z80_szp[r]|(d8&Z80_CF); break;')
    l('       case 4: /*SLA*/ r=d8<<1; f=_z80_szp[r]|(d8>>7&Z80_CF); break;')
    l('       case 5: /*SRA*/ r=d8>>1|(d8&0x80); f=_z80_szp[r]|(d8&Z80_CF); break;')
    l('       case 6: /*SLL*/ r=d8<<1|1; f=_z80_szp[r]|(d8>>7&Z80_CF); break;')
    l('       case 7: /*SRL*/ r=d8>>1; f=_z80_szp[r]|(d8&Z80_CF); break;')
    l('     }')
    l('     break;')
    l('  case 1:')
    l('    /* BIT (bit test) */')
    l('    r = d8 & (1<<y);')
    l('    f = (f&Z80_CF) | Z80_HF | (r?(r&Z80_SF):(Z80_ZF|Z80_PF));')
    l('    if (z == 6) {')
    l('      f |= (_G_WZ()>>8) & (Z80_YF|Z80_XF);')
    l('    }')
    l('    else {')
    l('      f |= d8 & (Z80_YF|Z80_XF);')
    l('    }')
    l('    break;')
    l('  case 2:')
    l('    /* RES (bit clear) */')
    l('    r = d8 & ~(1<<y);')
    l('    break;')
    l('  case 3:')
    l('    /* SET (bit set) */')
    l('    r = d8 | (1<<y);')
    l('    break;')
    l('}')
    l('if (x != 1) {')
    l('  /* write result back */')
    l('  if ((z == 6) || (r2 & (_BIT_USE_IX|_BIT_USE_IY))) {')
    l('    /* (HL), (IX+d), (IY+d): write back to memory, for extended ops,')
    l('       even when the op is actually a register op')
    l('    */')
    l('    _MW(addr,r);')
    l('  }')
    l('  if (z != 6) {')
    l('    /* write result back to register */')
    l('    _S8(ws,rz,r);')
    l('  }')
    l('}')
    l('_S_F(f);')
    dec_indent()
    l('}')
    l('break;')

#-------------------------------------------------------------------------------
# Return code to setup an address variable 'a' with the address of HL
# or (IX+d), (IY+d). For the index instructions also update WZ with
# IX+d or IY+d
#
def addr(ext_ticks) :
    return '_ADDR(addr,'+str(ext_ticks)+');'

#-------------------------------------------------------------------------------
#   out_n_a
#
#   Generate code for OUT (n),A
#
def out_n_a():
    src = '{'
    src += '_IMM8(d8);'
    src += 'uint8_t a=_G_A();'
    src += 'addr=(a<<8)|d8;'
    src += '_OUT(addr,a);'
    src += '_S_WZ((addr&0xFF00)|((addr+1)&0x00FF));'
    src += '}'
    return src

#-------------------------------------------------------------------------------
#   in_A_n
#
#   Generate code for IN A,(n)
#
def in_n_a():
    src = '{'
    src += '_IMM8(d8);'
    src += 'uint8_t a=_G_A();'
    src += 'addr=(a<<8)|d8;'
    src += '_IN(addr++,a);'
    src += '_S_A(a);'
    src += '_S_WZ(addr);'
    src += '}'
    return src

#-------------------------------------------------------------------------------
#   ex_af
#
#   Generate code for EX AF,AF'
#
def ex_af():
    src ='{'
    src+='r0=_z80_flush_r0(ws,r0,r2);'
    src+='uint16_t fa=_G16(r0,_FA);'
    src+='uint16_t fa_=_G16(r3,_FA);'
    src+='_S16(r0,_FA,fa_);'
    src+='_S16(r3,_FA,fa);'
    src+='ws=_z80_map_regs(r0,r1,r2);'
    src+='}'
    return src

#-------------------------------------------------------------------------------
#   ex_de_hl
#
#   Generate code for EX DE,HL
#
def ex_de_hl():
    src ='{'
    src+='r0=_z80_flush_r0(ws,r0,r2);'
    src+='uint16_t de=_G16(r0,_DE);'
    src+='uint16_t hl=_G16(r0,_HL);'
    src+='_S16(r0,_DE,hl);'
    src+='_S16(r0,_HL,de);'
    src+='ws=_z80_map_regs(r0,r1,r2);'
    src+='}'
    return src

#-------------------------------------------------------------------------------
#   ex_sp_dd
#
#   Generate code for EX (SP),HL; EX (SP),IX and EX (SP),IY
#
def ex_sp_dd():
    src ='{'
    src+='_T(3);'
    src+='addr=_G_SP();'
    src+='d16=_G_HL();'
    src+='uint8_t l,h;'
    src+='_MR(addr,l);'
    src+='_MR(addr+1,h);'
    src+='_MW(addr,d16);'
    src+='_MW(addr+1,d16>>8);'
    src+='d16=(h<<8)|l;'
    src+='_S_HL(d16);'
    src+='_S_WZ(d16);'
    src+='}'
    return src

#-------------------------------------------------------------------------------
#   exx
#
#   Generate code for EXX
#
def exx():
    src ='{'
    src+='r0=_z80_flush_r0(ws,r0,r2);'
    src+='const uint64_t rx=r3;'
    src+='r3=(r3&0xffff)|(r0&0xffffffffffff0000);'
    src+='r0=(r0&0xffff)|(rx&0xffffffffffff0000);'
    src+='ws=_z80_map_regs(r0, r1, r2);'
    src+='}'
    return src

#-------------------------------------------------------------------------------
#   pop_dd
#
#   Generate code for POP dd.
#
def pop_dd(p):
    src ='addr=_G_SP();'
    src+='_MR(addr++,d8);'
    # special case POP AF, F<=>A
    if p==3:
        src+='d16=d8<<8;'
    else:
        src+='d16=d8;'
    src+='_MR(addr++,d8);'
    if p==3:
        src+='d16|=d8;'
    else:
        src+='d16|=d8<<8;'
    src+='_S_'+rp2[p]+'(d16);'
    src+='_S_SP(addr);'
    return src

#-------------------------------------------------------------------------------
#   push_dd
#
#   Generate code for PUSH dd
#
def push_dd(p):
    src ='_T(1);'
    src+='addr=_G_SP();'
    src+='d16=_G_'+rp2[p]+'();'
    # special case PUSH AF, F<=>A
    if p==3:
        src+='_MW(--addr,d16);'
        src+='_MW(--addr,d16>>8);'
    else:
        src+='_MW(--addr,d16>>8);'
        src+='_MW(--addr,d16);'
    src+='_S_SP(addr);'
    return src

#-------------------------------------------------------------------------------
#   ld_inn_dd
#   LD (nn),dd
#
def ld_inn_dd(p):
    src  = '_IMM16(addr);'
    src += 'd16=_G_'+rp[p]+'();'
    src += '_MW(addr++,d16&0xFF);'
    src += '_MW(addr,d16>>8);'
    src += '_S_WZ(addr);'
    return src

#-------------------------------------------------------------------------------
#   ld_dd_inn
#   LD dd,(nn)
#
def ld_dd_inn(p):
    src  = '_IMM16(addr);'
    src += '_MR(addr++,d8);d16=d8;'
    src += '_MR(addr,d8);d16|=d8<<8;'
    src += '_S_'+rp[p]+'(d16);'
    src += '_S_WZ(addr);'
    return src

#-------------------------------------------------------------------------------
#   call_nn
#
#   Generate code for CALL nn
#
def call_nn():
    src ='_IMM16(addr);'
    src+='_T(1);'
    src+='d16=_G_SP();'
    src+='_MW(--d16,pc>>8);'
    src+='_MW(--d16,pc);'
    src+='_S_SP(d16);'
    src+='pc=addr;'
    return src

#-------------------------------------------------------------------------------
#   call_cc_nn
#
#   Generate code for CALL cc,nn
#
def call_cc_nn(y):
    src ='_IMM16(addr);'
    src+='if('+cond[y]+'){'
    src+='_T(1);'
    src+='uint16_t sp=_G_SP();'
    src+='_MW(--sp,pc>>8);'
    src+='_MW(--sp,pc);'
    src+='_S_SP(sp);'
    src+='pc=addr;'
    src+='}'
    return src

#-------------------------------------------------------------------------------
#   ldi_ldd_ldir_lddr()
#
#   Generate code for LDI, LDIR, LDD, LDDR
#
def ldi_ldd_ldir_lddr(y):
    src ='{'
    src+='uint16_t hl=_G_HL();'
    src+='uint16_t de=_G_DE();'
    src+='_MR(hl,d8);'
    src+='_MW(de,d8);'
    if y & 1:
        src+='hl--;de--;'
    else:
        src+='hl++;de++;'
    src+='_S_HL(hl);'
    src+='_S_DE(de);'
    src+='_T(2);'
    src+='d8+=_G_A();'
    src+='uint8_t f=_G_F()&(Z80_SF|Z80_ZF|Z80_CF);'
    src+='if(d8&0x02){f|=Z80_YF;}'
    src+='if(d8&0x08){f|=Z80_XF;}'
    src+='uint16_t bc=_G_BC();'
    src+='bc--;'
    src+='_S_BC(bc);'
    src+='if(bc){f|=Z80_VF;}'
    src+='_S_F(f);'
    if y >= 6:
        src+='if(bc){'
        src+='pc-=2;'
        src+='_S_WZ(pc+1);'
        src+='_T(5);'
        src+='}'
    src+='}'
    return src

#-------------------------------------------------------------------------------
#   cpi_cpd_cpir_cpdr()
#
#   Generate code for CPI, CPD, CPIR, CPDR
#
def cpi_cpd_cpir_cpdr(y):
    src ='{'
    src+='uint16_t hl = _G_HL();'
    src+='_MR(hl,d8);'
    src+='uint16_t wz = _G_WZ();'
    if y & 1:
        src+='hl--;wz--;'
    else:
        src+='hl++;wz++;'
    src+='_S_WZ(wz);'
    src+='_S_HL(hl);'
    src+='_T(5);'
    src+='int r=((int)_G_A())-d8;'
    src+='uint8_t f=(_G_F()&Z80_CF)|Z80_NF|_SZ(r);'
    src+='if((r&0x0F)>(_G_A()&0x0F)){'
    src+='f|=Z80_HF;'
    src+='r--;'
    src+='}'
    src+='if(r&0x02){f|=Z80_YF;}'
    src+='if(r&0x08){f|=Z80_XF;}'
    src+='uint16_t bc=_G_BC();'
    src+='bc--;'
    src+='_S_BC(bc);'
    src+='if(bc){f|=Z80_VF;}'
    src+='_S8(ws,_F,f);'
    if y >= 6:
        src+='if(bc&&!(f&Z80_ZF)){'
        src+='pc-=2;'
        src+='_S_WZ(pc+1);'
        src+='_T(5);'
        src+='}'
    src+='}'
    return src

#-------------------------------------------------------------------------------
#   ini_ind_inir_indr()
#
#   Generate code for INI, IND, INIR, INDR
#
def ini_ind_inir_indr(y):
    src ='{'
    src+='_T(1);'
    src+='addr=_G_BC();'
    src+='uint16_t hl=_G_HL();'
    src+='_IN(addr,d8);'
    src+='_MW(hl,d8);'
    src+='uint8_t b=_G_B();'
    src+='uint8_t c=_G_C();'
    src+='b--;'
    if y & 1:
        src+='addr--;hl--;c--;'
    else:
        src+='addr++;hl++;c++;'
    src+='_S_B(b);'
    src+='_S_HL(hl);'
    src+='_S_WZ(addr);'
    src+='uint8_t f=b?(b&Z80_SF):Z80_ZF;'
    src+='if(d8&Z80_SF){f|=Z80_NF;}'
    src+='uint32_t t=(uint32_t)(c&0xFF)+d8;'
    src+='if(t&0x100){f|=Z80_HF|Z80_CF;}'
    src+='f|=_z80_szp[((uint8_t)(t&0x07))^b]&Z80_PF;'
    src+='_S_F(f);'
    if y >= 6:
        src+='if(b){'
        src+='pc-=2;'
        src+='_T(5);'
        src+='}'
    src+='}'
    return src

#-------------------------------------------------------------------------------
#   outi_outd_otir_otdr()
#
#   Generate code OUTI, OUTD, OTIR, OTDR
#
def outi_outd_otir_otdr(y):
    src ='{'
    src+='_T(1);'
    src+='uint16_t hl=_G_HL();'
    src+='_MR(hl,d8);'
    src+='uint8_t b=_G_B();'
    src+='b--;'
    src+='_S_B(b);'
    src+='addr=_G_BC();'
    src+='_OUT(addr,d8);'
    if y & 1:
        src+='addr--;hl--;'
    else:
        src+='addr++; hl++;'
    src+='_S_HL(hl);'
    src+='_S_WZ(addr);'
    src+='uint8_t f=b?(b&Z80_SF):Z80_ZF;'
    src+='if(d8&Z80_SF){f|=Z80_NF;}'
    src+='uint32_t t=(uint32_t)_G_L()+(uint32_t)d8;'
    src+='if (t&0x0100){f|=Z80_HF|Z80_CF;}'
    src+='f|=_z80_szp[((uint8_t)(t&0x07))^b]&Z80_PF;'
    src+='_S_F(f);'
    if y >= 6:
        src+='if(b){'
        src+='pc-=2;'
        src+='_T(5);'
        src+='}'
    src+='}'
    return src

#-------------------------------------------------------------------------------
#   djnz()
#
def djnz():
    src ='{'
    src+='_T(1);'
    src+='int8_t d;_IMM8(d);'
    src+='d8=_G_B()-1;'
    src+='_S_B(d8);'
    src+='if(d8>0){pc+=d;_S_WZ(pc);_T(5);}'
    src+='}'
    return src

#-------------------------------------------------------------------------------
#   jr()
#
def jr():
    return '{int8_t d;_IMM8(d);pc+=d;_S_WZ(pc);_T(5);}'

#-------------------------------------------------------------------------------
#   jr_cc()
#
def jr_cc(y):
    src ='{int8_t d;_IMM8(d);'
    src+='if('+cond[y-4]+'){pc+=d;_S_WZ(pc);_T(5);}'
    src+='}'
    return src

#-------------------------------------------------------------------------------
#   ret()
#
def ret():
    src  = 'd16=_G_SP();'
    src += '_MR(d16++,d8);pc=d8;'
    src += '_MR(d16++,d8);pc|=d8<<8;'
    src += '_S_SP(d16);'
    src += '_S_WZ(pc);'
    return src

#-------------------------------------------------------------------------------
#   ret_cc()
#
def ret_cc(y):
    src ='_T(1);'
    src+='if ('+cond[y]+'){'
    src+='uint8_t w,z;'
    src+='d16=_G_SP();'
    src+='_MR(d16++,z);'
    src+='_MR(d16++,w);'
    src+='_S_SP(d16);'
    src+='pc=(w<<8)|z;'
    src+='_S_WZ(pc);'
    src+='}'
    return src

#-------------------------------------------------------------------------------
#   reti()
#
def reti():
    # same as RET, but also set the virtual Z80_RETI pin
    src  = '_ON(Z80_RETI);'
    src += 'd16=_G_SP();'
    src += '_MR(d16++,d8);pc=d8;'
    src += '_MR(d16++,d8);pc|=d8<<8;'
    src += '_S_SP(d16);'
    src += '_S_WZ(pc);'
    return src

#-------------------------------------------------------------------------------
#   rst()
#
def rst(y):
    src ='_T(1);'
    src+='d16= _G_SP();'
    src+='_MW(--d16, pc>>8);'
    src+='_MW(--d16, pc);'
    src+='_S_SP(d16);'
    src+='pc='+hex(y*8)+';'
    src+='_S_WZ(pc);'
    return src

#-------------------------------------------------------------------------------
#   in_r_ic
#   IN r,(C)
#
def in_r_ic(y):
    src ='{'
    src+='addr=_G_BC();'
    src+='_IN(addr++,d8);'
    src+='_S_WZ(addr);'
    src+='uint8_t f=(_G_F()&Z80_CF)|_z80_szp[d8];'
    src+='_S8(ws,_F,f);'
    # handle undocumented special case IN F,(C): 
    # only set flags, don't store result
    if (y != 6):
        src+='_S_'+r[y]+'(d8);'
    src+='}'
    return src

#-------------------------------------------------------------------------------
#   out_r_ic()
#   OUT r,(C)
#
def out_r_ic(y):
    src ='addr=_G_BC();'
    if y == 6:
        src+='_OUT(addr++,0);'
    else:
        src+='_OUT(addr++,_G_'+r[y]+'());'
    src+='_S_WZ(addr);'
    return src

#-------------------------------------------------------------------------------
#   ALU functions.
#
def add8():
    src ='{'
    src+='uint8_t acc=_G_A();'
    src+='uint32_t res=acc+d8;'
    src+='_S_F(_ADD_FLAGS(acc,d8,res));'
    src+='_S_A(res);'
    src+='}'
    return src

def adc8():
    src ='{'
    src+='uint8_t acc=_G_A();'
    src+='uint32_t res=acc+d8+(_G_F()&Z80_CF);'
    src+='_S_F(_ADD_FLAGS(acc,d8,res));'
    src+='_S_A(res);'
    src+='}'
    return src

def sub8():
    src ='{'
    src+='uint8_t acc=_G_A();'
    src+='uint32_t res=(uint32_t)((int)acc-(int)d8);'
    src+='_S_F(_SUB_FLAGS(acc,d8,res));'
    src+='_S_A(res);'
    src+='}'
    return src

def sbc8():
    src ='{'
    src+='uint8_t acc=_G_A();'
    src+='uint32_t res=(uint32_t)((int)acc-(int)d8-(_G_F()&Z80_CF));'
    src+='_S_F(_SUB_FLAGS(acc,d8,res));'
    src+='_S_A(res);'
    src+='}'
    return src

def and8():
    src ='{'
    src+='d8&=_G_A();'
    src+='_S_F(_z80_szp[d8]|Z80_HF);'
    src+='_S_A(d8);'
    src+='}'
    return src

def xor8():
    src ='{'
    src+='d8^=_G_A();'
    src+='_S_F(_z80_szp[d8]);'
    src+='_S_A(d8);'
    src+='}'
    return src

def or8():
    src ='{'
    src+='d8|=_G_A();'
    src+='_S_F(_z80_szp[d8]);'
    src+='_S_A(d8);'
    src+='}'
    return src

def cp8():
    src ='{'
    src+='uint8_t acc=_G_A();'
    src+='int32_t res=(uint32_t)((int)acc-(int)d8);'
    src+='_S_F(_CP_FLAGS(acc,d8,res));'
    src+='}'
    return src

def alu8(y):
    if (y==0):
        return add8()
    elif (y==1):
        return adc8()
    elif (y==2):
        return sub8()
    elif (y==3):
        return sbc8()
    elif (y==4):
        return and8()
    elif (y==5):
        return xor8()
    elif (y==6):
        return or8()
    elif (y==7):
        return cp8()

def neg8():
    src ='d8=_G_A();'
    src+='_S_A(0);'
    src+=sub8();
    return src

def inc8():
    src ='{'
    src+='uint8_t r=d8+1;'
    src+='uint8_t f=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);'
    src+='if(r==0x80){f|=Z80_VF;}'
    src+='_S_F(f|(_G_F()&Z80_CF));'
    src+='d8=r;'
    src+='}'
    return src

def dec8():
    src ='{'
    src+='uint8_t r=d8-1;'
    src+='uint8_t f=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);'
    src+='if(r==0x7F){f|=Z80_VF;}'
    src+='_S_F(f|(_G_F()&Z80_CF));'
    src+='d8=r;'
    src+='}'
    return src

#-------------------------------------------------------------------------------
#   16-bit add,adc,sbc
#
#   flags computation taken from MAME
#
def add16(p):
    src ='{'
    src+='uint16_t acc=_G_HL();'
    src+='_S_WZ(acc+1);'
    src+='d16=_G_'+rp[p]+'();'
    src+='uint32_t r=acc+d16;'
    src+='_S_HL(r);'
    src+='uint8_t f=_G_F()&(Z80_SF|Z80_ZF|Z80_VF);'
    src+='f|=((acc^r^d16)>>8)&Z80_HF;'
    src+='f|=((r>>16)&Z80_CF)|((r>>8)&(Z80_YF|Z80_XF));'
    src+='_S_F(f);'
    src+='_T(7);'
    src+='}'
    return src

def adc16(p):
    src ='{'
    src+='uint16_t acc=_G_HL();'
    src+='_S_WZ(acc+1);'
    src+='d16=_G_'+rp[p]+'();'
    src+='uint32_t r=acc+d16+(_G_F()&Z80_CF);'
    src+='_S_HL(r);'
    src+='uint8_t f=((d16^acc^0x8000)&(d16^r)&0x8000)>>13;'
    src+='f|=((acc^r^d16)>>8)&Z80_HF;'
    src+='f|=(r>>16)&Z80_CF;'
    src+='f|=(r>>8)&(Z80_SF|Z80_YF|Z80_XF);'
    src+='f|=(r&0xFFFF)?0:Z80_ZF;'
    src+='_S_F(f);'
    src+='_T(7);'
    src+='}'
    return src

def sbc16(p):
    src ='{'
    src+='uint16_t acc=_G_HL();'
    src+='_S_WZ(acc+1);'
    src+='d16=_G_'+rp[p]+'();'
    src+='uint32_t r=acc-d16-(_G_F()&Z80_CF);'
    src+='uint8_t f=Z80_NF|(((d16^acc)&(acc^r)&0x8000)>>13);'
    src+='_S_HL(r);'
    src+='f|=((acc^r^d16)>>8) & Z80_HF;'
    src+='f|=(r>>16)&Z80_CF;'
    src+='f|=(r>>8)&(Z80_SF|Z80_YF|Z80_XF);'
    src+='f|=(r&0xFFFF)?0:Z80_ZF;'
    src+='_S_F(f);'
    src+='_T(7);'
    src+='}'
    return src

#-------------------------------------------------------------------------------
#   rotate and shift functions
#
def rrd():
    src ='{'
    src+='addr=_G_HL();'
    src+='uint8_t a=_G_A();'
    src+='_MR(addr,d8);'
    src+='uint8_t l=a&0x0F;'
    src+='a=(a&0xF0)|(d8&0x0F);'
    src+='_S_A(a);'
    src+='d8=(d8>>4)|(l<<4);'
    src+='_MW(addr++,d8);'
    src+='_S_WZ(addr);'
    src+='_S_F((_G_F()&Z80_CF)|_z80_szp[a]);'
    src+='_T(4);'
    src+='}'
    return src

def rld():
    src ='{'
    src+='addr=_G_HL();'
    src+='uint8_t a=_G_A();'
    src+='_MR(addr,d8);'
    src+='uint8_t l=a&0x0F;'
    src+='a=(a&0xF0)|(d8>>4);'
    src+='_S_A(a);'
    src+='d8=(d8<<4)|l;'
    src+='_MW(addr++,d8);'
    src+='_S_WZ(addr);'
    src+='_S_F((_G_F()&Z80_CF)|_z80_szp[a]);'
    src+='_T(4);'
    src+='}'
    return src

#-------------------------------------------------------------------------------
#   misc ops
#
def halt():
    return '_ON(Z80_HALT);pc--;'

def di():
    return 'r2&=~(_BIT_IFF1|_BIT_IFF2);'

def ei():
    return 'r2|=_BIT_EI;'

#-------------------------------------------------------------------------------
# Encode a main instruction, or an DD or FD prefix instruction.
# Takes an opcode byte and returns an opcode object, for invalid instructions
# the opcode object will be in its default state (opcode.src==None).
# cc is the name of the cycle-count table.
#
def enc_op(op) :

    o = opcode(op)

    # split opcode byte into bit groups, these identify groups
    # or subgroups of instructions, or serve as register indices 
    #
    #   |xx|yyy|zzz|
    #   |xx|pp|q|zzz|
    x = op>>6
    y = (op>>3)&7
    z = op&7
    p = y>>1
    q = y&1

    #---- block 1: 8-bit loads, and HALT
    if x == 1:
        if y == 6:
            if z == 6:
                # special case: LD (HL),(HL) is HALT
                o.cmt = 'HALT'
                o.src = halt()
            else:
                # LD (HL),r; LD (IX+d),r; LD (IX+d),r
                o.cmt = 'LD (HL/IX+d/IY+d),'+r[z]
                # special case LD (IX+d),L LD (IX+d),H
                if z in [4,5]:
                    o.src = 'd8=_IDX()?_G8(r0,_'+r[z]+'):_G_'+r[z]+'();'
                else:
                    o.src = 'd8=_G_'+r[z]+'();'
                o.src += addr(5)+'_MW(addr,d8);'
        elif z == 6:
            # LD r,(HL); LD r,(IX+d); LD r,(IY+d)
            o.cmt = 'LD '+r[y]+',(HL/IX+d/IY+d)'
            o.src = addr(5)+'_MR(addr,d8);'
            if y in [4,5]:
                o.src += 'if(_IDX()){_S8(r0,_'+r[y]+',d8);}else{_S_'+r[y]+'(d8);}'
            else:
                o.src += '_S_'+r[y]+'(d8);'
        else:
            # LD r,s
            o.cmt = 'LD '+r[y]+','+r[z]
            o.src = '_S_'+r[y]+'(_G_'+r[z]+'());'

    #---- block 2: 8-bit ALU instructions (ADD, ADC, SUB, SBC, AND, XOR, OR, CP)
    elif x == 2:
        if z == 6:
            # ALU (HL); ALU (IX+d); ALU (IY+d)
            o.cmt = alu_cmt[y]+',(HL/IX+d/IY+d)'
            o.src = addr(5) + '_MR(addr,d8);'+alu8(y)
        else:
            # ALU r
            o.cmt = alu_cmt[y]+' '+r[z]
            o.src = 'd8=_G_'+r[z]+'();'+alu8(y)

    #---- block 0: misc ops
    elif x == 0:
        if z == 0:
            if y == 0:
                # NOP
                o.cmt = 'NOP'
                o.src = ' '
            elif y == 1:
                # EX AF,AF'
                o.cmt = "EX AF,AF'"
                o.src = ex_af()
            elif y == 2:
                # DJNZ d
                o.cmt = 'DJNZ'
                o.src = djnz()
            elif  y == 3:
                # JR d
                o.cmt = 'JR d'
                o.src = jr()
            else:
                # JR cc,d
                o.cmt = 'JR '+cond_cmt[y-4]+',d'
                o.src = jr_cc(y)
        elif z == 1:
            if q == 0:
                # 16-bit immediate loads
                o.cmt = 'LD '+rp[p]+',nn'
                o.src = '_IMM16(d16);_S_'+rp[p]+'(d16);'
            else :
                # ADD HL,rr; ADD IX,rr; ADD IY,rr
                o.cmt = 'ADD '+rp[2]+','+rp[p]
                o.src = add16(p)
        elif z == 2:
            # indirect loads
            op_tbl = [
                [ 'LD (BC),A',          'addr=_G_BC();d8=_G_A();_MW(addr++,d8);_S_WZ((d8<<8)|(addr&0x00FF));' ],
                [ 'LD A,(BC)',          'addr=_G_BC();_MR(addr++,d8);_S_A(d8);_S_WZ(addr);' ],
                [ 'LD (DE),A',          'addr=_G_DE();d8=_G_A();_MW(addr++,d8);_S_WZ((d8<<8)|(addr&0x00FF));' ],
                [ 'LD A,(DE)',          'addr=_G_DE();_MR(addr++,d8);_S_A(d8);_S_WZ(addr);' ],
                [ 'LD (nn),'+rp[2],     '_IMM16(addr);_MW(addr++,_G_L());_MW(addr,_G_H());_S_WZ(addr);' ],
                [ 'LD '+rp[2]+',(nn)',  '_IMM16(addr);_MR(addr++,d8);_S_L(d8);_MR(addr,d8);_S_H(d8);_S_WZ(addr);' ],
                [ 'LD (nn),A',          '_IMM16(addr);d8=_G_A();_MW(addr++,d8);_S_WZ((d8<<8)|(addr&0x00FF));' ],
                [ 'LD A,(nn)',          '_IMM16(addr);_MR(addr++,d8);_S_A(d8);_S_WZ(addr);' ],
            ]
            o.cmt = op_tbl[y][0]
            o.src = op_tbl[y][1]
        elif z == 3:
            # 16-bit INC/DEC 
            if q == 0:
                o.cmt = 'INC '+rp[p]
                o.src = '_T(2);_S_'+rp[p]+'(_G_'+rp[p]+'()+1);'
            else:
                o.cmt = 'DEC '+rp[p]
                o.src = '_T(2);_S_'+rp[p]+'(_G_'+rp[p]+'()-1);'
        elif z == 4 or z == 5:
            cmt = 'INC' if z == 4 else 'DEC'
            fn = inc8() if z==4 else dec8()
            if y == 6:
                # INC/DEC (HL)/(IX+d)/(IY+d)
                o.cmt = cmt+' (HL/IX+d/IY+d)'
                o.src = addr(5)+'_T(1);_MR(addr,d8);'+fn+'_MW(addr,d8);'
            else:
                # INC/DEC r
                o.cmt = cmt+' '+r[y]
                o.src = 'd8=_G_'+r[y]+'();'+fn+'_S_'+r[y]+'(d8);'
        elif z == 6:
            if y == 6:
                # LD (HL),n; LD (IX+d),n; LD (IY+d),n
                o.cmt = 'LD (HL/IX+d/IY+d),n'
                o.src = addr(2) + '_IMM8(d8);_MW(addr,d8);'
            else:
                # LD r,n
                o.cmt = 'LD '+r[y]+',n'
                o.src = '_IMM8(d8);_S_'+r[y]+'(d8);'
        elif z == 7:
            # misc ops on A and F
            op_tbl = [
                [ 'RLCA', 'ws=_z80_rlca(ws);' ],
                [ 'RRCA', 'ws=_z80_rrca(ws);' ],
                [ 'RLA',  'ws=_z80_rla(ws);' ],
                [ 'RRA',  'ws=_z80_rra(ws);' ],
                [ 'DAA',  'ws=_z80_daa(ws);' ],
                [ 'CPL',  'ws=_z80_cpl(ws);' ],
                [ 'SCF',  'ws=_z80_scf(ws);' ],
                [ 'CCF',  'ws=_z80_ccf(ws);' ]
            ]
            o.cmt = op_tbl[y][0]
            o.src = op_tbl[y][1]

    #--- block 3: misc and extended ops
    elif x == 3:
        if z == 0:
            # RET cc
            o.cmt = 'RET '+cond_cmt[y]
            o.src = ret_cc(y)
        if z == 1:
            if q == 0:
                # POP BC,DE,HL,IX,IY,AF
                o.cmt = 'POP '+rp2[p]
                o.src = pop_dd(p)
            else:
                # misc ops
                op_tbl = [
                    [ 'RET', ret() ],
                    [ 'EXX', exx() ],
                    [ 'JP '+rp[2], 'pc=_G_HL();' ],
                    [ 'LD SP,'+rp[2], '_T(2);_S_SP(_G_HL());' ]
                ]
                o.cmt = op_tbl[p][0]
                o.src = op_tbl[p][1]
        if z == 2:
            # JP cc,nn
            o.cmt = 'JP {},nn'.format(cond_cmt[y])
            o.src = '_IMM16(addr);if('+cond[y]+'){pc=addr;}'
        if z == 3:
            # misc ops
            op_tbl = [
                [ 'JP nn', '_IMM16(pc);' ],
                [ None, None ], # CB prefix instructions
                [ 'OUT (n),A', out_n_a() ],
                [ 'IN A,(n)', in_n_a() ],
                [ 'EX (SP),'+rp[2], ex_sp_dd() ],
                [ 'EX DE,HL', ex_de_hl() ],
                [ 'DI', di() ],
                [ 'EI', ei() ]
            ]
            o.cmt = op_tbl[y][0]
            o.src = op_tbl[y][1]
        if z == 4:
            # CALL cc,nn
            o.cmt = 'CALL {},nn'.format(cond_cmt[y])
            o.src = call_cc_nn(y)
        if z == 5:
            if q == 0:
                # PUSH BC,DE,HL,IX,IY,AF
                o.cmt = 'PUSH {}'.format(rp2[p])
                o.src = push_dd(p)
            else:
                op_tbl = [
                    [ 'CALL nn', call_nn() ],
                    [ 'DD prefix', 'map_bits|=_BIT_USE_IX;continue;' ],
                    [ None, None ], # ED prefix instructions
                    [ 'FD prefix', 'map_bits|=_BIT_USE_IY;continue;'],
                ]
                o.cmt = op_tbl[p][0]
                o.src = op_tbl[p][1]
        if z == 6:
            # ALU n
            o.cmt = '{} n'.format(alu_cmt[y])
            o.src = '_IMM8(d8);'+alu8(y)
        if z == 7:
            # RST
            o.cmt = 'RST {}'.format(hex(y*8))
            o.src = rst(y)

    return o

#-------------------------------------------------------------------------------
#   ED prefix instructions
#
def enc_ed_op(op) :
    o = opcode(op)
    cc = 'cc_ed'

    x = op>>6
    y = (op>>3)&7
    z = op&7
    p = y>>1
    q = y&1

    if x == 2:
        # block instructions (LDIR etc)
        if y >= 4 and z < 4 :
            op_tbl = [
                [ 
                    [ 'LDI',  ldi_ldd_ldir_lddr(y) ],
                    [ 'LDD',  ldi_ldd_ldir_lddr(y) ],
                    [ 'LDIR', ldi_ldd_ldir_lddr(y) ],
                    [ 'LDDR', ldi_ldd_ldir_lddr(y) ]
                ],
                [
                    [ 'CPI',  cpi_cpd_cpir_cpdr(y) ],
                    [ 'CPD',  cpi_cpd_cpir_cpdr(y) ],
                    [ 'CPIR', cpi_cpd_cpir_cpdr(y) ],
                    [ 'CPDR', cpi_cpd_cpir_cpdr(y) ]
                ],
                [
                    [ 'INI',  ini_ind_inir_indr(y) ],
                    [ 'IND',  ini_ind_inir_indr(y) ],
                    [ 'INIR', ini_ind_inir_indr(y) ],
                    [ 'INDR', ini_ind_inir_indr(y) ]
                ],
                [
                    [ 'OUTI', outi_outd_otir_otdr(y) ],
                    [ 'OUTD', outi_outd_otir_otdr(y) ],
                    [ 'OTIR', outi_outd_otir_otdr(y) ],
                    [ 'OTDR', outi_outd_otir_otdr(y) ]
                ]
            ]
            o.cmt = op_tbl[z][y-4][0]
            o.src = op_tbl[z][y-4][1]

    if x == 1:
        # misc ops
        if z == 0:
            # IN r,(C)
            o.cmt = 'IN {},(C)'.format(r[y])
            o.src = in_r_ic(y)
        if z == 1:
            # OUT (C),r
            o.cmt = 'OUT (C),{}'.format(r[y])
            o.src = out_r_ic(y)
        if z == 2:
            # SBC/ADC HL,rr
            if q==0:
                o.cmt = 'SBC HL,'+rp[p]
                o.src = sbc16(p)
            else:
                o.cmt = 'ADC HL,'+rp[p]
                o.src = adc16(p)
        if z == 3:
            # 16-bit immediate address load/store
            if q == 0:
                o.cmt = 'LD (nn),{}'.format(rp[p])
                o.src = ld_inn_dd(p)
            else:
                o.cmt = 'LD {},(nn)'.format(rp[p])
                o.src = ld_dd_inn(p)
        if z == 4:
            # NEG
            o.cmt = 'NEG'
            o.src = neg8()
        if z == 5:
            # RETN, RETI (only RETI implemented!)
            if y == 1:
                o.cmt = 'RETI'
                o.src = reti()
        if z == 6:
            # IM m
            im_mode = [ 0, 0, 1, 2, 0, 0, 1, 2 ]
            o.cmt = 'IM {}'.format(im_mode[y])
            o.src = '_S_IM({});'.format(im_mode[y])
        if z == 7:
            # misc ops on I,R and A
            op_tbl = [
                [ 'LD I,A', '_T(1);_S_I(_G_A());' ],
                [ 'LD R,A', '_T(1);_S_R(_G_A());' ],
                [ 'LD A,I', '_T(1);d8=_G_I();_S_A(d8);_S_F(_SZIFF2_FLAGS(d8));' ],
                [ 'LD A,R', '_T(1);d8=_G_R();_S_A(d8);_S_F(_SZIFF2_FLAGS(d8));' ],
                [ 'RRD', rrd() ],
                [ 'RLD', rld() ],
                [ 'NOP (ED)', ' ' ],
                [ 'NOP (ED)', ' ' ],
            ]
            o.cmt = op_tbl[y][0]
            o.src = op_tbl[y][1]
    return o

#-------------------------------------------------------------------------------
#   CB prefix instructions
#
def enc_cb_op(op) :
    o = opcode(op)

    x = op>>6
    y = (op>>3)&7
    z = op&7

    # NOTE x==0 (ROT), x==1 (BIT), x==2 (RES) and x==3 (SET) instructions are
    # handled dynamically in the fallthrough path!
    return o

#-------------------------------------------------------------------------------
# indent and tab stuff
#
def inc_indent():
    global indent
    indent += 1

def dec_indent():
    global indent
    indent -= 1

def tab() :
    return ' '*TabWidth*indent

#-------------------------------------------------------------------------------
# write a single (writes a case inside the current switch)
#
def write_op(op) :
    if op.src :
        if not op.cmt:
            op.cmt='???'
        l('case '+hex(op.byte)+':/*'+op.cmt+'*/'+op.src+'break;')

#-------------------------------------------------------------------------------
# main encoder function, this populates all the opcode tables and
# generates the C++ source code into the file f
#
Out = open(OutPath, 'w')
write_header()

# loop over all instruction bytes
indent = 3
for i in range(0, 256):
    # ED prefix instructions
    if i == 0xED:
        write_ed_ops()
    # CB prefix instructions
    elif i == 0xCB:
        write_cb_ops()
    # non-prefixed instruction
    else:
        write_op(enc_op(i))
indent = 0
write_op_post()
write_interrupt_handling()
write_footer()
Out.close()
