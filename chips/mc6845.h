#pragma once
/*#
    # mc6845.h

    Header-only emulator for the Motorola MC6847 CRT controller.

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

    ## Emulated Pins
    **********************************
    *           +----------+         *
    *           |          |         *
    *     CS -->|          |--> MA0  *
    *     RS -->|          |...      *
    *     RW -->|          |--> MA13 *
    *           |          |         *
    *     DE <--|          |--> RA0  *
    *     VS <--|  MC6845  |...      *
    *     HS <--|          |--> RA4  *
    * CURSOR <--|          |         *
    *           |          |--> D0   *
    *  LPSTB -->|          |...      *
    *  RESET -->|          |--> D7   *
    *           |          |         *
    *           +----------+         *
    **********************************

    * CS:   chips select (must be set to read or write registers)
    * RS:   register select ()
    
    ## Not Emulated:

    * the E pin (enable)
    * the RESET pin, call mc6845_reset() instead

    ## MIT License

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
#*/
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* the address output pins share the same pin locations as the system address bus
   but they are only set in the output pins mask of mc6845_tick()
*/
#define MC6845_MA0  (1ULL<<0)
#define MC6845_MA1  (1ULL<<1)
#define MC6845_MA2  (1ULL<<2)
#define MC6845_MA3  (1ULL<<3)
#define MC6845_MA4  (1ULL<<4)
#define MC6845_MA5  (1ULL<<5)
#define MC6845_MA6  (1ULL<<6)
#define MC6845_MA7  (1ULL<<7)
#define MC6845_MA8  (1ULL<<8)
#define MC6845_MA9  (1ULL<<9)
#define MC6845_MA10 (1ULL<<10)
#define MC6845_MA11 (1ULL<<11)
#define MC6845_MA12 (1ULL<<12)
#define MC6845_MA13 (1ULL<<13)

/* data bus pins */
#define MC6845_D0   (1ULL<<16)
#define MC6845_D1   (1ULL<<17)
#define MC6845_D2   (1ULL<<18)
#define MC6845_D3   (1ULL<<19)
#define MC6845_D4   (1ULL<<20)
#define MC6845_D5   (1ULL<<21)
#define MC6845_D6   (1ULL<<22)
#define MC6845_D7   (1ULL<<23)

/* control pins */
#define MC6845_CS       (1ULL<<40)     /* chip select */
#define MC6845_RS       (1ULL<<41)     /* register select (active: data register, inactive: address register) */
#define MC6845_RW       (1ULL<<42)     /* read/write (active: write, inactive: read) */
#define MC6845_LPSTB    (1ULL<<43)     /* light pen strobe */

/* display status pins */
#define MC6845_DE       (1ULL<<44)     /* display enable */
#define MC6845_VS       (1ULL<<45)     /* vsync active */
#define MC6845_HS       (1ULL<<46)     /* hsync active */

/* row-address output pins */
#define MC6845_RA0      (1ULL<<48)
#define MC6845_RA1      (1ULL<<49)
#define MC6845_RA2      (1ULL<<50)
#define MC6845_RA3      (1ULL<<51)
#define MC6845_RA4      (1ULL<<52)

/* register identifiers */
#define MC6845_REG_HTOTAL           (0)
#define MC6845_REG_HDISPLAYED       (1)
#define MC6845_REG_HSYNCPOS         (2)
#define MC6845_REG_HSYNCWIDTH       (3)
#define MC6845_REG_VTOTAL           (4)
#define MC6845_REG_VTOTALADJUST     (5)
#define MC6845_REG_VDISPLAYED       (6)
#define MC6845_REG_VSYNCPOS         (7)
#define MC6845_REG_INTERLACEMODE    (8)
#define MC6845_REG_MAXSCANLINEADDR  (9)
#define MC6845_REG_CURSORSTART      (10)
#define MC6845_REG_CURSOREND        (11)
#define MC6845_REG_STARTADDRHI      (12)
#define MC6845_REG_STARTADDRLO      (13)
#define MC6845_REG_CURSORHI         (14)
#define MC6845_REG_CURSORLO         (15)
#define MC6845_REG_LIGHTPENHI       (16)
#define MC6845_REG_LIGHTPENLO       (17)
#define MC6845_NUM_REGS             (18)

/* register readable/writable bits */
#define MC6845_READABLE (0<<0)
#define MC6845_WRITABLE (1<<1)

/* chip subtypes */
typedef enum {
    MC6845_TYPE_UM6845,
    MC6845_TYPE_UM6845R,
    MC6845_TYPE_MC6845,
} mc6845_type_t;

/* mc6845 state */
typedef struct {
    mc6845_type_t type;
    uint8_t sel;                    /* 5 bits, currently selected register */
    uint8_t reg[MC6845_NUM_REGS];   /* 18 registers */
    uint8_t mask[MC6845_NUM_REGS];  /* masks of register bit-widths */
    uint8_t rw[MC6845_NUM_REGS];    /* register readable/writable */
} mc6845_t;

/* helper macros to extract address and data values from pin mask */

/* extract 13-bit address bus from 64-bit pins */
#define MC6845_GET_ADDR(p) ((uint16_t)(p&0xFFFFULL))
/* merge 13-bit address bus value into 64-bit pins */
#define MC6845_SET_ADDR(p,a) {p=((p&~0xFFFFULL)|((a)&0xFFFFULL));}
/* extract 8-bit data bus from 64-bit pins */
#define MC6845_GET_DATA(p) ((uint8_t)((p&0xFF0000ULL)>>16))
/* merge 8-bit data bus value into 64-bit pins */
#define MC6845_SET_DATA(p,d) {p=((p&~0xFF0000ULL)|(((d)<<16)&0xFF0000ULL));}
/* get 5-bit row-address from 64-bit pins */
#define MC6845_GET_RA(p) ((uint8_t)((p&0x00FF000000000000ULL)>>48))
/* merge 5-bit row-address into 64-bit pins */
#define MC6845_SET_RA(p,a) {p=((p&~0x00FF000000000000ULL)|(((a<<48)&)0x00FF000000000000ULL));}

/* initialize a new mc6845 instance */
extern void mc6845_init(mc6845_t* mc6845, mc6845_type_t type);
/* reset an existing mc6845 instance */
extern void mc6845_reset(mc6845_t* mc6845);
/* perform an IO request */
extern uint64_t mc6845_iorq(mc6845_t* mc6845, uint64_t pins);
/* tick the mc6845, the returned pin mask overwrittes addr bus pins with MA0..MA13! */
extern uint64_t mc6845_tick(mc6845_t* mc6845);

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

void mc6845_init(mc6845_t* mc6845, mc6845_type_t type) {
    CHIPS_ASSERT(mc6845);
    memset(mc6845, 0, sizeof(mc6845_t));
    mc6845->type = type;
}

void mc6845_reset(mc6845_t* mc6845) {
    /* reset behaviour:
        - all counters in the CRTC are cleared and the device stops 
          the display operation
        - all the outputs are driven low
        - the control registers in the CRTC are not affected and
          remain unchanged
        - the CRTC resumes the display operation immediately after
          the release of RESET. DE is not active until after 
          the first VS pulse occurs.
    */
    // FIXME
}

#ifdef __cplusplus
} /* extern "C" */
#endif