#pragma once
/*#
    # z80dasm.h

    A stateless Z80 disassembler that doesn't call any CRT functions.

    Do this:
    ~~~C
    #define CHIPS_UTIL_IMPL
    ~~~
    before you include this file in *one* C or C++ file to create the 
    implementation.

    Optionally provide the following macros with your own implementation
    
    ~~~C
    CHIPS_ASSERT(c)
    ~~~
        your own assert macro (default: assert(c))

    ## Usage

    There's only one function to call which consumes a stream of instruction bytes
    and produces a stream of ASCII characters for exactly one instruction:

    ~~~C
    uint16_t z80dasm_op(uint16_t pc, z80dasm_input_t in_cb, z80dasm_output_t out_cb, void* user_data)
    ~~~

    pc      - the current 16-bit program counter, this is used to compute 
              absolute target addresses for relative jumps
    in_cb   - this function is called when the disassembler needs the next 
              instruction byte: uint8_t in_cb(void* user_data)
    out_cb  - (optional) this function is called when the disassembler produces a single
              ASCII character: void out_cb(char c, void* user_data)
    user_data   - a user-provided context pointer for the callbacks

    z80dasm_op() returns the new program counter (pc), this should be
    used as input arg when calling z80dasm_op() for the next instruction.

    NOTE that the output callback will never be called with a null character,
    you need to terminate the resulting string yourself if needed.

    All undocumented instructions are supported, but are currently
    not marked as such.

    ## Links

    The disassembler uses this decoding strategy:

    http://www.z80.info/decoding.htm

    ## zlib/libpng license

    Copyright (c) 2018 Andre Weissflog
    This software is provided 'as-is', without any express or implied warranty.
    In no event will the authors be held liable for any damages arising from the
    use of this software.
    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:
        1. The origin of this software must not be misrepresented; you must not
        claim that you wrote the original software. If you use this software in a
        product, an acknowledgment in the product documentation would be
        appreciated but is not required.
        2. Altered source versions must be plainly marked as such, and must not
        be misrepresented as being the original software.
        3. This notice may not be removed or altered from any source
        distribution. 
#*/
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* the input callback type */
typedef uint8_t (*z80dasm_input_t)(void* user_data);
/* the output callback type */
typedef void (*z80dasm_output_t)(char c, void* user_data);

/* disassemble a single Z80 instruction into a stream of ASCII characters */
uint16_t z80dasm_op(uint16_t pc, z80dasm_input_t in_cb, z80dasm_output_t out_cb, void* user_data);

#ifdef __cplusplus
} /* extern "C" */
#endif

/*-- IMPLEMENTATION ----------------------------------------------------------*/
#ifdef CHIPS_UTIL_IMPL
#ifndef CHIPS_ASSERT
    #include <assert.h>
    #define CHIPS_ASSERT(c) assert(c)
#endif

/* fetch unsigned 8-bit value and track pc */
#ifdef _FETCH_U8
#undef _FETCH_U8
#endif
#define _FETCH_U8(v) v=in_cb(user_data);pc++;
/* fetch signed 8-bit value and track pc */
#ifdef _FETCH_I8
#undef _FETCH_I8
#endif
#define _FETCH_I8(v) v=(int8_t)in_cb(user_data);pc++;
/* fetch unsigned 16-bit value and track pc */
#ifdef _FETCH_U16
#undef _FETCH_U16
#endif
#define _FETCH_U16(v) v=in_cb(user_data);v|=in_cb(user_data)<<8;pc+=2;
/* output character */
#ifdef _CHR
#undef _CHR
#endif
#define _CHR(c) if (out_cb) { out_cb(c,user_data); }
/* output string */
#ifdef _STR
#undef _STR
#endif
#define _STR(s) _z80dasm_str(s,out_cb,user_data);
/* output offset as signed 8-bit string (decimal) */
#ifdef _STR_D8
#undef _STR_D8
#endif
#define _STR_D8(d8) _z80dasm_d8((int8_t)(d8),out_cb,user_data);
/* output number as unsigned 8-bit string (hex) */
#ifdef _STR_U8
#undef _STR_U8
#endif
#define _STR_U8(u8) _z80dasm_u8((uint8_t)(u8),out_cb,user_data);
/* output number number as unsigned 16-bit string (hex) */
#ifdef _STR_U16
#undef _STR_U16
#endif
#define _STR_U16(u16) _z80dasm_u16((uint16_t)(u16),out_cb,user_data);
/* (HL)/(IX+d)/(IX+d) */
#ifdef _M
#undef _M
#endif
#define _M() _STR(r[6]);if(pre){_FETCH_I8(d);_STR_D8(d);_CHR(')');}
/* same as _M, but with given offset byte */
#ifdef _Md
#undef _Md
#endif
#define _Md(d) _STR(r[6]);if(pre){_STR_D8(d);_CHR(')');}
/* (HL)/(IX+d)/(IX+d) or r */
#ifdef _MR
#undef _MR
#endif
#define _MR(i) if(i==6){_M();}else{_STR(r[i]);}
/* same as _MR, but with given offset byte */
#ifdef _MRd
#undef _MRd
#endif
#define _MRd(i,d) _STR(r[i]);if(i==6 && pre){_STR_D8(d);_CHR(')');}
/* output 16-bit immediate operand */
#ifdef _IMM16
#undef _IMM16
#endif
#define _IMM16() _FETCH_U16(u16); _STR_U16(u16);
/* output 8-bit immediate operand */
#ifdef _IMM8
#undef _IMM8
#endif
#define _IMM8() _FETCH_U8(u8); _STR_U8(u8);

static const char* _z80dasm_r[8] = { "B", "C", "D", "E", "H", "L", "(HL)", "A" };
static const char* _z80dasm_rix[8] = { "B", "C", "D", "E", "IXH", "IXL", "(IX", "A" };
static const char* _z80dasm_riy[8] = { "B", "C", "D", "E", "IYH", "IYL", "(IY", "A" };
static const char* _z80dasm_rp[4] = { "BC", "DE", "HL", "SP" };
static const char* _z80dasm_rpix[4] = { "BC", "DE", "IX", "SP" };
static const char* _z80dasm_rpiy[4] = { "BC", "DE", "IY", "SP" };
static const char* _z80dasm_rp2[4] = { "BC", "DE", "HL", "AF"};
static const char* _z80dasm_rp2ix[4] = { "BC", "DE", "IX", "AF"};
static const char* _z80dasm_rp2iy[4] = { "BC", "DE", "IY", "AF"};
static const char* _z80dasm_cc[8] = { "NZ", "Z", "NC", "C", "PO", "PE", "P", "M" };
static const char* _z80dasm_alu[8] = { "ADD A,", "ADC A,", "SUB ", "SBC A,", "AND ", "XOR ", "OR ", "CP " };
static const char* _z80dasm_rot[8] = { "RLC ", "RRC ", "RL ", "RR ", "SLA ", "SRA ", "SLL ", "SRL " };
static const char* _z80dasm_x0z7[8] = { "RLCA", "RRCA", "RLA", "RRA", "DAA", "CPL", "SCF", "CCF" };
static const char* _z80dasm_edx1z7[8] = { "LD I,A", "LD R,A", "LD A,I", "LD A,R", "RRD", "RLD", "NOP (ED)", "NOP (ED)" };
static const char* _z80dasm_im[8] = { "0", "0", "1", "2", "0", "0", "1", "2" };
static const char* _z80dasm_bli[4][4] = {
    { "LDI", "CPI", "INI", "OUTI" },
    { "LDD", "CPD", "IND", "OUTD" },
    { "LDIR", "CPIR", "INIR", "OTIR" },
    { "LDDR", "CPDR", "INDR", "OTDR" }
};
static const char* _z80dasm_oct = "01234567";
static const char* _z80dasm_dec = "0123456789";
static const char* _z80dasm_hex = "0123456789ABCDEF";

/* output a string */
static void _z80dasm_str(const char* str, z80dasm_output_t out_cb, void* user_data) {
    if (out_cb) {
        char c;
        while (0 != (c = *str++)) {
            out_cb(c, user_data);
        }
    }
}

/* output a signed 8-bit offset value as decimal string */
static void _z80dasm_d8(int8_t val, z80dasm_output_t out_cb, void* user_data) {
    if (out_cb) {
        if (val < 0) {
            out_cb('-', user_data);
            val = -val;
        }
        else {
            out_cb('+', user_data);
        }
        if (val >= 100) {
            out_cb('1', user_data);
            val -= 100;
        }
        if ((val/10) != 0) {
            out_cb(_z80dasm_dec[val/10], user_data);
        }
        out_cb(_z80dasm_dec[val%10], user_data);
    }
}

/* output an unsigned 8-bit value as hex string */
static void _z80dasm_u8(uint8_t val, z80dasm_output_t out_cb, void* user_data) {
    if (out_cb) {
        for (int i = 1; i >= 0; i--) {
            out_cb(_z80dasm_hex[(val>>(i*4)) & 0xF], user_data);
        }
        out_cb('h',user_data);
    }
}

/* output an unsigned 16-bit value as hex string */
static void _z80dasm_u16(uint16_t val, z80dasm_output_t out_cb, void* user_data) {
    if (out_cb) {
        for (int i = 3; i >= 0; i--) {
            out_cb(_z80dasm_hex[(val>>(i*4)) & 0xF], user_data);
        }
        out_cb('h',user_data);
    }
}

/* main disassembler function */
uint16_t z80dasm_op(uint16_t pc, z80dasm_input_t in_cb, z80dasm_output_t out_cb, void* user_data) {
    CHIPS_ASSERT(in_cb);
    uint8_t op = 0, pre = 0, u8 = 0;
    int8_t d = 0;
    uint16_t u16 = 0;
    const char** cc = _z80dasm_cc;
    const char** alu = _z80dasm_alu;
    const char** r = _z80dasm_r;
    const char** rp = _z80dasm_rp;
    const char** rp2 = _z80dasm_rp2;

    /* fetch the first instruction byte */
    _FETCH_U8(op);
    /* prefixed op? */
    if ((0xFD == op) || (0xDD == op)) {
        pre = op;
        _FETCH_U8(op);
        if (op == 0xED) {
            pre = 0; /* an ED following a prefix cancels the prefix */
        }
        /* if prefixed op, use register tables that replace HL with IX/IY */
        if (pre == 0xDD) {
            r  = _z80dasm_rix;
            rp = _z80dasm_rpix;
            rp2 = _z80dasm_rp2ix;
        }
        else if (pre == 0xFD) {
            r  = _z80dasm_riy;
            rp = _z80dasm_rpiy;
            rp2 = _z80dasm_rp2iy;
        }
    }
    
    /* parse the opcode */
    uint8_t x = (op >> 6) & 3;
    uint8_t y = (op >> 3) & 7;
    uint8_t z = op & 7;
    uint8_t p = y >> 1;
    uint8_t q = y & 1;
    if (x == 1) {
        /* 8-bit load block */
        if (y == 6) {
            if (z == 6) {
                /* special case LD (HL),(HL) */
                _STR("HALT");
            }
            else {
                /* LD (HL),r; LD (IX+d),r; LD (IY+d),r */
                _STR("LD "); _M(); _CHR(',');
                if (pre && ((z == 4) || (z == 5))) {
                    /* special case LD (IX+d),L/H (don't use IXL/IXH) */
                    _STR(_z80dasm_r[z]);
                }
                else {
                    _STR(r[z]);
                }
            }
        }
        else if (z == 6) {
            /* LD r,(HL); LD r,(IX+d); LD r,(IY+d) */
            _STR("LD ");
            if (pre && ((y == 4) || (y == 5))) {
                /* special case LD H/L,(IX+d) (don't use IXL/IXH) */
                _STR(_z80dasm_r[y]);
            }
            else {
                _STR(r[y]);
            }
            _CHR(','); _M();
        }
        else {
            /* regular LD r,s */
            _STR("LD "); _STR(r[y]); _CHR(','); _STR(r[z]);
        }
    }
    else if (x == 2) {
        /* 8-bit ALU block */
        _STR(alu[y]); _MR(z);
    }
    else if (x == 0) {
        switch (z) {
            case 0:
                switch (y) {
                    case 0: _STR("NOP"); break;
                    case 1: _STR("EX AF,AF'"); break;
                    case 2: _STR("DJNZ "); _FETCH_I8(d); _STR_U16(pc+d); break;
                    case 3: _STR("JR "); _FETCH_I8(d); _STR_U16(pc+d); break;
                    default: _STR("JR "); _STR(cc[y-4]); _CHR(','); _FETCH_I8(d); _STR_U16(pc+d); break;
                }
                break;
            case 1:
                if (q == 0) {
                    _STR("LD "); _STR(rp[p]); _CHR(','); _IMM16();
                }
                else {
                    _STR("ADD "); _STR(rp[2]); _CHR(','); _STR(rp[p]);
                }
                break;
            case 2: 
                {
                    _STR("LD ");
                    switch (y) {
                        case 0: _STR("(BC),A"); break;
                        case 1: _STR("A,(BC)"); break;
                        case 2: _STR("(DE),A"); break;
                        case 3: _STR("A,(DE)"); break;
                        case 4: _STR("("); _IMM16(); _STR("),"); _STR(rp[2]); break;
                        case 5: _STR(rp[2]); _STR(",("); _IMM16(); _STR(")"); break;
                        case 6: _STR("("); _IMM16(); _STR("),A"); break;
                        case 7: _STR("A,("); _IMM16(); _STR(")"); break;
                    }
                }
                break;
            case 3: _STR(q==0?"INC ":"DEC "); _STR(rp[p]); break;
            case 4: _STR("INC "); _MR(y); break;
            case 5: _STR("DEC "); _MR(y); break;
            case 6: _STR("LD "); _MR(y); _CHR(','); _IMM8(); break;
            case 7: _STR(_z80dasm_x0z7[y]); break;
        }
    }
    else {
        switch (z) {
            case 0: _STR("RET "); _STR(cc[y]); break;
            case 1:
                if (q == 0) {
                    _STR("POP "); _STR(rp2[p]);
                }
                else {
                    switch (p) {
                        case 0: _STR("RET"); break;
                        case 1: _STR("EXX"); break;
                        case 2: _STR("JP "); _CHR('('); _STR(rp[2]); _CHR(')'); break;
                        case 3: _STR("LD SP,"); _STR(rp[2]); break;
                    }
                }
                break;
            case 2: _STR("JP "); _STR(cc[y]); _CHR(','); _IMM16(); break;
            case 3:
                switch (y) {
                    case 0: _STR("JP "); _IMM16(); break;
                    case 2: _STR("OUT ("); _IMM8(); _CHR(')'); _STR(",A"); break;
                    case 3: _STR("IN A,("); _IMM8(); _CHR(')'); break;
                    case 4: _STR("EX (SP),"); _STR(rp[2]); break;
                    case 5: _STR("EX DE,HL"); break;
                    case 6: _STR("DI"); break;
                    case 7: _STR("EI"); break;
                    case 1: /* CB prefix */
                        if (pre) {
                            _FETCH_I8(d);
                        }
                        _FETCH_U8(op);
                        x = (op >> 6) & 3;
                        y = (op >> 3) & 7;
                        z = op & 7;
                        if (x == 0) {
                            /* rot and shift instructions */
                            _STR(_z80dasm_rot[y]); _MRd(z,d);
                        }
                        else {
                            /* bit instructions */
                            if (x == 1) { _STR("BIT "); }
                            else if (x == 2) { _STR("RES "); }
                            else { _STR("SET "); }
                            _CHR(_z80dasm_oct[y]);
                            if (pre) {
                                _CHR(','); _Md(d);
                            }
                            if (!pre || (z != 6)) {
                                _CHR(','); _STR(r[z]);
                            }
                        }
                        break;
                }
                break;
            case 4: _STR("CALL "); _STR(cc[y]); _CHR(','); _IMM16(); break;
            case 5: 
                if (q == 0) {
                    _STR("PUSH "); _STR(rp2[p]);
                }
                else {
                    switch (p) {
                        case 0: _STR("CALL "); _IMM16(); break;
                        case 1: _STR("DBL PREFIX"); break;
                        case 3: _STR("DBL PREFIX"); break;
                        case 2: /* ED prefix */
                            _FETCH_U8(op);
                            x = (op >> 6) & 3;
                            y = (op >> 3) & 7;
                            z = op & 7;
                            p = y >> 1;
                            q = y & 1;
                            if ((x == 0) || (x == 3)) {
                                _STR("NOP (ED)");
                            }
                            else if (x == 2) {
                                if ((y >= 4) && (z <= 3)) {
                                    /* block instructions */
                                    _STR(_z80dasm_bli[y-4][z]);
                                }
                                else {
                                    _STR("NOP (ED)");
                                }
                            }
                            else {
                                switch (z) {
                                    case 0: _STR("IN "); if(y!=6){_STR(r[y]);_CHR(',');} _STR("(C)"); break;
                                    case 1: _STR("OUT (C),"); _STR(y==6?"0":r[y]); break;
                                    case 2: _STR(q==0?"SBC":"ADC"); _STR(" HL,"); _STR(rp[p]); break;
                                    case 3:
                                        _STR("LD ");
                                        if (q == 0) {
                                            _CHR('('); _IMM16(); _STR("),"); _STR(rp[p]);
                                        }
                                        else {
                                            _STR(rp[p]); _STR(",("); _IMM16(); _CHR(')');
                                        }
                                        break;
                                    case 4: _STR("NEG"); break;
                                    case 5: _STR(y==1?"RETI":"RETN"); break;
                                    case 6: _STR("IM "); _STR(_z80dasm_im[y]); break;
                                    case 7: _STR(_z80dasm_edx1z7[y]); break;
                                }
                            }
                            break;
                    }
                }
                break;
            case 6: _STR(alu[y]); _IMM8(); break; /* ALU n */
            case 7: _STR("RST "); _STR_U8(y*8); break;
        }
    }
    return pc;
}

#undef _FETCH_U8
#undef _FETCH_I8
#undef _FETCH_U16
#undef _CHR
#undef _STR
#undef _STR_D8
#undef _STR_U8
#undef _STR_U16
#undef _M
#undef _Md
#undef _MR
#undef _MRd
#undef _IMM16
#undef _IMM8
#endif /* CHIPS_UTIL_IMPL */
