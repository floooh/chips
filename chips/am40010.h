#pragma once
/*#
    # am40010.h

    Amstrad CPC 40010 gate array (and PAL) emulator.

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

    ************************************ *
    *            +-----------+           *
    *     M1 --->|           |---> SYNC  *
    *   MREQ --->|           |---> INT   *
    *   IORQ --->|           |---> READY *
    *     RD --->|           |           *
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

    - the pins M1, MREQ, IORQ, RD, INT pins are directly 
      mapped to the Z80 pins of the same names both for the
      am40010_iorq() and am40010_tick() functions.
    - the pins D0..D7 and A14..15 are directly mapped to the
      corresponding Z80 pins in the am40010_iorq() function
    - the HSYNC, VSYNC and DISPEN pins are mapped to the
      mc6845_t pins HS, VS and DE for the am40010_tick() function
    - unlike the hardware, the am40010_t performs its own
      memory accesses during am40010_tick(), and for this reason
      the tick function takes two pin mask (the usual CPU pins,
      and the pin mask returned from mc6845_tick())
    - the READY pin is connected to the Z80 WAIT pin, but since
      z80_t has wait-count pins, every time am40010_tick() returns
      with the READY pin set, a wait cycle must be added to the
      wait counter pins
    - the A13 input pin doesn't exist on the real gate array 
      chip, it's used because the memory bank switching has
      been integrated to select the upper ROM bank
    - memory bank switching is implemented through a custom callback
      which is called whenever the memory configuration needs to change

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

#ifdef __cplusplus
extern "C" {
#endif

#define AM40010_DISPLAY_WIDTH (768)
#define AM40010_DISPLAY_HEIGHT (272)
#define AM40010_DBG_DISPLAY_WIDTH (1024)
#define AM40010_DBG_DISPLAY_HEIGHT (312)

/* Z80-compatible pins */
#define AM40010_A13     (1ULL<<13)
#define AM40010_A14     (1ULL<<14)
#define AM40010_A15     (1ULL<<15)

#define AM40010_D0      (1ULL<<16)
#define AM40010_D1      (1ULL<<17)
#define AM40010_D2      (1ULL<<18)
#define AM40010_D3      (1ULL<<19)
#define AM40010_D4      (1ULL<<20)
#define AM40010_D5      (1ULL<<21)
#define AM40010_D6      (1ULL<<22)
#define AM40010_D7      (1ULL<<23)

#define AM40010_M1      (1ULL<<24)
#define AM40010_MREQ    (1ULL<<25)
#define AM40010_IORQ    (1ULL<<26)
#define AM40010_RD      (1ULL<<27)
#define AM40010_INT     (1ULL<<30)

/* MC6845 compatible pins */
#define AM40010_MA0     (1ULL<<0)
#define AM40010_MA1     (1ULL<<1)
#define AM40010_MA2     (1ULL<<2)
#define AM40010_MA3     (1ULL<<3)
#define AM40010_MA4     (1ULL<<4)
#define AM40010_MA5     (1ULL<<5)
#define AM40010_MA6     (1ULL<<6)
#define AM40010_MA7     (1ULL<<7)
#define AM40010_MA8     (1ULL<<8)
#define AM40010_MA9     (1ULL<<9)
#define AM40010_MA10    (1ULL<<10)
#define AM40010_MA11    (1ULL<<11)
#define AM40010_MA12    (1ULL<<12)
#define AM40010_MA13    (1ULL<<13)

#define AM40010_DE      (1ULL<<44)
#define AM40010_VS      (1ULL<<45)
#define AM40010_HS      (1ULL<<46)

#define AM40010_RA0     (1ULL<<48)
#define AM40010_RA1     (1ULL<<49)
#define AM40010_RA2     (1ULL<<50)
#define AM40010_RA3     (1ULL<<51)
#define AM40010_RA4     (1ULL<<52)

/* AM40010 specific pins (starting at pin 40) */
#define AM40010_READY   (1ULL<<40)
#define AM40010_SYNC    (1ULL<<41)

/* memory configuration callback */
typedef void (*am40010_bank_t)(uint8_t ram_config, uint8_t rom_select, void* user_data);

/* host system type (same as cpc_type_t) */
typedef enum am40010_cpc_type_t {
    AM40010_CPC_TYPE_6128,
    AM40010_CPC_TYPE_464,
    AM40010_CPC_TYPE_KCCOMPACT,
} am40010_cpc_type_t;

/* setup parameters for am40010_init() */
typedef struct am40010_desc_t {
    am40010_cpc_type_t cpc_type;    /* host system type (mainly for bank switching) */
    am40010_bank_t bank_cb;         /* memory bank-switching callback */
    const uint8_t* ram;             /* direct pointer to 8*16 KByte RAM banks */
    uint32_t ram_size;              /* must be 128 KBytes */
    uint32_t* rgba8_buffer;         /* pointer the RGBA8 output framebuffer */
    uint32_t rgba8_buffer_size;     /* must be at least 1024*312*4 bytes */
    void* user_data;                /* optional userdata for callbacks */
} am40010_desc_t;

/* registers */
typedef struct am40010_registers_t {
    uint8_t inksel;     /* 5 bits */
    uint8_t mode;       /* 2 bits, not the currently displayed mode, see mode_is! */
    uint8_t irq_reset;  /* 1 bit */
    uint8_t border;     /* 6 bits, see also border_rgba8 */
    uint8_t ink[16];    /* 5 bits, see also ink_rgba8 */
} am40010_registers_t;

/* decoded RGBA8 colors */
typedef struct am40010_colors_t {
    uint32_t ink_rgba8[16];         /* the current ink colors as RGBA8 */
    uint32_t border_rgba8;          /* the current border color as RGBA8 */
    uint32_t hw_rgba8[32];          /* the hardware color RGBA8 values */
} am40010_colors_t;

/* vsync/video unit */
typedef struct am40010_video_t {
    uint8_t mode_is;    /* currently active mode updated at hsync */
    uint8_t hcnt;       /* 5-bit counter */
    uint8_t intcnt;     /* 6-bit counter */
} am40010_video_t;

/* AM40010 state */
typedef struct am40010_t {
    am40010_cpc_type_t cpc_type;
    am40010_registers_t regs;
    am40010_video_t video;
    am40010_colors_t colors;
    am40010_bank_t bank_cb;
    const uint8_t* ram;
    uint32_t* rgba8_buffer;
    void* user_data;
} am40010_t;

void am40010_init(am40010_t* ga, am40010_desc_t* desc);
void am40010_reset(am40010_t* ga);
uint64_t am40010_iorq(am40010_t* ga, uint64_t cpu_pins);
uint64_t am40010_tick(am40010_t* ga, uint64_t cpu_pins, uint64_t crtc_pins);

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

#define _AM40010_MAX_FB_SIZE (AM40010_DBG_DISPLAY_WIDTH*AM40010_DBG_DISPLAY_HEIGHT*4)

/* the first 32 bytes of the KC Compact color ROM */
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

/* initialize registers for poweron and reset */
static void _am40010_init_regs(am40010_t* ga) {
    memset(&ga->regs, 0, sizeof(ga->regs));
}

/* initialize video/vsync unit for poweron and reset */
static void _am40010_init_video(am40010_t* ga) {
    memset(&ga->video, 0, sizeof(ga->video));
}

/* initialize the RGBA8 color caches, assumes already zero-initialized */
static void _am40010_init_colors(am40010_t* ga) {
    if (ga->cpc_type != AM40010_CPC_TYPE_KCCOMPACT) {
        /* Amstrad CPC colors */
        for (int i = 0; i < 32; i++) {
            ga->colors.hw_rgba8[i] = _am40010_cpc_colors[i];
        }
    }
    else {
        /* KC Compact colors */
        for (int i = 0; i < 32; i++) {
            uint32_t c = 0xFF000000;
            const uint8_t val = _am40010_kcc_color_rom[i];
            /* color bits: xx|gg|rr|bb */
            const uint8_t b = val & 0x03;
            const uint8_t r = (val>>2) & 0x03;
            const uint8_t g = (val>>4) & 0x03;
            if (b == 0x03)     c |= 0x00FF0000;    /* full blue */
            else if (b != 0)   c |= 0x007F0000;    /* half blue */
            if (g == 0x03)     c |= 0x0000FF00;    /* full green */
            else if (g != 0)   c |= 0x00007F00;    /* half green */
            if (r == 0x03)     c |= 0x000000FF;    /* full red */
            else if (r != 0)   c |= 0x0000007F;    /* half red */
            ga->colors.hw_rgba8[i] = c;
        }
    }
}

/* initialize am40010_t instance */
void am40010_init(am40010_t* ga, am40010_desc_t* desc) {
    CHIPS_ASSERT(ga && desc);
    CHIPS_ASSERT(desc->bank_cb);
    CHIPS_ASSERT(desc->rgba8_buffer && (desc->rgba8_buffer_size >= _AM40010_MAX_FB_SIZE));
    CHIPS_ASSERT(desc->ram && (desc->ram_size == (128*1024)));
    memset(ga, 0, sizeof(am40010_t));
    ga->cpc_type = desc->cpc_type;
    ga->bank_cb = desc->bank_cb;
    ga->rgba8_buffer = desc->rgba8_buffer;
    ga->user_data = desc->user_data;
    _am40010_init_regs(ga);
    _am40010_init_video(ga);
    _am40010_init_colors(ga);
}

void am40010_reset(am40010_t* ga) {
    CHIPS_ASSERT(ga);
    _am40010_init_regs(ga);
    _am40010_init_video(ga);
}

uint64_t am40010_iorq(am40010_t* ga, uint64_t cpu_pins) {
    // FIXME
    return cpu_pins;
}

uint64_t am40010_tick(am40010_t* ga, uint64_t cpu_pins, uint64_t crtc_pins) {
    // FIXME
    uint64_t pins = crtc_pins;
    return pins;
}

#endif /* CHIPS_IMPL */