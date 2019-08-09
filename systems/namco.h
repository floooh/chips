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
    uint8_t int_vector;
    uint8_t int_enable;
    uint8_t sound_enable;
    uint8_t flip_screen;
    uint8_t p1_start_light;
    uint8_t p2_start_light;
    uint8_t coin_lockout;
    mem_t mem;
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
#define NAMCO_DEFAULT_DIP_SWITCHES  (0)
#define NAMCO_ADDR_MASK         (0x7FFF)    /* Pacman has only 15 addr pins wired */
#define NAMCO_IOMAP_BASE        (0x5000)
/* memory mapped IO: read locations*/
#define NAMCO_ADDR_IN0          (0x5000)
#define NAMCO_ADDR_IN1          (0x5040)
#define NAMCO_ADDR_DSW1         (0x5080)
/* memory mapped IO: write locations */
#define NAMCO_ADDR_SPRITES_ATTR     (0x4FF0)
#define NAMCO_ADDR_INT_ENABLE       (0x5000)
#define NAMCO_ADDR_SOUND_ENABLE     (0x5001)
#define NAMCO_ADDR_FLIP_SCREEN      (0x5003)
#define NAMCO_ADDR_P1_START_LIGHT   (0x5004)
#define NAMCO_ADDR_P2_START_LIGHT   (0x5005)
#define NAMCO_ADDR_COIN_LOCKOUT     (0x5006)
#define NAMCO_ADDR_COIN_COUNTER     (0x5007)
#define NAMCO_ADDR_SOUND        (0x5040)
#define NAMCO_ADDR_SPRITES_POS  (0x5060)
#define NAMCO_ADDR_WATCHDOG     (0x50C0)
#else /* PENGO */
#define NAMCO_NUM_SPRITES       (6)
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

    /* dip switches */
    sys->dsw1 = NAMCO_DEFAULT_DIP_SWITCHES;

    /* memory map:

        Pacman: only 15 address bits used, mirroring will happen in tick callback

        0000..3FFF:     16KB ROM
        4000..43FF:     1KB video RAM
        4400..47FF:     1KB color RAM
        4800..4C00:     unmapped?
        4C00..4FEF:     <1KB main RAM
        4FF0..4FFF:     sprite RAM
        5000+           memory mapped registers

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
                    case NAMCO_ADDR_P1_START_LIGHT:
                        sys->p1_start_light = data;
                        break;
                    case NAMCO_ADDR_P2_START_LIGHT:
                        sys->p2_start_light = data;break;
                    case NAMCO_ADDR_COIN_LOCKOUT:
                        sys->coin_lockout = data;
                        break;
                    case NAMCO_ADDR_COIN_COUNTER:
                        /*???*/
                        break;
                    case NAMCO_ADDR_WATCHDOG:
                        /*???*/
                        break;
                }
                printf("WRITE: %02X -> %04X\n", data, addr);
            }
        }
        else if (pins & Z80_RD) {
            /* memory read access */
            if (addr < NAMCO_IOMAP_BASE) {
                Z80_SET_DATA(pins, mem_rd(&sys->mem, addr));
            }
            else {
                /* memory-mapped IO */
                // FIXME
                printf("READ: %04X\n", addr);
                Z80_SET_DATA(pins, 0xFF);
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
        else if (pins & Z80_RD) {
            printf("IN: %04X\n", addr);
        }
        else if (pins & Z80_M1) {
            /* set interrupt vector on data bus */
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

static void _namco_decode_chars(namco_t* sys) {
    uint32_t* ptr = sys->pixel_buffer;
    uint8_t* tile_base = &sys->rom_gfx[0][0];
    for (uint32_t y = 0; y < 28; y++) {
        for (uint32_t x = 0; x < 36; x++) {
            uint16_t offset = _namco_video_offset(x, y);
            uint8_t char_code = sys->video_ram[offset];
            //uint8_t colr_code = sys->color_ram[offset];

            /* TODO: explain tile layout */
            for (uint32_t yy = 0; yy < 8; yy++) {
                /* unpack 8 horizontal bits at a time */
                int tile_index = char_code*16 + yy;
                uint8_t p3_hi = (tile_base[tile_index+8]>>4) & 1;
                uint8_t p3_lo = (tile_base[tile_index+8]>>0) & 1;
                uint8_t p2_hi = (tile_base[tile_index+8]>>5) & 1;
                uint8_t p2_lo = (tile_base[tile_index+8]>>1) & 1;
                uint8_t p1_hi = (tile_base[tile_index+8]>>6) & 1;
                uint8_t p1_lo = (tile_base[tile_index+8]>>2) & 1;
                uint8_t p0_hi = (tile_base[tile_index+8]>>7) & 1;
                uint8_t p0_lo = (tile_base[tile_index+8]>>3) & 1;
                uint8_t p7_hi = (tile_base[tile_index]>>4) & 1;
                uint8_t p7_lo = (tile_base[tile_index]>>0) & 1;
                uint8_t p6_hi = (tile_base[tile_index]>>5) & 1;
                uint8_t p6_lo = (tile_base[tile_index]>>1) & 1;
                uint8_t p5_hi = (tile_base[tile_index]>>6) & 1;
                uint8_t p5_lo = (tile_base[tile_index]>>2) & 1;
                uint8_t p4_hi = (tile_base[tile_index]>>7) & 1;
                uint8_t p4_lo = (tile_base[tile_index]>>3) & 1;

                uint8_t p0 = (p0_hi<<1)|p0_lo;
                uint8_t p1 = (p1_hi<<1)|p1_lo;
                uint8_t p2 = (p2_hi<<1)|p2_lo;
                uint8_t p3 = (p3_hi<<1)|p3_lo;
                uint8_t p4 = (p4_hi<<1)|p4_lo;
                uint8_t p5 = (p5_hi<<1)|p5_lo;
                uint8_t p6 = (p6_hi<<1)|p6_lo;
                uint8_t p7 = (p7_hi<<1)|p7_lo;

                int dst_index = (y*8+yy)*288 + (x*8);
                ptr[dst_index+0] = 0xFF000000 | (p0<<22) | (p0<<14) | (p0<<6);
                ptr[dst_index+1] = 0xFF000000 | (p1<<22) | (p1<<14) | (p1<<6);
                ptr[dst_index+2] = 0xFF000000 | (p2<<22) | (p2<<14) | (p2<<6);
                ptr[dst_index+3] = 0xFF000000 | (p3<<22) | (p3<<14) | (p3<<6);
                ptr[dst_index+4] = 0xFF000000 | (p4<<22) | (p4<<14) | (p4<<6);
                ptr[dst_index+5] = 0xFF000000 | (p5<<22) | (p5<<14) | (p5<<6);
                ptr[dst_index+6] = 0xFF000000 | (p6<<22) | (p6<<14) | (p6<<6);
                ptr[dst_index+7] = 0xFF000000 | (p7<<22) | (p7<<14) | (p7<<6);
            }
        }
    }
}

static void _namco_decode_sprites(namco_t* sys) {
    // FIXME
}

void namco_decode_video(namco_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    if (sys->pixel_buffer) {
        _namco_decode_chars(sys);
        _namco_decode_sprites(sys);
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
