#pragma once
/*#
    # clk.h

    Emulator clock helpers.

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

    ## Functions

    ~~~C
    void clk_init(clk_t* clk, uint32_t freq_hz)
    ~~~
        Initialize a clk_t instance with a frequency in Hz.

    ~~~C
    uint32_t clk_ticks_to_run(clk_t* clk, uint32_t micro_seconds)
    ~~~
        Compute the number of ticks to execute for the given number of micro-seconds.
        Usually this is called once per frame to compute the required
        number of ticks for a CPU emulator to run in realtime.

    ~~~C
    void clk_ticks_executed(clk_t* clk, uint32_t ticks)
    ~~~
        Call this function after the CPU emulator has executed ticks 
        with the actual number of ticks executed by the emulator. 
        This computes the number of 'overrun ticks' to be subtracted
        from the number of ticks to execute in the next call to
        clk_ticks_to_tun().

    ## Example

    For a Z80 system running a 2 MHz initialize a clk_t instance like
    this:

    ~~~C
        clk_t clk;
        clk_init(&clk, 2000000);
    ~~~

    Then in the per-frame callback, you can call the z80_exec() function
    like this to run the emulated system at the right speed:

    ~~~C
    double frame_time_in_seconds = ...;
    uint32_t ticks_to_run = clk_ticks_to_run(&clk, frame_time_in_seconds));
    uint32_t ticks_executed = z80_exec(&cpu, ticks_to_run);
    clk_ticks_executed(&clk, ticks_executed);
    ~~~

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

typedef struct {
    int64_t freq_hz;
    int ticks_to_run;
    int overrun_ticks;
} clk_t;

/* setup a clock instance with a frequency in Hz */
void clk_init(clk_t* clk, uint32_t freq_hz);
/* call once per frame to compute number of ticks to execute */
uint32_t clk_ticks_to_run(clk_t* clk, uint32_t micro_seconds);
/* call once per frame with actual number of executed ticks */
void clk_ticks_executed(clk_t* clk, uint32_t ticks);

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

void clk_init(clk_t* clk, uint32_t freq_hz) {
    CHIPS_ASSERT(clk && (freq_hz > 1));
    memset(clk, 0, sizeof(clk_t));
    clk->freq_hz = freq_hz;
}

uint32_t clk_ticks_to_run(clk_t* clk, uint32_t micro_seconds) {
    CHIPS_ASSERT(clk && (micro_seconds > 0));
    int ticks = (int) ((clk->freq_hz * micro_seconds) / 1000000);
    clk->ticks_to_run = ticks - clk->overrun_ticks;
    if (clk->ticks_to_run < 1) {
        clk->ticks_to_run = 1;
    }
    return clk->ticks_to_run;
}

void clk_ticks_executed(clk_t* clk, uint32_t ticks_executed) {
    if ((int)ticks_executed > clk->ticks_to_run) {
        clk->overrun_ticks = (int)ticks_executed - clk->ticks_to_run;
    }
    else {
        clk->overrun_ticks = 0;
    }
}
#endif
