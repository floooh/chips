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
    * RFSH  <---|           |<--> ... *
    *           |           |<--> D7  *
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
                else if (pins & Z80_RD) {
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
                else if (pins & Z80_RD) {
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
#define Z80_A0  (1ULL<<0)
#define Z80_A1  (1ULL<<1)
#define Z80_A2  (1ULL<<2)
#define Z80_A3  (1ULL<<3)
#define Z80_A4  (1ULL<<4)
#define Z80_A5  (1ULL<<5)
#define Z80_A6  (1ULL<<6)
#define Z80_A7  (1ULL<<7)
#define Z80_A8  (1ULL<<8)
#define Z80_A9  (1ULL<<9)
#define Z80_A10 (1ULL<<10)
#define Z80_A11 (1ULL<<11)
#define Z80_A12 (1ULL<<12)
#define Z80_A13 (1ULL<<13)
#define Z80_A14 (1ULL<<14)
#define Z80_A15 (1ULL<<15)

// data pins
#define Z80_D0  (1ULL<<16)
#define Z80_D1  (1ULL<<17)
#define Z80_D2  (1ULL<<18)
#define Z80_D3  (1ULL<<19)
#define Z80_D4  (1ULL<<20)
#define Z80_D5  (1ULL<<21)
#define Z80_D6  (1ULL<<22)
#define Z80_D7  (1ULL<<23)

// control pins
#define Z80_M1    (1ULL<<24)        // machine cycle 1
#define Z80_MREQ  (1ULL<<25)        // memory request
#define Z80_IORQ  (1ULL<<26)        // input/output request
#define Z80_RD    (1ULL<<27)        // read
#define Z80_WR    (1ULL<<28)        // write
#define Z80_HALT  (1ULL<<29)        // halt state
#define Z80_INT   (1ULL<<30)        // interrupt request
#define Z80_RES   (1ULL<<31)        // reset requested
#define Z80_NMI   (1ULL<<32)        // non-maskable interrupt
#define Z80_WAIT  (1ULL<<33)        // wait requested
#define Z80_RFSH  (1ULL<<34)        // refresh

// virtual pins (for interrupt daisy chain protocol)
#define Z80_IEIO    (1ULL<<37)      // unified daisy chain 'Interrupt Enable In+Out'
#define Z80_RETI    (1ULL<<38)      // cpu has decoded a RETI instruction

#define Z80_CTRL_PIN_MASK (Z80_M1|Z80_MREQ|Z80_IORQ|Z80_RD|Z80_WR|Z80_RFSH)
#define Z80_PIN_MASK ((1ULL<<40)-1)

// pin access helper macros
#define Z80_MAKE_PINS(ctrl, addr, data) ((ctrl)|(((data)<<16)&0xFF0000ULL)|((addr)&0xFFFFULL))
#define Z80_GET_ADDR(p) ((uint16_t)((p)&0xFFFF))
#define Z80_SET_ADDR(p,a) {p=((p)&~0xFFFF)|((a)&0xFFFF);}
#define Z80_GET_DATA(p) ((uint8_t)(((p)>>16)&0xFF))
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

typedef struct {
    uint16_t step;  // first or current decoder switch-case branch step
    uint16_t flags; // _Z80_OPSTATE_FLAGS_
} z80_opstate_t;

// CPU state
typedef struct {
    z80_opstate_t op;       // the currently active op
    uint64_t pins;          // last pin state, used for NMI detection
    uint64_t int_bits;      // track INT and NMI state
    union {
        struct {
            uint16_t prefix_offset; // opstate table offset: 0x100 on ED prefix, 0x200 on CB prefix
            uint8_t hlx_idx;        // index into hlx[] for mapping hl to ix or iy (0: hl, 1: ix, 2: iy)
            uint8_t prefix;         // one of _Z80_PREFIX_*
        };
        uint32_t prefix_state;
    };

    union { struct { uint8_t pcl; uint8_t pch; }; uint16_t pc; };
    uint16_t addr;      // effective address for (HL),(IX+d),(IY+d)
    uint8_t opcode;     // current opcode
    uint8_t dlatch;     // temporary store for data bus value

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
    uint8_t im;
    bool iff1, iff2;
    uint16_t af2, bc2, de2, hl2; // shadow register bank
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

// flags for z80_opstate.flags
#define _Z80_OPSTATE_FLAGS_INDIRECT  (1<<0)  // this is a (HL)/(IX+d)/(IY+d) instruction
#define _Z80_OPSTATE_FLAGS_IMM8 (1<<1)       // this is an 8-bit immediate load instruction

// values for hlx_idx for mapping HL, IX or IY, used as index into hlx[]
#define _Z80_MAP_HL (0)
#define _Z80_MAP_IX (1)
#define _Z80_MAP_IY (2)

// currently active prefix
#define _Z80_PREFIX_NONE (0)
#define _Z80_PREFIX_CB   (1<<0)
#define _Z80_PREFIX_DD   (1<<1)
#define _Z80_PREFIX_ED   (1<<2)
#define _Z80_PREFIX_FD   (1<<3)

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
    return (0 == cpu->op.step) && (cpu->prefix == 0);
}

static inline void _z80_skip(z80_t* cpu, int steps) {
    cpu->op.step += steps;
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
    if ((res & 0xF) > (cpu->a & 0xF)) {
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
    return pins>>16;
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

// load the opcode from data bus for DD+CB/FD+CB instructions
static inline void _z80_ddfdcb_opcode(z80_t* cpu, uint8_t oc) {
    cpu->opcode = oc;
}

// special case opstate table slots
#define _Z80_OPSTATE_SLOT_CB        (512)
#define _Z80_OPSTATE_SLOT_CBHL      (512+1)
#define _Z80_OPSTATE_SLOT_DDFDCB    (512+2)
#define _Z80_OPSTATE_SLOT_INT_IM0   (512+3)
#define _Z80_OPSTATE_SLOT_INT_IM1   (512+4)
#define _Z80_OPSTATE_SLOT_INT_IM2   (512+5)
#define _Z80_OPSTATE_SLOT_NMI       (512+6)

#define _Z80_OPSTATE_NUM_SPECIAL_OPS (7)

static const z80_opstate_t _z80_opstate_table[2*256 + _Z80_OPSTATE_NUM_SPECIAL_OPS] = {
    {   21, 0 },  //  00: nop (M:1 T:4 steps:1)
    {   22, 0 },  //  01: ld bc,nn (M:3 T:10 steps:7)
    {   29, 0 },  //  02: ld (bc),a (M:2 T:7 steps:4)
    {   33, 0 },  //  03: inc bc (M:2 T:6 steps:3)
    {   36, 0 },  //  04: inc b (M:1 T:4 steps:1)
    {   37, 0 },  //  05: dec b (M:1 T:4 steps:1)
    {   38, _Z80_OPSTATE_FLAGS_IMM8 },  //  06: ld b,n (M:2 T:7 steps:4)
    {   42, 0 },  //  07: rlca (M:1 T:4 steps:1)
    {   43, 0 },  //  08: ex af,af' (M:1 T:4 steps:1)
    {   44, 0 },  //  09: add hl,bc (M:2 T:11 steps:8)
    {   52, 0 },  //  0A: ld a,(bc) (M:2 T:7 steps:4)
    {   56, 0 },  //  0B: dec bc (M:2 T:6 steps:3)
    {   59, 0 },  //  0C: inc c (M:1 T:4 steps:1)
    {   60, 0 },  //  0D: dec c (M:1 T:4 steps:1)
    {   61, _Z80_OPSTATE_FLAGS_IMM8 },  //  0E: ld c,n (M:2 T:7 steps:4)
    {   65, 0 },  //  0F: rrca (M:1 T:4 steps:1)
    {   66, 0 },  //  10: djnz d (M:4 T:13 steps:10)
    {   76, 0 },  //  11: ld de,nn (M:3 T:10 steps:7)
    {   83, 0 },  //  12: ld (de),a (M:2 T:7 steps:4)
    {   87, 0 },  //  13: inc de (M:2 T:6 steps:3)
    {   90, 0 },  //  14: inc d (M:1 T:4 steps:1)
    {   91, 0 },  //  15: dec d (M:1 T:4 steps:1)
    {   92, _Z80_OPSTATE_FLAGS_IMM8 },  //  16: ld d,n (M:2 T:7 steps:4)
    {   96, 0 },  //  17: rla (M:1 T:4 steps:1)
    {   97, 0 },  //  18: jr d (M:3 T:12 steps:9)
    {  106, 0 },  //  19: add hl,de (M:2 T:11 steps:8)
    {  114, 0 },  //  1A: ld a,(de) (M:2 T:7 steps:4)
    {  118, 0 },  //  1B: dec de (M:2 T:6 steps:3)
    {  121, 0 },  //  1C: inc e (M:1 T:4 steps:1)
    {  122, 0 },  //  1D: dec e (M:1 T:4 steps:1)
    {  123, _Z80_OPSTATE_FLAGS_IMM8 },  //  1E: ld e,n (M:2 T:7 steps:4)
    {  127, 0 },  //  1F: rra (M:1 T:4 steps:1)
    {  128, 0 },  //  20: jr nz,d (M:3 T:12 steps:9)
    {  137, 0 },  //  21: ld hl,nn (M:3 T:10 steps:7)
    {  144, 0 },  //  22: ld (nn),hl (M:5 T:16 steps:13)
    {  157, 0 },  //  23: inc hl (M:2 T:6 steps:3)
    {  160, 0 },  //  24: inc h (M:1 T:4 steps:1)
    {  161, 0 },  //  25: dec h (M:1 T:4 steps:1)
    {  162, _Z80_OPSTATE_FLAGS_IMM8 },  //  26: ld h,n (M:2 T:7 steps:4)
    {  166, 0 },  //  27: daa (M:1 T:4 steps:1)
    {  167, 0 },  //  28: jr z,d (M:3 T:12 steps:9)
    {  176, 0 },  //  29: add hl,hl (M:2 T:11 steps:8)
    {  184, 0 },  //  2A: ld hl,(nn) (M:5 T:16 steps:13)
    {  197, 0 },  //  2B: dec hl (M:2 T:6 steps:3)
    {  200, 0 },  //  2C: inc l (M:1 T:4 steps:1)
    {  201, 0 },  //  2D: dec l (M:1 T:4 steps:1)
    {  202, _Z80_OPSTATE_FLAGS_IMM8 },  //  2E: ld l,n (M:2 T:7 steps:4)
    {  206, 0 },  //  2F: cpl (M:1 T:4 steps:1)
    {  207, 0 },  //  30: jr nc,d (M:3 T:12 steps:9)
    {  216, 0 },  //  31: ld sp,nn (M:3 T:10 steps:7)
    {  223, 0 },  //  32: ld (nn),a (M:4 T:13 steps:10)
    {  233, 0 },  //  33: inc sp (M:2 T:6 steps:3)
    {  236, _Z80_OPSTATE_FLAGS_INDIRECT },  //  34: inc (hl) (M:3 T:11 steps:8)
    {  244, _Z80_OPSTATE_FLAGS_INDIRECT },  //  35: dec (hl) (M:3 T:11 steps:8)
    {  252, _Z80_OPSTATE_FLAGS_INDIRECT|_Z80_OPSTATE_FLAGS_IMM8 },  //  36: ld (hl),n (M:3 T:10 steps:7)
    {  259, 0 },  //  37: scf (M:1 T:4 steps:1)
    {  260, 0 },  //  38: jr c,d (M:3 T:12 steps:9)
    {  269, 0 },  //  39: add hl,sp (M:2 T:11 steps:8)
    {  277, 0 },  //  3A: ld a,(nn) (M:4 T:13 steps:10)
    {  287, 0 },  //  3B: dec sp (M:2 T:6 steps:3)
    {  290, 0 },  //  3C: inc a (M:1 T:4 steps:1)
    {  291, 0 },  //  3D: dec a (M:1 T:4 steps:1)
    {  292, _Z80_OPSTATE_FLAGS_IMM8 },  //  3E: ld a,n (M:2 T:7 steps:4)
    {  296, 0 },  //  3F: ccf (M:1 T:4 steps:1)
    {  297, 0 },  //  40: ld b,b (M:1 T:4 steps:1)
    {  298, 0 },  //  41: ld b,c (M:1 T:4 steps:1)
    {  299, 0 },  //  42: ld b,d (M:1 T:4 steps:1)
    {  300, 0 },  //  43: ld b,e (M:1 T:4 steps:1)
    {  301, 0 },  //  44: ld b,h (M:1 T:4 steps:1)
    {  302, 0 },  //  45: ld b,l (M:1 T:4 steps:1)
    {  303, _Z80_OPSTATE_FLAGS_INDIRECT },  //  46: ld b,(hl) (M:2 T:7 steps:4)
    {  307, 0 },  //  47: ld b,a (M:1 T:4 steps:1)
    {  308, 0 },  //  48: ld c,b (M:1 T:4 steps:1)
    {  309, 0 },  //  49: ld c,c (M:1 T:4 steps:1)
    {  310, 0 },  //  4A: ld c,d (M:1 T:4 steps:1)
    {  311, 0 },  //  4B: ld c,e (M:1 T:4 steps:1)
    {  312, 0 },  //  4C: ld c,h (M:1 T:4 steps:1)
    {  313, 0 },  //  4D: ld c,l (M:1 T:4 steps:1)
    {  314, _Z80_OPSTATE_FLAGS_INDIRECT },  //  4E: ld c,(hl) (M:2 T:7 steps:4)
    {  318, 0 },  //  4F: ld c,a (M:1 T:4 steps:1)
    {  319, 0 },  //  50: ld d,b (M:1 T:4 steps:1)
    {  320, 0 },  //  51: ld d,c (M:1 T:4 steps:1)
    {  321, 0 },  //  52: ld d,d (M:1 T:4 steps:1)
    {  322, 0 },  //  53: ld d,e (M:1 T:4 steps:1)
    {  323, 0 },  //  54: ld d,h (M:1 T:4 steps:1)
    {  324, 0 },  //  55: ld d,l (M:1 T:4 steps:1)
    {  325, _Z80_OPSTATE_FLAGS_INDIRECT },  //  56: ld d,(hl) (M:2 T:7 steps:4)
    {  329, 0 },  //  57: ld d,a (M:1 T:4 steps:1)
    {  330, 0 },  //  58: ld e,b (M:1 T:4 steps:1)
    {  331, 0 },  //  59: ld e,c (M:1 T:4 steps:1)
    {  332, 0 },  //  5A: ld e,d (M:1 T:4 steps:1)
    {  333, 0 },  //  5B: ld e,e (M:1 T:4 steps:1)
    {  334, 0 },  //  5C: ld e,h (M:1 T:4 steps:1)
    {  335, 0 },  //  5D: ld e,l (M:1 T:4 steps:1)
    {  336, _Z80_OPSTATE_FLAGS_INDIRECT },  //  5E: ld e,(hl) (M:2 T:7 steps:4)
    {  340, 0 },  //  5F: ld e,a (M:1 T:4 steps:1)
    {  341, 0 },  //  60: ld h,b (M:1 T:4 steps:1)
    {  342, 0 },  //  61: ld h,c (M:1 T:4 steps:1)
    {  343, 0 },  //  62: ld h,d (M:1 T:4 steps:1)
    {  344, 0 },  //  63: ld h,e (M:1 T:4 steps:1)
    {  345, 0 },  //  64: ld h,h (M:1 T:4 steps:1)
    {  346, 0 },  //  65: ld h,l (M:1 T:4 steps:1)
    {  347, _Z80_OPSTATE_FLAGS_INDIRECT },  //  66: ld h,(hl) (M:2 T:7 steps:4)
    {  351, 0 },  //  67: ld h,a (M:1 T:4 steps:1)
    {  352, 0 },  //  68: ld l,b (M:1 T:4 steps:1)
    {  353, 0 },  //  69: ld l,c (M:1 T:4 steps:1)
    {  354, 0 },  //  6A: ld l,d (M:1 T:4 steps:1)
    {  355, 0 },  //  6B: ld l,e (M:1 T:4 steps:1)
    {  356, 0 },  //  6C: ld l,h (M:1 T:4 steps:1)
    {  357, 0 },  //  6D: ld l,l (M:1 T:4 steps:1)
    {  358, _Z80_OPSTATE_FLAGS_INDIRECT },  //  6E: ld l,(hl) (M:2 T:7 steps:4)
    {  362, 0 },  //  6F: ld l,a (M:1 T:4 steps:1)
    {  363, _Z80_OPSTATE_FLAGS_INDIRECT },  //  70: ld (hl),b (M:2 T:7 steps:4)
    {  367, _Z80_OPSTATE_FLAGS_INDIRECT },  //  71: ld (hl),c (M:2 T:7 steps:4)
    {  371, _Z80_OPSTATE_FLAGS_INDIRECT },  //  72: ld (hl),d (M:2 T:7 steps:4)
    {  375, _Z80_OPSTATE_FLAGS_INDIRECT },  //  73: ld (hl),e (M:2 T:7 steps:4)
    {  379, _Z80_OPSTATE_FLAGS_INDIRECT },  //  74: ld (hl),h (M:2 T:7 steps:4)
    {  383, _Z80_OPSTATE_FLAGS_INDIRECT },  //  75: ld (hl),l (M:2 T:7 steps:4)
    {  387, 0 },  //  76: halt (M:1 T:4 steps:1)
    {  388, _Z80_OPSTATE_FLAGS_INDIRECT },  //  77: ld (hl),a (M:2 T:7 steps:4)
    {  392, 0 },  //  78: ld a,b (M:1 T:4 steps:1)
    {  393, 0 },  //  79: ld a,c (M:1 T:4 steps:1)
    {  394, 0 },  //  7A: ld a,d (M:1 T:4 steps:1)
    {  395, 0 },  //  7B: ld a,e (M:1 T:4 steps:1)
    {  396, 0 },  //  7C: ld a,h (M:1 T:4 steps:1)
    {  397, 0 },  //  7D: ld a,l (M:1 T:4 steps:1)
    {  398, _Z80_OPSTATE_FLAGS_INDIRECT },  //  7E: ld a,(hl) (M:2 T:7 steps:4)
    {  402, 0 },  //  7F: ld a,a (M:1 T:4 steps:1)
    {  403, 0 },  //  80: add b (M:1 T:4 steps:1)
    {  404, 0 },  //  81: add c (M:1 T:4 steps:1)
    {  405, 0 },  //  82: add d (M:1 T:4 steps:1)
    {  406, 0 },  //  83: add e (M:1 T:4 steps:1)
    {  407, 0 },  //  84: add h (M:1 T:4 steps:1)
    {  408, 0 },  //  85: add l (M:1 T:4 steps:1)
    {  409, _Z80_OPSTATE_FLAGS_INDIRECT },  //  86: add (hl) (M:2 T:7 steps:4)
    {  413, 0 },  //  87: add a (M:1 T:4 steps:1)
    {  414, 0 },  //  88: adc b (M:1 T:4 steps:1)
    {  415, 0 },  //  89: adc c (M:1 T:4 steps:1)
    {  416, 0 },  //  8A: adc d (M:1 T:4 steps:1)
    {  417, 0 },  //  8B: adc e (M:1 T:4 steps:1)
    {  418, 0 },  //  8C: adc h (M:1 T:4 steps:1)
    {  419, 0 },  //  8D: adc l (M:1 T:4 steps:1)
    {  420, _Z80_OPSTATE_FLAGS_INDIRECT },  //  8E: adc (hl) (M:2 T:7 steps:4)
    {  424, 0 },  //  8F: adc a (M:1 T:4 steps:1)
    {  425, 0 },  //  90: sub b (M:1 T:4 steps:1)
    {  426, 0 },  //  91: sub c (M:1 T:4 steps:1)
    {  427, 0 },  //  92: sub d (M:1 T:4 steps:1)
    {  428, 0 },  //  93: sub e (M:1 T:4 steps:1)
    {  429, 0 },  //  94: sub h (M:1 T:4 steps:1)
    {  430, 0 },  //  95: sub l (M:1 T:4 steps:1)
    {  431, _Z80_OPSTATE_FLAGS_INDIRECT },  //  96: sub (hl) (M:2 T:7 steps:4)
    {  435, 0 },  //  97: sub a (M:1 T:4 steps:1)
    {  436, 0 },  //  98: sbc b (M:1 T:4 steps:1)
    {  437, 0 },  //  99: sbc c (M:1 T:4 steps:1)
    {  438, 0 },  //  9A: sbc d (M:1 T:4 steps:1)
    {  439, 0 },  //  9B: sbc e (M:1 T:4 steps:1)
    {  440, 0 },  //  9C: sbc h (M:1 T:4 steps:1)
    {  441, 0 },  //  9D: sbc l (M:1 T:4 steps:1)
    {  442, _Z80_OPSTATE_FLAGS_INDIRECT },  //  9E: sbc (hl) (M:2 T:7 steps:4)
    {  446, 0 },  //  9F: sbc a (M:1 T:4 steps:1)
    {  447, 0 },  //  A0: and b (M:1 T:4 steps:1)
    {  448, 0 },  //  A1: and c (M:1 T:4 steps:1)
    {  449, 0 },  //  A2: and d (M:1 T:4 steps:1)
    {  450, 0 },  //  A3: and e (M:1 T:4 steps:1)
    {  451, 0 },  //  A4: and h (M:1 T:4 steps:1)
    {  452, 0 },  //  A5: and l (M:1 T:4 steps:1)
    {  453, _Z80_OPSTATE_FLAGS_INDIRECT },  //  A6: and (hl) (M:2 T:7 steps:4)
    {  457, 0 },  //  A7: and a (M:1 T:4 steps:1)
    {  458, 0 },  //  A8: xor b (M:1 T:4 steps:1)
    {  459, 0 },  //  A9: xor c (M:1 T:4 steps:1)
    {  460, 0 },  //  AA: xor d (M:1 T:4 steps:1)
    {  461, 0 },  //  AB: xor e (M:1 T:4 steps:1)
    {  462, 0 },  //  AC: xor h (M:1 T:4 steps:1)
    {  463, 0 },  //  AD: xor l (M:1 T:4 steps:1)
    {  464, _Z80_OPSTATE_FLAGS_INDIRECT },  //  AE: xor (hl) (M:2 T:7 steps:4)
    {  468, 0 },  //  AF: xor a (M:1 T:4 steps:1)
    {  469, 0 },  //  B0: or b (M:1 T:4 steps:1)
    {  470, 0 },  //  B1: or c (M:1 T:4 steps:1)
    {  471, 0 },  //  B2: or d (M:1 T:4 steps:1)
    {  472, 0 },  //  B3: or e (M:1 T:4 steps:1)
    {  473, 0 },  //  B4: or h (M:1 T:4 steps:1)
    {  474, 0 },  //  B5: or l (M:1 T:4 steps:1)
    {  475, _Z80_OPSTATE_FLAGS_INDIRECT },  //  B6: or (hl) (M:2 T:7 steps:4)
    {  479, 0 },  //  B7: or a (M:1 T:4 steps:1)
    {  480, 0 },  //  B8: cp b (M:1 T:4 steps:1)
    {  481, 0 },  //  B9: cp c (M:1 T:4 steps:1)
    {  482, 0 },  //  BA: cp d (M:1 T:4 steps:1)
    {  483, 0 },  //  BB: cp e (M:1 T:4 steps:1)
    {  484, 0 },  //  BC: cp h (M:1 T:4 steps:1)
    {  485, 0 },  //  BD: cp l (M:1 T:4 steps:1)
    {  486, _Z80_OPSTATE_FLAGS_INDIRECT },  //  BE: cp (hl) (M:2 T:7 steps:4)
    {  490, 0 },  //  BF: cp a (M:1 T:4 steps:1)
    {  491, 0 },  //  C0: ret nz (M:4 T:11 steps:8)
    {  499, 0 },  //  C1: pop bc (M:3 T:10 steps:7)
    {  506, 0 },  //  C2: jp nz,nn (M:3 T:10 steps:7)
    {  513, 0 },  //  C3: jp nn (M:3 T:10 steps:7)
    {  520, 0 },  //  C4: call nz,nn (M:6 T:17 steps:14)
    {  534, 0 },  //  C5: push bc (M:4 T:11 steps:8)
    {  542, _Z80_OPSTATE_FLAGS_IMM8 },  //  C6: add n (M:2 T:7 steps:4)
    {  546, 0 },  //  C7: rst 0h (M:4 T:11 steps:8)
    {  554, 0 },  //  C8: ret z (M:4 T:11 steps:8)
    {  562, 0 },  //  C9: ret (M:3 T:10 steps:7)
    {  569, 0 },  //  CA: jp z,nn (M:3 T:10 steps:7)
    {  576, 0 },  //  CB: cb prefix (M:1 T:4 steps:1)
    {  577, 0 },  //  CC: call z,nn (M:6 T:17 steps:14)
    {  591, 0 },  //  CD: call nn (M:5 T:17 steps:14)
    {  605, _Z80_OPSTATE_FLAGS_IMM8 },  //  CE: adc n (M:2 T:7 steps:4)
    {  609, 0 },  //  CF: rst 8h (M:4 T:11 steps:8)
    {  617, 0 },  //  D0: ret nc (M:4 T:11 steps:8)
    {  625, 0 },  //  D1: pop de (M:3 T:10 steps:7)
    {  632, 0 },  //  D2: jp nc,nn (M:3 T:10 steps:7)
    {  639, 0 },  //  D3: out (n),a (M:3 T:11 steps:8)
    {  647, 0 },  //  D4: call nc,nn (M:6 T:17 steps:14)
    {  661, 0 },  //  D5: push de (M:4 T:11 steps:8)
    {  669, _Z80_OPSTATE_FLAGS_IMM8 },  //  D6: sub n (M:2 T:7 steps:4)
    {  673, 0 },  //  D7: rst 10h (M:4 T:11 steps:8)
    {  681, 0 },  //  D8: ret c (M:4 T:11 steps:8)
    {  689, 0 },  //  D9: exx (M:1 T:4 steps:1)
    {  690, 0 },  //  DA: jp c,nn (M:3 T:10 steps:7)
    {  697, 0 },  //  DB: in a,(n) (M:3 T:11 steps:8)
    {  705, 0 },  //  DC: call c,nn (M:6 T:17 steps:14)
    {  719, 0 },  //  DD: dd prefix (M:1 T:4 steps:1)
    {  720, _Z80_OPSTATE_FLAGS_IMM8 },  //  DE: sbc n (M:2 T:7 steps:4)
    {  724, 0 },  //  DF: rst 18h (M:4 T:11 steps:8)
    {  732, 0 },  //  E0: ret po (M:4 T:11 steps:8)
    {  740, 0 },  //  E1: pop hl (M:3 T:10 steps:7)
    {  747, 0 },  //  E2: jp po,nn (M:3 T:10 steps:7)
    {  754, 0 },  //  E3: ex (sp),hl (M:5 T:19 steps:16)
    {  770, 0 },  //  E4: call po,nn (M:6 T:17 steps:14)
    {  784, 0 },  //  E5: push hl (M:4 T:11 steps:8)
    {  792, _Z80_OPSTATE_FLAGS_IMM8 },  //  E6: and n (M:2 T:7 steps:4)
    {  796, 0 },  //  E7: rst 20h (M:4 T:11 steps:8)
    {  804, 0 },  //  E8: ret pe (M:4 T:11 steps:8)
    {  812, 0 },  //  E9: jp hl (M:1 T:4 steps:1)
    {  813, 0 },  //  EA: jp pe,nn (M:3 T:10 steps:7)
    {  820, 0 },  //  EB: ex de,hl (M:1 T:4 steps:1)
    {  821, 0 },  //  EC: call pe,nn (M:6 T:17 steps:14)
    {  835, 0 },  //  ED: ed prefix (M:1 T:4 steps:1)
    {  836, _Z80_OPSTATE_FLAGS_IMM8 },  //  EE: xor n (M:2 T:7 steps:4)
    {  840, 0 },  //  EF: rst 28h (M:4 T:11 steps:8)
    {  848, 0 },  //  F0: ret p (M:4 T:11 steps:8)
    {  856, 0 },  //  F1: pop af (M:3 T:10 steps:7)
    {  863, 0 },  //  F2: jp p,nn (M:3 T:10 steps:7)
    {  870, 0 },  //  F3: di (M:1 T:4 steps:1)
    {  871, 0 },  //  F4: call p,nn (M:6 T:17 steps:14)
    {  885, 0 },  //  F5: push af (M:4 T:11 steps:8)
    {  893, _Z80_OPSTATE_FLAGS_IMM8 },  //  F6: or n (M:2 T:7 steps:4)
    {  897, 0 },  //  F7: rst 30h (M:4 T:11 steps:8)
    {  905, 0 },  //  F8: ret m (M:4 T:11 steps:8)
    {  913, 0 },  //  F9: ld sp,hl (M:2 T:6 steps:3)
    {  916, 0 },  //  FA: jp m,nn (M:3 T:10 steps:7)
    {  923, 0 },  //  FB: ei (M:1 T:4 steps:1)
    {  924, 0 },  //  FC: call m,nn (M:6 T:17 steps:14)
    {  938, 0 },  //  FD: fd prefix (M:1 T:4 steps:1)
    {  939, _Z80_OPSTATE_FLAGS_IMM8 },  //  FE: cp n (M:2 T:7 steps:4)
    {  943, 0 },  //  FF: rst 38h (M:4 T:11 steps:8)
    {  951, 0 },  // ED 00: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 01: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 02: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 03: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 04: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 05: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 06: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 07: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 08: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 09: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 0A: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 0B: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 0C: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 0D: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 0E: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 0F: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 10: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 11: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 12: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 13: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 14: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 15: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 16: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 17: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 18: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 19: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 1A: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 1B: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 1C: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 1D: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 1E: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 1F: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 20: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 21: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 22: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 23: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 24: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 25: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 26: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 27: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 28: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 29: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 2A: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 2B: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 2C: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 2D: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 2E: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 2F: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 30: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 31: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 32: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 33: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 34: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 35: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 36: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 37: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 38: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 39: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 3A: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 3B: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 3C: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 3D: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 3E: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 3F: ed nop (M:1 T:4 steps:1)
    {  952, 0 },  // ED 40: in b,(c) (M:2 T:8 steps:5)
    {  957, 0 },  // ED 41: out (c),b (M:2 T:8 steps:5)
    {  962, 0 },  // ED 42: sbc hl,bc (M:2 T:11 steps:8)
    {  970, 0 },  // ED 43: ld (nn),bc (M:5 T:16 steps:13)
    {  983, 0 },  // ED 44: neg (M:1 T:4 steps:1)
    {  984, 0 },  // ED 45: reti/retn (M:3 T:10 steps:7)
    {  991, 0 },  // ED 46: im 0 (M:1 T:4 steps:1)
    {  992, 0 },  // ED 47: ld i,a (M:2 T:5 steps:2)
    {  994, 0 },  // ED 48: in c,(c) (M:2 T:8 steps:5)
    {  999, 0 },  // ED 49: out (c),c (M:2 T:8 steps:5)
    { 1004, 0 },  // ED 4A: adc hl,bc (M:2 T:11 steps:8)
    { 1012, 0 },  // ED 4B: ld bc,(nn) (M:5 T:16 steps:13)
    {  983, 0 },  // ED 4C: neg (M:1 T:4 steps:1)
    {  984, 0 },  // ED 4D: reti/retn (M:3 T:10 steps:7)
    { 1025, 0 },  // ED 4E: im 0 (M:1 T:4 steps:1)
    { 1026, 0 },  // ED 4F: ld r,a (M:2 T:5 steps:2)
    { 1028, 0 },  // ED 50: in d,(c) (M:2 T:8 steps:5)
    { 1033, 0 },  // ED 51: out (c),d (M:2 T:8 steps:5)
    { 1038, 0 },  // ED 52: sbc hl,de (M:2 T:11 steps:8)
    { 1046, 0 },  // ED 53: ld (nn),de (M:5 T:16 steps:13)
    {  983, 0 },  // ED 54: neg (M:1 T:4 steps:1)
    {  984, 0 },  // ED 55: reti/retn (M:3 T:10 steps:7)
    { 1059, 0 },  // ED 56: im 1 (M:1 T:4 steps:1)
    { 1060, 0 },  // ED 57: ld a,i (M:2 T:5 steps:2)
    { 1062, 0 },  // ED 58: in e,(c) (M:2 T:8 steps:5)
    { 1067, 0 },  // ED 59: out (c),e (M:2 T:8 steps:5)
    { 1072, 0 },  // ED 5A: adc hl,de (M:2 T:11 steps:8)
    { 1080, 0 },  // ED 5B: ld de,(nn) (M:5 T:16 steps:13)
    {  983, 0 },  // ED 5C: neg (M:1 T:4 steps:1)
    {  984, 0 },  // ED 5D: reti/retn (M:3 T:10 steps:7)
    { 1093, 0 },  // ED 5E: im 2 (M:1 T:4 steps:1)
    { 1094, 0 },  // ED 5F: ld a,r (M:2 T:5 steps:2)
    { 1096, 0 },  // ED 60: in h,(c) (M:2 T:8 steps:5)
    { 1101, 0 },  // ED 61: out (c),h (M:2 T:8 steps:5)
    { 1106, 0 },  // ED 62: sbc hl,hl (M:2 T:11 steps:8)
    { 1114, 0 },  // ED 63: ld (nn),hl (M:5 T:16 steps:13)
    {  983, 0 },  // ED 64: neg (M:1 T:4 steps:1)
    {  984, 0 },  // ED 65: reti/retn (M:3 T:10 steps:7)
    { 1127, 0 },  // ED 66: im 0 (M:1 T:4 steps:1)
    { 1128, 0 },  // ED 67: rrd (M:4 T:14 steps:11)
    { 1139, 0 },  // ED 68: in l,(c) (M:2 T:8 steps:5)
    { 1144, 0 },  // ED 69: out (c),l (M:2 T:8 steps:5)
    { 1149, 0 },  // ED 6A: adc hl,hl (M:2 T:11 steps:8)
    { 1157, 0 },  // ED 6B: ld hl,(nn) (M:5 T:16 steps:13)
    {  983, 0 },  // ED 6C: neg (M:1 T:4 steps:1)
    {  984, 0 },  // ED 6D: reti/retn (M:3 T:10 steps:7)
    { 1170, 0 },  // ED 6E: im 0 (M:1 T:4 steps:1)
    { 1171, 0 },  // ED 6F: rld (M:4 T:14 steps:11)
    { 1182, 0 },  // ED 70: in (c) (M:2 T:8 steps:5)
    { 1187, 0 },  // ED 71: out (c),0 (M:2 T:8 steps:5)
    { 1192, 0 },  // ED 72: sbc hl,sp (M:2 T:11 steps:8)
    { 1200, 0 },  // ED 73: ld (nn),sp (M:5 T:16 steps:13)
    {  983, 0 },  // ED 74: neg (M:1 T:4 steps:1)
    {  984, 0 },  // ED 75: reti/retn (M:3 T:10 steps:7)
    { 1213, 0 },  // ED 76: im 1 (M:1 T:4 steps:1)
    {  951, 0 },  // ED 77: ed nop (M:1 T:4 steps:1)
    { 1214, 0 },  // ED 78: in a,(c) (M:2 T:8 steps:5)
    { 1219, 0 },  // ED 79: out (c),a (M:2 T:8 steps:5)
    { 1224, 0 },  // ED 7A: adc hl,sp (M:2 T:11 steps:8)
    { 1232, 0 },  // ED 7B: ld sp,(nn) (M:5 T:16 steps:13)
    {  983, 0 },  // ED 7C: neg (M:1 T:4 steps:1)
    {  984, 0 },  // ED 7D: reti/retn (M:3 T:10 steps:7)
    { 1245, 0 },  // ED 7E: im 2 (M:1 T:4 steps:1)
    {  951, 0 },  // ED 7F: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 80: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 81: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 82: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 83: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 84: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 85: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 86: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 87: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 88: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 89: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 8A: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 8B: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 8C: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 8D: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 8E: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 8F: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 90: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 91: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 92: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 93: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 94: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 95: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 96: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 97: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 98: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 99: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 9A: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 9B: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 9C: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 9D: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 9E: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 9F: ed nop (M:1 T:4 steps:1)
    { 1246, 0 },  // ED A0: ldi (M:4 T:12 steps:9)
    { 1255, 0 },  // ED A1: cpi (M:3 T:12 steps:9)
    { 1264, 0 },  // ED A2: ini (M:4 T:12 steps:9)
    { 1273, 0 },  // ED A3: outi (M:4 T:12 steps:9)
    {  951, 0 },  // ED A4: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED A5: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED A6: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED A7: ed nop (M:1 T:4 steps:1)
    { 1282, 0 },  // ED A8: ldd (M:4 T:12 steps:9)
    { 1291, 0 },  // ED A9: cpd (M:3 T:12 steps:9)
    { 1300, 0 },  // ED AA: ind (M:4 T:12 steps:9)
    { 1309, 0 },  // ED AB: outd (M:4 T:12 steps:9)
    {  951, 0 },  // ED AC: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED AD: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED AE: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED AF: ed nop (M:1 T:4 steps:1)
    { 1318, 0 },  // ED B0: ldir (M:5 T:17 steps:14)
    { 1332, 0 },  // ED B1: cpir (M:4 T:17 steps:14)
    { 1346, 0 },  // ED B2: inir (M:5 T:17 steps:14)
    { 1360, 0 },  // ED B3: otir (M:5 T:17 steps:14)
    {  951, 0 },  // ED B4: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED B5: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED B6: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED B7: ed nop (M:1 T:4 steps:1)
    { 1374, 0 },  // ED B8: lddr (M:5 T:17 steps:14)
    { 1388, 0 },  // ED B9: cpdr (M:4 T:17 steps:14)
    { 1402, 0 },  // ED BA: indr (M:5 T:17 steps:14)
    { 1416, 0 },  // ED BB: otdr (M:5 T:17 steps:14)
    {  951, 0 },  // ED BC: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED BD: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED BE: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED BF: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED C0: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED C1: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED C2: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED C3: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED C4: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED C5: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED C6: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED C7: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED C8: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED C9: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED CA: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED CB: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED CC: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED CD: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED CE: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED CF: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED D0: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED D1: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED D2: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED D3: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED D4: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED D5: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED D6: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED D7: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED D8: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED D9: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED DA: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED DB: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED DC: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED DD: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED DE: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED DF: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED E0: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED E1: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED E2: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED E3: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED E4: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED E5: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED E6: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED E7: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED E8: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED E9: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED EA: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED EB: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED EC: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED ED: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED EE: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED EF: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED F0: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED F1: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED F2: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED F3: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED F4: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED F5: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED F6: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED F7: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED F8: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED F9: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED FA: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED FB: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED FC: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED FD: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED FE: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED FF: ed nop (M:1 T:4 steps:1)
    { 1430, 0 },  // CB 00: cb (M:1 T:4 steps:1)
    { 1431, 0 },  // CB 01: cbhl (M:3 T:11 steps:8)
    { 1439, 0 },  // CB 02: ddfdcb (M:6 T:18 steps:15)
    { 1454, 0 },  //  03: int_im0 (M:6 T:9 steps:6)
    { 1460, 0 },  //  04: int_im1 (M:7 T:16 steps:13)
    { 1473, 0 },  //  05: int_im2 (M:9 T:22 steps:19)
    { 1492, 0 },  //  06: nmi (M:5 T:14 steps:11)

};

// initiate refresh cycle
static inline uint64_t _z80_refresh(z80_t* cpu, uint64_t pins) {
    pins = _z80_set_ab_x(pins, cpu->ir, Z80_MREQ|Z80_RFSH);
    cpu->r = (cpu->r & 0x80) | ((cpu->r + 1) & 0x7F);
    return pins;
}

// initiate M1 cycle of NMI
static inline uint64_t _z80_nmi_step0(z80_t* cpu, uint64_t pins) {
    // the next regular opcode which is on the data bus is ignored!

    // disable interrupts
    cpu->iff1 = false;

    // if in HALT state, continue
    if (pins & Z80_HALT) {
        pins &= ~Z80_HALT;
        cpu->pc++;
    }
    return pins;
}

// IM0..IM2 initial step
static inline uint64_t _z80_int012_step0(z80_t* cpu, uint64_t pins) {
    // disable interrupts
    cpu->iff1 = cpu->iff2 = false;
    // if in HALT state, continue
    if (pins & Z80_HALT) {
        pins &= ~Z80_HALT;
        cpu->pc++;
    }
    return pins;
}

// IM0..IM2 step 1: issue M1|IORQ cycle
static inline uint64_t _z80_int012_step1(z80_t* cpu, uint64_t pins) {
    (void)cpu;
    // issue M1|IORQ to get opcode byte
    return pins | (Z80_M1|Z80_IORQ);
}

// IM0 step 2: load data bus into opcode
static inline uint64_t _z80_int0_step2(z80_t* cpu, uint64_t pins) {
    // store opcode byte
    cpu->opcode = _z80_get_db(pins);
    return pins;
}

// IM0 step 3: refresh cycle and start executing loaded opcode
static inline uint64_t _z80_int0_step3(z80_t* cpu, uint64_t pins) {
    // branch to interrupt 'payload' instruction (usually an RST)
    cpu->op = _z80_opstate_table[cpu->opcode];
    return pins;
}

// initiate a fetch machine cycle
static inline uint64_t _z80_fetch(z80_t* cpu, uint64_t pins) {
    if (cpu->int_bits & Z80_NMI) {
        // non-maskable interrupt starts with a regular M1 machine cycle
        cpu->op = _z80_opstate_table[_Z80_OPSTATE_SLOT_NMI];
        cpu->int_bits = 0;
        // NOTE: PC is *not* incremented!
        pins = _z80_set_ab_x(pins, cpu->pc, Z80_M1|Z80_MREQ|Z80_RD);
    }
    else if ((cpu->int_bits & Z80_INT) && cpu->iff1) {
        // maskable interrupts start with a special M1 machine cycle which
        // doesn't fetch the next opcode, but instead activate the
        // pins M1|IOQR to request a special byte which is handled differently
        // depending on interrupt mode
        cpu->op = _z80_opstate_table[_Z80_OPSTATE_SLOT_INT_IM0 + cpu->im];
        cpu->int_bits = 0;
        // NOTE: PC is not incremented, and no pins are activated here
    }
    else {
        // no interrupt, continue with next opcode
        cpu->op.step = 0xFFFF;
        pins = _z80_set_ab_x(pins, cpu->pc++, Z80_M1|Z80_MREQ|Z80_RD);
    }
    cpu->prefix_state = 0;
    return pins;
}

static inline uint64_t _z80_fetch_prefix(z80_t* cpu, uint64_t pins, uint8_t prefix) {
    // reset the decoder to continue at step 0
    cpu->op.step = 0xFFFF;
    switch (prefix) {
        case _Z80_PREFIX_CB: // CB prefix preserves current DD/FD prefix
            cpu->prefix |= _Z80_PREFIX_CB;
            if (cpu->prefix & (_Z80_PREFIX_DD|_Z80_PREFIX_FD)) {
                // this is a DD+CB / FD+CB instruction, continue
                // execution on the special DDCB/FDCB decoder block which
                // loads the d-offset first and then the opcode in a 
                // regular memory read machine cycle
                cpu->op = _z80_opstate_table[_Z80_OPSTATE_SLOT_DDFDCB];
            }
            else {
                // this is a regular CB-prefixed instruction, continue
                // execution on a special fetch machine cycle which doesn't
                // handle DD/FD prefix and then branches either to the
                // special CB or CBHL decoder block
                cpu->op.step = 19 - 1; // => step 19
                pins = _z80_set_ab_x(pins, cpu->pc++, Z80_M1|Z80_MREQ|Z80_RD);
            }
            break;
        case _Z80_PREFIX_DD:
            cpu->prefix_offset = 0;
            cpu->hlx_idx = 1;
            cpu->prefix = _Z80_PREFIX_DD;
            pins = _z80_set_ab_x(pins, cpu->pc++, Z80_M1|Z80_MREQ|Z80_RD);
            break;
        case _Z80_PREFIX_ED: // ED prefix clears current DD/FD prefix
            cpu->prefix_offset = 0x0100;
            cpu->hlx_idx = 0;
            cpu->prefix = _Z80_PREFIX_ED;
            pins = _z80_set_ab_x(pins, cpu->pc++, Z80_M1|Z80_MREQ|Z80_RD);
            break;
        case _Z80_PREFIX_FD:
            cpu->prefix_offset = 0;
            cpu->hlx_idx = 2;
            cpu->prefix = _Z80_PREFIX_FD;
            pins = _z80_set_ab_x(pins, cpu->pc++, Z80_M1|Z80_MREQ|Z80_RD);
            break;
    }
    return pins;
}

uint64_t z80_prefetch(z80_t* cpu, uint16_t new_pc) {
    cpu->pc = new_pc;
    // overlapped M1:T1 of the NOP instruction to initiate opcode fetch at new pc
    cpu->op.step = _z80_opstate_table[0].step + 1;
    return 0;
}

// pin helper macros
#define _sa(ab)             pins=_z80_set_ab(pins,ab)
#define _sax(ab,x)          pins=_z80_set_ab_x(pins,ab,x)
#define _sad(ab,d)          pins=_z80_set_ab_db(pins,ab,d)
#define _sadx(ab,d,x)       pins=_z80_set_ab_db_x(pins,ab,d,x)
#define _gd()               _z80_get_db(pins)

// high level helper macros
#define _fetch_dd()     pins=_z80_fetch_prefix(cpu,pins,_Z80_PREFIX_DD);
#define _fetch_fd()     pins=_z80_fetch_prefix(cpu,pins,_Z80_PREFIX_FD);
#define _fetch_ed()     pins=_z80_fetch_prefix(cpu,pins,_Z80_PREFIX_ED);
#define _fetch_cb()     pins=_z80_fetch_prefix(cpu,pins,_Z80_PREFIX_CB);
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
    // process the next active tcycle
    pins &= ~(Z80_CTRL_PIN_MASK|Z80_RETI);
    switch (cpu->op.step) {
        // M1/T2: shared fetch machine cycle for all opcodes
        case 0: _wait(); cpu->opcode = _gd(); goto step_next;
        // M1/T3: refresh cycle
        case 1: pins = _z80_refresh(cpu, pins); goto step_next;
        // M1/T4: branch to instruction 'payload'
        case 2: {
            cpu->op = _z80_opstate_table[cpu->opcode + cpu->prefix_offset];
            // if this is a (HL)/(IX+d)/(IY+d) instruction, insert
            // d-load cycle if needed and compute effective address
            if (cpu->op.flags & _Z80_OPSTATE_FLAGS_INDIRECT) {
                cpu->addr = cpu->hlx[cpu->hlx_idx].hl;
                if (cpu->hlx_idx != _Z80_MAP_HL) {
                    if (cpu->op.flags & _Z80_OPSTATE_FLAGS_IMM8) {
                        // special case: if this is indirect+immediate (which is
                        // just LD (HL),n, then the immediate-load is 'hidden' within
                        // the 8-tcycle d-offset computation)
                        cpu->op.step = 10;  // continue at step 11
                    }
                    else {
                        // regular (IX+d)/(IY+d) instruction
                        cpu->op.step = 2;   // continue at step 3
                    }
                }
            }
        } goto step_next;
        //=== optional d-loading cycle for (IX+d), (IY+d)
        //--- mread
        case 3: goto step_next;
        case 4: _wait();_mread(cpu->pc++); goto step_next;
        case 5: cpu->addr += (int8_t)_gd(); cpu->wz = cpu->addr; goto step_next;
        //--- filler ticks
        case 6: goto step_next;
        case 7: goto step_next;
        case 8: goto step_next;
        case 9: goto step_next;
        case 10: {
            // branch to original instruction
            cpu->op = _z80_opstate_table[cpu->opcode];
        } goto step_next;
        //=== special case d-loading cycle for (IX+d),n where the immediate load
        //    is hidden in the d-cycle load
        //--- mread for d offset
        case 11: goto step_next;
        case 12: _wait();_mread(cpu->pc++); goto step_next;
        case 13: cpu->addr += (int8_t)_gd(); cpu->wz = cpu->addr; goto step_next;
        //--- mread for n
        case 14: goto step_next;
        case 15: _wait();_mread(cpu->pc++); goto step_next;
        case 16: cpu->dlatch=_gd(); goto step_next;
        //--- filler tick
        case 17: goto step_next;
        case 18: {
            // branch to ld (hl),n and skip the original mread cycle for loading 'n'
            cpu->op = _z80_opstate_table[cpu->opcode];
            cpu->op.step += 3;
        } goto step_next;
        //=== special opcode fetch machine cycle for CB-prefixed instructions
        case 19: _wait(); cpu->opcode = _gd(); goto step_next;
        case 20: pins = _z80_refresh(cpu, pins); goto step_next;
        case 21: {
            if ((cpu->opcode & 7) == 6) {
                // this is a (HL) instruction
                cpu->addr = cpu->hl;
                cpu->op = _z80_opstate_table[_Z80_OPSTATE_SLOT_CBHL];
            }
            else {
                cpu->op = _z80_opstate_table[_Z80_OPSTATE_SLOT_CB];
            }
        } goto step_next;
        
        //  00: nop (M:1 T:4)
        // -- overlapped
        case   22: goto fetch_next;
        
        //  01: ld bc,nn (M:3 T:10)
        // -- mread
        case   23: goto step_next;
        case   24: _wait();_mread(cpu->pc++);goto step_next;
        case   25: cpu->c=_gd();;goto step_next;
        // -- mread
        case   26: goto step_next;
        case   27: _wait();_mread(cpu->pc++);goto step_next;
        case   28: cpu->b=_gd();;goto step_next;
        // -- overlapped
        case   29: goto fetch_next;
        
        //  02: ld (bc),a (M:2 T:7)
        // -- mwrite
        case   30: goto step_next;
        case   31: _wait();_mwrite(cpu->bc,cpu->a);cpu->wzl=cpu->c+1;cpu->wzh=cpu->a;goto step_next;
        case   32: goto step_next;
        // -- overlapped
        case   33: goto fetch_next;
        
        //  03: inc bc (M:2 T:6)
        // -- generic
        case   34: cpu->bc++;goto step_next;
        case   35: goto step_next;
        // -- overlapped
        case   36: goto fetch_next;
        
        //  04: inc b (M:1 T:4)
        // -- overlapped
        case   37: cpu->b=_z80_inc8(cpu,cpu->b);goto fetch_next;
        
        //  05: dec b (M:1 T:4)
        // -- overlapped
        case   38: cpu->b=_z80_dec8(cpu,cpu->b);goto fetch_next;
        
        //  06: ld b,n (M:2 T:7)
        // -- mread
        case   39: goto step_next;
        case   40: _wait();_mread(cpu->pc++);goto step_next;
        case   41: cpu->b=_gd();;goto step_next;
        // -- overlapped
        case   42: goto fetch_next;
        
        //  07: rlca (M:1 T:4)
        // -- overlapped
        case   43: _z80_rlca(cpu);goto fetch_next;
        
        //  08: ex af,af' (M:1 T:4)
        // -- overlapped
        case   44: _z80_ex_af_af2(cpu);goto fetch_next;
        
        //  09: add hl,bc (M:2 T:11)
        // -- generic
        case   45: _z80_add16(cpu,cpu->bc);goto step_next;
        case   46: goto step_next;
        case   47: goto step_next;
        case   48: goto step_next;
        case   49: goto step_next;
        case   50: goto step_next;
        case   51: goto step_next;
        // -- overlapped
        case   52: goto fetch_next;
        
        //  0A: ld a,(bc) (M:2 T:7)
        // -- mread
        case   53: goto step_next;
        case   54: _wait();_mread(cpu->bc);goto step_next;
        case   55: cpu->a=_gd();cpu->wz=cpu->bc+1;goto step_next;
        // -- overlapped
        case   56: goto fetch_next;
        
        //  0B: dec bc (M:2 T:6)
        // -- generic
        case   57: cpu->bc--;goto step_next;
        case   58: goto step_next;
        // -- overlapped
        case   59: goto fetch_next;
        
        //  0C: inc c (M:1 T:4)
        // -- overlapped
        case   60: cpu->c=_z80_inc8(cpu,cpu->c);goto fetch_next;
        
        //  0D: dec c (M:1 T:4)
        // -- overlapped
        case   61: cpu->c=_z80_dec8(cpu,cpu->c);goto fetch_next;
        
        //  0E: ld c,n (M:2 T:7)
        // -- mread
        case   62: goto step_next;
        case   63: _wait();_mread(cpu->pc++);goto step_next;
        case   64: cpu->c=_gd();;goto step_next;
        // -- overlapped
        case   65: goto fetch_next;
        
        //  0F: rrca (M:1 T:4)
        // -- overlapped
        case   66: _z80_rrca(cpu);goto fetch_next;
        
        //  10: djnz d (M:4 T:13)
        // -- generic
        case   67: ;goto step_next;
        // -- mread
        case   68: goto step_next;
        case   69: _wait();_mread(cpu->pc++);goto step_next;
        case   70: cpu->dlatch=_gd();if(--cpu->b==0){_z80_skip(cpu,5);};goto step_next;
        // -- generic
        case   71: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;;goto step_next;
        case   72: goto step_next;
        case   73: goto step_next;
        case   74: goto step_next;
        case   75: goto step_next;
        // -- overlapped
        case   76: goto fetch_next;
        
        //  11: ld de,nn (M:3 T:10)
        // -- mread
        case   77: goto step_next;
        case   78: _wait();_mread(cpu->pc++);goto step_next;
        case   79: cpu->e=_gd();;goto step_next;
        // -- mread
        case   80: goto step_next;
        case   81: _wait();_mread(cpu->pc++);goto step_next;
        case   82: cpu->d=_gd();;goto step_next;
        // -- overlapped
        case   83: goto fetch_next;
        
        //  12: ld (de),a (M:2 T:7)
        // -- mwrite
        case   84: goto step_next;
        case   85: _wait();_mwrite(cpu->de,cpu->a);cpu->wzl=cpu->e+1;cpu->wzh=cpu->a;goto step_next;
        case   86: goto step_next;
        // -- overlapped
        case   87: goto fetch_next;
        
        //  13: inc de (M:2 T:6)
        // -- generic
        case   88: cpu->de++;goto step_next;
        case   89: goto step_next;
        // -- overlapped
        case   90: goto fetch_next;
        
        //  14: inc d (M:1 T:4)
        // -- overlapped
        case   91: cpu->d=_z80_inc8(cpu,cpu->d);goto fetch_next;
        
        //  15: dec d (M:1 T:4)
        // -- overlapped
        case   92: cpu->d=_z80_dec8(cpu,cpu->d);goto fetch_next;
        
        //  16: ld d,n (M:2 T:7)
        // -- mread
        case   93: goto step_next;
        case   94: _wait();_mread(cpu->pc++);goto step_next;
        case   95: cpu->d=_gd();;goto step_next;
        // -- overlapped
        case   96: goto fetch_next;
        
        //  17: rla (M:1 T:4)
        // -- overlapped
        case   97: _z80_rla(cpu);goto fetch_next;
        
        //  18: jr d (M:3 T:12)
        // -- mread
        case   98: goto step_next;
        case   99: _wait();_mread(cpu->pc++);goto step_next;
        case  100: cpu->dlatch=_gd();;goto step_next;
        // -- generic
        case  101: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;;goto step_next;
        case  102: goto step_next;
        case  103: goto step_next;
        case  104: goto step_next;
        case  105: goto step_next;
        // -- overlapped
        case  106: goto fetch_next;
        
        //  19: add hl,de (M:2 T:11)
        // -- generic
        case  107: _z80_add16(cpu,cpu->de);goto step_next;
        case  108: goto step_next;
        case  109: goto step_next;
        case  110: goto step_next;
        case  111: goto step_next;
        case  112: goto step_next;
        case  113: goto step_next;
        // -- overlapped
        case  114: goto fetch_next;
        
        //  1A: ld a,(de) (M:2 T:7)
        // -- mread
        case  115: goto step_next;
        case  116: _wait();_mread(cpu->de);goto step_next;
        case  117: cpu->a=_gd();cpu->wz=cpu->de+1;goto step_next;
        // -- overlapped
        case  118: goto fetch_next;
        
        //  1B: dec de (M:2 T:6)
        // -- generic
        case  119: cpu->de--;goto step_next;
        case  120: goto step_next;
        // -- overlapped
        case  121: goto fetch_next;
        
        //  1C: inc e (M:1 T:4)
        // -- overlapped
        case  122: cpu->e=_z80_inc8(cpu,cpu->e);goto fetch_next;
        
        //  1D: dec e (M:1 T:4)
        // -- overlapped
        case  123: cpu->e=_z80_dec8(cpu,cpu->e);goto fetch_next;
        
        //  1E: ld e,n (M:2 T:7)
        // -- mread
        case  124: goto step_next;
        case  125: _wait();_mread(cpu->pc++);goto step_next;
        case  126: cpu->e=_gd();;goto step_next;
        // -- overlapped
        case  127: goto fetch_next;
        
        //  1F: rra (M:1 T:4)
        // -- overlapped
        case  128: _z80_rra(cpu);goto fetch_next;
        
        //  20: jr nz,d (M:3 T:12)
        // -- mread
        case  129: goto step_next;
        case  130: _wait();_mread(cpu->pc++);goto step_next;
        case  131: cpu->dlatch=_gd();if(!(_cc_nz)){_z80_skip(cpu,5);};goto step_next;
        // -- generic
        case  132: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;;goto step_next;
        case  133: goto step_next;
        case  134: goto step_next;
        case  135: goto step_next;
        case  136: goto step_next;
        // -- overlapped
        case  137: goto fetch_next;
        
        //  21: ld hl,nn (M:3 T:10)
        // -- mread
        case  138: goto step_next;
        case  139: _wait();_mread(cpu->pc++);goto step_next;
        case  140: cpu->hlx[cpu->hlx_idx].l=_gd();;goto step_next;
        // -- mread
        case  141: goto step_next;
        case  142: _wait();_mread(cpu->pc++);goto step_next;
        case  143: cpu->hlx[cpu->hlx_idx].h=_gd();;goto step_next;
        // -- overlapped
        case  144: goto fetch_next;
        
        //  22: ld (nn),hl (M:5 T:16)
        // -- mread
        case  145: goto step_next;
        case  146: _wait();_mread(cpu->pc++);goto step_next;
        case  147: cpu->wzl=_gd();;goto step_next;
        // -- mread
        case  148: goto step_next;
        case  149: _wait();_mread(cpu->pc++);goto step_next;
        case  150: cpu->wzh=_gd();;goto step_next;
        // -- mwrite
        case  151: goto step_next;
        case  152: _wait();_mwrite(cpu->wz++,cpu->hlx[cpu->hlx_idx].l);;goto step_next;
        case  153: goto step_next;
        // -- mwrite
        case  154: goto step_next;
        case  155: _wait();_mwrite(cpu->wz,cpu->hlx[cpu->hlx_idx].h);;goto step_next;
        case  156: goto step_next;
        // -- overlapped
        case  157: goto fetch_next;
        
        //  23: inc hl (M:2 T:6)
        // -- generic
        case  158: cpu->hlx[cpu->hlx_idx].hl++;goto step_next;
        case  159: goto step_next;
        // -- overlapped
        case  160: goto fetch_next;
        
        //  24: inc h (M:1 T:4)
        // -- overlapped
        case  161: cpu->hlx[cpu->hlx_idx].h=_z80_inc8(cpu,cpu->hlx[cpu->hlx_idx].h);goto fetch_next;
        
        //  25: dec h (M:1 T:4)
        // -- overlapped
        case  162: cpu->hlx[cpu->hlx_idx].h=_z80_dec8(cpu,cpu->hlx[cpu->hlx_idx].h);goto fetch_next;
        
        //  26: ld h,n (M:2 T:7)
        // -- mread
        case  163: goto step_next;
        case  164: _wait();_mread(cpu->pc++);goto step_next;
        case  165: cpu->hlx[cpu->hlx_idx].h=_gd();;goto step_next;
        // -- overlapped
        case  166: goto fetch_next;
        
        //  27: daa (M:1 T:4)
        // -- overlapped
        case  167: _z80_daa(cpu);goto fetch_next;
        
        //  28: jr z,d (M:3 T:12)
        // -- mread
        case  168: goto step_next;
        case  169: _wait();_mread(cpu->pc++);goto step_next;
        case  170: cpu->dlatch=_gd();if(!(_cc_z)){_z80_skip(cpu,5);};goto step_next;
        // -- generic
        case  171: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;;goto step_next;
        case  172: goto step_next;
        case  173: goto step_next;
        case  174: goto step_next;
        case  175: goto step_next;
        // -- overlapped
        case  176: goto fetch_next;
        
        //  29: add hl,hl (M:2 T:11)
        // -- generic
        case  177: _z80_add16(cpu,cpu->hlx[cpu->hlx_idx].hl);goto step_next;
        case  178: goto step_next;
        case  179: goto step_next;
        case  180: goto step_next;
        case  181: goto step_next;
        case  182: goto step_next;
        case  183: goto step_next;
        // -- overlapped
        case  184: goto fetch_next;
        
        //  2A: ld hl,(nn) (M:5 T:16)
        // -- mread
        case  185: goto step_next;
        case  186: _wait();_mread(cpu->pc++);goto step_next;
        case  187: cpu->wzl=_gd();;goto step_next;
        // -- mread
        case  188: goto step_next;
        case  189: _wait();_mread(cpu->pc++);goto step_next;
        case  190: cpu->wzh=_gd();;goto step_next;
        // -- mread
        case  191: goto step_next;
        case  192: _wait();_mread(cpu->wz++);goto step_next;
        case  193: cpu->hlx[cpu->hlx_idx].l=_gd();;goto step_next;
        // -- mread
        case  194: goto step_next;
        case  195: _wait();_mread(cpu->wz);goto step_next;
        case  196: cpu->hlx[cpu->hlx_idx].h=_gd();;goto step_next;
        // -- overlapped
        case  197: goto fetch_next;
        
        //  2B: dec hl (M:2 T:6)
        // -- generic
        case  198: cpu->hlx[cpu->hlx_idx].hl--;goto step_next;
        case  199: goto step_next;
        // -- overlapped
        case  200: goto fetch_next;
        
        //  2C: inc l (M:1 T:4)
        // -- overlapped
        case  201: cpu->hlx[cpu->hlx_idx].l=_z80_inc8(cpu,cpu->hlx[cpu->hlx_idx].l);goto fetch_next;
        
        //  2D: dec l (M:1 T:4)
        // -- overlapped
        case  202: cpu->hlx[cpu->hlx_idx].l=_z80_dec8(cpu,cpu->hlx[cpu->hlx_idx].l);goto fetch_next;
        
        //  2E: ld l,n (M:2 T:7)
        // -- mread
        case  203: goto step_next;
        case  204: _wait();_mread(cpu->pc++);goto step_next;
        case  205: cpu->hlx[cpu->hlx_idx].l=_gd();;goto step_next;
        // -- overlapped
        case  206: goto fetch_next;
        
        //  2F: cpl (M:1 T:4)
        // -- overlapped
        case  207: _z80_cpl(cpu);goto fetch_next;
        
        //  30: jr nc,d (M:3 T:12)
        // -- mread
        case  208: goto step_next;
        case  209: _wait();_mread(cpu->pc++);goto step_next;
        case  210: cpu->dlatch=_gd();if(!(_cc_nc)){_z80_skip(cpu,5);};goto step_next;
        // -- generic
        case  211: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;;goto step_next;
        case  212: goto step_next;
        case  213: goto step_next;
        case  214: goto step_next;
        case  215: goto step_next;
        // -- overlapped
        case  216: goto fetch_next;
        
        //  31: ld sp,nn (M:3 T:10)
        // -- mread
        case  217: goto step_next;
        case  218: _wait();_mread(cpu->pc++);goto step_next;
        case  219: cpu->spl=_gd();;goto step_next;
        // -- mread
        case  220: goto step_next;
        case  221: _wait();_mread(cpu->pc++);goto step_next;
        case  222: cpu->sph=_gd();;goto step_next;
        // -- overlapped
        case  223: goto fetch_next;
        
        //  32: ld (nn),a (M:4 T:13)
        // -- mread
        case  224: goto step_next;
        case  225: _wait();_mread(cpu->pc++);goto step_next;
        case  226: cpu->wzl=_gd();;goto step_next;
        // -- mread
        case  227: goto step_next;
        case  228: _wait();_mread(cpu->pc++);goto step_next;
        case  229: cpu->wzh=_gd();;goto step_next;
        // -- mwrite
        case  230: goto step_next;
        case  231: _wait();_mwrite(cpu->wz++,cpu->a);cpu->wzh=cpu->a;goto step_next;
        case  232: goto step_next;
        // -- overlapped
        case  233: goto fetch_next;
        
        //  33: inc sp (M:2 T:6)
        // -- generic
        case  234: cpu->sp++;goto step_next;
        case  235: goto step_next;
        // -- overlapped
        case  236: goto fetch_next;
        
        //  34: inc (hl) (M:3 T:11)
        // -- mread
        case  237: goto step_next;
        case  238: _wait();_mread(cpu->addr);goto step_next;
        case  239: cpu->dlatch=_gd();cpu->dlatch=_z80_inc8(cpu,cpu->dlatch);goto step_next;
        case  240: goto step_next;
        // -- mwrite
        case  241: goto step_next;
        case  242: _wait();_mwrite(cpu->addr,cpu->dlatch);;goto step_next;
        case  243: goto step_next;
        // -- overlapped
        case  244: goto fetch_next;
        
        //  35: dec (hl) (M:3 T:11)
        // -- mread
        case  245: goto step_next;
        case  246: _wait();_mread(cpu->addr);goto step_next;
        case  247: cpu->dlatch=_gd();cpu->dlatch=_z80_dec8(cpu,cpu->dlatch);goto step_next;
        case  248: goto step_next;
        // -- mwrite
        case  249: goto step_next;
        case  250: _wait();_mwrite(cpu->addr,cpu->dlatch);;goto step_next;
        case  251: goto step_next;
        // -- overlapped
        case  252: goto fetch_next;
        
        //  36: ld (hl),n (M:3 T:10)
        // -- mread
        case  253: goto step_next;
        case  254: _wait();_mread(cpu->pc++);goto step_next;
        case  255: cpu->dlatch=_gd();;goto step_next;
        // -- mwrite
        case  256: goto step_next;
        case  257: _wait();_mwrite(cpu->addr,cpu->dlatch);;goto step_next;
        case  258: goto step_next;
        // -- overlapped
        case  259: goto fetch_next;
        
        //  37: scf (M:1 T:4)
        // -- overlapped
        case  260: _z80_scf(cpu);goto fetch_next;
        
        //  38: jr c,d (M:3 T:12)
        // -- mread
        case  261: goto step_next;
        case  262: _wait();_mread(cpu->pc++);goto step_next;
        case  263: cpu->dlatch=_gd();if(!(_cc_c)){_z80_skip(cpu,5);};goto step_next;
        // -- generic
        case  264: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;;goto step_next;
        case  265: goto step_next;
        case  266: goto step_next;
        case  267: goto step_next;
        case  268: goto step_next;
        // -- overlapped
        case  269: goto fetch_next;
        
        //  39: add hl,sp (M:2 T:11)
        // -- generic
        case  270: _z80_add16(cpu,cpu->sp);goto step_next;
        case  271: goto step_next;
        case  272: goto step_next;
        case  273: goto step_next;
        case  274: goto step_next;
        case  275: goto step_next;
        case  276: goto step_next;
        // -- overlapped
        case  277: goto fetch_next;
        
        //  3A: ld a,(nn) (M:4 T:13)
        // -- mread
        case  278: goto step_next;
        case  279: _wait();_mread(cpu->pc++);goto step_next;
        case  280: cpu->wzl=_gd();;goto step_next;
        // -- mread
        case  281: goto step_next;
        case  282: _wait();_mread(cpu->pc++);goto step_next;
        case  283: cpu->wzh=_gd();;goto step_next;
        // -- mread
        case  284: goto step_next;
        case  285: _wait();_mread(cpu->wz++);goto step_next;
        case  286: cpu->a=_gd();;goto step_next;
        // -- overlapped
        case  287: goto fetch_next;
        
        //  3B: dec sp (M:2 T:6)
        // -- generic
        case  288: cpu->sp--;goto step_next;
        case  289: goto step_next;
        // -- overlapped
        case  290: goto fetch_next;
        
        //  3C: inc a (M:1 T:4)
        // -- overlapped
        case  291: cpu->a=_z80_inc8(cpu,cpu->a);goto fetch_next;
        
        //  3D: dec a (M:1 T:4)
        // -- overlapped
        case  292: cpu->a=_z80_dec8(cpu,cpu->a);goto fetch_next;
        
        //  3E: ld a,n (M:2 T:7)
        // -- mread
        case  293: goto step_next;
        case  294: _wait();_mread(cpu->pc++);goto step_next;
        case  295: cpu->a=_gd();;goto step_next;
        // -- overlapped
        case  296: goto fetch_next;
        
        //  3F: ccf (M:1 T:4)
        // -- overlapped
        case  297: _z80_ccf(cpu);goto fetch_next;
        
        //  40: ld b,b (M:1 T:4)
        // -- overlapped
        case  298: cpu->b=cpu->b;goto fetch_next;
        
        //  41: ld b,c (M:1 T:4)
        // -- overlapped
        case  299: cpu->b=cpu->c;goto fetch_next;
        
        //  42: ld b,d (M:1 T:4)
        // -- overlapped
        case  300: cpu->b=cpu->d;goto fetch_next;
        
        //  43: ld b,e (M:1 T:4)
        // -- overlapped
        case  301: cpu->b=cpu->e;goto fetch_next;
        
        //  44: ld b,h (M:1 T:4)
        // -- overlapped
        case  302: cpu->b=cpu->hlx[cpu->hlx_idx].h;goto fetch_next;
        
        //  45: ld b,l (M:1 T:4)
        // -- overlapped
        case  303: cpu->b=cpu->hlx[cpu->hlx_idx].l;goto fetch_next;
        
        //  46: ld b,(hl) (M:2 T:7)
        // -- mread
        case  304: goto step_next;
        case  305: _wait();_mread(cpu->addr);goto step_next;
        case  306: cpu->b=_gd();;goto step_next;
        // -- overlapped
        case  307: goto fetch_next;
        
        //  47: ld b,a (M:1 T:4)
        // -- overlapped
        case  308: cpu->b=cpu->a;goto fetch_next;
        
        //  48: ld c,b (M:1 T:4)
        // -- overlapped
        case  309: cpu->c=cpu->b;goto fetch_next;
        
        //  49: ld c,c (M:1 T:4)
        // -- overlapped
        case  310: cpu->c=cpu->c;goto fetch_next;
        
        //  4A: ld c,d (M:1 T:4)
        // -- overlapped
        case  311: cpu->c=cpu->d;goto fetch_next;
        
        //  4B: ld c,e (M:1 T:4)
        // -- overlapped
        case  312: cpu->c=cpu->e;goto fetch_next;
        
        //  4C: ld c,h (M:1 T:4)
        // -- overlapped
        case  313: cpu->c=cpu->hlx[cpu->hlx_idx].h;goto fetch_next;
        
        //  4D: ld c,l (M:1 T:4)
        // -- overlapped
        case  314: cpu->c=cpu->hlx[cpu->hlx_idx].l;goto fetch_next;
        
        //  4E: ld c,(hl) (M:2 T:7)
        // -- mread
        case  315: goto step_next;
        case  316: _wait();_mread(cpu->addr);goto step_next;
        case  317: cpu->c=_gd();;goto step_next;
        // -- overlapped
        case  318: goto fetch_next;
        
        //  4F: ld c,a (M:1 T:4)
        // -- overlapped
        case  319: cpu->c=cpu->a;goto fetch_next;
        
        //  50: ld d,b (M:1 T:4)
        // -- overlapped
        case  320: cpu->d=cpu->b;goto fetch_next;
        
        //  51: ld d,c (M:1 T:4)
        // -- overlapped
        case  321: cpu->d=cpu->c;goto fetch_next;
        
        //  52: ld d,d (M:1 T:4)
        // -- overlapped
        case  322: cpu->d=cpu->d;goto fetch_next;
        
        //  53: ld d,e (M:1 T:4)
        // -- overlapped
        case  323: cpu->d=cpu->e;goto fetch_next;
        
        //  54: ld d,h (M:1 T:4)
        // -- overlapped
        case  324: cpu->d=cpu->hlx[cpu->hlx_idx].h;goto fetch_next;
        
        //  55: ld d,l (M:1 T:4)
        // -- overlapped
        case  325: cpu->d=cpu->hlx[cpu->hlx_idx].l;goto fetch_next;
        
        //  56: ld d,(hl) (M:2 T:7)
        // -- mread
        case  326: goto step_next;
        case  327: _wait();_mread(cpu->addr);goto step_next;
        case  328: cpu->d=_gd();;goto step_next;
        // -- overlapped
        case  329: goto fetch_next;
        
        //  57: ld d,a (M:1 T:4)
        // -- overlapped
        case  330: cpu->d=cpu->a;goto fetch_next;
        
        //  58: ld e,b (M:1 T:4)
        // -- overlapped
        case  331: cpu->e=cpu->b;goto fetch_next;
        
        //  59: ld e,c (M:1 T:4)
        // -- overlapped
        case  332: cpu->e=cpu->c;goto fetch_next;
        
        //  5A: ld e,d (M:1 T:4)
        // -- overlapped
        case  333: cpu->e=cpu->d;goto fetch_next;
        
        //  5B: ld e,e (M:1 T:4)
        // -- overlapped
        case  334: cpu->e=cpu->e;goto fetch_next;
        
        //  5C: ld e,h (M:1 T:4)
        // -- overlapped
        case  335: cpu->e=cpu->hlx[cpu->hlx_idx].h;goto fetch_next;
        
        //  5D: ld e,l (M:1 T:4)
        // -- overlapped
        case  336: cpu->e=cpu->hlx[cpu->hlx_idx].l;goto fetch_next;
        
        //  5E: ld e,(hl) (M:2 T:7)
        // -- mread
        case  337: goto step_next;
        case  338: _wait();_mread(cpu->addr);goto step_next;
        case  339: cpu->e=_gd();;goto step_next;
        // -- overlapped
        case  340: goto fetch_next;
        
        //  5F: ld e,a (M:1 T:4)
        // -- overlapped
        case  341: cpu->e=cpu->a;goto fetch_next;
        
        //  60: ld h,b (M:1 T:4)
        // -- overlapped
        case  342: cpu->hlx[cpu->hlx_idx].h=cpu->b;goto fetch_next;
        
        //  61: ld h,c (M:1 T:4)
        // -- overlapped
        case  343: cpu->hlx[cpu->hlx_idx].h=cpu->c;goto fetch_next;
        
        //  62: ld h,d (M:1 T:4)
        // -- overlapped
        case  344: cpu->hlx[cpu->hlx_idx].h=cpu->d;goto fetch_next;
        
        //  63: ld h,e (M:1 T:4)
        // -- overlapped
        case  345: cpu->hlx[cpu->hlx_idx].h=cpu->e;goto fetch_next;
        
        //  64: ld h,h (M:1 T:4)
        // -- overlapped
        case  346: cpu->hlx[cpu->hlx_idx].h=cpu->hlx[cpu->hlx_idx].h;goto fetch_next;
        
        //  65: ld h,l (M:1 T:4)
        // -- overlapped
        case  347: cpu->hlx[cpu->hlx_idx].h=cpu->hlx[cpu->hlx_idx].l;goto fetch_next;
        
        //  66: ld h,(hl) (M:2 T:7)
        // -- mread
        case  348: goto step_next;
        case  349: _wait();_mread(cpu->addr);goto step_next;
        case  350: cpu->h=_gd();;goto step_next;
        // -- overlapped
        case  351: goto fetch_next;
        
        //  67: ld h,a (M:1 T:4)
        // -- overlapped
        case  352: cpu->hlx[cpu->hlx_idx].h=cpu->a;goto fetch_next;
        
        //  68: ld l,b (M:1 T:4)
        // -- overlapped
        case  353: cpu->hlx[cpu->hlx_idx].l=cpu->b;goto fetch_next;
        
        //  69: ld l,c (M:1 T:4)
        // -- overlapped
        case  354: cpu->hlx[cpu->hlx_idx].l=cpu->c;goto fetch_next;
        
        //  6A: ld l,d (M:1 T:4)
        // -- overlapped
        case  355: cpu->hlx[cpu->hlx_idx].l=cpu->d;goto fetch_next;
        
        //  6B: ld l,e (M:1 T:4)
        // -- overlapped
        case  356: cpu->hlx[cpu->hlx_idx].l=cpu->e;goto fetch_next;
        
        //  6C: ld l,h (M:1 T:4)
        // -- overlapped
        case  357: cpu->hlx[cpu->hlx_idx].l=cpu->hlx[cpu->hlx_idx].h;goto fetch_next;
        
        //  6D: ld l,l (M:1 T:4)
        // -- overlapped
        case  358: cpu->hlx[cpu->hlx_idx].l=cpu->hlx[cpu->hlx_idx].l;goto fetch_next;
        
        //  6E: ld l,(hl) (M:2 T:7)
        // -- mread
        case  359: goto step_next;
        case  360: _wait();_mread(cpu->addr);goto step_next;
        case  361: cpu->l=_gd();;goto step_next;
        // -- overlapped
        case  362: goto fetch_next;
        
        //  6F: ld l,a (M:1 T:4)
        // -- overlapped
        case  363: cpu->hlx[cpu->hlx_idx].l=cpu->a;goto fetch_next;
        
        //  70: ld (hl),b (M:2 T:7)
        // -- mwrite
        case  364: goto step_next;
        case  365: _wait();_mwrite(cpu->addr,cpu->b);;goto step_next;
        case  366: goto step_next;
        // -- overlapped
        case  367: goto fetch_next;
        
        //  71: ld (hl),c (M:2 T:7)
        // -- mwrite
        case  368: goto step_next;
        case  369: _wait();_mwrite(cpu->addr,cpu->c);;goto step_next;
        case  370: goto step_next;
        // -- overlapped
        case  371: goto fetch_next;
        
        //  72: ld (hl),d (M:2 T:7)
        // -- mwrite
        case  372: goto step_next;
        case  373: _wait();_mwrite(cpu->addr,cpu->d);;goto step_next;
        case  374: goto step_next;
        // -- overlapped
        case  375: goto fetch_next;
        
        //  73: ld (hl),e (M:2 T:7)
        // -- mwrite
        case  376: goto step_next;
        case  377: _wait();_mwrite(cpu->addr,cpu->e);;goto step_next;
        case  378: goto step_next;
        // -- overlapped
        case  379: goto fetch_next;
        
        //  74: ld (hl),h (M:2 T:7)
        // -- mwrite
        case  380: goto step_next;
        case  381: _wait();_mwrite(cpu->addr,cpu->h);;goto step_next;
        case  382: goto step_next;
        // -- overlapped
        case  383: goto fetch_next;
        
        //  75: ld (hl),l (M:2 T:7)
        // -- mwrite
        case  384: goto step_next;
        case  385: _wait();_mwrite(cpu->addr,cpu->l);;goto step_next;
        case  386: goto step_next;
        // -- overlapped
        case  387: goto fetch_next;
        
        //  76: halt (M:1 T:4)
        // -- overlapped
        case  388: pins=_z80_halt(cpu,pins);goto fetch_next;
        
        //  77: ld (hl),a (M:2 T:7)
        // -- mwrite
        case  389: goto step_next;
        case  390: _wait();_mwrite(cpu->addr,cpu->a);;goto step_next;
        case  391: goto step_next;
        // -- overlapped
        case  392: goto fetch_next;
        
        //  78: ld a,b (M:1 T:4)
        // -- overlapped
        case  393: cpu->a=cpu->b;goto fetch_next;
        
        //  79: ld a,c (M:1 T:4)
        // -- overlapped
        case  394: cpu->a=cpu->c;goto fetch_next;
        
        //  7A: ld a,d (M:1 T:4)
        // -- overlapped
        case  395: cpu->a=cpu->d;goto fetch_next;
        
        //  7B: ld a,e (M:1 T:4)
        // -- overlapped
        case  396: cpu->a=cpu->e;goto fetch_next;
        
        //  7C: ld a,h (M:1 T:4)
        // -- overlapped
        case  397: cpu->a=cpu->hlx[cpu->hlx_idx].h;goto fetch_next;
        
        //  7D: ld a,l (M:1 T:4)
        // -- overlapped
        case  398: cpu->a=cpu->hlx[cpu->hlx_idx].l;goto fetch_next;
        
        //  7E: ld a,(hl) (M:2 T:7)
        // -- mread
        case  399: goto step_next;
        case  400: _wait();_mread(cpu->addr);goto step_next;
        case  401: cpu->a=_gd();;goto step_next;
        // -- overlapped
        case  402: goto fetch_next;
        
        //  7F: ld a,a (M:1 T:4)
        // -- overlapped
        case  403: cpu->a=cpu->a;goto fetch_next;
        
        //  80: add b (M:1 T:4)
        // -- overlapped
        case  404: _z80_add8(cpu,cpu->b);goto fetch_next;
        
        //  81: add c (M:1 T:4)
        // -- overlapped
        case  405: _z80_add8(cpu,cpu->c);goto fetch_next;
        
        //  82: add d (M:1 T:4)
        // -- overlapped
        case  406: _z80_add8(cpu,cpu->d);goto fetch_next;
        
        //  83: add e (M:1 T:4)
        // -- overlapped
        case  407: _z80_add8(cpu,cpu->e);goto fetch_next;
        
        //  84: add h (M:1 T:4)
        // -- overlapped
        case  408: _z80_add8(cpu,cpu->hlx[cpu->hlx_idx].h);goto fetch_next;
        
        //  85: add l (M:1 T:4)
        // -- overlapped
        case  409: _z80_add8(cpu,cpu->hlx[cpu->hlx_idx].l);goto fetch_next;
        
        //  86: add (hl) (M:2 T:7)
        // -- mread
        case  410: goto step_next;
        case  411: _wait();_mread(cpu->addr);goto step_next;
        case  412: cpu->dlatch=_gd();;goto step_next;
        // -- overlapped
        case  413: _z80_add8(cpu,cpu->dlatch);goto fetch_next;
        
        //  87: add a (M:1 T:4)
        // -- overlapped
        case  414: _z80_add8(cpu,cpu->a);goto fetch_next;
        
        //  88: adc b (M:1 T:4)
        // -- overlapped
        case  415: _z80_adc8(cpu,cpu->b);goto fetch_next;
        
        //  89: adc c (M:1 T:4)
        // -- overlapped
        case  416: _z80_adc8(cpu,cpu->c);goto fetch_next;
        
        //  8A: adc d (M:1 T:4)
        // -- overlapped
        case  417: _z80_adc8(cpu,cpu->d);goto fetch_next;
        
        //  8B: adc e (M:1 T:4)
        // -- overlapped
        case  418: _z80_adc8(cpu,cpu->e);goto fetch_next;
        
        //  8C: adc h (M:1 T:4)
        // -- overlapped
        case  419: _z80_adc8(cpu,cpu->hlx[cpu->hlx_idx].h);goto fetch_next;
        
        //  8D: adc l (M:1 T:4)
        // -- overlapped
        case  420: _z80_adc8(cpu,cpu->hlx[cpu->hlx_idx].l);goto fetch_next;
        
        //  8E: adc (hl) (M:2 T:7)
        // -- mread
        case  421: goto step_next;
        case  422: _wait();_mread(cpu->addr);goto step_next;
        case  423: cpu->dlatch=_gd();;goto step_next;
        // -- overlapped
        case  424: _z80_adc8(cpu,cpu->dlatch);goto fetch_next;
        
        //  8F: adc a (M:1 T:4)
        // -- overlapped
        case  425: _z80_adc8(cpu,cpu->a);goto fetch_next;
        
        //  90: sub b (M:1 T:4)
        // -- overlapped
        case  426: _z80_sub8(cpu,cpu->b);goto fetch_next;
        
        //  91: sub c (M:1 T:4)
        // -- overlapped
        case  427: _z80_sub8(cpu,cpu->c);goto fetch_next;
        
        //  92: sub d (M:1 T:4)
        // -- overlapped
        case  428: _z80_sub8(cpu,cpu->d);goto fetch_next;
        
        //  93: sub e (M:1 T:4)
        // -- overlapped
        case  429: _z80_sub8(cpu,cpu->e);goto fetch_next;
        
        //  94: sub h (M:1 T:4)
        // -- overlapped
        case  430: _z80_sub8(cpu,cpu->hlx[cpu->hlx_idx].h);goto fetch_next;
        
        //  95: sub l (M:1 T:4)
        // -- overlapped
        case  431: _z80_sub8(cpu,cpu->hlx[cpu->hlx_idx].l);goto fetch_next;
        
        //  96: sub (hl) (M:2 T:7)
        // -- mread
        case  432: goto step_next;
        case  433: _wait();_mread(cpu->addr);goto step_next;
        case  434: cpu->dlatch=_gd();;goto step_next;
        // -- overlapped
        case  435: _z80_sub8(cpu,cpu->dlatch);goto fetch_next;
        
        //  97: sub a (M:1 T:4)
        // -- overlapped
        case  436: _z80_sub8(cpu,cpu->a);goto fetch_next;
        
        //  98: sbc b (M:1 T:4)
        // -- overlapped
        case  437: _z80_sbc8(cpu,cpu->b);goto fetch_next;
        
        //  99: sbc c (M:1 T:4)
        // -- overlapped
        case  438: _z80_sbc8(cpu,cpu->c);goto fetch_next;
        
        //  9A: sbc d (M:1 T:4)
        // -- overlapped
        case  439: _z80_sbc8(cpu,cpu->d);goto fetch_next;
        
        //  9B: sbc e (M:1 T:4)
        // -- overlapped
        case  440: _z80_sbc8(cpu,cpu->e);goto fetch_next;
        
        //  9C: sbc h (M:1 T:4)
        // -- overlapped
        case  441: _z80_sbc8(cpu,cpu->hlx[cpu->hlx_idx].h);goto fetch_next;
        
        //  9D: sbc l (M:1 T:4)
        // -- overlapped
        case  442: _z80_sbc8(cpu,cpu->hlx[cpu->hlx_idx].l);goto fetch_next;
        
        //  9E: sbc (hl) (M:2 T:7)
        // -- mread
        case  443: goto step_next;
        case  444: _wait();_mread(cpu->addr);goto step_next;
        case  445: cpu->dlatch=_gd();;goto step_next;
        // -- overlapped
        case  446: _z80_sbc8(cpu,cpu->dlatch);goto fetch_next;
        
        //  9F: sbc a (M:1 T:4)
        // -- overlapped
        case  447: _z80_sbc8(cpu,cpu->a);goto fetch_next;
        
        //  A0: and b (M:1 T:4)
        // -- overlapped
        case  448: _z80_and8(cpu,cpu->b);goto fetch_next;
        
        //  A1: and c (M:1 T:4)
        // -- overlapped
        case  449: _z80_and8(cpu,cpu->c);goto fetch_next;
        
        //  A2: and d (M:1 T:4)
        // -- overlapped
        case  450: _z80_and8(cpu,cpu->d);goto fetch_next;
        
        //  A3: and e (M:1 T:4)
        // -- overlapped
        case  451: _z80_and8(cpu,cpu->e);goto fetch_next;
        
        //  A4: and h (M:1 T:4)
        // -- overlapped
        case  452: _z80_and8(cpu,cpu->hlx[cpu->hlx_idx].h);goto fetch_next;
        
        //  A5: and l (M:1 T:4)
        // -- overlapped
        case  453: _z80_and8(cpu,cpu->hlx[cpu->hlx_idx].l);goto fetch_next;
        
        //  A6: and (hl) (M:2 T:7)
        // -- mread
        case  454: goto step_next;
        case  455: _wait();_mread(cpu->addr);goto step_next;
        case  456: cpu->dlatch=_gd();;goto step_next;
        // -- overlapped
        case  457: _z80_and8(cpu,cpu->dlatch);goto fetch_next;
        
        //  A7: and a (M:1 T:4)
        // -- overlapped
        case  458: _z80_and8(cpu,cpu->a);goto fetch_next;
        
        //  A8: xor b (M:1 T:4)
        // -- overlapped
        case  459: _z80_xor8(cpu,cpu->b);goto fetch_next;
        
        //  A9: xor c (M:1 T:4)
        // -- overlapped
        case  460: _z80_xor8(cpu,cpu->c);goto fetch_next;
        
        //  AA: xor d (M:1 T:4)
        // -- overlapped
        case  461: _z80_xor8(cpu,cpu->d);goto fetch_next;
        
        //  AB: xor e (M:1 T:4)
        // -- overlapped
        case  462: _z80_xor8(cpu,cpu->e);goto fetch_next;
        
        //  AC: xor h (M:1 T:4)
        // -- overlapped
        case  463: _z80_xor8(cpu,cpu->hlx[cpu->hlx_idx].h);goto fetch_next;
        
        //  AD: xor l (M:1 T:4)
        // -- overlapped
        case  464: _z80_xor8(cpu,cpu->hlx[cpu->hlx_idx].l);goto fetch_next;
        
        //  AE: xor (hl) (M:2 T:7)
        // -- mread
        case  465: goto step_next;
        case  466: _wait();_mread(cpu->addr);goto step_next;
        case  467: cpu->dlatch=_gd();;goto step_next;
        // -- overlapped
        case  468: _z80_xor8(cpu,cpu->dlatch);goto fetch_next;
        
        //  AF: xor a (M:1 T:4)
        // -- overlapped
        case  469: _z80_xor8(cpu,cpu->a);goto fetch_next;
        
        //  B0: or b (M:1 T:4)
        // -- overlapped
        case  470: _z80_or8(cpu,cpu->b);goto fetch_next;
        
        //  B1: or c (M:1 T:4)
        // -- overlapped
        case  471: _z80_or8(cpu,cpu->c);goto fetch_next;
        
        //  B2: or d (M:1 T:4)
        // -- overlapped
        case  472: _z80_or8(cpu,cpu->d);goto fetch_next;
        
        //  B3: or e (M:1 T:4)
        // -- overlapped
        case  473: _z80_or8(cpu,cpu->e);goto fetch_next;
        
        //  B4: or h (M:1 T:4)
        // -- overlapped
        case  474: _z80_or8(cpu,cpu->hlx[cpu->hlx_idx].h);goto fetch_next;
        
        //  B5: or l (M:1 T:4)
        // -- overlapped
        case  475: _z80_or8(cpu,cpu->hlx[cpu->hlx_idx].l);goto fetch_next;
        
        //  B6: or (hl) (M:2 T:7)
        // -- mread
        case  476: goto step_next;
        case  477: _wait();_mread(cpu->addr);goto step_next;
        case  478: cpu->dlatch=_gd();;goto step_next;
        // -- overlapped
        case  479: _z80_or8(cpu,cpu->dlatch);goto fetch_next;
        
        //  B7: or a (M:1 T:4)
        // -- overlapped
        case  480: _z80_or8(cpu,cpu->a);goto fetch_next;
        
        //  B8: cp b (M:1 T:4)
        // -- overlapped
        case  481: _z80_cp8(cpu,cpu->b);goto fetch_next;
        
        //  B9: cp c (M:1 T:4)
        // -- overlapped
        case  482: _z80_cp8(cpu,cpu->c);goto fetch_next;
        
        //  BA: cp d (M:1 T:4)
        // -- overlapped
        case  483: _z80_cp8(cpu,cpu->d);goto fetch_next;
        
        //  BB: cp e (M:1 T:4)
        // -- overlapped
        case  484: _z80_cp8(cpu,cpu->e);goto fetch_next;
        
        //  BC: cp h (M:1 T:4)
        // -- overlapped
        case  485: _z80_cp8(cpu,cpu->hlx[cpu->hlx_idx].h);goto fetch_next;
        
        //  BD: cp l (M:1 T:4)
        // -- overlapped
        case  486: _z80_cp8(cpu,cpu->hlx[cpu->hlx_idx].l);goto fetch_next;
        
        //  BE: cp (hl) (M:2 T:7)
        // -- mread
        case  487: goto step_next;
        case  488: _wait();_mread(cpu->addr);goto step_next;
        case  489: cpu->dlatch=_gd();;goto step_next;
        // -- overlapped
        case  490: _z80_cp8(cpu,cpu->dlatch);goto fetch_next;
        
        //  BF: cp a (M:1 T:4)
        // -- overlapped
        case  491: _z80_cp8(cpu,cpu->a);goto fetch_next;
        
        //  C0: ret nz (M:4 T:11)
        // -- generic
        case  492: if(!_cc_nz){_z80_skip(cpu,6);};goto step_next;
        // -- mread
        case  493: goto step_next;
        case  494: _wait();_mread(cpu->sp++);goto step_next;
        case  495: cpu->wzl=_gd();;goto step_next;
        // -- mread
        case  496: goto step_next;
        case  497: _wait();_mread(cpu->sp++);goto step_next;
        case  498: cpu->wzh=_gd();cpu->pc=cpu->wz;goto step_next;
        // -- overlapped
        case  499: goto fetch_next;
        
        //  C1: pop bc (M:3 T:10)
        // -- mread
        case  500: goto step_next;
        case  501: _wait();_mread(cpu->sp++);goto step_next;
        case  502: cpu->c=_gd();;goto step_next;
        // -- mread
        case  503: goto step_next;
        case  504: _wait();_mread(cpu->sp++);goto step_next;
        case  505: cpu->b=_gd();;goto step_next;
        // -- overlapped
        case  506: goto fetch_next;
        
        //  C2: jp nz,nn (M:3 T:10)
        // -- mread
        case  507: goto step_next;
        case  508: _wait();_mread(cpu->pc++);goto step_next;
        case  509: cpu->wzl=_gd();;goto step_next;
        // -- mread
        case  510: goto step_next;
        case  511: _wait();_mread(cpu->pc++);goto step_next;
        case  512: cpu->wzh=_gd();if(_cc_nz){cpu->pc=cpu->wz;};goto step_next;
        // -- overlapped
        case  513: goto fetch_next;
        
        //  C3: jp nn (M:3 T:10)
        // -- mread
        case  514: goto step_next;
        case  515: _wait();_mread(cpu->pc++);goto step_next;
        case  516: cpu->wzl=_gd();;goto step_next;
        // -- mread
        case  517: goto step_next;
        case  518: _wait();_mread(cpu->pc++);goto step_next;
        case  519: cpu->wzh=_gd();cpu->pc=cpu->wz;goto step_next;
        // -- overlapped
        case  520: goto fetch_next;
        
        //  C4: call nz,nn (M:6 T:17)
        // -- mread
        case  521: goto step_next;
        case  522: _wait();_mread(cpu->pc++);goto step_next;
        case  523: cpu->wzl=_gd();;goto step_next;
        // -- mread
        case  524: goto step_next;
        case  525: _wait();_mread(cpu->pc++);goto step_next;
        case  526: cpu->wzh=_gd();if (!_cc_nz){_z80_skip(cpu,7);};goto step_next;
        // -- generic
        case  527: ;goto step_next;
        // -- mwrite
        case  528: goto step_next;
        case  529: _wait();_mwrite(--cpu->sp,cpu->pch);;goto step_next;
        case  530: goto step_next;
        // -- mwrite
        case  531: goto step_next;
        case  532: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz;goto step_next;
        case  533: goto step_next;
        // -- overlapped
        case  534: goto fetch_next;
        
        //  C5: push bc (M:4 T:11)
        // -- generic
        case  535: ;goto step_next;
        // -- mwrite
        case  536: goto step_next;
        case  537: _wait();_mwrite(--cpu->sp,cpu->b);;goto step_next;
        case  538: goto step_next;
        // -- mwrite
        case  539: goto step_next;
        case  540: _wait();_mwrite(--cpu->sp,cpu->c);;goto step_next;
        case  541: goto step_next;
        // -- overlapped
        case  542: goto fetch_next;
        
        //  C6: add n (M:2 T:7)
        // -- mread
        case  543: goto step_next;
        case  544: _wait();_mread(cpu->pc++);goto step_next;
        case  545: cpu->dlatch=_gd();;goto step_next;
        // -- overlapped
        case  546: _z80_add8(cpu,cpu->dlatch);goto fetch_next;
        
        //  C7: rst 0h (M:4 T:11)
        // -- generic
        case  547: ;goto step_next;
        // -- mwrite
        case  548: goto step_next;
        case  549: _wait();_mwrite(--cpu->sp,cpu->pch);;goto step_next;
        case  550: goto step_next;
        // -- mwrite
        case  551: goto step_next;
        case  552: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x00;cpu->pc=cpu->wz;goto step_next;
        case  553: goto step_next;
        // -- overlapped
        case  554: goto fetch_next;
        
        //  C8: ret z (M:4 T:11)
        // -- generic
        case  555: if(!_cc_z){_z80_skip(cpu,6);};goto step_next;
        // -- mread
        case  556: goto step_next;
        case  557: _wait();_mread(cpu->sp++);goto step_next;
        case  558: cpu->wzl=_gd();;goto step_next;
        // -- mread
        case  559: goto step_next;
        case  560: _wait();_mread(cpu->sp++);goto step_next;
        case  561: cpu->wzh=_gd();cpu->pc=cpu->wz;goto step_next;
        // -- overlapped
        case  562: goto fetch_next;
        
        //  C9: ret (M:3 T:10)
        // -- mread
        case  563: goto step_next;
        case  564: _wait();_mread(cpu->sp++);goto step_next;
        case  565: cpu->wzl=_gd();;goto step_next;
        // -- mread
        case  566: goto step_next;
        case  567: _wait();_mread(cpu->sp++);goto step_next;
        case  568: cpu->wzh=_gd();cpu->pc=cpu->wz;goto step_next;
        // -- overlapped
        case  569: goto fetch_next;
        
        //  CA: jp z,nn (M:3 T:10)
        // -- mread
        case  570: goto step_next;
        case  571: _wait();_mread(cpu->pc++);goto step_next;
        case  572: cpu->wzl=_gd();;goto step_next;
        // -- mread
        case  573: goto step_next;
        case  574: _wait();_mread(cpu->pc++);goto step_next;
        case  575: cpu->wzh=_gd();if(_cc_z){cpu->pc=cpu->wz;};goto step_next;
        // -- overlapped
        case  576: goto fetch_next;
        
        //  CB: cb prefix (M:1 T:4)
        // -- overlapped
        case  577: ;_fetch_cb();goto step_next;
        
        //  CC: call z,nn (M:6 T:17)
        // -- mread
        case  578: goto step_next;
        case  579: _wait();_mread(cpu->pc++);goto step_next;
        case  580: cpu->wzl=_gd();;goto step_next;
        // -- mread
        case  581: goto step_next;
        case  582: _wait();_mread(cpu->pc++);goto step_next;
        case  583: cpu->wzh=_gd();if (!_cc_z){_z80_skip(cpu,7);};goto step_next;
        // -- generic
        case  584: ;goto step_next;
        // -- mwrite
        case  585: goto step_next;
        case  586: _wait();_mwrite(--cpu->sp,cpu->pch);;goto step_next;
        case  587: goto step_next;
        // -- mwrite
        case  588: goto step_next;
        case  589: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz;goto step_next;
        case  590: goto step_next;
        // -- overlapped
        case  591: goto fetch_next;
        
        //  CD: call nn (M:5 T:17)
        // -- mread
        case  592: goto step_next;
        case  593: _wait();_mread(cpu->pc++);goto step_next;
        case  594: cpu->wzl=_gd();;goto step_next;
        // -- mread
        case  595: goto step_next;
        case  596: _wait();_mread(cpu->pc++);goto step_next;
        case  597: cpu->wzh=_gd();;goto step_next;
        case  598: goto step_next;
        // -- mwrite
        case  599: goto step_next;
        case  600: _wait();_mwrite(--cpu->sp,cpu->pch);;goto step_next;
        case  601: goto step_next;
        // -- mwrite
        case  602: goto step_next;
        case  603: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz;goto step_next;
        case  604: goto step_next;
        // -- overlapped
        case  605: goto fetch_next;
        
        //  CE: adc n (M:2 T:7)
        // -- mread
        case  606: goto step_next;
        case  607: _wait();_mread(cpu->pc++);goto step_next;
        case  608: cpu->dlatch=_gd();;goto step_next;
        // -- overlapped
        case  609: _z80_adc8(cpu,cpu->dlatch);goto fetch_next;
        
        //  CF: rst 8h (M:4 T:11)
        // -- generic
        case  610: ;goto step_next;
        // -- mwrite
        case  611: goto step_next;
        case  612: _wait();_mwrite(--cpu->sp,cpu->pch);;goto step_next;
        case  613: goto step_next;
        // -- mwrite
        case  614: goto step_next;
        case  615: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x08;cpu->pc=cpu->wz;goto step_next;
        case  616: goto step_next;
        // -- overlapped
        case  617: goto fetch_next;
        
        //  D0: ret nc (M:4 T:11)
        // -- generic
        case  618: if(!_cc_nc){_z80_skip(cpu,6);};goto step_next;
        // -- mread
        case  619: goto step_next;
        case  620: _wait();_mread(cpu->sp++);goto step_next;
        case  621: cpu->wzl=_gd();;goto step_next;
        // -- mread
        case  622: goto step_next;
        case  623: _wait();_mread(cpu->sp++);goto step_next;
        case  624: cpu->wzh=_gd();cpu->pc=cpu->wz;goto step_next;
        // -- overlapped
        case  625: goto fetch_next;
        
        //  D1: pop de (M:3 T:10)
        // -- mread
        case  626: goto step_next;
        case  627: _wait();_mread(cpu->sp++);goto step_next;
        case  628: cpu->e=_gd();;goto step_next;
        // -- mread
        case  629: goto step_next;
        case  630: _wait();_mread(cpu->sp++);goto step_next;
        case  631: cpu->d=_gd();;goto step_next;
        // -- overlapped
        case  632: goto fetch_next;
        
        //  D2: jp nc,nn (M:3 T:10)
        // -- mread
        case  633: goto step_next;
        case  634: _wait();_mread(cpu->pc++);goto step_next;
        case  635: cpu->wzl=_gd();;goto step_next;
        // -- mread
        case  636: goto step_next;
        case  637: _wait();_mread(cpu->pc++);goto step_next;
        case  638: cpu->wzh=_gd();if(_cc_nc){cpu->pc=cpu->wz;};goto step_next;
        // -- overlapped
        case  639: goto fetch_next;
        
        //  D3: out (n),a (M:3 T:11)
        // -- mread
        case  640: goto step_next;
        case  641: _wait();_mread(cpu->pc++);goto step_next;
        case  642: cpu->wzl=_gd();cpu->wzh=cpu->a;goto step_next;
        // -- iowrite
        case  643: goto step_next;
        case  644: _iowrite(cpu->wz,cpu->a);goto step_next;
        case  645: _wait();cpu->wzl++;goto step_next;
        case  646: goto step_next;
        // -- overlapped
        case  647: goto fetch_next;
        
        //  D4: call nc,nn (M:6 T:17)
        // -- mread
        case  648: goto step_next;
        case  649: _wait();_mread(cpu->pc++);goto step_next;
        case  650: cpu->wzl=_gd();;goto step_next;
        // -- mread
        case  651: goto step_next;
        case  652: _wait();_mread(cpu->pc++);goto step_next;
        case  653: cpu->wzh=_gd();if (!_cc_nc){_z80_skip(cpu,7);};goto step_next;
        // -- generic
        case  654: ;goto step_next;
        // -- mwrite
        case  655: goto step_next;
        case  656: _wait();_mwrite(--cpu->sp,cpu->pch);;goto step_next;
        case  657: goto step_next;
        // -- mwrite
        case  658: goto step_next;
        case  659: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz;goto step_next;
        case  660: goto step_next;
        // -- overlapped
        case  661: goto fetch_next;
        
        //  D5: push de (M:4 T:11)
        // -- generic
        case  662: ;goto step_next;
        // -- mwrite
        case  663: goto step_next;
        case  664: _wait();_mwrite(--cpu->sp,cpu->d);;goto step_next;
        case  665: goto step_next;
        // -- mwrite
        case  666: goto step_next;
        case  667: _wait();_mwrite(--cpu->sp,cpu->e);;goto step_next;
        case  668: goto step_next;
        // -- overlapped
        case  669: goto fetch_next;
        
        //  D6: sub n (M:2 T:7)
        // -- mread
        case  670: goto step_next;
        case  671: _wait();_mread(cpu->pc++);goto step_next;
        case  672: cpu->dlatch=_gd();;goto step_next;
        // -- overlapped
        case  673: _z80_sub8(cpu,cpu->dlatch);goto fetch_next;
        
        //  D7: rst 10h (M:4 T:11)
        // -- generic
        case  674: ;goto step_next;
        // -- mwrite
        case  675: goto step_next;
        case  676: _wait();_mwrite(--cpu->sp,cpu->pch);;goto step_next;
        case  677: goto step_next;
        // -- mwrite
        case  678: goto step_next;
        case  679: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x10;cpu->pc=cpu->wz;goto step_next;
        case  680: goto step_next;
        // -- overlapped
        case  681: goto fetch_next;
        
        //  D8: ret c (M:4 T:11)
        // -- generic
        case  682: if(!_cc_c){_z80_skip(cpu,6);};goto step_next;
        // -- mread
        case  683: goto step_next;
        case  684: _wait();_mread(cpu->sp++);goto step_next;
        case  685: cpu->wzl=_gd();;goto step_next;
        // -- mread
        case  686: goto step_next;
        case  687: _wait();_mread(cpu->sp++);goto step_next;
        case  688: cpu->wzh=_gd();cpu->pc=cpu->wz;goto step_next;
        // -- overlapped
        case  689: goto fetch_next;
        
        //  D9: exx (M:1 T:4)
        // -- overlapped
        case  690: _z80_exx(cpu);goto fetch_next;
        
        //  DA: jp c,nn (M:3 T:10)
        // -- mread
        case  691: goto step_next;
        case  692: _wait();_mread(cpu->pc++);goto step_next;
        case  693: cpu->wzl=_gd();;goto step_next;
        // -- mread
        case  694: goto step_next;
        case  695: _wait();_mread(cpu->pc++);goto step_next;
        case  696: cpu->wzh=_gd();if(_cc_c){cpu->pc=cpu->wz;};goto step_next;
        // -- overlapped
        case  697: goto fetch_next;
        
        //  DB: in a,(n) (M:3 T:11)
        // -- mread
        case  698: goto step_next;
        case  699: _wait();_mread(cpu->pc++);goto step_next;
        case  700: cpu->wzl=_gd();cpu->wzh=cpu->a;goto step_next;
        // -- ioread
        case  701: goto step_next;
        case  702: goto step_next;
        case  703: _wait();_ioread(cpu->wz++);goto step_next;
        case  704: cpu->dlatch=_gd();;goto step_next;
        // -- overlapped
        case  705: cpu->a=cpu->dlatch;goto fetch_next;
        
        //  DC: call c,nn (M:6 T:17)
        // -- mread
        case  706: goto step_next;
        case  707: _wait();_mread(cpu->pc++);goto step_next;
        case  708: cpu->wzl=_gd();;goto step_next;
        // -- mread
        case  709: goto step_next;
        case  710: _wait();_mread(cpu->pc++);goto step_next;
        case  711: cpu->wzh=_gd();if (!_cc_c){_z80_skip(cpu,7);};goto step_next;
        // -- generic
        case  712: ;goto step_next;
        // -- mwrite
        case  713: goto step_next;
        case  714: _wait();_mwrite(--cpu->sp,cpu->pch);;goto step_next;
        case  715: goto step_next;
        // -- mwrite
        case  716: goto step_next;
        case  717: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz;goto step_next;
        case  718: goto step_next;
        // -- overlapped
        case  719: goto fetch_next;
        
        //  DD: dd prefix (M:1 T:4)
        // -- overlapped
        case  720: ;_fetch_dd();goto step_next;
        
        //  DE: sbc n (M:2 T:7)
        // -- mread
        case  721: goto step_next;
        case  722: _wait();_mread(cpu->pc++);goto step_next;
        case  723: cpu->dlatch=_gd();;goto step_next;
        // -- overlapped
        case  724: _z80_sbc8(cpu,cpu->dlatch);goto fetch_next;
        
        //  DF: rst 18h (M:4 T:11)
        // -- generic
        case  725: ;goto step_next;
        // -- mwrite
        case  726: goto step_next;
        case  727: _wait();_mwrite(--cpu->sp,cpu->pch);;goto step_next;
        case  728: goto step_next;
        // -- mwrite
        case  729: goto step_next;
        case  730: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x18;cpu->pc=cpu->wz;goto step_next;
        case  731: goto step_next;
        // -- overlapped
        case  732: goto fetch_next;
        
        //  E0: ret po (M:4 T:11)
        // -- generic
        case  733: if(!_cc_po){_z80_skip(cpu,6);};goto step_next;
        // -- mread
        case  734: goto step_next;
        case  735: _wait();_mread(cpu->sp++);goto step_next;
        case  736: cpu->wzl=_gd();;goto step_next;
        // -- mread
        case  737: goto step_next;
        case  738: _wait();_mread(cpu->sp++);goto step_next;
        case  739: cpu->wzh=_gd();cpu->pc=cpu->wz;goto step_next;
        // -- overlapped
        case  740: goto fetch_next;
        
        //  E1: pop hl (M:3 T:10)
        // -- mread
        case  741: goto step_next;
        case  742: _wait();_mread(cpu->sp++);goto step_next;
        case  743: cpu->hlx[cpu->hlx_idx].l=_gd();;goto step_next;
        // -- mread
        case  744: goto step_next;
        case  745: _wait();_mread(cpu->sp++);goto step_next;
        case  746: cpu->hlx[cpu->hlx_idx].h=_gd();;goto step_next;
        // -- overlapped
        case  747: goto fetch_next;
        
        //  E2: jp po,nn (M:3 T:10)
        // -- mread
        case  748: goto step_next;
        case  749: _wait();_mread(cpu->pc++);goto step_next;
        case  750: cpu->wzl=_gd();;goto step_next;
        // -- mread
        case  751: goto step_next;
        case  752: _wait();_mread(cpu->pc++);goto step_next;
        case  753: cpu->wzh=_gd();if(_cc_po){cpu->pc=cpu->wz;};goto step_next;
        // -- overlapped
        case  754: goto fetch_next;
        
        //  E3: ex (sp),hl (M:5 T:19)
        // -- mread
        case  755: goto step_next;
        case  756: _wait();_mread(cpu->sp);goto step_next;
        case  757: cpu->wzl=_gd();;goto step_next;
        // -- mread
        case  758: goto step_next;
        case  759: _wait();_mread(cpu->sp+1);goto step_next;
        case  760: cpu->wzh=_gd();;goto step_next;
        case  761: goto step_next;
        // -- mwrite
        case  762: goto step_next;
        case  763: _wait();_mwrite(cpu->sp+1,cpu->hlx[cpu->hlx_idx].h);;goto step_next;
        case  764: goto step_next;
        // -- mwrite
        case  765: goto step_next;
        case  766: _wait();_mwrite(cpu->sp,cpu->hlx[cpu->hlx_idx].l);cpu->hlx[cpu->hlx_idx].hl=cpu->wz;goto step_next;
        case  767: goto step_next;
        case  768: goto step_next;
        case  769: goto step_next;
        // -- overlapped
        case  770: goto fetch_next;
        
        //  E4: call po,nn (M:6 T:17)
        // -- mread
        case  771: goto step_next;
        case  772: _wait();_mread(cpu->pc++);goto step_next;
        case  773: cpu->wzl=_gd();;goto step_next;
        // -- mread
        case  774: goto step_next;
        case  775: _wait();_mread(cpu->pc++);goto step_next;
        case  776: cpu->wzh=_gd();if (!_cc_po){_z80_skip(cpu,7);};goto step_next;
        // -- generic
        case  777: ;goto step_next;
        // -- mwrite
        case  778: goto step_next;
        case  779: _wait();_mwrite(--cpu->sp,cpu->pch);;goto step_next;
        case  780: goto step_next;
        // -- mwrite
        case  781: goto step_next;
        case  782: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz;goto step_next;
        case  783: goto step_next;
        // -- overlapped
        case  784: goto fetch_next;
        
        //  E5: push hl (M:4 T:11)
        // -- generic
        case  785: ;goto step_next;
        // -- mwrite
        case  786: goto step_next;
        case  787: _wait();_mwrite(--cpu->sp,cpu->hlx[cpu->hlx_idx].h);;goto step_next;
        case  788: goto step_next;
        // -- mwrite
        case  789: goto step_next;
        case  790: _wait();_mwrite(--cpu->sp,cpu->hlx[cpu->hlx_idx].l);;goto step_next;
        case  791: goto step_next;
        // -- overlapped
        case  792: goto fetch_next;
        
        //  E6: and n (M:2 T:7)
        // -- mread
        case  793: goto step_next;
        case  794: _wait();_mread(cpu->pc++);goto step_next;
        case  795: cpu->dlatch=_gd();;goto step_next;
        // -- overlapped
        case  796: _z80_and8(cpu,cpu->dlatch);goto fetch_next;
        
        //  E7: rst 20h (M:4 T:11)
        // -- generic
        case  797: ;goto step_next;
        // -- mwrite
        case  798: goto step_next;
        case  799: _wait();_mwrite(--cpu->sp,cpu->pch);;goto step_next;
        case  800: goto step_next;
        // -- mwrite
        case  801: goto step_next;
        case  802: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x20;cpu->pc=cpu->wz;goto step_next;
        case  803: goto step_next;
        // -- overlapped
        case  804: goto fetch_next;
        
        //  E8: ret pe (M:4 T:11)
        // -- generic
        case  805: if(!_cc_pe){_z80_skip(cpu,6);};goto step_next;
        // -- mread
        case  806: goto step_next;
        case  807: _wait();_mread(cpu->sp++);goto step_next;
        case  808: cpu->wzl=_gd();;goto step_next;
        // -- mread
        case  809: goto step_next;
        case  810: _wait();_mread(cpu->sp++);goto step_next;
        case  811: cpu->wzh=_gd();cpu->pc=cpu->wz;goto step_next;
        // -- overlapped
        case  812: goto fetch_next;
        
        //  E9: jp hl (M:1 T:4)
        // -- overlapped
        case  813: cpu->pc=cpu->hlx[cpu->hlx_idx].hl;goto fetch_next;
        
        //  EA: jp pe,nn (M:3 T:10)
        // -- mread
        case  814: goto step_next;
        case  815: _wait();_mread(cpu->pc++);goto step_next;
        case  816: cpu->wzl=_gd();;goto step_next;
        // -- mread
        case  817: goto step_next;
        case  818: _wait();_mread(cpu->pc++);goto step_next;
        case  819: cpu->wzh=_gd();if(_cc_pe){cpu->pc=cpu->wz;};goto step_next;
        // -- overlapped
        case  820: goto fetch_next;
        
        //  EB: ex de,hl (M:1 T:4)
        // -- overlapped
        case  821: _z80_ex_de_hl(cpu);goto fetch_next;
        
        //  EC: call pe,nn (M:6 T:17)
        // -- mread
        case  822: goto step_next;
        case  823: _wait();_mread(cpu->pc++);goto step_next;
        case  824: cpu->wzl=_gd();;goto step_next;
        // -- mread
        case  825: goto step_next;
        case  826: _wait();_mread(cpu->pc++);goto step_next;
        case  827: cpu->wzh=_gd();if (!_cc_pe){_z80_skip(cpu,7);};goto step_next;
        // -- generic
        case  828: ;goto step_next;
        // -- mwrite
        case  829: goto step_next;
        case  830: _wait();_mwrite(--cpu->sp,cpu->pch);;goto step_next;
        case  831: goto step_next;
        // -- mwrite
        case  832: goto step_next;
        case  833: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz;goto step_next;
        case  834: goto step_next;
        // -- overlapped
        case  835: goto fetch_next;
        
        //  ED: ed prefix (M:1 T:4)
        // -- overlapped
        case  836: ;_fetch_ed();goto step_next;
        
        //  EE: xor n (M:2 T:7)
        // -- mread
        case  837: goto step_next;
        case  838: _wait();_mread(cpu->pc++);goto step_next;
        case  839: cpu->dlatch=_gd();;goto step_next;
        // -- overlapped
        case  840: _z80_xor8(cpu,cpu->dlatch);goto fetch_next;
        
        //  EF: rst 28h (M:4 T:11)
        // -- generic
        case  841: ;goto step_next;
        // -- mwrite
        case  842: goto step_next;
        case  843: _wait();_mwrite(--cpu->sp,cpu->pch);;goto step_next;
        case  844: goto step_next;
        // -- mwrite
        case  845: goto step_next;
        case  846: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x28;cpu->pc=cpu->wz;goto step_next;
        case  847: goto step_next;
        // -- overlapped
        case  848: goto fetch_next;
        
        //  F0: ret p (M:4 T:11)
        // -- generic
        case  849: if(!_cc_p){_z80_skip(cpu,6);};goto step_next;
        // -- mread
        case  850: goto step_next;
        case  851: _wait();_mread(cpu->sp++);goto step_next;
        case  852: cpu->wzl=_gd();;goto step_next;
        // -- mread
        case  853: goto step_next;
        case  854: _wait();_mread(cpu->sp++);goto step_next;
        case  855: cpu->wzh=_gd();cpu->pc=cpu->wz;goto step_next;
        // -- overlapped
        case  856: goto fetch_next;
        
        //  F1: pop af (M:3 T:10)
        // -- mread
        case  857: goto step_next;
        case  858: _wait();_mread(cpu->sp++);goto step_next;
        case  859: cpu->f=_gd();;goto step_next;
        // -- mread
        case  860: goto step_next;
        case  861: _wait();_mread(cpu->sp++);goto step_next;
        case  862: cpu->a=_gd();;goto step_next;
        // -- overlapped
        case  863: goto fetch_next;
        
        //  F2: jp p,nn (M:3 T:10)
        // -- mread
        case  864: goto step_next;
        case  865: _wait();_mread(cpu->pc++);goto step_next;
        case  866: cpu->wzl=_gd();;goto step_next;
        // -- mread
        case  867: goto step_next;
        case  868: _wait();_mread(cpu->pc++);goto step_next;
        case  869: cpu->wzh=_gd();if(_cc_p){cpu->pc=cpu->wz;};goto step_next;
        // -- overlapped
        case  870: goto fetch_next;
        
        //  F3: di (M:1 T:4)
        // -- overlapped
        case  871: cpu->iff1=cpu->iff2=false;;goto fetch_next;
        
        //  F4: call p,nn (M:6 T:17)
        // -- mread
        case  872: goto step_next;
        case  873: _wait();_mread(cpu->pc++);goto step_next;
        case  874: cpu->wzl=_gd();;goto step_next;
        // -- mread
        case  875: goto step_next;
        case  876: _wait();_mread(cpu->pc++);goto step_next;
        case  877: cpu->wzh=_gd();if (!_cc_p){_z80_skip(cpu,7);};goto step_next;
        // -- generic
        case  878: ;goto step_next;
        // -- mwrite
        case  879: goto step_next;
        case  880: _wait();_mwrite(--cpu->sp,cpu->pch);;goto step_next;
        case  881: goto step_next;
        // -- mwrite
        case  882: goto step_next;
        case  883: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz;goto step_next;
        case  884: goto step_next;
        // -- overlapped
        case  885: goto fetch_next;
        
        //  F5: push af (M:4 T:11)
        // -- generic
        case  886: ;goto step_next;
        // -- mwrite
        case  887: goto step_next;
        case  888: _wait();_mwrite(--cpu->sp,cpu->a);;goto step_next;
        case  889: goto step_next;
        // -- mwrite
        case  890: goto step_next;
        case  891: _wait();_mwrite(--cpu->sp,cpu->f);;goto step_next;
        case  892: goto step_next;
        // -- overlapped
        case  893: goto fetch_next;
        
        //  F6: or n (M:2 T:7)
        // -- mread
        case  894: goto step_next;
        case  895: _wait();_mread(cpu->pc++);goto step_next;
        case  896: cpu->dlatch=_gd();;goto step_next;
        // -- overlapped
        case  897: _z80_or8(cpu,cpu->dlatch);goto fetch_next;
        
        //  F7: rst 30h (M:4 T:11)
        // -- generic
        case  898: ;goto step_next;
        // -- mwrite
        case  899: goto step_next;
        case  900: _wait();_mwrite(--cpu->sp,cpu->pch);;goto step_next;
        case  901: goto step_next;
        // -- mwrite
        case  902: goto step_next;
        case  903: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x30;cpu->pc=cpu->wz;goto step_next;
        case  904: goto step_next;
        // -- overlapped
        case  905: goto fetch_next;
        
        //  F8: ret m (M:4 T:11)
        // -- generic
        case  906: if(!_cc_m){_z80_skip(cpu,6);};goto step_next;
        // -- mread
        case  907: goto step_next;
        case  908: _wait();_mread(cpu->sp++);goto step_next;
        case  909: cpu->wzl=_gd();;goto step_next;
        // -- mread
        case  910: goto step_next;
        case  911: _wait();_mread(cpu->sp++);goto step_next;
        case  912: cpu->wzh=_gd();cpu->pc=cpu->wz;goto step_next;
        // -- overlapped
        case  913: goto fetch_next;
        
        //  F9: ld sp,hl (M:2 T:6)
        // -- generic
        case  914: cpu->sp=cpu->hlx[cpu->hlx_idx].hl;goto step_next;
        case  915: goto step_next;
        // -- overlapped
        case  916: goto fetch_next;
        
        //  FA: jp m,nn (M:3 T:10)
        // -- mread
        case  917: goto step_next;
        case  918: _wait();_mread(cpu->pc++);goto step_next;
        case  919: cpu->wzl=_gd();;goto step_next;
        // -- mread
        case  920: goto step_next;
        case  921: _wait();_mread(cpu->pc++);goto step_next;
        case  922: cpu->wzh=_gd();if(_cc_m){cpu->pc=cpu->wz;};goto step_next;
        // -- overlapped
        case  923: goto fetch_next;
        
        //  FB: ei (M:1 T:4)
        // -- overlapped
        case  924: cpu->iff1=cpu->iff2=false;;pins=_z80_fetch(cpu,pins);cpu->iff1=cpu->iff2=true;goto step_next;
        
        //  FC: call m,nn (M:6 T:17)
        // -- mread
        case  925: goto step_next;
        case  926: _wait();_mread(cpu->pc++);goto step_next;
        case  927: cpu->wzl=_gd();;goto step_next;
        // -- mread
        case  928: goto step_next;
        case  929: _wait();_mread(cpu->pc++);goto step_next;
        case  930: cpu->wzh=_gd();if (!_cc_m){_z80_skip(cpu,7);};goto step_next;
        // -- generic
        case  931: ;goto step_next;
        // -- mwrite
        case  932: goto step_next;
        case  933: _wait();_mwrite(--cpu->sp,cpu->pch);;goto step_next;
        case  934: goto step_next;
        // -- mwrite
        case  935: goto step_next;
        case  936: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz;goto step_next;
        case  937: goto step_next;
        // -- overlapped
        case  938: goto fetch_next;
        
        //  FD: fd prefix (M:1 T:4)
        // -- overlapped
        case  939: ;_fetch_fd();goto step_next;
        
        //  FE: cp n (M:2 T:7)
        // -- mread
        case  940: goto step_next;
        case  941: _wait();_mread(cpu->pc++);goto step_next;
        case  942: cpu->dlatch=_gd();;goto step_next;
        // -- overlapped
        case  943: _z80_cp8(cpu,cpu->dlatch);goto fetch_next;
        
        //  FF: rst 38h (M:4 T:11)
        // -- generic
        case  944: ;goto step_next;
        // -- mwrite
        case  945: goto step_next;
        case  946: _wait();_mwrite(--cpu->sp,cpu->pch);;goto step_next;
        case  947: goto step_next;
        // -- mwrite
        case  948: goto step_next;
        case  949: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x38;cpu->pc=cpu->wz;goto step_next;
        case  950: goto step_next;
        // -- overlapped
        case  951: goto fetch_next;
        
        // ED 00: ed nop (M:1 T:4)
        // -- overlapped
        case  952: goto fetch_next;
        
        // ED 40: in b,(c) (M:2 T:8)
        // -- ioread
        case  953: goto step_next;
        case  954: goto step_next;
        case  955: _wait();_ioread(cpu->bc);goto step_next;
        case  956: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;goto step_next;
        // -- overlapped
        case  957: cpu->b=_z80_in(cpu,cpu->dlatch);goto fetch_next;
        
        // ED 41: out (c),b (M:2 T:8)
        // -- iowrite
        case  958: goto step_next;
        case  959: _iowrite(cpu->bc,cpu->b);goto step_next;
        case  960: _wait();cpu->wz=cpu->bc+1;goto step_next;
        case  961: goto step_next;
        // -- overlapped
        case  962: goto fetch_next;
        
        // ED 42: sbc hl,bc (M:2 T:11)
        // -- generic
        case  963: _z80_sbc16(cpu,cpu->bc);goto step_next;
        case  964: goto step_next;
        case  965: goto step_next;
        case  966: goto step_next;
        case  967: goto step_next;
        case  968: goto step_next;
        case  969: goto step_next;
        // -- overlapped
        case  970: goto fetch_next;
        
        // ED 43: ld (nn),bc (M:5 T:16)
        // -- mread
        case  971: goto step_next;
        case  972: _wait();_mread(cpu->pc++);goto step_next;
        case  973: cpu->wzl=_gd();;goto step_next;
        // -- mread
        case  974: goto step_next;
        case  975: _wait();_mread(cpu->pc++);goto step_next;
        case  976: cpu->wzh=_gd();;goto step_next;
        // -- mwrite
        case  977: goto step_next;
        case  978: _wait();_mwrite(cpu->wz++,cpu->c);;goto step_next;
        case  979: goto step_next;
        // -- mwrite
        case  980: goto step_next;
        case  981: _wait();_mwrite(cpu->wz,cpu->b);;goto step_next;
        case  982: goto step_next;
        // -- overlapped
        case  983: goto fetch_next;
        
        // ED 44: neg (M:1 T:4)
        // -- overlapped
        case  984: _z80_neg8(cpu);goto fetch_next;
        
        // ED 45: reti/retn (M:3 T:10)
        // -- mread
        case  985: goto step_next;
        case  986: _wait();_mread(cpu->sp++);goto step_next;
        case  987: cpu->wzl=_gd();pins|=Z80_RETI;goto step_next;
        // -- mread
        case  988: goto step_next;
        case  989: _wait();_mread(cpu->sp++);goto step_next;
        case  990: cpu->wzh=_gd();cpu->pc=cpu->wz;goto step_next;
        // -- overlapped
        case  991: ;pins=_z80_fetch(cpu,pins);cpu->iff1=cpu->iff2;goto step_next;
        
        // ED 46: im 0 (M:1 T:4)
        // -- overlapped
        case  992: cpu->im=0;goto fetch_next;
        
        // ED 47: ld i,a (M:2 T:5)
        // -- generic
        case  993: ;goto step_next;
        // -- overlapped
        case  994: cpu->i=cpu->a;goto fetch_next;
        
        // ED 48: in c,(c) (M:2 T:8)
        // -- ioread
        case  995: goto step_next;
        case  996: goto step_next;
        case  997: _wait();_ioread(cpu->bc);goto step_next;
        case  998: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;goto step_next;
        // -- overlapped
        case  999: cpu->c=_z80_in(cpu,cpu->dlatch);goto fetch_next;
        
        // ED 49: out (c),c (M:2 T:8)
        // -- iowrite
        case 1000: goto step_next;
        case 1001: _iowrite(cpu->bc,cpu->c);goto step_next;
        case 1002: _wait();cpu->wz=cpu->bc+1;goto step_next;
        case 1003: goto step_next;
        // -- overlapped
        case 1004: goto fetch_next;
        
        // ED 4A: adc hl,bc (M:2 T:11)
        // -- generic
        case 1005: _z80_adc16(cpu,cpu->bc);goto step_next;
        case 1006: goto step_next;
        case 1007: goto step_next;
        case 1008: goto step_next;
        case 1009: goto step_next;
        case 1010: goto step_next;
        case 1011: goto step_next;
        // -- overlapped
        case 1012: goto fetch_next;
        
        // ED 4B: ld bc,(nn) (M:5 T:16)
        // -- mread
        case 1013: goto step_next;
        case 1014: _wait();_mread(cpu->pc++);goto step_next;
        case 1015: cpu->wzl=_gd();;goto step_next;
        // -- mread
        case 1016: goto step_next;
        case 1017: _wait();_mread(cpu->pc++);goto step_next;
        case 1018: cpu->wzh=_gd();;goto step_next;
        // -- mread
        case 1019: goto step_next;
        case 1020: _wait();_mread(cpu->wz++);goto step_next;
        case 1021: cpu->c=_gd();;goto step_next;
        // -- mread
        case 1022: goto step_next;
        case 1023: _wait();_mread(cpu->wz);goto step_next;
        case 1024: cpu->b=_gd();;goto step_next;
        // -- overlapped
        case 1025: goto fetch_next;
        
        // ED 4E: im 0 (M:1 T:4)
        // -- overlapped
        case 1026: cpu->im=0;goto fetch_next;
        
        // ED 4F: ld r,a (M:2 T:5)
        // -- generic
        case 1027: ;goto step_next;
        // -- overlapped
        case 1028: cpu->r=cpu->a;goto fetch_next;
        
        // ED 50: in d,(c) (M:2 T:8)
        // -- ioread
        case 1029: goto step_next;
        case 1030: goto step_next;
        case 1031: _wait();_ioread(cpu->bc);goto step_next;
        case 1032: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;goto step_next;
        // -- overlapped
        case 1033: cpu->d=_z80_in(cpu,cpu->dlatch);goto fetch_next;
        
        // ED 51: out (c),d (M:2 T:8)
        // -- iowrite
        case 1034: goto step_next;
        case 1035: _iowrite(cpu->bc,cpu->d);goto step_next;
        case 1036: _wait();cpu->wz=cpu->bc+1;goto step_next;
        case 1037: goto step_next;
        // -- overlapped
        case 1038: goto fetch_next;
        
        // ED 52: sbc hl,de (M:2 T:11)
        // -- generic
        case 1039: _z80_sbc16(cpu,cpu->de);goto step_next;
        case 1040: goto step_next;
        case 1041: goto step_next;
        case 1042: goto step_next;
        case 1043: goto step_next;
        case 1044: goto step_next;
        case 1045: goto step_next;
        // -- overlapped
        case 1046: goto fetch_next;
        
        // ED 53: ld (nn),de (M:5 T:16)
        // -- mread
        case 1047: goto step_next;
        case 1048: _wait();_mread(cpu->pc++);goto step_next;
        case 1049: cpu->wzl=_gd();;goto step_next;
        // -- mread
        case 1050: goto step_next;
        case 1051: _wait();_mread(cpu->pc++);goto step_next;
        case 1052: cpu->wzh=_gd();;goto step_next;
        // -- mwrite
        case 1053: goto step_next;
        case 1054: _wait();_mwrite(cpu->wz++,cpu->e);;goto step_next;
        case 1055: goto step_next;
        // -- mwrite
        case 1056: goto step_next;
        case 1057: _wait();_mwrite(cpu->wz,cpu->d);;goto step_next;
        case 1058: goto step_next;
        // -- overlapped
        case 1059: goto fetch_next;
        
        // ED 56: im 1 (M:1 T:4)
        // -- overlapped
        case 1060: cpu->im=1;goto fetch_next;
        
        // ED 57: ld a,i (M:2 T:5)
        // -- generic
        case 1061: ;goto step_next;
        // -- overlapped
        case 1062: cpu->a=cpu->i;cpu->f=_z80_sziff2_flags(cpu, cpu->i);goto fetch_next;
        
        // ED 58: in e,(c) (M:2 T:8)
        // -- ioread
        case 1063: goto step_next;
        case 1064: goto step_next;
        case 1065: _wait();_ioread(cpu->bc);goto step_next;
        case 1066: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;goto step_next;
        // -- overlapped
        case 1067: cpu->e=_z80_in(cpu,cpu->dlatch);goto fetch_next;
        
        // ED 59: out (c),e (M:2 T:8)
        // -- iowrite
        case 1068: goto step_next;
        case 1069: _iowrite(cpu->bc,cpu->e);goto step_next;
        case 1070: _wait();cpu->wz=cpu->bc+1;goto step_next;
        case 1071: goto step_next;
        // -- overlapped
        case 1072: goto fetch_next;
        
        // ED 5A: adc hl,de (M:2 T:11)
        // -- generic
        case 1073: _z80_adc16(cpu,cpu->de);goto step_next;
        case 1074: goto step_next;
        case 1075: goto step_next;
        case 1076: goto step_next;
        case 1077: goto step_next;
        case 1078: goto step_next;
        case 1079: goto step_next;
        // -- overlapped
        case 1080: goto fetch_next;
        
        // ED 5B: ld de,(nn) (M:5 T:16)
        // -- mread
        case 1081: goto step_next;
        case 1082: _wait();_mread(cpu->pc++);goto step_next;
        case 1083: cpu->wzl=_gd();;goto step_next;
        // -- mread
        case 1084: goto step_next;
        case 1085: _wait();_mread(cpu->pc++);goto step_next;
        case 1086: cpu->wzh=_gd();;goto step_next;
        // -- mread
        case 1087: goto step_next;
        case 1088: _wait();_mread(cpu->wz++);goto step_next;
        case 1089: cpu->e=_gd();;goto step_next;
        // -- mread
        case 1090: goto step_next;
        case 1091: _wait();_mread(cpu->wz);goto step_next;
        case 1092: cpu->d=_gd();;goto step_next;
        // -- overlapped
        case 1093: goto fetch_next;
        
        // ED 5E: im 2 (M:1 T:4)
        // -- overlapped
        case 1094: cpu->im=2;goto fetch_next;
        
        // ED 5F: ld a,r (M:2 T:5)
        // -- generic
        case 1095: ;goto step_next;
        // -- overlapped
        case 1096: cpu->a=cpu->r;cpu->f=_z80_sziff2_flags(cpu, cpu->r);goto fetch_next;
        
        // ED 60: in h,(c) (M:2 T:8)
        // -- ioread
        case 1097: goto step_next;
        case 1098: goto step_next;
        case 1099: _wait();_ioread(cpu->bc);goto step_next;
        case 1100: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;goto step_next;
        // -- overlapped
        case 1101: cpu->hlx[cpu->hlx_idx].h=_z80_in(cpu,cpu->dlatch);goto fetch_next;
        
        // ED 61: out (c),h (M:2 T:8)
        // -- iowrite
        case 1102: goto step_next;
        case 1103: _iowrite(cpu->bc,cpu->hlx[cpu->hlx_idx].h);goto step_next;
        case 1104: _wait();cpu->wz=cpu->bc+1;goto step_next;
        case 1105: goto step_next;
        // -- overlapped
        case 1106: goto fetch_next;
        
        // ED 62: sbc hl,hl (M:2 T:11)
        // -- generic
        case 1107: _z80_sbc16(cpu,cpu->hl);goto step_next;
        case 1108: goto step_next;
        case 1109: goto step_next;
        case 1110: goto step_next;
        case 1111: goto step_next;
        case 1112: goto step_next;
        case 1113: goto step_next;
        // -- overlapped
        case 1114: goto fetch_next;
        
        // ED 63: ld (nn),hl (M:5 T:16)
        // -- mread
        case 1115: goto step_next;
        case 1116: _wait();_mread(cpu->pc++);goto step_next;
        case 1117: cpu->wzl=_gd();;goto step_next;
        // -- mread
        case 1118: goto step_next;
        case 1119: _wait();_mread(cpu->pc++);goto step_next;
        case 1120: cpu->wzh=_gd();;goto step_next;
        // -- mwrite
        case 1121: goto step_next;
        case 1122: _wait();_mwrite(cpu->wz++,cpu->l);;goto step_next;
        case 1123: goto step_next;
        // -- mwrite
        case 1124: goto step_next;
        case 1125: _wait();_mwrite(cpu->wz,cpu->h);;goto step_next;
        case 1126: goto step_next;
        // -- overlapped
        case 1127: goto fetch_next;
        
        // ED 66: im 0 (M:1 T:4)
        // -- overlapped
        case 1128: cpu->im=0;goto fetch_next;
        
        // ED 67: rrd (M:4 T:14)
        // -- mread
        case 1129: goto step_next;
        case 1130: _wait();_mread(cpu->hl);goto step_next;
        case 1131: cpu->dlatch=_gd();;goto step_next;
        // -- generic
        case 1132: cpu->dlatch=_z80_rrd(cpu,cpu->dlatch);goto step_next;
        case 1133: goto step_next;
        case 1134: goto step_next;
        case 1135: goto step_next;
        // -- mwrite
        case 1136: goto step_next;
        case 1137: _wait();_mwrite(cpu->hl,cpu->dlatch);cpu->wz=cpu->hl+1;goto step_next;
        case 1138: goto step_next;
        // -- overlapped
        case 1139: goto fetch_next;
        
        // ED 68: in l,(c) (M:2 T:8)
        // -- ioread
        case 1140: goto step_next;
        case 1141: goto step_next;
        case 1142: _wait();_ioread(cpu->bc);goto step_next;
        case 1143: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;goto step_next;
        // -- overlapped
        case 1144: cpu->hlx[cpu->hlx_idx].l=_z80_in(cpu,cpu->dlatch);goto fetch_next;
        
        // ED 69: out (c),l (M:2 T:8)
        // -- iowrite
        case 1145: goto step_next;
        case 1146: _iowrite(cpu->bc,cpu->hlx[cpu->hlx_idx].l);goto step_next;
        case 1147: _wait();cpu->wz=cpu->bc+1;goto step_next;
        case 1148: goto step_next;
        // -- overlapped
        case 1149: goto fetch_next;
        
        // ED 6A: adc hl,hl (M:2 T:11)
        // -- generic
        case 1150: _z80_adc16(cpu,cpu->hl);goto step_next;
        case 1151: goto step_next;
        case 1152: goto step_next;
        case 1153: goto step_next;
        case 1154: goto step_next;
        case 1155: goto step_next;
        case 1156: goto step_next;
        // -- overlapped
        case 1157: goto fetch_next;
        
        // ED 6B: ld hl,(nn) (M:5 T:16)
        // -- mread
        case 1158: goto step_next;
        case 1159: _wait();_mread(cpu->pc++);goto step_next;
        case 1160: cpu->wzl=_gd();;goto step_next;
        // -- mread
        case 1161: goto step_next;
        case 1162: _wait();_mread(cpu->pc++);goto step_next;
        case 1163: cpu->wzh=_gd();;goto step_next;
        // -- mread
        case 1164: goto step_next;
        case 1165: _wait();_mread(cpu->wz++);goto step_next;
        case 1166: cpu->l=_gd();;goto step_next;
        // -- mread
        case 1167: goto step_next;
        case 1168: _wait();_mread(cpu->wz);goto step_next;
        case 1169: cpu->h=_gd();;goto step_next;
        // -- overlapped
        case 1170: goto fetch_next;
        
        // ED 6E: im 0 (M:1 T:4)
        // -- overlapped
        case 1171: cpu->im=0;goto fetch_next;
        
        // ED 6F: rld (M:4 T:14)
        // -- mread
        case 1172: goto step_next;
        case 1173: _wait();_mread(cpu->hl);goto step_next;
        case 1174: cpu->dlatch=_gd();;goto step_next;
        // -- generic
        case 1175: cpu->dlatch=_z80_rld(cpu,cpu->dlatch);goto step_next;
        case 1176: goto step_next;
        case 1177: goto step_next;
        case 1178: goto step_next;
        // -- mwrite
        case 1179: goto step_next;
        case 1180: _wait();_mwrite(cpu->hl,cpu->dlatch);cpu->wz=cpu->hl+1;goto step_next;
        case 1181: goto step_next;
        // -- overlapped
        case 1182: goto fetch_next;
        
        // ED 70: in (c) (M:2 T:8)
        // -- ioread
        case 1183: goto step_next;
        case 1184: goto step_next;
        case 1185: _wait();_ioread(cpu->bc);goto step_next;
        case 1186: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;goto step_next;
        // -- overlapped
        case 1187: _z80_in(cpu,cpu->dlatch);goto fetch_next;
        
        // ED 71: out (c),0 (M:2 T:8)
        // -- iowrite
        case 1188: goto step_next;
        case 1189: _iowrite(cpu->bc,0);goto step_next;
        case 1190: _wait();cpu->wz=cpu->bc+1;goto step_next;
        case 1191: goto step_next;
        // -- overlapped
        case 1192: goto fetch_next;
        
        // ED 72: sbc hl,sp (M:2 T:11)
        // -- generic
        case 1193: _z80_sbc16(cpu,cpu->sp);goto step_next;
        case 1194: goto step_next;
        case 1195: goto step_next;
        case 1196: goto step_next;
        case 1197: goto step_next;
        case 1198: goto step_next;
        case 1199: goto step_next;
        // -- overlapped
        case 1200: goto fetch_next;
        
        // ED 73: ld (nn),sp (M:5 T:16)
        // -- mread
        case 1201: goto step_next;
        case 1202: _wait();_mread(cpu->pc++);goto step_next;
        case 1203: cpu->wzl=_gd();;goto step_next;
        // -- mread
        case 1204: goto step_next;
        case 1205: _wait();_mread(cpu->pc++);goto step_next;
        case 1206: cpu->wzh=_gd();;goto step_next;
        // -- mwrite
        case 1207: goto step_next;
        case 1208: _wait();_mwrite(cpu->wz++,cpu->spl);;goto step_next;
        case 1209: goto step_next;
        // -- mwrite
        case 1210: goto step_next;
        case 1211: _wait();_mwrite(cpu->wz,cpu->sph);;goto step_next;
        case 1212: goto step_next;
        // -- overlapped
        case 1213: goto fetch_next;
        
        // ED 76: im 1 (M:1 T:4)
        // -- overlapped
        case 1214: cpu->im=1;goto fetch_next;
        
        // ED 78: in a,(c) (M:2 T:8)
        // -- ioread
        case 1215: goto step_next;
        case 1216: goto step_next;
        case 1217: _wait();_ioread(cpu->bc);goto step_next;
        case 1218: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;goto step_next;
        // -- overlapped
        case 1219: cpu->a=_z80_in(cpu,cpu->dlatch);goto fetch_next;
        
        // ED 79: out (c),a (M:2 T:8)
        // -- iowrite
        case 1220: goto step_next;
        case 1221: _iowrite(cpu->bc,cpu->a);goto step_next;
        case 1222: _wait();cpu->wz=cpu->bc+1;goto step_next;
        case 1223: goto step_next;
        // -- overlapped
        case 1224: goto fetch_next;
        
        // ED 7A: adc hl,sp (M:2 T:11)
        // -- generic
        case 1225: _z80_adc16(cpu,cpu->sp);goto step_next;
        case 1226: goto step_next;
        case 1227: goto step_next;
        case 1228: goto step_next;
        case 1229: goto step_next;
        case 1230: goto step_next;
        case 1231: goto step_next;
        // -- overlapped
        case 1232: goto fetch_next;
        
        // ED 7B: ld sp,(nn) (M:5 T:16)
        // -- mread
        case 1233: goto step_next;
        case 1234: _wait();_mread(cpu->pc++);goto step_next;
        case 1235: cpu->wzl=_gd();;goto step_next;
        // -- mread
        case 1236: goto step_next;
        case 1237: _wait();_mread(cpu->pc++);goto step_next;
        case 1238: cpu->wzh=_gd();;goto step_next;
        // -- mread
        case 1239: goto step_next;
        case 1240: _wait();_mread(cpu->wz++);goto step_next;
        case 1241: cpu->spl=_gd();;goto step_next;
        // -- mread
        case 1242: goto step_next;
        case 1243: _wait();_mread(cpu->wz);goto step_next;
        case 1244: cpu->sph=_gd();;goto step_next;
        // -- overlapped
        case 1245: goto fetch_next;
        
        // ED 7E: im 2 (M:1 T:4)
        // -- overlapped
        case 1246: cpu->im=2;goto fetch_next;
        
        // ED A0: ldi (M:4 T:12)
        // -- mread
        case 1247: goto step_next;
        case 1248: _wait();_mread(cpu->hl++);goto step_next;
        case 1249: cpu->dlatch=_gd();;goto step_next;
        // -- mwrite
        case 1250: goto step_next;
        case 1251: _wait();_mwrite(cpu->de++,cpu->dlatch);;goto step_next;
        case 1252: goto step_next;
        // -- generic
        case 1253: _z80_ldi_ldd(cpu,cpu->dlatch);goto step_next;
        case 1254: goto step_next;
        // -- overlapped
        case 1255: goto fetch_next;
        
        // ED A1: cpi (M:3 T:12)
        // -- mread
        case 1256: goto step_next;
        case 1257: _wait();_mread(cpu->hl++);goto step_next;
        case 1258: cpu->dlatch=_gd();;goto step_next;
        // -- generic
        case 1259: cpu->wz++;_z80_cpi_cpd(cpu,cpu->dlatch);goto step_next;
        case 1260: goto step_next;
        case 1261: goto step_next;
        case 1262: goto step_next;
        case 1263: goto step_next;
        // -- overlapped
        case 1264: goto fetch_next;
        
        // ED A2: ini (M:4 T:12)
        // -- generic
        case 1265: ;goto step_next;
        // -- ioread
        case 1266: goto step_next;
        case 1267: goto step_next;
        case 1268: _wait();_ioread(cpu->bc);goto step_next;
        case 1269: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;cpu->b--;;goto step_next;
        // -- mwrite
        case 1270: goto step_next;
        case 1271: _wait();_mwrite(cpu->hl++,cpu->dlatch);_z80_ini_ind(cpu,cpu->dlatch,cpu->c+1);goto step_next;
        case 1272: goto step_next;
        // -- overlapped
        case 1273: goto fetch_next;
        
        // ED A3: outi (M:4 T:12)
        // -- generic
        case 1274: ;goto step_next;
        // -- mread
        case 1275: goto step_next;
        case 1276: _wait();_mread(cpu->hl++);goto step_next;
        case 1277: cpu->dlatch=_gd();cpu->b--;goto step_next;
        // -- iowrite
        case 1278: goto step_next;
        case 1279: _iowrite(cpu->bc,cpu->dlatch);goto step_next;
        case 1280: _wait();cpu->wz=cpu->bc+1;_z80_outi_outd(cpu,cpu->dlatch);goto step_next;
        case 1281: goto step_next;
        // -- overlapped
        case 1282: goto fetch_next;
        
        // ED A8: ldd (M:4 T:12)
        // -- mread
        case 1283: goto step_next;
        case 1284: _wait();_mread(cpu->hl--);goto step_next;
        case 1285: cpu->dlatch=_gd();;goto step_next;
        // -- mwrite
        case 1286: goto step_next;
        case 1287: _wait();_mwrite(cpu->de--,cpu->dlatch);;goto step_next;
        case 1288: goto step_next;
        // -- generic
        case 1289: _z80_ldi_ldd(cpu,cpu->dlatch);goto step_next;
        case 1290: goto step_next;
        // -- overlapped
        case 1291: goto fetch_next;
        
        // ED A9: cpd (M:3 T:12)
        // -- mread
        case 1292: goto step_next;
        case 1293: _wait();_mread(cpu->hl--);goto step_next;
        case 1294: cpu->dlatch=_gd();;goto step_next;
        // -- generic
        case 1295: cpu->wz--;_z80_cpi_cpd(cpu,cpu->dlatch);goto step_next;
        case 1296: goto step_next;
        case 1297: goto step_next;
        case 1298: goto step_next;
        case 1299: goto step_next;
        // -- overlapped
        case 1300: goto fetch_next;
        
        // ED AA: ind (M:4 T:12)
        // -- generic
        case 1301: ;goto step_next;
        // -- ioread
        case 1302: goto step_next;
        case 1303: goto step_next;
        case 1304: _wait();_ioread(cpu->bc);goto step_next;
        case 1305: cpu->dlatch=_gd();cpu->wz=cpu->bc-1;cpu->b--;;goto step_next;
        // -- mwrite
        case 1306: goto step_next;
        case 1307: _wait();_mwrite(cpu->hl--,cpu->dlatch);_z80_ini_ind(cpu,cpu->dlatch,cpu->c-1);goto step_next;
        case 1308: goto step_next;
        // -- overlapped
        case 1309: goto fetch_next;
        
        // ED AB: outd (M:4 T:12)
        // -- generic
        case 1310: ;goto step_next;
        // -- mread
        case 1311: goto step_next;
        case 1312: _wait();_mread(cpu->hl--);goto step_next;
        case 1313: cpu->dlatch=_gd();cpu->b--;goto step_next;
        // -- iowrite
        case 1314: goto step_next;
        case 1315: _iowrite(cpu->bc,cpu->dlatch);goto step_next;
        case 1316: _wait();cpu->wz=cpu->bc-1;_z80_outi_outd(cpu,cpu->dlatch);goto step_next;
        case 1317: goto step_next;
        // -- overlapped
        case 1318: goto fetch_next;
        
        // ED B0: ldir (M:5 T:17)
        // -- mread
        case 1319: goto step_next;
        case 1320: _wait();_mread(cpu->hl++);goto step_next;
        case 1321: cpu->dlatch=_gd();;goto step_next;
        // -- mwrite
        case 1322: goto step_next;
        case 1323: _wait();_mwrite(cpu->de++,cpu->dlatch);;goto step_next;
        case 1324: goto step_next;
        // -- generic
        case 1325: if(!_z80_ldi_ldd(cpu,cpu->dlatch)){_z80_skip(cpu,5);};goto step_next;
        case 1326: goto step_next;
        // -- generic
        case 1327: cpu->wz=--cpu->pc;--cpu->pc;;goto step_next;
        case 1328: goto step_next;
        case 1329: goto step_next;
        case 1330: goto step_next;
        case 1331: goto step_next;
        // -- overlapped
        case 1332: goto fetch_next;
        
        // ED B1: cpir (M:4 T:17)
        // -- mread
        case 1333: goto step_next;
        case 1334: _wait();_mread(cpu->hl++);goto step_next;
        case 1335: cpu->dlatch=_gd();;goto step_next;
        // -- generic
        case 1336: cpu->wz++;if(!_z80_cpi_cpd(cpu,cpu->dlatch)){_z80_skip(cpu,5);};goto step_next;
        case 1337: goto step_next;
        case 1338: goto step_next;
        case 1339: goto step_next;
        case 1340: goto step_next;
        // -- generic
        case 1341: cpu->wz=--cpu->pc;--cpu->pc;goto step_next;
        case 1342: goto step_next;
        case 1343: goto step_next;
        case 1344: goto step_next;
        case 1345: goto step_next;
        // -- overlapped
        case 1346: goto fetch_next;
        
        // ED B2: inir (M:5 T:17)
        // -- generic
        case 1347: ;goto step_next;
        // -- ioread
        case 1348: goto step_next;
        case 1349: goto step_next;
        case 1350: _wait();_ioread(cpu->bc);goto step_next;
        case 1351: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;cpu->b--;;goto step_next;
        // -- mwrite
        case 1352: goto step_next;
        case 1353: _wait();_mwrite(cpu->hl++,cpu->dlatch);if (!_z80_ini_ind(cpu,cpu->dlatch,cpu->c+1)){_z80_skip(cpu,5);};goto step_next;
        case 1354: goto step_next;
        // -- generic
        case 1355: cpu->wz=--cpu->pc;--cpu->pc;goto step_next;
        case 1356: goto step_next;
        case 1357: goto step_next;
        case 1358: goto step_next;
        case 1359: goto step_next;
        // -- overlapped
        case 1360: goto fetch_next;
        
        // ED B3: otir (M:5 T:17)
        // -- generic
        case 1361: ;goto step_next;
        // -- mread
        case 1362: goto step_next;
        case 1363: _wait();_mread(cpu->hl++);goto step_next;
        case 1364: cpu->dlatch=_gd();cpu->b--;goto step_next;
        // -- iowrite
        case 1365: goto step_next;
        case 1366: _iowrite(cpu->bc,cpu->dlatch);goto step_next;
        case 1367: _wait();cpu->wz=cpu->bc+1;if(!_z80_outi_outd(cpu,cpu->dlatch)){_z80_skip(cpu,5);};goto step_next;
        case 1368: goto step_next;
        // -- generic
        case 1369: cpu->wz=--cpu->pc;--cpu->pc;goto step_next;
        case 1370: goto step_next;
        case 1371: goto step_next;
        case 1372: goto step_next;
        case 1373: goto step_next;
        // -- overlapped
        case 1374: goto fetch_next;
        
        // ED B8: lddr (M:5 T:17)
        // -- mread
        case 1375: goto step_next;
        case 1376: _wait();_mread(cpu->hl--);goto step_next;
        case 1377: cpu->dlatch=_gd();;goto step_next;
        // -- mwrite
        case 1378: goto step_next;
        case 1379: _wait();_mwrite(cpu->de--,cpu->dlatch);;goto step_next;
        case 1380: goto step_next;
        // -- generic
        case 1381: if(!_z80_ldi_ldd(cpu,cpu->dlatch)){_z80_skip(cpu,5);};goto step_next;
        case 1382: goto step_next;
        // -- generic
        case 1383: cpu->wz=--cpu->pc;--cpu->pc;;goto step_next;
        case 1384: goto step_next;
        case 1385: goto step_next;
        case 1386: goto step_next;
        case 1387: goto step_next;
        // -- overlapped
        case 1388: goto fetch_next;
        
        // ED B9: cpdr (M:4 T:17)
        // -- mread
        case 1389: goto step_next;
        case 1390: _wait();_mread(cpu->hl--);goto step_next;
        case 1391: cpu->dlatch=_gd();;goto step_next;
        // -- generic
        case 1392: cpu->wz--;if(!_z80_cpi_cpd(cpu,cpu->dlatch)){_z80_skip(cpu,5);};goto step_next;
        case 1393: goto step_next;
        case 1394: goto step_next;
        case 1395: goto step_next;
        case 1396: goto step_next;
        // -- generic
        case 1397: cpu->wz=--cpu->pc;--cpu->pc;goto step_next;
        case 1398: goto step_next;
        case 1399: goto step_next;
        case 1400: goto step_next;
        case 1401: goto step_next;
        // -- overlapped
        case 1402: goto fetch_next;
        
        // ED BA: indr (M:5 T:17)
        // -- generic
        case 1403: ;goto step_next;
        // -- ioread
        case 1404: goto step_next;
        case 1405: goto step_next;
        case 1406: _wait();_ioread(cpu->bc);goto step_next;
        case 1407: cpu->dlatch=_gd();cpu->wz=cpu->bc-1;cpu->b--;;goto step_next;
        // -- mwrite
        case 1408: goto step_next;
        case 1409: _wait();_mwrite(cpu->hl--,cpu->dlatch);if (!_z80_ini_ind(cpu,cpu->dlatch,cpu->c-1)){_z80_skip(cpu,5);};goto step_next;
        case 1410: goto step_next;
        // -- generic
        case 1411: cpu->wz=--cpu->pc;--cpu->pc;goto step_next;
        case 1412: goto step_next;
        case 1413: goto step_next;
        case 1414: goto step_next;
        case 1415: goto step_next;
        // -- overlapped
        case 1416: goto fetch_next;
        
        // ED BB: otdr (M:5 T:17)
        // -- generic
        case 1417: ;goto step_next;
        // -- mread
        case 1418: goto step_next;
        case 1419: _wait();_mread(cpu->hl--);goto step_next;
        case 1420: cpu->dlatch=_gd();cpu->b--;goto step_next;
        // -- iowrite
        case 1421: goto step_next;
        case 1422: _iowrite(cpu->bc,cpu->dlatch);goto step_next;
        case 1423: _wait();cpu->wz=cpu->bc-1;if(!_z80_outi_outd(cpu,cpu->dlatch)){_z80_skip(cpu,5);};goto step_next;
        case 1424: goto step_next;
        // -- generic
        case 1425: cpu->wz=--cpu->pc;--cpu->pc;goto step_next;
        case 1426: goto step_next;
        case 1427: goto step_next;
        case 1428: goto step_next;
        case 1429: goto step_next;
        // -- overlapped
        case 1430: goto fetch_next;
        
        // CB 00: cb (M:1 T:4)
        // -- overlapped
        case 1431: {uint8_t z=cpu->opcode&7;_z80_cb_action(cpu,z,z);};goto fetch_next;
        
        // CB 00: cbhl (M:3 T:11)
        // -- mread
        case 1432: goto step_next;
        case 1433: _wait();_mread(cpu->hl);goto step_next;
        case 1434: cpu->dlatch=_gd();if(!_z80_cb_action(cpu,6,6)){_z80_skip(cpu,3);};goto step_next;
        case 1435: goto step_next;
        // -- mwrite
        case 1436: goto step_next;
        case 1437: _wait();_mwrite(cpu->hl,cpu->dlatch);;goto step_next;
        case 1438: goto step_next;
        // -- overlapped
        case 1439: goto fetch_next;
        
        // CB 00: ddfdcb (M:6 T:18)
        // -- generic
        case 1440: _wait();_mread(cpu->pc++);goto step_next;
        // -- generic
        case 1441: _z80_ddfdcb_addr(cpu,pins);goto step_next;
        // -- mread
        case 1442: goto step_next;
        case 1443: _wait();_mread(cpu->pc++);goto step_next;
        case 1444: cpu->dlatch=_gd();_z80_ddfdcb_opcode(cpu,cpu->dlatch);goto step_next;
        case 1445: goto step_next;
        case 1446: goto step_next;
        // -- mread
        case 1447: goto step_next;
        case 1448: _wait();_mread(cpu->addr);goto step_next;
        case 1449: cpu->dlatch=_gd();if(!_z80_cb_action(cpu,6,cpu->opcode&7)){_z80_skip(cpu,3);};goto step_next;
        case 1450: goto step_next;
        // -- mwrite
        case 1451: goto step_next;
        case 1452: _wait();_mwrite(cpu->addr,cpu->dlatch);;goto step_next;
        case 1453: goto step_next;
        // -- overlapped
        case 1454: goto fetch_next;
        
        //  00: int_im0 (M:6 T:9)
        // -- generic
        case 1455: pins=_z80_int012_step0(cpu,pins);goto step_next;
        // -- generic
        case 1456: pins=_z80_int012_step1(cpu,pins);goto step_next;
        // -- generic
        case 1457: _wait();pins=_z80_int0_step2(cpu,pins);goto step_next;
        // -- generic
        case 1458: pins=_z80_refresh(cpu,pins);goto step_next;
        // -- generic
        case 1459: pins=_z80_int0_step3(cpu,pins);goto step_next;
        // -- overlapped
        case 1460: goto fetch_next;
        
        //  00: int_im1 (M:7 T:16)
        // -- generic
        case 1461: pins=_z80_int012_step0(cpu,pins);goto step_next;
        // -- generic
        case 1462: pins=_z80_int012_step1(cpu,pins);goto step_next;
        // -- generic
        case 1463: _wait();;goto step_next;
        // -- generic
        case 1464: pins=_z80_refresh(cpu,pins);goto step_next;
        case 1465: goto step_next;
        case 1466: goto step_next;
        // -- mwrite
        case 1467: goto step_next;
        case 1468: _wait();_mwrite(--cpu->sp,cpu->pch);;goto step_next;
        case 1469: goto step_next;
        // -- mwrite
        case 1470: goto step_next;
        case 1471: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=cpu->pc=0x0038;goto step_next;
        case 1472: goto step_next;
        // -- overlapped
        case 1473: goto fetch_next;
        
        //  00: int_im2 (M:9 T:22)
        // -- generic
        case 1474: pins=_z80_int012_step0(cpu,pins);goto step_next;
        // -- generic
        case 1475: pins=_z80_int012_step1(cpu,pins);goto step_next;
        // -- generic
        case 1476: _wait();cpu->dlatch=_z80_get_db(pins);goto step_next;
        // -- generic
        case 1477: pins=_z80_refresh(cpu,pins);goto step_next;
        case 1478: goto step_next;
        case 1479: goto step_next;
        // -- mwrite
        case 1480: goto step_next;
        case 1481: _wait();_mwrite(--cpu->sp,cpu->pch);;goto step_next;
        case 1482: goto step_next;
        // -- mwrite
        case 1483: goto step_next;
        case 1484: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wzl=cpu->dlatch;cpu->wzh=cpu->i;;goto step_next;
        case 1485: goto step_next;
        // -- mread
        case 1486: goto step_next;
        case 1487: _wait();_mread(cpu->wz++);goto step_next;
        case 1488: cpu->dlatch=_gd();;goto step_next;
        // -- mread
        case 1489: goto step_next;
        case 1490: _wait();_mread(cpu->wz);goto step_next;
        case 1491: cpu->wzh=_gd();cpu->wzl=cpu->dlatch;cpu->pc=cpu->wz;goto step_next;
        // -- overlapped
        case 1492: goto fetch_next;
        
        //  00: nmi (M:5 T:14)
        // -- generic
        case 1493: pins=_z80_nmi_step0(cpu,pins);goto step_next;
        // -- generic
        case 1494: pins=_z80_refresh(cpu,pins);goto step_next;
        case 1495: goto step_next;
        case 1496: goto step_next;
        // -- mwrite
        case 1497: goto step_next;
        case 1498: _wait();_mwrite(--cpu->sp,cpu->pch);;goto step_next;
        case 1499: goto step_next;
        // -- mwrite
        case 1500: goto step_next;
        case 1501: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=cpu->pc=0x0066;goto step_next;
        case 1502: goto step_next;
        // -- overlapped
        case 1503: goto fetch_next;

        default: _Z80_UNREACHABLE;
    }
fetch_next: pins = _z80_fetch(cpu, pins);
step_next:  cpu->op.step += 1;
track_int_bits: {
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
