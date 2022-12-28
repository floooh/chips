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
#include <stddef.h>
#include <stdalign.h>

#ifdef __cplusplus
extern "C" {
#endif

// increase when namco_t memory layout changes
#define NAMCO_SNAPSHOT_VERSION (1)

#define NAMCO_MAX_AUDIO_SAMPLES (1024)
#define NAMCO_DEFAULT_AUDIO_SAMPLES (128)
#define NAMCO_FRAMEBUFFER_WIDTH (512)
#define NAMCO_FRAMEBUFFER_HEIGHT (224)
#define NAMCO_FRAMEBUFFER_SIZE_BYTES (NAMCO_FRAMEBUFFER_WIDTH * NAMCO_FRAMEBUFFER_HEIGHT)
#define NAMCO_DISPLAY_WIDTH     (288)
#define NAMCO_DISPLAY_HEIGHT    (224)

// input bits (use with namco_input_set() and namco_input_clear())
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

// configuration parameters for namco_init()
typedef struct {
    chips_debug_t debug;
    chips_audio_desc_t audio;
    struct {
        // common ROM areas for Pacman and Pengo
        struct {
            chips_range_t cpu_0000_0FFF;
            chips_range_t cpu_1000_1FFF;
            chips_range_t cpu_2000_2FFF;
            chips_range_t cpu_3000_3FFF;
            chips_range_t prom_0000_001F;
            chips_range_t sound_0000_00FF;
            chips_range_t sound_0100_01FF;
        } common;
        // Pengo specific ROM areas
        struct {
            chips_range_t cpu_4000_4FFF;
            chips_range_t cpu_5000_5FFF;
            chips_range_t cpu_6000_6FFF;
            chips_range_t cpu_7000_7FFF;
            chips_range_t gfx_0000_1FFF;
            chips_range_t gfx_2000_3FFF;
            chips_range_t prom_0020_041F;
        } pengo;
        // Pacman specific ROM areas
        struct {
            chips_range_t gfx_0000_0FFF;
            chips_range_t gfx_1000_1FFF;
            chips_range_t prom_0020_011F;
        } pacman;
    } roms;
} namco_desc_t;

// audio state
typedef struct {
    int tick_counter;
    int sample_period;
    int sample_counter;
    float volume;
    struct {
        uint32_t frequency; // 20-bit frequency
        uint32_t counter;   // 20-bit counter (top 5 bits are index into 32-byte wave table)
        uint8_t waveform;   // 3-bit waveform
        uint8_t volume;     // 4-bit volume
        float sample;       // accumulated sample value
        float sample_div  ; // oversampling divider
    } voice[3];
    uint8_t rom[2][0x0100]; // wave table ROM
    int num_samples;
    int sample_pos;
    chips_audio_callback_t callback;
    float sample_buffer[NAMCO_MAX_AUDIO_SAMPLES];
} namco_sound_t;

// the Namco arcade machine state
typedef struct {
    z80_t cpu;
    mem_t mem;
    uint8_t in0;    // inverted bits (active-low)
    uint8_t in1;    // inverted bits (active-low)
    uint8_t dsw1;   // dip-switches as-is (active-high)
    uint8_t dsw2;   // Pengo only
    uint64_t pins;
    int vsync_count;
    uint8_t int_vector;     // IM2 interrupt vector set with OUT on port 0
    uint8_t int_enable;
    uint8_t sound_enable;
    uint8_t flip_screen;    // screen-flip (for cocktail-cabinet) is not implemented
    uint8_t pal_select;     // Pengo only
    uint8_t clut_select;    // Pengo only
    uint8_t tile_select;    // Pengo only
    uint8_t sprite_coords[16];      // 8 sprites, uint8_t x, uint8_t y

    bool valid;
    chips_debug_t debug;

    namco_sound_t sound;
    uint8_t video_ram[0x0400];
    uint8_t color_ram[0x0400];
    uint8_t main_ram[0x0800];       // Pacman: 1 KB, Pengo: 2 KB
    uint8_t rom_cpu[0x8000];        // program ROM: Pacman: 16 KB, Pengo: 32 KB
    uint8_t rom_gfx[0x4000];        // tile ROM: Pacman: 8 KB, Pengo: 16 KB
    uint8_t rom_prom[0x0420];       // palette and color lookup ROM
    uint32_t hw_colors[32];         // decoded color palette from palette ROM
    uint8_t palette_cache[512];     // palette indirection table, Pacman: 256 entries , Pengo: 512 entries
    alignas(64) uint8_t fb[NAMCO_FRAMEBUFFER_SIZE_BYTES];   // indices into palette
} namco_t;

// initialize a new namco_t instance
void namco_init(namco_t* sys, const namco_desc_t* desc);
// discard a namco_t instance
void namco_discard(namco_t* sys);
// reset a namco_t instance
void namco_reset(namco_t* sys);
// query display, framebuffer and color palette (note: palette requires a valid sys ptr!)
chips_display_info_t namco_display_info(namco_t* sys);
// run namco_t instance for given amount of microseconds, return number of ticks executed
uint32_t namco_exec(namco_t* sys, uint32_t micro_seconds);
// set input bits
void namco_input_set(namco_t* sys, uint32_t mask);
// clear input bits
void namco_input_clear(namco_t* sys, uint32_t mask);
// take a snapshot, patches any pointers to zero, returns a snapshot version
uint32_t namco_save_snapshot(namco_t* sys, namco_t* dst);
// load a snapshot, returns false if snapshot version doesn't match
bool namco_load_snapshot(namco_t* sys, uint32_t version, namco_t* src);

#ifdef __cplusplus
} // extern "C"
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
#define NAMCO_ADDR_SOUND_V1_FC0     (NAMCO_ADDR_SOUND_BASE+0x00)    // voice1 frequency counter nibble 0
#define NAMCO_ADDR_SOUND_V1_FC1     (NAMCO_ADDR_SOUND_BASE+0x01)    //                          nibble 1
#define NAMCO_ADDR_SOUND_V1_FC2     (NAMCO_ADDR_SOUND_BASE+0x02)    //                          nibble 2
#define NAMCO_ADDR_SOUND_V1_FC3     (NAMCO_ADDR_SOUND_BASE+0x03)    //                          nibble 3
#define NAMCO_ADDR_SOUND_V1_FC4     (NAMCO_ADDR_SOUND_BASE+0x04)    //                          nibble 4
#define NAMCO_ADDR_SOUND_V1_WAVE    (NAMCO_ADDR_SOUND_BASE+0x05)    // voice1 wave form
#define NAMCO_ADDR_SOUND_V2_FC1     (NAMCO_ADDR_SOUND_BASE+0x06)    // voice2 frequency counter nibble 1
#define NAMCO_ADDR_SOUND_V2_FC2     (NAMCO_ADDR_SOUND_BASE+0x07)    //                          nibble 2
#define NAMCO_ADDR_SOUND_V2_FC3     (NAMCO_ADDR_SOUND_BASE+0x08)    //                          nibble 3
#define NAMCO_ADDR_SOUND_V2_FC4     (NAMCO_ADDR_SOUND_BASE+0x09)    //                          nibble 4
#define NAMCO_ADDR_SOUND_V2_WAVE    (NAMCO_ADDR_SOUND_BASE+0x0A)    // voice2 wave form
#define NAMCO_ADDR_SOUND_V3_FC1     (NAMCO_ADDR_SOUND_BASE+0x0B)    // voice3 frequency counter nibble 1
#define NAMCO_ADDR_SOUND_V3_FC2     (NAMCO_ADDR_SOUND_BASE+0x0C)    //                          nibble 2
#define NAMCO_ADDR_SOUND_V3_FC3     (NAMCO_ADDR_SOUND_BASE+0x0D)    //                          nibble 3
#define NAMCO_ADDR_SOUND_V3_FC4     (NAMCO_ADDR_SOUND_BASE+0x0E)    //                          nibble 4
#define NAMCO_ADDR_SOUND_V3_WAVE    (NAMCO_ADDR_SOUND_BASE+0x0F)    // voice3 wave form
#define NAMCO_ADDR_SOUND_V1_FQ0     (NAMCO_ADDR_SOUND_BASE+0x10)    // voice1 frequency nibble 0
#define NAMCO_ADDR_SOUND_V1_FQ1     (NAMCO_ADDR_SOUND_BASE+0x11)    //                  nibble 1
#define NAMCO_ADDR_SOUND_V1_FQ2     (NAMCO_ADDR_SOUND_BASE+0x12)    //                  nibble 2
#define NAMCO_ADDR_SOUND_V1_FQ3     (NAMCO_ADDR_SOUND_BASE+0x13)    //                  nibble 3
#define NAMCO_ADDR_SOUND_V1_FQ4     (NAMCO_ADDR_SOUND_BASE+0x14)    //                  nibble 4
#define NAMCO_ADDR_SOUND_V1_VOLUME  (NAMCO_ADDR_SOUND_BASE+0x15)    // voice1 volume
#define NAMCO_ADDR_SOUND_V2_FQ1     (NAMCO_ADDR_SOUND_BASE+0x16)    // voice2 frequency nibble 1
#define NAMCO_ADDR_SOUND_V2_FQ2     (NAMCO_ADDR_SOUND_BASE+0x17)    //                  nibble 2
#define NAMCO_ADDR_SOUND_V2_FQ3     (NAMCO_ADDR_SOUND_BASE+0x18)    //                  nibble 3
#define NAMCO_ADDR_SOUND_V2_FQ4     (NAMCO_ADDR_SOUND_BASE+0x19)    //                  nibble 4
#define NAMCO_ADDR_SOUND_V2_VOLUME  (NAMCO_ADDR_SOUND_BASE+0x1A)    // voice2 volume
#define NAMCO_ADDR_SOUND_V3_FQ1     (NAMCO_ADDR_SOUND_BASE+0x1B)    // voice3 frequency nibble 1
#define NAMCO_ADDR_SOUND_V3_FQ2     (NAMCO_ADDR_SOUND_BASE+0x1C)    //                  nibble 2
#define NAMCO_ADDR_SOUND_V3_FQ3     (NAMCO_ADDR_SOUND_BASE+0x1D)    //                  nibble 3
#define NAMCO_ADDR_SOUND_V3_FQ4     (NAMCO_ADDR_SOUND_BASE+0x1E)    //                  nibble 4
#define NAMCO_ADDR_SOUND_V3_VOLUME  (NAMCO_ADDR_SOUND_BASE+0x1F)    // voice3 volume

#define NAMCO_MASTER_CLOCK      (18432000)
#define NAMCO_CPU_CLOCK         (NAMCO_MASTER_CLOCK / 6)
#define NAMCO_SOUND_PERIOD      (32)    // sound is ticked every 32 CPU ticks
#define NAMCO_SOUND_OVERSAMPLE  (2)
#define NAMCO_SAMPLE_SCALE      (16)
#define NAMCO_VSYNC_PERIOD      (NAMCO_CPU_CLOCK / 60)

static void _namco_sound_init(namco_t* sys, const namco_desc_t* desc);
static void _namco_sound_wr(namco_t* sys, uint16_t addr, uint8_t data);
static void _namco_sound_tick(namco_t* sys);

#define _namco_def(val, def) (val == 0 ? def : val)

void namco_init(namco_t* sys, const namco_desc_t* desc) {
    CHIPS_ASSERT(sys && desc);
    if (desc->debug.callback.func) { CHIPS_ASSERT(desc->debug.stopped); }

    memset(sys, 0, sizeof(namco_t));
    sys->valid = true;
    sys->debug = desc->debug;
    sys->vsync_count = NAMCO_VSYNC_PERIOD;
    _namco_sound_init(sys, desc);
    sys->pins = z80_init(&sys->cpu);

    // copy over ROM images
    CHIPS_ASSERT(desc->roms.common.cpu_0000_0FFF.ptr && (desc->roms.common.cpu_0000_0FFF.size == 0x1000));
    CHIPS_ASSERT(desc->roms.common.cpu_1000_1FFF.ptr && (desc->roms.common.cpu_1000_1FFF.size == 0x1000));
    CHIPS_ASSERT(desc->roms.common.cpu_2000_2FFF.ptr && (desc->roms.common.cpu_2000_2FFF.size == 0x1000));
    CHIPS_ASSERT(desc->roms.common.cpu_3000_3FFF.ptr && (desc->roms.common.cpu_3000_3FFF.size == 0x1000));
    CHIPS_ASSERT(desc->roms.common.prom_0000_001F.ptr && (desc->roms.common.prom_0000_001F.size == 0x0020));
    CHIPS_ASSERT(desc->roms.common.sound_0000_00FF.ptr && (desc->roms.common.sound_0000_00FF.size == 0x0100));
    CHIPS_ASSERT(desc->roms.common.sound_0100_01FF.ptr && (desc->roms.common.sound_0100_01FF.size == 0x0100));
    memcpy(&sys->rom_cpu[0x0000], desc->roms.common.cpu_0000_0FFF.ptr, 0x1000);
    memcpy(&sys->rom_cpu[0x1000], desc->roms.common.cpu_1000_1FFF.ptr, 0x1000);
    memcpy(&sys->rom_cpu[0x2000], desc->roms.common.cpu_2000_2FFF.ptr, 0x1000);
    memcpy(&sys->rom_cpu[0x3000], desc->roms.common.cpu_3000_3FFF.ptr, 0x1000);
    memcpy(&sys->rom_prom[0], desc->roms.common.prom_0000_001F.ptr, 0x0020);
    memcpy(sys->sound.rom[0], desc->roms.common.sound_0000_00FF.ptr, 0x0100);
    memcpy(sys->sound.rom[1], desc->roms.common.sound_0100_01FF.ptr, 0x0100);
    #if defined(NAMCO_PENGO)
    CHIPS_ASSERT(desc->roms.pengo.cpu_4000_4FFF.ptr && (desc->roms.pengo.cpu_4000_4FFF.size == 0x1000));
    CHIPS_ASSERT(desc->roms.pengo.cpu_5000_5FFF.ptr && (desc->roms.pengo.cpu_5000_5FFF.size == 0x1000));
    CHIPS_ASSERT(desc->roms.pengo.cpu_6000_6FFF.ptr && (desc->roms.pengo.cpu_6000_6FFF.size == 0x1000));
    CHIPS_ASSERT(desc->roms.pengo.cpu_7000_7FFF.ptr && (desc->roms.pengo.cpu_7000_7FFF.size == 0x1000));
    CHIPS_ASSERT(desc->roms.pengo.gfx_0000_1FFF.ptr && (desc->roms.pengo.gfx_0000_1FFF.size == 0x2000));
    CHIPS_ASSERT(desc->roms.pengo.gfx_2000_3FFF.ptr && (desc->roms.pengo.gfx_2000_3FFF.size == 0x2000));
    CHIPS_ASSERT(desc->roms.pengo.prom_0020_041F.ptr && (desc->roms.pengo.prom_0020_041F.size == 0x0400));
    memcpy(&sys->rom_cpu[0x4000], desc->roms.pengo.cpu_4000_4FFF.ptr, 0x1000);
    memcpy(&sys->rom_cpu[0x5000], desc->roms.pengo.cpu_5000_5FFF.ptr, 0x1000);
    memcpy(&sys->rom_cpu[0x6000], desc->roms.pengo.cpu_6000_6FFF.ptr, 0x1000);
    memcpy(&sys->rom_cpu[0x7000], desc->roms.pengo.cpu_7000_7FFF.ptr, 0x1000);
    memcpy(&sys->rom_gfx[0x0000], desc->roms.pengo.gfx_0000_1FFF.ptr, 0x2000);
    memcpy(&sys->rom_gfx[0x2000], desc->roms.pengo.gfx_2000_3FFF.ptr, 0x2000);
    memcpy(&sys->rom_prom[0x0020], desc->roms.pengo.prom_0020_041F.ptr, 0x0400);
    #endif
    #if defined(NAMCO_PACMAN)
    CHIPS_ASSERT(desc->roms.pacman.gfx_0000_0FFF.ptr && (desc->roms.pacman.gfx_0000_0FFF.size == 0x1000));
    CHIPS_ASSERT(desc->roms.pacman.gfx_1000_1FFF.ptr && (desc->roms.pacman.gfx_1000_1FFF.size == 0x1000));
    CHIPS_ASSERT(desc->roms.pacman.prom_0020_011F.ptr && (desc->roms.pacman.prom_0020_011F.size == 0x0100));
    memcpy(&sys->rom_gfx[0x0000], desc->roms.pacman.gfx_0000_0FFF.ptr, 0x1000);
    memcpy(&sys->rom_gfx[0x1000], desc->roms.pacman.gfx_1000_1FFF.ptr, 0x1000);
    memcpy(&sys->rom_prom[0x0020], desc->roms.pacman.prom_0020_011F.ptr, 0x0100);
    #endif

    // memory mapped IO config
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

    // setup an RGBA palette from the 8-bit RGB values in PROM
    for (size_t i = 0; i < 32; i++) {
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
        sys->hw_colors[i] = 0xFF000000 | (b<<16) | (g<<8) | r;
    }
    for (size_t i = 0; i < 256; i++) {
        uint8_t pal_index = sys->rom_prom[i + 0x20] & 0xF;
        sys->palette_cache[i] = pal_index;
        sys->palette_cache[256 + i] = 0x10 | pal_index;
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

static uint64_t _namco_tick(namco_t* sys, uint64_t pins) {
    // update the vsync counter and trigger VSYNC interrupt
    sys->vsync_count--;
    if (sys->vsync_count < 0) {
        sys->vsync_count += NAMCO_VSYNC_PERIOD;
        if (sys->int_enable) {
            pins |= Z80_INT;
        }
    }

    // tick the sound chip
    _namco_sound_tick(sys);

    // tick the cpu
    pins = z80_tick(&sys->cpu, pins);

    // memory requests
    uint16_t addr = Z80_GET_ADDR(pins) & NAMCO_ADDR_MASK;
    if (pins & Z80_MREQ) {
        if (pins & Z80_WR) {
            // memory write access
            uint8_t data = Z80_GET_DATA(pins);
            if (addr < NAMCO_IOMAP_BASE) {
                mem_wr(&sys->mem, addr, data);
            }
            else {
                // memory-mapped IO
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
            // memory read access
            if (addr < NAMCO_IOMAP_BASE) {
                Z80_SET_DATA(pins, mem_rd(&sys->mem, addr));
            }
            else {
                uint8_t data = 0xFF;
                // memory-mapped IO
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
                    case NAMCO_ADDR_DSW2:
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
                // OUT to port 0: set interrupt vector latch
                sys->int_vector = data;
            }
        }
        else if (pins & Z80_M1) {
            // an interrupt handling machine cycle, set interrupt vector on data bus
            // and clear the INT pin
            Z80_SET_DATA(pins, sys->int_vector);
            pins &= ~Z80_INT;
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

// 8x4 video tile decoder (used both for background tiles and sprites)
static inline void _namco_8x4(
    uint8_t* fb,
    uint8_t* tile_base,
    uint8_t* pal_base,
    uint8_t* pal_rom,
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
        int tile_index = char_code * tile_stride + tile_offset + yy;
        for (uint32_t xx = 0; xx < 4; xx++) {
            uint32_t x = px + (xx ^ xor_x);
            if (x >= NAMCO_DISPLAY_WIDTH) {
                continue;
            }
            uint8_t p2_hi = (tile_base[tile_index]>>(7-xx)) & 1;
            uint8_t p2_lo = (tile_base[tile_index]>>(3-xx)) & 1;
            uint8_t p2 = (p2_hi<<1)|p2_lo;
            uint8_t hw_color = pal_base[(color_code<<2)|p2];
            if (opaque || (pal_rom[hw_color] != 0)) {
                fb[y * NAMCO_FRAMEBUFFER_WIDTH + x] = hw_color;
            }
        }
    }
}

// decode background tiles
static void _namco_decode_chars(namco_t* sys) {
    uint8_t* pal_base = &sys->palette_cache[(sys->pal_select<<8)|(sys->clut_select<<7)];
    uint8_t* tile_base = &sys->rom_gfx[0x0000] + (sys->tile_select * 0x2000);
    for (uint32_t y = 0; y < 28; y++) {
        for (uint32_t x = 0; x < 36; x++) {
            uint16_t offset = _namco_video_offset(x, y);
            uint8_t char_code = sys->video_ram[offset];
            uint8_t color_code = sys->color_ram[offset] & 0x1F;
            _namco_8x4(sys->fb, tile_base, pal_base, sys->rom_prom, 16, 8, x*8, y*8, char_code, color_code, true, false, false);
            _namco_8x4(sys->fb, tile_base, pal_base, sys->rom_prom, 16, 0, x*8+4, y*8, char_code, color_code, true, false, false);
        }
    }
}

static void _namco_decode_sprites(namco_t* sys) {
    uint8_t* pal_base = &sys->palette_cache[(sys->pal_select<<8)|(sys->clut_select<<7)];
    uint8_t* tile_base = &sys->rom_gfx[0x1000] + (sys->tile_select * 0x2000);
    #if defined(NAMCO_PACMAN)
    const int max_sprite = 6;
    const int min_sprite = 1;
    #else
    const int max_sprite = 7;
    const int min_sprite = 0;
    #endif
    for (size_t sprite_index = max_sprite; sprite_index >= min_sprite; --sprite_index) {
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
        _namco_8x4(sys->fb, tile_base, pal_base, sys->rom_prom, 64, 8,  px+fx0, py+fy0, char_code, color_code, false, flip_x, flip_y);
        _namco_8x4(sys->fb, tile_base, pal_base, sys->rom_prom, 64, 16, px+fx1, py+fy0, char_code, color_code, false, flip_x, flip_y);
        _namco_8x4(sys->fb, tile_base, pal_base, sys->rom_prom, 64, 24, px+fx2, py+fy0, char_code, color_code, false, flip_x, flip_y);
        _namco_8x4(sys->fb, tile_base, pal_base, sys->rom_prom, 64, 0,  px+fx3, py+fy0, char_code, color_code, false, flip_x, flip_y);
        _namco_8x4(sys->fb, tile_base, pal_base, sys->rom_prom, 64, 40, px+fx0, py+fy1, char_code, color_code, false, flip_x, flip_y);
        _namco_8x4(sys->fb, tile_base, pal_base, sys->rom_prom, 64, 48, px+fx1, py+fy1, char_code, color_code, false, flip_x, flip_y);
        _namco_8x4(sys->fb, tile_base, pal_base, sys->rom_prom, 64, 56, px+fx2, py+fy1, char_code, color_code, false, flip_x, flip_y);
        _namco_8x4(sys->fb, tile_base, pal_base, sys->rom_prom, 64, 32, px+fx3, py+fy1, char_code, color_code, false, flip_x, flip_y);
    }
}

void _namco_decode_video(namco_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    _namco_decode_chars(sys);
    _namco_decode_sprites(sys);
}

uint32_t namco_exec(namco_t* sys, uint32_t micro_seconds) {
    CHIPS_ASSERT(sys && sys->valid);
    const uint32_t num_ticks = clk_us_to_ticks(NAMCO_CPU_CLOCK, micro_seconds);
    uint64_t pins = sys->pins;
    if (0 == sys->debug.callback.func) {
        // run without debug hook
        for (uint32_t tick = 0; tick < num_ticks; tick++) {
            pins = _namco_tick(sys, pins);
        }
    }
    else {
        // run with debug hook
        for (uint32_t tick = 0; (tick < num_ticks) && !(*sys->debug.stopped); tick++) {
            pins = _namco_tick(sys, pins);
            sys->debug.callback.func(sys->debug.callback.user_data, pins);
        }
    }
    sys->pins = pins;
    _namco_decode_video(sys);
    return num_ticks;
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

static void _namco_sound_init(namco_t* sys, const namco_desc_t* desc) {
    CHIPS_ASSERT(desc->audio.num_samples <= NAMCO_MAX_AUDIO_SAMPLES);
    // assume zero-initialized
    namco_sound_t* snd = &sys->sound;
    snd->tick_counter = NAMCO_SOUND_PERIOD;
    snd->sample_period = (NAMCO_CPU_CLOCK * NAMCO_SAMPLE_SCALE) / _namco_def(desc->audio.sample_rate, 44100);
    snd->sample_counter = sys->sound.sample_period;
    snd->volume = _namco_def(desc->audio.volume, 1.0f);
    snd->num_samples = _namco_def(desc->audio.num_samples, NAMCO_DEFAULT_AUDIO_SAMPLES);
    snd->callback = desc->audio.callback;
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

static void _namco_sound_tick(namco_t* sys) {
    namco_sound_t* snd = &sys->sound;
    // tick the sound chip?
    snd->tick_counter--;
    if (snd->tick_counter < 0) {
        // handle 96KHz tick
        snd->tick_counter += NAMCO_SOUND_PERIOD / NAMCO_SOUND_OVERSAMPLE;
        for (int i = 0; i < 3; i++) {
            if ((snd->voice[i].frequency > 0) && (sys->sound_enable & 1)) {
                snd->voice[i].counter += (snd->voice[i].frequency / NAMCO_SOUND_OVERSAMPLE);
                /* lookup current 4-bit sample from waveform number and the topmost 5
                   bits of the 20-bit sample counter, multiple with 4-bit volume
                */
                uint32_t smp_index = ((snd->voice[i].waveform<<5) | ((snd->voice[i].counter>>15) & 0x1F)) & 0xFF;
                // integer sample value now 7-bits plus sign bit
                int val = (((int)(snd->rom[0][smp_index] & 0xF)) - 8) * snd->voice[i].volume;
                snd->voice[i].sample += (float)val;
            }
            snd->voice[i].sample_div += 128.0f;
        }
    }

    // generate a new sample?
    snd->sample_counter -= NAMCO_SAMPLE_SCALE;
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
            if (snd->callback.func) {
                snd->callback.func(snd->sample_buffer, snd->num_samples, snd->callback.user_data);
            }
            snd->sample_pos = 0;
        }
    }
}

chips_display_info_t namco_display_info(namco_t* sys) {
    const chips_display_info_t res = {
        .frame = {
            .dim = {
                .width = NAMCO_FRAMEBUFFER_WIDTH,
                .height = NAMCO_FRAMEBUFFER_HEIGHT,
            },
            .bytes_per_pixel = 1,
            .buffer = {
                .ptr = sys ? sys->fb : 0,
                .size = NAMCO_FRAMEBUFFER_SIZE_BYTES,
            },
        },
        .screen = {
            .x = 0,
            .y = 0,
            .width = NAMCO_DISPLAY_WIDTH,
            .height = NAMCO_DISPLAY_HEIGHT,
        },
        .palette = {
            .ptr = sys ?  sys->hw_colors : 0,
            .size = 32 * sizeof(uint32_t)
        }
    };
    CHIPS_ASSERT(((sys == 0) && (res.frame.buffer.ptr == 0)) || ((sys != 0) && (res.frame.buffer.ptr != 0)));
    CHIPS_ASSERT(((sys == 0) && (res.palette.ptr == 0)) || ((sys != 0) && (res.palette.ptr != 0)));
    return res;
}

uint32_t namco_save_snapshot(namco_t* sys, namco_t* dst) {
    CHIPS_ASSERT(sys && dst);
    *dst = *sys;
    dst->debug.callback.func = 0;
    dst->debug.callback.user_data = 0;
    dst->debug.stopped = 0;
    dst->sound.callback.func = 0;
    dst->sound.callback.user_data = 0;
    mem_pointers_to_offsets(&dst->mem, sys);
    return NAMCO_SNAPSHOT_VERSION;
}

bool namco_load_snapshot(namco_t* sys, uint32_t version, namco_t* src) {
    CHIPS_ASSERT(sys && src);
    if (version != NAMCO_SNAPSHOT_VERSION) {
        return false;
    }
    static namco_t im;
    im = *src;
    im.debug = sys->debug;
    im.sound.callback = sys->sound.callback;
    mem_offsets_to_pointers(&im.mem, sys);
    *sys = im;
    return true;
}

#endif // CHIPS_IMPL
