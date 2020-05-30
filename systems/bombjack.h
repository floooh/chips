#pragma once
/*#
    # bombjack.h

    Bomb Jack arcade machine emulator in a C header.

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

    You need to include the following headers before including bombjack.h:

    - chips/z80.h
    - chips/ay38910.h
    - chips/clk.h
    - chips/mem.h

    ## The Bomb Jack Arcade Machine

    See: 
    
        - https://floooh.github.io/2018/10/06/bombjack.html
        - https://github.com/floooh/emu-info/blob/master/misc/bombjack-schematics.pdf
        
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

#define BOMBJACK_MAX_AUDIO_SAMPLES (1024)
#define BOMBJACK_DEFAULT_AUDIO_SAMPLES (128)

/* joystick mask bits */
#define BOMBJACK_JOYSTICK_RIGHT (1<<0)
#define BOMBJACK_JOYSTICK_LEFT (1<<1)
#define BOMBJACK_JOYSTICK_UP (1<<2)
#define BOMBJACK_JOYSTICK_DOWN (1<<3)
#define BOMBJACK_JOYSTICK_BUTTON (1<<4)

/* system bits (bombjack_t.main.sys) */
#define BOMBJACK_SYS_P1_COIN    (1<<0)      /* player 1 coin slot */
#define BOMBJACK_SYS_P2_COIN    (1<<1)      /* player 2 coin slot*/
#define BOMBJACK_SYS_P1_START   (1<<2)      /* player 1 start button */
#define BOMBJACK_SYS_P2_START   (1<<2)      /* player 2 start button */

/* DIP switches 1 (bombjack.main.dsw1) */
#define BOMBJACK_DSW1_P1_MASK           (3)
#define BOMBJACK_DSW1_P1_1COIN_1PLAY    (0)
#define BOMBJACK_DSW1_P1_1COIN_2PLAY    (1)
#define BOMBJACK_DSW1_P1_1COIN_3PLAY    (2)
#define BOMBJACK_DSW1_P1_1COIN_5PLAY    (3)

#define BOMBJACK_DSW1_P2_MASK           (3<<2)
#define BOMBJACK_DSW1_P2_1COIN_1PLAY    (0)
#define BOMBJACK_DSW1_P2_1COIN_2PLAY    (1<<2)
#define BOMBJACK_DSW1_P2_1COIN_3PLAY    (2<<2)
#define BOMBJACK_DSW1_P2_1COIN_5PLAY    (3<<2)

#define BOMBJACK_DSW1_JACKS_MASK        (3<<4)
#define BOMBJACK_DSW1_JACKS_3           (0)
#define BOMBJACK_DSW1_JACKS_4           (1<<4)
#define BOMBJACK_DSW1_JACKS_5           (2<<4)
#define BOMBJACK_DSW1_JACKS_2           (3<<4)

#define BOMBJACK_DSW1_CABINET_MASK      (1<<6)
#define BOMBJACK_DSW1_CABINET_COCKTAIL  (0)
#define BOMBJACK_DSW1_CABINET_UPRIGHT   (1<<6)

#define BOMBJACK_DSW1_DEMOSOUND_MASK    (1<<7)
#define BOMBJACK_DSW1_DEMOSOUND_OFF     (0)
#define BOMBJACK_DSW1_DEMOSOUND_ON      (1<<7)

/* DIP switches 2 (bombjack.main.dsw2) */
#define BOMBJACK_DSW2_BIRDSPEED_MASK        (3<<3)
#define BOMBJACK_DSW2_BIRDSPEED_EASY        (0)
#define BOMBJACK_DSW2_BIRDSPEED_MODERATE    (1<<3)
#define BOMBJACK_DSW2_BIRDSPEED_HARD        (2<<3)
#define BOMBJACK_DSW2_BIRDSPEED_HARDER      (3<<3)

#define BOMBJACK_DSW2_DIFFICULTY_MASK       (3<<5)
#define BOMBJACK_DSW2_DIFFICULTY_MODERATE   (0)
#define BOMBJACK_DSW2_DIFFICULTY_EASY       (1<<5)
#define BOMBJACK_DSW2_DIFFICULTY_HARD       (2<<5)
#define BOMBJACK_DSW2_DIFFICULTY_HARDER     (3<<5)

#define BOMBJACK_DSW2_SPECIALCOIN_MASK      (1<<7)
#define BOMBJACK_DSW2_SPECIALCOIN_EASY      (0)
#define BOMBJACK_DSW2_SPECIALCOIN_HARD      (1<<7)

/* default DIP-switch configuration */
#define BOMBJACK_DSW1_DEFAULT (BOMBJACK_DSW1_CABINET_UPRIGHT|BOMBJACK_DSW1_DEMOSOUND_ON)
#define BOMBJACK_DSW2_DEFAULT (BOMBJACK_DSW2_DIFFICULTY_EASY)

/* audio sample-data callback */
typedef void (*bombjack_audio_callback_t)(const float* samples, int num_samples, void* user_data);

/* configuration parameters for bombjack_init() */
typedef struct {
    /* video output config */
    void* pixel_buffer;         /* pointer to a linear RGBA8 pixel buffer, at least 256*256*4 bytes */
    int pixel_buffer_size;      /* size of the pixel buffer in bytes */

    /* optional user-data for audio callback */
    void* user_data;

    /* audio output config (if you don't want audio, set audio_cb to zero) */
    bombjack_audio_callback_t audio_cb;     /* called when audio_num_samples are ready */
    int audio_num_samples;                  /* default is BOMBJACK_DEFAULT_AUDIO_SAMPLES */
    int audio_sample_rate;                  /* playback sample rate, default is 44100 */
    float audio_volume;                     /* audio volume, 0.0..1.0, default is 1.0 */

    /* ROM images */
    const void* rom_main_0000_1FFF;     /* main-board ROM 0x0000..0x1FFF */
    const void* rom_main_2000_3FFF;     /* main-board ROM 0x2000..0x3FFF */
    const void* rom_main_4000_5FFF;     /* main-board ROM 0x4000..0x5FFF */
    const void* rom_main_6000_7FFF;     /* main-board ROM 0x6000..0x7FFF */
    const void* rom_main_C000_DFFF;     /* main-board ROM 0xC000..0xDFFF */
    const void* rom_sound_0000_1FFF;    /* sound-board ROM 0x0000..0x2000 */
    const void* rom_chars_0000_0FFF;    /* char ROM 0x0000..0x0FFF */
    const void* rom_chars_1000_1FFF;    /* char ROM 0x1000..0x1FFF */
    const void* rom_chars_2000_2FFF;    /* char ROM 0x2000..0x2FFF */
    const void* rom_tiles_0000_1FFF;    /* tile ROM 0x0000..0x1FFF */
    const void* rom_tiles_2000_3FFF;    /* tile ROM 0x2000..0x3FFF */
    const void* rom_tiles_4000_5FFF;    /* tile ROM 0x4000..0x5FFF */
    const void* rom_sprites_0000_1FFF;  /* sprite ROM 0x0000..0x1FFF */
    const void* rom_sprites_2000_3FFF;  /* sprite ROM 0x2000..0x3FFF */
    const void* rom_sprites_4000_5FFF;  /* sprite ROM 0x4000..0x5FFF */
    const void* rom_maps_0000_0FFF;     /* map ROM 0x0000..0x0FFF */
    int rom_main_0000_1FFF_size;
    int rom_main_2000_3FFF_size;
    int rom_main_4000_5FFF_size;
    int rom_main_6000_7FFF_size;
    int rom_main_C000_DFFF_size;
    int rom_sound_0000_1FFF_size;
    int rom_chars_0000_0FFF_size;
    int rom_chars_1000_1FFF_size;
    int rom_chars_2000_2FFF_size;
    int rom_tiles_0000_1FFF_size;
    int rom_tiles_2000_3FFF_size;
    int rom_tiles_4000_5FFF_size;
    int rom_sprites_0000_1FFF_size;
    int rom_sprites_2000_3FFF_size;
    int rom_sprites_4000_5FFF_size;
    int rom_maps_0000_0FFF_size;
} bombjack_desc_t;

/* the whole Bomb Jack arcade machine state */
typedef struct {
    bool valid;
    struct {
        z80_t cpu;
        clk_t clk;
        uint8_t p1;     /* joystick 1 state */
        uint8_t p2;     /* joystick 2 state */
        uint8_t sys;            /* coins and start buttons */
        uint8_t dsw1;           /* dip-switches 1 */
        uint8_t dsw2;           /* dip-switches 2 */
        uint8_t nmi_mask;       /* if 0, no NMIs are generated */
        uint8_t bg_image;       /* current background image */
        int vsync_count;
        int vblank_count;
        mem_t mem;
        uint32_t palette[128];
    } mainboard;
    struct {
        z80_t cpu;
        clk_t clk;
        ay38910_t psg[3];
        uint32_t tick_count;
        int vsync_count;
        mem_t mem;
    } soundboard;
    uint8_t sound_latch;            /* shared latch, written by main board, read by sound board */
    uint8_t main_ram[0x1C00];
    uint8_t sound_ram[0x0400];
    uint8_t rom_main[5][0x2000];
    uint8_t rom_sound[1][0x2000];
    uint8_t rom_chars[3][0x1000];
    uint8_t rom_tiles[3][0x2000];
    uint8_t rom_sprites[3][0x2000];
    uint8_t rom_maps[1][0x1000];
    void* user_data;
    /* audio and video 'rendering' */
    struct {
        bombjack_audio_callback_t callback;
        int num_samples;
        int sample_pos;
        float volume;
        float sample_buffer[BOMBJACK_MAX_AUDIO_SAMPLES];
    } audio;
    uint32_t* pixel_buffer;
    struct {
        bool draw_background_layer;
        bool draw_foreground_layer;
        bool draw_sprite_layer;
        bool clear_background_layer;
    } dbg;
} bombjack_t;

/* initialize a new bombjack instance */
void bombjack_init(bombjack_t* sys, const bombjack_desc_t* desc);
/* discard a bombjack instance */
void bombjack_discard(bombjack_t* sys);
/* reset a bombjack instance */
void bombjack_reset(bombjack_t* sys);
/* run bombjack instance for given amount of microseconds */
void bombjack_exec(bombjack_t* sys, uint32_t micro_seconds);
/* decode video to pixel buffer, must be called once per frame */
void bombjack_decode_video(bombjack_t* sys);
/* get the standard framebuffer width and height in pixels */
int bombjack_std_display_width(void);
int bombjack_std_display_height(void);
/* get the maximum framebuffer size in number of bytes */
int bombjack_max_display_size(void);
/* get the current framebuffer width and height in pixels */
int bombjack_display_width(bombjack_t* sys);
int bombjack_display_height(bombjack_t* sys);

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

#define _BOMBJACK_MAINBOARD_FREQUENCY (4000000)
#define _BOMBJACK_SOUNDBOARD_FREQUENCY (3000000)
#define _BOMBJACK_VSYNC_PERIOD_4MHZ (4000000/60)
#define _BOMBJACK_VBLANK_DURATION_4MHZ (((4000000/60)/525)*(525-483))
#define _BOMBJACK_VSYNC_PERIOD_3MHZ (3000000/60)
#define _BOMBJACK_DISPLAY_WIDTH (256)
#define _BOMBJACK_DISPLAY_HEIGHT (256)
#define _BOMBJACK_DISPLAY_SIZE (_BOMBJACK_DISPLAY_WIDTH*_BOMBJACK_DISPLAY_HEIGHT*4)

static uint64_t _bombjack_tick_mainboard(int num, uint64_t pins, void* user_data);
static uint64_t _bombjack_tick_soundboard(int num, uint64_t pins, void* user_data);

#define _bombjack_def(val, def) (val == 0 ? def : val)

void bombjack_init(bombjack_t* sys, const bombjack_desc_t* desc) {
    CHIPS_ASSERT(sys && desc);
    
    memset(sys, 0, sizeof(bombjack_t));
    sys->valid = true;
    sys->dbg.draw_background_layer = true;
    sys->dbg.draw_foreground_layer = true;
    sys->dbg.draw_sprite_layer = true;
    sys->dbg.clear_background_layer = true;

    /* copy over ROM images */
    CHIPS_ASSERT(desc->rom_main_0000_1FFF && (desc->rom_main_0000_1FFF_size == sizeof(sys->rom_main[0])));
    CHIPS_ASSERT(desc->rom_main_2000_3FFF && (desc->rom_main_2000_3FFF_size == sizeof(sys->rom_main[1])));
    CHIPS_ASSERT(desc->rom_main_4000_5FFF && (desc->rom_main_4000_5FFF_size == sizeof(sys->rom_main[2])));
    CHIPS_ASSERT(desc->rom_main_6000_7FFF && (desc->rom_main_6000_7FFF_size == sizeof(sys->rom_main[3])));
    CHIPS_ASSERT(desc->rom_main_C000_DFFF && (desc->rom_main_C000_DFFF_size == sizeof(sys->rom_main[4])));
    CHIPS_ASSERT(desc->rom_sound_0000_1FFF && (desc->rom_sound_0000_1FFF_size == sizeof(sys->rom_sound[0])));
    CHIPS_ASSERT(desc->rom_chars_0000_0FFF && (desc->rom_chars_0000_0FFF_size == sizeof(sys->rom_chars[0])));
    CHIPS_ASSERT(desc->rom_chars_1000_1FFF && (desc->rom_chars_1000_1FFF_size == sizeof(sys->rom_chars[1])));
    CHIPS_ASSERT(desc->rom_chars_2000_2FFF && (desc->rom_chars_2000_2FFF_size == sizeof(sys->rom_chars[2])));
    CHIPS_ASSERT(desc->rom_tiles_0000_1FFF && (desc->rom_tiles_0000_1FFF_size == sizeof(sys->rom_tiles[0])));
    CHIPS_ASSERT(desc->rom_tiles_2000_3FFF && (desc->rom_tiles_2000_3FFF_size == sizeof(sys->rom_tiles[1])));
    CHIPS_ASSERT(desc->rom_tiles_4000_5FFF && (desc->rom_tiles_4000_5FFF_size == sizeof(sys->rom_tiles[2])));
    CHIPS_ASSERT(desc->rom_sprites_0000_1FFF && (desc->rom_sprites_0000_1FFF_size == sizeof(sys->rom_sprites[0])));
    CHIPS_ASSERT(desc->rom_sprites_2000_3FFF && (desc->rom_sprites_2000_3FFF_size == sizeof(sys->rom_sprites[1])));
    CHIPS_ASSERT(desc->rom_sprites_4000_5FFF && (desc->rom_sprites_4000_5FFF_size == sizeof(sys->rom_sprites[2])));
    CHIPS_ASSERT(desc->rom_maps_0000_0FFF && (desc->rom_maps_0000_0FFF_size == sizeof(sys->rom_maps[0])));
    memcpy(sys->rom_main[0], desc->rom_main_0000_1FFF, sizeof(sys->rom_main[0]));
    memcpy(sys->rom_main[1], desc->rom_main_2000_3FFF, sizeof(sys->rom_main[1]));
    memcpy(sys->rom_main[2], desc->rom_main_4000_5FFF, sizeof(sys->rom_main[2]));
    memcpy(sys->rom_main[3], desc->rom_main_6000_7FFF, sizeof(sys->rom_main[3]));
    memcpy(sys->rom_main[4], desc->rom_main_C000_DFFF, sizeof(sys->rom_main[4]));
    memcpy(sys->rom_sound[0], desc->rom_sound_0000_1FFF, sizeof(sys->rom_sound[0]));
    memcpy(sys->rom_chars[0], desc->rom_chars_0000_0FFF, sizeof(sys->rom_chars[0]));
    memcpy(sys->rom_chars[1], desc->rom_chars_1000_1FFF, sizeof(sys->rom_chars[1]));
    memcpy(sys->rom_chars[2], desc->rom_chars_2000_2FFF, sizeof(sys->rom_chars[2]));
    memcpy(sys->rom_tiles[0], desc->rom_tiles_0000_1FFF, sizeof(sys->rom_tiles[0]));
    memcpy(sys->rom_tiles[1], desc->rom_tiles_2000_3FFF, sizeof(sys->rom_tiles[1]));
    memcpy(sys->rom_tiles[2], desc->rom_tiles_4000_5FFF, sizeof(sys->rom_tiles[2]));
    memcpy(sys->rom_sprites[0], desc->rom_sprites_0000_1FFF, sizeof(sys->rom_sprites[0]));
    memcpy(sys->rom_sprites[1], desc->rom_sprites_2000_3FFF, sizeof(sys->rom_sprites[1]));
    memcpy(sys->rom_sprites[2], desc->rom_sprites_4000_5FFF, sizeof(sys->rom_sprites[2]));
    memcpy(sys->rom_maps[0], desc->rom_maps_0000_0FFF, sizeof(sys->rom_maps[0]));

    /* The VSYNC/VBLANK mainly controls the interrupts (Bombjack generally
        uses NMIs for simplicity. The mainboard's NMI is connected to the
        VBLANK signal, and stays active for the VBLANK period, the
        soundboard's NMI is connected to a flip-flop which is switched
        active when VSYNC happens, and deactivated when the interrupt
        service routine reads from the shared sound-command latch
        (mapped into the mainboard CPU's address space at 0xB800, and
        the sound board's address space at 0x6000).

        NOTE: it's important that the machine doesn't start right off with a vsync
        when it is switched on, since this would cause an NMI before
        the stack pointer is setup!)
    */
    sys->mainboard.vsync_count = _BOMBJACK_VSYNC_PERIOD_4MHZ;
    sys->mainboard.vblank_count = 0;
    sys->soundboard.vsync_count = _BOMBJACK_VSYNC_PERIOD_3MHZ;

    /* setup the main board (4 MHz Z80) */
    clk_init(&sys->mainboard.clk, _BOMBJACK_MAINBOARD_FREQUENCY);
    z80_desc_t cpu_desc;
    memset(&cpu_desc, 0, sizeof(cpu_desc));
    cpu_desc.tick_cb = _bombjack_tick_mainboard;
    cpu_desc.user_data = sys;
    z80_init(&sys->mainboard.cpu, &cpu_desc);

    /* setup the sound board (3 MHz Z80 and 3x 1.5 MHz AY-38910) */
    clk_init(&sys->soundboard.clk, _BOMBJACK_SOUNDBOARD_FREQUENCY);
    memset(&cpu_desc, 0, sizeof(cpu_desc));
    cpu_desc.tick_cb = _bombjack_tick_soundboard;
    cpu_desc.user_data = sys;
    z80_init(&sys->soundboard.cpu, &cpu_desc);
    ay38910_desc_t psg_desc;
    memset(&psg_desc, 0, sizeof(psg_desc));
    psg_desc.type = AY38910_TYPE_8910;
    psg_desc.tick_hz = 1500000;
    psg_desc.sound_hz = _bombjack_def(desc->audio_sample_rate, 44100);
    psg_desc.magnitude = 0.2f;
    for (int i = 0; i < 3; i++) {
        ay38910_init(&sys->soundboard.psg[i], &psg_desc);
    }

    /* dip switches */
    sys->mainboard.dsw1 = BOMBJACK_DSW1_DEFAULT;
    sys->mainboard.dsw2 = BOMBJACK_DSW2_DEFAULT;

    /* main board memory map:
        0000..7FFF: ROM
        8000..8FFF: RAM
        9000..93FF: video ram
        9400..97FF: color ram
        9820..987F: sprite ram
        9C00..9CFF: palette ram (write-only?)
        9E00:       select background (write-only?)
        B000:       read: joystick 1, write: NMI mask
        B001:       read: joystick 2
        B002:       read: coins and start button
        B003:       read/write: watchdog reset (not emulated)
        B004:       read: dip-switches 1, write: flip screen
        B005:       read: dip-switches 2
        B800:       sound latch
        C000..DFFF: ROM

      palette RAM is 128 entries with 16-bit per entry (xxxxBBBBGGGGRRRR).
    
      NOTE that ROM data that's not accessible by CPU isn't accessed
      through a memory mapper.
    */
    mem_init(&sys->mainboard.mem);
    mem_map_rom(&sys->mainboard.mem, 0, 0x0000, 0x2000, sys->rom_main[0]);
    mem_map_rom(&sys->mainboard.mem, 0, 0x2000, 0x2000, sys->rom_main[1]);
    mem_map_rom(&sys->mainboard.mem, 0, 0x4000, 0x2000, sys->rom_main[2]);
    mem_map_rom(&sys->mainboard.mem, 0, 0x6000, 0x2000, sys->rom_main[3]);
    mem_map_ram(&sys->mainboard.mem, 0, 0x8000, 0x1C00, sys->main_ram);
    mem_map_rom(&sys->mainboard.mem, 0, 0xC000, 0x2000, sys->rom_main[4]);

    /* sound board memory map */
    mem_init(&sys->soundboard.mem);
    mem_map_rom(&sys->soundboard.mem, 0, 0x0000, 0x2000, sys->rom_sound[0]);
    mem_map_ram(&sys->soundboard.mem, 0, 0x4000, 0x0400, sys->sound_ram);

    /* move over audio- and video config parameters */
    CHIPS_ASSERT(desc->audio_num_samples <= BOMBJACK_MAX_AUDIO_SAMPLES);
    sys->audio.callback = desc->audio_cb;
    sys->audio.num_samples = _bombjack_def(desc->audio_num_samples, BOMBJACK_DEFAULT_AUDIO_SAMPLES);
    sys->audio.volume = _bombjack_def(desc->audio_volume, 1.0f);
    sys->user_data = desc->user_data;
    CHIPS_ASSERT((0 == desc->pixel_buffer) || (desc->pixel_buffer && (desc->pixel_buffer_size >= _BOMBJACK_DISPLAY_SIZE)));
    sys->pixel_buffer = (uint32_t*) desc->pixel_buffer;
}

void bombjack_discard(bombjack_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    sys->valid = false;
}

int bombjack_std_display_width(void) {
    return _BOMBJACK_DISPLAY_WIDTH;
}

int bombjack_std_display_height(void) {
    return _BOMBJACK_DISPLAY_HEIGHT;
}

int bombjack_display_size(void) {
    return _BOMBJACK_DISPLAY_SIZE;
}

int bombjack_display_width(bombjack_t* sys) {
    (void)sys;
    return _BOMBJACK_DISPLAY_WIDTH;
}

int bombjack_display_height(bombjack_t* sys) {
    (void)sys;
    return _BOMBJACK_DISPLAY_HEIGHT;
}

void bombjack_reset(bombjack_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    z80_reset(&sys->mainboard.cpu);
    z80_reset(&sys->soundboard.cpu);
    for (int i = 0; i < 3; i++) {
        ay38910_reset(&sys->soundboard.psg[i]);
    }
}

void bombjack_exec(bombjack_t* sys, uint32_t micro_seconds) {
    CHIPS_ASSERT(sys && sys->valid);
    /* Run the main board and sound board interleaved for half a frame.
       This simplifies the communication via the sound latch (the main CPU
       writes a command byte to the sound latch, the sound board reads
       the command latch in its interrupt service routine.

       The main board issues at most one command per 60Hz frame, but since the
       host machine is also running at roughly 60 Hz it may happen that the
       main board writes 2 sound commands per host frame. For this reason
       run the 2 boards interleaved for half a frame, so it is guaranteed
       that at most one sound command can be written by the main board
       before the sound board is ticked (that way we don't need to implement
       a complicated command queue.
    */
    const uint32_t slice_us = micro_seconds/2;
    for (int i = 0; i < 2; i++) {
        /* tick the main board */
        {
            uint32_t ticks_to_run = clk_ticks_to_run(&sys->mainboard.clk, slice_us);
            uint32_t ticks_executed = z80_exec(&sys->mainboard.cpu, ticks_to_run);
            clk_ticks_executed(&sys->mainboard.clk, ticks_executed);
        }
        /* tick the sound board */
        {
            uint32_t ticks_to_run = clk_ticks_to_run(&sys->soundboard.clk, slice_us);
            uint32_t ticks_executed = z80_exec(&sys->soundboard.cpu, ticks_to_run);
            clk_ticks_executed(&sys->soundboard.clk, ticks_executed);
        }
    }
}

/* Maintain a color palette cache with 32-bit colors, this is called for
    CPU writes to the palette RAM area. The hardware palette is 128
    entries of 16-bit colors (xxxxBBBBGGGGRRRR), the function keeps
    a 'palette cache' with 32-bit colors uptodate, so that the
    32-bit colors don't need to be computed for each pixel in the
    video decoding code.
*/
static inline void _bombjack_update_palette_cache(bombjack_t* sys, uint16_t addr, uint8_t data) {
    assert((addr >= 0x9C00) && (addr < 0x9D00));
    int pal_index = (addr - 0x9C00) / 2;
    uint32_t c = sys->mainboard.palette[pal_index];
    if (addr & 1) {
        /* uneven addresses are the xxxxBBBB part */
        uint8_t b = (data & 0x0F) | ((data<<4)&0xF0);
        c = 0xFF000000 | (c & 0x0000FFFF) | (b<<16);
    }
    else {
        /* even addresses are the GGGGRRRR part */
        uint8_t g = (data & 0xF0) | ((data>>4)&0x0F);
        uint8_t r = (data & 0x0F) | ((data<<4)&0xF0);
        c = 0xFF000000 | (c & 0x00FF0000) | (g<<8) | r;
    }
    sys->mainboard.palette[pal_index] = c;
}

/* main board tick callback

    Bomb Jack uses memory mapped IO (the Z80's IORQ pin isn't connected).

    Special memory locations:

    9C00..9D00: the hardware color palette (128 entries @ 16-bit)
                I'm not sure if this area is normal memory and readable,
                the CPU doesn't appear to do any read accesses, write accesses
                are caught and used to update the color palette cache.
    B000:       read:  player 1 joystick state
                        bit 0: right
                            1: left
                            2: up
                            3: down
                            4: btn
                write: NMI mask (NMI on VSYNC disabled when 0 written to B000)
    B001:       read:  player 2 joystick state
                write: ???
    B002:       read:  coin detector and start button:
                        bit 0: player 1 coin
                            1: player 2 coin
                            2: player 1 start button
                            3: player 2 start button
                write:  ???
    B003:       read/write: connected to a WDCLR (watchdog clear?) line which together
                       with the VBLANK are connected to the RESET pin, the
                       game code seems to read B003 quite often, I guess
                       if those read or the VBLANK are not happening for
                       a little while because of a software or hardware
                       failure, the machine resets itself
    B004:       read:  dip-switches 1
                        bits [1,0]: 00: 1 COIN 1 PLAY (player 1)
                                    01: 1 COIN 2 PLAY
                                    10: 1 COIN 3 PLAY
                                    11: 1 COIN 5 PLAY
                             [3,2]: coin/play for player 2
                             [5,4]: 00: 3 Jacks
                                    01: 4 Jacks
                                    10: 5 Jacks
                                    11: 2 Jacks
                             6:     0: cocktail
                                    1: upright
                             7:     0: no demo sound
                                    1: demo sound
                write: flip-screen (not emulated)
    B005:       read: dip-switches 2
                        bits [4,3]: difficulty 1 (bird speed)
                                    00: moderate
                                    01: difficult
                                    10: more difficult
                                    11: top difficult
                        bits [6,5]: difficulty 2 (enemy number & speed)
                                    00: moderate
                                    01: easy
                                    10: difficult
                                    11: more difficult
                        7:          ratio of special coin appearance
                                    0:  easy
                                    1:  difficult
    B800:       sound command latch,

*/
static uint64_t _bombjack_tick_mainboard(int num_ticks, uint64_t pins, void* user_data) {
    bombjack_t* sys = (bombjack_t*) user_data;

    /* activate NMI pin during VBLANK */
    sys->mainboard.vsync_count -= num_ticks;
    if (sys->mainboard.vsync_count < 0) {
        sys->mainboard.vsync_count += _BOMBJACK_VSYNC_PERIOD_4MHZ;
        sys->mainboard.vblank_count = _BOMBJACK_VBLANK_DURATION_4MHZ;
    }
    if (sys->mainboard.vblank_count != 0) {
        sys->mainboard.vblank_count -= num_ticks;
        if (sys->mainboard.vblank_count < 0) {
            sys->mainboard.vblank_count = 0;
        }
    }
    if (sys->mainboard.nmi_mask && (sys->mainboard.vblank_count > 0)) {
        pins |= Z80_NMI;
    }
    else {
        pins &= ~Z80_NMI;
    }

    /* handle memory requests

        In hardware, the address decoding is mostly implemented
        with cascaded 1-in-4 and 1-in-8 decoder chips. We'll take
        a little shortcut and just check for the expected address ranges.
    */
    uint16_t addr = Z80_GET_ADDR(pins);
    if (pins & Z80_MREQ) {
        if (pins & Z80_WR) {
            /* memory write access */
            uint8_t data = Z80_GET_DATA(pins);
            if ((addr >= 0x8000) && (addr < 0x9900)) {
                /* regular RAM, video/color RAM, sprite RAM */
                mem_wr(&sys->mainboard.mem, addr, data);
            }
            else if ((addr >= 0x9C00) && (addr < 0x9D00)) {
                /* color palette */
                _bombjack_update_palette_cache(sys, addr, data);
            }
            else if (addr == 0x9E00) {
                /* background image selection */
                sys->mainboard.bg_image = data;
            }
            else if (addr == 0xB000) {
                /* NMI mask */
                sys->mainboard.nmi_mask = data;
            }
            /* FIXME: 0xB004: flip screen */
            else if (addr == 0xB800) {
                /* shared sound latch */
                sys->sound_latch = data;
            }
        }
        else if (pins & Z80_RD) {
            /* memory read access */
            if ((addr >= 0xB000) && (addr <= 0xB005)) {
                /* IO ports */
                switch (addr) {
                    case 0xB000: Z80_SET_DATA(pins, sys->mainboard.p1); break;
                    case 0xB001: Z80_SET_DATA(pins, sys->mainboard.p2); break;
                    case 0xB002: Z80_SET_DATA(pins, sys->mainboard.sys); break;
                    case 0xB004: Z80_SET_DATA(pins, sys->mainboard.dsw1); break;
                    case 0xB005: Z80_SET_DATA(pins, sys->mainboard.dsw2); break;
                }
            }
            else {
                /* regular memory */
                Z80_SET_DATA(pins, mem_rd(&sys->mainboard.mem, addr));
            }
        }
    }
    /* the Z80 IORQ pin isn't connected, so no IO instructions need to be handled */
    return pins & Z80_PIN_MASK;
}

/* sound board tick callback

    The sound board receives commands from the main board via the shared
    sound command latch (mapped to address 0xB800 on the main board, and
    address 0x6000 on the sound board).

    The sound board reads the command latch at address in the interrupt
    handler (the NMI pin is activated by the VSYNC signal), and only
    copies the byte to RAM location 0x4391, and then sets the first bit
    in location 0x4390. The soundboard's "main loop" loops on the
    bit in 0x4390 which means a new sound command has been received.

    Reading the 8-bit latch at address 0x6000 automatically clears the
    latch and also clears the flip-flop connected to the soundboard's
    CPU NMI pin.

    Communication with the 3 sound chips is done through IO requests
    (not memory mapped IO like on the main board).

    The memory map of the sound board is as follows:

    0000 .. 1FFF    ROM
    4000 .. 43FF    RAM
    6000            shared sound latch

    The IO map:

    00 .. 01:       1st AY-3-8910
    10 .. 11:       2nd AY-3-8910
    80 .. 81:       3rd AY-3-8910
*/
static uint64_t _bombjack_tick_soundboard(int num_ticks, uint64_t pins, void* user_data) {
    bombjack_t* sys = (bombjack_t*) user_data;

    /* vsync triggers a flip-flop connected to the CPU's NMI, the flip-flop
       is reset on a read from address 0x6000 (this read happens in the
       interrupt service routine
    */
    sys->soundboard.vsync_count -= num_ticks;
    if (sys->soundboard.vsync_count < 0) {
        sys->soundboard.vsync_count += _BOMBJACK_VSYNC_PERIOD_3MHZ;
        pins |= Z80_NMI;
    }

    /* tick the 3 sound chips at half frequency */
    for (int i = 0; i < num_ticks; i++) {
        if (sys->soundboard.tick_count++ & 1) {
            ay38910_tick(&sys->soundboard.psg[2]);
            ay38910_tick(&sys->soundboard.psg[1]);
            if (ay38910_tick(&sys->soundboard.psg[0])) {
                /* new sample ready */
                float s = sys->soundboard.psg[0].sample +
                          sys->soundboard.psg[1].sample + 
                          sys->soundboard.psg[2].sample;
                sys->audio.sample_buffer[sys->audio.sample_pos++] = s * sys->audio.volume;
                if (sys->audio.sample_pos == sys->audio.num_samples) {
                    if (sys->audio.callback) {
                        sys->audio.callback(sys->audio.sample_buffer, sys->audio.num_samples, sys->user_data);
                    }
                    sys->audio.sample_pos = 0;
                }
            }
        }
    }

    const uint16_t addr = Z80_GET_ADDR(pins);
    if (pins & Z80_MREQ) {
        /* memory requests */
        if (pins & Z80_RD) {
            /* special case: read and clear sound latch and NMI flip-flop */
            if (addr == 0x6000) {
                Z80_SET_DATA(pins, sys->sound_latch);
                sys->sound_latch = 0;
                pins &= ~Z80_NMI;
            }
            else {
                /* regular memory read */
                Z80_SET_DATA(pins, mem_rd(&sys->soundboard.mem, addr));
            }
        }
        else if (pins & Z80_WR) {
            /* regular memory write */
            mem_wr(&sys->soundboard.mem, addr, Z80_GET_DATA(pins));
        }
    }
    else if (pins & Z80_IORQ) {
        /* IO requests, schematics page 9 and 10

            PSG1, PSG2 and PSG3 are selected through a
            LS-138 1-of-4 decoder from address lines 4 and 7:

            A7 A4
             0  0   -> PSG 1
             0  1   -> PSG 2
             1  0   -> PSG 3
             1  1   -> not connected

            A0 is connected to BC1(!) (I guess that's an error in the
            schematics since these show BC2).
        */
        int psg_index = ((pins&Z80_A7)>>6) | ((pins&Z80_A4)>>4);
        if (psg_index < 4) {
            uint64_t psg_pins = (pins & Z80_PIN_MASK);
            if (pins & Z80_WR) {
                psg_pins |= AY38910_BDIR;
            }
            if (0 == (addr & 1)) {
                psg_pins |= AY38910_BC1;
            }
            pins = ay38910_iorq(&sys->soundboard.psg[psg_index], psg_pins);
        }
    }
    return pins & Z80_PIN_MASK;
}

/* render background tiles

    Background tiles are 16x16 pixels, and the screen is made of
    16x16 tiles. A background images consists of 16x16=256 tile
    'char codes', followed by 256 color code bytes. So each background
    image occupies 512 (0x200) bytes in the 'map rom'.

    The map-rom is 4 KByte, room for 8 background images (although I'm
    not sure yet whether all 8 are actually used). The background
    image number is written to address 0x9E00 (only the 3 LSB bits are
    considered). If bit 4 is cleared, no background image is shown
    (all tile codes are 0).

    A tile's image is created from 3 bitmaps, each bitmap stored in
    32 bytes with the following layout (the numbers are the byte index,
    each byte contains the bitmap pattern for 8 pixels):

    0: +--------+   8: +--------+
    1: +--------+   9: +--------+
    2: +--------+   10:+--------+
    3: +--------+   11:+--------+
    4: +--------+   12:+--------+
    5: +--------+   13:+--------+
    6: +--------+   14:+--------+
    7: +--------+   15:+--------+

    16:+--------+   24:+--------+
    17:+--------+   25:+--------+
    18:+--------+   26:+--------+
    19:+--------+   27:+--------+
    20:+--------+   28:+--------+
    21:+--------+   29:+--------+
    22:+--------+   30:+--------+
    23:+--------+   31:+--------+

    The 3 bitmaps for each tile are 8 KBytes apart (basically each
    of the 3 background-tile ROM chips contains one set of bitmaps
    for all 256 tiles).

    The 3 bitmaps are combined to get the lower 3 bits of the
    color palette index. The remaining 4 bits of the palette
    index are provided by the color attribute byte (for 7 bits
    = 128 color palette entries).

    This is how a color palette entry is constructed from the 4
    attribute bits, and 3 tile bitmap bits:

    |x|attr3|attr2|attr1|attr0|bm0|bm1|bm2|

    This basically means that each 16x16 background tile
    can select one of 16 color blocks from the palette, and
    each pixel of the tile can select one of 8 colors in the
    tile's color block.

    Bit 7 in the attribute byte defines whether the tile should
    be flipped around the Y axis.
*/
#define BOMBJACK_GATHER16(rom,off) \
    ((uint16_t)rom[0+off]<<8)|((uint16_t)rom[8+off])

static void _bombjack_decode_background(bombjack_t* sys) {
    CHIPS_ASSERT(sys && sys->pixel_buffer);
    uint32_t* ptr = sys->pixel_buffer;
    int img_base_addr = (sys->mainboard.bg_image & 7) * 0x0200;
    bool img_valid = (sys->mainboard.bg_image & 0x10) != 0;
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            int addr = img_base_addr + (y * 16 + x);
            /* 256 tiles */
            uint8_t tile_code = img_valid ? sys->rom_maps[0][addr] : 0;
            uint8_t attr = sys->rom_maps[0][addr + 0x0100];
            uint8_t color_block = (attr & 0x0F)<<3;
            bool flip_y = (attr & 0x80) != 0;
            if (flip_y) {
                ptr += 15*256;
            }
            /* every tile is 32 bytes */
            int off = tile_code * 32;
            for (int yy = 0; yy < 16; yy++) {
                uint16_t bm0 = BOMBJACK_GATHER16(sys->rom_tiles[0], off);
                uint16_t bm1 = BOMBJACK_GATHER16(sys->rom_tiles[1], off);
                uint16_t bm2 = BOMBJACK_GATHER16(sys->rom_tiles[2], off);
                off++;
                if (yy == 7) {
                    off += 8;
                }
                for (int xx = 15; xx >= 0; xx--) {
                    uint8_t pen = ((bm2>>xx)&1) | (((bm1>>xx)&1)<<1) | (((bm0>>xx)&1)<<2);
                    *ptr++ = sys->mainboard.palette[color_block | pen];
                }
                ptr += flip_y ? -272 : 240;
            }
            if (flip_y) {
                ptr += 256 + 16;
            }
            else {
                ptr -= (16 * 256) - 16;
            }
        }
        ptr += (15 * 256);
    }
    CHIPS_ASSERT(ptr == sys->pixel_buffer+256*256);
}

/* render foreground tiles

    Similar to the background tiles, but each tile is 8x8 pixels,
    for 32x32 tiles on the screen.

    Tile char- and color-bytes are not stored in ROM, but in RAM
    at address 0x9000 (1 KB char codes) and 0x9400 (1 KB color codes).

    There are actually 512 char-codes, bit 4 of the color byte
    is used as the missing bit 8 of the char-code.

    The color decoding is the same as the background tiles, the lower
    3 bits are provided by the 3 tile bitmaps, and the remaining
    4 upper bits by the color byte.

    Only 7 foreground colors are possible, since 0 defines a transparent
    pixel.
*/
static void _bombjack_decode_foreground(bombjack_t* sys) {
    CHIPS_ASSERT(sys && sys->pixel_buffer);
    uint32_t* ptr = sys->pixel_buffer;
    /* 32x32 tiles, each 8x8 */
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 32; x++) {
            int addr = y * 32 + x;
            /* char codes are at 0x9000, color codes at 0x9400, RAM starts at 0x8000 */
            uint8_t chr = sys->main_ram[(0x9000-0x8000) + addr];
            uint8_t clr = sys->main_ram[(0x9400-0x8000) + addr];
            /* 512 foreground tiles, take 9th bit from color code */
            int tile_code = chr | ((clr & 0x10)<<4);
            /* 16 color blocks a 8 colors */
            int color_block = (clr & 0x0F)<<3;
            /* 8 bytes per char bitmap */
            int off = tile_code * 8;
            for (int yy = 0; yy < 8; yy++) {
                /* 3 bit planes per char (8 colors per pixel within
                   the palette color block of the char
                */
                uint8_t bm0 = sys->rom_chars[0][off];
                uint8_t bm1 = sys->rom_chars[1][off];
                uint8_t bm2 = sys->rom_chars[2][off];
                off++;
                for (int xx = 7; xx >= 0; xx--) {
                    uint8_t pen = ((bm2>>xx)&1) | (((bm1>>xx)&1)<<1) | (((bm0>>xx)&1)<<2);
                    if (pen) {
                        *ptr = sys->mainboard.palette[color_block | pen];
                    }
                    ptr++;
                }
                ptr += 248;
            }
            ptr -= (8 * 256) - 8;
        }
        ptr += (7 * 256);
    }
    CHIPS_ASSERT(ptr == sys->pixel_buffer+256*256);
}

/*  render sprites

    Each sprite is described by 4 bytes in the 'sprite RAM'
    (0x9820..0x987F => 96 bytes => 24 sprites):

    ABBBBBBB CDEFGGGG XXXXXXXX YYYYYYYY

    A:  sprite size (16x16 or 32x32)
    B:  sprite index
    C:  X flip
    D:  Y flip
    E:  ?
    F:  ?
    G:  color
    X:  x pos
    Y:  y pos
*/
#define BOMBJACK_GATHER32(rom,off) \
    ((uint32_t)rom[0+off]<<24)|\
    ((uint32_t)rom[8+off]<<16)|\
    ((uint32_t)rom[32+off]<<8)|\
    ((uint32_t)rom[40+off])

static void _bombjack_decode_sprites(bombjack_t* sys) {
    CHIPS_ASSERT(sys && sys->pixel_buffer);
    uint32_t* dst = sys->pixel_buffer;
    /* 24 hardware sprites, sprite 0 has highest priority */
    for (int sprite_nr = 23; sprite_nr >= 0; sprite_nr--) {
        /* sprite RAM starts at 0x9820, RAM starts at 0x8000 */
        int addr = (0x9820 - 0x8000) + sprite_nr*4;
        uint8_t b0 = sys->main_ram[addr + 0];
        uint8_t b1 = sys->main_ram[addr + 1];
        uint8_t b2 = sys->main_ram[addr + 2];
        uint8_t b3 = sys->main_ram[addr + 3];
        uint8_t color_block = (b1 & 0x0F)<<3;

        /* screen is 90 degree rotated, so x and y are switched */
        int px = b3;
        int sprite_code = b0 & 0x7F;
        if (b0 & 0x80) {
            /* 32x32 'large' sprites (no flip-x/y needed) */
            int py = 225 - b2;
            uint32_t* ptr = dst + py*256 + px;
            /* offset into sprite ROM to gather sprite bitmap pixels */
            int off = sprite_code * 128;
            for (int y = 0; y < 32; y++) {
                uint32_t bm0 = BOMBJACK_GATHER32(sys->rom_sprites[0], off);
                uint32_t bm1 = BOMBJACK_GATHER32(sys->rom_sprites[1], off);
                uint32_t bm2 = BOMBJACK_GATHER32(sys->rom_sprites[2], off);
                off++;
                if ((y & 7) == 7) {
                    off += 8;
                }
                if ((y & 15) == 15) {
                    off += 32;
                }
                for (int x = 31; x >= 0; x--) {
                    uint8_t pen = ((bm2>>x)&1) | (((bm1>>x)&1)<<1) | (((bm0>>x)&1)<<2);
                    if (0 != pen) {
                        *ptr = sys->mainboard.palette[color_block | pen];
                    }
                    ptr++;
                }
                ptr += 224;
            }
        }
        else {
            /* 16*16 sprites are decoded like 16x16 background tiles */
            int py = 241 - b2;
            uint32_t* ptr = dst + py*256 + px;
            bool flip_x = (b1 & 0x80) != 0;
            bool flip_y = (b1 & 0x40) != 0;
            if (flip_x) {
                ptr += 16*256;
            }
            /* offset into sprite ROM to gather sprite bitmap pixels */
            int off = sprite_code * 32;
            for (int y = 0; y < 16; y++) {
                uint16_t bm0 = BOMBJACK_GATHER16(sys->rom_sprites[0], off);
                uint16_t bm1 = BOMBJACK_GATHER16(sys->rom_sprites[1], off);
                uint16_t bm2 = BOMBJACK_GATHER16(sys->rom_sprites[2], off);
                off++;
                if (y == 7) {
                    off += 8;
                }
                if (flip_y) {
                    for (int x=0; x<=15; x++) {
                        uint8_t pen = ((bm2>>x)&1) | (((bm1>>x)&1)<<1) | (((bm0>>x)&1)<<2);
                        if (0 != pen) {
                            *ptr = sys->mainboard.palette[color_block | pen];
                        }
                        ptr++;
                    }
                }
                else {
                    for (int x=15; x>=0; x--) {
                        uint8_t pen = ((bm2>>x)&1) | (((bm1>>x)&1)<<1) | (((bm0>>x)&1)<<2);
                        if (0 != pen) {
                            *ptr = sys->mainboard.palette[color_block | pen];
                        }
                        ptr++;
                    }
                }
                ptr += flip_x ? -272 : 240;
            }
        }
    }
}

void bombjack_decode_video(bombjack_t* sys) {
    if (sys->pixel_buffer) {
        if (sys->dbg.draw_background_layer) {
            _bombjack_decode_background(sys);
        }
        else {
            if (sys->dbg.clear_background_layer) {
                for (int i = 0; i < _BOMBJACK_DISPLAY_WIDTH*_BOMBJACK_DISPLAY_HEIGHT; i++) {
                    sys->pixel_buffer[i] = 0xFF000000;
                }
            }
        }
        if (sys->dbg.draw_foreground_layer) {
            _bombjack_decode_foreground(sys);
        }
        if (sys->dbg.draw_sprite_layer) {
            _bombjack_decode_sprites(sys);
        }
    }
}

#endif /* CHIPS_IMPL */
