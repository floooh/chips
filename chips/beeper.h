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

/* super-sampling precision */
#define BEEPER_SUPER_SAMPLES (2)

/* beeper state */
typedef struct {
    /* current on/off state */
    bool state;
    /* super-sample period in ticks */
    int period;
    /* current tick down-counter */
    int tick_counter;
    /* current super-sample counter */
    int super_sample_counter;
    /* max sample magnitude (default 1.0) */
    float mag;
    /* current super-sample-accumulation value */
    float acc;
    /* latest super-sampled audio sample value (between 0.0 and 'mag') */
    float sample;
} beeper_t;

/* initialize beeper instance */
extern void beeper_init(beeper_t* beeper, int tick_khz, int sound_hz, float magnitude);
/* set current on/off state */
static inline void beeper_write(beeper_t* beeper, bool state) {
    beeper->state = state;
}
/* toggle current state (on->off or off->on) */
static inline void beeper_toggle(beeper_t* beeper) {
    beeper->state = !beeper->state;
}
/* tick the beeper, return true if a new sample is ready */
static inline bool beeper_tick(beeper_t* b, int num_ticks) {
    b->tick_counter -= num_ticks;
    while (b->tick_counter <= 0) {
        b->tick_counter += b->period;
        if (b->state) {
            b->acc += b->mag;
        }
        if (--b->super_sample_counter == 0) {
            b->super_sample_counter = BEEPER_SUPER_SAMPLES;
            b->sample = b->acc / BEEPER_SUPER_SAMPLES;
            b->acc = 0.0f;
            return true;
        }
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

void beeper_init(beeper_t* b, int tick_khz, int sound_hz, float magnitude) {
    CHIPS_ASSERT(b);
    CHIPS_ASSERT((tick_khz > 0) && (sound_hz > 0));
    memset(b, 0, sizeof(*b));
    b->period = (tick_khz * 1000) / (sound_hz * BEEPER_SUPER_SAMPLES);
    b->tick_counter = b->period;
    b->super_sample_counter = BEEPER_SUPER_SAMPLES;
    b->mag = magnitude;
}

#endif /* CHIPS_IMPL */

#ifdef __cplusplus
} /* extern "C" */
#endif
