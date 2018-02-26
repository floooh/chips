#pragma once
/*#
    # m6567.h

    MOS Technology 6567 / 6569 emulator (aka VIC-II)

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
    *    CS --->|           |---> A0  *
    *    RW --->|           |...      *
    *   IRQ <---|           |---> A13 *
    *    BA <---|           |         *
    *   AEC <---|   M6567   |<--> D0  *
    *           |   M6569   |...      *
    *           |           |<--> D7  *
    *           |           |<--- D8  *
    *           |           |...      *
    *           |           |<--- D11 *
    *           |           |         *
    *           +-----------+         *
    ***********************************

    The real VIC-II has multiplexed address bus pins, the emulation
    doesn't.

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

/* address bus lines (with CS active A0..A6 is register address) */
#define M6567_A0    (1ULL<<0)
#define M6567_A1    (1ULL<<1)
#define M6567_A2    (1ULL<<2)
#define M6567_A3    (1ULL<<3)
#define M6567_A4    (1ULL<<4)
#define M6567_A5    (1ULL<<5)
#define M6567_A6    (1ULL<<6)
#define M6567_A7    (1ULL<<7)
#define M6567_A8    (1ULL<<8)
#define M6567_A9    (1ULL<<9)
#define M6567_A10   (1ULL<<10)
#define M6567_A11   (1ULL<<11)
#define M6567_A12   (1ULL<<12)
#define M6567_A13   (1ULL<<13)

/* data bus pins D0..D7 */
#define M6567_D0    (1ULL<<16)
#define M6567_D1    (1ULL<<17)
#define M6567_D2    (1ULL<<18)
#define M6567_D3    (1ULL<<19)
#define M6567_D4    (1ULL<<20)
#define M6567_D5    (1ULL<<21)
#define M6567_D6    (1ULL<<22)
#define M6567_D7    (1ULL<<23)

/* shared control pins */
#define M6567_RW    (1ULL<<24)      /* shared with m6502 CPU */
#define M6502_IRQ   (1ULL<<26)      /* shared with m6502 CPU */

/* m6567 specific control pins */
#define M6567_CS    (1ULL<<40)
#define M6567_BA    (1ULL<<41)
#define M6567_AEC   (1ULL<<42)

/* remaining data bus pins (input only) */
#define M6567_DB8   (1ULL<<48)
#define M6567_DB9   (1ULL<<49)
#define M6567_DB10  (1ULL<<50)
#define M6567_DB11  (1ULL<<51)

/* number of registers */
#define M6567_NUM_REGS (64)
/* register address mask */
#define M6567_REG_MASK (M6567_NUM_REGS-1)
/* number of sprites */
#define M6567_NUM_MOBS (8)

/* extract 8-bit data bus from 64-bit pins */
#define M6567_GET_DATA(p) ((uint8_t)((p&0xFF0000ULL)>>16))
/* merge 8-bit data bus value into 64-bit pins */
#define M6567_SET_DATA(p,d) {p=(((p)&~0xFF0000ULL)|(((d)<<16)&0xFF0000ULL));}

/* memory fetch callback, used to feed pixel- and color-data into the m6567 */
typedef uint64_t (*m6567_fetch_t)(uint64_t pins);

/* chip subtypes */
typedef enum {
    M6567_TYPE_6567R8 = 0,      /* NTSC-M variant */
    M6567_TYPE_6569,            /* PAL-B variant */
} m6567_type_t;

/* setup parameters for m6567_init() function */
typedef struct {
    /* chip subtype (default is NTSC type 6567R8) */
    m6567_type_t type;
    /* pointer to RGBA8 framebuffer for generated image */
    uint32_t* rgba8_buffer;
    /* size of the RGBA framebuffer (must be at least 418x284) */
    uint32_t rgba8_buffer_size;
    /* the memory-fetch callback */
    m6567_fetch_t fetch_cb;
} m6567_desc_t;

/* the m6567 state structure */
typedef struct {
    m6567_type_t type;
    union {
        uint8_t regs[M6567_NUM_REGS];
        struct {
            uint8_t mob_xy[M6567_NUM_MOBS][2];  /* sprite X/Y coords */
            uint8_t mob_x_msb;                  /* x coordinate MSBs */
            uint8_t ctrl_1;                     /* control register 1 */
            uint8_t raster;                     /* raster counter */
            uint8_t lightpen_xy[2];             /* lightpen coords */
            uint8_t mob_enabled;                /* sprite enabled bits */
            uint8_t ctrl_2;                     /* control register 2 */
            uint8_t mob_yexp;                   /* sprite Y expansion */
            uint8_t mem_ptrs;                   /* memory pointers */
            uint8_t interrupt;                  /* interrupt register */
            uint8_t int_enable;                 /* interrupt enabled bits */
            uint8_t mob_data_priority;          /* sprite data priority bits */
            uint8_t mob_multicolor;             /* sprite multicolor bits */
            uint8_t mob_xexp;                   /* sprite X expansion */
            uint8_t mob_mob_coll;               /* sprite-sprite collision bits */
            uint8_t mob_data_coll;              /* sprite-data collision bits */
            uint8_t border_color;               /* border color */
            uint8_t background_color[4];        /* background colors */
            uint8_t mob_mc[2];                  /* sprite multicolor 0 */
            uint8_t mob_color[8];               /* sprite colors */
            uint8_t unused[17];                 /* not writable, return 0xFF on read */
        };
    };
    /* internal counters */
    uint16_t h_count;
    uint16_t h_period;
    uint16_t l_count;
    uint16_t l_period;

    m6567_fetch_t fetch_cb;
    uint32_t* rgba8_buffer;
} m6567_t;

/* initialize a new m6567_t instance */
extern void m6567_init(m6567_t* vic, m6567_desc_t* desc);
/* reset a m6567_t instance */
extern void m6567_reset(m6567_t* vic);
/* get the visible display size in pixels (different for PAL/NTSC) */
extern void m6567_display_size(m6567_t* vic, int* out_width, int* out_height);
/* read/write m6567 registers */
extern uint64_t m6567_iorq(m6567_t* vic, uint64_t pins);
/* tick the m6567_y instance */
extern uint64_t m6567_tick(m6567_t* vic, uint64_t pins);
/* get 32-bit RGBA8 value from color index (0..15) */
extern uint32_t m6567_color(int i);

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

/* color palette (see: http://unusedino.de/ec64/technical/misc/vic656x/colors/) */
#define _M6567_RGBA8(r,g,b) (0xFF000000|(b<<16)|(g<<8)|(r))
static const uint32_t _m6567_colors[16] = {
    _M6567_RGBA8(0x00,0x00,0x00),     /* 0: black */
    _M6567_RGBA8(0xFF,0xFF,0xFF),     /* 1: white */
    _M6567_RGBA8(0x68,0x37,0x2B),     /* 2: red */
    _M6567_RGBA8(0x70,0xA4,0xB2),     /* 3: cyan */
    _M6567_RGBA8(0x6F,0x3D,0x86),     /* 4: purple */
    _M6567_RGBA8(0x58,0x8D,0x43),     /* 5: green */
    _M6567_RGBA8(0x35,0x28,0x79),     /* 6: blue */
    _M6567_RGBA8(0xB8,0xC7,0x6F),     /* 7: yellow */
    _M6567_RGBA8(0x6F,0x4F,0x25),     /* 8: orange */
    _M6567_RGBA8(0x43,0x39,0x00),     /* 9: brown */
    _M6567_RGBA8(0x9A,0x67,0x59),     /* A: light red */
    _M6567_RGBA8(0x44,0x44,0x44),     /* B: dark grey */
    _M6567_RGBA8(0x6C,0x6C,0x6C),     /* C: grey */
    _M6567_RGBA8(0x9A,0xD2,0x84),     /* D: light green */
    _M6567_RGBA8(0x6C,0x5E,0xB5),     /* E: light blue */
    _M6567_RGBA8(0x95,0x95,0x95)      /* F: light grey */
};

/* valid register bits */
static const uint8_t _m6567_reg_mask[M6567_NUM_REGS] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,     /* mob 0..3 xy */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,     /* mob 4..7 xy */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,     /* msbx, ctrl1, raster, lpx, lpy, mob enabled */
    0x3F,   /* ctrl2 */
    0xFF,   /* mob y expansion */
    0xFE,   /* memory pointers */
    0x8F,   /* interrupt register */
    0x0F,   /* interrupt enabled */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   /* mob data pri, multicolor, x expansion, mob-mob coll, mob-data coll */
    0x0F,                       /* border color */
    0x0F, 0x0F, 0x0F, 0x0F,     /* background colors */
    0x0F, 0x0F,                 /* sprite multicolor 0,1 */
    0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,     /* sprite colors */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     /* unused 0..7 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     /* unused 8..15 */
    0x00                                                /* unused 16 */
};

void m6567_init(m6567_t* vic, m6567_desc_t* desc) {
    CHIPS_ASSERT(vic && desc);
    CHIPS_ASSERT(desc->rgba8_buffer_size >= (418*284));
    CHIPS_ASSERT((desc->type == M6567_TYPE_6567R8) || (desc->type == M6567_TYPE_6569));
    memset(vic, 0, sizeof(*vic));
    vic->type = desc->type;
    vic->fetch_cb = desc->fetch_cb;
    vic->rgba8_buffer = desc->rgba8_buffer;
    if (vic->type == M6567_TYPE_6569) {
        /* PAL-B */
        vic->h_period = 63;     /* 63 cycles per line */
        vic->l_period = 312;    /* 312 lines per frame */
    }
    else {
        /* NTSC */
        vic->h_period = 65;     /* 65 cycles per line */
        vic->l_period = 263;    /* 263 lines per frame */
    }
}

void m6567_reset(m6567_t* vic) {
    CHIPS_ASSERT(vic);
    vic->h_count = 0;
    vic->l_count = 0;
    memset(vic->regs, 0, sizeof(vic->regs));
}

void m6567_display_size(m6567_t* vic, int* out_width, int* out_height) {
    CHIPS_ASSERT(vic && out_width && out_height);
    if (vic->type == M6567_TYPE_6569) {
        /* PAL-B */
        *out_width = 403;
        *out_height = 284;
    }
    else {
        /* NTSC */
        *out_width = 418;
        *out_height = 235;
    }
}

uint64_t m6567_iorq(m6567_t* vic, uint64_t pins) {
    if (pins & M6567_CS) {
        uint8_t reg_addr = pins & M6567_REG_MASK;
        if (pins & M6567_RW) {
            /* read register, with some special cases */
            uint8_t data;
            switch (reg_addr) {
                case 0x11:
                    /* bit 7 of 0x11 is bit 8 of the current raster counter */
                    data = (vic->regs[0x11] & 0x7F) | ((vic->l_count & 0x100)>>1);
                    break;
                case 0x12:
                    /* reading 0x12 returns bits 0..7 of current raster position */
                    data = (uint8_t)vic->l_count;
                    break;
                case 0x1E:
                case 0x1F:
                    /* registers 0x1E and 0x1F (mob collisions) are cleared on reading */
                    data = vic->regs[reg_addr];
                    vic->regs[reg_addr] = 0;
                    break;
                default:
                    /* unconnected bits are returned as 1 */
                    data = vic->regs[reg_addr] | ~_m6567_reg_mask[reg_addr];
                    break;
            }
            M6567_SET_DATA(pins, data);
        }
        else {
            /* write register, with special cases */
            switch (reg_addr) {
                case 0x1E:
                case 0x1F:
                    /* mob collision registers cannot be written */
                    break;
                default:
                    vic->regs[reg_addr] = M6567_GET_DATA(pins) & _m6567_reg_mask[reg_addr];
                    break;
            }
        }
    }
    return pins;
}

uint64_t m6567_tick(m6567_t* vic, uint64_t pins) {
    vic->h_count++;
    if (vic->h_count >= vic->h_period) {
        /* new raster line */
        vic->h_count = 0;
        vic->l_count++;
        if (vic->l_count >= vic->l_period) {
            /* new frame */
            vic->l_count = 0;
        }
    }
    return pins;
}

uint32_t m6567_color(int i) {
    CHIPS_ASSERT((i >= 0) && (i < 16));
    return _m6567_colors[i];
}
#endif /* CHIPS_IMPL */

#ifdef __cplusplus
} /* extern "C" */
#endif
