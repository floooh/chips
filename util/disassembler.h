#pragma once
/*#
    # disassembler.h

    A generic disassembler convenience wrapper around the lower level
    m6502dasm.h and z80dasm.h headers.

    Do this:
    ~~~C
    #define CHIPS_UTIL_IMPL
    ~~~
    before you include this file in *one* C or C++ file to create the
    implementation.

    Optionally provide the following macros with your own implementation

    Select the supported CPUs with the following macros (define one
    or the other, but not both):

    UI_DBG_USE_Z80
    UI_DBG_USE_M6502

    ~~~C
    CHIPS_ASSERT(c)
    ~~~
        your own assert macro (default: assert(c))

    Include one of the following headers (depending on the selected cpu) before
    including the disassembler.h implementation:

        m6502dasm.h
        z80dasm.h

    ## Usage

    TODO

    ## zlib/libpng license

    Copyright (c) 2023 Andre Weissflog
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
*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#if !defined(UI_DBG_USE_Z80) && !defined(UI_DBG_USE_M6502)
#error "please define UI_DBG_USE_Z80 or UI_DBG_USE_M6502"
#endif

#ifdef __cplusplus
extern "C" {
#endif

// callback for checking if an address is known to be the start of an instruction
typedef bool (*dasm_known_op_t)(uint16_t addr, void* user_data);
// callback for reading a byte at address
typedef uint8_t (*dasm_read_byte_t)(uint16_t addr, void* user_data);
// a (computer-system-specific) pattern matcher callback for detecting and skipping trailing inline data
typedef uint16_t (*dasm_next_pc_t)(uint16_t op_addr, uint16_t op_length, dasm_read_byte_t read_fn, void* user_data);

#define DASM_LINE_TYPE_START        (0)     // first input only: starts disassembling a new instruction
#define DASM_LINE_TYPE_INSTRUCTION  (1)     // very likely an instruction
#define DASM_LINE_TYPE_INLINE_DATA  (2)     // very likely data inlined into code
#define DASM_LINE_TYPE_UNKNOWN      (3)     // maybe code, maybe data

// a disassembled 'line'
#define DASM_LINE_MAX_BYTES (8)
#define DASM_LINE_MAX_CHARS (64)
typedef struct dasm_line_t {
    uint16_t type;              // DASM_LINE_TYPE_xxx
    uint16_t addr;
    uint16_t next_pc;
    uint8_t num_bytes;
    uint8_t num_chars;
    uint8_t bytes[DASM_LINE_MAX_BYTES];
    uint8_t chars[DASM_LINE_MAX_CHARS];
} dasm_line_t;

// dasm state
typedef struct dasm_funcs_t {
    dasm_known_op_t knownop_fn;
    dasm_read_byte_t readbyte_fn;
    dasm_next_pc_t nextpc_fn;
    void* userdata;
} dasm_funcs_t;

// call first with a inout_line { .type=START, .addr=start_addr }, and then with the last output line until result == true
bool dasm_disasm(uint16_t pc, dasm_line_t* inout_line);

#ifdef __cplusplus
} /* extern "C" */
#endif

/*-- IMPLEMENTATION ----------------------------------------------------------*/
#ifdef CHIPS_UI_IMPL
#include <string.h>
#ifndef CHIPS_ASSERT
    #include <assert.h>
    #define CHIPS_ASSERT(c) assert(c)
#endif

bool dasm_disasm(dasm_funcs_t* funcs, dasm_line_t* inout_line) {
    CHIPS_ASSERT(funcs);
    CHIPS_ASSERT(func->knownop_fn);
    CHIPS_ASSERT(func->readbyte_fn);
    CHIPS_ASSERT(func->nextpc_fn);
    CHIPS_ASSERT(inout_line);
    dasm_line_t* l = inout_line;
    l->num_bytes = 0;
    l->num_chars = 0;
    l->chars = 0;
    if (l->type == DASM_LINE_TYPE_START) {
        if (funcs->knownop_fn(l->addr, funcs->userdata)) {
            // we're starting at a known instruction
            l->type = DASM_LINE_TYPE_INSTRUCTION;
            // FIXME:
            //  - disassemble instruction
            //  - call nextpc to check for inline bytes:
            //      - yes: return false, to gather inline bytes in next call, set nextpc!
            //      - no: just return true
        } else {
            // not a known op, return a "line" of unknown bytes
            l->type = DASM_LINE_TYPE_UNKNOWN;
            // read up to max bytes or a known instruction start and return
            static const char* hex_digits = "0123456789ABCDEF";
            while (!funcs->knownop_fn(l->addr + l->num_bytes, funcs->userdata) && (l->num_bytes < DASM_LINE_MAX_BYTES)) {
                l->bytes[l->num_bytes++] = funcs->readbyte_fn(l->addr + l->num_bytes, funcs->userdata);
            }
            return true;
        }
    } else {
        // gather the next chunk of inline bytes
        CHIPS_ASSERT((l->type == DASM_LINE_TYPE_INSTRUCTION) || (l->type == DASM_LINE_TYPE_INLINE_DATA));
        l->type = DASM_LINE_TYPE_INLINE_DATA;
        // NOTE: addr may wrap around
        while ((l->addr != l->next_pc) && (l->num_bytes < DASM_LINE_MAX_BYTES)) {
            l->bytes[l->num_bytes++] = funcs->readbyte_fn(l->addr + l->num_bytes, funcs->userdata);
        }
        // if more inline data follows, return false
        return l->addr != l->next_pc;
    }
}
#endif