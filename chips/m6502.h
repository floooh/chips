#pragma once
/*
    m6502.h   -- M6502 CPU emulator

    Do this:
        #define CHIPS_IMPL
    before you include this file in *one* C or C++ file to create the 
    implementation.

    Optionally provide the following macros with your own implementation
    
        CHIPS_ASSERT(c)     -- your own assert macro (default: assert(c))

    EMULATED PINS:

             +-----------+
      IRQ -->|           |--> A0
      NMI -->|           |...
       RW <--|           |--> A15
     SYNC <--|   m6502   |
             |           |--> D0
             |           |...
             |           |--> D7
             +-----------+
    
    FIXME: documentation
    
    LICENSE:

    MIT License

    Copyright (c) 2017 Andre Weissflog

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* tick callback function typedef */
typedef uint64_t (*m6502_tick_t)(uint64_t pins);

/* address lines */
#define M6502_A0  (1ULL<<0)
#define M6502_A1  (1ULL<<1)
#define M6502_A2  (1ULL<<2)
#define M6502_A3  (1ULL<<3)
#define M6502_A4  (1ULL<<4)
#define M6502_A5  (1ULL<<5)
#define M6502_A6  (1ULL<<6)
#define M6502_A7  (1ULL<<7)
#define M6502_A8  (1ULL<<8)
#define M6502_A9  (1ULL<<9)
#define M6502_A10 (1ULL<<10)
#define M6502_A11 (1ULL<<11)
#define M6502_A12 (1ULL<<12)
#define M6502_A13 (1ULL<<13)
#define M6502_A14 (1ULL<<14)
#define M6502_A15 (1ULL<<15)

/*--- data lines ------*/
#define M6502_D0  (1ULL<<16)
#define M6502_D1  (1ULL<<17)
#define M6502_D2  (1ULL<<18)
#define M6502_D3  (1ULL<<19)
#define M6502_D4  (1ULL<<20)
#define M6502_D5  (1ULL<<21)
#define M6502_D6  (1ULL<<22)
#define M6502_D7  (1ULL<<23)

/*--- control pins ---*/
#define M6502_RW    (1ULL<<24)
#define M6502_SYNC  (1ULL<<25)
#define M6502_IRQ   (1ULL<<26)
#define M6502_NMI   (1ULL<<27)

/* bit mask for all CPU pins */
#define M6502_PIN_MASK (0xFFFFFFFF)

/*--- status indicator flags ---*/
#define M6502_CF (1<<0)   /* carry */
#define M6502_ZF (1<<1)   /* zero */
#define M6502_IF (1<<2)   /* IRQ disable */
#define M6502_DF (1<<3)   /* decimal mode */
#define M6502_BF (1<<4)   /* BRK command */
#define M6502_XF (1<<5)   /* unused */
#define M6502_VF (1<<6)   /* overflow */
#define M6502_NF (1<<7)   /* negative */

/* M6502 CPU state */
typedef struct {
    m6502_tick_t tick;
    uint64_t PINS;
    /* 8-bit registers */
    uint8_t A,X,Y,S,P;
    /* 16-bit program counter */
    uint16_t PC;
    /* state of interrupt enable flag at the time when the interrupt is sampled,
       this is used to implement 'delayed IRQ response'
       (see: https://wiki.nesdev.com/w/index.php/CPU_interrupts)
    */
    uint8_t pi;
    /* some variations of the m6502 don't have BCD arithmetic support */
    bool bcd_supported;
    /* break out of m6502_exec() if (PINS & break_mask) */
    uint64_t break_mask;
} m6502_t;

/* initialize a new m6502 instance */
extern void m6502_init(m6502_t* cpu, m6502_tick_t tick_cb);
/* reset an existing m6502 instance */
extern void m6502_reset(m6502_t* cpu);
/* execute instruction for at least 'ticks', return number of executed ticks */
extern uint32_t m6502_exec(m6502_t* cpu, uint32_t ticks);

/* extract 16-bit address bus from 64-bit pins */
#define M6502_GET_ADDR(p) ((uint16_t)(p&0xFFFFULL))
/* merge 16-bit address bus value into 64-bit pins */
#define M6502_SET_ADDR(p,a) {p=((p&~0xFFFFULL)|((a)&0xFFFFULL));}
/* extract 8-bit data bus from 64-bit pins */
#define M6502_GET_DATA(p) ((uint8_t)((p&0xFF0000ULL)>>16))
/* merge 8-bit data bus value into 64-bit pins */
#define M6502_SET_DATA(p,d) {p=(((p)&~0xFF0000ULL)|(((d)<<16)&0xFF0000ULL));}
/* return a pin mask with control-pins, address and data bus */
#define M6502_MAKE_PINS(ctrl, addr, data) ((ctrl)|(((data)<<16)&0xFF0000ULL)|((addr)&0xFFFFULL))

/*-- IMPLEMENTATION ----------------------------------------------------------*/
#ifdef CHIPS_IMPL
#include <string.h>
#ifndef CHIPS_DEBUG
    #ifdef _DEBUG
        #define CHIPS_DEBUG
    #endif
#endif
#ifndef CHIPS_ASSERT
    #include <assert.h>
    #define CHIPS_ASSERT(c) assert(c)
#endif

/* helper macros and functions for code-generated instruction decoder */
#define _M6502_NZ(p,v) ((p&~(M6502_NF|M6502_ZF))|((v&0xFF)?(v&M6502_NF):M6502_ZF))

static inline void _m6502_adc(m6502_t* cpu, uint8_t val) {
    if (cpu->bcd_supported && (cpu->P & M6502_DF)) {
        /* decimal mode (credit goes to MAME) */
        uint8_t c = cpu->P & M6502_CF ? 1 : 0;
        cpu->P &= ~(M6502_NF|M6502_VF|M6502_ZF|M6502_CF);
        uint8_t al = (cpu->A & 0x0F) + (val & 0x0F) + c;
        if (al > 9) {
            al += 6;
        }
        uint8_t ah = (cpu->A >> 4) + (val >> 4) + (al > 0x0F);
        if (0 == (uint8_t)(cpu->A + val + c)) {
            cpu->P |= M6502_ZF;
        }
        else if (ah & 0x08) {
            cpu->P |= M6502_NF;
        }
        if (~(cpu->A^val) & (cpu->A^(ah<<4)) & 0x80) {
            cpu->P |= M6502_VF;
        }
        if (ah > 9) {
            ah += 6;
        }
        if (ah > 15) {
            cpu->P |= M6502_CF;
        }
        cpu->A = (ah<<4) | (al & 0x0F);
    }
    else {
        /* default mode */
        uint16_t sum = cpu->A + val + (cpu->P & M6502_CF ? 1:0);
        cpu->P &= ~(M6502_VF|M6502_CF);
        cpu->P = _M6502_NZ(cpu->P,sum);
        if (~(cpu->A^val) & (cpu->A^sum) & 0x80) {
            cpu->P |= M6502_VF;
        }
        if (sum & 0xFF00) {
            cpu->P |= M6502_CF;
        }
        cpu->A = sum & 0xFF;
    }    
}

static inline void _m6502_sbc(m6502_t* cpu, uint8_t val) {
    if (cpu->bcd_supported && (cpu->P & M6502_DF)) {
        /* decimal mode (credit goes to MAME) */
        uint8_t c = cpu->P & M6502_CF ? 0 : 1;
        cpu->P &= ~(M6502_NF|M6502_VF|M6502_ZF|M6502_CF);
        uint16_t diff = cpu->A - val - c;
        uint8_t al = (cpu->A & 0x0F) - (val & 0x0F) - c;
        if ((int8_t)al < 0) {
            al -= 6;
        }
        uint8_t ah = (cpu->A>>4) - (val>>4) - ((int8_t)al < 0);
        if (0 == (uint8_t)diff) {
            cpu->P |= M6502_ZF;
        }
        else if (diff & 0x80) {
            cpu->P |= M6502_NF;
        }
        if ((cpu->A^val) & (cpu->A^diff) & 0x80) {
            cpu->P |= M6502_VF;
        }
        if (!(diff & 0xFF00)) {
            cpu->P |= M6502_CF;
        }
        if (ah & 0x80) {
            ah -= 6;
        }
        cpu->A = (ah<<4) | (al & 0x0F);
    }
    else {
        /* default mode */
        uint16_t diff = cpu->A - val - (cpu->P & M6502_CF ? 0 : 1);
        cpu->P &= ~(M6502_VF|M6502_CF);
        cpu->P = _M6502_NZ(cpu->P, (uint8_t)diff);
        if ((cpu->A^val) & (cpu->A^diff) & 0x80) {
            cpu->P |= M6502_VF;
        }
        if (!(diff & 0xFF00)) {
            cpu->P |= M6502_CF;
        }
        cpu->A = diff & 0xFF;
    }
}

static inline void _m6502_arr(m6502_t* cpu) {
    /* undocumented, unreliable ARR instruction, but this is tested
       by the Wolfgang Lorenz C64 test suite
       implementation taken from MAME
    */
    if (cpu->bcd_supported && (cpu->P & M6502_DF)) {
        bool c = cpu->P & M6502_CF;
        cpu->P &= ~(M6502_NF|M6502_VF|M6502_ZF|M6502_CF);
        uint8_t a = cpu->A>>1;
        if (c) {
            a |= 0x80;
        }
        cpu->P = _M6502_NZ(cpu->P,a);
        if ((a ^ cpu->A) & 0x40) {
            cpu->P |= M6502_VF;
        }
        if ((cpu->A & 0xF) >= 5) {
            a = ((a + 6) & 0xF) | (a & 0xF0);
        }
        if ((cpu->A & 0xF0) >= 0x50) {
            a += 0x60;
            cpu->P |= M6502_CF;
        }
        cpu->A = a;
    }
    else {
        bool c = cpu->P & M6502_CF;
        cpu->P &= ~(M6502_NF|M6502_VF|M6502_ZF|M6502_CF);
        cpu->A >>= 1;
        if (c) {
            cpu->A |= 0x80;
        }
        cpu->P = _M6502_NZ(cpu->P,cpu->A);
        if (cpu->A & 0x40) {
            cpu->P |= M6502_VF|M6502_CF;
        }
        if (cpu->A & 0x20) {
            cpu->P ^= M6502_VF;
        }
    }
}

#undef _M6502_NZ

#include "_m6502_decoder.h"

void m6502_init(m6502_t* c, m6502_tick_t tick_cb) {
    CHIPS_ASSERT(c);
    CHIPS_ASSERT(tick_cb);
    memset(c, 0, sizeof(*c));
    c->tick = tick_cb;
    c->PINS = M6502_RW;
    c->P = M6502_IF|M6502_XF;
    c->S = 0xFD;
    c->bcd_supported = true;
}

void m6502_reset(m6502_t* c) {
    CHIPS_ASSERT(c);
    c->P = M6502_IF|M6502_XF;
    c->S = 0xFD;
    c->PINS = M6502_RW;
    /* load reset vector from 0xFFFD into PC */
    uint8_t l = M6502_GET_DATA(c->tick(M6502_MAKE_PINS(M6502_RW, 0xFFFC, 0x00)));
    uint8_t h = M6502_GET_DATA(c->tick(M6502_MAKE_PINS(M6502_RW, 0xFFFD, 0x00)));
    c->PC = (h<<8)|l;
}
#endif /* CHIPS_IMPL */

#ifdef __cplusplus
} /* extern "C" */
#endif
