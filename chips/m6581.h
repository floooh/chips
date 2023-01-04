#pragma once
/*#
    # m6581.h

    MOS Technology 6581 emulator (aka SID)

    based on tedplay (c) 2012 Attila Grosz, used under Unlicense http://unlicense.org/

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
    *           +-----------+         *
    ***********************************

    The emulation has an additional "virtual pin" which is set to active
    whenever a new sample is ready (M6581_SAMPLE).

    ## Links

    - http://blog.kevtris.org/?p=13

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
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// address bus pins A0..A4
#define M6581_PIN_A0    (0)
#define M6581_PIN_A1    (1)
#define M6581_PIN_A2    (2)
#define M6581_PIN_A3    (3)
#define M6581_PIN_A4    (4)

// data bus pins D0..D7
#define M6581_PIN_D0    (16)
#define M6581_PIN_D1    (17)
#define M6581_PIN_D2    (18)
#define M6581_PIN_D3    (19)
#define M6581_PIN_D4    (20)
#define M6581_PIN_D5    (21)
#define M6581_PIN_D6    (22)
#define M6581_PIN_D7    (23)

// shared control pins
#define M6581_PIN_RW    (24)      /* same as M6502_RW */

// chip-specific pins
#define M6581_PIN_CS        (40)      /* chip-select */
#define M6581_PIN_SAMPLE    (41)      /* virtual "audio sample ready" pin */

// pin bit masks
#define M6581_A0    (1ULL<<M6581_PIN_A0)
#define M6581_A1    (1ULL<<M6581_PIN_A1)
#define M6581_A2    (1ULL<<M6581_PIN_A2)
#define M6581_A3    (1ULL<<M6581_PIN_A3)
#define M6581_A4    (1ULL<<M6581_PIN_A4)
#define M6581_ADDR_MASK (0x1F)
#define M6581_D0    (1ULL<<M6581_PIN_D0)
#define M6581_D1    (1ULL<<M6581_PIN_D1)
#define M6581_D2    (1ULL<<M6581_PIN_D2)
#define M6581_D3    (1ULL<<M6581_PIN_D3)
#define M6581_D4    (1ULL<<M6581_PIN_D4)
#define M6581_D5    (1ULL<<M6581_PIN_D5)
#define M6581_D6    (1ULL<<M6581_PIN_D6)
#define M6581_D7    (1ULL<<M6581_PIN_D7)
#define M6581_RW    (1ULL<<M6581_PIN_RW)
#define M6581_CS        (1ULL<<M6581_PIN_CS)
#define M6581_SAMPLE    (1ULL<<M6581_PIN_SAMPLE)

// registers
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

// voice control bits
#define M6581_CTRL_GATE     (1<<0)
#define M6581_CTRL_SYNC     (1<<1)
#define M6581_CTRL_RINGMOD  (1<<2)
#define M6581_CTRL_TEST     (1<<3)
#define M6581_CTRL_TRIANGLE (1<<4)
#define M6581_CTRL_SAWTOOTH (1<<5)
#define M6581_CTRL_PULSE    (1<<6)
#define M6581_CTRL_NOISE    (1<<7)

// filter routing bits
#define M6581_FILTER_FILT1  (1<<0)
#define M6581_FILTER_FILT2  (1<<1)
#define M6581_FILTER_FILT3  (1<<2)
#define M6581_FILTER_FILTEX (1<<3)

// filter mode bits
#define M6581_FILTER_LP     (1<<0)
#define M6581_FILTER_BP     (1<<1)
#define M6581_FILTER_HP     (1<<2)
#define M6581_FILTER_3OFF   (1<<3)

// setup parameters for m6581_init()
typedef struct {
    int tick_hz;        // frequency at which m6581_tick() will be called in Hz
    int sound_hz;       // sound sample frequency
    float magnitude;    // output sample magnitude (0=silence to 1=max volume)
} m6581_desc_t;

// envelope generator state
typedef enum {
    M6581_ENV_FROZEN,
    M6581_ENV_ATTACK,
    M6581_ENV_DECAY,
    M6581_ENV_RELEASE
} m6581_env_state_t;

// voice state
typedef struct {
    bool muted;
    // wave generator state
    uint16_t freq;
    uint16_t pulse_width;
    uint8_t ctrl;
    bool sync;
    uint32_t noise_shift;           // 24 bit
    uint32_t wav_accum;             // 8.16 fixed
    uint32_t wav_output;            // 12 bit

    // envelope generator state
    m6581_env_state_t env_state;
    uint32_t env_attack_add;
    uint32_t env_decay_sub;
    uint32_t env_sustain_level;
    uint32_t env_release_sub;
    uint32_t env_cur_level;
    uint32_t env_counter;
    uint32_t env_exp_counter;
    uint32_t env_counter_compare;
} m6581_voice_t;

// filter state
typedef struct {
    uint16_t cutoff;
    uint8_t resonance;
    uint8_t voices;
    uint8_t mode;
    uint8_t volume;
    int nyquist_freq;
    int resonance_coeff_div_1024;
    int w0;
    int v_hp;
    int v_bp;
    int v_lp;
} m6581_filter_t;

// m6581 instance state
typedef struct {
    int sound_hz;
    /* reading a write-only register returns the last value
       written to *any* register for about 0x2000 ticks
    */
    uint8_t bus_value;
    uint16_t bus_decay;
    // voice state
    m6581_voice_t voice[3];
    // filter state
    m6581_filter_t filter;
    // sample generation state
    int sample_period;
    int sample_counter;
    float sample_accum;
    float sample_accum_count;
    float sample_mag;
    float sample;
    // debug inspection
    uint64_t pins;
} m6581_t;

// initialize a new m6581_t instance
void m6581_init(m6581_t* sid, const m6581_desc_t* desc);
// reset a m6581_t instance
void m6581_reset(m6581_t* sid);
// tick a m6581_t instance
uint64_t m6581_tick(m6581_t* sid, uint64_t pins);

#ifdef __cplusplus
} // extern "C"
#endif

/*-- IMPLEMENTATION ----------------------------------------------------------*/
#ifdef CHIPS_IMPL
#include <string.h>
#ifdef _MSC_VER
#define _USE_MATH_DEFINES
#endif
#include <math.h>   /* tanh */
#ifndef CHIPS_ASSERT
    #include <assert.h>
    #define CHIPS_ASSERT(c) assert(c)
#endif

/* extract 8-bit data bus from 64-bit pins */
#define M6581_GET_DATA(p) ((uint8_t)(((p)&0xFF0000ULL)>>16))
/* merge 8-bit data bus value into 64-bit pins */
#define M6581_SET_DATA(p,d) {p=(((p)&~0xFF0000ULL)|(((d)<<16)&0xFF0000ULL));}
/* fixed point precision for sample period */
#define M6581_FIXEDPOINT_SCALE (16)
/* move bit into first position */
#define M6581_BIT(val,bitnr) ((val>>bitnr)&1)
/* filter constants */
#define M6581_DCWAVE (0) // (0x380)
#define M6581_DCMIXER (0) // (((-0xFFF*0xFF)/18) / (1<<7))
#define M6581_DCVOICE (0) // (0x800*0xFF)

static const uint32_t _m6581_rate_count_period[16] = {
    0x7F00, 0x0006, 0x003C, 0x0330, 0x20C0, 0x6755, 0x3800, 0x500E,
    0x1212, 0x0222, 0x1848, 0x59B8, 0x3840, 0x77E2, 0x7625, 0x0A93
};

static const uint8_t _m6581_env_gen_dr_divisors[256] = {
    30,30,30,30,30,30,16,16,16,16,16,16,16,16,8,8,
    8,8,8,8,8,8,8,8,8,8,4,4,4,4,4,4,
    4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
    4,4,4,4,4,4,2,2,2,2,2,2,2,2,2,2,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
    2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};

static float _m6581_cutoff_freq[2048];

static void _m6581_init_voice(m6581_voice_t* v) {
    memset(v, 0, sizeof(*v));
    v->noise_shift = 0x007FFFFC;
    v->env_state = M6581_ENV_FROZEN;
    v->env_counter = 0x7FFF;
}

static void _m6581_init_cutoff_table() {
    for (int i = 0; i < 2048; i++) {
        float x = i / 8.0f;
        float cf = -0.0156f * x * x + 48.473f * x - 45.074f;
        _m6581_cutoff_freq[i] = cf <= 0 ? 0 : cf;
    }
}

static void _m6581_set_filter_cutoff(m6581_filter_t*);
static void _m6581_set_resonance(m6581_filter_t*);

static void _m6581_init_filter(m6581_filter_t* f, int sound_hz) {
    memset(f, 0, sizeof(*f));
    f->nyquist_freq = sound_hz / 2;
    _m6581_set_filter_cutoff(f);
    _m6581_set_resonance(f);
}

void m6581_init(m6581_t* sid, const m6581_desc_t* desc) {
    CHIPS_ASSERT(sid && desc);
    CHIPS_ASSERT(desc->tick_hz > 0);
    CHIPS_ASSERT(desc->sound_hz > 0);
    memset(sid, 0, sizeof(*sid));
    sid->sound_hz = desc->sound_hz;
    sid->sample_period = (desc->tick_hz * M6581_FIXEDPOINT_SCALE) / desc->sound_hz;
    sid->sample_counter = sid->sample_period;
    sid->sample_mag = desc->magnitude;
    sid->sample_accum_count = 1.0f;
    for (int i = 0; i < 3; i++) {
        _m6581_init_voice(&sid->voice[i]);
    }
    _m6581_init_cutoff_table();
    _m6581_init_filter(&sid->filter, sid->sound_hz);
}

void m6581_reset(m6581_t* sid) {
    CHIPS_ASSERT(sid);
    sid->bus_value = 0;
    sid->bus_decay = 0x2000;
    for (int i = 0; i < 3; i++) {
        _m6581_init_voice(&sid->voice[i]);
    }
    _m6581_init_filter(&sid->filter, sid->sound_hz);
    sid->sample_counter = sid->sample_period;
    sid->sample = 0.0f;
    sid->sample_accum = 0.0f;
    sid->sample_accum_count = 1.0f;
    sid->pins = 0;
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
    v->env_attack_add = data >> 4;
    v->env_decay_sub = data & 0x0F;
    if (v->env_state == M6581_ENV_ATTACK) {
        v->env_counter_compare = v->env_attack_add;
    }
    else if (v->env_state == M6581_ENV_DECAY) {
        v->env_counter_compare = v->env_decay_sub;
    }
}

static inline void _m6581_set_susrel(m6581_voice_t* v, uint8_t data) {
    v->env_sustain_level = (data >> 4) * 0x11;
    v->env_release_sub = data & 0x0F;
    if (v->env_state == M6581_ENV_RELEASE) {
        v->env_counter_compare = v->env_release_sub;
    }
}

static inline void _m6581_set_ctrl(m6581_voice_t* v, uint8_t data) {
    /* wave/noise generator */
    if ((data & M6581_CTRL_TEST) && !(v->ctrl & M6581_CTRL_TEST)) {
        v->wav_accum = 0;
        uint32_t bit19 = (v->noise_shift >> 19) & 1;
        v->noise_shift = (v->noise_shift & 0x007FFFFD) | ((bit19^1)<<1);
    }
    else if (!(data & M6581_CTRL_TEST) && (v->ctrl & M6581_CTRL_TEST)) {
        uint32_t bit0 = ((v->noise_shift >> 22) ^ (v->noise_shift >> 17)) & 0x01;
        v->noise_shift <<= 1;
        v->noise_shift &= 0x007FFFFF;
        v->noise_shift |= bit0;
    }

    /* envelope generator */
    if ((data & M6581_CTRL_GATE) && !(v->ctrl & M6581_CTRL_GATE)) {
        /* gate bit on: start attack */
        v->env_state = M6581_ENV_ATTACK;
        v->env_counter_compare = v->env_attack_add;
    }
    else if (!(data & M6581_CTRL_GATE) && (v->ctrl & M6581_CTRL_GATE)) {
        /* gate bit off: start release */
        v->env_state = M6581_ENV_RELEASE;
        v->env_counter_compare = v->env_release_sub;
    }

    /* noise combined with other waveform? */
    if ((data & 0xF0) > 0x80) {
        v->noise_shift &= 0x007FFFFF^(1<<22)^(1<<20)^(1<<16)^(1<<13)^(1<<11)^(1<<7)^(1<<4)^(1<<2);
    }

    v->ctrl = data;
}

static inline uint32_t _m6581_wavnone(m6581_voice_t* v) {
    if (v->wav_accum) {
        uint32_t sm = (v->wav_accum >> 12);
        v->wav_accum >>= 1;
        return sm;
    }
    else {
        return 0;
    }
}

static inline uint32_t _m6581_triangle(m6581_voice_t* v, m6581_voice_t* v_sync) {
    const bool ring = 0 != (v->ctrl & M6581_CTRL_RINGMOD);
    uint32_t msb = (ring ? v->wav_accum ^ v_sync->wav_accum : v->wav_accum) & 0x00800000;
    return ((msb ? ~v->wav_accum : v->wav_accum) >> 11) & 0x0FFF;
}

static inline uint32_t _m6581_sawtooth(m6581_voice_t* v) {
    return v->wav_accum >> 12;
}

static inline uint32_t _m6581_pulse(m6581_voice_t* v) {
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

static inline uint32_t _m6581_trisaw(m6581_voice_t* v, m6581_voice_t* v_sync) {
    uint32_t sm = _m6581_triangle(v, v_sync) & _m6581_sawtooth(v);
    return (sm >> 1) & (sm << 1);
}

static inline uint32_t _m6581_tripulse(m6581_voice_t* v, m6581_voice_t* v_sync) {
    uint32_t sm = _m6581_triangle(v, v_sync) & _m6581_pulse(v);
    return (sm >> 1) & (sm << 1);
}

static inline uint32_t _m6581_sawpulse(m6581_voice_t* v) {
    uint32_t sm = _m6581_sawtooth(v) & _m6581_pulse(v);
    return (sm >> 1) & (sm << 1);
}

static inline uint32_t _m6581_trisawpulse(m6581_voice_t* v, m6581_voice_t* v_sync) {
    uint32_t sm = _m6581_triangle(v, v_sync) & _m6581_sawtooth(v) & _m6581_pulse(v);
    return (sm >> 1) && (sm << 1);
}

static inline uint16_t _m6581_noise(m6581_voice_t* v) {
    uint32_t s = v->noise_shift;
    return (M6581_BIT(s,22)<<11) |
           (M6581_BIT(s,20)<<10) |
           (M6581_BIT(s,16)<<9) |
           (M6581_BIT(s,13)<<8) |
           (M6581_BIT(s,11)<<7) |
           (M6581_BIT(s,7)<<6) |
           (M6581_BIT(s,4)<<5) |
           (M6581_BIT(s,2)<<4);
}

static inline void _m6581_voice_tick(m6581_t* sid, int voice_index) {
    m6581_voice_t* v = &sid->voice[voice_index];

    /* waveform generator */
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
        v->sync = (v->wav_accum & 0x00800000) && !(prev_accum & 0x00800000);
    }
    m6581_voice_t* v_sync = &sid->voice[(voice_index+2)%3];
    uint32_t sm;
    switch ((v->ctrl>>4) & 0x0F) {
        case 0: sm = _m6581_wavnone(v); break;
        case 1: sm = _m6581_triangle(v, v_sync); break;
        case 2: sm = _m6581_sawtooth(v); break;
        case 3: sm = _m6581_trisaw(v, v_sync); break;
        case 4: sm = _m6581_pulse(v); break;
        case 5: sm = _m6581_tripulse(v, v_sync); break;
        case 6: sm = _m6581_sawpulse(v); break;
        case 7: sm = _m6581_trisawpulse(v, v_sync); break;
        case 8: sm = _m6581_noise(v); break;
        default: sm = 0; break;
    }
    v->wav_output = sm;

    /* envelope generator */
    uint32_t lfsr = v->env_counter;
    if (lfsr != _m6581_rate_count_period[v->env_counter_compare & 0x0F]) {
        const uint32_t feedback = ((lfsr >> 14) ^ (lfsr >> 13)) & 1;
        lfsr = ((lfsr << 1) | feedback) & 0x7FFF;
        v->env_counter = lfsr;
    }
    else {
        v->env_counter = 0x7FFF;
        if ((v->env_state == M6581_ENV_ATTACK) ||
            (++v->env_exp_counter == _m6581_env_gen_dr_divisors[v->env_cur_level & 0xFF]))
        {
            v->env_exp_counter = 0;
            switch (v->env_state) {
                case M6581_ENV_ATTACK:
                    if (((++v->env_cur_level) & 0xFF) == 0xFF) {
                        v->env_state = M6581_ENV_DECAY;
                        v->env_counter_compare = v->env_decay_sub;
                    }
                    break;
                case M6581_ENV_DECAY:
                    if (v->env_cur_level != v->env_sustain_level) {
                        v->env_cur_level = (v->env_cur_level - 1) & 0xFF;
                        if (0 == v->env_cur_level) {
                            v->env_state = M6581_ENV_FROZEN;
                        }
                    }
                    break;
                case M6581_ENV_RELEASE:
                    v->env_cur_level = (v->env_cur_level - 1) & 0xFF;
                    if (0 == v->env_cur_level) {
                        v->env_state = M6581_ENV_FROZEN;
                    }
                    break;
                case M6581_ENV_FROZEN:
                    v->env_cur_level = 0;
                    break;
            }
        }
    }
}

static inline void _m6581_voice_sync(m6581_t* sid, int voice_index) {
    m6581_voice_t* v = &sid->voice[voice_index];
    m6581_voice_t* v_sync = &sid->voice[(voice_index+2)%3];
    if (v->sync && (v_sync->ctrl & M6581_CTRL_SYNC)) {
        v_sync->wav_accum = 0;
    }
}

/*--- FILTER IMPLEMENTATION ---------------------------------------------------*/
static void _m6581_set_filter_cutoff(m6581_filter_t* f) {
    const float freq_domain_div_coeff = 2.0f * ((float)M_PI) * 1.048576f;
    f->w0 = (int) (_m6581_cutoff_freq[f->cutoff] * freq_domain_div_coeff);
    const float nyquist_freq = (float) f->nyquist_freq;
    const float max_cutoff = nyquist_freq > 16000.0f ? 16000.0f : nyquist_freq;
    const int w0_max_dt = (int)(max_cutoff * freq_domain_div_coeff);
    if (f->w0 > w0_max_dt) {
        f->w0 = w0_max_dt;
    }
}

static void _m6581_set_resonance(m6581_filter_t* f) {
    f->resonance_coeff_div_1024 = (int) (1024.0f/(0.707f + 1.9f * ((float)f->resonance) / 15.0f) + 0.5f);
}

static void _m6581_set_cutoff_lo(m6581_filter_t* f, uint8_t data) {
    if ((data ^ f->cutoff) & 7) {
        f->cutoff = (f->cutoff & 0x7F8) | (data & 7);
        _m6581_set_filter_cutoff(f);
    }
}

static void _m6581_set_cutoff_hi(m6581_filter_t* f, uint8_t data) {
    f->cutoff = (data<< 3) | (f->cutoff & 7);
    _m6581_set_filter_cutoff(f);
}

static void _m6581_set_resfilt(m6581_filter_t* f, uint8_t data) {
    f->voices = data & 7;
    f->resonance = data >> 4;
    _m6581_set_resonance(f);
}

static void _m6581_set_modevol(m6581_t* sid, uint8_t data) {
    sid->filter.volume = data & 0x0F;
    sid->filter.mode = data >> 4;
    sid->voice[2].muted = 0 != (sid->filter.mode & M6581_FILTER_3OFF);
}

static inline int _m6581_filter_output(m6581_filter_t* f, int vi) {
    int w0_dt = f->w0 / (1<<6);
    vi = vi / (1<<7);

    int d_vlp = (w0_dt * f->v_bp) / (1<<14);
    f->v_lp -= d_vlp;
    int d_vbp = (w0_dt * f->v_hp) / (1<<14);
    f->v_bp -= d_vbp;
    f->v_hp = ((f->v_bp * f->resonance_coeff_div_1024) / (1<<10)) - f->v_lp - vi;

    int vf = 0;
    if (f->mode & M6581_FILTER_LP) {
        vf += f->v_lp;
    }
    if (f->mode & M6581_FILTER_BP) {
        vf += f->v_bp;
    }
    if (f->mode & M6581_FILTER_HP) {
        vf += f->v_hp;
    }
    return vf * (1<<7);
}

/* tick the sound generation, return true when new sample ready */
static uint64_t _m6581_tick(m6581_t* sid, uint64_t pins) {
    /* decay the last written register value */
    if (sid->bus_decay > 0) {
        if (--sid->bus_decay == 0) {
            sid->bus_value = 0;
        }
    }

    /* tick wave and envelope generators */
    for (int i = 0; i < 3; i++) {
        _m6581_voice_tick(sid, i);
    }
    /* handle voice synchronization */
    for (int i = 0; i < 3; i++) {
        _m6581_voice_sync(sid, i);
    }
    /* filter */
    int sum_filtered_outp = 0;
    int sum_outp = 0;
    for (int i = 0; i < 3; i++) {
        m6581_voice_t* v = &sid->voice[i];
        int wav_out = (int) v->wav_output;
        int env_out = (int) v->env_cur_level;
        if (sid->filter.voices & (1<<i)) {
            sum_filtered_outp += (wav_out - M6581_DCWAVE) * env_out + M6581_DCVOICE;
        }
        else {
            if (v->muted) {
                sum_outp += (0 - M6581_DCWAVE) * env_out + M6581_DCVOICE;
            }
            else {
                sum_outp += (wav_out - M6581_DCWAVE) * env_out + M6581_DCVOICE;
            }
        }
    }
    int accu = (sum_outp + _m6581_filter_output(&sid->filter, sum_filtered_outp) + M6581_DCMIXER) * sid->filter.volume;
    int sample = accu / (1<<12);
    sid->sample_accum += (sample / 16384.0f);
    sid->sample_accum_count += 1.0f;

    /* new sample? */
    sid->sample_counter -= M6581_FIXEDPOINT_SCALE;
    if (sid->sample_counter <= 0) {
        sid->sample_counter += sid->sample_period;
        float s = sid->sample_accum / sid->sample_accum_count;
        sid->sample = sid->sample_mag * s;
        sid->sample_accum = 0.0f;
        sid->sample_accum_count = 0.0f;
        pins |= M6581_SAMPLE;
    }
    else {
        pins &= ~M6581_SAMPLE;
    }
    return pins;
}

/* read a register */
static uint64_t _m6581_read(m6581_t* sid, uint64_t pins) {
    uint8_t reg = pins & M6581_ADDR_MASK;
    uint8_t data;
    switch (reg) {
        case M6581_POT_X:
        case M6581_POT_Y:
            /* FIXME: potentiometers */
            data = 0;
            sid->bus_value = 0;
            break;
        case M6581_OSC3RAND:
            /* FIXME */
            sid->bus_value = 0;
            data = sid->voice[2].wav_output >> 4;
            break;
        case M6581_ENV3:
            sid->bus_value = 0;
            data = sid->voice[2].env_cur_level;
            break;
        default:
            data = sid->bus_value;
            break;
    }
    M6581_SET_DATA(pins, data);
    return pins;
}

/* write a register */
static void _m6581_write(m6581_t* sid, uint64_t pins) {
    uint8_t reg = pins & M6581_ADDR_MASK;
    uint8_t data = M6581_GET_DATA(pins);
    sid->bus_value = data;
    sid->bus_decay = 0x2000;
    switch (reg) {
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
            _m6581_set_cutoff_lo(&sid->filter, data);
            break;
        case M6581_FC_HI:
            _m6581_set_cutoff_hi(&sid->filter, data);
            break;
        case M6581_RES_FILT:
            _m6581_set_resfilt(&sid->filter, data);
            break;
        case M6581_MODE_VOL:
            _m6581_set_modevol(sid, data);
            break;
    }
}

/* the all-in-one tick function */
uint64_t m6581_tick(m6581_t* sid, uint64_t pins) {
    CHIPS_ASSERT(sid);

    /* first perform the regular per-tick actions */
    pins = _m6581_tick(sid, pins);

    /* register read/write */
    if (pins & M6581_CS) {
        if (pins & M6581_RW) {
            pins = _m6581_read(sid, pins);
        }
        else {
            _m6581_write(sid, pins);
        }
    }
    sid->pins = pins;
    return pins;
}

#endif /* CHIPS_IMPL */
