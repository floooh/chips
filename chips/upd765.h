#pragma once
/*#
    # upd765.h

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

    ## NOT IMPLEMENTED

        Initially, only the features required by Amstrad CPC are implemented,
        this means:

        - no DMA mode
        - no interrupt-driven operation

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

/* the A0 pin is usually connected to the address bus A0 pin */
#define UPD765_A0   (1ULL<<41)  /* in: data/status register select */

/* data bus pins (in/out) */
#define UPD765_D0   (1ULL<<16)
#define UPD765_D1   (1ULL<<17)
#define UPD765_D2   (1ULL<<18)
#define UPD765_D3   (1ULL<<19)
#define UPD765_D4   (1ULL<<20)
#define UPD765_D5   (1ULL<<21)
#define UPD765_D6   (1ULL<<22)
#define UPD765_D7   (1ULL<<23)

/* control pins shared with CPU */
#define UPD765_RD   (1ULL<<27)  /* in: read data from controller */
#define UPD765_WR   (1ULL<<28)  /* in: write data to controller */

/* control pins */
#define UPD765_CS   (1ULL<<40)  /* in: chip select */

/* extract 8-bit data bus from 64-bit pins */
#define UPD765_GET_DATA(p) ((uint8_t)((p&0xFF0000ULL)>>16))
/* merge 8-bit data bus value into 64-bit pins */
#define UPD765_SET_DATA(p,d) {p=((p&~0xFF0000ULL)|(((d)<<16)&0xFF0000ULL));}

/* main status register bits */
#define UPD765_STATUS_D0B   (1<<0)      /* FDD 0 busy */
#define UPD765_STATUS_D1B   (1<<1)      /* FDD 1 busy */
#define UPD765_STATUS_D2B   (1<<2)      /* FDD 2 busy */
#define UPD765_STATUS_D3B   (1<<3)      /* FDD 3 busy */
#define UPD765_STATUS_CB    (1<<4)      /* FDC busy */
#define UPD765_STATUS_EXM   (1<<5)      /* execution mode */
#define UPD765_STATUS_DIO   (1<<6)      /* direction of data transfer */
#define UPD765_STATUS_RQM   (1<<7)      /* request for master */

/* command codes */
#define UPD765_CMD_READ_DATA            ((1<<2)|(1<<1))
#define UPD765_CMD_READ_DELETED_DATA    ((1<<3)|(1<<2))
#define UPD765_CMD_WRITE_DATA           ((1<<2)|(1<<0))
#define UPD765_CMD_WRITE_DELETED_DATA   ((1<<3)|(1<<0))
#define UPD765_CMD_READ_TRACK           ((1<<1))
#define UPD765_CMD_READ_ID              ((1<<3)|(1<<1))
#define UPD765_CMD_FORMAT_TRACK         ((1<<3)|(1<<2)|(1<<0))
#define UPD765_CMD_SCAN_EQUAL           ((1<<4)|(1<<0))
#define UPD765_CMD_SCAN_LOW_OR_EQUAL    ((1<<4)|(1<<3)|(1<<0))
#define UPD765_CMD_SCAN_HIGH_OR_EQUAL   ((1<<4)|(1<<3)|(1<<2)|(1<<0))
#define UPD765_CMD_RECALIBRATE          ((1<<2)|(1<<1)|(1<<0))
#define UPD765_CMD_SENSE_INT_STATUS     ((1<<3))
#define UPD765_CMD_SPECIFY              ((1<<1)|(1<<0))
#define UPD765_CMD_SENSE_DRIVE_STATUS   ((1<<2))
#define UPD765_CMD_SEEK                 ((1<<3)|(1<<2)|(1<<1)|(1<<0))

/* callback to read data byte and status flags from disc drive */
typedef uint16_t (*upd765_read_cb)(int drive, int side, int track, int sector, int index);
/* callback to write data byte to disc, and return status flags */
typedef uint16_t (*upd765_write_cb)(int drive, int side, int track, int sector, int index, uint8_t value);

/* upd765 initialization parameters */
typedef struct {
    /* data read callback */
    upd765_read_cb read_cb;
    /* data write callback */
    upd765_write_cb write_cb;
} upd765_desc_t;

/* upd765 state */
typedef struct {
    upd765_read_cb read_cb;
    upd765_write_cb write_cb;
    int phase;
} upd765_t;

/* initialize a new upd765 instance */
extern void upd765_init(upd765_t* upd, upd765_desc_t* desc);
/* reset an upd765 instance */
extern void upd765_reset(upd765_t* upd);
/* perform an IO request on the upd765 */
extern uint64_t upd765_iorq(upd765_t* upd, uint64_t pins);
/* tick the upd765 instance */
extern void upd765_tick(upd765_t* upd);

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

/* internal phases */
#define _UPD765_PHASE_COMMAND (0)
#define _UPD765_PHASE_EXECUTE (1)
#define _UPD765_PHASE_RESULT  (2)

void upd765_init(upd765_t* upd, upd765_desc_t* desc) {
    CHIPS_ASSERT(upd && desc);
    CHIPS_ASSERT(desc->read_cb && desc->write_cb);
    memset(upd, 0, sizeof(upd765_t));
    upd->read_cb = desc->read_cb;
    upd->write_cb = desc->write_cb;
    upd->phase = _UPD765_PHASE_COMMAND;
}

#endif /* CHIPS_IMPL */
