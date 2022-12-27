#pragma once
/*#
    # z1013.h

    A Robotron Z1013 emulator in a C header.

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

    - chips/chips_common.h
    - chips/z80.h
    - chips/z80pio.h
    - chips/mem.h
    - chips/kbd.h
    - chips/clk.h

    ## The Robotron Z1013

    The Z1013 was a very simple East German home computer, basically
    just a Z80 CPU connected to some memory, and a PIO connected to
    a keyboard matrix. It's easy to emulate because the system didn't
    use any interrupts, and only simple PIO IN/OUT is required to
    scan the keyboard matrix.

    It had a 32x32 monochrome ASCII framebuffer starting at EC00,
    and a 2 KByte operating system ROM starting at F000.

    No cassette-tape / beeper sound emulated!

    ## TODO: add hardware/software reference links

    ## TODO: Describe Usage


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

// bump this whenever the z1013_t struct layout changes
#define Z1013_SNAPSHOT_VERSION (0x0001)

#define Z1013_FRAMEBUFFER_WIDTH (256)
#define Z1013_FRAMEBUFFER_HEIGHT (256)
#define Z1013_FRAMEBUFFER_SIZE_BYTES (Z1013_FRAMEBUFFER_WIDTH * Z1013_FRAMEBUFFER_HEIGHT)
#define Z1013_DISPLAY_WIDTH (256)
#define Z1013_DISPLAY_HEIGHT (256)

// Z1013 model types
typedef enum {
    Z1013_TYPE_64,      // Z1013.64 (default, latest model with 2 MHz and 64 KB RAM, new ROM)
    Z1013_TYPE_16,      // Z1013.16 (2 MHz model with 16 KB RAM, new ROM)
    Z1013_TYPE_01,      // Z1013.01 (original model, 1 MHz, 16 KB RAM)
} z1013_type_t;

// configuration parameters for z1013_setup()
typedef struct {
    z1013_type_t type;          // default is Z1013_TYPE_64
    chips_debug_t debug;        // optional debug callback and userdata ptr

    // ROM images
    struct {
        chips_range_t mon202;
        chips_range_t mon_a2;
        chips_range_t font;
    } roms;
} z1013_desc_t;

// Z1013 emulator state
typedef struct {
    z80_t cpu;
    mem_t mem;
    z80pio_t pio;
    chips_debug_t debug;
    uint64_t pins;
    z1013_type_t type;
    bool valid;
    uint16_t kbd_request_line_mask;
    int kbd_request_line_hilo_shift;
    kbd_t kbd;
    uint64_t freq_hz;
    uint8_t ram[1<<16];
    uint8_t rom_os[2048];
    uint8_t rom_font[2048];
    alignas(64) uint8_t fb[Z1013_FRAMEBUFFER_SIZE_BYTES];
} z1013_t;

// initialize a new Z1013 instance
void z1013_init(z1013_t* sys, const z1013_desc_t* desc);
// discard a z1013 instance
void z1013_discard(z1013_t* sys);
// reset Z1013 instance
void z1013_reset(z1013_t* sys);
// query information about display requirements, can be called with nullptr
chips_display_info_t z1013_display_info(z1013_t* sys);
// run the Z1013 instance for a given number of microseconds, returns number of executed ticks
uint32_t z1013_exec(z1013_t* sys, uint32_t micro_seconds);
// send a key-down event
void z1013_key_down(z1013_t* sys, int key_code);
// send a key-up event
void z1013_key_up(z1013_t* sys, int key_code);
// load a "KC .z80" file into the emulator
bool z1013_quickload(z1013_t* sys, chips_range_t data);
// take snapshot, patches any pointers to zero, returns a snapshot version
uint32_t z1013_save_snapshot(z1013_t* sys, z1013_t* dst);
// load a snapshot, returns false if snapshot version doesn't match
bool z1013_load_snapshot(z1013_t* sys, uint32_t version, const z1013_t* src);

#ifdef __cplusplus
} // extern "C"
#endif

//-- IMPLEMENTATION ------------------------------------------------------------
#ifdef CHIPS_IMPL
#include <string.h>
#ifndef CHIPS_ASSERT
#include <assert.h>
#define CHIPS_ASSERT(c) assert(c)
#endif

/*
    IO address decoding.

    The PIO Chip-Enable pin (Z80PIO_CE) is connected to output pin 0 of
    a MH7442 BCD-to-Decimal decoder (looks like this is a Czech
    clone of a SN7442). The lower 3 input pins of the MH7442
    are connected to address bus pins A2..A4, and the 4th input
    pin is connected to IORQ. This means the PIO is enabled when
    the CPU IORQ pin is low (active), and address bus bits 2..4 are
    off. This puts the PIO at the lowest 4 addresses of an 32-entry
    address space (so the PIO should be visible at port number
    0..4, but also at 32..35, 64..67 and so on).

    The PIO Control/Data select pin (Z80PIO_CDSEL) is connected to
    address bus pin A0. This means even addresses select a PIO data
    operation, and odd addresses select a PIO control operation.

    The PIO port A/B select pin (Z80PIO_BASEL) is connected to address
    bus pin A1. This means the lower 2 port numbers address the PIO
    port A, and the upper 2 port numbers address the PIO port B.

    The keyboard matrix columns are connected to another MH7442
    BCD-to-Decimal converter, this converts a hardware latch at port
    address 8 which stores a keyboard matrix column number from the CPU
    to the column lines. The operating system scans the keyboard by
    writing the numbers 0..7 to this latch, which is then converted
    by the MH7442 to light up the keyboard matrix column lines
    in that order. Next the CPU reads back the keyboard matrix lines
    in 2 steps of 4 bits each from PIO port B.
*/
#define _Z1013_PIO_SEL_MASK (Z80_IORQ|Z80_A4|Z80_A3|Z80_A2)
#define _Z1013_PIO_SEL_PINS (Z80_IORQ)
#define _Z1013_PORT8_SEL_MASK (Z80_IORQ|Z80_WR|Z80_A4|Z80_A3|Z80_A2)
#define _Z1013_PORT8_SEL_PINS (Z80_IORQ|Z80_WR|Z80_A3)

void z1013_init(z1013_t* sys, const z1013_desc_t* desc) {
    CHIPS_ASSERT(sys && desc);
    if (desc->debug.callback.func) { CHIPS_ASSERT(desc->debug.stopped); }

    memset(sys, 0, sizeof(z1013_t));
    sys->type = desc->type;
    sys->valid = true;
    sys->freq_hz = (Z1013_TYPE_01 == desc->type) ? 1000000 : 2000000;
    sys->debug = desc->debug;

    // copy ROM dumps
    CHIPS_ASSERT(desc->roms.font.ptr && (desc->roms.font.size == sizeof(sys->rom_font)));
    memcpy(sys->rom_font, desc->roms.font.ptr, sizeof(sys->rom_font));
    if (desc->type == Z1013_TYPE_01) {
        CHIPS_ASSERT(desc->roms.mon202.ptr && (desc->roms.mon202.size == sizeof(sys->rom_os)));
        memcpy(sys->rom_os, desc->roms.mon202.ptr, sizeof(sys->rom_os));
    }
    else {
        CHIPS_ASSERT(desc->roms.mon_a2.ptr && (desc->roms.mon_a2.size == sizeof(sys->rom_os)));
        memcpy(sys->rom_os, desc->roms.mon_a2.ptr, sizeof(sys->rom_os));
    }

    // initialize the hardware
    z80_init(&sys->cpu);
    z80pio_init(&sys->pio);

    /* setup the memory map:
        - the Z1013.64 has 64 KByte RAM, all other models 16 KByte RAM
        - there's a 1 KByte character frame buffer at 0xEC00
        - and finally 2 KByte ROM starting at 0xF000
        - the memory map is fixed
    */
    mem_init(&sys->mem);
    if (Z1013_TYPE_64 == sys->type) {
        mem_map_ram(&sys->mem, 1, 0x0000, 0x10000, sys->ram);
    }
    else {
        mem_map_ram(&sys->mem, 1, 0x0000, 0x4000, sys->ram);
        mem_map_ram(&sys->mem, 1, 0xEC00, 0x0400, &(sys->ram[0xEC00]));
    }
    mem_map_rom(&sys->mem, 0, 0xF000, 0x0800, sys->rom_os);

    /* Setup the keyboard matrix, the original Z1013.01 has a 8x4 matrix with
       4 shift keys, later models also support a more traditional 8x8 matrix.
    */
    // keep key presses sticky for 2 frames
    kbd_init(&sys->kbd, 2);
    if (Z1013_TYPE_01 == sys->type) {
        // 8x4 keyboard matrix
        kbd_register_modifier(&sys->kbd, 0, 0, 3);
        kbd_register_modifier(&sys->kbd, 1, 1, 3);
        kbd_register_modifier(&sys->kbd, 2, 2, 3);
        kbd_register_modifier(&sys->kbd, 3, 3, 3);
        const char* keymap =
            // no shift
            "@ABCDEFG"  "HIJKLMNO"  "PQRSTUVW"  "        "
            // shift 1
            "XYZ[\\]^-" "01234567"  "89:;<=>?"  "        "
            // shift 2
            "   {|}~ "  " !\"#$%&'" "()*+,-./"  "        "
            // shift 3
            " abcdefg"  "hijklmno"  "pqrstuvw"  "        "
            // shift 4
            "xyz     "  "        "  "        "  "        ";
        for (int layer = 0; layer < 5; layer++) {
            for (int line = 0; line < 4; line++) {
                for (int column = 0; column < 8; column++) {
                    int c = keymap[layer*32 + line*8 + column];
                    if (c != 0x20) {
                        kbd_register_key(&sys->kbd, c, column, line, layer ? (1<<(layer-1)) : 0);
                    }
                }
            }
        }
        // special keys
        kbd_register_key(&sys->kbd, ' ', 5, 3, 0);   // Space
        kbd_register_key(&sys->kbd, 0x08, 4, 3, 0);  // Cursor Left
        kbd_register_key(&sys->kbd, 0x09, 6, 3, 0);  // Cursor Right
        kbd_register_key(&sys->kbd, 0x0D, 7, 3, 0);  // Enter
        kbd_register_key(&sys->kbd, 0x03, 3, 1, 4);  // Break/Escape
    }
    else {
        // 8x8 keyboard matrix (http://www.z1013.de/images/21.gif)
        // shift key modifier is column 7 line 6
        const int shift = 0, shift_mask = (1<<shift);
        kbd_register_modifier(&sys->kbd, shift, 7, 6);
        // ctrl key modifier is column 6 line 5
        const int ctrl = 1, ctrl_mask = (1<<ctrl);
        kbd_register_modifier(&sys->kbd, ctrl, 6, 5);
        const char* keymap =
            // no shift
            "13579-  QETUO@  ADGJL*  YCBM.^  24680[  WRZIP]  SFHK+\\  XVN,/_  "
            // shift
            "!#%')=  qetuo`  adgjl:  ycbm>~  \"$&( {  wrzip}  sfhk;|  xvn<?   ";
        for (int layer = 0; layer < 2; layer++) {
            for (int line = 0; line < 8; line++) {
                for (int column = 0; column < 8; column++) {
                    int c = keymap[layer*64 + line*8 + column];
                    if (c != 0x20) {
                        kbd_register_key(&sys->kbd, c, column, line, layer ? shift_mask : 0);
                    }
                }
            }
        }
        // special keys
        kbd_register_key(&sys->kbd, ' ',  6, 4, 0);  // space
        kbd_register_key(&sys->kbd, 0x08, 6, 2, 0);  // cursor left
        kbd_register_key(&sys->kbd, 0x09, 6, 3, 0);  // cursor right
        kbd_register_key(&sys->kbd, 0x0A, 6, 7, 0);  // cursor down
        kbd_register_key(&sys->kbd, 0x0B, 6, 6, 0);  // cursor up
        kbd_register_key(&sys->kbd, 0x0D, 6, 1, 0);  // enter
        kbd_register_key(&sys->kbd, 0x03, 1, 3, ctrl_mask); // map Esc to Ctrl+C (STOP/BREAK)
    }

    // execution starts at 0xF000
    sys->pins = z80_prefetch(&sys->cpu, 0xF000);
}

void z1013_discard(z1013_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    sys->valid = false;
}

void z1013_reset(z1013_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    z80_reset(&sys->cpu);
    z80pio_reset(&sys->pio);
    sys->kbd_request_line_mask = 0;
    sys->kbd_request_line_hilo_shift = 0;
    sys->pins = z80_prefetch(&sys->cpu, 0xF000);
}

static uint64_t _z1013_tick(z1013_t* sys, uint64_t pins) {
    pins = z80_tick(&sys->cpu, pins) & Z80_PIN_MASK;

    // handle memory requests
    if (pins & Z80_MREQ) {
        const uint16_t addr = Z80_GET_ADDR(pins);
        if (pins & Z80_RD) {
            Z80_SET_DATA(pins, mem_rd(&sys->mem, addr));
        }
        else if (pins & Z80_WR) {
            mem_wr(&sys->mem, addr, Z80_GET_DATA(pins));
        }
    }

    // tick the PIO, no interrupts on the Z1013
    {
        if ((pins & _Z1013_PIO_SEL_MASK) == _Z1013_PIO_SEL_PINS) {
            pins |= Z80PIO_CE;
        }
        if (pins & Z80_A0) { pins |= Z80PIO_CDSEL; }
        if (pins & Z80_A1) { pins |= Z80PIO_BASEL; }
        uint8_t pb = sys->kbd_request_line_mask >> sys->kbd_request_line_hilo_shift;
        Z80PIO_SET_PAB(pins, 0xFF, pb);
        pins = z80pio_tick(&sys->pio, pins);
        // bit 4 for 8x8 keyboard selects upper or lower 4 kbd matrix line bits
        if (Z1013_TYPE_01 != sys->type) {
            // kbd_request_line_hilo_shift will be 0 or 4
            sys->kbd_request_line_hilo_shift = (Z80PIO_GET_PB(pins) & (1<<4)) >> 2;
        }
        // bit 7 is cassette output, not emulated
        pins &= Z80_PIN_MASK;
    }

    // 8-bit write-only latch at port address 8 to store the requested
    // keyboard column for the next keyboard scan
    if ((pins & _Z1013_PORT8_SEL_MASK) == _Z1013_PORT8_SEL_PINS) {
        uint8_t column_mask = 1 << (Z80_GET_DATA(pins) & 7);
        sys->kbd_request_line_mask = ~kbd_test_lines(&sys->kbd, column_mask);
    }

    return pins & Z80_PIN_MASK;
}

/* since the Z1013 didn't have any sort of programmable video output,
    we're cheating a bit and decode the entire frame in one go
*/
static void _z1013_decode_vidmem(z1013_t* sys) {
    static const uint32_t lut32[16] = {
        0x00000000, 0x01000000, 0x00010000, 0x01010000,
        0x00000100, 0x01000100, 0x00010100, 0x01010100,
        0x00000001, 0x01000001, 0x00010001, 0x01010001,
        0x00000101, 0x01000101, 0x00010101, 0x01010101,
    };
    uint32_t* dst32 = (uint32_t*) sys->fb;
    const uint8_t* src = &sys->ram[0xEC00];   // the 32x32 framebuffer starts at EC00
    const uint8_t* font = sys->rom_font;
    for (size_t y = 0; y < 32; y++) {
        for (size_t py = 0; py < 8; py++) {
            for (size_t x = 0; x < 32; x++) {
                uint8_t chr = src[(y<<5) + x];
                uint8_t pixels = font[(chr<<3)|py];
                *dst32++ = lut32[pixels >> 4];
                *dst32++ = lut32[pixels & 0xF];
            }
        }
    }
}

uint32_t z1013_exec(z1013_t* sys, uint32_t micro_seconds) {
    CHIPS_ASSERT(sys && sys->valid);
    const uint32_t num_ticks = clk_us_to_ticks(sys->freq_hz, micro_seconds);
    uint64_t pins = sys->pins;
    if (0 == sys->debug.callback.func) {
        // run without debug hook
        for (uint32_t ticks = 0; ticks < num_ticks; ticks++) {
            pins = _z1013_tick(sys, pins);
        }
    }
    else {
        // run with debug hook
        for (uint32_t ticks = 0; (ticks < num_ticks) && !(*sys->debug.stopped); ticks++) {
            pins = _z1013_tick(sys, pins);
            sys->debug.callback.func(sys->debug.callback.user_data, pins);
        }
    }
    sys->pins = pins;
    kbd_update(&sys->kbd, micro_seconds);
    _z1013_decode_vidmem(sys);
    return num_ticks;
}

void z1013_key_down(z1013_t* sys, int key_code) {
    CHIPS_ASSERT(sys && sys->valid);
    kbd_key_down(&sys->kbd, key_code);
}

void z1013_key_up(z1013_t* sys, int key_code) {
    CHIPS_ASSERT(sys && sys->valid);
    kbd_key_up(&sys->kbd, key_code);
}

typedef struct {
    uint8_t load_addr_l;
    uint8_t load_addr_h;
    uint8_t end_addr_l;
    uint8_t end_addr_h;
    uint8_t exec_addr_l;
    uint8_t exec_addr_h;
    uint8_t free[6];
    uint8_t typ;
    uint8_t d3[3];        /* d3 d3 d3 */
    uint8_t name[16];
} _z1013_kcz80_header;

bool z1013_quickload(z1013_t* sys, chips_range_t data) {
    CHIPS_ASSERT(sys && sys->valid && data.ptr);
    if (data.size < sizeof(_z1013_kcz80_header)) {
        return false;
    }
    const uint8_t* ptr = (uint8_t*)data.ptr;
    const _z1013_kcz80_header* hdr = (const _z1013_kcz80_header*)ptr;
    if ((hdr->d3[0] != 0xD3) || (hdr->d3[1] != 0xD3) || (hdr->d3[2] != 0xD3)) {
        return false;
    }
    ptr += sizeof(_z1013_kcz80_header);
    uint16_t exec_addr = 0;
    int addr = (hdr->load_addr_h<<8 | hdr->load_addr_l) & 0xFFFF;
    int end_addr = (hdr->end_addr_h<<8 | hdr->end_addr_l) & 0xFFFF;
    if (end_addr <= addr) {
        return false;
    }
    exec_addr = (hdr->exec_addr_h<<8 | hdr->exec_addr_l) & 0xFFFF;
    mem_write_range(&sys->mem, addr, ptr, end_addr - addr);

    z80_reset(&sys->cpu);
    sys->cpu.a = 0x00;
    sys->cpu.f = 0x10;
    sys->cpu.bc = 0x0000; sys->cpu.bc2 = 0x0000;
    sys->cpu.de = 0x0000; sys->cpu.de2 = 0x0000;
    sys->cpu.hl = 0x0000; sys->cpu.hl2 = 0x0000;
    sys->cpu.af2 = 0x0000;
    z80_prefetch(&sys->cpu, exec_addr);
    return true;
}

chips_display_info_t z1013_display_info(z1013_t* sys) {
    static const uint32_t palette[2] = {
        0xFF000000, // black
        0xFFFFFFFF, // white
    };
    const chips_display_info_t res = {
        .frame = {
            .dim = {
                .width = Z1013_FRAMEBUFFER_WIDTH,
                .height = Z1013_FRAMEBUFFER_HEIGHT,
            },
            .buffer = {
                .ptr = sys ? sys->fb : 0,
                .size = Z1013_FRAMEBUFFER_SIZE_BYTES,
            },
            .bytes_per_pixel = 1,
        },
        .screen = {
            .x = 0,
            .y = 0,
            .width = Z1013_DISPLAY_WIDTH,
            .height = Z1013_DISPLAY_HEIGHT,
        },
        .palette = {
            .ptr = (void*)palette,
            .size = sizeof(palette)
        }
    };
    CHIPS_ASSERT(((sys == 0) && (res.frame.buffer.ptr == 0)) || ((sys != 0) && (res.frame.buffer.ptr != 0)));
    return res;
}

uint32_t z1013_save_snapshot(z1013_t* sys, z1013_t* dst) {
    CHIPS_ASSERT(sys && dst);
    *dst = *sys;
    // patch any external pointers to zero and replace internal
    // pointers with offsets
    dst->debug.callback.func = 0;
    dst->debug.callback.user_data = 0;
    dst->debug.stopped = 0;
    mem_pointers_to_offsets(&dst->mem, sys);
    return Z1013_SNAPSHOT_VERSION;
}

bool z1013_load_snapshot(z1013_t* sys, uint32_t version, const z1013_t* src) {
    CHIPS_ASSERT(sys && src);
    if (version != Z1013_SNAPSHOT_VERSION) {
        return false;
    }
    // intermediate copy
    static z1013_t im;
    im = *src;
    im.debug = sys->debug;
    mem_offsets_to_pointers(&im.mem, sys);
    *sys = im;
    return true;
}

#endif // CHIPS_IMPL
