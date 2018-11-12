#pragma once
/*#
    # z80dasm.h

    A stateless Z80 disassembler without CRT IO functions.

    Do this:
    ~~~C
    #define CHIPS_IMPL
    ~~~
    before you include this file in *one* C or C++ file to create the 
    implementation.

    Optionally provide the following macros with your own implementation
    
    ~~~C
    CHIPS_ASSERT(c)
    ~~~
        your own assert macro (default: assert(c))

    ## Usage

        FIXME

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
extern uint16_t z80dasm_op(uint16_t pc, z80dasm_input_t in_cb, z80dasm_output_t out_cb, void* user_data);

#ifdef __cplusplus
} /* extern "C" */
#endif

/*-- IMPLEMENTATION ----------------------------------------------------------*/
#ifdef CHIPS_IMPL
#ifndef CHIPS_ASSERT
    #include <assert.h>
    #define CHIPS_ASSERT(c) assert(c)
#endif

#ifdef _FETCH
#undef _FETCH
#endif
#ifdef _CHR
#undef _CHR
#endif
#ifdef _STR
#undef _STR
#endif
#ifdef _STR_D8
#undef _STR_D8
#endif
#ifdef _STR_U8
#undef _STR_U8
#endif
#ifdef _STR_U16
#undef _STR_U16
#endif
#ifdef _M
#undef _M
#endif
/* fetch unsigned 8-bit value and track pc */
#define _FETCH_U8(v) v=in_cb(user_data);pc++;
/* fetch signed 8-bit value and track pc */
#define _FETCH_I8(v) v=(int8_t)in_cb(user_data);pc++;
/* fetch unsigned 16-bit value and track pc */
#define _FETCH_U16(v) v=in_cb(user_data);v|=in_cb(user_data)<<8;pc+=2;
/* output character */
#define _CHR(c) out_cb(c,user_data);
/* output string */
#define _STR(s) _z80dasm_str(s,out_cb,user_data);
/* output offset as signed 8-bit string (decimal) */
#define _STR_D8(d8) _z80dasm_d8((int8_t)(d8),out_cb,user_data);
/* output number as unsigned 8-bit string (hex) */
#define _STR_U8(u8) _z80dasm_u8((uint8_t)(u8),out_cb,user_data);
/* output number number as unsigned 16-bit string (hex) */
#define _STR_U16(u16) _z80dasm_u16((uint16_t)(u16),out_cb,user_data);
/* (HL)/(IX+d)/(IX+d) */
#define _M() _STR(r[6]);if(pre){_FETCH_I8(d);_STR_D8(d);_CHR(')');}
/* (HL)/(IX+d)/(IX+d) or r */
#define _MR(i) if(i==6){_M();}else{_STR(r[i]);}


/* output a string, return number of characters written */
static void _z80dasm_str(const char* str, z80dasm_output_t out_cb, void* user_data) {
    char c;
    while (0 != (c = *str++)) {
        out_cb(c, user_data);
    }
}

/* output a signed 8-bit offset value as decimal string */
static void _z80dasm_d8(int8_t val, z80dasm_output_t out_cb, void* user_data) {
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
        out_cb("0123456789"[val/10], user_data);
    }
    out_cb("0123456789"[val%10], user_data);
}

/* output an unsigned 8-bit value as hex string */
static void _z80dasm_u8(uint8_t val, z80dasm_output_t out_cb, void* user_data) {
    for (int i = 1; i >= 0; i--) {
        out_cb("0123456789ABCDEF"[(val>>(i*4)) & 0xF], user_data);
    }
    out_cb('H',user_data);
}

/* output an unsigned 16-bit value as hex string */
static void _z80dasm_u16(uint16_t val, z80dasm_output_t out_cb, void* user_data) {
    for (int i = 3; i >= 0; i--) {
        out_cb("0123456789ABCDEF"[(val>>(i*4)) & 0xF], user_data);
    }
    out_cb('H',user_data);
}

static const char* _z80dasm_r[8] = { "B", "C", "D", "E", "H", "L", "(HL)", "A" };
static const char* _z80dasm_rix[8] = { "B", "C", "D", "E", "IXH", "IXL", "(IX", "A" };
static const char* _z80dasm_riy[8] = { "B", "C", "D", "E", "IYH", "IYL", "(IY", "A" };
static const char* _z80dasm_rp[4] = { "BC", "DE", "HL", "SP" };
static const char* _z80dasm_rpix[4] = { "BC", "DE", "IX", "SP" };
static const char* _z80dasm_rpiy[4] = { "BC", "DE", "IY", "SP" };
static const char* _z80dasm_rp2[4] = { "BC", "DE", "HL", "AF"};
static const char* _z80dasm_cc[8] = { "NZ", "Z", "NC", "C", "PO", "PE", "P", "M" };
static const char* _z80dasm_alu[8] = { "ADD A,", "ADC A,", "SUB ", "SBC A,", "AND ", "XOR ", "OR ", "CP " };
static const char* _z80dasm_rot[8] = { "RLC ", "RRC ", "RL ", "RR ", "SLA ", "SRA ", "SLL ", "SRL " };
static const char* _z80dasm_x0z7[8] = { "RLCA", "RRCA", "RLA", "RRA", "DAA", "CPL", "SCF", "CCF" };
static const char* _z80dasm_im[8] = { "0", "0/1", "1", "2", "0", "0/1", "1", "2" };
static const char* _z80dasm_bli[4][4] = {
    { "LDI", "CPI", "INI", "OUTI" },
    { "LDD", "CPD", "IND", "OUTD" },
    { "LDIR", "CPIR", "INIR", "OTIR" },
    { "LDDR", "CPPR", "INDR", "OTDR" }
};

uint16_t z80dasm_op(uint16_t pc, z80dasm_input_t in_cb, z80dasm_output_t out_cb, void* user_data) {
    CHIPS_ASSERT(in_cb && out_cb);

    uint8_t op;
    uint8_t pre = 0;
    int8_t d, u8;
    uint16_t u16;

    _FETCH_U8(op);
    const char** r = _z80dasm_r;
    const char** rp = _z80dasm_rp;
    if ((0xFD == op) || (0xDD == op)) {
        /* prefixed op */
        pre = op;
        _FETCH_U8(op);
        r  = (0xDD == pre) ? _z80dasm_rix : _z80dasm_riy;
        rp = (0xDD == pre) ? _z80dasm_rpix : _z80dasm_rpiy;
    }
    const uint8_t x = (op >> 6) & 3;
    const uint8_t y = (op >> 3) & 7;
    const uint8_t z = op & 7;
    const uint8_t p = y >> 1;
    const uint8_t q = y & 1;

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
        _STR(_z80dasm_alu[y]); _STR(r[z]);
    }
    else if (x == 0) {
        switch (z) {
            case 0:
                switch (y) {
                    case 0: _STR("NOP"); break;
                    case 1: _STR("EX AF,AF'"); break;
                    case 2: _STR("DJNZ "); _FETCH_I8(d); _STR_U16(pc+d); break;
                    case 3: _STR("JR "); _FETCH_I8(d); _STR_U16(pc+d); break;
                    default: _STR("JR "); _STR(_z80dasm_cc[y-4]); _FETCH_I8(d); _STR_U16(pc+d); break;
                }
                break;
            case 1:
                if (q == 0) {
                    _STR("LD "); _STR(rp[p]); _CHR(','); _FETCH_U16(u16); _STR_U16(u16);
                }
                else {
                    _STR("ADD HL,"); _STR(rp[p]);
                }
                break;
            case 2: _STR("FIXME"); break;
            case 3: _STR("FIXME"); break;
            case 4: _STR("FIXME"); break;
            case 5: _STR("FIXME"); break;
            case 6: _STR("LD "); _MR(y); _CHR(','); _FETCH_U8(u8); _STR_U8(u8); break;
            case 7: _STR(_z80dasm_x0z7[y]); break;
        }
    }
    else {
        _STR("FIXME");
    }
    return pc;
}

#endif /* CHIPS_IMPL */
