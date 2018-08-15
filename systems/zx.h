#pragma once
/*#
    # zx.h

    A ZX Spectrum 48K / 128 emulator in a C header.

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

    You need to include the following headers before including z1013.h:

    - chips/z80.h
    - chips/beeper.h
    - chips/ay38910.h
    - chips/mem.h
    - chips/kbd.h
    - chips/clk.h

    ## The ZX Spectrum 48K

    TODO!

    ## The ZX Spectrum 128

    TODO! 

    ## TODO:
    - wait states when CPU accesses 'contended memory' and IO ports
    - reads from port 0xFF must return 'current VRAM bytes
    - video decoding only has scanline accuracy, not pixel accuracy

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

/* display width/height in pixels */
#define ZX_DISPLAY_WIDTH (320)
#define ZX_DISPLAY_HEIGHT (256)

/* ZX Spectrum models */
typedef enum {
    ZX_TYPE_48K,
    ZX_TYPE_128,
} zx_type_t;

/* ZX Spectrum joystick types */
typedef enum {
    ZX_JOYSTICK_NONE,
    ZX_JOYSTICK_KEMPSTON,
    ZX_JOYSTICK_SINCLAIR_1,
    ZX_JOYSTICK_SINCLAIR_2,
} zx_joystick_t;

/* audio sample data callback */
typedef int (*zx_audio_callback_t)(const float* samples, int num_samples);

/* max number of audio samples in internal sample buffer */
#define ZX_MAX_AUDIO_SAMPLES (1024)
/* default number of audio samples to generate until audio callback is invoked */
#define ZX_DEFAULT_AUDIO_SAMPLES (128)

/* config parameters for zx_init() */
typedef struct {
    zx_type_t type;                 /* default is ZX_TYPE_48K */
    zx_joystick_t joystick_type;    /* what joystick to emulate, default is ZX_JOYSTICK_NONE */

    /* video output config */
    void* pixel_buffer;         /* pointer to a linear RGBA8 pixel buffer, at least 320*256*4 bytes */
    int pixel_buffer_size;      /* size of the pixel buffer in bytes */

    /* audio output config */
    zx_audio_callback_t audio_cb;   /* called when audio_num_samples are ready */
    int audio_num_samples;          /* default is ZX_AUDIO_NUM_SAMPLES */
    int audio_sample_rate;          /* playback sample rate, default is 44100 */
    float audio_sample_magnitude;   /* magnitude/volume of audio samples: 0.0..1.0, default is 0.5 */

    /* ROMs for ZX Spectrum 48K */
    const void* rom_zx48k;
    int rom_zx48k_size;

    /* ROMs for ZX Spectrum 128 */
    const void* rom_zx128_0;
    const void* rom_zx128_1;
    int rom_zx128_0_size;
    int rom_zx128_1_size;
} zx_desc_t;

/* ZX emulator state */
typedef struct {
    z80_t cpu;
    beeper_t beeper;
    ay38910_t ay;
    clk_t clk;
    bool valid;
    bool memory_paging_disabled;
    zx_joystick_type_t joystick_type;
    uint8_t kempston_mask;
    uint32_t tick_count;
    uint8_t last_fe_out;            // last out value to 0xFE port */
    uint8_t blink_counter;          // incremented on each vblank
    int frame_scan_lines;
    int top_border_scanlines;
    int scanline_period;
    int scanline_counter;
    int scanline_y;
    uint32_t display_ram_bank;
    uint32_t border_color;
    kbd_t kbd;
    mem_t mem;
    uint32_t pixel_buffer;
    const void* rom_zx48k;
    const void* rom_zx128_0;
    const void* rom_zx128_1;
    z9001_audio_callback_t audio_cb;
    int num_samples;
    int sample_pos;
    float sample_buffer[Z9001_MAX_AUDIO_SAMPLES];
    uint8_t ram[8][0x4000];
    uint8_t junk[0x4000];
} zx_t;

/* initialize a new ZX Spectrum instance */
extern void zx_init(zx_t* sys, const zx_desc_t* desc);
/* discard a ZX Spectrum instance */
extern void zx_discard(zx_t* sys);
/* reset a ZX Spectrum instance */
extern void zx_reset(zx_t* sys);
/* run ZX Spectrum instance for a given time in seconds */
extern void zx_exec(zx_t* sys, double seconds);
/* send a key-down event */
extern void zx_key_down(zx_t* sys, int key_code);
/* send a key-up event */
extern void zx_key_up(zx_t* sys, int key_code);
/* load a ZX Z80 file into the emulator */
extern bool zx_quickload(zx_t* sys, const uint8_t* ptr, int num_bytes); 

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

#define _ZX_DISPLAY_SIZE (ZX_DISPLAY_WIDTH*ZX_DISPLAY_HEIGHT*4)
#define _ZX_48K_FREQUENCY (3500000)
#define _ZX_128K_FREQUENCY (3546894)

static uint64_t _zx_tick(int num, uint64_t pins, void* user_data);
static bool zx_decode_scanline(zx_t* sys);

#endif /* CHIPS_IMPL */
