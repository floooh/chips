#pragma once
/*#
    # mc6845.h

    Header-only emulator for the Motorola MC6845 CRT controller.

    FIXME: the whole HSYNC/VSYNC generation needs work for
    "edge situations"...

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

    ## Emulated Pins
    **********************************
    *           +----------+         *
    *           |          |         *
    *     CS -->|          |--> MA0  *
    *     RS -->|          |...      *
    *     RW -->|          |--> MA13 *
    *           |          |         *
    *     DE <--|          |--> RA0  *
    *     VS <--|  MC6845  |...      *
    *     HS <--|          |--> RA4  *
    * CURSOR <--|          |         *
    *           |          |--> D0   *
    *  LPSTB -->|          |...      *
    *  RESET -->|          |--> D7   *
    *           |          |         *
    *           +----------+         *
    **********************************

    * CS:   chips select (1: chip is active)
    * RS:   register select (0: select address register, 1: select register files)
    * RW:   read/write select (0: write, 1: read)

    ## Not Emulated:

    * the E pin (enable)
    * the RESET pin, call mc6845_reset() instead
    * interlace stuff
    * the CURSOR pin
    * the light pen stuff

    ## Datasheet Notes

    * all the important information on internal counters can be gathered
      from the "FIGURE 11 - CRTC BLOCK DIAGRAM" image in the data sheet,
      and the timing diagrams "FIGURE 13 - CRTC HORIZONTAL TIMING" and
      "FIGURE 14 - CRTC VERTICAL TIMING"
    * all the TOTAL register values (HTOTAL, VTOTAL, MAX_SCANLINE_ADDR)
      which cause counters to restart are '+1' (so the wrap around happens
      at the value stored in the 'total' register plus one), this is
      illustrated in "FIGURE 12 - ILLUSTRATION OF THE SCREEN FORMAT"

    ## Links

    - http://www.cpcwiki.eu/index.php/CRTC
    - http://www.6502.org/users/andre/hwinfo/crtc/crtc.html
    - http://www.6502.org/users/andre/hwinfo/crtc/internals/index.html
    - http://bitsavers.informatik.uni-stuttgart.de/components/motorola/_dataSheets/6845.pdf
    - https://github.com/hoglet67/BeebFpga/blob/master/src/common/mc6845.vhd

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

/* the address output pins share the same pin locations as the system address bus
   but they are only set in the output pins mask of mc6845_tick()
*/
#define MC6845_PIN_MA0  (0)
#define MC6845_PIN_MA1  (1)
#define MC6845_PIN_MA2  (2)
#define MC6845_PIN_MA3  (3)
#define MC6845_PIN_MA4  (4)
#define MC6845_PIN_MA5  (5)
#define MC6845_PIN_MA6  (6)
#define MC6845_PIN_MA7  (7)
#define MC6845_PIN_MA8  (8)
#define MC6845_PIN_MA9  (9)
#define MC6845_PIN_MA10 (10)
#define MC6845_PIN_MA11 (11)
#define MC6845_PIN_MA12 (12)
#define MC6845_PIN_MA13 (13)

// data bus pins
#define MC6845_PIN_D0   (16)
#define MC6845_PIN_D1   (17)
#define MC6845_PIN_D2   (18)
#define MC6845_PIN_D3   (19)
#define MC6845_PIN_D4   (20)
#define MC6845_PIN_D5   (21)
#define MC6845_PIN_D6   (22)
#define MC6845_PIN_D7   (23)

// control pins
#define MC6845_PIN_CS       (40)     // chip select
#define MC6845_PIN_RS       (41)     // register select (active: data register, inactive: address register)
#define MC6845_PIN_RW       (42)     // read/write (active: write, inactive: read)
#define MC6845_PIN_LPSTB    (43)     // light pen strobe

// display status pins
#define MC6845_PIN_DE       (44)     // display enable
#define MC6845_PIN_VS       (45)     // vsync active
#define MC6845_PIN_HS       (46)     // hsync active

// row-address output pins
#define MC6845_PIN_RA0      (48)
#define MC6845_PIN_RA1      (49)
#define MC6845_PIN_RA2      (50)
#define MC6845_PIN_RA3      (51)
#define MC6845_PIN_RA4      (52)

// pin bit masks
#define MC6845_MA0  (1ULL<<MC6845_PIN_MA0)
#define MC6845_MA1  (1ULL<<MC6845_PIN_MA1)
#define MC6845_MA2  (1ULL<<MC6845_PIN_MA2)
#define MC6845_MA3  (1ULL<<MC6845_PIN_MA3)
#define MC6845_MA4  (1ULL<<MC6845_PIN_MA4)
#define MC6845_MA5  (1ULL<<MC6845_PIN_MA5)
#define MC6845_MA6  (1ULL<<MC6845_PIN_MA6)
#define MC6845_MA7  (1ULL<<MC6845_PIN_MA7)
#define MC6845_MA8  (1ULL<<MC6845_PIN_MA8)
#define MC6845_MA9  (1ULL<<MC6845_PIN_MA9)
#define MC6845_MA10 (1ULL<<MC6845_PIN_MA10)
#define MC6845_MA11 (1ULL<<MC6845_PIN_MA11)
#define MC6845_MA12 (1ULL<<MC6845_PIN_MA12)
#define MC6845_MA13 (1ULL<<MC6845_PIN_MA13)
#define MC6845_D0   (1ULL<<MC6845_PIN_D0)
#define MC6845_D1   (1ULL<<MC6845_PIN_D1)
#define MC6845_D2   (1ULL<<MC6845_PIN_D2)
#define MC6845_D3   (1ULL<<MC6845_PIN_D3)
#define MC6845_D4   (1ULL<<MC6845_PIN_D4)
#define MC6845_D5   (1ULL<<MC6845_PIN_D5)
#define MC6845_D6   (1ULL<<MC6845_PIN_D6)
#define MC6845_D7   (1ULL<<MC6845_PIN_D7)
#define MC6845_CS       (1ULL<<MC6845_PIN_CS)
#define MC6845_RS       (1ULL<<MC6845_PIN_RS)
#define MC6845_RW       (1ULL<<MC6845_PIN_RW)
#define MC6845_LPSTB    (1ULL<<MC6845_PIN_LPSTB)
#define MC6845_DE       (1ULL<<MC6845_PIN_DE)
#define MC6845_VS       (1ULL<<MC6845_PIN_VS)
#define MC6845_HS       (1ULL<<MC6845_PIN_HS)
#define MC6845_RA0      (1ULL<<MC6845_PIN_RA0)
#define MC6845_RA1      (1ULL<<MC6845_PIN_RA1)
#define MC6845_RA2      (1ULL<<MC6845_PIN_RA2)
#define MC6845_RA3      (1ULL<<MC6845_PIN_RA3)
#define MC6845_RA4      (1ULL<<MC6845_PIN_RA4)

/* I/O request pin mask (control and databus) */
#define MC6845_IORQ_PINS (MC6845_D0|MC6845_D1|MC6845_D2|MC6845_D3|MC6845_D4|MC6845_D5|MC6845_D6|MC6845_D7|MC6845_CS|MC6845_RS|MC6845_RW|MC6845_LPSTB)

/* register names */
#define MC6845_HTOTAL           (0)
#define MC6845_HDISPLAYED       (1)
#define MC6845_HSYNCPOS         (2)
#define MC6845_SYNCWIDTHS       (3)
#define MC6845_VTOTAL           (4)
#define MC6845_VTOTALADJ        (5)
#define MC6845_VDISPLAYED       (6)
#define MC6845_VSYNCPOS         (7)
#define MC6845_INTERLACEMODE    (8)
#define MC6845_MAXSCANLINEADDR  (9)
#define MC6845_CURSORSTART      (10)
#define MC6845_CURSOREND        (11)
#define MC6845_STARTADDRHI      (12)
#define MC6845_STARTADDRLO      (13)
#define MC6845_CURSORHI         (14)
#define MC6845_CURSORLO         (15)
#define MC6845_LIGHTPENHI       (16)
#define MC6845_LIGHTPENLO       (17)

/* chip subtypes */
typedef enum {
    MC6845_TYPE_UM6845 = 0,     /* CRTC type 0 */
    MC6845_TYPE_UM6845R,        /* CRTC type 1 */
    MC6845_TYPE_MC6845,         /* CRTC type 2 */
    MC6845_NUM_TYPES,
} mc6845_type_t;

/* mc6845 state */
typedef struct {
    mc6845_type_t type;
    /* currently selected register */
    uint8_t sel;
    /* register bank */
    union {
        uint8_t reg[0x20];              /* only 17 registers exist */
        struct {
            uint8_t h_total;            /* horizontal total (minus 1) */
            uint8_t h_displayed;        /* horizontal displayed */
            uint8_t h_sync_pos;         /* horizontal sync pos */
            uint8_t sync_widths;        /* horizontal, and optionally vertical sync widths */
            uint8_t v_total;            /* vertical total (minus 1) */
            uint8_t v_total_adjust;     /* end-of-frame scanline adjust value */
            uint8_t v_displayed;        /* vertical displayed */
            uint8_t v_sync_pos;         /* vertical sync pos */
            uint8_t interlace_mode;     /* interlace and skew */
            uint8_t max_scanline_addr;  /* max scanline ctr value (minus 1) */
            uint8_t cursor_start;
            uint8_t cursor_end;
            uint8_t start_addr_hi;
            uint8_t start_addr_lo;
            uint8_t cursor_hi;
            uint8_t cursor_lo;
            uint8_t lightpen_hi;
            uint8_t lightpen_lo;
        };
    };
    /* coincidence circuit comparison flags */
    bool co_htotal;                 /* h_ctr == (h_total + 1) */
    bool co_hdisp;                  /* h_ctr == h_disp */
    bool co_hspos;                  /* h_ctr == h_sync_pos */
    bool co_hswidth;                /* hsync_ctr == h_sync_width */
    bool co_vtotal;                 /* v_ctr == (v_total + 1) */
    bool co_vdisp;                  /* v_ctr == v_disp */
    bool co_vspos;                  /* v_ctr == v_sync_pos */
    bool co_vswidth;                /* v_ctr == v_sync_width */
    bool co_raster;                 /* r_ctr == max_scanline_addr */

    /* internal counters */
    uint8_t h_ctr;                  /* 8-bit horizontal counter */
    uint8_t hsync_ctr;              /* 4-bit horizontal sync-width counter */
    uint16_t ma;                    /* the memory address */
    uint16_t ma_row_start;          /* memory address reset latch */
    uint16_t ma_store;              /* internal latch for storing intermediate MA values */
    uint8_t v_ctr;                  /* 7-bit vertical counter */
    uint8_t r_ctr;                  /* 5-bit raster-line counter */
    uint8_t vsync_ctr;              /* vertical sync counter */

    bool hs;                        /* current HSYNC state */
    bool vs;                        /* current VSYNC state */
    bool h_de;                      /* horizontal display enable */
    bool v_de;                      /* vertical display enable */
    uint64_t pins;                  /* pin state after last tick */
} mc6845_t;

/* helper macros to extract address and data values from pin mask */

/* extract 13-bit address bus from 64-bit pins */
#define MC6845_GET_ADDR(p) ((uint16_t)((p)&0xFFFFULL))
/* merge 13-bit address bus value into 64-bit pins */
#define MC6845_SET_ADDR(p,a) {p=(((p)&~0xFFFFULL)|((a)&0xFFFFULL));}
/* extract 8-bit data bus from 64-bit pins */
#define MC6845_GET_DATA(p) ((uint8_t)(((p)&0xFF0000ULL)>>16))
/* merge 8-bit data bus value into 64-bit pins */
#define MC6845_SET_DATA(p,d) {p=(((p)&~0xFF0000ULL)|(((d)<<16)&0xFF0000ULL));}
/* get 5-bit row-address from 64-bit pins */
#define MC6845_GET_RA(p) ((uint8_t)(((p)&0x00FF000000000000ULL)>>48))
/* merge 5-bit row-address into 64-bit pins */
#define MC6845_SET_RA(p,a) {p=(((p)&~0x00FF000000000000ULL)|((((a)<<48)&)0x00FF000000000000ULL));}

/* initialize a new mc6845 instance */
void mc6845_init(mc6845_t* mc6845, mc6845_type_t type);
/* reset an existing mc6845 instance */
void mc6845_reset(mc6845_t* mc6845);
/* perform an IO request */
uint64_t mc6845_iorq(mc6845_t* mc6845, uint64_t pins);
/* tick the mc6845, the returned pin mask overwrittes addr bus pins with MA0..MA13! */
uint64_t mc6845_tick(mc6845_t* mc6845);

#ifdef __cplusplus
} /* extern "C" */
#endif

/*-- IMPLEMENTATION ----------------------------------------------------------*/
#ifdef CHIPS_IMPL
#include <string.h>
#ifndef CHIPS_ASSERT
    #include <assert.h>
    #define CHIPS_ASSERT(c) assert(c)
#endif

/* some registers are not full width */
static uint8_t _mc6845_mask[0x20] = {
    0xFF,       /* HTOTAL */
    0xFF,       /* HDISPLAYED */
    0xFF,       /* HSYNCPOS */
    0xFF,       /* SYNCWIDTHS */
    0x7F,       /* VTOTAL */
    0x1F,       /* VTOTALADJUST */
    0x7F,       /* VDISPLAYED */
    0x7F,       /* VSYNCPOS */
    0xF3,       /* INTERLACEMODE */
    0x1F,       /* MAXSCANLINEADDR */
    0x7F,       /* CURSORSTART */
    0x1F,       /* CURSOREND */
    0x3F,       /* STARTADDRHI */
    0xFF,       /* STARTADDRLO */
    0x3F,       /* CURSORHI */
    0xFF,       /* CURSORLO */
    0x3F,       /* LIGHTPENHI */
    0xFF,       /* LIGHTPENLO */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xFF     /* register 31 reads as 0xFF on UM6845R (CPC CRTC type 1) */
};

/* readable/writable per chip type and register (1: writable, 2: readable, 3: read/write) */
static uint8_t _mc6845_rw[MC6845_NUM_TYPES][0x20] = {
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 3, 3, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

void mc6845_init(mc6845_t* c, mc6845_type_t type) {
    CHIPS_ASSERT(c);
    memset(c, 0, sizeof(mc6845_t));
    c->type = type;
    c->reg[0x1F] = 0xFF;    /* register 31 reads as 0xFF on UM6845R (type 1) */
}

/* clear the coincidence circuit flags */
static inline void _mc6845_co_clear(mc6845_t* c) {
    c->co_htotal = false;
    c->co_hdisp = false;
    c->co_hspos = false;
    c->co_hswidth = false;
    c->co_vtotal = false;
    c->co_vdisp = false;
    c->co_vspos = false;
    c->co_vswidth = false;
    c->co_raster = false;
}

/* compute pin state */
static inline uint64_t _mc6845_pins(mc6845_t* c) {
    uint64_t pins = ((c->r_ctr & 0x1F) * MC6845_RA0) | c->ma;
    if (c->hs) {
        pins |= MC6845_HS;
    }
    if (c->vs) {
        pins |= MC6845_VS;
    }
    /* disabling DE via R8 (Skew) only on Type 0 */
    if (c->type == MC6845_TYPE_UM6845) {
        if (c->h_de && c->v_de && ((c->interlace_mode & 0x30) != 0x30)) {
            pins |= MC6845_DE;
        }
    }
    else {
        if (c->h_de && c->v_de) {
            pins |= MC6845_DE;
        }
    }
    c->pins = (c->pins & MC6845_IORQ_PINS) | pins;
    return pins;
}

void mc6845_reset(mc6845_t* c) {
    /* reset behaviour:
        - all counters in the CRTC are cleared and the device stops
          the display operation
        - all the outputs are driven low
        - the control registers in the CRTC are not affected and
          remain unchanged
        - the CRTC resumes the display operation immediately after
          the release of RESET. DE is not active until after
          the first VS pulse occurs.
    */
    c->ma = 0;
    c->ma_row_start = 0;
    c->h_ctr = 0;
    c->hsync_ctr = 0;
    c->vsync_ctr = 0;
    c->v_ctr = 0;
    c->r_ctr = 0;
    c->hs = false;
    c->vs = false;
    c->h_de = false;
    c->v_de = false;
    _mc6845_co_clear(c);
}

/* coincidence circuits comparison */
static inline uint8_t _mc6845_hswidth(mc6845_t* c) {
    uint8_t hs_width = c->sync_widths & 0x0F;
    if (c->type == MC6845_TYPE_MC6845) {
        if (hs_width == 0) {
            hs_width = 0x10;
        }
    }
    return hs_width;
}

static inline uint8_t _mc6845_vswidth(mc6845_t* c) {
    uint8_t vs_width;
    if (c->type == MC6845_TYPE_UM6845) {
        vs_width = (c->sync_widths >> 4) & 0x0F;
        if (0 == vs_width) {
            vs_width = 0x10;
        }
    }
    else {
        vs_width = 0x10;
    }
    return vs_width;
}

static inline void _mc6845_co_cmp_htotal(mc6845_t* c) {
    if (c->h_ctr >= (c->h_total + 1)) {
        c->co_htotal = true;
    }
}

static inline void _mc6845_co_cmp_hdisp(mc6845_t* c) {
    if (c->h_ctr == c->h_displayed) {
        c->co_hdisp = true;
    }
}

static inline void _mc6845_co_cmp_hspos(mc6845_t* c) {
    if (c->h_ctr == c->h_sync_pos) {
        c->co_hspos = true;
    }
}

static inline void _mc6845_co_cmp_hswidth(mc6845_t* c) {
    if (c->hsync_ctr == _mc6845_hswidth(c)) {
        c->co_hswidth = true;
    }
}

static inline void _mc6845_co_cmp_vtotal(mc6845_t* c) {
    /* why >= ??? */
    if (c->v_ctr >= (c->v_total + 1)) {
        c->co_vtotal = true;
    }
}

static inline void _mc6845_co_cmp_vdisp(mc6845_t* c) {
    if (c->v_ctr == c->v_displayed) {
        c->co_vdisp = true;
    }
}

static inline void _mc6845_co_cmp_vspos(mc6845_t* c) {
    if (c->v_ctr == c->v_sync_pos) {
        c->co_vspos = true;
    }
}

static inline void _mc6845_co_cmp_hctr(mc6845_t* c) {
    _mc6845_co_cmp_htotal(c);
    _mc6845_co_cmp_hdisp(c);
    _mc6845_co_cmp_hspos(c);
}

static inline void _mc6845_co_cmp_vctr(mc6845_t* c) {
    _mc6845_co_cmp_vtotal(c);
    _mc6845_co_cmp_vdisp(c);
    _mc6845_co_cmp_vspos(c);
}

static inline void _mc6845_co_cmp_vswidth(mc6845_t* c) {
    if (c->vsync_ctr == _mc6845_vswidth(c)) {
        c->co_vswidth = true;
    }
}

static inline void _mc6845_co_cmp_raster(mc6845_t* c) {
    uint8_t max_scanline = c->max_scanline_addr;
    if (c->v_ctr == c->v_total) {
        max_scanline += c->v_total_adjust;
    }
    if (c->r_ctr >= (max_scanline + 1)) {
        c->co_raster = true;
    }
}

uint64_t mc6845_iorq(mc6845_t* c, uint64_t pins) {
    if (pins & MC6845_CS) {
        if (pins & MC6845_RS) {
            /* read/write register value */
            CHIPS_ASSERT(c->type < MC6845_NUM_TYPES);
            int i = c->sel & 0x1F;
            if (pins & MC6845_RW) {
                /* read register value (only if register is readable) */
                uint8_t val = 0;
                if (_mc6845_rw[c->type][i] & (1<<1)) {
                    val = c->reg[i] & _mc6845_mask[i];
                }
                MC6845_SET_DATA(pins, val);
            }
            else {
                /* write register value (only if register is writable) */
                if (_mc6845_rw[c->type][i] & (1<<0)) {
                    c->reg[i] = MC6845_GET_DATA(pins) & _mc6845_mask[i];
                    /* update the coincidence circuit state */
                    switch (i) {
                        case MC6845_HTOTAL:
                            _mc6845_co_cmp_htotal(c);
                            break;
                        case MC6845_HDISPLAYED:
                            _mc6845_co_cmp_hdisp(c);
                            break;
                        case MC6845_HSYNCPOS:
                            _mc6845_co_cmp_hspos(c);
                            break;
                        case MC6845_SYNCWIDTHS:
                            _mc6845_co_cmp_hswidth(c);
                            _mc6845_co_cmp_vswidth(c);
                            break;
                        case MC6845_VTOTAL:
                            _mc6845_co_cmp_vtotal(c);
                            break;
                        case MC6845_VTOTALADJ:
                            /* FIXME! */
                            break;
                        case MC6845_VDISPLAYED:
                            _mc6845_co_cmp_vdisp(c);
                            break;
                        case MC6845_VSYNCPOS:
                            _mc6845_co_cmp_vspos(c);
                            break;
                        case MC6845_INTERLACEMODE:
                            /* FIXME? */
                            break;
                        case MC6845_MAXSCANLINEADDR:
                            _mc6845_co_cmp_raster(c);
                            break;
                        case MC6845_CURSORSTART:
                        case MC6845_CURSOREND:
                        case MC6845_STARTADDRHI:
                        case MC6845_STARTADDRLO:
                        case MC6845_CURSORHI:
                        case MC6845_CURSORLO:
                        case MC6845_LIGHTPENHI:
                        case MC6845_LIGHTPENLO:
                            break;
                    }
                }
            }
        }
        else {
            /* register address selected */
            if (pins & MC6845_RW) {
                /* on UM6845R and UM6845 read status register
                   bit 6: LPEN (not implemented)
                   bit 5: currently in vertical blanking
                */
                uint8_t val = 0;
                if (c->type != MC6845_TYPE_MC6845) {
                    val |= c->v_de ? 0: (1<<5);
                }
                MC6845_SET_DATA(pins, val);
            }
            else {
                /* write to address register */
                c->sel = MC6845_GET_DATA(pins) & 0x1F;
            }
        }
        c->pins = (c->pins & ~MC6845_IORQ_PINS) | (pins & MC6845_IORQ_PINS);
    }
    return pins;
}

/* per scanline updates */
static inline void _mc6845_scanline(mc6845_t* c) {
    /* FIXME: vertical adjust! */
    c->r_ctr = (c->r_ctr + 1) & 0x1F;
    _mc6845_co_cmp_raster(c);
    if (c->co_raster) {
        c->co_raster = false;
        c->r_ctr = 0;
        _mc6845_co_cmp_raster(c);
        c->v_ctr = (c->v_ctr + 1) & 0x7F;
        _mc6845_co_cmp_vctr(c);
        /* if co_vdisp and co_vtotal happen at the same time, display must be enabled */
        if (c->co_vdisp) {
            c->co_vdisp = false;
            c->v_de = false;
        }
        if (c->co_vtotal) {
            c->co_vtotal = false;
            /* new frame */
            c->v_ctr = 0;
            _mc6845_co_cmp_vctr(c);
            c->v_de = true;
            c->ma_store = (c->start_addr_hi<<8) | c->start_addr_lo;
        }
        if (c->co_vspos) {
            c->co_vspos = false;
            c->vs = true;
            c->vsync_ctr = 0;
        }
        c->ma_row_start = c->ma_store;
    }
    /* special case TYPE 0, reload ma_row_start at each scanline of row 0 */
    if ((c->type == MC6845_TYPE_UM6845R) && (c->v_ctr == 0)) {
        c->ma_store = (c->start_addr_hi<<8) | c->start_addr_lo;
        c->ma_row_start = c->ma_store;
    }
    if (c->vs) {
        _mc6845_co_cmp_vswidth(c);
        c->vsync_ctr++;
        if (c->co_vswidth) {
            c->co_vswidth = false;
            c->vs = false;
        }
    }
}

uint64_t mc6845_tick(mc6845_t* c) {
    c->ma = (c->ma + 1) & 0x3FFF;
    c->h_ctr = c->h_ctr + 1;
    _mc6845_co_cmp_hctr(c);
    if (c->co_htotal) {
        c->co_htotal = false;
        _mc6845_scanline(c);
        c->h_de = true;         /* FIXME: skew control */
        c->h_ctr = 0;
        _mc6845_co_cmp_hctr(c);
        c->ma = c->ma_row_start;
    }
    if (c->co_hdisp) {
        c->co_hdisp = false;
        c->h_de = false;
        c->ma_store = c->ma;
    }
    if (c->co_hspos) {
        c->co_hspos = false;
        c->hs = true;
        c->hsync_ctr = 0;
    }
    if (c->hs) {
        _mc6845_co_cmp_hswidth(c);
        c->hsync_ctr++;
        if (c->co_hswidth) {
            c->co_hswidth = false;
            c->hs = false;
        }
    }
    return _mc6845_pins(c);
}

#endif /* CHIPS_IMPL */
