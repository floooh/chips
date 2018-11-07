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

    ## WIP IMPLEMENTATION NOTES

    - the chips is always in one of four phases:
        - IDLE: expects a command byte to be written
        - COMMAND: a command byte has been written, and parameter bytes are
          expected to be written, at the end of the COMMAND phase (last
          parameter byte received), an CommandAction function will be executed,
          this prepares and switches to either the EXECUTE, RESULT or IDLE phase
        - EXECUTE (optional): a variable length execution phase, controlled by a command-
          specifc execute function, the execute function defines when it is
          done and switches to the RESULT phase
        - RESULT (optional): a fixed number of result bytes are expected to be
          read by the CPU, after all are read, switch back to the IDLE phase
    - a floppy disk is described by:
        - 1 or 2 sides
        - N tracks
        - N sectors per track
        - N bytes per sector

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

/* status register 0 bits */
#define UPD765_ST0_US0      (1<<0)      /* drive unit number at interrupt */
#define UPD765_ST0_US1      (1<<1)      /* drive unit number at interrupt */
#define UPD765_ST0_HD       (1<<2)      /* head address */
#define UPD765_ST0_NR       (1<<3)      /* not ready */
#define UPD765_ST0_EC       (1<<4)      /* equipment check */
#define UPD765_ST0_SE       (1<<5)      /* seek end */
#define UPD765_ST0_RES      ((1<<7)|(1<<6)) /* command result bitmask */
#define UPD765_ST0_NT       (0)         /* D7=0,D6=0: normal termination of command */
#define UPD765_ST0_AT       (1<<6)      /* D7=0,D6=0: abnormal termination of command */
#define UPD765_ST0_IC       (1<<7)      /* D7=1,D6=0: invalid command issue */
#define UPD765_ST0_ATRM     ((1<<7)|(1<<6)) /* D7=1,D6=1: abormal termination */

/* status register 1 bits */
#define UPD765_ST1_MA       (1<<0)      /* missing address mark */
#define UPD765_ST1_NW       (1<<1)      /* not writable */
#define UPD765_ST1_ND       (1<<2)      /* no data */
#define UPD765_ST1_D3       (1<<3)      /* UNUSED */
#define UPD765_ST1_OR       (1<<4)      /* overrun */
#define UPD765_ST1_DE       (1<<5)      /* data error (crc check failed) */
#define UPD765_ST1_D6       (1<<6)      /* UNUSED */
#define UPD765_ST1_EN       (1<<7)      /* end of cylinder */

/* status register 2 bits */
#define UPD765_ST2_MD       (1<<0)      /* missing address mark */
#define UPD765_ST2_BC       (1<<1)      /* bad cylinder */
#define UPD765_ST2_SN       (1<<2)      /* scan not satisfied */
#define UPD765_ST2_SH       (1<<3)      /* scan equal hit */
#define UPD765_ST2_WC       (1<<4)      /* wrong cylinder */
#define UPD765_ST2_DD       (1<<5)      /* data error in data field */
#define UPD765_ST2_CM       (1<<6)      /* control mark */
#define UPD765_ST2_D7       (1<<7)      /* UNUSED */

/* status register 3 bits */
#define UPD765_ST3_US0      (1<<0)      /* unit select 0 signal to FDD */
#define UPD765_ST3_US1      (1<<1)      /* unit select 1 signal to FDD */
#define UPD765_ST3_HD       (1<<2)      /* head select signal to FDD */
#define UPD765_ST3_TS       (1<<3)      /* two-side signal to FDD */
#define UPD765_ST3_T0       (1<<4)      /* track-0 signal to FDD */
#define UPD765_ST3_RY       (1<<5)      /* status of READY signal from FDD */
#define UPD765_ST3_WP       (1<<6)      /* status of WRITE PROTECT signal from FDD */
#define UPD765_ST3_FT       (1<<7)      /* status of FAULT signal from FDD */

/* command codes */
#define UPD765_CMD_INVALID                  (0)
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

/* sector info block for the info callback */
typedef struct {
    /* physical track content */
    int physical_track;
    /* first sector-info block (with logical attributes) */
    uint8_t c;              /* cylinder (logical track number) */
    uint8_t h;              /* head address (logical side) */
    uint8_t r;              /* record (sector id byte) */
    uint8_t n;              /* number (sector size byte) */
    uint8_t st1;            /* return status 1 */
    uint8_t st2;            /* return status 2 */
} upd765_trackinfo_t;

/* callback to seek to a phyiscal track */
typedef bool (*upd765_seektrack_cb)(int drive, int track, void* user_data);
/* callback to seek to a sector on current physical track */
typedef bool (*upd765_seeksector_cb)(int drive, uint8_t c, uint8_t h, uint8_t r, uint8_t n, void* user_data);
/* callback to read the next sector data byte */
typedef bool (*upd765_read_cb)(int drive, uint8_t h, void* user_data, uint8_t* out_data);
/* callback to read info about currently seeked-to track */
typedef bool (*upd765_trackinfo_cb)(int drive, int side, void* user_data, upd765_trackinfo_t* out_info);

/* upd765 initialization parameters */
typedef struct {
    upd765_seektrack_cb seektrack_cb;
    upd765_seeksector_cb seeksector_cb;
    upd765_read_cb read_cb;
    upd765_trackinfo_cb trackinfo_cb;
    void* user_data;
} upd765_desc_t;

/* upd765 state */
typedef struct {
    upd765_seektrack_cb seektrack_cb;
    upd765_seeksector_cb seeksector_cb;
    upd765_read_cb read_cb;
    upd765_trackinfo_cb trackinfo_cb;
    void* user_data;

    /* internal state machine */
    int phase;                  /* current phase in command */
    int cmd;                    /* current command */
    int fifo_pos;               /* position in fifo buffer */
    int fifo_num;               /* number of valid or expected items in fifo */
    uint8_t fifo[UPD765_FIFO_SIZE];

    /* current status */
    upd765_trackinfo_t track_info;
    uint8_t eot;                /* end-of-track param of last command */
    uint8_t gpl;                /* gap-length param of last command */
    uint8_t dtl;                /* data-length param of last command */
    uint8_t st[4];
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

static inline void _upd765_fifo_reset(upd765_t* upd, int num) {
    upd->fifo_pos = 0;
    upd->fifo_num = num;
}

static inline void _upd765_fifo_wr(upd765_t* upd, uint8_t val) {
    CHIPS_ASSERT(upd->fifo_pos < UPD765_FIFO_SIZE);
    upd->fifo[upd->fifo_pos++] = val;
}

static inline uint8_t _upd765_fifo_rd(upd765_t* upd) {
    CHIPS_ASSERT(upd->fifo_pos < UPD765_FIFO_SIZE);
    return upd->fifo[upd->fifo_pos++];
}

static void _upd765_to_phase_result(upd765_t* upd);
static void _upd765_to_phase_idle(upd765_t* upd);

static void _upd765_result_invalid(upd765_t* upd) {
    upd->fifo[0] = UPD765_ST0_IC;
}

static void _upd765_result_std(upd765_t* upd) {
    // FIXME
}

static void _upd765_action_read_a_track(upd765_t* upd) {
    // FIXME
}

static void _upd765_exec_read_a_track(upd765_t* upd) {
    // FIXME
    _upd765_to_phase_idle(upd);
}

static void _upd765_action_specify(upd765_t* upd) {
    /* nothing useful to do in specify, this just configures some
       timing and DMA-mode params that are not relevant for
       this emulation
    */
}

static void _upd765_action_sense_drive_status(upd765_t* upd) {
    // FIXME
}

static void _upd765_result_sense_drive_status(upd765_t* upd) {
    // FIXME
}

static void _upd765_action_write_data(upd765_t* upd) {
    // FIXME
}

static void _upd765_exec_write_data(upd765_t* upd) {
    // FIXME
    _upd765_to_phase_result(upd);
}

static void _upd765_action_read_data(upd765_t* upd) {
    /* seek to requested sector on current physical track */
    upd->track_info.c = upd->fifo[2];
    upd->track_info.h = upd->fifo[3];
    upd->track_info.r = upd->fifo[4];
    upd->track_info.n = upd->fifo[5];
    upd->track_info.st1 = 0;
    upd->track_info.st2 = 0;
    upd->eot = upd->fifo[6];
    upd->gpl = upd->fifo[7];
    upd->dtl = upd->fifo[8];
    /* FIXME: handle length of read data via n=0 and dtl!=0xFF */
    CHIPS_ASSERT((upd->track_info.n != 0) && (upd->dtl == 0xFF));
    /* FIXME: handle read several sectors at a time */
    CHIPS_ASSERT(upd->track_info.r == upd->eot);
    upd->st[0] = upd->fifo[1] & 7;
    const int fdd_index = upd->st[0] & 3;
    if (!upd->seeksector_cb(fdd_index,
            upd->track_info.c,
            upd->track_info.h,
            upd->track_info.r,
            upd->track_info.n,
            upd->user_data))
    {
        /* seek failed, switch to result phase with error */
        upd->st[0] = upd->st[0] | UPD765_ST0_AT; /* FIXME: NOT READY if no disc in drive */
        upd->st[1] = UPD765_ST1_ND; /* FIXME: 0 if no disc in drive */
        upd->st[2] = 0;
        _upd765_to_phase_result(upd);
    }
}

static void _upd765_exec_read_data(upd765_t* upd) {
    // FIXME
}

static void _upd765_result_read_data(upd765_t* upd) {
    upd->fifo[0] = upd->st[0];
    upd->fifo[1] = upd->st[1];
    upd->fifo[2] = upd->st[2];
    upd->fifo[3] = upd->track_info.c;
    upd->fifo[4] = upd->track_info.h;
    upd->fifo[5] = upd->track_info.r;
    upd->fifo[6] = upd->track_info.n;
}

static void _upd765_exec_recalibrate(upd765_t* upd) {
    /* set drive head to track 0 */
    const int fdd_index = upd->fifo[1] & 3;
    upd->seektrack_cb(fdd_index, 0, upd->user_data);
    upd->track_info.physical_track = 0;
    upd->st[0] = fdd_index | UPD765_ST0_SE;
    _upd765_to_phase_idle(upd);
}

static void _upd765_result_sense_interrupt_status(upd765_t* upd) {
    if (upd->st[0] & UPD765_ST0_SE) {
        /* on seek-end, return current track */
        const int fdd_index = upd->st[0] & 3;
        if (upd->trackinfo_cb(fdd_index, 0, upd->user_data, &upd->track_info)) {
            upd->fifo[0] = upd->st[0];
            upd->fifo[1] = upd->track_info.physical_track;
            /* FIXME: INVALID COMMAND ISSUE bit here too? */
            upd->st[0] &= ~(UPD765_ST0_RES|UPD765_ST0_SE);
            return;
        }
    }
    else {
        upd->fifo[0] = UPD765_ST0_IC;
        upd->fifo_num = 1;
    }
}

static void _upd765_action_write_deleted_data(upd765_t* upd) {
    // FIXME
}

static void _upd765_exec_write_deleted_data(upd765_t* upd) {
    // FIXME
    _upd765_to_phase_result(upd);
}

static void _upd765_result_read_id(upd765_t* upd) {
    const int fdd_index = upd->fifo[1] & 3;
    const int head_index = (upd->fifo[1] & 4) >> 2;
    if (upd->trackinfo_cb(fdd_index, head_index, upd->user_data, &upd->track_info)) {
        upd->st[0] = upd->fifo[1] & 7;
        upd->st[1] = upd->track_info.st1;
        upd->st[2] = upd->track_info.st2;
        upd->fifo[0] = upd->st[0];
        upd->fifo[1] = upd->st[1];
        upd->fifo[2] = upd->st[2];
        upd->fifo[3] = upd->track_info.c;
        upd->fifo[4] = upd->track_info.h;
        upd->fifo[5] = upd->track_info.r;
        upd->fifo[6] = upd->track_info.n;
    }
    else {
        /* FIXME: ??? */
        upd->fifo[0] = UPD765_ST0_IC;
        upd->fifo_num = 1;
    }
}

static void _upd765_action_read_deleted_data(upd765_t* upd) {
    // FIXME
}

static void _upd765_exec_read_deleted_data(upd765_t* upd) {
    // FIXME
    _upd765_to_phase_result(upd);
}

static void _upd765_action_format_a_track(upd765_t* upd) {
    // FIXME
}

static void _upd765_exec_format_a_track(upd765_t* upd) {
    // FIXME
    _upd765_to_phase_result(upd);
}

static void _upd765_action_seek(upd765_t* upd) {
    // FIXME
}

static void _upd765_exec_seek(upd765_t* upd) {
    // FIXME
    _upd765_to_phase_idle(upd);
}

static void _upd765_action_scan_equal(upd765_t* upd) {
    // FIXME
}

static void _upd765_exec_scan_equal(upd765_t* upd) {
    // FIXME
    _upd765_to_phase_result(upd);
}

static void _upd765_action_scan_low_or_equal(upd765_t* upd) {
    // FIXME
}

static void _upd765_exec_scan_low_or_equal(upd765_t* upd) {
    // FIXME
    _upd765_to_phase_result(upd);
}

static void _upd765_action_scan_high_or_equal(upd765_t* upd) {
    // FIXME
}

static void _upd765_exec_scan_high_or_equal(upd765_t* upd) {
    // FIXME
    _upd765_to_phase_result(upd);
}

typedef struct {
    int cmd;            /* the command */
    int cmd_num_bytes;  /* number of expected command bytes */
    int res_num_bytes;  /* number of result bytes */
    void (*action)(upd765_t*);  /* the action handler to be called at end of command phase */
    void (*exec)(upd765_t*);    /* the exec handler to be called during execute phase */
    void (*result)(upd765_t*);  /* the result handler called before result phase */
} _upd765_cmd_desc_t;

static _upd765_cmd_desc_t _upd765_cmd_table[32] = {
    /* 0 */     { UPD765_CMD_INVALID,                   1, 1, 0, 0, _upd765_result_invalid },
    /* 1 */     { UPD765_CMD_INVALID,                   1, 1, 0, 0, _upd765_result_invalid },
    /* 2 */     { UPD765_CMD_READ_A_TRACK,              9, 7, _upd765_action_read_a_track, _upd765_exec_read_a_track, _upd765_result_std },
    /* 3 */     { UPD765_CMD_SPECIFY,                   3, 0, _upd765_action_specify, 0, 0 },
    /* 4 */     { UPD765_CMD_SENSE_DRIVE_STATUS,        2, 1, _upd765_action_sense_drive_status, 0, _upd765_result_sense_drive_status },
    /* 5 */     { UPD765_CMD_WRITE_DATA,                9, 7, _upd765_action_write_data, _upd765_exec_write_data, _upd765_result_std },
    /* 6 */     { UPD765_CMD_READ_DATA,                 9, 7, _upd765_action_read_data, _upd765_exec_read_data, _upd765_result_read_data },
    /* 7 */     { UPD765_CMD_RECALIBRATE,               2, 0, 0, _upd765_exec_recalibrate, 0 },
    /* 8 */     { UPD765_CMD_SENSE_INTERRUPT_STATUS,    1, 2, 0, 0, _upd765_result_sense_interrupt_status },
    /* 9 */     { UPD765_CMD_WRITE_DELETED_DATA,        9, 7, _upd765_action_write_deleted_data, _upd765_exec_write_deleted_data, _upd765_result_std },
    /*10 */     { UPD765_CMD_READ_ID,                   2, 7, 0, 0, _upd765_result_read_id },
    /*11 */     { UPD765_CMD_INVALID,                   1, 1, 0, 0, _upd765_result_invalid },
    /*12 */     { UPD765_CMD_READ_DELETED_DATA,         9, 7, _upd765_action_read_deleted_data, _upd765_exec_read_deleted_data, _upd765_result_std },
    /*13 */     { UPD765_CMD_FORMAT_A_TRACK,            6, 7, _upd765_action_format_a_track, _upd765_exec_format_a_track, _upd765_result_std },
    /*14 */     { UPD765_CMD_INVALID,                   1, 1, 0, 0, _upd765_result_invalid },
    /*15 */     { UPD765_CMD_SEEK,                      3, 0, _upd765_action_seek, _upd765_exec_seek, 0 },
    /*16 */     { UPD765_CMD_INVALID,                   1, 1, 0, 0, _upd765_result_invalid },
    /*17 */     { UPD765_CMD_SCAN_EQUAL,                9, 7, _upd765_action_scan_equal, _upd765_exec_scan_equal, _upd765_result_std },
    /*18 */     { UPD765_CMD_INVALID,                   1, 1, 0, 0, _upd765_result_invalid },
    /*19 */     { UPD765_CMD_INVALID,                   1, 1, 0, 0, _upd765_result_invalid },
    /*20 */     { UPD765_CMD_INVALID,                   1, 1, 0, 0, _upd765_result_invalid },
    /*21 */     { UPD765_CMD_INVALID,                   1, 1, 0, 0, _upd765_result_invalid },
    /*22 */     { UPD765_CMD_INVALID,                   1, 1, 0, 0, _upd765_result_invalid },
    /*23 */     { UPD765_CMD_INVALID,                   1, 1, 0, 0, _upd765_result_invalid },
    /*24 */     { UPD765_CMD_INVALID,                   1, 1, 0, 0, _upd765_result_invalid },
    /*25 */     { UPD765_CMD_SCAN_LOW_OR_EQUAL,         1, 1, _upd765_action_scan_low_or_equal, _upd765_exec_scan_low_or_equal, _upd765_result_std },
    /*26 */     { UPD765_CMD_INVALID,                   1, 1, 0, 0, _upd765_result_invalid },
    /*27 */     { UPD765_CMD_INVALID,                   1, 1, 0, 0, _upd765_result_invalid },
    /*28 */     { UPD765_CMD_INVALID,                   1, 1, 0, 0, _upd765_result_invalid },
    /*29 */     { UPD765_CMD_SCAN_HIGH_OR_EQUAL,        1, 1, _upd765_action_scan_high_or_equal, _upd765_exec_scan_high_or_equal, _upd765_result_std },
    /*30 */     { UPD765_CMD_INVALID,                   1, 1, 0, 0, _upd765_result_invalid },
    /*31 */     { UPD765_CMD_INVALID,                   1, 1, 0, 0, _upd765_result_invalid }
};

static inline uint8_t _upd765_read_status(upd765_t* upd) {
    // FIXME: drive bits 0..2 should be set while drive is seeking
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
            status |= UPD765_STATUS_CB|UPD765_STATUS_EXM|UPD765_STATUS_RQM;
            break;
        case UPD765_PHASE_RESULT:
            status |= UPD765_STATUS_CB|UPD765_STATUS_DIO|UPD765_STATUS_RQM;
            break;
    }
    return status;
}

/* start the command, exec, result, idle phases */
static void _upd765_to_phase_command(upd765_t* upd, uint8_t data) {
    CHIPS_ASSERT(upd->phase == UPD765_PHASE_IDLE);
    upd->phase = UPD765_PHASE_COMMAND;
    upd->cmd = data & 0x1F;
    CHIPS_ASSERT((_upd765_cmd_table[upd->cmd].cmd == UPD765_CMD_INVALID) || 
                (_upd765_cmd_table[upd->cmd].cmd == upd->cmd));
    _upd765_fifo_reset(upd, _upd765_cmd_table[upd->cmd].cmd_num_bytes);
    _upd765_fifo_wr(upd, data);
}

static void _upd765_to_phase_exec(upd765_t* upd) {
    CHIPS_ASSERT(upd->phase == UPD765_PHASE_COMMAND);
    upd->phase = UPD765_PHASE_EXECUTE;
}

static void _upd765_to_phase_result(upd765_t* upd) {
    CHIPS_ASSERT((upd->phase == UPD765_PHASE_COMMAND) || (upd->phase == UPD765_PHASE_EXECUTE));
    upd->phase = UPD765_PHASE_RESULT;
    _upd765_fifo_reset(upd, _upd765_cmd_table[upd->cmd].res_num_bytes);
    if (_upd765_cmd_table[upd->cmd].result) {
        _upd765_cmd_table[upd->cmd].result(upd);
    }
}

static void _upd765_to_phase_idle(upd765_t* upd) {
    CHIPS_ASSERT(upd->phase != UPD765_PHASE_IDLE);
    upd->phase = UPD765_PHASE_IDLE;
}

/* write a data byte to the upd765 */
static void _upd765_write_data(upd765_t* upd, uint8_t data) {
    if ((UPD765_PHASE_IDLE == upd->phase) || (UPD765_PHASE_COMMAND == upd->phase)) {
        if (UPD765_PHASE_IDLE == upd->phase) {
            /* start a new command */
            _upd765_to_phase_command(upd, data);
        }
        else {
            /* continue gathering parameters */
            _upd765_fifo_wr(upd, data);
        }
        /* if no more params expected, proceed to exec, result or idle phase */
        if (upd->fifo_pos == upd->fifo_num) {
            const _upd765_cmd_desc_t* cmd_desc = &_upd765_cmd_table[upd->cmd];
            /* invoke command action callback */
            if (cmd_desc->action) {
                cmd_desc->action(upd);
            }
            /* decide what's the next phase */
            if (cmd_desc->exec) {
                _upd765_to_phase_exec(upd);
            }
            else if (cmd_desc->result) {
                _upd765_to_phase_result(upd);
            }
            else {
                _upd765_to_phase_idle(upd);
            }
        }
    }
    else if (UPD765_PHASE_EXECUTE == upd->phase) {
        // FIXME
    }
}

/* read a data byte from the upd765 */
static uint8_t _upd765_read_data(upd765_t* upd) {
    uint8_t data = 0xFF;
    if (UPD765_PHASE_RESULT == upd->phase) {
        data = _upd765_fifo_rd(upd);
        if (upd->fifo_pos == upd->fifo_num) {
            /* all result bytes transfered, transition to idle phase */
            _upd765_to_phase_idle(upd);
        }
    }
    else if (UPD765_PHASE_EXECUTE == upd->phase) {
        switch (upd->cmd) {
            case UPD765_CMD_READ_DATA:
                /* read next sector data byte from FDD */
                if (!upd->read_cb(upd->st[0]&3, upd->track_info.h, upd->user_data, &data)) {
                    _upd765_to_phase_result(upd);
                }
                break;
        }
    }
    return data;
}

void upd765_init(upd765_t* upd, upd765_desc_t* desc) {
    CHIPS_ASSERT(upd && desc);
    CHIPS_ASSERT(desc->seektrack_cb);
    CHIPS_ASSERT(desc->seeksector_cb);
    CHIPS_ASSERT(desc->read_cb);
    CHIPS_ASSERT(desc->trackinfo_cb);
    memset(upd, 0, sizeof(upd765_t));
    upd->seektrack_cb = desc->seektrack_cb;
    upd->seeksector_cb = desc->seeksector_cb;
    upd->read_cb = desc->read_cb;
    upd->trackinfo_cb = desc->trackinfo_cb;
    upd->user_data = desc->user_data;
    upd765_reset(upd);
}

void upd765_reset(upd765_t* upd) {
    CHIPS_ASSERT(upd);
    upd->phase = UPD765_PHASE_IDLE;
    _upd765_fifo_reset(upd, 0);
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
        if (_upd765_cmd_table[upd->cmd].exec) {
            _upd765_cmd_table[upd->cmd].exec(upd);
        }
    }
}

#endif /* CHIPS_IMPL */
