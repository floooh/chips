#pragma once
/*#
    # clk.h

    Emulator clock helpers.

    Do this:
    ~~~C
    #define CHIPS_IMPL
    ~~~
    before you include this file in *one* C file to create the
    implementation.

    Optionally provide the following macros with your own implementation
    
    ~~~C
    CHIPS_ASSERT(c)
    ~~~
        your own assert macro (default: assert(c))

    ## Functions

    ~~~C
    uint32_t clk_us_to_ticks(uint64_t freq_hz, uint32_t micro_seconds)
    ~~~
        Convert micro-seconds to system ticks.

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

#ifdef __cplusplus
extern "C" {
#endif

// helper func to convert micro_seconds into ticks
uint32_t clk_us_to_ticks(uint64_t freq_hz, uint32_t micro_seconds);

#ifdef __cplusplus
} /* extern "C" */
#endif

/*-- IMPLEMENTATION ----------------------------------------------------------*/
#ifdef CHIPS_IMPL
#ifndef CHIPS_ASSERT
    #include <assert.h>
    #define CHIPS_ASSERT(c) assert(c)
#endif

uint32_t clk_us_to_ticks(uint64_t freq_hz, uint32_t micro_seconds) {
    return (uint32_t) ((freq_hz * micro_seconds) / 1000000);
}
#endif
