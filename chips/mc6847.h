#pragma once
/*
    mc6847.h -- Motorola 6847 Video Display Generator

    Do this:
        #define CHIPS_IMPL
    before you include this file in *one* C or C++ file to create the 
    implementation.

    Optionally provide the following macros with your own implementation
    
        CHIPS_ASSERT(c)     -- your own assert macro (default: assert(c))

    EMULATED PINS:

                  +-----------+
            FS <--|           |--> A0
            HS <--|           |...
          (RP) <--|           |--> A12
                  |           |
            AG -->|           |
            AS -->|  MC6847   |
        INTEXT -->|           |<-- D0
           GM0 -->|           |...
           GM1 -->|           |<-- D7
           GM2 -->|           |
           INV -->|           |
           CSS -->|           |
                  +-----------+

    FIXME: documentation

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
*/
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
    Pin definitions

    The 13 address bus pins A0 to A12 are on the same location as the
    first 13 CPU address bus pins.

    The data bus pins are on the same location as the CPU's.

    The control pins start at location 40.

    For the synchronization output pins FS (field sync), HS (horizontal sync),
    and RP (Row Preset) not only the current state is important, but
    also the transitions from active to inactive. To capture these
    transitions, two extra 64-bit pin struct members are provided
    which store the raising and falling transition state for each
    pin bit.

    FS (field sync)         --  The inactive-to-active transition coincides with the
                                end of the active display area. The active-to-inactive
                                transition coincides with the trailing edge of the
                                vertical synchronization pulse.

    HS (horizontal sync)    --  The HS pulse coincides with the horizontal synchronization
                                pulse furnished to the television receiver by the VDG.
                                The inactive-to-active transition coincides with the
                                leading edge of the horizontal synchronization pulse
                                and the active-to-inactive transition coincides with the
                                trailing egde.

    RP (row preset)         --  The RP pin will go active every 12 lines to reset
                                a counter for an optional external character rom.

    Mode select pin functions:

    | AG | AS | INT/EXT | INV | GM2 | GM1 | GM0 | # colors | description
    +----+----+---------+-----+-----+-----+-----+----------+------------------------------
    |  0 |  0 |    0    |  0  |  -  |  -  |  -  |     2    | internal alphanum
    |  0 |  0 |    0    |  1  |  -  |  -  |  -  |     2    | internal alphanum, inverted
    |  0 |  0 |    1    |  0  |  -  |  -  |  -  |     2    | external alphanum
    |  0 |  0 |    1    |  1  |  -  |  -  |  -  |     2    | external alphanum, inverted
    +----+----+---------+-----+-----+-----+-----+----------+------------------------------
    |  0 |  1 |    0    |  -  |  -  |  -  |  -  |     8    | semigraphics4 (SG4)
    |  0 |  1 |    1    |  -  |  -  |  -  |  -  |     8    | semigraphics6 (SG6)
    +----+----+---------+-----+-----+-----+-----+----------+------------------------------
    |  1 |  - |    -    |  -  |  0  |  0  |  0  |     4    | 64x64 color graphics 1 (CG1)
    |  1 |  - |    -    |  -  |  0  |  0  |  1  |     2    | 128x64 resolution graphics 1 (RG1)
    |  1 |  - |    -    |  -  |  0  |  1  |  0  |     4    | 128x64 color graphics 2 (CG2)
    |  1 |  - |    -    |  -  |  0  |  1  |  1  |     2    | 128x96 resolution graphics 2 (RG2)
    |  1 |  - |    -    |  -  |  1  |  0  |  0  |     4    | 128x96 color graphics 3 (CG3)
    |  1 |  - |    -    |  -  |  1  |  0  |  1  |     2    | 128x192 resolution graphics 3 (RG3)
    |  1 |  - |    -    |  -  |  1  |  1  |  0  |     4    | 128x192 color graphics 6 (CG6)
    |  1 |  - |    -    |  -  |  1  |  1  |  1  |     2    | 256x192 resolution graphics 6 (RG6)

    The CSS pins select between 2 possible color sets.
*/

/* address bus pins */
#define MC6847_A0   (1ULL<<0)
#define MC6847_A1   (1ULL<<1)
#define MC6847_A2   (1ULL<<2)
#define MC6847_A3   (1ULL<<3)
#define MC6847_A4   (1ULL<<4)
#define MC6847_A5   (1ULL<<5)
#define MC6847_A6   (1ULL<<6)
#define MC6847_A7   (1ULL<<7)
#define MC6847_A8   (1ULL<<8)
#define MC6847_A9   (1ULL<<9)
#define MC6847_A10  (1ULL<<10)
#define MC6847_A11  (1ULL<<11)
#define MC6847_A12  (1ULL<<12)

/* data bus pins */
#define MC6847_D0   (1ULL<<16)
#define MC6847_D1   (1ULL<<17)
#define MC6847_D2   (1ULL<<18)
#define MC6847_D3   (1ULL<<19)
#define MC6847_D4   (1ULL<<20)
#define MC6847_D5   (1ULL<<21)
#define MC6847_D6   (1ULL<<22)
#define MC6847_D7   (1ULL<<23)

/* synchronization output pins */
#define MC6847_FS   (1ULL<<40)      /* field sync */
#define MC6847_HS   (1ULL<<41)      /* horizontal sync */
#define MC6847_RP   (1ULL<<42)      /* row preset (not emulated) */

/* mode-select input pins */
#define MC6847_AG       (1ULL<<43)      /* graphics mode enable */
#define MC6847_AS       (1ULL<<44)      /* semi-graphics mode enable */
#define MC6847_INTEXT   (1ULL<<45)      /* internal/external select */
#define MC6847_INV      (1ULL<<46)      /* invert enable */
#define MC6847_GM0      (1ULL<<47)      /* graphics mode select 0 */
#define MC6847_GM1      (1ULL<<48)      /* graphics mode select 1 */
#define MC6847_GM2      (1ULL<<49)      /* graphics mode select 2 */
#define MC6847_CSS      (1ULL<<50)      /* color select pin */
#define MC6847_CTRL_PINS (MC6847_AG|MC6847_AS|MC6847_INTEXT|MC6847_INV|MC6847_GM0|MC6847_GM1|MC6847_GM2|MC6847_CSS)

/* helper macros to set and extract address and data to/from pin mask */

/* extract 13-bit address bus from 64-bit pins */
#define MC6847_GET_ADDR(p) ((uint16_t)(p&0xFFFFULL))
/* merge 13-bit address bus value into 64-bit pins */
#define MC6847_SET_ADDR(p,a) {p=((p&~0xFFFFULL)|((a)&0xFFFFULL));}
/* extract 8-bit data bus from 64-bit pins */
#define MC6847_GET_DATA(p) ((uint8_t)((p&0xFF0000ULL)>>16))
/* merge 8-bit data bus value into 64-bit pins */
#define MC6847_SET_DATA(p,d) {p=((p&~0xFF0000ULL)|(((d)<<16)&0xFF0000ULL));}

/* public constants */
#define MC6847_VBLANK_LINES         (13)    /* 13 lines vblank at top of screen */
#define MC6847_TOP_BORDER_LINES     (25)    /* 25 lines top border */
#define MC6847_DISPLAY_LINES        (192)   /* 192 lines visible display area */
#define MC6847_BOTTOM_BORDER_LINES  (26)    /* 26 lines bottom border */
#define MC6847_VRETRACE_LINES       (6)     /* 6 'lines' for vertical retrace */
#define MC6847_ALL_LINES            (262)   /* all of the above */
#define MC6847_DISPLAY_START        (MC6847_VBLANK_LINES+MC6847_TOP_BORDER_LINES)
#define MC6847_DISPLAY_END          (MC6847_DISPLAY_START+MC6847_DISPLAY_LINES)
#define MC6847_BOTTOM_BORDER_END    (MC6847_DISPLAY_END+MC6847_BOTTOM_BORDER_LINES)
#define MC6847_FSYNC_START          (MC6847_DISPLAY_END)

/* pixel width and height of entire visible area, including border */
#define MC6847_DISPLAY_WIDTH (320)
#define MC6847_DISPLAY_HEIGHT (MC6847_TOP_BORDER_LINES+MC6847_DISPLAY_LINES+MC6847_BOTTOM_BORDER_LINES)

/* pixel width and height of only the image area, without border */
#define MC6847_IMAGE_WIDTH (256)
#define MC6847_IMAGE_HEIGHT (192)

/* horizontal border width */
#define MC6847_BORDER_PIXELS ((MC6847_DISPLAY_WIDTH-MC6847_IMAGE_WIDTH)/2)

/* the MC6847 is always clocked at 3.579 MHz */
#define MC6847_TICK_HZ (3579545)

/* fixed point precision for more precise error accumulation */
#define MC6847_FIXEDPOINT_SCALE (16)

/* a memory-fetch callback, used to read video memory bytes into the MC6847 */
typedef uint64_t (*mc6847_fetch_t)(uint64_t pins, void* user_data);

/* the mc6847 setup parameters */
typedef struct {
    /* the CPU tick rate in hz */
    int tick_hz;
    /* pointer to an RGBA8 framebuffer where video image is written to */
    uint32_t* rgba8_buffer;
    /* size of rgba8_buffer in bytes (must be at least 320*244*4=312320 bytes) */
    uint32_t rgba8_buffer_size;
    /* memory-fetch callback */
    mc6847_fetch_t fetch_cb;
    /* optional user-data for the fetch callback */
    void* user_data;
} mc6847_desc_t;

/* the mc6847 state struct */
typedef struct {
    /* last pin state */
    uint64_t pins;
    /* the graphics mode color palette (RGBA8) */
    uint32_t palette[8];
    /* the black color as RGBA8 */
    uint32_t black;
    /* the alpha-numeric mode bright and dark green and orange colors */
    uint32_t alnum_green;
    uint32_t alnum_dark_green;
    uint32_t alnum_orange;
    uint32_t alnum_dark_orange;

    /* internal counters */
    int h_count;
    int h_sync_start;
    int h_sync_end;
    int h_period;
    int l_count;

    /* true during field-sync */
    bool fs;

    /* the fetch callback function */
    mc6847_fetch_t fetch_cb;
    /* optional user-data for the fetch-callback */
    void* user_data;
    /* pointer to RGBA8 buffer where decoded video image is written too */
    uint32_t* rgba8_buffer;
} mc6847_t;

/* initialize a new mc6847_t instance */
void mc6847_init(mc6847_t* vdg, const mc6847_desc_t* desc);
/* reset a mc6847_t instance */
void mc6847_reset(mc6847_t* vdg);
/* tick the mc6847_t instance, this will call the fetch_cb and generate the image */
uint64_t mc6847_tick(mc6847_t* vdg, uint64_t pins);

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

#define _MC6847_CLAMP(x) ((x)>255?255:(x))
#define _MC6847_RGBA(r,g,b) (0xFF000000|_MC6847_CLAMP((r*4)/3)|(_MC6847_CLAMP((g*4)/3)<<8)|(_MC6847_CLAMP((b*4)/3)<<16))

void mc6847_init(mc6847_t* vdg, const mc6847_desc_t* desc) {
    CHIPS_ASSERT(vdg && desc);
    CHIPS_ASSERT(desc->rgba8_buffer);
    CHIPS_ASSERT(desc->rgba8_buffer_size >= (MC6847_DISPLAY_WIDTH*MC6847_DISPLAY_HEIGHT*sizeof(uint32_t)));
    CHIPS_ASSERT(desc->fetch_cb);
    CHIPS_ASSERT((desc->tick_hz > 0) && (desc->tick_hz < MC6847_TICK_HZ));

    memset(vdg, 0, sizeof(*vdg));
    vdg->rgba8_buffer = desc->rgba8_buffer;
    vdg->fetch_cb = desc->fetch_cb;
    vdg->user_data = desc->user_data;

    /* compute counter periods, the MC6847 is always clocked at 3.579 MHz,
       and the frequency of how the tick function is called must be 
       communicated to the init function

       one scanline is 228 3.5 MC6847 ticks
    */
    int64_t tmp = (228LL * desc->tick_hz * MC6847_FIXEDPOINT_SCALE) / MC6847_TICK_HZ;
    vdg->h_period = (int) tmp;
    /* hsync starts at tick 10 of a scanline */
    tmp = (10LL * desc->tick_hz * MC6847_FIXEDPOINT_SCALE) / MC6847_TICK_HZ;
    vdg->h_sync_start = (int) tmp;
    /* hsync is 16 ticks long */
    tmp = (26LL * desc->tick_hz * MC6847_FIXEDPOINT_SCALE) / MC6847_TICK_HZ;
    vdg->h_sync_end = (int) tmp;

    /* the default graphics mode color palette

       the MC6847 outputs three color values:
        - Y' - six level analog luminance
        - phiA - three level analog (U)
        - phiB - three level analog (V)

        see discussion here: http://forums.bannister.org/ubbthreads.php?ubb=showflat&Number=64986

        NEW VALUES from here: http://www.stardot.org.uk/forums/viewtopic.php?f=44&t=12503

        green:      19 146  11
        yellow:    155 150  10
        blue:        2  22 175
        red:       155  22   7
        buff:      141 150 154
        cyan:       15 143 155
        magenta:   139  39 155
        orange:    140  31  11

        color intensities are slightly boosted
    */
    vdg->palette[0] = _MC6847_RGBA(19, 146, 11);      /* green */
    vdg->palette[1] = _MC6847_RGBA(155, 150, 10);     /* yellow */
    vdg->palette[2] = _MC6847_RGBA(2, 22, 175);       /* blue */
    vdg->palette[3] = _MC6847_RGBA(155, 22, 7);       /* red */
    vdg->palette[4] = _MC6847_RGBA(141, 150, 154);    /* buff */
    vdg->palette[5] = _MC6847_RGBA(15, 143, 155);     /* cyan */
    vdg->palette[6] = _MC6847_RGBA(139, 39, 155);     /* cyan */
    vdg->palette[7] = _MC6847_RGBA(140, 31, 11);      /* orange */

    /* black level color, and alpha-numeric display mode colors */
    vdg->black = 0xFF111111;
    vdg->alnum_green = _MC6847_RGBA(19, 146, 11);
    vdg->alnum_dark_green = 0xFF002400;
    vdg->alnum_orange = _MC6847_RGBA(140, 31, 11);
    vdg->alnum_dark_orange = 0xFF000E22;
}

void mc6847_reset(mc6847_t* vdg) {
    CHIPS_ASSERT(vdg);
    vdg->h_count = 0;
    vdg->l_count = 0;
}

/*
    internal character ROM dump from MAME
    (ntsc_square_fontdata8x12 in devices/video/mc6847.cpp)
*/
static const uint8_t _mc6847_font[64 * 12] = {
    0x00, 0x00, 0x00, 0x1C, 0x22, 0x02, 0x1A, 0x2A, 0x2A, 0x1C, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x08, 0x14, 0x22, 0x22, 0x3E, 0x22, 0x22, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x3C, 0x12, 0x12, 0x1C, 0x12, 0x12, 0x3C, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x1C, 0x22, 0x20, 0x20, 0x20, 0x22, 0x1C, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x3C, 0x12, 0x12, 0x12, 0x12, 0x12, 0x3C, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x3E, 0x20, 0x20, 0x38, 0x20, 0x20, 0x3E, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x3E, 0x20, 0x20, 0x38, 0x20, 0x20, 0x20, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x1E, 0x20, 0x20, 0x26, 0x22, 0x22, 0x1E, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x22, 0x22, 0x22, 0x3E, 0x22, 0x22, 0x22, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x1C, 0x08, 0x08, 0x08, 0x08, 0x08, 0x1C, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x02, 0x02, 0x02, 0x02, 0x22, 0x22, 0x1C, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x22, 0x24, 0x28, 0x30, 0x28, 0x24, 0x22, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x3E, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x22, 0x36, 0x2A, 0x2A, 0x22, 0x22, 0x22, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x22, 0x32, 0x2A, 0x26, 0x22, 0x22, 0x22, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x3E, 0x22, 0x22, 0x22, 0x22, 0x22, 0x3E, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x3C, 0x22, 0x22, 0x3C, 0x20, 0x20, 0x20, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x1C, 0x22, 0x22, 0x22, 0x2A, 0x24, 0x1A, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x3C, 0x22, 0x22, 0x3C, 0x28, 0x24, 0x22, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x1C, 0x22, 0x10, 0x08, 0x04, 0x22, 0x1C, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x3E, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x1C, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x22, 0x22, 0x22, 0x14, 0x14, 0x08, 0x08, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x22, 0x22, 0x22, 0x2A, 0x2A, 0x36, 0x22, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x22, 0x22, 0x14, 0x08, 0x14, 0x22, 0x22, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x22, 0x22, 0x14, 0x08, 0x08, 0x08, 0x08, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x3E, 0x02, 0x04, 0x08, 0x10, 0x20, 0x3E, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x38, 0x20, 0x20, 0x20, 0x20, 0x20, 0x38, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x20, 0x20, 0x10, 0x08, 0x04, 0x02, 0x02, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x0E, 0x02, 0x02, 0x02, 0x02, 0x02, 0x0E, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x08, 0x1C, 0x2A, 0x08, 0x08, 0x08, 0x08, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x08, 0x10, 0x3E, 0x10, 0x08, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0x08, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x14, 0x14, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x14, 0x14, 0x36, 0x00, 0x36, 0x14, 0x14, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x08, 0x1E, 0x20, 0x1C, 0x02, 0x3C, 0x08, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x32, 0x32, 0x04, 0x08, 0x10, 0x26, 0x26, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x10, 0x28, 0x28, 0x10, 0x2A, 0x24, 0x1A, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x08, 0x10, 0x20, 0x20, 0x20, 0x10, 0x08, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x08, 0x04, 0x02, 0x02, 0x02, 0x04, 0x08, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x08, 0x1C, 0x3E, 0x1C, 0x08, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x08, 0x08, 0x3E, 0x08, 0x08, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x10, 0x20, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x02, 0x02, 0x04, 0x08, 0x10, 0x20, 0x20, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x18, 0x24, 0x24, 0x24, 0x24, 0x24, 0x18, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x08, 0x18, 0x08, 0x08, 0x08, 0x08, 0x1C, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x1C, 0x22, 0x02, 0x1C, 0x20, 0x20, 0x3E, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x1C, 0x22, 0x02, 0x04, 0x02, 0x22, 0x1C, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x04, 0x0C, 0x14, 0x3E, 0x04, 0x04, 0x04, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x3E, 0x20, 0x3C, 0x02, 0x02, 0x22, 0x1C, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x1C, 0x20, 0x20, 0x3C, 0x22, 0x22, 0x1C, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x3E, 0x02, 0x04, 0x08, 0x10, 0x20, 0x20, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x1C, 0x22, 0x22, 0x1C, 0x22, 0x22, 0x1C, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x1C, 0x22, 0x22, 0x1E, 0x02, 0x02, 0x1C, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x08, 0x10, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x04, 0x08, 0x10, 0x20, 0x10, 0x08, 0x04, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x3E, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x10, 0x08, 0x04, 0x02, 0x04, 0x08, 0x10, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x18, 0x24, 0x04, 0x08, 0x08, 0x00, 0x08, 0x00, 0x00,
};


static inline uint32_t _mc6847_border_color(mc6847_t* vdg, uint64_t pins) {
    if (pins & MC6847_AG) {
        /* a graphics mode, either green or buff, depending on CSS pin */
        return (pins & MC6847_CSS) ? vdg->palette[4] : vdg->palette[0];
    }
    else {
        /* alphanumeric or semigraphics mode, always black */
        return vdg->black;
    }
}

static void _mc6847_decode_border(mc6847_t* vdg, uint64_t pins, int y) {
    uint32_t* dst = &(vdg->rgba8_buffer[y * MC6847_DISPLAY_WIDTH]);
    uint32_t c = _mc6847_border_color(vdg, pins);
    for (int x = 0; x < MC6847_DISPLAY_WIDTH; x++) {
        *dst++ = c;
    }
}

static uint64_t _mc6847_decode_scanline(mc6847_t* vdg, uint64_t pins, int y) {
    uint32_t* dst = &(vdg->rgba8_buffer[(y+MC6847_TOP_BORDER_LINES) * MC6847_DISPLAY_WIDTH]);
    uint32_t bc = _mc6847_border_color(vdg, pins);
    void* ud = vdg->user_data;

    /* left border */
    for (int i = 0; i < MC6847_BORDER_PIXELS; i++) {
        *dst++ = bc;
    }

    /* visible scanline */
    if (pins & MC6847_AG) {
        /* one of the 8 graphics modes */
        uint8_t sub_mode = (uint8_t) ((pins & (MC6847_GM2|MC6847_GM1)) / MC6847_GM1);
        if (pins & MC6847_GM0) {
            /*  one of the 'resolution modes' (1 bit == 1 pixel block)
                GM2|GM1:
                    00:    RG1, 128x64, 16 bytes per row
                    01:    RG2, 128x96, 16 bytes per row
                    10:    RG3, 128x192, 16 bytes per row
                    11:    RG6, 256x192, 32 bytes per row
            */
            int dots_per_bit  = (sub_mode < 3) ? 2 : 1;
            int bytes_per_row = (sub_mode < 3) ? 16 : 32;
            int row_height = (pins & MC6847_GM2) ? 1 : (pins & MC6847_GM1) ? 2 : 3;
            uint16_t addr = (y / row_height) * bytes_per_row;
            uint32_t fg_color = (pins & MC6847_CSS) ? vdg->palette[4] : vdg->palette[0];
            for (int x = 0; x < bytes_per_row; x++) {
                MC6847_SET_ADDR(pins, addr++);
                pins = vdg->fetch_cb(pins, ud);
                uint8_t m = MC6847_GET_DATA(pins);
                for (int p = 7; p >= 0; p--) {
                    uint32_t c = ((m>>p) & 1) ? fg_color : vdg->black;
                    for (int d = 0; d < dots_per_bit; d++) {
                        *dst++ = c;
                    }
                }
            }
        }
        else {
            /*  one of the 'color modes' (2 bits per pixel == 4 colors, CSS select
                lower or upper half of palette)
                GM2|GM1:
                    00: CG1, 64x64, 16 bytes per row
                    01: CG2, 128x64, 32 bytes per row
                    10: CG3, 128x96, 32 bytes per row
                    11: CG6, 128x192, 32 bytes per row
            */
            uint32_t pal_offset = (pins & MC6847_CSS) ? 4 : 0;
            int dots_per_2bit = (sub_mode == 0) ? 4 : 2;
            int bytes_per_row = (sub_mode == 0) ? 16 : 32;
            int row_height = (pins & MC6847_GM2) ? ((pins & MC6847_GM1) ? 1 : 2) : 3;
            uint16_t addr = (y / row_height) * bytes_per_row;
            for (int x = 0; x < bytes_per_row; x++) {
                MC6847_SET_ADDR(pins, addr++);
                pins = vdg->fetch_cb(pins, ud);
                uint8_t m = MC6847_GET_DATA(pins);
                for (int p = 6; p >= 0; p -= 2) {
                    const uint32_t c = vdg->palette[((m>>p) & 3) + pal_offset];
                    for (int d = 0; d < dots_per_2bit; d++) {
                        *dst++ = c;
                    }
                }
            }
        }
    }
    else {
        /*  we're in alphanumeric/semigraphics mode, one cell is 8x12 pixels */

        /* the vidmem src address and offset into the font data */
        uint16_t addr = (y / 12) * 32;
        uint8_t m; /* the pixel bitmask */
        int chr_y = y % 12;
        /* bit shifters to extract a 2x2 or 2x3 semigraphics 2-bit stack */
        int shift_2x2 = (1 - (chr_y / 6))*2;
        int shift_2x3 = (2 - (chr_y / 4))*2;
        uint32_t alnum_fg = (pins & MC6847_CSS) ? vdg->alnum_orange : vdg->alnum_green;
        uint32_t alnum_bg = (pins & MC6847_CSS) ? vdg->alnum_dark_orange : vdg->alnum_dark_green;
        for (int x = 0; x < 32; x++) {
            MC6847_SET_ADDR(pins, addr++);
            pins = vdg->fetch_cb(pins, ud);
            uint8_t chr = MC6847_GET_DATA(pins);
            if (pins & MC6847_AS) {
                /* semigraphics mode */
                uint32_t fg_color;
                if (pins & MC6847_INTEXT) {
                    /*  2x3 semigraphics, 2 color sets at 4 colors (selected by CSS pin)
                        |C1|C0|L5|L4|L3|L2|L1|L0|
                    
                        +--+--+
                        |L5|L4|
                        +--+--+
                        |L3|L2|
                        +--+--+
                        |L1|L0|
                        +--+--+
                    */

                    /* extract the 2 horizontal bits from one of the 3 stacks */
                    m = (chr>>shift_2x3) & 3;
                    /* 2 bits of color, CSS bit selects upper or lower half of color palette */
                    fg_color = vdg->palette[((chr>>6)&3) + ((pins&MC6847_CSS)?4:0)];
                }
                else {
                    /*  2x2 semigraphics, 8 colors + black
                        |xx|C2|C1|C0|L3|L2|L1|L0|
                    
                        +--+--+
                        |L3|L2|
                        +--+--+
                        |L1|L0|
                        +--+--+
                    */

                    /* extract the 2 horizontal bits from the upper or lower stack */
                    m = (chr>>shift_2x2) & 3;
                    /* 3 color bits directly point into the color palette */
                    fg_color = vdg->palette[(chr>>4) & 7];
                }
                /* write the horizontal pixel blocks (2 blocks @ 4 pixel each) */
                for (int p = 1; p>=0; p--) {
                    uint32_t c = (m & (1<<p)) ? fg_color : vdg->black;
                    *dst++=c; *dst++=c; *dst++=c; *dst++=c;
                }
            }
            else {
                /*  alphanumeric mode
                    FIXME: INT_EXT (switch between internal and external font
                */
                uint8_t m = _mc6847_font[(chr&0x3F)*12 + chr_y];
                if (pins & MC6847_INV) {
                    m = ~m;
                }
                for (int p = 7; p >= 0; p--) {
                    *dst++ = m & (1<<p) ? alnum_fg : alnum_bg;
                }                
            }
        }
    }

    /* right border */
    for (int i = 0; i < MC6847_BORDER_PIXELS; i++) {
        *dst++ = bc;
    }

    return pins;
}

uint64_t mc6847_tick(mc6847_t* vdg, uint64_t pins) {

    /* output pins will be set each tick */
    pins &= ~(MC6847_HS|MC6847_FS);

    /* horizontal and vertical field sync */
    vdg->h_count += MC6847_FIXEDPOINT_SCALE;
    if ((vdg->h_count >= vdg->h_sync_start) && (vdg->h_count < vdg->h_sync_end)) {
        /* horizontal sync on */
        pins |= MC6847_HS;
        if (vdg->l_count == MC6847_FSYNC_START) {
            /* switch field sync on */
            vdg->fs = true;
        }
    }
    if (vdg->fs) {
        pins |= MC6847_FS;
    }

    /* rewind horizontal counter? */
    if (vdg->h_count >= vdg->h_period) {
        vdg->h_count -= vdg->h_period;
        vdg->l_count++;
        if (vdg->l_count >= MC6847_ALL_LINES) {
            /* rewind line counter, field sync off */
            vdg->l_count = 0;
            vdg->fs = false;
        }
        if (vdg->l_count < MC6847_VBLANK_LINES) {
            /* inside vblank area, nothing to do */
        }
        else if (vdg->l_count < MC6847_DISPLAY_START) {
            /* top border */
            int y = vdg->l_count - MC6847_VBLANK_LINES;
            _mc6847_decode_border(vdg, pins, y);
        }
        else if (vdg->l_count < MC6847_DISPLAY_END) {
            /* visible area */
            int y = vdg->l_count - MC6847_DISPLAY_START;
            pins = _mc6847_decode_scanline(vdg, pins, y);
        }
        else if (vdg->l_count < MC6847_BOTTOM_BORDER_END) {
            /* bottom border */
            int y = vdg->l_count - MC6847_VBLANK_LINES;
            _mc6847_decode_border(vdg, pins, y);
        }
    }
    vdg->pins = pins;
    return pins;
}

# endif /* CHIPS_IMPL */
