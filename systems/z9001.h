#pragma once
/*#
    # z9001.h

    A Robotron Z9001/KC87 emulator in a C header.

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
    - chips/z80pio.h
    - chips/z80ctc.h
    - chips/mem.h
    - chips/kbd.h
    - chips/clk.h
  
    ## The Robotron Z9001 / KC87

    FIXME!

    ## NOT EMULATED:
    - border color
    - 40x20 video mode

    ## Reference Info

    - schematics: http://www.sax.de/~zander/kc/kcsch_1.pdf
    - manual: http://www.sax.de/~zander/z9001/doku/z9_fub.pdf

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
#define Z9001_DISPLAY_WIDTH (320)
#define Z9001_DISPLAY_HEIGHT (192)

/* Z9001/KC87 model types */
typedef enum {
    Z9001_TYPE_Z9001,   /* the original Z9001 (default) */
    Z9001_TYPE_KC87,    /* the revised KC87 with built-in BASIC and color module */
} z9001_type_t;

/* Z9001 audio sample data callback */
typedef void (*z9001_audio_callback_t)(const float* samples, int num_samples);

/* max number of audio samples in internal sample buffer */
#define Z9001_MAX_AUDIO_SAMPLES (1024)
/* default number of audio samples to generate until audio callback is invoked */
#define Z9001_DEFAULT_AUDIO_SAMPLES (128)

/* configuration parameters for z9001_init() */
typedef struct {
    z9001_type_t type;                  /* default is Z9001_TYPE_Z9001 */
    z9001_audio_callback_t* audio_cb;   /* called when audio_num_samples are ready */
    int audio_num_samples;              /* default is Z9001_DEFAULT_AUDIO_SAMPLES */
    /* ROMs for Z9001 */
    const void* rom_z9001_os_1;
    const void* rom_z9001_os_2;
    const void* rom_z9001_font;
    int rom_z9001_os_1_size;
    int rom_z9001_os_2_size;
    int rom_z9001_font_size;
    /* optional BASIC module for z9001 */
    const void* rom_z9001_basic;
    int rom_z9001_basic_size;
    /* ROMs for KC87 */
    const void* rom_kc87_os;
    const void* rom_kc87_basic;
    const void* rom_kc87_font;
    int rom_kc87_os_size;
    int rom_kc87_basic_size;
    int rom_kc87_font_size;
    /* output pixel buffer (must be at least 320*192*4 bytes) */
    void* pixel_buffer;
    int pixel_buffer_size;
} z9001_desc_t;

/* Z9001 emulator state */
typedef struct {
    z80_t cpu;
    z80pio_t pio1;
    z80pio_t pio2;
    z80ctc_t ctc;
    clk_t clk;
    bool valid;
    z9001_type_t type;
    uint64_t ctc_zcto2;     /* pin mask to store state of CTC ZCTO2 */
    uint32_t blink_counter;
    bool blink_flip_flop;
    uint8_t border_color;
    mem_t mem;
    kbd_t kbd;
    uint32_t* pixel_buffer;
    const void* rom_z9001_os_1;
    const void* rom_z9001_os_2;
    const void* rom_z9001_basic;
    const void* rom_z9001_font;
    const void* rom_kc87_os;
    const void* rom_kc87_basic;
    const void* rom_kc87_font;
    z9001_audio_callback_t audio_cb;
    int num_samples;
    int sample_pos;
    float sample_buffer[Z9001_MAX_AUDIO_SAMPLES];
    uint8_t ram[1<<16];
} z9001_t;

/* initialize a new Z9001 instance */
extern void z9001_init(z9001_t* sys, const z9001_desc_t* desc);
/* discard a Z9001 instance */
extern void z9001_discard(z9001_t* sys);
/* reset Z9001 instance */
extern void z9001_reset(z9001_t* sys);
/* run Z9001 instance for a given time in seconds */
extern void z9001_exec(z9001_t* sys, double seconds);
/* send a key-down event */
extern void z9001_key_down(z9001_t* sys, int key_code);
/* send a key-up event */
extern void z9001_key_up(z9001_t* sys, int key_code);
/* load a KC TAP file into the emulator */
extern bool z9001_load_kctap(z9001_t* sys, const uint8_t* ptr, int num_bytes);
/* load a KCC file into the emulator */
extern bool z9001_load_kcc(z9001_t* sys, const uint8_t* ptr, int num_bytes);

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

#define _Z9001_DISPLAY_SIZE (Z9001_DISPLAY_WIDTH*Z9001_DISPLAY_HEIGHT*4)
#define _Z9001_FREQUENCY (2457600)
#define _Z9001_ROM_Z9001_OS_1_SIZE  (2048)
#define _Z9001_ROM_Z9001_OS_2_SIZE  (2048)
#define _Z9001_ROM_Z9001_BASIC_SIZE (10240)
#define _Z9001_ROM_Z9001_FONT_SIZE  (2048)
#define _Z9001_ROM_KC87_OS_SIZE     (8192)
#define _Z9001_ROM_KC87_BASIC_SIZE  (8192)
#define _Z9001_ROM_KC87_FONT_SIZE   (2048)

extern uint64_t _z9001_tick(int num, uint64_t pins, void* user_data);
extern uint8_t _z9001_pio1_in(int port_id, void* user_data);
extern void _z9001_pio1_out(int port_id, uint8_t data, void* user_data);
extern uint8_t _z9001_pio2_in(int port_id, void* user_data);
extern void _z9001_pio2_out(int port_id, uint8_t data, void* user_data);

static const uint32_t _z9001_palette[8] = {
    0xFF000000,     /* black */
    0xFF0000FF,     /* red */
    0xFF00FF00,     /* green */
    0xFF00FFFF,     /* yellow */
    0xFFFF0000,     /* blue */
    0xFFFF00FF,     /* purple */
    0xFFFFFF00,     /* cyan */
    0xFFFFFFFF,     /* white */
};

/* xorshift randomness for memory initialization */
static inline uint32_t _z9001_xorshift32(uint32_t x) {
    x ^= x<<13;
    x ^= x>>17;
    x ^= x<<5;
    return x;
}

void z9001_init(z9001_t* sys, z9001_desc_t* desc) {
    CHIPS_ASSERT(sys && desc);
    CHIPS_ASSERT(desc->audio_cb);
    CHIPS_ASSERT(desc->pixel_buffer && (desc->pixel_buffer_size >= _Z9001_DISPLAY_SIZE));
    if (desc->type == Z9001_TYPE_Z9001) {
        CHIPS_ASSERT(desc->rom_z9001_os_1 && (desc->rom_z9001_os_1_size == _Z9001_ROM_Z9001_OS_1_SIZE));
        CHIPS_ASSERT(desc->rom_z9001_os_2 && (desc->rom_z9001_os_2_size == _Z9001_ROM_Z9001_OS_2_SIZE));
        if (desc->rom_z9001_basic)) {
            CHIPS_ASSERT(desc->rom_z9001_basic_size == _Z9001_ROM_Z9001_BASIC_SIZE);
            CHIPS_ASSERT(desc->rom_z9001_font && (desc->rom_z9001_font_size == _Z9001_ROM_Z9001_FONT_SIZE));
        }
    }
    else {
        CHIPS_ASSERT(desc->rom_kc87_os && (desc->rom_kc87_os_size == _Z9001_ROM_KC87_OS_SIZE));
        CHIPS_ASSERT(desc->rom_kc87_basic && (desc->rom_kc87_basic_size == _Z9001_ROM_KC87_BASIC_SIZE));
        CHIPS_ASSERT(desc->rom_kc87_font && (desc->rom_kc87_font_size == _Z9001_ROM_KC87_FONT_SIZE));
    }

    memset(sys, 0, sizeof(z9001_t));
    sys->valid = true;
    sys->type = desc->type;
    if (desc->type == Z9001_TYPE_Z9001) {
        sys->rom_z9001_os_1 = desc->rom_z9001_os_1;
        sys->rom_z9001_os_2 = desc->rom_z9001_os_2;
        sys->rom_z9001_basic = desc->rom_z9001_basic;
        sys->rom_z9001_font = desc->rom_z9001_font;
    }
    else {
        sys->rom_kc87_os = desc->rom_kc87_os;
        sys->rom_kc87_basic = desc->rom_kc87_basic;
        sys->rom_kc87_font = desc->rom_kc87_font;
    }
    sys->pixel_buffer = desc->pixel_buffer;
    sys->audio_cb = desc->audio_cb;
    sys->num_samples = (desc->audio_num_samples > 0) ? desc->audio_num_samples : Z9001_DEFAULT_AUDIO_SAMPLES;
    CHIPS_ASSERT(sys->num_samples <= Z9001_MAX_AUDIO_SAMPLES);

    /* initialize the hardware */
    clk_init(&sys->clk, _Z9001_FREQUENCY);
    z80ctc_init(&sys->ctc);

    z80_desc_t cpu_desc = {0};
    cpu_desc.tick_cb = _z9001_tick;
    cpu_desc.user_data = sys;
    z80_init(&sys->cpu, &cpu_desc);
    
    z80pio_desc_t pio1_desc = {0};
    pio1_desc.in_cb = _z9001_pio1_in;
    pio1_desc.out_cb = _z9001_pio1_out;
    pio1_desc.user_data = sys;
    z80pio_init(&sys->pio1, &pio1_desc);
    
    z80pio_desc_t pio2_desc = {0};
    pio2_desc.in_cb = _z9001_pio2_in;
    pio2_desc.out_cb = _z9001_pio2_out;
    pio2_desc.user_data = sys;
    z80pio_init(&sys->pio2, &pio2_desc);
    
    /* execution starts at 0xF000 */
    z80_set_pc(&sys->cpu, 0xF000);

    /* fill memory with randomness */


    mem_t mem;
    kbd_t kbd;
    uint8_t ram[1<<16];
}
