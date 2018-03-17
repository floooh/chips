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
#define M6567_AEC   (1ULL<<29)      /* shared with m6510 AEC */

/* m6567 specific control pins */
#define M6567_CS    (1ULL<<40)

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
    /* size of the RGBA framebuffer (must be at least 512x312) */
    uint32_t rgba8_buffer_size;
    /* visible CRT area blitted to rgba8_buffer (in pixels) */
    uint16_t vis_x, vis_y, vis_w, vis_h;
    /* the memory-fetch callback */
    m6567_fetch_t fetch_cb;
} m6567_desc_t;

/* register bank */
typedef struct {
    union {
        uint8_t regs[M6567_NUM_REGS];
        struct {
            uint8_t mxy[M6567_NUM_MOBS][2];     /* sprite X/Y coords */
            uint8_t mx8;                        /* x coordinate MSBs */
            uint8_t ctrl_1;                     /* control register 1 */
            uint8_t raster;                     /* raster counter */
            uint8_t lightpen_xy[2];             /* lightpen coords */
            uint8_t me;                         /* sprite enabled bits */
            uint8_t ctrl_2;                     /* control register 2 */
            uint8_t mye;                        /* sprite Y expansion */
            uint8_t mem_ptrs;                   /* memory pointers */
            uint8_t int_latch;                  /* interrupt latch */
            uint8_t int_mask;                   /* interrupt-enabled mask */
            uint8_t mob_data_priority;          /* sprite data priority bits */
            uint8_t mob_multicolor;             /* sprite multicolor bits */
            uint8_t mxe;                        /* sprite X expansion */
            uint8_t mob_mob_coll;               /* sprite-sprite collision bits */
            uint8_t mob_data_coll;              /* sprite-data collision bits */
            uint8_t border_color;               /* border color */
            uint8_t background_color[4];        /* background colors */
            uint8_t mob_mc[2];                  /* sprite multicolor 0 */
            uint8_t mob_color[8];               /* sprite colors */
            uint8_t unused[17];                 /* not writable, return 0xFF on read */
        };
    };
} _m6567_registers_t;

/* control- and interrupt-register bits */
#define M6567_CTRL1_RST8    (1<<7)
#define M6567_CTRL1_ECM     (1<<6)
#define M6567_CTRL1_BMM     (1<<5)
#define M6567_CTRL1_DEN     (1<<4)
#define M6567_CTRL1_RSEL    (1<<3)
#define M6567_CTRL1_YSCROLL ((1<<2)|(1<<1)|(1<<0))
#define M6567_CTRL2_RES     (1<<5)
#define M6567_CTRL2_MCM     (1<<4)
#define M6567_CTRL2_CSEL    (1<<3)
#define M6567_CTRL2_XSCROLL ((1<<2)|(1<<1)|(1<<0))
#define M6567_INT_IRQ       (1<<7)      /* int_latch: interrupt requested */
#define M6567_INT_ILP       (1<<3)      /* int_latch: lightpen interrupt */
#define M6567_INT_IMMC      (1<<2)      /* int_latch: mob/mob collision interrupt */
#define M6567_INT_IMBC      (1<<1)      /* int_latch: mob/bitmap collision interrupt */
#define M6567_INT_IRST      (1<<0)      /* int_latch: raster interrupt */
#define M6567_INT_ELP       (1<<3)      /* int_mask: lightpen interrupt enabled */
#define M6567_INT_EMMC      (1<<2)      /* int_mask: mob/mob collision interrupt enabled */
#define M6567_INT_EMBC      (1<<1)      /* int_mask: mob/bitmap collision interrupt enabled */
#define M6567_INT_ERST      (1<<0)      /* int_mask: raster interrupt enabled */

/* raster unit state */
typedef struct {
    uint16_t h_count, h_total;
    uint16_t v_count, v_total;
    uint16_t v_irqline;     /* raster interrupt line, updated when ctrl_1 or raster is written */
    uint16_t sh_count;      /* separate counter for sprite, reset at h_count=55 */
    uint16_t vc;            /* 10-bit video counter */
    uint16_t vc_base;       /* 10-bit video counter base */
    uint8_t rc;             /* 3-bit raster counter */
    bool display_state;             /* true: in display state, false: in idle state */
    bool badline;                   /* true when the badline state is active */
    bool frame_badlines_enabled;    /* true when badlines are enabled in frame */
} _m6567_raster_unit_t;

/* address generator / memory interface state */
typedef struct {
    uint16_t c_addr_or;     /* OR-mask for c-accesses, computed from mem_ptrs */
    uint16_t g_addr_and;    /* AND-mask for g-accesses, computed from mem_ptrs */
    uint16_t g_addr_or;     /* OR-mask for g-accesses, computed from ECM bit */
    uint16_t i_addr;        /* address for i-accesses, 0x3FFF or 0x39FF (if ECM bit set) */
    uint16_t p_addr_or;     /* OR-mask for p-accesses */
    m6567_fetch_t fetch_cb; /* memory-fetch callback */
} _m6567_memory_unit_t;

/* video matrix state */
typedef struct {
    uint8_t vmli;           /* 6-bit 'vmli' video-matrix line buffer index */
    uint16_t line[64];      /* 40x 8+4 bits line buffer (64 items because vmli is a 6-bit ctr) */
} _m6567_video_matrix_t;

/* border unit state */
typedef struct {
    uint16_t left, right, top, bottom;
    bool main;          /* main border flip-flop */
    bool vert;          /* vertical border flip flop */
    uint32_t bc_rgba8;  /* border color as RGBA8, udpated when border color register is updated */
} _m6567_border_unit_t;

/* CRT state tracking */
typedef struct {
    uint16_t x, y;              /* bream pos reset on crt_retrace_h/crt_retrace_v zero */
    uint16_t vis_x0, vis_y0, vis_x1, vis_y1;  /* the visible area */
    uint16_t vis_w, vis_h;      /* width of visible area */
    uint32_t* rgba8_buffer;
} _m6567_crt_t;

/* graphics sequencer state */
typedef struct {
    uint8_t mode;               /* display mode 0..7 precomputed from ECM/BMM/MCM bits */
    bool enabled;               /* true while g_accesses are happening */
    uint8_t count;              /* counts from 0..8 */
    uint8_t shift;              /* current pixel shifter */
    uint8_t outp;               /* current output byte (bit 7) */
    uint8_t outp2;              /* current output byte at half frequency (bits 7 and 6) */
    uint16_t c_data;            /* loaded from video matrix line buffer */
    uint32_t bg_rgba8[4];       /* background colors as RGBA8 */
} _m6567_graphics_unit_t;

/* sprite sequencer state */
typedef struct {
    /* updated when setting sprite position registers */
    uint8_t h_first;            /* first horizontal visible tick */
    uint8_t h_last;             /* last horizontal visible tick */
    uint8_t h_offset;           /* x-offset within 8-pixel raster */

    uint8_t p_data;             /* the byte read by p_access memory fetch */
    
    bool dma_enabled;           /* sprite dma is enabled */
    bool disp_enabled;          /* sprite display is enabled */
    bool expand;                /* expand flip-flop */
    uint8_t mc;                 /* 6-bit mob-data-counter */
    uint8_t mc_base;            /* 6-bit mob-data-counter base */
    uint8_t delay_count;        /* 0..7 delay pixels */
    uint8_t outp2_count;        /* outp2 is updated when bit 0 is on */
    uint8_t xexp_count;         /* if x stretched, only shift every second pixel tick */
    uint32_t shift;             /* 24-bit shift register */
    uint32_t outp;              /* current shifter output (bit 31) */
    uint32_t outp2;             /* current shifter output at half frequency (bits 31 and 30) */
    uint32_t color;             /* current sprite color */
} _m6567_sprite_unit_t;

/* the m6567 state structure */
typedef struct {
    m6567_type_t type;
    bool debug_vis;             /* toggle this to switch debug visualization on/off */
    _m6567_registers_t reg;
    _m6567_raster_unit_t rs;
    _m6567_crt_t crt;
    _m6567_border_unit_t brd;
    _m6567_memory_unit_t mem;
    _m6567_video_matrix_t vm;
    _m6567_graphics_unit_t gunit;
    _m6567_sprite_unit_t sunit[8];
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

/*
    color palette (see: http://unusedino.de/ec64/technical/misc/vic656x/colors/)
*/
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
    0x8F,   /* interrupt latch */
    0x0F,   /* interrupt enabled mask */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   /* mob data pri, multicolor, x expansion, mob-mob coll, mob-data coll */
    0x0F,                       /* border color */
    0x0F, 0x0F, 0x0F, 0x0F,     /* background colors */
    0x0F, 0x0F,                 /* sprite multicolor 0,1 */
    0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,     /* sprite colors */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     /* unused 0..7 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     /* unused 8..15 */
    0x00                                                /* unused 16 */
};

/* internal implementation constants */
#define _M6567_HTOTAL               (62)    /* 63 cycles per line (PAL) */
#define _M6567_HRETRACEPOS          (3)     /* start of horizontal beam retrace */
#define _M6567_VTOTAL               (311)   /* 312 lines total (PAL) */
#define _M6567_VRETRACEPOS          (303)   /* start of vertical beam retrace */

#define _M6567_RSEL1_BORDER_TOP     (51)    /* top border when RSEL=1 (25 rows) */
#define _M6567_RSEL1_BORDER_BOTTOM  (251)   /* bottom border when RSEL=1 */
#define _M6567_RSEL0_BORDER_TOP     (55)    /* top border when RSEL=0 (24 rows) */
#define _M6567_RSEL0_BORDER_BOTTOM  (247)   /* bottom border when RSEL=0 (24 rows) */
#define _M6567_CSEL1_BORDER_LEFT    (15)    /* left border when CSEL=1 (40 columns) */
#define _M6567_CSEL1_BORDER_RIGHT   (55)    /* right border when CSEL=1 */
#define _M6567_CSEL0_BORDER_LEFT    (16)    /* left border when CSEL=0 (38 columns) */
#define _M6567_CSEL0_BORDER_RIGHT   (54)    /* right border when CSEL=0 */

/* internal helper macros to check for horizontal ticks with coordinates 
used here: http://www.zimmers.net/cbmpics/cbm/c64/vic-ii.txt
*/
#define _M6567_HTICK(t)             (vic->rs.h_count == (t))
#define _M6567_HTICK_GE(t)          (vic->rs.h_count >= (t))
#define _M6567_HTICK_RANGE(t0,t1)   ((vic->rs.h_count >= (t0)) && (vic->rs.h_count <= (t1)))
#define _M6567_RAST(r)              (vic->rs.v_count == (r))
#define _M6567_RAST_RANGE(r0,r1)    ((vic->rs.v_count >= (r0)) && (vic->rs.v_count <= (r1)))

/*--- init -------------------------------------------------------------------*/
static void _m6567_init_raster_unit(_m6567_raster_unit_t* r, m6567_desc_t* desc) {
    /* only allow the PAL version for now */
    CHIPS_ASSERT(desc->type == M6567_TYPE_6569);
    r->h_total = _M6567_HTOTAL;
    r->v_total = _M6567_VTOTAL;
    CHIPS_ASSERT(desc->rgba8_buffer_size >= (r->h_total*8*r->v_total*sizeof(uint32_t)));
}

static void _m6567_init_crt(_m6567_crt_t* crt, m6567_desc_t* desc) {
    /* vis area horizontal coords must be multiple of 8 */
    CHIPS_ASSERT((desc->vis_x & 7) == 0);
    CHIPS_ASSERT((desc->vis_w & 7) == 0);
    crt->rgba8_buffer = desc->rgba8_buffer;
    crt->vis_x0 = desc->vis_x/8;
    crt->vis_y0 = desc->vis_y;
    crt->vis_w = desc->vis_w/8;
    crt->vis_h = desc->vis_h;
    crt->vis_x1 = crt->vis_x0 + crt->vis_w;
    crt->vis_y1 = crt->vis_y0 + crt->vis_h;
}

void m6567_init(m6567_t* vic, m6567_desc_t* desc) {
    CHIPS_ASSERT(vic && desc);
    CHIPS_ASSERT((desc->type == M6567_TYPE_6567R8) || (desc->type == M6567_TYPE_6569));
    memset(vic, 0, sizeof(*vic));
    vic->type = desc->type;
    _m6567_init_raster_unit(&vic->rs, desc);
    _m6567_init_crt(&vic->crt, desc);
    vic->mem.fetch_cb = desc->fetch_cb;
}

/*--- reset ------------------------------------------------------------------*/
static void _m6567_reset_register_bank(_m6567_registers_t* r) {
    memset(r, 0, sizeof(*r));
}

static void _m6567_reset_raster_unit(_m6567_raster_unit_t* r) {
    r->h_count = r->v_count = 0;
    r->sh_count = 0;
    r->v_irqline = 0;
    r->vc = r->vc_base = 0;
    r->rc = 0;
    r->display_state = false;
    r->badline = false;
    r->frame_badlines_enabled = false;
}

static void _m6567_reset_memory_unit(_m6567_memory_unit_t* m) {
    m->c_addr_or = 0;
    m->g_addr_and = 0;
    m->g_addr_or = 0;
    m->i_addr = 0;
    m->p_addr_or = 0;
}

static void _m6567_reset_video_matrix_unit(_m6567_video_matrix_t* vm) {
    memset(vm, 0, sizeof(*vm));
}

static void _m6567_reset_graphics_unit(_m6567_graphics_unit_t* gu) {
    memset(gu, 0, sizeof(*gu));
}

static void _m6567_reset_sprite_unit(_m6567_sprite_unit_t* su) {
    memset(su, 0, sizeof(*su));
}

static void _m6567_reset_border_unit(_m6567_border_unit_t* b) {
    b->main = b->vert = false;
}

static void _m6567_reset_crt(_m6567_crt_t* c) {
    c->x = c->y = 0;
}

void m6567_reset(m6567_t* vic) {
    CHIPS_ASSERT(vic);
    _m6567_reset_register_bank(&vic->reg);
    _m6567_reset_raster_unit(&vic->rs);
    _m6567_reset_crt(&vic->crt);
    _m6567_reset_border_unit(&vic->brd);
    _m6567_reset_memory_unit(&vic->mem);
    _m6567_reset_video_matrix_unit(&vic->vm);
    _m6567_reset_graphics_unit(&vic->gunit);
    for (int i = 0; i < 8; i++) {
        _m6567_reset_sprite_unit(&(vic->sunit[i]));
    }
}

/*--- I/O requests -----------------------------------------------------------*/

/* update the raster-interrupt line from ctrl_1 and raster register updates */
static void _m6567_io_update_irq_line(_m6567_raster_unit_t* rs, uint8_t ctrl_1, uint8_t rast) {
    rs->v_irqline = ((ctrl_1 & M6567_CTRL1_RST8)<<1) | rast;
}

/* update memory unit values after update mem_ptrs or ctrl_1 registers */
static void _m6567_io_update_memory_unit(_m6567_memory_unit_t* m, uint8_t mem_ptrs, uint8_t ctrl_1) {
    /* c-access: addr=|VM13|VM12|VM11|VM10|VC9|VC8|VC7|VC6|VC5|VC4|VC3|VC2|VC1|VC0| */
    m->c_addr_or = (mem_ptrs & 0xF0)<<6;
    /* g-access: addr=|CB13|CB12|CB11|D7|D6|D5|D4|D3|D2|D1|D0|RC2|RC1|RC0| */
    m->g_addr_or = (mem_ptrs & 0x0E)<<10;
    m->g_addr_and = 0xFFFF;
    m->i_addr = 0x3FFF;
    /* p-access: addr=|VM13|VM12|VM11|VM10|1|1|1|1|1|1|1|2..0:sprite-num| */
    m->p_addr_or = ((mem_ptrs & 0xF0)<<6) | 0x3F8;

    /* "...If the ECM bit is set, the address generator always holds the
        address lines 9 and 10 low without any other changes to the
        addressing scheme"
    */
    if (ctrl_1 & M6567_CTRL1_ECM) {
        m->g_addr_and &= ~((1<<10)|(1<<9));
        m->i_addr &= ~((1<<10)|(1<<9));
    }
}

/* update the border top/bottom position when updating csel */
static void _m6567_io_update_border_rsel(_m6567_border_unit_t* b, uint8_t ctrl_1) {
    if (ctrl_1 & M6567_CTRL1_RSEL) {
        /* RSEL 1: 25 rows */
        b->top = _M6567_RSEL1_BORDER_TOP;
        b->bottom = _M6567_RSEL1_BORDER_BOTTOM;
    }
    else {
        /* RSEL 0: 24 rows */
        b->top = _M6567_RSEL0_BORDER_TOP;
        b->bottom = _M6567_RSEL0_BORDER_BOTTOM;
    }
}

/* update the border left/right position when updating csel */
static void _m6567_io_update_border_csel(_m6567_border_unit_t* b, uint8_t ctrl_2) {
    if (ctrl_2 & M6567_CTRL2_CSEL) {
        /* CSEL 1: 40 columns */
        b->left = _M6567_CSEL1_BORDER_LEFT;
        b->right = _M6567_CSEL1_BORDER_RIGHT;
    }
    else {
        /* CSEL 0: 38 columns */
        b->left = _M6567_CSEL0_BORDER_LEFT;
        b->right = _M6567_CSEL0_BORDER_RIGHT;
    }
}

/* updates the graphics sequencer display mode (0..7) from the ECM/BMM/MCM bits */
static void _m6567_io_update_gunit_mode(_m6567_graphics_unit_t* gu, uint8_t ctrl_1, uint8_t ctrl_2) {
    gu->mode = ((ctrl_1&(M6567_CTRL1_ECM|M6567_CTRL1_BMM))|(ctrl_2&M6567_CTRL2_MCM))>>4;
}

/* update sprite unit positions and sizes when updating registers */
static void _m6567_io_update_sunit(m6567_t* vic, int i, uint8_t mx, uint8_t my, uint8_t mx8, uint8_t mxe, uint8_t mye) {
    _m6567_sprite_unit_t* su = &vic->sunit[i];
    /* mxb: MSB for each xpos */
    uint16_t xpos = ((mx8 & (1<<i))<<(8-i)) | mx;
    su->h_first  = (xpos / 8) + 12;
    su->h_offset = (xpos & 7);
    const uint16_t w = ((mxe & (1<<i)) ? 5 : 2) + ((su->h_offset > 0) ? 1:0);
    su->h_last   = su->h_first + w;
    /* 1. The expansion flip flip is set as long as the bit in MxYE in register
        $d017 corresponding to the sprite is cleared.
    */
    if ((mye & (1<<i)) == 0) {
        su->expand = true;
    }
}

/* perform an I/O request on the VIC-II */
uint64_t m6567_iorq(m6567_t* vic, uint64_t pins) {
    if (pins & M6567_CS) {
        uint8_t r_addr = pins & M6567_REG_MASK;
        _m6567_registers_t* r = &vic->reg;
        if (pins & M6567_RW) {
            /* read register, with some special cases */
            const _m6567_raster_unit_t* rs = &vic->rs;
            uint8_t data;
            switch (r_addr) {
                case 0x11:
                    /* bit 7 of 0x11 is bit 8 of the current raster counter */
                    data = (r->ctrl_1 & 0x7F) | ((rs->v_count & 0x100)>>1);
                    break;
                case 0x12:
                    /* reading 0x12 returns bits 0..7 of current raster position */
                    data = (uint8_t)rs->v_count;
                    break;
                case 0x1E:
                case 0x1F:
                    /* registers 0x1E and 0x1F (mob collisions) are cleared on reading */
                    data = r->regs[r_addr];
                    r->regs[r_addr] = 0;
                    break;
                default:
                    /* unconnected bits are returned as 1 */
                    data = r->regs[r_addr] | ~_m6567_reg_mask[r_addr];
                    break;
            }
            M6567_SET_DATA(pins, data);
        }
        else {
            /* write register, with special cases */
            const uint8_t data = M6567_GET_DATA(pins) & _m6567_reg_mask[r_addr];
            bool write = true;
            switch (r_addr) {
                case 0x00:  /* m0x */
                    _m6567_io_update_sunit(vic, 0, data, r->mxy[0][1], r->mx8, r->mxe, r->mye);
                    break;
                case 0x01:  /* m0y */
                    _m6567_io_update_sunit(vic, 0, r->mxy[0][0], data, r->mx8, r->mxe, r->mye);
                    break;
                case 0x02:  /* m1x */
                    _m6567_io_update_sunit(vic, 1, data, r->mxy[1][1], r->mx8, r->mxe, r->mye);
                    break;
                case 0x03:  /* m1y */
                    _m6567_io_update_sunit(vic, 1, r->mxy[1][0], data, r->mx8, r->mxe, r->mye);
                    break;
                case 0x04:  /* m2x */
                    _m6567_io_update_sunit(vic, 2, data, r->mxy[2][1], r->mx8, r->mxe, r->mye);
                    break;
                case 0x05:  /* m2y */
                    _m6567_io_update_sunit(vic, 2, r->mxy[2][0], data, r->mx8, r->mxe, r->mye);
                    break;
                case 0x06:  /* m3x */
                    _m6567_io_update_sunit(vic, 3, data, r->mxy[3][1], r->mx8, r->mxe, r->mye);
                    break;
                case 0x07:  /* m3y */
                    _m6567_io_update_sunit(vic, 3, r->mxy[3][0], data, r->mx8, r->mxe, r->mye);
                    break;
                case 0x08:  /* m4x */
                    _m6567_io_update_sunit(vic, 4, data, r->mxy[4][1], r->mx8, r->mxe, r->mye);
                    break;
                case 0x09:  /* m4y */
                    _m6567_io_update_sunit(vic, 4, r->mxy[4][0], data, r->mx8, r->mxe, r->mye);
                    break;
                case 0x0A:  /* m5x */
                    _m6567_io_update_sunit(vic, 5, data, r->mxy[5][1], r->mx8, r->mxe, r->mye);
                    break;
                case 0x0B:  /* m5y */
                    _m6567_io_update_sunit(vic, 5, r->mxy[5][0], data, r->mx8, r->mxe, r->mye);
                    break;
                case 0x0C:  /* m6x */
                    _m6567_io_update_sunit(vic, 6, data, r->mxy[6][1], r->mx8, r->mxe, r->mye);
                    break;
                case 0x0D:  /* m6y */
                    _m6567_io_update_sunit(vic, 6, r->mxy[6][0], data, r->mx8, r->mxe, r->mye);
                    break;
                case 0x0E:  /* m7x */
                    _m6567_io_update_sunit(vic, 7, data, r->mxy[7][1], r->mx8, r->mxe, r->mye);
                    break;
                case 0x0F:  /* m7y */
                    _m6567_io_update_sunit(vic, 7, r->mxy[7][0], data, r->mx8, r->mxe, r->mye);
                    break;
                case 0x10:  /* mx8 */
                    for (int i = 0; i < 8; i++) {
                        _m6567_io_update_sunit(vic, i, r->mxy[i][0], r->mxy[i][1], data, r->mxe, r->mye);
                    }
                    break;
                case 0x11:  /* ctrl_1 */
                    /* update raster interrupt line */
                    _m6567_io_update_irq_line(&vic->rs, data, r->raster);
                    /* update border top/bottom from RSEL flag */
                    _m6567_io_update_border_rsel(&vic->brd, data);
                    /* ECM bit updates the precomputed fetch address masks */
                    _m6567_io_update_memory_unit(&vic->mem, r->mem_ptrs, data);
                    /* update the graphics mode */
                    _m6567_io_update_gunit_mode(&vic->gunit, data, r->ctrl_2);
                    break;
                case 0x12:
                    /* raster irq value lower 8 bits */
                    _m6567_io_update_irq_line(&vic->rs, r->ctrl_1, data);
                    break;
                case 0x16: /* ctrl_2 */
                    /* update border left/right from CSEL flag */
                    _m6567_io_update_border_csel(&vic->brd, data);
                    /* update the graphics mode */
                    _m6567_io_update_gunit_mode(&vic->gunit, r->ctrl_1, data);
                    break;
                case 0x17:  /* mye */
                    for (int i = 0; i < 8; i++) {
                        _m6567_io_update_sunit(vic, i, r->mxy[i][0], r->mxy[i][1], r->mx8, r->mxe, data);
                    }
                    break;
                case 0x18:
                    /* memory-ptrs register , update precomputed fetch address masks */
                    _m6567_io_update_memory_unit(&vic->mem, data, r->ctrl_1);
                    break;
                case 0x19:
                    /* interrupt latch: to clear a bit in the latch, a 1-bit
                       must be written to the latch!
                    */
                    r->int_latch = (r->int_latch & ~data) & _m6567_reg_mask[0x19];
                    write = false;
                    break;
                case 0x1D:  /* mxe */
                    for (int i = 0; i < 8; i++) {
                        _m6567_io_update_sunit(vic, i, r->mxy[i][0], r->mxy[i][1], r->mx8, data, r->mye);
                    }
                    break;
                case 0x1E: case 0x1F:
                    /* mob collision registers cannot be written */
                    write = false;
                    break;
                case 0x20:
                    /* border color */
                    vic->brd.bc_rgba8 = _m6567_colors[data & 0xF];
                    break;
                case 0x21: case 0x22: case 0x23: case 0x24:
                    /* background colors */
                    vic->gunit.bg_rgba8[r_addr-0x21] = _m6567_colors[data & 0xF];
                    break;
                case 0x27: case 0x28: case 0x29: case 0x2A: 
                case 0x2B: case 0x2C: case 0x2D: case 0x2E:
                    /* sprite colors */
                    vic->sunit[r_addr-0x27].color = _m6567_colors[data & 0xF];
                    break;
            }
            if (write) {
                r->regs[r_addr] = data;
            }
        }
    }
    return pins;
}

/*--- graphics sequencer helpers ---------------------------------------------*/

/* start the graphics sequencer, this happens at the first g_access,
   the graphics sequencer must be delayed by xscroll
*/
static inline void _m6567_gunit_reload(m6567_t* vic, uint8_t xscroll) {
    vic->gunit.count = xscroll;
    vic->gunit.shift = 0;
    vic->gunit.outp = 0;
    vic->gunit.outp2 = 0;
    vic->gunit.c_data = 0;
}

/* Tick the graphics sequencer, this will countdown a counter, when it
   hits 0 the pixel shifter will be reloaded from the last g_access data
   byte, and the video-matrix value with the current video-matrix-value
   (or 0 if the graphics sequencer is idle).
*/
static inline void _m6567_gunit_tick(m6567_t* vic, uint8_t g_data) {
    if (vic->gunit.count == 0) {
        vic->gunit.count = 7;
        vic->gunit.shift |= g_data;
        vic->gunit.c_data = vic->gunit.enabled ? vic->vm.line[vic->vm.vmli] : 0;
    }
    else {
        vic->gunit.count--;
    }
    vic->gunit.outp = vic->gunit.shift;
    if (1 == (vic->gunit.count & 1)) {
        vic->gunit.outp2 = vic->gunit.shift;
    }
    vic->gunit.shift <<= 1;
}

/* graphics sequencer decoding functions for 1 pixel */
static inline uint32_t _m6567_gunit_decode_mode0(m6567_t* vic, uint8_t g_data) {
    _m6567_gunit_tick(vic, g_data);
    if (vic->gunit.outp & 0x80) {
        /* foreground color */
        return _m6567_colors[(vic->gunit.c_data>>8)&0xF];
    }
    else {
        /* background color */
        return vic->gunit.bg_rgba8[0];
    }
}

static inline uint32_t _m6567_gunit_decode_mode1(m6567_t* vic, uint8_t g_data) {
    _m6567_gunit_tick(vic, g_data);
    /* only seven colors in multicolor mode */
    const uint32_t fg = _m6567_colors[(vic->gunit.c_data>>8) & 0x7];
    if (vic->gunit.c_data & (1<<11)) {
        /* outp2 is only updated every 2 ticks */
        uint8_t bits = ((vic->gunit.outp2)>>6) & 3;
        /* half resolution multicolor char
            need 2 bits from the pixel sequencer
                "00": Background color 0 ($d021)
                "01": Background color 1 ($d022)
                "10": Background color 2 ($d023)
                "11": Color from bits 8-10 of c-data
        */
        if (bits == 3) {
            /* special case '11' */
            return fg;
        }
        else {
            /* one of the 3 background colors */
            return vic->gunit.bg_rgba8[bits];
        }
    }
    else {
        /* standard text mode char, but with only 7 foreground colors */
        if (vic->gunit.outp & 0x80) {
            /* foreground color */
            return fg;
        }
        else {
            /* background color */
            return vic->gunit.bg_rgba8[0];
        }
    }
}

static inline uint32_t _m6567_gunit_decode_mode2(m6567_t* vic, uint8_t g_data) {
    _m6567_gunit_tick(vic, g_data);
    if (vic->gunit.outp & 0x80) {
        /* foreground pixel */
        return _m6567_colors[(vic->gunit.c_data >> 4) & 0xF];
    }
    else {
        /* background pixel */
        return _m6567_colors[vic->gunit.c_data & 0xF];
    }
}

static inline uint32_t _m6567_gunit_decode_mode3(m6567_t* vic, uint8_t g_data) {
    _m6567_gunit_tick(vic, g_data);
    /* shift 2 is only updated every 2 ticks */
    uint8_t bits = vic->gunit.outp2;
    /* half resolution multicolor char
        need 2 bits from the pixel sequencer
            "00": Background color 0 ($d021)
            "01": Color from bits 4-7 of c-data
            "10": Color from bits 0-3 of c-data
            "11": Color from bits 8-11 of c-data
    */
    switch ((bits>>6)&3) {
        case 0:     return vic->gunit.bg_rgba8[0]; break;
        case 1:     return _m6567_colors[(vic->gunit.c_data>>4) & 0xF]; break;
        case 2:     return _m6567_colors[vic->gunit.c_data & 0xF]; break;
        default:    return _m6567_colors[(vic->gunit.c_data>>8) & 0xF]; break;
    }
}

static inline uint32_t _m6567_gunit_decode_mode4(m6567_t* vic, uint8_t g_data) {
    _m6567_gunit_tick(vic, g_data);
    if (vic->gunit.outp & 0x80) {
        /* foreground color as usual bits 8..11 of c_data */
        return _m6567_colors[(vic->gunit.c_data>>8) & 0xF];
    }
    else {
        /* bg color selected by bits 6 and 7 of c_data */
        return vic->gunit.bg_rgba8[(vic->gunit.c_data>>6) & 3];
    }
}

/*--- sprite sequencer helper ------------------------------------------------*/
static inline uint32_t _m6567_sunit_decode(m6567_t* vic) {
    /* this will tick all the sprite units and return the color
        of the highest-priority sprite color for the current pixel,
        or 0 if the sprite units didn't produce a color 
    */
    uint32_t c = 0;
    for (int i = 0; i < 8; i++) {
        _m6567_sprite_unit_t* su = &vic->sunit[i];
        if (su->disp_enabled && _M6567_HTICK_RANGE(su->h_first, su->h_last)) {
            if (su->delay_count == 0) {
                if ((0 == (su->xexp_count++ & 1)) || (0 == (vic->reg.mxe & (1<<i)))) {
                    /* bit 31 of outp is the current shifter output */
                    su->outp = su->shift;
                    /* bits 31 and 30 of outp is half-frequency shifter output */
                    if (1 == (su->outp2_count++ & 1)) {
                        su->outp2 = su->shift;
                    }
                    su->shift <<= 1;
                }

                /* FIXME */
                if (su->outp & (1<<31)) {
                    c = su->color;
                }
            }
            else {
                su->delay_count--;
            }
        }
    }
    return c;
}

/* decode the next 8 pixels */
static inline void _m6567_decode_pixels(m6567_t* vic, uint8_t g_data, uint32_t* dst) {
    /*
        "...the vertical border flip flop controls the output of the graphics
        data sequencer. The sequencer only outputs data if the flip flop is
        not set..."
    */
    if (!vic->brd.vert) {
        switch (vic->gunit.mode) {
            case 0:
                /* ECM/BMM/MCM=000, standard text mode */
                for (int i = 0; i < 8; i++) {
                    uint32_t bm = _m6567_gunit_decode_mode0(vic, g_data);
                    uint32_t sm = _m6567_sunit_decode(vic);
                    dst[i] = sm ? sm : bm;
                }
                break;
            case 1:
                /* ECM/BMM/MCM=001, multicolor text mode */
                for (int i = 0; i < 8; i++) {
                    uint32_t bm = _m6567_gunit_decode_mode1(vic, g_data);
                    uint32_t sm = _m6567_sunit_decode(vic);
                    dst[i] = sm ? sm : bm;
                }
                break;
            case 2:
                /* ECM/BMM/MCM=010, standard bitmap mode */
                for (int i = 0; i < 8; i++) {
                    uint32_t bm = _m6567_gunit_decode_mode2(vic, g_data);
                    dst[i] = bm;
                }
                break;
            case 3:
                /* ECM/BMM/MCM=011, multicolor bitmap mode */
                {
                    for (int i = 0; i < 8; i++) {
                        uint32_t bm = _m6567_gunit_decode_mode3(vic, g_data);
                        dst[i] = bm;
                    }
                }
                break;
            case 4:
                /* ECM/BMM/MCM=100, ECM text mode */
                for (int i = 0; i < 8; i++) {
                    uint32_t bm = _m6567_gunit_decode_mode4(vic, g_data);
                    dst[i] = bm;
                }
                break;

            default:
                /*
                    All other modes are 'invalid modes' and output black.
                    FIXME: those invalid modes still trigger the sprite collision,
                    may and output black bitmap foreground pixels in front 
                    of sprites
                */
                for (int i = 0; i < 8; i++) {
                    dst[i] = 0xFF000000;
                }
                break;
        }
    }
    /*
        "...otherwise it displays the background color"
    */
    else {
        const uint32_t c = vic->gunit.bg_rgba8[0];
        for (int i = 0; i < 8; i++) {
            dst[i] = c;
        }
    }
    /*
        "The main border flip flop controls the border display. If it is set, the
        VIC displays the color stored in register $d020, otherwise it displays the
        color that the priority multiplexer switches through from the graphics or
        sprite data sequencer. So the border overlays the text/bitmap graphics as
        well as the sprites. It has the highest display priority."
    */
    if (vic->brd.main) {
        const uint32_t c = vic->brd.bc_rgba8;
        for (int i = 0; i < 8; i++) {
            dst[i] = c;
        }
    }
}

/*=== TICK FUNCTION ==========================================================*/
uint64_t m6567_tick(m6567_t* vic, uint64_t pins) {
    /*--- raster unit --------------------------------------------------------*/
    bool c_access = false;
    bool g_access = false;
    bool ba_pin = false;
    bool aec_pin = false;
    /*--- update the raster unit ---------------------------------------------*/
    {
        /* 
            Update the raster unit.

            (see 3.7.2 in http://www.zimmers.net/cbmpics/cbm/c64/vic-ii.txt)
            
            FIXME: "Raster line 0 is, however, an exception: In this line, IRQ
            and incrementing (resp. resetting) of RASTER are performed one cycle later
            than in the other lines.
        */
        if (vic->rs.h_count == vic->rs.h_total) {
            vic->rs.h_count = 0;
            /* new scanline */
            if (vic->rs.v_count == vic->rs.v_total) {
                vic->rs.v_count = 0;
                vic->rs.vc_base = 0;
            }
            else {
                vic->rs.v_count++;
            }
        }
        else {
            vic->rs.h_count++;
        }
        /* update CRT beam pos */
        if (vic->rs.h_count == _M6567_HRETRACEPOS) {
            vic->crt.x = 0;
            if (vic->rs.v_count == _M6567_VRETRACEPOS) {
                vic->crt.y = 0;
            }
            else {
                vic->crt.y++;
            }
        }
        else {
            vic->crt.x++;
        }

        /* check for raster interrupt */
        if (_M6567_HTICK(0) && (vic->rs.v_count == vic->rs.v_irqline)) {
            vic->reg.int_latch = M6567_INT_IRST;
        }

        /*
            Update the badline state:

            ( see 3.5 http://www.zimmers.net/cbmpics/cbm/c64/vic-ii.txt )
            
            "A Bad Line Condition is given at any arbitrary clock cycle, if at the
            negative edge of �0 at the beginning of the cycle RASTER >= $30 (48) and RASTER
            <= $f7 (247) and the lower three bits of RASTER are equal to YSCROLL and if the
            DEN bit was set during an arbitrary cycle of raster line $30."
        */
        if (_M6567_RAST_RANGE(48, 247)) {
            /* DEN bit must have been set in raster line $30 */
            if (_M6567_RAST(48) && (vic->reg.ctrl_1 & M6567_CTRL1_DEN)) {
                vic->rs.frame_badlines_enabled = true;
            }
            /* a badline is active when the low 3 bits of raster position
                are identical with YSCROLL
            */
            bool yscroll_match = ((vic->rs.v_count & 7) == (vic->reg.ctrl_1 & 7));
            vic->rs.badline = vic->rs.frame_badlines_enabled && yscroll_match;
        }
        else {
            vic->rs.frame_badlines_enabled = false;
            vic->rs.badline = false;
        }

        /*
            Update the display/idle state flag.

            ( see 3.7.1 in http://www.zimmers.net/cbmpics/cbm/c64/vic-ii.txt )

            "The transition from idle to display state occurs as soon as there is a Bad
            Line Condition (see section 3.5.). The transition from display to idle
            state occurs in cycle 58 of a line if the RC (see next section) contains
            the value 7 and there is no Bad Line Condition."
            "...In the first phase of cycle 58, the VIC checks if RC=7. If so, the video
            logic goes to idle state and VCBASE is loaded from VC (VC->VCBASE). If
            the video logic is in display state afterwards (this is always the case
            if there is a Bad Line Condition), RC is incremented.""
        */
        if (vic->rs.badline) {
            vic->rs.display_state = true;
        }
        if (_M6567_HTICK(58)) {
            if (vic->rs.rc == 7) {
                vic->rs.vc_base = vic->rs.vc;
                if (!vic->rs.badline) {
                    vic->rs.display_state = false;
                }
            }
            if (vic->rs.display_state) {
                vic->rs.rc = (vic->rs.rc + 1) & 7;
            }
        }

        /* If there is a Bad Line Condition in cycles 12-54, BA is set low and the
            c-accesses are started. Also set the AEC pin 3 cycles later.
        */
        if (_M6567_HTICK_RANGE(12,54)) {
            if (vic->rs.badline) {
                ba_pin = true;
                if (_M6567_HTICK_GE(15)) {
                    aec_pin = true;
                    c_access = true;
                }
            }
            /* In the first phase of cycle 14 of each line, VC is loaded from VCBASE
                (VCBASE->VC) and VMLI is cleared. If there is a Bad Line Condition in
                this phase, RC is also reset to zero.
            */
            if (_M6567_HTICK(14)) {
                vic->rs.vc = vic->rs.vc_base;
                vic->vm.vmli = 0;
                if (vic->rs.badline) {
                    vic->rs.rc = 0;
                }
            }
        }
        /* g-accesses start at cycle 15 of each line in display state */
        g_access = vic->rs.display_state && _M6567_HTICK_RANGE(15,54);
        vic->gunit.enabled = g_access;
        if (_M6567_HTICK(15)) {
            /* reset the graphics sequencer, potentially delayed by xscroll value */
            _m6567_gunit_reload(vic, vic->reg.ctrl_2 & M6567_CTRL2_XSCROLL);
        }
    }

    /*--- sprite unit preparations -------------------------------------------*/
    /* (see 3.8. in http://www.zimmers.net/cbmpics/cbm/c64/vic-ii.txt) */
    /* run separate sprite raster counters which wrap around at h_count=55 */
    if (vic->rs.h_count == 55) {
        vic->rs.sh_count = 0;
    }
    else {
        vic->rs.sh_count++;
    }

    /* p-accesses happen in each scanline */
    int p_index = -1;
    for (int i = 0; i < 8; i++) {
        if (vic->rs.sh_count == 3+i*2) {
            p_index = i;
            break;
        }
    }

    /* 1. The expansion flip flip is set as long as the bit in MxYE in register
        $d017 corresponding to the sprite is cleared.
        (FIXME: this is currently only done when updating the MxYE register)
    */

    /*
       2. If the MxYE bit is set in the first phase of cycle 55, the expansion
        flip flop is inverted.
       3. In the first phases of cycle 55 and 56, the VIC checks for every sprite
        if the corresponding MxE bit in register $d015 is set and the Y
        coordinate of the sprite (odd registers $d001-$d00f) match the lower 8
        bits of RASTER. If this is the case and the DMA for the sprite is still
        off, the DMA is switched on, MCBASE is cleared, and if the MxYE bit is
        set the expansion flip flip is reset.

        NOTE: sprite display_enabled flag is turned off here if dma flag is off,
        this is different from the recipe in vic-ii.txt (switching off display
        in tick 15/16 as described in the recipe turns off rendering the
        last line of a sprite)
    */
    if (_M6567_HTICK(55)) {
        const uint8_t me = vic->reg.me;
        const uint8_t mye = vic->reg.mye;
        for (int i = 0; i < 8; i++) {
            _m6567_sprite_unit_t* su = &vic->sunit[i];
            const uint8_t mask = (1<<i);
            if (mye & mask) {
                su->expand = !su->expand;
            }
            if ((me & mask) && ((vic->rs.v_count & 0xFF) == vic->reg.mxy[i][1])) {
                if (!su->dma_enabled) {
                    su->dma_enabled = true;
                    su->mc_base = 0;
                    if (mye & mask) {
                        su->expand = false;
                    }
                }
            }
            if (!su->dma_enabled) {
                su->disp_enabled = false;
            }
        }
    }

    /* 4. In the first phase of cycle 58, the MC of every sprite is loaded from
        its belonging MCBASE (MCBASE->MC) and it is checked if the DMA for the
        sprite is turned on and the Y coordinate of the sprite matches the lower
        8 bits of RASTER. If this is the case, the display of the sprite is
        turned on.
    */
    if (_M6567_HTICK(58)) {
        for (int i = 0; i < 8; i++) {
            _m6567_sprite_unit_t* su = &vic->sunit[i];
            su->mc = su->mc_base;
            if (su->dma_enabled && ((vic->rs.v_count & 0xFF) == vic->reg.mxy[i][1])) {
                su->disp_enabled = true;
            }
        }
    }

    /* 7. In the first phase of cycle 15, it is checked if the expansion flip flop
        is set. If so, MCBASE is incremented by 2.
       8. In the first phase of cycle 16, it is checked if the expansion flip flop
        is set. If so, MCBASE is incremented by 1. After that, the VIC checks if
        MCBASE is equal to 63 and turns of the DMA and the display of the sprite
        if it is.

        NOTE: 
            - I have merged actions of cycle 15 and 16 into cycle 15
            - I have moved switching off the display enable flag at the end
              of line into tick 55
    */
    if (_M6567_HTICK(15)) {
        for (int i = 0; i < 8; i++) {
            _m6567_sprite_unit_t* su = &vic->sunit[i];
            if (su->expand) {
                su->mc_base = (su->mc_base + 3) & 0x3F;
            }
            if (su->mc_base == 63) {
                su->dma_enabled = false;
            }
        }
    }

    /* on the first visible-sprite tick each line, pre-shift by the
       sprites position inside the tick's 8-pixel grid (same idea as
       the xscroll delay for vertical scrolling)
    */
    for (int i = 0; i < 8; i++) {
        _m6567_sprite_unit_t* su = &vic->sunit[i];
        if (_M6567_HTICK(su->h_first) && su->disp_enabled) {
            su->delay_count = su->h_offset;
            su->outp2_count = 0;
            su->xexp_count = 0;
        }
    }

    /* s-access, ba/aec, for dma_enabled sprites */
    int s_index = -1;
    if (vic->rs.sh_count < (2*8 + 3)) {
        uint16_t sh = 3;
        for (int i = 0; i < 8; i++, sh+=2) {
            _m6567_sprite_unit_t* su = &vic->sunit[i];
            if (su->dma_enabled) {
                if ((vic->rs.sh_count >= (sh-3)) && (vic->rs.sh_count <= (sh+1))) {
                    ba_pin = true;
                    if (vic->rs.sh_count >= sh) {
                        aec_pin = true;
                        s_index = i;
                    }
                    break;
                }
            }
        }
    }

    /*--- perform memory fetches ---------------------------------------------*/
    /* (see 3.6.3. in http://www.zimmers.net/cbmpics/cbm/c64/vic-ii.txt) */
    bool i_access = true;
    if (c_access) {
        /* addr=|VM13|VM12|VM11|VM10|VC9|VC8|VC7|VC6|VC5|VC4|VC3|VC2|VC1|VC0| */
        uint16_t addr = vic->rs.vc | vic->mem.c_addr_or;
        vic->vm.line[vic->vm.vmli] = vic->mem.fetch_cb(addr) & 0x0FFF;
        i_access = false;
    }
    else if (p_index >= 0) {
        /* a sprite p-access */
        uint16_t addr = vic->mem.p_addr_or + p_index;
        vic->sunit[p_index].p_data = (uint8_t) vic->mem.fetch_cb(addr);
        i_access = false;
    }

    /* in the first half-cycle, either a g_access, p_access or i_access happens */
    uint8_t g_data;
    if (g_access) {
        uint16_t addr;
        if (vic->reg.ctrl_1 & M6567_CTRL1_BMM) {
            /* bitmap mode: addr=|CB13|VC9|VC8|VC7|VC6|VC5|VC4|VC3|VC2|VC1|VC0|RC2|RC1|RC0| */
            addr = vic->rs.vc<<3 | vic->rs.rc;
            addr = (addr | (vic->mem.g_addr_or & (1<<13))) & vic->mem.g_addr_and;
        }
        else {
            /* text mode: addr=|CB13|CB12|CB11|D7|D6|D5|D4|D3|D2|D1|D0|RC2|RC1|RC0| */
            addr = ((vic->vm.line[vic->vm.vmli]&0xFF)<<3) | vic->rs.rc;
            addr = (addr | vic->mem.g_addr_or) & vic->mem.g_addr_and;
        }
        g_data = (uint8_t) vic->mem.fetch_cb(addr);
        i_access = false;
    }
    else if (s_index >= 0) {
        /* sprite s-access: |MP7|MP6|MP5|MP4|MP3|MP2|MP1|MP0|MC5|MC4|MC3|MC2|MC1|MC0| */
        _m6567_sprite_unit_t* su = &vic->sunit[s_index];
        uint16_t addr = (su->p_data<<6) | su->mc;
        uint8_t s_data = vic->mem.fetch_cb(addr);
        su->shift = (su->shift<<8) | (s_data<<8);
        su->mc = (su->mc + 1) & 0x3F;
        /* in the tick *after* the p-access, need to do 2 s-accesses (one each half-tick) */
        if (p_index == -1) {
            uint16_t addr = (su->p_data<<6) | su->mc;
            uint8_t s_data = vic->mem.fetch_cb(addr);
            su->shift = (su->shift<<8) | (s_data<<8);
            su->mc = (su->mc + 1) & 0x3F;
        }
    }

    /* if no other accesses happened, do an i-access */
    if (i_access) {
        g_data = (uint8_t) vic->mem.fetch_cb(vic->mem.i_addr);
    }

    /*--- update the border flip-flops ---------------------------------------*/
    /* (see 3.9 in http://www.zimmers.net/cbmpics/cbm/c64/vic-ii.txt) */
    {
        /* 1. If the X coordinate reaches the right comparison value, the main border
              flip flop is set.
        */
        if (vic->rs.h_count == vic->brd.right) {
            vic->brd.main = true;
        }
        if (_M6567_HTICK(0)) {
            /* 2. If the Y coordinate reaches the bottom comparison value in cycle 63, the
                  vertical border flip flop is set.
            */
            if (vic->rs.v_count == vic->brd.bottom) {
                vic->brd.vert = true;
            }
            /* 3. If the Y coordinate reaches the top comparison value in cycle 63 and the
                  DEN bit in register $d011 is set, the vertical border flip flop is
                  reset.
            */
            if ((vic->rs.v_count == vic->brd.top) && (vic->reg.ctrl_1 & M6567_CTRL1_DEN)) {
                vic->brd.vert = false;
            }
        }
        if (vic->rs.h_count == vic->brd.left) {
            /* 4. If the X coordinate reaches the left comparison value and the Y
                  coordinate reaches the bottom one, the vertical border flip flop is set.
            */
            if (vic->rs.v_count == vic->brd.bottom) {
                vic->brd.vert = true;
            }
            /* 5. If the X coordinate reaches the left comparison value and the Y
                  coordinate reaches the top one and the DEN bit in register $d011 is set,
                  the vertical border flip flop is reset.
            */
            if ((vic->rs.v_count == vic->brd.top) && (vic->reg.ctrl_1 & M6567_CTRL1_DEN)) {
                vic->brd.vert = false;
            }
            /* 6. If the X coordinate reaches the left comparison value and the vertical
                  border flip flop is not set, the main flip flop is reset.
            */
            if (!vic->brd.vert) {
                vic->brd.main = false;
            }
        }
    }

    /*-- main interrupt bit --*/
    if (0 != ((vic->reg.int_latch & vic->reg.int_mask) & 0x0F)) {
        vic->reg.int_latch |= M6567_INT_IRQ;
    }

    /*--- decode pixels into framebuffer -------------------------------------*/
    {
        if (vic->debug_vis) {
            /*=== DEBUG VISUALIZATION ===*/
            const int x = vic->rs.h_count;
            const int y = vic->rs.v_count;
            const int w = vic->rs.h_total + 1;
            uint32_t* dst = vic->crt.rgba8_buffer + (y * w + x) * 8;;
            _m6567_decode_pixels(vic, g_data, dst);
            dst[0] = (dst[0] & 0xFF000000) | 0x00222222;
            uint32_t mask = 0x00000000;
            if (vic->rs.badline) {
                mask = 0x0000007F;
            }
            if (ba_pin) {
                mask = 0x000000FF;
            }
            /* sprites */
            for (int si = 0; si < 8; si++) {
                const _m6567_sprite_unit_t* su = &vic->sunit[si];
                if (su->disp_enabled) {
                    if (_M6567_HTICK_RANGE(su->h_first, su->h_last)) {
                        mask |= 0xFFFF00FF;
                    }
                }
            }
            /* main interrupt bit */
            if (vic->reg.int_latch & (1<<7)) {
                mask = 0x0000FF00;
            }
            if (mask != 0) {
                for (int i = 0; i < 8; i++) {
                    dst[i] = (dst[i] & 0xFF000000) | mask;
                }
            }
        }
        else if ((vic->crt.x >= vic->crt.vis_x0) && (vic->crt.x < vic->crt.vis_x1) &&
                (vic->crt.y >= vic->crt.vis_y0) && (vic->crt.y < vic->crt.vis_y1))
        {
            const int x = vic->crt.x - vic->crt.vis_x0;
            const int y = vic->crt.y - vic->crt.vis_y0;
            const int w = vic->crt.vis_w;
            uint32_t* dst = vic->crt.rgba8_buffer + (y * w + x) * 8;;
            _m6567_decode_pixels(vic, g_data, dst);
        }
    }

    /*--- bump the VC and vmli counters --------------------------------------*/
    if (g_access) {
        vic->rs.vc = (vic->rs.vc + 1) & 0x3FF;        /* VS is a 10-bit counter */
        vic->vm.vmli = (vic->vm.vmli + 1) & 0x3F;     /* VMLI is a 6-bit counter */
    }

    /*--- set CPU pins -------------------------------------------------------*/
    if (ba_pin) {
        pins |= M6567_BA;
    }
    if (aec_pin) {
        pins |= M6567_AEC;
    }
    if (vic->reg.int_latch & (1<<7)) {
        pins |= M6567_IRQ;
    }
    return pins;
}

void m6567_display_size(m6567_t* vic, int* out_width, int* out_height) {
    CHIPS_ASSERT(vic && out_width && out_height);
    if (vic->debug_vis) {
        *out_width = (vic->rs.h_total+1)*8;
        *out_height = vic->rs.v_total+1;
    }
    else {
        *out_width = vic->crt.vis_w*8;
        *out_height = vic->crt.vis_h;
    }
}

uint32_t m6567_color(int i) {
    CHIPS_ASSERT((i >= 0) && (i < 16));
    return _m6567_colors[i];
}
#endif /* CHIPS_IMPL */

#ifdef __cplusplus
} /* extern "C" */
#endif
