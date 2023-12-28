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

    - chips/chips_common.h
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
#include <stddef.h>
#include <stdalign.h>

#ifdef __cplusplus
extern "C" {
#endif

// bump when cpc_t memory layout changes
#define CPC_SNAPSHOT_VERSION (0x0001)

#define CPC_MAX_AUDIO_SAMPLES (1024)        // max number of audio samples in internal sample buffer
#define CPC_DEFAULT_AUDIO_SAMPLES (128)     // default number of samples in internal sample buffer
#define CPC_MAX_TAPE_SIZE (128*1024)        // max size of tape file in bytes

// CPC model types
typedef enum {
    CPC_TYPE_6128,          // default
    CPC_TYPE_464,
    CPC_TYPE_KCCOMPACT
} cpc_type_t;

// joystick types
typedef enum {
    CPC_JOYSTICK_NONE,
    CPC_JOYSTICK_DIGITAL,
    CPC_JOYSTICK_ANALOG,
} cpc_joystick_type_t;

// joystick mask bits
#define CPC_JOYSTICK_UP    (1<<0)
#define CPC_JOYSTICK_DOWN  (1<<1)
#define CPC_JOYSTICK_LEFT  (1<<2)
#define CPC_JOYSTICK_RIGHT (1<<3)
#define CPC_JOYSTICK_BTN0  (1<<4)
#define CPC_JOYSTICK_BTN1  (1<<4)

// configuration parameters for cpc_init()
typedef struct {
    cpc_type_t type;                // default is the CPC 6128
    cpc_joystick_type_t joystick_type;
    chips_debug_t debug;
    chips_audio_desc_t audio;

    // ROM images
    struct {
        // CPC 464
        struct {
            chips_range_t os;
            chips_range_t basic;
        } cpc464;
        // CPC 6128
        struct {
            chips_range_t os;
            chips_range_t basic;
            chips_range_t amsdos;
        } cpc6128;
        // KC Compact
        struct {
            chips_range_t os;
            chips_range_t basic;
        } kcc;
    } roms;
} cpc_desc_t;

// CPC emulator state
typedef struct {
    z80_t cpu;
    ay38910_t psg;
    mc6845_t crtc;
    i8255_t ppi;
    upd765_t fdc;
    am40010_t ga;

    cpc_type_t type;
    cpc_joystick_type_t joystick_type;
    uint8_t kbd_joymask;
    uint8_t joy_joymask;

    kbd_t kbd;
    mem_t mem;

    uint64_t pins;
    bool valid;
    chips_debug_t debug;

    struct {
        chips_audio_callback_t callback;
        int num_samples;
        int sample_pos;
        float sample_buffer[CPC_MAX_AUDIO_SAMPLES];
    } audio;
    uint8_t ram[8][0x4000];
    uint8_t rom_os[0x4000];
    uint8_t rom_basic[0x4000];
    uint8_t rom_amsdos[0x4000];
    alignas(64) uint8_t fb[AM40010_FRAMEBUFFER_SIZE_BYTES];
    fdd_t fdd;
} cpc_t;

// initialize a new CPC instance
void cpc_init(cpc_t* cpc, const cpc_desc_t* desc);
// discard a CPC instance
void cpc_discard(cpc_t* cpc);
// reset a CPC instance
void cpc_reset(cpc_t* cpc);
// get display requirements and framebuffer content, may be called with nullptr
chips_display_info_t cpc_display_info(cpc_t* cpc);
// run CPC instance for given amount of micro_seconds, returns number of ticks executed
uint32_t cpc_exec(cpc_t* cpc, uint32_t micro_seconds);
// send a key down event
void cpc_key_down(cpc_t* cpc, int key_code);
// send a key up event
void cpc_key_up(cpc_t* cpc, int key_code);
// enable/disable joystick emulation
void cpc_set_joystick_type(cpc_t* sys, cpc_joystick_type_t type);
// get current joystick emulation type
cpc_joystick_type_t cpc_joystick_type(cpc_t* sys);
// set joystick mask (combination of CPC_JOYSTICK_*)
void cpc_joystick(cpc_t* sys, uint8_t mask);
// get current joystick bitmask state
uint8_t cpc_joystick_mask(cpc_t* sys);
// load a snapshot file (.sna or .bin) into the emulator
bool cpc_quickload(cpc_t* cpc, chips_range_t data, bool start);
// return the exec address of a quickload file (.sna or .bin)
uint16_t cpc_quickload_exec_addr(chips_range_t data);
// return the return-address for a quickloaded file
uint16_t cpc_quickload_return_addr(void);
// insert a disk image file (.dsk)
bool cpc_insert_disc(cpc_t* cpc, chips_range_t data);
// remove current disc
void cpc_remove_disc(cpc_t* cpc);
// return true if a floppy disc is currently inserted
bool cpc_disc_inserted(cpc_t* cpc);
// if enabled, start calling the video-debugging-callback
void cpc_enable_video_debugging(cpc_t* cpc, bool enabled);
// get current display debug visualization enabled/disabled state
bool cpc_video_debugging_enabled(cpc_t* cpc);
// take a snapshot, patches any pointers to zero, returns snapshot version
uint32_t cpc_save_snapshot(cpc_t* sys, cpc_t* dst);
// load a snapshot, returns false if snapshot version doesn't match
bool cpc_load_snapshot(cpc_t* sys, uint32_t version, cpc_t* src);

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

#define _CPC_FREQUENCY (4000000)

static uint64_t _cpc_cclk(void* user_data);
static void _cpc_psg_out(int port_id, uint8_t data, void* user_data);
static uint8_t _cpc_psg_in(int port_id, void* user_data);
static void _cpc_init_keymap(cpc_t* sys);
static void _cpc_bankswitch(uint8_t ram_config, uint8_t rom_enable, uint8_t rom_select, void* user_data);
static int _cpc_fdc_seektrack(int drive, int track, void* user_data);
static int _cpc_fdc_seeksector(int drive, int side, upd765_sectorinfo_t* inout_info, void* user_data);
static int _cpc_fdc_read(int drive, int side, void* user_data, uint8_t* out_data);
static int _cpc_fdc_trackinfo(int drive, int side, void* user_data, upd765_sectorinfo_t* out_info);
static void _cpc_fdc_driveinfo(int drive, void* user_data, upd765_driveinfo_t* out_info);

#define _CPC_DEFAULT(val,def) (((val) != 0) ? (val) : (def))

void cpc_init(cpc_t* sys, const cpc_desc_t* desc) {
    CHIPS_ASSERT(sys && desc);
    if (desc->debug.callback.func) { CHIPS_ASSERT(desc->debug.stopped); }

    memset(sys, 0, sizeof(cpc_t));
    sys->valid = true;
    sys->debug = desc->debug;
    sys->type = desc->type;
    sys->joystick_type = desc->joystick_type;
    sys->audio.callback = desc->audio.callback;
    sys->audio.num_samples = _CPC_DEFAULT(desc->audio.num_samples, CPC_DEFAULT_AUDIO_SAMPLES);
    CHIPS_ASSERT(sys->audio.num_samples <= CPC_MAX_AUDIO_SAMPLES);
    if (CPC_TYPE_464 == desc->type) {
        CHIPS_ASSERT(desc->roms.cpc464.os.ptr && (desc->roms.cpc464.os.size == 0x4000));
        CHIPS_ASSERT(desc->roms.cpc464.basic.ptr && (desc->roms.cpc464.basic.size == 0x4000));
        memcpy(sys->rom_os, desc->roms.cpc464.os.ptr, 0x4000);
        memcpy(sys->rom_basic, desc->roms.cpc464.basic.ptr, 0x4000);
    } else if (CPC_TYPE_6128 == desc->type) {
        CHIPS_ASSERT(desc->roms.cpc6128.os.ptr && (desc->roms.cpc6128.os.size == 0x4000));
        CHIPS_ASSERT(desc->roms.cpc6128.basic.ptr && (desc->roms.cpc6128.basic.size == 0x4000));
        CHIPS_ASSERT(desc->roms.cpc6128.amsdos.ptr && (desc->roms.cpc6128.amsdos.size == 0x4000));
        memcpy(sys->rom_os, desc->roms.cpc6128.os.ptr, 0x4000);
        memcpy(sys->rom_basic, desc->roms.cpc6128.basic.ptr, 0x4000);
        memcpy(sys->rom_amsdos, desc->roms.cpc6128.amsdos.ptr, 0x4000);
    } else { // KC Compact
        CHIPS_ASSERT(desc->roms.kcc.os.ptr && (desc->roms.kcc.os.size == 0x4000));
        CHIPS_ASSERT(desc->roms.kcc.basic.ptr && (desc->roms.kcc.basic.size == 0x4000));
        memcpy(sys->rom_os, desc->roms.kcc.os.ptr, 0x4000);
        memcpy(sys->rom_basic, desc->roms.kcc.basic.ptr, 0x4000);
    }

    // initialize the hardware
    sys->pins = z80_init(&sys->cpu);
    i8255_init(&sys->ppi);
    ay38910_init(&sys->psg, &(ay38910_desc_t){
        .type = AY38910_TYPE_8912,
        .in_cb = _cpc_psg_in,
        .out_cb = _cpc_psg_out,
        .tick_hz = _CPC_FREQUENCY / 4,
        .sound_hz = _CPC_DEFAULT(desc->audio.sample_rate, 44100),
        .magnitude = _CPC_DEFAULT(desc->audio.volume, 0.5f),
        .user_data = sys
    });
    mc6845_init(&sys->crtc, MC6845_TYPE_UM6845R);
    mem_init(&sys->mem);
    am40010_init(&sys->ga, &(am40010_desc_t){
        .cpc_type = (am40010_cpc_type_t) sys->type,
        .bankswitch_cb = _cpc_bankswitch,
        .cclk_cb = _cpc_cclk,
        .ram = {
            .ptr = &sys->ram[0][0],
            .size = sizeof(sys->ram)
        },
        .framebuffer = {
            .ptr = &sys->fb[0],
            .size = sizeof(sys->fb),
        },
        .user_data = sys,
    });
    upd765_init(&sys->fdc, &(upd765_desc_t){
        .seektrack_cb = _cpc_fdc_seektrack,
        .seeksector_cb = _cpc_fdc_seeksector,
        .read_cb = _cpc_fdc_read,
        .trackinfo_cb = _cpc_fdc_trackinfo,
        .driveinfo_cb = _cpc_fdc_driveinfo,
        .user_data = sys,
    });
    fdd_init(&sys->fdd);

    _cpc_init_keymap(sys);
}

void cpc_discard(cpc_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    sys->valid = false;
}

void cpc_reset(cpc_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    mem_unmap_all(&sys->mem);
    mc6845_reset(&sys->crtc);
    ay38910_reset(&sys->psg);
    i8255_reset(&sys->ppi);
    am40010_reset(&sys->ga);
    sys->pins = z80_reset(&sys->cpu);
    sys->kbd_joymask = 0;
    sys->joy_joymask = 0;
}

static uint64_t _cpc_tick(cpc_t* sys, uint64_t cpu_pins) {
    cpu_pins = z80_tick(&sys->cpu, cpu_pins);

    // memory and IO requests
    if (cpu_pins & Z80_MREQ) {
        const uint16_t addr = Z80_GET_ADDR(cpu_pins);
        if (cpu_pins & Z80_RD) {
            Z80_SET_DATA(cpu_pins, mem_rd(&sys->mem, addr));
        } else if (cpu_pins & Z80_WR) {
            mem_wr(&sys->mem, addr, Z80_GET_DATA(cpu_pins));
        }
    } else if ((cpu_pins & (Z80_M1|Z80_IORQ)) == Z80_IORQ) {
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
            // i8255 in/out
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
                I8255_SET_PA(ppi_pins, AY38910_GET_DATA(ay_pins));
            }
            ppi_pins |= I8255_PB1|I8255_PB2|I8255_PB3;  // "Amstrad"
            ppi_pins |= I8255_PB4;  // PAL (50Hz)
            if (sys->crtc.vs) {
                ppi_pins |= I8255_PB0;   // VSYNC
            }
            ppi_pins = i8255_tick(&sys->ppi, ppi_pins);
            // copy data bus value to cpu pins
            if ((ppi_pins & (I8255_CS|I8255_RD)) == (I8255_CS|I8255_RD)) {
                Z80_SET_DATA(cpu_pins, I8255_GET_DATA(ppi_pins));
            }
            // update PSG state
            if ((ppi_pins & (I8255_PC7|I8255_PC6)) != 0) {
                uint64_t ay_pins = 0;
                if (ppi_pins & I8255_PC7) { ay_pins |= AY38910_BDIR; }
                if (ppi_pins & I8255_PC6) { ay_pins |= AY38910_BC1; }
                const uint8_t ay_data = I8255_GET_PA(ppi_pins);
                AY38910_SET_DATA(ay_pins, ay_data);
                ay38910_iorq(&sys->psg, ay_pins);
            }
            // PC0..PC3: select keyboard matrix line
            uint16_t col_mask = 1<<(I8255_GET_PC(ppi_pins) & 0x0F);
            kbd_set_active_columns(&sys->kbd, col_mask);
            // FIXME: cassette write data
            // FIXME: cassette deck motor control
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
            // 6845 in/out
            uint64_t crtc_pins = (cpu_pins & Z80_PIN_MASK)|MC6845_CS;
            if (cpu_pins & Z80_A9) { crtc_pins |= MC6845_RW; }
            if (cpu_pins & Z80_A8) { crtc_pins |= MC6845_RS; }
            cpu_pins = mc6845_iorq(&sys->crtc, crtc_pins) & Z80_PIN_MASK;
        }
        // Floppy Disk Interface
        if ((cpu_pins & (Z80_A10|Z80_A8|Z80_A7)) == 0) {
            if (cpu_pins & Z80_WR) {
                fdd_motor(&sys->fdd, 0 != (Z80_GET_DATA(cpu_pins) & 1));
            }
        } else if ((cpu_pins & (Z80_A10|Z80_A8|Z80_A7)) == Z80_A8) {
            // floppy controller status/data register
            uint64_t fdc_pins = UPD765_CS | (cpu_pins & Z80_PIN_MASK);
            cpu_pins = upd765_iorq(&sys->fdc, fdc_pins) & Z80_PIN_MASK;
        }
    }

    /* Tick the gate array, this will in turn tick the
       CRTC and PSG chips at the generated 1 MHz CCLK frequency
       (see _cpc_cclk callback). The returned CPU pin mask
       will have the WAIT and INT pin set as needed.
    */
    cpu_pins = am40010_tick(&sys->ga, cpu_pins) & Z80_PIN_MASK;
    return cpu_pins;
}

/* handle a 1 MHz CCLK tick generated by the gate array, this ticks the
   MC6845 CRTC and AY-3-8912 PSG, and must return the CRTC pins.
*/
static uint64_t _cpc_cclk(void* user_data) {
    cpc_t* sys = (cpc_t*) user_data;
    // tick the sound chip...
    if (ay38910_tick(&sys->psg)) {
        // new sound sample ready
        sys->audio.sample_buffer[sys->audio.sample_pos++] = sys->psg.sample;
        if (sys->audio.sample_pos == sys->audio.num_samples) {
            if (sys->audio.callback.func) {
                // new sample packet is ready
                sys->audio.callback.func(sys->audio.sample_buffer, sys->audio.num_samples, sys->audio.callback.user_data);
            }
            sys->audio.sample_pos = 0;
        }
    }
    // tick the CRTC and return its pin mask
    uint64_t crtc_pins = mc6845_tick(&sys->crtc);
    return crtc_pins;
}

// PSG OUT callback (nothing to do here)
static void _cpc_psg_out(int port_id, uint8_t data, void* user_data) {
    // this shouldn't be called
    (void)port_id;
    (void)data;
    (void)user_data;
}

// PSG IN callback (read keyboard matrix and joystick port)
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
    } else {
        // this shouldn't happen since the AY-3-8912 only has one IO port
        return 0xFF;
    }
}

// CPC6128 RAM block indices
static const int _cpc_ram_config[8][4] = {
    { 0, 1, 2, 3 },
    { 0, 1, 2, 7 },
    { 4, 5, 6, 7 },
    { 0, 3, 2, 7 },
    { 0, 4, 2, 3 },
    { 0, 5, 2, 3 },
    { 0, 6, 2, 3 },
    { 0, 7, 2, 3 }
};

// memory bankswitch callback, invoked by gate array (am40010)
static void _cpc_bankswitch(uint8_t ram_config, uint8_t rom_enable, uint8_t rom_select, void* user_data) {
    cpc_t* sys = (cpc_t*) user_data;
    int ram_config_index;
    const uint8_t* rom0_ptr;
    const uint8_t* rom1_ptr;
    if (CPC_TYPE_6128 == sys->type) {
        ram_config_index = ram_config & 7;
        rom0_ptr = sys->rom_os;
        rom1_ptr = (rom_select == 7) ? sys->rom_amsdos : sys->rom_basic;
    } else {
        ram_config_index = 0;
        rom0_ptr = sys->rom_os;
        rom1_ptr = sys->rom_basic;
    }
    const int i0 = _cpc_ram_config[ram_config_index][0];
    const int i1 = _cpc_ram_config[ram_config_index][1];
    const int i2 = _cpc_ram_config[ram_config_index][2];
    const int i3 = _cpc_ram_config[ram_config_index][3];

    // 0x0000 .. 0x3FFF
    if (rom_enable & AM40010_CONFIG_LROMEN) {
        // read/write RAM
        mem_map_ram(&sys->mem, 0, 0x0000, 0x4000, sys->ram[i0]);
    } else {
        // RAM-behind-ROM
        mem_map_rw(&sys->mem, 0, 0x0000, 0x4000, rom0_ptr, sys->ram[i0]);
    }
    // 0x4000 .. 0x7FFF
    mem_map_ram(&sys->mem, 0, 0x4000, 0x4000, sys->ram[i1]);
    // 0x8000 .. 0xBFFF
    mem_map_ram(&sys->mem, 0, 0x8000, 0x4000, sys->ram[i2]);
    // 0xC000 .. 0xFFFF
    if (rom_enable & AM40010_CONFIG_HROMEN) {
        // read/write RAM
        mem_map_ram(&sys->mem, 0, 0xC000, 0x4000, sys->ram[i3]);
    } else {
        // RAM-behind-ROM
        mem_map_rw(&sys->mem, 0, 0xC000, 0x4000, rom1_ptr, sys->ram[i3]);
    }
}

uint32_t cpc_exec(cpc_t* sys, uint32_t micro_seconds) {
    CHIPS_ASSERT(sys && sys->valid);
    const uint32_t num_ticks = clk_us_to_ticks(_CPC_FREQUENCY, micro_seconds);
    uint64_t pins = sys->pins;
    if (0 == sys->debug.callback.func) {
        // run without debug hook
        for (uint32_t tick = 0; tick < num_ticks; tick++) {
            pins = _cpc_tick(sys, pins);
        }
    } else {
        // run with debug hook
        for (uint32_t tick = 0; (tick < num_ticks) && !(*sys->debug.stopped); tick++) {
            pins = _cpc_tick(sys, pins);
            sys->debug.callback.func(sys->debug.callback.user_data, pins);
        }
    }
    sys->pins = pins;
    kbd_update(&sys->kbd, micro_seconds);
    return num_ticks;
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
    } else {
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
    } else {
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

uint8_t cpc_joystick_mask(cpc_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    return sys->kbd_joymask | sys->joy_joymask;
}

void cpc_enable_video_debugging(cpc_t* sys, bool enabled) {
    CHIPS_ASSERT(sys && sys->valid);
    sys->ga.dbg_vis = enabled;
}

bool cpc_video_debugging_enabled(cpc_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    return sys->ga.dbg_vis;
}

// keyboard matrix initialization
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
        // no shift
        "   ^08641 "
        "  [-97532 "
        "   @oure  "
        "  ]piytwq "
        "   ;lhgs  "
        "   :kjfda "
        "  \\/mnbc  "
        "   ., vxz "

        // shift
        "    _(&$! "
        "  {=)'%#\" "
        "   |OURE  "
        "  }PIYTWQ "
        "   +LHGS  "
        "   *KJFDA "
        "  `?MNBC  "
        "   >< VXZ ";
    // shift key is on column 2, line 5
    kbd_register_modifier(&sys->kbd, 0, 2, 5);
    // ctrl key is on column 2, line 7
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

    // special keys
    kbd_register_key(&sys->kbd, 0x20, 5, 7, 0); // space
    kbd_register_key(&sys->kbd, 0x08, 1, 0, 0); // cursor left
    kbd_register_key(&sys->kbd, 0x09, 0, 1, 0); // cursor right
    kbd_register_key(&sys->kbd, 0x0A, 0, 2, 0); // cursor down
    kbd_register_key(&sys->kbd, 0x0B, 0, 0, 0); // cursor up
    kbd_register_key(&sys->kbd, 0x01, 9, 7, 0); // delete
    kbd_register_key(&sys->kbd, 0x0C, 2, 0, 0); // clr
    kbd_register_key(&sys->kbd, 0x0D, 2, 2, 0); // return
    kbd_register_key(&sys->kbd, 0x03, 8, 2, 0); // escape
    kbd_register_key(&sys->kbd, 0xF1, 1, 5, 0); // F1...
    kbd_register_key(&sys->kbd, 0xF2, 1, 6, 0);
    kbd_register_key(&sys->kbd, 0xF3, 0, 5, 0);
    kbd_register_key(&sys->kbd, 0xF4, 4, 2, 0);
    kbd_register_key(&sys->kbd, 0xF5, 4, 1, 0);
    kbd_register_key(&sys->kbd, 0xF6, 4, 0, 0);
    kbd_register_key(&sys->kbd, 0xF7, 2, 1, 0);
    kbd_register_key(&sys->kbd, 0xF8, 3, 1, 0);
    kbd_register_key(&sys->kbd, 0xF9, 3, 0, 0);
    kbd_register_key(&sys->kbd, 0xFA, 7, 1, 0); // F0 -> F10
}

/*=== SNAPSHOT FILE LOADING ==================================================*/

// CPC SNA fileformat header: http://cpctech.cpc-live.com/docs/snapshot.html
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

static bool _cpc_is_valid_sna(chips_range_t data) {
    if (data.size <= sizeof(_cpc_sna_header)) {
        return false;
    }
    const _cpc_sna_header* hdr = (const _cpc_sna_header*)data.ptr;
    static uint8_t magic[8] = { 'M', 'V', 0x20, '-', 0x20, 'S', 'N', 'A' };
    for (size_t i = 0; i < 8; i++) {
        if (magic[i] != hdr->magic[i]) {
            return false;
        }
    }
    // FIXME: check version field?
    return true;
}

static bool _cpc_load_sna(cpc_t* sys, chips_range_t data) {
    const uint8_t* ptr = (uint8_t*) data.ptr;
    const _cpc_sna_header* hdr = (const _cpc_sna_header*) ptr;
    ptr += sizeof(_cpc_sna_header);

    // copy 64 or 128 KByte memory dump
    const uint16_t dump_size = hdr->dump_size_h<<8 | hdr->dump_size_l;
    const uint32_t dump_num_bytes = (dump_size == 64) ? 0x10000 : 0x20000;
    if (data.size > (sizeof(_cpc_sna_header) + dump_num_bytes)) {
        return false;
    }
    if (dump_num_bytes > sizeof(sys->ram)) {
        return false;
    }
    memcpy(sys->ram, ptr, dump_num_bytes);

    z80_reset(&sys->cpu);
    sys->cpu.f = hdr->F; sys->cpu.a = hdr->A;
    sys->cpu.c = hdr->C; sys->cpu.b = hdr->B;
    sys->cpu.e = hdr->E; sys->cpu.d = hdr->D;
    sys->cpu.l = hdr->L; sys->cpu.h = hdr->H;
    sys->cpu.r = hdr->R; sys->cpu.i = hdr->I;
    sys->cpu.iff1 = (hdr->IFF1 & 1) != 0;
    sys->cpu.iff2 = (hdr->IFF2 & 1) != 0;
    sys->cpu.ix = (hdr->IX_h<<8) | hdr->IX_l;
    sys->cpu.iy = (hdr->IY_h<<8) | hdr->IY_l;
    sys->cpu.sp = (hdr->SP_h<<8) | hdr->SP_l;
    sys->cpu.pc = (hdr->PC_h<<8) | hdr->PC_l;
    sys->cpu.im = hdr->IM;
    sys->cpu.af2 = (hdr->A_<<8) | hdr->F_;
    sys->cpu.bc2 = (hdr->B_<<8) | hdr->C_;
    sys->cpu.de2 = (hdr->D_<<8) | hdr->E_;
    sys->cpu.hl2 = (hdr->H_<<8) | hdr->L_;

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
        ay38910_set_register(&sys->psg, i, hdr->psg_regs[i]);
    }
    ay38910_set_addr_latch(&sys->psg, hdr->psg_selected);
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

static bool _cpc_is_valid_bin(chips_range_t data) {
    if (data.size <= sizeof(_cpc_bin_header)) {
        return false;
    }
    return true;
}

static bool _cpc_load_bin(cpc_t* sys, chips_range_t data, bool start) {
    const uint8_t* ptr = (uint8_t*) data.ptr;
    const _cpc_bin_header* hdr = (const _cpc_bin_header*) ptr;
    ptr += sizeof(_cpc_bin_header);
    const uint16_t load_addr = (hdr->load_addr_h<<8)|hdr->load_addr_l;
    const uint16_t start_addr = (hdr->start_addr_h<<8)|hdr->start_addr_l;
    const uint16_t len = (hdr->length_h<<8)|hdr->length_l;
    for (uint16_t i = 0; i < len; i++) {
        mem_wr(&sys->mem, load_addr+i, *ptr++);
    }
    if (start) {
        // FIXME: alternatively via CALL xxxx?
        sys->cpu.iff1 = true;
        sys->cpu.iff2 = true;
        sys->cpu.c = 0; // FIXME: "ROM select number"
        sys->cpu.hl = start_addr;
        sys->pins = z80_prefetch(&sys->cpu, 0xBD16); // MC START PROGRAM
    }
    return true;
}

bool cpc_quickload(cpc_t* sys, chips_range_t data, bool start) {
    CHIPS_ASSERT(sys && sys->valid && data.ptr && (data.size > 0));
    if (_cpc_is_valid_sna(data)) {
        return _cpc_load_sna(sys, data);
    } else if (_cpc_is_valid_bin(data)) {
        return _cpc_load_bin(sys, data, start);
    } else {
        // not a known file type, or not enough data
        return false;
    }
}

uint16_t cpc_quickload_return_addr(void) {
    // FIXME!
    return 0xFFFF;
}

uint16_t cpc_quickload_exec_addr(chips_range_t data) {
    uint16_t start_addr = 0xFFFF;
    if (_cpc_is_valid_sna(data)) {
        const _cpc_sna_header* hdr = (const _cpc_sna_header*)data.ptr;
        start_addr = (hdr->PC_h<<8) | hdr->PC_l;
    } else if (_cpc_is_valid_bin(data)) {
        const _cpc_bin_header* hdr = (const _cpc_bin_header*)data.ptr;
        start_addr = (hdr->start_addr_h<<8)|hdr->start_addr_l;
    }
    return start_addr;
}

/*=== FLOPPY DISC SUPPORT ====================================================*/
static int _cpc_fdc_seektrack(int drive, int track, void* user_data) {
    if (0 == drive) {
        cpc_t* sys = (cpc_t*) user_data;
        return fdd_seek_track(&sys->fdd, track);
    } else {
        return UPD765_RESULT_NOT_READY;
    }
}

static int _cpc_fdc_seeksector(int drive, int side, upd765_sectorinfo_t* inout_info, void* user_data) {
    if (0 == drive) {
        cpc_t* sys = (cpc_t*) user_data;
        const uint8_t c = inout_info->c;
        const uint8_t h = inout_info->h;
        const uint8_t r = inout_info->r;
        const uint8_t n = inout_info->n;
        int res = fdd_seek_sector(&sys->fdd, side, c, h, r, n);
        if (res == UPD765_RESULT_SUCCESS) {
            const fdd_sector_t* sector = &sys->fdd.disc.tracks[side][sys->fdd.cur_track_index].sectors[sys->fdd.cur_sector_index];
            inout_info->c = sector->info.upd765.c;
            inout_info->h = sector->info.upd765.h;
            inout_info->r = sector->info.upd765.r;
            inout_info->n = sector->info.upd765.n;
            inout_info->st1 = sector->info.upd765.st1;
            inout_info->st2 = sector->info.upd765.st2;
        }
        return res;
    } else {
        return UPD765_RESULT_NOT_READY;
    }
}

static int _cpc_fdc_read(int drive, int side, void* user_data, uint8_t* out_data) {
    if (0 == drive) {
        cpc_t* sys = (cpc_t*) user_data;
        return fdd_read(&sys->fdd, side, out_data);
    } else {
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
    } else {
        out_info->physical_track = 0;
        out_info->sides = 1;
        out_info->head = 0;
        out_info->ready = false;
        out_info->write_protected = true;
        out_info->fault = false;
    }
}

bool cpc_insert_disc(cpc_t* sys, chips_range_t data) {
    CHIPS_ASSERT(sys && sys->valid);
    return fdd_cpc_insert_dsk(&sys->fdd, data);
}

void cpc_remove_disc(cpc_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    fdd_eject_disc(&sys->fdd);
}

bool cpc_disc_inserted(cpc_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    return fdd_disc_inserted(&sys->fdd);
}

chips_display_info_t cpc_display_info(cpc_t* sys) {
    const chips_display_info_t res = {
        .frame = {
            .dim = {
                .width = AM40010_FRAMEBUFFER_WIDTH,
                .height = AM40010_FRAMEBUFFER_HEIGHT,
            },
            .bytes_per_pixel = 1,
            .buffer = {
                .ptr = sys ? sys->fb : 0,
                .size = AM40010_FRAMEBUFFER_SIZE_BYTES,
            }
        },
        .screen = {
            .x = 0,
            .y = 0,
            .width = (sys && sys->ga.dbg_vis) ? AM40010_FRAMEBUFFER_WIDTH : AM40010_DISPLAY_WIDTH,
            .height = (sys && sys->ga.dbg_vis) ? AM40010_FRAMEBUFFER_HEIGHT : AM40010_DISPLAY_HEIGHT,
        },
        .palette = {
            .ptr = sys ? sys->ga.hw_colors : 0,
            .size = AM40010_NUM_HWCOLORS * sizeof(uint32_t)
        }
    };
    CHIPS_ASSERT(((sys == 0) && (res.frame.buffer.ptr == 0)) || ((sys != 0) && (res.frame.buffer.ptr != 0)));
    CHIPS_ASSERT(((sys == 0) && (res.palette.ptr == 0)) || ((sys != 0) && (res.palette.ptr != 0)));
    return res;
}

uint32_t cpc_save_snapshot(cpc_t* sys, cpc_t* dst) {
    CHIPS_ASSERT(sys && dst);
    *dst = *sys;
    chips_debug_snapshot_onsave(&dst->debug);
    chips_audio_callback_snapshot_onsave(&dst->audio.callback);
    ay38910_snapshot_onsave(&dst->psg);
    upd765_snapshot_onsave(&dst->fdc);
    am40010_snapshot_onsave(&dst->ga);
    mem_snapshot_onsave(&dst->mem, sys);
    return CPC_SNAPSHOT_VERSION;
}

bool cpc_load_snapshot(cpc_t* sys, uint32_t version, cpc_t* src) {
    CHIPS_ASSERT(sys && src);
    if (version != CPC_SNAPSHOT_VERSION) {
        return false;
    }
    static cpc_t im;
    im = *src;
    chips_debug_snapshot_onload(&im.debug, &sys->debug);
    chips_audio_callback_snapshot_onload(&im.audio.callback, &sys->audio.callback);
    ay38910_snapshot_onload(&im.psg, &sys->psg);
    upd765_snapshot_onload(&im.fdc, &sys->fdc);
    am40010_snapshot_onload(&im.ga, &sys->ga);
    mem_snapshot_onload(&im.mem, sys);
    *sys = im;
    return true;
}

#endif /* CHIPS_IMPL */
