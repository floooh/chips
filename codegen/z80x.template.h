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

// 
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

// machine cycle execution pipeline bits (TODO: explain this stuff)
#define Z80_PIP_BIT_STEP        (1ULL<<0)  // step the instruction decoder forward
#define Z80_PIP_BIT_WAIT        (1ULL<<32) // sample the wait pin

#define Z80_PIP_BITS (Z80_PIP_BIT_STEP|Z80_PIP_BIT_WAIT)

#define Z80_PIP_MASK_STEP       (0xFFFFFFFFULL)
#define Z80_PIP_MASK_WAIT       (0xFFFFFFFF00000000ULL)

// CPU state
typedef struct {
    uint64_t pins;      // last stored pin state
    uint64_t pip;       // execution pipeline
    uint16_t ir;        // instruction register with extra bits for next 'active' tcycle
    uint16_t pc;        // program counter
    uint8_t f, a, c, b, e, d, l, h;
    uint16_t wz;
    uint16_t sp;
    uint16_t ix;
    uint16_t iy;
    uint8_t i;
    uint8_t r;
    uint8_t im;
    uint8_t af2, bc2, de2, hl2; // shadow register bank
    uint8_t dlatch;     // temporary store for data bus value
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

static const uint64_t z80_pip_table[256] = {
$pip_table;
};

uint64_t z80_init(z80_t* cpu) {
    CHIPS_ASSERT(cpu);
    memset(cpu, 0, sizeof(cpu));
    // initial state according to visualz80
    cpu->f = cpu->a = cpu->c = cpu->b = 0x55;
    cpu->e = cpu->d = cpu->l = cpu->h = 0x55f;
    cpu->wz = cpu->sp = cpu->ix = cpu->ix = 0x5555;
    cpu->af2 = cpu->bc2 = cpu->de2 = cpu->hl2 = 0x5555;
    // FIXME: iff1/2 disabled, initial value of IM???
}

static inline uint64_t z80_set_ab(uint64_t pins, uint16_t ab) {
    return (pins & ~0xFFFF) | ab;
}

static inline uint64_t z80_set_ab_x(uint64_t pins, uint16_t ab, uint64_t x) {
    return (pins & ~0xFFFF) | ab | x;
}

static inline uint64_t z80_set_ab_db(uint64_t pins, uint16_t ab, uint8_t db) {
    return (pins & ~0xFFFFFF) | (db<<16) | ab;
}

static inline uint64_t z80_set_ab_db_x(uint64_t pins, uint16_t ab, uint8_t db, uint64_t x) {
    return (pins & ~0xFFFFFF) | (db<<16) | ab | x;
}

static inline uint8_t z80_get_db(uint64_t pins) {
    return pins>>16;
}

// register helper macros
#define _gaf()      ((uint16_t)(cpu->f<<8)|cpu->a)
#define _gbc()      ((uint16_t)(cpu->b<<8)|cpu->c)
#define _gde()      ((uint16_t)(cpu->d<<8)|cpu->e)
#define _ghl()      ((uint16_t)(cpu->h<<8)|cpu->l)
#define _gsp()      (cpu->sp)
#define _saf(af)    {cpu->f=af>>8;cpu->a=af}
#define _sbc(bc)    {cpu->b=bc>>8;cpu->c=bc}
#define _sde(de)    {cpu->d=de>>8;cpu->e=de}
#define _shl(hl)    {cpu->h=hl>>8;cpu->l=hl}
#define _ssp(sp)    {cpu->sp=sp;}

// pin helper macros
#define _sa(ab)             pins=z80_set_ab(pins,ab)
#define _sax(ab,x)          pins=z80_set_ab_x(pins,ab,x)
#define _sad(ab,d)          pins=z80_set_ab_db(pins,ab,d)
#define _sadx(ab,d,x)       pins=z80_set_ab_db_x(pins,ab,d,x)
#define _gd()               z80_get_db(pins)

// high level helper macros
#define _fetch()    _sax(cpu->pc++,Z80_M1|Z80_MREQ|Z80_RD)
#define _rfsh()     _sax(cpu->r,Z80_MREQ|Z80_RFSH);cpu->r=(cpu->r&0x80)|((cpu->r+1)&0x7F)
#define _mr(ab)     _sax(ab,Z80_MREQ|Z80_RD)
#define _mw(ab,d)   _sadx(ab,d,Z80_MREQ|Z80_WR)
#define _ior(ab)    _sax(ab,Z80_IORQ|Z80_RD)
#define _iow(ab,d)  _sadx(ab,d,Z80_IORQ|Z80_WR)

// function call helpers
#define _add(val)   z80_add(cpu,val)
#define _adc(val)   z80_adc(cpu,val)
#define _sub(val)   z80_sub(cpu,val)
#define _sbc(val)   z80_sbc(cpu,val)
#define _and(val)   z80_and(cpu,val)
#define _xor(val)   z80_xor(cpu,val)
#define _or(val)    z80_or(cpu,val)
#define _cp(val)    z80_cp(cpu,val)

uint64_t z80_tick(z80_t* cpu, uint64_t pins) {
    uint64_t pip = cpu->pip;
    // wait cycle?
    if ((pip & Z80_PIP_BIT_WAIT) && (pins & Z80_WAIT)) {
        cpu->pins = pins & ~Z80_CTRL_PIN_MASK;
        return pins;
    }
    // check if new opcode must be loaded from data bus
    if ((pins & (Z80_M1|Z80_MREQ|Z80_RD)) == (Z80_M1|Z80_MREQ|Z80_RD)) {
        uint8_t opcode = _gd();
        cpu->ir = opcode<<3;
        pip = z80_pip_table[opcode];
    }
    // process the next 'active' tcycle
    pins &= ~Z80_CTRL_PIN_MASK;
    if (pip & Z80_PIP_BIT_STEP) {
        switch (cpu->ir++) {
$decode_block
        }
    }
    cpu->pip = (pip & ~Z80_PIP_BITS) >> 1;
    cpu->pins = pins;
    return pins;
}

#undef _gaf
#undef _gbc
#undef _gde
#undef _ghl
#undef _gsp
#undef _saf
#undef _sbc
#undef _sde
#undef _shl
#undef _ssp
#undef _sa
#undef _sax
#undef _sad
#undef _sadx
#undef _gd
#undef _fetch
#undef _rfsh
#undef _mr
#undef _mw
#undef _ior
#undef _iow
#undef _add
#undef _adc
#undef _sub
#undef _sbc
#undef _and
#undef _xor
#undef _or
#undef _cp

#endif // CHIPS_IMPL