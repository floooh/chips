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
#define Z80_DDFD_M1_T2 1685
#define Z80_DDFD_M1_T3 1686
#define Z80_DDFD_M1_T4 1687
#define Z80_DDFD_D_T1 1688
#define Z80_DDFD_D_T2 1689
#define Z80_DDFD_D_T3 1690
#define Z80_DDFD_D_T4 1691
#define Z80_DDFD_D_T5 1692
#define Z80_DDFD_D_T6 1693
#define Z80_DDFD_D_T7 1694
#define Z80_DDFD_D_T8 1695
#define Z80_DDFD_LDHLN_WR_T1 1696
#define Z80_DDFD_LDHLN_WR_T2 1697
#define Z80_DDFD_LDHLN_WR_T3 1698
#define Z80_DDFD_LDHLN_OVERLAPPED 1699
#define Z80_CB_M1_T2 1700
#define Z80_CB_M1_T3 1701
#define Z80_CB_M1_T4 1702
#define Z80_ED_M1_T2 1703
#define Z80_ED_M1_T3 1704
#define Z80_ED_M1_T4 1705
#define Z80_M1_T2 1706
#define Z80_M1_T3 1707
#define Z80_M1_T4 1708
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
                default: _Z80_UNREACHABLE;
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
        return pins;
    } else {
        // this is a regular CB-prefixed instruction, continue
        // execution on a special fetch machine cycle which doesn't
        // handle DD/FD prefix and then branches either to the
        // special CB or CBHL decoder block
        cpu->step = Z80_CB_M1_T2; // => opcode fetch for CB prefixed instructions
        return _z80_set_ab_x(pins, cpu->pc++, Z80_M1|Z80_MREQ|Z80_RD);
    }
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
        case  512: _wait();_mread(cpu->pc++);_goto(513); // LD BC,nn T:1
        case  513: cpu->c=_gd();_goto(514); // LD BC,nn T:2
        case  514: _goto(515); // LD BC,nn T:3
        case  515: _wait();_mread(cpu->pc++);_goto(516); // LD BC,nn T:4
        case  516: cpu->b=_gd();_goto(517); // LD BC,nn T:5
        case  517: _fetch(); // LD BC,nn T:6
        case  518: _wait();_mwrite(cpu->bc,cpu->a);cpu->wzl=cpu->c+1;cpu->wzh=cpu->a;_goto(519); // LD (BC),A T:1
        case  519: _goto(520); // LD (BC),A T:2
        case  520: _fetch(); // LD (BC),A T:3
        case  521: _goto(522); // INC BC T:1
        case  522: _fetch(); // INC BC T:2
        case  523: _wait();_mread(cpu->pc++);_goto(524); // LD B,n T:1
        case  524: cpu->b=_gd();_goto(525); // LD B,n T:2
        case  525: _fetch(); // LD B,n T:3
        case  526: _goto(527); // ADD HL,BC T:1
        case  527: _goto(528); // ADD HL,BC T:2
        case  528: _goto(529); // ADD HL,BC T:3
        case  529: _goto(530); // ADD HL,BC T:4
        case  530: _goto(531); // ADD HL,BC T:5
        case  531: _goto(532); // ADD HL,BC T:6
        case  532: _fetch(); // ADD HL,BC T:7
        case  533: _wait();_mread(cpu->bc);_goto(534); // LD A,(BC) T:1
        case  534: cpu->a=_gd();cpu->wz=cpu->bc+1;_goto(535); // LD A,(BC) T:2
        case  535: _fetch(); // LD A,(BC) T:3
        case  536: _goto(537); // DEC BC T:1
        case  537: _fetch(); // DEC BC T:2
        case  538: _wait();_mread(cpu->pc++);_goto(539); // LD C,n T:1
        case  539: cpu->c=_gd();_goto(540); // LD C,n T:2
        case  540: _fetch(); // LD C,n T:3
        case  541: _goto(542); // DJNZ d T:1
        case  542: _wait();_mread(cpu->pc++);_goto(543); // DJNZ d T:2
        case  543: cpu->dlatch=_gd();if(--cpu->b==0){_goto(544+5);};_goto(544); // DJNZ d T:3
        case  544: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;_goto(545); // DJNZ d T:4
        case  545: _goto(546); // DJNZ d T:5
        case  546: _goto(547); // DJNZ d T:6
        case  547: _goto(548); // DJNZ d T:7
        case  548: _goto(549); // DJNZ d T:8
        case  549: _fetch(); // DJNZ d T:9
        case  550: _wait();_mread(cpu->pc++);_goto(551); // LD DE,nn T:1
        case  551: cpu->e=_gd();_goto(552); // LD DE,nn T:2
        case  552: _goto(553); // LD DE,nn T:3
        case  553: _wait();_mread(cpu->pc++);_goto(554); // LD DE,nn T:4
        case  554: cpu->d=_gd();_goto(555); // LD DE,nn T:5
        case  555: _fetch(); // LD DE,nn T:6
        case  556: _wait();_mwrite(cpu->de,cpu->a);cpu->wzl=cpu->e+1;cpu->wzh=cpu->a;_goto(557); // LD (DE),A T:1
        case  557: _goto(558); // LD (DE),A T:2
        case  558: _fetch(); // LD (DE),A T:3
        case  559: _goto(560); // INC DE T:1
        case  560: _fetch(); // INC DE T:2
        case  561: _wait();_mread(cpu->pc++);_goto(562); // LD D,n T:1
        case  562: cpu->d=_gd();_goto(563); // LD D,n T:2
        case  563: _fetch(); // LD D,n T:3
        case  564: _wait();_mread(cpu->pc++);_goto(565); // JR d T:1
        case  565: cpu->dlatch=_gd();_goto(566); // JR d T:2
        case  566: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;_goto(567); // JR d T:3
        case  567: _goto(568); // JR d T:4
        case  568: _goto(569); // JR d T:5
        case  569: _goto(570); // JR d T:6
        case  570: _goto(571); // JR d T:7
        case  571: _fetch(); // JR d T:8
        case  572: _goto(573); // ADD HL,DE T:1
        case  573: _goto(574); // ADD HL,DE T:2
        case  574: _goto(575); // ADD HL,DE T:3
        case  575: _goto(576); // ADD HL,DE T:4
        case  576: _goto(577); // ADD HL,DE T:5
        case  577: _goto(578); // ADD HL,DE T:6
        case  578: _fetch(); // ADD HL,DE T:7
        case  579: _wait();_mread(cpu->de);_goto(580); // LD A,(DE) T:1
        case  580: cpu->a=_gd();cpu->wz=cpu->de+1;_goto(581); // LD A,(DE) T:2
        case  581: _fetch(); // LD A,(DE) T:3
        case  582: _goto(583); // DEC DE T:1
        case  583: _fetch(); // DEC DE T:2
        case  584: _wait();_mread(cpu->pc++);_goto(585); // LD E,n T:1
        case  585: cpu->e=_gd();_goto(586); // LD E,n T:2
        case  586: _fetch(); // LD E,n T:3
        case  587: _wait();_mread(cpu->pc++);_goto(588); // JR NZ,d T:1
        case  588: cpu->dlatch=_gd();if(!(_cc_nz)){_goto(589+5);};_goto(589); // JR NZ,d T:2
        case  589: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;_goto(590); // JR NZ,d T:3
        case  590: _goto(591); // JR NZ,d T:4
        case  591: _goto(592); // JR NZ,d T:5
        case  592: _goto(593); // JR NZ,d T:6
        case  593: _goto(594); // JR NZ,d T:7
        case  594: _fetch(); // JR NZ,d T:8
        case  595: _wait();_mread(cpu->pc++);_goto(596); // LD HL,nn T:1
        case  596: cpu->hlx[cpu->hlx_idx].l=_gd();_goto(597); // LD HL,nn T:2
        case  597: _goto(598); // LD HL,nn T:3
        case  598: _wait();_mread(cpu->pc++);_goto(599); // LD HL,nn T:4
        case  599: cpu->hlx[cpu->hlx_idx].h=_gd();_goto(600); // LD HL,nn T:5
        case  600: _fetch(); // LD HL,nn T:6
        case  601: _wait();_mread(cpu->pc++);_goto(602); // LD (nn),HL T:1
        case  602: cpu->wzl=_gd();_goto(603); // LD (nn),HL T:2
        case  603: _goto(604); // LD (nn),HL T:3
        case  604: _wait();_mread(cpu->pc++);_goto(605); // LD (nn),HL T:4
        case  605: cpu->wzh=_gd();_goto(606); // LD (nn),HL T:5
        case  606: _goto(607); // LD (nn),HL T:6
        case  607: _wait();_mwrite(cpu->wz++,cpu->hlx[cpu->hlx_idx].l);_goto(608); // LD (nn),HL T:7
        case  608: _goto(609); // LD (nn),HL T:8
        case  609: _goto(610); // LD (nn),HL T:9
        case  610: _wait();_mwrite(cpu->wz,cpu->hlx[cpu->hlx_idx].h);_goto(611); // LD (nn),HL T:10
        case  611: _goto(612); // LD (nn),HL T:11
        case  612: _fetch(); // LD (nn),HL T:12
        case  613: _goto(614); // INC HL T:1
        case  614: _fetch(); // INC HL T:2
        case  615: _wait();_mread(cpu->pc++);_goto(616); // LD H,n T:1
        case  616: cpu->hlx[cpu->hlx_idx].h=_gd();_goto(617); // LD H,n T:2
        case  617: _fetch(); // LD H,n T:3
        case  618: _wait();_mread(cpu->pc++);_goto(619); // JR Z,d T:1
        case  619: cpu->dlatch=_gd();if(!(_cc_z)){_goto(620+5);};_goto(620); // JR Z,d T:2
        case  620: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;_goto(621); // JR Z,d T:3
        case  621: _goto(622); // JR Z,d T:4
        case  622: _goto(623); // JR Z,d T:5
        case  623: _goto(624); // JR Z,d T:6
        case  624: _goto(625); // JR Z,d T:7
        case  625: _fetch(); // JR Z,d T:8
        case  626: _goto(627); // ADD HL,HL T:1
        case  627: _goto(628); // ADD HL,HL T:2
        case  628: _goto(629); // ADD HL,HL T:3
        case  629: _goto(630); // ADD HL,HL T:4
        case  630: _goto(631); // ADD HL,HL T:5
        case  631: _goto(632); // ADD HL,HL T:6
        case  632: _fetch(); // ADD HL,HL T:7
        case  633: _wait();_mread(cpu->pc++);_goto(634); // LD HL,(nn) T:1
        case  634: cpu->wzl=_gd();_goto(635); // LD HL,(nn) T:2
        case  635: _goto(636); // LD HL,(nn) T:3
        case  636: _wait();_mread(cpu->pc++);_goto(637); // LD HL,(nn) T:4
        case  637: cpu->wzh=_gd();_goto(638); // LD HL,(nn) T:5
        case  638: _goto(639); // LD HL,(nn) T:6
        case  639: _wait();_mread(cpu->wz++);_goto(640); // LD HL,(nn) T:7
        case  640: cpu->hlx[cpu->hlx_idx].l=_gd();_goto(641); // LD HL,(nn) T:8
        case  641: _goto(642); // LD HL,(nn) T:9
        case  642: _wait();_mread(cpu->wz);_goto(643); // LD HL,(nn) T:10
        case  643: cpu->hlx[cpu->hlx_idx].h=_gd();_goto(644); // LD HL,(nn) T:11
        case  644: _fetch(); // LD HL,(nn) T:12
        case  645: _goto(646); // DEC HL T:1
        case  646: _fetch(); // DEC HL T:2
        case  647: _wait();_mread(cpu->pc++);_goto(648); // LD L,n T:1
        case  648: cpu->hlx[cpu->hlx_idx].l=_gd();_goto(649); // LD L,n T:2
        case  649: _fetch(); // LD L,n T:3
        case  650: _wait();_mread(cpu->pc++);_goto(651); // JR NC,d T:1
        case  651: cpu->dlatch=_gd();if(!(_cc_nc)){_goto(652+5);};_goto(652); // JR NC,d T:2
        case  652: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;_goto(653); // JR NC,d T:3
        case  653: _goto(654); // JR NC,d T:4
        case  654: _goto(655); // JR NC,d T:5
        case  655: _goto(656); // JR NC,d T:6
        case  656: _goto(657); // JR NC,d T:7
        case  657: _fetch(); // JR NC,d T:8
        case  658: _wait();_mread(cpu->pc++);_goto(659); // LD SP,nn T:1
        case  659: cpu->spl=_gd();_goto(660); // LD SP,nn T:2
        case  660: _goto(661); // LD SP,nn T:3
        case  661: _wait();_mread(cpu->pc++);_goto(662); // LD SP,nn T:4
        case  662: cpu->sph=_gd();_goto(663); // LD SP,nn T:5
        case  663: _fetch(); // LD SP,nn T:6
        case  664: _wait();_mread(cpu->pc++);_goto(665); // LD (nn),A T:1
        case  665: cpu->wzl=_gd();_goto(666); // LD (nn),A T:2
        case  666: _goto(667); // LD (nn),A T:3
        case  667: _wait();_mread(cpu->pc++);_goto(668); // LD (nn),A T:4
        case  668: cpu->wzh=_gd();_goto(669); // LD (nn),A T:5
        case  669: _goto(670); // LD (nn),A T:6
        case  670: _wait();_mwrite(cpu->wz++,cpu->a);cpu->wzh=cpu->a;_goto(671); // LD (nn),A T:7
        case  671: _goto(672); // LD (nn),A T:8
        case  672: _fetch(); // LD (nn),A T:9
        case  673: _goto(674); // INC SP T:1
        case  674: _fetch(); // INC SP T:2
        case  675: _wait();_mread(cpu->addr);_goto(676); // INC (HL) T:1
        case  676: cpu->dlatch=_gd();cpu->dlatch=_z80_inc8(cpu,cpu->dlatch);_goto(677); // INC (HL) T:2
        case  677: _goto(678); // INC (HL) T:3
        case  678: _goto(679); // INC (HL) T:4
        case  679: _wait();_mwrite(cpu->addr,cpu->dlatch);_goto(680); // INC (HL) T:5
        case  680: _goto(681); // INC (HL) T:6
        case  681: _fetch(); // INC (HL) T:7
        case  682: _wait();_mread(cpu->addr);_goto(683); // DEC (HL) T:1
        case  683: cpu->dlatch=_gd();cpu->dlatch=_z80_dec8(cpu,cpu->dlatch);_goto(684); // DEC (HL) T:2
        case  684: _goto(685); // DEC (HL) T:3
        case  685: _goto(686); // DEC (HL) T:4
        case  686: _wait();_mwrite(cpu->addr,cpu->dlatch);_goto(687); // DEC (HL) T:5
        case  687: _goto(688); // DEC (HL) T:6
        case  688: _fetch(); // DEC (HL) T:7
        case  689: _wait();_mread(cpu->pc++);_goto(690); // LD (HL),n T:1
        case  690: cpu->dlatch=_gd();_goto(691); // LD (HL),n T:2
        case  691: _goto(692); // LD (HL),n T:3
        case  692: _wait();_mwrite(cpu->addr,cpu->dlatch);_goto(693); // LD (HL),n T:4
        case  693: _goto(694); // LD (HL),n T:5
        case  694: _fetch(); // LD (HL),n T:6
        case  695: _wait();_mread(cpu->pc++);_goto(696); // JR C,d T:1
        case  696: cpu->dlatch=_gd();if(!(_cc_c)){_goto(697+5);};_goto(697); // JR C,d T:2
        case  697: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;_goto(698); // JR C,d T:3
        case  698: _goto(699); // JR C,d T:4
        case  699: _goto(700); // JR C,d T:5
        case  700: _goto(701); // JR C,d T:6
        case  701: _goto(702); // JR C,d T:7
        case  702: _fetch(); // JR C,d T:8
        case  703: _goto(704); // ADD HL,SP T:1
        case  704: _goto(705); // ADD HL,SP T:2
        case  705: _goto(706); // ADD HL,SP T:3
        case  706: _goto(707); // ADD HL,SP T:4
        case  707: _goto(708); // ADD HL,SP T:5
        case  708: _goto(709); // ADD HL,SP T:6
        case  709: _fetch(); // ADD HL,SP T:7
        case  710: _wait();_mread(cpu->pc++);_goto(711); // LD A,(nn) T:1
        case  711: cpu->wzl=_gd();_goto(712); // LD A,(nn) T:2
        case  712: _goto(713); // LD A,(nn) T:3
        case  713: _wait();_mread(cpu->pc++);_goto(714); // LD A,(nn) T:4
        case  714: cpu->wzh=_gd();_goto(715); // LD A,(nn) T:5
        case  715: _goto(716); // LD A,(nn) T:6
        case  716: _wait();_mread(cpu->wz++);_goto(717); // LD A,(nn) T:7
        case  717: cpu->a=_gd();_goto(718); // LD A,(nn) T:8
        case  718: _fetch(); // LD A,(nn) T:9
        case  719: _goto(720); // DEC SP T:1
        case  720: _fetch(); // DEC SP T:2
        case  721: _wait();_mread(cpu->pc++);_goto(722); // LD A,n T:1
        case  722: cpu->a=_gd();_goto(723); // LD A,n T:2
        case  723: _fetch(); // LD A,n T:3
        case  724: _wait();_mread(cpu->addr);_goto(725); // LD B,(HL) T:1
        case  725: cpu->b=_gd();_goto(726); // LD B,(HL) T:2
        case  726: _fetch(); // LD B,(HL) T:3
        case  727: _wait();_mread(cpu->addr);_goto(728); // LD C,(HL) T:1
        case  728: cpu->c=_gd();_goto(729); // LD C,(HL) T:2
        case  729: _fetch(); // LD C,(HL) T:3
        case  730: _wait();_mread(cpu->addr);_goto(731); // LD D,(HL) T:1
        case  731: cpu->d=_gd();_goto(732); // LD D,(HL) T:2
        case  732: _fetch(); // LD D,(HL) T:3
        case  733: _wait();_mread(cpu->addr);_goto(734); // LD E,(HL) T:1
        case  734: cpu->e=_gd();_goto(735); // LD E,(HL) T:2
        case  735: _fetch(); // LD E,(HL) T:3
        case  736: _wait();_mread(cpu->addr);_goto(737); // LD H,(HL) T:1
        case  737: cpu->h=_gd();_goto(738); // LD H,(HL) T:2
        case  738: _fetch(); // LD H,(HL) T:3
        case  739: _wait();_mread(cpu->addr);_goto(740); // LD L,(HL) T:1
        case  740: cpu->l=_gd();_goto(741); // LD L,(HL) T:2
        case  741: _fetch(); // LD L,(HL) T:3
        case  742: _wait();_mwrite(cpu->addr,cpu->b);_goto(743); // LD (HL),B T:1
        case  743: _goto(744); // LD (HL),B T:2
        case  744: _fetch(); // LD (HL),B T:3
        case  745: _wait();_mwrite(cpu->addr,cpu->c);_goto(746); // LD (HL),C T:1
        case  746: _goto(747); // LD (HL),C T:2
        case  747: _fetch(); // LD (HL),C T:3
        case  748: _wait();_mwrite(cpu->addr,cpu->d);_goto(749); // LD (HL),D T:1
        case  749: _goto(750); // LD (HL),D T:2
        case  750: _fetch(); // LD (HL),D T:3
        case  751: _wait();_mwrite(cpu->addr,cpu->e);_goto(752); // LD (HL),E T:1
        case  752: _goto(753); // LD (HL),E T:2
        case  753: _fetch(); // LD (HL),E T:3
        case  754: _wait();_mwrite(cpu->addr,cpu->h);_goto(755); // LD (HL),H T:1
        case  755: _goto(756); // LD (HL),H T:2
        case  756: _fetch(); // LD (HL),H T:3
        case  757: _wait();_mwrite(cpu->addr,cpu->l);_goto(758); // LD (HL),L T:1
        case  758: _goto(759); // LD (HL),L T:2
        case  759: _fetch(); // LD (HL),L T:3
        case  760: _wait();_mwrite(cpu->addr,cpu->a);_goto(761); // LD (HL),A T:1
        case  761: _goto(762); // LD (HL),A T:2
        case  762: _fetch(); // LD (HL),A T:3
        case  763: _wait();_mread(cpu->addr);_goto(764); // LD A,(HL) T:1
        case  764: cpu->a=_gd();_goto(765); // LD A,(HL) T:2
        case  765: _fetch(); // LD A,(HL) T:3
        case  766: _wait();_mread(cpu->addr);_goto(767); // ADD (HL) T:1
        case  767: cpu->dlatch=_gd();_goto(768); // ADD (HL) T:2
        case  768: _z80_add8(cpu,cpu->dlatch);_fetch(); // ADD (HL) T:3
        case  769: _wait();_mread(cpu->addr);_goto(770); // ADC (HL) T:1
        case  770: cpu->dlatch=_gd();_goto(771); // ADC (HL) T:2
        case  771: _z80_adc8(cpu,cpu->dlatch);_fetch(); // ADC (HL) T:3
        case  772: _wait();_mread(cpu->addr);_goto(773); // SUB (HL) T:1
        case  773: cpu->dlatch=_gd();_goto(774); // SUB (HL) T:2
        case  774: _z80_sub8(cpu,cpu->dlatch);_fetch(); // SUB (HL) T:3
        case  775: _wait();_mread(cpu->addr);_goto(776); // SBC (HL) T:1
        case  776: cpu->dlatch=_gd();_goto(777); // SBC (HL) T:2
        case  777: _z80_sbc8(cpu,cpu->dlatch);_fetch(); // SBC (HL) T:3
        case  778: _wait();_mread(cpu->addr);_goto(779); // AND (HL) T:1
        case  779: cpu->dlatch=_gd();_goto(780); // AND (HL) T:2
        case  780: _z80_and8(cpu,cpu->dlatch);_fetch(); // AND (HL) T:3
        case  781: _wait();_mread(cpu->addr);_goto(782); // XOR (HL) T:1
        case  782: cpu->dlatch=_gd();_goto(783); // XOR (HL) T:2
        case  783: _z80_xor8(cpu,cpu->dlatch);_fetch(); // XOR (HL) T:3
        case  784: _wait();_mread(cpu->addr);_goto(785); // OR (HL) T:1
        case  785: cpu->dlatch=_gd();_goto(786); // OR (HL) T:2
        case  786: _z80_or8(cpu,cpu->dlatch);_fetch(); // OR (HL) T:3
        case  787: _wait();_mread(cpu->addr);_goto(788); // CP (HL) T:1
        case  788: cpu->dlatch=_gd();_goto(789); // CP (HL) T:2
        case  789: _z80_cp8(cpu,cpu->dlatch);_fetch(); // CP (HL) T:3
        case  790: _goto(791); // RET NZ T:1
        case  791: _wait();_mread(cpu->sp++);_goto(792); // RET NZ T:2
        case  792: cpu->wzl=_gd();_goto(793); // RET NZ T:3
        case  793: _goto(794); // RET NZ T:4
        case  794: _wait();_mread(cpu->sp++);_goto(795); // RET NZ T:5
        case  795: cpu->wzh=_gd();cpu->pc=cpu->wz;_goto(796); // RET NZ T:6
        case  796: _fetch(); // RET NZ T:7
        case  797: _wait();_mread(cpu->sp++);_goto(798); // POP BC T:1
        case  798: cpu->c=_gd();_goto(799); // POP BC T:2
        case  799: _goto(800); // POP BC T:3
        case  800: _wait();_mread(cpu->sp++);_goto(801); // POP BC T:4
        case  801: cpu->b=_gd();_goto(802); // POP BC T:5
        case  802: _fetch(); // POP BC T:6
        case  803: _wait();_mread(cpu->pc++);_goto(804); // JP NZ,nn T:1
        case  804: cpu->wzl=_gd();_goto(805); // JP NZ,nn T:2
        case  805: _goto(806); // JP NZ,nn T:3
        case  806: _wait();_mread(cpu->pc++);_goto(807); // JP NZ,nn T:4
        case  807: cpu->wzh=_gd();if(_cc_nz){cpu->pc=cpu->wz;};_goto(808); // JP NZ,nn T:5
        case  808: _fetch(); // JP NZ,nn T:6
        case  809: _wait();_mread(cpu->pc++);_goto(810); // JP nn T:1
        case  810: cpu->wzl=_gd();_goto(811); // JP nn T:2
        case  811: _goto(812); // JP nn T:3
        case  812: _wait();_mread(cpu->pc++);_goto(813); // JP nn T:4
        case  813: cpu->wzh=_gd();cpu->pc=cpu->wz;_goto(814); // JP nn T:5
        case  814: _fetch(); // JP nn T:6
        case  815: _wait();_mread(cpu->pc++);_goto(816); // CALL NZ,nn T:1
        case  816: cpu->wzl=_gd();_goto(817); // CALL NZ,nn T:2
        case  817: _goto(818); // CALL NZ,nn T:3
        case  818: _wait();_mread(cpu->pc++);_goto(819); // CALL NZ,nn T:4
        case  819: cpu->wzh=_gd();if (!_cc_nz){_goto(820+7);};_goto(820); // CALL NZ,nn T:5
        case  820: _goto(821); // CALL NZ,nn T:6
        case  821: _goto(822); // CALL NZ,nn T:7
        case  822: _wait();_mwrite(--cpu->sp,cpu->pch);_goto(823); // CALL NZ,nn T:8
        case  823: _goto(824); // CALL NZ,nn T:9
        case  824: _goto(825); // CALL NZ,nn T:10
        case  825: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz;_goto(826); // CALL NZ,nn T:11
        case  826: _goto(827); // CALL NZ,nn T:12
        case  827: _fetch(); // CALL NZ,nn T:13
        case  828: _goto(829); // PUSH BC T:1
        case  829: _wait();_mwrite(--cpu->sp,cpu->b);_goto(830); // PUSH BC T:2
        case  830: _goto(831); // PUSH BC T:3
        case  831: _goto(832); // PUSH BC T:4
        case  832: _wait();_mwrite(--cpu->sp,cpu->c);_goto(833); // PUSH BC T:5
        case  833: _goto(834); // PUSH BC T:6
        case  834: _fetch(); // PUSH BC T:7
        case  835: _wait();_mread(cpu->pc++);_goto(836); // ADD n T:1
        case  836: cpu->dlatch=_gd();_goto(837); // ADD n T:2
        case  837: _z80_add8(cpu,cpu->dlatch);_fetch(); // ADD n T:3
        case  838: _goto(839); // RST 0h T:1
        case  839: _wait();_mwrite(--cpu->sp,cpu->pch);_goto(840); // RST 0h T:2
        case  840: _goto(841); // RST 0h T:3
        case  841: _goto(842); // RST 0h T:4
        case  842: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x00;cpu->pc=cpu->wz;_goto(843); // RST 0h T:5
        case  843: _goto(844); // RST 0h T:6
        case  844: _fetch(); // RST 0h T:7
        case  845: _goto(846); // RET Z T:1
        case  846: _wait();_mread(cpu->sp++);_goto(847); // RET Z T:2
        case  847: cpu->wzl=_gd();_goto(848); // RET Z T:3
        case  848: _goto(849); // RET Z T:4
        case  849: _wait();_mread(cpu->sp++);_goto(850); // RET Z T:5
        case  850: cpu->wzh=_gd();cpu->pc=cpu->wz;_goto(851); // RET Z T:6
        case  851: _fetch(); // RET Z T:7
        case  852: _wait();_mread(cpu->sp++);_goto(853); // RET T:1
        case  853: cpu->wzl=_gd();_goto(854); // RET T:2
        case  854: _goto(855); // RET T:3
        case  855: _wait();_mread(cpu->sp++);_goto(856); // RET T:4
        case  856: cpu->wzh=_gd();cpu->pc=cpu->wz;_goto(857); // RET T:5
        case  857: _fetch(); // RET T:6
        case  858: _wait();_mread(cpu->pc++);_goto(859); // JP Z,nn T:1
        case  859: cpu->wzl=_gd();_goto(860); // JP Z,nn T:2
        case  860: _goto(861); // JP Z,nn T:3
        case  861: _wait();_mread(cpu->pc++);_goto(862); // JP Z,nn T:4
        case  862: cpu->wzh=_gd();if(_cc_z){cpu->pc=cpu->wz;};_goto(863); // JP Z,nn T:5
        case  863: _fetch(); // JP Z,nn T:6
        case  864: _wait();_mread(cpu->pc++);_goto(865); // CALL Z,nn T:1
        case  865: cpu->wzl=_gd();_goto(866); // CALL Z,nn T:2
        case  866: _goto(867); // CALL Z,nn T:3
        case  867: _wait();_mread(cpu->pc++);_goto(868); // CALL Z,nn T:4
        case  868: cpu->wzh=_gd();if (!_cc_z){_goto(869+7);};_goto(869); // CALL Z,nn T:5
        case  869: _goto(870); // CALL Z,nn T:6
        case  870: _goto(871); // CALL Z,nn T:7
        case  871: _wait();_mwrite(--cpu->sp,cpu->pch);_goto(872); // CALL Z,nn T:8
        case  872: _goto(873); // CALL Z,nn T:9
        case  873: _goto(874); // CALL Z,nn T:10
        case  874: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz;_goto(875); // CALL Z,nn T:11
        case  875: _goto(876); // CALL Z,nn T:12
        case  876: _fetch(); // CALL Z,nn T:13
        case  877: _wait();_mread(cpu->pc++);_goto(878); // CALL nn T:1
        case  878: cpu->wzl=_gd();_goto(879); // CALL nn T:2
        case  879: _goto(880); // CALL nn T:3
        case  880: _wait();_mread(cpu->pc++);_goto(881); // CALL nn T:4
        case  881: cpu->wzh=_gd();_goto(882); // CALL nn T:5
        case  882: _goto(883); // CALL nn T:6
        case  883: _goto(884); // CALL nn T:7
        case  884: _wait();_mwrite(--cpu->sp,cpu->pch);_goto(885); // CALL nn T:8
        case  885: _goto(886); // CALL nn T:9
        case  886: _goto(887); // CALL nn T:10
        case  887: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz;_goto(888); // CALL nn T:11
        case  888: _goto(889); // CALL nn T:12
        case  889: _fetch(); // CALL nn T:13
        case  890: _wait();_mread(cpu->pc++);_goto(891); // ADC n T:1
        case  891: cpu->dlatch=_gd();_goto(892); // ADC n T:2
        case  892: _z80_adc8(cpu,cpu->dlatch);_fetch(); // ADC n T:3
        case  893: _goto(894); // RST 8h T:1
        case  894: _wait();_mwrite(--cpu->sp,cpu->pch);_goto(895); // RST 8h T:2
        case  895: _goto(896); // RST 8h T:3
        case  896: _goto(897); // RST 8h T:4
        case  897: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x08;cpu->pc=cpu->wz;_goto(898); // RST 8h T:5
        case  898: _goto(899); // RST 8h T:6
        case  899: _fetch(); // RST 8h T:7
        case  900: _goto(901); // RET NC T:1
        case  901: _wait();_mread(cpu->sp++);_goto(902); // RET NC T:2
        case  902: cpu->wzl=_gd();_goto(903); // RET NC T:3
        case  903: _goto(904); // RET NC T:4
        case  904: _wait();_mread(cpu->sp++);_goto(905); // RET NC T:5
        case  905: cpu->wzh=_gd();cpu->pc=cpu->wz;_goto(906); // RET NC T:6
        case  906: _fetch(); // RET NC T:7
        case  907: _wait();_mread(cpu->sp++);_goto(908); // POP DE T:1
        case  908: cpu->e=_gd();_goto(909); // POP DE T:2
        case  909: _goto(910); // POP DE T:3
        case  910: _wait();_mread(cpu->sp++);_goto(911); // POP DE T:4
        case  911: cpu->d=_gd();_goto(912); // POP DE T:5
        case  912: _fetch(); // POP DE T:6
        case  913: _wait();_mread(cpu->pc++);_goto(914); // JP NC,nn T:1
        case  914: cpu->wzl=_gd();_goto(915); // JP NC,nn T:2
        case  915: _goto(916); // JP NC,nn T:3
        case  916: _wait();_mread(cpu->pc++);_goto(917); // JP NC,nn T:4
        case  917: cpu->wzh=_gd();if(_cc_nc){cpu->pc=cpu->wz;};_goto(918); // JP NC,nn T:5
        case  918: _fetch(); // JP NC,nn T:6
        case  919: _wait();_mread(cpu->pc++);_goto(920); // OUT (n),A T:1
        case  920: cpu->wzl=_gd();cpu->wzh=cpu->a;_goto(921); // OUT (n),A T:2
        case  921: _goto(922); // OUT (n),A T:3
        case  922: _iowrite(cpu->wz,cpu->a);_goto(923); // OUT (n),A T:4
        case  923: _wait();cpu->wzl++;_goto(924); // OUT (n),A T:5
        case  924: _goto(925); // OUT (n),A T:6
        case  925: _fetch(); // OUT (n),A T:7
        case  926: _wait();_mread(cpu->pc++);_goto(927); // CALL NC,nn T:1
        case  927: cpu->wzl=_gd();_goto(928); // CALL NC,nn T:2
        case  928: _goto(929); // CALL NC,nn T:3
        case  929: _wait();_mread(cpu->pc++);_goto(930); // CALL NC,nn T:4
        case  930: cpu->wzh=_gd();if (!_cc_nc){_goto(931+7);};_goto(931); // CALL NC,nn T:5
        case  931: _goto(932); // CALL NC,nn T:6
        case  932: _goto(933); // CALL NC,nn T:7
        case  933: _wait();_mwrite(--cpu->sp,cpu->pch);_goto(934); // CALL NC,nn T:8
        case  934: _goto(935); // CALL NC,nn T:9
        case  935: _goto(936); // CALL NC,nn T:10
        case  936: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz;_goto(937); // CALL NC,nn T:11
        case  937: _goto(938); // CALL NC,nn T:12
        case  938: _fetch(); // CALL NC,nn T:13
        case  939: _goto(940); // PUSH DE T:1
        case  940: _wait();_mwrite(--cpu->sp,cpu->d);_goto(941); // PUSH DE T:2
        case  941: _goto(942); // PUSH DE T:3
        case  942: _goto(943); // PUSH DE T:4
        case  943: _wait();_mwrite(--cpu->sp,cpu->e);_goto(944); // PUSH DE T:5
        case  944: _goto(945); // PUSH DE T:6
        case  945: _fetch(); // PUSH DE T:7
        case  946: _wait();_mread(cpu->pc++);_goto(947); // SUB n T:1
        case  947: cpu->dlatch=_gd();_goto(948); // SUB n T:2
        case  948: _z80_sub8(cpu,cpu->dlatch);_fetch(); // SUB n T:3
        case  949: _goto(950); // RST 10h T:1
        case  950: _wait();_mwrite(--cpu->sp,cpu->pch);_goto(951); // RST 10h T:2
        case  951: _goto(952); // RST 10h T:3
        case  952: _goto(953); // RST 10h T:4
        case  953: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x10;cpu->pc=cpu->wz;_goto(954); // RST 10h T:5
        case  954: _goto(955); // RST 10h T:6
        case  955: _fetch(); // RST 10h T:7
        case  956: _goto(957); // RET C T:1
        case  957: _wait();_mread(cpu->sp++);_goto(958); // RET C T:2
        case  958: cpu->wzl=_gd();_goto(959); // RET C T:3
        case  959: _goto(960); // RET C T:4
        case  960: _wait();_mread(cpu->sp++);_goto(961); // RET C T:5
        case  961: cpu->wzh=_gd();cpu->pc=cpu->wz;_goto(962); // RET C T:6
        case  962: _fetch(); // RET C T:7
        case  963: _wait();_mread(cpu->pc++);_goto(964); // JP C,nn T:1
        case  964: cpu->wzl=_gd();_goto(965); // JP C,nn T:2
        case  965: _goto(966); // JP C,nn T:3
        case  966: _wait();_mread(cpu->pc++);_goto(967); // JP C,nn T:4
        case  967: cpu->wzh=_gd();if(_cc_c){cpu->pc=cpu->wz;};_goto(968); // JP C,nn T:5
        case  968: _fetch(); // JP C,nn T:6
        case  969: _wait();_mread(cpu->pc++);_goto(970); // IN A,(n) T:1
        case  970: cpu->wzl=_gd();cpu->wzh=cpu->a;_goto(971); // IN A,(n) T:2
        case  971: _goto(972); // IN A,(n) T:3
        case  972: _goto(973); // IN A,(n) T:4
        case  973: _wait();_ioread(cpu->wz++);_goto(974); // IN A,(n) T:5
        case  974: cpu->a=_gd();_goto(975); // IN A,(n) T:6
        case  975: _fetch(); // IN A,(n) T:7
        case  976: _wait();_mread(cpu->pc++);_goto(977); // CALL C,nn T:1
        case  977: cpu->wzl=_gd();_goto(978); // CALL C,nn T:2
        case  978: _goto(979); // CALL C,nn T:3
        case  979: _wait();_mread(cpu->pc++);_goto(980); // CALL C,nn T:4
        case  980: cpu->wzh=_gd();if (!_cc_c){_goto(981+7);};_goto(981); // CALL C,nn T:5
        case  981: _goto(982); // CALL C,nn T:6
        case  982: _goto(983); // CALL C,nn T:7
        case  983: _wait();_mwrite(--cpu->sp,cpu->pch);_goto(984); // CALL C,nn T:8
        case  984: _goto(985); // CALL C,nn T:9
        case  985: _goto(986); // CALL C,nn T:10
        case  986: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz;_goto(987); // CALL C,nn T:11
        case  987: _goto(988); // CALL C,nn T:12
        case  988: _fetch(); // CALL C,nn T:13
        case  989: _wait();_mread(cpu->pc++);_goto(990); // SBC n T:1
        case  990: cpu->dlatch=_gd();_goto(991); // SBC n T:2
        case  991: _z80_sbc8(cpu,cpu->dlatch);_fetch(); // SBC n T:3
        case  992: _goto(993); // RST 18h T:1
        case  993: _wait();_mwrite(--cpu->sp,cpu->pch);_goto(994); // RST 18h T:2
        case  994: _goto(995); // RST 18h T:3
        case  995: _goto(996); // RST 18h T:4
        case  996: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x18;cpu->pc=cpu->wz;_goto(997); // RST 18h T:5
        case  997: _goto(998); // RST 18h T:6
        case  998: _fetch(); // RST 18h T:7
        case  999: _goto(1000); // RET PO T:1
        case 1000: _wait();_mread(cpu->sp++);_goto(1001); // RET PO T:2
        case 1001: cpu->wzl=_gd();_goto(1002); // RET PO T:3
        case 1002: _goto(1003); // RET PO T:4
        case 1003: _wait();_mread(cpu->sp++);_goto(1004); // RET PO T:5
        case 1004: cpu->wzh=_gd();cpu->pc=cpu->wz;_goto(1005); // RET PO T:6
        case 1005: _fetch(); // RET PO T:7
        case 1006: _wait();_mread(cpu->sp++);_goto(1007); // POP HL T:1
        case 1007: cpu->hlx[cpu->hlx_idx].l=_gd();_goto(1008); // POP HL T:2
        case 1008: _goto(1009); // POP HL T:3
        case 1009: _wait();_mread(cpu->sp++);_goto(1010); // POP HL T:4
        case 1010: cpu->hlx[cpu->hlx_idx].h=_gd();_goto(1011); // POP HL T:5
        case 1011: _fetch(); // POP HL T:6
        case 1012: _wait();_mread(cpu->pc++);_goto(1013); // JP PO,nn T:1
        case 1013: cpu->wzl=_gd();_goto(1014); // JP PO,nn T:2
        case 1014: _goto(1015); // JP PO,nn T:3
        case 1015: _wait();_mread(cpu->pc++);_goto(1016); // JP PO,nn T:4
        case 1016: cpu->wzh=_gd();if(_cc_po){cpu->pc=cpu->wz;};_goto(1017); // JP PO,nn T:5
        case 1017: _fetch(); // JP PO,nn T:6
        case 1018: _wait();_mread(cpu->sp);_goto(1019); // EX (SP),HL T:1
        case 1019: cpu->wzl=_gd();_goto(1020); // EX (SP),HL T:2
        case 1020: _goto(1021); // EX (SP),HL T:3
        case 1021: _wait();_mread(cpu->sp+1);_goto(1022); // EX (SP),HL T:4
        case 1022: cpu->wzh=_gd();_goto(1023); // EX (SP),HL T:5
        case 1023: _goto(1024); // EX (SP),HL T:6
        case 1024: _goto(1025); // EX (SP),HL T:7
        case 1025: _wait();_mwrite(cpu->sp+1,cpu->hlx[cpu->hlx_idx].h);_goto(1026); // EX (SP),HL T:8
        case 1026: _goto(1027); // EX (SP),HL T:9
        case 1027: _goto(1028); // EX (SP),HL T:10
        case 1028: _wait();_mwrite(cpu->sp,cpu->hlx[cpu->hlx_idx].l);cpu->hlx[cpu->hlx_idx].hl=cpu->wz;_goto(1029); // EX (SP),HL T:11
        case 1029: _goto(1030); // EX (SP),HL T:12
        case 1030: _goto(1031); // EX (SP),HL T:13
        case 1031: _goto(1032); // EX (SP),HL T:14
        case 1032: _fetch(); // EX (SP),HL T:15
        case 1033: _wait();_mread(cpu->pc++);_goto(1034); // CALL PO,nn T:1
        case 1034: cpu->wzl=_gd();_goto(1035); // CALL PO,nn T:2
        case 1035: _goto(1036); // CALL PO,nn T:3
        case 1036: _wait();_mread(cpu->pc++);_goto(1037); // CALL PO,nn T:4
        case 1037: cpu->wzh=_gd();if (!_cc_po){_goto(1038+7);};_goto(1038); // CALL PO,nn T:5
        case 1038: _goto(1039); // CALL PO,nn T:6
        case 1039: _goto(1040); // CALL PO,nn T:7
        case 1040: _wait();_mwrite(--cpu->sp,cpu->pch);_goto(1041); // CALL PO,nn T:8
        case 1041: _goto(1042); // CALL PO,nn T:9
        case 1042: _goto(1043); // CALL PO,nn T:10
        case 1043: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz;_goto(1044); // CALL PO,nn T:11
        case 1044: _goto(1045); // CALL PO,nn T:12
        case 1045: _fetch(); // CALL PO,nn T:13
        case 1046: _goto(1047); // PUSH HL T:1
        case 1047: _wait();_mwrite(--cpu->sp,cpu->hlx[cpu->hlx_idx].h);_goto(1048); // PUSH HL T:2
        case 1048: _goto(1049); // PUSH HL T:3
        case 1049: _goto(1050); // PUSH HL T:4
        case 1050: _wait();_mwrite(--cpu->sp,cpu->hlx[cpu->hlx_idx].l);_goto(1051); // PUSH HL T:5
        case 1051: _goto(1052); // PUSH HL T:6
        case 1052: _fetch(); // PUSH HL T:7
        case 1053: _wait();_mread(cpu->pc++);_goto(1054); // AND n T:1
        case 1054: cpu->dlatch=_gd();_goto(1055); // AND n T:2
        case 1055: _z80_and8(cpu,cpu->dlatch);_fetch(); // AND n T:3
        case 1056: _goto(1057); // RST 20h T:1
        case 1057: _wait();_mwrite(--cpu->sp,cpu->pch);_goto(1058); // RST 20h T:2
        case 1058: _goto(1059); // RST 20h T:3
        case 1059: _goto(1060); // RST 20h T:4
        case 1060: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x20;cpu->pc=cpu->wz;_goto(1061); // RST 20h T:5
        case 1061: _goto(1062); // RST 20h T:6
        case 1062: _fetch(); // RST 20h T:7
        case 1063: _goto(1064); // RET PE T:1
        case 1064: _wait();_mread(cpu->sp++);_goto(1065); // RET PE T:2
        case 1065: cpu->wzl=_gd();_goto(1066); // RET PE T:3
        case 1066: _goto(1067); // RET PE T:4
        case 1067: _wait();_mread(cpu->sp++);_goto(1068); // RET PE T:5
        case 1068: cpu->wzh=_gd();cpu->pc=cpu->wz;_goto(1069); // RET PE T:6
        case 1069: _fetch(); // RET PE T:7
        case 1070: _wait();_mread(cpu->pc++);_goto(1071); // JP PE,nn T:1
        case 1071: cpu->wzl=_gd();_goto(1072); // JP PE,nn T:2
        case 1072: _goto(1073); // JP PE,nn T:3
        case 1073: _wait();_mread(cpu->pc++);_goto(1074); // JP PE,nn T:4
        case 1074: cpu->wzh=_gd();if(_cc_pe){cpu->pc=cpu->wz;};_goto(1075); // JP PE,nn T:5
        case 1075: _fetch(); // JP PE,nn T:6
        case 1076: _wait();_mread(cpu->pc++);_goto(1077); // CALL PE,nn T:1
        case 1077: cpu->wzl=_gd();_goto(1078); // CALL PE,nn T:2
        case 1078: _goto(1079); // CALL PE,nn T:3
        case 1079: _wait();_mread(cpu->pc++);_goto(1080); // CALL PE,nn T:4
        case 1080: cpu->wzh=_gd();if (!_cc_pe){_goto(1081+7);};_goto(1081); // CALL PE,nn T:5
        case 1081: _goto(1082); // CALL PE,nn T:6
        case 1082: _goto(1083); // CALL PE,nn T:7
        case 1083: _wait();_mwrite(--cpu->sp,cpu->pch);_goto(1084); // CALL PE,nn T:8
        case 1084: _goto(1085); // CALL PE,nn T:9
        case 1085: _goto(1086); // CALL PE,nn T:10
        case 1086: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz;_goto(1087); // CALL PE,nn T:11
        case 1087: _goto(1088); // CALL PE,nn T:12
        case 1088: _fetch(); // CALL PE,nn T:13
        case 1089: _wait();_mread(cpu->pc++);_goto(1090); // XOR n T:1
        case 1090: cpu->dlatch=_gd();_goto(1091); // XOR n T:2
        case 1091: _z80_xor8(cpu,cpu->dlatch);_fetch(); // XOR n T:3
        case 1092: _goto(1093); // RST 28h T:1
        case 1093: _wait();_mwrite(--cpu->sp,cpu->pch);_goto(1094); // RST 28h T:2
        case 1094: _goto(1095); // RST 28h T:3
        case 1095: _goto(1096); // RST 28h T:4
        case 1096: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x28;cpu->pc=cpu->wz;_goto(1097); // RST 28h T:5
        case 1097: _goto(1098); // RST 28h T:6
        case 1098: _fetch(); // RST 28h T:7
        case 1099: _goto(1100); // RET P T:1
        case 1100: _wait();_mread(cpu->sp++);_goto(1101); // RET P T:2
        case 1101: cpu->wzl=_gd();_goto(1102); // RET P T:3
        case 1102: _goto(1103); // RET P T:4
        case 1103: _wait();_mread(cpu->sp++);_goto(1104); // RET P T:5
        case 1104: cpu->wzh=_gd();cpu->pc=cpu->wz;_goto(1105); // RET P T:6
        case 1105: _fetch(); // RET P T:7
        case 1106: _wait();_mread(cpu->sp++);_goto(1107); // POP AF T:1
        case 1107: cpu->f=_gd();_goto(1108); // POP AF T:2
        case 1108: _goto(1109); // POP AF T:3
        case 1109: _wait();_mread(cpu->sp++);_goto(1110); // POP AF T:4
        case 1110: cpu->a=_gd();_goto(1111); // POP AF T:5
        case 1111: _fetch(); // POP AF T:6
        case 1112: _wait();_mread(cpu->pc++);_goto(1113); // JP P,nn T:1
        case 1113: cpu->wzl=_gd();_goto(1114); // JP P,nn T:2
        case 1114: _goto(1115); // JP P,nn T:3
        case 1115: _wait();_mread(cpu->pc++);_goto(1116); // JP P,nn T:4
        case 1116: cpu->wzh=_gd();if(_cc_p){cpu->pc=cpu->wz;};_goto(1117); // JP P,nn T:5
        case 1117: _fetch(); // JP P,nn T:6
        case 1118: _wait();_mread(cpu->pc++);_goto(1119); // CALL P,nn T:1
        case 1119: cpu->wzl=_gd();_goto(1120); // CALL P,nn T:2
        case 1120: _goto(1121); // CALL P,nn T:3
        case 1121: _wait();_mread(cpu->pc++);_goto(1122); // CALL P,nn T:4
        case 1122: cpu->wzh=_gd();if (!_cc_p){_goto(1123+7);};_goto(1123); // CALL P,nn T:5
        case 1123: _goto(1124); // CALL P,nn T:6
        case 1124: _goto(1125); // CALL P,nn T:7
        case 1125: _wait();_mwrite(--cpu->sp,cpu->pch);_goto(1126); // CALL P,nn T:8
        case 1126: _goto(1127); // CALL P,nn T:9
        case 1127: _goto(1128); // CALL P,nn T:10
        case 1128: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz;_goto(1129); // CALL P,nn T:11
        case 1129: _goto(1130); // CALL P,nn T:12
        case 1130: _fetch(); // CALL P,nn T:13
        case 1131: _goto(1132); // PUSH AF T:1
        case 1132: _wait();_mwrite(--cpu->sp,cpu->a);_goto(1133); // PUSH AF T:2
        case 1133: _goto(1134); // PUSH AF T:3
        case 1134: _goto(1135); // PUSH AF T:4
        case 1135: _wait();_mwrite(--cpu->sp,cpu->f);_goto(1136); // PUSH AF T:5
        case 1136: _goto(1137); // PUSH AF T:6
        case 1137: _fetch(); // PUSH AF T:7
        case 1138: _wait();_mread(cpu->pc++);_goto(1139); // OR n T:1
        case 1139: cpu->dlatch=_gd();_goto(1140); // OR n T:2
        case 1140: _z80_or8(cpu,cpu->dlatch);_fetch(); // OR n T:3
        case 1141: _goto(1142); // RST 30h T:1
        case 1142: _wait();_mwrite(--cpu->sp,cpu->pch);_goto(1143); // RST 30h T:2
        case 1143: _goto(1144); // RST 30h T:3
        case 1144: _goto(1145); // RST 30h T:4
        case 1145: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x30;cpu->pc=cpu->wz;_goto(1146); // RST 30h T:5
        case 1146: _goto(1147); // RST 30h T:6
        case 1147: _fetch(); // RST 30h T:7
        case 1148: _goto(1149); // RET M T:1
        case 1149: _wait();_mread(cpu->sp++);_goto(1150); // RET M T:2
        case 1150: cpu->wzl=_gd();_goto(1151); // RET M T:3
        case 1151: _goto(1152); // RET M T:4
        case 1152: _wait();_mread(cpu->sp++);_goto(1153); // RET M T:5
        case 1153: cpu->wzh=_gd();cpu->pc=cpu->wz;_goto(1154); // RET M T:6
        case 1154: _fetch(); // RET M T:7
        case 1155: _goto(1156); // LD SP,HL T:1
        case 1156: _fetch(); // LD SP,HL T:2
        case 1157: _wait();_mread(cpu->pc++);_goto(1158); // JP M,nn T:1
        case 1158: cpu->wzl=_gd();_goto(1159); // JP M,nn T:2
        case 1159: _goto(1160); // JP M,nn T:3
        case 1160: _wait();_mread(cpu->pc++);_goto(1161); // JP M,nn T:4
        case 1161: cpu->wzh=_gd();if(_cc_m){cpu->pc=cpu->wz;};_goto(1162); // JP M,nn T:5
        case 1162: _fetch(); // JP M,nn T:6
        case 1163: _wait();_mread(cpu->pc++);_goto(1164); // CALL M,nn T:1
        case 1164: cpu->wzl=_gd();_goto(1165); // CALL M,nn T:2
        case 1165: _goto(1166); // CALL M,nn T:3
        case 1166: _wait();_mread(cpu->pc++);_goto(1167); // CALL M,nn T:4
        case 1167: cpu->wzh=_gd();if (!_cc_m){_goto(1168+7);};_goto(1168); // CALL M,nn T:5
        case 1168: _goto(1169); // CALL M,nn T:6
        case 1169: _goto(1170); // CALL M,nn T:7
        case 1170: _wait();_mwrite(--cpu->sp,cpu->pch);_goto(1171); // CALL M,nn T:8
        case 1171: _goto(1172); // CALL M,nn T:9
        case 1172: _goto(1173); // CALL M,nn T:10
        case 1173: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz;_goto(1174); // CALL M,nn T:11
        case 1174: _goto(1175); // CALL M,nn T:12
        case 1175: _fetch(); // CALL M,nn T:13
        case 1176: _wait();_mread(cpu->pc++);_goto(1177); // CP n T:1
        case 1177: cpu->dlatch=_gd();_goto(1178); // CP n T:2
        case 1178: _z80_cp8(cpu,cpu->dlatch);_fetch(); // CP n T:3
        case 1179: _goto(1180); // RST 38h T:1
        case 1180: _wait();_mwrite(--cpu->sp,cpu->pch);_goto(1181); // RST 38h T:2
        case 1181: _goto(1182); // RST 38h T:3
        case 1182: _goto(1183); // RST 38h T:4
        case 1183: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x38;cpu->pc=cpu->wz;_goto(1184); // RST 38h T:5
        case 1184: _goto(1185); // RST 38h T:6
        case 1185: _fetch(); // RST 38h T:7
        case 1186: _goto(1187); // IN B,(C) T:1
        case 1187: _wait();_ioread(cpu->bc);_goto(1188); // IN B,(C) T:2
        case 1188: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;_goto(1189); // IN B,(C) T:3
        case 1189: cpu->b=_z80_in(cpu,cpu->dlatch);_fetch(); // IN B,(C) T:4
        case 1190: _iowrite(cpu->bc,cpu->b);_goto(1191); // OUT (C),B T:1
        case 1191: _wait();cpu->wz=cpu->bc+1;_goto(1192); // OUT (C),B T:2
        case 1192: _goto(1193); // OUT (C),B T:3
        case 1193: _fetch(); // OUT (C),B T:4
        case 1194: _goto(1195); // SBC HL,BC T:1
        case 1195: _goto(1196); // SBC HL,BC T:2
        case 1196: _goto(1197); // SBC HL,BC T:3
        case 1197: _goto(1198); // SBC HL,BC T:4
        case 1198: _goto(1199); // SBC HL,BC T:5
        case 1199: _goto(1200); // SBC HL,BC T:6
        case 1200: _fetch(); // SBC HL,BC T:7
        case 1201: _wait();_mread(cpu->pc++);_goto(1202); // LD (nn),BC T:1
        case 1202: cpu->wzl=_gd();_goto(1203); // LD (nn),BC T:2
        case 1203: _goto(1204); // LD (nn),BC T:3
        case 1204: _wait();_mread(cpu->pc++);_goto(1205); // LD (nn),BC T:4
        case 1205: cpu->wzh=_gd();_goto(1206); // LD (nn),BC T:5
        case 1206: _goto(1207); // LD (nn),BC T:6
        case 1207: _wait();_mwrite(cpu->wz++,cpu->c);_goto(1208); // LD (nn),BC T:7
        case 1208: _goto(1209); // LD (nn),BC T:8
        case 1209: _goto(1210); // LD (nn),BC T:9
        case 1210: _wait();_mwrite(cpu->wz,cpu->b);_goto(1211); // LD (nn),BC T:10
        case 1211: _goto(1212); // LD (nn),BC T:11
        case 1212: _fetch(); // LD (nn),BC T:12
        case 1213: _wait();_mread(cpu->sp++);_goto(1214); // RETN T:1
        case 1214: cpu->wzl=_gd();_goto(1215); // RETN T:2
        case 1215: _goto(1216); // RETN T:3
        case 1216: _wait();_mread(cpu->sp++);_goto(1217); // RETN T:4
        case 1217: cpu->wzh=_gd();cpu->pc=cpu->wz;_goto(1218); // RETN T:5
        case 1218: pins=_z80_fetch(cpu,pins);cpu->iff1=cpu->iff2;goto step_to; // RETN T:6
        case 1219: cpu->i=cpu->a;_fetch(); // LD I,A T:1
        case 1220: _goto(1221); // IN C,(C) T:1
        case 1221: _wait();_ioread(cpu->bc);_goto(1222); // IN C,(C) T:2
        case 1222: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;_goto(1223); // IN C,(C) T:3
        case 1223: cpu->c=_z80_in(cpu,cpu->dlatch);_fetch(); // IN C,(C) T:4
        case 1224: _iowrite(cpu->bc,cpu->c);_goto(1225); // OUT (C),C T:1
        case 1225: _wait();cpu->wz=cpu->bc+1;_goto(1226); // OUT (C),C T:2
        case 1226: _goto(1227); // OUT (C),C T:3
        case 1227: _fetch(); // OUT (C),C T:4
        case 1228: _goto(1229); // ADC HL,BC T:1
        case 1229: _goto(1230); // ADC HL,BC T:2
        case 1230: _goto(1231); // ADC HL,BC T:3
        case 1231: _goto(1232); // ADC HL,BC T:4
        case 1232: _goto(1233); // ADC HL,BC T:5
        case 1233: _goto(1234); // ADC HL,BC T:6
        case 1234: _fetch(); // ADC HL,BC T:7
        case 1235: _wait();_mread(cpu->pc++);_goto(1236); // LD BC,(nn) T:1
        case 1236: cpu->wzl=_gd();_goto(1237); // LD BC,(nn) T:2
        case 1237: _goto(1238); // LD BC,(nn) T:3
        case 1238: _wait();_mread(cpu->pc++);_goto(1239); // LD BC,(nn) T:4
        case 1239: cpu->wzh=_gd();_goto(1240); // LD BC,(nn) T:5
        case 1240: _goto(1241); // LD BC,(nn) T:6
        case 1241: _wait();_mread(cpu->wz++);_goto(1242); // LD BC,(nn) T:7
        case 1242: cpu->c=_gd();_goto(1243); // LD BC,(nn) T:8
        case 1243: _goto(1244); // LD BC,(nn) T:9
        case 1244: _wait();_mread(cpu->wz);_goto(1245); // LD BC,(nn) T:10
        case 1245: cpu->b=_gd();_goto(1246); // LD BC,(nn) T:11
        case 1246: _fetch(); // LD BC,(nn) T:12
        case 1247: _wait();_mread(cpu->sp++);_goto(1248); // RETI T:1
        case 1248: cpu->wzl=_gd();pins|=Z80_RETI;_goto(1249); // RETI T:2
        case 1249: _goto(1250); // RETI T:3
        case 1250: _wait();_mread(cpu->sp++);_goto(1251); // RETI T:4
        case 1251: cpu->wzh=_gd();cpu->pc=cpu->wz;_goto(1252); // RETI T:5
        case 1252: pins=_z80_fetch(cpu,pins);cpu->iff1=cpu->iff2;goto step_to; // RETI T:6
        case 1253: cpu->r=cpu->a;_fetch(); // LD R,A T:1
        case 1254: _goto(1255); // IN D,(C) T:1
        case 1255: _wait();_ioread(cpu->bc);_goto(1256); // IN D,(C) T:2
        case 1256: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;_goto(1257); // IN D,(C) T:3
        case 1257: cpu->d=_z80_in(cpu,cpu->dlatch);_fetch(); // IN D,(C) T:4
        case 1258: _iowrite(cpu->bc,cpu->d);_goto(1259); // OUT (C),D T:1
        case 1259: _wait();cpu->wz=cpu->bc+1;_goto(1260); // OUT (C),D T:2
        case 1260: _goto(1261); // OUT (C),D T:3
        case 1261: _fetch(); // OUT (C),D T:4
        case 1262: _goto(1263); // SBC HL,DE T:1
        case 1263: _goto(1264); // SBC HL,DE T:2
        case 1264: _goto(1265); // SBC HL,DE T:3
        case 1265: _goto(1266); // SBC HL,DE T:4
        case 1266: _goto(1267); // SBC HL,DE T:5
        case 1267: _goto(1268); // SBC HL,DE T:6
        case 1268: _fetch(); // SBC HL,DE T:7
        case 1269: _wait();_mread(cpu->pc++);_goto(1270); // LD (nn),DE T:1
        case 1270: cpu->wzl=_gd();_goto(1271); // LD (nn),DE T:2
        case 1271: _goto(1272); // LD (nn),DE T:3
        case 1272: _wait();_mread(cpu->pc++);_goto(1273); // LD (nn),DE T:4
        case 1273: cpu->wzh=_gd();_goto(1274); // LD (nn),DE T:5
        case 1274: _goto(1275); // LD (nn),DE T:6
        case 1275: _wait();_mwrite(cpu->wz++,cpu->e);_goto(1276); // LD (nn),DE T:7
        case 1276: _goto(1277); // LD (nn),DE T:8
        case 1277: _goto(1278); // LD (nn),DE T:9
        case 1278: _wait();_mwrite(cpu->wz,cpu->d);_goto(1279); // LD (nn),DE T:10
        case 1279: _goto(1280); // LD (nn),DE T:11
        case 1280: _fetch(); // LD (nn),DE T:12
        case 1281: pins=_z80_fetch(cpu,pins);cpu->iff1=cpu->iff2;goto step_to; // RETI T:6
        case 1282: cpu->a=cpu->i;cpu->f=_z80_sziff2_flags(cpu, cpu->i);_fetch(); // LD A,I T:1
        case 1283: _goto(1284); // IN E,(C) T:1
        case 1284: _wait();_ioread(cpu->bc);_goto(1285); // IN E,(C) T:2
        case 1285: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;_goto(1286); // IN E,(C) T:3
        case 1286: cpu->e=_z80_in(cpu,cpu->dlatch);_fetch(); // IN E,(C) T:4
        case 1287: _iowrite(cpu->bc,cpu->e);_goto(1288); // OUT (C),E T:1
        case 1288: _wait();cpu->wz=cpu->bc+1;_goto(1289); // OUT (C),E T:2
        case 1289: _goto(1290); // OUT (C),E T:3
        case 1290: _fetch(); // OUT (C),E T:4
        case 1291: _goto(1292); // ADC HL,DE T:1
        case 1292: _goto(1293); // ADC HL,DE T:2
        case 1293: _goto(1294); // ADC HL,DE T:3
        case 1294: _goto(1295); // ADC HL,DE T:4
        case 1295: _goto(1296); // ADC HL,DE T:5
        case 1296: _goto(1297); // ADC HL,DE T:6
        case 1297: _fetch(); // ADC HL,DE T:7
        case 1298: _wait();_mread(cpu->pc++);_goto(1299); // LD DE,(nn) T:1
        case 1299: cpu->wzl=_gd();_goto(1300); // LD DE,(nn) T:2
        case 1300: _goto(1301); // LD DE,(nn) T:3
        case 1301: _wait();_mread(cpu->pc++);_goto(1302); // LD DE,(nn) T:4
        case 1302: cpu->wzh=_gd();_goto(1303); // LD DE,(nn) T:5
        case 1303: _goto(1304); // LD DE,(nn) T:6
        case 1304: _wait();_mread(cpu->wz++);_goto(1305); // LD DE,(nn) T:7
        case 1305: cpu->e=_gd();_goto(1306); // LD DE,(nn) T:8
        case 1306: _goto(1307); // LD DE,(nn) T:9
        case 1307: _wait();_mread(cpu->wz);_goto(1308); // LD DE,(nn) T:10
        case 1308: cpu->d=_gd();_goto(1309); // LD DE,(nn) T:11
        case 1309: _fetch(); // LD DE,(nn) T:12
        case 1310: pins=_z80_fetch(cpu,pins);cpu->iff1=cpu->iff2;goto step_to; // RETI T:6
        case 1311: cpu->a=cpu->r;cpu->f=_z80_sziff2_flags(cpu, cpu->r);_fetch(); // LD A,R T:1
        case 1312: _goto(1313); // IN H,(C) T:1
        case 1313: _wait();_ioread(cpu->bc);_goto(1314); // IN H,(C) T:2
        case 1314: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;_goto(1315); // IN H,(C) T:3
        case 1315: cpu->h=_z80_in(cpu,cpu->dlatch);_fetch(); // IN H,(C) T:4
        case 1316: _iowrite(cpu->bc,cpu->h);_goto(1317); // OUT (C),H T:1
        case 1317: _wait();cpu->wz=cpu->bc+1;_goto(1318); // OUT (C),H T:2
        case 1318: _goto(1319); // OUT (C),H T:3
        case 1319: _fetch(); // OUT (C),H T:4
        case 1320: _goto(1321); // SBC HL,HL T:1
        case 1321: _goto(1322); // SBC HL,HL T:2
        case 1322: _goto(1323); // SBC HL,HL T:3
        case 1323: _goto(1324); // SBC HL,HL T:4
        case 1324: _goto(1325); // SBC HL,HL T:5
        case 1325: _goto(1326); // SBC HL,HL T:6
        case 1326: _fetch(); // SBC HL,HL T:7
        case 1327: _wait();_mread(cpu->pc++);_goto(1328); // LD (nn),HL T:1
        case 1328: cpu->wzl=_gd();_goto(1329); // LD (nn),HL T:2
        case 1329: _goto(1330); // LD (nn),HL T:3
        case 1330: _wait();_mread(cpu->pc++);_goto(1331); // LD (nn),HL T:4
        case 1331: cpu->wzh=_gd();_goto(1332); // LD (nn),HL T:5
        case 1332: _goto(1333); // LD (nn),HL T:6
        case 1333: _wait();_mwrite(cpu->wz++,cpu->l);_goto(1334); // LD (nn),HL T:7
        case 1334: _goto(1335); // LD (nn),HL T:8
        case 1335: _goto(1336); // LD (nn),HL T:9
        case 1336: _wait();_mwrite(cpu->wz,cpu->h);_goto(1337); // LD (nn),HL T:10
        case 1337: _goto(1338); // LD (nn),HL T:11
        case 1338: _fetch(); // LD (nn),HL T:12
        case 1339: pins=_z80_fetch(cpu,pins);cpu->iff1=cpu->iff2;goto step_to; // RETI T:6
        case 1340: _wait();_mread(cpu->hl);_goto(1341); // RRD T:1
        case 1341: cpu->dlatch=_gd();_goto(1342); // RRD T:2
        case 1342: cpu->dlatch=_z80_rrd(cpu,cpu->dlatch);_goto(1343); // RRD T:3
        case 1343: _goto(1344); // RRD T:4
        case 1344: _goto(1345); // RRD T:5
        case 1345: _goto(1346); // RRD T:6
        case 1346: _goto(1347); // RRD T:7
        case 1347: _wait();_mwrite(cpu->hl,cpu->dlatch);cpu->wz=cpu->hl+1;_goto(1348); // RRD T:8
        case 1348: _goto(1349); // RRD T:9
        case 1349: _fetch(); // RRD T:10
        case 1350: _goto(1351); // IN L,(C) T:1
        case 1351: _wait();_ioread(cpu->bc);_goto(1352); // IN L,(C) T:2
        case 1352: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;_goto(1353); // IN L,(C) T:3
        case 1353: cpu->l=_z80_in(cpu,cpu->dlatch);_fetch(); // IN L,(C) T:4
        case 1354: _iowrite(cpu->bc,cpu->l);_goto(1355); // OUT (C),L T:1
        case 1355: _wait();cpu->wz=cpu->bc+1;_goto(1356); // OUT (C),L T:2
        case 1356: _goto(1357); // OUT (C),L T:3
        case 1357: _fetch(); // OUT (C),L T:4
        case 1358: _goto(1359); // ADC HL,HL T:1
        case 1359: _goto(1360); // ADC HL,HL T:2
        case 1360: _goto(1361); // ADC HL,HL T:3
        case 1361: _goto(1362); // ADC HL,HL T:4
        case 1362: _goto(1363); // ADC HL,HL T:5
        case 1363: _goto(1364); // ADC HL,HL T:6
        case 1364: _fetch(); // ADC HL,HL T:7
        case 1365: _wait();_mread(cpu->pc++);_goto(1366); // LD HL,(nn) T:1
        case 1366: cpu->wzl=_gd();_goto(1367); // LD HL,(nn) T:2
        case 1367: _goto(1368); // LD HL,(nn) T:3
        case 1368: _wait();_mread(cpu->pc++);_goto(1369); // LD HL,(nn) T:4
        case 1369: cpu->wzh=_gd();_goto(1370); // LD HL,(nn) T:5
        case 1370: _goto(1371); // LD HL,(nn) T:6
        case 1371: _wait();_mread(cpu->wz++);_goto(1372); // LD HL,(nn) T:7
        case 1372: cpu->l=_gd();_goto(1373); // LD HL,(nn) T:8
        case 1373: _goto(1374); // LD HL,(nn) T:9
        case 1374: _wait();_mread(cpu->wz);_goto(1375); // LD HL,(nn) T:10
        case 1375: cpu->h=_gd();_goto(1376); // LD HL,(nn) T:11
        case 1376: _fetch(); // LD HL,(nn) T:12
        case 1377: pins=_z80_fetch(cpu,pins);cpu->iff1=cpu->iff2;goto step_to; // RETI T:6
        case 1378: _wait();_mread(cpu->hl);_goto(1379); // RLD T:1
        case 1379: cpu->dlatch=_gd();_goto(1380); // RLD T:2
        case 1380: cpu->dlatch=_z80_rld(cpu,cpu->dlatch);_goto(1381); // RLD T:3
        case 1381: _goto(1382); // RLD T:4
        case 1382: _goto(1383); // RLD T:5
        case 1383: _goto(1384); // RLD T:6
        case 1384: _goto(1385); // RLD T:7
        case 1385: _wait();_mwrite(cpu->hl,cpu->dlatch);cpu->wz=cpu->hl+1;_goto(1386); // RLD T:8
        case 1386: _goto(1387); // RLD T:9
        case 1387: _fetch(); // RLD T:10
        case 1388: _goto(1389); // IN (C) T:1
        case 1389: _wait();_ioread(cpu->bc);_goto(1390); // IN (C) T:2
        case 1390: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;_goto(1391); // IN (C) T:3
        case 1391: _z80_in(cpu,cpu->dlatch);_fetch(); // IN (C) T:4
        case 1392: _iowrite(cpu->bc,0);_goto(1393); // OUT (C),0 T:1
        case 1393: _wait();cpu->wz=cpu->bc+1;_goto(1394); // OUT (C),0 T:2
        case 1394: _goto(1395); // OUT (C),0 T:3
        case 1395: _fetch(); // OUT (C),0 T:4
        case 1396: _goto(1397); // SBC HL,SP T:1
        case 1397: _goto(1398); // SBC HL,SP T:2
        case 1398: _goto(1399); // SBC HL,SP T:3
        case 1399: _goto(1400); // SBC HL,SP T:4
        case 1400: _goto(1401); // SBC HL,SP T:5
        case 1401: _goto(1402); // SBC HL,SP T:6
        case 1402: _fetch(); // SBC HL,SP T:7
        case 1403: _wait();_mread(cpu->pc++);_goto(1404); // LD (nn),SP T:1
        case 1404: cpu->wzl=_gd();_goto(1405); // LD (nn),SP T:2
        case 1405: _goto(1406); // LD (nn),SP T:3
        case 1406: _wait();_mread(cpu->pc++);_goto(1407); // LD (nn),SP T:4
        case 1407: cpu->wzh=_gd();_goto(1408); // LD (nn),SP T:5
        case 1408: _goto(1409); // LD (nn),SP T:6
        case 1409: _wait();_mwrite(cpu->wz++,cpu->spl);_goto(1410); // LD (nn),SP T:7
        case 1410: _goto(1411); // LD (nn),SP T:8
        case 1411: _goto(1412); // LD (nn),SP T:9
        case 1412: _wait();_mwrite(cpu->wz,cpu->sph);_goto(1413); // LD (nn),SP T:10
        case 1413: _goto(1414); // LD (nn),SP T:11
        case 1414: _fetch(); // LD (nn),SP T:12
        case 1415: pins=_z80_fetch(cpu,pins);cpu->iff1=cpu->iff2;goto step_to; // RETI T:6
        case 1416: _goto(1417); // IN A,(C) T:1
        case 1417: _wait();_ioread(cpu->bc);_goto(1418); // IN A,(C) T:2
        case 1418: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;_goto(1419); // IN A,(C) T:3
        case 1419: cpu->a=_z80_in(cpu,cpu->dlatch);_fetch(); // IN A,(C) T:4
        case 1420: _iowrite(cpu->bc,cpu->a);_goto(1421); // OUT (C),A T:1
        case 1421: _wait();cpu->wz=cpu->bc+1;_goto(1422); // OUT (C),A T:2
        case 1422: _goto(1423); // OUT (C),A T:3
        case 1423: _fetch(); // OUT (C),A T:4
        case 1424: _goto(1425); // ADC HL,SP T:1
        case 1425: _goto(1426); // ADC HL,SP T:2
        case 1426: _goto(1427); // ADC HL,SP T:3
        case 1427: _goto(1428); // ADC HL,SP T:4
        case 1428: _goto(1429); // ADC HL,SP T:5
        case 1429: _goto(1430); // ADC HL,SP T:6
        case 1430: _fetch(); // ADC HL,SP T:7
        case 1431: _wait();_mread(cpu->pc++);_goto(1432); // LD SP,(nn) T:1
        case 1432: cpu->wzl=_gd();_goto(1433); // LD SP,(nn) T:2
        case 1433: _goto(1434); // LD SP,(nn) T:3
        case 1434: _wait();_mread(cpu->pc++);_goto(1435); // LD SP,(nn) T:4
        case 1435: cpu->wzh=_gd();_goto(1436); // LD SP,(nn) T:5
        case 1436: _goto(1437); // LD SP,(nn) T:6
        case 1437: _wait();_mread(cpu->wz++);_goto(1438); // LD SP,(nn) T:7
        case 1438: cpu->spl=_gd();_goto(1439); // LD SP,(nn) T:8
        case 1439: _goto(1440); // LD SP,(nn) T:9
        case 1440: _wait();_mread(cpu->wz);_goto(1441); // LD SP,(nn) T:10
        case 1441: cpu->sph=_gd();_goto(1442); // LD SP,(nn) T:11
        case 1442: _fetch(); // LD SP,(nn) T:12
        case 1443: pins=_z80_fetch(cpu,pins);cpu->iff1=cpu->iff2;goto step_to; // RETI T:6
        case 1444: _wait();_mread(cpu->hl++);_goto(1445); // LDI T:1
        case 1445: cpu->dlatch=_gd();_goto(1446); // LDI T:2
        case 1446: _goto(1447); // LDI T:3
        case 1447: _wait();_mwrite(cpu->de++,cpu->dlatch);_goto(1448); // LDI T:4
        case 1448: _goto(1449); // LDI T:5
        case 1449: _z80_ldi_ldd(cpu,cpu->dlatch);_goto(1450); // LDI T:6
        case 1450: _goto(1451); // LDI T:7
        case 1451: _fetch(); // LDI T:8
        case 1452: _wait();_mread(cpu->hl++);_goto(1453); // CPI T:1
        case 1453: cpu->dlatch=_gd();_goto(1454); // CPI T:2
        case 1454: cpu->wz++;_z80_cpi_cpd(cpu,cpu->dlatch);_goto(1455); // CPI T:3
        case 1455: _goto(1456); // CPI T:4
        case 1456: _goto(1457); // CPI T:5
        case 1457: _goto(1458); // CPI T:6
        case 1458: _goto(1459); // CPI T:7
        case 1459: _fetch(); // CPI T:8
        case 1460: _goto(1461); // INI T:1
        case 1461: _goto(1462); // INI T:2
        case 1462: _wait();_ioread(cpu->bc);_goto(1463); // INI T:3
        case 1463: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;cpu->b--;;_goto(1464); // INI T:4
        case 1464: _goto(1465); // INI T:5
        case 1465: _wait();_mwrite(cpu->hl++,cpu->dlatch);_z80_ini_ind(cpu,cpu->dlatch,cpu->c+1);_goto(1466); // INI T:6
        case 1466: _goto(1467); // INI T:7
        case 1467: _fetch(); // INI T:8
        case 1468: _goto(1469); // OUTI T:1
        case 1469: _wait();_mread(cpu->hl++);_goto(1470); // OUTI T:2
        case 1470: cpu->dlatch=_gd();cpu->b--;_goto(1471); // OUTI T:3
        case 1471: _goto(1472); // OUTI T:4
        case 1472: _iowrite(cpu->bc,cpu->dlatch);_goto(1473); // OUTI T:5
        case 1473: _wait();cpu->wz=cpu->bc+1;_z80_outi_outd(cpu,cpu->dlatch);_goto(1474); // OUTI T:6
        case 1474: _goto(1475); // OUTI T:7
        case 1475: _fetch(); // OUTI T:8
        case 1476: _wait();_mread(cpu->hl--);_goto(1477); // LDD T:1
        case 1477: cpu->dlatch=_gd();_goto(1478); // LDD T:2
        case 1478: _goto(1479); // LDD T:3
        case 1479: _wait();_mwrite(cpu->de--,cpu->dlatch);_goto(1480); // LDD T:4
        case 1480: _goto(1481); // LDD T:5
        case 1481: _z80_ldi_ldd(cpu,cpu->dlatch);_goto(1482); // LDD T:6
        case 1482: _goto(1483); // LDD T:7
        case 1483: _fetch(); // LDD T:8
        case 1484: _wait();_mread(cpu->hl--);_goto(1485); // CPD T:1
        case 1485: cpu->dlatch=_gd();_goto(1486); // CPD T:2
        case 1486: cpu->wz--;_z80_cpi_cpd(cpu,cpu->dlatch);_goto(1487); // CPD T:3
        case 1487: _goto(1488); // CPD T:4
        case 1488: _goto(1489); // CPD T:5
        case 1489: _goto(1490); // CPD T:6
        case 1490: _goto(1491); // CPD T:7
        case 1491: _fetch(); // CPD T:8
        case 1492: _goto(1493); // IND T:1
        case 1493: _goto(1494); // IND T:2
        case 1494: _wait();_ioread(cpu->bc);_goto(1495); // IND T:3
        case 1495: cpu->dlatch=_gd();cpu->wz=cpu->bc-1;cpu->b--;;_goto(1496); // IND T:4
        case 1496: _goto(1497); // IND T:5
        case 1497: _wait();_mwrite(cpu->hl--,cpu->dlatch);_z80_ini_ind(cpu,cpu->dlatch,cpu->c-1);_goto(1498); // IND T:6
        case 1498: _goto(1499); // IND T:7
        case 1499: _fetch(); // IND T:8
        case 1500: _goto(1501); // OUTD T:1
        case 1501: _wait();_mread(cpu->hl--);_goto(1502); // OUTD T:2
        case 1502: cpu->dlatch=_gd();cpu->b--;_goto(1503); // OUTD T:3
        case 1503: _goto(1504); // OUTD T:4
        case 1504: _iowrite(cpu->bc,cpu->dlatch);_goto(1505); // OUTD T:5
        case 1505: _wait();cpu->wz=cpu->bc-1;_z80_outi_outd(cpu,cpu->dlatch);_goto(1506); // OUTD T:6
        case 1506: _goto(1507); // OUTD T:7
        case 1507: _fetch(); // OUTD T:8
        case 1508: _wait();_mread(cpu->hl++);_goto(1509); // LDIR T:1
        case 1509: cpu->dlatch=_gd();_goto(1510); // LDIR T:2
        case 1510: _goto(1511); // LDIR T:3
        case 1511: _wait();_mwrite(cpu->de++,cpu->dlatch);_goto(1512); // LDIR T:4
        case 1512: _goto(1513); // LDIR T:5
        case 1513: if(!_z80_ldi_ldd(cpu,cpu->dlatch)){_goto(1514+5);};_goto(1514); // LDIR T:6
        case 1514: _goto(1515); // LDIR T:7
        case 1515: cpu->wz=--cpu->pc;--cpu->pc;;_goto(1516); // LDIR T:8
        case 1516: _goto(1517); // LDIR T:9
        case 1517: _goto(1518); // LDIR T:10
        case 1518: _goto(1519); // LDIR T:11
        case 1519: _goto(1520); // LDIR T:12
        case 1520: _fetch(); // LDIR T:13
        case 1521: _wait();_mread(cpu->hl++);_goto(1522); // CPIR T:1
        case 1522: cpu->dlatch=_gd();_goto(1523); // CPIR T:2
        case 1523: cpu->wz++;if(!_z80_cpi_cpd(cpu,cpu->dlatch)){_goto(1524+5);};_goto(1524); // CPIR T:3
        case 1524: _goto(1525); // CPIR T:4
        case 1525: _goto(1526); // CPIR T:5
        case 1526: _goto(1527); // CPIR T:6
        case 1527: _goto(1528); // CPIR T:7
        case 1528: cpu->wz=--cpu->pc;--cpu->pc;_goto(1529); // CPIR T:8
        case 1529: _goto(1530); // CPIR T:9
        case 1530: _goto(1531); // CPIR T:10
        case 1531: _goto(1532); // CPIR T:11
        case 1532: _goto(1533); // CPIR T:12
        case 1533: _fetch(); // CPIR T:13
        case 1534: _goto(1535); // INIR T:1
        case 1535: _goto(1536); // INIR T:2
        case 1536: _wait();_ioread(cpu->bc);_goto(1537); // INIR T:3
        case 1537: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;cpu->b--;;_goto(1538); // INIR T:4
        case 1538: _goto(1539); // INIR T:5
        case 1539: _wait();_mwrite(cpu->hl++,cpu->dlatch);if (!_z80_ini_ind(cpu,cpu->dlatch,cpu->c+1)){_goto(1540+5);};_goto(1540); // INIR T:6
        case 1540: _goto(1541); // INIR T:7
        case 1541: cpu->wz=--cpu->pc;--cpu->pc;_goto(1542); // INIR T:8
        case 1542: _goto(1543); // INIR T:9
        case 1543: _goto(1544); // INIR T:10
        case 1544: _goto(1545); // INIR T:11
        case 1545: _goto(1546); // INIR T:12
        case 1546: _fetch(); // INIR T:13
        case 1547: _goto(1548); // OTIR T:1
        case 1548: _wait();_mread(cpu->hl++);_goto(1549); // OTIR T:2
        case 1549: cpu->dlatch=_gd();cpu->b--;_goto(1550); // OTIR T:3
        case 1550: _goto(1551); // OTIR T:4
        case 1551: _iowrite(cpu->bc,cpu->dlatch);_goto(1552); // OTIR T:5
        case 1552: _wait();cpu->wz=cpu->bc+1;if(!_z80_outi_outd(cpu,cpu->dlatch)){_goto(1553+5);};_goto(1553); // OTIR T:6
        case 1553: _goto(1554); // OTIR T:7
        case 1554: cpu->wz=--cpu->pc;--cpu->pc;_goto(1555); // OTIR T:8
        case 1555: _goto(1556); // OTIR T:9
        case 1556: _goto(1557); // OTIR T:10
        case 1557: _goto(1558); // OTIR T:11
        case 1558: _goto(1559); // OTIR T:12
        case 1559: _fetch(); // OTIR T:13
        case 1560: _wait();_mread(cpu->hl--);_goto(1561); // LDDR T:1
        case 1561: cpu->dlatch=_gd();_goto(1562); // LDDR T:2
        case 1562: _goto(1563); // LDDR T:3
        case 1563: _wait();_mwrite(cpu->de--,cpu->dlatch);_goto(1564); // LDDR T:4
        case 1564: _goto(1565); // LDDR T:5
        case 1565: if(!_z80_ldi_ldd(cpu,cpu->dlatch)){_goto(1566+5);};_goto(1566); // LDDR T:6
        case 1566: _goto(1567); // LDDR T:7
        case 1567: cpu->wz=--cpu->pc;--cpu->pc;;_goto(1568); // LDDR T:8
        case 1568: _goto(1569); // LDDR T:9
        case 1569: _goto(1570); // LDDR T:10
        case 1570: _goto(1571); // LDDR T:11
        case 1571: _goto(1572); // LDDR T:12
        case 1572: _fetch(); // LDDR T:13
        case 1573: _wait();_mread(cpu->hl--);_goto(1574); // CPDR T:1
        case 1574: cpu->dlatch=_gd();_goto(1575); // CPDR T:2
        case 1575: cpu->wz--;if(!_z80_cpi_cpd(cpu,cpu->dlatch)){_goto(1576+5);};_goto(1576); // CPDR T:3
        case 1576: _goto(1577); // CPDR T:4
        case 1577: _goto(1578); // CPDR T:5
        case 1578: _goto(1579); // CPDR T:6
        case 1579: _goto(1580); // CPDR T:7
        case 1580: cpu->wz=--cpu->pc;--cpu->pc;_goto(1581); // CPDR T:8
        case 1581: _goto(1582); // CPDR T:9
        case 1582: _goto(1583); // CPDR T:10
        case 1583: _goto(1584); // CPDR T:11
        case 1584: _goto(1585); // CPDR T:12
        case 1585: _fetch(); // CPDR T:13
        case 1586: _goto(1587); // INDR T:1
        case 1587: _goto(1588); // INDR T:2
        case 1588: _wait();_ioread(cpu->bc);_goto(1589); // INDR T:3
        case 1589: cpu->dlatch=_gd();cpu->wz=cpu->bc-1;cpu->b--;;_goto(1590); // INDR T:4
        case 1590: _goto(1591); // INDR T:5
        case 1591: _wait();_mwrite(cpu->hl--,cpu->dlatch);if (!_z80_ini_ind(cpu,cpu->dlatch,cpu->c-1)){_goto(1592+5);};_goto(1592); // INDR T:6
        case 1592: _goto(1593); // INDR T:7
        case 1593: cpu->wz=--cpu->pc;--cpu->pc;_goto(1594); // INDR T:8
        case 1594: _goto(1595); // INDR T:9
        case 1595: _goto(1596); // INDR T:10
        case 1596: _goto(1597); // INDR T:11
        case 1597: _goto(1598); // INDR T:12
        case 1598: _fetch(); // INDR T:13
        case 1599: _goto(1600); // OTDR T:1
        case 1600: _wait();_mread(cpu->hl--);_goto(1601); // OTDR T:2
        case 1601: cpu->dlatch=_gd();cpu->b--;_goto(1602); // OTDR T:3
        case 1602: _goto(1603); // OTDR T:4
        case 1603: _iowrite(cpu->bc,cpu->dlatch);_goto(1604); // OTDR T:5
        case 1604: _wait();cpu->wz=cpu->bc-1;if(!_z80_outi_outd(cpu,cpu->dlatch)){_goto(1605+5);};_goto(1605); // OTDR T:6
        case 1605: _goto(1606); // OTDR T:7
        case 1606: cpu->wz=--cpu->pc;--cpu->pc;_goto(1607); // OTDR T:8
        case 1607: _goto(1608); // OTDR T:9
        case 1608: _goto(1609); // OTDR T:10
        case 1609: _goto(1610); // OTDR T:11
        case 1610: _goto(1611); // OTDR T:12
        case 1611: _fetch(); // OTDR T:13
        case 1612: {uint8_t z=cpu->opcode&7;_z80_cb_action(cpu,z,z);};_fetch(); // cb T:0
        case 1613: _goto(1614); // cbhl T:0
        case 1614: _wait();_mread(cpu->hl);_goto(1615); // cbhl T:1
        case 1615: cpu->dlatch=_gd();if(!_z80_cb_action(cpu,6,6)){_goto(1616+3);};_goto(1616); // cbhl T:2
        case 1616: _goto(1617); // cbhl T:3
        case 1617: _goto(1618); // cbhl T:4
        case 1618: _wait();_mwrite(cpu->hl,cpu->dlatch);_goto(1619); // cbhl T:5
        case 1619: _goto(1620); // cbhl T:6
        case 1620: _fetch(); // cbhl T:7
        case 1621: _wait();_mread(cpu->pc++);_goto(1622); // ddfdcb T:0
        case 1622: _z80_ddfdcb_addr(cpu,pins);_goto(1623); // ddfdcb T:1
        case 1623: _goto(1624); // ddfdcb T:2
        case 1624: _wait();_mread(cpu->pc++);_goto(1625); // ddfdcb T:3
        case 1625: cpu->opcode=_gd();_goto(1626); // ddfdcb T:4
        case 1626: _goto(1627); // ddfdcb T:5
        case 1627: _goto(1628); // ddfdcb T:6
        case 1628: _goto(1629); // ddfdcb T:7
        case 1629: _wait();_mread(cpu->addr);_goto(1630); // ddfdcb T:8
        case 1630: cpu->dlatch=_gd();if(!_z80_cb_action(cpu,6,cpu->opcode&7)){_goto(1631+3);};_goto(1631); // ddfdcb T:9
        case 1631: _goto(1632); // ddfdcb T:10
        case 1632: _goto(1633); // ddfdcb T:11
        case 1633: _wait();_mwrite(cpu->addr,cpu->dlatch);_goto(1634); // ddfdcb T:12
        case 1634: _goto(1635); // ddfdcb T:13
        case 1635: _fetch(); // ddfdcb T:14
        case 1636: cpu->iff1=cpu->iff2=false;_goto(1637); // int_im0 T:0
        case 1637: pins|=(Z80_M1|Z80_IORQ);_goto(1638); // int_im0 T:1
        case 1638: _wait();cpu->opcode=_z80_get_db(pins);_goto(1639); // int_im0 T:2
        case 1639: pins=_z80_refresh(cpu,pins);_goto(1640); // int_im0 T:3
        case 1640: cpu->addr=cpu->hl;_goto(cpu->opcode);_goto(1641); // int_im0 T:4
        case 1641: _fetch(); // int_im0 T:5
        case 1642: cpu->iff1=cpu->iff2=false;_goto(1643); // int_im1 T:0
        case 1643: pins|=(Z80_M1|Z80_IORQ);_goto(1644); // int_im1 T:1
        case 1644: _wait();_goto(1645); // int_im1 T:2
        case 1645: pins=_z80_refresh(cpu,pins);_goto(1646); // int_im1 T:3
        case 1646: _goto(1647); // int_im1 T:4
        case 1647: _goto(1648); // int_im1 T:5
        case 1648: _goto(1649); // int_im1 T:6
        case 1649: _wait();_mwrite(--cpu->sp,cpu->pch);_goto(1650); // int_im1 T:7
        case 1650: _goto(1651); // int_im1 T:8
        case 1651: _goto(1652); // int_im1 T:9
        case 1652: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=cpu->pc=0x0038;_goto(1653); // int_im1 T:10
        case 1653: _goto(1654); // int_im1 T:11
        case 1654: _fetch(); // int_im1 T:12
        case 1655: cpu->iff1=cpu->iff2=false;_goto(1656); // int_im2 T:0
        case 1656: pins|=(Z80_M1|Z80_IORQ);_goto(1657); // int_im2 T:1
        case 1657: _wait();cpu->dlatch=_z80_get_db(pins);_goto(1658); // int_im2 T:2
        case 1658: pins=_z80_refresh(cpu,pins);_goto(1659); // int_im2 T:3
        case 1659: _goto(1660); // int_im2 T:4
        case 1660: _goto(1661); // int_im2 T:5
        case 1661: _goto(1662); // int_im2 T:6
        case 1662: _wait();_mwrite(--cpu->sp,cpu->pch);_goto(1663); // int_im2 T:7
        case 1663: _goto(1664); // int_im2 T:8
        case 1664: _goto(1665); // int_im2 T:9
        case 1665: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wzl=cpu->dlatch;cpu->wzh=cpu->i;_goto(1666); // int_im2 T:10
        case 1666: _goto(1667); // int_im2 T:11
        case 1667: _goto(1668); // int_im2 T:12
        case 1668: _wait();_mread(cpu->wz++);_goto(1669); // int_im2 T:13
        case 1669: cpu->dlatch=_gd();_goto(1670); // int_im2 T:14
        case 1670: _goto(1671); // int_im2 T:15
        case 1671: _wait();_mread(cpu->wz);_goto(1672); // int_im2 T:16
        case 1672: cpu->wzh=_gd();cpu->wzl=cpu->dlatch;cpu->pc=cpu->wz;_goto(1673); // int_im2 T:17
        case 1673: _fetch(); // int_im2 T:18
        case 1674: _wait();cpu->iff1=false;_goto(1675); // nmi T:0
        case 1675: pins=_z80_refresh(cpu,pins);_goto(1676); // nmi T:1
        case 1676: _goto(1677); // nmi T:2
        case 1677: _goto(1678); // nmi T:3
        case 1678: _goto(1679); // nmi T:4
        case 1679: _wait();_mwrite(--cpu->sp,cpu->pch);_goto(1680); // nmi T:5
        case 1680: _goto(1681); // nmi T:6
        case 1681: _goto(1682); // nmi T:7
        case 1682: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=cpu->pc=0x0066;_goto(1683); // nmi T:8
        case 1683: _goto(1684); // nmi T:9
        case 1684: _fetch(); // nmi T:10
        // %>
        //=== shared fetch machine cycle for DD/FD-prefixed ops
        case Z80_DDFD_M1_T2: _wait(); cpu->opcode = _gd(); _goto(Z80_DDFD_M1_T3);
        case Z80_DDFD_M1_T3: pins = _z80_refresh(cpu, pins); _goto(Z80_DDFD_M1_T4);
        case Z80_DDFD_M1_T4:
            cpu->addr = cpu->hlx[cpu->hlx_idx].hl;
            _goto(_z80_indirect_table[cpu->opcode] ? Z80_DDFD_D_T1 : cpu->opcode);
        //=== optional d-loading cycle for (IX+d), (IY+d)
        case Z80_DDFD_D_T1: _goto(Z80_DDFD_D_T2);
        case Z80_DDFD_D_T2: _wait(); _mread(cpu->pc++); _goto(Z80_DDFD_D_T3);
        case Z80_DDFD_D_T3: cpu->addr += (int8_t)_gd(); cpu->wz = cpu->addr; _goto(Z80_DDFD_D_T4);
        //--- special case LD (IX/IY+d),n or filler ticks
        case Z80_DDFD_D_T4: _goto(Z80_DDFD_D_T5);
        case Z80_DDFD_D_T5: if (cpu->opcode == 0x36) { _wait();_mread(cpu->pc++); }; _goto(Z80_DDFD_D_T6);
        case Z80_DDFD_D_T6: if (cpu->opcode == 0x36) { cpu->dlatch = _gd(); }; _goto(Z80_DDFD_D_T7);
        case Z80_DDFD_D_T7: _goto(Z80_DDFD_D_T8);
        case Z80_DDFD_D_T8: _goto((cpu->opcode==0x36) ? Z80_DDFD_LDHLN_WR_T1 : cpu->opcode);
        //--- special case LD (IX/IY+d),n write mcycle
        case Z80_DDFD_LDHLN_WR_T1: _goto(Z80_DDFD_LDHLN_WR_T2);
        case Z80_DDFD_LDHLN_WR_T2: _wait(); _mwrite(cpu->addr,cpu->dlatch); _goto(Z80_DDFD_LDHLN_WR_T3);
        case Z80_DDFD_LDHLN_WR_T3: _goto(Z80_DDFD_LDHLN_OVERLAPPED);
        case Z80_DDFD_LDHLN_OVERLAPPED: _fetch();
        //=== special opcode fetch machine cycle for CB-prefixed instructions
        case Z80_CB_M1_T2: _wait(); cpu->opcode = _gd(); _goto(Z80_CB_M1_T3);
        case Z80_CB_M1_T3: pins = _z80_refresh(cpu, pins); _goto(Z80_CB_M1_T4);
        case Z80_CB_M1_T4:
            if ((cpu->opcode & 7) == 6) {
                // this is a (HL) instruction
                cpu->addr = cpu->hl;
                _goto(Z80_CBHL_STEP);
            }
            else {
                _goto(Z80_CB_STEP);
            }
        //=== special opcode fetch machine cycle for ED-prefixed instructions
        case Z80_ED_M1_T2: _wait(); cpu->opcode = _gd(); _goto(Z80_ED_M1_T3);
        case Z80_ED_M1_T3: pins = _z80_refresh(cpu, pins); _goto(Z80_ED_M1_T4);
        case Z80_ED_M1_T4: _goto(cpu->opcode + 256);
        //=== shared fetch machine cycle for non-DD/FD-prefixed ops
        case Z80_M1_T2: _wait(); cpu->opcode = _gd(); _goto(Z80_M1_T3);
        case Z80_M1_T3: pins = _z80_refresh(cpu, pins); _goto(Z80_M1_T4);
        case Z80_M1_T4: cpu->addr = cpu->hl; _goto(cpu->opcode);
        default: _Z80_UNREACHABLE;
    }
fetch_next:
    pins = _z80_fetch(cpu, pins);
step_to: {
    // track NMI 0 => 1 edge and current INT pin state, this will track the
    // relevant interrupt status up to the last instruction cycle and will
    // be checked in the first M1 cycle (during _fetch)
        const uint64_t rising_nmi = (pins ^ cpu->pins) & pins; // NMI 0 => 1
        cpu->pins = pins;
        cpu->int_bits = ((cpu->int_bits | rising_nmi) & Z80_NMI) | (pins & Z80_INT);
    }
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
