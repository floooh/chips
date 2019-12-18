#pragma once
/*#
    # m6561.h

    MOS Technology 6561 emulator (aka VIC for PAL)

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
    TODO

    TODO: Documentation

    ## zlib/libpng license

    Copyright (c) 2019 Andre Weissflog
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

/* memory fetch callback, used to feed pixel- and color-data into the m6561 */
typedef uint16_t (*m6561_fetch_t)(uint16_t addr, void* user_data);

/* setup parameters for m6561_init() function */
typedef struct {
    /* pointer to RGBA8 framebuffer for generated image (optional) */
    uint32_t* rgba8_buffer;
    /* size of the RGBA framebuffer (optional) */
    uint32_t rgba8_buffer_size;
    /* visible CRT area blitted to rgba8_buffer (in pixels) */
    uint16_t vis_x, vis_y, vis_w, vis_h;
    /* the memory-fetch callback */
    m6561_fetch_t fetch_cb;
    /* optional user-data for fetch callback */
    void* user_data;
} m6561_desc_t;

/* CRT state tracking */
typedef struct {
    uint16_t x, y;              /* beam pos reset on crt_retrace_h/crt_retrace_v zero */
    uint16_t vis_x0, vis_y0, vis_x1, vis_y1;  /* the visible area */
    uint16_t vis_w, vis_h;      /* width of visible area */
    uint32_t* rgba8_buffer;
} m6561_crt_t;

/* the m6561_t state struct */
typedef struct {
    // FIXME
    bool debug_vis;
    m6561_crt_t crt;
    uint64_t pins;
} m6561_t;

/* initialize a new m6561_t instance */
void m6561_init(m6561_t* vic, const m6561_desc_t* desc);
/* reset a m6561_t instance */
void m6561_reset(m6561_t* vic);
/* get the visible display width in pixels */
int m6561_display_width(m6561_t* vic);
/* get the visible display height in pixels */
int m6561_display_height(m6561_t* vic);
/* read/write m6561 registers */
uint64_t m6561_iorq(m6561_t* vic, uint64_t pins);
/* tick the m6561 instance */
uint64_t m6561_tick(m6561_t* vic, uint64_t pins);
/* get 32-bit RGBA8 value from color index (0..15) */
uint32_t m6561_color(int i);

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

/* internal constants */
#define _M6561_HTOTAL   (71)
#define _M6561_VTOTAL   (312)

#define _M6561_RGBA8(r,g,b) (0xFF000000|(b<<16)|(g<<8)|(r))

/* this palette taken from MAME */
static const uint32_t _m6561_colors[16] = {
    _M6561_RGBA8(0x00, 0x00, 0x00),
    _M6561_RGBA8(0xff, 0xff, 0xff),
    _M6561_RGBA8(0xf0, 0x00, 0x00),
    _M6561_RGBA8(0x00, 0xf0, 0xf0),
    _M6561_RGBA8(0x60, 0x00, 0x60),
    _M6561_RGBA8(0x00, 0xa0, 0x00),
    _M6561_RGBA8(0x00, 0x00, 0xf0),
    _M6561_RGBA8(0xd0, 0xd0, 0x00),
    _M6561_RGBA8(0xc0, 0xa0, 0x00),
    _M6561_RGBA8(0xff, 0xa0, 0x00),
    _M6561_RGBA8(0xf0, 0x80, 0x80),
    _M6561_RGBA8(0x00, 0xff, 0xff),
    _M6561_RGBA8(0xff, 0x00, 0xff),
    _M6561_RGBA8(0x00, 0xff, 0x00),
    _M6561_RGBA8(0x00, 0xa0, 0xff),
    _M6561_RGBA8(0xff, 0xff, 0x00)
};

static void _m6561_init_crt(m6561_crt_t* crt, const m6561_desc_t* desc) {
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

void m6561_init(m6561_t* vic, const m6561_desc_t* desc) {
    CHIPS_ASSERT(vic && desc);
    CHIPS_ASSERT((0 == desc->rgba8_buffer) || (desc->rgba8_buffer_size >= (_M6561_HTOTAL*8*_M6561_VTOTAL*sizeof(uint32_t))));
    memset(vic, 0, sizeof(*vic));
    _m6561_init_crt(&vic->crt, desc);
}

static void _m6561_reset_crt(m6561_crt_t* c) {
    c->x = c->y = 0;
}

void m6561_reset(m6561_t* vic) {
    CHIPS_ASSERT(vic);
    _m6561_reset_crt(&vic->crt);
    // FIXME
}

int m6561_display_width(m6561_t* vic) {
    CHIPS_ASSERT(vic);
    return 8 * (vic->debug_vis ? _M6561_HTOTAL : vic->crt.vis_w);
}

int m6561_display_height(m6561_t* vic) {
    CHIPS_ASSERT(vic);
    return vic->debug_vis ? _M6561_VTOTAL : vic->crt.vis_h;
}

uint32_t m6561_color(int i) {
    CHIPS_ASSERT((i >= 0) && (i < 16));
    return _m6561_colors[i];
}

uint64_t m6561_iorq(m6561_t* vic, uint64_t pins) {
    // FIXME
    return pins;
}

uint64_t m6561_tick(m6561_t* vic, uint64_t pins) {
    // FIXME
    return pins;
}

#endif
