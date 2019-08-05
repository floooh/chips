#-------------------------------------------------------------------------------
#   m6502_decoder.py
#   Generate instruction decoder for m6502.h emulator.
#-------------------------------------------------------------------------------
import sys
from string import Template

InpPath = 'm6502.template.h'
OutPath = '../chips/m6502.h'

# flag bits
CF = (1<<0)
ZF = (1<<1)
IF = (1<<2)
DF = (1<<3)
BF = (1<<4)
XF = (1<<5)
VF = (1<<6)
NF = (1<<7)

def flag_name(f):
    if f == CF: return 'C'
    elif f == ZF: return 'Z'
    elif f == IF: return 'I'
    elif f == DF: return 'D'
    elif f == BF: return 'B'
    elif f == XF: return 'X'
    elif f == VF: return 'V'
    elif f == NF: return 'N'

def branch_name(m, v):
    if m == NF:
        return 'BPL' if v==0 else 'BMI'
    elif m == VF:
        return 'BVC' if v==0 else 'BVS'
    elif m == CF:
        return 'BCC' if v==0 else 'BCS'
    elif m == ZF:
        return 'BNE' if v==0 else 'BEQ'

# addressing mode constants
A____ = 0       # no addressing mode
A_IMM = 1       # immediate
A_ZER = 2       # zero-page
A_ZPX = 3       # zp,X
A_ZPY = 4       # zp,Y
A_ABS = 5       # abs
A_ABX = 6       # abs,X
A_ABY = 7       # abs,Y
A_IDX = 8       # (zp,X)
A_IDY = 9       # (zp),Y
A_JMP = 10      # special JMP abs
A_JSR = 11      # special JSR abs
A_INV = 12      # an invalid instruction

# addressing mode strings
addr_mode_str = ['', '#', 'zp', 'zp,X', 'zp,Y', 'abs', 'abs,X', 'abs,Y', '(zp,X)', '(zp),Y', '', '', 'INVALID']

# memory access modes
M___ = 0        # no memory access
M_R_ = 1        # read access
M__W = 2        # write access
M_RW = 3        # read-modify-write

# addressing-modes and memory accesses for each instruction
ops = [
    # cc = 00
    [
        # ---         BIT          JMP          JMP()        STY          LDY          CPY          CPX
        [[A____,M___],[A_JSR,M_R_],[A____,M_R_],[A____,M_R_],[A_IMM,M_R_],[A_IMM,M_R_],[A_IMM,M_R_],[A_IMM,M_R_]],
        [[A_ZER,M_R_],[A_ZER,M_R_],[A_ZER,M_R_],[A_ZER,M_R_],[A_ZER,M__W],[A_ZER,M_R_],[A_ZER,M_R_],[A_ZER,M_R_]],
        [[A____,M___],[A____,M___],[A____,M__W],[A____,M___],[A____,M___],[A____,M___],[A____,M___],[A____,M___]],
        [[A_ABS,M_R_],[A_ABS,M_R_],[A_JMP,M_R_],[A_JMP,M_R_],[A_ABS,M__W],[A_ABS,M_R_],[A_ABS,M_R_],[A_ABS,M_R_]],
        [[A_IMM,M_R_],[A_IMM,M_R_],[A_IMM,M_R_],[A_IMM,M_R_],[A_IMM,M_R_],[A_IMM,M_R_],[A_IMM,M_R_],[A_IMM,M_R_]],  # relative branches
        [[A_ZPX,M_R_],[A_ZPX,M_R_],[A_ZPX,M_R_],[A_ZPX,M_R_],[A_ZPX,M__W],[A_ZPX,M_R_],[A_ZPX,M_R_],[A_ZPX,M_R_]],
        [[A____,M___],[A____,M___],[A____,M___],[A____,M___],[A____,M___],[A____,M___],[A____,M___],[A____,M___]],
        [[A_ABX,M_R_],[A_ABX,M_R_],[A_ABS,M_R_],[A_ABS,M_R_],[A_ABX,M__W],[A_ABX,M_R_],[A_ABX,M_R_],[A_ABX,M_R_]]        
    ],
    # cc = 01
    [
        # ORA         AND          EOR          ADC          STA          LDA          CMP          SBC
        [[A_IDX,M_R_],[A_IDX,M_R_],[A_IDX,M_R_],[A_IDX,M_R_],[A_IDX,M__W],[A_IDX,M_R_],[A_IDX,M_R_],[A_IDX,M_R_]],
        [[A_ZER,M_R_],[A_ZER,M_R_],[A_ZER,M_R_],[A_ZER,M_R_],[A_ZER,M__W],[A_ZER,M_R_],[A_ZER,M_R_],[A_ZER,M_R_]],
        [[A_IMM,M_R_],[A_IMM,M_R_],[A_IMM,M_R_],[A_IMM,M_R_],[A_IMM,M_R_],[A_IMM,M_R_],[A_IMM,M_R_],[A_IMM,M_R_]],
        [[A_ABS,M_R_],[A_ABS,M_R_],[A_ABS,M_R_],[A_ABS,M_R_],[A_ABS,M__W],[A_ABS,M_R_],[A_ABS,M_R_],[A_ABS,M_R_]],
        [[A_IDY,M_R_],[A_IDY,M_R_],[A_IDY,M_R_],[A_IDY,M_R_],[A_IDY,M__W],[A_IDY,M_R_],[A_IDY,M_R_],[A_IDY,M_R_]],
        [[A_ZPX,M_R_],[A_ZPX,M_R_],[A_ZPX,M_R_],[A_ZPX,M_R_],[A_ZPX,M__W],[A_ZPX,M_R_],[A_ZPX,M_R_],[A_ZPX,M_R_]],
        [[A_ABY,M_R_],[A_ABY,M_R_],[A_ABY,M_R_],[A_ABY,M_R_],[A_ABY,M__W],[A_ABY,M_R_],[A_ABY,M_R_],[A_ABY,M_R_]],
        [[A_ABX,M_R_],[A_ABX,M_R_],[A_ABX,M_R_],[A_ABX,M_R_],[A_ABX,M__W],[A_ABX,M_R_],[A_ABX,M_R_],[A_ABX,M_R_]]
    ],
    # cc = 02
    [
        # ASL         ROL          LSR          ROR          STX          LDX          DEC          INC
        [[A_INV,M_RW],[A_INV,M_RW],[A_INV,M_RW],[A_INV,M_RW],[A_IMM,M_R_],[A_IMM,M_R_],[A_IMM,M_R_],[A_IMM,M_R_]],
        [[A_ZER,M_RW],[A_ZER,M_RW],[A_ZER,M_RW],[A_ZER,M_RW],[A_ZER,M__W],[A_ZER,M_R_],[A_ZER,M_RW],[A_ZER,M_RW]],
        [[A____,M___],[A____,M___],[A____,M___],[A____,M___],[A____,M___],[A____,M___],[A____,M___],[A____,M___]],
        [[A_ABS,M_RW],[A_ABS,M_RW],[A_ABS,M_RW],[A_ABS,M_RW],[A_ABS,M__W],[A_ABS,M_R_],[A_ABS,M_RW],[A_ABS,M_RW]],
        [[A_INV,M_RW],[A_INV,M_RW],[A_INV,M_RW],[A_INV,M_RW],[A_INV,M__W],[A_INV,M_R_],[A_INV,M_RW],[A_INV,M_RW]],
        [[A_ZPX,M_RW],[A_ZPX,M_RW],[A_ZPX,M_RW],[A_ZPX,M_RW],[A_ZPY,M__W],[A_ZPY,M_R_],[A_ZPX,M_RW],[A_ZPX,M_RW]],
        [[A____,M_R_],[A____,M_R_],[A____,M_R_],[A____,M_R_],[A____,M___],[A____,M___],[A____,M_R_],[A____,M_R_]],
        [[A_ABX,M_RW],[A_ABX,M_RW],[A_ABX,M_RW],[A_ABX,M_RW],[A_ABY,M__W],[A_ABY,M_R_],[A_ABX,M_RW],[A_ABX,M_RW]]
    ],
    # cc = 03
    [
        [[A_IDX,M_RW],[A_IDX,M_RW],[A_IDX,M_RW],[A_IDX,M_RW],[A_IDX,M__W],[A_IDX,M_R_],[A_IDX,M_RW],[A_IDX,M_RW]],
        [[A_ZER,M_RW],[A_ZER,M_RW],[A_ZER,M_RW],[A_ZER,M_RW],[A_ZER,M__W],[A_ZER,M_R_],[A_ZER,M_RW],[A_ZER,M_RW]],
        [[A_IMM,M_R_],[A_IMM,M_R_],[A_IMM,M_R_],[A_IMM,M_R_],[A_IMM,M_R_],[A_IMM,M_R_],[A_IMM,M_R_],[A_IMM,M_R_]],
        [[A_ABS,M_RW],[A_ABS,M_RW],[A_ABS,M_RW],[A_ABS,M_RW],[A_ABS,M__W],[A_ABS,M_R_],[A_ABS,M_RW],[A_ABS,M_RW]],
        [[A_IDY,M_RW],[A_IDY,M_RW],[A_IDY,M_RW],[A_IDY,M_RW],[A_IDY,M_RW],[A_IDY,M_R_],[A_IDY,M_RW],[A_IDY,M_RW]],
        [[A_ZPX,M_RW],[A_ZPX,M_RW],[A_ZPX,M_RW],[A_ZPX,M_RW],[A_ZPY,M__W],[A_ZPY,M_R_],[A_ZPX,M_RW],[A_ZPX,M_RW]],
        [[A_ABY,M_RW],[A_ABY,M_RW],[A_ABY,M_RW],[A_ABY,M_RW],[A_ABY,M__W],[A_ABY,M_R_],[A_ABY,M_RW],[A_ABY,M_RW]],
        [[A_ABX,M_RW],[A_ABX,M_RW],[A_ABX,M_RW],[A_ABX,M_RW],[A_ABY,M__W],[A_ABY,M_R_],[A_ABX,M_RW],[A_ABX,M_RW]]
    ]
]

class opcode:
    def __init__(self, op):
        self.byte = op
        self.cmt = None
        self.src = None

#-------------------------------------------------------------------------------
#   output a src line
#
out_lines = ''
def l(s) :
    global out_lines
    out_lines += s + '\n'

#-------------------------------------------------------------------------------
def write_op(op):
    if not op.cmt:
        op.cmt = '???'
    l('            case '+hex(op.byte)+':/*'+op.cmt+'*/'+op.src+'break;')

#-------------------------------------------------------------------------------
def cmt(o,cmd):
    cc = o.byte & 3
    bbb = (o.byte>>2) & 7
    aaa = (o.byte>>5) & 7
    addr_mode = ops[cc][bbb][aaa][0]
    o.cmt = cmd;
    if addr_mode != '':
        o.cmt += ' '+addr_mode_str[addr_mode]

#-------------------------------------------------------------------------------
def u_cmt(o,cmd):
    cmt(o,cmd)
    o.cmt += ' (undoc)'

#-------------------------------------------------------------------------------
def invalid_opcode(op):
    cc = op & 3
    bbb = (op>>2) & 7
    aaa = (op>>5) & 7
    addr_mode = ops[cc][bbb][aaa][0]
    return addr_mode == A_INV

#-------------------------------------------------------------------------------
def enc_addr(op):
    # returns a string performing the addressing mode decode steps, 
    # result will be in the address bus pins
    cc = op & 3
    bbb = (op>>2) & 7
    aaa = (op>>5) & 7
    addr_mode = ops[cc][bbb][aaa][0]
    mem_access = ops[cc][bbb][aaa][1]
    if addr_mode == A____:
        # no addressing, this still puts the PC on the address bus without 
        # incrementing the PC
        src = '_A_IMP();'
    elif addr_mode == A_IMM:
        # immediate mode
        src = '_A_IMM();'
    elif addr_mode == A_ZER:
        # zero page
        src = '_A_ZER();'
    elif addr_mode == A_ZPX:
        # zero page + X
        src = '_A_ZPX();'
    elif addr_mode == A_ZPY:
        # zero page + Y
        src = '_A_ZPY();'
    elif addr_mode == A_ABS:
        # absolute
        src = '_A_ABS();'
    elif addr_mode == A_ABX:
        # absolute + X
        # this needs to check if a page boundary is crossed, which costs
        # and additional cycle, but this early-out only happens when the
        # instruction doesn't need to write back to memory
        if mem_access == M_R_:
            src = '_A_ABX_R();'
        else:
            src = '_A_ABX_W();'
    elif addr_mode == A_ABY:
        # absolute + Y
        # same page-boundary-crossed special case as absolute+X
        if mem_access == M_R_:
            src = '_A_ABY_R();'
        else:
            src = '_A_ABY_W();'
    elif addr_mode == A_IDX:
        # (zp,X)
        src = '_A_IDX();'
    elif addr_mode == A_IDY:
        # (zp),Y
        # same page-boundary-crossed special case as absolute+X
        if mem_access == M_R_:
            src = '_A_IDY_R();'
        else:
            src = '_A_IDY_W();'
    elif addr_mode == A_JMP:
        # jmp is completely handled in instruction decoding
        src = ''
    elif addr_mode == A_JSR:
        # jsr is completely handled in instruction decoding 
        src = ''
    else:
        # invalid instruction
        src = ''
    return src

#-------------------------------------------------------------------------------
def i_brk(o):
    # this only covers the normal instruction version of brk, not
    # an interrupt acknowlegde brk!
    cmt(o, 'BRK')
    o.src += '_RD();'
    o.src += 'c.PC++;'
    # write PC high byte to stack
    o.src += '_SAD(0x0100|c.S--,c.PC>>8);_WR();'
    # write PC low byte to stack
    o.src += '_SAD(0x0100|c.S--,c.PC);_WR();'
    # write status flags to stack (with set BF flag)
    o.src += '_SAD(0x0100|c.S--,c.P|M6502_BF);_WR();'
    # load jump vector low byte from 0xFFFE
    o.src += '_SA(0xFFFE);_RD();l=_GD();'
    # load jump vector high byte from 0xFFFF
    o.src += '_SA(0xFFFF);_RD();h=_GD();'
    # build PC
    o.src += 'c.PC=(h<<8)|l;'
    # disable interrupts
    o.src += 'c.P|=M6502_IF;'

#-------------------------------------------------------------------------------
def i_nop(o):
    cmt(o,'NOP')
    o.src += '_RD();'

#-------------------------------------------------------------------------------
def u_nop(o):
    u_cmt(o,'NOP')
    o.src += '_RD();'

#-------------------------------------------------------------------------------
def i_lda(o):
    cmt(o,'LDA')
    o.src += '_RD();c.A=_GD();_NZ(c.A);'

#-------------------------------------------------------------------------------
def i_ldx(o):
    cmt(o,'LDX')
    o.src += '_RD();c.X=_GD();_NZ(c.X);'

#-------------------------------------------------------------------------------
def i_ldy(o):
    cmt(o,'LDY')
    o.src += '_RD();c.Y=_GD();_NZ(c.Y);'

#-------------------------------------------------------------------------------
def u_lax(o):
    u_cmt(o,'LAX')
    o.src += '_RD();c.A=c.X=_GD();_NZ(c.A);'

#-------------------------------------------------------------------------------
def x_lxa(o):
    # undocumented LXA
    # and immediate byte with A, then load X with A
    # this fails in the Wolfgang Lorenz test suite
    u_cmt(o,'LXA')
    o.src += '_RD();c.A&=_GD();c.X=c.A;_NZ(c.A);'

#-------------------------------------------------------------------------------
def i_sta(o):
    cmt(o,'STA')
    o.src += '_SD(c.A);_WR();'

#-------------------------------------------------------------------------------
def i_stx(o):
    cmt(o,'STX')
    o.src += '_SD(c.X);_WR();'

#-------------------------------------------------------------------------------
def i_sty(o):
    cmt(o,'STY')
    o.src += '_SD(c.Y);_WR();'

#-------------------------------------------------------------------------------
def u_sax(o):
    u_cmt(o,'SAX')
    o.src += '_SD(c.A&c.X);_WR();'

#-------------------------------------------------------------------------------
def i_tax(o):
    cmt(o,'TAX')
    o.src += '_RD();c.X=c.A;_NZ(c.X);'

#-------------------------------------------------------------------------------
def i_tay(o):
    cmt(o,'TAY')
    o.src += '_RD();c.Y=c.A;_NZ(c.Y);'

#-------------------------------------------------------------------------------
def i_txa(o):
    cmt(o,'TXA')
    o.src += '_RD();c.A=c.X;_NZ(c.A);'

#-------------------------------------------------------------------------------
def i_tya(o):
    cmt(o,'TYA')
    o.src += '_RD();c.A=c.Y;_NZ(c.A);'

#-------------------------------------------------------------------------------
def i_txs(o):
    cmt(o,'TXS')
    o.src += '_RD();c.S=c.X;'

#-------------------------------------------------------------------------------
def i_tsx(o):
    cmt(o,'TSX')
    o.src += '_RD();c.X=c.S;_NZ(c.X);'

#-------------------------------------------------------------------------------
def i_php(o):
    cmt(o,'PHP')
    o.src += '_RD();_SAD(0x0100|c.S--,c.P|M6502_BF);_WR();'

#-------------------------------------------------------------------------------
def i_plp(o):
    cmt(o,'PLP')
    o.src += '_RD();'
    o.src += '_SA(0x0100|c.S++);_RD();' # read junk byte from current SP
    o.src += '_SA(0x0100|c.S);_RD();'   # read actual byte
    o.src += 'c.P=(_GD()&~M6502_BF)|M6502_XF;'

#-------------------------------------------------------------------------------
def i_pha(o):
    cmt(o,'PHA')
    o.src += '_RD();_SAD(0x0100|c.S--,c.A);_WR();'

#-------------------------------------------------------------------------------
def i_pla(o):
    cmt(o,'PLA')
    o.src += '_RD();'
    o.src += '_SA(0x0100|c.S++);_RD();' # read junk byte from current SP
    o.src += '_SA(0x0100|c.S);_RD();'   # read actual byte
    o.src += 'c.A=_GD();_NZ(c.A);'

#-------------------------------------------------------------------------------
def i_se(o, f):
    cmt(o,'SE'+flag_name(f))
    o.src += '_RD();c.P|='+hex(f)+';'

#-------------------------------------------------------------------------------
def i_cl(o, f):
    cmt(o,'CL'+flag_name(f))
    o.src += '_RD();c.P&=~'+hex(f)+';'

#-------------------------------------------------------------------------------
def i_br(o, m, v):
    cmt(o,branch_name(m,v))
    o.src += '_RD();'
    o.src += 'if((c.P&'+hex(m)+')=='+hex(v)+'){'
    o.src +=   '_RD();'   # branch taken, at least 3 cycles
    o.src +=   't=c.PC+(int8_t)_GD();'
    o.src +=   'if((t&0xFF00)!=(c.PC&0xFF00)){' 
    o.src +=     '_RD();' # target address not in same memory page, 4 cycles
    o.src +=   '}'
    o.src +=   'c.PC=t;'
    o.src += '}'

#-------------------------------------------------------------------------------
def x_bra(o):
    # this is 'branch always' instruction on the 65C02
    cmt(o,'BRA (65C02 ext)')
    o.src += '_RD();'
    o.src += 'if(c.m65c02_mode){'
    o.src += '_RD();'   # branch always taken, at least 3 cycles
    o.src += 't=c.PC+(int8_t)_GD();'
    o.src += 'if((t&0xFF00)!=(c.PC&0xFF00)){' 
    o.src +=   '_RD();' # target address not in same memory page, 4 cycles
    o.src += '}'
    o.src += 'c.PC=t;'
    o.src += '}'

#-------------------------------------------------------------------------------
def i_jmp(o):
    cmt(o,'JMP')
    o.src += '_SA(c.PC++);_RD();l=_GD();'
    o.src += '_SA(c.PC++);_RD();h=_GD();'
    o.src += 'c.PC=(h<<8)|l;'

#-------------------------------------------------------------------------------
def i_jmpi(o):
    cmt(o,'JMPI')
    o.src += '_SA(c.PC++);_RD();l=_GD();'
    o.src += '_SA(c.PC++);_RD();h=_GD();'
    o.src += 'a=(h<<8)|l;'
    o.src += '_SA(a);_RD();l=_GD();'    # load first byte of target address
    o.src += 'a=(a&0xFF00)|((a+1)&0x00FF);'
    o.src += '_SA(a);_RD();h=_GD();'    # load second byte of target address
    o.src += 'c.PC=(h<<8)|l;'

#-------------------------------------------------------------------------------
def i_jsr(o):
    cmt(o,'JSR')
    # read low byte of target address
    o.src += '_SA(c.PC++);_RD();l=_GD();'
    # put SP on addr bus, next cycle is a junk read
    o.src += '_SA(0x0100|c.S);_RD();'
    # write PC high byte to stack
    o.src += '_SAD(0x0100|c.S--,c.PC>>8);_WR();'
    # write PC low byte to stack
    o.src += '_SAD(0x0100|c.S--,c.PC);_WR();'
    # load target address high byte
    o.src += '_SA(c.PC);_RD();h=_GD();'
    # build new PC
    o.src += 'c.PC=(h<<8)|l;'

#-------------------------------------------------------------------------------
def i_rts(o):
    cmt(o,'RTS')
    o.src += '_RD();'
    # put SP on stack and do a junk read
    o.src += '_SA(0x0100|c.S++);_RD();'
    # load return address low byte from stack
    o.src += '_SA(0x0100|c.S++);_RD();l=_GD();'
    # load return address high byte from stack
    o.src += '_SA(0x0100|c.S);_RD();h=_GD();'
    # put return address in PC, this is one byte before next op
    o.src += 'c.PC=(h<<8)|l;'
    # do a junk read from PC, increment PC to actual return-to instruction
    o.src += '_SA(c.PC++);_RD();'

#-------------------------------------------------------------------------------
def i_rti(o):
    cmt(o,'RTI')
    o.src += '_RD();'
    # put SP on stack and do a junk read
    o.src += '_SA(0x0100|c.S++);_RD();'
    # load processor status flag from stack
    o.src += '_SA(0x0100|c.S++);_RD();c.P=(_GD()&~M6502_BF)|M6502_XF;'
    # load return address low byte from stack
    o.src += '_SA(0x0100|c.S++);_RD();l=_GD();'
    # load return address high byte from stack
    o.src += '_SA(0x0100|c.S);_RD();h=_GD();'
    # update PC (which is already placed on the right return-to instruction)
    o.src += 'c.PC=(h<<8)|l;'
    # interrupt reponse after RTI is immediately
    o.src += 'c.pi=c.P;'

#-------------------------------------------------------------------------------
def i_ora(o):
    cmt(o,'ORA')
    o.src += '_RD();c.A|=_GD();_NZ(c.A);'

#-------------------------------------------------------------------------------
def i_and(o):
    cmt(o,'AND')
    o.src += '_RD();c.A&=_GD();_NZ(c.A);'

#-------------------------------------------------------------------------------
def i_eor(o):
    cmt(o,'EOR')
    o.src += '_RD();c.A^=_GD();_NZ(c.A);'

#-------------------------------------------------------------------------------
def i_adc(o):
    cmt(o,'ADC')
    o.src += '_RD();_m6502_adc(&c,_GD());'

#-------------------------------------------------------------------------------
def i_sbc(o):
    cmt(o,'SBC')
    o.src += '_RD();_m6502_sbc(&c,_GD());'

#-------------------------------------------------------------------------------
def u_sbc(o):
    u_cmt(o,'SBC')
    o.src += '_RD();_m6502_sbc(&c,_GD());'

#-------------------------------------------------------------------------------
def i_cmp(o):
    cmt(o,'CMP')
    o.src += '_RD();l=_GD();'
    o.src += 't=c.A-l;'
    o.src += '_NZ((uint8_t)t)&~M6502_CF;'
    o.src += 'if(!(t&0xFF00)){c.P|=M6502_CF;}'

#-------------------------------------------------------------------------------
def i_cpx(o):
    cmt(o,'CPX')
    o.src += '_RD();l=_GD();'
    o.src += 't=c.X-l;'
    o.src += '_NZ((uint8_t)t)&~M6502_CF;'
    o.src += 'if(!(t&0xFF00)){c.P|=M6502_CF;}'

#-------------------------------------------------------------------------------
def i_cpy(o):
    cmt(o,'CPY')
    o.src += '_RD();l=_GD();'
    o.src += 't=c.Y-l;'
    o.src += '_NZ((uint8_t)t)&~M6502_CF;'
    o.src += 'if(!(t&0xFF00)){c.P|=M6502_CF;}'

#-------------------------------------------------------------------------------
def i_dec(o):
    cmt(o,'DEC')
    o.src += '_RD();l=_GD();'
    o.src += '_WR();'   # first write is the unmodified value
    o.src += 'l--;_NZ(l);'
    o.src += '_SD(l);_WR();'

#-------------------------------------------------------------------------------
def u_dcp(o):
    # undocumented 'decrement and compare'
    u_cmt(o,'DCP')
    o.src += '_RD();'
    o.src += '_WR();'
    o.src += 'l=_GD();l--;_NZ(l);_SD(l);_WR();'
    # do a cmp operation on the decremented value
    o.src += 't=c.A-l;'
    o.src += '_NZ((uint8_t)t)&~M6502_CF;'
    o.src += 'if(!(t&0xFF00)){c.P|=M6502_CF;}'

#-------------------------------------------------------------------------------
def x_sbx(o):
    # undocumented SBX
    # AND X register with accumulator and store result in X register, then
    # subtract byte from X register (without borrow)
    #
    # we just ignore this for now and treat it like a imm-nop
    #
    u_cmt(o,'SBX (not impl)')
    o.src += '_RD();'

#-------------------------------------------------------------------------------
def i_dex(o):
    cmt(o,'DEX')
    o.src += '_RD();c.X--;_NZ(c.X);'

#-------------------------------------------------------------------------------
def i_dey(o):
    cmt(o,'DEY')
    o.src += '_RD();c.Y--;_NZ(c.Y);'

#-------------------------------------------------------------------------------
def i_inc(o):
    cmt(o,'INC')
    o.src += '_RD();l=_GD();'
    o.src += '_WR();'   # first write is the unmodified value
    o.src += 'l++;_NZ(l);'
    o.src += '_SD(l);_WR();'

#-------------------------------------------------------------------------------
def i_inx(o):
    cmt(o,'INX')
    o.src += '_RD();c.X++;_NZ(c.X);'

#-------------------------------------------------------------------------------
def i_iny(o):
    cmt(o,'INY')
    o.src += '_RD();c.Y++;_NZ(c.Y);'

#-------------------------------------------------------------------------------
def u_isb(o):
    # undocumented INC+SBC instruction
    u_cmt(o,'ISB')
    o.src += '_RD();'
    o.src += '_WR();'
    o.src += 'l=_GD();l++;_SD(l);_WR();'
    o.src += '_m6502_sbc(&c,l);'

#-------------------------------------------------------------------------------
def _asl(val):
    s  = 'c.P=(c.P&~M6502_CF)|(('+val+'&0x80)?M6502_CF:0);'
    s += val+'<<=1;'
    s += '_NZ('+val+');'
    return s

#-------------------------------------------------------------------------------
def i_asl(o):
    cmt(o,'ASL')
    o.src += '_RD();'
    o.src += '_WR();' # write unmodified value
    o.src += 'l=_GD();'
    o.src += _asl('l')
    o.src += '_SD(l);'
    o.src += '_WR();'

#-------------------------------------------------------------------------------
def i_asla(o):
    cmt(o,'ASLA')
    o.src += '_RD();'
    o.src += _asl('c.A')

#-------------------------------------------------------------------------------
def u_slo(o):
    # undocumented ASL+OR
    u_cmt(o,'SLO')
    o.src += '_RD();'
    o.src += '_WR();'
    o.src += 'l=_GD();'
    o.src += _asl('l')
    o.src += '_SD(l);'
    o.src += '_WR();'
    o.src += 'c.A|=l;_NZ(c.A);'

#-------------------------------------------------------------------------------
def _lsr(val):
    s  = 'c.P=(c.P&~M6502_CF)|(('+val+'&0x01)?M6502_CF:0);'
    s += val+'>>=1;'
    s += '_NZ('+val+');'
    return s

#-------------------------------------------------------------------------------
def i_lsr(o):
    cmt(o,'LSR')
    o.src += '_RD();'
    o.src += '_WR();' # write unmodified value
    o.src += 'l=_GD();'
    o.src += _lsr('l')
    o.src += '_SD(l);'
    o.src += '_WR();'

#-------------------------------------------------------------------------------
def i_lsra(o):
    cmt(o,'LSRA')
    o.src += '_RD();'
    o.src += _lsr('c.A')

#-------------------------------------------------------------------------------
def x_asr(o):
    # undocumented AND+LSR
    u_cmt(o, 'ASR')
    o.src += '_RD();'
    o.src += 'c.A&=_GD();'
    o.src += _lsr('c.A')

#-------------------------------------------------------------------------------
def u_sre(o):
    # undocumented LSR+EOR
    u_cmt(o,'SRE')
    o.src += '_RD();'
    o.src += '_WR();'
    o.src += 'l=_GD();'
    o.src += _lsr('l')
    o.src += '_SD(l);'
    o.src += '_WR();'
    o.src += 'c.A^=l;_NZ(c.A);'

#-------------------------------------------------------------------------------
def _rol(val):
    s  = '{'
    s += 'bool carry=c.P&M6502_CF;'
    s += 'c.P&=~(M6502_NF|M6502_ZF|M6502_CF);'
    s += 'if('+val+'&0x80){c.P|=M6502_CF;}'
    s += val+'<<=1;'
    s += 'if(carry){'+val+'|=0x01;}'
    s += '_NZ('+val+');'
    s += '}'
    return s

#-------------------------------------------------------------------------------
def i_rol(o):
    cmt(o,'ROL')
    o.src += '_RD();'
    o.src += '_WR();' # write unmodified value
    o.src += 'l=_GD();'
    o.src += _rol('l')
    o.src += '_SD(l);'
    o.src += '_WR();'

#-------------------------------------------------------------------------------
def i_rola(o):
    cmt(o,'ROLA')
    o.src += '_RD();'
    o.src += _rol('c.A')

#-------------------------------------------------------------------------------
def u_rla(o):
    # uncodumented ROL+AND
    u_cmt(o,'RLA')
    o.src += '_RD();'
    o.src += '_WR();'
    o.src += 'l=_GD();'
    o.src += _rol('l')
    o.src += '_SD(l);'
    o.src += '_WR();'
    o.src += 'c.A&=l;_NZ(c.A);'

#-------------------------------------------------------------------------------
def _ror(val):
    s  = '{'
    s += 'bool carry=c.P&M6502_CF;'
    s += 'c.P&=~(M6502_NF|M6502_ZF|M6502_CF);'
    s += 'if('+val+'&0x01){c.P|=M6502_CF;}'
    s += val+'>>=1;'
    s += 'if(carry){'+val+'|=0x80;}'
    s += '_NZ('+val+');'
    s += '}'
    return s

#-------------------------------------------------------------------------------
def i_ror(o):
    cmt(o,'ROR')
    o.src += '_RD();'
    o.src += '_WR();' # write unmodified value
    o.src += 'l=_GD();'
    o.src += _ror('l')
    o.src += '_SD(l);'
    o.src += '_WR();'

#-------------------------------------------------------------------------------
def i_rora(o):
    cmt(o,'RORA')
    o.src += '_RD();'
    o.src += _ror('c.A')

#-------------------------------------------------------------------------------
def u_rra(o):
    # undocumented ROR+ADC
    u_cmt(o,'RRA')
    o.src += '_RD();'
    o.src += '_WR();'
    o.src += 'l=_GD();'
    o.src += _ror('l')
    o.src += '_SD(l);'
    o.src += '_WR();'
    o.src += '_m6502_adc(&c,l);'

#-------------------------------------------------------------------------------
def x_arr(o):
    # undocumented AND+ROR
    u_cmt(o,'ARR')
    o.src += '_RD();'
    o.src += 'c.A&=_GD();'
    o.src += '_m6502_arr(&c);'

#-------------------------------------------------------------------------------
def x_ane(o):
    # undocumented ANE
    # NOTE: this implementation fails in the Wolfgang Lorenz test suite
    u_cmt(o,'ANE')
    o.src += '_RD();'
    o.src += 'l=_GD();c.A&=l&c.X;_NZ(c.A);'

#-------------------------------------------------------------------------------
def x_sha(o):
    # undocumented SHA
    # AND X register with accumulator then AND result with 7 and store in
    # memory.
    #
    # we just ignore this for now
    u_cmt(o,'SHA (not impl)')
    o.src += '_RD();'

#-------------------------------------------------------------------------------
def x_shx(o):
    # undocumented SHX
    # AND X register with the high byte of the target address of the argument
    # + 1. Store the result in memory.
    #
    # we just ignore this for now
    u_cmt(o, 'SHX (not impl)')
    o.src += '_RD();'

#-------------------------------------------------------------------------------
def x_shy(o):
    # undocumented SHX
    # AND Y register with the high byte of the target address of the argument
    # + 1. Store the result in memory.
    #
    # we just ignore this for now
    u_cmt(o, 'SHY (not impl)')
    o.src += '_RD();'

#-------------------------------------------------------------------------------
def x_shs(o):
    # undocumented SHS
    # AND X register with accumulator and store result in stack pointer, then
    # AND stack pointer with the high byte of the target address of the
    # argument + 1. Store result in memory.
    #
    # we just ignore this for now
    u_cmt(o, 'SHS (not impl)')
    o.src += '_RD();'

#-------------------------------------------------------------------------------
def x_anc(o):
    # undocumented ANC
    # AND byte with accumulator. If result is negative then carry is set.
    #
    u_cmt(o, 'ANC')
    o.src += '_RD();'
    o.src += 'c.A&=_GD();'
    o.src += '_NZ(c.A);'
    o.src += 'if(c.A&0x80){c.P|=M6502_CF;}else{c.P&=~M6502_CF;}'

#-------------------------------------------------------------------------------
def x_las(o):
    # undocumented LAS
    # AND memory with stack pointer, transfer result to accumulator, X
    # register and stack pointer.
    #
    # we just ignore this for now
    u_cmt(o, 'LAS (not impl)')
    o.src += '_RD();'

#-------------------------------------------------------------------------------
def i_bit(o):
    cmt(o,'BIT')
    o.src += '_RD();'
    o.src += 'l=_GD();h=c.A&l;'
    o.src += 'c.P&=~(M6502_NF|M6502_VF|M6502_ZF);'
    o.src += 'if(!h){c.P|=M6502_ZF;}'
    o.src += 'c.P|=l&(M6502_NF|M6502_VF);'

#-------------------------------------------------------------------------------
def enc_op(op):
    o = opcode(op)
    if invalid_opcode(op):
        o.cmt = 'INVALID'
        o.src = ''
        return o
    # addressing mode decoder
    o.src = enc_addr(op)
    # instruction decoding
    cc = op & 3
    bbb = (op>>2) & 7
    aaa = (op>>5) & 7
    if cc == 0:
        if aaa == 0:
            if bbb == 0:        i_brk(o)
            elif bbb == 2:      i_php(o)
            elif bbb == 4:      i_br(o, NF, 0)  # BPL
            elif bbb == 6:      i_cl(o, CF)
            else:               u_nop(o)
        elif aaa == 1:
            if bbb == 0:        i_jsr(o)
            elif bbb == 2:      i_plp(o)
            elif bbb == 4:      i_br(o, NF, NF) # BMI
            elif bbb == 6:      i_se(o, CF)
            elif bbb in [5, 7]: u_nop(o)
            else:               i_bit(o)
        elif aaa == 2:
            if bbb == 0:        i_rti(o)
            elif bbb == 2:      i_pha(o)
            elif bbb == 3:      i_jmp(o)
            elif bbb == 4:      i_br(o, VF, 0)  # BVC
            elif bbb == 6:      i_cl(o, IF)
            else:               u_nop(o)
        elif aaa == 3:
            if bbb == 0:        i_rts(o)
            elif bbb == 2:      i_pla(o)
            elif bbb == 3:      i_jmpi(o)
            elif bbb == 4:      i_br(o, VF, VF) # BVS
            elif bbb == 6:      i_se(o, IF)
            else:               u_nop(o)
        elif aaa == 4:
            if bbb == 0:        u_nop(o)
            elif bbb == 2:      i_dey(o)
            elif bbb == 4:      i_br(o, CF, 0)  # BCC
            elif bbb == 6:      i_tya(o)
            elif bbb == 7:      x_shy(o)
            else:               i_sty(o)
        elif aaa == 5:
            if bbb == 2:        i_tay(o)
            elif bbb == 4:      i_br(o, CF, CF) # BCS
            elif bbb == 6:      i_cl(o, VF)
            else:               i_ldy(o)
        elif aaa == 6:
            if bbb == 2:        i_iny(o)
            elif bbb == 4:      i_br(o, ZF, 0)  # BNE
            elif bbb == 6:      i_cl(o, DF)
            elif bbb in [5, 7]: u_nop(o)
            else:               i_cpy(o)
        elif aaa == 7:
            if bbb == 2:        i_inx(o)
            elif bbb == 4:      i_br(o, ZF, ZF) # BEQ
            elif bbb == 6:      i_se(o, DF)
            elif bbb in [5, 7]: u_nop(o)
            else:               i_cpx(o)
    elif cc == 1:
        if aaa == 0:    i_ora(o)
        elif aaa == 1:  i_and(o)
        elif aaa == 2:  i_eor(o)
        elif aaa == 3:  i_adc(o)
        elif aaa == 4:
            if bbb == 2:    u_nop(o)
            else:           i_sta(o)
        elif aaa == 5:  i_lda(o)
        elif aaa == 6:  i_cmp(o)
        else:           i_sbc(o)
    elif cc == 2:
        if aaa == 0:
            if bbb == 2:    i_asla(o)
            elif bbb == 6:  u_nop(o)
            else:           i_asl(o)
        elif aaa == 1:
            if bbb == 2:    i_rola(o)
            elif bbb == 6:  u_nop(o)
            else:           i_rol(o)
        elif aaa == 2:
            if bbb == 2:    i_lsra(o)
            elif bbb == 6:  u_nop(o)
            else:           i_lsr(o)
        elif aaa == 3:
            if bbb == 2:    i_rora(o)
            elif bbb == 6:  u_nop(o)
            else:           i_ror(o)
        elif aaa == 4:
            if bbb == 0:    u_nop(o)
            elif bbb == 2:  i_txa(o)
            elif bbb == 6:  i_txs(o)
            elif bbb == 7:  x_shx(o)
            else:           i_stx(o)
        elif aaa == 5:
            if bbb == 2:    i_tax(o)
            elif bbb == 6:  i_tsx(o)
            else:           i_ldx(o)
        elif aaa == 6:
            if bbb == 2:        i_dex(o)
            elif bbb in [0, 6]: u_nop(o)
            else:               i_dec(o)
        elif aaa == 7:
            if bbb == 2:        i_nop(o)
            elif bbb in [0, 6]: u_nop(o)
            else:               i_inc(o)
    elif cc == 3:
        # undocumented block
        if aaa == 0:    
            if bbb == 2:    x_anc(o)
            else:           u_slo(o)
        elif aaa == 1:  
            if bbb == 2:    x_anc(o)
            else:           u_rla(o)
        elif aaa == 2:
            if bbb == 2:    x_asr(o)
            else:           u_sre(o)
        elif aaa == 3:  
            if bbb == 2:    x_arr(o)
            else:           u_rra(o)
        elif aaa == 4:
            if bbb == 2:        x_ane(o)
            elif bbb == 6:      x_shs(o)
            elif bbb in [4,7]:  x_sha(o)
            else:               u_sax(o)
        elif aaa == 5:
            if bbb == 2:    x_lxa(o)
            elif bbb == 6:  x_las(o)
            else:           u_lax(o)
        elif aaa == 6:
            if bbb == 2:    x_sbx(o)
            else:           u_dcp(o)
        elif aaa == 7:
            if bbb == 2:    u_sbc(o)
            else:           u_isb(o)
    return o

#-------------------------------------------------------------------------------
#   execution starts here
#
for i in range(0, 256):
    write_op(enc_op(i))

with open(InpPath, 'r') as inf:
    templ = Template(inf.read())
    c_src = templ.safe_substitute(decode_block=out_lines)
    with open(OutPath, 'w') as outf:
        outf.write(c_src)