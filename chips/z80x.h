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

// pin access helper macros
#define Z80_GET_ADDR(p) ((uint16_t)(p&0xFFFF))
#define Z80_SET_ADDR(p,a) {p=(p&~0xFFFF)|((a)&0xFFFF);}
#define Z80_GET_DATA(p) ((uint8_t)((p>>16)&0xFF))
#define Z80_SET_DATA(p,d) {p=(p&~0xFF0000)|((d<<16)&0xFF0000);}

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

typedef struct {
    uint64_t pip;   // the op's decode pipeline
    uint64_t step;  // first or current decoder switch-case branch step
} z80_opstate_t;

// CPU state
typedef struct {
    uint64_t pins;      // last stored pin state
    z80_opstate_t op;   // the currently active op
    uint16_t pc;        // program counter
    uint8_t dlatch;     // temporary store for data bus value

    // NOTE: These unions are fine in C, but not C++.
    union { struct { uint8_t f; uint8_t a; }; uint16_t af; };
    union { struct { uint8_t c; uint8_t b; }; uint16_t bc; };
    union { struct { uint8_t e; uint8_t d; }; uint16_t de; };
    union { struct { uint8_t l; uint8_t h; }; uint16_t hl; };
    union { struct { uint8_t wzl; uint8_t wzh; }; uint16_t wz; };
    union { struct { uint8_t spl; uint8_t sph; }; uint16_t sp; };
    union { struct { uint8_t ixl; uint8_t ixh; }; uint16_t ix; };
    union { struct { uint8_t iyl; uint8_t iyh; }; uint16_t iy; };
    uint8_t i;
    uint8_t r;
    uint8_t im;
    uint16_t af2, bc2, de2, hl2; // shadow register bank
} z80_t;

// initialize a new Z80 instance and return initial pin mask
uint64_t z80_init(z80_t* cpu);
// execute one tick, return new pin mask
uint64_t z80_tick(z80_t* cpu, uint64_t pins);
// return true when full instruction has finished
bool z80_opdone(z80_t* cpu) {
    // because of the overlapped cycle, the result of the previous
    // instruction is only available in the refresh cycle
    // FIXME: prefixed instructions!
    return 0 != (cpu->pins & Z80_RFSH);
}

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
    memset(cpu, 0, sizeof(z80_t));
    // initial state according to visualz80
    cpu->af = cpu->bc = cpu->de = cpu->hl = 0x5555;
    cpu->wz = cpu->sp = cpu->ix = cpu->iy = 0x5555;
    cpu->af2 = cpu->bc2 = cpu->de2 = cpu->hl2 = 0x5555;
    // FIXME: iff1/2 disabled, initial value of IM???

    // return bit mask which causes the CPU to execute one
    // NOP in order to 'ignore' instruction processing
    return Z80_M1|Z80_MREQ|Z80_RD;
}

static inline void z80_halt(z80_t* cpu) {
    // FIXME
    (void)cpu;
}

static inline void z80_add(z80_t* cpu, uint8_t val) {
    // FIXME
    (void)cpu; (void)val;
}

static inline void z80_adc(z80_t* cpu, uint8_t val) {
    // FIXME
    (void)cpu; (void)val;
}

static inline void z80_sub(z80_t* cpu, uint8_t val) {
    // FIXME
    (void)cpu; (void)val;
}

static inline void z80_sbc(z80_t* cpu, uint8_t val) {
    // FIXME
    (void)cpu; (void)val;
}

static inline void z80_and(z80_t* cpu, uint8_t val) {
    // FIXME
    (void)cpu; (void)val;
}

static inline void z80_xor(z80_t* cpu, uint8_t val) {
    // FIXME
    (void)cpu; (void)val;
}

static inline void z80_or(z80_t* cpu, uint8_t val) {
    // FIXME
    (void)cpu; (void)val;
}

static inline void z80_cp(z80_t* cpu, uint8_t val) {
    // FIXME
    (void)cpu; (void)val;
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

static const z80_opstate_t z80_opstate_table[256] = {
    // 0x00: nop (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0000 },
    // 0x01: ld bc,nn (M:3 T:10 steps:6)
    { 0x00000091000002DA, 0x0002 },
    // 0x02: ld (bc),a (M:2 T:7 steps:3)
    { 0x0000001100000052, 0x0008 },
    // 0x03: inc bc (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x000B },
    // 0x04: inc b (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x000D },
    // 0x05: dec b (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x000F },
    // 0x06: ld b,n (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x0011 },
    // 0x07: rlca (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0015 },
    // 0x08: ex af,af' (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0017 },
    // 0x09: add hl,bc (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0019 },
    // 0x0A: ld a,(bc) (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x001B },
    // 0x0B: dec bc (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x001F },
    // 0x0C: inc c (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0021 },
    // 0x0D: dec c (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0023 },
    // 0x0E: ld c,n (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x0025 },
    // 0x0F: rrca (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0029 },
    // 0x10: djnz d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x002B },
    // 0x11: ld de,nn (M:3 T:10 steps:6)
    { 0x00000091000002DA, 0x002D },
    // 0x12: ld (de),a (M:2 T:7 steps:3)
    { 0x0000001100000052, 0x0033 },
    // 0x13: inc de (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0036 },
    // 0x14: inc d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0038 },
    // 0x15: dec d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x003A },
    // 0x16: ld d,n (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x003C },
    // 0x17: rla (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0040 },
    // 0x18: jr d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0042 },
    // 0x19: add hl,de (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0044 },
    // 0x1A: ld a,(de) (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x0046 },
    // 0x1B: dec de (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x004A },
    // 0x1C: inc e (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x004C },
    // 0x1D: dec e (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x004E },
    // 0x1E: ld e,n (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x0050 },
    // 0x1F: rra (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0054 },
    // 0x20: jr nz,d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0056 },
    // 0x21: ld hl,nn (M:3 T:10 steps:6)
    { 0x00000091000002DA, 0x0058 },
    // 0x22: ld (nn),hl (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x005E },
    // 0x23: inc hl (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0060 },
    // 0x24: inc h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0062 },
    // 0x25: dec h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0064 },
    // 0x26: ld h,n (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x0066 },
    // 0x27: daa (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x006A },
    // 0x28: jr z,d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x006C },
    // 0x29: add hl,hl (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x006E },
    // 0x2A: ld hl,(nn) (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0070 },
    // 0x2B: dec hl (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0072 },
    // 0x2C: inc l (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0074 },
    // 0x2D: dec l (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0076 },
    // 0x2E: ld l,n (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x0078 },
    // 0x2F: cpl (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x007C },
    // 0x30: jr nc,d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x007E },
    // 0x31: ld sp,nn (M:3 T:10 steps:6)
    { 0x00000091000002DA, 0x0080 },
    // 0x32: ld (nn),a (M:4 T:13 steps:7)
    { 0x00000491000014DA, 0x0086 },
    // 0x33: inc sp (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x008D },
    // 0x34: inc (hl) (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x008F },
    // 0x35: dec (hl) (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0091 },
    // 0x36: ld (HL),n (M:3 T:10 steps:5)
    { 0x000000910000029A, 0x0093 },
    // 0x37: scf (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0098 },
    // 0x38: jr c,d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x009A },
    // 0x39: add hl,sp (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x009C },
    // 0x3A: ld a,(nn) (M:4 T:13 steps:8)
    { 0x00000491000016DA, 0x009E },
    // 0x3B: dec sp (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00A6 },
    // 0x3C: inc a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00A8 },
    // 0x3D: dec a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00AA },
    // 0x3E: ld a,n (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x00AC },
    // 0x3F: ccf (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00B0 },
    // 0x40: ld b,b (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00B2 },
    // 0x41: ld b,c (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00B4 },
    // 0x42: ld b,d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00B6 },
    // 0x43: ld b,e (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00B8 },
    // 0x44: ld b,h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00BA },
    // 0x45: ld b,l (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00BC },
    // 0x46: ld b,(hl) (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x00BE },
    // 0x47: ld b,a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00C2 },
    // 0x48: ld c,b (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00C4 },
    // 0x49: ld c,c (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00C6 },
    // 0x4A: ld c,d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00C8 },
    // 0x4B: ld c,e (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00CA },
    // 0x4C: ld c,h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00CC },
    // 0x4D: ld c,l (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00CE },
    // 0x4E: ld c,(hl) (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x00D0 },
    // 0x4F: ld c,a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00D4 },
    // 0x50: ld d,b (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00D6 },
    // 0x51: ld d,c (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00D8 },
    // 0x52: ld d,d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00DA },
    // 0x53: ld d,e (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00DC },
    // 0x54: ld d,h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00DE },
    // 0x55: ld d,l (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00E0 },
    // 0x56: ld d,(hl) (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x00E2 },
    // 0x57: ld d,a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00E6 },
    // 0x58: ld e,b (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00E8 },
    // 0x59: ld e,c (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00EA },
    // 0x5A: ld e,d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00EC },
    // 0x5B: ld e,e (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00EE },
    // 0x5C: ld e,h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00F0 },
    // 0x5D: ld e,l (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00F2 },
    // 0x5E: ld e,(hl) (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x00F4 },
    // 0x5F: ld e,a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00F8 },
    // 0x60: ld h,b (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00FA },
    // 0x61: ld h,c (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00FC },
    // 0x62: ld h,d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00FE },
    // 0x63: ld h,e (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0100 },
    // 0x64: ld h,h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0102 },
    // 0x65: ld h,l (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0104 },
    // 0x66: ld h,(hl) (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x0106 },
    // 0x67: ld h,a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x010A },
    // 0x68: ld l,b (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x010C },
    // 0x69: ld l,c (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x010E },
    // 0x6A: ld l,d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0110 },
    // 0x6B: ld l,e (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0112 },
    // 0x6C: ld l,h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0114 },
    // 0x6D: ld l,l (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0116 },
    // 0x6E: ld l,(hl) (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x0118 },
    // 0x6F: ld l,a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x011C },
    // 0x70: ld (hl),b (M:2 T:7 steps:3)
    { 0x0000001100000052, 0x011E },
    // 0x71: ld (hl),c (M:2 T:7 steps:3)
    { 0x0000001100000052, 0x0121 },
    // 0x72: ld (hl),d (M:2 T:7 steps:3)
    { 0x0000001100000052, 0x0124 },
    // 0x73: ld (hl),e (M:2 T:7 steps:3)
    { 0x0000001100000052, 0x0127 },
    // 0x74: ld (hl),h (M:2 T:7 steps:3)
    { 0x0000001100000052, 0x012A },
    // 0x75: ld (hl),l (M:2 T:7 steps:3)
    { 0x0000001100000052, 0x012D },
    // 0x76: halt (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0130 },
    // 0x77: ld (hl),a (M:2 T:7 steps:3)
    { 0x0000001100000052, 0x0132 },
    // 0x78: ld a,b (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0135 },
    // 0x79: ld a,c (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0137 },
    // 0x7A: ld a,d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0139 },
    // 0x7B: ld a,e (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x013B },
    // 0x7C: ld a,h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x013D },
    // 0x7D: ld a,l (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x013F },
    // 0x7E: ld a,(hl) (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x0141 },
    // 0x7F: ld a,a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0145 },
    // 0x80: add b (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0147 },
    // 0x81: add c (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0149 },
    // 0x82: add d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x014B },
    // 0x83: add e (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x014D },
    // 0x84: add h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x014F },
    // 0x85: add l (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0151 },
    // 0x86: add (hl) (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x0153 },
    // 0x87: add a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0157 },
    // 0x88: adc b (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0159 },
    // 0x89: adc c (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x015B },
    // 0x8A: adc d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x015D },
    // 0x8B: adc e (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x015F },
    // 0x8C: adc h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0161 },
    // 0x8D: adc l (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0163 },
    // 0x8E: adc (hl) (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x0165 },
    // 0x8F: adc a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0169 },
    // 0x90: sub b (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x016B },
    // 0x91: sub c (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x016D },
    // 0x92: sub d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x016F },
    // 0x93: sub e (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0171 },
    // 0x94: sub h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0173 },
    // 0x95: sub l (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0175 },
    // 0x96: sub (hl) (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x0177 },
    // 0x97: sub a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x017B },
    // 0x98: sbc b (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x017D },
    // 0x99: sbc c (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x017F },
    // 0x9A: sbc d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0181 },
    // 0x9B: sbc e (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0183 },
    // 0x9C: sbc h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0185 },
    // 0x9D: sbc l (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0187 },
    // 0x9E: sbc (hl) (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x0189 },
    // 0x9F: sbc a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x018D },
    // 0xA0: and b (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x018F },
    // 0xA1: and c (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0191 },
    // 0xA2: and d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0193 },
    // 0xA3: and e (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0195 },
    // 0xA4: and h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0197 },
    // 0xA5: and l (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0199 },
    // 0xA6: and (hl) (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x019B },
    // 0xA7: and a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x019F },
    // 0xA8: xor b (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01A1 },
    // 0xA9: xor c (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01A3 },
    // 0xAA: xor d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01A5 },
    // 0xAB: xor e (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01A7 },
    // 0xAC: xor h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01A9 },
    // 0xAD: xor l (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01AB },
    // 0xAE: xor (hl) (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x01AD },
    // 0xAF: xor a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01B1 },
    // 0xB0: or b (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01B3 },
    // 0xB1: or c (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01B5 },
    // 0xB2: or d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01B7 },
    // 0xB3: or e (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01B9 },
    // 0xB4: or h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01BB },
    // 0xB5: or l (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01BD },
    // 0xB6: or (hl) (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x01BF },
    // 0xB7: or a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01C3 },
    // 0xB8: cp b (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01C5 },
    // 0xB9: cp c (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01C7 },
    // 0xBA: cp d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01C9 },
    // 0xBB: cp e (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01CB },
    // 0xBC: cp h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01CD },
    // 0xBD: cp l (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01CF },
    // 0xBE: cp (hl) (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x01D1 },
    // 0xBF: cp a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01D5 },
    // 0xC0: ret nz (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01D7 },
    // 0xC1: pop bc2 (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01D9 },
    // 0xC2: jp nz,nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01DB },
    // 0xC3: jp nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01DD },
    // 0xC4: call nz,nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01DF },
    // 0xC5: push bc2 (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01E1 },
    // 0xC6: add n (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01E3 },
    // 0xC7: rst 0h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01E5 },
    // 0xC8: ret z (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01E7 },
    // 0xC9: ret (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01E9 },
    // 0xCA: jp z,nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01EB },
    // 0xCB: cb prefix (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01ED },
    // 0xCC: call z,nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01EF },
    // 0xCD: call nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01F1 },
    // 0xCE: adc n (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01F3 },
    // 0xCF: rst 8h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01F5 },
    // 0xD0: ret nc (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01F7 },
    // 0xD1: pop de2 (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01F9 },
    // 0xD2: jp nc,nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01FB },
    // 0xD3: out (n),a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01FD },
    // 0xD4: call nc,nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01FF },
    // 0xD5: push de2 (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0201 },
    // 0xD6: sub n (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0203 },
    // 0xD7: rst 10h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0205 },
    // 0xD8: ret c (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0207 },
    // 0xD9: exx (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0209 },
    // 0xDA: jp c,nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x020B },
    // 0xDB: in a,(n) (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x020D },
    // 0xDC: call c,nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x020F },
    // 0xDD: dd prefix (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0211 },
    // 0xDE: sbc n (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0213 },
    // 0xDF: rst 18h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0215 },
    // 0xE0: ret po (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0217 },
    // 0xE1: pop hl2 (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0219 },
    // 0xE2: jp po,nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x021B },
    // 0xE3: ex (sp),hl (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x021D },
    // 0xE4: call po,nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x021F },
    // 0xE5: push hl2 (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0221 },
    // 0xE6: and n (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0223 },
    // 0xE7: rst 20h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0225 },
    // 0xE8: ret pe (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0227 },
    // 0xE9: jp hl (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0229 },
    // 0xEA: jp pe,nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x022B },
    // 0xEB: ex de,hl (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x022D },
    // 0xEC: call pe,nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x022F },
    // 0xED: ed prefix (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0231 },
    // 0xEE: xor n (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0233 },
    // 0xEF: rst 28h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0235 },
    // 0xF0: ret p (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0237 },
    // 0xF1: pop sp2 (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0239 },
    // 0xF2: jp p,nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x023B },
    // 0xF3: di (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x023D },
    // 0xF4: call p,nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x023F },
    // 0xF5: push sp2 (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0241 },
    // 0xF6: or n (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0243 },
    // 0xF7: rst 30h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0245 },
    // 0xF8: ret m (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0247 },
    // 0xF9: ld sp,hl (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0249 },
    // 0xFA: jp m,nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x024B },
    // 0xFB: ei (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x024D },
    // 0xFC: call m,nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x024F },
    // 0xFD: fd prefix (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0251 },
    // 0xFE: cp n (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0253 },
    // 0xFF: rst 38h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0255 },

};

// pin helper macros
#define _sa(ab)             pins=z80_set_ab(pins,ab)
#define _sax(ab,x)          pins=z80_set_ab_x(pins,ab,x)
#define _sad(ab,d)          pins=z80_set_ab_db(pins,ab,d)
#define _sadx(ab,d,x)       pins=z80_set_ab_db_x(pins,ab,d,x)
#define _gd()               z80_get_db(pins)

// high level helper macros
#define _fetch()        _sax(cpu->pc++,Z80_M1|Z80_MREQ|Z80_RD)
#define _rfsh()         _sax(cpu->r,Z80_MREQ|Z80_RFSH);cpu->r=(cpu->r&0x80)|((cpu->r+1)&0x7F)
#define _mread(ab)      _sax(ab,Z80_MREQ|Z80_RD)
#define _mwrite(ab,d)   _sadx(ab,d,Z80_MREQ|Z80_WR)
#define _ioread(ab)     _sax(ab,Z80_IORQ|Z80_RD)
#define _iowrite(ab,d)  _sadx(ab,d,Z80_IORQ|Z80_WR)

uint64_t z80_tick(z80_t* cpu, uint64_t pins) {
    // wait cycle? (wait pin sampling only happens in specific tcycles)
    if ((cpu->op.pip & Z80_PIP_BIT_WAIT) && (pins & Z80_WAIT)) {
        cpu->pins = pins & ~Z80_CTRL_PIN_MASK;
        return pins;
    }
    // load next instruction opcode from data bus
    if ((pins & (Z80_M1|Z80_MREQ|Z80_RD)) == (Z80_M1|Z80_MREQ|Z80_RD)) {
        uint8_t opcode = _gd();
        cpu->op = z80_opstate_table[opcode];
    }
    // process the next active tcycle
    pins &= ~Z80_CTRL_PIN_MASK;
    if (cpu->op.pip & Z80_PIP_BIT_STEP) {
        switch (cpu->op.step++) {
            
            // 0x00: nop (M:1 T:4)
            // -- M1
            case 0x0000: _rfsh(); break;
            // -- OVERLAP
            case 0x0001: _fetch(); break;
            
            // 0x01: ld bc,nn (M:3 T:10)
            // -- M1
            case 0x0002: _rfsh(); break;
            // -- M2
            case 0x0003: _mread(cpu->pc++); break;
            case 0x0004: cpu->c=_gd(); break;
            // -- M3
            case 0x0005: _mread(cpu->pc++); break;
            case 0x0006: cpu->b=_gd(); break;
            // -- OVERLAP
            case 0x0007: _fetch(); break;
            
            // 0x02: ld (bc),a (M:2 T:7)
            // -- M1
            case 0x0008: _rfsh(); break;
            // -- M2
            case 0x0009: _mwrite(cpu->bc,cpu->a);cpu->wzl=cpu->c+1;cpu->wzh=cpu->a; break;
            // -- OVERLAP
            case 0x000A: _fetch(); break;
            
            // 0x03: inc bc (M:1 T:4)
            // -- M1
            case 0x000B: _rfsh(); break;
            // -- OVERLAP
            case 0x000C: _fetch(); break;
            
            // 0x04: inc b (M:1 T:4)
            // -- M1
            case 0x000D: _rfsh(); break;
            // -- OVERLAP
            case 0x000E: _fetch(); break;
            
            // 0x05: dec b (M:1 T:4)
            // -- M1
            case 0x000F: _rfsh(); break;
            // -- OVERLAP
            case 0x0010: _fetch(); break;
            
            // 0x06: ld b,n (M:2 T:7)
            // -- M1
            case 0x0011: _rfsh(); break;
            // -- M2
            case 0x0012: _mread(cpu->pc++); break;
            case 0x0013: cpu->b=_gd(); break;
            // -- OVERLAP
            case 0x0014: _fetch(); break;
            
            // 0x07: rlca (M:1 T:4)
            // -- M1
            case 0x0015: _rfsh(); break;
            // -- OVERLAP
            case 0x0016: _fetch(); break;
            
            // 0x08: ex af,af' (M:1 T:4)
            // -- M1
            case 0x0017: _rfsh(); break;
            // -- OVERLAP
            case 0x0018: _fetch(); break;
            
            // 0x09: add hl,bc (M:1 T:4)
            // -- M1
            case 0x0019: _rfsh(); break;
            // -- OVERLAP
            case 0x001A: _fetch(); break;
            
            // 0x0A: ld a,(bc) (M:2 T:7)
            // -- M1
            case 0x001B: _rfsh(); break;
            // -- M2
            case 0x001C: _mread(cpu->bc);cpu->wz=cpu->bc+1; break;
            case 0x001D: cpu->a=_gd(); break;
            // -- OVERLAP
            case 0x001E: _fetch(); break;
            
            // 0x0B: dec bc (M:1 T:4)
            // -- M1
            case 0x001F: _rfsh(); break;
            // -- OVERLAP
            case 0x0020: _fetch(); break;
            
            // 0x0C: inc c (M:1 T:4)
            // -- M1
            case 0x0021: _rfsh(); break;
            // -- OVERLAP
            case 0x0022: _fetch(); break;
            
            // 0x0D: dec c (M:1 T:4)
            // -- M1
            case 0x0023: _rfsh(); break;
            // -- OVERLAP
            case 0x0024: _fetch(); break;
            
            // 0x0E: ld c,n (M:2 T:7)
            // -- M1
            case 0x0025: _rfsh(); break;
            // -- M2
            case 0x0026: _mread(cpu->pc++); break;
            case 0x0027: cpu->c=_gd(); break;
            // -- OVERLAP
            case 0x0028: _fetch(); break;
            
            // 0x0F: rrca (M:1 T:4)
            // -- M1
            case 0x0029: _rfsh(); break;
            // -- OVERLAP
            case 0x002A: _fetch(); break;
            
            // 0x10: djnz d (M:1 T:4)
            // -- M1
            case 0x002B: _rfsh(); break;
            // -- OVERLAP
            case 0x002C: _fetch(); break;
            
            // 0x11: ld de,nn (M:3 T:10)
            // -- M1
            case 0x002D: _rfsh(); break;
            // -- M2
            case 0x002E: _mread(cpu->pc++); break;
            case 0x002F: cpu->e=_gd(); break;
            // -- M3
            case 0x0030: _mread(cpu->pc++); break;
            case 0x0031: cpu->d=_gd(); break;
            // -- OVERLAP
            case 0x0032: _fetch(); break;
            
            // 0x12: ld (de),a (M:2 T:7)
            // -- M1
            case 0x0033: _rfsh(); break;
            // -- M2
            case 0x0034: _mwrite(cpu->de,cpu->a);cpu->wzl=cpu->e+1;cpu->wzh=cpu->a; break;
            // -- OVERLAP
            case 0x0035: _fetch(); break;
            
            // 0x13: inc de (M:1 T:4)
            // -- M1
            case 0x0036: _rfsh(); break;
            // -- OVERLAP
            case 0x0037: _fetch(); break;
            
            // 0x14: inc d (M:1 T:4)
            // -- M1
            case 0x0038: _rfsh(); break;
            // -- OVERLAP
            case 0x0039: _fetch(); break;
            
            // 0x15: dec d (M:1 T:4)
            // -- M1
            case 0x003A: _rfsh(); break;
            // -- OVERLAP
            case 0x003B: _fetch(); break;
            
            // 0x16: ld d,n (M:2 T:7)
            // -- M1
            case 0x003C: _rfsh(); break;
            // -- M2
            case 0x003D: _mread(cpu->pc++); break;
            case 0x003E: cpu->d=_gd(); break;
            // -- OVERLAP
            case 0x003F: _fetch(); break;
            
            // 0x17: rla (M:1 T:4)
            // -- M1
            case 0x0040: _rfsh(); break;
            // -- OVERLAP
            case 0x0041: _fetch(); break;
            
            // 0x18: jr d (M:1 T:4)
            // -- M1
            case 0x0042: _rfsh(); break;
            // -- OVERLAP
            case 0x0043: _fetch(); break;
            
            // 0x19: add hl,de (M:1 T:4)
            // -- M1
            case 0x0044: _rfsh(); break;
            // -- OVERLAP
            case 0x0045: _fetch(); break;
            
            // 0x1A: ld a,(de) (M:2 T:7)
            // -- M1
            case 0x0046: _rfsh(); break;
            // -- M2
            case 0x0047: _mread(cpu->de);cpu->wz=cpu->de+1; break;
            case 0x0048: cpu->a=_gd(); break;
            // -- OVERLAP
            case 0x0049: _fetch(); break;
            
            // 0x1B: dec de (M:1 T:4)
            // -- M1
            case 0x004A: _rfsh(); break;
            // -- OVERLAP
            case 0x004B: _fetch(); break;
            
            // 0x1C: inc e (M:1 T:4)
            // -- M1
            case 0x004C: _rfsh(); break;
            // -- OVERLAP
            case 0x004D: _fetch(); break;
            
            // 0x1D: dec e (M:1 T:4)
            // -- M1
            case 0x004E: _rfsh(); break;
            // -- OVERLAP
            case 0x004F: _fetch(); break;
            
            // 0x1E: ld e,n (M:2 T:7)
            // -- M1
            case 0x0050: _rfsh(); break;
            // -- M2
            case 0x0051: _mread(cpu->pc++); break;
            case 0x0052: cpu->e=_gd(); break;
            // -- OVERLAP
            case 0x0053: _fetch(); break;
            
            // 0x1F: rra (M:1 T:4)
            // -- M1
            case 0x0054: _rfsh(); break;
            // -- OVERLAP
            case 0x0055: _fetch(); break;
            
            // 0x20: jr nz,d (M:1 T:4)
            // -- M1
            case 0x0056: _rfsh(); break;
            // -- OVERLAP
            case 0x0057: _fetch(); break;
            
            // 0x21: ld hl,nn (M:3 T:10)
            // -- M1
            case 0x0058: _rfsh(); break;
            // -- M2
            case 0x0059: _mread(cpu->pc++); break;
            case 0x005A: cpu->l=_gd(); break;
            // -- M3
            case 0x005B: _mread(cpu->pc++); break;
            case 0x005C: cpu->h=_gd(); break;
            // -- OVERLAP
            case 0x005D: _fetch(); break;
            
            // 0x22: ld (nn),hl (M:1 T:4)
            // -- M1
            case 0x005E: _rfsh(); break;
            // -- OVERLAP
            case 0x005F: _fetch(); break;
            
            // 0x23: inc hl (M:1 T:4)
            // -- M1
            case 0x0060: _rfsh(); break;
            // -- OVERLAP
            case 0x0061: _fetch(); break;
            
            // 0x24: inc h (M:1 T:4)
            // -- M1
            case 0x0062: _rfsh(); break;
            // -- OVERLAP
            case 0x0063: _fetch(); break;
            
            // 0x25: dec h (M:1 T:4)
            // -- M1
            case 0x0064: _rfsh(); break;
            // -- OVERLAP
            case 0x0065: _fetch(); break;
            
            // 0x26: ld h,n (M:2 T:7)
            // -- M1
            case 0x0066: _rfsh(); break;
            // -- M2
            case 0x0067: _mread(cpu->pc++); break;
            case 0x0068: cpu->h=_gd(); break;
            // -- OVERLAP
            case 0x0069: _fetch(); break;
            
            // 0x27: daa (M:1 T:4)
            // -- M1
            case 0x006A: _rfsh(); break;
            // -- OVERLAP
            case 0x006B: _fetch(); break;
            
            // 0x28: jr z,d (M:1 T:4)
            // -- M1
            case 0x006C: _rfsh(); break;
            // -- OVERLAP
            case 0x006D: _fetch(); break;
            
            // 0x29: add hl,hl (M:1 T:4)
            // -- M1
            case 0x006E: _rfsh(); break;
            // -- OVERLAP
            case 0x006F: _fetch(); break;
            
            // 0x2A: ld hl,(nn) (M:1 T:4)
            // -- M1
            case 0x0070: _rfsh(); break;
            // -- OVERLAP
            case 0x0071: _fetch(); break;
            
            // 0x2B: dec hl (M:1 T:4)
            // -- M1
            case 0x0072: _rfsh(); break;
            // -- OVERLAP
            case 0x0073: _fetch(); break;
            
            // 0x2C: inc l (M:1 T:4)
            // -- M1
            case 0x0074: _rfsh(); break;
            // -- OVERLAP
            case 0x0075: _fetch(); break;
            
            // 0x2D: dec l (M:1 T:4)
            // -- M1
            case 0x0076: _rfsh(); break;
            // -- OVERLAP
            case 0x0077: _fetch(); break;
            
            // 0x2E: ld l,n (M:2 T:7)
            // -- M1
            case 0x0078: _rfsh(); break;
            // -- M2
            case 0x0079: _mread(cpu->pc++); break;
            case 0x007A: cpu->l=_gd(); break;
            // -- OVERLAP
            case 0x007B: _fetch(); break;
            
            // 0x2F: cpl (M:1 T:4)
            // -- M1
            case 0x007C: _rfsh(); break;
            // -- OVERLAP
            case 0x007D: _fetch(); break;
            
            // 0x30: jr nc,d (M:1 T:4)
            // -- M1
            case 0x007E: _rfsh(); break;
            // -- OVERLAP
            case 0x007F: _fetch(); break;
            
            // 0x31: ld sp,nn (M:3 T:10)
            // -- M1
            case 0x0080: _rfsh(); break;
            // -- M2
            case 0x0081: _mread(cpu->pc++); break;
            case 0x0082: cpu->spl=_gd(); break;
            // -- M3
            case 0x0083: _mread(cpu->pc++); break;
            case 0x0084: cpu->sph=_gd(); break;
            // -- OVERLAP
            case 0x0085: _fetch(); break;
            
            // 0x32: ld (nn),a (M:4 T:13)
            // -- M1
            case 0x0086: _rfsh(); break;
            // -- M2
            case 0x0087: _mread(cpu->pc++); break;
            case 0x0088: cpu->wzl=_gd(); break;
            // -- M3
            case 0x0089: _mread(cpu->pc++); break;
            case 0x008A: cpu->wzh=_gd(); break;
            // -- M4
            case 0x008B: _mwrite(cpu->wz++,cpu->a);cpu->wzh=cpu->a; break;
            // -- OVERLAP
            case 0x008C: _fetch(); break;
            
            // 0x33: inc sp (M:1 T:4)
            // -- M1
            case 0x008D: _rfsh(); break;
            // -- OVERLAP
            case 0x008E: _fetch(); break;
            
            // 0x34: inc (hl) (M:1 T:4)
            // -- M1
            case 0x008F: _rfsh(); break;
            // -- OVERLAP
            case 0x0090: _fetch(); break;
            
            // 0x35: dec (hl) (M:1 T:4)
            // -- M1
            case 0x0091: _rfsh(); break;
            // -- OVERLAP
            case 0x0092: _fetch(); break;
            
            // 0x36: ld (HL),n (M:3 T:10)
            // -- M1
            case 0x0093: _rfsh(); break;
            // -- M2
            case 0x0094: _mread(cpu->pc++); break;
            case 0x0095: cpu->dlatch=_gd(); break;
            // -- M3
            case 0x0096: _mwrite(cpu->hl,cpu->dlatch); break;
            // -- OVERLAP
            case 0x0097: _fetch(); break;
            
            // 0x37: scf (M:1 T:4)
            // -- M1
            case 0x0098: _rfsh(); break;
            // -- OVERLAP
            case 0x0099: _fetch(); break;
            
            // 0x38: jr c,d (M:1 T:4)
            // -- M1
            case 0x009A: _rfsh(); break;
            // -- OVERLAP
            case 0x009B: _fetch(); break;
            
            // 0x39: add hl,sp (M:1 T:4)
            // -- M1
            case 0x009C: _rfsh(); break;
            // -- OVERLAP
            case 0x009D: _fetch(); break;
            
            // 0x3A: ld a,(nn) (M:4 T:13)
            // -- M1
            case 0x009E: _rfsh(); break;
            // -- M2
            case 0x009F: _mread(cpu->pc++); break;
            case 0x00A0: cpu->wzl=_gd(); break;
            // -- M3
            case 0x00A1: _mread(cpu->pc++); break;
            case 0x00A2: cpu->wzh=_gd(); break;
            // -- M4
            case 0x00A3: _mread(cpu->wz++); break;
            case 0x00A4: cpu->a=_gd(); break;
            // -- OVERLAP
            case 0x00A5: _fetch(); break;
            
            // 0x3B: dec sp (M:1 T:4)
            // -- M1
            case 0x00A6: _rfsh(); break;
            // -- OVERLAP
            case 0x00A7: _fetch(); break;
            
            // 0x3C: inc a (M:1 T:4)
            // -- M1
            case 0x00A8: _rfsh(); break;
            // -- OVERLAP
            case 0x00A9: _fetch(); break;
            
            // 0x3D: dec a (M:1 T:4)
            // -- M1
            case 0x00AA: _rfsh(); break;
            // -- OVERLAP
            case 0x00AB: _fetch(); break;
            
            // 0x3E: ld a,n (M:2 T:7)
            // -- M1
            case 0x00AC: _rfsh(); break;
            // -- M2
            case 0x00AD: _mread(cpu->pc++); break;
            case 0x00AE: cpu->a=_gd(); break;
            // -- OVERLAP
            case 0x00AF: _fetch(); break;
            
            // 0x3F: ccf (M:1 T:4)
            // -- M1
            case 0x00B0: _rfsh(); break;
            // -- OVERLAP
            case 0x00B1: _fetch(); break;
            
            // 0x40: ld b,b (M:1 T:4)
            // -- M1
            case 0x00B2: _rfsh(); break;
            // -- OVERLAP
            case 0x00B3: cpu->b=cpu->b;_fetch(); break;
            
            // 0x41: ld b,c (M:1 T:4)
            // -- M1
            case 0x00B4: _rfsh(); break;
            // -- OVERLAP
            case 0x00B5: cpu->b=cpu->c;_fetch(); break;
            
            // 0x42: ld b,d (M:1 T:4)
            // -- M1
            case 0x00B6: _rfsh(); break;
            // -- OVERLAP
            case 0x00B7: cpu->b=cpu->d;_fetch(); break;
            
            // 0x43: ld b,e (M:1 T:4)
            // -- M1
            case 0x00B8: _rfsh(); break;
            // -- OVERLAP
            case 0x00B9: cpu->b=cpu->e;_fetch(); break;
            
            // 0x44: ld b,h (M:1 T:4)
            // -- M1
            case 0x00BA: _rfsh(); break;
            // -- OVERLAP
            case 0x00BB: cpu->b=cpu->h;_fetch(); break;
            
            // 0x45: ld b,l (M:1 T:4)
            // -- M1
            case 0x00BC: _rfsh(); break;
            // -- OVERLAP
            case 0x00BD: cpu->b=cpu->l;_fetch(); break;
            
            // 0x46: ld b,(hl) (M:2 T:7)
            // -- M1
            case 0x00BE: _rfsh(); break;
            // -- M2
            case 0x00BF: _mread(cpu->hl); break;
            case 0x00C0: cpu->b=_gd(); break;
            // -- OVERLAP
            case 0x00C1: _fetch(); break;
            
            // 0x47: ld b,a (M:1 T:4)
            // -- M1
            case 0x00C2: _rfsh(); break;
            // -- OVERLAP
            case 0x00C3: cpu->b=cpu->a;_fetch(); break;
            
            // 0x48: ld c,b (M:1 T:4)
            // -- M1
            case 0x00C4: _rfsh(); break;
            // -- OVERLAP
            case 0x00C5: cpu->c=cpu->b;_fetch(); break;
            
            // 0x49: ld c,c (M:1 T:4)
            // -- M1
            case 0x00C6: _rfsh(); break;
            // -- OVERLAP
            case 0x00C7: cpu->c=cpu->c;_fetch(); break;
            
            // 0x4A: ld c,d (M:1 T:4)
            // -- M1
            case 0x00C8: _rfsh(); break;
            // -- OVERLAP
            case 0x00C9: cpu->c=cpu->d;_fetch(); break;
            
            // 0x4B: ld c,e (M:1 T:4)
            // -- M1
            case 0x00CA: _rfsh(); break;
            // -- OVERLAP
            case 0x00CB: cpu->c=cpu->e;_fetch(); break;
            
            // 0x4C: ld c,h (M:1 T:4)
            // -- M1
            case 0x00CC: _rfsh(); break;
            // -- OVERLAP
            case 0x00CD: cpu->c=cpu->h;_fetch(); break;
            
            // 0x4D: ld c,l (M:1 T:4)
            // -- M1
            case 0x00CE: _rfsh(); break;
            // -- OVERLAP
            case 0x00CF: cpu->c=cpu->l;_fetch(); break;
            
            // 0x4E: ld c,(hl) (M:2 T:7)
            // -- M1
            case 0x00D0: _rfsh(); break;
            // -- M2
            case 0x00D1: _mread(cpu->hl); break;
            case 0x00D2: cpu->c=_gd(); break;
            // -- OVERLAP
            case 0x00D3: _fetch(); break;
            
            // 0x4F: ld c,a (M:1 T:4)
            // -- M1
            case 0x00D4: _rfsh(); break;
            // -- OVERLAP
            case 0x00D5: cpu->c=cpu->a;_fetch(); break;
            
            // 0x50: ld d,b (M:1 T:4)
            // -- M1
            case 0x00D6: _rfsh(); break;
            // -- OVERLAP
            case 0x00D7: cpu->d=cpu->b;_fetch(); break;
            
            // 0x51: ld d,c (M:1 T:4)
            // -- M1
            case 0x00D8: _rfsh(); break;
            // -- OVERLAP
            case 0x00D9: cpu->d=cpu->c;_fetch(); break;
            
            // 0x52: ld d,d (M:1 T:4)
            // -- M1
            case 0x00DA: _rfsh(); break;
            // -- OVERLAP
            case 0x00DB: cpu->d=cpu->d;_fetch(); break;
            
            // 0x53: ld d,e (M:1 T:4)
            // -- M1
            case 0x00DC: _rfsh(); break;
            // -- OVERLAP
            case 0x00DD: cpu->d=cpu->e;_fetch(); break;
            
            // 0x54: ld d,h (M:1 T:4)
            // -- M1
            case 0x00DE: _rfsh(); break;
            // -- OVERLAP
            case 0x00DF: cpu->d=cpu->h;_fetch(); break;
            
            // 0x55: ld d,l (M:1 T:4)
            // -- M1
            case 0x00E0: _rfsh(); break;
            // -- OVERLAP
            case 0x00E1: cpu->d=cpu->l;_fetch(); break;
            
            // 0x56: ld d,(hl) (M:2 T:7)
            // -- M1
            case 0x00E2: _rfsh(); break;
            // -- M2
            case 0x00E3: _mread(cpu->hl); break;
            case 0x00E4: cpu->d=_gd(); break;
            // -- OVERLAP
            case 0x00E5: _fetch(); break;
            
            // 0x57: ld d,a (M:1 T:4)
            // -- M1
            case 0x00E6: _rfsh(); break;
            // -- OVERLAP
            case 0x00E7: cpu->d=cpu->a;_fetch(); break;
            
            // 0x58: ld e,b (M:1 T:4)
            // -- M1
            case 0x00E8: _rfsh(); break;
            // -- OVERLAP
            case 0x00E9: cpu->e=cpu->b;_fetch(); break;
            
            // 0x59: ld e,c (M:1 T:4)
            // -- M1
            case 0x00EA: _rfsh(); break;
            // -- OVERLAP
            case 0x00EB: cpu->e=cpu->c;_fetch(); break;
            
            // 0x5A: ld e,d (M:1 T:4)
            // -- M1
            case 0x00EC: _rfsh(); break;
            // -- OVERLAP
            case 0x00ED: cpu->e=cpu->d;_fetch(); break;
            
            // 0x5B: ld e,e (M:1 T:4)
            // -- M1
            case 0x00EE: _rfsh(); break;
            // -- OVERLAP
            case 0x00EF: cpu->e=cpu->e;_fetch(); break;
            
            // 0x5C: ld e,h (M:1 T:4)
            // -- M1
            case 0x00F0: _rfsh(); break;
            // -- OVERLAP
            case 0x00F1: cpu->e=cpu->h;_fetch(); break;
            
            // 0x5D: ld e,l (M:1 T:4)
            // -- M1
            case 0x00F2: _rfsh(); break;
            // -- OVERLAP
            case 0x00F3: cpu->e=cpu->l;_fetch(); break;
            
            // 0x5E: ld e,(hl) (M:2 T:7)
            // -- M1
            case 0x00F4: _rfsh(); break;
            // -- M2
            case 0x00F5: _mread(cpu->hl); break;
            case 0x00F6: cpu->e=_gd(); break;
            // -- OVERLAP
            case 0x00F7: _fetch(); break;
            
            // 0x5F: ld e,a (M:1 T:4)
            // -- M1
            case 0x00F8: _rfsh(); break;
            // -- OVERLAP
            case 0x00F9: cpu->e=cpu->a;_fetch(); break;
            
            // 0x60: ld h,b (M:1 T:4)
            // -- M1
            case 0x00FA: _rfsh(); break;
            // -- OVERLAP
            case 0x00FB: cpu->h=cpu->b;_fetch(); break;
            
            // 0x61: ld h,c (M:1 T:4)
            // -- M1
            case 0x00FC: _rfsh(); break;
            // -- OVERLAP
            case 0x00FD: cpu->h=cpu->c;_fetch(); break;
            
            // 0x62: ld h,d (M:1 T:4)
            // -- M1
            case 0x00FE: _rfsh(); break;
            // -- OVERLAP
            case 0x00FF: cpu->h=cpu->d;_fetch(); break;
            
            // 0x63: ld h,e (M:1 T:4)
            // -- M1
            case 0x0100: _rfsh(); break;
            // -- OVERLAP
            case 0x0101: cpu->h=cpu->e;_fetch(); break;
            
            // 0x64: ld h,h (M:1 T:4)
            // -- M1
            case 0x0102: _rfsh(); break;
            // -- OVERLAP
            case 0x0103: cpu->h=cpu->h;_fetch(); break;
            
            // 0x65: ld h,l (M:1 T:4)
            // -- M1
            case 0x0104: _rfsh(); break;
            // -- OVERLAP
            case 0x0105: cpu->h=cpu->l;_fetch(); break;
            
            // 0x66: ld h,(hl) (M:2 T:7)
            // -- M1
            case 0x0106: _rfsh(); break;
            // -- M2
            case 0x0107: _mread(cpu->hl); break;
            case 0x0108: cpu->h=_gd(); break;
            // -- OVERLAP
            case 0x0109: _fetch(); break;
            
            // 0x67: ld h,a (M:1 T:4)
            // -- M1
            case 0x010A: _rfsh(); break;
            // -- OVERLAP
            case 0x010B: cpu->h=cpu->a;_fetch(); break;
            
            // 0x68: ld l,b (M:1 T:4)
            // -- M1
            case 0x010C: _rfsh(); break;
            // -- OVERLAP
            case 0x010D: cpu->l=cpu->b;_fetch(); break;
            
            // 0x69: ld l,c (M:1 T:4)
            // -- M1
            case 0x010E: _rfsh(); break;
            // -- OVERLAP
            case 0x010F: cpu->l=cpu->c;_fetch(); break;
            
            // 0x6A: ld l,d (M:1 T:4)
            // -- M1
            case 0x0110: _rfsh(); break;
            // -- OVERLAP
            case 0x0111: cpu->l=cpu->d;_fetch(); break;
            
            // 0x6B: ld l,e (M:1 T:4)
            // -- M1
            case 0x0112: _rfsh(); break;
            // -- OVERLAP
            case 0x0113: cpu->l=cpu->e;_fetch(); break;
            
            // 0x6C: ld l,h (M:1 T:4)
            // -- M1
            case 0x0114: _rfsh(); break;
            // -- OVERLAP
            case 0x0115: cpu->l=cpu->h;_fetch(); break;
            
            // 0x6D: ld l,l (M:1 T:4)
            // -- M1
            case 0x0116: _rfsh(); break;
            // -- OVERLAP
            case 0x0117: cpu->l=cpu->l;_fetch(); break;
            
            // 0x6E: ld l,(hl) (M:2 T:7)
            // -- M1
            case 0x0118: _rfsh(); break;
            // -- M2
            case 0x0119: _mread(cpu->hl); break;
            case 0x011A: cpu->l=_gd(); break;
            // -- OVERLAP
            case 0x011B: _fetch(); break;
            
            // 0x6F: ld l,a (M:1 T:4)
            // -- M1
            case 0x011C: _rfsh(); break;
            // -- OVERLAP
            case 0x011D: cpu->l=cpu->a;_fetch(); break;
            
            // 0x70: ld (hl),b (M:2 T:7)
            // -- M1
            case 0x011E: _rfsh(); break;
            // -- M2
            case 0x011F: _mwrite(cpu->hl,cpu->b); break;
            // -- OVERLAP
            case 0x0120: _fetch(); break;
            
            // 0x71: ld (hl),c (M:2 T:7)
            // -- M1
            case 0x0121: _rfsh(); break;
            // -- M2
            case 0x0122: _mwrite(cpu->hl,cpu->c); break;
            // -- OVERLAP
            case 0x0123: _fetch(); break;
            
            // 0x72: ld (hl),d (M:2 T:7)
            // -- M1
            case 0x0124: _rfsh(); break;
            // -- M2
            case 0x0125: _mwrite(cpu->hl,cpu->d); break;
            // -- OVERLAP
            case 0x0126: _fetch(); break;
            
            // 0x73: ld (hl),e (M:2 T:7)
            // -- M1
            case 0x0127: _rfsh(); break;
            // -- M2
            case 0x0128: _mwrite(cpu->hl,cpu->e); break;
            // -- OVERLAP
            case 0x0129: _fetch(); break;
            
            // 0x74: ld (hl),h (M:2 T:7)
            // -- M1
            case 0x012A: _rfsh(); break;
            // -- M2
            case 0x012B: _mwrite(cpu->hl,cpu->h); break;
            // -- OVERLAP
            case 0x012C: _fetch(); break;
            
            // 0x75: ld (hl),l (M:2 T:7)
            // -- M1
            case 0x012D: _rfsh(); break;
            // -- M2
            case 0x012E: _mwrite(cpu->hl,cpu->l); break;
            // -- OVERLAP
            case 0x012F: _fetch(); break;
            
            // 0x76: halt (M:1 T:4)
            // -- M1
            case 0x0130: _rfsh(); break;
            // -- OVERLAP
            case 0x0131: z80_halt(cpu);_fetch(); break;
            
            // 0x77: ld (hl),a (M:2 T:7)
            // -- M1
            case 0x0132: _rfsh(); break;
            // -- M2
            case 0x0133: _mwrite(cpu->hl,cpu->a); break;
            // -- OVERLAP
            case 0x0134: _fetch(); break;
            
            // 0x78: ld a,b (M:1 T:4)
            // -- M1
            case 0x0135: _rfsh(); break;
            // -- OVERLAP
            case 0x0136: cpu->a=cpu->b;_fetch(); break;
            
            // 0x79: ld a,c (M:1 T:4)
            // -- M1
            case 0x0137: _rfsh(); break;
            // -- OVERLAP
            case 0x0138: cpu->a=cpu->c;_fetch(); break;
            
            // 0x7A: ld a,d (M:1 T:4)
            // -- M1
            case 0x0139: _rfsh(); break;
            // -- OVERLAP
            case 0x013A: cpu->a=cpu->d;_fetch(); break;
            
            // 0x7B: ld a,e (M:1 T:4)
            // -- M1
            case 0x013B: _rfsh(); break;
            // -- OVERLAP
            case 0x013C: cpu->a=cpu->e;_fetch(); break;
            
            // 0x7C: ld a,h (M:1 T:4)
            // -- M1
            case 0x013D: _rfsh(); break;
            // -- OVERLAP
            case 0x013E: cpu->a=cpu->h;_fetch(); break;
            
            // 0x7D: ld a,l (M:1 T:4)
            // -- M1
            case 0x013F: _rfsh(); break;
            // -- OVERLAP
            case 0x0140: cpu->a=cpu->l;_fetch(); break;
            
            // 0x7E: ld a,(hl) (M:2 T:7)
            // -- M1
            case 0x0141: _rfsh(); break;
            // -- M2
            case 0x0142: _mread(cpu->hl); break;
            case 0x0143: cpu->a=_gd(); break;
            // -- OVERLAP
            case 0x0144: _fetch(); break;
            
            // 0x7F: ld a,a (M:1 T:4)
            // -- M1
            case 0x0145: _rfsh(); break;
            // -- OVERLAP
            case 0x0146: cpu->a=cpu->a;_fetch(); break;
            
            // 0x80: add b (M:1 T:4)
            // -- M1
            case 0x0147: _rfsh(); break;
            // -- OVERLAP
            case 0x0148: z80_add(cpu,cpu->b);_fetch(); break;
            
            // 0x81: add c (M:1 T:4)
            // -- M1
            case 0x0149: _rfsh(); break;
            // -- OVERLAP
            case 0x014A: z80_add(cpu,cpu->c);_fetch(); break;
            
            // 0x82: add d (M:1 T:4)
            // -- M1
            case 0x014B: _rfsh(); break;
            // -- OVERLAP
            case 0x014C: z80_add(cpu,cpu->d);_fetch(); break;
            
            // 0x83: add e (M:1 T:4)
            // -- M1
            case 0x014D: _rfsh(); break;
            // -- OVERLAP
            case 0x014E: z80_add(cpu,cpu->e);_fetch(); break;
            
            // 0x84: add h (M:1 T:4)
            // -- M1
            case 0x014F: _rfsh(); break;
            // -- OVERLAP
            case 0x0150: z80_add(cpu,cpu->h);_fetch(); break;
            
            // 0x85: add l (M:1 T:4)
            // -- M1
            case 0x0151: _rfsh(); break;
            // -- OVERLAP
            case 0x0152: z80_add(cpu,cpu->l);_fetch(); break;
            
            // 0x86: add (hl) (M:2 T:7)
            // -- M1
            case 0x0153: _rfsh(); break;
            // -- M2
            case 0x0154: _mread(cpu->hl); break;
            case 0x0155: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x0156: z80_add(cpu,cpu->dlatch);_fetch(); break;
            
            // 0x87: add a (M:1 T:4)
            // -- M1
            case 0x0157: _rfsh(); break;
            // -- OVERLAP
            case 0x0158: z80_add(cpu,cpu->a);_fetch(); break;
            
            // 0x88: adc b (M:1 T:4)
            // -- M1
            case 0x0159: _rfsh(); break;
            // -- OVERLAP
            case 0x015A: z80_adc(cpu,cpu->b);_fetch(); break;
            
            // 0x89: adc c (M:1 T:4)
            // -- M1
            case 0x015B: _rfsh(); break;
            // -- OVERLAP
            case 0x015C: z80_adc(cpu,cpu->c);_fetch(); break;
            
            // 0x8A: adc d (M:1 T:4)
            // -- M1
            case 0x015D: _rfsh(); break;
            // -- OVERLAP
            case 0x015E: z80_adc(cpu,cpu->d);_fetch(); break;
            
            // 0x8B: adc e (M:1 T:4)
            // -- M1
            case 0x015F: _rfsh(); break;
            // -- OVERLAP
            case 0x0160: z80_adc(cpu,cpu->e);_fetch(); break;
            
            // 0x8C: adc h (M:1 T:4)
            // -- M1
            case 0x0161: _rfsh(); break;
            // -- OVERLAP
            case 0x0162: z80_adc(cpu,cpu->h);_fetch(); break;
            
            // 0x8D: adc l (M:1 T:4)
            // -- M1
            case 0x0163: _rfsh(); break;
            // -- OVERLAP
            case 0x0164: z80_adc(cpu,cpu->l);_fetch(); break;
            
            // 0x8E: adc (hl) (M:2 T:7)
            // -- M1
            case 0x0165: _rfsh(); break;
            // -- M2
            case 0x0166: _mread(cpu->hl); break;
            case 0x0167: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x0168: z80_adc(cpu,cpu->dlatch);_fetch(); break;
            
            // 0x8F: adc a (M:1 T:4)
            // -- M1
            case 0x0169: _rfsh(); break;
            // -- OVERLAP
            case 0x016A: z80_adc(cpu,cpu->a);_fetch(); break;
            
            // 0x90: sub b (M:1 T:4)
            // -- M1
            case 0x016B: _rfsh(); break;
            // -- OVERLAP
            case 0x016C: z80_sub(cpu,cpu->b);_fetch(); break;
            
            // 0x91: sub c (M:1 T:4)
            // -- M1
            case 0x016D: _rfsh(); break;
            // -- OVERLAP
            case 0x016E: z80_sub(cpu,cpu->c);_fetch(); break;
            
            // 0x92: sub d (M:1 T:4)
            // -- M1
            case 0x016F: _rfsh(); break;
            // -- OVERLAP
            case 0x0170: z80_sub(cpu,cpu->d);_fetch(); break;
            
            // 0x93: sub e (M:1 T:4)
            // -- M1
            case 0x0171: _rfsh(); break;
            // -- OVERLAP
            case 0x0172: z80_sub(cpu,cpu->e);_fetch(); break;
            
            // 0x94: sub h (M:1 T:4)
            // -- M1
            case 0x0173: _rfsh(); break;
            // -- OVERLAP
            case 0x0174: z80_sub(cpu,cpu->h);_fetch(); break;
            
            // 0x95: sub l (M:1 T:4)
            // -- M1
            case 0x0175: _rfsh(); break;
            // -- OVERLAP
            case 0x0176: z80_sub(cpu,cpu->l);_fetch(); break;
            
            // 0x96: sub (hl) (M:2 T:7)
            // -- M1
            case 0x0177: _rfsh(); break;
            // -- M2
            case 0x0178: _mread(cpu->hl); break;
            case 0x0179: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x017A: z80_sub(cpu,cpu->dlatch);_fetch(); break;
            
            // 0x97: sub a (M:1 T:4)
            // -- M1
            case 0x017B: _rfsh(); break;
            // -- OVERLAP
            case 0x017C: z80_sub(cpu,cpu->a);_fetch(); break;
            
            // 0x98: sbc b (M:1 T:4)
            // -- M1
            case 0x017D: _rfsh(); break;
            // -- OVERLAP
            case 0x017E: z80_sbc(cpu,cpu->b);_fetch(); break;
            
            // 0x99: sbc c (M:1 T:4)
            // -- M1
            case 0x017F: _rfsh(); break;
            // -- OVERLAP
            case 0x0180: z80_sbc(cpu,cpu->c);_fetch(); break;
            
            // 0x9A: sbc d (M:1 T:4)
            // -- M1
            case 0x0181: _rfsh(); break;
            // -- OVERLAP
            case 0x0182: z80_sbc(cpu,cpu->d);_fetch(); break;
            
            // 0x9B: sbc e (M:1 T:4)
            // -- M1
            case 0x0183: _rfsh(); break;
            // -- OVERLAP
            case 0x0184: z80_sbc(cpu,cpu->e);_fetch(); break;
            
            // 0x9C: sbc h (M:1 T:4)
            // -- M1
            case 0x0185: _rfsh(); break;
            // -- OVERLAP
            case 0x0186: z80_sbc(cpu,cpu->h);_fetch(); break;
            
            // 0x9D: sbc l (M:1 T:4)
            // -- M1
            case 0x0187: _rfsh(); break;
            // -- OVERLAP
            case 0x0188: z80_sbc(cpu,cpu->l);_fetch(); break;
            
            // 0x9E: sbc (hl) (M:2 T:7)
            // -- M1
            case 0x0189: _rfsh(); break;
            // -- M2
            case 0x018A: _mread(cpu->hl); break;
            case 0x018B: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x018C: z80_sbc(cpu,cpu->dlatch);_fetch(); break;
            
            // 0x9F: sbc a (M:1 T:4)
            // -- M1
            case 0x018D: _rfsh(); break;
            // -- OVERLAP
            case 0x018E: z80_sbc(cpu,cpu->a);_fetch(); break;
            
            // 0xA0: and b (M:1 T:4)
            // -- M1
            case 0x018F: _rfsh(); break;
            // -- OVERLAP
            case 0x0190: z80_and(cpu,cpu->b);_fetch(); break;
            
            // 0xA1: and c (M:1 T:4)
            // -- M1
            case 0x0191: _rfsh(); break;
            // -- OVERLAP
            case 0x0192: z80_and(cpu,cpu->c);_fetch(); break;
            
            // 0xA2: and d (M:1 T:4)
            // -- M1
            case 0x0193: _rfsh(); break;
            // -- OVERLAP
            case 0x0194: z80_and(cpu,cpu->d);_fetch(); break;
            
            // 0xA3: and e (M:1 T:4)
            // -- M1
            case 0x0195: _rfsh(); break;
            // -- OVERLAP
            case 0x0196: z80_and(cpu,cpu->e);_fetch(); break;
            
            // 0xA4: and h (M:1 T:4)
            // -- M1
            case 0x0197: _rfsh(); break;
            // -- OVERLAP
            case 0x0198: z80_and(cpu,cpu->h);_fetch(); break;
            
            // 0xA5: and l (M:1 T:4)
            // -- M1
            case 0x0199: _rfsh(); break;
            // -- OVERLAP
            case 0x019A: z80_and(cpu,cpu->l);_fetch(); break;
            
            // 0xA6: and (hl) (M:2 T:7)
            // -- M1
            case 0x019B: _rfsh(); break;
            // -- M2
            case 0x019C: _mread(cpu->hl); break;
            case 0x019D: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x019E: z80_and(cpu,cpu->dlatch);_fetch(); break;
            
            // 0xA7: and a (M:1 T:4)
            // -- M1
            case 0x019F: _rfsh(); break;
            // -- OVERLAP
            case 0x01A0: z80_and(cpu,cpu->a);_fetch(); break;
            
            // 0xA8: xor b (M:1 T:4)
            // -- M1
            case 0x01A1: _rfsh(); break;
            // -- OVERLAP
            case 0x01A2: z80_xor(cpu,cpu->b);_fetch(); break;
            
            // 0xA9: xor c (M:1 T:4)
            // -- M1
            case 0x01A3: _rfsh(); break;
            // -- OVERLAP
            case 0x01A4: z80_xor(cpu,cpu->c);_fetch(); break;
            
            // 0xAA: xor d (M:1 T:4)
            // -- M1
            case 0x01A5: _rfsh(); break;
            // -- OVERLAP
            case 0x01A6: z80_xor(cpu,cpu->d);_fetch(); break;
            
            // 0xAB: xor e (M:1 T:4)
            // -- M1
            case 0x01A7: _rfsh(); break;
            // -- OVERLAP
            case 0x01A8: z80_xor(cpu,cpu->e);_fetch(); break;
            
            // 0xAC: xor h (M:1 T:4)
            // -- M1
            case 0x01A9: _rfsh(); break;
            // -- OVERLAP
            case 0x01AA: z80_xor(cpu,cpu->h);_fetch(); break;
            
            // 0xAD: xor l (M:1 T:4)
            // -- M1
            case 0x01AB: _rfsh(); break;
            // -- OVERLAP
            case 0x01AC: z80_xor(cpu,cpu->l);_fetch(); break;
            
            // 0xAE: xor (hl) (M:2 T:7)
            // -- M1
            case 0x01AD: _rfsh(); break;
            // -- M2
            case 0x01AE: _mread(cpu->hl); break;
            case 0x01AF: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x01B0: z80_xor(cpu,cpu->dlatch);_fetch(); break;
            
            // 0xAF: xor a (M:1 T:4)
            // -- M1
            case 0x01B1: _rfsh(); break;
            // -- OVERLAP
            case 0x01B2: z80_xor(cpu,cpu->a);_fetch(); break;
            
            // 0xB0: or b (M:1 T:4)
            // -- M1
            case 0x01B3: _rfsh(); break;
            // -- OVERLAP
            case 0x01B4: z80_or(cpu,cpu->b);_fetch(); break;
            
            // 0xB1: or c (M:1 T:4)
            // -- M1
            case 0x01B5: _rfsh(); break;
            // -- OVERLAP
            case 0x01B6: z80_or(cpu,cpu->c);_fetch(); break;
            
            // 0xB2: or d (M:1 T:4)
            // -- M1
            case 0x01B7: _rfsh(); break;
            // -- OVERLAP
            case 0x01B8: z80_or(cpu,cpu->d);_fetch(); break;
            
            // 0xB3: or e (M:1 T:4)
            // -- M1
            case 0x01B9: _rfsh(); break;
            // -- OVERLAP
            case 0x01BA: z80_or(cpu,cpu->e);_fetch(); break;
            
            // 0xB4: or h (M:1 T:4)
            // -- M1
            case 0x01BB: _rfsh(); break;
            // -- OVERLAP
            case 0x01BC: z80_or(cpu,cpu->h);_fetch(); break;
            
            // 0xB5: or l (M:1 T:4)
            // -- M1
            case 0x01BD: _rfsh(); break;
            // -- OVERLAP
            case 0x01BE: z80_or(cpu,cpu->l);_fetch(); break;
            
            // 0xB6: or (hl) (M:2 T:7)
            // -- M1
            case 0x01BF: _rfsh(); break;
            // -- M2
            case 0x01C0: _mread(cpu->hl); break;
            case 0x01C1: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x01C2: z80_or(cpu,cpu->dlatch);_fetch(); break;
            
            // 0xB7: or a (M:1 T:4)
            // -- M1
            case 0x01C3: _rfsh(); break;
            // -- OVERLAP
            case 0x01C4: z80_or(cpu,cpu->a);_fetch(); break;
            
            // 0xB8: cp b (M:1 T:4)
            // -- M1
            case 0x01C5: _rfsh(); break;
            // -- OVERLAP
            case 0x01C6: z80_cp(cpu,cpu->b);_fetch(); break;
            
            // 0xB9: cp c (M:1 T:4)
            // -- M1
            case 0x01C7: _rfsh(); break;
            // -- OVERLAP
            case 0x01C8: z80_cp(cpu,cpu->c);_fetch(); break;
            
            // 0xBA: cp d (M:1 T:4)
            // -- M1
            case 0x01C9: _rfsh(); break;
            // -- OVERLAP
            case 0x01CA: z80_cp(cpu,cpu->d);_fetch(); break;
            
            // 0xBB: cp e (M:1 T:4)
            // -- M1
            case 0x01CB: _rfsh(); break;
            // -- OVERLAP
            case 0x01CC: z80_cp(cpu,cpu->e);_fetch(); break;
            
            // 0xBC: cp h (M:1 T:4)
            // -- M1
            case 0x01CD: _rfsh(); break;
            // -- OVERLAP
            case 0x01CE: z80_cp(cpu,cpu->h);_fetch(); break;
            
            // 0xBD: cp l (M:1 T:4)
            // -- M1
            case 0x01CF: _rfsh(); break;
            // -- OVERLAP
            case 0x01D0: z80_cp(cpu,cpu->l);_fetch(); break;
            
            // 0xBE: cp (hl) (M:2 T:7)
            // -- M1
            case 0x01D1: _rfsh(); break;
            // -- M2
            case 0x01D2: _mread(cpu->hl); break;
            case 0x01D3: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x01D4: z80_cp(cpu,cpu->dlatch);_fetch(); break;
            
            // 0xBF: cp a (M:1 T:4)
            // -- M1
            case 0x01D5: _rfsh(); break;
            // -- OVERLAP
            case 0x01D6: z80_cp(cpu,cpu->a);_fetch(); break;
            
            // 0xC0: ret nz (M:1 T:4)
            // -- M1
            case 0x01D7: _rfsh(); break;
            // -- OVERLAP
            case 0x01D8: _fetch(); break;
            
            // 0xC1: pop bc2 (M:1 T:4)
            // -- M1
            case 0x01D9: _rfsh(); break;
            // -- OVERLAP
            case 0x01DA: _fetch(); break;
            
            // 0xC2: jp nz,nn (M:1 T:4)
            // -- M1
            case 0x01DB: _rfsh(); break;
            // -- OVERLAP
            case 0x01DC: _fetch(); break;
            
            // 0xC3: jp nn (M:1 T:4)
            // -- M1
            case 0x01DD: _rfsh(); break;
            // -- OVERLAP
            case 0x01DE: _fetch(); break;
            
            // 0xC4: call nz,nn (M:1 T:4)
            // -- M1
            case 0x01DF: _rfsh(); break;
            // -- OVERLAP
            case 0x01E0: _fetch(); break;
            
            // 0xC5: push bc2 (M:1 T:4)
            // -- M1
            case 0x01E1: _rfsh(); break;
            // -- OVERLAP
            case 0x01E2: _fetch(); break;
            
            // 0xC6: add n (M:1 T:4)
            // -- M1
            case 0x01E3: _rfsh(); break;
            // -- OVERLAP
            case 0x01E4: _fetch(); break;
            
            // 0xC7: rst 0h (M:1 T:4)
            // -- M1
            case 0x01E5: _rfsh(); break;
            // -- OVERLAP
            case 0x01E6: _fetch(); break;
            
            // 0xC8: ret z (M:1 T:4)
            // -- M1
            case 0x01E7: _rfsh(); break;
            // -- OVERLAP
            case 0x01E8: _fetch(); break;
            
            // 0xC9: ret (M:1 T:4)
            // -- M1
            case 0x01E9: _rfsh(); break;
            // -- OVERLAP
            case 0x01EA: _fetch(); break;
            
            // 0xCA: jp z,nn (M:1 T:4)
            // -- M1
            case 0x01EB: _rfsh(); break;
            // -- OVERLAP
            case 0x01EC: _fetch(); break;
            
            // 0xCB: cb prefix (M:1 T:4)
            // -- M1
            case 0x01ED: _rfsh(); break;
            // -- OVERLAP
            case 0x01EE: _fetch(); break;
            
            // 0xCC: call z,nn (M:1 T:4)
            // -- M1
            case 0x01EF: _rfsh(); break;
            // -- OVERLAP
            case 0x01F0: _fetch(); break;
            
            // 0xCD: call nn (M:1 T:4)
            // -- M1
            case 0x01F1: _rfsh(); break;
            // -- OVERLAP
            case 0x01F2: _fetch(); break;
            
            // 0xCE: adc n (M:1 T:4)
            // -- M1
            case 0x01F3: _rfsh(); break;
            // -- OVERLAP
            case 0x01F4: _fetch(); break;
            
            // 0xCF: rst 8h (M:1 T:4)
            // -- M1
            case 0x01F5: _rfsh(); break;
            // -- OVERLAP
            case 0x01F6: _fetch(); break;
            
            // 0xD0: ret nc (M:1 T:4)
            // -- M1
            case 0x01F7: _rfsh(); break;
            // -- OVERLAP
            case 0x01F8: _fetch(); break;
            
            // 0xD1: pop de2 (M:1 T:4)
            // -- M1
            case 0x01F9: _rfsh(); break;
            // -- OVERLAP
            case 0x01FA: _fetch(); break;
            
            // 0xD2: jp nc,nn (M:1 T:4)
            // -- M1
            case 0x01FB: _rfsh(); break;
            // -- OVERLAP
            case 0x01FC: _fetch(); break;
            
            // 0xD3: out (n),a (M:1 T:4)
            // -- M1
            case 0x01FD: _rfsh(); break;
            // -- OVERLAP
            case 0x01FE: _fetch(); break;
            
            // 0xD4: call nc,nn (M:1 T:4)
            // -- M1
            case 0x01FF: _rfsh(); break;
            // -- OVERLAP
            case 0x0200: _fetch(); break;
            
            // 0xD5: push de2 (M:1 T:4)
            // -- M1
            case 0x0201: _rfsh(); break;
            // -- OVERLAP
            case 0x0202: _fetch(); break;
            
            // 0xD6: sub n (M:1 T:4)
            // -- M1
            case 0x0203: _rfsh(); break;
            // -- OVERLAP
            case 0x0204: _fetch(); break;
            
            // 0xD7: rst 10h (M:1 T:4)
            // -- M1
            case 0x0205: _rfsh(); break;
            // -- OVERLAP
            case 0x0206: _fetch(); break;
            
            // 0xD8: ret c (M:1 T:4)
            // -- M1
            case 0x0207: _rfsh(); break;
            // -- OVERLAP
            case 0x0208: _fetch(); break;
            
            // 0xD9: exx (M:1 T:4)
            // -- M1
            case 0x0209: _rfsh(); break;
            // -- OVERLAP
            case 0x020A: _fetch(); break;
            
            // 0xDA: jp c,nn (M:1 T:4)
            // -- M1
            case 0x020B: _rfsh(); break;
            // -- OVERLAP
            case 0x020C: _fetch(); break;
            
            // 0xDB: in a,(n) (M:1 T:4)
            // -- M1
            case 0x020D: _rfsh(); break;
            // -- OVERLAP
            case 0x020E: _fetch(); break;
            
            // 0xDC: call c,nn (M:1 T:4)
            // -- M1
            case 0x020F: _rfsh(); break;
            // -- OVERLAP
            case 0x0210: _fetch(); break;
            
            // 0xDD: dd prefix (M:1 T:4)
            // -- M1
            case 0x0211: _rfsh(); break;
            // -- OVERLAP
            case 0x0212: _fetch(); break;
            
            // 0xDE: sbc n (M:1 T:4)
            // -- M1
            case 0x0213: _rfsh(); break;
            // -- OVERLAP
            case 0x0214: _fetch(); break;
            
            // 0xDF: rst 18h (M:1 T:4)
            // -- M1
            case 0x0215: _rfsh(); break;
            // -- OVERLAP
            case 0x0216: _fetch(); break;
            
            // 0xE0: ret po (M:1 T:4)
            // -- M1
            case 0x0217: _rfsh(); break;
            // -- OVERLAP
            case 0x0218: _fetch(); break;
            
            // 0xE1: pop hl2 (M:1 T:4)
            // -- M1
            case 0x0219: _rfsh(); break;
            // -- OVERLAP
            case 0x021A: _fetch(); break;
            
            // 0xE2: jp po,nn (M:1 T:4)
            // -- M1
            case 0x021B: _rfsh(); break;
            // -- OVERLAP
            case 0x021C: _fetch(); break;
            
            // 0xE3: ex (sp),hl (M:1 T:4)
            // -- M1
            case 0x021D: _rfsh(); break;
            // -- OVERLAP
            case 0x021E: _fetch(); break;
            
            // 0xE4: call po,nn (M:1 T:4)
            // -- M1
            case 0x021F: _rfsh(); break;
            // -- OVERLAP
            case 0x0220: _fetch(); break;
            
            // 0xE5: push hl2 (M:1 T:4)
            // -- M1
            case 0x0221: _rfsh(); break;
            // -- OVERLAP
            case 0x0222: _fetch(); break;
            
            // 0xE6: and n (M:1 T:4)
            // -- M1
            case 0x0223: _rfsh(); break;
            // -- OVERLAP
            case 0x0224: _fetch(); break;
            
            // 0xE7: rst 20h (M:1 T:4)
            // -- M1
            case 0x0225: _rfsh(); break;
            // -- OVERLAP
            case 0x0226: _fetch(); break;
            
            // 0xE8: ret pe (M:1 T:4)
            // -- M1
            case 0x0227: _rfsh(); break;
            // -- OVERLAP
            case 0x0228: _fetch(); break;
            
            // 0xE9: jp hl (M:1 T:4)
            // -- M1
            case 0x0229: _rfsh(); break;
            // -- OVERLAP
            case 0x022A: _fetch(); break;
            
            // 0xEA: jp pe,nn (M:1 T:4)
            // -- M1
            case 0x022B: _rfsh(); break;
            // -- OVERLAP
            case 0x022C: _fetch(); break;
            
            // 0xEB: ex de,hl (M:1 T:4)
            // -- M1
            case 0x022D: _rfsh(); break;
            // -- OVERLAP
            case 0x022E: _fetch(); break;
            
            // 0xEC: call pe,nn (M:1 T:4)
            // -- M1
            case 0x022F: _rfsh(); break;
            // -- OVERLAP
            case 0x0230: _fetch(); break;
            
            // 0xED: ed prefix (M:1 T:4)
            // -- M1
            case 0x0231: _rfsh(); break;
            // -- OVERLAP
            case 0x0232: _fetch(); break;
            
            // 0xEE: xor n (M:1 T:4)
            // -- M1
            case 0x0233: _rfsh(); break;
            // -- OVERLAP
            case 0x0234: _fetch(); break;
            
            // 0xEF: rst 28h (M:1 T:4)
            // -- M1
            case 0x0235: _rfsh(); break;
            // -- OVERLAP
            case 0x0236: _fetch(); break;
            
            // 0xF0: ret p (M:1 T:4)
            // -- M1
            case 0x0237: _rfsh(); break;
            // -- OVERLAP
            case 0x0238: _fetch(); break;
            
            // 0xF1: pop sp2 (M:1 T:4)
            // -- M1
            case 0x0239: _rfsh(); break;
            // -- OVERLAP
            case 0x023A: _fetch(); break;
            
            // 0xF2: jp p,nn (M:1 T:4)
            // -- M1
            case 0x023B: _rfsh(); break;
            // -- OVERLAP
            case 0x023C: _fetch(); break;
            
            // 0xF3: di (M:1 T:4)
            // -- M1
            case 0x023D: _rfsh(); break;
            // -- OVERLAP
            case 0x023E: _fetch(); break;
            
            // 0xF4: call p,nn (M:1 T:4)
            // -- M1
            case 0x023F: _rfsh(); break;
            // -- OVERLAP
            case 0x0240: _fetch(); break;
            
            // 0xF5: push sp2 (M:1 T:4)
            // -- M1
            case 0x0241: _rfsh(); break;
            // -- OVERLAP
            case 0x0242: _fetch(); break;
            
            // 0xF6: or n (M:1 T:4)
            // -- M1
            case 0x0243: _rfsh(); break;
            // -- OVERLAP
            case 0x0244: _fetch(); break;
            
            // 0xF7: rst 30h (M:1 T:4)
            // -- M1
            case 0x0245: _rfsh(); break;
            // -- OVERLAP
            case 0x0246: _fetch(); break;
            
            // 0xF8: ret m (M:1 T:4)
            // -- M1
            case 0x0247: _rfsh(); break;
            // -- OVERLAP
            case 0x0248: _fetch(); break;
            
            // 0xF9: ld sp,hl (M:1 T:4)
            // -- M1
            case 0x0249: _rfsh(); break;
            // -- OVERLAP
            case 0x024A: _fetch(); break;
            
            // 0xFA: jp m,nn (M:1 T:4)
            // -- M1
            case 0x024B: _rfsh(); break;
            // -- OVERLAP
            case 0x024C: _fetch(); break;
            
            // 0xFB: ei (M:1 T:4)
            // -- M1
            case 0x024D: _rfsh(); break;
            // -- OVERLAP
            case 0x024E: _fetch(); break;
            
            // 0xFC: call m,nn (M:1 T:4)
            // -- M1
            case 0x024F: _rfsh(); break;
            // -- OVERLAP
            case 0x0250: _fetch(); break;
            
            // 0xFD: fd prefix (M:1 T:4)
            // -- M1
            case 0x0251: _rfsh(); break;
            // -- OVERLAP
            case 0x0252: _fetch(); break;
            
            // 0xFE: cp n (M:1 T:4)
            // -- M1
            case 0x0253: _rfsh(); break;
            // -- OVERLAP
            case 0x0254: _fetch(); break;
            
            // 0xFF: rst 38h (M:1 T:4)
            // -- M1
            case 0x0255: _rfsh(); break;
            // -- OVERLAP
            case 0x0256: _fetch(); break;

        }
    }
    // advance the decode pipeline by one tcycle
    cpu->op.pip = (cpu->op.pip & ~Z80_PIP_BITS) >> 1;
    cpu->pins = pins;
    return pins;
}

#undef _sa
#undef _sax
#undef _sad
#undef _sadx
#undef _gd
#undef _fetch
#undef _rfsh
#undef _mread
#undef _mwrite
#undef _ioread
#undef _iowrite

#endif // CHIPS_IMPL