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
    namco_audio_callback_t audio_cb;     /* called when audio_num_samples are ready */
    int audio_num_samples;                  /* default is BOMBJACK_DEFAULT_AUDIO_SAMPLES */
    int audio_sample_rate;                  /* playback sample rate, default is 44100 */
    float audio_volume;                     /* audio volume, 0.0..1.0, default is 1.0 */

    /* ROM images */

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
void namco_init(namco_t* sys, namco_desc_t* desc);
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

#endif /* CHIPS_IMPL */
