#-------------------------------------------------------------------------------
#   z80_opcodes.py
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
OutPath = '../chips/_z80_opcodes.h'

# the target file handle
Out = None

# 8-bit register table, the 'HL' entry is for instructions that use
# (HL), (IX+d) and (IY+d), and will be patched to 'IX' or 'IY' for
# the DD/FD prefix instructions
r = [ 'B', 'C', 'D', 'E', 'H', 'L', 'HL', 'A' ]

# the same, but the 'HL' item is never patched to IX/IY, this is
# for indexed instructions that load into H or L (e.g. LD H,(IX+d))
r2 = [ 'B', 'C', 'D', 'E', 'H', 'L', 'HL', 'A' ]

# 16-bit register table, with SP
rp = [ 'BC', 'DE', 'HL', 'SP' ]

# 16-bit register table, with AF (only used for PUSH/POP)
rp2 = [ 'BC', 'DE', 'HL', 'AF' ]

# condition-code table (for conditional jumps etc)
cond = [
    '!(c->F&Z80_ZF)',  # NZ
    '(c->F&Z80_ZF)',   # Z
    '!(c->F&Z80_CF)',   # NC
    '(c->F&Z80_CF)',    # C
    '!(c->F&Z80_PF)',   # PO
    '(c->F&Z80_PF)',    # PE
    '!(c->F&Z80_SF)',   # P
    '(c->F&Z80_SF)'     # M
]

# the same as 'human readable' flags for comments
cond_cmt = [ 'NZ', 'Z', 'NC', 'C', 'PO', 'PE', 'P', 'M' ]

# 8-bit ALU instruction table (C++ method names)
alu = [ '_z80_add', '_z80_adc', '_z80_sub', '_z80_sbc', '_z80_and', '_z80_xor', '_z80_or', '_z80_cp' ]

# the same 'human readable' for comments
alu_cmt = [ 'ADD', 'ADC', 'SUB', 'SBC', 'AND', 'XOR', 'OR', 'CP' ]

# rot and shift instruction table (C++ method names)
rot = [ '_z80_rlc', '_z80_rrc', '_z80_rl', '_z80_rr', '_z80_sla', '_z80_sra', '_z80_sll', '_z80_srl' ]

# the same 'human readbla for comments
rot_cmt = [ 'RLC', 'RRC', 'RL', 'RR', 'SLA', 'SRA', 'SLL', 'SRL' ]

# an 'opcode' wraps the instruction byte, human-readable asm mnemonics,
# and the source code which implements the instruction
class opcode :
    def __init__(self, op) :
        self.byte = op
        self.cmt = None
        self.src = None

#-------------------------------------------------------------------------------
# return comment string for (HL), (IX+d), (IY+d)
#
def iHLcmt(ext) :
    if (ext) :
        return '(c->{}+d)'.format(r[6])
    else :
        return '(c->{})'.format(r[6])

#-------------------------------------------------------------------------------
# Return code to setup an address variable 'a' with the address of HL
# or (IX+d), (IY+d). For the index instructions also update WZ with
# IX+d or IY+d
#
def iHLsrc(ext) :
    if (ext) :
        # IX+d or IY+d
        return 'uint16_t a=c->WZ=c->{}+_RD(c->PC++)'.format(r[6])
    else :
        # HL
        return 'uint16_t a=c->{}'.format(r[6])

#-------------------------------------------------------------------------------
# Return code to setup an variable 'a' with the address of HL or (IX+d), (IY+d).
# For the index instructions, also update WZ with IX+d or IY+d
#
def iHLdsrc(ext) :
    if (ext) :
        # IX+d or IY+d
        return 'uint16_t a=c->WZ=c->{}+d;'.format(r[6])
    else :
        # HL
        return 'uint16_t a=c->{}'.format(r[6])

#-------------------------------------------------------------------------------
# Encode a main instruction, or an DD or FD prefix instruction.
# Takes an opcode byte and returns an opcode object, for invalid instructions
# the opcode object will be in its default state (opcode.src==None).
# cc is the name of the cycle-count table.
#
def enc_op(op, ext, cc) :

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
                o.src = '_z80_halt(c);'
            else:
                # LD (HL),r; LD (IX+d),r; LD (IX+d),r
                o.cmt = 'LD {},{}'.format(iHLcmt(ext), r2[z])
                o.src = '{{ {}; _WR(a,c->{}); }};'.format(iHLsrc(ext), r2[z])
        elif z == 6:
            # LD r,(HL); LD r,(IX+d); LD r,(IY+d)
            o.cmt = 'LD {},{}'.format(r2[y], iHLcmt(ext))
            o.src = '{{ {}; c->{}=_RD(a); }};'.format(iHLsrc(ext), r2[y])
        else:
            # LD r,s
            o.cmt = 'LD {},{}'.format(r[y], r[z])
            o.src = 'c->{}=c->{};'.format(r[y], r[z])

    #---- block 2: 8-bit ALU instructions (ADD, ADC, SUB, SBC, AND, XOR, OR, CP)
    elif x == 2:
        if z == 6:
            # ALU (HL); ALU (IX+d); ALU (IY+d)
            o.cmt = '{} {}'.format(alu_cmt[y], iHLcmt(ext))
            o.src = '{{ {}; {}(c,_RD(a)); }};'.format(iHLsrc(ext), alu[y])
        else:
            # ALU r
            o.cmt = '{} {}'.format(alu_cmt[y], r[z])
            o.src = '{}(c,c->{});'.format(alu[y], r[z])

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
                o.src = '_SWP16(c->AF,c->AF_);'
            elif y == 2:
                # DJNZ d
                o.cmt = 'DJNZ'
                o.src = '_z80_djnz(c);'
            elif  y == 3:
                # JR d
                o.cmt = 'JR d'
                o.src = '_z80_jr(c);'
            else:
                # JR cc,d
                o.cmt = 'JR {},d'.format(cond_cmt[y-4])
                o.src = '_z80_jr_cc(c,{});'.format(cond[y-4])
        elif z == 1:
            if q == 0:
                # 16-bit immediate loads
                o.cmt = 'LD {},nn'.format(rp[p])
                o.src = '_IMM16(); c->{}=c->WZ;'.format(rp[p])
            else :
                # ADD HL,rr; ADD IX,rr; ADD IY,rr
                o.cmt = 'ADD {},{}'.format(rp[2],rp[p])
                o.src = 'c->{}=_z80_add16(c,c->{},c->{});'.format(rp[2],rp[2],rp[p]) 
        elif z == 2:
            # indirect loads
            op_tbl = [
                [ 'LD (BC),A', 'c->WZ=c->BC;_WR(c->WZ++,c->A);c->W=c->A;' ],
                [ 'LD A,(BC)', 'c->WZ=c->BC;c->A=_RD(c->WZ++);' ],
                [ 'LD (DE),A', 'c->WZ=c->DE;_WR(c->WZ++,c->A);c->W=c->A;' ],
                [ 'LD A,(DE)', 'c->WZ=c->DE;c->A=_RD(c->WZ++);' ],
                [ 'LD (nn),{}'.format(rp[2]), '{{_IMM16();_WR(c->WZ++,(uint8_t)c->{});_WR(c->WZ,(uint8_t)(c->{}>>8));}}'.format(rp[2],rp[2]) ],
                [ 'LD {},(nn)'.format(rp[2]), '{{_IMM16();uint8_t l=_RD(c->WZ++);uint8_t h=_RD(c->WZ);c->{}=(h<<8)|l;}}'.format(rp[2]) ],
                [ 'LD (nn),A', '_IMM16();_WR(c->WZ++,c->A);c->W=c->A;' ],
                [ 'LD A,(nn)', '_IMM16();c->A=_RD(c->WZ++);' ]
            ]
            o.cmt = op_tbl[y][0]
            o.src = op_tbl[y][1]
        elif z == 3:
            # 16-bit INC/DEC 
            if q == 0:
                o.cmt = 'INC {}'.format(rp[p])
                o.src = 'c->{}++;'.format(rp[p])
            else:
                o.cmt = 'DEC {}'.format(rp[p])
                o.src = 'c->{}--;'.format(rp[p])
        elif z == 4 or z == 5:
            cmt = 'INC' if z == 4 else 'DEC'
            fn = '_z80_inc' if z == 4 else '_z80_dec'
            if y == 6:
                # INC/DEC (HL)/(IX+d)/(IY+d)
                o.cmt = '{} {}'.format(cmt, iHLcmt(ext))
                o.src = '{{ {}; _WR(a,{}(c,_RD(a))); }}'.format(iHLsrc(ext), fn)
            else:
                # INC/DEC r
                o.cmt = '{} {}'.format(cmt, r[y])
                o.src = 'c->{}={}(c,c->{});'.format(r[y], fn, r[y])
        elif z == 6:
            if y == 6:
                # LD (HL),n; LD (IX+d),n; LD (IY+d),n
                o.cmt = 'LD {},n'.format(iHLcmt(ext))
                o.src = '{{ {}; _WR(a,_RD(c->PC++)); }}'.format(iHLsrc(ext))
            else:
                # LD r,n
                o.cmt = 'LD {},n'.format(r[y])
                o.src = 'c->{}=_RD(c->PC++);'.format(r[y])
        elif z == 7:
            # misc ops on A and F
            op_tbl = [
                [ 'RLCA', '_z80_rlca(c);'],
                [ 'RRCA', '_z80_rrca(c);'],
                [ 'RLA',  '_z80_rla(c);'],
                [ 'RRA',  '_z80_rra(c);'],
                [ 'DAA',  '_z80_daa(c);'],
                [ 'CPL',  '_z80_cpl(c);'],
                [ 'SCF',  '_z80_scf(c);'],
                [ 'CCF',  '_z80_ccf(c);']
            ]
            o.cmt = op_tbl[y][0]
            o.src = op_tbl[y][1]

    #--- block 3: misc and extended ops
    elif x == 3:
        if z == 0:
            # RET cc
            o.cmt = 'RET {}'.format(cond_cmt[y])
            o.src = '_z80_retcc(c,{});'.format(cond[y])
        elif z == 1:
            if q == 0:
                # POP BC,DE,HL,IX,IY,AF
                o.cmt = 'POP {}'.format(rp2[p])
                o.src = '{{ uint8_t l=_RD(c->SP++);uint8_t h=_RD(c->SP++);c->{}=(h<<8)|l; }}'.format(rp2[p])
            else:
                # misc ops
                op_tbl = [
                    [ 'RET', '_z80_ret(c);' ],
                    [ 'EXX', '_SWP16(c->BC,c->BC_);_SWP16(c->DE,c->DE_);_SWP16(c->HL,c->HL_);_SWP16(c->WZ,c->WZ_);' ],
                    [ 'JP {}'.format(rp[2]), 'c->PC=c->{};'.format(rp[2]) ],
                    [ 'LD SP,{}'.format(rp[2]), '_T();_T();c->SP=c->{};'.format(rp[2]) ]
                ]
                o.cmt = op_tbl[p][0]
                o.src = op_tbl[p][1]
        elif z == 2:
            # JP cc,nn
            o.cmt = 'JP {},nn'.format(cond_cmt[y])
            o.src = '_IMM16(); if ({}) {{ c->PC=c->WZ; }}'.format(cond[y])
        elif z == 3:
            # misc ops
            op_tbl = [
                [ 'JP nn', '_IMM16(); c->PC=c->WZ;' ],
                [ None, None ], # CB prefix instructions
                [ 'OUT (n),A', '_OUT((c->A<<8)|_RD(c->PC++), c->A);' ],
                [ 'IN A,(n)', 'c->A=_IN((c->A<<8)|_RD(c->PC++));' ],
                [ 
                    'EX (SP),{}'.format(rp[2]), 
                    'c->{}=_z80_exsp(c,c->{});'.format(rp[2], rp[2])
                ],
                [ 'EX DE,HL', '_SWP16(c->DE,c->HL);' ],
                [ 'DI', '_z80_di(c);' ],
                [ 'EI', '_z80_ei(c);' ]
            ]
            o.cmt = op_tbl[y][0]
            o.src = op_tbl[y][1]
        elif z == 4:
            # CALL cc,nn
            o.cmt = 'CALL {},nn'.format(cond_cmt[y])
            o.src = '_z80_callcc(c, {});'.format(cond[y])
        elif z == 5:
            if q == 0:
                # PUSH BC,DE,HL,IX,IY,AF
                o.cmt = 'PUSH {}'.format(rp2[p])
                o.src = '_WR(--c->SP,(uint8_t)(c->{}>>8)); _WR(--c->SP,(uint8_t)c->{});'.format(rp2[p], rp2[p])
            else:
                op_tbl = [
                    [ 'CALL nn', '_z80_call(c);' ],
                    [ None, None ], # DD prefix instructions
                    [ None, None ], # ED prefix instructions
                    [ None, None ], # FD prefix instructions
                ]
                o.cmt = op_tbl[p][0]
                o.src = op_tbl[p][1]
        elif z == 6:
            # ALU n
            o.cmt = '{} n'.format(alu_cmt[y])
            o.src = '{}(c,_RD(c->PC++));'.format(alu[y])
        elif z == 7:
            # RST
            o.cmt = 'RST {}'.format(hex(y*8))
            o.src = '_z80_rst(c,{});'.format(hex(y*8))

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
                    [ 'LDI',    '_z80_ldi(c);' ],
                    [ 'LDD',    '_z80_ldd(c);' ],
                    [ 'LDIR',   '_z80_ldir(c);' ],
                    [ 'LDDR',   '_z80_lddr(c);' ]
                ],
                [
                    [ 'CPI',    '_z80_cpi(c);' ],
                    [ 'CPD',    '_z80_cpd(c);' ],
                    [ 'CPIR',   '_z80_cpir(c);' ],
                    [ 'CPDR',   '_z80_cpdr(c);' ]
                ],
                [
                    [ 'INI',    '_z80_ini(c);' ],
                    [ 'IND',    '_z80_ind(c);' ],
                    [ 'INIR',   '_z80_inir(c);' ],
                    [ 'INDR',   '_z80_indr(c);' ]
                ],
                [
                    [ 'OUTI',   '_z80_outi(c);' ],
                    [ 'OUTD',   '_z80_outd(c);' ],
                    [ 'OTIR',   '_z80_otir(c);' ],
                    [ 'OTDR',   '_z80_otdr(c);' ]
                ]
            ]
            o.cmt = op_tbl[z][y-4][0]
            o.src = op_tbl[z][y-4][1]

    elif x == 1:
        # misc ops
        if z == 0:
            # IN r,(C)
            if y == 6:
                # undocumented special case 'IN F,(C)', only alter flags, don't store result
                o.cmt = 'IN (C)';
                o.src = 'c->F=c->szp[_IN(c->BC)]|(c->F&Z80_CF);'
            else:
                o.cmt = 'IN {},(C)'.format(r[y])
                o.src = 'c->{}=_IN(c->BC); c->F=c->szp[c->{}]|(c->F&Z80_CF);'.format(r[y],r[y])
        elif z == 1:
            # OUT (C),r
            if y == 6:
                # undocumented special case 'OUT (C),F', always output 0
                o.cmd = 'OUT (C)';
                o.src = '_OUT(c->BC,0);'
            else:
                o.cmt = 'OUT (C),{}'.format(r[y])
                o.src = '_OUT(c->BC,c->{});'.format(r[y])
        elif z == 2:
            # SBC/ADC HL,rr
            cmt = 'SBC' if q == 0 else 'ADC'
            src = '_z80_sbc16' if q == 0 else '_z80_adc16'
            o.cmt = '{} HL,{}'.format(cmt, rp[p])
            o.src = 'c->HL={}(c,c->HL,c->{});'.format(src, rp[p])
        elif z == 3:
            # 16-bit immediate address load/store
            if q == 0:
                o.cmt = 'LD (nn),{}'.format(rp[p])
                o.src = '_IMM16();_WR(c->WZ++,(uint8_t)c->{});_WR(c->WZ,(uint8_t)(c->{}>>8));'.format(rp[p],rp[p])
            else:
                o.cmt = 'LD {},(nn)'.format(rp[p])
                o.src = '{{_IMM16();uint8_t l=_RD(c->WZ++);uint8_t h=_RD(c->WZ);c->{}=(h<<8)|l;}}'.format(rp[p])
        elif z == 4:
            # NEG
            o.cmt = 'NEG'
            o.src = '_z80_neg(c);'
        elif z == 5:
            # RETN, RETI (only RETI implemented!)
            if y == 1:
                o.cmt = 'RETI'
                o.src = '_z80_reti(c);'
        elif z == 6:
            # IM m
            im_mode = [ 0, 0, 1, 2, 0, 0, 1, 2 ]
            o.cmt = 'IM {}'.format(im_mode[y])
            o.src = 'c->IM={};'.format(im_mode[y])
        elif z == 7:
            # misc ops on I,R and A
            op_tbl = [
                [ 'LD I,A', 'c->I=c->A;' ],
                [ 'LD R,A', 'c->R=c->A;' ],
                [ 'LD A,I', 'c->A=c->I; c->F=_z80_sziff2(c->I,c->IFF2)|(c->F&Z80_CF);' ],
                [ 'LD A,R', 'c->A=c->R; c->F=_z80_sziff2(c->R,c->IFF2)|(c->F&Z80_CF);' ],
                [ 'RRD',    '_z80_rrd(c);' ],
                [ 'RLD',    '_z80_rld(c);' ],
                [ 'NOP (ED)', ' ' ],
                [ 'NOP (ED)', ' ' ],
            ]
            o.cmt = op_tbl[y][0]
            o.src = op_tbl[y][1]
    return o

#-------------------------------------------------------------------------------
#   CB prefix instructions
#
def enc_cb_op(op, ext, cc) :
    o = opcode(op)

    x = op>>6
    y = (op>>3)&7
    z = op&7

    if x == 0:
        # rotates and shifts
        if z == 6:
            # ROT (HL); ROT (IX+d); ROT (IY+d)
            o.cmt = '{} {}'.format(rot_cmt[y],iHLcmt(ext))
            o.src = '{{ {}; _WR(a,{}(c,_RD(a))); }}'.format(iHLdsrc(ext), rot[y])
        elif ext:
            # undocumented: ROT (IX+d),(IY+d),r (also stores result in a register)
            o.cmt = '{} {},{}'.format(rot_cmt[y],iHLcmt(ext),r2[z])
            o.src = '{{ {}; c->{}={}(c,_RD(a)); _WR(a,c->{}); }}'.format(iHLdsrc(ext), r2[z], rot[y], r2[z])
        else:
            # ROT r
            o.cmt = '{} {}'.format(rot_cmt[y],r2[z])
            o.src = 'c->{}={}(c,c->{});'.format(r2[z], rot[y], r2[z])
    elif x == 1:
        # BIT n
        if z == 6 or ext:
            # BIT n,(HL); BIT n,(IX+d); BIT n,(IY+d)
            o.cmt = 'BIT {},{}'.format(y,iHLcmt(ext))
            o.src = '{{ {}; _z80_ibit(c,_RD(a),{}); }}'.format(iHLdsrc(ext), hex(1<<y))
        else:
            # BIT n,r
            o.cmt = 'BIT {},{}'.format(y,r2[z])
            o.src = '_z80_bit(c,c->{},{});'.format(r2[z], hex(1<<y))
    elif x == 2:
        # RES n
        if z == 6:
            # RES n,(HL); RES n,(IX+d); RES n,(IY+d)
            o.cmt = 'RES {},{}'.format(y,iHLcmt(ext))
            o.src = '{{ {}; _WR(a,_RD(a)&~{}); }}'.format(iHLdsrc(ext), hex(1<<y))
        elif ext:
            # undocumented: RES n,(IX+d),r; RES n,(IY+d),r
            o.cmt = 'RES {},{},{}'.format(y,iHLcmt(ext),r2[z])
            o.src = '{{ {}; c->{}=_RD(a)&~{}; _WR(a,c->{}); }}'.format(iHLdsrc(ext), r2[z], hex(1<<y), r2[z])
        else:
            # RES n,r
            o.cmt = 'RES {},{}'.format(y,r2[z])
            o.src = 'c->{}&=~{};'.format(r2[z], hex(1<<y))
    elif x == 3:
        # SET n
        if z == 6:
            # SET n,(HL); RES n,(IX+d); RES n,(IY+d)
            o.cmt = 'SET {},{}'.format(y,iHLcmt(ext))
            o.src = '{{ {}; _WR(a,_RD(a)|{});}}'.format(iHLdsrc(ext), hex(1<<y))
        elif ext:
            # undocumented: SET n,(IX+d),r; SET n,(IY+d),r
            o.cmt = 'SET {},{},{}'.format(y,iHLcmt(ext),r2[z])
            o.src = '{{ {}; c->{}=_RD(a)|{}; _WR(a,c->{});}}'.format(iHLdsrc(ext), r2[z], hex(1<<y), r[z])
        else:
            # SET n,r
            o.cmt = 'SET {},{}'.format(y,r2[z])
            o.src = 'c->{}|={};'.format(r2[z], hex(1<<y))
    return o

#-------------------------------------------------------------------------------
# patch register lookup tables for IX or IY instructions
#
def patch_reg_tables(reg) :
    r[4] = reg+'H'; r[5] = reg+'L'; r[6] = reg;
    rp[2] = rp2[2] = reg

#-------------------------------------------------------------------------------
# 'unpatch' the register lookup tables
#
def unpatch_reg_tables() :
    r[4] = 'H'; r[5] = 'L'; r[6] = 'HL'
    rp[2] = rp2[2] = 'HL'

#-------------------------------------------------------------------------------
# return a tab-string for given indent level
#
def tab(indent) :
    return ' '*TabWidth*indent

#-------------------------------------------------------------------------------
# output a src line
def l(s) :
    Out.write(s+'\n')

#-------------------------------------------------------------------------------
# write source header
#
def write_header() :
    l('// machine generated, do not edit!')
    l('void _z80_op(z80* c) {')

#-------------------------------------------------------------------------------
# begin a new instruction group (begins a switch statement)
#
def write_begin_group(indent, ext_byte=None, read_offset=False) :
    if ext_byte :
        # this is a prefix instruction, need to write a case
        l('{}case {}:'.format(tab(indent), hex(ext_byte)))
    indent += 1
    # special case for DD/FD CB 'double extended' instructions,
    # these have the d offset after the CB byte and before
    # the actual instruction byte
    if read_offset :
        l('{}{{ const int8_t d = _RD(c->PC++);'.format(tab(indent)))
    l('{}switch (_z80_fetch(c)) {{'.format(tab(indent)))
    indent += 1
    return indent

#-------------------------------------------------------------------------------
# write a single (writes a case inside the current switch)
#
def write_op(indent, op) :
    if op.src :
        l('{}case {}: {} return; // {}'.format(tab(indent), hex(op.byte), op.src, op.cmt))

#-------------------------------------------------------------------------------
# finish an instruction group (ends current statement)
#
def write_end_group(indent, inv_op_bytes, ext_byte=None, read_offset=False) :
    l('{}default: return _INVALID_OPCODE({});'.format(tab(indent), inv_op_bytes))
    indent -= 1
    l('{}}}'.format(tab(indent)))
    # if this was a prefix instruction, need to write a final break
    if ext_byte:
        l('{}break;'.format(tab(indent)))
    if read_offset :
        l('{}}}'.format(tab(indent)))
    indent -= 1
    return indent

#-------------------------------------------------------------------------------
# write source footer
#
def write_footer() :
    l('}')

#-------------------------------------------------------------------------------
# main encoder function, this populates all the opcode tables and
# generates the C++ source code into the file f
#
Out = open(OutPath, 'w')
write_header()

# loop over all instruction bytes
indent = write_begin_group(0)
for i in range(0, 256) :
    # DD or FD prefix instruction?
    if i == 0xDD or i == 0xFD:
        indent = write_begin_group(indent, i)
        patch_reg_tables('IX' if i==0xDD else 'IY')
        cc_ix_table = 'cc_dd' if i==0xDD else 'cc_fd'
        cc_ixcb_table = 'cc_ddcb' if i==0xDD else 'cc_fdcb'
        for ii in range(0, 256) :
            if ii == 0xCB:
                # DD/FD CB prefix
                indent = write_begin_group(indent, ii, True)
                for iii in range(0, 256) :
                    write_op(indent, enc_cb_op(iii, True, cc_ixcb_table))
                indent = write_end_group(indent, 4, True, True)
            else:
                write_op(indent, enc_op(ii, True, cc_ix_table))
        unpatch_reg_tables()
        indent = write_end_group(indent, 2, True)
    # ED prefix instructions
    elif i == 0xED:
        indent = write_begin_group(indent, i)
        for ii in range(0, 256) :
            write_op(indent, enc_ed_op(ii))
        indent = write_end_group(indent, 2, True)
    # CB prefix instructions
    elif i == 0xCB:
        indent = write_begin_group(indent, i, False)
        for ii in range(0, 256) :
            write_op(indent, enc_cb_op(ii, False, 'cc_cb'))
        indent = write_end_group(indent, 2, True)
    # non-prefixed instruction
    else:
        write_op(indent, enc_op(i, False, 'cc_op'))
write_end_group(indent, 1)
write_footer()

