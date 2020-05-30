#pragma once
/*#
    # atomx.h

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

    You need to include the following headers before including atom.h:

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

#define ATOM_FREQUENCY (1000000)
#define ATOM_MAX_AUDIO_SAMPLES (1024)       /* max number of audio samples in internal sample buffer */
#define ATOM_DEFAULT_AUDIO_SAMPLES (128)    /* default number of samples in internal sample buffer */
#define ATOM_MAX_TAPE_SIZE (1<<16)          /* max size of tape file in bytes */

/* joystick emulation types */
typedef enum {
    ATOM_JOYSTICKTYPE_NONE,
    ATOM_JOYSTICKTYPE_MMC
} atom_joystick_type_t;

/* joystick mask bits */
#define ATOM_JOYSTICK_RIGHT (1<<0)
#define ATOM_JOYSTICK_LEFT  (1<<1)
#define ATOM_JOYSTICK_DOWN  (1<<2)
#define ATOM_JOYSTICK_UP    (1<<3)
#define ATOM_JOYSTICK_BTN   (1<<4)

/* audio sample data callback */
typedef void (*atom_audio_callback_t)(const float* samples, int num_samples, void* user_data);

/* configuration parameters for atom_init() */
typedef struct {
    atom_joystick_type_t joystick_type;     /* what joystick type to emulate, default is ATOM_JOYSTICK_NONE */

    /* video output config */
    void* pixel_buffer;         /* pointer to a linear RGBA8 pixel buffer, at least 320*256*4 bytes */
    int pixel_buffer_size;      /* size of the pixel buffer in bytes */

    /* optional user-data for callbacks */
    void* user_data;

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
    uint64_t pins;
    m6502_t cpu;
    mc6847_t vdg;
    i8255_t ppi;
    m6522_t via;
    beeper_t beeper;
    bool valid;
    int counter_2_4khz;
    int period_2_4khz;
    bool state_2_4khz;
    atom_joystick_type_t joystick_type;
    uint8_t kbd_joymask;        /* joystick mask from keyboard-joystick-emulation */
    uint8_t joy_joymask;        /* joystick mask from calls to atom_joystick() */
    uint8_t mmc_cmd;
    uint8_t mmc_latch;
    mem_t mem;
    kbd_t kbd;
    void* user_data;
    atom_audio_callback_t audio_cb;
    int num_samples;
    int sample_pos;
    float sample_buffer[ATOM_MAX_AUDIO_SAMPLES];
    uint8_t ram[0xA000];
    uint8_t rom_abasic[0x2000];
    uint8_t rom_afloat[0x1000];
    uint8_t rom_dosrom[0x1000];
    /* tape loading */
    int tape_size;  /* tape_size is > 0 if a tape is inserted */
    int tape_pos;
    uint8_t tape_buf[ATOM_MAX_TAPE_SIZE];
} atom_t;

/* initialize a new Atom instance */
void atom_init(atom_t* sys, const atom_desc_t* desc);
/* discard Atom instance */
void atom_discard(atom_t* sys);
/* get the standard framebuffer width and height in pixels */
int atom_std_display_width(void);
int atom_std_display_height(void);
/* get the maximum framebuffer size in number of bytes */
int atom_max_display_size(void);
/* get the current framebuffer width and height in pixels */
int atom_display_width(atom_t* sys);
int atom_display_height(atom_t* sys);
/* reset Atom instance */
void atom_reset(atom_t* sys);
/* execute a single tick */
void atom_tick(atom_t* sys);
/* run Atom instance for a number of microseconds */
void atom_exec(atom_t* sys, uint32_t micro_seconds);
/* send a key down event */
void atom_key_down(atom_t* sys, int key_code);
/* send a key up event */
void atom_key_up(atom_t* sys, int key_code);
/* enable/disable joystick emulation */
void atom_set_joystick_type(atom_t* sys, atom_joystick_type_t type);
/* get current joystick emulation type */
atom_joystick_type_t atom_joystick_type(atom_t* sys);
/* set joystick mask (combination of ATOM_JOYSTICK_*) */
void atom_joystick(atom_t* sys, uint8_t mask);
/* insert a tape for loading (must be an Atom TAP file), data will be copied */
bool atom_insert_tape(atom_t* sys, const uint8_t* ptr, int num_bytes);
/* remove tape */
void atom_remove_tape(atom_t* sys);

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

#define _ATOM_ROM_DOSROM_SIZE (0x1000)

static uint64_t _atom_tick(atom_t* sys, uint64_t pins);
static uint64_t _atom_vdg_fetch(uint64_t pins, void* user_data);
static void _atom_init_keymap(atom_t* sys);
static void _atom_init_memorymap(atom_t* sys);
static uint64_t _atom_osload(atom_t* sys, uint64_t pins);

#define _ATOM_DEFAULT(val,def) (((val) != 0) ? (val) : (def))
#define _ATOM_CLEAR(val) memset(&val, 0, sizeof(val))

void atom_init(atom_t* sys, const atom_desc_t* desc) {
    CHIPS_ASSERT(sys && desc);
    CHIPS_ASSERT(desc->pixel_buffer && (desc->pixel_buffer_size >= atom_max_display_size()));

    memset(sys, 0, sizeof(atom_t));
    sys->valid = true;
    sys->joystick_type = desc->joystick_type;
    sys->user_data = desc->user_data;
    sys->audio_cb = desc->audio_cb;
    sys->num_samples = _ATOM_DEFAULT(desc->audio_num_samples, ATOM_DEFAULT_AUDIO_SAMPLES);
    CHIPS_ASSERT(sys->num_samples <= ATOM_MAX_AUDIO_SAMPLES);
    CHIPS_ASSERT(desc->rom_abasic && (desc->rom_abasic_size == sizeof(sys->rom_abasic)));
    memcpy(sys->rom_abasic, desc->rom_abasic, sizeof(sys->rom_abasic));
    CHIPS_ASSERT(desc->rom_afloat && (desc->rom_afloat_size == sizeof(sys->rom_afloat)));
    memcpy(&sys->rom_afloat, desc->rom_afloat, sizeof(sys->rom_afloat));
    CHIPS_ASSERT(desc->rom_dosrom && (desc->rom_dosrom_size == sizeof(sys->rom_dosrom)));
    memcpy(&sys->rom_dosrom, desc->rom_dosrom, sizeof(sys->rom_dosrom));

    /* initialize the hardware */
    sys->period_2_4khz = ATOM_FREQUENCY / 4800;

    m6502_desc_t cpu_desc;
    _ATOM_CLEAR(cpu_desc);
    sys->pins = m6502_init(&sys->cpu, &cpu_desc);

    mc6847_desc_t vdg_desc;
    _ATOM_CLEAR(vdg_desc);
    vdg_desc.tick_hz = ATOM_FREQUENCY;
    vdg_desc.rgba8_buffer = (uint32_t*) desc->pixel_buffer;
    vdg_desc.rgba8_buffer_size = desc->pixel_buffer_size;
    vdg_desc.fetch_cb = _atom_vdg_fetch;
    vdg_desc.user_data = sys;
    mc6847_init(&sys->vdg, &vdg_desc);

    i8255_init(&sys->ppi);
    m6522_init(&sys->via);

    const int audio_hz = _ATOM_DEFAULT(desc->audio_sample_rate, 44100);
    const float audio_vol = _ATOM_DEFAULT(desc->audio_volume, 0.5f);
    beeper_init(&sys->beeper, ATOM_FREQUENCY, audio_hz, audio_vol);

    /* setup memory map and keyboard matrix */
    _atom_init_memorymap(sys);
    _atom_init_keymap(sys);
}

void atom_discard(atom_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    sys->valid = false;
}

int atom_std_display_width(void) {
    return MC6847_DISPLAY_WIDTH;
}

int atom_std_display_height(void) {
    return MC6847_DISPLAY_HEIGHT;
}

int atom_max_display_size(void) {
    return MC6847_DISPLAY_WIDTH * MC6847_DISPLAY_HEIGHT * 4;
}

int atom_display_width(atom_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    (void)sys;
    return MC6847_DISPLAY_WIDTH;
}

int atom_display_height(atom_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    (void)sys;
    return MC6847_DISPLAY_HEIGHT;
}

void atom_reset(atom_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    sys->pins |= M6502_RES;
    i8255_reset(&sys->ppi);
    m6522_reset(&sys->via);
    mc6847_reset(&sys->vdg);
    beeper_reset(&sys->beeper);
    sys->state_2_4khz = false;
}

void atom_tick(atom_t* sys) {
    sys->pins = _atom_tick(sys, sys->pins);
}

void atom_exec(atom_t* sys, uint32_t micro_seconds) {
    CHIPS_ASSERT(sys && sys->valid);
    uint32_t num_ticks = clk_us_to_ticks(ATOM_FREQUENCY, micro_seconds);
    for (uint32_t ticks = 0; ticks < num_ticks; ticks++) {
        sys->pins = _atom_tick(sys, sys->pins);
    }
    kbd_update(&sys->kbd, micro_seconds);
}

void atom_key_down(atom_t* sys, int key_code) {
    CHIPS_ASSERT(sys && sys->valid);
    switch (sys->joystick_type) {
        case ATOM_JOYSTICKTYPE_NONE:
            kbd_key_down(&sys->kbd, key_code);
            break;
        case ATOM_JOYSTICKTYPE_MMC:
            switch (key_code) {
                case 0x20:  sys->kbd_joymask |= ATOM_JOYSTICK_BTN; break;
                case 0x08:  sys->kbd_joymask |= ATOM_JOYSTICK_LEFT; break;
                case 0x09:  sys->kbd_joymask |= ATOM_JOYSTICK_RIGHT; break;
                case 0x0A:  sys->kbd_joymask |= ATOM_JOYSTICK_DOWN; break;
                case 0x0B:  sys->kbd_joymask |= ATOM_JOYSTICK_UP; break;
                default:    kbd_key_down(&sys->kbd, key_code); break;
            }
            break;
    }
}

void atom_key_up(atom_t* sys, int key_code) {
    CHIPS_ASSERT(sys && sys->valid);
    switch (sys->joystick_type) {
        case ATOM_JOYSTICKTYPE_NONE:
            kbd_key_up(&sys->kbd, key_code);
            break;
        case ATOM_JOYSTICKTYPE_MMC:
            switch (key_code) {
                case 0x20:  sys->kbd_joymask &= ~ATOM_JOYSTICK_BTN; break;
                case 0x08:  sys->kbd_joymask &= ~ATOM_JOYSTICK_LEFT; break;
                case 0x09:  sys->kbd_joymask &= ~ATOM_JOYSTICK_RIGHT; break;
                case 0x0A:  sys->kbd_joymask &= ~ATOM_JOYSTICK_DOWN; break;
                case 0x0B:  sys->kbd_joymask &= ~ATOM_JOYSTICK_UP; break;
                default:    kbd_key_up(&sys->kbd, key_code); break;
            }
            break;
    }
}

void atom_set_joystick_type(atom_t* sys, atom_joystick_type_t type) {
    CHIPS_ASSERT(sys && sys->valid);
    sys->joystick_type = type;
}

atom_joystick_type_t atom_joystick_type(atom_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    return sys->joystick_type;
}

void atom_joystick(atom_t* sys, uint8_t mask) {
    CHIPS_ASSERT(sys && sys->valid);
    sys->joy_joymask = mask;
}

/* CPU tick callback */
uint64_t _atom_tick(atom_t* sys, uint64_t cpu_pins) {

    /* tick the CPU */
    cpu_pins = m6502_tick(&sys->cpu, cpu_pins);

    /* tick the 2.4khz counter */
    sys->counter_2_4khz++;
    if (sys->counter_2_4khz >= sys->period_2_4khz) {
        sys->state_2_4khz = !sys->state_2_4khz;
        sys->counter_2_4khz -= sys->period_2_4khz;
    }

    /* update beeper */
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

    /* address decoding */
    const uint16_t addr = M6502_GET_ADDR(cpu_pins);
    uint64_t via_pins = cpu_pins & M6502_PIN_MASK;
    uint64_t ppi_pins = cpu_pins & M6502_PIN_MASK & ~(I8255_RD|I8255_WR|I8255_PC_PINS);
    uint64_t vdg_pins = 0;
    if ((addr >= 0xB000) && (addr < 0xC000)) {
        /* memory-mapped IO area */
        if ((addr >= 0xB000) && (addr < 0xB400)) {
            ppi_pins |= I8255_CS;
        }
        else if ((addr >= 0xB400) && (addr < 0xB800)) {
            /* extensions (only rudimentary)
                FIXME: implement a proper AtoMMC emulation, for now just
                a quick'n'dirty hack for joystick input
            */
            if (cpu_pins & M6502_RW) {
                /* read from MMC extension */
                if (addr == 0xB400) {
                    /* reading from 0xB400 returns a status/error code, the important
                        ones are STATUS_OK=0x3F, and STATUS_BUSY=0x80, STATUS_COMPLETE
                        together with an error code is used to communicate errors
                    */
                    M6502_SET_DATA(cpu_pins, 0x3F);
                }
                else if ((addr == 0xB401) && (sys->mmc_cmd == 0xA2)) {
                    /* read MMC joystick */
                    M6502_SET_DATA(cpu_pins, ~(sys->kbd_joymask | sys->joy_joymask));
                }
            }
            else {
                /* write to MMC extension */
                if (addr == 0xB400) {
                    sys->mmc_cmd = M6502_GET_DATA(cpu_pins);
                }
            }
        } 
        else if ((addr >= 0xB800) && (addr < 0xBC00)) {
            /* 6522 VIA: http://www.acornatom.nl/sites/fpga/www.howell1964.freeserve.co.uk/acorn/atom/amb/amb_6522.htm */
            via_pins |= M6522_CS1;
        }
        else {
            /* remaining IO space is for expansion devices */
            if (cpu_pins & M6502_RW) {
                M6502_SET_DATA(cpu_pins, 0x00);
            }
        }
    }
    else {
        /* regular memory access */
        if (cpu_pins & M6502_RW) {
            /* memory read */
            M6502_SET_DATA(cpu_pins, mem_rd(&sys->mem, addr));
        }
        else {
            /* memory access */
            mem_wr(&sys->mem, addr, M6502_GET_DATA(cpu_pins));
        }
    }

    /* tick the PPI
        http://www.acornatom.nl/sites/fpga/www.howell1964.freeserve.co.uk/acorn/atom/amb/amb_8255.htm

        Port inputs:
            PB0..PB7:   keyboard matrix rows
            PC4:        2400 Hz tick
            PC5:        cassette input (FIXME: not emulated)
            PC6:        keyboard repeat (FIXME: not emulated)
            PC7:        MC6847 FSYNC

        Port output:
            PA0..PA3:   keyboard matrix column nr.
            PA4..PA7:   MC6847 graphics mode (4: A/G, 5..7: GM0..2)
            PC0:        cassette output (FIXME: not emulated)
            PC1:        enable 2.4kHz to cassette output
            PC2:        beeper
            PC3:        MC6847 CSS

        The port C output lines (PC0 to PC3) may be used for user
        applications when the cassette interface is not being used.
    */
    {
        ppi_pins |= (cpu_pins & M6502_RW) ? I8255_RD : I8255_WR;
        const uint8_t kbd_lines = (uint8_t) kbd_scan_lines(&sys->kbd);
        I8255_SET_PB(ppi_pins, ~kbd_lines);
        if (sys->state_2_4khz) {
            ppi_pins |= I8255_PC4;
        }
        ppi_pins |= I8255_PC6;
        if (0 == (sys->vdg.pins & MC6847_FS)) {
            ppi_pins |= I8255_PC7;
        }
        ppi_pins = i8255_tick(&sys->ppi, ppi_pins);
        const uint16_t kbd_column = 1<<(I8255_GET_PA(ppi_pins) & 0x0F);
        kbd_set_active_columns(&sys->kbd, kbd_column);
        if (ppi_pins & I8255_PA4) { vdg_pins |= MC6847_AG; }
        if (ppi_pins & I8255_PA5) { vdg_pins |= MC6847_GM0; }
        if (ppi_pins & I8255_PA6) { vdg_pins |= MC6847_GM1; }
        if (ppi_pins & I8255_PA7) { vdg_pins |= MC6847_GM2; }
        beeper_set(&sys->beeper, 0 == (ppi_pins & I8255_PC2));
        if (ppi_pins & I8255_PC3) {
            vdg_pins |= MC6847_CSS;
        }
        if((ppi_pins & (I8255_RD|I8255_CS)) == (I8255_RD|I8255_CS)) {
            cpu_pins = M6502_COPY_DATA(cpu_pins, ppi_pins);
        }
    }

    /* tick the VIA */
    {
        via_pins = m6522_tick(&sys->via, via_pins);
        if ((via_pins & (M6522_RW|M6522_CS1)) == (M6522_RW|M6522_CS1)) {
            cpu_pins = M6502_COPY_DATA(cpu_pins, via_pins);
        }
        cpu_pins = (cpu_pins & ~M6502_IRQ) | (via_pins & M6502_IRQ);
    }

    /* tick the VDG, we'll need the HS pin in the next tick as input
       to the VIA, but we can get this directly from sys->vdg.pins,
       so no point in looking at the returned pin mask
    */
    mc6847_tick(&sys->vdg, vdg_pins);

    /* check if the trapped OSLoad function was hit to implement tape file loading
        http://ladybug.xs4all.nl/arlet/fpga/6502/kernel.dis
    */
    if (sys->tape_size > 0) {
        const uint64_t trap_mask = M6502_SYNC|0xFFFF;
        const uint64_t trap_val  = M6502_SYNC|0xF96E;
        if ((cpu_pins & trap_mask) == trap_val) {
            cpu_pins = _atom_osload(sys, cpu_pins);
        }
    }
    return cpu_pins;
}

uint64_t _atom_vdg_fetch(uint64_t pins, void* user_data) {
    atom_t* sys = (atom_t*) user_data;
    const uint16_t addr = MC6847_GET_ADDR(pins);
    uint8_t data = sys->ram[(addr + 0x8000) & 0xFFFF];
    MC6847_SET_DATA(pins, data);

    /*  the upper 2 databus bits are directly wired to MC6847 pins:
        bit 7 -> INV pin (in text mode, invert pixel pattern)
        bit 6 -> A/S and INT/EXT pin, A/S actives semigraphics mode
                 and INT/EXT selects the 2x3 semigraphics pattern
                 (so 4x4 semigraphics isn't possible)
    */
    if (data & (1<<7)) { pins |= MC6847_INV; }
    else               { pins &= ~MC6847_INV; }
    if (data & (1<<6)) { pins |= (MC6847_AS|MC6847_INTEXT); }
    else               { pins &= ~(MC6847_AS|MC6847_INTEXT); }
    return pins;
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

/*=== FILE LOADING ===========================================================*/
/* Atom TAP / ATM header (https://github.com/hoglet67/Atomulator/blob/master/docs/atommmc2.txt ) */
typedef struct {
    uint8_t name[16];
    uint16_t load_addr;
    uint16_t exec_addr;
    uint16_t length;
} _atom_tap_header;

bool atom_insert_tape(atom_t* sys, const uint8_t* ptr, int num_bytes) {
    CHIPS_ASSERT(sys && sys->valid);
    CHIPS_ASSERT(ptr);
    atom_remove_tape(sys);
    /* check for valid size */
    if ((num_bytes < (int)sizeof(_atom_tap_header)) || (num_bytes > ATOM_MAX_TAPE_SIZE)) {
        return false;
    }
    memcpy(sys->tape_buf, ptr, num_bytes);
    sys->tape_pos = 0;
    sys->tape_size = num_bytes;
    return true;
}

void atom_remove_tape(atom_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    sys->tape_pos = 0;
    sys->tape_size = 0;
}

/*
    trapped OSLOAD function, load ATM block in a TAP file:
      https://github.com/hoglet67/Atomulator/blob/master/docs/atommmc2.tx
    from: http://ladybug.xs4all.nl/arlet/fpga/6502/kernel.di
      OSLOAD Load File subroutine
       --------------------------
     - Entry: 0,X = LSB File name string address
              1,X = MSB File name string address
              2,X = LSB Data dump start address
              3,X = MSB Data dump start address
              4,X : If bit 7 is clear, then the file's own start address is
                    to be used
              #DD = FLOAD flag - bit 7 is set if in FLOAD mod
     - Uses:  #C9 = LSB File name string address
              #CA = MSB File name string address
              #CB = LSB Data dump start address
              #CC = MSB Data dump start address
              #CD = load flag - if bit 7 is set, then the load address at
                    (#CB) is to be used instead of the file's load address
              #D0 = MSB Current block number
              #D1 = LSB Current block numbe
     - Header format: <*>                      )
                      <*>                      )
                      <*>                      )
                      <*>                      ) Header preamble
                      <Filename>               ) Name is 1 to 13 bytes long
                      <Status Flag>            ) Bit 7 clear if last block
                                               ) Bit 6 clear to skip block
                                               ) Bit 5 clear if first block
                      <LSB block number>
                      <MSB block number>       ) Always zero
                      <Bytes in block>
                      <MSB run address>
                      <LSB run address>
                      <MSB block load address>
                      <LSB block load address
     - Data format:   <....data....>           ) 1 to #FF bytes
                      <Checksum>               ) LSB sum of all data byte
*/
uint64_t _atom_osload(atom_t* sys, uint64_t pins) {
    bool success = false;

    /* tape inserted? */
    uint16_t exec_addr = 0;
    if ((sys->tape_size > 0) && (sys->tape_pos < sys->tape_size)) {
        /* read next tape chunk */
        if ((int)(sys->tape_pos + sizeof(_atom_tap_header)) < sys->tape_size) {
            const _atom_tap_header* hdr = (const _atom_tap_header*) &sys->tape_buf[sys->tape_pos];
            sys->tape_pos += sizeof(_atom_tap_header);
            exec_addr = hdr->exec_addr;
            uint16_t addr = hdr->load_addr;
            /* override file load address? */
            if (mem_rd(&sys->mem, 0xCD) & 0x80) {
                addr = mem_rd16(&sys->mem, 0xCB);
            }
            if ((sys->tape_pos + hdr->length) <= sys->tape_size) {
                for (int i = 0; i < hdr->length; i++) {
                    mem_wr(&sys->mem, addr++, sys->tape_buf[sys->tape_pos++]);
                }
                success = true;
            }
        }
    }
    /* if tape at end, remove tape */
    if (sys->tape_pos >= sys->tape_size) {
        atom_remove_tape(sys);
    }
    /* success/fail: set or clear bit 6 and clear bit 7 of 0xDD */
    uint8_t dd = mem_rd(&sys->mem, 0xDD);
    if (success) {
        dd |= (1<<6);
    }
    else {
        dd &= ~(1<<6);
    }
    dd &= ~(1<<7);
    mem_wr(&sys->mem, 0xDD, dd);

    if (success) {
        /* on success, continue with start of loaded code */
        sys->cpu.S += 2;
        M6502_SET_ADDR(pins, exec_addr);
        M6502_SET_DATA(pins, mem_rd(&sys->mem, exec_addr));
        m6502_set_pc(&sys->cpu, exec_addr);
    }
    else {
        /* otherwise just continue with an RTS */
        M6502_SET_ADDR(pins, 0xF9A1);
        M6502_SET_DATA(pins, mem_rd(&sys->mem, 0xF9A1));
        m6502_set_pc(&sys->cpu, 0xF9A1);
    }
    return pins;
}

#endif /* CHIPS_IMPL */
