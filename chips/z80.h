#pragma once
/*#
    # z80.h

    A cycle-stepped Z80 emulator in a C header.

    Do this:
    ~~~~C
    #define CHIPS_IMPL
    ~~~~
    before you include this file in *one* C or C++ file to create the
    implementation.

    Optionally provide
    ~~~C
    #define CHIPS_ASSERT(x) your_own_asset_macro(x)
    ~~~

    ## Emulated Pins
    ***********************************
    *           +-----------+         *
    * M1    <---|           |---> A0  *
    * MREQ  <---|           |---> A1  *
    * IORQ  <---|           |---> A2  *
    * RD    <---|           |---> ..  *
    * WR    <---|    Z80    |---> A15 *
    * HALT  <---|           |         *
    * WAIT  --->|           |<--> D0  *
    * INT   --->|           |<--> D1  *
    * NMI   --->|           |<--> ... *
    * RFSH  <---|           |<--> D7  *
    *           +-----------+         *
    ***********************************

    ## Functions

    ~~~C
    uint64_t z80_init(z80_t* cpu);
    ~~~
        Initializes a new z80_t instance, returns initial pin mask to start
        execution at address 0.

    ~~~C
    uint64_t z80_reset(z80_t* cpu)
    ~~~
        Resets a z80_t instance, returns pin mask to start execution at
        address 0.

    ~~~C
    uint64_t z80_tick(z80_t* cpu, uint64_t pins)
    ~~~
        Step the z80_t instance for one clock cycle.

    ~~~C
    uint64_t z80_prefetch(z80_t* cpu, uint16_t new_pc)
    ~~~
        Call this function to force execution to start at a specific
        PC. Use the returned pin mask as argument into the next z80_tick() call.

    ~~~C
    bool z80_opdone(z80_t* cpu)
    ~~~
        Helper function to detect whether the z80_t instance has completed
        an instruction.

    ## HOWTO

    Initialize a new z80_t instance and start ticking it:
    ~~~C
        z80_t cpu;
        uint64_t pins = z80_init(&cpu);
        while (!done) {
            pins = z80_tick(&cpu, pins);
        }
    ~~~
    Since there is no memory attached yet, the CPU will simply run whatever opcode
    bytes are present on the data bus (in this case the data bus is zero, so the CPU
    just runs throught the same NOP over and over).

    Next, add some memory and inspect and modify the pin mask to handle memory accesses:
    ~~~C
        uint8_t mem[(1<<16)] = {0};
        z80_t cpu;
        uint64_t pins = z80_init(&cpu);
        while (!done) {
            pins = z80_tick(&cpu, pins);
            if (pins & Z80_MREQ) {
                const uint16_t addr = Z80_GET_ADDR(pins);
                if (pins & Z80_RD) {
                    uint8_t data = mem[addr];
                    Z80_SET_DATA(pins, data);
                }
                else if (pins & Z80_WR) {
                    uint8_t data = Z80_GET_DATA(pins);
                    mem[addr] = data;
                }
            }
        }
    ~~~
    The CPU will now run through the whole address space executing NOPs (because the memory is
    filled with 0s instead of a valid program). If there would be a valid Z80 program at memory
    address 0, this would be executed instead.

    IO requests are handled the same as memory requests, but instead of the MREQ pin, the
    IORQ pin must be checked:
    ~~~C
        uint8_t mem[(1<<16)] = {0};
        z80_t cpu;
        uint64_t pins = z80_init(&cpu);
        while (!done) {
            pins = z80_tick(&cpu, pins);
            if (pins & Z80_MREQ) {
                const uint16_t addr = Z80_GET_ADDR(pins);
                if (pins & Z80_RD) {
                    uint8_t data = mem[addr];
                    Z80_SET_DATA(pins, data);
                }
                else if (pins & Z80_WR) {
                    uint8_t data = Z80_GET_DATA(pins);
                    mem[addr] = data;
                }
            }
            else if (pins & Z80_IORQ) {
                const uint16_t port = Z80_GET_ADDR(pins);
                if (pins & Z80_RD) {
                    // handle IO input request at port
                    ...
                }
                else if (pins & Z80_WR) {
                    // handle IO output request at port
                    ...
                }
            }
        }
    ~~~

    Handle interrupt acknowledge cycles by checking for Z80_IORQ|Z80_M1:
    ~~~C
        uint8_t mem[(1<<16)] = {0};
        z80_t cpu;
        uint64_t pins = z80_init(&cpu);
        while (!done) {
            pins = z80_tick(&cpu, pins);
            if (pins & Z80_MREQ) {
                const uint16_t addr = Z80_GET_ADDR(pins);
                if (pins & Z80_RD) {
                    uint8_t data = mem[addr];
                    Z80_SET_DATA(pins, data);
                }
                else if (pins & Z80_WR) {
                    uint8_t data = Z80_GET_DATA(pins);
                    mem[addr] = data;
                }
            }
            else if (pins & Z80_IORQ) {
                const uint16_t addr = Z80_GET_ADDR(pins);
                if (pins & Z80_M1) {
                    // an interrupt acknowledge cycle, depending on the emulated system,
                    // put either an instruction byte, or an interrupt vector on the data bus
                    Z80_SET_DATA(pins, opcode_or_intvec);
                }
                else if (pins & Z80_RD) {
                    // handle IO input request at port `addr`
                    ...
                }
                else if (pins & Z80_WR) {
                    // handle IO output request at port `addr`
                    ...
                }
            }
        }
    ~~~

    To request an interrupt, or inject a wait state just set the respective pin
    (Z80_INT, Z80_NMI, Z80_WAIT), don't forget to clear the pin again later (the
    details on when those pins are set and cleared depend heavily on the
    emulated system).

    !!! note
        NOTE: The Z80_RES pin is currently not emulated. Instead call the `z80_reset()` function.

    To emulate a whole computer system, add the per-tick code for the rest of the system to the
    basic ticking code above.

    If the emulated system uses the Z80 daisychain interrupt protocol (for instance when using
    the Z80 family chips like the PIO or CTC), tick those chips in interrupt priority order and
    set the Z80_IEIO pin before the highest priority chip in the daisychain is ticked:

    ~~~C
        ...
        while (!done) {
            pins = z80_tick(&cpu, pins);
            ...
            // tick Z80 family chips in 'daisychain order':
            pins |= Z80_IEIO;
            ...
            pins = z80ctc_tick(&ctc, pins);
            ...
            pins = z80pio_tick(&pio, pins);
            ...
            // the Z80_INT pin will now be set if any of the chips wants to issue an interrupt request
        }
    ~~~
#*/
/*
    zlib/libpng license

    Copyright (c) 2021 Andre Weissflog
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
*/
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// address pins
#define Z80_PIN_A0  (0)
#define Z80_PIN_A1  (1)
#define Z80_PIN_A2  (2)
#define Z80_PIN_A3  (3)
#define Z80_PIN_A4  (4)
#define Z80_PIN_A5  (5)
#define Z80_PIN_A6  (6)
#define Z80_PIN_A7  (7)
#define Z80_PIN_A8  (8)
#define Z80_PIN_A9  (9)
#define Z80_PIN_A10 (10)
#define Z80_PIN_A11 (11)
#define Z80_PIN_A12 (12)
#define Z80_PIN_A13 (13)
#define Z80_PIN_A14 (14)
#define Z80_PIN_A15 (15)

// data pins
#define Z80_PIN_D0  (16)
#define Z80_PIN_D1  (17)
#define Z80_PIN_D2  (18)
#define Z80_PIN_D3  (19)
#define Z80_PIN_D4  (20)
#define Z80_PIN_D5  (21)
#define Z80_PIN_D6  (22)
#define Z80_PIN_D7  (23)

// control pins
#define Z80_PIN_M1    (24)        // machine cycle 1
#define Z80_PIN_MREQ  (25)        // memory request
#define Z80_PIN_IORQ  (26)        // input/output request
#define Z80_PIN_RD    (27)        // read
#define Z80_PIN_WR    (28)        // write
#define Z80_PIN_HALT  (29)        // halt state
#define Z80_PIN_INT   (30)        // interrupt request
#define Z80_PIN_RES   (31)        // reset requested
#define Z80_PIN_NMI   (32)        // non-maskable interrupt
#define Z80_PIN_WAIT  (33)        // wait requested
#define Z80_PIN_RFSH  (34)        // refresh

// virtual pins (for interrupt daisy chain protocol)
#define Z80_PIN_IEIO  (37)      // unified daisy chain 'Interrupt Enable In+Out'
#define Z80_PIN_RETI  (38)      // cpu has decoded a RETI instruction

// pin bit masks
#define Z80_A0    (1ULL<<Z80_PIN_A0)
#define Z80_A1    (1ULL<<Z80_PIN_A1)
#define Z80_A2    (1ULL<<Z80_PIN_A2)
#define Z80_A3    (1ULL<<Z80_PIN_A3)
#define Z80_A4    (1ULL<<Z80_PIN_A4)
#define Z80_A5    (1ULL<<Z80_PIN_A5)
#define Z80_A6    (1ULL<<Z80_PIN_A6)
#define Z80_A7    (1ULL<<Z80_PIN_A7)
#define Z80_A8    (1ULL<<Z80_PIN_A8)
#define Z80_A9    (1ULL<<Z80_PIN_A9)
#define Z80_A10   (1ULL<<Z80_PIN_A10)
#define Z80_A11   (1ULL<<Z80_PIN_A11)
#define Z80_A12   (1ULL<<Z80_PIN_A12)
#define Z80_A13   (1ULL<<Z80_PIN_A13)
#define Z80_A14   (1ULL<<Z80_PIN_A14)
#define Z80_A15   (1ULL<<Z80_PIN_A15)
#define Z80_D0    (1ULL<<Z80_PIN_D0)
#define Z80_D1    (1ULL<<Z80_PIN_D1)
#define Z80_D2    (1ULL<<Z80_PIN_D2)
#define Z80_D3    (1ULL<<Z80_PIN_D3)
#define Z80_D4    (1ULL<<Z80_PIN_D4)
#define Z80_D5    (1ULL<<Z80_PIN_D5)
#define Z80_D6    (1ULL<<Z80_PIN_D6)
#define Z80_D7    (1ULL<<Z80_PIN_D7)
#define Z80_M1    (1ULL<<Z80_PIN_M1)
#define Z80_MREQ  (1ULL<<Z80_PIN_MREQ)
#define Z80_IORQ  (1ULL<<Z80_PIN_IORQ)
#define Z80_RD    (1ULL<<Z80_PIN_RD)
#define Z80_WR    (1ULL<<Z80_PIN_WR)
#define Z80_HALT  (1ULL<<Z80_PIN_HALT)
#define Z80_INT   (1ULL<<Z80_PIN_INT)
#define Z80_RES   (1ULL<<Z80_PIN_RES)
#define Z80_NMI   (1ULL<<Z80_PIN_NMI)
#define Z80_WAIT  (1ULL<<Z80_PIN_WAIT)
#define Z80_RFSH  (1ULL<<Z80_PIN_RFSH)
#define Z80_IEIO  (1ULL<<Z80_PIN_IEIO)
#define Z80_RETI  (1ULL<<Z80_PIN_RETI)

#define Z80_CTRL_PIN_MASK (Z80_M1|Z80_MREQ|Z80_IORQ|Z80_RD|Z80_WR|Z80_RFSH)
#define Z80_PIN_MASK ((1ULL<<40)-1)

// pin access helper macros
#define Z80_MAKE_PINS(ctrl, addr, data) ((ctrl)|((data&0xFF)<<16)|((addr)&0xFFFFULL))
#define Z80_GET_ADDR(p) ((uint16_t)(p))
#define Z80_SET_ADDR(p,a) {p=((p)&~0xFFFF)|((a)&0xFFFF);}
#define Z80_GET_DATA(p) ((uint8_t)((p)>>16))
#define Z80_SET_DATA(p,d) {p=((p)&~0xFF0000ULL)|(((d)<<16)&0xFF0000ULL);}

// status flags
#define Z80_CF (1<<0)           // carry
#define Z80_NF (1<<1)           // add/subtract
#define Z80_VF (1<<2)           // parity/overflow
#define Z80_PF Z80_VF
#define Z80_XF (1<<3)           // undocumented bit 3
#define Z80_HF (1<<4)           // half carry
#define Z80_YF (1<<5)           // undocumented bit 5
#define Z80_ZF (1<<6)           // zero
#define Z80_SF (1<<7)           // sign

// CPU state
typedef struct {
    uint16_t step;      // the currently active decoder step
    uint16_t addr;      // effective address for (HL),(IX+d),(IY+d)
    uint8_t dlatch;     // temporary store for data bus value
    uint8_t opcode;     // current opcode
    uint8_t hlx_idx;    // index into hlx[] for mapping hl to ix or iy (0: hl, 1: ix, 2: iy)
    bool prefix_active; // true if any prefix currently active (only needed in z80_opdone())
    uint64_t pins;      // last pin state, used for NMI detection
    uint64_t int_bits;  // track INT and NMI state
    union { struct { uint8_t pcl; uint8_t pch; }; uint16_t pc; };

    // NOTE: These unions are fine in C, but not C++.
    union { struct { uint8_t f; uint8_t a; }; uint16_t af; };
    union { struct { uint8_t c; uint8_t b; }; uint16_t bc; };
    union { struct { uint8_t e; uint8_t d; }; uint16_t de; };
    union {
        struct {
            union { struct { uint8_t l; uint8_t h; }; uint16_t hl; };
            union { struct { uint8_t ixl; uint8_t ixh; }; uint16_t ix; };
            union { struct { uint8_t iyl; uint8_t iyh; }; uint16_t iy; };
        };
        struct { union { struct { uint8_t l; uint8_t h; }; uint16_t hl; }; } hlx[3];
    };
    union { struct { uint8_t wzl; uint8_t wzh; }; uint16_t wz; };
    union { struct { uint8_t spl; uint8_t sph; }; uint16_t sp; };
    union { struct { uint8_t r; uint8_t i; }; uint16_t ir; };
    uint16_t af2, bc2, de2, hl2; // shadow register bank
    uint8_t im;
    bool iff1, iff2;
} z80_t;

// initialize a new Z80 instance and return initial pin mask
uint64_t z80_init(z80_t* cpu);
// immediately put Z80 into reset state
uint64_t z80_reset(z80_t* cpu);
// execute one tick, return new pin mask
uint64_t z80_tick(z80_t* cpu, uint64_t pins);
// force execution to continue at address 'new_pc'
uint64_t z80_prefetch(z80_t* cpu, uint16_t new_pc);
// return true when full instruction has finished
bool z80_opdone(z80_t* cpu);

#ifdef __cplusplus
} // extern C
#endif

//-- IMPLEMENTATION ------------------------------------------------------------
#ifdef CHIPS_IMPL
#include <string.h> // memset
#ifndef CHIPS_ASSERT
#include <assert.h>
#define CHIPS_ASSERT(c) assert(c)
#endif

#if defined(__GNUC__)
#define _Z80_UNREACHABLE __builtin_unreachable()
#elif defined(_MSC_VER)
#define _Z80_UNREACHABLE __assume(0)
#else
#define _Z80_UNREACHABLE
#endif

// extra/special decoder steps
// <% extra_step_defines
#define Z80_M1_T2 1685
#define Z80_M1_T3 1686
#define Z80_M1_T4 1687
#define Z80_DDFD_M1_T2 1688
#define Z80_DDFD_M1_T3 1689
#define Z80_DDFD_M1_T4 1690
#define Z80_DDFD_D_T1 1691
#define Z80_DDFD_D_T2 1692
#define Z80_DDFD_D_T3 1693
#define Z80_DDFD_D_T4 1694
#define Z80_DDFD_D_T5 1695
#define Z80_DDFD_D_T6 1696
#define Z80_DDFD_D_T7 1697
#define Z80_DDFD_D_T8 1698
#define Z80_DDFD_LDHLN_WR_T1 1699
#define Z80_DDFD_LDHLN_WR_T2 1700
#define Z80_DDFD_LDHLN_WR_T3 1701
#define Z80_DDFD_LDHLN_OVERLAPPED 1702
#define Z80_ED_M1_T2 1703
#define Z80_ED_M1_T3 1704
#define Z80_ED_M1_T4 1705
#define Z80_CB_M1_T2 1706
#define Z80_CB_M1_T3 1707
#define Z80_CB_M1_T4 1708
#define Z80_CB_STEP 1612
#define Z80_CBHL_STEP 1613
#define Z80_DDFDCB_STEP 1621
#define Z80_INT_IM0_STEP 1636
#define Z80_INT_IM1_STEP 1642
#define Z80_INT_IM2_STEP 1655
#define Z80_NMI_STEP 1674
// %>

uint64_t z80_init(z80_t* cpu) {
    CHIPS_ASSERT(cpu);
    // initial state as described in 'The Undocumented Z80 Documented'
    memset(cpu, 0, sizeof(z80_t));
    cpu->af = cpu->bc = cpu->de = cpu->hl = 0xFFFF;
    cpu->wz = cpu->sp = cpu->ix = cpu->iy = 0xFFFF;
    cpu->af2 = cpu->bc2 = cpu->de2 = cpu->hl2 = 0xFFFF;
    return z80_prefetch(cpu, 0x0000);
}

uint64_t z80_reset(z80_t* cpu) {
    // reset state as described in 'The Undocumented Z80 Documented'
    memset(cpu, 0, sizeof(z80_t));
    cpu->af = cpu->bc = cpu->de = cpu->hl = 0xFFFF;
    cpu->wz = cpu->sp = cpu->ix = cpu->iy = 0xFFFF;
    cpu->af2 = cpu->bc2 = cpu->de2 = cpu->hl2 = 0xFFFF;
    return z80_prefetch(cpu, 0x0000);
}

bool z80_opdone(z80_t* cpu) {
    // because of the overlapped cycle, the result of the previous
    // instruction is only available in M1/T2
    return ((cpu->pins & (Z80_M1|Z80_RD)) == (Z80_M1|Z80_RD)) && !cpu->prefix_active;
}

static inline uint64_t _z80_halt(z80_t* cpu, uint64_t pins) {
    cpu->pc--;
    return pins | Z80_HALT;
}

// sign+zero+parity lookup table
static const uint8_t _z80_szp_flags[256] = {
  0x44,0x00,0x00,0x04,0x00,0x04,0x04,0x00,0x08,0x0c,0x0c,0x08,0x0c,0x08,0x08,0x0c,
  0x00,0x04,0x04,0x00,0x04,0x00,0x00,0x04,0x0c,0x08,0x08,0x0c,0x08,0x0c,0x0c,0x08,
  0x20,0x24,0x24,0x20,0x24,0x20,0x20,0x24,0x2c,0x28,0x28,0x2c,0x28,0x2c,0x2c,0x28,
  0x24,0x20,0x20,0x24,0x20,0x24,0x24,0x20,0x28,0x2c,0x2c,0x28,0x2c,0x28,0x28,0x2c,
  0x00,0x04,0x04,0x00,0x04,0x00,0x00,0x04,0x0c,0x08,0x08,0x0c,0x08,0x0c,0x0c,0x08,
  0x04,0x00,0x00,0x04,0x00,0x04,0x04,0x00,0x08,0x0c,0x0c,0x08,0x0c,0x08,0x08,0x0c,
  0x24,0x20,0x20,0x24,0x20,0x24,0x24,0x20,0x28,0x2c,0x2c,0x28,0x2c,0x28,0x28,0x2c,
  0x20,0x24,0x24,0x20,0x24,0x20,0x20,0x24,0x2c,0x28,0x28,0x2c,0x28,0x2c,0x2c,0x28,
  0x80,0x84,0x84,0x80,0x84,0x80,0x80,0x84,0x8c,0x88,0x88,0x8c,0x88,0x8c,0x8c,0x88,
  0x84,0x80,0x80,0x84,0x80,0x84,0x84,0x80,0x88,0x8c,0x8c,0x88,0x8c,0x88,0x88,0x8c,
  0xa4,0xa0,0xa0,0xa4,0xa0,0xa4,0xa4,0xa0,0xa8,0xac,0xac,0xa8,0xac,0xa8,0xa8,0xac,
  0xa0,0xa4,0xa4,0xa0,0xa4,0xa0,0xa0,0xa4,0xac,0xa8,0xa8,0xac,0xa8,0xac,0xac,0xa8,
  0x84,0x80,0x80,0x84,0x80,0x84,0x84,0x80,0x88,0x8c,0x8c,0x88,0x8c,0x88,0x88,0x8c,
  0x80,0x84,0x84,0x80,0x84,0x80,0x80,0x84,0x8c,0x88,0x88,0x8c,0x88,0x8c,0x8c,0x88,
  0xa0,0xa4,0xa4,0xa0,0xa4,0xa0,0xa0,0xa4,0xac,0xa8,0xa8,0xac,0xa8,0xac,0xac,0xa8,
  0xa4,0xa0,0xa0,0xa4,0xa0,0xa4,0xa4,0xa0,0xa8,0xac,0xac,0xa8,0xac,0xa8,0xa8,0xac,
};

// lookup table for (HL)/(IX/IY+d) ops
static const uint8_t _z80_indirect_table[256] = {
    // <% indirect_table
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,
    0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,
    0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,
    1,1,1,1,1,1,0,1,0,0,0,0,0,0,1,0,
    0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,
    0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,
    0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,
    0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    // %>
};

static inline uint8_t _z80_sz_flags(uint8_t val) {
    return (val != 0) ? (val & Z80_SF) : Z80_ZF;
}

static inline uint8_t _z80_szyxch_flags(uint8_t acc, uint8_t val, uint32_t res) {
    return _z80_sz_flags(res) |
        (res & (Z80_YF|Z80_XF)) |
        ((res >> 8) & Z80_CF) |
        ((acc ^ val ^ res) & Z80_HF);
}

static inline uint8_t _z80_add_flags(uint8_t acc, uint8_t val, uint32_t res) {
    return _z80_szyxch_flags(acc, val, res) | ((((val ^ acc ^ 0x80) & (val ^ res)) >> 5) & Z80_VF);
}

static inline uint8_t _z80_sub_flags(uint8_t acc, uint8_t val, uint32_t res) {
    return Z80_NF | _z80_szyxch_flags(acc, val, res) | ((((val ^ acc) & (res ^ acc)) >> 5) & Z80_VF);
}

static inline uint8_t _z80_cp_flags(uint8_t acc, uint8_t val, uint32_t res) {
    return Z80_NF |
        _z80_sz_flags(res) |
        (val & (Z80_YF|Z80_XF)) |
        ((res >> 8) & Z80_CF) |
        ((acc ^ val ^ res) & Z80_HF) |
        ((((val ^ acc) & (res ^ acc)) >> 5) & Z80_VF);
}

static inline uint8_t _z80_sziff2_flags(z80_t* cpu, uint8_t val) {
    return (cpu->f & Z80_CF) | _z80_sz_flags(val) | (val & (Z80_YF|Z80_XF)) | (cpu->iff2 ? Z80_PF : 0);
}

static inline void _z80_add8(z80_t* cpu, uint8_t val) {
    uint32_t res = cpu->a + val;
    cpu->f = _z80_add_flags(cpu->a, val, res);
    cpu->a = (uint8_t)res;
}

static inline void _z80_adc8(z80_t* cpu, uint8_t val) {
    uint32_t res = cpu->a + val + (cpu->f & Z80_CF);
    cpu->f = _z80_add_flags(cpu->a, val, res);
    cpu->a = (uint8_t)res;
}

static inline void _z80_sub8(z80_t* cpu, uint8_t val) {
    uint32_t res = (uint32_t) ((int)cpu->a - (int)val);
    cpu->f = _z80_sub_flags(cpu->a, val, res);
    cpu->a = (uint8_t)res;
}

static inline void _z80_sbc8(z80_t* cpu, uint8_t val) {
    uint32_t res = (uint32_t) ((int)cpu->a - (int)val - (cpu->f & Z80_CF));
    cpu->f = _z80_sub_flags(cpu->a, val, res);
    cpu->a = (uint8_t)res;
}

static inline void _z80_and8(z80_t* cpu, uint8_t val) {
    cpu->a &= val;
    cpu->f = _z80_szp_flags[cpu->a] | Z80_HF;
}

static inline void _z80_xor8(z80_t* cpu, uint8_t val) {
    cpu->a ^= val;
    cpu->f = _z80_szp_flags[cpu->a];
}

static inline void _z80_or8(z80_t* cpu, uint8_t val) {
    cpu->a |= val;
    cpu->f = _z80_szp_flags[cpu->a];
}

static inline void _z80_cp8(z80_t* cpu, uint8_t val) {
    uint32_t res = (uint32_t) ((int)cpu->a - (int)val);
    cpu->f = _z80_cp_flags(cpu->a, val, res);
}

static inline void _z80_neg8(z80_t* cpu) {
    uint32_t res = (uint32_t) (0 - (int)cpu->a);
    cpu->f = _z80_sub_flags(0, cpu->a, res);
    cpu->a = (uint8_t)res;
}

static inline uint8_t _z80_inc8(z80_t* cpu, uint8_t val) {
    uint8_t res = val + 1;
    uint8_t f = _z80_sz_flags(res) | (res & (Z80_XF|Z80_YF)) | ((res ^ val) & Z80_HF);
    if (res == 0x80) {
        f |= Z80_VF;
    }
    cpu->f = f | (cpu->f & Z80_CF);
    return res;
}

static inline uint8_t _z80_dec8(z80_t* cpu, uint8_t val) {
    uint8_t res = val - 1;
    uint8_t f = Z80_NF | _z80_sz_flags(res) | (res & (Z80_XF|Z80_YF)) | ((res ^ val) & Z80_HF);
    if (res == 0x7F) {
        f |= Z80_VF;
    }
    cpu->f = f | (cpu->f & Z80_CF);
    return res;
}

static inline void _z80_ex_de_hl(z80_t* cpu) {
    uint16_t tmp = cpu->hl;
    cpu->hl = cpu->de;
    cpu->de = tmp;
}

static inline void _z80_ex_af_af2(z80_t* cpu) {
    uint16_t tmp = cpu->af2;
    cpu->af2 = cpu->af;
    cpu->af = tmp;
}

static inline void _z80_exx(z80_t* cpu) {
    uint16_t tmp;
    tmp = cpu->bc; cpu->bc = cpu->bc2; cpu->bc2 = tmp;
    tmp = cpu->de; cpu->de = cpu->de2; cpu->de2 = tmp;
    tmp = cpu->hl; cpu->hl = cpu->hl2; cpu->hl2 = tmp;
}

static inline void _z80_rlca(z80_t* cpu) {
    uint8_t res = (cpu->a << 1) | (cpu->a >> 7);
    cpu->f = ((cpu->a >> 7) & Z80_CF) | (cpu->f & (Z80_SF|Z80_ZF|Z80_PF)) | (res & (Z80_YF|Z80_XF));
    cpu->a = res;
}

static inline void _z80_rrca(z80_t* cpu) {
    uint8_t res = (cpu->a >> 1) | (cpu->a << 7);
    cpu->f = (cpu->a & Z80_CF) | (cpu->f & (Z80_SF|Z80_ZF|Z80_PF)) | (res & (Z80_YF|Z80_XF));
    cpu->a = res;
}

static inline void _z80_rla(z80_t* cpu) {
    uint8_t res = (cpu->a << 1) | (cpu->f & Z80_CF);
    cpu->f = ((cpu->a >> 7) & Z80_CF) | (cpu->f & (Z80_SF|Z80_ZF|Z80_PF)) | (res & (Z80_YF|Z80_XF));
    cpu->a = res;
}

static inline void _z80_rra(z80_t* cpu) {
    uint8_t res = (cpu->a >> 1) | ((cpu->f & Z80_CF) << 7);
    cpu->f = (cpu->a & Z80_CF) | (cpu->f & (Z80_SF|Z80_ZF|Z80_PF)) | (res & (Z80_YF|Z80_XF));
    cpu->a = res;
}

static inline void _z80_daa(z80_t* cpu) {
    uint8_t res = cpu->a;
    if (cpu->f & Z80_NF) {
        if (((cpu->a & 0xF)>0x9) || (cpu->f & Z80_HF)) {
            res -= 0x06;
        }
        if ((cpu->a > 0x99) || (cpu->f & Z80_CF)) {
            res -= 0x60;
        }
    }
    else {
        if (((cpu->a & 0xF)>0x9) || (cpu->f & Z80_HF)) {
            res += 0x06;
        }
        if ((cpu->a > 0x99) || (cpu->f & Z80_CF)) {
            res += 0x60;
        }
    }
    cpu->f &= Z80_CF|Z80_NF;
    cpu->f |= (cpu->a > 0x99) ? Z80_CF : 0;
    cpu->f |= (cpu->a ^ res) & Z80_HF;
    cpu->f |= _z80_szp_flags[res];
    cpu->a = res;
}

static inline void _z80_cpl(z80_t* cpu) {
    cpu->a ^= 0xFF;
    cpu->f= (cpu->f & (Z80_SF|Z80_ZF|Z80_PF|Z80_CF)) |Z80_HF|Z80_NF| (cpu->a & (Z80_YF|Z80_XF));
}

static inline void _z80_scf(z80_t* cpu) {
    cpu->f = (cpu->f & (Z80_SF|Z80_ZF|Z80_PF|Z80_CF)) | Z80_CF | (cpu->a & (Z80_YF|Z80_XF));
}

static inline void _z80_ccf(z80_t* cpu) {
    cpu->f = ((cpu->f & (Z80_SF|Z80_ZF|Z80_PF|Z80_CF)) | ((cpu->f & Z80_CF)<<4) | (cpu->a & (Z80_YF|Z80_XF))) ^ Z80_CF;
}

static inline void _z80_add16(z80_t* cpu, uint16_t val) {
    const uint16_t acc = cpu->hlx[cpu->hlx_idx].hl;
    cpu->wz = acc + 1;
    const uint32_t res = acc + val;
    cpu->hlx[cpu->hlx_idx].hl = res;
    cpu->f = (cpu->f & (Z80_SF|Z80_ZF|Z80_VF)) |
             (((acc ^ res ^ val)>>8)&Z80_HF) |
             ((res >> 16) & Z80_CF) |
             ((res >> 8) & (Z80_YF|Z80_XF));
}

static inline void _z80_adc16(z80_t* cpu, uint16_t val) {
    // NOTE: adc is ED-prefixed, so they are never rewired to IX/IY
    const uint16_t acc = cpu->hl;
    cpu->wz = acc + 1;
    const uint32_t res = acc + val + (cpu->f & Z80_CF);
    cpu->hl = res;
    cpu->f = (((val ^ acc ^ 0x8000) & (val ^ res) & 0x8000) >> 13) |
             (((acc ^ res ^ val) >>8 ) & Z80_HF) |
             ((res >> 16) & Z80_CF) |
             ((res >> 8) & (Z80_SF|Z80_YF|Z80_XF)) |
             ((res & 0xFFFF) ? 0 : Z80_ZF);
}

static inline void _z80_sbc16(z80_t* cpu, uint16_t val) {
    // NOTE: sbc is ED-prefixed, so they are never rewired to IX/IY
    const uint16_t acc = cpu->hl;
    cpu->wz = acc + 1;
    const uint32_t res = acc - val - (cpu->f & Z80_CF);
    cpu->hl = res;
    cpu->f = (Z80_NF | (((val ^ acc) & (acc ^ res) & 0x8000) >> 13)) |
             (((acc ^ res ^ val) >> 8) & Z80_HF) |
             ((res >> 16) & Z80_CF) |
             ((res >> 8) & (Z80_SF|Z80_YF|Z80_XF)) |
             ((res & 0xFFFF) ? 0 : Z80_ZF);
}

static inline bool _z80_ldi_ldd(z80_t* cpu, uint8_t val) {
    const uint8_t res = cpu->a + val;
    cpu->bc -= 1;
    cpu->f = (cpu->f & (Z80_SF|Z80_ZF|Z80_CF)) |
             ((res & 2) ? Z80_YF : 0) |
             ((res & 8) ? Z80_XF : 0) |
             (cpu->bc ? Z80_VF : 0);
    return cpu->bc != 0;
}

static inline bool _z80_cpi_cpd(z80_t* cpu, uint8_t val) {
    uint32_t res = (uint32_t) ((int)cpu->a - (int)val);
    cpu->bc -= 1;
    uint8_t f = (cpu->f & Z80_CF)|Z80_NF|_z80_sz_flags(res);
    if ((res & 0xF) > ((uint32_t)cpu->a & 0xF)) {
        f |= Z80_HF;
        res--;
    }
    if (res & 2) { f |= Z80_YF; }
    if (res & 8) { f |= Z80_XF; }
    if (cpu->bc) { f |= Z80_VF; }
    cpu->f = f;
    return (cpu->bc != 0) && !(f & Z80_ZF);
}

static inline bool _z80_ini_ind(z80_t* cpu, uint8_t val, uint8_t c) {
    const uint8_t b = cpu->b;
    uint8_t f = _z80_sz_flags(b) | (b & (Z80_XF|Z80_YF));
    if (val & Z80_SF) { f |= Z80_NF; }
    uint32_t t = (uint32_t)c + val;
    if (t & 0x100) { f |= Z80_HF|Z80_CF; }
    f |= _z80_szp_flags[((uint8_t)(t & 7)) ^ b] & Z80_PF;
    cpu->f = f;
    return (b != 0);
}

static inline bool _z80_outi_outd(z80_t* cpu, uint8_t val) {
    const uint8_t b = cpu->b;
    uint8_t f = _z80_sz_flags(b) | (b & (Z80_XF|Z80_YF));
    if (val & Z80_SF) { f |= Z80_NF; }
    uint32_t t = (uint32_t)cpu->l + val;
    if (t & 0x0100) { f |= Z80_HF|Z80_CF; }
    f |= _z80_szp_flags[((uint8_t)(t & 7)) ^ b] & Z80_PF;
    cpu->f = f;
    return (b != 0);
}

static inline uint8_t _z80_in(z80_t* cpu, uint8_t val) {
    cpu->f = (cpu->f & Z80_CF) | _z80_szp_flags[val];
    return val;
}

static inline uint8_t _z80_rrd(z80_t* cpu, uint8_t val) {
    const uint8_t l = cpu->a & 0x0F;
    cpu->a = (cpu->a & 0xF0) | (val & 0x0F);
    val = (val >> 4) | (l << 4);
    cpu->f = (cpu->f & Z80_CF) | _z80_szp_flags[cpu->a];
    return val;
}

static inline uint8_t _z80_rld(z80_t* cpu, uint8_t val) {
    const uint8_t l = cpu->a & 0x0F;
    cpu->a = (cpu->a & 0xF0) | (val >> 4);
    val = (val << 4) | l;
    cpu->f = (cpu->f & Z80_CF) | _z80_szp_flags[cpu->a];
    return val;
}

static inline uint8_t _z80_rlc(z80_t* cpu, uint8_t val) {
    uint8_t res = (val<<1) | (val>>7);
    cpu->f = _z80_szp_flags[res] | ((val>>7) & Z80_CF);
    return res;
}

static inline uint8_t _z80_rrc(z80_t* cpu, uint8_t val) {
    uint8_t res = (val>>1) | (val<<7);
    cpu->f = _z80_szp_flags[res] | (val & Z80_CF);
    return res;
}

static inline uint8_t _z80_rl(z80_t* cpu, uint8_t val) {
    uint8_t res = (val<<1) | (cpu->f & Z80_CF);
    cpu->f = _z80_szp_flags[res] | ((val>>7) & Z80_CF);
    return res;
}

static inline uint8_t _z80_rr(z80_t* cpu, uint8_t val) {
    uint8_t res = (val>>1) | ((cpu->f & Z80_CF)<<7);
    cpu->f = _z80_szp_flags[res] | (val & Z80_CF);
    return res;
}

static inline uint8_t _z80_sla(z80_t* cpu, uint8_t val) {
    uint8_t res = val<<1;
    cpu->f = _z80_szp_flags[res] | ((val>>7) & Z80_CF);
    return res;
}

static inline uint8_t _z80_sra(z80_t* cpu, uint8_t val) {
    uint8_t res = (val>>1) | (val & 0x80);
    cpu->f = _z80_szp_flags[res] | (val & Z80_CF);
    return res;
}

static inline uint8_t _z80_sll(z80_t* cpu, uint8_t val) {
    uint8_t res = (val<<1) | 1;
    cpu->f = _z80_szp_flags[res] | ((val>>7) & Z80_CF);
    return res;
}

static inline uint8_t _z80_srl(z80_t* cpu, uint8_t val) {
    uint8_t res = val>>1;
    cpu->f = _z80_szp_flags[res] | (val & Z80_CF);
    return res;
}

static inline uint64_t _z80_set_ab(uint64_t pins, uint16_t ab) {
    return (pins & ~0xFFFF) | ab;
}

static inline uint64_t _z80_set_ab_x(uint64_t pins, uint16_t ab, uint64_t x) {
    return (pins & ~0xFFFF) | ab | x;
}

static inline uint64_t _z80_set_ab_db(uint64_t pins, uint16_t ab, uint8_t db) {
    return (pins & ~0xFFFFFF) | (db<<16) | ab;
}

static inline uint64_t _z80_set_ab_db_x(uint64_t pins, uint16_t ab, uint8_t db, uint64_t x) {
    return (pins & ~0xFFFFFF) | (db<<16) | ab | x;
}

static inline uint8_t _z80_get_db(uint64_t pins) {
    return (uint8_t)(pins>>16);
}

// CB-prefix block action
static inline bool _z80_cb_action(z80_t* cpu, uint8_t z0, uint8_t z1) {
    const uint8_t x = cpu->opcode>>6;
    const uint8_t y = (cpu->opcode>>3)&7;
    uint8_t val, res;
    switch (z0) {
        case 0: val = cpu->b; break;
        case 1: val = cpu->c; break;
        case 2: val = cpu->d; break;
        case 3: val = cpu->e; break;
        case 4: val = cpu->h; break;
        case 5: val = cpu->l; break;
        case 6: val = cpu->dlatch; break;   // (HL)
        case 7: val = cpu->a; break;
        default: _Z80_UNREACHABLE;
    }
    switch (x) {
        case 0: // rot/shift
            switch (y) {
                case 0: res = _z80_rlc(cpu, val); break;
                case 1: res = _z80_rrc(cpu, val); break;
                case 2: res = _z80_rl(cpu, val); break;
                case 3: res = _z80_rr(cpu, val); break;
                case 4: res = _z80_sla(cpu, val); break;
                case 5: res = _z80_sra(cpu, val); break;
                case 6: res = _z80_sll(cpu, val); break;
                case 7: res = _z80_srl(cpu, val); break;
                default: _Z80_UNREACHABLE;
            }
            break;
        case 1: // bit
            res = val & (1<<y);
            cpu->f = (cpu->f & Z80_CF) | Z80_HF | (res ? (res & Z80_SF) : (Z80_ZF|Z80_PF));
            if (z0 == 6) {
                cpu->f |= (cpu->wz >> 8) & (Z80_YF|Z80_XF);
            }
            else {
                cpu->f |= val & (Z80_YF|Z80_XF);
            }
            break;
        case 2: // res
            res = val & ~(1 << y);
            break;
        case 3: // set
            res = val | (1 << y);
            break;
        default: _Z80_UNREACHABLE;
    }
    // don't write result back for BIT
    if (x != 1) {
        cpu->dlatch = res;
        switch (z1) {
            case 0: cpu->b = res; break;
            case 1: cpu->c = res; break;
            case 2: cpu->d = res; break;
            case 3: cpu->e = res; break;
            case 4: cpu->h = res; break;
            case 5: cpu->l = res; break;
            case 6: break;   // (HL)
            case 7: cpu->a = res; break;
            default: _Z80_UNREACHABLE;
        }
        return true;
    }
    else {
        return false;
    }
}

// compute the effective memory address for DD+CB/FD+CB instructions
static inline void _z80_ddfdcb_addr(z80_t* cpu, uint64_t pins) {
    uint8_t d = _z80_get_db(pins);
    cpu->addr = cpu->hlx[cpu->hlx_idx].hl + (int8_t)d;
    cpu->wz = cpu->addr;
}

// initiate refresh cycle
static inline uint64_t _z80_refresh(z80_t* cpu, uint64_t pins) {
    pins = _z80_set_ab_x(pins, cpu->ir, Z80_MREQ|Z80_RFSH);
    cpu->r = (cpu->r & 0x80) | ((cpu->r + 1) & 0x7F);
    return pins;
}

// track NMI 0 => 1 edge and current INT pin state, this will track the
// relevant interrupt status up to the last instruction cycle and will
// be checked in the first M1 cycle (during _fetch)
static inline void _z80_track_int_bits(z80_t* cpu, uint64_t pins) {
    const uint64_t rising_nmi = (pins ^ cpu->pins) & pins; // NMI 0 => 1
    cpu->pins = pins;
    cpu->int_bits = ((cpu->int_bits | rising_nmi) & Z80_NMI) | (pins & Z80_INT);
}

// initiate a fetch machine cycle for regular (non-prefixed) instructions, or initiate interrupt handling
static inline uint64_t _z80_fetch(z80_t* cpu, uint64_t pins) {
    cpu->hlx_idx = 0;
    cpu->prefix_active = false;
    // shortcut no interrupts requested
    if (cpu->int_bits == 0) {
        cpu->step = Z80_M1_T2;
        return _z80_set_ab_x(pins, cpu->pc++, Z80_M1|Z80_MREQ|Z80_RD);
    } else if (cpu->int_bits & Z80_NMI) {
        // non-maskable interrupt starts with a regular M1 machine cycle
        cpu->step = Z80_NMI_STEP;
        cpu->int_bits = 0;
        if (pins & Z80_HALT) {
            pins &= ~Z80_HALT;
            cpu->pc++;
        }
        // NOTE: PC is *not* incremented!
        return _z80_set_ab_x(pins, cpu->pc, Z80_M1|Z80_MREQ|Z80_RD);
    } else if (cpu->int_bits & Z80_INT) {
        if (cpu->iff1) {
            // maskable interrupts start with a special M1 machine cycle which
            // doesn't fetch the next opcode, but instead activate the
            // pins M1|IOQR to request a special byte which is handled differently
            // depending on interrupt mode
            switch (cpu->im) {
                case 0: cpu->step = Z80_INT_IM0_STEP; break;
                case 1: cpu->step = Z80_INT_IM1_STEP; break;
                case 2: cpu->step = Z80_INT_IM2_STEP; break;
            }
            cpu->int_bits = 0;
            if (pins & Z80_HALT) {
                pins &= ~Z80_HALT;
                cpu->pc++;
            }
            // NOTE: PC is not incremented, and no pins are activated here
            return pins;
        } else {
            // oops, maskable interrupt requested but disabled
            cpu->step = Z80_M1_T2;
            return _z80_set_ab_x(pins, cpu->pc++, Z80_M1|Z80_MREQ|Z80_RD);
        }
    } else {
        _Z80_UNREACHABLE;
        return pins;
    }
}

static inline uint64_t _z80_fetch_cb(z80_t* cpu, uint64_t pins) {
    cpu->prefix_active = true;
    if (cpu->hlx_idx > 0) {
        // this is a DD+CB / FD+CB instruction, continue
        // execution on the special DDCB/FDCB decoder block which
        // loads the d-offset first and then the opcode in a
        // regular memory read machine cycle
        cpu->step = Z80_DDFDCB_STEP;
    } else {
        // this is a regular CB-prefixed instruction, continue
        // execution on a special fetch machine cycle which doesn't
        // handle DD/FD prefix and then branches either to the
        // special CB or CBHL decoder block
        cpu->step = Z80_CB_M1_T2; // => opcode fetch for CB prefixed instructions
        pins = _z80_set_ab_x(pins, cpu->pc++, Z80_M1|Z80_MREQ|Z80_RD);
    }
    return pins;
}

static inline uint64_t _z80_fetch_dd(z80_t* cpu, uint64_t pins) {
    cpu->step = Z80_DDFD_M1_T2;
    cpu->hlx_idx = 1;
    cpu->prefix_active = true;
    return _z80_set_ab_x(pins, cpu->pc++, Z80_M1|Z80_MREQ|Z80_RD);
}

static inline uint64_t _z80_fetch_fd(z80_t* cpu, uint64_t pins) {
    cpu->step = Z80_DDFD_M1_T2;
    cpu->hlx_idx = 2;
    cpu->prefix_active = true;
    return _z80_set_ab_x(pins, cpu->pc++, Z80_M1|Z80_MREQ|Z80_RD);
}

static inline uint64_t _z80_fetch_ed(z80_t* cpu, uint64_t pins) {
    cpu->step = Z80_ED_M1_T2;
    cpu->hlx_idx = 0;
    cpu->prefix_active = true;
    return _z80_set_ab_x(pins, cpu->pc++, Z80_M1|Z80_MREQ|Z80_RD);
}

uint64_t z80_prefetch(z80_t* cpu, uint16_t new_pc) {
    cpu->pc = new_pc;
    // overlapped M1:T1 of the NOP instruction to initiate opcode fetch at new pc
    cpu->step = 0;
    return 0;
}

// pin helper macros
#define _sa(ab)             pins=_z80_set_ab(pins,ab)
#define _sax(ab,x)          pins=_z80_set_ab_x(pins,ab,x)
#define _sad(ab,d)          pins=_z80_set_ab_db(pins,ab,d)
#define _sadx(ab,d,x)       pins=_z80_set_ab_db_x(pins,ab,d,x)
#define _gd()               _z80_get_db(pins)

// high level helper macros
#define _goto(n)        cpu->step=n;goto step_to
#define _step()         goto step_next
#define _fetch()        goto fetch_next
#define _fetch_dd()     pins=_z80_fetch_dd(cpu,pins);
#define _fetch_fd()     pins=_z80_fetch_fd(cpu,pins);
#define _fetch_ed()     pins=_z80_fetch_ed(cpu,pins);
#define _fetch_cb()     pins=_z80_fetch_cb(cpu,pins);
#define _mread(ab)      _sax(ab,Z80_MREQ|Z80_RD)
#define _mwrite(ab,d)   _sadx(ab,d,Z80_MREQ|Z80_WR)
#define _ioread(ab)     _sax(ab,Z80_IORQ|Z80_RD)
#define _iowrite(ab,d)  _sadx(ab,d,Z80_IORQ|Z80_WR)
#define _wait()         {if(pins&Z80_WAIT)goto step_to;}
#define _cc_nz          (!(cpu->f&Z80_ZF))
#define _cc_z           (cpu->f&Z80_ZF)
#define _cc_nc          (!(cpu->f&Z80_CF))
#define _cc_c           (cpu->f&Z80_CF)
#define _cc_po          (!(cpu->f&Z80_PF))
#define _cc_pe          (cpu->f&Z80_PF)
#define _cc_p           (!(cpu->f&Z80_SF))
#define _cc_m           (cpu->f&Z80_SF)

uint64_t z80_tick(z80_t* cpu, uint64_t pins) {
    pins &= ~(Z80_CTRL_PIN_MASK|Z80_RETI);
    switch (cpu->step) {
        // <% decoder
        case    0: _fetch(); // NOP T:0
        case    1: _goto(512); // LD BC,nn T:0
        case    2: _goto(518); // LD (BC),A T:0
        case    3: cpu->bc++;_goto(521); // INC BC T:0
        case    4: cpu->b=_z80_inc8(cpu,cpu->b);_fetch(); // INC B T:0
        case    5: cpu->b=_z80_dec8(cpu,cpu->b);_fetch(); // DEC B T:0
        case    6: _goto(523); // LD B,n T:0
        case    7: _z80_rlca(cpu);_fetch(); // RLCA T:0
        case    8: _z80_ex_af_af2(cpu);_fetch(); // EX AF,AF' T:0
        case    9: _z80_add16(cpu,cpu->bc);_goto(526); // ADD HL,BC T:0
        case   10: _goto(533); // LD A,(BC) T:0
        case   11: cpu->bc--;_goto(536); // DEC BC T:0
        case   12: cpu->c=_z80_inc8(cpu,cpu->c);_fetch(); // INC C T:0
        case   13: cpu->c=_z80_dec8(cpu,cpu->c);_fetch(); // DEC C T:0
        case   14: _goto(538); // LD C,n T:0
        case   15: _z80_rrca(cpu);_fetch(); // RRCA T:0
        case   16: _goto(541); // DJNZ d T:0
        case   17: _goto(550); // LD DE,nn T:0
        case   18: _goto(556); // LD (DE),A T:0
        case   19: cpu->de++;_goto(559); // INC DE T:0
        case   20: cpu->d=_z80_inc8(cpu,cpu->d);_fetch(); // INC D T:0
        case   21: cpu->d=_z80_dec8(cpu,cpu->d);_fetch(); // DEC D T:0
        case   22: _goto(561); // LD D,n T:0
        case   23: _z80_rla(cpu);_fetch(); // RLA T:0
        case   24: _goto(564); // JR d T:0
        case   25: _z80_add16(cpu,cpu->de);_goto(572); // ADD HL,DE T:0
        case   26: _goto(579); // LD A,(DE) T:0
        case   27: cpu->de--;_goto(582); // DEC DE T:0
        case   28: cpu->e=_z80_inc8(cpu,cpu->e);_fetch(); // INC E T:0
        case   29: cpu->e=_z80_dec8(cpu,cpu->e);_fetch(); // DEC E T:0
        case   30: _goto(584); // LD E,n T:0
        case   31: _z80_rra(cpu);_fetch(); // RRA T:0
        case   32: _goto(587); // JR NZ,d T:0
        case   33: _goto(595); // LD HL,nn T:0
        case   34: _goto(601); // LD (nn),HL T:0
        case   35: cpu->hlx[cpu->hlx_idx].hl++;_goto(613); // INC HL T:0
        case   36: cpu->hlx[cpu->hlx_idx].h=_z80_inc8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); // INC H T:0
        case   37: cpu->hlx[cpu->hlx_idx].h=_z80_dec8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); // DEC H T:0
        case   38: _goto(615); // LD H,n T:0
        case   39: _z80_daa(cpu);_fetch(); // DAA T:0
        case   40: _goto(618); // JR Z,d T:0
        case   41: _z80_add16(cpu,cpu->hlx[cpu->hlx_idx].hl);_goto(626); // ADD HL,HL T:0
        case   42: _goto(633); // LD HL,(nn) T:0
        case   43: cpu->hlx[cpu->hlx_idx].hl--;_goto(645); // DEC HL T:0
        case   44: cpu->hlx[cpu->hlx_idx].l=_z80_inc8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); // INC L T:0
        case   45: cpu->hlx[cpu->hlx_idx].l=_z80_dec8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); // DEC L T:0
        case   46: _goto(647); // LD L,n T:0
        case   47: _z80_cpl(cpu);_fetch(); // CPL T:0
        case   48: _goto(650); // JR NC,d T:0
        case   49: _goto(658); // LD SP,nn T:0
        case   50: _goto(664); // LD (nn),A T:0
        case   51: cpu->sp++;_goto(673); // INC SP T:0
        case   52: _goto(675); // INC (HL) T:0
        case   53: _goto(682); // DEC (HL) T:0
        case   54: _goto(689); // LD (HL),n T:0
        case   55: _z80_scf(cpu);_fetch(); // SCF T:0
        case   56: _goto(695); // JR C,d T:0
        case   57: _z80_add16(cpu,cpu->sp);_goto(703); // ADD HL,SP T:0
        case   58: _goto(710); // LD A,(nn) T:0
        case   59: cpu->sp--;_goto(719); // DEC SP T:0
        case   60: cpu->a=_z80_inc8(cpu,cpu->a);_fetch(); // INC A T:0
        case   61: cpu->a=_z80_dec8(cpu,cpu->a);_fetch(); // DEC A T:0
        case   62: _goto(721); // LD A,n T:0
        case   63: _z80_ccf(cpu);_fetch(); // CCF T:0
        case   64: cpu->b=cpu->b;_fetch(); // LD B,B T:0
        case   65: cpu->b=cpu->c;_fetch(); // LD B,C T:0
        case   66: cpu->b=cpu->d;_fetch(); // LD B,D T:0
        case   67: cpu->b=cpu->e;_fetch(); // LD B,E T:0
        case   68: cpu->b=cpu->hlx[cpu->hlx_idx].h;_fetch(); // LD B,H T:0
        case   69: cpu->b=cpu->hlx[cpu->hlx_idx].l;_fetch(); // LD B,L T:0
        case   70: _goto(724); // LD B,(HL) T:0
        case   71: cpu->b=cpu->a;_fetch(); // LD B,A T:0
        case   72: cpu->c=cpu->b;_fetch(); // LD C,B T:0
        case   73: cpu->c=cpu->c;_fetch(); // LD C,C T:0
        case   74: cpu->c=cpu->d;_fetch(); // LD C,D T:0
        case   75: cpu->c=cpu->e;_fetch(); // LD C,E T:0
        case   76: cpu->c=cpu->hlx[cpu->hlx_idx].h;_fetch(); // LD C,H T:0
        case   77: cpu->c=cpu->hlx[cpu->hlx_idx].l;_fetch(); // LD C,L T:0
        case   78: _goto(727); // LD C,(HL) T:0
        case   79: cpu->c=cpu->a;_fetch(); // LD C,A T:0
        case   80: cpu->d=cpu->b;_fetch(); // LD D,B T:0
        case   81: cpu->d=cpu->c;_fetch(); // LD D,C T:0
        case   82: cpu->d=cpu->d;_fetch(); // LD D,D T:0
        case   83: cpu->d=cpu->e;_fetch(); // LD D,E T:0
        case   84: cpu->d=cpu->hlx[cpu->hlx_idx].h;_fetch(); // LD D,H T:0
        case   85: cpu->d=cpu->hlx[cpu->hlx_idx].l;_fetch(); // LD D,L T:0
        case   86: _goto(730); // LD D,(HL) T:0
        case   87: cpu->d=cpu->a;_fetch(); // LD D,A T:0
        case   88: cpu->e=cpu->b;_fetch(); // LD E,B T:0
        case   89: cpu->e=cpu->c;_fetch(); // LD E,C T:0
        case   90: cpu->e=cpu->d;_fetch(); // LD E,D T:0
        case   91: cpu->e=cpu->e;_fetch(); // LD E,E T:0
        case   92: cpu->e=cpu->hlx[cpu->hlx_idx].h;_fetch(); // LD E,H T:0
        case   93: cpu->e=cpu->hlx[cpu->hlx_idx].l;_fetch(); // LD E,L T:0
        case   94: _goto(733); // LD E,(HL) T:0
        case   95: cpu->e=cpu->a;_fetch(); // LD E,A T:0
        case   96: cpu->hlx[cpu->hlx_idx].h=cpu->b;_fetch(); // LD H,B T:0
        case   97: cpu->hlx[cpu->hlx_idx].h=cpu->c;_fetch(); // LD H,C T:0
        case   98: cpu->hlx[cpu->hlx_idx].h=cpu->d;_fetch(); // LD H,D T:0
        case   99: cpu->hlx[cpu->hlx_idx].h=cpu->e;_fetch(); // LD H,E T:0
        case  100: cpu->hlx[cpu->hlx_idx].h=cpu->hlx[cpu->hlx_idx].h;_fetch(); // LD H,H T:0
        case  101: cpu->hlx[cpu->hlx_idx].h=cpu->hlx[cpu->hlx_idx].l;_fetch(); // LD H,L T:0
        case  102: _goto(736); // LD H,(HL) T:0
        case  103: cpu->hlx[cpu->hlx_idx].h=cpu->a;_fetch(); // LD H,A T:0
        case  104: cpu->hlx[cpu->hlx_idx].l=cpu->b;_fetch(); // LD L,B T:0
        case  105: cpu->hlx[cpu->hlx_idx].l=cpu->c;_fetch(); // LD L,C T:0
        case  106: cpu->hlx[cpu->hlx_idx].l=cpu->d;_fetch(); // LD L,D T:0
        case  107: cpu->hlx[cpu->hlx_idx].l=cpu->e;_fetch(); // LD L,E T:0
        case  108: cpu->hlx[cpu->hlx_idx].l=cpu->hlx[cpu->hlx_idx].h;_fetch(); // LD L,H T:0
        case  109: cpu->hlx[cpu->hlx_idx].l=cpu->hlx[cpu->hlx_idx].l;_fetch(); // LD L,L T:0
        case  110: _goto(739); // LD L,(HL) T:0
        case  111: cpu->hlx[cpu->hlx_idx].l=cpu->a;_fetch(); // LD L,A T:0
        case  112: _goto(742); // LD (HL),B T:0
        case  113: _goto(745); // LD (HL),C T:0
        case  114: _goto(748); // LD (HL),D T:0
        case  115: _goto(751); // LD (HL),E T:0
        case  116: _goto(754); // LD (HL),H T:0
        case  117: _goto(757); // LD (HL),L T:0
        case  118: pins=_z80_halt(cpu,pins);_fetch(); // HALT T:0
        case  119: _goto(760); // LD (HL),A T:0
        case  120: cpu->a=cpu->b;_fetch(); // LD A,B T:0
        case  121: cpu->a=cpu->c;_fetch(); // LD A,C T:0
        case  122: cpu->a=cpu->d;_fetch(); // LD A,D T:0
        case  123: cpu->a=cpu->e;_fetch(); // LD A,E T:0
        case  124: cpu->a=cpu->hlx[cpu->hlx_idx].h;_fetch(); // LD A,H T:0
        case  125: cpu->a=cpu->hlx[cpu->hlx_idx].l;_fetch(); // LD A,L T:0
        case  126: _goto(763); // LD A,(HL) T:0
        case  127: cpu->a=cpu->a;_fetch(); // LD A,A T:0
        case  128: _z80_add8(cpu,cpu->b);_fetch(); // ADD B T:0
        case  129: _z80_add8(cpu,cpu->c);_fetch(); // ADD C T:0
        case  130: _z80_add8(cpu,cpu->d);_fetch(); // ADD D T:0
        case  131: _z80_add8(cpu,cpu->e);_fetch(); // ADD E T:0
        case  132: _z80_add8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); // ADD H T:0
        case  133: _z80_add8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); // ADD L T:0
        case  134: _goto(766); // ADD (HL) T:0
        case  135: _z80_add8(cpu,cpu->a);_fetch(); // ADD A T:0
        case  136: _z80_adc8(cpu,cpu->b);_fetch(); // ADC B T:0
        case  137: _z80_adc8(cpu,cpu->c);_fetch(); // ADC C T:0
        case  138: _z80_adc8(cpu,cpu->d);_fetch(); // ADC D T:0
        case  139: _z80_adc8(cpu,cpu->e);_fetch(); // ADC E T:0
        case  140: _z80_adc8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); // ADC H T:0
        case  141: _z80_adc8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); // ADC L T:0
        case  142: _goto(769); // ADC (HL) T:0
        case  143: _z80_adc8(cpu,cpu->a);_fetch(); // ADC A T:0
        case  144: _z80_sub8(cpu,cpu->b);_fetch(); // SUB B T:0
        case  145: _z80_sub8(cpu,cpu->c);_fetch(); // SUB C T:0
        case  146: _z80_sub8(cpu,cpu->d);_fetch(); // SUB D T:0
        case  147: _z80_sub8(cpu,cpu->e);_fetch(); // SUB E T:0
        case  148: _z80_sub8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); // SUB H T:0
        case  149: _z80_sub8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); // SUB L T:0
        case  150: _goto(772); // SUB (HL) T:0
        case  151: _z80_sub8(cpu,cpu->a);_fetch(); // SUB A T:0
        case  152: _z80_sbc8(cpu,cpu->b);_fetch(); // SBC B T:0
        case  153: _z80_sbc8(cpu,cpu->c);_fetch(); // SBC C T:0
        case  154: _z80_sbc8(cpu,cpu->d);_fetch(); // SBC D T:0
        case  155: _z80_sbc8(cpu,cpu->e);_fetch(); // SBC E T:0
        case  156: _z80_sbc8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); // SBC H T:0
        case  157: _z80_sbc8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); // SBC L T:0
        case  158: _goto(775); // SBC (HL) T:0
        case  159: _z80_sbc8(cpu,cpu->a);_fetch(); // SBC A T:0
        case  160: _z80_and8(cpu,cpu->b);_fetch(); // AND B T:0
        case  161: _z80_and8(cpu,cpu->c);_fetch(); // AND C T:0
        case  162: _z80_and8(cpu,cpu->d);_fetch(); // AND D T:0
        case  163: _z80_and8(cpu,cpu->e);_fetch(); // AND E T:0
        case  164: _z80_and8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); // AND H T:0
        case  165: _z80_and8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); // AND L T:0
        case  166: _goto(778); // AND (HL) T:0
        case  167: _z80_and8(cpu,cpu->a);_fetch(); // AND A T:0
        case  168: _z80_xor8(cpu,cpu->b);_fetch(); // XOR B T:0
        case  169: _z80_xor8(cpu,cpu->c);_fetch(); // XOR C T:0
        case  170: _z80_xor8(cpu,cpu->d);_fetch(); // XOR D T:0
        case  171: _z80_xor8(cpu,cpu->e);_fetch(); // XOR E T:0
        case  172: _z80_xor8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); // XOR H T:0
        case  173: _z80_xor8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); // XOR L T:0
        case  174: _goto(781); // XOR (HL) T:0
        case  175: _z80_xor8(cpu,cpu->a);_fetch(); // XOR A T:0
        case  176: _z80_or8(cpu,cpu->b);_fetch(); // OR B T:0
        case  177: _z80_or8(cpu,cpu->c);_fetch(); // OR C T:0
        case  178: _z80_or8(cpu,cpu->d);_fetch(); // OR D T:0
        case  179: _z80_or8(cpu,cpu->e);_fetch(); // OR E T:0
        case  180: _z80_or8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); // OR H T:0
        case  181: _z80_or8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); // OR L T:0
        case  182: _goto(784); // OR (HL) T:0
        case  183: _z80_or8(cpu,cpu->a);_fetch(); // OR A T:0
        case  184: _z80_cp8(cpu,cpu->b);_fetch(); // CP B T:0
        case  185: _z80_cp8(cpu,cpu->c);_fetch(); // CP C T:0
        case  186: _z80_cp8(cpu,cpu->d);_fetch(); // CP D T:0
        case  187: _z80_cp8(cpu,cpu->e);_fetch(); // CP E T:0
        case  188: _z80_cp8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); // CP H T:0
        case  189: _z80_cp8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); // CP L T:0
        case  190: _goto(787); // CP (HL) T:0
        case  191: _z80_cp8(cpu,cpu->a);_fetch(); // CP A T:0
        case  192: if(!_cc_nz){_goto(790+6);};_goto(790); // RET NZ T:0
        case  193: _goto(797); // POP BC T:0
        case  194: _goto(803); // JP NZ,nn T:0
        case  195: _goto(809); // JP nn T:0
        case  196: _goto(815); // CALL NZ,nn T:0
        case  197: _goto(828); // PUSH BC T:0
        case  198: _goto(835); // ADD n T:0
        case  199: _goto(838); // RST 0h T:0
        case  200: if(!_cc_z){_goto(845+6);};_goto(845); // RET Z T:0
        case  201: _goto(852); // RET T:0
        case  202: _goto(858); // JP Z,nn T:0
        case  203: _fetch_cb();goto step_to; // CB prefix T:0
        case  204: _goto(864); // CALL Z,nn T:0
        case  205: _goto(877); // CALL nn T:0
        case  206: _goto(890); // ADC n T:0
        case  207: _goto(893); // RST 8h T:0
        case  208: if(!_cc_nc){_goto(900+6);};_goto(900); // RET NC T:0
        case  209: _goto(907); // POP DE T:0
        case  210: _goto(913); // JP NC,nn T:0
        case  211: _goto(919); // OUT (n),A T:0
        case  212: _goto(926); // CALL NC,nn T:0
        case  213: _goto(939); // PUSH DE T:0
        case  214: _goto(946); // SUB n T:0
        case  215: _goto(949); // RST 10h T:0
        case  216: if(!_cc_c){_goto(956+6);};_goto(956); // RET C T:0
        case  217: _z80_exx(cpu);_fetch(); // EXX T:0
        case  218: _goto(963); // JP C,nn T:0
        case  219: _goto(969); // IN A,(n) T:0
        case  220: _goto(976); // CALL C,nn T:0
        case  221: _fetch_dd();goto step_to; // DD prefix T:0
        case  222: _goto(989); // SBC n T:0
        case  223: _goto(992); // RST 18h T:0
        case  224: if(!_cc_po){_goto(999+6);};_goto(999); // RET PO T:0
        case  225: _goto(1006); // POP HL T:0
        case  226: _goto(1012); // JP PO,nn T:0
        case  227: _goto(1018); // EX (SP),HL T:0
        case  228: _goto(1033); // CALL PO,nn T:0
        case  229: _goto(1046); // PUSH HL T:0
        case  230: _goto(1053); // AND n T:0
        case  231: _goto(1056); // RST 20h T:0
        case  232: if(!_cc_pe){_goto(1063+6);};_goto(1063); // RET PE T:0
        case  233: cpu->pc=cpu->hlx[cpu->hlx_idx].hl;_fetch(); // JP HL T:0
        case  234: _goto(1070); // JP PE,nn T:0
        case  235: _z80_ex_de_hl(cpu);_fetch(); // EX DE,HL T:0
        case  236: _goto(1076); // CALL PE,nn T:0
        case  237: _fetch_ed();goto step_to; // ED prefix T:0
        case  238: _goto(1089); // XOR n T:0
        case  239: _goto(1092); // RST 28h T:0
        case  240: if(!_cc_p){_goto(1099+6);};_goto(1099); // RET P T:0
        case  241: _goto(1106); // POP AF T:0
        case  242: _goto(1112); // JP P,nn T:0
        case  243: cpu->iff1=cpu->iff2=false;_fetch(); // DI T:0
        case  244: _goto(1118); // CALL P,nn T:0
        case  245: _goto(1131); // PUSH AF T:0
        case  246: _goto(1138); // OR n T:0
        case  247: _goto(1141); // RST 30h T:0
        case  248: if(!_cc_m){_goto(1148+6);};_goto(1148); // RET M T:0
        case  249: cpu->sp=cpu->hlx[cpu->hlx_idx].hl;_goto(1155); // LD SP,HL T:0
        case  250: _goto(1157); // JP M,nn T:0
        case  251: cpu->iff1=cpu->iff2=false;pins=_z80_fetch(cpu,pins);cpu->iff1=cpu->iff2=true;goto step_to; // EI T:0
        case  252: _goto(1163); // CALL M,nn T:0
        case  253: _fetch_fd();goto step_to; // FD prefix T:0
        case  254: _goto(1176); // CP n T:0
        case  255: _goto(1179); // RST 38h T:0
        case  256: _fetch(); // ED NOP T:0
        case  257: _fetch(); // ED NOP T:0
        case  258: _fetch(); // ED NOP T:0
        case  259: _fetch(); // ED NOP T:0
        case  260: _fetch(); // ED NOP T:0
        case  261: _fetch(); // ED NOP T:0
        case  262: _fetch(); // ED NOP T:0
        case  263: _fetch(); // ED NOP T:0
        case  264: _fetch(); // ED NOP T:0
        case  265: _fetch(); // ED NOP T:0
        case  266: _fetch(); // ED NOP T:0
        case  267: _fetch(); // ED NOP T:0
        case  268: _fetch(); // ED NOP T:0
        case  269: _fetch(); // ED NOP T:0
        case  270: _fetch(); // ED NOP T:0
        case  271: _fetch(); // ED NOP T:0
        case  272: _fetch(); // ED NOP T:0
        case  273: _fetch(); // ED NOP T:0
        case  274: _fetch(); // ED NOP T:0
        case  275: _fetch(); // ED NOP T:0
        case  276: _fetch(); // ED NOP T:0
        case  277: _fetch(); // ED NOP T:0
        case  278: _fetch(); // ED NOP T:0
        case  279: _fetch(); // ED NOP T:0
        case  280: _fetch(); // ED NOP T:0
        case  281: _fetch(); // ED NOP T:0
        case  282: _fetch(); // ED NOP T:0
        case  283: _fetch(); // ED NOP T:0
        case  284: _fetch(); // ED NOP T:0
        case  285: _fetch(); // ED NOP T:0
        case  286: _fetch(); // ED NOP T:0
        case  287: _fetch(); // ED NOP T:0
        case  288: _fetch(); // ED NOP T:0
        case  289: _fetch(); // ED NOP T:0
        case  290: _fetch(); // ED NOP T:0
        case  291: _fetch(); // ED NOP T:0
        case  292: _fetch(); // ED NOP T:0
        case  293: _fetch(); // ED NOP T:0
        case  294: _fetch(); // ED NOP T:0
        case  295: _fetch(); // ED NOP T:0
        case  296: _fetch(); // ED NOP T:0
        case  297: _fetch(); // ED NOP T:0
        case  298: _fetch(); // ED NOP T:0
        case  299: _fetch(); // ED NOP T:0
        case  300: _fetch(); // ED NOP T:0
        case  301: _fetch(); // ED NOP T:0
        case  302: _fetch(); // ED NOP T:0
        case  303: _fetch(); // ED NOP T:0
        case  304: _fetch(); // ED NOP T:0
        case  305: _fetch(); // ED NOP T:0
        case  306: _fetch(); // ED NOP T:0
        case  307: _fetch(); // ED NOP T:0
        case  308: _fetch(); // ED NOP T:0
        case  309: _fetch(); // ED NOP T:0
        case  310: _fetch(); // ED NOP T:0
        case  311: _fetch(); // ED NOP T:0
        case  312: _fetch(); // ED NOP T:0
        case  313: _fetch(); // ED NOP T:0
        case  314: _fetch(); // ED NOP T:0
        case  315: _fetch(); // ED NOP T:0
        case  316: _fetch(); // ED NOP T:0
        case  317: _fetch(); // ED NOP T:0
        case  318: _fetch(); // ED NOP T:0
        case  319: _fetch(); // ED NOP T:0
        case  320: _goto(1186); // IN B,(C) T:0
        case  321: _goto(1190); // OUT (C),B T:0
        case  322: _z80_sbc16(cpu,cpu->bc);_goto(1194); // SBC HL,BC T:0
        case  323: _goto(1201); // LD (nn),BC T:0
        case  324: _z80_neg8(cpu);_fetch(); // NEG T:0
        case  325: _goto(1213); // RETN T:0
        case  326: cpu->im=0;_fetch(); // IM 0 T:0
        case  327: _goto(1219); // LD I,A T:0
        case  328: _goto(1220); // IN C,(C) T:0
        case  329: _goto(1224); // OUT (C),C T:0
        case  330: _z80_adc16(cpu,cpu->bc);_goto(1228); // ADC HL,BC T:0
        case  331: _goto(1235); // LD BC,(nn) T:0
        case  332: _z80_neg8(cpu);_fetch(); // NEG T:0
        case  333: _goto(1247); // RETI T:0
        case  334: cpu->im=0;_fetch(); // IM 0 T:0
        case  335: _goto(1253); // LD R,A T:0
        case  336: _goto(1254); // IN D,(C) T:0
        case  337: _goto(1258); // OUT (C),D T:0
        case  338: _z80_sbc16(cpu,cpu->de);_goto(1262); // SBC HL,DE T:0
        case  339: _goto(1269); // LD (nn),DE T:0
        case  340: _z80_neg8(cpu);_fetch(); // NEG T:0
        case  341: _goto(1247); // RETI T:0
        case  342: cpu->im=1;_fetch(); // IM 1 T:0
        case  343: _goto(1282); // LD A,I T:0
        case  344: _goto(1283); // IN E,(C) T:0
        case  345: _goto(1287); // OUT (C),E T:0
        case  346: _z80_adc16(cpu,cpu->de);_goto(1291); // ADC HL,DE T:0
        case  347: _goto(1298); // LD DE,(nn) T:0
        case  348: _z80_neg8(cpu);_fetch(); // NEG T:0
        case  349: _goto(1247); // RETI T:0
        case  350: cpu->im=2;_fetch(); // IM 2 T:0
        case  351: _goto(1311); // LD A,R T:0
        case  352: _goto(1312); // IN H,(C) T:0
        case  353: _goto(1316); // OUT (C),H T:0
        case  354: _z80_sbc16(cpu,cpu->hl);_goto(1320); // SBC HL,HL T:0
        case  355: _goto(1327); // LD (nn),HL T:0
        case  356: _z80_neg8(cpu);_fetch(); // NEG T:0
        case  357: _goto(1247); // RETI T:0
        case  358: cpu->im=0;_fetch(); // IM 0 T:0
        case  359: _goto(1340); // RRD T:0
        case  360: _goto(1350); // IN L,(C) T:0
        case  361: _goto(1354); // OUT (C),L T:0
        case  362: _z80_adc16(cpu,cpu->hl);_goto(1358); // ADC HL,HL T:0
        case  363: _goto(1365); // LD HL,(nn) T:0
        case  364: _z80_neg8(cpu);_fetch(); // NEG T:0
        case  365: _goto(1247); // RETI T:0
        case  366: cpu->im=0;_fetch(); // IM 0 T:0
        case  367: _goto(1378); // RLD T:0
        case  368: _goto(1388); // IN (C) T:0
        case  369: _goto(1392); // OUT (C),0 T:0
        case  370: _z80_sbc16(cpu,cpu->sp);_goto(1396); // SBC HL,SP T:0
        case  371: _goto(1403); // LD (nn),SP T:0
        case  372: _z80_neg8(cpu);_fetch(); // NEG T:0
        case  373: _goto(1247); // RETI T:0
        case  374: cpu->im=1;_fetch(); // IM 1 T:0
        case  375: _fetch(); // ED NOP T:0
        case  376: _goto(1416); // IN A,(C) T:0
        case  377: _goto(1420); // OUT (C),A T:0
        case  378: _z80_adc16(cpu,cpu->sp);_goto(1424); // ADC HL,SP T:0
        case  379: _goto(1431); // LD SP,(nn) T:0
        case  380: _z80_neg8(cpu);_fetch(); // NEG T:0
        case  381: _goto(1247); // RETI T:0
        case  382: cpu->im=2;_fetch(); // IM 2 T:0
        case  383: _fetch(); // ED NOP T:0
        case  384: _fetch(); // ED NOP T:0
        case  385: _fetch(); // ED NOP T:0
        case  386: _fetch(); // ED NOP T:0
        case  387: _fetch(); // ED NOP T:0
        case  388: _fetch(); // ED NOP T:0
        case  389: _fetch(); // ED NOP T:0
        case  390: _fetch(); // ED NOP T:0
        case  391: _fetch(); // ED NOP T:0
        case  392: _fetch(); // ED NOP T:0
        case  393: _fetch(); // ED NOP T:0
        case  394: _fetch(); // ED NOP T:0
        case  395: _fetch(); // ED NOP T:0
        case  396: _fetch(); // ED NOP T:0
        case  397: _fetch(); // ED NOP T:0
        case  398: _fetch(); // ED NOP T:0
        case  399: _fetch(); // ED NOP T:0
        case  400: _fetch(); // ED NOP T:0
        case  401: _fetch(); // ED NOP T:0
        case  402: _fetch(); // ED NOP T:0
        case  403: _fetch(); // ED NOP T:0
        case  404: _fetch(); // ED NOP T:0
        case  405: _fetch(); // ED NOP T:0
        case  406: _fetch(); // ED NOP T:0
        case  407: _fetch(); // ED NOP T:0
        case  408: _fetch(); // ED NOP T:0
        case  409: _fetch(); // ED NOP T:0
        case  410: _fetch(); // ED NOP T:0
        case  411: _fetch(); // ED NOP T:0
        case  412: _fetch(); // ED NOP T:0
        case  413: _fetch(); // ED NOP T:0
        case  414: _fetch(); // ED NOP T:0
        case  415: _fetch(); // ED NOP T:0
        case  416: _goto(1444); // LDI T:0
        case  417: _goto(1452); // CPI T:0
        case  418: _goto(1460); // INI T:0
        case  419: _goto(1468); // OUTI T:0
        case  420: _fetch(); // ED NOP T:0
        case  421: _fetch(); // ED NOP T:0
        case  422: _fetch(); // ED NOP T:0
        case  423: _fetch(); // ED NOP T:0
        case  424: _goto(1476); // LDD T:0
        case  425: _goto(1484); // CPD T:0
        case  426: _goto(1492); // IND T:0
        case  427: _goto(1500); // OUTD T:0
        case  428: _fetch(); // ED NOP T:0
        case  429: _fetch(); // ED NOP T:0
        case  430: _fetch(); // ED NOP T:0
        case  431: _fetch(); // ED NOP T:0
        case  432: _goto(1508); // LDIR T:0
        case  433: _goto(1521); // CPIR T:0
        case  434: _goto(1534); // INIR T:0
        case  435: _goto(1547); // OTIR T:0
        case  436: _fetch(); // ED NOP T:0
        case  437: _fetch(); // ED NOP T:0
        case  438: _fetch(); // ED NOP T:0
        case  439: _fetch(); // ED NOP T:0
        case  440: _goto(1560); // LDDR T:0
        case  441: _goto(1573); // CPDR T:0
        case  442: _goto(1586); // INDR T:0
        case  443: _goto(1599); // OTDR T:0
        case  444: _fetch(); // ED NOP T:0
        case  445: _fetch(); // ED NOP T:0
        case  446: _fetch(); // ED NOP T:0
        case  447: _fetch(); // ED NOP T:0
        case  448: _fetch(); // ED NOP T:0
        case  449: _fetch(); // ED NOP T:0
        case  450: _fetch(); // ED NOP T:0
        case  451: _fetch(); // ED NOP T:0
        case  452: _fetch(); // ED NOP T:0
        case  453: _fetch(); // ED NOP T:0
        case  454: _fetch(); // ED NOP T:0
        case  455: _fetch(); // ED NOP T:0
        case  456: _fetch(); // ED NOP T:0
        case  457: _fetch(); // ED NOP T:0
        case  458: _fetch(); // ED NOP T:0
        case  459: _fetch(); // ED NOP T:0
        case  460: _fetch(); // ED NOP T:0
        case  461: _fetch(); // ED NOP T:0
        case  462: _fetch(); // ED NOP T:0
        case  463: _fetch(); // ED NOP T:0
        case  464: _fetch(); // ED NOP T:0
        case  465: _fetch(); // ED NOP T:0
        case  466: _fetch(); // ED NOP T:0
        case  467: _fetch(); // ED NOP T:0
        case  468: _fetch(); // ED NOP T:0
        case  469: _fetch(); // ED NOP T:0
        case  470: _fetch(); // ED NOP T:0
        case  471: _fetch(); // ED NOP T:0
        case  472: _fetch(); // ED NOP T:0
        case  473: _fetch(); // ED NOP T:0
        case  474: _fetch(); // ED NOP T:0
        case  475: _fetch(); // ED NOP T:0
        case  476: _fetch(); // ED NOP T:0
        case  477: _fetch(); // ED NOP T:0
        case  478: _fetch(); // ED NOP T:0
        case  479: _fetch(); // ED NOP T:0
        case  480: _fetch(); // ED NOP T:0
        case  481: _fetch(); // ED NOP T:0
        case  482: _fetch(); // ED NOP T:0
        case  483: _fetch(); // ED NOP T:0
        case  484: _fetch(); // ED NOP T:0
        case  485: _fetch(); // ED NOP T:0
        case  486: _fetch(); // ED NOP T:0
        case  487: _fetch(); // ED NOP T:0
        case  488: _fetch(); // ED NOP T:0
        case  489: _fetch(); // ED NOP T:0
        case  490: _fetch(); // ED NOP T:0
        case  491: _fetch(); // ED NOP T:0
        case  492: _fetch(); // ED NOP T:0
        case  493: _fetch(); // ED NOP T:0
        case  494: _fetch(); // ED NOP T:0
        case  495: _fetch(); // ED NOP T:0
        case  496: _fetch(); // ED NOP T:0
        case  497: _fetch(); // ED NOP T:0
        case  498: _fetch(); // ED NOP T:0
        case  499: _fetch(); // ED NOP T:0
        case  500: _fetch(); // ED NOP T:0
        case  501: _fetch(); // ED NOP T:0
        case  502: _fetch(); // ED NOP T:0
        case  503: _fetch(); // ED NOP T:0
        case  504: _fetch(); // ED NOP T:0
        case  505: _fetch(); // ED NOP T:0
        case  506: _fetch(); // ED NOP T:0
        case  507: _fetch(); // ED NOP T:0
        case  508: _fetch(); // ED NOP T:0
        case  509: _fetch(); // ED NOP T:0
        case  510: _fetch(); // ED NOP T:0
        case  511: _fetch(); // ED NOP T:0
        case  512: _wait();_mread(cpu->pc++);_step(); // LD BC,nn T:1
        case  513: cpu->c=_gd();_step(); // LD BC,nn T:2
        case  514: _step(); // LD BC,nn T:3
        case  515: _wait();_mread(cpu->pc++);_step(); // LD BC,nn T:4
        case  516: cpu->b=_gd();_step(); // LD BC,nn T:5
        case  517: _fetch(); // LD BC,nn T:6
        case  518: _wait();_mwrite(cpu->bc,cpu->a);cpu->wzl=cpu->c+1;cpu->wzh=cpu->a;_step(); // LD (BC),A T:1
        case  519: _step(); // LD (BC),A T:2
        case  520: _fetch(); // LD (BC),A T:3
        case  521: _step(); // INC BC T:1
        case  522: _fetch(); // INC BC T:2
        case  523: _wait();_mread(cpu->pc++);_step(); // LD B,n T:1
        case  524: cpu->b=_gd();_step(); // LD B,n T:2
        case  525: _fetch(); // LD B,n T:3
        case  526: _step(); // ADD HL,BC T:1
        case  527: _step(); // ADD HL,BC T:2
        case  528: _step(); // ADD HL,BC T:3
        case  529: _step(); // ADD HL,BC T:4
        case  530: _step(); // ADD HL,BC T:5
        case  531: _step(); // ADD HL,BC T:6
        case  532: _fetch(); // ADD HL,BC T:7
        case  533: _wait();_mread(cpu->bc);_step(); // LD A,(BC) T:1
        case  534: cpu->a=_gd();cpu->wz=cpu->bc+1;_step(); // LD A,(BC) T:2
        case  535: _fetch(); // LD A,(BC) T:3
        case  536: _step(); // DEC BC T:1
        case  537: _fetch(); // DEC BC T:2
        case  538: _wait();_mread(cpu->pc++);_step(); // LD C,n T:1
        case  539: cpu->c=_gd();_step(); // LD C,n T:2
        case  540: _fetch(); // LD C,n T:3
        case  541: _step(); // DJNZ d T:1
        case  542: _wait();_mread(cpu->pc++);_step(); // DJNZ d T:2
        case  543: cpu->dlatch=_gd();if(--cpu->b==0){_goto(544+5);};_step(); // DJNZ d T:3
        case  544: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;_step(); // DJNZ d T:4
        case  545: _step(); // DJNZ d T:5
        case  546: _step(); // DJNZ d T:6
        case  547: _step(); // DJNZ d T:7
        case  548: _step(); // DJNZ d T:8
        case  549: _fetch(); // DJNZ d T:9
        case  550: _wait();_mread(cpu->pc++);_step(); // LD DE,nn T:1
        case  551: cpu->e=_gd();_step(); // LD DE,nn T:2
        case  552: _step(); // LD DE,nn T:3
        case  553: _wait();_mread(cpu->pc++);_step(); // LD DE,nn T:4
        case  554: cpu->d=_gd();_step(); // LD DE,nn T:5
        case  555: _fetch(); // LD DE,nn T:6
        case  556: _wait();_mwrite(cpu->de,cpu->a);cpu->wzl=cpu->e+1;cpu->wzh=cpu->a;_step(); // LD (DE),A T:1
        case  557: _step(); // LD (DE),A T:2
        case  558: _fetch(); // LD (DE),A T:3
        case  559: _step(); // INC DE T:1
        case  560: _fetch(); // INC DE T:2
        case  561: _wait();_mread(cpu->pc++);_step(); // LD D,n T:1
        case  562: cpu->d=_gd();_step(); // LD D,n T:2
        case  563: _fetch(); // LD D,n T:3
        case  564: _wait();_mread(cpu->pc++);_step(); // JR d T:1
        case  565: cpu->dlatch=_gd();_step(); // JR d T:2
        case  566: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;_step(); // JR d T:3
        case  567: _step(); // JR d T:4
        case  568: _step(); // JR d T:5
        case  569: _step(); // JR d T:6
        case  570: _step(); // JR d T:7
        case  571: _fetch(); // JR d T:8
        case  572: _step(); // ADD HL,DE T:1
        case  573: _step(); // ADD HL,DE T:2
        case  574: _step(); // ADD HL,DE T:3
        case  575: _step(); // ADD HL,DE T:4
        case  576: _step(); // ADD HL,DE T:5
        case  577: _step(); // ADD HL,DE T:6
        case  578: _fetch(); // ADD HL,DE T:7
        case  579: _wait();_mread(cpu->de);_step(); // LD A,(DE) T:1
        case  580: cpu->a=_gd();cpu->wz=cpu->de+1;_step(); // LD A,(DE) T:2
        case  581: _fetch(); // LD A,(DE) T:3
        case  582: _step(); // DEC DE T:1
        case  583: _fetch(); // DEC DE T:2
        case  584: _wait();_mread(cpu->pc++);_step(); // LD E,n T:1
        case  585: cpu->e=_gd();_step(); // LD E,n T:2
        case  586: _fetch(); // LD E,n T:3
        case  587: _wait();_mread(cpu->pc++);_step(); // JR NZ,d T:1
        case  588: cpu->dlatch=_gd();if(!(_cc_nz)){_goto(589+5);};_step(); // JR NZ,d T:2
        case  589: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;_step(); // JR NZ,d T:3
        case  590: _step(); // JR NZ,d T:4
        case  591: _step(); // JR NZ,d T:5
        case  592: _step(); // JR NZ,d T:6
        case  593: _step(); // JR NZ,d T:7
        case  594: _fetch(); // JR NZ,d T:8
        case  595: _wait();_mread(cpu->pc++);_step(); // LD HL,nn T:1
        case  596: cpu->hlx[cpu->hlx_idx].l=_gd();_step(); // LD HL,nn T:2
        case  597: _step(); // LD HL,nn T:3
        case  598: _wait();_mread(cpu->pc++);_step(); // LD HL,nn T:4
        case  599: cpu->hlx[cpu->hlx_idx].h=_gd();_step(); // LD HL,nn T:5
        case  600: _fetch(); // LD HL,nn T:6
        case  601: _wait();_mread(cpu->pc++);_step(); // LD (nn),HL T:1
        case  602: cpu->wzl=_gd();_step(); // LD (nn),HL T:2
        case  603: _step(); // LD (nn),HL T:3
        case  604: _wait();_mread(cpu->pc++);_step(); // LD (nn),HL T:4
        case  605: cpu->wzh=_gd();_step(); // LD (nn),HL T:5
        case  606: _step(); // LD (nn),HL T:6
        case  607: _wait();_mwrite(cpu->wz++,cpu->hlx[cpu->hlx_idx].l);_step(); // LD (nn),HL T:7
        case  608: _step(); // LD (nn),HL T:8
        case  609: _step(); // LD (nn),HL T:9
        case  610: _wait();_mwrite(cpu->wz,cpu->hlx[cpu->hlx_idx].h);_step(); // LD (nn),HL T:10
        case  611: _step(); // LD (nn),HL T:11
        case  612: _fetch(); // LD (nn),HL T:12
        case  613: _step(); // INC HL T:1
        case  614: _fetch(); // INC HL T:2
        case  615: _wait();_mread(cpu->pc++);_step(); // LD H,n T:1
        case  616: cpu->hlx[cpu->hlx_idx].h=_gd();_step(); // LD H,n T:2
        case  617: _fetch(); // LD H,n T:3
        case  618: _wait();_mread(cpu->pc++);_step(); // JR Z,d T:1
        case  619: cpu->dlatch=_gd();if(!(_cc_z)){_goto(620+5);};_step(); // JR Z,d T:2
        case  620: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;_step(); // JR Z,d T:3
        case  621: _step(); // JR Z,d T:4
        case  622: _step(); // JR Z,d T:5
        case  623: _step(); // JR Z,d T:6
        case  624: _step(); // JR Z,d T:7
        case  625: _fetch(); // JR Z,d T:8
        case  626: _step(); // ADD HL,HL T:1
        case  627: _step(); // ADD HL,HL T:2
        case  628: _step(); // ADD HL,HL T:3
        case  629: _step(); // ADD HL,HL T:4
        case  630: _step(); // ADD HL,HL T:5
        case  631: _step(); // ADD HL,HL T:6
        case  632: _fetch(); // ADD HL,HL T:7
        case  633: _wait();_mread(cpu->pc++);_step(); // LD HL,(nn) T:1
        case  634: cpu->wzl=_gd();_step(); // LD HL,(nn) T:2
        case  635: _step(); // LD HL,(nn) T:3
        case  636: _wait();_mread(cpu->pc++);_step(); // LD HL,(nn) T:4
        case  637: cpu->wzh=_gd();_step(); // LD HL,(nn) T:5
        case  638: _step(); // LD HL,(nn) T:6
        case  639: _wait();_mread(cpu->wz++);_step(); // LD HL,(nn) T:7
        case  640: cpu->hlx[cpu->hlx_idx].l=_gd();_step(); // LD HL,(nn) T:8
        case  641: _step(); // LD HL,(nn) T:9
        case  642: _wait();_mread(cpu->wz);_step(); // LD HL,(nn) T:10
        case  643: cpu->hlx[cpu->hlx_idx].h=_gd();_step(); // LD HL,(nn) T:11
        case  644: _fetch(); // LD HL,(nn) T:12
        case  645: _step(); // DEC HL T:1
        case  646: _fetch(); // DEC HL T:2
        case  647: _wait();_mread(cpu->pc++);_step(); // LD L,n T:1
        case  648: cpu->hlx[cpu->hlx_idx].l=_gd();_step(); // LD L,n T:2
        case  649: _fetch(); // LD L,n T:3
        case  650: _wait();_mread(cpu->pc++);_step(); // JR NC,d T:1
        case  651: cpu->dlatch=_gd();if(!(_cc_nc)){_goto(652+5);};_step(); // JR NC,d T:2
        case  652: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;_step(); // JR NC,d T:3
        case  653: _step(); // JR NC,d T:4
        case  654: _step(); // JR NC,d T:5
        case  655: _step(); // JR NC,d T:6
        case  656: _step(); // JR NC,d T:7
        case  657: _fetch(); // JR NC,d T:8
        case  658: _wait();_mread(cpu->pc++);_step(); // LD SP,nn T:1
        case  659: cpu->spl=_gd();_step(); // LD SP,nn T:2
        case  660: _step(); // LD SP,nn T:3
        case  661: _wait();_mread(cpu->pc++);_step(); // LD SP,nn T:4
        case  662: cpu->sph=_gd();_step(); // LD SP,nn T:5
        case  663: _fetch(); // LD SP,nn T:6
        case  664: _wait();_mread(cpu->pc++);_step(); // LD (nn),A T:1
        case  665: cpu->wzl=_gd();_step(); // LD (nn),A T:2
        case  666: _step(); // LD (nn),A T:3
        case  667: _wait();_mread(cpu->pc++);_step(); // LD (nn),A T:4
        case  668: cpu->wzh=_gd();_step(); // LD (nn),A T:5
        case  669: _step(); // LD (nn),A T:6
        case  670: _wait();_mwrite(cpu->wz++,cpu->a);cpu->wzh=cpu->a;_step(); // LD (nn),A T:7
        case  671: _step(); // LD (nn),A T:8
        case  672: _fetch(); // LD (nn),A T:9
        case  673: _step(); // INC SP T:1
        case  674: _fetch(); // INC SP T:2
        case  675: _wait();_mread(cpu->addr);_step(); // INC (HL) T:1
        case  676: cpu->dlatch=_gd();cpu->dlatch=_z80_inc8(cpu,cpu->dlatch);_step(); // INC (HL) T:2
        case  677: _step(); // INC (HL) T:3
        case  678: _step(); // INC (HL) T:4
        case  679: _wait();_mwrite(cpu->addr,cpu->dlatch);_step(); // INC (HL) T:5
        case  680: _step(); // INC (HL) T:6
        case  681: _fetch(); // INC (HL) T:7
        case  682: _wait();_mread(cpu->addr);_step(); // DEC (HL) T:1
        case  683: cpu->dlatch=_gd();cpu->dlatch=_z80_dec8(cpu,cpu->dlatch);_step(); // DEC (HL) T:2
        case  684: _step(); // DEC (HL) T:3
        case  685: _step(); // DEC (HL) T:4
        case  686: _wait();_mwrite(cpu->addr,cpu->dlatch);_step(); // DEC (HL) T:5
        case  687: _step(); // DEC (HL) T:6
        case  688: _fetch(); // DEC (HL) T:7
        case  689: _wait();_mread(cpu->pc++);_step(); // LD (HL),n T:1
        case  690: cpu->dlatch=_gd();_step(); // LD (HL),n T:2
        case  691: _step(); // LD (HL),n T:3
        case  692: _wait();_mwrite(cpu->addr,cpu->dlatch);_step(); // LD (HL),n T:4
        case  693: _step(); // LD (HL),n T:5
        case  694: _fetch(); // LD (HL),n T:6
        case  695: _wait();_mread(cpu->pc++);_step(); // JR C,d T:1
        case  696: cpu->dlatch=_gd();if(!(_cc_c)){_goto(697+5);};_step(); // JR C,d T:2
        case  697: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;_step(); // JR C,d T:3
        case  698: _step(); // JR C,d T:4
        case  699: _step(); // JR C,d T:5
        case  700: _step(); // JR C,d T:6
        case  701: _step(); // JR C,d T:7
        case  702: _fetch(); // JR C,d T:8
        case  703: _step(); // ADD HL,SP T:1
        case  704: _step(); // ADD HL,SP T:2
        case  705: _step(); // ADD HL,SP T:3
        case  706: _step(); // ADD HL,SP T:4
        case  707: _step(); // ADD HL,SP T:5
        case  708: _step(); // ADD HL,SP T:6
        case  709: _fetch(); // ADD HL,SP T:7
        case  710: _wait();_mread(cpu->pc++);_step(); // LD A,(nn) T:1
        case  711: cpu->wzl=_gd();_step(); // LD A,(nn) T:2
        case  712: _step(); // LD A,(nn) T:3
        case  713: _wait();_mread(cpu->pc++);_step(); // LD A,(nn) T:4
        case  714: cpu->wzh=_gd();_step(); // LD A,(nn) T:5
        case  715: _step(); // LD A,(nn) T:6
        case  716: _wait();_mread(cpu->wz++);_step(); // LD A,(nn) T:7
        case  717: cpu->a=_gd();_step(); // LD A,(nn) T:8
        case  718: _fetch(); // LD A,(nn) T:9
        case  719: _step(); // DEC SP T:1
        case  720: _fetch(); // DEC SP T:2
        case  721: _wait();_mread(cpu->pc++);_step(); // LD A,n T:1
        case  722: cpu->a=_gd();_step(); // LD A,n T:2
        case  723: _fetch(); // LD A,n T:3
        case  724: _wait();_mread(cpu->addr);_step(); // LD B,(HL) T:1
        case  725: cpu->b=_gd();_step(); // LD B,(HL) T:2
        case  726: _fetch(); // LD B,(HL) T:3
        case  727: _wait();_mread(cpu->addr);_step(); // LD C,(HL) T:1
        case  728: cpu->c=_gd();_step(); // LD C,(HL) T:2
        case  729: _fetch(); // LD C,(HL) T:3
        case  730: _wait();_mread(cpu->addr);_step(); // LD D,(HL) T:1
        case  731: cpu->d=_gd();_step(); // LD D,(HL) T:2
        case  732: _fetch(); // LD D,(HL) T:3
        case  733: _wait();_mread(cpu->addr);_step(); // LD E,(HL) T:1
        case  734: cpu->e=_gd();_step(); // LD E,(HL) T:2
        case  735: _fetch(); // LD E,(HL) T:3
        case  736: _wait();_mread(cpu->addr);_step(); // LD H,(HL) T:1
        case  737: cpu->h=_gd();_step(); // LD H,(HL) T:2
        case  738: _fetch(); // LD H,(HL) T:3
        case  739: _wait();_mread(cpu->addr);_step(); // LD L,(HL) T:1
        case  740: cpu->l=_gd();_step(); // LD L,(HL) T:2
        case  741: _fetch(); // LD L,(HL) T:3
        case  742: _wait();_mwrite(cpu->addr,cpu->b);_step(); // LD (HL),B T:1
        case  743: _step(); // LD (HL),B T:2
        case  744: _fetch(); // LD (HL),B T:3
        case  745: _wait();_mwrite(cpu->addr,cpu->c);_step(); // LD (HL),C T:1
        case  746: _step(); // LD (HL),C T:2
        case  747: _fetch(); // LD (HL),C T:3
        case  748: _wait();_mwrite(cpu->addr,cpu->d);_step(); // LD (HL),D T:1
        case  749: _step(); // LD (HL),D T:2
        case  750: _fetch(); // LD (HL),D T:3
        case  751: _wait();_mwrite(cpu->addr,cpu->e);_step(); // LD (HL),E T:1
        case  752: _step(); // LD (HL),E T:2
        case  753: _fetch(); // LD (HL),E T:3
        case  754: _wait();_mwrite(cpu->addr,cpu->h);_step(); // LD (HL),H T:1
        case  755: _step(); // LD (HL),H T:2
        case  756: _fetch(); // LD (HL),H T:3
        case  757: _wait();_mwrite(cpu->addr,cpu->l);_step(); // LD (HL),L T:1
        case  758: _step(); // LD (HL),L T:2
        case  759: _fetch(); // LD (HL),L T:3
        case  760: _wait();_mwrite(cpu->addr,cpu->a);_step(); // LD (HL),A T:1
        case  761: _step(); // LD (HL),A T:2
        case  762: _fetch(); // LD (HL),A T:3
        case  763: _wait();_mread(cpu->addr);_step(); // LD A,(HL) T:1
        case  764: cpu->a=_gd();_step(); // LD A,(HL) T:2
        case  765: _fetch(); // LD A,(HL) T:3
        case  766: _wait();_mread(cpu->addr);_step(); // ADD (HL) T:1
        case  767: cpu->dlatch=_gd();_step(); // ADD (HL) T:2
        case  768: _z80_add8(cpu,cpu->dlatch);_fetch(); // ADD (HL) T:3
        case  769: _wait();_mread(cpu->addr);_step(); // ADC (HL) T:1
        case  770: cpu->dlatch=_gd();_step(); // ADC (HL) T:2
        case  771: _z80_adc8(cpu,cpu->dlatch);_fetch(); // ADC (HL) T:3
        case  772: _wait();_mread(cpu->addr);_step(); // SUB (HL) T:1
        case  773: cpu->dlatch=_gd();_step(); // SUB (HL) T:2
        case  774: _z80_sub8(cpu,cpu->dlatch);_fetch(); // SUB (HL) T:3
        case  775: _wait();_mread(cpu->addr);_step(); // SBC (HL) T:1
        case  776: cpu->dlatch=_gd();_step(); // SBC (HL) T:2
        case  777: _z80_sbc8(cpu,cpu->dlatch);_fetch(); // SBC (HL) T:3
        case  778: _wait();_mread(cpu->addr);_step(); // AND (HL) T:1
        case  779: cpu->dlatch=_gd();_step(); // AND (HL) T:2
        case  780: _z80_and8(cpu,cpu->dlatch);_fetch(); // AND (HL) T:3
        case  781: _wait();_mread(cpu->addr);_step(); // XOR (HL) T:1
        case  782: cpu->dlatch=_gd();_step(); // XOR (HL) T:2
        case  783: _z80_xor8(cpu,cpu->dlatch);_fetch(); // XOR (HL) T:3
        case  784: _wait();_mread(cpu->addr);_step(); // OR (HL) T:1
        case  785: cpu->dlatch=_gd();_step(); // OR (HL) T:2
        case  786: _z80_or8(cpu,cpu->dlatch);_fetch(); // OR (HL) T:3
        case  787: _wait();_mread(cpu->addr);_step(); // CP (HL) T:1
        case  788: cpu->dlatch=_gd();_step(); // CP (HL) T:2
        case  789: _z80_cp8(cpu,cpu->dlatch);_fetch(); // CP (HL) T:3
        case  790: _step(); // RET NZ T:1
        case  791: _wait();_mread(cpu->sp++);_step(); // RET NZ T:2
        case  792: cpu->wzl=_gd();_step(); // RET NZ T:3
        case  793: _step(); // RET NZ T:4
        case  794: _wait();_mread(cpu->sp++);_step(); // RET NZ T:5
        case  795: cpu->wzh=_gd();cpu->pc=cpu->wz;_step(); // RET NZ T:6
        case  796: _fetch(); // RET NZ T:7
        case  797: _wait();_mread(cpu->sp++);_step(); // POP BC T:1
        case  798: cpu->c=_gd();_step(); // POP BC T:2
        case  799: _step(); // POP BC T:3
        case  800: _wait();_mread(cpu->sp++);_step(); // POP BC T:4
        case  801: cpu->b=_gd();_step(); // POP BC T:5
        case  802: _fetch(); // POP BC T:6
        case  803: _wait();_mread(cpu->pc++);_step(); // JP NZ,nn T:1
        case  804: cpu->wzl=_gd();_step(); // JP NZ,nn T:2
        case  805: _step(); // JP NZ,nn T:3
        case  806: _wait();_mread(cpu->pc++);_step(); // JP NZ,nn T:4
        case  807: cpu->wzh=_gd();if(_cc_nz){cpu->pc=cpu->wz;};_step(); // JP NZ,nn T:5
        case  808: _fetch(); // JP NZ,nn T:6
        case  809: _wait();_mread(cpu->pc++);_step(); // JP nn T:1
        case  810: cpu->wzl=_gd();_step(); // JP nn T:2
        case  811: _step(); // JP nn T:3
        case  812: _wait();_mread(cpu->pc++);_step(); // JP nn T:4
        case  813: cpu->wzh=_gd();cpu->pc=cpu->wz;_step(); // JP nn T:5
        case  814: _fetch(); // JP nn T:6
        case  815: _wait();_mread(cpu->pc++);_step(); // CALL NZ,nn T:1
        case  816: cpu->wzl=_gd();_step(); // CALL NZ,nn T:2
        case  817: _step(); // CALL NZ,nn T:3
        case  818: _wait();_mread(cpu->pc++);_step(); // CALL NZ,nn T:4
        case  819: cpu->wzh=_gd();if (!_cc_nz){_goto(820+7);};_step(); // CALL NZ,nn T:5
        case  820: _step(); // CALL NZ,nn T:6
        case  821: _step(); // CALL NZ,nn T:7
        case  822: _wait();_mwrite(--cpu->sp,cpu->pch);_step(); // CALL NZ,nn T:8
        case  823: _step(); // CALL NZ,nn T:9
        case  824: _step(); // CALL NZ,nn T:10
        case  825: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz;_step(); // CALL NZ,nn T:11
        case  826: _step(); // CALL NZ,nn T:12
        case  827: _fetch(); // CALL NZ,nn T:13
        case  828: _step(); // PUSH BC T:1
        case  829: _wait();_mwrite(--cpu->sp,cpu->b);_step(); // PUSH BC T:2
        case  830: _step(); // PUSH BC T:3
        case  831: _step(); // PUSH BC T:4
        case  832: _wait();_mwrite(--cpu->sp,cpu->c);_step(); // PUSH BC T:5
        case  833: _step(); // PUSH BC T:6
        case  834: _fetch(); // PUSH BC T:7
        case  835: _wait();_mread(cpu->pc++);_step(); // ADD n T:1
        case  836: cpu->dlatch=_gd();_step(); // ADD n T:2
        case  837: _z80_add8(cpu,cpu->dlatch);_fetch(); // ADD n T:3
        case  838: _step(); // RST 0h T:1
        case  839: _wait();_mwrite(--cpu->sp,cpu->pch);_step(); // RST 0h T:2
        case  840: _step(); // RST 0h T:3
        case  841: _step(); // RST 0h T:4
        case  842: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x00;cpu->pc=cpu->wz;_step(); // RST 0h T:5
        case  843: _step(); // RST 0h T:6
        case  844: _fetch(); // RST 0h T:7
        case  845: _step(); // RET Z T:1
        case  846: _wait();_mread(cpu->sp++);_step(); // RET Z T:2
        case  847: cpu->wzl=_gd();_step(); // RET Z T:3
        case  848: _step(); // RET Z T:4
        case  849: _wait();_mread(cpu->sp++);_step(); // RET Z T:5
        case  850: cpu->wzh=_gd();cpu->pc=cpu->wz;_step(); // RET Z T:6
        case  851: _fetch(); // RET Z T:7
        case  852: _wait();_mread(cpu->sp++);_step(); // RET T:1
        case  853: cpu->wzl=_gd();_step(); // RET T:2
        case  854: _step(); // RET T:3
        case  855: _wait();_mread(cpu->sp++);_step(); // RET T:4
        case  856: cpu->wzh=_gd();cpu->pc=cpu->wz;_step(); // RET T:5
        case  857: _fetch(); // RET T:6
        case  858: _wait();_mread(cpu->pc++);_step(); // JP Z,nn T:1
        case  859: cpu->wzl=_gd();_step(); // JP Z,nn T:2
        case  860: _step(); // JP Z,nn T:3
        case  861: _wait();_mread(cpu->pc++);_step(); // JP Z,nn T:4
        case  862: cpu->wzh=_gd();if(_cc_z){cpu->pc=cpu->wz;};_step(); // JP Z,nn T:5
        case  863: _fetch(); // JP Z,nn T:6
        case  864: _wait();_mread(cpu->pc++);_step(); // CALL Z,nn T:1
        case  865: cpu->wzl=_gd();_step(); // CALL Z,nn T:2
        case  866: _step(); // CALL Z,nn T:3
        case  867: _wait();_mread(cpu->pc++);_step(); // CALL Z,nn T:4
        case  868: cpu->wzh=_gd();if (!_cc_z){_goto(869+7);};_step(); // CALL Z,nn T:5
        case  869: _step(); // CALL Z,nn T:6
        case  870: _step(); // CALL Z,nn T:7
        case  871: _wait();_mwrite(--cpu->sp,cpu->pch);_step(); // CALL Z,nn T:8
        case  872: _step(); // CALL Z,nn T:9
        case  873: _step(); // CALL Z,nn T:10
        case  874: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz;_step(); // CALL Z,nn T:11
        case  875: _step(); // CALL Z,nn T:12
        case  876: _fetch(); // CALL Z,nn T:13
        case  877: _wait();_mread(cpu->pc++);_step(); // CALL nn T:1
        case  878: cpu->wzl=_gd();_step(); // CALL nn T:2
        case  879: _step(); // CALL nn T:3
        case  880: _wait();_mread(cpu->pc++);_step(); // CALL nn T:4
        case  881: cpu->wzh=_gd();_step(); // CALL nn T:5
        case  882: _step(); // CALL nn T:6
        case  883: _step(); // CALL nn T:7
        case  884: _wait();_mwrite(--cpu->sp,cpu->pch);_step(); // CALL nn T:8
        case  885: _step(); // CALL nn T:9
        case  886: _step(); // CALL nn T:10
        case  887: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz;_step(); // CALL nn T:11
        case  888: _step(); // CALL nn T:12
        case  889: _fetch(); // CALL nn T:13
        case  890: _wait();_mread(cpu->pc++);_step(); // ADC n T:1
        case  891: cpu->dlatch=_gd();_step(); // ADC n T:2
        case  892: _z80_adc8(cpu,cpu->dlatch);_fetch(); // ADC n T:3
        case  893: _step(); // RST 8h T:1
        case  894: _wait();_mwrite(--cpu->sp,cpu->pch);_step(); // RST 8h T:2
        case  895: _step(); // RST 8h T:3
        case  896: _step(); // RST 8h T:4
        case  897: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x08;cpu->pc=cpu->wz;_step(); // RST 8h T:5
        case  898: _step(); // RST 8h T:6
        case  899: _fetch(); // RST 8h T:7
        case  900: _step(); // RET NC T:1
        case  901: _wait();_mread(cpu->sp++);_step(); // RET NC T:2
        case  902: cpu->wzl=_gd();_step(); // RET NC T:3
        case  903: _step(); // RET NC T:4
        case  904: _wait();_mread(cpu->sp++);_step(); // RET NC T:5
        case  905: cpu->wzh=_gd();cpu->pc=cpu->wz;_step(); // RET NC T:6
        case  906: _fetch(); // RET NC T:7
        case  907: _wait();_mread(cpu->sp++);_step(); // POP DE T:1
        case  908: cpu->e=_gd();_step(); // POP DE T:2
        case  909: _step(); // POP DE T:3
        case  910: _wait();_mread(cpu->sp++);_step(); // POP DE T:4
        case  911: cpu->d=_gd();_step(); // POP DE T:5
        case  912: _fetch(); // POP DE T:6
        case  913: _wait();_mread(cpu->pc++);_step(); // JP NC,nn T:1
        case  914: cpu->wzl=_gd();_step(); // JP NC,nn T:2
        case  915: _step(); // JP NC,nn T:3
        case  916: _wait();_mread(cpu->pc++);_step(); // JP NC,nn T:4
        case  917: cpu->wzh=_gd();if(_cc_nc){cpu->pc=cpu->wz;};_step(); // JP NC,nn T:5
        case  918: _fetch(); // JP NC,nn T:6
        case  919: _wait();_mread(cpu->pc++);_step(); // OUT (n),A T:1
        case  920: cpu->wzl=_gd();cpu->wzh=cpu->a;_step(); // OUT (n),A T:2
        case  921: _step(); // OUT (n),A T:3
        case  922: _iowrite(cpu->wz,cpu->a);_step(); // OUT (n),A T:4
        case  923: _wait();cpu->wzl++;_step(); // OUT (n),A T:5
        case  924: _step(); // OUT (n),A T:6
        case  925: _fetch(); // OUT (n),A T:7
        case  926: _wait();_mread(cpu->pc++);_step(); // CALL NC,nn T:1
        case  927: cpu->wzl=_gd();_step(); // CALL NC,nn T:2
        case  928: _step(); // CALL NC,nn T:3
        case  929: _wait();_mread(cpu->pc++);_step(); // CALL NC,nn T:4
        case  930: cpu->wzh=_gd();if (!_cc_nc){_goto(931+7);};_step(); // CALL NC,nn T:5
        case  931: _step(); // CALL NC,nn T:6
        case  932: _step(); // CALL NC,nn T:7
        case  933: _wait();_mwrite(--cpu->sp,cpu->pch);_step(); // CALL NC,nn T:8
        case  934: _step(); // CALL NC,nn T:9
        case  935: _step(); // CALL NC,nn T:10
        case  936: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz;_step(); // CALL NC,nn T:11
        case  937: _step(); // CALL NC,nn T:12
        case  938: _fetch(); // CALL NC,nn T:13
        case  939: _step(); // PUSH DE T:1
        case  940: _wait();_mwrite(--cpu->sp,cpu->d);_step(); // PUSH DE T:2
        case  941: _step(); // PUSH DE T:3
        case  942: _step(); // PUSH DE T:4
        case  943: _wait();_mwrite(--cpu->sp,cpu->e);_step(); // PUSH DE T:5
        case  944: _step(); // PUSH DE T:6
        case  945: _fetch(); // PUSH DE T:7
        case  946: _wait();_mread(cpu->pc++);_step(); // SUB n T:1
        case  947: cpu->dlatch=_gd();_step(); // SUB n T:2
        case  948: _z80_sub8(cpu,cpu->dlatch);_fetch(); // SUB n T:3
        case  949: _step(); // RST 10h T:1
        case  950: _wait();_mwrite(--cpu->sp,cpu->pch);_step(); // RST 10h T:2
        case  951: _step(); // RST 10h T:3
        case  952: _step(); // RST 10h T:4
        case  953: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x10;cpu->pc=cpu->wz;_step(); // RST 10h T:5
        case  954: _step(); // RST 10h T:6
        case  955: _fetch(); // RST 10h T:7
        case  956: _step(); // RET C T:1
        case  957: _wait();_mread(cpu->sp++);_step(); // RET C T:2
        case  958: cpu->wzl=_gd();_step(); // RET C T:3
        case  959: _step(); // RET C T:4
        case  960: _wait();_mread(cpu->sp++);_step(); // RET C T:5
        case  961: cpu->wzh=_gd();cpu->pc=cpu->wz;_step(); // RET C T:6
        case  962: _fetch(); // RET C T:7
        case  963: _wait();_mread(cpu->pc++);_step(); // JP C,nn T:1
        case  964: cpu->wzl=_gd();_step(); // JP C,nn T:2
        case  965: _step(); // JP C,nn T:3
        case  966: _wait();_mread(cpu->pc++);_step(); // JP C,nn T:4
        case  967: cpu->wzh=_gd();if(_cc_c){cpu->pc=cpu->wz;};_step(); // JP C,nn T:5
        case  968: _fetch(); // JP C,nn T:6
        case  969: _wait();_mread(cpu->pc++);_step(); // IN A,(n) T:1
        case  970: cpu->wzl=_gd();cpu->wzh=cpu->a;_step(); // IN A,(n) T:2
        case  971: _step(); // IN A,(n) T:3
        case  972: _step(); // IN A,(n) T:4
        case  973: _wait();_ioread(cpu->wz++);_step(); // IN A,(n) T:5
        case  974: cpu->a=_gd();_step(); // IN A,(n) T:6
        case  975: _fetch(); // IN A,(n) T:7
        case  976: _wait();_mread(cpu->pc++);_step(); // CALL C,nn T:1
        case  977: cpu->wzl=_gd();_step(); // CALL C,nn T:2
        case  978: _step(); // CALL C,nn T:3
        case  979: _wait();_mread(cpu->pc++);_step(); // CALL C,nn T:4
        case  980: cpu->wzh=_gd();if (!_cc_c){_goto(981+7);};_step(); // CALL C,nn T:5
        case  981: _step(); // CALL C,nn T:6
        case  982: _step(); // CALL C,nn T:7
        case  983: _wait();_mwrite(--cpu->sp,cpu->pch);_step(); // CALL C,nn T:8
        case  984: _step(); // CALL C,nn T:9
        case  985: _step(); // CALL C,nn T:10
        case  986: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz;_step(); // CALL C,nn T:11
        case  987: _step(); // CALL C,nn T:12
        case  988: _fetch(); // CALL C,nn T:13
        case  989: _wait();_mread(cpu->pc++);_step(); // SBC n T:1
        case  990: cpu->dlatch=_gd();_step(); // SBC n T:2
        case  991: _z80_sbc8(cpu,cpu->dlatch);_fetch(); // SBC n T:3
        case  992: _step(); // RST 18h T:1
        case  993: _wait();_mwrite(--cpu->sp,cpu->pch);_step(); // RST 18h T:2
        case  994: _step(); // RST 18h T:3
        case  995: _step(); // RST 18h T:4
        case  996: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x18;cpu->pc=cpu->wz;_step(); // RST 18h T:5
        case  997: _step(); // RST 18h T:6
        case  998: _fetch(); // RST 18h T:7
        case  999: _step(); // RET PO T:1
        case 1000: _wait();_mread(cpu->sp++);_step(); // RET PO T:2
        case 1001: cpu->wzl=_gd();_step(); // RET PO T:3
        case 1002: _step(); // RET PO T:4
        case 1003: _wait();_mread(cpu->sp++);_step(); // RET PO T:5
        case 1004: cpu->wzh=_gd();cpu->pc=cpu->wz;_step(); // RET PO T:6
        case 1005: _fetch(); // RET PO T:7
        case 1006: _wait();_mread(cpu->sp++);_step(); // POP HL T:1
        case 1007: cpu->hlx[cpu->hlx_idx].l=_gd();_step(); // POP HL T:2
        case 1008: _step(); // POP HL T:3
        case 1009: _wait();_mread(cpu->sp++);_step(); // POP HL T:4
        case 1010: cpu->hlx[cpu->hlx_idx].h=_gd();_step(); // POP HL T:5
        case 1011: _fetch(); // POP HL T:6
        case 1012: _wait();_mread(cpu->pc++);_step(); // JP PO,nn T:1
        case 1013: cpu->wzl=_gd();_step(); // JP PO,nn T:2
        case 1014: _step(); // JP PO,nn T:3
        case 1015: _wait();_mread(cpu->pc++);_step(); // JP PO,nn T:4
        case 1016: cpu->wzh=_gd();if(_cc_po){cpu->pc=cpu->wz;};_step(); // JP PO,nn T:5
        case 1017: _fetch(); // JP PO,nn T:6
        case 1018: _wait();_mread(cpu->sp);_step(); // EX (SP),HL T:1
        case 1019: cpu->wzl=_gd();_step(); // EX (SP),HL T:2
        case 1020: _step(); // EX (SP),HL T:3
        case 1021: _wait();_mread(cpu->sp+1);_step(); // EX (SP),HL T:4
        case 1022: cpu->wzh=_gd();_step(); // EX (SP),HL T:5
        case 1023: _step(); // EX (SP),HL T:6
        case 1024: _step(); // EX (SP),HL T:7
        case 1025: _wait();_mwrite(cpu->sp+1,cpu->hlx[cpu->hlx_idx].h);_step(); // EX (SP),HL T:8
        case 1026: _step(); // EX (SP),HL T:9
        case 1027: _step(); // EX (SP),HL T:10
        case 1028: _wait();_mwrite(cpu->sp,cpu->hlx[cpu->hlx_idx].l);cpu->hlx[cpu->hlx_idx].hl=cpu->wz;_step(); // EX (SP),HL T:11
        case 1029: _step(); // EX (SP),HL T:12
        case 1030: _step(); // EX (SP),HL T:13
        case 1031: _step(); // EX (SP),HL T:14
        case 1032: _fetch(); // EX (SP),HL T:15
        case 1033: _wait();_mread(cpu->pc++);_step(); // CALL PO,nn T:1
        case 1034: cpu->wzl=_gd();_step(); // CALL PO,nn T:2
        case 1035: _step(); // CALL PO,nn T:3
        case 1036: _wait();_mread(cpu->pc++);_step(); // CALL PO,nn T:4
        case 1037: cpu->wzh=_gd();if (!_cc_po){_goto(1038+7);};_step(); // CALL PO,nn T:5
        case 1038: _step(); // CALL PO,nn T:6
        case 1039: _step(); // CALL PO,nn T:7
        case 1040: _wait();_mwrite(--cpu->sp,cpu->pch);_step(); // CALL PO,nn T:8
        case 1041: _step(); // CALL PO,nn T:9
        case 1042: _step(); // CALL PO,nn T:10
        case 1043: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz;_step(); // CALL PO,nn T:11
        case 1044: _step(); // CALL PO,nn T:12
        case 1045: _fetch(); // CALL PO,nn T:13
        case 1046: _step(); // PUSH HL T:1
        case 1047: _wait();_mwrite(--cpu->sp,cpu->hlx[cpu->hlx_idx].h);_step(); // PUSH HL T:2
        case 1048: _step(); // PUSH HL T:3
        case 1049: _step(); // PUSH HL T:4
        case 1050: _wait();_mwrite(--cpu->sp,cpu->hlx[cpu->hlx_idx].l);_step(); // PUSH HL T:5
        case 1051: _step(); // PUSH HL T:6
        case 1052: _fetch(); // PUSH HL T:7
        case 1053: _wait();_mread(cpu->pc++);_step(); // AND n T:1
        case 1054: cpu->dlatch=_gd();_step(); // AND n T:2
        case 1055: _z80_and8(cpu,cpu->dlatch);_fetch(); // AND n T:3
        case 1056: _step(); // RST 20h T:1
        case 1057: _wait();_mwrite(--cpu->sp,cpu->pch);_step(); // RST 20h T:2
        case 1058: _step(); // RST 20h T:3
        case 1059: _step(); // RST 20h T:4
        case 1060: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x20;cpu->pc=cpu->wz;_step(); // RST 20h T:5
        case 1061: _step(); // RST 20h T:6
        case 1062: _fetch(); // RST 20h T:7
        case 1063: _step(); // RET PE T:1
        case 1064: _wait();_mread(cpu->sp++);_step(); // RET PE T:2
        case 1065: cpu->wzl=_gd();_step(); // RET PE T:3
        case 1066: _step(); // RET PE T:4
        case 1067: _wait();_mread(cpu->sp++);_step(); // RET PE T:5
        case 1068: cpu->wzh=_gd();cpu->pc=cpu->wz;_step(); // RET PE T:6
        case 1069: _fetch(); // RET PE T:7
        case 1070: _wait();_mread(cpu->pc++);_step(); // JP PE,nn T:1
        case 1071: cpu->wzl=_gd();_step(); // JP PE,nn T:2
        case 1072: _step(); // JP PE,nn T:3
        case 1073: _wait();_mread(cpu->pc++);_step(); // JP PE,nn T:4
        case 1074: cpu->wzh=_gd();if(_cc_pe){cpu->pc=cpu->wz;};_step(); // JP PE,nn T:5
        case 1075: _fetch(); // JP PE,nn T:6
        case 1076: _wait();_mread(cpu->pc++);_step(); // CALL PE,nn T:1
        case 1077: cpu->wzl=_gd();_step(); // CALL PE,nn T:2
        case 1078: _step(); // CALL PE,nn T:3
        case 1079: _wait();_mread(cpu->pc++);_step(); // CALL PE,nn T:4
        case 1080: cpu->wzh=_gd();if (!_cc_pe){_goto(1081+7);};_step(); // CALL PE,nn T:5
        case 1081: _step(); // CALL PE,nn T:6
        case 1082: _step(); // CALL PE,nn T:7
        case 1083: _wait();_mwrite(--cpu->sp,cpu->pch);_step(); // CALL PE,nn T:8
        case 1084: _step(); // CALL PE,nn T:9
        case 1085: _step(); // CALL PE,nn T:10
        case 1086: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz;_step(); // CALL PE,nn T:11
        case 1087: _step(); // CALL PE,nn T:12
        case 1088: _fetch(); // CALL PE,nn T:13
        case 1089: _wait();_mread(cpu->pc++);_step(); // XOR n T:1
        case 1090: cpu->dlatch=_gd();_step(); // XOR n T:2
        case 1091: _z80_xor8(cpu,cpu->dlatch);_fetch(); // XOR n T:3
        case 1092: _step(); // RST 28h T:1
        case 1093: _wait();_mwrite(--cpu->sp,cpu->pch);_step(); // RST 28h T:2
        case 1094: _step(); // RST 28h T:3
        case 1095: _step(); // RST 28h T:4
        case 1096: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x28;cpu->pc=cpu->wz;_step(); // RST 28h T:5
        case 1097: _step(); // RST 28h T:6
        case 1098: _fetch(); // RST 28h T:7
        case 1099: _step(); // RET P T:1
        case 1100: _wait();_mread(cpu->sp++);_step(); // RET P T:2
        case 1101: cpu->wzl=_gd();_step(); // RET P T:3
        case 1102: _step(); // RET P T:4
        case 1103: _wait();_mread(cpu->sp++);_step(); // RET P T:5
        case 1104: cpu->wzh=_gd();cpu->pc=cpu->wz;_step(); // RET P T:6
        case 1105: _fetch(); // RET P T:7
        case 1106: _wait();_mread(cpu->sp++);_step(); // POP AF T:1
        case 1107: cpu->f=_gd();_step(); // POP AF T:2
        case 1108: _step(); // POP AF T:3
        case 1109: _wait();_mread(cpu->sp++);_step(); // POP AF T:4
        case 1110: cpu->a=_gd();_step(); // POP AF T:5
        case 1111: _fetch(); // POP AF T:6
        case 1112: _wait();_mread(cpu->pc++);_step(); // JP P,nn T:1
        case 1113: cpu->wzl=_gd();_step(); // JP P,nn T:2
        case 1114: _step(); // JP P,nn T:3
        case 1115: _wait();_mread(cpu->pc++);_step(); // JP P,nn T:4
        case 1116: cpu->wzh=_gd();if(_cc_p){cpu->pc=cpu->wz;};_step(); // JP P,nn T:5
        case 1117: _fetch(); // JP P,nn T:6
        case 1118: _wait();_mread(cpu->pc++);_step(); // CALL P,nn T:1
        case 1119: cpu->wzl=_gd();_step(); // CALL P,nn T:2
        case 1120: _step(); // CALL P,nn T:3
        case 1121: _wait();_mread(cpu->pc++);_step(); // CALL P,nn T:4
        case 1122: cpu->wzh=_gd();if (!_cc_p){_goto(1123+7);};_step(); // CALL P,nn T:5
        case 1123: _step(); // CALL P,nn T:6
        case 1124: _step(); // CALL P,nn T:7
        case 1125: _wait();_mwrite(--cpu->sp,cpu->pch);_step(); // CALL P,nn T:8
        case 1126: _step(); // CALL P,nn T:9
        case 1127: _step(); // CALL P,nn T:10
        case 1128: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz;_step(); // CALL P,nn T:11
        case 1129: _step(); // CALL P,nn T:12
        case 1130: _fetch(); // CALL P,nn T:13
        case 1131: _step(); // PUSH AF T:1
        case 1132: _wait();_mwrite(--cpu->sp,cpu->a);_step(); // PUSH AF T:2
        case 1133: _step(); // PUSH AF T:3
        case 1134: _step(); // PUSH AF T:4
        case 1135: _wait();_mwrite(--cpu->sp,cpu->f);_step(); // PUSH AF T:5
        case 1136: _step(); // PUSH AF T:6
        case 1137: _fetch(); // PUSH AF T:7
        case 1138: _wait();_mread(cpu->pc++);_step(); // OR n T:1
        case 1139: cpu->dlatch=_gd();_step(); // OR n T:2
        case 1140: _z80_or8(cpu,cpu->dlatch);_fetch(); // OR n T:3
        case 1141: _step(); // RST 30h T:1
        case 1142: _wait();_mwrite(--cpu->sp,cpu->pch);_step(); // RST 30h T:2
        case 1143: _step(); // RST 30h T:3
        case 1144: _step(); // RST 30h T:4
        case 1145: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x30;cpu->pc=cpu->wz;_step(); // RST 30h T:5
        case 1146: _step(); // RST 30h T:6
        case 1147: _fetch(); // RST 30h T:7
        case 1148: _step(); // RET M T:1
        case 1149: _wait();_mread(cpu->sp++);_step(); // RET M T:2
        case 1150: cpu->wzl=_gd();_step(); // RET M T:3
        case 1151: _step(); // RET M T:4
        case 1152: _wait();_mread(cpu->sp++);_step(); // RET M T:5
        case 1153: cpu->wzh=_gd();cpu->pc=cpu->wz;_step(); // RET M T:6
        case 1154: _fetch(); // RET M T:7
        case 1155: _step(); // LD SP,HL T:1
        case 1156: _fetch(); // LD SP,HL T:2
        case 1157: _wait();_mread(cpu->pc++);_step(); // JP M,nn T:1
        case 1158: cpu->wzl=_gd();_step(); // JP M,nn T:2
        case 1159: _step(); // JP M,nn T:3
        case 1160: _wait();_mread(cpu->pc++);_step(); // JP M,nn T:4
        case 1161: cpu->wzh=_gd();if(_cc_m){cpu->pc=cpu->wz;};_step(); // JP M,nn T:5
        case 1162: _fetch(); // JP M,nn T:6
        case 1163: _wait();_mread(cpu->pc++);_step(); // CALL M,nn T:1
        case 1164: cpu->wzl=_gd();_step(); // CALL M,nn T:2
        case 1165: _step(); // CALL M,nn T:3
        case 1166: _wait();_mread(cpu->pc++);_step(); // CALL M,nn T:4
        case 1167: cpu->wzh=_gd();if (!_cc_m){_goto(1168+7);};_step(); // CALL M,nn T:5
        case 1168: _step(); // CALL M,nn T:6
        case 1169: _step(); // CALL M,nn T:7
        case 1170: _wait();_mwrite(--cpu->sp,cpu->pch);_step(); // CALL M,nn T:8
        case 1171: _step(); // CALL M,nn T:9
        case 1172: _step(); // CALL M,nn T:10
        case 1173: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz;_step(); // CALL M,nn T:11
        case 1174: _step(); // CALL M,nn T:12
        case 1175: _fetch(); // CALL M,nn T:13
        case 1176: _wait();_mread(cpu->pc++);_step(); // CP n T:1
        case 1177: cpu->dlatch=_gd();_step(); // CP n T:2
        case 1178: _z80_cp8(cpu,cpu->dlatch);_fetch(); // CP n T:3
        case 1179: _step(); // RST 38h T:1
        case 1180: _wait();_mwrite(--cpu->sp,cpu->pch);_step(); // RST 38h T:2
        case 1181: _step(); // RST 38h T:3
        case 1182: _step(); // RST 38h T:4
        case 1183: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x38;cpu->pc=cpu->wz;_step(); // RST 38h T:5
        case 1184: _step(); // RST 38h T:6
        case 1185: _fetch(); // RST 38h T:7
        case 1186: _step(); // IN B,(C) T:1
        case 1187: _wait();_ioread(cpu->bc);_step(); // IN B,(C) T:2
        case 1188: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;_step(); // IN B,(C) T:3
        case 1189: cpu->b=_z80_in(cpu,cpu->dlatch);_fetch(); // IN B,(C) T:4
        case 1190: _iowrite(cpu->bc,cpu->b);_step(); // OUT (C),B T:1
        case 1191: _wait();cpu->wz=cpu->bc+1;_step(); // OUT (C),B T:2
        case 1192: _step(); // OUT (C),B T:3
        case 1193: _fetch(); // OUT (C),B T:4
        case 1194: _step(); // SBC HL,BC T:1
        case 1195: _step(); // SBC HL,BC T:2
        case 1196: _step(); // SBC HL,BC T:3
        case 1197: _step(); // SBC HL,BC T:4
        case 1198: _step(); // SBC HL,BC T:5
        case 1199: _step(); // SBC HL,BC T:6
        case 1200: _fetch(); // SBC HL,BC T:7
        case 1201: _wait();_mread(cpu->pc++);_step(); // LD (nn),BC T:1
        case 1202: cpu->wzl=_gd();_step(); // LD (nn),BC T:2
        case 1203: _step(); // LD (nn),BC T:3
        case 1204: _wait();_mread(cpu->pc++);_step(); // LD (nn),BC T:4
        case 1205: cpu->wzh=_gd();_step(); // LD (nn),BC T:5
        case 1206: _step(); // LD (nn),BC T:6
        case 1207: _wait();_mwrite(cpu->wz++,cpu->c);_step(); // LD (nn),BC T:7
        case 1208: _step(); // LD (nn),BC T:8
        case 1209: _step(); // LD (nn),BC T:9
        case 1210: _wait();_mwrite(cpu->wz,cpu->b);_step(); // LD (nn),BC T:10
        case 1211: _step(); // LD (nn),BC T:11
        case 1212: _fetch(); // LD (nn),BC T:12
        case 1213: _wait();_mread(cpu->sp++);_step(); // RETN T:1
        case 1214: cpu->wzl=_gd();_step(); // RETN T:2
        case 1215: _step(); // RETN T:3
        case 1216: _wait();_mread(cpu->sp++);_step(); // RETN T:4
        case 1217: cpu->wzh=_gd();cpu->pc=cpu->wz;_step(); // RETN T:5
        case 1218: pins=_z80_fetch(cpu,pins);cpu->iff1=cpu->iff2;goto step_to; // RETN T:6
        case 1219: cpu->i=cpu->a;_fetch(); // LD I,A T:1
        case 1220: _step(); // IN C,(C) T:1
        case 1221: _wait();_ioread(cpu->bc);_step(); // IN C,(C) T:2
        case 1222: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;_step(); // IN C,(C) T:3
        case 1223: cpu->c=_z80_in(cpu,cpu->dlatch);_fetch(); // IN C,(C) T:4
        case 1224: _iowrite(cpu->bc,cpu->c);_step(); // OUT (C),C T:1
        case 1225: _wait();cpu->wz=cpu->bc+1;_step(); // OUT (C),C T:2
        case 1226: _step(); // OUT (C),C T:3
        case 1227: _fetch(); // OUT (C),C T:4
        case 1228: _step(); // ADC HL,BC T:1
        case 1229: _step(); // ADC HL,BC T:2
        case 1230: _step(); // ADC HL,BC T:3
        case 1231: _step(); // ADC HL,BC T:4
        case 1232: _step(); // ADC HL,BC T:5
        case 1233: _step(); // ADC HL,BC T:6
        case 1234: _fetch(); // ADC HL,BC T:7
        case 1235: _wait();_mread(cpu->pc++);_step(); // LD BC,(nn) T:1
        case 1236: cpu->wzl=_gd();_step(); // LD BC,(nn) T:2
        case 1237: _step(); // LD BC,(nn) T:3
        case 1238: _wait();_mread(cpu->pc++);_step(); // LD BC,(nn) T:4
        case 1239: cpu->wzh=_gd();_step(); // LD BC,(nn) T:5
        case 1240: _step(); // LD BC,(nn) T:6
        case 1241: _wait();_mread(cpu->wz++);_step(); // LD BC,(nn) T:7
        case 1242: cpu->c=_gd();_step(); // LD BC,(nn) T:8
        case 1243: _step(); // LD BC,(nn) T:9
        case 1244: _wait();_mread(cpu->wz);_step(); // LD BC,(nn) T:10
        case 1245: cpu->b=_gd();_step(); // LD BC,(nn) T:11
        case 1246: _fetch(); // LD BC,(nn) T:12
        case 1247: _wait();_mread(cpu->sp++);_step(); // RETI T:1
        case 1248: cpu->wzl=_gd();pins|=Z80_RETI;_step(); // RETI T:2
        case 1249: _step(); // RETI T:3
        case 1250: _wait();_mread(cpu->sp++);_step(); // RETI T:4
        case 1251: cpu->wzh=_gd();cpu->pc=cpu->wz;_step(); // RETI T:5
        case 1252: pins=_z80_fetch(cpu,pins);cpu->iff1=cpu->iff2;goto step_to; // RETI T:6
        case 1253: cpu->r=cpu->a;_fetch(); // LD R,A T:1
        case 1254: _step(); // IN D,(C) T:1
        case 1255: _wait();_ioread(cpu->bc);_step(); // IN D,(C) T:2
        case 1256: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;_step(); // IN D,(C) T:3
        case 1257: cpu->d=_z80_in(cpu,cpu->dlatch);_fetch(); // IN D,(C) T:4
        case 1258: _iowrite(cpu->bc,cpu->d);_step(); // OUT (C),D T:1
        case 1259: _wait();cpu->wz=cpu->bc+1;_step(); // OUT (C),D T:2
        case 1260: _step(); // OUT (C),D T:3
        case 1261: _fetch(); // OUT (C),D T:4
        case 1262: _step(); // SBC HL,DE T:1
        case 1263: _step(); // SBC HL,DE T:2
        case 1264: _step(); // SBC HL,DE T:3
        case 1265: _step(); // SBC HL,DE T:4
        case 1266: _step(); // SBC HL,DE T:5
        case 1267: _step(); // SBC HL,DE T:6
        case 1268: _fetch(); // SBC HL,DE T:7
        case 1269: _wait();_mread(cpu->pc++);_step(); // LD (nn),DE T:1
        case 1270: cpu->wzl=_gd();_step(); // LD (nn),DE T:2
        case 1271: _step(); // LD (nn),DE T:3
        case 1272: _wait();_mread(cpu->pc++);_step(); // LD (nn),DE T:4
        case 1273: cpu->wzh=_gd();_step(); // LD (nn),DE T:5
        case 1274: _step(); // LD (nn),DE T:6
        case 1275: _wait();_mwrite(cpu->wz++,cpu->e);_step(); // LD (nn),DE T:7
        case 1276: _step(); // LD (nn),DE T:8
        case 1277: _step(); // LD (nn),DE T:9
        case 1278: _wait();_mwrite(cpu->wz,cpu->d);_step(); // LD (nn),DE T:10
        case 1279: _step(); // LD (nn),DE T:11
        case 1280: _fetch(); // LD (nn),DE T:12
        case 1281: pins=_z80_fetch(cpu,pins);cpu->iff1=cpu->iff2;goto step_to; // RETI T:6
        case 1282: cpu->a=cpu->i;cpu->f=_z80_sziff2_flags(cpu, cpu->i);_fetch(); // LD A,I T:1
        case 1283: _step(); // IN E,(C) T:1
        case 1284: _wait();_ioread(cpu->bc);_step(); // IN E,(C) T:2
        case 1285: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;_step(); // IN E,(C) T:3
        case 1286: cpu->e=_z80_in(cpu,cpu->dlatch);_fetch(); // IN E,(C) T:4
        case 1287: _iowrite(cpu->bc,cpu->e);_step(); // OUT (C),E T:1
        case 1288: _wait();cpu->wz=cpu->bc+1;_step(); // OUT (C),E T:2
        case 1289: _step(); // OUT (C),E T:3
        case 1290: _fetch(); // OUT (C),E T:4
        case 1291: _step(); // ADC HL,DE T:1
        case 1292: _step(); // ADC HL,DE T:2
        case 1293: _step(); // ADC HL,DE T:3
        case 1294: _step(); // ADC HL,DE T:4
        case 1295: _step(); // ADC HL,DE T:5
        case 1296: _step(); // ADC HL,DE T:6
        case 1297: _fetch(); // ADC HL,DE T:7
        case 1298: _wait();_mread(cpu->pc++);_step(); // LD DE,(nn) T:1
        case 1299: cpu->wzl=_gd();_step(); // LD DE,(nn) T:2
        case 1300: _step(); // LD DE,(nn) T:3
        case 1301: _wait();_mread(cpu->pc++);_step(); // LD DE,(nn) T:4
        case 1302: cpu->wzh=_gd();_step(); // LD DE,(nn) T:5
        case 1303: _step(); // LD DE,(nn) T:6
        case 1304: _wait();_mread(cpu->wz++);_step(); // LD DE,(nn) T:7
        case 1305: cpu->e=_gd();_step(); // LD DE,(nn) T:8
        case 1306: _step(); // LD DE,(nn) T:9
        case 1307: _wait();_mread(cpu->wz);_step(); // LD DE,(nn) T:10
        case 1308: cpu->d=_gd();_step(); // LD DE,(nn) T:11
        case 1309: _fetch(); // LD DE,(nn) T:12
        case 1310: pins=_z80_fetch(cpu,pins);cpu->iff1=cpu->iff2;goto step_to; // RETI T:6
        case 1311: cpu->a=cpu->r;cpu->f=_z80_sziff2_flags(cpu, cpu->r);_fetch(); // LD A,R T:1
        case 1312: _step(); // IN H,(C) T:1
        case 1313: _wait();_ioread(cpu->bc);_step(); // IN H,(C) T:2
        case 1314: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;_step(); // IN H,(C) T:3
        case 1315: cpu->h=_z80_in(cpu,cpu->dlatch);_fetch(); // IN H,(C) T:4
        case 1316: _iowrite(cpu->bc,cpu->h);_step(); // OUT (C),H T:1
        case 1317: _wait();cpu->wz=cpu->bc+1;_step(); // OUT (C),H T:2
        case 1318: _step(); // OUT (C),H T:3
        case 1319: _fetch(); // OUT (C),H T:4
        case 1320: _step(); // SBC HL,HL T:1
        case 1321: _step(); // SBC HL,HL T:2
        case 1322: _step(); // SBC HL,HL T:3
        case 1323: _step(); // SBC HL,HL T:4
        case 1324: _step(); // SBC HL,HL T:5
        case 1325: _step(); // SBC HL,HL T:6
        case 1326: _fetch(); // SBC HL,HL T:7
        case 1327: _wait();_mread(cpu->pc++);_step(); // LD (nn),HL T:1
        case 1328: cpu->wzl=_gd();_step(); // LD (nn),HL T:2
        case 1329: _step(); // LD (nn),HL T:3
        case 1330: _wait();_mread(cpu->pc++);_step(); // LD (nn),HL T:4
        case 1331: cpu->wzh=_gd();_step(); // LD (nn),HL T:5
        case 1332: _step(); // LD (nn),HL T:6
        case 1333: _wait();_mwrite(cpu->wz++,cpu->l);_step(); // LD (nn),HL T:7
        case 1334: _step(); // LD (nn),HL T:8
        case 1335: _step(); // LD (nn),HL T:9
        case 1336: _wait();_mwrite(cpu->wz,cpu->h);_step(); // LD (nn),HL T:10
        case 1337: _step(); // LD (nn),HL T:11
        case 1338: _fetch(); // LD (nn),HL T:12
        case 1339: pins=_z80_fetch(cpu,pins);cpu->iff1=cpu->iff2;goto step_to; // RETI T:6
        case 1340: _wait();_mread(cpu->hl);_step(); // RRD T:1
        case 1341: cpu->dlatch=_gd();_step(); // RRD T:2
        case 1342: cpu->dlatch=_z80_rrd(cpu,cpu->dlatch);_step(); // RRD T:3
        case 1343: _step(); // RRD T:4
        case 1344: _step(); // RRD T:5
        case 1345: _step(); // RRD T:6
        case 1346: _step(); // RRD T:7
        case 1347: _wait();_mwrite(cpu->hl,cpu->dlatch);cpu->wz=cpu->hl+1;_step(); // RRD T:8
        case 1348: _step(); // RRD T:9
        case 1349: _fetch(); // RRD T:10
        case 1350: _step(); // IN L,(C) T:1
        case 1351: _wait();_ioread(cpu->bc);_step(); // IN L,(C) T:2
        case 1352: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;_step(); // IN L,(C) T:3
        case 1353: cpu->l=_z80_in(cpu,cpu->dlatch);_fetch(); // IN L,(C) T:4
        case 1354: _iowrite(cpu->bc,cpu->l);_step(); // OUT (C),L T:1
        case 1355: _wait();cpu->wz=cpu->bc+1;_step(); // OUT (C),L T:2
        case 1356: _step(); // OUT (C),L T:3
        case 1357: _fetch(); // OUT (C),L T:4
        case 1358: _step(); // ADC HL,HL T:1
        case 1359: _step(); // ADC HL,HL T:2
        case 1360: _step(); // ADC HL,HL T:3
        case 1361: _step(); // ADC HL,HL T:4
        case 1362: _step(); // ADC HL,HL T:5
        case 1363: _step(); // ADC HL,HL T:6
        case 1364: _fetch(); // ADC HL,HL T:7
        case 1365: _wait();_mread(cpu->pc++);_step(); // LD HL,(nn) T:1
        case 1366: cpu->wzl=_gd();_step(); // LD HL,(nn) T:2
        case 1367: _step(); // LD HL,(nn) T:3
        case 1368: _wait();_mread(cpu->pc++);_step(); // LD HL,(nn) T:4
        case 1369: cpu->wzh=_gd();_step(); // LD HL,(nn) T:5
        case 1370: _step(); // LD HL,(nn) T:6
        case 1371: _wait();_mread(cpu->wz++);_step(); // LD HL,(nn) T:7
        case 1372: cpu->l=_gd();_step(); // LD HL,(nn) T:8
        case 1373: _step(); // LD HL,(nn) T:9
        case 1374: _wait();_mread(cpu->wz);_step(); // LD HL,(nn) T:10
        case 1375: cpu->h=_gd();_step(); // LD HL,(nn) T:11
        case 1376: _fetch(); // LD HL,(nn) T:12
        case 1377: pins=_z80_fetch(cpu,pins);cpu->iff1=cpu->iff2;goto step_to; // RETI T:6
        case 1378: _wait();_mread(cpu->hl);_step(); // RLD T:1
        case 1379: cpu->dlatch=_gd();_step(); // RLD T:2
        case 1380: cpu->dlatch=_z80_rld(cpu,cpu->dlatch);_step(); // RLD T:3
        case 1381: _step(); // RLD T:4
        case 1382: _step(); // RLD T:5
        case 1383: _step(); // RLD T:6
        case 1384: _step(); // RLD T:7
        case 1385: _wait();_mwrite(cpu->hl,cpu->dlatch);cpu->wz=cpu->hl+1;_step(); // RLD T:8
        case 1386: _step(); // RLD T:9
        case 1387: _fetch(); // RLD T:10
        case 1388: _step(); // IN (C) T:1
        case 1389: _wait();_ioread(cpu->bc);_step(); // IN (C) T:2
        case 1390: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;_step(); // IN (C) T:3
        case 1391: _z80_in(cpu,cpu->dlatch);_fetch(); // IN (C) T:4
        case 1392: _iowrite(cpu->bc,0);_step(); // OUT (C),0 T:1
        case 1393: _wait();cpu->wz=cpu->bc+1;_step(); // OUT (C),0 T:2
        case 1394: _step(); // OUT (C),0 T:3
        case 1395: _fetch(); // OUT (C),0 T:4
        case 1396: _step(); // SBC HL,SP T:1
        case 1397: _step(); // SBC HL,SP T:2
        case 1398: _step(); // SBC HL,SP T:3
        case 1399: _step(); // SBC HL,SP T:4
        case 1400: _step(); // SBC HL,SP T:5
        case 1401: _step(); // SBC HL,SP T:6
        case 1402: _fetch(); // SBC HL,SP T:7
        case 1403: _wait();_mread(cpu->pc++);_step(); // LD (nn),SP T:1
        case 1404: cpu->wzl=_gd();_step(); // LD (nn),SP T:2
        case 1405: _step(); // LD (nn),SP T:3
        case 1406: _wait();_mread(cpu->pc++);_step(); // LD (nn),SP T:4
        case 1407: cpu->wzh=_gd();_step(); // LD (nn),SP T:5
        case 1408: _step(); // LD (nn),SP T:6
        case 1409: _wait();_mwrite(cpu->wz++,cpu->spl);_step(); // LD (nn),SP T:7
        case 1410: _step(); // LD (nn),SP T:8
        case 1411: _step(); // LD (nn),SP T:9
        case 1412: _wait();_mwrite(cpu->wz,cpu->sph);_step(); // LD (nn),SP T:10
        case 1413: _step(); // LD (nn),SP T:11
        case 1414: _fetch(); // LD (nn),SP T:12
        case 1415: pins=_z80_fetch(cpu,pins);cpu->iff1=cpu->iff2;goto step_to; // RETI T:6
        case 1416: _step(); // IN A,(C) T:1
        case 1417: _wait();_ioread(cpu->bc);_step(); // IN A,(C) T:2
        case 1418: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;_step(); // IN A,(C) T:3
        case 1419: cpu->a=_z80_in(cpu,cpu->dlatch);_fetch(); // IN A,(C) T:4
        case 1420: _iowrite(cpu->bc,cpu->a);_step(); // OUT (C),A T:1
        case 1421: _wait();cpu->wz=cpu->bc+1;_step(); // OUT (C),A T:2
        case 1422: _step(); // OUT (C),A T:3
        case 1423: _fetch(); // OUT (C),A T:4
        case 1424: _step(); // ADC HL,SP T:1
        case 1425: _step(); // ADC HL,SP T:2
        case 1426: _step(); // ADC HL,SP T:3
        case 1427: _step(); // ADC HL,SP T:4
        case 1428: _step(); // ADC HL,SP T:5
        case 1429: _step(); // ADC HL,SP T:6
        case 1430: _fetch(); // ADC HL,SP T:7
        case 1431: _wait();_mread(cpu->pc++);_step(); // LD SP,(nn) T:1
        case 1432: cpu->wzl=_gd();_step(); // LD SP,(nn) T:2
        case 1433: _step(); // LD SP,(nn) T:3
        case 1434: _wait();_mread(cpu->pc++);_step(); // LD SP,(nn) T:4
        case 1435: cpu->wzh=_gd();_step(); // LD SP,(nn) T:5
        case 1436: _step(); // LD SP,(nn) T:6
        case 1437: _wait();_mread(cpu->wz++);_step(); // LD SP,(nn) T:7
        case 1438: cpu->spl=_gd();_step(); // LD SP,(nn) T:8
        case 1439: _step(); // LD SP,(nn) T:9
        case 1440: _wait();_mread(cpu->wz);_step(); // LD SP,(nn) T:10
        case 1441: cpu->sph=_gd();_step(); // LD SP,(nn) T:11
        case 1442: _fetch(); // LD SP,(nn) T:12
        case 1443: pins=_z80_fetch(cpu,pins);cpu->iff1=cpu->iff2;goto step_to; // RETI T:6
        case 1444: _wait();_mread(cpu->hl++);_step(); // LDI T:1
        case 1445: cpu->dlatch=_gd();_step(); // LDI T:2
        case 1446: _step(); // LDI T:3
        case 1447: _wait();_mwrite(cpu->de++,cpu->dlatch);_step(); // LDI T:4
        case 1448: _step(); // LDI T:5
        case 1449: _z80_ldi_ldd(cpu,cpu->dlatch);_step(); // LDI T:6
        case 1450: _step(); // LDI T:7
        case 1451: _fetch(); // LDI T:8
        case 1452: _wait();_mread(cpu->hl++);_step(); // CPI T:1
        case 1453: cpu->dlatch=_gd();_step(); // CPI T:2
        case 1454: cpu->wz++;_z80_cpi_cpd(cpu,cpu->dlatch);_step(); // CPI T:3
        case 1455: _step(); // CPI T:4
        case 1456: _step(); // CPI T:5
        case 1457: _step(); // CPI T:6
        case 1458: _step(); // CPI T:7
        case 1459: _fetch(); // CPI T:8
        case 1460: _step(); // INI T:1
        case 1461: _step(); // INI T:2
        case 1462: _wait();_ioread(cpu->bc);_step(); // INI T:3
        case 1463: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;cpu->b--;;_step(); // INI T:4
        case 1464: _step(); // INI T:5
        case 1465: _wait();_mwrite(cpu->hl++,cpu->dlatch);_z80_ini_ind(cpu,cpu->dlatch,cpu->c+1);_step(); // INI T:6
        case 1466: _step(); // INI T:7
        case 1467: _fetch(); // INI T:8
        case 1468: _step(); // OUTI T:1
        case 1469: _wait();_mread(cpu->hl++);_step(); // OUTI T:2
        case 1470: cpu->dlatch=_gd();cpu->b--;_step(); // OUTI T:3
        case 1471: _step(); // OUTI T:4
        case 1472: _iowrite(cpu->bc,cpu->dlatch);_step(); // OUTI T:5
        case 1473: _wait();cpu->wz=cpu->bc+1;_z80_outi_outd(cpu,cpu->dlatch);_step(); // OUTI T:6
        case 1474: _step(); // OUTI T:7
        case 1475: _fetch(); // OUTI T:8
        case 1476: _wait();_mread(cpu->hl--);_step(); // LDD T:1
        case 1477: cpu->dlatch=_gd();_step(); // LDD T:2
        case 1478: _step(); // LDD T:3
        case 1479: _wait();_mwrite(cpu->de--,cpu->dlatch);_step(); // LDD T:4
        case 1480: _step(); // LDD T:5
        case 1481: _z80_ldi_ldd(cpu,cpu->dlatch);_step(); // LDD T:6
        case 1482: _step(); // LDD T:7
        case 1483: _fetch(); // LDD T:8
        case 1484: _wait();_mread(cpu->hl--);_step(); // CPD T:1
        case 1485: cpu->dlatch=_gd();_step(); // CPD T:2
        case 1486: cpu->wz--;_z80_cpi_cpd(cpu,cpu->dlatch);_step(); // CPD T:3
        case 1487: _step(); // CPD T:4
        case 1488: _step(); // CPD T:5
        case 1489: _step(); // CPD T:6
        case 1490: _step(); // CPD T:7
        case 1491: _fetch(); // CPD T:8
        case 1492: _step(); // IND T:1
        case 1493: _step(); // IND T:2
        case 1494: _wait();_ioread(cpu->bc);_step(); // IND T:3
        case 1495: cpu->dlatch=_gd();cpu->wz=cpu->bc-1;cpu->b--;;_step(); // IND T:4
        case 1496: _step(); // IND T:5
        case 1497: _wait();_mwrite(cpu->hl--,cpu->dlatch);_z80_ini_ind(cpu,cpu->dlatch,cpu->c-1);_step(); // IND T:6
        case 1498: _step(); // IND T:7
        case 1499: _fetch(); // IND T:8
        case 1500: _step(); // OUTD T:1
        case 1501: _wait();_mread(cpu->hl--);_step(); // OUTD T:2
        case 1502: cpu->dlatch=_gd();cpu->b--;_step(); // OUTD T:3
        case 1503: _step(); // OUTD T:4
        case 1504: _iowrite(cpu->bc,cpu->dlatch);_step(); // OUTD T:5
        case 1505: _wait();cpu->wz=cpu->bc-1;_z80_outi_outd(cpu,cpu->dlatch);_step(); // OUTD T:6
        case 1506: _step(); // OUTD T:7
        case 1507: _fetch(); // OUTD T:8
        case 1508: _wait();_mread(cpu->hl++);_step(); // LDIR T:1
        case 1509: cpu->dlatch=_gd();_step(); // LDIR T:2
        case 1510: _step(); // LDIR T:3
        case 1511: _wait();_mwrite(cpu->de++,cpu->dlatch);_step(); // LDIR T:4
        case 1512: _step(); // LDIR T:5
        case 1513: if(!_z80_ldi_ldd(cpu,cpu->dlatch)){_goto(1514+5);};_step(); // LDIR T:6
        case 1514: _step(); // LDIR T:7
        case 1515: cpu->wz=--cpu->pc;--cpu->pc;;_step(); // LDIR T:8
        case 1516: _step(); // LDIR T:9
        case 1517: _step(); // LDIR T:10
        case 1518: _step(); // LDIR T:11
        case 1519: _step(); // LDIR T:12
        case 1520: _fetch(); // LDIR T:13
        case 1521: _wait();_mread(cpu->hl++);_step(); // CPIR T:1
        case 1522: cpu->dlatch=_gd();_step(); // CPIR T:2
        case 1523: cpu->wz++;if(!_z80_cpi_cpd(cpu,cpu->dlatch)){_goto(1524+5);};_step(); // CPIR T:3
        case 1524: _step(); // CPIR T:4
        case 1525: _step(); // CPIR T:5
        case 1526: _step(); // CPIR T:6
        case 1527: _step(); // CPIR T:7
        case 1528: cpu->wz=--cpu->pc;--cpu->pc;_step(); // CPIR T:8
        case 1529: _step(); // CPIR T:9
        case 1530: _step(); // CPIR T:10
        case 1531: _step(); // CPIR T:11
        case 1532: _step(); // CPIR T:12
        case 1533: _fetch(); // CPIR T:13
        case 1534: _step(); // INIR T:1
        case 1535: _step(); // INIR T:2
        case 1536: _wait();_ioread(cpu->bc);_step(); // INIR T:3
        case 1537: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;cpu->b--;;_step(); // INIR T:4
        case 1538: _step(); // INIR T:5
        case 1539: _wait();_mwrite(cpu->hl++,cpu->dlatch);if (!_z80_ini_ind(cpu,cpu->dlatch,cpu->c+1)){_goto(1540+5);};_step(); // INIR T:6
        case 1540: _step(); // INIR T:7
        case 1541: cpu->wz=--cpu->pc;--cpu->pc;_step(); // INIR T:8
        case 1542: _step(); // INIR T:9
        case 1543: _step(); // INIR T:10
        case 1544: _step(); // INIR T:11
        case 1545: _step(); // INIR T:12
        case 1546: _fetch(); // INIR T:13
        case 1547: _step(); // OTIR T:1
        case 1548: _wait();_mread(cpu->hl++);_step(); // OTIR T:2
        case 1549: cpu->dlatch=_gd();cpu->b--;_step(); // OTIR T:3
        case 1550: _step(); // OTIR T:4
        case 1551: _iowrite(cpu->bc,cpu->dlatch);_step(); // OTIR T:5
        case 1552: _wait();cpu->wz=cpu->bc+1;if(!_z80_outi_outd(cpu,cpu->dlatch)){_goto(1553+5);};_step(); // OTIR T:6
        case 1553: _step(); // OTIR T:7
        case 1554: cpu->wz=--cpu->pc;--cpu->pc;_step(); // OTIR T:8
        case 1555: _step(); // OTIR T:9
        case 1556: _step(); // OTIR T:10
        case 1557: _step(); // OTIR T:11
        case 1558: _step(); // OTIR T:12
        case 1559: _fetch(); // OTIR T:13
        case 1560: _wait();_mread(cpu->hl--);_step(); // LDDR T:1
        case 1561: cpu->dlatch=_gd();_step(); // LDDR T:2
        case 1562: _step(); // LDDR T:3
        case 1563: _wait();_mwrite(cpu->de--,cpu->dlatch);_step(); // LDDR T:4
        case 1564: _step(); // LDDR T:5
        case 1565: if(!_z80_ldi_ldd(cpu,cpu->dlatch)){_goto(1566+5);};_step(); // LDDR T:6
        case 1566: _step(); // LDDR T:7
        case 1567: cpu->wz=--cpu->pc;--cpu->pc;;_step(); // LDDR T:8
        case 1568: _step(); // LDDR T:9
        case 1569: _step(); // LDDR T:10
        case 1570: _step(); // LDDR T:11
        case 1571: _step(); // LDDR T:12
        case 1572: _fetch(); // LDDR T:13
        case 1573: _wait();_mread(cpu->hl--);_step(); // CPDR T:1
        case 1574: cpu->dlatch=_gd();_step(); // CPDR T:2
        case 1575: cpu->wz--;if(!_z80_cpi_cpd(cpu,cpu->dlatch)){_goto(1576+5);};_step(); // CPDR T:3
        case 1576: _step(); // CPDR T:4
        case 1577: _step(); // CPDR T:5
        case 1578: _step(); // CPDR T:6
        case 1579: _step(); // CPDR T:7
        case 1580: cpu->wz=--cpu->pc;--cpu->pc;_step(); // CPDR T:8
        case 1581: _step(); // CPDR T:9
        case 1582: _step(); // CPDR T:10
        case 1583: _step(); // CPDR T:11
        case 1584: _step(); // CPDR T:12
        case 1585: _fetch(); // CPDR T:13
        case 1586: _step(); // INDR T:1
        case 1587: _step(); // INDR T:2
        case 1588: _wait();_ioread(cpu->bc);_step(); // INDR T:3
        case 1589: cpu->dlatch=_gd();cpu->wz=cpu->bc-1;cpu->b--;;_step(); // INDR T:4
        case 1590: _step(); // INDR T:5
        case 1591: _wait();_mwrite(cpu->hl--,cpu->dlatch);if (!_z80_ini_ind(cpu,cpu->dlatch,cpu->c-1)){_goto(1592+5);};_step(); // INDR T:6
        case 1592: _step(); // INDR T:7
        case 1593: cpu->wz=--cpu->pc;--cpu->pc;_step(); // INDR T:8
        case 1594: _step(); // INDR T:9
        case 1595: _step(); // INDR T:10
        case 1596: _step(); // INDR T:11
        case 1597: _step(); // INDR T:12
        case 1598: _fetch(); // INDR T:13
        case 1599: _step(); // OTDR T:1
        case 1600: _wait();_mread(cpu->hl--);_step(); // OTDR T:2
        case 1601: cpu->dlatch=_gd();cpu->b--;_step(); // OTDR T:3
        case 1602: _step(); // OTDR T:4
        case 1603: _iowrite(cpu->bc,cpu->dlatch);_step(); // OTDR T:5
        case 1604: _wait();cpu->wz=cpu->bc-1;if(!_z80_outi_outd(cpu,cpu->dlatch)){_goto(1605+5);};_step(); // OTDR T:6
        case 1605: _step(); // OTDR T:7
        case 1606: cpu->wz=--cpu->pc;--cpu->pc;_step(); // OTDR T:8
        case 1607: _step(); // OTDR T:9
        case 1608: _step(); // OTDR T:10
        case 1609: _step(); // OTDR T:11
        case 1610: _step(); // OTDR T:12
        case 1611: _fetch(); // OTDR T:13
        case 1612: {uint8_t z=cpu->opcode&7;_z80_cb_action(cpu,z,z);};_fetch(); // cb T:0
        case 1613: _step(); // cbhl T:0
        case 1614: _wait();_mread(cpu->hl);_step(); // cbhl T:1
        case 1615: cpu->dlatch=_gd();if(!_z80_cb_action(cpu,6,6)){_goto(1616+3);};_step(); // cbhl T:2
        case 1616: _step(); // cbhl T:3
        case 1617: _step(); // cbhl T:4
        case 1618: _wait();_mwrite(cpu->hl,cpu->dlatch);_step(); // cbhl T:5
        case 1619: _step(); // cbhl T:6
        case 1620: _fetch(); // cbhl T:7
        case 1621: _wait();_mread(cpu->pc++);_step(); // ddfdcb T:0
        case 1622: _z80_ddfdcb_addr(cpu,pins);_step(); // ddfdcb T:1
        case 1623: _step(); // ddfdcb T:2
        case 1624: _wait();_mread(cpu->pc++);_step(); // ddfdcb T:3
        case 1625: cpu->opcode=_gd();_step(); // ddfdcb T:4
        case 1626: _step(); // ddfdcb T:5
        case 1627: _step(); // ddfdcb T:6
        case 1628: _step(); // ddfdcb T:7
        case 1629: _wait();_mread(cpu->addr);_step(); // ddfdcb T:8
        case 1630: cpu->dlatch=_gd();if(!_z80_cb_action(cpu,6,cpu->opcode&7)){_goto(1631+3);};_step(); // ddfdcb T:9
        case 1631: _step(); // ddfdcb T:10
        case 1632: _step(); // ddfdcb T:11
        case 1633: _wait();_mwrite(cpu->addr,cpu->dlatch);_step(); // ddfdcb T:12
        case 1634: _step(); // ddfdcb T:13
        case 1635: _fetch(); // ddfdcb T:14
        case 1636: cpu->iff1=cpu->iff2=false;_step(); // int_im0 T:0
        case 1637: pins|=(Z80_M1|Z80_IORQ);_step(); // int_im0 T:1
        case 1638: _wait();cpu->opcode=_z80_get_db(pins);_step(); // int_im0 T:2
        case 1639: pins=_z80_refresh(cpu,pins);_step(); // int_im0 T:3
        case 1640: cpu->addr=cpu->hl;_goto(cpu->opcode);_step(); // int_im0 T:4
        case 1641: _fetch(); // int_im0 T:5
        case 1642: cpu->iff1=cpu->iff2=false;_step(); // int_im1 T:0
        case 1643: pins|=(Z80_M1|Z80_IORQ);_step(); // int_im1 T:1
        case 1644: _wait();_step(); // int_im1 T:2
        case 1645: pins=_z80_refresh(cpu,pins);_step(); // int_im1 T:3
        case 1646: _step(); // int_im1 T:4
        case 1647: _step(); // int_im1 T:5
        case 1648: _step(); // int_im1 T:6
        case 1649: _wait();_mwrite(--cpu->sp,cpu->pch);_step(); // int_im1 T:7
        case 1650: _step(); // int_im1 T:8
        case 1651: _step(); // int_im1 T:9
        case 1652: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=cpu->pc=0x0038;_step(); // int_im1 T:10
        case 1653: _step(); // int_im1 T:11
        case 1654: _fetch(); // int_im1 T:12
        case 1655: cpu->iff1=cpu->iff2=false;_step(); // int_im2 T:0
        case 1656: pins|=(Z80_M1|Z80_IORQ);_step(); // int_im2 T:1
        case 1657: _wait();cpu->dlatch=_z80_get_db(pins);_step(); // int_im2 T:2
        case 1658: pins=_z80_refresh(cpu,pins);_step(); // int_im2 T:3
        case 1659: _step(); // int_im2 T:4
        case 1660: _step(); // int_im2 T:5
        case 1661: _step(); // int_im2 T:6
        case 1662: _wait();_mwrite(--cpu->sp,cpu->pch);_step(); // int_im2 T:7
        case 1663: _step(); // int_im2 T:8
        case 1664: _step(); // int_im2 T:9
        case 1665: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wzl=cpu->dlatch;cpu->wzh=cpu->i;_step(); // int_im2 T:10
        case 1666: _step(); // int_im2 T:11
        case 1667: _step(); // int_im2 T:12
        case 1668: _wait();_mread(cpu->wz++);_step(); // int_im2 T:13
        case 1669: cpu->dlatch=_gd();_step(); // int_im2 T:14
        case 1670: _step(); // int_im2 T:15
        case 1671: _wait();_mread(cpu->wz);_step(); // int_im2 T:16
        case 1672: cpu->wzh=_gd();cpu->wzl=cpu->dlatch;cpu->pc=cpu->wz;_step(); // int_im2 T:17
        case 1673: _fetch(); // int_im2 T:18
        case 1674: _wait();cpu->iff1=false;_step(); // nmi T:0
        case 1675: pins=_z80_refresh(cpu,pins);_step(); // nmi T:1
        case 1676: _step(); // nmi T:2
        case 1677: _step(); // nmi T:3
        case 1678: _step(); // nmi T:4
        case 1679: _wait();_mwrite(--cpu->sp,cpu->pch);_step(); // nmi T:5
        case 1680: _step(); // nmi T:6
        case 1681: _step(); // nmi T:7
        case 1682: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=cpu->pc=0x0066;_step(); // nmi T:8
        case 1683: _step(); // nmi T:9
        case 1684: _fetch(); // nmi T:10
        // %>
        //=== shared fetch machine cycle for non-DD/FD-prefixed ops
        case Z80_M1_T2: _wait(); cpu->opcode = _gd(); _step();
        case Z80_M1_T3: pins = _z80_refresh(cpu, pins); _step();
        case Z80_M1_T4: cpu->addr = cpu->hl; _goto(cpu->opcode);
        //=== shared fetch machine cycle for DD/FD-prefixed ops
        case Z80_DDFD_M1_T2: _wait(); cpu->opcode = _gd(); _step();
        case Z80_DDFD_M1_T3: pins = _z80_refresh(cpu, pins); _step();
        case Z80_DDFD_M1_T4:
            cpu->addr = cpu->hlx[cpu->hlx_idx].hl;
            _goto(_z80_indirect_table[cpu->opcode] ? Z80_DDFD_D_T1 : cpu->opcode);
        //=== optional d-loading cycle for (IX+d), (IY+d)
        case Z80_DDFD_D_T1: _step();
        case Z80_DDFD_D_T2: _wait(); _mread(cpu->pc++); _step();
        case Z80_DDFD_D_T3: cpu->addr += (int8_t)_gd(); cpu->wz = cpu->addr; _step();
        //--- special case LD (IX/IY+d),n or filler ticks
        case Z80_DDFD_D_T4: _step();
        case Z80_DDFD_D_T5: if (cpu->opcode == 0x36) { _wait();_mread(cpu->pc++); }; _step();
        case Z80_DDFD_D_T6: if (cpu->opcode == 0x36) { cpu->dlatch = _gd(); }; _step();
        case Z80_DDFD_D_T7: _step();
        case Z80_DDFD_D_T8: _goto((cpu->opcode==0x36) ? Z80_DDFD_LDHLN_WR_T1 : cpu->opcode);
        //--- special case LD (IX/IY+d),n write mcycle
        case Z80_DDFD_LDHLN_WR_T1: _step();
        case Z80_DDFD_LDHLN_WR_T2: _wait(); _mwrite(cpu->addr,cpu->dlatch); _step();
        case Z80_DDFD_LDHLN_WR_T3: _step();
        case Z80_DDFD_LDHLN_OVERLAPPED: _fetch();
        //=== special opcode fetch machine cycle for ED-prefixed instructions
        case Z80_ED_M1_T2: _wait(); cpu->opcode = _gd(); _step();
        case Z80_ED_M1_T3: pins = _z80_refresh(cpu, pins); _step();
        case Z80_ED_M1_T4: _goto(cpu->opcode + 256);
        //=== special opcode fetch machine cycle for CB-prefixed instructions
        case Z80_CB_M1_T2: _wait(); cpu->opcode = _gd(); _step();
        case Z80_CB_M1_T3: pins = _z80_refresh(cpu, pins); _step();
        case Z80_CB_M1_T4:
            if ((cpu->opcode & 7) == 6) {
                // this is a (HL) instruction
                cpu->addr = cpu->hl;
                _goto(Z80_CBHL_STEP);
            }
            else {
                _goto(Z80_CB_STEP);
            }
        //=== from here on code-generated
        default: _Z80_UNREACHABLE;
    }
fetch_next:
    pins = _z80_fetch(cpu, pins);
    _z80_track_int_bits(cpu, pins);
    return pins;
step_next:
    cpu->step += 1;
step_to:
    _z80_track_int_bits(cpu, pins);
    return pins;
}

#undef _sa
#undef _sax
#undef _sad
#undef _sadx
#undef _gd
#undef _skip
#undef _fetch_dd
#undef _fetch_fd
#undef _fetch_ed
#undef _fetch_cb
#undef _mread
#undef _mwrite
#undef _ioread
#undef _iowrite
#undef _wait
#undef _cc_nz
#undef _cc_z
#undef _cc_nc
#undef _cc_c
#undef _cc_po
#undef _cc_pe
#undef _cc_p
#undef _cc_m

#endif // CHIPS_IMPL
