#pragma once
/*
    kc85.h -- a KC85/2, /3 and /4 emulator in a C header.

    Do this:

    #define CHIPS_IMPL

    before you include this file in *one* C file to create the
    implementation.

    Define the KC85 type to build before including this header (both the
    declaration and implementation):

        CHIPS_KC85_TYPE_2
        CHIPS_KC85_TYPE_3
        CHIPS_KC85_TYPE_4

    Optionally provide the following macros with your own implementation

    CHIPS_ASSERT(c)
        your own assert macro (default: assert(c))

    You need to include the following headers before including kc85.h:

    - chips/chips_common.h
    - chips/z80.h
    - chips/z80ctc.h
    - chips/z80pio.h
    - chips/beeper.h
    - chips/kbd.h
    - chips/mem.h
    - chips/clk.h

    ## The KC85/2

    This was the ur-model of the KC85 family designed and manufactured
    by VEB Mikroelektronikkombinat Muehlhausen. The KC85/2 was introduced
    in 1984 as HC-900, and renamed to KC85/2 in 1985 (at the same time
    when the completely unrelated Z9001 was renamed to KC85/1).

        - U880 CPU @ 1.75 MHz (the U880 was an "unlicensed" East German Z80 clone)
        - 1x U855 (clone of the Z80-PIO)
        - 1x U857 (clone of the Z80-CTC)
        - 16 KByte RAM at 0000..3FFF
        - 16 KByte video RAM (IRM) at 8000..BFFF
        - 4 KByte ROM in 2 sections (E000..E800 and F000..F800)
        - the operating system was called CAOS (Cassette Aided Operating System)
        - 50 Hz PAL video at 320x256 pixels
        - Speccy-like color attributes (1 color byte per 8x4 pixels)
        - fixed palette of 16 foreground and 8 background colors
        - square-wave-beeper sound
        - separate keyboard with a serial-encoder chip to transfer
          key strokes to the main unit
        - flexible expansion module system (2 slots in the base units,
          4 additional slots per 'BUSDRIVER' units)
        - a famously bizarre video memory layout, consisting of a
          256x256 chunk on the left, and a separate 64x256 chunk on the right,
          with vertically 'interleaved' vertical addressing similar to the
          ZX Spectrum but with different offsets

    ### Memory Map:
        - 0000..01FF:   OS variables, interrupt vectors, and stack
        - 0200..3FFF:   usable RAM
        - 8000..A7FF:   pixel video RAM (1 byte => 8 pixels)
        - A800..B1FF:   color video RAM (1 byte => 8x4 color attribute block)
        - B200..B6FF:   ASCII backing buffer
        - B700..B77F:   cassette tape buffer
        - B780..B8FF:   more OS variables
        - B800..B8FF:   backing buffer for expansion module control bytes
        - B900..B97F:   buffer for actions assigned to function keys
        - B980..B9FF:   window attributes buffers
        - BA00..BBFF:   "additional programs"
        - BC00..BFFF:   usable 'slow-RAM'
        - E000..E7FF:   2 KB ROM
        - F000..F7FF:   2 KB ROM

        The video memory from A000..BFFF has slow CPU access (2.4us) because
        it needs to share memory accesses with the video system. Also, CPU
        accesses to this RAM block are visible as 'display needling' artefacts.

        (NOTE: the slow video memory access is not emulation, display needling
        is emulated, but I haven't verified against real hardware
        whether it actually looks correct)

    ### Special Operating System Conditions

        - the index register IX is reserved for operating system use
          and must not be changed while interrupts are enabled
        - only interrupt mode IM2 is supported

    ### Interrupt Vectors:
        - 01E4:     PIO-A (cassette tape input)
        - 01E6:     PIO-B (keyboard input)
        - 01E8:     CTC-0 (free)
        - 01EA:     CTC-1 (cassette tape output)
        - 01EC:     CTC-2 (timer interrupt used for sound length)

    ## IO Port Map:
        - 80:   Expansion module control (OUT: write module control byte,
                IN: read module id in slot). The upper 8 bits on the
                address bus identify the module slot (in the base
                unit the two slot addresses are 08 and 0C).
        - 88:   PIO port A, data
        - 89:   PIO port B, data
        - 8A:   PIO port A, control
        - 8B:   PIO port B, control
        - 8C:   CTC channel 0
        - 8D:   CTC channel 1
        - 8E:   CTC channel 2
        - 8F:   CTC channel 3

        The PIO port A and B bits are used to control bank switching and
        other hardware features:

        - PIO-A:
            - bit 0:    switch ROM at E000..FFFF on/off
            - bit 1:    switch RAM at 0000..3FFF on/off
            - bit 2:    switch video RAM (IRM) at 8000..BFFF on/off
            - bit 3:    write-protect RAM at 0000
            - bit 4:    unused
            - bit 5:    switch the front-plate LED on/off
            - bit 6:    cassette tape player motor control
            - bit 7:    expansion ROM at C000 on/off
        - PIO-B:
            - bits 0..4:    sound volume (currently not implemented)
            - bits 5/6:     unused
            - bit 7:        enable/disable the foreground-color blinking

        The CTC channels are used for sound frequency and other timing tasks:

        - CTC-0:    sound output (left?)
        - CTC-1:    sound output (right?)
        - CTC-2:    foreground color blink frequency, timer for cassette input
        - CTC-3:    timer for keyboard input

    ## The Module System:

    The emulator supports the most common RAM- and ROM-modules,
    but doesn't emulate special-hardware modules like the V24 or
    A/D converter module.

    The module system works with 4 byte values:

    - The **slot address**, the two base unit slots are at address 08 and 0C
    - The **module id**, this is a fixed value that identifies a module type.
      All 16 KByte ROM application modules had the same id.
      The module id can be queried by reading from port 80, with the
      slot address in the upper 8 bit of the 16-bit port address (so
      to query what module is in slot C, you would do an IN A,(C),
      with the value 0C80 in BC). If no module is in the slot, the value
      FF would be written to A, otherwise the module's id byte.
    - The module's **address mask**, this is a byte value that's ANDed
      against the upper 8 address bytes when mapping the module to memory,
      this essentially clamps a module's address to a 'round' 8- or
      16 KByte value (these are the 2 values I've seen in the wild)
    - The module control byte, this controls whether a module is currently
      active (bit 0), write-protected (bit 1), and at what address the
      module is mapped into the 16-bit address space (upper 3 bits)

    The module system is controlled with the SWITCH command, for instance
    the following command would map a ROM module in slot 8 to address
    C000:

        SWITCH 8 C1

    A RAM module in slot 0C mapped to address 4000:

        SWITCH C 43

    If you want to write-protect the RAM:

        SWITCH C 41

    ## The KC85/3

    The KC85/3 had the same hardware as the KC85/2 but came with a builtin
    8 KByte BASIC ROM at address C000..DFFF, and the OS was bumped to
    CAOS 3.1, now taking up a full 8 KBytes. Despite being just a minor
    update to the KC85/2, the KC85/3 was (most likely) the most popular
    model of the KC85/2 family.

    ## The KC85/4

    The KC85/4 was a major upgrade to the KC85/2 hardware architecture:

    - 64 KByte usable RAM
    - 64 KByte video RAM split up into 4 16-KByte banks
    - 20 KByte ROM (8 KByte BASIC, and 8+4 KByte OS)
    - Improved color attribute resolution (8x1 pixels instead of 8x4)
    - An additional per-pixel color mode which allowed to assign each
      individual pixel one of 4 hardwired colors at full 320x256
      resolution, this was realized by using 1 bit from the
      pixel-bank and the other bit from the color-bank, so setting
      one pixel required 2 memory accesses and a bank switch. Maybe
      this was the reason why this mode was hardly used.
    - Improved '90-degree-rotated' video memory layout, the 320x256
      pixel video memory was organized as 40 vertical stacks of 256 bytes,
      and the entire video memory was linear, this was perfectly suited
      to the Z80's 8+8 bit register pairs. The upper 8-bit register
      (for instance H) would hold the 'x coordinate' (columns 0 to 39),
      and the lower 8-bit register (L) the y coordinate (lines 0 to 255).
    - 64 KByte video memory was organized into 4 16-KByte banks, 2 banks
      for pixels, and 2 banks for colors. One pixel+color bank pair could
      be displayed while the other could be accessed by the CPU, this enabled
      true hardware double-buffering (unfortunately everything else was
      hardwired, so things like hardware-scrolling were not possible).

    The additional memory bank switching options were realized through
    previously unused bits in the PIO A/B ports, and 2 additional
    write-only 8-bit latches at port address 84 and 86:

    New bits in PIO port B:
        - bit 5:    enable the 2 stacked RAM banks at address 8000
        - bit 6:    write protect RAM bank at address 8000

    Output port 84:
        - bit 0:    select  the pixel/color bank pair 0 or 1 for display
        - bit 1:    select the pixel (0) or color bank (1) for CPU access
        - bit 2:    select the pixel/color bank pair 0 or 1 for CPU access
        - bit 3:    active the per-pixel-color-mode
        - bit 4:    select one of two RAM banks at address 8000
        - bit 5:    ??? (the docs say "RAM Block Select Bit for RAM8")
        - bits 6/7: unused

    Output port 86:
        - bit 0:        enable the 16K RAM bank at address 4000
        - bit 1:        write-protection for for RAM bank at address 4000
        - bits 2..6:    unused
        - bit 7:        enable the 4 KByte CAOS ROM bank at C000

    ## TODO:

    - optionally proper keyboard emulation (the current implementation
      uses a shortcut to directly write the key code into a memory address)
    - wait states for video RAM access

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

#if !(defined(CHIPS_KC85_TYPE_2) || defined(CHIPS_KC85_TYPE_3) || defined(CHIPS_KC85_TYPE_4))
#error "Please define one of CHIPS_KC85_TYPE_2, CHIPS_KC85_TYPE_3 or CHIPS_KC85_TYPE_4 before including kc85.h!"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined(CHIPS_KC85_TYPE_2)
#define KC85_TYPE_ID (0x00020000)
#elif defined(CHIPS_KC85_TYPE_3)
#define KC85_TYPE_ID (0x00030000)
#elif defined(CHIPS_KC85_TYPE_4)
#define KC85_TYPE_ID (0x00040000)
#endif

#if defined(CHIPS_KC85_TYPE_4)
#define KC85_FREQUENCY (1770000)
#define KC85_SCANLINE_TICKS (113)
#else
#define KC85_FREQUENCY (1750000)
#define KC85_SCANLINE_TICKS (112)
#endif
#define KC85_NUM_SCANLINES (312)
#define KC85_IRM0_PAGE (4)

// bump this whenever the kc85_t struct layout changes
#define KC85_SNAPSHOT_VERSION (KC85_TYPE_ID | 0x0002)

#define KC85_MAX_AUDIO_SAMPLES (1024U)      // max number of audio samples in internal sample buffer
#define KC85_DEFAULT_AUDIO_SAMPLES (128)    // default number of samples in internal sample buffer
#define KC85_EXP_NUM_SLOTS (2U)             // 2 expansion slots in main unit, each needs one mem_t layer!
#define KC85_EXP_BUFSIZE (KC85_EXP_NUM_SLOTS*64U*1024U) // expansion system buffer size (64 KB per slot)

#define KC85_FRAMEBUFFER_WIDTH (512)   // multiple of 256
#define KC85_FRAMEBUFFER_HEIGHT (256)  // FIXME: allow border?
#define KC85_FRAMEBUFFER_SIZE_BYTES (KC85_FRAMEBUFFER_WIDTH * KC85_FRAMEBUFFER_HEIGHT)
#define KC85_DISPLAY_WIDTH (320)
#define KC85_DISPLAY_HEIGHT (256)

// IO bits
#define KC85_PIO_CAOS_ROM        Z80PIO_PA0
#define KC85_PIO_RAM             Z80PIO_PA1
#define KC85_PIO_IRM             Z80PIO_PA2
#define KC85_PIO_RAM_RO          Z80PIO_PA3
#define KC85_PIO_NMI             Z80PIO_PA4   // KC84/2,3 only: trigger an NMI
#define KC85_PIO_TAPE_LED        Z80PIO_PA5
#define KC85_PIO_TAPE_MOTOR      Z80PIO_PA6
#define KC85_PIO_BASIC_ROM       Z80PIO_PA7
#define KC85_PIO_RAM8            Z80PIO_PB5  // KC85/4 only
#define KC85_PIO_RAM8_RO         Z80PIO_PB6  // KC85/4 only
#define KC85_PIO_BLINK_ENABLED   Z80PIO_PB7

// KC85/4 only IO latches
#define KC85_IO84_SEL_VIEW_IMG     (1<<0)  // 0: display img0, 1: display img1
#define KC85_IO84_SEL_CPU_COLOR    (1<<1)  // 0: access pixels, 1: access colors
#define KC85_IO84_SEL_CPU_IMG      (1<<2)  // 0: access img0, 1: access img1
#define KC85_IO84_HICOLOR          (1<<3)  // 0: hicolor mode off, 1: hicolor mode on
#define KC85_IO84_SEL_RAM8         (1<<4)  // select RAM8 block 0 or 1
#define KC85_IO84_BLOCKSEL_RAM8    (1<<5)  // no idea what that does...?
#define KC85_IO86_RAM4             (1<<0)
#define KC85_IO86_RAM4_RO          (1<<1)
#define KC85_IO86_CAOS_ROM_C       (1<<7)

// PIO and IO latch bit masks which affect the memory mapping
#define KC85_PIO_MEMORY_BITS  (KC85_PIO_CAOS_ROM|KC85_PIO_RAM|KC85_PIO_IRM|KC85_PIO_RAM_RO|KC85_PIO_BASIC_ROM|KC85_PIO_RAM8|KC85_PIO_RAM8_RO)
#define KC85_IO84_MEMORY_BITS   (KC85_IO84_SEL_CPU_COLOR|KC85_IO84_SEL_CPU_IMG|KC85_IO84_SEL_RAM8)
#define KC85_IO86_MEMORY_BITS   (KC85_IO86_RAM4|KC85_IO86_RAM4_RO|KC85_IO86_CAOS_ROM_C)

// pin mask aliases
#define KC85_FLIPFLOP_BEEPER_1  (Z80CTC_ZCTO0)
#define KC85_FLIPFLOP_BEEPER_2  (Z80CTC_ZCTO1)
#define KC85_FLIPFLOP_BLINK     (Z80CTC_ZCTO2)

// callback to apply patches after a snapshot is loaded
typedef struct {
    void (*func)(const char* snapshot_name, void* user_data);
    void* user_data;
} kc85_patch_callback_t;

// config parameters for kc85_init()
typedef struct {
    chips_debug_t debug;
    chips_audio_desc_t audio;

    // an optional callback to be invoked after a snapshot file is loaded to apply patches
    kc85_patch_callback_t patch_callback;

    // ROM images
    struct {
        #if defined(CHIPS_KC85_TYPE_2)
            chips_range_t caos22;    // CAOS 2.2 (used in KC85/2)
        #elif defined(CHIPS_KC85_TYPE_3)
            chips_range_t caos31;    // CAOS 3.1 (used in KC85/3)
        #elif defined(CHIPS_KC85_TYPE_4)
            chips_range_t caos42c;   // CAOS 4.2 at 0xC000 (KC85/4)
            chips_range_t caos42e;   // CAOS 4.2 at 0xE000 (KC85/4)
        #endif
        #if defined(CHIPS_KC85_TYPE_3) || defined(CHIPS_KC85_TYPE_4)
            chips_range_t kcbasic;   // same BASIC version for KC85/3 and KC85/4
        #endif
    } roms;
} kc85_desc_t;

// KC85 expansion module types
typedef enum {
    KC85_MODULE_NONE = 0,
    KC85_MODULE_M006_BASIC,         // BASIC+CAOS 16K ROM module for the KC85/2 (id=0xFC)
    KC85_MODULE_M011_64KBYE,        // 64 KByte RAM expansion (id=0xF6)
    KC85_MODULE_M012_TEXOR,         // TEXOR text editing (id=0xFB)
    KC85_MODULE_M022_16KBYTE,       // 16 KByte RAM expansion (id=0xF4)
    KC85_MODULE_M026_FORTH,         // FORTH IDE (id=0xFB)
    KC85_MODULE_M027_DEVELOPMENT,   // Assembler IDE (id=0xFB)

    KC85_MODULE_NUM,
} kc85_module_type_t;

// KC85 expansion module attributes
typedef struct {
    kc85_module_type_t type;
    uint8_t id;                     // id of currently inserted module
    bool writable;                  // RAM or ROM module
    uint8_t addr_mask;              // the module's address mask
    uint32_t size;                  // the module's byte size
} kc85_module_t;

// KC85 expansion slot
typedef struct {
    uint8_t addr;                   // 0x0C (left slot) or 0x08 (right slot)
    uint8_t ctrl;                   // current control byte
    uint32_t buf_offset;            // byte-offset in expansion system data buffer
    kc85_module_t mod;              // attributes of currently inserted module
} kc85_slot_t;

/* KC85 expansion system state:
    NOTE that each expansion slot needs its own memory-mapping layer starting
    at layer 1 (layer 0 is used by the base system)
*/
typedef struct {
    kc85_slot_t slot[KC85_EXP_NUM_SLOTS];   // slots 0x08 and 0x0C in KC85 main unit
    uint32_t buf_top;                       // offset of free area in expansion buffer (kc85_t.exp_buf[])
} kc85_exp_t;

// KC85 emulator state
typedef struct {
    z80_t cpu;
    mem_t mem;
    struct {
        uint16_t h_tick;
        uint16_t v_count;
    } video;
    uint64_t pio_pins;
    #if defined(CHIPS_KC85_TYPE_4)
        uint8_t io84;           // byte latch at port 0x84, only on KC85/4
        uint8_t io86;           // byte latch at port 0x86, only on KC85/4
    #endif
    z80ctc_t ctc;
    uint64_t flip_flops;    // audio and blink flip flop bits controlled by CTC
    beeper_t beeper_1;
    beeper_t beeper_2;
    z80pio_t pio;
    kc85_exp_t exp;         // expansion module system

    uint64_t pins;
    uint64_t freq_hz;
    kbd_t kbd;

    bool valid;
    chips_debug_t debug;

    struct {
        chips_audio_callback_t callback;
        int num_samples;
        int sample_pos;
        float sample_buffer[KC85_MAX_AUDIO_SAMPLES];
    } audio;
    kc85_patch_callback_t patch_callback;

    uint8_t ram[8][0x4000];             // up to 8 16-KByte RAM banks
    #if defined(CHIPS_KC85_TYPE_3) || defined(CHIPS_KC85_TYPE_4)
        uint8_t rom_basic[0x2000];          // 8 KByte BASIC ROM (KC85/3 and /4 only)
    #endif
    #if defined(CHIPS_KC85_TYPE_4)
        uint8_t rom_caos_c[0x1000];         // 4 KByte CAOS ROM at 0xC000 (KC85/4 only)
    #endif
    uint8_t rom_caos_e[0x2000];         // 8 KByte CAOS ROM at 0xE000
    uint8_t exp_buf[KC85_EXP_BUFSIZE];  // expansion system RAM/ROM
    alignas(64) uint8_t fb[KC85_FRAMEBUFFER_SIZE_BYTES];
} kc85_t;

// initialize a new KC85 instance
void kc85_init(kc85_t* sys, const kc85_desc_t* desc);
// discard a KC85 instance
void kc85_discard(kc85_t* sys);
// reset a KC85 instance
void kc85_reset(kc85_t* sys);
// query information about display requirements, can be called with nullptr
chips_display_info_t kc85_display_info(kc85_t* sys);
// run KC85 emulation for a given number of microseconds, returns number of ticks executed
uint32_t kc85_exec(kc85_t* sys, uint32_t micro_seconds);
// send a key-down event
void kc85_key_down(kc85_t* sys, int key_code);
// send a key-up event
void kc85_key_up(kc85_t* sys, int key_code);
// insert a RAM module (slot must be 0x08 or 0x0C)
bool kc85_insert_ram_module(kc85_t* sys, uint8_t slot, kc85_module_type_t type);
// insert a ROM module (slot must be 0x08 or 0x0C)
bool kc85_insert_rom_module(kc85_t* sys, uint8_t slot, kc85_module_type_t type, chips_range_t rom_data);
// remove module in slot
bool kc85_remove_module(kc85_t* sys, uint8_t slot);
// get a descriptive module name by module type
const char* kc85_mod_name(kc85_module_type_t mod_type);
// get a short module name (M022...) by module type
const char* kc85_mod_short_name(kc85_module_type_t mod_type);
// lookup slot struct by slot address (0x08 or 0x0C)
kc85_slot_t* kc85_slot_by_addr(kc85_t* sys, uint8_t slot_addr);
// return true if a slot contains a module
bool kc85_slot_occupied(kc85_t* sys, uint8_t slot_addr);
// test if module in slot is currently mapped to CPU-visible memory
bool kc85_slot_cpu_visible(kc85_t* sys, uint8_t slot_addr);
// compute the current CPU address of module in slot (0 if no active module in slot)
uint16_t kc85_slot_cpu_addr(kc85_t* sys, uint8_t slot_addr);
// get byte-size of module in slot (0 if no module in slot)
uint32_t kc85_slot_mod_size(kc85_t* sys, uint8_t slot_addr);
// get descriptive name of module in slot ("NONE" if no module in slot)
const char* kc85_slot_mod_name(kc85_t* sys, uint8_t slot_addr);
// get short name of module slot slot
const char* kc85_slot_mod_short_name(kc85_t* sys, uint8_t slot_addr);
// get a slot's control byte
uint8_t kc85_slot_ctrl(kc85_t* sys, uint8_t slot_addr);
// test if data is a valid TAP file
bool kc85_is_valid_kctap(chips_range_t data);
// test if data is a valid KCC file
bool kc85_is_valid_kcc(chips_range_t data);
// extract the exed address from a KCC file
uint16_t kc85_kcc_exec_addr(chips_range_t data);
// get the return address in ROM which is pushed on the stack for quickloaded code
uint16_t kc85_quickload_return_addr(void);
// load a .KCC or .TAP snapshot file into the emulator and optionally try to start
bool kc85_quickload(kc85_t* sys, chips_range_t data, bool start);
// take snapshot, patches any pointers to zero, returns a snapshot version
uint32_t kc85_save_snapshot(kc85_t* sys, kc85_t* dst);
// load a snapshot, returns false if snapshot version doesn't match
bool kc85_load_snapshot(kc85_t* sys, uint32_t version, const kc85_t* src);

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


/*
    IO address decoding.

    On the KC85/3, the chips-select signals for the CTC and PIO
    are generated through logic gates, on KC85/4 this is implemented
    with a PROM chip (details are in the KC85/3 and KC85/4 service manuals)

    the I/O addresses are as follows:

         0x88:   PIO Port A, data
         0x89:   PIO Port B, data
         0x8A:   PIO Port A, control
         0x8B:   PIO Port B, control
         0x8C:   CTC Channel 0
         0x8D:   CTC Channel 1
         0x8E:   CTC Channel 2
         0x8F:   CTC Channel 3

         0x80:   controls the expansion module system, the upper
                 8-bits of the port number address the module slot
         0x84:   (KC85/4 only) control the vide memory bank switching
         0x86:   (KC85/4 only) control RAM block at 0x4000 and ROM switching
*/
// IO area 0x80..0x8F
#define _KC85_IO_SEL_MASK (Z80_M1|Z80_IORQ|Z80_A7|Z80_A6|Z80_A5|Z80_A4)
#define _KC85_IO_SEL_PINS (Z80_IORQ|Z80_A7)
// CTC ports 0x8C..0x8F
#define _KC85_CTC_SEL_MASK (_KC85_IO_SEL_MASK|Z80_A3|Z80_A2)
#define _KC85_CTC_SEL_PINS (_KC85_IO_SEL_PINS|Z80_A3|Z80_A2)
// PIO ports 0x88..0x8B
#define _KC85_PIO_SEL_MASK (_KC85_IO_SEL_MASK|Z80_A3|Z80_A2)
#define _KC85_PIO_SEL_PINS (_KC85_IO_SEL_PINS|Z80_A3)
// expansion module system port 0x80
#define _KC85_EXP_SEL_MASK (_KC85_IO_SEL_MASK|Z80_A3|Z80_A2|Z80_A1|Z80_A0)
#define _KC85_EXP_SEL_PINS (_KC85_IO_SEL_PINS)
#if defined(CHIPS_KC85_TYPE_4)
// KC85/4 port 0x84
#define _KC85_IO84_SEL_MASK (_KC85_IO_SEL_MASK|Z80_A3|Z80_A2|Z80_A1|Z80_A0)
#define _KC85_IO84_SEL_PINS (_KC85_IO_SEL_PINS|Z80_A2)
// KC85/4 port 0x86
#define _KC85_IO86_SEL_MASK (_KC85_IO_SEL_MASK|Z80_A3|Z80_A2|Z80_A1|Z80_A0)
#define _KC85_IO86_SEL_PINS (_KC85_IO_SEL_PINS|Z80_A2|Z80_A1)
#endif

static void _kc85_update_memory_map(kc85_t* sys);
static void _kc85_init_memory_map(kc85_t* sys);
static void _kc85_handle_keyboard(kc85_t* sys);

// expansion module private functions
static void _kc85_exp_init(kc85_t* sys);
static void _kc85_exp_reset(kc85_t* sys);
static bool _kc85_exp_write_ctrl(kc85_t* sys, uint8_t slot_addr, uint8_t ctrl_byte);
static uint8_t _kc85_exp_module_id(kc85_t* sys, uint8_t slot_addr);
static void _kc85_exp_update_memory_mapping(kc85_t* sys);

// xorshift randomness for memory initialization
static inline uint32_t _kc85_xorshift32(uint32_t x) {
    x ^= x<<13;
    x ^= x>>17;
    x ^= x<<5;
    return x;
}

#define _KC85_DEFAULT(val,def) (((val) != 0) ? (val) : (def))

void kc85_init(kc85_t* sys, const kc85_desc_t* desc) {
    CHIPS_ASSERT(sys && desc);
    if (desc->debug.callback.func) { CHIPS_ASSERT(desc->debug.stopped); }

    memset(sys, 0, sizeof(kc85_t));
    sys->valid = true;
    sys->freq_hz = KC85_FREQUENCY;
    sys->patch_callback = desc->patch_callback;
    sys->debug = desc->debug;

    // copy ROM images
    #if defined(CHIPS_KC85_TYPE_2)
        // KC85/2 only has an 8 KByte OS ROM
        CHIPS_ASSERT(desc->roms.caos22.ptr && (desc->roms.caos22.size == sizeof(sys->rom_caos_e)));
        memcpy(sys->rom_caos_e, desc->roms.caos22.ptr, sizeof(sys->rom_caos_e));
    #elif defined(CHIPS_KC85_TYPE_3)
        // KC85/3 has 8 KByte BASIC ROM and 8 KByte OS ROM
        CHIPS_ASSERT(desc->roms.kcbasic.ptr && (desc->roms.kcbasic.size == sizeof(sys->rom_basic)));
        memcpy(sys->rom_basic, desc->roms.kcbasic.ptr, sizeof(sys->rom_basic));
        CHIPS_ASSERT(desc->roms.caos31.ptr && (desc->roms.caos31.size == sizeof(sys->rom_caos_e)));
        memcpy(sys->rom_caos_e, desc->roms.caos31.ptr, sizeof(sys->rom_caos_e));
    #else
        // KC85/4 has 8 KByte BASIC ROM, and 2 OS ROMs (4 KB and 8 KB)
        CHIPS_ASSERT(desc->roms.kcbasic.ptr && (desc->roms.kcbasic.size == sizeof(sys->rom_basic)));
        memcpy(sys->rom_basic, desc->roms.kcbasic.ptr, sizeof(sys->rom_basic));
        CHIPS_ASSERT(desc->roms.caos42c.ptr && (desc->roms.caos42c.size == sizeof(sys->rom_caos_c)));
        memcpy(sys->rom_caos_c, desc->roms.caos42c.ptr, sizeof(sys->rom_caos_c));
        CHIPS_ASSERT(desc->roms.caos42e.ptr && (desc->roms.caos42e.size == sizeof(sys->rom_caos_e)));
        memcpy(sys->rom_caos_e, desc->roms.caos42e.ptr, sizeof(sys->rom_caos_e));
    #endif

    // fill RAM with noise (only KC85/2 and /3)
    #if !defined(CHIPS_KC85_TYPE_4)
        uint32_t r = 0x6D98302B;
        uint8_t* ptr = &sys->ram[0][0];
        for (size_t i = 0; i < sizeof(sys->ram);) {
            r = _kc85_xorshift32(r);
            ptr[i++] = r;
            ptr[i++] = (r>>8);
            ptr[i++] = (r>>16);
            ptr[i++] = (r>>24);
        }
    #endif

    // initialize the hardware
    z80_init(&sys->cpu);
    z80ctc_init(&sys->ctc);
    z80pio_init(&sys->pio);

    sys->audio.callback = desc->audio.callback;
    sys->audio.num_samples = _KC85_DEFAULT(desc->audio.num_samples, KC85_DEFAULT_AUDIO_SAMPLES);
    const beeper_desc_t beeper_desc = {
        .tick_hz = (int)sys->freq_hz,
        .sound_hz = _KC85_DEFAULT(desc->audio.sample_rate, 44100),
        .base_volume = _KC85_DEFAULT(desc->audio.volume, 0.6f),
    };
    beeper_init(&sys->beeper_1, &beeper_desc);
    beeper_init(&sys->beeper_2, &beeper_desc);

    // expansion module system
    _kc85_exp_init(sys);

    // initial memory map (must happen after expansion system)
    _kc85_init_memory_map(sys);

    /* the kbd_t helper functions are only used as a simple keystroke buffer,
       the KC85 doesn't have a typical keyboard matrix in the computer
       base unit, since the keyboard unit sent keystroke information
       in a serial-encoded signal to the base unit. Currently this
       serial encoding/decoding isn't emulated (it wasn't very reliable
       anyway and required slow typing).
    */
    kbd_init(&sys->kbd, 2);

    // execution on power-up starts at 0xF000
    sys->pins = z80_prefetch(&sys->cpu, 0xF000);
}

void kc85_discard(kc85_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    sys->valid = false;
}

void kc85_reset(kc85_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    z80_reset(&sys->cpu);
    z80ctc_reset(&sys->ctc);
    z80pio_reset(&sys->pio);
    beeper_reset(&sys->beeper_1);
    beeper_reset(&sys->beeper_2);
    #if defined(CHIPS_KC85_TYPE_4)
        sys->io84 = 0;
        sys->io86 = 0;
    #endif
    _kc85_exp_reset(sys);
    sys->pio_pins = KC85_PIO_RAM | KC85_PIO_RAM_RO | KC85_PIO_IRM | KC85_PIO_CAOS_ROM;
    _kc85_update_memory_map(sys);

    // execution after reset starts at 0xE000
    sys->pins = z80_prefetch(&sys->cpu, 0xE000);
}

#define CHIPS_KC85_USE_BIT_TWIDDLING (1)

static inline void _kc85_decode_8pixels(uint8_t* dst, uint8_t pixels, uint8_t colors) {
    /*
        select foreground- and background color:
        bit 7: blinking
        bits 6..3: foreground color
        bits 2..0: background color

        index 0 is background color, index 1 is foreground color

        NOTE: using 'bit twiddling' to compose spread the 8 pixels into an uint64_t
        and writing this as a single 64-bit values doesn't appear to result
        in a performance advantage, also see:

        https://www.godbolt.org/z/7dsW4v6vr

        (and this bit twiddling code is even still missing reversing the pixel bit order)
    */
    #if CHIPS_KC85_USE_BIT_TWIDDLING
        // courtesy of ryg: https://mastodon.gamedev.place/@rygorous/109531596140414988
        static const uint32_t lut32[16] = {
            0x00000000, 0xff000000, 0x00ff0000, 0xffff0000,
            0x0000ff00, 0xff00ff00, 0x00ffff00, 0xffffff00,
            0x000000ff, 0xff0000ff, 0x00ff00ff, 0xffff00ff,
            0x0000ffff, 0xff00ffff, 0x00ffffff, 0xffffffff,
        };
        const uint32_t colors32 = colors * 0x01010101u;
        const uint32_t bg32 = 0x10101010 | (colors32 & 0x07070707);
        const uint32_t fg32 = (colors32 >> 3) & 0x0F0F0F0F;
        const uint32_t xor32 = bg32 ^ fg32;
        uint32_t* dst32 = (uint32_t*)dst;
        dst32[0] = bg32 ^ (xor32 & lut32[pixels >> 4]);
        dst32[1] = bg32 ^ (xor32 & lut32[pixels & 0xf]);
    #else
        const uint8_t bg = 0x10 | (colors & 0x7);
        const uint8_t fg = (colors>>3) & 0xF;
        dst[0] = pixels & 0x80 ? fg : bg;
        dst[1] = pixels & 0x40 ? fg : bg;
        dst[2] = pixels & 0x20 ? fg : bg;
        dst[3] = pixels & 0x10 ? fg : bg;
        dst[4] = pixels & 0x08 ? fg : bg;
        dst[5] = pixels & 0x04 ? fg : bg;
        dst[6] = pixels & 0x02 ? fg : bg;
        dst[7] = pixels & 0x01 ? fg : bg;
    #endif
}

static inline uint64_t _kc85_update_raster_counters(kc85_t* sys, uint64_t pins) {
    sys->video.h_tick++;
    // feed '_h4' into CTC CLKTRG0 and 1, per scanline:
    //   0..31 ticks lo
    //  32..63 ticks hi
    //  64..95 ticks lo
    //  remainder: hi
    if (sys->video.h_tick & 0x20) {
        pins |= (Z80CTC_CLKTRG0 | Z80CTC_CLKTRG1);
    }
    // vertical blanking interval (/BI) active for the last 56 scanlines
    if (sys->video.v_count & 0x100) {
        pins |= (Z80CTC_CLKTRG2 | Z80CTC_CLKTRG3);
    }
    if (sys->video.h_tick == KC85_SCANLINE_TICKS) {
        sys->video.h_tick = 0;
        sys->video.v_count++;
        if (sys->video.v_count == KC85_NUM_SCANLINES) {
            sys->video.v_count = 0;
        }
    }
    return pins;
}

#if defined(CHIPS_KC85_TYPE_2) || defined(CHIPS_KC85_TYPE_3)
static uint64_t _kc85_tick_video(kc85_t* sys, uint64_t pins) {
    // every 2 CPU ticks, 8 pixels are decoded
    if (sys->video.h_tick & 1) {
        uint16_t x = sys->video.h_tick>>1;
        uint16_t y = sys->video.v_count;
        if ((y < 256) && (x < 40)) {
            uint32_t pixel_offset, color_offset;
            if (x & 0x20) {
                // right 64x256 area
                pixel_offset = 0x2000 + ((x&0x7) | (((y>>4)&0x3)<<3) | (((y>>2)&0x3)<<5) | ((y&0x3)<<7) | (((y>>6)&0x3)<<9));
                color_offset = 0x3000 + ((x&0x7) | (((y>>4)&0x3)<<3) | (((y>>2)&0x3)<<5) | (((y>>6)&0x3)<<7));
            }
            else {
                // left 256x256 area
                pixel_offset = x | (((y>>2)&0x3)<<5) | ((y&0x3)<<7) | (((y>>4)&0xF)<<9);
                color_offset = 0x2800 + (x | (((y>>2)&0x3f)<<5));
            }
            // cpu_access: emulate display needling on KC85/2 and /3, this happens when the
            // CPU accesses video memory, which will force the background color
            // a short duration
            //
            uint8_t color_bits = sys->ram[KC85_IRM0_PAGE][color_offset];
            bool fg_blank = 0 != (color_bits & (sys->flip_flops>>(Z80CTC_BIT_ZCTO2-7)) & (sys->pio_pins>>(Z80PIO_PIN_PB7-7)) & (1<<7));
            // same as (pins & Z80_WR) && (addr >= 0x8000) && (addr < 0xC000)
            bool cpu_access = (pins & (Z80_WR | 0xC000)) == (Z80_WR | 0x8000);
            uint8_t pixel_bits = (fg_blank || cpu_access) ? 0 : sys->ram[KC85_IRM0_PAGE][pixel_offset];
            uint8_t* dst = &(sys->fb[y*KC85_FRAMEBUFFER_WIDTH + x*8]);
            _kc85_decode_8pixels(dst, pixel_bits, color_bits);
        }
    }
    return _kc85_update_raster_counters(sys, pins);
}
#endif // KC85/2,/3

#if defined(CHIPS_KC85_TYPE_4)

static inline void _kc85_decode_hicolor_8pixels(uint8_t* dst, uint8_t p0, uint8_t p1) {
    /*
        KC85/4 "hicolor" mode
        Decode 8 pixels for the "HICOLOR" mode with 2-bits per-pixel color.
        p0 and p1 are the two bitplanes (taken from the pixel and color RAM
        bank). The color palette is hardwired.

        p0: 8 bits from first IRM page
        p1: 8 bits from second IRM page
    */
    // FIXME: come up with an alternative 'bit-twiddling hack'
    dst[0] = 0x20 | ((p0>>7)&1)|((p1>>6)&2);
    dst[1] = 0x20 | ((p0>>6)&1)|((p1>>5)&2);
    dst[2] = 0x20 | ((p0>>5)&1)|((p1>>4)&2);
    dst[3] = 0x20 | ((p0>>4)&1)|((p1>>3)&2);
    dst[4] = 0x20 | ((p0>>3)&1)|((p1>>2)&2);
    dst[5] = 0x20 | ((p0>>2)&1)|((p1>>1)&2);
    dst[6] = 0x20 | ((p0>>1)&1)|((p1>>0)&2);
    dst[7] = 0x20 | ((p0>>0)&1)|((p1<<1)&2);
}

static uint64_t _kc85_tick_video(kc85_t* sys, uint64_t pins) {
    // decode 8 pixels every second tick
    if (sys->video.h_tick & 1) {
        uint16_t x = sys->video.h_tick>>1;
        uint16_t y = sys->video.v_count;
        if ((y < 256) && (x < 40)) {
            size_t irm_index = (sys->io84 & 1) * 2;
            size_t offset = (x<<8) | y;
            uint8_t color_bits = sys->ram[KC85_IRM0_PAGE + irm_index + 1][offset];
            uint8_t* dst = &sys->fb[y * KC85_FRAMEBUFFER_WIDTH + x * 8];
            if (sys->io84 & KC85_IO84_HICOLOR) {
                // regular KC85/4 video mode
                bool fg_blank = 0 != (color_bits & (sys->flip_flops>>(Z80CTC_BIT_ZCTO2-7)) & (sys->pio_pins>>(Z80PIO_PIN_PB7-7)) & (1<<7));
                uint8_t pixel_bits = fg_blank ? 0 : sys->ram[KC85_IRM0_PAGE + irm_index][offset];
                _kc85_decode_8pixels(dst, pixel_bits, color_bits);
            }
            else {
                // hicolor mode
                uint8_t p0 = sys->ram[KC85_IRM0_PAGE + irm_index][offset];
                uint8_t p1 = color_bits;
                _kc85_decode_hicolor_8pixels(dst, p0, p1);
            }
        }
    }
    return _kc85_update_raster_counters(sys, pins);
}
#endif

static void _kc85_update_memory_map(kc85_t* sys) {
    mem_unmap_layer(&sys->mem, 0);
    const uint64_t pio_pins = sys->pio_pins;

    // all models have 16 KB builtin RAM at 0x0000 and 8 KB ROM at 0xE000
    if (pio_pins & KC85_PIO_RAM) {
        if (pio_pins & KC85_PIO_RAM_RO) {
            mem_map_ram(&sys->mem, 0, 0x0000, 0x4000, sys->ram[0]);
        }
        else {
            mem_map_rom(&sys->mem, 0, 0x0000, 0x4000, sys->ram[0]);
        }
    }
    if (pio_pins & KC85_PIO_CAOS_ROM) {
        mem_map_rom(&sys->mem, 0, 0xE000, 0x2000, sys->rom_caos_e);
    }

    // KC85/3 and KC85/4: builtin 8 KB BASIC ROM at 0xC000
    #if !defined(CHIPS_KC85_TYPE_2)
        if (pio_pins & KC85_PIO_BASIC_ROM) {
            mem_map_rom(&sys->mem, 0, 0xC000, 0x2000, sys->rom_basic);
        }
    #endif

    #if !defined(CHIPS_KC85_TYPE_4) // KC85/2 and /3
        // 16 KB Video RAM at 0x8000
        if (pio_pins & KC85_PIO_IRM) {
            mem_map_ram(&sys->mem, 0, 0x8000, 0x4000, sys->ram[KC85_IRM0_PAGE]);
        }
    #else // KC84/4
        // 16 KB RAM at 0x4000
        if (sys->io86 & KC85_IO86_RAM4) {
            if (sys->io86 & KC85_IO86_RAM4_RO) {
                mem_map_ram(&sys->mem, 0, 0x4000, 0x4000, sys->ram[1]);
            }
            else {
                mem_map_rom(&sys->mem, 0, 0x4000, 0x4000, sys->ram[1]);
            }
        }
        // 16 KB RAM at 0x8000 (2 banks)
        if (pio_pins & KC85_PIO_RAM8) {
            // select one of two RAM banks
            uint8_t* ram8_ptr = (sys->io84 & KC85_IO84_SEL_RAM8) ? sys->ram[3] : sys->ram[2];
            if (pio_pins & KC85_PIO_RAM8_RO) {
                mem_map_ram(&sys->mem, 0, 0x8000, 0x4000, ram8_ptr);
            }
            else {
                mem_map_rom(&sys->mem, 0, 0x8000, 0x4000, ram8_ptr);
            }
        }
        /* video memory is 4 banks, 2 for pixels, 2 for colors,
            the area at 0xA800 to 0xBFFF is always mapped to IRM0!
        */
        if (pio_pins & KC85_PIO_IRM) {
            uint32_t irm_index = (sys->io84 & 6)>>1;
            uint8_t* irm_ptr = sys->ram[KC85_IRM0_PAGE + irm_index];
            /* on the KC85, an access to IRM banks other than the
              first is only possible for the first 10 KByte until
              A800, memory access to the remaining 6 KBytes
              (A800 to BFFF) is always forced to the first IRM bank
              by the address decoder hardware (see KC85/4 service manual)
            */
            mem_map_ram(&sys->mem, 0, 0x8000, 0x2800, irm_ptr);

            // always force access to 0xA800 and above to the first IRM bank
            mem_map_ram(&sys->mem, 0, 0xA800, 0x1800, sys->ram[KC85_IRM0_PAGE] + 0x2800);
       }
       // 4 KB CAOS-C ROM at 0xC000 (on top of BASIC)
       if (sys->io86 & KC85_IO86_CAOS_ROM_C) {
           mem_map_rom(&sys->mem, 0, 0xC000, 0x1000, sys->rom_caos_c);
       }
    #endif // KC85/4

    // let the module system update it's memory mapping
    _kc85_exp_update_memory_mapping(sys);
}

static uint64_t _kc85_tick(kc85_t* sys, uint64_t pins) {
    // tick the CPU
    pins = z80_tick(&sys->cpu, pins) & Z80_PIN_MASK;

    // handle memory requests
    if (pins & Z80_MREQ) {
        const uint16_t addr = Z80_GET_ADDR(pins);
        if (pins & Z80_RD) {
            Z80_SET_DATA(pins, mem_rd(&sys->mem, addr));
        }
        else if (pins & Z80_WR) {
            mem_wr(&sys->mem, addr, Z80_GET_DATA(pins));
        }
    }

    // tick the video system, may set CLKTRG0..3
    pins = _kc85_tick_video(sys, pins);

    // tick the CTC
    {
        // set virtual IEIO pin because CTC is highest priority interrupt device
        pins |= Z80_IEIO;
        if ((pins & _KC85_CTC_SEL_MASK) == _KC85_CTC_SEL_PINS) {
            pins |= Z80CTC_CE;
        }
        if (pins & Z80_A0) { pins |= Z80CTC_CS0; }
        if (pins & Z80_A1) { pins |= Z80CTC_CS1; }
        pins = z80ctc_tick(&sys->ctc, pins);
        // toggle audio and blink flip flops
        sys->flip_flops ^= pins;
        pins &= Z80_PIN_MASK;
    }

    // tick the PIO
    bool memory_mapping_dirty = false;
    {
        if ((pins & _KC85_PIO_SEL_MASK) == _KC85_PIO_SEL_PINS) {
            pins |= Z80PIO_CE;
        }
        if (pins & Z80_A0) { pins |= Z80PIO_BASEL; }
        if (pins & Z80_A1) { pins |= Z80PIO_CDSEL; }
        Z80PIO_SET_PAB(pins, 0xFF, 0xFF);
        pins = z80pio_tick(&sys->pio, pins);
        #if defined(CHIPS_KC85_TYPE_4)
            // volume and symmetry-flip-flop control on KC85/4
            if (((pins ^ sys->pio_pins)>>Z80PIO_PIN_PB1) & 0x0F) {
                // volume has changed
                float vol = ((~pins >> Z80PIO_PIN_PB1) & 0x0F) / 15.0f;
                beeper_set_volume(&sys->beeper_1, vol);
                beeper_set_volume(&sys->beeper_2, vol);
            }
            // PIO-B bit 0 cleared forces the audio beeper flip flop to low
            if (0 == (pins & Z80PIO_PB0)) {
                sys->flip_flops &= ~(KC85_FLIPFLOP_BEEPER_1|KC85_FLIPFLOP_BEEPER_2);
            }
        #else
            // on KC85/2 and /3, PA4 is connected to CPU NMI pin
            if (pins & KC85_PIO_NMI) { pins &= ~Z80_NMI; }
            else                     { pins |= Z80_NMI; }
        #endif
        memory_mapping_dirty |= ((pins^sys->pio_pins) & KC85_PIO_MEMORY_BITS);
        sys->pio_pins = pins;
        pins &= Z80_PIN_MASK;
    }

    // tick the audio beepers
    beeper_set(&sys->beeper_1, sys->flip_flops & KC85_FLIPFLOP_BEEPER_1);
    beeper_set(&sys->beeper_2, sys->flip_flops & KC85_FLIPFLOP_BEEPER_2);
    beeper_tick(&sys->beeper_1);
    if (beeper_tick(&sys->beeper_2)) {
        // new audio sample ready
        sys->audio.sample_buffer[sys->audio.sample_pos++] = sys->beeper_1.sample + sys->beeper_2.sample;
        if (sys->audio.sample_pos == sys->audio.num_samples) {
            if (sys->audio.callback.func) {
                sys->audio.callback.func(sys->audio.sample_buffer, sys->audio.num_samples, sys->audio.callback.user_data);
            }
            sys->audio.sample_pos = 0;
        }
    }

    // IO port 0x80: expansion module control, high byte of
    // port address contains module slot address
    if ((pins & _KC85_EXP_SEL_MASK) == _KC85_EXP_SEL_PINS) {
        const uint8_t slot_addr = pins>>Z80_PIN_A8;
        if (pins & Z80_WR) {
            // write new control byte and update the memory mapping
            const uint8_t data = Z80_GET_DATA(pins);
            memory_mapping_dirty |= _kc85_exp_write_ctrl(sys, slot_addr, data);
        }
        else if (pins & Z80_RD) {
            // read module id in slot
            Z80_SET_DATA(pins, _kc85_exp_module_id(sys, slot_addr));
        }
    }

    // KC85/4 ports 0x84 and 0x86, these are write-only 8-bit latches
    #if defined(CHIPS_KC85_TYPE_4)
    if ((pins & _KC85_IO84_SEL_MASK) == _KC85_IO84_SEL_PINS) {
        if (pins & Z80_WR) {
            const uint8_t data = Z80_GET_DATA(pins);
            memory_mapping_dirty |= (data ^ sys->io84) & KC85_IO84_MEMORY_BITS;
            sys->io84 = data;
        }
    }
    if ((pins & _KC85_IO86_SEL_MASK) == _KC85_IO86_SEL_PINS) {
        if (pins & Z80_WR) {
            const uint8_t data = Z80_GET_DATA(pins);
            memory_mapping_dirty |= (data ^ sys->io86) & KC85_IO86_MEMORY_BITS;
            sys->io86 = data;
        }
    }
    #endif

    if (memory_mapping_dirty) {
        _kc85_update_memory_map(sys);
    }
    return pins;
}

uint32_t kc85_exec(kc85_t* sys, uint32_t micro_seconds) {
    CHIPS_ASSERT(sys && sys->valid);
    const uint32_t num_ticks = clk_us_to_ticks(sys->freq_hz, micro_seconds);
    uint64_t pins = sys->pins;
    if (0 == sys->debug.callback.func) {
        // run without debug hook
        for (uint32_t tick = 0; tick < num_ticks; tick++) {
            pins = _kc85_tick(sys, pins);
        }
    }
    else {
        // run with debug hook
        for (uint32_t tick = 0; (tick < num_ticks) && !(*sys->debug.stopped); tick++) {
            pins = _kc85_tick(sys, pins);
            sys->debug.callback.func(sys->debug.callback.user_data, pins);
        }
    }
    sys->pins = pins;
    kbd_update(&sys->kbd, micro_seconds);
    _kc85_handle_keyboard(sys);
    return num_ticks;
}

static void _kc85_init_memory_map(kc85_t* sys) {
    mem_init(&sys->mem);
    sys->pio_pins = KC85_PIO_RAM | KC85_PIO_RAM_RO | KC85_PIO_IRM | KC85_PIO_CAOS_ROM;
    _kc85_update_memory_map(sys);
}

/*
    KEYBOARD INPUT

    this is a simplified version of the PIO-B interrupt service routine
    which is normally triggered when the serial keyboard hardware
    sends a new pulse (for details, see
    https://github.com/floooh/yakc/blob/master/misc/kc85_3_kbdint.md )

    we ignore the whole tricky serial decoding and patch the
    keycode directly into the right memory locations.
*/

#define _KC85_KBD_TIMEOUT (1<<3)
#define _KC85_KBD_KEYREADY (1<<0)
#define _KC85_KBD_REPEAT (1<<4)
#define _KC85_KBD_SHORT_REPEAT_COUNT (8)
#define _KC85_KBD_LONG_REPEAT_COUNT (60)

static void _kc85_handle_keyboard(kc85_t* sys) {
    // don't do anything if interrupts disabled, IX might point to the wrong base address!
    if (!sys->cpu.iff1) {
        return;
    }

    // get the first valid key code from the key buffer
    uint8_t key_code = 0;
    for (size_t i = 0; i < KBD_MAX_PRESSED_KEYS; i++) {
        if (sys->kbd.key_buffer[i].key != 0) {
            key_code = sys->kbd.key_buffer[i].key;
            break;
        }
    }

    const uint16_t ix = sys->cpu.ix;
    if (0 == key_code) {
        // if keycode is 0, this basically means the CTC3 timeout was hit
        mem_wr(&sys->mem, ix+0x8, mem_rd(&sys->mem, ix+0x8) | _KC85_KBD_TIMEOUT); /* set the CTC3 timeout bit */
        mem_wr(&sys->mem, ix+0xD, 0); /* clear current keycode */
    }
    else {
        // a valid keycode has been received, clear the timeout bit
        mem_wr(&sys->mem, ix+0x8, mem_rd(&sys->mem, ix+0x8) & ~_KC85_KBD_TIMEOUT);

        // check for key-repeat
        if (key_code != mem_rd(&sys->mem, ix+0xD)) {
            // no key-repeat
            mem_wr(&sys->mem, ix+0xD, key_code);                                     // write new keycode
            mem_wr(&sys->mem, ix+0x8, mem_rd(&sys->mem, ix+0x8)&~_KC85_KBD_REPEAT);  // clear the first-key-repeat bit
            mem_wr(&sys->mem, ix+0x8, mem_rd(&sys->mem, ix+0x8)|_KC85_KBD_KEYREADY); // set the key-ready bit
            mem_wr(&sys->mem, ix+0xA, 0);                                            // clear the key-repeat counter
        }
        else {
            // handle key-repeat
            mem_wr(&sys->mem, ix+0xA, mem_rd(&sys->mem, ix+0xA)+1);   // increment repeat-pause-counter
            if (mem_rd(&sys->mem, ix+0x8) & _KC85_KBD_REPEAT) {
                // this is a followup, short key-repeat
                if (mem_rd(&sys->mem, ix+0xA) < _KC85_KBD_SHORT_REPEAT_COUNT) {
                    // wait some more...
                    return;
                }
            }
            else {
                // this is the first, long key-repeat
                if (mem_rd(&sys->mem, ix+0xA) < _KC85_KBD_LONG_REPEAT_COUNT) {
                    // wait some more...
                    return;
                }
                else {
                    // first key-repeat pause over, set first-key-repeat flag
                    mem_wr(&sys->mem, ix+0x8, mem_rd(&sys->mem, ix+0x8)|_KC85_KBD_REPEAT);
                }
            }
            // key-repeat triggered, just set the key-ready flag and reset repeat-count
            mem_wr(&sys->mem, ix+0x8, mem_rd(&sys->mem, ix+0x8)|_KC85_KBD_KEYREADY);
            mem_wr(&sys->mem, ix+0xA, 0);
        }
    }
}

/*=== EXPANSION MODULE SUBSYSTEM =============================================*/
static void _kc85_exp_init(kc85_t* sys) {
    // assumes that the entire kc85_t struct was memory cleared already!
    CHIPS_ASSERT(2 == KC85_EXP_NUM_SLOTS);
    sys->exp.slot[0].addr = 0x08;
    sys->exp.slot[1].addr = 0x0C;
    for (size_t i = 0; i < KC85_EXP_NUM_SLOTS; i++) {
        sys->exp.slot[i].mod.id = 0xFF;
    }
}

static void _kc85_exp_reset(kc85_t* sys) {
    (void)sys;
    /* FIXME? */
}

const char* kc85_mod_name(kc85_module_type_t mod_type) {
    switch (mod_type) {
        case KC85_MODULE_NONE:          return "NONE";
        case KC85_MODULE_M006_BASIC:    return "M006 BASIC";
        case KC85_MODULE_M011_64KBYE:   return "M011 64KBYTE";
        case KC85_MODULE_M012_TEXOR:    return "M012 TEXOR";
        case KC85_MODULE_M022_16KBYTE:  return "M022 16KBYTE";
        case KC85_MODULE_M026_FORTH:    return "M026 FORTH";
        case KC85_MODULE_M027_DEVELOPMENT:  return "M027 DEV";
        default: return "???";
    }
}

const char* kc85_mod_short_name(kc85_module_type_t mod_type) {
    switch (mod_type) {
        case KC85_MODULE_NONE:          return "NONE";
        case KC85_MODULE_M006_BASIC:    return "M006";
        case KC85_MODULE_M011_64KBYE:   return "M011";
        case KC85_MODULE_M012_TEXOR:    return "M012";
        case KC85_MODULE_M022_16KBYTE:  return "M022";
        case KC85_MODULE_M026_FORTH:    return "M026";
        case KC85_MODULE_M027_DEVELOPMENT:  return "M027";
        default: return "???";
    }
}

kc85_slot_t* kc85_slot_by_addr(kc85_t* sys, uint8_t slot_addr) {
    CHIPS_ASSERT(sys && sys->valid);
    for (size_t i = 0; i < KC85_EXP_NUM_SLOTS; i++) {
        kc85_slot_t* slot = &sys->exp.slot[i];
        if (slot_addr == slot->addr) {
            return slot;
        }
    }
    return 0;
}

bool kc85_slot_occupied(kc85_t* sys, uint8_t slot_addr) {
    CHIPS_ASSERT(sys && sys->valid);
    kc85_slot_t* slot = kc85_slot_by_addr(sys, slot_addr);
    return slot && (slot->mod.type != KC85_MODULE_NONE);
}

bool kc85_slot_cpu_visible(kc85_t* sys, uint8_t slot_addr) {
    CHIPS_ASSERT(sys && sys->valid);
    kc85_slot_t* slot = kc85_slot_by_addr(sys, slot_addr);
    if (slot) {
        if (slot->mod.type != KC85_MODULE_NONE) {
            const uint16_t addr = (slot->ctrl & slot->mod.addr_mask) << 8;
            const uint8_t* mod_ptr = mem_readptr(&sys->mem, addr);
            const uint8_t* start_ptr = &sys->exp_buf[slot->buf_offset];
            const uint8_t* end_ptr = start_ptr + slot->mod.size;
            if ((mod_ptr >= start_ptr) && (mod_ptr < end_ptr)) {
                return true;
            }
        }
    }
    return false;
}

uint16_t kc85_slot_cpu_addr(kc85_t* sys, uint8_t slot_addr) {
    CHIPS_ASSERT(sys && sys->valid);
    kc85_slot_t* slot = kc85_slot_by_addr(sys, slot_addr);
    if (slot) {
        return (slot->ctrl & slot->mod.addr_mask) << 8;
    }
    else {
        return 0;
    }
}

uint32_t kc85_slot_mod_size(kc85_t* sys, uint8_t slot_addr) {
    CHIPS_ASSERT(sys && sys->valid);
    kc85_slot_t* slot = kc85_slot_by_addr(sys, slot_addr);
    if (slot) {
        return slot->mod.size;
    }
    else {
        return 0;
    }
}

const char* kc85_slot_mod_name(kc85_t* sys, uint8_t slot_addr) {
    CHIPS_ASSERT(sys && sys->valid);
    kc85_slot_t* slot = kc85_slot_by_addr(sys, slot_addr);
    if (slot) {
        return kc85_mod_name(slot->mod.type);
    }
    else {
        return "NONE";
    }
}

const char* kc85_slot_mod_short_name(kc85_t* sys, uint8_t slot_addr) {
    CHIPS_ASSERT(sys && sys->valid);
    kc85_slot_t* slot = kc85_slot_by_addr(sys, slot_addr);
    if (slot) {
        return kc85_mod_short_name(slot->mod.type);
    }
    else {
        return "NONE";
    }
}

uint8_t kc85_slot_ctrl(kc85_t* sys, uint8_t slot_addr) {
    CHIPS_ASSERT(sys && sys->valid);
    kc85_slot_t* slot = kc85_slot_by_addr(sys, slot_addr);
    if (slot) {
        return slot->ctrl;
    }
    else {
        return 0;
    }
}

/* allocate expansion buffer space for a module to be inserted into a slot
    updates:
        slot->buf_offset
        sys->exp.buf_top
*/
static bool _kc85_exp_alloc(kc85_t* sys, kc85_slot_t* slot) {
    // check if there's enough room
    if ((slot->mod.size + sys->exp.buf_top) > KC85_EXP_BUFSIZE) {
        return false;
    }
    // update offsets and sizes
    slot->buf_offset = sys->exp.buf_top;
    sys->exp.buf_top += slot->mod.size;
    return true;
}

/* free room in the expansion buffer, and close the hole
    updates:
        sys->exp.buf_top
        sys->exp_buf (any holes are merged)
        for each slot "behind" the to-be-freed slot:
            slot->buf_offset
*/
static void _kc85_exp_free(kc85_t* sys, kc85_slot_t* free_slot) {
    CHIPS_ASSERT(free_slot->mod.size > 0);
    const uint32_t bytes_to_free = free_slot->mod.size;
    CHIPS_ASSERT(sys->exp.buf_top >= bytes_to_free);
    sys->exp.buf_top -= bytes_to_free;
    for (size_t i = 0; i < KC85_EXP_NUM_SLOTS; i++) {
        kc85_slot_t* slot = &sys->exp.slot[i];
        // no module in slot: nothing to do
        if (slot->mod.type == KC85_MODULE_NONE) {
            continue;
        }
        // if slot is 'behind' the to-be-freed slot...
        if (slot->buf_offset > free_slot->buf_offset) {
            CHIPS_ASSERT(slot->buf_offset >= bytes_to_free);
            // move data backward to close the hole
            const uint8_t* from = &sys->exp_buf[slot->buf_offset];
            uint8_t* to = &sys->exp_buf[slot->buf_offset - bytes_to_free];
            memmove(to, from, bytes_to_free);
            slot->buf_offset -= bytes_to_free;
        }
    }
}

static bool _kc85_insert_module(kc85_t* sys, uint8_t slot_addr, kc85_module_type_t type, chips_range_t rom_data) {
    kc85_slot_t* slot = kc85_slot_by_addr(sys, slot_addr);
    if (!slot) {
        return false;
    }
    slot->mod.type = type;
    switch (type) {
        case KC85_MODULE_M006_BASIC:
            slot->mod.id = 0xFC;
            slot->mod.writable = false;
            slot->mod.addr_mask = 0xC0;
            slot->mod.size = 16*1024;
            break;
        case KC85_MODULE_M011_64KBYE:
            slot->mod.id = 0xF6;
            slot->mod.writable = true;
            slot->mod.addr_mask = 0xC0;
            slot->mod.size = 64*1024;
            break;
        case KC85_MODULE_M022_16KBYTE:
            slot->mod.id = 0xF4;
            slot->mod.writable = true;
            slot->mod.addr_mask = 0xC0;
            slot->mod.size = 16*1024;
            break;
        case KC85_MODULE_M012_TEXOR:
        case KC85_MODULE_M026_FORTH:
        case KC85_MODULE_M027_DEVELOPMENT:
            slot->mod.id = 0xFB;
            slot->mod.writable = false;
            slot->mod.addr_mask = 0xE0;
            slot->mod.size = 8*1024;
            break;
        default:
            CHIPS_ASSERT(false);
            break;
    }

    // allocate space in expansion buffer
    if (!_kc85_exp_alloc(sys, slot)) {
        // not enough space left in buffer
        slot->mod.type = KC85_MODULE_NONE;
        slot->mod.id = 0xFF;
        slot->mod.size = 0;
        return false;
    }

    // copy optional ROM image, or clear RAM
    if (rom_data.ptr) {
        if (rom_data.size != slot->mod.size) {
            return false;
        }
        memcpy(&sys->exp_buf[slot->buf_offset], rom_data.ptr, slot->mod.size);
    }
    else {
        memset(&sys->exp_buf[slot->buf_offset], 0, slot->mod.size);
    }

    // update the memory mapping
    _kc85_update_memory_map(sys);

    return true;
}

bool kc85_insert_ram_module(kc85_t* sys, uint8_t slot_addr, kc85_module_type_t type) {
    CHIPS_ASSERT(sys && sys->valid && (type != KC85_MODULE_NONE));
    kc85_remove_module(sys, slot_addr);
    return _kc85_insert_module(sys, slot_addr, type, (chips_range_t){0});
}

bool kc85_insert_rom_module(kc85_t* sys, uint8_t slot_addr, kc85_module_type_t type, chips_range_t rom_data) {
    CHIPS_ASSERT(sys && sys->valid && (type != KC85_MODULE_NONE));
    kc85_remove_module(sys, slot_addr);
    return _kc85_insert_module(sys, slot_addr, type, rom_data);
}

bool kc85_remove_module(kc85_t* sys, uint8_t slot_addr) {
    CHIPS_ASSERT(sys && sys->valid);
    kc85_slot_t* slot = kc85_slot_by_addr(sys, slot_addr);
    if (!slot) {
        return false;
    }
    // slot not occupied?
    if (slot->mod.type == KC85_MODULE_NONE) {
        CHIPS_ASSERT(slot->mod.id == 0xFF);
        CHIPS_ASSERT(slot->mod.size == 0);
        return false;
    }

    // first free the expansion buffer space
    _kc85_exp_free(sys, slot);

    // de-init the module attributes
    slot->mod.type = KC85_MODULE_NONE;
    slot->mod.id = 0xFF;
    slot->mod.writable = false;
    slot->mod.addr_mask = 0;
    slot->mod.size = 0;

    // update the memory mapping
    _kc85_update_memory_map(sys);

    return true;
}

static bool _kc85_exp_write_ctrl(kc85_t* sys, uint8_t slot_addr, uint8_t ctrl_byte) {
    kc85_slot_t* slot = kc85_slot_by_addr(sys, slot_addr);
    if (slot) {
        slot->ctrl = ctrl_byte;
        return true;
    }
    else {
        return false;
    }
}

static uint8_t _kc85_exp_module_id(kc85_t* sys, uint8_t slot_addr) {
    const kc85_slot_t* slot = kc85_slot_by_addr(sys, slot_addr);
    return slot ? slot->mod.id : 0xFF;
}

static void _kc85_exp_update_memory_mapping(kc85_t* sys) {
    for (size_t i = 0; i < KC85_EXP_NUM_SLOTS; i++) {
        const kc85_slot_t* slot = &sys->exp.slot[i];

        // nothing to do if no module in slot
        if (KC85_MODULE_NONE == slot->mod.type) {
            continue;
        }

        /* each slot gets its own memory system mapping layer,
           layer 0 is used by computer base unit
        */
        const size_t layer = i + 1;
        mem_unmap_layer(&sys->mem, layer);

        // module is only active if bit 0 in control byte is set
        if (slot->ctrl & 0x01) {
            // compute CPU- and host-address
            const uint16_t addr = (slot->ctrl & slot->mod.addr_mask) << 8;
            uint8_t* host_addr = &sys->exp_buf[slot->buf_offset];

            // RAM modules are only writable if bit 1 in control-byte is set
            const bool writable = (slot->ctrl & 0x02) && slot->mod.writable;
            if (writable) {
                mem_map_ram(&sys->mem, layer, addr, slot->mod.size, host_addr);
            }
            else {
                mem_map_rom(&sys->mem, layer, addr, slot->mod.size, host_addr);
            }
        }
    }
}

/*=== FILE LOADING ===========================================================*/

uint16_t kc85_quickload_return_addr(void) {
    // FIXME: KC85/2: find proper return address
    #if defined(CHIPS_KC85_TYPE_2)
        return 0xffff;
    #elif defined(CHIPS_KC85_TYPE_3)
        return 0xf15c;
    #elif defined(CHIPS_KC85_TYPE_4)
        return 0xf17e;
    #else
    #error "unknown KC85 type!"
    #endif
}

// common start function for all snapshot file formats
static void _kc85_load_start(kc85_t* sys, uint16_t exec_addr) {
    sys->cpu.a = 0x00;
    sys->cpu.f = 0x10;
    sys->cpu.bc = 0x0000; sys->cpu.bc2 = 0x0000;
    sys->cpu.de = 0x0000; sys->cpu.de2 = 0x0000;
    sys->cpu.hl = 0x0000; sys->cpu.hl2 = 0x0000;
    sys->cpu.af2 = 0x0000;
    sys->cpu.sp = 0x01C2;
    // delete ASCII buffer
    for (uint16_t addr = 0xb200; addr < 0xb700; addr++) {
        mem_wr(&sys->mem, addr, 0);
    }
    mem_wr(&sys->mem, 0xb7a0, 0);
    #if defined(CHIPS_KC85_TYPE_3)
        _kc85_tick(sys, Z80_MAKE_PINS(Z80_IORQ|Z80_WR, 0x89, 0x9f));
    #elif defined(CHIPS_KC85_TYPE_4)
        _kc85_tick(sys, Z80_MAKE_PINS(Z80_IORQ|Z80_WR, 0x89, 0xff));
    #endif
    mem_wr16(&sys->mem, sys->cpu.sp, kc85_quickload_return_addr());
    z80_prefetch(&sys->cpu, exec_addr);
}

/* KCC file format support */
typedef struct {
    uint8_t name[16];
    uint8_t num_addr;
    uint8_t load_addr_l;
    uint8_t load_addr_h;
    uint8_t end_addr_l;
    uint8_t end_addr_h;
    uint8_t exec_addr_l;
    uint8_t exec_addr_h;
    uint8_t pad[128 - 23];  /* pad to 128 bytes */
} _kc85_kcc_header;

/* invoke the post-snapshot-loading patch callback */
static void _kc85_invoke_patch_callback(kc85_t* sys, const _kc85_kcc_header* hdr) {
    if (sys->patch_callback.func) {
        char img_name[17];
        memcpy(img_name, hdr->name, 16);
        img_name[16] = 0;
        sys->patch_callback.func(img_name, sys->patch_callback.user_data);
    }
}

/* KCC files cannot really be identified since they have no magic number */
bool kc85_is_valid_kcc(chips_range_t data) {
    if (data.size <= sizeof(_kc85_kcc_header)) {
        return false;
    }
    const _kc85_kcc_header* hdr = (const _kc85_kcc_header*) data.ptr;
    if (hdr->num_addr > 3) {
        return false;
    }
    uint16_t load_addr = hdr->load_addr_h<<8 | hdr->load_addr_l;
    uint16_t end_addr = hdr->end_addr_h<<8 | hdr->end_addr_l;
    if (end_addr <= load_addr) {
        return false;
    }
    if (hdr->num_addr > 2) {
        uint16_t exec_addr = hdr->exec_addr_h<<8 | hdr->exec_addr_l;
        if ((exec_addr < load_addr) || (exec_addr > end_addr)) {
            return false;
        }
    }
    size_t required_len = (end_addr - load_addr) + sizeof(_kc85_kcc_header);
    if (required_len > data.size) {
        return false;
    }
    /* could be a KCC file */
    return true;
}

uint16_t kc85_kcc_exec_addr(chips_range_t data) {
    assert(kc85_is_valid_kcc(data));
    const _kc85_kcc_header* hdr = (const _kc85_kcc_header*) data.ptr;
    return hdr->exec_addr_h<<8 | hdr->exec_addr_l;
}

static bool _kc85_load_kcc(kc85_t* sys, chips_range_t data, bool start) {
    const _kc85_kcc_header* hdr = (_kc85_kcc_header*) data.ptr;
    uint16_t addr = hdr->load_addr_h<<8 | hdr->load_addr_l;
    uint16_t end_addr  = hdr->end_addr_h<<8 | hdr->end_addr_l;
    const uint8_t* ptr = (const uint8_t*)data.ptr + sizeof(_kc85_kcc_header);
    while (addr < end_addr) {
        mem_wr(&sys->mem, addr++, *ptr++);
    }
    _kc85_invoke_patch_callback(sys, hdr);
    if (start && (hdr->num_addr > 2)) {
        _kc85_load_start(sys, hdr->exec_addr_h<<8 | hdr->exec_addr_l);
    }
    return true;
}

/* KC TAP file format support */
typedef struct {
    uint8_t sig[16];        /* "\xC3KC-TAPE by AF. " */
    uint8_t type;           /* 00: KCTAP_Z9001, 01: KCTAP_KC85, else: KCTAP_SYS */
    _kc85_kcc_header kcc;   /* from here on identical with KCC */
} _kc85_kctap_header;

bool kc85_is_valid_kctap(chips_range_t data) {
    if (data.size <= sizeof(_kc85_kctap_header)) {
        return false;
    }
    const _kc85_kctap_header* hdr = (const _kc85_kctap_header*)data.ptr;
    static uint8_t sig[16] = { 0xC3,'K','C','-','T','A','P','E',0x20,'b','y',0x20,'A','F','.',0x20 };
    for (size_t i = 0; i < 16; i++) {
        if (sig[i] != hdr->sig[i]) {
            return false;
        }
    }
    if (hdr->kcc.num_addr > 3) {
        return false;
    }
    uint16_t load_addr = hdr->kcc.load_addr_h<<8 | hdr->kcc.load_addr_l;
    uint16_t end_addr = hdr->kcc.end_addr_h<<8 | hdr->kcc.end_addr_l;
    if (end_addr <= load_addr) {
        return false;
    }
    if (hdr->kcc.num_addr > 2) {
        uint16_t exec_addr = hdr->kcc.exec_addr_h<<8 | hdr->kcc.exec_addr_l;
        if ((exec_addr < load_addr) || (exec_addr > end_addr)) {
            return false;
        }
    }
    size_t required_len = (end_addr - load_addr) + sizeof(_kc85_kctap_header);
    if (required_len > data.size) {
        return false;
    }
    /* this appears to be a valid KC TAP file */
    return true;
}

static bool _kc85_load_kctap(kc85_t* sys, chips_range_t data, bool start) {
    const _kc85_kctap_header* hdr = (const _kc85_kctap_header*) data.ptr;
    uint16_t addr = hdr->kcc.load_addr_h<<8 | hdr->kcc.load_addr_l;
    uint16_t end_addr  = hdr->kcc.end_addr_h<<8 | hdr->kcc.end_addr_l;
    const uint8_t* ptr = (const uint8_t*)data.ptr + sizeof(_kc85_kctap_header);
    while (addr < end_addr) {
        /* each block is 1 lead-byte + 128 bytes data */
        ptr++;
        for (size_t i = 0; i < 128; i++) {
            mem_wr(&sys->mem, addr++, *ptr++);
        }
    }
    _kc85_invoke_patch_callback(sys, &hdr->kcc);
    /* if file has an exec-address, start the program */
    if (start && (hdr->kcc.num_addr > 2)) {
        _kc85_load_start(sys, hdr->kcc.exec_addr_h<<8 | hdr->kcc.exec_addr_l);
    }
    return true;
}

bool kc85_quickload(kc85_t* sys, chips_range_t data, bool start) {
    CHIPS_ASSERT(sys && sys->valid && data.ptr);
    /* first check for KC-TAP format, since this can be properly identified */
    if (kc85_is_valid_kctap(data)) {
        return _kc85_load_kctap(sys, data, start);
    }
    else if (kc85_is_valid_kcc(data)) {
        return _kc85_load_kcc(sys, data, start);
    }
    else {
        /* not a known file type, or not enough data */
        return false;
    }
}

void kc85_key_down(kc85_t* sys, int key_code) {
    CHIPS_ASSERT(sys && sys->valid);
    kbd_key_down(&sys->kbd, key_code);
}

void kc85_key_up(kc85_t* sys, int key_code) {
    CHIPS_ASSERT(sys && sys->valid);
    kbd_key_up(&sys->kbd, key_code);
}

chips_display_info_t kc85_display_info(kc85_t* sys) {
    static const uint32_t palette[36] = {
        // 16 foreground colors
        0xFF000000,     // black
        0xFFFF0000,     // blue
        0xFF0000FF,     // red
        0xFFFF00FF,     // magenta
        0xFF00FF00,     // green
        0xFFFFFF00,     // cyan
        0xFF00FFFF,     // yellow
        0xFFFFFFFF,     // white
        0xFF000000,     // black #2
        0xFFFF00A0,     // violet
        0xFF00A0FF,     // orange
        0xFFA000FF,     // purple
        0xFFA0FF00,     // blueish green
        0xFFFFA000,     // greenish blue
        0xFF00FFA0,     // yellow-green
        0xFFFFFFFF,     // white #2

        // 8 background colors
        0xFF000000,      // black
        0xFFA00000,      // dark-blue
        0xFF0000A0,      // dark-red
        0xFFA000A0,      // dark-magenta
        0xFF00A000,      // dark-green
        0xFFA0A000,      // dark-cyan
        0xFF00A0A0,      // dark-yellow
        0xFFA0A0A0,      // gray

        // padding to get next block at 2^N
        0xFFFF00FF,
        0xFFFF00FF,
        0xFFFF00FF,
        0xFFFF00FF,
        0xFFFF00FF,
        0xFFFF00FF,
        0xFFFF00FF,
        0xFFFF00FF,

        // KC85/4 only: 4 extra HICOLOR colors
        0xFF000000,     // black
        0xFF0000FF,     // red
        0xFFFFFF00,     // cyan
        0xFFFFFFFF,     // white
    };
    const chips_display_info_t res = {
        .frame = {
            .dim = {
                .width = KC85_FRAMEBUFFER_WIDTH,
                .height = KC85_FRAMEBUFFER_HEIGHT,
            },
            .bytes_per_pixel = 1,
            .buffer = {
                .ptr = sys ? sys->fb : 0,
                .size = KC85_FRAMEBUFFER_SIZE_BYTES
            }
        },
        .screen = {
            .x = 0,
            .y = 0,
            .width = KC85_DISPLAY_WIDTH,
            .height = KC85_DISPLAY_HEIGHT,
        },
        .palette = {
            .ptr = (void*)palette,
            .size = sizeof(palette)
        }
    };
    CHIPS_ASSERT(((sys == 0) && (res.frame.buffer.ptr == 0)) || ((sys != 0) && (res.frame.buffer.ptr != 0)));
    return res;
}

uint32_t kc85_save_snapshot(kc85_t* sys, kc85_t* dst) {
    CHIPS_ASSERT(sys && dst);
    *dst = *sys;
    chips_debug_snapshot_onsave(&dst->debug);
    chips_audio_callback_snapshot_onsave(&dst->audio.callback);
    dst->patch_callback.func = 0;
    dst->patch_callback.user_data = 0;
    mem_snapshot_onsave(&dst->mem, sys);
    return KC85_SNAPSHOT_VERSION;
}

// load a snapshot, returns false if snapshot version doesn't match
bool kc85_load_snapshot(kc85_t* sys, uint32_t version, const kc85_t* src) {
    CHIPS_ASSERT(sys && src);
    if (version != KC85_SNAPSHOT_VERSION) {
        return false;
    }
    // intermediate copy
    static kc85_t im;
    im = *src;
    chips_debug_snapshot_onload(&im.debug, &sys->debug);
    chips_audio_callback_snapshot_onload(&im.audio.callback, &sys->audio.callback);
    im.patch_callback = sys->patch_callback;
    mem_snapshot_onload(&im.mem, sys);
    *sys = im;
    return true;
}

#endif /* CHIPS_IMPL */
