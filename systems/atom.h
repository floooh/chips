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
    beeper_t beeper;
    bool valid;
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
    const uint8_t* rom_abasic;
    const uint8_t* rom_afloat;
    const uint8_t* rom_dosrom;
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
static void _atom_init_keymap(atom_t* sys);
static void _atom_init_memorymap(atom_t* sys);

#define _ATOM_DEFAULT(val,def) (((val) != 0) ? (val) : (def));

void atom_init(atom_t* sys, const atom_desc_t* desc) {
    CHIPS_ASSERT(sys && desc);
    CHIPS_ASSERT(desc->pixel_buffer && (desc->pixel_buffer_size >= _ATOM_DISPLAY_SIZE));
    CHIPS_ASSERT(desc->rom_abasic && (desc->rom_abasic_size == _ATOM_ROM_ABASIC_SIZE));
    CHIPS_ASSERT(desc->rom_afloat && (desc->rom_afloat_size == _ATOM_ROM_AFLOAT_SIZE));
    CHIPS_ASSERT(desc->rom_dosrom && (desc->rom_dosrom_size == _ATOM_ROM_DOSROM_SIZE));

    memset(sys, 0, sizeof(atom_t));
    sys->valid = true;
    sys->audio_cb = desc->audio_cb;
    sys->num_samples = _ATOM_DEFAULT(desc->audio_num_samples, ATOM_DEFAULT_AUDIO_SAMPLES);
    CHIPS_ASSERT(sys->num_samples <= ATOM_MAX_AUDIO_SAMPLES);
    sys->rom_abasic = (const uint8_t*) desc->rom_abasic;
    sys->rom_afloat = (const uint8_t*) desc->rom_afloat;
    sys->rom_dosrom = (const uint8_t*) desc->rom_dosrom;

    /* initialize the hardware */
    clk_init(&sys->clk, _ATOM_FREQUENCY);
    sys->period_2_4khz = _ATOM_FREQUENCY / 4800;

    m6502_desc_t cpu_desc = {0};
    cpu_desc.tick_cb = _atom_tick;
    cpu_desc.user_data = sys;
    m6502_init(&sys->cpu, &cpu_desc);

    mc6847_desc_t vdg_desc = {0};
    vdg_desc.tick_hz = _ATOM_FREQUENCY;
    vdg_desc.rgba8_buffer = desc->pixel_buffer;
    vdg_desc.rgba8_buffer_size = desc->pixel_buffer_size;
    vdg_desc.fetch_cb = _atom_vdg_fetch;
    vdg_desc.user_data = sys;
    mc6847_init(&sys->vdg, &vdg_desc);

    i8255_desc_t ppi_desc = {0};
    ppi_desc.in_cb = _atom_ppi_in;
    ppi_desc.out_cb = _atom_ppi_out;
    ppi_desc.user_data = sys;
    i8255_init(&sys->ppi, &ppi_desc);

    m6522_desc_t via_desc = {0};
    via_desc.in_cb = _atom_via_in;
    via_desc.out_cb = _atom_via_out;
    via_desc.user_data = sys;
    m6522_init(&sys->via, &via_desc);

    const int audio_hz = _ATOM_DEFAULT(desc->audio_sample_rate, 44100);
    const float audio_vol = _ATOM_DEFAULT(desc->audio_volume, 0.5f);
    beeper_init(&sys->beeper, _ATOM_FREQUENCY, audio_hz, audio_vol);

    /* setup memory map and keyboard matrix */
    _atom_init_memorymap(sys);
    _atom_init_keymap(sys);

    /* trap the OSLOAD function (http://ladybug.xs4all.nl/arlet/fpga/6502/kernel.dis) */
    m6502_set_trap(&sys->cpu, 1, 0xF96E);

    /* CPU start state */
    m6502_reset(&sys->cpu);
}

void atom_discard(atom_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    sys->valid = false;
}

void atom_reset(atom_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    m6502_reset(&sys->cpu);
    i8255_reset(&sys->ppi);
    m6522_reset(&sys->via);
    mc6847_reset(&sys->vdg);
    beeper_reset(&sys->beeper);
    sys->state_2_4khz = false;
    sys->out_cass0 = false;
    sys->out_cass1 = false;
}

void atom_exec(atom_t* sys, double seconds) {
    CHIPS_ASSERT(sys && sys->valid);
    uint32_t ticks_to_run = clk_ticks_to_run(&sys->clk, seconds);
    uint32_t ticks_executed = m6502_exec(&sys->cpu, ticks_to_run);
    clk_ticks_executed(&sys->clk, ticks_executed);
    kbd_update(&sys->kbd);
}

static void _atom_via_out(int port_id, uint8_t data, void* user_data) {
    /* FIXME */
}

static uint8_t _atom_via_in(int port_id, void* user_data) {
    /* FIXME */
    return 0x00;
}

static void _atom_init_keymap(atom_t* sys) {
    /*  setup the keyboard matrix
        the Atom has a 10x8 keyboard matrix, where the
        entire line 6 is for the Ctrl key, and the entire
        line 7 is the Shift key
    */
    kbd_init(&sys->kbd, 1);
    /* shift key is entire line 7 */
    const int shift = (1<<0); kbd_register_modifier_line(&sys->kbd, 0, 7);
    /* ctrl key is entire line 6 */
    const int ctrl = (1<<1); kbd_register_modifier_line(&sys->kbd, 1, 6);
    /* alpha-numeric keys */
    const char* keymap = 
        /* no shift */
        "     ^]\\[ "/**/"3210      "/* */"-,;:987654"/**/"GFEDCBA@/."/**/"QPONMLKJIH"/**/" ZYXWVUTSR"
        /* shift */
        "          "/* */"#\"!       "/**/"=<+*)('&%$"/**/"gfedcba ?>"/**/"qponmlkjih"/**/" zyxwvutsr";
    for (int layer = 0; layer < 2; layer++) {
        for (int column = 0; column < 10; column++) {
            for (int line = 0; line < 6; line++) {
                int c = keymap[layer*60 + line*10 + column];
                if (c != 0x20) {
                    kbd_register_key(&sys->kbd, c, column, line, layer?shift:0);
                }
            }
        }
    }
    /* special keys */
    kbd_register_key(&sys->kbd, 0x20, 9, 0, 0);         /* space */
    kbd_register_key(&sys->kbd, 0x01, 4, 1, 0);         /* backspace */
    kbd_register_key(&sys->kbd, 0x07, 0, 3, ctrl);      /* Ctrl+G: bleep */
    kbd_register_key(&sys->kbd, 0x08, 3, 0, shift);     /* key left */
    kbd_register_key(&sys->kbd, 0x09, 3, 0, 0);         /* key right */
    kbd_register_key(&sys->kbd, 0x0A, 2, 0, shift);     /* key down */
    kbd_register_key(&sys->kbd, 0x0B, 2, 0, 0);         /* key up */
    kbd_register_key(&sys->kbd, 0x0D, 6, 1, 0);         /* return/enter */
    kbd_register_key(&sys->kbd, 0x0C, 5, 4, ctrl);      /* Ctrl+L clear screen */
    kbd_register_key(&sys->kbd, 0x0E, 3, 4, ctrl);      /* Ctrl+N page mode on */
    kbd_register_key(&sys->kbd, 0x0F, 2, 4, ctrl);      /* Ctrl+O page mode off */
    kbd_register_key(&sys->kbd, 0x15, 6, 5, ctrl);      /* Ctrl+U end screen */
    kbd_register_key(&sys->kbd, 0x18, 3, 5, ctrl);      /* Ctrl+X cancel */
    kbd_register_key(&sys->kbd, 0x1B, 0, 5, 0);         /* escape */
}

static uint32_t _atom_xorshift32(uint32_t x) {
    x ^= x<<13;
    x ^= x>>17;
    x ^= x<<5;
    return x;
}

static void _atom_init_memorymap(atom_t* sys) {
    mem_init(&sys->mem);

    /* fill memory with random junk */
    uint32_t r = 0x6D98302B;
    for (int i = 0; i < (int)sizeof(sys->ram);) {
        r = _atom_xorshift32(r);
        sys->ram[i++] = r;
        sys->ram[i++] = (r>>8);
        sys->ram[i++] = (r>>16);
        sys->ram[i++] = (r>>24);
    }
    /* 32 KB RAM (with RAM extension) + 8 KB vidmem */
    mem_map_ram(&sys->mem, 0, 0x0000, 0xA000, sys->ram);
    /* hole in 0xA000 to 0xAFFF (for utility ROMs) */
    /* 0xB000 to 0xBFFF: IO area, not mapped */
    /* 16 KB ROMs from 0xC000 */
    mem_map_rom(&sys->mem, 0, 0xC000, 0x1000, sys->rom_abasic);
    mem_map_rom(&sys->mem, 0, 0xD000, 0x1000, sys->rom_afloat);
    mem_map_rom(&sys->mem, 0, 0xE000, 0x1000, sys->rom_dosrom);
    mem_map_rom(&sys->mem, 0, 0xF000, 0x1000, sys->rom_abasic + 0x1000);
}

#endif /* CHIPS_IMPL */
