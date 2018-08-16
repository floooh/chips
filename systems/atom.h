#pragma once
/*#
    # atom.h

    Acorn Atom emulator in a C header.

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

    - chips/m6502.h
    - chips/mc6847.h
    - chips/i8255.h
    - chips/m6522.h
    - chips/beeper.h
    - chips/mem.h
    - chips/kbd.h
    - chips/clk.h

    ## The Acorn Atom

    FIXME!

    ## TODO

    - VIA emulation is currently only minimal
    - handle shift key (some games use this as jump button)
    - AtomMMC is very incomplete (only what's needed for joystick)

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

#define ATOM_DISPLAY_WIDTH (MC6847_DISPLAY_WIDTH)
#define ATOM_DISPLAY_HEIGHT (MC6847_DISPLAY_HEIGHT)
#define ATOM_MAX_AUDIO_SAMPLES (1024)      /* max number of audio samples in internal sample buffer */
#define ATOM_DEFAULT_AUDIO_SAMPLES (128)   /* default number of samples in internal sample buffer */ 

/* joystick emulation types */
typedef enum {
    ATOM_JOYSTICK_NONE,
    ATOM_JOYSTICK_MMC
} atom_joystick_t;

/* audio sample data callback */
typedef int (*atom_audio_callback_t)(const float* samples, int num_samples);
/* max number of audio samples in internal sample buffer */
#define ATOM_MAX_AUDIO_SAMPLES (1024)
/* default number of audio samples to generate until audio callback is invoked */
#define ATOM_DEFAULT_AUDIO_SAMPLES (128)

/* configuration parameters for atom_init() */
typedef struct {
    atom_joystick_t joystick_type;  /* what joystick type to emulate, default is ATOM_JOYSTICK_NONE */

    /* video output config */
    void* pixel_buffer;         /* pointer to a linear RGBA8 pixel buffer, at least 320*256*4 bytes */
    int pixel_buffer_size;      /* size of the pixel buffer in bytes */

    /* audio output config (if you don't want audio, set audio_cb to zero) */
    atom_audio_callback_t audio_cb;   /* called when audio_num_samples are ready */
    int audio_num_samples;          /* default is ZX_AUDIO_NUM_SAMPLES */
    int audio_sample_rate;          /* playback sample rate, default is 44100 */
    float audio_volume;             /* audio volume: 0.0..1.0, default is 0.25 */

    /* ROM images */
    const void* rom_abasic;
    const void* rom_afloat;
    const void* rom_dosrom;
    int rom_abasic_size;
    int rom_afloat_size;
    int rom_dosrom_size;
} atom_desc_t;

/* Acorn Atom emulation state */
typedef struct {
    m6502_t cpu;
    mc6847_t vdg;
    i8255_t ppi;
    m6522_t via;
    int counter_2_4khz;
    int period_2_4khz;
    bool state_2_4khz;
    bool out_cass0;
    bool out_cass1;
    uint8_t mmc_joymask;
    uint8_t mmc_cmd;
    uint8_t mmc_latch;
    clk_t clk;
    mem_t mem;
    kbd_t kbd;
    uint32_t pixel_buffer;
    const void* rom_abasic;
    const void* rom_afloat;
    const void* rom_dosrom;
    atom_audio_callback_t audio_cb;
    int num_samples;
    int sample_pos;
    float sample_buffer[ATOM_MAX_AUDIO_SAMPLES];
    uint8_t ram[1<<16];
} atom_t;

/* initialize a new Atom instance */
void atom_init(atom_t* sys, const atom_desc_t* desc);
/* discard Atom instance */
void atom_discard(atom_t* sys);
/* reset Atom instance */
void atom_reset(atom_t* sys);
/* run Atom instance for a given time in seconds */
void atom_exec(atom_t* sys, double seconds);
/* send a key down event */
void atom_key_down(atom_t* sys, int key_code);
/* send a key up event */
void atom_key_up(atom_t* sys, int key_code);
/* load an Atom TAP file */
void atom_quickload(atom_t* sys, const uint8_t* ptr, int num_bytes);

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

#define _ATOM_DISPLAY_SIZE (ATOM_DISPLAY_WIDTH*ATOM_DISPLAY_HEIGHT*4)
#define _ATOM_FREQUENCY (1000000)
#define _ATOM_ROM_ABASIC_SIZE (0x2000)
#define _ATOM_ROM_AFLOAT_SIZE (0x1000)
#define _ATOM_ROM_DOSROM_SIZE (0x1000)

static uint64_t _atom_tick(uint64_t pins, void* user_data);
static uint64_t _atom_vdg_fetch(uint64_t pins, void* user_data);
static uint8_t _atom_ppi_in(int port_id, void* user_data);
static uint64_t _atom_ppi_out(int port_id, uint64_t pins, uint8_t data, void* user_data);
static uint8_t _atom_via_in(int port_id, void* user_data);
static void _atom_via_out(int port_id, uint8_t data, void* user_data);

#endif /* CHIPS_IMPL */
