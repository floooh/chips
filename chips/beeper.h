#pragma once
/*
    beeper.h    -- simple square-wave beeper

    TODO: docs

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
*/
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* error-accumulation precision boost */
#define BEEPER_FIXEDPOINT_SCALE (16)
/* DC adjust buffer size */
#define BEEPER_DCADJ_BUFLEN (512)

/* beeper state */
typedef struct {
    int state;
    int period;
    int counter;
    float mag;
    float sample;
    float dcadj_sum;
    uint32_t dcadj_pos;
    float dcadj_buf[BEEPER_DCADJ_BUFLEN];
} beeper_t;

/* initialize beeper instance */
void beeper_init(beeper_t* beeper, int tick_hz, int sound_hz, float magnitude);
/* reset the beeper instance */
void beeper_reset(beeper_t* beeper);
/* set current on/off state */
static inline void beeper_set(beeper_t* beeper, bool state) {
    beeper->state = state ? 1 : 0;
}
/* toggle current state (on->off or off->on) */
static inline void beeper_toggle(beeper_t* beeper) {
    beeper->state = !beeper->state;
}
/* tick the beeper, return true if a new sample is ready */
bool beeper_tick(beeper_t* beeper);

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

void beeper_init(beeper_t* b, int tick_hz, int sound_hz, float magnitude) {
    CHIPS_ASSERT(b);
    CHIPS_ASSERT((tick_hz > 0) && (sound_hz > 0));
    memset(b, 0, sizeof(*b));
    b->period = (tick_hz * BEEPER_FIXEDPOINT_SCALE) / sound_hz;
    b->counter = b->period;
    b->mag = magnitude;
}

void beeper_reset(beeper_t* b) {
    CHIPS_ASSERT(b);
    b->state = 0;
    b->counter = b->period;
    b->sample = 0;
}

/* DC adjustment filter from StSound, this moves an "offcenter"
   signal back to the zero-line (e.g. the volume-level output
   from the chip simulation which is >0.0 gets converted to
   a +/- sample value)
*/
static float _beeper_dcadjust(beeper_t* bp, float s) {
    bp->dcadj_sum -= bp->dcadj_buf[bp->dcadj_pos];
    bp->dcadj_sum += s;
    bp->dcadj_buf[bp->dcadj_pos] = s;
    bp->dcadj_pos = (bp->dcadj_pos + 1) & (BEEPER_DCADJ_BUFLEN-1);
    return s - (bp->dcadj_sum / BEEPER_DCADJ_BUFLEN);
}

bool beeper_tick(beeper_t* bp) {
    /* generate a new sample? */
    bp->counter -= BEEPER_FIXEDPOINT_SCALE;
    if (bp->counter <= 0) {
        bp->counter += bp->period;
        bp->sample = _beeper_dcadjust(bp, (float)bp->state) * bp->mag;
        return true;
    }
    return false;
}


#endif /* CHIPS_IMPL */
