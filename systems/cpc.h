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

    You need to include the following headers before including cpc.h:

    - chips/z80.h
    - chips/ay38910.h
    - chips/i8255.h
    - chips/mc6845.h
    - chips/am40010.h
    - chips/upd765.h
    - chips/mem.h
    - chips/kbd.h
    - chips/clk.h
    - chips/fdd.h
    - chips/fdd_cpc.h

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

#ifdef __cplusplus
extern "C" {
#endif

#define CPC_MAX_AUDIO_SAMPLES (1024)        /* max number of audio samples in internal sample buffer */
#define CPC_DEFAULT_AUDIO_SAMPLES (128)     /* default number of samples in internal sample buffer */
#define CPC_MAX_TAPE_SIZE (128*1024)        /* max size of tape file in bytes */

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
} cpc_joystick_type_t;

/* joystick mask bits */
#define CPC_JOYSTICK_UP    (1<<0)
#define CPC_JOYSTICK_DOWN  (1<<1)
#define CPC_JOYSTICK_LEFT  (1<<2)
#define CPC_JOYSTICK_RIGHT (1<<3)
#define CPC_JOYSTICK_BTN0  (1<<4)
#define CPC_JOYSTICK_BTN1  (1<<4)

/* audio sample data callback */
typedef void (*cpc_audio_callback_t)(const float* samples, int num_samples, void* user_data);

/* configuration parameters for cpc_init() */
typedef struct {
    cpc_type_t type;                /* default is the CPC 6128 */
    cpc_joystick_type_t joystick_type;

    /* video output config */
    void* pixel_buffer;         /* pointer to a linear RGBA8 pixel buffer, at least 1024*312*4 bytes */
    int pixel_buffer_size;      /* size of the pixel buffer in bytes */

    /* optional user-data for audio- and video-debugging callbacks */
    void* user_data;

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

/* CPC emulator state */
typedef struct {
    z80_t cpu;
    ay38910_t psg;
    mc6845_t crtc;
    am40010_t ga;
    i8255_t ppi;
    upd765_t fdc;

    bool valid;
    cpc_type_t type;
    cpc_joystick_type_t joystick_type;
    uint8_t kbd_joymask;
    uint8_t joy_joymask;
    uint16_t casread_trap;
    uint16_t casread_ret;

    clk_t clk;
    kbd_t kbd;
    mem_t mem;
    void* user_data;
    cpc_audio_callback_t audio_cb;
    int num_samples;
    int sample_pos;
    float sample_buffer[CPC_MAX_AUDIO_SAMPLES];
    uint8_t ram[8][0x4000];
    uint8_t rom_os[0x4000];
    uint8_t rom_basic[0x4000];
    uint8_t rom_amsdos[0x4000];
    /* tape loading */
    int tape_size;      /* tape_size is > 0 if a tape is inserted */
    int tape_pos;
    uint8_t tape_buf[CPC_MAX_TAPE_SIZE];
    /* floppy disc drive */
    fdd_t fdd;
} cpc_t;

/* initialize a new CPC instance */
void cpc_init(cpc_t* cpc, const cpc_desc_t* desc);
/* discard a CPC instance */
void cpc_discard(cpc_t* cpc);
/* get the standard framebuffer width and height in pixels */
int cpc_std_display_width(void);
int cpc_std_display_height(void);
/* get the maximum framebuffer size in number of bytes */
int cpc_max_display_size(void);
/* get the current framebuffer width and height in pixels */
int cpc_display_width(cpc_t* sys);
int cpc_display_height(cpc_t* sys);
/* reset a CPC instance */
void cpc_reset(cpc_t* cpc);
/* run CPC instance for given amount of micro_seconds */
void cpc_exec(cpc_t* cpc, uint32_t micro_seconds);
/* send a key down event */
void cpc_key_down(cpc_t* cpc, int key_code);
/* send a key up event */
void cpc_key_up(cpc_t* cpc, int key_code);
/* enable/disable joystick emulation */
void cpc_set_joystick_type(cpc_t* sys, cpc_joystick_type_t type);
/* get current joystick emulation type */
cpc_joystick_type_t cpc_joystick_type(cpc_t* sys);
/* set joystick mask (combination of CPC_JOYSTICK_*) */
void cpc_joystick(cpc_t* sys, uint8_t mask);
/* load a snapshot file (.sna or .bin) into the emulator */
bool cpc_quickload(cpc_t* cpc, const uint8_t* ptr, int num_bytes);
/* insert a tape file (.tap) */
bool cpc_insert_tape(cpc_t* cpc, const uint8_t* ptr, int num_bytes);
/* remove currently inserted tape */
void cpc_remove_tape(cpc_t* cpc);
/* insert a disk image file (.dsk) */
bool cpc_insert_disc(cpc_t* cpc, const uint8_t* ptr, int num_bytes);
/* remove current disc */
void cpc_remove_disc(cpc_t* cpc);
/* if enabled, start calling the video-debugging-callback */
void cpc_enable_video_debugging(cpc_t* cpc, bool enabled);
/* get current display debug visualization enabled/disabled state */
bool cpc_video_debugging_enabled(cpc_t* cpc);

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

#define _CPC_FREQUENCY (4000000)

static uint64_t _cpc_tick(int num, uint64_t pins, void* user_data);
static uint64_t _cpc_cclk(void* user_data);
static void _cpc_psg_out(int port_id, uint8_t data, void* user_data);
static uint8_t _cpc_psg_in(int port_id, void* user_data);
static void _cpc_init_keymap(cpc_t* sys);
static void _cpc_bankswitch(uint8_t ram_config, uint8_t rom_enable, uint8_t rom_select, void* user_data);
static void _cpc_cas_read(cpc_t* sys);
static int _cpc_fdc_seektrack(int drive, int track, void* user_data);
static int _cpc_fdc_seeksector(int drive, upd765_sectorinfo_t* inout_info, void* user_data);
static int _cpc_fdc_read(int drive, uint8_t h, void* user_data, uint8_t* out_data);
static int _cpc_fdc_trackinfo(int drive, int side, void* user_data, upd765_sectorinfo_t* out_info);
static void _cpc_fdc_driveinfo(int drive, void* user_data, upd765_driveinfo_t* out_info);

#define _CPC_DEFAULT(val,def) (((val) != 0) ? (val) : (def));
#define _CPC_CLEAR(val) memset(&val, 0, sizeof(val))

void cpc_init(cpc_t* sys, const cpc_desc_t* desc) {
    CHIPS_ASSERT(sys && desc);
    CHIPS_ASSERT(desc->pixel_buffer && (desc->pixel_buffer_size >= cpc_max_display_size()));

    memset(sys, 0, sizeof(cpc_t));
    sys->valid = true;
    sys->type = desc->type;
    sys->joystick_type = desc->joystick_type;
    if (CPC_TYPE_464 == desc->type) {
        CHIPS_ASSERT(desc->rom_464_os && (desc->rom_464_os_size == 0x4000));
        CHIPS_ASSERT(desc->rom_464_basic && (desc->rom_464_basic_size == 0x4000));
        memcpy(sys->rom_os, desc->rom_464_os, 0x4000);
        memcpy(sys->rom_basic, desc->rom_464_basic, 0x4000);
    }
    else if (CPC_TYPE_6128 == desc->type) {
        CHIPS_ASSERT(desc->rom_6128_os && (desc->rom_6128_os_size == 0x4000));
        CHIPS_ASSERT(desc->rom_6128_basic && (desc->rom_6128_basic_size == 0x4000));
        CHIPS_ASSERT(desc->rom_6128_amsdos && (desc->rom_6128_amsdos_size == 0x4000));
        memcpy(sys->rom_os, desc->rom_6128_os, 0x4000);
        memcpy(sys->rom_basic, desc->rom_6128_basic, 0x4000);
        memcpy(sys->rom_amsdos, desc->rom_6128_amsdos, 0x4000);
    }
    else { /* KC Compact */
        CHIPS_ASSERT(desc->rom_kcc_os && (desc->rom_kcc_os_size == 0x4000));
        CHIPS_ASSERT(desc->rom_kcc_basic && (desc->rom_kcc_basic_size == 0x4000));
        memcpy(sys->rom_os, desc->rom_kcc_os, 0x4000);
        memcpy(sys->rom_basic, desc->rom_kcc_basic, 0x4000);
    }
    sys->user_data = desc->user_data;
    sys->audio_cb = desc->audio_cb;
    sys->num_samples = _CPC_DEFAULT(desc->audio_num_samples, CPC_DEFAULT_AUDIO_SAMPLES);
    CHIPS_ASSERT(sys->num_samples <= CPC_MAX_AUDIO_SAMPLES);

    /* initialize the hardware */
    clk_init(&sys->clk, _CPC_FREQUENCY);
    mem_init(&sys->mem);

    z80_desc_t cpu_desc;
    _CPC_CLEAR(cpu_desc);
    cpu_desc.tick_cb = _cpc_tick;
    cpu_desc.user_data = sys;
    z80_init(&sys->cpu, &cpu_desc);

    i8255_init(&sys->ppi);

    ay38910_desc_t psg_desc;
    _CPC_CLEAR(psg_desc);
    psg_desc.type = AY38910_TYPE_8912;
    psg_desc.in_cb = _cpc_psg_in;
    psg_desc.out_cb = _cpc_psg_out;
    psg_desc.tick_hz = _CPC_FREQUENCY / 4;
    psg_desc.sound_hz = _CPC_DEFAULT(desc->audio_sample_rate, 44100);
    psg_desc.magnitude = _CPC_DEFAULT(desc->audio_volume, 0.5f);
    psg_desc.user_data = sys;
    ay38910_init(&sys->psg, &psg_desc);

    mc6845_init(&sys->crtc, MC6845_TYPE_UM6845R);

    am40010_desc_t ga_desc;
    _CPC_CLEAR(ga_desc);
    ga_desc.cpc_type = (am40010_cpc_type_t) sys->type;
    ga_desc.bankswitch_cb = _cpc_bankswitch;
    ga_desc.cclk_cb = _cpc_cclk;
    ga_desc.ram = &sys->ram[0][0];
    ga_desc.ram_size = sizeof(sys->ram);
    ga_desc.rgba8_buffer = (uint32_t*) desc->pixel_buffer;
    ga_desc.rgba8_buffer_size = desc->pixel_buffer_size;
    ga_desc.user_data = sys;
    am40010_init(&sys->ga, &ga_desc);

    upd765_desc_t fdc_desc;
    _CPC_CLEAR(fdc_desc);
    fdc_desc.seektrack_cb = _cpc_fdc_seektrack;
    fdc_desc.seeksector_cb = _cpc_fdc_seeksector;
    fdc_desc.read_cb = _cpc_fdc_read;
    fdc_desc.trackinfo_cb = _cpc_fdc_trackinfo;
    fdc_desc.driveinfo_cb = _cpc_fdc_driveinfo;
    fdc_desc.user_data = sys;
    upd765_init(&sys->fdc, &fdc_desc);
    fdd_init(&sys->fdd);

    _cpc_init_keymap(sys);

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
    /* execution starts as address 0 */
    z80_set_pc(&sys->cpu, 0x0000);
}

void cpc_discard(cpc_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    sys->valid = false;
}

int cpc_std_display_width(void) {
    return AM40010_DISPLAY_WIDTH;
}

int cpc_std_display_height(void) {
    return AM40010_DISPLAY_HEIGHT;
}

int cpc_max_display_size(void) {
    /* take debugging visualization into account */
    return AM40010_DBG_DISPLAY_WIDTH * AM40010_DBG_DISPLAY_HEIGHT * 4;
}

int cpc_display_width(cpc_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    return sys->ga.dbg_vis ? AM40010_DBG_DISPLAY_WIDTH : AM40010_DISPLAY_WIDTH;
}

int cpc_display_height(cpc_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    return sys->ga.dbg_vis ? AM40010_DBG_DISPLAY_HEIGHT : AM40010_DISPLAY_HEIGHT;
}

void cpc_reset(cpc_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    mem_unmap_all(&sys->mem);
    mc6845_reset(&sys->crtc);
    ay38910_reset(&sys->psg);
    i8255_reset(&sys->ppi);
    am40010_reset(&sys->ga);
    z80_reset(&sys->cpu);
    z80_set_pc(&sys->cpu, 0x0000);
    sys->kbd_joymask = 0;
    sys->joy_joymask = 0;
}

void cpc_exec(cpc_t* sys, uint32_t micro_seconds) {
    CHIPS_ASSERT(sys && sys->valid);
    uint32_t ticks_to_run = clk_ticks_to_run(&sys->clk, micro_seconds);
    uint32_t ticks_executed = 0;
    int trap_id = 0;
    while ((ticks_executed < ticks_to_run) && (0 == trap_id)) {
        ticks_executed += z80_exec(&sys->cpu, ticks_to_run);
        /* check if casread trap has been hit, and the right ROM is mapped in */
        trap_id = sys->cpu.trap_id;
        if (trap_id == 1) {
            if (sys->type == CPC_TYPE_6128) {
                if (0 == (sys->ga.regs.config & (1<<2))) {
                    _cpc_cas_read(sys);
                }
            }
            else {
                /* no memory mapping on KC Compact, 464 or 664 */
                _cpc_cas_read(sys);
            }
            trap_id = 0;
        }
    }
    clk_ticks_executed(&sys->clk, ticks_executed);
    kbd_update(&sys->kbd, micro_seconds);
}

void cpc_key_down(cpc_t* sys, int key_code) {
    CHIPS_ASSERT(sys && sys->valid);
    if (sys->joystick_type == CPC_JOYSTICK_DIGITAL) {
        switch (key_code) {
            case 0x20: sys->kbd_joymask |= CPC_JOYSTICK_BTN0; break;
            case 0x08: sys->kbd_joymask |= CPC_JOYSTICK_LEFT; break;
            case 0x09: sys->kbd_joymask |= CPC_JOYSTICK_RIGHT; break;
            case 0x0A: sys->kbd_joymask |= CPC_JOYSTICK_DOWN; break;
            case 0x0B: sys->kbd_joymask |= CPC_JOYSTICK_UP; break;
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
            case 0x20: sys->kbd_joymask &= ~CPC_JOYSTICK_BTN0; break;
            case 0x08: sys->kbd_joymask &= ~CPC_JOYSTICK_LEFT; break;
            case 0x09: sys->kbd_joymask &= ~CPC_JOYSTICK_RIGHT; break;
            case 0x0A: sys->kbd_joymask &= ~CPC_JOYSTICK_DOWN; break;
            case 0x0B: sys->kbd_joymask &= ~CPC_JOYSTICK_UP; break;
            default: kbd_key_up(&sys->kbd, key_code); break;
        }
    }
    else {
        kbd_key_up(&sys->kbd, key_code);
    }
}

void cpc_set_joystick_type(cpc_t* sys, cpc_joystick_type_t type) {
    CHIPS_ASSERT(sys && sys->valid);
    sys->joystick_type = type;
}

cpc_joystick_type_t cpc_joystick_type(cpc_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    return sys->joystick_type;
}

void cpc_joystick(cpc_t* sys, uint8_t mask) {
    CHIPS_ASSERT(sys && sys->valid);
    sys->joy_joymask = mask;
}

void cpc_enable_video_debugging(cpc_t* sys, bool enabled) {
    CHIPS_ASSERT(sys && sys->valid);
    sys->ga.dbg_vis = enabled;
}

bool cpc_video_debugging_enabled(cpc_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    return sys->ga.dbg_vis;
}

/* the CPU tick callback */
static uint64_t _cpc_tick(int num_ticks, uint64_t cpu_pins, void* user_data) {
    cpc_t* sys = (cpc_t*) user_data;

    /* memory and IO requests */
    if (cpu_pins & Z80_MREQ) {
        /* CPU MEMORY REQUEST */
        const uint16_t addr = Z80_GET_ADDR(cpu_pins);
        if (cpu_pins & Z80_RD) {
            Z80_SET_DATA(cpu_pins, mem_rd(&sys->mem, addr));
        }
        else if (cpu_pins & Z80_WR) {
            mem_wr(&sys->mem, addr, Z80_GET_DATA(cpu_pins));
        }
    }
    else if ((cpu_pins & Z80_IORQ) && (cpu_pins & (Z80_RD|Z80_WR))) {
        /* CPU IO address decoding

            For address decoding, see the main board schematics!
            also: http://cpcwiki.eu/index.php/Default_I/O_Port_Summary
        */

        /*
            Z80 to i8255 PPI pin connections:
                ~A11 -> CS (CS is active-low)
                    A8 -> A0
                    A9 -> A1
                    RD -> RD
                    WR -> WR
                D0..D7 -> D0..D7

            i8255 PPI to AY-3-8912 PSG pin connections:
                PA0..PA7    -> D0..D7
                     PC7    -> BDIR
                     PC6    -> BC1

            i8255 Port B:
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

            i8255 Port C:
                PC0..PC3: select keyboard matrix line
        */
        if ((cpu_pins & Z80_A11) == 0) {
            /* i8255 in/out */
            uint64_t ppi_pins = (cpu_pins & Z80_PIN_MASK & ~(I8255_PC_PINS|I8255_A1|I8255_A0)) | I8255_CS;
            if (cpu_pins & Z80_A9) { ppi_pins |= I8255_A1; }
            if (cpu_pins & Z80_A8) { ppi_pins |= I8255_A0; }
            if ((sys->ppi.pins & (I8255_PC7|I8255_PC6)) != 0) {
                uint64_t ay_pins = 0;
                if (sys->ppi.pins & I8255_PC7) { ay_pins |= AY38910_BDIR; }
                if (sys->ppi.pins & I8255_PC6) { ay_pins |= AY38910_BC1; }
                const uint8_t ay_data = I8255_GET_PA(sys->ppi.pins);
                AY38910_SET_DATA(ay_pins, ay_data);
                ay_pins = ay38910_iorq(&sys->psg, ay_pins);
                I8255_SET_PA(ppi_pins, AY38910_DATA(ay_pins));
            }
            ppi_pins |= I8255_PB1|I8255_PB2|I8255_PB3;  /* Amstrad */
            ppi_pins |= I8255_PB4;  /* PAL (50Hz) */
            if (sys->crtc.vs) {
                ppi_pins |= I8255_PB0;   /* VSYNC */
            }
            ppi_pins = i8255_tick(&sys->ppi, ppi_pins);
            /* copy data bus value to cpu pins */
            if ((ppi_pins & (I8255_CS|I8255_RD)) == (I8255_CS|I8255_RD)) {
                Z80_SET_DATA(cpu_pins, I8255_GET_DATA(ppi_pins));
            }
            /* update PSG state */
            if ((ppi_pins & (I8255_PC7|I8255_PC6)) != 0) {
                uint64_t ay_pins = 0;
                if (ppi_pins & I8255_PC7) { ay_pins |= AY38910_BDIR; }
                if (ppi_pins & I8255_PC6) { ay_pins |= AY38910_BC1; }
                const uint8_t ay_data = I8255_GET_PA(ppi_pins);
                AY38910_SET_DATA(ay_pins, ay_data);
                ay38910_iorq(&sys->psg, ay_pins);
            }
            /* PC0..PC3: select keyboard matrix line*/
            uint16_t col_mask = 1<<(I8255_GET_PC(ppi_pins) & 0x0F);
            kbd_set_active_columns(&sys->kbd, col_mask);
            /* FIXME: cassette write data */
            /* FIXME: cassette deck motor control */
        }
        /*
            Gate Array Function and upper rom select
            (only writing to the gate array
            is possible, but the gate array doesn't check the
            CPU R/W pins, so each access is a write).

            This is used by the Arnold Acid test "OnlyInc", which
            access the PPI and gate array in the same IO operation
            to move data directly from the PPI into the gate array.

            Because of this the gate array must be ticker *after* the PPI
            and use the returned pins from the PPI as input.
        */
        am40010_iorq(&sys->ga, cpu_pins);
        /*
            Z80 to MC6845 pin connections:

                ~A14 -> CS (CS is active low)
                A9  -> RW (high: read, low: write)
                A8  -> RS
            D0..D7  -> D0..D7
        */
        if ((cpu_pins & Z80_A14) == 0) {
            /* 6845 in/out */
            uint64_t crtc_pins = (cpu_pins & Z80_PIN_MASK)|MC6845_CS;
            if (cpu_pins & Z80_A9) { crtc_pins |= MC6845_RW; }
            if (cpu_pins & Z80_A8) { crtc_pins |= MC6845_RS; }
            cpu_pins = mc6845_iorq(&sys->crtc, crtc_pins) & Z80_PIN_MASK;
        }
        /*
            Floppy Disk Interface
        */
        if ((cpu_pins & (Z80_A10|Z80_A8|Z80_A7)) == 0) {
            if (cpu_pins & Z80_WR) {
                fdd_motor(&sys->fdd, 0 != (Z80_GET_DATA(cpu_pins) & 1));
            }
        }
        else if ((cpu_pins & (Z80_A10|Z80_A8|Z80_A7)) == Z80_A8) {
            /* floppy controller status/data register */
            uint64_t fdc_pins = UPD765_CS | (cpu_pins & Z80_PIN_MASK);
            cpu_pins = upd765_iorq(&sys->fdc, fdc_pins) & Z80_PIN_MASK;
        }
    }

    /* Tick the gate array, this will in turn tick the
       CRTC and PSG chips at the generated 1 MHz CCLK frequency
       (see _cpc_cclk callback). The returned CPU pin mask
       has been updated with the necessary WAIT states to inject, and
       the INT pin when the gate array requests an interrupt
    */
    cpu_pins = am40010_tick(&sys->ga, num_ticks, cpu_pins) & Z80_PIN_MASK;
    return cpu_pins;
}

/* called when a new sample is ready from the sound chip */
static inline void _cpc_sample_ready(cpc_t* sys) {
    sys->sample_buffer[sys->sample_pos++] = sys->psg.sample;
    if (sys->sample_pos == sys->num_samples) {
        if (sys->audio_cb) {
            /* new sample packet is ready */
            sys->audio_cb(sys->sample_buffer, sys->num_samples, sys->user_data);
        }
        sys->sample_pos = 0;
    }
}

/* handle a 1 MHz CCLK tick generated by the gate array, this ticks the
   MC6845 CRTC and AY-3-8912 PSG, and must return the CRTC pins.
*/
static uint64_t _cpc_cclk(void* user_data) {
    cpc_t* sys = (cpc_t*) user_data;
    /* tick the sound chip... */
    if (ay38910_tick(&sys->psg)) {
        /* new sound sample ready */
        _cpc_sample_ready(sys);
    }
    /* tick the CRTC and return its pin mask */
    uint64_t crtc_pins = mc6845_tick(&sys->crtc);
    return crtc_pins;
}

/* PSG OUT callback (nothing to do here) */
static void _cpc_psg_out(int port_id, uint8_t data, void* user_data) {
    /* this shouldn't be called */
    (void)port_id;
    (void)data;
    (void)user_data;
}

/* PSG IN callback (read keyboard matrix and joystick port) */
static uint8_t _cpc_psg_in(int port_id, void* user_data) {
    cpc_t* sys = (cpc_t*) user_data;
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
            data |= (sys->kbd_joymask | sys->joy_joymask);
        }
        return ~data;
    }
    else {
        /* this shouldn't happen since the AY-3-8912 only has one IO port */
        return 0xFF;
    }
}

/* keyboard matrix initialization */
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
    kbd_register_key(&sys->kbd, 0x20, 5, 7, 0); /* space */
    kbd_register_key(&sys->kbd, 0x08, 1, 0, 0); /* cursor left */
    kbd_register_key(&sys->kbd, 0x09, 0, 1, 0); /* cursor right */
    kbd_register_key(&sys->kbd, 0x0A, 0, 2, 0); /* cursor down */
    kbd_register_key(&sys->kbd, 0x0B, 0, 0, 0); /* cursor up */
    kbd_register_key(&sys->kbd, 0x01, 9, 7, 0); /* delete */
    kbd_register_key(&sys->kbd, 0x0C, 2, 0, 0); /* clr */
    kbd_register_key(&sys->kbd, 0x0D, 2, 2, 0); /* return */
    kbd_register_key(&sys->kbd, 0x03, 8, 2, 0); /* escape */
    kbd_register_key(&sys->kbd, 0xF1, 1, 5, 0); /* F1...*/
    kbd_register_key(&sys->kbd, 0xF2, 1, 6, 0);
    kbd_register_key(&sys->kbd, 0xF3, 0, 5, 0);
    kbd_register_key(&sys->kbd, 0xF4, 4, 2, 0);
    kbd_register_key(&sys->kbd, 0xF5, 4, 1, 0);
    kbd_register_key(&sys->kbd, 0xF6, 4, 0, 0);
    kbd_register_key(&sys->kbd, 0xF7, 2, 1, 0);
    kbd_register_key(&sys->kbd, 0xF8, 3, 1, 0);
    kbd_register_key(&sys->kbd, 0xF9, 3, 0, 0);
    kbd_register_key(&sys->kbd, 0xFA, 7, 1, 0); /* F0 -> F10 */
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

/* memory bankswitch callback, invoked by gate array (am40010) */
static void _cpc_bankswitch(uint8_t ram_config, uint8_t rom_enable, uint8_t rom_select, void* user_data) {
    cpc_t* sys = (cpc_t*) user_data;
    int ram_config_index;
    const uint8_t* rom0_ptr;
    const uint8_t* rom1_ptr;
    if (CPC_TYPE_6128 == sys->type) {
        ram_config_index = ram_config & 7;
        rom0_ptr = sys->rom_os;
        rom1_ptr = (rom_select == 7) ? sys->rom_amsdos : sys->rom_basic;
    }
    else {
        ram_config_index = 0;
        rom0_ptr = sys->rom_os;
        rom1_ptr = sys->rom_basic;
    }
    const int i0 = _cpc_ram_config[ram_config_index][0];
    const int i1 = _cpc_ram_config[ram_config_index][1];
    const int i2 = _cpc_ram_config[ram_config_index][2];
    const int i3 = _cpc_ram_config[ram_config_index][3];

    /* 0x0000 .. 0x3FFF */
    if (rom_enable & AM40010_CONFIG_LROMEN) {
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
    if (rom_enable & AM40010_CONFIG_HROMEN) {
        /* read/write RAM */
        mem_map_ram(&sys->mem, 0, 0xC000, 0x4000, sys->ram[i3]);
    }
    else {
        /* RAM-behind-ROM */
        mem_map_rw(&sys->mem, 0, 0xC000, 0x4000, rom1_ptr, sys->ram[i3]);
    }
}

/*=== SNAPSHOT FILE LOADING ==================================================*/

/* CPC SNA fileformat header: http://cpctech.cpc-live.com/docs/snapshot.html */
typedef struct {
    uint8_t magic[8];     // must be "MV - SNA"
    uint8_t pad0[8];
    uint8_t version;
    uint8_t F, A, C, B, E, D, L, H, R, I;
    uint8_t IFF1, IFF2;
    uint8_t IX_l, IX_h;
    uint8_t IY_l, IY_h;
    uint8_t SP_l, SP_h;
    uint8_t PC_l, PC_h;
    uint8_t IM;
    uint8_t F_, A_, C_, B_, E_, D_, L_, H_;
    uint8_t selected_pen;
    uint8_t pens[17];             // palette + border colors
    uint8_t gate_array_config;
    uint8_t ram_config;
    uint8_t crtc_selected;
    uint8_t crtc_regs[18];
    uint8_t rom_config;
    uint8_t ppi_a;
    uint8_t ppi_b;
    uint8_t ppi_c;
    uint8_t ppi_control;
    uint8_t psg_selected;
    uint8_t psg_regs[16];
    uint8_t dump_size_l;
    uint8_t dump_size_h;
    uint8_t pad1[0x93];
} _cpc_sna_header;

static bool _cpc_is_valid_sna(const uint8_t* ptr, int num_bytes) {
    if (num_bytes <= (int)sizeof(_cpc_sna_header)) {
        return false;
    }
    const _cpc_sna_header* hdr = (const _cpc_sna_header*) ptr;
    static uint8_t magic[8] = { 'M', 'V', 0x20, '-', 0x20, 'S', 'N', 'A' };
    for (int i = 0; i < 8; i++) {
        if (magic[i] != hdr->magic[i]) {
            return false;
        }
    }
    /* FIXME: check version field? */
    return true;
}

static bool _cpc_load_sna(cpc_t* sys, const uint8_t* ptr, int num_bytes) {
    const _cpc_sna_header* hdr = (const _cpc_sna_header*) ptr;
    ptr += sizeof(_cpc_sna_header);
    
    // copy 64 or 128 KByte memory dump
    const uint16_t dump_size = hdr->dump_size_h<<8 | hdr->dump_size_l;
    const uint32_t dump_num_bytes = (dump_size == 64) ? 0x10000 : 0x20000;
    if (num_bytes > (int) (sizeof(_cpc_sna_header) + dump_num_bytes)) {
        return false;
    }
    if (dump_num_bytes > sizeof(sys->ram)) {
        return false;
    }
    memcpy(sys->ram, ptr, dump_num_bytes);

//    z80_reset(&sys->cpu);
    z80_set_f(&sys->cpu, hdr->F); z80_set_a(&sys->cpu,hdr->A);
    z80_set_c(&sys->cpu, hdr->C); z80_set_b(&sys->cpu, hdr->B);
    z80_set_e(&sys->cpu, hdr->E); z80_set_d(&sys->cpu, hdr->D);
    z80_set_l(&sys->cpu, hdr->L); z80_set_h(&sys->cpu, hdr->H);
    z80_set_r(&sys->cpu, hdr->R); z80_set_i(&sys->cpu, hdr->I);
    z80_set_iff1(&sys->cpu, (hdr->IFF1 & 1) != 0);
    z80_set_iff2(&sys->cpu, (hdr->IFF2 & 1) != 0);
    z80_set_ix(&sys->cpu, hdr->IX_h<<8 | hdr->IX_l);
    z80_set_iy(&sys->cpu, hdr->IY_h<<8 | hdr->IY_l);
    z80_set_sp(&sys->cpu, hdr->SP_h<<8 | hdr->SP_l);
    z80_set_pc(&sys->cpu, hdr->PC_h<<8 | hdr->PC_l);
    z80_set_im(&sys->cpu, hdr->IM);
    z80_set_af_(&sys->cpu, hdr->A_<<8 | hdr->F_);
    z80_set_bc_(&sys->cpu, hdr->B_<<8 | hdr->C_);
    z80_set_de_(&sys->cpu, hdr->D_<<8 | hdr->E_);
    z80_set_hl_(&sys->cpu, hdr->H_<<8 | hdr->L_);

    sys->ga.colors.dirty = true;
    for (int i = 0; i < 16; i++) {
        sys->ga.regs.ink[i] = hdr->pens[i] & 0x1F;
    }
    sys->ga.regs.border = hdr->pens[16] & 0x1F;
    sys->ga.regs.inksel = hdr->selected_pen & 0x1F;
    sys->ga.regs.config = hdr->gate_array_config & 0x3F;
    sys->ga.ram_config = hdr->ram_config & 0x3F;
    sys->ga.rom_select = hdr->rom_config;
    _cpc_bankswitch(sys->ga.ram_config, sys->ga.regs.config, sys->ga.rom_select, sys);

    for (int i = 0; i < 18; i++) {
        sys->crtc.reg[i] = hdr->crtc_regs[i];
    }
    sys->crtc.sel = hdr->crtc_selected;

    sys->ppi.pa.outp = hdr->ppi_a;
    sys->ppi.pb.outp = hdr->ppi_b;
    sys->ppi.pc.outp = hdr->ppi_c;
    sys->ppi.control = hdr->ppi_control;

    for (int i = 0; i < 16; i++) {
        ay38910_iorq(&sys->psg, AY38910_BDIR|AY38910_BC1|(i<<16));
        ay38910_iorq(&sys->psg, AY38910_BDIR|(hdr->psg_regs[i]<<16));
    }
    ay38910_iorq(&sys->psg, AY38910_BDIR|AY38910_BC1|(hdr->psg_selected<<16));
    return true;
}

/* CPC AMSDOS BIN files */
typedef struct {
    uint8_t user_number;
    uint8_t file_name[8];
    uint8_t file_ext[3];
    uint8_t pad_0[6];
    uint8_t type;
    uint8_t pad_1[2];
    uint8_t load_addr_l;
    uint8_t load_addr_h;
    uint8_t pad_2;
    uint8_t length_l;
    uint8_t length_h;
    uint8_t start_addr_l;
    uint8_t start_addr_h;
    uint8_t pad_4[4];
    uint8_t pad_5[0x60];
} _cpc_bin_header;

static bool _cpc_is_valid_bin(int num_bytes) {
    if (num_bytes <= (int)sizeof(_cpc_bin_header)) {
        return false;
    }
    return true;
}

static bool _cpc_load_bin(cpc_t* sys, const uint8_t* ptr) {
    const _cpc_bin_header* hdr = (const _cpc_bin_header*) ptr;
    ptr += sizeof(_cpc_bin_header);
    const uint16_t load_addr = (hdr->load_addr_h<<8)|hdr->load_addr_l;
    const uint16_t start_addr = (hdr->start_addr_h<<8)|hdr->start_addr_l;
    const uint16_t len = (hdr->length_h<<8)|hdr->length_l;
    for (uint16_t i = 0; i < len; i++) {
        mem_wr(&sys->mem, load_addr+i, *ptr++);
    }
    z80_set_iff1(&sys->cpu, true);
    z80_set_iff2(&sys->cpu, true);
    z80_set_c(&sys->cpu, 0);        /* FIXME: "ROM select number" */
    z80_set_hl(&sys->cpu, start_addr);
    z80_set_pc(&sys->cpu, 0xBD16);  /* MC START PROGRAM */
    return true;}

bool cpc_quickload(cpc_t* sys, const uint8_t* ptr, int num_bytes) {
    CHIPS_ASSERT(sys && sys->valid && ptr);
    if (_cpc_is_valid_sna(ptr, num_bytes)) {
        return _cpc_load_sna(sys, ptr, num_bytes);
    }
    else if (_cpc_is_valid_bin(num_bytes)) {
        return _cpc_load_bin(sys, ptr);
    }
    else {
        /* not a known file type, or not enough data */
        return false;
    }
}

/*=== CASSETTE TAPE FILE LOADING =============================================*/
/* CPU trap handler to check for casread */
static int _cpc_trap_cb(uint16_t pc, uint32_t ticks, uint64_t pins, void* user_data) {
    (void)ticks;
    (void)pins;
    cpc_t* sys = (cpc_t*) user_data;
    return (pc == sys->casread_trap) ? 1 : 0;
}

bool cpc_insert_tape(cpc_t* sys, const uint8_t* ptr, int num_bytes) {
    CHIPS_ASSERT(sys && sys->valid);
    CHIPS_ASSERT(ptr);
    cpc_remove_tape(sys);
    if (num_bytes > CPC_MAX_TAPE_SIZE) {
        return false;
    }
    memcpy(sys->tape_buf, ptr, num_bytes);
    sys->tape_pos = 0;
    sys->tape_size = num_bytes;
    z80_trap_cb(&sys->cpu, _cpc_trap_cb, sys);
    return true;
}

void cpc_remove_tape(cpc_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    sys->tape_pos = 0;
    sys->tape_size = 0;
    z80_trap_cb(&sys->cpu, 0, 0);
}

/* the trapped OS casread function, reads one tape block into memory */
static void _cpc_cas_read(cpc_t* sys) {
    bool success = false;
    /* if no tape is currently inserted, both tape_pos and tape_size is 0 */
    if ((sys->tape_pos + 3) < sys->tape_size) {
        uint8_t len_l = sys->tape_buf[sys->tape_pos++];
        uint8_t len_h = sys->tape_buf[sys->tape_pos++];
        uint16_t len = len_h<<8 | len_l;
        if ((sys->tape_pos + len) <= sys->tape_size) {
            uint8_t sync = sys->tape_buf[sys->tape_pos++];
            if (sync == z80_a(&sys->cpu)) {
                success = true;
                for (uint16_t i = 0; i < (len-1); i++) {
                    uint16_t hl = z80_hl(&sys->cpu);
                    uint8_t val = sys->tape_buf[sys->tape_pos++];
                    mem_wr(&sys->mem, hl++, val);
                    z80_set_hl(&sys->cpu, hl);
                }
            }
        }
    }
    z80_set_f(&sys->cpu, success ? 0x45 : 0x00);
    z80_set_pc(&sys->cpu, sys->casread_ret);
    if (sys->tape_pos >= sys->tape_size) {
        /* reached end of tape, remove tape */
        cpc_remove_tape(sys);
    }
}

/*=== FLOPPY DISC SUPPORT ====================================================*/
static int _cpc_fdc_seektrack(int drive, int track, void* user_data) {
    if (0 == drive) {
        cpc_t* sys = (cpc_t*) user_data;
        return fdd_seek_track(&sys->fdd, track);
    }
    else {
        return UPD765_RESULT_NOT_READY;
    }
}

static int _cpc_fdc_seeksector(int drive, upd765_sectorinfo_t* inout_info, void* user_data) {
    if (0 == drive) {
        cpc_t* sys = (cpc_t*) user_data;
        const uint8_t c = inout_info->c;
        const uint8_t h = inout_info->h;
        const uint8_t r = inout_info->r;
        const uint8_t n = inout_info->n;
        int res = fdd_seek_sector(&sys->fdd, c, h, r, n);
        if (res == UPD765_RESULT_SUCCESS) {
            const fdd_sector_t* sector = &sys->fdd.disc.tracks[h][sys->fdd.cur_track_index].sectors[sys->fdd.cur_sector_index];
            inout_info->c = sector->info.upd765.c;
            inout_info->h = sector->info.upd765.h;
            inout_info->r = sector->info.upd765.r;
            inout_info->n = sector->info.upd765.n;
            inout_info->st1 = sector->info.upd765.st1;
            inout_info->st2 = sector->info.upd765.st2;
        }
        return res;
    }
    else {
        return UPD765_RESULT_NOT_READY;
    }
}

static int _cpc_fdc_read(int drive, uint8_t h, void* user_data, uint8_t* out_data) {
    if (0 == drive) {
        cpc_t* sys = (cpc_t*) user_data;
        return fdd_read(&sys->fdd, h, out_data);
    }
    else {
        return UPD765_RESULT_NOT_READY;
    }
}

static int _cpc_fdc_trackinfo(int drive, int side, void* user_data, upd765_sectorinfo_t* out_info) {
    CHIPS_ASSERT((side >= 0) && (side < 2));
    if (0 == drive) {
        cpc_t* sys = (cpc_t*) user_data;
        if (sys->fdd.has_disc && sys->fdd.motor_on) {
            // FIXME: this should be a fdd_ call
            out_info->physical_track = sys->fdd.cur_track_index;
            const fdd_sector_t* sector = &sys->fdd.disc.tracks[side][sys->fdd.cur_track_index].sectors[0];
            out_info->c = sector->info.upd765.c;
            out_info->h = sector->info.upd765.h;
            out_info->r = sector->info.upd765.r;
            out_info->n = sector->info.upd765.n;
            out_info->st1 = sector->info.upd765.st1;
            out_info->st2 = sector->info.upd765.st2;
            return FDD_RESULT_SUCCESS;
        }
    }
    return FDD_RESULT_NOT_READY;
}

static void _cpc_fdc_driveinfo(int drive, void* user_data, upd765_driveinfo_t* out_info) {
    cpc_t* sys = (cpc_t*) user_data;
    if ((0 == drive) && sys->fdd.has_disc) {
        out_info->physical_track = sys->fdd.cur_track_index;
        out_info->sides = sys->fdd.disc.num_sides;
        out_info->head = sys->fdd.cur_side;
        out_info->ready = sys->fdd.motor_on;
        out_info->write_protected = sys->fdd.disc.write_protected;
        out_info->fault = false;
    }
    else {
        out_info->physical_track = 0;
        out_info->sides = 1;
        out_info->head = 0;
        out_info->ready = false;
        out_info->write_protected = true;
        out_info->fault = false;
    }
}

bool cpc_insert_disc(cpc_t* sys, const uint8_t* ptr, int num_bytes) {
    return fdd_cpc_insert_dsk(&sys->fdd, ptr, num_bytes);
}

void cpc_remove_disc(cpc_t* sys) {
    fdd_eject_disc(&sys->fdd);
}

#endif /* CHIPS_IMPL */
