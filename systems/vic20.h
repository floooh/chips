#pragma once
/*#
    # vic20.h

    A Commodore VIC-20 emulator in a C header.

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

    You need to include the following headers before including c64.h:

    - chips/m6502.h
    - chips/m6522.h
    - chips/m6561.h
    - chips/kbd.h
    - chips/mem.h
    - chips/clk.h

    ## The Commodore VIC-20

    TODO!

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

#define VIC20_FREQUENCY (1108404)
#define VIC20_MAX_AUDIO_SAMPLES (1024)        /* max number of audio samples in internal sample buffer */
#define VIC20_DEFAULT_AUDIO_SAMPLES (128)     /* default number of samples in internal sample buffer */ 

/* VIC-20 joystick types */
typedef enum {
    C64_JOYSTICKTYPE_NONE,
    C64_JOYSTICKTYPE_DIGITAL_1,
    C64_JOYSTICKTYPE_DIGITAL_2,
    C64_JOYSTICKTYPE_DIGITAL_12,    /* input routed to both joysticks */
} vic20_joystick_type_t;

/* joystick mask bits */
#define VIC20_JOYSTICK_UP    (1<<0)
#define VIC20_JOYSTICK_DOWN  (1<<1)
#define VIC20_JOYSTICK_LEFT  (1<<2)
#define VIC20_JOYSTICK_RIGHT (1<<3)
#define VIC20_JOYSTICK_BTN   (1<<4)

/* casette port bits, same as C1530_CASPORT_* */
#define VIC20_CASPORT_MOTOR   (1<<0)  /* 1: motor off, 0: motor on */
#define VIC20_CASPORT_READ    (1<<1)  /* 1: read signal from datasette, connected to CIA-1 FLAG */
#define VIC20_CASPORT_WRITE   (1<<2)  /* not implemented */
#define VIC20_CASPORT_SENSE   (1<<3)  /* 1: play button up, 0: play button down */

/* IEC port bits, same as C1541_IECPORT_* */
#define VIC20_IECPORT_RESET   (1<<0)  /* 1: RESET, 0: no reset */
#define VIC20_IECPORT_SRQIN   (1<<1)  /* connected to CIA-1 FLAG */
#define VIC20_IECPORT_DATA    (1<<2)
#define VIC20_IECPORT_CLK     (1<<3)
#define VIC20_IECPORT_ATN     (1<<4)

/* audio sample data callback */
typedef void (*vic20_audio_callback_t)(const float* samples, int num_samples, void* user_data);

/* config parameters for c64_init() */
typedef struct {
    vic20_joystick_type_t joystick_type;  /* default is VIC20_JOYSTICK_NONE */

    /* video output config (if you don't want video decoding, set these to 0) */
    void* pixel_buffer;         /* pointer to a linear RGBA8 pixel buffer,
                                   query required size via vic20_max_display_size() */
    int pixel_buffer_size;      /* size of the pixel buffer in bytes */

    /* optional user-data for callback functions */
    void* user_data;

    /* audio output config (if you don't want audio, set audio_cb to zero) */
    vic20_audio_callback_t audio_cb;  /* called when audio_num_samples are ready */
    int audio_num_samples;          /* default is VIC20_AUDIO_NUM_SAMPLES */
    int audio_sample_rate;          /* playback sample rate in Hz, default is 44100 */
    float audio_sid_volume;         /* audio volume of the VIC chip (0.0 .. 1.0), default is 1.0 */

    /* ROM images */
    const void* rom_char;           /* 4 KByte character ROM dump */
    const void* rom_basic;          /* 8 KByte BASIC dump */
    const void* rom_kernal;         /* 8 KByte KERNAL dump */
    int rom_char_size;
    int rom_basic_size;
    int rom_kernal_size;
} vic20_desc_t;

/* VIC-20 emulator state */
typedef struct {
    uint64_t pins;
    m6502_t cpu;
    m6522_t via_1;
    m6522_t via_2;
    //m6561_t vic;
    
    bool valid;
    vic20_joystick_type_t joystick_type;
    uint8_t joystick_active;
    uint8_t cas_port;           /* cassette port, shared with c1530_t if datasette is connected */
    uint8_t iec_port;           /* IEC serial port, shared with c1541_t if connected */
    uint8_t kbd_joy1_mask;      /* current joystick-1 state from keyboard-joystick emulation */
    uint8_t kbd_joy2_mask;      /* current joystick-2 state from keyboard-joystick emulation */
    uint8_t joy_joy1_mask;      /* current joystick-1 state from c64_joystick() */
    uint8_t joy_joy2_mask;      /* current joystick-2 state from c64_joystick() */

    kbd_t kbd;                  /* keyboard matrix state */
    mem_t mem_cpu;              /* CPU-visible memory mapping */

    void* user_data;
    uint32_t* pixel_buffer;
    vic20_audio_callback_t audio_cb;
    int num_samples;
    int sample_pos;
    float sample_buffer[VIC20_MAX_AUDIO_SAMPLES];

    uint8_t ram[1<<15];             /* 32KB RAM + RAM Expansion */
    uint8_t rom_char[0x1000];       /* 4 KB character ROM image */
    uint8_t rom_basic[0x2000];      /* 8 KB BASIC ROM image */
    uint8_t rom_kernal[0x2000];     /* 8 KB KERNAL V3 ROM image */
} vic20_t;

/* initialize a new VIC-20 instance */
void vic20_init(vic20_t* sys, const vic20_desc_t* desc);
/* discard VIC-20 instance */
void vic20_discard(vic20_t* sys);
/* get the standard framebuffer width and height in pixels */
int vic20_std_display_width(void);
int vic20_std_display_height(void);
/* get the maximum framebuffer size in number of bytes */
int vic20_max_display_size(void);
/* get the current framebuffer width and height in pixels */
int vic20_display_width(vic20_t* sys);
int vic20_display_height(vic20_t* sys);
/* reset a VIC-20 instance */
void vic20_reset(vic20_t* sys);
/* tick VIC-20 instance for a given number of microseconds, also updates keyboard state */
void vic20_exec(vic20_t* sys, uint32_t micro_seconds);
/* ...or optionally: tick the VIC-20 instance once, does not update keyboard state! */
void vic20_tick(vic20_t* sys);
/* send a key-down event to the VIC-20 */
void vic20_key_down(vic20_t* sys, int key_code);
/* send a key-up event to the VIC-20 */
void vic20_key_up(vic20_t* sys, int key_code);
/* enable/disable joystick emulation */
void vic20_set_joystick_type(vic20_t* sys, vic20_joystick_type_t type);
/* get current joystick emulation type */
vic20_joystick_type_t vic20_joystick_type(vic20_t* sys);
/* set joystick mask (combination of VIC20_JOYSTICK_*) */
void vic20_joystick(vic20_t* sys, uint8_t joy1_mask, uint8_t joy2_mask);
/* quickload a .prg/.bin file */
bool vic20_quickload(vic20_t* sys, const uint8_t* ptr, int num_bytes);

#ifdef __cplusplus
} /* extern "C" */
#endif

/*-- IMPLEMENTATION ----------------------------------------------------------*/
#ifdef CHIPS_IMPL
#include <string.h> /* memcpy, memset */
#ifndef CHIPS_ASSERT
    #include <assert.h>
    #define CHIPS_ASSERT(c) assert(c)
#endif

#endif /* CHIPS_IMPL */
