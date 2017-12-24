#pragma once
/*
    clk.h -- clock, timer and counter helpers
*/
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* convert microseconds to number of clock ticks */
extern int64_t clk_ticks(int freq_hz, int micro_secs);

/*-- IMPLEMENTATION ----------------------------------------------------------*/
#ifdef CHIPS_IMPL
#include <string.h>
#ifndef CHIPS_DEBUG
    #ifdef _DEBUG
        #define CHIPS_DEBUG
    #endif
#endif
#ifndef CHIPS_ASSERT
    #include <assert.h>
    #define CHIPS_ASSERT(c) assert(c)
#endif

int64_t clk_ticks(int freq_hz, int micro_secs) {
    return ((int64_t)freq_hz * micro_secs) / 1000000;
}
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif
