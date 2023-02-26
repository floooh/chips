#pragma once
/*#
    # nes.h

    A NES emulator in a C header

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

    You need to include the following headers before including nes.h:

    - chips/chips_common.h
    - chips/m6502.h
    - chips/r2c02.h

    ## zlib/libpng license

    Copyright (c) 2023 Scemino
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

// NES emulator state
typedef struct {   
    m6502_t cpu; 
    bool valid;

    alignas(64) uint8_t fb[PPU_FRAMEBUFFER_SIZE_BYTES];
} nes_t;

// initialize a new NES instance
void nes_init(nes_t* nes);
// discard a NES instance
void nes_discard(nes_t* nes);
// get display requirements and framebuffer content, may be called with nullptr
chips_display_info_t nes_display_info(nes_t* nes);

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

void nes_init(nes_t* sys) {
    CHIPS_ASSERT(sys);
    memset(sys, 0, sizeof(nes_t));
    sys->valid = true;
}

void nes_discard(nes_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    sys->valid = false;
}

chips_display_info_t nes_display_info(nes_t* sys) {
    const chips_display_info_t res = {
        .frame = {
            .dim = {
                .width = PPU_DISPLAY_WIDTH,
                .height = PPU_DISPLAY_HEIGHT,
            },
            .bytes_per_pixel = 1,
            .buffer = {
                .ptr = sys ? sys->fb : 0,
                .size = PPU_FRAMEBUFFER_SIZE_BYTES,
            }
        },
        .screen = {
            .x = 0,
            .y = 0,
            .width = PPU_DISPLAY_WIDTH,
            .height = PPU_DISPLAY_HEIGHT,
        },
        .palette = {
            .ptr = (void*)ppu_palette,
            .size = sizeof(ppu_palette)
        }
    };
    return res;
}

#endif /* CHIPS_IMPL */
