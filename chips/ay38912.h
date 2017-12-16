#pragma once
/*
    ay38912.h   -- AY-3-8912 sound chip emulator

    TODO: docs!
*/
#include <stdint.h>
#include <stdlib.h>

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
#define AY38912_PERIOD_A_FINE       (0)
#define AY38912_PERIOD_A_COARSE     (1)
#define AY38912_PERIOD_B_FINE       (2)
#define AY38912_PERIOD_B_COARSE     (3)
#define AY38912_PERIOD_C_FINE       (4)
#define AY38912_PERIOD_C_COARSE     (5)
#define AY38912_PERIOD_NOISE        (6)
#define AY38912_ENABLE              (7)
#define AY38912_AMP_A               (8)
#define AY38912_AMP_B               (9)
#define AY38912_AMP_C               (10)
#define AY38912_ENV_PERIOD_FINE     (11)
#define AY38912_ENV_PERIOD_COARSE   (12)
#define AY38912_ENV_SHAPE_CYCLE     (13)
#define AY38912_IO_PORT_A           (14)
#define AY38912_IO_PORT_B           (15)    /* not on AY-3-8912 */

/* super-sampling precision */
#define AY38912_SUPER_SAMPLES (2)
/* number of registers */
#define AY38912_NUM_REGISTERS       (16)
/* number of channels */
#define AY38912_NUM_CHANNELS    (3)

/* a tone channel */
typedef struct {
    int period;
    int counter;
    int bit;
    int tone_disable;
    int noise_disable;
} ay38912_tone;

/* the noise channel state */
typedef struct {
    int period;
    int counter;
    int bit;
    int rng;
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

    /* the 3 tone channels */
    ay38912_tone chn[AY38912_NUM_CHANNELS];
    /* the noise generator state */
    ay38912_noise noise;
    /* the envelope generator state */
    ay38912_envelope envelope;

    /* sample generation state */
    int period;
    int tick_counter;
    int super_sample_counter;
    float mag;
    float acc;
    float sample;
} ay38912;

/* initialize a AY-3-8912 instance */
extern void ay38912_init(ay38912* ay, int tick_khz, int sound_hz, float magnitude);
/* reset an existing AY-3-8912 instance */
extern void ay38912_reset(ay38912* ay);
/* perform an IO request machine cycle */
extern uint64_t ay38912_iorq(ay38912* ay, uint64_t pins);
/* tick the AY-3-8912, return true if a new sample is ready */
extern bool ay38912_tick(ay38912* ay);

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

void ay38912_init(ay38912* ay, int tick_khz, int sound)hz, float magnitude) {
    CHIPS_ASSERT(ay);
    CHIPS_ASSERT((cpu_khz > 0) && (sound_hz > 0));
    memset(ay, 0, sizeof(*ay));
    ay->noise.rng = 1;
    ay->period = (tick_khz * 1000) / (sound_hz * AY38912_SUPER_SAMPLES);
    ay->tick_counter = ay->period;
    ay->super_sample_counter = AY38912_SUPER_SAMPLES;
    ay->mag = magnitude;
}

#endif /* CHIPS_IMPL */

#ifdef __cplusplus
} /* extern "C" */
#endif