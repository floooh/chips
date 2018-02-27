#pragma once
/*#
    # m6526.h

    MOS Technology 6526 Complex Interface Adapter (CIA)

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

    ************************************
    *           +-----------+          *
    *    CS --->|           |<--- FLAG *
    *    RW --->|           |---> PC   *
    *   RES --->|           |---> SP   *
    *   IRQ <---|           |<--- TOD  *
    *           |           |<--- CNT  *
    *           |           |          *
    *   RS0 --->|   M6526   |<--> PA0  *
    *   RS1 --->|           |...       *
    *   RS2 --->|           |<--> PA7  *
    *   RS3 --->|           |          *
    *           |           |<--> PB0  *
    *   DB0 --->|           |...       *
    *        ...|           |<--> PB7  *
    *   DB7 --->|           |          *
    *           +-----------+          *
    ************************************
    
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

/* register select same as lower 4 shared address bus bits */
#define M6526_RS0   (1ULL<<0)
#define M6526_RS1   (1ULL<<1)
#define M6526_RS2   (1ULL<<2)
#define M6526_RS3   (1ULL<<3)

/* data bus pins shared with CPU */
#define M6526_D0    (1ULL<<16)
#define M6526_D1    (1ULL<<17)
#define M6526_D2    (1ULL<<18)
#define M6526_D3    (1ULL<<19)
#define M6526_D4    (1ULL<<20)
#define M6526_D5    (1ULL<<21)
#define M6526_D6    (1ULL<<22)
#define M6526_D7    (1ULL<<23)

/* control pins shared with CPU */
#define M6526_RW    (1ULL<<24)      /* same as M6502_RW */
#define M6522_IRQ   (1ULL<<26)      /* same as M6502_IRQ */

/* chip-specific control pins */
#define M6526_CS    (1ULL<<40)
#define M6526_FLAG  (1ULL<<41)
#define M6526_PC    (1ULL<<42)
#define M6526_SP    (1ULL<<43)
#define M6526_TOD   (1ULL<<44)
#define M6526_CNT   (1ULL<<45)

/* port A in/out pins */
#define M6526_PA0   (1ULL<<48)
#define M6526_PA1   (1ULL<<49)
#define M6526_PA2   (1ULL<<50)
#define M6526_PA3   (1ULL<<51)
#define M6526_PA4   (1ULL<<52)
#define M6526_PA5   (1ULL<<53)
#define M6526_PA6   (1ULL<<54)
#define M6526_PA7   (1ULL<<55)

/* port B in/out pins */
#define M6526_PB0   (1ULL<<56)
#define M6526_PB1   (1ULL<<57)
#define M6526_PB2   (1ULL<<58)
#define M6526_PB3   (1ULL<<59)
#define M6526_PB4   (1ULL<<60)
#define M6526_PB5   (1ULL<<61)
#define M6526_PB6   (1ULL<<62)
#define M6526_PB7   (1ULL<<63)

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

#endif /* CHIPS_IMPL */

#ifdef __cplusplus
} /* extern "C" */
#endif
