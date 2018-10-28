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
#define UPD765_A0   (1ULL<<0)  /* in: data/status register select */

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
#define UPD765_CMD_READ_DATA                ((1<<2)|(1<<1))
#define UPD765_CMD_READ_DELETED_DATA        ((1<<3)|(1<<2))
#define UPD765_CMD_WRITE_DATA               ((1<<2)|(1<<0))
#define UPD765_CMD_WRITE_DELETED_DATA       ((1<<3)|(1<<0))
#define UPD765_CMD_READ_A_TRACK             ((1<<1))
#define UPD765_CMD_READ_ID                  ((1<<3)|(1<<1))
#define UPD765_CMD_FORMAT_A_TRACK           ((1<<3)|(1<<2)|(1<<0))
#define UPD765_CMD_SCAN_EQUAL               ((1<<4)|(1<<0))
#define UPD765_CMD_SCAN_LOW_OR_EQUAL        ((1<<4)|(1<<3)|(1<<0))
#define UPD765_CMD_SCAN_HIGH_OR_EQUAL       ((1<<4)|(1<<3)|(1<<2)|(1<<0))
#define UPD765_CMD_RECALIBRATE              ((1<<2)|(1<<1)|(1<<0))
#define UPD765_CMD_SENSE_INTERRUPT_STATUS   ((1<<3))
#define UPD765_CMD_SPECIFY                  ((1<<1)|(1<<0))
#define UPD765_CMD_SENSE_DRIVE_STATUS       ((1<<2))
#define UPD765_CMD_SEEK                     ((1<<3)|(1<<2)|(1<<1)|(1<<0))

/* internal phases */
#define UPD765_PHASE_IDLE    (0)
#define UPD765_PHASE_COMMAND (1)
#define UPD765_PHASE_EXECUTE (2)
#define UPD765_PHASE_RESULT  (3)

/* misc constants */
#define UPD765_FIFO_SIZE (16)
#define UPD765_MAX_FDDS  (4)

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

/* internal floppy disc drive state */
typedef struct {
    int cylinder;
    int track;
} upd765_fdd_t;

/* upd765 state */
typedef struct {
    upd765_read_cb read_cb;
    upd765_write_cb write_cb;

    /* internal state machine */
    int phase;                  /* current phase in command */
    int cmd;                    /* current command */
    int cmd_tick;               /* current command tick */
    int cmd_byte_count;         /* remaining expected bytes in command phase */
    int res_byte_count;         /* remaining expected bytes in result phase */
    int exec_tick_count;        /* number of ticks in execute phase */
    int fifo_pos;               /* position in fifo buffer */
    uint8_t fifo[UPD765_FIFO_SIZE];

    /* status registers */
    uint8_t st0;
    uint8_t st1;
    uint8_t st2;
    uint8_t st3;
    
    /* written by SPECIFY command */
    uint8_t srt;                /* step rate time */
    uint8_t hut;                /* head unload time */
    uint8_t hlt;                /* head load time */
    uint8_t nd;                 /* non-dma mode */

    /* floppy-disc-drive state */
    int fdd_index;
    upd765_fdd_t fdd[UPD765_MAX_FDDS];
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

static inline void _upd765_start_execute(upd765_t* upd, int ticks) {
    upd->phase = UPD765_PHASE_EXECUTE;
    upd->exec_tick_count = ticks;
}

static inline void _upd765_start_cmd(upd765_t* upd, int cmd) {
    upd->cmd = cmd & 0x1F;
    upd->fifo_pos = 0;
    switch (upd->cmd) {
        case UPD765_CMD_READ_DATA:
        case UPD765_CMD_READ_DELETED_DATA:
        case UPD765_CMD_WRITE_DATA:
        case UPD765_CMD_WRITE_DELETED_DATA:
        case UPD765_CMD_READ_A_TRACK:
        case UPD765_CMD_SCAN_EQUAL:
        case UPD765_CMD_SCAN_LOW_OR_EQUAL:
        case UPD765_CMD_SCAN_HIGH_OR_EQUAL:
            upd->phase = UPD765_PHASE_COMMAND;
            upd->cmd_byte_count = 8;
            break;
        case UPD765_CMD_READ_ID:
            upd->phase = UPD765_PHASE_COMMAND;
            upd->cmd_byte_count = 1;
            break;
        case UPD765_CMD_FORMAT_A_TRACK:
            upd->phase = UPD765_PHASE_COMMAND;
            upd->cmd_byte_count = 5;
            break;
        case UPD765_CMD_RECALIBRATE:
            upd->phase = UPD765_PHASE_COMMAND;
            upd->cmd_byte_count = 1;
            break;
        case UPD765_CMD_SENSE_INTERRUPT_STATUS:
            upd->cmd_byte_count = 0;
            _upd765_start_execute(upd, 0);
            break;
        case UPD765_CMD_SPECIFY:
            upd->phase = UPD765_PHASE_COMMAND;
            upd->cmd_byte_count = 2;
            break;
        case UPD765_CMD_SENSE_DRIVE_STATUS:
            upd->phase = UPD765_PHASE_COMMAND;
            upd->cmd_byte_count = 1;
            break;
        case UPD765_CMD_SEEK:
            upd->phase = UPD765_PHASE_COMMAND;
            upd->cmd_byte_count = 2;
            break;
        default:
            upd->phase = UPD765_PHASE_RESULT;
            upd->cmd_byte_count = 0;
            break;
    }
}

static inline void _upd765_continue_cmd(upd765_t* upd, uint8_t data) {
    CHIPS_ASSERT(upd->fifo_pos < UPD765_FIFO_SIZE);
    CHIPS_ASSERT(upd->phase == UPD765_PHASE_COMMAND);
    upd->fifo[upd->fifo_pos++] = data;
    upd->cmd_byte_count--;
    if (0 == upd->cmd_byte_count) {
        /* transition to next phase */
        switch (upd->cmd) {
            case UPD765_CMD_READ_DATA:
            case UPD765_CMD_READ_DELETED_DATA:
            case UPD765_CMD_WRITE_DATA:
            case UPD765_CMD_WRITE_DELETED_DATA:
            case UPD765_CMD_READ_A_TRACK:
            case UPD765_CMD_READ_ID:
            case UPD765_CMD_FORMAT_A_TRACK:
            case UPD765_CMD_SCAN_EQUAL:
            case UPD765_CMD_SCAN_LOW_OR_EQUAL:
            case UPD765_CMD_SCAN_HIGH_OR_EQUAL:
            case UPD765_CMD_SEEK:
                _upd765_start_execute(upd, 1);
                break;
            case UPD765_CMD_RECALIBRATE:
                _upd765_start_execute(upd, 100);
                break;
            case UPD765_CMD_SPECIFY:
                _upd765_start_execute(upd, 0);
                break;
            case UPD765_CMD_SENSE_INTERRUPT_STATUS:
                /* can't happen */
                CHIPS_ASSERT(false);
                break;
            case UPD765_CMD_SENSE_DRIVE_STATUS:
                // FIXME
                upd->phase = UPD765_PHASE_RESULT;
                break;
            default:
                // FIXME
                upd->phase = UPD765_PHASE_RESULT;
                break;
        }
    }
}

static inline void _upd765_execute_invalid(upd765_t* upd) {
    if (0 == upd->exec_tick_count) {
        upd->phase = UPD765_PHASE_IDLE;
    }
    else {
        upd->exec_tick_count--;
    }
}

static inline void _upd765_execute_specify(upd765_t* upd) {
    if (0 == upd->exec_tick_count) {
        upd->srt = (upd->fifo[0]>>4) & 0x0F;    /* step rate time */
        upd->hut = upd->fifo[0] & 0x0F;         /* head unload time */
        upd->hlt = (upd->fifo[1]>>1) & 0x7F;    /* head load time */
        upd->nd  = upd->fifo[1] & 1;            /* non-dma mode */
        upd->phase = UPD765_PHASE_IDLE;
    }
    else {
        upd->exec_tick_count--;
    }
}

static inline void _upd765_execute_recalibrate(upd765_t* upd) {
    if (0 == upd->exec_tick_count) {
        upd->fdd_index = upd->fifo[0] & 3;
        upd->fdd[upd->fdd_index].track = 0;
        upd->phase = UPD765_PHASE_IDLE;
    }
    else {
        upd->exec_tick_count--;
    }
}

static inline void _upd765_execute_sense_interrupt_status(upd765_t* upd) {
    if (0 == upd->exec_tick_count) {
        upd->phase = UPD765_PHASE_RESULT;
        upd->res_byte_count = 2;
        upd->fifo_pos = 0;
        upd->fifo[0] = upd->st0;    // FIXME: status register 0 content
        upd->fifo[1] = upd->fdd[upd->fdd_index].cylinder;   // FIXME: present cylinder number
    }
    else {
        upd->exec_tick_count--;
    }
}

static inline void _upd765_execute(upd765_t* upd) {
    switch (upd->cmd) {
        case UPD765_CMD_READ_DATA:
        case UPD765_CMD_READ_DELETED_DATA:
        case UPD765_CMD_WRITE_DATA:
        case UPD765_CMD_WRITE_DELETED_DATA:
        case UPD765_CMD_READ_A_TRACK:
        case UPD765_CMD_READ_ID:
        case UPD765_CMD_FORMAT_A_TRACK:
        case UPD765_CMD_SCAN_EQUAL:
        case UPD765_CMD_SCAN_LOW_OR_EQUAL:
        case UPD765_CMD_SCAN_HIGH_OR_EQUAL:
        case UPD765_CMD_SEEK:
            _upd765_execute_invalid(upd);
            break;
        case UPD765_CMD_RECALIBRATE:
            _upd765_execute_recalibrate(upd);
            break;
        case UPD765_CMD_SPECIFY:
            _upd765_execute_specify(upd);
            break;
        case UPD765_CMD_SENSE_INTERRUPT_STATUS:
            _upd765_execute_sense_interrupt_status(upd);
            break;
        case UPD765_CMD_SENSE_DRIVE_STATUS:
        default:
            _upd765_execute_invalid(upd);
            break;
    }
}

static inline uint8_t _upd765_read_status(upd765_t* upd) {
    // FIXME: drive seek bits
    uint8_t status = 0;
    /* FIXME: RQM is a handshake flag and remains inactive 
        for between 2us and 50us, for now just indicate
        that we're always ready during the command and result phase
    */
    switch (upd->phase) {
        case UPD765_PHASE_IDLE:
            status |= UPD765_STATUS_RQM;
            break;
        case UPD765_PHASE_COMMAND:
            status |= UPD765_STATUS_CB|UPD765_STATUS_RQM;
            break;
        case UPD765_PHASE_EXECUTE:
            status |= UPD765_STATUS_CB|UPD765_STATUS_EXM;
            break;
        case UPD765_PHASE_RESULT:
            status |= UPD765_STATUS_CB|UPD765_STATUS_DIO|UPD765_STATUS_RQM;
            break;
    }
    return status;
}

static inline uint8_t _upd765_read_data(upd765_t* upd) {
    if (UPD765_PHASE_RESULT == upd->phase) {
        CHIPS_ASSERT(upd->res_byte_count > 0);
        if (--upd->res_byte_count == 0) {
            upd->phase = UPD765_PHASE_IDLE;
        }
        return upd->fifo[upd->fifo_pos++];
    }
    else {
        return 0xFF;
    }
}

static inline void _upd765_write_data(upd765_t* upd, uint8_t data) {
    if (UPD765_PHASE_IDLE == upd->phase) {
        _upd765_start_cmd(upd, data);
    }
    else if (UPD765_PHASE_COMMAND == upd->phase) {
        _upd765_continue_cmd(upd, data);
    }
}

void upd765_init(upd765_t* upd, upd765_desc_t* desc) {
    CHIPS_ASSERT(upd && desc);
//    CHIPS_ASSERT(desc->read_cb && desc->write_cb);
    memset(upd, 0, sizeof(upd765_t));
    upd->read_cb = desc->read_cb;
    upd->write_cb = desc->write_cb;
    upd765_reset(upd);
}

void upd765_reset(upd765_t* upd) {
    CHIPS_ASSERT(upd);
    upd->phase = UPD765_PHASE_IDLE;
}

uint64_t upd765_iorq(upd765_t* upd, uint64_t pins) {
    if (pins & UPD765_CS) {
        if (pins & UPD765_RD) {
            if (pins & UPD765_A0) {
                UPD765_SET_DATA(pins, _upd765_read_data(upd));
            }
            else {
                UPD765_SET_DATA(pins, _upd765_read_status(upd));
            }
        }
        else if (pins & UPD765_WR) {
            if (pins & UPD765_A0) {
                _upd765_write_data(upd, UPD765_GET_DATA(pins));
            }
        }
    }
    return pins;
}

void upd765_tick(upd765_t* upd) {
    if (upd->phase == UPD765_PHASE_EXECUTE) {
        _upd765_execute(upd);
    }
}

#endif /* CHIPS_IMPL */
