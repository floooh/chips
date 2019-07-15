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

    You need to include the following headers before including c64.h:

    - chips/m6502.h
    - chips/m6526.h
    - chips/m6569.h
    - chips/m6581.h
    - chips/beeper.h
    - chips/kbd.h
    - chips/mem.h
    - chips/clk.h

    ## The Commodore C64

    TODO!

    ## TODO:

    - emulate separate joystick 1 and 2
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

#define C64_MAX_AUDIO_SAMPLES (1024)        /* max number of audio samples in internal sample buffer */
#define C64_DEFAULT_AUDIO_SAMPLES (128)     /* default number of samples in internal sample buffer */ 
#define C64_MAX_TAPE_SIZE (512*1024)        /* max size of cassette tape image */

/* C64 joystick types */
typedef enum {
    C64_JOYSTICKTYPE_NONE,
    C64_JOYSTICKTYPE_DIGITAL_1,
    C64_JOYSTICKTYPE_DIGITAL_2,
    C64_JOYSTICKTYPE_PADDLE_1,  /* FIXME: not emulated */
    C64_JOYSTICKTYPE_PADDLE_2,  /* FIXME: not emulated */
} c64_joystick_type_t;

/* joystick mask bits */
#define C64_JOYSTICK_UP    (1<<0)
#define C64_JOYSTICK_DOWN  (1<<1)
#define C64_JOYSTICK_LEFT  (1<<2)
#define C64_JOYSTICK_RIGHT (1<<3)
#define C64_JOYSTICK_BTN   (1<<4)

/* CPU port memory mapping bits */
#define C64_CPUPORT_LORAM (1<<0)
#define C64_CPUPORT_HIRAM (1<<1)
#define C64_CPUPORT_CHAREN (1<<2)

/* audio sample data callback */
typedef void (*c64_audio_callback_t)(const float* samples, int num_samples, void* user_data);

/* config parameters for c64_init() */
typedef struct {
    c64_joystick_type_t joystick_type;  /* default is C64_JOYSTICK_NONE */

    /* video output config (if you don't want video decoding, set these to 0) */
    void* pixel_buffer;         /* pointer to a linear RGBA8 pixel buffer, at least 392*272*4 bytes */
    int pixel_buffer_size;      /* size of the pixel buffer in bytes */

    /* optional user-data for callback functions */
    void* user_data;

    /* audio output config (if you don't want audio, set audio_cb to zero) */
    c64_audio_callback_t audio_cb;  /* called when audio_num_samples are ready */
    int audio_num_samples;          /* default is C64_AUDIO_NUM_SAMPLES */
    int audio_sample_rate;          /* playback sample rate, default is 44100 */
    float audio_sid_volume;         /* audio volume of the SID chip (0.0 .. 1.0), default is 1.0 */
    float audio_beeper_volume;      /* audio volume of the tape-beeper (0.0 .. 1.0), default is 0.1 */
    bool audio_tape_sound;          /* if true, tape loading is audible */

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
    beeper_t beeper;            /* used for the tape-sound */
    
    bool valid;
    c64_joystick_type_t joystick_type;
    uint8_t joystick_active;
    bool io_mapped;             /* true when D000..DFFF is has IO area mapped in */
    uint8_t cpu_port;           /* last state of CPU port (for memory mapping) */
    uint8_t kbd_joy1_mask;      /* current joystick-1 state from keyboard-joystick emulation */
    uint8_t kbd_joy2_mask;      /* current joystick-2 state from keyboard-joystick emulation */
    uint8_t joy_joy1_mask;      /* current joystick-1 state from c64_joystick() */
    uint8_t joy_joy2_mask;      /* current joystick-2 state from c64_joystick() */
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

    bool tape_motor;    /* tape motor on/off */
    bool tape_button;   /* play button on tape pressed/unpressed */
    bool tape_sound;    /* true when tape is audible */
    int tape_size;      /* tape_size > 0: a tape is inserted */
    int tape_pos;        
    int tape_tick_count;
    uint8_t tape_buf[C64_MAX_TAPE_SIZE];
} c64_t;

/* initialize a new C64 instance */
void c64_init(c64_t* sys, const c64_desc_t* desc);
/* discard C64 instance */
void c64_discard(c64_t* sys);
/* get the standard framebuffer width and height in pixels */
int c64_std_display_width(void);
int c64_std_display_height(void);
/* get the maximum framebuffer size in number of bytes */
int c64_max_display_size(void);
/* get the current framebuffer width and height in pixels */
int c64_display_width(c64_t* sys);
int c64_display_height(c64_t* sys);
/* reset a C64 instance */
void c64_reset(c64_t* sys);
/* tick C64 instance for a given number of microseconds */
void c64_exec(c64_t* sys, uint32_t micro_seconds);
/* send a key-down event to the C64 */
void c64_key_down(c64_t* sys, int key_code);
/* send a key-up event to the C64 */
void c64_key_up(c64_t* sys, int key_code);
/* enable/disable joystick emulation */
void c64_set_joystick_type(c64_t* sys, c64_joystick_type_t type);
/* get current joystick emulation type */
c64_joystick_type_t c64_joystick_type(c64_t* sys);
/* set joystick mask (combination of C64_JOYSTICK_*) */
void c64_joystick(c64_t* sys, uint8_t joy1_mask, uint8_t joy2_mask);
/* insert a tape file */
bool c64_insert_tape(c64_t* sys, const uint8_t* ptr, int num_bytes);
/* remove tape file */
void c64_remove_tape(c64_t* sys);
/* start the tape (press the Play button) */
void c64_start_tape(c64_t* sys);
/* stop the tape (unpress the Play button */
void c64_stop_tape(c64_t* sys);
/* quickload a .bin file (only tested with wlorenz tests) */
bool c64_quickload(c64_t* sys, const uint8_t* ptr, int num_bytes);

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

#define _C64_STD_DISPLAY_WIDTH (392)
#define _C64_STD_DISPLAY_HEIGHT (272)
#define _C64_DBG_DISPLAY_WIDTH ((_M6569_HTOTAL+1)*8)
#define _C64_DBG_DISPLAY_HEIGHT (_M6569_VTOTAL+1)
#define _C64_DISPLAY_SIZE (_C64_DBG_DISPLAY_WIDTH*_C64_DBG_DISPLAY_HEIGHT*4)
#define _C64_FREQUENCY (985248)
#define _C64_DISPLAY_X (64)
#define _C64_DISPLAY_Y (24)

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
static bool _c64_tape_tick(c64_t* sys);

#define _C64_DEFAULT(val,def) (((val) != 0) ? (val) : (def));
#define _C64_CLEAR(val) memset(&val, 0, sizeof(val))

void c64_init(c64_t* sys, const c64_desc_t* desc) {
    CHIPS_ASSERT(sys && desc);
    CHIPS_ASSERT(!desc->pixel_buffer || (desc->pixel_buffer_size >= _C64_DISPLAY_SIZE));

    memset(sys, 0, sizeof(c64_t));
    sys->valid = true;
    sys->joystick_type = desc->joystick_type;
    sys->tape_sound = desc->audio_tape_sound;
    CHIPS_ASSERT(desc->rom_char && (desc->rom_char_size == sizeof(sys->rom_char)));
    CHIPS_ASSERT(desc->rom_basic && (desc->rom_basic_size == sizeof(sys->rom_basic)));
    CHIPS_ASSERT(desc->rom_kernal && (desc->rom_kernal_size == sizeof(sys->rom_kernal)));
    memcpy(sys->rom_char, desc->rom_char, sizeof(sys->rom_char));
    memcpy(sys->rom_basic, desc->rom_basic, sizeof(sys->rom_basic));
    memcpy(sys->rom_kernal, desc->rom_kernal, sizeof(sys->rom_kernal));
    sys->user_data = desc->user_data;
    sys->audio_cb = desc->audio_cb;
    sys->num_samples = _C64_DEFAULT(desc->audio_num_samples, C64_DEFAULT_AUDIO_SAMPLES);
    CHIPS_ASSERT(sys->num_samples <= C64_MAX_AUDIO_SAMPLES);

    /* initialize the hardware */
    clk_init(&sys->clk, _C64_FREQUENCY);
    sys->cpu_port = 0xF7;       /* for initial memory mapping */
    sys->io_mapped = true;
    
    m6502_desc_t cpu_desc;
    _C64_CLEAR(cpu_desc);
    cpu_desc.tick_cb = _c64_tick;
    cpu_desc.in_cb = _c64_cpu_port_in;
    cpu_desc.out_cb = _c64_cpu_port_out;
    cpu_desc.m6510_io_pullup = 0x17;
    cpu_desc.m6510_io_floating = 0xC8;
    cpu_desc.user_data = sys;
    m6502_init(&sys->cpu, &cpu_desc);

    m6526_desc_t cia_desc;
    _C64_CLEAR(cia_desc);
    cia_desc.user_data = sys;
    cia_desc.in_cb = _c64_cia1_in;
    cia_desc.out_cb = _c64_cia1_out;
    m6526_init(&sys->cia_1, &cia_desc);
    cia_desc.in_cb = _c64_cia2_in;
    cia_desc.out_cb = _c64_cia2_out;
    m6526_init(&sys->cia_2, &cia_desc);

    m6569_desc_t vic_desc;
    _C64_CLEAR(vic_desc);
    vic_desc.fetch_cb = _c64_vic_fetch;
    vic_desc.rgba8_buffer = (uint32_t*) desc->pixel_buffer;
    vic_desc.rgba8_buffer_size = desc->pixel_buffer_size;
    vic_desc.vis_x = _C64_DISPLAY_X;
    vic_desc.vis_y = _C64_DISPLAY_Y;
    vic_desc.vis_w = _C64_STD_DISPLAY_WIDTH;
    vic_desc.vis_h = _C64_STD_DISPLAY_HEIGHT;
    vic_desc.user_data = sys;
    m6569_init(&sys->vic, &vic_desc);

    const int sound_hz = _C64_DEFAULT(desc->audio_sample_rate, 44100);
    const float sid_volume = _C64_DEFAULT(desc->audio_sid_volume, 1.0f);
    const float beeper_volume = _C64_DEFAULT(desc->audio_beeper_volume, 0.1f);
    m6581_desc_t sid_desc;
    _C64_CLEAR(sid_desc);
    sid_desc.tick_hz = _C64_FREQUENCY;
    sid_desc.sound_hz = sound_hz;
    sid_desc.magnitude = sid_volume;
    m6581_init(&sys->sid, &sid_desc);

    beeper_init(&sys->beeper, _C64_FREQUENCY, sound_hz, beeper_volume);

    _c64_init_key_map(sys);
    _c64_init_memory_map(sys);

    m6502_reset(&sys->cpu);
}

void c64_discard(c64_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    sys->valid = false;
}

int c64_std_display_width(void) {
    return _C64_STD_DISPLAY_WIDTH;
}

int c64_std_display_height(void) {
    return _C64_STD_DISPLAY_HEIGHT;
}

int c64_max_display_size(void) {
    return _C64_DISPLAY_SIZE;
}

int c64_display_width(c64_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    return m6569_display_width(&sys->vic);
}

int c64_display_height(c64_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    return m6569_display_height(&sys->vic);
}

void c64_reset(c64_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    sys->cpu_port = 0xF7;
    sys->kbd_joy1_mask = sys->kbd_joy2_mask = 0;
    sys->joy_joy1_mask = sys->joy_joy2_mask = 0;
    sys->io_mapped = true;
    _c64_update_memory_map(sys);
    m6502_reset(&sys->cpu);
    m6526_reset(&sys->cia_1);
    m6526_reset(&sys->cia_2);
    m6569_reset(&sys->vic);
    m6581_reset(&sys->sid);
    beeper_reset(&sys->beeper);
    sys->tape_motor = false;
    sys->tape_button = false;
    sys->tape_size = 0;
    sys->tape_pos = 0;
    sys->tape_tick_count = 0;
}

void c64_exec(c64_t* sys, uint32_t micro_seconds) {
    CHIPS_ASSERT(sys && sys->valid);
    uint32_t ticks_to_run = clk_ticks_to_run(&sys->clk, micro_seconds);
    uint32_t ticks_executed = m6502_exec(&sys->cpu, ticks_to_run);
    clk_ticks_executed(&sys->clk, ticks_executed);
    kbd_update(&sys->kbd);
}

void c64_key_down(c64_t* sys, int key_code) {
    CHIPS_ASSERT(sys && sys->valid);
    if (sys->joystick_type == C64_JOYSTICKTYPE_NONE) {
        kbd_key_down(&sys->kbd, key_code);
    }
    else {
        uint8_t m = 0;
        switch (key_code) {
            case 0x20: m = C64_JOYSTICK_BTN; break;
            case 0x08: m = C64_JOYSTICK_LEFT; break;
            case 0x09: m = C64_JOYSTICK_RIGHT; break;
            case 0x0A: m = C64_JOYSTICK_DOWN; break;
            case 0x0B: m = C64_JOYSTICK_UP; break;
            default: kbd_key_down(&sys->kbd, key_code); break;
        }
        if (m != 0) {
            if (sys->joystick_type == C64_JOYSTICKTYPE_DIGITAL_1) {
                sys->kbd_joy1_mask |= m;
            }
            else {
                sys->kbd_joy2_mask |= m;
            }
        }
    }
}

void c64_key_up(c64_t* sys, int key_code) {
    CHIPS_ASSERT(sys && sys->valid);
    if (sys->joystick_type == C64_JOYSTICKTYPE_NONE) {
        kbd_key_up(&sys->kbd, key_code);
    }
    else {
        uint8_t m = 0;
        switch (key_code) {
            case 0x20: m = C64_JOYSTICK_BTN; break;
            case 0x08: m = C64_JOYSTICK_LEFT; break;
            case 0x09: m = C64_JOYSTICK_RIGHT; break;
            case 0x0A: m = C64_JOYSTICK_DOWN; break;
            case 0x0B: m = C64_JOYSTICK_UP; break;
            default: kbd_key_up(&sys->kbd, key_code); break;
        }
        if (m != 0) {
            if (sys->joystick_type == C64_JOYSTICKTYPE_DIGITAL_1) {
                sys->kbd_joy1_mask &= ~m;
            }
            else {
                sys->kbd_joy2_mask &= ~m;
            }
        }
    }
}

void c64_set_joystick_type(c64_t* sys, c64_joystick_type_t type) {
    CHIPS_ASSERT(sys && sys->valid);
    sys->joystick_type = type;
}

c64_joystick_type_t c64_joystick_type(c64_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    return sys->joystick_type;
}

void c64_joystick(c64_t* sys, uint8_t joy1_mask, uint8_t joy2_mask) {
    CHIPS_ASSERT(sys && sys->valid);
    sys->joy_joy1_mask = joy1_mask;
    sys->joy_joy2_mask = joy2_mask;
}

static uint64_t _c64_tick(uint64_t pins, void* user_data) {
    c64_t* sys = (c64_t*) user_data;
    const uint16_t addr = M6502_GET_ADDR(pins);

    /* tick the datasette, when the datasette output pulse
       toggles, the FLAG input pin on CIA-1 will go active for 1 tick
    */
    uint64_t cia1_pins = pins;
    if (_c64_tape_tick(sys)) {
        cia1_pins |= M6526_FLAG;
    }

    /* if the tape should be audible, toggle the beeper */
    if (sys->tape_sound) {
        beeper_tick(&sys->beeper);
    }

    /* tick the SID */
    if (m6581_tick(&sys->sid)) {
        /* new audio sample ready */
        float sample = sys->sid.sample;
        if (sys->tape_motor) {
            sample += sys->beeper.sample;
        }
        sys->sample_buffer[sys->sample_pos++] = sample;
        if (sys->sample_pos == sys->num_samples) {
            if (sys->audio_cb) {
                sys->audio_cb(sys->sample_buffer, sys->num_samples, sys->user_data);
            }
            sys->sample_pos = 0;
        }
    }

    /* tick the CIAs:
        - CIA-1 gets the FLAG pin from the datasette
        - the CIA-1 IRQ pin is connected to the CPU IRQ pin
        - the CIA-2 IRQ pin is connected to the CPU NMI pin
    */
    if (m6526_tick(&sys->cia_1, cia1_pins & ~M6502_IRQ) & M6502_IRQ) {
        pins |= M6502_IRQ;
    }
    if (m6526_tick(&sys->cia_2, pins & ~M6502_IRQ) & M6502_IRQ) {
        pins |= M6502_NMI;
    }

    /* tick the VIC-II display chip:
        - the VIC-II IRQ pin is connected to the CPU IRQ pin and goes
        active when the VIC-II requests a rasterline interrupt
        - the VIC-II BA pin is connected to the CPU RDY pin, and stops
        the CPU on the first CPU read access after BA goes active
        - the VIC-II AEC pin is connected to the CPU AEC pin, currently
        this goes active during a badline, but is not checked
    */
    pins = m6569_tick(&sys->vic, pins);

    /* Special handling when the VIC-II asks the CPU to stop during a
        'badline' via the BA=>RDY pin. If the RDY pin is active, the
        CPU will 'loop' on the tick callback during the next READ access
        until the RDY pin goes inactive. The tick callback must make sure
        that only a single READ access happens during the entire RDY period.
        Currently this happens on the very last tick when RDY goes from
        active to inactive (I haven't found definitive documentation if
        this is the right behaviour, but it made the Boulderdash fast loader work).
    */
    if ((pins & (M6502_RDY|M6502_RW)) == (M6502_RDY|M6502_RW)) {
        return pins;
    }

    /* handle IO requests */
    if (M6510_CHECK_IO(pins)) {
        /* ...the integrated IO port in the M6510 CPU at addresses 0 and 1 */
        pins = m6510_iorq(&sys->cpu, pins);
    }
    else {
        /* ...the memory-mapped IO area from 0xD000 to 0xDFFF */
        if (sys->io_mapped && ((addr & 0xF000) == 0xD000)) {
            if (addr < 0xD400) {
                /* VIC-II (D000..D3FF) */
                uint64_t vic_pins = (pins & M6502_PIN_MASK)|M6569_CS;
                pins = m6569_iorq(&sys->vic, vic_pins) & M6502_PIN_MASK;
            }
            else if (addr < 0xD800) {
                /* SID (D400..D7FF) */
                uint64_t sid_pins = (pins & M6502_PIN_MASK)|M6581_CS;
                pins = m6581_iorq(&sys->sid, sid_pins) & M6502_PIN_MASK;
            }
            else if (addr < 0xDC00) {
                /* read or write the special color Static-RAM bank (D800..DBFF) */
                if (pins & M6502_RW) {
                    M6502_SET_DATA(pins, sys->color_ram[addr & 0x03FF]);
                }
                else {
                    sys->color_ram[addr & 0x03FF] = M6502_GET_DATA(pins);
                }
            }
            else if (addr < 0xDD00) {
                /* CIA-1 (DC00..DCFF) */
                uint64_t cia_pins = (pins & M6502_PIN_MASK)|M6526_CS;
                pins = m6526_iorq(&sys->cia_1, cia_pins) & M6502_PIN_MASK;
            }
            else if (addr < 0xDE00) {
                /* CIA-2 (DD00..DDFF) */
                uint64_t cia_pins = (pins & M6502_PIN_MASK)|M6526_CS;
                pins = m6526_iorq(&sys->cia_2, cia_pins) & M6502_PIN_MASK;
            }
            else {
                /* FIXME: expansion system (not implemented) */
            }
        }
        else {
            /* a regular memory access */
            if (pins & M6502_RW) {
                /* memory read */
                M6502_SET_DATA(pins, mem_rd(&sys->mem_cpu, addr));
            }
            else {
                /* memory write */
                mem_wr(&sys->mem_cpu, addr, M6502_GET_DATA(pins));
            }
        }
    }
    return pins;
}

static uint8_t _c64_cpu_port_in(void* user_data) {
    c64_t* sys = (c64_t*) user_data;
    /*
        Input from the integrated M6510 CPU IO port

        bit 4: [in] datasette button status (1: no button pressed)
    */
    uint8_t val = 7;
    if (!sys->tape_button) {
        val |= (1<<4);
    }
    return val;
}

static void _c64_cpu_port_out(uint8_t data, void* user_data) {
    c64_t* sys = (c64_t*) user_data;
    /*
        Output to the integrated M6510 CPU IO port

        bits 0..2:  [out] memory configuration

        bit 3: [out] datasette output signal level
        bit 5: [out] datasette motor control (1: motor off)
    */
    if (data & (1<<5)) {
        sys->tape_motor = false;
    }
    else {
        sys->tape_motor = true;
    }
    /* only update memory configuration if the relevant bits have changed */
    bool need_mem_update = 0 != ((sys->cpu_port ^ data) & 7);
    sys->cpu_port = data;
    if (need_mem_update) {
        _c64_update_memory_map(sys);
    }
}

static void _c64_cia1_out(int port_id, uint8_t data, void* user_data) {
    c64_t* sys = (c64_t*) user_data;
    /*
        Write CIA-1 ports:

        port A:
            write keyboard matrix lines
        port B:
            ---
    */
    if (port_id == M6526_PORT_A) {
        kbd_set_active_lines(&sys->kbd, ~data);
    }
}

static uint8_t _c64_cia1_in(int port_id, void* user_data) {
    c64_t* sys = (c64_t*) user_data;
    /*
        Read CIA-1 ports:

        Port A:
            joystick 2 input
        Port B:
            combined keyboard matrix columns and joystick 1
    */
    if (port_id == M6526_PORT_A) {
        /* FIXME: currently use the same input for joystick 2 and joystick 1 */
        return ~(sys->kbd_joy2_mask | sys->joy_joy2_mask);
    }
    else {
        /* read keyboard matrix columns */
        return ~(kbd_scan_columns(&sys->kbd) | sys->kbd_joy1_mask | sys->joy_joy1_mask);
    }
}

static void _c64_cia2_out(int port_id, uint8_t data, void* user_data) {
    c64_t* sys = (c64_t*) user_data;
    /*
        Write CIA-2 ports:

        Port A:
            bits 0..1: VIC-II bank select:
                00: bank 3 C000..FFFF
                01: bank 2 8000..BFFF
                10: bank 1 4000..7FFF
                11: bank 0 0000..3FFF
            bit 2: RS-232 TXD Outout (not implemented)
            bit 3..5: serial bus output (not implemented)
            bit 6..7: input (see cia2_in)
        Port B:
            RS232 / user functionality (not implemented)
    */
    if (port_id == M6526_PORT_A) {
        sys->vic_bank_select = ((~data)&3)<<14;
    }
}

static uint8_t _c64_cia2_in(int port_id, void* user_data) {
    /*
        Read CIA-2 ports:

        Port A:
            bits 0..5: output (see cia2_out)
            bits 6..7: serial bus input, not implemented
        Port B:
            RS232 / user functionality (not implemented)
    */
    return ~0;
}

static uint16_t _c64_vic_fetch(uint16_t addr, void* user_data) {
    c64_t* sys = (c64_t*) user_data;
    /*
        Fetch data into the VIC-II.

        The VIC-II has a 14-bit address bus and 12-bit data bus, and
        has a different memory mapping then the CPU (that's why it
        goes through the mem_vic pagetable):
            - a full 16-bit address is formed by taking the address bits
              14 and 15 from the value written to CIA-1 port A
            - the lower 8 bits of the VIC-II data bus are connected
              to the shared system data bus, this is used to read
              character mask and pixel data
            - the upper 4 bits of the VIC-II data bus are hardwired to the
              static color RAM
    */
    addr |= sys->vic_bank_select;
    uint16_t data = (sys->color_ram[addr & 0x03FF]<<8) | mem_rd(&sys->mem_vic, addr);
    return data;
}

static void _c64_update_memory_map(c64_t* sys) {
    sys->io_mapped = false;
    uint8_t* read_ptr;
    /* shortcut if HIRAM and LORAM is 0, everything is RAM */
    if ((sys->cpu_port & (C64_CPUPORT_HIRAM|C64_CPUPORT_LORAM)) == 0) {
        mem_map_ram(&sys->mem_cpu, 0, 0xA000, 0x6000, sys->ram+0xA000);
    }
    else {
        /* A000..BFFF is either RAM-behind-BASIC-ROM or RAM */
        if ((sys->cpu_port & (C64_CPUPORT_HIRAM|C64_CPUPORT_LORAM)) == (C64_CPUPORT_HIRAM|C64_CPUPORT_LORAM)) {
            read_ptr = sys->rom_basic;
        }
        else {
            read_ptr = sys->ram + 0xA000;
        }
        mem_map_rw(&sys->mem_cpu, 0, 0xA000, 0x2000, read_ptr, sys->ram+0xA000);

        /* E000..FFFF is either RAM-behind-KERNAL-ROM or RAM */
        if (sys->cpu_port & C64_CPUPORT_HIRAM) {
            read_ptr = sys->rom_kernal;
        }
        else {
            read_ptr = sys->ram + 0xE000;
        }
        mem_map_rw(&sys->mem_cpu, 0, 0xE000, 0x2000, read_ptr, sys->ram+0xE000);

        /* D000..DFFF can be Char-ROM or I/O */
        if  (sys->cpu_port & C64_CPUPORT_CHAREN) {
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

/*=== CASSETTE TAPE FILE LOADING =============================================*/

/* C64 TAP file header */
typedef struct {
    uint8_t signature[12];  /* "C64-TAPE-RAW" */
    uint8_t version;        /* 0x00 or 0x01 */
    uint8_t pad[3];         /* reserved */
    uint32_t size;          /* size of the following data */
} _c64_tap_header;

bool c64_insert_tape(c64_t* sys, const uint8_t* ptr, int num_bytes) {
    CHIPS_ASSERT(sys && sys->valid && ptr);
    c64_remove_tape(sys);
    if (num_bytes <= (int) sizeof(_c64_tap_header)) {
        return false;
    }
    const _c64_tap_header* hdr = (const _c64_tap_header*) ptr;
    ptr += sizeof(_c64_tap_header);
    const uint8_t sig[12] = { 'C','6','4','-','T','A','P','E','-','R','A','W'};
    for (int i = 0; i < 12; i++) {
        if (sig[i] != hdr->signature[i]) {
            return false;
        }
    }
    if (1 != hdr->version) {
        return false;
    }
    if (num_bytes < (int)(hdr->size + sizeof(_c64_tap_header))) {
        return false;
    }
    if (num_bytes > (int)sizeof(sys->tape_buf)) {
        return false;
    }
    memcpy(sys->tape_buf, ptr, hdr->size);
    sys->tape_size = hdr->size;
    sys->tape_pos = 0;
    sys->tape_tick_count = 0;
    return true;
}

void c64_remove_tape(c64_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    sys->tape_motor = false;
    sys->tape_button = false;
    sys->tape_size = 0;
    sys->tape_pos = 0;
    sys->tape_tick_count = 0;
}

void c64_start_tape(c64_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    sys->tape_button = true;
    sys->tape_motor = true;
}

void c64_stop_tape(c64_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    sys->tape_button = false;
    sys->tape_motor = false;
}

static bool _c64_tape_tick(c64_t* sys) {
    if (sys->tape_motor && (sys->tape_size > 0) && (sys->tape_pos <= sys->tape_size)) {
        if (sys->tape_tick_count == 0) {
            if (sys->tape_sound) {
                beeper_toggle(&sys->beeper);
            }
            uint8_t val = sys->tape_buf[sys->tape_pos++];
            if (val == 0) {
                uint8_t s[3];
                for (int i = 0; i < 3; i++) {
                    s[i] = sys->tape_buf[sys->tape_pos++];
                }
                sys->tape_tick_count = (s[2]<<16) | (s[1]<<8) | s[0];
            }
            else {
                sys->tape_tick_count = val * 8;
            }
            return true;
        }
        else {
            sys->tape_tick_count--;
        }
    }
    return false;
}

/* FIXME: add proper snapshot file formats */
bool c64_quickload(c64_t* sys, const uint8_t* ptr, int num_bytes) {
    CHIPS_ASSERT(sys && sys->valid);
    if (num_bytes < 2) {
        return false;
    }
    const uint16_t start_addr = ptr[1]<<8 | ptr[0];
    ptr += 2;
    const uint16_t end_addr = start_addr + (num_bytes - 2);
    uint16_t addr = start_addr;
    while (addr < end_addr) {
        mem_wr(&sys->mem_cpu, addr++, *ptr++);
    }
    return true;
}
#endif /* CHIPS_IMPL */
