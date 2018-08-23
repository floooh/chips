#pragma once
/*#
    # 64.h

    An Commodore C64 (PAL) emulator in a C header

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

    You need to include the following headers before including cpc.h:

    - chips/m6502.h"
    - chips/m6526.h"
    - chips/m6569.h"
    - chips/m6581.h"
    - chips/kbd.h"
    - chips/mem.h"
    - chips/clk.h

    ## The Commodore C64

    TODO!

    ## TODO:

    - improve game fast loader compatibility
    - floppy disc support

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

#define C64_DISPLAY_WIDTH (392)             /* required framebuffer width in pixels */
#define C64_DISPLAY_HEIGHT (272)            /* required framebuffer hight in pixels */
#define C64_MAX_AUDIO_SAMPLES (1024)        /* max number of audio samples in internal sample buffer */
#define C64_DEFAULT_AUDIO_SAMPLES (128)     /* default number of samples in internal sample buffer */ 
#define C64_MAX_TAPE_SIZE (128*1024)        /* max size of cassette tape image */

/* C64 joystick types */
typedef enum {
    C64_JOYSTICK_NONE,
    C64_JOYSTICK_DIGITAL,
    C64_JOYSTICK_PADDLE
} c64_joystick_t;

/* audio sample data callback */
typedef void (*c64_audio_callback_t)(const float* samples, int num_samples, void* user_data);

/* config parameters for c64_init() */
typedef struct {
    c64_joystick_t joystick_type;       /* default is C64_JOYSTICK_NONE */

    /* video output config */
    void* pixel_buffer;         /* pointer to a linear RGBA8 pixel buffer, at least 392*272*4 bytes */
    int pixel_buffer_size;      /* size of the pixel buffer in bytes */

    /* optional user-data for callback functions */
    void* user_data;

    /* optional user data for callback functions */
    /* audio output config (if you don't want audio, set audio_cb to zero) */
    c64_audio_callback_t audio_cb;  /* called when audio_num_samples are ready */
    int audio_num_samples;          /* default is C64_AUDIO_NUM_SAMPLES */
    int audio_sample_rate;          /* playback sample rate, default is 44100 */
    float audio_volume;             /* audio volume (0.0 .. 1.0), default is 1.0 */

    /* ROM images */
    const void* rom_char;           /* 4 KByte character ROM dump */
    const void* rom_basic;          /* 8 KByte BASIC dump */
    const void* rom_kernal;         /* 8 KByte KERNAL dump */
    int rom_char_size;
    int rom_basic_size;
    int rom_kernal_size;
} c64_desc_t;

/* C64 emulator state */
typedef struct {
    m6502_t cpu;
    m6526_t cia_1;
    m6526_t cia_2;
    m6569_t vic;
    m6581_t sid;
    
    bool valid;
    c64_joystick_t joystick_type;
    bool io_mapped;             /* true when D000..DFFF is has IO area mapped in */
    uint8_t cpu_port;           /* last state of CPU port (for memory mapping) */
    uint8_t cia1_joy_mask;      /* current joystick state */
    uint16_t vic_bank_select;   /* upper 4 address bits from CIA-2 port A */

    clk_t clk;
    kbd_t kbd;
    mem_t mem_cpu;
    mem_t mem_vic;

    void* user_data;
    uint32_t* pixel_buffer;
    c64_audio_callback_t audio_cb;
    int num_samples;
    int sample_pos;
    float sample_buffer[C64_MAX_AUDIO_SAMPLES];

    uint8_t color_ram[1024];        /* special static color ram */
    uint8_t ram[1<<16];             /* general ram */
    uint8_t rom_char[0x1000];       /* 4 KB character ROM image */
    uint8_t rom_basic[0x2000];      /* 8 KB BASIC ROM image */
    uint8_t rom_kernal[0x2000];     /* 8 KB KERNAL V3 ROM image */

    bool tape_on;   /* tape motor on/off */
    int tape_size;  /* tape_size > 0: a tape is inserted */
    int tape_pos;        
    uint8_t tape_buf[C64_MAX_TAPE_SIZE];
} c64_t;

/* initialize a new C64 instance */
extern void c64_init(c64_t* sys, const c64_desc_t* desc);
/* discard C64 instance */
extern void c64_discard(c64_t* sys);
/* reset a C64 instance */
extern void c64_reset(c64_t* sys);
/* tick C64 instance for a given time in seconds */
extern void c64_exec(c64_t* sys, double seconds);
/* send a key-down event to the C64 */
extern void c64_key_down(c64_t* sys, int key_code);
/* send a key-up event to the C64 */
extern void c64_key_up(c64_t* sys, int key_code);
/* insert a tape file */
extern void c64_insert_tape(c64_t* sys, const uint8_t* ptr, int num_bytes);
/* remove tape file */
extern void c64_remove_tape(c64_t* sys);

#ifdef __cplusplus
} /* extern "C" */
#endif

/*-- IMPLEMENTATION ----------------------------------------------------------*/
#ifdef CHIPS_IMPL
#include <string.h> /* memcpy, memset */
#ifndef CHIPS_DEBUG
    #ifdef _DEBUG
        #define CHIPS_DEBUG
    #endif
#endif
#ifndef CHIPS_ASSERT
    #include <assert.h>
    #define CHIPS_ASSERT(c) assert(c)
#endif

#define _C64_DISPLAY_SIZE (C64_DISPLAY_WIDTH*C64_DISPLAY_HEIGHT*4)
#define _C64_FREQUENCY (985248)
#define _C64_DEFAULT(val,def) (((val) != 0) ? (val) : (def));
#define _C64_DISPLAY_X (64)
#define _C64_DISPLAY_Y (24)
#define _C64_CPUPORT_CHAREN (1<<2)
#define _C64_CPUPORT_HIRAM (1<<1)
#define _C64_CPUPORT_LORAM (1<<0)

static uint64_t _c64_tick(uint64_t pins, void* user_data);
static uint8_t _c64_cpu_port_in(void* user_data);
static void _c64_cpu_port_out(uint8_t data, void* user_data);
static void _c64_cia1_out(int port_id, uint8_t data, void* user_data);
static uint8_t _c64_cia1_in(int port_id, void* user_data);
static void _c64_cia2_out(int port_id, uint8_t data, void* user_data);
static uint8_t _c64_cia2_in(int port_id, void* user_data);
static uint16_t _c64_vic_fetch(uint16_t addr, void* user_data);
static void _c64_update_memory_map(c64_t* sys);
static void _c64_init_key_map(c64_t* sys);
static void _c64_init_memory_map(c64_t* sys);

void c64_init(c64_t* sys, const c64_desc_t* desc) {
    CHIPS_ASSERT(sys && desc);
    CHIPS_ASSERT(desc->pixel_buffer && (desc->pixel_buffer_size >= _C64_DISPLAY_SIZE));

    memset(sys, 0, sizeof(c64_t));
    sys->valid = true;
    sys->joystick_type = desc->joystick_type;
    CHIPS_ASSERT(desc->rom_char && (desc->rom_char_size == sizeof(sys->rom_char)));
    CHIPS_ASSERT(desc->rom_basic && (desc->rom_basic_size == sizeof(sys->rom_basic)));
    CHIPS_ASSERT(desc->rom_kernal && (desc->rom_kernal_size == sizeof(sys->rom_kernal)));
    memcpy(sys->rom_char, desc->rom_char, sizeof(sys->rom_char));
    memcpy(sys->rom_basic, desc->rom_basic, sizeof(sys->rom_basic));
    memcpy(sys->rom_kernal, desc->rom_kernal, sizeof(sys->rom_kernal));
    sys->pixel_buffer = desc->pixel_buffer;
    sys->user_data = desc->user_data;
    sys->audio_cb = desc->audio_cb;
    sys->num_samples = _C64_DEFAULT(desc->audio_num_samples, C64_DEFAULT_AUDIO_SAMPLES);
    CHIPS_ASSERT(sys->num_samples <= C64_MAX_AUDIO_SAMPLES);

    /* initialize the hardware */
    clk_init(&sys->clk, _C64_FREQUENCY);
    sys->cpu_port = 0xF7;       /* for initial memory mapping */
    sys->io_mapped = true;
    
    m6502_desc_t cpu_desc = {0};
    cpu_desc.tick_cb = _c64_tick;
    cpu_desc.in_cb = _c64_cpu_port_in;
    cpu_desc.out_cb = _c64_cpu_port_out;
    cpu_desc.m6510_io_pullup = 0x17;
    cpu_desc.m6510_io_floating = 0xC8;
    cpu_desc.user_data = sys;
    m6502_init(&sys->cpu, &cpu_desc);

    m6526_desc_t cia_desc = {0};
    cia_desc.user_data = sys;
    cia_desc.in_cb = _c64_cia1_in;
    cia_desc.out_cb = _c64_cia1_out;
    m6526_init(&sys->cia_1, &cia_desc);
    cia_desc.in_cb = _c64_cia2_in;
    cia_desc.out_cb = _c64_cia2_out;
    m6526_init(&sys->cia_2, &cia_desc);

    m6569_desc_t vic_desc = {0};
    vic_desc.fetch_cb = _c64_vic_fetch;
    vic_desc.rgba8_buffer = desc->pixel_buffer;
    vic_desc.rgba8_buffer_size = desc->pixel_buffer_size;
    vic_desc.vis_x = _C64_DISPLAY_X;
    vic_desc.vis_y = _C64_DISPLAY_Y;
    vic_desc.vis_w = C64_DISPLAY_WIDTH;
    vic_desc.vis_h = C64_DISPLAY_HEIGHT;
    vic_desc.user_data = sys;
    m6569_init(&sys->vic, &vic_desc);

    m6581_desc_t sid_desc = {0};
    sid_desc.tick_hz = _C64_FREQUENCY;
    sid_desc.sound_hz = _C64_DEFAULT(desc->audio_sample_rate, 44100);
    sid_desc.magnitude = _C64_DEFAULT(desc->audio_volume, 1.0f);

    _c64_init_key_map(sys);
    _c64_init_memory_map(sys);

    m6502_reset(&sys->cpu);
}

void c64_discard(c64_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    sys->valid = false;
}

void c64_reset(c64_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    sys->cpu_port = 0xF7;
    sys->cia1_joy_mask = 0;
    sys->io_mapped = true;
    _c64_update_memory_map(sys);
    m6502_reset(&sys->cpu);
    m6526_reset(&sys->cia_1);
    m6526_reset(&sys->cia_2);
    m6569_reset(&sys->vic);
    m6581_reset(&sys->sid);
    sys->tape_on = 0;
    sys->tape_pos = 0;
    sys->tape_size = 0;
}

void c64_exec(c64_t* sys, double seconds) {
    CHIPS_ASSERT(sys && sys->valid);
    uint32_t ticks_to_run = clk_ticks_to_run(&sys->clk, seconds);
    uint32_t ticks_executed = m6502_exec(&sys->cpu, ticks_to_run);
    clk_ticks_executed(&sys->clk, ticks_executed);
    kbd_update(&sys->kbd);
}

static void _c64_update_memory_map(c64_t* sys) {
    sys->io_mapped = false;
    uint8_t* read_ptr;
    /* shortcut if HIRAM and LORAM is 0, everything is RAM */
    if ((sys->cpu_port & (_C64_CPUPORT_HIRAM|_C64_CPUPORT_LORAM)) == 0) {
        mem_map_ram(&sys->mem_cpu, 0, 0xA000, 0x6000, sys->ram+0xA000);
    }
    else {
        /* A000..BFFF is either RAM-behind-BASIC-ROM or RAM */
        if ((sys->cpu_port & (_C64_CPUPORT_HIRAM|_C64_CPUPORT_LORAM)) == (_C64_CPUPORT_HIRAM|_C64_CPUPORT_LORAM)) {
            read_ptr = sys->rom_basic;
        }
        else {
            read_ptr = sys->ram + 0xA000;
        }
        mem_map_rw(&sys->mem_cpu, 0, 0xA000, 0x2000, read_ptr, sys->ram+0xA000);

        /* E000..FFFF is either RAM-behind-KERNAL-ROM or RAM */
        if (sys->cpu_port & _C64_CPUPORT_HIRAM) {
            read_ptr = sys->rom_kernal;
        }
        else {
            read_ptr = sys->ram + 0xE000;
        }
        mem_map_rw(&sys->mem_cpu, 0, 0xE000, 0x2000, read_ptr, sys->ram+0xE000);

        /* D000..DFFF can be Char-ROM or I/O */
        if  (sys->cpu_port & _C64_CPUPORT_CHAREN) {
            sys->io_mapped = true;
        }
        else {
            mem_map_rw(&sys->mem_cpu, 0, 0xD000, 0x1000, sys->rom_char, sys->ram+0xD000);
        }
    }
}

static void _c64_init_memory_map(c64_t* sys) {
    mem_init(&sys->mem_cpu);
    mem_init(&sys->mem_vic);

    /*
        the C64 has a weird RAM init pattern of 64 bytes 00 and 64 bytes FF
        alternating, probably with some randomness sprinkled in
        (see this thread: http://csdb.dk/forums/?roomid=11&topicid=116800&firstpost=2)
        this is important at least for the value of the 'ghost byte' at 0x3FFF,
        which is 0xFF
    */
    int i;
    for (i = 0; i < (1<<16);) {
        for (int j = 0; j < 64; j++, i++) {
            sys->ram[i] = 0x00;
        }
        for (int j = 0; j < 64; j++, i++) {
            sys->ram[i] = 0xFF;
        }
    }
    CHIPS_ASSERT(i == 0x10000);

    /* setup the initial CPU memory map
       0000..9FFF and C000.CFFF is always RAM
    */
    mem_map_ram(&sys->mem_cpu, 0, 0x0000, 0xA000, sys->ram);
    mem_map_ram(&sys->mem_cpu, 0, 0xC000, 0x1000, sys->ram+0xC000);
    /* A000..BFFF, D000..DFFF and E000..FFFF are configurable */
    _c64_update_memory_map(sys);

    /* setup the separate VIC-II memory map (64 KByte RAM) overlayed with
       character ROMS at 0x1000.0x1FFF and 0x9000..0x9FFF
    */
    mem_map_ram(&sys->mem_vic, 1, 0x0000, 0x10000, sys->ram);
    mem_map_rom(&sys->mem_vic, 0, 0x1000, 0x1000, sys->rom_char);
    mem_map_rom(&sys->mem_vic, 0, 0x9000, 0x1000, sys->rom_char);
}

static void _c64_init_key_map(c64_t* sys) {
    /*
        http://sta.c64.org/cbm64kbdlay.html
        http://sta.c64.org/cbm64petkey.html
    */
    kbd_init(&sys->kbd, 1);

    const char* keymap =
        /* no shift */
        "        "
        "3WA4ZSE "
        "5RD6CFTX"
        "7YG8BHUV"
        "9IJ0MKON"
        "+PL-.:@,"
        "~*;  = /"  /* ~ is actually the British Pound sign */
        "1  2  Q "

        /* shift */
        "        "
        "#wa$zse "
        "%rd&cftx"
        "'yg(bhuv"
        ")ij0mkon"
        " pl >[ <"
        "$ ]    ?"
        "!  \"  q ";
    CHIPS_ASSERT(strlen(keymap) == 128);
    /* shift is column 7, line 1 */
    kbd_register_modifier(&sys->kbd, 0, 7, 1);
    /* ctrl is column 2, line 7 */
    kbd_register_modifier(&sys->kbd, 1, 2, 7);
    for (int shift = 0; shift < 2; shift++) {
        for (int column = 0; column < 8; column++) {
            for (int line = 0; line < 8; line++) {
                int c = keymap[shift*64 + line*8 + column];
                if (c != 0x20) {
                    kbd_register_key(&sys->kbd, c, column, line, shift?(1<<0):0);
                }
            }
        }
    }

    /* special keys */
    kbd_register_key(&sys->kbd, 0x20, 4, 7, 0);    /* space */
    kbd_register_key(&sys->kbd, 0x08, 2, 0, 1);    /* cursor left */
    kbd_register_key(&sys->kbd, 0x09, 2, 0, 0);    /* cursor right */
    kbd_register_key(&sys->kbd, 0x0A, 7, 0, 0);    /* cursor down */
    kbd_register_key(&sys->kbd, 0x0B, 7, 0, 1);    /* cursor up */
    kbd_register_key(&sys->kbd, 0x01, 0, 0, 0);    /* delete */
    kbd_register_key(&sys->kbd, 0x0C, 3, 6, 1);    /* clear */
    kbd_register_key(&sys->kbd, 0x0D, 1, 0, 0);    /* return */
    kbd_register_key(&sys->kbd, 0x03, 7, 7, 0);    /* stop */
    kbd_register_key(&sys->kbd, 0xF1, 4, 0, 0);
    kbd_register_key(&sys->kbd, 0xF2, 4, 0, 1);
    kbd_register_key(&sys->kbd, 0xF3, 5, 0, 0);
    kbd_register_key(&sys->kbd, 0xF4, 5, 0, 1);
    kbd_register_key(&sys->kbd, 0xF5, 6, 0, 0);
    kbd_register_key(&sys->kbd, 0xF6, 6, 0, 1);
    kbd_register_key(&sys->kbd, 0xF7, 3, 0, 0);
    kbd_register_key(&sys->kbd, 0xF8, 3, 0, 1);
}


#endif /* CHIPS_IMPL */
