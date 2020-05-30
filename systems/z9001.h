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

    You need to include the following headers before including z9001.h:

    - chips/z80.h
    - chips/z80pio.h
    - chips/z80ctc.h
    - chips/beeper.h
    - chips/mem.h
    - chips/kbd.h
    - chips/clk.h
  
    ## The Robotron Z9001

    The Z9001 (later retconned to KC85/1) was independently developed
    to the HC900 (aka KC85/2) by Robotron Dresden. It had a pretty
    slick design with an integrated keyboard which was legendary for
    how hard it was to type on.\n\nThe standard model had 16 KByte RAM,
    a monochrome 40x24 character display, and a 2.5 MHz U880 CPU
    (making it the fastest East German 8-bitter). The Z9001 could
    be extended by up to 4 expansion modules, usually one or two
    16 KByte RAM modules, and a 10 KByte ROM BASIC module (the
    version emulated here comes with 32 KByte RAM and a BASIC module).

    ## The Robotron KC87

    The KC87 was the successor to the KC85/1. The only real difference
    was the built-in BASIC interpreter in ROM. The KC87 emulated here
    has 48 KByte RAM and the video color extension which could
    assign 8 foreground and 8 (identical) background colors per character,
    plus a blinking flag. This video extension was already available on the
    Z9001 though.

    ## TODO:
    - enable/disable audio on PIO1-A bit 7
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

#define Z9001_MAX_AUDIO_SAMPLES (1024)      /* max number of audio samples in internal sample buffer */
#define Z9001_DEFAULT_AUDIO_SAMPLES (128)   /* default number of samples in internal sample buffer */ 

/* Z9001/KC87 model types */
typedef enum {
    Z9001_TYPE_Z9001,   /* the original Z9001 (default) */
    Z9001_TYPE_KC87,    /* the revised KC87 with built-in BASIC and color module */
} z9001_type_t;

/* Z9001 audio sample data callback */
typedef void (*z9001_audio_callback_t)(const float* samples, int num_samples, void* user_data);

/* configuration parameters for z9001_init() */
typedef struct {
    z9001_type_t type;                  /* default is Z9001_TYPE_Z9001 */

    /* video output config */
    void* pixel_buffer;         /* pointer to a linear RGBA8 pixel buffer, at least 320*192*4 bytes */
    int pixel_buffer_size;      /* size of the pixel buffer in bytes */

    /* optional user data for call back functions */
    void* user_data;

    /* audio output config (if you don't want audio, set audio_cb to zero) */
    z9001_audio_callback_t audio_cb;    /* called when audio_num_samples are ready */
    int audio_num_samples;              /* default is Z9001_DEFAULT_AUDIO_SAMPLES */
    int audio_sample_rate;              /* playback sample rate, default is 44100 */
    float audio_volume;                 /* volume of generated audio: 0.0..1.0, default is 0.5 */

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
} z9001_desc_t;

/* Z9001 emulator state */
typedef struct {
    z80_t cpu;
    z80pio_t pio1;
    z80pio_t pio2;
    z80ctc_t ctc;
    beeper_t beeper;
    bool valid;
    bool z9001_has_basic_rom;
    z9001_type_t type;
    uint64_t ctc_zcto2;     /* pin mask to store state of CTC ZCTO2 */
    uint32_t blink_counter;
    bool blink_flip_flop;
    /* FIXME: uint8_t border_color; */
    clk_t clk;
    mem_t mem;
    kbd_t kbd;
    uint32_t* pixel_buffer;
    void* user_data;
    z9001_audio_callback_t audio_cb;
    int num_samples;
    int sample_pos;
    float sample_buffer[Z9001_MAX_AUDIO_SAMPLES];
    uint8_t ram[1<<16];
    uint8_t rom[0x4000];
    uint8_t rom_font[0x0800];   /* 2 KB font ROM (not mapped into CPU address space) */
} z9001_t;

/* initialize a new Z9001 instance */
void z9001_init(z9001_t* sys, const z9001_desc_t* desc);
/* discard a Z9001 instance */
void z9001_discard(z9001_t* sys);
/* get the standard framebuffer width and height in pixels */
int z9001_std_display_width(void);
int z9001_std_display_height(void);
/* get the maximum framebuffer size in number of bytes */
int z9001_max_display_size(void);
/* get the current framebuffer width and height in pixels */
int z9001_display_width(z9001_t* sys);
int z9001_display_height(z9001_t* sys);
/* reset Z9001 instance */
void z9001_reset(z9001_t* sys);
/* run Z9001 instance for a given number of microseconds */
void z9001_exec(z9001_t* sys, uint32_t micro_seconds);
/* send a key-down event */
void z9001_key_down(z9001_t* sys, int key_code);
/* send a key-up event */
void z9001_key_up(z9001_t* sys, int key_code);
/* load a KC TAP or KCC file into the emulator */
bool z9001_quickload(z9001_t* sys, const uint8_t* ptr, int num_bytes);

#ifdef __cplusplus
} /* extern "C" */
#endif

/*-- IMPLEMENTATION ----------------------------------------------------------*/
#ifdef CHIPS_IMPL
#include <string.h>
#ifndef CHIPS_ASSERT
    #include <assert.h>
    #define CHIPS_ASSERT(c) assert(c)
#endif

#define _Z9001_DISPLAY_WIDTH (320)
#define _Z9001_DISPLAY_HEIGHT (192)
#define _Z9001_DISPLAY_SIZE (_Z9001_DISPLAY_WIDTH*_Z9001_DISPLAY_HEIGHT*4)
#define _Z9001_FREQUENCY (2457600)

static uint64_t _z9001_tick(int num, uint64_t pins, void* user_data);
static uint8_t _z9001_pio1_in(int port_id, void* user_data);
static void _z9001_pio1_out(int port_id, uint8_t data, void* user_data);
static uint8_t _z9001_pio2_in(int port_id, void* user_data);
static void _z9001_pio2_out(int port_id, uint8_t data, void* user_data);
static void _z9001_decode_vidmem(z9001_t* sys);

/* xorshift randomness for memory initialization */
static inline uint32_t _z9001_xorshift32(uint32_t x) {
    x ^= x<<13;
    x ^= x>>17;
    x ^= x<<5;
    return x;
}

#define _Z9001_DEFAULT(val,def) (((val) != 0) ? (val) : (def));
#define _Z9001_CLEAR(val) memset(&val, 0, sizeof(val))

void z9001_init(z9001_t* sys, const z9001_desc_t* desc) {
    CHIPS_ASSERT(sys && desc);

    memset(sys, 0, sizeof(z9001_t));
    sys->valid = true;
    sys->type = desc->type;
    if (desc->type == Z9001_TYPE_Z9001) {
        CHIPS_ASSERT(desc->rom_z9001_font && (desc->rom_z9001_font_size == 0x0800));
        memcpy(sys->rom_font, desc->rom_z9001_font, 0x0800);
        if (desc->rom_z9001_basic) {
            CHIPS_ASSERT(desc->rom_z9001_basic_size == 0x2800);
            memcpy(&sys->rom[0x0000], desc->rom_z9001_basic, 0x2800);
            sys->z9001_has_basic_rom = true;
        }
        CHIPS_ASSERT(desc->rom_z9001_os_1 && (desc->rom_z9001_os_1_size == 0x0800));
        memcpy(&sys->rom[0x3000], desc->rom_z9001_os_1, 0x0800);
        CHIPS_ASSERT(desc->rom_z9001_os_2 && (desc->rom_z9001_os_2_size == 0x0800));
        memcpy(&sys->rom[0x3800], desc->rom_z9001_os_2, 0x0800);
    }
    else {
        CHIPS_ASSERT(desc->rom_kc87_font && (desc->rom_kc87_font_size == 0x0800));
        memcpy(sys->rom_font, desc->rom_kc87_font, 0x0800);
        CHIPS_ASSERT(desc->rom_kc87_basic && (desc->rom_kc87_basic_size == 0x2000));
        memcpy(&sys->rom[0x0000], desc->rom_kc87_basic, 0x2000);
        CHIPS_ASSERT(desc->rom_kc87_os && (desc->rom_kc87_os_size == 0x2000));
        memcpy(&sys->rom[0x2000], desc->rom_kc87_os, 0x2000);
    }
    CHIPS_ASSERT(desc->pixel_buffer && (desc->pixel_buffer_size >= _Z9001_DISPLAY_SIZE));
    sys->pixel_buffer = (uint32_t*) desc->pixel_buffer;
    sys->audio_cb = desc->audio_cb;
    sys->user_data = desc->user_data;
    sys->num_samples = _Z9001_DEFAULT(desc->audio_num_samples, Z9001_DEFAULT_AUDIO_SAMPLES);
    CHIPS_ASSERT(sys->num_samples <= Z9001_MAX_AUDIO_SAMPLES);

    /* initialize the hardware */
    clk_init(&sys->clk, _Z9001_FREQUENCY);
    z80ctc_init(&sys->ctc);

    z80_desc_t cpu_desc;
    _Z9001_CLEAR(cpu_desc);
    cpu_desc.tick_cb = _z9001_tick;
    cpu_desc.user_data = sys;
    z80_init(&sys->cpu, &cpu_desc);
    
    z80pio_desc_t pio1_desc;
    _Z9001_CLEAR(pio1_desc);
    pio1_desc.in_cb = _z9001_pio1_in;
    pio1_desc.out_cb = _z9001_pio1_out;
    pio1_desc.user_data = sys;
    z80pio_init(&sys->pio1, &pio1_desc);
    
    z80pio_desc_t pio2_desc;
    _Z9001_CLEAR(pio2_desc);
    pio2_desc.in_cb = _z9001_pio2_in;
    pio2_desc.out_cb = _z9001_pio2_out;
    pio2_desc.user_data = sys;
    z80pio_init(&sys->pio2, &pio2_desc);

    const int audio_hz = _Z9001_DEFAULT(desc->audio_sample_rate, 44100);
    const float audio_vol = _Z9001_DEFAULT(desc->audio_volume, 0.5f);
    beeper_init(&sys->beeper, _Z9001_FREQUENCY, audio_hz, audio_vol);
    
    /* execution starts at 0xF000 */
    z80_set_pc(&sys->cpu, 0xF000);

    /* setup memory map:
        - memory mapping is static and cannot be changed
        - initial memory state is random
        - configure the Z9001 with an additional 16 KB RAM module, and BASIC module
        - configure the KC87 with 48 KByte RAM and color module
        - 1 KB ASCII frame buffer at 0xEC00
        - KC87 has a 1 KB color buffer at 0xEC800
    */
    uint32_t r = 0x6D98302B;
    for (int i = 0; i < (int)sizeof(sys->ram);) {
        r = _z9001_xorshift32(r);
        sys->ram[i++] = r;
        sys->ram[i++] = (r>>8);
        sys->ram[i++] = (r>>16);
        sys->ram[i++] = (r>>24);
    }
    mem_init(&sys->mem);
    if (Z9001_TYPE_Z9001 == sys->type) {
        /* 16 KB RAM + 16 KB RAM module */
        mem_map_ram(&sys->mem, 0, 0x0000, 0x8000, sys->ram);
        /* optional 12 KB BASIC ROM module at 0xC000 */
        if (desc->rom_z9001_basic) {
            mem_map_rom(&sys->mem, 1, 0xC000, 0x2800, &sys->rom[0x0000]);
        }
        /* 2 OS ROMs at 2 KB each */
        mem_map_rom(&sys->mem, 1, 0xF000, 0x0800, &sys->rom[0x3000]);
        mem_map_rom(&sys->mem, 1, 0xF800, 0x0800, &sys->rom[0x3800]);
    }
    else {
        /* 48 KB RAM */
        mem_map_ram(&sys->mem, 0, 0x0000, 0xC000, sys->ram);
        /* 1 KB color RAM */
        mem_map_ram(&sys->mem, 0, 0xE800, 0x0400, &sys->ram[0xE800]);
        /* 8 KB builtin BASIC */
        mem_map_rom(&sys->mem, 1, 0xC000, 0x2000, &sys->rom[0x0000]);
        /* 8 KB ROM (overlayed by 1 KB at 0xEC00 for ASCII video RAM) */
        mem_map_rom(&sys->mem, 1, 0xE000, 0x2000, &sys->rom[0x2000]);
    }
    /* 1 KB ASCII video RAM */
    mem_map_ram(&sys->mem, 0, 0xEC00, 0x0400, &sys->ram[0xEC00]);

    /* setup the 8x8 keyboard matrix, keep pressed keys sticky for 3 frames 
        to give the keyboard scanning code enough time to read the key
    */
    kbd_init(&sys->kbd, 3);
    /* shift key is column 0, line 7 */
    kbd_register_modifier(&sys->kbd, 0, 0, 7);
    /* register alpha-numeric keys */
    const char* keymap =
        /* unshifted keys */
        "01234567"
        "89:;,=.?"
        "@ABCDEFG"
        "HIJKLMNO"
        "PQRSTUVW"
        "XYZ   ^ "
        "        "
        "        "
        /* shifted keys */
        "_!\"#$%&'"
        "()*+<->/"
        " abcdefg"
        "hijklmno"
        "pqrstuvw"
        "xyz     "
        "        "
        "        ";
    for (int shift = 0; shift < 2; shift++) {
        for (int line = 0; line < 8; line++) {
            for (int column = 0; column < 8; column++) {
                int c = keymap[shift*64 + line*8 + column];
                if (c != 0x20) {
                    kbd_register_key(&sys->kbd, c, column, line, shift?(1<<0):0);
                }
            }
        }
    }
    /* special keys */
    kbd_register_key(&sys->kbd, 0x03, 6, 6, 0);      /* stop (Esc) */
    kbd_register_key(&sys->kbd, 0x08, 0, 6, 0);      /* cursor left */
    kbd_register_key(&sys->kbd, 0x09, 1, 6, 0);      /* cursor right */
    kbd_register_key(&sys->kbd, 0x0A, 2, 6, 0);      /* cursor up */
    kbd_register_key(&sys->kbd, 0x0B, 3, 6, 0);      /* cursor down */
    kbd_register_key(&sys->kbd, 0x0D, 5, 6, 0);      /* enter */
    kbd_register_key(&sys->kbd, 0x13, 4, 5, 0);      /* pause */
    kbd_register_key(&sys->kbd, 0x14, 1, 7, 0);      /* color */
    kbd_register_key(&sys->kbd, 0x19, 3, 5, 0);      /* home */
    kbd_register_key(&sys->kbd, 0x1A, 5, 5, 0);      /* insert */
    kbd_register_key(&sys->kbd, 0x1B, 4, 6, 0);      /* esc (Shift+Esc) */
    kbd_register_key(&sys->kbd, 0x1C, 4, 7, 0);      /* list */
    kbd_register_key(&sys->kbd, 0x1D, 5, 7, 0);      /* run */
    kbd_register_key(&sys->kbd, 0x20, 7, 6, 0);      /* space */
}

void z9001_discard(z9001_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    sys->valid = false;
}

int z9001_std_display_width(void) {
    return _Z9001_DISPLAY_WIDTH;
}

int z9001_std_display_height(void) {
    return _Z9001_DISPLAY_HEIGHT;
}

int z9001_max_display_size(void) {
    return _Z9001_DISPLAY_SIZE;
}

int z9001_display_width(z9001_t* sys) {
    (void)sys;
    return _Z9001_DISPLAY_WIDTH;
}

int z9001_display_height(z9001_t* sys) {
    (void)sys;
    return _Z9001_DISPLAY_HEIGHT;
}

void z9001_reset(z9001_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    z80_reset(&sys->cpu);
    z80pio_reset(&sys->pio1);
    z80pio_reset(&sys->pio2);
    z80ctc_reset(&sys->ctc);
    beeper_reset(&sys->beeper);
    z80_set_pc(&sys->cpu, 0xF000);
}

void z9001_exec(z9001_t* sys, uint32_t micro_seconds) {
    CHIPS_ASSERT(sys && sys->valid);
    uint32_t ticks_to_run = clk_ticks_to_run(&sys->clk, micro_seconds);
    uint32_t ticks_executed = z80_exec(&sys->cpu, ticks_to_run);
    clk_ticks_executed(&sys->clk, ticks_executed);
    kbd_update(&sys->kbd, micro_seconds);
    _z9001_decode_vidmem(sys);
}

void z9001_key_down(z9001_t* sys, int key_code) {
    CHIPS_ASSERT(sys && sys->valid);
    kbd_key_down(&sys->kbd, key_code);
    /* keyboard matrix lines are directly connected to the PIO2's Port B */
    z80pio_write_port(&sys->pio2, Z80PIO_PORT_B, ~kbd_scan_lines(&sys->kbd));
}

void z9001_key_up(z9001_t* sys, int key_code) {
    CHIPS_ASSERT(sys && sys->valid);
    kbd_key_up(&sys->kbd, key_code);
    /* keyboard matrix lines are directly connected to the PIO2's Port B */
    z80pio_write_port(&sys->pio2, Z80PIO_PORT_B, ~kbd_scan_lines(&sys->kbd));
}

/* the CPU tick callback performs memory and I/O reads/writes */
static uint64_t _z9001_tick(int num_ticks, uint64_t pins, void* user_data) {
    z9001_t* sys = (z9001_t*) user_data;

    /* tick the CTC channels, the CTC channel 2 output signal ZCTO2 is connected
       to CTC channel 3 input signal CLKTRG3 to form a timer cascade
       which drives the system clock, store the state of ZCTO2 for the
       next tick
    */
    pins |= sys->ctc_zcto2;
    for (int i = 0; i < num_ticks; i++) {
        if (pins & Z80CTC_ZCTO2) { pins |= Z80CTC_CLKTRG3; }
        else                     { pins &= ~Z80CTC_CLKTRG3; }
        pins = z80ctc_tick(&sys->ctc, pins);
        if (pins & Z80CTC_ZCTO0) {
            /* CTC channel 0 controls the beeper frequency */
            beeper_toggle(&sys->beeper);
        }
        if (beeper_tick(&sys->beeper)) {
            /* new audio sample ready */
            sys->sample_buffer[sys->sample_pos++] = sys->beeper.sample;
            if (sys->sample_pos == sys->num_samples) {
                if (sys->audio_cb) {
                    sys->audio_cb(sys->sample_buffer, sys->num_samples, sys->user_data);
                }
                sys->sample_pos = 0;
            }
        }
        /* the blink flip flop is controlled by a 'bisync' video signal
            (I guess that means it triggers at half PAL frequency: 25Hz),
            going into a binary counter, bit 4 of the counter is connected
            to the blink flip flop.
        */
        if (0 >= sys->blink_counter--) {
            sys->blink_counter = (_Z9001_FREQUENCY * 8) / 25;
            sys->blink_flip_flop = !sys->blink_flip_flop;
        }
    }
    sys->ctc_zcto2 = (pins & Z80CTC_ZCTO2);
    pins = pins & Z80_PIN_MASK;

    /* memory and IO requests */
    if (pins & Z80_MREQ) {
        /* a memory request machine cycle */
        const uint16_t addr = Z80_GET_ADDR(pins);
        if (pins & Z80_RD) {
            /* read memory byte */
            Z80_SET_DATA(pins, mem_rd(&sys->mem, addr));
        }
        else if (pins & Z80_WR) {
            mem_wr(&sys->mem, addr, Z80_GET_DATA(pins));
        }
    }
    else if (pins & Z80_IORQ) {
        /* an IO request machine cycle */

        /* check if any of the PIO/CTC chips is enabled */
        /* address line 7 must be on, 6 must be off, IORQ on, M1 off */
        const bool chip_enable = (pins & (Z80_IORQ|Z80_M1|Z80_A7|Z80_A6)) == (Z80_IORQ|Z80_A7);
        const int chip_select = (pins & (Z80_A5|Z80_A4|Z80_A3))>>3;
        if (chip_enable) {
            switch (chip_select) {
                /* IO request on CTC? */
                case 0:
                    /* CTC is mapped to ports 0x80 to 0x87 (each port is mapped twice) */
                    pins |= Z80CTC_CE;
                    if (pins & Z80_A0) { pins |= Z80CTC_CS0; };
                    if (pins & Z80_A1) { pins |= Z80CTC_CS1; };
                    pins = z80ctc_iorq(&sys->ctc, pins) & Z80_PIN_MASK;
                    break;
                /* IO request on PIO1? */
                case 1:
                    /* PIO1 is mapped to ports 0x88 to 0x8F (each port is mapped twice) */
                    pins |= Z80PIO_CE;
                    if (pins & Z80_A0) { pins |= Z80PIO_BASEL; }
                    if (pins & Z80_A1) { pins |= Z80PIO_CDSEL; }
                    pins = z80pio_iorq(&sys->pio1, pins) & Z80_PIN_MASK;
                    break;
                /* IO request on PIO2? */
                case 2:
                    /* PIO2 is mapped to ports 0x90 to 0x97 (each port is mapped twice) */
                    pins |= Z80PIO_CE;
                    if (pins & Z80_A0) { pins |= Z80PIO_BASEL; }
                    if (pins & Z80_A1) { pins |= Z80PIO_CDSEL; }
                    pins = z80pio_iorq(&sys->pio2, pins) & Z80_PIN_MASK;
            }
        }
    }

    /* handle interrupt requests by PIOs and CTCs, the interrupt priority
       is PIO1>PIO2>CTC, the interrupt handling functions must be called
       in this order
    */
    Z80_DAISYCHAIN_BEGIN(pins)
    {
        pins = z80pio_int(&sys->pio1, pins);
        pins = z80pio_int(&sys->pio2, pins);
        pins = z80ctc_int(&sys->ctc, pins);
    }
    Z80_DAISYCHAIN_END(pins);
    return (pins & Z80_PIN_MASK);
}


static uint8_t _z9001_pio1_in(int port_id, void* user_data) {
    (void)port_id; (void)user_data;
    return 0x00;
}

static void _z9001_pio1_out(int port_id, uint8_t data, void* user_data) {
    (void)data; (void)user_data;
    if (Z80PIO_PORT_A == port_id) {
        /*
            PIO1-A bits:
            0..1:    unused
            2:       display mode (0: 24 lines, 1: 20 lines)
            3..5:    border color
            6:       graphics LED on keyboard (0: off)
            7:       enable audio output (1: enabled)
        */
        /* FIXME: border_color = (data>>3) & 7; */
    }
    else {
        /* PIO1-B is reserved for external user devices */
    }
}

/* PIO2: keyboard input */
uint8_t _z9001_pio2_in(int port_id, void* user_data) {
    z9001_t* sys = (z9001_t*) user_data;
    if (Z80PIO_PORT_A == port_id) {
        /* return keyboard matrix column bits for requested line bits */
        uint8_t columns = (uint8_t) kbd_scan_columns(&sys->kbd);
        return ~columns;
    }
    else {
        /* return keyboard matrix line bits for requested column bits */
        uint8_t lines = (uint8_t) kbd_scan_lines(&sys->kbd);
        return ~lines;
    }
}

void _z9001_pio2_out(int port_id, uint8_t data, void* user_data) {
    z9001_t* sys = (z9001_t*) user_data;
    if (Z80PIO_PORT_A == port_id) {
        kbd_set_active_columns(&sys->kbd, ~data);
    }
    else {
        kbd_set_active_lines(&sys->kbd, ~data);
    }
}

/* decode the KC87 40x24 framebuffer to a linear 320x192 RGBA8 buffer */
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
static void _z9001_decode_vidmem(z9001_t* sys) {
    /* FIXME: there's also a 40x20 video mode */
    uint32_t* dst = sys->pixel_buffer;
    const uint8_t* vidmem = &sys->ram[0xEC00];     /* 1 KB ASCII buffer at EC00 */
    const uint8_t* colmem = &sys->ram[0xE800];     /* 1 KB color buffer at E800 */
    int offset = 0;
    if (Z9001_TYPE_KC87 == sys->type) {
        /* KC87 with color module */
        const uint8_t* font = sys->rom_font;
        uint32_t fg, bg;
        for (int y = 0; y < 24; y++) {
            for (int py = 0; py < 8; py++) {
                for (int x = 0; x < 40; x++) {
                    uint8_t chr = vidmem[offset+x];
                    uint8_t pixels = font[(chr<<3)|py];
                    uint8_t color = colmem[offset+x];
                    if ((color & 0x80) && sys->blink_flip_flop) {
                        /* blinking: swap back- and foreground color */
                        fg = _z9001_palette[color&7];
                        bg = _z9001_palette[(color>>4)&7];
                    }
                    else {
                        fg = _z9001_palette[(color>>4)&7];
                        bg = _z9001_palette[color&7];
                    }
                    for (int px = 7; px >= 0; px--) {
                        *dst++ = pixels & (1<<px) ? fg:bg;
                    }
                }
            }
            offset += 40;
        }
    }
    else {
        /* Z9001 monochrome display */
        const uint8_t* font = sys->rom_font;
        for (int y = 0; y < 24; y++) {
            for (int py = 0; py < 8; py++) {
                for (int x = 0; x < 40; x++) {
                    uint8_t chr = vidmem[offset + x];
                    uint8_t pixels = font[(chr<<3)|py];
                    for (int px = 7; px >=0; px--) {
                        *dst++ = pixels & (1<<px) ? 0xFFFFFFFF : 0xFF000000;
                    }
                }
            }
            offset += 40;
        }
    }
}

/*=== FILE LOADING ===========================================================*/

/* common start function for file loading routines */
static void _z9001_load_start(z9001_t* sys, uint16_t exec_addr) {
    z80_set_a(&sys->cpu, 0x00); z80_set_f(&sys->cpu, 0x10);
    z80_set_bc(&sys->cpu, 0x0000); z80_set_bc_(&sys->cpu, 0x0000);
    z80_set_de(&sys->cpu, 0x0000); z80_set_de_(&sys->cpu, 0x0000);
    z80_set_hl(&sys->cpu, 0x0000); z80_set_hl_(&sys->cpu, 0x0000);
    z80_set_af_(&sys->cpu, 0x0000);
    z80_set_pc(&sys->cpu, exec_addr);
}

/* KCC file format support */
typedef struct {
    uint8_t name[16];
    uint8_t num_addr;
    uint8_t load_addr_l;
    uint8_t load_addr_h;
    uint8_t end_addr_l;
    uint8_t end_addr_h;
    uint8_t exec_addr_l;
    uint8_t exec_addr_h;
    uint8_t pad[128 - 23];  /* pad to 128 bytes */
} _z9001_kcc_header;

/* KCC files cannot really be identified since they have no magic number */
static bool _z9001_is_valid_kcc(const uint8_t* ptr, int num_bytes) {
    if (num_bytes <= (int) sizeof (_z9001_kcc_header)) {
        return false;
    }
    const _z9001_kcc_header* hdr = (const _z9001_kcc_header*) ptr;
    for (int i = 0; i < 16; i++) {
        if (hdr->name[i] >= 128) {
            return false;
        }
    }
    if (hdr->num_addr > 3) {
        return false;
    }
    uint16_t load_addr = hdr->load_addr_h<<8 | hdr->load_addr_l;
    uint16_t end_addr = hdr->end_addr_h<<8 | hdr->end_addr_l;
    if (end_addr <= load_addr) {
        return false;
    }
    if (hdr->num_addr > 2) {
        uint16_t exec_addr = hdr->exec_addr_h<<8 | hdr->exec_addr_l;
        if ((exec_addr < load_addr) || (exec_addr > end_addr)) {
            return false;
        }
    }
    int required_len = (end_addr - load_addr) + sizeof(_z9001_kcc_header);
    if (required_len > num_bytes) {
        return false;
    }
    /* could be a KCC file */
    return true;
}

static bool _z9001_load_kcc(z9001_t* sys, const uint8_t* ptr) {
    const _z9001_kcc_header* hdr = (_z9001_kcc_header*) ptr;
    uint16_t addr = hdr->load_addr_h<<8 | hdr->load_addr_l;
    uint16_t end_addr  = hdr->end_addr_h<<8 | hdr->end_addr_l;
    ptr += sizeof(_z9001_kcc_header);
    while (addr < end_addr) {
        /* data is continuous */
        mem_wr(&sys->mem, addr++, *ptr++);
    }
    return false;
}

/* KC TAP file format support */
typedef struct {
    uint8_t sig[16];        /* "\xC3KC-TAPE by AF. " */
    uint8_t type;           /* 00: KCTAP_Z9001, 01: KCTAP_KC85, else: KCTAP_SYS */
    _z9001_kcc_header kcc;  /* from here on identical with KCC */
} _z9001_kctap_header;

static bool _z9001_is_valid_kctap(const uint8_t* ptr, int num_bytes) {
    if (num_bytes <= (int) sizeof (_z9001_kctap_header)) {
        return false;
    }
    const _z9001_kctap_header* hdr = (const _z9001_kctap_header*) ptr;
    static uint8_t sig[16] = { 0xC3,'K','C','-','T','A','P','E',0x20,'b','y',0x20,'A','F','.',0x20 };
    for (int i = 0; i < 16; i++) {
        if (sig[i] != hdr->sig[i]) {
            return false;
        }
    }
    if (hdr->kcc.num_addr > 3) {
        return false;
    }
    uint16_t load_addr = hdr->kcc.load_addr_h<<8 | hdr->kcc.load_addr_l;
    uint16_t end_addr = hdr->kcc.end_addr_h<<8 | hdr->kcc.end_addr_l;
    if (end_addr <= load_addr) {
        return false;
    }
    if (hdr->kcc.num_addr > 2) {
        uint16_t exec_addr = hdr->kcc.exec_addr_h<<8 | hdr->kcc.exec_addr_l;
        if ((exec_addr < load_addr) || (exec_addr > end_addr)) {
            return false;
        }
    }
    int required_len = (end_addr - load_addr) + sizeof(_z9001_kctap_header);
    if (required_len > num_bytes) {
        return false;
    }
    /* this appears to be a valid KC TAP file */
    return true;
}

static bool _z9001_load_kctap(z9001_t* sys, const uint8_t* ptr) {
    const _z9001_kctap_header* hdr = (const _z9001_kctap_header*) ptr;
    uint16_t addr = hdr->kcc.load_addr_h<<8 | hdr->kcc.load_addr_l;
    uint16_t end_addr  = hdr->kcc.end_addr_h<<8 | hdr->kcc.end_addr_l;
    ptr += sizeof(_z9001_kctap_header);
    while (addr < end_addr) {
        /* each block is 1 lead-byte + 128 bytes data */
        ptr++;
        for (int i = 0; i < 128; i++) {
            mem_wr(&sys->mem, addr++, *ptr++);
        }
    }
    /* if file has an exec-address, start the program */
    if (hdr->kcc.num_addr > 2) {
        _z9001_load_start(sys, hdr->kcc.exec_addr_h<<8 | hdr->kcc.exec_addr_l);
    }
    return true;
}

bool z9001_quickload(z9001_t* sys, const uint8_t* ptr, int num_bytes) {
    CHIPS_ASSERT(sys && sys->valid && ptr);
    /* first check for KC TAP, since this can be properly identified */
    if (_z9001_is_valid_kctap(ptr, num_bytes)) {
        return _z9001_load_kctap(sys, ptr);
    }
    else if (_z9001_is_valid_kcc(ptr, num_bytes)) {
        return _z9001_load_kcc(sys, ptr);
    }
    else {
        /* not a known file type, or not enough data */
        return false;
    }
}

#endif /* CHIPS_IMPL */
