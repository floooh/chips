#pragma once
/*#
    # am40010.h

    Amstrad CPC combined 40010 gate array and PAL emulator.

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

    Include the following files before am40010.h:

        chips/chips_common.h

    ## Emulated Pins

    ************************************ *
    *            +-----------+           *
    *     M1 --->|           |---> SYNC  *
    *   MREQ --->|           |---> INT   *
    *   IORQ --->|           |---> READY *
    *     RD --->|           |           *
    *     WR --->|           |           *
    *            |           |           *
    *  HSYNC --->|           |           *
    *  VSYNC --->|           |           *
    * DISPEN --->|   40010   |           *
    *            |           |           *
    *    A13 --->|           |           *
    *    A14 --->|           |           *
    *    A15 --->|           |           *
    *            |           |           *
    *     D0 --->|           |           *
    *     .. --->|           |           *
    *     D7 --->|           |           *
    *            +-----------+           *
    **************************************

    ## Notes
    (TODO)

    ## Links

    TODO

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
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define AM40010_DISPLAY_WIDTH (768)
#define AM40010_DISPLAY_HEIGHT (272)
#define AM40010_FRAMEBUFFER_WIDTH (1024)
#define AM40010_FRAMEBUFFER_HEIGHT (312)
#define AM40010_FRAMEBUFFER_SIZE_BYTES (AM40010_FRAMEBUFFER_WIDTH * AM40010_FRAMEBUFFER_HEIGHT)
#define AM40010_NUM_HWCOLORS (32 + 32)  // 32 colors plus pure black plus debug visualization colors

// Z80-compatible pins
#define AM40010_PIN_A13     (13)
#define AM40010_PIN_A14     (14)
#define AM40010_PIN_A15     (15)

#define AM40010_PIN_D0      (16)
#define AM40010_PIN_D1      (17)
#define AM40010_PIN_D2      (18)
#define AM40010_PIN_D3      (19)
#define AM40010_PIN_D4      (20)
#define AM40010_PIN_D5      (21)
#define AM40010_PIN_D6      (22)
#define AM40010_PIN_D7      (23)

#define AM40010_PIN_M1      (24)
#define AM40010_PIN_MREQ    (25)
#define AM40010_PIN_IORQ    (26)
#define AM40010_PIN_RD      (27)
#define AM40010_PIN_WR      (28)
#define AM40010_PIN_INT     (30)
#define AM40010_PIN_WAIT    (33)
#define AM40010_PIN_READY   (AM40010_PIN_WAIT)

// MC6845 compatible pins
#define AM40010_PIN_MA0     (0)
#define AM40010_PIN_MA1     (1)
#define AM40010_PIN_MA2     (2)
#define AM40010_PIN_MA3     (3)
#define AM40010_PIN_MA4     (4)
#define AM40010_PIN_MA5     (5)
#define AM40010_PIN_MA6     (6)
#define AM40010_PIN_MA7     (7)
#define AM40010_PIN_MA8     (8)
#define AM40010_PIN_MA9     (9)
#define AM40010_PIN_MA10    (10)
#define AM40010_PIN_MA11    (11)
#define AM40010_PIN_MA12    (12)
#define AM40010_PIN_MA13    (13)

#define AM40010_PIN_DE      (44)
#define AM40010_PIN_VS      (45)
#define AM40010_PIN_HS      (46)

#define AM40010_PIN_RA0     (48)
#define AM40010_PIN_RA1     (49)
#define AM40010_PIN_RA2     (50)
#define AM40010_PIN_RA3     (51)
#define AM40010_PIN_RA4     (52)

// AM40010 specific pins (starting at pin 40)
#define AM40010_PIN_SYNC    (41)

// pin masks
#define AM40010_A13     (1ULL<<AM40010_PIN_A13)
#define AM40010_A14     (1ULL<<AM40010_PIN_A14)
#define AM40010_A15     (1ULL<<AM40010_PIN_A15)
#define AM40010_D0      (1ULL<<AM40010_PIN_D0)
#define AM40010_D1      (1ULL<<AM40010_PIN_D1)
#define AM40010_D2      (1ULL<<AM40010_PIN_D2)
#define AM40010_D3      (1ULL<<AM40010_PIN_D3)
#define AM40010_D4      (1ULL<<AM40010_PIN_D4)
#define AM40010_D5      (1ULL<<AM40010_PIN_D5)
#define AM40010_D6      (1ULL<<AM40010_PIN_D6)
#define AM40010_D7      (1ULL<<AM40010_PIN_D7)
#define AM40010_M1      (1ULL<<AM40010_PIN_M1)
#define AM40010_MREQ    (1ULL<<AM40010_PIN_MREQ)
#define AM40010_IORQ    (1ULL<<AM40010_PIN_IORQ)
#define AM40010_RD      (1ULL<<AM40010_PIN_RD)
#define AM40010_WR      (1ULL<<AM40010_PIN_WR)
#define AM40010_INT     (1ULL<<AM40010_PIN_INT)
#define AM40010_WAIT    (1ULL<<AM40010_PIN_WAIT)
#define AM40010_READY   (1ULL<<AM40010_PIN_READY)
#define AM40010_MA0     (1ULL<<AM40010_PIN_MA0)
#define AM40010_MA1     (1ULL<<AM40010_PIN_MA1)
#define AM40010_MA2     (1ULL<<AM40010_PIN_MA2)
#define AM40010_MA3     (1ULL<<AM40010_PIN_MA3)
#define AM40010_MA4     (1ULL<<AM40010_PIN_MA4)
#define AM40010_MA5     (1ULL<<AM40010_PIN_MA5)
#define AM40010_MA6     (1ULL<<AM40010_PIN_MA6)
#define AM40010_MA7     (1ULL<<AM40010_PIN_MA7)
#define AM40010_MA8     (1ULL<<AM40010_PIN_MA8)
#define AM40010_MA9     (1ULL<<AM40010_PIN_MA9)
#define AM40010_MA10    (1ULL<<AM40010_PIN_MA10)
#define AM40010_MA11    (1ULL<<AM40010_PIN_MA11)
#define AM40010_MA12    (1ULL<<AM40010_PIN_MA12)
#define AM40010_MA13    (1ULL<<AM40010_PIN_MA13)
#define AM40010_DE      (1ULL<<AM40010_PIN_DE)
#define AM40010_VS      (1ULL<<AM40010_PIN_VS)
#define AM40010_HS      (1ULL<<AM40010_PIN_HS)
#define AM40010_RA0     (1ULL<<AM40010_PIN_RA0)
#define AM40010_RA1     (1ULL<<AM40010_PIN_RA1)
#define AM40010_RA2     (1ULL<<AM40010_PIN_RA2)
#define AM40010_RA3     (1ULL<<AM40010_PIN_RA3)
#define AM40010_RA4     (1ULL<<AM40010_PIN_RA4)
#define AM40010_SYNC    (1ULL<<AM40010_PIN_SYNC)

// config register bits
#define AM40010_CONFIG_MODE     ((1<<0)|(1<<1)) // video mode
#define AM40010_CONFIG_LROMEN   (1<<2)          // lower ROM enable
#define AM40010_CONFIG_HROMEN   (1<<3)          // higher ROM enable
#define AM40010_CONFIG_IRQRESET (1<<4)          // reset IRQ counter

// memory configuration callback
typedef void (*am40010_bankswitch_t)(uint8_t ram_config, uint8_t rom_enable, uint8_t rom_select, void* user_data);
/* CCLK callback, this will be called at 1 MHz frequency and must
   return the CRTC pin mask. Use this callback to tick the MC6845
   and AY-3-8910 chips.
*/
typedef uint64_t (*am40010_cclk_t)(void* user_data);

// host system type (same as cpc_type_t)
typedef enum am40010_cpc_type_t {
    AM40010_CPC_TYPE_6128,
    AM40010_CPC_TYPE_464,
    AM40010_CPC_TYPE_KCCOMPACT,
} am40010_cpc_type_t;

// setup parameters for am40010_init()
typedef struct am40010_desc_t {
    am40010_cpc_type_t cpc_type;        // host system type (mainly for bank switching)
    am40010_bankswitch_t bankswitch_cb; // memory bank-switching callback
    am40010_cclk_t cclk_cb;             // the 1 MHz CCLK callback
    chips_range_t ram;                  // direct pointer to the gate-array-visible 4*16 KByte RAM banks
    chips_range_t framebuffer;          // pointer to framebuffer (at least 1024 * 312 bytes)
    void* user_data;                    // optional userdata for callbacks
} am40010_desc_t;

// registers
typedef struct am40010_registers_t {
    uint8_t inksel;     // 5 bits
    uint8_t config;     // bit4: IRQ reset, bit3: HROMEN, bit2: LROMEN, bit1+0: mode
    uint8_t border;     // 5 bits
    uint8_t ink[16];    // 5 bits
} am40010_registers_t;

// vsync/video/irq generation
typedef struct am40010_video_t {
    int hscount;        // 5-bit counter updated at HSYNC falling edge
    int intcnt;         // 6-bit counter updated at HSYNC falling egde
    int clkcnt;         // 4-bit counter updated at 1 MHz
    uint8_t mode;       // currently active mode updated at hsync
    bool sync;          // state of the sync output pin
    bool intr;          // interrupt flip-flop
} am40010_video_t;

// CRT beam tracking
typedef struct am40010_crt_t {
    int pos_x, pos_y;   // current beam position in visible region
    int sync_count;     // number of ticks since sync raised
    int h_pos;          // current horizontal pos (0..63)
    int v_pos;          // current vertical position (0..312)
    int h_retrace;      // horizontal retrace counter
    int v_retrace;      // vertical retrace counter
    bool visible;       // true if beam is currently in visible region
    bool sync;          // last syns state for sync raise detection
    bool h_blank;       // true if currently in horizontal blanking
    bool v_blank;       // true if currently in vertical blanking
} am40010_crt_t;

// AM40010 state
typedef struct am40010_t {
    bool dbg_vis;               // debug visualization currently enabled?
    am40010_cpc_type_t cpc_type;
    uint32_t seq_tick_count;    // gate array sequencer ticks
    uint64_t crtc_pins;         // previous crtc pins
    uint8_t rom_select;         // select upper ROM (AMSDOS or BASIC)
    uint8_t ram_config;         // 3 bits, CPC 6128 RAM configuration
    am40010_registers_t regs;
    am40010_video_t video;
    am40010_crt_t crt;
    am40010_bankswitch_t bankswitch_cb;
    am40010_cclk_t cclk_cb;
    const uint8_t* ram;
    void* user_data;
    uint64_t pins;              // only for debug inspection
    uint8_t* fb;                // decoded framebuffer pixels as hw palette indices
    uint32_t hw_colors[AM40010_NUM_HWCOLORS]; // hardware colors (different for CPC and KCC)
} am40010_t;

void am40010_init(am40010_t* ga, const am40010_desc_t* desc);
void am40010_reset(am40010_t* ga);

/*
    Call the iorq function once per Z80 machine cycle when the
    IORQ and either RD or WR pins are set. This may call the
    bankswitch callback if the memory configuration needs to be changed.
*/
void am40010_iorq(am40010_t* ga, uint64_t cpu_pins);
/*
    Call the tick function once per Z80 tcycle with the CPU pins. The am40010_tick
    function will call the CCLK callback as needed (at 1 MHz frequency),
    tick the MC6845 and AY-3-8910 from this callback.

    am40010_tick() will return a new Z80 CPU pin mask with the following
    pins updated:

        AM40010_READY/Z80_WAIT - gate array wants the Z80 to wait
        AM40010_INT/Z80_INT    - interrupt request from the gate array was triggered
*/
uint64_t am40010_tick(am40010_t* ga, uint64_t cpu_pins);

// prepare am40010_t snapshot before saving
void am40010_snapshot_onsave(am40010_t* snapshot);
// fixup am40010_t snapshot after loading
void am40010_snapshot_onload(am40010_t* snapshot, am40010_t* sys);

#ifdef __cplusplus
} // extern "C"
#endif

/*--- IMPLEMENTATION ---------------------------------------------------------*/
#ifdef CHIPS_IMPL
#include <string.h>
#ifndef CHIPS_ASSERT
    #include <assert.h>
    #define CHIPS_ASSERT(c) assert(c)
#endif

#if defined(__GNUC__)
#define _AM40010_UNREACHABLE __builtin_unreachable()
#elif defined(_MSC_VER)
#define _AM40010_UNREACHABLE __assume(0)
#else
#define _AM40010_UNREACHABLE
#endif

#define _AM40010_MAX_FB_SIZE (AM40010_DBG_DISPLAY_WIDTH*AM40010_DBG_DISPLAY_HEIGHT*4)

// extract 8-bit data bus from 64-bit pin mask
#define _AM40010_GET_DATA(p) ((uint8_t)(((p)&0xFF0000ULL)>>16))

// the first 32 bytes of the KC Compact color ROM
static uint8_t _am40010_kcc_color_rom[32] = {
    0x15, 0x15, 0x31, 0x3d, 0x01, 0x0d, 0x11, 0x1d,
    0x0d, 0x3d, 0x3c, 0x3f, 0x0c, 0x0f, 0x1c, 0x1f,
    0x01, 0x31, 0x30, 0x33, 0x00, 0x03, 0x10, 0x13,
    0x05, 0x35, 0x34, 0x37, 0x04, 0x07, 0x14, 0x17
};

/*
  hardware color RGBA8 values for CPC 6128 and 464

  http://www.cpcwiki.eu/index.php/CPC_Palette
  http://www.grimware.org/doku.php/documentations/devices/gatearray
*/
static uint32_t _am40010_cpc_colors[32] = {
    0xff6B7D6E,         // #40 white
    0xff6D7D6E,         // #41 white
    0xff6BF300,         // #42 sea green
    0xff6DF3F3,         // #43 pastel yellow
    0xff6B0200,         // #44 blue
    0xff6802F0,         // #45 purple
    0xff687800,         // #46 cyan
    0xff6B7DF3,         // #47 pink
    0xff6802F3,         // #48 purple
    0xff6BF3F3,         // #49 pastel yellow
    0xff0DF3F3,         // #4A bright yellow
    0xffF9F3FF,         // #4B bright white
    0xff0605F3,         // #4C bright red
    0xffF402F3,         // #4D bright magenta
    0xff0D7DF3,         // #4E orange
    0xffF980FA,         // #4F pastel magenta
    0xff680200,         // #50 blue
    0xff6BF302,         // #51 sea green
    0xff01F002,         // #52 bright green
    0xffF2F30F,         // #53 bright cyan
    0xff010200,         // #54 black
    0xffF4020C,         // #55 bright blue
    0xff017802,         // #56 green
    0xffF47B0C,         // #57 sky blue
    0xff680269,         // #58 magenta
    0xff6BF371,         // #59 pastel green
    0xff04F571,         // #5A lime
    0xffF4F371,         // #5B pastel cyan
    0xff01026C,         // #5C red
    0xffF2026C,         // #5D mauve
    0xff017B6E,         // #5E yellow
    0xffF67B6E,         // #5F pastel blue
};

// initialize registers for poweron and reset
static void _am40010_init_regs(am40010_t* ga) {
    memset(&ga->regs, 0, sizeof(ga->regs));
}

// initialize video/vsync unit for poweron and reset
static void _am40010_init_video(am40010_t* ga) {
    memset(&ga->video, 0, sizeof(ga->video));
}

// initialize the crt init
static void _am40010_init_crt(am40010_t* ga) {
    memset(&ga->crt, 0, sizeof(ga->crt));
}

// initialize the hardware color palette
static void _am40010_init_hwcolors(am40010_t* ga) {
    if (ga->cpc_type != AM40010_CPC_TYPE_KCCOMPACT) {
        // Amstrad CPC colors
        for (size_t i = 0; i < 32; i++) {
            ga->hw_colors[i] = _am40010_cpc_colors[i];
        }
    }
    else {
        // KC Compact colors
        for (size_t i = 0; i < 32; i++) {
            uint32_t c = 0xFF000000;
            const uint8_t val = _am40010_kcc_color_rom[i];
            // color bits: xx|gg|rr|bb
            const uint8_t b = val & 0x03;
            const uint8_t r = (val>>2) & 0x03;
            const uint8_t g = (val>>4) & 0x03;
            if (b == 0x03)     c |= 0x00FF0000;    // full blue
            else if (b != 0)   c |= 0x007F0000;    // half blue
            if (g == 0x03)     c |= 0x0000FF00;    // full green
            else if (g != 0)   c |= 0x00007F00;    // half green
            if (r == 0x03)     c |= 0x000000FF;    // full red
            else if (r != 0)   c |= 0x0000007F;    // half red
            ga->hw_colors[i] = c;
        }
    }

    // debug visualization colors
    for (size_t i = 0x20; i < 0x3F; i++) {
        uint8_t r = 0x22, g = 0x22, b = 0x22;
        if (i >= 0x30) {
            // current ray position
            r = 0xFF;
            g = 0xFF;
            b = 0xFF;
        }
        else {
            if (i & 1) {
                r = 0x55;   // HS set
            }
            if (i & 2) {
                g = 0x55;   // VS set
            }
            if (i & 4) {
                r = 0xFF;   // SYNC set
            }
            if (i & 8) {
                g = 0xFF;   // INTR set
            }
        }
        ga->hw_colors[i] = 0xFF000000 | (b << 16) | (g << 8) | r;
    }

    // pure black (for area outside border)
    ga->hw_colors[0x3F] = 0xFF000000;
}

// initialize am40010_t instance
void am40010_init(am40010_t* ga, const am40010_desc_t* desc) {
    CHIPS_ASSERT(ga && desc);
    CHIPS_ASSERT(desc->bankswitch_cb && desc->cclk_cb);
    CHIPS_ASSERT(desc->framebuffer.ptr && (desc->framebuffer.size >= AM40010_FRAMEBUFFER_SIZE_BYTES));
    CHIPS_ASSERT(desc->ram.ptr && (desc->ram.size >= (64*1024)));
    memset(ga, 0, sizeof(am40010_t));
    ga->cpc_type = desc->cpc_type;
    ga->bankswitch_cb = desc->bankswitch_cb;
    ga->cclk_cb = desc->cclk_cb;
    ga->ram = desc->ram.ptr;
    ga->fb = desc->framebuffer.ptr;
    ga->user_data = desc->user_data;
    _am40010_init_regs(ga);
    _am40010_init_video(ga);
    _am40010_init_crt(ga);
    _am40010_init_hwcolors(ga);
    ga->bankswitch_cb(ga->ram_config, ga->regs.config, ga->rom_select, ga->user_data);
}

void am40010_reset(am40010_t* ga) {
    CHIPS_ASSERT(ga);
    ga->seq_tick_count = 0;
    _am40010_init_regs(ga);
    _am40010_init_video(ga);
    ga->bankswitch_cb(ga->ram_config, ga->regs.config, ga->rom_select, ga->user_data);
}

/* Call the am40010_iorq() function in the Z80 tick callback
   whenever the IORQ pin and RD or WR pin is set.
   The CPC gate array will always perform a register write,
   no matter if the CPU performs an IN or OUT operation.

   It's not possible to read data back from the gate array
   (that's why iorq has no return value).
*/
void am40010_iorq(am40010_t* ga, uint64_t pins) {
    // check that this is called during an IORQ machine cycle
    CHIPS_ASSERT(((pins & (AM40010_M1|AM40010_IORQ)) == (AM40010_IORQ)) && ((pins & (AM40010_RD|AM40010_WR)) != 0));
    // a gate array register write
    if ((pins & (AM40010_A14|AM40010_A15)) == AM40010_A14) {
        const uint8_t data = _AM40010_GET_DATA(pins);
        // data bits 6 and 7 select the register type
        switch (data & ((1<<7)|(1<<6))) {
            /* select color pen:
                bit 4 set means 'select border pen', otherwise
                one of the 16 ink pens
            */
            case 0:
                ga->regs.inksel = data & 0x1F;
                break;

            /* update the selected color, the actual color change
               will only be made visible during the tick() function
            */
            case (1<<6):
                if (ga->regs.inksel & (1<<4)) {
                    ga->regs.border = data & 0x1F;
                }
                else {
                    ga->regs.ink[ga->regs.inksel] = data & 0x1F;
                }
                break;

            /* update the config register:
                - bits 0 and 1 select the video mode (updated at next HSYNC):
                    00: 160x200 @ 16 colors
                    01: 320x200 @ 4 colors
                    10: 620x200 @ 2 colors
                    11: 160x200 @ 2 colors (undocumented, currently not supported)
                - bit 2: LROMEN (lower ROM enable)
                - bit 3: HROMEN (upper ROM enable)
                - bit 4: IRQ_RESET (not a flip-flop, only a one-shot signal)
            */
            case (1<<7):
                {
                    uint8_t romen_dirty = (ga->regs.config ^ data) & (AM40010_CONFIG_LROMEN|AM40010_CONFIG_HROMEN);
                    ga->regs.config = data & 0x1F;
                    if (0 != romen_dirty) {
                        ga->bankswitch_cb(ga->ram_config, ga->regs.config, ga->rom_select, ga->user_data);
                    }
                }
                break;

            // RAM bank switching (6128 only)
            case (1<<6)|(1<<7):
                if (AM40010_CPC_TYPE_6128 == ga->cpc_type) {
                    uint8_t ram_dirty = (ga->ram_config ^ data) & 7;
                    ga->ram_config = data & 7;
                    if (0 != ram_dirty) {
                        ga->bankswitch_cb(ga->ram_config, ga->regs.config, ga->rom_select, ga->user_data);
                    }
                }
                break;

            default: _AM40010_UNREACHABLE;
        }
    }

    // upper ROM bank select
    if ((pins & (AM40010_A13|AM40010_WR)) == AM40010_WR) {
        const uint8_t data = _AM40010_GET_DATA(pins);
        bool rom_select_dirty = ga->rom_select != data;
        ga->rom_select = data;
        if (rom_select_dirty) {
            ga->bankswitch_cb(ga->ram_config, ga->regs.config, ga->rom_select, ga->user_data);
        }
    }
}

#define _AM40010_CRT_VIS_X0  (6)        // start of CRT beam visible area
#define _AM40010_CRT_VIS_Y0  (32)
#define _AM40010_CRT_VIS_X1  (6+48)     // end of CRT beam visible area
#define _AM40010_CRT_VIS_Y1  (32+272)
#define _AM40010_CRT_H_DISPLAY_START    (6)
#define _AM40010_CRT_V_DISPLAY_START    (5)

/* tick the CRT emulation, call this at 1 MHz frequency (CCLK signal)
    FIXME: this needs work to properly emulate a "running picture"
    when the SYNC signal is off-limits or missing.
*/
static void _am40010_crt_tick(am40010_t* ga, bool sync) {
    am40010_crt_t* crt = &ga->crt;
    bool sync_raise = sync && !crt->sync;
    bool new_line = false;
    bool new_frame = false;
    crt->sync = sync;

    // bump sync counter if inside sync
    if (sync) {
        crt->sync_count++;
    }
    // if sync signal raised, start horizontal retrace and reset sync counter
    if (sync_raise) {
        crt->h_retrace = 7;
        crt->h_blank = true;
        crt->sync_count = 0;
    }
    /* if this is a 'long sync', start vertical retrace
        FIXME: ...or should we use a separate H/V sync signal instead?
    */
    if (crt->sync_count == 64) {
        crt->v_retrace = 3;
        crt->v_blank = true;
    }
    // horizontal update
    crt->h_pos++;
    if (crt->h_pos == _AM40010_CRT_H_DISPLAY_START) {
        crt->h_blank = false;
    }
    else if (crt->h_pos == 64) {
        // no hsync on this line
        new_line = true;
    }
    if (crt->h_retrace > 0) {
        crt->h_retrace--;
        if (crt->h_retrace == 0) {
            new_line = true;
        }
    }
    if (new_line) {
        // new scanline
        crt->h_pos = 0;
        crt->v_pos++;
        if (crt->v_pos == _AM40010_CRT_V_DISPLAY_START) {
            crt->v_blank = false;
        }
        else if (crt->v_pos == 312) {
            // no vsync on this frame
            new_frame = true;
        }
        if (crt->v_retrace > 0) {
            crt->v_retrace--;
            if (crt->v_retrace == 0) {
                new_frame = true;
            }
        }
    }
    if (new_frame) {
        crt->v_pos = 0;
    }

    // compute visible beam state
    if ((crt->h_pos >= _AM40010_CRT_VIS_X0) && (crt->h_pos < _AM40010_CRT_VIS_X1) &&
        (crt->v_pos >= _AM40010_CRT_VIS_Y0) && (crt->v_pos < _AM40010_CRT_VIS_Y1))
    {
        crt->visible = true;
        crt->pos_x = crt->h_pos - _AM40010_CRT_VIS_X0;
        crt->pos_y = crt->v_pos - _AM40010_CRT_VIS_Y0;
    }
    else {
        crt->visible = false;
    }
}

// helper functions to detect falling/rising edge on a bit
static inline bool _am40010_falling_u8(uint8_t new_val, uint8_t old_val, uint8_t mask) {
    return 0 != (mask & (~new_val & (new_val ^ old_val)));
}
static inline bool _am40010_falling_u64(uint64_t new_val, uint64_t old_val, uint64_t mask) {
    return 0 != (mask & (~new_val & (new_val ^ old_val)));
}
static inline bool _am40010_rising_u64(uint64_t new_val, uint64_t old_val, uint64_t mask) {
    return 0 != (mask & (new_val & (new_val ^ old_val)));
}

/* SYNC and IRQ generation, called at the CCLK frequency (1 MHz)
   returns the state of the sync pin (used as input for the CRT tick function).
   NOTE that the interrupt counter is also modified outside this
   function in am40010_tick
*/
static bool _am40010_sync_irq(am40010_t* ga, uint64_t crtc_pins) {
    bool hs_fall = _am40010_falling_u64(crtc_pins, ga->crtc_pins, AM40010_HS);
    bool vs_rise = _am40010_rising_u64(crtc_pins, ga->crtc_pins, AM40010_VS);

    // on VSYNC rise, clear the HSYNC counter
    if (vs_rise) {
        ga->video.hscount = 0;
    }
    // ...on falling HSYNC
    if (hs_fall) {

        // increment 6-bit INTCNT on hsync falling edge
        ga->video.intcnt = (ga->video.intcnt + 1) & 0x3F;
        ga->video.hscount++;

        // 2 HSYNCs after start of VSYNC, reset the interrupt counter
        if (ga->video.hscount == 2) {
            if (ga->video.intcnt >= 32) {
                ga->video.intr = true;
            }
            ga->video.intcnt = 0;
        }

        // if interrupt count reaches 52, it is reset to 0 and an interrupt is requested
        if (ga->video.intcnt == 52) {
            ga->video.intr = true;
            ga->video.intcnt = 0;
        }
    }

    // SYNC and MODESYNC via 1 MHz 4-bit CLKCNT
    uint8_t clkcnt = ga->video.clkcnt;
    if (clkcnt == 7) {
        // trigger video-mode switch
        ga->video.mode = ga->regs.config & AM40010_CONFIG_MODE;
    }
    // if HSYNC is off, force the clkcnt counter to 0
    if (0 == (crtc_pins & AM40010_HS)) {
        clkcnt = 0;
    }
    // FIXME: figure out why this "< 8" is needed (otherwise purple left column in Demoizart)
    else if (clkcnt < 8) {
        clkcnt++;
    }
    // v_sync is on as long as hscount is < 4
    bool v_sync = ga->video.hscount < 4;
    // h_sync is 2 ticks delayed from the CRTC HSYNC, and at most 4 ticks long
    bool h_sync = (clkcnt > 2) && (clkcnt < 7);
    ga->video.sync = h_sync || v_sync;
    // write back clkcnt
    ga->video.clkcnt = clkcnt;

    return ga->video.sync;
}

static void _am40010_decode_pixels(am40010_t* ga, uint8_t* dst, uint64_t crtc_pins) {
    /*
         compute the source address from current CRTC ma (memory address)
         and ra (raster address) like this:

         |ma13|ma12|ra2|ra1|ra0|ma9|ma8|ma7|ma6|ma5|ma4|ma3|ma2|ma1|ma0|0|

        Bits ma13 and m12 point to the 16 KByte page, and all
        other bits are the index into that page.
    */
    const uint16_t addr = ((crtc_pins & 0x3000) << 2) |     // MA13,MA12
                          ((crtc_pins & 0x3FF) << 1) |      // MA9..MA0
                          (((crtc_pins>>48) & 7) << 11);    // RA0..RA2
    const uint8_t* src = &(ga->ram[addr]);
    uint8_t c;
    uint8_t p;
    switch (ga->video.mode) {
        case 0:
            /*
                160x200 @ 16 colors (2 pixels per byte)
                pixel    bit mask
                0:       |1|5|3|7|
                1:       |0|4|2|6|
            */
            for (size_t i = 0; i < 2; i++) {
                c = *src++;
                p = ga->regs.ink[((c>>7)&0x1)|((c>>2)&0x2)|((c>>3)&0x4)|((c<<2)&0x8)];
                *dst++ = p; *dst++ = p; *dst++ = p; *dst++ = p;
                p = ga->regs.ink[((c>>6)&0x1)|((c>>1)&0x2)|((c>>2)&0x4)|((c<<3)&0x8)];
                *dst++ = p; *dst++ = p; *dst++ = p; *dst++ = p;
            }
            break;
        case 1:
            /*
                320x200 @ 4 colors (4 pixels per byte)
                pixel    bit mask
                0:       |3|7|
                1:       |2|6|
                2:       |1|5|
                3:       |0|4|
            */
            for (size_t i = 0; i < 2; i++) {
                c = *src++;
                p = ga->regs.ink[((c>>2)&2)|((c>>7)&1)];
                *dst++ = p; *dst++ = p;
                p = ga->regs.ink[((c>>1)&2)|((c>>6)&1)];
                *dst++ = p; *dst++ = p;
                p = ga->regs.ink[((c>>0)&2)|((c>>5)&1)];
                *dst++ = p; *dst++ = p;
                p = ga->regs.ink[((c<<1)&2)|((c>>4)&1)];
                *dst++ = p; *dst++ = p;
            }
            break;
        case 2:
            // 640x200 @ 2 colors (8 pixels per byte)
            for (size_t i = 0; i < 2; i++) {
                c = *src++;
                *dst++ = ga->regs.ink[(c>>7)&1];
                *dst++ = ga->regs.ink[(c>>6)&1];
                *dst++ = ga->regs.ink[(c>>5)&1];
                *dst++ = ga->regs.ink[(c>>4)&1];
                *dst++ = ga->regs.ink[(c>>3)&1];
                *dst++ = ga->regs.ink[(c>>2)&1];
                *dst++ = ga->regs.ink[(c>>1)&1];
                *dst++ = ga->regs.ink[(c>>0)&1];
            }
            break;
        case 3:
            /*  undocumented mode 3:
                160x200 @ 4 colors (2 pixels per byte)
                pixel    bit mask
                0:       |x|x|3|7|
                1:       |x|x|2|6|
            */
            for (size_t i = 0; i < 2; i++) {
                c = *src++;
                p = ga->regs.ink[((c>>7)&0x1)|((c>>2)&0x2)];
                *dst++ = p; *dst++ = p; *dst++ = p; *dst++ = p;
                p = ga->regs.ink[((c>>6)&0x1)|((c>>1)&0x2)];
                *dst++ = p; *dst++ = p; *dst++ = p; *dst++ = p;
            }
            break;
        default: _AM40010_UNREACHABLE;
    }
}

// video signal generator, call this at 1 MHz frequency
static void _am40010_decode_video(am40010_t* ga, uint64_t crtc_pins) {
    if (ga->dbg_vis) {
        size_t dst_x = ga->crt.h_pos * 16;
        size_t dst_y = ga->crt.v_pos;
        if ((dst_x <= (AM40010_FRAMEBUFFER_WIDTH-16)) && (dst_y < AM40010_FRAMEBUFFER_HEIGHT)) {
            uint8_t* dst = &(ga->fb[dst_x + dst_y * AM40010_FRAMEBUFFER_WIDTH]);
            uint8_t* prev_dst;
            if (dst == ga->fb) {
                prev_dst = dst;
            }
            else {
                prev_dst = dst - 16;
            }
            uint8_t c = 0x20;
            if (crtc_pins & AM40010_HS) {
                c |= 0x01;
            }
            if (crtc_pins & AM40010_VS) {
                c |= 0x02;
            }
            if (ga->video.sync) {
                c |= 0x04;
            }
            if (ga->video.intr) {
                c |= 0x08;
            }
            if (crtc_pins & AM40010_DE) {
                _am40010_decode_pixels(ga, dst, crtc_pins);
                for (size_t i = 0; i < 16; i++) {
                    if (0 == (i & 2)) {
                        dst[i] = c ^ 0x10;
                        prev_dst[i] ^= 0x10;
                    }
                }
            }
            else {
                for (size_t i = 0; i < 16; i++) {
                    if (0 == (i & 2)) {
                        dst[i] = c ^ 0x10;
                        prev_dst[i] ^= 0x10;
                    }
                    else {
                        dst[i] = 63;
                    }
                }
            }
        }
    }
    else if (ga->crt.visible) {
        size_t dst_x = ga->crt.pos_x * 16;
        size_t dst_y = ga->crt.pos_y;
        bool black = ga->video.sync;
        uint8_t* dst = &ga->fb[dst_x + dst_y * AM40010_FRAMEBUFFER_WIDTH];
        if (crtc_pins & AM40010_DE) {
            _am40010_decode_pixels(ga, dst, crtc_pins);
        }
        else if (black) {
            for (int i = 0; i < 16; i++) {
                *dst++ = 63;    // special 'pure black' hw color
            }
        }
        else {
            for (int i = 0; i < 16; i++) {
                *dst++ = ga->regs.border;
            }
        }
    }
}

// the actions which need to happen on CCLK (1 MHz frequency)
static inline void _am40010_do_cclk(am40010_t* ga, uint64_t crtc_pins) {
    bool sync = _am40010_sync_irq(ga, crtc_pins);
    _am40010_crt_tick(ga, sync);
    _am40010_decode_video(ga, crtc_pins);
}

// the tick function must be called at 4 MHz
uint64_t am40010_tick(am40010_t* ga, uint64_t pins) {
    /* The hardware has a 'main sequencer' with a rotating bit
        pattern which defines when the different actions happen in
        the 16 MHz ticks.
        Since the software emu is only ticked at 4 MHz, we'll replace
        the sequencer with a counter updated at the 4 MHz tick rate.
    */
    ga->seq_tick_count++;

    /* the sequencer is reset on an interrupt acknowledge machine cycle
        NOTE: the actual clock tick in the machine cycle may be important here
    */
    const bool int_ack = (pins & (Z80_M1|Z80_IORQ)) == (Z80_M1|Z80_IORQ);
    if (int_ack) {
        ga->seq_tick_count = 0;
    }
    /* derive the 1 MHz CCLK signal from the sequencer, and perform
        the actions that need to happen at the CCLK tick
        NOTE: the actual position where RDY and CCLK happens is important,
        experiment with 0, 1, 2, 3.

        NOTE: Logon's Run crashes on rdy:1 and rdy:2
    */
    const bool rdy  = 0 != (ga->seq_tick_count & 3);
    const bool cclk = 1 == (ga->seq_tick_count & 3);
    if (rdy) {
        // READY is connected to Z80 WAIT, this sets the WAIT pin
        // in 3 out of 4 CPU clock cycles
        pins |= AM40010_READY;
    }
    else {
        pins &= ~AM40010_READY;
    }
    if (cclk) {
        uint64_t crtc_pins = ga->cclk_cb(ga->user_data);
        _am40010_do_cclk(ga, crtc_pins);
        ga->crtc_pins = crtc_pins;
    }

    // perform the per-4Mhz-tick actions, the AM40010_READY pin is also the Z80_WAIT pin
    if ((ga->regs.config & AM40010_CONFIG_IRQRESET) != 0) {
        // when the IRQ_RESET bit is set, reset the interrupt counter and clear the interrupt flipflop
        ga->regs.config &= ~AM40010_CONFIG_IRQRESET;
        ga->video.intcnt = 0;
        ga->video.intr = false;
    }
    if (int_ack && ga->video.intr) {
        // on M1|IORQ|INT set, clear the interrupt flip-flop, and bit 5 of the interrupt counter
       ga->video.intcnt &= 0x1F;
       ga->video.intr = false;
    }

    // FIXME: drop .intr and directly set INT pin?
    if (ga->video.intr) { pins |= AM40010_INT; }
    else                { pins &= ~AM40010_INT; }
    // FIXME: drop .sync and directly set SYNC pin?
    if (ga->video.sync) {
        pins |= AM40010_SYNC;
    }
    ga->pins = pins | ((AM40010_DE|AM40010_HS|AM40010_VS) & ga->crtc_pins);
    return pins;
}

void am40010_snapshot_onsave(am40010_t* snapshot) {
    CHIPS_ASSERT(snapshot);
    snapshot->bankswitch_cb = 0;
    snapshot->cclk_cb = 0;
    snapshot->user_data = 0;
    snapshot->ram = 0;
    snapshot->fb = 0;
}

void am40010_snapshot_onload(am40010_t* snapshot, am40010_t* sys) {
    CHIPS_ASSERT(snapshot && sys);
    snapshot->bankswitch_cb = sys->bankswitch_cb;
    snapshot->cclk_cb = sys->cclk_cb;
    snapshot->user_data = sys->user_data;
    snapshot->ram = sys->ram;
    snapshot->fb = sys->fb;
}

#endif // CHIPS_IMPL
