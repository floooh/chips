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
            uint16_t prefix_offset; // opstate table offset: 0x100 on ED prefix, 0x200 on CB prefix
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

static inline void z80_neg8(z80_t* cpu) {
    uint32_t res = (uint32_t) (0 - (int)cpu->a);
    cpu->f = z80_sub_flags(0, cpu->a, res);
    cpu->a = (uint8_t)res;
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

static inline void z80_add16(z80_t* cpu, uint16_t val) {
    const uint16_t acc = cpu->hlx[cpu->hlx_idx].hl;
    cpu->wz = acc + 1;
    const uint32_t res = acc + val;
    cpu->hlx[cpu->hlx_idx].hl = res;
    cpu->f = (cpu->f & (Z80_SF|Z80_ZF|Z80_VF)) |
             (((acc ^ res ^ val)>>8)&Z80_HF) | 
             ((res >> 16) & Z80_CF) |
             ((res >> 8) & (Z80_YF|Z80_XF));
}

static inline void z80_adc16(z80_t* cpu, uint16_t val) {
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

static inline void z80_sbc16(z80_t* cpu, uint16_t val) {
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

static inline bool z80_ldi_ldd(z80_t* cpu) {
    // this just handles the action after the actual byte transfer
    // dlatch is the transferred byte
    const uint8_t res = cpu->a + cpu->dlatch;
    cpu->bc -= 1;
    cpu->f = (cpu->f & (Z80_SF|Z80_ZF|Z80_CF)) |
             ((res & 2) ? Z80_YF : 0) |
             ((res & 8) ? Z80_XF : 0) |
             (cpu->bc ? Z80_VF : 0);
    return cpu->bc != 0;
}

static inline bool z80_cpi_cpd(z80_t* cpu) {
    uint32_t res = (uint32_t) ((int)cpu->a - (int)cpu->dlatch);
    cpu->bc -= 1;
    uint8_t f = (cpu->f & Z80_CF)|Z80_NF|z80_sz_flags(res);
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

static inline uint8_t z80_in(z80_t* cpu, uint8_t val) {
    cpu->f = (cpu->f & Z80_CF) | z80_szp_flags[val];
    return val;
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
            cpu->prefix_offset = 0x0200;
            cpu->prefix |= Z80_PREFIX_CB;
            break;
        case Z80_PREFIX_DD:
            cpu->prefix_offset = 0;
            cpu->hlx_idx = 1;
            cpu->prefix = Z80_PREFIX_DD;
            break;
        case Z80_PREFIX_ED: // ED prefix clears current DD/FD prefix
            cpu->prefix_offset = 0x0100;
            cpu->hlx_idx = 0;
            cpu->prefix = Z80_PREFIX_ED;
            break;
        case Z80_PREFIX_FD:
            cpu->prefix_offset = 0;
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
    { 0x00000002, 0x0004, 0 },  //  00: nop (M:1 T:4 steps:1)
    { 0x000000B6, 0x0005, 0 },  //  01: ld bc,nn (M:3 T:10 steps:5)
    { 0x0000001C, 0x000A, 0 },  //  02: ld (bc),a (M:2 T:7 steps:3)
    { 0x0000000A, 0x000D, 0 },  //  03: inc bc (M:2 T:6 steps:2)
    { 0x00000002, 0x000F, 0 },  //  04: inc b (M:1 T:4 steps:1)
    { 0x00000002, 0x0010, 0 },  //  05: dec b (M:1 T:4 steps:1)
    { 0x00000016, 0x0011, Z80_OPSTATE_FLAGS_IMM8 },  //  06: ld b,n (M:2 T:7 steps:3)
    { 0x00000002, 0x0014, 0 },  //  07: rlca (M:1 T:4 steps:1)
    { 0x00000002, 0x0015, 0 },  //  08: ex af,af' (M:1 T:4 steps:1)
    { 0x00000102, 0x0016, 0 },  //  09: add hl,bc (M:2 T:11 steps:2)
    { 0x00000016, 0x0018, 0 },  //  0A: ld a,(bc) (M:2 T:7 steps:3)
    { 0x0000000A, 0x001B, 0 },  //  0B: dec bc (M:2 T:6 steps:2)
    { 0x00000002, 0x001D, 0 },  //  0C: inc c (M:1 T:4 steps:1)
    { 0x00000002, 0x001E, 0 },  //  0D: dec c (M:1 T:4 steps:1)
    { 0x00000016, 0x001F, Z80_OPSTATE_FLAGS_IMM8 },  //  0E: ld c,n (M:2 T:7 steps:3)
    { 0x00000002, 0x0022, 0 },  //  0F: rrca (M:1 T:4 steps:1)
    { 0x0000042C, 0x0023, 0 },  //  10: djnz d (M:3 T:13 steps:4)
    { 0x000000B6, 0x0027, 0 },  //  11: ld de,nn (M:3 T:10 steps:5)
    { 0x0000001C, 0x002C, 0 },  //  12: ld (de),a (M:2 T:7 steps:3)
    { 0x0000000A, 0x002F, 0 },  //  13: inc de (M:2 T:6 steps:2)
    { 0x00000002, 0x0031, 0 },  //  14: inc d (M:1 T:4 steps:1)
    { 0x00000002, 0x0032, 0 },  //  15: dec d (M:1 T:4 steps:1)
    { 0x00000016, 0x0033, Z80_OPSTATE_FLAGS_IMM8 },  //  16: ld d,n (M:2 T:7 steps:3)
    { 0x00000002, 0x0036, 0 },  //  17: rla (M:1 T:4 steps:1)
    { 0x00000216, 0x0037, 0 },  //  18: jr d (M:3 T:12 steps:4)
    { 0x00000102, 0x003B, 0 },  //  19: add hl,de (M:2 T:11 steps:2)
    { 0x00000016, 0x003D, 0 },  //  1A: ld a,(de) (M:2 T:7 steps:3)
    { 0x0000000A, 0x0040, 0 },  //  1B: dec de (M:2 T:6 steps:2)
    { 0x00000002, 0x0042, 0 },  //  1C: inc e (M:1 T:4 steps:1)
    { 0x00000002, 0x0043, 0 },  //  1D: dec e (M:1 T:4 steps:1)
    { 0x00000016, 0x0044, Z80_OPSTATE_FLAGS_IMM8 },  //  1E: ld e,n (M:2 T:7 steps:3)
    { 0x00000002, 0x0047, 0 },  //  1F: rra (M:1 T:4 steps:1)
    { 0x00000216, 0x0048, 0 },  //  20: jr nz,d (M:3 T:12 steps:4)
    { 0x000000B6, 0x004C, 0 },  //  21: ld hl,nn (M:3 T:10 steps:5)
    { 0x00003B36, 0x0051, 0 },  //  22: ld (nn),hl (M:5 T:16 steps:9)
    { 0x0000000A, 0x005A, 0 },  //  23: inc hl (M:2 T:6 steps:2)
    { 0x00000002, 0x005C, 0 },  //  24: inc h (M:1 T:4 steps:1)
    { 0x00000002, 0x005D, 0 },  //  25: dec h (M:1 T:4 steps:1)
    { 0x00000016, 0x005E, Z80_OPSTATE_FLAGS_IMM8 },  //  26: ld h,n (M:2 T:7 steps:3)
    { 0x00000002, 0x0061, 0 },  //  27: daa (M:1 T:4 steps:1)
    { 0x00000216, 0x0062, 0 },  //  28: jr z,d (M:3 T:12 steps:4)
    { 0x00000102, 0x0066, 0 },  //  29: add hl,hl (M:2 T:11 steps:2)
    { 0x00002DB6, 0x0068, 0 },  //  2A: ld hl,(nn) (M:5 T:16 steps:9)
    { 0x0000000A, 0x0071, 0 },  //  2B: dec hl (M:2 T:6 steps:2)
    { 0x00000002, 0x0073, 0 },  //  2C: inc l (M:1 T:4 steps:1)
    { 0x00000002, 0x0074, 0 },  //  2D: dec l (M:1 T:4 steps:1)
    { 0x00000016, 0x0075, Z80_OPSTATE_FLAGS_IMM8 },  //  2E: ld l,n (M:2 T:7 steps:3)
    { 0x00000002, 0x0078, 0 },  //  2F: cpl (M:1 T:4 steps:1)
    { 0x00000216, 0x0079, 0 },  //  30: jr nc,d (M:3 T:12 steps:4)
    { 0x000000B6, 0x007D, 0 },  //  31: ld sp,nn (M:3 T:10 steps:5)
    { 0x00000736, 0x0082, 0 },  //  32: ld (nn),a (M:4 T:13 steps:7)
    { 0x0000000A, 0x0089, 0 },  //  33: inc sp (M:2 T:6 steps:2)
    { 0x000001C6, 0x008B, Z80_OPSTATE_FLAGS_INDIRECT },  //  34: inc (hl) (M:3 T:11 steps:5)
    { 0x000001C6, 0x0090, Z80_OPSTATE_FLAGS_INDIRECT },  //  35: dec (hl) (M:3 T:11 steps:5)
    { 0x000000E6, 0x0095, Z80_OPSTATE_FLAGS_INDIRECT|Z80_OPSTATE_FLAGS_IMM8 },  //  36: ld (hl),n (M:3 T:10 steps:5)
    { 0x00000002, 0x009A, 0 },  //  37: scf (M:1 T:4 steps:1)
    { 0x00000216, 0x009B, 0 },  //  38: jr c,d (M:3 T:12 steps:4)
    { 0x00000102, 0x009F, 0 },  //  39: add hl,sp (M:2 T:11 steps:2)
    { 0x000005B6, 0x00A1, 0 },  //  3A: ld a,(nn) (M:4 T:13 steps:7)
    { 0x0000000A, 0x00A8, 0 },  //  3B: dec sp (M:2 T:6 steps:2)
    { 0x00000002, 0x00AA, 0 },  //  3C: inc a (M:1 T:4 steps:1)
    { 0x00000002, 0x00AB, 0 },  //  3D: dec a (M:1 T:4 steps:1)
    { 0x00000016, 0x00AC, Z80_OPSTATE_FLAGS_IMM8 },  //  3E: ld a,n (M:2 T:7 steps:3)
    { 0x00000002, 0x00AF, 0 },  //  3F: ccf (M:1 T:4 steps:1)
    { 0x00000002, 0x00B0, 0 },  //  40: ld b,b (M:1 T:4 steps:1)
    { 0x00000002, 0x00B1, 0 },  //  41: ld b,c (M:1 T:4 steps:1)
    { 0x00000002, 0x00B2, 0 },  //  42: ld b,d (M:1 T:4 steps:1)
    { 0x00000002, 0x00B3, 0 },  //  43: ld b,e (M:1 T:4 steps:1)
    { 0x00000002, 0x00B4, 0 },  //  44: ld b,h (M:1 T:4 steps:1)
    { 0x00000002, 0x00B5, 0 },  //  45: ld b,l (M:1 T:4 steps:1)
    { 0x00000016, 0x00B6, Z80_OPSTATE_FLAGS_INDIRECT },  //  46: ld b,(hl) (M:2 T:7 steps:3)
    { 0x00000002, 0x00B9, 0 },  //  47: ld b,a (M:1 T:4 steps:1)
    { 0x00000002, 0x00BA, 0 },  //  48: ld c,b (M:1 T:4 steps:1)
    { 0x00000002, 0x00BB, 0 },  //  49: ld c,c (M:1 T:4 steps:1)
    { 0x00000002, 0x00BC, 0 },  //  4A: ld c,d (M:1 T:4 steps:1)
    { 0x00000002, 0x00BD, 0 },  //  4B: ld c,e (M:1 T:4 steps:1)
    { 0x00000002, 0x00BE, 0 },  //  4C: ld c,h (M:1 T:4 steps:1)
    { 0x00000002, 0x00BF, 0 },  //  4D: ld c,l (M:1 T:4 steps:1)
    { 0x00000016, 0x00C0, Z80_OPSTATE_FLAGS_INDIRECT },  //  4E: ld c,(hl) (M:2 T:7 steps:3)
    { 0x00000002, 0x00C3, 0 },  //  4F: ld c,a (M:1 T:4 steps:1)
    { 0x00000002, 0x00C4, 0 },  //  50: ld d,b (M:1 T:4 steps:1)
    { 0x00000002, 0x00C5, 0 },  //  51: ld d,c (M:1 T:4 steps:1)
    { 0x00000002, 0x00C6, 0 },  //  52: ld d,d (M:1 T:4 steps:1)
    { 0x00000002, 0x00C7, 0 },  //  53: ld d,e (M:1 T:4 steps:1)
    { 0x00000002, 0x00C8, 0 },  //  54: ld d,h (M:1 T:4 steps:1)
    { 0x00000002, 0x00C9, 0 },  //  55: ld d,l (M:1 T:4 steps:1)
    { 0x00000016, 0x00CA, Z80_OPSTATE_FLAGS_INDIRECT },  //  56: ld d,(hl) (M:2 T:7 steps:3)
    { 0x00000002, 0x00CD, 0 },  //  57: ld d,a (M:1 T:4 steps:1)
    { 0x00000002, 0x00CE, 0 },  //  58: ld e,b (M:1 T:4 steps:1)
    { 0x00000002, 0x00CF, 0 },  //  59: ld e,c (M:1 T:4 steps:1)
    { 0x00000002, 0x00D0, 0 },  //  5A: ld e,d (M:1 T:4 steps:1)
    { 0x00000002, 0x00D1, 0 },  //  5B: ld e,e (M:1 T:4 steps:1)
    { 0x00000002, 0x00D2, 0 },  //  5C: ld e,h (M:1 T:4 steps:1)
    { 0x00000002, 0x00D3, 0 },  //  5D: ld e,l (M:1 T:4 steps:1)
    { 0x00000016, 0x00D4, Z80_OPSTATE_FLAGS_INDIRECT },  //  5E: ld e,(hl) (M:2 T:7 steps:3)
    { 0x00000002, 0x00D7, 0 },  //  5F: ld e,a (M:1 T:4 steps:1)
    { 0x00000002, 0x00D8, 0 },  //  60: ld h,b (M:1 T:4 steps:1)
    { 0x00000002, 0x00D9, 0 },  //  61: ld h,c (M:1 T:4 steps:1)
    { 0x00000002, 0x00DA, 0 },  //  62: ld h,d (M:1 T:4 steps:1)
    { 0x00000002, 0x00DB, 0 },  //  63: ld h,e (M:1 T:4 steps:1)
    { 0x00000002, 0x00DC, 0 },  //  64: ld h,h (M:1 T:4 steps:1)
    { 0x00000002, 0x00DD, 0 },  //  65: ld h,l (M:1 T:4 steps:1)
    { 0x00000016, 0x00DE, Z80_OPSTATE_FLAGS_INDIRECT },  //  66: ld h,(hl) (M:2 T:7 steps:3)
    { 0x00000002, 0x00E1, 0 },  //  67: ld h,a (M:1 T:4 steps:1)
    { 0x00000002, 0x00E2, 0 },  //  68: ld l,b (M:1 T:4 steps:1)
    { 0x00000002, 0x00E3, 0 },  //  69: ld l,c (M:1 T:4 steps:1)
    { 0x00000002, 0x00E4, 0 },  //  6A: ld l,d (M:1 T:4 steps:1)
    { 0x00000002, 0x00E5, 0 },  //  6B: ld l,e (M:1 T:4 steps:1)
    { 0x00000002, 0x00E6, 0 },  //  6C: ld l,h (M:1 T:4 steps:1)
    { 0x00000002, 0x00E7, 0 },  //  6D: ld l,l (M:1 T:4 steps:1)
    { 0x00000016, 0x00E8, Z80_OPSTATE_FLAGS_INDIRECT },  //  6E: ld l,(hl) (M:2 T:7 steps:3)
    { 0x00000002, 0x00EB, 0 },  //  6F: ld l,a (M:1 T:4 steps:1)
    { 0x0000001C, 0x00EC, Z80_OPSTATE_FLAGS_INDIRECT },  //  70: ld (hl),b (M:2 T:7 steps:3)
    { 0x0000001C, 0x00EF, Z80_OPSTATE_FLAGS_INDIRECT },  //  71: ld (hl),c (M:2 T:7 steps:3)
    { 0x0000001C, 0x00F2, Z80_OPSTATE_FLAGS_INDIRECT },  //  72: ld (hl),d (M:2 T:7 steps:3)
    { 0x0000001C, 0x00F5, Z80_OPSTATE_FLAGS_INDIRECT },  //  73: ld (hl),e (M:2 T:7 steps:3)
    { 0x0000001C, 0x00F8, Z80_OPSTATE_FLAGS_INDIRECT },  //  74: ld (hl),h (M:2 T:7 steps:3)
    { 0x0000001C, 0x00FB, Z80_OPSTATE_FLAGS_INDIRECT },  //  75: ld (hl),l (M:2 T:7 steps:3)
    { 0x00000002, 0x00FE, 0 },  //  76: halt (M:1 T:4 steps:1)
    { 0x0000001C, 0x00FF, Z80_OPSTATE_FLAGS_INDIRECT },  //  77: ld (hl),a (M:2 T:7 steps:3)
    { 0x00000002, 0x0102, 0 },  //  78: ld a,b (M:1 T:4 steps:1)
    { 0x00000002, 0x0103, 0 },  //  79: ld a,c (M:1 T:4 steps:1)
    { 0x00000002, 0x0104, 0 },  //  7A: ld a,d (M:1 T:4 steps:1)
    { 0x00000002, 0x0105, 0 },  //  7B: ld a,e (M:1 T:4 steps:1)
    { 0x00000002, 0x0106, 0 },  //  7C: ld a,h (M:1 T:4 steps:1)
    { 0x00000002, 0x0107, 0 },  //  7D: ld a,l (M:1 T:4 steps:1)
    { 0x00000016, 0x0108, Z80_OPSTATE_FLAGS_INDIRECT },  //  7E: ld a,(hl) (M:2 T:7 steps:3)
    { 0x00000002, 0x010B, 0 },  //  7F: ld a,a (M:1 T:4 steps:1)
    { 0x00000002, 0x010C, 0 },  //  80: add b (M:1 T:4 steps:1)
    { 0x00000002, 0x010D, 0 },  //  81: add c (M:1 T:4 steps:1)
    { 0x00000002, 0x010E, 0 },  //  82: add d (M:1 T:4 steps:1)
    { 0x00000002, 0x010F, 0 },  //  83: add e (M:1 T:4 steps:1)
    { 0x00000002, 0x0110, 0 },  //  84: add h (M:1 T:4 steps:1)
    { 0x00000002, 0x0111, 0 },  //  85: add l (M:1 T:4 steps:1)
    { 0x00000016, 0x0112, Z80_OPSTATE_FLAGS_INDIRECT },  //  86: add (hl) (M:2 T:7 steps:3)
    { 0x00000002, 0x0115, 0 },  //  87: add a (M:1 T:4 steps:1)
    { 0x00000002, 0x0116, 0 },  //  88: adc b (M:1 T:4 steps:1)
    { 0x00000002, 0x0117, 0 },  //  89: adc c (M:1 T:4 steps:1)
    { 0x00000002, 0x0118, 0 },  //  8A: adc d (M:1 T:4 steps:1)
    { 0x00000002, 0x0119, 0 },  //  8B: adc e (M:1 T:4 steps:1)
    { 0x00000002, 0x011A, 0 },  //  8C: adc h (M:1 T:4 steps:1)
    { 0x00000002, 0x011B, 0 },  //  8D: adc l (M:1 T:4 steps:1)
    { 0x00000016, 0x011C, Z80_OPSTATE_FLAGS_INDIRECT },  //  8E: adc (hl) (M:2 T:7 steps:3)
    { 0x00000002, 0x011F, 0 },  //  8F: adc a (M:1 T:4 steps:1)
    { 0x00000002, 0x0120, 0 },  //  90: sub b (M:1 T:4 steps:1)
    { 0x00000002, 0x0121, 0 },  //  91: sub c (M:1 T:4 steps:1)
    { 0x00000002, 0x0122, 0 },  //  92: sub d (M:1 T:4 steps:1)
    { 0x00000002, 0x0123, 0 },  //  93: sub e (M:1 T:4 steps:1)
    { 0x00000002, 0x0124, 0 },  //  94: sub h (M:1 T:4 steps:1)
    { 0x00000002, 0x0125, 0 },  //  95: sub l (M:1 T:4 steps:1)
    { 0x00000016, 0x0126, Z80_OPSTATE_FLAGS_INDIRECT },  //  96: sub (hl) (M:2 T:7 steps:3)
    { 0x00000002, 0x0129, 0 },  //  97: sub a (M:1 T:4 steps:1)
    { 0x00000002, 0x012A, 0 },  //  98: sbc b (M:1 T:4 steps:1)
    { 0x00000002, 0x012B, 0 },  //  99: sbc c (M:1 T:4 steps:1)
    { 0x00000002, 0x012C, 0 },  //  9A: sbc d (M:1 T:4 steps:1)
    { 0x00000002, 0x012D, 0 },  //  9B: sbc e (M:1 T:4 steps:1)
    { 0x00000002, 0x012E, 0 },  //  9C: sbc h (M:1 T:4 steps:1)
    { 0x00000002, 0x012F, 0 },  //  9D: sbc l (M:1 T:4 steps:1)
    { 0x00000016, 0x0130, Z80_OPSTATE_FLAGS_INDIRECT },  //  9E: sbc (hl) (M:2 T:7 steps:3)
    { 0x00000002, 0x0133, 0 },  //  9F: sbc a (M:1 T:4 steps:1)
    { 0x00000002, 0x0134, 0 },  //  A0: and b (M:1 T:4 steps:1)
    { 0x00000002, 0x0135, 0 },  //  A1: and c (M:1 T:4 steps:1)
    { 0x00000002, 0x0136, 0 },  //  A2: and d (M:1 T:4 steps:1)
    { 0x00000002, 0x0137, 0 },  //  A3: and e (M:1 T:4 steps:1)
    { 0x00000002, 0x0138, 0 },  //  A4: and h (M:1 T:4 steps:1)
    { 0x00000002, 0x0139, 0 },  //  A5: and l (M:1 T:4 steps:1)
    { 0x00000016, 0x013A, Z80_OPSTATE_FLAGS_INDIRECT },  //  A6: and (hl) (M:2 T:7 steps:3)
    { 0x00000002, 0x013D, 0 },  //  A7: and a (M:1 T:4 steps:1)
    { 0x00000002, 0x013E, 0 },  //  A8: xor b (M:1 T:4 steps:1)
    { 0x00000002, 0x013F, 0 },  //  A9: xor c (M:1 T:4 steps:1)
    { 0x00000002, 0x0140, 0 },  //  AA: xor d (M:1 T:4 steps:1)
    { 0x00000002, 0x0141, 0 },  //  AB: xor e (M:1 T:4 steps:1)
    { 0x00000002, 0x0142, 0 },  //  AC: xor h (M:1 T:4 steps:1)
    { 0x00000002, 0x0143, 0 },  //  AD: xor l (M:1 T:4 steps:1)
    { 0x00000016, 0x0144, Z80_OPSTATE_FLAGS_INDIRECT },  //  AE: xor (hl) (M:2 T:7 steps:3)
    { 0x00000002, 0x0147, 0 },  //  AF: xor a (M:1 T:4 steps:1)
    { 0x00000002, 0x0148, 0 },  //  B0: or b (M:1 T:4 steps:1)
    { 0x00000002, 0x0149, 0 },  //  B1: or c (M:1 T:4 steps:1)
    { 0x00000002, 0x014A, 0 },  //  B2: or d (M:1 T:4 steps:1)
    { 0x00000002, 0x014B, 0 },  //  B3: or e (M:1 T:4 steps:1)
    { 0x00000002, 0x014C, 0 },  //  B4: or h (M:1 T:4 steps:1)
    { 0x00000002, 0x014D, 0 },  //  B5: or l (M:1 T:4 steps:1)
    { 0x00000016, 0x014E, Z80_OPSTATE_FLAGS_INDIRECT },  //  B6: or (hl) (M:2 T:7 steps:3)
    { 0x00000002, 0x0151, 0 },  //  B7: or a (M:1 T:4 steps:1)
    { 0x00000002, 0x0152, 0 },  //  B8: cp b (M:1 T:4 steps:1)
    { 0x00000002, 0x0153, 0 },  //  B9: cp c (M:1 T:4 steps:1)
    { 0x00000002, 0x0154, 0 },  //  BA: cp d (M:1 T:4 steps:1)
    { 0x00000002, 0x0155, 0 },  //  BB: cp e (M:1 T:4 steps:1)
    { 0x00000002, 0x0156, 0 },  //  BC: cp h (M:1 T:4 steps:1)
    { 0x00000002, 0x0157, 0 },  //  BD: cp l (M:1 T:4 steps:1)
    { 0x00000016, 0x0158, Z80_OPSTATE_FLAGS_INDIRECT },  //  BE: cp (hl) (M:2 T:7 steps:3)
    { 0x00000002, 0x015B, 0 },  //  BF: cp a (M:1 T:4 steps:1)
    { 0x0000016E, 0x015C, 0 },  //  C0: ret nz (M:4 T:11 steps:6)
    { 0x000000B6, 0x0162, 0 },  //  C1: pop bc (M:3 T:10 steps:5)
    { 0x000000B6, 0x0167, 0 },  //  C2: jp nz,nn (M:3 T:10 steps:5)
    { 0x000000B6, 0x016C, 0 },  //  C3: jp nn (M:3 T:10 steps:5)
    { 0x000076B6, 0x0171, 0 },  //  C4: call nz,nn (M:6 T:17 steps:10)
    { 0x000001D8, 0x017B, 0 },  //  C5: push bc (M:3 T:11 steps:5)
    { 0x00000016, 0x0180, Z80_OPSTATE_FLAGS_IMM8 },  //  C6: add n (M:2 T:7 steps:3)
    { 0x000001D8, 0x0183, 0 },  //  C7: rst 0h (M:3 T:11 steps:5)
    { 0x0000016E, 0x0188, 0 },  //  C8: ret z (M:4 T:11 steps:6)
    { 0x000000B6, 0x018E, 0 },  //  C9: ret (M:3 T:10 steps:5)
    { 0x000000B6, 0x0193, 0 },  //  CA: jp z,nn (M:3 T:10 steps:5)
    { 0x00000002, 0x0198, 0 },  //  CB: cb prefix (M:1 T:4 steps:1)
    { 0x000076B6, 0x0199, 0 },  //  CC: call z,nn (M:6 T:17 steps:10)
    { 0x00007636, 0x01A3, 0 },  //  CD: call nn (M:5 T:17 steps:9)
    { 0x00000016, 0x01AC, Z80_OPSTATE_FLAGS_IMM8 },  //  CE: adc n (M:2 T:7 steps:3)
    { 0x000001D8, 0x01AF, 0 },  //  CF: rst 8h (M:3 T:11 steps:5)
    { 0x0000016E, 0x01B4, 0 },  //  D0: ret nc (M:4 T:11 steps:6)
    { 0x000000B6, 0x01BA, 0 },  //  D1: pop de (M:3 T:10 steps:5)
    { 0x000000B6, 0x01BF, 0 },  //  D2: jp nc,nn (M:3 T:10 steps:5)
    { 0x00000166, 0x01C4, 0 },  //  D3: out (n),a (M:3 T:11 steps:5)
    { 0x000076B6, 0x01C9, 0 },  //  D4: call nc,nn (M:6 T:17 steps:10)
    { 0x000001D8, 0x01D3, 0 },  //  D5: push de (M:3 T:11 steps:5)
    { 0x00000016, 0x01D8, Z80_OPSTATE_FLAGS_IMM8 },  //  D6: sub n (M:2 T:7 steps:3)
    { 0x000001D8, 0x01DB, 0 },  //  D7: rst 10h (M:3 T:11 steps:5)
    { 0x0000016E, 0x01E0, 0 },  //  D8: ret c (M:4 T:11 steps:6)
    { 0x00000002, 0x01E6, 0 },  //  D9: exx (M:1 T:4 steps:1)
    { 0x000000B6, 0x01E7, 0 },  //  DA: jp c,nn (M:3 T:10 steps:5)
    { 0x00000166, 0x01EC, 0 },  //  DB: in a,(n) (M:3 T:11 steps:5)
    { 0x000076B6, 0x01F1, 0 },  //  DC: call c,nn (M:6 T:17 steps:10)
    { 0x00000002, 0x01FB, 0 },  //  DD: dd prefix (M:1 T:4 steps:1)
    { 0x00000016, 0x01FC, Z80_OPSTATE_FLAGS_IMM8 },  //  DE: sbc n (M:2 T:7 steps:3)
    { 0x000001D8, 0x01FF, 0 },  //  DF: rst 18h (M:3 T:11 steps:5)
    { 0x0000016E, 0x0204, 0 },  //  E0: ret po (M:4 T:11 steps:6)
    { 0x000000B6, 0x020A, 0 },  //  E1: pop hl (M:3 T:10 steps:5)
    { 0x000000B6, 0x020F, 0 },  //  E2: jp po,nn (M:3 T:10 steps:5)
    { 0x00013636, 0x0214, 0 },  //  E3: ex (sp),hl (M:5 T:19 steps:9)
    { 0x000076B6, 0x021D, 0 },  //  E4: call po,nn (M:6 T:17 steps:10)
    { 0x000001D8, 0x0227, 0 },  //  E5: push hl (M:3 T:11 steps:5)
    { 0x00000016, 0x022C, Z80_OPSTATE_FLAGS_IMM8 },  //  E6: and n (M:2 T:7 steps:3)
    { 0x000001D8, 0x022F, 0 },  //  E7: rst 20h (M:3 T:11 steps:5)
    { 0x0000016E, 0x0234, 0 },  //  E8: ret pe (M:4 T:11 steps:6)
    { 0x00000002, 0x023A, 0 },  //  E9: jp hl (M:1 T:4 steps:1)
    { 0x000000B6, 0x023B, 0 },  //  EA: jp pe,nn (M:3 T:10 steps:5)
    { 0x00000002, 0x0240, 0 },  //  EB: ex de,hl (M:1 T:4 steps:1)
    { 0x000076B6, 0x0241, 0 },  //  EC: call pe,nn (M:6 T:17 steps:10)
    { 0x00000002, 0x024B, 0 },  //  ED: ed prefix (M:1 T:4 steps:1)
    { 0x00000016, 0x024C, Z80_OPSTATE_FLAGS_IMM8 },  //  EE: xor n (M:2 T:7 steps:3)
    { 0x000001D8, 0x024F, 0 },  //  EF: rst 28h (M:3 T:11 steps:5)
    { 0x0000016E, 0x0254, 0 },  //  F0: ret p (M:4 T:11 steps:6)
    { 0x000000B6, 0x025A, 0 },  //  F1: pop af (M:3 T:10 steps:5)
    { 0x000000B6, 0x025F, 0 },  //  F2: jp p,nn (M:3 T:10 steps:5)
    { 0x00000002, 0x0264, 0 },  //  F3: di (M:1 T:4 steps:1)
    { 0x000076B6, 0x0265, 0 },  //  F4: call p,nn (M:6 T:17 steps:10)
    { 0x000001D8, 0x026F, 0 },  //  F5: push af (M:3 T:11 steps:5)
    { 0x00000016, 0x0274, Z80_OPSTATE_FLAGS_IMM8 },  //  F6: or n (M:2 T:7 steps:3)
    { 0x000001D8, 0x0277, 0 },  //  F7: rst 30h (M:3 T:11 steps:5)
    { 0x0000016E, 0x027C, 0 },  //  F8: ret m (M:4 T:11 steps:6)
    { 0x0000000A, 0x0282, 0 },  //  F9: ld sp,hl (M:2 T:6 steps:2)
    { 0x000000B6, 0x0284, 0 },  //  FA: jp m,nn (M:3 T:10 steps:5)
    { 0x00000002, 0x0289, 0 },  //  FB: ei (M:1 T:4 steps:1)
    { 0x000076B6, 0x028A, 0 },  //  FC: call m,nn (M:6 T:17 steps:10)
    { 0x00000002, 0x0294, 0 },  //  FD: fd prefix (M:1 T:4 steps:1)
    { 0x00000016, 0x0295, Z80_OPSTATE_FLAGS_IMM8 },  //  FE: cp n (M:2 T:7 steps:3)
    { 0x000001D8, 0x0298, 0 },  //  FF: rst 38h (M:3 T:11 steps:5)
    { 0x00000002, 0x029D, 0 },  // ED 00: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 01: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 02: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 03: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 04: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 05: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 06: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 07: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 08: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 09: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 0A: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 0B: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 0C: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 0D: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 0E: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 0F: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 10: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 11: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 12: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 13: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 14: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 15: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 16: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 17: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 18: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 19: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 1A: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 1B: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 1C: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 1D: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 1E: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 1F: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 20: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 21: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 22: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 23: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 24: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 25: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 26: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 27: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 28: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 29: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 2A: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 2B: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 2C: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 2D: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 2E: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 2F: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 30: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 31: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 32: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 33: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 34: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 35: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 36: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 37: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 38: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 39: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 3A: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 3B: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 3C: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 3D: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 3E: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 3F: ed nop (M:1 T:4 steps:1)
    { 0x0000002C, 0x029E, 0 },  // ED 40: in b,(c) (M:2 T:8 steps:3)
    { 0x0000002C, 0x02A1, 0 },  // ED 41: out (c),b (M:2 T:8 steps:3)
    { 0x00000102, 0x02A4, 0 },  // ED 42: sbc hl,bc (M:2 T:11 steps:2)
    { 0x00003B36, 0x02A6, 0 },  // ED 43: ld (nn),bc (M:5 T:16 steps:9)
    { 0x00000002, 0x02AF, 0 },  // ED 44: neg (M:1 T:4 steps:1)
    { 0x00000002, 0x02B0, 0 },  // ED 45: retn (M:1 T:4 steps:1)
    { 0x00000002, 0x02B1, 0 },  // ED 46: im IM0 (M:1 T:4 steps:1)
    { 0x00000002, 0x02B2, 0 },  // ED 47: ld i,a (M:1 T:4 steps:1)
    { 0x0000002C, 0x02B3, 0 },  // ED 48: in c,(c) (M:2 T:8 steps:3)
    { 0x0000002C, 0x02B6, 0 },  // ED 49: out (c),c (M:2 T:8 steps:3)
    { 0x00000102, 0x02B9, 0 },  // ED 4A: adc hl,bc (M:2 T:11 steps:2)
    { 0x00002DB6, 0x02BB, 0 },  // ED 4B: ld bc,(nn) (M:5 T:16 steps:9)
    { 0x00000002, 0x02AF, 0 },  // ED 4C: neg (M:1 T:4 steps:1)
    { 0x00000002, 0x02C4, 0 },  // ED 4D: reti (M:1 T:4 steps:1)
    { 0x00000002, 0x02C5, 0 },  // ED 4E: im IM1 (M:1 T:4 steps:1)
    { 0x00000002, 0x02C6, 0 },  // ED 4F: ld r,a (M:1 T:4 steps:1)
    { 0x0000002C, 0x02C7, 0 },  // ED 50: in d,(c) (M:2 T:8 steps:3)
    { 0x0000002C, 0x02CA, 0 },  // ED 51: out (c),d (M:2 T:8 steps:3)
    { 0x00000102, 0x02CD, 0 },  // ED 52: sbc hl,de (M:2 T:11 steps:2)
    { 0x00003B36, 0x02CF, 0 },  // ED 53: ld (nn),de (M:5 T:16 steps:9)
    { 0x00000002, 0x02AF, 0 },  // ED 54: neg (M:1 T:4 steps:1)
    { 0x00000002, 0x02B0, 0 },  // ED 55: retn (M:1 T:4 steps:1)
    { 0x00000002, 0x02D8, 0 },  // ED 56: im IM2 (M:1 T:4 steps:1)
    { 0x00000002, 0x02D9, 0 },  // ED 57: ld a,i (M:1 T:4 steps:1)
    { 0x0000002C, 0x02DA, 0 },  // ED 58: in e,(c) (M:2 T:8 steps:3)
    { 0x0000002C, 0x02DD, 0 },  // ED 59: out (c),e (M:2 T:8 steps:3)
    { 0x00000102, 0x02E0, 0 },  // ED 5A: adc hl,de (M:2 T:11 steps:2)
    { 0x00002DB6, 0x02E2, 0 },  // ED 5B: ld de,(nn) (M:5 T:16 steps:9)
    { 0x00000002, 0x02AF, 0 },  // ED 5C: neg (M:1 T:4 steps:1)
    { 0x00000002, 0x02B0, 0 },  // ED 5D: retn (M:1 T:4 steps:1)
    { 0x00000002, 0x02EB, 0 },  // ED 5E: im IM3 (M:1 T:4 steps:1)
    { 0x00000002, 0x02EC, 0 },  // ED 5F: ld a,r (M:1 T:4 steps:1)
    { 0x0000002C, 0x02ED, 0 },  // ED 60: in h,(c) (M:2 T:8 steps:3)
    { 0x0000002C, 0x02F0, 0 },  // ED 61: out (c),h (M:2 T:8 steps:3)
    { 0x00000102, 0x02F3, 0 },  // ED 62: sbc hl,hl (M:2 T:11 steps:2)
    { 0x00003B36, 0x02F5, 0 },  // ED 63: ld (nn),hl (M:5 T:16 steps:9)
    { 0x00000002, 0x02AF, 0 },  // ED 64: neg (M:1 T:4 steps:1)
    { 0x00000002, 0x02B0, 0 },  // ED 65: retn (M:1 T:4 steps:1)
    { 0x00000002, 0x02FE, 0 },  // ED 66: im IM4 (M:1 T:4 steps:1)
    { 0x00000002, 0x02FF, 0 },  // ED 67: rrd (M:1 T:4 steps:1)
    { 0x0000002C, 0x0300, 0 },  // ED 68: in l,(c) (M:2 T:8 steps:3)
    { 0x0000002C, 0x0303, 0 },  // ED 69: out (c),l (M:2 T:8 steps:3)
    { 0x00000102, 0x0306, 0 },  // ED 6A: adc hl,hl (M:2 T:11 steps:2)
    { 0x00002DB6, 0x0308, 0 },  // ED 6B: ld hl,(nn) (M:5 T:16 steps:9)
    { 0x00000002, 0x02AF, 0 },  // ED 6C: neg (M:1 T:4 steps:1)
    { 0x00000002, 0x02B0, 0 },  // ED 6D: retn (M:1 T:4 steps:1)
    { 0x00000002, 0x0311, 0 },  // ED 6E: im IM5 (M:1 T:4 steps:1)
    { 0x00000002, 0x0312, 0 },  // ED 6F: rld (M:1 T:4 steps:1)
    { 0x0000002C, 0x0313, 0 },  // ED 70: in (c) (M:2 T:8 steps:3)
    { 0x0000002C, 0x0316, 0 },  // ED 71: out (c),0 (M:2 T:8 steps:3)
    { 0x00000102, 0x0319, 0 },  // ED 72: sbc hl,sp (M:2 T:11 steps:2)
    { 0x00003B36, 0x031B, 0 },  // ED 73: ld (nn),sp (M:5 T:16 steps:9)
    { 0x00000002, 0x02AF, 0 },  // ED 74: neg (M:1 T:4 steps:1)
    { 0x00000002, 0x02B0, 0 },  // ED 75: retn (M:1 T:4 steps:1)
    { 0x00000002, 0x0324, 0 },  // ED 76: im IM6 (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 77: ed nop (M:1 T:4 steps:1)
    { 0x0000002C, 0x0325, 0 },  // ED 78: in a,(c) (M:2 T:8 steps:3)
    { 0x0000002C, 0x0328, 0 },  // ED 79: out (c),a (M:2 T:8 steps:3)
    { 0x00000102, 0x032B, 0 },  // ED 7A: adc hl,sp (M:2 T:11 steps:2)
    { 0x00002DB6, 0x032D, 0 },  // ED 7B: ld sp,(nn) (M:5 T:16 steps:9)
    { 0x00000002, 0x02AF, 0 },  // ED 7C: neg (M:1 T:4 steps:1)
    { 0x00000002, 0x02B0, 0 },  // ED 7D: retn (M:1 T:4 steps:1)
    { 0x00000002, 0x0336, 0 },  // ED 7E: im IM7 (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 7F: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 80: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 81: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 82: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 83: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 84: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 85: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 86: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 87: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 88: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 89: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 8A: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 8B: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 8C: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 8D: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 8E: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 8F: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 90: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 91: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 92: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 93: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 94: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 95: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 96: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 97: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 98: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 99: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 9A: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 9B: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 9C: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 9D: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 9E: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED 9F: ed nop (M:1 T:4 steps:1)
    { 0x000002E6, 0x0337, 0 },  // ED A0: ldi (M:4 T:12 steps:6)
    { 0x00000216, 0x033D, 0 },  // ED A1: cpi (M:3 T:12 steps:4)
    { 0x00000002, 0x0341, 0 },  // ED A2: ini (M:1 T:4 steps:1)
    { 0x00000002, 0x0342, 0 },  // ED A3: outi (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED A4: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED A5: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED A6: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED A7: ed nop (M:1 T:4 steps:1)
    { 0x000002E6, 0x0343, 0 },  // ED A8: ldd (M:4 T:12 steps:6)
    { 0x00000216, 0x0349, 0 },  // ED A9: cpd (M:3 T:12 steps:4)
    { 0x00000002, 0x034D, 0 },  // ED AA: ind (M:1 T:4 steps:1)
    { 0x00000002, 0x034E, 0 },  // ED AB: outd (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED AC: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED AD: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED AE: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED AF: ed nop (M:1 T:4 steps:1)
    { 0x000042E6, 0x034F, 0 },  // ED B0: ldir (M:5 T:17 steps:7)
    { 0x00004216, 0x0356, 0 },  // ED B1: cpir (M:4 T:17 steps:5)
    { 0x00000002, 0x035B, 0 },  // ED B2: inir (M:1 T:4 steps:1)
    { 0x00000002, 0x035C, 0 },  // ED B3: otir (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED B4: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED B5: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED B6: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED B7: ed nop (M:1 T:4 steps:1)
    { 0x000042E6, 0x035D, 0 },  // ED B8: lddr (M:5 T:17 steps:7)
    { 0x00004216, 0x0364, 0 },  // ED B9: cpdr (M:4 T:17 steps:5)
    { 0x00000002, 0x0369, 0 },  // ED BA: indr (M:1 T:4 steps:1)
    { 0x00000002, 0x036A, 0 },  // ED BB: otdr (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED BC: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED BD: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED BE: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED BF: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED C0: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED C1: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED C2: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED C3: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED C4: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED C5: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED C6: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED C7: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED C8: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED C9: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED CA: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED CB: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED CC: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED CD: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED CE: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED CF: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED D0: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED D1: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED D2: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED D3: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED D4: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED D5: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED D6: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED D7: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED D8: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED D9: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED DA: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED DB: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED DC: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED DD: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED DE: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED DF: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED E0: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED E1: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED E2: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED E3: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED E4: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED E5: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED E6: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED E7: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED E8: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED E9: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED EA: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED EB: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED EC: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED ED: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED EE: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED EF: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED F0: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED F1: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED F2: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED F3: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED F4: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED F5: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED F6: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED F7: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED F8: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED F9: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED FA: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED FB: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED FC: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED FD: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED FE: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x029D, 0 },  // ED FF: ed nop (M:1 T:4 steps:1)
    { 0x00000002, 0x036B, 0 },  // CB 00: rlc b (M:1 T:4 steps:1)
    { 0x00000002, 0x036C, 0 },  // CB 01: rlc c (M:1 T:4 steps:1)
    { 0x00000002, 0x036D, 0 },  // CB 02: rlc d (M:1 T:4 steps:1)
    { 0x00000002, 0x036E, 0 },  // CB 03: rlc e (M:1 T:4 steps:1)
    { 0x00000002, 0x036F, 0 },  // CB 04: rlc h (M:1 T:4 steps:1)
    { 0x00000002, 0x0370, 0 },  // CB 05: rlc l (M:1 T:4 steps:1)
    { 0x00000002, 0x0371, 0 },  // CB 06: rlc (hl) (M:1 T:4 steps:1)
    { 0x00000002, 0x0372, 0 },  // CB 07: rlc a (M:1 T:4 steps:1)
    { 0x00000002, 0x0373, 0 },  // CB 08: rrc b (M:1 T:4 steps:1)
    { 0x00000002, 0x0374, 0 },  // CB 09: rrc c (M:1 T:4 steps:1)
    { 0x00000002, 0x0375, 0 },  // CB 0A: rrc d (M:1 T:4 steps:1)
    { 0x00000002, 0x0376, 0 },  // CB 0B: rrc e (M:1 T:4 steps:1)
    { 0x00000002, 0x0377, 0 },  // CB 0C: rrc h (M:1 T:4 steps:1)
    { 0x00000002, 0x0378, 0 },  // CB 0D: rrc l (M:1 T:4 steps:1)
    { 0x00000002, 0x0379, 0 },  // CB 0E: rrc (hl) (M:1 T:4 steps:1)
    { 0x00000002, 0x037A, 0 },  // CB 0F: rrc a (M:1 T:4 steps:1)
    { 0x00000002, 0x037B, 0 },  // CB 10: rl b (M:1 T:4 steps:1)
    { 0x00000002, 0x037C, 0 },  // CB 11: rl c (M:1 T:4 steps:1)
    { 0x00000002, 0x037D, 0 },  // CB 12: rl d (M:1 T:4 steps:1)
    { 0x00000002, 0x037E, 0 },  // CB 13: rl e (M:1 T:4 steps:1)
    { 0x00000002, 0x037F, 0 },  // CB 14: rl h (M:1 T:4 steps:1)
    { 0x00000002, 0x0380, 0 },  // CB 15: rl l (M:1 T:4 steps:1)
    { 0x00000002, 0x0381, 0 },  // CB 16: rl (hl) (M:1 T:4 steps:1)
    { 0x00000002, 0x0382, 0 },  // CB 17: rl a (M:1 T:4 steps:1)
    { 0x00000002, 0x0383, 0 },  // CB 18: rr b (M:1 T:4 steps:1)
    { 0x00000002, 0x0384, 0 },  // CB 19: rr c (M:1 T:4 steps:1)
    { 0x00000002, 0x0385, 0 },  // CB 1A: rr d (M:1 T:4 steps:1)
    { 0x00000002, 0x0386, 0 },  // CB 1B: rr e (M:1 T:4 steps:1)
    { 0x00000002, 0x0387, 0 },  // CB 1C: rr h (M:1 T:4 steps:1)
    { 0x00000002, 0x0388, 0 },  // CB 1D: rr l (M:1 T:4 steps:1)
    { 0x00000002, 0x0389, 0 },  // CB 1E: rr (hl) (M:1 T:4 steps:1)
    { 0x00000002, 0x038A, 0 },  // CB 1F: rr a (M:1 T:4 steps:1)
    { 0x00000002, 0x038B, 0 },  // CB 20: sla b (M:1 T:4 steps:1)
    { 0x00000002, 0x038C, 0 },  // CB 21: sla c (M:1 T:4 steps:1)
    { 0x00000002, 0x038D, 0 },  // CB 22: sla d (M:1 T:4 steps:1)
    { 0x00000002, 0x038E, 0 },  // CB 23: sla e (M:1 T:4 steps:1)
    { 0x00000002, 0x038F, 0 },  // CB 24: sla h (M:1 T:4 steps:1)
    { 0x00000002, 0x0390, 0 },  // CB 25: sla l (M:1 T:4 steps:1)
    { 0x00000002, 0x0391, 0 },  // CB 26: sla (hl) (M:1 T:4 steps:1)
    { 0x00000002, 0x0392, 0 },  // CB 27: sla a (M:1 T:4 steps:1)
    { 0x00000002, 0x0393, 0 },  // CB 28: sra b (M:1 T:4 steps:1)
    { 0x00000002, 0x0394, 0 },  // CB 29: sra c (M:1 T:4 steps:1)
    { 0x00000002, 0x0395, 0 },  // CB 2A: sra d (M:1 T:4 steps:1)
    { 0x00000002, 0x0396, 0 },  // CB 2B: sra e (M:1 T:4 steps:1)
    { 0x00000002, 0x0397, 0 },  // CB 2C: sra h (M:1 T:4 steps:1)
    { 0x00000002, 0x0398, 0 },  // CB 2D: sra l (M:1 T:4 steps:1)
    { 0x00000002, 0x0399, 0 },  // CB 2E: sra (hl) (M:1 T:4 steps:1)
    { 0x00000002, 0x039A, 0 },  // CB 2F: sra a (M:1 T:4 steps:1)
    { 0x00000002, 0x039B, 0 },  // CB 30: sll b (M:1 T:4 steps:1)
    { 0x00000002, 0x039C, 0 },  // CB 31: sll c (M:1 T:4 steps:1)
    { 0x00000002, 0x039D, 0 },  // CB 32: sll d (M:1 T:4 steps:1)
    { 0x00000002, 0x039E, 0 },  // CB 33: sll e (M:1 T:4 steps:1)
    { 0x00000002, 0x039F, 0 },  // CB 34: sll h (M:1 T:4 steps:1)
    { 0x00000002, 0x03A0, 0 },  // CB 35: sll l (M:1 T:4 steps:1)
    { 0x00000002, 0x03A1, 0 },  // CB 36: sll (hl) (M:1 T:4 steps:1)
    { 0x00000002, 0x03A2, 0 },  // CB 37: sll a (M:1 T:4 steps:1)
    { 0x00000002, 0x03A3, 0 },  // CB 38: srl b (M:1 T:4 steps:1)
    { 0x00000002, 0x03A4, 0 },  // CB 39: srl c (M:1 T:4 steps:1)
    { 0x00000002, 0x03A5, 0 },  // CB 3A: srl d (M:1 T:4 steps:1)
    { 0x00000002, 0x03A6, 0 },  // CB 3B: srl e (M:1 T:4 steps:1)
    { 0x00000002, 0x03A7, 0 },  // CB 3C: srl h (M:1 T:4 steps:1)
    { 0x00000002, 0x03A8, 0 },  // CB 3D: srl l (M:1 T:4 steps:1)
    { 0x00000002, 0x03A9, 0 },  // CB 3E: srl (hl) (M:1 T:4 steps:1)
    { 0x00000002, 0x03AA, 0 },  // CB 3F: srl a (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 40: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 41: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 42: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 43: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 44: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 45: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AC, 0 },  // CB 46: bit n,(hl) (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 47: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 48: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 49: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 4A: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 4B: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 4C: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 4D: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AC, 0 },  // CB 4E: bit n,(hl) (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 4F: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 50: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 51: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 52: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 53: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 54: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 55: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AC, 0 },  // CB 56: bit n,(hl) (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 57: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 58: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 59: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 5A: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 5B: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 5C: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 5D: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AC, 0 },  // CB 5E: bit n,(hl) (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 5F: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 60: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 61: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 62: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 63: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 64: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 65: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AC, 0 },  // CB 66: bit n,(hl) (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 67: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 68: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 69: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 6A: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 6B: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 6C: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 6D: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AC, 0 },  // CB 6E: bit n,(hl) (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 6F: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 70: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 71: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 72: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 73: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 74: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 75: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AC, 0 },  // CB 76: bit n,(hl) (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 77: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 78: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 79: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 7A: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 7B: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 7C: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 7D: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AC, 0 },  // CB 7E: bit n,(hl) (M:1 T:4 steps:1)
    { 0x00000002, 0x03AB, 0 },  // CB 7F: bit n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB 80: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB 81: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB 82: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB 83: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB 84: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB 85: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AE, 0 },  // CB 86: res n,(hl) (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB 87: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB 88: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB 89: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB 8A: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB 8B: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB 8C: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB 8D: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AE, 0 },  // CB 8E: res n,(hl) (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB 8F: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB 90: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB 91: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB 92: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB 93: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB 94: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB 95: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AE, 0 },  // CB 96: res n,(hl) (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB 97: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB 98: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB 99: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB 9A: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB 9B: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB 9C: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB 9D: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AE, 0 },  // CB 9E: res n,(hl) (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB 9F: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB A0: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB A1: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB A2: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB A3: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB A4: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB A5: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AE, 0 },  // CB A6: res n,(hl) (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB A7: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB A8: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB A9: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB AA: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB AB: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB AC: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB AD: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AE, 0 },  // CB AE: res n,(hl) (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB AF: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB B0: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB B1: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB B2: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB B3: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB B4: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB B5: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AE, 0 },  // CB B6: res n,(hl) (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB B7: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB B8: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB B9: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB BA: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB BB: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB BC: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB BD: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AE, 0 },  // CB BE: res n,(hl) (M:1 T:4 steps:1)
    { 0x00000002, 0x03AD, 0 },  // CB BF: res n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB C0: set n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB C1: set n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB C2: set n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB C3: set n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB C4: set n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB C5: set n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03B0, 0 },  // CB C6: set n,(hl) (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB C7: set n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB C8: set n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB C9: set n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB CA: set n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB CB: set n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB CC: set n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB CD: set n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03B0, 0 },  // CB CE: set n,(hl) (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB CF: set n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB D0: set n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB D1: set n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB D2: set n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB D3: set n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB D4: set n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB D5: set n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03B0, 0 },  // CB D6: set n,(hl) (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB D7: set n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB D8: set n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB D9: set n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB DA: set n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB DB: set n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB DC: set n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB DD: set n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03B0, 0 },  // CB DE: set n,(hl) (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB DF: set n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB E0: set n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB E1: set n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB E2: set n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB E3: set n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB E4: set n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB E5: set n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03B0, 0 },  // CB E6: set n,(hl) (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB E7: set n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB E8: set n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB E9: set n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB EA: set n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB EB: set n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB EC: set n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB ED: set n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03B0, 0 },  // CB EE: set n,(hl) (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB EF: set n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB F0: set n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB F1: set n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB F2: set n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB F3: set n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB F4: set n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB F5: set n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03B0, 0 },  // CB F6: set n,(hl) (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB F7: set n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB F8: set n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB F9: set n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB FA: set n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB FB: set n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB FC: set n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB FD: set n,r (M:1 T:4 steps:1)
    { 0x00000002, 0x03B0, 0 },  // CB FE: set n,(hl) (M:1 T:4 steps:1)
    { 0x00000002, 0x03AF, 0 },  // CB FF: set n,r (M:1 T:4 steps:1)

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
                cpu->op = z80_opstate_table[cpu->ir + cpu->prefix_offset];

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
            
            //  09: add hl,bc (M:2 T:11)
            // -- M2 (generic)
            case 0x0017: z80_add16(cpu,cpu->bc); break;
            // -- OVERLAP
            case 0x0018: _fetch(); break;
            
            //  0A: ld a,(bc) (M:2 T:7)
            // -- M2
            case 0x0019: _wait();_mread(cpu->bc); break;
            case 0x001A: cpu->a=_gd();cpu->wz=cpu->bc+1; break;
            // -- OVERLAP
            case 0x001B: _fetch(); break;
            
            //  0B: dec bc (M:2 T:6)
            // -- M2 (generic)
            case 0x001C: cpu->bc--; break;
            // -- OVERLAP
            case 0x001D: _fetch(); break;
            
            //  0C: inc c (M:1 T:4)
            // -- OVERLAP
            case 0x001E: cpu->c=z80_inc8(cpu,cpu->c);_fetch(); break;
            
            //  0D: dec c (M:1 T:4)
            // -- OVERLAP
            case 0x001F: cpu->c=z80_dec8(cpu,cpu->c);_fetch(); break;
            
            //  0E: ld c,n (M:2 T:7)
            // -- M2
            case 0x0020: _wait();_mread(cpu->pc++); break;
            case 0x0021: cpu->c=_gd(); break;
            // -- OVERLAP
            case 0x0022: _fetch(); break;
            
            //  0F: rrca (M:1 T:4)
            // -- OVERLAP
            case 0x0023: z80_rrca(cpu);_fetch(); break;
            
            //  10: djnz d (M:3 T:13)
            // -- M2
            case 0x0024: _wait();_mread(cpu->pc++); break;
            case 0x0025: cpu->dlatch=_gd();if(--cpu->b==0){z80_skip(cpu,1,7,2);}; break;
            // -- M3 (generic)
            case 0x0026: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;; break;
            // -- OVERLAP
            case 0x0027: _fetch(); break;
            
            //  11: ld de,nn (M:3 T:10)
            // -- M2
            case 0x0028: _wait();_mread(cpu->pc++); break;
            case 0x0029: cpu->e=_gd(); break;
            // -- M3
            case 0x002A: _wait();_mread(cpu->pc++); break;
            case 0x002B: cpu->d=_gd(); break;
            // -- OVERLAP
            case 0x002C: _fetch(); break;
            
            //  12: ld (de),a (M:2 T:7)
            // -- M2
            case 0x002D: _mwrite(cpu->de,cpu->a);cpu->wzl=cpu->e+1;cpu->wzh=cpu->a; break;
            case 0x002E: _wait() break;
            // -- OVERLAP
            case 0x002F: _fetch(); break;
            
            //  13: inc de (M:2 T:6)
            // -- M2 (generic)
            case 0x0030: cpu->de++; break;
            // -- OVERLAP
            case 0x0031: _fetch(); break;
            
            //  14: inc d (M:1 T:4)
            // -- OVERLAP
            case 0x0032: cpu->d=z80_inc8(cpu,cpu->d);_fetch(); break;
            
            //  15: dec d (M:1 T:4)
            // -- OVERLAP
            case 0x0033: cpu->d=z80_dec8(cpu,cpu->d);_fetch(); break;
            
            //  16: ld d,n (M:2 T:7)
            // -- M2
            case 0x0034: _wait();_mread(cpu->pc++); break;
            case 0x0035: cpu->d=_gd(); break;
            // -- OVERLAP
            case 0x0036: _fetch(); break;
            
            //  17: rla (M:1 T:4)
            // -- OVERLAP
            case 0x0037: z80_rla(cpu);_fetch(); break;
            
            //  18: jr d (M:3 T:12)
            // -- M2
            case 0x0038: _wait();_mread(cpu->pc++); break;
            case 0x0039: cpu->dlatch=_gd(); break;
            // -- M3 (generic)
            case 0x003A: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;; break;
            // -- OVERLAP
            case 0x003B: _fetch(); break;
            
            //  19: add hl,de (M:2 T:11)
            // -- M2 (generic)
            case 0x003C: z80_add16(cpu,cpu->de); break;
            // -- OVERLAP
            case 0x003D: _fetch(); break;
            
            //  1A: ld a,(de) (M:2 T:7)
            // -- M2
            case 0x003E: _wait();_mread(cpu->de); break;
            case 0x003F: cpu->a=_gd();cpu->wz=cpu->de+1; break;
            // -- OVERLAP
            case 0x0040: _fetch(); break;
            
            //  1B: dec de (M:2 T:6)
            // -- M2 (generic)
            case 0x0041: cpu->de--; break;
            // -- OVERLAP
            case 0x0042: _fetch(); break;
            
            //  1C: inc e (M:1 T:4)
            // -- OVERLAP
            case 0x0043: cpu->e=z80_inc8(cpu,cpu->e);_fetch(); break;
            
            //  1D: dec e (M:1 T:4)
            // -- OVERLAP
            case 0x0044: cpu->e=z80_dec8(cpu,cpu->e);_fetch(); break;
            
            //  1E: ld e,n (M:2 T:7)
            // -- M2
            case 0x0045: _wait();_mread(cpu->pc++); break;
            case 0x0046: cpu->e=_gd(); break;
            // -- OVERLAP
            case 0x0047: _fetch(); break;
            
            //  1F: rra (M:1 T:4)
            // -- OVERLAP
            case 0x0048: z80_rra(cpu);_fetch(); break;
            
            //  20: jr nz,d (M:3 T:12)
            // -- M2
            case 0x0049: _wait();_mread(cpu->pc++); break;
            case 0x004A: cpu->dlatch=_gd();if(!(_cc_nz)){z80_skip(cpu,1,7,2);}; break;
            // -- M3 (generic)
            case 0x004B: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;; break;
            // -- OVERLAP
            case 0x004C: _fetch(); break;
            
            //  21: ld hl,nn (M:3 T:10)
            // -- M2
            case 0x004D: _wait();_mread(cpu->pc++); break;
            case 0x004E: cpu->hlx[cpu->hlx_idx].l=_gd(); break;
            // -- M3
            case 0x004F: _wait();_mread(cpu->pc++); break;
            case 0x0050: cpu->hlx[cpu->hlx_idx].h=_gd(); break;
            // -- OVERLAP
            case 0x0051: _fetch(); break;
            
            //  22: ld (nn),hl (M:5 T:16)
            // -- M2
            case 0x0052: _wait();_mread(cpu->pc++); break;
            case 0x0053: cpu->wzl=_gd(); break;
            // -- M3
            case 0x0054: _wait();_mread(cpu->pc++); break;
            case 0x0055: cpu->wzh=_gd(); break;
            // -- M4
            case 0x0056: _mwrite(cpu->wz++,cpu->hlx[cpu->hlx_idx].l); break;
            case 0x0057: _wait() break;
            // -- M5
            case 0x0058: _mwrite(cpu->wz,cpu->hlx[cpu->hlx_idx].h); break;
            case 0x0059: _wait() break;
            // -- OVERLAP
            case 0x005A: _fetch(); break;
            
            //  23: inc hl (M:2 T:6)
            // -- M2 (generic)
            case 0x005B: cpu->hlx[cpu->hlx_idx].hl++; break;
            // -- OVERLAP
            case 0x005C: _fetch(); break;
            
            //  24: inc h (M:1 T:4)
            // -- OVERLAP
            case 0x005D: cpu->hlx[cpu->hlx_idx].h=z80_inc8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); break;
            
            //  25: dec h (M:1 T:4)
            // -- OVERLAP
            case 0x005E: cpu->hlx[cpu->hlx_idx].h=z80_dec8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); break;
            
            //  26: ld h,n (M:2 T:7)
            // -- M2
            case 0x005F: _wait();_mread(cpu->pc++); break;
            case 0x0060: cpu->hlx[cpu->hlx_idx].h=_gd(); break;
            // -- OVERLAP
            case 0x0061: _fetch(); break;
            
            //  27: daa (M:1 T:4)
            // -- OVERLAP
            case 0x0062: z80_daa(cpu);_fetch(); break;
            
            //  28: jr z,d (M:3 T:12)
            // -- M2
            case 0x0063: _wait();_mread(cpu->pc++); break;
            case 0x0064: cpu->dlatch=_gd();if(!(_cc_z)){z80_skip(cpu,1,7,2);}; break;
            // -- M3 (generic)
            case 0x0065: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;; break;
            // -- OVERLAP
            case 0x0066: _fetch(); break;
            
            //  29: add hl,hl (M:2 T:11)
            // -- M2 (generic)
            case 0x0067: z80_add16(cpu,cpu->hlx[cpu->hlx_idx].hl); break;
            // -- OVERLAP
            case 0x0068: _fetch(); break;
            
            //  2A: ld hl,(nn) (M:5 T:16)
            // -- M2
            case 0x0069: _wait();_mread(cpu->pc++); break;
            case 0x006A: cpu->wzl=_gd(); break;
            // -- M3
            case 0x006B: _wait();_mread(cpu->pc++); break;
            case 0x006C: cpu->wzh=_gd(); break;
            // -- M4
            case 0x006D: _wait();_mread(cpu->wz++); break;
            case 0x006E: cpu->hlx[cpu->hlx_idx].l=_gd(); break;
            // -- M5
            case 0x006F: _wait();_mread(cpu->wz); break;
            case 0x0070: cpu->hlx[cpu->hlx_idx].h=_gd(); break;
            // -- OVERLAP
            case 0x0071: _fetch(); break;
            
            //  2B: dec hl (M:2 T:6)
            // -- M2 (generic)
            case 0x0072: cpu->hlx[cpu->hlx_idx].hl--; break;
            // -- OVERLAP
            case 0x0073: _fetch(); break;
            
            //  2C: inc l (M:1 T:4)
            // -- OVERLAP
            case 0x0074: cpu->hlx[cpu->hlx_idx].l=z80_inc8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); break;
            
            //  2D: dec l (M:1 T:4)
            // -- OVERLAP
            case 0x0075: cpu->hlx[cpu->hlx_idx].l=z80_dec8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); break;
            
            //  2E: ld l,n (M:2 T:7)
            // -- M2
            case 0x0076: _wait();_mread(cpu->pc++); break;
            case 0x0077: cpu->hlx[cpu->hlx_idx].l=_gd(); break;
            // -- OVERLAP
            case 0x0078: _fetch(); break;
            
            //  2F: cpl (M:1 T:4)
            // -- OVERLAP
            case 0x0079: z80_cpl(cpu);_fetch(); break;
            
            //  30: jr nc,d (M:3 T:12)
            // -- M2
            case 0x007A: _wait();_mread(cpu->pc++); break;
            case 0x007B: cpu->dlatch=_gd();if(!(_cc_nc)){z80_skip(cpu,1,7,2);}; break;
            // -- M3 (generic)
            case 0x007C: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;; break;
            // -- OVERLAP
            case 0x007D: _fetch(); break;
            
            //  31: ld sp,nn (M:3 T:10)
            // -- M2
            case 0x007E: _wait();_mread(cpu->pc++); break;
            case 0x007F: cpu->spl=_gd(); break;
            // -- M3
            case 0x0080: _wait();_mread(cpu->pc++); break;
            case 0x0081: cpu->sph=_gd(); break;
            // -- OVERLAP
            case 0x0082: _fetch(); break;
            
            //  32: ld (nn),a (M:4 T:13)
            // -- M2
            case 0x0083: _wait();_mread(cpu->pc++); break;
            case 0x0084: cpu->wzl=_gd(); break;
            // -- M3
            case 0x0085: _wait();_mread(cpu->pc++); break;
            case 0x0086: cpu->wzh=_gd(); break;
            // -- M4
            case 0x0087: _mwrite(cpu->wz++,cpu->a);cpu->wzh=cpu->a; break;
            case 0x0088: _wait() break;
            // -- OVERLAP
            case 0x0089: _fetch(); break;
            
            //  33: inc sp (M:2 T:6)
            // -- M2 (generic)
            case 0x008A: cpu->sp++; break;
            // -- OVERLAP
            case 0x008B: _fetch(); break;
            
            //  34: inc (hl) (M:3 T:11)
            // -- M2
            case 0x008C: _wait();_mread(cpu->addr); break;
            case 0x008D: cpu->dlatch=_gd();cpu->dlatch=z80_inc8(cpu,cpu->dlatch); break;
            // -- M3
            case 0x008E: _mwrite(cpu->addr,cpu->dlatch); break;
            case 0x008F: _wait() break;
            // -- OVERLAP
            case 0x0090: _fetch(); break;
            
            //  35: dec (hl) (M:3 T:11)
            // -- M2
            case 0x0091: _wait();_mread(cpu->addr); break;
            case 0x0092: cpu->dlatch=_gd();cpu->dlatch=z80_dec8(cpu,cpu->dlatch); break;
            // -- M3
            case 0x0093: _mwrite(cpu->addr,cpu->dlatch); break;
            case 0x0094: _wait() break;
            // -- OVERLAP
            case 0x0095: _fetch(); break;
            
            //  36: ld (hl),n (M:3 T:10)
            // -- M2
            case 0x0096: _wait();_mread(cpu->pc++); break;
            case 0x0097: cpu->dlatch=_gd(); break;
            // -- M3
            case 0x0098: _mwrite(cpu->addr,cpu->dlatch); break;
            case 0x0099: _wait() break;
            // -- OVERLAP
            case 0x009A: _fetch(); break;
            
            //  37: scf (M:1 T:4)
            // -- OVERLAP
            case 0x009B: z80_scf(cpu);_fetch(); break;
            
            //  38: jr c,d (M:3 T:12)
            // -- M2
            case 0x009C: _wait();_mread(cpu->pc++); break;
            case 0x009D: cpu->dlatch=_gd();if(!(_cc_c)){z80_skip(cpu,1,7,2);}; break;
            // -- M3 (generic)
            case 0x009E: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;; break;
            // -- OVERLAP
            case 0x009F: _fetch(); break;
            
            //  39: add hl,sp (M:2 T:11)
            // -- M2 (generic)
            case 0x00A0: z80_add16(cpu,cpu->sp); break;
            // -- OVERLAP
            case 0x00A1: _fetch(); break;
            
            //  3A: ld a,(nn) (M:4 T:13)
            // -- M2
            case 0x00A2: _wait();_mread(cpu->pc++); break;
            case 0x00A3: cpu->wzl=_gd(); break;
            // -- M3
            case 0x00A4: _wait();_mread(cpu->pc++); break;
            case 0x00A5: cpu->wzh=_gd(); break;
            // -- M4
            case 0x00A6: _wait();_mread(cpu->wz++); break;
            case 0x00A7: cpu->a=_gd(); break;
            // -- OVERLAP
            case 0x00A8: _fetch(); break;
            
            //  3B: dec sp (M:2 T:6)
            // -- M2 (generic)
            case 0x00A9: cpu->sp--; break;
            // -- OVERLAP
            case 0x00AA: _fetch(); break;
            
            //  3C: inc a (M:1 T:4)
            // -- OVERLAP
            case 0x00AB: cpu->a=z80_inc8(cpu,cpu->a);_fetch(); break;
            
            //  3D: dec a (M:1 T:4)
            // -- OVERLAP
            case 0x00AC: cpu->a=z80_dec8(cpu,cpu->a);_fetch(); break;
            
            //  3E: ld a,n (M:2 T:7)
            // -- M2
            case 0x00AD: _wait();_mread(cpu->pc++); break;
            case 0x00AE: cpu->a=_gd(); break;
            // -- OVERLAP
            case 0x00AF: _fetch(); break;
            
            //  3F: ccf (M:1 T:4)
            // -- OVERLAP
            case 0x00B0: z80_ccf(cpu);_fetch(); break;
            
            //  40: ld b,b (M:1 T:4)
            // -- OVERLAP
            case 0x00B1: cpu->b=cpu->b;_fetch(); break;
            
            //  41: ld b,c (M:1 T:4)
            // -- OVERLAP
            case 0x00B2: cpu->b=cpu->c;_fetch(); break;
            
            //  42: ld b,d (M:1 T:4)
            // -- OVERLAP
            case 0x00B3: cpu->b=cpu->d;_fetch(); break;
            
            //  43: ld b,e (M:1 T:4)
            // -- OVERLAP
            case 0x00B4: cpu->b=cpu->e;_fetch(); break;
            
            //  44: ld b,h (M:1 T:4)
            // -- OVERLAP
            case 0x00B5: cpu->b=cpu->hlx[cpu->hlx_idx].h;_fetch(); break;
            
            //  45: ld b,l (M:1 T:4)
            // -- OVERLAP
            case 0x00B6: cpu->b=cpu->hlx[cpu->hlx_idx].l;_fetch(); break;
            
            //  46: ld b,(hl) (M:2 T:7)
            // -- M2
            case 0x00B7: _wait();_mread(cpu->addr); break;
            case 0x00B8: cpu->b=_gd(); break;
            // -- OVERLAP
            case 0x00B9: _fetch(); break;
            
            //  47: ld b,a (M:1 T:4)
            // -- OVERLAP
            case 0x00BA: cpu->b=cpu->a;_fetch(); break;
            
            //  48: ld c,b (M:1 T:4)
            // -- OVERLAP
            case 0x00BB: cpu->c=cpu->b;_fetch(); break;
            
            //  49: ld c,c (M:1 T:4)
            // -- OVERLAP
            case 0x00BC: cpu->c=cpu->c;_fetch(); break;
            
            //  4A: ld c,d (M:1 T:4)
            // -- OVERLAP
            case 0x00BD: cpu->c=cpu->d;_fetch(); break;
            
            //  4B: ld c,e (M:1 T:4)
            // -- OVERLAP
            case 0x00BE: cpu->c=cpu->e;_fetch(); break;
            
            //  4C: ld c,h (M:1 T:4)
            // -- OVERLAP
            case 0x00BF: cpu->c=cpu->hlx[cpu->hlx_idx].h;_fetch(); break;
            
            //  4D: ld c,l (M:1 T:4)
            // -- OVERLAP
            case 0x00C0: cpu->c=cpu->hlx[cpu->hlx_idx].l;_fetch(); break;
            
            //  4E: ld c,(hl) (M:2 T:7)
            // -- M2
            case 0x00C1: _wait();_mread(cpu->addr); break;
            case 0x00C2: cpu->c=_gd(); break;
            // -- OVERLAP
            case 0x00C3: _fetch(); break;
            
            //  4F: ld c,a (M:1 T:4)
            // -- OVERLAP
            case 0x00C4: cpu->c=cpu->a;_fetch(); break;
            
            //  50: ld d,b (M:1 T:4)
            // -- OVERLAP
            case 0x00C5: cpu->d=cpu->b;_fetch(); break;
            
            //  51: ld d,c (M:1 T:4)
            // -- OVERLAP
            case 0x00C6: cpu->d=cpu->c;_fetch(); break;
            
            //  52: ld d,d (M:1 T:4)
            // -- OVERLAP
            case 0x00C7: cpu->d=cpu->d;_fetch(); break;
            
            //  53: ld d,e (M:1 T:4)
            // -- OVERLAP
            case 0x00C8: cpu->d=cpu->e;_fetch(); break;
            
            //  54: ld d,h (M:1 T:4)
            // -- OVERLAP
            case 0x00C9: cpu->d=cpu->hlx[cpu->hlx_idx].h;_fetch(); break;
            
            //  55: ld d,l (M:1 T:4)
            // -- OVERLAP
            case 0x00CA: cpu->d=cpu->hlx[cpu->hlx_idx].l;_fetch(); break;
            
            //  56: ld d,(hl) (M:2 T:7)
            // -- M2
            case 0x00CB: _wait();_mread(cpu->addr); break;
            case 0x00CC: cpu->d=_gd(); break;
            // -- OVERLAP
            case 0x00CD: _fetch(); break;
            
            //  57: ld d,a (M:1 T:4)
            // -- OVERLAP
            case 0x00CE: cpu->d=cpu->a;_fetch(); break;
            
            //  58: ld e,b (M:1 T:4)
            // -- OVERLAP
            case 0x00CF: cpu->e=cpu->b;_fetch(); break;
            
            //  59: ld e,c (M:1 T:4)
            // -- OVERLAP
            case 0x00D0: cpu->e=cpu->c;_fetch(); break;
            
            //  5A: ld e,d (M:1 T:4)
            // -- OVERLAP
            case 0x00D1: cpu->e=cpu->d;_fetch(); break;
            
            //  5B: ld e,e (M:1 T:4)
            // -- OVERLAP
            case 0x00D2: cpu->e=cpu->e;_fetch(); break;
            
            //  5C: ld e,h (M:1 T:4)
            // -- OVERLAP
            case 0x00D3: cpu->e=cpu->hlx[cpu->hlx_idx].h;_fetch(); break;
            
            //  5D: ld e,l (M:1 T:4)
            // -- OVERLAP
            case 0x00D4: cpu->e=cpu->hlx[cpu->hlx_idx].l;_fetch(); break;
            
            //  5E: ld e,(hl) (M:2 T:7)
            // -- M2
            case 0x00D5: _wait();_mread(cpu->addr); break;
            case 0x00D6: cpu->e=_gd(); break;
            // -- OVERLAP
            case 0x00D7: _fetch(); break;
            
            //  5F: ld e,a (M:1 T:4)
            // -- OVERLAP
            case 0x00D8: cpu->e=cpu->a;_fetch(); break;
            
            //  60: ld h,b (M:1 T:4)
            // -- OVERLAP
            case 0x00D9: cpu->hlx[cpu->hlx_idx].h=cpu->b;_fetch(); break;
            
            //  61: ld h,c (M:1 T:4)
            // -- OVERLAP
            case 0x00DA: cpu->hlx[cpu->hlx_idx].h=cpu->c;_fetch(); break;
            
            //  62: ld h,d (M:1 T:4)
            // -- OVERLAP
            case 0x00DB: cpu->hlx[cpu->hlx_idx].h=cpu->d;_fetch(); break;
            
            //  63: ld h,e (M:1 T:4)
            // -- OVERLAP
            case 0x00DC: cpu->hlx[cpu->hlx_idx].h=cpu->e;_fetch(); break;
            
            //  64: ld h,h (M:1 T:4)
            // -- OVERLAP
            case 0x00DD: cpu->hlx[cpu->hlx_idx].h=cpu->hlx[cpu->hlx_idx].h;_fetch(); break;
            
            //  65: ld h,l (M:1 T:4)
            // -- OVERLAP
            case 0x00DE: cpu->hlx[cpu->hlx_idx].h=cpu->hlx[cpu->hlx_idx].l;_fetch(); break;
            
            //  66: ld h,(hl) (M:2 T:7)
            // -- M2
            case 0x00DF: _wait();_mread(cpu->addr); break;
            case 0x00E0: cpu->h=_gd(); break;
            // -- OVERLAP
            case 0x00E1: _fetch(); break;
            
            //  67: ld h,a (M:1 T:4)
            // -- OVERLAP
            case 0x00E2: cpu->hlx[cpu->hlx_idx].h=cpu->a;_fetch(); break;
            
            //  68: ld l,b (M:1 T:4)
            // -- OVERLAP
            case 0x00E3: cpu->hlx[cpu->hlx_idx].l=cpu->b;_fetch(); break;
            
            //  69: ld l,c (M:1 T:4)
            // -- OVERLAP
            case 0x00E4: cpu->hlx[cpu->hlx_idx].l=cpu->c;_fetch(); break;
            
            //  6A: ld l,d (M:1 T:4)
            // -- OVERLAP
            case 0x00E5: cpu->hlx[cpu->hlx_idx].l=cpu->d;_fetch(); break;
            
            //  6B: ld l,e (M:1 T:4)
            // -- OVERLAP
            case 0x00E6: cpu->hlx[cpu->hlx_idx].l=cpu->e;_fetch(); break;
            
            //  6C: ld l,h (M:1 T:4)
            // -- OVERLAP
            case 0x00E7: cpu->hlx[cpu->hlx_idx].l=cpu->hlx[cpu->hlx_idx].h;_fetch(); break;
            
            //  6D: ld l,l (M:1 T:4)
            // -- OVERLAP
            case 0x00E8: cpu->hlx[cpu->hlx_idx].l=cpu->hlx[cpu->hlx_idx].l;_fetch(); break;
            
            //  6E: ld l,(hl) (M:2 T:7)
            // -- M2
            case 0x00E9: _wait();_mread(cpu->addr); break;
            case 0x00EA: cpu->l=_gd(); break;
            // -- OVERLAP
            case 0x00EB: _fetch(); break;
            
            //  6F: ld l,a (M:1 T:4)
            // -- OVERLAP
            case 0x00EC: cpu->hlx[cpu->hlx_idx].l=cpu->a;_fetch(); break;
            
            //  70: ld (hl),b (M:2 T:7)
            // -- M2
            case 0x00ED: _mwrite(cpu->addr,cpu->b); break;
            case 0x00EE: _wait() break;
            // -- OVERLAP
            case 0x00EF: _fetch(); break;
            
            //  71: ld (hl),c (M:2 T:7)
            // -- M2
            case 0x00F0: _mwrite(cpu->addr,cpu->c); break;
            case 0x00F1: _wait() break;
            // -- OVERLAP
            case 0x00F2: _fetch(); break;
            
            //  72: ld (hl),d (M:2 T:7)
            // -- M2
            case 0x00F3: _mwrite(cpu->addr,cpu->d); break;
            case 0x00F4: _wait() break;
            // -- OVERLAP
            case 0x00F5: _fetch(); break;
            
            //  73: ld (hl),e (M:2 T:7)
            // -- M2
            case 0x00F6: _mwrite(cpu->addr,cpu->e); break;
            case 0x00F7: _wait() break;
            // -- OVERLAP
            case 0x00F8: _fetch(); break;
            
            //  74: ld (hl),h (M:2 T:7)
            // -- M2
            case 0x00F9: _mwrite(cpu->addr,cpu->h); break;
            case 0x00FA: _wait() break;
            // -- OVERLAP
            case 0x00FB: _fetch(); break;
            
            //  75: ld (hl),l (M:2 T:7)
            // -- M2
            case 0x00FC: _mwrite(cpu->addr,cpu->l); break;
            case 0x00FD: _wait() break;
            // -- OVERLAP
            case 0x00FE: _fetch(); break;
            
            //  76: halt (M:1 T:4)
            // -- OVERLAP
            case 0x00FF: pins=z80_halt(cpu,pins);_fetch(); break;
            
            //  77: ld (hl),a (M:2 T:7)
            // -- M2
            case 0x0100: _mwrite(cpu->addr,cpu->a); break;
            case 0x0101: _wait() break;
            // -- OVERLAP
            case 0x0102: _fetch(); break;
            
            //  78: ld a,b (M:1 T:4)
            // -- OVERLAP
            case 0x0103: cpu->a=cpu->b;_fetch(); break;
            
            //  79: ld a,c (M:1 T:4)
            // -- OVERLAP
            case 0x0104: cpu->a=cpu->c;_fetch(); break;
            
            //  7A: ld a,d (M:1 T:4)
            // -- OVERLAP
            case 0x0105: cpu->a=cpu->d;_fetch(); break;
            
            //  7B: ld a,e (M:1 T:4)
            // -- OVERLAP
            case 0x0106: cpu->a=cpu->e;_fetch(); break;
            
            //  7C: ld a,h (M:1 T:4)
            // -- OVERLAP
            case 0x0107: cpu->a=cpu->hlx[cpu->hlx_idx].h;_fetch(); break;
            
            //  7D: ld a,l (M:1 T:4)
            // -- OVERLAP
            case 0x0108: cpu->a=cpu->hlx[cpu->hlx_idx].l;_fetch(); break;
            
            //  7E: ld a,(hl) (M:2 T:7)
            // -- M2
            case 0x0109: _wait();_mread(cpu->addr); break;
            case 0x010A: cpu->a=_gd(); break;
            // -- OVERLAP
            case 0x010B: _fetch(); break;
            
            //  7F: ld a,a (M:1 T:4)
            // -- OVERLAP
            case 0x010C: cpu->a=cpu->a;_fetch(); break;
            
            //  80: add b (M:1 T:4)
            // -- OVERLAP
            case 0x010D: z80_add8(cpu,cpu->b);_fetch(); break;
            
            //  81: add c (M:1 T:4)
            // -- OVERLAP
            case 0x010E: z80_add8(cpu,cpu->c);_fetch(); break;
            
            //  82: add d (M:1 T:4)
            // -- OVERLAP
            case 0x010F: z80_add8(cpu,cpu->d);_fetch(); break;
            
            //  83: add e (M:1 T:4)
            // -- OVERLAP
            case 0x0110: z80_add8(cpu,cpu->e);_fetch(); break;
            
            //  84: add h (M:1 T:4)
            // -- OVERLAP
            case 0x0111: z80_add8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); break;
            
            //  85: add l (M:1 T:4)
            // -- OVERLAP
            case 0x0112: z80_add8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); break;
            
            //  86: add (hl) (M:2 T:7)
            // -- M2
            case 0x0113: _wait();_mread(cpu->addr); break;
            case 0x0114: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x0115: z80_add8(cpu,cpu->dlatch);_fetch(); break;
            
            //  87: add a (M:1 T:4)
            // -- OVERLAP
            case 0x0116: z80_add8(cpu,cpu->a);_fetch(); break;
            
            //  88: adc b (M:1 T:4)
            // -- OVERLAP
            case 0x0117: z80_adc8(cpu,cpu->b);_fetch(); break;
            
            //  89: adc c (M:1 T:4)
            // -- OVERLAP
            case 0x0118: z80_adc8(cpu,cpu->c);_fetch(); break;
            
            //  8A: adc d (M:1 T:4)
            // -- OVERLAP
            case 0x0119: z80_adc8(cpu,cpu->d);_fetch(); break;
            
            //  8B: adc e (M:1 T:4)
            // -- OVERLAP
            case 0x011A: z80_adc8(cpu,cpu->e);_fetch(); break;
            
            //  8C: adc h (M:1 T:4)
            // -- OVERLAP
            case 0x011B: z80_adc8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); break;
            
            //  8D: adc l (M:1 T:4)
            // -- OVERLAP
            case 0x011C: z80_adc8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); break;
            
            //  8E: adc (hl) (M:2 T:7)
            // -- M2
            case 0x011D: _wait();_mread(cpu->addr); break;
            case 0x011E: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x011F: z80_adc8(cpu,cpu->dlatch);_fetch(); break;
            
            //  8F: adc a (M:1 T:4)
            // -- OVERLAP
            case 0x0120: z80_adc8(cpu,cpu->a);_fetch(); break;
            
            //  90: sub b (M:1 T:4)
            // -- OVERLAP
            case 0x0121: z80_sub8(cpu,cpu->b);_fetch(); break;
            
            //  91: sub c (M:1 T:4)
            // -- OVERLAP
            case 0x0122: z80_sub8(cpu,cpu->c);_fetch(); break;
            
            //  92: sub d (M:1 T:4)
            // -- OVERLAP
            case 0x0123: z80_sub8(cpu,cpu->d);_fetch(); break;
            
            //  93: sub e (M:1 T:4)
            // -- OVERLAP
            case 0x0124: z80_sub8(cpu,cpu->e);_fetch(); break;
            
            //  94: sub h (M:1 T:4)
            // -- OVERLAP
            case 0x0125: z80_sub8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); break;
            
            //  95: sub l (M:1 T:4)
            // -- OVERLAP
            case 0x0126: z80_sub8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); break;
            
            //  96: sub (hl) (M:2 T:7)
            // -- M2
            case 0x0127: _wait();_mread(cpu->addr); break;
            case 0x0128: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x0129: z80_sub8(cpu,cpu->dlatch);_fetch(); break;
            
            //  97: sub a (M:1 T:4)
            // -- OVERLAP
            case 0x012A: z80_sub8(cpu,cpu->a);_fetch(); break;
            
            //  98: sbc b (M:1 T:4)
            // -- OVERLAP
            case 0x012B: z80_sbc8(cpu,cpu->b);_fetch(); break;
            
            //  99: sbc c (M:1 T:4)
            // -- OVERLAP
            case 0x012C: z80_sbc8(cpu,cpu->c);_fetch(); break;
            
            //  9A: sbc d (M:1 T:4)
            // -- OVERLAP
            case 0x012D: z80_sbc8(cpu,cpu->d);_fetch(); break;
            
            //  9B: sbc e (M:1 T:4)
            // -- OVERLAP
            case 0x012E: z80_sbc8(cpu,cpu->e);_fetch(); break;
            
            //  9C: sbc h (M:1 T:4)
            // -- OVERLAP
            case 0x012F: z80_sbc8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); break;
            
            //  9D: sbc l (M:1 T:4)
            // -- OVERLAP
            case 0x0130: z80_sbc8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); break;
            
            //  9E: sbc (hl) (M:2 T:7)
            // -- M2
            case 0x0131: _wait();_mread(cpu->addr); break;
            case 0x0132: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x0133: z80_sbc8(cpu,cpu->dlatch);_fetch(); break;
            
            //  9F: sbc a (M:1 T:4)
            // -- OVERLAP
            case 0x0134: z80_sbc8(cpu,cpu->a);_fetch(); break;
            
            //  A0: and b (M:1 T:4)
            // -- OVERLAP
            case 0x0135: z80_and8(cpu,cpu->b);_fetch(); break;
            
            //  A1: and c (M:1 T:4)
            // -- OVERLAP
            case 0x0136: z80_and8(cpu,cpu->c);_fetch(); break;
            
            //  A2: and d (M:1 T:4)
            // -- OVERLAP
            case 0x0137: z80_and8(cpu,cpu->d);_fetch(); break;
            
            //  A3: and e (M:1 T:4)
            // -- OVERLAP
            case 0x0138: z80_and8(cpu,cpu->e);_fetch(); break;
            
            //  A4: and h (M:1 T:4)
            // -- OVERLAP
            case 0x0139: z80_and8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); break;
            
            //  A5: and l (M:1 T:4)
            // -- OVERLAP
            case 0x013A: z80_and8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); break;
            
            //  A6: and (hl) (M:2 T:7)
            // -- M2
            case 0x013B: _wait();_mread(cpu->addr); break;
            case 0x013C: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x013D: z80_and8(cpu,cpu->dlatch);_fetch(); break;
            
            //  A7: and a (M:1 T:4)
            // -- OVERLAP
            case 0x013E: z80_and8(cpu,cpu->a);_fetch(); break;
            
            //  A8: xor b (M:1 T:4)
            // -- OVERLAP
            case 0x013F: z80_xor8(cpu,cpu->b);_fetch(); break;
            
            //  A9: xor c (M:1 T:4)
            // -- OVERLAP
            case 0x0140: z80_xor8(cpu,cpu->c);_fetch(); break;
            
            //  AA: xor d (M:1 T:4)
            // -- OVERLAP
            case 0x0141: z80_xor8(cpu,cpu->d);_fetch(); break;
            
            //  AB: xor e (M:1 T:4)
            // -- OVERLAP
            case 0x0142: z80_xor8(cpu,cpu->e);_fetch(); break;
            
            //  AC: xor h (M:1 T:4)
            // -- OVERLAP
            case 0x0143: z80_xor8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); break;
            
            //  AD: xor l (M:1 T:4)
            // -- OVERLAP
            case 0x0144: z80_xor8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); break;
            
            //  AE: xor (hl) (M:2 T:7)
            // -- M2
            case 0x0145: _wait();_mread(cpu->addr); break;
            case 0x0146: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x0147: z80_xor8(cpu,cpu->dlatch);_fetch(); break;
            
            //  AF: xor a (M:1 T:4)
            // -- OVERLAP
            case 0x0148: z80_xor8(cpu,cpu->a);_fetch(); break;
            
            //  B0: or b (M:1 T:4)
            // -- OVERLAP
            case 0x0149: z80_or8(cpu,cpu->b);_fetch(); break;
            
            //  B1: or c (M:1 T:4)
            // -- OVERLAP
            case 0x014A: z80_or8(cpu,cpu->c);_fetch(); break;
            
            //  B2: or d (M:1 T:4)
            // -- OVERLAP
            case 0x014B: z80_or8(cpu,cpu->d);_fetch(); break;
            
            //  B3: or e (M:1 T:4)
            // -- OVERLAP
            case 0x014C: z80_or8(cpu,cpu->e);_fetch(); break;
            
            //  B4: or h (M:1 T:4)
            // -- OVERLAP
            case 0x014D: z80_or8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); break;
            
            //  B5: or l (M:1 T:4)
            // -- OVERLAP
            case 0x014E: z80_or8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); break;
            
            //  B6: or (hl) (M:2 T:7)
            // -- M2
            case 0x014F: _wait();_mread(cpu->addr); break;
            case 0x0150: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x0151: z80_or8(cpu,cpu->dlatch);_fetch(); break;
            
            //  B7: or a (M:1 T:4)
            // -- OVERLAP
            case 0x0152: z80_or8(cpu,cpu->a);_fetch(); break;
            
            //  B8: cp b (M:1 T:4)
            // -- OVERLAP
            case 0x0153: z80_cp8(cpu,cpu->b);_fetch(); break;
            
            //  B9: cp c (M:1 T:4)
            // -- OVERLAP
            case 0x0154: z80_cp8(cpu,cpu->c);_fetch(); break;
            
            //  BA: cp d (M:1 T:4)
            // -- OVERLAP
            case 0x0155: z80_cp8(cpu,cpu->d);_fetch(); break;
            
            //  BB: cp e (M:1 T:4)
            // -- OVERLAP
            case 0x0156: z80_cp8(cpu,cpu->e);_fetch(); break;
            
            //  BC: cp h (M:1 T:4)
            // -- OVERLAP
            case 0x0157: z80_cp8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); break;
            
            //  BD: cp l (M:1 T:4)
            // -- OVERLAP
            case 0x0158: z80_cp8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); break;
            
            //  BE: cp (hl) (M:2 T:7)
            // -- M2
            case 0x0159: _wait();_mread(cpu->addr); break;
            case 0x015A: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x015B: z80_cp8(cpu,cpu->dlatch);_fetch(); break;
            
            //  BF: cp a (M:1 T:4)
            // -- OVERLAP
            case 0x015C: z80_cp8(cpu,cpu->a);_fetch(); break;
            
            //  C0: ret nz (M:4 T:11)
            // -- M2 (generic)
            case 0x015D: if(!_cc_nz){z80_skip(cpu,4,7,1);}; break;
            // -- M3
            case 0x015E: _wait();_mread(cpu->sp++); break;
            case 0x015F: cpu->wzl=_gd(); break;
            // -- M4
            case 0x0160: _wait();_mread(cpu->sp++); break;
            case 0x0161: cpu->wzh=_gd();cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x0162: _fetch(); break;
            
            //  C1: pop bc (M:3 T:10)
            // -- M2
            case 0x0163: _wait();_mread(cpu->sp++); break;
            case 0x0164: cpu->c=_gd(); break;
            // -- M3
            case 0x0165: _wait();_mread(cpu->sp++); break;
            case 0x0166: cpu->b=_gd(); break;
            // -- OVERLAP
            case 0x0167: _fetch(); break;
            
            //  C2: jp nz,nn (M:3 T:10)
            // -- M2
            case 0x0168: _wait();_mread(cpu->pc++); break;
            case 0x0169: cpu->wzl=_gd(); break;
            // -- M3
            case 0x016A: _wait();_mread(cpu->pc++); break;
            case 0x016B: cpu->wzh=_gd();if(_cc_nz){cpu->pc=cpu->wz;}; break;
            // -- OVERLAP
            case 0x016C: _fetch(); break;
            
            //  C3: jp nn (M:3 T:10)
            // -- M2
            case 0x016D: _wait();_mread(cpu->pc++); break;
            case 0x016E: cpu->wzl=_gd(); break;
            // -- M3
            case 0x016F: _wait();_mread(cpu->pc++); break;
            case 0x0170: cpu->wzh=_gd();cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x0171: _fetch(); break;
            
            //  C4: call nz,nn (M:6 T:17)
            // -- M2
            case 0x0172: _wait();_mread(cpu->pc++); break;
            case 0x0173: cpu->wzl=_gd(); break;
            // -- M3
            case 0x0174: _wait();_mread(cpu->pc++); break;
            case 0x0175: cpu->wzh=_gd();if (!_cc_nz){z80_skip(cpu,5,9,2);}; break;
            // -- M4 (generic)
            case 0x0176:  break;
            // -- M5
            case 0x0177: _mwrite(--cpu->sp,cpu->pch); break;
            case 0x0178: _wait() break;
            // -- M6
            case 0x0179: _mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz; break;
            case 0x017A: _wait() break;
            // -- OVERLAP
            case 0x017B: _fetch(); break;
            
            //  C5: push bc (M:3 T:11)
            // -- M2
            case 0x017C: _mwrite(--cpu->sp,cpu->b); break;
            case 0x017D: _wait() break;
            // -- M3
            case 0x017E: _mwrite(--cpu->sp,cpu->c); break;
            case 0x017F: _wait() break;
            // -- OVERLAP
            case 0x0180: _fetch(); break;
            
            //  C6: add n (M:2 T:7)
            // -- M2
            case 0x0181: _wait();_mread(cpu->pc++); break;
            case 0x0182: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x0183: z80_add8(cpu,cpu->dlatch);_fetch(); break;
            
            //  C7: rst 0h (M:3 T:11)
            // -- M2
            case 0x0184: _mwrite(--cpu->sp,cpu->pch); break;
            case 0x0185: _wait() break;
            // -- M3
            case 0x0186: _mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x00;cpu->pc=cpu->wz; break;
            case 0x0187: _wait() break;
            // -- OVERLAP
            case 0x0188: _fetch(); break;
            
            //  C8: ret z (M:4 T:11)
            // -- M2 (generic)
            case 0x0189: if(!_cc_z){z80_skip(cpu,4,7,1);}; break;
            // -- M3
            case 0x018A: _wait();_mread(cpu->sp++); break;
            case 0x018B: cpu->wzl=_gd(); break;
            // -- M4
            case 0x018C: _wait();_mread(cpu->sp++); break;
            case 0x018D: cpu->wzh=_gd();cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x018E: _fetch(); break;
            
            //  C9: ret (M:3 T:10)
            // -- M2
            case 0x018F: _wait();_mread(cpu->sp++); break;
            case 0x0190: cpu->wzl=_gd(); break;
            // -- M3
            case 0x0191: _wait();_mread(cpu->sp++); break;
            case 0x0192: cpu->wzh=_gd();cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x0193: _fetch(); break;
            
            //  CA: jp z,nn (M:3 T:10)
            // -- M2
            case 0x0194: _wait();_mread(cpu->pc++); break;
            case 0x0195: cpu->wzl=_gd(); break;
            // -- M3
            case 0x0196: _wait();_mread(cpu->pc++); break;
            case 0x0197: cpu->wzh=_gd();if(_cc_z){cpu->pc=cpu->wz;}; break;
            // -- OVERLAP
            case 0x0198: _fetch(); break;
            
            //  CB: cb prefix (M:1 T:4)
            // -- OVERLAP
            case 0x0199: _fetch_cb(); break;
            
            //  CC: call z,nn (M:6 T:17)
            // -- M2
            case 0x019A: _wait();_mread(cpu->pc++); break;
            case 0x019B: cpu->wzl=_gd(); break;
            // -- M3
            case 0x019C: _wait();_mread(cpu->pc++); break;
            case 0x019D: cpu->wzh=_gd();if (!_cc_z){z80_skip(cpu,5,9,2);}; break;
            // -- M4 (generic)
            case 0x019E:  break;
            // -- M5
            case 0x019F: _mwrite(--cpu->sp,cpu->pch); break;
            case 0x01A0: _wait() break;
            // -- M6
            case 0x01A1: _mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz; break;
            case 0x01A2: _wait() break;
            // -- OVERLAP
            case 0x01A3: _fetch(); break;
            
            //  CD: call nn (M:5 T:17)
            // -- M2
            case 0x01A4: _wait();_mread(cpu->pc++); break;
            case 0x01A5: cpu->wzl=_gd(); break;
            // -- M3
            case 0x01A6: _wait();_mread(cpu->pc++); break;
            case 0x01A7: cpu->wzh=_gd(); break;
            // -- M4
            case 0x01A8: _mwrite(--cpu->sp,cpu->pch); break;
            case 0x01A9: _wait() break;
            // -- M5
            case 0x01AA: _mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz; break;
            case 0x01AB: _wait() break;
            // -- OVERLAP
            case 0x01AC: _fetch(); break;
            
            //  CE: adc n (M:2 T:7)
            // -- M2
            case 0x01AD: _wait();_mread(cpu->pc++); break;
            case 0x01AE: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x01AF: z80_adc8(cpu,cpu->dlatch);_fetch(); break;
            
            //  CF: rst 8h (M:3 T:11)
            // -- M2
            case 0x01B0: _mwrite(--cpu->sp,cpu->pch); break;
            case 0x01B1: _wait() break;
            // -- M3
            case 0x01B2: _mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x08;cpu->pc=cpu->wz; break;
            case 0x01B3: _wait() break;
            // -- OVERLAP
            case 0x01B4: _fetch(); break;
            
            //  D0: ret nc (M:4 T:11)
            // -- M2 (generic)
            case 0x01B5: if(!_cc_nc){z80_skip(cpu,4,7,1);}; break;
            // -- M3
            case 0x01B6: _wait();_mread(cpu->sp++); break;
            case 0x01B7: cpu->wzl=_gd(); break;
            // -- M4
            case 0x01B8: _wait();_mread(cpu->sp++); break;
            case 0x01B9: cpu->wzh=_gd();cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x01BA: _fetch(); break;
            
            //  D1: pop de (M:3 T:10)
            // -- M2
            case 0x01BB: _wait();_mread(cpu->sp++); break;
            case 0x01BC: cpu->e=_gd(); break;
            // -- M3
            case 0x01BD: _wait();_mread(cpu->sp++); break;
            case 0x01BE: cpu->d=_gd(); break;
            // -- OVERLAP
            case 0x01BF: _fetch(); break;
            
            //  D2: jp nc,nn (M:3 T:10)
            // -- M2
            case 0x01C0: _wait();_mread(cpu->pc++); break;
            case 0x01C1: cpu->wzl=_gd(); break;
            // -- M3
            case 0x01C2: _wait();_mread(cpu->pc++); break;
            case 0x01C3: cpu->wzh=_gd();if(_cc_nc){cpu->pc=cpu->wz;}; break;
            // -- OVERLAP
            case 0x01C4: _fetch(); break;
            
            //  D3: out (n),a (M:3 T:11)
            // -- M2
            case 0x01C5: _wait();_mread(cpu->pc++); break;
            case 0x01C6: cpu->wzl=_gd();cpu->wzh=cpu->a; break;
            // -- M3 (iowrite)
            case 0x01C7: _iowrite(cpu->wz,cpu->a);cpu->wzl++; break;
            case 0x01C8: _wait() break;
            // -- OVERLAP
            case 0x01C9: _fetch(); break;
            
            //  D4: call nc,nn (M:6 T:17)
            // -- M2
            case 0x01CA: _wait();_mread(cpu->pc++); break;
            case 0x01CB: cpu->wzl=_gd(); break;
            // -- M3
            case 0x01CC: _wait();_mread(cpu->pc++); break;
            case 0x01CD: cpu->wzh=_gd();if (!_cc_nc){z80_skip(cpu,5,9,2);}; break;
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
            
            //  D5: push de (M:3 T:11)
            // -- M2
            case 0x01D4: _mwrite(--cpu->sp,cpu->d); break;
            case 0x01D5: _wait() break;
            // -- M3
            case 0x01D6: _mwrite(--cpu->sp,cpu->e); break;
            case 0x01D7: _wait() break;
            // -- OVERLAP
            case 0x01D8: _fetch(); break;
            
            //  D6: sub n (M:2 T:7)
            // -- M2
            case 0x01D9: _wait();_mread(cpu->pc++); break;
            case 0x01DA: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x01DB: z80_sub8(cpu,cpu->dlatch);_fetch(); break;
            
            //  D7: rst 10h (M:3 T:11)
            // -- M2
            case 0x01DC: _mwrite(--cpu->sp,cpu->pch); break;
            case 0x01DD: _wait() break;
            // -- M3
            case 0x01DE: _mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x10;cpu->pc=cpu->wz; break;
            case 0x01DF: _wait() break;
            // -- OVERLAP
            case 0x01E0: _fetch(); break;
            
            //  D8: ret c (M:4 T:11)
            // -- M2 (generic)
            case 0x01E1: if(!_cc_c){z80_skip(cpu,4,7,1);}; break;
            // -- M3
            case 0x01E2: _wait();_mread(cpu->sp++); break;
            case 0x01E3: cpu->wzl=_gd(); break;
            // -- M4
            case 0x01E4: _wait();_mread(cpu->sp++); break;
            case 0x01E5: cpu->wzh=_gd();cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x01E6: _fetch(); break;
            
            //  D9: exx (M:1 T:4)
            // -- OVERLAP
            case 0x01E7: z80_exx(cpu);_fetch(); break;
            
            //  DA: jp c,nn (M:3 T:10)
            // -- M2
            case 0x01E8: _wait();_mread(cpu->pc++); break;
            case 0x01E9: cpu->wzl=_gd(); break;
            // -- M3
            case 0x01EA: _wait();_mread(cpu->pc++); break;
            case 0x01EB: cpu->wzh=_gd();if(_cc_c){cpu->pc=cpu->wz;}; break;
            // -- OVERLAP
            case 0x01EC: _fetch(); break;
            
            //  DB: in a,(n) (M:3 T:11)
            // -- M2
            case 0x01ED: _wait();_mread(cpu->pc++); break;
            case 0x01EE: cpu->wzl=_gd();cpu->wzh=cpu->a; break;
            // -- M3 (ioread)
            case 0x01EF: _wait();_ioread(cpu->wz++); break;
            case 0x01F0: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x01F1: cpu->a=cpu->dlatch;_fetch(); break;
            
            //  DC: call c,nn (M:6 T:17)
            // -- M2
            case 0x01F2: _wait();_mread(cpu->pc++); break;
            case 0x01F3: cpu->wzl=_gd(); break;
            // -- M3
            case 0x01F4: _wait();_mread(cpu->pc++); break;
            case 0x01F5: cpu->wzh=_gd();if (!_cc_c){z80_skip(cpu,5,9,2);}; break;
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
            
            //  DD: dd prefix (M:1 T:4)
            // -- OVERLAP
            case 0x01FC: _fetch_dd(); break;
            
            //  DE: sbc n (M:2 T:7)
            // -- M2
            case 0x01FD: _wait();_mread(cpu->pc++); break;
            case 0x01FE: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x01FF: z80_sbc8(cpu,cpu->dlatch);_fetch(); break;
            
            //  DF: rst 18h (M:3 T:11)
            // -- M2
            case 0x0200: _mwrite(--cpu->sp,cpu->pch); break;
            case 0x0201: _wait() break;
            // -- M3
            case 0x0202: _mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x18;cpu->pc=cpu->wz; break;
            case 0x0203: _wait() break;
            // -- OVERLAP
            case 0x0204: _fetch(); break;
            
            //  E0: ret po (M:4 T:11)
            // -- M2 (generic)
            case 0x0205: if(!_cc_po){z80_skip(cpu,4,7,1);}; break;
            // -- M3
            case 0x0206: _wait();_mread(cpu->sp++); break;
            case 0x0207: cpu->wzl=_gd(); break;
            // -- M4
            case 0x0208: _wait();_mread(cpu->sp++); break;
            case 0x0209: cpu->wzh=_gd();cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x020A: _fetch(); break;
            
            //  E1: pop hl (M:3 T:10)
            // -- M2
            case 0x020B: _wait();_mread(cpu->sp++); break;
            case 0x020C: cpu->hlx[cpu->hlx_idx].l=_gd(); break;
            // -- M3
            case 0x020D: _wait();_mread(cpu->sp++); break;
            case 0x020E: cpu->hlx[cpu->hlx_idx].h=_gd(); break;
            // -- OVERLAP
            case 0x020F: _fetch(); break;
            
            //  E2: jp po,nn (M:3 T:10)
            // -- M2
            case 0x0210: _wait();_mread(cpu->pc++); break;
            case 0x0211: cpu->wzl=_gd(); break;
            // -- M3
            case 0x0212: _wait();_mread(cpu->pc++); break;
            case 0x0213: cpu->wzh=_gd();if(_cc_po){cpu->pc=cpu->wz;}; break;
            // -- OVERLAP
            case 0x0214: _fetch(); break;
            
            //  E3: ex (sp),hl (M:5 T:19)
            // -- M2
            case 0x0215: _wait();_mread(cpu->sp); break;
            case 0x0216: cpu->wzl=_gd(); break;
            // -- M3
            case 0x0217: _wait();_mread(cpu->sp+1); break;
            case 0x0218: cpu->wzh=_gd(); break;
            // -- M4
            case 0x0219: _mwrite(cpu->sp+1,cpu->hlx[cpu->hlx_idx].h); break;
            case 0x021A: _wait() break;
            // -- M5
            case 0x021B: _mwrite(cpu->sp,cpu->hlx[cpu->hlx_idx].l);cpu->hlx[cpu->hlx_idx].hl=cpu->wz; break;
            case 0x021C: _wait() break;
            // -- OVERLAP
            case 0x021D: _fetch(); break;
            
            //  E4: call po,nn (M:6 T:17)
            // -- M2
            case 0x021E: _wait();_mread(cpu->pc++); break;
            case 0x021F: cpu->wzl=_gd(); break;
            // -- M3
            case 0x0220: _wait();_mread(cpu->pc++); break;
            case 0x0221: cpu->wzh=_gd();if (!_cc_po){z80_skip(cpu,5,9,2);}; break;
            // -- M4 (generic)
            case 0x0222:  break;
            // -- M5
            case 0x0223: _mwrite(--cpu->sp,cpu->pch); break;
            case 0x0224: _wait() break;
            // -- M6
            case 0x0225: _mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz; break;
            case 0x0226: _wait() break;
            // -- OVERLAP
            case 0x0227: _fetch(); break;
            
            //  E5: push hl (M:3 T:11)
            // -- M2
            case 0x0228: _mwrite(--cpu->sp,cpu->hlx[cpu->hlx_idx].h); break;
            case 0x0229: _wait() break;
            // -- M3
            case 0x022A: _mwrite(--cpu->sp,cpu->hlx[cpu->hlx_idx].l); break;
            case 0x022B: _wait() break;
            // -- OVERLAP
            case 0x022C: _fetch(); break;
            
            //  E6: and n (M:2 T:7)
            // -- M2
            case 0x022D: _wait();_mread(cpu->pc++); break;
            case 0x022E: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x022F: z80_and8(cpu,cpu->dlatch);_fetch(); break;
            
            //  E7: rst 20h (M:3 T:11)
            // -- M2
            case 0x0230: _mwrite(--cpu->sp,cpu->pch); break;
            case 0x0231: _wait() break;
            // -- M3
            case 0x0232: _mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x20;cpu->pc=cpu->wz; break;
            case 0x0233: _wait() break;
            // -- OVERLAP
            case 0x0234: _fetch(); break;
            
            //  E8: ret pe (M:4 T:11)
            // -- M2 (generic)
            case 0x0235: if(!_cc_pe){z80_skip(cpu,4,7,1);}; break;
            // -- M3
            case 0x0236: _wait();_mread(cpu->sp++); break;
            case 0x0237: cpu->wzl=_gd(); break;
            // -- M4
            case 0x0238: _wait();_mread(cpu->sp++); break;
            case 0x0239: cpu->wzh=_gd();cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x023A: _fetch(); break;
            
            //  E9: jp hl (M:1 T:4)
            // -- OVERLAP
            case 0x023B: cpu->pc=cpu->hlx[cpu->hlx_idx].hl;_fetch(); break;
            
            //  EA: jp pe,nn (M:3 T:10)
            // -- M2
            case 0x023C: _wait();_mread(cpu->pc++); break;
            case 0x023D: cpu->wzl=_gd(); break;
            // -- M3
            case 0x023E: _wait();_mread(cpu->pc++); break;
            case 0x023F: cpu->wzh=_gd();if(_cc_pe){cpu->pc=cpu->wz;}; break;
            // -- OVERLAP
            case 0x0240: _fetch(); break;
            
            //  EB: ex de,hl (M:1 T:4)
            // -- OVERLAP
            case 0x0241: z80_ex_de_hl(cpu);_fetch(); break;
            
            //  EC: call pe,nn (M:6 T:17)
            // -- M2
            case 0x0242: _wait();_mread(cpu->pc++); break;
            case 0x0243: cpu->wzl=_gd(); break;
            // -- M3
            case 0x0244: _wait();_mread(cpu->pc++); break;
            case 0x0245: cpu->wzh=_gd();if (!_cc_pe){z80_skip(cpu,5,9,2);}; break;
            // -- M4 (generic)
            case 0x0246:  break;
            // -- M5
            case 0x0247: _mwrite(--cpu->sp,cpu->pch); break;
            case 0x0248: _wait() break;
            // -- M6
            case 0x0249: _mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz; break;
            case 0x024A: _wait() break;
            // -- OVERLAP
            case 0x024B: _fetch(); break;
            
            //  ED: ed prefix (M:1 T:4)
            // -- OVERLAP
            case 0x024C: _fetch_ed(); break;
            
            //  EE: xor n (M:2 T:7)
            // -- M2
            case 0x024D: _wait();_mread(cpu->pc++); break;
            case 0x024E: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x024F: z80_xor8(cpu,cpu->dlatch);_fetch(); break;
            
            //  EF: rst 28h (M:3 T:11)
            // -- M2
            case 0x0250: _mwrite(--cpu->sp,cpu->pch); break;
            case 0x0251: _wait() break;
            // -- M3
            case 0x0252: _mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x28;cpu->pc=cpu->wz; break;
            case 0x0253: _wait() break;
            // -- OVERLAP
            case 0x0254: _fetch(); break;
            
            //  F0: ret p (M:4 T:11)
            // -- M2 (generic)
            case 0x0255: if(!_cc_p){z80_skip(cpu,4,7,1);}; break;
            // -- M3
            case 0x0256: _wait();_mread(cpu->sp++); break;
            case 0x0257: cpu->wzl=_gd(); break;
            // -- M4
            case 0x0258: _wait();_mread(cpu->sp++); break;
            case 0x0259: cpu->wzh=_gd();cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x025A: _fetch(); break;
            
            //  F1: pop af (M:3 T:10)
            // -- M2
            case 0x025B: _wait();_mread(cpu->sp++); break;
            case 0x025C: cpu->f=_gd(); break;
            // -- M3
            case 0x025D: _wait();_mread(cpu->sp++); break;
            case 0x025E: cpu->a=_gd(); break;
            // -- OVERLAP
            case 0x025F: _fetch(); break;
            
            //  F2: jp p,nn (M:3 T:10)
            // -- M2
            case 0x0260: _wait();_mread(cpu->pc++); break;
            case 0x0261: cpu->wzl=_gd(); break;
            // -- M3
            case 0x0262: _wait();_mread(cpu->pc++); break;
            case 0x0263: cpu->wzh=_gd();if(_cc_p){cpu->pc=cpu->wz;}; break;
            // -- OVERLAP
            case 0x0264: _fetch(); break;
            
            //  F3: di (M:1 T:4)
            // -- OVERLAP
            case 0x0265: _fetch(); break;
            
            //  F4: call p,nn (M:6 T:17)
            // -- M2
            case 0x0266: _wait();_mread(cpu->pc++); break;
            case 0x0267: cpu->wzl=_gd(); break;
            // -- M3
            case 0x0268: _wait();_mread(cpu->pc++); break;
            case 0x0269: cpu->wzh=_gd();if (!_cc_p){z80_skip(cpu,5,9,2);}; break;
            // -- M4 (generic)
            case 0x026A:  break;
            // -- M5
            case 0x026B: _mwrite(--cpu->sp,cpu->pch); break;
            case 0x026C: _wait() break;
            // -- M6
            case 0x026D: _mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz; break;
            case 0x026E: _wait() break;
            // -- OVERLAP
            case 0x026F: _fetch(); break;
            
            //  F5: push af (M:3 T:11)
            // -- M2
            case 0x0270: _mwrite(--cpu->sp,cpu->a); break;
            case 0x0271: _wait() break;
            // -- M3
            case 0x0272: _mwrite(--cpu->sp,cpu->f); break;
            case 0x0273: _wait() break;
            // -- OVERLAP
            case 0x0274: _fetch(); break;
            
            //  F6: or n (M:2 T:7)
            // -- M2
            case 0x0275: _wait();_mread(cpu->pc++); break;
            case 0x0276: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x0277: z80_or8(cpu,cpu->dlatch);_fetch(); break;
            
            //  F7: rst 30h (M:3 T:11)
            // -- M2
            case 0x0278: _mwrite(--cpu->sp,cpu->pch); break;
            case 0x0279: _wait() break;
            // -- M3
            case 0x027A: _mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x30;cpu->pc=cpu->wz; break;
            case 0x027B: _wait() break;
            // -- OVERLAP
            case 0x027C: _fetch(); break;
            
            //  F8: ret m (M:4 T:11)
            // -- M2 (generic)
            case 0x027D: if(!_cc_m){z80_skip(cpu,4,7,1);}; break;
            // -- M3
            case 0x027E: _wait();_mread(cpu->sp++); break;
            case 0x027F: cpu->wzl=_gd(); break;
            // -- M4
            case 0x0280: _wait();_mread(cpu->sp++); break;
            case 0x0281: cpu->wzh=_gd();cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x0282: _fetch(); break;
            
            //  F9: ld sp,hl (M:2 T:6)
            // -- M2 (generic)
            case 0x0283: cpu->sp=cpu->hlx[cpu->hlx_idx].hl; break;
            // -- OVERLAP
            case 0x0284: _fetch(); break;
            
            //  FA: jp m,nn (M:3 T:10)
            // -- M2
            case 0x0285: _wait();_mread(cpu->pc++); break;
            case 0x0286: cpu->wzl=_gd(); break;
            // -- M3
            case 0x0287: _wait();_mread(cpu->pc++); break;
            case 0x0288: cpu->wzh=_gd();if(_cc_m){cpu->pc=cpu->wz;}; break;
            // -- OVERLAP
            case 0x0289: _fetch(); break;
            
            //  FB: ei (M:1 T:4)
            // -- OVERLAP
            case 0x028A: _fetch(); break;
            
            //  FC: call m,nn (M:6 T:17)
            // -- M2
            case 0x028B: _wait();_mread(cpu->pc++); break;
            case 0x028C: cpu->wzl=_gd(); break;
            // -- M3
            case 0x028D: _wait();_mread(cpu->pc++); break;
            case 0x028E: cpu->wzh=_gd();if (!_cc_m){z80_skip(cpu,5,9,2);}; break;
            // -- M4 (generic)
            case 0x028F:  break;
            // -- M5
            case 0x0290: _mwrite(--cpu->sp,cpu->pch); break;
            case 0x0291: _wait() break;
            // -- M6
            case 0x0292: _mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz; break;
            case 0x0293: _wait() break;
            // -- OVERLAP
            case 0x0294: _fetch(); break;
            
            //  FD: fd prefix (M:1 T:4)
            // -- OVERLAP
            case 0x0295: _fetch_fd(); break;
            
            //  FE: cp n (M:2 T:7)
            // -- M2
            case 0x0296: _wait();_mread(cpu->pc++); break;
            case 0x0297: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x0298: z80_cp8(cpu,cpu->dlatch);_fetch(); break;
            
            //  FF: rst 38h (M:3 T:11)
            // -- M2
            case 0x0299: _mwrite(--cpu->sp,cpu->pch); break;
            case 0x029A: _wait() break;
            // -- M3
            case 0x029B: _mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x38;cpu->pc=cpu->wz; break;
            case 0x029C: _wait() break;
            // -- OVERLAP
            case 0x029D: _fetch(); break;
            
            // ED 00: ed nop (M:1 T:4)
            // -- OVERLAP
            case 0x029E: _fetch(); break;
            
            // ED 40: in b,(c) (M:2 T:8)
            // -- M2 (ioread)
            case 0x029F: _wait();_ioread(cpu->bc); break;
            case 0x02A0: cpu->dlatch=_gd();cpu->wz=cpu->bc+1; break;
            // -- OVERLAP
            case 0x02A1: cpu->b=z80_in(cpu,cpu->dlatch);_fetch(); break;
            
            // ED 41: out (c),b (M:2 T:8)
            // -- M2 (iowrite)
            case 0x02A2: _iowrite(cpu->bc,cpu->b);cpu->wz=cpu->bc+1; break;
            case 0x02A3: _wait() break;
            // -- OVERLAP
            case 0x02A4: _fetch(); break;
            
            // ED 42: sbc hl,bc (M:2 T:11)
            // -- M2 (generic)
            case 0x02A5: z80_sbc16(cpu,cpu->bc); break;
            // -- OVERLAP
            case 0x02A6: _fetch(); break;
            
            // ED 43: ld (nn),bc (M:5 T:16)
            // -- M2
            case 0x02A7: _wait();_mread(cpu->pc++); break;
            case 0x02A8: cpu->wzl=_gd(); break;
            // -- M3
            case 0x02A9: _wait();_mread(cpu->pc++); break;
            case 0x02AA: cpu->wzh=_gd(); break;
            // -- M4
            case 0x02AB: _mwrite(cpu->wz++,cpu->c); break;
            case 0x02AC: _wait() break;
            // -- M5
            case 0x02AD: _mwrite(cpu->wz,cpu->b); break;
            case 0x02AE: _wait() break;
            // -- OVERLAP
            case 0x02AF: _fetch(); break;
            
            // ED 44: neg (M:1 T:4)
            // -- OVERLAP
            case 0x02B0: z80_neg8(cpu);_fetch(); break;
            
            // ED 45: retn (M:1 T:4)
            // -- OVERLAP
            case 0x02B1: _fetch(); break;
            
            // ED 46: im IM0 (M:1 T:4)
            // -- OVERLAP
            case 0x02B2: _fetch(); break;
            
            // ED 47: ld i,a (M:1 T:4)
            // -- OVERLAP
            case 0x02B3: _fetch(); break;
            
            // ED 48: in c,(c) (M:2 T:8)
            // -- M2 (ioread)
            case 0x02B4: _wait();_ioread(cpu->bc); break;
            case 0x02B5: cpu->dlatch=_gd();cpu->wz=cpu->bc+1; break;
            // -- OVERLAP
            case 0x02B6: cpu->c=z80_in(cpu,cpu->dlatch);_fetch(); break;
            
            // ED 49: out (c),c (M:2 T:8)
            // -- M2 (iowrite)
            case 0x02B7: _iowrite(cpu->bc,cpu->c);cpu->wz=cpu->bc+1; break;
            case 0x02B8: _wait() break;
            // -- OVERLAP
            case 0x02B9: _fetch(); break;
            
            // ED 4A: adc hl,bc (M:2 T:11)
            // -- M2 (generic)
            case 0x02BA: z80_adc16(cpu,cpu->bc); break;
            // -- OVERLAP
            case 0x02BB: _fetch(); break;
            
            // ED 4B: ld bc,(nn) (M:5 T:16)
            // -- M2
            case 0x02BC: _wait();_mread(cpu->pc++); break;
            case 0x02BD: cpu->wzl=_gd(); break;
            // -- M3
            case 0x02BE: _wait();_mread(cpu->pc++); break;
            case 0x02BF: cpu->wzh=_gd(); break;
            // -- M4
            case 0x02C0: _wait();_mread(cpu->wz++); break;
            case 0x02C1: cpu->c=_gd(); break;
            // -- M5
            case 0x02C2: _wait();_mread(cpu->wz); break;
            case 0x02C3: cpu->b=_gd(); break;
            // -- OVERLAP
            case 0x02C4: _fetch(); break;
            
            // ED 4D: reti (M:1 T:4)
            // -- OVERLAP
            case 0x02C5: _fetch(); break;
            
            // ED 4E: im IM1 (M:1 T:4)
            // -- OVERLAP
            case 0x02C6: _fetch(); break;
            
            // ED 4F: ld r,a (M:1 T:4)
            // -- OVERLAP
            case 0x02C7: _fetch(); break;
            
            // ED 50: in d,(c) (M:2 T:8)
            // -- M2 (ioread)
            case 0x02C8: _wait();_ioread(cpu->bc); break;
            case 0x02C9: cpu->dlatch=_gd();cpu->wz=cpu->bc+1; break;
            // -- OVERLAP
            case 0x02CA: cpu->d=z80_in(cpu,cpu->dlatch);_fetch(); break;
            
            // ED 51: out (c),d (M:2 T:8)
            // -- M2 (iowrite)
            case 0x02CB: _iowrite(cpu->bc,cpu->d);cpu->wz=cpu->bc+1; break;
            case 0x02CC: _wait() break;
            // -- OVERLAP
            case 0x02CD: _fetch(); break;
            
            // ED 52: sbc hl,de (M:2 T:11)
            // -- M2 (generic)
            case 0x02CE: z80_sbc16(cpu,cpu->de); break;
            // -- OVERLAP
            case 0x02CF: _fetch(); break;
            
            // ED 53: ld (nn),de (M:5 T:16)
            // -- M2
            case 0x02D0: _wait();_mread(cpu->pc++); break;
            case 0x02D1: cpu->wzl=_gd(); break;
            // -- M3
            case 0x02D2: _wait();_mread(cpu->pc++); break;
            case 0x02D3: cpu->wzh=_gd(); break;
            // -- M4
            case 0x02D4: _mwrite(cpu->wz++,cpu->e); break;
            case 0x02D5: _wait() break;
            // -- M5
            case 0x02D6: _mwrite(cpu->wz,cpu->d); break;
            case 0x02D7: _wait() break;
            // -- OVERLAP
            case 0x02D8: _fetch(); break;
            
            // ED 56: im IM2 (M:1 T:4)
            // -- OVERLAP
            case 0x02D9: _fetch(); break;
            
            // ED 57: ld a,i (M:1 T:4)
            // -- OVERLAP
            case 0x02DA: _fetch(); break;
            
            // ED 58: in e,(c) (M:2 T:8)
            // -- M2 (ioread)
            case 0x02DB: _wait();_ioread(cpu->bc); break;
            case 0x02DC: cpu->dlatch=_gd();cpu->wz=cpu->bc+1; break;
            // -- OVERLAP
            case 0x02DD: cpu->e=z80_in(cpu,cpu->dlatch);_fetch(); break;
            
            // ED 59: out (c),e (M:2 T:8)
            // -- M2 (iowrite)
            case 0x02DE: _iowrite(cpu->bc,cpu->e);cpu->wz=cpu->bc+1; break;
            case 0x02DF: _wait() break;
            // -- OVERLAP
            case 0x02E0: _fetch(); break;
            
            // ED 5A: adc hl,de (M:2 T:11)
            // -- M2 (generic)
            case 0x02E1: z80_adc16(cpu,cpu->de); break;
            // -- OVERLAP
            case 0x02E2: _fetch(); break;
            
            // ED 5B: ld de,(nn) (M:5 T:16)
            // -- M2
            case 0x02E3: _wait();_mread(cpu->pc++); break;
            case 0x02E4: cpu->wzl=_gd(); break;
            // -- M3
            case 0x02E5: _wait();_mread(cpu->pc++); break;
            case 0x02E6: cpu->wzh=_gd(); break;
            // -- M4
            case 0x02E7: _wait();_mread(cpu->wz++); break;
            case 0x02E8: cpu->e=_gd(); break;
            // -- M5
            case 0x02E9: _wait();_mread(cpu->wz); break;
            case 0x02EA: cpu->d=_gd(); break;
            // -- OVERLAP
            case 0x02EB: _fetch(); break;
            
            // ED 5E: im IM3 (M:1 T:4)
            // -- OVERLAP
            case 0x02EC: _fetch(); break;
            
            // ED 5F: ld a,r (M:1 T:4)
            // -- OVERLAP
            case 0x02ED: _fetch(); break;
            
            // ED 60: in h,(c) (M:2 T:8)
            // -- M2 (ioread)
            case 0x02EE: _wait();_ioread(cpu->bc); break;
            case 0x02EF: cpu->dlatch=_gd();cpu->wz=cpu->bc+1; break;
            // -- OVERLAP
            case 0x02F0: cpu->hlx[cpu->hlx_idx].h=z80_in(cpu,cpu->dlatch);_fetch(); break;
            
            // ED 61: out (c),h (M:2 T:8)
            // -- M2 (iowrite)
            case 0x02F1: _iowrite(cpu->bc,cpu->hlx[cpu->hlx_idx].h);cpu->wz=cpu->bc+1; break;
            case 0x02F2: _wait() break;
            // -- OVERLAP
            case 0x02F3: _fetch(); break;
            
            // ED 62: sbc hl,hl (M:2 T:11)
            // -- M2 (generic)
            case 0x02F4: z80_sbc16(cpu,cpu->hl); break;
            // -- OVERLAP
            case 0x02F5: _fetch(); break;
            
            // ED 63: ld (nn),hl (M:5 T:16)
            // -- M2
            case 0x02F6: _wait();_mread(cpu->pc++); break;
            case 0x02F7: cpu->wzl=_gd(); break;
            // -- M3
            case 0x02F8: _wait();_mread(cpu->pc++); break;
            case 0x02F9: cpu->wzh=_gd(); break;
            // -- M4
            case 0x02FA: _mwrite(cpu->wz++,cpu->l); break;
            case 0x02FB: _wait() break;
            // -- M5
            case 0x02FC: _mwrite(cpu->wz,cpu->h); break;
            case 0x02FD: _wait() break;
            // -- OVERLAP
            case 0x02FE: _fetch(); break;
            
            // ED 66: im IM4 (M:1 T:4)
            // -- OVERLAP
            case 0x02FF: _fetch(); break;
            
            // ED 67: rrd (M:1 T:4)
            // -- OVERLAP
            case 0x0300: _fetch(); break;
            
            // ED 68: in l,(c) (M:2 T:8)
            // -- M2 (ioread)
            case 0x0301: _wait();_ioread(cpu->bc); break;
            case 0x0302: cpu->dlatch=_gd();cpu->wz=cpu->bc+1; break;
            // -- OVERLAP
            case 0x0303: cpu->hlx[cpu->hlx_idx].l=z80_in(cpu,cpu->dlatch);_fetch(); break;
            
            // ED 69: out (c),l (M:2 T:8)
            // -- M2 (iowrite)
            case 0x0304: _iowrite(cpu->bc,cpu->hlx[cpu->hlx_idx].l);cpu->wz=cpu->bc+1; break;
            case 0x0305: _wait() break;
            // -- OVERLAP
            case 0x0306: _fetch(); break;
            
            // ED 6A: adc hl,hl (M:2 T:11)
            // -- M2 (generic)
            case 0x0307: z80_adc16(cpu,cpu->hl); break;
            // -- OVERLAP
            case 0x0308: _fetch(); break;
            
            // ED 6B: ld hl,(nn) (M:5 T:16)
            // -- M2
            case 0x0309: _wait();_mread(cpu->pc++); break;
            case 0x030A: cpu->wzl=_gd(); break;
            // -- M3
            case 0x030B: _wait();_mread(cpu->pc++); break;
            case 0x030C: cpu->wzh=_gd(); break;
            // -- M4
            case 0x030D: _wait();_mread(cpu->wz++); break;
            case 0x030E: cpu->l=_gd(); break;
            // -- M5
            case 0x030F: _wait();_mread(cpu->wz); break;
            case 0x0310: cpu->h=_gd(); break;
            // -- OVERLAP
            case 0x0311: _fetch(); break;
            
            // ED 6E: im IM5 (M:1 T:4)
            // -- OVERLAP
            case 0x0312: _fetch(); break;
            
            // ED 6F: rld (M:1 T:4)
            // -- OVERLAP
            case 0x0313: _fetch(); break;
            
            // ED 70: in (c) (M:2 T:8)
            // -- M2 (ioread)
            case 0x0314: _wait();_ioread(cpu->bc); break;
            case 0x0315: cpu->dlatch=_gd();cpu->wz=cpu->bc+1; break;
            // -- OVERLAP
            case 0x0316: z80_in(cpu,cpu->dlatch);_fetch(); break;
            
            // ED 71: out (c),0 (M:2 T:8)
            // -- M2 (iowrite)
            case 0x0317: _iowrite(cpu->bc,0);cpu->wz=cpu->bc+1; break;
            case 0x0318: _wait() break;
            // -- OVERLAP
            case 0x0319: _fetch(); break;
            
            // ED 72: sbc hl,sp (M:2 T:11)
            // -- M2 (generic)
            case 0x031A: z80_sbc16(cpu,cpu->sp); break;
            // -- OVERLAP
            case 0x031B: _fetch(); break;
            
            // ED 73: ld (nn),sp (M:5 T:16)
            // -- M2
            case 0x031C: _wait();_mread(cpu->pc++); break;
            case 0x031D: cpu->wzl=_gd(); break;
            // -- M3
            case 0x031E: _wait();_mread(cpu->pc++); break;
            case 0x031F: cpu->wzh=_gd(); break;
            // -- M4
            case 0x0320: _mwrite(cpu->wz++,cpu->spl); break;
            case 0x0321: _wait() break;
            // -- M5
            case 0x0322: _mwrite(cpu->wz,cpu->sph); break;
            case 0x0323: _wait() break;
            // -- OVERLAP
            case 0x0324: _fetch(); break;
            
            // ED 76: im IM6 (M:1 T:4)
            // -- OVERLAP
            case 0x0325: _fetch(); break;
            
            // ED 78: in a,(c) (M:2 T:8)
            // -- M2 (ioread)
            case 0x0326: _wait();_ioread(cpu->bc); break;
            case 0x0327: cpu->dlatch=_gd();cpu->wz=cpu->bc+1; break;
            // -- OVERLAP
            case 0x0328: cpu->a=z80_in(cpu,cpu->dlatch);_fetch(); break;
            
            // ED 79: out (c),a (M:2 T:8)
            // -- M2 (iowrite)
            case 0x0329: _iowrite(cpu->bc,cpu->a);cpu->wz=cpu->bc+1; break;
            case 0x032A: _wait() break;
            // -- OVERLAP
            case 0x032B: _fetch(); break;
            
            // ED 7A: adc hl,sp (M:2 T:11)
            // -- M2 (generic)
            case 0x032C: z80_adc16(cpu,cpu->sp); break;
            // -- OVERLAP
            case 0x032D: _fetch(); break;
            
            // ED 7B: ld sp,(nn) (M:5 T:16)
            // -- M2
            case 0x032E: _wait();_mread(cpu->pc++); break;
            case 0x032F: cpu->wzl=_gd(); break;
            // -- M3
            case 0x0330: _wait();_mread(cpu->pc++); break;
            case 0x0331: cpu->wzh=_gd(); break;
            // -- M4
            case 0x0332: _wait();_mread(cpu->wz++); break;
            case 0x0333: cpu->spl=_gd(); break;
            // -- M5
            case 0x0334: _wait();_mread(cpu->wz); break;
            case 0x0335: cpu->sph=_gd(); break;
            // -- OVERLAP
            case 0x0336: _fetch(); break;
            
            // ED 7E: im IM7 (M:1 T:4)
            // -- OVERLAP
            case 0x0337: _fetch(); break;
            
            // ED A0: ldi (M:4 T:12)
            // -- M2
            case 0x0338: _wait();_mread(cpu->hl++); break;
            case 0x0339: cpu->dlatch=_gd(); break;
            // -- M3
            case 0x033A: _mwrite(cpu->de++,cpu->dlatch); break;
            case 0x033B: _wait() break;
            // -- M4 (generic)
            case 0x033C: z80_ldi_ldd(cpu); break;
            // -- OVERLAP
            case 0x033D: _fetch(); break;
            
            // ED A1: cpi (M:3 T:12)
            // -- M2
            case 0x033E: _wait();_mread(cpu->hl++); break;
            case 0x033F: cpu->dlatch=_gd(); break;
            // -- M3 (generic)
            case 0x0340: cpu->wz++;z80_cpi_cpd(cpu); break;
            // -- OVERLAP
            case 0x0341: _fetch(); break;
            
            // ED A2: ini (M:1 T:4)
            // -- OVERLAP
            case 0x0342: _fetch(); break;
            
            // ED A3: outi (M:1 T:4)
            // -- OVERLAP
            case 0x0343: _fetch(); break;
            
            // ED A8: ldd (M:4 T:12)
            // -- M2
            case 0x0344: _wait();_mread(cpu->hl--); break;
            case 0x0345: cpu->dlatch=_gd(); break;
            // -- M3
            case 0x0346: _mwrite(cpu->de--,cpu->dlatch); break;
            case 0x0347: _wait() break;
            // -- M4 (generic)
            case 0x0348: z80_ldi_ldd(cpu); break;
            // -- OVERLAP
            case 0x0349: _fetch(); break;
            
            // ED A9: cpd (M:3 T:12)
            // -- M2
            case 0x034A: _wait();_mread(cpu->hl--); break;
            case 0x034B: cpu->dlatch=_gd(); break;
            // -- M3 (generic)
            case 0x034C: cpu->wz--;z80_cpi_cpd(cpu); break;
            // -- OVERLAP
            case 0x034D: _fetch(); break;
            
            // ED AA: ind (M:1 T:4)
            // -- OVERLAP
            case 0x034E: _fetch(); break;
            
            // ED AB: outd (M:1 T:4)
            // -- OVERLAP
            case 0x034F: _fetch(); break;
            
            // ED B0: ldir (M:5 T:17)
            // -- M2
            case 0x0350: _wait();_mread(cpu->hl++); break;
            case 0x0351: cpu->dlatch=_gd(); break;
            // -- M3
            case 0x0352: _mwrite(cpu->de++,cpu->dlatch); break;
            case 0x0353: _wait() break;
            // -- M4 (generic)
            case 0x0354: if(!z80_ldi_ldd(cpu)){z80_skip(cpu,1,7,2);}; break;
            // -- M5 (generic)
            case 0x0355: cpu->wz=--cpu->pc;--cpu->pc;; break;
            // -- OVERLAP
            case 0x0356: _fetch(); break;
            
            // ED B1: cpir (M:4 T:17)
            // -- M2
            case 0x0357: _wait();_mread(cpu->hl++); break;
            case 0x0358: cpu->dlatch=_gd(); break;
            // -- M3 (generic)
            case 0x0359: cpu->wz++;if(!z80_cpi_cpd(cpu)){z80_skip(cpu,1,7,2);}; break;
            // -- M4 (generic)
            case 0x035A: cpu->wz=--cpu->pc;--cpu->pc; break;
            // -- OVERLAP
            case 0x035B: _fetch(); break;
            
            // ED B2: inir (M:1 T:4)
            // -- OVERLAP
            case 0x035C: _fetch(); break;
            
            // ED B3: otir (M:1 T:4)
            // -- OVERLAP
            case 0x035D: _fetch(); break;
            
            // ED B8: lddr (M:5 T:17)
            // -- M2
            case 0x035E: _wait();_mread(cpu->hl--); break;
            case 0x035F: cpu->dlatch=_gd(); break;
            // -- M3
            case 0x0360: _mwrite(cpu->de--,cpu->dlatch); break;
            case 0x0361: _wait() break;
            // -- M4 (generic)
            case 0x0362: if(!z80_ldi_ldd(cpu)){z80_skip(cpu,1,7,2);}; break;
            // -- M5 (generic)
            case 0x0363: cpu->wz=--cpu->pc;--cpu->pc;; break;
            // -- OVERLAP
            case 0x0364: _fetch(); break;
            
            // ED B9: cpdr (M:4 T:17)
            // -- M2
            case 0x0365: _wait();_mread(cpu->hl--); break;
            case 0x0366: cpu->dlatch=_gd(); break;
            // -- M3 (generic)
            case 0x0367: cpu->wz--;if(!z80_cpi_cpd(cpu)){z80_skip(cpu,1,7,2);}; break;
            // -- M4 (generic)
            case 0x0368: cpu->wz=--cpu->pc;--cpu->pc; break;
            // -- OVERLAP
            case 0x0369: _fetch(); break;
            
            // ED BA: indr (M:1 T:4)
            // -- OVERLAP
            case 0x036A: _fetch(); break;
            
            // ED BB: otdr (M:1 T:4)
            // -- OVERLAP
            case 0x036B: _fetch(); break;
            
            // CB 00: rlc b (M:1 T:4)
            // -- OVERLAP
            case 0x036C: _fetch(); break;
            
            // CB 01: rlc c (M:1 T:4)
            // -- OVERLAP
            case 0x036D: _fetch(); break;
            
            // CB 02: rlc d (M:1 T:4)
            // -- OVERLAP
            case 0x036E: _fetch(); break;
            
            // CB 03: rlc e (M:1 T:4)
            // -- OVERLAP
            case 0x036F: _fetch(); break;
            
            // CB 04: rlc h (M:1 T:4)
            // -- OVERLAP
            case 0x0370: _fetch(); break;
            
            // CB 05: rlc l (M:1 T:4)
            // -- OVERLAP
            case 0x0371: _fetch(); break;
            
            // CB 06: rlc (hl) (M:1 T:4)
            // -- OVERLAP
            case 0x0372: _fetch(); break;
            
            // CB 07: rlc a (M:1 T:4)
            // -- OVERLAP
            case 0x0373: _fetch(); break;
            
            // CB 08: rrc b (M:1 T:4)
            // -- OVERLAP
            case 0x0374: _fetch(); break;
            
            // CB 09: rrc c (M:1 T:4)
            // -- OVERLAP
            case 0x0375: _fetch(); break;
            
            // CB 0A: rrc d (M:1 T:4)
            // -- OVERLAP
            case 0x0376: _fetch(); break;
            
            // CB 0B: rrc e (M:1 T:4)
            // -- OVERLAP
            case 0x0377: _fetch(); break;
            
            // CB 0C: rrc h (M:1 T:4)
            // -- OVERLAP
            case 0x0378: _fetch(); break;
            
            // CB 0D: rrc l (M:1 T:4)
            // -- OVERLAP
            case 0x0379: _fetch(); break;
            
            // CB 0E: rrc (hl) (M:1 T:4)
            // -- OVERLAP
            case 0x037A: _fetch(); break;
            
            // CB 0F: rrc a (M:1 T:4)
            // -- OVERLAP
            case 0x037B: _fetch(); break;
            
            // CB 10: rl b (M:1 T:4)
            // -- OVERLAP
            case 0x037C: _fetch(); break;
            
            // CB 11: rl c (M:1 T:4)
            // -- OVERLAP
            case 0x037D: _fetch(); break;
            
            // CB 12: rl d (M:1 T:4)
            // -- OVERLAP
            case 0x037E: _fetch(); break;
            
            // CB 13: rl e (M:1 T:4)
            // -- OVERLAP
            case 0x037F: _fetch(); break;
            
            // CB 14: rl h (M:1 T:4)
            // -- OVERLAP
            case 0x0380: _fetch(); break;
            
            // CB 15: rl l (M:1 T:4)
            // -- OVERLAP
            case 0x0381: _fetch(); break;
            
            // CB 16: rl (hl) (M:1 T:4)
            // -- OVERLAP
            case 0x0382: _fetch(); break;
            
            // CB 17: rl a (M:1 T:4)
            // -- OVERLAP
            case 0x0383: _fetch(); break;
            
            // CB 18: rr b (M:1 T:4)
            // -- OVERLAP
            case 0x0384: _fetch(); break;
            
            // CB 19: rr c (M:1 T:4)
            // -- OVERLAP
            case 0x0385: _fetch(); break;
            
            // CB 1A: rr d (M:1 T:4)
            // -- OVERLAP
            case 0x0386: _fetch(); break;
            
            // CB 1B: rr e (M:1 T:4)
            // -- OVERLAP
            case 0x0387: _fetch(); break;
            
            // CB 1C: rr h (M:1 T:4)
            // -- OVERLAP
            case 0x0388: _fetch(); break;
            
            // CB 1D: rr l (M:1 T:4)
            // -- OVERLAP
            case 0x0389: _fetch(); break;
            
            // CB 1E: rr (hl) (M:1 T:4)
            // -- OVERLAP
            case 0x038A: _fetch(); break;
            
            // CB 1F: rr a (M:1 T:4)
            // -- OVERLAP
            case 0x038B: _fetch(); break;
            
            // CB 20: sla b (M:1 T:4)
            // -- OVERLAP
            case 0x038C: _fetch(); break;
            
            // CB 21: sla c (M:1 T:4)
            // -- OVERLAP
            case 0x038D: _fetch(); break;
            
            // CB 22: sla d (M:1 T:4)
            // -- OVERLAP
            case 0x038E: _fetch(); break;
            
            // CB 23: sla e (M:1 T:4)
            // -- OVERLAP
            case 0x038F: _fetch(); break;
            
            // CB 24: sla h (M:1 T:4)
            // -- OVERLAP
            case 0x0390: _fetch(); break;
            
            // CB 25: sla l (M:1 T:4)
            // -- OVERLAP
            case 0x0391: _fetch(); break;
            
            // CB 26: sla (hl) (M:1 T:4)
            // -- OVERLAP
            case 0x0392: _fetch(); break;
            
            // CB 27: sla a (M:1 T:4)
            // -- OVERLAP
            case 0x0393: _fetch(); break;
            
            // CB 28: sra b (M:1 T:4)
            // -- OVERLAP
            case 0x0394: _fetch(); break;
            
            // CB 29: sra c (M:1 T:4)
            // -- OVERLAP
            case 0x0395: _fetch(); break;
            
            // CB 2A: sra d (M:1 T:4)
            // -- OVERLAP
            case 0x0396: _fetch(); break;
            
            // CB 2B: sra e (M:1 T:4)
            // -- OVERLAP
            case 0x0397: _fetch(); break;
            
            // CB 2C: sra h (M:1 T:4)
            // -- OVERLAP
            case 0x0398: _fetch(); break;
            
            // CB 2D: sra l (M:1 T:4)
            // -- OVERLAP
            case 0x0399: _fetch(); break;
            
            // CB 2E: sra (hl) (M:1 T:4)
            // -- OVERLAP
            case 0x039A: _fetch(); break;
            
            // CB 2F: sra a (M:1 T:4)
            // -- OVERLAP
            case 0x039B: _fetch(); break;
            
            // CB 30: sll b (M:1 T:4)
            // -- OVERLAP
            case 0x039C: _fetch(); break;
            
            // CB 31: sll c (M:1 T:4)
            // -- OVERLAP
            case 0x039D: _fetch(); break;
            
            // CB 32: sll d (M:1 T:4)
            // -- OVERLAP
            case 0x039E: _fetch(); break;
            
            // CB 33: sll e (M:1 T:4)
            // -- OVERLAP
            case 0x039F: _fetch(); break;
            
            // CB 34: sll h (M:1 T:4)
            // -- OVERLAP
            case 0x03A0: _fetch(); break;
            
            // CB 35: sll l (M:1 T:4)
            // -- OVERLAP
            case 0x03A1: _fetch(); break;
            
            // CB 36: sll (hl) (M:1 T:4)
            // -- OVERLAP
            case 0x03A2: _fetch(); break;
            
            // CB 37: sll a (M:1 T:4)
            // -- OVERLAP
            case 0x03A3: _fetch(); break;
            
            // CB 38: srl b (M:1 T:4)
            // -- OVERLAP
            case 0x03A4: _fetch(); break;
            
            // CB 39: srl c (M:1 T:4)
            // -- OVERLAP
            case 0x03A5: _fetch(); break;
            
            // CB 3A: srl d (M:1 T:4)
            // -- OVERLAP
            case 0x03A6: _fetch(); break;
            
            // CB 3B: srl e (M:1 T:4)
            // -- OVERLAP
            case 0x03A7: _fetch(); break;
            
            // CB 3C: srl h (M:1 T:4)
            // -- OVERLAP
            case 0x03A8: _fetch(); break;
            
            // CB 3D: srl l (M:1 T:4)
            // -- OVERLAP
            case 0x03A9: _fetch(); break;
            
            // CB 3E: srl (hl) (M:1 T:4)
            // -- OVERLAP
            case 0x03AA: _fetch(); break;
            
            // CB 3F: srl a (M:1 T:4)
            // -- OVERLAP
            case 0x03AB: _fetch(); break;
            
            // CB 40: bit n,r (M:1 T:4)
            // -- OVERLAP
            case 0x03AC: _fetch(); break;
            
            // CB 46: bit n,(hl) (M:1 T:4)
            // -- OVERLAP
            case 0x03AD: _fetch(); break;
            
            // CB 80: res n,r (M:1 T:4)
            // -- OVERLAP
            case 0x03AE: _fetch(); break;
            
            // CB 86: res n,(hl) (M:1 T:4)
            // -- OVERLAP
            case 0x03AF: _fetch(); break;
            
            // CB C0: set n,r (M:1 T:4)
            // -- OVERLAP
            case 0x03B0: _fetch(); break;
            
            // CB C6: set n,(hl) (M:1 T:4)
            // -- OVERLAP
            case 0x03B1: _fetch(); break;

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
