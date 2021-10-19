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

// values for hlx_idx for mapping HL, IX or IY, used as index into hlx[]
#define Z80_MAP_HL (0)
#define Z80_MAP_IX (1)
#define Z80_MAP_IY (2)

// currently active prefix
#define Z80_PREFIX_NONE (0)
#define Z80_PREFIX_CB   (1<<0)
#define Z80_PREFIX_DD   (1<<1)
#define Z80_PREFIX_ED   (1<<2)
#define Z80_PREFIX_FD   (1<<3)

typedef struct {
    uint32_t pip;   // the op's decode pipeline
    uint16_t step;  // first or current decoder switch-case branch step
    uint16_t flags; // Z80_OPSTATE_FLAGS_
} z80_opstate_t;

// CPU state
typedef struct {
    z80_opstate_t op;       // the currently active op
    union {
        struct {
            uint16_t step_offset;   // 0x100 on ED prefix, 0x200 on CB prefix
            uint8_t hlx_idx;        // index into hlx[] for mapping hl to ix or iy (0: hl, 1: ix, 2: iy)
            uint8_t prefix;         // one of Z80_PREFIX_*
        };
        uint32_t prefix_state;
    };

    union { struct { uint8_t pcl; uint8_t pch; }; uint16_t pc; };
    uint16_t addr;      // effective address for (HL),(IX+d),(IY+d)
    uint8_t ir;         // instruction register
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
    return (0 == cpu->op.step) && (cpu->prefix == 0);
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
static inline uint64_t z80_fetch(z80_t* cpu, uint64_t pins) {
    cpu->op.pip = 5<<1;
    cpu->op.step = 0xFFFF;
    cpu->prefix_state = 0;
    pins = z80_set_ab_x(pins, cpu->pc++, Z80_M1|Z80_MREQ|Z80_RD);
    return pins;
}

static inline uint64_t z80_fetch_prefix(z80_t* cpu, uint64_t pins, uint8_t prefix) {
    // reset the decoder to continue at step 0
    cpu->op.pip = 5<<1;
    cpu->op.step = 0xFFFF;
    switch (prefix) {
        case Z80_PREFIX_CB: // CB prefix preserves current DD/FD prefix
            cpu->step_offset = 0x0200;
            cpu->prefix |= Z80_PREFIX_CB;
            break;
        case Z80_PREFIX_DD:
            cpu->step_offset = 0;
            cpu->hlx_idx = 1;
            cpu->prefix = Z80_PREFIX_DD;
            break;
        case Z80_PREFIX_ED: // ED prefix clears current DD/FD prefix
            cpu->step_offset = 0x0100;
            cpu->hlx_idx = 0;
            cpu->prefix = Z80_PREFIX_ED;
            break;
        case Z80_PREFIX_FD:
            cpu->step_offset = 0;
            cpu->hlx_idx = 2;
            cpu->prefix = Z80_PREFIX_FD;
            break;
    }
    pins = z80_set_ab_x(pins, cpu->pc++, Z80_M1|Z80_MREQ|Z80_RD);
    return pins;
}

// initiate refresh cycle
static inline uint64_t z80_refresh(z80_t* cpu, uint64_t pins) {
    pins = z80_set_ab_x(pins, cpu->r, Z80_MREQ|Z80_RFSH);
    cpu->r = (cpu->r & 0x80) | ((cpu->r + 1) & 0x7F);
    return pins;
}

static const z80_opstate_t z80_opstate_table[3*256] = {
    //  00: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    //  01: ld bc,nn (M:3 T:10 steps:5)
    { 0x000000B6, 0x0005, 0 },
    //  02: ld (bc),a (M:2 T:7 steps:3)
    { 0x0000001C, 0x000A, 0 },
    //  03: inc bc (M:2 T:6 steps:2)
    { 0x0000000A, 0x000D, 0 },
    //  04: inc b (M:1 T:4 steps:1)
    { 0x00000002, 0x000F, 0 },
    //  05: dec b (M:1 T:4 steps:1)
    { 0x00000002, 0x0010, 0 },
    //  06: ld b,n (M:2 T:7 steps:3)
    { 0x00000016, 0x0011, Z80_OPSTATE_FLAGS_IMM8 },
    //  07: rlca (M:1 T:4 steps:1)
    { 0x00000002, 0x0014, 0 },
    //  08: ex af,af' (M:1 T:4 steps:1)
    { 0x00000002, 0x0015, 0 },
    //  09: add hl,bc (M:1 T:4 steps:1)
    { 0x00000002, 0x0016, 0 },
    //  0A: ld a,(bc) (M:2 T:7 steps:3)
    { 0x00000016, 0x0017, 0 },
    //  0B: dec bc (M:2 T:6 steps:2)
    { 0x0000000A, 0x001A, 0 },
    //  0C: inc c (M:1 T:4 steps:1)
    { 0x00000002, 0x001C, 0 },
    //  0D: dec c (M:1 T:4 steps:1)
    { 0x00000002, 0x001D, 0 },
    //  0E: ld c,n (M:2 T:7 steps:3)
    { 0x00000016, 0x001E, Z80_OPSTATE_FLAGS_IMM8 },
    //  0F: rrca (M:1 T:4 steps:1)
    { 0x00000002, 0x0021, 0 },
    //  10: djnz d (M:3 T:13 steps:4)
    { 0x0000042C, 0x0022, 0 },
    //  11: ld de,nn (M:3 T:10 steps:5)
    { 0x000000B6, 0x0026, 0 },
    //  12: ld (de),a (M:2 T:7 steps:3)
    { 0x0000001C, 0x002B, 0 },
    //  13: inc de (M:2 T:6 steps:2)
    { 0x0000000A, 0x002E, 0 },
    //  14: inc d (M:1 T:4 steps:1)
    { 0x00000002, 0x0030, 0 },
    //  15: dec d (M:1 T:4 steps:1)
    { 0x00000002, 0x0031, 0 },
    //  16: ld d,n (M:2 T:7 steps:3)
    { 0x00000016, 0x0032, Z80_OPSTATE_FLAGS_IMM8 },
    //  17: rla (M:1 T:4 steps:1)
    { 0x00000002, 0x0035, 0 },
    //  18: jr d (M:3 T:12 steps:4)
    { 0x00000216, 0x0036, 0 },
    //  19: add hl,de (M:1 T:4 steps:1)
    { 0x00000002, 0x003A, 0 },
    //  1A: ld a,(de) (M:2 T:7 steps:3)
    { 0x00000016, 0x003B, 0 },
    //  1B: dec de (M:2 T:6 steps:2)
    { 0x0000000A, 0x003E, 0 },
    //  1C: inc e (M:1 T:4 steps:1)
    { 0x00000002, 0x0040, 0 },
    //  1D: dec e (M:1 T:4 steps:1)
    { 0x00000002, 0x0041, 0 },
    //  1E: ld e,n (M:2 T:7 steps:3)
    { 0x00000016, 0x0042, Z80_OPSTATE_FLAGS_IMM8 },
    //  1F: rra (M:1 T:4 steps:1)
    { 0x00000002, 0x0045, 0 },
    //  20: jr nz,d (M:3 T:12 steps:4)
    { 0x00000216, 0x0046, 0 },
    //  21: ld hl,nn (M:3 T:10 steps:5)
    { 0x000000B6, 0x004A, 0 },
    //  22: ld (nn),hl (M:1 T:4 steps:1)
    { 0x00000002, 0x004F, 0 },
    //  23: inc hl (M:2 T:6 steps:2)
    { 0x0000000A, 0x0050, 0 },
    //  24: inc h (M:1 T:4 steps:1)
    { 0x00000002, 0x0052, 0 },
    //  25: dec h (M:1 T:4 steps:1)
    { 0x00000002, 0x0053, 0 },
    //  26: ld h,n (M:2 T:7 steps:3)
    { 0x00000016, 0x0054, Z80_OPSTATE_FLAGS_IMM8 },
    //  27: daa (M:1 T:4 steps:1)
    { 0x00000002, 0x0057, 0 },
    //  28: jr z,d (M:3 T:12 steps:4)
    { 0x00000216, 0x0058, 0 },
    //  29: add hl,hl (M:1 T:4 steps:1)
    { 0x00000002, 0x005C, 0 },
    //  2A: ld hl,(nn) (M:5 T:16 steps:9)
    { 0x00002DB6, 0x005D, 0 },
    //  2B: dec hl (M:2 T:6 steps:2)
    { 0x0000000A, 0x0066, 0 },
    //  2C: inc l (M:1 T:4 steps:1)
    { 0x00000002, 0x0068, 0 },
    //  2D: dec l (M:1 T:4 steps:1)
    { 0x00000002, 0x0069, 0 },
    //  2E: ld l,n (M:2 T:7 steps:3)
    { 0x00000016, 0x006A, Z80_OPSTATE_FLAGS_IMM8 },
    //  2F: cpl (M:1 T:4 steps:1)
    { 0x00000002, 0x006D, 0 },
    //  30: jr nc,d (M:3 T:12 steps:4)
    { 0x00000216, 0x006E, 0 },
    //  31: ld sp,nn (M:3 T:10 steps:5)
    { 0x000000B6, 0x0072, 0 },
    //  32: ld (nn),a (M:4 T:13 steps:7)
    { 0x00000736, 0x0077, 0 },
    //  33: inc sp (M:2 T:6 steps:2)
    { 0x0000000A, 0x007E, 0 },
    //  34: inc (hl) (M:3 T:11 steps:5)
    { 0x000001C6, 0x0080, Z80_OPSTATE_FLAGS_INDIRECT },
    //  35: dec (hl) (M:3 T:11 steps:5)
    { 0x000001C6, 0x0085, Z80_OPSTATE_FLAGS_INDIRECT },
    //  36: ld (hl),n (M:3 T:10 steps:5)
    { 0x000000E6, 0x008A, Z80_OPSTATE_FLAGS_INDIRECT|Z80_OPSTATE_FLAGS_IMM8 },
    //  37: scf (M:1 T:4 steps:1)
    { 0x00000002, 0x008F, 0 },
    //  38: jr c,d (M:3 T:12 steps:4)
    { 0x00000216, 0x0090, 0 },
    //  39: add hl,sp (M:1 T:4 steps:1)
    { 0x00000002, 0x0094, 0 },
    //  3A: ld a,(nn) (M:4 T:13 steps:7)
    { 0x000005B6, 0x0095, 0 },
    //  3B: dec sp (M:2 T:6 steps:2)
    { 0x0000000A, 0x009C, 0 },
    //  3C: inc a (M:1 T:4 steps:1)
    { 0x00000002, 0x009E, 0 },
    //  3D: dec a (M:1 T:4 steps:1)
    { 0x00000002, 0x009F, 0 },
    //  3E: ld a,n (M:2 T:7 steps:3)
    { 0x00000016, 0x00A0, Z80_OPSTATE_FLAGS_IMM8 },
    //  3F: ccf (M:1 T:4 steps:1)
    { 0x00000002, 0x00A3, 0 },
    //  40: ld b,b (M:1 T:4 steps:1)
    { 0x00000002, 0x00A4, 0 },
    //  41: ld b,c (M:1 T:4 steps:1)
    { 0x00000002, 0x00A5, 0 },
    //  42: ld b,d (M:1 T:4 steps:1)
    { 0x00000002, 0x00A6, 0 },
    //  43: ld b,e (M:1 T:4 steps:1)
    { 0x00000002, 0x00A7, 0 },
    //  44: ld b,h (M:1 T:4 steps:1)
    { 0x00000002, 0x00A8, 0 },
    //  45: ld b,l (M:1 T:4 steps:1)
    { 0x00000002, 0x00A9, 0 },
    //  46: ld b,(hl) (M:2 T:7 steps:3)
    { 0x00000016, 0x00AA, Z80_OPSTATE_FLAGS_INDIRECT },
    //  47: ld b,a (M:1 T:4 steps:1)
    { 0x00000002, 0x00AD, 0 },
    //  48: ld c,b (M:1 T:4 steps:1)
    { 0x00000002, 0x00AE, 0 },
    //  49: ld c,c (M:1 T:4 steps:1)
    { 0x00000002, 0x00AF, 0 },
    //  4A: ld c,d (M:1 T:4 steps:1)
    { 0x00000002, 0x00B0, 0 },
    //  4B: ld c,e (M:1 T:4 steps:1)
    { 0x00000002, 0x00B1, 0 },
    //  4C: ld c,h (M:1 T:4 steps:1)
    { 0x00000002, 0x00B2, 0 },
    //  4D: ld c,l (M:1 T:4 steps:1)
    { 0x00000002, 0x00B3, 0 },
    //  4E: ld c,(hl) (M:2 T:7 steps:3)
    { 0x00000016, 0x00B4, Z80_OPSTATE_FLAGS_INDIRECT },
    //  4F: ld c,a (M:1 T:4 steps:1)
    { 0x00000002, 0x00B7, 0 },
    //  50: ld d,b (M:1 T:4 steps:1)
    { 0x00000002, 0x00B8, 0 },
    //  51: ld d,c (M:1 T:4 steps:1)
    { 0x00000002, 0x00B9, 0 },
    //  52: ld d,d (M:1 T:4 steps:1)
    { 0x00000002, 0x00BA, 0 },
    //  53: ld d,e (M:1 T:4 steps:1)
    { 0x00000002, 0x00BB, 0 },
    //  54: ld d,h (M:1 T:4 steps:1)
    { 0x00000002, 0x00BC, 0 },
    //  55: ld d,l (M:1 T:4 steps:1)
    { 0x00000002, 0x00BD, 0 },
    //  56: ld d,(hl) (M:2 T:7 steps:3)
    { 0x00000016, 0x00BE, Z80_OPSTATE_FLAGS_INDIRECT },
    //  57: ld d,a (M:1 T:4 steps:1)
    { 0x00000002, 0x00C1, 0 },
    //  58: ld e,b (M:1 T:4 steps:1)
    { 0x00000002, 0x00C2, 0 },
    //  59: ld e,c (M:1 T:4 steps:1)
    { 0x00000002, 0x00C3, 0 },
    //  5A: ld e,d (M:1 T:4 steps:1)
    { 0x00000002, 0x00C4, 0 },
    //  5B: ld e,e (M:1 T:4 steps:1)
    { 0x00000002, 0x00C5, 0 },
    //  5C: ld e,h (M:1 T:4 steps:1)
    { 0x00000002, 0x00C6, 0 },
    //  5D: ld e,l (M:1 T:4 steps:1)
    { 0x00000002, 0x00C7, 0 },
    //  5E: ld e,(hl) (M:2 T:7 steps:3)
    { 0x00000016, 0x00C8, Z80_OPSTATE_FLAGS_INDIRECT },
    //  5F: ld e,a (M:1 T:4 steps:1)
    { 0x00000002, 0x00CB, 0 },
    //  60: ld h,b (M:1 T:4 steps:1)
    { 0x00000002, 0x00CC, 0 },
    //  61: ld h,c (M:1 T:4 steps:1)
    { 0x00000002, 0x00CD, 0 },
    //  62: ld h,d (M:1 T:4 steps:1)
    { 0x00000002, 0x00CE, 0 },
    //  63: ld h,e (M:1 T:4 steps:1)
    { 0x00000002, 0x00CF, 0 },
    //  64: ld h,h (M:1 T:4 steps:1)
    { 0x00000002, 0x00D0, 0 },
    //  65: ld h,l (M:1 T:4 steps:1)
    { 0x00000002, 0x00D1, 0 },
    //  66: ld h,(hl) (M:2 T:7 steps:3)
    { 0x00000016, 0x00D2, Z80_OPSTATE_FLAGS_INDIRECT },
    //  67: ld h,a (M:1 T:4 steps:1)
    { 0x00000002, 0x00D5, 0 },
    //  68: ld l,b (M:1 T:4 steps:1)
    { 0x00000002, 0x00D6, 0 },
    //  69: ld l,c (M:1 T:4 steps:1)
    { 0x00000002, 0x00D7, 0 },
    //  6A: ld l,d (M:1 T:4 steps:1)
    { 0x00000002, 0x00D8, 0 },
    //  6B: ld l,e (M:1 T:4 steps:1)
    { 0x00000002, 0x00D9, 0 },
    //  6C: ld l,h (M:1 T:4 steps:1)
    { 0x00000002, 0x00DA, 0 },
    //  6D: ld l,l (M:1 T:4 steps:1)
    { 0x00000002, 0x00DB, 0 },
    //  6E: ld l,(hl) (M:2 T:7 steps:3)
    { 0x00000016, 0x00DC, Z80_OPSTATE_FLAGS_INDIRECT },
    //  6F: ld l,a (M:1 T:4 steps:1)
    { 0x00000002, 0x00DF, 0 },
    //  70: ld (hl),b (M:2 T:7 steps:3)
    { 0x0000001C, 0x00E0, Z80_OPSTATE_FLAGS_INDIRECT },
    //  71: ld (hl),c (M:2 T:7 steps:3)
    { 0x0000001C, 0x00E3, Z80_OPSTATE_FLAGS_INDIRECT },
    //  72: ld (hl),d (M:2 T:7 steps:3)
    { 0x0000001C, 0x00E6, Z80_OPSTATE_FLAGS_INDIRECT },
    //  73: ld (hl),e (M:2 T:7 steps:3)
    { 0x0000001C, 0x00E9, Z80_OPSTATE_FLAGS_INDIRECT },
    //  74: ld (hl),h (M:2 T:7 steps:3)
    { 0x0000001C, 0x00EC, Z80_OPSTATE_FLAGS_INDIRECT },
    //  75: ld (hl),l (M:2 T:7 steps:3)
    { 0x0000001C, 0x00EF, Z80_OPSTATE_FLAGS_INDIRECT },
    //  76: halt (M:1 T:4 steps:1)
    { 0x00000002, 0x00F2, 0 },
    //  77: ld (hl),a (M:2 T:7 steps:3)
    { 0x0000001C, 0x00F3, Z80_OPSTATE_FLAGS_INDIRECT },
    //  78: ld a,b (M:1 T:4 steps:1)
    { 0x00000002, 0x00F6, 0 },
    //  79: ld a,c (M:1 T:4 steps:1)
    { 0x00000002, 0x00F7, 0 },
    //  7A: ld a,d (M:1 T:4 steps:1)
    { 0x00000002, 0x00F8, 0 },
    //  7B: ld a,e (M:1 T:4 steps:1)
    { 0x00000002, 0x00F9, 0 },
    //  7C: ld a,h (M:1 T:4 steps:1)
    { 0x00000002, 0x00FA, 0 },
    //  7D: ld a,l (M:1 T:4 steps:1)
    { 0x00000002, 0x00FB, 0 },
    //  7E: ld a,(hl) (M:2 T:7 steps:3)
    { 0x00000016, 0x00FC, Z80_OPSTATE_FLAGS_INDIRECT },
    //  7F: ld a,a (M:1 T:4 steps:1)
    { 0x00000002, 0x00FF, 0 },
    //  80: add b (M:1 T:4 steps:1)
    { 0x00000002, 0x0100, 0 },
    //  81: add c (M:1 T:4 steps:1)
    { 0x00000002, 0x0101, 0 },
    //  82: add d (M:1 T:4 steps:1)
    { 0x00000002, 0x0102, 0 },
    //  83: add e (M:1 T:4 steps:1)
    { 0x00000002, 0x0103, 0 },
    //  84: add h (M:1 T:4 steps:1)
    { 0x00000002, 0x0104, 0 },
    //  85: add l (M:1 T:4 steps:1)
    { 0x00000002, 0x0105, 0 },
    //  86: add (hl) (M:2 T:7 steps:3)
    { 0x00000016, 0x0106, Z80_OPSTATE_FLAGS_INDIRECT },
    //  87: add a (M:1 T:4 steps:1)
    { 0x00000002, 0x0109, 0 },
    //  88: adc b (M:1 T:4 steps:1)
    { 0x00000002, 0x010A, 0 },
    //  89: adc c (M:1 T:4 steps:1)
    { 0x00000002, 0x010B, 0 },
    //  8A: adc d (M:1 T:4 steps:1)
    { 0x00000002, 0x010C, 0 },
    //  8B: adc e (M:1 T:4 steps:1)
    { 0x00000002, 0x010D, 0 },
    //  8C: adc h (M:1 T:4 steps:1)
    { 0x00000002, 0x010E, 0 },
    //  8D: adc l (M:1 T:4 steps:1)
    { 0x00000002, 0x010F, 0 },
    //  8E: adc (hl) (M:2 T:7 steps:3)
    { 0x00000016, 0x0110, Z80_OPSTATE_FLAGS_INDIRECT },
    //  8F: adc a (M:1 T:4 steps:1)
    { 0x00000002, 0x0113, 0 },
    //  90: sub b (M:1 T:4 steps:1)
    { 0x00000002, 0x0114, 0 },
    //  91: sub c (M:1 T:4 steps:1)
    { 0x00000002, 0x0115, 0 },
    //  92: sub d (M:1 T:4 steps:1)
    { 0x00000002, 0x0116, 0 },
    //  93: sub e (M:1 T:4 steps:1)
    { 0x00000002, 0x0117, 0 },
    //  94: sub h (M:1 T:4 steps:1)
    { 0x00000002, 0x0118, 0 },
    //  95: sub l (M:1 T:4 steps:1)
    { 0x00000002, 0x0119, 0 },
    //  96: sub (hl) (M:2 T:7 steps:3)
    { 0x00000016, 0x011A, Z80_OPSTATE_FLAGS_INDIRECT },
    //  97: sub a (M:1 T:4 steps:1)
    { 0x00000002, 0x011D, 0 },
    //  98: sbc b (M:1 T:4 steps:1)
    { 0x00000002, 0x011E, 0 },
    //  99: sbc c (M:1 T:4 steps:1)
    { 0x00000002, 0x011F, 0 },
    //  9A: sbc d (M:1 T:4 steps:1)
    { 0x00000002, 0x0120, 0 },
    //  9B: sbc e (M:1 T:4 steps:1)
    { 0x00000002, 0x0121, 0 },
    //  9C: sbc h (M:1 T:4 steps:1)
    { 0x00000002, 0x0122, 0 },
    //  9D: sbc l (M:1 T:4 steps:1)
    { 0x00000002, 0x0123, 0 },
    //  9E: sbc (hl) (M:2 T:7 steps:3)
    { 0x00000016, 0x0124, Z80_OPSTATE_FLAGS_INDIRECT },
    //  9F: sbc a (M:1 T:4 steps:1)
    { 0x00000002, 0x0127, 0 },
    //  A0: and b (M:1 T:4 steps:1)
    { 0x00000002, 0x0128, 0 },
    //  A1: and c (M:1 T:4 steps:1)
    { 0x00000002, 0x0129, 0 },
    //  A2: and d (M:1 T:4 steps:1)
    { 0x00000002, 0x012A, 0 },
    //  A3: and e (M:1 T:4 steps:1)
    { 0x00000002, 0x012B, 0 },
    //  A4: and h (M:1 T:4 steps:1)
    { 0x00000002, 0x012C, 0 },
    //  A5: and l (M:1 T:4 steps:1)
    { 0x00000002, 0x012D, 0 },
    //  A6: and (hl) (M:2 T:7 steps:3)
    { 0x00000016, 0x012E, Z80_OPSTATE_FLAGS_INDIRECT },
    //  A7: and a (M:1 T:4 steps:1)
    { 0x00000002, 0x0131, 0 },
    //  A8: xor b (M:1 T:4 steps:1)
    { 0x00000002, 0x0132, 0 },
    //  A9: xor c (M:1 T:4 steps:1)
    { 0x00000002, 0x0133, 0 },
    //  AA: xor d (M:1 T:4 steps:1)
    { 0x00000002, 0x0134, 0 },
    //  AB: xor e (M:1 T:4 steps:1)
    { 0x00000002, 0x0135, 0 },
    //  AC: xor h (M:1 T:4 steps:1)
    { 0x00000002, 0x0136, 0 },
    //  AD: xor l (M:1 T:4 steps:1)
    { 0x00000002, 0x0137, 0 },
    //  AE: xor (hl) (M:2 T:7 steps:3)
    { 0x00000016, 0x0138, Z80_OPSTATE_FLAGS_INDIRECT },
    //  AF: xor a (M:1 T:4 steps:1)
    { 0x00000002, 0x013B, 0 },
    //  B0: or b (M:1 T:4 steps:1)
    { 0x00000002, 0x013C, 0 },
    //  B1: or c (M:1 T:4 steps:1)
    { 0x00000002, 0x013D, 0 },
    //  B2: or d (M:1 T:4 steps:1)
    { 0x00000002, 0x013E, 0 },
    //  B3: or e (M:1 T:4 steps:1)
    { 0x00000002, 0x013F, 0 },
    //  B4: or h (M:1 T:4 steps:1)
    { 0x00000002, 0x0140, 0 },
    //  B5: or l (M:1 T:4 steps:1)
    { 0x00000002, 0x0141, 0 },
    //  B6: or (hl) (M:2 T:7 steps:3)
    { 0x00000016, 0x0142, Z80_OPSTATE_FLAGS_INDIRECT },
    //  B7: or a (M:1 T:4 steps:1)
    { 0x00000002, 0x0145, 0 },
    //  B8: cp b (M:1 T:4 steps:1)
    { 0x00000002, 0x0146, 0 },
    //  B9: cp c (M:1 T:4 steps:1)
    { 0x00000002, 0x0147, 0 },
    //  BA: cp d (M:1 T:4 steps:1)
    { 0x00000002, 0x0148, 0 },
    //  BB: cp e (M:1 T:4 steps:1)
    { 0x00000002, 0x0149, 0 },
    //  BC: cp h (M:1 T:4 steps:1)
    { 0x00000002, 0x014A, 0 },
    //  BD: cp l (M:1 T:4 steps:1)
    { 0x00000002, 0x014B, 0 },
    //  BE: cp (hl) (M:2 T:7 steps:3)
    { 0x00000016, 0x014C, Z80_OPSTATE_FLAGS_INDIRECT },
    //  BF: cp a (M:1 T:4 steps:1)
    { 0x00000002, 0x014F, 0 },
    //  C0: ret nz (M:4 T:11 steps:6)
    { 0x0000016E, 0x0150, 0 },
    //  C1: pop bc (M:3 T:10 steps:5)
    { 0x000000B6, 0x0156, 0 },
    //  C2: jp nz,nn (M:3 T:10 steps:5)
    { 0x000000B6, 0x015B, 0 },
    //  C3: jp nn (M:3 T:10 steps:5)
    { 0x000000B6, 0x0160, 0 },
    //  C4: call nz,nn (M:6 T:17 steps:10)
    { 0x000076B6, 0x0165, 0 },
    //  C5: push bc (M:3 T:11 steps:5)
    { 0x000001D8, 0x016F, 0 },
    //  C6: add n (M:2 T:7 steps:3)
    { 0x00000016, 0x0174, Z80_OPSTATE_FLAGS_IMM8 },
    //  C7: rst 0h (M:1 T:4 steps:1)
    { 0x00000002, 0x0177, 0 },
    //  C8: ret z (M:4 T:11 steps:6)
    { 0x0000016E, 0x0178, 0 },
    //  C9: ret (M:3 T:10 steps:5)
    { 0x000000B6, 0x017E, 0 },
    //  CA: jp z,nn (M:3 T:10 steps:5)
    { 0x000000B6, 0x0183, 0 },
    //  CB: cb prefix (M:1 T:4 steps:1)
    { 0x00000002, 0x0188, 0 },
    //  CC: call z,nn (M:6 T:17 steps:10)
    { 0x000076B6, 0x0189, 0 },
    //  CD: call nn (M:5 T:17 steps:9)
    { 0x00007636, 0x0193, 0 },
    //  CE: adc n (M:2 T:7 steps:3)
    { 0x00000016, 0x019C, Z80_OPSTATE_FLAGS_IMM8 },
    //  CF: rst 8h (M:1 T:4 steps:1)
    { 0x00000002, 0x019F, 0 },
    //  D0: ret nc (M:4 T:11 steps:6)
    { 0x0000016E, 0x01A0, 0 },
    //  D1: pop de (M:3 T:10 steps:5)
    { 0x000000B6, 0x01A6, 0 },
    //  D2: jp nc,nn (M:3 T:10 steps:5)
    { 0x000000B6, 0x01AB, 0 },
    //  D3: out (n),a (M:1 T:4 steps:1)
    { 0x00000002, 0x01B0, 0 },
    //  D4: call nc,nn (M:6 T:17 steps:10)
    { 0x000076B6, 0x01B1, 0 },
    //  D5: push de (M:3 T:11 steps:5)
    { 0x000001D8, 0x01BB, 0 },
    //  D6: sub n (M:2 T:7 steps:3)
    { 0x00000016, 0x01C0, Z80_OPSTATE_FLAGS_IMM8 },
    //  D7: rst 10h (M:1 T:4 steps:1)
    { 0x00000002, 0x01C3, 0 },
    //  D8: ret c (M:4 T:11 steps:6)
    { 0x0000016E, 0x01C4, 0 },
    //  D9: exx (M:1 T:4 steps:1)
    { 0x00000002, 0x01CA, 0 },
    //  DA: jp c,nn (M:3 T:10 steps:5)
    { 0x000000B6, 0x01CB, 0 },
    //  DB: in a,(n) (M:1 T:4 steps:1)
    { 0x00000002, 0x01D0, 0 },
    //  DC: call c,nn (M:6 T:17 steps:10)
    { 0x000076B6, 0x01D1, 0 },
    //  DD: dd prefix (M:1 T:4 steps:1)
    { 0x00000002, 0x01DB, 0 },
    //  DE: sbc n (M:2 T:7 steps:3)
    { 0x00000016, 0x01DC, Z80_OPSTATE_FLAGS_IMM8 },
    //  DF: rst 18h (M:1 T:4 steps:1)
    { 0x00000002, 0x01DF, 0 },
    //  E0: ret po (M:4 T:11 steps:6)
    { 0x0000016E, 0x01E0, 0 },
    //  E1: pop hl (M:3 T:10 steps:5)
    { 0x000000B6, 0x01E6, 0 },
    //  E2: jp po,nn (M:3 T:10 steps:5)
    { 0x000000B6, 0x01EB, 0 },
    //  E3: ex (sp),hl (M:5 T:19 steps:9)
    { 0x00013636, 0x01F0, 0 },
    //  E4: call po,nn (M:6 T:17 steps:10)
    { 0x000076B6, 0x01F9, 0 },
    //  E5: push hl (M:3 T:11 steps:5)
    { 0x000001D8, 0x0203, 0 },
    //  E6: and n (M:2 T:7 steps:3)
    { 0x00000016, 0x0208, Z80_OPSTATE_FLAGS_IMM8 },
    //  E7: rst 20h (M:1 T:4 steps:1)
    { 0x00000002, 0x020B, 0 },
    //  E8: ret pe (M:4 T:11 steps:6)
    { 0x0000016E, 0x020C, 0 },
    //  E9: jp hl (M:1 T:4 steps:1)
    { 0x00000002, 0x0212, 0 },
    //  EA: jp pe,nn (M:3 T:10 steps:5)
    { 0x000000B6, 0x0213, 0 },
    //  EB: ex de,hl (M:1 T:4 steps:1)
    { 0x00000002, 0x0218, 0 },
    //  EC: call pe,nn (M:6 T:17 steps:10)
    { 0x000076B6, 0x0219, 0 },
    //  ED: ed prefix (M:1 T:4 steps:1)
    { 0x00000002, 0x0223, 0 },
    //  EE: xor n (M:2 T:7 steps:3)
    { 0x00000016, 0x0224, Z80_OPSTATE_FLAGS_IMM8 },
    //  EF: rst 28h (M:1 T:4 steps:1)
    { 0x00000002, 0x0227, 0 },
    //  F0: ret p (M:4 T:11 steps:6)
    { 0x0000016E, 0x0228, 0 },
    //  F1: pop af (M:3 T:10 steps:5)
    { 0x000000B6, 0x022E, 0 },
    //  F2: jp p,nn (M:3 T:10 steps:5)
    { 0x000000B6, 0x0233, 0 },
    //  F3: di (M:1 T:4 steps:1)
    { 0x00000002, 0x0238, 0 },
    //  F4: call p,nn (M:6 T:17 steps:10)
    { 0x000076B6, 0x0239, 0 },
    //  F5: push af (M:3 T:11 steps:5)
    { 0x000001D8, 0x0243, 0 },
    //  F6: or n (M:2 T:7 steps:3)
    { 0x00000016, 0x0248, Z80_OPSTATE_FLAGS_IMM8 },
    //  F7: rst 30h (M:1 T:4 steps:1)
    { 0x00000002, 0x024B, 0 },
    //  F8: ret m (M:4 T:11 steps:6)
    { 0x0000016E, 0x024C, 0 },
    //  F9: ld sp,hl (M:2 T:6 steps:2)
    { 0x0000000A, 0x0252, 0 },
    //  FA: jp m,nn (M:3 T:10 steps:5)
    { 0x000000B6, 0x0254, 0 },
    //  FB: ei (M:1 T:4 steps:1)
    { 0x00000002, 0x0259, 0 },
    //  FC: call m,nn (M:6 T:17 steps:10)
    { 0x000076B6, 0x025A, 0 },
    //  FD: fd prefix (M:1 T:4 steps:1)
    { 0x00000002, 0x0264, 0 },
    //  FE: cp n (M:2 T:7 steps:3)
    { 0x00000016, 0x0265, Z80_OPSTATE_FLAGS_IMM8 },
    //  FF: rst 38h (M:1 T:4 steps:1)
    { 0x00000002, 0x0268, 0 },
    // ED 00: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 01: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 02: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 03: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 04: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 05: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 06: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 07: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 08: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 09: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 0A: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 0B: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 0C: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 0D: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 0E: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 0F: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 10: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 11: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 12: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 13: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 14: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 15: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 16: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 17: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 18: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 19: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 1A: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 1B: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 1C: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 1D: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 1E: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 1F: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 20: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 21: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 22: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 23: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 24: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 25: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 26: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 27: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 28: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 29: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 2A: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 2B: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 2C: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 2D: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 2E: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 2F: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 30: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 31: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 32: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 33: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 34: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 35: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 36: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 37: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 38: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 39: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 3A: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 3B: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 3C: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 3D: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 3E: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 3F: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 40: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 41: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 42: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 43: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 44: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 45: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 46: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 47: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 48: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 49: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 4A: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 4B: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 4C: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 4D: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 4E: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 4F: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 50: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 51: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 52: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 53: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 54: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 55: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 56: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 57: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 58: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 59: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 5A: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 5B: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 5C: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 5D: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 5E: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 5F: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 60: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 61: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 62: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 63: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 64: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 65: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 66: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 67: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 68: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 69: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 6A: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 6B: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 6C: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 6D: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 6E: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 6F: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 70: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 71: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 72: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 73: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 74: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 75: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 76: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 77: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 78: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 79: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 7A: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 7B: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 7C: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 7D: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 7E: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 7F: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 80: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 81: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 82: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 83: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 84: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 85: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 86: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 87: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 88: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 89: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 8A: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 8B: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 8C: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 8D: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 8E: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 8F: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 90: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 91: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 92: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 93: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 94: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 95: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 96: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 97: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 98: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 99: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 9A: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 9B: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 9C: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 9D: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 9E: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED 9F: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED A0: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED A1: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED A2: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED A3: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED A4: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED A5: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED A6: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED A7: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED A8: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED A9: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED AA: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED AB: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED AC: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED AD: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED AE: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED AF: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED B0: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED B1: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED B2: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED B3: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED B4: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED B5: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED B6: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED B7: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED B8: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED B9: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED BA: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED BB: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED BC: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED BD: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED BE: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED BF: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED C0: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED C1: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED C2: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED C3: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED C4: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED C5: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED C6: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED C7: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED C8: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED C9: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED CA: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED CB: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED CC: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED CD: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED CE: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED CF: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED D0: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED D1: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED D2: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED D3: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED D4: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED D5: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED D6: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED D7: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED D8: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED D9: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED DA: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED DB: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED DC: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED DD: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED DE: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED DF: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED E0: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED E1: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED E2: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED E3: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED E4: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED E5: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED E6: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED E7: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED E8: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED E9: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED EA: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED EB: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED EC: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED ED: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED EE: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED EF: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED F0: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED F1: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED F2: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED F3: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED F4: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED F5: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED F6: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED F7: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED F8: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED F9: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED FA: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED FB: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED FC: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED FD: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED FE: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // ED FF: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 00: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 01: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 02: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 03: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 04: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 05: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 06: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 07: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 08: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 09: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 0A: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 0B: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 0C: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 0D: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 0E: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 0F: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 10: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 11: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 12: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 13: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 14: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 15: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 16: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 17: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 18: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 19: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 1A: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 1B: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 1C: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 1D: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 1E: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 1F: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 20: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 21: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 22: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 23: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 24: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 25: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 26: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 27: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 28: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 29: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 2A: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 2B: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 2C: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 2D: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 2E: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 2F: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 30: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 31: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 32: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 33: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 34: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 35: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 36: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 37: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 38: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 39: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 3A: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 3B: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 3C: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 3D: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 3E: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 3F: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 40: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 41: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 42: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 43: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 44: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 45: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 46: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 47: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 48: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 49: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 4A: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 4B: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 4C: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 4D: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 4E: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 4F: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 50: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 51: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 52: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 53: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 54: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 55: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 56: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 57: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 58: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 59: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 5A: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 5B: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 5C: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 5D: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 5E: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 5F: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 60: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 61: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 62: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 63: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 64: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 65: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 66: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 67: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 68: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 69: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 6A: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 6B: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 6C: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 6D: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 6E: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 6F: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 70: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 71: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 72: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 73: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 74: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 75: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 76: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 77: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 78: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 79: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 7A: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 7B: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 7C: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 7D: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 7E: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 7F: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 80: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 81: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 82: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 83: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 84: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 85: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 86: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 87: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 88: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 89: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 8A: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 8B: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 8C: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 8D: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 8E: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 8F: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 90: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 91: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 92: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 93: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 94: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 95: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 96: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 97: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 98: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 99: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 9A: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 9B: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 9C: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 9D: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 9E: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB 9F: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB A0: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB A1: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB A2: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB A3: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB A4: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB A5: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB A6: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB A7: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB A8: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB A9: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB AA: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB AB: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB AC: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB AD: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB AE: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB AF: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB B0: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB B1: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB B2: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB B3: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB B4: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB B5: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB B6: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB B7: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB B8: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB B9: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB BA: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB BB: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB BC: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB BD: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB BE: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB BF: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB C0: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB C1: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB C2: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB C3: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB C4: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB C5: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB C6: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB C7: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB C8: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB C9: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB CA: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB CB: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB CC: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB CD: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB CE: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB CF: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB D0: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB D1: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB D2: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB D3: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB D4: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB D5: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB D6: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB D7: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB D8: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB D9: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB DA: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB DB: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB DC: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB DD: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB DE: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB DF: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB E0: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB E1: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB E2: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB E3: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB E4: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB E5: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB E6: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB E7: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB E8: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB E9: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB EA: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB EB: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB EC: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB ED: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB EE: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB EF: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB F0: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB F1: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB F2: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB F3: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB F4: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB F5: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB F6: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB F7: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB F8: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB F9: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB FA: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB FB: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB FC: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB FD: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB FE: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },
    // CB FF: nop (M:1 T:4 steps:1)
    { 0x00000002, 0x0004, 0 },

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
#define _fetch()        pins=z80_fetch(cpu,pins)
#define _fetch_dd()     pins=z80_fetch_prefix(cpu,pins,Z80_PREFIX_DD);
#define _fetch_fd()     pins=z80_fetch_prefix(cpu,pins,Z80_PREFIX_FD);
#define _fetch_ed()     pins=z80_fetch_prefix(cpu,pins,Z80_PREFIX_ED);
#define _fetch_cb()     pins=z80_fetch_prefix(cpu,pins,Z80_PREFIX_CB);
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
                cpu->op.step += cpu->step_offset;

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
            
            //  00: nop (M:1 T:4)
            // -- OVERLAP
            case 0x0005: _fetch(); break;
            
            //  01: ld bc,nn (M:3 T:10)
            // -- M2
            case 0x0006: _wait();_mread(cpu->pc++); break;
            case 0x0007: cpu->c=_gd(); break;
            // -- M3
            case 0x0008: _wait();_mread(cpu->pc++); break;
            case 0x0009: cpu->b=_gd(); break;
            // -- OVERLAP
            case 0x000A: _fetch(); break;
            
            //  02: ld (bc),a (M:2 T:7)
            // -- M2
            case 0x000B: _mwrite(cpu->bc,cpu->a);cpu->wzl=cpu->c+1;cpu->wzh=cpu->a; break;
            case 0x000C: _wait() break;
            // -- OVERLAP
            case 0x000D: _fetch(); break;
            
            //  03: inc bc (M:2 T:6)
            // -- M2 (generic)
            case 0x000E: cpu->bc++; break;
            // -- OVERLAP
            case 0x000F: _fetch(); break;
            
            //  04: inc b (M:1 T:4)
            // -- OVERLAP
            case 0x0010: cpu->b=z80_inc8(cpu,cpu->b);_fetch(); break;
            
            //  05: dec b (M:1 T:4)
            // -- OVERLAP
            case 0x0011: cpu->b=z80_dec8(cpu,cpu->b);_fetch(); break;
            
            //  06: ld b,n (M:2 T:7)
            // -- M2
            case 0x0012: _wait();_mread(cpu->pc++); break;
            case 0x0013: cpu->b=_gd(); break;
            // -- OVERLAP
            case 0x0014: _fetch(); break;
            
            //  07: rlca (M:1 T:4)
            // -- OVERLAP
            case 0x0015: z80_rlca(cpu);_fetch(); break;
            
            //  08: ex af,af' (M:1 T:4)
            // -- OVERLAP
            case 0x0016: z80_ex_af_af2(cpu);_fetch(); break;
            
            //  09: add hl,bc (M:1 T:4)
            // -- OVERLAP
            case 0x0017: _fetch(); break;
            
            //  0A: ld a,(bc) (M:2 T:7)
            // -- M2
            case 0x0018: _wait();_mread(cpu->bc); break;
            case 0x0019: cpu->a=_gd();cpu->wz=cpu->bc+1; break;
            // -- OVERLAP
            case 0x001A: _fetch(); break;
            
            //  0B: dec bc (M:2 T:6)
            // -- M2 (generic)
            case 0x001B: cpu->bc--; break;
            // -- OVERLAP
            case 0x001C: _fetch(); break;
            
            //  0C: inc c (M:1 T:4)
            // -- OVERLAP
            case 0x001D: cpu->c=z80_inc8(cpu,cpu->c);_fetch(); break;
            
            //  0D: dec c (M:1 T:4)
            // -- OVERLAP
            case 0x001E: cpu->c=z80_dec8(cpu,cpu->c);_fetch(); break;
            
            //  0E: ld c,n (M:2 T:7)
            // -- M2
            case 0x001F: _wait();_mread(cpu->pc++); break;
            case 0x0020: cpu->c=_gd(); break;
            // -- OVERLAP
            case 0x0021: _fetch(); break;
            
            //  0F: rrca (M:1 T:4)
            // -- OVERLAP
            case 0x0022: z80_rrca(cpu);_fetch(); break;
            
            //  10: djnz d (M:3 T:13)
            // -- M2
            case 0x0023: _wait();_mread(cpu->pc++); break;
            case 0x0024: cpu->dlatch=_gd();if(--cpu->b==0){z80_skip(cpu,1,7,2);}; break;
            // -- M3 (generic)
            case 0x0025: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;; break;
            // -- OVERLAP
            case 0x0026: _fetch(); break;
            
            //  11: ld de,nn (M:3 T:10)
            // -- M2
            case 0x0027: _wait();_mread(cpu->pc++); break;
            case 0x0028: cpu->e=_gd(); break;
            // -- M3
            case 0x0029: _wait();_mread(cpu->pc++); break;
            case 0x002A: cpu->d=_gd(); break;
            // -- OVERLAP
            case 0x002B: _fetch(); break;
            
            //  12: ld (de),a (M:2 T:7)
            // -- M2
            case 0x002C: _mwrite(cpu->de,cpu->a);cpu->wzl=cpu->e+1;cpu->wzh=cpu->a; break;
            case 0x002D: _wait() break;
            // -- OVERLAP
            case 0x002E: _fetch(); break;
            
            //  13: inc de (M:2 T:6)
            // -- M2 (generic)
            case 0x002F: cpu->de++; break;
            // -- OVERLAP
            case 0x0030: _fetch(); break;
            
            //  14: inc d (M:1 T:4)
            // -- OVERLAP
            case 0x0031: cpu->d=z80_inc8(cpu,cpu->d);_fetch(); break;
            
            //  15: dec d (M:1 T:4)
            // -- OVERLAP
            case 0x0032: cpu->d=z80_dec8(cpu,cpu->d);_fetch(); break;
            
            //  16: ld d,n (M:2 T:7)
            // -- M2
            case 0x0033: _wait();_mread(cpu->pc++); break;
            case 0x0034: cpu->d=_gd(); break;
            // -- OVERLAP
            case 0x0035: _fetch(); break;
            
            //  17: rla (M:1 T:4)
            // -- OVERLAP
            case 0x0036: z80_rla(cpu);_fetch(); break;
            
            //  18: jr d (M:3 T:12)
            // -- M2
            case 0x0037: _wait();_mread(cpu->pc++); break;
            case 0x0038: cpu->dlatch=_gd(); break;
            // -- M3 (generic)
            case 0x0039: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;; break;
            // -- OVERLAP
            case 0x003A: _fetch(); break;
            
            //  19: add hl,de (M:1 T:4)
            // -- OVERLAP
            case 0x003B: _fetch(); break;
            
            //  1A: ld a,(de) (M:2 T:7)
            // -- M2
            case 0x003C: _wait();_mread(cpu->de); break;
            case 0x003D: cpu->a=_gd();cpu->wz=cpu->de+1; break;
            // -- OVERLAP
            case 0x003E: _fetch(); break;
            
            //  1B: dec de (M:2 T:6)
            // -- M2 (generic)
            case 0x003F: cpu->de--; break;
            // -- OVERLAP
            case 0x0040: _fetch(); break;
            
            //  1C: inc e (M:1 T:4)
            // -- OVERLAP
            case 0x0041: cpu->e=z80_inc8(cpu,cpu->e);_fetch(); break;
            
            //  1D: dec e (M:1 T:4)
            // -- OVERLAP
            case 0x0042: cpu->e=z80_dec8(cpu,cpu->e);_fetch(); break;
            
            //  1E: ld e,n (M:2 T:7)
            // -- M2
            case 0x0043: _wait();_mread(cpu->pc++); break;
            case 0x0044: cpu->e=_gd(); break;
            // -- OVERLAP
            case 0x0045: _fetch(); break;
            
            //  1F: rra (M:1 T:4)
            // -- OVERLAP
            case 0x0046: z80_rra(cpu);_fetch(); break;
            
            //  20: jr nz,d (M:3 T:12)
            // -- M2
            case 0x0047: _wait();_mread(cpu->pc++); break;
            case 0x0048: cpu->dlatch=_gd();if(!(_cc_nz)){z80_skip(cpu,1,7,2);}; break;
            // -- M3 (generic)
            case 0x0049: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;; break;
            // -- OVERLAP
            case 0x004A: _fetch(); break;
            
            //  21: ld hl,nn (M:3 T:10)
            // -- M2
            case 0x004B: _wait();_mread(cpu->pc++); break;
            case 0x004C: cpu->hlx[cpu->hlx_idx].l=_gd(); break;
            // -- M3
            case 0x004D: _wait();_mread(cpu->pc++); break;
            case 0x004E: cpu->hlx[cpu->hlx_idx].h=_gd(); break;
            // -- OVERLAP
            case 0x004F: _fetch(); break;
            
            //  22: ld (nn),hl (M:1 T:4)
            // -- OVERLAP
            case 0x0050: _fetch(); break;
            
            //  23: inc hl (M:2 T:6)
            // -- M2 (generic)
            case 0x0051: cpu->hlx[cpu->hlx_idx].hl++; break;
            // -- OVERLAP
            case 0x0052: _fetch(); break;
            
            //  24: inc h (M:1 T:4)
            // -- OVERLAP
            case 0x0053: cpu->hlx[cpu->hlx_idx].h=z80_inc8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); break;
            
            //  25: dec h (M:1 T:4)
            // -- OVERLAP
            case 0x0054: cpu->hlx[cpu->hlx_idx].h=z80_dec8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); break;
            
            //  26: ld h,n (M:2 T:7)
            // -- M2
            case 0x0055: _wait();_mread(cpu->pc++); break;
            case 0x0056: cpu->hlx[cpu->hlx_idx].h=_gd(); break;
            // -- OVERLAP
            case 0x0057: _fetch(); break;
            
            //  27: daa (M:1 T:4)
            // -- OVERLAP
            case 0x0058: z80_daa(cpu);_fetch(); break;
            
            //  28: jr z,d (M:3 T:12)
            // -- M2
            case 0x0059: _wait();_mread(cpu->pc++); break;
            case 0x005A: cpu->dlatch=_gd();if(!(_cc_z)){z80_skip(cpu,1,7,2);}; break;
            // -- M3 (generic)
            case 0x005B: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;; break;
            // -- OVERLAP
            case 0x005C: _fetch(); break;
            
            //  29: add hl,hl (M:1 T:4)
            // -- OVERLAP
            case 0x005D: _fetch(); break;
            
            //  2A: ld hl,(nn) (M:5 T:16)
            // -- M2
            case 0x005E: _wait();_mread(cpu->pc++); break;
            case 0x005F: cpu->wzl=_gd(); break;
            // -- M3
            case 0x0060: _wait();_mread(cpu->pc++); break;
            case 0x0061: cpu->wzh=_gd(); break;
            // -- M4
            case 0x0062: _wait();_mread(cpu->wz++); break;
            case 0x0063: cpu->hlx[cpu->hlx_idx].l=_gd(); break;
            // -- M5
            case 0x0064: _wait();_mread(cpu->wz); break;
            case 0x0065: cpu->hlx[cpu->hlx_idx].h=_gd(); break;
            // -- OVERLAP
            case 0x0066: _fetch(); break;
            
            //  2B: dec hl (M:2 T:6)
            // -- M2 (generic)
            case 0x0067: cpu->hlx[cpu->hlx_idx].hl--; break;
            // -- OVERLAP
            case 0x0068: _fetch(); break;
            
            //  2C: inc l (M:1 T:4)
            // -- OVERLAP
            case 0x0069: cpu->hlx[cpu->hlx_idx].l=z80_inc8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); break;
            
            //  2D: dec l (M:1 T:4)
            // -- OVERLAP
            case 0x006A: cpu->hlx[cpu->hlx_idx].l=z80_dec8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); break;
            
            //  2E: ld l,n (M:2 T:7)
            // -- M2
            case 0x006B: _wait();_mread(cpu->pc++); break;
            case 0x006C: cpu->hlx[cpu->hlx_idx].l=_gd(); break;
            // -- OVERLAP
            case 0x006D: _fetch(); break;
            
            //  2F: cpl (M:1 T:4)
            // -- OVERLAP
            case 0x006E: z80_cpl(cpu);_fetch(); break;
            
            //  30: jr nc,d (M:3 T:12)
            // -- M2
            case 0x006F: _wait();_mread(cpu->pc++); break;
            case 0x0070: cpu->dlatch=_gd();if(!(_cc_nc)){z80_skip(cpu,1,7,2);}; break;
            // -- M3 (generic)
            case 0x0071: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;; break;
            // -- OVERLAP
            case 0x0072: _fetch(); break;
            
            //  31: ld sp,nn (M:3 T:10)
            // -- M2
            case 0x0073: _wait();_mread(cpu->pc++); break;
            case 0x0074: cpu->spl=_gd(); break;
            // -- M3
            case 0x0075: _wait();_mread(cpu->pc++); break;
            case 0x0076: cpu->sph=_gd(); break;
            // -- OVERLAP
            case 0x0077: _fetch(); break;
            
            //  32: ld (nn),a (M:4 T:13)
            // -- M2
            case 0x0078: _wait();_mread(cpu->pc++); break;
            case 0x0079: cpu->wzl=_gd(); break;
            // -- M3
            case 0x007A: _wait();_mread(cpu->pc++); break;
            case 0x007B: cpu->wzh=_gd(); break;
            // -- M4
            case 0x007C: _mwrite(cpu->wz++,cpu->a);cpu->wzh=cpu->a; break;
            case 0x007D: _wait() break;
            // -- OVERLAP
            case 0x007E: _fetch(); break;
            
            //  33: inc sp (M:2 T:6)
            // -- M2 (generic)
            case 0x007F: cpu->sp++; break;
            // -- OVERLAP
            case 0x0080: _fetch(); break;
            
            //  34: inc (hl) (M:3 T:11)
            // -- M2
            case 0x0081: _wait();_mread(cpu->addr); break;
            case 0x0082: cpu->dlatch=_gd();cpu->dlatch=z80_inc8(cpu,cpu->dlatch); break;
            // -- M3
            case 0x0083: _mwrite(cpu->addr,cpu->dlatch); break;
            case 0x0084: _wait() break;
            // -- OVERLAP
            case 0x0085: _fetch(); break;
            
            //  35: dec (hl) (M:3 T:11)
            // -- M2
            case 0x0086: _wait();_mread(cpu->addr); break;
            case 0x0087: cpu->dlatch=_gd();cpu->dlatch=z80_dec8(cpu,cpu->dlatch); break;
            // -- M3
            case 0x0088: _mwrite(cpu->addr,cpu->dlatch); break;
            case 0x0089: _wait() break;
            // -- OVERLAP
            case 0x008A: _fetch(); break;
            
            //  36: ld (hl),n (M:3 T:10)
            // -- M2
            case 0x008B: _wait();_mread(cpu->pc++); break;
            case 0x008C: cpu->dlatch=_gd(); break;
            // -- M3
            case 0x008D: _mwrite(cpu->addr,cpu->dlatch); break;
            case 0x008E: _wait() break;
            // -- OVERLAP
            case 0x008F: _fetch(); break;
            
            //  37: scf (M:1 T:4)
            // -- OVERLAP
            case 0x0090: z80_scf(cpu);_fetch(); break;
            
            //  38: jr c,d (M:3 T:12)
            // -- M2
            case 0x0091: _wait();_mread(cpu->pc++); break;
            case 0x0092: cpu->dlatch=_gd();if(!(_cc_c)){z80_skip(cpu,1,7,2);}; break;
            // -- M3 (generic)
            case 0x0093: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;; break;
            // -- OVERLAP
            case 0x0094: _fetch(); break;
            
            //  39: add hl,sp (M:1 T:4)
            // -- OVERLAP
            case 0x0095: _fetch(); break;
            
            //  3A: ld a,(nn) (M:4 T:13)
            // -- M2
            case 0x0096: _wait();_mread(cpu->pc++); break;
            case 0x0097: cpu->wzl=_gd(); break;
            // -- M3
            case 0x0098: _wait();_mread(cpu->pc++); break;
            case 0x0099: cpu->wzh=_gd(); break;
            // -- M4
            case 0x009A: _wait();_mread(cpu->wz++); break;
            case 0x009B: cpu->a=_gd(); break;
            // -- OVERLAP
            case 0x009C: _fetch(); break;
            
            //  3B: dec sp (M:2 T:6)
            // -- M2 (generic)
            case 0x009D: cpu->sp--; break;
            // -- OVERLAP
            case 0x009E: _fetch(); break;
            
            //  3C: inc a (M:1 T:4)
            // -- OVERLAP
            case 0x009F: cpu->a=z80_inc8(cpu,cpu->a);_fetch(); break;
            
            //  3D: dec a (M:1 T:4)
            // -- OVERLAP
            case 0x00A0: cpu->a=z80_dec8(cpu,cpu->a);_fetch(); break;
            
            //  3E: ld a,n (M:2 T:7)
            // -- M2
            case 0x00A1: _wait();_mread(cpu->pc++); break;
            case 0x00A2: cpu->a=_gd(); break;
            // -- OVERLAP
            case 0x00A3: _fetch(); break;
            
            //  3F: ccf (M:1 T:4)
            // -- OVERLAP
            case 0x00A4: z80_ccf(cpu);_fetch(); break;
            
            //  40: ld b,b (M:1 T:4)
            // -- OVERLAP
            case 0x00A5: cpu->b=cpu->b;_fetch(); break;
            
            //  41: ld b,c (M:1 T:4)
            // -- OVERLAP
            case 0x00A6: cpu->b=cpu->c;_fetch(); break;
            
            //  42: ld b,d (M:1 T:4)
            // -- OVERLAP
            case 0x00A7: cpu->b=cpu->d;_fetch(); break;
            
            //  43: ld b,e (M:1 T:4)
            // -- OVERLAP
            case 0x00A8: cpu->b=cpu->e;_fetch(); break;
            
            //  44: ld b,h (M:1 T:4)
            // -- OVERLAP
            case 0x00A9: cpu->b=cpu->hlx[cpu->hlx_idx].h;_fetch(); break;
            
            //  45: ld b,l (M:1 T:4)
            // -- OVERLAP
            case 0x00AA: cpu->b=cpu->hlx[cpu->hlx_idx].l;_fetch(); break;
            
            //  46: ld b,(hl) (M:2 T:7)
            // -- M2
            case 0x00AB: _wait();_mread(cpu->addr); break;
            case 0x00AC: cpu->b=_gd(); break;
            // -- OVERLAP
            case 0x00AD: _fetch(); break;
            
            //  47: ld b,a (M:1 T:4)
            // -- OVERLAP
            case 0x00AE: cpu->b=cpu->a;_fetch(); break;
            
            //  48: ld c,b (M:1 T:4)
            // -- OVERLAP
            case 0x00AF: cpu->c=cpu->b;_fetch(); break;
            
            //  49: ld c,c (M:1 T:4)
            // -- OVERLAP
            case 0x00B0: cpu->c=cpu->c;_fetch(); break;
            
            //  4A: ld c,d (M:1 T:4)
            // -- OVERLAP
            case 0x00B1: cpu->c=cpu->d;_fetch(); break;
            
            //  4B: ld c,e (M:1 T:4)
            // -- OVERLAP
            case 0x00B2: cpu->c=cpu->e;_fetch(); break;
            
            //  4C: ld c,h (M:1 T:4)
            // -- OVERLAP
            case 0x00B3: cpu->c=cpu->hlx[cpu->hlx_idx].h;_fetch(); break;
            
            //  4D: ld c,l (M:1 T:4)
            // -- OVERLAP
            case 0x00B4: cpu->c=cpu->hlx[cpu->hlx_idx].l;_fetch(); break;
            
            //  4E: ld c,(hl) (M:2 T:7)
            // -- M2
            case 0x00B5: _wait();_mread(cpu->addr); break;
            case 0x00B6: cpu->c=_gd(); break;
            // -- OVERLAP
            case 0x00B7: _fetch(); break;
            
            //  4F: ld c,a (M:1 T:4)
            // -- OVERLAP
            case 0x00B8: cpu->c=cpu->a;_fetch(); break;
            
            //  50: ld d,b (M:1 T:4)
            // -- OVERLAP
            case 0x00B9: cpu->d=cpu->b;_fetch(); break;
            
            //  51: ld d,c (M:1 T:4)
            // -- OVERLAP
            case 0x00BA: cpu->d=cpu->c;_fetch(); break;
            
            //  52: ld d,d (M:1 T:4)
            // -- OVERLAP
            case 0x00BB: cpu->d=cpu->d;_fetch(); break;
            
            //  53: ld d,e (M:1 T:4)
            // -- OVERLAP
            case 0x00BC: cpu->d=cpu->e;_fetch(); break;
            
            //  54: ld d,h (M:1 T:4)
            // -- OVERLAP
            case 0x00BD: cpu->d=cpu->hlx[cpu->hlx_idx].h;_fetch(); break;
            
            //  55: ld d,l (M:1 T:4)
            // -- OVERLAP
            case 0x00BE: cpu->d=cpu->hlx[cpu->hlx_idx].l;_fetch(); break;
            
            //  56: ld d,(hl) (M:2 T:7)
            // -- M2
            case 0x00BF: _wait();_mread(cpu->addr); break;
            case 0x00C0: cpu->d=_gd(); break;
            // -- OVERLAP
            case 0x00C1: _fetch(); break;
            
            //  57: ld d,a (M:1 T:4)
            // -- OVERLAP
            case 0x00C2: cpu->d=cpu->a;_fetch(); break;
            
            //  58: ld e,b (M:1 T:4)
            // -- OVERLAP
            case 0x00C3: cpu->e=cpu->b;_fetch(); break;
            
            //  59: ld e,c (M:1 T:4)
            // -- OVERLAP
            case 0x00C4: cpu->e=cpu->c;_fetch(); break;
            
            //  5A: ld e,d (M:1 T:4)
            // -- OVERLAP
            case 0x00C5: cpu->e=cpu->d;_fetch(); break;
            
            //  5B: ld e,e (M:1 T:4)
            // -- OVERLAP
            case 0x00C6: cpu->e=cpu->e;_fetch(); break;
            
            //  5C: ld e,h (M:1 T:4)
            // -- OVERLAP
            case 0x00C7: cpu->e=cpu->hlx[cpu->hlx_idx].h;_fetch(); break;
            
            //  5D: ld e,l (M:1 T:4)
            // -- OVERLAP
            case 0x00C8: cpu->e=cpu->hlx[cpu->hlx_idx].l;_fetch(); break;
            
            //  5E: ld e,(hl) (M:2 T:7)
            // -- M2
            case 0x00C9: _wait();_mread(cpu->addr); break;
            case 0x00CA: cpu->e=_gd(); break;
            // -- OVERLAP
            case 0x00CB: _fetch(); break;
            
            //  5F: ld e,a (M:1 T:4)
            // -- OVERLAP
            case 0x00CC: cpu->e=cpu->a;_fetch(); break;
            
            //  60: ld h,b (M:1 T:4)
            // -- OVERLAP
            case 0x00CD: cpu->hlx[cpu->hlx_idx].h=cpu->b;_fetch(); break;
            
            //  61: ld h,c (M:1 T:4)
            // -- OVERLAP
            case 0x00CE: cpu->hlx[cpu->hlx_idx].h=cpu->c;_fetch(); break;
            
            //  62: ld h,d (M:1 T:4)
            // -- OVERLAP
            case 0x00CF: cpu->hlx[cpu->hlx_idx].h=cpu->d;_fetch(); break;
            
            //  63: ld h,e (M:1 T:4)
            // -- OVERLAP
            case 0x00D0: cpu->hlx[cpu->hlx_idx].h=cpu->e;_fetch(); break;
            
            //  64: ld h,h (M:1 T:4)
            // -- OVERLAP
            case 0x00D1: cpu->hlx[cpu->hlx_idx].h=cpu->hlx[cpu->hlx_idx].h;_fetch(); break;
            
            //  65: ld h,l (M:1 T:4)
            // -- OVERLAP
            case 0x00D2: cpu->hlx[cpu->hlx_idx].h=cpu->hlx[cpu->hlx_idx].l;_fetch(); break;
            
            //  66: ld h,(hl) (M:2 T:7)
            // -- M2
            case 0x00D3: _wait();_mread(cpu->addr); break;
            case 0x00D4: cpu->h=_gd(); break;
            // -- OVERLAP
            case 0x00D5: _fetch(); break;
            
            //  67: ld h,a (M:1 T:4)
            // -- OVERLAP
            case 0x00D6: cpu->hlx[cpu->hlx_idx].h=cpu->a;_fetch(); break;
            
            //  68: ld l,b (M:1 T:4)
            // -- OVERLAP
            case 0x00D7: cpu->hlx[cpu->hlx_idx].l=cpu->b;_fetch(); break;
            
            //  69: ld l,c (M:1 T:4)
            // -- OVERLAP
            case 0x00D8: cpu->hlx[cpu->hlx_idx].l=cpu->c;_fetch(); break;
            
            //  6A: ld l,d (M:1 T:4)
            // -- OVERLAP
            case 0x00D9: cpu->hlx[cpu->hlx_idx].l=cpu->d;_fetch(); break;
            
            //  6B: ld l,e (M:1 T:4)
            // -- OVERLAP
            case 0x00DA: cpu->hlx[cpu->hlx_idx].l=cpu->e;_fetch(); break;
            
            //  6C: ld l,h (M:1 T:4)
            // -- OVERLAP
            case 0x00DB: cpu->hlx[cpu->hlx_idx].l=cpu->hlx[cpu->hlx_idx].h;_fetch(); break;
            
            //  6D: ld l,l (M:1 T:4)
            // -- OVERLAP
            case 0x00DC: cpu->hlx[cpu->hlx_idx].l=cpu->hlx[cpu->hlx_idx].l;_fetch(); break;
            
            //  6E: ld l,(hl) (M:2 T:7)
            // -- M2
            case 0x00DD: _wait();_mread(cpu->addr); break;
            case 0x00DE: cpu->l=_gd(); break;
            // -- OVERLAP
            case 0x00DF: _fetch(); break;
            
            //  6F: ld l,a (M:1 T:4)
            // -- OVERLAP
            case 0x00E0: cpu->hlx[cpu->hlx_idx].l=cpu->a;_fetch(); break;
            
            //  70: ld (hl),b (M:2 T:7)
            // -- M2
            case 0x00E1: _mwrite(cpu->addr,cpu->b); break;
            case 0x00E2: _wait() break;
            // -- OVERLAP
            case 0x00E3: _fetch(); break;
            
            //  71: ld (hl),c (M:2 T:7)
            // -- M2
            case 0x00E4: _mwrite(cpu->addr,cpu->c); break;
            case 0x00E5: _wait() break;
            // -- OVERLAP
            case 0x00E6: _fetch(); break;
            
            //  72: ld (hl),d (M:2 T:7)
            // -- M2
            case 0x00E7: _mwrite(cpu->addr,cpu->d); break;
            case 0x00E8: _wait() break;
            // -- OVERLAP
            case 0x00E9: _fetch(); break;
            
            //  73: ld (hl),e (M:2 T:7)
            // -- M2
            case 0x00EA: _mwrite(cpu->addr,cpu->e); break;
            case 0x00EB: _wait() break;
            // -- OVERLAP
            case 0x00EC: _fetch(); break;
            
            //  74: ld (hl),h (M:2 T:7)
            // -- M2
            case 0x00ED: _mwrite(cpu->addr,cpu->h); break;
            case 0x00EE: _wait() break;
            // -- OVERLAP
            case 0x00EF: _fetch(); break;
            
            //  75: ld (hl),l (M:2 T:7)
            // -- M2
            case 0x00F0: _mwrite(cpu->addr,cpu->l); break;
            case 0x00F1: _wait() break;
            // -- OVERLAP
            case 0x00F2: _fetch(); break;
            
            //  76: halt (M:1 T:4)
            // -- OVERLAP
            case 0x00F3: pins=z80_halt(cpu,pins);_fetch(); break;
            
            //  77: ld (hl),a (M:2 T:7)
            // -- M2
            case 0x00F4: _mwrite(cpu->addr,cpu->a); break;
            case 0x00F5: _wait() break;
            // -- OVERLAP
            case 0x00F6: _fetch(); break;
            
            //  78: ld a,b (M:1 T:4)
            // -- OVERLAP
            case 0x00F7: cpu->a=cpu->b;_fetch(); break;
            
            //  79: ld a,c (M:1 T:4)
            // -- OVERLAP
            case 0x00F8: cpu->a=cpu->c;_fetch(); break;
            
            //  7A: ld a,d (M:1 T:4)
            // -- OVERLAP
            case 0x00F9: cpu->a=cpu->d;_fetch(); break;
            
            //  7B: ld a,e (M:1 T:4)
            // -- OVERLAP
            case 0x00FA: cpu->a=cpu->e;_fetch(); break;
            
            //  7C: ld a,h (M:1 T:4)
            // -- OVERLAP
            case 0x00FB: cpu->a=cpu->hlx[cpu->hlx_idx].h;_fetch(); break;
            
            //  7D: ld a,l (M:1 T:4)
            // -- OVERLAP
            case 0x00FC: cpu->a=cpu->hlx[cpu->hlx_idx].l;_fetch(); break;
            
            //  7E: ld a,(hl) (M:2 T:7)
            // -- M2
            case 0x00FD: _wait();_mread(cpu->addr); break;
            case 0x00FE: cpu->a=_gd(); break;
            // -- OVERLAP
            case 0x00FF: _fetch(); break;
            
            //  7F: ld a,a (M:1 T:4)
            // -- OVERLAP
            case 0x0100: cpu->a=cpu->a;_fetch(); break;
            
            //  80: add b (M:1 T:4)
            // -- OVERLAP
            case 0x0101: z80_add8(cpu,cpu->b);_fetch(); break;
            
            //  81: add c (M:1 T:4)
            // -- OVERLAP
            case 0x0102: z80_add8(cpu,cpu->c);_fetch(); break;
            
            //  82: add d (M:1 T:4)
            // -- OVERLAP
            case 0x0103: z80_add8(cpu,cpu->d);_fetch(); break;
            
            //  83: add e (M:1 T:4)
            // -- OVERLAP
            case 0x0104: z80_add8(cpu,cpu->e);_fetch(); break;
            
            //  84: add h (M:1 T:4)
            // -- OVERLAP
            case 0x0105: z80_add8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); break;
            
            //  85: add l (M:1 T:4)
            // -- OVERLAP
            case 0x0106: z80_add8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); break;
            
            //  86: add (hl) (M:2 T:7)
            // -- M2
            case 0x0107: _wait();_mread(cpu->addr); break;
            case 0x0108: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x0109: z80_add8(cpu,cpu->dlatch);_fetch(); break;
            
            //  87: add a (M:1 T:4)
            // -- OVERLAP
            case 0x010A: z80_add8(cpu,cpu->a);_fetch(); break;
            
            //  88: adc b (M:1 T:4)
            // -- OVERLAP
            case 0x010B: z80_adc8(cpu,cpu->b);_fetch(); break;
            
            //  89: adc c (M:1 T:4)
            // -- OVERLAP
            case 0x010C: z80_adc8(cpu,cpu->c);_fetch(); break;
            
            //  8A: adc d (M:1 T:4)
            // -- OVERLAP
            case 0x010D: z80_adc8(cpu,cpu->d);_fetch(); break;
            
            //  8B: adc e (M:1 T:4)
            // -- OVERLAP
            case 0x010E: z80_adc8(cpu,cpu->e);_fetch(); break;
            
            //  8C: adc h (M:1 T:4)
            // -- OVERLAP
            case 0x010F: z80_adc8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); break;
            
            //  8D: adc l (M:1 T:4)
            // -- OVERLAP
            case 0x0110: z80_adc8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); break;
            
            //  8E: adc (hl) (M:2 T:7)
            // -- M2
            case 0x0111: _wait();_mread(cpu->addr); break;
            case 0x0112: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x0113: z80_adc8(cpu,cpu->dlatch);_fetch(); break;
            
            //  8F: adc a (M:1 T:4)
            // -- OVERLAP
            case 0x0114: z80_adc8(cpu,cpu->a);_fetch(); break;
            
            //  90: sub b (M:1 T:4)
            // -- OVERLAP
            case 0x0115: z80_sub8(cpu,cpu->b);_fetch(); break;
            
            //  91: sub c (M:1 T:4)
            // -- OVERLAP
            case 0x0116: z80_sub8(cpu,cpu->c);_fetch(); break;
            
            //  92: sub d (M:1 T:4)
            // -- OVERLAP
            case 0x0117: z80_sub8(cpu,cpu->d);_fetch(); break;
            
            //  93: sub e (M:1 T:4)
            // -- OVERLAP
            case 0x0118: z80_sub8(cpu,cpu->e);_fetch(); break;
            
            //  94: sub h (M:1 T:4)
            // -- OVERLAP
            case 0x0119: z80_sub8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); break;
            
            //  95: sub l (M:1 T:4)
            // -- OVERLAP
            case 0x011A: z80_sub8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); break;
            
            //  96: sub (hl) (M:2 T:7)
            // -- M2
            case 0x011B: _wait();_mread(cpu->addr); break;
            case 0x011C: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x011D: z80_sub8(cpu,cpu->dlatch);_fetch(); break;
            
            //  97: sub a (M:1 T:4)
            // -- OVERLAP
            case 0x011E: z80_sub8(cpu,cpu->a);_fetch(); break;
            
            //  98: sbc b (M:1 T:4)
            // -- OVERLAP
            case 0x011F: z80_sbc8(cpu,cpu->b);_fetch(); break;
            
            //  99: sbc c (M:1 T:4)
            // -- OVERLAP
            case 0x0120: z80_sbc8(cpu,cpu->c);_fetch(); break;
            
            //  9A: sbc d (M:1 T:4)
            // -- OVERLAP
            case 0x0121: z80_sbc8(cpu,cpu->d);_fetch(); break;
            
            //  9B: sbc e (M:1 T:4)
            // -- OVERLAP
            case 0x0122: z80_sbc8(cpu,cpu->e);_fetch(); break;
            
            //  9C: sbc h (M:1 T:4)
            // -- OVERLAP
            case 0x0123: z80_sbc8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); break;
            
            //  9D: sbc l (M:1 T:4)
            // -- OVERLAP
            case 0x0124: z80_sbc8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); break;
            
            //  9E: sbc (hl) (M:2 T:7)
            // -- M2
            case 0x0125: _wait();_mread(cpu->addr); break;
            case 0x0126: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x0127: z80_sbc8(cpu,cpu->dlatch);_fetch(); break;
            
            //  9F: sbc a (M:1 T:4)
            // -- OVERLAP
            case 0x0128: z80_sbc8(cpu,cpu->a);_fetch(); break;
            
            //  A0: and b (M:1 T:4)
            // -- OVERLAP
            case 0x0129: z80_and8(cpu,cpu->b);_fetch(); break;
            
            //  A1: and c (M:1 T:4)
            // -- OVERLAP
            case 0x012A: z80_and8(cpu,cpu->c);_fetch(); break;
            
            //  A2: and d (M:1 T:4)
            // -- OVERLAP
            case 0x012B: z80_and8(cpu,cpu->d);_fetch(); break;
            
            //  A3: and e (M:1 T:4)
            // -- OVERLAP
            case 0x012C: z80_and8(cpu,cpu->e);_fetch(); break;
            
            //  A4: and h (M:1 T:4)
            // -- OVERLAP
            case 0x012D: z80_and8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); break;
            
            //  A5: and l (M:1 T:4)
            // -- OVERLAP
            case 0x012E: z80_and8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); break;
            
            //  A6: and (hl) (M:2 T:7)
            // -- M2
            case 0x012F: _wait();_mread(cpu->addr); break;
            case 0x0130: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x0131: z80_and8(cpu,cpu->dlatch);_fetch(); break;
            
            //  A7: and a (M:1 T:4)
            // -- OVERLAP
            case 0x0132: z80_and8(cpu,cpu->a);_fetch(); break;
            
            //  A8: xor b (M:1 T:4)
            // -- OVERLAP
            case 0x0133: z80_xor8(cpu,cpu->b);_fetch(); break;
            
            //  A9: xor c (M:1 T:4)
            // -- OVERLAP
            case 0x0134: z80_xor8(cpu,cpu->c);_fetch(); break;
            
            //  AA: xor d (M:1 T:4)
            // -- OVERLAP
            case 0x0135: z80_xor8(cpu,cpu->d);_fetch(); break;
            
            //  AB: xor e (M:1 T:4)
            // -- OVERLAP
            case 0x0136: z80_xor8(cpu,cpu->e);_fetch(); break;
            
            //  AC: xor h (M:1 T:4)
            // -- OVERLAP
            case 0x0137: z80_xor8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); break;
            
            //  AD: xor l (M:1 T:4)
            // -- OVERLAP
            case 0x0138: z80_xor8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); break;
            
            //  AE: xor (hl) (M:2 T:7)
            // -- M2
            case 0x0139: _wait();_mread(cpu->addr); break;
            case 0x013A: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x013B: z80_xor8(cpu,cpu->dlatch);_fetch(); break;
            
            //  AF: xor a (M:1 T:4)
            // -- OVERLAP
            case 0x013C: z80_xor8(cpu,cpu->a);_fetch(); break;
            
            //  B0: or b (M:1 T:4)
            // -- OVERLAP
            case 0x013D: z80_or8(cpu,cpu->b);_fetch(); break;
            
            //  B1: or c (M:1 T:4)
            // -- OVERLAP
            case 0x013E: z80_or8(cpu,cpu->c);_fetch(); break;
            
            //  B2: or d (M:1 T:4)
            // -- OVERLAP
            case 0x013F: z80_or8(cpu,cpu->d);_fetch(); break;
            
            //  B3: or e (M:1 T:4)
            // -- OVERLAP
            case 0x0140: z80_or8(cpu,cpu->e);_fetch(); break;
            
            //  B4: or h (M:1 T:4)
            // -- OVERLAP
            case 0x0141: z80_or8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); break;
            
            //  B5: or l (M:1 T:4)
            // -- OVERLAP
            case 0x0142: z80_or8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); break;
            
            //  B6: or (hl) (M:2 T:7)
            // -- M2
            case 0x0143: _wait();_mread(cpu->addr); break;
            case 0x0144: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x0145: z80_or8(cpu,cpu->dlatch);_fetch(); break;
            
            //  B7: or a (M:1 T:4)
            // -- OVERLAP
            case 0x0146: z80_or8(cpu,cpu->a);_fetch(); break;
            
            //  B8: cp b (M:1 T:4)
            // -- OVERLAP
            case 0x0147: z80_cp8(cpu,cpu->b);_fetch(); break;
            
            //  B9: cp c (M:1 T:4)
            // -- OVERLAP
            case 0x0148: z80_cp8(cpu,cpu->c);_fetch(); break;
            
            //  BA: cp d (M:1 T:4)
            // -- OVERLAP
            case 0x0149: z80_cp8(cpu,cpu->d);_fetch(); break;
            
            //  BB: cp e (M:1 T:4)
            // -- OVERLAP
            case 0x014A: z80_cp8(cpu,cpu->e);_fetch(); break;
            
            //  BC: cp h (M:1 T:4)
            // -- OVERLAP
            case 0x014B: z80_cp8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); break;
            
            //  BD: cp l (M:1 T:4)
            // -- OVERLAP
            case 0x014C: z80_cp8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); break;
            
            //  BE: cp (hl) (M:2 T:7)
            // -- M2
            case 0x014D: _wait();_mread(cpu->addr); break;
            case 0x014E: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x014F: z80_cp8(cpu,cpu->dlatch);_fetch(); break;
            
            //  BF: cp a (M:1 T:4)
            // -- OVERLAP
            case 0x0150: z80_cp8(cpu,cpu->a);_fetch(); break;
            
            //  C0: ret nz (M:4 T:11)
            // -- M2 (generic)
            case 0x0151: if(!_cc_nz){z80_skip(cpu,4,7,1);}; break;
            // -- M3
            case 0x0152: _wait();_mread(cpu->sp++); break;
            case 0x0153: cpu->wzl=_gd(); break;
            // -- M4
            case 0x0154: _wait();_mread(cpu->sp++); break;
            case 0x0155: cpu->wzh=_gd();cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x0156: _fetch(); break;
            
            //  C1: pop bc (M:3 T:10)
            // -- M2
            case 0x0157: _wait();_mread(cpu->sp++); break;
            case 0x0158: cpu->c=_gd(); break;
            // -- M3
            case 0x0159: _wait();_mread(cpu->sp++); break;
            case 0x015A: cpu->b=_gd(); break;
            // -- OVERLAP
            case 0x015B: _fetch(); break;
            
            //  C2: jp nz,nn (M:3 T:10)
            // -- M2
            case 0x015C: _wait();_mread(cpu->pc++); break;
            case 0x015D: cpu->wzl=_gd(); break;
            // -- M3
            case 0x015E: _wait();_mread(cpu->pc++); break;
            case 0x015F: cpu->wzh=_gd();if(_cc_nz){cpu->pc=cpu->wz;}; break;
            // -- OVERLAP
            case 0x0160: _fetch(); break;
            
            //  C3: jp nn (M:3 T:10)
            // -- M2
            case 0x0161: _wait();_mread(cpu->pc++); break;
            case 0x0162: cpu->wzl=_gd(); break;
            // -- M3
            case 0x0163: _wait();_mread(cpu->pc++); break;
            case 0x0164: cpu->wzh=_gd();cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x0165: _fetch(); break;
            
            //  C4: call nz,nn (M:6 T:17)
            // -- M2
            case 0x0166: _wait();_mread(cpu->pc++); break;
            case 0x0167: cpu->wzl=_gd(); break;
            // -- M3
            case 0x0168: _wait();_mread(cpu->pc++); break;
            case 0x0169: cpu->wzh=_gd();if (!_cc_nz){z80_skip(cpu,5,9,2);}; break;
            // -- M4 (generic)
            case 0x016A:  break;
            // -- M5
            case 0x016B: _mwrite(--cpu->sp,cpu->pch); break;
            case 0x016C: _wait() break;
            // -- M6
            case 0x016D: _mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz; break;
            case 0x016E: _wait() break;
            // -- OVERLAP
            case 0x016F: _fetch(); break;
            
            //  C5: push bc (M:3 T:11)
            // -- M2
            case 0x0170: _mwrite(--cpu->sp,cpu->b); break;
            case 0x0171: _wait() break;
            // -- M3
            case 0x0172: _mwrite(--cpu->sp,cpu->c); break;
            case 0x0173: _wait() break;
            // -- OVERLAP
            case 0x0174: _fetch(); break;
            
            //  C6: add n (M:2 T:7)
            // -- M2
            case 0x0175: _wait();_mread(cpu->pc++); break;
            case 0x0176: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x0177: z80_add8(cpu,cpu->dlatch);_fetch(); break;
            
            //  C7: rst 0h (M:1 T:4)
            // -- OVERLAP
            case 0x0178: _fetch(); break;
            
            //  C8: ret z (M:4 T:11)
            // -- M2 (generic)
            case 0x0179: if(!_cc_z){z80_skip(cpu,4,7,1);}; break;
            // -- M3
            case 0x017A: _wait();_mread(cpu->sp++); break;
            case 0x017B: cpu->wzl=_gd(); break;
            // -- M4
            case 0x017C: _wait();_mread(cpu->sp++); break;
            case 0x017D: cpu->wzh=_gd();cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x017E: _fetch(); break;
            
            //  C9: ret (M:3 T:10)
            // -- M2
            case 0x017F: _wait();_mread(cpu->sp++); break;
            case 0x0180: cpu->wzl=_gd(); break;
            // -- M3
            case 0x0181: _wait();_mread(cpu->sp++); break;
            case 0x0182: cpu->wzh=_gd();cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x0183: _fetch(); break;
            
            //  CA: jp z,nn (M:3 T:10)
            // -- M2
            case 0x0184: _wait();_mread(cpu->pc++); break;
            case 0x0185: cpu->wzl=_gd(); break;
            // -- M3
            case 0x0186: _wait();_mread(cpu->pc++); break;
            case 0x0187: cpu->wzh=_gd();if(_cc_z){cpu->pc=cpu->wz;}; break;
            // -- OVERLAP
            case 0x0188: _fetch(); break;
            
            //  CB: cb prefix (M:1 T:4)
            // -- OVERLAP
            case 0x0189: _fetch_cb(); break;
            
            //  CC: call z,nn (M:6 T:17)
            // -- M2
            case 0x018A: _wait();_mread(cpu->pc++); break;
            case 0x018B: cpu->wzl=_gd(); break;
            // -- M3
            case 0x018C: _wait();_mread(cpu->pc++); break;
            case 0x018D: cpu->wzh=_gd();if (!_cc_z){z80_skip(cpu,5,9,2);}; break;
            // -- M4 (generic)
            case 0x018E:  break;
            // -- M5
            case 0x018F: _mwrite(--cpu->sp,cpu->pch); break;
            case 0x0190: _wait() break;
            // -- M6
            case 0x0191: _mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz; break;
            case 0x0192: _wait() break;
            // -- OVERLAP
            case 0x0193: _fetch(); break;
            
            //  CD: call nn (M:5 T:17)
            // -- M2
            case 0x0194: _wait();_mread(cpu->pc++); break;
            case 0x0195: cpu->wzl=_gd(); break;
            // -- M3
            case 0x0196: _wait();_mread(cpu->pc++); break;
            case 0x0197: cpu->wzh=_gd(); break;
            // -- M4
            case 0x0198: _mwrite(--cpu->sp,cpu->pch); break;
            case 0x0199: _wait() break;
            // -- M5
            case 0x019A: _mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz; break;
            case 0x019B: _wait() break;
            // -- OVERLAP
            case 0x019C: _fetch(); break;
            
            //  CE: adc n (M:2 T:7)
            // -- M2
            case 0x019D: _wait();_mread(cpu->pc++); break;
            case 0x019E: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x019F: z80_adc8(cpu,cpu->dlatch);_fetch(); break;
            
            //  CF: rst 8h (M:1 T:4)
            // -- OVERLAP
            case 0x01A0: _fetch(); break;
            
            //  D0: ret nc (M:4 T:11)
            // -- M2 (generic)
            case 0x01A1: if(!_cc_nc){z80_skip(cpu,4,7,1);}; break;
            // -- M3
            case 0x01A2: _wait();_mread(cpu->sp++); break;
            case 0x01A3: cpu->wzl=_gd(); break;
            // -- M4
            case 0x01A4: _wait();_mread(cpu->sp++); break;
            case 0x01A5: cpu->wzh=_gd();cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x01A6: _fetch(); break;
            
            //  D1: pop de (M:3 T:10)
            // -- M2
            case 0x01A7: _wait();_mread(cpu->sp++); break;
            case 0x01A8: cpu->e=_gd(); break;
            // -- M3
            case 0x01A9: _wait();_mread(cpu->sp++); break;
            case 0x01AA: cpu->d=_gd(); break;
            // -- OVERLAP
            case 0x01AB: _fetch(); break;
            
            //  D2: jp nc,nn (M:3 T:10)
            // -- M2
            case 0x01AC: _wait();_mread(cpu->pc++); break;
            case 0x01AD: cpu->wzl=_gd(); break;
            // -- M3
            case 0x01AE: _wait();_mread(cpu->pc++); break;
            case 0x01AF: cpu->wzh=_gd();if(_cc_nc){cpu->pc=cpu->wz;}; break;
            // -- OVERLAP
            case 0x01B0: _fetch(); break;
            
            //  D3: out (n),a (M:1 T:4)
            // -- OVERLAP
            case 0x01B1: _fetch(); break;
            
            //  D4: call nc,nn (M:6 T:17)
            // -- M2
            case 0x01B2: _wait();_mread(cpu->pc++); break;
            case 0x01B3: cpu->wzl=_gd(); break;
            // -- M3
            case 0x01B4: _wait();_mread(cpu->pc++); break;
            case 0x01B5: cpu->wzh=_gd();if (!_cc_nc){z80_skip(cpu,5,9,2);}; break;
            // -- M4 (generic)
            case 0x01B6:  break;
            // -- M5
            case 0x01B7: _mwrite(--cpu->sp,cpu->pch); break;
            case 0x01B8: _wait() break;
            // -- M6
            case 0x01B9: _mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz; break;
            case 0x01BA: _wait() break;
            // -- OVERLAP
            case 0x01BB: _fetch(); break;
            
            //  D5: push de (M:3 T:11)
            // -- M2
            case 0x01BC: _mwrite(--cpu->sp,cpu->d); break;
            case 0x01BD: _wait() break;
            // -- M3
            case 0x01BE: _mwrite(--cpu->sp,cpu->e); break;
            case 0x01BF: _wait() break;
            // -- OVERLAP
            case 0x01C0: _fetch(); break;
            
            //  D6: sub n (M:2 T:7)
            // -- M2
            case 0x01C1: _wait();_mread(cpu->pc++); break;
            case 0x01C2: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x01C3: z80_sub8(cpu,cpu->dlatch);_fetch(); break;
            
            //  D7: rst 10h (M:1 T:4)
            // -- OVERLAP
            case 0x01C4: _fetch(); break;
            
            //  D8: ret c (M:4 T:11)
            // -- M2 (generic)
            case 0x01C5: if(!_cc_c){z80_skip(cpu,4,7,1);}; break;
            // -- M3
            case 0x01C6: _wait();_mread(cpu->sp++); break;
            case 0x01C7: cpu->wzl=_gd(); break;
            // -- M4
            case 0x01C8: _wait();_mread(cpu->sp++); break;
            case 0x01C9: cpu->wzh=_gd();cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x01CA: _fetch(); break;
            
            //  D9: exx (M:1 T:4)
            // -- OVERLAP
            case 0x01CB: z80_exx(cpu);_fetch(); break;
            
            //  DA: jp c,nn (M:3 T:10)
            // -- M2
            case 0x01CC: _wait();_mread(cpu->pc++); break;
            case 0x01CD: cpu->wzl=_gd(); break;
            // -- M3
            case 0x01CE: _wait();_mread(cpu->pc++); break;
            case 0x01CF: cpu->wzh=_gd();if(_cc_c){cpu->pc=cpu->wz;}; break;
            // -- OVERLAP
            case 0x01D0: _fetch(); break;
            
            //  DB: in a,(n) (M:1 T:4)
            // -- OVERLAP
            case 0x01D1: _fetch(); break;
            
            //  DC: call c,nn (M:6 T:17)
            // -- M2
            case 0x01D2: _wait();_mread(cpu->pc++); break;
            case 0x01D3: cpu->wzl=_gd(); break;
            // -- M3
            case 0x01D4: _wait();_mread(cpu->pc++); break;
            case 0x01D5: cpu->wzh=_gd();if (!_cc_c){z80_skip(cpu,5,9,2);}; break;
            // -- M4 (generic)
            case 0x01D6:  break;
            // -- M5
            case 0x01D7: _mwrite(--cpu->sp,cpu->pch); break;
            case 0x01D8: _wait() break;
            // -- M6
            case 0x01D9: _mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz; break;
            case 0x01DA: _wait() break;
            // -- OVERLAP
            case 0x01DB: _fetch(); break;
            
            //  DD: dd prefix (M:1 T:4)
            // -- OVERLAP
            case 0x01DC: _fetch_dd(); break;
            
            //  DE: sbc n (M:2 T:7)
            // -- M2
            case 0x01DD: _wait();_mread(cpu->pc++); break;
            case 0x01DE: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x01DF: z80_sbc8(cpu,cpu->dlatch);_fetch(); break;
            
            //  DF: rst 18h (M:1 T:4)
            // -- OVERLAP
            case 0x01E0: _fetch(); break;
            
            //  E0: ret po (M:4 T:11)
            // -- M2 (generic)
            case 0x01E1: if(!_cc_po){z80_skip(cpu,4,7,1);}; break;
            // -- M3
            case 0x01E2: _wait();_mread(cpu->sp++); break;
            case 0x01E3: cpu->wzl=_gd(); break;
            // -- M4
            case 0x01E4: _wait();_mread(cpu->sp++); break;
            case 0x01E5: cpu->wzh=_gd();cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x01E6: _fetch(); break;
            
            //  E1: pop hl (M:3 T:10)
            // -- M2
            case 0x01E7: _wait();_mread(cpu->sp++); break;
            case 0x01E8: cpu->hlx[cpu->hlx_idx].l=_gd(); break;
            // -- M3
            case 0x01E9: _wait();_mread(cpu->sp++); break;
            case 0x01EA: cpu->hlx[cpu->hlx_idx].h=_gd(); break;
            // -- OVERLAP
            case 0x01EB: _fetch(); break;
            
            //  E2: jp po,nn (M:3 T:10)
            // -- M2
            case 0x01EC: _wait();_mread(cpu->pc++); break;
            case 0x01ED: cpu->wzl=_gd(); break;
            // -- M3
            case 0x01EE: _wait();_mread(cpu->pc++); break;
            case 0x01EF: cpu->wzh=_gd();if(_cc_po){cpu->pc=cpu->wz;}; break;
            // -- OVERLAP
            case 0x01F0: _fetch(); break;
            
            //  E3: ex (sp),hl (M:5 T:19)
            // -- M2
            case 0x01F1: _wait();_mread(cpu->sp); break;
            case 0x01F2: cpu->wzl=_gd(); break;
            // -- M3
            case 0x01F3: _wait();_mread(cpu->sp+1); break;
            case 0x01F4: cpu->wzh=_gd(); break;
            // -- M4
            case 0x01F5: _mwrite(cpu->sp+1,cpu->hlx[cpu->hlx_idx].h); break;
            case 0x01F6: _wait() break;
            // -- M5
            case 0x01F7: _mwrite(cpu->sp,cpu->hlx[cpu->hlx_idx].l);cpu->hlx[cpu->hlx_idx].hl=cpu->wz; break;
            case 0x01F8: _wait() break;
            // -- OVERLAP
            case 0x01F9: _fetch(); break;
            
            //  E4: call po,nn (M:6 T:17)
            // -- M2
            case 0x01FA: _wait();_mread(cpu->pc++); break;
            case 0x01FB: cpu->wzl=_gd(); break;
            // -- M3
            case 0x01FC: _wait();_mread(cpu->pc++); break;
            case 0x01FD: cpu->wzh=_gd();if (!_cc_po){z80_skip(cpu,5,9,2);}; break;
            // -- M4 (generic)
            case 0x01FE:  break;
            // -- M5
            case 0x01FF: _mwrite(--cpu->sp,cpu->pch); break;
            case 0x0200: _wait() break;
            // -- M6
            case 0x0201: _mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz; break;
            case 0x0202: _wait() break;
            // -- OVERLAP
            case 0x0203: _fetch(); break;
            
            //  E5: push hl (M:3 T:11)
            // -- M2
            case 0x0204: _mwrite(--cpu->sp,cpu->hlx[cpu->hlx_idx].h); break;
            case 0x0205: _wait() break;
            // -- M3
            case 0x0206: _mwrite(--cpu->sp,cpu->hlx[cpu->hlx_idx].l); break;
            case 0x0207: _wait() break;
            // -- OVERLAP
            case 0x0208: _fetch(); break;
            
            //  E6: and n (M:2 T:7)
            // -- M2
            case 0x0209: _wait();_mread(cpu->pc++); break;
            case 0x020A: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x020B: z80_and8(cpu,cpu->dlatch);_fetch(); break;
            
            //  E7: rst 20h (M:1 T:4)
            // -- OVERLAP
            case 0x020C: _fetch(); break;
            
            //  E8: ret pe (M:4 T:11)
            // -- M2 (generic)
            case 0x020D: if(!_cc_pe){z80_skip(cpu,4,7,1);}; break;
            // -- M3
            case 0x020E: _wait();_mread(cpu->sp++); break;
            case 0x020F: cpu->wzl=_gd(); break;
            // -- M4
            case 0x0210: _wait();_mread(cpu->sp++); break;
            case 0x0211: cpu->wzh=_gd();cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x0212: _fetch(); break;
            
            //  E9: jp hl (M:1 T:4)
            // -- OVERLAP
            case 0x0213: cpu->pc=cpu->hlx[cpu->hlx_idx].hl;_fetch(); break;
            
            //  EA: jp pe,nn (M:3 T:10)
            // -- M2
            case 0x0214: _wait();_mread(cpu->pc++); break;
            case 0x0215: cpu->wzl=_gd(); break;
            // -- M3
            case 0x0216: _wait();_mread(cpu->pc++); break;
            case 0x0217: cpu->wzh=_gd();if(_cc_pe){cpu->pc=cpu->wz;}; break;
            // -- OVERLAP
            case 0x0218: _fetch(); break;
            
            //  EB: ex de,hl (M:1 T:4)
            // -- OVERLAP
            case 0x0219: z80_ex_de_hl(cpu);_fetch(); break;
            
            //  EC: call pe,nn (M:6 T:17)
            // -- M2
            case 0x021A: _wait();_mread(cpu->pc++); break;
            case 0x021B: cpu->wzl=_gd(); break;
            // -- M3
            case 0x021C: _wait();_mread(cpu->pc++); break;
            case 0x021D: cpu->wzh=_gd();if (!_cc_pe){z80_skip(cpu,5,9,2);}; break;
            // -- M4 (generic)
            case 0x021E:  break;
            // -- M5
            case 0x021F: _mwrite(--cpu->sp,cpu->pch); break;
            case 0x0220: _wait() break;
            // -- M6
            case 0x0221: _mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz; break;
            case 0x0222: _wait() break;
            // -- OVERLAP
            case 0x0223: _fetch(); break;
            
            //  ED: ed prefix (M:1 T:4)
            // -- OVERLAP
            case 0x0224: _fetch_ed(); break;
            
            //  EE: xor n (M:2 T:7)
            // -- M2
            case 0x0225: _wait();_mread(cpu->pc++); break;
            case 0x0226: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x0227: z80_xor8(cpu,cpu->dlatch);_fetch(); break;
            
            //  EF: rst 28h (M:1 T:4)
            // -- OVERLAP
            case 0x0228: _fetch(); break;
            
            //  F0: ret p (M:4 T:11)
            // -- M2 (generic)
            case 0x0229: if(!_cc_p){z80_skip(cpu,4,7,1);}; break;
            // -- M3
            case 0x022A: _wait();_mread(cpu->sp++); break;
            case 0x022B: cpu->wzl=_gd(); break;
            // -- M4
            case 0x022C: _wait();_mread(cpu->sp++); break;
            case 0x022D: cpu->wzh=_gd();cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x022E: _fetch(); break;
            
            //  F1: pop af (M:3 T:10)
            // -- M2
            case 0x022F: _wait();_mread(cpu->sp++); break;
            case 0x0230: cpu->f=_gd(); break;
            // -- M3
            case 0x0231: _wait();_mread(cpu->sp++); break;
            case 0x0232: cpu->a=_gd(); break;
            // -- OVERLAP
            case 0x0233: _fetch(); break;
            
            //  F2: jp p,nn (M:3 T:10)
            // -- M2
            case 0x0234: _wait();_mread(cpu->pc++); break;
            case 0x0235: cpu->wzl=_gd(); break;
            // -- M3
            case 0x0236: _wait();_mread(cpu->pc++); break;
            case 0x0237: cpu->wzh=_gd();if(_cc_p){cpu->pc=cpu->wz;}; break;
            // -- OVERLAP
            case 0x0238: _fetch(); break;
            
            //  F3: di (M:1 T:4)
            // -- OVERLAP
            case 0x0239: _fetch(); break;
            
            //  F4: call p,nn (M:6 T:17)
            // -- M2
            case 0x023A: _wait();_mread(cpu->pc++); break;
            case 0x023B: cpu->wzl=_gd(); break;
            // -- M3
            case 0x023C: _wait();_mread(cpu->pc++); break;
            case 0x023D: cpu->wzh=_gd();if (!_cc_p){z80_skip(cpu,5,9,2);}; break;
            // -- M4 (generic)
            case 0x023E:  break;
            // -- M5
            case 0x023F: _mwrite(--cpu->sp,cpu->pch); break;
            case 0x0240: _wait() break;
            // -- M6
            case 0x0241: _mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz; break;
            case 0x0242: _wait() break;
            // -- OVERLAP
            case 0x0243: _fetch(); break;
            
            //  F5: push af (M:3 T:11)
            // -- M2
            case 0x0244: _mwrite(--cpu->sp,cpu->a); break;
            case 0x0245: _wait() break;
            // -- M3
            case 0x0246: _mwrite(--cpu->sp,cpu->f); break;
            case 0x0247: _wait() break;
            // -- OVERLAP
            case 0x0248: _fetch(); break;
            
            //  F6: or n (M:2 T:7)
            // -- M2
            case 0x0249: _wait();_mread(cpu->pc++); break;
            case 0x024A: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x024B: z80_or8(cpu,cpu->dlatch);_fetch(); break;
            
            //  F7: rst 30h (M:1 T:4)
            // -- OVERLAP
            case 0x024C: _fetch(); break;
            
            //  F8: ret m (M:4 T:11)
            // -- M2 (generic)
            case 0x024D: if(!_cc_m){z80_skip(cpu,4,7,1);}; break;
            // -- M3
            case 0x024E: _wait();_mread(cpu->sp++); break;
            case 0x024F: cpu->wzl=_gd(); break;
            // -- M4
            case 0x0250: _wait();_mread(cpu->sp++); break;
            case 0x0251: cpu->wzh=_gd();cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x0252: _fetch(); break;
            
            //  F9: ld sp,hl (M:2 T:6)
            // -- M2 (generic)
            case 0x0253: cpu->sp=cpu->hlx[cpu->hlx_idx].hl; break;
            // -- OVERLAP
            case 0x0254: _fetch(); break;
            
            //  FA: jp m,nn (M:3 T:10)
            // -- M2
            case 0x0255: _wait();_mread(cpu->pc++); break;
            case 0x0256: cpu->wzl=_gd(); break;
            // -- M3
            case 0x0257: _wait();_mread(cpu->pc++); break;
            case 0x0258: cpu->wzh=_gd();if(_cc_m){cpu->pc=cpu->wz;}; break;
            // -- OVERLAP
            case 0x0259: _fetch(); break;
            
            //  FB: ei (M:1 T:4)
            // -- OVERLAP
            case 0x025A: _fetch(); break;
            
            //  FC: call m,nn (M:6 T:17)
            // -- M2
            case 0x025B: _wait();_mread(cpu->pc++); break;
            case 0x025C: cpu->wzl=_gd(); break;
            // -- M3
            case 0x025D: _wait();_mread(cpu->pc++); break;
            case 0x025E: cpu->wzh=_gd();if (!_cc_m){z80_skip(cpu,5,9,2);}; break;
            // -- M4 (generic)
            case 0x025F:  break;
            // -- M5
            case 0x0260: _mwrite(--cpu->sp,cpu->pch); break;
            case 0x0261: _wait() break;
            // -- M6
            case 0x0262: _mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz; break;
            case 0x0263: _wait() break;
            // -- OVERLAP
            case 0x0264: _fetch(); break;
            
            //  FD: fd prefix (M:1 T:4)
            // -- OVERLAP
            case 0x0265: _fetch_fd(); break;
            
            //  FE: cp n (M:2 T:7)
            // -- M2
            case 0x0266: _wait();_mread(cpu->pc++); break;
            case 0x0267: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x0268: z80_cp8(cpu,cpu->dlatch);_fetch(); break;
            
            //  FF: rst 38h (M:1 T:4)
            // -- OVERLAP
            case 0x0269: _fetch(); break;

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
