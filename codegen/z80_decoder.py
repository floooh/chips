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
# for extended ops, H, L, HL will be patched with IXH, IXL, IX
r = [ 'B', 'C', 'D', 'E', 'H', 'L', 'F', 'A' ]

# same as 'r', but not patched for extended ops
rx = [ 'B', 'C', 'D', 'E', 'H', 'L', 'F', 'A' ]

def rpsp_cmt(p):
    if p==3:
        return 'SP'
    elif p==2:
        if r[4]=='IXH':
            return 'IX'
        elif r[4]=='IYH':
            return 'IY'
        else:
            return 'HL'
    else:
       return r[p*2+0]+r[p*2+1]

def rpaf_cmt(p):
    if p==3:
        return 'AF'
    elif p==2:
        if r[4]=='IXH':
            return 'IX'
        elif r[4]=='IYH':
            return 'IY'
        else:
            return 'HL'
    else:
       return r[p*2+0]+r[p*2+1]

# condition-code table (for conditional jumps etc)
cond = [
    '!(F&Z80_ZF)',  # NZ
    '(F&Z80_ZF)',   # Z
    '!(F&Z80_CF)',  # NC
    '(F&Z80_CF)',   # C
    '!(F&Z80_PF)',  # PO
    '(F&Z80_PF)',   # PE
    '!(F&Z80_SF)',  # P
    '(F&Z80_SF)'    # M
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
    l('  uint8_t B=cpu->b, C=cpu->c;')
    l('  uint8_t D=cpu->d, E=cpu->e;')
    l('  uint8_t H=cpu->h, L=cpu->l;')
    l('  uint8_t A=cpu->a, F=cpu->f;')
    l('  uint8_t IXH=cpu->ixh, IXL=cpu->ixl;')
    l('  uint8_t IYH=cpu->iyh, IYL=cpu->iyl;')
    l('  uint16_t BC_=cpu->bc_, DE_=cpu->de_, HL_=cpu->hl_, FA_=cpu->fa_;')
    l('  uint16_t WZ=cpu->wz, SP=cpu->sp, PC=cpu->pc;')
    l('  uint8_t IM=cpu->im, I=cpu->i, R=cpu->r;')
    l('  bool IFF1=cpu->iff1, IFF2=cpu->iff2, EI_PENDING=cpu->ei_pending;')
    l('  uint64_t pins = cpu->pins;')
    l('  const uint64_t trap_addr = cpu->trap_addr;')
    l('  const z80_tick_t tick = cpu->tick;')
    l('  void* ud = cpu->user_data;')
    l('  int trap_id = -1;')
    l('  uint32_t ticks = 0;')
    l('  uint8_t op, d8;')
    l('  uint16_t addr, d16;')
    l('  uint64_t pre_pins = pins;')
    l('  do {')
    l('    _FETCH(op)')
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
    l('    pins&=~Z80_INT;')
    l('    /* delay-enable interrupt flags */')
    l('    if (EI_PENDING) {')
    l('      EI_PENDING=false;')
    l('      IFF1=IFF2=true;')
    l('    }')
    l('    if (trap_addr != 0xFFFFFFFFFFFFFFFF) {')
    l('      uint64_t ta = trap_addr;')
    l('      for (int i = 0; i < Z80_MAX_NUM_TRAPS; i++) {')
    l('        if (((ta & 0xFFFF) == PC) && (PC != 0xFFFF)) {')
    l('          trap_id = i;')
    l('          break;')
    l('        }')
    l('        ta >>= 16;')
    l('      }')
    l('    }')
    l('    pre_pins = pins;')
    l('  } while ((ticks < num_ticks) && (trap_id < 0));')
    l('  cpu->b=B; cpu->c=C;')
    l('  cpu->d=D; cpu->e=E;')
    l('  cpu->h=H; cpu->l=L;')
    l('  cpu->a=A; cpu->f=F;')
    l('  cpu->ixh=IXH; cpu->ixl=IXL;')
    l('  cpu->iyh=IYH; cpu->iyl=IYL;')
    l('  cpu->bc_=BC_; cpu->de_=DE_; cpu->hl_=HL_; cpu->fa_=FA_;')
    l('  cpu->wz=WZ; cpu->sp=SP; cpu->pc=PC;')
    l('  cpu->im=IM; cpu->i=I; cpu->r=R;')
    l('  cpu->iff1=IFF1; cpu->iff2=IFF2; cpu->ei_pending=EI_PENDING;')
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
#   Updated:
#       - 05-Sep-2018: don't handle interrupt when _BIT_EI is set, this prevents
#                      interrupt handling during sequences of EI
#
def write_interrupt_handling():
    l('    bool nmi = 0 != ((pins & (pre_pins ^ pins)) & Z80_NMI);')
    l('    if (nmi || (((pins & (Z80_INT|Z80_BUSREQ))==Z80_INT) && IFF1)) {')
    l('      IFF1=false;');
    l('      if (pins & Z80_INT) {');
    l('        IFF2=false;');
    l('      }')
    l('      if (pins & Z80_HALT) {')
    l('        pins &= ~Z80_HALT;')
    l('        PC++;')
    l('      }')
    l('      _SA(PC);')
    l('      if (nmi) {')
    l('        _TWM(5,Z80_M1|Z80_MREQ|Z80_RD);_BUMPR();')
    l('        _MW(--SP,PC>>8);')
    l('        _MW(--SP,PC);')
    l('        PC=WZ=0x0066;')
    l('      }')
    l('      else {')
    l('        _TWM(4,Z80_M1|Z80_IORQ);')
    l('        const uint8_t int_vec=_GD();')
    l('        _BUMPR();')
    l('        _T(2);')
    l('        switch (IM) {')
    l('          case 0:')
    l('            break;')
    l('          case 1:')
    l('            {')
    l('              _MW(--SP,PC>>8);')
    l('              _MW(--SP,PC);')
    l('              PC=WZ=0x0038;')
    l('            }')
    l('            break;')
    l('          case 2:')
    l('            {')
    l('              _MW(--SP,PC>>8);')
    l('              _MW(--SP,PC);')
    l('              addr = (I<<8) | (int_vec & 0xFE);')
    l('              uint8_t z,w;')
    l('              _MR(addr++,z);')
    l('              _MR(addr,w);')
    l('              PC=WZ=(w<<8)|z;')
    l('            }')
    l('            break;')
    l('        }')
    l('       }')
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
    l('default: break;')
    dec_indent()
    l('}')
    dec_indent()
    l('}')
    l('break;')

#-------------------------------------------------------------------------------
# Write the CB extended instruction block as 'hand-decoded' ops
#
def write_cb_ops(ixy):
    l('case 0xCB: {')
    inc_indent()
    l('/* special handling for undocumented DD/FD+CB double prefix instructions,')
    l(' these always load the value from memory (IX+d),')
    l(' and write the value back, even for normal')
    l(' "register" instructions')
    l(' see: http://www.baltazarstudios.com/files/ddcb.html')
    l('*/')
    if ixy:
        l('int8_t d;_IMM8(d);')
        l('_FETCH_CB(op);')
        l('const uint8_t x = op>>6;')
        l('const uint8_t y = (op>>3)&7;')
        l('const uint8_t z = op&7;')
        l(addr_cb(ixy,2))
        l('_MR(addr,d8);')
    else:
        l('_FETCH_CB(op);_BUMPR();')
        l('const uint8_t x = op>>6;')
        l('const uint8_t y = (op>>3)&7;')
        l('const uint8_t z = op&7;')
        l('switch(z) {')
        l('  case 0: d8=B; break;')
        l('  case 1: d8=C; break;')
        l('  case 2: d8=D; break;')
        l('  case 3: d8=E; break;')
        l('  case 4: d8=H; break;')
        l('  case 5: d8=L; break;')
        l('  case 6: _T(1); addr=(H<<8)|L;_MR(addr,d8); break;')
        l('  case 7: d8=A; break;')
        l('}')
    l('uint8_t r;')
    l('switch (x) {')
    l('  case 0:')
    l('     /* rot/shift */')
    l('     switch (y) {')
    l('       case 0: /*RLC*/ r=d8<<1|d8>>7; F=_z80_szp[r]|(d8>>7&Z80_CF); break;')
    l('       case 1: /*RRC*/ r=d8>>1|d8<<7; F=_z80_szp[r]|(d8&Z80_CF); break;')
    l('       case 2: /*RL */ r=d8<<1|(F&Z80_CF); F=_z80_szp[r]|(d8>>7&Z80_CF); break;')
    l('       case 3: /*RR */ r=d8>>1|((F&Z80_CF)<<7); F=_z80_szp[r]|(d8&Z80_CF); break;')
    l('       case 4: /*SLA*/ r=d8<<1; F=_z80_szp[r]|(d8>>7&Z80_CF); break;')
    l('       case 5: /*SRA*/ r=d8>>1|(d8&0x80); F=_z80_szp[r]|(d8&Z80_CF); break;')
    l('       case 6: /*SLL*/ r=d8<<1|1; F=_z80_szp[r]|(d8>>7&Z80_CF); break;')
    l('       case 7: /*SRL*/ r=d8>>1; F=_z80_szp[r]|(d8&Z80_CF); break;')
    l('     }')
    l('     break;')
    l('  case 1:')
    l('    /* BIT (bit test) */')
    l('    r = d8 & (1<<y);')
    l('    F = (F&Z80_CF) | Z80_HF | (r?(r&Z80_SF):(Z80_ZF|Z80_PF));')
    if ixy:
        l('    F |= (WZ>>8) & (Z80_YF|Z80_XF);')
    else:
        l('    if (z == 6) {')
        l('      F |= (WZ>>8) & (Z80_YF|Z80_XF);')
        l('    }')
        l('    else {')
        l('      F |= d8 & (Z80_YF|Z80_XF);')
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
    if ixy:
        l('  _MW(addr,r);')
    l('  switch (z) {')
    l('    case 0: B=r; break;')
    l('    case 1: C=r; break;')
    l('    case 2: D=r; break;')
    l('    case 3: E=r; break;')
    l('    case 4: H=r; break;')
    l('    case 5: L=r; break;')
    if ixy:
        l('    case 6: break;')
    else:
        l('    case 6: _MW(addr,r); break;')
    l('    case 7: A=r; break;')
    l('  }')
    l('}')
    dec_indent()
    l('}')
    l('break;')

#-------------------------------------------------------------------------------
# Write the ED extended instruction block.
#
def write_ixy_ops(op):
    if op==0xDD:
        ixy = 'IX'
        r[4] = 'IXH'
        r[5] = 'IXL'
    else:
        ixy = 'IY'
        r[4] = 'IYH'
        r[5] = 'IYL'
    l('case '+hex(op)+': {')
    inc_indent()
    l('_FETCH(op);')
    l('switch (op) {')
    inc_indent()
    for i in range(0, 256):
        # CB prefix instructions
        if i == 0xCB:
            write_cb_ops(ixy)
        # non-prefixed instruction
        else:
            write_op(enc_op(ixy,i))
    l('default: break;')
    l('}')
    dec_indent()
    l('}')
    l('break;')
    r[4] = 'H'
    r[5] = 'L'

#-------------------------------------------------------------------------------
# Return code to setup an address variable 'a' with the address of HL
# or (IX+d), (IY+d). For the index instructions also update WZ with
# IX+d or IY+d
#
def addr(ixy, ext_ticks):
    if ixy:
        src = 'addr='
        if ixy=='IX':
            src += 'IXH<<8|IXL;'
        else:
            src += 'IYH<<8|IYL;'
        src += '{int8_t d;_MR(PC++,d);addr+=d;}WZ=addr;_T('+str(ext_ticks)+');'
        return src
    else:
        return 'addr=(H<<8)|L;'

#-------------------------------------------------------------------------------
#   same as addr(), but with existing d (for CB prefixed ops)
#
def addr_cb(ixy, ext_ticks):
    if ixy:
        src = 'addr='
        if ixy=='IX':
            src += 'IXH<<8|IXL;'
        else:
            src += 'IYH<<8|IYL;'
        src += 'addr+=d;WZ=addr;_T('+str(ext_ticks)+');'
        return src
    else:
        return 'addr=(H<<8)|L;'

#-------------------------------------------------------------------------------
# Return a (HL)/(IX+d)/(IY+d) comment string
#
def addr_cmt(ixy):
    if ixy:
        return '('+ixy+'+d)'
    else:
        return '(HL)'

#-------------------------------------------------------------------------------
#   out_n_a
#
#   Generate code for OUT (n),A
#
def out_n_a():
    return ('{'
            '_IMM8(d8);'
            'addr=(A<<8)|d8;'
            '_OUT(addr,A);'
            'WZ=(addr&0xFF00)|((addr+1)&0x00FF);'
            '}')

#-------------------------------------------------------------------------------
#   in_A_n
#
#   Generate code for IN A,(n)
#
def in_n_a():
    return ('{'
            '_IMM8(d8);'
            'WZ=(A<<8)|d8;'
            '_IN(WZ++,A);'
            '}')

#-------------------------------------------------------------------------------
#   ex_af
#
#   Generate code for EX AF,AF'
#
def ex_af():
    return ('{'
            'uint16_t tmp=(F<<8)|A;'
            'A=FA_&0xFF;F=FA_>>8;'
            'FA_=tmp;'
            '}')

#-------------------------------------------------------------------------------
#   ex_de_hl
#
#   Generate code for EX DE,HL
#
def ex_de_hl():
    return 'D=D^H;H=H^D;D=D^H;E=E^L;L=L^E;E=E^L;'

#-------------------------------------------------------------------------------
#   ex_sp_dd
#
#   Generate code for EX (SP),HL; EX (SP),IX and EX (SP),IY
#
def ex_sp_dd():
    return ('{'
            '_T(3);'
            'uint8_t l,h;'
            '_MR(SP,l);'
            '_MR(SP+1,h);'
            '_MW(SP,'+r[5]+');'
            '_MW(SP+1,'+r[4]+');'
            +r[4]+'=h;'+r[5]+'=l;'
            'WZ=(h<<8)|l;'
            '}')

#-------------------------------------------------------------------------------
#   exx
#
#   Generate code for EXX
#
def exx():
    return ('{'
            'uint16_t hl=(H<<8)|L;'
            'uint16_t de=(D<<8)|E;'
            'uint16_t bc=(B<<8)|C;'
            'H=HL_>>8;L=HL_;HL_=hl;'
            'D=DE_>>8;E=DE_;DE_=de;'
            'B=BC_>>8;C=BC_;BC_=bc;'
            '}')

#-------------------------------------------------------------------------------
#   pop_dd
#
#   Generate code for POP dd.
#
def pop_dd(p):
    if p==3:
        # special case PUSH AF
        return '_MR(SP++,F);_MR(SP++,A);'
    else:
        return '_MR(SP++,'+r[p*2+1]+');_MR(SP++,'+r[p*2+0]+');'

#-------------------------------------------------------------------------------
#   push_dd
#
#   Generate code for PUSH dd
#
def push_dd(p):
    if p==3:
        # special case PUSH AF
        return '_T(1);_MW(--SP,A);_MW(--SP,F);'
    else:
        return '_T(1);_MW(--SP,'+r[p*2+0]+');_MW(--SP,'+r[p*2+1]+');'

#-------------------------------------------------------------------------------
#   ld_inn_dd
#   LD (nn),dd
#
def ld_inn_dd(p):
    src  = '_IMM16();'
    if p==3:
        src += '_MW(WZ++,SP);'
        src += '_MW(WZ,SP>>8);'
    else:
        src += '_MW(WZ++,'+r[p*2+1]+');'
        src += '_MW(WZ,'+r[p*2+0]+');'
    return src

#-------------------------------------------------------------------------------
#   ld_dd_inn
#   LD dd,(nn)
#
def ld_dd_inn(p):
    src  = '_IMM16();'
    if p==3:
        src += '{'
        src += 'uint8_t l,h;'
        src += '_MR(WZ++,l);'
        src += '_MR(WZ,h);'
        src += 'SP=(h<<8)|l;'
        src += '}'
    else:
        src += '_MR(WZ++,'+r[p*2+1]+');'
        src += '_MR(WZ,'+r[p*2+0]+');'
    return src

#-------------------------------------------------------------------------------
#   call_nn
#
#   Generate code for CALL nn
#
def call_nn():
    return ('_IMM16();'
            '_T(1);'
            '_MW(--SP,PC>>8);'
            '_MW(--SP,PC);'
            'PC=WZ;')

#-------------------------------------------------------------------------------
#   call_cc_nn
#
#   Generate code for CALL cc,nn
#
def call_cc_nn(y):
    return ('_IMM16();'
            'if('+cond[y]+'){'
            '_T(1);'
            '_MW(--SP,PC>>8);'
            '_MW(--SP,PC);'
            'PC=WZ;'
            '}')

#-------------------------------------------------------------------------------
#   ldi_ldd_ldir_lddr()
#
#   Generate code for LDI, LDIR, LDD, LDDR
#
def ldi_ldd_ldir_lddr(y):
    src ='{'
    src+='uint16_t hl=(H<<8)|L;'
    src+='uint16_t de=(D<<8)|E;'
    src+='_MR(hl,d8);'
    src+='_MW(de,d8);'
    if y & 1:
        src+='hl--;de--;'
    else:
        src+='hl++;de++;'
    src+='H=hl>>8;L=hl;'
    src+='D=de>>8;E=de;'
    src+='_T(2);'
    src+='d8+=A;'
    src+='F&=Z80_SF|Z80_ZF|Z80_CF;'
    src+='if(d8&0x02){F|=Z80_YF;}'
    src+='if(d8&0x08){F|=Z80_XF;}'
    src+='uint16_t bc=(B<<8)|C;'
    src+='bc--;'
    src+='B=bc>>8;C=bc;'
    src+='if(bc){F|=Z80_VF;}'
    if y >= 6:
        src+='if(bc){'
        src+='PC-=2;'
        src+='WZ=PC+1;'
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
    src+='uint16_t hl=(H<<8)|L;'
    src+='_MR(hl,d8);'
    if y & 1:
        src+='hl--;WZ--;'
    else:
        src+='hl++;WZ++;'
    src+='H=hl>>8;L=hl;'
    src+='_T(5);'
    src+='int r=((int)A)-d8;'
    src+='F=(F&Z80_CF)|Z80_NF|_SZ(r);'
    src+='if((r&0x0F)>(A&0x0F)){'
    src+='F|=Z80_HF;'
    src+='r--;'
    src+='}'
    src+='if(r&0x02){F|=Z80_YF;}'
    src+='if(r&0x08){F|=Z80_XF;}'
    src+='uint16_t bc=(B<<8)|C;'
    src+='bc--;'
    src+='B=bc>>8;C=bc;'
    src+='if(bc){F|=Z80_VF;}'
    if y >= 6:
        src+='if(bc&&!(F&Z80_ZF)){'
        src+='PC-=2;'
        src+='WZ=PC+1;'
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
    src+='WZ=(B<<8)|C;'
    src+='uint16_t hl=(H<<8)|L;'
    src+='_IN(WZ,d8);'
    src+='_MW(hl,d8);'
    src+='B--;'
    src+='uint8_t c=C;'
    if y & 1:
        src+='WZ--;hl--;c--;'
    else:
        src+='WZ++;hl++;c++;'
    src+='H=hl>>8;L=hl;'
    src+='F=(B?(B&Z80_SF):Z80_ZF)|(B&(Z80_XF|Z80_YF));'
    src+='if(d8&Z80_SF){F|=Z80_NF;}'
    src+='uint32_t t=(uint32_t)(c+d8);'
    src+='if(t&0x100){F|=Z80_HF|Z80_CF;}'
    src+='F|=_z80_szp[((uint8_t)(t&0x07))^B]&Z80_PF;'
    if y >= 6:
        src+='if(B){'
        src+='PC-=2;'
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
    src+='uint16_t hl=(H<<8)|L;'
    src+='_MR(hl,d8);'
    src+='B--;'
    src+='WZ=(B<<8)|C;'
    src+='_OUT(WZ,d8);'
    if y & 1:
        src+='WZ--;hl--;'
    else:
        src+='WZ++; hl++;'
    src+='H=hl>>8;L=hl;'
    src+='F=(B?(B&Z80_SF):Z80_ZF)|(B&(Z80_XF|Z80_YF));'
    src+='if(d8&Z80_SF){F|=Z80_NF;}'
    src+='uint32_t t=(uint32_t)(L+d8);'
    src+='if (t&0x0100){F|=Z80_HF|Z80_CF;}'
    src+='F|=_z80_szp[((uint8_t)(t&0x07))^B]&Z80_PF;'
    if y >= 6:
        src+='if(B){'
        src+='PC-=2;'
        src+='_T(5);'
        src+='}'
    src+='}'
    return src

#-------------------------------------------------------------------------------
#   djnz()
#
def djnz():
    return ('{'
            '_T(1);'
            'int8_t d;_IMM8(d);'
            'B--;'
            'if(B>0){PC+=d;WZ=PC;_T(5);}'
            '}')

#-------------------------------------------------------------------------------
#   jr()
#
def jr():
    return '{int8_t d;_IMM8(d);PC+=d;WZ=PC;_T(5);}'

#-------------------------------------------------------------------------------
#   jr_cc()
#
def jr_cc(y):
    return ('{int8_t d;_IMM8(d);'
            'if('+cond[y-4]+'){PC+=d;WZ=PC;_T(5);}'
            '}')

#-------------------------------------------------------------------------------
#   ret()
#
def ret():
    return ('_MR(SP++,d8);PC=d8;'
            '_MR(SP++,d8);PC|=d8<<8;'
            'WZ=PC;')

#-------------------------------------------------------------------------------
#   ret_cc()
#
def ret_cc(y):
    return ('_T(1);'
            'if ('+cond[y]+'){'
            'uint8_t w,z;'
            '_MR(SP++,z);'
            '_MR(SP++,w);'
            'PC=(w<<8)|z;'
            'WZ=PC;'
            '}')

#-------------------------------------------------------------------------------
#   retin()
#
#   NOTE: according to Undocumented Z80 Documented, IFF2 is also 
#   copied into IFF1 in RETI, not just RETN, and RETI and RETN
#   are in fact identical!
#
def retin():
    # same as RET, but also set the virtual Z80_RETI pin
    return ('pins|=Z80_RETI;'
            '_MR(SP++,d8);PC=d8;'
            '_MR(SP++,d8);PC|=d8<<8;'
            'WZ=PC;'
            'IFF1=IFF2;')

#-------------------------------------------------------------------------------
#   rst()
#
def rst(y):
    return ('_T(1);'
            '_MW(--SP,PC>>8);'
            '_MW(--SP,PC);'
            'PC=WZ='+hex(y*8)+';')

#-------------------------------------------------------------------------------
#   in_r_ic
#   IN r,(C)
#
def in_r_ic(y):
    src ='{'
    src+='WZ=(B<<8)|C;'
    src+='_IN(WZ++,d8);'
    src+='F=(F&Z80_CF)|_z80_szp[d8];'
    # handle undocumented special case IN F,(C): 
    # only set flags, don't store result
    if (y != 6):
        src+=r[y]+'=d8;'
    src+='}'
    return src

#-------------------------------------------------------------------------------
#   out_r_ic()
#   OUT r,(C)
#
def out_r_ic(y):
    src ='WZ=(B<<8)|C;'
    if y == 6:
        src+='_OUT(WZ++,0);'
    else:
        src+='_OUT(WZ++,'+r[y]+');'
    return src

#-------------------------------------------------------------------------------
#   ALU functions.
#
def add8():
    return ('{'
            'uint32_t res=A+d8;'
            'F=_ADD_FLAGS(A,d8,res);'
            'A=res;'
            '}')

def adc8():
    return ('{'
            'uint32_t res=A+d8+(F&Z80_CF);'
            'F=_ADD_FLAGS(A,d8,res);'
            'A=res;'
            '}')

def sub8():
    return ('{'
            'uint32_t res=(uint32_t)((int)A-(int)d8);'
            'F=_SUB_FLAGS(A,d8,res);'
            'A=res;'
            '}')

def sbc8():
    return ('{'
            'uint32_t res=(uint32_t)((int)A-(int)d8-(F&Z80_CF));'
            'F=_SUB_FLAGS(A,d8,res);'
            'A=res;'
            '}')

def and8():
    return 'A&=d8;F=_z80_szp[A]|Z80_HF;'

def xor8():
    return 'A^=d8;F=_z80_szp[A];'

def or8():
    return 'A|=d8;F=_z80_szp[A];'

def cp8():
    return ('{'
            'int32_t res=(uint32_t)((int)A-(int)d8);'
            'F=_CP_FLAGS(A,d8,res);'
            '}')

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
    return 'd8=A;A=0;'+sub8()

def inc8():
    return ('{'
            'uint8_t r=d8+1;'
            'uint8_t f=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);'
            'if(r==0x80){f|=Z80_VF;}'
            'F=f|(F&Z80_CF);'
            'd8=r;'
            '}')

def dec8():
    return ('{'
            'uint8_t r=d8-1;'
            'uint8_t f=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);'
            'if(r==0x7F){f|=Z80_VF;}'
            'F=f|(F&Z80_CF);'
            'd8=r;'
            '}')

#-------------------------------------------------------------------------------
#   16-bit add,adc,sbc
#
#   flags computation taken from MAME
#
def add16(p):
    src ='{'
    src+='uint16_t acc=('+r[4]+'<<8)|'+r[5]+';'
    src+='WZ=acc+1;'
    if p==3:
        src+='d16=SP;'
    else:
        src+='d16=('+r[p*2+0]+'<<8)|'+r[p*2+1]+';'
    src+='uint32_t r=acc+d16;'
    src+=r[4]+'=r>>8;'+r[5]+'=r;'
    src+='uint8_t f=F&(Z80_SF|Z80_ZF|Z80_VF);'
    src+='f|=((acc^r^d16)>>8)&Z80_HF;'
    src+='f|=((r>>16)&Z80_CF)|((r>>8)&(Z80_YF|Z80_XF));'
    src+='F=f;'
    src+='_T(7);'
    src+='}'
    return src

def adc16(p):
    src ='{'
    src+='uint16_t acc=('+r[4]+'<<8)|'+r[5]+';'
    src+='WZ=acc+1;'
    if p==3:
        src+='d16=SP;'
    else:
        src+='d16=('+r[p*2+0]+'<<8)|'+r[p*2+1]+';'
    src+='uint32_t r=acc+d16+(F&Z80_CF);'
    src+=r[4]+'=r>>8;'+r[5]+'=r;'
    src+='uint8_t f=((d16^acc^0x8000)&(d16^r)&0x8000)>>13;'
    src+='f|=((acc^r^d16)>>8)&Z80_HF;'
    src+='f|=(r>>16)&Z80_CF;'
    src+='f|=(r>>8)&(Z80_SF|Z80_YF|Z80_XF);'
    src+='f|=(r&0xFFFF)?0:Z80_ZF;'
    src+='F=f;'
    src+='_T(7);'
    src+='}'
    return src

def sbc16(p):
    src ='{'
    src+='uint16_t acc=('+r[4]+'<<8)|'+r[5]+';'
    src+='WZ=acc+1;'
    if p==3:
        src+='d16=SP;'
    else:
        src+='d16=('+r[p*2+0]+'<<8)|'+r[p*2+1]+';'
    src+='uint32_t r=acc-d16-(F&Z80_CF);'
    src+='uint8_t f=Z80_NF|(((d16^acc)&(acc^r)&0x8000)>>13);'
    src+=r[4]+'=r>>8;'+r[5]+'=r;'
    src+='f|=((acc^r^d16)>>8) & Z80_HF;'
    src+='f|=(r>>16)&Z80_CF;'
    src+='f|=(r>>8)&(Z80_SF|Z80_YF|Z80_XF);'
    src+='f|=(r&0xFFFF)?0:Z80_ZF;'
    src+='F=f;'
    src+='_T(7);'
    src+='}'
    return src

#-------------------------------------------------------------------------------
#   rotate and shift functions
#
def rrd():
    return ('{'
            'WZ=(H<<8)|L;'
            '_MR(WZ,d8);'
            'uint8_t l=A&0x0F;'
            'A=(A&0xF0)|(d8&0x0F);'
            'd8=(d8>>4)|(l<<4);'
            '_MW(WZ++,d8);'
            'F=(F&Z80_CF)|_z80_szp[A];'
            '_T(4);'
            '}')

def rld():
    return ('{'
            'WZ=(H<<8)|L;'
            '_MR(WZ,d8);'
            'uint8_t l=A&0x0F;'
            'A=(A&0xF0)|(d8>>4);'
            'd8=(d8<<4)|l;'
            '_MW(WZ++,d8);'
            'F=(F&Z80_CF)|_z80_szp[A];'
            '_T(4);'
            '}')

def rlca():
    return ('{'
            'uint8_t r=(A<<1)|(A>>7);'
            'F=((A>>7)&Z80_CF)|(F&(Z80_SF|Z80_ZF|Z80_PF))|(r&(Z80_YF|Z80_XF));'
            'A=r;'
            '}')

def rrca():
    return ('{'
            'uint8_t r=(A>>1)|(A<<7);'
            'F=(A&Z80_CF)|(F&(Z80_SF|Z80_ZF|Z80_PF))|(r&(Z80_YF|Z80_XF));'
            'A=r;'
            '}')

def rla():
    return ('{'
            'uint8_t r=(A<<1)|(F&Z80_CF);'
            'F=((A>>7)&Z80_CF)|(F&(Z80_SF|Z80_ZF|Z80_PF))|(r&(Z80_YF|Z80_XF));'
            'A=r;'
            '}')

def rra():
    return ('{'
            'uint8_t r=(A>>1)|((F&Z80_CF)<<7);'
            'F=(A&Z80_CF)|(F&(Z80_SF|Z80_ZF|Z80_PF))|(r&(Z80_YF|Z80_XF));'
            'A=r;'
            '}')

#-------------------------------------------------------------------------------
#   misc ops
#
#   NOTE: during EI, interrupts are not accepted (for instance, during
#   a long sequence of EI, no interrupts will be handled, see 
#   Undocumented Z80 Documented)
#
def halt():
    return 'pins|=Z80_HALT;PC--;'

def di():
    return 'IFF1=IFF2=false;'

def ei():
    return 'IFF1=IFF2=false;EI_PENDING=true;'

def daa():
    return ('{'
            'uint8_t v=A;'
            'if(F & Z80_NF){'
              'if(((A&0x0F)>0x09)||(F&Z80_HF)){v-=0x06;}'
              'if((A>0x99)||(F&Z80_CF)){v-=0x60;}'
            '}else{'
              'if(((A&0x0F)>0x09)||(F&Z80_HF)){v+=0x06;}'
              'if((A>0x99)||(F&Z80_CF)){v+=0x60;}'
            '}'
            'F&=Z80_CF|Z80_NF;'
            'F|=(A>0x99)?Z80_CF:0;'
            'F|=(A^v)&Z80_HF;'
            'F|=_z80_szp[v];'
            'A=v;'
            '}')

def cpl():
    return 'A^=0xFF;F=(F&(Z80_SF|Z80_ZF|Z80_PF|Z80_CF))|Z80_HF|Z80_NF|(A&(Z80_YF|Z80_XF));'

def scf():
    return 'F=(F&(Z80_SF|Z80_ZF|Z80_PF|Z80_CF))|Z80_CF|(A&(Z80_YF|Z80_XF));'

def ccf():
    return 'F=((F&(Z80_SF|Z80_ZF|Z80_PF|Z80_CF))|((F&Z80_CF)<<4)|(A&(Z80_YF|Z80_XF)))^Z80_CF;'

#-------------------------------------------------------------------------------
# Encode a main instruction, or an DD or FD prefix instruction.
# Takes an opcode byte and returns an opcode object, for invalid instructions
# the opcode object will be in its default state (opcode.src==None).
# cc is the name of the cycle-count table.
#
def enc_op(ixy, op) :
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
                o.cmt = 'LD '+addr_cmt(ixy)+','+r[z]
                o.src = addr(ixy,5)+'_MW(addr,'+rx[z]+');'
        elif z == 6:
            # LD r,(HL); LD r,(IX+d); LD r,(IY+d)
            o.cmt = 'LD '+r[y]+','+addr_cmt(ixy)
            o.src = addr(ixy,5)+'_MR(addr,'+rx[y]+');'
        else:
            # LD r,s
            o.cmt = 'LD '+r[y]+','+r[z]
            if y != z:
                o.src = r[y]+'='+r[z]+';'
            else:
                o.src = ' ';

    #---- block 2: 8-bit ALU instructions (ADD, ADC, SUB, SBC, AND, XOR, OR, CP)
    elif x == 2:
        if z == 6:
            # ALU (HL); ALU (IX+d); ALU (IY+d)
            o.cmt = alu_cmt[y]+','+addr_cmt(ixy)
            o.src = addr(ixy,5) + '_MR(addr,d8);'+alu8(y)
        else:
            # ALU r
            o.cmt = alu_cmt[y]+' '+r[z]
            o.src = 'd8='+r[z]+';'+alu8(y)

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
                o.cmt = 'LD '+rpsp_cmt(p)+',nn'
                if p==3:
                    o.src = '_IMM16();SP=WZ;'
                else:
                    o.src = '_IMM16();'+r[p*2+0]+'=WZ>>8;'+r[p*2+1]+'=WZ;'
            else :
                # ADD HL,rr; ADD IX,rr; ADD IY,rr
                o.cmt = 'ADD '+rpsp_cmt(2)+','+rpsp_cmt(p)
                o.src = add16(p)
        elif z == 2:
            # indirect loads
            op_tbl = [
                [ 'LD (BC),A',              'WZ=(B<<8)|C;_MW(WZ++,A);WZ=(A<<8)|(WZ&0x00FF);' ],
                [ 'LD A,(BC)',              'WZ=(B<<8)|C;_MR(WZ++,A);' ],
                [ 'LD (DE),A',              'WZ=(D<<8)|E;_MW(WZ++,A);WZ=(A<<8)|(WZ&0x00FF);' ],
                [ 'LD A,(DE)',              'WZ=(D<<8)|E;_MR(WZ++,A);' ],
                [ 'LD (nn),'+rpsp_cmt(2),   '_IMM16();_MW(WZ++,'+r[2*2+1]+');_MW(WZ,'+r[2*2+0]+');' ],
                [ 'LD '+rpsp_cmt(2)+',(nn)','_IMM16();_MR(WZ++,'+r[2*2+1]+');_MR(WZ,'+r[2*2+0]+');' ],
                [ 'LD (nn),A',              '_IMM16();_MW(WZ++,A);WZ=(A<<8)|(WZ&0x00FF);' ],
                [ 'LD A,(nn)',              '_IMM16();_MR(WZ++,A);' ],
            ]
            o.cmt = op_tbl[y][0]
            o.src = op_tbl[y][1]
        elif z == 3:
            # 16-bit INC/DEC 
            if q == 0:
                o.cmt = 'INC '+rpsp_cmt(p)
                if p==3:
                    o.src = '_T(2);SP++;'
                else:
                    o.src = '_T(2);d16=('+r[p*2+0]+'<<8)|'+r[p*2+1]+';d16++;'+r[p*2+0]+'=d16>>8;'+r[p*2+1]+'=d16;'
            else:
                o.cmt = 'DEC '+rpsp_cmt(p)
                if p==3:
                    o.src = '_T(2);SP--;'
                else:
                    o.src = '_T(2);d16=('+r[p*2+0]+'<<8)|'+r[p*2+1]+';d16--;'+r[p*2+0]+'=d16>>8;'+r[p*2+1]+'=d16;'
        elif z == 4 or z == 5:
            cmt = 'INC' if z == 4 else 'DEC'
            fn = inc8() if z==4 else dec8()
            if y == 6:
                # INC/DEC (HL)/(IX+d)/(IY+d)
                o.cmt = cmt+' '+addr_cmt(ixy)
                o.src = addr(ixy,5)+'_T(1);_MR(addr,d8);'+fn+'_MW(addr,d8);'
            else:
                # INC/DEC r
                o.cmt = cmt+' '+r[y]
                o.src = 'd8='+r[y]+';'+fn+r[y]+'=d8;'
        elif z == 6:
            if y == 6:
                # LD (HL),n; LD (IX+d),n; LD (IY+d),n
                o.cmt = 'LD '+addr_cmt(ixy)+',n'
                o.src = addr(ixy,2) + '_IMM8(d8);_MW(addr,d8);'
            else:
                # LD r,n
                o.cmt = 'LD '+r[y]+',n'
                o.src = '_IMM8('+r[y]+');'
        elif z == 7:
            # misc ops on A and F
            op_tbl = [
                [ 'RLCA', rlca() ],
                [ 'RRCA', rrca() ],
                [ 'RLA',  rla() ],
                [ 'RRA',  rra() ],
                [ 'DAA',  daa() ],
                [ 'CPL',  cpl() ],
                [ 'SCF',  scf() ],
                [ 'CCF',  ccf() ],
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
                o.cmt = 'POP '+rpaf_cmt(p)
                o.src = pop_dd(p)
            else:
                # misc ops
                op_tbl = [
                    [ 'RET', ret() ],
                    [ 'EXX', exx() ],
                    [ 'JP '+rpsp_cmt(2), 'PC=('+r[2*2+0]+'<<8)|'+r[2*2+1]+';' ],
                    [ 'LD SP,'+rpaf_cmt(2), '_T(2);SP=('+r[2*2+0]+'<<8)|'+r[2*2+1]+';' ]
                ]
                o.cmt = op_tbl[p][0]
                o.src = op_tbl[p][1]
        if z == 2:
            # JP cc,nn
            o.cmt = 'JP {},nn'.format(cond_cmt[y])
            o.src = '_IMM16();if('+cond[y]+'){PC=WZ;}'
        if z == 3:
            # misc ops
            op_tbl = [
                [ 'JP nn', '_IMM16();PC=WZ;' ],
                [ None, None ], # CB prefix instructions
                [ 'OUT (n),A', out_n_a() ],
                [ 'IN A,(n)', in_n_a() ],
                [ 'EX (SP),'+rpaf_cmt(2), ex_sp_dd() ],
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
                o.cmt = 'PUSH '+rpaf_cmt(p)
                o.src = push_dd(p)
            else:
                op_tbl = [
                    [ 'CALL nn', call_nn() ],
                    [ 'DD prefix', '/*FIXME!*/' ],
                    [ None, None ], # ED prefix instructions
                    [ 'FD prefix', '/*FIXME!*/'],
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
                o.cmt = 'SBC HL,'+rpsp_cmt(p)
                o.src = sbc16(p)
            else:
                o.cmt = 'ADC HL,'+rpsp_cmt(p)
                o.src = adc16(p)
        if z == 3:
            # 16-bit immediate address load/store
            if q == 0:
                o.cmt = 'LD (nn),'+rpsp_cmt(p)
                o.src = ld_inn_dd(p)
            else:
                o.cmt = 'LD '+rpsp_cmt(p)+',(nn)'
                o.src = ld_dd_inn(p)
        if z == 4:
            # NEG
            o.cmt = 'NEG'
            o.src = neg8()
        if z == 5:
            # RETN, RETI (both are identical according to Undocumented Z80 Documented)
            if y == 1:
                o.cmt = 'RETI'
            else:
                o.cmt = 'RETN'
            o.src = retin()
        if z == 6:
            # IM m
            im_mode = [ '0', '0', '1', '2', '0', '0', '1', '2' ]
            o.cmt = 'IM {}'.format(im_mode[y])
            o.src = 'IM='+im_mode[y]+';'
        if z == 7:
            # misc ops on I,R and A
            op_tbl = [
                [ 'LD I,A', '_T(1);I=A;' ],
                [ 'LD R,A', '_T(1);R=A;' ],
                [ 'LD A,I', '_T(1);A=I;F=_SZIFF2_FLAGS(A);' ],
                [ 'LD A,R', '_T(1);A=R;F=_SZIFF2_FLAGS(A);' ],
                [ 'RRD', rrd() ],
                [ 'RLD', rld() ],
                [ 'NOP (ED)', ' ' ],
                [ 'NOP (ED)', ' ' ],
            ]
            o.cmt = op_tbl[y][0]
            o.src = op_tbl[y][1]
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
        write_cb_ops(None)
    # DD/FD prefixes
    elif i == 0xDD or i == 0xFD:
        write_ixy_ops(i)
    # non-prefixed instruction
    else:
        write_op(enc_op(None,i))
indent = 0
write_op_post()
write_interrupt_handling()
write_footer()
Out.close()
