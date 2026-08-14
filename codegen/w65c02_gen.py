#-------------------------------------------------------------------------------
#   w65c02_gen.py
#   Generate the instruction decoder for the w65c02.h emulator.
#
#   The CMOS additions (e.g. 0x12 'ORA (zp)', the BBR/BBS/RMB/SMB block) don't
#   fit the regular aaa/bbb/cc opcode bit-field pattern, so this generator drives
#   a flat 256-entry decode table. Cycle timing and bus behaviour follow the WDC
#   W65C02S as measured by the SingleStepTests/ProcessorTests 'wdc65c02' vectors.
#-------------------------------------------------------------------------------
import templ

INOUT_PATH = '../chips/w65c02.h'

# addressing modes
A_NONE = 0      # no operand cycle (1-cycle NOPs)
A_IMP  = 1      # implied: one dummy read of PC
A_IMM  = 2      # #
A_ZER  = 3      # zp
A_ZPX  = 4      # zp,X
A_ZPY  = 5      # zp,Y
A_ABS  = 6      # abs
A_ABX  = 7      # abs,X
A_ABY  = 8      # abs,Y
A_IDX  = 9      # (zp,X)
A_IDY  = 10     # (zp),Y
A_IZP  = 11     # (zp)

# memory access class
M___ = 0        # no memory access
M_R_ = 1        # read
M__W = 2        # write
M_RW = 3        # read-modify-write, fixed length (INC/DEC/TSB/TRB): abs,X always +dummy
M_RS = 4        # read-modify-write shift (ASL/LSR/ROL/ROR): abs,X skips dummy if no page cross

#-------------------------------------------------------------------------------
class opcode:
    def __init__(self, op):
        self.code = op
        self.cmt = None
        self.i = 0
        self.src = [None] * 8
    def t(self, src):
        self.src[self.i] = src
        self.i += 1
    def ta(self, src):
        self.src[self.i-1] += src

out_lines = ''
def l(s):
    global out_lines
    out_lines += s + '\n'

def write_op(op):
    if not op.cmt: op.cmt = '???'
    l('    /* {} */'.format(op.cmt))
    for t in range(0, 8):
        if t < op.i:
            l('        case (0x{:02X}<<3)|{}: {}break;'.format(op.code, t, op.src[t]))
        else:
            l('        case (0x{:02X}<<3)|{}: assert(false);break;'.format(op.code, t))

#-------------------------------------------------------------------------------
#   addressing: leave the effective address on the bus; for reads the operand
#   is available via _GD() in the following step. 65C02 indexed accesses place
#   their dummy read at the last instruction byte (PC-1 after operand fetch).
#-------------------------------------------------------------------------------
def enc_addr(op, am, mem):
    if am == A_NONE:
        pass
    elif am == A_IMP:
        op.t('_SA(c->PC);')
    elif am == A_IMM:
        op.t('_SA(c->PC++);')
    elif am == A_ZER:
        op.t('_SA(c->PC++);')
        op.t('_SA(_GD());')
    elif am == A_ZPX:
        op.t('_SA(c->PC++);')
        op.t('c->AD=_GD();_SA(c->AD);')
        op.t('_SA((c->AD+c->X)&0x00FF);')
    elif am == A_ZPY:
        op.t('_SA(c->PC++);')
        op.t('c->AD=_GD();_SA(c->AD);')
        op.t('_SA((c->AD+c->Y)&0x00FF);')
    elif am == A_ABS:
        op.t('_SA(c->PC++);')
        op.t('_SA(c->PC++);c->AD=_GD();')
        op.t('_SA((_GD()<<8)|c->AD);')
    elif am in (A_ABX, A_ABY):
        r = 'c->X' if am==A_ABX else 'c->Y'
        op.t('_SA(c->PC++);')
        op.t('_SA(c->PC++);c->AD=_GD();')
        if mem in (M_R_, M_RS):
            # read/shift: dummy (@ last instr byte) only if page crossed, else read effective now
            op.t('c->AD|=_GD()<<8;if((((c->AD)^(c->AD+%s))&0xFF00)==0){_SA(c->AD+%s);c->IR++;}else{_SA((c->PC-1)&0xFFFF);}' % (r, r))
            op.t('_SA(c->AD+%s);' % r)
        else:
            # write/rmw: always a dummy read of the last instruction byte
            op.t('c->AD|=_GD()<<8;_SA((c->PC-1)&0xFFFF);')
            op.t('_SA(c->AD+%s);' % r)
    elif am == A_IDX:
        op.t('_SA(c->PC++);')
        op.t('c->AD=_GD();_SA(c->AD);')
        op.t('c->AD=(c->AD+c->X)&0xFF;_SA(c->AD);')
        op.t('_SA((c->AD+1)&0xFF);c->AD=_GD();')
        op.t('_SA((_GD()<<8)|c->AD);')
    elif am == A_IDY:
        op.t('_SA(c->PC++);')
        op.t('c->AD=_GD();_SA(c->AD);')
        op.t('_SA((c->AD+1)&0xFF);c->AD=_GD();')
        if mem in (M_R_,):
            op.t('c->AD|=_GD()<<8;if((((c->AD)^(c->AD+c->Y))&0xFF00)==0){_SA(c->AD+c->Y);c->IR++;}else{_SA((c->PC-1)&0xFFFF);}')
            op.t('_SA(c->AD+c->Y);')
        else:
            op.t('c->AD|=_GD()<<8;_SA((c->PC-1)&0xFFFF);')
            op.t('_SA(c->AD+c->Y);')
    elif am == A_IZP:
        op.t('_SA(c->PC++);')
        op.t('c->AD=_GD();_SA(c->AD);')
        op.t('_SA((c->AD+1)&0xFF);c->AD=_GD();')
        op.t('_SA((_GD()<<8)|c->AD);')

#-------------------------------------------------------------------------------
#   emitters (operate on the operand read at the effective address)
#-------------------------------------------------------------------------------
def i_nop(o):   o.t('')
def i_lda(o):   o.t('c->A=_GD();_NZ(c->A);')
def i_ldx(o):   o.t('c->X=_GD();_NZ(c->X);')
def i_ldy(o):   o.t('c->Y=_GD();_NZ(c->Y);')
def i_sta(o):   o.ta('_SD(c->A);_WR();')
def i_stx(o):   o.ta('_SD(c->X);_WR();')
def i_sty(o):   o.ta('_SD(c->Y);_WR();')
def i_stz(o):   o.ta('_SD(0x00);_WR();')
def i_tax(o):   o.t('c->X=c->A;_NZ(c->X);')
def i_tay(o):   o.t('c->Y=c->A;_NZ(c->Y);')
def i_txa(o):   o.t('c->A=c->X;_NZ(c->A);')
def i_tya(o):   o.t('c->A=c->Y;_NZ(c->A);')
def i_txs(o):   o.t('c->S=c->X;')
def i_tsx(o):   o.t('c->X=c->S;_NZ(c->X);')
def i_dex(o):   o.t('c->X--;_NZ(c->X);')
def i_dey(o):   o.t('c->Y--;_NZ(c->Y);')
def i_inx(o):   o.t('c->X++;_NZ(c->X);')
def i_iny(o):   o.t('c->Y++;_NZ(c->Y);')
def i_inca(o):  o.t('c->A++;_NZ(c->A);')
def i_deca(o):  o.t('c->A--;_NZ(c->A);')
def i_ora(o):   o.t('c->A|=_GD();_NZ(c->A);')
def i_and(o):   o.t('c->A&=_GD();_NZ(c->A);')
def i_eor(o):   o.t('c->A^=_GD();_NZ(c->A);')
def i_cmp(o):   o.t('_w65c02_cmp(c,c->A,_GD());')
def i_cpx(o):   o.t('_w65c02_cmp(c,c->X,_GD());')
def i_cpy(o):   o.t('_w65c02_cmp(c,c->Y,_GD());')
def i_bit(o):   o.t('_w65c02_bit(c,_GD());')
def i_bitimm(o):o.t('c->P&=~W65C02_ZF;if(0==(c->A&_GD())){c->P|=W65C02_ZF;}')

# ADC/SBC: +1 cycle in decimal mode, a dummy read of the effective address.
# For immediate mode there is no effective address; the WDC part drives a fixed
# internal-latch value onto the bus (0x007F for ADC #, 0x0000 for SBC #, as
# observed in the SingleStepTests wdc65c02 traces).
def _adcsbc(fn, dummy='_GA()'):
    def i(o):
        o.t('c->AD=_GD();if(0==(c->P&W65C02_DF)){%s(c,c->AD);_FETCH();c->IR++;}else{_SA(%s);}' % (fn, dummy))
        o.t('%s(c,c->AD);_FETCH();' % fn)
    return i
i_adc = _adcsbc('_w65c02_adc')
i_sbc = _adcsbc('_w65c02_sbc')
i_adc_imm = _adcsbc('_w65c02_adc', '0x007F')
i_sbc_imm = _adcsbc('_w65c02_sbc', '0x0000')

# accumulator shifts/rotates (2-cycle, dummy read of PC done by A_IMP)
def i_asla(o):  o.t('c->A=_w65c02_asl(c,c->A);')
def i_lsra(o):  o.t('c->A=_w65c02_lsr(c,c->A);')
def i_rola(o):  o.t('c->A=_w65c02_rol(c,c->A);')
def i_rora(o):  o.t('c->A=_w65c02_ror(c,c->A);')

# memory read-modify-write: 65C02 read-READ-write (dummy read, not dummy write)
def _rmw(expr):
    def i(o):
        o.t('c->AD=_GD();')
        o.t('%s;_SD(c->AD);_WR();' % expr)
    return i
i_asl = _rmw('c->AD=_w65c02_asl(c,c->AD)')
i_lsr = _rmw('c->AD=_w65c02_lsr(c,c->AD)')
i_rol = _rmw('c->AD=_w65c02_rol(c,c->AD)')
i_ror = _rmw('c->AD=_w65c02_ror(c,c->AD)')
i_inc = _rmw('c->AD++;_NZ(c->AD)')
i_dec = _rmw('c->AD--;_NZ(c->AD)')
def i_tsb(o):
    o.t('c->AD=_GD();')
    o.t('c->P&=~W65C02_ZF;if(0==(c->A&c->AD)){c->P|=W65C02_ZF;}c->AD|=c->A;_SD(c->AD);_WR();')
def i_trb(o):
    o.t('c->AD=_GD();')
    o.t('c->P&=~W65C02_ZF;if(0==(c->A&c->AD)){c->P|=W65C02_ZF;}c->AD&=~c->A;_SD(c->AD);_WR();')
def rmb(bit):
    def i(o):
        o.t('c->AD=_GD();')
        o.t('c->AD&=~0x%02X;_SD(c->AD);_WR();' % (1<<bit))
    return i
def smb(bit):
    def i(o):
        o.t('c->AD=_GD();')
        o.t('c->AD|=0x%02X;_SD(c->AD);_WR();' % (1<<bit))
    return i

# stack
def i_php(o):   o.t('_SAD(0x0100|c->S--,c->P|W65C02_BF|W65C02_XF);_WR();')
def i_pha(o):   o.t('_SAD(0x0100|c->S--,c->A);_WR();')
def i_phx(o):   o.t('_SAD(0x0100|c->S--,c->X);_WR();')
def i_phy(o):   o.t('_SAD(0x0100|c->S--,c->Y);_WR();')
def i_plp(o):
    o.t('_SA(0x0100|c->S++);')
    o.t('_SA(0x0100|c->S);')
    o.t('c->P=(_GD()|W65C02_XF)&~W65C02_BF;')
def i_pla(o):
    o.t('_SA(0x0100|c->S++);')
    o.t('_SA(0x0100|c->S);')
    o.t('c->A=_GD();_NZ(c->A);')
def i_plx(o):
    o.t('_SA(0x0100|c->S++);')
    o.t('_SA(0x0100|c->S);')
    o.t('c->X=_GD();_NZ(c->X);')
def i_ply(o):
    o.t('_SA(0x0100|c->S++);')
    o.t('_SA(0x0100|c->S);')
    o.t('c->Y=_GD();_NZ(c->Y);')

def se(f):
    def i(o): o.t('c->P|=W65C02_%s;'%f)
    return i
def cl(f):
    def i(o): o.t('c->P&=~W65C02_%s;'%f)
    return i

# branches (after A_IMM operand fetch)
def br(m, v):
    def i(o):
        o.t('_SA(c->PC);c->AD=c->PC+(int8_t)_GD();if((c->P&W65C02_%s)!=%s){_FETCH();};'%(m,v))
        o.t('_SA((c->PC&0xFF00)|(c->AD&0x00FF));if((c->AD&0xFF00)==(c->PC&0xFF00)){c->PC=c->AD;c->irq_pip>>=1;c->nmi_pip>>=1;_FETCH();};')
        o.t('c->PC=c->AD;')
    return i
def i_bra(o):
    o.t('_SA(c->PC);c->AD=c->PC+(int8_t)_GD();')
    o.t('_SA((c->PC&0xFF00)|(c->AD&0x00FF));if((c->AD&0xFF00)==(c->PC&0xFF00)){c->PC=c->AD;c->irq_pip>>=1;c->nmi_pip>>=1;_FETCH();};')
    o.t('c->PC=c->AD;')

# BBRx/BBSx: all reads, 5 cycles not taken / 6 taken / 7 taken across a page
def bbr(bit): return _bitbr(bit, taken='0==')     # branch if bit clear
def bbs(bit): return _bitbr(bit, taken='0!=')     # branch if bit set
def _bitbr(bit, taken):
    def i(o):
        o.t('_SA(c->PC++);')                                   # fetch zp addr
        o.t('c->AD=_GD();_SA(c->AD);')                         # read zp value
        o.t('c->AD=_GD();_SA(_GA());')                         # re-read, address held
        o.t('c->AD=(c->AD>>%d)&1;_SA(c->PC++);' % bit)         # AD=selected bit; fetch rel
        o.t('if(%s(uint8_t)c->AD){c->AD=c->PC+(int8_t)_GD();_SA(c->PC);}else{_FETCH();}' % taken)
        o.t('if((c->AD&0xFF00)==(c->PC&0xFF00)){c->PC=c->AD;_FETCH();}else{_SA(c->PC);}')
        o.t('c->PC=c->AD;')
    return i

# jumps
def i_jmp(o):
    o.t('_SA(c->PC++);')
    o.t('_SA(c->PC++);c->AD=_GD();')
    o.t('c->PC=(_GD()<<8)|c->AD;')
def i_jmpi(o):
    # JMP (abs): a pointer ending in $FF still reads the wrapped high byte, then
    # spends an extra cycle re-reading it from the correct address (6 cycles).
    o.t('_SA(c->PC++);')
    o.t('_SA(c->PC++);c->AD=_GD();')
    o.t('c->AD|=_GD()<<8;_SA(c->AD);')
    o.t('c->PC=_GD();_SA((c->AD&0xFF00)|((c->AD+1)&0x00FF));')
    o.t('_SA((c->AD+1)&0xFFFF);')
    o.t('c->PC=(_GD()<<8)|(c->PC&0xFF);')
def i_jmpix(o):
    o.t('_SA(c->PC++);')
    o.t('_SA(c->PC++);c->AD=_GD();')
    o.t('c->AD|=_GD()<<8;_SA((c->PC-2)&0xFFFF);')
    o.t('c->AD=(c->AD+c->X)&0xFFFF;_SA(c->AD);')
    o.t('_SA((c->AD+1)&0xFFFF);c->AD=_GD();')
    o.t('c->PC=(_GD()<<8)|c->AD;')
def i_jsr(o):
    o.t('_SA(c->PC++);')
    o.t('_SA(0x0100|c->S);c->AD=_GD();')
    o.t('_SAD(0x0100|c->S--,c->PC>>8);_WR();')
    o.t('_SAD(0x0100|c->S--,c->PC);_WR();')
    o.t('_SA(c->PC);')
    o.t('c->PC=(_GD()<<8)|c->AD;')
def i_rts(o):
    o.t('_SA(c->PC);')
    o.t('_SA(0x0100|c->S++);')
    o.t('_SA(0x0100|c->S++);')
    o.t('_SA(0x0100|c->S);c->AD=_GD();')
    o.t('c->PC=(_GD()<<8)|c->AD;_SA(c->PC++);')
    o.t('')
def i_rti(o):
    o.t('_SA(c->PC);')
    o.t('_SA(0x0100|c->S++);')
    o.t('_SA(0x0100|c->S++);')
    o.t('_SA(0x0100|c->S++);c->P=(_GD()|W65C02_XF)&~W65C02_BF;')
    o.t('_SA(0x0100|c->S);c->AD=_GD();')
    o.t('c->PC=(_GD()<<8)|c->AD;')
def i_brk(o):
    o.t('_SA(c->PC);')
    o.t('if(0==(c->brk_flags&(W65C02_BRK_IRQ|W65C02_BRK_NMI))){c->PC++;}_SAD(0x0100|c->S--,c->PC>>8);if(0==(c->brk_flags&W65C02_BRK_RESET)){_WR();}')
    o.t('_SAD(0x0100|c->S--,c->PC);if(0==(c->brk_flags&W65C02_BRK_RESET)){_WR();}')
    o.t('_SAD(0x0100|c->S--,c->P|W65C02_XF|((c->brk_flags&(W65C02_BRK_IRQ|W65C02_BRK_NMI))?0:W65C02_BF));if(c->brk_flags&W65C02_BRK_RESET){c->AD=0xFFFC;}else{_WR();if(c->brk_flags&W65C02_BRK_NMI){c->AD=0xFFFA;}else{c->AD=0xFFFE;}}')
    o.t('_SA(c->AD++);c->P|=W65C02_IF;c->P&=~W65C02_DF;c->brk_flags=0;')
    o.t('_SA(c->AD);c->AD=_GD();')
    o.t('c->PC=(_GD()<<8)|c->AD;')
# WAI/STP: real stall via wait_flag/stop_flag, handled in the tick prologue
def i_wai(o):
    o.t('_SA(c->PC);')
    o.t('_SA(c->PC);c->wait_flag=1;')
def i_stp(o):
    o.t('_SA(c->PC);')
    o.t('_SA(c->PC);c->stop_flag=1;')
# 3-byte multi-cycle NOP ($5C/$DC/$FC): re-read the high operand byte
def i_nopa(o):
    o.t('_SA(c->PC++);')
    o.t('_SA(c->PC++);')
    o.t('_SA((c->PC-1)&0xFFFF);')
    o.t('')

#-------------------------------------------------------------------------------
#   256-entry decode table (mirrors util/w65c02dasm.h)
#-------------------------------------------------------------------------------
DASM = [
("BRK","IMP"),("ORA","IZX"),("NOP2","IMM"),("NOP1","NON"),("TSB","ZP"),("ORA","ZP"),("ASL","ZP"),("RMB0","ZP"),("PHP","IMP"),("ORA","IMM"),("ASLA","ACC"),("NOP1","NON"),("TSB","ABS"),("ORA","ABS"),("ASL","ABS"),("BBR0","ZPR"),
("BPL","REL"),("ORA","IZY"),("ORA","IZP"),("NOP1","NON"),("TRB","ZP"),("ORA","ZPX"),("ASL","ZPX"),("RMB1","ZP"),("CLC","IMP"),("ORA","ABY"),("INCA","ACC"),("NOP1","NON"),("TRB","ABS"),("ORA","ABX"),("ASL","ABX"),("BBR1","ZPR"),
("JSR","ABS"),("AND","IZX"),("NOP2","IMM"),("NOP1","NON"),("BIT","ZP"),("AND","ZP"),("ROL","ZP"),("RMB2","ZP"),("PLP","IMP"),("AND","IMM"),("ROLA","ACC"),("NOP1","NON"),("BIT","ABS"),("AND","ABS"),("ROL","ABS"),("BBR2","ZPR"),
("BMI","REL"),("AND","IZY"),("AND","IZP"),("NOP1","NON"),("BIT","ZPX"),("AND","ZPX"),("ROL","ZPX"),("RMB3","ZP"),("SEC","IMP"),("AND","ABY"),("DECA","ACC"),("NOP1","NON"),("BIT","ABX"),("AND","ABX"),("ROL","ABX"),("BBR3","ZPR"),
("RTI","IMP"),("EOR","IZX"),("NOP2","IMM"),("NOP1","NON"),("NOP","ZP"),("EOR","ZP"),("LSR","ZP"),("RMB4","ZP"),("PHA","IMP"),("EOR","IMM"),("LSRA","ACC"),("NOP1","NON"),("JMP","ABS"),("EOR","ABS"),("LSR","ABS"),("BBR4","ZPR"),
("BVC","REL"),("EOR","IZY"),("EOR","IZP"),("NOP1","NON"),("NOP","ZPX"),("EOR","ZPX"),("LSR","ZPX"),("RMB5","ZP"),("CLI","IMP"),("EOR","ABY"),("PHY","IMP"),("NOP1","NON"),("NOPA","AB3"),("EOR","ABX"),("LSR","ABX"),("BBR5","ZPR"),
("RTS","IMP"),("ADC","IZX"),("NOP2","IMM"),("NOP1","NON"),("STZ","ZP"),("ADC","ZP"),("ROR","ZP"),("RMB6","ZP"),("PLA","IMP"),("ADC","IMM"),("RORA","ACC"),("NOP1","NON"),("JMPI","IND"),("ADC","ABS"),("ROR","ABS"),("BBR6","ZPR"),
("BVS","REL"),("ADC","IZY"),("ADC","IZP"),("NOP1","NON"),("STZ","ZPX"),("ADC","ZPX"),("ROR","ZPX"),("RMB7","ZP"),("SEI","IMP"),("ADC","ABY"),("PLY","IMP"),("NOP1","NON"),("JMPX","IAX"),("ADC","ABX"),("ROR","ABX"),("BBR7","ZPR"),
("BRA","REL"),("STA","IZX"),("NOP2","IMM"),("NOP1","NON"),("STY","ZP"),("STA","ZP"),("STX","ZP"),("SMB0","ZP"),("DEY","IMP"),("BITIMM","IMM"),("TXA","IMP"),("NOP1","NON"),("STY","ABS"),("STA","ABS"),("STX","ABS"),("BBS0","ZPR"),
("BCC","REL"),("STA","IZY"),("STA","IZP"),("NOP1","NON"),("STY","ZPX"),("STA","ZPX"),("STX","ZPY"),("SMB1","ZP"),("TYA","IMP"),("STA","ABY"),("TXS","IMP"),("NOP1","NON"),("STZ","ABS"),("STA","ABX"),("STZ","ABX"),("BBS1","ZPR"),
("LDY","IMM"),("LDA","IZX"),("LDX","IMM"),("NOP1","NON"),("LDY","ZP"),("LDA","ZP"),("LDX","ZP"),("SMB2","ZP"),("TAY","IMP"),("LDA","IMM"),("TAX","IMP"),("NOP1","NON"),("LDY","ABS"),("LDA","ABS"),("LDX","ABS"),("BBS2","ZPR"),
("BCS","REL"),("LDA","IZY"),("LDA","IZP"),("NOP1","NON"),("LDY","ZPX"),("LDA","ZPX"),("LDX","ZPY"),("SMB3","ZP"),("CLV","IMP"),("LDA","ABY"),("TSX","IMP"),("NOP1","NON"),("LDY","ABX"),("LDA","ABX"),("LDX","ABY"),("BBS3","ZPR"),
("CPY","IMM"),("CMP","IZX"),("NOP2","IMM"),("NOP1","NON"),("CPY","ZP"),("CMP","ZP"),("DEC","ZP"),("SMB4","ZP"),("INY","IMP"),("CMP","IMM"),("DEX","IMP"),("WAI","IMP"),("CPY","ABS"),("CMP","ABS"),("DEC","ABS"),("BBS4","ZPR"),
("BNE","REL"),("CMP","IZY"),("CMP","IZP"),("NOP1","NON"),("NOP","ZPX"),("CMP","ZPX"),("DEC","ZPX"),("SMB5","ZP"),("CLD","IMP"),("CMP","ABY"),("PHX","IMP"),("STP","IMP"),("NOPA","AB3"),("CMP","ABX"),("DEC","ABX"),("BBS5","ZPR"),
("CPX","IMM"),("SBC","IZX"),("NOP2","IMM"),("NOP1","NON"),("CPX","ZP"),("SBC","ZP"),("INC","ZP"),("SMB6","ZP"),("INX","IMP"),("SBC","IMM"),("NOP","IMP"),("NOP1","NON"),("CPX","ABS"),("SBC","ABS"),("INC","ABS"),("BBS6","ZPR"),
("BEQ","REL"),("SBC","IZY"),("SBC","IZP"),("NOP1","NON"),("NOP","ZPX"),("SBC","ZPX"),("INC","ZPX"),("SMB7","ZP"),("SED","IMP"),("SBC","ABY"),("PLX","IMP"),("NOP1","NON"),("NOPA","AB3"),("SBC","ABX"),("INC","ABX"),("BBS7","ZPR"),
]
assert len(DASM) == 256

AMAP = {"IMM":A_IMM,"ZP":A_ZER,"ZPX":A_ZPX,"ZPY":A_ZPY,"IZX":A_IDX,"IZY":A_IDY,"IZP":A_IZP,
        "ABS":A_ABS,"ABX":A_ABX,"ABY":A_ABY,"ACC":A_IMP,"IMP":A_IMP,"NON":A_NONE,"REL":A_IMM}

READ = {"ORA","AND","EOR","ADC","SBC","CMP","CPX","CPY","BIT","BITIMM","LDA","LDX","LDY","NOP","NOP2"}
WRITE = {"STA","STX","STY","STZ"}
RMW  = {"INC","DEC","TSB","TRB"} | {"RMB%d"%b for b in range(8)} | {"SMB%d"%b for b in range(8)}
SHIFT = {"ASL","LSR","ROL","ROR"}
ACC = {"ASLA","LSRA","ROLA","RORA","INCA","DECA"}
PUSH = {"PHP","PHA","PHX","PHY"}
# self-contained (emitter provides all cycles incl. addressing)
SELF = {"JMP","JMPI","JMPX","JSR","RTS","RTI","BRK","NOPA"} \
       | {"BBR%d"%b for b in range(8)} | {"BBS%d"%b for b in range(8)}
BRANCH = {"BPL","BMI","BVC","BVS","BCC","BCS","BNE","BEQ","BRA"}

EMIT = {
    "ORA":i_ora,"AND":i_and,"EOR":i_eor,"ADC":i_adc,"SBC":i_sbc,"CMP":i_cmp,"CPX":i_cpx,"CPY":i_cpy,
    "BIT":i_bit,"BITIMM":i_bitimm,"LDA":i_lda,"LDX":i_ldx,"LDY":i_ldy,"NOP":i_nop,"NOP1":i_nop,"NOP2":i_nop,
    "STA":i_sta,"STX":i_stx,"STY":i_sty,"STZ":i_stz,
    "TAX":i_tax,"TAY":i_tay,"TXA":i_txa,"TYA":i_tya,"TXS":i_txs,"TSX":i_tsx,
    "PHP":i_php,"PHA":i_pha,"PHX":i_phx,"PHY":i_phy,"PLP":i_plp,"PLA":i_pla,"PLX":i_plx,"PLY":i_ply,
    "DEX":i_dex,"DEY":i_dey,"INX":i_inx,"INY":i_iny,"INCA":i_inca,"DECA":i_deca,
    "SEC":se("CF"),"SEI":se("IF"),"SED":se("DF"),"CLC":cl("CF"),"CLI":cl("IF"),"CLD":cl("DF"),"CLV":cl("VF"),
    "ASL":i_asl,"LSR":i_lsr,"ROL":i_rol,"ROR":i_ror,"INC":i_inc,"DEC":i_dec,"TSB":i_tsb,"TRB":i_trb,
    "ASLA":i_asla,"LSRA":i_lsra,"ROLA":i_rola,"RORA":i_rora,
    "JMP":i_jmp,"JMPI":i_jmpi,"JMPX":i_jmpix,"JSR":i_jsr,"RTS":i_rts,"RTI":i_rti,"BRK":i_brk,
    "BRA":i_bra,"WAI":i_wai,"STP":i_stp,"NOPA":i_nopa,
}
for b in range(8):
    EMIT["RMB%d"%b]=rmb(b); EMIT["SMB%d"%b]=smb(b)
    EMIT["BBR%d"%b]=bbr(b); EMIT["BBS%d"%b]=bbs(b)
BRAN = {"BPL":br("NF","0"),"BMI":br("NF","0x80"),"BVC":br("VF","0"),"BVS":br("VF","0x40"),
        "BCC":br("CF","0"),"BCS":br("CF","0x1"),"BNE":br("ZF","0"),"BEQ":br("ZF","0x2")}

def enc_op(op):
    o = opcode(op)
    mnem, am = DASM[op]
    o.cmt = mnem

    if mnem in SELF:
        EMIT[mnem](o)
        o.ta('_FETCH();')
        return o
    if mnem in ("WAI","STP"):
        EMIT[mnem](o); o.ta('_FETCH();'); return o

    addr = AMAP[am]
    if mnem in BRANCH or mnem in READ: mem = M_R_
    elif mnem in WRITE: mem = M__W
    elif mnem in RMW: mem = M_RW
    elif mnem in SHIFT: mem = M_RS
    else: mem = M___

    enc_addr(o, addr, mem)
    if mnem in BRAN:  BRAN[mnem](o)
    elif mnem == "BRA": i_bra(o)
    elif mnem == "ADC" and am == "IMM": i_adc_imm(o)
    elif mnem == "SBC" and am == "IMM": i_sbc_imm(o)
    else: EMIT[mnem](o)

    if mnem in ("ADC","SBC"):
        return o          # emitters emit their own _FETCH
    sep = (mem in (M__W, M_RW, M_RS)) or (mnem in PUSH)
    if sep: o.t('_FETCH();')
    else:   o.ta('_FETCH();')
    return o

def write_result():
    with open(INOUT_PATH, 'r') as f:
        lines = f.read().splitlines()
        lines = templ.replace(lines, 'decoder', out_lines)
    out_str = '\n'.join(lines) + '\n'
    with open(INOUT_PATH, 'w') as f:
        f.write(out_str)

if __name__ == '__main__':
    for op in range(0, 256):
        write_op(enc_op(op))
    write_result()
