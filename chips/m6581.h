#pragma once
/*#
    # m6581.h

    MOS Technology 6581 emulator (aka SID)

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

    ## Emulated Pins

    ***********************************
    *           +-----------+         *
    *    CS --->|           |<--- A0  *
    *    RW --->|           |...      *
    *           |           |<--- A4  *
    *           |   m6581   |         *
    *           |           |<--> D0  *
    *           |           |...      *
    *           |           |<--> D7  *
    *           |           |         *
    *           +-----------+         *§
    ***********************************

    TODO: Documentation

    ## MIT License

    Copyright (c) 2018 Andre Weissflog

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
#*/
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* address bus pins A0..A4 */
#define M6581_A0    (1ULL<<0)
#define M6581_A1    (1ULL<<1)
#define M6581_A2    (1ULL<<2)
#define M6581_A3    (1ULL<<3)
#define M6581_A4    (1ULL<<4)
#define M6581_ADDR_MASK (0x1F)

/* data bus pins D0..D7 */
#define M6581_D0    (1ULL<<16)
#define M6581_D1    (1ULL<<17)
#define M6581_D2    (1ULL<<18)
#define M6581_D3    (1ULL<<19)
#define M6581_D4    (1ULL<<20)
#define M6581_D5    (1ULL<<21)
#define M6581_D6    (1ULL<<22)
#define M6581_D7    (1ULL<<23)

/* shared control pins */
#define M6581_RW    (1ULL<<24)      /* same as M6502_RW */

/* chip-specific pins */
#define M6581_CS    (1ULL<<40)      /* chip-select */

/* registers */
#define M6581_V1_FREQ_LO    (0)
#define M6581_V1_FREQ_HI    (1)
#define M6581_V1_PW_LO      (2)
#define M6581_V1_PW_HI      (3)
#define M6581_V1_CTRL       (4)
#define M6581_V1_ATKDEC     (5)
#define M6581_V1_SUSREL     (6)
#define M6581_V2_FREQ_LO    (7)
#define M6581_V2_FREQ_HI    (8)
#define M6581_V2_PW_LO      (9)
#define M6581_V2_PW_HI      (10)
#define M6581_V2_CTRL       (11)
#define M6581_V2_ATKDEC     (12)
#define M6581_V2_SUSREL     (13)
#define M6581_V3_FREQ_LO    (14)
#define M6581_V3_FREQ_HI    (15)
#define M6581_V3_PW_LO      (16)
#define M6581_V3_PW_HI      (17)
#define M6581_V3_CTRL       (18)
#define M6581_V3_ATKDEC     (19)
#define M6581_V3_SUSREL     (20)
#define M6581_FC_LO         (21)
#define M6581_FC_HI         (22)
#define M6581_RES_FILT      (23)
#define M6581_MODE_VOL      (24)
#define M6581_POT_X         (25)
#define M6581_POT_Y         (26)
#define M6581_OSC3RAND      (27)
#define M6581_ENV3          (28)
#define M6581_INV_0         (29)
#define M6581_INV_1         (30)
#define M6581_INV_2         (31)
#define M6581_NUM_REGS (32)

/* voice control bits */
#define M6581_CTRL_GATE     (1<<0)
#define M6581_CTRL_SYNC     (1<<1)
#define M6581_CTRL_RINGMOD  (1<<2)
#define M6581_CTRL_TEST     (1<<3)
#define M6581_CTRL_TRIANGLE (1<<4)
#define M6581_CTRL_SAWTOOTH (1<<5)
#define M6581_CTRL_PULSE    (1<<6)
#define M6581_CTRL_NOISE    (1<<7)

/* filter routing bits */
#define M6581_FILTER_FILT1  (1<<0)
#define M6581_FILTER_FILT2  (1<<1)
#define M6581_FILTER_FILT3  (1<<2)
#define M6581_FILTER_FILTEX (1<<3)

/* filter mode bits */
#define M6581_FILTER_LOWPASS    (1<<0)
#define M6581_FILTER_BANDPASS   (1<<1)
#define M6581_FILTER_HIGHPASS   (1<<2)
#define M6581_FILTER_3OFF       (1<<3)

/* setup parameters for m6581_init() */
typedef struct {
    int tick_hz;        /* frequency at which m6581_tick() will be called in Hz */
    int sound_hz;       /* sound sample frequency */
    float magnitude;    /* output sample magnitude (0=silence to 1=max volume) */
} m6581_desc_t;

/* envelope generator state */
typedef enum {
    M6581_ENV_ATTACK,
    M6581_ENV_DECAY_SUSTAIN,
    M6581_ENV_RELEASE
} _m6581_env_state_t;

/* voice state */
typedef struct {
    uint16_t freq;
    uint16_t pulse_width;
    uint8_t ctrl;
    uint8_t attack;
    uint8_t decay;
    uint8_t sustain;
    uint8_t release;
    bool sync;
    uint32_t noise_shift;           /* 24 bit */
    uint32_t wav_accum;             /* 24 bit */
    uint16_t wav_output;            /* 12 bit */
    _m6581_env_state_t env_state;
    uint16_t env_rate_count;
    uint16_t env_rate_period;
    uint16_t env_exp_count;
    uint8_t env_count;
    bool env_hold;
} m6581_voice_t;

/* filter state */
typedef struct {
    uint16_t cutoff;
    uint8_t resonance;
    uint8_t filter;
    uint8_t mode;
    uint8_t volume;
} m6581_filter_t;

/* m6581 instance state */
typedef struct {
    /* reading a write-only register returns the last value
       written to *any* register for about 0x2000 ticks
    */
    uint8_t bus_value;
    uint16_t bus_decay;
    /* voice state */
    m6581_voice_t voice[3];
    /* filter state */
    m6581_filter_t filter;
    /* sample generation state */
    int sample_period;
    int sample_counter;
    float mag;
    float acc;
    float sample;
} m6581_t;

/* initialize a new m6581_t instance */
extern void m6581_init(m6581_t* sid, m6581_desc_t* desc);
/* reset a m6581_t instance */
extern void m6581_reset(m6581_t* sid);
/* read/write m6581_t registers */
extern uint64_t m6581_iorq(m6581_t* sid, uint64_t pins);
/* tick a m6581_t instance, returns true when new sample is ready */
extern bool m6581_tick(m6581_t* sid);

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

/* extract 8-bit data bus from 64-bit pins */
#define M6581_GET_DATA(p) ((uint8_t)((p&0xFF0000ULL)>>16))
/* merge 8-bit data bus value into 64-bit pins */
#define M6581_SET_DATA(p,d) {p=(((p)&~0xFF0000ULL)|(((d)<<16)&0xFF0000ULL));}
/* fixed point precision for sample period */
#define M6581_FIXEDPOINT_SCALE (16)

static const uint16_t _m6581_env_period[16] = {
    9, 32, 63, 95, 149, 220, 267, 313, 
    392, 977, 1954, 3126, 3906, 11720, 19531, 31251
};

static void _m6581_init_voice(m6581_voice_t* v) {
    v->freq = v->pulse_width = 0;
    v->ctrl = 0;
    v->attack = v->decay = v->sustain = v->release = 0;
    v->sync = false;
    v->wav_accum = 0;
    v->noise_shift = 0x007FFFF8;
    v->wav_output = 0;
    v->env_state = M6581_ENV_RELEASE;
    v->env_rate_count = 0;
    v->env_rate_period = _m6581_env_period[0];
    v->env_exp_count = 0;
    v->env_count = 0;
    v->env_hold = true;
}

void m6581_init(m6581_t* sid, m6581_desc_t* desc) {
    CHIPS_ASSERT(sid && desc);
    CHIPS_ASSERT(desc->tick_hz > 0);
    CHIPS_ASSERT(desc->sound_hz > 0);
    memset(sid, 0, sizeof(*sid));
    sid->sample_period = (desc->tick_hz * M6581_FIXEDPOINT_SCALE) / desc->sound_hz;
    sid->sample_counter = sid->sample_period;
    sid->mag = desc->magnitude;
    for (int i = 0; i < 3; i++) {
        _m6581_init_voice(&sid->voice[i]);
    }
}

void m6581_reset(m6581_t* sid) {
    CHIPS_ASSERT(sid);
    sid->bus_value = 0;
    sid->bus_decay = 0x2000;
    for (int i = 0; i < 3; i++) {
        _m6581_init_voice(&sid->voice[i]);
    }
    sid->filter.cutoff = 0;
    sid->filter.resonance = 0;
    sid->filter.filter = 0;
    sid->filter.mode = 0;
    sid->filter.volume = 0;
    sid->sample_counter = sid->sample_period;
    sid->sample = 0.0f;
}

/*--- VOICE IMPLEMENTATION ---------------------------------------------------*/
static inline void _m6581_set_freq_lo(m6581_voice_t* v, uint8_t data) {
    v->freq = (v->freq & 0xFF00) | data;
}

static inline void _m6581_set_freq_hi(m6581_voice_t* v, uint8_t data) {
    v->freq = (data<<8) | (v->freq & 0x00FF);
}

static inline void _m6581_set_pw_lo(m6581_voice_t* v, uint8_t data) {
    v->pulse_width = (v->pulse_width & 0x0F00) | data;
}

static inline void _m6581_set_pw_hi(m6581_voice_t* v, uint8_t data) {
    v->pulse_width = ((data & 0x0F)<<8) | (v->pulse_width & 0x00FF);
}

static inline void _m6581_set_atkdec(m6581_voice_t* v, uint8_t data) {
    v->attack = data >> 4;
    v->decay = data & 0x0F;
    if (v->env_state == M6581_ENV_ATTACK) {
        v->env_rate_period = _m6581_env_period[v->attack];
    }
    else if (v->env_state == M6581_ENV_DECAY_SUSTAIN) {
        v->env_rate_period = _m6581_env_period[v->decay];
    }
}

static inline void _m6581_set_susrel(m6581_voice_t* v, uint8_t data) {
    v->sustain = data >> 4;
    v->release = data & 0x0F;
    if (v->env_state == M6581_ENV_RELEASE) {
        v->env_rate_period = _m6581_env_period[v->release];
    }
}

static inline void _m6581_set_ctrl(m6581_voice_t* v, uint8_t data) {
    /* wave/noise generator */
    if (data & M6581_CTRL_TEST) {
        v->wav_accum = 0;
        v->noise_shift = 0;
    }
    else if (v->ctrl & M6581_CTRL_TEST) {
        v->noise_shift = 0x007FFFF8;
    }

    /* envelope generator */
    if ((data & M6581_CTRL_GATE) && !(v->ctrl & M6581_CTRL_GATE)) {
        /* gate bit on: start attack */
        v->env_state = M6581_ENV_ATTACK;
        v->env_rate_period = _m6581_env_period[v->attack];
        v->env_hold = false;
    }
    else if (!(data & M6581_CTRL_GATE) && (v->ctrl & M6581_CTRL_GATE)) {
        /* gate bit off: start release */
        v->env_state = M6581_ENV_RELEASE;
        v->env_rate_period = _m6581_env_period[v->release];
    }
    
    v->ctrl = data;
}

static inline uint16_t _m6581_triangle(m6581_voice_t* v, m6581_voice_t* v_sync) {
    uint32_t accum;
    if (v->ctrl & M6581_CTRL_RINGMOD) {
        if ((v->wav_accum ^ v_sync->wav_accum) & 0x00800000) {
            accum = ~v->wav_accum;
        }
        else {
            accum = v->wav_accum;
        }
    }
    else {
        accum = (v->wav_accum & 0x00800000) ? ~v->wav_accum : v->wav_accum;
    }
    return (accum>>11) & 0x0FFF;
}

static inline uint16_t _m6581_sawtooth(m6581_voice_t* v) {
    return (v->wav_accum>>12) & 0x0FFF;
}

static inline uint16_t _m6581_pulse(m6581_voice_t* v) {
    if (v->ctrl & M6581_CTRL_TEST) {
        return 0x0FFF;
    }
    else {
        if ((v->wav_accum >> 12) >= v->pulse_width) {
            return 0x0FFF;
        }
        else {
            return 0x0000;
        }
    }
}

static inline uint16_t _m6581_noise(m6581_voice_t* v) {
    uint16_t s = v->noise_shift;
    return ((s & 0x00400000)>>11) |
           ((s & 0x00100000)>>10) |
           ((s & 0x00010000)>>7) |
           ((s & 0x00002000)>>5) |
           ((s & 0x00000800)>>4) |
           ((s & 0x00000080)>>1) |
           ((s & 0x00000010)<<1) |
           ((s & 0x00000004)<<2);
}

static inline uint16_t _m6581_env_exp_period(uint8_t cnt) {
    if ((cnt >= 1) && (cnt < 7)) {
        return 30;
    }
    else if ((cnt >= 7) && (cnt < 15)) {
        return 16;
    }
    else if ((cnt >= 15) && (cnt < 27)) {
        return 8;
    }
    else if ((cnt >= 27) && (cnt < 55)) {
        return 4;
    }
    else if ((cnt >= 55) && (cnt < 94)) {
        return 2;
    }
    else {
        return 1;
    }
}

static inline void _m6581_voice_tick(m6581_t* sid, int voice_index) {
    m6581_voice_t* v = &sid->voice[voice_index];
    if (0 == (v->ctrl & M6581_CTRL_TEST)) {
        /* frequency accumulator */
        uint32_t prev_accum = v->wav_accum;
        v->wav_accum = (v->wav_accum + v->freq) & 0x00FFFFFF;
        /* noise */
        if ((v->wav_accum & 0x00080000) && !(prev_accum & 0x00080000)) {
            uint32_t s = v->noise_shift;
            uint32_t new_bit = ((s>>22)^(s>>17)) & 1;
            v->noise_shift = ((s<<1)|new_bit) & 0x007FFFFF;
        }
        /* sync state */
        v->sync = (v->wav_accum & 0x00800000) &  !(prev_accum & 0x00800000);
    }

    /* waveform output */
    m6581_voice_t* v_sync = &sid->voice[(voice_index+2)%3];
    switch ((v->ctrl>>4) & 0x0F) {
        case 0: /* none */
            v->wav_output = 0;
            break;
        case 1: /* triangle */
            v->wav_output = _m6581_triangle(v, v_sync);
            break;
        case 2: /* sawtooth */
            v->wav_output = _m6581_sawtooth(v);
            break;
        case 3: /* FIXME: triangle/sawtooth */
            v->wav_output = 0;
            break;
        case 4: /* pulse */
            v->wav_output = _m6581_pulse(v);
            break;
        case 5: /* FIXME: pulse/triangle */
            v->wav_output = 0;
            break;
        case 6: /* FIXME: pulse/sawtooth */
            v->wav_output = 0;
            break;
        case 7: /* FIXME: triangle/sawtooth/pulse */
            v->wav_output = 0;
            break;
        case 8: /* noise */
            v->wav_output = _m6581_noise(v);
            break;
        default:
            v->wav_output = 0;
            break;
    }

    /* envelope generator */
    if (v->env_rate_count == v->env_rate_period) {
        v->env_rate_count = 0;
        if ((v->env_state == M6581_ENV_ATTACK) ||
            (++v->env_exp_count == _m6581_env_exp_period(v->env_count)))
        {
            v->env_exp_count = 0;
        }
        if (!v->env_hold) {
            switch (v->env_state) {
                case M6581_ENV_ATTACK:
                    if (++v->env_count == 0xFF) {
                        v->env_state = M6581_ENV_DECAY_SUSTAIN;
                        v->env_rate_period = _m6581_env_period[v->decay];
                    }
                    break;
                case M6581_ENV_DECAY_SUSTAIN:
                    if (v->env_count != ((v->sustain<<4)|v->sustain)) {
                        v->env_count--;
                    }
                    break;
                case M6581_ENV_RELEASE:
                    v->env_count--;
                    break;
            }
            if (0 == v->env_count) {
                v->env_hold = true;
            }
        }
    }
    else {
        v->env_rate_count = (v->env_rate_count + 1) & 0x7FFF;
    }
}

bool m6581_tick(m6581_t* sid) {
    CHIPS_ASSERT(sid);

    /* decay the last written register value */
    if (sid->bus_decay > 0) {
        if (--sid->bus_decay == 0) {
            sid->bus_value = 0;
        }
    }

    for (int i = 0; i < 3; i++) {
        _m6581_voice_tick(sid, i);
    }

    // FIXME
    return false;
}

uint64_t m6581_iorq(m6581_t* sid, uint64_t pins) {
    CHIPS_ASSERT(sid);
    if (pins & M6581_CS) {
        uint8_t addr = pins & M6581_ADDR_MASK;
        if (pins & M6581_RW) {
            uint8_t data;
            switch (addr) {
                case M6581_POT_X:
                case M6581_POT_Y:
                    /* FIXME: potentiometers */
                    data = 0x00;
                    break;
                case M6581_OSC3RAND:
                    /* FIXME */
                    data = 0x00;
                    break;
                case M6581_ENV3:
                    /* FIXME */
                    data = 0x00;
                    break;
                default:
                    data = sid->bus_value;
                    break;
            }
            M6581_SET_DATA(pins, data);
        }
        else {
            /* write access (only voice and filter regs) */
            uint8_t data = M6581_GET_DATA(pins);
            sid->bus_value = data;
            sid->bus_decay = 0x2000;
            switch (addr) {
                case M6581_V1_FREQ_LO:
                    _m6581_set_freq_lo(&sid->voice[0], data);
                    break;
                case M6581_V1_FREQ_HI:
                    _m6581_set_freq_hi(&sid->voice[0], data);
                    break;
                case M6581_V1_PW_LO:
                    _m6581_set_pw_lo(&sid->voice[0], data);
                    break;
                case M6581_V1_PW_HI:
                    _m6581_set_pw_hi(&sid->voice[0], data);
                    break;
                case M6581_V1_CTRL:
                    _m6581_set_ctrl(&sid->voice[0], data);
                    break;
                case M6581_V1_ATKDEC:
                    _m6581_set_atkdec(&sid->voice[0], data);
                    break;
                case M6581_V1_SUSREL:
                    _m6581_set_susrel(&sid->voice[0], data);
                    break;
                case M6581_V2_FREQ_LO:
                    _m6581_set_freq_lo(&sid->voice[1], data);
                    break;
                case M6581_V2_FREQ_HI:
                    _m6581_set_freq_hi(&sid->voice[1], data);
                    break;
                case M6581_V2_PW_LO:
                    _m6581_set_pw_lo(&sid->voice[1], data);
                    break;
                case M6581_V2_PW_HI:
                    _m6581_set_pw_hi(&sid->voice[1], data);
                    break;
                case M6581_V2_CTRL:
                    _m6581_set_ctrl(&sid->voice[1], data);
                    break;
                case M6581_V2_ATKDEC:
                    _m6581_set_atkdec(&sid->voice[1], data);
                    break;
                case M6581_V2_SUSREL:
                    _m6581_set_susrel(&sid->voice[1], data);
                    break;
                case M6581_V3_FREQ_LO:
                    _m6581_set_freq_lo(&sid->voice[2], data);
                    break;
                case M6581_V3_FREQ_HI:
                    _m6581_set_freq_hi(&sid->voice[2], data);
                    break;
                case M6581_V3_PW_LO:
                    _m6581_set_pw_lo(&sid->voice[2], data);
                    break;
                case M6581_V3_PW_HI:
                    _m6581_set_pw_hi(&sid->voice[2], data);
                    break;
                case M6581_V3_CTRL:
                    _m6581_set_ctrl(&sid->voice[2], data);
                    break;
                case M6581_V3_ATKDEC:
                    _m6581_set_atkdec(&sid->voice[2], data);
                    break;
                case M6581_V3_SUSREL:
                    _m6581_set_susrel(&sid->voice[2], data);
                    break;
                case M6581_FC_LO:
                    sid->filter.cutoff = (sid->filter.cutoff & ~7) | (data & 7);
                    break;
                case M6581_FC_HI:
                    sid->filter.cutoff = (data<<3) | (sid->filter.cutoff & 7);
                    break;
                case M6581_RES_FILT:
                    sid->filter.resonance = data >> 4;
                    sid->filter.filter = data & 0x0F;
                    break;
                case M6581_MODE_VOL:
                    sid->filter.mode = data >> 4;
                    sid->filter.volume = data & 0x0F;
                    break;
            }
        }
    }
    return pins;
}
#endif /* CHIPS_IMPL */

#ifdef __cplusplus
} /* extern "C" */
#endif
