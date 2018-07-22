#pragma once
/*
    ay38910.h   -- AY-3-8910/2/3 sound chip emulator

    Do this:
        #define CHIPS_IMPL
    before you include this file in *one* C or C++ file to create the 
    implementation.

    Optionally provde the following macros with your own implementation
    
    CHIPS_ASSERT(c)     -- your own assert macro (default: assert(c))

    EMULATED PINS:

             +-----------+
      BC1 -->|           |<-> DA7
     BDIR -->|           |...
             |           |<-> DA7
             |           |
             |           |<-> (IOA7)
             |           |...
             |           |<-> (IOA0)
             |           |
             |           |<-> (IOB7)
             |           |...
             |           |<-> (IOB0)
             +-----------+

    NOT EMULATED:

    - the BC2 pin is ignored since it makes only sense when connected to
      a CP1610 CPU
    - the RESET pin state is ignored, instead call ay38910_reset()

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
#define AY38910_DA0 (1ULL<<16)
#define AY38910_DA1 (1ULL<<17)
#define AY38910_DA2 (1ULL<<18)
#define AY38910_DA3 (1ULL<<19)
#define AY38910_DA4 (1ULL<<20)
#define AY38910_DA5 (1ULL<<21)
#define AY38910_DA6 (1ULL<<22)
#define AY38910_DA7 (1ULL<<23)

/* reset pin shared with CPU */
#define AY38910_RESET   (1ULL<<34)

/* chip-specific pins start at position 44 */
#define AY38910_BDIR    (1ULL<<44)
#define AY38910_BC1     (1ULL<<45)
#define AY38910_A8      (1ULL<<46)

/* AY-3-8910 registers */
#define AY38910_REG_PERIOD_A_FINE       (0)
#define AY38910_REG_PERIOD_A_COARSE     (1)
#define AY38910_REG_PERIOD_B_FINE       (2)
#define AY38910_REG_PERIOD_B_COARSE     (3)
#define AY38910_REG_PERIOD_C_FINE       (4)
#define AY38910_REG_PERIOD_C_COARSE     (5)
#define AY38910_REG_PERIOD_NOISE        (6)
#define AY38910_REG_ENABLE              (7)
#define AY38910_REG_AMP_A               (8)
#define AY38910_REG_AMP_B               (9)
#define AY38910_REG_AMP_C               (10)
#define AY38910_REG_ENV_PERIOD_FINE     (11)
#define AY38910_REG_ENV_PERIOD_COARSE   (12)
#define AY38910_REG_ENV_SHAPE_CYCLE     (13)
#define AY38910_REG_IO_PORT_A           (14)    /* not on AY-3-8913 */
#define AY38910_REG_IO_PORT_B           (15)    /* not on AY-3-8912/3 */
/* number of registers */
#define AY38910_NUM_REGISTERS (16)
/* error-accumulation precision boost */
#define AY38910_FIXEDPOINT_SCALE (16)
/* number of channels */
#define AY38910_NUM_CHANNELS (3)

/* IO port names */
#define AY38910_PORT_A (0)
#define AY38910_PORT_B (1)

/* envelope shape bits */
#define AY38910_ENV_HOLD        (1<<0)
#define AY38910_ENV_ALTERNATE   (1<<1)
#define AY38910_ENV_ATTACK      (1<<2)
#define AY38910_ENV_CONTINUE    (1<<3)

/* callbacks for input/output on I/O ports */
typedef uint8_t (*ay38910_in_t)(int port_id, void* user_data);
typedef void (*ay38910_out_t)(int port_id, uint8_t data, void* user_data);

/* chip subtypes */
typedef enum {
    AY38910_TYPE_8910 = 0,
    AY38910_TYPE_8912,
    AY38910_TYPE_8913
} ay38910_type_t;

/* setup parameters for ay38910_init() call */
typedef struct {
    ay38910_type_t type;    /* the subtype (default 0 is AY-3-8910) */
    int tick_hz;            /* frequency at which ay38910_tick() will be called in Hz */
    int sound_hz;           /* number of samples that will be produced per second */
    float magnitude;        /* output sample magnitude, from 0.0 (silence) to 1.0 (max volume) */ 
    ay38910_in_t in_cb;     /* I/O port input callback */
    ay38910_out_t out_cb;   /* I/O port output callback */
    void* user_data;        /* optional user-data for callbacks */
} ay38910_desc_t;

/* a tone channel */
typedef struct {
    uint16_t period;
    uint16_t counter;
    uint32_t bit;
    uint32_t tone_disable;
    uint32_t noise_disable;
} ay38910_tone_t;

/* the noise channel state */
typedef struct {
    uint16_t period;
    uint16_t counter;
    uint32_t rng;
    uint32_t bit;
} ay38910_noise_t;

/* the envelope generator */
typedef struct {
    uint16_t period;
    uint16_t counter;
    bool shape_holding;
    bool shape_hold;
    uint8_t shape_counter;
    uint8_t shape_state;
} ay38910_env_t;

/* AY-3-8910 state */
typedef struct {
    ay38910_type_t type;        /* the chip flavour */
    ay38910_in_t in_cb;         /* the port-input callback */
    ay38910_out_t out_cb;       /* the port-output callback */
    void* user_data;            /* optional user-data for callbacks */
    uint32_t tick;              /* a tick counter for internal clock division */
    uint8_t addr;               /* 4-bit address latch */
    union {                     /* the register bank */
        uint8_t reg[AY38910_NUM_REGISTERS];
        struct {
            uint8_t period_a_fine;
            uint8_t period_a_coarse;
            uint8_t period_b_fine;
            uint8_t period_b_coarse;
            uint8_t period_c_fine;
            uint8_t period_c_coarse;
            uint8_t period_noise;
            uint8_t enable;
            uint8_t amp_a;
            uint8_t amp_b;
            uint8_t amp_c;
            uint8_t period_env_fine;
            uint8_t period_env_coarse;
            uint8_t env_shape_cycle;
            uint8_t port_a;
            uint8_t port_b;
        };
    };
    ay38910_tone_t tone[AY38910_NUM_CHANNELS];  /* the 3 tone channels */
    ay38910_noise_t noise;                      /* the noise generator state */
    ay38910_env_t env;                          /* the envelope generator state */

    /* sample generation state */
    int sample_period;
    int sample_counter;
    float mag;
    float sample;
} ay38910_t;

/* extract 8-bit data bus from 64-bit pins */
#define AY38910_GET_DATA(p) ((uint8_t)(p>>16))
/* merge 8-bit data bus value into 64-bit pins */
#define AY38910_SET_DATA(p,d) {p=((p&~0xFF0000)|((d&0xFF)<<16));}

/* initialize a AY-3-8910 instance */
extern void ay38910_init(ay38910_t* ay, ay38910_desc_t* desc);
/* reset an existing AY-3-8910 instance */
extern void ay38910_reset(ay38910_t* ay);
/* perform an IO request machine cycle */
extern uint64_t ay38910_iorq(ay38910_t* ay, uint64_t pins);
/* tick the AY-3-8910, return true if a new sample is ready */
extern bool ay38910_tick(ay38910_t* ay);

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
#define AY38910_DATA(p) ((uint8_t)(p>>16))
/* merge 8-bit data bus value into 64-bit pins */
#define AY38910_SET_DATA(p,d) {p=((p&~0xFF0000)|((d&0xFF)<<16));}

/* valid register content bitmasks */
static const uint8_t _ay38910_reg_mask[AY38910_NUM_REGISTERS] = {
    0xFF,       /* AY38910_REG_PERIOD_A_FINE */
    0x0F,       /* AY38910_REG_PERIOD_A_COARSE */
    0xFF,       /* AY38910_REG_PERIOD_B_FINE */
    0x0F,       /* AY38910_REG_PERIOD_B_COARSE */
    0xFF,       /* AY38910_REG_PERIOD_C_FINE */
    0x0F,       /* AY38910_REG_PERIOD_C_COARSE */
    0x1F,       /* AY38910_REG_PERIOD_NOISE */
    0xFF,       /* AY38910_REG_ENABLE */
    0x1F,       /* AY38910_REG_AMP_A (0..3: 4-bit volume, 4: use envelope) */
    0x1F,       /* AY38910_REG_AMP_B (0..3: 4-bit volume, 4: use envelope) */
    0x1F,       /* AY38910_REG_AMP_C (0..3: 4-bit volume, 4: use envelope) */
    0xFF,       /* AY38910_REG_ENV_PERIOD_FINE */
    0xFF,       /* AY38910_REG_ENV_PERIOD_COARSE */
    0x0F,       /* AY38910_REG_ENV_SHAPE_CYCLE */
    0xFF,       /* AY38910_REG_IO_PORT_A */
    0xFF,       /* AY38910_REG_IO_PORT_B */
};

/* volume table from: https://github.com/true-grue/ayumi/blob/master/ayumi.c */
static const float _ay38910_volumes[16] = {
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

/* canned envelope generator shapes */
static const uint8_t _ay38910_shapes[16][32] = {
    /* CONTINUE ATTACK ALTERNATE HOLD */
    /* 0 0 X X */
    { 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    /* 0 1 X X */
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    /* 1 0 0 0 */
    { 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 },
    /* 1 0 0 1 */
    { 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    /* 1 0 1 0 */
    { 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
    /* 1 0 1 1 */
    { 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15 },
    /* 1 1 0 0 */
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
    /* 1 1 0 1 */
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15 },
    /* 1 1 1 0 */
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 },
    /* 1 1 1 1 */
    { 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};

/* update computed values after registers have been reprogrammed */
static void _ay38910_update_values(ay38910_t* ay) {
    for (int i = 0; i < AY38910_NUM_CHANNELS; i++) {
        ay38910_tone_t* chn = &ay->tone[i];
        /* "...Note also that due to the design technique used in the Tone Period
           count-down, the lowest period value is 000000000001 (divide by 1)
           and the highest period value is 111111111111 (divide by 4095)
        */
        chn->period = (ay->reg[2*i+1]<<8)|(ay->reg[2*i]);
        if (0 == chn->period) {
            chn->period = 1;
        }
        /* a set 'enable bit' actually means 'disabled' */
        chn->tone_disable = (ay->enable>>i) & 1;
        chn->noise_disable = (ay->enable>>(3+i)) & 1;
    }
    /* noise generator values */
    ay->noise.period = ay->period_noise;
    if (ay->noise.period == 0) {
        ay->noise.period = 1;
    }
    /* envelope generator values */
    ay->env.period = (ay->period_env_coarse<<8)|ay->period_env_fine;
    if (ay->env.period == 0) {
        ay->env.period = 1;
    }
}

/* reset the env shape generator, only called when env-shape register is updated */
static void _ay38910_restart_env_shape(ay38910_t* ay) {
    ay->env.shape_holding = false;
    ay->env.shape_counter = 0;
    if (!(ay->env_shape_cycle & AY38910_ENV_CONTINUE) || (ay->env_shape_cycle & AY38910_ENV_HOLD)) {
        ay->env.shape_hold = true;
    }
    else {
        ay->env.shape_hold = false;
    }
}

void ay38910_init(ay38910_t* ay, ay38910_desc_t* desc) {
    CHIPS_ASSERT(ay && desc);
    CHIPS_ASSERT(desc->tick_hz > 0);
    CHIPS_ASSERT(desc->sound_hz > 0);
    memset(ay, 0, sizeof(*ay));
    /* note: input and output callbacks are optional */
    ay->in_cb = desc->in_cb;
    ay->out_cb = desc->out_cb;
    ay->user_data = desc->user_data;
    ay->type = desc->type;
    ay->noise.rng = 1;
    ay->sample_period = (desc->tick_hz * AY38910_FIXEDPOINT_SCALE) / desc->sound_hz;
    ay->sample_counter = ay->sample_period;
    ay->mag = desc->magnitude;
    _ay38910_update_values(ay);
    _ay38910_restart_env_shape(ay);
}

void ay38910_reset(ay38910_t* ay) {
    CHIPS_ASSERT(ay);
    ay->addr = 0;
    ay->tick = 0;
    for (int i = 0; i < AY38910_NUM_REGISTERS; i++) {
        ay->reg[i] = 0;
    }
    _ay38910_update_values(ay);
    _ay38910_restart_env_shape(ay);
}

bool ay38910_tick(ay38910_t* ay) {
    ay->tick++;
    if ((ay->tick & 7) == 0) {
        /* tick the tone channels */
        for (int i = 0; i < AY38910_NUM_CHANNELS; i++) {
            ay38910_tone_t* chn = &ay->tone[i];
            if (++chn->counter >= chn->period) {
                chn->counter = 0;
                chn->bit ^= 1;
            }
        }

        /* tick the noise channel */
        if (++ay->noise.counter >= ay->noise.period) {
            ay->noise.counter = 0;
            ay->noise.bit ^= 1;
            if (ay->noise.bit) {
                // random number generator from MAME:
                // https://github.com/mamedev/mame/blob/master/src/devices/sound/ay8910.cpp
                // The Random Number Generator of the 8910 is a 17-bit shift
                // register. The input to the shift register is bit0 XOR bit3
                // (bit0 is the output). This was verified on AY-3-8910 and YM2149 chips.
                ay->noise.rng ^= (((ay->noise.rng & 1) ^ ((ay->noise.rng >> 3) & 1)) << 17);
                ay->noise.rng >>= 1;
            }
        }
    }

    /* tick the envelope generator */
    if ((ay->tick & 15) == 0) {
        if (++ay->env.counter >= ay->env.period) {
            ay->env.counter = 0;
            if (!ay->env.shape_holding) {
                ay->env.shape_counter = (ay->env.shape_counter + 1) & 0x1F;
                if (ay->env.shape_hold && (0x1F == ay->env.shape_counter)) {
                    ay->env.shape_holding = true;
                }
            }
            ay->env.shape_state = _ay38910_shapes[ay->env_shape_cycle][ay->env.shape_counter];
        }
    }

    /* generate new sample? */
    ay->sample_counter -= AY38910_FIXEDPOINT_SCALE;
    if (ay->sample_counter <= 0) {
        ay->sample_counter += ay->sample_period;
        float vol = 0.0f;
        float sm = 0.0f;
        for (int i = 0; i < AY38910_NUM_CHANNELS; i++) {
            const ay38910_tone_t* chn = &ay->tone[i];
            if (0 == (ay->reg[AY38910_REG_AMP_A+i] & (1<<4))) {
                /* fixed amplitude */
                vol = _ay38910_volumes[ay->reg[AY38910_REG_AMP_A+i] & 0x0F];
            }
            else {
                /* envelope control */
                vol = _ay38910_volumes[ay->env.shape_state];
            }
            int vol_enable = (chn->bit|chn->tone_disable) & ((ay->noise.rng&1)|(chn->noise_disable));
            sm += (vol_enable ? vol : -vol);
        }
        ay->sample = sm * ay->mag * 0.33333f;
        return true;    /* new sample is ready */
    }
    /* fallthrough: no new sample ready yet */
    return false;
}

uint64_t ay38910_iorq(ay38910_t* ay, uint64_t pins) {
    if (pins & (AY38910_BDIR|AY38910_BC1)) {
        if (pins & AY38910_BDIR) {
            const uint8_t data = AY38910_DATA(pins);
            if (pins & AY38910_BC1) {
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
                if (ay->addr < AY38910_NUM_REGISTERS) {
                    /* write register content, and update dependent values */
                    ay->reg[ay->addr] = data & _ay38910_reg_mask[ay->addr];
                    _ay38910_update_values(ay);
                    if (ay->addr == AY38910_REG_ENV_SHAPE_CYCLE) {
                        _ay38910_restart_env_shape(ay);
                    }
                    /* Handle port output:

                        If port A or B is in output mode, call the
                        port output callback to notify the outer world
                        about the new register value.

                        input/output mode is defined by bits 6 and 7 of
                        the 'enable' register
                            bit6 = 1: port A in output mode
                            bit7 = 1: port B in output mode
                    */
                    else if (ay->addr == AY38910_REG_IO_PORT_A) {
                        if (ay->enable & (1<<6)) {
                            if (ay->out_cb) {
                                ay->out_cb(AY38910_PORT_A, ay->port_a, ay->user_data);
                            }
                        }
                    }
                    else if (ay->addr == AY38910_REG_IO_PORT_B) {
                        if (ay->enable & (1<<7)) {
                            if (ay->out_cb) {
                                ay->out_cb(AY38910_PORT_B, ay->port_b, ay->user_data);
                            }
                        }
                    }
                }
            }
        }
        else {
            /* Read from register using the currently latched address.
               See 'write' for why the latched address must be in the
               valid register range to have an effect.
            */
            if (ay->addr < AY38910_NUM_REGISTERS) {
                /* Handle port input:

                    If port A or B is in input mode, first call the port
                    input callback to update the port register content.

                    input/output mode is defined by bits 6 and 7 of
                    the 'enable' register:
                        bit6 = 0: port A in input mode
                        bit7 = 0: port B in input mode
                */
                if (ay->addr == AY38910_REG_IO_PORT_A) {
                    if ((ay->enable & (1<<6)) == 0) {
                        if (ay->in_cb) {
                            ay->port_a = ay->in_cb(AY38910_PORT_A, ay->user_data);
                        }
                        else {
                            ay->port_a = 0xFF;
                        }
                    }
                }
                else if (ay->addr == AY38910_REG_IO_PORT_B) {
                    if ((ay->enable & (1<<7)) == 0) {
                        if (ay->in_cb) {
                            ay->port_b = ay->in_cb(AY38910_PORT_B, ay->user_data);
                        }
                        else {
                            ay->port_b = 0xFF;
                        }
                    }
                }
                /* read register content into data pins */
                const uint8_t data = ay->reg[ay->addr];
                AY38910_SET_DATA(pins, data);
            }
        }
    }
    return pins;
}

#endif /* CHIPS_IMPL */

#ifdef __cplusplus
} /* extern "C" */
#endif
