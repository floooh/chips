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
#define NAMCO_P1_JOYSTICK_UP    (1<<0)
#define NAMCO_P1_JOYSTICK_LEFT  (1<<1)
#define NAMCO_P1_JOYSTICK_RIGHT (1<<2)
#define NAMCO_P1_JOYSTICK_DOWN  (1<<3)
#define NAMCO_P1_COIN           (1<<4)
#define NAMCO_P1_START          (1<<5)
#define NAMCO_P2_JOYSTICK_UP    (1<<6)
#define NAMCO_P2_JOYSTICK_LEFT  (1<<7)
#define NAMCO_P2_JOYSTICK_RIGHT (1<<8)
#define NAMCO_P2_JOYSTICK_DOWN  (1<<9)
#define NAMCO_P2_COIN           (1<<10)
#define NAMCO_P2_START          (1<<11)

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
    uint8_t int_vector;
    uint8_t int_enable;
    uint8_t sound_enable;
    uint8_t flip_screen;
    uint8_t sprite_coords[16];
    mem_t mem;
    uint32_t palette_cache[256];
    uint8_t video_ram[0x0400];
    uint8_t color_ram[0x0400];
    uint8_t main_ram[0x0800];
    uint8_t rom_cpu[8][0x1000];
    uint8_t rom_gfx[4][0x1000];
    uint8_t rom_prom[0x0420];
    uint8_t rom_sound[2][0x0100];
    void* user_data;
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
/* DSW1 bits (active low) */
#define NAMCO_DSW1_DEFAULT      (NAMCO_DSW1_COINS_1C1G|NAMCO_DSW1_LIVES_3|NAMCO_DSW1_EXTRALIFE_15K|NAMCO_DSW1_DIFFICULTY_NORM)
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
/* memory mapped IO: read locations*/
#define NAMCO_ADDR_IN0          (0x5000)
#define NAMCO_ADDR_IN1          (0x5040)
#define NAMCO_ADDR_DSW1         (0x5080)
/* memory mapped IO: write locations */
#define NAMCO_ADDR_SPRITES_ATTR     (0x4FF0)
#define NAMCO_ADDR_INT_ENABLE       (0x5000)
#define NAMCO_ADDR_SOUND_ENABLE     (0x5001)
#define NAMCO_ADDR_FLIP_SCREEN      (0x5003)
#define NAMCO_ADDR_SOUND        (0x5040)
#define NAMCO_ADDR_SPRITES_POS  (0x5060)
#else /* PENGO */
#define NAMCO_DEFAULT_DIP_SWITCHES  (0)
#define NAMCO_ADDR_MASK         (0xFFFF)
#define NAMCO_IOMAP_BASE        (0x9000)
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
#define NAMCO_VSYNC_PERIOD      (NAMCO_CPU_CLOCK / 60)
#define NAMCO_DISPLAY_WIDTH     (288)
#define NAMCO_DISPLAY_HEIGHT    (224)
#define NAMCO_DISPLAY_SIZE      (NAMCO_DISPLAY_WIDTH*NAMCO_DISPLAY_HEIGHT*4)

static uint64_t _namco_tick(int num, uint64_t pins, void* user_data);

#define _namco_def(val, def) (val == 0 ? def : val)

void namco_init(namco_t* sys, const namco_desc_t* desc) {
    CHIPS_ASSERT(sys && desc);

    memset(sys, 0, sizeof(namco_t));
    sys->valid = true;

    /* audio and video output */
    CHIPS_ASSERT(desc->audio_num_samples <= NAMCO_MAX_AUDIO_SAMPLES);
    sys->audio.callback = desc->audio_cb;
    sys->audio.num_samples = _namco_def(desc->audio_num_samples, NAMCO_DEFAULT_AUDIO_SAMPLES);
    sys->audio.volume = _namco_def(desc->audio_volume, 1.0f);
    sys->user_data = desc->user_data;
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

    /* vsync/vblank counters */
    sys->vsync_count = NAMCO_VSYNC_PERIOD;

    /* system clock and CPU */
    clk_init(&sys->clk, NAMCO_CPU_CLOCK);
    z80_desc_t cpu_desc;
    memset(&cpu_desc, 0, sizeof(cpu_desc));
    cpu_desc.tick_cb = _namco_tick;
    cpu_desc.user_data = sys;
    z80_init(&sys->cpu, &cpu_desc);

    /* FIXME: setup WSG */

    /* memory mapped IO config */
    sys->in0 = 0xFF;
    sys->in1 = 0xFF;
    sys->dsw1 = NAMCO_DSW1_DEFAULT;

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
    mem_map_rom(&sys->mem, 0, 0x0000, 0x1000, sys->rom_cpu[0]);
    mem_map_rom(&sys->mem, 0, 0x1000, 0x1000, sys->rom_cpu[1]);
    mem_map_rom(&sys->mem, 0, 0x2000, 0x1000, sys->rom_cpu[2]);
    mem_map_rom(&sys->mem, 0, 0x3000, 0x1000, sys->rom_cpu[3]);
    #if defined(NAMCO_PACMAN)
        mem_map_ram(&sys->mem, 0, 0x4000, 0x0400, sys->video_ram);
        mem_map_ram(&sys->mem, 0, 0x4400, 0x0400, sys->color_ram);
        mem_map_ram(&sys->mem, 0, 0x4C00, 0x0400, sys->main_ram);
    #endif
    #if defined(NAMCO_PENGO)
        mem_map_rom(&sys->mem, 0, 0x4000, 0x1000, sys->rom_cpu[4]);
        mem_map_rom(&sys->mem, 0, 0x5000, 0x1000, sys->rom_cpu[5]);
        mem_map_rom(&sys->mem, 0, 0x6000, 0x1000, sys->rom_cpu[6]);
        mem_map_rom(&sys->mem, 0, 0x7000, 0x1000, sys->rom_cpu[7]);
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
    }
}

void namco_discard(namco_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    sys->valid = false;
}

void namco_reset(namco_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    z80_reset(&sys->cpu);
    //nwsg_reset(&sys->wsg);
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
                switch (addr & 0x00F0) {
                    case 0x0000:
                        switch (addr) {
                            case NAMCO_ADDR_INT_ENABLE:
                                sys->int_enable = data;
                                break;
                            case NAMCO_ADDR_SOUND_ENABLE:
                                sys->sound_enable = data;
                                break;
                            case NAMCO_ADDR_FLIP_SCREEN:
                                sys->flip_screen = data;
                                break;
                            /* start light, coint lockout, coin counter, watchdog all ignored */
                        }
                        break;
                    case 0x0040:
                    case 0x0050:
                        /* sound registers */
                        // FIXME
                        break;
                    case 0x0060:
                        /* sprite coords */
                        sys->sprite_coords[addr & 0x000F] = data;
                        break;
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
                        data = sys->in0;
                        break;
                    case NAMCO_ADDR_IN1:
                        data = sys->in1;
                        break;
                    case NAMCO_ADDR_DSW1:
                        data = sys->dsw1;
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

/* https://www.walkofmind.com/programming/pie/video_memory.htm */
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

/* 8x4 tile decoder */
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
    int tile_index = char_code*tile_stride + tile_offset;
    for (uint32_t yy = 0; yy < 8; yy++, tile_index++) {
        uint32_t y = py + (yy ^ xor_y);
        if (y >= NAMCO_DISPLAY_HEIGHT) {
            break;
        }
        int tile_index = char_code*tile_stride + tile_offset + yy;
        for (uint32_t xx = 0; xx < 4; xx++) {
            uint32_t x = px + (xx ^ xor_x);
            if (x >= NAMCO_DISPLAY_WIDTH) {
                break;
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

static void _namco_decode_chars(namco_t* sys) {
    uint32_t* pixel_base = sys->pixel_buffer;
    uint32_t* pal_base = sys->palette_cache;
    uint8_t* tile_base = &sys->rom_gfx[0][0];
    for (uint32_t y = 0; y < 28; y++) {
        for (uint32_t x = 0; x < 36; x++) {
            uint16_t offset = _namco_video_offset(x, y);
            uint8_t char_code = sys->video_ram[offset];
            uint8_t color_code = sys->color_ram[offset] & 0x3F;
            _namco_8x4(pixel_base, tile_base, pal_base, 16, 8, x*8, y*8, char_code, color_code, true, false, false);
            _namco_8x4(pixel_base, tile_base, pal_base, 16, 0, x*8+4, y*8, char_code, color_code, true, false, false);
        }
    }
}

static void _namco_decode_sprites(namco_t* sys) {
    uint32_t* pixel_base = sys->pixel_buffer;
    uint32_t* pal_base = sys->palette_cache;
    uint8_t* tile_base = &sys->rom_gfx[1][0];
    for (int sprite_index = 6; sprite_index >= 1; --sprite_index) {
        uint32_t py = sys->sprite_coords[sprite_index*2 + 0] - 31;
        uint32_t px = 272 - sys->sprite_coords[sprite_index*2 + 1];
        uint8_t shape = sys->main_ram[0x03F0 + sprite_index*2 + 0];
        uint8_t char_code = shape>>2;
        uint8_t color_code = sys->main_ram[0x03F0 + sprite_index*2 + 1];
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
    if (mask & NAMCO_P1_JOYSTICK_UP) {
        sys->in0 &= ~NAMCO_IN0_UP;
    }
    if (mask & NAMCO_P1_JOYSTICK_LEFT) {
        sys->in0 &= ~NAMCO_IN0_LEFT;
    }
    if (mask & NAMCO_P1_JOYSTICK_RIGHT) {
        sys->in0 &= ~NAMCO_IN0_RIGHT;
    }
    if (mask & NAMCO_P1_JOYSTICK_DOWN) {
        sys->in0 &= ~NAMCO_IN0_DOWN;
    }
    if (mask & NAMCO_P1_COIN) {
        sys->in0 &= ~NAMCO_IN0_COIN1;
    }
    if (mask & NAMCO_P1_START) {
        sys->in1 &= ~NAMCO_IN1_P1_START;
    }
    if (mask & NAMCO_P2_JOYSTICK_UP) {
        sys->in1 &= ~NAMCO_IN1_UP;
    }
    if (mask & NAMCO_P2_JOYSTICK_LEFT) {
        sys->in1 &= ~NAMCO_IN1_LEFT;
    }
    if (mask & NAMCO_P2_JOYSTICK_RIGHT) {
        sys->in1 &= ~NAMCO_IN1_RIGHT;
    }
    if (mask & NAMCO_P2_JOYSTICK_DOWN) {
        sys->in1 &= ~NAMCO_IN1_DOWN;
    }
    if (mask & NAMCO_P2_COIN) {
        sys->in0 &= ~NAMCO_IN0_COIN2;
    }
    if (mask & NAMCO_P2_START) {
        sys->in1 &= ~NAMCO_IN1_P2_START;
    }
}

void namco_input_clear(namco_t* sys, uint32_t mask) {
    CHIPS_ASSERT(sys && sys->valid);
    if (mask & NAMCO_P1_JOYSTICK_UP) {
        sys->in0 |= NAMCO_IN0_UP;
    }
    if (mask & NAMCO_P1_JOYSTICK_LEFT) {
        sys->in0 |= NAMCO_IN0_LEFT;
    }
    if (mask & NAMCO_P1_JOYSTICK_RIGHT) {
        sys->in0 |= NAMCO_IN0_RIGHT;
    }
    if (mask & NAMCO_P1_JOYSTICK_DOWN) {
        sys->in0 |= NAMCO_IN0_DOWN;
    }
    if (mask & NAMCO_P1_COIN) {
        sys->in0 |= NAMCO_IN0_COIN1;
    }
    if (mask & NAMCO_P1_START) {
        sys->in0 |= NAMCO_IN1_P1_START;
    }
    if (mask & NAMCO_P2_JOYSTICK_UP) {
        sys->in1 |= NAMCO_IN1_UP;
    }
    if (mask & NAMCO_P2_JOYSTICK_LEFT) {
        sys->in1 |= NAMCO_IN1_LEFT;
    }
    if (mask & NAMCO_P2_JOYSTICK_RIGHT) {
        sys->in1 |= NAMCO_IN1_RIGHT;
    }
    if (mask & NAMCO_P2_JOYSTICK_DOWN) {
        sys->in1 |= NAMCO_IN1_DOWN;
    }
    if (mask & NAMCO_P2_COIN) {
        sys->in0 |= NAMCO_IN0_COIN2;
    }
    if (mask & NAMCO_P2_START) {
        sys->in1 |= NAMCO_IN1_P2_START;
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
    return NAMCO_DISPLAY_WIDTH;
}

int namco_display_height(namco_t* sys) {
    return NAMCO_DISPLAY_HEIGHT;
}

#endif /* CHIPS_IMPL */
