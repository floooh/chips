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

    ## TODO
        - DOCS!
        - cleanup callbacks

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

// the A0 pin is usually connected to the address bus A0 pin
#define UPD765_PIN_A0   (0)  /* in: data/status register select */

// data bus pins (in/out)
#define UPD765_PIN_D0   (16)
#define UPD765_PIN_D1   (17)
#define UPD765_PIN_D2   (18)
#define UPD765_PIN_D3   (19)
#define UPD765_PIN_D4   (20)
#define UPD765_PIN_D5   (21)
#define UPD765_PIN_D6   (22)
#define UPD765_PIN_D7   (23)

// control pins shared with CPU
#define UPD765_PIN_RD   (27)  // in: read data from controller
#define UPD765_PIN_WR   (28)  // in: write data to controller

// control pins
#define UPD765_PIN_CS   (40)  // in: chip select

// pin bit masks
#define UPD765_A0   (1ULL<<UPD765_PIN_A0)
#define UPD765_D0   (1ULL<<UPD765_PIN_D0)
#define UPD765_D1   (1ULL<<UPD765_PIN_D1)
#define UPD765_D2   (1ULL<<UPD765_PIN_D2)
#define UPD765_D3   (1ULL<<UPD765_PIN_D3)
#define UPD765_D4   (1ULL<<UPD765_PIN_D4)
#define UPD765_D5   (1ULL<<UPD765_PIN_D5)
#define UPD765_D6   (1ULL<<UPD765_PIN_D6)
#define UPD765_D7   (1ULL<<UPD765_PIN_D7)
#define UPD765_RD   (1ULL<<UPD765_PIN_RD)
#define UPD765_WR   (1ULL<<UPD765_PIN_WR)
#define UPD765_CS   (1ULL<<UPD765_PIN_CS)

/* extract 8-bit data bus from 64-bit pins */
#define UPD765_GET_DATA(p) ((uint8_t)(((p)&0xFF0000ULL)>>16))
/* merge 8-bit data bus value into 64-bit pins */
#define UPD765_SET_DATA(p,d) {p=(((p)&~0xFF0000ULL)|(((d)<<16)&0xFF0000ULL));}

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
#define UPD765_PHASE_EXEC    (2)
#define UPD765_PHASE_RESULT  (3)

/* misc constants */
#define UPD765_FIFO_SIZE (16)

/* sector info block for the info callback */
typedef struct {
    /* current physical track number */
    int physical_track;
    /* sector-info block (with logical attributes) */
    uint8_t c;              /* cylinder (logical track number) */
    uint8_t h;              /* head address (logical side) */
    uint8_t r;              /* record (sector id byte) */
    uint8_t n;              /* number (sector size byte) */
    uint8_t st1;            /* return status 1 */
    uint8_t st2;            /* return status 2 */
} upd765_sectorinfo_t;

/* drive info struct filled out by the upd765_driveinfo_cb callback */
typedef struct {
    int physical_track;     /* current track index */
    int sides;              /* number of sides (1 or 2) */
    int head;               /* current side (0 or 1) */
    bool ready;             /* status of the drive's ready signal */
    bool write_protected;   /* status of the drive's write protected signal */
    bool fault;             /* status of the drive's fault signal */
} upd765_driveinfo_t;

/* callback result bits (compatible with FDD_RESULT_*) */
#define UPD765_RESULT_SUCCESS (0)
#define UPD765_RESULT_NOT_READY (1<<0)
#define UPD765_RESULT_NOT_FOUND (1<<1)
#define UPD765_RESULT_END_OF_SECTOR (1<<2)

/* callback to seek to a phyiscal track */
typedef int (*upd765_seektrack_cb)(int drive, int track, void* user_data);
/* callback to seek to a sector on current physical track */
typedef int (*upd765_seeksector_cb)(int drive, int side, upd765_sectorinfo_t* inout_info, void* user_data);
/* callback to read the next sector data byte */
typedef int (*upd765_read_cb)(int drive, int side, void* user_data, uint8_t* out_data);
/* callback to read info about first sector on current reack */
typedef int (*upd765_trackinfo_cb)(int drive, int side, void* user_data, upd765_sectorinfo_t* out_info);
/* callback to get info about disk drive (called on SENSE_DRIVE_STATUS command) */
typedef void (*upd765_driveinfo_cb)(int drive, void* user_data, upd765_driveinfo_t* out_info);

/* upd765 initialization parameters */
typedef struct {
    upd765_seektrack_cb seektrack_cb;
    upd765_seeksector_cb seeksector_cb;
    upd765_read_cb read_cb;
    upd765_trackinfo_cb trackinfo_cb;
    upd765_driveinfo_cb driveinfo_cb;
    void* user_data;
} upd765_desc_t;

/* upd765 state */
typedef struct {
    /* internal state machine */
    int phase;                  /* current phase in command */
    int cmd;                    /* current command */
    int fifo_pos;               /* current argument/result fifo pos */
    int fifo_num;               /* number of valid items in fifo */
    uint8_t fifo[UPD765_FIFO_SIZE];

    /* current status */
    upd765_sectorinfo_t sector_info;
    upd765_driveinfo_t drive_info;      /* only valid after SENSE_DRIVE_CMD */
    uint8_t st[4];

    /* callback functions */
    upd765_seektrack_cb seektrack_cb;
    upd765_seeksector_cb seeksector_cb;
    upd765_read_cb read_cb;
    upd765_trackinfo_cb trackinfo_cb;
    upd765_driveinfo_cb driveinfo_cb;
    void* user_data;

    /* debug inspection */
    uint64_t pins;  /* pin state at last _ui_upd765_iorq */
    uint8_t status; /* last result of _ui_upd765_read_status */
} upd765_t;

/* initialize a new upd765 instance */
void upd765_init(upd765_t* upd, const upd765_desc_t* desc);
/* reset an upd765 instance */
void upd765_reset(upd765_t* upd);
/* perform an IO request on the upd765 */
uint64_t upd765_iorq(upd765_t* upd, uint64_t pins);
// prepare upd765_t snapshot for saving
void upd765_snapshot_onsave(upd765_t* snapshot);
// fixup upd765_t snapshot after loading
void upd765_snapshot_onload(upd765_t* snapshot, upd765_t* sys);

#ifdef __cplusplus
} /* extern "C" */
#endif

/*--- IMPLEMENTATION ---------------------------------------------------------*/
#ifdef CHIPS_IMPL
#include <string.h>
#ifndef CHIPS_ASSERT
    #include <assert.h>
    #define CHIPS_ASSERT(c) assert(c)
#endif

/* internal argument/result fifo helpers */
static inline void _upd765_fifo_reset(upd765_t* upd, int num) {
    CHIPS_ASSERT(num <= UPD765_FIFO_SIZE);
    upd->fifo_pos = 0;
    upd->fifo_num = num;
}

static inline void _upd765_fifo_wr(upd765_t* upd, uint8_t val) {
    CHIPS_ASSERT((upd->fifo_pos < UPD765_FIFO_SIZE) && (upd->fifo_pos < upd->fifo_num));
    upd->fifo[upd->fifo_pos++] = val;
}

static inline uint8_t _upd765_fifo_rd(upd765_t* upd) {
    CHIPS_ASSERT((upd->fifo_pos < UPD765_FIFO_SIZE) && (upd->fifo_pos < upd->fifo_num));
    return upd->fifo[upd->fifo_pos++];
}

/* called in IDLE phase when data byte written (this means start of a new command) */
static void _upd765_to_phase_command(upd765_t* upd, uint8_t data) {
    CHIPS_ASSERT(upd->phase == UPD765_PHASE_IDLE);
    upd->phase = UPD765_PHASE_COMMAND;
    upd->cmd = data & 0x1F;
    int num_cmd_bytes;
    switch (upd->cmd) {
        case UPD765_CMD_READ_DATA:
        case UPD765_CMD_READ_DELETED_DATA:
        case UPD765_CMD_WRITE_DATA:
        case UPD765_CMD_WRITE_DELETED_DATA:
        case UPD765_CMD_READ_A_TRACK:
        case UPD765_CMD_SCAN_EQUAL:
        case UPD765_CMD_SCAN_LOW_OR_EQUAL:
        case UPD765_CMD_SCAN_HIGH_OR_EQUAL:
            num_cmd_bytes = 9;
            break;
        case UPD765_CMD_READ_ID:
        case UPD765_CMD_RECALIBRATE:
        case UPD765_CMD_SENSE_DRIVE_STATUS:
            num_cmd_bytes = 2;
            break;
        case UPD765_CMD_SEEK:
        case UPD765_CMD_SPECIFY:
            num_cmd_bytes = 3;
            break;
        case UPD765_CMD_FORMAT_A_TRACK:
            num_cmd_bytes = 6;
            break;
        case UPD765_CMD_SENSE_INTERRUPT_STATUS:
        default:
            num_cmd_bytes = 1;
            break;
    }
    _upd765_fifo_reset(upd, num_cmd_bytes);
    _upd765_fifo_wr(upd, data);
}

/* may be called from _upd765_cmd() to transition to EXEC phase */
static void _upd765_to_phase_exec(upd765_t* upd) {
    CHIPS_ASSERT(upd->phase == UPD765_PHASE_COMMAND);
    upd->phase = UPD765_PHASE_EXEC;
}

/* called from any other phase to transition to IDLE phase (ready for new command */
static void _upd765_to_phase_idle(upd765_t* upd) {
    CHIPS_ASSERT(upd->phase != UPD765_PHASE_IDLE);
    upd->phase = UPD765_PHASE_IDLE;
}

/* may be called from _upd765_cmd() or during EXEC phase to prepare command result */
static void _upd765_to_phase_result(upd765_t* upd) {
    CHIPS_ASSERT((upd->phase == UPD765_PHASE_COMMAND) || (upd->phase == UPD765_PHASE_EXEC));
    upd->phase = UPD765_PHASE_RESULT;
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
            _upd765_fifo_reset(upd, 7);
            upd->fifo[0] = upd->st[0];
            upd->fifo[1] = upd->st[1];
            upd->fifo[2] = upd->st[2];
            upd->fifo[3] = upd->sector_info.c;
            upd->fifo[4] = upd->sector_info.h;
            upd->fifo[5] = upd->sector_info.r;
            upd->fifo[6] = upd->sector_info.n;
            break;
        case UPD765_CMD_SENSE_INTERRUPT_STATUS:
            _upd765_fifo_reset(upd, 2);
            upd->fifo[0] = upd->st[0];
            upd->fifo[1] = upd->sector_info.physical_track;
            break;
        case UPD765_CMD_SENSE_DRIVE_STATUS:
            _upd765_fifo_reset(upd, 1);
            upd->fifo[0] = upd->st[3];
            break;
        case UPD765_CMD_RECALIBRATE:
        case UPD765_CMD_SPECIFY:
        case UPD765_CMD_SEEK:
            /* this shouldn't actually happen */
            _upd765_to_phase_idle(upd);
            break;
        default:
            _upd765_fifo_reset(upd, 1);
            upd->fifo[0] = UPD765_ST0_IC;
            break;
    }
}

/* called after all command argument bytes have been read into fifo
   to execute command action and transition to next phase
*/
static void _upd765_cmd(upd765_t* upd) {
    CHIPS_ASSERT(upd->phase == UPD765_PHASE_COMMAND);
    switch (upd->cmd) {
        case UPD765_CMD_READ_DATA:
            {
                upd->st[0] = upd->fifo[1] & 7;      /* HD, US1, US0 */
                upd->sector_info.c = upd->fifo[2];
                upd->sector_info.h = upd->fifo[3];
                upd->sector_info.r = upd->fifo[4];
                upd->sector_info.n = upd->fifo[5];
                upd->sector_info.st1 = 0;
                upd->sector_info.st2 = 0;
                /* FIXME: handle length of read data via n=0 and DTL!=0xFF */
                // the following assert triggers in the CPC demo "CRTC"
                //CHIPS_ASSERT((upd->sector_info.n != 0) && (upd->fifo[8] == 0xFF));
                /* FIXME: handle read several sectors at a time via EOT arg */
                CHIPS_ASSERT(upd->sector_info.r == upd->fifo[6]);
                const int fdd_index = upd->st[0] & 3;
                const int side = (upd->st[0] & 4) >> 2;
                const int res = upd->seeksector_cb(fdd_index, side, &upd->sector_info, upd->user_data);
                if (UPD765_RESULT_SUCCESS == res) {
                    _upd765_to_phase_exec(upd);
                }
                else {
                    upd->st[0] |= UPD765_ST0_AT;
                    if (UPD765_RESULT_NOT_READY & res) {
                        upd->st[0] |= UPD765_ST0_NR;
                    }
                    if (UPD765_RESULT_NOT_FOUND & res) {
                        upd->st[1] |= UPD765_ST1_ND;
                    }
                    _upd765_to_phase_result(upd);
                }
            }
            break;

        case UPD765_CMD_READ_ID:
            {
                const int fdd_index = upd->fifo[1] & 3;
                const int side = (upd->fifo[1] & 4) >> 2;
                const int res = upd->trackinfo_cb(fdd_index, side, upd->user_data, &upd->sector_info);
                upd->st[0] = upd->fifo[1] & 7;
                upd->st[1] = upd->sector_info.st1;
                upd->st[2] = upd->sector_info.st2;
                if (res & UPD765_RESULT_NOT_READY) {
                    upd->st[0] |= UPD765_ST0_NR;
                }
                if (res & UPD765_RESULT_NOT_FOUND) {
                    upd->st[1] |= UPD765_ST1_ND;
                }
                _upd765_to_phase_result(upd);
            }
            break;

        case UPD765_CMD_RECALIBRATE:
            {
                /* set drive head to track 0 */
                const int fdd_index = upd->fifo[1] & 3;
                const int res = upd->seektrack_cb(fdd_index, 0, upd->user_data);
                upd->sector_info.physical_track = 0;
                upd->st[0] = fdd_index | UPD765_ST0_SE;
                if (UPD765_RESULT_SUCCESS != res) {
                    upd->st[0] |= UPD765_ST0_EC | UPD765_ST0_AT;
                }
                if (UPD765_RESULT_NOT_READY & res) {
                    upd->st[0] |= UPD765_ST0_NR;
                }
                _upd765_to_phase_idle(upd);
            }
            break;

        case UPD765_CMD_SENSE_INTERRUPT_STATUS:
            if (upd->st[0] & UPD765_ST0_SE) {
                /* on seek-end, return current track */
                const int fdd_index = upd->st[0] & 3;
                upd->trackinfo_cb(fdd_index, 0, upd->user_data, &upd->sector_info);
            }
            else {
                /* FIXME??? */
                upd->st[0] |= UPD765_ST0_ATRM;
            }
            _upd765_to_phase_result(upd);
            break;

        case UPD765_CMD_SENSE_DRIVE_STATUS:
            {
                const int fdd_index = upd->st[0] & 3;
                upd->driveinfo_cb(fdd_index, upd->user_data, &upd->drive_info);
                upd->st[3] = fdd_index;
                if (upd->drive_info.head > 0) {
                    upd->st[3] |= UPD765_ST3_HD;
                }
                if (upd->drive_info.sides > 1) {
                    upd->st[3] |= UPD765_ST3_TS;
                }
                if (upd->drive_info.physical_track == 0) {
                    upd->st[3] |= UPD765_ST3_T0;
                }
                if (upd->drive_info.ready) {
                    upd->st[3] |= UPD765_ST3_RY;
                }
                if (upd->drive_info.write_protected) {
                    upd->st[3] |= UPD765_ST3_WP;
                }
                if (upd->drive_info.fault) {
                    upd->st[3] |= UPD765_ST3_FT;
                }
                _upd765_to_phase_result(upd);
            }
            break;

        case UPD765_CMD_SPECIFY:
            /* nothing useful to do in specify, this just configures some
               timing and DMA-mode params that are not relevant for
               this emulation
            */
            _upd765_to_phase_idle(upd);
            break;

        case UPD765_CMD_SEEK:
            {
                /* seek to track in fifo 2 */
                const int fdd_index = upd->fifo[1] & 3;
                const int track = upd->fifo[2];
                const int res = upd->seektrack_cb(fdd_index, track, upd->user_data);
                upd->sector_info.physical_track = track;
                upd->st[0] = fdd_index | UPD765_ST0_SE;
                if (UPD765_RESULT_SUCCESS != res) {
                    upd->st[0] |= UPD765_ST0_EC | UPD765_ST0_AT;
                }
                if (UPD765_RESULT_NOT_READY & res) {
                    upd->st[0] |= UPD765_ST0_NR;
                }
                _upd765_to_phase_idle(upd);
            }
            break;

        case UPD765_CMD_READ_DELETED_DATA:
        case UPD765_CMD_WRITE_DATA:
        case UPD765_CMD_WRITE_DELETED_DATA:
        case UPD765_CMD_READ_A_TRACK:
        case UPD765_CMD_FORMAT_A_TRACK:
        case UPD765_CMD_SCAN_EQUAL:
        case UPD765_CMD_SCAN_LOW_OR_EQUAL:
        case UPD765_CMD_SCAN_HIGH_OR_EQUAL:
            /* NOT IMPLEMENTED */
            CHIPS_ASSERT(false);
            break;

        default:
            /* INVALID CMD */
            _upd765_to_phase_result(upd);
            break;
    }
}

/* called when a byte is read during the exec phase */
static uint8_t _upd765_exec_rd(upd765_t* upd) {
    CHIPS_ASSERT(upd->phase == UPD765_PHASE_EXEC);
    uint8_t data = 0xFF;
    switch (upd->cmd) {
        case UPD765_CMD_READ_DATA:
            {
                /* read next sector data byte from FDD */
                const int fdd_index = upd->st[0] & 3;
                const int side = (upd->st[0] & 4) >> 2;
                const int res = upd->read_cb(fdd_index, side, upd->user_data, &data);
                if (res != UPD765_RESULT_SUCCESS) {
                    if (res & UPD765_RESULT_NOT_READY) {
                        upd->st[0] |= UPD765_ST0_NR;
                    }
                    _upd765_to_phase_result(upd);
                }
            }
            break;
        default:
            /* shouldn't happen */
            CHIPS_ASSERT(false);
            break;
    }
    return data;
}

/* called when a byte is written during the exec phase */
static void _upd765_exec_wr(upd765_t* upd, uint8_t data) {
    // FIXME
    (void)upd;
    (void)data;
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
        /* if command is complete, execute the command action (and transition
           to next phase
        */
        if (upd->fifo_pos == upd->fifo_num) {
            _upd765_cmd(upd);
        }
    }
    else if (UPD765_PHASE_EXEC == upd->phase) {
        _upd765_exec_wr(upd, data);
    }
}

/* read a data byte from the upd765 */
static uint8_t _upd765_read_data(upd765_t* upd) {
    uint8_t data = 0xFF;
    if (UPD765_PHASE_RESULT == upd->phase) {
        /* read the next result byte */
        data = _upd765_fifo_rd(upd);
        if (upd->fifo_pos == upd->fifo_num) {
            /* all result bytes transfered, transition to idle phase */
            _upd765_to_phase_idle(upd);
        }
    }
    else if (UPD765_PHASE_EXEC == upd->phase) {
        data = _upd765_exec_rd(upd);
    }
    return data;
}

static inline uint8_t _upd765_read_status(upd765_t* upd) {
    // FIXME: drive bits 0..2 should be set while drive is seeking
    uint8_t status = 0;
    /* FIXME: RQM is a handshake flag and remains inactive
        for between 2us and 50us, for now just indicate
        that we're always ready during the command and result phase
    */
    /* FIXME: data direction is currently always set as FDC->CPU,
       since the emulation doesn't support write operations
    */
    switch (upd->phase) {
        case UPD765_PHASE_IDLE:
            status |= UPD765_STATUS_RQM;
            break;
        case UPD765_PHASE_COMMAND:
            status |= UPD765_STATUS_CB|UPD765_STATUS_RQM;
            break;
        case UPD765_PHASE_EXEC:
            status |= UPD765_STATUS_CB|UPD765_STATUS_EXM|UPD765_STATUS_DIO|UPD765_STATUS_RQM;
            break;
        case UPD765_PHASE_RESULT:
            status |= UPD765_STATUS_CB|UPD765_STATUS_DIO|UPD765_STATUS_RQM;
            break;
    }
    return status;
}

void upd765_init(upd765_t* upd, const upd765_desc_t* desc) {
    CHIPS_ASSERT(upd && desc);
    CHIPS_ASSERT(desc->seektrack_cb);
    CHIPS_ASSERT(desc->seeksector_cb);
    CHIPS_ASSERT(desc->read_cb);
    CHIPS_ASSERT(desc->trackinfo_cb);
    CHIPS_ASSERT(desc->driveinfo_cb);
    memset(upd, 0, sizeof(upd765_t));
    upd->seektrack_cb = desc->seektrack_cb;
    upd->seeksector_cb = desc->seeksector_cb;
    upd->read_cb = desc->read_cb;
    upd->trackinfo_cb = desc->trackinfo_cb;
    upd->driveinfo_cb = desc->driveinfo_cb;
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
                uint8_t s = _upd765_read_status(upd);
                UPD765_SET_DATA(pins, s);
                upd->status = s;
            }
        }
        else if (pins & UPD765_WR) {
            if (pins & UPD765_A0) {
                _upd765_write_data(upd, UPD765_GET_DATA(pins));
            }
        }
        upd->pins = pins;
    }
    return pins;
}

void upd765_snapshot_onsave(upd765_t* snapshot) {
    CHIPS_ASSERT(snapshot);
    snapshot->seektrack_cb = 0;
    snapshot->seeksector_cb = 0;
    snapshot->read_cb = 0;
    snapshot->trackinfo_cb = 0;
    snapshot->driveinfo_cb = 0;
    snapshot->user_data = 0;
}

void upd765_snapshot_onload(upd765_t* snapshot, upd765_t* sys) {
    CHIPS_ASSERT(snapshot && sys);
    snapshot->seektrack_cb = sys->seektrack_cb;
    snapshot->seeksector_cb = sys->seeksector_cb;
    snapshot->read_cb = sys->read_cb;
    snapshot->trackinfo_cb = sys->trackinfo_cb;
    snapshot->driveinfo_cb = sys->driveinfo_cb;
    snapshot->user_data = sys->user_data;
}
#endif /* CHIPS_IMPL */
