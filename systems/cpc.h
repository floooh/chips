#pragma once
/*#
    # cpc.h

    An Amstrad CPC emulator in a C header

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
    - chips/ay38910.h
    - chips/i8255.h
    - chips/mc6845.h
    - chips/crt.h
    - chips/mem.h
    - chips/kbd.h
    - chips/clk.h

    ## The Amstrad CPC 464

    FIXME!

    ## The Amstrad CPC 6128

    FIXME!

    ## The KC Compact

    FIXME!

    ## TODO

    - improve CRTC emulation, some graphics demos don't work yet
    - DSK file support

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
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CPC_DISPLAY_WIDTH (768)
#define CPC_DISPLAY_HEIGHT (272)
#define CPC_DISPLAY_WIDTH_DEBUG (1024)      /* display-width with debug-visualization enabled */
#define CPC_DISPLAY_HEIGHT_DEBUG (312)      /* display-width with debug-visualization enabled */
#define CPC_MAX_AUDIO_SAMPLES (1024)        /* max number of audio samples in internal sample buffer */
#define CPC_DEFAULT_AUDIO_SAMPLES (128)     /* default number of samples in internal sample buffer */
#define CPC_MAX_TAPE_SIZE (1<<16)           /* max size of tape file in bytes */

/* CPC model types */
typedef enum {
    CPC_TYPE_6128,          /* default */
    CPC_TYPE_464,
    CPC_TYPE_KCCOMPACT
} cpc_type_t;

/* joystick types */
typedef enum {
    CPC_JOYSTICK_NONE,
    CPC_JOYSTICK_DIGITAL,
    CPC_JOYSTICK_ANALOG,
} cpc_joystick_t;

/* audio sample data callback */
typedef int (*cpc_audio_callback_t)(const float* samples, int num_samples);
/* max number of audio samples in internal sample buffer */
#define CPC_MAX_AUDIO_SAMPLES (1024)
/* default number of audio samples to generate until audio callback is invoked */
#define CPC_DEFAULT_AUDIO_SAMPLES (128)

/* configuration parameters for cpc_init() */
typedef struct {
    cpc_type_t type;                /* default is the CPC 6128 */
    cpc_joystick_t joystick_type;

    /* video output config */
    void* pixel_buffer;         /* pointer to a linear RGBA8 pixel buffer, at least 1024*312*4 bytes */
    int pixel_buffer_size;      /* size of the pixel buffer in bytes */

    /* audio output config (if you don't want audio, set audio_cb to zero) */
    cpc_audio_callback_t audio_cb;  /* called when audio_num_samples are ready */
    int audio_num_samples;          /* default is ZX_AUDIO_NUM_SAMPLES */
    int audio_sample_rate;          /* playback sample rate, default is 44100 */
    float audio_volume;             /* audio volume: 0.0..1.0, default is 0.25 */

    /* ROM images */
    const void* rom_464_os;
    const void* rom_464_basic;
    const void* rom_6128_os;
    const void* rom_6128_basic;
    const void* rom_6128_amsdos;
    const void* rom_kcc_os;
    const void* rom_kcc_basic;
    int rom_464_os_size;
    int rom_464_basic_size;
    int rom_6128_os_size;
    int rom_6128_basic_size;
    int rom_6128_amsdos_size;
    int rom_kcc_os_size;
    int rom_kcc_basic_size;
} cpc_desc_t;

/* CPCP gate array state */
typedef struct {
    uint8_t config;                 /* out to port 0x7Fxx func 0x80 */
    uint8_t next_video_mode;
    uint8_t video_mode;
    uint8_t ram_config;             /* out to port 0x7Fxx func 0xC0 */
    uint8_t pen;                    /* currently selected pen (or border) */
    uint32_t colors[32];            /* CPC and KC Compact have slightly different colors */
    uint32_t palette[16];           /* the current pen colors */
    uint32_t border_color;          /* the current border color */
    int hsync_irq_counter;          /* incremented each scanline, reset at 52 */
    int hsync_after_vsync_counter;   /* for 2-hsync-delay after vsync */
    int hsync_delay_counter;        /* hsync to monitor is delayed 2 ticks */
    int hsync_counter;              /* countdown until hsync to monitor is deactivated */
    bool sync;                      /* gate-array generated video sync (modified HSYNC) */
    bool intr;                      /* GA interrupt pin active */
    uint64_t crtc_pins;             /* store CRTC pins to detect rising/falling bits */
} cpc_gatearray_t;

/* CPC emulator state */
typedef struct {
    z80_t cpu;
    ay38910_t psg;
    mc6845_t vdg;
    i8255_t ppi;

    bool valid;
    bool dbgvis;                    /* debug visualzation enabled? */
    cpc_type_t type;
    cpc_joystick_t joystick_type;
    uint8_t joy_mask;
    uint8_t upper_rom_select;
    uint32_t tick_count;
    uint16_t casread_trap;
    uint16_t casread_ret;
    cpc_gatearray_t ga;

    crt_t crt;
    clk_t clk;
    kbd_t kbd;
    mem_t mem;
    uint32_t* pixel_buffer;
    const void* rom_464_os;
    const void* rom_464_basic;
    const void* rom_6128_os;
    const void* rom_6128_basic;
    const void* rom_6128_amsdos;
    const void* rom_kcc_os;
    const void* rom_kcc_basic;

    cpc_audio_callback_t audio_cb;
    int num_samples;
    int sample_pos;
    float sample_buffer[CPC_MAX_AUDIO_SAMPLES];

    uint8_t ram[8][0x4000];
} cpc_t;

/* initialize a new CPC instance */
extern void cpc_init(cpc_t* cpc, cpc_desc_t* desc);
/* discard a CPC instance */
extern void cpc_discard(cpc_t* cpc);
/* reset a CPC instance */
extern void cpc_reset(cpc_t* cpc);
/* run CPC instance for given amount of seconds */
extern void cpc_exec(cpc_t* cpc, double seconds);
/* send a key down event */
extern void cpc_key_down(cpc_t* cpc, int key_code);
/* send a key up event */
extern void cpc_key_up(cpc_t* cpc, int key_code);
/* load a snapshot file (.sna or .bin) into the emulator */
extern bool cpc_quickload(cpc_t* cpc, const uint8_t* ptr, int num_bytes);
/* insert a tape file (.tap) */
extern bool cpc_insert_tape(cpc_t* cpc, const uint8_t* ptr, int num_bytes);
/* remove currently inserted tape */
extern void cpc_remove_tape(cpc_t* cpc);
/* enable/disable the display debug visualization (size is CPC_DISPLAY_WIDTH_DEBUG x CPC_DISPLAY_HEIGHT_DEBUG !) */
extern void cpc_enable_dbgvis(cpc_t* cpc, bool enabled);
/* get current display debug visualization enabled/disabled state */
extern bool cpc_dbgvis_enabled(cpc_t* cpc);

#ifdef __cplusplus
} /* extern "C" */
#endif

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

#define _CPC_DISPLAY_SIZE_DEBUG (CPC_DISPLAY_WIDTH*CPC_DISPLAY_HEIGHT*3)
#define _CPC_FREQUENCY (4000000)
#define _CPC_ROM_SIZE (0x4000)

static uint64_t _cpc_tick(int num, uint64_t pins, void* user_data);
static uint64_t _cpc_cpu_iorq(cpc_t* sys, uint64_t pins);
static uint64_t _cpc_ppi_out(int port_id, uint64_t pins, uint8_t data, void* user_data);
static uint8_t _cpc_ppi_in(int port_id, void* user_data);
static void _cpc_psg_out(int port_id, uint8_t data, void* user_data);
static uint8_t _cpc_psg_in(int port_id, void* user_data);
static void _cpc_ga_init(cpc_t* sys);
static uint64_t _cpc_ga_tick(cpc_t* sys, uint64_t pins);
static void _cpc_ga_int_ack(cpc_t* sys);
static void _cpc_ga_decode_video(cpc_t* sys, uint64_t crtc_pins);
static void _cpc_ga_decode_pixels(cpc_t* sys, uint32_t* dst, uint64_t crtc_pins);
static void _cpc_init_keymap(cpc_t* sys);
static void _cpc_update_memory_mapping(cpc_t* sys);

#define _CPC_DEFAULT(val,def) (((val) != 0) ? (val) : (def));

void cpc_init(cpc_t* sys, cpc_desc_t* desc) {
    CHIPS_ASSERT(sys && desc);
    CHIPS_ASSERT(desc->pixel_buffer && (desc->pixel_buffer_size >= _CPC_DISPLAY_SIZE_DEBUG));
    if (CPC_TYPE_464 == desc->type) {
        CHIPS_ASSERT(desc->rom_464_os && (desc->rom_464_os_size == _CPC_ROM_SIZE));
        CHIPS_ASSERT(desc->rom_464_basic && (desc->rom_464_basic_size == _CPC_ROM_SIZE));
    }
    else if (CPC_TYPE_6128 == desc->type) {
        CHIPS_ASSERT(desc->rom_6128_os && (desc->rom_6128_os_size == _CPC_ROM_SIZE));
        CHIPS_ASSERT(desc->rom_6128_basic && (desc->rom_6128_basic_size == _CPC_ROM_SIZE));
        CHIPS_ASSERT(desc->rom_6128_amsdos && (desc->rom_6128_amsdos_size == _CPC_ROM_SIZE));
    }
    else {
        CHIPS_ASSERT(desc->rom_kcc_os && (desc->rom_kcc_os_size == _CPC_ROM_SIZE));
        CHIPS_ASSERT(desc->rom_kcc_basic && (desc->rom_kcc_basic_size == _CPC_ROM_SIZE));
    }
    
    memset(sys, 0, sizeof(cpc_t));
    sys->valid = true;
    sys->type = desc->type;
    sys->joystick_type = desc->joystick_type;
    sys->rom_464_os = desc->rom_464_os;
    sys->rom_464_basic = desc->rom_464_basic;
    sys->rom_6128_os = desc->rom_6128_os;
    sys->rom_6128_basic = desc->rom_6128_basic;
    sys->rom_6128_amsdos = desc->rom_6128_amsdos;
    sys->rom_kcc_os = desc->rom_kcc_os;
    sys->rom_kcc_basic = desc->rom_kcc_basic;
    sys->pixel_buffer = (uint32_t*) desc->pixel_buffer;
    sys->audio_cb = desc->audio_cb;
    sys->num_samples = _CPC_DEFAULT(desc->audio_num_samples, CPC_DEFAULT_AUDIO_SAMPLES);
    CHIPS_ASSERT(sys->num_samples <= CPC_MAX_AUDIO_SAMPLES);

    /* initialize the hardware */
    clk_init(&sys->clk, _CPC_FREQUENCY);

    z80_desc_t cpu_desc = {0};
    cpu_desc.tick_cb = _cpc_tick;
    cpu_desc.user_data = sys;
    z80_init(&sys->cpu, &cpu_desc);

    i8255_desc_t ppi_desc = {0};
    ppi_desc.in_cb = _cpc_ppi_in;
    ppi_desc.out_cb = _cpc_ppi_out;
    ppi_desc.user_data = sys;
    i8255_init(&sys->ppi, &ppi_desc);

    ay38910_desc_t psg_desc = {0};
    psg_desc.type = AY38910_TYPE_8912;
    psg_desc.in_cb = _cpc_psg_in;
    psg_desc.out_cb = _cpc_psg_out;
    psg_desc.tick_hz = _CPC_FREQUENCY / 4;
    psg_desc.sound_hz = _CPC_DEFAULT(desc->audio_sample_rate, 44100);
    psg_desc.magnitude = _CPC_DEFAULT(desc->audio_volume, 0.7f);
    psg_desc.user_data = sys;
    ay38910_init(&sys->psg, &psg_desc);

    mc6845_init(&sys->vdg, MC6845_TYPE_UM6845R);
    crt_init(&sys->crt, CRT_PAL, 6, 32, CPC_DISPLAY_WIDTH/16, CPC_DISPLAY_HEIGHT);

    _cpc_ga_init(sys);
    _cpc_init_keymap(sys);
    mem_init(&sys->mem);
    _cpc_update_memory_mapping(sys);

    /* cassette tape loading
        (http://www.cpcwiki.eu/index.php/Format:TAP_tape_image_file_format)
    */
    if (CPC_TYPE_464 == sys->type) {
        sys->casread_trap = 0x2836;
        sys->casread_ret  = 0x2872;
    }
    else {
        sys->casread_trap = 0x29A6;
        sys->casread_ret  = 0x29E2;
    }
    z80_set_trap(&sys->cpu, 1, sys->casread_trap);

    /* execution starts as address 0 */
    z80_set_pc(&sys->cpu, 0x0000);
}

void cpc_discard(cpc_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    sys->valid = false;
}

void cpc_reset(cpc_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    mc6845_reset(&sys->vdg);
    crt_reset(&sys->crt);
    ay38910_reset(&sys->psg);
    i8255_reset(&sys->ppi);
    z80_reset(&sys->cpu);
    z80_set_pc(&sys->cpu, 0x0000);
    sys->joy_mask = 0;
    sys->tick_count = 0;
    sys->upper_rom_select = 0;
    _cpc_ga_init(sys);
    mem_unmap_all(&sys->mem);
    _cpc_update_memory_mapping(sys);
}

void cpc_exec(cpc_t* sys, double seconds) {
    CHIPS_ASSERT(sys && sys->valid);
    uint32_t ticks_to_run = clk_ticks_to_run(&sys->clk, seconds);
    uint32_t ticks_executed = z80_exec(&sys->cpu, ticks_to_run);
    clk_ticks_executed(&sys->clk, ticks_executed);
    kbd_update(&sys->kbd);
    /* check if casread trap has been hit, and the right ROM is mapped in */
    if (sys->cpu.trap_id == 1) {
        if (sys->type == CPC_TYPE_6128) {
            if (0 == (sys->ga.config & (1<<2))) {
//                _cpc_casread(sys);
            }
        }
        else {
            /* no memory mapping on KC Compact, 464 or 664 */
//            _cpc_casread(sys);
        }
    }
}

void cpc_key_down(cpc_t* sys, int key_code) {
    CHIPS_ASSERT(sys && sys->valid);
    if (sys->joystick_type == CPC_JOYSTICK_DIGITAL) {
        switch (key_code) {
            case SAPP_KEYCODE_SPACE: sys->joy_mask |= 1<<4; break;
            case SAPP_KEYCODE_LEFT:  sys->joy_mask |= 1<<2; break;
            case SAPP_KEYCODE_RIGHT: sys->joy_mask |= 1<<3; break;
            case SAPP_KEYCODE_DOWN:  sys->joy_mask |= 1<<1; break;
            case SAPP_KEYCODE_UP:    sys->joy_mask |= 1<<0; break;
            default: kbd_key_down(&sys->kbd, key_code); break;
        }
    }
    else {
        kbd_key_down(&sys->kbd, key_code);
    }
}

void cpc_key_up(cpc_t* sys, int key_code) {
    CHIPS_ASSERT(sys && sys->valid);
    if (sys->joystick_type == CPC_JOYSTICK_DIGITAL) {
        switch (key_code) {
            case SAPP_KEYCODE_SPACE: sys->joy_mask &= ~(1<<4); break;
            case SAPP_KEYCODE_LEFT:  sys->joy_mask &= ~(1<<2); break;
            case SAPP_KEYCODE_RIGHT: sys->joy_mask &= ~(1<<3); break;
            case SAPP_KEYCODE_DOWN:  sys->joy_mask &= ~(1<<1); break;
            case SAPP_KEYCODE_UP:    sys->joy_mask &= ~(1<<0); break;
            default: kbd_key_up(&sys->kbd, key_code); break;
        }
    }
    else {
        kbd_key_up(&sys->kbd, key_code);
    }
}

/* the CPU tick callback */
uint64_t _cpc_tick(int num_ticks, uint64_t pins, void* user_data) {
    cpc_t* sys = (cpc_t*) user_data;

    /* interrupt acknowledge? */
    if ((pins & (Z80_M1|Z80_IORQ)) == (Z80_M1|Z80_IORQ)) {
        _cpc_ga_int_ack(sys);
    }

    /* memory and IO requests */
    if (pins & Z80_MREQ) {
        /* CPU MEMORY REQUEST */
        const uint16_t addr = Z80_GET_ADDR(pins);
        if (pins & Z80_RD) {
            Z80_SET_DATA(pins, mem_rd(&sys->mem, addr));
        }
        else if (pins & Z80_WR) {
            mem_wr(&sys->mem, addr, Z80_GET_DATA(pins));
        }
    }
    else if ((pins & Z80_IORQ) && (pins & (Z80_RD|Z80_WR))) {
        /* CPU IO REQUEST */
        pins = _cpc_cpu_iorq(sys, pins);
    }
    
    /*
        tick the gate array and audio chip at 1 MHz, and decide how many wait
        states must be injected, the CPC signals the wait line in 3 out of 4
        cycles:
    
         0: wait inactive
         1: wait active
         2: wait active
         3: wait active
    
        the CPU samples the wait line only on specific clock ticks
        during memory or IO operations, wait states are only injected
        if the 'wait active' happens on the same clock tick as the
        CPU would sample the wait line
    */
    int wait_scan_tick = -1;
    if (pins & Z80_MREQ) {
        /* a memory request or opcode fetch, wait is sampled on second clock tick */
        wait_scan_tick = 1;
    }
    else if (pins & Z80_IORQ) {
        if (pins & Z80_M1) {
            /* an interrupt acknowledge cycle, wait is sampled on fourth clock tick */
            wait_scan_tick = 3;
        }
        else {
            /* an IO cycle, wait is sampled on third clock tick */
            wait_scan_tick = 2;
        }
    }
    bool wait = false;
    uint32_t wait_cycles = 0;
    for (int i = 0; i<num_ticks; i++) {
        do {
            /* CPC gate array sets the wait pin for 3 out of 4 clock ticks */
            bool wait_pin = (sys->tick_count++ & 3) != 0;
            wait = (wait_pin && (wait || (i == wait_scan_tick)));
            if (wait) {
                wait_cycles++;
            }
            /* on every 4th clock cycle, tick the system */
            if (!wait_pin) {
                if (ay38910_tick(&sys->psg)) {
                    sys->sample_buffer[sys->sample_pos++] = sys->psg.sample;
                    if (sys->sample_pos == sys->num_samples) {
                        if (sys->audio_cb) {
                            sys->audio_cb(sys->sample_buffer, sys->num_samples);
                        }
                        sys->sample_pos = 0;
                    }
                }
                pins = _cpc_ga_tick(sys, pins);
            }
        }
        while (wait);
    }
    Z80_SET_WAIT(pins, wait_cycles);
    return pins;
}

uint64_t _cpc_cpu_iorq(cpc_t* sys, uint64_t pins) {
    /*
        CPU IO REQUEST

        For address decoding, see the main board schematics!
        also: http://cpcwiki.eu/index.php/Default_I/O_Port_Summary

        Z80 to i8255 PPI pin connections:
            ~A11 -> CS (CS is active-low)
                A8 -> A0
                A9 -> A1
                RD -> RD
                WR -> WR
            D0..D7 -> D0..D7
    */
    if ((pins & Z80_A11) == 0) {
        /* i8255 in/out */
        uint64_t ppi_pins = (pins & Z80_PIN_MASK)|I8255_CS;
        if (pins & Z80_A9) { ppi_pins |= I8255_A1; }
        if (pins & Z80_A8) { ppi_pins |= I8255_A0; }
        if (pins & Z80_RD) { ppi_pins |= I8255_RD; }
        if (pins & Z80_WR) { ppi_pins |= I8255_WR; }
        pins = i8255_iorq(&sys->ppi, ppi_pins) & Z80_PIN_MASK;
    }
    /*
        Z80 to MC6845 pin connections:

            ~A14 -> CS (CS is active low)
            A9  -> RW (high: read, low: write)
            A8  -> RS
        D0..D7  -> D0..D7
    */
    if ((pins & Z80_A14) == 0) {
        /* 6845 in/out */
        uint64_t vdg_pins = (pins & Z80_PIN_MASK)|MC6845_CS;
        if (pins & Z80_A9) { vdg_pins |= MC6845_RW; }
        if (pins & Z80_A8) { vdg_pins |= MC6845_RS; }
        pins = mc6845_iorq(&sys->vdg, vdg_pins) & Z80_PIN_MASK;
    }
    /*
        Gate Array Function (only writing to the gate array
        is possible, but the gate array doesn't check the
        CPU R/W pins, so each access is a write).

        This is used by the Arnold Acid test "OnlyInc", which
        access the PPI and gate array in the same IO operation
        to move data directly from the PPI into the gate array.
    */
    if ((pins & (Z80_A15|Z80_A14)) == Z80_A14) {
        /* D6 and D7 select the gate array operation */
        const uint8_t data = Z80_GET_DATA(pins);
        switch (data & ((1<<7)|(1<<6))) {
            case 0:
                /* select pen:
                    bit 4 set means 'select border', otherwise
                    bits 0..3 contain the pen number
                */
                sys->ga.pen = data & 0x1F;
                break;
            case (1<<6):
                /* select color for border or selected pen: */
                if (sys->ga.pen & (1<<4)) {
                    /* border color */
                    sys->ga.border_color = sys->ga.colors[data & 0x1F];
                }
                else {
                    sys->ga.palette[sys->ga.pen & 0x0F] = sys->ga.colors[data & 0x1F];
                }
                break;
            case (1<<7):
                /* select screen mode, ROM config and interrupt control
                    - bits 0 and 1 select the screen mode
                        00: Mode 0 (160x200 @ 16 colors)
                        01: Mode 1 (320x200 @ 4 colors)
                        02: Mode 2 (640x200 @ 2 colors)
                        11: Mode 3 (160x200 @ 2 colors, undocumented)
                  
                    - bit 2: disable/enable lower ROM
                    - bit 3: disable/enable upper ROM
                  
                    - bit 4: interrupt generation control
                */
                sys->ga.config = data;
                sys->ga.next_video_mode = data & 3;
                if (data & (1<<4)) {
                    sys->ga.hsync_irq_counter = 0;
                    sys->ga.intr = false;
                }
                _cpc_update_memory_mapping(sys);
                break;
            case (1<<7)|(1<<6):
                /* RAM memory management (only CPC6128) */
                if (CPC_TYPE_6128 == sys->type) {
                    sys->ga.ram_config = data;
                    _cpc_update_memory_mapping(sys);
                }
                break;
        }
    }
    /*
        Upper ROM Bank number

        This is used to select a ROM in the 0xC000..0xFFFF region, without expansions,
        this is just the BASIC and AMSDOS ROM.
    */
    if ((pins & Z80_A13) == 0) {
        sys->upper_rom_select = Z80_GET_DATA(pins);
        _cpc_update_memory_mapping(sys);
    }
    /*
        Floppy Disk Interface
    */
    if ((pins & (Z80_A10|Z80_A8|Z80_A7)) == 0) {
        /* FIXME: floppy disk motor control */
    }
    else if ((pins & (Z80_A10|Z80_A8|Z80_A7)) == Z80_A8) {
        /* floppy controller status/data register */
        if (pins & Z80_RD) {
            Z80_SET_DATA(pins, 0xFF);
        }
    }
    return pins;
}

uint64_t _cpc_ppi_out(int port_id, uint64_t pins, uint8_t data, void* user_data) {
    cpc_t* sys = (cpc_t*) user_data;
    /*
        i8255 PPI to AY-3-8912 PSG pin connections:
            PA0..PA7    -> D0..D7
                 PC7    -> BDIR
                 PC6    -> BC1
    */
    if ((I8255_PORT_A == port_id) || (I8255_PORT_C == port_id)) {
        const uint8_t ay_ctrl = sys->ppi.output[I8255_PORT_C] & ((1<<7)|(1<<6));
        if (ay_ctrl) {
            uint64_t ay_pins = 0;
            if (ay_ctrl & (1<<7)) { ay_pins |= AY38910_BDIR; }
            if (ay_ctrl & (1<<6)) { ay_pins |= AY38910_BC1; }
            const uint8_t ay_data = sys->ppi.output[I8255_PORT_A];
            AY38910_SET_DATA(ay_pins, ay_data);
            ay38910_iorq(&sys->psg, ay_pins);
        }
    }
    if (I8255_PORT_C == port_id) {
        // bits 0..3: select keyboard matrix line
        kbd_set_active_columns(&sys->kbd, 1<<(data & 0x0F));

        /* FIXME: cassette write data */
        /* FIXME: cassette deck motor control */
        if (data & (1<<4)) {
            // tape_start_motor();
        }
        else {
            // tape_stop_motor();
        }
    }
    return pins;
}

uint8_t _cpc_ppi_in(int port_id, void* user_data) {
    cpc_t* sys = (cpc_t*) user_data;
    if (I8255_PORT_A == port_id) {
        /* AY-3-8912 PSG function (indirectly this may also trigger
            a read of the keyboard matrix via the AY's IO port
        */
        uint64_t ay_pins = 0;
        uint8_t ay_ctrl = sys->ppi.output[I8255_PORT_C];
        if (ay_ctrl & (1<<7)) ay_pins |= AY38910_BDIR;
        if (ay_ctrl & (1<<6)) ay_pins |= AY38910_BC1;
        uint8_t ay_data = sys->ppi.output[I8255_PORT_A];
        AY38910_SET_DATA(ay_pins, ay_data);
        ay_pins = ay38910_iorq(&sys->psg, ay_pins);
        return AY38910_GET_DATA(ay_pins);
    }
    else if (I8255_PORT_B == port_id) {
        /*
            Bit 7: cassette data input
            Bit 6: printer port ready (1=not ready, 0=ready)
            Bit 5: expansion port /EXP pin
            Bit 4: screen refresh rate (1=50Hz, 0=60Hz)
            Bit 3..1: distributor id (shown in start screen)
                0: Isp
                1: Triumph
                2: Saisho
                3: Solavox
                4: Awa
                5: Schneider
                6: Orion
                7: Amstrad
            Bit 0: vsync
        */
        uint8_t val = (1<<4) | (7<<1);    // 50Hz refresh rate, Amstrad
        /* PPI Port B Bit 0 is directly wired to the 6845 VSYNC pin (see schematics) */
        if (sys->vdg.vs) {
            val |= (1<<0);
        }
        return val;
    }
    else {
        /* shouldn't happen */
        return 0xFF;
    }
}

void _cpc_psg_out(int port_id, uint8_t data, void* user_data) {
    /* this shouldn't be called */
}

uint8_t _cpc_psg_in(int port_id, void* user_data) {
    cpc_t* sys = (cpc_t*) user_data;
    /* read the keyboard matrix and joystick port */
    if (port_id == AY38910_PORT_A) {
        uint8_t data = (uint8_t) kbd_scan_lines(&sys->kbd);
        if (sys->kbd.active_columns & (1<<9)) {
            /*
                joystick input is implemented like this:
                - the keyboard column 9 is routed to the joystick
                  ports "COM1" pin, this means the joystick is only
                  "powered" when the keyboard line 9 is active
                - the joysticks direction and fire pins are
                  connected to the keyboard matrix lines as
                  input to PSG port A
                - thus, only when the keyboard matrix column 9 is sampled,
                  joystick input will be provided on the keyboard
                  matrix lines
            */
            data |= sys->joy_mask;
        }
        return ~data;
    }
    else {
        /* this shouldn't happen since the AY-3-8912 only has one IO port */
        return 0xFF;
    }
}

/* the first 32 bytes of the KC Compact color ROM */
static uint8_t _cpc_kcc_color_rom[32] = {
    0x15, 0x15, 0x31, 0x3d, 0x01, 0x0d, 0x11, 0x1d,
    0x0d, 0x3d, 0x3c, 0x3f, 0x0c, 0x0f, 0x1c, 0x1f,
    0x01, 0x31, 0x30, 0x33, 0x00, 0x03, 0x10, 0x13,
    0x05, 0x35, 0x34, 0x37, 0x04, 0x07, 0x14, 0x17
};

/*
  the fixed hardware color palette

  http://www.cpcwiki.eu/index.php/CPC_Palette
  http://www.grimware.org/doku.php/documentations/devices/gatearray

  index into this palette is the 'hardware color number' & 0x1F
  order is ABGR
*/
static uint32_t _cpc_colors[32] = {
    0xff6B7D6E,         // #40 white
    0xff6D7D6E,         // #41 white
    0xff6BF300,         // #42 sea green
    0xff6DF3F3,         // #43 pastel yellow
    0xff6B0200,         // #44 blue
    0xff6802F0,         // #45 purple
    0xff687800,         // #46 cyan
    0xff6B7DF3,         // #47 pink
    0xff6802F3,         // #48 purple
    0xff6BF3F3,         // #49 pastel yellow
    0xff0DF3F3,         // #4A bright yellow
    0xffF9F3FF,         // #4B bright white
    0xff0605F3,         // #4C bright red
    0xffF402F3,         // #4D bright magenta
    0xff0D7DF3,         // #4E orange
    0xffF980FA,         // #4F pastel magenta
    0xff680200,         // #50 blue
    0xff6BF302,         // #51 sea green
    0xff01F002,         // #52 bright green
    0xffF2F30F,         // #53 bright cyan
    0xff010200,         // #54 black
    0xffF4020C,         // #55 bright blue
    0xff017802,         // #56 green
    0xffF47B0C,         // #57 sky blue
    0xff680269,         // #58 magenta
    0xff6BF371,         // #59 pastel green
    0xff04F571,         // #5A lime
    0xffF4F371,         // #5B pastel cyan
    0xff01026C,         // #5C red
    0xffF2026C,         // #5D mauve
    0xff017B6E,         // #5E yellow
    0xffF67B6E,         // #5F pastel blue
};

static void _cpc_ga_init(cpc_t* sys) {
    memset(&sys->ga, 0, sizeof(sys->ga));
    sys->ga.next_video_mode = 1;
    sys->ga.video_mode = 1;
    sys->ga.hsync_delay_counter = 2;

    /* setup the hardware colors, these are different between KC Compact and CPC */
    if (CPC_TYPE_KCCOMPACT == sys->type) {
        /* setup from the KC Compact color ROM */
        for (int i = 0; i < 32; i++) {
            uint32_t rgba8 = 0xFF000000;
            const uint8_t val = _cpc_kcc_color_rom[i];
            /* color bits: xx|gg|rr|bb */
            const uint8_t b = val & 0x03;
            const uint8_t r = (val>>2) & 0x03;
            const uint8_t g = (val>>4) & 0x03;
            if (b == 0x03)     rgba8 |= 0x00FF0000;    /* full blue */
            else if (b != 0)   rgba8 |= 0x007F0000;    /* half blue */
            if (g == 0x03)     rgba8 |= 0x0000FF00;    /* full green */
            else if (g != 0)   rgba8 |= 0x00007F00;    /* half green */
            if (r == 0x03)     rgba8 |= 0x000000FF;    /* full red */
            else if (r != 0)   rgba8 |= 0x0000007F;    /* half red */
            sys->ga.colors[i] = rgba8;
        }
    }
    else {
        /* the original-CPC hardware colors */
        for (int i = 0; i < 32; i++) {
            sys->ga.colors[i] = _cpc_colors[i];
        }
    }
}

void _cpc_ga_int_ack(cpc_t* sys) {
    /* on interrupt acknowledge from the CPU, clear the top bit from the
        hsync counter, so the next interrupt can't occur closer then 
        32 HSYNC, and clear the gate array interrupt pin state
    */
    sys->ga.hsync_irq_counter &= 0x1F;
    sys->ga.intr = false;
}

static bool _cpc_falling_edge(uint64_t new_pins, uint64_t old_pins, uint64_t mask) {
    return 0 != (mask & (~new_pins & (new_pins ^ old_pins)));
}

static bool _cpc_rising_edge(uint64_t new_pins, uint64_t old_pins, uint64_t mask) {
    return 0 != (mask & (new_pins & (new_pins ^ old_pins)));
}

uint64_t _cpc_ga_tick(cpc_t* sys, uint64_t cpu_pins) {
    /*
        http://cpctech.cpc-live.com/docs/ints.html
        http://www.cpcwiki.eu/forum/programming/frame-flyback-and-interrupts/msg25106/#msg25106
        https://web.archive.org/web/20170612081209/http://www.grimware.org/doku.php/documentations/devices/gatearray
    */
    uint64_t crtc_pins = mc6845_tick(&sys->vdg);

    /*
        INTERRUPT GENERATION:

        From: https://web.archive.org/web/20170612081209/http://www.grimware.org/doku.php/documentations/devices/gatearray

        - On every falling edge of the HSYNC signal (from the 6845),
          the gate array will increment the counter by one. When the
          counter reaches 52, the gate array will raise the INT signal
          and reset the counter.
        - When the CPU acknowledges the interrupt, the gate array will
          reset bit5 of the counter, so the next interrupt can't occur
          closer than 32 HSync.
        - When a VSync occurs, the gate array will wait for 2 HSync and:
            - if the counter>=32 (bit5=1) then no interrupt request is issued
              and counter is reset to 0
            - if the counter<32 (bit5=0) then an interrupt request is issued
              and counter is reset to 0
        - This 2 HSync delay after a VSync is used to let the main program,
          executed by the CPU, enough time to sense the VSync...

        From: http://www.cpcwiki.eu/index.php?title=CRTC
        
        - The HSYNC is modified before being sent to the monitor. It happens
          2us after the HSYNC from the CRTC and lasts 4us when HSYNC length
          is greater or equal to 6.
        - The VSYNC is also modified before being sent to the monitor, it happens
          two lines after the VSYNC from the CRTC and stay 2 lines (same cut
          rule if VSYNC is lower than 4).

        NOTES:
            - the interrupt acknowledge is handled once per machine 
              cycle in cpu_tick()
            - the video mode will take effect *after the next HSYNC*
    */
    if (_cpc_rising_edge(crtc_pins, sys->ga.crtc_pins, MC6845_VS)) {
        sys->ga.hsync_after_vsync_counter = 2;
    }
    if (_cpc_falling_edge(crtc_pins, sys->ga.crtc_pins, MC6845_HS)) {
        sys->ga.video_mode = sys->ga.next_video_mode;
        sys->ga.hsync_irq_counter = (sys->ga.hsync_irq_counter + 1) & 0x3F;

        /* 2 HSync delay? */
        if (sys->ga.hsync_after_vsync_counter > 0) {
            sys->ga.hsync_after_vsync_counter--;
            if (sys->ga.hsync_after_vsync_counter == 0) {
                if (sys->ga.hsync_irq_counter >= 32) {
                    sys->ga.intr = true;
                }
                sys->ga.hsync_irq_counter = 0;
            }
        }
        /* normal behaviour, request interrupt each 52 scanlines */
        if (sys->ga.hsync_irq_counter == 52) {
            sys->ga.hsync_irq_counter = 0;
            sys->ga.intr = true;
        }
    }

    /* generate HSYNC signal to monitor:
        - starts 2 ticks after HSYNC rising edge from CRTC
        - stays active for 4 ticks or less if CRTC HSYNC goes inactive earlier
    */
    if (_cpc_rising_edge(crtc_pins, sys->ga.crtc_pins, MC6845_HS)) {
        sys->ga.hsync_delay_counter = 3;
    }
    if (_cpc_falling_edge(crtc_pins, sys->ga.crtc_pins, MC6845_HS)) {
        sys->ga.hsync_delay_counter = 0;
        sys->ga.hsync_counter = 0;
        sys->ga.sync = false;
    }
    if (sys->ga.hsync_delay_counter > 0) {
        sys->ga.hsync_delay_counter--;
        if (sys->ga.hsync_delay_counter == 0) {
            sys->ga.sync = true;
            sys->ga.hsync_counter = 5;
        }
    }
    if (sys->ga.hsync_counter > 0) {
        sys->ga.hsync_counter--;
        if (sys->ga.hsync_counter == 0) {
            sys->ga.sync = false;
        }
    }

    // FIXME delayed VSYNC to monitor

    const bool vsync = 0 != (crtc_pins & MC6845_VS);
    crt_tick(&sys->crt, sys->ga.sync, vsync);
    _cpc_ga_decode_video(sys, crtc_pins);

    sys->ga.crtc_pins = crtc_pins;

    if (sys->ga.intr) {
        cpu_pins |= Z80_INT;
    }
    return cpu_pins;
}

void _cpc_ga_decode_pixels(cpc_t* sys, uint32_t* dst, uint64_t crtc_pins) {
    /*
        compute the source address from current CRTC ma (memory address)
        and ra (raster address) like this:
    
        |ma12|ma11|ra2|ra1|ra0|ma9|ma8|...|ma2|ma1|ma0|0|
    
       Bits ma12 and m11 point to the 16 KByte page, and all
       other bits are the index into that page.
    */
    const uint16_t ma = MC6845_GET_ADDR(crtc_pins);
    const uint8_t ra = MC6845_GET_RA(crtc_pins);
    const uint32_t page_index  = (ma>>12) & 3;
    const uint32_t page_offset = ((ma & 0x03FF)<<1) | ((ra & 7)<<11);
    const uint8_t* src = &(sys->ram[page_index][page_offset]);
    uint8_t c;
    uint32_t p;
    if (0 == sys->ga.video_mode) {
        /* 160x200 @ 16 colors
           pixel    bit mask
           0:       |3|7|
           1:       |2|6|
           2:       |1|5|
           3:       |0|4|
        */
        for (int i = 0; i < 2; i++) {
            c = *src++;
            p = sys->ga.palette[((c>>7)&0x1)|((c>>2)&0x2)|((c>>3)&0x4)|((c<<2)&0x8)];
            *dst++ = p; *dst++ = p; *dst++ = p; *dst++ = p;
            p = sys->ga.palette[((c>>6)&0x1)|((c>>1)&0x2)|((c>>2)&0x4)|((c<<3)&0x8)];
            *dst++ = p; *dst++ = p; *dst++ = p; *dst++ = p;
        }
    }
    else if (1 == sys->ga.video_mode) {
        /* 320x200 @ 4 colors
           pixel    bit mask
           0:       |3|7|
           1:       |2|6|
           2:       |1|5|
           3:       |0|4|
        */
        for (int i = 0; i < 2; i++) {
            c = *src++;
            p = sys->ga.palette[((c>>2)&2)|((c>>7)&1)];
            *dst++ = p; *dst++ = p;
            p = sys->ga.palette[((c>>1)&2)|((c>>6)&1)];
            *dst++ = p; *dst++ = p;
            p = sys->ga.palette[((c>>0)&2)|((c>>5)&1)];
            *dst++ = p; *dst++ = p;
            p = sys->ga.palette[((c<<1)&2)|((c>>4)&1)];
            *dst++ = p; *dst++ = p;
        }
    }
    else if (2 == sys->ga.video_mode) {
        /* 640x200 @ 2 colors */
        for (int i = 0; i < 2; i++) {
            c = *src++;
            for (int j = 7; j >= 0; j--) {
                *dst++ = sys->ga.palette[(c>>j)&1];
            }
        }
    }
}

/* FIXME: debug visualzation! */
void _cpc_ga_decode_video(cpc_t* sys, uint64_t crtc_pins) {
    if (sys->crt.visible) {
        int dst_x = sys->crt.pos_x * 16;
        int dst_y = sys->crt.pos_y;
        uint32_t* dst = &(sys->pixel_buffer[dst_x + dst_y * CPC_DISPLAY_WIDTH]);
        if (crtc_pins & MC6845_DE) {
            /* decode visible pixels */
            _cpc_ga_decode_pixels(sys, dst, crtc_pins);
        }
        else if (crtc_pins & (MC6845_HS|MC6845_VS)) {
            /* during horizontal/vertical sync: blacker than black */
            for (int i = 0; i < 16; i++) {
                dst[i] = 0xFF000000;
            }
        }
        else {
            /* border color */
            for (int i = 0; i < 16; i++) {
                dst[i] = sys->ga.border_color;
            }
        }
    }
}

static void _cpc_init_keymap(cpc_t* sys) {
    /*
        http://cpctech.cpc-live.com/docs/keyboard.html
    
        CPC has a 10 columns by 8 lines keyboard matrix. The 10 columns
        are lit up by bits 0..3 of PPI port C connected to a 74LS145
        BCD decoder, and the lines are read through port A of the
        AY-3-8910 chip.
    */
    kbd_init(&sys->kbd, 1);
    const char* keymap =
        /* no shift */
        "   ^08641 "
        "  [-97532 "
        "   @oure  "
        "  ]piytwq "
        "   ;lhgs  "
        "   :kjfda "
        "  \\/mnbc  "
        "   ., vxz "

        /* shift */
        "    _(&$! "
        "  {=)'%#\" "
        "   |OURE  "
        "  }PIYTWQ "
        "   +LHGS  "
        "   *KJFDA "
        "  `?MNBC  "
        "   >< VXZ ";
    /* shift key is on column 2, line 5 */
    kbd_register_modifier(&sys->kbd, 0, 2, 5);
    /* ctrl key is on column 2, line 7 */
    kbd_register_modifier(&sys->kbd, 1, 2, 7);

    for (int shift = 0; shift < 2; shift++) {
        for (int col = 0; col < 10; col++) {
            for (int line = 0; line < 8; line++) {
                int c = keymap[shift*80 + line*10 + col];
                if (c != 0x20) {
                    kbd_register_key(&sys->kbd, c, col, line, shift?(1<<0):0);
                }
            }
        }
    }

    /* special keys */
    kbd_register_key(&sys->kbd, 0x20, 5, 7, 0);    /* space */
    kbd_register_key(&sys->kbd, 0x08, 1, 0, 0);    /* cursor left */
    kbd_register_key(&sys->kbd, 0x09, 0, 1, 0);    /* cursor right */
    kbd_register_key(&sys->kbd, 0x0A, 0, 2, 0);    /* cursor down */
    kbd_register_key(&sys->kbd, 0x0B, 0, 0, 0);    /* cursor up */
    kbd_register_key(&sys->kbd, 0x01, 9, 7, 0);    /* delete */
    kbd_register_key(&sys->kbd, 0x0C, 2, 0, 0);    /* clr */
    kbd_register_key(&sys->kbd, 0x0D, 2, 2, 0);    /* return */
    kbd_register_key(&sys->kbd, 0x03, 8, 2, 0);    /* escape */
}

/* CPC6128 RAM block indices */
static int _cpc_ram_config[8][4] = {
    { 0, 1, 2, 3 },
    { 0, 1, 2, 7 },
    { 4, 5, 6, 7 },
    { 0, 3, 2, 7 },
    { 0, 4, 2, 3 },
    { 0, 5, 2, 3 },
    { 0, 6, 2, 3 },
    { 0, 7, 2, 3 }
};

static void _cpc_update_memory_mapping(cpc_t* sys) {
    /* select RAM config and ROMs */
    int ram_config_index;
    const uint8_t* rom0_ptr;
    const uint8_t* rom1_ptr;
    if (CPC_TYPE_6128 == sys->type) {
        ram_config_index = sys->ga.ram_config & 7;
        rom0_ptr = sys->rom_6128_os;
        rom1_ptr = (sys->upper_rom_select == 7) ? sys->rom_6128_amsdos : sys->rom_6128_basic;
    }
    else if (CPC_TYPE_KCCOMPACT == sys->type) {
        ram_config_index = 0;
        rom0_ptr = sys->rom_464_os;
        rom1_ptr = sys->rom_464_basic;
    }
    else {
        ram_config_index = 0;
        rom0_ptr = sys->rom_kcc_os;
        rom1_ptr = sys->rom_kcc_basic;
    }
    const int i0 = _cpc_ram_config[ram_config_index][0];
    const int i1 = _cpc_ram_config[ram_config_index][1];
    const int i2 = _cpc_ram_config[ram_config_index][2];
    const int i3 = _cpc_ram_config[ram_config_index][3];

    /* 0x0000 .. 0x3FFF */
    if (sys->ga.config & (1<<2)) {
        /* read/write RAM */
        mem_map_ram(&sys->mem, 0, 0x0000, 0x4000, sys->ram[i0]);
    }
    else {
        /* RAM-behind-ROM */
        mem_map_rw(&sys->mem, 0, 0x0000, 0x4000, rom0_ptr, sys->ram[i0]);
    }
    /* 0x4000 .. 0x7FFF */
    mem_map_ram(&sys->mem, 0, 0x4000, 0x4000, sys->ram[i1]);
    /* 0x8000 .. 0xBFFF */
    mem_map_ram(&sys->mem, 0, 0x8000, 0x4000, sys->ram[i2]);
    /* 0xC000 .. 0xFFFF */
    if (sys->ga.config & (1<<3)) {
        /* read/write RAM */
        mem_map_ram(&sys->mem, 0, 0xC000, 0x4000, sys->ram[i3]);
    }
    else {
        /* RAM-behind-ROM */
        mem_map_rw(&sys->mem, 0, 0xC000, 0x4000, rom1_ptr, sys->ram[i3]);
    }
}

#endif /* CHIPS_IMPL */