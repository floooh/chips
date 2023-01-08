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

    You need to include the following headers before including vic20.h:

    - chips/chips_common.h
    - chips/m6502.h
    - chips/m6522.h
    - chips/m6561.h
    - chips/kbd.h
    - chips/mem.h
    - chips/clk.h
    - systems/c1530.h

    ## The Commodore VIC-20


    TODO!

    ## Links

    http://blog.tynemouthsoftware.co.uk/2019/09/how-the-vic20-works.html

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

#ifdef __cplusplus
extern "C" {
#endif

// bump snapshot version when vic20_t memory layout changes
#define VIC20_SNAPSHOT_VERSION (1)

#define VIC20_FREQUENCY (1108404)
#define VIC20_MAX_AUDIO_SAMPLES (1024)        // max number of audio samples in internal sample buffer
#define VIC20_DEFAULT_AUDIO_SAMPLES (128)     // default number of samples in internal sample buffer

// VIC-20 joystick types (only one joystick supported)
typedef enum {
    VIC20_JOYSTICKTYPE_NONE,
    VIC20_JOYSTICKTYPE_DIGITAL,
} vic20_joystick_type_t;

// memory configuration (used in vic20_desc_t.mem_config)
typedef enum {
    VIC20_MEMCONFIG_STANDARD,       // unexpanded
    VIC20_MEMCONFIG_8K,             // Block 1
    VIC20_MEMCONFIG_16K,            // Block 1+2
    VIC20_MEMCONFIG_24K,            // Block 1+2+3
    VIC20_MEMCONFIG_32K,            // Block 1+2+3+5 (note that BASIC can only use blocks 1+2+3)
    VIC20_MEMCONFIG_MAX             // 32K + 3KB at 0400..0FFF
} vic20_memory_config_t;

// joystick mask bits
#define VIC20_JOYSTICK_UP    (1<<0)
#define VIC20_JOYSTICK_DOWN  (1<<1)
#define VIC20_JOYSTICK_LEFT  (1<<2)
#define VIC20_JOYSTICK_RIGHT (1<<3)
#define VIC20_JOYSTICK_BTN   (1<<4)

// casette port bits, same as C1530_CASPORT_*
#define VIC20_CASPORT_MOTOR   (1<<0)
#define VIC20_CASPORT_READ    (1<<1)
#define VIC20_CASPORT_WRITE   (1<<2)
#define VIC20_CASPORT_SENSE   (1<<3)

// IEC port bits, same as C1541_IECPORT_*
#define VIC20_IECPORT_RESET   (1<<0)
#define VIC20_IECPORT_SRQIN   (1<<1)
#define VIC20_IECPORT_DATA    (1<<2)
#define VIC20_IECPORT_CLK     (1<<3)
#define VIC20_IECPORT_ATN     (1<<4)

// config parameters for vic20_init()
typedef struct {
    bool c1530_enabled;             // set to true to enable C1530 datassette emulation
    vic20_joystick_type_t joystick_type;    // default is VIC20_JOYSTICK_NONE
    vic20_memory_config_t mem_config;       // default is VIC20_MEMCONFIG_STANDARD
    chips_debug_t debug;            // optional debugging hook
    chips_audio_desc_t audio;
    struct {
        chips_range_t chars;    // 4 KByte character ROM dump
        chips_range_t basic;    // 8 KByte BASIC dump
        chips_range_t kernal;   // 8 KByte KERNAL dump
    } roms;
} vic20_desc_t;

// VIC-20 emulator state
typedef struct {
    m6502_t cpu;
    m6522_t via_1;
    m6522_t via_2;
    m6561_t vic;
    uint64_t pins;

    vic20_joystick_type_t joystick_type;
    vic20_memory_config_t mem_config;
    uint8_t cas_port;           // cassette port, shared with c1530_t if datasette is connected
    uint8_t iec_port;           // IEC serial port, shared with c1541_t if connected
    uint8_t kbd_joy_mask;       // current joystick state from keyboard-joystick emulation
    uint8_t joy_joy_mask;       // current joystick state from vic20_joystick()
    uint64_t via1_joy_mask;     // merged keyboard/joystick mask ready for or-ing with VIA1 input pins
    uint64_t via2_joy_mask;     // merged keyboard/joystick mask ready for or-ing with VIA2 input pins

    kbd_t kbd;                  // keyboard matrix state
    mem_t mem_cpu;              // CPU-visible memory mapping
    mem_t mem_vic;              // VIC-visible memory mapping
    bool valid;
    chips_debug_t debug;

    struct {
        chips_audio_callback_t callback;
        int num_samples;
        int sample_pos;
        float sample_buffer[VIC20_MAX_AUDIO_SAMPLES];
    } audio;

    uint8_t color_ram[0x0400];      // special color RAM
    uint8_t ram0[0x0400];           // 1 KB zero page, stack, system work area
    uint8_t ram_3k[0x0C00];         // optional 3K exp RAM
    uint8_t ram1[0x1000];           // 4 KB main RAM
    uint8_t rom_char[0x1000];       // 4 KB character ROM image
    uint8_t rom_basic[0x2000];      // 8 KB BASIC ROM image
    uint8_t rom_kernal[0x2000];     // 8 KB KERNAL V3 ROM image
    uint8_t ram_exp[4][0x2000];     // optional expansion 8K RAM blocks
    uint8_t fb[M6561_FRAMEBUFFER_SIZE_BYTES];

    c1530_t c1530;                  // c1530.valid = true if enabled

    mem_t mem_cart;                 // special ROM cartridge memory mapping helper
} vic20_t;

// initialize a new VIC-20 instance
void vic20_init(vic20_t* sys, const vic20_desc_t* desc);
// discard VIC-20 instance
void vic20_discard(vic20_t* sys);
// reset a VIC-20 instance
void vic20_reset(vic20_t* sys);
// query display information
chips_display_info_t vic20_display_info(vic20_t* sys);
// tick VIC-20 instance for a given number of microseconds, return number of executed ticks
uint32_t vic20_exec(vic20_t* sys, uint32_t micro_seconds);
// send a key-down event to the VIC-20
void vic20_key_down(vic20_t* sys, int key_code);
// send a key-up event to the VIC-20
void vic20_key_up(vic20_t* sys, int key_code);
// enable/disable joystick emulation
void vic20_set_joystick_type(vic20_t* sys, vic20_joystick_type_t type);
// get current joystick emulation type
vic20_joystick_type_t vic20_joystick_type(vic20_t* sys);
// set joystick mask (combination of VIC20_JOYSTICK_*)
void vic20_joystick(vic20_t* sys, uint8_t joy_mask);
// quickload a .prg/.bin file
bool vic20_quickload(vic20_t* sys, chips_range_t data);
// load a .prg/.bin file as ROM cartridge
bool vic20_insert_rom_cartridge(vic20_t* sys, chips_range_t data);
// remove current ROM cartridge
void vic20_remove_rom_cartridge(vic20_t* sys);
// insert tape as .TAP file (c1530 must be enabled)
bool vic20_insert_tape(vic20_t* sys, chips_range_t data);
// remove tape file
void vic20_remove_tape(vic20_t* sys);
// return true if a tape is currently inserted
bool vic20_tape_inserted(vic20_t* sys);
// start the tape (press the Play button)
void vic20_tape_play(vic20_t* sys);
// stop the tape (unpress the Play button
void vic20_tape_stop(vic20_t* sys);
// return true if tape motor is on
bool vic20_is_tape_motor_on(vic20_t* sys);
// take a snapshot, patches pointers to zero or offsets, returns snapshot version
uint32_t vic20_save_snapshot(vic20_t* sys, vic20_t* dst);
// load a snapshot, returns false if snapshot version doesn't match
bool vic20_load_snapshot(vic20_t* sys, uint32_t version, vic20_t* src);

#ifdef __cplusplus
} // extern "C"
#endif

/*-- IMPLEMENTATION ----------------------------------------------------------*/
#ifdef CHIPS_IMPL
#include <string.h> /* memcpy, memset */
#ifndef CHIPS_ASSERT
    #include <assert.h>
    #define CHIPS_ASSERT(c) assert(c)
#endif

#define _VIC20_SCREEN_WIDTH (232) // actually 229, but rounded up to 8x
#define _VIC20_SCREEN_HEIGHT (272)
#define _VIC20_SCREEN_X (32)
#define _VIC20_SCREEN_Y (8)

static uint16_t _vic20_vic_fetch(uint16_t addr, void* user_data);
static void _vic20_init_key_map(vic20_t* sys);

#define _VIC20_DEFAULT(val,def) (((val) != 0) ? (val) : (def))

void vic20_init(vic20_t* sys, const vic20_desc_t* desc) {
    CHIPS_ASSERT(sys && desc);
    if (desc->debug.callback.func) { CHIPS_ASSERT(desc->debug.stopped); }

    memset(sys, 0, sizeof(vic20_t));
    sys->valid = true;
    sys->joystick_type = desc->joystick_type;
    sys->mem_config = desc->mem_config;
    sys->via1_joy_mask = M6522_PA2|M6522_PA3|M6522_PA4|M6522_PA5;
    sys->via2_joy_mask = M6522_PB7;
    sys->debug = desc->debug;
    sys->audio.callback = desc->audio.callback;
    sys->audio.num_samples = _VIC20_DEFAULT(desc->audio.num_samples, VIC20_DEFAULT_AUDIO_SAMPLES);
    CHIPS_ASSERT(sys->audio.num_samples <= VIC20_MAX_AUDIO_SAMPLES);
    CHIPS_ASSERT(desc->roms.chars.ptr && (desc->roms.chars.size == sizeof(sys->rom_char)));
    CHIPS_ASSERT(desc->roms.basic.ptr && (desc->roms.basic.size == sizeof(sys->rom_basic)));
    CHIPS_ASSERT(desc->roms.kernal.ptr && (desc->roms.kernal.size == sizeof(sys->rom_kernal)));
    memcpy(sys->rom_char, desc->roms.chars.ptr, sizeof(sys->rom_char));
    memcpy(sys->rom_basic, desc->roms.basic.ptr, sizeof(sys->rom_basic));
    memcpy(sys->rom_kernal, desc->roms.kernal.ptr, sizeof(sys->rom_kernal));

    // datasette: motor off, no buttons pressed
    sys->cas_port = VIC20_CASPORT_MOTOR|VIC20_CASPORT_SENSE;

    sys->pins = m6502_init(&sys->cpu, &(m6502_desc_t){0});
    m6522_init(&sys->via_1);
    m6522_init(&sys->via_2);
    m6561_init(&sys->vic, &(m6561_desc_t){
        .fetch_cb = _vic20_vic_fetch,
        .framebuffer = {
            .ptr = sys->fb,
            .size = sizeof(sys->fb)
        },
        .screen = {
            .x = _VIC20_SCREEN_X,
            .y = _VIC20_SCREEN_Y,
            .width = _VIC20_SCREEN_WIDTH,
            .height = _VIC20_SCREEN_HEIGHT,
        },
        .user_data = sys,
        .tick_hz = VIC20_FREQUENCY,
        .sound_hz = _VIC20_DEFAULT(desc->audio.sample_rate, 44100),
        .sound_magnitude = _VIC20_DEFAULT(desc->audio.volume, 1.0f),
    });
    _vic20_init_key_map(sys);

    /*
        VIC-20 CPU memory map:

        0000..03FF      zero-page, stack, system area
        [0400..0FFF]    3 KB Expansion RAM
        1000..1FFF      4 KB Main RAM (block 0)
        [2000..3FFF]    8 KB Expansion Block 1
        [4000..5FFF]    8 KB Expansion Block 2
        [6000..7FFF]    8 KB Expansion Block 3
        8000..8FFF      4 KB Character ROM
        9000..900F      VIC Registers
        9110..911F      VIA #1 Registers
        9120..912F      VIA #2 Registers
        9400..97FF      1Kx4 bit color ram (either at 9600 or 9400)
        [9800..9BFF]    1 KB I/O Expansion 2
        [9C00..9FFF]    1 KB I/O Expansion 3
        [A000..BFFF]    8 KB Expansion Block 5 (usually ROM cartridges)
        C000..DFFF      8 KB BASIC ROM
        E000..FFFF      8 KB KERNAL ROM

        NOTE: use mem layer 1 for the standard RAM/ROM, so that
        the higher-priority layer 0 can be used for ROM cartridges
    */
    mem_init(&sys->mem_cpu);
    mem_map_ram(&sys->mem_cpu, 1, 0x0000, 0x0400, sys->ram0);
    if (desc->mem_config == VIC20_MEMCONFIG_MAX) {
        mem_map_ram(&sys->mem_cpu, 1, 0x0400, 0x0C00, sys->ram_3k);
    }
    mem_map_ram(&sys->mem_cpu, 1, 0x1000, 0x1000, sys->ram1);
    if (desc->mem_config >= VIC20_MEMCONFIG_8K) {
        mem_map_ram(&sys->mem_cpu, 1, 0x2000, 0x2000, sys->ram_exp[0]);
    }
    if (desc->mem_config >= VIC20_MEMCONFIG_16K) {
        mem_map_ram(&sys->mem_cpu, 1, 0x4000, 0x2000, sys->ram_exp[1]);
    }
    if (desc->mem_config >= VIC20_MEMCONFIG_24K) {
        mem_map_ram(&sys->mem_cpu, 1, 0x6000, 0x2000, sys->ram_exp[2]);
    }
    mem_map_rom(&sys->mem_cpu, 1, 0x8000, 0x1000, sys->rom_char);
    mem_map_ram(&sys->mem_cpu, 1, 0x9400, 0x0400, sys->color_ram);
    if (desc->mem_config >= VIC20_MEMCONFIG_32K) {
        mem_map_ram(&sys->mem_cpu, 1, 0xA000, 0x2000, sys->ram_exp[3]);
    }
    mem_map_rom(&sys->mem_cpu, 1, 0xC000, 0x2000, sys->rom_basic);
    mem_map_rom(&sys->mem_cpu, 1, 0xE000, 0x2000, sys->rom_kernal);

    /*
        VIC-I memory map:

        The VIC-I has 14 address bus bits VA0..VA13, for 16 KB of
        addressable memory. Bits VA0..VA12 are identical with the
        lower 13 CPU address bus pins, VA13 is the inverted BLK4
        address decoding bit.
    */
    mem_init(&sys->mem_vic);
    mem_map_rom(&sys->mem_vic, 0, 0x0000, 0x1000, sys->rom_char);       // CPU: 8000..8FFF
    // FIXME: can the VIC read the color RAM as data?
    //mem_map_rom(&sys->mem_vic, 0, 0x1400, 0x0400, sys->color_ram);      // CPU: 9400..97FF
    mem_map_rom(&sys->mem_vic, 0, 0x2000, 0x0400, sys->ram0);           // CPU: 0000..03FF
    if (desc->mem_config == VIC20_MEMCONFIG_MAX) {
        mem_map_rom(&sys->mem_vic, 0, 0x2400, 0x0C00, sys->ram_3k);     // CPU: 0400..0FFF
    }
    mem_map_rom(&sys->mem_vic, 0, 0x3000, 0x1000, sys->ram1);           // CPU: 1000..1FFF

    /*
        A special memory mapping used to copy ROM cartridge PRG files
        into the VIC-20. Those PRG files may be merged from several files
        and have gaps in them. The data in those gaps must not
        scribble over the VIC-20's RAM, ROM or IO regions.
    */
    mem_init(&sys->mem_cart);
    mem_map_ram(&sys->mem_cart, 0, 0x2000, 0x2000, sys->ram_exp[0]);
    mem_map_ram(&sys->mem_cart, 0, 0x4000, 0x2000, sys->ram_exp[1]);
    mem_map_ram(&sys->mem_cart, 0, 0x6000, 0x2000, sys->ram_exp[2]);
    mem_map_ram(&sys->mem_cart, 0, 0xA000, 0x2000, sys->ram_exp[3]);

    // optionally setup C1530 datasette drive
    if (desc->c1530_enabled) {
        c1530_init(&sys->c1530, &(c1530_desc_t){
            .cas_port = &sys->cas_port,
        });
    }
}

void vic20_discard(vic20_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    if (sys->c1530.valid) {
        c1530_discard(&sys->c1530);
    }
    sys->valid = false;
}

void vic20_reset(vic20_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    sys->kbd_joy_mask = 0;
    sys->joy_joy_mask = 0;
    sys->via1_joy_mask = M6522_PA2|M6522_PA3|M6522_PA4|M6522_PA5;
    sys->via2_joy_mask = M6522_PB7;
    sys->pins |= M6502_RES;
    sys->cas_port = VIC20_CASPORT_MOTOR|VIC20_CASPORT_SENSE;
    m6522_reset(&sys->via_1);
    m6522_reset(&sys->via_2);
    m6561_reset(&sys->vic);
    if (sys->c1530.valid) {
        c1530_reset(&sys->c1530);
    }
}

static uint64_t _vic20_tick(vic20_t* sys, uint64_t pins) {

    // tick the CPU
    pins = m6502_tick(&sys->cpu, pins);

    // the IRQ and NMI pins will be set by the VIAs each tick
    pins &= ~(M6502_IRQ|M6502_NMI);

    // VIC+VIAs address decoding and memory access
    uint64_t vic_pins  = pins & M6502_PIN_MASK;
    uint64_t via1_pins = pins & M6502_PIN_MASK;
    uint64_t via2_pins = pins & M6502_PIN_MASK;
    if ((pins & 0xFC00) == 0x9000) {
        /* 9000..93FF: VIA and VIC IO area

            A4 high:    VIA-1
            A5 high:    VIA-2

            The VIC has no chip-select pin instead it's directly snooping
            the address bus for a specific pin state:

            A8,9,10,11,13 low, A12 high

            (FIXME: why is the VIC only access on address 9000 (A15=1, A14=0),
            and not on 5000 (A15=0, A14=1) and D000 (A15=1, A14=1)???)

            NOTE: Unlike a real VIC, the VIC emulation has a traditional
            chip-select pin.
        */
        if (M6561_SELECTED_ADDR(pins)) {
            vic_pins |= M6561_CS;
        }
        if (pins & M6502_A4) {
            via1_pins |= M6522_CS1;
        }
        if (pins & M6502_A5) {
            via2_pins |= M6522_CS1;
        }
    }
    else {
        // regular memory access
        const uint16_t addr = M6502_GET_ADDR(pins);
        if (pins & M6502_RW) {
            M6502_SET_DATA(pins, mem_rd(&sys->mem_cpu, addr));
        }
        else {
            mem_wr(&sys->mem_cpu, addr, M6502_GET_DATA(pins));
        }
    }

    /* tick VIA1

        VIA1 IRQ pin is connected to CPU NMI pin

        VIA1 Port A input:
            PA0:    Serial CLK (FIXME)
            PA1:    Serial DATA (FIXME)
            PA2:    in: JOY0 (up)
            PA3:    in: JOY1 (down)
            PA4:    in: JOY2 (left)
            PA5:    in: PEN/BTN (fire)
            PA6:    in: CASS SENSE
            PA7:    SERIAL ATN OUT (???)

            CA1:    in: RESTORE KEY(???)
            CA2:    out: CASS MOTOR

        VIA1 Port B input:
            all connected to USR port

        NOTE: the IRQ/NMI mapping is reversed from the C64
    */
    {
        // FIXME: SERIAL PORT
        // FIXME: RESTORE key to M6522_CA1
        via1_pins |= sys->via1_joy_mask | (M6522_PA0|M6522_PA1|M6522_PA7);
        if (sys->cas_port & VIC20_CASPORT_SENSE) {
            via1_pins |= M6522_PA6;
        }
        via1_pins = m6522_tick(&sys->via_1, via1_pins);
        if (via1_pins & M6522_CA2) {
            sys->cas_port |= VIC20_CASPORT_MOTOR;
        }
        else {
            sys->cas_port &= ~VIC20_CASPORT_MOTOR;
        }
        if (via1_pins & M6522_IRQ) {
            pins |= M6502_NMI;
        }
        if ((via1_pins & (M6522_CS1|M6522_RW)) == (M6522_CS1|M6522_RW)) {
            pins = M6502_COPY_DATA(pins, via1_pins);
        }
    }

    /* tick VIA2

        VIA2 IRQ pin is connected to CPU IRQ pin

        VIA2 Port A input:
            read keyboard matrix rows

            CA1: in: CASS READ

        VIA2 Port B input:
            PB7:    JOY3 (Right)

        VIA2 Port A output:
            ---(???)

        VIA2 Port B output:
            write keyboard matrix columns
            PB3 -> CASS WRITE (not implemented)
    */
    {
        uint8_t kbd_lines = ~kbd_scan_lines(&sys->kbd);
        M6522_SET_PA(via2_pins, kbd_lines);
        via2_pins |= sys->via2_joy_mask;
        if (sys->cas_port & VIC20_CASPORT_READ) {
            via2_pins |= M6522_CA1;
        }
        via2_pins = m6522_tick(&sys->via_2, via2_pins);
        uint8_t kbd_cols = ~M6522_GET_PB(via2_pins);
        kbd_set_active_columns(&sys->kbd, kbd_cols);
        if (via2_pins & M6522_IRQ) {
            pins |= M6502_IRQ;
        }
        if ((via2_pins & (M6522_CS1|M6522_RW)) == (M6522_CS1|M6522_RW)) {
            pins = M6502_COPY_DATA(pins, via2_pins);
        }
    }

    // tick the VIC
    {
        vic_pins = m6561_tick(&sys->vic, vic_pins);
        if ((vic_pins & (M6561_CS|M6561_RW)) == (M6561_CS|M6561_RW)) {
            pins = M6502_COPY_DATA(pins, vic_pins);
        }
        if (vic_pins & M6561_SAMPLE) {
            sys->audio.sample_buffer[sys->audio.sample_pos++] = sys->vic.sound.sample;
            if (sys->audio.sample_pos == sys->audio.num_samples) {
                if (sys->audio.callback.func) {
                    sys->audio.callback.func(sys->audio.sample_buffer, sys->audio.num_samples, sys->audio.callback.user_data);
                }
                sys->audio.sample_pos = 0;
            }
        }
    }

    // optionally tick the C1530 datassette
    if (sys->c1530.valid) {
        c1530_tick(&sys->c1530);
    }
    return pins;
}

uint32_t vic20_exec(vic20_t* sys, uint32_t micro_seconds) {
    CHIPS_ASSERT(sys && sys->valid);
    uint32_t num_ticks = clk_us_to_ticks(VIC20_FREQUENCY, micro_seconds);
    uint64_t pins = sys->pins;
    if (0 == sys->debug.callback.func) {
        // run without debug callback
        for (uint32_t ticks = 0; ticks < num_ticks; ticks++) {
            pins = _vic20_tick(sys, pins);
        }
    }
    else {
        // run with debug callback
        for (uint32_t ticks = 0; (ticks < num_ticks) && !(*sys->debug.stopped); ticks++) {
            pins = _vic20_tick(sys, pins);
            sys->debug.callback.func(sys->debug.callback.user_data, pins);
        }
    }
    sys->pins = pins;
    kbd_update(&sys->kbd, micro_seconds);
    return num_ticks;
}

static uint16_t _vic20_vic_fetch(uint16_t addr, void* user_data) {
    vic20_t* sys = (vic20_t*) user_data;
    uint16_t data = (sys->color_ram[addr & 0x03FF]<<8) | mem_rd(&sys->mem_vic, addr);
    return data;
}

static void _vic20_init_key_map(vic20_t* sys) {
    kbd_init(&sys->kbd, 1);
    const char* keymap =
    // no shift
    //   01234567 (col)
        "1     Q2"  // row 0
        "3WA ZSE4"  // row 1
        "5RDXCFT6"  // row 2
        "7YGVBHU8"  // row 3
        "9IJNMKO0"  // row 4
        "+PL,.:@-"  // row 5
        "~*;/ =  "  // row 6, ~ is british pound
        "        "  // row 7

        /* shift */
        "!     q\""
        "#wa zse$"
        "%rdxcft^"
        "&ygvbhu*"
        "(ijnmko)"
        " pl<>[  "
        "$ ]?    "
        "        ";
    CHIPS_ASSERT(strlen(keymap) == 128);
    // shift is column 3, line 1
    kbd_register_modifier(&sys->kbd, 0, 3, 1);
    // ctrl is column 2, line 0
    kbd_register_modifier(&sys->kbd, 1, 2, 0);
    for (int shift = 0; shift < 2; shift++) {
        for (int column = 0; column < 8; column++) {
            for (int line = 0; line < 8; line++) {
                int c = keymap[shift*64 + line*8 + column];
                if (c != 0x20) {
                    kbd_register_key(&sys->kbd, c, column, line, shift?(1<<0):0);
                }
            }
        }
    }

    // special keys
    kbd_register_key(&sys->kbd, 0x20, 4, 0, 0);    // space
    kbd_register_key(&sys->kbd, 0x08, 2, 7, 1);    // cursor left
    kbd_register_key(&sys->kbd, 0x09, 2, 7, 0);    // cursor right
    kbd_register_key(&sys->kbd, 0x0A, 3, 7, 0);    // cursor down
    kbd_register_key(&sys->kbd, 0x0B, 3, 7, 1);    // cursor up
    kbd_register_key(&sys->kbd, 0x01, 0, 7, 0);    // delete
    kbd_register_key(&sys->kbd, 0x0D, 1, 7, 0);    // return
    kbd_register_key(&sys->kbd, 0x03, 3, 0, 0);    // stop
    kbd_register_key(&sys->kbd, 0xF1, 4, 7, 0);
    kbd_register_key(&sys->kbd, 0xF2, 4, 7, 1);
    kbd_register_key(&sys->kbd, 0xF3, 5, 7, 0);
    kbd_register_key(&sys->kbd, 0xF4, 5, 7, 1);
    kbd_register_key(&sys->kbd, 0xF5, 6, 7, 0);
    kbd_register_key(&sys->kbd, 0xF6, 6, 7, 1);
    kbd_register_key(&sys->kbd, 0xF7, 7, 7, 0);
    kbd_register_key(&sys->kbd, 0xF8, 7, 7, 1);
}

bool vic20_quickload(vic20_t* sys, chips_range_t data) {
    CHIPS_ASSERT(sys && sys->valid && data.ptr && (data.size > 0));
    if (data.size < 2) {
        return false;
    }
    const uint8_t* ptr = (uint8_t*)data.ptr;
    const uint16_t start_addr = ptr[1]<<8 | ptr[0];
    ptr += 2;
    const uint16_t end_addr = start_addr + (data.size - 2);
    uint16_t addr = start_addr;
    while (addr < end_addr) {
        mem_wr(&sys->mem_cpu, addr++, *ptr++);
    }
    return true;
}

bool vic20_insert_rom_cartridge(vic20_t* sys, chips_range_t data) {
    CHIPS_ASSERT(sys && sys->valid && data.ptr && (data.size > 0));
    if (data.size < 2) {
        return false;
    }

    /* the cartridge ROM may be a special merged PRG with a gap between the
       two memory regions with valid data, we cannot scribble over memory
       in that gap, so use a temporary memory mapping
    */
    const uint8_t* ptr = (uint8_t*)data.ptr;
    const uint16_t start_addr = ptr[1]<<8 | ptr[0];
    ptr += 2;
    const uint16_t end_addr = start_addr + (data.size - 2);
    uint16_t addr = start_addr;
    while (addr < end_addr) {
        mem_wr(&sys->mem_cart, addr++, *ptr++);
    }

    // map the ROM cartridge into the CPU's memory layer 0
    mem_unmap_layer(&sys->mem_cpu, 0);
    if (start_addr == 0x2000) {
        mem_map_rom(&sys->mem_cpu, 0, 0x2000, 0x2000, sys->ram_exp[0]);
    }
    else if (start_addr == 0x4000) {
        mem_map_rom(&sys->mem_cpu, 0, 0x4000, 0x2000, sys->ram_exp[1]);
    }
    else if (start_addr == 0x6000) {
        mem_map_rom(&sys->mem_cpu, 0, 0x6000, 0x2000, sys->ram_exp[2]);
    }
    mem_map_rom(&sys->mem_cpu, 0, 0xA000, 0x2000, sys->ram_exp[3]);
    sys->pins |= M6502_RES;
    return true;
}

void vic20_remove_rom_cartridge(vic20_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    mem_unmap_layer(&sys->mem_cpu, 0);
    sys->pins |= M6502_RES;
}

// generate precomputed VIA-1 and VIA-2 joystick port masks
static void _vic20_update_joymasks(vic20_t* sys) {
    uint8_t jm = sys->kbd_joy_mask | sys->joy_joy_mask;
    sys->via1_joy_mask = M6522_PA2|M6522_PA3|M6522_PA4|M6522_PA5;
    sys->via2_joy_mask = M6522_PB7;
    if (jm & VIC20_JOYSTICK_LEFT) {
        sys->via1_joy_mask &= ~M6522_PA2;
    }
    if (jm & VIC20_JOYSTICK_DOWN) {
        sys->via1_joy_mask &= ~M6522_PA3;
    }
    if (jm & VIC20_JOYSTICK_LEFT) {
        sys->via1_joy_mask &= ~M6522_PA4;
    }
    if (jm & VIC20_JOYSTICK_BTN) {
        sys->via1_joy_mask &= ~M6522_PA5;
    }
    if (jm & VIC20_JOYSTICK_RIGHT) {
        sys->via2_joy_mask &= ~M6522_PB7;
    }
}

void vic20_key_down(vic20_t* sys, int key_code) {
    CHIPS_ASSERT(sys && sys->valid);
    if (sys->joystick_type == VIC20_JOYSTICKTYPE_NONE) {
        kbd_key_down(&sys->kbd, key_code);
    }
    else {
        uint8_t m = 0;
        switch (key_code) {
            case 0x20: m = VIC20_JOYSTICK_BTN; break;
            case 0x08: m = VIC20_JOYSTICK_LEFT; break;
            case 0x09: m = VIC20_JOYSTICK_RIGHT; break;
            case 0x0A: m = VIC20_JOYSTICK_DOWN; break;
            case 0x0B: m = VIC20_JOYSTICK_UP; break;
            default: kbd_key_down(&sys->kbd, key_code); break;
        }
        if (m != 0) {
            if (sys->joystick_type == VIC20_JOYSTICKTYPE_DIGITAL) {
                sys->kbd_joy_mask |= m;
                _vic20_update_joymasks(sys);
            }
        }
    }
}

void vic20_key_up(vic20_t* sys, int key_code) {
    CHIPS_ASSERT(sys && sys->valid);
    if (sys->joystick_type == VIC20_JOYSTICKTYPE_NONE) {
        kbd_key_up(&sys->kbd, key_code);
    }
    else {
        uint8_t m = 0;
        switch (key_code) {
            case 0x20: m = VIC20_JOYSTICK_BTN; break;
            case 0x08: m = VIC20_JOYSTICK_LEFT; break;
            case 0x09: m = VIC20_JOYSTICK_RIGHT; break;
            case 0x0A: m = VIC20_JOYSTICK_DOWN; break;
            case 0x0B: m = VIC20_JOYSTICK_UP; break;
            default: kbd_key_up(&sys->kbd, key_code); break;
        }
        if (m != 0) {
            if (sys->joystick_type == VIC20_JOYSTICKTYPE_DIGITAL) {
                sys->kbd_joy_mask &= ~m;
                _vic20_update_joymasks(sys);
            }
        }
    }
}

void vic20_set_joystick_type(vic20_t* sys, vic20_joystick_type_t type) {
    CHIPS_ASSERT(sys && sys->valid);
    sys->joystick_type = type;
}

vic20_joystick_type_t vic20_joystick_type(vic20_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    return sys->joystick_type;
}

void vic20_joystick(vic20_t* sys, uint8_t joy_mask) {
    CHIPS_ASSERT(sys && sys->valid);
    sys->joy_joy_mask = joy_mask;
    _vic20_update_joymasks(sys);
}

bool vic20_insert_tape(vic20_t* sys, chips_range_t data) {
    CHIPS_ASSERT(sys && sys->valid && sys->c1530.valid);
    return c1530_insert_tape(&sys->c1530, data);
}

void vic20_remove_tape(vic20_t* sys) {
    CHIPS_ASSERT(sys && sys->valid && sys->c1530.valid);
    c1530_remove_tape(&sys->c1530);
}

bool vic20_tape_inserted(vic20_t* sys) {
    CHIPS_ASSERT(sys && sys->valid && sys->c1530.valid);
    return c1530_tape_inserted(&sys->c1530);
}

void vic20_tape_play(vic20_t* sys) {
    CHIPS_ASSERT(sys && sys->valid && sys->c1530.valid);
    c1530_play(&sys->c1530);
}

void vic20_tape_stop(vic20_t* sys) {
    CHIPS_ASSERT(sys && sys->valid && sys->c1530.valid);
    c1530_stop(&sys->c1530);
}

bool vic20_is_tape_motor_on(vic20_t* sys) {
    CHIPS_ASSERT(sys && sys->valid && sys->c1530.valid);
    return c1530_is_motor_on(&sys->c1530);
}

chips_display_info_t vic20_display_info(vic20_t* sys) {
    chips_display_info_t res = {
        .frame = {
            .dim = {
                .width = M6561_FRAMEBUFFER_WIDTH,
                .height = M6561_FRAMEBUFFER_HEIGHT,
            },
            .bytes_per_pixel = 1,
            .buffer = {
                .ptr = sys ? sys->fb : 0,
                .size = M6561_FRAMEBUFFER_SIZE_BYTES,
            }
        },
        .palette = m6561_palette(),
    };
    if (sys) {
        res.screen = m6561_screen(&sys->vic);
    }
    else {
        res.screen = (chips_rect_t){
            .x = 0,
            .y = 0,
            .width = _VIC20_SCREEN_WIDTH,
            .height = _VIC20_SCREEN_HEIGHT,
        };
    }
    CHIPS_ASSERT(((sys == 0) && (res.frame.buffer.ptr == 0)) || ((sys != 0) && (res.frame.buffer.ptr != 0)));
    return res;
}

uint32_t vic20_save_snapshot(vic20_t* sys, vic20_t* dst) {
    CHIPS_ASSERT(sys && dst);
    *dst = *sys;
    chips_debug_snapshot_onsave(&dst->debug);
    chips_audio_callback_snapshot_onsave(&dst->audio.callback);
    m6502_snapshot_onsave(&dst->cpu);
    m6561_snapshot_onsave(&dst->vic);
    c1530_snapshot_onsave(&dst->c1530);
    mem_snapshot_onsave(&dst->mem_cpu, sys);
    mem_snapshot_onsave(&dst->mem_vic, sys);
    mem_snapshot_onsave(&dst->mem_cart, sys);
    return VIC20_SNAPSHOT_VERSION;
}

bool vic20_load_snapshot(vic20_t* sys, uint32_t version, vic20_t* src) {
    CHIPS_ASSERT(sys && src);
    if (version != VIC20_SNAPSHOT_VERSION) {
        return false;
    }
    static vic20_t im;
    im = *src;
    chips_debug_snapshot_onload(&im.debug, &sys->debug);
    chips_audio_callback_snapshot_onload(&im.audio.callback, &sys->audio.callback);
    m6502_snapshot_onload(&im.cpu, &sys->cpu);
    m6561_snapshot_onload(&im.vic, &sys->vic);
    c1530_snapshot_onload(&im.c1530, &sys->c1530);
    mem_snapshot_onload(&im.mem_cpu, sys);
    mem_snapshot_onload(&im.mem_vic, sys);
    mem_snapshot_onload(&im.mem_cart, sys);
    *sys = im;
    return true;
}

#endif // CHIPS_IMPL
