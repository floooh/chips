#pragma once
/*#
    # kc85.h

    A KC85/2, /3 and /4 emulator in a C header.

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

    You need to include the following headers before including kc85.h:

    - chips/z80.h
    - chips/z80ctc.h
    - chips/z80pio.h
    - chips/beeper.h
    - chips/kbd.h
    - chips/mem.h
    - chips/clk.h

    ## The KC85/2

    TODO!

    ## The KC85/3

    TODO!

    ## The KC85/4

    TODO!

    ## TODO:

    - fix 'slowdown' problem triggered by the SERIOUS.KCC demo
    - optionally proper keyboard emulation (the current implementation
      uses a shortcut to directly write the key code into a memory address)
    - video-decoding is currently per-scanline
    - KC85/4 pixel-color mode
    - wait states for video RAM access
    - display needling
    - the READ-ONLY mapping bits for RAM banks are ignored
    - audio volume is currently not implemented

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

#define KC85_DISPLAY_WIDTH (320)
#define KC85_DISPLAY_HEIGHT (256)
#define KC85_MAX_AUDIO_SAMPLES (1024)       /* max number of audio samples in internal sample buffer */
#define KC85_DEFAULT_AUDIO_SAMPLES (128)    /* default number of samples in internal sample buffer */ 
#define KC85_MAX_TAPE_SIZE (64 * 1024)      /* max size of a snapshot file in bytes */

/* KC85 model types */
typedef enum {
    KC85_TYPE_2,        /* KC85/2 (default) */
    KC85_TYPE_3,        /* KC85/3 */
    KC85_TYPE_4,        /* KC85/4 */
} kc85_type_t;

/* audio sample callback */
typedef void (*kc85_audio_callback_t)(const float* samples, int num_samples, void* user_data);

/* config parameters for kc85_init() */
typedef struct {
    kc85_type_t type;           /* default is KC85_TYPE_2 */

    /* video output config */
    void* pixel_buffer;         /* pointer to a linear RGBA8 pixel buffer, at least 320*256*4 bytes */
    int pixel_buffer_size;      /* size of the pixel buffer in bytes */

    /* optional user-data for callback functions */
    void* user_data;

    /* audio output config (if you don't want audio, set audio_cb to zero) */
    kc85_audio_callback_t audio_cb;     /* called when audio_num_samples are ready */
    int audio_num_samples;              /* default is KC85_AUDIO_NUM_SAMPLES */
    int audio_sample_rate;              /* playback sample rate, default is 44100 */
    float audio_volume;                 /* audio volume (0.0 .. 1.0), default is 0.5 */

    /* ROM images */
    const void* rom_caos22;             /* CAOS 2.2 (used in KC85/2) */
    const void* rom_caos31;             /* CAOS 3.1 (used in KC85/3) */
    const void* rom_caos42c;            /* CAOS 4.2 at 0xC000 (KC85/4) */
    const void* rom_caos42e;            /* CAOS 4.2 at 0xE000 (KC85/4) */
    const void* rom_kcbasic;            /* same BASIC version for KC85/3 and KC85/4 */
    int rom_caos22_size;
    int rom_caos31_size;
    int rom_caos42c_size;
    int rom_caos42e_size;
    int rom_kcbasic_size;
} kc85_desc_t;

/* KC85 emulator state */
typedef struct {
    z80_t cpu;
    z80ctc_t ctc;
    z80pio_t pio;
    beeper_t beeper_1;
    beeper_t beeper_2;

    bool valid;
    kc85_type_t type;
    uint8_t pio_a;          /* current PIO-A value, used for bankswitching */
    uint8_t pio_b;          /* current PIO-B value, used for bankswitching */
    uint8_t io84;           /* byte latch at port 0x84, only on KC85/4 */
    uint8_t io86;           /* byte latch at port 0x86, only on KC85/4 */
    bool blink_flag;        /* foreground color blinking flag toggled by CTC */

    int scanline_period;
    int scanline_counter;
    int cur_scanline;

    clk_t clk;
    kbd_t kbd;
    mem_t mem;

    uint32_t* pixel_buffer;
    void* user_data;
    kc85_audio_callback_t audio_cb;
    int num_samples;
    int sample_pos;
    float sample_buffer[KC85_MAX_AUDIO_SAMPLES];

    uint8_t ram[8][0x4000];         /* up to 8 16-KByte RAM banks */
    uint8_t rom_basic[0x2000];      /* 8 KByte BASIC ROM (KC85/3 and /4 only) */
    uint8_t rom_caos_c[0x1000];     /* 4 KByte CAOS ROM at 0xC000 (KC85/4 only) */
    uint8_t rom_caos_e[0x2000];     /* 8 KByte CAOS ROM at 0xE000 */
} kc85_t;

/* initialize a new KC85 instance */
void kc85_init(kc85_t* sys, const kc85_desc_t* desc);
/* discard a KC85 instance */
void kc85_discard(kc85_t* sys);
/* reset a KC85 instance */
void kc85_reset(kc85_t* sys);
/* run KC85 emulation for a given time in seconds */
void kc85_exec(kc85_t* sys, double seconds);
/* send a key-down event */
void kc85_key_down(kc85_t* sys, int key_code);
/* send a key-up event */
void kc85_key_up(kc85_t* sys, int key_code);
/* load a .KCC or .TAP snapshot file into the emulator */
void kc85_quickload(kc85_t* sys, const uint8_t* ptr, int num_bytes);

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

#define _KC85_DISPLAY_SIZE (KC85_DISPLAY_WIDTH*KC85_DISPLAY_HEIGHT*4)
#define _KC85_2_3_FREQUENCY (1750000)
#define _KC85_4_FREQUENCY (1770000)
#define _KC85_IRM0_PAGE (4)
#define _KC85_NUM_SCANLINES (312)

/* IO bits */
#define _KC85_PIO_A_CAOS_ROM        (1<<0)
#define _KC85_PIO_A_RAM             (1<<1)
#define _KC85_PIO_A_IRM             (1<<2)
#define _KC85_PIO_A_RAM_RO          (1<<3)
#define _KC85_PIO_A_UNUSED          (1<<4)
#define _KC85_PIO_A_TAPE_LED        (1<<5)
#define _KC85_PIO_A_TAPE_MOTOR      (1<<6)
#define _KC85_PIO_A_BASIC_ROM       (1<<7)
#define _KC85_PIO_B_VOLUME_MASK     ((1<<5)-1)
#define _KC85_PIO_B_RAM8            (1<<5)  /* KC85/4 only */
#define _KC85_PIO_B_RAM8_RO         (1<<6)  /* KC85/4 only */
#define _KC85_PIO_B_BLINK_ENABLED   (1<<7)
/* KC85/4 only IO latches */
#define _KC85_IO84_SEL_VIEW_IMG     (1<<0)  /* 0: display img0, 1: display img1 */
#define _KC85_IO84_SEL_CPU_COLOR    (1<<1)  /* 0: access pixels, 1: access colors */
#define _KC85_IO84_SEL_CPU_IMG      (1<<2)  /* 0: access img0, 1: access img1 */
#define _KC85_IO84_HICOLOR          (1<<3)  /* 0: hicolor mode off, 1: hicolor mode on */
#define _KC85_IO84_SEL_RAM8         (1<<4)  /* select RAM8 block 0 or 1 */
#define _KC85_IO84_BLOCKSEL_RAM8    (1<<5)  /* no idea what that does...? */
#define _KC85_IO86_RAM4             (1<<0)
#define _KC85_IO86_RAM4_RO          (1<<1)
#define _KC85_IO86_CAOS_ROM_C       (1<<7)

static uint64_t _kc85_tick(int num, uint64_t pins, void* user_data);
static uint8_t _kc85_pio_in(int port_id, void* user_data);
static void _kc85_pio_out(int port_id, uint8_t data, void* user_data);
static void _kc85_decode_scanline(kc85_t* sys);
static void _kc85_update_memory_map(kc85_t* sys);
static void _kc85_init_memory_map(kc85_t* sys);
static void _kc85_handle_keyboard(kc85_t* sys);

/* xorshift randomness for memory initialization */
static inline uint32_t _kc85_xorshift32(uint32_t x) {
    x ^= x<<13;
    x ^= x>>17;
    x ^= x<<5;
    return x;
}

#define _KC85_DEFAULT(val,def) (((val) != 0) ? (val) : (def));

void kc85_init(kc85_t* sys, const kc85_desc_t* desc) {
    CHIPS_ASSERT(sys && desc);

    memset(sys, 0, sizeof(kc85_t));
    sys->valid = true;
    sys->type = desc->type;

    /* copy ROM images */
    if (desc->type == KC85_TYPE_2) {
        /* KC85/2 only has an 8 KByte OS ROM */
        CHIPS_ASSERT(desc->rom_caos22 && (desc->rom_caos22_size == sizeof(sys->rom_caos_e)));
        memcpy(sys->rom_caos_e, desc->rom_caos22, sizeof(sys->rom_caos_e));
    }
    else if (desc->type == KC85_TYPE_3) {
        /* KC85/3 has 8 KByte BASIC ROM and 8 KByte OS ROM */
        CHIPS_ASSERT(desc->rom_kcbasic && (desc->rom_kcbasic_size == sizeof(sys->rom_basic)));
        memcpy(sys->rom_basic, desc->rom_kcbasic, sizeof(sys->rom_basic));
        CHIPS_ASSERT(desc->rom_caos31 && (desc->rom_caos31_size == sizeof(sys->rom_caos_e)));
        memcpy(sys->rom_caos_e, desc->rom_caos31, sizeof(sys->rom_caos_e));
    }
    else {
        /* KC85/4 has 8 KByte BASIC ROM, and 2 OS ROMs (4 KB and 8 KB) */
        CHIPS_ASSERT(desc->rom_kcbasic && (desc->rom_kcbasic_size == sizeof(sys->rom_basic)));
        memcpy(sys->rom_basic, desc->rom_kcbasic, sizeof(sys->rom_basic));
        CHIPS_ASSERT(desc->rom_caos42c && (desc->rom_caos42c_size == sizeof(sys->rom_caos_c)));
        memcpy(sys->rom_caos_c, desc->rom_caos42c, sizeof(sys->rom_caos_c));
        CHIPS_ASSERT(desc->rom_caos42e && (desc->rom_caos42e_size == sizeof(sys->rom_caos_e)));
        memcpy(sys->rom_caos_e, desc->rom_caos42e, sizeof(sys->rom_caos_e));
    }

    /* fill RAM with noise */
    {
        uint32_t r = 0x6D98302B;
        uint8_t* ptr = &sys->ram[0][0];
        for (int i = 0; i < (int)sizeof(sys->ram);) {
            r = _kc85_xorshift32(r);
            ptr[i++] = r;
            ptr[i++] = (r>>8);
            ptr[i++] = (r>>16);
            ptr[i++] = (r>>24);
        }
    }

    /* video- and audio-output */
    CHIPS_ASSERT(desc->pixel_buffer && (desc->pixel_buffer_size >= _KC85_DISPLAY_SIZE));
    sys->pixel_buffer = desc->pixel_buffer;
    sys->audio_cb = desc->audio_cb;
    sys->user_data = desc->user_data;
    sys->num_samples = _KC85_DEFAULT(desc->audio_num_samples, KC85_DEFAULT_AUDIO_SAMPLES);
    CHIPS_ASSERT(sys->num_samples <= KC85_MAX_AUDIO_SAMPLES);

    /* initialize the hardware */
    const uint32_t freq_hz = (sys->type == KC85_TYPE_4) ? _KC85_4_FREQUENCY : _KC85_2_3_FREQUENCY;
    clk_init(&sys->clk, freq_hz);
    z80ctc_init(&sys->ctc);

    z80_desc_t cpu_desc = {0};
    cpu_desc.tick_cb = _kc85_tick;
    cpu_desc.user_data = sys;
    z80_init(&sys->cpu, &cpu_desc);

    z80pio_desc_t pio_desc = {0};
    pio_desc.in_cb = _kc85_pio_in;
    pio_desc.out_cb = _kc85_pio_out;
    pio_desc.user_data = sys;
    z80pio_init(&sys->pio, &pio_desc);

    const int audio_hz = _KC85_DEFAULT(desc->audio_sample_rate, 44100);
    const float audio_vol = _KC85_DEFAULT(desc->audio_volume, 0.5f);
    beeper_init(&sys->beeper_1, freq_hz, audio_hz, audio_vol);
    beeper_init(&sys->beeper_2, freq_hz, audio_hz, audio_vol);

    sys->scanline_period = (sys->type == KC85_TYPE_4) ? 113 : 112;
    sys->scanline_counter = sys->scanline_period;
    _kc85_init_memory_map(sys);

    /* the kbd_t helper functions are only used as a simple keystroke buffer,
       the KC85 doesn't have a typical keyboard matrix in the in the
       main unit, since key presses were serially encoded/decoded, currently
       this isn't emulated (see _kc85_handle_keyboard)
    */
    kbd_init(&sys->kbd, 1);

    /* execution on power-up starts at 0xF000 */
    z80_set_pc(&sys->cpu, 0xF000);
}

void kc85_discard(kc85_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    sys->valid = false;
}

void kc85_reset(kc85_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    z80_reset(&sys->cpu);
    z80ctc_reset(&sys->ctc);
    z80pio_reset(&sys->pio);
    beeper_reset(&sys->beeper_1);
    beeper_reset(&sys->beeper_2);
    sys->pio_a = 0;
    sys->pio_b = 0;
    sys->io84 = 0;
    sys->io86 = 0;
    sys->cur_scanline = 0;
    sys->scanline_counter = sys->scanline_period;

    /* execution after reset starts at 0xF000 */
    z80_set_pc(&sys->cpu, 0xE000);
}

void kc85_exec(kc85_t* sys, double seconds) {
    CHIPS_ASSERT(sys && sys->valid);
    uint32_t ticks_to_run = clk_ticks_to_run(&sys->clk, seconds);
    uint32_t ticks_executed = z80_exec(&sys->cpu, ticks_to_run);
    clk_ticks_executed(&sys->clk, ticks_executed);
    kbd_update(&sys->kbd);
    _kc85_handle_keyboard(sys);
}

void kc85_key_down(kc85_t* sys, int key_code) {
    CHIPS_ASSERT(sys && sys->valid);
    kbd_key_down(&sys->kbd, key_code);
}

void kc85_key_up(kc85_t* sys, int key_code) {
    CHIPS_ASSERT(sys && sys->valid);
    kbd_key_up(&sys->kbd, key_code);
}

static uint64_t _kc85_tick(int num_ticks, uint64_t pins, void* user_data) {
    kc85_t* sys = (kc85_t*) user_data;

    /* video decoding */
    sys->scanline_counter -= num_ticks;
    if (sys->scanline_counter <= 0) {
        sys->scanline_counter += sys->scanline_period;
        if (sys->cur_scanline < KC85_DISPLAY_HEIGHT) {
            _kc85_decode_scanline(sys);
        }
        sys->cur_scanline++;
        /* vertical blank signal? this triggers CTC2 for the video blinking effect */
        if (sys->cur_scanline >= _KC85_NUM_SCANLINES) {
            sys->cur_scanline = 0;
            pins |= Z80CTC_CLKTRG2;
        }
    }

    /* tick the CTC */
    for (int i = 0; i < num_ticks; i++) {
        pins = z80ctc_tick(&sys->ctc, pins);
        /* CTC channels 0 and 1 triggers control audio frequencies */
        if (pins & Z80CTC_ZCTO0) {
            beeper_toggle(&sys->beeper_1);
        }
        if (pins & Z80CTC_ZCTO1) {
            beeper_toggle(&sys->beeper_2);
        }
        /* CTC channel 2 trigger controls video blink frequency */
        if (pins & Z80CTC_ZCTO2) {
            sys->blink_flag = !sys->blink_flag;
        }
        pins &= Z80_PIN_MASK;
        beeper_tick(&sys->beeper_1);
        if (beeper_tick(&sys->beeper_2)) {
            /* new audio sample ready */
            sys->sample_buffer[sys->sample_pos++] = sys->beeper_1.sample + sys->beeper_2.sample;
            if (sys->sample_pos == sys->num_samples) {
                if (sys->audio_cb) {
                    sys->audio_cb(sys->sample_buffer, sys->num_samples, sys->user_data);
                }
                sys->sample_pos = 0;
            }
        }
    }    

    /* memory and IO requests */
    if (pins & Z80_MREQ) {
        /* memory request machine cycle */
        const uint16_t addr = Z80_GET_ADDR(pins);
        if (pins & Z80_RD) {
            Z80_SET_DATA(pins, mem_rd(&sys->mem, addr));
        }
        else if (pins & Z80_WR) {
            mem_wr(&sys->mem, addr, Z80_GET_DATA(pins));
        }
    }
    else if (pins & Z80_IORQ) {
        /*
            IO request machine cycle

            on the KC85/3, the chips-select signals for the CTC and PIO
            are generated through logic gates, on KC85/4 this is implemented
            with a PROM chip (details are in the KC85/3 and KC85/4 service manuals)

            the I/O addresses are as follows:

                 0x88:   PIO Port A, data
                 0x89:   PIO Port B, data
                 0x8A:   PIO Port A, control
                 0x8B:   PIO Port B, control
                 0x8C:   CTC Channel 0
                 0x8D:   CTC Channel 0
                 0x8E:   CTC Channel 0
                 0x8F:   CTC Channel 0

                 0x80:   controls the expansion module system, the upper
                         8-bits of the port number address the module slot
                 0x84:   (KC85/4 only) control the vide memory bank switching
                 0x86:   (KC85/4 only) control RAM block at 0x4000 and ROM switching
        */

        /* check if any of the valid port numbers is addressed (0x80..0x8F) */
        if ((pins & (Z80_A7|Z80_A6|Z80_A5|Z80_A4)) == Z80_A7) {
            /* check if the PIO or CTC is addressed (0x88 to 0x8F) */
            if (pins & Z80_A3) {
                pins &= Z80_PIN_MASK;
                /* bit A2 selects the PIO or CTC */
                if (pins & Z80_A2) {
                    /* a CTC IO request */
                    pins |= Z80CTC_CE;
                    if (pins & Z80_A0) { pins |= Z80CTC_CS0; }
                    if (pins & Z80_A1) { pins |= Z80CTC_CS1; }
                    pins = z80ctc_iorq(&sys->ctc, pins) & Z80_PIN_MASK;
                }
                else {
                    /* a PIO IO request */
                    pins |= Z80PIO_CE;
                    if (pins & Z80_A0) { pins |= Z80PIO_BASEL; }
                    if (pins & Z80_A1) { pins |= Z80PIO_CDSEL; }
                    pins = z80pio_iorq(&sys->pio, pins) & Z80_PIN_MASK;
                }
            }
            else {
                /* we're in range 0x80..0x87 */
                const uint8_t data = Z80_GET_DATA(pins);
                switch (pins & (Z80_A2|Z80_A1|Z80_A0)) {
                    case 0x00:
                        /*
                            port 0x80: expansion module control, high byte
                            of port address contains module slot address
                        */
                        /* FIXME
                        {
                            const uint8_t slot_addr = Z80_GET_ADDR(pins)>>8;
                            if (pins & Z80_WR) {
                                if (kc85.exp.slot_exists(slot_addr)) {
                                    kc85.exp.update_control_byte(slot_addr, data);
                                    kc85.update_bank_switching();
                                }
                            }
                            else {
                                Z80_SET_DATA(pins, kc85.exp.module_type_in_slot(slot_addr));
                            }
                        }
                        */
                        break;

                    case 0x04:
                        /* port 0x84, KC85/4 only, this is a write-only 8-bit latch */
                        if ((KC85_TYPE_4 == sys->type) && (pins & Z80_WR)) {
                            sys->io84 = data;
                            _kc85_update_memory_map(sys);
                        }
                        break;

                    case 0x06:
                        /* port 0x86, KC85/4 only, this is a write-only 8-bit latch */
                        if ((KC85_TYPE_4 == sys->type) && (pins & Z80_WR)) {
                            sys->io86 = data;
                            _kc85_update_memory_map(sys);
                        }
                        break;
                }
            }
        }
    }

    /* interrupt daisy chain, CTC is higher priority then PIO */
    Z80_DAISYCHAIN_BEGIN(pins)
    {
        pins = z80ctc_int(&sys->ctc, pins);
        pins = z80pio_int(&sys->pio, pins);
    }
    Z80_DAISYCHAIN_END(pins);
    
    return (pins & Z80_PIN_MASK);    
}

static uint8_t _kc85_pio_in(int port_id, void* user_data) {
    return 0xFF;
}

static void _kc85_pio_out(int port_id, uint8_t data, void* user_data) {
    kc85_t* sys = (kc85_t*) user_data;
    if (Z80PIO_PORT_A == port_id) {
        sys->pio_a = data;
    }
    else {
        sys->pio_b = data;
        /* FIXME: audio volume */
    }
    _kc85_update_memory_map(sys);
}

/* hardwired foreground colors */
static uint32_t _kc85_fg_pal[16] = {
    0xFF000000,     /* black */
    0xFFFF0000,     /* blue */
    0xFF0000FF,     /* red */
    0xFFFF00FF,     /* magenta */
    0xFF00FF00,     /* green */
    0xFFFFFF00,     /* cyan */
    0xFF00FFFF,     /* yellow */
    0xFFFFFFFF,     /* white */
    0xFF000000,     /* black #2 */
    0xFFFF00A0,     /* violet */
    0xFF00A0FF,     /* orange */
    0xFFA000FF,     /* purple */
    0xFFA0FF00,     /* blueish green */
    0xFFFFA000,     /* greenish blue */
    0xFF00FFA0,     /* yellow-green */
    0xFFFFFFFF,     /* white #2 */
};

// background colors
static uint32_t _kc85_bg_pal[8] = {
    0xFF000000,      /* black */
    0xFFA00000,      /* dark-blue */
    0xFF0000A0,      /* dark-red */
    0xFFA000A0,      /* dark-magenta */
    0xFF00A000,      /* dark-green */
    0xFFA0A000,      /* dark-cyan */
    0xFF00A0A0,      /* dark-yellow */
    0xFFA0A0A0,      /* gray */
};

static inline void _kc85_decode_8pixels(uint32_t* ptr, uint8_t pixels, uint8_t colors, bool blink_bg) {
    /*
        select foreground- and background color:
        bit 7: blinking
        bits 6..3: foreground color
        bits 2..0: background color

        index 0 is background color, index 1 is foreground color
    */
    const uint8_t bg_index = colors & 0x7;
    const uint8_t fg_index = (colors>>3)&0xF;
    const unsigned int bg = _kc85_bg_pal[bg_index];
    const unsigned int fg = (blink_bg && (colors & 0x80)) ? bg : _kc85_fg_pal[fg_index];
    ptr[0] = pixels & 0x80 ? fg : bg;
    ptr[1] = pixels & 0x40 ? fg : bg;
    ptr[2] = pixels & 0x20 ? fg : bg;
    ptr[3] = pixels & 0x10 ? fg : bg;
    ptr[4] = pixels & 0x08 ? fg : bg;
    ptr[5] = pixels & 0x04 ? fg : bg;
    ptr[6] = pixels & 0x02 ? fg : bg;
    ptr[7] = pixels & 0x01 ? fg : bg;   
}

static void _kc85_decode_scanline(kc85_t* sys) {
    const int y = sys->cur_scanline;
    const bool blink_bg = sys->blink_flag && (sys->pio_b & _KC85_PIO_B_BLINK_ENABLED);
    const int width = KC85_DISPLAY_WIDTH>>3;
    unsigned int* dst_ptr = &(sys->pixel_buffer[y*KC85_DISPLAY_WIDTH]);
    if (KC85_TYPE_4 == sys->type) {
        int irm_index = (sys->io84 & 1) * 2;
        const uint8_t* pixel_data = sys->ram[_KC85_IRM0_PAGE + irm_index];
        const uint8_t* color_data = sys->ram[_KC85_IRM0_PAGE + irm_index + 1];
        for (int x = 0; x < width; x++) {
            int offset = y | (x<<8);
            uint8_t src_pixels = pixel_data[offset];
            uint8_t src_colors = color_data[offset];
            _kc85_decode_8pixels(&(dst_ptr[x<<3]), src_pixels, src_colors, blink_bg);
        }
    }
    else {
        const uint8_t* pixel_data = sys->ram[_KC85_IRM0_PAGE];
        const uint8_t* color_data = sys->ram[_KC85_IRM0_PAGE] + 0x2800;
        const int left_pixel_offset  = (((y>>2)&0x3)<<5) | ((y&0x3)<<7) | (((y>>4)&0xF)<<9);
        const int left_color_offset  = (((y>>2)&0x3f)<<5);
        const int right_pixel_offset = (((y>>4)&0x3)<<3) | (((y>>2)&0x3)<<5) | ((y&0x3)<<7) | (((y>>6)&0x3)<<9);
        const int right_color_offset = (((y>>4)&0x3)<<3) | (((y>>2)&0x3)<<5) | (((y>>6)&0x3)<<7);
        int pixel_offset, color_offset;
        for (int x = 0; x < width; x++) {
            if (x < 0x20) {
                /* left 256x256 quad */
                pixel_offset = x | left_pixel_offset;
                color_offset = x | left_color_offset;
            }
            else {
                /* right 64x256 strip */
                pixel_offset = 0x2000 + ((x&0x7) | right_pixel_offset);
                color_offset = 0x0800 + ((x&0x7) | right_color_offset);
            }
            uint8_t src_pixels = pixel_data[pixel_offset];
            uint8_t src_colors = color_data[color_offset];
            _kc85_decode_8pixels(&(dst_ptr[x<<3]), src_pixels, src_colors, blink_bg);
        }
    }
}

static void _kc85_init_memory_map(kc85_t* sys) {
    mem_init(&sys->mem);

    /* intitial memory config */
    sys->pio_a = _KC85_PIO_A_RAM | _KC85_PIO_A_IRM | _KC85_PIO_A_CAOS_ROM;
    if (KC85_TYPE_3 == sys->type) {
        sys->pio_a |= _KC85_PIO_A_BASIC_ROM;
    }
    _kc85_update_memory_map(sys);
}

static void _kc85_update_memory_map(kc85_t* sys) {
    /* all models have 16 KB builtin RAM at 0x0000 and 8 KB ROM at 0xE000 */
    if (sys->pio_a & _KC85_PIO_A_RAM) {
        mem_map_ram(&sys->mem, 0, 0x0000, 0x4000, sys->ram[0]);
    }
    if (sys->pio_a & _KC85_PIO_A_CAOS_ROM) {
        mem_map_rom(&sys->mem, 0, 0xE000, 0x2000, sys->rom_caos_e);
    }

    /* KC85/3 and KC85/4: builtin 8 KB BASIC ROM at 0xC000 */
    if (sys->type != KC85_TYPE_2) {
        if (sys->pio_a & _KC85_PIO_A_BASIC_ROM) {
            mem_map_rom(&sys->mem, 0, 0xC000, 0x2000, sys->rom_basic);
        }
    }

    if (sys->type != KC85_TYPE_4) { /* KC85/2 and /3 */
        /* 16 KB Video RAM at 0x8000 */
        if (sys->pio_a & _KC85_PIO_A_IRM) {
            mem_map_ram(&sys->mem, 0, 0x8000, 0x4000, sys->ram[_KC85_IRM0_PAGE]);
        }
    }
    else { /* KC85/4 */
        /* 16 KB RAM at 0x4000 */
        if (sys->io86 & _KC85_IO86_RAM4) {
            mem_map_ram(&sys->mem, 0, 0x4000, 0x4000, sys->ram[1]);
        }
        /* 16 KB RAM at 0x8000 (2 banks) */
        if (sys->pio_b & _KC85_PIO_B_RAM8) {
            /* select one of two RAM banks */
            uint8_t* ram8_ptr = (sys->io84 & _KC85_IO84_SEL_RAM8) ? sys->ram[3] : sys->ram[2];
            mem_map_ram(&sys->mem, 0, 0x8000, 0x4000, ram8_ptr);
        }
        /* video memory is 4 banks, 2 for pixels, 2 for colors,
            the area at 0xA800 to 0xBFFF is alwazs mapped to IRM0!
        */
        if (sys->pio_a & _KC85_PIO_A_IRM) {
            uint32_t irm_index = (sys->io84 & 6)>>1;
            uint8_t* irm_ptr = sys->ram[_KC85_IRM0_PAGE + irm_index];
            /* on the KC85, an access to IRM banks other than the
              first is only possible for the first 10 KByte until
              A800, memory access to the remaining 6 KBytes
              (A800 to BFFF) is always forced to the first IRM bank
              by the address decoder hardware (see KC85/4 service manual)
            */
            mem_map_ram(&sys->mem, 0, 0x8000, 0x2800, irm_ptr);

            /* always force access to 0xA800 and above to the first IRM bank */
            mem_map_ram(&sys->mem, 0, 0xA800, 0x1800, sys->ram[_KC85_IRM0_PAGE] + 0x2800);
       }
       /* 4 KB CAOS-C ROM at 0xC000 (on top of BASIC) */
       if (sys->io86 & _KC85_IO86_CAOS_ROM_C) {
           mem_map_rom(&sys->mem, 0, 0xC000, 0x1000, sys->rom_caos_c);
       }
    }
}

/* keyboard emulation */
#define _KC85_KBD_TIMEOUT (1<<3)
#define _KC85_KBD_KEYREADY (1<<0)
#define _KC85_KBD_REPEAT (1<<4)
#define _KC85_KBD_SHORT_REPEAT_COUNT (8)
#define _KC85_KBD_LONG_REPEAT_COUNT (60)

static void _kc85_handle_keyboard(kc85_t* sys) {
    /*
        this is a simplified version of the PIO-B interrupt service routine
        which is normally triggered when the serial keyboard hardware
        sends a new pulse (for details, see
        https://github.com/floooh/yakc/blob/master/misc/kc85_3_kbdint.md )
    
        we ignore the whole tricky serial decoding and patch the
        keycode directly into the right memory locations.
    */

    /* don't do anything if interrupts disabled, IX might point to the wrong base address! */
    if (!z80_iff1(&sys->cpu)) {
        return;
    }

    /* get the first valid key code from the key buffer */
    uint8_t key_code = 0;
    for (int i = 0; i < KBD_MAX_PRESSED_KEYS; i++) {
        if (sys->kbd.key_buffer[i].key != 0) {
            key_code = sys->kbd.key_buffer[i].key;
            break;
        }
    }

    const uint16_t ix = z80_ix(&sys->cpu);
    if (0 == key_code) {
        /* if keycode is 0, this basically means the CTC3 timeout was hit */
        mem_wr(&sys->mem, ix+0x8, mem_rd(&sys->mem, ix+0x8) | _KC85_KBD_TIMEOUT); /* set the CTC3 timeout bit */
        mem_wr(&sys->mem, ix+0xD, 0); /* clear current keycode */
    }
    else {
        /* a valid keycode has been received, clear the timeout bit */
        mem_wr(&sys->mem, ix+0x8, mem_rd(&sys->mem, ix+0x8) & ~_KC85_KBD_TIMEOUT);

        /* check for key-repeat */
        if (key_code != mem_rd(&sys->mem, ix+0xD)) {
            /* no key-repeat */
            mem_wr(&sys->mem, ix+0xD, key_code);                                    /* write new keycode */
            mem_wr(&sys->mem, ix+0x8, mem_rd(&sys->mem, ix+0x8)&~_KC85_KBD_REPEAT);  /* clear the first-key-repeat bit */
            mem_wr(&sys->mem, ix+0x8, mem_rd(&sys->mem, ix+0x8)|_KC85_KBD_KEYREADY); /* set the key-ready bit */
            mem_wr(&sys->mem, ix+0xA, 0);                                           /* clear the key-repeat counter */
        }
        else {
            /* handle key-repeat */
            mem_wr(&sys->mem, ix+0xA, mem_rd(&sys->mem, ix+0xA)+1);   /* increment repeat-pause-counter */
            if (mem_rd(&sys->mem, ix+0x8) & _KC85_KBD_REPEAT) {
                /* this is a followup, short key-repeat */
                if (mem_rd(&sys->mem, ix+0xA) < _KC85_KBD_SHORT_REPEAT_COUNT) {
                    /* wait some more... */
                    return;
                }
            }
            else {
                /* this is the first, long key-repeat */
                if (mem_rd(&sys->mem, ix+0xA) < _KC85_KBD_LONG_REPEAT_COUNT) {
                    // wait some more...
                    return;
                }
                else {
                    // first key-repeat pause over, set first-key-repeat flag
                    mem_wr(&sys->mem, ix+0x8, mem_rd(&sys->mem, ix+0x8)|_KC85_KBD_REPEAT);
                }
            }
            /* key-repeat triggered, just set the key-ready flag and reset repeat-count */
            mem_wr(&sys->mem, ix+0x8, mem_rd(&sys->mem, ix+0x8)|_KC85_KBD_KEYREADY);
            mem_wr(&sys->mem, ix+0xA, 0);
        }
    }
}

#endif /* CHIPS_IMPL */
