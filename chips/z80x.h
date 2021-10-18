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

// flags for z80_opstate.flags
#define Z80_OPSTATE_FLAGS_INDIRECT  (1<<0)  // this is a (HL)/(IX+d)/(IY+d) instruction
#define Z80_OPSTATE_FLAGS_IMM8 (1<<1)       // this is an 8-bit immediate load instruction

// values for hlx_idx for mapping HL, IX or IY
#define Z80_MAP_HL (0)
#define Z80_MAP_IX (1)
#define Z80_MAP_IY (2)

typedef struct {
    uint64_t pip;   // the op's decode pipeline
    uint32_t step;  // first or current decoder switch-case branch step
    uint32_t flags; // Z80_OPSTATE_FLAGS_
} z80_opstate_t;

// CPU state
typedef struct {
    uint64_t pins;      // last stored pin state
    z80_opstate_t op;   // the currently active op
    uint16_t pc;        // program counter
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
    cpu->op.pip = (1<<31)|5;
    return Z80_M1|Z80_MREQ|Z80_RD;
}

bool z80_opdone(z80_t* cpu) {
    // because of the overlapped cycle, the result of the previous
    // instruction is only available in M1/T2
    return (0 == cpu->op.step) && (cpu->hlx_idx == 0);
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
    // reset the decoder to continue at case 0
    cpu->op.pip = (1ULL<<32)|(5ULL<<1);
    cpu->op.step = 0;
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
    { 0x0000000000000002, 0x0005, 0 },
    // 0x01: ld bc,nn (M:3 T:10 steps:5)
    { 0x00000024000000B6, 0x0006, 0 },
    // 0x02: ld (bc),a (M:2 T:7 steps:2)
    { 0x0000000400000014, 0x000B, 0 },
    // 0x03: inc bc (M:2 T:6 steps:2)
    { 0x000000000000000A, 0x000D, 0 },
    // 0x04: inc b (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x000F, 0 },
    // 0x05: dec b (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0010, 0 },
    // 0x06: ld b,n (M:2 T:7 steps:3)
    { 0x0000000400000016, 0x0011, Z80_OPSTATE_FLAGS_IMM8 },
    // 0x07: rlca (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0014, 0 },
    // 0x08: ex af,af' (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0015, 0 },
    // 0x09: add hl,bc (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0016, 0 },
    // 0x0A: ld a,(bc) (M:2 T:7 steps:3)
    { 0x0000000400000016, 0x0017, 0 },
    // 0x0B: dec bc (M:2 T:6 steps:2)
    { 0x000000000000000A, 0x001A, 0 },
    // 0x0C: inc c (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x001C, 0 },
    // 0x0D: dec c (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x001D, 0 },
    // 0x0E: ld c,n (M:2 T:7 steps:3)
    { 0x0000000400000016, 0x001E, Z80_OPSTATE_FLAGS_IMM8 },
    // 0x0F: rrca (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0021, 0 },
    // 0x10: djnz d (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0022, 0 },
    // 0x11: ld de,nn (M:3 T:10 steps:5)
    { 0x00000024000000B6, 0x0023, 0 },
    // 0x12: ld (de),a (M:2 T:7 steps:2)
    { 0x0000000400000014, 0x0028, 0 },
    // 0x13: inc de (M:2 T:6 steps:2)
    { 0x000000000000000A, 0x002A, 0 },
    // 0x14: inc d (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x002C, 0 },
    // 0x15: dec d (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x002D, 0 },
    // 0x16: ld d,n (M:2 T:7 steps:3)
    { 0x0000000400000016, 0x002E, Z80_OPSTATE_FLAGS_IMM8 },
    // 0x17: rla (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0031, 0 },
    // 0x18: jr d (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0032, 0 },
    // 0x19: add hl,de (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0033, 0 },
    // 0x1A: ld a,(de) (M:2 T:7 steps:3)
    { 0x0000000400000016, 0x0034, 0 },
    // 0x1B: dec de (M:2 T:6 steps:2)
    { 0x000000000000000A, 0x0037, 0 },
    // 0x1C: inc e (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0039, 0 },
    // 0x1D: dec e (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x003A, 0 },
    // 0x1E: ld e,n (M:2 T:7 steps:3)
    { 0x0000000400000016, 0x003B, Z80_OPSTATE_FLAGS_IMM8 },
    // 0x1F: rra (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x003E, 0 },
    // 0x20: jr nz,d (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x003F, 0 },
    // 0x21: ld hl,nn (M:3 T:10 steps:5)
    { 0x00000024000000B6, 0x0040, 0 },
    // 0x22: ld (nn),hl (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0045, 0 },
    // 0x23: inc hl (M:2 T:6 steps:2)
    { 0x000000000000000A, 0x0046, 0 },
    // 0x24: inc h (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0048, 0 },
    // 0x25: dec h (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0049, 0 },
    // 0x26: ld h,n (M:2 T:7 steps:3)
    { 0x0000000400000016, 0x004A, Z80_OPSTATE_FLAGS_IMM8 },
    // 0x27: daa (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x004D, 0 },
    // 0x28: jr z,d (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x004E, 0 },
    // 0x29: add hl,hl (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x004F, 0 },
    // 0x2A: ld hl,(nn) (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0050, 0 },
    // 0x2B: dec hl (M:2 T:6 steps:2)
    { 0x000000000000000A, 0x0051, 0 },
    // 0x2C: inc l (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0053, 0 },
    // 0x2D: dec l (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0054, 0 },
    // 0x2E: ld l,n (M:2 T:7 steps:3)
    { 0x0000000400000016, 0x0055, Z80_OPSTATE_FLAGS_IMM8 },
    // 0x2F: cpl (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0058, 0 },
    // 0x30: jr nc,d (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0059, 0 },
    // 0x31: ld sp,nn (M:3 T:10 steps:5)
    { 0x00000024000000B6, 0x005A, 0 },
    // 0x32: ld (nn),a (M:4 T:13 steps:6)
    { 0x0000012400000536, 0x005F, 0 },
    // 0x33: inc sp (M:2 T:6 steps:2)
    { 0x000000000000000A, 0x0065, 0 },
    // 0x34: inc (hl) (M:3 T:11 steps:4)
    { 0x0000004400000146, 0x0067, Z80_OPSTATE_FLAGS_INDIRECT },
    // 0x35: dec (hl) (M:3 T:11 steps:4)
    { 0x0000004400000146, 0x006B, Z80_OPSTATE_FLAGS_INDIRECT },
    // 0x36: ld (hl),n (M:3 T:10 steps:4)
    { 0x00000024000000A6, 0x006F, Z80_OPSTATE_FLAGS_INDIRECT|Z80_OPSTATE_FLAGS_IMM8 },
    // 0x37: scf (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0073, 0 },
    // 0x38: jr c,d (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0074, 0 },
    // 0x39: add hl,sp (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0075, 0 },
    // 0x3A: ld a,(nn) (M:4 T:13 steps:7)
    { 0x00000124000005B6, 0x0076, 0 },
    // 0x3B: dec sp (M:2 T:6 steps:2)
    { 0x000000000000000A, 0x007D, 0 },
    // 0x3C: inc a (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x007F, 0 },
    // 0x3D: dec a (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0080, 0 },
    // 0x3E: ld a,n (M:2 T:7 steps:3)
    { 0x0000000400000016, 0x0081, Z80_OPSTATE_FLAGS_IMM8 },
    // 0x3F: ccf (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0084, 0 },
    // 0x40: ld b,b (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0085, 0 },
    // 0x41: ld b,c (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0086, 0 },
    // 0x42: ld b,d (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0087, 0 },
    // 0x43: ld b,e (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0088, 0 },
    // 0x44: ld b,h (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0089, 0 },
    // 0x45: ld b,l (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x008A, 0 },
    // 0x46: ld b,(hl) (M:2 T:7 steps:3)
    { 0x0000000400000016, 0x008B, Z80_OPSTATE_FLAGS_INDIRECT },
    // 0x47: ld b,a (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x008E, 0 },
    // 0x48: ld c,b (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x008F, 0 },
    // 0x49: ld c,c (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0090, 0 },
    // 0x4A: ld c,d (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0091, 0 },
    // 0x4B: ld c,e (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0092, 0 },
    // 0x4C: ld c,h (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0093, 0 },
    // 0x4D: ld c,l (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0094, 0 },
    // 0x4E: ld c,(hl) (M:2 T:7 steps:3)
    { 0x0000000400000016, 0x0095, Z80_OPSTATE_FLAGS_INDIRECT },
    // 0x4F: ld c,a (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0098, 0 },
    // 0x50: ld d,b (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0099, 0 },
    // 0x51: ld d,c (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x009A, 0 },
    // 0x52: ld d,d (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x009B, 0 },
    // 0x53: ld d,e (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x009C, 0 },
    // 0x54: ld d,h (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x009D, 0 },
    // 0x55: ld d,l (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x009E, 0 },
    // 0x56: ld d,(hl) (M:2 T:7 steps:3)
    { 0x0000000400000016, 0x009F, Z80_OPSTATE_FLAGS_INDIRECT },
    // 0x57: ld d,a (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00A2, 0 },
    // 0x58: ld e,b (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00A3, 0 },
    // 0x59: ld e,c (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00A4, 0 },
    // 0x5A: ld e,d (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00A5, 0 },
    // 0x5B: ld e,e (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00A6, 0 },
    // 0x5C: ld e,h (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00A7, 0 },
    // 0x5D: ld e,l (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00A8, 0 },
    // 0x5E: ld e,(hl) (M:2 T:7 steps:3)
    { 0x0000000400000016, 0x00A9, Z80_OPSTATE_FLAGS_INDIRECT },
    // 0x5F: ld e,a (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00AC, 0 },
    // 0x60: ld h,b (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00AD, 0 },
    // 0x61: ld h,c (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00AE, 0 },
    // 0x62: ld h,d (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00AF, 0 },
    // 0x63: ld h,e (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00B0, 0 },
    // 0x64: ld h,h (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00B1, 0 },
    // 0x65: ld h,l (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00B2, 0 },
    // 0x66: ld h,(hl) (M:2 T:7 steps:3)
    { 0x0000000400000016, 0x00B3, Z80_OPSTATE_FLAGS_INDIRECT },
    // 0x67: ld h,a (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00B6, 0 },
    // 0x68: ld l,b (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00B7, 0 },
    // 0x69: ld l,c (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00B8, 0 },
    // 0x6A: ld l,d (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00B9, 0 },
    // 0x6B: ld l,e (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00BA, 0 },
    // 0x6C: ld l,h (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00BB, 0 },
    // 0x6D: ld l,l (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00BC, 0 },
    // 0x6E: ld l,(hl) (M:2 T:7 steps:3)
    { 0x0000000400000016, 0x00BD, Z80_OPSTATE_FLAGS_INDIRECT },
    // 0x6F: ld l,a (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00C0, 0 },
    // 0x70: ld (hl),b (M:2 T:7 steps:2)
    { 0x0000000400000014, 0x00C1, Z80_OPSTATE_FLAGS_INDIRECT },
    // 0x71: ld (hl),c (M:2 T:7 steps:2)
    { 0x0000000400000014, 0x00C3, Z80_OPSTATE_FLAGS_INDIRECT },
    // 0x72: ld (hl),d (M:2 T:7 steps:2)
    { 0x0000000400000014, 0x00C5, Z80_OPSTATE_FLAGS_INDIRECT },
    // 0x73: ld (hl),e (M:2 T:7 steps:2)
    { 0x0000000400000014, 0x00C7, Z80_OPSTATE_FLAGS_INDIRECT },
    // 0x74: ld (hl),h (M:2 T:7 steps:2)
    { 0x0000000400000014, 0x00C9, Z80_OPSTATE_FLAGS_INDIRECT },
    // 0x75: ld (hl),l (M:2 T:7 steps:2)
    { 0x0000000400000014, 0x00CB, Z80_OPSTATE_FLAGS_INDIRECT },
    // 0x76: halt (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00CD, 0 },
    // 0x77: ld (hl),a (M:2 T:7 steps:2)
    { 0x0000000400000014, 0x00CE, Z80_OPSTATE_FLAGS_INDIRECT },
    // 0x78: ld a,b (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00D0, 0 },
    // 0x79: ld a,c (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00D1, 0 },
    // 0x7A: ld a,d (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00D2, 0 },
    // 0x7B: ld a,e (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00D3, 0 },
    // 0x7C: ld a,h (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00D4, 0 },
    // 0x7D: ld a,l (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00D5, 0 },
    // 0x7E: ld a,(hl) (M:2 T:7 steps:3)
    { 0x0000000400000016, 0x00D6, Z80_OPSTATE_FLAGS_INDIRECT },
    // 0x7F: ld a,a (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00D9, 0 },
    // 0x80: add b (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00DA, 0 },
    // 0x81: add c (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00DB, 0 },
    // 0x82: add d (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00DC, 0 },
    // 0x83: add e (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00DD, 0 },
    // 0x84: add h (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00DE, 0 },
    // 0x85: add l (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00DF, 0 },
    // 0x86: add (hl) (M:2 T:7 steps:3)
    { 0x0000000400000016, 0x00E0, Z80_OPSTATE_FLAGS_INDIRECT },
    // 0x87: add a (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00E3, 0 },
    // 0x88: adc b (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00E4, 0 },
    // 0x89: adc c (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00E5, 0 },
    // 0x8A: adc d (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00E6, 0 },
    // 0x8B: adc e (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00E7, 0 },
    // 0x8C: adc h (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00E8, 0 },
    // 0x8D: adc l (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00E9, 0 },
    // 0x8E: adc (hl) (M:2 T:7 steps:3)
    { 0x0000000400000016, 0x00EA, Z80_OPSTATE_FLAGS_INDIRECT },
    // 0x8F: adc a (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00ED, 0 },
    // 0x90: sub b (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00EE, 0 },
    // 0x91: sub c (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00EF, 0 },
    // 0x92: sub d (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00F0, 0 },
    // 0x93: sub e (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00F1, 0 },
    // 0x94: sub h (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00F2, 0 },
    // 0x95: sub l (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00F3, 0 },
    // 0x96: sub (hl) (M:2 T:7 steps:3)
    { 0x0000000400000016, 0x00F4, Z80_OPSTATE_FLAGS_INDIRECT },
    // 0x97: sub a (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00F7, 0 },
    // 0x98: sbc b (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00F8, 0 },
    // 0x99: sbc c (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00F9, 0 },
    // 0x9A: sbc d (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00FA, 0 },
    // 0x9B: sbc e (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00FB, 0 },
    // 0x9C: sbc h (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00FC, 0 },
    // 0x9D: sbc l (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x00FD, 0 },
    // 0x9E: sbc (hl) (M:2 T:7 steps:3)
    { 0x0000000400000016, 0x00FE, Z80_OPSTATE_FLAGS_INDIRECT },
    // 0x9F: sbc a (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0101, 0 },
    // 0xA0: and b (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0102, 0 },
    // 0xA1: and c (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0103, 0 },
    // 0xA2: and d (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0104, 0 },
    // 0xA3: and e (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0105, 0 },
    // 0xA4: and h (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0106, 0 },
    // 0xA5: and l (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0107, 0 },
    // 0xA6: and (hl) (M:2 T:7 steps:3)
    { 0x0000000400000016, 0x0108, Z80_OPSTATE_FLAGS_INDIRECT },
    // 0xA7: and a (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x010B, 0 },
    // 0xA8: xor b (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x010C, 0 },
    // 0xA9: xor c (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x010D, 0 },
    // 0xAA: xor d (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x010E, 0 },
    // 0xAB: xor e (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x010F, 0 },
    // 0xAC: xor h (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0110, 0 },
    // 0xAD: xor l (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0111, 0 },
    // 0xAE: xor (hl) (M:2 T:7 steps:3)
    { 0x0000000400000016, 0x0112, Z80_OPSTATE_FLAGS_INDIRECT },
    // 0xAF: xor a (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0115, 0 },
    // 0xB0: or b (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0116, 0 },
    // 0xB1: or c (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0117, 0 },
    // 0xB2: or d (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0118, 0 },
    // 0xB3: or e (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0119, 0 },
    // 0xB4: or h (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x011A, 0 },
    // 0xB5: or l (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x011B, 0 },
    // 0xB6: or (hl) (M:2 T:7 steps:3)
    { 0x0000000400000016, 0x011C, Z80_OPSTATE_FLAGS_INDIRECT },
    // 0xB7: or a (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x011F, 0 },
    // 0xB8: cp b (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0120, 0 },
    // 0xB9: cp c (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0121, 0 },
    // 0xBA: cp d (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0122, 0 },
    // 0xBB: cp e (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0123, 0 },
    // 0xBC: cp h (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0124, 0 },
    // 0xBD: cp l (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0125, 0 },
    // 0xBE: cp (hl) (M:2 T:7 steps:3)
    { 0x0000000400000016, 0x0126, Z80_OPSTATE_FLAGS_INDIRECT },
    // 0xBF: cp a (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0129, 0 },
    // 0xC0: ret nz (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x012A, 0 },
    // 0xC1: pop bc (M:3 T:10 steps:5)
    { 0x00000024000000B6, 0x012B, 0 },
    // 0xC2: jp nz,nn (M:3 T:10 steps:5)
    { 0x00000024000000B6, 0x0130, 0 },
    // 0xC3: jp nn (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0135, 0 },
    // 0xC4: call nz,nn (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0136, 0 },
    // 0xC5: push bc (M:3 T:11 steps:3)
    { 0x0000004800000148, 0x0137, 0 },
    // 0xC6: add n (M:2 T:7 steps:3)
    { 0x0000000400000016, 0x013A, Z80_OPSTATE_FLAGS_IMM8 },
    // 0xC7: rst 0h (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x013D, 0 },
    // 0xC8: ret z (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x013E, 0 },
    // 0xC9: ret (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x013F, 0 },
    // 0xCA: jp z,nn (M:3 T:10 steps:5)
    { 0x00000024000000B6, 0x0140, 0 },
    // 0xCB: cb prefix (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0145, 0 },
    // 0xCC: call z,nn (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0146, 0 },
    // 0xCD: call nn (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0147, 0 },
    // 0xCE: adc n (M:2 T:7 steps:3)
    { 0x0000000400000016, 0x0148, Z80_OPSTATE_FLAGS_IMM8 },
    // 0xCF: rst 8h (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x014B, 0 },
    // 0xD0: ret nc (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x014C, 0 },
    // 0xD1: pop de (M:3 T:10 steps:5)
    { 0x00000024000000B6, 0x014D, 0 },
    // 0xD2: jp nc,nn (M:3 T:10 steps:5)
    { 0x00000024000000B6, 0x0152, 0 },
    // 0xD3: out (n),a (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0157, 0 },
    // 0xD4: call nc,nn (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0158, 0 },
    // 0xD5: push de (M:3 T:11 steps:3)
    { 0x0000004800000148, 0x0159, 0 },
    // 0xD6: sub n (M:2 T:7 steps:3)
    { 0x0000000400000016, 0x015C, Z80_OPSTATE_FLAGS_IMM8 },
    // 0xD7: rst 10h (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x015F, 0 },
    // 0xD8: ret c (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0160, 0 },
    // 0xD9: exx (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0161, 0 },
    // 0xDA: jp c,nn (M:3 T:10 steps:5)
    { 0x00000024000000B6, 0x0162, 0 },
    // 0xDB: in a,(n) (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0167, 0 },
    // 0xDC: call c,nn (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0168, 0 },
    // 0xDD: dd prefix (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0169, 0 },
    // 0xDE: sbc n (M:2 T:7 steps:3)
    { 0x0000000400000016, 0x016A, Z80_OPSTATE_FLAGS_IMM8 },
    // 0xDF: rst 18h (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x016D, 0 },
    // 0xE0: ret po (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x016E, 0 },
    // 0xE1: pop hl (M:3 T:10 steps:5)
    { 0x00000024000000B6, 0x016F, 0 },
    // 0xE2: jp po,nn (M:3 T:10 steps:5)
    { 0x00000024000000B6, 0x0174, 0 },
    // 0xE3: ex (sp),hl (M:5 T:19 steps:7)
    { 0x0000122400011236, 0x0179, 0 },
    // 0xE4: call po,nn (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0180, 0 },
    // 0xE5: push hl (M:3 T:11 steps:3)
    { 0x0000004800000148, 0x0181, 0 },
    // 0xE6: and n (M:2 T:7 steps:3)
    { 0x0000000400000016, 0x0184, Z80_OPSTATE_FLAGS_IMM8 },
    // 0xE7: rst 20h (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0187, 0 },
    // 0xE8: ret pe (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0188, 0 },
    // 0xE9: jp hl (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0189, 0 },
    // 0xEA: jp pe,nn (M:3 T:10 steps:5)
    { 0x00000024000000B6, 0x018A, 0 },
    // 0xEB: ex de,hl (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x018F, 0 },
    // 0xEC: call pe,nn (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0190, 0 },
    // 0xED: ed prefix (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0191, 0 },
    // 0xEE: xor n (M:2 T:7 steps:3)
    { 0x0000000400000016, 0x0192, Z80_OPSTATE_FLAGS_IMM8 },
    // 0xEF: rst 28h (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0195, 0 },
    // 0xF0: ret p (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x0196, 0 },
    // 0xF1: pop af (M:3 T:10 steps:5)
    { 0x00000024000000B6, 0x0197, 0 },
    // 0xF2: jp p,nn (M:3 T:10 steps:5)
    { 0x00000024000000B6, 0x019C, 0 },
    // 0xF3: di (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x01A1, 0 },
    // 0xF4: call p,nn (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x01A2, 0 },
    // 0xF5: push af (M:3 T:11 steps:3)
    { 0x0000004800000148, 0x01A3, 0 },
    // 0xF6: or n (M:2 T:7 steps:3)
    { 0x0000000400000016, 0x01A6, Z80_OPSTATE_FLAGS_IMM8 },
    // 0xF7: rst 30h (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x01A9, 0 },
    // 0xF8: ret m (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x01AA, 0 },
    // 0xF9: ld sp,hl (M:2 T:6 steps:2)
    { 0x000000000000000A, 0x01AB, 0 },
    // 0xFA: jp m,nn (M:3 T:10 steps:5)
    { 0x00000024000000B6, 0x01AD, 0 },
    // 0xFB: ei (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x01B2, 0 },
    // 0xFC: call m,nn (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x01B3, 0 },
    // 0xFD: fd prefix (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x01B4, 0 },
    // 0xFE: cp n (M:2 T:7 steps:3)
    { 0x0000000400000016, 0x01B5, Z80_OPSTATE_FLAGS_IMM8 },
    // 0xFF: rst 38h (M:1 T:4 steps:1)
    { 0x0000000000000002, 0x01B8, 0 },

};

uint64_t z80_prefetch(z80_t* cpu, uint16_t new_pc) {
    cpu->pc = new_pc;
    cpu->op.pip = 1;
    // overlapped M1:T1 of the NOP instruction to initiate opcode fetch at new pc
    cpu->op.step = z80_opstate_table[0].step;
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
#define _cc_nz          (!(cpu->f&Z80_ZF))
#define _cc_z           (cpu->f&Z80_ZF)
#define _cc_nc          (!(cpu->f&Z80_CF))
#define _cc_c           (cpu->f&Z80_CF)
#define _cc_po          (!(cpu->f&Z80_PF))
#define _cc_pe          (cpu->f&Z80_PF)
#define _cc_p           (!(cpu->f&Z80_SF))
#define _cc_m           (cpu->f&Z80_SF)

uint64_t z80_tick(z80_t* cpu, uint64_t pins) {
    // wait cycle? (wait pin sampling only happens in specific tcycles)
    if ((cpu->op.pip & Z80_PIP_BIT_WAIT) && (pins & Z80_WAIT)) {
        cpu->pins = pins & ~Z80_CTRL_PIN_MASK;
        return pins;
    }
    // process the next active tcycle
    pins &= ~Z80_CTRL_PIN_MASK;
    if (cpu->op.pip & Z80_PIP_BIT_STEP) {
        switch (cpu->op.step++) {
            // shared fetch machine cycle for all opcodes
            case 0: {
                cpu->ir = _gd();
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
                        cpu->op.pip = (2ULL<<33)|(3ULL<<1);
                        // special case: if this is indirect+immediate (which is
                        // just LD (HL),n, then the immediate-load is 'hidden' within
                        // the 8-tcycle d-offset computation)
                        if (cpu->op.flags & Z80_OPSTATE_FLAGS_IMM8) {
                            cpu->op.pip |= 1ULL<<3;
                        }
                        else {
                            cpu->op.pip |= 1ULL<<8;
                        }
                        cpu->op.step = 2;
                    }
                }
            } break;
            //=== optional d-loading cycle for (HL), (IX+d), (IY+d)
            case 2: {
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
                    const uint64_t mask = 0x0000000F0000000F;
                    cpu->op.pip = (cpu->op.pip & mask) | ((cpu->op.pip & ~mask)<<2);
                }
            } break;
            // FIXME: optional interrupt handling(?) 
            
            // 0x00: nop (M:1 T:4)
            // -- OVERLAP
            case 0x0005: _fetch(); break;
            
            // 0x01: ld bc,nn (M:3 T:10)
            // -- M2
            case 0x0006: _mread(cpu->pc++); break;
            case 0x0007: cpu->c=_gd(); break;
            // -- M3
            case 0x0008: _mread(cpu->pc++); break;
            case 0x0009: cpu->b=_gd(); break;
            // -- OVERLAP
            case 0x000A: _fetch(); break;
            
            // 0x02: ld (bc),a (M:2 T:7)
            // -- M2
            case 0x000B: _mwrite(cpu->bc,cpu->a);cpu->wzl=cpu->c+1;cpu->wzh=cpu->a; break;
            // -- OVERLAP
            case 0x000C: _fetch(); break;
            
            // 0x03: inc bc (M:2 T:6)
            // -- M2 (generic)
            case 0x000D: cpu->bc++; break;
            // -- OVERLAP
            case 0x000E: _fetch(); break;
            
            // 0x04: inc b (M:1 T:4)
            // -- OVERLAP
            case 0x000F: cpu->b=z80_inc8(cpu,cpu->b);_fetch(); break;
            
            // 0x05: dec b (M:1 T:4)
            // -- OVERLAP
            case 0x0010: cpu->b=z80_dec8(cpu,cpu->b);_fetch(); break;
            
            // 0x06: ld b,n (M:2 T:7)
            // -- M2
            case 0x0011: _mread(cpu->pc++); break;
            case 0x0012: cpu->b=_gd(); break;
            // -- OVERLAP
            case 0x0013: _fetch(); break;
            
            // 0x07: rlca (M:1 T:4)
            // -- OVERLAP
            case 0x0014: z80_rlca(cpu);_fetch(); break;
            
            // 0x08: ex af,af' (M:1 T:4)
            // -- OVERLAP
            case 0x0015: z80_ex_af_af2(cpu);_fetch(); break;
            
            // 0x09: add hl,bc (M:1 T:4)
            // -- OVERLAP
            case 0x0016: _fetch(); break;
            
            // 0x0A: ld a,(bc) (M:2 T:7)
            // -- M2
            case 0x0017: _mread(cpu->bc); break;
            case 0x0018: cpu->a=_gd();cpu->wz=cpu->bc+1; break;
            // -- OVERLAP
            case 0x0019: _fetch(); break;
            
            // 0x0B: dec bc (M:2 T:6)
            // -- M2 (generic)
            case 0x001A: cpu->bc--; break;
            // -- OVERLAP
            case 0x001B: _fetch(); break;
            
            // 0x0C: inc c (M:1 T:4)
            // -- OVERLAP
            case 0x001C: cpu->c=z80_inc8(cpu,cpu->c);_fetch(); break;
            
            // 0x0D: dec c (M:1 T:4)
            // -- OVERLAP
            case 0x001D: cpu->c=z80_dec8(cpu,cpu->c);_fetch(); break;
            
            // 0x0E: ld c,n (M:2 T:7)
            // -- M2
            case 0x001E: _mread(cpu->pc++); break;
            case 0x001F: cpu->c=_gd(); break;
            // -- OVERLAP
            case 0x0020: _fetch(); break;
            
            // 0x0F: rrca (M:1 T:4)
            // -- OVERLAP
            case 0x0021: z80_rrca(cpu);_fetch(); break;
            
            // 0x10: djnz d (M:1 T:4)
            // -- OVERLAP
            case 0x0022: _fetch(); break;
            
            // 0x11: ld de,nn (M:3 T:10)
            // -- M2
            case 0x0023: _mread(cpu->pc++); break;
            case 0x0024: cpu->e=_gd(); break;
            // -- M3
            case 0x0025: _mread(cpu->pc++); break;
            case 0x0026: cpu->d=_gd(); break;
            // -- OVERLAP
            case 0x0027: _fetch(); break;
            
            // 0x12: ld (de),a (M:2 T:7)
            // -- M2
            case 0x0028: _mwrite(cpu->de,cpu->a);cpu->wzl=cpu->e+1;cpu->wzh=cpu->a; break;
            // -- OVERLAP
            case 0x0029: _fetch(); break;
            
            // 0x13: inc de (M:2 T:6)
            // -- M2 (generic)
            case 0x002A: cpu->de++; break;
            // -- OVERLAP
            case 0x002B: _fetch(); break;
            
            // 0x14: inc d (M:1 T:4)
            // -- OVERLAP
            case 0x002C: cpu->d=z80_inc8(cpu,cpu->d);_fetch(); break;
            
            // 0x15: dec d (M:1 T:4)
            // -- OVERLAP
            case 0x002D: cpu->d=z80_dec8(cpu,cpu->d);_fetch(); break;
            
            // 0x16: ld d,n (M:2 T:7)
            // -- M2
            case 0x002E: _mread(cpu->pc++); break;
            case 0x002F: cpu->d=_gd(); break;
            // -- OVERLAP
            case 0x0030: _fetch(); break;
            
            // 0x17: rla (M:1 T:4)
            // -- OVERLAP
            case 0x0031: z80_rla(cpu);_fetch(); break;
            
            // 0x18: jr d (M:1 T:4)
            // -- OVERLAP
            case 0x0032: _fetch(); break;
            
            // 0x19: add hl,de (M:1 T:4)
            // -- OVERLAP
            case 0x0033: _fetch(); break;
            
            // 0x1A: ld a,(de) (M:2 T:7)
            // -- M2
            case 0x0034: _mread(cpu->de); break;
            case 0x0035: cpu->a=_gd();cpu->wz=cpu->de+1; break;
            // -- OVERLAP
            case 0x0036: _fetch(); break;
            
            // 0x1B: dec de (M:2 T:6)
            // -- M2 (generic)
            case 0x0037: cpu->de--; break;
            // -- OVERLAP
            case 0x0038: _fetch(); break;
            
            // 0x1C: inc e (M:1 T:4)
            // -- OVERLAP
            case 0x0039: cpu->e=z80_inc8(cpu,cpu->e);_fetch(); break;
            
            // 0x1D: dec e (M:1 T:4)
            // -- OVERLAP
            case 0x003A: cpu->e=z80_dec8(cpu,cpu->e);_fetch(); break;
            
            // 0x1E: ld e,n (M:2 T:7)
            // -- M2
            case 0x003B: _mread(cpu->pc++); break;
            case 0x003C: cpu->e=_gd(); break;
            // -- OVERLAP
            case 0x003D: _fetch(); break;
            
            // 0x1F: rra (M:1 T:4)
            // -- OVERLAP
            case 0x003E: z80_rra(cpu);_fetch(); break;
            
            // 0x20: jr nz,d (M:1 T:4)
            // -- OVERLAP
            case 0x003F: _fetch(); break;
            
            // 0x21: ld hl,nn (M:3 T:10)
            // -- M2
            case 0x0040: _mread(cpu->pc++); break;
            case 0x0041: cpu->hlx[cpu->hlx_idx].l=_gd(); break;
            // -- M3
            case 0x0042: _mread(cpu->pc++); break;
            case 0x0043: cpu->hlx[cpu->hlx_idx].h=_gd(); break;
            // -- OVERLAP
            case 0x0044: _fetch(); break;
            
            // 0x22: ld (nn),hl (M:1 T:4)
            // -- OVERLAP
            case 0x0045: _fetch(); break;
            
            // 0x23: inc hl (M:2 T:6)
            // -- M2 (generic)
            case 0x0046: cpu->hlx[cpu->hlx_idx].hl++; break;
            // -- OVERLAP
            case 0x0047: _fetch(); break;
            
            // 0x24: inc h (M:1 T:4)
            // -- OVERLAP
            case 0x0048: cpu->hlx[cpu->hlx_idx].h=z80_inc8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); break;
            
            // 0x25: dec h (M:1 T:4)
            // -- OVERLAP
            case 0x0049: cpu->hlx[cpu->hlx_idx].h=z80_dec8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); break;
            
            // 0x26: ld h,n (M:2 T:7)
            // -- M2
            case 0x004A: _mread(cpu->pc++); break;
            case 0x004B: cpu->hlx[cpu->hlx_idx].h=_gd(); break;
            // -- OVERLAP
            case 0x004C: _fetch(); break;
            
            // 0x27: daa (M:1 T:4)
            // -- OVERLAP
            case 0x004D: z80_daa(cpu);_fetch(); break;
            
            // 0x28: jr z,d (M:1 T:4)
            // -- OVERLAP
            case 0x004E: _fetch(); break;
            
            // 0x29: add hl,hl (M:1 T:4)
            // -- OVERLAP
            case 0x004F: _fetch(); break;
            
            // 0x2A: ld hl,(nn) (M:1 T:4)
            // -- OVERLAP
            case 0x0050: _fetch(); break;
            
            // 0x2B: dec hl (M:2 T:6)
            // -- M2 (generic)
            case 0x0051: cpu->hlx[cpu->hlx_idx].hl--; break;
            // -- OVERLAP
            case 0x0052: _fetch(); break;
            
            // 0x2C: inc l (M:1 T:4)
            // -- OVERLAP
            case 0x0053: cpu->hlx[cpu->hlx_idx].l=z80_inc8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); break;
            
            // 0x2D: dec l (M:1 T:4)
            // -- OVERLAP
            case 0x0054: cpu->hlx[cpu->hlx_idx].l=z80_dec8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); break;
            
            // 0x2E: ld l,n (M:2 T:7)
            // -- M2
            case 0x0055: _mread(cpu->pc++); break;
            case 0x0056: cpu->hlx[cpu->hlx_idx].l=_gd(); break;
            // -- OVERLAP
            case 0x0057: _fetch(); break;
            
            // 0x2F: cpl (M:1 T:4)
            // -- OVERLAP
            case 0x0058: z80_cpl(cpu);_fetch(); break;
            
            // 0x30: jr nc,d (M:1 T:4)
            // -- OVERLAP
            case 0x0059: _fetch(); break;
            
            // 0x31: ld sp,nn (M:3 T:10)
            // -- M2
            case 0x005A: _mread(cpu->pc++); break;
            case 0x005B: cpu->spl=_gd(); break;
            // -- M3
            case 0x005C: _mread(cpu->pc++); break;
            case 0x005D: cpu->sph=_gd(); break;
            // -- OVERLAP
            case 0x005E: _fetch(); break;
            
            // 0x32: ld (nn),a (M:4 T:13)
            // -- M2
            case 0x005F: _mread(cpu->pc++); break;
            case 0x0060: cpu->wzl=_gd(); break;
            // -- M3
            case 0x0061: _mread(cpu->pc++); break;
            case 0x0062: cpu->wzh=_gd(); break;
            // -- M4
            case 0x0063: _mwrite(cpu->wz++,cpu->a);cpu->wzh=cpu->a; break;
            // -- OVERLAP
            case 0x0064: _fetch(); break;
            
            // 0x33: inc sp (M:2 T:6)
            // -- M2 (generic)
            case 0x0065: cpu->sp++; break;
            // -- OVERLAP
            case 0x0066: _fetch(); break;
            
            // 0x34: inc (hl) (M:3 T:11)
            // -- M2
            case 0x0067: _mread(cpu->addr); break;
            case 0x0068: cpu->dlatch=_gd();cpu->dlatch=z80_inc8(cpu,cpu->dlatch); break;
            // -- M3
            case 0x0069: _mwrite(cpu->addr,cpu->dlatch); break;
            // -- OVERLAP
            case 0x006A: _fetch(); break;
            
            // 0x35: dec (hl) (M:3 T:11)
            // -- M2
            case 0x006B: _mread(cpu->addr); break;
            case 0x006C: cpu->dlatch=_gd();cpu->dlatch=z80_dec8(cpu,cpu->dlatch); break;
            // -- M3
            case 0x006D: _mwrite(cpu->addr,cpu->dlatch); break;
            // -- OVERLAP
            case 0x006E: _fetch(); break;
            
            // 0x36: ld (hl),n (M:3 T:10)
            // -- M2
            case 0x006F: _mread(cpu->pc++); break;
            case 0x0070: cpu->dlatch=_gd(); break;
            // -- M3
            case 0x0071: _mwrite(cpu->addr,cpu->dlatch); break;
            // -- OVERLAP
            case 0x0072: _fetch(); break;
            
            // 0x37: scf (M:1 T:4)
            // -- OVERLAP
            case 0x0073: z80_scf(cpu);_fetch(); break;
            
            // 0x38: jr c,d (M:1 T:4)
            // -- OVERLAP
            case 0x0074: _fetch(); break;
            
            // 0x39: add hl,sp (M:1 T:4)
            // -- OVERLAP
            case 0x0075: _fetch(); break;
            
            // 0x3A: ld a,(nn) (M:4 T:13)
            // -- M2
            case 0x0076: _mread(cpu->pc++); break;
            case 0x0077: cpu->wzl=_gd(); break;
            // -- M3
            case 0x0078: _mread(cpu->pc++); break;
            case 0x0079: cpu->wzh=_gd(); break;
            // -- M4
            case 0x007A: _mread(cpu->wz++); break;
            case 0x007B: cpu->a=_gd(); break;
            // -- OVERLAP
            case 0x007C: _fetch(); break;
            
            // 0x3B: dec sp (M:2 T:6)
            // -- M2 (generic)
            case 0x007D: cpu->sp--; break;
            // -- OVERLAP
            case 0x007E: _fetch(); break;
            
            // 0x3C: inc a (M:1 T:4)
            // -- OVERLAP
            case 0x007F: cpu->a=z80_inc8(cpu,cpu->a);_fetch(); break;
            
            // 0x3D: dec a (M:1 T:4)
            // -- OVERLAP
            case 0x0080: cpu->a=z80_dec8(cpu,cpu->a);_fetch(); break;
            
            // 0x3E: ld a,n (M:2 T:7)
            // -- M2
            case 0x0081: _mread(cpu->pc++); break;
            case 0x0082: cpu->a=_gd(); break;
            // -- OVERLAP
            case 0x0083: _fetch(); break;
            
            // 0x3F: ccf (M:1 T:4)
            // -- OVERLAP
            case 0x0084: z80_ccf(cpu);_fetch(); break;
            
            // 0x40: ld b,b (M:1 T:4)
            // -- OVERLAP
            case 0x0085: cpu->b=cpu->b;_fetch(); break;
            
            // 0x41: ld b,c (M:1 T:4)
            // -- OVERLAP
            case 0x0086: cpu->b=cpu->c;_fetch(); break;
            
            // 0x42: ld b,d (M:1 T:4)
            // -- OVERLAP
            case 0x0087: cpu->b=cpu->d;_fetch(); break;
            
            // 0x43: ld b,e (M:1 T:4)
            // -- OVERLAP
            case 0x0088: cpu->b=cpu->e;_fetch(); break;
            
            // 0x44: ld b,h (M:1 T:4)
            // -- OVERLAP
            case 0x0089: cpu->b=cpu->hlx[cpu->hlx_idx].h;_fetch(); break;
            
            // 0x45: ld b,l (M:1 T:4)
            // -- OVERLAP
            case 0x008A: cpu->b=cpu->hlx[cpu->hlx_idx].l;_fetch(); break;
            
            // 0x46: ld b,(hl) (M:2 T:7)
            // -- M2
            case 0x008B: _mread(cpu->addr); break;
            case 0x008C: cpu->b=_gd(); break;
            // -- OVERLAP
            case 0x008D: _fetch(); break;
            
            // 0x47: ld b,a (M:1 T:4)
            // -- OVERLAP
            case 0x008E: cpu->b=cpu->a;_fetch(); break;
            
            // 0x48: ld c,b (M:1 T:4)
            // -- OVERLAP
            case 0x008F: cpu->c=cpu->b;_fetch(); break;
            
            // 0x49: ld c,c (M:1 T:4)
            // -- OVERLAP
            case 0x0090: cpu->c=cpu->c;_fetch(); break;
            
            // 0x4A: ld c,d (M:1 T:4)
            // -- OVERLAP
            case 0x0091: cpu->c=cpu->d;_fetch(); break;
            
            // 0x4B: ld c,e (M:1 T:4)
            // -- OVERLAP
            case 0x0092: cpu->c=cpu->e;_fetch(); break;
            
            // 0x4C: ld c,h (M:1 T:4)
            // -- OVERLAP
            case 0x0093: cpu->c=cpu->hlx[cpu->hlx_idx].h;_fetch(); break;
            
            // 0x4D: ld c,l (M:1 T:4)
            // -- OVERLAP
            case 0x0094: cpu->c=cpu->hlx[cpu->hlx_idx].l;_fetch(); break;
            
            // 0x4E: ld c,(hl) (M:2 T:7)
            // -- M2
            case 0x0095: _mread(cpu->addr); break;
            case 0x0096: cpu->c=_gd(); break;
            // -- OVERLAP
            case 0x0097: _fetch(); break;
            
            // 0x4F: ld c,a (M:1 T:4)
            // -- OVERLAP
            case 0x0098: cpu->c=cpu->a;_fetch(); break;
            
            // 0x50: ld d,b (M:1 T:4)
            // -- OVERLAP
            case 0x0099: cpu->d=cpu->b;_fetch(); break;
            
            // 0x51: ld d,c (M:1 T:4)
            // -- OVERLAP
            case 0x009A: cpu->d=cpu->c;_fetch(); break;
            
            // 0x52: ld d,d (M:1 T:4)
            // -- OVERLAP
            case 0x009B: cpu->d=cpu->d;_fetch(); break;
            
            // 0x53: ld d,e (M:1 T:4)
            // -- OVERLAP
            case 0x009C: cpu->d=cpu->e;_fetch(); break;
            
            // 0x54: ld d,h (M:1 T:4)
            // -- OVERLAP
            case 0x009D: cpu->d=cpu->hlx[cpu->hlx_idx].h;_fetch(); break;
            
            // 0x55: ld d,l (M:1 T:4)
            // -- OVERLAP
            case 0x009E: cpu->d=cpu->hlx[cpu->hlx_idx].l;_fetch(); break;
            
            // 0x56: ld d,(hl) (M:2 T:7)
            // -- M2
            case 0x009F: _mread(cpu->addr); break;
            case 0x00A0: cpu->d=_gd(); break;
            // -- OVERLAP
            case 0x00A1: _fetch(); break;
            
            // 0x57: ld d,a (M:1 T:4)
            // -- OVERLAP
            case 0x00A2: cpu->d=cpu->a;_fetch(); break;
            
            // 0x58: ld e,b (M:1 T:4)
            // -- OVERLAP
            case 0x00A3: cpu->e=cpu->b;_fetch(); break;
            
            // 0x59: ld e,c (M:1 T:4)
            // -- OVERLAP
            case 0x00A4: cpu->e=cpu->c;_fetch(); break;
            
            // 0x5A: ld e,d (M:1 T:4)
            // -- OVERLAP
            case 0x00A5: cpu->e=cpu->d;_fetch(); break;
            
            // 0x5B: ld e,e (M:1 T:4)
            // -- OVERLAP
            case 0x00A6: cpu->e=cpu->e;_fetch(); break;
            
            // 0x5C: ld e,h (M:1 T:4)
            // -- OVERLAP
            case 0x00A7: cpu->e=cpu->hlx[cpu->hlx_idx].h;_fetch(); break;
            
            // 0x5D: ld e,l (M:1 T:4)
            // -- OVERLAP
            case 0x00A8: cpu->e=cpu->hlx[cpu->hlx_idx].l;_fetch(); break;
            
            // 0x5E: ld e,(hl) (M:2 T:7)
            // -- M2
            case 0x00A9: _mread(cpu->addr); break;
            case 0x00AA: cpu->e=_gd(); break;
            // -- OVERLAP
            case 0x00AB: _fetch(); break;
            
            // 0x5F: ld e,a (M:1 T:4)
            // -- OVERLAP
            case 0x00AC: cpu->e=cpu->a;_fetch(); break;
            
            // 0x60: ld h,b (M:1 T:4)
            // -- OVERLAP
            case 0x00AD: cpu->hlx[cpu->hlx_idx].h=cpu->b;_fetch(); break;
            
            // 0x61: ld h,c (M:1 T:4)
            // -- OVERLAP
            case 0x00AE: cpu->hlx[cpu->hlx_idx].h=cpu->c;_fetch(); break;
            
            // 0x62: ld h,d (M:1 T:4)
            // -- OVERLAP
            case 0x00AF: cpu->hlx[cpu->hlx_idx].h=cpu->d;_fetch(); break;
            
            // 0x63: ld h,e (M:1 T:4)
            // -- OVERLAP
            case 0x00B0: cpu->hlx[cpu->hlx_idx].h=cpu->e;_fetch(); break;
            
            // 0x64: ld h,h (M:1 T:4)
            // -- OVERLAP
            case 0x00B1: cpu->hlx[cpu->hlx_idx].h=cpu->hlx[cpu->hlx_idx].h;_fetch(); break;
            
            // 0x65: ld h,l (M:1 T:4)
            // -- OVERLAP
            case 0x00B2: cpu->hlx[cpu->hlx_idx].h=cpu->hlx[cpu->hlx_idx].l;_fetch(); break;
            
            // 0x66: ld h,(hl) (M:2 T:7)
            // -- M2
            case 0x00B3: _mread(cpu->addr); break;
            case 0x00B4: cpu->h=_gd(); break;
            // -- OVERLAP
            case 0x00B5: _fetch(); break;
            
            // 0x67: ld h,a (M:1 T:4)
            // -- OVERLAP
            case 0x00B6: cpu->hlx[cpu->hlx_idx].h=cpu->a;_fetch(); break;
            
            // 0x68: ld l,b (M:1 T:4)
            // -- OVERLAP
            case 0x00B7: cpu->hlx[cpu->hlx_idx].l=cpu->b;_fetch(); break;
            
            // 0x69: ld l,c (M:1 T:4)
            // -- OVERLAP
            case 0x00B8: cpu->hlx[cpu->hlx_idx].l=cpu->c;_fetch(); break;
            
            // 0x6A: ld l,d (M:1 T:4)
            // -- OVERLAP
            case 0x00B9: cpu->hlx[cpu->hlx_idx].l=cpu->d;_fetch(); break;
            
            // 0x6B: ld l,e (M:1 T:4)
            // -- OVERLAP
            case 0x00BA: cpu->hlx[cpu->hlx_idx].l=cpu->e;_fetch(); break;
            
            // 0x6C: ld l,h (M:1 T:4)
            // -- OVERLAP
            case 0x00BB: cpu->hlx[cpu->hlx_idx].l=cpu->hlx[cpu->hlx_idx].h;_fetch(); break;
            
            // 0x6D: ld l,l (M:1 T:4)
            // -- OVERLAP
            case 0x00BC: cpu->hlx[cpu->hlx_idx].l=cpu->hlx[cpu->hlx_idx].l;_fetch(); break;
            
            // 0x6E: ld l,(hl) (M:2 T:7)
            // -- M2
            case 0x00BD: _mread(cpu->addr); break;
            case 0x00BE: cpu->l=_gd(); break;
            // -- OVERLAP
            case 0x00BF: _fetch(); break;
            
            // 0x6F: ld l,a (M:1 T:4)
            // -- OVERLAP
            case 0x00C0: cpu->hlx[cpu->hlx_idx].l=cpu->a;_fetch(); break;
            
            // 0x70: ld (hl),b (M:2 T:7)
            // -- M2
            case 0x00C1: _mwrite(cpu->addr,cpu->b); break;
            // -- OVERLAP
            case 0x00C2: _fetch(); break;
            
            // 0x71: ld (hl),c (M:2 T:7)
            // -- M2
            case 0x00C3: _mwrite(cpu->addr,cpu->c); break;
            // -- OVERLAP
            case 0x00C4: _fetch(); break;
            
            // 0x72: ld (hl),d (M:2 T:7)
            // -- M2
            case 0x00C5: _mwrite(cpu->addr,cpu->d); break;
            // -- OVERLAP
            case 0x00C6: _fetch(); break;
            
            // 0x73: ld (hl),e (M:2 T:7)
            // -- M2
            case 0x00C7: _mwrite(cpu->addr,cpu->e); break;
            // -- OVERLAP
            case 0x00C8: _fetch(); break;
            
            // 0x74: ld (hl),h (M:2 T:7)
            // -- M2
            case 0x00C9: _mwrite(cpu->addr,cpu->h); break;
            // -- OVERLAP
            case 0x00CA: _fetch(); break;
            
            // 0x75: ld (hl),l (M:2 T:7)
            // -- M2
            case 0x00CB: _mwrite(cpu->addr,cpu->l); break;
            // -- OVERLAP
            case 0x00CC: _fetch(); break;
            
            // 0x76: halt (M:1 T:4)
            // -- OVERLAP
            case 0x00CD: pins=z80_halt(cpu,pins);_fetch(); break;
            
            // 0x77: ld (hl),a (M:2 T:7)
            // -- M2
            case 0x00CE: _mwrite(cpu->addr,cpu->a); break;
            // -- OVERLAP
            case 0x00CF: _fetch(); break;
            
            // 0x78: ld a,b (M:1 T:4)
            // -- OVERLAP
            case 0x00D0: cpu->a=cpu->b;_fetch(); break;
            
            // 0x79: ld a,c (M:1 T:4)
            // -- OVERLAP
            case 0x00D1: cpu->a=cpu->c;_fetch(); break;
            
            // 0x7A: ld a,d (M:1 T:4)
            // -- OVERLAP
            case 0x00D2: cpu->a=cpu->d;_fetch(); break;
            
            // 0x7B: ld a,e (M:1 T:4)
            // -- OVERLAP
            case 0x00D3: cpu->a=cpu->e;_fetch(); break;
            
            // 0x7C: ld a,h (M:1 T:4)
            // -- OVERLAP
            case 0x00D4: cpu->a=cpu->hlx[cpu->hlx_idx].h;_fetch(); break;
            
            // 0x7D: ld a,l (M:1 T:4)
            // -- OVERLAP
            case 0x00D5: cpu->a=cpu->hlx[cpu->hlx_idx].l;_fetch(); break;
            
            // 0x7E: ld a,(hl) (M:2 T:7)
            // -- M2
            case 0x00D6: _mread(cpu->addr); break;
            case 0x00D7: cpu->a=_gd(); break;
            // -- OVERLAP
            case 0x00D8: _fetch(); break;
            
            // 0x7F: ld a,a (M:1 T:4)
            // -- OVERLAP
            case 0x00D9: cpu->a=cpu->a;_fetch(); break;
            
            // 0x80: add b (M:1 T:4)
            // -- OVERLAP
            case 0x00DA: z80_add8(cpu,cpu->b);_fetch(); break;
            
            // 0x81: add c (M:1 T:4)
            // -- OVERLAP
            case 0x00DB: z80_add8(cpu,cpu->c);_fetch(); break;
            
            // 0x82: add d (M:1 T:4)
            // -- OVERLAP
            case 0x00DC: z80_add8(cpu,cpu->d);_fetch(); break;
            
            // 0x83: add e (M:1 T:4)
            // -- OVERLAP
            case 0x00DD: z80_add8(cpu,cpu->e);_fetch(); break;
            
            // 0x84: add h (M:1 T:4)
            // -- OVERLAP
            case 0x00DE: z80_add8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); break;
            
            // 0x85: add l (M:1 T:4)
            // -- OVERLAP
            case 0x00DF: z80_add8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); break;
            
            // 0x86: add (hl) (M:2 T:7)
            // -- M2
            case 0x00E0: _mread(cpu->addr); break;
            case 0x00E1: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x00E2: z80_add8(cpu,cpu->dlatch);_fetch(); break;
            
            // 0x87: add a (M:1 T:4)
            // -- OVERLAP
            case 0x00E3: z80_add8(cpu,cpu->a);_fetch(); break;
            
            // 0x88: adc b (M:1 T:4)
            // -- OVERLAP
            case 0x00E4: z80_adc8(cpu,cpu->b);_fetch(); break;
            
            // 0x89: adc c (M:1 T:4)
            // -- OVERLAP
            case 0x00E5: z80_adc8(cpu,cpu->c);_fetch(); break;
            
            // 0x8A: adc d (M:1 T:4)
            // -- OVERLAP
            case 0x00E6: z80_adc8(cpu,cpu->d);_fetch(); break;
            
            // 0x8B: adc e (M:1 T:4)
            // -- OVERLAP
            case 0x00E7: z80_adc8(cpu,cpu->e);_fetch(); break;
            
            // 0x8C: adc h (M:1 T:4)
            // -- OVERLAP
            case 0x00E8: z80_adc8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); break;
            
            // 0x8D: adc l (M:1 T:4)
            // -- OVERLAP
            case 0x00E9: z80_adc8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); break;
            
            // 0x8E: adc (hl) (M:2 T:7)
            // -- M2
            case 0x00EA: _mread(cpu->addr); break;
            case 0x00EB: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x00EC: z80_adc8(cpu,cpu->dlatch);_fetch(); break;
            
            // 0x8F: adc a (M:1 T:4)
            // -- OVERLAP
            case 0x00ED: z80_adc8(cpu,cpu->a);_fetch(); break;
            
            // 0x90: sub b (M:1 T:4)
            // -- OVERLAP
            case 0x00EE: z80_sub8(cpu,cpu->b);_fetch(); break;
            
            // 0x91: sub c (M:1 T:4)
            // -- OVERLAP
            case 0x00EF: z80_sub8(cpu,cpu->c);_fetch(); break;
            
            // 0x92: sub d (M:1 T:4)
            // -- OVERLAP
            case 0x00F0: z80_sub8(cpu,cpu->d);_fetch(); break;
            
            // 0x93: sub e (M:1 T:4)
            // -- OVERLAP
            case 0x00F1: z80_sub8(cpu,cpu->e);_fetch(); break;
            
            // 0x94: sub h (M:1 T:4)
            // -- OVERLAP
            case 0x00F2: z80_sub8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); break;
            
            // 0x95: sub l (M:1 T:4)
            // -- OVERLAP
            case 0x00F3: z80_sub8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); break;
            
            // 0x96: sub (hl) (M:2 T:7)
            // -- M2
            case 0x00F4: _mread(cpu->addr); break;
            case 0x00F5: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x00F6: z80_sub8(cpu,cpu->dlatch);_fetch(); break;
            
            // 0x97: sub a (M:1 T:4)
            // -- OVERLAP
            case 0x00F7: z80_sub8(cpu,cpu->a);_fetch(); break;
            
            // 0x98: sbc b (M:1 T:4)
            // -- OVERLAP
            case 0x00F8: z80_sbc8(cpu,cpu->b);_fetch(); break;
            
            // 0x99: sbc c (M:1 T:4)
            // -- OVERLAP
            case 0x00F9: z80_sbc8(cpu,cpu->c);_fetch(); break;
            
            // 0x9A: sbc d (M:1 T:4)
            // -- OVERLAP
            case 0x00FA: z80_sbc8(cpu,cpu->d);_fetch(); break;
            
            // 0x9B: sbc e (M:1 T:4)
            // -- OVERLAP
            case 0x00FB: z80_sbc8(cpu,cpu->e);_fetch(); break;
            
            // 0x9C: sbc h (M:1 T:4)
            // -- OVERLAP
            case 0x00FC: z80_sbc8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); break;
            
            // 0x9D: sbc l (M:1 T:4)
            // -- OVERLAP
            case 0x00FD: z80_sbc8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); break;
            
            // 0x9E: sbc (hl) (M:2 T:7)
            // -- M2
            case 0x00FE: _mread(cpu->addr); break;
            case 0x00FF: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x0100: z80_sbc8(cpu,cpu->dlatch);_fetch(); break;
            
            // 0x9F: sbc a (M:1 T:4)
            // -- OVERLAP
            case 0x0101: z80_sbc8(cpu,cpu->a);_fetch(); break;
            
            // 0xA0: and b (M:1 T:4)
            // -- OVERLAP
            case 0x0102: z80_and8(cpu,cpu->b);_fetch(); break;
            
            // 0xA1: and c (M:1 T:4)
            // -- OVERLAP
            case 0x0103: z80_and8(cpu,cpu->c);_fetch(); break;
            
            // 0xA2: and d (M:1 T:4)
            // -- OVERLAP
            case 0x0104: z80_and8(cpu,cpu->d);_fetch(); break;
            
            // 0xA3: and e (M:1 T:4)
            // -- OVERLAP
            case 0x0105: z80_and8(cpu,cpu->e);_fetch(); break;
            
            // 0xA4: and h (M:1 T:4)
            // -- OVERLAP
            case 0x0106: z80_and8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); break;
            
            // 0xA5: and l (M:1 T:4)
            // -- OVERLAP
            case 0x0107: z80_and8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); break;
            
            // 0xA6: and (hl) (M:2 T:7)
            // -- M2
            case 0x0108: _mread(cpu->addr); break;
            case 0x0109: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x010A: z80_and8(cpu,cpu->dlatch);_fetch(); break;
            
            // 0xA7: and a (M:1 T:4)
            // -- OVERLAP
            case 0x010B: z80_and8(cpu,cpu->a);_fetch(); break;
            
            // 0xA8: xor b (M:1 T:4)
            // -- OVERLAP
            case 0x010C: z80_xor8(cpu,cpu->b);_fetch(); break;
            
            // 0xA9: xor c (M:1 T:4)
            // -- OVERLAP
            case 0x010D: z80_xor8(cpu,cpu->c);_fetch(); break;
            
            // 0xAA: xor d (M:1 T:4)
            // -- OVERLAP
            case 0x010E: z80_xor8(cpu,cpu->d);_fetch(); break;
            
            // 0xAB: xor e (M:1 T:4)
            // -- OVERLAP
            case 0x010F: z80_xor8(cpu,cpu->e);_fetch(); break;
            
            // 0xAC: xor h (M:1 T:4)
            // -- OVERLAP
            case 0x0110: z80_xor8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); break;
            
            // 0xAD: xor l (M:1 T:4)
            // -- OVERLAP
            case 0x0111: z80_xor8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); break;
            
            // 0xAE: xor (hl) (M:2 T:7)
            // -- M2
            case 0x0112: _mread(cpu->addr); break;
            case 0x0113: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x0114: z80_xor8(cpu,cpu->dlatch);_fetch(); break;
            
            // 0xAF: xor a (M:1 T:4)
            // -- OVERLAP
            case 0x0115: z80_xor8(cpu,cpu->a);_fetch(); break;
            
            // 0xB0: or b (M:1 T:4)
            // -- OVERLAP
            case 0x0116: z80_or8(cpu,cpu->b);_fetch(); break;
            
            // 0xB1: or c (M:1 T:4)
            // -- OVERLAP
            case 0x0117: z80_or8(cpu,cpu->c);_fetch(); break;
            
            // 0xB2: or d (M:1 T:4)
            // -- OVERLAP
            case 0x0118: z80_or8(cpu,cpu->d);_fetch(); break;
            
            // 0xB3: or e (M:1 T:4)
            // -- OVERLAP
            case 0x0119: z80_or8(cpu,cpu->e);_fetch(); break;
            
            // 0xB4: or h (M:1 T:4)
            // -- OVERLAP
            case 0x011A: z80_or8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); break;
            
            // 0xB5: or l (M:1 T:4)
            // -- OVERLAP
            case 0x011B: z80_or8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); break;
            
            // 0xB6: or (hl) (M:2 T:7)
            // -- M2
            case 0x011C: _mread(cpu->addr); break;
            case 0x011D: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x011E: z80_or8(cpu,cpu->dlatch);_fetch(); break;
            
            // 0xB7: or a (M:1 T:4)
            // -- OVERLAP
            case 0x011F: z80_or8(cpu,cpu->a);_fetch(); break;
            
            // 0xB8: cp b (M:1 T:4)
            // -- OVERLAP
            case 0x0120: z80_cp8(cpu,cpu->b);_fetch(); break;
            
            // 0xB9: cp c (M:1 T:4)
            // -- OVERLAP
            case 0x0121: z80_cp8(cpu,cpu->c);_fetch(); break;
            
            // 0xBA: cp d (M:1 T:4)
            // -- OVERLAP
            case 0x0122: z80_cp8(cpu,cpu->d);_fetch(); break;
            
            // 0xBB: cp e (M:1 T:4)
            // -- OVERLAP
            case 0x0123: z80_cp8(cpu,cpu->e);_fetch(); break;
            
            // 0xBC: cp h (M:1 T:4)
            // -- OVERLAP
            case 0x0124: z80_cp8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); break;
            
            // 0xBD: cp l (M:1 T:4)
            // -- OVERLAP
            case 0x0125: z80_cp8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); break;
            
            // 0xBE: cp (hl) (M:2 T:7)
            // -- M2
            case 0x0126: _mread(cpu->addr); break;
            case 0x0127: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x0128: z80_cp8(cpu,cpu->dlatch);_fetch(); break;
            
            // 0xBF: cp a (M:1 T:4)
            // -- OVERLAP
            case 0x0129: z80_cp8(cpu,cpu->a);_fetch(); break;
            
            // 0xC0: ret nz (M:1 T:4)
            // -- OVERLAP
            case 0x012A: _fetch(); break;
            
            // 0xC1: pop bc (M:3 T:10)
            // -- M2
            case 0x012B: _mread(cpu->sp++); break;
            case 0x012C: cpu->c=_gd(); break;
            // -- M3
            case 0x012D: _mread(cpu->sp++); break;
            case 0x012E: cpu->b=_gd(); break;
            // -- OVERLAP
            case 0x012F: _fetch(); break;
            
            // 0xC2: jp nz,nn (M:3 T:10)
            // -- M2
            case 0x0130: _mread(cpu->pc++); break;
            case 0x0131: cpu->wzl=_gd(); break;
            // -- M3
            case 0x0132: _mread(cpu->pc++); break;
            case 0x0133: cpu->wzh=_gd();if(_cc_nz){cpu->pc=cpu->wz;}; break;
            // -- OVERLAP
            case 0x0134: _fetch(); break;
            
            // 0xC3: jp nn (M:1 T:4)
            // -- OVERLAP
            case 0x0135: _fetch(); break;
            
            // 0xC4: call nz,nn (M:1 T:4)
            // -- OVERLAP
            case 0x0136: _fetch(); break;
            
            // 0xC5: push bc (M:3 T:11)
            // -- M2
            case 0x0137: _mwrite(--cpu->sp,cpu->b); break;
            // -- M3
            case 0x0138: _mwrite(--cpu->sp,cpu->c); break;
            // -- OVERLAP
            case 0x0139: _fetch(); break;
            
            // 0xC6: add n (M:2 T:7)
            // -- M2
            case 0x013A: _mread(cpu->pc++); break;
            case 0x013B: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x013C: z80_add8(cpu,cpu->dlatch);_fetch(); break;
            
            // 0xC7: rst 0h (M:1 T:4)
            // -- OVERLAP
            case 0x013D: _fetch(); break;
            
            // 0xC8: ret z (M:1 T:4)
            // -- OVERLAP
            case 0x013E: _fetch(); break;
            
            // 0xC9: ret (M:1 T:4)
            // -- OVERLAP
            case 0x013F: _fetch(); break;
            
            // 0xCA: jp z,nn (M:3 T:10)
            // -- M2
            case 0x0140: _mread(cpu->pc++); break;
            case 0x0141: cpu->wzl=_gd(); break;
            // -- M3
            case 0x0142: _mread(cpu->pc++); break;
            case 0x0143: cpu->wzh=_gd();if(_cc_z){cpu->pc=cpu->wz;}; break;
            // -- OVERLAP
            case 0x0144: _fetch(); break;
            
            // 0xCB: cb prefix (M:1 T:4)
            // -- OVERLAP
            case 0x0145: _fetch(); break;
            
            // 0xCC: call z,nn (M:1 T:4)
            // -- OVERLAP
            case 0x0146: _fetch(); break;
            
            // 0xCD: call nn (M:1 T:4)
            // -- OVERLAP
            case 0x0147: _fetch(); break;
            
            // 0xCE: adc n (M:2 T:7)
            // -- M2
            case 0x0148: _mread(cpu->pc++); break;
            case 0x0149: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x014A: z80_adc8(cpu,cpu->dlatch);_fetch(); break;
            
            // 0xCF: rst 8h (M:1 T:4)
            // -- OVERLAP
            case 0x014B: _fetch(); break;
            
            // 0xD0: ret nc (M:1 T:4)
            // -- OVERLAP
            case 0x014C: _fetch(); break;
            
            // 0xD1: pop de (M:3 T:10)
            // -- M2
            case 0x014D: _mread(cpu->sp++); break;
            case 0x014E: cpu->e=_gd(); break;
            // -- M3
            case 0x014F: _mread(cpu->sp++); break;
            case 0x0150: cpu->d=_gd(); break;
            // -- OVERLAP
            case 0x0151: _fetch(); break;
            
            // 0xD2: jp nc,nn (M:3 T:10)
            // -- M2
            case 0x0152: _mread(cpu->pc++); break;
            case 0x0153: cpu->wzl=_gd(); break;
            // -- M3
            case 0x0154: _mread(cpu->pc++); break;
            case 0x0155: cpu->wzh=_gd();if(_cc_nc){cpu->pc=cpu->wz;}; break;
            // -- OVERLAP
            case 0x0156: _fetch(); break;
            
            // 0xD3: out (n),a (M:1 T:4)
            // -- OVERLAP
            case 0x0157: _fetch(); break;
            
            // 0xD4: call nc,nn (M:1 T:4)
            // -- OVERLAP
            case 0x0158: _fetch(); break;
            
            // 0xD5: push de (M:3 T:11)
            // -- M2
            case 0x0159: _mwrite(--cpu->sp,cpu->d); break;
            // -- M3
            case 0x015A: _mwrite(--cpu->sp,cpu->e); break;
            // -- OVERLAP
            case 0x015B: _fetch(); break;
            
            // 0xD6: sub n (M:2 T:7)
            // -- M2
            case 0x015C: _mread(cpu->pc++); break;
            case 0x015D: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x015E: z80_sub8(cpu,cpu->dlatch);_fetch(); break;
            
            // 0xD7: rst 10h (M:1 T:4)
            // -- OVERLAP
            case 0x015F: _fetch(); break;
            
            // 0xD8: ret c (M:1 T:4)
            // -- OVERLAP
            case 0x0160: _fetch(); break;
            
            // 0xD9: exx (M:1 T:4)
            // -- OVERLAP
            case 0x0161: z80_exx(cpu);_fetch(); break;
            
            // 0xDA: jp c,nn (M:3 T:10)
            // -- M2
            case 0x0162: _mread(cpu->pc++); break;
            case 0x0163: cpu->wzl=_gd(); break;
            // -- M3
            case 0x0164: _mread(cpu->pc++); break;
            case 0x0165: cpu->wzh=_gd();if(_cc_c){cpu->pc=cpu->wz;}; break;
            // -- OVERLAP
            case 0x0166: _fetch(); break;
            
            // 0xDB: in a,(n) (M:1 T:4)
            // -- OVERLAP
            case 0x0167: _fetch(); break;
            
            // 0xDC: call c,nn (M:1 T:4)
            // -- OVERLAP
            case 0x0168: _fetch(); break;
            
            // 0xDD: dd prefix (M:1 T:4)
            // -- OVERLAP
            case 0x0169: _fetch_ix(); break;
            
            // 0xDE: sbc n (M:2 T:7)
            // -- M2
            case 0x016A: _mread(cpu->pc++); break;
            case 0x016B: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x016C: z80_sbc8(cpu,cpu->dlatch);_fetch(); break;
            
            // 0xDF: rst 18h (M:1 T:4)
            // -- OVERLAP
            case 0x016D: _fetch(); break;
            
            // 0xE0: ret po (M:1 T:4)
            // -- OVERLAP
            case 0x016E: _fetch(); break;
            
            // 0xE1: pop hl (M:3 T:10)
            // -- M2
            case 0x016F: _mread(cpu->sp++); break;
            case 0x0170: cpu->hlx[cpu->hlx_idx].l=_gd(); break;
            // -- M3
            case 0x0171: _mread(cpu->sp++); break;
            case 0x0172: cpu->hlx[cpu->hlx_idx].h=_gd(); break;
            // -- OVERLAP
            case 0x0173: _fetch(); break;
            
            // 0xE2: jp po,nn (M:3 T:10)
            // -- M2
            case 0x0174: _mread(cpu->pc++); break;
            case 0x0175: cpu->wzl=_gd(); break;
            // -- M3
            case 0x0176: _mread(cpu->pc++); break;
            case 0x0177: cpu->wzh=_gd();if(_cc_po){cpu->pc=cpu->wz;}; break;
            // -- OVERLAP
            case 0x0178: _fetch(); break;
            
            // 0xE3: ex (sp),hl (M:5 T:19)
            // -- M2
            case 0x0179: _mread(cpu->sp); break;
            case 0x017A: cpu->wzl=_gd(); break;
            // -- M3
            case 0x017B: _mread(cpu->sp+1); break;
            case 0x017C: cpu->wzh=_gd(); break;
            // -- M4
            case 0x017D: _mwrite(cpu->sp+1,cpu->hlx[cpu->hlx_idx].h); break;
            // -- M5
            case 0x017E: _mwrite(cpu->sp,cpu->hlx[cpu->hlx_idx].l);cpu->hlx[cpu->hlx_idx].hl=cpu->wz; break;
            // -- OVERLAP
            case 0x017F: _fetch(); break;
            
            // 0xE4: call po,nn (M:1 T:4)
            // -- OVERLAP
            case 0x0180: _fetch(); break;
            
            // 0xE5: push hl (M:3 T:11)
            // -- M2
            case 0x0181: _mwrite(--cpu->sp,cpu->hlx[cpu->hlx_idx].h); break;
            // -- M3
            case 0x0182: _mwrite(--cpu->sp,cpu->hlx[cpu->hlx_idx].l); break;
            // -- OVERLAP
            case 0x0183: _fetch(); break;
            
            // 0xE6: and n (M:2 T:7)
            // -- M2
            case 0x0184: _mread(cpu->pc++); break;
            case 0x0185: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x0186: z80_and8(cpu,cpu->dlatch);_fetch(); break;
            
            // 0xE7: rst 20h (M:1 T:4)
            // -- OVERLAP
            case 0x0187: _fetch(); break;
            
            // 0xE8: ret pe (M:1 T:4)
            // -- OVERLAP
            case 0x0188: _fetch(); break;
            
            // 0xE9: jp hl (M:1 T:4)
            // -- OVERLAP
            case 0x0189: _fetch(); break;
            
            // 0xEA: jp pe,nn (M:3 T:10)
            // -- M2
            case 0x018A: _mread(cpu->pc++); break;
            case 0x018B: cpu->wzl=_gd(); break;
            // -- M3
            case 0x018C: _mread(cpu->pc++); break;
            case 0x018D: cpu->wzh=_gd();if(_cc_pe){cpu->pc=cpu->wz;}; break;
            // -- OVERLAP
            case 0x018E: _fetch(); break;
            
            // 0xEB: ex de,hl (M:1 T:4)
            // -- OVERLAP
            case 0x018F: z80_ex_de_hl(cpu);_fetch(); break;
            
            // 0xEC: call pe,nn (M:1 T:4)
            // -- OVERLAP
            case 0x0190: _fetch(); break;
            
            // 0xED: ed prefix (M:1 T:4)
            // -- OVERLAP
            case 0x0191: _fetch(); break;
            
            // 0xEE: xor n (M:2 T:7)
            // -- M2
            case 0x0192: _mread(cpu->pc++); break;
            case 0x0193: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x0194: z80_xor8(cpu,cpu->dlatch);_fetch(); break;
            
            // 0xEF: rst 28h (M:1 T:4)
            // -- OVERLAP
            case 0x0195: _fetch(); break;
            
            // 0xF0: ret p (M:1 T:4)
            // -- OVERLAP
            case 0x0196: _fetch(); break;
            
            // 0xF1: pop af (M:3 T:10)
            // -- M2
            case 0x0197: _mread(cpu->sp++); break;
            case 0x0198: cpu->f=_gd(); break;
            // -- M3
            case 0x0199: _mread(cpu->sp++); break;
            case 0x019A: cpu->a=_gd(); break;
            // -- OVERLAP
            case 0x019B: _fetch(); break;
            
            // 0xF2: jp p,nn (M:3 T:10)
            // -- M2
            case 0x019C: _mread(cpu->pc++); break;
            case 0x019D: cpu->wzl=_gd(); break;
            // -- M3
            case 0x019E: _mread(cpu->pc++); break;
            case 0x019F: cpu->wzh=_gd();if(_cc_p){cpu->pc=cpu->wz;}; break;
            // -- OVERLAP
            case 0x01A0: _fetch(); break;
            
            // 0xF3: di (M:1 T:4)
            // -- OVERLAP
            case 0x01A1: _fetch(); break;
            
            // 0xF4: call p,nn (M:1 T:4)
            // -- OVERLAP
            case 0x01A2: _fetch(); break;
            
            // 0xF5: push af (M:3 T:11)
            // -- M2
            case 0x01A3: _mwrite(--cpu->sp,cpu->a); break;
            // -- M3
            case 0x01A4: _mwrite(--cpu->sp,cpu->f); break;
            // -- OVERLAP
            case 0x01A5: _fetch(); break;
            
            // 0xF6: or n (M:2 T:7)
            // -- M2
            case 0x01A6: _mread(cpu->pc++); break;
            case 0x01A7: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x01A8: z80_or8(cpu,cpu->dlatch);_fetch(); break;
            
            // 0xF7: rst 30h (M:1 T:4)
            // -- OVERLAP
            case 0x01A9: _fetch(); break;
            
            // 0xF8: ret m (M:1 T:4)
            // -- OVERLAP
            case 0x01AA: _fetch(); break;
            
            // 0xF9: ld sp,hl (M:2 T:6)
            // -- M2 (generic)
            case 0x01AB: cpu->sp=cpu->hlx[cpu->hlx_idx].hl; break;
            // -- OVERLAP
            case 0x01AC: _fetch(); break;
            
            // 0xFA: jp m,nn (M:3 T:10)
            // -- M2
            case 0x01AD: _mread(cpu->pc++); break;
            case 0x01AE: cpu->wzl=_gd(); break;
            // -- M3
            case 0x01AF: _mread(cpu->pc++); break;
            case 0x01B0: cpu->wzh=_gd();if(_cc_m){cpu->pc=cpu->wz;}; break;
            // -- OVERLAP
            case 0x01B1: _fetch(); break;
            
            // 0xFB: ei (M:1 T:4)
            // -- OVERLAP
            case 0x01B2: _fetch(); break;
            
            // 0xFC: call m,nn (M:1 T:4)
            // -- OVERLAP
            case 0x01B3: _fetch(); break;
            
            // 0xFD: fd prefix (M:1 T:4)
            // -- OVERLAP
            case 0x01B4: _fetch_iy(); break;
            
            // 0xFE: cp n (M:2 T:7)
            // -- M2
            case 0x01B5: _mread(cpu->pc++); break;
            case 0x01B6: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x01B7: z80_cp8(cpu,cpu->dlatch);_fetch(); break;
            
            // 0xFF: rst 38h (M:1 T:4)
            // -- OVERLAP
            case 0x01B8: _fetch(); break;

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
#undef _fetch_ix
#undef _fetch_iy
#undef _mread
#undef _mwrite
#undef _ioread
#undef _iowrite
#undef _cc_nz
#undef _cc_z
#undef _cc_nc
#undef _cc_c
#undef _cc_po
#undef _cc_pe
#undef _cc_p
#undef _cc_m

#endif // CHIPS_IMPL