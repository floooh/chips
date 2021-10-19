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

// flags for z80_opstate.flags
#define Z80_OPSTATE_FLAGS_INDIRECT  (1<<0)  // this is a (HL)/(IX+d)/(IY+d) instruction
#define Z80_OPSTATE_FLAGS_IMM8 (1<<1)       // this is an 8-bit immediate load instruction

// values for hlx_idx for mapping HL, IX or IY
#define Z80_MAP_HL (0)
#define Z80_MAP_IX (1)
#define Z80_MAP_IY (2)

typedef struct {
    uint32_t pip;   // the op's decode pipeline
    uint16_t step;  // first or current decoder switch-case branch step
    uint16_t flags; // Z80_OPSTATE_FLAGS_
} z80_opstate_t;

// CPU state
typedef struct {
    z80_opstate_t op;   // the currently active op
    union { struct { uint8_t pcl; uint8_t pch; }; uint16_t pc; };
    uint16_t addr;      // effective address for (HL),(IX+d),(IY+d)
    uint8_t ir;         // instruction register
    uint8_t dlatch;     // temporary store for data bus value
    uint8_t hlx_idx;    // index into hlx[] for mapping hl to ix or iy (0: hl, 1: ix, 2: iy)

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
    uint8_t i;
    uint8_t r;
    uint8_t im;
    uint16_t af2, bc2, de2, hl2; // shadow register bank
} z80_t;

// initialize a new Z80 instance and return initial pin mask
uint64_t z80_init(z80_t* cpu);
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

uint64_t z80_init(z80_t* cpu) {
    CHIPS_ASSERT(cpu);
    memset(cpu, 0, sizeof(z80_t));
    // initial state according to visualz80
    cpu->af = cpu->bc = cpu->de = cpu->hl = 0x5555;
    cpu->wz = cpu->sp = cpu->ix = cpu->iy = 0x5555;
    cpu->af2 = cpu->bc2 = cpu->de2 = cpu->hl2 = 0x5555;
    // FIXME: iff1/2 disabled, initial value of IM???

    // setup CPU state to execute one initial NOP
    cpu->op.pip = 5;
    return Z80_M1|Z80_MREQ|Z80_RD;
}

bool z80_opdone(z80_t* cpu) {
    // because of the overlapped cycle, the result of the previous
    // instruction is only available in M1/T2
    return (0 == cpu->op.step) && (cpu->hlx_idx == 0);
}

static inline void z80_skip(z80_t* cpu, int steps, int tcycles, int delay) {
    cpu->op.step += steps;
    cpu->op.pip = (cpu->op.pip >> tcycles) << delay;
}

static inline uint64_t z80_halt(z80_t* cpu, uint64_t pins) {
    cpu->pc--;
    return pins | Z80_HALT;
}

// sign+zero+parity lookup table
static uint8_t z80_szp_flags[256] = {
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

static inline uint8_t z80_sz_flags(uint8_t val) {
    return (val != 0) ? (val & Z80_SF) : Z80_ZF;
}

static inline uint8_t z80_szyxch_flags(uint8_t acc, uint8_t val, uint32_t res) {
    return z80_sz_flags(res) |
        (res & (Z80_YF|Z80_XF)) |
        ((res >> 8) & Z80_CF) |
        ((acc ^ val ^ res) & Z80_HF);
}

static inline uint8_t z80_add_flags(uint8_t acc, uint8_t val, uint32_t res) {
    return z80_szyxch_flags(acc, val, res) | ((((val ^ acc ^ 0x80) & (val ^ res)) >> 5) & Z80_VF);
}

static inline uint8_t z80_sub_flags(uint8_t acc, uint8_t val, uint32_t res) {
    return Z80_NF | z80_szyxch_flags(acc, val, res) | ((((val ^ acc) & (res ^ acc)) >> 5) & Z80_VF);
}

static inline uint8_t z80_cp_flags(uint8_t acc, uint8_t val, uint32_t res) {
    return Z80_NF | 
        z80_sz_flags(res) |
        (val & (Z80_YF|Z80_XF)) |
        ((res >> 8) & Z80_CF) |
        ((acc ^ val ^ res) & Z80_HF) |
        ((((val ^ acc) & (res ^ acc)) >> 5) & Z80_VF);    
}

static inline void z80_add8(z80_t* cpu, uint8_t val) {
    uint32_t res = cpu->a + val;
    cpu->f = z80_add_flags(cpu->a, val, res);
    cpu->a = (uint8_t)res;
}

static inline void z80_adc8(z80_t* cpu, uint8_t val) {
    uint32_t res = cpu->a + val + (cpu->f & Z80_CF);
    cpu->f = z80_add_flags(cpu->a, val, res);
    cpu->a = (uint8_t)res;
}

static inline void z80_sub8(z80_t* cpu, uint8_t val) {
    uint32_t res = (uint32_t) ((int)cpu->a - (int)val);
    cpu->f = z80_sub_flags(cpu->a, val, res);
    cpu->a = (uint8_t)res;
}

static inline void z80_sbc8(z80_t* cpu, uint8_t val) {
    uint32_t res = (uint32_t) ((int)cpu->a - (int)val - (cpu->f & Z80_CF));
    cpu->f = z80_sub_flags(cpu->a, val, res);
    cpu->a = (uint8_t)res;
}

static inline void z80_and8(z80_t* cpu, uint8_t val) {
    cpu->a &= val;
    cpu->f = z80_szp_flags[cpu->a] | Z80_HF;
}

static inline void z80_xor8(z80_t* cpu, uint8_t val) {
    cpu->a ^= val;
    cpu->f = z80_szp_flags[cpu->a];
}

static inline void z80_or8(z80_t* cpu, uint8_t val) {
    cpu->a |= val;
    cpu->f = z80_szp_flags[cpu->a];
}

static inline void z80_cp8(z80_t* cpu, uint8_t val) {
    uint32_t res = (uint32_t) ((int)cpu->a - (int)val);
    cpu->f = z80_cp_flags(cpu->a, val, res);
}

static inline uint8_t z80_inc8(z80_t* cpu, uint8_t val) {
    uint8_t res = val + 1;
    uint8_t f = z80_sz_flags(res) | (res & (Z80_XF|Z80_YF)) | ((res ^ val) & Z80_HF);
    if (res == 0x80) {
        f |= Z80_VF;
    }
    cpu->f = f | (cpu->f & Z80_CF);
    return res;
}

static inline uint8_t z80_dec8(z80_t* cpu, uint8_t val) {
    uint8_t res = val - 1;
    uint8_t f = Z80_NF | z80_sz_flags(res) | (res & (Z80_XF|Z80_YF)) | ((res ^ val) & Z80_HF);
    if (res == 0x7F) {
        f |= Z80_VF;
    }
    cpu->f = f | (cpu->f & Z80_CF);
    return res;
}

static inline void z80_ex_de_hl(z80_t* cpu) {
    uint16_t tmp = cpu->hl;
    cpu->hl = cpu->de;
    cpu->de = tmp;
}

static inline void z80_ex_af_af2(z80_t* cpu) {
    uint16_t tmp = cpu->af2;
    cpu->af2 = cpu->af;
    cpu->af = tmp;
}

static inline void z80_exx(z80_t* cpu) {
    uint16_t tmp;
    tmp = cpu->bc; cpu->bc = cpu->bc2; cpu->bc2 = tmp;
    tmp = cpu->de; cpu->de = cpu->de2; cpu->de2 = tmp;
    tmp = cpu->hl; cpu->hl = cpu->hl2; cpu->hl2 = tmp;
}

static inline void z80_rlca(z80_t* cpu) {
    uint8_t res = (cpu->a << 1) | (cpu->a >> 7);
    cpu->f = ((cpu->a >> 7) & Z80_CF) | (cpu->f & (Z80_SF|Z80_ZF|Z80_PF)) | (res & (Z80_YF|Z80_XF));
    cpu->a = res;
}

static inline void z80_rrca(z80_t* cpu) {
    uint8_t res = (cpu->a >> 1) | (cpu->a << 7);
    cpu->f = (cpu->a & Z80_CF) | (cpu->f & (Z80_SF|Z80_ZF|Z80_PF)) | (res & (Z80_YF|Z80_XF));
    cpu->a = res;
}

static inline void z80_rla(z80_t* cpu) {
    uint8_t res = (cpu->a << 1) | (cpu->f & Z80_CF);
    cpu->f = ((cpu->a >> 7) & Z80_CF) | (cpu->f & (Z80_SF|Z80_ZF|Z80_PF)) | (res & (Z80_YF|Z80_XF));
    cpu->a = res;
}

static inline void z80_rra(z80_t* cpu) {
    uint8_t res = (cpu->a >> 1) | ((cpu->f & Z80_CF) << 7);
    cpu->f = (cpu->a & Z80_CF) | (cpu->f & (Z80_SF|Z80_ZF|Z80_PF)) | (res & (Z80_YF|Z80_XF));
    cpu->a = res;
}

static inline void z80_daa(z80_t* cpu) {
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
    cpu->f |= z80_szp_flags[res];
    cpu->a = res;
}

static inline void z80_cpl(z80_t* cpu) {
    cpu->a ^= 0xFF;
    cpu->f= (cpu->f & (Z80_SF|Z80_ZF|Z80_PF|Z80_CF)) |Z80_HF|Z80_NF| (cpu->a & (Z80_YF|Z80_XF));
}

static inline void z80_scf(z80_t* cpu) {
    cpu->f = (cpu->f & (Z80_SF|Z80_ZF|Z80_PF|Z80_CF)) | Z80_CF | (cpu->a & (Z80_YF|Z80_XF));
}

static inline void z80_ccf(z80_t* cpu) {
    cpu->f = ((cpu->f & (Z80_SF|Z80_ZF|Z80_PF|Z80_CF)) | ((cpu->f & Z80_CF)<<4) | (cpu->a & (Z80_YF|Z80_XF))) ^ Z80_CF;
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

// initiate a fetch machine cycle
static inline uint64_t z80_fetch(z80_t* cpu, uint64_t pins, uint8_t hlx_idx) {
    // reset the decoder to continue at step 0
    cpu->op.pip = 5<<1;
    cpu->op.step = 0xFFFF;
    cpu->hlx_idx = hlx_idx;
    pins = z80_set_ab_x(pins, cpu->pc++, Z80_M1|Z80_MREQ|Z80_RD);
    return pins;
}

// initiate refresh cycle
static inline uint64_t z80_refresh(z80_t* cpu, uint64_t pins) {
    pins = z80_set_ab_x(pins, cpu->r, Z80_MREQ|Z80_RFSH);
    cpu->r = (cpu->r & 0x80) | ((cpu->r + 1) & 0x7F);
    return pins;
}

static const z80_opstate_t z80_opstate_table[256] = {
    // 0x00: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // 0x01: ld bc,nn (M:3 T:10 steps:5)
    { 0x000000B6, 0x0005, 0 },
    // 0x02: ld (bc),a (M:2 T:7 steps:3)
    { 0x0000001C, 0x000A, 0 },
    // 0x03: inc bc (M:2 T:6 steps:2)
    { 0x0000000A, 0x000D, 0 },
    // 0x04: inc b (M:1 T:4 steps:1)
    { 0x00000002, 0x000F, 0 },
    // 0x05: dec b (M:1 T:4 steps:1)
    { 0x00000002, 0x0010, 0 },
    // 0x06: ld b,n (M:2 T:7 steps:3)
    { 0x00000016, 0x0011, Z80_OPSTATE_FLAGS_IMM8 },
    // 0x07: rlca (M:1 T:4 steps:1)
    { 0x00000002, 0x0014, 0 },
    // 0x08: ex af,af' (M:1 T:4 steps:1)
    { 0x00000002, 0x0015, 0 },
    // 0x09: add hl,bc (M:1 T:4 steps:1)
    { 0x00000002, 0x0016, 0 },
    // 0x0A: ld a,(bc) (M:2 T:7 steps:3)
    { 0x00000016, 0x0017, 0 },
    // 0x0B: dec bc (M:2 T:6 steps:2)
    { 0x0000000A, 0x001A, 0 },
    // 0x0C: inc c (M:1 T:4 steps:1)
    { 0x00000002, 0x001C, 0 },
    // 0x0D: dec c (M:1 T:4 steps:1)
    { 0x00000002, 0x001D, 0 },
    // 0x0E: ld c,n (M:2 T:7 steps:3)
    { 0x00000016, 0x001E, Z80_OPSTATE_FLAGS_IMM8 },
    // 0x0F: rrca (M:1 T:4 steps:1)
    { 0x00000002, 0x0021, 0 },
    // 0x10: djnz d (M:3 T:13 steps:4)
    { 0x0000042C, 0x0022, 0 },
    // 0x11: ld de,nn (M:3 T:10 steps:5)
    { 0x000000B6, 0x0026, 0 },
    // 0x12: ld (de),a (M:2 T:7 steps:3)
    { 0x0000001C, 0x002B, 0 },
    // 0x13: inc de (M:2 T:6 steps:2)
    { 0x0000000A, 0x002E, 0 },
    // 0x14: inc d (M:1 T:4 steps:1)
    { 0x00000002, 0x0030, 0 },
    // 0x15: dec d (M:1 T:4 steps:1)
    { 0x00000002, 0x0031, 0 },
    // 0x16: ld d,n (M:2 T:7 steps:3)
    { 0x00000016, 0x0032, Z80_OPSTATE_FLAGS_IMM8 },
    // 0x17: rla (M:1 T:4 steps:1)
    { 0x00000002, 0x0035, 0 },
    // 0x18: jr d (M:3 T:12 steps:4)
    { 0x00000216, 0x0036, 0 },
    // 0x19: add hl,de (M:1 T:4 steps:1)
    { 0x00000002, 0x003A, 0 },
    // 0x1A: ld a,(de) (M:2 T:7 steps:3)
    { 0x00000016, 0x003B, 0 },
    // 0x1B: dec de (M:2 T:6 steps:2)
    { 0x0000000A, 0x003E, 0 },
    // 0x1C: inc e (M:1 T:4 steps:1)
    { 0x00000002, 0x0040, 0 },
    // 0x1D: dec e (M:1 T:4 steps:1)
    { 0x00000002, 0x0041, 0 },
    // 0x1E: ld e,n (M:2 T:7 steps:3)
    { 0x00000016, 0x0042, Z80_OPSTATE_FLAGS_IMM8 },
    // 0x1F: rra (M:1 T:4 steps:1)
    { 0x00000002, 0x0045, 0 },
    // 0x20: jr nz,d (M:3 T:12 steps:4)
    { 0x00000216, 0x0046, 0 },
    // 0x21: ld hl,nn (M:3 T:10 steps:5)
    { 0x000000B6, 0x004A, 0 },
    // 0x22: ld (nn),hl (M:1 T:4 steps:1)
    { 0x00000002, 0x004F, 0 },
    // 0x23: inc hl (M:2 T:6 steps:2)
    { 0x0000000A, 0x0050, 0 },
    // 0x24: inc h (M:1 T:4 steps:1)
    { 0x00000002, 0x0052, 0 },
    // 0x25: dec h (M:1 T:4 steps:1)
    { 0x00000002, 0x0053, 0 },
    // 0x26: ld h,n (M:2 T:7 steps:3)
    { 0x00000016, 0x0054, Z80_OPSTATE_FLAGS_IMM8 },
    // 0x27: daa (M:1 T:4 steps:1)
    { 0x00000002, 0x0057, 0 },
    // 0x28: jr z,d (M:3 T:12 steps:4)
    { 0x00000216, 0x0058, 0 },
    // 0x29: add hl,hl (M:1 T:4 steps:1)
    { 0x00000002, 0x005C, 0 },
    // 0x2A: ld hl,(nn) (M:1 T:4 steps:1)
    { 0x00000002, 0x005D, 0 },
    // 0x2B: dec hl (M:2 T:6 steps:2)
    { 0x0000000A, 0x005E, 0 },
    // 0x2C: inc l (M:1 T:4 steps:1)
    { 0x00000002, 0x0060, 0 },
    // 0x2D: dec l (M:1 T:4 steps:1)
    { 0x00000002, 0x0061, 0 },
    // 0x2E: ld l,n (M:2 T:7 steps:3)
    { 0x00000016, 0x0062, Z80_OPSTATE_FLAGS_IMM8 },
    // 0x2F: cpl (M:1 T:4 steps:1)
    { 0x00000002, 0x0065, 0 },
    // 0x30: jr nc,d (M:3 T:12 steps:4)
    { 0x00000216, 0x0066, 0 },
    // 0x31: ld sp,nn (M:3 T:10 steps:5)
    { 0x000000B6, 0x006A, 0 },
    // 0x32: ld (nn),a (M:4 T:13 steps:7)
    { 0x00000736, 0x006F, 0 },
    // 0x33: inc sp (M:2 T:6 steps:2)
    { 0x0000000A, 0x0076, 0 },
    // 0x34: inc (hl) (M:3 T:11 steps:5)
    { 0x000001C6, 0x0078, Z80_OPSTATE_FLAGS_INDIRECT },
    // 0x35: dec (hl) (M:3 T:11 steps:5)
    { 0x000001C6, 0x007D, Z80_OPSTATE_FLAGS_INDIRECT },
    // 0x36: ld (hl),n (M:3 T:10 steps:5)
    { 0x000000E6, 0x0082, Z80_OPSTATE_FLAGS_INDIRECT|Z80_OPSTATE_FLAGS_IMM8 },
    // 0x37: scf (M:1 T:4 steps:1)
    { 0x00000002, 0x0087, 0 },
    // 0x38: jr c,d (M:3 T:12 steps:4)
    { 0x00000216, 0x0088, 0 },
    // 0x39: add hl,sp (M:1 T:4 steps:1)
    { 0x00000002, 0x008C, 0 },
    // 0x3A: ld a,(nn) (M:4 T:13 steps:7)
    { 0x000005B6, 0x008D, 0 },
    // 0x3B: dec sp (M:2 T:6 steps:2)
    { 0x0000000A, 0x0094, 0 },
    // 0x3C: inc a (M:1 T:4 steps:1)
    { 0x00000002, 0x0096, 0 },
    // 0x3D: dec a (M:1 T:4 steps:1)
    { 0x00000002, 0x0097, 0 },
    // 0x3E: ld a,n (M:2 T:7 steps:3)
    { 0x00000016, 0x0098, Z80_OPSTATE_FLAGS_IMM8 },
    // 0x3F: ccf (M:1 T:4 steps:1)
    { 0x00000002, 0x009B, 0 },
    // 0x40: ld b,b (M:1 T:4 steps:1)
    { 0x00000002, 0x009C, 0 },
    // 0x41: ld b,c (M:1 T:4 steps:1)
    { 0x00000002, 0x009D, 0 },
    // 0x42: ld b,d (M:1 T:4 steps:1)
    { 0x00000002, 0x009E, 0 },
    // 0x43: ld b,e (M:1 T:4 steps:1)
    { 0x00000002, 0x009F, 0 },
    // 0x44: ld b,h (M:1 T:4 steps:1)
    { 0x00000002, 0x00A0, 0 },
    // 0x45: ld b,l (M:1 T:4 steps:1)
    { 0x00000002, 0x00A1, 0 },
    // 0x46: ld b,(hl) (M:2 T:7 steps:3)
    { 0x00000016, 0x00A2, Z80_OPSTATE_FLAGS_INDIRECT },
    // 0x47: ld b,a (M:1 T:4 steps:1)
    { 0x00000002, 0x00A5, 0 },
    // 0x48: ld c,b (M:1 T:4 steps:1)
    { 0x00000002, 0x00A6, 0 },
    // 0x49: ld c,c (M:1 T:4 steps:1)
    { 0x00000002, 0x00A7, 0 },
    // 0x4A: ld c,d (M:1 T:4 steps:1)
    { 0x00000002, 0x00A8, 0 },
    // 0x4B: ld c,e (M:1 T:4 steps:1)
    { 0x00000002, 0x00A9, 0 },
    // 0x4C: ld c,h (M:1 T:4 steps:1)
    { 0x00000002, 0x00AA, 0 },
    // 0x4D: ld c,l (M:1 T:4 steps:1)
    { 0x00000002, 0x00AB, 0 },
    // 0x4E: ld c,(hl) (M:2 T:7 steps:3)
    { 0x00000016, 0x00AC, Z80_OPSTATE_FLAGS_INDIRECT },
    // 0x4F: ld c,a (M:1 T:4 steps:1)
    { 0x00000002, 0x00AF, 0 },
    // 0x50: ld d,b (M:1 T:4 steps:1)
    { 0x00000002, 0x00B0, 0 },
    // 0x51: ld d,c (M:1 T:4 steps:1)
    { 0x00000002, 0x00B1, 0 },
    // 0x52: ld d,d (M:1 T:4 steps:1)
    { 0x00000002, 0x00B2, 0 },
    // 0x53: ld d,e (M:1 T:4 steps:1)
    { 0x00000002, 0x00B3, 0 },
    // 0x54: ld d,h (M:1 T:4 steps:1)
    { 0x00000002, 0x00B4, 0 },
    // 0x55: ld d,l (M:1 T:4 steps:1)
    { 0x00000002, 0x00B5, 0 },
    // 0x56: ld d,(hl) (M:2 T:7 steps:3)
    { 0x00000016, 0x00B6, Z80_OPSTATE_FLAGS_INDIRECT },
    // 0x57: ld d,a (M:1 T:4 steps:1)
    { 0x00000002, 0x00B9, 0 },
    // 0x58: ld e,b (M:1 T:4 steps:1)
    { 0x00000002, 0x00BA, 0 },
    // 0x59: ld e,c (M:1 T:4 steps:1)
    { 0x00000002, 0x00BB, 0 },
    // 0x5A: ld e,d (M:1 T:4 steps:1)
    { 0x00000002, 0x00BC, 0 },
    // 0x5B: ld e,e (M:1 T:4 steps:1)
    { 0x00000002, 0x00BD, 0 },
    // 0x5C: ld e,h (M:1 T:4 steps:1)
    { 0x00000002, 0x00BE, 0 },
    // 0x5D: ld e,l (M:1 T:4 steps:1)
    { 0x00000002, 0x00BF, 0 },
    // 0x5E: ld e,(hl) (M:2 T:7 steps:3)
    { 0x00000016, 0x00C0, Z80_OPSTATE_FLAGS_INDIRECT },
    // 0x5F: ld e,a (M:1 T:4 steps:1)
    { 0x00000002, 0x00C3, 0 },
    // 0x60: ld h,b (M:1 T:4 steps:1)
    { 0x00000002, 0x00C4, 0 },
    // 0x61: ld h,c (M:1 T:4 steps:1)
    { 0x00000002, 0x00C5, 0 },
    // 0x62: ld h,d (M:1 T:4 steps:1)
    { 0x00000002, 0x00C6, 0 },
    // 0x63: ld h,e (M:1 T:4 steps:1)
    { 0x00000002, 0x00C7, 0 },
    // 0x64: ld h,h (M:1 T:4 steps:1)
    { 0x00000002, 0x00C8, 0 },
    // 0x65: ld h,l (M:1 T:4 steps:1)
    { 0x00000002, 0x00C9, 0 },
    // 0x66: ld h,(hl) (M:2 T:7 steps:3)
    { 0x00000016, 0x00CA, Z80_OPSTATE_FLAGS_INDIRECT },
    // 0x67: ld h,a (M:1 T:4 steps:1)
    { 0x00000002, 0x00CD, 0 },
    // 0x68: ld l,b (M:1 T:4 steps:1)
    { 0x00000002, 0x00CE, 0 },
    // 0x69: ld l,c (M:1 T:4 steps:1)
    { 0x00000002, 0x00CF, 0 },
    // 0x6A: ld l,d (M:1 T:4 steps:1)
    { 0x00000002, 0x00D0, 0 },
    // 0x6B: ld l,e (M:1 T:4 steps:1)
    { 0x00000002, 0x00D1, 0 },
    // 0x6C: ld l,h (M:1 T:4 steps:1)
    { 0x00000002, 0x00D2, 0 },
    // 0x6D: ld l,l (M:1 T:4 steps:1)
    { 0x00000002, 0x00D3, 0 },
    // 0x6E: ld l,(hl) (M:2 T:7 steps:3)
    { 0x00000016, 0x00D4, Z80_OPSTATE_FLAGS_INDIRECT },
    // 0x6F: ld l,a (M:1 T:4 steps:1)
    { 0x00000002, 0x00D7, 0 },
    // 0x70: ld (hl),b (M:2 T:7 steps:3)
    { 0x0000001C, 0x00D8, Z80_OPSTATE_FLAGS_INDIRECT },
    // 0x71: ld (hl),c (M:2 T:7 steps:3)
    { 0x0000001C, 0x00DB, Z80_OPSTATE_FLAGS_INDIRECT },
    // 0x72: ld (hl),d (M:2 T:7 steps:3)
    { 0x0000001C, 0x00DE, Z80_OPSTATE_FLAGS_INDIRECT },
    // 0x73: ld (hl),e (M:2 T:7 steps:3)
    { 0x0000001C, 0x00E1, Z80_OPSTATE_FLAGS_INDIRECT },
    // 0x74: ld (hl),h (M:2 T:7 steps:3)
    { 0x0000001C, 0x00E4, Z80_OPSTATE_FLAGS_INDIRECT },
    // 0x75: ld (hl),l (M:2 T:7 steps:3)
    { 0x0000001C, 0x00E7, Z80_OPSTATE_FLAGS_INDIRECT },
    // 0x76: halt (M:1 T:4 steps:1)
    { 0x00000002, 0x00EA, 0 },
    // 0x77: ld (hl),a (M:2 T:7 steps:3)
    { 0x0000001C, 0x00EB, Z80_OPSTATE_FLAGS_INDIRECT },
    // 0x78: ld a,b (M:1 T:4 steps:1)
    { 0x00000002, 0x00EE, 0 },
    // 0x79: ld a,c (M:1 T:4 steps:1)
    { 0x00000002, 0x00EF, 0 },
    // 0x7A: ld a,d (M:1 T:4 steps:1)
    { 0x00000002, 0x00F0, 0 },
    // 0x7B: ld a,e (M:1 T:4 steps:1)
    { 0x00000002, 0x00F1, 0 },
    // 0x7C: ld a,h (M:1 T:4 steps:1)
    { 0x00000002, 0x00F2, 0 },
    // 0x7D: ld a,l (M:1 T:4 steps:1)
    { 0x00000002, 0x00F3, 0 },
    // 0x7E: ld a,(hl) (M:2 T:7 steps:3)
    { 0x00000016, 0x00F4, Z80_OPSTATE_FLAGS_INDIRECT },
    // 0x7F: ld a,a (M:1 T:4 steps:1)
    { 0x00000002, 0x00F7, 0 },
    // 0x80: add b (M:1 T:4 steps:1)
    { 0x00000002, 0x00F8, 0 },
    // 0x81: add c (M:1 T:4 steps:1)
    { 0x00000002, 0x00F9, 0 },
    // 0x82: add d (M:1 T:4 steps:1)
    { 0x00000002, 0x00FA, 0 },
    // 0x83: add e (M:1 T:4 steps:1)
    { 0x00000002, 0x00FB, 0 },
    // 0x84: add h (M:1 T:4 steps:1)
    { 0x00000002, 0x00FC, 0 },
    // 0x85: add l (M:1 T:4 steps:1)
    { 0x00000002, 0x00FD, 0 },
    // 0x86: add (hl) (M:2 T:7 steps:3)
    { 0x00000016, 0x00FE, Z80_OPSTATE_FLAGS_INDIRECT },
    // 0x87: add a (M:1 T:4 steps:1)
    { 0x00000002, 0x0101, 0 },
    // 0x88: adc b (M:1 T:4 steps:1)
    { 0x00000002, 0x0102, 0 },
    // 0x89: adc c (M:1 T:4 steps:1)
    { 0x00000002, 0x0103, 0 },
    // 0x8A: adc d (M:1 T:4 steps:1)
    { 0x00000002, 0x0104, 0 },
    // 0x8B: adc e (M:1 T:4 steps:1)
    { 0x00000002, 0x0105, 0 },
    // 0x8C: adc h (M:1 T:4 steps:1)
    { 0x00000002, 0x0106, 0 },
    // 0x8D: adc l (M:1 T:4 steps:1)
    { 0x00000002, 0x0107, 0 },
    // 0x8E: adc (hl) (M:2 T:7 steps:3)
    { 0x00000016, 0x0108, Z80_OPSTATE_FLAGS_INDIRECT },
    // 0x8F: adc a (M:1 T:4 steps:1)
    { 0x00000002, 0x010B, 0 },
    // 0x90: sub b (M:1 T:4 steps:1)
    { 0x00000002, 0x010C, 0 },
    // 0x91: sub c (M:1 T:4 steps:1)
    { 0x00000002, 0x010D, 0 },
    // 0x92: sub d (M:1 T:4 steps:1)
    { 0x00000002, 0x010E, 0 },
    // 0x93: sub e (M:1 T:4 steps:1)
    { 0x00000002, 0x010F, 0 },
    // 0x94: sub h (M:1 T:4 steps:1)
    { 0x00000002, 0x0110, 0 },
    // 0x95: sub l (M:1 T:4 steps:1)
    { 0x00000002, 0x0111, 0 },
    // 0x96: sub (hl) (M:2 T:7 steps:3)
    { 0x00000016, 0x0112, Z80_OPSTATE_FLAGS_INDIRECT },
    // 0x97: sub a (M:1 T:4 steps:1)
    { 0x00000002, 0x0115, 0 },
    // 0x98: sbc b (M:1 T:4 steps:1)
    { 0x00000002, 0x0116, 0 },
    // 0x99: sbc c (M:1 T:4 steps:1)
    { 0x00000002, 0x0117, 0 },
    // 0x9A: sbc d (M:1 T:4 steps:1)
    { 0x00000002, 0x0118, 0 },
    // 0x9B: sbc e (M:1 T:4 steps:1)
    { 0x00000002, 0x0119, 0 },
    // 0x9C: sbc h (M:1 T:4 steps:1)
    { 0x00000002, 0x011A, 0 },
    // 0x9D: sbc l (M:1 T:4 steps:1)
    { 0x00000002, 0x011B, 0 },
    // 0x9E: sbc (hl) (M:2 T:7 steps:3)
    { 0x00000016, 0x011C, Z80_OPSTATE_FLAGS_INDIRECT },
    // 0x9F: sbc a (M:1 T:4 steps:1)
    { 0x00000002, 0x011F, 0 },
    // 0xA0: and b (M:1 T:4 steps:1)
    { 0x00000002, 0x0120, 0 },
    // 0xA1: and c (M:1 T:4 steps:1)
    { 0x00000002, 0x0121, 0 },
    // 0xA2: and d (M:1 T:4 steps:1)
    { 0x00000002, 0x0122, 0 },
    // 0xA3: and e (M:1 T:4 steps:1)
    { 0x00000002, 0x0123, 0 },
    // 0xA4: and h (M:1 T:4 steps:1)
    { 0x00000002, 0x0124, 0 },
    // 0xA5: and l (M:1 T:4 steps:1)
    { 0x00000002, 0x0125, 0 },
    // 0xA6: and (hl) (M:2 T:7 steps:3)
    { 0x00000016, 0x0126, Z80_OPSTATE_FLAGS_INDIRECT },
    // 0xA7: and a (M:1 T:4 steps:1)
    { 0x00000002, 0x0129, 0 },
    // 0xA8: xor b (M:1 T:4 steps:1)
    { 0x00000002, 0x012A, 0 },
    // 0xA9: xor c (M:1 T:4 steps:1)
    { 0x00000002, 0x012B, 0 },
    // 0xAA: xor d (M:1 T:4 steps:1)
    { 0x00000002, 0x012C, 0 },
    // 0xAB: xor e (M:1 T:4 steps:1)
    { 0x00000002, 0x012D, 0 },
    // 0xAC: xor h (M:1 T:4 steps:1)
    { 0x00000002, 0x012E, 0 },
    // 0xAD: xor l (M:1 T:4 steps:1)
    { 0x00000002, 0x012F, 0 },
    // 0xAE: xor (hl) (M:2 T:7 steps:3)
    { 0x00000016, 0x0130, Z80_OPSTATE_FLAGS_INDIRECT },
    // 0xAF: xor a (M:1 T:4 steps:1)
    { 0x00000002, 0x0133, 0 },
    // 0xB0: or b (M:1 T:4 steps:1)
    { 0x00000002, 0x0134, 0 },
    // 0xB1: or c (M:1 T:4 steps:1)
    { 0x00000002, 0x0135, 0 },
    // 0xB2: or d (M:1 T:4 steps:1)
    { 0x00000002, 0x0136, 0 },
    // 0xB3: or e (M:1 T:4 steps:1)
    { 0x00000002, 0x0137, 0 },
    // 0xB4: or h (M:1 T:4 steps:1)
    { 0x00000002, 0x0138, 0 },
    // 0xB5: or l (M:1 T:4 steps:1)
    { 0x00000002, 0x0139, 0 },
    // 0xB6: or (hl) (M:2 T:7 steps:3)
    { 0x00000016, 0x013A, Z80_OPSTATE_FLAGS_INDIRECT },
    // 0xB7: or a (M:1 T:4 steps:1)
    { 0x00000002, 0x013D, 0 },
    // 0xB8: cp b (M:1 T:4 steps:1)
    { 0x00000002, 0x013E, 0 },
    // 0xB9: cp c (M:1 T:4 steps:1)
    { 0x00000002, 0x013F, 0 },
    // 0xBA: cp d (M:1 T:4 steps:1)
    { 0x00000002, 0x0140, 0 },
    // 0xBB: cp e (M:1 T:4 steps:1)
    { 0x00000002, 0x0141, 0 },
    // 0xBC: cp h (M:1 T:4 steps:1)
    { 0x00000002, 0x0142, 0 },
    // 0xBD: cp l (M:1 T:4 steps:1)
    { 0x00000002, 0x0143, 0 },
    // 0xBE: cp (hl) (M:2 T:7 steps:3)
    { 0x00000016, 0x0144, Z80_OPSTATE_FLAGS_INDIRECT },
    // 0xBF: cp a (M:1 T:4 steps:1)
    { 0x00000002, 0x0147, 0 },
    // 0xC0: ret nz (M:4 T:11 steps:6)
    { 0x0000016E, 0x0148, 0 },
    // 0xC1: pop bc (M:3 T:10 steps:5)
    { 0x000000B6, 0x014E, 0 },
    // 0xC2: jp nz,nn (M:3 T:10 steps:5)
    { 0x000000B6, 0x0153, 0 },
    // 0xC3: jp nn (M:3 T:10 steps:5)
    { 0x000000B6, 0x0158, 0 },
    // 0xC4: call nz,nn (M:6 T:17 steps:10)
    { 0x000076B6, 0x015D, 0 },
    // 0xC5: push bc (M:3 T:11 steps:5)
    { 0x000001D8, 0x0167, 0 },
    // 0xC6: add n (M:2 T:7 steps:3)
    { 0x00000016, 0x016C, Z80_OPSTATE_FLAGS_IMM8 },
    // 0xC7: rst 0h (M:1 T:4 steps:1)
    { 0x00000002, 0x016F, 0 },
    // 0xC8: ret z (M:4 T:11 steps:6)
    { 0x0000016E, 0x0170, 0 },
    // 0xC9: ret (M:3 T:10 steps:5)
    { 0x000000B6, 0x0176, 0 },
    // 0xCA: jp z,nn (M:3 T:10 steps:5)
    { 0x000000B6, 0x017B, 0 },
    // 0xCB: cb prefix (M:1 T:4 steps:1)
    { 0x00000002, 0x0180, 0 },
    // 0xCC: call z,nn (M:6 T:17 steps:10)
    { 0x000076B6, 0x0181, 0 },
    // 0xCD: call nn (M:5 T:17 steps:9)
    { 0x00007636, 0x018B, 0 },
    // 0xCE: adc n (M:2 T:7 steps:3)
    { 0x00000016, 0x0194, Z80_OPSTATE_FLAGS_IMM8 },
    // 0xCF: rst 8h (M:1 T:4 steps:1)
    { 0x00000002, 0x0197, 0 },
    // 0xD0: ret nc (M:4 T:11 steps:6)
    { 0x0000016E, 0x0198, 0 },
    // 0xD1: pop de (M:3 T:10 steps:5)
    { 0x000000B6, 0x019E, 0 },
    // 0xD2: jp nc,nn (M:3 T:10 steps:5)
    { 0x000000B6, 0x01A3, 0 },
    // 0xD3: out (n),a (M:1 T:4 steps:1)
    { 0x00000002, 0x01A8, 0 },
    // 0xD4: call nc,nn (M:6 T:17 steps:10)
    { 0x000076B6, 0x01A9, 0 },
    // 0xD5: push de (M:3 T:11 steps:5)
    { 0x000001D8, 0x01B3, 0 },
    // 0xD6: sub n (M:2 T:7 steps:3)
    { 0x00000016, 0x01B8, Z80_OPSTATE_FLAGS_IMM8 },
    // 0xD7: rst 10h (M:1 T:4 steps:1)
    { 0x00000002, 0x01BB, 0 },
    // 0xD8: ret c (M:4 T:11 steps:6)
    { 0x0000016E, 0x01BC, 0 },
    // 0xD9: exx (M:1 T:4 steps:1)
    { 0x00000002, 0x01C2, 0 },
    // 0xDA: jp c,nn (M:3 T:10 steps:5)
    { 0x000000B6, 0x01C3, 0 },
    // 0xDB: in a,(n) (M:1 T:4 steps:1)
    { 0x00000002, 0x01C8, 0 },
    // 0xDC: call c,nn (M:6 T:17 steps:10)
    { 0x000076B6, 0x01C9, 0 },
    // 0xDD: dd prefix (M:1 T:4 steps:1)
    { 0x00000002, 0x01D3, 0 },
    // 0xDE: sbc n (M:2 T:7 steps:3)
    { 0x00000016, 0x01D4, Z80_OPSTATE_FLAGS_IMM8 },
    // 0xDF: rst 18h (M:1 T:4 steps:1)
    { 0x00000002, 0x01D7, 0 },
    // 0xE0: ret po (M:4 T:11 steps:6)
    { 0x0000016E, 0x01D8, 0 },
    // 0xE1: pop hl (M:3 T:10 steps:5)
    { 0x000000B6, 0x01DE, 0 },
    // 0xE2: jp po,nn (M:3 T:10 steps:5)
    { 0x000000B6, 0x01E3, 0 },
    // 0xE3: ex (sp),hl (M:5 T:19 steps:9)
    { 0x00013636, 0x01E8, 0 },
    // 0xE4: call po,nn (M:6 T:17 steps:10)
    { 0x000076B6, 0x01F1, 0 },
    // 0xE5: push hl (M:3 T:11 steps:5)
    { 0x000001D8, 0x01FB, 0 },
    // 0xE6: and n (M:2 T:7 steps:3)
    { 0x00000016, 0x0200, Z80_OPSTATE_FLAGS_IMM8 },
    // 0xE7: rst 20h (M:1 T:4 steps:1)
    { 0x00000002, 0x0203, 0 },
    // 0xE8: ret pe (M:4 T:11 steps:6)
    { 0x0000016E, 0x0204, 0 },
    // 0xE9: jp hl (M:1 T:4 steps:1)
    { 0x00000002, 0x020A, 0 },
    // 0xEA: jp pe,nn (M:3 T:10 steps:5)
    { 0x000000B6, 0x020B, 0 },
    // 0xEB: ex de,hl (M:1 T:4 steps:1)
    { 0x00000002, 0x0210, 0 },
    // 0xEC: call pe,nn (M:6 T:17 steps:10)
    { 0x000076B6, 0x0211, 0 },
    // 0xED: ed prefix (M:1 T:4 steps:1)
    { 0x00000002, 0x021B, 0 },
    // 0xEE: xor n (M:2 T:7 steps:3)
    { 0x00000016, 0x021C, Z80_OPSTATE_FLAGS_IMM8 },
    // 0xEF: rst 28h (M:1 T:4 steps:1)
    { 0x00000002, 0x021F, 0 },
    // 0xF0: ret p (M:4 T:11 steps:6)
    { 0x0000016E, 0x0220, 0 },
    // 0xF1: pop af (M:3 T:10 steps:5)
    { 0x000000B6, 0x0226, 0 },
    // 0xF2: jp p,nn (M:3 T:10 steps:5)
    { 0x000000B6, 0x022B, 0 },
    // 0xF3: di (M:1 T:4 steps:1)
    { 0x00000002, 0x0230, 0 },
    // 0xF4: call p,nn (M:6 T:17 steps:10)
    { 0x000076B6, 0x0231, 0 },
    // 0xF5: push af (M:3 T:11 steps:5)
    { 0x000001D8, 0x023B, 0 },
    // 0xF6: or n (M:2 T:7 steps:3)
    { 0x00000016, 0x0240, Z80_OPSTATE_FLAGS_IMM8 },
    // 0xF7: rst 30h (M:1 T:4 steps:1)
    { 0x00000002, 0x0243, 0 },
    // 0xF8: ret m (M:4 T:11 steps:6)
    { 0x0000016E, 0x0244, 0 },
    // 0xF9: ld sp,hl (M:2 T:6 steps:2)
    { 0x0000000A, 0x024A, 0 },
    // 0xFA: jp m,nn (M:3 T:10 steps:5)
    { 0x000000B6, 0x024C, 0 },
    // 0xFB: ei (M:1 T:4 steps:1)
    { 0x00000002, 0x0251, 0 },
    // 0xFC: call m,nn (M:6 T:17 steps:10)
    { 0x000076B6, 0x0252, 0 },
    // 0xFD: fd prefix (M:1 T:4 steps:1)
    { 0x00000002, 0x025C, 0 },
    // 0xFE: cp n (M:2 T:7 steps:3)
    { 0x00000016, 0x025D, Z80_OPSTATE_FLAGS_IMM8 },
    // 0xFF: rst 38h (M:1 T:4 steps:1)
    { 0x00000002, 0x0260, 0 },

};

uint64_t z80_prefetch(z80_t* cpu, uint16_t new_pc) {
    cpu->pc = new_pc;
    cpu->op.pip = 1;
    // overlapped M1:T1 of the NOP instruction to initiate opcode fetch at new pc
    cpu->op.step = z80_opstate_table[0].step + 1;
    return 0;
}

// pin helper macros
#define _sa(ab)             pins=z80_set_ab(pins,ab)
#define _sax(ab,x)          pins=z80_set_ab_x(pins,ab,x)
#define _sad(ab,d)          pins=z80_set_ab_db(pins,ab,d)
#define _sadx(ab,d,x)       pins=z80_set_ab_db_x(pins,ab,d,x)
#define _gd()               z80_get_db(pins)

// high level helper macros
#define _fetch()        pins=z80_fetch(cpu,pins,Z80_MAP_HL)
#define _fetch_ix()     pins=z80_fetch(cpu,pins,Z80_MAP_IX)
#define _fetch_iy()     pins=z80_fetch(cpu,pins,Z80_MAP_IY)
#define _mread(ab)      _sax(ab,Z80_MREQ|Z80_RD)
#define _mwrite(ab,d)   _sadx(ab,d,Z80_MREQ|Z80_WR)
#define _ioread(ab)     _sax(ab,Z80_IORQ|Z80_RD)
#define _iowrite(ab,d)  _sadx(ab,d,Z80_IORQ|Z80_WR)
#define _wait()         if(pins&Z80_WAIT){return pins;}
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
    pins &= ~Z80_CTRL_PIN_MASK;
    if (cpu->op.pip & 1) {
        switch (cpu->op.step) {
            // shared fetch machine cycle for all opcodes
            case 0: {
                cpu->ir = _gd();
                _wait();
            } break;
            // refresh cycle
            case 1: {
                pins = z80_refresh(cpu, pins);
                cpu->op = z80_opstate_table[cpu->ir];
                // if this is a (HL)/(IX+d)/(IY+d) instruction, insert
                // d-load cycle if needed and compute effective address
                if (cpu->op.flags & Z80_OPSTATE_FLAGS_INDIRECT) {
                    cpu->addr = cpu->hlx[cpu->hlx_idx].hl;
                    if (cpu->hlx_idx != Z80_MAP_HL) {
                        // (IX+d) or (IY+d): insert 3 4-cycle machine cycles
                        // to load d offset and setup effective address
                        cpu->op.pip = 3<<1;
                        // special case: if this is indirect+immediate (which is
                        // just LD (HL),n, then the immediate-load is 'hidden' within
                        // the 8-tcycle d-offset computation)
                        if (cpu->op.flags & Z80_OPSTATE_FLAGS_IMM8) {
                            cpu->op.pip |= 1<<3;
                        }
                        else {
                            cpu->op.pip |= 1<<8;
                        }
                        cpu->op.step = 1; // => continues at step 2
                    }
                }
            } break;
            //=== optional d-loading cycle for (HL), (IX+d), (IY+d)
            case 2: {
                _wait();
                _mread(cpu->pc++);
            } break;
            case 3: {
                cpu->addr += (int8_t)_gd();
                cpu->wz = cpu->addr;
            } break;
            case 4: {
                // continue with original instruction
                cpu->op = z80_opstate_table[cpu->ir];
                // special case: if this is indirect+immediate (which is just LD 
                // (HL),n), then stretch the immediate-load machine cycle by 3 tcycles
                // because it is 'hidden' in the d-offset 8-tcycle load
                if (cpu->op.flags & Z80_OPSTATE_FLAGS_IMM8) {
                    const uint64_t mask = 0xF;
                    cpu->op.pip = (cpu->op.pip & mask) | ((cpu->op.pip & ~mask)<<2);
                }
            } break;
            // FIXME: optional interrupt handling(?) 
            
            // 0x00: nop (M:1 T:4)
            // -- OVERLAP
            case 0x0005: _fetch(); break;
            
            // 0x01: ld bc,nn (M:3 T:10)
            // -- M2
            case 0x0006: _wait();_mread(cpu->pc++); break;
            case 0x0007: cpu->c=_gd(); break;
            // -- M3
            case 0x0008: _wait();_mread(cpu->pc++); break;
            case 0x0009: cpu->b=_gd(); break;
            // -- OVERLAP
            case 0x000A: _fetch(); break;
            
            // 0x02: ld (bc),a (M:2 T:7)
            // -- M2
            case 0x000B: _mwrite(cpu->bc,cpu->a);cpu->wzl=cpu->c+1;cpu->wzh=cpu->a; break;
            case 0x000C: _wait() break;
            // -- OVERLAP
            case 0x000D: _fetch(); break;
            
            // 0x03: inc bc (M:2 T:6)
            // -- M2 (generic)
            case 0x000E: cpu->bc++; break;
            // -- OVERLAP
            case 0x000F: _fetch(); break;
            
            // 0x04: inc b (M:1 T:4)
            // -- OVERLAP
            case 0x0010: cpu->b=z80_inc8(cpu,cpu->b);_fetch(); break;
            
            // 0x05: dec b (M:1 T:4)
            // -- OVERLAP
            case 0x0011: cpu->b=z80_dec8(cpu,cpu->b);_fetch(); break;
            
            // 0x06: ld b,n (M:2 T:7)
            // -- M2
            case 0x0012: _wait();_mread(cpu->pc++); break;
            case 0x0013: cpu->b=_gd(); break;
            // -- OVERLAP
            case 0x0014: _fetch(); break;
            
            // 0x07: rlca (M:1 T:4)
            // -- OVERLAP
            case 0x0015: z80_rlca(cpu);_fetch(); break;
            
            // 0x08: ex af,af' (M:1 T:4)
            // -- OVERLAP
            case 0x0016: z80_ex_af_af2(cpu);_fetch(); break;
            
            // 0x09: add hl,bc (M:1 T:4)
            // -- OVERLAP
            case 0x0017: _fetch(); break;
            
            // 0x0A: ld a,(bc) (M:2 T:7)
            // -- M2
            case 0x0018: _wait();_mread(cpu->bc); break;
            case 0x0019: cpu->a=_gd();cpu->wz=cpu->bc+1; break;
            // -- OVERLAP
            case 0x001A: _fetch(); break;
            
            // 0x0B: dec bc (M:2 T:6)
            // -- M2 (generic)
            case 0x001B: cpu->bc--; break;
            // -- OVERLAP
            case 0x001C: _fetch(); break;
            
            // 0x0C: inc c (M:1 T:4)
            // -- OVERLAP
            case 0x001D: cpu->c=z80_inc8(cpu,cpu->c);_fetch(); break;
            
            // 0x0D: dec c (M:1 T:4)
            // -- OVERLAP
            case 0x001E: cpu->c=z80_dec8(cpu,cpu->c);_fetch(); break;
            
            // 0x0E: ld c,n (M:2 T:7)
            // -- M2
            case 0x001F: _wait();_mread(cpu->pc++); break;
            case 0x0020: cpu->c=_gd(); break;
            // -- OVERLAP
            case 0x0021: _fetch(); break;
            
            // 0x0F: rrca (M:1 T:4)
            // -- OVERLAP
            case 0x0022: z80_rrca(cpu);_fetch(); break;
            
            // 0x10: djnz d (M:3 T:13)
            // -- M2
            case 0x0023: _wait();_mread(cpu->pc++); break;
            case 0x0024: cpu->dlatch=_gd();if(--cpu->b==0){z80_skip(cpu,1,7,2);}; break;
            // -- M3 (generic)
            case 0x0025: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;; break;
            // -- OVERLAP
            case 0x0026: _fetch(); break;
            
            // 0x11: ld de,nn (M:3 T:10)
            // -- M2
            case 0x0027: _wait();_mread(cpu->pc++); break;
            case 0x0028: cpu->e=_gd(); break;
            // -- M3
            case 0x0029: _wait();_mread(cpu->pc++); break;
            case 0x002A: cpu->d=_gd(); break;
            // -- OVERLAP
            case 0x002B: _fetch(); break;
            
            // 0x12: ld (de),a (M:2 T:7)
            // -- M2
            case 0x002C: _mwrite(cpu->de,cpu->a);cpu->wzl=cpu->e+1;cpu->wzh=cpu->a; break;
            case 0x002D: _wait() break;
            // -- OVERLAP
            case 0x002E: _fetch(); break;
            
            // 0x13: inc de (M:2 T:6)
            // -- M2 (generic)
            case 0x002F: cpu->de++; break;
            // -- OVERLAP
            case 0x0030: _fetch(); break;
            
            // 0x14: inc d (M:1 T:4)
            // -- OVERLAP
            case 0x0031: cpu->d=z80_inc8(cpu,cpu->d);_fetch(); break;
            
            // 0x15: dec d (M:1 T:4)
            // -- OVERLAP
            case 0x0032: cpu->d=z80_dec8(cpu,cpu->d);_fetch(); break;
            
            // 0x16: ld d,n (M:2 T:7)
            // -- M2
            case 0x0033: _wait();_mread(cpu->pc++); break;
            case 0x0034: cpu->d=_gd(); break;
            // -- OVERLAP
            case 0x0035: _fetch(); break;
            
            // 0x17: rla (M:1 T:4)
            // -- OVERLAP
            case 0x0036: z80_rla(cpu);_fetch(); break;
            
            // 0x18: jr d (M:3 T:12)
            // -- M2
            case 0x0037: _wait();_mread(cpu->pc++); break;
            case 0x0038: cpu->dlatch=_gd(); break;
            // -- M3 (generic)
            case 0x0039: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;; break;
            // -- OVERLAP
            case 0x003A: _fetch(); break;
            
            // 0x19: add hl,de (M:1 T:4)
            // -- OVERLAP
            case 0x003B: _fetch(); break;
            
            // 0x1A: ld a,(de) (M:2 T:7)
            // -- M2
            case 0x003C: _wait();_mread(cpu->de); break;
            case 0x003D: cpu->a=_gd();cpu->wz=cpu->de+1; break;
            // -- OVERLAP
            case 0x003E: _fetch(); break;
            
            // 0x1B: dec de (M:2 T:6)
            // -- M2 (generic)
            case 0x003F: cpu->de--; break;
            // -- OVERLAP
            case 0x0040: _fetch(); break;
            
            // 0x1C: inc e (M:1 T:4)
            // -- OVERLAP
            case 0x0041: cpu->e=z80_inc8(cpu,cpu->e);_fetch(); break;
            
            // 0x1D: dec e (M:1 T:4)
            // -- OVERLAP
            case 0x0042: cpu->e=z80_dec8(cpu,cpu->e);_fetch(); break;
            
            // 0x1E: ld e,n (M:2 T:7)
            // -- M2
            case 0x0043: _wait();_mread(cpu->pc++); break;
            case 0x0044: cpu->e=_gd(); break;
            // -- OVERLAP
            case 0x0045: _fetch(); break;
            
            // 0x1F: rra (M:1 T:4)
            // -- OVERLAP
            case 0x0046: z80_rra(cpu);_fetch(); break;
            
            // 0x20: jr nz,d (M:3 T:12)
            // -- M2
            case 0x0047: _wait();_mread(cpu->pc++); break;
            case 0x0048: cpu->dlatch=_gd();if(!(_cc_nz)){z80_skip(cpu,1,7,2);}; break;
            // -- M3 (generic)
            case 0x0049: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;; break;
            // -- OVERLAP
            case 0x004A: _fetch(); break;
            
            // 0x21: ld hl,nn (M:3 T:10)
            // -- M2
            case 0x004B: _wait();_mread(cpu->pc++); break;
            case 0x004C: cpu->hlx[cpu->hlx_idx].l=_gd(); break;
            // -- M3
            case 0x004D: _wait();_mread(cpu->pc++); break;
            case 0x004E: cpu->hlx[cpu->hlx_idx].h=_gd(); break;
            // -- OVERLAP
            case 0x004F: _fetch(); break;
            
            // 0x22: ld (nn),hl (M:1 T:4)
            // -- OVERLAP
            case 0x0050: _fetch(); break;
            
            // 0x23: inc hl (M:2 T:6)
            // -- M2 (generic)
            case 0x0051: cpu->hlx[cpu->hlx_idx].hl++; break;
            // -- OVERLAP
            case 0x0052: _fetch(); break;
            
            // 0x24: inc h (M:1 T:4)
            // -- OVERLAP
            case 0x0053: cpu->hlx[cpu->hlx_idx].h=z80_inc8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); break;
            
            // 0x25: dec h (M:1 T:4)
            // -- OVERLAP
            case 0x0054: cpu->hlx[cpu->hlx_idx].h=z80_dec8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); break;
            
            // 0x26: ld h,n (M:2 T:7)
            // -- M2
            case 0x0055: _wait();_mread(cpu->pc++); break;
            case 0x0056: cpu->hlx[cpu->hlx_idx].h=_gd(); break;
            // -- OVERLAP
            case 0x0057: _fetch(); break;
            
            // 0x27: daa (M:1 T:4)
            // -- OVERLAP
            case 0x0058: z80_daa(cpu);_fetch(); break;
            
            // 0x28: jr z,d (M:3 T:12)
            // -- M2
            case 0x0059: _wait();_mread(cpu->pc++); break;
            case 0x005A: cpu->dlatch=_gd();if(!(_cc_z)){z80_skip(cpu,1,7,2);}; break;
            // -- M3 (generic)
            case 0x005B: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;; break;
            // -- OVERLAP
            case 0x005C: _fetch(); break;
            
            // 0x29: add hl,hl (M:1 T:4)
            // -- OVERLAP
            case 0x005D: _fetch(); break;
            
            // 0x2A: ld hl,(nn) (M:1 T:4)
            // -- OVERLAP
            case 0x005E: _fetch(); break;
            
            // 0x2B: dec hl (M:2 T:6)
            // -- M2 (generic)
            case 0x005F: cpu->hlx[cpu->hlx_idx].hl--; break;
            // -- OVERLAP
            case 0x0060: _fetch(); break;
            
            // 0x2C: inc l (M:1 T:4)
            // -- OVERLAP
            case 0x0061: cpu->hlx[cpu->hlx_idx].l=z80_inc8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); break;
            
            // 0x2D: dec l (M:1 T:4)
            // -- OVERLAP
            case 0x0062: cpu->hlx[cpu->hlx_idx].l=z80_dec8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); break;
            
            // 0x2E: ld l,n (M:2 T:7)
            // -- M2
            case 0x0063: _wait();_mread(cpu->pc++); break;
            case 0x0064: cpu->hlx[cpu->hlx_idx].l=_gd(); break;
            // -- OVERLAP
            case 0x0065: _fetch(); break;
            
            // 0x2F: cpl (M:1 T:4)
            // -- OVERLAP
            case 0x0066: z80_cpl(cpu);_fetch(); break;
            
            // 0x30: jr nc,d (M:3 T:12)
            // -- M2
            case 0x0067: _wait();_mread(cpu->pc++); break;
            case 0x0068: cpu->dlatch=_gd();if(!(_cc_nc)){z80_skip(cpu,1,7,2);}; break;
            // -- M3 (generic)
            case 0x0069: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;; break;
            // -- OVERLAP
            case 0x006A: _fetch(); break;
            
            // 0x31: ld sp,nn (M:3 T:10)
            // -- M2
            case 0x006B: _wait();_mread(cpu->pc++); break;
            case 0x006C: cpu->spl=_gd(); break;
            // -- M3
            case 0x006D: _wait();_mread(cpu->pc++); break;
            case 0x006E: cpu->sph=_gd(); break;
            // -- OVERLAP
            case 0x006F: _fetch(); break;
            
            // 0x32: ld (nn),a (M:4 T:13)
            // -- M2
            case 0x0070: _wait();_mread(cpu->pc++); break;
            case 0x0071: cpu->wzl=_gd(); break;
            // -- M3
            case 0x0072: _wait();_mread(cpu->pc++); break;
            case 0x0073: cpu->wzh=_gd(); break;
            // -- M4
            case 0x0074: _mwrite(cpu->wz++,cpu->a);cpu->wzh=cpu->a; break;
            case 0x0075: _wait() break;
            // -- OVERLAP
            case 0x0076: _fetch(); break;
            
            // 0x33: inc sp (M:2 T:6)
            // -- M2 (generic)
            case 0x0077: cpu->sp++; break;
            // -- OVERLAP
            case 0x0078: _fetch(); break;
            
            // 0x34: inc (hl) (M:3 T:11)
            // -- M2
            case 0x0079: _wait();_mread(cpu->addr); break;
            case 0x007A: cpu->dlatch=_gd();cpu->dlatch=z80_inc8(cpu,cpu->dlatch); break;
            // -- M3
            case 0x007B: _mwrite(cpu->addr,cpu->dlatch); break;
            case 0x007C: _wait() break;
            // -- OVERLAP
            case 0x007D: _fetch(); break;
            
            // 0x35: dec (hl) (M:3 T:11)
            // -- M2
            case 0x007E: _wait();_mread(cpu->addr); break;
            case 0x007F: cpu->dlatch=_gd();cpu->dlatch=z80_dec8(cpu,cpu->dlatch); break;
            // -- M3
            case 0x0080: _mwrite(cpu->addr,cpu->dlatch); break;
            case 0x0081: _wait() break;
            // -- OVERLAP
            case 0x0082: _fetch(); break;
            
            // 0x36: ld (hl),n (M:3 T:10)
            // -- M2
            case 0x0083: _wait();_mread(cpu->pc++); break;
            case 0x0084: cpu->dlatch=_gd(); break;
            // -- M3
            case 0x0085: _mwrite(cpu->addr,cpu->dlatch); break;
            case 0x0086: _wait() break;
            // -- OVERLAP
            case 0x0087: _fetch(); break;
            
            // 0x37: scf (M:1 T:4)
            // -- OVERLAP
            case 0x0088: z80_scf(cpu);_fetch(); break;
            
            // 0x38: jr c,d (M:3 T:12)
            // -- M2
            case 0x0089: _wait();_mread(cpu->pc++); break;
            case 0x008A: cpu->dlatch=_gd();if(!(_cc_c)){z80_skip(cpu,1,7,2);}; break;
            // -- M3 (generic)
            case 0x008B: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;; break;
            // -- OVERLAP
            case 0x008C: _fetch(); break;
            
            // 0x39: add hl,sp (M:1 T:4)
            // -- OVERLAP
            case 0x008D: _fetch(); break;
            
            // 0x3A: ld a,(nn) (M:4 T:13)
            // -- M2
            case 0x008E: _wait();_mread(cpu->pc++); break;
            case 0x008F: cpu->wzl=_gd(); break;
            // -- M3
            case 0x0090: _wait();_mread(cpu->pc++); break;
            case 0x0091: cpu->wzh=_gd(); break;
            // -- M4
            case 0x0092: _wait();_mread(cpu->wz++); break;
            case 0x0093: cpu->a=_gd(); break;
            // -- OVERLAP
            case 0x0094: _fetch(); break;
            
            // 0x3B: dec sp (M:2 T:6)
            // -- M2 (generic)
            case 0x0095: cpu->sp--; break;
            // -- OVERLAP
            case 0x0096: _fetch(); break;
            
            // 0x3C: inc a (M:1 T:4)
            // -- OVERLAP
            case 0x0097: cpu->a=z80_inc8(cpu,cpu->a);_fetch(); break;
            
            // 0x3D: dec a (M:1 T:4)
            // -- OVERLAP
            case 0x0098: cpu->a=z80_dec8(cpu,cpu->a);_fetch(); break;
            
            // 0x3E: ld a,n (M:2 T:7)
            // -- M2
            case 0x0099: _wait();_mread(cpu->pc++); break;
            case 0x009A: cpu->a=_gd(); break;
            // -- OVERLAP
            case 0x009B: _fetch(); break;
            
            // 0x3F: ccf (M:1 T:4)
            // -- OVERLAP
            case 0x009C: z80_ccf(cpu);_fetch(); break;
            
            // 0x40: ld b,b (M:1 T:4)
            // -- OVERLAP
            case 0x009D: cpu->b=cpu->b;_fetch(); break;
            
            // 0x41: ld b,c (M:1 T:4)
            // -- OVERLAP
            case 0x009E: cpu->b=cpu->c;_fetch(); break;
            
            // 0x42: ld b,d (M:1 T:4)
            // -- OVERLAP
            case 0x009F: cpu->b=cpu->d;_fetch(); break;
            
            // 0x43: ld b,e (M:1 T:4)
            // -- OVERLAP
            case 0x00A0: cpu->b=cpu->e;_fetch(); break;
            
            // 0x44: ld b,h (M:1 T:4)
            // -- OVERLAP
            case 0x00A1: cpu->b=cpu->hlx[cpu->hlx_idx].h;_fetch(); break;
            
            // 0x45: ld b,l (M:1 T:4)
            // -- OVERLAP
            case 0x00A2: cpu->b=cpu->hlx[cpu->hlx_idx].l;_fetch(); break;
            
            // 0x46: ld b,(hl) (M:2 T:7)
            // -- M2
            case 0x00A3: _wait();_mread(cpu->addr); break;
            case 0x00A4: cpu->b=_gd(); break;
            // -- OVERLAP
            case 0x00A5: _fetch(); break;
            
            // 0x47: ld b,a (M:1 T:4)
            // -- OVERLAP
            case 0x00A6: cpu->b=cpu->a;_fetch(); break;
            
            // 0x48: ld c,b (M:1 T:4)
            // -- OVERLAP
            case 0x00A7: cpu->c=cpu->b;_fetch(); break;
            
            // 0x49: ld c,c (M:1 T:4)
            // -- OVERLAP
            case 0x00A8: cpu->c=cpu->c;_fetch(); break;
            
            // 0x4A: ld c,d (M:1 T:4)
            // -- OVERLAP
            case 0x00A9: cpu->c=cpu->d;_fetch(); break;
            
            // 0x4B: ld c,e (M:1 T:4)
            // -- OVERLAP
            case 0x00AA: cpu->c=cpu->e;_fetch(); break;
            
            // 0x4C: ld c,h (M:1 T:4)
            // -- OVERLAP
            case 0x00AB: cpu->c=cpu->hlx[cpu->hlx_idx].h;_fetch(); break;
            
            // 0x4D: ld c,l (M:1 T:4)
            // -- OVERLAP
            case 0x00AC: cpu->c=cpu->hlx[cpu->hlx_idx].l;_fetch(); break;
            
            // 0x4E: ld c,(hl) (M:2 T:7)
            // -- M2
            case 0x00AD: _wait();_mread(cpu->addr); break;
            case 0x00AE: cpu->c=_gd(); break;
            // -- OVERLAP
            case 0x00AF: _fetch(); break;
            
            // 0x4F: ld c,a (M:1 T:4)
            // -- OVERLAP
            case 0x00B0: cpu->c=cpu->a;_fetch(); break;
            
            // 0x50: ld d,b (M:1 T:4)
            // -- OVERLAP
            case 0x00B1: cpu->d=cpu->b;_fetch(); break;
            
            // 0x51: ld d,c (M:1 T:4)
            // -- OVERLAP
            case 0x00B2: cpu->d=cpu->c;_fetch(); break;
            
            // 0x52: ld d,d (M:1 T:4)
            // -- OVERLAP
            case 0x00B3: cpu->d=cpu->d;_fetch(); break;
            
            // 0x53: ld d,e (M:1 T:4)
            // -- OVERLAP
            case 0x00B4: cpu->d=cpu->e;_fetch(); break;
            
            // 0x54: ld d,h (M:1 T:4)
            // -- OVERLAP
            case 0x00B5: cpu->d=cpu->hlx[cpu->hlx_idx].h;_fetch(); break;
            
            // 0x55: ld d,l (M:1 T:4)
            // -- OVERLAP
            case 0x00B6: cpu->d=cpu->hlx[cpu->hlx_idx].l;_fetch(); break;
            
            // 0x56: ld d,(hl) (M:2 T:7)
            // -- M2
            case 0x00B7: _wait();_mread(cpu->addr); break;
            case 0x00B8: cpu->d=_gd(); break;
            // -- OVERLAP
            case 0x00B9: _fetch(); break;
            
            // 0x57: ld d,a (M:1 T:4)
            // -- OVERLAP
            case 0x00BA: cpu->d=cpu->a;_fetch(); break;
            
            // 0x58: ld e,b (M:1 T:4)
            // -- OVERLAP
            case 0x00BB: cpu->e=cpu->b;_fetch(); break;
            
            // 0x59: ld e,c (M:1 T:4)
            // -- OVERLAP
            case 0x00BC: cpu->e=cpu->c;_fetch(); break;
            
            // 0x5A: ld e,d (M:1 T:4)
            // -- OVERLAP
            case 0x00BD: cpu->e=cpu->d;_fetch(); break;
            
            // 0x5B: ld e,e (M:1 T:4)
            // -- OVERLAP
            case 0x00BE: cpu->e=cpu->e;_fetch(); break;
            
            // 0x5C: ld e,h (M:1 T:4)
            // -- OVERLAP
            case 0x00BF: cpu->e=cpu->hlx[cpu->hlx_idx].h;_fetch(); break;
            
            // 0x5D: ld e,l (M:1 T:4)
            // -- OVERLAP
            case 0x00C0: cpu->e=cpu->hlx[cpu->hlx_idx].l;_fetch(); break;
            
            // 0x5E: ld e,(hl) (M:2 T:7)
            // -- M2
            case 0x00C1: _wait();_mread(cpu->addr); break;
            case 0x00C2: cpu->e=_gd(); break;
            // -- OVERLAP
            case 0x00C3: _fetch(); break;
            
            // 0x5F: ld e,a (M:1 T:4)
            // -- OVERLAP
            case 0x00C4: cpu->e=cpu->a;_fetch(); break;
            
            // 0x60: ld h,b (M:1 T:4)
            // -- OVERLAP
            case 0x00C5: cpu->hlx[cpu->hlx_idx].h=cpu->b;_fetch(); break;
            
            // 0x61: ld h,c (M:1 T:4)
            // -- OVERLAP
            case 0x00C6: cpu->hlx[cpu->hlx_idx].h=cpu->c;_fetch(); break;
            
            // 0x62: ld h,d (M:1 T:4)
            // -- OVERLAP
            case 0x00C7: cpu->hlx[cpu->hlx_idx].h=cpu->d;_fetch(); break;
            
            // 0x63: ld h,e (M:1 T:4)
            // -- OVERLAP
            case 0x00C8: cpu->hlx[cpu->hlx_idx].h=cpu->e;_fetch(); break;
            
            // 0x64: ld h,h (M:1 T:4)
            // -- OVERLAP
            case 0x00C9: cpu->hlx[cpu->hlx_idx].h=cpu->hlx[cpu->hlx_idx].h;_fetch(); break;
            
            // 0x65: ld h,l (M:1 T:4)
            // -- OVERLAP
            case 0x00CA: cpu->hlx[cpu->hlx_idx].h=cpu->hlx[cpu->hlx_idx].l;_fetch(); break;
            
            // 0x66: ld h,(hl) (M:2 T:7)
            // -- M2
            case 0x00CB: _wait();_mread(cpu->addr); break;
            case 0x00CC: cpu->h=_gd(); break;
            // -- OVERLAP
            case 0x00CD: _fetch(); break;
            
            // 0x67: ld h,a (M:1 T:4)
            // -- OVERLAP
            case 0x00CE: cpu->hlx[cpu->hlx_idx].h=cpu->a;_fetch(); break;
            
            // 0x68: ld l,b (M:1 T:4)
            // -- OVERLAP
            case 0x00CF: cpu->hlx[cpu->hlx_idx].l=cpu->b;_fetch(); break;
            
            // 0x69: ld l,c (M:1 T:4)
            // -- OVERLAP
            case 0x00D0: cpu->hlx[cpu->hlx_idx].l=cpu->c;_fetch(); break;
            
            // 0x6A: ld l,d (M:1 T:4)
            // -- OVERLAP
            case 0x00D1: cpu->hlx[cpu->hlx_idx].l=cpu->d;_fetch(); break;
            
            // 0x6B: ld l,e (M:1 T:4)
            // -- OVERLAP
            case 0x00D2: cpu->hlx[cpu->hlx_idx].l=cpu->e;_fetch(); break;
            
            // 0x6C: ld l,h (M:1 T:4)
            // -- OVERLAP
            case 0x00D3: cpu->hlx[cpu->hlx_idx].l=cpu->hlx[cpu->hlx_idx].h;_fetch(); break;
            
            // 0x6D: ld l,l (M:1 T:4)
            // -- OVERLAP
            case 0x00D4: cpu->hlx[cpu->hlx_idx].l=cpu->hlx[cpu->hlx_idx].l;_fetch(); break;
            
            // 0x6E: ld l,(hl) (M:2 T:7)
            // -- M2
            case 0x00D5: _wait();_mread(cpu->addr); break;
            case 0x00D6: cpu->l=_gd(); break;
            // -- OVERLAP
            case 0x00D7: _fetch(); break;
            
            // 0x6F: ld l,a (M:1 T:4)
            // -- OVERLAP
            case 0x00D8: cpu->hlx[cpu->hlx_idx].l=cpu->a;_fetch(); break;
            
            // 0x70: ld (hl),b (M:2 T:7)
            // -- M2
            case 0x00D9: _mwrite(cpu->addr,cpu->b); break;
            case 0x00DA: _wait() break;
            // -- OVERLAP
            case 0x00DB: _fetch(); break;
            
            // 0x71: ld (hl),c (M:2 T:7)
            // -- M2
            case 0x00DC: _mwrite(cpu->addr,cpu->c); break;
            case 0x00DD: _wait() break;
            // -- OVERLAP
            case 0x00DE: _fetch(); break;
            
            // 0x72: ld (hl),d (M:2 T:7)
            // -- M2
            case 0x00DF: _mwrite(cpu->addr,cpu->d); break;
            case 0x00E0: _wait() break;
            // -- OVERLAP
            case 0x00E1: _fetch(); break;
            
            // 0x73: ld (hl),e (M:2 T:7)
            // -- M2
            case 0x00E2: _mwrite(cpu->addr,cpu->e); break;
            case 0x00E3: _wait() break;
            // -- OVERLAP
            case 0x00E4: _fetch(); break;
            
            // 0x74: ld (hl),h (M:2 T:7)
            // -- M2
            case 0x00E5: _mwrite(cpu->addr,cpu->h); break;
            case 0x00E6: _wait() break;
            // -- OVERLAP
            case 0x00E7: _fetch(); break;
            
            // 0x75: ld (hl),l (M:2 T:7)
            // -- M2
            case 0x00E8: _mwrite(cpu->addr,cpu->l); break;
            case 0x00E9: _wait() break;
            // -- OVERLAP
            case 0x00EA: _fetch(); break;
            
            // 0x76: halt (M:1 T:4)
            // -- OVERLAP
            case 0x00EB: pins=z80_halt(cpu,pins);_fetch(); break;
            
            // 0x77: ld (hl),a (M:2 T:7)
            // -- M2
            case 0x00EC: _mwrite(cpu->addr,cpu->a); break;
            case 0x00ED: _wait() break;
            // -- OVERLAP
            case 0x00EE: _fetch(); break;
            
            // 0x78: ld a,b (M:1 T:4)
            // -- OVERLAP
            case 0x00EF: cpu->a=cpu->b;_fetch(); break;
            
            // 0x79: ld a,c (M:1 T:4)
            // -- OVERLAP
            case 0x00F0: cpu->a=cpu->c;_fetch(); break;
            
            // 0x7A: ld a,d (M:1 T:4)
            // -- OVERLAP
            case 0x00F1: cpu->a=cpu->d;_fetch(); break;
            
            // 0x7B: ld a,e (M:1 T:4)
            // -- OVERLAP
            case 0x00F2: cpu->a=cpu->e;_fetch(); break;
            
            // 0x7C: ld a,h (M:1 T:4)
            // -- OVERLAP
            case 0x00F3: cpu->a=cpu->hlx[cpu->hlx_idx].h;_fetch(); break;
            
            // 0x7D: ld a,l (M:1 T:4)
            // -- OVERLAP
            case 0x00F4: cpu->a=cpu->hlx[cpu->hlx_idx].l;_fetch(); break;
            
            // 0x7E: ld a,(hl) (M:2 T:7)
            // -- M2
            case 0x00F5: _wait();_mread(cpu->addr); break;
            case 0x00F6: cpu->a=_gd(); break;
            // -- OVERLAP
            case 0x00F7: _fetch(); break;
            
            // 0x7F: ld a,a (M:1 T:4)
            // -- OVERLAP
            case 0x00F8: cpu->a=cpu->a;_fetch(); break;
            
            // 0x80: add b (M:1 T:4)
            // -- OVERLAP
            case 0x00F9: z80_add8(cpu,cpu->b);_fetch(); break;
            
            // 0x81: add c (M:1 T:4)
            // -- OVERLAP
            case 0x00FA: z80_add8(cpu,cpu->c);_fetch(); break;
            
            // 0x82: add d (M:1 T:4)
            // -- OVERLAP
            case 0x00FB: z80_add8(cpu,cpu->d);_fetch(); break;
            
            // 0x83: add e (M:1 T:4)
            // -- OVERLAP
            case 0x00FC: z80_add8(cpu,cpu->e);_fetch(); break;
            
            // 0x84: add h (M:1 T:4)
            // -- OVERLAP
            case 0x00FD: z80_add8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); break;
            
            // 0x85: add l (M:1 T:4)
            // -- OVERLAP
            case 0x00FE: z80_add8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); break;
            
            // 0x86: add (hl) (M:2 T:7)
            // -- M2
            case 0x00FF: _wait();_mread(cpu->addr); break;
            case 0x0100: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x0101: z80_add8(cpu,cpu->dlatch);_fetch(); break;
            
            // 0x87: add a (M:1 T:4)
            // -- OVERLAP
            case 0x0102: z80_add8(cpu,cpu->a);_fetch(); break;
            
            // 0x88: adc b (M:1 T:4)
            // -- OVERLAP
            case 0x0103: z80_adc8(cpu,cpu->b);_fetch(); break;
            
            // 0x89: adc c (M:1 T:4)
            // -- OVERLAP
            case 0x0104: z80_adc8(cpu,cpu->c);_fetch(); break;
            
            // 0x8A: adc d (M:1 T:4)
            // -- OVERLAP
            case 0x0105: z80_adc8(cpu,cpu->d);_fetch(); break;
            
            // 0x8B: adc e (M:1 T:4)
            // -- OVERLAP
            case 0x0106: z80_adc8(cpu,cpu->e);_fetch(); break;
            
            // 0x8C: adc h (M:1 T:4)
            // -- OVERLAP
            case 0x0107: z80_adc8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); break;
            
            // 0x8D: adc l (M:1 T:4)
            // -- OVERLAP
            case 0x0108: z80_adc8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); break;
            
            // 0x8E: adc (hl) (M:2 T:7)
            // -- M2
            case 0x0109: _wait();_mread(cpu->addr); break;
            case 0x010A: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x010B: z80_adc8(cpu,cpu->dlatch);_fetch(); break;
            
            // 0x8F: adc a (M:1 T:4)
            // -- OVERLAP
            case 0x010C: z80_adc8(cpu,cpu->a);_fetch(); break;
            
            // 0x90: sub b (M:1 T:4)
            // -- OVERLAP
            case 0x010D: z80_sub8(cpu,cpu->b);_fetch(); break;
            
            // 0x91: sub c (M:1 T:4)
            // -- OVERLAP
            case 0x010E: z80_sub8(cpu,cpu->c);_fetch(); break;
            
            // 0x92: sub d (M:1 T:4)
            // -- OVERLAP
            case 0x010F: z80_sub8(cpu,cpu->d);_fetch(); break;
            
            // 0x93: sub e (M:1 T:4)
            // -- OVERLAP
            case 0x0110: z80_sub8(cpu,cpu->e);_fetch(); break;
            
            // 0x94: sub h (M:1 T:4)
            // -- OVERLAP
            case 0x0111: z80_sub8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); break;
            
            // 0x95: sub l (M:1 T:4)
            // -- OVERLAP
            case 0x0112: z80_sub8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); break;
            
            // 0x96: sub (hl) (M:2 T:7)
            // -- M2
            case 0x0113: _wait();_mread(cpu->addr); break;
            case 0x0114: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x0115: z80_sub8(cpu,cpu->dlatch);_fetch(); break;
            
            // 0x97: sub a (M:1 T:4)
            // -- OVERLAP
            case 0x0116: z80_sub8(cpu,cpu->a);_fetch(); break;
            
            // 0x98: sbc b (M:1 T:4)
            // -- OVERLAP
            case 0x0117: z80_sbc8(cpu,cpu->b);_fetch(); break;
            
            // 0x99: sbc c (M:1 T:4)
            // -- OVERLAP
            case 0x0118: z80_sbc8(cpu,cpu->c);_fetch(); break;
            
            // 0x9A: sbc d (M:1 T:4)
            // -- OVERLAP
            case 0x0119: z80_sbc8(cpu,cpu->d);_fetch(); break;
            
            // 0x9B: sbc e (M:1 T:4)
            // -- OVERLAP
            case 0x011A: z80_sbc8(cpu,cpu->e);_fetch(); break;
            
            // 0x9C: sbc h (M:1 T:4)
            // -- OVERLAP
            case 0x011B: z80_sbc8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); break;
            
            // 0x9D: sbc l (M:1 T:4)
            // -- OVERLAP
            case 0x011C: z80_sbc8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); break;
            
            // 0x9E: sbc (hl) (M:2 T:7)
            // -- M2
            case 0x011D: _wait();_mread(cpu->addr); break;
            case 0x011E: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x011F: z80_sbc8(cpu,cpu->dlatch);_fetch(); break;
            
            // 0x9F: sbc a (M:1 T:4)
            // -- OVERLAP
            case 0x0120: z80_sbc8(cpu,cpu->a);_fetch(); break;
            
            // 0xA0: and b (M:1 T:4)
            // -- OVERLAP
            case 0x0121: z80_and8(cpu,cpu->b);_fetch(); break;
            
            // 0xA1: and c (M:1 T:4)
            // -- OVERLAP
            case 0x0122: z80_and8(cpu,cpu->c);_fetch(); break;
            
            // 0xA2: and d (M:1 T:4)
            // -- OVERLAP
            case 0x0123: z80_and8(cpu,cpu->d);_fetch(); break;
            
            // 0xA3: and e (M:1 T:4)
            // -- OVERLAP
            case 0x0124: z80_and8(cpu,cpu->e);_fetch(); break;
            
            // 0xA4: and h (M:1 T:4)
            // -- OVERLAP
            case 0x0125: z80_and8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); break;
            
            // 0xA5: and l (M:1 T:4)
            // -- OVERLAP
            case 0x0126: z80_and8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); break;
            
            // 0xA6: and (hl) (M:2 T:7)
            // -- M2
            case 0x0127: _wait();_mread(cpu->addr); break;
            case 0x0128: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x0129: z80_and8(cpu,cpu->dlatch);_fetch(); break;
            
            // 0xA7: and a (M:1 T:4)
            // -- OVERLAP
            case 0x012A: z80_and8(cpu,cpu->a);_fetch(); break;
            
            // 0xA8: xor b (M:1 T:4)
            // -- OVERLAP
            case 0x012B: z80_xor8(cpu,cpu->b);_fetch(); break;
            
            // 0xA9: xor c (M:1 T:4)
            // -- OVERLAP
            case 0x012C: z80_xor8(cpu,cpu->c);_fetch(); break;
            
            // 0xAA: xor d (M:1 T:4)
            // -- OVERLAP
            case 0x012D: z80_xor8(cpu,cpu->d);_fetch(); break;
            
            // 0xAB: xor e (M:1 T:4)
            // -- OVERLAP
            case 0x012E: z80_xor8(cpu,cpu->e);_fetch(); break;
            
            // 0xAC: xor h (M:1 T:4)
            // -- OVERLAP
            case 0x012F: z80_xor8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); break;
            
            // 0xAD: xor l (M:1 T:4)
            // -- OVERLAP
            case 0x0130: z80_xor8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); break;
            
            // 0xAE: xor (hl) (M:2 T:7)
            // -- M2
            case 0x0131: _wait();_mread(cpu->addr); break;
            case 0x0132: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x0133: z80_xor8(cpu,cpu->dlatch);_fetch(); break;
            
            // 0xAF: xor a (M:1 T:4)
            // -- OVERLAP
            case 0x0134: z80_xor8(cpu,cpu->a);_fetch(); break;
            
            // 0xB0: or b (M:1 T:4)
            // -- OVERLAP
            case 0x0135: z80_or8(cpu,cpu->b);_fetch(); break;
            
            // 0xB1: or c (M:1 T:4)
            // -- OVERLAP
            case 0x0136: z80_or8(cpu,cpu->c);_fetch(); break;
            
            // 0xB2: or d (M:1 T:4)
            // -- OVERLAP
            case 0x0137: z80_or8(cpu,cpu->d);_fetch(); break;
            
            // 0xB3: or e (M:1 T:4)
            // -- OVERLAP
            case 0x0138: z80_or8(cpu,cpu->e);_fetch(); break;
            
            // 0xB4: or h (M:1 T:4)
            // -- OVERLAP
            case 0x0139: z80_or8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); break;
            
            // 0xB5: or l (M:1 T:4)
            // -- OVERLAP
            case 0x013A: z80_or8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); break;
            
            // 0xB6: or (hl) (M:2 T:7)
            // -- M2
            case 0x013B: _wait();_mread(cpu->addr); break;
            case 0x013C: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x013D: z80_or8(cpu,cpu->dlatch);_fetch(); break;
            
            // 0xB7: or a (M:1 T:4)
            // -- OVERLAP
            case 0x013E: z80_or8(cpu,cpu->a);_fetch(); break;
            
            // 0xB8: cp b (M:1 T:4)
            // -- OVERLAP
            case 0x013F: z80_cp8(cpu,cpu->b);_fetch(); break;
            
            // 0xB9: cp c (M:1 T:4)
            // -- OVERLAP
            case 0x0140: z80_cp8(cpu,cpu->c);_fetch(); break;
            
            // 0xBA: cp d (M:1 T:4)
            // -- OVERLAP
            case 0x0141: z80_cp8(cpu,cpu->d);_fetch(); break;
            
            // 0xBB: cp e (M:1 T:4)
            // -- OVERLAP
            case 0x0142: z80_cp8(cpu,cpu->e);_fetch(); break;
            
            // 0xBC: cp h (M:1 T:4)
            // -- OVERLAP
            case 0x0143: z80_cp8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); break;
            
            // 0xBD: cp l (M:1 T:4)
            // -- OVERLAP
            case 0x0144: z80_cp8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); break;
            
            // 0xBE: cp (hl) (M:2 T:7)
            // -- M2
            case 0x0145: _wait();_mread(cpu->addr); break;
            case 0x0146: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x0147: z80_cp8(cpu,cpu->dlatch);_fetch(); break;
            
            // 0xBF: cp a (M:1 T:4)
            // -- OVERLAP
            case 0x0148: z80_cp8(cpu,cpu->a);_fetch(); break;
            
            // 0xC0: ret nz (M:4 T:11)
            // -- M2 (generic)
            case 0x0149: if(!_cc_nz){z80_skip(cpu,4,7,1);}; break;
            // -- M3
            case 0x014A: _wait();_mread(cpu->sp++); break;
            case 0x014B: cpu->wzl=_gd(); break;
            // -- M4
            case 0x014C: _wait();_mread(cpu->sp++); break;
            case 0x014D: cpu->wzh=_gd();cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x014E: _fetch(); break;
            
            // 0xC1: pop bc (M:3 T:10)
            // -- M2
            case 0x014F: _wait();_mread(cpu->sp++); break;
            case 0x0150: cpu->c=_gd(); break;
            // -- M3
            case 0x0151: _wait();_mread(cpu->sp++); break;
            case 0x0152: cpu->b=_gd(); break;
            // -- OVERLAP
            case 0x0153: _fetch(); break;
            
            // 0xC2: jp nz,nn (M:3 T:10)
            // -- M2
            case 0x0154: _wait();_mread(cpu->pc++); break;
            case 0x0155: cpu->wzl=_gd(); break;
            // -- M3
            case 0x0156: _wait();_mread(cpu->pc++); break;
            case 0x0157: cpu->wzh=_gd();if(_cc_nz){cpu->pc=cpu->wz;}; break;
            // -- OVERLAP
            case 0x0158: _fetch(); break;
            
            // 0xC3: jp nn (M:3 T:10)
            // -- M2
            case 0x0159: _wait();_mread(cpu->pc++); break;
            case 0x015A: cpu->wzl=_gd(); break;
            // -- M3
            case 0x015B: _wait();_mread(cpu->pc++); break;
            case 0x015C: cpu->wzh=_gd();cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x015D: _fetch(); break;
            
            // 0xC4: call nz,nn (M:6 T:17)
            // -- M2
            case 0x015E: _wait();_mread(cpu->pc++); break;
            case 0x015F: cpu->wzl=_gd(); break;
            // -- M3
            case 0x0160: _wait();_mread(cpu->pc++); break;
            case 0x0161: cpu->wzh=_gd();if (!_cc_nz){z80_skip(cpu,5,9,2);}; break;
            // -- M4 (generic)
            case 0x0162:  break;
            // -- M5
            case 0x0163: _mwrite(--cpu->sp,cpu->pch); break;
            case 0x0164: _wait() break;
            // -- M6
            case 0x0165: _mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz; break;
            case 0x0166: _wait() break;
            // -- OVERLAP
            case 0x0167: _fetch(); break;
            
            // 0xC5: push bc (M:3 T:11)
            // -- M2
            case 0x0168: _mwrite(--cpu->sp,cpu->b); break;
            case 0x0169: _wait() break;
            // -- M3
            case 0x016A: _mwrite(--cpu->sp,cpu->c); break;
            case 0x016B: _wait() break;
            // -- OVERLAP
            case 0x016C: _fetch(); break;
            
            // 0xC6: add n (M:2 T:7)
            // -- M2
            case 0x016D: _wait();_mread(cpu->pc++); break;
            case 0x016E: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x016F: z80_add8(cpu,cpu->dlatch);_fetch(); break;
            
            // 0xC7: rst 0h (M:1 T:4)
            // -- OVERLAP
            case 0x0170: _fetch(); break;
            
            // 0xC8: ret z (M:4 T:11)
            // -- M2 (generic)
            case 0x0171: if(!_cc_z){z80_skip(cpu,4,7,1);}; break;
            // -- M3
            case 0x0172: _wait();_mread(cpu->sp++); break;
            case 0x0173: cpu->wzl=_gd(); break;
            // -- M4
            case 0x0174: _wait();_mread(cpu->sp++); break;
            case 0x0175: cpu->wzh=_gd();cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x0176: _fetch(); break;
            
            // 0xC9: ret (M:3 T:10)
            // -- M2
            case 0x0177: _wait();_mread(cpu->sp++); break;
            case 0x0178: cpu->wzl=_gd(); break;
            // -- M3
            case 0x0179: _wait();_mread(cpu->sp++); break;
            case 0x017A: cpu->wzh=_gd();cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x017B: _fetch(); break;
            
            // 0xCA: jp z,nn (M:3 T:10)
            // -- M2
            case 0x017C: _wait();_mread(cpu->pc++); break;
            case 0x017D: cpu->wzl=_gd(); break;
            // -- M3
            case 0x017E: _wait();_mread(cpu->pc++); break;
            case 0x017F: cpu->wzh=_gd();if(_cc_z){cpu->pc=cpu->wz;}; break;
            // -- OVERLAP
            case 0x0180: _fetch(); break;
            
            // 0xCB: cb prefix (M:1 T:4)
            // -- OVERLAP
            case 0x0181: _fetch(); break;
            
            // 0xCC: call z,nn (M:6 T:17)
            // -- M2
            case 0x0182: _wait();_mread(cpu->pc++); break;
            case 0x0183: cpu->wzl=_gd(); break;
            // -- M3
            case 0x0184: _wait();_mread(cpu->pc++); break;
            case 0x0185: cpu->wzh=_gd();if (!_cc_z){z80_skip(cpu,5,9,2);}; break;
            // -- M4 (generic)
            case 0x0186:  break;
            // -- M5
            case 0x0187: _mwrite(--cpu->sp,cpu->pch); break;
            case 0x0188: _wait() break;
            // -- M6
            case 0x0189: _mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz; break;
            case 0x018A: _wait() break;
            // -- OVERLAP
            case 0x018B: _fetch(); break;
            
            // 0xCD: call nn (M:5 T:17)
            // -- M2
            case 0x018C: _wait();_mread(cpu->pc++); break;
            case 0x018D: cpu->wzl=_gd(); break;
            // -- M3
            case 0x018E: _wait();_mread(cpu->pc++); break;
            case 0x018F: cpu->wzh=_gd(); break;
            // -- M4
            case 0x0190: _mwrite(--cpu->sp,cpu->pch); break;
            case 0x0191: _wait() break;
            // -- M5
            case 0x0192: _mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz; break;
            case 0x0193: _wait() break;
            // -- OVERLAP
            case 0x0194: _fetch(); break;
            
            // 0xCE: adc n (M:2 T:7)
            // -- M2
            case 0x0195: _wait();_mread(cpu->pc++); break;
            case 0x0196: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x0197: z80_adc8(cpu,cpu->dlatch);_fetch(); break;
            
            // 0xCF: rst 8h (M:1 T:4)
            // -- OVERLAP
            case 0x0198: _fetch(); break;
            
            // 0xD0: ret nc (M:4 T:11)
            // -- M2 (generic)
            case 0x0199: if(!_cc_nc){z80_skip(cpu,4,7,1);}; break;
            // -- M3
            case 0x019A: _wait();_mread(cpu->sp++); break;
            case 0x019B: cpu->wzl=_gd(); break;
            // -- M4
            case 0x019C: _wait();_mread(cpu->sp++); break;
            case 0x019D: cpu->wzh=_gd();cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x019E: _fetch(); break;
            
            // 0xD1: pop de (M:3 T:10)
            // -- M2
            case 0x019F: _wait();_mread(cpu->sp++); break;
            case 0x01A0: cpu->e=_gd(); break;
            // -- M3
            case 0x01A1: _wait();_mread(cpu->sp++); break;
            case 0x01A2: cpu->d=_gd(); break;
            // -- OVERLAP
            case 0x01A3: _fetch(); break;
            
            // 0xD2: jp nc,nn (M:3 T:10)
            // -- M2
            case 0x01A4: _wait();_mread(cpu->pc++); break;
            case 0x01A5: cpu->wzl=_gd(); break;
            // -- M3
            case 0x01A6: _wait();_mread(cpu->pc++); break;
            case 0x01A7: cpu->wzh=_gd();if(_cc_nc){cpu->pc=cpu->wz;}; break;
            // -- OVERLAP
            case 0x01A8: _fetch(); break;
            
            // 0xD3: out (n),a (M:1 T:4)
            // -- OVERLAP
            case 0x01A9: _fetch(); break;
            
            // 0xD4: call nc,nn (M:6 T:17)
            // -- M2
            case 0x01AA: _wait();_mread(cpu->pc++); break;
            case 0x01AB: cpu->wzl=_gd(); break;
            // -- M3
            case 0x01AC: _wait();_mread(cpu->pc++); break;
            case 0x01AD: cpu->wzh=_gd();if (!_cc_nc){z80_skip(cpu,5,9,2);}; break;
            // -- M4 (generic)
            case 0x01AE:  break;
            // -- M5
            case 0x01AF: _mwrite(--cpu->sp,cpu->pch); break;
            case 0x01B0: _wait() break;
            // -- M6
            case 0x01B1: _mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz; break;
            case 0x01B2: _wait() break;
            // -- OVERLAP
            case 0x01B3: _fetch(); break;
            
            // 0xD5: push de (M:3 T:11)
            // -- M2
            case 0x01B4: _mwrite(--cpu->sp,cpu->d); break;
            case 0x01B5: _wait() break;
            // -- M3
            case 0x01B6: _mwrite(--cpu->sp,cpu->e); break;
            case 0x01B7: _wait() break;
            // -- OVERLAP
            case 0x01B8: _fetch(); break;
            
            // 0xD6: sub n (M:2 T:7)
            // -- M2
            case 0x01B9: _wait();_mread(cpu->pc++); break;
            case 0x01BA: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x01BB: z80_sub8(cpu,cpu->dlatch);_fetch(); break;
            
            // 0xD7: rst 10h (M:1 T:4)
            // -- OVERLAP
            case 0x01BC: _fetch(); break;
            
            // 0xD8: ret c (M:4 T:11)
            // -- M2 (generic)
            case 0x01BD: if(!_cc_c){z80_skip(cpu,4,7,1);}; break;
            // -- M3
            case 0x01BE: _wait();_mread(cpu->sp++); break;
            case 0x01BF: cpu->wzl=_gd(); break;
            // -- M4
            case 0x01C0: _wait();_mread(cpu->sp++); break;
            case 0x01C1: cpu->wzh=_gd();cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x01C2: _fetch(); break;
            
            // 0xD9: exx (M:1 T:4)
            // -- OVERLAP
            case 0x01C3: z80_exx(cpu);_fetch(); break;
            
            // 0xDA: jp c,nn (M:3 T:10)
            // -- M2
            case 0x01C4: _wait();_mread(cpu->pc++); break;
            case 0x01C5: cpu->wzl=_gd(); break;
            // -- M3
            case 0x01C6: _wait();_mread(cpu->pc++); break;
            case 0x01C7: cpu->wzh=_gd();if(_cc_c){cpu->pc=cpu->wz;}; break;
            // -- OVERLAP
            case 0x01C8: _fetch(); break;
            
            // 0xDB: in a,(n) (M:1 T:4)
            // -- OVERLAP
            case 0x01C9: _fetch(); break;
            
            // 0xDC: call c,nn (M:6 T:17)
            // -- M2
            case 0x01CA: _wait();_mread(cpu->pc++); break;
            case 0x01CB: cpu->wzl=_gd(); break;
            // -- M3
            case 0x01CC: _wait();_mread(cpu->pc++); break;
            case 0x01CD: cpu->wzh=_gd();if (!_cc_c){z80_skip(cpu,5,9,2);}; break;
            // -- M4 (generic)
            case 0x01CE:  break;
            // -- M5
            case 0x01CF: _mwrite(--cpu->sp,cpu->pch); break;
            case 0x01D0: _wait() break;
            // -- M6
            case 0x01D1: _mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz; break;
            case 0x01D2: _wait() break;
            // -- OVERLAP
            case 0x01D3: _fetch(); break;
            
            // 0xDD: dd prefix (M:1 T:4)
            // -- OVERLAP
            case 0x01D4: _fetch_ix(); break;
            
            // 0xDE: sbc n (M:2 T:7)
            // -- M2
            case 0x01D5: _wait();_mread(cpu->pc++); break;
            case 0x01D6: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x01D7: z80_sbc8(cpu,cpu->dlatch);_fetch(); break;
            
            // 0xDF: rst 18h (M:1 T:4)
            // -- OVERLAP
            case 0x01D8: _fetch(); break;
            
            // 0xE0: ret po (M:4 T:11)
            // -- M2 (generic)
            case 0x01D9: if(!_cc_po){z80_skip(cpu,4,7,1);}; break;
            // -- M3
            case 0x01DA: _wait();_mread(cpu->sp++); break;
            case 0x01DB: cpu->wzl=_gd(); break;
            // -- M4
            case 0x01DC: _wait();_mread(cpu->sp++); break;
            case 0x01DD: cpu->wzh=_gd();cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x01DE: _fetch(); break;
            
            // 0xE1: pop hl (M:3 T:10)
            // -- M2
            case 0x01DF: _wait();_mread(cpu->sp++); break;
            case 0x01E0: cpu->hlx[cpu->hlx_idx].l=_gd(); break;
            // -- M3
            case 0x01E1: _wait();_mread(cpu->sp++); break;
            case 0x01E2: cpu->hlx[cpu->hlx_idx].h=_gd(); break;
            // -- OVERLAP
            case 0x01E3: _fetch(); break;
            
            // 0xE2: jp po,nn (M:3 T:10)
            // -- M2
            case 0x01E4: _wait();_mread(cpu->pc++); break;
            case 0x01E5: cpu->wzl=_gd(); break;
            // -- M3
            case 0x01E6: _wait();_mread(cpu->pc++); break;
            case 0x01E7: cpu->wzh=_gd();if(_cc_po){cpu->pc=cpu->wz;}; break;
            // -- OVERLAP
            case 0x01E8: _fetch(); break;
            
            // 0xE3: ex (sp),hl (M:5 T:19)
            // -- M2
            case 0x01E9: _wait();_mread(cpu->sp); break;
            case 0x01EA: cpu->wzl=_gd(); break;
            // -- M3
            case 0x01EB: _wait();_mread(cpu->sp+1); break;
            case 0x01EC: cpu->wzh=_gd(); break;
            // -- M4
            case 0x01ED: _mwrite(cpu->sp+1,cpu->hlx[cpu->hlx_idx].h); break;
            case 0x01EE: _wait() break;
            // -- M5
            case 0x01EF: _mwrite(cpu->sp,cpu->hlx[cpu->hlx_idx].l);cpu->hlx[cpu->hlx_idx].hl=cpu->wz; break;
            case 0x01F0: _wait() break;
            // -- OVERLAP
            case 0x01F1: _fetch(); break;
            
            // 0xE4: call po,nn (M:6 T:17)
            // -- M2
            case 0x01F2: _wait();_mread(cpu->pc++); break;
            case 0x01F3: cpu->wzl=_gd(); break;
            // -- M3
            case 0x01F4: _wait();_mread(cpu->pc++); break;
            case 0x01F5: cpu->wzh=_gd();if (!_cc_po){z80_skip(cpu,5,9,2);}; break;
            // -- M4 (generic)
            case 0x01F6:  break;
            // -- M5
            case 0x01F7: _mwrite(--cpu->sp,cpu->pch); break;
            case 0x01F8: _wait() break;
            // -- M6
            case 0x01F9: _mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz; break;
            case 0x01FA: _wait() break;
            // -- OVERLAP
            case 0x01FB: _fetch(); break;
            
            // 0xE5: push hl (M:3 T:11)
            // -- M2
            case 0x01FC: _mwrite(--cpu->sp,cpu->hlx[cpu->hlx_idx].h); break;
            case 0x01FD: _wait() break;
            // -- M3
            case 0x01FE: _mwrite(--cpu->sp,cpu->hlx[cpu->hlx_idx].l); break;
            case 0x01FF: _wait() break;
            // -- OVERLAP
            case 0x0200: _fetch(); break;
            
            // 0xE6: and n (M:2 T:7)
            // -- M2
            case 0x0201: _wait();_mread(cpu->pc++); break;
            case 0x0202: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x0203: z80_and8(cpu,cpu->dlatch);_fetch(); break;
            
            // 0xE7: rst 20h (M:1 T:4)
            // -- OVERLAP
            case 0x0204: _fetch(); break;
            
            // 0xE8: ret pe (M:4 T:11)
            // -- M2 (generic)
            case 0x0205: if(!_cc_pe){z80_skip(cpu,4,7,1);}; break;
            // -- M3
            case 0x0206: _wait();_mread(cpu->sp++); break;
            case 0x0207: cpu->wzl=_gd(); break;
            // -- M4
            case 0x0208: _wait();_mread(cpu->sp++); break;
            case 0x0209: cpu->wzh=_gd();cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x020A: _fetch(); break;
            
            // 0xE9: jp hl (M:1 T:4)
            // -- OVERLAP
            case 0x020B: cpu->pc=cpu->hlx[cpu->hlx_idx].hl;_fetch(); break;
            
            // 0xEA: jp pe,nn (M:3 T:10)
            // -- M2
            case 0x020C: _wait();_mread(cpu->pc++); break;
            case 0x020D: cpu->wzl=_gd(); break;
            // -- M3
            case 0x020E: _wait();_mread(cpu->pc++); break;
            case 0x020F: cpu->wzh=_gd();if(_cc_pe){cpu->pc=cpu->wz;}; break;
            // -- OVERLAP
            case 0x0210: _fetch(); break;
            
            // 0xEB: ex de,hl (M:1 T:4)
            // -- OVERLAP
            case 0x0211: z80_ex_de_hl(cpu);_fetch(); break;
            
            // 0xEC: call pe,nn (M:6 T:17)
            // -- M2
            case 0x0212: _wait();_mread(cpu->pc++); break;
            case 0x0213: cpu->wzl=_gd(); break;
            // -- M3
            case 0x0214: _wait();_mread(cpu->pc++); break;
            case 0x0215: cpu->wzh=_gd();if (!_cc_pe){z80_skip(cpu,5,9,2);}; break;
            // -- M4 (generic)
            case 0x0216:  break;
            // -- M5
            case 0x0217: _mwrite(--cpu->sp,cpu->pch); break;
            case 0x0218: _wait() break;
            // -- M6
            case 0x0219: _mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz; break;
            case 0x021A: _wait() break;
            // -- OVERLAP
            case 0x021B: _fetch(); break;
            
            // 0xED: ed prefix (M:1 T:4)
            // -- OVERLAP
            case 0x021C: _fetch(); break;
            
            // 0xEE: xor n (M:2 T:7)
            // -- M2
            case 0x021D: _wait();_mread(cpu->pc++); break;
            case 0x021E: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x021F: z80_xor8(cpu,cpu->dlatch);_fetch(); break;
            
            // 0xEF: rst 28h (M:1 T:4)
            // -- OVERLAP
            case 0x0220: _fetch(); break;
            
            // 0xF0: ret p (M:4 T:11)
            // -- M2 (generic)
            case 0x0221: if(!_cc_p){z80_skip(cpu,4,7,1);}; break;
            // -- M3
            case 0x0222: _wait();_mread(cpu->sp++); break;
            case 0x0223: cpu->wzl=_gd(); break;
            // -- M4
            case 0x0224: _wait();_mread(cpu->sp++); break;
            case 0x0225: cpu->wzh=_gd();cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x0226: _fetch(); break;
            
            // 0xF1: pop af (M:3 T:10)
            // -- M2
            case 0x0227: _wait();_mread(cpu->sp++); break;
            case 0x0228: cpu->f=_gd(); break;
            // -- M3
            case 0x0229: _wait();_mread(cpu->sp++); break;
            case 0x022A: cpu->a=_gd(); break;
            // -- OVERLAP
            case 0x022B: _fetch(); break;
            
            // 0xF2: jp p,nn (M:3 T:10)
            // -- M2
            case 0x022C: _wait();_mread(cpu->pc++); break;
            case 0x022D: cpu->wzl=_gd(); break;
            // -- M3
            case 0x022E: _wait();_mread(cpu->pc++); break;
            case 0x022F: cpu->wzh=_gd();if(_cc_p){cpu->pc=cpu->wz;}; break;
            // -- OVERLAP
            case 0x0230: _fetch(); break;
            
            // 0xF3: di (M:1 T:4)
            // -- OVERLAP
            case 0x0231: _fetch(); break;
            
            // 0xF4: call p,nn (M:6 T:17)
            // -- M2
            case 0x0232: _wait();_mread(cpu->pc++); break;
            case 0x0233: cpu->wzl=_gd(); break;
            // -- M3
            case 0x0234: _wait();_mread(cpu->pc++); break;
            case 0x0235: cpu->wzh=_gd();if (!_cc_p){z80_skip(cpu,5,9,2);}; break;
            // -- M4 (generic)
            case 0x0236:  break;
            // -- M5
            case 0x0237: _mwrite(--cpu->sp,cpu->pch); break;
            case 0x0238: _wait() break;
            // -- M6
            case 0x0239: _mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz; break;
            case 0x023A: _wait() break;
            // -- OVERLAP
            case 0x023B: _fetch(); break;
            
            // 0xF5: push af (M:3 T:11)
            // -- M2
            case 0x023C: _mwrite(--cpu->sp,cpu->a); break;
            case 0x023D: _wait() break;
            // -- M3
            case 0x023E: _mwrite(--cpu->sp,cpu->f); break;
            case 0x023F: _wait() break;
            // -- OVERLAP
            case 0x0240: _fetch(); break;
            
            // 0xF6: or n (M:2 T:7)
            // -- M2
            case 0x0241: _wait();_mread(cpu->pc++); break;
            case 0x0242: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x0243: z80_or8(cpu,cpu->dlatch);_fetch(); break;
            
            // 0xF7: rst 30h (M:1 T:4)
            // -- OVERLAP
            case 0x0244: _fetch(); break;
            
            // 0xF8: ret m (M:4 T:11)
            // -- M2 (generic)
            case 0x0245: if(!_cc_m){z80_skip(cpu,4,7,1);}; break;
            // -- M3
            case 0x0246: _wait();_mread(cpu->sp++); break;
            case 0x0247: cpu->wzl=_gd(); break;
            // -- M4
            case 0x0248: _wait();_mread(cpu->sp++); break;
            case 0x0249: cpu->wzh=_gd();cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x024A: _fetch(); break;
            
            // 0xF9: ld sp,hl (M:2 T:6)
            // -- M2 (generic)
            case 0x024B: cpu->sp=cpu->hlx[cpu->hlx_idx].hl; break;
            // -- OVERLAP
            case 0x024C: _fetch(); break;
            
            // 0xFA: jp m,nn (M:3 T:10)
            // -- M2
            case 0x024D: _wait();_mread(cpu->pc++); break;
            case 0x024E: cpu->wzl=_gd(); break;
            // -- M3
            case 0x024F: _wait();_mread(cpu->pc++); break;
            case 0x0250: cpu->wzh=_gd();if(_cc_m){cpu->pc=cpu->wz;}; break;
            // -- OVERLAP
            case 0x0251: _fetch(); break;
            
            // 0xFB: ei (M:1 T:4)
            // -- OVERLAP
            case 0x0252: _fetch(); break;
            
            // 0xFC: call m,nn (M:6 T:17)
            // -- M2
            case 0x0253: _wait();_mread(cpu->pc++); break;
            case 0x0254: cpu->wzl=_gd(); break;
            // -- M3
            case 0x0255: _wait();_mread(cpu->pc++); break;
            case 0x0256: cpu->wzh=_gd();if (!_cc_m){z80_skip(cpu,5,9,2);}; break;
            // -- M4 (generic)
            case 0x0257:  break;
            // -- M5
            case 0x0258: _mwrite(--cpu->sp,cpu->pch); break;
            case 0x0259: _wait() break;
            // -- M6
            case 0x025A: _mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz; break;
            case 0x025B: _wait() break;
            // -- OVERLAP
            case 0x025C: _fetch(); break;
            
            // 0xFD: fd prefix (M:1 T:4)
            // -- OVERLAP
            case 0x025D: _fetch_iy(); break;
            
            // 0xFE: cp n (M:2 T:7)
            // -- M2
            case 0x025E: _wait();_mread(cpu->pc++); break;
            case 0x025F: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x0260: z80_cp8(cpu,cpu->dlatch);_fetch(); break;
            
            // 0xFF: rst 38h (M:1 T:4)
            // -- OVERLAP
            case 0x0261: _fetch(); break;

        }
        cpu->op.step += 1;
    }
    // advance the decode pipeline by one tcycle
    cpu->op.pip >>= 1;
    return pins;
}

#undef _sa
#undef _sax
#undef _sad
#undef _sadx
#undef _gd
#undef _fetch
#undef _fetch_ix
#undef _fetch_iy
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
