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

/* register bank */
typedef struct {
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
} _m6567_registers_t;

/* raster unit state */
typedef struct {
    uint16_t h_count, h_total, h_retracepos;
    uint16_t v_count, v_total, v_retracepos;
    uint16_t vc;            /* 10-bit video counter */
    uint16_t vc_base;       /* 10-bit video counter base */
    uint8_t rc;             /* 3-bit raster counter */
    bool display_state;             /* true: in display state, false: in idle state */
    bool badline;                   /* true when the badline state is active */
    bool frame_badlines_enabled;    /* true when badlines are enabled in frame */
    bool irq;               /* true when IRQ requested */
} _m6567_raster_unit_t;

/* address generator / memory interface state */
typedef struct {
    uint16_t c_addr_or;     /* OR-mask for c-accesses, computed from mem_ptrs */
    uint16_t g_addr_and;    /* AND-mask for g-accesses, computed from mem_ptrs */
    uint16_t g_addr_or;     /* OR-mask for g-accesses, computed from ECM bit */
    uint16_t i_addr;        /* address for i-accesses, 0x3FFF or 0x39FF (if ECM bit set) */
    uint8_t g_data;         /* byte read by last g-access */
    bool g_access;          /* true when g_access needs to happen */
    bool c_access;          /* true when c_access needs to happen */
    bool ba_pin;            /* state of the BA pin */
    bool aec_pin;           /* state of the AEC pin */
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
    uint16_t vis_x, vis_y;      /* current position in visible area */
    bool vis_enabled;           /* beam is currently in visible area */
    uint16_t vis_x0, vis_y0, vis_x1, vis_y1;  /* the visible area */
    uint16_t vis_w, vis_h;      /* width of visible area */
    uint32_t* rgba8_buffer;
} _m6567_crt_t;

/* graphics sequencer state */
typedef struct {
    uint8_t mode;               /* display mode 0..7 precomputed from ECM/BMM/MCM bits */
    bool enabled;               /* true while g_accesses are happening */
    uint8_t count;              /* counts from 0..8 */
    uint16_t shift;             /* pixel mask shifter, bits > 8 are the 'shift-out' result */
    uint16_t shift2;            /* copied from pixel mask shifter every even tick */
    uint16_t vm;                /* loaded from video matrix line buffer */
    uint32_t bg_rgba8[4];       /* background colors as RGBA8 */
} _m6567_graphics_sequencer_t;

/* the m6567 state structure */
typedef struct {
    m6567_type_t type;
    _m6567_registers_t reg;
    _m6567_raster_unit_t rs;
    _m6567_memory_unit_t mem;
    _m6567_video_matrix_t vm;
    _m6567_graphics_sequencer_t gseq;
    _m6567_border_unit_t brd;
    _m6567_crt_t crt;
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

/*--- init -------------------------------------------------------------------*/
static void _m6567_init_raster_unit(_m6567_raster_unit_t* r, m6567_desc_t* desc) {
    if (desc->type == M6567_TYPE_6569) {
        /* PAL-B */
        r->h_total = 63;          /* 63 cycles per line */
        r->h_retracepos = 3;
        r->v_total = 312;         /* 312 lines total (PAL) */
        r->v_retracepos = 303;
    }
    else {
        /* NTSC */
        r->h_total = 65;          /* 65 cycles per line */
        r->h_retracepos = 3;
        r->v_total = 263;         /* 263 lines total (NTSC) */
        r->v_retracepos = 16;
    }
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
    CHIPS_ASSERT(desc->rgba8_buffer_size >= (512*312));
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
    r->vc = r->vc_base = 0;
    r->rc = 0;
    r->display_state = false;
    r->badline = false;
    r->frame_badlines_enabled = false;
    r->irq = false;
}

static void _m6567_reset_memory_unit(_m6567_memory_unit_t* m) {
    m->c_addr_or = 0;
    m->g_addr_and = 0;
    m->g_addr_or = 0;
    m->i_addr = 0;
    m->g_access = false;
    m->c_access = false;
    m->ba_pin = false;
    m->aec_pin = false;
    m->g_data = 0;
}

static void _m6567_reset_video_matrix_unit(_m6567_video_matrix_t* vm) {
    memset(vm, 0, sizeof(*vm));
}

static void _m6567_reset_graphics_sequencer(_m6567_graphics_sequencer_t* gs) {
    memset(gs, 0, sizeof(*gs));
}

static void _m6567_reset_border_unit(_m6567_border_unit_t* b) {
    b->main = b->vert = false;
}

static void _m6567_reset_crt(_m6567_crt_t* c) {
    c->x = c->y = 0;
    c->vis_x = c->vis_y = 0;
    c->vis_enabled = false;
}

void m6567_reset(m6567_t* vic) {
    CHIPS_ASSERT(vic);
    _m6567_reset_register_bank(&vic->reg);
    _m6567_reset_raster_unit(&vic->rs);
    _m6567_reset_memory_unit(&vic->mem);
    _m6567_reset_video_matrix_unit(&vic->vm);
    _m6567_reset_graphics_sequencer(&vic->gseq);
    _m6567_reset_border_unit(&vic->brd);
    _m6567_reset_crt(&vic->crt);
}

/*--- I/O requests -----------------------------------------------------------*/

/* update memory unit values after update mem_ptrs or ctrl_1 registers */
static void _m6567_io_update_memory_unit(_m6567_memory_unit_t* m, uint8_t mem_ptrs, uint8_t ctrl_1) {
    /* c-access: addr=|VM13|VM12|VM11|VM10|VC9|VC8|VC7|VC6|VC5|VC4|VC3|VC2|VC1|VC0| */
    m->c_addr_or = (mem_ptrs & 0xF0)<<6;
    /* g-access: addr=|CB13|CB12|CB11|D7|D6|D5|D4|D3|D2|D1|D0|RC2|RC1|RC0| */
    m->g_addr_or = (mem_ptrs & 0x0E)<<10;
    m->g_addr_and = 0xFFFF;
    m->i_addr = 0x3FFF;
    /* "...If the ECM bit is set, the address generator always holds the
        address lines 9 and 10 low without any other changes to the
        addressing scheme"
    */
    if (ctrl_1 & (1<<6)) {
        m->g_addr_and &= ~((1<<10)|(1<<9));
        m->i_addr &= ~((1<<10)|(1<<9));
    }
}

/* update the border top/bottom position when updating csel */
static void _m6567_io_update_border_rsel(_m6567_border_unit_t* b, uint8_t ctrl_1) {
    if (ctrl_1 & (1<<3)) {
        /* RSEL 1: 25 rows */
        b->top = 51;
        b->bottom = 251;
    }
    else {
        /* RSEL 0: 24 rows */
        b->top = 55;
        b->bottom = 247;
    }
}

/* update the border left/right position when updating csel */
static void _m6567_io_update_border_csel(_m6567_border_unit_t* b, uint8_t ctrl_2) {
    if (ctrl_2 & (1<<3)) {
        /* CSEL 1: 40 columns */
        b->left = 15;
        b->right = 55;
    }
    else {
        /* CSEL 0: 38 columns */
        b->left = 16;
        b->right = 54;
    }
}

/* updates the graphics sequencer display mode (0..7) from the ECM/BMM/MCM bits */
static void _m6567_io_update_gseq_mode(_m6567_graphics_sequencer_t* gseq, uint8_t ctrl_1, uint8_t ctrl_2) {
    gseq->mode = ((ctrl_1&((1<<6)|(1<<5)))|(ctrl_2&(1<<4)))>>4;
}

/* perform an I/O request on the VIC-II */
uint64_t m6567_iorq(m6567_t* vic, uint64_t pins) {
    if (pins & M6567_CS) {
        uint8_t reg_addr = pins & M6567_REG_MASK;
        if (pins & M6567_RW) {
            /* read register, with some special cases */
            _m6567_registers_t* rb = &vic->reg;
            const _m6567_raster_unit_t* rs = &vic->rs;
            uint8_t data;
            switch (reg_addr) {
                case 0x11:
                    /* bit 7 of 0x11 is bit 8 of the current raster counter */
                    data = (rb->ctrl_1 & 0x7F) | ((rs->v_count & 0x100)>>1);
                    break;
                case 0x12:
                    /* reading 0x12 returns bits 0..7 of current raster position */
                    data = (uint8_t)rs->v_count;
                    break;
                case 0x1E:
                case 0x1F:
                    /* registers 0x1E and 0x1F (mob collisions) are cleared on reading */
                    data = rb->regs[reg_addr];
                    rb->regs[reg_addr] = 0;
                    break;
                default:
                    /* unconnected bits are returned as 1 */
                    data = rb->regs[reg_addr] | ~_m6567_reg_mask[reg_addr];
                    break;
            }
            M6567_SET_DATA(pins, data);
        }
        else {
            /* write register, with special cases */
            _m6567_registers_t* rb = &vic->reg;
            const uint8_t data = M6567_GET_DATA(pins) & _m6567_reg_mask[reg_addr];
            bool write = true;
            switch (reg_addr) {
                case 0x11:
                    /* update border top/bottom from RSEL flag */
                    _m6567_io_update_border_rsel(&vic->brd, data);
                    /* ECM bit updates the precomputed fetch address masks */
                    _m6567_io_update_memory_unit(&vic->mem, rb->mem_ptrs, data);
                    /* update the graphics mode */
                    _m6567_io_update_gseq_mode(&vic->gseq, data, rb->ctrl_2);
                    break;
                case 0x16:
                    /* update border left/right from CSEL flag */
                    _m6567_io_update_border_csel(&vic->brd, data);
                    /* update the graphics mode */
                    _m6567_io_update_gseq_mode(&vic->gseq, rb->ctrl_1, data);
                    break;
                case 0x18:
                    /* memory-ptrs register , update precomputed fetch address masks */
                    _m6567_io_update_memory_unit(&vic->mem, data, rb->ctrl_1);
                    break;
                case 0x1E:
                case 0x1F:
                    /* mob collision registers cannot be written */
                    write = false;
                    break;
                case 0x20:
                    /* border color */
                    vic->brd.bc_rgba8 = _m6567_colors[data & 0xF];
                    break;
                case 0x21:
                case 0x22:
                case 0x23:
                case 0x24:
                    /* background colors */
                    vic->gseq.bg_rgba8[reg_addr-0x21] = _m6567_colors[data & 0xF];
                    break;
            }
            if (write) {
                rb->regs[reg_addr] = data;
            }
        }
    }
    return pins;
}

/*--- graphics sequencer helpers ---------------------------------------------*/

/* start the graphics sequencer, this happens at the first g_access,
   the graphics sequencer must be delayed by xscroll
*/
static inline void _m6567_gseq_start(_m6567_graphics_sequencer_t* gseq, uint8_t xscroll) {
    gseq->enabled = true;
    gseq->vm = 0;
    gseq->shift = 0;
    gseq->shift2 = 0;
    gseq->count = xscroll;
}

/* stop the graphics sequencer, this will set the video-matrix-value to 0 */
static inline void _m6567_gseq_stop(_m6567_graphics_sequencer_t* gseq) {
    gseq->enabled = false;
    gseq->vm = 0;
}

/* Tick the graphics sequencer, this will countdown a counter, when it
   hits 0 the pixel shifter will be reloaded from the last g_access data
   byte, and the video-matrix value with the current video-matrix-value
   (or 0 if the graphics sequencer is idle).

   The function returns the complete 16-bit shift register, bit 8 is the
   last 'out-shifted' bit.
*/
static inline uint16_t _m6567_gseq_tick(m6567_t* vic) {
    if (vic->gseq.count-- == 0) {
        vic->gseq.count = 7;
        vic->gseq.shift |= vic->mem.g_data;
        vic->gseq.vm = vic->gseq.enabled ? vic->vm.line[vic->vm.vmli] : 0;
    }
    vic->gseq.shift <<= 1;
    /* store the last 'bit pair' shift value for half-resolution modes */
    if (0 == (vic->gseq.count & 1)) {
        vic->gseq.shift2 = vic->gseq.shift;
    }
    return vic->gseq.shift;
}

/*=== TICK FUNCTION ==========================================================*/
uint64_t m6567_tick(m6567_t* vic, uint64_t pins) {
    /*--- raster unit --------------------------------------------------------*/
    const bool den = vic->reg.ctrl_1 & (1<<4);
    const uint8_t yscroll = vic->reg.ctrl_1 & 7;
    const uint8_t xscroll = vic->reg.ctrl_2 & 7;
    {
        /* (see 3.7.2 in http://www.zimmers.net/cbmpics/cbm/c64/vic-ii.txt) */
        _m6567_raster_unit_t* rs = &vic->rs;
        _m6567_crt_t* crt = &vic->crt;
        _m6567_video_matrix_t* vm = &vic->vm;
        _m6567_graphics_sequencer_t* gseq = &vic->gseq;
        _m6567_memory_unit_t* mem = &vic->mem;

        /* update horizontal and vertical counters */
        if (rs->h_count == rs->h_total) {
            rs->h_count = 0;
            /* new scanline */
            if (rs->v_count == rs->v_total) {
                rs->v_count = 0;
                rs->vc_base = 0;
            }
            else {
                rs->v_count++;
            }
        }
        else {
            rs->h_count++;
            rs->irq = false;
        }

        /* update the crt beam position */
        if (rs->h_count == rs->h_retracepos) {
            crt->x = 0;
            if (rs->v_count == rs->v_retracepos) {
                crt->y = 0;
            }
            else {
                crt->y++;
            }
        }
        else {
            crt->x++;
        }

        /* update visible-area coordinates and enabled-state */
        if ((crt->x >= crt->vis_x0) && (crt->x < crt->vis_x1) &&
            (crt->y >= crt->vis_y0) && (crt->y < crt->vis_y1))
        {
            crt->vis_enabled = true;
            crt->vis_x = crt->x - crt->vis_x0;
            crt->vis_y = crt->y - crt->vis_y0;
        }
        else {
            crt->vis_enabled = false;
        }

        /* badline state ((see 3.5 http://www.zimmers.net/cbmpics/cbm/c64/vic-ii.txt) */
        if ((rs->v_count >= 0x30) && (rs->v_count <= 0xF7)) {
            /* DEN bit must have been set in raster line $30 */
            if ((rs->v_count == 0x30) && den) {
                rs->frame_badlines_enabled = true;
            }
            /* a badline is active when the low 3 bits of raster position
               are identical with YSCROLL
            */
            rs->badline = (rs->frame_badlines_enabled && ((rs->v_count&7)==yscroll));
        }
        else {
            rs->frame_badlines_enabled = false;
            rs->badline = false;
        }

        if ((rs->h_count >= 12) && (rs->h_count <= 54)) {
            /* If there is a Bad Line Condition in cycles 12-54, BA is set low and the
               c-accesses are started. Also set the AEC pin 3 cycles later.
            */
            if (rs->badline) {
                mem->ba_pin = true;
                if (rs->h_count == 15) {
                    mem->aec_pin = mem->ba_pin;
                    mem->c_access = true;
                }
            }
            /* In the first phase of cycle 14 of each line, VC is loaded from VCBASE
               (VCBASE->VC) and VMLI is cleared. If there is a Bad Line Condition in
               this phase, RC is also reset to zero.
            */
            if (rs->h_count == 14) {
                rs->vc = rs->vc_base;
                vm->vmli = 0;
                if (rs->badline) {
                    rs->rc = 0;
                }
            }
        }
        /* g-accesses start at cycle 15 of each line in display state */
        if (rs->display_state) {
            if (rs->h_count == 15) {
                mem->g_access = true;
                /* reset the graphics sequencer, potentially delayed by xscroll value */
                _m6567_gseq_start(gseq, xscroll);
            }
            else if (rs->h_count == 55) {
                _m6567_gseq_stop(gseq);
                mem->g_access = false;
                mem->c_access = false;
                mem->ba_pin = false;
                mem->aec_pin = false;
            }
        }

        /* display/idle state (see 3.7.1 in http://www.zimmers.net/cbmpics/cbm/c64/vic-ii.txt )
           ...In the first phase of cycle 58, the VIC checks if RC=7. If so, the video
            logic goes to idle state and VCBASE is loaded from VC (VC->VCBASE). If
            the video logic is in display state afterwards (this is always the case
            if there is a Bad Line Condition), RC is incremented.
        */
        if (rs->badline) {
            rs->display_state = true;
        }
        if (rs->h_count == 58) {
            if ((rs->rc == 7) && !rs->badline) {
                rs->display_state = false;
                rs->vc_base = rs->vc;
            }
            if (rs->display_state) {
                rs->rc = (rs->rc + 1) & 7;
            }
        }
    }

    /*--- perform memory fetches ---------------------------------------------*/
    /* (see 3.6.3. in http://www.zimmers.net/cbmpics/cbm/c64/vic-ii.txt) */
    {
        const bool bmm = vic->reg.ctrl_1 & (1<<5);
        const _m6567_raster_unit_t* rs = &vic->rs;
        _m6567_memory_unit_t* mem = &vic->mem;
        _m6567_video_matrix_t* vm = &vic->vm;
        uint16_t addr;
        if (mem->c_access) {
            /* addr=|VM13|VM12|VM11|VM10|VC9|VC8|VC7|VC6|VC5|VC4|VC3|VC2|VC1|VC0| */
            addr = rs->vc | mem->c_addr_or;
            vm->line[vm->vmli] = mem->fetch_cb(addr);
        }
        if (mem->g_access) {
            if (bmm) {
                /* bitmap mode: addr=|CB13|VC9|VC8|VC7|VC6|VC5|VC4|VC3|VC2|VC1|VC0|RC2|RC1|RC0| */
                addr = rs->vc<<3 | rs->rc;
                addr = (addr | (mem->g_addr_or & (1<<13))) & mem->g_addr_and;
            }
            else {
                /* text mode: addr=|CB13|CB12|CB11|D7|D6|D5|D4|D3|D2|D1|D0|RC2|RC1|RC0| */
                addr = ((vm->line[vm->vmli]&0xFF)<<3) | rs->rc;
                addr = (addr | mem->g_addr_or) & mem->g_addr_and;
            }
            mem->g_data = mem->fetch_cb(addr);
        }
        else {
            /* an idle access */
            mem->g_data = mem->fetch_cb(mem->i_addr);
        }
    }

    /*--- update the border flip-flops ---------------------------------------*/
    /* (see 3.9 in http://www.zimmers.net/cbmpics/cbm/c64/vic-ii.txt) */
    {
        const _m6567_raster_unit_t* rs = &vic->rs;
        _m6567_border_unit_t* brd = &vic->brd;

        /* 1. If the X coordinate reaches the right comparison value, the main border
              flip flop is set.
        */
        if (rs->h_count == brd->right) {
            brd->main = true;
        }
        if (rs->h_count == 0) {
            /* 2. If the Y coordinate reaches the bottom comparison value in cycle 63, the
                  vertical border flip flop is set.
            */
            if (rs->v_count == brd->bottom) {
                brd->vert = true;
            }
            /* 3. If the Y coordinate reaches the top comparison value in cycle 63 and the
                  DEN bit in register $d011 is set, the vertical border flip flop is
                  reset.
            */
            if ((rs->v_count == brd->top) && den) {
                brd->vert = false;
            }
        }
        if (rs->h_count == brd->left) {
            /* 4. If the X coordinate reaches the left comparison value and the Y
                  coordinate reaches the bottom one, the vertical border flip flop is set.
            */
            if (rs->v_count == brd->bottom) {
                brd->vert = true;
            }
            /* 5. If the X coordinate reaches the left comparison value and the Y
                  coordinate reaches the top one and the DEN bit in register $d011 is set,
                  the vertical border flip flop is reset.
            */
            if ((rs->v_count == brd->top) && den) {
                brd->vert = false;
            }
            /* 6. If the X coordinate reaches the left comparison value and the vertical
                  border flip flop is not set, the main flip flop is reset.
            */
            if (!brd->vert) {
                brd->main = false;
            }
        }
    }

    /*--- decode pixels into framebuffer -------------------------------------*/
    {
        const _m6567_crt_t* crt = &vic->crt;
        if (crt->vis_enabled) {
            const int dst_x = crt->vis_x * 8;
            const int dst_y = crt->vis_y;
            uint32_t* dst = crt->rgba8_buffer + dst_y*crt->vis_w*8 + dst_x;
            /*
                "...the vertical border flip flop controls the output of the graphics
                data sequencer. The sequencer only outputs data if the flip flop is
                not set..."
            */
            const _m6567_border_unit_t* brd = &vic->brd;
            _m6567_graphics_sequencer_t* gseq = &vic->gseq;
            if (!brd->vert) {
                switch (gseq->mode) {
                    case 0:
                        /* ECM/BMM/MCM=000, standard text mode */
                        {
                            const uint32_t bg = gseq->bg_rgba8[0];
                            for (int i = 0; i < 8; i++) {
                                dst[i] = _m6567_gseq_tick(vic) & 0x100 ? _m6567_colors[(gseq->vm>>8)&0xF] : bg;
                            }
                        }
                        break;
                    case 1:
                        /* ECM/BMM/MCM=001, multicolor text mode */
                        {
                            const uint32_t bg = gseq->bg_rgba8[0];
                            for (int i = 0; i < 8; i++) {
                                _m6567_gseq_tick(vic);
                                /* only seven colors in multicolor mode */
                                const uint32_t fg = _m6567_colors[(gseq->vm>>8) & 0x7];
                                if (gseq->vm & (1<<11)) {
                                    /* shift 2 is only updated every 2 ticks */
                                    uint16_t bits = gseq->shift2;
                                    /* half resolution multicolor char
                                       need 2 bits from the pixel sequencer
                                            "00": Background color 0 ($d021)
                                            "01": Background color 1 ($d022)
                                            "10": Background color 2 ($d023)
                                            "11": Color from bits 8-10 of c-data
                                    */
                                    if ((bits & 0x300) == 0x300) {
                                        /* special case '11' */
                                        dst[i] = fg;
                                    }
                                    else {
                                        /* one of the 3 background colors */
                                        dst[i] = gseq->bg_rgba8[(bits>>8)&3];
                                    }
                                }
                                else {
                                    /* standard text mode char */
                                    dst[i] = gseq->shift & 0x100 ? fg : bg;
                                }
                            }
                        }
                        break;
                    case 2:
                        /* ECM/BMM/MCM=010, standard bitmap mode */
                        {
                            for (int i = 0; i < 8; i++) {
                                if (_m6567_gseq_tick(vic) & 0x100) {
                                    /* foreground pixel */
                                    dst[i] = _m6567_colors[(gseq->vm >> 4) & 0xF];
                                }
                                else {
                                    /* background pixel */
                                    dst[i] = _m6567_colors[gseq->vm & 0xF];
                                }
                            }
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
                const uint32_t c = gseq->bg_rgba8[0];
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
            if (brd->main) {
                const uint32_t c = brd->bc_rgba8;
                for (int i = 0; i < 8; i++) {
                    dst[i] = c;
                }
            }
        }
    }

    /*--- bump the VC and vmli counters --------------------------------------*/
    if (vic->mem.g_access) {
        vic->rs.vc = (vic->rs.vc + 1) & 0x3FF;        /* VS is a 10-bit counter */
        vic->vm.vmli = (vic->vm.vmli + 1) & 0x3F;     /* VMLI is a 6-bit counter */
    }

    /*--- set CPU pins -------------------------------------------------------*/
    if (vic->mem.ba_pin) {
        pins |= M6567_BA;
    }
    if (vic->mem.aec_pin) {
        pins |= M6567_AEC;
    }
    /* FIXME
    if (vic->irq) {
        pins |= M6567_IRQ;
    }
    */
    return pins;
}

void m6567_display_size(m6567_t* vic, int* out_width, int* out_height) {
    CHIPS_ASSERT(vic && out_width && out_height);
    *out_width = vic->crt.vis_w*8;
    *out_height = vic->crt.vis_h;
}

uint32_t m6567_color(int i) {
    CHIPS_ASSERT((i >= 0) && (i < 16));
    return _m6567_colors[i];
}
#endif /* CHIPS_IMPL */

#ifdef __cplusplus
} /* extern "C" */
#endif
