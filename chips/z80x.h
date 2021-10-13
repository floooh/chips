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
#define Z80_WAIT  (1ULL<<34)        // wait requested

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

// machine cycle execution pipeline bits (TODO: explain this stuff)
#define Z80_PIP_BIT_STEP        (1<<0)  // step the instruction decoder forward
#define Z80_PIP_BIT_LOADIR      (1<<8)  // load the next opcode from data bus
#define Z80_PIP_BIT_WAIT        (1<<16) // sample the wait pin
#define Z80_PIP_BIT_IRQ         (1<<24) // handle interrupt request

#define Z80_PIP_BITS (Z80_PIP_BIT_STEP|Z80_PIP_BIT_LOADIR|Z80_PIP_BIT_WAIT|Z80_PIP_BIT_IRQ)

#define Z80_PIP_MASK_STEP       (0xFF)
#define Z80_PIP_MASK_LOADIR     (0xFF00)
#define Z80_PIP_MASK_WAIT       (0xFF0000)
#define Z80_PIP_MASK_IRQ        (0xFF000000)

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
    pins &= ~Z80_CTRL_MASK;
    if ((cpu->pip & Z80_PIP_BIT_WAIT) && (pins & Z80_WAIT)) {
        cpu->pins = pins;
        return pins;
    }
    cpu->pip = (cpu->pip & ~Z80_PIP_BITS) >> 1;
    if (cpu->pip & Z80_PIP_BIT_LOADIR) {
        // load next instruction byte from data bus into instruction
        // register, make room for machine cycle counter
        cpu->ir = _gd()<<3;
    }
    // early out if there's nothing else to do this tick
    if (0 == (cpu->pip & Z80_PIP_BIT_STEP)) {
        cpu->pins = pins;
        return pins;
    }
    // process the next 'active' tcycle
    switch (cpu->ir++) {
        
        // NOP
        // -- M1
        case (0x00<<3)|0: cpu->pip=0xB; break;
        case (0x00<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x00<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // LD BC,nn
        // -- M1
        case (0x01<<3)|0: cpu->pip=0xB; break;
        case (0x01<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x01<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // LD (BC),A
        // -- M1
        case (0x02<<3)|0: cpu->pip=0xB; break;
        case (0x02<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x02<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // INC BC
        // -- M1
        case (0x03<<3)|0: cpu->pip=0xB; break;
        case (0x03<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x03<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // INC B
        // -- M1
        case (0x04<<3)|0: cpu->pip=0xB; break;
        case (0x04<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x04<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // DEC B
        // -- M1
        case (0x05<<3)|0: cpu->pip=0xB; break;
        case (0x05<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x05<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // LD B,n
        // -- M1
        case (0x06<<3)|0: cpu->pip=0xB; break;
        case (0x06<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x06<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // RLCA
        // -- M1
        case (0x07<<3)|0: cpu->pip=0xB; break;
        case (0x07<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x07<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // EX AF,AF'
        // -- M1
        case (0x08<<3)|0: cpu->pip=0xB; break;
        case (0x08<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x08<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // ADD HL,BC
        // -- M1
        case (0x09<<3)|0: cpu->pip=0xB; break;
        case (0x09<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x09<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // LD A,(BC)
        // -- M1
        case (0x0A<<3)|0: cpu->pip=0xB; break;
        case (0x0A<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x0A<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // DEC BC
        // -- M1
        case (0x0B<<3)|0: cpu->pip=0xB; break;
        case (0x0B<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x0B<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // INC C
        // -- M1
        case (0x0C<<3)|0: cpu->pip=0xB; break;
        case (0x0C<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x0C<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // DEC C
        // -- M1
        case (0x0D<<3)|0: cpu->pip=0xB; break;
        case (0x0D<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x0D<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // LD C,n
        // -- M1
        case (0x0E<<3)|0: cpu->pip=0xB; break;
        case (0x0E<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x0E<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // RRCA
        // -- M1
        case (0x0F<<3)|0: cpu->pip=0xB; break;
        case (0x0F<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x0F<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // DJNZ d
        // -- M1
        case (0x10<<3)|0: cpu->pip=0xB; break;
        case (0x10<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x10<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // LD DE,nn
        // -- M1
        case (0x11<<3)|0: cpu->pip=0xB; break;
        case (0x11<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x11<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // LD (DE),A
        // -- M1
        case (0x12<<3)|0: cpu->pip=0xB; break;
        case (0x12<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x12<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // INC DE
        // -- M1
        case (0x13<<3)|0: cpu->pip=0xB; break;
        case (0x13<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x13<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // INC D
        // -- M1
        case (0x14<<3)|0: cpu->pip=0xB; break;
        case (0x14<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x14<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // DEC D
        // -- M1
        case (0x15<<3)|0: cpu->pip=0xB; break;
        case (0x15<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x15<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // LD D,n
        // -- M1
        case (0x16<<3)|0: cpu->pip=0xB; break;
        case (0x16<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x16<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // RLA
        // -- M1
        case (0x17<<3)|0: cpu->pip=0xB; break;
        case (0x17<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x17<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // JR d
        // -- M1
        case (0x18<<3)|0: cpu->pip=0xB; break;
        case (0x18<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x18<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // ADD HL,DE
        // -- M1
        case (0x19<<3)|0: cpu->pip=0xB; break;
        case (0x19<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x19<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // LD A,(DE)
        // -- M1
        case (0x1A<<3)|0: cpu->pip=0xB; break;
        case (0x1A<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x1A<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // DEC DE
        // -- M1
        case (0x1B<<3)|0: cpu->pip=0xB; break;
        case (0x1B<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x1B<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // INC E
        // -- M1
        case (0x1C<<3)|0: cpu->pip=0xB; break;
        case (0x1C<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x1C<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // DEC E
        // -- M1
        case (0x1D<<3)|0: cpu->pip=0xB; break;
        case (0x1D<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x1D<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // LD E,n
        // -- M1
        case (0x1E<<3)|0: cpu->pip=0xB; break;
        case (0x1E<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x1E<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // RRA
        // -- M1
        case (0x1F<<3)|0: cpu->pip=0xB; break;
        case (0x1F<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x1F<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // JR NZ,d
        // -- M1
        case (0x20<<3)|0: cpu->pip=0xB; break;
        case (0x20<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x20<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // LD HL,nn
        // -- M1
        case (0x21<<3)|0: cpu->pip=0xB; break;
        case (0x21<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x21<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // LD (nn),HL
        // -- M1
        case (0x22<<3)|0: cpu->pip=0xB; break;
        case (0x22<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x22<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // INC HL
        // -- M1
        case (0x23<<3)|0: cpu->pip=0xB; break;
        case (0x23<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x23<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // INC H
        // -- M1
        case (0x24<<3)|0: cpu->pip=0xB; break;
        case (0x24<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x24<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // DEC H
        // -- M1
        case (0x25<<3)|0: cpu->pip=0xB; break;
        case (0x25<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x25<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // LD H,n
        // -- M1
        case (0x26<<3)|0: cpu->pip=0xB; break;
        case (0x26<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x26<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // DAA
        // -- M1
        case (0x27<<3)|0: cpu->pip=0xB; break;
        case (0x27<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x27<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // JR Z,d
        // -- M1
        case (0x28<<3)|0: cpu->pip=0xB; break;
        case (0x28<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x28<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // ADD HL,HL
        // -- M1
        case (0x29<<3)|0: cpu->pip=0xB; break;
        case (0x29<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x29<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // LD HL,(nn)
        // -- M1
        case (0x2A<<3)|0: cpu->pip=0xB; break;
        case (0x2A<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x2A<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // DEC HL
        // -- M1
        case (0x2B<<3)|0: cpu->pip=0xB; break;
        case (0x2B<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x2B<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // INC L
        // -- M1
        case (0x2C<<3)|0: cpu->pip=0xB; break;
        case (0x2C<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x2C<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // DEC L
        // -- M1
        case (0x2D<<3)|0: cpu->pip=0xB; break;
        case (0x2D<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x2D<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // LD L,n
        // -- M1
        case (0x2E<<3)|0: cpu->pip=0xB; break;
        case (0x2E<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x2E<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // CPL
        // -- M1
        case (0x2F<<3)|0: cpu->pip=0xB; break;
        case (0x2F<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x2F<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // JR NC,d
        // -- M1
        case (0x30<<3)|0: cpu->pip=0xB; break;
        case (0x30<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x30<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // LD SP,nn
        // -- M1
        case (0x31<<3)|0: cpu->pip=0xB; break;
        case (0x31<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x31<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // LD (nn),A
        // -- M1
        case (0x32<<3)|0: cpu->pip=0xB; break;
        case (0x32<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x32<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // INC SP
        // -- M1
        case (0x33<<3)|0: cpu->pip=0xB; break;
        case (0x33<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x33<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // INC (HL)
        // -- M1
        case (0x34<<3)|0: cpu->pip=0xB; break;
        case (0x34<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x34<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // DEC (HL)
        // -- M1
        case (0x35<<3)|0: cpu->pip=0xB; break;
        case (0x35<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x35<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // LD (HL),n
        // -- M1
        case (0x36<<3)|0: cpu->pip=0xB; break;
        case (0x36<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x36<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // SCF
        // -- M1
        case (0x37<<3)|0: cpu->pip=0xB; break;
        case (0x37<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x37<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // JR C,d
        // -- M1
        case (0x38<<3)|0: cpu->pip=0xB; break;
        case (0x38<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x38<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // ADD HL,SP
        // -- M1
        case (0x39<<3)|0: cpu->pip=0xB; break;
        case (0x39<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x39<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // LD A,(nn)
        // -- M1
        case (0x3A<<3)|0: cpu->pip=0xB; break;
        case (0x3A<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x3A<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // DEC SP
        // -- M1
        case (0x3B<<3)|0: cpu->pip=0xB; break;
        case (0x3B<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x3B<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // INC A
        // -- M1
        case (0x3C<<3)|0: cpu->pip=0xB; break;
        case (0x3C<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x3C<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // DEC A
        // -- M1
        case (0x3D<<3)|0: cpu->pip=0xB; break;
        case (0x3D<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x3D<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // LD A,n
        // -- M1
        case (0x3E<<3)|0: cpu->pip=0xB; break;
        case (0x3E<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x3E<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // CCF
        // -- M1
        case (0x3F<<3)|0: cpu->pip=0xB; break;
        case (0x3F<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x3F<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // LD B,B
        // -- M1
        case (0x40<<3)|0: cpu->pip=0xB; break;
        case (0x40<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x40<<3)|2: cpu->pip=0x20203;_fetch();cpu->b=cpu->b; break;
        
        // LD C,B
        // -- M1
        case (0x41<<3)|0: cpu->pip=0xB; break;
        case (0x41<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x41<<3)|2: cpu->pip=0x20203;_fetch();cpu->b=cpu->c; break;
        
        // LD D,B
        // -- M1
        case (0x42<<3)|0: cpu->pip=0xB; break;
        case (0x42<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x42<<3)|2: cpu->pip=0x20203;_fetch();cpu->b=cpu->d; break;
        
        // LD E,B
        // -- M1
        case (0x43<<3)|0: cpu->pip=0xB; break;
        case (0x43<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x43<<3)|2: cpu->pip=0x20203;_fetch();cpu->b=cpu->e; break;
        
        // LD H,B
        // -- M1
        case (0x44<<3)|0: cpu->pip=0xB; break;
        case (0x44<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x44<<3)|2: cpu->pip=0x20203;_fetch();cpu->b=cpu->h; break;
        
        // LD L,B
        // -- M1
        case (0x45<<3)|0: cpu->pip=0xB; break;
        case (0x45<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x45<<3)|2: cpu->pip=0x20203;_fetch();cpu->b=cpu->l; break;
        
        // LD B,(HL)
        // -- M1
        case (0x46<<3)|0: cpu->pip=0xB; break;
        case (0x46<<3)|1: _rfsh(); break;
        // -- M2
        case (0x46<<3)|2: cpu->pip=0x2000B;_mr(_ghl()); break;
        case (0x46<<3)|3: cpu->b=_gd(); break;
        // -- OVERLAP
        case (0x46<<3)|4: cpu->pip=0x20203;_fetch();; break;
        
        // LD A,B
        // -- M1
        case (0x47<<3)|0: cpu->pip=0xB; break;
        case (0x47<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x47<<3)|2: cpu->pip=0x20203;_fetch();cpu->b=cpu->a; break;
        
        // LD B,C
        // -- M1
        case (0x48<<3)|0: cpu->pip=0xB; break;
        case (0x48<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x48<<3)|2: cpu->pip=0x20203;_fetch();cpu->c=cpu->b; break;
        
        // LD C,C
        // -- M1
        case (0x49<<3)|0: cpu->pip=0xB; break;
        case (0x49<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x49<<3)|2: cpu->pip=0x20203;_fetch();cpu->c=cpu->c; break;
        
        // LD D,C
        // -- M1
        case (0x4A<<3)|0: cpu->pip=0xB; break;
        case (0x4A<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x4A<<3)|2: cpu->pip=0x20203;_fetch();cpu->c=cpu->d; break;
        
        // LD E,C
        // -- M1
        case (0x4B<<3)|0: cpu->pip=0xB; break;
        case (0x4B<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x4B<<3)|2: cpu->pip=0x20203;_fetch();cpu->c=cpu->e; break;
        
        // LD H,C
        // -- M1
        case (0x4C<<3)|0: cpu->pip=0xB; break;
        case (0x4C<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x4C<<3)|2: cpu->pip=0x20203;_fetch();cpu->c=cpu->h; break;
        
        // LD L,C
        // -- M1
        case (0x4D<<3)|0: cpu->pip=0xB; break;
        case (0x4D<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x4D<<3)|2: cpu->pip=0x20203;_fetch();cpu->c=cpu->l; break;
        
        // LD C,(HL)
        // -- M1
        case (0x4E<<3)|0: cpu->pip=0xB; break;
        case (0x4E<<3)|1: _rfsh(); break;
        // -- M2
        case (0x4E<<3)|2: cpu->pip=0x2000B;_mr(_ghl()); break;
        case (0x4E<<3)|3: cpu->c=_gd(); break;
        // -- OVERLAP
        case (0x4E<<3)|4: cpu->pip=0x20203;_fetch();; break;
        
        // LD A,C
        // -- M1
        case (0x4F<<3)|0: cpu->pip=0xB; break;
        case (0x4F<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x4F<<3)|2: cpu->pip=0x20203;_fetch();cpu->c=cpu->a; break;
        
        // LD B,D
        // -- M1
        case (0x50<<3)|0: cpu->pip=0xB; break;
        case (0x50<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x50<<3)|2: cpu->pip=0x20203;_fetch();cpu->d=cpu->b; break;
        
        // LD C,D
        // -- M1
        case (0x51<<3)|0: cpu->pip=0xB; break;
        case (0x51<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x51<<3)|2: cpu->pip=0x20203;_fetch();cpu->d=cpu->c; break;
        
        // LD D,D
        // -- M1
        case (0x52<<3)|0: cpu->pip=0xB; break;
        case (0x52<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x52<<3)|2: cpu->pip=0x20203;_fetch();cpu->d=cpu->d; break;
        
        // LD E,D
        // -- M1
        case (0x53<<3)|0: cpu->pip=0xB; break;
        case (0x53<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x53<<3)|2: cpu->pip=0x20203;_fetch();cpu->d=cpu->e; break;
        
        // LD H,D
        // -- M1
        case (0x54<<3)|0: cpu->pip=0xB; break;
        case (0x54<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x54<<3)|2: cpu->pip=0x20203;_fetch();cpu->d=cpu->h; break;
        
        // LD L,D
        // -- M1
        case (0x55<<3)|0: cpu->pip=0xB; break;
        case (0x55<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x55<<3)|2: cpu->pip=0x20203;_fetch();cpu->d=cpu->l; break;
        
        // LD D,(HL)
        // -- M1
        case (0x56<<3)|0: cpu->pip=0xB; break;
        case (0x56<<3)|1: _rfsh(); break;
        // -- M2
        case (0x56<<3)|2: cpu->pip=0x2000B;_mr(_ghl()); break;
        case (0x56<<3)|3: cpu->d=_gd(); break;
        // -- OVERLAP
        case (0x56<<3)|4: cpu->pip=0x20203;_fetch();; break;
        
        // LD A,D
        // -- M1
        case (0x57<<3)|0: cpu->pip=0xB; break;
        case (0x57<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x57<<3)|2: cpu->pip=0x20203;_fetch();cpu->d=cpu->a; break;
        
        // LD B,E
        // -- M1
        case (0x58<<3)|0: cpu->pip=0xB; break;
        case (0x58<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x58<<3)|2: cpu->pip=0x20203;_fetch();cpu->e=cpu->b; break;
        
        // LD C,E
        // -- M1
        case (0x59<<3)|0: cpu->pip=0xB; break;
        case (0x59<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x59<<3)|2: cpu->pip=0x20203;_fetch();cpu->e=cpu->c; break;
        
        // LD D,E
        // -- M1
        case (0x5A<<3)|0: cpu->pip=0xB; break;
        case (0x5A<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x5A<<3)|2: cpu->pip=0x20203;_fetch();cpu->e=cpu->d; break;
        
        // LD E,E
        // -- M1
        case (0x5B<<3)|0: cpu->pip=0xB; break;
        case (0x5B<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x5B<<3)|2: cpu->pip=0x20203;_fetch();cpu->e=cpu->e; break;
        
        // LD H,E
        // -- M1
        case (0x5C<<3)|0: cpu->pip=0xB; break;
        case (0x5C<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x5C<<3)|2: cpu->pip=0x20203;_fetch();cpu->e=cpu->h; break;
        
        // LD L,E
        // -- M1
        case (0x5D<<3)|0: cpu->pip=0xB; break;
        case (0x5D<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x5D<<3)|2: cpu->pip=0x20203;_fetch();cpu->e=cpu->l; break;
        
        // LD E,(HL)
        // -- M1
        case (0x5E<<3)|0: cpu->pip=0xB; break;
        case (0x5E<<3)|1: _rfsh(); break;
        // -- M2
        case (0x5E<<3)|2: cpu->pip=0x2000B;_mr(_ghl()); break;
        case (0x5E<<3)|3: cpu->e=_gd(); break;
        // -- OVERLAP
        case (0x5E<<3)|4: cpu->pip=0x20203;_fetch();; break;
        
        // LD A,E
        // -- M1
        case (0x5F<<3)|0: cpu->pip=0xB; break;
        case (0x5F<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x5F<<3)|2: cpu->pip=0x20203;_fetch();cpu->e=cpu->a; break;
        
        // LD B,H
        // -- M1
        case (0x60<<3)|0: cpu->pip=0xB; break;
        case (0x60<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x60<<3)|2: cpu->pip=0x20203;_fetch();cpu->h=cpu->b; break;
        
        // LD C,H
        // -- M1
        case (0x61<<3)|0: cpu->pip=0xB; break;
        case (0x61<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x61<<3)|2: cpu->pip=0x20203;_fetch();cpu->h=cpu->c; break;
        
        // LD D,H
        // -- M1
        case (0x62<<3)|0: cpu->pip=0xB; break;
        case (0x62<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x62<<3)|2: cpu->pip=0x20203;_fetch();cpu->h=cpu->d; break;
        
        // LD E,H
        // -- M1
        case (0x63<<3)|0: cpu->pip=0xB; break;
        case (0x63<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x63<<3)|2: cpu->pip=0x20203;_fetch();cpu->h=cpu->e; break;
        
        // LD H,H
        // -- M1
        case (0x64<<3)|0: cpu->pip=0xB; break;
        case (0x64<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x64<<3)|2: cpu->pip=0x20203;_fetch();cpu->h=cpu->h; break;
        
        // LD L,H
        // -- M1
        case (0x65<<3)|0: cpu->pip=0xB; break;
        case (0x65<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x65<<3)|2: cpu->pip=0x20203;_fetch();cpu->h=cpu->l; break;
        
        // LD H,(HL)
        // -- M1
        case (0x66<<3)|0: cpu->pip=0xB; break;
        case (0x66<<3)|1: _rfsh(); break;
        // -- M2
        case (0x66<<3)|2: cpu->pip=0x2000B;_mr(_ghl()); break;
        case (0x66<<3)|3: cpu->h=_gd(); break;
        // -- OVERLAP
        case (0x66<<3)|4: cpu->pip=0x20203;_fetch();; break;
        
        // LD A,H
        // -- M1
        case (0x67<<3)|0: cpu->pip=0xB; break;
        case (0x67<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x67<<3)|2: cpu->pip=0x20203;_fetch();cpu->h=cpu->a; break;
        
        // LD B,L
        // -- M1
        case (0x68<<3)|0: cpu->pip=0xB; break;
        case (0x68<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x68<<3)|2: cpu->pip=0x20203;_fetch();cpu->l=cpu->b; break;
        
        // LD C,L
        // -- M1
        case (0x69<<3)|0: cpu->pip=0xB; break;
        case (0x69<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x69<<3)|2: cpu->pip=0x20203;_fetch();cpu->l=cpu->c; break;
        
        // LD D,L
        // -- M1
        case (0x6A<<3)|0: cpu->pip=0xB; break;
        case (0x6A<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x6A<<3)|2: cpu->pip=0x20203;_fetch();cpu->l=cpu->d; break;
        
        // LD E,L
        // -- M1
        case (0x6B<<3)|0: cpu->pip=0xB; break;
        case (0x6B<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x6B<<3)|2: cpu->pip=0x20203;_fetch();cpu->l=cpu->e; break;
        
        // LD H,L
        // -- M1
        case (0x6C<<3)|0: cpu->pip=0xB; break;
        case (0x6C<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x6C<<3)|2: cpu->pip=0x20203;_fetch();cpu->l=cpu->h; break;
        
        // LD L,L
        // -- M1
        case (0x6D<<3)|0: cpu->pip=0xB; break;
        case (0x6D<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x6D<<3)|2: cpu->pip=0x20203;_fetch();cpu->l=cpu->l; break;
        
        // LD L,(HL)
        // -- M1
        case (0x6E<<3)|0: cpu->pip=0xB; break;
        case (0x6E<<3)|1: _rfsh(); break;
        // -- M2
        case (0x6E<<3)|2: cpu->pip=0x2000B;_mr(_ghl()); break;
        case (0x6E<<3)|3: cpu->l=_gd(); break;
        // -- OVERLAP
        case (0x6E<<3)|4: cpu->pip=0x20203;_fetch();; break;
        
        // LD A,L
        // -- M1
        case (0x6F<<3)|0: cpu->pip=0xB; break;
        case (0x6F<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x6F<<3)|2: cpu->pip=0x20203;_fetch();cpu->l=cpu->a; break;
        
        // LD (HL),B
        // -- M1
        case (0x70<<3)|0: cpu->pip=0xB; break;
        case (0x70<<3)|1: _rfsh(); break;
        // -- M2
        // -- OVERLAP
        case (0x70<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // LD (HL),C
        // -- M1
        case (0x71<<3)|0: cpu->pip=0xB; break;
        case (0x71<<3)|1: _rfsh(); break;
        // -- M2
        // -- OVERLAP
        case (0x71<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // LD (HL),D
        // -- M1
        case (0x72<<3)|0: cpu->pip=0xB; break;
        case (0x72<<3)|1: _rfsh(); break;
        // -- M2
        // -- OVERLAP
        case (0x72<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // LD (HL),E
        // -- M1
        case (0x73<<3)|0: cpu->pip=0xB; break;
        case (0x73<<3)|1: _rfsh(); break;
        // -- M2
        // -- OVERLAP
        case (0x73<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // LD (HL),H
        // -- M1
        case (0x74<<3)|0: cpu->pip=0xB; break;
        case (0x74<<3)|1: _rfsh(); break;
        // -- M2
        // -- OVERLAP
        case (0x74<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // LD (HL),L
        // -- M1
        case (0x75<<3)|0: cpu->pip=0xB; break;
        case (0x75<<3)|1: _rfsh(); break;
        // -- M2
        // -- OVERLAP
        case (0x75<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // HALT
        // -- M1
        case (0x76<<3)|0: cpu->pip=0xB; break;
        case (0x76<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x76<<3)|2: cpu->pip=0x20203;_fetch();z80_halt(c); break;
        
        // LD (HL),A
        // -- M1
        case (0x77<<3)|0: cpu->pip=0xB; break;
        case (0x77<<3)|1: _rfsh(); break;
        // -- M2
        // -- OVERLAP
        case (0x77<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // LD B,A
        // -- M1
        case (0x78<<3)|0: cpu->pip=0xB; break;
        case (0x78<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x78<<3)|2: cpu->pip=0x20203;_fetch();cpu->a=cpu->b; break;
        
        // LD C,A
        // -- M1
        case (0x79<<3)|0: cpu->pip=0xB; break;
        case (0x79<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x79<<3)|2: cpu->pip=0x20203;_fetch();cpu->a=cpu->c; break;
        
        // LD D,A
        // -- M1
        case (0x7A<<3)|0: cpu->pip=0xB; break;
        case (0x7A<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x7A<<3)|2: cpu->pip=0x20203;_fetch();cpu->a=cpu->d; break;
        
        // LD E,A
        // -- M1
        case (0x7B<<3)|0: cpu->pip=0xB; break;
        case (0x7B<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x7B<<3)|2: cpu->pip=0x20203;_fetch();cpu->a=cpu->e; break;
        
        // LD H,A
        // -- M1
        case (0x7C<<3)|0: cpu->pip=0xB; break;
        case (0x7C<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x7C<<3)|2: cpu->pip=0x20203;_fetch();cpu->a=cpu->h; break;
        
        // LD L,A
        // -- M1
        case (0x7D<<3)|0: cpu->pip=0xB; break;
        case (0x7D<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x7D<<3)|2: cpu->pip=0x20203;_fetch();cpu->a=cpu->l; break;
        
        // LD A,(HL)
        // -- M1
        case (0x7E<<3)|0: cpu->pip=0xB; break;
        case (0x7E<<3)|1: _rfsh(); break;
        // -- M2
        case (0x7E<<3)|2: cpu->pip=0x2000B;_mr(_ghl()); break;
        case (0x7E<<3)|3: cpu->a=_gd(); break;
        // -- OVERLAP
        case (0x7E<<3)|4: cpu->pip=0x20203;_fetch();; break;
        
        // LD A,A
        // -- M1
        case (0x7F<<3)|0: cpu->pip=0xB; break;
        case (0x7F<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x7F<<3)|2: cpu->pip=0x20203;_fetch();cpu->a=cpu->a; break;
        
        // ADD B
        // -- M1
        case (0x80<<3)|0: cpu->pip=0xB; break;
        case (0x80<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x80<<3)|2: cpu->pip=0x20203;_fetch();_add(cpu->b); break;
        
        // ADD C
        // -- M1
        case (0x81<<3)|0: cpu->pip=0xB; break;
        case (0x81<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x81<<3)|2: cpu->pip=0x20203;_fetch();_add(cpu->c); break;
        
        // ADD D
        // -- M1
        case (0x82<<3)|0: cpu->pip=0xB; break;
        case (0x82<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x82<<3)|2: cpu->pip=0x20203;_fetch();_add(cpu->d); break;
        
        // ADD E
        // -- M1
        case (0x83<<3)|0: cpu->pip=0xB; break;
        case (0x83<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x83<<3)|2: cpu->pip=0x20203;_fetch();_add(cpu->e); break;
        
        // ADD H
        // -- M1
        case (0x84<<3)|0: cpu->pip=0xB; break;
        case (0x84<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x84<<3)|2: cpu->pip=0x20203;_fetch();_add(cpu->h); break;
        
        // ADD L
        // -- M1
        case (0x85<<3)|0: cpu->pip=0xB; break;
        case (0x85<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x85<<3)|2: cpu->pip=0x20203;_fetch();_add(cpu->l); break;
        
        // ADD (HL)
        // -- M1
        case (0x86<<3)|0: cpu->pip=0xB; break;
        case (0x86<<3)|1: _rfsh(); break;
        // -- M2
        case (0x86<<3)|2: cpu->pip=0x2000B;_mr(_ghl()); break;
        case (0x86<<3)|3: cpu->dlatch=_gd(); break;
        // -- OVERLAP
        case (0x86<<3)|4: cpu->pip=0x20203;_fetch();_add(cpu->dlatch); break;
        
        // ADD A
        // -- M1
        case (0x87<<3)|0: cpu->pip=0xB; break;
        case (0x87<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x87<<3)|2: cpu->pip=0x20203;_fetch();_add(cpu->a); break;
        
        // ADC B
        // -- M1
        case (0x88<<3)|0: cpu->pip=0xB; break;
        case (0x88<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x88<<3)|2: cpu->pip=0x20203;_fetch();_adc(cpu->b); break;
        
        // ADC C
        // -- M1
        case (0x89<<3)|0: cpu->pip=0xB; break;
        case (0x89<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x89<<3)|2: cpu->pip=0x20203;_fetch();_adc(cpu->c); break;
        
        // ADC D
        // -- M1
        case (0x8A<<3)|0: cpu->pip=0xB; break;
        case (0x8A<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x8A<<3)|2: cpu->pip=0x20203;_fetch();_adc(cpu->d); break;
        
        // ADC E
        // -- M1
        case (0x8B<<3)|0: cpu->pip=0xB; break;
        case (0x8B<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x8B<<3)|2: cpu->pip=0x20203;_fetch();_adc(cpu->e); break;
        
        // ADC H
        // -- M1
        case (0x8C<<3)|0: cpu->pip=0xB; break;
        case (0x8C<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x8C<<3)|2: cpu->pip=0x20203;_fetch();_adc(cpu->h); break;
        
        // ADC L
        // -- M1
        case (0x8D<<3)|0: cpu->pip=0xB; break;
        case (0x8D<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x8D<<3)|2: cpu->pip=0x20203;_fetch();_adc(cpu->l); break;
        
        // ADC (HL)
        // -- M1
        case (0x8E<<3)|0: cpu->pip=0xB; break;
        case (0x8E<<3)|1: _rfsh(); break;
        // -- M2
        case (0x8E<<3)|2: cpu->pip=0x2000B;_mr(_ghl()); break;
        case (0x8E<<3)|3: cpu->dlatch=_gd(); break;
        // -- OVERLAP
        case (0x8E<<3)|4: cpu->pip=0x20203;_fetch();_adc(cpu->dlatch); break;
        
        // ADC A
        // -- M1
        case (0x8F<<3)|0: cpu->pip=0xB; break;
        case (0x8F<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x8F<<3)|2: cpu->pip=0x20203;_fetch();_adc(cpu->a); break;
        
        // SUB B
        // -- M1
        case (0x90<<3)|0: cpu->pip=0xB; break;
        case (0x90<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x90<<3)|2: cpu->pip=0x20203;_fetch();_sub(cpu->b); break;
        
        // SUB C
        // -- M1
        case (0x91<<3)|0: cpu->pip=0xB; break;
        case (0x91<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x91<<3)|2: cpu->pip=0x20203;_fetch();_sub(cpu->c); break;
        
        // SUB D
        // -- M1
        case (0x92<<3)|0: cpu->pip=0xB; break;
        case (0x92<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x92<<3)|2: cpu->pip=0x20203;_fetch();_sub(cpu->d); break;
        
        // SUB E
        // -- M1
        case (0x93<<3)|0: cpu->pip=0xB; break;
        case (0x93<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x93<<3)|2: cpu->pip=0x20203;_fetch();_sub(cpu->e); break;
        
        // SUB H
        // -- M1
        case (0x94<<3)|0: cpu->pip=0xB; break;
        case (0x94<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x94<<3)|2: cpu->pip=0x20203;_fetch();_sub(cpu->h); break;
        
        // SUB L
        // -- M1
        case (0x95<<3)|0: cpu->pip=0xB; break;
        case (0x95<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x95<<3)|2: cpu->pip=0x20203;_fetch();_sub(cpu->l); break;
        
        // SUB (HL)
        // -- M1
        case (0x96<<3)|0: cpu->pip=0xB; break;
        case (0x96<<3)|1: _rfsh(); break;
        // -- M2
        case (0x96<<3)|2: cpu->pip=0x2000B;_mr(_ghl()); break;
        case (0x96<<3)|3: cpu->dlatch=_gd(); break;
        // -- OVERLAP
        case (0x96<<3)|4: cpu->pip=0x20203;_fetch();_sub(cpu->dlatch); break;
        
        // SUB A
        // -- M1
        case (0x97<<3)|0: cpu->pip=0xB; break;
        case (0x97<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x97<<3)|2: cpu->pip=0x20203;_fetch();_sub(cpu->a); break;
        
        // SBC B
        // -- M1
        case (0x98<<3)|0: cpu->pip=0xB; break;
        case (0x98<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x98<<3)|2: cpu->pip=0x20203;_fetch();_s_gbc()(cpu->b); break;
        
        // SBC C
        // -- M1
        case (0x99<<3)|0: cpu->pip=0xB; break;
        case (0x99<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x99<<3)|2: cpu->pip=0x20203;_fetch();_s_gbc()(cpu->c); break;
        
        // SBC D
        // -- M1
        case (0x9A<<3)|0: cpu->pip=0xB; break;
        case (0x9A<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x9A<<3)|2: cpu->pip=0x20203;_fetch();_s_gbc()(cpu->d); break;
        
        // SBC E
        // -- M1
        case (0x9B<<3)|0: cpu->pip=0xB; break;
        case (0x9B<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x9B<<3)|2: cpu->pip=0x20203;_fetch();_s_gbc()(cpu->e); break;
        
        // SBC H
        // -- M1
        case (0x9C<<3)|0: cpu->pip=0xB; break;
        case (0x9C<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x9C<<3)|2: cpu->pip=0x20203;_fetch();_s_gbc()(cpu->h); break;
        
        // SBC L
        // -- M1
        case (0x9D<<3)|0: cpu->pip=0xB; break;
        case (0x9D<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x9D<<3)|2: cpu->pip=0x20203;_fetch();_s_gbc()(cpu->l); break;
        
        // SBC (HL)
        // -- M1
        case (0x9E<<3)|0: cpu->pip=0xB; break;
        case (0x9E<<3)|1: _rfsh(); break;
        // -- M2
        case (0x9E<<3)|2: cpu->pip=0x2000B;_mr(_ghl()); break;
        case (0x9E<<3)|3: cpu->dlatch=_gd(); break;
        // -- OVERLAP
        case (0x9E<<3)|4: cpu->pip=0x20203;_fetch();_s_gbc()(cpu->dlatch); break;
        
        // SBC A
        // -- M1
        case (0x9F<<3)|0: cpu->pip=0xB; break;
        case (0x9F<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0x9F<<3)|2: cpu->pip=0x20203;_fetch();_s_gbc()(cpu->a); break;
        
        // AND B
        // -- M1
        case (0xA0<<3)|0: cpu->pip=0xB; break;
        case (0xA0<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xA0<<3)|2: cpu->pip=0x20203;_fetch();_and(cpu->b); break;
        
        // AND C
        // -- M1
        case (0xA1<<3)|0: cpu->pip=0xB; break;
        case (0xA1<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xA1<<3)|2: cpu->pip=0x20203;_fetch();_and(cpu->c); break;
        
        // AND D
        // -- M1
        case (0xA2<<3)|0: cpu->pip=0xB; break;
        case (0xA2<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xA2<<3)|2: cpu->pip=0x20203;_fetch();_and(cpu->d); break;
        
        // AND E
        // -- M1
        case (0xA3<<3)|0: cpu->pip=0xB; break;
        case (0xA3<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xA3<<3)|2: cpu->pip=0x20203;_fetch();_and(cpu->e); break;
        
        // AND H
        // -- M1
        case (0xA4<<3)|0: cpu->pip=0xB; break;
        case (0xA4<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xA4<<3)|2: cpu->pip=0x20203;_fetch();_and(cpu->h); break;
        
        // AND L
        // -- M1
        case (0xA5<<3)|0: cpu->pip=0xB; break;
        case (0xA5<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xA5<<3)|2: cpu->pip=0x20203;_fetch();_and(cpu->l); break;
        
        // AND (HL)
        // -- M1
        case (0xA6<<3)|0: cpu->pip=0xB; break;
        case (0xA6<<3)|1: _rfsh(); break;
        // -- M2
        case (0xA6<<3)|2: cpu->pip=0x2000B;_mr(_ghl()); break;
        case (0xA6<<3)|3: cpu->dlatch=_gd(); break;
        // -- OVERLAP
        case (0xA6<<3)|4: cpu->pip=0x20203;_fetch();_and(cpu->dlatch); break;
        
        // AND A
        // -- M1
        case (0xA7<<3)|0: cpu->pip=0xB; break;
        case (0xA7<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xA7<<3)|2: cpu->pip=0x20203;_fetch();_and(cpu->a); break;
        
        // XOR B
        // -- M1
        case (0xA8<<3)|0: cpu->pip=0xB; break;
        case (0xA8<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xA8<<3)|2: cpu->pip=0x20203;_fetch();_xor(cpu->b); break;
        
        // XOR C
        // -- M1
        case (0xA9<<3)|0: cpu->pip=0xB; break;
        case (0xA9<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xA9<<3)|2: cpu->pip=0x20203;_fetch();_xor(cpu->c); break;
        
        // XOR D
        // -- M1
        case (0xAA<<3)|0: cpu->pip=0xB; break;
        case (0xAA<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xAA<<3)|2: cpu->pip=0x20203;_fetch();_xor(cpu->d); break;
        
        // XOR E
        // -- M1
        case (0xAB<<3)|0: cpu->pip=0xB; break;
        case (0xAB<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xAB<<3)|2: cpu->pip=0x20203;_fetch();_xor(cpu->e); break;
        
        // XOR H
        // -- M1
        case (0xAC<<3)|0: cpu->pip=0xB; break;
        case (0xAC<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xAC<<3)|2: cpu->pip=0x20203;_fetch();_xor(cpu->h); break;
        
        // XOR L
        // -- M1
        case (0xAD<<3)|0: cpu->pip=0xB; break;
        case (0xAD<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xAD<<3)|2: cpu->pip=0x20203;_fetch();_xor(cpu->l); break;
        
        // XOR (HL)
        // -- M1
        case (0xAE<<3)|0: cpu->pip=0xB; break;
        case (0xAE<<3)|1: _rfsh(); break;
        // -- M2
        case (0xAE<<3)|2: cpu->pip=0x2000B;_mr(_ghl()); break;
        case (0xAE<<3)|3: cpu->dlatch=_gd(); break;
        // -- OVERLAP
        case (0xAE<<3)|4: cpu->pip=0x20203;_fetch();_xor(cpu->dlatch); break;
        
        // XOR A
        // -- M1
        case (0xAF<<3)|0: cpu->pip=0xB; break;
        case (0xAF<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xAF<<3)|2: cpu->pip=0x20203;_fetch();_xor(cpu->a); break;
        
        // OR B
        // -- M1
        case (0xB0<<3)|0: cpu->pip=0xB; break;
        case (0xB0<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xB0<<3)|2: cpu->pip=0x20203;_fetch();_or(cpu->b); break;
        
        // OR C
        // -- M1
        case (0xB1<<3)|0: cpu->pip=0xB; break;
        case (0xB1<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xB1<<3)|2: cpu->pip=0x20203;_fetch();_or(cpu->c); break;
        
        // OR D
        // -- M1
        case (0xB2<<3)|0: cpu->pip=0xB; break;
        case (0xB2<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xB2<<3)|2: cpu->pip=0x20203;_fetch();_or(cpu->d); break;
        
        // OR E
        // -- M1
        case (0xB3<<3)|0: cpu->pip=0xB; break;
        case (0xB3<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xB3<<3)|2: cpu->pip=0x20203;_fetch();_or(cpu->e); break;
        
        // OR H
        // -- M1
        case (0xB4<<3)|0: cpu->pip=0xB; break;
        case (0xB4<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xB4<<3)|2: cpu->pip=0x20203;_fetch();_or(cpu->h); break;
        
        // OR L
        // -- M1
        case (0xB5<<3)|0: cpu->pip=0xB; break;
        case (0xB5<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xB5<<3)|2: cpu->pip=0x20203;_fetch();_or(cpu->l); break;
        
        // OR (HL)
        // -- M1
        case (0xB6<<3)|0: cpu->pip=0xB; break;
        case (0xB6<<3)|1: _rfsh(); break;
        // -- M2
        case (0xB6<<3)|2: cpu->pip=0x2000B;_mr(_ghl()); break;
        case (0xB6<<3)|3: cpu->dlatch=_gd(); break;
        // -- OVERLAP
        case (0xB6<<3)|4: cpu->pip=0x20203;_fetch();_or(cpu->dlatch); break;
        
        // OR A
        // -- M1
        case (0xB7<<3)|0: cpu->pip=0xB; break;
        case (0xB7<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xB7<<3)|2: cpu->pip=0x20203;_fetch();_or(cpu->a); break;
        
        // CP B
        // -- M1
        case (0xB8<<3)|0: cpu->pip=0xB; break;
        case (0xB8<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xB8<<3)|2: cpu->pip=0x20203;_fetch();_cp(cpu->b); break;
        
        // CP C
        // -- M1
        case (0xB9<<3)|0: cpu->pip=0xB; break;
        case (0xB9<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xB9<<3)|2: cpu->pip=0x20203;_fetch();_cp(cpu->c); break;
        
        // CP D
        // -- M1
        case (0xBA<<3)|0: cpu->pip=0xB; break;
        case (0xBA<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xBA<<3)|2: cpu->pip=0x20203;_fetch();_cp(cpu->d); break;
        
        // CP E
        // -- M1
        case (0xBB<<3)|0: cpu->pip=0xB; break;
        case (0xBB<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xBB<<3)|2: cpu->pip=0x20203;_fetch();_cp(cpu->e); break;
        
        // CP H
        // -- M1
        case (0xBC<<3)|0: cpu->pip=0xB; break;
        case (0xBC<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xBC<<3)|2: cpu->pip=0x20203;_fetch();_cp(cpu->h); break;
        
        // CP L
        // -- M1
        case (0xBD<<3)|0: cpu->pip=0xB; break;
        case (0xBD<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xBD<<3)|2: cpu->pip=0x20203;_fetch();_cp(cpu->l); break;
        
        // CP (HL)
        // -- M1
        case (0xBE<<3)|0: cpu->pip=0xB; break;
        case (0xBE<<3)|1: _rfsh(); break;
        // -- M2
        case (0xBE<<3)|2: cpu->pip=0x2000B;_mr(_ghl()); break;
        case (0xBE<<3)|3: cpu->dlatch=_gd(); break;
        // -- OVERLAP
        case (0xBE<<3)|4: cpu->pip=0x20203;_fetch();_cp(cpu->dlatch); break;
        
        // CP A
        // -- M1
        case (0xBF<<3)|0: cpu->pip=0xB; break;
        case (0xBF<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xBF<<3)|2: cpu->pip=0x20203;_fetch();_cp(cpu->a); break;
        
        // RET NZ
        // -- M1
        case (0xC0<<3)|0: cpu->pip=0xB; break;
        case (0xC0<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xC0<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // POP BC2
        // -- M1
        case (0xC1<<3)|0: cpu->pip=0xB; break;
        case (0xC1<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xC1<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // JP NZ,nn
        // -- M1
        case (0xC2<<3)|0: cpu->pip=0xB; break;
        case (0xC2<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xC2<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // JP nn
        // -- M1
        case (0xC3<<3)|0: cpu->pip=0xB; break;
        case (0xC3<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xC3<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // CALL NZ,nn
        // -- M1
        case (0xC4<<3)|0: cpu->pip=0xB; break;
        case (0xC4<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xC4<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // PUSH BC2
        // -- M1
        case (0xC5<<3)|0: cpu->pip=0xB; break;
        case (0xC5<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xC5<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // ADD n
        // -- M1
        case (0xC6<<3)|0: cpu->pip=0xB; break;
        case (0xC6<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xC6<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // RST 0h
        // -- M1
        case (0xC7<<3)|0: cpu->pip=0xB; break;
        case (0xC7<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xC7<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // RET Z
        // -- M1
        case (0xC8<<3)|0: cpu->pip=0xB; break;
        case (0xC8<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xC8<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // RET
        // -- M1
        case (0xC9<<3)|0: cpu->pip=0xB; break;
        case (0xC9<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xC9<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // JP Z,nn
        // -- M1
        case (0xCA<<3)|0: cpu->pip=0xB; break;
        case (0xCA<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xCA<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // CB prefix
        // -- M1
        case (0xCB<<3)|0: cpu->pip=0xB; break;
        case (0xCB<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xCB<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // CALL Z,nn
        // -- M1
        case (0xCC<<3)|0: cpu->pip=0xB; break;
        case (0xCC<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xCC<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // CALL nn
        // -- M1
        case (0xCD<<3)|0: cpu->pip=0xB; break;
        case (0xCD<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xCD<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // ADC n
        // -- M1
        case (0xCE<<3)|0: cpu->pip=0xB; break;
        case (0xCE<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xCE<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // RST 8h
        // -- M1
        case (0xCF<<3)|0: cpu->pip=0xB; break;
        case (0xCF<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xCF<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // RET NC
        // -- M1
        case (0xD0<<3)|0: cpu->pip=0xB; break;
        case (0xD0<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xD0<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // POP DE2
        // -- M1
        case (0xD1<<3)|0: cpu->pip=0xB; break;
        case (0xD1<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xD1<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // JP NC,nn
        // -- M1
        case (0xD2<<3)|0: cpu->pip=0xB; break;
        case (0xD2<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xD2<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // OUT (n),A
        // -- M1
        case (0xD3<<3)|0: cpu->pip=0xB; break;
        case (0xD3<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xD3<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // CALL NC,nn
        // -- M1
        case (0xD4<<3)|0: cpu->pip=0xB; break;
        case (0xD4<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xD4<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // PUSH DE2
        // -- M1
        case (0xD5<<3)|0: cpu->pip=0xB; break;
        case (0xD5<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xD5<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // SUB n
        // -- M1
        case (0xD6<<3)|0: cpu->pip=0xB; break;
        case (0xD6<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xD6<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // RST 10h
        // -- M1
        case (0xD7<<3)|0: cpu->pip=0xB; break;
        case (0xD7<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xD7<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // RET C
        // -- M1
        case (0xD8<<3)|0: cpu->pip=0xB; break;
        case (0xD8<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xD8<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // EXX
        // -- M1
        case (0xD9<<3)|0: cpu->pip=0xB; break;
        case (0xD9<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xD9<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // JP C,nn
        // -- M1
        case (0xDA<<3)|0: cpu->pip=0xB; break;
        case (0xDA<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xDA<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // IN A,(n)
        // -- M1
        case (0xDB<<3)|0: cpu->pip=0xB; break;
        case (0xDB<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xDB<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // CALL C,nn
        // -- M1
        case (0xDC<<3)|0: cpu->pip=0xB; break;
        case (0xDC<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xDC<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // DD prefix
        // -- M1
        case (0xDD<<3)|0: cpu->pip=0xB; break;
        case (0xDD<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xDD<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // SBC n
        // -- M1
        case (0xDE<<3)|0: cpu->pip=0xB; break;
        case (0xDE<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xDE<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // RST 18h
        // -- M1
        case (0xDF<<3)|0: cpu->pip=0xB; break;
        case (0xDF<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xDF<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // RET PO
        // -- M1
        case (0xE0<<3)|0: cpu->pip=0xB; break;
        case (0xE0<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xE0<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // POP HL2
        // -- M1
        case (0xE1<<3)|0: cpu->pip=0xB; break;
        case (0xE1<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xE1<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // JP PO,nn
        // -- M1
        case (0xE2<<3)|0: cpu->pip=0xB; break;
        case (0xE2<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xE2<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // EX (SP),HL
        // -- M1
        case (0xE3<<3)|0: cpu->pip=0xB; break;
        case (0xE3<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xE3<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // CALL PO,nn
        // -- M1
        case (0xE4<<3)|0: cpu->pip=0xB; break;
        case (0xE4<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xE4<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // PUSH HL2
        // -- M1
        case (0xE5<<3)|0: cpu->pip=0xB; break;
        case (0xE5<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xE5<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // AND n
        // -- M1
        case (0xE6<<3)|0: cpu->pip=0xB; break;
        case (0xE6<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xE6<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // RST 20h
        // -- M1
        case (0xE7<<3)|0: cpu->pip=0xB; break;
        case (0xE7<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xE7<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // RET PE
        // -- M1
        case (0xE8<<3)|0: cpu->pip=0xB; break;
        case (0xE8<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xE8<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // JP HL
        // -- M1
        case (0xE9<<3)|0: cpu->pip=0xB; break;
        case (0xE9<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xE9<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // JP PE,nn
        // -- M1
        case (0xEA<<3)|0: cpu->pip=0xB; break;
        case (0xEA<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xEA<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // EX DE,HL
        // -- M1
        case (0xEB<<3)|0: cpu->pip=0xB; break;
        case (0xEB<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xEB<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // CALL PE,nn
        // -- M1
        case (0xEC<<3)|0: cpu->pip=0xB; break;
        case (0xEC<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xEC<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // ED prefix
        // -- M1
        case (0xED<<3)|0: cpu->pip=0xB; break;
        case (0xED<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xED<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // XOR n
        // -- M1
        case (0xEE<<3)|0: cpu->pip=0xB; break;
        case (0xEE<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xEE<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // RST 28h
        // -- M1
        case (0xEF<<3)|0: cpu->pip=0xB; break;
        case (0xEF<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xEF<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // RET P
        // -- M1
        case (0xF0<<3)|0: cpu->pip=0xB; break;
        case (0xF0<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xF0<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // POP SP2
        // -- M1
        case (0xF1<<3)|0: cpu->pip=0xB; break;
        case (0xF1<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xF1<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // JP P,nn
        // -- M1
        case (0xF2<<3)|0: cpu->pip=0xB; break;
        case (0xF2<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xF2<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // DI
        // -- M1
        case (0xF3<<3)|0: cpu->pip=0xB; break;
        case (0xF3<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xF3<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // CALL P,nn
        // -- M1
        case (0xF4<<3)|0: cpu->pip=0xB; break;
        case (0xF4<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xF4<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // PUSH SP2
        // -- M1
        case (0xF5<<3)|0: cpu->pip=0xB; break;
        case (0xF5<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xF5<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // OR n
        // -- M1
        case (0xF6<<3)|0: cpu->pip=0xB; break;
        case (0xF6<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xF6<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // RST 30h
        // -- M1
        case (0xF7<<3)|0: cpu->pip=0xB; break;
        case (0xF7<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xF7<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // RET M
        // -- M1
        case (0xF8<<3)|0: cpu->pip=0xB; break;
        case (0xF8<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xF8<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // LD SP,HL
        // -- M1
        case (0xF9<<3)|0: cpu->pip=0xB; break;
        case (0xF9<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xF9<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // JP M,nn
        // -- M1
        case (0xFA<<3)|0: cpu->pip=0xB; break;
        case (0xFA<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xFA<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // EI
        // -- M1
        case (0xFB<<3)|0: cpu->pip=0xB; break;
        case (0xFB<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xFB<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // CALL M,nn
        // -- M1
        case (0xFC<<3)|0: cpu->pip=0xB; break;
        case (0xFC<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xFC<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // FD prefix
        // -- M1
        case (0xFD<<3)|0: cpu->pip=0xB; break;
        case (0xFD<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xFD<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // CP n
        // -- M1
        case (0xFE<<3)|0: cpu->pip=0xB; break;
        case (0xFE<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xFE<<3)|2: cpu->pip=0x20203;_fetch();; break;
        
        // RST 38h
        // -- M1
        case (0xFF<<3)|0: cpu->pip=0xB; break;
        case (0xFF<<3)|1: _rfsh(); break;
        // -- OVERLAP
        case (0xFF<<3)|2: cpu->pip=0x20203;_fetch();; break;

    }
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