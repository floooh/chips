#pragma once
/*#
    # m6569.h

    MOS Technology 6569 emulator (aka VIC-II for PAL-B)

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
    *   AEC <---|   M6569   |<--> D0  *
    *           |           |...      *
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

/* address bus lines (with CS active A0..A6 is register address) */
#define M6569_A0    (1ULL<<0)
#define M6569_A1    (1ULL<<1)
#define M6569_A2    (1ULL<<2)
#define M6569_A3    (1ULL<<3)
#define M6569_A4    (1ULL<<4)
#define M6569_A5    (1ULL<<5)
#define M6569_A6    (1ULL<<6)
#define M6569_A7    (1ULL<<7)
#define M6569_A8    (1ULL<<8)
#define M6569_A9    (1ULL<<9)
#define M6569_A10   (1ULL<<10)
#define M6569_A11   (1ULL<<11)
#define M6569_A12   (1ULL<<12)
#define M6569_A13   (1ULL<<13)

/* data bus pins D0..D7 */
#define M6569_D0    (1ULL<<16)
#define M6569_D1    (1ULL<<17)
#define M6569_D2    (1ULL<<18)
#define M6569_D3    (1ULL<<19)
#define M6569_D4    (1ULL<<20)
#define M6569_D5    (1ULL<<21)
#define M6569_D6    (1ULL<<22)
#define M6569_D7    (1ULL<<23)

/* shared control pins */
#define M6569_RW    (1ULL<<24)      /* shared with m6502 CPU */
#define M6569_IRQ   (1ULL<<26)      /* shared with m6502 CPU */
#define M6569_BA    (1ULL<<28)      /* shared with m6502 RDY */
#define M6569_AEC   (1ULL<<29)      /* shared with m6510 AEC */

/* chip-specific control pins */
#define M6569_CS    (1ULL<<40)

/* number of registers */
#define M6569_NUM_REGS (64)
/* register address mask */
#define M6569_REG_MASK (M6569_NUM_REGS-1)
/* number of sprites */
#define M6569_NUM_MOBS (8)

/* extract 8-bit data bus from 64-bit pins */
#define M6569_GET_DATA(p) ((uint8_t)((p&0xFF0000ULL)>>16))
/* merge 8-bit data bus value into 64-bit pins */
#define M6569_SET_DATA(p,d) {p=(((p)&~0xFF0000ULL)|(((d)<<16)&0xFF0000ULL));}

/* memory fetch callback, used to feed pixel- and color-data into the m6569 */
typedef uint16_t (*m6569_fetch_t)(uint16_t addr, void* user_data);

/* setup parameters for m6569_init() function */
typedef struct {
    /* pointer to RGBA8 framebuffer for generated image (optional) */
    uint32_t* rgba8_buffer;
    /* size of the RGBA framebuffer (must be at least 512x312, optional) */
    uint32_t rgba8_buffer_size;
    /* visible CRT area blitted to rgba8_buffer (in pixels) */
    uint16_t vis_x, vis_y, vis_w, vis_h;
    /* the memory-fetch callback */
    m6569_fetch_t fetch_cb;
    /* optional user-data for fetch callback */
    void* user_data;
} m6569_desc_t;

/* register bank */
typedef struct {
    union {
        uint8_t regs[M6569_NUM_REGS];
        struct {
            uint8_t mxy[M6569_NUM_MOBS][2];     /* sprite X/Y coords */
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
            uint8_t mdp;                        /* sprite data priority bits */
            uint8_t mmc;                        /* sprite multicolor bits */
            uint8_t mxe;                        /* sprite X expansion */
            uint8_t mcm;                        /* sprite-sprite collision bits */
            uint8_t mcd;                        /* sprite-data collision bits */
            uint8_t ec;                         /* border color */
            uint8_t bc[4];                      /* background colors */
            uint8_t mm[2];                      /* sprite multicolor 0 */
            uint8_t mc[8];                      /* sprite colors */
            uint8_t unused[17];                 /* not writable, return 0xFF on read */
        };
    };
} m6569_registers_t;

/* control- and interrupt-register bits */
#define M6569_CTRL1_RST8    (1<<7)
#define M6569_CTRL1_ECM     (1<<6)
#define M6569_CTRL1_BMM     (1<<5)
#define M6569_CTRL1_DEN     (1<<4)
#define M6569_CTRL1_RSEL    (1<<3)
#define M6569_CTRL1_YSCROLL ((1<<2)|(1<<1)|(1<<0))
#define M6569_CTRL2_RES     (1<<5)
#define M6569_CTRL2_MCM     (1<<4)
#define M6569_CTRL2_CSEL    (1<<3)
#define M6569_CTRL2_XSCROLL ((1<<2)|(1<<1)|(1<<0))
#define M6569_INT_IRQ       (1<<7)      /* int_latch: interrupt requested */
#define M6569_INT_ILP       (1<<3)      /* int_latch: lightpen interrupt */
#define M6569_INT_IMMC      (1<<2)      /* int_latch: mob/mob collision interrupt */
#define M6569_INT_IMBC      (1<<1)      /* int_latch: mob/bitmap collision interrupt */
#define M6569_INT_IRST      (1<<0)      /* int_latch: raster interrupt */
#define M6569_INT_ELP       (1<<3)      /* int_mask: lightpen interrupt enabled */
#define M6569_INT_EMMC      (1<<2)      /* int_mask: mob/mob collision interrupt enabled */
#define M6569_INT_EMBC      (1<<1)      /* int_mask: mob/bitmap collision interrupt enabled */
#define M6569_INT_ERST      (1<<0)      /* int_mask: raster interrupt enabled */

/* raster unit state */
typedef struct {
    uint8_t h_count;
    uint16_t v_count;
    uint16_t v_irqline;     /* raster interrupt line, updated when ctrl_1 or raster is written */
    uint16_t vc;            /* 10-bit video counter */
    uint16_t vc_base;       /* 10-bit video counter base */
    uint8_t rc;             /* 3-bit raster counter */
    bool display_state;             /* true: in display state, false: in idle state */
    bool badline;                   /* true when the badline state is active */
    bool frame_badlines_enabled;    /* true when badlines are enabled in frame */
} m6569_raster_unit_t;

/* address generator / memory interface state */
typedef struct {
    uint16_t c_addr_or;     /* OR-mask for c-accesses, computed from mem_ptrs */
    uint16_t g_addr_and;    /* AND-mask for g-accesses, computed from mem_ptrs */
    uint16_t g_addr_or;     /* OR-mask for g-accesses, computed from ECM bit */
    uint16_t i_addr;        /* address for i-accesses, 0x3FFF or 0x39FF (if ECM bit set) */
    uint16_t p_addr_or;     /* OR-mask for p-accesses */
    m6569_fetch_t fetch_cb; /* memory-fetch callback */
    void* user_data;        /* optional user-data for fetch callback */
} m6569_memory_unit_t;

/* video matrix state */
typedef struct {
    uint8_t vmli;           /* 6-bit 'vmli' video-matrix line buffer index */
    uint8_t next_vmli;
    uint16_t line[64];      /* 40x 8+4 bits line buffer (64 items because vmli is a 6-bit ctr) */
} m6569_video_matrix_t;

/* border unit state */
typedef struct {
    uint16_t left, right, top, bottom;
    bool main;          /* main border flip-flop */
    bool vert;          /* vertical border flip flop */
    uint8_t bc_index;   /* border color as palette index (not used, but may be useful for outside code) */
    uint32_t bc_rgba8;  /* border color as RGBA8, udpated when border color register is updated */
} m6569_border_unit_t;

/* CRT state tracking */
typedef struct {
    uint16_t x, y;              /* beam pos reset on crt_retrace_h/crt_retrace_v zero */
    uint16_t vis_x0, vis_y0, vis_x1, vis_y1;  /* the visible area */
    uint16_t vis_w, vis_h;      /* width of visible area */
    uint32_t* rgba8_buffer;
} m6569_crt_t;

/* graphics sequencer state */
typedef struct {
    bool enabled;               /* true while g_accesses are happening */
    uint8_t mode;               /* display mode 0..7 precomputed from ECM/BMM/MCM bits */
    uint8_t count;              /* counts from 0..8 */
    uint8_t shift;              /* current pixel shifter */
    uint8_t outp;               /* current output byte (bit 7) */
    uint8_t outp2;              /* current output byte at half frequency (bits 7 and 6) */
    uint16_t c_data;            /* loaded from video matrix line buffer */
    uint8_t bg_index[4];        /* background color as palette index (not used, but may be useful for outside code) */
    uint32_t bg_rgba8[4];       /* background colors as RGBA8 */
} m6569_graphics_unit_t;

/* sprite sequencer state */
typedef struct {
    /* updated when setting sprite position registers */
    uint8_t h_first[8];         /* first horizontal visible tick */
    uint8_t h_last[8];          /* last horizontal visible tick */
    uint8_t h_offset[8];        /* x-offset within 8-pixel raster */
    uint8_t p_data[8];          /* the byte read by p_access memory fetch */
    bool dma_enabled[8];        /* sprite dma is enabled */
    bool disp_enabled[8];       /* sprite display is enabled */
    bool expand[8];             /* expand flip-flop */
    uint8_t mc[8];              /* 6-bit mob-data-counter */
    uint8_t mc_base[8];         /* 6-bit mob-data-counter base */
    uint8_t delay_count[8];     /* 0..7 delay pixels */
    uint8_t outp2_count[8];     /* outp2 is updated when bit 0 is on */
    uint8_t xexp_count[8];      /* if x stretched, only shift every second pixel tick */
    uint32_t shift[8];          /* 24-bit shift register */
    uint32_t outp[8];           /* current shifter output (bit 31) */
    uint32_t outp2[8];          /* current shifter output at half frequency (bits 31 and 30) */
    uint32_t colors[8][4];      /* 0: unused, 1: multicolor0, 2: main color, 3: multicolor1
                                   the alpha channel is cleared and used as bitmask for sprites
                                   which produced a color
                                */
} m6569_sprite_unit_t;

/* the m6569 state structure */
typedef struct {
    bool debug_vis;             /* toggle this to switch debug visualization on/off */
    m6569_registers_t reg;
    m6569_crt_t crt;
    m6569_border_unit_t brd;
    m6569_raster_unit_t rs;
    m6569_memory_unit_t mem;
    m6569_graphics_unit_t gunit;
    m6569_sprite_unit_t sunit;
    m6569_video_matrix_t vm;
    uint64_t pins;
} m6569_t;

/* initialize a new m6569_t instance */
void m6569_init(m6569_t* vic, const m6569_desc_t* desc);
/* reset a m6569_t instance */
void m6569_reset(m6569_t* vic);
/* tick the m6569 instance */
uint64_t m6569_tick(m6569_t* vic, uint64_t pins);
/* get the visible display width in pixels */
int m6569_display_width(m6569_t* vic);
/* get the visible display height in pixels */
int m6569_display_height(m6569_t* vic);
/* get 32-bit RGBA8 value from color index (0..15) */
uint32_t m6569_color(int i);

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

#define _M6569_RGBA8(r,g,b) (0xFF000000|(b<<16)|(g<<8)|(r))

/*
http://unusedino.de/ec64/technical/misc/vic656x/colors/
static const uint32_t _m6569_colors[16] = {
    _M6569_RGBA8(0x00,0x00,0x00),
    _M6569_RGBA8(0xFF,0xFF,0xFF),
    _M6569_RGBA8(0x68,0x37,0x2B),
    _M6569_RGBA8(0x70,0xA4,0xB2),
    _M6569_RGBA8(0x6F,0x3D,0x86),
    _M6569_RGBA8(0x58,0x8D,0x43),
    _M6569_RGBA8(0x35,0x28,0x79),
    _M6569_RGBA8(0xB8,0xC7,0x6F),
    _M6569_RGBA8(0x6F,0x4F,0x25),
    _M6569_RGBA8(0x43,0x39,0x00),
    _M6569_RGBA8(0x9A,0x67,0x59),
    _M6569_RGBA8(0x44,0x44,0x44),
    _M6569_RGBA8(0x6C,0x6C,0x6C),
    _M6569_RGBA8(0x9A,0xD2,0x84),
    _M6569_RGBA8(0x6C,0x5E,0xB5),
    _M6569_RGBA8(0x95,0x95,0x95)
};
*/

/* https://www.pepto.de/projects/colorvic/ */
static const uint32_t _m6569_colors[16] = {
    _M6569_RGBA8(0x00,0x00,0x00),
    _M6569_RGBA8(0xff,0xff,0xff),
    _M6569_RGBA8(0x81,0x33,0x38),
    _M6569_RGBA8(0x75,0xce,0xc8),
    _M6569_RGBA8(0x8e,0x3c,0x97),
    _M6569_RGBA8(0x56,0xac,0x4d),
    _M6569_RGBA8(0x2e,0x2c,0x9b),
    _M6569_RGBA8(0xed,0xf1,0x71),
    _M6569_RGBA8(0x8e,0x50,0x29),
    _M6569_RGBA8(0x55,0x38,0x00),
    _M6569_RGBA8(0xc4,0x6c,0x71),
    _M6569_RGBA8(0x4a,0x4a,0x4a),
    _M6569_RGBA8(0x7b,0x7b,0x7b),
    _M6569_RGBA8(0xa9,0xff,0x9f),
    _M6569_RGBA8(0x70,0x6d,0xeb),
    _M6569_RGBA8(0xb2,0xb2,0xb2),
};

/* valid register bits */
static const uint8_t _m6569_reg_mask[M6569_NUM_REGS] = {
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
#define _M6569_HTOTAL               (63)    /* 63 cycles per line (PAL) */
#define _M6569_VTOTAL               (312)   /* 312 lines total (PAL) */
#define _M6569_VRETRACEPOS          (303)   /* start of vertical beam retrace */
#define _M6569_RSEL1_BORDER_TOP     (51)    /* top border when RSEL=1 (25 rows) */
#define _M6569_RSEL1_BORDER_BOTTOM  (251)   /* bottom border when RSEL=1 */
#define _M6569_RSEL0_BORDER_TOP     (55)    /* top border when RSEL=0 (24 rows) */
#define _M6569_RSEL0_BORDER_BOTTOM  (247)   /* bottom border when RSEL=0 (24 rows) */
#define _M6569_CSEL1_BORDER_LEFT    (16)    /* left border when CSEL=1 (40 columns) */
#define _M6569_CSEL1_BORDER_RIGHT   (56)    /* right border when CSEL=1 */
#define _M6569_CSEL0_BORDER_LEFT    (17)    /* left border when CSEL=0 (38 columns) */
#define _M6569_CSEL0_BORDER_RIGHT   (55)    /* right border when CSEL=0 */

#define _M6569_RAST(r)              (vic->rs.v_count == (r))
#define _M6569_RAST_RANGE(r0,r1)    ((vic->rs.v_count >= (r0)) && (vic->rs.v_count <= (r1)))

/*--- init -------------------------------------------------------------------*/
static void _m6569_init_crt(m6569_crt_t* crt, const m6569_desc_t* desc) {
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

void m6569_init(m6569_t* vic, const m6569_desc_t* desc) {
    CHIPS_ASSERT(vic && desc);
    CHIPS_ASSERT((0 == desc->rgba8_buffer) || (desc->rgba8_buffer_size >= (_M6569_HTOTAL*8*_M6569_VTOTAL*sizeof(uint32_t))));
    memset(vic, 0, sizeof(*vic));
    _m6569_init_crt(&vic->crt, desc);
    vic->mem.fetch_cb = desc->fetch_cb;
    vic->mem.user_data = desc->user_data;
}

/*--- reset ------------------------------------------------------------------*/
static void _m6569_reset_register_bank(m6569_registers_t* r) {
    memset(r, 0, sizeof(*r));
}

static void _m6569_reset_raster_unit(m6569_raster_unit_t* r) {
    r->h_count = 0;
    r->v_count = 0;
    r->v_irqline = 0;
    r->vc = r->vc_base = 0;
    r->rc = 0;
    r->display_state = false;
    r->badline = false;
    r->frame_badlines_enabled = false;
}

static void _m6569_reset_memory_unit(m6569_memory_unit_t* m) {
    m->c_addr_or = 0;
    m->g_addr_and = 0;
    m->g_addr_or = 0;
    m->i_addr = 0;
    m->p_addr_or = 0;
}

static void _m6569_reset_video_matrix_unit(m6569_video_matrix_t* vm) {
    memset(vm, 0, sizeof(*vm));
}

static void _m6569_reset_graphics_unit(m6569_graphics_unit_t* gu) {
    memset(gu, 0, sizeof(*gu));
}

static void _m6569_reset_sprite_unit(m6569_sprite_unit_t* su) {
    memset(su, 0, sizeof(*su));
}

static void _m6569_reset_border_unit(m6569_border_unit_t* b) {
    b->main = b->vert = false;
}

static void _m6569_reset_crt(m6569_crt_t* c) {
    c->x = c->y = 0;
}

void m6569_reset(m6569_t* vic) {
    CHIPS_ASSERT(vic);
    _m6569_reset_register_bank(&vic->reg);
    _m6569_reset_raster_unit(&vic->rs);
    _m6569_reset_crt(&vic->crt);
    _m6569_reset_border_unit(&vic->brd);
    _m6569_reset_memory_unit(&vic->mem);
    _m6569_reset_video_matrix_unit(&vic->vm);
    _m6569_reset_graphics_unit(&vic->gunit);
    _m6569_reset_sprite_unit(&vic->sunit);
}

/*--- register read/writes ---------------------------------------------------*/

/* update the raster-interrupt line from ctrl_1 and raster register updates */
static inline void _m6569_io_update_irq_line(m6569_raster_unit_t* rs, uint8_t ctrl_1, uint8_t rast) {
    rs->v_irqline = ((ctrl_1 & M6569_CTRL1_RST8)<<1) | rast;
}

/* update memory unit values after update mem_ptrs or ctrl_1 registers */
static inline void _m6569_io_update_memory_unit(m6569_memory_unit_t* m, uint8_t mem_ptrs, uint8_t ctrl_1) {
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
    if (ctrl_1 & M6569_CTRL1_ECM) {
        m->g_addr_and &= ~((1<<10)|(1<<9));
        m->i_addr &= ~((1<<10)|(1<<9));
    }
}

/* update the border top/bottom position when updating csel */
static inline void _m6569_io_update_border_rsel(m6569_border_unit_t* b, uint8_t ctrl_1) {
    if (ctrl_1 & M6569_CTRL1_RSEL) {
        /* RSEL 1: 25 rows */
        b->top = _M6569_RSEL1_BORDER_TOP;
        b->bottom = _M6569_RSEL1_BORDER_BOTTOM;
    }
    else {
        /* RSEL 0: 24 rows */
        b->top = _M6569_RSEL0_BORDER_TOP;
        b->bottom = _M6569_RSEL0_BORDER_BOTTOM;
    }
}

/* update the border left/right position when updating csel */
static inline void _m6569_io_update_border_csel(m6569_border_unit_t* b, uint8_t ctrl_2) {
    if (ctrl_2 & M6569_CTRL2_CSEL) {
        /* CSEL 1: 40 columns */
        b->left = _M6569_CSEL1_BORDER_LEFT;
        b->right = _M6569_CSEL1_BORDER_RIGHT;
    }
    else {
        /* CSEL 0: 38 columns */
        b->left = _M6569_CSEL0_BORDER_LEFT;
        b->right = _M6569_CSEL0_BORDER_RIGHT;
    }
}

/* updates the graphics sequencer display mode (0..7) from the ECM/BMM/MCM bits */
static inline void _m6569_io_update_gunit_mode(m6569_graphics_unit_t* gu, uint8_t ctrl_1, uint8_t ctrl_2) {
    gu->mode = ((ctrl_1&(M6569_CTRL1_ECM|M6569_CTRL1_BMM))|(ctrl_2&M6569_CTRL2_MCM))>>4;
}

/* update sprite unit positions and sizes when updating registers */
static void _m6569_io_update_sunit(m6569_t* vic, int i, uint8_t mx, uint8_t my, uint8_t mx8, uint8_t mxe, uint8_t mye) {
    (void)my;   // FIXME: my is really unused?
    m6569_sprite_unit_t* su = &vic->sunit;
    /* mxb: MSB for each xpos */
    uint16_t xpos = ((mx8 & (1<<i))<<(8-i)) | mx;
    su->h_first[i]  = (xpos / 8) + 13;
    su->h_offset[i] = (xpos & 7);
    const uint16_t w = ((mxe & (1<<i)) ? 5 : 2) + ((su->h_offset[i] > 0) ? 1:0);
    su->h_last[i] = su->h_first[i] + w;
    /* 1. The expansion flip flop is set as long as the bit in MxYE in register
        $d017 corresponding to the sprite is cleared.
    */
    if ((mye & (1<<i)) == 0) {
        su->expand[i] = true;
    }
}

/* read chip registers */
static uint64_t _m6569_read(m6569_t* vic, uint64_t pins) {
    m6569_registers_t* r = &vic->reg;
    const m6569_raster_unit_t* rs = &vic->rs;
    uint8_t r_addr = pins & M6569_REG_MASK;
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
            data = r->regs[r_addr] | ~_m6569_reg_mask[r_addr];
            break;
    }
    M6569_SET_DATA(pins, data);
    return pins;
}

/* write chip registers */
static void _m6569_write(m6569_t* vic, uint64_t pins) {
    m6569_registers_t* r = &vic->reg;
    uint8_t r_addr = pins & M6569_REG_MASK;
    const uint8_t data = M6569_GET_DATA(pins) & _m6569_reg_mask[r_addr];
    bool write = true;
    switch (r_addr) {
        case 0x00:  /* m0x */
            _m6569_io_update_sunit(vic, 0, data, r->mxy[0][1], r->mx8, r->mxe, r->mye);
            break;
        case 0x01:  /* m0y */
            _m6569_io_update_sunit(vic, 0, r->mxy[0][0], data, r->mx8, r->mxe, r->mye);
            break;
        case 0x02:  /* m1x */
            _m6569_io_update_sunit(vic, 1, data, r->mxy[1][1], r->mx8, r->mxe, r->mye);
            break;
        case 0x03:  /* m1y */
            _m6569_io_update_sunit(vic, 1, r->mxy[1][0], data, r->mx8, r->mxe, r->mye);
            break;
        case 0x04:  /* m2x */
            _m6569_io_update_sunit(vic, 2, data, r->mxy[2][1], r->mx8, r->mxe, r->mye);
            break;
        case 0x05:  /* m2y */
            _m6569_io_update_sunit(vic, 2, r->mxy[2][0], data, r->mx8, r->mxe, r->mye);
            break;
        case 0x06:  /* m3x */
            _m6569_io_update_sunit(vic, 3, data, r->mxy[3][1], r->mx8, r->mxe, r->mye);
            break;
        case 0x07:  /* m3y */
            _m6569_io_update_sunit(vic, 3, r->mxy[3][0], data, r->mx8, r->mxe, r->mye);
            break;
        case 0x08:  /* m4x */
            _m6569_io_update_sunit(vic, 4, data, r->mxy[4][1], r->mx8, r->mxe, r->mye);
            break;
        case 0x09:  /* m4y */
            _m6569_io_update_sunit(vic, 4, r->mxy[4][0], data, r->mx8, r->mxe, r->mye);
            break;
        case 0x0A:  /* m5x */
            _m6569_io_update_sunit(vic, 5, data, r->mxy[5][1], r->mx8, r->mxe, r->mye);
            break;
        case 0x0B:  /* m5y */
            _m6569_io_update_sunit(vic, 5, r->mxy[5][0], data, r->mx8, r->mxe, r->mye);
            break;
        case 0x0C:  /* m6x */
            _m6569_io_update_sunit(vic, 6, data, r->mxy[6][1], r->mx8, r->mxe, r->mye);
            break;
        case 0x0D:  /* m6y */
            _m6569_io_update_sunit(vic, 6, r->mxy[6][0], data, r->mx8, r->mxe, r->mye);
            break;
        case 0x0E:  /* m7x */
            _m6569_io_update_sunit(vic, 7, data, r->mxy[7][1], r->mx8, r->mxe, r->mye);
            break;
        case 0x0F:  /* m7y */
            _m6569_io_update_sunit(vic, 7, r->mxy[7][0], data, r->mx8, r->mxe, r->mye);
            break;
        case 0x10:  /* mx8 */
            for (int i = 0; i < 8; i++) {
                _m6569_io_update_sunit(vic, i, r->mxy[i][0], r->mxy[i][1], data, r->mxe, r->mye);
            }
            break;
        case 0x11:  /* ctrl_1 */
            /* update raster interrupt line */
            _m6569_io_update_irq_line(&vic->rs, data, r->raster);
            /* update border top/bottom from RSEL flag */
            _m6569_io_update_border_rsel(&vic->brd, data);
            /* ECM bit updates the precomputed fetch address masks */
            _m6569_io_update_memory_unit(&vic->mem, r->mem_ptrs, data);
            /* update the graphics mode */
            _m6569_io_update_gunit_mode(&vic->gunit, data, r->ctrl_2);
            break;
        case 0x12:
            /* raster irq value lower 8 bits */
            _m6569_io_update_irq_line(&vic->rs, r->ctrl_1, data);
            break;
        case 0x16: /* ctrl_2 */
            /* update border left/right from CSEL flag */
            _m6569_io_update_border_csel(&vic->brd, data);
            /* update the graphics mode */
            _m6569_io_update_gunit_mode(&vic->gunit, r->ctrl_1, data);
            break;
        case 0x17:  /* mye */
            for (int i = 0; i < 8; i++) {
                _m6569_io_update_sunit(vic, i, r->mxy[i][0], r->mxy[i][1], r->mx8, r->mxe, data);
            }
            break;
        case 0x18:
            /* memory-ptrs register , update precomputed fetch address masks */
            _m6569_io_update_memory_unit(&vic->mem, data, r->ctrl_1);
            break;
        case 0x19:
            /* interrupt latch: to clear a bit in the latch, a 1-bit
               must be written to the latch!
            */
            r->int_latch = (r->int_latch & ~data) & _m6569_reg_mask[0x19];
            write = false;
            break;
        case 0x1D:  /* mxe */
            for (int i = 0; i < 8; i++) {
                _m6569_io_update_sunit(vic, i, r->mxy[i][0], r->mxy[i][1], r->mx8, data, r->mye);
            }
            break;
        case 0x1E: case 0x1F:
            /* mob collision registers cannot be written */
            write = false;
            break;
        case 0x20:
            /* border color */
            vic->brd.bc_index = data & 0xF;
            vic->brd.bc_rgba8 = _m6569_colors[data & 0xF];
            break;
        case 0x21: case 0x22:
            /* background colors (alpha bits 0 because these count as MCM BG colors) */
            vic->gunit.bg_index[r_addr-0x21] = data & 0xF;
            vic->gunit.bg_rgba8[r_addr-0x21] = _m6569_colors[data & 0xF] & 0x00FFFFFF;
            break;
        case 0x23: case 0x24:
            /* background colors (alpha bits 1 because these count as MCM FG colors) */
            vic->gunit.bg_index[r_addr-0x21] = data & 0xF;
            vic->gunit.bg_rgba8[r_addr-0x21] = _m6569_colors[data & 0xF];
            break;
        case 0x25:
            /* sprite multicolor 0 */
            for (int i = 0; i < 8; i++) {
                vic->sunit.colors[i][1] = _m6569_colors[data & 0xF] & 0x00FFFFFF;
            }
            break;
        case 0x26:
            /* sprite multicolor 1*/
            for (int i = 0; i < 8; i++) {
                vic->sunit.colors[i][3] = _m6569_colors[data & 0xF] & 0x00FFFFFF;
            }
            break;
        case 0x27: case 0x28: case 0x29: case 0x2A:
        case 0x2B: case 0x2C: case 0x2D: case 0x2E:
            /* sprite main color */
            vic->sunit.colors[r_addr-0x27][2] = _m6569_colors[data & 0xF] & 0x00FFFFFF;
            break;
    }
    if (write) {
        r->regs[r_addr] = data;
    }
}

/*--- graphics sequencer helpers ---------------------------------------------*/

/* start the graphics sequencer, this happens at the first g_access,
   the graphics sequencer must be delayed by xscroll
*/
static inline void _m6569_gunit_rewind(m6569_t* vic, uint8_t xscroll) {
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
static inline void _m6569_gunit_tick(m6569_t* vic, uint8_t g_data) {
    if (vic->gunit.count == 0) {
        vic->gunit.count = 7;
        vic->gunit.shift |= g_data;
        vic->gunit.c_data = vic->gunit.enabled ? vic->vm.line[vic->vm.vmli] : 0;
    }
    else {
        vic->gunit.count--;
    }
    vic->gunit.outp = vic->gunit.shift;
    if (0 != (vic->gunit.count & 1)) {
        vic->gunit.outp2 = vic->gunit.shift;
    }
    vic->gunit.shift <<= 1;
}

/* 
    graphics sequencer decoding functions for 1 pixel

    NOTE: the graphics sequencer returns alpha bits = 0 for background
    colors, and alpha bits = 0xFF for foregreound colors, this is important
    for the color multiplexer which selectes between the color
    produced by the graphics- and sprite-units
*/
static inline uint32_t _m6569_gunit_decode_mode0(m6569_t* vic) {
    if (vic->gunit.outp & 0x80) {
        /* foreground color (alpha bits set) */
        return _m6569_colors[(vic->gunit.c_data>>8)&0xF];
    }
    else {
        /* background color (alpha bits clear) */
        return vic->gunit.bg_rgba8[0];
    }
}

static inline uint32_t _m6569_gunit_decode_mode1(m6569_t* vic) {
    /* only seven colors in multicolor mode */
    const uint32_t fg = _m6569_colors[(vic->gunit.c_data>>8) & 0x7];
    if (vic->gunit.c_data & (1<<11)) {
        /* outp2 is only updated every 2 ticks */
        uint8_t bits = ((vic->gunit.outp2)>>6) & 3;
        /* half resolution multicolor char
            need 2 bits from the pixel sequencer
                "00": Background color 0 ($d021) (alpha bits clear)
                "01": Background color 1 ($d022) (alpha bits clear)
                "10": Background color 2 ($d023) (alpha bits set)
                "11": Color from bits 8-10 of c-data (alpha bits set)
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
            /* foreground color (alpha bits set) */
            return fg;
        }
        else {
            /* background color (alpha bits clear) */
            return vic->gunit.bg_rgba8[0];
        }
    }
}

static inline uint32_t _m6569_gunit_decode_mode2(m6569_t* vic) {
    if (vic->gunit.outp & 0x80) {
        /* foreground pixel */
        return _m6569_colors[(vic->gunit.c_data >> 4) & 0xF];
    }
    else {
        /* background pixel (alpha bits must be clear for multiplexer) */
        return _m6569_colors[vic->gunit.c_data & 0xF] & 0x00FFFFFF;
    }
}

static inline uint32_t _m6569_gunit_decode_mode3(m6569_t* vic) {
    /* shift 2 is only updated every 2 ticks */
    uint8_t bits = vic->gunit.outp2;
    /* half resolution multicolor char
        need 2 bits from the pixel sequencer
            "00": Background color 0 ($d021) (alpha bits clear)
            "01": Color from bits 4-7 of c-data (alpha bits clear)
            "10": Color from bits 0-3 of c-data (alpha bits set)
            "11": Color from bits 8-11 of c-data (alpha bits set)
    */
    switch ((bits>>6)&3) {
        case 0:     return vic->gunit.bg_rgba8[0]; break;
        case 1:     return _m6569_colors[(vic->gunit.c_data>>4) & 0xF] & 0x00FFFFFF; break;
        case 2:     return _m6569_colors[vic->gunit.c_data & 0xF]; break;
        default:    return _m6569_colors[(vic->gunit.c_data>>8) & 0xF]; break;
    }
}

static inline uint32_t _m6569_gunit_decode_mode4(m6569_t* vic) {
    if (vic->gunit.outp & 0x80) {
        /* foreground color as usual bits 8..11 of c_data */
        return _m6569_colors[(vic->gunit.c_data>>8) & 0xF];
    }
    else {
        /* bg color selected by bits 6 and 7 of c_data */
        /* FIXME: is the foreground/background selection right? 
            values 00 and 01 would return as background color,
            and 10 and 11 as foreground color?
        */
        return vic->gunit.bg_rgba8[(vic->gunit.c_data>>6) & 3];
    }
}

/*--- sprite sequencer helper ------------------------------------------------*/

static inline void _m6569_sunit_start(m6569_t* vic) {
    /*
        1. The expansion flip flop is set as long as the bit in MxYE in register
         $d017 corresponding to the sprite is cleared.
         (FIXME: this is currently only done when updating the MxYE register)
        2. If the MxYE bit is set in the first phase of cycle 55, the expansion
         flip flop is inverted.
        3. In the first phases of cycle 55 and 56, the VIC checks for every sprite
         if the corresponding MxE bit in register $d015 is set and the Y
         coordinate of the sprite (odd registers $d001-$d00f) match the lower 8
         bits of RASTER. If this is the case and the DMA for the sprite is still
         off, the DMA is switched on, MCBASE is cleared, and if the MxYE bit is
         set the expansion flip flip is reset.

         NOTE: sprite disp_enabled flag is turned off here if dma flag is off,
         this is different from the recipe in vic-ii.txt (switching off display
         in tick 15/16 as described in the recipe turns off rendering the
         last line of a sprite)
    */
    const uint8_t me = vic->reg.me;
    const uint8_t mye = vic->reg.mye;
    m6569_sprite_unit_t* su = &vic->sunit;
    for (int i = 0; i < 8; i++) {
        const uint8_t mask = (1<<i);
        if (mye & mask) {
            su->expand[i] = !su->expand[i];
        }
        if ((me & mask) && ((vic->rs.v_count & 0xFF) == vic->reg.mxy[i][1])) {
            if (!su->dma_enabled[i]) {
                su->dma_enabled[i] = true;
                su->mc_base[i] = 0;
                if (mye & mask) {
                    su->expand[i] = false;
                }
            }
        }
        /* NOTE: the following behaviour differes from the recipe */
        if (!su->dma_enabled[i]) {
            su->disp_enabled[i] = false;
        }
    }
}

static inline void _m6569_sunit_update_mc_disp_enable(m6569_t* vic) {
    /* 4. In the first phase of cycle 58, the MC of every sprite is loaded from
        its belonging MCBASE (MCBASE->MC) and it is checked if the DMA for the
        sprite is turned on and the Y coordinate of the sprite matches the lower
        8 bits of RASTER. If this is the case, the display of the sprite is
        turned on.
    */
    m6569_sprite_unit_t* su = &vic->sunit;
    for (int i = 0; i < 8; i++) {
        su->mc[i] = su->mc_base[i];
        if (su->dma_enabled[i] && ((vic->rs.v_count & 0xFF) == vic->reg.mxy[i][1])) {
            su->disp_enabled[i] = true;
        }
    }
}

/*
    7. In the first phase of cycle 15, it is checked if the expansion flip flop
    is set. If so, MCBASE is incremented by 2.
*/
static inline void _m6569_sunit_update_mcbase(m6569_t* vic) {
    m6569_sprite_unit_t* su = &vic->sunit;
    for (int i = 0; i < 8; i++) {
        if (su->expand[i]) {
            su->mc_base[i] = (su->mc_base[i] + 2) & 0x3F;
        }
    }
}

/*
    8. In the first phase of cycle 16, it is checked if the expansion flip flop
    is set. If so, MCBASE is incremented by 1. After that, the VIC checks if
    MCBASE is equal to 63 and turns of the DMA and the display of the sprite
    if it is.

    NOTE: The display is turned off in _m6569_sunit_update_mc_disp_enable()
    instead (at the end of the last valid rasterline the sprite must
    be displayed). Otherwise the last line won't be displayed.

    This is different behaviour than described in the recipe!.
    
    BUT: https://sourceforge.net/p/vice-emu/code/HEAD/tree/techdocs/VICII/VIC-Addendum.txt
    mentions this:

        Note: The original rule 8 mentions turning the display of the sprite off
        if MCBASE is equal to 63. If this were true, then the last line of the sprite
        would not be displayed beyond coordinates corresponsing to cycle 16.
        The above rewritten rule corrects this. The actual disabling of sprite display
        is likely handled during the first phase of cycle 58 (see rule 4).
*/
static inline void _m6569_sunit_dma_disp_disable(m6569_t* vic) {
    m6569_sprite_unit_t* su = &vic->sunit;
    for (int i = 0; i < 8; i++) {
        if (su->expand[i]) {
            su->mc_base[i] = (su->mc_base[i] + 1) & 0x3F;
        }
        if (su->mc_base[i] == 0x3F) {
            su->dma_enabled[i] = false;
        }
    }
}

/* set the BA pin if a sprite's DMA is enabled */
static inline uint64_t _m6569_sunit_dma_ba(m6569_t* vic, uint32_t s_index, uint64_t pins) {
    if (vic->sunit.dma_enabled[s_index]) {
        pins |= M6569_BA;
    }
    return pins;
}

/* set the AEC pin if a sprite's DMA is enabled */
static inline uint64_t _m6569_sunit_dma_aec(m6569_t* vic, uint32_t s_index, uint64_t pins) {
    if (vic->sunit.dma_enabled[s_index]) {
        pins |= M6569_AEC;
    }
    return pins;
}

static inline uint32_t _m6569_sunit_decode(m6569_t* vic, uint8_t hpos) {
    /* this will tick all the sprite units and return the color
        of the highest-priority sprite color for the current pixel,
        or 0 if the sprite units didn't produce a color 

        NOTE: The alpha channel bits of the color are cleared, and 
        instead a bit is set for the highest-priority sprite color
        which produced the color!
    */
    uint32_t c = 0;
    bool collision = false;
    m6569_sprite_unit_t* su = &vic->sunit;
    uint8_t mxe = vic->reg.mxe;
    uint8_t mmc = vic->reg.mmc;
    for (int i = 0; i < 8; i++) {
        if (su->disp_enabled[i] && (hpos >= su->h_first[i]) && (hpos <= su->h_last[i])) {
            if (su->delay_count[i] == 0) {
                if ((0 == (su->xexp_count[i]++ & 1)) || (0 == (mxe & (1<<i)))) {
                    /* bit 31 of outp is the current shifter output */
                    su->outp[i] = su->shift[i];
                    /* bits 31 and 30 of outp is half-frequency shifter output */
                    if (0 == (su->outp2_count[i]++ & 1)) {
                        su->outp2[i] = su->shift[i];
                    }
                    su->shift[i] <<= 1;
                }
                if (mmc & (1<<i)) {
                    /* multicolor mode */
                    uint32_t ci = (su->outp2[i] & ((1<<31)|(1<<30)))>>30;
                    if (ci != 0) {
                        /* don't overwrite higher-priority colors */
                        if (0 == c) {
                            c = su->colors[i][ci];
                        }
                        else {
                            collision = true;
                        }
                        c |= (1<<(24+i));
                    }
                }
                else {
                    /* standard color mode */
                    if (su->outp[i] & (1<<31)) {
                        /* don't overwrite higher-priority colors */
                        if (0 == c) {
                            c = su->colors[i][2] | (1<<(24+i));
                        }
                        else {
                            collision = true;
                        }
                        c |= (1<<(24+i));
                    }
                }
            }
            else {
                su->delay_count[i]--;
            }
        }
    }
    if (collision) {
        vic->reg.mcm |= (c>>24);
        vic->reg.int_latch |= M6569_INT_IMMC;
    }
    return c;
}

/*
    Check for mob-data collision.

    Takes the bitmap color bmc (alpha bits 0 if background color)
    the sprite color sc (alpha bits set for each sprite which
    produced a non-transparant color), and sets the md
    collision bitmask, and the IMBC interrupt bit.
*/
static inline void _m6569_test_mob_data_col(m6569_t* vic, uint32_t bmc, uint32_t sc) {
    if ((sc & bmc & 0xFF000000) != 0) {
        vic->reg.mcd |= (sc>>24);
        vic->reg.int_latch |= M6569_INT_IMBC;
    }
}

/* 
    The graphics/sprite color priority multiplexer.

    - the sprite color is 0 if the sprite unit didn't produce a color
    - the alpha channel bits of the sprite color have a bit set for the
      sprite unit which produced the color
    - the alpha channel bits of the bitmap color are 0x00 for a background
      color, or 0xFF for a foreground color
*/
static inline uint32_t _m6569_color_multiplex(uint32_t bmc, uint32_t sc, uint8_t mdp) {
    uint32_t c;
    if (sc == 0) {
        /* sprite unit didn't produce a color, use the bitmap color */
        c = bmc;
    }
    else if ((sc>>24) & mdp) {
        /* data priority bit is set, sprite color is behind bitmap foreground color */
        if ((bmc & 0xFF000000) == 0) {
            /* bitmap color is background, use sprite color */
            c = sc;
        }
        else {
            /* bitmap color is foreground */
            c = bmc;
        }
    }
    else {
        /* sprite color is in front of bitmap color */
        c = sc;
    }
    return c | 0xFF000000;
}

/* decode the next 8 pixels */
static inline void _m6569_decode_pixels(m6569_t* vic, uint8_t g_data, uint32_t* dst, uint8_t hpos) {

    m6569_sprite_unit_t* su = &vic->sunit;
    for (int i = 0; i < 8; i++) {
        if (su->disp_enabled[i] && (hpos == su->h_first[i])) {
            su->delay_count[i] = su->h_offset[i];
            su->outp2_count[i] = 0;
            su->xexp_count[i] = 0;
        }
    }

    /*
        "...the vertical border flip flop controls the output of the graphics
        data sequencer. The sequencer only outputs data if the flip flop is
        not set..."

        NOTES about colors:
            - if none of the sprite units produced a color the returned
              color is 0
            - the alpha channel of the sprite color is discarded and instead
              used as bitmask to communicate which of the sprite units
              produced the color, this is used to check the priority bit
              for this sprite
            - likewise the alpha channel bits for the graphics sequencer
              are used to communicate whether the returned color is a 
              background or foreground colors (fg: alpha == 0xFF, bg: alpha == 0)

        "The main border flip flop controls the border display. If it is set, the
        VIC displays the color stored in register $d020, otherwise it displays the
        color that the priority multiplexer switches through from the graphics or
        sprite data sequencer. So the border overlays the text/bitmap graphics as
        well as the sprites. It has the highest display priority."

        ...otherwise it displays the background color
    */
    bool brd = vic->brd.vert | vic->brd.main;
    uint32_t brd_color = vic->brd.main ? vic->brd.bc_rgba8 : vic->gunit.bg_rgba8[0];
    const uint8_t mdp = vic->reg.mdp;
    const uint8_t mode = vic->gunit.mode;
    uint32_t bmc = 0;
    for (int i = 0; i < 8; i++) {
        uint32_t sc = _m6569_sunit_decode(vic, hpos);
        _m6569_gunit_tick(vic, g_data);
        switch (mode) {
            case 0: bmc = _m6569_gunit_decode_mode0(vic); break;
            case 1: bmc = _m6569_gunit_decode_mode1(vic); break;
            case 2: bmc = _m6569_gunit_decode_mode2(vic); break;
            case 3: bmc = _m6569_gunit_decode_mode3(vic); break;
            case 4: bmc = _m6569_gunit_decode_mode4(vic); break;
        }
        _m6569_test_mob_data_col(vic, bmc, sc);
        dst[i] = brd ? brd_color : _m6569_color_multiplex(bmc, sc, mdp);
    }
}

/* decode the next 8 pixels as debug visualization */
static void _m6569_decode_pixels_debug(m6569_t* vic, uint8_t g_data, bool ba_pin, uint32_t* dst, uint8_t hpos) {
    _m6569_decode_pixels(vic, g_data, dst, hpos);
    dst[0] = (dst[0] & 0xFF000000) | 0x00222222;
    uint32_t mask = 0x00000000;
    if (vic->rs.badline) {
        mask = 0x00FF0000;
    }
    if (ba_pin) {
        mask = 0x000000FF;
    }
    /* sprites */
    const m6569_sprite_unit_t* su = &vic->sunit;
    for (int si = 0; si < 8; si++) {
        if (su->disp_enabled[si]) {
            if ((hpos >= su->h_first[si]) && (hpos <= su->h_last[si])) {
                mask |= 0x00880088;
            }
        }
    }
    /* main interrupt bit */
    if (vic->reg.int_latch & (1<<7)) {
        mask |= 0x0000FF00;
    }
    if (mask != 0) {
        for (int i = 0; i < 8; i++) {
            dst[i] = (dst[i] & 0xFF888888) | mask;
        }
    }
}

/*
    (see 3.7.2 in http://www.zimmers.net/cbmpics/cbm/c64/vic-ii.txt)

    FIXME: "Raster line 0 is, however, an exception: In this line, IRQ
    and incrementing (resp. resetting) of RASTER are performed one cycle later
    than in the other lines.
*/
static inline void _m6569_rs_next_rasterline(m6569_t* vic) {
    vic->rs.h_count = 0;
    /* new scanline */
    if (vic->rs.v_count == (_M6569_VTOTAL-1)) {
        vic->rs.v_count = 0;
        vic->rs.vc_base = 0;
    }
    else {
        vic->rs.v_count++;
    }
}

static inline void _m6569_rs_check_irq(m6569_t* vic) {
    if (vic->rs.v_count == vic->rs.v_irqline) {
        vic->reg.int_latch |= M6569_INT_IRST;
    }
}

/*
    Update the badline state:

    ( see 3.5 http://www.zimmers.net/cbmpics/cbm/c64/vic-ii.txt )

    "A Bad Line Condition is given at any arbitrary clock cycle, if at the
    negative edge of 0 at the beginning of the cycle RASTER >= $30 (48) and RASTER
    <= $f7 (247) and the lower three bits of RASTER are equal to YSCROLL and if the
    DEN bit was set during an arbitrary cycle of raster line $30."

    Update the display/idle state flag.

    ( see 3.7.1 in http://www.zimmers.net/cbmpics/cbm/c64/vic-ii.txt )

    "The transition from idle to display state occurs as soon as there is a Bad
    Line Condition (see section 3.5.)...

*/
static inline void _m6569_rs_update_badline(m6569_t* vic) {
    if (_M6569_RAST_RANGE(48, 247)) {
        /* DEN bit must have been set in raster line $30 */
        if (_M6569_RAST(48) && (vic->reg.ctrl_1 & M6569_CTRL1_DEN)) {
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
    if (vic->rs.badline) {
        vic->rs.display_state = true;
    }
}

/*
    Update the display/idle state flag.

    ( see 3.7.1 in http://www.zimmers.net/cbmpics/cbm/c64/vic-ii.txt )

    "The transition from idle to display state occurs as soon as there is a Bad
    Line Condition (see section 3.5.) (see _m6569_rs_update_badline())

    ...The transition from display to idle
    state occurs in cycle 58 of a line if the RC (see next section) contains
    the value 7 and there is no Bad Line Condition."
    "...In the first phase of cycle 58, the VIC checks if RC=7. If so, the video
    logic goes to idle state and VCBASE is loaded from VC (VC->VCBASE). If
    the video logic is in display state afterwards (this is always the case
    if there is a Bad Line Condition), RC is incremented.""
*/
static inline void _m6569_rs_update_display_state(m6569_t* vic) {
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

/* In the first phase of cycle 14 of each line, VC is loaded from VCBASE
    (VCBASE->VC) and VMLI is cleared. If there is a Bad Line Condition in
    this phase, RC is also reset to zero.
*/
static inline void _m6569_rs_rewind_vc_vmli_rc(m6569_t* vic) {
    vic->rs.vc = vic->rs.vc_base;
    vic->vm.vmli = 0;
    vic->vm.next_vmli = 0;
    if (vic->rs.badline) {
        vic->rs.rc = 0;
    }
}

static inline void _m6569_crt_next_crtline(m6569_t* vic) {
    vic->crt.x = 0;
    if (vic->rs.v_count == _M6569_VRETRACEPOS) {
        vic->crt.y = 0;
    }
    else {
        vic->crt.y++;
    }
}

/* border unit functions */
static inline void _m6569_bunit_left(m6569_t* vic, uint32_t hpos) {
    if (hpos == vic->brd.left) {
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
        else if ((vic->rs.v_count == vic->brd.top) && (vic->reg.ctrl_1 & M6569_CTRL1_DEN)) {
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

static inline void _m6569_bunit_right(m6569_t* vic, uint32_t hpos) {
    if (hpos == vic->brd.right) {
        vic->brd.main = true;
    }
}

static inline void _m6569_bunit_end(m6569_t* vic) {
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
    else if ((vic->rs.v_count == vic->brd.top) && (vic->reg.ctrl_1 & M6569_CTRL1_DEN)) {
        vic->brd.vert = false;
    }
}

/* memory access functions */
static inline void _m6569_c_access(m6569_t* vic) {
    if (vic->rs.badline) {
        /* addr=|VM13|VM12|VM11|VM10|VC9|VC8|VC7|VC6|VC5|VC4|VC3|VC2|VC1|VC0| */
        uint16_t addr = vic->rs.vc | vic->mem.c_addr_or;
        vic->vm.line[vic->vm.vmli] = vic->mem.fetch_cb(addr, vic->mem.user_data) & 0x0FFF;
    }
}

static inline uint8_t _m6569_i_access(m6569_t* vic) {
    return (uint8_t) vic->mem.fetch_cb(vic->mem.i_addr, vic->mem.user_data);
}

static inline uint8_t _m6569_g_i_access(m6569_t* vic) {
    /* perform a g-access or i access */
    if (vic->rs.display_state) {
        uint16_t addr;
        if (vic->reg.ctrl_1 & M6569_CTRL1_BMM) {
            /* bitmap mode: addr=|CB13|VC9|VC8|VC7|VC6|VC5|VC4|VC3|VC2|VC1|VC0|RC2|RC1|RC0| */
            addr = vic->rs.vc<<3 | vic->rs.rc;
            addr = (addr | (vic->mem.g_addr_or & (1<<13))) & vic->mem.g_addr_and;
        }
        else {
            /* text mode: addr=|CB13|CB12|CB11|D7|D6|D5|D4|D3|D2|D1|D0|RC2|RC1|RC0| */
            addr = ((vic->vm.line[vic->vm.vmli]&0xFF)<<3) | vic->rs.rc;
            addr = (addr | vic->mem.g_addr_or) & vic->mem.g_addr_and;
        }
        vic->rs.vc = (vic->rs.vc + 1) & 0x3FF;          /* VC is a 10-bit counter */
        vic->vm.next_vmli = (vic->vm.vmli + 1) & 0x3F;  /* VMLI is a 6-bit counter */
        return (uint8_t) vic->mem.fetch_cb(addr, vic->mem.user_data);
    }
    else {
        return _m6569_i_access(vic);
    }
}

static inline void _m6569_p_access(m6569_t* vic, uint32_t p_index) {
    uint16_t addr = vic->mem.p_addr_or + p_index;
    vic->sunit.p_data[p_index] = (uint8_t) vic->mem.fetch_cb(addr, vic->mem.user_data);
}

static inline void _m6569_s_access(m6569_t* vic, uint32_t s_index) {
    /* sprite s-access: |MP7|MP6|MP5|MP4|MP3|MP2|MP1|MP0|MC5|MC4|MC3|MC2|MC1|MC0| */
    m6569_sprite_unit_t* su = &vic->sunit;
    if (su->dma_enabled[s_index]) {
        uint16_t addr = (su->p_data[s_index]<<6) | su->mc[s_index];
        uint8_t s_data = (uint8_t) vic->mem.fetch_cb(addr, vic->mem.user_data);
        su->shift[s_index] = (su->shift[s_index]<<8) | (s_data<<8);
        su->mc[s_index] = (su->mc[s_index] + 1) & 0x3F;
    }
}

static inline uint8_t _m6569_s_i_access(m6569_t* vic, uint32_t s_index) {
    /* perform an s-access if dma is enabled, otherwise an i-access */
    m6569_sprite_unit_t* su = &vic->sunit;
    if (su->dma_enabled[s_index]) {
        uint16_t addr = (su->p_data[s_index]<<6) | su->mc[s_index];
        uint8_t s_data = (uint8_t) vic->mem.fetch_cb(addr, vic->mem.user_data);
        su->shift[s_index] = (su->shift[s_index]<<8) | (s_data<<8);
        su->mc[s_index] = (su->mc[s_index] + 1) & 0x3F;
        return 0;
    }
    else {
        return _m6569_i_access(vic);
    }
}

/* set BA pin if badline flag is set */
static inline uint64_t _m6569_ba(m6569_t* vic, uint64_t pins) {
    if (vic->rs.badline) {
        pins |= M6569_BA;
    }
    return pins;
}

/* set AEC pin */
static inline uint64_t _m6569_aec(uint64_t pins) {
    return pins | M6569_AEC;
}

/* internal tick function */
static uint64_t _m6569_tick(m6569_t* vic, uint64_t pins) {
    pins &= ~M6569_BA;
    uint8_t g_data = 0x00;
    _m6569_rs_update_badline(vic);

    /* a raster line is 63 ticks, and each line goes through a fixed 'program' */
    vic->rs.h_count++;
    vic->crt.x++;
    switch (vic->rs.h_count) {
        case 1:
            _m6569_p_access(vic, 3);
            _m6569_s_access(vic, 3);
            pins = _m6569_sunit_dma_aec(vic, 3, pins);
            pins = _m6569_sunit_dma_ba(vic, 3, pins);
            pins = _m6569_sunit_dma_ba(vic, 4, pins);
            break;
        case 2:
            g_data = _m6569_s_i_access(vic, 3);
            _m6569_s_access(vic, 3);
            pins = _m6569_sunit_dma_aec(vic, 3, pins);
            pins = _m6569_sunit_dma_ba(vic, 3, pins);
            pins = _m6569_sunit_dma_ba(vic, 4, pins);
            pins = _m6569_sunit_dma_ba(vic, 5, pins);
            break;
        case 3:
            _m6569_p_access(vic, 4);
            _m6569_s_access(vic, 4);
            pins = _m6569_sunit_dma_aec(vic, 4, pins);
            pins = _m6569_sunit_dma_ba(vic, 4, pins);
            pins = _m6569_sunit_dma_ba(vic, 5, pins);
            break;
        case 4:
            _m6569_crt_next_crtline(vic);
            g_data = _m6569_s_i_access(vic, 4);
            _m6569_s_access(vic, 4);
            pins = _m6569_sunit_dma_aec(vic, 4, pins);
            pins = _m6569_sunit_dma_ba(vic, 4, pins);
            pins = _m6569_sunit_dma_ba(vic, 5, pins);
            pins = _m6569_sunit_dma_ba(vic, 6, pins);
            break;
        case 5:
            _m6569_p_access(vic, 5);
            _m6569_s_access(vic, 5);
            pins = _m6569_sunit_dma_aec(vic, 5, pins);
            pins = _m6569_sunit_dma_ba(vic, 5, pins);
            pins = _m6569_sunit_dma_ba(vic, 6, pins);
            break;
        case 6:
            g_data = _m6569_s_i_access(vic, 5);
            _m6569_s_access(vic, 5);
            pins = _m6569_sunit_dma_aec(vic, 5, pins);
            pins = _m6569_sunit_dma_ba(vic, 5, pins);
            pins = _m6569_sunit_dma_ba(vic, 6, pins);
            pins = _m6569_sunit_dma_ba(vic, 7, pins);
            break;
        case 7:
            _m6569_p_access(vic, 6);
            _m6569_s_access(vic, 6);
            pins = _m6569_sunit_dma_aec(vic, 6, pins);
            pins = _m6569_sunit_dma_ba(vic, 6, pins);
            pins = _m6569_sunit_dma_ba(vic, 7, pins);
            break;
        case 8:
            g_data = _m6569_s_i_access(vic, 6);
            _m6569_s_access(vic, 6);
            pins = _m6569_sunit_dma_aec(vic, 6, pins);
            pins = _m6569_sunit_dma_ba(vic, 6, pins);
            pins = _m6569_sunit_dma_ba(vic, 7, pins);
            break;
        case 9:
            _m6569_p_access(vic, 7);
            _m6569_s_access(vic, 7);
            pins = _m6569_sunit_dma_aec(vic, 7, pins);
            pins = _m6569_sunit_dma_ba(vic, 7, pins);
            break;
        case 10:
            g_data = _m6569_s_i_access(vic, 7);
            _m6569_s_access(vic, 7);
            pins = _m6569_sunit_dma_aec(vic, 7, pins);
            pins = _m6569_sunit_dma_ba(vic, 7, pins);
            break;
        case 11:
            break;
        case 12:
        case 13:
        case 14:
            pins = _m6569_ba(vic, pins);
            break;
        case 15:
            pins = _m6569_ba(vic, pins);
            pins = _m6569_aec(pins);
            _m6569_rs_rewind_vc_vmli_rc(vic);
            break;
        case 16:
            pins = _m6569_ba(vic, pins);
            pins = _m6569_aec(pins);
            vic->gunit.enabled = vic->rs.display_state;
            _m6569_gunit_rewind(vic, vic->reg.ctrl_2 & M6569_CTRL2_XSCROLL);
            _m6569_sunit_update_mcbase(vic);
            _m6569_c_access(vic);
            g_data = _m6569_g_i_access(vic);
            _m6569_bunit_left(vic, 16);
            break;
        case 17:
            pins = _m6569_ba(vic, pins);
            pins = _m6569_aec(pins);
            vic->gunit.enabled = vic->rs.display_state;
            _m6569_sunit_dma_disp_disable(vic);
            _m6569_c_access(vic);
            g_data = _m6569_g_i_access(vic);
            _m6569_bunit_left(vic, 17);
            break;
        case 18: case 19:
        case 20: case 21: case 22: case 23: case 24: case 25: case 26: case 27: case 28: case 29:
        case 30: case 31: case 32: case 33: case 34: case 35: case 36: case 37: case 38: case 39:
        case 40: case 41: case 42: case 43: case 44: case 45: case 46: case 47: case 48: case 49:
        case 50: case 51: case 52: case 53: case 54:
            pins = _m6569_ba(vic, pins);
            pins = _m6569_aec(pins);
            vic->gunit.enabled = vic->rs.display_state;
            _m6569_c_access(vic);
            g_data = _m6569_g_i_access(vic);
            break;
        case 55:
            pins = _m6569_aec(pins);
            vic->gunit.enabled = vic->rs.display_state;
            _m6569_c_access(vic);
            g_data = _m6569_g_i_access(vic);
            pins = _m6569_sunit_dma_ba(vic, 0, pins);
            _m6569_bunit_right(vic, 55);
            break;
        case 56:
            vic->gunit.enabled = false;
            _m6569_sunit_start(vic);
            g_data = _m6569_i_access(vic);
            pins = _m6569_sunit_dma_ba(vic, 0, pins);
            _m6569_bunit_right(vic, 56);
            break;
        case 57:
            g_data = _m6569_i_access(vic);
            pins = _m6569_sunit_dma_ba(vic, 0, pins);
            pins = _m6569_sunit_dma_ba(vic, 1, pins);
            break;
        case 58:
            _m6569_sunit_update_mc_disp_enable(vic);
            _m6569_p_access(vic, 0);
            _m6569_s_access(vic, 0);
            pins = _m6569_sunit_dma_aec(vic, 0, pins);
            pins = _m6569_sunit_dma_ba(vic, 0, pins);
            pins = _m6569_sunit_dma_ba(vic, 1, pins);
            break;
        case 59:
            _m6569_rs_update_display_state(vic);
            g_data = _m6569_s_i_access(vic, 0);
            _m6569_s_access(vic, 0);
            pins = _m6569_sunit_dma_aec(vic, 0, pins);
            pins = _m6569_sunit_dma_ba(vic, 0, pins);
            pins = _m6569_sunit_dma_ba(vic, 1, pins);
            pins = _m6569_sunit_dma_ba(vic, 2, pins);
            break;
        case 60:
            _m6569_p_access(vic, 1);
            _m6569_s_access(vic, 1);
            pins = _m6569_sunit_dma_aec(vic, 1, pins);
            pins = _m6569_sunit_dma_ba(vic, 1, pins);
            pins = _m6569_sunit_dma_ba(vic, 2, pins);
            break;
        case 61:
            g_data = _m6569_s_i_access(vic, 1);
            _m6569_s_access(vic, 1);
            pins = _m6569_sunit_dma_aec(vic, 1, pins);
            pins = _m6569_sunit_dma_ba(vic, 1, pins);
            pins = _m6569_sunit_dma_ba(vic, 2, pins);
            pins = _m6569_sunit_dma_ba(vic, 3, pins);
            break;
        case 62:
            _m6569_p_access(vic, 2);
            _m6569_s_access(vic, 2);
            pins = _m6569_sunit_dma_aec(vic, 2, pins);
            pins = _m6569_sunit_dma_ba(vic, 2, pins);
            pins = _m6569_sunit_dma_ba(vic, 3, pins);
            break;
        case 63:    /* HTOTAL */
            _m6569_rs_next_rasterline(vic);
            _m6569_rs_check_irq(vic);
            g_data = _m6569_s_i_access(vic, 2);
            _m6569_s_access(vic, 2);
            pins = _m6569_sunit_dma_aec(vic, 2, pins);
            pins = _m6569_sunit_dma_ba(vic, 2, pins);
            pins = _m6569_sunit_dma_ba(vic, 3, pins);
            pins = _m6569_sunit_dma_ba(vic, 4, pins);
            _m6569_bunit_end(vic);
            break;
    }
    /*-- main interrupt bit --*/
    if (vic->reg.int_latch & vic->reg.int_mask & 0x0F) {
        vic->reg.int_latch |= M6569_INT_IRQ;
    }
    else {
        vic->reg.int_latch &= ~M6569_INT_IRQ;
    }
    if (vic->reg.int_latch & M6569_INT_IRQ) {
        pins |= M6569_IRQ;
    }

    /*--- decode pixels into framebuffer -------------------------------------*/
    if (vic->crt.rgba8_buffer) {
        int x, y, w;
        if (vic->debug_vis) {
            x = vic->rs.h_count;
            y = vic->rs.v_count;
            w = _M6569_HTOTAL;
            uint32_t* dst = vic->crt.rgba8_buffer + (y * w + x) * 8;
            _m6569_decode_pixels_debug(vic, g_data, 0 != (pins & M6569_BA), dst, vic->rs.h_count);
        }
        else if ((vic->crt.x >= vic->crt.vis_x0) && (vic->crt.x < vic->crt.vis_x1) &&
                 (vic->crt.y >= vic->crt.vis_y0) && (vic->crt.y < vic->crt.vis_y1))
        {
            const int x = vic->crt.x - vic->crt.vis_x0;
            const int y = vic->crt.y - vic->crt.vis_y0;
            const int w = vic->crt.vis_w;
            uint32_t* dst = vic->crt.rgba8_buffer + (y * w + x) * 8;
            _m6569_decode_pixels(vic, g_data, dst, vic->rs.h_count);
        }
    }
    vic->vm.vmli = vic->vm.next_vmli;
    return pins;
}

/* all-in-one tick function */
uint64_t m6569_tick(m6569_t* vic, uint64_t pins) {
    /* per-tick actions */
    pins = _m6569_tick(vic, pins);

    /* register read/writes */
    if (pins & M6569_CS) {
        if (pins & M6569_RW) {
            pins = _m6569_read(vic, pins);
        }
        else {
            _m6569_write(vic, pins);
        }
    }
    vic->pins = pins;
    return pins;
}

int m6569_display_width(m6569_t* vic) {
    CHIPS_ASSERT(vic);
    return 8 * (vic->debug_vis ? _M6569_HTOTAL : vic->crt.vis_w);
}

int m6569_display_height(m6569_t* vic) {
    CHIPS_ASSERT(vic);
    return vic->debug_vis ? _M6569_VTOTAL : vic->crt.vis_h;
}

uint32_t m6569_color(int i) {
    CHIPS_ASSERT((i >= 0) && (i < 16));
    return _m6569_colors[i];
}
#endif /* CHIPS_IMPL */
