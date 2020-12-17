#pragma once
/*#
    # namco.h

    Pacman / Pengo arcade machine emulator in a C header.

    MAME used as reference.

    Project repository: https://github.com/floooh/chips

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
        your own assert macro (default: assert(c))

    Before including the implementation, select the hardware configuration
    through a define:

    NAMCO_PACMAN    - configure the emulator as Pacman arcade machine
    NAMCO_PENGO     - configure the emulator as Pengo arcade machine

    You need to include the following headers before including namco.h:

    - chips/z80.h
    - chips/clk.h
    - chips/mem.h

    For an example implementation, see:

    https://github.com/floooh/chips-test/blob/master/examples/sokol/pacman.c
    https://github.com/floooh/chips-test/blob/master/examples/sokol/pengo.c

    ## zlib/libpng license

    Copyright (c) 2019 Andre Weissflog
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

#define NAMCO_MAX_AUDIO_SAMPLES (1024)
#define NAMCO_DEFAULT_AUDIO_SAMPLES (128)

/* input bits (use with namco_input_set() and namco_input_clear()) */
#define NAMCO_INPUT_P1_UP       (1<<0)
#define NAMCO_INPUT_P1_LEFT     (1<<1)
#define NAMCO_INPUT_P1_RIGHT    (1<<2)
#define NAMCO_INPUT_P1_DOWN     (1<<3)
#define NAMCO_INPUT_P1_BUTTON   (1<<4)
#define NAMCO_INPUT_P1_COIN     (1<<5)
#define NAMCO_INPUT_P1_START    (1<<6)
#define NAMCO_INPUT_P2_UP       (1<<7)
#define NAMCO_INPUT_P2_LEFT     (1<<8)
#define NAMCO_INPUT_P2_RIGHT    (1<<9)
#define NAMCO_INPUT_P2_DOWN     (1<<10)
#define NAMCO_INPUT_P2_BUTTON   (1<<11)
#define NAMCO_INPUT_P2_COIN     (1<<12)
#define NAMCO_INPUT_P2_START    (1<<13)

/* audio sample-data callback */
typedef void (*namco_audio_callback_t)(const float* samples, int num_samples, void* user_data);

/* configuration parameters for namco_init() */
typedef struct {
    /* video output config */
    void* pixel_buffer;         /* pointer to a linear RGBA8 pixel buffer, at least 224*256*4 bytes */
    int pixel_buffer_size;      /* size of the pixel buffer in bytes */

    /* optional user-data for audio callback */
    void* user_data;

    /* audio output config (if you don't want audio, set audio_cb to zero) */
    namco_audio_callback_t audio_cb;        /* called when audio_num_samples are ready */
    int audio_num_samples;                  /* default is NAMCO_DEFAULT_AUDIO_SAMPLES */
    int audio_sample_rate;                  /* playback sample rate, default is 44100 */
    float audio_volume;                     /* audio volume, 0.0..1.0, default is 1.0 */

    /* ROM images */
    const void* rom_cpu_0000_0FFF;          /* Pacman+Pengo */
    const void* rom_cpu_1000_1FFF;          /* Pacman+Pengo */
    const void* rom_cpu_2000_2FFF;          /* Pacman+Pengo */
    const void* rom_cpu_3000_3FFF;          /* Pacman+Pengo */
    const void* rom_cpu_4000_4FFF;          /* Pengo only */
    const void* rom_cpu_5000_5FFF;          /* Pengo only */
    const void* rom_cpu_6000_6FFF;          /* Pengo only */
    const void* rom_cpu_7000_7FFF;          /* Pengo only */
    const void* rom_gfx_0000_0FFF;          /* Pacman only */
    const void* rom_gfx_1000_1FFF;          /* Pacman only */
    const void* rom_gfx_0000_1FFF;          /* Pengo only */
    const void* rom_gfx_2000_3FFF;          /* Pengo only */
    const void* rom_prom_0000_001F;         /* Pacman+Pengo */
    const void* rom_prom_0020_011F;         /* Pacman only */
    const void* rom_prom_0020_041F;         /* Pengo only */
    const void* rom_sound_0000_00FF;        /* Pacman+Pengo */
    const void* rom_sound_0100_01FF;        /* Pacman+Pengo */
    int rom_cpu_0000_0FFF_size;
    int rom_cpu_1000_1FFF_size;
    int rom_cpu_2000_2FFF_size;
    int rom_cpu_3000_3FFF_size;
    int rom_cpu_4000_4FFF_size;
    int rom_cpu_5000_5FFF_size;
    int rom_cpu_6000_6FFF_size;
    int rom_cpu_7000_7FFF_size;
    int rom_gfx_0000_0FFF_size;
    int rom_gfx_1000_1FFF_size;
    int rom_gfx_0000_1FFF_size;
    int rom_gfx_2000_3FFF_size;
    int rom_prom_0000_001F_size;
    int rom_prom_0020_011F_size;
    int rom_prom_0020_041F_size;
    int rom_sound_0000_00FF_size;
    int rom_sound_0100_01FF_size;
} namco_desc_t;

/* audio state */
typedef struct {
    int tick_counter;
    int sample_period;
    int sample_counter;
    float volume;
    struct {
        uint32_t frequency; /* 20-bit frequency */
        uint32_t counter;   /* 20-bit counter (top 5 bits are index into 32-byte wave table) */
        uint8_t waveform;   /* 3-bit waveform */
        uint8_t volume;     /* 4-bit volume */
        float sample;       /* accumulated sample value */
        float sample_div  ; /* oversampling divider */
    } voice[3];
    uint8_t rom[2][0x0100]; /* wave table ROM */
    int num_samples;
    int sample_pos;
    namco_audio_callback_t callback;
    float sample_buffer[NAMCO_MAX_AUDIO_SAMPLES];
} namco_sound_t;

/* the Namco arcade machine state */
typedef struct {
    bool valid;
    z80_t cpu;
    clk_t clk;
    uint8_t in0;    /* inverted bits (active-low) */
    uint8_t in1;    /* inverted bits (active-low) */
    uint8_t dsw1;   /* dip-switches as-is (active-high) */
    uint8_t dsw2;   /* Pengo only */
    int vsync_count;
    uint8_t int_vector;     /* IM2 interrupt vector set with OUT on port 0 */
    uint8_t int_enable;
    uint8_t sound_enable;
    uint8_t flip_screen;    /* screen-flip (for cocktail-cabinet) is not implemented */
    uint8_t pal_select;     /* Pengo only */
    uint8_t clut_select;    /* Pengo only */
    uint8_t tile_select;    /* Pengo only */
    uint8_t sprite_coords[16];      /* 8 sprites, uint8_t x, uint8_t y */
    mem_t mem;
    uint32_t* pixel_buffer;
    uint32_t palette_cache[512];    /* precomputed RGBA values, Pacman: 256 entries , Pengo: 512 entries*/
    void* user_data;
    namco_sound_t sound;
    uint8_t video_ram[0x0400];
    uint8_t color_ram[0x0400];
    uint8_t main_ram[0x0800];       /* Pacman: 1 KB, Pengo: 2 KB */
    uint8_t rom_cpu[0x8000];        /* program ROM: Pacman: 16 KB, Pengo: 32 KB */
    uint8_t rom_gfx[0x4000];        /* tile ROM: Pacman: 8 KB, Pengo: 16 KB*/
    uint8_t rom_prom[0x0420];       /* palette and color lookup ROM */
} namco_t;

/* initialize a new namco_t instance */
void namco_init(namco_t* sys, const namco_desc_t* desc);
/* discard a namco_t instance */
void namco_discard(namco_t* sys);
/* reset a namco_t instance */
void namco_reset(namco_t* sys);
/* run namco_t instance for given amount of microseconds */
void namco_exec(namco_t* sys, uint32_t micro_seconds);
/* decode video to pixel buffer, must be called once per frame */
void namco_decode_video(namco_t* sys);
/* set input bits */
void namco_input_set(namco_t* sys, uint32_t mask);
/* clear input bits */
void namco_input_clear(namco_t* sys, uint32_t mask);
/* get the standard framebuffer width and height in pixels */
int namco_std_display_width(void);
int namco_std_display_height(void);
/* get the maximum framebuffer size in number of bytes */
int namco_max_display_size(void);
/* get the current framebuffer width and height in pixels */
int namco_display_width(namco_t* sys);
int namco_display_height(namco_t* sys);

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
#if !defined(NAMCO_PACMAN) && !defined(NAMCO_PENGO)
#error "Please define NAMCO_PACMAN or NAMCO_PENGO before including the implementation"
#endif

#if defined NAMCO_PACMAN
    #define NAMCO_ADDR_MASK         (0x7FFF)    /* Pacman has only 15 addr pins wired */
    #define NAMCO_IOMAP_BASE        (0x5000)
    #define NAMCO_ADDR_SPRITES_ATTR (0x03F0)    /* offset in main_ram */
    /* IN0 bits (active low) */
    #define NAMCO_IN0_UP            (1<<0)
    #define NAMCO_IN0_LEFT          (1<<1)
    #define NAMCO_IN0_RIGHT         (1<<2)
    #define NAMCO_IN0_DOWN          (1<<3)
    #define NAMCO_IN0_RACK_ADVANCE  (1<<4)
    #define NAMCO_IN0_COIN1         (1<<5)
    #define NAMCO_IN0_COIN2         (1<<6)
    #define NAMCO_IN0_CREDIT        (1<<7)
    /* IN1 bits (active low) */
    #define NAMCO_IN1_UP            (1<<0)
    #define NAMCO_IN1_LEFT          (1<<1)
    #define NAMCO_IN1_RIGHT         (1<<2)
    #define NAMCO_IN1_DOWN          (1<<3)
    #define NAMCO_IN1_BOARD_TEST    (1<<4)
    #define NAMCO_IN1_P1_START      (1<<5)
    #define NAMCO_IN1_P2_START      (1<<6)
    /* DSW1 bits (active high) */
    #define NAMCO_DSW1_COINS_MASK       (3<<0)
    #define NAMCO_DSW1_COINS_FREE       (0)     /* free play */
    #define NAMCO_DSW1_COINS_1C1G       (1<<0)  /* 1 coin 1 game */
    #define NAMCO_DSW1_COINS_1C2G       (2<<0)  /* 1 coin 2 games */
    #define NAMCO_DSW1_COINS_2C1G       (3<<0)  /* 2 coins 1 game */
    #define NAMCO_DSW1_LIVES_MASK       (3<<2)
    #define NAMCO_DSW1_LIVES_1          (0<<2)
    #define NAMCO_DSW1_LIVES_2          (1<<2)
    #define NAMCO_DSW1_LIVES_3          (2<<2)
    #define NAMCO_DSW1_LIVES_5          (3<<2)
    #define NAMCO_DSW1_EXTRALIFE_MASK   (3<<4)
    #define NAMCO_DSW1_EXTRALIFE_10K    (0<<4)
    #define NAMCO_DSW1_EXTRALIFE_15K    (1<<4)
    #define NAMCO_DSW1_EXTRALIFE_20K    (2<<4)
    #define NAMCO_DSW1_EXTRALIFE_NONE   (3<<4)
    #define NAMCO_DSW1_DIFFICULTY_MASK  (1<<6)
    #define NAMCO_DSW1_DIFFICULTY_HARD  (0<<6)
    #define NAMCO_DSW1_DIFFICULTY_NORM  (1<<6)
    #define NAMCO_DSW1_GHOSTNAMES_MASK  (1<<7)
    #define NAMCO_DSW1_GHOSTNAMES_ALT   (0<<7)
    #define NAMCO_DSW1_GHOSTNAMES_NORM  (1<<7)
    #define NAMCO_DSW1_DEFAULT (NAMCO_DSW1_COINS_1C1G|NAMCO_DSW1_LIVES_3|NAMCO_DSW1_EXTRALIFE_15K|NAMCO_DSW1_DIFFICULTY_NORM|NAMCO_DSW1_GHOSTNAMES_NORM)
    /* memory mapped IO: read locations*/
    #define NAMCO_ADDR_IN0          (0x5000)
    #define NAMCO_ADDR_IN1          (0x5040)
    #define NAMCO_ADDR_DSW1         (0x5080)
    /* memory mapped IO: write locations */
    #define NAMCO_ADDR_INT_ENABLE       (0x5000)
    #define NAMCO_ADDR_SOUND_ENABLE     (0x5001)
    #define NAMCO_ADDR_FLIP_SCREEN      (0x5003)
    #define NAMCO_ADDR_SOUND_BASE       (0x5040)
    #define NAMCO_ADDR_SPRITES_COORD    (0x5060)
#else /* PENGO */
    #define NAMCO_ADDR_MASK         (0xFFFF)        /* Pengo has 16 address lines */
    #define NAMCO_IOMAP_BASE        (0x9000)
    #define NAMCO_ADDR_SPRITES_ATTR (0x07F0)        /* offset in main_ram */
    /* IN0 bits (active low) */
    #define NAMCO_IN0_UP            (1<<0)
    #define NAMCO_IN0_DOWN          (1<<1)
    #define NAMCO_IN0_LEFT          (1<<2)
    #define NAMCO_IN0_RIGHT         (1<<3)
    #define NAMCO_IN0_COIN1         (1<<4)
    #define NAMCO_IN0_COIN2         (1<<5)
    #define NAMCO_IN0_COIN3         (1<<6)          /* aka coin-aux, not supported */
    #define NAMCO_IN0_BUTTON        (1<<7)
    /* IN1 bits (active low) */
    #define NAMCO_IN1_UP            (1<<0)
    #define NAMCO_IN1_DOWN          (1<<1)
    #define NAMCO_IN1_LEFT          (1<<2)
    #define NAMCO_IN1_RIGHT         (1<<3)
    #define NAMCO_IN1_BOARD_TEST    (1<<4)
    #define NAMCO_IN1_P1_START      (1<<5)
    #define NAMCO_IN1_P2_START      (1<<6)
    #define NAMCO_IN1_BUTTON        (1<<7)
    /* DSW1 bits (active high) */
    #define NAMCO_DSW1_EXTRALIFE_MASK       (1<<0)
    #define NAMCO_DSW1_EXTRALIFE_30K        (0<<0)
    #define NAMCO_DSW1_EXTRALIFE_50K        (1<<0)
    #define NAMCO_DSW1_DEMOSOUND_MASK       (1<<1)
    #define NAMCO_DSW1_DEMOSOUND_ON         (0<<1)
    #define NAMCO_DSW1_DEMOSOUND_OFF        (1<<1)
    #define NAMCO_DSW1_CABINET_MASK         (1<<2)
    #define NAMCO_DSW1_CABINET_UPRIGHT      (0<<2)
    #define NAMCO_DSW1_CABINET_COCKTAIL     (1<<2)
    #define NAMCO_DSW1_LIVES_MASK           (3<<3)
    #define NAMCO_DSW1_LIVES_2              (0<<3)
    #define NAMCO_DSW1_LIVES_3              (1<<3)
    #define NAMCO_DSW1_LIVES_4              (2<<3)
    #define NAMCO_DSW1_LIVES_5              (3<<3)
    #define NAMCO_DSW1_RACKTEST_MASK        (1<<5)
    #define NAMCO_DSW1_RACKTEST_ON          (0<<5)
    #define NAMCO_DSW1_RACKTEST_OFF         (1<<5)
    #define NAMCO_DSW1_DIFFICULTY_MASK      (3<<6)
    #define NAMCO_DSW1_DIFFICULTY_EASY      (0<<6)
    #define NAMCO_DSW1_DIFFICULTY_MEDIUM    (1<<6)
    #define NAMCO_DSW1_DIFFICULTY_HARD      (2<<6)
    #define NAMCO_DSW1_DIFFICULTY_HARDEST   (3<<6)
    /* DSW2 bits (active high) */
    #define NAMCO_DSW2_COINA_MASK           (0xF<<0)    /* 16 combinations of N coins -> M games */
    #define NAMCO_DSW2_COINA_1C1G           (0xC<<0)
    #define NAMCO_DSW2_COINB_MASK           (0xF<<4)
    #define NAMCO_DSW2_COINB_1C1G           (0xC<<4)
    #define NAMCO_DSW1_DEFAULT (NAMCO_DSW1_EXTRALIFE_30K|NAMCO_DSW1_DEMOSOUND_ON|NAMCO_DSW1_CABINET_UPRIGHT|NAMCO_DSW1_LIVES_3|NAMCO_DSW1_RACKTEST_OFF|NAMCO_DSW1_DIFFICULTY_MEDIUM)
    #define NAMCO_DSW2_DEFAULT (NAMCO_DSW2_COINA_1C1G|NAMCO_DSW2_COINB_1C1G)
    /* memory mapped IO: read locations*/
    #define NAMCO_ADDR_IN0              (0x90C0)
    #define NAMCO_ADDR_IN1              (0x9080)
    #define NAMCO_ADDR_DSW1             (0x9040)
    #define NAMCO_ADDR_DSW2             (0x9000)
    /* memory mapped IO: write locations */
    #define NAMCO_ADDR_SPRITES_COORD    (0x9020)
    #define NAMCO_ADDR_INT_ENABLE       (0x9040)
    #define NAMCO_ADDR_SOUND_ENABLE     (0x9041)
    #define NAMCO_ADDR_PAL_SELECT       (0x9042)
    #define NAMCO_ADDR_FLIP_SCREEN      (0x9043)
    #define NAMCO_ADDR_CLUT_SELECT      (0x9046)
    #define NAMCO_ADDR_TILE_SELECT      (0x9047)
    #define NAMCO_ADDR_SOUND_BASE       (0x9000)
    #define NAMCO_ADDR_WATCHDOG         (0x9070)
#endif

/* sound registers */
#define NAMCO_ADDR_SOUND_V1_FC0     (NAMCO_ADDR_SOUND_BASE+0x00)    /* voice1 frequency counter nibble 0 */
#define NAMCO_ADDR_SOUND_V1_FC1     (NAMCO_ADDR_SOUND_BASE+0x01)    /*                          nibble 1 */
#define NAMCO_ADDR_SOUND_V1_FC2     (NAMCO_ADDR_SOUND_BASE+0x02)    /*                          nibble 2 */
#define NAMCO_ADDR_SOUND_V1_FC3     (NAMCO_ADDR_SOUND_BASE+0x03)    /*                          nibble 3 */
#define NAMCO_ADDR_SOUND_V1_FC4     (NAMCO_ADDR_SOUND_BASE+0x04)    /*                          nibble 4 */
#define NAMCO_ADDR_SOUND_V1_WAVE    (NAMCO_ADDR_SOUND_BASE+0x05)    /* voice1 wave form */
#define NAMCO_ADDR_SOUND_V2_FC1     (NAMCO_ADDR_SOUND_BASE+0x06)    /* voice2 frequency counter nibble 1 */
#define NAMCO_ADDR_SOUND_V2_FC2     (NAMCO_ADDR_SOUND_BASE+0x07)    /*                          nibble 2 */
#define NAMCO_ADDR_SOUND_V2_FC3     (NAMCO_ADDR_SOUND_BASE+0x08)    /*                          nibble 3 */
#define NAMCO_ADDR_SOUND_V2_FC4     (NAMCO_ADDR_SOUND_BASE+0x09)    /*                          nibble 4 */
#define NAMCO_ADDR_SOUND_V2_WAVE    (NAMCO_ADDR_SOUND_BASE+0x0A)    /* voice2 wave form */
#define NAMCO_ADDR_SOUND_V3_FC1     (NAMCO_ADDR_SOUND_BASE+0x0B)    /* voice3 frequency counter nibble 1 */
#define NAMCO_ADDR_SOUND_V3_FC2     (NAMCO_ADDR_SOUND_BASE+0x0C)    /*                          nibble 2 */
#define NAMCO_ADDR_SOUND_V3_FC3     (NAMCO_ADDR_SOUND_BASE+0x0D)    /*                          nibble 3 */
#define NAMCO_ADDR_SOUND_V3_FC4     (NAMCO_ADDR_SOUND_BASE+0x0E)    /*                          nibble 4 */
#define NAMCO_ADDR_SOUND_V3_WAVE    (NAMCO_ADDR_SOUND_BASE+0x0F)    /* voice3 wave form */
#define NAMCO_ADDR_SOUND_V1_FQ0     (NAMCO_ADDR_SOUND_BASE+0x10)    /* voice1 frequency nibble 0 */
#define NAMCO_ADDR_SOUND_V1_FQ1     (NAMCO_ADDR_SOUND_BASE+0x11)    /*                  nibble 1 */
#define NAMCO_ADDR_SOUND_V1_FQ2     (NAMCO_ADDR_SOUND_BASE+0x12)    /*                  nibble 2 */
#define NAMCO_ADDR_SOUND_V1_FQ3     (NAMCO_ADDR_SOUND_BASE+0x13)    /*                  nibble 3 */
#define NAMCO_ADDR_SOUND_V1_FQ4     (NAMCO_ADDR_SOUND_BASE+0x14)    /*                  nibble 4 */
#define NAMCO_ADDR_SOUND_V1_VOLUME  (NAMCO_ADDR_SOUND_BASE+0x15)    /* voice1 volume */
#define NAMCO_ADDR_SOUND_V2_FQ1     (NAMCO_ADDR_SOUND_BASE+0x16)    /* voice2 frequency nibble 1 */
#define NAMCO_ADDR_SOUND_V2_FQ2     (NAMCO_ADDR_SOUND_BASE+0x17)    /*                  nibble 2 */
#define NAMCO_ADDR_SOUND_V2_FQ3     (NAMCO_ADDR_SOUND_BASE+0x18)    /*                  nibble 3 */
#define NAMCO_ADDR_SOUND_V2_FQ4     (NAMCO_ADDR_SOUND_BASE+0x19)    /*                  nibble 4 */
#define NAMCO_ADDR_SOUND_V2_VOLUME  (NAMCO_ADDR_SOUND_BASE+0x1A)    /* voice2 volume */
#define NAMCO_ADDR_SOUND_V3_FQ1     (NAMCO_ADDR_SOUND_BASE+0x1B)    /* voice3 frequency nibble 1 */
#define NAMCO_ADDR_SOUND_V3_FQ2     (NAMCO_ADDR_SOUND_BASE+0x1C)    /*                  nibble 2 */
#define NAMCO_ADDR_SOUND_V3_FQ3     (NAMCO_ADDR_SOUND_BASE+0x1D)    /*                  nibble 3 */
#define NAMCO_ADDR_SOUND_V3_FQ4     (NAMCO_ADDR_SOUND_BASE+0x1E)    /*                  nibble 4 */
#define NAMCO_ADDR_SOUND_V3_VOLUME  (NAMCO_ADDR_SOUND_BASE+0x1F)    /* voice3 volume */

#define NAMCO_MASTER_CLOCK      (18432000)
#define NAMCO_CPU_CLOCK         (NAMCO_MASTER_CLOCK / 6)
#define NAMCO_SOUND_PERIOD      (32)    /* sound is ticked every 32 CPU ticks */
#define NAMCO_SOUND_OVERSAMPLE  (2)
#define NAMCO_SAMPLE_SCALE      (16)
#define NAMCO_VSYNC_PERIOD      (NAMCO_CPU_CLOCK / 60)
#define NAMCO_DISPLAY_WIDTH     (288)
#define NAMCO_DISPLAY_HEIGHT    (224)
#define NAMCO_DISPLAY_SIZE      (NAMCO_DISPLAY_WIDTH*NAMCO_DISPLAY_HEIGHT*4)

static uint64_t _namco_tick(int num, uint64_t pins, void* user_data);
static void _namco_sound_init(namco_t* sys, const namco_desc_t* desc);
static void _namco_sound_wr(namco_t* sys, uint16_t addr, uint8_t data);
static void _namco_sound_tick(namco_t* sys, int num_ticks);

#define _namco_def(val, def) (val == 0 ? def : val)

void namco_init(namco_t* sys, const namco_desc_t* desc) {
    CHIPS_ASSERT(sys && desc);
    CHIPS_ASSERT(desc->audio_sample_rate > 0);

    memset(sys, 0, sizeof(namco_t));
    sys->valid = true;

    /* audio and video output */
    sys->user_data = desc->user_data;
    _namco_sound_init(sys, desc);
    CHIPS_ASSERT((0 == desc->pixel_buffer) || (desc->pixel_buffer && (desc->pixel_buffer_size >= NAMCO_DISPLAY_SIZE)));
    sys->pixel_buffer = (uint32_t*) desc->pixel_buffer;
    
    /* copy over ROM images */
    CHIPS_ASSERT(desc->rom_cpu_0000_0FFF && (desc->rom_cpu_0000_0FFF_size == 0x1000));
    CHIPS_ASSERT(desc->rom_cpu_1000_1FFF && (desc->rom_cpu_1000_1FFF_size == 0x1000));
    CHIPS_ASSERT(desc->rom_cpu_2000_2FFF && (desc->rom_cpu_2000_2FFF_size == 0x1000));
    CHIPS_ASSERT(desc->rom_cpu_3000_3FFF && (desc->rom_cpu_3000_3FFF_size == 0x1000));
    #if defined(NAMCO_PENGO)
    CHIPS_ASSERT(desc->rom_cpu_4000_4FFF && (desc->rom_cpu_4000_4FFF_size == 0x1000));
    CHIPS_ASSERT(desc->rom_cpu_5000_5FFF && (desc->rom_cpu_5000_5FFF_size == 0x1000));
    CHIPS_ASSERT(desc->rom_cpu_6000_6FFF && (desc->rom_cpu_6000_6FFF_size == 0x1000));
    CHIPS_ASSERT(desc->rom_cpu_7000_7FFF && (desc->rom_cpu_7000_7FFF_size == 0x1000));
    #endif
    #if defined(NAMCO_PACMAN)
    CHIPS_ASSERT(desc->rom_gfx_0000_0FFF && (desc->rom_gfx_0000_0FFF_size == 0x1000));
    CHIPS_ASSERT(desc->rom_gfx_1000_1FFF && (desc->rom_gfx_1000_1FFF_size == 0x1000));
    #else
    CHIPS_ASSERT(desc->rom_gfx_0000_1FFF && (desc->rom_gfx_0000_1FFF_size == 0x2000));
    CHIPS_ASSERT(desc->rom_gfx_2000_3FFF && (desc->rom_gfx_2000_3FFF_size == 0x2000));
    #endif
    CHIPS_ASSERT(desc->rom_prom_0000_001F && (desc->rom_prom_0000_001F_size == 0x0020));
    #if defined(NAMCO_PACMAN)
    CHIPS_ASSERT(desc->rom_prom_0020_011F && (desc->rom_prom_0020_011F_size == 0x0100));
    #else
    CHIPS_ASSERT(desc->rom_prom_0020_041F && (desc->rom_prom_0020_041F_size == 0x0400));
    #endif
    CHIPS_ASSERT(desc->rom_sound_0000_00FF && (desc->rom_sound_0000_00FF_size == 0x0100));
    CHIPS_ASSERT(desc->rom_sound_0100_01FF && (desc->rom_sound_0100_01FF_size == 0x0100));
    memcpy(&sys->rom_cpu[0x0000], desc->rom_cpu_0000_0FFF, 0x1000);
    memcpy(&sys->rom_cpu[0x1000], desc->rom_cpu_1000_1FFF, 0x1000);
    memcpy(&sys->rom_cpu[0x2000], desc->rom_cpu_2000_2FFF, 0x1000);
    memcpy(&sys->rom_cpu[0x3000], desc->rom_cpu_3000_3FFF, 0x1000);
    #if defined(NAMCO_PENGO)
    memcpy(&sys->rom_cpu[0x4000], desc->rom_cpu_4000_4FFF, 0x1000);
    memcpy(&sys->rom_cpu[0x5000], desc->rom_cpu_5000_5FFF, 0x1000);
    memcpy(&sys->rom_cpu[0x6000], desc->rom_cpu_6000_6FFF, 0x1000);
    memcpy(&sys->rom_cpu[0x7000], desc->rom_cpu_7000_7FFF, 0x1000);
    #endif
    #if defined(NAMCO_PACMAN)
    memcpy(&sys->rom_gfx[0x0000], desc->rom_gfx_0000_0FFF, 0x1000);
    memcpy(&sys->rom_gfx[0x1000], desc->rom_gfx_1000_1FFF, 0x1000);
    #else
    memcpy(&sys->rom_gfx[0x0000], desc->rom_gfx_0000_1FFF, 0x2000);
    memcpy(&sys->rom_gfx[0x2000], desc->rom_gfx_2000_3FFF, 0x2000);
    #endif
    memcpy(&sys->rom_prom[0], desc->rom_prom_0000_001F, 0x0020);
    #if defined(NAMCO_PACMAN)
    memcpy(&sys->rom_prom[0x0020], desc->rom_prom_0020_011F, 0x0100);
    #else
    memcpy(&sys->rom_prom[0x0020], desc->rom_prom_0020_041F, 0x0400);
    #endif
    memcpy(sys->sound.rom[0], desc->rom_sound_0000_00FF, 0x0100);
    memcpy(sys->sound.rom[1], desc->rom_sound_0100_01FF, 0x0100);

    /* vsync/vblank counters */
    sys->vsync_count = NAMCO_VSYNC_PERIOD;

    /* system clock and CPU */
    clk_init(&sys->clk, NAMCO_CPU_CLOCK);
    z80_desc_t cpu_desc;
    memset(&cpu_desc, 0, sizeof(cpu_desc));
    cpu_desc.tick_cb = _namco_tick;
    cpu_desc.user_data = sys;
    z80_init(&sys->cpu, &cpu_desc);

    /* memory mapped IO config */
    sys->in0 = 0x00;
    sys->in1 = 0x00;
    sys->dsw1 = NAMCO_DSW1_DEFAULT;
    #if defined(NAMCO_PENGO)
    sys->dsw2 = NAMCO_DSW2_DEFAULT;
    #endif

    /* memory map:

        Pacman: only 15 address bits used, mirroring will happen in tick callback

        0000..3FFF:     16KB ROM
        4000..43FF:     1KB video RAM
        4400..47FF:     1KB color RAM
        4800..4C00:     unmapped?
        4C00..4FEF:     <1KB main RAM
        4FF0..4FFF:     sprite attributes (write only?)

        5000            write:  interrupt enable/disable
                        read:   IN0 (joystick + coin slot)
        5001            write:  sound enable
        5002            ???
        5003            write:  flip screen
        5004            write:  player 1 start light (ignored)
        5005            write:  player 2 start light (ignored)
        5006            write:  coin lockout (ignored)
        5007            write:  coin counter (ignored)
        5040..505F      write:  sound registers
        5040            read:   IN1 (joystick + coin slot)
        5060..506F      write:  sprite coordinates
        5080            read:   DIP switched

        Pengo: full 64KB address space

        0000..7FFF:     32KB ROM
        8000..83FF:     1KB video RAM
        8400..87FF:     1KB color RAM
        8800..8FEF:     2KB main RAM
        9000+           memory mapped registers

    */
    mem_init(&sys->mem);
    mem_map_rom(&sys->mem, 0, 0x0000, 0x1000, &sys->rom_cpu[0x0000]);
    mem_map_rom(&sys->mem, 0, 0x1000, 0x1000, &sys->rom_cpu[0x1000]);
    mem_map_rom(&sys->mem, 0, 0x2000, 0x1000, &sys->rom_cpu[0x2000]);
    mem_map_rom(&sys->mem, 0, 0x3000, 0x1000, &sys->rom_cpu[0x3000]);
    #if defined(NAMCO_PACMAN)
        mem_map_ram(&sys->mem, 0, 0x4000, 0x0400, sys->video_ram);
        mem_map_ram(&sys->mem, 0, 0x4400, 0x0400, sys->color_ram);
        mem_map_ram(&sys->mem, 0, 0x4C00, 0x0400, sys->main_ram);
    #endif
    #if defined(NAMCO_PENGO)
        mem_map_rom(&sys->mem, 0, 0x4000, 0x1000, &sys->rom_cpu[0x4000]);
        mem_map_rom(&sys->mem, 0, 0x5000, 0x1000, &sys->rom_cpu[0x5000]);
        mem_map_rom(&sys->mem, 0, 0x6000, 0x1000, &sys->rom_cpu[0x6000]);
        mem_map_rom(&sys->mem, 0, 0x7000, 0x1000, &sys->rom_cpu[0x7000]);
        mem_map_ram(&sys->mem, 0, 0x8000, 0x0400, sys->video_ram);
        mem_map_ram(&sys->mem, 0, 0x8400, 0x0400, sys->color_ram);
        mem_map_ram(&sys->mem, 0, 0x8800, 0x0800, sys->main_ram);
    #endif

    /* setup an RGBA palette from the 8-bit RGB values in PROM */
    uint32_t hw_colors[32];
    for (int i = 0; i < 32; i++) {
        /*
           Each color ROM entry describes an RGB color in 1 byte:

           | 7| 6| 5| 4| 3| 2| 1| 0|
           |B1|B0|G2|G1|G0|R2|R1|R0|

           Intensities are: 0x97 + 0x47 + 0x21
        */
        uint8_t rgb = sys->rom_prom[i];
        uint8_t r = ((rgb>>0)&1) * 0x21 + ((rgb>>1)&1) * 0x47 + ((rgb>>2)&1) * 0x97;
        uint8_t g = ((rgb>>3)&1) * 0x21 + ((rgb>>4)&1) * 0x47 + ((rgb>>5)&1) * 0x97;
        uint8_t b = ((rgb>>6)&1) * 0x47 + ((rgb>>7)&1) * 0x97;
        hw_colors[i] = 0xFF000000 | (b<<16) | (g<<8) | r;
    }
    for (int i = 0; i < 256; i++) {
        uint8_t pal_index = sys->rom_prom[i + 0x20] & 0xF;
        sys->palette_cache[i] = hw_colors[pal_index];
        sys->palette_cache[256 + i] = hw_colors[0x10 | pal_index];
    }
}

void namco_discard(namco_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    sys->valid = false;
}

void namco_reset(namco_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    z80_reset(&sys->cpu);
}

void namco_exec(namco_t* sys, uint32_t micro_seconds) {
    CHIPS_ASSERT(sys && sys->valid);
    uint32_t ticks_to_run = clk_ticks_to_run(&sys->clk, micro_seconds);
    uint32_t ticks_executed = z80_exec(&sys->cpu, ticks_to_run);
    clk_ticks_executed(&sys->clk, ticks_executed);
}

static uint64_t _namco_tick(int num_ticks, uint64_t pins, void* user_data) {
    namco_t* sys = (namco_t*) user_data;

    /* update the vsync counter and trigger VSYNC interrupt*/
    sys->vsync_count -= num_ticks;
    if (sys->vsync_count < 0) {
        sys->vsync_count += NAMCO_VSYNC_PERIOD;
        if (sys->int_enable) {
            pins |= Z80_INT;
        }
    }

    /* tick the sound chip */
    _namco_sound_tick(sys, num_ticks);

    /* memory requests */
    uint16_t addr = Z80_GET_ADDR(pins) & NAMCO_ADDR_MASK;
    if (pins & Z80_MREQ) {
        if (pins & Z80_WR) {
            /* memory write access */
            uint8_t data = Z80_GET_DATA(pins);
            if (addr < NAMCO_IOMAP_BASE) {
                mem_wr(&sys->mem, addr, data);
            }
            else {
                /* memory-mapped IO */
                if (addr == NAMCO_ADDR_INT_ENABLE) {
                    sys->int_enable = data & 1;
                }
                else if (addr == NAMCO_ADDR_SOUND_ENABLE) {
                    sys->sound_enable = data & 1;
                }
                else if (addr == NAMCO_ADDR_FLIP_SCREEN) {
                    sys->flip_screen = data & 1;
                }
                #if defined(NAMCO_PENGO)
                else if (addr == NAMCO_ADDR_PAL_SELECT) {
                    sys->pal_select = data & 1;
                }
                else if (addr == NAMCO_ADDR_CLUT_SELECT) {
                    sys->clut_select = data & 1;
                }
                else if (addr == NAMCO_ADDR_TILE_SELECT) {
                    sys->tile_select = data & 1;
                }
                #endif
                else if ((addr >= NAMCO_ADDR_SOUND_BASE) && (addr < (NAMCO_ADDR_SOUND_BASE+0x20))) {
                    _namco_sound_wr(sys, addr, data);
                }
                else if ((addr >= NAMCO_ADDR_SPRITES_COORD) && (addr < (NAMCO_ADDR_SPRITES_COORD+0x10))) {
                    sys->sprite_coords[addr & 0xF] = data;
                }
            }
        }
        else if (pins & Z80_RD) {
            /* memory read access */
            if (addr < NAMCO_IOMAP_BASE) {
                Z80_SET_DATA(pins, mem_rd(&sys->mem, addr));
            }
            else {
                uint8_t data = 0xFF;
                /* memory-mapped IO */
                switch (addr) {
                    /* FIXME: IN0, IN1, DSW1 are mirrored for 0x40 bytes */
                    case NAMCO_ADDR_IN0:
                        data = ~sys->in0;
                        break;
                    case NAMCO_ADDR_IN1:
                        data = ~sys->in1;
                        break;
                    case NAMCO_ADDR_DSW1:
                        data = sys->dsw1;
                        break;
                    #if defined(NAMCO_PENGO)
                    case NAMCO_ADDR_DSW2:   /* Pengo only */
                        data = sys->dsw2;
                        break;
                    #endif
                    default:
                        break;
                }
                Z80_SET_DATA(pins, data);
            }
        }
    }
    else if (pins & Z80_IORQ) {
        if (pins & Z80_WR) {
            uint8_t data = Z80_GET_DATA(pins);
            if ((addr & 0xFF) == 0) {
                /* OUT to port 0: set interrupt vector */
                sys->int_vector = data;
            }
        }
        else if (pins & Z80_M1) {
            /* an interrupt handling machine cycle, set interrupt vector on data bus */
            Z80_SET_DATA(pins, sys->int_vector);
        }
    }
    return pins & Z80_PIN_MASK;
}

/* get video memory offset from x/y coords:
    https://www.walkofmind.com/programming/pie/video_memory.htm
*/
static uint16_t _namco_video_offset(uint32_t x, uint32_t y) {
    uint16_t offset = 0;
    x -= 2;
    y += 2;
    if (x & 0x20) {
        offset = y + ((x & 0x1F)<<5);
    }
    else {
        offset = x + (y<<5);
    }
    return offset;
}

/* 8x4 video tile decoder (used both for background tiles and sprites) */
static inline void _namco_8x4(
    uint32_t* pixel_base,
    uint8_t* tile_base,
    uint32_t* palette_base,
    uint32_t tile_stride,
    uint32_t tile_offset,
    uint32_t px,
    uint32_t py,
    uint8_t char_code,
    uint8_t color_code,
    bool opaque,
    bool flip_x,
    bool flip_y)
{
    uint32_t xor_x = flip_x ? 3 : 0;
    uint32_t xor_y = flip_y ? 7 : 0;
    for (uint32_t yy = 0; yy < 8; yy++) {
        uint32_t y = py + (yy ^ xor_y);
        if (y >= NAMCO_DISPLAY_HEIGHT) {
            continue;
        }
        int tile_index = char_code*tile_stride + tile_offset + yy;
        for (uint32_t xx = 0; xx < 4; xx++) {
            uint32_t x = px + (xx ^ xor_x);
            if (x >= NAMCO_DISPLAY_WIDTH) {
                continue;
            }
            uint8_t p2_hi = (tile_base[tile_index]>>(7-xx)) & 1;
            uint8_t p2_lo = (tile_base[tile_index]>>(3-xx)) & 1;
            uint8_t p2 = (p2_hi<<1)|p2_lo;
            uint32_t rgba = palette_base[(color_code<<2)|p2];
            if (opaque || (rgba != 0xFF000000)) {
                uint32_t* dst = &pixel_base[y*NAMCO_DISPLAY_WIDTH + x];
                *dst = rgba;
            }
        }
    }
}

/* decode background tiles */
static void _namco_decode_chars(namco_t* sys) {
    uint32_t* pixel_base = sys->pixel_buffer;
    uint32_t* pal_base = &sys->palette_cache[(sys->pal_select<<8)|(sys->clut_select<<7)];
    uint8_t* tile_base = &sys->rom_gfx[0x0000] + (sys->tile_select * 0x2000);
    for (uint32_t y = 0; y < 28; y++) {
        for (uint32_t x = 0; x < 36; x++) {
            uint16_t offset = _namco_video_offset(x, y);
            uint8_t char_code = sys->video_ram[offset];
            uint8_t color_code = sys->color_ram[offset] & 0x1F;
            _namco_8x4(pixel_base, tile_base, pal_base, 16, 8, x*8, y*8, char_code, color_code, true, false, false);
            _namco_8x4(pixel_base, tile_base, pal_base, 16, 0, x*8+4, y*8, char_code, color_code, true, false, false);
        }
    }
}

static void _namco_decode_sprites(namco_t* sys) {
    uint32_t* pixel_base = sys->pixel_buffer;
    uint32_t* pal_base = &sys->palette_cache[(sys->pal_select<<8)|(sys->clut_select<<7)];
    uint8_t* tile_base = &sys->rom_gfx[0x1000] + (sys->tile_select * 0x2000);
    #if defined(NAMCO_PACMAN)
    const int max_sprite = 6;
    const int min_sprite = 1;
    #else
    const int max_sprite = 7;
    const int min_sprite = 0;
    #endif
    for (int sprite_index = max_sprite; sprite_index >= min_sprite; --sprite_index) {
        uint32_t py = sys->sprite_coords[sprite_index*2 + 0] - 31;
        uint32_t px = 272 - sys->sprite_coords[sprite_index*2 + 1];
        uint8_t shape = sys->main_ram[NAMCO_ADDR_SPRITES_ATTR + sprite_index*2 + 0];
        uint8_t char_code = shape>>2;
        uint8_t color_code = sys->main_ram[NAMCO_ADDR_SPRITES_ATTR + sprite_index*2 + 1];
        bool flip_x = shape & 1;
        bool flip_y = shape & 2;
        uint32_t fy0 = flip_y ? 8 : 0;
        uint32_t fy1 = flip_y ? 0 : 8;
        uint32_t fx0 = flip_x ? 12: 0;
        uint32_t fx1 = flip_x ? 8 : 4;
        uint32_t fx2 = flip_x ? 4 : 8;
        uint32_t fx3 = flip_x ? 0 :12;
        _namco_8x4(pixel_base, tile_base, pal_base, 64, 8,  px+fx0, py+fy0, char_code, color_code, false, flip_x, flip_y);
        _namco_8x4(pixel_base, tile_base, pal_base, 64, 16, px+fx1, py+fy0, char_code, color_code, false, flip_x, flip_y);
        _namco_8x4(pixel_base, tile_base, pal_base, 64, 24, px+fx2, py+fy0, char_code, color_code, false, flip_x, flip_y);
        _namco_8x4(pixel_base, tile_base, pal_base, 64, 0,  px+fx3, py+fy0, char_code, color_code, false, flip_x, flip_y);
        _namco_8x4(pixel_base, tile_base, pal_base, 64, 40, px+fx0, py+fy1, char_code, color_code, false, flip_x, flip_y);
        _namco_8x4(pixel_base, tile_base, pal_base, 64, 48, px+fx1, py+fy1, char_code, color_code, false, flip_x, flip_y);
        _namco_8x4(pixel_base, tile_base, pal_base, 64, 56, px+fx2, py+fy1, char_code, color_code, false, flip_x, flip_y);
        _namco_8x4(pixel_base, tile_base, pal_base, 64, 32, px+fx3, py+fy1, char_code, color_code, false, flip_x, flip_y);
    }
}

void namco_decode_video(namco_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    if (sys->pixel_buffer) {
        _namco_decode_chars(sys);
        _namco_decode_sprites(sys);
    }
}

void namco_input_set(namco_t* sys, uint32_t mask) {
    CHIPS_ASSERT(sys && sys->valid);
    if (mask & NAMCO_INPUT_P1_UP) {
        sys->in0 |= NAMCO_IN0_UP;
    }
    if (mask & NAMCO_INPUT_P1_LEFT) {
        sys->in0 |= NAMCO_IN0_LEFT;
    }
    if (mask & NAMCO_INPUT_P1_RIGHT) {
        sys->in0 |= NAMCO_IN0_RIGHT;
    }
    if (mask & NAMCO_INPUT_P1_DOWN) {
        sys->in0 |= NAMCO_IN0_DOWN;
    }
    #if defined(NAMCO_PENGO)
    if (mask & NAMCO_INPUT_P1_BUTTON) {
        sys->in0 |= NAMCO_IN0_BUTTON;
    }
    #endif
    if (mask & NAMCO_INPUT_P1_COIN) {
        sys->in0 |= NAMCO_IN0_COIN1;
    }
    if (mask & NAMCO_INPUT_P1_START) {
        sys->in1 |= NAMCO_IN1_P1_START;
    }
    if (mask & NAMCO_INPUT_P2_UP) {
        sys->in1 |= NAMCO_IN1_UP;
    }
    if (mask & NAMCO_INPUT_P2_LEFT) {
        sys->in1 |= NAMCO_IN1_LEFT;
    }
    if (mask & NAMCO_INPUT_P2_RIGHT) {
        sys->in1 |= NAMCO_IN1_RIGHT;
    }
    if (mask & NAMCO_INPUT_P2_DOWN) {
        sys->in1 |= NAMCO_IN1_DOWN;
    }
    #if defined(NAMCO_PENGO)
    if (mask & NAMCO_INPUT_P2_BUTTON) {
        sys->in1 |= NAMCO_IN1_BUTTON;
    }
    #endif
    if (mask & NAMCO_INPUT_P2_COIN) {
        sys->in0 |= NAMCO_IN0_COIN2;
    }
    if (mask & NAMCO_INPUT_P2_START) {
        sys->in1 |= NAMCO_IN1_P2_START;
    }
}

void namco_input_clear(namco_t* sys, uint32_t mask) {
    CHIPS_ASSERT(sys && sys->valid);
    if (mask & NAMCO_INPUT_P1_UP) {
        sys->in0 &= ~NAMCO_IN0_UP;
    }
    if (mask & NAMCO_INPUT_P1_LEFT) {
        sys->in0 &= ~NAMCO_IN0_LEFT;
    }
    if (mask & NAMCO_INPUT_P1_RIGHT) {
        sys->in0 &= ~NAMCO_IN0_RIGHT;
    }
    if (mask & NAMCO_INPUT_P1_DOWN) {
        sys->in0 &= ~NAMCO_IN0_DOWN;
    }
    #if defined(NAMCO_PENGO)
    if (mask & NAMCO_INPUT_P1_BUTTON){
        sys->in0 &= ~NAMCO_IN0_BUTTON;
    }
    #endif
    if (mask & NAMCO_INPUT_P1_COIN) {
        sys->in0 &= ~NAMCO_IN0_COIN1;
    }
    if (mask & NAMCO_INPUT_P1_START) {
        sys->in1 &= ~NAMCO_IN1_P1_START;
    }
    if (mask & NAMCO_INPUT_P2_UP) {
        sys->in1 &= ~NAMCO_IN1_UP;
    }
    if (mask & NAMCO_INPUT_P2_LEFT) {
        sys->in1 &= ~NAMCO_IN1_LEFT;
    }
    if (mask & NAMCO_INPUT_P2_RIGHT) {
        sys->in1 &= ~NAMCO_IN1_RIGHT;
    }
    if (mask & NAMCO_INPUT_P2_DOWN) {
        sys->in1 &= ~NAMCO_IN1_DOWN;
    }
    #if defined(NAMCO_PENGO)
    if (mask & NAMCO_INPUT_P2_BUTTON) {
        sys->in1 &= ~NAMCO_IN1_BUTTON;
    }
    #endif
    if (mask & NAMCO_INPUT_P2_COIN) {
        sys->in0 &= ~NAMCO_IN0_COIN2;
    }
    if (mask & NAMCO_INPUT_P2_START) {
        sys->in1 &= ~NAMCO_IN1_P2_START;
    }
}

int namco_std_display_width(void) {
    return NAMCO_DISPLAY_WIDTH;
}

int namco_display_size(void) {
    return NAMCO_DISPLAY_SIZE;
}

int namco_std_display_height(void) {
    return NAMCO_DISPLAY_HEIGHT;
}

int namco_display_width(namco_t* sys) {
    (void)sys;
    return NAMCO_DISPLAY_WIDTH;
}

int namco_display_height(namco_t* sys) {
    (void)sys;
    return NAMCO_DISPLAY_HEIGHT;
}

static void _namco_sound_init(namco_t* sys, const namco_desc_t* desc) {
    CHIPS_ASSERT(desc->audio_num_samples <= NAMCO_MAX_AUDIO_SAMPLES);
    /* assume zero-initialized */
    namco_sound_t* snd = &sys->sound;
    snd->tick_counter = NAMCO_SOUND_PERIOD;
    snd->sample_period = (NAMCO_CPU_CLOCK * NAMCO_SAMPLE_SCALE) / _namco_def(desc->audio_sample_rate, 44100);
    snd->sample_counter = sys->sound.sample_period;
    snd->volume = _namco_def(desc->audio_volume, 1.0f);
    snd->num_samples = _namco_def(desc->audio_num_samples, NAMCO_DEFAULT_AUDIO_SAMPLES);
    snd->callback = desc->audio_cb;
}

#define _NAMCO_SET_NIBBLE_0(val, data) (val=(val&~0x0000F)|((data&0xF)<<0))
#define _NAMCO_SET_NIBBLE_1(val, data) (val=(val&~0x000F0)|((data&0xF)<<4))
#define _NAMCO_SET_NIBBLE_2(val, data) (val=(val&~0x00F00)|((data&0xF)<<8))
#define _NAMCO_SET_NIBBLE_3(val, data) (val=(val&~0x0F000)|((data&0xF)<<12))
#define _NAMCO_SET_NIBBLE_4(val, data) (val=(val&~0xF0000)|((data&0xF)<<16))

static void _namco_sound_wr(namco_t* sys, uint16_t addr, uint8_t data) {
    namco_sound_t* snd = &sys->sound;
    switch (addr) {
        case NAMCO_ADDR_SOUND_V1_FC0:       _NAMCO_SET_NIBBLE_0(snd->voice[0].counter, data); break;
        case NAMCO_ADDR_SOUND_V1_FC1:       _NAMCO_SET_NIBBLE_1(snd->voice[0].counter, data); break;
        case NAMCO_ADDR_SOUND_V1_FC2:       _NAMCO_SET_NIBBLE_2(snd->voice[0].counter, data); break;
        case NAMCO_ADDR_SOUND_V1_FC3:       _NAMCO_SET_NIBBLE_3(snd->voice[0].counter, data); break;
        case NAMCO_ADDR_SOUND_V1_FC4:       _NAMCO_SET_NIBBLE_4(snd->voice[0].counter, data); break;
        case NAMCO_ADDR_SOUND_V1_WAVE:      snd->voice[0].waveform = data & 7; break;
        case NAMCO_ADDR_SOUND_V2_FC1:       _NAMCO_SET_NIBBLE_1(snd->voice[1].counter, data); break;
        case NAMCO_ADDR_SOUND_V2_FC2:       _NAMCO_SET_NIBBLE_2(snd->voice[1].counter, data); break;
        case NAMCO_ADDR_SOUND_V2_FC3:       _NAMCO_SET_NIBBLE_3(snd->voice[1].counter, data); break;
        case NAMCO_ADDR_SOUND_V2_FC4:       _NAMCO_SET_NIBBLE_4(snd->voice[1].counter, data); break;
        case NAMCO_ADDR_SOUND_V2_WAVE:      snd->voice[1].waveform = data & 7; break;
        case NAMCO_ADDR_SOUND_V3_FC1:       _NAMCO_SET_NIBBLE_1(snd->voice[2].counter, data); break;
        case NAMCO_ADDR_SOUND_V3_FC2:       _NAMCO_SET_NIBBLE_2(snd->voice[2].counter, data); break;
        case NAMCO_ADDR_SOUND_V3_FC3:       _NAMCO_SET_NIBBLE_3(snd->voice[2].counter, data); break;
        case NAMCO_ADDR_SOUND_V3_FC4:       _NAMCO_SET_NIBBLE_4(snd->voice[2].counter, data); break;
        case NAMCO_ADDR_SOUND_V3_WAVE:      snd->voice[2].waveform = data & 7; break;
        case NAMCO_ADDR_SOUND_V1_FQ0:       _NAMCO_SET_NIBBLE_0(snd->voice[0].frequency, data); break;
        case NAMCO_ADDR_SOUND_V1_FQ1:       _NAMCO_SET_NIBBLE_1(snd->voice[0].frequency, data); break;
        case NAMCO_ADDR_SOUND_V1_FQ2:       _NAMCO_SET_NIBBLE_2(snd->voice[0].frequency, data); break;
        case NAMCO_ADDR_SOUND_V1_FQ3:       _NAMCO_SET_NIBBLE_3(snd->voice[0].frequency, data); break;
        case NAMCO_ADDR_SOUND_V1_FQ4:       _NAMCO_SET_NIBBLE_4(snd->voice[0].frequency, data); break;
        case NAMCO_ADDR_SOUND_V1_VOLUME:    snd->voice[0].volume = data & 0xF; break;
        case NAMCO_ADDR_SOUND_V2_FQ1:       _NAMCO_SET_NIBBLE_1(snd->voice[1].frequency, data); break;
        case NAMCO_ADDR_SOUND_V2_FQ2:       _NAMCO_SET_NIBBLE_2(snd->voice[1].frequency, data); break;
        case NAMCO_ADDR_SOUND_V2_FQ3:       _NAMCO_SET_NIBBLE_3(snd->voice[1].frequency, data); break;
        case NAMCO_ADDR_SOUND_V2_FQ4:       _NAMCO_SET_NIBBLE_4(snd->voice[1].frequency, data); break;
        case NAMCO_ADDR_SOUND_V2_VOLUME:    snd->voice[1].volume = data & 0xF; break;
        case NAMCO_ADDR_SOUND_V3_FQ1:       _NAMCO_SET_NIBBLE_1(snd->voice[2].frequency, data); break;
        case NAMCO_ADDR_SOUND_V3_FQ2:       _NAMCO_SET_NIBBLE_2(snd->voice[2].frequency, data); break;
        case NAMCO_ADDR_SOUND_V3_FQ3:       _NAMCO_SET_NIBBLE_3(snd->voice[2].frequency, data); break;
        case NAMCO_ADDR_SOUND_V3_FQ4:       _NAMCO_SET_NIBBLE_4(snd->voice[2].frequency, data); break;
        case NAMCO_ADDR_SOUND_V3_VOLUME:    snd->voice[2].volume = data & 0xF; break;
    }
}

static void _namco_sound_tick(namco_t* sys, int num_ticks) {
    namco_sound_t* snd = &sys->sound;
    /* tick the sound chip? */
    snd->tick_counter -= num_ticks;
    if (snd->tick_counter < 0) {
        /* handle 96KHz tick */
        snd->tick_counter += NAMCO_SOUND_PERIOD / NAMCO_SOUND_OVERSAMPLE;
        for (int i = 0; i < 3; i++) {
            if ((snd->voice[i].frequency > 0) && (sys->sound_enable & 1)) {
                snd->voice[i].counter += (snd->voice[i].frequency / NAMCO_SOUND_OVERSAMPLE);
                /* lookup current 4-bit sample from waveform number and the topmost 5
                   bits of the 20-bit sample counter, multiple with 4-bit volume
                */
                uint32_t smp_index = ((snd->voice[i].waveform<<5) | ((snd->voice[i].counter>>15) & 0x1F)) & 0xFF;
                /* integer sample value now 7-bits plus sign bit */
                int val = (((int)(snd->rom[0][smp_index] & 0xF)) - 8) * snd->voice[i].volume;
                snd->voice[i].sample += (float)val;
            }
            snd->voice[i].sample_div += 128.0f;
        }
    }

    /* generate a new sample? */
    snd->sample_counter -= (num_ticks * NAMCO_SAMPLE_SCALE);
    if (snd->sample_counter < 0) {
        snd->sample_counter += snd->sample_period;
        float sm = 0.0f;
        for (int i = 0; i < 3; i++) {
            if (snd->voice[i].sample_div > 0.0f) {
                sm += snd->voice[i].sample / snd->voice[i].sample_div;
                snd->voice[i].sample = 0.0f;
                snd->voice[i].sample_div = 0.0f;
            }
        }
        sm *= snd->volume * 0.33333f;
        snd->sample_buffer[snd->sample_pos++] = sm;
        if (snd->sample_pos == snd->num_samples) {
            if (snd->callback) {
                snd->callback(snd->sample_buffer, snd->num_samples, sys->user_data);
            }
            snd->sample_pos = 0;
        }
    }
}
#endif /* CHIPS_IMPL */
