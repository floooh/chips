#pragma once
/*#
    # m6581.h

    MOS Technology 6581 emulator (aka SID)

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

    ## Emulated Pins

    ***********************************
    *           +-----------+         *
    *    CS --->|           |<--- A0  *
    *    RW --->|           |...      *
    *           |           |<--- A4  *
    *           |   m6581   |         *
    *           |           |<--> D0  *
    *           |           |...      *
    *           |           |<--> D7  *
    *           |           |         *
    *           +-----------+         *§
    ***********************************

    TODO: Documentation

    ## MIT License

    Copyright (c) 2018 Andre Weissflog

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
#*/
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* address bus pins A0..A4 */
#define M6581_A0    (1ULL<<0)
#define M6581_A1    (1ULL<<1)
#define M6581_A2    (1ULL<<2)
#define M6581_A3    (1ULL<<3)

/* data bus pins D0..D7 */
#define M6581_D0    (1ULL<<16)
#define M6581_D1    (1ULL<<17)
#define M6581_D2    (1ULL<<18)
#define M6581_D3    (1ULL<<19)
#define M6581_D4    (1ULL<<20)
#define M6581_D5    (1ULL<<21)
#define M6581_D6    (1ULL<<22)
#define M6581_D7    (1ULL<<23)

/* shared control pins */
#define M6581_RW    (1ULL<<24)      /* same as M6502_RW */

/* chip-specific pins */
#define M6581_CS    (1ULL<<40)      /* chip-select */

/* setup parameters for m6581_init() */
typedef struct {
    int tick_hz;        /* frequency at which m6581_tick() will be called in Hz */
    int sound_hz;       /* sound sample frequency */
    float magnitude;    /* output sample magnitude (0=silence to 1=max volume) */
} m6581_desc_t;

/* m6581 instance state */
typedef struct {

} m6581_t;

/* initialize a new m6581_t instance */
extern void m6581_init(m6581_t* sid, m6581_desc_t* desc);
/* reset a m6581_t instance */
extern void m6581_reset(m6581_t* sid);
/* read/write m6581_t registers */
extern uint64_t m6581_iorq(m6581_t* sid, uint64_t pins);
/* tick a m6581_t instance, returns true when new sample is ready */
extern bool m6581_tick(m6581_t* sid);

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

/* extract 8-bit data bus from 64-bit pins */
#define M6581_GET_DATA(p) ((uint8_t)((p&0xFF0000ULL)>>16))
/* merge 8-bit data bus value into 64-bit pins */
#define M6581_SET_DATA(p,d) {p=(((p)&~0xFF0000ULL)|(((d)<<16)&0xFF0000ULL));}

void m6581_init(m6581_t* sid, m6581_desc_t* desc) {
    CHIPS_ASSERT(sid && desc);
    CHIPS_ASSERT(desc->tick_hz > 0);
    CHIPS_ASSERT(desc->sound_hz > 0);
    memset(sid, 0, sizeof(*sid));
    // FIXME
}

void m6581_reset(m6581_t* sid) {
    CHIPS_ASSERT(sid);
    // FIXME
}

bool m6581_tick(m6581_t* sid) {
    CHIPS_ASSERT(sid);
    // FIXME
    return false;
}

uint64_t m6581_iorq(m6581_t* sid, uint64_t pins) {
    CHIPS_ASSERT(sid);
    // FIXME
    return pins;
}
#endif /* CHIPS_IMPL */

#ifdef __cplusplus
} /* extern "C" */
#endif
