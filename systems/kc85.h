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
#define KC85_NUM_SLOTS (2)                  /* 2 expansion slots in main unit, each needs one mem_t layer! */
#define KC85_EXP_BUFSIZE (KC85_NUM_SLOTS*64*1024) /* expansion system buffer size (64 KB per slot) */

/* KC85 model types */
typedef enum {
    KC85_TYPE_2,        /* KC85/2 (default) */
    KC85_TYPE_3,        /* KC85/3 */
    KC85_TYPE_4,        /* KC85/4 */
} kc85_type_t;

/* audio sample callback */
typedef void (*kc85_audio_callback_t)(const float* samples, int num_samples, void* user_data);
/* callback to apply patches after a snapshot is loaded */
typedef void (*kc85_patch_callback_t)(const char* snapshot_name, void* user_data);

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
    
    /* an optional callback to be invoked after a snapshot file is loaded to apply patches */
    kc85_patch_callback_t patch_cb;

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

/* KC85 expansion module types */
typedef enum {
    KC85_MODULE_NONE,
    KC85_MODULE_M006_BASIC,         /* BASIC+CAOS 16K ROM module for the KC85/2 (id=0xFC) */
    KC85_MODULE_M011_64KBYE,        /* 64 KByte RAM expansion (id=0xF6) */
    KC85_MODULE_M012_TEXOR,         /* TEXOR text editing (id=0xFB) */
    KC85_MODULE_M022_16KBYTE,       /* 16 KByte RAM expansion (id=0xF4) */
    KC85_MODULE_M026_FORTH,         /* FORTH IDE (id=0xFB) */
    KC85_MODULE_M027_DEVELOPMENT,   /* Assembler IDE (id=0xFB) */
} kc85_module_type_t;

/* KC85 expansion module attributes */
typedef struct {
    kc85_module_type_t type;
    uint8_t id;                     /* id of currently inserted module */
    bool writable;                  /* RAM or ROM module */
    uint8_t addr_mask;              /* the module's address mask */
    int size;                       /* the module's byte size */
} kc85_module_t;

/* KC85 expansion slot */
typedef struct {
    uint8_t addr;                   /* 0x0C (left slot) or 0x08 (right slot) */
    uint8_t ctrl;                   /* current control byte */
    uint32_t buf_offset;            /* byte-offset in expansion system data buffer */
    kc85_module_t mod;              /* attributes of currently inserted module */
} kc85_slot_t;

/* KC85 expansion system state:
    NOTE that each expansion slot needs its own memory-mapping layer starting 
    at layer 1 (layer 0 is used by the base system)
*/
typedef struct {
    kc85_slot_t slot[KC85_NUM_SLOTS];   /* slots 0x08 and 0x0C in KC85 main unit */
    uint32_t buf_top;                   /* offset of free area in expansion buffer (kc85_t.exp_buf[]) */
} kc85_exp_t;

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
    kc85_exp_t exp;         /* expansion module system */

    uint32_t* pixel_buffer;
    void* user_data;
    kc85_audio_callback_t audio_cb;
    int num_samples;
    int sample_pos;
    float sample_buffer[KC85_MAX_AUDIO_SAMPLES];
    kc85_patch_callback_t patch_cb;

    uint8_t ram[8][0x4000];             /* up to 8 16-KByte RAM banks */
    uint8_t rom_basic[0x2000];          /* 8 KByte BASIC ROM (KC85/3 and /4 only) */
    uint8_t rom_caos_c[0x1000];         /* 4 KByte CAOS ROM at 0xC000 (KC85/4 only) */
    uint8_t rom_caos_e[0x2000];         /* 8 KByte CAOS ROM at 0xE000 */
    uint8_t exp_buf[KC85_EXP_BUFSIZE];  /* expansion system RAM/ROM */
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
/* insert a RAM module (slot must be 0x08 or 0x0C) */
bool kc85_insert_ram_module(kc85_t* sys, uint8_t slot, kc85_module_type_t type);
/* insert a ROM module (slot must be 0x08 or 0x0C) */
bool kc85_insert_rom_module(kc85_t* sys, uint8_t slot, kc85_module_type_t type, const void* rom_ptr, int rom_size);
/* remove module in slot */
bool kc85_remove_module(kc85_t* sys, uint8_t slot);
/* load a .KCC or .TAP snapshot file into the emulator */
bool kc85_quickload(kc85_t* sys, const uint8_t* ptr, int num_bytes);

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

/* expansion module private functions */
static void _kc85_exp_init(kc85_t* sys);
static void _kc85_exp_reset(kc85_t* sys);
static bool _kc85_exp_write_ctrl(kc85_t* sys, uint8_t slot_addr, uint8_t ctrl_byte);
static uint8_t _kc85_exp_module_id(kc85_t* sys, uint8_t slot_addr);
static void _kc85_exp_update_memory_mapping(kc85_t* sys);

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

    /* fill RAM with noise (only KC85/2 and /3) */
    if (KC85_TYPE_4 != sys->type) {
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
    sys->patch_cb = desc->patch_cb;
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

    /* expansion module system */
    _kc85_exp_init(sys);

    /* initial memory map (must happen after expansion system) */
    _kc85_init_memory_map(sys);

    /* the kbd_t helper functions are only used as a simple keystroke buffer,
       the KC85 doesn't have a typical keyboard matrix in the computer
       base unit, since the keyboard unit sent keystroke information
       in a serial-encoded signal to the base unit. Currently this
       serial encoding/decoding isn't emulated (it wasn't very reliable
       anyway and required slot typing).
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
    _kc85_exp_reset(sys);
    _kc85_update_memory_map(sys);

    /* execution after reset starts at 0xE000 */
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

    /* tick the CTC and beepers */
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
                        {
                            const uint8_t slot_addr = Z80_GET_ADDR(pins)>>8;
                            if (pins & Z80_WR) {
                                /* write new control byte and update the memory mapping */
                                if (_kc85_exp_write_ctrl(sys, slot_addr, data)) {
                                    _kc85_update_memory_map(sys);
                                }
                            }
                            else {
                                /* read module id in slot */
                                Z80_SET_DATA(pins, _kc85_exp_module_id(sys, slot_addr));
                            }
                        }
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

/* background colors */
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
    sys->pio_a = _KC85_PIO_A_RAM | _KC85_PIO_A_IRM | _KC85_PIO_A_CAOS_ROM;
    _kc85_update_memory_map(sys);
}

static void _kc85_update_memory_map(kc85_t* sys) {
    mem_unmap_layer(&sys->mem, 0);

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
    
    /* let the module system update it's memory mapping */
    _kc85_exp_update_memory_mapping(sys);
}

/*
    KEYBOARD INPUT

    this is a simplified version of the PIO-B interrupt service routine
    which is normally triggered when the serial keyboard hardware
    sends a new pulse (for details, see
    https://github.com/floooh/yakc/blob/master/misc/kc85_3_kbdint.md )

    we ignore the whole tricky serial decoding and patch the
    keycode directly into the right memory locations.
*/

#define _KC85_KBD_TIMEOUT (1<<3)
#define _KC85_KBD_KEYREADY (1<<0)
#define _KC85_KBD_REPEAT (1<<4)
#define _KC85_KBD_SHORT_REPEAT_COUNT (8)
#define _KC85_KBD_LONG_REPEAT_COUNT (60)

static void _kc85_handle_keyboard(kc85_t* sys) {
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
                    /* wait some more... */
                    return;
                }
                else {
                    /* first key-repeat pause over, set first-key-repeat flag */
                    mem_wr(&sys->mem, ix+0x8, mem_rd(&sys->mem, ix+0x8)|_KC85_KBD_REPEAT);
                }
            }
            /* key-repeat triggered, just set the key-ready flag and reset repeat-count */
            mem_wr(&sys->mem, ix+0x8, mem_rd(&sys->mem, ix+0x8)|_KC85_KBD_KEYREADY);
            mem_wr(&sys->mem, ix+0xA, 0);
        }
    }
}

/*=== EXPANSION MODULE SUBSYSTEM =============================================*/
static void _kc85_exp_init(kc85_t* sys) {
    /* assumes that the entire kc85_t struct was memory cleared already! */
    CHIPS_ASSERT(2 == KC85_NUM_SLOTS);
    sys->exp.slot[0].addr = 0x08;
    sys->exp.slot[1].addr = 0x0C;
    for (int i = 0; i < KC85_NUM_SLOTS; i++) {
        sys->exp.slot[i].mod.id = 0xFF;
    }
}

static void _kc85_exp_reset(kc85_t* sys) {
    /* FIXME? */
}

static kc85_slot_t* _kc85_exp_slot_by_addr(kc85_t* sys, uint8_t slot_addr) {
    for (int i = 0; i < KC85_NUM_SLOTS; i++) {
        kc85_slot_t* slot = &sys->exp.slot[i];
        if (slot_addr == slot->addr) {
            return slot;
        }
    }
    return 0;
}

/* allocate expansion buffer space for a module to be inserted into a slot
    updates:
        slot->buf_offset
        sys->exp.buf_top
*/
static bool _kc85_exp_alloc(kc85_t* sys, kc85_slot_t* slot) {
    /* check if there's enough room */
    if ((slot->mod.size + sys->exp.buf_top) > KC85_EXP_BUFSIZE) {
        return false;
    }
    /* update offsets and sizes */
    slot->buf_offset = sys->exp.buf_top;
    sys->exp.buf_top += slot->mod.size;
    return true;
}

/* free room in the expansion buffer, and close the hole
    updates:
        sys->exp.buf_top
        sys->exp_buf (any holes are merged)
        for each slot "behind" the to-be-freed slot:
            slot->buf_offset
*/
static void _kc85_exp_free(kc85_t* sys, kc85_slot_t* free_slot) {
    CHIPS_ASSERT(free_slot->mod.size > 0);
    const uint32_t bytes_to_free = free_slot->mod.size;
    CHIPS_ASSERT(sys->exp.buf_top >= bytes_to_free);
    sys->exp.buf_top -= bytes_to_free;
    for (int i = 0; i < KC85_NUM_SLOTS; i++) {
        kc85_slot_t* slot = &sys->exp.slot[i];
        /* no module in slot: nothing to do */
        if (slot->mod.type == KC85_MODULE_NONE) {
            continue;
        }
        /* if slot is 'behind' the to-be-freed slot... */
        if (slot->buf_offset > free_slot->buf_offset) {
            CHIPS_ASSERT(slot->buf_offset >= bytes_to_free);
            /* move data backward to close the hole */
            const uint8_t* from = &sys->exp_buf[slot->buf_offset];
            uint8_t* to = &sys->exp_buf[slot->buf_offset - bytes_to_free];
            memmove(to, from, bytes_to_free);
            slot->buf_offset -= bytes_to_free;
        }
    }
}

static bool _kc85_insert_module(kc85_t* sys, uint8_t slot_addr, kc85_module_type_t type, const void* rom_ptr, int rom_size) {
    kc85_slot_t* slot = _kc85_exp_slot_by_addr(sys, slot_addr);
    if (!slot) {
        return false;
    }
    slot->mod.type = type;
    switch (type) {
        case KC85_MODULE_M006_BASIC:
            slot->mod.id = 0xFC;
            slot->mod.writable = false;
            slot->mod.addr_mask = 0xC0;
            slot->mod.size = 16*1024;
            break;
        case KC85_MODULE_M011_64KBYE:
            slot->mod.id = 0xF6;
            slot->mod.writable = true;
            slot->mod.addr_mask = 0xC0;
            slot->mod.size = 64*1024;
            break;
        case KC85_MODULE_M022_16KBYTE:
            slot->mod.id = 0xF4;
            slot->mod.writable = true;
            slot->mod.addr_mask = 0xC0;
            slot->mod.size = 16*1024;
            break;
        case KC85_MODULE_M012_TEXOR:
        case KC85_MODULE_M026_FORTH:
        case KC85_MODULE_M027_DEVELOPMENT:
            slot->mod.id = 0xFB;
            slot->mod.writable = false;
            slot->mod.addr_mask = 0xE0;
            slot->mod.size = 8*1024;
            break;
        default:
            CHIPS_ASSERT(false);
            break;
    }

    /* allocate space in expansion buffer */
    if (!_kc85_exp_alloc(sys, slot)) {
        /* not enough space left in buffer */
        slot->mod.type = KC85_MODULE_NONE;
        slot->mod.id = 0xFF;
        slot->mod.size = 0;
        return false;
    }

    /* copy optional ROM image, or clear RAM */
    if (rom_ptr) {
        if (rom_size != slot->mod.size) {
            return false;
        }
        memcpy(&sys->exp_buf[slot->buf_offset], rom_ptr, slot->mod.size);
    }
    else {
        memset(&sys->exp_buf[slot->buf_offset], 0, slot->mod.size);
    }

    /* update the memory mapping */
    _kc85_update_memory_map(sys);

    return true;
}

bool kc85_insert_ram_module(kc85_t* sys, uint8_t slot_addr, kc85_module_type_t type) {
    CHIPS_ASSERT(sys && sys->valid && (type != KC85_MODULE_NONE));
    kc85_remove_module(sys, slot_addr);
    return _kc85_insert_module(sys, slot_addr, type, 0, 0);
}

bool kc85_insert_rom_module(kc85_t* sys, uint8_t slot_addr, kc85_module_type_t type, const void* rom_ptr, int rom_size) {
    CHIPS_ASSERT(sys && sys->valid && (type != KC85_MODULE_NONE));
    kc85_remove_module(sys, slot_addr);
    return _kc85_insert_module(sys, slot_addr, type, rom_ptr, rom_size);
}

bool kc85_remove_module(kc85_t* sys, uint8_t slot_addr) {
    CHIPS_ASSERT(sys && sys->valid);
    kc85_slot_t* slot = _kc85_exp_slot_by_addr(sys, slot_addr);
    if (!slot) {
        return false;
    }
    /* slot not occupied? */
    if (slot->mod.type == KC85_MODULE_NONE) {
        CHIPS_ASSERT(slot->mod.id == 0xFF);
        CHIPS_ASSERT(slot->mod.size == 0);
        return false;
    }

    /* first free the expansion buffer space */
    _kc85_exp_free(sys, slot);

    /* de-init the module attributes */
    slot->mod.type = KC85_MODULE_NONE;
    slot->mod.id = 0xFF;
    slot->mod.writable = false;
    slot->mod.addr_mask = 0;
    slot->mod.size = 0;

    /* update the memory mapping */
    _kc85_update_memory_map(sys);

    return true;
}

static bool _kc85_exp_write_ctrl(kc85_t* sys, uint8_t slot_addr, uint8_t ctrl_byte) {
    kc85_slot_t* slot = _kc85_exp_slot_by_addr(sys, slot_addr);
    if (slot) {
        slot->ctrl = ctrl_byte;
        return true;
    }
    else {
        return false;
    }
}

static uint8_t _kc85_exp_module_id(kc85_t* sys, uint8_t slot_addr) {
    const kc85_slot_t* slot = _kc85_exp_slot_by_addr(sys, slot_addr);
    return slot ? slot->mod.id : 0xFF;
}

static void _kc85_exp_update_memory_mapping(kc85_t* sys) {
    for (int i = 0; i < KC85_NUM_SLOTS; i++) {
        const kc85_slot_t* slot = &sys->exp.slot[i];

        /* nothing to do if no module in slot */
        if (KC85_MODULE_NONE == slot->mod.type) {
            continue;
        }

        /* each slot gets its own memory system mapping layer, 
           layer 0 is used by computer base unit 
        */
        const int layer = i + 1;
        mem_unmap_layer(&sys->mem, layer);

        /* module is only active if bit 0 in control byte is set */
        if (slot->ctrl & 0x01) {
            /* compute CPU- and host-address */
            const uint16_t addr = (slot->ctrl & slot->mod.addr_mask) << 8;
            uint8_t* host_addr = &sys->exp_buf[slot->buf_offset];

            /* RAM modules are only writable if bit 1 in control-byte is set */
            const bool writable = (slot->ctrl & 0x02) && slot->mod.writable;
            if (writable) {
                mem_map_ram(&sys->mem, layer, addr, slot->mod.size, host_addr);
            }
            else {
                mem_map_rom(&sys->mem, layer, addr, slot->mod.size, host_addr);
            }
        }
    }
}

/*=== FILE LOADING ===========================================================*/

/* common start function for all snapshot file formats */
static void _kc85_load_start(kc85_t* sys, uint16_t exec_addr) {
    z80_set_a(&sys->cpu, 0x00);
    z80_set_f(&sys->cpu, 0x10);
    z80_set_bc(&sys->cpu, 0x0000); z80_set_bc_(&sys->cpu, 0x0000);
    z80_set_de(&sys->cpu, 0x0000); z80_set_de_(&sys->cpu, 0x0000);
    z80_set_hl(&sys->cpu, 0x0000); z80_set_hl_(&sys->cpu, 0x0000);
    z80_set_af_(&sys->cpu, 0x0000);
    z80_set_sp(&sys->cpu, 0x01C2);
    /* delete ASCII buffer */
    for (uint16_t addr = 0xb200; addr < 0xb700; addr++) {
        mem_wr(&sys->mem, addr, 0);
    }
    mem_wr(&sys->mem, 0xb7a0, 0);
    if (KC85_TYPE_3 == sys->type) {
        _kc85_tick(1, Z80_MAKE_PINS(Z80_IORQ|Z80_WR, 0x89, 0x9f), sys);
        mem_wr16(&sys->mem, z80_sp(&sys->cpu), 0xf15c);
    }
    else if (KC85_TYPE_4 == sys->type) {
        _kc85_tick(1, Z80_MAKE_PINS(Z80_IORQ|Z80_WR, 0x89, 0xff), sys);
        mem_wr16(&sys->mem, z80_sp(&sys->cpu), 0xf17e);
    }
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
} _kc85_kcc_header;

/* invoke the post-snapshot-loading patch callback */
static void _kc85_invoke_patch_callback(kc85_t* sys, const _kc85_kcc_header* hdr) {
    if (sys->patch_cb) {
        char img_name[17];
        memcpy(img_name, hdr->name, 16);
        img_name[16] = 0;
        sys->patch_cb(img_name, sys->user_data);
    }
}

/* KCC files cannot really be identified since they have no magic number */
static bool _kc85_is_valid_kcc(const uint8_t* ptr, int num_bytes) {
    if (num_bytes <= (int) sizeof (_kc85_kcc_header)) {
        return false;
    }
    const _kc85_kcc_header* hdr = (const _kc85_kcc_header*) ptr;
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
    int required_len = (end_addr - load_addr) + sizeof(_kc85_kcc_header);
    if (required_len > num_bytes) {
        return false;
    }
    /* could be a KCC file */
    return true;
}

static bool _kc85_load_kcc(kc85_t* sys, const uint8_t* ptr, int num_bytes) {
    const _kc85_kcc_header* hdr = (_kc85_kcc_header*) ptr;
    uint16_t addr = hdr->load_addr_h<<8 | hdr->load_addr_l;
    uint16_t end_addr  = hdr->end_addr_h<<8 | hdr->end_addr_l;
    ptr += sizeof(_kc85_kcc_header);
    while (addr < end_addr) {
        /* data is continuous */
        mem_wr(&sys->mem, addr++, *ptr++);
    }
    _kc85_invoke_patch_callback(sys, hdr);
    /* if file has an exec-address, start the program */
    if (hdr->num_addr > 2) {
        _kc85_load_start(sys, hdr->exec_addr_h<<8 | hdr->exec_addr_l);
    }
    return false;
}

/* KC TAP file format support */
typedef struct {
    uint8_t sig[16];        /* "\xC3KC-TAPE by AF. " */
    uint8_t type;           /* 00: KCTAP_Z9001, 01: KCTAP_KC85, else: KCTAP_SYS */
    _kc85_kcc_header kcc;   /* from here on identical with KCC */
} _kc85_kctap_header;

static bool _kc85_is_valid_kctap(const uint8_t* ptr, int num_bytes) {
    if (num_bytes <= (int) sizeof (_kc85_kctap_header)) {
        return false;
    }
    const _kc85_kctap_header* hdr = (const _kc85_kctap_header*) ptr;
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
    int required_len = (end_addr - load_addr) + sizeof(_kc85_kctap_header);
    if (required_len > num_bytes) {
        return false;
    }
    /* this appears to be a valid KC TAP file */
    return true;
}

static bool _kc85_load_kctap(kc85_t* sys, const uint8_t* ptr, int num_bytes) {
    const _kc85_kctap_header* hdr = (const _kc85_kctap_header*) ptr;
    uint16_t addr = hdr->kcc.load_addr_h<<8 | hdr->kcc.load_addr_l;
    uint16_t end_addr  = hdr->kcc.end_addr_h<<8 | hdr->kcc.end_addr_l;
    ptr += sizeof(_kc85_kctap_header);
    while (addr < end_addr) {
        /* each block is 1 lead-byte + 128 bytes data */
        ptr++;
        for (int i = 0; i < 128; i++) {
            mem_wr(&sys->mem, addr++, *ptr++);
        }
    }
    _kc85_invoke_patch_callback(sys, &hdr->kcc);
    /* if file has an exec-address, start the program */
    if (hdr->kcc.num_addr > 2) {
        _kc85_load_start(sys, hdr->kcc.exec_addr_h<<8 | hdr->kcc.exec_addr_l);
    }
    return true;
}

bool kc85_quickload(kc85_t* sys, const uint8_t* ptr, int num_bytes) {
    CHIPS_ASSERT(sys && sys->valid && ptr);
    /* first check for KC-TAP format, since this can be properly identified */
    if (_kc85_is_valid_kctap(ptr, num_bytes)) {
        return _kc85_load_kctap(sys, ptr, num_bytes);
    }
    else if (_kc85_is_valid_kcc(ptr, num_bytes)) {
        return _kc85_load_kcc(sys, ptr, num_bytes);
    }
    else {
        /* not a known file type, or not enough data */
        return false;
    }
}

#endif /* CHIPS_IMPL */
