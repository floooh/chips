#-------------------------------------------------------------------------------
#   z80_gen.py
#   Generate huge switch/case Z80 instruction decoder.
#   See: 
#       http://www.z80.info/decoding.htm
#       http://www.righto.com/2014/10/how-z80s-registers-are-implemented-down.html
#       http://www.z80.info/zip/z80-documented.pdf
#       https://www.omnimaga.org/asm-language/bit-n-(hl)-flags/5/?wap2
#-------------------------------------------------------------------------------
import sys
from string import Template

TabWidth = 4
InpPath = 'z80.template.h'
OutPath = '../chips/z80.h'

# 8-bit register table, the 'mem' entry is for instructions that use
# (HL), (IX+d) and (IY+d)
r = [ 'c.b', 'c.c', 'c.d', 'c.e', 'c.ih', 'c.il', 'memptr', 'c.a' ]

# the same for instructions that are hardwired to H,L
_r = [ 'c.b', 'c.c', 'c.d', 'c.e', 'c.h', 'c.l', 'memptr', 'c.a' ]

# the same as human-readable regs
r_cmt = [ 'B', 'C', 'D', 'E', 'H', 'L', '(HL/IX+d/IY+d)', 'A' ]
_r_cmt = [ 'B', 'C', 'D', 'E', 'H', 'L', 'F', 'A' ]

# 16-bit register table, with SP
rp = [ 'BC', 'DE', 'HL', 'SP' ]
rp_cmt = [ 'BC', 'DE', 'HL/IX/IY', 'SP' ]

# 16-bit register table, with AF (only used for PUSH/POP)
rp2_cmt = [ 'BC', 'DE', 'HL/IX/IY', 'SP' ]

# condition-code table (for conditional jumps etc)
cond = [
    '!(c.f&Z80_ZF)',  # NZ
    '(c.f&Z80_ZF)',   # Z
    '!(c.f&Z80_CF)',   # NC
    '(c.f&Z80_CF)',    # C
    '!(c.f&Z80_PF)',   # PO
    '(c.f&Z80_PF)',    # PE
    '!(c.f&Z80_SF)',   # P
    '(c.f&Z80_SF)'     # M
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
out_lines = ''

def inc_indent():
    global indent
    indent += 1

def dec_indent():
    global indent
    indent -= 1

def tab() :
    return ' '*TabWidth*indent

def l(s) :
    global out_lines
    out_lines += tab() + s + '\n'

#-------------------------------------------------------------------------------
# Return code to setup an address variable 'a' with the address of HL
# or (IX+d), (IY+d). For the index instructions also update WZ with
# IX+d or IY+d
#
def addr(ext_ticks) :
    return '_ADDR(addr,'+str(ext_ticks)+');'

#-------------------------------------------------------------------------------
# Return code to load and store 16-bit value from/to register pair, using SP for rp=3.
#
def rp_l(rp, val):
    if rp == 0:
        return val+'=(c.b<<8)|c.c;'
    elif rp == 1:
        return val+'=(c.d<<8)|c.e;'
    elif rp == 2:
        return val+'=(c.ih<<8)|c.il;'
    elif rp == 3:
        return val+'=c.sp;'

def rp_s(rp, val):
    if rp == 0:
        return 'c.b='+val+'>>8;c.c='+val+';'
    elif rp == 1:
        return 'c.d='+val+'>>8;c.e='+val+';'
    elif rp == 2:
        return 'c.ih='+val+'>>8;c.il='+val+';'
    elif rp == 3:
        return 'c.sp='+val+';'

#-------------------------------------------------------------------------------
# Return code to load and store 16-bit value from/to register pair, using AF for rp=3.
#
def rp2_l(rp, val):
    if rp == 0:
        return val+'=(c.b<<8)|c.c;'
    elif rp == 1:
        return val+'=(c.d<<8)|c.e;'
    elif rp == 2:
        return val+'=(c.ih<<8)|c.il;'
    elif rp == 3:
        return val+'=(c.f<<8)|c.a;'

def rp2_s(rp, val):
    if rp == 0:
        return 'c.b='+val+'>>8;c.c='+val+';'
    elif rp == 1:
        return 'c.d='+val+'>>8;c.e='+val+';'
    elif rp == 2:
        return 'c.ih='+val+'>>8;c.il='+val+';'
    elif rp == 3:
        return 'c.f='+val+'>>8;c.a='+val+';'

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
    l('    if ((z == 6) || _IDX()) {')
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
    l('  if ((z == 6) || _IDX()) {')
    l('    /* (HL), (IX+d), (IY+d): write back to memory, for extended ops,')
    l('       even when the op is actually a register op')
    l('    */')
    l('    _MW(addr,r);')
    l('  }')
    l('  if (z != 6) {')
    l('    /* write result back to register (special case for indexed + H/L! */')
    l('    if (_IDX() && ((z==4)||(z==5))) {')
    l('      _S8(r0,rz,r);')
    l('    }')
    l('    else {')
    l('      _S8(ws,rz,r);')
    l('    }')
    l('  }')
    l('}')
    l('_S_F(f);')
    dec_indent()
    l('}')
    l('break;')

#-------------------------------------------------------------------------------
#   out_n_a
#
#   Generate code for OUT (n),A
#
def out_n_a():
    src = '{'
    src += '_IMM8(d8);'
    src += 'addr=(c.a<<8)|d8;'
    src += '_OUT(addr,c.a);'
    src += 'c.wz=(addr&0xFF00)|((addr+1)&0x00FF);'
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
    src += 'addr=(c.a<<8)|d8;'
    src += '_IN(addr++,c.a);'
    src += 'c.wz=addr;'
    src += '}'
    return src

#-------------------------------------------------------------------------------
#   ex_af
#
#   Generate code for EX AF,AF'
#
def ex_af():
    src ='{'
    src+='uint16_t fa=(c.f<<8)|c.a;'
    src+='c.f=c.fa_>>8;'
    src+='c.a=c.fa_;'
    src+='c.fa_=fa;'
    src+='}'
    return src

#-------------------------------------------------------------------------------
#   ex_de_hl
#
#   Generate code for EX DE,HL
#
def ex_de_hl():
    src ='{'
    src+='_z80_flush_ihl(&c,c.bits);'
    src+='c.e^=c.l;c.l^=c.e;c.e^=c.l;'
    src+='c.d^=c.h;c.h^=c.d;c.d^=c.h;'
    src+='_z80_load_ihl(&c,c.bits);'
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
    src+='addr=c.sp;'
    src+='uint8_t l,h;'
    src+='_MR(addr,l);'
    src+='_MR(addr+1,h);'
    src+='_MW(addr,c.il);'
    src+='_MW(addr+1,c.ih);'
    src+='c.ih=h;c.il=l;c.wz=(h<<8)|l;'
    src+='}'
    return src

#-------------------------------------------------------------------------------
#   exx
#
#   Generate code for EXX
#
def exx():
    src ='{'
    src+='_z80_flush_ihl(&c,c.bits);'
    src+='uint16_t bc=(c.b<<8)|c.c;'
    src+='uint16_t de=(c.d<<8)|c.e;'
    src+='uint16_t hl=(c.h<<8)|c.l;'
    src+='c.b=c.bc_>>8;c.c=c.bc_;'
    src+='c.d=c.de_>>8;c.e=c.de_;'
    src+='c.h=c.hl_>>8;c.l=c.hl_;'
    src+='c.bc_=bc;c.de_=de;c.hl_=hl;'
    src+='_z80_load_ihl(&c,c.bits);'
    src+='}'
    return src

#-------------------------------------------------------------------------------
#   pop_dd
#
#   Generate code for POP dd.
#
def pop_dd(p):
    reg = [ ['c.c','c.b'], ['c.e','c.d'], ['c.il','c.ih'], ['c.f','c.a'] ]
    src ='_MR(c.sp++,'+reg[p][0]+');'
    src+='_MR(c.sp++,'+reg[p][1]+');'
    return src

#-------------------------------------------------------------------------------
#   push_dd
#
#   Generate code for PUSH dd
#
def push_dd(p):
    reg = [ ['c.c','c.b'], ['c.e','c.d'], ['c.il','c.ih'], ['c.f','c.a'] ]
    src ='_T(1);'
    src+='_MW(--c.sp,'+reg[p][1]+');'
    src+='_MW(--c.sp,'+reg[p][0]+');'
    return src

#-------------------------------------------------------------------------------
#   ld_inn_dd
#   LD (nn),dd
#
def ld_inn_dd(p):
    src  = '_IMM16(addr);'
    src += rp_l(p,'d16')
    src += '_MW(addr++,d16&0xFF);'
    src += '_MW(addr,d16>>8);'
    src += 'c.wz=addr;'
    return src

#-------------------------------------------------------------------------------
#   ld_dd_inn
#   LD dd,(nn)
#
def ld_dd_inn(p):
    src  = '_IMM16(addr);'
    src += '_MR(addr++,d8);d16=d8;'
    src += '_MR(addr,d8);d16|=d8<<8;'
    src += rp_s(p,'d16')
    src += 'c.wz=addr;'
    return src

#-------------------------------------------------------------------------------
#   call_nn
#
#   Generate code for CALL nn
#
def call_nn():
    src ='_IMM16(addr);'
    src+='_T(1);'
    src+='_MW(--c.sp,c.pc>>8);'
    src+='_MW(--c.sp,c.pc);'
    src+='c.pc=addr;'
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
    src+='_MW(--c.sp,c.pc>>8);'
    src+='_MW(--c.sp,c.pc);'
    src+='c.pc=addr;'
    src+='}'
    return src

#-------------------------------------------------------------------------------
#   ldi_ldd_ldir_lddr()
#
#   Generate code for LDI, LDIR, LDD, LDDR
#
def ldi_ldd_ldir_lddr(y):
    src ='{'
    src+='uint16_t hl=(c.ih<<8)|c.il;'
    src+='uint16_t de=(c.d<<8)|c.e;'
    src+='_MR(hl,d8);'
    src+='_MW(de,d8);'
    if y & 1:
        src+='hl--;de--;'
    else:
        src+='hl++;de++;'
    src+='c.ih=hl>>8;c.il=hl;'
    src+='c.d=de>>8;c.e=de;'
    src+='_T(2);'
    src+='d8+=c.a;'
    src+='c.f &=(Z80_SF|Z80_ZF|Z80_CF);'
    src+='if(d8&0x02){c.f|=Z80_YF;}'
    src+='if(d8&0x08){c.f|=Z80_XF;}'
    src+='uint16_t bc=(c.b<<8)|c.c;'
    src+='bc--;'
    src+='c.b=bc>>8;c.c=bc;'
    src+='if(bc){c.f|=Z80_VF;}'
    if y >= 6:
        src+='if(bc){'
        src+='c.pc-=2;'
        src+='c.wz=c.pc+1;'
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
    src+='uint16_t hl = (c.ih<<8)|c.il;'
    src+='_MR(hl,d8);'
    if y & 1:
        src+='hl--;c.wz--;'
    else:
        src+='hl++;c.wz++;'
    src+='c.ih=hl>>8;c.il=hl;'
    src+='_T(5);'
    src+='int r=(int)c.a-d8;'
    src+='c.f=(c.f&Z80_CF)|Z80_NF|_SZ(r);'
    src+='if((r&0x0F)>(c.a&0x0F)){'
    src+='c.f|=Z80_HF;'
    src+='r--;'
    src+='}'
    src+='if(r&0x02){c.f|=Z80_YF;}'
    src+='if(r&0x08){c.f|=Z80_XF;}'
    src+='uint16_t bc=(c.b<<8)|c.c;'
    src+='bc--;'
    src+='c.b=bc>>8;c.c=bc;'
    src+='if(bc){c.f|=Z80_VF;}'
    if y >= 6:
        src+='if(bc&&!(c.f&Z80_ZF)){'
        src+='c.pc-=2;'
        src+='c.wz=c.pc+1;'
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
    src+='addr=(c.b<<8)|c.c;'
    src+='uint16_t hl=(c.ih<<8)|c.il;'
    src+='_IN(addr,d8);'
    src+='_MW(hl,d8);'
    src+='uint8_t rc=c.c;'
    src+='c.b--;'
    if y & 1:
        src+='addr--;hl--;rc--;'
    else:
        src+='addr++;hl++;rc++;'
    src+='c.ih=hl>>8; c.il=hl;'
    src+='c.wz=addr;'
    src+='c.f=(c.b?(c.b&Z80_SF):Z80_ZF)|(c.b&(Z80_XF|Z80_YF));'
    src+='if(d8&Z80_SF){c.f|=Z80_NF;}'
    src+='uint32_t t=(uint32_t)(rc&0xFF)+d8;'
    src+='if(t&0x100){c.f|=Z80_HF|Z80_CF;}'
    src+='c.f|=_z80_szp[((uint8_t)(t&0x07))^c.b]&Z80_PF;'
    if y >= 6:
        src+='if(c.b){'
        src+='c.pc-=2;'
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
    src+='uint16_t hl=(c.ih<<8)|c.il;'
    src+='_MR(hl,d8);'
    src+='c.b--;'
    src+='addr=(c.b<<8)|c.c;'
    src+='_OUT(addr,d8);'
    if y & 1:
        src+='addr--;hl--;'
    else:
        src+='addr++; hl++;'
    src+='c.ih=hl>>8;c.il=hl;'
    src+='c.wz=addr;'
    src+='c.f=(c.b?(c.b&Z80_SF):Z80_ZF)|(c.b&(Z80_XF|Z80_YF));'
    src+='if(d8&Z80_SF){c.f|=Z80_NF;}'
    src+='uint32_t t=(uint32_t)c.il+(uint32_t)d8;'
    src+='if (t&0x0100){c.f|=Z80_HF|Z80_CF;}'
    src+='c.f|=_z80_szp[((uint8_t)(t&0x07))^c.b]&Z80_PF;'
    if y >= 6:
        src+='if(c.b){'
        src+='c.pc-=2;'
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
    src+='if(--c.b>0){c.pc+=d;c.wz=c.pc;_T(5);}'
    src+='}'
    return src

#-------------------------------------------------------------------------------
#   jr()
#
def jr():
    return '{int8_t d;_IMM8(d);c.pc+=d;c.wz=c.pc;_T(5);}'

#-------------------------------------------------------------------------------
#   jr_cc()
#
def jr_cc(y):
    src ='{int8_t d;_IMM8(d);'
    src+='if('+cond[y-4]+'){c.pc+=d;c.wz=c.pc;_T(5);}'
    src+='}'
    return src

#-------------------------------------------------------------------------------
#   ret()
#
def ret():
    src  = '_MR(c.sp++,d8);c.pc=d8;'
    src += '_MR(c.sp++,d8);c.pc|=d8<<8;'
    src += 'c.wz=c.pc;'
    return src

#-------------------------------------------------------------------------------
#   ret_cc()
#
def ret_cc(y):
    src ='_T(1);'
    src+='if ('+cond[y]+'){'
    src+='uint8_t w,z;'
    src+='_MR(c.sp++,z);'
    src+='_MR(c.sp++,w);'
    src+='c.pc=c.wz=(w<<8)|z;'
    src+='}'
    return src

#-------------------------------------------------------------------------------
#   retin()
#
#   NOTE: according to Undocumented Z80 Documented, IFF2 is also 
#   copied into IFF1 in RETI, not just RETN, and RETI and RETN
#   are in fact identical!
#
def retin():
    # same as RET, but also set the virtual Z80_RETI pin
    src  = 'pins|=Z80_RETI;'
    src += '_MR(c.sp++,d8);c.pc=d8;'
    src += '_MR(c.sp++,d8);c.pc|=d8<<8;'
    src += 'c.wz=c.pc;'
    src += 'if(c.bits&Z80_BIT_IFF2){c.bits|=Z80_BIT_IFF1;}else{c.bits&=~Z80_BIT_IFF1;}'
    return src

#-------------------------------------------------------------------------------
#   rst()
#
def rst(y):
    src ='_T(1);'
    src+='_MW(--c.sp,c.pc>>8);'
    src+='_MW(--c.sp,c.pc);'
    src+='c.pc=c.wz='+hex(y*8)+';'
    return src

#-------------------------------------------------------------------------------
#   in_r_ic
#   IN r,(C)
#
def in_r_ic(y):
    src ='{'
    src+='addr=(c.b<<8)|c.c;'
    src+='_IN(addr++,d8);'
    src+='c.wz=addr;'
    src+='c.f=(c.f&Z80_CF)|_z80_szp[d8];'
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
    src ='addr=(c.b<<8)|c.c;'
    if y == 6:
        src+='_OUT(addr++,0);'
    else:
        src+='_OUT(addr++,'+r[y]+');'
    src+='c.wz=addr;'
    return src

#-------------------------------------------------------------------------------
#   ALU functions.
#
def add8(val):
    src ='{'
    src+='uint32_t res=c.a+'+val+';'
    src+='c.f=_ADD_FLAGS(c.a,'+val+',res);'
    src+='c.a=res;'
    src+='}'
    return src

def adc8(val):
    src ='{'
    src+='uint32_t res=c.a+'+val+'+(c.f&Z80_CF);'
    src+='c.f=_ADD_FLAGS(c.a,'+val+',res);'
    src+='c.a=res;'
    src+='}'
    return src

def sub8(val):
    src ='{'
    src+='uint32_t res=(uint32_t)((int)c.a-(int)'+val+');'
    src+='c.f=_SUB_FLAGS(c.a,'+val+',res);'
    src+='c.a=res;'
    src+='}'
    return src

def sbc8(val):
    src ='{'
    src+='uint32_t res=(uint32_t)((int)c.a-(int)'+val+'-(c.f&Z80_CF));'
    src+='c.f=_SUB_FLAGS(c.a,'+val+',res);'
    src+='c.a=res;'
    src+='}'
    return src

def and8(val):
    src ='{'
    src+='c.a&='+val+';'
    src+='c.f=_z80_szp[c.a]|Z80_HF;'
    src+='}'
    return src

def xor8(val):
    src ='{'
    src+='c.a^='+val+';'
    src+='c.f=_z80_szp[c.a];'
    src+='}'
    return src

def or8(val):
    src ='{'
    src+='c.a|='+val+';'
    src+='c.f=_z80_szp[c.a];'
    src+='}'
    return src

def cp8(val):
    src ='{'
    src+='int32_t res=(uint32_t)((int)c.a-(int)'+val+');'
    src+='c.f=_CP_FLAGS(c.a,'+val+',res);'
    src+='}'
    return src

def alu8(y, val):
    if (y==0):
        return add8(val)
    elif (y==1):
        return adc8(val)
    elif (y==2):
        return sub8(val)
    elif (y==3):
        return sbc8(val)
    elif (y==4):
        return and8(val)
    elif (y==5):
        return xor8(val)
    elif (y==6):
        return or8(val)
    elif (y==7):
        return cp8(val)

def neg8():
    src ='d8=c.a;'
    src+='c.a=0;'
    src+=sub8('d8')
    return src

def inc8():
    src ='{'
    src+='uint8_t r=d8+1;'
    src+='c.f&=Z80_CF;'
    src+='c.f|=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);'
    src+='if(r==0x80){c.f|=Z80_VF;}'
    src+='d8=r;'
    src+='}'
    return src

def dec8():
    src ='{'
    src+='uint8_t r=d8-1;'
    src+='c.f&=Z80_CF;'
    src+='c.f|=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);'
    src+='if(r==0x7F){c.f|=Z80_VF;}'
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
    src+='uint16_t acc=(c.ih<<8)|c.il;'
    src+='c.wz=acc+1;'
    src+=rp_l(p,'d16')
    src+='uint32_t r=acc+d16;'
    src+='c.ih=acc>>8;c.il=acc;'
    src+='c.f&=(Z80_SF|Z80_ZF|Z80_VF);'
    src+='c.f|=((acc^r^d16)>>8)&Z80_HF;'
    src+='c.f|=((r>>16)&Z80_CF)|((r>>8)&(Z80_YF|Z80_XF));'
    src+='_T(7);'
    src+='}'
    return src

def adc16(p):
    src ='{'
    src+='uint16_t acc=(c.ih<<8)|c.il;'
    src+='c.wz=acc+1;'
    src+=rp_l(p,'d16')
    src+='uint32_t r=acc+d16+(c.f&Z80_CF);'
    src+='c.ih=acc>>8;c.il=acc;'
    src+='c.f=((d16^acc^0x8000)&(d16^r)&0x8000)>>13;'
    src+='c.f|=((acc^r^d16)>>8)&Z80_HF;'
    src+='c.f|=(r>>16)&Z80_CF;'
    src+='c.f|=(r>>8)&(Z80_SF|Z80_YF|Z80_XF);'
    src+='c.f|=(r&0xFFFF)?0:Z80_ZF;'
    src+='_T(7);'
    src+='}'
    return src

def sbc16(p):
    src ='{'
    src+='uint16_t acc=(c.ih<<8)|c.il;'
    src+='c.wz=acc+1;'
    src+=rp_l(p,'d16')
    src+='uint32_t r=acc-d16-(c.f&Z80_CF);'
    src+='c.ih=acc>>8;c.il=acc;'
    src+='c.f=Z80_NF|(((d16^acc)&(acc^r)&0x8000)>>13);'
    src+='c.f|=((acc^r^d16)>>8) & Z80_HF;'
    src+='c.f|=(r>>16)&Z80_CF;'
    src+='c.f|=(r>>8)&(Z80_SF|Z80_YF|Z80_XF);'
    src+='c.f|=(r&0xFFFF)?0:Z80_ZF;'
    src+='_T(7);'
    src+='}'
    return src

#-------------------------------------------------------------------------------
#   rotate and shift functions
#
def rrd():
    src ='{'
    src+='addr=(c.ih<<8)|c.il;'
    src+='_MR(addr,d8);'
    src+='uint8_t l=c.a&0x0F;'
    src+='c.a=(c.a&0xF0)|(d8&0x0F);'
    src+='d8=(d8>>4)|(l<<4);'
    src+='_MW(addr++,d8);'
    src+='c.wz=addr;'
    src+='c.f=(c.f&Z80_CF)|_z80_szp[c.a];'
    src+='_T(4);'
    src+='}'
    return src

def rld():
    src ='{'
    src+='addr=(c.ih<<8)|c.il;'
    src+='_MR(addr,d8);'
    src+='uint8_t l=c.a&0x0F;'
    src+='c.a=(c.a&0xF0)|(d8>>4);'
    src+='d8=(d8<<4)|l;'
    src+='_MW(addr++,d8);'
    src+='c.wz=addr;'
    src+='c.f=(c.f&Z80_CF)|_z80_szp[c.a];'
    src+='_T(4);'
    src+='}'
    return src

#-------------------------------------------------------------------------------
#   misc ops
#
#   NOTE: during EI, interrupts are not accepted (for instance, during
#   a long sequence of EI, no interrupts will be handled, see 
#   Undocumented Z80 Documented)
#
def halt():
    return 'pins|=Z80_HALT;c.pc--;'

def di():
    return 'c.bits&=~(Z80_BIT_IFF1|Z80_BIT_IFF2);'

def ei():
    return 'c.bits=(c.bits&~(Z80_BIT_IFF1|Z80_BIT_IFF2))|Z80_BIT_EI;'

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
                o.cmt = 'LD '+r_cmt[6]+','+r_cmt[z]
                # special case LD (IX+d),L LD (IX+d),H
                if z in [4,5]:
                    o.src = addr(5)+'if(_IDX()){_MW(addr,'+_r[z]+');}else{_MW(addr,'+r[z]+');}'
                else:
                    o.src = addr(5)+'_MW(addr,'+r[z]+');'
        elif z == 6:
            # LD r,(HL); LD r,(IX+d); LD r,(IY+d)
            o.cmt = 'LD '+r_cmt[y]+','+r_cmt[6]
            if y in [4,5]:
                o.src = addr(5)+'if(_IDX()){_MR(addr,'+_r[y]+');}else{_MR(addr,'+r[y]+');}'
            else:
                o.src = addr(5)+'_MR(addr,'+r[y]+');'
        else:
            # LD r,s
            o.cmt = 'LD '+r_cmt[y]+','+r_cmt[z]
            o.src = r[y]+'='+r[z]+';'

    #---- block 2: 8-bit ALU instructions (ADD, ADC, SUB, SBC, AND, XOR, OR, CP)
    elif x == 2:
        if z == 6:
            # ALU (HL); ALU (IX+d); ALU (IY+d)
            o.cmt = alu_cmt[y]+','+r_cmt[6]
            o.src = addr(5) + '_MR(addr,d8);'+alu8(y,'d8')
        else:
            # ALU r
            o.cmt = alu_cmt[y]+' '+r_cmt[z]
            o.src = alu8(y,r[z])

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
                o.cmt = 'LD '+rp_cmt[p]+',nn'
                o.src = '_IMM16(d16);'+rp_s(p, 'd16')
            else :
                # ADD HL,rr; ADD IX,rr; ADD IY,rr
                o.cmt = 'ADD '+rp_cmt[2]+','+rp_cmt[p]
                o.src = add16(p)
        elif z == 2:
            # indirect loads
            op_tbl = [
                [ 'LD (BC),A',          'addr=(c.b<<8)|c.c;_MW(addr++,c.a);c.wz=(c.a<<8)|(addr&0x00FF);' ],
                [ 'LD A,(BC)',          'addr=(c.b<<8)|c.c;_MR(addr++,c.a);c.wz=addr;' ],
                [ 'LD (DE),A',          'addr=(c.d<<8)|c.e;_MW(addr++,c.a);c.wz=(c.a<<8)|(addr&0x00FF);' ],
                [ 'LD A,(DE)',          'addr=(c.d<<8)|c.e;_MR(addr++,c.a);c.wz=addr;' ],
                [ 'LD (nn),'+rp_cmt[2],     '_IMM16(addr);_MW(addr++,c.il);_MW(addr,c.ih);c.wz=addr;' ],
                [ 'LD '+rp_cmt[2]+',(nn)',  '_IMM16(addr);_MR(addr++,c.il);_MR(addr,c.ih);c.wz=addr;' ],
                [ 'LD (nn),A',          '_IMM16(addr);_MW(addr++,c.a);c.wz=(c.a<<8)|(addr&0x00FF);' ],
                [ 'LD A,(nn)',          '_IMM16(addr);_MR(addr++,c.a);c.wz=addr;' ],
            ]
            o.cmt = op_tbl[y][0]
            o.src = op_tbl[y][1]
        elif z == 3:
            # 16-bit INC/DEC 
            if q == 0:
                o.cmt = 'INC '+rp_cmt[p]
                o.src = '_T(2);'+rp_l(p,'d16')+'d16++;'+rp_s(p,'d16')
            else:
                o.cmt = 'DEC '+rp_cmt[p]
                o.src = '_T(2);'+rp_l(p,'d16')+'d16--;'+rp_s(p,'d16')
        elif z == 4 or z == 5:
            cmt = 'INC' if z == 4 else 'DEC'
            fn = inc8() if z==4 else dec8()
            if y == 6:
                # INC/DEC (HL)/(IX+d)/(IY+d)
                o.cmt = cmt+' (HL/IX+d/IY+d)'
                o.src = addr(5)+'_T(1);_MR(addr,d8);'+fn+'_MW(addr,d8);'
            else:
                # INC/DEC r
                o.cmt = cmt+' '+r_cmt[y]
                o.src = 'd8='+r[y]+';'+fn+r[y]+'=d8;'
        elif z == 6:
            if y == 6:
                # LD (HL),n; LD (IX+d),n; LD (IY+d),n
                o.cmt = 'LD (HL/IX+d/IY+d),n'
                o.src = addr(2) + '_IMM8(d8);_MW(addr,d8);'
            else:
                # LD r,n
                o.cmt = 'LD '+r_cmt[y]+',n'
                o.src = '_IMM8('+r[y]+');'
        elif z == 7:
            # misc ops on A and F
            op_tbl = [
                [ 'RLCA', '_z80_rlca(&c);' ],
                [ 'RRCA', '_z80_rrca(&c);' ],
                [ 'RLA',  '_z80_rla(&c);' ],
                [ 'RRA',  '_z80_rra(&c);' ],
                [ 'DAA',  '_z80_daa(&c);' ],
                [ 'CPL',  '_z80_cpl(&c);' ],
                [ 'SCF',  '_z80_scf(&c);' ],
                [ 'CCF',  '_z80_ccf(&c);' ]
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
                o.cmt = 'POP '+rp2_cmt[p]
                o.src = pop_dd(p)
            else:
                # misc ops
                op_tbl = [
                    [ 'RET', ret() ],
                    [ 'EXX', exx() ],
                    [ 'JP '+rp[2], 'c.pc=(c.ih<<8)|c.il;' ],
                    [ 'LD SP,'+rp[2], '_T(2);c.sp=(c.ih<<8)|c.il;' ]
                ]
                o.cmt = op_tbl[p][0]
                o.src = op_tbl[p][1]
        if z == 2:
            # JP cc,nn
            o.cmt = 'JP {},nn'.format(cond_cmt[y])
            o.src = '_IMM16(addr);if('+cond[y]+'){c.pc=addr;}'
        if z == 3:
            # misc ops
            op_tbl = [
                [ 'JP nn', '_IMM16(c.pc);' ],
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
                o.cmt = 'PUSH {}'.format(rp2_cmt[p])
                o.src = push_dd(p)
            else:
                op_tbl = [
                    [ 'CALL nn', call_nn() ],
                    [ 'DD prefix', 'map_bits|=Z80_BIT_IX;continue;' ],
                    [ None, None ], # ED prefix instructions
                    [ 'FD prefix', 'map_bits|=Z80_BIT_IY;continue;'],
                ]
                o.cmt = op_tbl[p][0]
                o.src = op_tbl[p][1]
        if z == 6:
            # ALU n
            o.cmt = '{} n'.format(alu_cmt[y])
            o.src = '_IMM8(d8);'+alu8(y,'d8')
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
            o.cmt = 'IN {},(C)'.format(_r_cmt[y])
            o.src = in_r_ic(y)
        if z == 1:
            # OUT (C),r
            o.cmt = 'OUT (C),{}'.format(_r_cmt[y])
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
                o.cmt = 'LD (nn),{}'.format(rp_cmt[p])
                o.src = ld_inn_dd(p)
            else:
                o.cmt = 'LD {},(nn)'.format(rp_cmt[p])
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
            im_mode = [ 0, 0, 1, 2, 0, 0, 1, 2 ]
            o.cmt = 'IM {}'.format(im_mode[y])
            o.src = 'c.im={};'.format(im_mode[y])
        if z == 7:
            # misc ops on I,R and A
            op_tbl = [
                [ 'LD I,A', '_T(1);c.i=c.a;' ],
                [ 'LD R,A', '_T(1);c.r=c.a;' ],
                [ 'LD A,I', '_T(1);c.a=c.i;c.f=_SZIFF2_FLAGS(c.a);' ],
                [ 'LD A,R', '_T(1);c.a=c.r;c.f=_SZIFF2_FLAGS(c.a);' ],
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
indent = 3
for i in range(0, 256):
    # ED prefix instructions
    if i == 0xED:
        write_ed_ops()
    # CB prefix instructions
#    elif i == 0xCB:
#        write_cb_ops()
    # non-prefixed instruction
    else:
        write_op(enc_op(i))
indent = 0

with open(InpPath, 'r') as inf:
    templ = Template(inf.read())
    c_src = templ.safe_substitute(decode_block=out_lines)
    with open(OutPath, 'w') as outf:
        outf.write(c_src)
