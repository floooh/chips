#pragma once
/*#
    # zx.h

    A ZX Spectrum 48K / 128 emulator in a C header.

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

    You need to include the following headers before including zx.h:

    - chips/chips_common.h
    - chips/z80.h
    - chips/beeper.h
    - chips/ay38910.h
    - chips/mem.h
    - chips/kbd.h
    - chips/clk.h

    ## The ZX Spectrum 48K

    TODO!

    ## The ZX Spectrum 128

    TODO!

    ## TODO:
    - 'contended memory' timing and IO port timing
    - reads from port 0xFF must return 'current VRAM bytes
    - video decoding only has scanline accuracy, not pixel accuracy

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
#include <stddef.h>
#include <stdalign.h>

#ifdef __cplusplus
extern "C" {
#endif

// bump this whenever the zx_t struct layout changes
#define ZX_SNAPSHOT_VERSION (0x0001)

#define ZX_MAX_AUDIO_SAMPLES (1024)      // max number of audio samples in internal sample buffer
#define ZX_DEFAULT_AUDIO_SAMPLES (128)   // default number of samples in internal sample buffer
#define ZX_FRAMEBUFFER_WIDTH (512)
#define ZX_FRAMEBUFFER_HEIGHT (256)
#define ZX_FRAMEBUFFER_SIZE_BYTES (ZX_FRAMEBUFFER_WIDTH * ZX_FRAMEBUFFER_HEIGHT)
#define ZX_DISPLAY_WIDTH (320)
#define ZX_DISPLAY_HEIGHT (256)

// ZX Spectrum models
typedef enum {
    ZX_TYPE_48K,
    ZX_TYPE_128,
} zx_type_t;

// ZX Spectrum joystick types
typedef enum {
    ZX_JOYSTICKTYPE_NONE,
    ZX_JOYSTICKTYPE_KEMPSTON,
    ZX_JOYSTICKTYPE_SINCLAIR_1,
    ZX_JOYSTICKTYPE_SINCLAIR_2,
} zx_joystick_type_t;

// joystick mask bits
#define ZX_JOYSTICK_RIGHT   (1<<0)
#define ZX_JOYSTICK_LEFT    (1<<1)
#define ZX_JOYSTICK_DOWN    (1<<2)
#define ZX_JOYSTICK_UP      (1<<3)
#define ZX_JOYSTICK_BTN     (1<<4)

// config parameters for zx_init()
typedef struct {
    zx_type_t type;                     // default is ZX_TYPE_48K
    zx_joystick_type_t joystick_type;   // what joystick to emulate, default is ZX_JOYSTICK_NONE
    chips_debug_t debug;                // optional debugger hook
    struct {
        chips_audio_callback_t callback;
        int num_samples;
        int sample_rate;
        float beeper_volume;
        float ay_volume;
    } audio;
    // ROM images
    struct {
        // ZX Spectrum 48K
        chips_range_t zx48k;
        // ZX Spectrum 128
        chips_range_t zx128_0;
        chips_range_t zx128_1;
    } roms;
} zx_desc_t;

// ZX emulator state
typedef struct {
    z80_t cpu;
    beeper_t beeper;
    ay38910_t ay;
    zx_type_t type;
    zx_joystick_type_t joystick_type;
    bool memory_paging_disabled;
    uint8_t kbd_joymask;        // joystick mask from keyboard joystick emulation
    uint8_t joy_joymask;        // joystick mask from zx_joystick()
    uint32_t tick_count;
    uint8_t last_mem_config;    // last out to 0x7FFD
    uint8_t last_fe_out;        // last out value to 0xFE port
    uint8_t blink_counter;      // incremented on each vblank
    uint8_t border_color;
    int frame_scan_lines;
    int top_border_scanlines;
    int scanline_period;
    int scanline_counter;
    int scanline_y;
    int int_counter;
    uint32_t display_ram_bank;
    kbd_t kbd;
    mem_t mem;
    uint64_t pins;
    uint64_t freq_hz;
    bool valid;
    chips_debug_t debug;
    struct {
        chips_audio_callback_t callback;
        int num_samples;
        int sample_pos;
        float sample_buffer[ZX_MAX_AUDIO_SAMPLES];
    } audio;
    uint8_t ram[8][0x4000];
    uint8_t rom[2][0x4000];
    uint8_t junk[0x4000];
    alignas(64) uint8_t fb[ZX_FRAMEBUFFER_SIZE_BYTES];
} zx_t;

// initialize a new ZX Spectrum instance
void zx_init(zx_t* sys, const zx_desc_t* desc);
// discard a ZX Spectrum instance
void zx_discard(zx_t* sys);
// reset a ZX Spectrum instance
void zx_reset(zx_t* sys);
// query information about display requirements, can be called with nullptr
chips_display_info_t zx_display_info(zx_t* sys);
// run ZX Spectrum instance for a given number of microseconds, return number of ticks
uint32_t zx_exec(zx_t* sys, uint32_t micro_seconds);
// send a key-down event
void zx_key_down(zx_t* sys, int key_code);
// send a key-up event
void zx_key_up(zx_t* sys, int key_code);
// enable/disable joystick emulation
void zx_set_joystick_type(zx_t* sys, zx_joystick_type_t type);
// get current joystick emulation type
zx_joystick_type_t zx_joystick_type(zx_t* sys);
// set joystick mask (combination of ZX_JOYSTICK_*)
void zx_joystick(zx_t* sys, uint8_t mask);
// load a ZX Z80 file into the emulator
bool zx_quickload(zx_t* sys, chips_range_t data);
// save a snapshot, patches any pointers to zero, returns a snapshot version
uint32_t zx_save_snapshot(zx_t* sys, zx_t* dst);
// load a snapshot, returns false if snapshot version doesn't match
bool zx_load_snapshot(zx_t* sys, uint32_t version, zx_t* src);

#ifdef __cplusplus
} // extern "C"
#endif

/*-- IMPLEMENTATION ----------------------------------------------------------*/
#ifdef CHIPS_IMPL
#include <string.h>
#ifndef CHIPS_ASSERT
    #include <assert.h>
    #define CHIPS_ASSERT(c) assert(c)
#endif

static void _zx_init_memory_map(zx_t* sys);
static void _zx_init_keyboard_matrix(zx_t* sys);

#define _ZX_DEFAULT(val,def) (((val) != 0) ? (val) : (def))

#define _ZX_48K_FREQUENCY (3500000)
#define _ZX_128_FREQUENCY (3546894)

void zx_init(zx_t* sys, const zx_desc_t* desc) {
    CHIPS_ASSERT(sys && desc);
    if (desc->debug.callback.func) { CHIPS_ASSERT(desc->debug.stopped); }

    memset(sys, 0, sizeof(zx_t));
    sys->valid = true;
    sys->type = desc->type;
    sys->joystick_type = desc->joystick_type;
    sys->freq_hz = (sys->type == ZX_TYPE_48K) ? _ZX_48K_FREQUENCY : _ZX_128_FREQUENCY;
    sys->audio.callback = desc->audio.callback;
    sys->audio.num_samples = _ZX_DEFAULT(desc->audio.num_samples, ZX_DEFAULT_AUDIO_SAMPLES);
    CHIPS_ASSERT(sys->audio.num_samples <= ZX_MAX_AUDIO_SAMPLES);
    sys->debug = desc->debug;

    // initalize the hardware
    sys->border_color = 0;
    if (ZX_TYPE_128 == sys->type) {
        CHIPS_ASSERT(desc->roms.zx128_0.ptr && (desc->roms.zx128_0.size == 0x4000));
        CHIPS_ASSERT(desc->roms.zx128_1.ptr && (desc->roms.zx128_1.size == 0x4000));
        memcpy(sys->rom[0], desc->roms.zx128_0.ptr, 0x4000);
        memcpy(sys->rom[1], desc->roms.zx128_1.ptr, 0x4000);
        sys->display_ram_bank = 5;
        sys->frame_scan_lines = 311;
        sys->top_border_scanlines = 63;
        sys->scanline_period = 228;
    }
    else {
        CHIPS_ASSERT(desc->roms.zx48k.ptr && (desc->roms.zx48k.size == 0x4000));
        memcpy(sys->rom[0], desc->roms.zx48k.ptr, 0x4000);
        sys->display_ram_bank = 0;
        sys->frame_scan_lines = 312;
        sys->top_border_scanlines = 64;
        sys->scanline_period = 224;
    }
    sys->scanline_counter = sys->scanline_period;

    sys->pins = z80_init(&sys->cpu);

    const int audio_hz = _ZX_DEFAULT(desc->audio.sample_rate, 44100);
    beeper_init(&sys->beeper, &(beeper_desc_t){
        .tick_hz = (int)sys->freq_hz,
        .sound_hz = audio_hz,
        .base_volume = _ZX_DEFAULT(desc->audio.beeper_volume, 0.25f),
    });
    if (ZX_TYPE_128 == sys->type) {
        ay38910_init(&sys->ay, &(ay38910_desc_t){
            .type = AY38910_TYPE_8912,
            .tick_hz = (int)sys->freq_hz / 2,
            .sound_hz = audio_hz,
            .magnitude = _ZX_DEFAULT(desc->audio.ay_volume, 0.5f)
        });
    }
    _zx_init_memory_map(sys);
    _zx_init_keyboard_matrix(sys);
}

void zx_discard(zx_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    sys->valid = false;
}

void zx_reset(zx_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    sys->pins = z80_reset(&sys->cpu);
    beeper_reset(&sys->beeper);
    if (sys->type == ZX_TYPE_128) {
        ay38910_reset(&sys->ay);
    }
    sys->memory_paging_disabled = false;
    sys->kbd_joymask = 0;
    sys->joy_joymask = 0;
    sys->last_fe_out = 0;
    sys->scanline_counter = sys->scanline_period;
    sys->scanline_y = 0;
    sys->blink_counter = 0;
    if (sys->type == ZX_TYPE_48K) {
        sys->display_ram_bank = 0;
    }
    else {
        sys->display_ram_bank = 5;
    }
    _zx_init_memory_map(sys);
}

static bool _zx_decode_scanline(zx_t* sys) {
    /* this is called by the timer callback for every PAL line, controlling
        the vidmem decoding and vblank interrupt

        detailed information about frame timings is here:
        for 48K:    http://rk.nvg.ntnu.no/sinclair/faq/tech_48.html#48K
        for 128K:   http://rk.nvg.ntnu.no/sinclair/faq/tech_128.html

        one PAL line takes 224 T-states on 48K, and 228 T-states on 128K
        one PAL frame is 312 lines on 48K, and 311 lines on 128K

        decode the next videomem line into the emulator framebuffer,
        the border area of a real Spectrum is bigger than the emulator
        (the emu has 32 pixels border on each side, the hardware has:

        63 or 64 lines top border
        56 border lines bottom border
        48 pixels on each side horizontal border
    */
    const int top_decode_line = sys->top_border_scanlines - 32;
    const int btm_decode_line = sys->top_border_scanlines + 192 + 32;
    if ((sys->scanline_y >= top_decode_line) && (sys->scanline_y < btm_decode_line)) {
        const uint16_t y = sys->scanline_y - top_decode_line;
        uint8_t* dst = &sys->fb[y * ZX_FRAMEBUFFER_WIDTH];
        const uint8_t* vidmem_bank = sys->ram[sys->display_ram_bank];
        const bool blink = 0 != (sys->blink_counter & 0x10);
        if ((y < 32) || (y >= 224)) {
            // upper/lower border
            for (int x = 0; x < ZX_DISPLAY_WIDTH; x++) {
                *dst++ = sys->border_color;
            }
        }
        else {
            /* compute video memory Y offset (inside 256x192 area)
                this is how the 16-bit video memory address is computed
                from X and Y coordinates:
                | 0| 1| 0|Y7|Y6|Y2|Y1|Y0|Y5|Y4|Y3|X4|X3|X2|X1|X0|
            */
            const uint16_t yy = y-32;
            const uint16_t y_offset = ((yy & 0xC0)<<5) | ((yy & 0x07)<<8) | ((yy & 0x38)<<2);

            // left border
            for (int x = 0; x < (4*8); x++) {
                *dst++ = sys->border_color;
            }

            // valid 256x192 vidmem area
            for (uint16_t x = 0; x < 32; x++) {
                const uint16_t pix_offset = y_offset | x;
                const uint16_t clr_offset = 0x1800 + (((yy & ~0x7)<<2) | x);

                // pixel mask and color attribute bytes
                const uint8_t pix = vidmem_bank[pix_offset];
                const uint8_t clr = vidmem_bank[clr_offset];

                // foreground and background color
                uint8_t fg, bg;
                if ((clr & (1<<7)) && blink) {
                    fg = (clr>>3) & 7;
                    bg = clr & 7;
                }
                else {
                    fg = clr & 7;
                    bg = (clr>>3) & 7;
                }
                // color bit 6: standard vs bright
                fg |= (clr & (1<<6)) >> 3;
                bg |= (clr & (1<<6)) >> 3;

                for (int px = 7; px >=0; px--) {
                    *dst++ = pix & (1<<px) ? fg : bg;
                }
            }

            // right border
            for (int x = 0; x < (4*8); x++) {
                *dst++ = sys->border_color;
            }
        }
    }

    if (sys->scanline_y++ >= sys->frame_scan_lines) {
        // start new frame, request vblank interrupt
        sys->scanline_y = 0;
        sys->blink_counter++;
        return true;
    }
    else {
        return false;
    }
}

// ZX128 memory mapping
static void _zx_update_memory_map_zx128(zx_t* sys, uint8_t data) {
    if (!sys->memory_paging_disabled) {
        sys->last_mem_config = data;
        // bit 3 defines the video scanout memory bank (5 or 7)
        sys->display_ram_bank = (data & (1<<3)) ? 7 : 5;
        // only last memory bank is mappable
        mem_map_ram(&sys->mem, 0, 0xC000, 0x4000, sys->ram[data & 0x7]);

        // ROM0 or ROM1
        if (data & (1<<4)) {
            // bit 4 set: ROM1
            mem_map_rom(&sys->mem, 0, 0x0000, 0x4000, sys->rom[1]);
        }
        else {
            // bit 4 clear: ROM0
            mem_map_rom(&sys->mem, 0, 0x0000, 0x4000, sys->rom[0]);
        }
    }
    if (data & (1<<5)) {
        /* bit 5 prevents further changes to memory pages
            until computer is reset, this is used when switching
            to the 48k ROM
        */
        sys->memory_paging_disabled = true;
    }
}

static uint64_t _zx_tick(zx_t* sys, uint64_t pins) {
    pins = z80_tick(&sys->cpu, pins);

    // video decoding and vblank interrupt
    if (--sys->scanline_counter <= 0) {
        sys->scanline_counter += sys->scanline_period;
        // decode next video scanline
        if (_zx_decode_scanline(sys)) {
            // request vblank interrupt
            pins |= Z80_INT;
            // hold the INT pin for 32 ticks
            sys->int_counter = 32;
        }
    }

    // clear INT pin after 32 ticks
    if (pins & Z80_INT) {
        if (--sys->int_counter < 0) {
            pins &= ~Z80_INT;
        }
    }

    if (pins & Z80_MREQ) {
        // a memory request
        // FIXME: 'contended memory'
        const uint16_t addr = Z80_GET_ADDR(pins);
        if (pins & Z80_RD) {
            Z80_SET_DATA(pins, mem_rd(&sys->mem, addr));
        }
        else if (pins & Z80_WR) {
            mem_wr(&sys->mem, addr, Z80_GET_DATA(pins));
        }
    }
    else if (pins & Z80_IORQ) {
        if ((pins & Z80_A0) == 0) {
            /* Spectrum ULA (...............0)
                Bits 5 and 7 as read by INning from Port 0xfe are always one
            */
            if (pins & Z80_RD) {
                // read from ULA
                uint8_t data = (1<<7)|(1<<5);
                // MIC/EAR flags -> bit 6
                if (sys->last_fe_out & (1<<3|1<<4)) {
                    data |= (1<<6);
                }
                // keyboard matrix bits are encoded in the upper 8 bit of the port address
                uint16_t column_mask = (~(Z80_GET_ADDR(pins)>>8)) & 0x00FF;
                const uint16_t kbd_lines = kbd_test_lines(&sys->kbd, column_mask);
                data |= (~kbd_lines) & 0x1F;
                Z80_SET_DATA(pins, data);
            }
            else if (pins & Z80_WR) {
                // write to ULA
                // FIXME: bit 3: MIC output (CAS SAVE, 0=On, 1=Off)
                const uint8_t data = Z80_GET_DATA(pins);
                sys->border_color = data & 7;
                sys->last_fe_out = data;
                beeper_set(&sys->beeper, 0 != (data & (1<<4)));
            }
        }
        else if (((pins & (Z80_WR|Z80_A15|Z80_A1)) == Z80_WR) && (sys->type == ZX_TYPE_128)) {
            /* Spectrum 128 memory control (0.............0.)
                http://8bit.yarek.pl/computer/zx.128/
            */
            _zx_update_memory_map_zx128(sys, Z80_GET_DATA(pins));
        }
        else if (((pins & (Z80_A15|Z80_A1)) == Z80_A15) && (sys->type == ZX_TYPE_128)) {
            // AY-3-8912 access (1*............0.)
            if (pins & Z80_A14) { pins |= AY38910_BC1; }
            if (pins & Z80_WR) { pins |= AY38910_BDIR; }
            pins = ay38910_iorq(&sys->ay, pins) & Z80_PIN_MASK;
        }
        else if ((pins & (Z80_RD|Z80_A7|Z80_A6|Z80_A5)) == Z80_RD) {
            // Kempston Joystick (........000.....)
            Z80_SET_DATA(pins, sys->kbd_joymask | sys->joy_joymask);
        }
    }

    // tick the AY at half frequency, use the buffered chip select
    // pin mask so that the AY doesn't miss any IO requests
    if (++sys->tick_count & 1) {
        ay38910_tick(&sys->ay);
    }

    // tick the beeper
    if (beeper_tick(&sys->beeper)) {
        // new sample ready (if this is not a ZX128, sys->ay.sample will be 0)
        const float sample = sys->beeper.sample + sys->ay.sample;
        sys->audio.sample_buffer[sys->audio.sample_pos++] = sample;
        if (sys->audio.sample_pos == sys->audio.num_samples) {
            if (sys->audio.callback.func) {
                sys->audio.callback.func(sys->audio.sample_buffer, sys->audio.num_samples, sys->audio.callback.user_data);
            }
            sys->audio.sample_pos = 0;
        }
    }
    return pins;
}

uint32_t zx_exec(zx_t* sys, uint32_t micro_seconds) {
    CHIPS_ASSERT(sys && sys->valid);
    const uint32_t num_ticks = clk_us_to_ticks(sys->freq_hz, micro_seconds);
    uint64_t pins = sys->pins;
    if (0 == sys->debug.callback.func) {
        // run without debug hook
        for (uint32_t tick = 0; tick < num_ticks; tick++) {
            pins = _zx_tick(sys, pins);
        }
    }
    else {
        // run with debug hook
        for (uint32_t tick = 0; (tick < num_ticks) && !(*sys->debug.stopped); tick++) {
            pins = _zx_tick(sys, pins);
            sys->debug.callback.func(sys->debug.callback.user_data, pins);
        }
    }
    sys->pins = pins;
    kbd_update(&sys->kbd, micro_seconds);
    return num_ticks;
}

void zx_key_down(zx_t* sys, int key_code) {
    CHIPS_ASSERT(sys && sys->valid);
    switch (sys->joystick_type) {
        case ZX_JOYSTICKTYPE_NONE:
            kbd_key_down(&sys->kbd, key_code);
            break;
        case ZX_JOYSTICKTYPE_KEMPSTON:
            switch (key_code) {
                case 0x20:  sys->kbd_joymask |= ZX_JOYSTICK_BTN; break;
                case 0x08:  sys->kbd_joymask |= ZX_JOYSTICK_LEFT; break;
                case 0x09:  sys->kbd_joymask |= ZX_JOYSTICK_RIGHT; break;
                case 0x0A:  sys->kbd_joymask |= ZX_JOYSTICK_DOWN; break;
                case 0x0B:  sys->kbd_joymask |= ZX_JOYSTICK_UP; break;
                default:    kbd_key_down(&sys->kbd, key_code); break;
            }
            break;
        // the Sinclair joystick ports work as normal keys
        case ZX_JOYSTICKTYPE_SINCLAIR_1:
            switch (key_code) {
                case 0x20:  key_code = '5'; break;    // fire
                case 0x08:  key_code = '1'; break;    // left
                case 0x09:  key_code = '2'; break;    // right
                case 0x0A:  key_code = '3'; break;    // down
                case 0x0B:  key_code = '4'; break;    // up
                default: break;
            }
            kbd_key_down(&sys->kbd, key_code);
            break;
        case ZX_JOYSTICKTYPE_SINCLAIR_2:
            switch (key_code) {
                case 0x20:  key_code = '0'; break;    // fire
                case 0x08:  key_code = '6'; break;    // left
                case 0x09:  key_code = '7'; break;    // right
                case 0x0A:  key_code = '8'; break;    // down
                case 0x0B:  key_code = '9'; break;    // up
                default: break;
            }
            kbd_key_down(&sys->kbd, key_code);
            break;
    }
}

void zx_key_up(zx_t* sys, int key_code) {
    CHIPS_ASSERT(sys && sys->valid);
    switch (sys->joystick_type) {
        case ZX_JOYSTICKTYPE_NONE:
            kbd_key_up(&sys->kbd, key_code);
            break;
        case ZX_JOYSTICKTYPE_KEMPSTON:
            switch (key_code) {
                case 0x20:  sys->kbd_joymask &= ~ZX_JOYSTICK_BTN; break;
                case 0x08:  sys->kbd_joymask &= ~ZX_JOYSTICK_LEFT; break;
                case 0x09:  sys->kbd_joymask &= ~ZX_JOYSTICK_RIGHT; break;
                case 0x0A:  sys->kbd_joymask &= ~ZX_JOYSTICK_DOWN; break;
                case 0x0B:  sys->kbd_joymask &= ~ZX_JOYSTICK_UP; break;
                default:    kbd_key_up(&sys->kbd, key_code); break;
            }
            break;
        // the Sinclair joystick ports work as normal keys
        case ZX_JOYSTICKTYPE_SINCLAIR_1:
            switch (key_code) {
                case 0x20:  key_code = '5'; break;    // fire
                case 0x08:  key_code = '1'; break;    // left
                case 0x09:  key_code = '2'; break;    // right
                case 0x0A:  key_code = '3'; break;    // down
                case 0x0B:  key_code = '4'; break;    // up
                default: break;
            }
            kbd_key_up(&sys->kbd, key_code);
            break;
        case ZX_JOYSTICKTYPE_SINCLAIR_2:
            switch (key_code) {
                case 0x20:  key_code = '0'; break;    // fire
                case 0x08:  key_code = '6'; break;    // left
                case 0x09:  key_code = '7'; break;    // right
                case 0x0A:  key_code = '8'; break;    // down
                case 0x0B:  key_code = '9'; break;    // up
                default: break;
            }
            kbd_key_up(&sys->kbd, key_code);
            break;
    }
}

void zx_set_joystick_type(zx_t* sys, zx_joystick_type_t type) {
    CHIPS_ASSERT(sys && sys->valid);
    sys->joystick_type = type;
}

zx_joystick_type_t zx_joystick_type(zx_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    return sys->joystick_type;
}

void zx_joystick(zx_t* sys, uint8_t mask) {
    CHIPS_ASSERT(sys && sys->valid);
    if (sys->joystick_type == ZX_JOYSTICKTYPE_SINCLAIR_1) {
        if (mask & ZX_JOYSTICK_BTN)   { kbd_key_down(&sys->kbd, '5'); }
        else                          { kbd_key_up(&sys->kbd, '5'); }
        if (mask & ZX_JOYSTICK_LEFT)  { kbd_key_down(&sys->kbd, '1'); }
        else                          { kbd_key_up(&sys->kbd, '1'); }
        if (mask & ZX_JOYSTICK_RIGHT) { kbd_key_down(&sys->kbd, '2'); }
        else                          { kbd_key_up(&sys->kbd, '2'); }
        if (mask & ZX_JOYSTICK_DOWN)  { kbd_key_down(&sys->kbd, '3'); }
        else                          { kbd_key_up(&sys->kbd, '3'); }
        if (mask & ZX_JOYSTICK_UP)    { kbd_key_down(&sys->kbd, '4'); }
        else                          { kbd_key_up(&sys->kbd, '4'); }
    }
    else if (sys->joystick_type == ZX_JOYSTICKTYPE_SINCLAIR_2) {
        if (mask & ZX_JOYSTICK_BTN)   { kbd_key_down(&sys->kbd, '0'); }
        else                          { kbd_key_up(&sys->kbd, '0'); }
        if (mask & ZX_JOYSTICK_LEFT)  { kbd_key_down(&sys->kbd, '6'); }
        else                          { kbd_key_up(&sys->kbd, '6'); }
        if (mask & ZX_JOYSTICK_RIGHT) { kbd_key_down(&sys->kbd, '7'); }
        else                          { kbd_key_up(&sys->kbd, '7'); }
        if (mask & ZX_JOYSTICK_DOWN)  { kbd_key_down(&sys->kbd, '8'); }
        else                          { kbd_key_up(&sys->kbd, '8'); }
        if (mask & ZX_JOYSTICK_UP)    { kbd_key_down(&sys->kbd, '9'); }
        else                          { kbd_key_up(&sys->kbd, '9'); }
    }
    else {
        sys->joy_joymask = mask;
    }
}

static void _zx_init_memory_map(zx_t* sys) {
    mem_init(&sys->mem);
    if (sys->type == ZX_TYPE_128) {
        mem_map_ram(&sys->mem, 0, 0x4000, 0x4000, sys->ram[5]);
        mem_map_ram(&sys->mem, 0, 0x8000, 0x4000, sys->ram[2]);
        mem_map_ram(&sys->mem, 0, 0xC000, 0x4000, sys->ram[0]);
        mem_map_rom(&sys->mem, 0, 0x0000, 0x4000, sys->rom[0]);
    }
    else {
        mem_map_ram(&sys->mem, 0, 0x4000, 0x4000, sys->ram[0]);
        mem_map_ram(&sys->mem, 0, 0x8000, 0x4000, sys->ram[1]);
        mem_map_ram(&sys->mem, 0, 0xC000, 0x4000, sys->ram[2]);
        mem_map_rom(&sys->mem, 0, 0x0000, 0x4000, sys->rom[0]);
    }
}

static void _zx_init_keyboard_matrix(zx_t* sys) {
    // setup keyboard matrix
    kbd_init(&sys->kbd, 1);
    // caps-shift is column 0, line 0
    kbd_register_modifier(&sys->kbd, 0, 0, 0);
    // sym-shift is column 7, line 1
    kbd_register_modifier(&sys->kbd, 1, 7, 1);
    // alpha-numeric keys
    const char* keymap =
        /* no shift */
        " zxcv"         // A8       shift,z,x,c,v
        "asdfg"         // A9       a,s,d,f,g
        "qwert"         // A10      q,w,e,r,t
        "12345"         // A11      1,2,3,4,5
        "09876"         // A12      0,9,8,7,6
        "poiuy"         // A13      p,o,i,u,y
        " lkjh"         // A14      enter,l,k,j,h
        "  mnb"         // A15      space,symshift,m,n,b

        // shift
        " ZXCV"         // A8
        "ASDFG"         // A9
        "QWERT"         // A10
        "     "         // A11
        "     "         // A12
        "POIUY"         // A13
        " LKJH"         // A14
        "  MNB"         // A15

        // symshift
        " : ?/"         // A8
        "     "         // A9
        "   <>"         // A10
        "!@#$%"         // A11
        "_)('&"         // A12
        "\";   "        // A13
        " =+-^"         // A14
        "  .,*";        // A15
    for (int layer = 0; layer < 3; layer++) {
        for (int column = 0; column < 8; column++) {
            for (int line = 0; line < 5; line++) {
                const uint8_t c = keymap[layer*40 + column*5 + line];
                if (c != 0x20) {
                    kbd_register_key(&sys->kbd, c, column, line, (layer>0) ? (1<<(layer-1)) : 0);
                }
            }
        }
    }

    // special keys
    kbd_register_key(&sys->kbd, ' ', 7, 0, 0);  // Space
    kbd_register_key(&sys->kbd, 0x0F, 7, 1, 0); // SymShift
    kbd_register_key(&sys->kbd, 0x08, 3, 4, 1); // Cursor Left (Shift+5)
    kbd_register_key(&sys->kbd, 0x0A, 4, 4, 1); // Cursor Down (Shift+6)
    kbd_register_key(&sys->kbd, 0x0B, 4, 3, 1); // Cursor Up (Shift+7)
    kbd_register_key(&sys->kbd, 0x09, 4, 2, 1); // Cursor Right (Shift+8)
    kbd_register_key(&sys->kbd, 0x07, 3, 0, 1); // Edit (Shift+1)
    kbd_register_key(&sys->kbd, 0x0C, 4, 0, 1); // Delete (Shift+0)
    kbd_register_key(&sys->kbd, 0x0D, 6, 0, 0); // Enter
}

/*=== FILE LOADING ===========================================================*/

// ZX Z80 file format header (http://www.worldofspectrum.org/faq/reference/z80format.htm)
typedef struct {
    uint8_t A, F;
    uint8_t C, B;
    uint8_t L, H;
    uint8_t PC_l, PC_h;
    uint8_t SP_l, SP_h;
    uint8_t I, R;
    uint8_t flags0;
    uint8_t E, D;
    uint8_t C_, B_;
    uint8_t E_, D_;
    uint8_t L_, H_;
    uint8_t A_, F_;
    uint8_t IY_l, IY_h;
    uint8_t IX_l, IX_h;
    uint8_t EI;
    uint8_t IFF2;
    uint8_t flags1;
} _zx_z80_header;

typedef struct {
    uint8_t len_l;
    uint8_t len_h;
    uint8_t PC_l, PC_h;
    uint8_t hw_mode;
    uint8_t out_7ffd;
    uint8_t rom1;
    uint8_t flags;
    uint8_t out_fffd;
    uint8_t audio[16];
    uint8_t tlow_l;
    uint8_t tlow_h;
    uint8_t spectator_flags;
    uint8_t mgt_rom_paged;
    uint8_t multiface_rom_paged;
    uint8_t rom_0000_1fff;
    uint8_t rom_2000_3fff;
    uint8_t joy_mapping[10];
    uint8_t kbd_mapping[10];
    uint8_t mgt_type;
    uint8_t disciple_button_state;
    uint8_t disciple_flags;
    uint8_t out_1ffd;
} _zx_z80_ext_header;

typedef struct {
    uint8_t len_l;
    uint8_t len_h;
    uint8_t page_nr;
} _zx_z80_page_header;

static bool _zx_overflow(const uint8_t* ptr, intptr_t num_bytes, const uint8_t* end_ptr) {
    return (ptr + num_bytes) > end_ptr;
}

bool zx_quickload(zx_t* sys, chips_range_t data) {
    CHIPS_ASSERT(data.ptr && (data.size > 0));
    uint8_t* ptr = data.ptr;
    const uint8_t* end_ptr = ptr + data.size;
    if (_zx_overflow(ptr, sizeof(_zx_z80_header), end_ptr)) {
        return false;
    }
    const _zx_z80_header* hdr = (const _zx_z80_header*) ptr;
    ptr += sizeof(_zx_z80_header);
    const _zx_z80_ext_header* ext_hdr = 0;
    uint16_t pc = (hdr->PC_h<<8 | hdr->PC_l) & 0xFFFF;
    const bool is_version1 = 0 != pc;
    if (!is_version1) {
        if (_zx_overflow(ptr, sizeof(_zx_z80_ext_header), end_ptr)) {
            return false;
        }
        ext_hdr = (_zx_z80_ext_header*) ptr;
        int ext_hdr_len = (ext_hdr->len_h<<8)|ext_hdr->len_l;
        ptr += 2 + ext_hdr_len;
        if (ext_hdr->hw_mode < 3) {
            if (sys->type != ZX_TYPE_48K) {
                return false;
            }
        }
        else {
            if (sys->type != ZX_TYPE_128) {
                return false;
            }
        }
    }
    else {
        if (sys->type != ZX_TYPE_48K) {
            return false;
        }
    }
    const bool v1_compr = 0 != (hdr->flags0 & (1<<5));
    while (ptr < end_ptr) {
        int page_index = 0;
        int src_len = 0;
        if (is_version1) {
            src_len = data.size - sizeof(_zx_z80_header);
        }
        else {
            _zx_z80_page_header* phdr = (_zx_z80_page_header*) ptr;
            if (_zx_overflow(ptr, sizeof(_zx_z80_page_header), end_ptr)) {
                return false;
            }
            ptr += sizeof(_zx_z80_page_header);
            src_len = (phdr->len_h<<8 | phdr->len_l) & 0xFFFF;
            page_index = phdr->page_nr - 3;
            if ((sys->type == ZX_TYPE_48K) && (page_index == 5)) {
                page_index = 0;
            }
            if ((page_index < 0) || (page_index > 7)) {
                page_index = -1;
            }
        }
        uint8_t* dst_ptr;
        if (-1 == page_index) {
            dst_ptr = sys->junk;
        }
        else {
            dst_ptr = sys->ram[page_index];
        }
        if (0xFFFF == src_len) {
            // FIXME: uncompressed not supported yet
            return false;
        }
        else {
            // compressed
            int src_pos = 0;
            bool v1_done = false;
            uint8_t val[4];
            while ((src_pos < src_len) && !v1_done) {
                val[0] = ptr[src_pos];
                val[1] = ptr[src_pos+1];
                val[2] = ptr[src_pos+2];
                val[3] = ptr[src_pos+3];
                // check for version 1 end marker
                if (v1_compr && (0==val[0]) && (0xED==val[1]) && (0xED==val[2]) && (0==val[3])) {
                    v1_done = true;
                    src_pos += 4;
                }
                else if (0xED == val[0]) {
                    if (0xED == val[1]) {
                        uint8_t count = val[2];
                        CHIPS_ASSERT(0 != count);
                        uint8_t data = val[3];
                        src_pos += 4;
                        for (int i = 0; i < count; i++) {
                            *dst_ptr++ = data;
                        }
                    }
                    else {
                        // single ED
                        *dst_ptr++ = val[0];
                        src_pos++;
                    }
                }
                else {
                    // any value
                    *dst_ptr++ = val[0];
                    src_pos++;
                }
            }
            CHIPS_ASSERT(src_pos == src_len);
        }
        if (0xFFFF == src_len) {
            ptr += 0x4000;
        }
        else {
            ptr += src_len;
        }
    }

    // start loaded image
    z80_reset(&sys->cpu);
    sys->cpu.a = hdr->A; sys->cpu.f = hdr->F;
    sys->cpu.b = hdr->B; sys->cpu.c = hdr->C;
    sys->cpu.d = hdr->D; sys->cpu.e = hdr->E;
    sys->cpu.h = hdr->H; sys->cpu.l = hdr->L;
    sys->cpu.ix = (hdr->IX_h<<8)|hdr->IX_l;
    sys->cpu.iy = (hdr->IY_h<<8)|hdr->IY_l;
    sys->cpu.af2 = (hdr->A_<<8)|hdr->F_;
    sys->cpu.bc2 = (hdr->B_<<8)|hdr->C_;
    sys->cpu.de2 = (hdr->D_<<8)|hdr->E_;
    sys->cpu.hl2 = (hdr->H_<<8)|hdr->L_;
    sys->cpu.sp = (hdr->SP_h<<8)|hdr->SP_l;
    sys->cpu.i = hdr->I;
    sys->cpu.r = (hdr->R & 0x7F) | ((hdr->flags0 & 1)<<7);
    sys->cpu.iff2 = (hdr->IFF2 != 0);
    sys->cpu.iff1 = (hdr->EI != 0);
    if (hdr->flags1 != 0xFF) {
        sys->cpu.im = hdr->flags1 & 3;
    }
    else {
        sys->cpu.im = 1;
    }
    if (ext_hdr) {
        sys->pins = z80_prefetch(&sys->cpu, (ext_hdr->PC_h<<8)|ext_hdr->PC_l);
        if (sys->type == ZX_TYPE_128) {
            ay38910_reset(&sys->ay);
            for (uint8_t i = 0; i < AY38910_NUM_REGISTERS; i++) {
                ay38910_set_register(&sys->ay, i, ext_hdr->audio[i]);
            }
            ay38910_set_addr_latch(&sys->ay, ext_hdr->out_fffd);
            _zx_update_memory_map_zx128(sys, ext_hdr->out_7ffd);
        }
    }
    else {
        sys->pins = z80_prefetch(&sys->cpu, (hdr->PC_h<<8)|hdr->PC_l);
    }
    sys->border_color = (hdr->flags0>>1) & 7;
    return true;
}

chips_display_info_t zx_display_info(zx_t* sys) {
    static const uint32_t palette[16] = {
        0xFF000000,     // std black
        0xFFD70000,     // std blue
        0xFF0000D7,     // std red
        0xFFD700D7,     // std magenta
        0xFF00D700,     // std green
        0xFFD7D700,     // std cyan
        0xFF00D7D7,     // std yellow
        0xFFD7D7D7,     // std white
        0xFF000000,     // bright black
        0xFFFF0000,     // bright blue
        0xFF0000FF,     // bright red
        0xFFFF00FF,     // bright magenta
        0xFF00FF00,     // bright green
        0xFFFFFF00,     // bright cyan
        0xFF00FFFF,     // bright yellow
        0xFFFFFFFF,     // bright white
    };
    const chips_display_info_t res = {
        .frame = {
            .dim = {
                .width = ZX_FRAMEBUFFER_WIDTH,
                .height = ZX_FRAMEBUFFER_HEIGHT,
            },
            .buffer = {
                .ptr = sys ? sys->fb : 0,
                .size = ZX_FRAMEBUFFER_SIZE_BYTES,
            },
            .bytes_per_pixel = 1,
        },
        .screen = {
            .x = 0,
            .y = 0,
            .width = ZX_DISPLAY_WIDTH,
            .height = ZX_DISPLAY_HEIGHT,
        },
        .palette = {
            .ptr = (void*)palette,
            .size = sizeof(palette),
        }
    };
    CHIPS_ASSERT(((sys == 0) && (res.frame.buffer.ptr == 0)) || ((sys != 0) && (res.frame.buffer.ptr != 0)));
    return res;
}

uint32_t zx_save_snapshot(zx_t* sys, zx_t* dst) {
    CHIPS_ASSERT(sys && dst);
    *dst = *sys;
    dst->debug.callback.func = 0;
    dst->debug.callback.user_data = 0;
    dst->debug.stopped = 0;
    dst->audio.callback.func = 0;
    dst->audio.callback.user_data = 0;
    dst->ay.in_cb = 0;
    dst->ay.out_cb = 0;
    dst->ay.user_data = 0;
    mem_pointers_to_offsets(&dst->mem, sys);
    return ZX_SNAPSHOT_VERSION;
}

bool zx_load_snapshot(zx_t* sys, uint32_t version, zx_t* src) {
    CHIPS_ASSERT(sys && src);
    if (version != ZX_SNAPSHOT_VERSION) {
        return false;
    }
    static zx_t im;
    im = *src;
    im.debug = sys->debug;
    im.audio.callback = sys->audio.callback;
    im.ay.in_cb = sys->ay.in_cb;
    im.ay.out_cb = sys->ay.out_cb;
    im.ay.user_data = sys->ay.user_data;
    mem_offsets_to_pointers(&im.mem, sys);
    *sys = im;
    return true;
}

#endif // CHIPS_IMPL
