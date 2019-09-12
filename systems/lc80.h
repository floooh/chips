#pragma once
/*#
    # lc80.h

    LC80 emulator in a C header.

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

    You need to include the following headers before including lc80.h:

    - chips/z80.h
    - chips/z80ctc.h
    - chips/z80pio.h
    - chips/beeper.h
    - chips/kbd.h
    - chips/mem.h
    - chips/clk.h

    ## The LC80

    The LC80 (**L**ern **C**omputer 80, Learning Computer 80) was an East
    German Z80 based computer used for education and simple controller tasks.

    It didn't have a proper keyboard and couldn't be attached to a TV,
    instead it had a pocket-calculator-derived mini-keyboard for hexadecimal
    numbers and a 6-digit LED display planted right on the open motherboard.

    The hardware looks like this:

    - 1x U880 CPU @ 900 kHz (equiv Z80)
    - 2x U855 (equiv Z80-PIO)
    - 1x U857 (equiv Z80-CTC)
    - 2x U505D (8kBit / 1kByte ROM == 2 KByte ROM)
    - 2..8x U214D (1024 x 4 bit SRAM, only default config of 1KB RAM emulated)
    - 0..3x K573RF2 (Soviet 2KByte EPROM, equiv Intel 2716, not emulated)
    - 2x DS8205 3-to-8 decoder used for address decoding (equiv LS138)
    - 3x VQE23 2-digits LED display block (equiv ???)

    TODO: more details about the hardware and emulator
        
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

/* audio output callback */
typedef void (*lc80_audio_callback)(const float* samples, int num_samples, void* user_data);

#define LC80_MAX_AUDIO_SAMPLES (1024)
#define LC80_DEFAULT_AUDIO_SAMPLES (128)

/* U505D ROM chip pins */
#define LC80_U505_A0     (1<<0)
#define LC80_U505_A1     (1<<1)
#define LC80_U505_A2     (1<<2)
#define LC80_U505_A3     (1<<3)
#define LC80_U505_A4     (1<<4)
#define LC80_U505_A5     (1<<5)
#define LC80_U505_A6     (1<<6)
#define LC80_U505_A7     (1<<7)
#define LC80_U505_A8     (1<<8)
#define LC80_U505_A9     (1<<9)
#define LC80_U505_A10    (1<<10)
#define LC80_U505_D0     (1<<16)
#define LC80_U505_D1     (1<<17)
#define LC80_U505_D2     (1<<18)
#define LC80_U505_D3     (1<<19)
#define LC80_U505_D4     (1<<20)
#define LC80_U505_D5     (1<<21)
#define LC80_U505_D6     (1<<22)
#define LC80_U505_D7     (1<<23)
#define LC80_U505_CS     (1<<24)

/* U214 ROM chip pins */
#define LC80_U214_A0     (1<<0)
#define LC80_U214_A1     (1<<1)
#define LC80_U214_A2     (1<<2)
#define LC80_U214_A3     (1<<3)
#define LC80_U214_A4     (1<<4)
#define LC80_U214_A5     (1<<5)
#define LC80_U214_A6     (1<<6)
#define LC80_U214_A7     (1<<7)
#define LC80_U214_A8     (1<<8)
#define LC80_U214_A9     (1<<9)
#define LC80_U214_D0     (1<<16)
#define LC80_U214_D1     (1<<17)
#define LC80_U214_D2     (1<<18)
#define LC80_U214_D3     (1<<19)
#define LC80_U214_CS     (1<<24)
#define LC80_U214_WR     (1<<28)     /* same as Z80_WR */

/* DS8205 / LS138 3-to-8 decoder pins */
#define LC80_DS8205_Y0   (1<<0)      /* output pins */
#define LC80_DS8205_Y1   (1<<1)
#define LC80_DS8205_Y2   (1<<2)
#define LC80_DS8205_Y3   (1<<3)
#define LC80_DS8205_Y4   (1<<4)
#define LC80_DS8205_Y5   (1<<5)
#define LC80_DS8205_Y6   (1<<6)
#define LC80_DS8205_Y7   (1<<7)
#define LC80_DS8205_A    (1<<8)      /* select inputs */
#define LC80_DS8205_B    (1<<9)
#define LC80_DS8205_C    (1<<10)
#define LC80_DS8205_G1   (1<<11)     /* enable inputs */
#define LC80_DS8205_G2A  (1<<12)
#define LC80_DS8205_G2B  (1<<13)

/* VQE23 2-digit LED state */
#define VQE23_A1    (1<<0)
#define VQE23_B1    (1<<1)
#define VQE23_C1    (1<<2)
#define VQE23_D1    (1<<3)
#define VQE23_E1    (1<<4)
#define VQE23_F1    (1<<5)
#define VQE23_G1    (1<<6)
#define VQE23_dP1   (1<<7)
#define VQE23_A2    (1<<8)
#define VQE23_B2    (1<<9)
#define VQE23_C2    (1<<10)
#define VQE23_D2    (1<<11)
#define VQE23_E2    (1<<12)
#define VQE23_F2    (1<<13)
#define VQE23_G2    (1<<14)
#define VQE23_dP2   (1<<15)
#define VQE23_K1    (1<<16)
#define VQE23_K2    (1<<17)

/* config parameters for lc80_init() */
typedef struct {
    /* optional userdata pointer for callbacks */
    void* user_data;

    /* audio output config (if you don't want audio, set audio_cb to zero) */
    lc80_audio_callback_t audio_cb;     /* called when audio_num_samples are ready */
    int audio_num_samples;              /* default is LC80_DEFAULT_AUDIO_SAMPLES */
    int audio_sample_rate;              /* playback sample rate, default is 44100 */
    float audio_volume;                 /* audio volume (0.0 .. 1.0), default is 0.4 */

    /* ROM image (must be single 2KByte image) */
    const void* rom_ptr;
    int rom_size;
} lc80_desc_t;

/* LC80 emulator state */
typedef struct {
    bool valid;

    z80_t cpu;
    z80ctc_t ctc;
    z80pio_t pio_sys;
    z80pio_t pio_usr;
    uint32_t led[3];            /* state of the 2-digit LED modules */
    uint32_t u505[2];           /* pin state of the 2 U505D ROM chips */
    uint32_t u214[2];           /* pin state of the 2 U214D RAM chips */
    uint32_t ds8205[2];         /* pin state of the 2 DS8205 3-to-8 decoders (equiv LS138) */

    beeper_t beeper;
    clk_t clk;
    kbd_t kbd;
    mem_t mem;
    
    void* user_data;
    lc80_audio_callback audio_cb;
    int num_samples;
    int sample_pos;
    float sample_buffer[LC80_MAX_AUDIO_SAMPLES];

    uint8_t ram[0x0400];
    uint8_t rom[0x0800];
} lc80_t;

void lc80_init(lc80_t* sys, const lc80_desc_t* desc);
void lc80_discard(lc80_t* sys);
void lc80_reset(lc80_t* sys);
void lc80_exec(lc80_t* sys, uint32_t micro_seconds);
void lc80_keydown(lc80_t* sys, int key_code);
void lc80_keyup(lc80_t* sys, int key_code);

#ifdef __cplusplus
} /* extern "C" */
#endif

/*-- IMPLEMENTATION ----------------------------------------------------------*/
#ifdef CHIPS_IMPL
#include <string.h> /* memcpy, memset */
#ifndef CHIPS_ASSERT
    #include <assert.h>
    #define CHIPS_ASSERT(c) assert(c)
#endif

#endif /* CHIPS_IMPL */