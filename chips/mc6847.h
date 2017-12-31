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
            RP <--|           |--> A12
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

    LICENSE:

    MIT License

    Copyright (c) 2017 Andre Weissflog

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
#define MC6847_RP   (1ULL<<42)      /* row preset */

/* mode-select input pins */
#define MC6847_AG       (1ULL<<43)      /* graphics mode enable */
#define MC6847_AS       (1ULL<<44)      /* semi-graphics mode enable */
#define MC6847_INTEXT   (1ULL<<45)      /* internal/external select */
#define MC6847_INV      (1ULL<<46)      /* invert enable */
#define MC6847_GM0      (1ULL<<47)      /* graphics mode select 0 */
#define MC6847_GM1      (1ULL<<48)      /* graphics mode select 1 */
#define MC6847_GM2      (1ULL<<49)      /* graphics mode select 2 */
#define MC6847_CSS      (1ULL<<50)      /* color select pin */

/* public constant */
#define MC6847_VBLANK_LINES         (13)    /* 13 lines vblank at top of screen */
#define MC6847_TOP_BORDER_LINES     (25)    /* 25 lines top border */
#define MC6847_DISPLAY_LINES        (192)   /* 192 lines visible display area */
#define MC6847_BOTTOM_BORDER_LINES  (26)    /* 26 lines bottom border */
#define MC6847_VRETRACE_LINES       (6)     /* 6 'lines' for vertical retrace */
#define MC6847_ALL_LINES            (262)   /* all of the above */
#define MC6847_DISPLAY_START        (MC6847_LINES_VBLANK+MC6847_LINES_BORDER_TOP)
#define MC6847_DISPLAY_END          (MC6847_LINES_DISP_START+MC6847_LINES_DISPLAY)
#define MC6847_BOTTOM_BORDER_END    (MC6847_LINES_DISPLAY_END+MC6847_LINES_BORDER_BOT)
#define MC6847_FSYNC_START          (MC6847_DISPLAY_END)

/* pixel width and height of entire visible area, including border */
#define MC6847_DISPLAY_WIDTH (320)
#define MC6847_DISPLAY_HEIGHT (MC6847_TOP_BORDER_LINES+MC6847_DISPLAY_LINES+MC6847_BOTTOM_BORDER_LINES)

/* pixel width and height of only the image area, without border */
#define MC6847_IMAGE_WIDTH (256)
#define MC6847_IMAGE_HEIGHT (192)

/* the MC6847 is always clocked at 3.579 MHz */
#define MC6847_TICK_HZ (3579545)

/* a memory-fetch callback, used to read video memory bytes into the MC6847 */
typedef uint64_t (*mc6847_fetch_t)(uint64_t pins);

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
} mc6847_desc_t;

/* the mc6847 state struct */
typedef struct {
    /* current pin state */
    uint64_t pins;
    /* pins that transitioned from inactive-to-active during last tick */
    uint64_t on;
    /* pins that transitioned from active-to-inactive during last tick */
    uint64_t off;
    /* the graphics mode color palette (RGBA8) */
    uint32_t gm_colors[8];
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

    /* the fetch callback function */
    mc6847_fetch_t fetch_cb;
    /* pointer to RGBA8 buffer where decoded video image is written too */
    uint32_t* rgba8_buffer;
} mc6847_t;

/* initialize a new mc6847_t instance */
extern void mc6847_init(mc6847_t* vdg, mc6847_desc_t* desc);
/* reset a mc6847_t instance */
extern void mc6847_reset(mc6847_t* vdg);
/* reprogram the mode-select pins */
extern void mc6847_iorq(mc6847_t* vdg, uint64_t pins);
/* tick the mc6847_t instance, this will call the fetch_cb and generate the image */
extern void mc6847_tick(mc6847_t* vdg);

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

#define _MC6847_PRECISION_BOOST (16)
#define _MC6847_CLAMP(x) ((x)>255?255:(x))
#define _MC6847_RGBA(r,g,b) (0xFF000000|_MC6847_CLAMP((r*4)/3)|(_MC6847_CLAMP((g*4)/3)<<8)|(_MC6847_CLAMP((b*4)/3)<<16))

void mc6847_init(mc6847_t* vdg, mc6847_desc_t* desc) {
    CHIPS_ASSERT(vdg && desc);
    CHIPS_ASSERT(desc->rgba8_buffer);
    CHIPS_ASSERT(desc->rgba8_buffer_size >= (MC6847_DISPLAY_WIDTH*MC6847_DISPLAY_HEIGHT*sizeof(uint32_t)));
    CHIPS_ASSERT(desc->fetch_cb);
    CHIPS_ASSERT((desc->tick_hz > 0) && (desc->tick_hz < MC6847_TICK_HZ));

    memset(vdg, 0, sizeof(*vdg));
    vdg->rgba8_buffer = desc->rgba8_buffer;
    vdg->fetch_cb = desc->fetch_cb;

    /* compute counter periods, the MC6847 is always clocked at 3.579 MHz,
       and the frequency of how the tick function is called must be 
       communicated to the init function

       one scanline is 228 3.5 MC6847 ticks
    */
    int64_t tmp = (228LL * desc->tick_hz * _MC6847_PRECISION_BOOST) / MC6847_TICK_HZ;
    vdg->h_period = (int) tmp;
    /* hsync starts at tick 10 of a scanline */
    tmp = (10LL * desc->tick_hz * _MC6847_PRECISION_BOOST) / MC6847_TICK_HZ;
    vdg->h_sync_start = tmp;
    /* hsync is 16 ticks long */
    tmp = (26LL * desc->tick_hz * _MC6847_PRECISION_BOOST) / MC6847_TICK_HZ;
    vdg->h_sync_end = tmp;

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
    vdg->gm_colors[0] = _MC6847_RGBA(19, 146, 11);      /* green */
    vdg->gm_colors[1] = _MC6847_RGBA(155, 150, 10);     /* yellow */
    vdg->gm_colors[2] = _MC6847_RGBA(2, 22, 175);       /* blue */
    vdg->gm_colors[3] = _MC6847_RGBA(155, 22, 7);       /* red */
    vdg->gm_colors[4] = _MC6847_RGBA(141, 150, 154);    /* buff */
    vdg->gm_colors[5] = _MC6847_RGBA(15, 143, 155);     /* cyan */
    vdg->gm_colors[6] = _MC6847_RGBA(139, 39, 155);     /* cyan */
    vdg->gm_colors[7] = _MC6847_RGBA(140, 31, 11);      /* orange */

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

# endif /* CHIPS_IMPL */

#ifdef __cplusplus
} /* extern "C" */
#endif
