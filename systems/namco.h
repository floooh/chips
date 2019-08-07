#pragma once
/*#
    # namco.h

    WIP!

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

    You need to include the following headers before including bombjack.h:

    - chips/z80.h
    - chips/namco_wsg.h
    - chips/clk.h
    - chips/mem.h

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

#define NAMCO_MAX_AUDIO_SAMPLES (1024)
#define NAMCO_DEFAULT_AUDIO_SAMPLES (128)

/* joystick mask bits */
#define NAMCO_JOYSTICK_UP       (1<<0)
#define NAMCO_JOYSTICK_LEFT     (1<<1)
#define NAMCO_JOYSTICK_RIGHT    (1<<2)
#define NAMCO_JOYSTICK_DOWN     (1<<3)

/* FIXME: dip switches */

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
    const void* rom_gfx_0000_0FFF;          /* Pacman+Pengo */
    const void* rom_gfx_1000_1FFF;          /* Pacman+Pengo */
    const void* rom_gfx_2000_2FFF;          /* Pengo only */
    const void* rom_gfx_3000_3FFF;          /* Pengo only */
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
    int rom_gfx_2000_2FFF_size;
    int rom_gfx_3000_3FFF_size;
    int rom_prom_0000_001F_size;
    int rom_prom_0020_011F_size;
    int rom_prom_0020_041F_size;
    int rom_sound_0000_00FF_size;
    int rom_sound_0100_01FF_size;
} namco_desc_t;

/* the Namco arcade machine state */
typedef struct {
    bool valid;
    z80_t cpu;
    clk_t clk;
    // nwsg_t wsg;
    uint8_t in0;
    uint8_t in1;
    uint8_t dsw1;
    int vsync_count;
    int vblank_count;
    mem_t mem;
    uint8_t rom_cpu[8][0x1000];
    uint8_t rom_gfx[4][0x1000];
    uint8_t rom_prom[0x0420];
    uint8_t rom_sound[2][0x0100];
    /* audio and video 'rendering' */
    struct {
        namco_audio_callback_t callback;
        int num_samples;
        int sample_pos;
        float volume;
        float sample_buffer[NAMCO_MAX_AUDIO_SAMPLES];
    } audio;
    uint32_t* pixel_buffer;
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
/* insert a coin for player */
void namco_p1_insert_coin(void);
/* insert a coin for player 1 */
void namco_p2_insert_coin(void);
/* set player1 joystick bits */
void namco_p1_joystick_set(uint8_t mask);
/* clear player1 joystick bits */
void namco_p1_joystick_clr(uint8_t mask);
/* set player2 joystick bits */
void namco_p2_joystick_set(uint8_t mask);
/* clear player2 joystick bits */
void namco_p2_joystick_clr(uint8_t mask);
/* set dip switches bits */
void namco_dsw1_set(uint8_t mask);
/* clear dip switches bits */
void namco_dsw1_clr(uint8_t mask);
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
#define NAMCO_NUM_SPRITES       (8)
/* memory mapped IO: read locations*/
#define NAMCO_ADDR_IN0          (0x5000)
#define NAMCO_ADDR_IN1          (0x5040)
#define NAMCO_ADDR_DSW1         (0x5080)
/* memory mapped IO: write locations */
#define NAMCO_ADDR_SPRITES_ATTR (0x4FF0)
#define NAMCO_ADDR_INT_ENABLE   (0x5000)
#define NAMCO_ADDR_SOUND_ENABLE (0x5001)
#define NAMCO_ADDR_FLIP_SCREEN  (0x5003)
#define NAMCO_ADDR_SOUND        (0x5040)
#define NAMCO_ADDR_SPRITES_POS  (0x5060)
#define NAMCO_ADDR_WATCHDOG     (0x50C0)
#else /* PENGO */
#define NAMCO_NUM_SPRITES       (6)
/* memory mapped IO: read locations*/
#define NAMCO_ADDR_IN0          (0x90C0)
#define NAMCO_ADDR_IN1          (0x9080)
#define NAMCO_ADDR_DSW1         (0x9000)
#define NAMCO_ADDR_DSW0         (0x9040)
/* memory mapped IO: write locations */
#define NAMCO_ADDR_SPRITES_ATTR (0x8FF2)
#define NAMCO_ADDR_SOUND        (0x9005)
#define NAMCO_ADDR_SPRITE_POS   (0x9022)
#define NAMCO_ADDR_INT_ENABLE   (0x9040)
#define NAMCO_ADDR_SOUND_ENABLE (0x9041)
#define NAMCO_ADDR_PAL_SELECT   (0x9042)
#define NAMCO_ADDR_FLIP_SCREEN  (0x9043)
#define NAMCO_ADDR_CLUT_SELECT  (0x9046)
#define NAMCO_ADDR_TILE_SELECT  (0x9047)
#define NAMCO_ADDR_WATCHDOG     (0x9070)
#endif
#define NAMCO_MASTER_CLOCK      (18432000)
#define NAMCO_CPU_CLOCK         (NAMCO_MASTER_CLOCK / 6)
#define NAMCO_SOUND_CLOCK       (NAMCO_MASTER_CLOCK / 128)
#define NAMCO_VBLANK_DURATION   (2500)
#define NAMCO_DISPLAY_WIDTH     (256)
#define NAMCO_DISPLAY_HEIGHT    (224)
#define NAMCO_DISPLAY_SIZE      (NAMCO_DISPLAY_WIDTH*NAMCO_DISPLAY_HEIGHT*4)

//static uint64_t _namco_tick(int num, uint64_t pins, void* user_data);

#define _namco_def(val, def) (val == 0 ? def : val)

void namco_init(namco_t* sys, const namco_desc_t* desc) {
    CHIPS_ASSERT(sys && desc);

    memset(sys, 0, sizeof(namco_t));
    sys->valid = true;
    
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
    CHIPS_ASSERT(desc->rom_gfx_0000_0FFF && (desc->rom_gfx_0000_0FFF_size == 0x1000));
    CHIPS_ASSERT(desc->rom_gfx_1000_1FFF && (desc->rom_gfx_1000_1FFF_size == 0x1000));
    #if defined(NAMCO_PENGO)
    CHIPS_ASSERT(desc->rom_gfx_2000_2FFF && (desc->rom_gfx_2000_2FFF_size == 0x1000));
    CHIPS_ASSERT(desc->rom_gfx_3000_3FFF && (desc->rom_gfx_3000_3FFF_size == 0x1000));
    #endif
    CHIPS_ASSERT(desc->rom_prom_0000_001F && (desc->rom_prom_0000_001F_size == 0x0020));
    #if defined(NAMCO_PACMAN)
    CHIPS_ASSERT(desc->rom_prom_0020_011F && (desc->rom_prom_0020_011F_size == 0x0100));
    #else
    CHIPS_ASSERT(desc->rom_prom_0020_041F && (desc->rom_prom_0020_041F_size == 0x0400));
    #endif
    CHIPS_ASSERT(desc->rom_sound_0000_00FF && (desc->rom_sound_0000_00FF_size == 0x0100));
    CHIPS_ASSERT(desc->rom_sound_0100_01FF && (desc->rom_sound_0100_01FF_size == 0x0100));
    memcpy(sys->rom_cpu[0], desc->rom_cpu_0000_0FFF, 0x1000);
    memcpy(sys->rom_cpu[1], desc->rom_cpu_1000_1FFF, 0x1000);
    memcpy(sys->rom_cpu[2], desc->rom_cpu_2000_2FFF, 0x1000);
    memcpy(sys->rom_cpu[3], desc->rom_cpu_3000_3FFF, 0x1000);
    #if defined(NAMCO_PENGO)
    memcpy(sys->rom_cpu[4], desc->rom_cpu_4000_4FFF, 0x1000);
    memcpy(sys->rom_cpu[5], desc->rom_cpu_5000_5FFF, 0x1000);
    memcpy(sys->rom_cpu[6], desc->rom_cpu_6000_6FFF, 0x1000);
    memcpy(sys->rom_cpu[7], desc->rom_cpu_7000_7FFF, 0x1000);
    #endif
    memcpy(sys->rom_gfx[0], desc->rom_gfx_0000_0FFF, 0x1000);
    memcpy(sys->rom_gfx[1], desc->rom_gfx_1000_1FFF, 0x1000);
    #if defined(NAMCO_PENGO)
    memcpy(sys->rom_gfx[2], desc->rom_gfx_2000_2FFF, 0x1000);
    memcpy(sys->rom_gfx[3], desc->rom_gfx_3000_3FFF, 0x1000);
    #endif
    memcpy(&sys->rom_prom[0], desc->rom_prom_0000_001F, 0x0020);
    #if defined(NAMCO_PACMAN)
    memcpy(&sys->rom_prom[0x0020], desc->rom_prom_0020_011F, 0x0100);
    #else
    memcpy(&sys->rom_prom[0x0020], desc->rom_prom_0020_041F, 0x0400);
    #endif
    memcpy(sys->rom_sound[0], desc->rom_sound_0000_00FF, 0x0100);
    memcpy(sys->rom_sound[1], desc->rom_sound_0100_01FF, 0x0100);    
}

int namco_std_display_width(void) {
    return NAMCO_DISPLAY_WIDTH;
}

int namco_std_display_height(void) {
    return NAMCO_DISPLAY_HEIGHT;
}
#endif /* CHIPS_IMPL */
