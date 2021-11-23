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
    {   24, 0 },  //  00: nop (M:1 T:4 steps:1)
    {   25, 0 },  //  01: ld bc,nn (M:3 T:10 steps:7)
    {   32, 0 },  //  02: ld (bc),a (M:2 T:7 steps:4)
    {   36, 0 },  //  03: inc bc (M:2 T:6 steps:3)
    {   39, 0 },  //  04: inc b (M:1 T:4 steps:1)
    {   40, 0 },  //  05: dec b (M:1 T:4 steps:1)
    {   41, _Z80_OPSTATE_FLAGS_IMM8 },  //  06: ld b,n (M:2 T:7 steps:4)
    {   45, 0 },  //  07: rlca (M:1 T:4 steps:1)
    {   46, 0 },  //  08: ex af,af' (M:1 T:4 steps:1)
    {   47, 0 },  //  09: add hl,bc (M:2 T:11 steps:8)
    {   55, 0 },  //  0A: ld a,(bc) (M:2 T:7 steps:4)
    {   59, 0 },  //  0B: dec bc (M:2 T:6 steps:3)
    {   62, 0 },  //  0C: inc c (M:1 T:4 steps:1)
    {   63, 0 },  //  0D: dec c (M:1 T:4 steps:1)
    {   64, _Z80_OPSTATE_FLAGS_IMM8 },  //  0E: ld c,n (M:2 T:7 steps:4)
    {   68, 0 },  //  0F: rrca (M:1 T:4 steps:1)
    {   69, 0 },  //  10: djnz d (M:4 T:13 steps:10)
    {   79, 0 },  //  11: ld de,nn (M:3 T:10 steps:7)
    {   86, 0 },  //  12: ld (de),a (M:2 T:7 steps:4)
    {   90, 0 },  //  13: inc de (M:2 T:6 steps:3)
    {   93, 0 },  //  14: inc d (M:1 T:4 steps:1)
    {   94, 0 },  //  15: dec d (M:1 T:4 steps:1)
    {   95, _Z80_OPSTATE_FLAGS_IMM8 },  //  16: ld d,n (M:2 T:7 steps:4)
    {   99, 0 },  //  17: rla (M:1 T:4 steps:1)
    {  100, 0 },  //  18: jr d (M:3 T:12 steps:9)
    {  109, 0 },  //  19: add hl,de (M:2 T:11 steps:8)
    {  117, 0 },  //  1A: ld a,(de) (M:2 T:7 steps:4)
    {  121, 0 },  //  1B: dec de (M:2 T:6 steps:3)
    {  124, 0 },  //  1C: inc e (M:1 T:4 steps:1)
    {  125, 0 },  //  1D: dec e (M:1 T:4 steps:1)
    {  126, _Z80_OPSTATE_FLAGS_IMM8 },  //  1E: ld e,n (M:2 T:7 steps:4)
    {  130, 0 },  //  1F: rra (M:1 T:4 steps:1)
    {  131, 0 },  //  20: jr nz,d (M:3 T:12 steps:9)
    {  140, 0 },  //  21: ld hl,nn (M:3 T:10 steps:7)
    {  147, 0 },  //  22: ld (nn),hl (M:5 T:16 steps:13)
    {  160, 0 },  //  23: inc hl (M:2 T:6 steps:3)
    {  163, 0 },  //  24: inc h (M:1 T:4 steps:1)
    {  164, 0 },  //  25: dec h (M:1 T:4 steps:1)
    {  165, _Z80_OPSTATE_FLAGS_IMM8 },  //  26: ld h,n (M:2 T:7 steps:4)
    {  169, 0 },  //  27: daa (M:1 T:4 steps:1)
    {  170, 0 },  //  28: jr z,d (M:3 T:12 steps:9)
    {  179, 0 },  //  29: add hl,hl (M:2 T:11 steps:8)
    {  187, 0 },  //  2A: ld hl,(nn) (M:5 T:16 steps:13)
    {  200, 0 },  //  2B: dec hl (M:2 T:6 steps:3)
    {  203, 0 },  //  2C: inc l (M:1 T:4 steps:1)
    {  204, 0 },  //  2D: dec l (M:1 T:4 steps:1)
    {  205, _Z80_OPSTATE_FLAGS_IMM8 },  //  2E: ld l,n (M:2 T:7 steps:4)
    {  209, 0 },  //  2F: cpl (M:1 T:4 steps:1)
    {  210, 0 },  //  30: jr nc,d (M:3 T:12 steps:9)
    {  219, 0 },  //  31: ld sp,nn (M:3 T:10 steps:7)
    {  226, 0 },  //  32: ld (nn),a (M:4 T:13 steps:10)
    {  236, 0 },  //  33: inc sp (M:2 T:6 steps:3)
    {  239, _Z80_OPSTATE_FLAGS_INDIRECT },  //  34: inc (hl) (M:3 T:11 steps:8)
    {  247, _Z80_OPSTATE_FLAGS_INDIRECT },  //  35: dec (hl) (M:3 T:11 steps:8)
    {  255, _Z80_OPSTATE_FLAGS_INDIRECT|_Z80_OPSTATE_FLAGS_IMM8 },  //  36: ld (hl),n (M:3 T:10 steps:7)
    {  262, 0 },  //  37: scf (M:1 T:4 steps:1)
    {  263, 0 },  //  38: jr c,d (M:3 T:12 steps:9)
    {  272, 0 },  //  39: add hl,sp (M:2 T:11 steps:8)
    {  280, 0 },  //  3A: ld a,(nn) (M:4 T:13 steps:10)
    {  290, 0 },  //  3B: dec sp (M:2 T:6 steps:3)
    {  293, 0 },  //  3C: inc a (M:1 T:4 steps:1)
    {  294, 0 },  //  3D: dec a (M:1 T:4 steps:1)
    {  295, _Z80_OPSTATE_FLAGS_IMM8 },  //  3E: ld a,n (M:2 T:7 steps:4)
    {  299, 0 },  //  3F: ccf (M:1 T:4 steps:1)
    {  300, 0 },  //  40: ld b,b (M:1 T:4 steps:1)
    {  301, 0 },  //  41: ld b,c (M:1 T:4 steps:1)
    {  302, 0 },  //  42: ld b,d (M:1 T:4 steps:1)
    {  303, 0 },  //  43: ld b,e (M:1 T:4 steps:1)
    {  304, 0 },  //  44: ld b,h (M:1 T:4 steps:1)
    {  305, 0 },  //  45: ld b,l (M:1 T:4 steps:1)
    {  306, _Z80_OPSTATE_FLAGS_INDIRECT },  //  46: ld b,(hl) (M:2 T:7 steps:4)
    {  310, 0 },  //  47: ld b,a (M:1 T:4 steps:1)
    {  311, 0 },  //  48: ld c,b (M:1 T:4 steps:1)
    {  312, 0 },  //  49: ld c,c (M:1 T:4 steps:1)
    {  313, 0 },  //  4A: ld c,d (M:1 T:4 steps:1)
    {  314, 0 },  //  4B: ld c,e (M:1 T:4 steps:1)
    {  315, 0 },  //  4C: ld c,h (M:1 T:4 steps:1)
    {  316, 0 },  //  4D: ld c,l (M:1 T:4 steps:1)
    {  317, _Z80_OPSTATE_FLAGS_INDIRECT },  //  4E: ld c,(hl) (M:2 T:7 steps:4)
    {  321, 0 },  //  4F: ld c,a (M:1 T:4 steps:1)
    {  322, 0 },  //  50: ld d,b (M:1 T:4 steps:1)
    {  323, 0 },  //  51: ld d,c (M:1 T:4 steps:1)
    {  324, 0 },  //  52: ld d,d (M:1 T:4 steps:1)
    {  325, 0 },  //  53: ld d,e (M:1 T:4 steps:1)
    {  326, 0 },  //  54: ld d,h (M:1 T:4 steps:1)
    {  327, 0 },  //  55: ld d,l (M:1 T:4 steps:1)
    {  328, _Z80_OPSTATE_FLAGS_INDIRECT },  //  56: ld d,(hl) (M:2 T:7 steps:4)
    {  332, 0 },  //  57: ld d,a (M:1 T:4 steps:1)
    {  333, 0 },  //  58: ld e,b (M:1 T:4 steps:1)
    {  334, 0 },  //  59: ld e,c (M:1 T:4 steps:1)
    {  335, 0 },  //  5A: ld e,d (M:1 T:4 steps:1)
    {  336, 0 },  //  5B: ld e,e (M:1 T:4 steps:1)
    {  337, 0 },  //  5C: ld e,h (M:1 T:4 steps:1)
    {  338, 0 },  //  5D: ld e,l (M:1 T:4 steps:1)
    {  339, _Z80_OPSTATE_FLAGS_INDIRECT },  //  5E: ld e,(hl) (M:2 T:7 steps:4)
    {  343, 0 },  //  5F: ld e,a (M:1 T:4 steps:1)
    {  344, 0 },  //  60: ld h,b (M:1 T:4 steps:1)
    {  345, 0 },  //  61: ld h,c (M:1 T:4 steps:1)
    {  346, 0 },  //  62: ld h,d (M:1 T:4 steps:1)
    {  347, 0 },  //  63: ld h,e (M:1 T:4 steps:1)
    {  348, 0 },  //  64: ld h,h (M:1 T:4 steps:1)
    {  349, 0 },  //  65: ld h,l (M:1 T:4 steps:1)
    {  350, _Z80_OPSTATE_FLAGS_INDIRECT },  //  66: ld h,(hl) (M:2 T:7 steps:4)
    {  354, 0 },  //  67: ld h,a (M:1 T:4 steps:1)
    {  355, 0 },  //  68: ld l,b (M:1 T:4 steps:1)
    {  356, 0 },  //  69: ld l,c (M:1 T:4 steps:1)
    {  357, 0 },  //  6A: ld l,d (M:1 T:4 steps:1)
    {  358, 0 },  //  6B: ld l,e (M:1 T:4 steps:1)
    {  359, 0 },  //  6C: ld l,h (M:1 T:4 steps:1)
    {  360, 0 },  //  6D: ld l,l (M:1 T:4 steps:1)
    {  361, _Z80_OPSTATE_FLAGS_INDIRECT },  //  6E: ld l,(hl) (M:2 T:7 steps:4)
    {  365, 0 },  //  6F: ld l,a (M:1 T:4 steps:1)
    {  366, _Z80_OPSTATE_FLAGS_INDIRECT },  //  70: ld (hl),b (M:2 T:7 steps:4)
    {  370, _Z80_OPSTATE_FLAGS_INDIRECT },  //  71: ld (hl),c (M:2 T:7 steps:4)
    {  374, _Z80_OPSTATE_FLAGS_INDIRECT },  //  72: ld (hl),d (M:2 T:7 steps:4)
    {  378, _Z80_OPSTATE_FLAGS_INDIRECT },  //  73: ld (hl),e (M:2 T:7 steps:4)
    {  382, _Z80_OPSTATE_FLAGS_INDIRECT },  //  74: ld (hl),h (M:2 T:7 steps:4)
    {  386, _Z80_OPSTATE_FLAGS_INDIRECT },  //  75: ld (hl),l (M:2 T:7 steps:4)
    {  390, 0 },  //  76: halt (M:1 T:4 steps:1)
    {  391, _Z80_OPSTATE_FLAGS_INDIRECT },  //  77: ld (hl),a (M:2 T:7 steps:4)
    {  395, 0 },  //  78: ld a,b (M:1 T:4 steps:1)
    {  396, 0 },  //  79: ld a,c (M:1 T:4 steps:1)
    {  397, 0 },  //  7A: ld a,d (M:1 T:4 steps:1)
    {  398, 0 },  //  7B: ld a,e (M:1 T:4 steps:1)
    {  399, 0 },  //  7C: ld a,h (M:1 T:4 steps:1)
    {  400, 0 },  //  7D: ld a,l (M:1 T:4 steps:1)
    {  401, _Z80_OPSTATE_FLAGS_INDIRECT },  //  7E: ld a,(hl) (M:2 T:7 steps:4)
    {  405, 0 },  //  7F: ld a,a (M:1 T:4 steps:1)
    {  406, 0 },  //  80: add b (M:1 T:4 steps:1)
    {  407, 0 },  //  81: add c (M:1 T:4 steps:1)
    {  408, 0 },  //  82: add d (M:1 T:4 steps:1)
    {  409, 0 },  //  83: add e (M:1 T:4 steps:1)
    {  410, 0 },  //  84: add h (M:1 T:4 steps:1)
    {  411, 0 },  //  85: add l (M:1 T:4 steps:1)
    {  412, _Z80_OPSTATE_FLAGS_INDIRECT },  //  86: add (hl) (M:2 T:7 steps:4)
    {  416, 0 },  //  87: add a (M:1 T:4 steps:1)
    {  417, 0 },  //  88: adc b (M:1 T:4 steps:1)
    {  418, 0 },  //  89: adc c (M:1 T:4 steps:1)
    {  419, 0 },  //  8A: adc d (M:1 T:4 steps:1)
    {  420, 0 },  //  8B: adc e (M:1 T:4 steps:1)
    {  421, 0 },  //  8C: adc h (M:1 T:4 steps:1)
    {  422, 0 },  //  8D: adc l (M:1 T:4 steps:1)
    {  423, _Z80_OPSTATE_FLAGS_INDIRECT },  //  8E: adc (hl) (M:2 T:7 steps:4)
    {  427, 0 },  //  8F: adc a (M:1 T:4 steps:1)
    {  428, 0 },  //  90: sub b (M:1 T:4 steps:1)
    {  429, 0 },  //  91: sub c (M:1 T:4 steps:1)
    {  430, 0 },  //  92: sub d (M:1 T:4 steps:1)
    {  431, 0 },  //  93: sub e (M:1 T:4 steps:1)
    {  432, 0 },  //  94: sub h (M:1 T:4 steps:1)
    {  433, 0 },  //  95: sub l (M:1 T:4 steps:1)
    {  434, _Z80_OPSTATE_FLAGS_INDIRECT },  //  96: sub (hl) (M:2 T:7 steps:4)
    {  438, 0 },  //  97: sub a (M:1 T:4 steps:1)
    {  439, 0 },  //  98: sbc b (M:1 T:4 steps:1)
    {  440, 0 },  //  99: sbc c (M:1 T:4 steps:1)
    {  441, 0 },  //  9A: sbc d (M:1 T:4 steps:1)
    {  442, 0 },  //  9B: sbc e (M:1 T:4 steps:1)
    {  443, 0 },  //  9C: sbc h (M:1 T:4 steps:1)
    {  444, 0 },  //  9D: sbc l (M:1 T:4 steps:1)
    {  445, _Z80_OPSTATE_FLAGS_INDIRECT },  //  9E: sbc (hl) (M:2 T:7 steps:4)
    {  449, 0 },  //  9F: sbc a (M:1 T:4 steps:1)
    {  450, 0 },  //  A0: and b (M:1 T:4 steps:1)
    {  451, 0 },  //  A1: and c (M:1 T:4 steps:1)
    {  452, 0 },  //  A2: and d (M:1 T:4 steps:1)
    {  453, 0 },  //  A3: and e (M:1 T:4 steps:1)
    {  454, 0 },  //  A4: and h (M:1 T:4 steps:1)
    {  455, 0 },  //  A5: and l (M:1 T:4 steps:1)
    {  456, _Z80_OPSTATE_FLAGS_INDIRECT },  //  A6: and (hl) (M:2 T:7 steps:4)
    {  460, 0 },  //  A7: and a (M:1 T:4 steps:1)
    {  461, 0 },  //  A8: xor b (M:1 T:4 steps:1)
    {  462, 0 },  //  A9: xor c (M:1 T:4 steps:1)
    {  463, 0 },  //  AA: xor d (M:1 T:4 steps:1)
    {  464, 0 },  //  AB: xor e (M:1 T:4 steps:1)
    {  465, 0 },  //  AC: xor h (M:1 T:4 steps:1)
    {  466, 0 },  //  AD: xor l (M:1 T:4 steps:1)
    {  467, _Z80_OPSTATE_FLAGS_INDIRECT },  //  AE: xor (hl) (M:2 T:7 steps:4)
    {  471, 0 },  //  AF: xor a (M:1 T:4 steps:1)
    {  472, 0 },  //  B0: or b (M:1 T:4 steps:1)
    {  473, 0 },  //  B1: or c (M:1 T:4 steps:1)
    {  474, 0 },  //  B2: or d (M:1 T:4 steps:1)
    {  475, 0 },  //  B3: or e (M:1 T:4 steps:1)
    {  476, 0 },  //  B4: or h (M:1 T:4 steps:1)
    {  477, 0 },  //  B5: or l (M:1 T:4 steps:1)
    {  478, _Z80_OPSTATE_FLAGS_INDIRECT },  //  B6: or (hl) (M:2 T:7 steps:4)
    {  482, 0 },  //  B7: or a (M:1 T:4 steps:1)
    {  483, 0 },  //  B8: cp b (M:1 T:4 steps:1)
    {  484, 0 },  //  B9: cp c (M:1 T:4 steps:1)
    {  485, 0 },  //  BA: cp d (M:1 T:4 steps:1)
    {  486, 0 },  //  BB: cp e (M:1 T:4 steps:1)
    {  487, 0 },  //  BC: cp h (M:1 T:4 steps:1)
    {  488, 0 },  //  BD: cp l (M:1 T:4 steps:1)
    {  489, _Z80_OPSTATE_FLAGS_INDIRECT },  //  BE: cp (hl) (M:2 T:7 steps:4)
    {  493, 0 },  //  BF: cp a (M:1 T:4 steps:1)
    {  494, 0 },  //  C0: ret nz (M:4 T:11 steps:8)
    {  502, 0 },  //  C1: pop bc (M:3 T:10 steps:7)
    {  509, 0 },  //  C2: jp nz,nn (M:3 T:10 steps:7)
    {  516, 0 },  //  C3: jp nn (M:3 T:10 steps:7)
    {  523, 0 },  //  C4: call nz,nn (M:6 T:17 steps:14)
    {  537, 0 },  //  C5: push bc (M:4 T:11 steps:8)
    {  545, _Z80_OPSTATE_FLAGS_IMM8 },  //  C6: add n (M:2 T:7 steps:4)
    {  549, 0 },  //  C7: rst 0h (M:4 T:11 steps:8)
    {  557, 0 },  //  C8: ret z (M:4 T:11 steps:8)
    {  565, 0 },  //  C9: ret (M:3 T:10 steps:7)
    {  572, 0 },  //  CA: jp z,nn (M:3 T:10 steps:7)
    {  579, 0 },  //  CB: cb prefix (M:1 T:4 steps:1)
    {  580, 0 },  //  CC: call z,nn (M:6 T:17 steps:14)
    {  594, 0 },  //  CD: call nn (M:5 T:17 steps:14)
    {  608, _Z80_OPSTATE_FLAGS_IMM8 },  //  CE: adc n (M:2 T:7 steps:4)
    {  612, 0 },  //  CF: rst 8h (M:4 T:11 steps:8)
    {  620, 0 },  //  D0: ret nc (M:4 T:11 steps:8)
    {  628, 0 },  //  D1: pop de (M:3 T:10 steps:7)
    {  635, 0 },  //  D2: jp nc,nn (M:3 T:10 steps:7)
    {  642, 0 },  //  D3: out (n),a (M:3 T:11 steps:8)
    {  650, 0 },  //  D4: call nc,nn (M:6 T:17 steps:14)
    {  664, 0 },  //  D5: push de (M:4 T:11 steps:8)
    {  672, _Z80_OPSTATE_FLAGS_IMM8 },  //  D6: sub n (M:2 T:7 steps:4)
    {  676, 0 },  //  D7: rst 10h (M:4 T:11 steps:8)
    {  684, 0 },  //  D8: ret c (M:4 T:11 steps:8)
    {  692, 0 },  //  D9: exx (M:1 T:4 steps:1)
    {  693, 0 },  //  DA: jp c,nn (M:3 T:10 steps:7)
    {  700, 0 },  //  DB: in a,(n) (M:3 T:11 steps:8)
    {  708, 0 },  //  DC: call c,nn (M:6 T:17 steps:14)
    {  722, 0 },  //  DD: dd prefix (M:1 T:4 steps:1)
    {  723, _Z80_OPSTATE_FLAGS_IMM8 },  //  DE: sbc n (M:2 T:7 steps:4)
    {  727, 0 },  //  DF: rst 18h (M:4 T:11 steps:8)
    {  735, 0 },  //  E0: ret po (M:4 T:11 steps:8)
    {  743, 0 },  //  E1: pop hl (M:3 T:10 steps:7)
    {  750, 0 },  //  E2: jp po,nn (M:3 T:10 steps:7)
    {  757, 0 },  //  E3: ex (sp),hl (M:5 T:19 steps:16)
    {  773, 0 },  //  E4: call po,nn (M:6 T:17 steps:14)
    {  787, 0 },  //  E5: push hl (M:4 T:11 steps:8)
    {  795, _Z80_OPSTATE_FLAGS_IMM8 },  //  E6: and n (M:2 T:7 steps:4)
    {  799, 0 },  //  E7: rst 20h (M:4 T:11 steps:8)
    {  807, 0 },  //  E8: ret pe (M:4 T:11 steps:8)
    {  815, 0 },  //  E9: jp hl (M:1 T:4 steps:1)
    {  816, 0 },  //  EA: jp pe,nn (M:3 T:10 steps:7)
    {  823, 0 },  //  EB: ex de,hl (M:1 T:4 steps:1)
    {  824, 0 },  //  EC: call pe,nn (M:6 T:17 steps:14)
    {  838, 0 },  //  ED: ed prefix (M:1 T:4 steps:1)
    {  839, _Z80_OPSTATE_FLAGS_IMM8 },  //  EE: xor n (M:2 T:7 steps:4)
    {  843, 0 },  //  EF: rst 28h (M:4 T:11 steps:8)
    {  851, 0 },  //  F0: ret p (M:4 T:11 steps:8)
    {  859, 0 },  //  F1: pop af (M:3 T:10 steps:7)
    {  866, 0 },  //  F2: jp p,nn (M:3 T:10 steps:7)
    {  873, 0 },  //  F3: di (M:1 T:4 steps:1)
    {  874, 0 },  //  F4: call p,nn (M:6 T:17 steps:14)
    {  888, 0 },  //  F5: push af (M:4 T:11 steps:8)
    {  896, _Z80_OPSTATE_FLAGS_IMM8 },  //  F6: or n (M:2 T:7 steps:4)
    {  900, 0 },  //  F7: rst 30h (M:4 T:11 steps:8)
    {  908, 0 },  //  F8: ret m (M:4 T:11 steps:8)
    {  916, 0 },  //  F9: ld sp,hl (M:2 T:6 steps:3)
    {  919, 0 },  //  FA: jp m,nn (M:3 T:10 steps:7)
    {  926, 0 },  //  FB: ei (M:1 T:4 steps:1)
    {  927, 0 },  //  FC: call m,nn (M:6 T:17 steps:14)
    {  941, 0 },  //  FD: fd prefix (M:1 T:4 steps:1)
    {  942, _Z80_OPSTATE_FLAGS_IMM8 },  //  FE: cp n (M:2 T:7 steps:4)
    {  946, 0 },  //  FF: rst 38h (M:4 T:11 steps:8)
    {  954, 0 },  // ED 00: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 01: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 02: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 03: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 04: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 05: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 06: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 07: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 08: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 09: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 0A: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 0B: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 0C: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 0D: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 0E: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 0F: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 10: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 11: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 12: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 13: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 14: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 15: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 16: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 17: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 18: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 19: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 1A: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 1B: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 1C: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 1D: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 1E: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 1F: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 20: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 21: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 22: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 23: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 24: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 25: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 26: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 27: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 28: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 29: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 2A: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 2B: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 2C: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 2D: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 2E: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 2F: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 30: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 31: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 32: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 33: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 34: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 35: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 36: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 37: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 38: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 39: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 3A: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 3B: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 3C: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 3D: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 3E: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 3F: ed nop (M:1 T:4 steps:1)
    {  955, 0 },  // ED 40: in b,(c) (M:2 T:8 steps:5)
    {  960, 0 },  // ED 41: out (c),b (M:2 T:8 steps:5)
    {  965, 0 },  // ED 42: sbc hl,bc (M:2 T:11 steps:8)
    {  973, 0 },  // ED 43: ld (nn),bc (M:5 T:16 steps:13)
    {  986, 0 },  // ED 44: neg (M:1 T:4 steps:1)
    {  987, 0 },  // ED 45: reti/retn (M:3 T:10 steps:7)
    {  994, 0 },  // ED 46: im 0 (M:1 T:4 steps:1)
    {  995, 0 },  // ED 47: ld i,a (M:2 T:5 steps:2)
    {  997, 0 },  // ED 48: in c,(c) (M:2 T:8 steps:5)
    { 1002, 0 },  // ED 49: out (c),c (M:2 T:8 steps:5)
    { 1007, 0 },  // ED 4A: adc hl,bc (M:2 T:11 steps:8)
    { 1015, 0 },  // ED 4B: ld bc,(nn) (M:5 T:16 steps:13)
    {  986, 0 },  // ED 4C: neg (M:1 T:4 steps:1)
    {  987, 0 },  // ED 4D: reti/retn (M:3 T:10 steps:7)
    { 1028, 0 },  // ED 4E: im 0 (M:1 T:4 steps:1)
    { 1029, 0 },  // ED 4F: ld r,a (M:2 T:5 steps:2)
    { 1031, 0 },  // ED 50: in d,(c) (M:2 T:8 steps:5)
    { 1036, 0 },  // ED 51: out (c),d (M:2 T:8 steps:5)
    { 1041, 0 },  // ED 52: sbc hl,de (M:2 T:11 steps:8)
    { 1049, 0 },  // ED 53: ld (nn),de (M:5 T:16 steps:13)
    {  986, 0 },  // ED 54: neg (M:1 T:4 steps:1)
    {  987, 0 },  // ED 55: reti/retn (M:3 T:10 steps:7)
    { 1062, 0 },  // ED 56: im 1 (M:1 T:4 steps:1)
    { 1063, 0 },  // ED 57: ld a,i (M:2 T:5 steps:2)
    { 1065, 0 },  // ED 58: in e,(c) (M:2 T:8 steps:5)
    { 1070, 0 },  // ED 59: out (c),e (M:2 T:8 steps:5)
    { 1075, 0 },  // ED 5A: adc hl,de (M:2 T:11 steps:8)
    { 1083, 0 },  // ED 5B: ld de,(nn) (M:5 T:16 steps:13)
    {  986, 0 },  // ED 5C: neg (M:1 T:4 steps:1)
    {  987, 0 },  // ED 5D: reti/retn (M:3 T:10 steps:7)
    { 1096, 0 },  // ED 5E: im 2 (M:1 T:4 steps:1)
    { 1097, 0 },  // ED 5F: ld a,r (M:2 T:5 steps:2)
    { 1099, 0 },  // ED 60: in h,(c) (M:2 T:8 steps:5)
    { 1104, 0 },  // ED 61: out (c),h (M:2 T:8 steps:5)
    { 1109, 0 },  // ED 62: sbc hl,hl (M:2 T:11 steps:8)
    { 1117, 0 },  // ED 63: ld (nn),hl (M:5 T:16 steps:13)
    {  986, 0 },  // ED 64: neg (M:1 T:4 steps:1)
    {  987, 0 },  // ED 65: reti/retn (M:3 T:10 steps:7)
    { 1130, 0 },  // ED 66: im 0 (M:1 T:4 steps:1)
    { 1131, 0 },  // ED 67: rrd (M:4 T:14 steps:11)
    { 1142, 0 },  // ED 68: in l,(c) (M:2 T:8 steps:5)
    { 1147, 0 },  // ED 69: out (c),l (M:2 T:8 steps:5)
    { 1152, 0 },  // ED 6A: adc hl,hl (M:2 T:11 steps:8)
    { 1160, 0 },  // ED 6B: ld hl,(nn) (M:5 T:16 steps:13)
    {  986, 0 },  // ED 6C: neg (M:1 T:4 steps:1)
    {  987, 0 },  // ED 6D: reti/retn (M:3 T:10 steps:7)
    { 1173, 0 },  // ED 6E: im 0 (M:1 T:4 steps:1)
    { 1174, 0 },  // ED 6F: rld (M:4 T:14 steps:11)
    { 1185, 0 },  // ED 70: in (c) (M:2 T:8 steps:5)
    { 1190, 0 },  // ED 71: out (c),0 (M:2 T:8 steps:5)
    { 1195, 0 },  // ED 72: sbc hl,sp (M:2 T:11 steps:8)
    { 1203, 0 },  // ED 73: ld (nn),sp (M:5 T:16 steps:13)
    {  986, 0 },  // ED 74: neg (M:1 T:4 steps:1)
    {  987, 0 },  // ED 75: reti/retn (M:3 T:10 steps:7)
    { 1216, 0 },  // ED 76: im 1 (M:1 T:4 steps:1)
    {  954, 0 },  // ED 77: ed nop (M:1 T:4 steps:1)
    { 1217, 0 },  // ED 78: in a,(c) (M:2 T:8 steps:5)
    { 1222, 0 },  // ED 79: out (c),a (M:2 T:8 steps:5)
    { 1227, 0 },  // ED 7A: adc hl,sp (M:2 T:11 steps:8)
    { 1235, 0 },  // ED 7B: ld sp,(nn) (M:5 T:16 steps:13)
    {  986, 0 },  // ED 7C: neg (M:1 T:4 steps:1)
    {  987, 0 },  // ED 7D: reti/retn (M:3 T:10 steps:7)
    { 1248, 0 },  // ED 7E: im 2 (M:1 T:4 steps:1)
    {  954, 0 },  // ED 7F: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 80: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 81: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 82: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 83: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 84: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 85: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 86: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 87: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 88: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 89: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 8A: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 8B: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 8C: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 8D: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 8E: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 8F: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 90: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 91: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 92: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 93: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 94: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 95: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 96: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 97: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 98: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 99: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 9A: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 9B: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 9C: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 9D: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 9E: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED 9F: ed nop (M:1 T:4 steps:1)
    { 1249, 0 },  // ED A0: ldi (M:4 T:12 steps:9)
    { 1258, 0 },  // ED A1: cpi (M:3 T:12 steps:9)
    { 1267, 0 },  // ED A2: ini (M:4 T:12 steps:9)
    { 1276, 0 },  // ED A3: outi (M:4 T:12 steps:9)
    {  954, 0 },  // ED A4: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED A5: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED A6: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED A7: ed nop (M:1 T:4 steps:1)
    { 1285, 0 },  // ED A8: ldd (M:4 T:12 steps:9)
    { 1294, 0 },  // ED A9: cpd (M:3 T:12 steps:9)
    { 1303, 0 },  // ED AA: ind (M:4 T:12 steps:9)
    { 1312, 0 },  // ED AB: outd (M:4 T:12 steps:9)
    {  954, 0 },  // ED AC: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED AD: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED AE: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED AF: ed nop (M:1 T:4 steps:1)
    { 1321, 0 },  // ED B0: ldir (M:5 T:17 steps:14)
    { 1335, 0 },  // ED B1: cpir (M:4 T:17 steps:14)
    { 1349, 0 },  // ED B2: inir (M:5 T:17 steps:14)
    { 1363, 0 },  // ED B3: otir (M:5 T:17 steps:14)
    {  954, 0 },  // ED B4: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED B5: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED B6: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED B7: ed nop (M:1 T:4 steps:1)
    { 1377, 0 },  // ED B8: lddr (M:5 T:17 steps:14)
    { 1391, 0 },  // ED B9: cpdr (M:4 T:17 steps:14)
    { 1405, 0 },  // ED BA: indr (M:5 T:17 steps:14)
    { 1419, 0 },  // ED BB: otdr (M:5 T:17 steps:14)
    {  954, 0 },  // ED BC: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED BD: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED BE: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED BF: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED C0: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED C1: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED C2: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED C3: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED C4: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED C5: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED C6: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED C7: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED C8: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED C9: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED CA: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED CB: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED CC: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED CD: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED CE: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED CF: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED D0: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED D1: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED D2: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED D3: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED D4: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED D5: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED D6: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED D7: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED D8: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED D9: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED DA: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED DB: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED DC: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED DD: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED DE: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED DF: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED E0: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED E1: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED E2: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED E3: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED E4: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED E5: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED E6: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED E7: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED E8: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED E9: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED EA: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED EB: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED EC: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED ED: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED EE: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED EF: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED F0: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED F1: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED F2: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED F3: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED F4: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED F5: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED F6: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED F7: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED F8: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED F9: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED FA: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED FB: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED FC: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED FD: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED FE: ed nop (M:1 T:4 steps:1)
    {  954, 0 },  // ED FF: ed nop (M:1 T:4 steps:1)
    { 1433, 0 },  // CB 00: cb (M:1 T:4 steps:1)
    { 1434, 0 },  // CB 01: cbhl (M:3 T:11 steps:8)
    { 1442, 0 },  // CB 02: ddfdcb (M:6 T:18 steps:15)
    { 1457, 0 },  //  03: int_im0 (M:6 T:9 steps:6)
    { 1463, 0 },  //  04: int_im1 (M:7 T:16 steps:13)
    { 1476, 0 },  //  05: int_im2 (M:9 T:22 steps:19)
    { 1495, 0 },  //  06: nmi (M:5 T:14 steps:11)

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

// initiate a fetch machine cycle for regular (non-prefixed) instructions, or initiate interrupt handling
static inline uint64_t _z80_fetch(z80_t* cpu, uint64_t pins) {
    cpu->prefix_state = 0;
    // shortcut no interrupts requested
    if (cpu->int_bits == 0) {
        cpu->op.step = 0xFFFF;
        return _z80_set_ab_x(pins, cpu->pc++, Z80_M1|Z80_MREQ|Z80_RD);
    }
    else if (cpu->int_bits & Z80_NMI) {
        // non-maskable interrupt starts with a regular M1 machine cycle
        cpu->op = _z80_opstate_table[_Z80_OPSTATE_SLOT_NMI];
        cpu->int_bits = 0;
        // NOTE: PC is *not* incremented!
        return _z80_set_ab_x(pins, cpu->pc, Z80_M1|Z80_MREQ|Z80_RD);
    }
    else if (cpu->int_bits & Z80_INT) {
        if (cpu->iff1) {
            // maskable interrupts start with a special M1 machine cycle which
            // doesn't fetch the next opcode, but instead activate the
            // pins M1|IOQR to request a special byte which is handled differently
            // depending on interrupt mode
            cpu->op = _z80_opstate_table[_Z80_OPSTATE_SLOT_INT_IM0 + cpu->im];
            cpu->int_bits = 0;
            // NOTE: PC is not incremented, and no pins are activated here
            return pins;
        }
        else {
            // oops, maskable interrupt requested but disabled
            cpu->op.step = 0xFFFF;
            return _z80_set_ab_x(pins, cpu->pc++, Z80_M1|Z80_MREQ|Z80_RD);
        }
    }
    else {
        _Z80_UNREACHABLE;
        return pins;
    }
}

static inline uint64_t _z80_fetch_cb(z80_t* cpu, uint64_t pins) {
    cpu->prefix = (cpu->prefix & (_Z80_PREFIX_DD|_Z80_PREFIX_FD)) | _Z80_PREFIX_CB;
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
        cpu->op.step = 21; // => step 22
        pins = _z80_set_ab_x(pins, cpu->pc++, Z80_M1|Z80_MREQ|Z80_RD);
    }
    return pins;
}

static inline uint64_t _z80_fetch_dd(z80_t* cpu, uint64_t pins) {
    cpu->op.step = 2;   // => step 3
    cpu->prefix_offset = 0;
    cpu->hlx_idx = 1;
    cpu->prefix = _Z80_PREFIX_DD;
    return _z80_set_ab_x(pins, cpu->pc++, Z80_M1|Z80_MREQ|Z80_RD);
}

static inline uint64_t _z80_fetch_fd(z80_t* cpu, uint64_t pins) {
    cpu->op.step = 2;   // => step 3
    cpu->prefix_offset = 0;
    cpu->hlx_idx = 2;
    cpu->prefix = _Z80_PREFIX_FD;
    return _z80_set_ab_x(pins, cpu->pc++, Z80_M1|Z80_MREQ|Z80_RD);
}

static inline uint64_t _z80_fetch_ed(z80_t* cpu, uint64_t pins) {
    cpu->op.step = 0xFFFF;
    cpu->prefix_offset = 0x0100;
    cpu->hlx_idx = 0;
    cpu->prefix = _Z80_PREFIX_ED;
    return _z80_set_ab_x(pins, cpu->pc++, Z80_M1|Z80_MREQ|Z80_RD);
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
#define _skip(n)        cpu->op.step+=(n);
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
    // process the next active tcycle
    pins &= ~(Z80_CTRL_PIN_MASK|Z80_RETI);
    switch (cpu->op.step) {
        //=== shared fetch machine cycle for non-DD/FD-prefixed ops
        // M1/T2: load opcode from data bus
        case 0: _wait(); cpu->opcode = _gd(); goto step_next;
        // M1/T3: refresh cycle
        case 1: pins = _z80_refresh(cpu, pins); goto step_next;
        // M1/T4: branch to instruction 'payload'
        case 2: {
            cpu->op = _z80_opstate_table[cpu->opcode + cpu->prefix_offset];
            // this is only needed for (HL) ops, but probably pointless to do a 
            // conditional branch just for this
            cpu->addr = cpu->hlx[cpu->hlx_idx].hl;
        } goto step_next;
        //=== shared fetch machine cycle for DD/FD-prefixed ops
        // M1/T2: load opcode from data bus
        case 3: _wait(); cpu->opcode = _gd(); goto step_next;
        // M1/T3: refresh cycle
        case 4: pins = _z80_refresh(cpu, pins); goto step_next;
        // M1/T4: branch to instruction 'payload'
        case 5: {
            cpu->op = _z80_opstate_table[cpu->opcode];
            cpu->addr = cpu->hlx[cpu->hlx_idx].hl;
            if (cpu->op.flags & _Z80_OPSTATE_FLAGS_INDIRECT) {
                if (cpu->op.flags & _Z80_OPSTATE_FLAGS_IMM8) {
                    // special case: if this is indirect+immediate (which is
                    // just LD (HL),n, then the immediate-load is 'hidden' within
                    // the 8-tcycle d-offset computation)
                    cpu->op.step = 13;  // continue at step 14
                }
                else {
                    // regular (IX+d)/(IY+d) instruction
                    cpu->op.step = 5;   // continue at step 6
                }
            }
        } goto step_next;
        //=== optional d-loading cycle for (IX+d), (IY+d)
        //--- mread
        case 6: goto step_next;
        case 7: _wait();_mread(cpu->pc++); goto step_next;
        case 8: cpu->addr += (int8_t)_gd(); cpu->wz = cpu->addr; goto step_next;
        //--- filler ticks
        case 9: goto step_next;
        case 10: goto step_next;
        case 11: goto step_next;
        case 12: goto step_next;
        case 13: {
            // branch to actual instruction
            cpu->op = _z80_opstate_table[cpu->opcode];
        } goto step_next;
        //=== special case d-loading cycle for (IX+d),n where the immediate load
        //    is hidden in the d-cycle load
        //--- mread for d offset
        case 14: goto step_next;
        case 15: _wait();_mread(cpu->pc++); goto step_next;
        case 16: cpu->addr += (int8_t)_gd(); cpu->wz = cpu->addr; goto step_next;
        //--- mread for n
        case 17: goto step_next;
        case 18: _wait();_mread(cpu->pc++); goto step_next;
        case 19: cpu->dlatch=_gd(); goto step_next;
        //--- filler tick
        case 20: goto step_next;
        case 21: {
            // branch to ld (hl),n and skip the original mread cycle for loading 'n'
            cpu->op = _z80_opstate_table[cpu->opcode];
            cpu->op.step += 3;
        } goto step_next;
        //=== special opcode fetch machine cycle for CB-prefixed instructions
        case 22: _wait(); cpu->opcode = _gd(); goto step_next;
        case 23: pins = _z80_refresh(cpu, pins); goto step_next;
        case 24: {
            if ((cpu->opcode & 7) == 6) {
                // this is a (HL) instruction
                cpu->addr = cpu->hl;
                cpu->op = _z80_opstate_table[_Z80_OPSTATE_SLOT_CBHL];
            }
            else {
                cpu->op = _z80_opstate_table[_Z80_OPSTATE_SLOT_CB];
            }
        } goto step_next;
        //=== from here on code-generated
        
        //  00: nop (M:1 T:4)
        // -- overlapped
        case   25: goto fetch_next;
        
        //  01: ld bc,nn (M:3 T:10)
        // -- mread
        case   26: goto step_next;
        case   27: _wait();_mread(cpu->pc++);goto step_next;
        case   28: cpu->c=_gd();goto step_next;
        // -- mread
        case   29: goto step_next;
        case   30: _wait();_mread(cpu->pc++);goto step_next;
        case   31: cpu->b=_gd();goto step_next;
        // -- overlapped
        case   32: goto fetch_next;
        
        //  02: ld (bc),a (M:2 T:7)
        // -- mwrite
        case   33: goto step_next;
        case   34: _wait();_mwrite(cpu->bc,cpu->a);cpu->wzl=cpu->c+1;cpu->wzh=cpu->a;goto step_next;
        case   35: goto step_next;
        // -- overlapped
        case   36: goto fetch_next;
        
        //  03: inc bc (M:2 T:6)
        // -- generic
        case   37: cpu->bc++;goto step_next;
        case   38: goto step_next;
        // -- overlapped
        case   39: goto fetch_next;
        
        //  04: inc b (M:1 T:4)
        // -- overlapped
        case   40: cpu->b=_z80_inc8(cpu,cpu->b);goto fetch_next;
        
        //  05: dec b (M:1 T:4)
        // -- overlapped
        case   41: cpu->b=_z80_dec8(cpu,cpu->b);goto fetch_next;
        
        //  06: ld b,n (M:2 T:7)
        // -- mread
        case   42: goto step_next;
        case   43: _wait();_mread(cpu->pc++);goto step_next;
        case   44: cpu->b=_gd();goto step_next;
        // -- overlapped
        case   45: goto fetch_next;
        
        //  07: rlca (M:1 T:4)
        // -- overlapped
        case   46: _z80_rlca(cpu);goto fetch_next;
        
        //  08: ex af,af' (M:1 T:4)
        // -- overlapped
        case   47: _z80_ex_af_af2(cpu);goto fetch_next;
        
        //  09: add hl,bc (M:2 T:11)
        // -- generic
        case   48: _z80_add16(cpu,cpu->bc);goto step_next;
        case   49: goto step_next;
        case   50: goto step_next;
        case   51: goto step_next;
        case   52: goto step_next;
        case   53: goto step_next;
        case   54: goto step_next;
        // -- overlapped
        case   55: goto fetch_next;
        
        //  0A: ld a,(bc) (M:2 T:7)
        // -- mread
        case   56: goto step_next;
        case   57: _wait();_mread(cpu->bc);goto step_next;
        case   58: cpu->a=_gd();cpu->wz=cpu->bc+1;goto step_next;
        // -- overlapped
        case   59: goto fetch_next;
        
        //  0B: dec bc (M:2 T:6)
        // -- generic
        case   60: cpu->bc--;goto step_next;
        case   61: goto step_next;
        // -- overlapped
        case   62: goto fetch_next;
        
        //  0C: inc c (M:1 T:4)
        // -- overlapped
        case   63: cpu->c=_z80_inc8(cpu,cpu->c);goto fetch_next;
        
        //  0D: dec c (M:1 T:4)
        // -- overlapped
        case   64: cpu->c=_z80_dec8(cpu,cpu->c);goto fetch_next;
        
        //  0E: ld c,n (M:2 T:7)
        // -- mread
        case   65: goto step_next;
        case   66: _wait();_mread(cpu->pc++);goto step_next;
        case   67: cpu->c=_gd();goto step_next;
        // -- overlapped
        case   68: goto fetch_next;
        
        //  0F: rrca (M:1 T:4)
        // -- overlapped
        case   69: _z80_rrca(cpu);goto fetch_next;
        
        //  10: djnz d (M:4 T:13)
        // -- generic
        case   70: goto step_next;
        // -- mread
        case   71: goto step_next;
        case   72: _wait();_mread(cpu->pc++);goto step_next;
        case   73: cpu->dlatch=_gd();if(--cpu->b==0){_skip(5);};goto step_next;
        // -- generic
        case   74: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;goto step_next;
        case   75: goto step_next;
        case   76: goto step_next;
        case   77: goto step_next;
        case   78: goto step_next;
        // -- overlapped
        case   79: goto fetch_next;
        
        //  11: ld de,nn (M:3 T:10)
        // -- mread
        case   80: goto step_next;
        case   81: _wait();_mread(cpu->pc++);goto step_next;
        case   82: cpu->e=_gd();goto step_next;
        // -- mread
        case   83: goto step_next;
        case   84: _wait();_mread(cpu->pc++);goto step_next;
        case   85: cpu->d=_gd();goto step_next;
        // -- overlapped
        case   86: goto fetch_next;
        
        //  12: ld (de),a (M:2 T:7)
        // -- mwrite
        case   87: goto step_next;
        case   88: _wait();_mwrite(cpu->de,cpu->a);cpu->wzl=cpu->e+1;cpu->wzh=cpu->a;goto step_next;
        case   89: goto step_next;
        // -- overlapped
        case   90: goto fetch_next;
        
        //  13: inc de (M:2 T:6)
        // -- generic
        case   91: cpu->de++;goto step_next;
        case   92: goto step_next;
        // -- overlapped
        case   93: goto fetch_next;
        
        //  14: inc d (M:1 T:4)
        // -- overlapped
        case   94: cpu->d=_z80_inc8(cpu,cpu->d);goto fetch_next;
        
        //  15: dec d (M:1 T:4)
        // -- overlapped
        case   95: cpu->d=_z80_dec8(cpu,cpu->d);goto fetch_next;
        
        //  16: ld d,n (M:2 T:7)
        // -- mread
        case   96: goto step_next;
        case   97: _wait();_mread(cpu->pc++);goto step_next;
        case   98: cpu->d=_gd();goto step_next;
        // -- overlapped
        case   99: goto fetch_next;
        
        //  17: rla (M:1 T:4)
        // -- overlapped
        case  100: _z80_rla(cpu);goto fetch_next;
        
        //  18: jr d (M:3 T:12)
        // -- mread
        case  101: goto step_next;
        case  102: _wait();_mread(cpu->pc++);goto step_next;
        case  103: cpu->dlatch=_gd();goto step_next;
        // -- generic
        case  104: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;goto step_next;
        case  105: goto step_next;
        case  106: goto step_next;
        case  107: goto step_next;
        case  108: goto step_next;
        // -- overlapped
        case  109: goto fetch_next;
        
        //  19: add hl,de (M:2 T:11)
        // -- generic
        case  110: _z80_add16(cpu,cpu->de);goto step_next;
        case  111: goto step_next;
        case  112: goto step_next;
        case  113: goto step_next;
        case  114: goto step_next;
        case  115: goto step_next;
        case  116: goto step_next;
        // -- overlapped
        case  117: goto fetch_next;
        
        //  1A: ld a,(de) (M:2 T:7)
        // -- mread
        case  118: goto step_next;
        case  119: _wait();_mread(cpu->de);goto step_next;
        case  120: cpu->a=_gd();cpu->wz=cpu->de+1;goto step_next;
        // -- overlapped
        case  121: goto fetch_next;
        
        //  1B: dec de (M:2 T:6)
        // -- generic
        case  122: cpu->de--;goto step_next;
        case  123: goto step_next;
        // -- overlapped
        case  124: goto fetch_next;
        
        //  1C: inc e (M:1 T:4)
        // -- overlapped
        case  125: cpu->e=_z80_inc8(cpu,cpu->e);goto fetch_next;
        
        //  1D: dec e (M:1 T:4)
        // -- overlapped
        case  126: cpu->e=_z80_dec8(cpu,cpu->e);goto fetch_next;
        
        //  1E: ld e,n (M:2 T:7)
        // -- mread
        case  127: goto step_next;
        case  128: _wait();_mread(cpu->pc++);goto step_next;
        case  129: cpu->e=_gd();goto step_next;
        // -- overlapped
        case  130: goto fetch_next;
        
        //  1F: rra (M:1 T:4)
        // -- overlapped
        case  131: _z80_rra(cpu);goto fetch_next;
        
        //  20: jr nz,d (M:3 T:12)
        // -- mread
        case  132: goto step_next;
        case  133: _wait();_mread(cpu->pc++);goto step_next;
        case  134: cpu->dlatch=_gd();if(!(_cc_nz)){_skip(5);};goto step_next;
        // -- generic
        case  135: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;goto step_next;
        case  136: goto step_next;
        case  137: goto step_next;
        case  138: goto step_next;
        case  139: goto step_next;
        // -- overlapped
        case  140: goto fetch_next;
        
        //  21: ld hl,nn (M:3 T:10)
        // -- mread
        case  141: goto step_next;
        case  142: _wait();_mread(cpu->pc++);goto step_next;
        case  143: cpu->hlx[cpu->hlx_idx].l=_gd();goto step_next;
        // -- mread
        case  144: goto step_next;
        case  145: _wait();_mread(cpu->pc++);goto step_next;
        case  146: cpu->hlx[cpu->hlx_idx].h=_gd();goto step_next;
        // -- overlapped
        case  147: goto fetch_next;
        
        //  22: ld (nn),hl (M:5 T:16)
        // -- mread
        case  148: goto step_next;
        case  149: _wait();_mread(cpu->pc++);goto step_next;
        case  150: cpu->wzl=_gd();goto step_next;
        // -- mread
        case  151: goto step_next;
        case  152: _wait();_mread(cpu->pc++);goto step_next;
        case  153: cpu->wzh=_gd();goto step_next;
        // -- mwrite
        case  154: goto step_next;
        case  155: _wait();_mwrite(cpu->wz++,cpu->hlx[cpu->hlx_idx].l);goto step_next;
        case  156: goto step_next;
        // -- mwrite
        case  157: goto step_next;
        case  158: _wait();_mwrite(cpu->wz,cpu->hlx[cpu->hlx_idx].h);goto step_next;
        case  159: goto step_next;
        // -- overlapped
        case  160: goto fetch_next;
        
        //  23: inc hl (M:2 T:6)
        // -- generic
        case  161: cpu->hlx[cpu->hlx_idx].hl++;goto step_next;
        case  162: goto step_next;
        // -- overlapped
        case  163: goto fetch_next;
        
        //  24: inc h (M:1 T:4)
        // -- overlapped
        case  164: cpu->hlx[cpu->hlx_idx].h=_z80_inc8(cpu,cpu->hlx[cpu->hlx_idx].h);goto fetch_next;
        
        //  25: dec h (M:1 T:4)
        // -- overlapped
        case  165: cpu->hlx[cpu->hlx_idx].h=_z80_dec8(cpu,cpu->hlx[cpu->hlx_idx].h);goto fetch_next;
        
        //  26: ld h,n (M:2 T:7)
        // -- mread
        case  166: goto step_next;
        case  167: _wait();_mread(cpu->pc++);goto step_next;
        case  168: cpu->hlx[cpu->hlx_idx].h=_gd();goto step_next;
        // -- overlapped
        case  169: goto fetch_next;
        
        //  27: daa (M:1 T:4)
        // -- overlapped
        case  170: _z80_daa(cpu);goto fetch_next;
        
        //  28: jr z,d (M:3 T:12)
        // -- mread
        case  171: goto step_next;
        case  172: _wait();_mread(cpu->pc++);goto step_next;
        case  173: cpu->dlatch=_gd();if(!(_cc_z)){_skip(5);};goto step_next;
        // -- generic
        case  174: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;goto step_next;
        case  175: goto step_next;
        case  176: goto step_next;
        case  177: goto step_next;
        case  178: goto step_next;
        // -- overlapped
        case  179: goto fetch_next;
        
        //  29: add hl,hl (M:2 T:11)
        // -- generic
        case  180: _z80_add16(cpu,cpu->hlx[cpu->hlx_idx].hl);goto step_next;
        case  181: goto step_next;
        case  182: goto step_next;
        case  183: goto step_next;
        case  184: goto step_next;
        case  185: goto step_next;
        case  186: goto step_next;
        // -- overlapped
        case  187: goto fetch_next;
        
        //  2A: ld hl,(nn) (M:5 T:16)
        // -- mread
        case  188: goto step_next;
        case  189: _wait();_mread(cpu->pc++);goto step_next;
        case  190: cpu->wzl=_gd();goto step_next;
        // -- mread
        case  191: goto step_next;
        case  192: _wait();_mread(cpu->pc++);goto step_next;
        case  193: cpu->wzh=_gd();goto step_next;
        // -- mread
        case  194: goto step_next;
        case  195: _wait();_mread(cpu->wz++);goto step_next;
        case  196: cpu->hlx[cpu->hlx_idx].l=_gd();goto step_next;
        // -- mread
        case  197: goto step_next;
        case  198: _wait();_mread(cpu->wz);goto step_next;
        case  199: cpu->hlx[cpu->hlx_idx].h=_gd();goto step_next;
        // -- overlapped
        case  200: goto fetch_next;
        
        //  2B: dec hl (M:2 T:6)
        // -- generic
        case  201: cpu->hlx[cpu->hlx_idx].hl--;goto step_next;
        case  202: goto step_next;
        // -- overlapped
        case  203: goto fetch_next;
        
        //  2C: inc l (M:1 T:4)
        // -- overlapped
        case  204: cpu->hlx[cpu->hlx_idx].l=_z80_inc8(cpu,cpu->hlx[cpu->hlx_idx].l);goto fetch_next;
        
        //  2D: dec l (M:1 T:4)
        // -- overlapped
        case  205: cpu->hlx[cpu->hlx_idx].l=_z80_dec8(cpu,cpu->hlx[cpu->hlx_idx].l);goto fetch_next;
        
        //  2E: ld l,n (M:2 T:7)
        // -- mread
        case  206: goto step_next;
        case  207: _wait();_mread(cpu->pc++);goto step_next;
        case  208: cpu->hlx[cpu->hlx_idx].l=_gd();goto step_next;
        // -- overlapped
        case  209: goto fetch_next;
        
        //  2F: cpl (M:1 T:4)
        // -- overlapped
        case  210: _z80_cpl(cpu);goto fetch_next;
        
        //  30: jr nc,d (M:3 T:12)
        // -- mread
        case  211: goto step_next;
        case  212: _wait();_mread(cpu->pc++);goto step_next;
        case  213: cpu->dlatch=_gd();if(!(_cc_nc)){_skip(5);};goto step_next;
        // -- generic
        case  214: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;goto step_next;
        case  215: goto step_next;
        case  216: goto step_next;
        case  217: goto step_next;
        case  218: goto step_next;
        // -- overlapped
        case  219: goto fetch_next;
        
        //  31: ld sp,nn (M:3 T:10)
        // -- mread
        case  220: goto step_next;
        case  221: _wait();_mread(cpu->pc++);goto step_next;
        case  222: cpu->spl=_gd();goto step_next;
        // -- mread
        case  223: goto step_next;
        case  224: _wait();_mread(cpu->pc++);goto step_next;
        case  225: cpu->sph=_gd();goto step_next;
        // -- overlapped
        case  226: goto fetch_next;
        
        //  32: ld (nn),a (M:4 T:13)
        // -- mread
        case  227: goto step_next;
        case  228: _wait();_mread(cpu->pc++);goto step_next;
        case  229: cpu->wzl=_gd();goto step_next;
        // -- mread
        case  230: goto step_next;
        case  231: _wait();_mread(cpu->pc++);goto step_next;
        case  232: cpu->wzh=_gd();goto step_next;
        // -- mwrite
        case  233: goto step_next;
        case  234: _wait();_mwrite(cpu->wz++,cpu->a);cpu->wzh=cpu->a;goto step_next;
        case  235: goto step_next;
        // -- overlapped
        case  236: goto fetch_next;
        
        //  33: inc sp (M:2 T:6)
        // -- generic
        case  237: cpu->sp++;goto step_next;
        case  238: goto step_next;
        // -- overlapped
        case  239: goto fetch_next;
        
        //  34: inc (hl) (M:3 T:11)
        // -- mread
        case  240: goto step_next;
        case  241: _wait();_mread(cpu->addr);goto step_next;
        case  242: cpu->dlatch=_gd();cpu->dlatch=_z80_inc8(cpu,cpu->dlatch);goto step_next;
        case  243: goto step_next;
        // -- mwrite
        case  244: goto step_next;
        case  245: _wait();_mwrite(cpu->addr,cpu->dlatch);goto step_next;
        case  246: goto step_next;
        // -- overlapped
        case  247: goto fetch_next;
        
        //  35: dec (hl) (M:3 T:11)
        // -- mread
        case  248: goto step_next;
        case  249: _wait();_mread(cpu->addr);goto step_next;
        case  250: cpu->dlatch=_gd();cpu->dlatch=_z80_dec8(cpu,cpu->dlatch);goto step_next;
        case  251: goto step_next;
        // -- mwrite
        case  252: goto step_next;
        case  253: _wait();_mwrite(cpu->addr,cpu->dlatch);goto step_next;
        case  254: goto step_next;
        // -- overlapped
        case  255: goto fetch_next;
        
        //  36: ld (hl),n (M:3 T:10)
        // -- mread
        case  256: goto step_next;
        case  257: _wait();_mread(cpu->pc++);goto step_next;
        case  258: cpu->dlatch=_gd();goto step_next;
        // -- mwrite
        case  259: goto step_next;
        case  260: _wait();_mwrite(cpu->addr,cpu->dlatch);goto step_next;
        case  261: goto step_next;
        // -- overlapped
        case  262: goto fetch_next;
        
        //  37: scf (M:1 T:4)
        // -- overlapped
        case  263: _z80_scf(cpu);goto fetch_next;
        
        //  38: jr c,d (M:3 T:12)
        // -- mread
        case  264: goto step_next;
        case  265: _wait();_mread(cpu->pc++);goto step_next;
        case  266: cpu->dlatch=_gd();if(!(_cc_c)){_skip(5);};goto step_next;
        // -- generic
        case  267: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;goto step_next;
        case  268: goto step_next;
        case  269: goto step_next;
        case  270: goto step_next;
        case  271: goto step_next;
        // -- overlapped
        case  272: goto fetch_next;
        
        //  39: add hl,sp (M:2 T:11)
        // -- generic
        case  273: _z80_add16(cpu,cpu->sp);goto step_next;
        case  274: goto step_next;
        case  275: goto step_next;
        case  276: goto step_next;
        case  277: goto step_next;
        case  278: goto step_next;
        case  279: goto step_next;
        // -- overlapped
        case  280: goto fetch_next;
        
        //  3A: ld a,(nn) (M:4 T:13)
        // -- mread
        case  281: goto step_next;
        case  282: _wait();_mread(cpu->pc++);goto step_next;
        case  283: cpu->wzl=_gd();goto step_next;
        // -- mread
        case  284: goto step_next;
        case  285: _wait();_mread(cpu->pc++);goto step_next;
        case  286: cpu->wzh=_gd();goto step_next;
        // -- mread
        case  287: goto step_next;
        case  288: _wait();_mread(cpu->wz++);goto step_next;
        case  289: cpu->a=_gd();goto step_next;
        // -- overlapped
        case  290: goto fetch_next;
        
        //  3B: dec sp (M:2 T:6)
        // -- generic
        case  291: cpu->sp--;goto step_next;
        case  292: goto step_next;
        // -- overlapped
        case  293: goto fetch_next;
        
        //  3C: inc a (M:1 T:4)
        // -- overlapped
        case  294: cpu->a=_z80_inc8(cpu,cpu->a);goto fetch_next;
        
        //  3D: dec a (M:1 T:4)
        // -- overlapped
        case  295: cpu->a=_z80_dec8(cpu,cpu->a);goto fetch_next;
        
        //  3E: ld a,n (M:2 T:7)
        // -- mread
        case  296: goto step_next;
        case  297: _wait();_mread(cpu->pc++);goto step_next;
        case  298: cpu->a=_gd();goto step_next;
        // -- overlapped
        case  299: goto fetch_next;
        
        //  3F: ccf (M:1 T:4)
        // -- overlapped
        case  300: _z80_ccf(cpu);goto fetch_next;
        
        //  40: ld b,b (M:1 T:4)
        // -- overlapped
        case  301: cpu->b=cpu->b;goto fetch_next;
        
        //  41: ld b,c (M:1 T:4)
        // -- overlapped
        case  302: cpu->b=cpu->c;goto fetch_next;
        
        //  42: ld b,d (M:1 T:4)
        // -- overlapped
        case  303: cpu->b=cpu->d;goto fetch_next;
        
        //  43: ld b,e (M:1 T:4)
        // -- overlapped
        case  304: cpu->b=cpu->e;goto fetch_next;
        
        //  44: ld b,h (M:1 T:4)
        // -- overlapped
        case  305: cpu->b=cpu->hlx[cpu->hlx_idx].h;goto fetch_next;
        
        //  45: ld b,l (M:1 T:4)
        // -- overlapped
        case  306: cpu->b=cpu->hlx[cpu->hlx_idx].l;goto fetch_next;
        
        //  46: ld b,(hl) (M:2 T:7)
        // -- mread
        case  307: goto step_next;
        case  308: _wait();_mread(cpu->addr);goto step_next;
        case  309: cpu->b=_gd();goto step_next;
        // -- overlapped
        case  310: goto fetch_next;
        
        //  47: ld b,a (M:1 T:4)
        // -- overlapped
        case  311: cpu->b=cpu->a;goto fetch_next;
        
        //  48: ld c,b (M:1 T:4)
        // -- overlapped
        case  312: cpu->c=cpu->b;goto fetch_next;
        
        //  49: ld c,c (M:1 T:4)
        // -- overlapped
        case  313: cpu->c=cpu->c;goto fetch_next;
        
        //  4A: ld c,d (M:1 T:4)
        // -- overlapped
        case  314: cpu->c=cpu->d;goto fetch_next;
        
        //  4B: ld c,e (M:1 T:4)
        // -- overlapped
        case  315: cpu->c=cpu->e;goto fetch_next;
        
        //  4C: ld c,h (M:1 T:4)
        // -- overlapped
        case  316: cpu->c=cpu->hlx[cpu->hlx_idx].h;goto fetch_next;
        
        //  4D: ld c,l (M:1 T:4)
        // -- overlapped
        case  317: cpu->c=cpu->hlx[cpu->hlx_idx].l;goto fetch_next;
        
        //  4E: ld c,(hl) (M:2 T:7)
        // -- mread
        case  318: goto step_next;
        case  319: _wait();_mread(cpu->addr);goto step_next;
        case  320: cpu->c=_gd();goto step_next;
        // -- overlapped
        case  321: goto fetch_next;
        
        //  4F: ld c,a (M:1 T:4)
        // -- overlapped
        case  322: cpu->c=cpu->a;goto fetch_next;
        
        //  50: ld d,b (M:1 T:4)
        // -- overlapped
        case  323: cpu->d=cpu->b;goto fetch_next;
        
        //  51: ld d,c (M:1 T:4)
        // -- overlapped
        case  324: cpu->d=cpu->c;goto fetch_next;
        
        //  52: ld d,d (M:1 T:4)
        // -- overlapped
        case  325: cpu->d=cpu->d;goto fetch_next;
        
        //  53: ld d,e (M:1 T:4)
        // -- overlapped
        case  326: cpu->d=cpu->e;goto fetch_next;
        
        //  54: ld d,h (M:1 T:4)
        // -- overlapped
        case  327: cpu->d=cpu->hlx[cpu->hlx_idx].h;goto fetch_next;
        
        //  55: ld d,l (M:1 T:4)
        // -- overlapped
        case  328: cpu->d=cpu->hlx[cpu->hlx_idx].l;goto fetch_next;
        
        //  56: ld d,(hl) (M:2 T:7)
        // -- mread
        case  329: goto step_next;
        case  330: _wait();_mread(cpu->addr);goto step_next;
        case  331: cpu->d=_gd();goto step_next;
        // -- overlapped
        case  332: goto fetch_next;
        
        //  57: ld d,a (M:1 T:4)
        // -- overlapped
        case  333: cpu->d=cpu->a;goto fetch_next;
        
        //  58: ld e,b (M:1 T:4)
        // -- overlapped
        case  334: cpu->e=cpu->b;goto fetch_next;
        
        //  59: ld e,c (M:1 T:4)
        // -- overlapped
        case  335: cpu->e=cpu->c;goto fetch_next;
        
        //  5A: ld e,d (M:1 T:4)
        // -- overlapped
        case  336: cpu->e=cpu->d;goto fetch_next;
        
        //  5B: ld e,e (M:1 T:4)
        // -- overlapped
        case  337: cpu->e=cpu->e;goto fetch_next;
        
        //  5C: ld e,h (M:1 T:4)
        // -- overlapped
        case  338: cpu->e=cpu->hlx[cpu->hlx_idx].h;goto fetch_next;
        
        //  5D: ld e,l (M:1 T:4)
        // -- overlapped
        case  339: cpu->e=cpu->hlx[cpu->hlx_idx].l;goto fetch_next;
        
        //  5E: ld e,(hl) (M:2 T:7)
        // -- mread
        case  340: goto step_next;
        case  341: _wait();_mread(cpu->addr);goto step_next;
        case  342: cpu->e=_gd();goto step_next;
        // -- overlapped
        case  343: goto fetch_next;
        
        //  5F: ld e,a (M:1 T:4)
        // -- overlapped
        case  344: cpu->e=cpu->a;goto fetch_next;
        
        //  60: ld h,b (M:1 T:4)
        // -- overlapped
        case  345: cpu->hlx[cpu->hlx_idx].h=cpu->b;goto fetch_next;
        
        //  61: ld h,c (M:1 T:4)
        // -- overlapped
        case  346: cpu->hlx[cpu->hlx_idx].h=cpu->c;goto fetch_next;
        
        //  62: ld h,d (M:1 T:4)
        // -- overlapped
        case  347: cpu->hlx[cpu->hlx_idx].h=cpu->d;goto fetch_next;
        
        //  63: ld h,e (M:1 T:4)
        // -- overlapped
        case  348: cpu->hlx[cpu->hlx_idx].h=cpu->e;goto fetch_next;
        
        //  64: ld h,h (M:1 T:4)
        // -- overlapped
        case  349: cpu->hlx[cpu->hlx_idx].h=cpu->hlx[cpu->hlx_idx].h;goto fetch_next;
        
        //  65: ld h,l (M:1 T:4)
        // -- overlapped
        case  350: cpu->hlx[cpu->hlx_idx].h=cpu->hlx[cpu->hlx_idx].l;goto fetch_next;
        
        //  66: ld h,(hl) (M:2 T:7)
        // -- mread
        case  351: goto step_next;
        case  352: _wait();_mread(cpu->addr);goto step_next;
        case  353: cpu->h=_gd();goto step_next;
        // -- overlapped
        case  354: goto fetch_next;
        
        //  67: ld h,a (M:1 T:4)
        // -- overlapped
        case  355: cpu->hlx[cpu->hlx_idx].h=cpu->a;goto fetch_next;
        
        //  68: ld l,b (M:1 T:4)
        // -- overlapped
        case  356: cpu->hlx[cpu->hlx_idx].l=cpu->b;goto fetch_next;
        
        //  69: ld l,c (M:1 T:4)
        // -- overlapped
        case  357: cpu->hlx[cpu->hlx_idx].l=cpu->c;goto fetch_next;
        
        //  6A: ld l,d (M:1 T:4)
        // -- overlapped
        case  358: cpu->hlx[cpu->hlx_idx].l=cpu->d;goto fetch_next;
        
        //  6B: ld l,e (M:1 T:4)
        // -- overlapped
        case  359: cpu->hlx[cpu->hlx_idx].l=cpu->e;goto fetch_next;
        
        //  6C: ld l,h (M:1 T:4)
        // -- overlapped
        case  360: cpu->hlx[cpu->hlx_idx].l=cpu->hlx[cpu->hlx_idx].h;goto fetch_next;
        
        //  6D: ld l,l (M:1 T:4)
        // -- overlapped
        case  361: cpu->hlx[cpu->hlx_idx].l=cpu->hlx[cpu->hlx_idx].l;goto fetch_next;
        
        //  6E: ld l,(hl) (M:2 T:7)
        // -- mread
        case  362: goto step_next;
        case  363: _wait();_mread(cpu->addr);goto step_next;
        case  364: cpu->l=_gd();goto step_next;
        // -- overlapped
        case  365: goto fetch_next;
        
        //  6F: ld l,a (M:1 T:4)
        // -- overlapped
        case  366: cpu->hlx[cpu->hlx_idx].l=cpu->a;goto fetch_next;
        
        //  70: ld (hl),b (M:2 T:7)
        // -- mwrite
        case  367: goto step_next;
        case  368: _wait();_mwrite(cpu->addr,cpu->b);goto step_next;
        case  369: goto step_next;
        // -- overlapped
        case  370: goto fetch_next;
        
        //  71: ld (hl),c (M:2 T:7)
        // -- mwrite
        case  371: goto step_next;
        case  372: _wait();_mwrite(cpu->addr,cpu->c);goto step_next;
        case  373: goto step_next;
        // -- overlapped
        case  374: goto fetch_next;
        
        //  72: ld (hl),d (M:2 T:7)
        // -- mwrite
        case  375: goto step_next;
        case  376: _wait();_mwrite(cpu->addr,cpu->d);goto step_next;
        case  377: goto step_next;
        // -- overlapped
        case  378: goto fetch_next;
        
        //  73: ld (hl),e (M:2 T:7)
        // -- mwrite
        case  379: goto step_next;
        case  380: _wait();_mwrite(cpu->addr,cpu->e);goto step_next;
        case  381: goto step_next;
        // -- overlapped
        case  382: goto fetch_next;
        
        //  74: ld (hl),h (M:2 T:7)
        // -- mwrite
        case  383: goto step_next;
        case  384: _wait();_mwrite(cpu->addr,cpu->h);goto step_next;
        case  385: goto step_next;
        // -- overlapped
        case  386: goto fetch_next;
        
        //  75: ld (hl),l (M:2 T:7)
        // -- mwrite
        case  387: goto step_next;
        case  388: _wait();_mwrite(cpu->addr,cpu->l);goto step_next;
        case  389: goto step_next;
        // -- overlapped
        case  390: goto fetch_next;
        
        //  76: halt (M:1 T:4)
        // -- overlapped
        case  391: pins=_z80_halt(cpu,pins);goto fetch_next;
        
        //  77: ld (hl),a (M:2 T:7)
        // -- mwrite
        case  392: goto step_next;
        case  393: _wait();_mwrite(cpu->addr,cpu->a);goto step_next;
        case  394: goto step_next;
        // -- overlapped
        case  395: goto fetch_next;
        
        //  78: ld a,b (M:1 T:4)
        // -- overlapped
        case  396: cpu->a=cpu->b;goto fetch_next;
        
        //  79: ld a,c (M:1 T:4)
        // -- overlapped
        case  397: cpu->a=cpu->c;goto fetch_next;
        
        //  7A: ld a,d (M:1 T:4)
        // -- overlapped
        case  398: cpu->a=cpu->d;goto fetch_next;
        
        //  7B: ld a,e (M:1 T:4)
        // -- overlapped
        case  399: cpu->a=cpu->e;goto fetch_next;
        
        //  7C: ld a,h (M:1 T:4)
        // -- overlapped
        case  400: cpu->a=cpu->hlx[cpu->hlx_idx].h;goto fetch_next;
        
        //  7D: ld a,l (M:1 T:4)
        // -- overlapped
        case  401: cpu->a=cpu->hlx[cpu->hlx_idx].l;goto fetch_next;
        
        //  7E: ld a,(hl) (M:2 T:7)
        // -- mread
        case  402: goto step_next;
        case  403: _wait();_mread(cpu->addr);goto step_next;
        case  404: cpu->a=_gd();goto step_next;
        // -- overlapped
        case  405: goto fetch_next;
        
        //  7F: ld a,a (M:1 T:4)
        // -- overlapped
        case  406: cpu->a=cpu->a;goto fetch_next;
        
        //  80: add b (M:1 T:4)
        // -- overlapped
        case  407: _z80_add8(cpu,cpu->b);goto fetch_next;
        
        //  81: add c (M:1 T:4)
        // -- overlapped
        case  408: _z80_add8(cpu,cpu->c);goto fetch_next;
        
        //  82: add d (M:1 T:4)
        // -- overlapped
        case  409: _z80_add8(cpu,cpu->d);goto fetch_next;
        
        //  83: add e (M:1 T:4)
        // -- overlapped
        case  410: _z80_add8(cpu,cpu->e);goto fetch_next;
        
        //  84: add h (M:1 T:4)
        // -- overlapped
        case  411: _z80_add8(cpu,cpu->hlx[cpu->hlx_idx].h);goto fetch_next;
        
        //  85: add l (M:1 T:4)
        // -- overlapped
        case  412: _z80_add8(cpu,cpu->hlx[cpu->hlx_idx].l);goto fetch_next;
        
        //  86: add (hl) (M:2 T:7)
        // -- mread
        case  413: goto step_next;
        case  414: _wait();_mread(cpu->addr);goto step_next;
        case  415: cpu->dlatch=_gd();goto step_next;
        // -- overlapped
        case  416: _z80_add8(cpu,cpu->dlatch);goto fetch_next;
        
        //  87: add a (M:1 T:4)
        // -- overlapped
        case  417: _z80_add8(cpu,cpu->a);goto fetch_next;
        
        //  88: adc b (M:1 T:4)
        // -- overlapped
        case  418: _z80_adc8(cpu,cpu->b);goto fetch_next;
        
        //  89: adc c (M:1 T:4)
        // -- overlapped
        case  419: _z80_adc8(cpu,cpu->c);goto fetch_next;
        
        //  8A: adc d (M:1 T:4)
        // -- overlapped
        case  420: _z80_adc8(cpu,cpu->d);goto fetch_next;
        
        //  8B: adc e (M:1 T:4)
        // -- overlapped
        case  421: _z80_adc8(cpu,cpu->e);goto fetch_next;
        
        //  8C: adc h (M:1 T:4)
        // -- overlapped
        case  422: _z80_adc8(cpu,cpu->hlx[cpu->hlx_idx].h);goto fetch_next;
        
        //  8D: adc l (M:1 T:4)
        // -- overlapped
        case  423: _z80_adc8(cpu,cpu->hlx[cpu->hlx_idx].l);goto fetch_next;
        
        //  8E: adc (hl) (M:2 T:7)
        // -- mread
        case  424: goto step_next;
        case  425: _wait();_mread(cpu->addr);goto step_next;
        case  426: cpu->dlatch=_gd();goto step_next;
        // -- overlapped
        case  427: _z80_adc8(cpu,cpu->dlatch);goto fetch_next;
        
        //  8F: adc a (M:1 T:4)
        // -- overlapped
        case  428: _z80_adc8(cpu,cpu->a);goto fetch_next;
        
        //  90: sub b (M:1 T:4)
        // -- overlapped
        case  429: _z80_sub8(cpu,cpu->b);goto fetch_next;
        
        //  91: sub c (M:1 T:4)
        // -- overlapped
        case  430: _z80_sub8(cpu,cpu->c);goto fetch_next;
        
        //  92: sub d (M:1 T:4)
        // -- overlapped
        case  431: _z80_sub8(cpu,cpu->d);goto fetch_next;
        
        //  93: sub e (M:1 T:4)
        // -- overlapped
        case  432: _z80_sub8(cpu,cpu->e);goto fetch_next;
        
        //  94: sub h (M:1 T:4)
        // -- overlapped
        case  433: _z80_sub8(cpu,cpu->hlx[cpu->hlx_idx].h);goto fetch_next;
        
        //  95: sub l (M:1 T:4)
        // -- overlapped
        case  434: _z80_sub8(cpu,cpu->hlx[cpu->hlx_idx].l);goto fetch_next;
        
        //  96: sub (hl) (M:2 T:7)
        // -- mread
        case  435: goto step_next;
        case  436: _wait();_mread(cpu->addr);goto step_next;
        case  437: cpu->dlatch=_gd();goto step_next;
        // -- overlapped
        case  438: _z80_sub8(cpu,cpu->dlatch);goto fetch_next;
        
        //  97: sub a (M:1 T:4)
        // -- overlapped
        case  439: _z80_sub8(cpu,cpu->a);goto fetch_next;
        
        //  98: sbc b (M:1 T:4)
        // -- overlapped
        case  440: _z80_sbc8(cpu,cpu->b);goto fetch_next;
        
        //  99: sbc c (M:1 T:4)
        // -- overlapped
        case  441: _z80_sbc8(cpu,cpu->c);goto fetch_next;
        
        //  9A: sbc d (M:1 T:4)
        // -- overlapped
        case  442: _z80_sbc8(cpu,cpu->d);goto fetch_next;
        
        //  9B: sbc e (M:1 T:4)
        // -- overlapped
        case  443: _z80_sbc8(cpu,cpu->e);goto fetch_next;
        
        //  9C: sbc h (M:1 T:4)
        // -- overlapped
        case  444: _z80_sbc8(cpu,cpu->hlx[cpu->hlx_idx].h);goto fetch_next;
        
        //  9D: sbc l (M:1 T:4)
        // -- overlapped
        case  445: _z80_sbc8(cpu,cpu->hlx[cpu->hlx_idx].l);goto fetch_next;
        
        //  9E: sbc (hl) (M:2 T:7)
        // -- mread
        case  446: goto step_next;
        case  447: _wait();_mread(cpu->addr);goto step_next;
        case  448: cpu->dlatch=_gd();goto step_next;
        // -- overlapped
        case  449: _z80_sbc8(cpu,cpu->dlatch);goto fetch_next;
        
        //  9F: sbc a (M:1 T:4)
        // -- overlapped
        case  450: _z80_sbc8(cpu,cpu->a);goto fetch_next;
        
        //  A0: and b (M:1 T:4)
        // -- overlapped
        case  451: _z80_and8(cpu,cpu->b);goto fetch_next;
        
        //  A1: and c (M:1 T:4)
        // -- overlapped
        case  452: _z80_and8(cpu,cpu->c);goto fetch_next;
        
        //  A2: and d (M:1 T:4)
        // -- overlapped
        case  453: _z80_and8(cpu,cpu->d);goto fetch_next;
        
        //  A3: and e (M:1 T:4)
        // -- overlapped
        case  454: _z80_and8(cpu,cpu->e);goto fetch_next;
        
        //  A4: and h (M:1 T:4)
        // -- overlapped
        case  455: _z80_and8(cpu,cpu->hlx[cpu->hlx_idx].h);goto fetch_next;
        
        //  A5: and l (M:1 T:4)
        // -- overlapped
        case  456: _z80_and8(cpu,cpu->hlx[cpu->hlx_idx].l);goto fetch_next;
        
        //  A6: and (hl) (M:2 T:7)
        // -- mread
        case  457: goto step_next;
        case  458: _wait();_mread(cpu->addr);goto step_next;
        case  459: cpu->dlatch=_gd();goto step_next;
        // -- overlapped
        case  460: _z80_and8(cpu,cpu->dlatch);goto fetch_next;
        
        //  A7: and a (M:1 T:4)
        // -- overlapped
        case  461: _z80_and8(cpu,cpu->a);goto fetch_next;
        
        //  A8: xor b (M:1 T:4)
        // -- overlapped
        case  462: _z80_xor8(cpu,cpu->b);goto fetch_next;
        
        //  A9: xor c (M:1 T:4)
        // -- overlapped
        case  463: _z80_xor8(cpu,cpu->c);goto fetch_next;
        
        //  AA: xor d (M:1 T:4)
        // -- overlapped
        case  464: _z80_xor8(cpu,cpu->d);goto fetch_next;
        
        //  AB: xor e (M:1 T:4)
        // -- overlapped
        case  465: _z80_xor8(cpu,cpu->e);goto fetch_next;
        
        //  AC: xor h (M:1 T:4)
        // -- overlapped
        case  466: _z80_xor8(cpu,cpu->hlx[cpu->hlx_idx].h);goto fetch_next;
        
        //  AD: xor l (M:1 T:4)
        // -- overlapped
        case  467: _z80_xor8(cpu,cpu->hlx[cpu->hlx_idx].l);goto fetch_next;
        
        //  AE: xor (hl) (M:2 T:7)
        // -- mread
        case  468: goto step_next;
        case  469: _wait();_mread(cpu->addr);goto step_next;
        case  470: cpu->dlatch=_gd();goto step_next;
        // -- overlapped
        case  471: _z80_xor8(cpu,cpu->dlatch);goto fetch_next;
        
        //  AF: xor a (M:1 T:4)
        // -- overlapped
        case  472: _z80_xor8(cpu,cpu->a);goto fetch_next;
        
        //  B0: or b (M:1 T:4)
        // -- overlapped
        case  473: _z80_or8(cpu,cpu->b);goto fetch_next;
        
        //  B1: or c (M:1 T:4)
        // -- overlapped
        case  474: _z80_or8(cpu,cpu->c);goto fetch_next;
        
        //  B2: or d (M:1 T:4)
        // -- overlapped
        case  475: _z80_or8(cpu,cpu->d);goto fetch_next;
        
        //  B3: or e (M:1 T:4)
        // -- overlapped
        case  476: _z80_or8(cpu,cpu->e);goto fetch_next;
        
        //  B4: or h (M:1 T:4)
        // -- overlapped
        case  477: _z80_or8(cpu,cpu->hlx[cpu->hlx_idx].h);goto fetch_next;
        
        //  B5: or l (M:1 T:4)
        // -- overlapped
        case  478: _z80_or8(cpu,cpu->hlx[cpu->hlx_idx].l);goto fetch_next;
        
        //  B6: or (hl) (M:2 T:7)
        // -- mread
        case  479: goto step_next;
        case  480: _wait();_mread(cpu->addr);goto step_next;
        case  481: cpu->dlatch=_gd();goto step_next;
        // -- overlapped
        case  482: _z80_or8(cpu,cpu->dlatch);goto fetch_next;
        
        //  B7: or a (M:1 T:4)
        // -- overlapped
        case  483: _z80_or8(cpu,cpu->a);goto fetch_next;
        
        //  B8: cp b (M:1 T:4)
        // -- overlapped
        case  484: _z80_cp8(cpu,cpu->b);goto fetch_next;
        
        //  B9: cp c (M:1 T:4)
        // -- overlapped
        case  485: _z80_cp8(cpu,cpu->c);goto fetch_next;
        
        //  BA: cp d (M:1 T:4)
        // -- overlapped
        case  486: _z80_cp8(cpu,cpu->d);goto fetch_next;
        
        //  BB: cp e (M:1 T:4)
        // -- overlapped
        case  487: _z80_cp8(cpu,cpu->e);goto fetch_next;
        
        //  BC: cp h (M:1 T:4)
        // -- overlapped
        case  488: _z80_cp8(cpu,cpu->hlx[cpu->hlx_idx].h);goto fetch_next;
        
        //  BD: cp l (M:1 T:4)
        // -- overlapped
        case  489: _z80_cp8(cpu,cpu->hlx[cpu->hlx_idx].l);goto fetch_next;
        
        //  BE: cp (hl) (M:2 T:7)
        // -- mread
        case  490: goto step_next;
        case  491: _wait();_mread(cpu->addr);goto step_next;
        case  492: cpu->dlatch=_gd();goto step_next;
        // -- overlapped
        case  493: _z80_cp8(cpu,cpu->dlatch);goto fetch_next;
        
        //  BF: cp a (M:1 T:4)
        // -- overlapped
        case  494: _z80_cp8(cpu,cpu->a);goto fetch_next;
        
        //  C0: ret nz (M:4 T:11)
        // -- generic
        case  495: if(!_cc_nz){_skip(6);};goto step_next;
        // -- mread
        case  496: goto step_next;
        case  497: _wait();_mread(cpu->sp++);goto step_next;
        case  498: cpu->wzl=_gd();goto step_next;
        // -- mread
        case  499: goto step_next;
        case  500: _wait();_mread(cpu->sp++);goto step_next;
        case  501: cpu->wzh=_gd();cpu->pc=cpu->wz;goto step_next;
        // -- overlapped
        case  502: goto fetch_next;
        
        //  C1: pop bc (M:3 T:10)
        // -- mread
        case  503: goto step_next;
        case  504: _wait();_mread(cpu->sp++);goto step_next;
        case  505: cpu->c=_gd();goto step_next;
        // -- mread
        case  506: goto step_next;
        case  507: _wait();_mread(cpu->sp++);goto step_next;
        case  508: cpu->b=_gd();goto step_next;
        // -- overlapped
        case  509: goto fetch_next;
        
        //  C2: jp nz,nn (M:3 T:10)
        // -- mread
        case  510: goto step_next;
        case  511: _wait();_mread(cpu->pc++);goto step_next;
        case  512: cpu->wzl=_gd();goto step_next;
        // -- mread
        case  513: goto step_next;
        case  514: _wait();_mread(cpu->pc++);goto step_next;
        case  515: cpu->wzh=_gd();if(_cc_nz){cpu->pc=cpu->wz;};goto step_next;
        // -- overlapped
        case  516: goto fetch_next;
        
        //  C3: jp nn (M:3 T:10)
        // -- mread
        case  517: goto step_next;
        case  518: _wait();_mread(cpu->pc++);goto step_next;
        case  519: cpu->wzl=_gd();goto step_next;
        // -- mread
        case  520: goto step_next;
        case  521: _wait();_mread(cpu->pc++);goto step_next;
        case  522: cpu->wzh=_gd();cpu->pc=cpu->wz;goto step_next;
        // -- overlapped
        case  523: goto fetch_next;
        
        //  C4: call nz,nn (M:6 T:17)
        // -- mread
        case  524: goto step_next;
        case  525: _wait();_mread(cpu->pc++);goto step_next;
        case  526: cpu->wzl=_gd();goto step_next;
        // -- mread
        case  527: goto step_next;
        case  528: _wait();_mread(cpu->pc++);goto step_next;
        case  529: cpu->wzh=_gd();if (!_cc_nz){_skip(7);};goto step_next;
        // -- generic
        case  530: goto step_next;
        // -- mwrite
        case  531: goto step_next;
        case  532: _wait();_mwrite(--cpu->sp,cpu->pch);goto step_next;
        case  533: goto step_next;
        // -- mwrite
        case  534: goto step_next;
        case  535: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz;goto step_next;
        case  536: goto step_next;
        // -- overlapped
        case  537: goto fetch_next;
        
        //  C5: push bc (M:4 T:11)
        // -- generic
        case  538: goto step_next;
        // -- mwrite
        case  539: goto step_next;
        case  540: _wait();_mwrite(--cpu->sp,cpu->b);goto step_next;
        case  541: goto step_next;
        // -- mwrite
        case  542: goto step_next;
        case  543: _wait();_mwrite(--cpu->sp,cpu->c);goto step_next;
        case  544: goto step_next;
        // -- overlapped
        case  545: goto fetch_next;
        
        //  C6: add n (M:2 T:7)
        // -- mread
        case  546: goto step_next;
        case  547: _wait();_mread(cpu->pc++);goto step_next;
        case  548: cpu->dlatch=_gd();goto step_next;
        // -- overlapped
        case  549: _z80_add8(cpu,cpu->dlatch);goto fetch_next;
        
        //  C7: rst 0h (M:4 T:11)
        // -- generic
        case  550: goto step_next;
        // -- mwrite
        case  551: goto step_next;
        case  552: _wait();_mwrite(--cpu->sp,cpu->pch);goto step_next;
        case  553: goto step_next;
        // -- mwrite
        case  554: goto step_next;
        case  555: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x00;cpu->pc=cpu->wz;goto step_next;
        case  556: goto step_next;
        // -- overlapped
        case  557: goto fetch_next;
        
        //  C8: ret z (M:4 T:11)
        // -- generic
        case  558: if(!_cc_z){_skip(6);};goto step_next;
        // -- mread
        case  559: goto step_next;
        case  560: _wait();_mread(cpu->sp++);goto step_next;
        case  561: cpu->wzl=_gd();goto step_next;
        // -- mread
        case  562: goto step_next;
        case  563: _wait();_mread(cpu->sp++);goto step_next;
        case  564: cpu->wzh=_gd();cpu->pc=cpu->wz;goto step_next;
        // -- overlapped
        case  565: goto fetch_next;
        
        //  C9: ret (M:3 T:10)
        // -- mread
        case  566: goto step_next;
        case  567: _wait();_mread(cpu->sp++);goto step_next;
        case  568: cpu->wzl=_gd();goto step_next;
        // -- mread
        case  569: goto step_next;
        case  570: _wait();_mread(cpu->sp++);goto step_next;
        case  571: cpu->wzh=_gd();cpu->pc=cpu->wz;goto step_next;
        // -- overlapped
        case  572: goto fetch_next;
        
        //  CA: jp z,nn (M:3 T:10)
        // -- mread
        case  573: goto step_next;
        case  574: _wait();_mread(cpu->pc++);goto step_next;
        case  575: cpu->wzl=_gd();goto step_next;
        // -- mread
        case  576: goto step_next;
        case  577: _wait();_mread(cpu->pc++);goto step_next;
        case  578: cpu->wzh=_gd();if(_cc_z){cpu->pc=cpu->wz;};goto step_next;
        // -- overlapped
        case  579: goto fetch_next;
        
        //  CB: cb prefix (M:1 T:4)
        // -- overlapped
        case  580: _fetch_cb();goto step_next;
        
        //  CC: call z,nn (M:6 T:17)
        // -- mread
        case  581: goto step_next;
        case  582: _wait();_mread(cpu->pc++);goto step_next;
        case  583: cpu->wzl=_gd();goto step_next;
        // -- mread
        case  584: goto step_next;
        case  585: _wait();_mread(cpu->pc++);goto step_next;
        case  586: cpu->wzh=_gd();if (!_cc_z){_skip(7);};goto step_next;
        // -- generic
        case  587: goto step_next;
        // -- mwrite
        case  588: goto step_next;
        case  589: _wait();_mwrite(--cpu->sp,cpu->pch);goto step_next;
        case  590: goto step_next;
        // -- mwrite
        case  591: goto step_next;
        case  592: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz;goto step_next;
        case  593: goto step_next;
        // -- overlapped
        case  594: goto fetch_next;
        
        //  CD: call nn (M:5 T:17)
        // -- mread
        case  595: goto step_next;
        case  596: _wait();_mread(cpu->pc++);goto step_next;
        case  597: cpu->wzl=_gd();goto step_next;
        // -- mread
        case  598: goto step_next;
        case  599: _wait();_mread(cpu->pc++);goto step_next;
        case  600: cpu->wzh=_gd();goto step_next;
        case  601: goto step_next;
        // -- mwrite
        case  602: goto step_next;
        case  603: _wait();_mwrite(--cpu->sp,cpu->pch);goto step_next;
        case  604: goto step_next;
        // -- mwrite
        case  605: goto step_next;
        case  606: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz;goto step_next;
        case  607: goto step_next;
        // -- overlapped
        case  608: goto fetch_next;
        
        //  CE: adc n (M:2 T:7)
        // -- mread
        case  609: goto step_next;
        case  610: _wait();_mread(cpu->pc++);goto step_next;
        case  611: cpu->dlatch=_gd();goto step_next;
        // -- overlapped
        case  612: _z80_adc8(cpu,cpu->dlatch);goto fetch_next;
        
        //  CF: rst 8h (M:4 T:11)
        // -- generic
        case  613: goto step_next;
        // -- mwrite
        case  614: goto step_next;
        case  615: _wait();_mwrite(--cpu->sp,cpu->pch);goto step_next;
        case  616: goto step_next;
        // -- mwrite
        case  617: goto step_next;
        case  618: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x08;cpu->pc=cpu->wz;goto step_next;
        case  619: goto step_next;
        // -- overlapped
        case  620: goto fetch_next;
        
        //  D0: ret nc (M:4 T:11)
        // -- generic
        case  621: if(!_cc_nc){_skip(6);};goto step_next;
        // -- mread
        case  622: goto step_next;
        case  623: _wait();_mread(cpu->sp++);goto step_next;
        case  624: cpu->wzl=_gd();goto step_next;
        // -- mread
        case  625: goto step_next;
        case  626: _wait();_mread(cpu->sp++);goto step_next;
        case  627: cpu->wzh=_gd();cpu->pc=cpu->wz;goto step_next;
        // -- overlapped
        case  628: goto fetch_next;
        
        //  D1: pop de (M:3 T:10)
        // -- mread
        case  629: goto step_next;
        case  630: _wait();_mread(cpu->sp++);goto step_next;
        case  631: cpu->e=_gd();goto step_next;
        // -- mread
        case  632: goto step_next;
        case  633: _wait();_mread(cpu->sp++);goto step_next;
        case  634: cpu->d=_gd();goto step_next;
        // -- overlapped
        case  635: goto fetch_next;
        
        //  D2: jp nc,nn (M:3 T:10)
        // -- mread
        case  636: goto step_next;
        case  637: _wait();_mread(cpu->pc++);goto step_next;
        case  638: cpu->wzl=_gd();goto step_next;
        // -- mread
        case  639: goto step_next;
        case  640: _wait();_mread(cpu->pc++);goto step_next;
        case  641: cpu->wzh=_gd();if(_cc_nc){cpu->pc=cpu->wz;};goto step_next;
        // -- overlapped
        case  642: goto fetch_next;
        
        //  D3: out (n),a (M:3 T:11)
        // -- mread
        case  643: goto step_next;
        case  644: _wait();_mread(cpu->pc++);goto step_next;
        case  645: cpu->wzl=_gd();cpu->wzh=cpu->a;goto step_next;
        // -- iowrite
        case  646: goto step_next;
        case  647: _iowrite(cpu->wz,cpu->a);goto step_next;
        case  648: _wait();cpu->wzl++;goto step_next;
        case  649: goto step_next;
        // -- overlapped
        case  650: goto fetch_next;
        
        //  D4: call nc,nn (M:6 T:17)
        // -- mread
        case  651: goto step_next;
        case  652: _wait();_mread(cpu->pc++);goto step_next;
        case  653: cpu->wzl=_gd();goto step_next;
        // -- mread
        case  654: goto step_next;
        case  655: _wait();_mread(cpu->pc++);goto step_next;
        case  656: cpu->wzh=_gd();if (!_cc_nc){_skip(7);};goto step_next;
        // -- generic
        case  657: goto step_next;
        // -- mwrite
        case  658: goto step_next;
        case  659: _wait();_mwrite(--cpu->sp,cpu->pch);goto step_next;
        case  660: goto step_next;
        // -- mwrite
        case  661: goto step_next;
        case  662: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz;goto step_next;
        case  663: goto step_next;
        // -- overlapped
        case  664: goto fetch_next;
        
        //  D5: push de (M:4 T:11)
        // -- generic
        case  665: goto step_next;
        // -- mwrite
        case  666: goto step_next;
        case  667: _wait();_mwrite(--cpu->sp,cpu->d);goto step_next;
        case  668: goto step_next;
        // -- mwrite
        case  669: goto step_next;
        case  670: _wait();_mwrite(--cpu->sp,cpu->e);goto step_next;
        case  671: goto step_next;
        // -- overlapped
        case  672: goto fetch_next;
        
        //  D6: sub n (M:2 T:7)
        // -- mread
        case  673: goto step_next;
        case  674: _wait();_mread(cpu->pc++);goto step_next;
        case  675: cpu->dlatch=_gd();goto step_next;
        // -- overlapped
        case  676: _z80_sub8(cpu,cpu->dlatch);goto fetch_next;
        
        //  D7: rst 10h (M:4 T:11)
        // -- generic
        case  677: goto step_next;
        // -- mwrite
        case  678: goto step_next;
        case  679: _wait();_mwrite(--cpu->sp,cpu->pch);goto step_next;
        case  680: goto step_next;
        // -- mwrite
        case  681: goto step_next;
        case  682: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x10;cpu->pc=cpu->wz;goto step_next;
        case  683: goto step_next;
        // -- overlapped
        case  684: goto fetch_next;
        
        //  D8: ret c (M:4 T:11)
        // -- generic
        case  685: if(!_cc_c){_skip(6);};goto step_next;
        // -- mread
        case  686: goto step_next;
        case  687: _wait();_mread(cpu->sp++);goto step_next;
        case  688: cpu->wzl=_gd();goto step_next;
        // -- mread
        case  689: goto step_next;
        case  690: _wait();_mread(cpu->sp++);goto step_next;
        case  691: cpu->wzh=_gd();cpu->pc=cpu->wz;goto step_next;
        // -- overlapped
        case  692: goto fetch_next;
        
        //  D9: exx (M:1 T:4)
        // -- overlapped
        case  693: _z80_exx(cpu);goto fetch_next;
        
        //  DA: jp c,nn (M:3 T:10)
        // -- mread
        case  694: goto step_next;
        case  695: _wait();_mread(cpu->pc++);goto step_next;
        case  696: cpu->wzl=_gd();goto step_next;
        // -- mread
        case  697: goto step_next;
        case  698: _wait();_mread(cpu->pc++);goto step_next;
        case  699: cpu->wzh=_gd();if(_cc_c){cpu->pc=cpu->wz;};goto step_next;
        // -- overlapped
        case  700: goto fetch_next;
        
        //  DB: in a,(n) (M:3 T:11)
        // -- mread
        case  701: goto step_next;
        case  702: _wait();_mread(cpu->pc++);goto step_next;
        case  703: cpu->wzl=_gd();cpu->wzh=cpu->a;goto step_next;
        // -- ioread
        case  704: goto step_next;
        case  705: goto step_next;
        case  706: _wait();_ioread(cpu->wz++);goto step_next;
        case  707: cpu->dlatch=_gd();goto step_next;
        // -- overlapped
        case  708: cpu->a=cpu->dlatch;goto fetch_next;
        
        //  DC: call c,nn (M:6 T:17)
        // -- mread
        case  709: goto step_next;
        case  710: _wait();_mread(cpu->pc++);goto step_next;
        case  711: cpu->wzl=_gd();goto step_next;
        // -- mread
        case  712: goto step_next;
        case  713: _wait();_mread(cpu->pc++);goto step_next;
        case  714: cpu->wzh=_gd();if (!_cc_c){_skip(7);};goto step_next;
        // -- generic
        case  715: goto step_next;
        // -- mwrite
        case  716: goto step_next;
        case  717: _wait();_mwrite(--cpu->sp,cpu->pch);goto step_next;
        case  718: goto step_next;
        // -- mwrite
        case  719: goto step_next;
        case  720: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz;goto step_next;
        case  721: goto step_next;
        // -- overlapped
        case  722: goto fetch_next;
        
        //  DD: dd prefix (M:1 T:4)
        // -- overlapped
        case  723: _fetch_dd();goto step_next;
        
        //  DE: sbc n (M:2 T:7)
        // -- mread
        case  724: goto step_next;
        case  725: _wait();_mread(cpu->pc++);goto step_next;
        case  726: cpu->dlatch=_gd();goto step_next;
        // -- overlapped
        case  727: _z80_sbc8(cpu,cpu->dlatch);goto fetch_next;
        
        //  DF: rst 18h (M:4 T:11)
        // -- generic
        case  728: goto step_next;
        // -- mwrite
        case  729: goto step_next;
        case  730: _wait();_mwrite(--cpu->sp,cpu->pch);goto step_next;
        case  731: goto step_next;
        // -- mwrite
        case  732: goto step_next;
        case  733: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x18;cpu->pc=cpu->wz;goto step_next;
        case  734: goto step_next;
        // -- overlapped
        case  735: goto fetch_next;
        
        //  E0: ret po (M:4 T:11)
        // -- generic
        case  736: if(!_cc_po){_skip(6);};goto step_next;
        // -- mread
        case  737: goto step_next;
        case  738: _wait();_mread(cpu->sp++);goto step_next;
        case  739: cpu->wzl=_gd();goto step_next;
        // -- mread
        case  740: goto step_next;
        case  741: _wait();_mread(cpu->sp++);goto step_next;
        case  742: cpu->wzh=_gd();cpu->pc=cpu->wz;goto step_next;
        // -- overlapped
        case  743: goto fetch_next;
        
        //  E1: pop hl (M:3 T:10)
        // -- mread
        case  744: goto step_next;
        case  745: _wait();_mread(cpu->sp++);goto step_next;
        case  746: cpu->hlx[cpu->hlx_idx].l=_gd();goto step_next;
        // -- mread
        case  747: goto step_next;
        case  748: _wait();_mread(cpu->sp++);goto step_next;
        case  749: cpu->hlx[cpu->hlx_idx].h=_gd();goto step_next;
        // -- overlapped
        case  750: goto fetch_next;
        
        //  E2: jp po,nn (M:3 T:10)
        // -- mread
        case  751: goto step_next;
        case  752: _wait();_mread(cpu->pc++);goto step_next;
        case  753: cpu->wzl=_gd();goto step_next;
        // -- mread
        case  754: goto step_next;
        case  755: _wait();_mread(cpu->pc++);goto step_next;
        case  756: cpu->wzh=_gd();if(_cc_po){cpu->pc=cpu->wz;};goto step_next;
        // -- overlapped
        case  757: goto fetch_next;
        
        //  E3: ex (sp),hl (M:5 T:19)
        // -- mread
        case  758: goto step_next;
        case  759: _wait();_mread(cpu->sp);goto step_next;
        case  760: cpu->wzl=_gd();goto step_next;
        // -- mread
        case  761: goto step_next;
        case  762: _wait();_mread(cpu->sp+1);goto step_next;
        case  763: cpu->wzh=_gd();goto step_next;
        case  764: goto step_next;
        // -- mwrite
        case  765: goto step_next;
        case  766: _wait();_mwrite(cpu->sp+1,cpu->hlx[cpu->hlx_idx].h);goto step_next;
        case  767: goto step_next;
        // -- mwrite
        case  768: goto step_next;
        case  769: _wait();_mwrite(cpu->sp,cpu->hlx[cpu->hlx_idx].l);cpu->hlx[cpu->hlx_idx].hl=cpu->wz;goto step_next;
        case  770: goto step_next;
        case  771: goto step_next;
        case  772: goto step_next;
        // -- overlapped
        case  773: goto fetch_next;
        
        //  E4: call po,nn (M:6 T:17)
        // -- mread
        case  774: goto step_next;
        case  775: _wait();_mread(cpu->pc++);goto step_next;
        case  776: cpu->wzl=_gd();goto step_next;
        // -- mread
        case  777: goto step_next;
        case  778: _wait();_mread(cpu->pc++);goto step_next;
        case  779: cpu->wzh=_gd();if (!_cc_po){_skip(7);};goto step_next;
        // -- generic
        case  780: goto step_next;
        // -- mwrite
        case  781: goto step_next;
        case  782: _wait();_mwrite(--cpu->sp,cpu->pch);goto step_next;
        case  783: goto step_next;
        // -- mwrite
        case  784: goto step_next;
        case  785: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz;goto step_next;
        case  786: goto step_next;
        // -- overlapped
        case  787: goto fetch_next;
        
        //  E5: push hl (M:4 T:11)
        // -- generic
        case  788: goto step_next;
        // -- mwrite
        case  789: goto step_next;
        case  790: _wait();_mwrite(--cpu->sp,cpu->hlx[cpu->hlx_idx].h);goto step_next;
        case  791: goto step_next;
        // -- mwrite
        case  792: goto step_next;
        case  793: _wait();_mwrite(--cpu->sp,cpu->hlx[cpu->hlx_idx].l);goto step_next;
        case  794: goto step_next;
        // -- overlapped
        case  795: goto fetch_next;
        
        //  E6: and n (M:2 T:7)
        // -- mread
        case  796: goto step_next;
        case  797: _wait();_mread(cpu->pc++);goto step_next;
        case  798: cpu->dlatch=_gd();goto step_next;
        // -- overlapped
        case  799: _z80_and8(cpu,cpu->dlatch);goto fetch_next;
        
        //  E7: rst 20h (M:4 T:11)
        // -- generic
        case  800: goto step_next;
        // -- mwrite
        case  801: goto step_next;
        case  802: _wait();_mwrite(--cpu->sp,cpu->pch);goto step_next;
        case  803: goto step_next;
        // -- mwrite
        case  804: goto step_next;
        case  805: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x20;cpu->pc=cpu->wz;goto step_next;
        case  806: goto step_next;
        // -- overlapped
        case  807: goto fetch_next;
        
        //  E8: ret pe (M:4 T:11)
        // -- generic
        case  808: if(!_cc_pe){_skip(6);};goto step_next;
        // -- mread
        case  809: goto step_next;
        case  810: _wait();_mread(cpu->sp++);goto step_next;
        case  811: cpu->wzl=_gd();goto step_next;
        // -- mread
        case  812: goto step_next;
        case  813: _wait();_mread(cpu->sp++);goto step_next;
        case  814: cpu->wzh=_gd();cpu->pc=cpu->wz;goto step_next;
        // -- overlapped
        case  815: goto fetch_next;
        
        //  E9: jp hl (M:1 T:4)
        // -- overlapped
        case  816: cpu->pc=cpu->hlx[cpu->hlx_idx].hl;goto fetch_next;
        
        //  EA: jp pe,nn (M:3 T:10)
        // -- mread
        case  817: goto step_next;
        case  818: _wait();_mread(cpu->pc++);goto step_next;
        case  819: cpu->wzl=_gd();goto step_next;
        // -- mread
        case  820: goto step_next;
        case  821: _wait();_mread(cpu->pc++);goto step_next;
        case  822: cpu->wzh=_gd();if(_cc_pe){cpu->pc=cpu->wz;};goto step_next;
        // -- overlapped
        case  823: goto fetch_next;
        
        //  EB: ex de,hl (M:1 T:4)
        // -- overlapped
        case  824: _z80_ex_de_hl(cpu);goto fetch_next;
        
        //  EC: call pe,nn (M:6 T:17)
        // -- mread
        case  825: goto step_next;
        case  826: _wait();_mread(cpu->pc++);goto step_next;
        case  827: cpu->wzl=_gd();goto step_next;
        // -- mread
        case  828: goto step_next;
        case  829: _wait();_mread(cpu->pc++);goto step_next;
        case  830: cpu->wzh=_gd();if (!_cc_pe){_skip(7);};goto step_next;
        // -- generic
        case  831: goto step_next;
        // -- mwrite
        case  832: goto step_next;
        case  833: _wait();_mwrite(--cpu->sp,cpu->pch);goto step_next;
        case  834: goto step_next;
        // -- mwrite
        case  835: goto step_next;
        case  836: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz;goto step_next;
        case  837: goto step_next;
        // -- overlapped
        case  838: goto fetch_next;
        
        //  ED: ed prefix (M:1 T:4)
        // -- overlapped
        case  839: _fetch_ed();goto step_next;
        
        //  EE: xor n (M:2 T:7)
        // -- mread
        case  840: goto step_next;
        case  841: _wait();_mread(cpu->pc++);goto step_next;
        case  842: cpu->dlatch=_gd();goto step_next;
        // -- overlapped
        case  843: _z80_xor8(cpu,cpu->dlatch);goto fetch_next;
        
        //  EF: rst 28h (M:4 T:11)
        // -- generic
        case  844: goto step_next;
        // -- mwrite
        case  845: goto step_next;
        case  846: _wait();_mwrite(--cpu->sp,cpu->pch);goto step_next;
        case  847: goto step_next;
        // -- mwrite
        case  848: goto step_next;
        case  849: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x28;cpu->pc=cpu->wz;goto step_next;
        case  850: goto step_next;
        // -- overlapped
        case  851: goto fetch_next;
        
        //  F0: ret p (M:4 T:11)
        // -- generic
        case  852: if(!_cc_p){_skip(6);};goto step_next;
        // -- mread
        case  853: goto step_next;
        case  854: _wait();_mread(cpu->sp++);goto step_next;
        case  855: cpu->wzl=_gd();goto step_next;
        // -- mread
        case  856: goto step_next;
        case  857: _wait();_mread(cpu->sp++);goto step_next;
        case  858: cpu->wzh=_gd();cpu->pc=cpu->wz;goto step_next;
        // -- overlapped
        case  859: goto fetch_next;
        
        //  F1: pop af (M:3 T:10)
        // -- mread
        case  860: goto step_next;
        case  861: _wait();_mread(cpu->sp++);goto step_next;
        case  862: cpu->f=_gd();goto step_next;
        // -- mread
        case  863: goto step_next;
        case  864: _wait();_mread(cpu->sp++);goto step_next;
        case  865: cpu->a=_gd();goto step_next;
        // -- overlapped
        case  866: goto fetch_next;
        
        //  F2: jp p,nn (M:3 T:10)
        // -- mread
        case  867: goto step_next;
        case  868: _wait();_mread(cpu->pc++);goto step_next;
        case  869: cpu->wzl=_gd();goto step_next;
        // -- mread
        case  870: goto step_next;
        case  871: _wait();_mread(cpu->pc++);goto step_next;
        case  872: cpu->wzh=_gd();if(_cc_p){cpu->pc=cpu->wz;};goto step_next;
        // -- overlapped
        case  873: goto fetch_next;
        
        //  F3: di (M:1 T:4)
        // -- overlapped
        case  874: cpu->iff1=cpu->iff2=false;goto fetch_next;
        
        //  F4: call p,nn (M:6 T:17)
        // -- mread
        case  875: goto step_next;
        case  876: _wait();_mread(cpu->pc++);goto step_next;
        case  877: cpu->wzl=_gd();goto step_next;
        // -- mread
        case  878: goto step_next;
        case  879: _wait();_mread(cpu->pc++);goto step_next;
        case  880: cpu->wzh=_gd();if (!_cc_p){_skip(7);};goto step_next;
        // -- generic
        case  881: goto step_next;
        // -- mwrite
        case  882: goto step_next;
        case  883: _wait();_mwrite(--cpu->sp,cpu->pch);goto step_next;
        case  884: goto step_next;
        // -- mwrite
        case  885: goto step_next;
        case  886: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz;goto step_next;
        case  887: goto step_next;
        // -- overlapped
        case  888: goto fetch_next;
        
        //  F5: push af (M:4 T:11)
        // -- generic
        case  889: goto step_next;
        // -- mwrite
        case  890: goto step_next;
        case  891: _wait();_mwrite(--cpu->sp,cpu->a);goto step_next;
        case  892: goto step_next;
        // -- mwrite
        case  893: goto step_next;
        case  894: _wait();_mwrite(--cpu->sp,cpu->f);goto step_next;
        case  895: goto step_next;
        // -- overlapped
        case  896: goto fetch_next;
        
        //  F6: or n (M:2 T:7)
        // -- mread
        case  897: goto step_next;
        case  898: _wait();_mread(cpu->pc++);goto step_next;
        case  899: cpu->dlatch=_gd();goto step_next;
        // -- overlapped
        case  900: _z80_or8(cpu,cpu->dlatch);goto fetch_next;
        
        //  F7: rst 30h (M:4 T:11)
        // -- generic
        case  901: goto step_next;
        // -- mwrite
        case  902: goto step_next;
        case  903: _wait();_mwrite(--cpu->sp,cpu->pch);goto step_next;
        case  904: goto step_next;
        // -- mwrite
        case  905: goto step_next;
        case  906: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x30;cpu->pc=cpu->wz;goto step_next;
        case  907: goto step_next;
        // -- overlapped
        case  908: goto fetch_next;
        
        //  F8: ret m (M:4 T:11)
        // -- generic
        case  909: if(!_cc_m){_skip(6);};goto step_next;
        // -- mread
        case  910: goto step_next;
        case  911: _wait();_mread(cpu->sp++);goto step_next;
        case  912: cpu->wzl=_gd();goto step_next;
        // -- mread
        case  913: goto step_next;
        case  914: _wait();_mread(cpu->sp++);goto step_next;
        case  915: cpu->wzh=_gd();cpu->pc=cpu->wz;goto step_next;
        // -- overlapped
        case  916: goto fetch_next;
        
        //  F9: ld sp,hl (M:2 T:6)
        // -- generic
        case  917: cpu->sp=cpu->hlx[cpu->hlx_idx].hl;goto step_next;
        case  918: goto step_next;
        // -- overlapped
        case  919: goto fetch_next;
        
        //  FA: jp m,nn (M:3 T:10)
        // -- mread
        case  920: goto step_next;
        case  921: _wait();_mread(cpu->pc++);goto step_next;
        case  922: cpu->wzl=_gd();goto step_next;
        // -- mread
        case  923: goto step_next;
        case  924: _wait();_mread(cpu->pc++);goto step_next;
        case  925: cpu->wzh=_gd();if(_cc_m){cpu->pc=cpu->wz;};goto step_next;
        // -- overlapped
        case  926: goto fetch_next;
        
        //  FB: ei (M:1 T:4)
        // -- overlapped
        case  927: cpu->iff1=cpu->iff2=false;pins=_z80_fetch(cpu,pins);cpu->iff1=cpu->iff2=true;goto step_next;
        
        //  FC: call m,nn (M:6 T:17)
        // -- mread
        case  928: goto step_next;
        case  929: _wait();_mread(cpu->pc++);goto step_next;
        case  930: cpu->wzl=_gd();goto step_next;
        // -- mread
        case  931: goto step_next;
        case  932: _wait();_mread(cpu->pc++);goto step_next;
        case  933: cpu->wzh=_gd();if (!_cc_m){_skip(7);};goto step_next;
        // -- generic
        case  934: goto step_next;
        // -- mwrite
        case  935: goto step_next;
        case  936: _wait();_mwrite(--cpu->sp,cpu->pch);goto step_next;
        case  937: goto step_next;
        // -- mwrite
        case  938: goto step_next;
        case  939: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz;goto step_next;
        case  940: goto step_next;
        // -- overlapped
        case  941: goto fetch_next;
        
        //  FD: fd prefix (M:1 T:4)
        // -- overlapped
        case  942: _fetch_fd();goto step_next;
        
        //  FE: cp n (M:2 T:7)
        // -- mread
        case  943: goto step_next;
        case  944: _wait();_mread(cpu->pc++);goto step_next;
        case  945: cpu->dlatch=_gd();goto step_next;
        // -- overlapped
        case  946: _z80_cp8(cpu,cpu->dlatch);goto fetch_next;
        
        //  FF: rst 38h (M:4 T:11)
        // -- generic
        case  947: goto step_next;
        // -- mwrite
        case  948: goto step_next;
        case  949: _wait();_mwrite(--cpu->sp,cpu->pch);goto step_next;
        case  950: goto step_next;
        // -- mwrite
        case  951: goto step_next;
        case  952: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x38;cpu->pc=cpu->wz;goto step_next;
        case  953: goto step_next;
        // -- overlapped
        case  954: goto fetch_next;
        
        // ED 00: ed nop (M:1 T:4)
        // -- overlapped
        case  955: goto fetch_next;
        
        // ED 40: in b,(c) (M:2 T:8)
        // -- ioread
        case  956: goto step_next;
        case  957: goto step_next;
        case  958: _wait();_ioread(cpu->bc);goto step_next;
        case  959: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;goto step_next;
        // -- overlapped
        case  960: cpu->b=_z80_in(cpu,cpu->dlatch);goto fetch_next;
        
        // ED 41: out (c),b (M:2 T:8)
        // -- iowrite
        case  961: goto step_next;
        case  962: _iowrite(cpu->bc,cpu->b);goto step_next;
        case  963: _wait();cpu->wz=cpu->bc+1;goto step_next;
        case  964: goto step_next;
        // -- overlapped
        case  965: goto fetch_next;
        
        // ED 42: sbc hl,bc (M:2 T:11)
        // -- generic
        case  966: _z80_sbc16(cpu,cpu->bc);goto step_next;
        case  967: goto step_next;
        case  968: goto step_next;
        case  969: goto step_next;
        case  970: goto step_next;
        case  971: goto step_next;
        case  972: goto step_next;
        // -- overlapped
        case  973: goto fetch_next;
        
        // ED 43: ld (nn),bc (M:5 T:16)
        // -- mread
        case  974: goto step_next;
        case  975: _wait();_mread(cpu->pc++);goto step_next;
        case  976: cpu->wzl=_gd();goto step_next;
        // -- mread
        case  977: goto step_next;
        case  978: _wait();_mread(cpu->pc++);goto step_next;
        case  979: cpu->wzh=_gd();goto step_next;
        // -- mwrite
        case  980: goto step_next;
        case  981: _wait();_mwrite(cpu->wz++,cpu->c);goto step_next;
        case  982: goto step_next;
        // -- mwrite
        case  983: goto step_next;
        case  984: _wait();_mwrite(cpu->wz,cpu->b);goto step_next;
        case  985: goto step_next;
        // -- overlapped
        case  986: goto fetch_next;
        
        // ED 44: neg (M:1 T:4)
        // -- overlapped
        case  987: _z80_neg8(cpu);goto fetch_next;
        
        // ED 45: reti/retn (M:3 T:10)
        // -- mread
        case  988: goto step_next;
        case  989: _wait();_mread(cpu->sp++);goto step_next;
        case  990: cpu->wzl=_gd();pins|=Z80_RETI;goto step_next;
        // -- mread
        case  991: goto step_next;
        case  992: _wait();_mread(cpu->sp++);goto step_next;
        case  993: cpu->wzh=_gd();cpu->pc=cpu->wz;goto step_next;
        // -- overlapped
        case  994: pins=_z80_fetch(cpu,pins);cpu->iff1=cpu->iff2;goto step_next;
        
        // ED 46: im 0 (M:1 T:4)
        // -- overlapped
        case  995: cpu->im=0;goto fetch_next;
        
        // ED 47: ld i,a (M:2 T:5)
        // -- generic
        case  996: goto step_next;
        // -- overlapped
        case  997: cpu->i=cpu->a;goto fetch_next;
        
        // ED 48: in c,(c) (M:2 T:8)
        // -- ioread
        case  998: goto step_next;
        case  999: goto step_next;
        case 1000: _wait();_ioread(cpu->bc);goto step_next;
        case 1001: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;goto step_next;
        // -- overlapped
        case 1002: cpu->c=_z80_in(cpu,cpu->dlatch);goto fetch_next;
        
        // ED 49: out (c),c (M:2 T:8)
        // -- iowrite
        case 1003: goto step_next;
        case 1004: _iowrite(cpu->bc,cpu->c);goto step_next;
        case 1005: _wait();cpu->wz=cpu->bc+1;goto step_next;
        case 1006: goto step_next;
        // -- overlapped
        case 1007: goto fetch_next;
        
        // ED 4A: adc hl,bc (M:2 T:11)
        // -- generic
        case 1008: _z80_adc16(cpu,cpu->bc);goto step_next;
        case 1009: goto step_next;
        case 1010: goto step_next;
        case 1011: goto step_next;
        case 1012: goto step_next;
        case 1013: goto step_next;
        case 1014: goto step_next;
        // -- overlapped
        case 1015: goto fetch_next;
        
        // ED 4B: ld bc,(nn) (M:5 T:16)
        // -- mread
        case 1016: goto step_next;
        case 1017: _wait();_mread(cpu->pc++);goto step_next;
        case 1018: cpu->wzl=_gd();goto step_next;
        // -- mread
        case 1019: goto step_next;
        case 1020: _wait();_mread(cpu->pc++);goto step_next;
        case 1021: cpu->wzh=_gd();goto step_next;
        // -- mread
        case 1022: goto step_next;
        case 1023: _wait();_mread(cpu->wz++);goto step_next;
        case 1024: cpu->c=_gd();goto step_next;
        // -- mread
        case 1025: goto step_next;
        case 1026: _wait();_mread(cpu->wz);goto step_next;
        case 1027: cpu->b=_gd();goto step_next;
        // -- overlapped
        case 1028: goto fetch_next;
        
        // ED 4E: im 0 (M:1 T:4)
        // -- overlapped
        case 1029: cpu->im=0;goto fetch_next;
        
        // ED 4F: ld r,a (M:2 T:5)
        // -- generic
        case 1030: goto step_next;
        // -- overlapped
        case 1031: cpu->r=cpu->a;goto fetch_next;
        
        // ED 50: in d,(c) (M:2 T:8)
        // -- ioread
        case 1032: goto step_next;
        case 1033: goto step_next;
        case 1034: _wait();_ioread(cpu->bc);goto step_next;
        case 1035: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;goto step_next;
        // -- overlapped
        case 1036: cpu->d=_z80_in(cpu,cpu->dlatch);goto fetch_next;
        
        // ED 51: out (c),d (M:2 T:8)
        // -- iowrite
        case 1037: goto step_next;
        case 1038: _iowrite(cpu->bc,cpu->d);goto step_next;
        case 1039: _wait();cpu->wz=cpu->bc+1;goto step_next;
        case 1040: goto step_next;
        // -- overlapped
        case 1041: goto fetch_next;
        
        // ED 52: sbc hl,de (M:2 T:11)
        // -- generic
        case 1042: _z80_sbc16(cpu,cpu->de);goto step_next;
        case 1043: goto step_next;
        case 1044: goto step_next;
        case 1045: goto step_next;
        case 1046: goto step_next;
        case 1047: goto step_next;
        case 1048: goto step_next;
        // -- overlapped
        case 1049: goto fetch_next;
        
        // ED 53: ld (nn),de (M:5 T:16)
        // -- mread
        case 1050: goto step_next;
        case 1051: _wait();_mread(cpu->pc++);goto step_next;
        case 1052: cpu->wzl=_gd();goto step_next;
        // -- mread
        case 1053: goto step_next;
        case 1054: _wait();_mread(cpu->pc++);goto step_next;
        case 1055: cpu->wzh=_gd();goto step_next;
        // -- mwrite
        case 1056: goto step_next;
        case 1057: _wait();_mwrite(cpu->wz++,cpu->e);goto step_next;
        case 1058: goto step_next;
        // -- mwrite
        case 1059: goto step_next;
        case 1060: _wait();_mwrite(cpu->wz,cpu->d);goto step_next;
        case 1061: goto step_next;
        // -- overlapped
        case 1062: goto fetch_next;
        
        // ED 56: im 1 (M:1 T:4)
        // -- overlapped
        case 1063: cpu->im=1;goto fetch_next;
        
        // ED 57: ld a,i (M:2 T:5)
        // -- generic
        case 1064: goto step_next;
        // -- overlapped
        case 1065: cpu->a=cpu->i;cpu->f=_z80_sziff2_flags(cpu, cpu->i);goto fetch_next;
        
        // ED 58: in e,(c) (M:2 T:8)
        // -- ioread
        case 1066: goto step_next;
        case 1067: goto step_next;
        case 1068: _wait();_ioread(cpu->bc);goto step_next;
        case 1069: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;goto step_next;
        // -- overlapped
        case 1070: cpu->e=_z80_in(cpu,cpu->dlatch);goto fetch_next;
        
        // ED 59: out (c),e (M:2 T:8)
        // -- iowrite
        case 1071: goto step_next;
        case 1072: _iowrite(cpu->bc,cpu->e);goto step_next;
        case 1073: _wait();cpu->wz=cpu->bc+1;goto step_next;
        case 1074: goto step_next;
        // -- overlapped
        case 1075: goto fetch_next;
        
        // ED 5A: adc hl,de (M:2 T:11)
        // -- generic
        case 1076: _z80_adc16(cpu,cpu->de);goto step_next;
        case 1077: goto step_next;
        case 1078: goto step_next;
        case 1079: goto step_next;
        case 1080: goto step_next;
        case 1081: goto step_next;
        case 1082: goto step_next;
        // -- overlapped
        case 1083: goto fetch_next;
        
        // ED 5B: ld de,(nn) (M:5 T:16)
        // -- mread
        case 1084: goto step_next;
        case 1085: _wait();_mread(cpu->pc++);goto step_next;
        case 1086: cpu->wzl=_gd();goto step_next;
        // -- mread
        case 1087: goto step_next;
        case 1088: _wait();_mread(cpu->pc++);goto step_next;
        case 1089: cpu->wzh=_gd();goto step_next;
        // -- mread
        case 1090: goto step_next;
        case 1091: _wait();_mread(cpu->wz++);goto step_next;
        case 1092: cpu->e=_gd();goto step_next;
        // -- mread
        case 1093: goto step_next;
        case 1094: _wait();_mread(cpu->wz);goto step_next;
        case 1095: cpu->d=_gd();goto step_next;
        // -- overlapped
        case 1096: goto fetch_next;
        
        // ED 5E: im 2 (M:1 T:4)
        // -- overlapped
        case 1097: cpu->im=2;goto fetch_next;
        
        // ED 5F: ld a,r (M:2 T:5)
        // -- generic
        case 1098: goto step_next;
        // -- overlapped
        case 1099: cpu->a=cpu->r;cpu->f=_z80_sziff2_flags(cpu, cpu->r);goto fetch_next;
        
        // ED 60: in h,(c) (M:2 T:8)
        // -- ioread
        case 1100: goto step_next;
        case 1101: goto step_next;
        case 1102: _wait();_ioread(cpu->bc);goto step_next;
        case 1103: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;goto step_next;
        // -- overlapped
        case 1104: cpu->hlx[cpu->hlx_idx].h=_z80_in(cpu,cpu->dlatch);goto fetch_next;
        
        // ED 61: out (c),h (M:2 T:8)
        // -- iowrite
        case 1105: goto step_next;
        case 1106: _iowrite(cpu->bc,cpu->hlx[cpu->hlx_idx].h);goto step_next;
        case 1107: _wait();cpu->wz=cpu->bc+1;goto step_next;
        case 1108: goto step_next;
        // -- overlapped
        case 1109: goto fetch_next;
        
        // ED 62: sbc hl,hl (M:2 T:11)
        // -- generic
        case 1110: _z80_sbc16(cpu,cpu->hl);goto step_next;
        case 1111: goto step_next;
        case 1112: goto step_next;
        case 1113: goto step_next;
        case 1114: goto step_next;
        case 1115: goto step_next;
        case 1116: goto step_next;
        // -- overlapped
        case 1117: goto fetch_next;
        
        // ED 63: ld (nn),hl (M:5 T:16)
        // -- mread
        case 1118: goto step_next;
        case 1119: _wait();_mread(cpu->pc++);goto step_next;
        case 1120: cpu->wzl=_gd();goto step_next;
        // -- mread
        case 1121: goto step_next;
        case 1122: _wait();_mread(cpu->pc++);goto step_next;
        case 1123: cpu->wzh=_gd();goto step_next;
        // -- mwrite
        case 1124: goto step_next;
        case 1125: _wait();_mwrite(cpu->wz++,cpu->l);goto step_next;
        case 1126: goto step_next;
        // -- mwrite
        case 1127: goto step_next;
        case 1128: _wait();_mwrite(cpu->wz,cpu->h);goto step_next;
        case 1129: goto step_next;
        // -- overlapped
        case 1130: goto fetch_next;
        
        // ED 66: im 0 (M:1 T:4)
        // -- overlapped
        case 1131: cpu->im=0;goto fetch_next;
        
        // ED 67: rrd (M:4 T:14)
        // -- mread
        case 1132: goto step_next;
        case 1133: _wait();_mread(cpu->hl);goto step_next;
        case 1134: cpu->dlatch=_gd();goto step_next;
        // -- generic
        case 1135: cpu->dlatch=_z80_rrd(cpu,cpu->dlatch);goto step_next;
        case 1136: goto step_next;
        case 1137: goto step_next;
        case 1138: goto step_next;
        // -- mwrite
        case 1139: goto step_next;
        case 1140: _wait();_mwrite(cpu->hl,cpu->dlatch);cpu->wz=cpu->hl+1;goto step_next;
        case 1141: goto step_next;
        // -- overlapped
        case 1142: goto fetch_next;
        
        // ED 68: in l,(c) (M:2 T:8)
        // -- ioread
        case 1143: goto step_next;
        case 1144: goto step_next;
        case 1145: _wait();_ioread(cpu->bc);goto step_next;
        case 1146: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;goto step_next;
        // -- overlapped
        case 1147: cpu->hlx[cpu->hlx_idx].l=_z80_in(cpu,cpu->dlatch);goto fetch_next;
        
        // ED 69: out (c),l (M:2 T:8)
        // -- iowrite
        case 1148: goto step_next;
        case 1149: _iowrite(cpu->bc,cpu->hlx[cpu->hlx_idx].l);goto step_next;
        case 1150: _wait();cpu->wz=cpu->bc+1;goto step_next;
        case 1151: goto step_next;
        // -- overlapped
        case 1152: goto fetch_next;
        
        // ED 6A: adc hl,hl (M:2 T:11)
        // -- generic
        case 1153: _z80_adc16(cpu,cpu->hl);goto step_next;
        case 1154: goto step_next;
        case 1155: goto step_next;
        case 1156: goto step_next;
        case 1157: goto step_next;
        case 1158: goto step_next;
        case 1159: goto step_next;
        // -- overlapped
        case 1160: goto fetch_next;
        
        // ED 6B: ld hl,(nn) (M:5 T:16)
        // -- mread
        case 1161: goto step_next;
        case 1162: _wait();_mread(cpu->pc++);goto step_next;
        case 1163: cpu->wzl=_gd();goto step_next;
        // -- mread
        case 1164: goto step_next;
        case 1165: _wait();_mread(cpu->pc++);goto step_next;
        case 1166: cpu->wzh=_gd();goto step_next;
        // -- mread
        case 1167: goto step_next;
        case 1168: _wait();_mread(cpu->wz++);goto step_next;
        case 1169: cpu->l=_gd();goto step_next;
        // -- mread
        case 1170: goto step_next;
        case 1171: _wait();_mread(cpu->wz);goto step_next;
        case 1172: cpu->h=_gd();goto step_next;
        // -- overlapped
        case 1173: goto fetch_next;
        
        // ED 6E: im 0 (M:1 T:4)
        // -- overlapped
        case 1174: cpu->im=0;goto fetch_next;
        
        // ED 6F: rld (M:4 T:14)
        // -- mread
        case 1175: goto step_next;
        case 1176: _wait();_mread(cpu->hl);goto step_next;
        case 1177: cpu->dlatch=_gd();goto step_next;
        // -- generic
        case 1178: cpu->dlatch=_z80_rld(cpu,cpu->dlatch);goto step_next;
        case 1179: goto step_next;
        case 1180: goto step_next;
        case 1181: goto step_next;
        // -- mwrite
        case 1182: goto step_next;
        case 1183: _wait();_mwrite(cpu->hl,cpu->dlatch);cpu->wz=cpu->hl+1;goto step_next;
        case 1184: goto step_next;
        // -- overlapped
        case 1185: goto fetch_next;
        
        // ED 70: in (c) (M:2 T:8)
        // -- ioread
        case 1186: goto step_next;
        case 1187: goto step_next;
        case 1188: _wait();_ioread(cpu->bc);goto step_next;
        case 1189: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;goto step_next;
        // -- overlapped
        case 1190: _z80_in(cpu,cpu->dlatch);goto fetch_next;
        
        // ED 71: out (c),0 (M:2 T:8)
        // -- iowrite
        case 1191: goto step_next;
        case 1192: _iowrite(cpu->bc,0);goto step_next;
        case 1193: _wait();cpu->wz=cpu->bc+1;goto step_next;
        case 1194: goto step_next;
        // -- overlapped
        case 1195: goto fetch_next;
        
        // ED 72: sbc hl,sp (M:2 T:11)
        // -- generic
        case 1196: _z80_sbc16(cpu,cpu->sp);goto step_next;
        case 1197: goto step_next;
        case 1198: goto step_next;
        case 1199: goto step_next;
        case 1200: goto step_next;
        case 1201: goto step_next;
        case 1202: goto step_next;
        // -- overlapped
        case 1203: goto fetch_next;
        
        // ED 73: ld (nn),sp (M:5 T:16)
        // -- mread
        case 1204: goto step_next;
        case 1205: _wait();_mread(cpu->pc++);goto step_next;
        case 1206: cpu->wzl=_gd();goto step_next;
        // -- mread
        case 1207: goto step_next;
        case 1208: _wait();_mread(cpu->pc++);goto step_next;
        case 1209: cpu->wzh=_gd();goto step_next;
        // -- mwrite
        case 1210: goto step_next;
        case 1211: _wait();_mwrite(cpu->wz++,cpu->spl);goto step_next;
        case 1212: goto step_next;
        // -- mwrite
        case 1213: goto step_next;
        case 1214: _wait();_mwrite(cpu->wz,cpu->sph);goto step_next;
        case 1215: goto step_next;
        // -- overlapped
        case 1216: goto fetch_next;
        
        // ED 76: im 1 (M:1 T:4)
        // -- overlapped
        case 1217: cpu->im=1;goto fetch_next;
        
        // ED 78: in a,(c) (M:2 T:8)
        // -- ioread
        case 1218: goto step_next;
        case 1219: goto step_next;
        case 1220: _wait();_ioread(cpu->bc);goto step_next;
        case 1221: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;goto step_next;
        // -- overlapped
        case 1222: cpu->a=_z80_in(cpu,cpu->dlatch);goto fetch_next;
        
        // ED 79: out (c),a (M:2 T:8)
        // -- iowrite
        case 1223: goto step_next;
        case 1224: _iowrite(cpu->bc,cpu->a);goto step_next;
        case 1225: _wait();cpu->wz=cpu->bc+1;goto step_next;
        case 1226: goto step_next;
        // -- overlapped
        case 1227: goto fetch_next;
        
        // ED 7A: adc hl,sp (M:2 T:11)
        // -- generic
        case 1228: _z80_adc16(cpu,cpu->sp);goto step_next;
        case 1229: goto step_next;
        case 1230: goto step_next;
        case 1231: goto step_next;
        case 1232: goto step_next;
        case 1233: goto step_next;
        case 1234: goto step_next;
        // -- overlapped
        case 1235: goto fetch_next;
        
        // ED 7B: ld sp,(nn) (M:5 T:16)
        // -- mread
        case 1236: goto step_next;
        case 1237: _wait();_mread(cpu->pc++);goto step_next;
        case 1238: cpu->wzl=_gd();goto step_next;
        // -- mread
        case 1239: goto step_next;
        case 1240: _wait();_mread(cpu->pc++);goto step_next;
        case 1241: cpu->wzh=_gd();goto step_next;
        // -- mread
        case 1242: goto step_next;
        case 1243: _wait();_mread(cpu->wz++);goto step_next;
        case 1244: cpu->spl=_gd();goto step_next;
        // -- mread
        case 1245: goto step_next;
        case 1246: _wait();_mread(cpu->wz);goto step_next;
        case 1247: cpu->sph=_gd();goto step_next;
        // -- overlapped
        case 1248: goto fetch_next;
        
        // ED 7E: im 2 (M:1 T:4)
        // -- overlapped
        case 1249: cpu->im=2;goto fetch_next;
        
        // ED A0: ldi (M:4 T:12)
        // -- mread
        case 1250: goto step_next;
        case 1251: _wait();_mread(cpu->hl++);goto step_next;
        case 1252: cpu->dlatch=_gd();goto step_next;
        // -- mwrite
        case 1253: goto step_next;
        case 1254: _wait();_mwrite(cpu->de++,cpu->dlatch);goto step_next;
        case 1255: goto step_next;
        // -- generic
        case 1256: _z80_ldi_ldd(cpu,cpu->dlatch);goto step_next;
        case 1257: goto step_next;
        // -- overlapped
        case 1258: goto fetch_next;
        
        // ED A1: cpi (M:3 T:12)
        // -- mread
        case 1259: goto step_next;
        case 1260: _wait();_mread(cpu->hl++);goto step_next;
        case 1261: cpu->dlatch=_gd();goto step_next;
        // -- generic
        case 1262: cpu->wz++;_z80_cpi_cpd(cpu,cpu->dlatch);goto step_next;
        case 1263: goto step_next;
        case 1264: goto step_next;
        case 1265: goto step_next;
        case 1266: goto step_next;
        // -- overlapped
        case 1267: goto fetch_next;
        
        // ED A2: ini (M:4 T:12)
        // -- generic
        case 1268: goto step_next;
        // -- ioread
        case 1269: goto step_next;
        case 1270: goto step_next;
        case 1271: _wait();_ioread(cpu->bc);goto step_next;
        case 1272: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;cpu->b--;;goto step_next;
        // -- mwrite
        case 1273: goto step_next;
        case 1274: _wait();_mwrite(cpu->hl++,cpu->dlatch);_z80_ini_ind(cpu,cpu->dlatch,cpu->c+1);goto step_next;
        case 1275: goto step_next;
        // -- overlapped
        case 1276: goto fetch_next;
        
        // ED A3: outi (M:4 T:12)
        // -- generic
        case 1277: goto step_next;
        // -- mread
        case 1278: goto step_next;
        case 1279: _wait();_mread(cpu->hl++);goto step_next;
        case 1280: cpu->dlatch=_gd();cpu->b--;goto step_next;
        // -- iowrite
        case 1281: goto step_next;
        case 1282: _iowrite(cpu->bc,cpu->dlatch);goto step_next;
        case 1283: _wait();cpu->wz=cpu->bc+1;_z80_outi_outd(cpu,cpu->dlatch);goto step_next;
        case 1284: goto step_next;
        // -- overlapped
        case 1285: goto fetch_next;
        
        // ED A8: ldd (M:4 T:12)
        // -- mread
        case 1286: goto step_next;
        case 1287: _wait();_mread(cpu->hl--);goto step_next;
        case 1288: cpu->dlatch=_gd();goto step_next;
        // -- mwrite
        case 1289: goto step_next;
        case 1290: _wait();_mwrite(cpu->de--,cpu->dlatch);goto step_next;
        case 1291: goto step_next;
        // -- generic
        case 1292: _z80_ldi_ldd(cpu,cpu->dlatch);goto step_next;
        case 1293: goto step_next;
        // -- overlapped
        case 1294: goto fetch_next;
        
        // ED A9: cpd (M:3 T:12)
        // -- mread
        case 1295: goto step_next;
        case 1296: _wait();_mread(cpu->hl--);goto step_next;
        case 1297: cpu->dlatch=_gd();goto step_next;
        // -- generic
        case 1298: cpu->wz--;_z80_cpi_cpd(cpu,cpu->dlatch);goto step_next;
        case 1299: goto step_next;
        case 1300: goto step_next;
        case 1301: goto step_next;
        case 1302: goto step_next;
        // -- overlapped
        case 1303: goto fetch_next;
        
        // ED AA: ind (M:4 T:12)
        // -- generic
        case 1304: goto step_next;
        // -- ioread
        case 1305: goto step_next;
        case 1306: goto step_next;
        case 1307: _wait();_ioread(cpu->bc);goto step_next;
        case 1308: cpu->dlatch=_gd();cpu->wz=cpu->bc-1;cpu->b--;;goto step_next;
        // -- mwrite
        case 1309: goto step_next;
        case 1310: _wait();_mwrite(cpu->hl--,cpu->dlatch);_z80_ini_ind(cpu,cpu->dlatch,cpu->c-1);goto step_next;
        case 1311: goto step_next;
        // -- overlapped
        case 1312: goto fetch_next;
        
        // ED AB: outd (M:4 T:12)
        // -- generic
        case 1313: goto step_next;
        // -- mread
        case 1314: goto step_next;
        case 1315: _wait();_mread(cpu->hl--);goto step_next;
        case 1316: cpu->dlatch=_gd();cpu->b--;goto step_next;
        // -- iowrite
        case 1317: goto step_next;
        case 1318: _iowrite(cpu->bc,cpu->dlatch);goto step_next;
        case 1319: _wait();cpu->wz=cpu->bc-1;_z80_outi_outd(cpu,cpu->dlatch);goto step_next;
        case 1320: goto step_next;
        // -- overlapped
        case 1321: goto fetch_next;
        
        // ED B0: ldir (M:5 T:17)
        // -- mread
        case 1322: goto step_next;
        case 1323: _wait();_mread(cpu->hl++);goto step_next;
        case 1324: cpu->dlatch=_gd();goto step_next;
        // -- mwrite
        case 1325: goto step_next;
        case 1326: _wait();_mwrite(cpu->de++,cpu->dlatch);goto step_next;
        case 1327: goto step_next;
        // -- generic
        case 1328: if(!_z80_ldi_ldd(cpu,cpu->dlatch)){_skip(5);};goto step_next;
        case 1329: goto step_next;
        // -- generic
        case 1330: cpu->wz=--cpu->pc;--cpu->pc;;goto step_next;
        case 1331: goto step_next;
        case 1332: goto step_next;
        case 1333: goto step_next;
        case 1334: goto step_next;
        // -- overlapped
        case 1335: goto fetch_next;
        
        // ED B1: cpir (M:4 T:17)
        // -- mread
        case 1336: goto step_next;
        case 1337: _wait();_mread(cpu->hl++);goto step_next;
        case 1338: cpu->dlatch=_gd();goto step_next;
        // -- generic
        case 1339: cpu->wz++;if(!_z80_cpi_cpd(cpu,cpu->dlatch)){_skip(5);};goto step_next;
        case 1340: goto step_next;
        case 1341: goto step_next;
        case 1342: goto step_next;
        case 1343: goto step_next;
        // -- generic
        case 1344: cpu->wz=--cpu->pc;--cpu->pc;goto step_next;
        case 1345: goto step_next;
        case 1346: goto step_next;
        case 1347: goto step_next;
        case 1348: goto step_next;
        // -- overlapped
        case 1349: goto fetch_next;
        
        // ED B2: inir (M:5 T:17)
        // -- generic
        case 1350: goto step_next;
        // -- ioread
        case 1351: goto step_next;
        case 1352: goto step_next;
        case 1353: _wait();_ioread(cpu->bc);goto step_next;
        case 1354: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;cpu->b--;;goto step_next;
        // -- mwrite
        case 1355: goto step_next;
        case 1356: _wait();_mwrite(cpu->hl++,cpu->dlatch);if (!_z80_ini_ind(cpu,cpu->dlatch,cpu->c+1)){_skip(5);};goto step_next;
        case 1357: goto step_next;
        // -- generic
        case 1358: cpu->wz=--cpu->pc;--cpu->pc;goto step_next;
        case 1359: goto step_next;
        case 1360: goto step_next;
        case 1361: goto step_next;
        case 1362: goto step_next;
        // -- overlapped
        case 1363: goto fetch_next;
        
        // ED B3: otir (M:5 T:17)
        // -- generic
        case 1364: goto step_next;
        // -- mread
        case 1365: goto step_next;
        case 1366: _wait();_mread(cpu->hl++);goto step_next;
        case 1367: cpu->dlatch=_gd();cpu->b--;goto step_next;
        // -- iowrite
        case 1368: goto step_next;
        case 1369: _iowrite(cpu->bc,cpu->dlatch);goto step_next;
        case 1370: _wait();cpu->wz=cpu->bc+1;if(!_z80_outi_outd(cpu,cpu->dlatch)){_skip(5);};goto step_next;
        case 1371: goto step_next;
        // -- generic
        case 1372: cpu->wz=--cpu->pc;--cpu->pc;goto step_next;
        case 1373: goto step_next;
        case 1374: goto step_next;
        case 1375: goto step_next;
        case 1376: goto step_next;
        // -- overlapped
        case 1377: goto fetch_next;
        
        // ED B8: lddr (M:5 T:17)
        // -- mread
        case 1378: goto step_next;
        case 1379: _wait();_mread(cpu->hl--);goto step_next;
        case 1380: cpu->dlatch=_gd();goto step_next;
        // -- mwrite
        case 1381: goto step_next;
        case 1382: _wait();_mwrite(cpu->de--,cpu->dlatch);goto step_next;
        case 1383: goto step_next;
        // -- generic
        case 1384: if(!_z80_ldi_ldd(cpu,cpu->dlatch)){_skip(5);};goto step_next;
        case 1385: goto step_next;
        // -- generic
        case 1386: cpu->wz=--cpu->pc;--cpu->pc;;goto step_next;
        case 1387: goto step_next;
        case 1388: goto step_next;
        case 1389: goto step_next;
        case 1390: goto step_next;
        // -- overlapped
        case 1391: goto fetch_next;
        
        // ED B9: cpdr (M:4 T:17)
        // -- mread
        case 1392: goto step_next;
        case 1393: _wait();_mread(cpu->hl--);goto step_next;
        case 1394: cpu->dlatch=_gd();goto step_next;
        // -- generic
        case 1395: cpu->wz--;if(!_z80_cpi_cpd(cpu,cpu->dlatch)){_skip(5);};goto step_next;
        case 1396: goto step_next;
        case 1397: goto step_next;
        case 1398: goto step_next;
        case 1399: goto step_next;
        // -- generic
        case 1400: cpu->wz=--cpu->pc;--cpu->pc;goto step_next;
        case 1401: goto step_next;
        case 1402: goto step_next;
        case 1403: goto step_next;
        case 1404: goto step_next;
        // -- overlapped
        case 1405: goto fetch_next;
        
        // ED BA: indr (M:5 T:17)
        // -- generic
        case 1406: goto step_next;
        // -- ioread
        case 1407: goto step_next;
        case 1408: goto step_next;
        case 1409: _wait();_ioread(cpu->bc);goto step_next;
        case 1410: cpu->dlatch=_gd();cpu->wz=cpu->bc-1;cpu->b--;;goto step_next;
        // -- mwrite
        case 1411: goto step_next;
        case 1412: _wait();_mwrite(cpu->hl--,cpu->dlatch);if (!_z80_ini_ind(cpu,cpu->dlatch,cpu->c-1)){_skip(5);};goto step_next;
        case 1413: goto step_next;
        // -- generic
        case 1414: cpu->wz=--cpu->pc;--cpu->pc;goto step_next;
        case 1415: goto step_next;
        case 1416: goto step_next;
        case 1417: goto step_next;
        case 1418: goto step_next;
        // -- overlapped
        case 1419: goto fetch_next;
        
        // ED BB: otdr (M:5 T:17)
        // -- generic
        case 1420: goto step_next;
        // -- mread
        case 1421: goto step_next;
        case 1422: _wait();_mread(cpu->hl--);goto step_next;
        case 1423: cpu->dlatch=_gd();cpu->b--;goto step_next;
        // -- iowrite
        case 1424: goto step_next;
        case 1425: _iowrite(cpu->bc,cpu->dlatch);goto step_next;
        case 1426: _wait();cpu->wz=cpu->bc-1;if(!_z80_outi_outd(cpu,cpu->dlatch)){_skip(5);};goto step_next;
        case 1427: goto step_next;
        // -- generic
        case 1428: cpu->wz=--cpu->pc;--cpu->pc;goto step_next;
        case 1429: goto step_next;
        case 1430: goto step_next;
        case 1431: goto step_next;
        case 1432: goto step_next;
        // -- overlapped
        case 1433: goto fetch_next;
        
        // CB 00: cb (M:1 T:4)
        // -- overlapped
        case 1434: {uint8_t z=cpu->opcode&7;_z80_cb_action(cpu,z,z);};goto fetch_next;
        
        // CB 00: cbhl (M:3 T:11)
        // -- mread
        case 1435: goto step_next;
        case 1436: _wait();_mread(cpu->hl);goto step_next;
        case 1437: cpu->dlatch=_gd();if(!_z80_cb_action(cpu,6,6)){_skip(3);};goto step_next;
        case 1438: goto step_next;
        // -- mwrite
        case 1439: goto step_next;
        case 1440: _wait();_mwrite(cpu->hl,cpu->dlatch);goto step_next;
        case 1441: goto step_next;
        // -- overlapped
        case 1442: goto fetch_next;
        
        // CB 00: ddfdcb (M:6 T:18)
        // -- generic
        case 1443: _wait();_mread(cpu->pc++);goto step_next;
        // -- generic
        case 1444: _z80_ddfdcb_addr(cpu,pins);goto step_next;
        // -- mread
        case 1445: goto step_next;
        case 1446: _wait();_mread(cpu->pc++);goto step_next;
        case 1447: cpu->dlatch=_gd();_z80_ddfdcb_opcode(cpu,cpu->dlatch);goto step_next;
        case 1448: goto step_next;
        case 1449: goto step_next;
        // -- mread
        case 1450: goto step_next;
        case 1451: _wait();_mread(cpu->addr);goto step_next;
        case 1452: cpu->dlatch=_gd();if(!_z80_cb_action(cpu,6,cpu->opcode&7)){_skip(3);};goto step_next;
        case 1453: goto step_next;
        // -- mwrite
        case 1454: goto step_next;
        case 1455: _wait();_mwrite(cpu->addr,cpu->dlatch);goto step_next;
        case 1456: goto step_next;
        // -- overlapped
        case 1457: goto fetch_next;
        
        //  00: int_im0 (M:6 T:9)
        // -- generic
        case 1458: pins=_z80_int012_step0(cpu,pins);goto step_next;
        // -- generic
        case 1459: pins=_z80_int012_step1(cpu,pins);goto step_next;
        // -- generic
        case 1460: _wait();pins=_z80_int0_step2(cpu,pins);goto step_next;
        // -- generic
        case 1461: pins=_z80_refresh(cpu,pins);goto step_next;
        // -- generic
        case 1462: pins=_z80_int0_step3(cpu,pins);goto step_next;
        // -- overlapped
        case 1463: goto fetch_next;
        
        //  00: int_im1 (M:7 T:16)
        // -- generic
        case 1464: pins=_z80_int012_step0(cpu,pins);goto step_next;
        // -- generic
        case 1465: pins=_z80_int012_step1(cpu,pins);goto step_next;
        // -- generic
        case 1466: _wait();goto step_next;
        // -- generic
        case 1467: pins=_z80_refresh(cpu,pins);goto step_next;
        case 1468: goto step_next;
        case 1469: goto step_next;
        // -- mwrite
        case 1470: goto step_next;
        case 1471: _wait();_mwrite(--cpu->sp,cpu->pch);goto step_next;
        case 1472: goto step_next;
        // -- mwrite
        case 1473: goto step_next;
        case 1474: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=cpu->pc=0x0038;goto step_next;
        case 1475: goto step_next;
        // -- overlapped
        case 1476: goto fetch_next;
        
        //  00: int_im2 (M:9 T:22)
        // -- generic
        case 1477: pins=_z80_int012_step0(cpu,pins);goto step_next;
        // -- generic
        case 1478: pins=_z80_int012_step1(cpu,pins);goto step_next;
        // -- generic
        case 1479: _wait();cpu->dlatch=_z80_get_db(pins);goto step_next;
        // -- generic
        case 1480: pins=_z80_refresh(cpu,pins);goto step_next;
        case 1481: goto step_next;
        case 1482: goto step_next;
        // -- mwrite
        case 1483: goto step_next;
        case 1484: _wait();_mwrite(--cpu->sp,cpu->pch);goto step_next;
        case 1485: goto step_next;
        // -- mwrite
        case 1486: goto step_next;
        case 1487: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wzl=cpu->dlatch;cpu->wzh=cpu->i;goto step_next;
        case 1488: goto step_next;
        // -- mread
        case 1489: goto step_next;
        case 1490: _wait();_mread(cpu->wz++);goto step_next;
        case 1491: cpu->dlatch=_gd();goto step_next;
        // -- mread
        case 1492: goto step_next;
        case 1493: _wait();_mread(cpu->wz);goto step_next;
        case 1494: cpu->wzh=_gd();cpu->wzl=cpu->dlatch;cpu->pc=cpu->wz;goto step_next;
        // -- overlapped
        case 1495: goto fetch_next;
        
        //  00: nmi (M:5 T:14)
        // -- generic
        case 1496: pins=_z80_nmi_step0(cpu,pins);goto step_next;
        // -- generic
        case 1497: pins=_z80_refresh(cpu,pins);goto step_next;
        case 1498: goto step_next;
        case 1499: goto step_next;
        // -- mwrite
        case 1500: goto step_next;
        case 1501: _wait();_mwrite(--cpu->sp,cpu->pch);goto step_next;
        case 1502: goto step_next;
        // -- mwrite
        case 1503: goto step_next;
        case 1504: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=cpu->pc=0x0066;goto step_next;
        case 1505: goto step_next;
        // -- overlapped
        case 1506: goto fetch_next;

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
