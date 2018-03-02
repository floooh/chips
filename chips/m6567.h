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
#define M6567_IRQ   (1ULL<<26)      /* shared with m6502 CPU */
#define M6567_BA    (1ULL<<28)      /* shared with m6502 RDY */

/* m6567 specific control pins */
#define M6567_CS    (1ULL<<40)
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
typedef uint16_t (*m6567_fetch_t)(uint16_t addr);

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
    /* visible CRT area blitted to rgba8_buffer (in pixels) */
    uint16_t vis_x, vis_y, vis_w, vis_h;
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
    /* internal state */
    uint16_t h_count;           /* horizontal count */
    uint16_t v_count;           /* vertical count */
    uint16_t h_total;
    uint16_t h_disppos;
    uint16_t h_dispend;
    uint16_t h_syncpos;
    uint16_t h_syncend;
    uint16_t v_total;
    uint16_t v_disppos;
    uint16_t v_dispend;
    uint16_t v_syncpos;
    uint16_t v_syncend;
    uint16_t border_left, border_right;
    uint16_t border_top, border_bottom;
    bool hs;            /* true during hsync */
    bool vs;            /* true during vsync */
    bool main_border;   /* main border flip-flop */
    bool vert_border;   /* vertical border flip flop */
    bool badline;       /* true when the badline state is active */
    bool frame_badlines_enabled;    /* true: when badlines are enabled in frame */
    bool display_state;             /* true: in display state, false: in idle state */
    bool irq;           /* state of the IRQ pin */

    uint16_t c_addr_or;     /* OR-mask for c-accesses, computed from mem_ptrs */
    uint16_t g_addr_and;    /* AND-mask for g-accesses, computed from mem_ptrs */
    uint16_t g_addr_or;     /* OR-mask for g-accesses, computed from ECM bit */
    uint16_t i_addr;        /* address for i-accesses, 0x3FFF or 0x39FF (if ECM bit set) */
    uint8_t g_mode;         /* mode 0..7 precomputed from ECM/BMM/MCM bits */
    uint32_t bc_rgba8;      /* border color as RGBA8, udpated when border color register is updated */
    uint32_t bg_rgba8[4];   /* background colors as RGBA8 */

    uint16_t vc;        /* 10-bit video counter */
    uint16_t vcbase;    /* 10-bit video counter base */
    uint8_t rc;         /* 3-bit raster counter */
    uint8_t vmli;       /* 6-bit 'vmli' video-matrix line buffer index */

    uint16_t crt_retrace_h;     /* hori retrace counter, started with h_sync */
    uint16_t crt_retrace_v;     /* vert retrace counter, started with v_sync */
    uint16_t crt_x, crt_y;      /* bream pos reset on crt_retrace_h/crt_retrace_v zero */
    uint16_t vis_x, vis_y;      /* current position in visible area */
    bool vis_enabled;           /* beam is currently in visible area */
    uint16_t vis_x0, vis_y0, vis_x1, vis_y1;  /* the visible area */
    uint16_t vis_w, vis_h;      /* width of visible area */

    uint16_t line_buffer[64];   /* 40x 8+4 bits line buffer (64 items because vmli is a 6-bit ctr) */
    uint8_t g_byte;             /* byte read by current g-access */
    uint8_t g_byte_prev;        /* previous g-access byte */

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
    /* vis area horizontal coords must be multiple of 8 */
    CHIPS_ASSERT((desc->vis_x & 7) == 0);
    CHIPS_ASSERT((desc->vis_w & 7) == 0);
    memset(vic, 0, sizeof(*vic));
    vic->type = desc->type;
    vic->fetch_cb = desc->fetch_cb;
    vic->rgba8_buffer = desc->rgba8_buffer;
    vic->vis_x0 = desc->vis_x/8;
    vic->vis_y0 = desc->vis_y;
    vic->vis_w = desc->vis_w/8;
    vic->vis_h = desc->vis_h;
    vic->vis_x1 = vic->vis_x0 + vic->vis_w;
    vic->vis_y1 = vic->vis_y0 + vic->vis_h;
    if (vic->type == M6567_TYPE_6569) {
        /* PAL-B */
        vic->h_total = 63;          /* 63 cycles per line */
        vic->h_syncpos = 61;
        vic->h_syncend = 10;

        vic->v_total = 312;         /* 312 lines total (PAL) */
        vic->v_syncpos = 300;
        vic->v_syncend = 7;
    }
    else {
        /* NTSC */
        vic->h_total = 65;          /* 65 cycles per line */
        vic->h_syncpos = 63;        /* start of hsync (6 chars right border) */
        vic->h_syncend = 10;        /* left border is 6 characters wide */

        vic->v_total = 263;         /* 263 lines total (NTSC) */
        vic->v_syncpos = 13;
        vic->v_syncend = 40;        /* this can't be right? */
    }
}

void m6567_reset(m6567_t* vic) {
    CHIPS_ASSERT(vic);
    memset(vic->regs, 0, sizeof(vic->regs));
    vic->g_mode = 0;
    vic->bc_rgba8 = _m6567_colors[0];
    for (int i = 0; i < 4; i++) {
        vic->bg_rgba8[i] = _m6567_colors[0];
    }
    vic->h_count = 0; vic->v_count = 0;
    vic->hs = false; vic->vs = false;
    vic->irq = false;
    vic->main_border = false;
    vic->vert_border = false;
    vic->crt_retrace_h = 0; vic->crt_retrace_v = 0;
    vic->crt_x = 0; vic->crt_y = 0;
    vic->vis_x = 0; vic->vis_y = 0;
    vic->vis_enabled = false;
}

void m6567_display_size(m6567_t* vic, int* out_width, int* out_height) {
    CHIPS_ASSERT(vic && out_width && out_height);
    *out_width = vic->vis_w*8;
    *out_height = vic->vis_h;
}

uint32_t m6567_color(int i) {
    CHIPS_ASSERT((i >= 0) && (i < 16));
    return _m6567_colors[i];
}

/* computes g-access mask, and address for idle accesses */
void _m6567_update_fetch_addr(m6567_t* vic, uint8_t mem_ptrs, uint8_t ctrl_1) {
    /* c-access: addr=|VM13|VM12|VM11|VM10|VC9|VC8|VC7|VC6|VC5|VC4|VC3|VC2|VC1|VC0| */
    vic->c_addr_or = (vic->mem_ptrs & 0xF0)<<6;
    /* g-access: addr=|CB13|CB12|CB11|D7|D6|D5|D4|D3|D2|D1|D0|RC2|RC1|RC0| */
    vic->g_addr_or = (mem_ptrs & 0x0E)<<10;
    vic->g_addr_and = 0xFFFF;
    vic->i_addr = 0x3FFF;
    /* "...If the ECM bit is set, the address generator always holds the
        address lines 9 and 10 low without any other changes to the
        addressing scheme"
    */
    if (ctrl_1 & (1<<6)) {
        vic->g_addr_and &= ~((1<<10)|(1<<9));
        vic->i_addr &= ~((1<<10)|(1<<9));
    }
}

/* updates the current display mode (0..7) from the ECM/BMM/MCM bits */
void _m6567_update_mode(m6567_t* vic, uint8_t ctrl_1, uint8_t ctrl_2) {
    vic->g_mode = ((ctrl_1&((1<<6)|(1<<5)))|(ctrl_2&(1<<4)))>>4;
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
                    data = (vic->regs[0x11] & 0x7F) | ((vic->v_count & 0x100)>>1);
                    break;
                case 0x12:
                    /* reading 0x12 returns bits 0..7 of current raster position */
                    data = (uint8_t)vic->v_count;
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
            const uint8_t data = M6567_GET_DATA(pins) & _m6567_reg_mask[reg_addr];
            bool write = true;
            switch (reg_addr) {
                case 0x11:
                    /* ctrl_1 */
                    if (data & (1<<3)) {
                        /* RSEL 1: 25 rows */
                        vic->border_top = 51;
                        vic->border_bottom = 251;
                    }
                    else {
                        /* RSEL 0: 24 rows */
                        vic->border_top = 55;
                        vic->border_bottom = 247;
                    }
                    /* ECM bit updates the precomputed fetch address masks */
                    _m6567_update_fetch_addr(vic, vic->mem_ptrs, data);
                    /* update the graphics mode */
                    _m6567_update_mode(vic, data, vic->ctrl_2);
                    break;
                case 0x16:
                    /* ctrl_2 */
                    if (data & (1<<3)) {
                        /* CSEL 1: 40 columns */
                        vic->border_left = 15;
                        vic->border_right = 55;
                    }
                    else {
                        /* CSEL 0: 38 columns */
                        vic->border_left = 16;
                        vic->border_right = 54;
                    }
                    /* update the graphics mode */
                    _m6567_update_mode(vic, vic->ctrl_1, data);
                    break;
                case 0x18:
                    /* memory-ptrs register , update precomputed fetch address masks */
                    _m6567_update_fetch_addr(vic, data, vic->ctrl_1);
                    break;
                case 0x1E:
                case 0x1F:
                    /* mob collision registers cannot be written */
                    write = false;
                    break;
                case 0x20:
                    /* border color */
                    vic->bc_rgba8 = _m6567_colors[data & 0xF];
                    break;
                case 0x21:
                case 0x22:
                case 0x23:
                case 0x24:
                    /* background colors */
                    vic->bg_rgba8[reg_addr-0x21] = _m6567_colors[data & 0xF];
                    break;
            }
            if (write) {
                vic->regs[reg_addr] = data;
            }
        }
    }
    return pins;
}

/*=== TICK FUNCTION ==========================================================*/
uint64_t m6567_tick(m6567_t* vic, uint64_t pins) {
    /*--- update raster counters ---------------------------------------------*/
    {
        if (vic->h_count == vic->h_total) {
            vic->h_count = 0;
            /* FIXME: only if raster interrupt enabled!
            vic->irq = true;
            */
        }
        else {
            vic->h_count++;
            vic->irq = false;
        }
        if (vic->h_count == vic->h_syncpos) {
            vic->hs = true;
            /* retrace the CRT beam 7 ticks after htick starts */
            vic->crt_retrace_h = 7;
        }
        if (vic->h_count == vic->h_syncend) {
            vic->hs = false;
        }

        if (vic->h_count == 0) {
            /* new scanline */
            if (vic->v_count == vic->v_total) {
                vic->v_count = 0;
            }
            else {
                vic->v_count++;
            }
            if (vic->v_count == vic->v_syncpos) {
                vic->vs = true;
                vic->crt_retrace_v = 3;
            }
            if (vic->v_count == vic->v_syncend) {
                vic->vs = false;
            }
        }
    }

    /*--- update badline state -----------------------------------------------*/
    /* (see 3.5 http://www.zimmers.net/cbmpics/cbm/c64/vic-ii.txt) */
    {
        if ((vic->v_count >= 0x30) && (vic->v_count <= 0xF7)) {
            /* DEN bit must have been set in raster line 30 */
            if ((vic->v_count == 0x30) && (vic->ctrl_1 & (1<<4))) {
                vic->frame_badlines_enabled = true;
            }
            /* a badline is active when the low 3 bits of raster position
               are identical with YSCROLL
            */
            vic->badline = (vic->frame_badlines_enabled && ((vic->v_count&7)==(vic->ctrl_1&7)));
        }
        else {
            vic->frame_badlines_enabled = false;
            vic->badline = false;
        }
    }

    /*--- update the display/idle state --------------------------------------*/
    /* (see 3.7.1 in http://www.zimmers.net/cbmpics/cbm/c64/vic-ii.txt) */
    {
        if (vic->badline) {
            vic->display_state = true;
        }
        else if ((vic->h_count == 58) && (vic->rc == 7)) {
            vic->display_state = false;
        }
    }

    /*--- update counters and memory accesses --------------------------------*/
    /* (see 3.7.2 in http://www.zimmers.net/cbmpics/cbm/c64/vic-ii.txt) */
    bool ba = false;
    bool aec = false;
    bool c_access = false;
    bool g_access = false;
    bool i_access = false;
    {
        /* 1. Once somewhere outside of the range of raster lines $30-$f7 (i.e.
            outside of the Bad Line range), VCBASE is reset to zero. This is
            presumably done in raster line 0, the exact moment cannot be determined
            and is irrelevant.
        */
        if (vic->v_count == 0) {
            vic->vcbase = 0;
        }
        /* 2. In the first phase of cycle 14 of each line, VC is loaded from VCBASE
            (VCBASE->VC) and VMLI is cleared. If there is a Bad Line Condition in
            this phase, RC is also reset to zero.
        */
        if (vic->h_count == 14) {
            vic->vc = vic->vcbase;
            vic->vmli = 0;
            if (vic->badline) {
                vic->rc = 0;
            }
        }
        /* 3. If there is a Bad Line Condition in cycles 12-54, BA is set low and the
              c-accesses are started.
              Also set the AEC pin 3 cycles later.
        */
        if ((vic->h_count >= 12) && (vic->h_count <= 54)) {
            if (vic->badline) {
                ba = true;
                if (vic->h_count >= 15) {
                    aec = ba;
                    c_access = true;
                }
            }
            if (vic->display_state && (vic->h_count >= 15)) {
                g_access = true;
                if (vic->h_count == 15) {
                    /* prevent ghost-byte from leaking into display area */
                    vic->g_byte_prev = 0;
                }
                else {
                    vic->g_byte_prev = vic->g_byte;
                }
            }
        }

        /* 5. In the first phase of cycle 58, the VIC checks if RC=7. If so, the video
            logic goes to idle state and VCBASE is loaded from VC (VC->VCBASE). If
            the video logic is in display state afterwards (this is always the case
            if there is a Bad Line Condition), RC is incremented.
        */
        if (vic->h_count == 58) {
            if (vic->rc == 7) {
                vic->vcbase = vic->vc;
            }
            if (vic->display_state) {
                vic->rc = (vic->rc + 1) & 7;
            }
        }
    }

    /*--- perform memory fetches ---------------------------------------------*/
    /* (see 3.6.3. in http://www.zimmers.net/cbmpics/cbm/c64/vic-ii.txt) */
    {
        uint16_t addr;
        if (c_access) {
            /* addr=|VM13|VM12|VM11|VM10|VC9|VC8|VC7|VC6|VC5|VC4|VC3|VC2|VC1|VC0| */
            addr = vic->vc | vic->c_addr_or;
            vic->line_buffer[vic->vmli] = vic->fetch_cb(addr);
        }
        if (g_access) {
            if (vic->ctrl_1 & (1<<5)) {
                /* bitmap mode: addr=|CB13|VC9|VC8|VC7|VC6|VC5|VC4|VC3|VC2|VC1|VC0|RC2|RC1|RC0| */
                addr = vic->vc<<3 | vic->rc;
                addr = (addr | (vic->g_addr_or & (1<<13))) & vic->g_addr_and;
            }
            else {
                /* text mode: addr=|CB13|CB12|CB11|D7|D6|D5|D4|D3|D2|D1|D0|RC2|RC1|RC0| */
                addr = ((vic->line_buffer[vic->vmli]&0xFF)<<3) | vic->rc;
                addr = (addr | vic->g_addr_or) & vic->g_addr_and;
            }
            vic->g_byte = vic->fetch_cb(addr);
        }
        else {
            /* an idle access (all 14 address bits active) */
            /* FIXME: 0x39FF if the ECM bit is set */
            vic->g_byte = vic->fetch_cb(vic->i_addr);
            i_access = true;
        }
    }

    /*--- update the border flip-flops ---------------------------------------*/
    /* (see 3.9 in http://www.zimmers.net/cbmpics/cbm/c64/vic-ii.txt) */
    {
        /* 1. If the X coordinate reaches the right comparison value, the main border
              flip flop is set.
        */
        if (vic->h_count == vic->border_right) {
            vic->main_border = true;
        }
        if (vic->h_count == 0) {
            /* 2. If the Y coordinate reaches the bottom comparison value in cycle 63, the
                  vertical border flip flop is set.
            */
            if (vic->v_count == vic->border_bottom) {
                vic->vert_border = true;
            }
            /* 3. If the Y coordinate reaches the top comparison value in cycle 63 and the
                  DEN bit in register $d011 is set, the vertical border flip flop is
                  reset.
            */
            if ((vic->v_count == vic->border_top) && (vic->ctrl_1 & (1<<4))) {
                vic->vert_border = false;
            }
        }
        if (vic->h_count == vic->border_left) {
            /* 4. If the X coordinate reaches the left comparison value and the Y
                  coordinate reaches the bottom one, the vertical border flip flop is set.
            */
            if (vic->v_count == vic->border_bottom) {
                vic->vert_border = true;
            }
            /* 5. If the X coordinate reaches the left comparison value and the Y
                  coordinate reaches the top one and the DEN bit in register $d011 is set,
                  the vertical border flip flop is reset.
            */
            if ((vic->v_count == vic->border_top) && (vic->ctrl_1 & (1<<4))) {
                vic->vert_border = false;
            }
            /* 6. If the X coordinate reaches the left comparison value and the vertical
                  border flip flop is not set, the main flip flop is reset.
            */
            if (!vic->vert_border) {
                vic->main_border = false;
            }
        }
    }

    /*--- update the CRT beam position ---------------------------------------*/
    {
        /* update current beam pos */
        vic->crt_x++;
        if (vic->crt_retrace_h > 0) {
            if (--vic->crt_retrace_h == 0) {
                vic->crt_x = 0;
                vic->crt_y++;
                if (vic->crt_retrace_v > 0) {
                    if (--vic->crt_retrace_v == 0) {
                        vic->crt_y = 0;
                    }
                }
            }
        }
        /* update visible-area coordinates and enabled-state */
        if ((vic->crt_x >= vic->vis_x0) && (vic->crt_x < vic->vis_x1) &&
            (vic->crt_y >= vic->vis_y0) && (vic->crt_y < vic->vis_y1))
        {
            vic->vis_enabled = true;
            vic->vis_x = vic->crt_x - vic->vis_x0;
            vic->vis_y = vic->crt_y - vic->vis_y0;
        }
        else {
            vic->vis_enabled = false;
        }
    }

    /*--- decode pixels into framebuffer -------------------------------------*/
    {
        /* FIXME: when XSCROLLing, the background color must scroll as well
           in bitmap mode!
        */

        if (vic->vis_enabled && !(vic->hs||vic->vs)) {
            const int dst_x = vic->vis_x * 8;
            const int dst_y = vic->vis_y;
            uint32_t* dst = vic->rgba8_buffer + dst_y*vic->vis_w*8 + dst_x;
            /*
                "The main border flip flop controls the border display. If it is set, the
                VIC displays the color stored in register $d020, otherwise it displays the
                color that the priority multiplexer switches through from the graphics or
                sprite data sequencer. So the border overlays the text/bitmap graphics as
                well as the sprites. It has the highest display priority."
            */
            if (vic->main_border) {
                const uint32_t c = vic->bc_rgba8;
                for (int i = 0; i < 8; i++) {
                    dst[i] = c;
                }
            }
            /*
                "...the vertical border flip flop controls the output of the graphics
                data sequencer. The sequencer only outputs data if the flip flop is
                not set..."
            */
            else if (!vic->vert_border) {
                const int xscroll = vic->ctrl_2 & 7;
                switch (vic->g_mode) {
                    case 0:
                        /* ECM/BMM/MCM=000, standard text mode */
                        {
                            const uint32_t bg = vic->bg_rgba8[0];
                            /* on idle access, video-matrix-data is treated as a '0' */
                            const uint32_t fg = _m6567_colors[i_access ? 0 : (vic->line_buffer[vic->vmli]>>8)&0xF];
                            const uint8_t pixels = (uint8_t) (((uint16_t)((vic->g_byte_prev<<8)|vic->g_byte))>>xscroll);
                            dst[0] = (pixels & 0x80) ? fg : bg;
                            dst[1] = (pixels & 0x40) ? fg : bg;
                            dst[2] = (pixels & 0x20) ? fg : bg;
                            dst[3] = (pixels & 0x10) ? fg : bg;
                            dst[4] = (pixels & 0x08) ? fg : bg;
                            dst[5] = (pixels & 0x04) ? fg : bg;
                            dst[6] = (pixels & 0x02) ? fg : bg;
                            dst[7] = (pixels & 0x01) ? fg : bg;
                        }
                        break;
                    case 1:
                        /* ECM/BMM/MCM=001, multicolor text mode */
                        {
                            /* only 3 bits for foreground color of '11' pixels */
                            const uint16_t c_data = i_access ? 0 : vic->line_buffer[vic->vmli];
                            const uint32_t fg = _m6567_colors[(c_data>>8) & 0x7];
                            const uint8_t pixels = (uint8_t) (((uint16_t)((vic->g_byte_prev<<8)|vic->g_byte))>>xscroll);
                            if (c_data & (1<<11)) {
                                /* multi-color character at half resolution */
                                dst[0] = dst[1] = (pixels&0xC0)==0xC0 ? fg : vic->bg_rgba8[(pixels&0xC0)>>6];
                                dst[2] = dst[3] = (pixels&0x30)==0x30 ? fg : vic->bg_rgba8[(pixels&0x30)>>4];
                                dst[4] = dst[5] = (pixels&0x0C)==0x0C ? fg : vic->bg_rgba8[(pixels&0x0C)>>2];
                                dst[6] = dst[7] = (pixels&0x03)==0x03 ? fg : vic->bg_rgba8[(pixels&0x03)];
                            }
                            else {
                                /* standard-text-mode character */
                                const uint32_t bg = vic->bg_rgba8[0];
                                dst[0] = (pixels & 0x80) ? fg : bg;
                                dst[1] = (pixels & 0x40) ? fg : bg;
                                dst[2] = (pixels & 0x20) ? fg : bg;
                                dst[3] = (pixels & 0x10) ? fg : bg;
                                dst[4] = (pixels & 0x08) ? fg : bg;
                                dst[5] = (pixels & 0x04) ? fg : bg;
                                dst[6] = (pixels & 0x02) ? fg : bg;
                                dst[7] = (pixels & 0x01) ? fg : bg;
                            }
                        }
                        break;
                    case 2:
                        /* ECM/BMM/MCM=010, standard bitmap mode */
                        {
                            const uint16_t c_data = i_access ? 0 : vic->line_buffer[vic->vmli];
                            const uint32_t fg = _m6567_colors[(c_data>>4) & 0xF];
                            const uint32_t bg = _m6567_colors[c_data & 0xF];
                            const uint8_t pixels = (uint8_t) (((uint16_t)((vic->g_byte_prev<<8)|vic->g_byte))>>xscroll);
                            dst[0] = (pixels & 0x80) ? fg : bg;
                            dst[1] = (pixels & 0x40) ? fg : bg;
                            dst[2] = (pixels & 0x20) ? fg : bg;
                            dst[3] = (pixels & 0x10) ? fg : bg;
                            dst[4] = (pixels & 0x08) ? fg : bg;
                            dst[5] = (pixels & 0x04) ? fg : bg;
                            dst[6] = (pixels & 0x02) ? fg : bg;
                            dst[7] = (pixels & 0x01) ? fg : bg;
                        }
                        break;
                    default:
                        /* invalid mode */
                        for (int i = 0; i < 8; i++) {
                            dst[i] = 0xFFFF00FF;
                        }
                        break;
                }
            }
            /*
                "...otherwise it displays the background color"
            */
            else {
                const uint32_t c = vic->bg_rgba8[0];
                for (int i = 0; i < 8; i++) {
                    dst[i] = c;
                }
            }
        }
    }

    /*--- bump the VC and vmli counters --------------------------------------*/
    if (g_access) {
        vic->vc = (vic->vc + 1) & 0x3FF;        /* VS is a 10-bit counter */
        vic->vmli = (vic->vmli + 1) & 0x3F;     /* VMLI is a 6-bit counter */
    }

    /*--- set CPU pins -------------------------------------------------------*/
    if (ba) {
        pins |= M6567_BA;
    }
    if (aec) {
        pins |= M6567_AEC;
    }
    /* FIXME
    if (vic->irq) {
        pins |= M6567_IRQ;
    }
    */
    return pins;
}
#endif /* CHIPS_IMPL */

#ifdef __cplusplus
} /* extern "C" */
#endif
