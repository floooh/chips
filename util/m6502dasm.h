#pragma once
/*#
    # m6502dasm.h

    A stateless MOS 6502 disassembler that doesn't call any CRT functions.
    
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
    uint16_t m6502dasm_op(uint16_t pc, m6502dasm_input_t in_cb, m6502dasm_output_t out_cb, void* user_data)
    ~~~

    pc      - the current 16-bit program counter, this is used to compute 
              absolute target addresses for relative jumps
    in_cb   - this function is called when the disassembler needs the next 
              instruction byte: uint8_t in_cb(void* user_data)
    out_cb  - (optional) this function is called when the disassembler produces a single
              ASCII character: void out_cb(char c, void* user_data)
    user_data   - a user-provided context pointer for the callbacks

    m6502dasm_op() returns the new program counter (pc), this should be
    used as input arg when calling m6502dasm_op() for the next instruction.

    NOTE that the output callback will never be called with a null character,
    you need to terminate the resulting string yourself if needed.

    Undocumented instructions are supported and are marked with a '*'.

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
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* the input callback type */
typedef uint8_t (*m6502dasm_input_t)(void* user_data);
/* the output callback type */
typedef void (*m6502dasm_output_t)(char c, void* user_data);

/* disassemble a single 6502 instruction into a stream of ASCII characters */
uint16_t m6502dasm_op(uint16_t pc, m6502dasm_input_t in_cb, m6502dasm_output_t out_cb, void* user_data);

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
#define _STR(s) _m6502dasm_str(s,out_cb,user_data);
/* output number as unsigned 8-bit string (hex) */
#ifdef _STR_U8
#undef _STR_U8
#endif
#define _STR_U8(u8) _m6502dasm_u8((uint8_t)(u8),out_cb,user_data);
/* output number number as unsigned 16-bit string (hex) */
#ifdef _STR_U16
#undef _STR_U16
#endif
#define _STR_U16(u16) _m6502dasm_u16((uint16_t)(u16),out_cb,user_data);

/* addressing modes */
#define A____    (0)     /* no addressing mode */
#define A_IMM    (1)     /* # */
#define A_ZER    (2)     /* zp */
#define A_ZPX    (3)     /* zp,X */
#define A_ZPY    (4)     /* zp,Y */
#define A_ABS    (5)     /* abs */
#define A_ABX    (6)     /* abs,X */
#define A_ABY    (7)     /* abs,Y */
#define A_IDX    (8)     /* (zp,X) */
#define A_IDY    (9)     /*(zp),Y */
#define A_JMP    (10)    /* special JMP abs */
#define A_JSR    (11)    /* special JSR abs */
#define A_BRA    (12)    /* special relative branch */
#define A_INV    (13)    /* this is an invalid instruction */

/* opcode descriptions */
static uint8_t _m6502dasm_ops[4][8][8] = {
/* cc = 00 */
{
    //---  BIT   JMP   JMP() STY   LDY   CPY   CPX
    {A____,A_JSR,A____,A____,A_IMM,A_IMM,A_IMM,A_IMM},
    {A_ZER,A_ZER,A_ZER,A_ZER,A_ZER,A_ZER,A_ZER,A_ZER},
    {A____,A____,A____,A____,A____,A____,A____,A____},
    {A_ABS,A_ABS,A_JMP,A_JMP,A_ABS,A_ABS,A_ABS,A_ABS},
    {A_BRA,A_BRA,A_BRA,A_BRA,A_BRA,A_BRA,A_BRA,A_BRA},  /* relative branches */
    {A_ZPX,A_ZPX,A_ZPX,A_ZPX,A_ZPX,A_ZPX,A_ZPX,A_ZPX},
    {A____,A____,A____,A____,A____,A____,A____,A____},
    {A_ABX,A_ABX,A_ABS,A_ABS,A_INV,A_ABX,A_ABX,A_ABX}
},
/* cc = 01 */
{
    //ORA  AND   EOR   ADC   STA   LDA   CMP   SBC
    {A_IDX,A_IDX,A_IDX,A_IDX,A_IDX,A_IDX,A_IDX,A_IDX},
    {A_ZER,A_ZER,A_ZER,A_ZER,A_ZER,A_ZER,A_ZER,A_ZER},
    {A_IMM,A_IMM,A_IMM,A_IMM,A_IMM,A_IMM,A_IMM,A_IMM},
    {A_ABS,A_ABS,A_ABS,A_ABS,A_ABS,A_ABS,A_ABS,A_ABS},
    {A_IDY,A_IDY,A_IDY,A_IDY,A_IDY,A_IDY,A_IDY,A_IDY},
    {A_ZPX,A_ZPX,A_ZPX,A_ZPX,A_ZPX,A_ZPX,A_ZPX,A_ZPX},
    {A_ABY,A_ABY,A_ABY,A_ABY,A_ABY,A_ABY,A_ABY,A_ABY},
    {A_ABX,A_ABX,A_ABX,A_ABX,A_ABX,A_ABX,A_ABX,A_ABX},
},
/* cc = 02 */
{
    //ASL  ROL   LSR   ROR   STX   LDX   DEC   INC
    {A_INV,A_INV,A_INV,A_INV,A_IMM,A_IMM,A_IMM,A_IMM},
    {A_ZER,A_ZER,A_ZER,A_ZER,A_ZER,A_ZER,A_ZER,A_ZER},
    {A____,A____,A____,A____,A____,A____,A____,A____},
    {A_ABS,A_ABS,A_ABS,A_ABS,A_ABS,A_ABS,A_ABS,A_ABS},
    {A_INV,A_INV,A_INV,A_INV,A_INV,A_INV,A_INV,A_INV},
    {A_ZPX,A_ZPX,A_ZPX,A_ZPX,A_ZPY,A_ZPY,A_ZPX,A_ZPX},
    {A____,A____,A____,A____,A____,A____,A____,A____},
    {A_ABX,A_ABX,A_ABX,A_ABX,A_INV,A_ABY,A_ABX,A_ABX},
},
/* cc = 03 */
{
    {A_IDX,A_IDX,A_IDX,A_IDX,A_IDX,A_IDX,A_IDX,A_IDX},
    {A_ZER,A_ZER,A_ZER,A_ZER,A_ZER,A_ZER,A_ZER,A_ZER},
    {A_INV,A_INV,A_INV,A_INV,A_INV,A_INV,A_INV,A_IMM},
    {A_ABS,A_ABS,A_ABS,A_ABS,A_ABS,A_ABS,A_ABS,A_ABS},
    {A_IDY,A_IDY,A_IDY,A_IDY,A_INV,A_IDY,A_IDY,A_IDY},
    {A_ZPX,A_ZPX,A_ZPX,A_ZPX,A_ZPY,A_ZPY,A_ZPX,A_ZPX},
    {A_ABY,A_ABY,A_ABY,A_ABY,A_INV,A_INV,A_ABY,A_ABY},
    {A_ABX,A_ABX,A_ABX,A_ABX,A_INV,A_ABY,A_ABX,A_ABX}
} };

static const char* _m6502dasm_hex = "0123456789ABCDEF";

/* helper function to output string */
static void _m6502dasm_str(const char* str, m6502dasm_output_t out_cb, void* user_data) {
    if (out_cb) {
        char c;
        while (0 != (c = *str++)) {
            out_cb(c, user_data);
        }
    }
}

/* helper function to output an unsigned 8-bit value as hex string */
static void _m6502dasm_u8(uint8_t val, m6502dasm_output_t out_cb, void* user_data) {
    if (out_cb) {
        out_cb('$', user_data);
        for (int i = 1; i >= 0; i--) {
            out_cb(_m6502dasm_hex[(val>>(i*4)) & 0xF], user_data);
        }
    }
}

/* helper function to output an unsigned 16-bit value as hex string */
static void _m6502dasm_u16(uint16_t val, m6502dasm_output_t out_cb, void* user_data) {
    if (out_cb) {
        out_cb('$', user_data);
        for (int i = 3; i >= 0; i--) {
            out_cb(_m6502dasm_hex[(val>>(i*4)) & 0xF], user_data);
        }
    }
}

/* main disassembler function */
uint16_t m6502dasm_op(uint16_t pc, m6502dasm_input_t in_cb, m6502dasm_output_t out_cb, void* user_data) {
    CHIPS_ASSERT(in_cb);
    uint8_t op;
    _FETCH_U8(op);
    uint8_t cc  = op & 0x03;
    uint8_t bbb = (op >> 2) & 0x07;
    uint8_t aaa = (op >> 5) & 0x07;

    /* opcode name */
    const char* n = "???";
    bool indirect = false;
    switch (cc) {
        case 0:
            switch (aaa) {
                case 0:
                    switch (bbb) {
                        case 0:  n = "BRK"; break;
                        case 2:  n = "PHP"; break;
                        case 4:  n = "BPL"; break;
                        case 6:  n = "CLC"; break;
                        default: n = "*NOP"; break;
                    }
                    break;
                case 1:
                    switch (bbb) {
                        case 0:  n = "JSR"; break;
                        case 2:  n = "PLP"; break;
                        case 4:  n = "BMI"; break;
                        case 5:  n = "*NOP"; break;
                        case 6:  n = "SEC"; break;
                        case 7:  n = "*NOP"; break;
                        default: n = "BIT"; break;
                    }
                    break;
                case 2:
                    switch (bbb) {
                        case 0:  n = "RTI"; break;
                        case 2:  n = "PHA"; break;
                        case 3:  n = "JMP"; break;
                        case 4:  n = "BVC"; break;
                        case 6:  n = "CLI"; break;
                        default: n = "*NOP"; break;
                    }
                    break;
                case 3:
                    switch (bbb) {
                        case 0:  n = "RTS"; break;
                        case 2:  n = "PLA"; break;
                        case 3:  n = "JMP"; indirect = true; break;  /* jmp () */
                        case 4:  n = "BVS"; break;
                        case 6:  n = "SEI"; break;
                        default: n = "*NOP"; break;
                    }
                    break;
                case 4:
                    switch (bbb) {
                        case 0:  n = "*NOP"; break;
                        case 2:  n = "DEY"; break;
                        case 4:  n = "BCC"; break;
                        case 6:  n = "TYA"; break;
                        default: n = "STY"; break;
                    }
                    break;
                case 5:
                    switch (bbb) {
                        case 2:  n = "TAY"; break;
                        case 4:  n = "BCS"; break;
                        case 6:  n = "CLV"; break;
                        default: n = "LDY"; break;
                    }
                    break;
                case 6:
                    switch (bbb) {
                        case 2:  n = "INY"; break;
                        case 4:  n = "BNE"; break;
                        case 5:  n = "*NOP"; break;
                        case 6:  n = "CLD"; break;
                        case 7:  n = "*NOP"; break;
                        default: n = "CPY"; break;
                    }
                    break;
                case 7:
                    switch (bbb) {
                        case 2:  n = "INX"; break;
                        case 4:  n = "BEQ"; break;
                        case 5:  n = "*NOP"; break;
                        case 6:  n = "SED"; break;
                        case 7:  n = "*NOP"; break;
                        default: n = "CPX"; break;
                    }
                    break;
            }
            break;

        case 1:
            switch (aaa) {
                case 0: n = "ORA"; break;
                case 1: n = "AND"; break; /* AND A */
                case 2: n = "EOR"; break;
                case 3: n = "ADC"; break;
                case 4:
                    switch (bbb) {
                        case 2:  n = "*NOP"; break;
                        default: n = "STA"; break;
                    }
                    break;
                case 5: n = "LDA"; break;
                case 6: n = "CMP"; break;
                case 7: n = "SBC"; break;
            }
            break;

        case 2:
            switch (aaa) {
                case 0:
                    switch (bbb) {
                        case 6:  n = "*NOP"; break;
                        default: n = "ASL"; break;
                    }
                    break;
                case 1:
                    switch (bbb) {
                        case 6:  n = "*NOP"; break;
                        default: n = "ROL"; break;
                    }
                    break;
                case 2:
                    switch (bbb) {
                        case 6:  n = "*NOP"; break;
                        default: n = "LSR"; break;
                    }
                    break;
                case 3:
                    switch (bbb) {
                        case 6:  n = "*NOP"; break;
                        default: n = "ROR"; break;
                    }
                    break;
                case 4:
                    switch (bbb) {
                        case 0:  n = "*NOP"; break;
                        case 2:  n = "TXA"; break;
                        case 6:  n = "TXS"; break;
                        default: n = "STX"; break;
                    }
                    break;
                case 5:
                    switch (bbb) {
                        case 2:  n = "TAX"; break;
                        case 6:  n = "TSX"; break;
                        default: n = "LDX"; break;
                    }
                    break;
                case 6:
                    switch (bbb) {
                        case 0:  n = "*NOP"; break;
                        case 2:  n = "DEX"; break;
                        case 6:  n = "*NOP"; break;
                        default: n = "DEC"; break;
                    }
                    break;
                case 7:
                    switch (bbb) {
                        case 0:  n = "*NOP"; break;
                        case 2:  n = "NOP"; break;
                        case 6:  n = "*NOP"; break;
                        default: n = "INC"; break;
                    }
                    break;
            }
            break;

        case 3:
            switch (aaa) {
                case 0: n = "*SLO"; break;
                case 1: n = "*RLA"; break;
                case 2: n = "*SRE"; break;
                case 3: n = "*RRA"; break;
                case 4: n = "*SAX"; break;
                case 5: n = "*LAX"; break;
                case 6: n = "*DCP"; break;
                case 7:
                    switch (bbb) {
                        case 2:  n = "*SBC"; break;
                        default: n = "*ISB"; break;
                    }
                    break;
            }
    }
    _STR(n);

    uint8_t u8; int8_t i8; uint16_t u16;
    switch (_m6502dasm_ops[cc][bbb][aaa]) {
        case A_IMM:
            _CHR(' '); _FETCH_U8(u8); _CHR('#'); _STR_U8(u8);
            break;
        case A_ZER:
            _CHR(' '); _FETCH_U8(u8); _STR_U8(u8);
            break;
        case A_ZPX:
            _CHR(' '); _FETCH_U8(u8); _STR_U8(u8); _STR(",X");
            break;
        case A_ZPY:
            _CHR(' '); _FETCH_U8(u8); _STR_U8(u8); _STR(",Y");
            break;
        case A_ABS:
        case A_JSR:
        case A_JMP:
            _CHR(' '); _FETCH_U16(u16);
            if (indirect) {
                _CHR('('); _STR_U16(u16); _CHR(')');
            }
            else {
                _STR_U16(u16);
            }
            break;
        case A_ABX:
            _CHR(' '); _FETCH_U16(u16); _STR_U16(u16); _STR(",X");
            break;
        case A_ABY:
            _CHR(' '); _FETCH_U16(u16); _STR_U16(u16); _STR(",Y");
            break;
        case A_IDX:
            _CHR(' '); _FETCH_U8(u8); _CHR('('); _STR_U8(u8); _STR(",X)");
            break;
        case A_IDY:
            _CHR(' '); _FETCH_U8(u8); _CHR('('); _STR_U8(u8); _STR("),Y");
            break;
        case A_BRA: /* relative branch, compute target address */
            _CHR(' '); _FETCH_I8(i8); _STR_U16(pc+i8);
            break;

    }
    return pc;
}

#undef _FETCH_I8
#undef _FETCH_U8
#undef _FETCH_U16
#undef _CHR
#undef _STR
#undef _STR_U8
#undef _STR_U16
#undef A____
#undef A_IMM
#undef A_ZER
#undef A_ZPX
#undef A_ZPY
#undef A_ABS
#undef A_ABX
#undef A_ABY
#undef A_IDX
#undef A_IDY
#undef A_JMP
#undef A_JSR
#undef A_INV
#undef M___
#undef M_R_
#undef M__W
#undef M_RW
#endif /* CHIPS_UTIL_IMPL */
