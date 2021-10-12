#pragma once
/*
    FIXME
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

// output pins
#define Z80_M1    (1ULL<<24)        // machine cycle 1
#define Z80_MREQ  (1ULL<<25)        // memory request
#define Z80_IORQ  (1ULL<<26)        // input/output request
#define Z80_RD    (1ULL<<27)        // read
#define Z80_WR    (1ULL<<28)        // write
#define Z80_HALT  (1ULL<<29)        // halt state
#define Z80_RFSH  (1ULL<<32)        // refresh
// NOTE: this mask not containing the HALT pin is intended
#define Z80_CTRL_MASK (Z80_M1|Z80_MREQ|Z80_IORQ|Z80_RD|Z80_WR|Z80_RFSH)

// input pins
#define Z80_INT   (1ULL<<30)        // interrupt request
#define Z80_NMI   (1ULL<<31)        // non-maskable interrupt
#define Z80_RES   (1ULL<<33)        // reset requested

// virtual pins (for interrupt daisy chain protocol)
#define Z80_IEIO    (1ULL<<37)      // unified daisy chain 'Interrupt Enable In+Out'
#define Z80_RETI    (1ULL<<38)      // cpu has decoded a RETI instruction

#define Z80_PIN_MASK ((1ULL<<40)-1)

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

/* The 'execution pipeline' is a single 64-bit mask which is shifted right with
   each tcycle and which controls execution of various 'parts' of the CPU
   emulation.

   The low 24 bits define 'active tcycles' in an instruction, if bit 0 is 
   set, the next case-branch in the big switch-case decoder will be called.

   The other bit ranges are used to trigger various delayed actions:

   - memory and io read/write machine cycles
   - interrupt handling
   - 
*/
#define Z80_PIP_MASK_LOADIR     (0x3ULL<<0) // load instruction register
#define Z80_PIP_MASK_TCYCLE     (0xFFFFFFFFULL<<2) // up to 32 opcode tcycles

#define Z80_PIP_BIT_LOAD_IR     (1ULL<<0)
#define Z80_PIP_BIT_TCYCLE      (1ULL<<2)
#define Z80_PIP_BITS_MASK       (Z80_PIP_BIT_TCYCLE|Z80_PIP_BIT_LOAD_IR)

// CPU state
typedef struct {
    uint64_t pins;      // last stored pin state
    uint64_t pip;       // execution pipeline
    uint16_t ir;        // instruction register with extra bits for next 'active' tcycle
    uint16_t pc;        // program counter
    uint8_t r;          // refresh register
} z80_t;

// initialize a new Z80 instance and return initial pin mask
uint64_t z80_init(z80_t* cpu);
// execute one tick, return new pin mask
uint64_t z80_tick(z80_t* cpu, uint64_t pins);

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

// set 16-bit address in 64-bit pin mask
#define _SA(addr) pins=(pins&~0xFFFF)|((addr)&0xFFFFULL)
// set 16-bit addess and extra pins
#define _SAX(addr,x) pins=(pins&~0xFFFF)|((addr)&0xFFFFULL)|(x)
// extract 16-bit addess from pin mask
#define _GA() ((uint16_t)(pins&0xFFFFULL))
// set 16-bit address and 8-bit data in 64-bit pin mask
#define _SAD(addr,data) pins=(pins&~0xFFFFFF)|((((data)&0xFF)<<16)&0xFF0000ULL)|((addr)&0xFFFFULL)
// set 8-bit data in 64-bit pin mask
#define _SD(data) pins=((pins&~0xFF0000ULL)|(((data&0xFF)<<16)&0xFF0000ULL))
// extract 8-bit data from 64-bit pin mask
#define _GD() ((uint8_t)((pins&0xFF0000ULL)>>16))
// set bit mask for an opcode fetch
#define _FETCH() _SAX(c->pc++,Z80_M1|Z80_MREQ|Z80_RD)
// set bit mask for the refresh sub-machine-cycle
#define _RFSH() _SAX(c->r,Z80_MREQ|Z80_RFSH);c->r=(c->r&0x80)|((c->r+1)&0x7F)

uint64_t z80_init(z80_t* cpu) {
    CHIPS_ASSERT(cpu);
    memset(cpu, 0, sizeof(cpu));
    // FIXME initial CPU state (according to visualz80)
}

uint64_t z80_tick(z80_t* c, uint64_t pins) {
    c->pip =  (c->pip & ~Z80_PIP_BITS_MASK) >> 1;
    if (c->pip & Z80_PIP_BIT_LOAD_IR) {
        // load next instruction byte from data bus into instruction
        // register, make room for machine cycle counter
        c->ir = _GD()<<3;
    }
    // early out if there's nothing else to do this tick
    pins &= ~Z80_CTRL_MASK;
    if (0 == (c->pip & Z80_PIP_BIT_TCYCLE)) {
        c->pins = pins;
        return pins;
    }
    // process the next 'active' tcycle
    switch (c->ir++) {
$decode_block
    }
    c->pins = pins;
}

#undef _SA
#undef _GA
#undef _SAD
#undef _SD
#undef _GD
#endif // CHIPS_IMPL