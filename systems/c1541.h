#pragma once
/*#
    # c1551.h

    A Commodore 1541 floppy drive emulation.

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

    - chips/chips_common.h
    - chips/m6502.h
    - chips/m6522.h
    - chips/mem.h

    ## zlib/libpng license

    Copyright (c) 2019 Andre Weissflog
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

#ifdef __cplusplus
extern "C" {
#endif

// IEC port bits, same as C64_IECPORT_*
#define C1541_IECPORT_RESET (1<<0)
#define C1541_IECPORT_SRQIN (1<<1)
#define C1541_IECPORT_DATA  (1<<2)
#define C1541_IECPORT_CLK   (1<<3)
#define C1541_IECPORT_ATN   (1<<4)

#define C1541_FREQUENCY (1000000)

// config params for c1541_init()
typedef struct {
    // pointer to a shared byte with IEC serial bus line state
    uint8_t* iec_port;
    // rom images
    struct {
        chips_range_t c000_dfff;
        chips_range_t e000_ffff;
    } roms;
} c1541_desc_t;

// 1541 emulator state
typedef struct {
    uint64_t pins;
    uint8_t* iec;
    m6502_t cpu;
    m6522_t via_1;
    m6522_t via_2;
    bool valid;
    mem_t mem;
    uint8_t ram[0x0800];
    uint8_t rom[0x4000];
} c1541_t;

// initialize a new c1541_t instance
void c1541_init(c1541_t* sys, const c1541_desc_t* desc);
// discard a c1541_t instance
void c1541_discard(c1541_t* sys);
// reset a c1541_t instance
void c1541_reset(c1541_t* sys);
// tick a c1541_t instance forward
void c1541_tick(c1541_t* sys);
// insert a disc image file (.d64)
void c1541_insert_disc(c1541_t* sys, chips_range_t data);
// remove current disc
void c1541_remove_disc(c1541_t* sys);
// prepare a c1541_t snapshot for saving
void c1541_snapshot_onsave(c1541_t* snapshot, void* base);
// prepare a c1541_t snapshot for loading
void c1541_snapshot_onload(c1541_t* snapshot, c1541_t* sys, void* base);

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

void c1541_init(c1541_t* sys, const c1541_desc_t* desc) {
    CHIPS_ASSERT(sys && desc);

    memset(sys, 0, sizeof(c1541_t));
    sys->valid = true;

    // copy ROM images
    CHIPS_ASSERT(desc->roms.c000_dfff.ptr && (0x2000 == desc->roms.c000_dfff.size));
    CHIPS_ASSERT(desc->roms.e000_ffff.ptr && (0x2000 == desc->roms.e000_ffff.size));
    memcpy(&sys->rom[0x0000], desc->roms.c000_dfff.ptr, 0x2000);
    memcpy(&sys->rom[0x2000], desc->roms.e000_ffff.ptr, 0x2000);

    // initialize the hardware
    m6502_desc_t cpu_desc;
    memset(&cpu_desc, 0, sizeof(cpu_desc));
    sys->pins = m6502_init(&sys->cpu, &cpu_desc);
    m6522_init(&sys->via_1);
    m6522_init(&sys->via_2);

    // setup memory map
    mem_init(&sys->mem);
    mem_map_ram(&sys->mem, 0, 0x0000, 0x0800, sys->ram);
    mem_map_rom(&sys->mem, 0, 0xC000, 0x4000, sys->rom);
}

void c1541_discard(c1541_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    sys->valid = false;
}

void c1541_reset(c1541_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    sys->pins |= M6502_RES;
    m6522_reset(&sys->via_1);
    m6522_reset(&sys->via_2);
}

void c1541_tick(c1541_t* sys) {
    uint64_t pins = sys->pins;

    pins = m6502_tick(&sys->cpu, pins);
    const uint16_t addr = M6502_GET_ADDR(pins);

    if (pins & M6502_RW) {
        M6502_SET_DATA(pins, mem_rd(&sys->mem, addr));
    }
    else {
        mem_wr(&sys->mem, addr, M6502_GET_DATA(pins));
    }

    sys->pins = pins;
}

void c1541_insert_disc(c1541_t* sys, chips_range_t data) {
    // FIXME
    (void)sys;
    (void)data;
}

void c1541_remove_disc(c1541_t* sys) {
    // FIXME
    (void)sys;
}

void c1541_snapshot_onsave(c1541_t* snapshot, void* base) {
    CHIPS_ASSERT(snapshot && base);
    snapshot->iec = 0;
    m6502_snapshot_onsave(&snapshot->cpu);
    mem_snapshot_onsave(&snapshot->mem, base);
}

void c1541_snapshot_onload(c1541_t* snapshot, c1541_t* sys, void* base) {
    CHIPS_ASSERT(snapshot && sys && base);
    snapshot->iec = sys->iec;
    m6502_snapshot_onload(&snapshot->cpu, &sys->cpu);
    mem_snapshot_onload(&snapshot->mem, base);
}

#endif // CHIPS_IMPL
