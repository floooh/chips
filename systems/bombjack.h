#pragma once
/*#
    # bombjack.h

    Bomb Jack arcade machine emulator in a C header.

    FIXME: WIP!

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
#include <stdio.h>

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
#define BOMBJACK_DSW2_P2_1COIN_5PLAY    (3<<2)

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
    float audio_volume;                     /* audio volume, 0.0..1.0, default is 0.25 */

    /* ROM images */
    const void* rom_main_0000_1FFF;     /* main-board ROM 0x0000..0x1FFF */
    const void* rom_main_2000_3FFF;     /* main-board ROM 0x2000..0x3FFF */
    const void* rom_main_4000_5FFF;     /* main-board ROM 0x4000..0x5FFF */
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

typedef struct {
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
    uint8_t rom_main[0x8000];
    uint8_t rom_sound[0x2000];
    uint8_t rom_chars[0x3000];
    uint8_t rom_tiles[0x6000];
    uint8_t rom_sprites[0x6000];
    uint8_t rom_maps[0x1000];
    /* small intermediate buffer for generated audio samples */
    int sample_pos;
    float sample_buffer[BOMBJACK_MAX_AUDIO_SAMPLES];
    /* 32-bit RGBA color palette cache */
    uint32_t palette_cache[128];
} bombjack_t;

/* initialize a new bombjack instance */
void bombjack_init(bombjack_t* sys, bombjack_desc_t* desc);
/* discard a bombjack instance */
void bombjack_discard(bombjack_t* sys);
/* get the standard framebuffer width and height in pixels */
int bombjack_std_display_width(void);
int bombjack_std_display_height(void);
/* get the maximum framebuffer size in number of bytes */
int bombjack_max_display_size(void);
/* get the current framebuffer width and height in pixels */
int bombjack_display_width(bombjack_t* sys);
int bombjack_display_height(bombjack_t* sys);
/* reset a bombjack instance */
void bombjack_reset(bombjack_t* sys);
/* run bombjack instance for given amount of microseconds */
void bombjack_exec(bombjack_t* sys, uint32_t micro_seconds);
/* set joystick mask (combination of BOMJACK_JOYSTICK_* flags) */
void bombjack_joystick(bombjack_t* sys, int player, uint8_t mask);


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

#endif /* CHIPS_IMPL */