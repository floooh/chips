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
    *   IORQ --->|           |---> ROMEN *
    *     RD --->|           |---> RAMRD *
    *            |           |---> READY *
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
#define AM40010_ROMEN   (1ULL<<42)
#define AM40010_RAMRD   (1ULL<<43)

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

/* AM40010 state */
typedef struct am40010_t {
    am40010_cpc_type_t cpc_type;

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

void am40010_init(am40010_t* ga, am40010_desc_t* desc) {
    CHIPS_ASSERT(ga && desc);
    CHIPS_ASSERT(desc->bank_cb);
    CHIPS_ASSERT(desc->rgba8_buffer && (desc->rgba8_buffer_size >= _AM40010_MAX_FB_SIZE));
    CHIPS_ASSERT(desc->ram && (desc->ram_size == (128*1024));
    memset(ga, 0, sizeof(am40010_t));
    ga->cpc_type = desc->cpc_type;
    ga->bank_cb = desc->bank_cb;
    ga->rgba8_buffer = desc->rgba8_buffer;
    ga->user_data = desc->user_data;
}

void am40010_reset(am40010_t* ga) {
    CHIPS_ASSERT(ga);
    // FIXME!
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