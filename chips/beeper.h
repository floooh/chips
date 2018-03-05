#pragma once
/*
    beeper.h    -- simple square-wave beeper

    TODO: docs!
*/
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* error-accumulation precision boost */
#define BEEPER_PRECISION_BOOST (16)

/* beeper state */
typedef struct {
    /* current on/off state */
    bool state;
    /* current active-state accumulator */
    uint32_t acc;
    /* sample period in ticks */
    int period;
    /* current tick down-counter */
    int counter;
    /* multiplier to get scaled sample value */
    float mul;
    /* latest computed sample */
    float sample;
} beeper_t;

/* initialize beeper instance */
extern void beeper_init(beeper_t* beeper, int tick_hz, int sound_hz, float magnitude);
/* reset the beeper instance */
extern void beeper_reset(beeper_t* beeper);
/* set current on/off state */
static inline void beeper_write(beeper_t* beeper, bool state) {
    beeper->state = state;
}
/* toggle current state (on->off or off->on) */
static inline void beeper_toggle(beeper_t* beeper) {
    beeper->state = !beeper->state;
}
/* tick the beeper, return true if a new sample is ready */
static inline bool beeper_tick(beeper_t* b) {
    if (b->state) {
        b->acc++;
    }
    b->counter -= BEEPER_PRECISION_BOOST;
    if (b->counter <= 0) {
        b->counter += b->period;
        b->sample = ((float)b->acc) * b->mul;
        b->acc = 0;
        return true;
    }
    return false;
}

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

void beeper_init(beeper_t* b, int tick_hz, int sound_hz, float magnitude) {
    CHIPS_ASSERT(b);
    CHIPS_ASSERT((tick_hz > 0) && (sound_hz > 0));
    memset(b, 0, sizeof(*b));
    b->period = (tick_hz * BEEPER_PRECISION_BOOST) / sound_hz;
    b->counter = b->period;
    b->mul = magnitude * (1.0f / ((float)tick_hz / (float)sound_hz));
}

void beeper_reset(beeper_t* b) {
    CHIPS_ASSERT(b);
    b->acc = 0;
    b->sample = 0;
    b->state = false;
    b->counter = b->period;
}

#endif /* CHIPS_IMPL */

#ifdef __cplusplus
} /* extern "C" */
#endif
