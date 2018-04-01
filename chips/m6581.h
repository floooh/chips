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

/* voice state */
typedef struct {
    uint16_t freq;
    uint16_t pulse_width;
    uint8_t ctrl;
    uint8_t attack;
    uint8_t decay;
    uint8_t sustain;
    uint8_t release;
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
/* set hi/lo parts of 16-bit word */
#define M6581_SET_HI(w,hi) {w=((hi)<<8)|(w&0x00FF);}
#define M6581_SET_LO(w,lo) {w=(w&0xFF00)|((lo)&0xFF);}
/* fixed point precision for sample period */
#define M6581_FIXEDPOINT_SCALE (16)

void m6581_init(m6581_t* sid, m6581_desc_t* desc) {
    CHIPS_ASSERT(sid && desc);
    CHIPS_ASSERT(desc->tick_hz > 0);
    CHIPS_ASSERT(desc->sound_hz > 0);
    memset(sid, 0, sizeof(*sid));
    sid->sample_period = (desc->tick_hz * M6581_FIXEDPOINT_SCALE) / desc->sound_hz;
    sid->sample_counter = sid->sample_period;
    sid->mag = desc->magnitude;
}

void m6581_reset(m6581_t* sid) {
    CHIPS_ASSERT(sid);
    sid->bus_value = 0;
    sid->bus_decay = 0x2000;
    for (int i = 0; i < 3; i++) {
        m6581_voice_t* v = &(sid->voice[i]);
        v->freq = v->pulse_width = 0;
        v->ctrl = 0;
        v->attack = v->decay = v->sustain = v->release = 0;
    }
    sid->filter.cutoff = 0;
    sid->filter.resonance = 0;
    sid->filter.filter = 0;
    sid->filter.mode = 0;
    sid->filter.volume = 0;
    sid->sample_counter = sid->sample_period;
    sid->sample = 0.0f;
}

bool m6581_tick(m6581_t* sid) {
    CHIPS_ASSERT(sid);

    /* decay the last written register value */
    if (sid->bus_decay > 0) {
        if (--sid->bus_decay == 0) {
            sid->bus_value = 0;
        }
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
                case M6581_OSC3RAND:
                    /* FIXME */
                    data = 0x00;
                case M6581_ENV3:
                    /* FIXME */
                    data = 0x00;
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
                    M6581_SET_LO(sid->voice[0].freq, data);
                    break;
                case M6581_V1_FREQ_HI:
                    M6581_SET_HI(sid->voice[0].freq, data);
                    break;
                case M6581_V1_PW_LO:
                    M6581_SET_LO(sid->voice[0].pulse_width, data);
                    break;
                case M6581_V1_PW_HI:
                    M6581_SET_HI(sid->voice[0].pulse_width, (data & 0x0F));
                    break;
                case M6581_V1_CTRL:
                    sid->voice[0].ctrl = data;
                    break;
                case M6581_V1_ATKDEC:
                    sid->voice[0].attack = (data>>4);
                    sid->voice[0].decay  = (data & 0x0F);
                    break;
                case M6581_V1_SUSREL:
                    sid->voice[0].sustain = (data>>4);
                    sid->voice[0].release = (data & 0x0F);
                    break;
                case M6581_V2_FREQ_LO:
                    M6581_SET_LO(sid->voice[1].freq, data);
                    break;
                case M6581_V2_FREQ_HI:
                    M6581_SET_HI(sid->voice[1].freq, data);
                    break;
                case M6581_V2_PW_LO:
                    M6581_SET_LO(sid->voice[1].pulse_width, data);
                    break;
                case M6581_V2_PW_HI:
                    M6581_SET_HI(sid->voice[1].pulse_width, (data & 0x0F));
                    break;
                case M6581_V2_CTRL:
                    sid->voice[1].ctrl = data;
                    break;
                case M6581_V2_ATKDEC:
                    sid->voice[1].attack = (data>>4);
                    sid->voice[1].decay  = (data & 0x0F);
                    break;
                case M6581_V2_SUSREL:
                    sid->voice[1].sustain = (data>>4);
                    sid->voice[1].release = (data & 0x0F);
                    break;
                case M6581_V3_FREQ_LO:
                    M6581_SET_LO(sid->voice[2].freq, data);
                    break;
                case M6581_V3_FREQ_HI:
                    M6581_SET_HI(sid->voice[2].freq, data);
                    break;
                case M6581_V3_PW_LO:
                    M6581_SET_LO(sid->voice[2].pulse_width, data);
                    break;
                case M6581_V3_PW_HI:
                    M6581_SET_HI(sid->voice[2].pulse_width, (data & 0x0F));
                    break;
                case M6581_V3_CTRL:
                    sid->voice[2].ctrl = data;
                    break;
                case M6581_V3_ATKDEC:
                    sid->voice[2].attack = data >> 4;
                    sid->voice[2].decay  = data & 0x0F;
                    break;
                case M6581_V3_SUSREL:
                    sid->voice[2].sustain = data >> 4;
                    sid->voice[2].release = data & 0x0F;
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
