#pragma once
/*#
    # mc6845.h

    Header-only emulator for the Motorola MC6845 CRT controller.

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
#define MC6845_MA0  (1ULL<<0)
#define MC6845_MA1  (1ULL<<1)
#define MC6845_MA2  (1ULL<<2)
#define MC6845_MA3  (1ULL<<3)
#define MC6845_MA4  (1ULL<<4)
#define MC6845_MA5  (1ULL<<5)
#define MC6845_MA6  (1ULL<<6)
#define MC6845_MA7  (1ULL<<7)
#define MC6845_MA8  (1ULL<<8)
#define MC6845_MA9  (1ULL<<9)
#define MC6845_MA10 (1ULL<<10)
#define MC6845_MA11 (1ULL<<11)
#define MC6845_MA12 (1ULL<<12)
#define MC6845_MA13 (1ULL<<13)

/* data bus pins */
#define MC6845_D0   (1ULL<<16)
#define MC6845_D1   (1ULL<<17)
#define MC6845_D2   (1ULL<<18)
#define MC6845_D3   (1ULL<<19)
#define MC6845_D4   (1ULL<<20)
#define MC6845_D5   (1ULL<<21)
#define MC6845_D6   (1ULL<<22)
#define MC6845_D7   (1ULL<<23)

/* control pins */
#define MC6845_CS       (1ULL<<40)     /* chip select */
#define MC6845_RS       (1ULL<<41)     /* register select (active: data register, inactive: address register) */
#define MC6845_RW       (1ULL<<42)     /* read/write (active: write, inactive: read) */
#define MC6845_LPSTB    (1ULL<<43)     /* light pen strobe */

/* display status pins */
#define MC6845_DE       (1ULL<<44)     /* display enable */
#define MC6845_VS       (1ULL<<45)     /* vsync active */
#define MC6845_HS       (1ULL<<46)     /* hsync active */

/* row-address output pins */
#define MC6845_RA0      (1ULL<<48)
#define MC6845_RA1      (1ULL<<49)
#define MC6845_RA2      (1ULL<<50)
#define MC6845_RA3      (1ULL<<51)
#define MC6845_RA4      (1ULL<<52)

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
#define MC6845_MAXSCALINEADDR   (9)
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
    MC6845_TYPE_UM6845 = 0,
    MC6845_TYPE_UM6845R,    
    MC6845_TYPE_MC6845,
    MC6845_NUM_TYPES,
} mc6845_type_t;

/* mc6845 state */
typedef struct {
    mc6845_type_t type;                       
    /* currently selected register */
    uint8_t sel;
    /* register bank */
    union {
        uint8_t reg[0x1F];              /* only 17 registers exist */
        struct {
            uint8_t h_total;            /* horizontal total (minus 1) */
            uint8_t h_displayed;        /* horizontal displayed */
            uint8_t h_sync_pos;         /* horizontal sync pos */
            uint8_t sync_widths;        /* horizontal, and optionally vertical sync widths */
            uint8_t v_total;            /* vertical total (minus 1) */
            uint8_t v_total_adjust;     /* end-of-frame scanline adjust value */
            uint8_t v_displayed;        /* vertical displayed */
            uint8_t v_sync_pos;          /* vertical sync pos */
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
    /* counters and latches */
    uint16_t ma;                    /* the memory address */
    uint16_t ma_row_start;          /* memory address reset latch */
    uint8_t h_ctr;                  /* horizontal counter (mod 256) */
    uint8_t hsync_ctr;              /* horizontal sync-width counter (mod 16) */
    uint8_t vsync_ctr;              /* vertical sync-height counter */
    uint8_t row_ctr;                /* character row counter (mod 128) */
    uint8_t scanline_ctr;           /* scanline counter (mod 32) */
    bool in_adj;                    /* true if inside scanline adjust range */
    bool hs;                        /* current HSYNC state */
    bool vs;                        /* current VSYNC state */
    bool h_de;                      /* horizontal display enable */
    bool v_de;                      /* vertical display enable */
    uint64_t pins;                  /* pin state after last tick */
} mc6845_t;

/* helper macros to extract address and data values from pin mask */

/* extract 13-bit address bus from 64-bit pins */
#define MC6845_GET_ADDR(p) ((uint16_t)(p&0xFFFFULL))
/* merge 13-bit address bus value into 64-bit pins */
#define MC6845_SET_ADDR(p,a) {p=((p&~0xFFFFULL)|((a)&0xFFFFULL));}
/* extract 8-bit data bus from 64-bit pins */
#define MC6845_GET_DATA(p) ((uint8_t)((p&0xFF0000ULL)>>16))
/* merge 8-bit data bus value into 64-bit pins */
#define MC6845_SET_DATA(p,d) {p=((p&~0xFF0000ULL)|(((d)<<16)&0xFF0000ULL));}
/* get 5-bit row-address from 64-bit pins */
#define MC6845_GET_RA(p) ((uint8_t)((p&0x00FF000000000000ULL)>>48))
/* merge 5-bit row-address into 64-bit pins */
#define MC6845_SET_RA(p,a) {p=((p&~0x00FF000000000000ULL)|(((a<<48)&)0x00FF000000000000ULL));}

/* initialize a new mc6845 instance */
extern void mc6845_init(mc6845_t* mc6845, mc6845_type_t type);
/* reset an existing mc6845 instance */
extern void mc6845_reset(mc6845_t* mc6845);
/* perform an IO request */
extern uint64_t mc6845_iorq(mc6845_t* mc6845, uint64_t pins);
/* tick the mc6845, the returned pin mask overwrittes addr bus pins with MA0..MA13! */
extern uint64_t mc6845_tick(mc6845_t* mc6845);

#ifdef __cplusplus
} /* extern "C" */
#endif

/*-- IMPLEMENTATION ----------------------------------------------------------*/
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
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* readable/writable per chip type and register (1: writable, 2: readable, 3: read/write) */
static uint8_t _mc6845_rw[MC6845_NUM_TYPES][0x20] = {
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 3, 3, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

void mc6845_init(mc6845_t* c, mc6845_type_t type) {
    CHIPS_ASSERT(c);
    memset(c, 0, sizeof(mc6845_t));
    c->type = type;
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
    c->row_ctr = 0;
    c->scanline_ctr = 0;
    c->in_adj = false;
    c->hs = false;
    c->vs = false;
    c->h_de = false;
    c->v_de = false;
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
                }
            }
        }
        else {
            /* register address selected */
            if (pins & MC6845_RW) {
                /* on UM6845R, read status register
                   bit 6: LPEN (not implemented)
                   bit 5: currently in vertical blanking
                */
                uint8_t val = c->v_de ? 0: (1<<5);
                MC6845_SET_DATA(pins, val);
            }
            else {
                /* write to address register */
                c->sel = MC6845_GET_DATA(pins) & 0x1F;
            }
        }
    }
    return pins;
}

uint64_t mc6845_tick(mc6845_t* c) {

    /* handle horizontal counter */
    if (c->h_ctr == c->h_total) {
        c->h_ctr = 0;           /* reset horizontal counter */
    }
    else {
        c->h_ctr++;     /* no masking needed since h_ctr is 8 bits */
        c->ma = (c->ma + 1) & 0x3FFF;
    }

    /* handle horizontal display-enabled */
    if (c->h_ctr == 0) {
        c->h_de = true;
    }
    if (c->h_ctr == c->h_displayed) {
        c->h_de = false;    /* end of horizontal display-enable range */
    }

    /* handle HSYNC on/off */
    if (c->h_ctr == c->h_sync_pos) {
        c->hs = true;       /* start of HSYNC range */
        c->hsync_ctr = 0;
    }
    else if (c->hs) {
        c->hsync_ctr = (c->hsync_ctr + 1) & 0xF;
        /* FIXME: on UM6845 and UM6845R, if 0 is programmed, no HSYNC is generated
           (see: http://www.cpcwiki.eu/index.php/CRTC#CRTC_Differences)

           since hsync_ctr will wrap-around at 16, a h_sync_width of 0
           will actually be treated as h_sync_width = 16
        */
        uint8_t h_sync_width = c->sync_widths & 0xF;
        if (c->hsync_ctr == h_sync_width) {
            c->hs = false;
        }
    }

    /* NEW SCANLINE? */
    /* FIXME: on UM6845R, R12/R13 (start_addr_hi/lo) is read into
       ma_row_start at the start of each scanline during character
       row 0.
    */
    if (c->h_ctr == 0) {
        bool need_adj = c->v_total_adjust != 0;
        uint8_t max_scan_line;
        if (c->in_adj) {
            /* this is an adjust row */
            max_scan_line = (c->v_total_adjust - 1) & 0x1F;
        }
        else {
            /* this is a normal row */
            max_scan_line = c->max_scanline_addr;
        }
        if ((c->scanline_ctr == max_scan_line) && ((!need_adj && (c->row_ctr == c->v_total)) || c->in_adj)) {
            /* new frame */
            c->scanline_ctr = 0;
            c->ma_row_start = (c->start_addr_hi<<8)|(c->start_addr_lo);
            c->row_ctr = 0;
            c->in_adj = false;
        }
        else if (!c->in_adj && (c->scanline_ctr == max_scan_line)) {
            /* new character row */
            c->scanline_ctr = 0;
            c->ma_row_start += c->h_displayed;
            c->row_ctr = (c->row_ctr + 1) & 0x7F;
            if ((c->row_ctr == c->v_total) && need_adj) {
                c->in_adj = true;
            }
        }
        else {
            /* new scanline */
            c->scanline_ctr = (c->scanline_ctr + 1) & 0x1F;
        }

        /* reload the memory address counter per scanline */
        c->ma = c->ma_row_start;

        /* handle vertical display enabled */
        if (c->row_ctr == 0) {
            c->v_de = true;
        }
        if (c->row_ctr == c->v_displayed) {
            c->v_de = false;
        }

        /* handle VSYNC on/off */
        if ((c->row_ctr == c->v_sync_pos) || c->vs) {
            c->vs = true;
            c->vsync_ctr = (c->vsync_ctr + 1) & 0x0F;
        }
        else {
            c->vsync_ctr = 0;
        }
        uint8_t v_sync_width;
        if ((c->type == MC6845_TYPE_UM6845R)||(c->type == MC6845_TYPE_MC6845)) {
            /* on these models, VSYNC width is fixed to 0 (== 16 lines) */
            v_sync_width = 0;
        }
        else {
            v_sync_width = (c->sync_widths >> 4) & 0x0F;
        }
        if ((c->vsync_ctr == v_sync_width) && c->vs) {
            c->vs = false;
        }
    }

    /* build the return pin mask */
    uint64_t new_pins = ((c->scanline_ctr & 0x1F) * MC6845_RA0)| c->ma;
    if (c->hs) {
        new_pins |= MC6845_HS;
    }
    if (c->vs) {
        new_pins |= MC6845_VS;
    }
    if (c->h_de && c->v_de) {
        new_pins |= MC6845_DE;
    }
    c->pins = new_pins;
    return new_pins;
}

#endif /* CHIPS_IMPL */
