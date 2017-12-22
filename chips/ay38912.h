#pragma once
/*
    ay38912.h   -- AY-3-8912 sound chip emulator


    Do this:
        #define CHIPS_IMPL
    before you include this file in *one* C or C++ file to create the 
    implementation.

    Optionally provde the following macros with your own implementation
    
    CHIPS_ASSERT(c)     -- your own assert macro (default: assert(c))

    EMULATED PINS:

             +-----------+
      BC1 -->|           |<-> DA0
     BDIR -->|           |<-> DA1
             |           |...
             |           |<-> DA7
             |           |
             |           |<-> IOA7
             |           |<-> IOA6
             |           |...
             |           |<-> IOA0
             +-----------+

    NOT EMULATED:

    - this is a AY-3-8912 emulator, not AY-3-8910, so there's only one IO port
    - the BC2 pin is ignored since it makes only sense when connected to
      a CP1610 CPU
    - the RESET pin state is ignored, instead call ay38912_reset()

    LICENSE:

    MIT License

    Copyright (c) 2017 Andre Weissflog

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
*/
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
    Pin definitions.

    The AY-3-8912 is in a smaller 28 pin package and only has one
    I/O channel (compared to the AY-3-8910 which has 40 pins and 
    2 I/O channels).

    Note that the BC2 is not emulated since it is usually always
    set to active when not connected to a CP1610 processor. The
    remaining BDIR and BC1 pins are interprested as follows:

    |BDIR|BC1|
    +----+---+
    |  0 | 0 |  INACTIVE
    |  0 | 1 |  READ FROM PSG
    |  1 | 0 |  WRITE TO PSG
    |  1 | 1 |  LATCH ADDRESS
*/

/* 8 bits data/address bus shared with CPU data bus */
#define AY38912_DA0 (1ULL<<16)
#define AY38912_DA1 (1ULL<<17)
#define AY38912_DA2 (1ULL<<18)
#define AY38912_DA3 (1ULL<<19)
#define AY38912_DA4 (1ULL<<20)
#define AY38912_DA5 (1ULL<<21)
#define AY38912_DA6 (1ULL<<22)
#define AY38912_DA7 (1ULL<<23)

/* reset pin shared with CPU */
#define AY38912_RESET   (1ULL<<34)

/* chip-specific pins start at position 44 */
#define AY38912_BDIR    (1ULL<<44)
#define AY38912_BC1     (1ULL<<45)
#define AY38912_A8      (1ULL<<46)

/* AY-3-8912 registers */
#define AY38912_REG_PERIOD_A_FINE       (0)
#define AY38912_REG_PERIOD_A_COARSE     (1)
#define AY38912_REG_PERIOD_B_FINE       (2)
#define AY38912_REG_PERIOD_B_COARSE     (3)
#define AY38912_REG_PERIOD_C_FINE       (4)
#define AY38912_REG_PERIOD_C_COARSE     (5)
#define AY38912_REG_PERIOD_NOISE        (6)
#define AY38912_REG_ENABLE              (7)
#define AY38912_REG_AMP_A               (8)
#define AY38912_REG_AMP_B               (9)
#define AY38912_REG_AMP_C               (10)
#define AY38912_REG_ENV_PERIOD_FINE     (11)
#define AY38912_REG_ENV_PERIOD_COARSE   (12)
#define AY38912_REG_ENV_SHAPE_CYCLE     (13)
#define AY38912_REG_IO_PORT_A           (14)
#define AY38912_REG_IO_PORT_B           (15)    /* not on AY-3-8912 */

/* super-sampling precision */
#define AY38912_SUPER_SAMPLES (4)
/* error-accumulation precision boost */
#define AY38912_PRECISION_BOOST (16)
/* number of registers */
#define AY38912_NUM_REGISTERS (16)
/* number of channels */
#define AY38912_NUM_CHANNELS (3)

/* a tone channel */
typedef struct {
    uint16_t period;
    uint16_t counter;
    bool bit;
    bool tone_disable;
    bool noise_disable;
} ay38912_tone;

/* the noise channel state */
typedef struct {
    uint16_t period;
    uint16_t counter;
    uint32_t rng;
    bool bit;
} ay38912_noise;

/* the envelope channel */
typedef struct {
    int period;
    int counter;
} ay38912_envelope;

/* AY-3-8912 state */
typedef struct {
    /* 4-bit address latch */
    uint8_t addr;
    /* 16 registers */
    uint8_t reg[AY38912_NUM_REGISTERS];
    /* main frequency period counter of 16 */
    int main_counter;

    /* the 3 tone channels */
    ay38912_tone tone[AY38912_NUM_CHANNELS];
    /* the noise generator state */
    ay38912_noise noise;
    /* the envelope generator state */
    ay38912_envelope envelope;

    /* sample generation state */
    int sample_period;
    int sample_counter;
    int super_sample_counter;
    float mag;
    float acc;
    float sample;
} ay38912;

/* initialize a AY-3-8912 instance */
extern void ay38912_init(ay38912* ay, int tick_hz, int sound_hz, float magnitude);
/* reset an existing AY-3-8912 instance */
extern void ay38912_reset(ay38912* ay);
/* perform an IO request machine cycle */
extern uint64_t ay38912_iorq(ay38912* ay, uint64_t pins);
/* tick the AY-3-8912, return true if a new sample is ready */
extern bool ay38912_tick(ay38912* ay, int num_ticks);

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
#define AY38912_DATA(p) ((uint8_t)(p>>16))
/* merge 8-bit data bus value into 64-bit pins */
#define AY38912_SET_DATA(p,d) {p=((p&~0xFF0000)|((d&0xFF)<<16));}

/* valid register content bitmasks */
static const uint8_t _ay38912_reg_mask[AY38912_NUM_REGISTERS] = {
    0xFF,       /* AY38912_REG_PERIOD_A_FINE */
    0x0F,       /* AY38912_REG_PERIOD_A_COARSE */
    0xFF,       /* AY38912_REG_PERIOD_B_FINE */
    0x0F,       /* AY38912_REG_PERIOD_B_COARSE */
    0xFF,       /* AY38912_REG_PERIOD_C_FINE */
    0x0F,       /* AY38912_REG_PERIOD_C_COARSE */
    0x1F,       /* AY38912_REG_PERIOD_NOISE */
    0xFF,       /* AY38912_REG_ENABLE */
    0x1F,       /* AY38912_REG_AMP_A (0..3: 4-bit volume, 4: use envelope) */
    0x1F,       /* AY38912_REG_AMP_B (0..3: 4-bit volume, 4: use envelope) */
    0x1F,       /* AY38912_REG_AMP_C (0..3: 4-bit volume, 4: use envelope) */
    0xFF,       /* AY38912_REG_ENV_PERIOD_FINE */
    0xFF,       /* AY38912_REG_ENV_PERIOD_COARSE */
    0x0F,       /* AY38912_REG_ENV_SHAPE_CYCLE */
    0xFF,       /* AY38912_REG_IO_PORT_A */
    0xFF,       /* AY38912_REG_IO_PORT_B */
};

/* volume table from: https://github.com/true-grue/ayumi/blob/master/ayumi.c */
static const float _ay38911_volumes[16] = {
  0.0f,
  0.00999465934234f,
  0.0144502937362f,
  0.0210574502174f,
  0.0307011520562f,
  0.0455481803616f,
  0.0644998855573f,
  0.107362478065f,
  0.126588845655f,
  0.20498970016f,
  0.292210269322f,
  0.372838941024f,
  0.492530708782f,
  0.635324635691f,
  0.805584802014f,
  1.0f
};

/* update computed values after registers have been reprogrammed */
static void _ay38912_update(ay38912* ay) {
    for (int i = 0; i < AY38912_NUM_CHANNELS; i++) {
        ay38912_tone* chn = &ay->tone[i];
        /* "...Note also that due to the design technique used in the Tone Period
           count-down, the lowest period value is 000000000001 (divide by 1)
           and the highest period value is 111111111111 (divide by 4095)
        */
        chn->period = (ay->reg[2*i+1]<<8)|(ay->reg[2*i]);
        if (0 == chn->period) {
            chn->period = 1;
        }
        /* a set 'enable bit' actually means 'disabled' */
        chn->tone_disable = (ay->reg[AY38912_REG_ENABLE]>>i) & 1;
        chn->noise_disable = (ay->reg[AY38912_REG_ENABLE]>>(3+i)) & 1;
    }
    ay->noise.period = ay->reg[AY38912_REG_PERIOD_NOISE];
    if (ay->noise.period == 0) {
        ay->noise.period = 1;
    }
    ay->envelope.period = 16 * ((ay->reg[AY38912_REG_ENV_PERIOD_COARSE]<<8)|(ay->reg[AY38912_REG_ENV_PERIOD_FINE]));
    /* FIXME: more envelope stuff */
}

void ay38912_init(ay38912* ay, int tick_hz, int sound_hz, float magnitude) {
    CHIPS_ASSERT(ay);
    CHIPS_ASSERT((tick_hz > 0) && (sound_hz > 0));
    memset(ay, 0, sizeof(*ay));
    ay->noise.rng = 1;
    ay->sample_period = (tick_hz * AY38912_PRECISION_BOOST) / (sound_hz * AY38912_SUPER_SAMPLES);
    ay->sample_counter = ay->sample_period;
    ay->super_sample_counter = AY38912_SUPER_SAMPLES;
    ay->mag = magnitude;
    _ay38912_update(ay);
}

void ay38912_reset(ay38912* ay) {
    CHIPS_ASSERT(ay);
    ay->addr = 0;
    for (int i = 0; i < AY38912_NUM_REGISTERS; i++) {
        ay->reg[i] = 0;
    }
    _ay38912_update(ay);
}

bool au38912_tick(ay38912* ay, int num_ticks) {
    /* main clock is divided by 16 */
    ay->main_counter -= num_ticks;
    while (ay->main_counter <= 0) {
        ay->main_counter += 16;

        /* tick the tone channels */
        for (int i = 0; i < AY38912_NUM_CHANNELS; i++) {
            ay38912_tone* chn = &ay->tone[i];
            if (++chn->counter >= chn->period) {
                chn->counter = 0;
                chn->bit = !chn->bit;
            }
        }

        /* FIXME: tick the noise channel */

        /* FIXME: tick the envelope generator */

        /* generate new sample */
    }

    /* generate new sample? */
    ay->sample_counter -= num_ticks * AY38912_PRECISION_BOOST;
    while (ay->sample_counter < 0) {
        ay->sample_counter += ay->sample_period;
        float vol = 0.0f;
        for (int i = 0; i < AY38912_NUM_CHANNELS; i++) {
            const ay38912_tone* chn = &ay->tone[i];
            if (0 == (ay->reg[AY38912_REG_AMP_A+i] & (1<<4))) {
                /* fixed amplitude */
                vol = _ay38911_volumes[ay->reg[AY38912_REG_AMP_A+i] & 0x0F];
            }
            else {
                /* FIXME: envelope control */
            }
            int vol_enable = (chn->bit|chn->tone_disable) & ((ay->noise.rng&1)|(chn->noise_disable));
            ay->acc += vol_enable ? vol : -vol;
        }
        if (--ay->super_sample_counter == 0) {
            ay->super_sample_counter = AY38912_SUPER_SAMPLES;
            ay->sample = ay->acc / AY38912_SUPER_SAMPLES;
            ay->acc = 0.0f;
            return true;    /* new sample is ready */
        }
    }
    /* fallthrough: no new sample ready yet */
    return false;
}

uint64_t ay38912_iorq(ay38912* ay, uint64_t pins) {
    if (pins & (AY38912_BDIR|AY38912_BC1)) {
        if (pins & AY38912_BDIR) {
            const uint8_t data = AY38912_DATA(pins);
            if (pins & AY38912_BC1) {
                /* latch address */
                ay->addr = data;
            }
            else {
                /* Write to register using the currently latched address.
                   The whole 8-bit address is considered, the low 4 bits
                   are the register index, and the upper bits are burned
                   into the chip as a 'chip select' and are usually 0
                   (this emulator assumes they are 0, so addresses greater
                   are ignored for reading and writing)
                */
                if (ay->addr < AY38912_NUM_REGISTERS) {
                    ay->reg[ay->addr] = data & _ay38912_reg_mask[ay->addr];
                    _ay38912_update(ay);
                }
            }
        }
        else {
            /* Read from register using the currently latched address.
               See 'write' for why the latched address must be in the
               valid register range to have an effect.
            */
            if (ay->addr < AY38912_NUM_REGISTERS) {
                const uint8_t data = ay->reg[ay->addr];
                AY38912_SET_DATA(pins, data);
            }
        }
    }
    return pins;
}

#endif /* CHIPS_IMPL */

#ifdef __cplusplus
} /* extern "C" */
#endif
