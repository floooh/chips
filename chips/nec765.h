#pragma once
/*#
    # nec765.h

    NEC uPD765 floppy disc controller.

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

/* data bus pins (in/out) */
#define NEC765_D0   (1ULL<<16)
#define NEC765_D1   (1ULL<<17)
#define NEC765_D2   (1ULL<<18)
#define NEC765_D3   (1ULL<<19)
#define NEC765_D4   (1ULL<<20)
#define NEC765_D5   (1ULL<<21)
#define NEC765_D6   (1ULL<<22)
#define NEC765_D7   (1ULL<<23)

/* control pins shared with CPU */
#define NEC765_RD   (1ULL<<27)  /* in: read data from controller */
#define NEC765_WR   (1ULL<<28)  /* in: write data to controller */

/* control pins */
#define NEC765_CS   (1ULL<<40)  /* in: chip select */
#define NEC765_A0   (1ULL<<41)  /* in: data/status register select */
#define NEC765_DACK (1ULL<<42)  /* FIXME */
#define NEC765_TC   (1ULL<<43)  /* FIXME */

/* extract 8-bit data bus from 64-bit pins */
#define NEC765_GET_DATA(p) ((uint8_t)((p&0xFF0000ULL)>>16))
/* merge 8-bit data bus value into 64-bit pins */
#define NEC765_SET_DATA(p,d) {p=((p&~0xFF0000ULL)|(((d)<<16)&0xFF0000ULL));}

/* main status register bits */
#define NEC765_STATUS_D0B   (1<<0)      /* FDD 0 busy */
#define NEC765_STATUS_D1B   (1<<1)      /* FDD 1 busy */
#define NEC765_STATUS_D2B   (1<<2)      /* FDD 2 busy */
#define NEC765_STATUS_D3B   (1<<3)      /* FDD 3 busy */
#define NEC765_STATUS_CB    (1<<4)      /* FDC busy */
#define NEC765_STATUS_EXM   (1<<5)      /* execution mode */
#define NEC765_STATUS_DIO   (1<<6)      /* direction of data transfer */
#define NEC765_STATUS_RQM   (1<<7)      /* request for master */

#ifdef __cplusplus
} /* extern "C" */
#endif

/*--- IMPLEMENTATION ---------------------------------------------------------*/
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