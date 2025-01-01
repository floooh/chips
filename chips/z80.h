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
#define Z80_M1_T2 1677
#define Z80_M1_T3 1678
#define Z80_M1_T4 1679
#define Z80_DDFD_M1_T2 1680
#define Z80_DDFD_M1_T3 1681
#define Z80_DDFD_M1_T4 1682
#define Z80_DDFD_D_T1 1683
#define Z80_DDFD_D_T2 1684
#define Z80_DDFD_D_T3 1685
#define Z80_DDFD_D_T4 1686
#define Z80_DDFD_D_T5 1687
#define Z80_DDFD_D_T6 1688
#define Z80_DDFD_D_T7 1689
#define Z80_DDFD_D_T8 1690
#define Z80_DDFD_LDHLN_WR_T1 1691
#define Z80_DDFD_LDHLN_WR_T2 1692
#define Z80_DDFD_LDHLN_WR_T3 1693
#define Z80_DDFD_LDHLN_OVERLAPPED 1694
#define Z80_ED_M1_T2 1695
#define Z80_ED_M1_T3 1696
#define Z80_ED_M1_T4 1697
#define Z80_CB_M1_T2 1698
#define Z80_CB_M1_T3 1699
#define Z80_CB_M1_T4 1700
#define Z80_CB_STEP 1604
#define Z80_CBHL_STEP 1605
#define Z80_DDFDCB_STEP 1613
#define Z80_INT_IM0_STEP 1628
#define Z80_INT_IM1_STEP 1634
#define Z80_INT_IM2_STEP 1647
#define Z80_NMI_STEP 1666
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
        cpu->step = 0;
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
#define _skip(n)        cpu->step+=(n);
#define _fetch_dd()     pins=_z80_fetch_dd(cpu,pins);
#define _fetch_fd()     pins=_z80_fetch_fd(cpu,pins);
#define _fetch_ed()     pins=_z80_fetch_ed(cpu,pins);
#define _fetch_cb()     pins=_z80_fetch_cb(cpu,pins);
#define _mread(ab)      _sax(ab,Z80_MREQ|Z80_RD)
#define _mwrite(ab,d)   _sadx(ab,d,Z80_MREQ|Z80_WR)
#define _ioread(ab)     _sax(ab,Z80_IORQ|Z80_RD)
#define _iowrite(ab,d)  _sadx(ab,d,Z80_IORQ|Z80_WR)
#define _wait()         {if(pins&Z80_WAIT)goto track_int_bits;}
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
        case    0: goto fetch_next; // NOP T:0
        case    1: cpu->step=512;goto step_to; // LD BC,nn T:0
        case    2: cpu->step=518;goto step_to; // LD (BC),A T:0
        case    3: cpu->bc++;cpu->step=521;goto step_to; // INC BC T:0
        case    4: cpu->b=_z80_inc8(cpu,cpu->b);goto fetch_next; // INC B T:0
        case    5: cpu->b=_z80_dec8(cpu,cpu->b);goto fetch_next; // DEC B T:0
        case    6: cpu->step=523;goto step_to; // LD B,n T:0
        case    7: _z80_rlca(cpu);goto fetch_next; // RLCA T:0
        case    8: _z80_ex_af_af2(cpu);goto fetch_next; // EX AF,AF' T:0
        case    9: _z80_add16(cpu,cpu->bc);cpu->step=526;goto step_to; // ADD HL,BC T:0
        case   10: cpu->step=533;goto step_to; // LD A,(BC) T:0
        case   11: cpu->bc--;cpu->step=536;goto step_to; // DEC BC T:0
        case   12: cpu->c=_z80_inc8(cpu,cpu->c);goto fetch_next; // INC C T:0
        case   13: cpu->c=_z80_dec8(cpu,cpu->c);goto fetch_next; // DEC C T:0
        case   14: cpu->step=538;goto step_to; // LD C,n T:0
        case   15: _z80_rrca(cpu);goto fetch_next; // RRCA T:0
        case   16: cpu->step=541;goto step_to; // DJNZ d T:0
        case   17: cpu->step=550;goto step_to; // LD DE,nn T:0
        case   18: cpu->step=556;goto step_to; // LD (DE),A T:0
        case   19: cpu->de++;cpu->step=559;goto step_to; // INC DE T:0
        case   20: cpu->d=_z80_inc8(cpu,cpu->d);goto fetch_next; // INC D T:0
        case   21: cpu->d=_z80_dec8(cpu,cpu->d);goto fetch_next; // DEC D T:0
        case   22: cpu->step=561;goto step_to; // LD D,n T:0
        case   23: _z80_rla(cpu);goto fetch_next; // RLA T:0
        case   24: cpu->step=564;goto step_to; // JR d T:0
        case   25: _z80_add16(cpu,cpu->de);cpu->step=572;goto step_to; // ADD HL,DE T:0
        case   26: cpu->step=579;goto step_to; // LD A,(DE) T:0
        case   27: cpu->de--;cpu->step=582;goto step_to; // DEC DE T:0
        case   28: cpu->e=_z80_inc8(cpu,cpu->e);goto fetch_next; // INC E T:0
        case   29: cpu->e=_z80_dec8(cpu,cpu->e);goto fetch_next; // DEC E T:0
        case   30: cpu->step=584;goto step_to; // LD E,n T:0
        case   31: _z80_rra(cpu);goto fetch_next; // RRA T:0
        case   32: cpu->step=587;goto step_to; // JR NZ,d T:0
        case   33: cpu->step=595;goto step_to; // LD HL,nn T:0
        case   34: cpu->step=601;goto step_to; // LD (nn),HL T:0
        case   35: cpu->hlx[cpu->hlx_idx].hl++;cpu->step=613;goto step_to; // INC HL T:0
        case   36: cpu->hlx[cpu->hlx_idx].h=_z80_inc8(cpu,cpu->hlx[cpu->hlx_idx].h);goto fetch_next; // INC H T:0
        case   37: cpu->hlx[cpu->hlx_idx].h=_z80_dec8(cpu,cpu->hlx[cpu->hlx_idx].h);goto fetch_next; // DEC H T:0
        case   38: cpu->step=615;goto step_to; // LD H,n T:0
        case   39: _z80_daa(cpu);goto fetch_next; // DAA T:0
        case   40: cpu->step=618;goto step_to; // JR Z,d T:0
        case   41: _z80_add16(cpu,cpu->hlx[cpu->hlx_idx].hl);cpu->step=626;goto step_to; // ADD HL,HL T:0
        case   42: cpu->step=633;goto step_to; // LD HL,(nn) T:0
        case   43: cpu->hlx[cpu->hlx_idx].hl--;cpu->step=645;goto step_to; // DEC HL T:0
        case   44: cpu->hlx[cpu->hlx_idx].l=_z80_inc8(cpu,cpu->hlx[cpu->hlx_idx].l);goto fetch_next; // INC L T:0
        case   45: cpu->hlx[cpu->hlx_idx].l=_z80_dec8(cpu,cpu->hlx[cpu->hlx_idx].l);goto fetch_next; // DEC L T:0
        case   46: cpu->step=647;goto step_to; // LD L,n T:0
        case   47: _z80_cpl(cpu);goto fetch_next; // CPL T:0
        case   48: cpu->step=650;goto step_to; // JR NC,d T:0
        case   49: cpu->step=658;goto step_to; // LD SP,nn T:0
        case   50: cpu->step=664;goto step_to; // LD (nn),A T:0
        case   51: cpu->sp++;cpu->step=673;goto step_to; // INC SP T:0
        case   52: cpu->step=675;goto step_to; // INC (HL) T:0
        case   53: cpu->step=682;goto step_to; // DEC (HL) T:0
        case   54: cpu->step=689;goto step_to; // LD (HL),n T:0
        case   55: _z80_scf(cpu);goto fetch_next; // SCF T:0
        case   56: cpu->step=695;goto step_to; // JR C,d T:0
        case   57: _z80_add16(cpu,cpu->sp);cpu->step=703;goto step_to; // ADD HL,SP T:0
        case   58: cpu->step=710;goto step_to; // LD A,(nn) T:0
        case   59: cpu->sp--;cpu->step=719;goto step_to; // DEC SP T:0
        case   60: cpu->a=_z80_inc8(cpu,cpu->a);goto fetch_next; // INC A T:0
        case   61: cpu->a=_z80_dec8(cpu,cpu->a);goto fetch_next; // DEC A T:0
        case   62: cpu->step=721;goto step_to; // LD A,n T:0
        case   63: _z80_ccf(cpu);goto fetch_next; // CCF T:0
        case   64: cpu->b=cpu->b;goto fetch_next; // LD B,B T:0
        case   65: cpu->b=cpu->c;goto fetch_next; // LD B,C T:0
        case   66: cpu->b=cpu->d;goto fetch_next; // LD B,D T:0
        case   67: cpu->b=cpu->e;goto fetch_next; // LD B,E T:0
        case   68: cpu->b=cpu->hlx[cpu->hlx_idx].h;goto fetch_next; // LD B,H T:0
        case   69: cpu->b=cpu->hlx[cpu->hlx_idx].l;goto fetch_next; // LD B,L T:0
        case   70: cpu->step=724;goto step_to; // LD B,(HL) T:0
        case   71: cpu->b=cpu->a;goto fetch_next; // LD B,A T:0
        case   72: cpu->c=cpu->b;goto fetch_next; // LD C,B T:0
        case   73: cpu->c=cpu->c;goto fetch_next; // LD C,C T:0
        case   74: cpu->c=cpu->d;goto fetch_next; // LD C,D T:0
        case   75: cpu->c=cpu->e;goto fetch_next; // LD C,E T:0
        case   76: cpu->c=cpu->hlx[cpu->hlx_idx].h;goto fetch_next; // LD C,H T:0
        case   77: cpu->c=cpu->hlx[cpu->hlx_idx].l;goto fetch_next; // LD C,L T:0
        case   78: cpu->step=727;goto step_to; // LD C,(HL) T:0
        case   79: cpu->c=cpu->a;goto fetch_next; // LD C,A T:0
        case   80: cpu->d=cpu->b;goto fetch_next; // LD D,B T:0
        case   81: cpu->d=cpu->c;goto fetch_next; // LD D,C T:0
        case   82: cpu->d=cpu->d;goto fetch_next; // LD D,D T:0
        case   83: cpu->d=cpu->e;goto fetch_next; // LD D,E T:0
        case   84: cpu->d=cpu->hlx[cpu->hlx_idx].h;goto fetch_next; // LD D,H T:0
        case   85: cpu->d=cpu->hlx[cpu->hlx_idx].l;goto fetch_next; // LD D,L T:0
        case   86: cpu->step=730;goto step_to; // LD D,(HL) T:0
        case   87: cpu->d=cpu->a;goto fetch_next; // LD D,A T:0
        case   88: cpu->e=cpu->b;goto fetch_next; // LD E,B T:0
        case   89: cpu->e=cpu->c;goto fetch_next; // LD E,C T:0
        case   90: cpu->e=cpu->d;goto fetch_next; // LD E,D T:0
        case   91: cpu->e=cpu->e;goto fetch_next; // LD E,E T:0
        case   92: cpu->e=cpu->hlx[cpu->hlx_idx].h;goto fetch_next; // LD E,H T:0
        case   93: cpu->e=cpu->hlx[cpu->hlx_idx].l;goto fetch_next; // LD E,L T:0
        case   94: cpu->step=733;goto step_to; // LD E,(HL) T:0
        case   95: cpu->e=cpu->a;goto fetch_next; // LD E,A T:0
        case   96: cpu->hlx[cpu->hlx_idx].h=cpu->b;goto fetch_next; // LD H,B T:0
        case   97: cpu->hlx[cpu->hlx_idx].h=cpu->c;goto fetch_next; // LD H,C T:0
        case   98: cpu->hlx[cpu->hlx_idx].h=cpu->d;goto fetch_next; // LD H,D T:0
        case   99: cpu->hlx[cpu->hlx_idx].h=cpu->e;goto fetch_next; // LD H,E T:0
        case  100: cpu->hlx[cpu->hlx_idx].h=cpu->hlx[cpu->hlx_idx].h;goto fetch_next; // LD H,H T:0
        case  101: cpu->hlx[cpu->hlx_idx].h=cpu->hlx[cpu->hlx_idx].l;goto fetch_next; // LD H,L T:0
        case  102: cpu->step=736;goto step_to; // LD H,(HL) T:0
        case  103: cpu->hlx[cpu->hlx_idx].h=cpu->a;goto fetch_next; // LD H,A T:0
        case  104: cpu->hlx[cpu->hlx_idx].l=cpu->b;goto fetch_next; // LD L,B T:0
        case  105: cpu->hlx[cpu->hlx_idx].l=cpu->c;goto fetch_next; // LD L,C T:0
        case  106: cpu->hlx[cpu->hlx_idx].l=cpu->d;goto fetch_next; // LD L,D T:0
        case  107: cpu->hlx[cpu->hlx_idx].l=cpu->e;goto fetch_next; // LD L,E T:0
        case  108: cpu->hlx[cpu->hlx_idx].l=cpu->hlx[cpu->hlx_idx].h;goto fetch_next; // LD L,H T:0
        case  109: cpu->hlx[cpu->hlx_idx].l=cpu->hlx[cpu->hlx_idx].l;goto fetch_next; // LD L,L T:0
        case  110: cpu->step=739;goto step_to; // LD L,(HL) T:0
        case  111: cpu->hlx[cpu->hlx_idx].l=cpu->a;goto fetch_next; // LD L,A T:0
        case  112: cpu->step=742;goto step_to; // LD (HL),B T:0
        case  113: cpu->step=745;goto step_to; // LD (HL),C T:0
        case  114: cpu->step=748;goto step_to; // LD (HL),D T:0
        case  115: cpu->step=751;goto step_to; // LD (HL),E T:0
        case  116: cpu->step=754;goto step_to; // LD (HL),H T:0
        case  117: cpu->step=757;goto step_to; // LD (HL),L T:0
        case  118: pins=_z80_halt(cpu,pins);goto fetch_next; // HALT T:0
        case  119: cpu->step=760;goto step_to; // LD (HL),A T:0
        case  120: cpu->a=cpu->b;goto fetch_next; // LD A,B T:0
        case  121: cpu->a=cpu->c;goto fetch_next; // LD A,C T:0
        case  122: cpu->a=cpu->d;goto fetch_next; // LD A,D T:0
        case  123: cpu->a=cpu->e;goto fetch_next; // LD A,E T:0
        case  124: cpu->a=cpu->hlx[cpu->hlx_idx].h;goto fetch_next; // LD A,H T:0
        case  125: cpu->a=cpu->hlx[cpu->hlx_idx].l;goto fetch_next; // LD A,L T:0
        case  126: cpu->step=763;goto step_to; // LD A,(HL) T:0
        case  127: cpu->a=cpu->a;goto fetch_next; // LD A,A T:0
        case  128: _z80_add8(cpu,cpu->b);goto fetch_next; // ADD B T:0
        case  129: _z80_add8(cpu,cpu->c);goto fetch_next; // ADD C T:0
        case  130: _z80_add8(cpu,cpu->d);goto fetch_next; // ADD D T:0
        case  131: _z80_add8(cpu,cpu->e);goto fetch_next; // ADD E T:0
        case  132: _z80_add8(cpu,cpu->hlx[cpu->hlx_idx].h);goto fetch_next; // ADD H T:0
        case  133: _z80_add8(cpu,cpu->hlx[cpu->hlx_idx].l);goto fetch_next; // ADD L T:0
        case  134: cpu->step=766;goto step_to; // ADD (HL) T:0
        case  135: _z80_add8(cpu,cpu->a);goto fetch_next; // ADD A T:0
        case  136: _z80_adc8(cpu,cpu->b);goto fetch_next; // ADC B T:0
        case  137: _z80_adc8(cpu,cpu->c);goto fetch_next; // ADC C T:0
        case  138: _z80_adc8(cpu,cpu->d);goto fetch_next; // ADC D T:0
        case  139: _z80_adc8(cpu,cpu->e);goto fetch_next; // ADC E T:0
        case  140: _z80_adc8(cpu,cpu->hlx[cpu->hlx_idx].h);goto fetch_next; // ADC H T:0
        case  141: _z80_adc8(cpu,cpu->hlx[cpu->hlx_idx].l);goto fetch_next; // ADC L T:0
        case  142: cpu->step=769;goto step_to; // ADC (HL) T:0
        case  143: _z80_adc8(cpu,cpu->a);goto fetch_next; // ADC A T:0
        case  144: _z80_sub8(cpu,cpu->b);goto fetch_next; // SUB B T:0
        case  145: _z80_sub8(cpu,cpu->c);goto fetch_next; // SUB C T:0
        case  146: _z80_sub8(cpu,cpu->d);goto fetch_next; // SUB D T:0
        case  147: _z80_sub8(cpu,cpu->e);goto fetch_next; // SUB E T:0
        case  148: _z80_sub8(cpu,cpu->hlx[cpu->hlx_idx].h);goto fetch_next; // SUB H T:0
        case  149: _z80_sub8(cpu,cpu->hlx[cpu->hlx_idx].l);goto fetch_next; // SUB L T:0
        case  150: cpu->step=772;goto step_to; // SUB (HL) T:0
        case  151: _z80_sub8(cpu,cpu->a);goto fetch_next; // SUB A T:0
        case  152: _z80_sbc8(cpu,cpu->b);goto fetch_next; // SBC B T:0
        case  153: _z80_sbc8(cpu,cpu->c);goto fetch_next; // SBC C T:0
        case  154: _z80_sbc8(cpu,cpu->d);goto fetch_next; // SBC D T:0
        case  155: _z80_sbc8(cpu,cpu->e);goto fetch_next; // SBC E T:0
        case  156: _z80_sbc8(cpu,cpu->hlx[cpu->hlx_idx].h);goto fetch_next; // SBC H T:0
        case  157: _z80_sbc8(cpu,cpu->hlx[cpu->hlx_idx].l);goto fetch_next; // SBC L T:0
        case  158: cpu->step=775;goto step_to; // SBC (HL) T:0
        case  159: _z80_sbc8(cpu,cpu->a);goto fetch_next; // SBC A T:0
        case  160: _z80_and8(cpu,cpu->b);goto fetch_next; // AND B T:0
        case  161: _z80_and8(cpu,cpu->c);goto fetch_next; // AND C T:0
        case  162: _z80_and8(cpu,cpu->d);goto fetch_next; // AND D T:0
        case  163: _z80_and8(cpu,cpu->e);goto fetch_next; // AND E T:0
        case  164: _z80_and8(cpu,cpu->hlx[cpu->hlx_idx].h);goto fetch_next; // AND H T:0
        case  165: _z80_and8(cpu,cpu->hlx[cpu->hlx_idx].l);goto fetch_next; // AND L T:0
        case  166: cpu->step=778;goto step_to; // AND (HL) T:0
        case  167: _z80_and8(cpu,cpu->a);goto fetch_next; // AND A T:0
        case  168: _z80_xor8(cpu,cpu->b);goto fetch_next; // XOR B T:0
        case  169: _z80_xor8(cpu,cpu->c);goto fetch_next; // XOR C T:0
        case  170: _z80_xor8(cpu,cpu->d);goto fetch_next; // XOR D T:0
        case  171: _z80_xor8(cpu,cpu->e);goto fetch_next; // XOR E T:0
        case  172: _z80_xor8(cpu,cpu->hlx[cpu->hlx_idx].h);goto fetch_next; // XOR H T:0
        case  173: _z80_xor8(cpu,cpu->hlx[cpu->hlx_idx].l);goto fetch_next; // XOR L T:0
        case  174: cpu->step=781;goto step_to; // XOR (HL) T:0
        case  175: _z80_xor8(cpu,cpu->a);goto fetch_next; // XOR A T:0
        case  176: _z80_or8(cpu,cpu->b);goto fetch_next; // OR B T:0
        case  177: _z80_or8(cpu,cpu->c);goto fetch_next; // OR C T:0
        case  178: _z80_or8(cpu,cpu->d);goto fetch_next; // OR D T:0
        case  179: _z80_or8(cpu,cpu->e);goto fetch_next; // OR E T:0
        case  180: _z80_or8(cpu,cpu->hlx[cpu->hlx_idx].h);goto fetch_next; // OR H T:0
        case  181: _z80_or8(cpu,cpu->hlx[cpu->hlx_idx].l);goto fetch_next; // OR L T:0
        case  182: cpu->step=784;goto step_to; // OR (HL) T:0
        case  183: _z80_or8(cpu,cpu->a);goto fetch_next; // OR A T:0
        case  184: _z80_cp8(cpu,cpu->b);goto fetch_next; // CP B T:0
        case  185: _z80_cp8(cpu,cpu->c);goto fetch_next; // CP C T:0
        case  186: _z80_cp8(cpu,cpu->d);goto fetch_next; // CP D T:0
        case  187: _z80_cp8(cpu,cpu->e);goto fetch_next; // CP E T:0
        case  188: _z80_cp8(cpu,cpu->hlx[cpu->hlx_idx].h);goto fetch_next; // CP H T:0
        case  189: _z80_cp8(cpu,cpu->hlx[cpu->hlx_idx].l);goto fetch_next; // CP L T:0
        case  190: cpu->step=787;goto step_to; // CP (HL) T:0
        case  191: _z80_cp8(cpu,cpu->a);goto fetch_next; // CP A T:0
        case  192: if(!_cc_nz){_skip(6);};cpu->step=790;goto step_to; // RET NZ T:0
        case  193: cpu->step=797;goto step_to; // POP BC T:0
        case  194: cpu->step=803;goto step_to; // JP NZ,nn T:0
        case  195: cpu->step=809;goto step_to; // JP nn T:0
        case  196: cpu->step=815;goto step_to; // CALL NZ,nn T:0
        case  197: cpu->step=828;goto step_to; // PUSH BC T:0
        case  198: cpu->step=835;goto step_to; // ADD n T:0
        case  199: cpu->step=838;goto step_to; // RST 0h T:0
        case  200: if(!_cc_z){_skip(6);};cpu->step=845;goto step_to; // RET Z T:0
        case  201: cpu->step=852;goto step_to; // RET T:0
        case  202: cpu->step=858;goto step_to; // JP Z,nn T:0
        case  203: _fetch_cb();goto step_to; // CB prefix
        case  204: cpu->step=864;goto step_to; // CALL Z,nn T:0
        case  205: cpu->step=877;goto step_to; // CALL nn T:0
        case  206: cpu->step=890;goto step_to; // ADC n T:0
        case  207: cpu->step=893;goto step_to; // RST 8h T:0
        case  208: if(!_cc_nc){_skip(6);};cpu->step=900;goto step_to; // RET NC T:0
        case  209: cpu->step=907;goto step_to; // POP DE T:0
        case  210: cpu->step=913;goto step_to; // JP NC,nn T:0
        case  211: cpu->step=919;goto step_to; // OUT (n),A T:0
        case  212: cpu->step=926;goto step_to; // CALL NC,nn T:0
        case  213: cpu->step=939;goto step_to; // PUSH DE T:0
        case  214: cpu->step=946;goto step_to; // SUB n T:0
        case  215: cpu->step=949;goto step_to; // RST 10h T:0
        case  216: if(!_cc_c){_skip(6);};cpu->step=956;goto step_to; // RET C T:0
        case  217: _z80_exx(cpu);goto fetch_next; // EXX T:0
        case  218: cpu->step=963;goto step_to; // JP C,nn T:0
        case  219: cpu->step=969;goto step_to; // IN A,(n) T:0
        case  220: cpu->step=976;goto step_to; // CALL C,nn T:0
        case  221: _fetch_dd();goto step_to; // DD prefix
        case  222: cpu->step=989;goto step_to; // SBC n T:0
        case  223: cpu->step=992;goto step_to; // RST 18h T:0
        case  224: if(!_cc_po){_skip(6);};cpu->step=999;goto step_to; // RET PO T:0
        case  225: cpu->step=1006;goto step_to; // POP HL T:0
        case  226: cpu->step=1012;goto step_to; // JP PO,nn T:0
        case  227: cpu->step=1018;goto step_to; // EX (SP),HL T:0
        case  228: cpu->step=1033;goto step_to; // CALL PO,nn T:0
        case  229: cpu->step=1046;goto step_to; // PUSH HL T:0
        case  230: cpu->step=1053;goto step_to; // AND n T:0
        case  231: cpu->step=1056;goto step_to; // RST 20h T:0
        case  232: if(!_cc_pe){_skip(6);};cpu->step=1063;goto step_to; // RET PE T:0
        case  233: cpu->pc=cpu->hlx[cpu->hlx_idx].hl;goto fetch_next; // JP HL T:0
        case  234: cpu->step=1070;goto step_to; // JP PE,nn T:0
        case  235: _z80_ex_de_hl(cpu);goto fetch_next; // EX DE,HL T:0
        case  236: cpu->step=1076;goto step_to; // CALL PE,nn T:0
        case  237: _fetch_ed();goto step_to; // ED prefix
        case  238: cpu->step=1089;goto step_to; // XOR n T:0
        case  239: cpu->step=1092;goto step_to; // RST 28h T:0
        case  240: if(!_cc_p){_skip(6);};cpu->step=1099;goto step_to; // RET P T:0
        case  241: cpu->step=1106;goto step_to; // POP AF T:0
        case  242: cpu->step=1112;goto step_to; // JP P,nn T:0
        case  243: cpu->iff1=cpu->iff2=false;goto fetch_next; // DI T:0
        case  244: cpu->step=1118;goto step_to; // CALL P,nn T:0
        case  245: cpu->step=1131;goto step_to; // PUSH AF T:0
        case  246: cpu->step=1138;goto step_to; // OR n T:0
        case  247: cpu->step=1141;goto step_to; // RST 30h T:0
        case  248: if(!_cc_m){_skip(6);};cpu->step=1148;goto step_to; // RET M T:0
        case  249: cpu->sp=cpu->hlx[cpu->hlx_idx].hl;cpu->step=1155;goto step_to; // LD SP,HL T:0
        case  250: cpu->step=1157;goto step_to; // JP M,nn T:0
        case  251: cpu->iff1=cpu->iff2=false;pins=_z80_fetch(cpu,pins);cpu->iff1=cpu->iff2=true;goto step_to; // EI
        case  252: cpu->step=1163;goto step_to; // CALL M,nn T:0
        case  253: _fetch_fd();goto step_to; // FD prefix
        case  254: cpu->step=1176;goto step_to; // CP n T:0
        case  255: cpu->step=1179;goto step_to; // RST 38h T:0
        case  256: goto fetch_next; // ED NOP T:0
        case  257: goto fetch_next; // ED NOP T:0
        case  258: goto fetch_next; // ED NOP T:0
        case  259: goto fetch_next; // ED NOP T:0
        case  260: goto fetch_next; // ED NOP T:0
        case  261: goto fetch_next; // ED NOP T:0
        case  262: goto fetch_next; // ED NOP T:0
        case  263: goto fetch_next; // ED NOP T:0
        case  264: goto fetch_next; // ED NOP T:0
        case  265: goto fetch_next; // ED NOP T:0
        case  266: goto fetch_next; // ED NOP T:0
        case  267: goto fetch_next; // ED NOP T:0
        case  268: goto fetch_next; // ED NOP T:0
        case  269: goto fetch_next; // ED NOP T:0
        case  270: goto fetch_next; // ED NOP T:0
        case  271: goto fetch_next; // ED NOP T:0
        case  272: goto fetch_next; // ED NOP T:0
        case  273: goto fetch_next; // ED NOP T:0
        case  274: goto fetch_next; // ED NOP T:0
        case  275: goto fetch_next; // ED NOP T:0
        case  276: goto fetch_next; // ED NOP T:0
        case  277: goto fetch_next; // ED NOP T:0
        case  278: goto fetch_next; // ED NOP T:0
        case  279: goto fetch_next; // ED NOP T:0
        case  280: goto fetch_next; // ED NOP T:0
        case  281: goto fetch_next; // ED NOP T:0
        case  282: goto fetch_next; // ED NOP T:0
        case  283: goto fetch_next; // ED NOP T:0
        case  284: goto fetch_next; // ED NOP T:0
        case  285: goto fetch_next; // ED NOP T:0
        case  286: goto fetch_next; // ED NOP T:0
        case  287: goto fetch_next; // ED NOP T:0
        case  288: goto fetch_next; // ED NOP T:0
        case  289: goto fetch_next; // ED NOP T:0
        case  290: goto fetch_next; // ED NOP T:0
        case  291: goto fetch_next; // ED NOP T:0
        case  292: goto fetch_next; // ED NOP T:0
        case  293: goto fetch_next; // ED NOP T:0
        case  294: goto fetch_next; // ED NOP T:0
        case  295: goto fetch_next; // ED NOP T:0
        case  296: goto fetch_next; // ED NOP T:0
        case  297: goto fetch_next; // ED NOP T:0
        case  298: goto fetch_next; // ED NOP T:0
        case  299: goto fetch_next; // ED NOP T:0
        case  300: goto fetch_next; // ED NOP T:0
        case  301: goto fetch_next; // ED NOP T:0
        case  302: goto fetch_next; // ED NOP T:0
        case  303: goto fetch_next; // ED NOP T:0
        case  304: goto fetch_next; // ED NOP T:0
        case  305: goto fetch_next; // ED NOP T:0
        case  306: goto fetch_next; // ED NOP T:0
        case  307: goto fetch_next; // ED NOP T:0
        case  308: goto fetch_next; // ED NOP T:0
        case  309: goto fetch_next; // ED NOP T:0
        case  310: goto fetch_next; // ED NOP T:0
        case  311: goto fetch_next; // ED NOP T:0
        case  312: goto fetch_next; // ED NOP T:0
        case  313: goto fetch_next; // ED NOP T:0
        case  314: goto fetch_next; // ED NOP T:0
        case  315: goto fetch_next; // ED NOP T:0
        case  316: goto fetch_next; // ED NOP T:0
        case  317: goto fetch_next; // ED NOP T:0
        case  318: goto fetch_next; // ED NOP T:0
        case  319: goto fetch_next; // ED NOP T:0
        case  320: cpu->step=1186;goto step_to; // IN B,(C) T:0
        case  321: cpu->step=1190;goto step_to; // OUT (C),B T:0
        case  322: _z80_sbc16(cpu,cpu->bc);cpu->step=1194;goto step_to; // SBC HL,BC T:0
        case  323: cpu->step=1201;goto step_to; // LD (nn),BC T:0
        case  324: _z80_neg8(cpu);goto fetch_next; // NEG T:0
        case  325: cpu->step=1213;goto step_to; // RETN T:0
        case  326: pins=_z80_fetch(cpu,pins);cpu->iff1=cpu->iff2;goto step_to; // RETN
        case  327: cpu->im=0;goto fetch_next; // IM 0 T:0
        case  328: cpu->step=1218;goto step_to; // LD I,A T:0
        case  329: cpu->step=1219;goto step_to; // IN C,(C) T:0
        case  330: cpu->step=1223;goto step_to; // OUT (C),C T:0
        case  331: _z80_adc16(cpu,cpu->bc);cpu->step=1227;goto step_to; // ADC HL,BC T:0
        case  332: cpu->step=1234;goto step_to; // LD BC,(nn) T:0
        case  333: _z80_neg8(cpu);goto fetch_next; // NEG T:0
        case  334: cpu->step=1246;goto step_to; // RETI T:0
        case  335: pins=_z80_fetch(cpu,pins);cpu->iff1=cpu->iff2;goto step_to; // RETI
        case  336: cpu->im=0;goto fetch_next; // IM 0 T:0
        case  337: cpu->step=1251;goto step_to; // LD R,A T:0
        case  338: cpu->step=1252;goto step_to; // IN D,(C) T:0
        case  339: cpu->step=1256;goto step_to; // OUT (C),D T:0
        case  340: _z80_sbc16(cpu,cpu->de);cpu->step=1260;goto step_to; // SBC HL,DE T:0
        case  341: cpu->step=1267;goto step_to; // LD (nn),DE T:0
        case  342: _z80_neg8(cpu);goto fetch_next; // NEG T:0
        case  343: cpu->step=1246;goto step_to; // RETI T:0
        case  344: pins=_z80_fetch(cpu,pins);cpu->iff1=cpu->iff2;goto step_to; // RETI
        case  345: cpu->im=1;goto fetch_next; // IM 1 T:0
        case  346: cpu->step=1279;goto step_to; // LD A,I T:0
        case  347: cpu->step=1280;goto step_to; // IN E,(C) T:0
        case  348: cpu->step=1284;goto step_to; // OUT (C),E T:0
        case  349: _z80_adc16(cpu,cpu->de);cpu->step=1288;goto step_to; // ADC HL,DE T:0
        case  350: cpu->step=1295;goto step_to; // LD DE,(nn) T:0
        case  351: _z80_neg8(cpu);goto fetch_next; // NEG T:0
        case  352: cpu->step=1246;goto step_to; // RETI T:0
        case  353: pins=_z80_fetch(cpu,pins);cpu->iff1=cpu->iff2;goto step_to; // RETI
        case  354: cpu->im=2;goto fetch_next; // IM 2 T:0
        case  355: cpu->step=1307;goto step_to; // LD A,R T:0
        case  356: cpu->step=1308;goto step_to; // IN H,(C) T:0
        case  357: cpu->step=1312;goto step_to; // OUT (C),H T:0
        case  358: _z80_sbc16(cpu,cpu->hl);cpu->step=1316;goto step_to; // SBC HL,HL T:0
        case  359: cpu->step=1323;goto step_to; // LD (nn),HL T:0
        case  360: _z80_neg8(cpu);goto fetch_next; // NEG T:0
        case  361: cpu->step=1246;goto step_to; // RETI T:0
        case  362: pins=_z80_fetch(cpu,pins);cpu->iff1=cpu->iff2;goto step_to; // RETI
        case  363: cpu->im=0;goto fetch_next; // IM 0 T:0
        case  364: cpu->step=1335;goto step_to; // RRD T:0
        case  365: cpu->step=1345;goto step_to; // IN L,(C) T:0
        case  366: cpu->step=1349;goto step_to; // OUT (C),L T:0
        case  367: _z80_adc16(cpu,cpu->hl);cpu->step=1353;goto step_to; // ADC HL,HL T:0
        case  368: cpu->step=1360;goto step_to; // LD HL,(nn) T:0
        case  369: _z80_neg8(cpu);goto fetch_next; // NEG T:0
        case  370: cpu->step=1246;goto step_to; // RETI T:0
        case  371: pins=_z80_fetch(cpu,pins);cpu->iff1=cpu->iff2;goto step_to; // RETI
        case  372: cpu->im=0;goto fetch_next; // IM 0 T:0
        case  373: cpu->step=1372;goto step_to; // RLD T:0
        case  374: cpu->step=1382;goto step_to; // IN (C) T:0
        case  375: cpu->step=1386;goto step_to; // OUT (C),0 T:0
        case  376: _z80_sbc16(cpu,cpu->sp);cpu->step=1390;goto step_to; // SBC HL,SP T:0
        case  377: cpu->step=1397;goto step_to; // LD (nn),SP T:0
        case  378: _z80_neg8(cpu);goto fetch_next; // NEG T:0
        case  379: cpu->step=1246;goto step_to; // RETI T:0
        case  380: pins=_z80_fetch(cpu,pins);cpu->iff1=cpu->iff2;goto step_to; // RETI
        case  381: cpu->im=1;goto fetch_next; // IM 1 T:0
        case  382: goto fetch_next; // ED NOP T:0
        case  383: cpu->step=1409;goto step_to; // IN A,(C) T:0
        case  384: cpu->step=1413;goto step_to; // OUT (C),A T:0
        case  385: _z80_adc16(cpu,cpu->sp);cpu->step=1417;goto step_to; // ADC HL,SP T:0
        case  386: cpu->step=1424;goto step_to; // LD SP,(nn) T:0
        case  387: _z80_neg8(cpu);goto fetch_next; // NEG T:0
        case  388: cpu->step=1246;goto step_to; // RETI T:0
        case  389: pins=_z80_fetch(cpu,pins);cpu->iff1=cpu->iff2;goto step_to; // RETI
        case  390: cpu->im=2;goto fetch_next; // IM 2 T:0
        case  391: goto fetch_next; // ED NOP T:0
        case  392: goto fetch_next; // ED NOP T:0
        case  393: goto fetch_next; // ED NOP T:0
        case  394: goto fetch_next; // ED NOP T:0
        case  395: goto fetch_next; // ED NOP T:0
        case  396: goto fetch_next; // ED NOP T:0
        case  397: goto fetch_next; // ED NOP T:0
        case  398: goto fetch_next; // ED NOP T:0
        case  399: goto fetch_next; // ED NOP T:0
        case  400: goto fetch_next; // ED NOP T:0
        case  401: goto fetch_next; // ED NOP T:0
        case  402: goto fetch_next; // ED NOP T:0
        case  403: goto fetch_next; // ED NOP T:0
        case  404: goto fetch_next; // ED NOP T:0
        case  405: goto fetch_next; // ED NOP T:0
        case  406: goto fetch_next; // ED NOP T:0
        case  407: goto fetch_next; // ED NOP T:0
        case  408: goto fetch_next; // ED NOP T:0
        case  409: goto fetch_next; // ED NOP T:0
        case  410: goto fetch_next; // ED NOP T:0
        case  411: goto fetch_next; // ED NOP T:0
        case  412: goto fetch_next; // ED NOP T:0
        case  413: goto fetch_next; // ED NOP T:0
        case  414: goto fetch_next; // ED NOP T:0
        case  415: goto fetch_next; // ED NOP T:0
        case  416: goto fetch_next; // ED NOP T:0
        case  417: goto fetch_next; // ED NOP T:0
        case  418: goto fetch_next; // ED NOP T:0
        case  419: goto fetch_next; // ED NOP T:0
        case  420: goto fetch_next; // ED NOP T:0
        case  421: goto fetch_next; // ED NOP T:0
        case  422: goto fetch_next; // ED NOP T:0
        case  423: goto fetch_next; // ED NOP T:0
        case  424: cpu->step=1436;goto step_to; // LDI T:0
        case  425: cpu->step=1444;goto step_to; // CPI T:0
        case  426: cpu->step=1452;goto step_to; // INI T:0
        case  427: cpu->step=1460;goto step_to; // OUTI T:0
        case  428: goto fetch_next; // ED NOP T:0
        case  429: goto fetch_next; // ED NOP T:0
        case  430: goto fetch_next; // ED NOP T:0
        case  431: goto fetch_next; // ED NOP T:0
        case  432: cpu->step=1468;goto step_to; // LDD T:0
        case  433: cpu->step=1476;goto step_to; // CPD T:0
        case  434: cpu->step=1484;goto step_to; // IND T:0
        case  435: cpu->step=1492;goto step_to; // OUTD T:0
        case  436: goto fetch_next; // ED NOP T:0
        case  437: goto fetch_next; // ED NOP T:0
        case  438: goto fetch_next; // ED NOP T:0
        case  439: goto fetch_next; // ED NOP T:0
        case  440: cpu->step=1500;goto step_to; // LDIR T:0
        case  441: cpu->step=1513;goto step_to; // CPIR T:0
        case  442: cpu->step=1526;goto step_to; // INIR T:0
        case  443: cpu->step=1539;goto step_to; // OTIR T:0
        case  444: goto fetch_next; // ED NOP T:0
        case  445: goto fetch_next; // ED NOP T:0
        case  446: goto fetch_next; // ED NOP T:0
        case  447: goto fetch_next; // ED NOP T:0
        case  448: cpu->step=1552;goto step_to; // LDDR T:0
        case  449: cpu->step=1565;goto step_to; // CPDR T:0
        case  450: cpu->step=1578;goto step_to; // INDR T:0
        case  451: cpu->step=1591;goto step_to; // OTDR T:0
        case  452: goto fetch_next; // ED NOP T:0
        case  453: goto fetch_next; // ED NOP T:0
        case  454: goto fetch_next; // ED NOP T:0
        case  455: goto fetch_next; // ED NOP T:0
        case  456: goto fetch_next; // ED NOP T:0
        case  457: goto fetch_next; // ED NOP T:0
        case  458: goto fetch_next; // ED NOP T:0
        case  459: goto fetch_next; // ED NOP T:0
        case  460: goto fetch_next; // ED NOP T:0
        case  461: goto fetch_next; // ED NOP T:0
        case  462: goto fetch_next; // ED NOP T:0
        case  463: goto fetch_next; // ED NOP T:0
        case  464: goto fetch_next; // ED NOP T:0
        case  465: goto fetch_next; // ED NOP T:0
        case  466: goto fetch_next; // ED NOP T:0
        case  467: goto fetch_next; // ED NOP T:0
        case  468: goto fetch_next; // ED NOP T:0
        case  469: goto fetch_next; // ED NOP T:0
        case  470: goto fetch_next; // ED NOP T:0
        case  471: goto fetch_next; // ED NOP T:0
        case  472: goto fetch_next; // ED NOP T:0
        case  473: goto fetch_next; // ED NOP T:0
        case  474: goto fetch_next; // ED NOP T:0
        case  475: goto fetch_next; // ED NOP T:0
        case  476: goto fetch_next; // ED NOP T:0
        case  477: goto fetch_next; // ED NOP T:0
        case  478: goto fetch_next; // ED NOP T:0
        case  479: goto fetch_next; // ED NOP T:0
        case  480: goto fetch_next; // ED NOP T:0
        case  481: goto fetch_next; // ED NOP T:0
        case  482: goto fetch_next; // ED NOP T:0
        case  483: goto fetch_next; // ED NOP T:0
        case  484: goto fetch_next; // ED NOP T:0
        case  485: goto fetch_next; // ED NOP T:0
        case  486: goto fetch_next; // ED NOP T:0
        case  487: goto fetch_next; // ED NOP T:0
        case  488: goto fetch_next; // ED NOP T:0
        case  489: goto fetch_next; // ED NOP T:0
        case  490: goto fetch_next; // ED NOP T:0
        case  491: goto fetch_next; // ED NOP T:0
        case  492: goto fetch_next; // ED NOP T:0
        case  493: goto fetch_next; // ED NOP T:0
        case  494: goto fetch_next; // ED NOP T:0
        case  495: goto fetch_next; // ED NOP T:0
        case  496: goto fetch_next; // ED NOP T:0
        case  497: goto fetch_next; // ED NOP T:0
        case  498: goto fetch_next; // ED NOP T:0
        case  499: goto fetch_next; // ED NOP T:0
        case  500: goto fetch_next; // ED NOP T:0
        case  501: goto fetch_next; // ED NOP T:0
        case  502: goto fetch_next; // ED NOP T:0
        case  503: goto fetch_next; // ED NOP T:0
        case  504: goto fetch_next; // ED NOP T:0
        case  505: goto fetch_next; // ED NOP T:0
        case  506: goto fetch_next; // ED NOP T:0
        case  507: goto fetch_next; // ED NOP T:0
        case  508: goto fetch_next; // ED NOP T:0
        case  509: goto fetch_next; // ED NOP T:0
        case  510: goto fetch_next; // ED NOP T:0
        case  511: goto fetch_next; // ED NOP T:0
        case  512: goto fetch_next; // ED NOP T:0
        case  513: goto fetch_next; // ED NOP T:0
        case  514: goto fetch_next; // ED NOP T:0
        case  515: goto fetch_next; // ED NOP T:0
        case  516: goto fetch_next; // ED NOP T:0
        case  517: goto fetch_next; // ED NOP T:0
        case  518: goto fetch_next; // ED NOP T:0
        case  519: goto fetch_next; // ED NOP T:0
        case  512: _wait();_mread(cpu->pc++);goto step_next; // LD BC,nn T:1
        case  513: cpu->c=_gd();goto step_next; // LD BC,nn T:2
        case  514: goto step_next; // LD BC,nn T:3
        case  515: _wait();_mread(cpu->pc++);goto step_next; // LD BC,nn T:4
        case  516: cpu->b=_gd();goto step_next; // LD BC,nn T:5
        case  517: goto fetch_next; // LD BC,nn T:6
        case  518: _wait();_mwrite(cpu->bc,cpu->a);cpu->wzl=cpu->c+1;cpu->wzh=cpu->a;goto step_next; // LD (BC),A T:1
        case  519: goto step_next; // LD (BC),A T:2
        case  520: goto fetch_next; // LD (BC),A T:3
        case  521: goto step_next; // INC BC T:1
        case  522: goto fetch_next; // INC BC T:2
        case  523: _wait();_mread(cpu->pc++);goto step_next; // LD B,n T:1
        case  524: cpu->b=_gd();goto step_next; // LD B,n T:2
        case  525: goto fetch_next; // LD B,n T:3
        case  526: goto step_next; // ADD HL,BC T:1
        case  527: goto step_next; // ADD HL,BC T:2
        case  528: goto step_next; // ADD HL,BC T:3
        case  529: goto step_next; // ADD HL,BC T:4
        case  530: goto step_next; // ADD HL,BC T:5
        case  531: goto step_next; // ADD HL,BC T:6
        case  532: goto fetch_next; // ADD HL,BC T:7
        case  533: _wait();_mread(cpu->bc);goto step_next; // LD A,(BC) T:1
        case  534: cpu->a=_gd();cpu->wz=cpu->bc+1;goto step_next; // LD A,(BC) T:2
        case  535: goto fetch_next; // LD A,(BC) T:3
        case  536: goto step_next; // DEC BC T:1
        case  537: goto fetch_next; // DEC BC T:2
        case  538: _wait();_mread(cpu->pc++);goto step_next; // LD C,n T:1
        case  539: cpu->c=_gd();goto step_next; // LD C,n T:2
        case  540: goto fetch_next; // LD C,n T:3
        case  541: goto step_next; // DJNZ d T:1
        case  542: _wait();_mread(cpu->pc++);goto step_next; // DJNZ d T:2
        case  543: cpu->dlatch=_gd();if(--cpu->b==0){_skip(5);};goto step_next; // DJNZ d T:3
        case  544: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;goto step_next; // DJNZ d T:4
        case  545: goto step_next; // DJNZ d T:5
        case  546: goto step_next; // DJNZ d T:6
        case  547: goto step_next; // DJNZ d T:7
        case  548: goto step_next; // DJNZ d T:8
        case  549: goto fetch_next; // DJNZ d T:9
        case  550: _wait();_mread(cpu->pc++);goto step_next; // LD DE,nn T:1
        case  551: cpu->e=_gd();goto step_next; // LD DE,nn T:2
        case  552: goto step_next; // LD DE,nn T:3
        case  553: _wait();_mread(cpu->pc++);goto step_next; // LD DE,nn T:4
        case  554: cpu->d=_gd();goto step_next; // LD DE,nn T:5
        case  555: goto fetch_next; // LD DE,nn T:6
        case  556: _wait();_mwrite(cpu->de,cpu->a);cpu->wzl=cpu->e+1;cpu->wzh=cpu->a;goto step_next; // LD (DE),A T:1
        case  557: goto step_next; // LD (DE),A T:2
        case  558: goto fetch_next; // LD (DE),A T:3
        case  559: goto step_next; // INC DE T:1
        case  560: goto fetch_next; // INC DE T:2
        case  561: _wait();_mread(cpu->pc++);goto step_next; // LD D,n T:1
        case  562: cpu->d=_gd();goto step_next; // LD D,n T:2
        case  563: goto fetch_next; // LD D,n T:3
        case  564: _wait();_mread(cpu->pc++);goto step_next; // JR d T:1
        case  565: cpu->dlatch=_gd();goto step_next; // JR d T:2
        case  566: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;goto step_next; // JR d T:3
        case  567: goto step_next; // JR d T:4
        case  568: goto step_next; // JR d T:5
        case  569: goto step_next; // JR d T:6
        case  570: goto step_next; // JR d T:7
        case  571: goto fetch_next; // JR d T:8
        case  572: goto step_next; // ADD HL,DE T:1
        case  573: goto step_next; // ADD HL,DE T:2
        case  574: goto step_next; // ADD HL,DE T:3
        case  575: goto step_next; // ADD HL,DE T:4
        case  576: goto step_next; // ADD HL,DE T:5
        case  577: goto step_next; // ADD HL,DE T:6
        case  578: goto fetch_next; // ADD HL,DE T:7
        case  579: _wait();_mread(cpu->de);goto step_next; // LD A,(DE) T:1
        case  580: cpu->a=_gd();cpu->wz=cpu->de+1;goto step_next; // LD A,(DE) T:2
        case  581: goto fetch_next; // LD A,(DE) T:3
        case  582: goto step_next; // DEC DE T:1
        case  583: goto fetch_next; // DEC DE T:2
        case  584: _wait();_mread(cpu->pc++);goto step_next; // LD E,n T:1
        case  585: cpu->e=_gd();goto step_next; // LD E,n T:2
        case  586: goto fetch_next; // LD E,n T:3
        case  587: _wait();_mread(cpu->pc++);goto step_next; // JR NZ,d T:1
        case  588: cpu->dlatch=_gd();if(!(_cc_nz)){_skip(5);};goto step_next; // JR NZ,d T:2
        case  589: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;goto step_next; // JR NZ,d T:3
        case  590: goto step_next; // JR NZ,d T:4
        case  591: goto step_next; // JR NZ,d T:5
        case  592: goto step_next; // JR NZ,d T:6
        case  593: goto step_next; // JR NZ,d T:7
        case  594: goto fetch_next; // JR NZ,d T:8
        case  595: _wait();_mread(cpu->pc++);goto step_next; // LD HL,nn T:1
        case  596: cpu->hlx[cpu->hlx_idx].l=_gd();goto step_next; // LD HL,nn T:2
        case  597: goto step_next; // LD HL,nn T:3
        case  598: _wait();_mread(cpu->pc++);goto step_next; // LD HL,nn T:4
        case  599: cpu->hlx[cpu->hlx_idx].h=_gd();goto step_next; // LD HL,nn T:5
        case  600: goto fetch_next; // LD HL,nn T:6
        case  601: _wait();_mread(cpu->pc++);goto step_next; // LD (nn),HL T:1
        case  602: cpu->wzl=_gd();goto step_next; // LD (nn),HL T:2
        case  603: goto step_next; // LD (nn),HL T:3
        case  604: _wait();_mread(cpu->pc++);goto step_next; // LD (nn),HL T:4
        case  605: cpu->wzh=_gd();goto step_next; // LD (nn),HL T:5
        case  606: goto step_next; // LD (nn),HL T:6
        case  607: _wait();_mwrite(cpu->wz++,cpu->hlx[cpu->hlx_idx].l);goto step_next; // LD (nn),HL T:7
        case  608: goto step_next; // LD (nn),HL T:8
        case  609: goto step_next; // LD (nn),HL T:9
        case  610: _wait();_mwrite(cpu->wz,cpu->hlx[cpu->hlx_idx].h);goto step_next; // LD (nn),HL T:10
        case  611: goto step_next; // LD (nn),HL T:11
        case  612: goto fetch_next; // LD (nn),HL T:12
        case  613: goto step_next; // INC HL T:1
        case  614: goto fetch_next; // INC HL T:2
        case  615: _wait();_mread(cpu->pc++);goto step_next; // LD H,n T:1
        case  616: cpu->hlx[cpu->hlx_idx].h=_gd();goto step_next; // LD H,n T:2
        case  617: goto fetch_next; // LD H,n T:3
        case  618: _wait();_mread(cpu->pc++);goto step_next; // JR Z,d T:1
        case  619: cpu->dlatch=_gd();if(!(_cc_z)){_skip(5);};goto step_next; // JR Z,d T:2
        case  620: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;goto step_next; // JR Z,d T:3
        case  621: goto step_next; // JR Z,d T:4
        case  622: goto step_next; // JR Z,d T:5
        case  623: goto step_next; // JR Z,d T:6
        case  624: goto step_next; // JR Z,d T:7
        case  625: goto fetch_next; // JR Z,d T:8
        case  626: goto step_next; // ADD HL,HL T:1
        case  627: goto step_next; // ADD HL,HL T:2
        case  628: goto step_next; // ADD HL,HL T:3
        case  629: goto step_next; // ADD HL,HL T:4
        case  630: goto step_next; // ADD HL,HL T:5
        case  631: goto step_next; // ADD HL,HL T:6
        case  632: goto fetch_next; // ADD HL,HL T:7
        case  633: _wait();_mread(cpu->pc++);goto step_next; // LD HL,(nn) T:1
        case  634: cpu->wzl=_gd();goto step_next; // LD HL,(nn) T:2
        case  635: goto step_next; // LD HL,(nn) T:3
        case  636: _wait();_mread(cpu->pc++);goto step_next; // LD HL,(nn) T:4
        case  637: cpu->wzh=_gd();goto step_next; // LD HL,(nn) T:5
        case  638: goto step_next; // LD HL,(nn) T:6
        case  639: _wait();_mread(cpu->wz++);goto step_next; // LD HL,(nn) T:7
        case  640: cpu->hlx[cpu->hlx_idx].l=_gd();goto step_next; // LD HL,(nn) T:8
        case  641: goto step_next; // LD HL,(nn) T:9
        case  642: _wait();_mread(cpu->wz);goto step_next; // LD HL,(nn) T:10
        case  643: cpu->hlx[cpu->hlx_idx].h=_gd();goto step_next; // LD HL,(nn) T:11
        case  644: goto fetch_next; // LD HL,(nn) T:12
        case  645: goto step_next; // DEC HL T:1
        case  646: goto fetch_next; // DEC HL T:2
        case  647: _wait();_mread(cpu->pc++);goto step_next; // LD L,n T:1
        case  648: cpu->hlx[cpu->hlx_idx].l=_gd();goto step_next; // LD L,n T:2
        case  649: goto fetch_next; // LD L,n T:3
        case  650: _wait();_mread(cpu->pc++);goto step_next; // JR NC,d T:1
        case  651: cpu->dlatch=_gd();if(!(_cc_nc)){_skip(5);};goto step_next; // JR NC,d T:2
        case  652: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;goto step_next; // JR NC,d T:3
        case  653: goto step_next; // JR NC,d T:4
        case  654: goto step_next; // JR NC,d T:5
        case  655: goto step_next; // JR NC,d T:6
        case  656: goto step_next; // JR NC,d T:7
        case  657: goto fetch_next; // JR NC,d T:8
        case  658: _wait();_mread(cpu->pc++);goto step_next; // LD SP,nn T:1
        case  659: cpu->spl=_gd();goto step_next; // LD SP,nn T:2
        case  660: goto step_next; // LD SP,nn T:3
        case  661: _wait();_mread(cpu->pc++);goto step_next; // LD SP,nn T:4
        case  662: cpu->sph=_gd();goto step_next; // LD SP,nn T:5
        case  663: goto fetch_next; // LD SP,nn T:6
        case  664: _wait();_mread(cpu->pc++);goto step_next; // LD (nn),A T:1
        case  665: cpu->wzl=_gd();goto step_next; // LD (nn),A T:2
        case  666: goto step_next; // LD (nn),A T:3
        case  667: _wait();_mread(cpu->pc++);goto step_next; // LD (nn),A T:4
        case  668: cpu->wzh=_gd();goto step_next; // LD (nn),A T:5
        case  669: goto step_next; // LD (nn),A T:6
        case  670: _wait();_mwrite(cpu->wz++,cpu->a);cpu->wzh=cpu->a;goto step_next; // LD (nn),A T:7
        case  671: goto step_next; // LD (nn),A T:8
        case  672: goto fetch_next; // LD (nn),A T:9
        case  673: goto step_next; // INC SP T:1
        case  674: goto fetch_next; // INC SP T:2
        case  675: _wait();_mread(cpu->addr);goto step_next; // INC (HL) T:1
        case  676: cpu->dlatch=_gd();cpu->dlatch=_z80_inc8(cpu,cpu->dlatch);goto step_next; // INC (HL) T:2
        case  677: goto step_next; // INC (HL) T:3
        case  678: goto step_next; // INC (HL) T:4
        case  679: _wait();_mwrite(cpu->addr,cpu->dlatch);goto step_next; // INC (HL) T:5
        case  680: goto step_next; // INC (HL) T:6
        case  681: goto fetch_next; // INC (HL) T:7
        case  682: _wait();_mread(cpu->addr);goto step_next; // DEC (HL) T:1
        case  683: cpu->dlatch=_gd();cpu->dlatch=_z80_dec8(cpu,cpu->dlatch);goto step_next; // DEC (HL) T:2
        case  684: goto step_next; // DEC (HL) T:3
        case  685: goto step_next; // DEC (HL) T:4
        case  686: _wait();_mwrite(cpu->addr,cpu->dlatch);goto step_next; // DEC (HL) T:5
        case  687: goto step_next; // DEC (HL) T:6
        case  688: goto fetch_next; // DEC (HL) T:7
        case  689: _wait();_mread(cpu->pc++);goto step_next; // LD (HL),n T:1
        case  690: cpu->dlatch=_gd();goto step_next; // LD (HL),n T:2
        case  691: goto step_next; // LD (HL),n T:3
        case  692: _wait();_mwrite(cpu->addr,cpu->dlatch);goto step_next; // LD (HL),n T:4
        case  693: goto step_next; // LD (HL),n T:5
        case  694: goto fetch_next; // LD (HL),n T:6
        case  695: _wait();_mread(cpu->pc++);goto step_next; // JR C,d T:1
        case  696: cpu->dlatch=_gd();if(!(_cc_c)){_skip(5);};goto step_next; // JR C,d T:2
        case  697: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;goto step_next; // JR C,d T:3
        case  698: goto step_next; // JR C,d T:4
        case  699: goto step_next; // JR C,d T:5
        case  700: goto step_next; // JR C,d T:6
        case  701: goto step_next; // JR C,d T:7
        case  702: goto fetch_next; // JR C,d T:8
        case  703: goto step_next; // ADD HL,SP T:1
        case  704: goto step_next; // ADD HL,SP T:2
        case  705: goto step_next; // ADD HL,SP T:3
        case  706: goto step_next; // ADD HL,SP T:4
        case  707: goto step_next; // ADD HL,SP T:5
        case  708: goto step_next; // ADD HL,SP T:6
        case  709: goto fetch_next; // ADD HL,SP T:7
        case  710: _wait();_mread(cpu->pc++);goto step_next; // LD A,(nn) T:1
        case  711: cpu->wzl=_gd();goto step_next; // LD A,(nn) T:2
        case  712: goto step_next; // LD A,(nn) T:3
        case  713: _wait();_mread(cpu->pc++);goto step_next; // LD A,(nn) T:4
        case  714: cpu->wzh=_gd();goto step_next; // LD A,(nn) T:5
        case  715: goto step_next; // LD A,(nn) T:6
        case  716: _wait();_mread(cpu->wz++);goto step_next; // LD A,(nn) T:7
        case  717: cpu->a=_gd();goto step_next; // LD A,(nn) T:8
        case  718: goto fetch_next; // LD A,(nn) T:9
        case  719: goto step_next; // DEC SP T:1
        case  720: goto fetch_next; // DEC SP T:2
        case  721: _wait();_mread(cpu->pc++);goto step_next; // LD A,n T:1
        case  722: cpu->a=_gd();goto step_next; // LD A,n T:2
        case  723: goto fetch_next; // LD A,n T:3
        case  724: _wait();_mread(cpu->addr);goto step_next; // LD B,(HL) T:1
        case  725: cpu->b=_gd();goto step_next; // LD B,(HL) T:2
        case  726: goto fetch_next; // LD B,(HL) T:3
        case  727: _wait();_mread(cpu->addr);goto step_next; // LD C,(HL) T:1
        case  728: cpu->c=_gd();goto step_next; // LD C,(HL) T:2
        case  729: goto fetch_next; // LD C,(HL) T:3
        case  730: _wait();_mread(cpu->addr);goto step_next; // LD D,(HL) T:1
        case  731: cpu->d=_gd();goto step_next; // LD D,(HL) T:2
        case  732: goto fetch_next; // LD D,(HL) T:3
        case  733: _wait();_mread(cpu->addr);goto step_next; // LD E,(HL) T:1
        case  734: cpu->e=_gd();goto step_next; // LD E,(HL) T:2
        case  735: goto fetch_next; // LD E,(HL) T:3
        case  736: _wait();_mread(cpu->addr);goto step_next; // LD H,(HL) T:1
        case  737: cpu->h=_gd();goto step_next; // LD H,(HL) T:2
        case  738: goto fetch_next; // LD H,(HL) T:3
        case  739: _wait();_mread(cpu->addr);goto step_next; // LD L,(HL) T:1
        case  740: cpu->l=_gd();goto step_next; // LD L,(HL) T:2
        case  741: goto fetch_next; // LD L,(HL) T:3
        case  742: _wait();_mwrite(cpu->addr,cpu->b);goto step_next; // LD (HL),B T:1
        case  743: goto step_next; // LD (HL),B T:2
        case  744: goto fetch_next; // LD (HL),B T:3
        case  745: _wait();_mwrite(cpu->addr,cpu->c);goto step_next; // LD (HL),C T:1
        case  746: goto step_next; // LD (HL),C T:2
        case  747: goto fetch_next; // LD (HL),C T:3
        case  748: _wait();_mwrite(cpu->addr,cpu->d);goto step_next; // LD (HL),D T:1
        case  749: goto step_next; // LD (HL),D T:2
        case  750: goto fetch_next; // LD (HL),D T:3
        case  751: _wait();_mwrite(cpu->addr,cpu->e);goto step_next; // LD (HL),E T:1
        case  752: goto step_next; // LD (HL),E T:2
        case  753: goto fetch_next; // LD (HL),E T:3
        case  754: _wait();_mwrite(cpu->addr,cpu->h);goto step_next; // LD (HL),H T:1
        case  755: goto step_next; // LD (HL),H T:2
        case  756: goto fetch_next; // LD (HL),H T:3
        case  757: _wait();_mwrite(cpu->addr,cpu->l);goto step_next; // LD (HL),L T:1
        case  758: goto step_next; // LD (HL),L T:2
        case  759: goto fetch_next; // LD (HL),L T:3
        case  760: _wait();_mwrite(cpu->addr,cpu->a);goto step_next; // LD (HL),A T:1
        case  761: goto step_next; // LD (HL),A T:2
        case  762: goto fetch_next; // LD (HL),A T:3
        case  763: _wait();_mread(cpu->addr);goto step_next; // LD A,(HL) T:1
        case  764: cpu->a=_gd();goto step_next; // LD A,(HL) T:2
        case  765: goto fetch_next; // LD A,(HL) T:3
        case  766: _wait();_mread(cpu->addr);goto step_next; // ADD (HL) T:1
        case  767: cpu->dlatch=_gd();goto step_next; // ADD (HL) T:2
        case  768: _z80_add8(cpu,cpu->dlatch);goto fetch_next; // ADD (HL) T:3
        case  769: _wait();_mread(cpu->addr);goto step_next; // ADC (HL) T:1
        case  770: cpu->dlatch=_gd();goto step_next; // ADC (HL) T:2
        case  771: _z80_adc8(cpu,cpu->dlatch);goto fetch_next; // ADC (HL) T:3
        case  772: _wait();_mread(cpu->addr);goto step_next; // SUB (HL) T:1
        case  773: cpu->dlatch=_gd();goto step_next; // SUB (HL) T:2
        case  774: _z80_sub8(cpu,cpu->dlatch);goto fetch_next; // SUB (HL) T:3
        case  775: _wait();_mread(cpu->addr);goto step_next; // SBC (HL) T:1
        case  776: cpu->dlatch=_gd();goto step_next; // SBC (HL) T:2
        case  777: _z80_sbc8(cpu,cpu->dlatch);goto fetch_next; // SBC (HL) T:3
        case  778: _wait();_mread(cpu->addr);goto step_next; // AND (HL) T:1
        case  779: cpu->dlatch=_gd();goto step_next; // AND (HL) T:2
        case  780: _z80_and8(cpu,cpu->dlatch);goto fetch_next; // AND (HL) T:3
        case  781: _wait();_mread(cpu->addr);goto step_next; // XOR (HL) T:1
        case  782: cpu->dlatch=_gd();goto step_next; // XOR (HL) T:2
        case  783: _z80_xor8(cpu,cpu->dlatch);goto fetch_next; // XOR (HL) T:3
        case  784: _wait();_mread(cpu->addr);goto step_next; // OR (HL) T:1
        case  785: cpu->dlatch=_gd();goto step_next; // OR (HL) T:2
        case  786: _z80_or8(cpu,cpu->dlatch);goto fetch_next; // OR (HL) T:3
        case  787: _wait();_mread(cpu->addr);goto step_next; // CP (HL) T:1
        case  788: cpu->dlatch=_gd();goto step_next; // CP (HL) T:2
        case  789: _z80_cp8(cpu,cpu->dlatch);goto fetch_next; // CP (HL) T:3
        case  790: goto step_next; // RET NZ T:1
        case  791: _wait();_mread(cpu->sp++);goto step_next; // RET NZ T:2
        case  792: cpu->wzl=_gd();goto step_next; // RET NZ T:3
        case  793: goto step_next; // RET NZ T:4
        case  794: _wait();_mread(cpu->sp++);goto step_next; // RET NZ T:5
        case  795: cpu->wzh=_gd();cpu->pc=cpu->wz;goto step_next; // RET NZ T:6
        case  796: goto fetch_next; // RET NZ T:7
        case  797: _wait();_mread(cpu->sp++);goto step_next; // POP BC T:1
        case  798: cpu->c=_gd();goto step_next; // POP BC T:2
        case  799: goto step_next; // POP BC T:3
        case  800: _wait();_mread(cpu->sp++);goto step_next; // POP BC T:4
        case  801: cpu->b=_gd();goto step_next; // POP BC T:5
        case  802: goto fetch_next; // POP BC T:6
        case  803: _wait();_mread(cpu->pc++);goto step_next; // JP NZ,nn T:1
        case  804: cpu->wzl=_gd();goto step_next; // JP NZ,nn T:2
        case  805: goto step_next; // JP NZ,nn T:3
        case  806: _wait();_mread(cpu->pc++);goto step_next; // JP NZ,nn T:4
        case  807: cpu->wzh=_gd();if(_cc_nz){cpu->pc=cpu->wz;};goto step_next; // JP NZ,nn T:5
        case  808: goto fetch_next; // JP NZ,nn T:6
        case  809: _wait();_mread(cpu->pc++);goto step_next; // JP nn T:1
        case  810: cpu->wzl=_gd();goto step_next; // JP nn T:2
        case  811: goto step_next; // JP nn T:3
        case  812: _wait();_mread(cpu->pc++);goto step_next; // JP nn T:4
        case  813: cpu->wzh=_gd();cpu->pc=cpu->wz;goto step_next; // JP nn T:5
        case  814: goto fetch_next; // JP nn T:6
        case  815: _wait();_mread(cpu->pc++);goto step_next; // CALL NZ,nn T:1
        case  816: cpu->wzl=_gd();goto step_next; // CALL NZ,nn T:2
        case  817: goto step_next; // CALL NZ,nn T:3
        case  818: _wait();_mread(cpu->pc++);goto step_next; // CALL NZ,nn T:4
        case  819: cpu->wzh=_gd();if (!_cc_nz){_skip(7);};goto step_next; // CALL NZ,nn T:5
        case  820: goto step_next; // CALL NZ,nn T:6
        case  821: goto step_next; // CALL NZ,nn T:7
        case  822: _wait();_mwrite(--cpu->sp,cpu->pch);goto step_next; // CALL NZ,nn T:8
        case  823: goto step_next; // CALL NZ,nn T:9
        case  824: goto step_next; // CALL NZ,nn T:10
        case  825: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz;goto step_next; // CALL NZ,nn T:11
        case  826: goto step_next; // CALL NZ,nn T:12
        case  827: goto fetch_next; // CALL NZ,nn T:13
        case  828: goto step_next; // PUSH BC T:1
        case  829: _wait();_mwrite(--cpu->sp,cpu->b);goto step_next; // PUSH BC T:2
        case  830: goto step_next; // PUSH BC T:3
        case  831: goto step_next; // PUSH BC T:4
        case  832: _wait();_mwrite(--cpu->sp,cpu->c);goto step_next; // PUSH BC T:5
        case  833: goto step_next; // PUSH BC T:6
        case  834: goto fetch_next; // PUSH BC T:7
        case  835: _wait();_mread(cpu->pc++);goto step_next; // ADD n T:1
        case  836: cpu->dlatch=_gd();goto step_next; // ADD n T:2
        case  837: _z80_add8(cpu,cpu->dlatch);goto fetch_next; // ADD n T:3
        case  838: goto step_next; // RST 0h T:1
        case  839: _wait();_mwrite(--cpu->sp,cpu->pch);goto step_next; // RST 0h T:2
        case  840: goto step_next; // RST 0h T:3
        case  841: goto step_next; // RST 0h T:4
        case  842: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x00;cpu->pc=cpu->wz;goto step_next; // RST 0h T:5
        case  843: goto step_next; // RST 0h T:6
        case  844: goto fetch_next; // RST 0h T:7
        case  845: goto step_next; // RET Z T:1
        case  846: _wait();_mread(cpu->sp++);goto step_next; // RET Z T:2
        case  847: cpu->wzl=_gd();goto step_next; // RET Z T:3
        case  848: goto step_next; // RET Z T:4
        case  849: _wait();_mread(cpu->sp++);goto step_next; // RET Z T:5
        case  850: cpu->wzh=_gd();cpu->pc=cpu->wz;goto step_next; // RET Z T:6
        case  851: goto fetch_next; // RET Z T:7
        case  852: _wait();_mread(cpu->sp++);goto step_next; // RET T:1
        case  853: cpu->wzl=_gd();goto step_next; // RET T:2
        case  854: goto step_next; // RET T:3
        case  855: _wait();_mread(cpu->sp++);goto step_next; // RET T:4
        case  856: cpu->wzh=_gd();cpu->pc=cpu->wz;goto step_next; // RET T:5
        case  857: goto fetch_next; // RET T:6
        case  858: _wait();_mread(cpu->pc++);goto step_next; // JP Z,nn T:1
        case  859: cpu->wzl=_gd();goto step_next; // JP Z,nn T:2
        case  860: goto step_next; // JP Z,nn T:3
        case  861: _wait();_mread(cpu->pc++);goto step_next; // JP Z,nn T:4
        case  862: cpu->wzh=_gd();if(_cc_z){cpu->pc=cpu->wz;};goto step_next; // JP Z,nn T:5
        case  863: goto fetch_next; // JP Z,nn T:6
        case  864: _wait();_mread(cpu->pc++);goto step_next; // CALL Z,nn T:1
        case  865: cpu->wzl=_gd();goto step_next; // CALL Z,nn T:2
        case  866: goto step_next; // CALL Z,nn T:3
        case  867: _wait();_mread(cpu->pc++);goto step_next; // CALL Z,nn T:4
        case  868: cpu->wzh=_gd();if (!_cc_z){_skip(7);};goto step_next; // CALL Z,nn T:5
        case  869: goto step_next; // CALL Z,nn T:6
        case  870: goto step_next; // CALL Z,nn T:7
        case  871: _wait();_mwrite(--cpu->sp,cpu->pch);goto step_next; // CALL Z,nn T:8
        case  872: goto step_next; // CALL Z,nn T:9
        case  873: goto step_next; // CALL Z,nn T:10
        case  874: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz;goto step_next; // CALL Z,nn T:11
        case  875: goto step_next; // CALL Z,nn T:12
        case  876: goto fetch_next; // CALL Z,nn T:13
        case  877: _wait();_mread(cpu->pc++);goto step_next; // CALL nn T:1
        case  878: cpu->wzl=_gd();goto step_next; // CALL nn T:2
        case  879: goto step_next; // CALL nn T:3
        case  880: _wait();_mread(cpu->pc++);goto step_next; // CALL nn T:4
        case  881: cpu->wzh=_gd();goto step_next; // CALL nn T:5
        case  882: goto step_next; // CALL nn T:6
        case  883: goto step_next; // CALL nn T:7
        case  884: _wait();_mwrite(--cpu->sp,cpu->pch);goto step_next; // CALL nn T:8
        case  885: goto step_next; // CALL nn T:9
        case  886: goto step_next; // CALL nn T:10
        case  887: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz;goto step_next; // CALL nn T:11
        case  888: goto step_next; // CALL nn T:12
        case  889: goto fetch_next; // CALL nn T:13
        case  890: _wait();_mread(cpu->pc++);goto step_next; // ADC n T:1
        case  891: cpu->dlatch=_gd();goto step_next; // ADC n T:2
        case  892: _z80_adc8(cpu,cpu->dlatch);goto fetch_next; // ADC n T:3
        case  893: goto step_next; // RST 8h T:1
        case  894: _wait();_mwrite(--cpu->sp,cpu->pch);goto step_next; // RST 8h T:2
        case  895: goto step_next; // RST 8h T:3
        case  896: goto step_next; // RST 8h T:4
        case  897: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x08;cpu->pc=cpu->wz;goto step_next; // RST 8h T:5
        case  898: goto step_next; // RST 8h T:6
        case  899: goto fetch_next; // RST 8h T:7
        case  900: goto step_next; // RET NC T:1
        case  901: _wait();_mread(cpu->sp++);goto step_next; // RET NC T:2
        case  902: cpu->wzl=_gd();goto step_next; // RET NC T:3
        case  903: goto step_next; // RET NC T:4
        case  904: _wait();_mread(cpu->sp++);goto step_next; // RET NC T:5
        case  905: cpu->wzh=_gd();cpu->pc=cpu->wz;goto step_next; // RET NC T:6
        case  906: goto fetch_next; // RET NC T:7
        case  907: _wait();_mread(cpu->sp++);goto step_next; // POP DE T:1
        case  908: cpu->e=_gd();goto step_next; // POP DE T:2
        case  909: goto step_next; // POP DE T:3
        case  910: _wait();_mread(cpu->sp++);goto step_next; // POP DE T:4
        case  911: cpu->d=_gd();goto step_next; // POP DE T:5
        case  912: goto fetch_next; // POP DE T:6
        case  913: _wait();_mread(cpu->pc++);goto step_next; // JP NC,nn T:1
        case  914: cpu->wzl=_gd();goto step_next; // JP NC,nn T:2
        case  915: goto step_next; // JP NC,nn T:3
        case  916: _wait();_mread(cpu->pc++);goto step_next; // JP NC,nn T:4
        case  917: cpu->wzh=_gd();if(_cc_nc){cpu->pc=cpu->wz;};goto step_next; // JP NC,nn T:5
        case  918: goto fetch_next; // JP NC,nn T:6
        case  919: _wait();_mread(cpu->pc++);goto step_next; // OUT (n),A T:1
        case  920: cpu->wzl=_gd();cpu->wzh=cpu->a;goto step_next; // OUT (n),A T:2
        case  921: goto step_next; // OUT (n),A T:3
        case  922: _iowrite(cpu->wz,cpu->a);goto step_next; // OUT (n),A T:4
        case  923: _wait();cpu->wzl++;goto step_next; // OUT (n),A T:5
        case  924: goto step_next; // OUT (n),A T:6
        case  925: goto fetch_next; // OUT (n),A T:7
        case  926: _wait();_mread(cpu->pc++);goto step_next; // CALL NC,nn T:1
        case  927: cpu->wzl=_gd();goto step_next; // CALL NC,nn T:2
        case  928: goto step_next; // CALL NC,nn T:3
        case  929: _wait();_mread(cpu->pc++);goto step_next; // CALL NC,nn T:4
        case  930: cpu->wzh=_gd();if (!_cc_nc){_skip(7);};goto step_next; // CALL NC,nn T:5
        case  931: goto step_next; // CALL NC,nn T:6
        case  932: goto step_next; // CALL NC,nn T:7
        case  933: _wait();_mwrite(--cpu->sp,cpu->pch);goto step_next; // CALL NC,nn T:8
        case  934: goto step_next; // CALL NC,nn T:9
        case  935: goto step_next; // CALL NC,nn T:10
        case  936: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz;goto step_next; // CALL NC,nn T:11
        case  937: goto step_next; // CALL NC,nn T:12
        case  938: goto fetch_next; // CALL NC,nn T:13
        case  939: goto step_next; // PUSH DE T:1
        case  940: _wait();_mwrite(--cpu->sp,cpu->d);goto step_next; // PUSH DE T:2
        case  941: goto step_next; // PUSH DE T:3
        case  942: goto step_next; // PUSH DE T:4
        case  943: _wait();_mwrite(--cpu->sp,cpu->e);goto step_next; // PUSH DE T:5
        case  944: goto step_next; // PUSH DE T:6
        case  945: goto fetch_next; // PUSH DE T:7
        case  946: _wait();_mread(cpu->pc++);goto step_next; // SUB n T:1
        case  947: cpu->dlatch=_gd();goto step_next; // SUB n T:2
        case  948: _z80_sub8(cpu,cpu->dlatch);goto fetch_next; // SUB n T:3
        case  949: goto step_next; // RST 10h T:1
        case  950: _wait();_mwrite(--cpu->sp,cpu->pch);goto step_next; // RST 10h T:2
        case  951: goto step_next; // RST 10h T:3
        case  952: goto step_next; // RST 10h T:4
        case  953: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x10;cpu->pc=cpu->wz;goto step_next; // RST 10h T:5
        case  954: goto step_next; // RST 10h T:6
        case  955: goto fetch_next; // RST 10h T:7
        case  956: goto step_next; // RET C T:1
        case  957: _wait();_mread(cpu->sp++);goto step_next; // RET C T:2
        case  958: cpu->wzl=_gd();goto step_next; // RET C T:3
        case  959: goto step_next; // RET C T:4
        case  960: _wait();_mread(cpu->sp++);goto step_next; // RET C T:5
        case  961: cpu->wzh=_gd();cpu->pc=cpu->wz;goto step_next; // RET C T:6
        case  962: goto fetch_next; // RET C T:7
        case  963: _wait();_mread(cpu->pc++);goto step_next; // JP C,nn T:1
        case  964: cpu->wzl=_gd();goto step_next; // JP C,nn T:2
        case  965: goto step_next; // JP C,nn T:3
        case  966: _wait();_mread(cpu->pc++);goto step_next; // JP C,nn T:4
        case  967: cpu->wzh=_gd();if(_cc_c){cpu->pc=cpu->wz;};goto step_next; // JP C,nn T:5
        case  968: goto fetch_next; // JP C,nn T:6
        case  969: _wait();_mread(cpu->pc++);goto step_next; // IN A,(n) T:1
        case  970: cpu->wzl=_gd();cpu->wzh=cpu->a;goto step_next; // IN A,(n) T:2
        case  971: goto step_next; // IN A,(n) T:3
        case  972: goto step_next; // IN A,(n) T:4
        case  973: _wait();_ioread(cpu->wz++);goto step_next; // IN A,(n) T:5
        case  974: cpu->a=_gd();goto step_next; // IN A,(n) T:6
        case  975: goto fetch_next; // IN A,(n) T:7
        case  976: _wait();_mread(cpu->pc++);goto step_next; // CALL C,nn T:1
        case  977: cpu->wzl=_gd();goto step_next; // CALL C,nn T:2
        case  978: goto step_next; // CALL C,nn T:3
        case  979: _wait();_mread(cpu->pc++);goto step_next; // CALL C,nn T:4
        case  980: cpu->wzh=_gd();if (!_cc_c){_skip(7);};goto step_next; // CALL C,nn T:5
        case  981: goto step_next; // CALL C,nn T:6
        case  982: goto step_next; // CALL C,nn T:7
        case  983: _wait();_mwrite(--cpu->sp,cpu->pch);goto step_next; // CALL C,nn T:8
        case  984: goto step_next; // CALL C,nn T:9
        case  985: goto step_next; // CALL C,nn T:10
        case  986: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz;goto step_next; // CALL C,nn T:11
        case  987: goto step_next; // CALL C,nn T:12
        case  988: goto fetch_next; // CALL C,nn T:13
        case  989: _wait();_mread(cpu->pc++);goto step_next; // SBC n T:1
        case  990: cpu->dlatch=_gd();goto step_next; // SBC n T:2
        case  991: _z80_sbc8(cpu,cpu->dlatch);goto fetch_next; // SBC n T:3
        case  992: goto step_next; // RST 18h T:1
        case  993: _wait();_mwrite(--cpu->sp,cpu->pch);goto step_next; // RST 18h T:2
        case  994: goto step_next; // RST 18h T:3
        case  995: goto step_next; // RST 18h T:4
        case  996: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x18;cpu->pc=cpu->wz;goto step_next; // RST 18h T:5
        case  997: goto step_next; // RST 18h T:6
        case  998: goto fetch_next; // RST 18h T:7
        case  999: goto step_next; // RET PO T:1
        case 1000: _wait();_mread(cpu->sp++);goto step_next; // RET PO T:2
        case 1001: cpu->wzl=_gd();goto step_next; // RET PO T:3
        case 1002: goto step_next; // RET PO T:4
        case 1003: _wait();_mread(cpu->sp++);goto step_next; // RET PO T:5
        case 1004: cpu->wzh=_gd();cpu->pc=cpu->wz;goto step_next; // RET PO T:6
        case 1005: goto fetch_next; // RET PO T:7
        case 1006: _wait();_mread(cpu->sp++);goto step_next; // POP HL T:1
        case 1007: cpu->hlx[cpu->hlx_idx].l=_gd();goto step_next; // POP HL T:2
        case 1008: goto step_next; // POP HL T:3
        case 1009: _wait();_mread(cpu->sp++);goto step_next; // POP HL T:4
        case 1010: cpu->hlx[cpu->hlx_idx].h=_gd();goto step_next; // POP HL T:5
        case 1011: goto fetch_next; // POP HL T:6
        case 1012: _wait();_mread(cpu->pc++);goto step_next; // JP PO,nn T:1
        case 1013: cpu->wzl=_gd();goto step_next; // JP PO,nn T:2
        case 1014: goto step_next; // JP PO,nn T:3
        case 1015: _wait();_mread(cpu->pc++);goto step_next; // JP PO,nn T:4
        case 1016: cpu->wzh=_gd();if(_cc_po){cpu->pc=cpu->wz;};goto step_next; // JP PO,nn T:5
        case 1017: goto fetch_next; // JP PO,nn T:6
        case 1018: _wait();_mread(cpu->sp);goto step_next; // EX (SP),HL T:1
        case 1019: cpu->wzl=_gd();goto step_next; // EX (SP),HL T:2
        case 1020: goto step_next; // EX (SP),HL T:3
        case 1021: _wait();_mread(cpu->sp+1);goto step_next; // EX (SP),HL T:4
        case 1022: cpu->wzh=_gd();goto step_next; // EX (SP),HL T:5
        case 1023: goto step_next; // EX (SP),HL T:6
        case 1024: goto step_next; // EX (SP),HL T:7
        case 1025: _wait();_mwrite(cpu->sp+1,cpu->hlx[cpu->hlx_idx].h);goto step_next; // EX (SP),HL T:8
        case 1026: goto step_next; // EX (SP),HL T:9
        case 1027: goto step_next; // EX (SP),HL T:10
        case 1028: _wait();_mwrite(cpu->sp,cpu->hlx[cpu->hlx_idx].l);cpu->hlx[cpu->hlx_idx].hl=cpu->wz;goto step_next; // EX (SP),HL T:11
        case 1029: goto step_next; // EX (SP),HL T:12
        case 1030: goto step_next; // EX (SP),HL T:13
        case 1031: goto step_next; // EX (SP),HL T:14
        case 1032: goto fetch_next; // EX (SP),HL T:15
        case 1033: _wait();_mread(cpu->pc++);goto step_next; // CALL PO,nn T:1
        case 1034: cpu->wzl=_gd();goto step_next; // CALL PO,nn T:2
        case 1035: goto step_next; // CALL PO,nn T:3
        case 1036: _wait();_mread(cpu->pc++);goto step_next; // CALL PO,nn T:4
        case 1037: cpu->wzh=_gd();if (!_cc_po){_skip(7);};goto step_next; // CALL PO,nn T:5
        case 1038: goto step_next; // CALL PO,nn T:6
        case 1039: goto step_next; // CALL PO,nn T:7
        case 1040: _wait();_mwrite(--cpu->sp,cpu->pch);goto step_next; // CALL PO,nn T:8
        case 1041: goto step_next; // CALL PO,nn T:9
        case 1042: goto step_next; // CALL PO,nn T:10
        case 1043: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz;goto step_next; // CALL PO,nn T:11
        case 1044: goto step_next; // CALL PO,nn T:12
        case 1045: goto fetch_next; // CALL PO,nn T:13
        case 1046: goto step_next; // PUSH HL T:1
        case 1047: _wait();_mwrite(--cpu->sp,cpu->hlx[cpu->hlx_idx].h);goto step_next; // PUSH HL T:2
        case 1048: goto step_next; // PUSH HL T:3
        case 1049: goto step_next; // PUSH HL T:4
        case 1050: _wait();_mwrite(--cpu->sp,cpu->hlx[cpu->hlx_idx].l);goto step_next; // PUSH HL T:5
        case 1051: goto step_next; // PUSH HL T:6
        case 1052: goto fetch_next; // PUSH HL T:7
        case 1053: _wait();_mread(cpu->pc++);goto step_next; // AND n T:1
        case 1054: cpu->dlatch=_gd();goto step_next; // AND n T:2
        case 1055: _z80_and8(cpu,cpu->dlatch);goto fetch_next; // AND n T:3
        case 1056: goto step_next; // RST 20h T:1
        case 1057: _wait();_mwrite(--cpu->sp,cpu->pch);goto step_next; // RST 20h T:2
        case 1058: goto step_next; // RST 20h T:3
        case 1059: goto step_next; // RST 20h T:4
        case 1060: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x20;cpu->pc=cpu->wz;goto step_next; // RST 20h T:5
        case 1061: goto step_next; // RST 20h T:6
        case 1062: goto fetch_next; // RST 20h T:7
        case 1063: goto step_next; // RET PE T:1
        case 1064: _wait();_mread(cpu->sp++);goto step_next; // RET PE T:2
        case 1065: cpu->wzl=_gd();goto step_next; // RET PE T:3
        case 1066: goto step_next; // RET PE T:4
        case 1067: _wait();_mread(cpu->sp++);goto step_next; // RET PE T:5
        case 1068: cpu->wzh=_gd();cpu->pc=cpu->wz;goto step_next; // RET PE T:6
        case 1069: goto fetch_next; // RET PE T:7
        case 1070: _wait();_mread(cpu->pc++);goto step_next; // JP PE,nn T:1
        case 1071: cpu->wzl=_gd();goto step_next; // JP PE,nn T:2
        case 1072: goto step_next; // JP PE,nn T:3
        case 1073: _wait();_mread(cpu->pc++);goto step_next; // JP PE,nn T:4
        case 1074: cpu->wzh=_gd();if(_cc_pe){cpu->pc=cpu->wz;};goto step_next; // JP PE,nn T:5
        case 1075: goto fetch_next; // JP PE,nn T:6
        case 1076: _wait();_mread(cpu->pc++);goto step_next; // CALL PE,nn T:1
        case 1077: cpu->wzl=_gd();goto step_next; // CALL PE,nn T:2
        case 1078: goto step_next; // CALL PE,nn T:3
        case 1079: _wait();_mread(cpu->pc++);goto step_next; // CALL PE,nn T:4
        case 1080: cpu->wzh=_gd();if (!_cc_pe){_skip(7);};goto step_next; // CALL PE,nn T:5
        case 1081: goto step_next; // CALL PE,nn T:6
        case 1082: goto step_next; // CALL PE,nn T:7
        case 1083: _wait();_mwrite(--cpu->sp,cpu->pch);goto step_next; // CALL PE,nn T:8
        case 1084: goto step_next; // CALL PE,nn T:9
        case 1085: goto step_next; // CALL PE,nn T:10
        case 1086: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz;goto step_next; // CALL PE,nn T:11
        case 1087: goto step_next; // CALL PE,nn T:12
        case 1088: goto fetch_next; // CALL PE,nn T:13
        case 1089: _wait();_mread(cpu->pc++);goto step_next; // XOR n T:1
        case 1090: cpu->dlatch=_gd();goto step_next; // XOR n T:2
        case 1091: _z80_xor8(cpu,cpu->dlatch);goto fetch_next; // XOR n T:3
        case 1092: goto step_next; // RST 28h T:1
        case 1093: _wait();_mwrite(--cpu->sp,cpu->pch);goto step_next; // RST 28h T:2
        case 1094: goto step_next; // RST 28h T:3
        case 1095: goto step_next; // RST 28h T:4
        case 1096: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x28;cpu->pc=cpu->wz;goto step_next; // RST 28h T:5
        case 1097: goto step_next; // RST 28h T:6
        case 1098: goto fetch_next; // RST 28h T:7
        case 1099: goto step_next; // RET P T:1
        case 1100: _wait();_mread(cpu->sp++);goto step_next; // RET P T:2
        case 1101: cpu->wzl=_gd();goto step_next; // RET P T:3
        case 1102: goto step_next; // RET P T:4
        case 1103: _wait();_mread(cpu->sp++);goto step_next; // RET P T:5
        case 1104: cpu->wzh=_gd();cpu->pc=cpu->wz;goto step_next; // RET P T:6
        case 1105: goto fetch_next; // RET P T:7
        case 1106: _wait();_mread(cpu->sp++);goto step_next; // POP AF T:1
        case 1107: cpu->f=_gd();goto step_next; // POP AF T:2
        case 1108: goto step_next; // POP AF T:3
        case 1109: _wait();_mread(cpu->sp++);goto step_next; // POP AF T:4
        case 1110: cpu->a=_gd();goto step_next; // POP AF T:5
        case 1111: goto fetch_next; // POP AF T:6
        case 1112: _wait();_mread(cpu->pc++);goto step_next; // JP P,nn T:1
        case 1113: cpu->wzl=_gd();goto step_next; // JP P,nn T:2
        case 1114: goto step_next; // JP P,nn T:3
        case 1115: _wait();_mread(cpu->pc++);goto step_next; // JP P,nn T:4
        case 1116: cpu->wzh=_gd();if(_cc_p){cpu->pc=cpu->wz;};goto step_next; // JP P,nn T:5
        case 1117: goto fetch_next; // JP P,nn T:6
        case 1118: _wait();_mread(cpu->pc++);goto step_next; // CALL P,nn T:1
        case 1119: cpu->wzl=_gd();goto step_next; // CALL P,nn T:2
        case 1120: goto step_next; // CALL P,nn T:3
        case 1121: _wait();_mread(cpu->pc++);goto step_next; // CALL P,nn T:4
        case 1122: cpu->wzh=_gd();if (!_cc_p){_skip(7);};goto step_next; // CALL P,nn T:5
        case 1123: goto step_next; // CALL P,nn T:6
        case 1124: goto step_next; // CALL P,nn T:7
        case 1125: _wait();_mwrite(--cpu->sp,cpu->pch);goto step_next; // CALL P,nn T:8
        case 1126: goto step_next; // CALL P,nn T:9
        case 1127: goto step_next; // CALL P,nn T:10
        case 1128: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz;goto step_next; // CALL P,nn T:11
        case 1129: goto step_next; // CALL P,nn T:12
        case 1130: goto fetch_next; // CALL P,nn T:13
        case 1131: goto step_next; // PUSH AF T:1
        case 1132: _wait();_mwrite(--cpu->sp,cpu->a);goto step_next; // PUSH AF T:2
        case 1133: goto step_next; // PUSH AF T:3
        case 1134: goto step_next; // PUSH AF T:4
        case 1135: _wait();_mwrite(--cpu->sp,cpu->f);goto step_next; // PUSH AF T:5
        case 1136: goto step_next; // PUSH AF T:6
        case 1137: goto fetch_next; // PUSH AF T:7
        case 1138: _wait();_mread(cpu->pc++);goto step_next; // OR n T:1
        case 1139: cpu->dlatch=_gd();goto step_next; // OR n T:2
        case 1140: _z80_or8(cpu,cpu->dlatch);goto fetch_next; // OR n T:3
        case 1141: goto step_next; // RST 30h T:1
        case 1142: _wait();_mwrite(--cpu->sp,cpu->pch);goto step_next; // RST 30h T:2
        case 1143: goto step_next; // RST 30h T:3
        case 1144: goto step_next; // RST 30h T:4
        case 1145: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x30;cpu->pc=cpu->wz;goto step_next; // RST 30h T:5
        case 1146: goto step_next; // RST 30h T:6
        case 1147: goto fetch_next; // RST 30h T:7
        case 1148: goto step_next; // RET M T:1
        case 1149: _wait();_mread(cpu->sp++);goto step_next; // RET M T:2
        case 1150: cpu->wzl=_gd();goto step_next; // RET M T:3
        case 1151: goto step_next; // RET M T:4
        case 1152: _wait();_mread(cpu->sp++);goto step_next; // RET M T:5
        case 1153: cpu->wzh=_gd();cpu->pc=cpu->wz;goto step_next; // RET M T:6
        case 1154: goto fetch_next; // RET M T:7
        case 1155: goto step_next; // LD SP,HL T:1
        case 1156: goto fetch_next; // LD SP,HL T:2
        case 1157: _wait();_mread(cpu->pc++);goto step_next; // JP M,nn T:1
        case 1158: cpu->wzl=_gd();goto step_next; // JP M,nn T:2
        case 1159: goto step_next; // JP M,nn T:3
        case 1160: _wait();_mread(cpu->pc++);goto step_next; // JP M,nn T:4
        case 1161: cpu->wzh=_gd();if(_cc_m){cpu->pc=cpu->wz;};goto step_next; // JP M,nn T:5
        case 1162: goto fetch_next; // JP M,nn T:6
        case 1163: _wait();_mread(cpu->pc++);goto step_next; // CALL M,nn T:1
        case 1164: cpu->wzl=_gd();goto step_next; // CALL M,nn T:2
        case 1165: goto step_next; // CALL M,nn T:3
        case 1166: _wait();_mread(cpu->pc++);goto step_next; // CALL M,nn T:4
        case 1167: cpu->wzh=_gd();if (!_cc_m){_skip(7);};goto step_next; // CALL M,nn T:5
        case 1168: goto step_next; // CALL M,nn T:6
        case 1169: goto step_next; // CALL M,nn T:7
        case 1170: _wait();_mwrite(--cpu->sp,cpu->pch);goto step_next; // CALL M,nn T:8
        case 1171: goto step_next; // CALL M,nn T:9
        case 1172: goto step_next; // CALL M,nn T:10
        case 1173: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz;goto step_next; // CALL M,nn T:11
        case 1174: goto step_next; // CALL M,nn T:12
        case 1175: goto fetch_next; // CALL M,nn T:13
        case 1176: _wait();_mread(cpu->pc++);goto step_next; // CP n T:1
        case 1177: cpu->dlatch=_gd();goto step_next; // CP n T:2
        case 1178: _z80_cp8(cpu,cpu->dlatch);goto fetch_next; // CP n T:3
        case 1179: goto step_next; // RST 38h T:1
        case 1180: _wait();_mwrite(--cpu->sp,cpu->pch);goto step_next; // RST 38h T:2
        case 1181: goto step_next; // RST 38h T:3
        case 1182: goto step_next; // RST 38h T:4
        case 1183: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x38;cpu->pc=cpu->wz;goto step_next; // RST 38h T:5
        case 1184: goto step_next; // RST 38h T:6
        case 1185: goto fetch_next; // RST 38h T:7
        case 1186: goto step_next; // IN B,(C) T:1
        case 1187: _wait();_ioread(cpu->bc);goto step_next; // IN B,(C) T:2
        case 1188: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;goto step_next; // IN B,(C) T:3
        case 1189: cpu->b=_z80_in(cpu,cpu->dlatch);goto fetch_next; // IN B,(C) T:4
        case 1190: _iowrite(cpu->bc,cpu->b);goto step_next; // OUT (C),B T:1
        case 1191: _wait();cpu->wz=cpu->bc+1;goto step_next; // OUT (C),B T:2
        case 1192: goto step_next; // OUT (C),B T:3
        case 1193: goto fetch_next; // OUT (C),B T:4
        case 1194: goto step_next; // SBC HL,BC T:1
        case 1195: goto step_next; // SBC HL,BC T:2
        case 1196: goto step_next; // SBC HL,BC T:3
        case 1197: goto step_next; // SBC HL,BC T:4
        case 1198: goto step_next; // SBC HL,BC T:5
        case 1199: goto step_next; // SBC HL,BC T:6
        case 1200: goto fetch_next; // SBC HL,BC T:7
        case 1201: _wait();_mread(cpu->pc++);goto step_next; // LD (nn),BC T:1
        case 1202: cpu->wzl=_gd();goto step_next; // LD (nn),BC T:2
        case 1203: goto step_next; // LD (nn),BC T:3
        case 1204: _wait();_mread(cpu->pc++);goto step_next; // LD (nn),BC T:4
        case 1205: cpu->wzh=_gd();goto step_next; // LD (nn),BC T:5
        case 1206: goto step_next; // LD (nn),BC T:6
        case 1207: _wait();_mwrite(cpu->wz++,cpu->c);goto step_next; // LD (nn),BC T:7
        case 1208: goto step_next; // LD (nn),BC T:8
        case 1209: goto step_next; // LD (nn),BC T:9
        case 1210: _wait();_mwrite(cpu->wz,cpu->b);goto step_next; // LD (nn),BC T:10
        case 1211: goto step_next; // LD (nn),BC T:11
        case 1212: goto fetch_next; // LD (nn),BC T:12
        case 1213: _wait();_mread(cpu->sp++);goto step_next; // RETN T:1
        case 1214: cpu->wzl=_gd();goto step_next; // RETN T:2
        case 1215: goto step_next; // RETN T:3
        case 1216: _wait();_mread(cpu->sp++);goto step_next; // RETN T:4
        case 1217: cpu->wzh=_gd();cpu->pc=cpu->wz;goto step_next; // RETN T:5
        case 1218: cpu->i=cpu->a;goto fetch_next; // LD I,A T:1
        case 1219: goto step_next; // IN C,(C) T:1
        case 1220: _wait();_ioread(cpu->bc);goto step_next; // IN C,(C) T:2
        case 1221: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;goto step_next; // IN C,(C) T:3
        case 1222: cpu->c=_z80_in(cpu,cpu->dlatch);goto fetch_next; // IN C,(C) T:4
        case 1223: _iowrite(cpu->bc,cpu->c);goto step_next; // OUT (C),C T:1
        case 1224: _wait();cpu->wz=cpu->bc+1;goto step_next; // OUT (C),C T:2
        case 1225: goto step_next; // OUT (C),C T:3
        case 1226: goto fetch_next; // OUT (C),C T:4
        case 1227: goto step_next; // ADC HL,BC T:1
        case 1228: goto step_next; // ADC HL,BC T:2
        case 1229: goto step_next; // ADC HL,BC T:3
        case 1230: goto step_next; // ADC HL,BC T:4
        case 1231: goto step_next; // ADC HL,BC T:5
        case 1232: goto step_next; // ADC HL,BC T:6
        case 1233: goto fetch_next; // ADC HL,BC T:7
        case 1234: _wait();_mread(cpu->pc++);goto step_next; // LD BC,(nn) T:1
        case 1235: cpu->wzl=_gd();goto step_next; // LD BC,(nn) T:2
        case 1236: goto step_next; // LD BC,(nn) T:3
        case 1237: _wait();_mread(cpu->pc++);goto step_next; // LD BC,(nn) T:4
        case 1238: cpu->wzh=_gd();goto step_next; // LD BC,(nn) T:5
        case 1239: goto step_next; // LD BC,(nn) T:6
        case 1240: _wait();_mread(cpu->wz++);goto step_next; // LD BC,(nn) T:7
        case 1241: cpu->c=_gd();goto step_next; // LD BC,(nn) T:8
        case 1242: goto step_next; // LD BC,(nn) T:9
        case 1243: _wait();_mread(cpu->wz);goto step_next; // LD BC,(nn) T:10
        case 1244: cpu->b=_gd();goto step_next; // LD BC,(nn) T:11
        case 1245: goto fetch_next; // LD BC,(nn) T:12
        case 1246: _wait();_mread(cpu->sp++);goto step_next; // RETI T:1
        case 1247: cpu->wzl=_gd();pins|=Z80_RETI;goto step_next; // RETI T:2
        case 1248: goto step_next; // RETI T:3
        case 1249: _wait();_mread(cpu->sp++);goto step_next; // RETI T:4
        case 1250: cpu->wzh=_gd();cpu->pc=cpu->wz;goto step_next; // RETI T:5
        case 1251: cpu->r=cpu->a;goto fetch_next; // LD R,A T:1
        case 1252: goto step_next; // IN D,(C) T:1
        case 1253: _wait();_ioread(cpu->bc);goto step_next; // IN D,(C) T:2
        case 1254: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;goto step_next; // IN D,(C) T:3
        case 1255: cpu->d=_z80_in(cpu,cpu->dlatch);goto fetch_next; // IN D,(C) T:4
        case 1256: _iowrite(cpu->bc,cpu->d);goto step_next; // OUT (C),D T:1
        case 1257: _wait();cpu->wz=cpu->bc+1;goto step_next; // OUT (C),D T:2
        case 1258: goto step_next; // OUT (C),D T:3
        case 1259: goto fetch_next; // OUT (C),D T:4
        case 1260: goto step_next; // SBC HL,DE T:1
        case 1261: goto step_next; // SBC HL,DE T:2
        case 1262: goto step_next; // SBC HL,DE T:3
        case 1263: goto step_next; // SBC HL,DE T:4
        case 1264: goto step_next; // SBC HL,DE T:5
        case 1265: goto step_next; // SBC HL,DE T:6
        case 1266: goto fetch_next; // SBC HL,DE T:7
        case 1267: _wait();_mread(cpu->pc++);goto step_next; // LD (nn),DE T:1
        case 1268: cpu->wzl=_gd();goto step_next; // LD (nn),DE T:2
        case 1269: goto step_next; // LD (nn),DE T:3
        case 1270: _wait();_mread(cpu->pc++);goto step_next; // LD (nn),DE T:4
        case 1271: cpu->wzh=_gd();goto step_next; // LD (nn),DE T:5
        case 1272: goto step_next; // LD (nn),DE T:6
        case 1273: _wait();_mwrite(cpu->wz++,cpu->e);goto step_next; // LD (nn),DE T:7
        case 1274: goto step_next; // LD (nn),DE T:8
        case 1275: goto step_next; // LD (nn),DE T:9
        case 1276: _wait();_mwrite(cpu->wz,cpu->d);goto step_next; // LD (nn),DE T:10
        case 1277: goto step_next; // LD (nn),DE T:11
        case 1278: goto fetch_next; // LD (nn),DE T:12
        case 1279: cpu->a=cpu->i;cpu->f=_z80_sziff2_flags(cpu, cpu->i);goto fetch_next; // LD A,I T:1
        case 1280: goto step_next; // IN E,(C) T:1
        case 1281: _wait();_ioread(cpu->bc);goto step_next; // IN E,(C) T:2
        case 1282: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;goto step_next; // IN E,(C) T:3
        case 1283: cpu->e=_z80_in(cpu,cpu->dlatch);goto fetch_next; // IN E,(C) T:4
        case 1284: _iowrite(cpu->bc,cpu->e);goto step_next; // OUT (C),E T:1
        case 1285: _wait();cpu->wz=cpu->bc+1;goto step_next; // OUT (C),E T:2
        case 1286: goto step_next; // OUT (C),E T:3
        case 1287: goto fetch_next; // OUT (C),E T:4
        case 1288: goto step_next; // ADC HL,DE T:1
        case 1289: goto step_next; // ADC HL,DE T:2
        case 1290: goto step_next; // ADC HL,DE T:3
        case 1291: goto step_next; // ADC HL,DE T:4
        case 1292: goto step_next; // ADC HL,DE T:5
        case 1293: goto step_next; // ADC HL,DE T:6
        case 1294: goto fetch_next; // ADC HL,DE T:7
        case 1295: _wait();_mread(cpu->pc++);goto step_next; // LD DE,(nn) T:1
        case 1296: cpu->wzl=_gd();goto step_next; // LD DE,(nn) T:2
        case 1297: goto step_next; // LD DE,(nn) T:3
        case 1298: _wait();_mread(cpu->pc++);goto step_next; // LD DE,(nn) T:4
        case 1299: cpu->wzh=_gd();goto step_next; // LD DE,(nn) T:5
        case 1300: goto step_next; // LD DE,(nn) T:6
        case 1301: _wait();_mread(cpu->wz++);goto step_next; // LD DE,(nn) T:7
        case 1302: cpu->e=_gd();goto step_next; // LD DE,(nn) T:8
        case 1303: goto step_next; // LD DE,(nn) T:9
        case 1304: _wait();_mread(cpu->wz);goto step_next; // LD DE,(nn) T:10
        case 1305: cpu->d=_gd();goto step_next; // LD DE,(nn) T:11
        case 1306: goto fetch_next; // LD DE,(nn) T:12
        case 1307: cpu->a=cpu->r;cpu->f=_z80_sziff2_flags(cpu, cpu->r);goto fetch_next; // LD A,R T:1
        case 1308: goto step_next; // IN H,(C) T:1
        case 1309: _wait();_ioread(cpu->bc);goto step_next; // IN H,(C) T:2
        case 1310: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;goto step_next; // IN H,(C) T:3
        case 1311: cpu->h=_z80_in(cpu,cpu->dlatch);goto fetch_next; // IN H,(C) T:4
        case 1312: _iowrite(cpu->bc,cpu->h);goto step_next; // OUT (C),H T:1
        case 1313: _wait();cpu->wz=cpu->bc+1;goto step_next; // OUT (C),H T:2
        case 1314: goto step_next; // OUT (C),H T:3
        case 1315: goto fetch_next; // OUT (C),H T:4
        case 1316: goto step_next; // SBC HL,HL T:1
        case 1317: goto step_next; // SBC HL,HL T:2
        case 1318: goto step_next; // SBC HL,HL T:3
        case 1319: goto step_next; // SBC HL,HL T:4
        case 1320: goto step_next; // SBC HL,HL T:5
        case 1321: goto step_next; // SBC HL,HL T:6
        case 1322: goto fetch_next; // SBC HL,HL T:7
        case 1323: _wait();_mread(cpu->pc++);goto step_next; // LD (nn),HL T:1
        case 1324: cpu->wzl=_gd();goto step_next; // LD (nn),HL T:2
        case 1325: goto step_next; // LD (nn),HL T:3
        case 1326: _wait();_mread(cpu->pc++);goto step_next; // LD (nn),HL T:4
        case 1327: cpu->wzh=_gd();goto step_next; // LD (nn),HL T:5
        case 1328: goto step_next; // LD (nn),HL T:6
        case 1329: _wait();_mwrite(cpu->wz++,cpu->l);goto step_next; // LD (nn),HL T:7
        case 1330: goto step_next; // LD (nn),HL T:8
        case 1331: goto step_next; // LD (nn),HL T:9
        case 1332: _wait();_mwrite(cpu->wz,cpu->h);goto step_next; // LD (nn),HL T:10
        case 1333: goto step_next; // LD (nn),HL T:11
        case 1334: goto fetch_next; // LD (nn),HL T:12
        case 1335: _wait();_mread(cpu->hl);goto step_next; // RRD T:1
        case 1336: cpu->dlatch=_gd();goto step_next; // RRD T:2
        case 1337: cpu->dlatch=_z80_rrd(cpu,cpu->dlatch);goto step_next; // RRD T:3
        case 1338: goto step_next; // RRD T:4
        case 1339: goto step_next; // RRD T:5
        case 1340: goto step_next; // RRD T:6
        case 1341: goto step_next; // RRD T:7
        case 1342: _wait();_mwrite(cpu->hl,cpu->dlatch);cpu->wz=cpu->hl+1;goto step_next; // RRD T:8
        case 1343: goto step_next; // RRD T:9
        case 1344: goto fetch_next; // RRD T:10
        case 1345: goto step_next; // IN L,(C) T:1
        case 1346: _wait();_ioread(cpu->bc);goto step_next; // IN L,(C) T:2
        case 1347: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;goto step_next; // IN L,(C) T:3
        case 1348: cpu->l=_z80_in(cpu,cpu->dlatch);goto fetch_next; // IN L,(C) T:4
        case 1349: _iowrite(cpu->bc,cpu->l);goto step_next; // OUT (C),L T:1
        case 1350: _wait();cpu->wz=cpu->bc+1;goto step_next; // OUT (C),L T:2
        case 1351: goto step_next; // OUT (C),L T:3
        case 1352: goto fetch_next; // OUT (C),L T:4
        case 1353: goto step_next; // ADC HL,HL T:1
        case 1354: goto step_next; // ADC HL,HL T:2
        case 1355: goto step_next; // ADC HL,HL T:3
        case 1356: goto step_next; // ADC HL,HL T:4
        case 1357: goto step_next; // ADC HL,HL T:5
        case 1358: goto step_next; // ADC HL,HL T:6
        case 1359: goto fetch_next; // ADC HL,HL T:7
        case 1360: _wait();_mread(cpu->pc++);goto step_next; // LD HL,(nn) T:1
        case 1361: cpu->wzl=_gd();goto step_next; // LD HL,(nn) T:2
        case 1362: goto step_next; // LD HL,(nn) T:3
        case 1363: _wait();_mread(cpu->pc++);goto step_next; // LD HL,(nn) T:4
        case 1364: cpu->wzh=_gd();goto step_next; // LD HL,(nn) T:5
        case 1365: goto step_next; // LD HL,(nn) T:6
        case 1366: _wait();_mread(cpu->wz++);goto step_next; // LD HL,(nn) T:7
        case 1367: cpu->l=_gd();goto step_next; // LD HL,(nn) T:8
        case 1368: goto step_next; // LD HL,(nn) T:9
        case 1369: _wait();_mread(cpu->wz);goto step_next; // LD HL,(nn) T:10
        case 1370: cpu->h=_gd();goto step_next; // LD HL,(nn) T:11
        case 1371: goto fetch_next; // LD HL,(nn) T:12
        case 1372: _wait();_mread(cpu->hl);goto step_next; // RLD T:1
        case 1373: cpu->dlatch=_gd();goto step_next; // RLD T:2
        case 1374: cpu->dlatch=_z80_rld(cpu,cpu->dlatch);goto step_next; // RLD T:3
        case 1375: goto step_next; // RLD T:4
        case 1376: goto step_next; // RLD T:5
        case 1377: goto step_next; // RLD T:6
        case 1378: goto step_next; // RLD T:7
        case 1379: _wait();_mwrite(cpu->hl,cpu->dlatch);cpu->wz=cpu->hl+1;goto step_next; // RLD T:8
        case 1380: goto step_next; // RLD T:9
        case 1381: goto fetch_next; // RLD T:10
        case 1382: goto step_next; // IN (C) T:1
        case 1383: _wait();_ioread(cpu->bc);goto step_next; // IN (C) T:2
        case 1384: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;goto step_next; // IN (C) T:3
        case 1385: _z80_in(cpu,cpu->dlatch);goto fetch_next; // IN (C) T:4
        case 1386: _iowrite(cpu->bc,0);goto step_next; // OUT (C),0 T:1
        case 1387: _wait();cpu->wz=cpu->bc+1;goto step_next; // OUT (C),0 T:2
        case 1388: goto step_next; // OUT (C),0 T:3
        case 1389: goto fetch_next; // OUT (C),0 T:4
        case 1390: goto step_next; // SBC HL,SP T:1
        case 1391: goto step_next; // SBC HL,SP T:2
        case 1392: goto step_next; // SBC HL,SP T:3
        case 1393: goto step_next; // SBC HL,SP T:4
        case 1394: goto step_next; // SBC HL,SP T:5
        case 1395: goto step_next; // SBC HL,SP T:6
        case 1396: goto fetch_next; // SBC HL,SP T:7
        case 1397: _wait();_mread(cpu->pc++);goto step_next; // LD (nn),SP T:1
        case 1398: cpu->wzl=_gd();goto step_next; // LD (nn),SP T:2
        case 1399: goto step_next; // LD (nn),SP T:3
        case 1400: _wait();_mread(cpu->pc++);goto step_next; // LD (nn),SP T:4
        case 1401: cpu->wzh=_gd();goto step_next; // LD (nn),SP T:5
        case 1402: goto step_next; // LD (nn),SP T:6
        case 1403: _wait();_mwrite(cpu->wz++,cpu->spl);goto step_next; // LD (nn),SP T:7
        case 1404: goto step_next; // LD (nn),SP T:8
        case 1405: goto step_next; // LD (nn),SP T:9
        case 1406: _wait();_mwrite(cpu->wz,cpu->sph);goto step_next; // LD (nn),SP T:10
        case 1407: goto step_next; // LD (nn),SP T:11
        case 1408: goto fetch_next; // LD (nn),SP T:12
        case 1409: goto step_next; // IN A,(C) T:1
        case 1410: _wait();_ioread(cpu->bc);goto step_next; // IN A,(C) T:2
        case 1411: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;goto step_next; // IN A,(C) T:3
        case 1412: cpu->a=_z80_in(cpu,cpu->dlatch);goto fetch_next; // IN A,(C) T:4
        case 1413: _iowrite(cpu->bc,cpu->a);goto step_next; // OUT (C),A T:1
        case 1414: _wait();cpu->wz=cpu->bc+1;goto step_next; // OUT (C),A T:2
        case 1415: goto step_next; // OUT (C),A T:3
        case 1416: goto fetch_next; // OUT (C),A T:4
        case 1417: goto step_next; // ADC HL,SP T:1
        case 1418: goto step_next; // ADC HL,SP T:2
        case 1419: goto step_next; // ADC HL,SP T:3
        case 1420: goto step_next; // ADC HL,SP T:4
        case 1421: goto step_next; // ADC HL,SP T:5
        case 1422: goto step_next; // ADC HL,SP T:6
        case 1423: goto fetch_next; // ADC HL,SP T:7
        case 1424: _wait();_mread(cpu->pc++);goto step_next; // LD SP,(nn) T:1
        case 1425: cpu->wzl=_gd();goto step_next; // LD SP,(nn) T:2
        case 1426: goto step_next; // LD SP,(nn) T:3
        case 1427: _wait();_mread(cpu->pc++);goto step_next; // LD SP,(nn) T:4
        case 1428: cpu->wzh=_gd();goto step_next; // LD SP,(nn) T:5
        case 1429: goto step_next; // LD SP,(nn) T:6
        case 1430: _wait();_mread(cpu->wz++);goto step_next; // LD SP,(nn) T:7
        case 1431: cpu->spl=_gd();goto step_next; // LD SP,(nn) T:8
        case 1432: goto step_next; // LD SP,(nn) T:9
        case 1433: _wait();_mread(cpu->wz);goto step_next; // LD SP,(nn) T:10
        case 1434: cpu->sph=_gd();goto step_next; // LD SP,(nn) T:11
        case 1435: goto fetch_next; // LD SP,(nn) T:12
        case 1436: _wait();_mread(cpu->hl++);goto step_next; // LDI T:1
        case 1437: cpu->dlatch=_gd();goto step_next; // LDI T:2
        case 1438: goto step_next; // LDI T:3
        case 1439: _wait();_mwrite(cpu->de++,cpu->dlatch);goto step_next; // LDI T:4
        case 1440: goto step_next; // LDI T:5
        case 1441: _z80_ldi_ldd(cpu,cpu->dlatch);goto step_next; // LDI T:6
        case 1442: goto step_next; // LDI T:7
        case 1443: goto fetch_next; // LDI T:8
        case 1444: _wait();_mread(cpu->hl++);goto step_next; // CPI T:1
        case 1445: cpu->dlatch=_gd();goto step_next; // CPI T:2
        case 1446: cpu->wz++;_z80_cpi_cpd(cpu,cpu->dlatch);goto step_next; // CPI T:3
        case 1447: goto step_next; // CPI T:4
        case 1448: goto step_next; // CPI T:5
        case 1449: goto step_next; // CPI T:6
        case 1450: goto step_next; // CPI T:7
        case 1451: goto fetch_next; // CPI T:8
        case 1452: goto step_next; // INI T:1
        case 1453: goto step_next; // INI T:2
        case 1454: _wait();_ioread(cpu->bc);goto step_next; // INI T:3
        case 1455: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;cpu->b--;;goto step_next; // INI T:4
        case 1456: goto step_next; // INI T:5
        case 1457: _wait();_mwrite(cpu->hl++,cpu->dlatch);_z80_ini_ind(cpu,cpu->dlatch,cpu->c+1);goto step_next; // INI T:6
        case 1458: goto step_next; // INI T:7
        case 1459: goto fetch_next; // INI T:8
        case 1460: goto step_next; // OUTI T:1
        case 1461: _wait();_mread(cpu->hl++);goto step_next; // OUTI T:2
        case 1462: cpu->dlatch=_gd();cpu->b--;goto step_next; // OUTI T:3
        case 1463: goto step_next; // OUTI T:4
        case 1464: _iowrite(cpu->bc,cpu->dlatch);goto step_next; // OUTI T:5
        case 1465: _wait();cpu->wz=cpu->bc+1;_z80_outi_outd(cpu,cpu->dlatch);goto step_next; // OUTI T:6
        case 1466: goto step_next; // OUTI T:7
        case 1467: goto fetch_next; // OUTI T:8
        case 1468: _wait();_mread(cpu->hl--);goto step_next; // LDD T:1
        case 1469: cpu->dlatch=_gd();goto step_next; // LDD T:2
        case 1470: goto step_next; // LDD T:3
        case 1471: _wait();_mwrite(cpu->de--,cpu->dlatch);goto step_next; // LDD T:4
        case 1472: goto step_next; // LDD T:5
        case 1473: _z80_ldi_ldd(cpu,cpu->dlatch);goto step_next; // LDD T:6
        case 1474: goto step_next; // LDD T:7
        case 1475: goto fetch_next; // LDD T:8
        case 1476: _wait();_mread(cpu->hl--);goto step_next; // CPD T:1
        case 1477: cpu->dlatch=_gd();goto step_next; // CPD T:2
        case 1478: cpu->wz--;_z80_cpi_cpd(cpu,cpu->dlatch);goto step_next; // CPD T:3
        case 1479: goto step_next; // CPD T:4
        case 1480: goto step_next; // CPD T:5
        case 1481: goto step_next; // CPD T:6
        case 1482: goto step_next; // CPD T:7
        case 1483: goto fetch_next; // CPD T:8
        case 1484: goto step_next; // IND T:1
        case 1485: goto step_next; // IND T:2
        case 1486: _wait();_ioread(cpu->bc);goto step_next; // IND T:3
        case 1487: cpu->dlatch=_gd();cpu->wz=cpu->bc-1;cpu->b--;;goto step_next; // IND T:4
        case 1488: goto step_next; // IND T:5
        case 1489: _wait();_mwrite(cpu->hl--,cpu->dlatch);_z80_ini_ind(cpu,cpu->dlatch,cpu->c-1);goto step_next; // IND T:6
        case 1490: goto step_next; // IND T:7
        case 1491: goto fetch_next; // IND T:8
        case 1492: goto step_next; // OUTD T:1
        case 1493: _wait();_mread(cpu->hl--);goto step_next; // OUTD T:2
        case 1494: cpu->dlatch=_gd();cpu->b--;goto step_next; // OUTD T:3
        case 1495: goto step_next; // OUTD T:4
        case 1496: _iowrite(cpu->bc,cpu->dlatch);goto step_next; // OUTD T:5
        case 1497: _wait();cpu->wz=cpu->bc-1;_z80_outi_outd(cpu,cpu->dlatch);goto step_next; // OUTD T:6
        case 1498: goto step_next; // OUTD T:7
        case 1499: goto fetch_next; // OUTD T:8
        case 1500: _wait();_mread(cpu->hl++);goto step_next; // LDIR T:1
        case 1501: cpu->dlatch=_gd();goto step_next; // LDIR T:2
        case 1502: goto step_next; // LDIR T:3
        case 1503: _wait();_mwrite(cpu->de++,cpu->dlatch);goto step_next; // LDIR T:4
        case 1504: goto step_next; // LDIR T:5
        case 1505: if(!_z80_ldi_ldd(cpu,cpu->dlatch)){_skip(5);};goto step_next; // LDIR T:6
        case 1506: goto step_next; // LDIR T:7
        case 1507: cpu->wz=--cpu->pc;--cpu->pc;;goto step_next; // LDIR T:8
        case 1508: goto step_next; // LDIR T:9
        case 1509: goto step_next; // LDIR T:10
        case 1510: goto step_next; // LDIR T:11
        case 1511: goto step_next; // LDIR T:12
        case 1512: goto fetch_next; // LDIR T:13
        case 1513: _wait();_mread(cpu->hl++);goto step_next; // CPIR T:1
        case 1514: cpu->dlatch=_gd();goto step_next; // CPIR T:2
        case 1515: cpu->wz++;if(!_z80_cpi_cpd(cpu,cpu->dlatch)){_skip(5);};goto step_next; // CPIR T:3
        case 1516: goto step_next; // CPIR T:4
        case 1517: goto step_next; // CPIR T:5
        case 1518: goto step_next; // CPIR T:6
        case 1519: goto step_next; // CPIR T:7
        case 1520: cpu->wz=--cpu->pc;--cpu->pc;goto step_next; // CPIR T:8
        case 1521: goto step_next; // CPIR T:9
        case 1522: goto step_next; // CPIR T:10
        case 1523: goto step_next; // CPIR T:11
        case 1524: goto step_next; // CPIR T:12
        case 1525: goto fetch_next; // CPIR T:13
        case 1526: goto step_next; // INIR T:1
        case 1527: goto step_next; // INIR T:2
        case 1528: _wait();_ioread(cpu->bc);goto step_next; // INIR T:3
        case 1529: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;cpu->b--;;goto step_next; // INIR T:4
        case 1530: goto step_next; // INIR T:5
        case 1531: _wait();_mwrite(cpu->hl++,cpu->dlatch);if (!_z80_ini_ind(cpu,cpu->dlatch,cpu->c+1)){_skip(5);};goto step_next; // INIR T:6
        case 1532: goto step_next; // INIR T:7
        case 1533: cpu->wz=--cpu->pc;--cpu->pc;goto step_next; // INIR T:8
        case 1534: goto step_next; // INIR T:9
        case 1535: goto step_next; // INIR T:10
        case 1536: goto step_next; // INIR T:11
        case 1537: goto step_next; // INIR T:12
        case 1538: goto fetch_next; // INIR T:13
        case 1539: goto step_next; // OTIR T:1
        case 1540: _wait();_mread(cpu->hl++);goto step_next; // OTIR T:2
        case 1541: cpu->dlatch=_gd();cpu->b--;goto step_next; // OTIR T:3
        case 1542: goto step_next; // OTIR T:4
        case 1543: _iowrite(cpu->bc,cpu->dlatch);goto step_next; // OTIR T:5
        case 1544: _wait();cpu->wz=cpu->bc+1;if(!_z80_outi_outd(cpu,cpu->dlatch)){_skip(5);};goto step_next; // OTIR T:6
        case 1545: goto step_next; // OTIR T:7
        case 1546: cpu->wz=--cpu->pc;--cpu->pc;goto step_next; // OTIR T:8
        case 1547: goto step_next; // OTIR T:9
        case 1548: goto step_next; // OTIR T:10
        case 1549: goto step_next; // OTIR T:11
        case 1550: goto step_next; // OTIR T:12
        case 1551: goto fetch_next; // OTIR T:13
        case 1552: _wait();_mread(cpu->hl--);goto step_next; // LDDR T:1
        case 1553: cpu->dlatch=_gd();goto step_next; // LDDR T:2
        case 1554: goto step_next; // LDDR T:3
        case 1555: _wait();_mwrite(cpu->de--,cpu->dlatch);goto step_next; // LDDR T:4
        case 1556: goto step_next; // LDDR T:5
        case 1557: if(!_z80_ldi_ldd(cpu,cpu->dlatch)){_skip(5);};goto step_next; // LDDR T:6
        case 1558: goto step_next; // LDDR T:7
        case 1559: cpu->wz=--cpu->pc;--cpu->pc;;goto step_next; // LDDR T:8
        case 1560: goto step_next; // LDDR T:9
        case 1561: goto step_next; // LDDR T:10
        case 1562: goto step_next; // LDDR T:11
        case 1563: goto step_next; // LDDR T:12
        case 1564: goto fetch_next; // LDDR T:13
        case 1565: _wait();_mread(cpu->hl--);goto step_next; // CPDR T:1
        case 1566: cpu->dlatch=_gd();goto step_next; // CPDR T:2
        case 1567: cpu->wz--;if(!_z80_cpi_cpd(cpu,cpu->dlatch)){_skip(5);};goto step_next; // CPDR T:3
        case 1568: goto step_next; // CPDR T:4
        case 1569: goto step_next; // CPDR T:5
        case 1570: goto step_next; // CPDR T:6
        case 1571: goto step_next; // CPDR T:7
        case 1572: cpu->wz=--cpu->pc;--cpu->pc;goto step_next; // CPDR T:8
        case 1573: goto step_next; // CPDR T:9
        case 1574: goto step_next; // CPDR T:10
        case 1575: goto step_next; // CPDR T:11
        case 1576: goto step_next; // CPDR T:12
        case 1577: goto fetch_next; // CPDR T:13
        case 1578: goto step_next; // INDR T:1
        case 1579: goto step_next; // INDR T:2
        case 1580: _wait();_ioread(cpu->bc);goto step_next; // INDR T:3
        case 1581: cpu->dlatch=_gd();cpu->wz=cpu->bc-1;cpu->b--;;goto step_next; // INDR T:4
        case 1582: goto step_next; // INDR T:5
        case 1583: _wait();_mwrite(cpu->hl--,cpu->dlatch);if (!_z80_ini_ind(cpu,cpu->dlatch,cpu->c-1)){_skip(5);};goto step_next; // INDR T:6
        case 1584: goto step_next; // INDR T:7
        case 1585: cpu->wz=--cpu->pc;--cpu->pc;goto step_next; // INDR T:8
        case 1586: goto step_next; // INDR T:9
        case 1587: goto step_next; // INDR T:10
        case 1588: goto step_next; // INDR T:11
        case 1589: goto step_next; // INDR T:12
        case 1590: goto fetch_next; // INDR T:13
        case 1591: goto step_next; // OTDR T:1
        case 1592: _wait();_mread(cpu->hl--);goto step_next; // OTDR T:2
        case 1593: cpu->dlatch=_gd();cpu->b--;goto step_next; // OTDR T:3
        case 1594: goto step_next; // OTDR T:4
        case 1595: _iowrite(cpu->bc,cpu->dlatch);goto step_next; // OTDR T:5
        case 1596: _wait();cpu->wz=cpu->bc-1;if(!_z80_outi_outd(cpu,cpu->dlatch)){_skip(5);};goto step_next; // OTDR T:6
        case 1597: goto step_next; // OTDR T:7
        case 1598: cpu->wz=--cpu->pc;--cpu->pc;goto step_next; // OTDR T:8
        case 1599: goto step_next; // OTDR T:9
        case 1600: goto step_next; // OTDR T:10
        case 1601: goto step_next; // OTDR T:11
        case 1602: goto step_next; // OTDR T:12
        case 1603: goto fetch_next; // OTDR T:13
        case 1604: {uint8_t z=cpu->opcode&7;_z80_cb_action(cpu,z,z);};goto fetch_next; // cb T:0
        case 1605: goto step_next; // cbhl T:0
        case 1606: _wait();_mread(cpu->hl);goto step_next; // cbhl T:1
        case 1607: cpu->dlatch=_gd();if(!_z80_cb_action(cpu,6,6)){_skip(3);};goto step_next; // cbhl T:2
        case 1608: goto step_next; // cbhl T:3
        case 1609: goto step_next; // cbhl T:4
        case 1610: _wait();_mwrite(cpu->hl,cpu->dlatch);goto step_next; // cbhl T:5
        case 1611: goto step_next; // cbhl T:6
        case 1612: goto fetch_next; // cbhl T:7
        case 1613: _wait();_mread(cpu->pc++);goto step_next; // ddfdcb T:0
        case 1614: _z80_ddfdcb_addr(cpu,pins);goto step_next; // ddfdcb T:1
        case 1615: goto step_next; // ddfdcb T:2
        case 1616: _wait();_mread(cpu->pc++);goto step_next; // ddfdcb T:3
        case 1617: cpu->opcode=_gd();goto step_next; // ddfdcb T:4
        case 1618: goto step_next; // ddfdcb T:5
        case 1619: goto step_next; // ddfdcb T:6
        case 1620: goto step_next; // ddfdcb T:7
        case 1621: _wait();_mread(cpu->addr);goto step_next; // ddfdcb T:8
        case 1622: cpu->dlatch=_gd();if(!_z80_cb_action(cpu,6,cpu->opcode&7)){_skip(3);};goto step_next; // ddfdcb T:9
        case 1623: goto step_next; // ddfdcb T:10
        case 1624: goto step_next; // ddfdcb T:11
        case 1625: _wait();_mwrite(cpu->addr,cpu->dlatch);goto step_next; // ddfdcb T:12
        case 1626: goto step_next; // ddfdcb T:13
        case 1627: goto fetch_next; // ddfdcb T:14
        case 1628: cpu->iff1=cpu->iff2=false;goto step_next; // int_im0 T:0
        case 1629: pins|=(Z80_M1|Z80_IORQ);goto step_next; // int_im0 T:1
        case 1630: _wait();cpu->opcode=_z80_get_db(pins);goto step_next; // int_im0 T:2
        case 1631: pins=_z80_refresh(cpu,pins);goto step_next; // int_im0 T:3
        case 1632: cpu->step=cpu->opcode; cpu->addr=cpu->hl;goto step_next; // int_im0 T:4
        case 1633: goto fetch_next; // int_im0 T:5
        case 1634: cpu->iff1=cpu->iff2=false;goto step_next; // int_im1 T:0
        case 1635: pins|=(Z80_M1|Z80_IORQ);goto step_next; // int_im1 T:1
        case 1636: _wait();goto step_next; // int_im1 T:2
        case 1637: pins=_z80_refresh(cpu,pins);goto step_next; // int_im1 T:3
        case 1638: goto step_next; // int_im1 T:4
        case 1639: goto step_next; // int_im1 T:5
        case 1640: goto step_next; // int_im1 T:6
        case 1641: _wait();_mwrite(--cpu->sp,cpu->pch);goto step_next; // int_im1 T:7
        case 1642: goto step_next; // int_im1 T:8
        case 1643: goto step_next; // int_im1 T:9
        case 1644: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=cpu->pc=0x0038;goto step_next; // int_im1 T:10
        case 1645: goto step_next; // int_im1 T:11
        case 1646: goto fetch_next; // int_im1 T:12
        case 1647: cpu->iff1=cpu->iff2=false;goto step_next; // int_im2 T:0
        case 1648: pins|=(Z80_M1|Z80_IORQ);goto step_next; // int_im2 T:1
        case 1649: _wait();cpu->dlatch=_z80_get_db(pins);goto step_next; // int_im2 T:2
        case 1650: pins=_z80_refresh(cpu,pins);goto step_next; // int_im2 T:3
        case 1651: goto step_next; // int_im2 T:4
        case 1652: goto step_next; // int_im2 T:5
        case 1653: goto step_next; // int_im2 T:6
        case 1654: _wait();_mwrite(--cpu->sp,cpu->pch);goto step_next; // int_im2 T:7
        case 1655: goto step_next; // int_im2 T:8
        case 1656: goto step_next; // int_im2 T:9
        case 1657: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wzl=cpu->dlatch;cpu->wzh=cpu->i;goto step_next; // int_im2 T:10
        case 1658: goto step_next; // int_im2 T:11
        case 1659: goto step_next; // int_im2 T:12
        case 1660: _wait();_mread(cpu->wz++);goto step_next; // int_im2 T:13
        case 1661: cpu->dlatch=_gd();goto step_next; // int_im2 T:14
        case 1662: goto step_next; // int_im2 T:15
        case 1663: _wait();_mread(cpu->wz);goto step_next; // int_im2 T:16
        case 1664: cpu->wzh=_gd();cpu->wzl=cpu->dlatch;cpu->pc=cpu->wz;goto step_next; // int_im2 T:17
        case 1665: goto fetch_next; // int_im2 T:18
        case 1666: _wait();cpu->iff1=false;goto step_next; // nmi T:0
        case 1667: pins=_z80_refresh(cpu,pins);goto step_next; // nmi T:1
        case 1668: goto step_next; // nmi T:2
        case 1669: goto step_next; // nmi T:3
        case 1670: goto step_next; // nmi T:4
        case 1671: _wait();_mwrite(--cpu->sp,cpu->pch);goto step_next; // nmi T:5
        case 1672: goto step_next; // nmi T:6
        case 1673: goto step_next; // nmi T:7
        case 1674: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=cpu->pc=0x0066;goto step_next; // nmi T:8
        case 1675: goto step_next; // nmi T:9
        case 1676: goto fetch_next; // nmi T:10
        // %>
        //=== shared fetch machine cycle for non-DD/FD-prefixed ops
        case Z80_M1_T2: _wait(); cpu->opcode = _gd(); goto step_next;
        case Z80_M1_T3: pins = _z80_refresh(cpu, pins); goto step_next;
        case Z80_M1_T4:
            cpu->step = cpu->opcode;
            cpu->addr = cpu->hl;
            goto step_to;
        //=== shared fetch machine cycle for DD/FD-prefixed ops
        case Z80_DDFD_M1_T2: _wait(); cpu->opcode = _gd(); goto step_next;
        case Z80_DDFD_M1_T3: pins = _z80_refresh(cpu, pins); goto step_next;
        case Z80_DDFD_M1_T4:
            cpu->step = _z80_indirect_table[cpu->opcode] ? Z80_DDFD_D_T1 : cpu->opcode;
            cpu->addr = cpu->hlx[cpu->hlx_idx].hl;
            goto step_to;
        //=== optional d-loading cycle for (IX+d), (IY+d)
        case Z80_DDFD_D_T1: goto step_next;
        case Z80_DDFD_D_T2: _wait(); _mread(cpu->pc++); goto step_next;
        case Z80_DDFD_D_T3: cpu->addr += (int8_t)_gd(); cpu->wz = cpu->addr; goto step_next;
        //--- special case LD (IX/IY+d),n or filler ticks
        case Z80_DDFD_D_T4: goto step_next;
        case Z80_DDFD_D_T5: if (cpu->opcode == 0x36) { _wait();_mread(cpu->pc++); }; goto step_next;
        case Z80_DDFD_D_T6: if (cpu->opcode == 0x36) { cpu->dlatch = _gd(); }; goto step_next;
        case Z80_DDFD_D_T7: goto step_next;
        case Z80_DDFD_D_T8: cpu->step = (cpu->opcode==0x36) ? Z80_DDFD_LDHLN_WR_T1 : cpu->opcode; goto step_to;
        //--- special case LD (IX/IY+d),n write mcycle
        case Z80_DDFD_LDHLN_WR_T1: goto step_next;
        case Z80_DDFD_LDHLN_WR_T2: _wait(); _mwrite(cpu->addr,cpu->dlatch); goto step_next;
        case Z80_DDFD_LDHLN_WR_T3: goto step_next;
        case Z80_DDFD_LDHLN_OVERLAPPED: goto fetch_next;
        //=== special opcode fetch machine cycle for ED-prefixed instructions
        case Z80_ED_M1_T2: _wait(); cpu->opcode = _gd(); goto step_next;
        case Z80_ED_M1_T3: pins = _z80_refresh(cpu, pins); goto step_next;
        case Z80_ED_M1_T4: cpu->step = cpu->opcode + 256; goto step_to;
        //=== special opcode fetch machine cycle for CB-prefixed instructions
        case Z80_CB_M1_T2: _wait(); cpu->opcode = _gd(); goto step_next;
        case Z80_CB_M1_T3: pins = _z80_refresh(cpu, pins); goto step_next;
        case Z80_CB_M1_T4:
            if ((cpu->opcode & 7) == 6) {
                // this is a (HL) instruction
                cpu->addr = cpu->hl;
                cpu->step = Z80_CBHL_STEP;
            }
            else {
                cpu->step = Z80_CB_STEP;
            }
            goto step_to;
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
