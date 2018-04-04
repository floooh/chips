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
#define BEEPER_FIXEDPOINT_SCALE (16)
/* minimal update rate in Hz before the beeper goes silent */
#ifndef BEEPER_SILENCE_HZ
#define BEEPER_SILENCE_HZ (10)
#endif

/* beeper state */
typedef struct {
    int state;    /* can be -1, +1 or 0 (if silent) */
    uint32_t silent_counter;
    uint32_t ticks_until_silence;
    int period;
    int counter;
    float mag;
    float sample;
} beeper_t;

/* initialize beeper instance */
extern void beeper_init(beeper_t* beeper, int tick_hz, int sound_hz, float magnitude);
/* reset the beeper instance */
extern void beeper_reset(beeper_t* beeper);
/* set current on/off state */
static inline void beeper_set(beeper_t* beeper, bool state) {
    int new_state = state ? 1 : -1;
    if (beeper->state != new_state) {
        beeper->silent_counter = 0;
        beeper->state = new_state;
    }
}
/* toggle current state (on->off or off->on) */
static inline void beeper_toggle(beeper_t* beeper) {
    if (beeper->state == 0) {
        beeper->state = 1;
    }
    else {
        beeper->state = -beeper->state;
    }
    beeper->silent_counter = 0;
}
/* tick the beeper, return true if a new sample is ready */
static inline bool beeper_tick(beeper_t* beeper) {
    if (beeper->state != 0) {
        if (beeper->silent_counter++ > beeper->ticks_until_silence) {
            beeper->state = 0;
        }
    }
    /* generate a new sample? */
    beeper->counter -= BEEPER_FIXEDPOINT_SCALE;
    if (beeper->counter <= 0) {
        beeper->counter += beeper->period;
        float goal = ((float)beeper->state) * beeper->mag;
        beeper->sample += (goal - beeper->sample) * 0.5f;
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
    b->period = (tick_hz * BEEPER_FIXEDPOINT_SCALE) / sound_hz;
    b->counter = b->period;
    b->mag = magnitude;
    b->ticks_until_silence = tick_hz / BEEPER_SILENCE_HZ;
}

void beeper_reset(beeper_t* b) {
    CHIPS_ASSERT(b);
    b->state = 0;
    b->silent_counter = 0;
    b->counter = b->period;
    b->sample = 0;
}

#endif /* CHIPS_IMPL */

#ifdef __cplusplus
} /* extern "C" */
#endif
