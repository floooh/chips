#pragma once
/*#
    # z1013.h

    Header-only Robotron Z1013 emulator.

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

    You need to include the following headers before including z1013.h:

    - chips/z80.h
    - chips/z80pio.h
    - chips/mem.h
    - chips/kbd.h

    ## TODO: Describe Z1013

    ## TODO: Describe embedding


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

/* Z1013 models */
typedef enum {
    Z1013_MODEL_64,      /* Z1013.64 (default, latest model with 2 MHz and 64 KB RAM, new ROM) */
    Z1013_MODEL_16,      /* Z1013.16 (2 MHz model with 16 KB RAM, new ROM) */
    Z1013_MODEL_01,      /* Z1013.01 (original model, 1 MHz, 16 KB RAM) */
} z1013_model_t;

/* configuration parameters for z1013_setup() */
typedef struct {
    z1013_model_t model;        /* default is Z1013_TYPE_64 */
    const void* rom_mon202;     /* pointer to static mon202 ROM data, required by Z1013_TYPE_01 */
    const void* rom_mon_a2;     /* pointer to static mon_a2 ROM data, required by Z1013_TYPE_16 and Z1013_TYPE_64 */
    const void* rom_font;       /* pointer to static font ROM data, required by all models */
    void* pixel_buffer;         /* pointer to a memory chunk used as RGBA8 framebuffer */
    int rom_mon202_size;        /* size of mon202 ROM data (must be 2048 bytes) */
    int rom_mon_a2_size;        /* size of mon_a2 ROM data (must be 2048 bytes) */
    int rom_font_size;          /* size of font ROM (must be 2048 bytes) */
    int pixel_buffer_size;      /* size of framebuffer memory chunk in bytes, must be at least 256 KBytes (256*256*4) */
} z1013_desc_t;

/* Z1013 instance state */
typedef struct {
    z1013_model_t model;
    z80_t cpu;
    z80pio_t pio;
    mem_t mem;
    kbd_t kbd;
    uint8_t kbd_request_column;
    bool kbd_request_hilo;
    uint32_t* pixel_buffer;
    const void* rom_mon202;
    const void* rom_mon_a2;
    const void* rom_font;
    uint8_t ram[1<<16];
} z1013_t;

/* initialize a new Z1013 instance */
extern void z1013_init(z1013_t* sys, const z1013_desc_t* desc);
/* reset Z1013 instance */
extern void z1013_reset(z1013_t* sys);
/* run the Z1013 instance for a given time in seconds */
extern void z1013_exec(z1013_t* sys, double seconds);
/* execute at least given number of ticks, return executed number of ticks */
extern uint32_t z1013_step(z1013_t* sys, uint32_t ticks);
/* send a key-down event */
extern void z1013_key_down(z1013_t* sys, uint8_t key_code);
/* send a key-up event */
extern void z1013_key_up(z1013_t* sys, uint8_t key_code);
/* get the fixed width of the Z1013 display in pixels */
extern int z1013_display_width(void);
/* get the fixed height of the Z1013 display in pixels */
extern int z1013_display_height(void);
/* load a .z80 file into the emulator */
extern bool z1013_load_z80(z1013_t* sys, const uint8_t* ptr, int num_bytes);

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

#define _Z1013_DISPLAY_WIDTH (256)
#define _Z1013_DISPLAY_HEIGHT (256)
#define _Z1013_DISPLAY_SIZE (_Z1013_DISPLAY_WITH*_Z1013_DISPLAY_HEIGHT*sizeof(uint32_t))
#define _Z1013_ROM_MON202_SIZE (2048)
#define _Z1013_ROM_MON_A2_SIZE (2048)
#define _Z1013_ROM_FONT_SIZE   (2048)

extern uint64_t _z1013_tick(int num, uint64_t pins, void* user_data);
extern uint8_t _z1013_pio_in(int port_id, void* user_data);
extern void _z1013_pio_out(int port_id, uint8_t data, void* user_data);


int z1013_display_width(void) {
    return _Z1013_DISPLAY_WIDTH;
}

int z1013_display_height(void) {
    return _Z1013_DISPLAY_HEIGHT;
}

void z1013_init(z1013_t* sys, const z1013_desc_t* desc) {
    CHIPS_ASSERT(sys && desc);
    CHIPS_ASSERT(desc->pixel_buffer && (desc->pixel_buffer_size >= _Z1013_DISPLAY_SIZE));
    CHIPS_ASSERT(desc->rom_font && (desc->rom_font_size == _Z1013_ROM_FONT_SIZE));
    if (desc->model == Z1013_MODEL_01) {
        CHIPS_ASSERT(desc->rom_mon202 && (desc->rom_mon202_size == _Z1013_ROM_MON202_SIZE));
    }
    else {
        CHIPS_ASSERT(desc->rom_mon_a2 && (desc->rom_mon_a2_size == _Z1013_ROM_MON_A2_SIZE));
    }

    memset(sys, 0, sizeof(z1013_t));
    sys->model = desc->model;
    sys->rom_font = desc->rom_font;
    sys->rom_mon202 = desc->rom_mon202;
    sys->rom_mon_a2 = desc->rom_mon_a2;
    sys->pixel_buffer = desc->pixel_buffer;

    /* initialize the hardware */
    z80_desc_t cpu_desc = {0};
    cpu_desc.tick_cb = _z1013_tick;
    cpu_desc.user_data = sys;
    z80_init(&sys->cpu, &cpu_desc);
    z80pio_desc_t pio_desc = {0};
    pio_desc.in_cb = _z1013_pio_in;
    pio_desc.out_cb = _z1013_pio_out;
    pio_desc.user_data = sys;
    z80pio_init(&sys->pio, &pio_desc);

    /* execution starts at 0xF000 */
    z80_set_pc(&sys->cpu, 0xF000);

    /* setup the memory map:
        - the Z1013.64 has 64 KByte RAM, all other models 16 KByte RAM
        - there's a 1 KByte character frame buffer at 0xEC00
        - and finally 2 KByte ROM starting at 0xF000
        - the memory map is fixed
    */
    mem_init(&sys->mem);
    if (Z1013_MODEL_64 == sys->model) {
        mem_map_ram(&sys->mem, 1, 0x0000, 0x10000, sys->ram);
    }
    else {
        mem_map_ram(&sys->mem, 1, 0x0000, 0x4000, sys->ram);
        mem_map_ram(&sys->mem, 1, 0xEC00, 0x0400, sys->ram);
    }
    if (Z1013_MODEL_01 == sys->model) {
        mem_map_rom(&sys->mem, 0, 0xF000, 0x0800, sys->rom_mon202);
    }
    else {
        mem_map_rom(&sys->mem, 0, 0xF000, 0x0800, sys->rom_mon_a2);
    }

    /* Setup the keyboard matrix, the original Z1013.01 has a 8x4 matrix with
       4 shift keys, later models also support a more traditional 8x8 matrix.
    */
    /* keep key presses sticky for 2 frames */
    kbd_init(&sys->kbd, 2);
    if (Z1013_MODEL_01 == sys->model) {
        /* 8x4 keyboard matrix */

        /* 4 shift key modifiers */
        kbd_register_modifier(&sys->kbd, 0, 0, 3);
        kbd_register_modifier(&sys->kbd, 1, 1, 3);
        kbd_register_modifier(&sys->kbd, 2, 2, 3);
        kbd_register_modifier(&sys->kbd, 3, 3, 3);       
        const char* keymap =
            /* no shift */
            "@ABCDEFG"  "HIJKLMNO"  "PQRSTUVW"  "        "
            /* shift 1 */
            "XYZ[\\]^-" "01234567"  "89:;<=>?"  "        "
            /* shift 2 */
            "   {|}~ "  " !\"#$%&'" "()*+,-./"  "        "
            /* shift 3 */
            " abcdefg"  "hijklmno"  "pqrstuvw"  "        "
            /* shift 4 */
            "xyz     "  "        "  "        "  "        ";
        for (int layer = 0; layer < 5; layer++) {
            for (int line = 0; line < 4; line++) {
                for (int column = 0; column < 8; column++) {
                    int c = keymap[layer*32 + line*8 + column];
                    if (c != 0x20) {
                        kbd_register_key(&sys->kbd, c, column, line, layer ? (1<<(layer-1)) : 0);
                    }
                }
            }
        }
        /* special keys */
        kbd_register_key(&sys->kbd, ' ', 5, 3, 0);   /* Space */
        kbd_register_key(&sys->kbd, 0x08, 4, 3, 0);  /* Cursor Left */
        kbd_register_key(&sys->kbd, 0x09, 6, 3, 0);  /* Cursor Right */
        kbd_register_key(&sys->kbd, 0x0D, 7, 3, 0);  /* Enter */
        kbd_register_key(&sys->kbd, 0x03, 3, 1, 4);  /* Break/Escape */
    }
    else {
        /* 8x8 keyboard matrix (http://www.z1013.de/images/21.gif) */

        /* shift key modifier is column 7 line 6 */
        const int shift = 0, shift_mask = (1<<shift);
        kbd_register_modifier(&sys->kbd, shift, 7, 6);
        /* ctrl key modifier is column 6 line 5 */
        const int ctrl = 1, ctrl_mask = (1<<ctrl);
        kbd_register_modifier(&sys->kbd, ctrl, 6, 5);
        const char* keymap = 
            /* no shift */
            "13579-  QETUO@  ADGJL*  YCBM.^  24680[  WRZIP]  SFHK+\\  XVN,/_  "
            /* shift */
            "!#%')=  qetuo`  adgjl:  ycbm>~  \"$&( {  wrzip}  sfhk;|  xvn<?   ";
        for (int layer = 0; layer < 2; layer++) {
            for (int line = 0; line < 8; line++) {
                for (int column = 0; column < 8; column++) {
                    int c = keymap[layer*64 + line*8 + column];
                    if (c != 0x20) {
                        kbd_register_key(&sys->kbd, key, column, line, layer ? shift_mask : 0);
                    }
                }
            }
        }
        /* special keys */
        kbd_register_key(&sys->kbd, ' ',  6, 4, 0);  /* space */
        kbd_register_key(&sys->kbd, 0x08, 6, 2, 0);  /* cursor left */
        kbd_register_key(&sys->kbd, 0x09, 6, 3, 0);  /* cursor right */
        kbd_register_key(&sys->kbd, 0x0A, 6, 7, 0);  /* cursor down */
        kbd_register_key(&sys->kbd, 0x0B, 6, 6, 0);  /* cursor up */
        kbd_register_key(&sys->kbd, 0x0D, 6, 1, 0);  /* enter */
        kbd_register_key(&sys->kbd, 0x03, 1, 3, ctrl_mask); /* map Esc to Ctrl+C (STOP/BREAK) */
    }
}

#endif /* CHIPS_IMPL */
