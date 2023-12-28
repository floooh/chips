#pragma once
/*#
    # 64.h

    An Commodore C64 (PAL) emulator in a C header

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

    - chips/chips_common.h
    - chips/m6502.h
    - chips/m6526.h
    - chips/m6569.h
    - chips/m6581.h
    - chips/kbd.h
    - chips/mem.h
    - chips/clk.h
    - systems/c1530.h
    - chips/m6522.h
    - systems/c1541.h

    ## The Commodore C64

    TODO!

    ## TODO:

    - floppy disc support

    ## Tests Status

    In chips-test/tests/testsuite-2.15/bin

        branchwrap:     ok
        cia1pb6:        ok
        cia1pb7:        ok
        cia1ta:         FAIL (OK, CIA sysclock not implemented)
        cia1tab:        ok
        cia1tb:         FAIL (OK, CIA sysclock not implemented)
        cia1tb123:      ok
        cia2pb6:        ok
        cia2pb7:        ok
        cia2ta:         FAIL (OK, CIA sysclock not implemented)
        cia2tb:         FAIL (OK, CIA sysclock not implemented)
        cntdef:         ok
        cnto2:          ok
        cpuport:        ok
        cputiming:      ok
        flipos:         ok
        icr01:          ok
        imr:            ok
        irq:            ok
        loadth:         ok
        mmu:            ok
        mmufetch:       ok
        nmi:            FAIL (1 error at 00/5)
        oneshot:        ok
        trap1..17:      ok

    In chips-test/tests/vice-tests/CIA:

    ciavarious:
        - all green, expect cia15.prg, which tests the CIA TOD clock,
          which isn't implemented

    cia-timer/cia-timer-oldcias.prg:
        - left side (CIA-1, IRQ) all green, right side (CIA-2, NMI) some red

    ciatimer/dd0dtest/dd0dtest.prg (NMI related):
        - some errors

    irqdelay:   all green

    mirrors/ciamirrors.prg: green

    reload0:
        reload0a.prg:   red
        reload0b.prg:   red

    shiftregister:
        cia-icr-test-continues-old.prg: green
        cia-icr-test-oneshot-old.prg: green
        cia-icr-test2-continues.prg: some red
        cia-icr-test2-oneshot.prg: some red
        cia-sp-test-continues-old.prg: much red (ok, CIA SP not implemented)
        cia-sp-test-oneshot-old.prg: much red (ok, CIA SP not implemented)

    timerbasics:    all green

    in chips-test/tests/vice-tests/interrupts:

    branchquirk:
        branchquirk-old.prg:    green
        branchquirk-nmiold.prg: red

    cia-int:
        cia-int-irq.prg:    green??
        cia-int-nmi.prg:    green??

    irq-ackn-bug:
        cia1.prg:       green
        cia2.prg:       green
        irq-ack-vicii.prg:  red
        irq-ackn_after_cli.prg: ???
        irq-ackn_after_cli2.prg: ???

    irqdma: (takes a long time)
        all fail?

    irqdummy/irqdummy.prg:  green

    irqnmi/irqnmi-old.prg: left (irq) green,right (nmi) red

    VICII:

    D011Test:                   TODO
    banking/banking.prg:        ok
    border:                     fail (border not opened)
    colorram/test.prg:          ok
    colorsplit/colorsplit.prg:  fail (horizontal offsets)
    dentest:    (these were mostly fixed by moving the raster interrupt check
                 in m6569.h to tick 63, which made the otherwise some tests
                 were flickering because a second raster interrupt wasn't stable)
        den01-48-0.prg:         ok
        den01-48-1.prg:         ok
        den01-48-2.prg:         ok
        den01-49-0.prg:         ok
        den01-49-1.prg:         ok
        den01-49-2.prg:         ok
        den10-48-0.prg:         ok
        den10-48-1.prg:         ok
        den10-48-2.prg:         FAIL
        den10-51-0.prg:         ok
        den10-51-1.prg:         ok
        den10-51-2.prg:         ok
        den10-51-3.prg:         ok
        denrsel-0.prg:          ok
        denrsel-1.prg:          ok
        denrsel-2.prg:          ok
        denrsel-63.prg:         ok
        denrsel-s0.prg:         ok
        denrsel-s1.prg:         ok
        denrsel-s2.prg:         ok
        denrsel55.prg:          ok
    dmadelay:
        test1-2a-03.prg:        ok
        test1-2a-04.prg:        FAIL (flickering)
        test1-2a-10.prg:        ok
        test1-2a-11.prg:        FAIL (1 char line off)
        test1-2a-16.prg:        ok
        test1-2a-17.prg:        FAIL (1 char line off)
        test1-2a-18.prg:        FAIL (1 char line/col off)
        test1.prg:              ??? (no reference image)
        test2-28-05.prg:        ok
        test2-28-06.prg:        FAIL (flickering)
        test2-28-11.prg:        ok
        test2-28-12.prg:        FAIL (one char line off)
        test2-28-16.prg:        ok
        test2-28-17.prg:        FAIL (one char line off)
        test2-28-18.prg:        FAIL (one char line/col off)
        test3-28-07.prg:        ok
        test3-28-08.prg:        FAIL (one char line off)
        test3-28-13.prg:        ok
        test3-28-14.prg:        FAIL (one char line off)
        test3-28-18.prg:        ok
        test3-28-19.prg:        FAIL (one char line off)
        test3-28-1a.prg:        FAIL (one char col off)

    fldscroll:  broken
    flibug/blackmail.prg:       reference image doesn't match

    frodotests:
        3fff.prg                ok
        d011h3.prg              FAIL (???)
        fld.prg                 ok
        lrborder:               FAIL (???)
        sprsync:                ok (???)
        stretch:                ok (???)
        tech-tech:              ok
        text26:                 ok

    gfxfetch/gfxfetch.prg:      FAIL (reference image doesn't match in boder)

    greydot/greydot.prg:        FAIL (ref image doesn't match, color bars start one tick late)

    lp-trigger:
        test1.prg:              FAIL (flickering)
        test2.prg:              FAIL

    lplatency/lplatency.prg:    FAIL

    movesplit:                  ????

    phi1timing:                 FAIL

    rasterirq:                  FAIL (reference image doesn't match)

    screenpos:                  FAIL (reference image doesn't match)

    split-tests:
        bascan          FAIL
        fetchsplit      FAIL (flickering characters)
        lightpen        FAIL
        modesplit       FAIL (ref image doesn't match)
        spritescan      FAIL

    sprite0move         ???

    spritebug           TODO
    all other sprite tests: TODO

    vicii-timing:       FAIL (ref image doesn't match)

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

// bump snapshot version when c64_t memory layout changes
#define C64_SNAPSHOT_VERSION (1)

#define C64_FREQUENCY (985248)              // clock frequency in Hz
#define C64_MAX_AUDIO_SAMPLES (1024)        // max number of audio samples in internal sample buffer
#define C64_DEFAULT_AUDIO_SAMPLES (128)     // default number of samples in internal sample buffer

// C64 joystick types
typedef enum {
    C64_JOYSTICKTYPE_NONE,
    C64_JOYSTICKTYPE_DIGITAL_1,
    C64_JOYSTICKTYPE_DIGITAL_2,
    C64_JOYSTICKTYPE_DIGITAL_12,    // input routed to both joysticks
    C64_JOYSTICKTYPE_PADDLE_1,      // FIXME: not emulated
    C64_JOYSTICKTYPE_PADDLE_2,      // FIXME: not emulated
} c64_joystick_type_t;

// joystick mask bits
#define C64_JOYSTICK_UP    (1<<0)
#define C64_JOYSTICK_DOWN  (1<<1)
#define C64_JOYSTICK_LEFT  (1<<2)
#define C64_JOYSTICK_RIGHT (1<<3)
#define C64_JOYSTICK_BTN   (1<<4)

// CPU port memory mapping bits
#define C64_CPUPORT_LORAM (1<<0)
#define C64_CPUPORT_HIRAM (1<<1)
#define C64_CPUPORT_CHAREN (1<<2)

// casette port bits, same as C1530_CASPORT_*
#define C64_CASPORT_MOTOR   (1<<0)  // 1: motor off, 0: motor on
#define C64_CASPORT_READ    (1<<1)  // 1: read signal from datasette, connected to CIA-1 FLAG
#define C64_CASPORT_WRITE   (1<<2)  // not implemented
#define C64_CASPORT_SENSE   (1<<3)  // 1: play button up, 0: play button down

// IEC port bits, same as C1541_IECPORT_*
#define C64_IECPORT_RESET   (1<<0)  // 1: RESET, 0: no reset
#define C64_IECPORT_SRQIN   (1<<1)  // connected to CIA-1 FLAG
#define C64_IECPORT_DATA    (1<<2)
#define C64_IECPORT_CLK     (1<<3)
#define C64_IECPORT_ATN     (1<<4)

// special keyboard keys
#define C64_KEY_SPACE    (0x20)     // space
#define C64_KEY_CSRLEFT  (0x08)     // cursor left (shift+cursor right)
#define C64_KEY_CSRRIGHT (0x09)     // cursor right
#define C64_KEY_CSRDOWN  (0x0A)     // cursor down
#define C64_KEY_CSRUP    (0x0B)     // cursor up (shift+cursor down)
#define C64_KEY_DEL      (0x01)     // delete
#define C64_KEY_INST     (0x10)     // inst (shift+del)
#define C64_KEY_HOME     (0x0C)     // home
#define C64_KEY_CLR      (0x02)     // clear (shift+home)
#define C64_KEY_RETURN   (0x0D)     // return
#define C64_KEY_CTRL     (0x0E)     // ctrl
#define C64_KEY_CBM      (0x0F)     // C= commodore key
#define C64_KEY_RESTORE  (0xFF)     // restore (connected to the NMI line)
#define C64_KEY_STOP     (0x03)     // run stop
#define C64_KEY_RUN      (0x07)     // run (shift+run stop)
#define C64_KEY_LEFT     (0x04)     // left arrow symbol
#define C64_KEY_F1       (0xF1)     // F1
#define C64_KEY_F2       (0xF2)     // F2
#define C64_KEY_F3       (0xF3)     // F3
#define C64_KEY_F4       (0xF4)     // F4
#define C64_KEY_F5       (0xF5)     // F5
#define C64_KEY_F6       (0xF6)     // F6
#define C64_KEY_F7       (0xF7)     // F7
#define C64_KEY_F8       (0xF8)     // F8

// config parameters for c64_init()
typedef struct {
    bool c1530_enabled;     // true to enable the C1530 datassette emulation
    bool c1541_enabled;     // true to enable the C1541 floppy drive emulation
    c64_joystick_type_t joystick_type;  // default is C64_JOYSTICK_NONE
    chips_debug_t debug;    // optional debugging hook
    chips_audio_desc_t audio;   // audio output options
    // ROM images
    struct {
        chips_range_t chars;     // 4 KByte character ROM dump
        chips_range_t basic;     // 8 KByte BASIC dump
        chips_range_t kernal;    // 8 KByte KERNAL dump
        // optional C1514 ROM images
        struct {
            chips_range_t c000_dfff;
            chips_range_t e000_ffff;
        } c1541;
    } roms;
} c64_desc_t;

// C64 emulator state
typedef struct {
    m6502_t cpu;
    m6526_t cia_1;
    m6526_t cia_2;
    m6569_t vic;
    m6581_t sid;
    uint64_t pins;

    c64_joystick_type_t joystick_type;
    bool io_mapped;             // true when D000..DFFF has IO area mapped in
    uint8_t cas_port;           // cassette port, shared with c1530_t if datasette is connected
    uint8_t iec_port;           // IEC serial port, shared with c1541_t if connected
    uint8_t cpu_port;           // last state of CPU port (for memory mapping)
    uint8_t kbd_joy1_mask;      // current joystick-1 state from keyboard-joystick emulation
    uint8_t kbd_joy2_mask;      // current joystick-2 state from keyboard-joystick emulation
    uint8_t joy_joy1_mask;      // current joystick-1 state from c64_joystick()
    uint8_t joy_joy2_mask;      // current joystick-2 state from c64_joystick()
    uint16_t vic_bank_select;   // upper 4 address bits from CIA-2 port A

    kbd_t kbd;                  // keyboard matrix state
    mem_t mem_cpu;              // CPU-visible memory mapping
    mem_t mem_vic;              // VIC-visible memory mapping
    bool valid;
    chips_debug_t debug;

    struct {
        chips_audio_callback_t callback;
        int num_samples;
        int sample_pos;
        float sample_buffer[C64_MAX_AUDIO_SAMPLES];
    } audio;

    uint8_t color_ram[1024];        // special static color ram
    uint8_t ram[1<<16];             // general ram
    uint8_t rom_char[0x1000];       // 4 KB character ROM image
    uint8_t rom_basic[0x2000];      // 8 KB BASIC ROM image
    uint8_t rom_kernal[0x2000];     // 8 KB KERNAL V3 ROM image
    alignas(64) uint8_t fb[M6569_FRAMEBUFFER_SIZE_BYTES];

    c1530_t c1530;      // optional datassette
    c1541_t c1541;      // optional floppy drive
} c64_t;

// initialize a new C64 instance
void c64_init(c64_t* sys, const c64_desc_t* desc);
// discard C64 instance
void c64_discard(c64_t* sys);
// reset a C64 instance
void c64_reset(c64_t* sys);
// get framebuffer and display attributes
chips_display_info_t c64_display_info(c64_t* sys);
// tick C64 instance for a given number of microseconds, return number of ticks executed
uint32_t c64_exec(c64_t* sys, uint32_t micro_seconds);
// send a key-down event to the C64
void c64_key_down(c64_t* sys, int key_code);
// send a key-up event to the C64
void c64_key_up(c64_t* sys, int key_code);
// enable/disable joystick emulation
void c64_set_joystick_type(c64_t* sys, c64_joystick_type_t type);
// get current joystick emulation type
c64_joystick_type_t c64_joystick_type(c64_t* sys);
// set joystick mask (combination of C64_JOYSTICK_*)
void c64_joystick(c64_t* sys, uint8_t joy1_mask, uint8_t joy2_mask);
// quickload a .bin/.prg file
bool c64_quickload(c64_t* sys, chips_range_t data);
// insert tape as .TAP file (c1530 must be enabled)
bool c64_insert_tape(c64_t* sys, chips_range_t data);
// remove tape file
void c64_remove_tape(c64_t* sys);
// return true if a tape is currently inserted
bool c64_tape_inserted(c64_t* sys);
// start the tape (press the Play button)
void c64_tape_play(c64_t* sys);
// stop the tape (unpress the Play button
void c64_tape_stop(c64_t* sys);
// return true if tape motor is on
bool c64_is_tape_motor_on(c64_t* sys);
// save a snapshot, patches pointers to zero and offsets, returns snapshot version
uint32_t c64_save_snapshot(c64_t* sys, c64_t* dst);
// load a snapshot, returns false if snapshot versions don't match
bool c64_load_snapshot(c64_t* sys, uint32_t version, c64_t* src);
// perform a RUN BASIC call
void c64_basic_run(c64_t* sys);
// perform a LOAD BASIC call
void c64_basic_load(c64_t* sys);
// perform a SYS xxxx BASIC call via the BASIC input buffer
void c64_basic_syscall(c64_t* sys, uint16_t addr);
// returns the SYS call return address (can be used to set a breakpoint)
uint16_t c64_syscall_return_addr(void);

#ifdef __cplusplus
} // extern "C"
#endif

/*-- IMPLEMENTATION ----------------------------------------------------------*/
#ifdef CHIPS_IMPL
#include <string.h> // memcpy, memset
#ifndef CHIPS_ASSERT
    #include <assert.h>
    #define CHIPS_ASSERT(c) assert(c)
#endif

#define _C64_SCREEN_WIDTH (392)
#define _C64_SCREEN_HEIGHT (272)
#define _C64_SCREEN_X (64)
#define _C64_SCREEN_Y (24)

static uint8_t _c64_cpu_port_in(void* user_data);
static void _c64_cpu_port_out(uint8_t data, void* user_data);
static uint16_t _c64_vic_fetch(uint16_t addr, void* user_data);
static void _c64_update_memory_map(c64_t* sys);
static void _c64_init_key_map(c64_t* sys);
static void _c64_init_memory_map(c64_t* sys);

#define _C64_DEFAULT(val,def) (((val) != 0) ? (val) : (def))

void c64_init(c64_t* sys, const c64_desc_t* desc) {
    CHIPS_ASSERT(sys && desc);
    if (desc->debug.callback.func) { CHIPS_ASSERT(desc->debug.stopped); }

    memset(sys, 0, sizeof(c64_t));
    sys->valid = true;
    sys->joystick_type = desc->joystick_type;
    sys->debug = desc->debug;
    sys->audio.callback = desc->audio.callback;
    sys->audio.num_samples = _C64_DEFAULT(desc->audio.num_samples, C64_DEFAULT_AUDIO_SAMPLES);
    CHIPS_ASSERT(sys->audio.num_samples <= C64_MAX_AUDIO_SAMPLES);
    CHIPS_ASSERT(desc->roms.chars.ptr && (desc->roms.chars.size == sizeof(sys->rom_char)));
    CHIPS_ASSERT(desc->roms.basic.ptr && (desc->roms.basic.size == sizeof(sys->rom_basic)));
    CHIPS_ASSERT(desc->roms.kernal.ptr && (desc->roms.kernal.size == sizeof(sys->rom_kernal)));
    memcpy(sys->rom_char, desc->roms.chars.ptr, sizeof(sys->rom_char));
    memcpy(sys->rom_basic, desc->roms.basic.ptr, sizeof(sys->rom_basic));
    memcpy(sys->rom_kernal, desc->roms.kernal.ptr, sizeof(sys->rom_kernal));

    // initialize the hardware
    sys->cpu_port = 0xF7;       // for initial memory mapping
    sys->io_mapped = true;
    sys->cas_port = C64_CASPORT_MOTOR|C64_CASPORT_SENSE;

    sys->pins = m6502_init(&sys->cpu, &(m6502_desc_t) {
        .m6510_in_cb = _c64_cpu_port_in,
        .m6510_out_cb = _c64_cpu_port_out,
        .m6510_io_pullup = 0x17,
        .m6510_io_floating = 0xC8,
        .m6510_user_data = sys,
    });
    m6526_init(&sys->cia_1);
    m6526_init(&sys->cia_2);
    m6569_init(&sys->vic, &(m6569_desc_t){
        .fetch_cb = _c64_vic_fetch,
        .framebuffer = {
            .ptr = sys->fb,
            .size = sizeof(sys->fb),
        },
        .screen = {
            .x = _C64_SCREEN_X,
            .y = _C64_SCREEN_Y,
            .width = _C64_SCREEN_WIDTH,
            .height = _C64_SCREEN_HEIGHT,
        },
        .user_data = sys,
    });
    m6581_init(&sys->sid, &(m6581_desc_t){
        .tick_hz = C64_FREQUENCY,
        .sound_hz = _C64_DEFAULT(desc->audio.sample_rate, 44100),
        .magnitude = _C64_DEFAULT(desc->audio.volume, 1.0f),
    });
    _c64_init_key_map(sys);
    _c64_init_memory_map(sys);
    if (desc->c1530_enabled) {
        c1530_init(&sys->c1530, &(c1530_desc_t){
            .cas_port = &sys->cas_port,
        });
    }
    if (desc->c1541_enabled) {
        c1541_init(&sys->c1541, &(c1541_desc_t){
            .iec_port = &sys->iec_port,
            .roms = {
                .c000_dfff = desc->roms.c1541.c000_dfff,
                .e000_ffff = desc->roms.c1541.e000_ffff
            },
        });
    }
}

void c64_discard(c64_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    sys->valid = false;
    if (sys->c1530.valid) {
        c1530_discard(&sys->c1530);
    }
    if (sys->c1541.valid) {
        c1541_discard(&sys->c1541);
    }
}

void c64_reset(c64_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    sys->cpu_port = 0xF7;
    sys->kbd_joy1_mask = sys->kbd_joy2_mask = 0;
    sys->joy_joy1_mask = sys->joy_joy2_mask = 0;
    sys->io_mapped = true;
    sys->cas_port = C64_CASPORT_MOTOR|C64_CASPORT_SENSE;
    _c64_update_memory_map(sys);
    sys->pins |= M6502_RES;
    m6526_reset(&sys->cia_1);
    m6526_reset(&sys->cia_2);
    m6569_reset(&sys->vic);
    m6581_reset(&sys->sid);
}

static uint64_t _c64_tick(c64_t* sys, uint64_t pins) {
    // FIXME: move datasette and floppy tick to end
    if (sys->c1530.valid) {
        c1530_tick(&sys->c1530);
    }
    if (sys->c1541.valid) {
        c1541_tick(&sys->c1541);
    }

    // tick the CPU
    pins = m6502_tick(&sys->cpu, pins);
    const uint16_t addr = M6502_GET_ADDR(pins);

    // those pins are set each tick by the CIAs and VIC
    pins &= ~(M6502_IRQ|M6502_NMI|M6502_RDY|M6510_AEC);

    /*  address decoding

        When the RDY pin is active (during bad lines), no CPU/chip
        communication takes place starting with the first read access.
    */
    bool cpu_io_access = false;
    bool color_ram_access = false;
    bool mem_access = false;
    uint64_t vic_pins = pins & M6502_PIN_MASK;
    uint64_t cia1_pins = pins & M6502_PIN_MASK;
    uint64_t cia2_pins = pins & M6502_PIN_MASK;
    uint64_t sid_pins = pins & M6502_PIN_MASK;
    if ((pins & (M6502_RDY|M6502_RW)) != (M6502_RDY|M6502_RW)) {
        if (M6510_CHECK_IO(pins)) {
            cpu_io_access = true;
        }
        else {
            if (sys->io_mapped && ((addr & 0xF000) == 0xD000)) {
                if (addr < 0xD400) {
                    // VIC-II (D000..D3FF)
                    vic_pins |= M6569_CS;
                }
                else if (addr < 0xD800) {
                    // SID (D400..D7FF)
                    sid_pins |= M6581_CS;
                }
                else if (addr < 0xDC00) {
                    // read or write the special color Static-RAM bank (D800..DBFF)
                    color_ram_access = true;
                }
                else if (addr < 0xDD00) {
                    // CIA-1 (DC00..DCFF)
                    cia1_pins |= M6526_CS;
                }
                else if (addr < 0xDE00) {
                    // CIA-2 (DD00..DDFF)
                    cia2_pins |= M6526_CS;
                }
            }
            else {
                mem_access = true;
            }
        }
    }

    // tick the SID
    {
        sid_pins = m6581_tick(&sys->sid, sid_pins);
        if (sid_pins & M6581_SAMPLE) {
            // new audio sample ready
            sys->audio.sample_buffer[sys->audio.sample_pos++] = sys->sid.sample;
            if (sys->audio.sample_pos == sys->audio.num_samples) {
                if (sys->audio.callback.func) {
                    sys->audio.callback.func(sys->audio.sample_buffer, sys->audio.num_samples, sys->audio.callback.user_data);
                }
                sys->audio.sample_pos = 0;
            }
        }
        if ((sid_pins & (M6581_CS|M6581_RW)) == (M6581_CS|M6581_RW)) {
            pins = M6502_COPY_DATA(pins, sid_pins);
        }
    }

    /* tick CIA-1:

        In Port A:
            joystick 2 input
        In Port B:
            combined keyboard matrix columns and joystick 1
        Cas Port Read => Flag pin

        Out Port A:
            write keyboard matrix lines

        IRQ pin is connected to the CPU IRQ pin
    */
    {
        // cassette port READ pin is connected to CIA-1 FLAG pin
        const uint8_t pa = ~(sys->kbd_joy2_mask|sys->joy_joy2_mask);
        const uint8_t pb = ~(kbd_scan_columns(&sys->kbd) | sys->kbd_joy1_mask | sys->joy_joy1_mask);
        M6526_SET_PAB(cia1_pins, pa, pb);
        if (sys->cas_port & C64_CASPORT_READ) {
            cia1_pins |= M6526_FLAG;
        }
        cia1_pins = m6526_tick(&sys->cia_1, cia1_pins);
        const uint8_t kbd_lines = ~M6526_GET_PA(cia1_pins);
        kbd_set_active_lines(&sys->kbd, kbd_lines);
        if (cia1_pins & M6502_IRQ) {
            pins |= M6502_IRQ;
        }
        if ((cia1_pins & (M6526_CS|M6526_RW)) == (M6526_CS|M6526_RW)) {
            pins = M6502_COPY_DATA(pins, cia1_pins);
        }
    }

    /* tick CIA-2
        In Port A:
            bits 0..5: output (see cia2_out)
            bits 6..7: serial bus input, not implemented
        In Port B:
            RS232 / user functionality (not implemented)

        Out Port A:
            bits 0..1: VIC-II bank select:
                00: bank 3 C000..FFFF
                01: bank 2 8000..BFFF
                10: bank 1 4000..7FFF
                11: bank 0 0000..3FFF
            bit 2: RS-232 TXD Outout (not implemented)
            bit 3..5: serial bus output (not implemented)
            bit 6..7: input (see cia2_in)
        Out Port B:
            RS232 / user functionality (not implemented)

        CIA-2 IRQ pin connected to CPU NMI pin
    */
    {
        M6526_SET_PAB(cia2_pins, 0xFF, 0xFF);
        cia2_pins = m6526_tick(&sys->cia_2, cia2_pins);
        sys->vic_bank_select = ((~M6526_GET_PA(cia2_pins))&3)<<14;
        if (cia2_pins & M6502_IRQ) {
            pins |= M6502_NMI;
        }
        if ((cia2_pins & (M6526_CS|M6526_RW)) == (M6526_CS|M6526_RW)) {
            pins = M6502_COPY_DATA(pins, cia2_pins);
        }
    }

    // the RESTORE key, along with CIA-2 IRQ, is connected to the NMI line,
    if(sys->kbd.scanout_column_masks[8] & 1) {
        pins |= M6502_NMI;
    }

    /* tick the VIC-II display chip:
        - the VIC-II IRQ pin is connected to the CPU IRQ pin and goes
        active when the VIC-II requests a rasterline interrupt
        - the VIC-II BA pin is connected to the CPU RDY pin, and stops
        the CPU on the first CPU read access after BA goes active
        - the VIC-II AEC pin is connected to the CPU AEC pin, currently
        this goes active during a badline, but is not checked
    */
    {
        vic_pins = m6569_tick(&sys->vic, vic_pins);
        pins |= (vic_pins & (M6502_IRQ|M6502_RDY|M6510_AEC));
        if ((vic_pins & (M6569_CS|M6569_RW)) == (M6569_CS|M6569_RW)) {
            pins = M6502_COPY_DATA(pins, vic_pins);
        }
    }

    /* remaining CPU IO and memory accesses, those don't fit into the
       "universal tick model" (yet?)
    */
    if (cpu_io_access) {
        // ...the integrated IO port in the M6510 CPU at addresses 0 and 1
        pins = m6510_iorq(&sys->cpu, pins);
    }
    else if (color_ram_access) {
        if (pins & M6502_RW) {
            M6502_SET_DATA(pins, sys->color_ram[addr & 0x03FF]);
        }
        else {
            sys->color_ram[addr & 0x03FF] = M6502_GET_DATA(pins);
        }
    }
    else if (mem_access) {
        if (pins & M6502_RW) {
            // memory read
            M6502_SET_DATA(pins, mem_rd(&sys->mem_cpu, addr));
        }
        else {
            // memory write
            mem_wr(&sys->mem_cpu, addr, M6502_GET_DATA(pins));
        }
    }
    return pins;
}

static uint8_t _c64_cpu_port_in(void* user_data) {
    c64_t* sys = (c64_t*) user_data;
    /*
        Input from the integrated M6510 CPU IO port

        bit 4: [in] datasette button status (1: no button pressed)
    */
    uint8_t val = 7;
    if (sys->cas_port & C64_CASPORT_SENSE) {
        val |= (1<<4);
    }
    return val;
}

static void _c64_cpu_port_out(uint8_t data, void* user_data) {
    c64_t* sys = (c64_t*) user_data;
    /*
        Output to the integrated M6510 CPU IO port

        bits 0..2:  [out] memory configuration

        bit 3: [out] datasette output signal level
        bit 5: [out] datasette motor control (1: motor off)
    */
    if (data & (1<<5)) {
        /* motor off */
        sys->cas_port |= C64_CASPORT_MOTOR;
    }
    else {
        /* motor on */
        sys->cas_port &= ~C64_CASPORT_MOTOR;
    }
    /* only update memory configuration if the relevant bits have changed */
    bool need_mem_update = 0 != ((sys->cpu_port ^ data) & 7);
    sys->cpu_port = data;
    if (need_mem_update) {
        _c64_update_memory_map(sys);
    }
}

static uint16_t _c64_vic_fetch(uint16_t addr, void* user_data) {
    c64_t* sys = (c64_t*) user_data;
    /*
        Fetch data into the VIC-II.

        The VIC-II has a 14-bit address bus and 12-bit data bus, and
        has a different memory mapping then the CPU (that's why it
        goes through the mem_vic pagetable):
            - a full 16-bit address is formed by taking the address bits
              14 and 15 from the value written to CIA-1 port A
            - the lower 8 bits of the VIC-II data bus are connected
              to the shared system data bus, this is used to read
              character mask and pixel data
            - the upper 4 bits of the VIC-II data bus are hardwired to the
              static color RAM
    */
    addr |= sys->vic_bank_select;
    uint16_t data = (sys->color_ram[addr & 0x03FF]<<8) | mem_rd(&sys->mem_vic, addr);
    return data;
}

static void _c64_update_memory_map(c64_t* sys) {
    sys->io_mapped = false;
    uint8_t* read_ptr;
    // shortcut if HIRAM and LORAM is 0, everything is RAM
    if ((sys->cpu_port & (C64_CPUPORT_HIRAM|C64_CPUPORT_LORAM)) == 0) {
        mem_map_ram(&sys->mem_cpu, 0, 0xA000, 0x6000, sys->ram+0xA000);
    }
    else {
        // A000..BFFF is either RAM-behind-BASIC-ROM or RAM
        if ((sys->cpu_port & (C64_CPUPORT_HIRAM|C64_CPUPORT_LORAM)) == (C64_CPUPORT_HIRAM|C64_CPUPORT_LORAM)) {
            read_ptr = sys->rom_basic;
        }
        else {
            read_ptr = sys->ram + 0xA000;
        }
        mem_map_rw(&sys->mem_cpu, 0, 0xA000, 0x2000, read_ptr, sys->ram+0xA000);

        // E000..FFFF is either RAM-behind-KERNAL-ROM or RAM
        if (sys->cpu_port & C64_CPUPORT_HIRAM) {
            read_ptr = sys->rom_kernal;
        }
        else {
            read_ptr = sys->ram + 0xE000;
        }
        mem_map_rw(&sys->mem_cpu, 0, 0xE000, 0x2000, read_ptr, sys->ram+0xE000);

        // D000..DFFF can be Char-ROM or I/O
        if  (sys->cpu_port & C64_CPUPORT_CHAREN) {
            sys->io_mapped = true;
        }
        else {
            mem_map_rw(&sys->mem_cpu, 0, 0xD000, 0x1000, sys->rom_char, sys->ram+0xD000);
        }
    }
}

static void _c64_init_memory_map(c64_t* sys) {
    // seperate memory mapping for CPU and VIC-II
    mem_init(&sys->mem_cpu);
    mem_init(&sys->mem_vic);

    /*
        the C64 has a weird RAM init pattern of 64 bytes 00 and 64 bytes FF
        alternating, probably with some randomness sprinkled in
        (see this thread: http://csdb.dk/forums/?roomid=11&topicid=116800&firstpost=2)
        this is important at least for the value of the 'ghost byte' at 0x3FFF,
        which is 0xFF
    */
    size_t i;
    for (i = 0; i < (1<<16);) {
        for (size_t j = 0; j < 64; j++, i++) {
            sys->ram[i] = 0x00;
        }
        for (size_t j = 0; j < 64; j++, i++) {
            sys->ram[i] = 0xFF;
        }
    }
    CHIPS_ASSERT(i == 0x10000);

    /* setup the initial CPU memory map
       0000..9FFF and C000.CFFF is always RAM
    */
    mem_map_ram(&sys->mem_cpu, 0, 0x0000, 0xA000, sys->ram);
    mem_map_ram(&sys->mem_cpu, 0, 0xC000, 0x1000, sys->ram+0xC000);
    // A000..BFFF, D000..DFFF and E000..FFFF are configurable
    _c64_update_memory_map(sys);

    /* setup the separate VIC-II memory map (64 KByte RAM) overlayed with
       character ROMS at 0x1000.0x1FFF and 0x9000..0x9FFF
    */
    mem_map_ram(&sys->mem_vic, 1, 0x0000, 0x10000, sys->ram);
    mem_map_rom(&sys->mem_vic, 0, 0x1000, 0x1000, sys->rom_char);
    mem_map_rom(&sys->mem_vic, 0, 0x9000, 0x1000, sys->rom_char);
}

static void _c64_init_key_map(c64_t* sys) {
    /*
        http://sta.c64.org/cbm64kbdlay.html
        http://sta.c64.org/cbm64petkey.html
    */
    kbd_init(&sys->kbd, 1);

    const char* keymap =
        // no shift
        "        "
        "3WA4ZSE "
        "5RD6CFTX"
        "7YG8BHUV"
        "9IJ0MKON"
        "+PL-.:@,"
        "~*;  = /"  // ~ is actually the British Pound sign
        "1  2  Q "

        // shift
        "        "
        "#wa$zse "
        "%rd&cftx"
        "'yg(bhuv"
        ")ij0mkon"
        " pl >[ <"
        "$ ]    ?"
        "!  \"  q ";
    CHIPS_ASSERT(strlen(keymap) == 128);
    // shift is column 7, line 1
    kbd_register_modifier(&sys->kbd, 0, 7, 1);
    // ctrl is column 2, line 7
    kbd_register_modifier(&sys->kbd, 1, 2, 7);
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
    kbd_register_key(&sys->kbd, C64_KEY_SPACE   , 4, 7, 0);    // space
    kbd_register_key(&sys->kbd, C64_KEY_CSRLEFT , 2, 0, 1);    // cursor left (shift+cursor right)
    kbd_register_key(&sys->kbd, C64_KEY_CSRRIGHT, 2, 0, 0);    // cursor right
    kbd_register_key(&sys->kbd, C64_KEY_CSRDOWN , 7, 0, 0);    // cursor down
    kbd_register_key(&sys->kbd, C64_KEY_CSRUP   , 7, 0, 1);    // cursor up (shift+cursor down)
    kbd_register_key(&sys->kbd, C64_KEY_DEL     , 0, 0, 0);    // delete
    kbd_register_key(&sys->kbd, C64_KEY_INST    , 0, 0, 1);    // inst (shift+del)
    kbd_register_key(&sys->kbd, C64_KEY_HOME    , 3, 6, 0);    // home
    kbd_register_key(&sys->kbd, C64_KEY_CLR     , 3, 6, 1);    // clear (shift+home)
    kbd_register_key(&sys->kbd, C64_KEY_RETURN  , 1, 0, 0);    // return
    kbd_register_key(&sys->kbd, C64_KEY_CTRL    , 2, 7, 0);    // ctrl
    kbd_register_key(&sys->kbd, C64_KEY_CBM     , 5, 7, 0);    // C= commodore key
    kbd_register_key(&sys->kbd, C64_KEY_RESTORE , 0, 8, 0);    // restore (connected to the NMI line)
    kbd_register_key(&sys->kbd, C64_KEY_STOP    , 7, 7, 0);    // run stop
    kbd_register_key(&sys->kbd, C64_KEY_RUN     , 7, 7, 1);    // run (shift+run stop)
    kbd_register_key(&sys->kbd, C64_KEY_LEFT    , 1, 7, 0);    // left arrow symbol
    kbd_register_key(&sys->kbd, C64_KEY_F1      , 4, 0, 0);    // F1
    kbd_register_key(&sys->kbd, C64_KEY_F2      , 4, 0, 1);    // F2
    kbd_register_key(&sys->kbd, C64_KEY_F3      , 5, 0, 0);    // F3
    kbd_register_key(&sys->kbd, C64_KEY_F4      , 5, 0, 1);    // F4
    kbd_register_key(&sys->kbd, C64_KEY_F5      , 6, 0, 0);    // F5
    kbd_register_key(&sys->kbd, C64_KEY_F6      , 6, 0, 1);    // F6
    kbd_register_key(&sys->kbd, C64_KEY_F7      , 3, 0, 0);    // F7
    kbd_register_key(&sys->kbd, C64_KEY_F8      , 3, 0, 1);    // F8
}

uint32_t c64_exec(c64_t* sys, uint32_t micro_seconds) {
    CHIPS_ASSERT(sys && sys->valid);
    uint32_t num_ticks = clk_us_to_ticks(C64_FREQUENCY, micro_seconds);
    uint64_t pins = sys->pins;
    if (0 == sys->debug.callback.func) {
        // run without debug callback
        for (uint32_t ticks = 0; ticks < num_ticks; ticks++) {
            pins = _c64_tick(sys, pins);
        }
    }
    else {
        // run with debug callback
        for (uint32_t ticks = 0; (ticks < num_ticks) && !(*sys->debug.stopped); ticks++) {
            pins = _c64_tick(sys, pins);
            sys->debug.callback.func(sys->debug.callback.user_data, pins);
        }
    }
    sys->pins = pins;
    kbd_update(&sys->kbd, micro_seconds);
    return num_ticks;
}

void c64_key_down(c64_t* sys, int key_code) {
    CHIPS_ASSERT(sys && sys->valid);
    if (sys->joystick_type == C64_JOYSTICKTYPE_NONE) {
        kbd_key_down(&sys->kbd, key_code);
    }
    else {
        uint8_t m = 0;
        switch (key_code) {
            case 0x20: m = C64_JOYSTICK_BTN; break;
            case 0x08: m = C64_JOYSTICK_LEFT; break;
            case 0x09: m = C64_JOYSTICK_RIGHT; break;
            case 0x0A: m = C64_JOYSTICK_DOWN; break;
            case 0x0B: m = C64_JOYSTICK_UP; break;
            default: kbd_key_down(&sys->kbd, key_code); break;
        }
        if (m != 0) {
            switch (sys->joystick_type) {
                case C64_JOYSTICKTYPE_DIGITAL_1:
                    sys->kbd_joy1_mask |= m;
                    break;
                case C64_JOYSTICKTYPE_DIGITAL_2:
                    sys->kbd_joy2_mask |= m;
                    break;
                case C64_JOYSTICKTYPE_DIGITAL_12:
                    sys->kbd_joy1_mask |= m;
                    sys->kbd_joy2_mask |= m;
                    break;
                default: break;
            }
        }
    }
}

void c64_key_up(c64_t* sys, int key_code) {
    CHIPS_ASSERT(sys && sys->valid);
    if (sys->joystick_type == C64_JOYSTICKTYPE_NONE) {
        kbd_key_up(&sys->kbd, key_code);
    }
    else {
        uint8_t m = 0;
        switch (key_code) {
            case 0x20: m = C64_JOYSTICK_BTN; break;
            case 0x08: m = C64_JOYSTICK_LEFT; break;
            case 0x09: m = C64_JOYSTICK_RIGHT; break;
            case 0x0A: m = C64_JOYSTICK_DOWN; break;
            case 0x0B: m = C64_JOYSTICK_UP; break;
            default: kbd_key_up(&sys->kbd, key_code); break;
        }
        if (m != 0) {
            switch (sys->joystick_type) {
                case C64_JOYSTICKTYPE_DIGITAL_1:
                    sys->kbd_joy1_mask &= ~m;
                    break;
                case C64_JOYSTICKTYPE_DIGITAL_2:
                    sys->kbd_joy2_mask &= ~m;
                    break;
                case C64_JOYSTICKTYPE_DIGITAL_12:
                    sys->kbd_joy1_mask &= ~m;
                    sys->kbd_joy2_mask &= ~m;
                    break;
                default: break;
            }
        }
    }
}

void c64_set_joystick_type(c64_t* sys, c64_joystick_type_t type) {
    CHIPS_ASSERT(sys && sys->valid);
    sys->joystick_type = type;
}

c64_joystick_type_t c64_joystick_type(c64_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    return sys->joystick_type;
}

void c64_joystick(c64_t* sys, uint8_t joy1_mask, uint8_t joy2_mask) {
    CHIPS_ASSERT(sys && sys->valid);
    sys->joy_joy1_mask = joy1_mask;
    sys->joy_joy2_mask = joy2_mask;
}

bool c64_quickload(c64_t* sys, chips_range_t data) {
    CHIPS_ASSERT(sys && sys->valid && data.ptr);
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

    // update the BASIC pointers
    mem_wr16(&sys->mem_cpu, 0x2d, end_addr);
    mem_wr16(&sys->mem_cpu, 0x2f, end_addr);
    mem_wr16(&sys->mem_cpu, 0x31, end_addr);
    mem_wr16(&sys->mem_cpu, 0x33, end_addr);
    mem_wr16(&sys->mem_cpu, 0xae, end_addr);

    return true;
}

bool c64_insert_tape(c64_t* sys, chips_range_t data) {
    CHIPS_ASSERT(sys && sys->valid && sys->c1530.valid);
    return c1530_insert_tape(&sys->c1530, data);
}

void c64_remove_tape(c64_t* sys) {
    CHIPS_ASSERT(sys && sys->valid && sys->c1530.valid);
    c1530_remove_tape(&sys->c1530);
}

bool c64_tape_inserted(c64_t* sys) {
    CHIPS_ASSERT(sys && sys->valid && sys->c1530.valid);
    return c1530_tape_inserted(&sys->c1530);
}

void c64_tape_play(c64_t* sys) {
    CHIPS_ASSERT(sys && sys->valid && sys->c1530.valid);
    c1530_play(&sys->c1530);
}

void c64_tape_stop(c64_t* sys) {
    CHIPS_ASSERT(sys && sys->valid && sys->c1530.valid);
    c1530_stop(&sys->c1530);
}

bool c64_is_tape_motor_on(c64_t* sys) {
    CHIPS_ASSERT(sys && sys->valid && sys->c1530.valid);
    return c1530_is_motor_on(&sys->c1530);
}

chips_display_info_t c64_display_info(c64_t* sys) {
    chips_display_info_t res = {
        .frame = {
            .dim = {
                .width = M6569_FRAMEBUFFER_WIDTH,
                .height = M6569_FRAMEBUFFER_HEIGHT,
            },
            .bytes_per_pixel = 1,
            .buffer = {
                .ptr = sys ? sys->fb : 0,
                .size = M6569_FRAMEBUFFER_SIZE_BYTES,
            }
        },
        .palette = m6569_dbg_palette(),
    };
    if (sys) {
        res.screen = m6569_screen(&sys->vic);
    }
    else {
        res.screen = (chips_rect_t){
            .x = 0,
            .y = 0,
            .width = _C64_SCREEN_WIDTH,
            .height = _C64_SCREEN_HEIGHT
        };
    };
    CHIPS_ASSERT(((sys == 0) && (res.frame.buffer.ptr == 0)) || ((sys != 0) && (res.frame.buffer.ptr != 0)));
    return res;
}

uint32_t c64_save_snapshot(c64_t* sys, c64_t* dst) {
    CHIPS_ASSERT(sys && dst);
    *dst = *sys;
    chips_debug_snapshot_onsave(&dst->debug);
    chips_audio_callback_snapshot_onsave(&dst->audio.callback);
    m6502_snapshot_onsave(&dst->cpu);
    m6569_snapshot_onsave(&dst->vic);
    mem_snapshot_onsave(&dst->mem_cpu, sys);
    mem_snapshot_onsave(&dst->mem_vic, sys);
    c1530_snapshot_onsave(&dst->c1530);
    c1541_snapshot_onsave(&dst->c1541, sys);
    return C64_SNAPSHOT_VERSION;
}

bool c64_load_snapshot(c64_t* sys, uint32_t version, c64_t* src) {
    CHIPS_ASSERT(sys && src);
    if (version != C64_SNAPSHOT_VERSION) {
        return false;
    }
    static c64_t im;
    im = *src;
    chips_debug_snapshot_onload(&im.debug, &sys->debug);
    chips_audio_callback_snapshot_onload(&im.audio.callback, &sys->audio.callback);
    m6502_snapshot_onload(&im.cpu, &sys->cpu);
    m6569_snapshot_onload(&im.vic, &sys->vic);
    mem_snapshot_onload(&im.mem_cpu, sys);
    mem_snapshot_onload(&im.mem_vic, sys);
    c1530_snapshot_onload(&im.c1530, &sys->c1530);
    c1541_snapshot_onload(&im.c1541, &sys->c1541, sys);
    *sys = im;
    return true;
}

void c64_basic_run(c64_t* sys) {
    CHIPS_ASSERT(sys);
    // write RUN into the keyboard buffer
    uint16_t keybuf = 0x277;
    mem_wr(&sys->mem_cpu, keybuf++, 'R');
    mem_wr(&sys->mem_cpu, keybuf++, 'U');
    mem_wr(&sys->mem_cpu, keybuf++, 'N');
    mem_wr(&sys->mem_cpu, keybuf++, 0x0D);
    // write number of characters, this kicks off evaluation
    mem_wr(&sys->mem_cpu, 0xC6, 4);
}

void c64_basic_load(c64_t* sys) {
    CHIPS_ASSERT(sys);
    // write LOAD
    uint16_t keybuf = 0x277;
    mem_wr(&sys->mem_cpu, keybuf++, 'L');
    mem_wr(&sys->mem_cpu, keybuf++, 'O');
    mem_wr(&sys->mem_cpu, keybuf++, 'A');
    mem_wr(&sys->mem_cpu, keybuf++, 'D');
    mem_wr(&sys->mem_cpu, keybuf++, 0x0D);
    // write number of characters, this kicks off evaluation
    mem_wr(&sys->mem_cpu, 0xC6, 5);
}

void c64_basic_syscall(c64_t* sys, uint16_t addr) {
    CHIPS_ASSERT(sys);
    // write SYS xxxx[Return] into the keyboard buffer (up to 10 chars)
    uint16_t keybuf = 0x277;
    mem_wr(&sys->mem_cpu, keybuf++, 'S');
    mem_wr(&sys->mem_cpu, keybuf++, 'Y');
    mem_wr(&sys->mem_cpu, keybuf++, 'S');
    mem_wr(&sys->mem_cpu, keybuf++, ((addr / 10000) % 10) + '0');
    mem_wr(&sys->mem_cpu, keybuf++, ((addr / 1000) % 10) + '0');
    mem_wr(&sys->mem_cpu, keybuf++, ((addr / 100) % 10) + '0');
    mem_wr(&sys->mem_cpu, keybuf++, ((addr / 10) % 10) + '0');
    mem_wr(&sys->mem_cpu, keybuf++, ((addr / 1) % 10) + '0');
    mem_wr(&sys->mem_cpu, keybuf++, 0x0D);
    // write number of characters, this kicks off evaluation
    mem_wr(&sys->mem_cpu, 0xC6, 9);
}

uint16_t c64_syscall_return_addr(void) {
    return 0xA7EA;
}
#endif /* CHIPS_IMPL */
