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
#define Z80_GET_ADDR(p) ((uint16_t)(p&0xFFFF))
#define Z80_SET_ADDR(p,a) {p=(p&~0xFFFF)|((a)&0xFFFF);}
#define Z80_GET_DATA(p) ((uint8_t)((p>>16)&0xFF))
#define Z80_SET_DATA(p,d) {p=(p&~0xFF0000)|((d<<16)&0xFF0000);}
#define Z80_COPY_DATA(p0,p1) (((p0)&~0xFF0000ULL)|((p1)&0xFF0000ULL))

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
    uint32_t pip;   // the op's decode pipeline
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

// the vanilla M1 machine cycle execution pipeline
#define _Z80_M1_PIP (3)

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

static inline void _z80_skip(z80_t* cpu, int steps, int tcycles, int delay) {
    cpu->op.step += steps;
    cpu->op.pip = (cpu->op.pip >> tcycles) << delay;
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
#define _Z80_OPSTATE_SLOT_CB         (512)
#define _Z80_OPSTATE_SLOT_CBHL       (512+1)
#define _Z80_OPSTATE_SLOT_DDFDCB     (512+2)
#define _Z80_OPSTATE_SLOT_INT_IM0    (512+3)
#define _Z80_OPSTATE_SLOT_INT_IM1    (512+4)
#define _Z80_OPSTATE_SLOT_INT_IM2    (512+5)
#define _Z80_OPSTATE_SLOT_NMI        (512+6)

#define _Z80_OPSTATE_NUM_SPECIAL_OPS (7)

static const z80_opstate_t _z80_opstate_table[2*256 + _Z80_OPSTATE_NUM_SPECIAL_OPS] = {
    { 0x00000004, 0x0006, 0 },  //  00: nop (M:1 T:4 steps:1)
    { 0x0000016C, 0x0007, 0 },  //  01: ld RP,nn (M:3 T:10 steps:5)
    { 0x00000038, 0x000C, 0 },  //  02: ld (bc),a (M:2 T:7 steps:3)
    { 0x00000014, 0x000F, 0 },  //  03: inc RP (M:2 T:6 steps:2)
    { 0x00000004, 0x0011, 0 },  //  04: inc RY (M:1 T:4 steps:1)
    { 0x00000004, 0x0012, 0 },  //  05: dec RY (M:1 T:4 steps:1)
    { 0x0000002C, 0x0013, _Z80_OPSTATE_FLAGS_IMM8 },  //  06: ld RY,n (M:2 T:7 steps:3)
    { 0x00000004, 0x0016, 0 },  //  07: rlca (M:1 T:4 steps:1)
    { 0x00000004, 0x0017, 0 },  //  08: ex af,af' (M:1 T:4 steps:1)
    { 0x00000204, 0x0018, 0 },  //  09: add hl,RP (M:2 T:11 steps:2)
    { 0x0000002C, 0x001A, 0 },  //  0A: ld a,(bc) (M:2 T:7 steps:3)
    { 0x00000014, 0x001D, 0 },  //  0B: dec RP (M:2 T:6 steps:2)
    { 0x00000004, 0x001F, 0 },  //  0C: inc RY (M:1 T:4 steps:1)
    { 0x00000004, 0x0020, 0 },  //  0D: dec RY (M:1 T:4 steps:1)
    { 0x0000002C, 0x0021, _Z80_OPSTATE_FLAGS_IMM8 },  //  0E: ld RY,n (M:2 T:7 steps:3)
    { 0x00000004, 0x0024, 0 },  //  0F: rrca (M:1 T:4 steps:1)
    { 0x00000858, 0x0025, 0 },  //  10: djnz d (M:3 T:13 steps:4)
    { 0x0000016C, 0x0029, 0 },  //  11: ld RP,nn (M:3 T:10 steps:5)
    { 0x00000038, 0x002E, 0 },  //  12: ld (de),a (M:2 T:7 steps:3)
    { 0x00000014, 0x0031, 0 },  //  13: inc RP (M:2 T:6 steps:2)
    { 0x00000004, 0x0033, 0 },  //  14: inc RY (M:1 T:4 steps:1)
    { 0x00000004, 0x0034, 0 },  //  15: dec RY (M:1 T:4 steps:1)
    { 0x0000002C, 0x0035, _Z80_OPSTATE_FLAGS_IMM8 },  //  16: ld RY,n (M:2 T:7 steps:3)
    { 0x00000004, 0x0038, 0 },  //  17: rla (M:1 T:4 steps:1)
    { 0x0000042C, 0x0039, 0 },  //  18: jr d (M:3 T:12 steps:4)
    { 0x00000204, 0x003D, 0 },  //  19: add hl,RP (M:2 T:11 steps:2)
    { 0x0000002C, 0x003F, 0 },  //  1A: ld a,(de) (M:2 T:7 steps:3)
    { 0x00000014, 0x0042, 0 },  //  1B: dec RP (M:2 T:6 steps:2)
    { 0x00000004, 0x0044, 0 },  //  1C: inc RY (M:1 T:4 steps:1)
    { 0x00000004, 0x0045, 0 },  //  1D: dec RY (M:1 T:4 steps:1)
    { 0x0000002C, 0x0046, _Z80_OPSTATE_FLAGS_IMM8 },  //  1E: ld RY,n (M:2 T:7 steps:3)
    { 0x00000004, 0x0049, 0 },  //  1F: rra (M:1 T:4 steps:1)
    { 0x0000042C, 0x004A, 0 },  //  20: jr CC-4,d (M:3 T:12 steps:4)
    { 0x0000016C, 0x004E, 0 },  //  21: ld RP,nn (M:3 T:10 steps:5)
    { 0x0000766C, 0x0053, 0 },  //  22: ld (nn),hl (M:5 T:16 steps:9)
    { 0x00000014, 0x005C, 0 },  //  23: inc RP (M:2 T:6 steps:2)
    { 0x00000004, 0x005E, 0 },  //  24: inc RY (M:1 T:4 steps:1)
    { 0x00000004, 0x005F, 0 },  //  25: dec RY (M:1 T:4 steps:1)
    { 0x0000002C, 0x0060, _Z80_OPSTATE_FLAGS_IMM8 },  //  26: ld RY,n (M:2 T:7 steps:3)
    { 0x00000004, 0x0063, 0 },  //  27: daa (M:1 T:4 steps:1)
    { 0x0000042C, 0x0064, 0 },  //  28: jr CC-4,d (M:3 T:12 steps:4)
    { 0x00000204, 0x0068, 0 },  //  29: add hl,RP (M:2 T:11 steps:2)
    { 0x00005B6C, 0x006A, 0 },  //  2A: ld hl,(nn) (M:5 T:16 steps:9)
    { 0x00000014, 0x0073, 0 },  //  2B: dec RP (M:2 T:6 steps:2)
    { 0x00000004, 0x0075, 0 },  //  2C: inc RY (M:1 T:4 steps:1)
    { 0x00000004, 0x0076, 0 },  //  2D: dec RY (M:1 T:4 steps:1)
    { 0x0000002C, 0x0077, _Z80_OPSTATE_FLAGS_IMM8 },  //  2E: ld RY,n (M:2 T:7 steps:3)
    { 0x00000004, 0x007A, 0 },  //  2F: cpl (M:1 T:4 steps:1)
    { 0x0000042C, 0x007B, 0 },  //  30: jr CC-4,d (M:3 T:12 steps:4)
    { 0x0000016C, 0x007F, 0 },  //  31: ld RP,nn (M:3 T:10 steps:5)
    { 0x00000E6C, 0x0084, 0 },  //  32: ld (nn),a (M:4 T:13 steps:7)
    { 0x00000014, 0x008B, 0 },  //  33: inc RP (M:2 T:6 steps:2)
    { 0x0000038C, 0x008D, _Z80_OPSTATE_FLAGS_INDIRECT },  //  34: inc (hl) (M:3 T:11 steps:5)
    { 0x0000038C, 0x0092, _Z80_OPSTATE_FLAGS_INDIRECT },  //  35: dec (hl) (M:3 T:11 steps:5)
    { 0x000001CC, 0x0097, _Z80_OPSTATE_FLAGS_INDIRECT|_Z80_OPSTATE_FLAGS_IMM8 },  //  36: ld (hl),n (M:3 T:10 steps:5)
    { 0x00000004, 0x009C, 0 },  //  37: scf (M:1 T:4 steps:1)
    { 0x0000042C, 0x009D, 0 },  //  38: jr CC-4,d (M:3 T:12 steps:4)
    { 0x00000204, 0x00A1, 0 },  //  39: add hl,RP (M:2 T:11 steps:2)
    { 0x00000B6C, 0x00A3, 0 },  //  3A: ld a,(nn) (M:4 T:13 steps:7)
    { 0x00000014, 0x00AA, 0 },  //  3B: dec RP (M:2 T:6 steps:2)
    { 0x00000004, 0x00AC, 0 },  //  3C: inc RY (M:1 T:4 steps:1)
    { 0x00000004, 0x00AD, 0 },  //  3D: dec RY (M:1 T:4 steps:1)
    { 0x0000002C, 0x00AE, _Z80_OPSTATE_FLAGS_IMM8 },  //  3E: ld RY,n (M:2 T:7 steps:3)
    { 0x00000004, 0x00B1, 0 },  //  3F: ccf (M:1 T:4 steps:1)
    { 0x00000004, 0x00B2, 0 },  //  40: ld RY,RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x00B3, 0 },  //  41: ld RY,RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x00B4, 0 },  //  42: ld RY,RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x00B5, 0 },  //  43: ld RY,RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x00B6, 0 },  //  44: ld RY,RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x00B7, 0 },  //  45: ld RY,RZ (M:1 T:4 steps:1)
    { 0x0000002C, 0x00B8, _Z80_OPSTATE_FLAGS_INDIRECT },  //  46: ld RY,(hl) (M:2 T:7 steps:3)
    { 0x00000004, 0x00BB, 0 },  //  47: ld RY,RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x00BC, 0 },  //  48: ld RY,RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x00BD, 0 },  //  49: ld RY,RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x00BE, 0 },  //  4A: ld RY,RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x00BF, 0 },  //  4B: ld RY,RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x00C0, 0 },  //  4C: ld RY,RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x00C1, 0 },  //  4D: ld RY,RZ (M:1 T:4 steps:1)
    { 0x0000002C, 0x00C2, _Z80_OPSTATE_FLAGS_INDIRECT },  //  4E: ld RY,(hl) (M:2 T:7 steps:3)
    { 0x00000004, 0x00C5, 0 },  //  4F: ld RY,RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x00C6, 0 },  //  50: ld RY,RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x00C7, 0 },  //  51: ld RY,RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x00C8, 0 },  //  52: ld RY,RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x00C9, 0 },  //  53: ld RY,RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x00CA, 0 },  //  54: ld RY,RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x00CB, 0 },  //  55: ld RY,RZ (M:1 T:4 steps:1)
    { 0x0000002C, 0x00CC, _Z80_OPSTATE_FLAGS_INDIRECT },  //  56: ld RY,(hl) (M:2 T:7 steps:3)
    { 0x00000004, 0x00CF, 0 },  //  57: ld RY,RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x00D0, 0 },  //  58: ld RY,RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x00D1, 0 },  //  59: ld RY,RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x00D2, 0 },  //  5A: ld RY,RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x00D3, 0 },  //  5B: ld RY,RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x00D4, 0 },  //  5C: ld RY,RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x00D5, 0 },  //  5D: ld RY,RZ (M:1 T:4 steps:1)
    { 0x0000002C, 0x00D6, _Z80_OPSTATE_FLAGS_INDIRECT },  //  5E: ld RY,(hl) (M:2 T:7 steps:3)
    { 0x00000004, 0x00D9, 0 },  //  5F: ld RY,RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x00DA, 0 },  //  60: ld RY,RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x00DB, 0 },  //  61: ld RY,RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x00DC, 0 },  //  62: ld RY,RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x00DD, 0 },  //  63: ld RY,RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x00DE, 0 },  //  64: ld RY,RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x00DF, 0 },  //  65: ld RY,RZ (M:1 T:4 steps:1)
    { 0x0000002C, 0x00E0, _Z80_OPSTATE_FLAGS_INDIRECT },  //  66: ld RY,(hl) (M:2 T:7 steps:3)
    { 0x00000004, 0x00E3, 0 },  //  67: ld RY,RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x00E4, 0 },  //  68: ld RY,RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x00E5, 0 },  //  69: ld RY,RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x00E6, 0 },  //  6A: ld RY,RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x00E7, 0 },  //  6B: ld RY,RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x00E8, 0 },  //  6C: ld RY,RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x00E9, 0 },  //  6D: ld RY,RZ (M:1 T:4 steps:1)
    { 0x0000002C, 0x00EA, _Z80_OPSTATE_FLAGS_INDIRECT },  //  6E: ld RY,(hl) (M:2 T:7 steps:3)
    { 0x00000004, 0x00ED, 0 },  //  6F: ld RY,RZ (M:1 T:4 steps:1)
    { 0x00000038, 0x00EE, _Z80_OPSTATE_FLAGS_INDIRECT },  //  70: ld (hl),RZ (M:2 T:7 steps:3)
    { 0x00000038, 0x00F1, _Z80_OPSTATE_FLAGS_INDIRECT },  //  71: ld (hl),RZ (M:2 T:7 steps:3)
    { 0x00000038, 0x00F4, _Z80_OPSTATE_FLAGS_INDIRECT },  //  72: ld (hl),RZ (M:2 T:7 steps:3)
    { 0x00000038, 0x00F7, _Z80_OPSTATE_FLAGS_INDIRECT },  //  73: ld (hl),RZ (M:2 T:7 steps:3)
    { 0x00000038, 0x00FA, _Z80_OPSTATE_FLAGS_INDIRECT },  //  74: ld (hl),RZ (M:2 T:7 steps:3)
    { 0x00000038, 0x00FD, _Z80_OPSTATE_FLAGS_INDIRECT },  //  75: ld (hl),RZ (M:2 T:7 steps:3)
    { 0x00000004, 0x0100, 0 },  //  76: halt (M:1 T:4 steps:1)
    { 0x00000038, 0x0101, _Z80_OPSTATE_FLAGS_INDIRECT },  //  77: ld (hl),RZ (M:2 T:7 steps:3)
    { 0x00000004, 0x0104, 0 },  //  78: ld RY,RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x0105, 0 },  //  79: ld RY,RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x0106, 0 },  //  7A: ld RY,RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x0107, 0 },  //  7B: ld RY,RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x0108, 0 },  //  7C: ld RY,RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x0109, 0 },  //  7D: ld RY,RZ (M:1 T:4 steps:1)
    { 0x0000002C, 0x010A, _Z80_OPSTATE_FLAGS_INDIRECT },  //  7E: ld RY,(hl) (M:2 T:7 steps:3)
    { 0x00000004, 0x010D, 0 },  //  7F: ld RY,RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x010E, 0 },  //  80: ALU RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x010F, 0 },  //  81: ALU RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x0110, 0 },  //  82: ALU RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x0111, 0 },  //  83: ALU RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x0112, 0 },  //  84: ALU RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x0113, 0 },  //  85: ALU RZ (M:1 T:4 steps:1)
    { 0x0000002C, 0x0114, _Z80_OPSTATE_FLAGS_INDIRECT },  //  86: ALU (hl) (M:2 T:7 steps:3)
    { 0x00000004, 0x0117, 0 },  //  87: ALU RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x0118, 0 },  //  88: ALU RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x0119, 0 },  //  89: ALU RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x011A, 0 },  //  8A: ALU RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x011B, 0 },  //  8B: ALU RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x011C, 0 },  //  8C: ALU RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x011D, 0 },  //  8D: ALU RZ (M:1 T:4 steps:1)
    { 0x0000002C, 0x011E, _Z80_OPSTATE_FLAGS_INDIRECT },  //  8E: ALU (hl) (M:2 T:7 steps:3)
    { 0x00000004, 0x0121, 0 },  //  8F: ALU RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x0122, 0 },  //  90: ALU RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x0123, 0 },  //  91: ALU RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x0124, 0 },  //  92: ALU RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x0125, 0 },  //  93: ALU RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x0126, 0 },  //  94: ALU RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x0127, 0 },  //  95: ALU RZ (M:1 T:4 steps:1)
    { 0x0000002C, 0x0128, _Z80_OPSTATE_FLAGS_INDIRECT },  //  96: ALU (hl) (M:2 T:7 steps:3)
    { 0x00000004, 0x012B, 0 },  //  97: ALU RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x012C, 0 },  //  98: ALU RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x012D, 0 },  //  99: ALU RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x012E, 0 },  //  9A: ALU RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x012F, 0 },  //  9B: ALU RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x0130, 0 },  //  9C: ALU RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x0131, 0 },  //  9D: ALU RZ (M:1 T:4 steps:1)
    { 0x0000002C, 0x0132, _Z80_OPSTATE_FLAGS_INDIRECT },  //  9E: ALU (hl) (M:2 T:7 steps:3)
    { 0x00000004, 0x0135, 0 },  //  9F: ALU RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x0136, 0 },  //  A0: ALU RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x0137, 0 },  //  A1: ALU RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x0138, 0 },  //  A2: ALU RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x0139, 0 },  //  A3: ALU RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x013A, 0 },  //  A4: ALU RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x013B, 0 },  //  A5: ALU RZ (M:1 T:4 steps:1)
    { 0x0000002C, 0x013C, _Z80_OPSTATE_FLAGS_INDIRECT },  //  A6: ALU (hl) (M:2 T:7 steps:3)
    { 0x00000004, 0x013F, 0 },  //  A7: ALU RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x0140, 0 },  //  A8: ALU RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x0141, 0 },  //  A9: ALU RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x0142, 0 },  //  AA: ALU RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x0143, 0 },  //  AB: ALU RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x0144, 0 },  //  AC: ALU RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x0145, 0 },  //  AD: ALU RZ (M:1 T:4 steps:1)
    { 0x0000002C, 0x0146, _Z80_OPSTATE_FLAGS_INDIRECT },  //  AE: ALU (hl) (M:2 T:7 steps:3)
    { 0x00000004, 0x0149, 0 },  //  AF: ALU RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x014A, 0 },  //  B0: ALU RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x014B, 0 },  //  B1: ALU RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x014C, 0 },  //  B2: ALU RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x014D, 0 },  //  B3: ALU RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x014E, 0 },  //  B4: ALU RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x014F, 0 },  //  B5: ALU RZ (M:1 T:4 steps:1)
    { 0x0000002C, 0x0150, _Z80_OPSTATE_FLAGS_INDIRECT },  //  B6: ALU (hl) (M:2 T:7 steps:3)
    { 0x00000004, 0x0153, 0 },  //  B7: ALU RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x0154, 0 },  //  B8: ALU RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x0155, 0 },  //  B9: ALU RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x0156, 0 },  //  BA: ALU RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x0157, 0 },  //  BB: ALU RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x0158, 0 },  //  BC: ALU RZ (M:1 T:4 steps:1)
    { 0x00000004, 0x0159, 0 },  //  BD: ALU RZ (M:1 T:4 steps:1)
    { 0x0000002C, 0x015A, _Z80_OPSTATE_FLAGS_INDIRECT },  //  BE: ALU (hl) (M:2 T:7 steps:3)
    { 0x00000004, 0x015D, 0 },  //  BF: ALU RZ (M:1 T:4 steps:1)
    { 0x000002DC, 0x015E, 0 },  //  C0: ret CC (M:4 T:11 steps:6)
    { 0x0000016C, 0x0164, 0 },  //  C1: pop RP2 (M:3 T:10 steps:5)
    { 0x0000016C, 0x0169, 0 },  //  C2: jp CC,nn (M:3 T:10 steps:5)
    { 0x0000016C, 0x016E, 0 },  //  C3: jp nn (M:3 T:10 steps:5)
    { 0x0000ED6C, 0x0173, 0 },  //  C4: call CC,nn (M:6 T:17 steps:10)
    { 0x000003B0, 0x017D, 0 },  //  C5: push RP2 (M:3 T:11 steps:5)
    { 0x0000002C, 0x0182, _Z80_OPSTATE_FLAGS_IMM8 },  //  C6: ALU n (M:2 T:7 steps:3)
    { 0x000003B0, 0x0185, 0 },  //  C7: rst Y*8 (M:3 T:11 steps:5)
    { 0x000002DC, 0x018A, 0 },  //  C8: ret CC (M:4 T:11 steps:6)
    { 0x0000016C, 0x0190, 0 },  //  C9: ret (M:3 T:10 steps:5)
    { 0x0000016C, 0x0195, 0 },  //  CA: jp CC,nn (M:3 T:10 steps:5)
    { 0x00000004, 0x019A, 0 },  //  CB: cb prefix (M:1 T:4 steps:1)
    { 0x0000ED6C, 0x019B, 0 },  //  CC: call CC,nn (M:6 T:17 steps:10)
    { 0x0000EC6C, 0x01A5, 0 },  //  CD: call nn (M:5 T:17 steps:9)
    { 0x0000002C, 0x01AE, _Z80_OPSTATE_FLAGS_IMM8 },  //  CE: ALU n (M:2 T:7 steps:3)
    { 0x000003B0, 0x01B1, 0 },  //  CF: rst Y*8 (M:3 T:11 steps:5)
    { 0x000002DC, 0x01B6, 0 },  //  D0: ret CC (M:4 T:11 steps:6)
    { 0x0000016C, 0x01BC, 0 },  //  D1: pop RP2 (M:3 T:10 steps:5)
    { 0x0000016C, 0x01C1, 0 },  //  D2: jp CC,nn (M:3 T:10 steps:5)
    { 0x000002CC, 0x01C6, 0 },  //  D3: out (n),a (M:3 T:11 steps:5)
    { 0x0000ED6C, 0x01CB, 0 },  //  D4: call CC,nn (M:6 T:17 steps:10)
    { 0x000003B0, 0x01D5, 0 },  //  D5: push RP2 (M:3 T:11 steps:5)
    { 0x0000002C, 0x01DA, _Z80_OPSTATE_FLAGS_IMM8 },  //  D6: ALU n (M:2 T:7 steps:3)
    { 0x000003B0, 0x01DD, 0 },  //  D7: rst Y*8 (M:3 T:11 steps:5)
    { 0x000002DC, 0x01E2, 0 },  //  D8: ret CC (M:4 T:11 steps:6)
    { 0x00000004, 0x01E8, 0 },  //  D9: exx (M:1 T:4 steps:1)
    { 0x0000016C, 0x01E9, 0 },  //  DA: jp CC,nn (M:3 T:10 steps:5)
    { 0x000002CC, 0x01EE, 0 },  //  DB: in a,(n) (M:3 T:11 steps:5)
    { 0x0000ED6C, 0x01F3, 0 },  //  DC: call CC,nn (M:6 T:17 steps:10)
    { 0x00000004, 0x01FD, 0 },  //  DD: dd prefix (M:1 T:4 steps:1)
    { 0x0000002C, 0x01FE, _Z80_OPSTATE_FLAGS_IMM8 },  //  DE: ALU n (M:2 T:7 steps:3)
    { 0x000003B0, 0x0201, 0 },  //  DF: rst Y*8 (M:3 T:11 steps:5)
    { 0x000002DC, 0x0206, 0 },  //  E0: ret CC (M:4 T:11 steps:6)
    { 0x0000016C, 0x020C, 0 },  //  E1: pop RP2 (M:3 T:10 steps:5)
    { 0x0000016C, 0x0211, 0 },  //  E2: jp CC,nn (M:3 T:10 steps:5)
    { 0x00026C6C, 0x0216, 0 },  //  E3: ex (sp),hl (M:5 T:19 steps:9)
    { 0x0000ED6C, 0x021F, 0 },  //  E4: call CC,nn (M:6 T:17 steps:10)
    { 0x000003B0, 0x0229, 0 },  //  E5: push RP2 (M:3 T:11 steps:5)
    { 0x0000002C, 0x022E, _Z80_OPSTATE_FLAGS_IMM8 },  //  E6: ALU n (M:2 T:7 steps:3)
    { 0x000003B0, 0x0231, 0 },  //  E7: rst Y*8 (M:3 T:11 steps:5)
    { 0x000002DC, 0x0236, 0 },  //  E8: ret CC (M:4 T:11 steps:6)
    { 0x00000004, 0x023C, 0 },  //  E9: jp hl (M:1 T:4 steps:1)
    { 0x0000016C, 0x023D, 0 },  //  EA: jp CC,nn (M:3 T:10 steps:5)
    { 0x00000004, 0x0242, 0 },  //  EB: ex de,hl (M:1 T:4 steps:1)
    { 0x0000ED6C, 0x0243, 0 },  //  EC: call CC,nn (M:6 T:17 steps:10)
    { 0x00000004, 0x024D, 0 },  //  ED: ed prefix (M:1 T:4 steps:1)
    { 0x0000002C, 0x024E, _Z80_OPSTATE_FLAGS_IMM8 },  //  EE: ALU n (M:2 T:7 steps:3)
    { 0x000003B0, 0x0251, 0 },  //  EF: rst Y*8 (M:3 T:11 steps:5)
    { 0x000002DC, 0x0256, 0 },  //  F0: ret CC (M:4 T:11 steps:6)
    { 0x0000016C, 0x025C, 0 },  //  F1: pop RP2 (M:3 T:10 steps:5)
    { 0x0000016C, 0x0261, 0 },  //  F2: jp CC,nn (M:3 T:10 steps:5)
    { 0x00000004, 0x0266, 0 },  //  F3: di (M:1 T:4 steps:1)
    { 0x0000ED6C, 0x0267, 0 },  //  F4: call CC,nn (M:6 T:17 steps:10)
    { 0x000003B0, 0x0271, 0 },  //  F5: push RP2 (M:3 T:11 steps:5)
    { 0x0000002C, 0x0276, _Z80_OPSTATE_FLAGS_IMM8 },  //  F6: ALU n (M:2 T:7 steps:3)
    { 0x000003B0, 0x0279, 0 },  //  F7: rst Y*8 (M:3 T:11 steps:5)
    { 0x000002DC, 0x027E, 0 },  //  F8: ret CC (M:4 T:11 steps:6)
    { 0x00000014, 0x0284, 0 },  //  F9: ld sp,hl (M:2 T:6 steps:2)
    { 0x0000016C, 0x0286, 0 },  //  FA: jp CC,nn (M:3 T:10 steps:5)
    { 0x00000004, 0x028B, 0 },  //  FB: ei (M:1 T:4 steps:1)
    { 0x0000ED6C, 0x028C, 0 },  //  FC: call CC,nn (M:6 T:17 steps:10)
    { 0x00000004, 0x0296, 0 },  //  FD: fd prefix (M:1 T:4 steps:1)
    { 0x0000002C, 0x0297, _Z80_OPSTATE_FLAGS_IMM8 },  //  FE: ALU n (M:2 T:7 steps:3)
    { 0x000003B0, 0x029A, 0 },  //  FF: rst Y*8 (M:3 T:11 steps:5)
    { 0x00000004, 0x029F, 0 },  // ED 00: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 01: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 02: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 03: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 04: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 05: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 06: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 07: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 08: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 09: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 0A: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 0B: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 0C: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 0D: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 0E: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 0F: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 10: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 11: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 12: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 13: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 14: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 15: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 16: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 17: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 18: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 19: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 1A: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 1B: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 1C: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 1D: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 1E: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 1F: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 20: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 21: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 22: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 23: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 24: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 25: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 26: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 27: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 28: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 29: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 2A: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 2B: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 2C: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 2D: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 2E: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 2F: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 30: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 31: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 32: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 33: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 34: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 35: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 36: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 37: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 38: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 39: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 3A: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 3B: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 3C: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 3D: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 3E: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 3F: ed nop (M:1 T:4 steps:1)
    { 0x00000058, 0x02A0, 0 },  // ED 40: in RY,(c) (M:2 T:8 steps:3)
    { 0x00000058, 0x02A3, 0 },  // ED 41: out (c),RY (M:2 T:8 steps:3)
    { 0x00000204, 0x02A6, 0 },  // ED 42: sbc hl,RP (M:2 T:11 steps:2)
    { 0x0000766C, 0x02A8, 0 },  // ED 43: ld (nn),RP (M:5 T:16 steps:9)
    { 0x00000004, 0x02B1, 0 },  // ED 44: neg (M:1 T:4 steps:1)
    { 0x0000016C, 0x02B2, 0 },  // ED 45: reti/retn (M:3 T:10 steps:5)
    { 0x00000004, 0x02B7, 0 },  // ED 46: im IMY (M:1 T:4 steps:1)
    { 0x00000008, 0x02B8, 0 },  // ED 47: ld i,a (M:1 T:5 steps:1)
    { 0x00000058, 0x02B9, 0 },  // ED 48: in RY,(c) (M:2 T:8 steps:3)
    { 0x00000058, 0x02BC, 0 },  // ED 49: out (c),RY (M:2 T:8 steps:3)
    { 0x00000204, 0x02BF, 0 },  // ED 4A: adc hl,RP (M:2 T:11 steps:2)
    { 0x00005B6C, 0x02C1, 0 },  // ED 4B: ld RP,(nn) (M:5 T:16 steps:9)
    { 0x00000004, 0x02B1, 0 },  // ED 4C: neg (M:1 T:4 steps:1)
    { 0x0000016C, 0x02B2, 0 },  // ED 4D: reti/retn (M:3 T:10 steps:5)
    { 0x00000004, 0x02CA, 0 },  // ED 4E: im IMY (M:1 T:4 steps:1)
    { 0x00000008, 0x02CB, 0 },  // ED 4F: ld r,a (M:1 T:5 steps:1)
    { 0x00000058, 0x02CC, 0 },  // ED 50: in RY,(c) (M:2 T:8 steps:3)
    { 0x00000058, 0x02CF, 0 },  // ED 51: out (c),RY (M:2 T:8 steps:3)
    { 0x00000204, 0x02D2, 0 },  // ED 52: sbc hl,RP (M:2 T:11 steps:2)
    { 0x0000766C, 0x02D4, 0 },  // ED 53: ld (nn),RP (M:5 T:16 steps:9)
    { 0x00000004, 0x02B1, 0 },  // ED 54: neg (M:1 T:4 steps:1)
    { 0x0000016C, 0x02B2, 0 },  // ED 55: reti/retn (M:3 T:10 steps:5)
    { 0x00000004, 0x02DD, 0 },  // ED 56: im IMY (M:1 T:4 steps:1)
    { 0x00000008, 0x02DE, 0 },  // ED 57: ld a,i (M:1 T:5 steps:1)
    { 0x00000058, 0x02DF, 0 },  // ED 58: in RY,(c) (M:2 T:8 steps:3)
    { 0x00000058, 0x02E2, 0 },  // ED 59: out (c),RY (M:2 T:8 steps:3)
    { 0x00000204, 0x02E5, 0 },  // ED 5A: adc hl,RP (M:2 T:11 steps:2)
    { 0x00005B6C, 0x02E7, 0 },  // ED 5B: ld RP,(nn) (M:5 T:16 steps:9)
    { 0x00000004, 0x02B1, 0 },  // ED 5C: neg (M:1 T:4 steps:1)
    { 0x0000016C, 0x02B2, 0 },  // ED 5D: reti/retn (M:3 T:10 steps:5)
    { 0x00000004, 0x02F0, 0 },  // ED 5E: im IMY (M:1 T:4 steps:1)
    { 0x00000008, 0x02F1, 0 },  // ED 5F: ld a,r (M:1 T:5 steps:1)
    { 0x00000058, 0x02F2, 0 },  // ED 60: in RY,(c) (M:2 T:8 steps:3)
    { 0x00000058, 0x02F5, 0 },  // ED 61: out (c),RY (M:2 T:8 steps:3)
    { 0x00000204, 0x02F8, 0 },  // ED 62: sbc hl,RP (M:2 T:11 steps:2)
    { 0x0000766C, 0x02FA, 0 },  // ED 63: ld (nn),RP (M:5 T:16 steps:9)
    { 0x00000004, 0x02B1, 0 },  // ED 64: neg (M:1 T:4 steps:1)
    { 0x0000016C, 0x02B2, 0 },  // ED 65: reti/retn (M:3 T:10 steps:5)
    { 0x00000004, 0x0303, 0 },  // ED 66: im IMY (M:1 T:4 steps:1)
    { 0x00001C2C, 0x0304, 0 },  // ED 67: rrd (M:4 T:14 steps:6)
    { 0x00000058, 0x030A, 0 },  // ED 68: in RY,(c) (M:2 T:8 steps:3)
    { 0x00000058, 0x030D, 0 },  // ED 69: out (c),RY (M:2 T:8 steps:3)
    { 0x00000204, 0x0310, 0 },  // ED 6A: adc hl,RP (M:2 T:11 steps:2)
    { 0x00005B6C, 0x0312, 0 },  // ED 6B: ld RP,(nn) (M:5 T:16 steps:9)
    { 0x00000004, 0x02B1, 0 },  // ED 6C: neg (M:1 T:4 steps:1)
    { 0x0000016C, 0x02B2, 0 },  // ED 6D: reti/retn (M:3 T:10 steps:5)
    { 0x00000004, 0x031B, 0 },  // ED 6E: im IMY (M:1 T:4 steps:1)
    { 0x00001C2C, 0x031C, 0 },  // ED 6F: rld (M:4 T:14 steps:6)
    { 0x00000058, 0x0322, 0 },  // ED 70: in (c) (M:2 T:8 steps:3)
    { 0x00000058, 0x0325, 0 },  // ED 71: out (c),0 (M:2 T:8 steps:3)
    { 0x00000204, 0x0328, 0 },  // ED 72: sbc hl,RP (M:2 T:11 steps:2)
    { 0x0000766C, 0x032A, 0 },  // ED 73: ld (nn),RP (M:5 T:16 steps:9)
    { 0x00000004, 0x02B1, 0 },  // ED 74: neg (M:1 T:4 steps:1)
    { 0x0000016C, 0x02B2, 0 },  // ED 75: reti/retn (M:3 T:10 steps:5)
    { 0x00000004, 0x0333, 0 },  // ED 76: im IMY (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 77: ed nop (M:1 T:4 steps:1)
    { 0x00000058, 0x0334, 0 },  // ED 78: in RY,(c) (M:2 T:8 steps:3)
    { 0x00000058, 0x0337, 0 },  // ED 79: out (c),RY (M:2 T:8 steps:3)
    { 0x00000204, 0x033A, 0 },  // ED 7A: adc hl,RP (M:2 T:11 steps:2)
    { 0x00005B6C, 0x033C, 0 },  // ED 7B: ld RP,(nn) (M:5 T:16 steps:9)
    { 0x00000004, 0x02B1, 0 },  // ED 7C: neg (M:1 T:4 steps:1)
    { 0x0000016C, 0x02B2, 0 },  // ED 7D: reti/retn (M:3 T:10 steps:5)
    { 0x00000004, 0x0345, 0 },  // ED 7E: im IMY (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 7F: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 80: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 81: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 82: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 83: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 84: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 85: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 86: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 87: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 88: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 89: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 8A: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 8B: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 8C: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 8D: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 8E: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 8F: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 90: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 91: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 92: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 93: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 94: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 95: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 96: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 97: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 98: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 99: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 9A: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 9B: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 9C: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 9D: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 9E: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED 9F: ed nop (M:1 T:4 steps:1)
    { 0x000005CC, 0x0346, 0 },  // ED A0: ldi (M:4 T:12 steps:6)
    { 0x0000042C, 0x034C, 0 },  // ED A1: cpi (M:3 T:12 steps:4)
    { 0x00000730, 0x0350, 0 },  // ED A2: ini (M:3 T:12 steps:5)
    { 0x00000598, 0x0355, 0 },  // ED A3: outi (M:3 T:12 steps:5)
    { 0x00000004, 0x029F, 0 },  // ED A4: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED A5: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED A6: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED A7: ed nop (M:1 T:4 steps:1)
    { 0x000005CC, 0x035A, 0 },  // ED A8: ldd (M:4 T:12 steps:6)
    { 0x0000042C, 0x0360, 0 },  // ED A9: cpd (M:3 T:12 steps:4)
    { 0x00000730, 0x0364, 0 },  // ED AA: ind (M:3 T:12 steps:5)
    { 0x00000598, 0x0369, 0 },  // ED AB: outd (M:3 T:12 steps:5)
    { 0x00000004, 0x029F, 0 },  // ED AC: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED AD: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED AE: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED AF: ed nop (M:1 T:4 steps:1)
    { 0x000085CC, 0x036E, 0 },  // ED B0: ldir (M:5 T:17 steps:7)
    { 0x0000842C, 0x0375, 0 },  // ED B1: cpir (M:4 T:17 steps:5)
    { 0x00008730, 0x037A, 0 },  // ED B2: inir (M:4 T:17 steps:6)
    { 0x00008598, 0x0380, 0 },  // ED B3: otir (M:4 T:17 steps:6)
    { 0x00000004, 0x029F, 0 },  // ED B4: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED B5: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED B6: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED B7: ed nop (M:1 T:4 steps:1)
    { 0x000085CC, 0x0386, 0 },  // ED B8: lddr (M:5 T:17 steps:7)
    { 0x0000842C, 0x038D, 0 },  // ED B9: cpdr (M:4 T:17 steps:5)
    { 0x00008730, 0x0392, 0 },  // ED BA: indr (M:4 T:17 steps:6)
    { 0x00008598, 0x0398, 0 },  // ED BB: otdr (M:4 T:17 steps:6)
    { 0x00000004, 0x029F, 0 },  // ED BC: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED BD: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED BE: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED BF: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED C0: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED C1: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED C2: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED C3: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED C4: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED C5: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED C6: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED C7: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED C8: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED C9: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED CA: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED CB: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED CC: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED CD: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED CE: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED CF: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED D0: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED D1: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED D2: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED D3: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED D4: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED D5: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED D6: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED D7: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED D8: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED D9: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED DA: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED DB: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED DC: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED DD: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED DE: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED DF: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED E0: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED E1: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED E2: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED E3: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED E4: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED E5: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED E6: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED E7: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED E8: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED E9: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED EA: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED EB: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED EC: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED ED: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED EE: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED EF: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED F0: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED F1: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED F2: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED F3: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED F4: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED F5: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED F6: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED F7: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED F8: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED F9: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED FA: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED FB: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED FC: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED FD: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED FE: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x029F, 0 },  // ED FF: ed nop (M:1 T:4 steps:1)
    { 0x00000004, 0x039E, 0 },  // CB 00: cb (M:1 T:4 steps:1)
    { 0x0000038C, 0x039F, 0 },  // CB 01: cbhl (M:3 T:11 steps:5)
    { 0x0001C634, 0x03A4, 0 },  // CB 02: ddfdcb (M:5 T:18 steps:8)
    { 0x000000BC, 0x03AC, 0 },  //  03: int_im0 (M:5 T:9 steps:5)
    { 0x0000762C, 0x03B1, 0 },  //  04: int_im1 (M:6 T:16 steps:8)
    { 0x0016F63C, 0x03B9, 0 },  //  05: int_im2 (M:9 T:22 steps:13)
    { 0x00001D8C, 0x03C6, 0 },  //  06: nmi (M:5 T:14 steps:7)

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
    // step3 is a regular refresh cycle
    pins = _z80_refresh(cpu, pins);
    // branch to interrupt 'payload' instruction (usually an RST)
    cpu->op = _z80_opstate_table[cpu->opcode];
    return pins;
}

// initiate a fetch machine cycle
static inline uint64_t _z80_fetch(z80_t* cpu, uint64_t pins) {
    // need to handle interrupt?
    if (cpu->int_bits & Z80_NMI) {
        // non-maskable interrupt starts with a regular M1 machine cycle
        cpu->op = _z80_opstate_table[_Z80_OPSTATE_SLOT_NMI];
        cpu->op.pip >>= 1;
        // NOTE: PC is *not* incremented!
        pins = _z80_set_ab_x(pins, cpu->pc, Z80_M1|Z80_MREQ|Z80_RD);
    }
    else if ((cpu->int_bits & Z80_INT) && cpu->iff1) {
        // maskable interrupts start with a special M1 machine cycle which
        // doesn't fetch the next opcode, but instead activate the
        // pins M1|IOQR to request a special byte which is handled differently
        // depending on interrupt mode
        cpu->op = _z80_opstate_table[_Z80_OPSTATE_SLOT_INT_IM0 + cpu->im];
        cpu->op.pip >>= 1;
        // NOTE: PC is not incremented, and no pins are activated here
    }
    else {
        // no interrupt, continue with next opcode
        cpu->op.pip = (_Z80_M1_PIP)<<1;
        cpu->op.step = 0xFFFF;
        pins = _z80_set_ab_x(pins, cpu->pc++, Z80_M1|Z80_MREQ|Z80_RD);
    }
    cpu->prefix_state = 0;
    cpu->int_bits = 0;
    return pins;
}

static inline uint64_t _z80_fetch_prefix(z80_t* cpu, uint64_t pins, uint8_t prefix) {
    // reset the decoder to continue at step 0
    cpu->op.pip = (_Z80_M1_PIP)<<1;
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
                cpu->op.pip >>= 1;
                // set pins for a regular read machine cycle to read d-offset
                pins = _z80_set_ab_x(pins, cpu->pc++, Z80_MREQ|Z80_RD);
            }
            else {
                // this is a regular CB-prefixed instruction, continue
                // execution on a special fetch machine cycle which doesn't
                // handle DD/FD prefix and then branches either to the
                // special CB or CBHL decoder block
                cpu->op.step = 5 - 1;   // => step 5
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
    cpu->op.pip = 1;
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
#define _fetch()        pins=_z80_fetch(cpu,pins)
#define _fetch_dd()     pins=_z80_fetch_prefix(cpu,pins,_Z80_PREFIX_DD);
#define _fetch_fd()     pins=_z80_fetch_prefix(cpu,pins,_Z80_PREFIX_FD);
#define _fetch_ed()     pins=_z80_fetch_prefix(cpu,pins,_Z80_PREFIX_ED);
#define _fetch_cb()     pins=_z80_fetch_prefix(cpu,pins,_Z80_PREFIX_CB);
#define _mread(ab)      _sax(ab,Z80_MREQ|Z80_RD)
#define _mwrite(ab,d)   _sadx(ab,d,Z80_MREQ|Z80_WR)
#define _ioread(ab)     _sax(ab,Z80_IORQ|Z80_RD)
#define _iowrite(ab,d)  _sadx(ab,d,Z80_IORQ|Z80_WR)
#define _wait()         if(pins&Z80_WAIT){goto track_int_bits;}
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
    if (cpu->op.pip & 1) {
        switch (cpu->op.step) {
            // shared fetch machine cycle for all opcodes
            case 0: {
                cpu->opcode = _gd();
            } break;
            // refresh cycle
            case 1: {
                pins = _z80_refresh(cpu, pins);
                cpu->op = _z80_opstate_table[cpu->opcode + cpu->prefix_offset];

                // if this is a (HL)/(IX+d)/(IY+d) instruction, insert
                // d-load cycle if needed and compute effective address
                if (cpu->op.flags & _Z80_OPSTATE_FLAGS_INDIRECT) {
                    cpu->addr = cpu->hlx[cpu->hlx_idx].hl;
                    if (cpu->hlx_idx != _Z80_MAP_HL) {
                        // (IX+d) or (IY+d): insert 3 4-cycle machine cycles
                        // to load d offset and setup effective address
                        cpu->op.pip = 3<<2;
                        // special case: if this is indirect+immediate (which is
                        // just LD (HL),n, then the immediate-load is 'hidden' within
                        // the 8-tcycle d-offset computation)
                        if (cpu->op.flags & _Z80_OPSTATE_FLAGS_IMM8) {
                            cpu->op.pip |= 1<<4;
                        }
                        else {
                            cpu->op.pip |= 1<<9;
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
                cpu->op = _z80_opstate_table[cpu->opcode];
                // special case: if this is indirect+immediate (which is just LD 
                // (HL),n), then stretch the immediate-load machine cycle by 3 tcycles
                // because it is 'hidden' in the d-offset 8-tcycle load
                if (cpu->op.flags & _Z80_OPSTATE_FLAGS_IMM8) {
                    const uint64_t mask = 0xF;
                    cpu->op.pip = (cpu->op.pip & mask) | ((cpu->op.pip & ~mask)<<2);
                }
                cpu->op.pip >>= 1;
            } break;
            //=== special opcode fetch machine cycle for CB-prefixed instructions
            case 5: {
                cpu->opcode = _gd();
            } break;
            case 6: {
                pins = _z80_refresh(cpu, pins);
                if ((cpu->opcode & 7) == 6) {
                    // this is a (HL) instruction
                    cpu->addr = cpu->hl;
                    cpu->op = _z80_opstate_table[_Z80_OPSTATE_SLOT_CBHL];
                }
                else {
                    cpu->op = _z80_opstate_table[_Z80_OPSTATE_SLOT_CB];
                }
            } break;
            
            //  00: nop (M:1 T:4)
            // -- OVERLAP
            case 0x0007: _wait();_fetch(); break;
            
            //  01: ld RP,nn (M:3 T:10)
            // -- M2
            case 0x0008: _wait();_mread(cpu->pc++); break;
            case 0x0009: cpu->c=_gd(); break;
            // -- M3
            case 0x000A: _wait();_mread(cpu->pc++); break;
            case 0x000B: cpu->b=_gd(); break;
            // -- OVERLAP
            case 0x000C: _wait();_fetch(); break;
            
            //  02: ld (bc),a (M:2 T:7)
            // -- M2
            case 0x000D: _mwrite(cpu->bc,cpu->a); break;
            case 0x000E: _wait();cpu->wzl=cpu->c+1;cpu->wzh=cpu->a; break;
            // -- OVERLAP
            case 0x000F: _wait();_fetch(); break;
            
            //  03: inc RP (M:2 T:6)
            // -- M2 (generic)
            case 0x0010: cpu->bc++; break;
            // -- OVERLAP
            case 0x0011: _wait();_fetch(); break;
            
            //  04: inc RY (M:1 T:4)
            // -- OVERLAP
            case 0x0012: _wait();cpu->b=_z80_inc8(cpu,cpu->b);_fetch(); break;
            
            //  05: dec RY (M:1 T:4)
            // -- OVERLAP
            case 0x0013: _wait();cpu->b=_z80_dec8(cpu,cpu->b);_fetch(); break;
            
            //  06: ld RY,n (M:2 T:7)
            // -- M2
            case 0x0014: _wait();_mread(cpu->pc++); break;
            case 0x0015: cpu->b=_gd(); break;
            // -- OVERLAP
            case 0x0016: _wait();_fetch(); break;
            
            //  07: rlca (M:1 T:4)
            // -- OVERLAP
            case 0x0017: _wait();_z80_rlca(cpu);_fetch(); break;
            
            //  08: ex af,af' (M:1 T:4)
            // -- OVERLAP
            case 0x0018: _wait();_z80_ex_af_af2(cpu);_fetch(); break;
            
            //  09: add hl,RP (M:2 T:11)
            // -- M2 (generic)
            case 0x0019: _z80_add16(cpu,cpu->bc); break;
            // -- OVERLAP
            case 0x001A: _wait();_fetch(); break;
            
            //  0A: ld a,(bc) (M:2 T:7)
            // -- M2
            case 0x001B: _wait();_mread(cpu->bc); break;
            case 0x001C: cpu->a=_gd();cpu->wz=cpu->bc+1; break;
            // -- OVERLAP
            case 0x001D: _wait();_fetch(); break;
            
            //  0B: dec RP (M:2 T:6)
            // -- M2 (generic)
            case 0x001E: cpu->bc--; break;
            // -- OVERLAP
            case 0x001F: _wait();_fetch(); break;
            
            //  0C: inc RY (M:1 T:4)
            // -- OVERLAP
            case 0x0020: _wait();cpu->c=_z80_inc8(cpu,cpu->c);_fetch(); break;
            
            //  0D: dec RY (M:1 T:4)
            // -- OVERLAP
            case 0x0021: _wait();cpu->c=_z80_dec8(cpu,cpu->c);_fetch(); break;
            
            //  0E: ld RY,n (M:2 T:7)
            // -- M2
            case 0x0022: _wait();_mread(cpu->pc++); break;
            case 0x0023: cpu->c=_gd(); break;
            // -- OVERLAP
            case 0x0024: _wait();_fetch(); break;
            
            //  0F: rrca (M:1 T:4)
            // -- OVERLAP
            case 0x0025: _wait();_z80_rrca(cpu);_fetch(); break;
            
            //  10: djnz d (M:3 T:13)
            // -- M2
            case 0x0026: _wait();_mread(cpu->pc++); break;
            case 0x0027: cpu->dlatch=_gd();if(--cpu->b==0){_z80_skip(cpu,1,7,2);}; break;
            // -- M3 (generic)
            case 0x0028: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;; break;
            // -- OVERLAP
            case 0x0029: _wait();_fetch(); break;
            
            //  11: ld RP,nn (M:3 T:10)
            // -- M2
            case 0x002A: _wait();_mread(cpu->pc++); break;
            case 0x002B: cpu->e=_gd(); break;
            // -- M3
            case 0x002C: _wait();_mread(cpu->pc++); break;
            case 0x002D: cpu->d=_gd(); break;
            // -- OVERLAP
            case 0x002E: _wait();_fetch(); break;
            
            //  12: ld (de),a (M:2 T:7)
            // -- M2
            case 0x002F: _mwrite(cpu->de,cpu->a); break;
            case 0x0030: _wait();cpu->wzl=cpu->e+1;cpu->wzh=cpu->a; break;
            // -- OVERLAP
            case 0x0031: _wait();_fetch(); break;
            
            //  13: inc RP (M:2 T:6)
            // -- M2 (generic)
            case 0x0032: cpu->de++; break;
            // -- OVERLAP
            case 0x0033: _wait();_fetch(); break;
            
            //  14: inc RY (M:1 T:4)
            // -- OVERLAP
            case 0x0034: _wait();cpu->d=_z80_inc8(cpu,cpu->d);_fetch(); break;
            
            //  15: dec RY (M:1 T:4)
            // -- OVERLAP
            case 0x0035: _wait();cpu->d=_z80_dec8(cpu,cpu->d);_fetch(); break;
            
            //  16: ld RY,n (M:2 T:7)
            // -- M2
            case 0x0036: _wait();_mread(cpu->pc++); break;
            case 0x0037: cpu->d=_gd(); break;
            // -- OVERLAP
            case 0x0038: _wait();_fetch(); break;
            
            //  17: rla (M:1 T:4)
            // -- OVERLAP
            case 0x0039: _wait();_z80_rla(cpu);_fetch(); break;
            
            //  18: jr d (M:3 T:12)
            // -- M2
            case 0x003A: _wait();_mread(cpu->pc++); break;
            case 0x003B: cpu->dlatch=_gd(); break;
            // -- M3 (generic)
            case 0x003C: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;; break;
            // -- OVERLAP
            case 0x003D: _wait();_fetch(); break;
            
            //  19: add hl,RP (M:2 T:11)
            // -- M2 (generic)
            case 0x003E: _z80_add16(cpu,cpu->de); break;
            // -- OVERLAP
            case 0x003F: _wait();_fetch(); break;
            
            //  1A: ld a,(de) (M:2 T:7)
            // -- M2
            case 0x0040: _wait();_mread(cpu->de); break;
            case 0x0041: cpu->a=_gd();cpu->wz=cpu->de+1; break;
            // -- OVERLAP
            case 0x0042: _wait();_fetch(); break;
            
            //  1B: dec RP (M:2 T:6)
            // -- M2 (generic)
            case 0x0043: cpu->de--; break;
            // -- OVERLAP
            case 0x0044: _wait();_fetch(); break;
            
            //  1C: inc RY (M:1 T:4)
            // -- OVERLAP
            case 0x0045: _wait();cpu->e=_z80_inc8(cpu,cpu->e);_fetch(); break;
            
            //  1D: dec RY (M:1 T:4)
            // -- OVERLAP
            case 0x0046: _wait();cpu->e=_z80_dec8(cpu,cpu->e);_fetch(); break;
            
            //  1E: ld RY,n (M:2 T:7)
            // -- M2
            case 0x0047: _wait();_mread(cpu->pc++); break;
            case 0x0048: cpu->e=_gd(); break;
            // -- OVERLAP
            case 0x0049: _wait();_fetch(); break;
            
            //  1F: rra (M:1 T:4)
            // -- OVERLAP
            case 0x004A: _wait();_z80_rra(cpu);_fetch(); break;
            
            //  20: jr CC-4,d (M:3 T:12)
            // -- M2
            case 0x004B: _wait();_mread(cpu->pc++); break;
            case 0x004C: cpu->dlatch=_gd();if(!(_cc_nz)){_z80_skip(cpu,1,7,2);}; break;
            // -- M3 (generic)
            case 0x004D: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;; break;
            // -- OVERLAP
            case 0x004E: _wait();_fetch(); break;
            
            //  21: ld RP,nn (M:3 T:10)
            // -- M2
            case 0x004F: _wait();_mread(cpu->pc++); break;
            case 0x0050: cpu->hlx[cpu->hlx_idx].l=_gd(); break;
            // -- M3
            case 0x0051: _wait();_mread(cpu->pc++); break;
            case 0x0052: cpu->hlx[cpu->hlx_idx].h=_gd(); break;
            // -- OVERLAP
            case 0x0053: _wait();_fetch(); break;
            
            //  22: ld (nn),hl (M:5 T:16)
            // -- M2
            case 0x0054: _wait();_mread(cpu->pc++); break;
            case 0x0055: cpu->wzl=_gd(); break;
            // -- M3
            case 0x0056: _wait();_mread(cpu->pc++); break;
            case 0x0057: cpu->wzh=_gd(); break;
            // -- M4
            case 0x0058: _mwrite(cpu->wz++,cpu->hlx[cpu->hlx_idx].l); break;
            case 0x0059: _wait(); break;
            // -- M5
            case 0x005A: _mwrite(cpu->wz,cpu->hlx[cpu->hlx_idx].h); break;
            case 0x005B: _wait(); break;
            // -- OVERLAP
            case 0x005C: _wait();_fetch(); break;
            
            //  23: inc RP (M:2 T:6)
            // -- M2 (generic)
            case 0x005D: cpu->hlx[cpu->hlx_idx].hl++; break;
            // -- OVERLAP
            case 0x005E: _wait();_fetch(); break;
            
            //  24: inc RY (M:1 T:4)
            // -- OVERLAP
            case 0x005F: _wait();cpu->hlx[cpu->hlx_idx].h=_z80_inc8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); break;
            
            //  25: dec RY (M:1 T:4)
            // -- OVERLAP
            case 0x0060: _wait();cpu->hlx[cpu->hlx_idx].h=_z80_dec8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); break;
            
            //  26: ld RY,n (M:2 T:7)
            // -- M2
            case 0x0061: _wait();_mread(cpu->pc++); break;
            case 0x0062: cpu->hlx[cpu->hlx_idx].h=_gd(); break;
            // -- OVERLAP
            case 0x0063: _wait();_fetch(); break;
            
            //  27: daa (M:1 T:4)
            // -- OVERLAP
            case 0x0064: _wait();_z80_daa(cpu);_fetch(); break;
            
            //  28: jr CC-4,d (M:3 T:12)
            // -- M2
            case 0x0065: _wait();_mread(cpu->pc++); break;
            case 0x0066: cpu->dlatch=_gd();if(!(_cc_z)){_z80_skip(cpu,1,7,2);}; break;
            // -- M3 (generic)
            case 0x0067: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;; break;
            // -- OVERLAP
            case 0x0068: _wait();_fetch(); break;
            
            //  29: add hl,RP (M:2 T:11)
            // -- M2 (generic)
            case 0x0069: _z80_add16(cpu,cpu->hlx[cpu->hlx_idx].hl); break;
            // -- OVERLAP
            case 0x006A: _wait();_fetch(); break;
            
            //  2A: ld hl,(nn) (M:5 T:16)
            // -- M2
            case 0x006B: _wait();_mread(cpu->pc++); break;
            case 0x006C: cpu->wzl=_gd(); break;
            // -- M3
            case 0x006D: _wait();_mread(cpu->pc++); break;
            case 0x006E: cpu->wzh=_gd(); break;
            // -- M4
            case 0x006F: _wait();_mread(cpu->wz++); break;
            case 0x0070: cpu->hlx[cpu->hlx_idx].l=_gd(); break;
            // -- M5
            case 0x0071: _wait();_mread(cpu->wz); break;
            case 0x0072: cpu->hlx[cpu->hlx_idx].h=_gd(); break;
            // -- OVERLAP
            case 0x0073: _wait();_fetch(); break;
            
            //  2B: dec RP (M:2 T:6)
            // -- M2 (generic)
            case 0x0074: cpu->hlx[cpu->hlx_idx].hl--; break;
            // -- OVERLAP
            case 0x0075: _wait();_fetch(); break;
            
            //  2C: inc RY (M:1 T:4)
            // -- OVERLAP
            case 0x0076: _wait();cpu->hlx[cpu->hlx_idx].l=_z80_inc8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); break;
            
            //  2D: dec RY (M:1 T:4)
            // -- OVERLAP
            case 0x0077: _wait();cpu->hlx[cpu->hlx_idx].l=_z80_dec8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); break;
            
            //  2E: ld RY,n (M:2 T:7)
            // -- M2
            case 0x0078: _wait();_mread(cpu->pc++); break;
            case 0x0079: cpu->hlx[cpu->hlx_idx].l=_gd(); break;
            // -- OVERLAP
            case 0x007A: _wait();_fetch(); break;
            
            //  2F: cpl (M:1 T:4)
            // -- OVERLAP
            case 0x007B: _wait();_z80_cpl(cpu);_fetch(); break;
            
            //  30: jr CC-4,d (M:3 T:12)
            // -- M2
            case 0x007C: _wait();_mread(cpu->pc++); break;
            case 0x007D: cpu->dlatch=_gd();if(!(_cc_nc)){_z80_skip(cpu,1,7,2);}; break;
            // -- M3 (generic)
            case 0x007E: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;; break;
            // -- OVERLAP
            case 0x007F: _wait();_fetch(); break;
            
            //  31: ld RP,nn (M:3 T:10)
            // -- M2
            case 0x0080: _wait();_mread(cpu->pc++); break;
            case 0x0081: cpu->spl=_gd(); break;
            // -- M3
            case 0x0082: _wait();_mread(cpu->pc++); break;
            case 0x0083: cpu->sph=_gd(); break;
            // -- OVERLAP
            case 0x0084: _wait();_fetch(); break;
            
            //  32: ld (nn),a (M:4 T:13)
            // -- M2
            case 0x0085: _wait();_mread(cpu->pc++); break;
            case 0x0086: cpu->wzl=_gd(); break;
            // -- M3
            case 0x0087: _wait();_mread(cpu->pc++); break;
            case 0x0088: cpu->wzh=_gd(); break;
            // -- M4
            case 0x0089: _mwrite(cpu->wz++,cpu->a); break;
            case 0x008A: _wait();cpu->wzh=cpu->a; break;
            // -- OVERLAP
            case 0x008B: _wait();_fetch(); break;
            
            //  33: inc RP (M:2 T:6)
            // -- M2 (generic)
            case 0x008C: cpu->sp++; break;
            // -- OVERLAP
            case 0x008D: _wait();_fetch(); break;
            
            //  34: inc (hl) (M:3 T:11)
            // -- M2
            case 0x008E: _wait();_mread(cpu->addr); break;
            case 0x008F: cpu->dlatch=_gd();cpu->dlatch=_z80_inc8(cpu,cpu->dlatch); break;
            // -- M3
            case 0x0090: _mwrite(cpu->addr,cpu->dlatch); break;
            case 0x0091: _wait(); break;
            // -- OVERLAP
            case 0x0092: _wait();_fetch(); break;
            
            //  35: dec (hl) (M:3 T:11)
            // -- M2
            case 0x0093: _wait();_mread(cpu->addr); break;
            case 0x0094: cpu->dlatch=_gd();cpu->dlatch=_z80_dec8(cpu,cpu->dlatch); break;
            // -- M3
            case 0x0095: _mwrite(cpu->addr,cpu->dlatch); break;
            case 0x0096: _wait(); break;
            // -- OVERLAP
            case 0x0097: _wait();_fetch(); break;
            
            //  36: ld (hl),n (M:3 T:10)
            // -- M2
            case 0x0098: _wait();_mread(cpu->pc++); break;
            case 0x0099: cpu->dlatch=_gd(); break;
            // -- M3
            case 0x009A: _mwrite(cpu->addr,cpu->dlatch); break;
            case 0x009B: _wait(); break;
            // -- OVERLAP
            case 0x009C: _wait();_fetch(); break;
            
            //  37: scf (M:1 T:4)
            // -- OVERLAP
            case 0x009D: _wait();_z80_scf(cpu);_fetch(); break;
            
            //  38: jr CC-4,d (M:3 T:12)
            // -- M2
            case 0x009E: _wait();_mread(cpu->pc++); break;
            case 0x009F: cpu->dlatch=_gd();if(!(_cc_c)){_z80_skip(cpu,1,7,2);}; break;
            // -- M3 (generic)
            case 0x00A0: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;; break;
            // -- OVERLAP
            case 0x00A1: _wait();_fetch(); break;
            
            //  39: add hl,RP (M:2 T:11)
            // -- M2 (generic)
            case 0x00A2: _z80_add16(cpu,cpu->sp); break;
            // -- OVERLAP
            case 0x00A3: _wait();_fetch(); break;
            
            //  3A: ld a,(nn) (M:4 T:13)
            // -- M2
            case 0x00A4: _wait();_mread(cpu->pc++); break;
            case 0x00A5: cpu->wzl=_gd(); break;
            // -- M3
            case 0x00A6: _wait();_mread(cpu->pc++); break;
            case 0x00A7: cpu->wzh=_gd(); break;
            // -- M4
            case 0x00A8: _wait();_mread(cpu->wz++); break;
            case 0x00A9: cpu->a=_gd(); break;
            // -- OVERLAP
            case 0x00AA: _wait();_fetch(); break;
            
            //  3B: dec RP (M:2 T:6)
            // -- M2 (generic)
            case 0x00AB: cpu->sp--; break;
            // -- OVERLAP
            case 0x00AC: _wait();_fetch(); break;
            
            //  3C: inc RY (M:1 T:4)
            // -- OVERLAP
            case 0x00AD: _wait();cpu->a=_z80_inc8(cpu,cpu->a);_fetch(); break;
            
            //  3D: dec RY (M:1 T:4)
            // -- OVERLAP
            case 0x00AE: _wait();cpu->a=_z80_dec8(cpu,cpu->a);_fetch(); break;
            
            //  3E: ld RY,n (M:2 T:7)
            // -- M2
            case 0x00AF: _wait();_mread(cpu->pc++); break;
            case 0x00B0: cpu->a=_gd(); break;
            // -- OVERLAP
            case 0x00B1: _wait();_fetch(); break;
            
            //  3F: ccf (M:1 T:4)
            // -- OVERLAP
            case 0x00B2: _wait();_z80_ccf(cpu);_fetch(); break;
            
            //  40: ld RY,RZ (M:1 T:4)
            // -- OVERLAP
            case 0x00B3: _wait();cpu->b=cpu->b;_fetch(); break;
            
            //  41: ld RY,RZ (M:1 T:4)
            // -- OVERLAP
            case 0x00B4: _wait();cpu->b=cpu->c;_fetch(); break;
            
            //  42: ld RY,RZ (M:1 T:4)
            // -- OVERLAP
            case 0x00B5: _wait();cpu->b=cpu->d;_fetch(); break;
            
            //  43: ld RY,RZ (M:1 T:4)
            // -- OVERLAP
            case 0x00B6: _wait();cpu->b=cpu->e;_fetch(); break;
            
            //  44: ld RY,RZ (M:1 T:4)
            // -- OVERLAP
            case 0x00B7: _wait();cpu->b=cpu->hlx[cpu->hlx_idx].h;_fetch(); break;
            
            //  45: ld RY,RZ (M:1 T:4)
            // -- OVERLAP
            case 0x00B8: _wait();cpu->b=cpu->hlx[cpu->hlx_idx].l;_fetch(); break;
            
            //  46: ld RY,(hl) (M:2 T:7)
            // -- M2
            case 0x00B9: _wait();_mread(cpu->addr); break;
            case 0x00BA: cpu->b=_gd(); break;
            // -- OVERLAP
            case 0x00BB: _wait();_fetch(); break;
            
            //  47: ld RY,RZ (M:1 T:4)
            // -- OVERLAP
            case 0x00BC: _wait();cpu->b=cpu->a;_fetch(); break;
            
            //  48: ld RY,RZ (M:1 T:4)
            // -- OVERLAP
            case 0x00BD: _wait();cpu->c=cpu->b;_fetch(); break;
            
            //  49: ld RY,RZ (M:1 T:4)
            // -- OVERLAP
            case 0x00BE: _wait();cpu->c=cpu->c;_fetch(); break;
            
            //  4A: ld RY,RZ (M:1 T:4)
            // -- OVERLAP
            case 0x00BF: _wait();cpu->c=cpu->d;_fetch(); break;
            
            //  4B: ld RY,RZ (M:1 T:4)
            // -- OVERLAP
            case 0x00C0: _wait();cpu->c=cpu->e;_fetch(); break;
            
            //  4C: ld RY,RZ (M:1 T:4)
            // -- OVERLAP
            case 0x00C1: _wait();cpu->c=cpu->hlx[cpu->hlx_idx].h;_fetch(); break;
            
            //  4D: ld RY,RZ (M:1 T:4)
            // -- OVERLAP
            case 0x00C2: _wait();cpu->c=cpu->hlx[cpu->hlx_idx].l;_fetch(); break;
            
            //  4E: ld RY,(hl) (M:2 T:7)
            // -- M2
            case 0x00C3: _wait();_mread(cpu->addr); break;
            case 0x00C4: cpu->c=_gd(); break;
            // -- OVERLAP
            case 0x00C5: _wait();_fetch(); break;
            
            //  4F: ld RY,RZ (M:1 T:4)
            // -- OVERLAP
            case 0x00C6: _wait();cpu->c=cpu->a;_fetch(); break;
            
            //  50: ld RY,RZ (M:1 T:4)
            // -- OVERLAP
            case 0x00C7: _wait();cpu->d=cpu->b;_fetch(); break;
            
            //  51: ld RY,RZ (M:1 T:4)
            // -- OVERLAP
            case 0x00C8: _wait();cpu->d=cpu->c;_fetch(); break;
            
            //  52: ld RY,RZ (M:1 T:4)
            // -- OVERLAP
            case 0x00C9: _wait();cpu->d=cpu->d;_fetch(); break;
            
            //  53: ld RY,RZ (M:1 T:4)
            // -- OVERLAP
            case 0x00CA: _wait();cpu->d=cpu->e;_fetch(); break;
            
            //  54: ld RY,RZ (M:1 T:4)
            // -- OVERLAP
            case 0x00CB: _wait();cpu->d=cpu->hlx[cpu->hlx_idx].h;_fetch(); break;
            
            //  55: ld RY,RZ (M:1 T:4)
            // -- OVERLAP
            case 0x00CC: _wait();cpu->d=cpu->hlx[cpu->hlx_idx].l;_fetch(); break;
            
            //  56: ld RY,(hl) (M:2 T:7)
            // -- M2
            case 0x00CD: _wait();_mread(cpu->addr); break;
            case 0x00CE: cpu->d=_gd(); break;
            // -- OVERLAP
            case 0x00CF: _wait();_fetch(); break;
            
            //  57: ld RY,RZ (M:1 T:4)
            // -- OVERLAP
            case 0x00D0: _wait();cpu->d=cpu->a;_fetch(); break;
            
            //  58: ld RY,RZ (M:1 T:4)
            // -- OVERLAP
            case 0x00D1: _wait();cpu->e=cpu->b;_fetch(); break;
            
            //  59: ld RY,RZ (M:1 T:4)
            // -- OVERLAP
            case 0x00D2: _wait();cpu->e=cpu->c;_fetch(); break;
            
            //  5A: ld RY,RZ (M:1 T:4)
            // -- OVERLAP
            case 0x00D3: _wait();cpu->e=cpu->d;_fetch(); break;
            
            //  5B: ld RY,RZ (M:1 T:4)
            // -- OVERLAP
            case 0x00D4: _wait();cpu->e=cpu->e;_fetch(); break;
            
            //  5C: ld RY,RZ (M:1 T:4)
            // -- OVERLAP
            case 0x00D5: _wait();cpu->e=cpu->hlx[cpu->hlx_idx].h;_fetch(); break;
            
            //  5D: ld RY,RZ (M:1 T:4)
            // -- OVERLAP
            case 0x00D6: _wait();cpu->e=cpu->hlx[cpu->hlx_idx].l;_fetch(); break;
            
            //  5E: ld RY,(hl) (M:2 T:7)
            // -- M2
            case 0x00D7: _wait();_mread(cpu->addr); break;
            case 0x00D8: cpu->e=_gd(); break;
            // -- OVERLAP
            case 0x00D9: _wait();_fetch(); break;
            
            //  5F: ld RY,RZ (M:1 T:4)
            // -- OVERLAP
            case 0x00DA: _wait();cpu->e=cpu->a;_fetch(); break;
            
            //  60: ld RY,RZ (M:1 T:4)
            // -- OVERLAP
            case 0x00DB: _wait();cpu->hlx[cpu->hlx_idx].h=cpu->b;_fetch(); break;
            
            //  61: ld RY,RZ (M:1 T:4)
            // -- OVERLAP
            case 0x00DC: _wait();cpu->hlx[cpu->hlx_idx].h=cpu->c;_fetch(); break;
            
            //  62: ld RY,RZ (M:1 T:4)
            // -- OVERLAP
            case 0x00DD: _wait();cpu->hlx[cpu->hlx_idx].h=cpu->d;_fetch(); break;
            
            //  63: ld RY,RZ (M:1 T:4)
            // -- OVERLAP
            case 0x00DE: _wait();cpu->hlx[cpu->hlx_idx].h=cpu->e;_fetch(); break;
            
            //  64: ld RY,RZ (M:1 T:4)
            // -- OVERLAP
            case 0x00DF: _wait();cpu->hlx[cpu->hlx_idx].h=cpu->hlx[cpu->hlx_idx].h;_fetch(); break;
            
            //  65: ld RY,RZ (M:1 T:4)
            // -- OVERLAP
            case 0x00E0: _wait();cpu->hlx[cpu->hlx_idx].h=cpu->hlx[cpu->hlx_idx].l;_fetch(); break;
            
            //  66: ld RY,(hl) (M:2 T:7)
            // -- M2
            case 0x00E1: _wait();_mread(cpu->addr); break;
            case 0x00E2: cpu->h=_gd(); break;
            // -- OVERLAP
            case 0x00E3: _wait();_fetch(); break;
            
            //  67: ld RY,RZ (M:1 T:4)
            // -- OVERLAP
            case 0x00E4: _wait();cpu->hlx[cpu->hlx_idx].h=cpu->a;_fetch(); break;
            
            //  68: ld RY,RZ (M:1 T:4)
            // -- OVERLAP
            case 0x00E5: _wait();cpu->hlx[cpu->hlx_idx].l=cpu->b;_fetch(); break;
            
            //  69: ld RY,RZ (M:1 T:4)
            // -- OVERLAP
            case 0x00E6: _wait();cpu->hlx[cpu->hlx_idx].l=cpu->c;_fetch(); break;
            
            //  6A: ld RY,RZ (M:1 T:4)
            // -- OVERLAP
            case 0x00E7: _wait();cpu->hlx[cpu->hlx_idx].l=cpu->d;_fetch(); break;
            
            //  6B: ld RY,RZ (M:1 T:4)
            // -- OVERLAP
            case 0x00E8: _wait();cpu->hlx[cpu->hlx_idx].l=cpu->e;_fetch(); break;
            
            //  6C: ld RY,RZ (M:1 T:4)
            // -- OVERLAP
            case 0x00E9: _wait();cpu->hlx[cpu->hlx_idx].l=cpu->hlx[cpu->hlx_idx].h;_fetch(); break;
            
            //  6D: ld RY,RZ (M:1 T:4)
            // -- OVERLAP
            case 0x00EA: _wait();cpu->hlx[cpu->hlx_idx].l=cpu->hlx[cpu->hlx_idx].l;_fetch(); break;
            
            //  6E: ld RY,(hl) (M:2 T:7)
            // -- M2
            case 0x00EB: _wait();_mread(cpu->addr); break;
            case 0x00EC: cpu->l=_gd(); break;
            // -- OVERLAP
            case 0x00ED: _wait();_fetch(); break;
            
            //  6F: ld RY,RZ (M:1 T:4)
            // -- OVERLAP
            case 0x00EE: _wait();cpu->hlx[cpu->hlx_idx].l=cpu->a;_fetch(); break;
            
            //  70: ld (hl),RZ (M:2 T:7)
            // -- M2
            case 0x00EF: _mwrite(cpu->addr,cpu->b); break;
            case 0x00F0: _wait(); break;
            // -- OVERLAP
            case 0x00F1: _wait();_fetch(); break;
            
            //  71: ld (hl),RZ (M:2 T:7)
            // -- M2
            case 0x00F2: _mwrite(cpu->addr,cpu->c); break;
            case 0x00F3: _wait(); break;
            // -- OVERLAP
            case 0x00F4: _wait();_fetch(); break;
            
            //  72: ld (hl),RZ (M:2 T:7)
            // -- M2
            case 0x00F5: _mwrite(cpu->addr,cpu->d); break;
            case 0x00F6: _wait(); break;
            // -- OVERLAP
            case 0x00F7: _wait();_fetch(); break;
            
            //  73: ld (hl),RZ (M:2 T:7)
            // -- M2
            case 0x00F8: _mwrite(cpu->addr,cpu->e); break;
            case 0x00F9: _wait(); break;
            // -- OVERLAP
            case 0x00FA: _wait();_fetch(); break;
            
            //  74: ld (hl),RZ (M:2 T:7)
            // -- M2
            case 0x00FB: _mwrite(cpu->addr,cpu->h); break;
            case 0x00FC: _wait(); break;
            // -- OVERLAP
            case 0x00FD: _wait();_fetch(); break;
            
            //  75: ld (hl),RZ (M:2 T:7)
            // -- M2
            case 0x00FE: _mwrite(cpu->addr,cpu->l); break;
            case 0x00FF: _wait(); break;
            // -- OVERLAP
            case 0x0100: _wait();_fetch(); break;
            
            //  76: halt (M:1 T:4)
            // -- OVERLAP
            case 0x0101: _wait();pins=_z80_halt(cpu,pins);_fetch(); break;
            
            //  77: ld (hl),RZ (M:2 T:7)
            // -- M2
            case 0x0102: _mwrite(cpu->addr,cpu->a); break;
            case 0x0103: _wait(); break;
            // -- OVERLAP
            case 0x0104: _wait();_fetch(); break;
            
            //  78: ld RY,RZ (M:1 T:4)
            // -- OVERLAP
            case 0x0105: _wait();cpu->a=cpu->b;_fetch(); break;
            
            //  79: ld RY,RZ (M:1 T:4)
            // -- OVERLAP
            case 0x0106: _wait();cpu->a=cpu->c;_fetch(); break;
            
            //  7A: ld RY,RZ (M:1 T:4)
            // -- OVERLAP
            case 0x0107: _wait();cpu->a=cpu->d;_fetch(); break;
            
            //  7B: ld RY,RZ (M:1 T:4)
            // -- OVERLAP
            case 0x0108: _wait();cpu->a=cpu->e;_fetch(); break;
            
            //  7C: ld RY,RZ (M:1 T:4)
            // -- OVERLAP
            case 0x0109: _wait();cpu->a=cpu->hlx[cpu->hlx_idx].h;_fetch(); break;
            
            //  7D: ld RY,RZ (M:1 T:4)
            // -- OVERLAP
            case 0x010A: _wait();cpu->a=cpu->hlx[cpu->hlx_idx].l;_fetch(); break;
            
            //  7E: ld RY,(hl) (M:2 T:7)
            // -- M2
            case 0x010B: _wait();_mread(cpu->addr); break;
            case 0x010C: cpu->a=_gd(); break;
            // -- OVERLAP
            case 0x010D: _wait();_fetch(); break;
            
            //  7F: ld RY,RZ (M:1 T:4)
            // -- OVERLAP
            case 0x010E: _wait();cpu->a=cpu->a;_fetch(); break;
            
            //  80: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x010F: _wait();_z80_add8(cpu,cpu->b);_fetch(); break;
            
            //  81: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x0110: _wait();_z80_add8(cpu,cpu->c);_fetch(); break;
            
            //  82: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x0111: _wait();_z80_add8(cpu,cpu->d);_fetch(); break;
            
            //  83: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x0112: _wait();_z80_add8(cpu,cpu->e);_fetch(); break;
            
            //  84: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x0113: _wait();_z80_add8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); break;
            
            //  85: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x0114: _wait();_z80_add8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); break;
            
            //  86: ALU (hl) (M:2 T:7)
            // -- M2
            case 0x0115: _wait();_mread(cpu->addr); break;
            case 0x0116: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x0117: _wait();_z80_add8(cpu,cpu->dlatch);_fetch(); break;
            
            //  87: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x0118: _wait();_z80_add8(cpu,cpu->a);_fetch(); break;
            
            //  88: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x0119: _wait();_z80_adc8(cpu,cpu->b);_fetch(); break;
            
            //  89: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x011A: _wait();_z80_adc8(cpu,cpu->c);_fetch(); break;
            
            //  8A: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x011B: _wait();_z80_adc8(cpu,cpu->d);_fetch(); break;
            
            //  8B: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x011C: _wait();_z80_adc8(cpu,cpu->e);_fetch(); break;
            
            //  8C: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x011D: _wait();_z80_adc8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); break;
            
            //  8D: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x011E: _wait();_z80_adc8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); break;
            
            //  8E: ALU (hl) (M:2 T:7)
            // -- M2
            case 0x011F: _wait();_mread(cpu->addr); break;
            case 0x0120: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x0121: _wait();_z80_adc8(cpu,cpu->dlatch);_fetch(); break;
            
            //  8F: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x0122: _wait();_z80_adc8(cpu,cpu->a);_fetch(); break;
            
            //  90: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x0123: _wait();_z80_sub8(cpu,cpu->b);_fetch(); break;
            
            //  91: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x0124: _wait();_z80_sub8(cpu,cpu->c);_fetch(); break;
            
            //  92: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x0125: _wait();_z80_sub8(cpu,cpu->d);_fetch(); break;
            
            //  93: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x0126: _wait();_z80_sub8(cpu,cpu->e);_fetch(); break;
            
            //  94: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x0127: _wait();_z80_sub8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); break;
            
            //  95: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x0128: _wait();_z80_sub8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); break;
            
            //  96: ALU (hl) (M:2 T:7)
            // -- M2
            case 0x0129: _wait();_mread(cpu->addr); break;
            case 0x012A: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x012B: _wait();_z80_sub8(cpu,cpu->dlatch);_fetch(); break;
            
            //  97: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x012C: _wait();_z80_sub8(cpu,cpu->a);_fetch(); break;
            
            //  98: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x012D: _wait();_z80_sbc8(cpu,cpu->b);_fetch(); break;
            
            //  99: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x012E: _wait();_z80_sbc8(cpu,cpu->c);_fetch(); break;
            
            //  9A: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x012F: _wait();_z80_sbc8(cpu,cpu->d);_fetch(); break;
            
            //  9B: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x0130: _wait();_z80_sbc8(cpu,cpu->e);_fetch(); break;
            
            //  9C: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x0131: _wait();_z80_sbc8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); break;
            
            //  9D: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x0132: _wait();_z80_sbc8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); break;
            
            //  9E: ALU (hl) (M:2 T:7)
            // -- M2
            case 0x0133: _wait();_mread(cpu->addr); break;
            case 0x0134: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x0135: _wait();_z80_sbc8(cpu,cpu->dlatch);_fetch(); break;
            
            //  9F: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x0136: _wait();_z80_sbc8(cpu,cpu->a);_fetch(); break;
            
            //  A0: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x0137: _wait();_z80_and8(cpu,cpu->b);_fetch(); break;
            
            //  A1: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x0138: _wait();_z80_and8(cpu,cpu->c);_fetch(); break;
            
            //  A2: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x0139: _wait();_z80_and8(cpu,cpu->d);_fetch(); break;
            
            //  A3: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x013A: _wait();_z80_and8(cpu,cpu->e);_fetch(); break;
            
            //  A4: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x013B: _wait();_z80_and8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); break;
            
            //  A5: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x013C: _wait();_z80_and8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); break;
            
            //  A6: ALU (hl) (M:2 T:7)
            // -- M2
            case 0x013D: _wait();_mread(cpu->addr); break;
            case 0x013E: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x013F: _wait();_z80_and8(cpu,cpu->dlatch);_fetch(); break;
            
            //  A7: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x0140: _wait();_z80_and8(cpu,cpu->a);_fetch(); break;
            
            //  A8: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x0141: _wait();_z80_xor8(cpu,cpu->b);_fetch(); break;
            
            //  A9: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x0142: _wait();_z80_xor8(cpu,cpu->c);_fetch(); break;
            
            //  AA: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x0143: _wait();_z80_xor8(cpu,cpu->d);_fetch(); break;
            
            //  AB: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x0144: _wait();_z80_xor8(cpu,cpu->e);_fetch(); break;
            
            //  AC: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x0145: _wait();_z80_xor8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); break;
            
            //  AD: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x0146: _wait();_z80_xor8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); break;
            
            //  AE: ALU (hl) (M:2 T:7)
            // -- M2
            case 0x0147: _wait();_mread(cpu->addr); break;
            case 0x0148: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x0149: _wait();_z80_xor8(cpu,cpu->dlatch);_fetch(); break;
            
            //  AF: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x014A: _wait();_z80_xor8(cpu,cpu->a);_fetch(); break;
            
            //  B0: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x014B: _wait();_z80_or8(cpu,cpu->b);_fetch(); break;
            
            //  B1: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x014C: _wait();_z80_or8(cpu,cpu->c);_fetch(); break;
            
            //  B2: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x014D: _wait();_z80_or8(cpu,cpu->d);_fetch(); break;
            
            //  B3: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x014E: _wait();_z80_or8(cpu,cpu->e);_fetch(); break;
            
            //  B4: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x014F: _wait();_z80_or8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); break;
            
            //  B5: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x0150: _wait();_z80_or8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); break;
            
            //  B6: ALU (hl) (M:2 T:7)
            // -- M2
            case 0x0151: _wait();_mread(cpu->addr); break;
            case 0x0152: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x0153: _wait();_z80_or8(cpu,cpu->dlatch);_fetch(); break;
            
            //  B7: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x0154: _wait();_z80_or8(cpu,cpu->a);_fetch(); break;
            
            //  B8: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x0155: _wait();_z80_cp8(cpu,cpu->b);_fetch(); break;
            
            //  B9: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x0156: _wait();_z80_cp8(cpu,cpu->c);_fetch(); break;
            
            //  BA: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x0157: _wait();_z80_cp8(cpu,cpu->d);_fetch(); break;
            
            //  BB: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x0158: _wait();_z80_cp8(cpu,cpu->e);_fetch(); break;
            
            //  BC: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x0159: _wait();_z80_cp8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch(); break;
            
            //  BD: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x015A: _wait();_z80_cp8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch(); break;
            
            //  BE: ALU (hl) (M:2 T:7)
            // -- M2
            case 0x015B: _wait();_mread(cpu->addr); break;
            case 0x015C: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x015D: _wait();_z80_cp8(cpu,cpu->dlatch);_fetch(); break;
            
            //  BF: ALU RZ (M:1 T:4)
            // -- OVERLAP
            case 0x015E: _wait();_z80_cp8(cpu,cpu->a);_fetch(); break;
            
            //  C0: ret CC (M:4 T:11)
            // -- M2 (generic)
            case 0x015F: if(!_cc_nz){_z80_skip(cpu,4,7,1);}; break;
            // -- M3
            case 0x0160: _wait();_mread(cpu->sp++); break;
            case 0x0161: cpu->wzl=_gd(); break;
            // -- M4
            case 0x0162: _wait();_mread(cpu->sp++); break;
            case 0x0163: cpu->wzh=_gd();cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x0164: _wait();_fetch(); break;
            
            //  C1: pop RP2 (M:3 T:10)
            // -- M2
            case 0x0165: _wait();_mread(cpu->sp++); break;
            case 0x0166: cpu->c=_gd(); break;
            // -- M3
            case 0x0167: _wait();_mread(cpu->sp++); break;
            case 0x0168: cpu->b=_gd(); break;
            // -- OVERLAP
            case 0x0169: _wait();_fetch(); break;
            
            //  C2: jp CC,nn (M:3 T:10)
            // -- M2
            case 0x016A: _wait();_mread(cpu->pc++); break;
            case 0x016B: cpu->wzl=_gd(); break;
            // -- M3
            case 0x016C: _wait();_mread(cpu->pc++); break;
            case 0x016D: cpu->wzh=_gd();if(_cc_nz){cpu->pc=cpu->wz;}; break;
            // -- OVERLAP
            case 0x016E: _wait();_fetch(); break;
            
            //  C3: jp nn (M:3 T:10)
            // -- M2
            case 0x016F: _wait();_mread(cpu->pc++); break;
            case 0x0170: cpu->wzl=_gd(); break;
            // -- M3
            case 0x0171: _wait();_mread(cpu->pc++); break;
            case 0x0172: cpu->wzh=_gd();cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x0173: _wait();_fetch(); break;
            
            //  C4: call CC,nn (M:6 T:17)
            // -- M2
            case 0x0174: _wait();_mread(cpu->pc++); break;
            case 0x0175: cpu->wzl=_gd(); break;
            // -- M3
            case 0x0176: _wait();_mread(cpu->pc++); break;
            case 0x0177: cpu->wzh=_gd();if (!_cc_nz){_z80_skip(cpu,5,9,2);}; break;
            // -- M4 (generic)
            case 0x0178:  break;
            // -- M5
            case 0x0179: _mwrite(--cpu->sp,cpu->pch); break;
            case 0x017A: _wait(); break;
            // -- M6
            case 0x017B: _mwrite(--cpu->sp,cpu->pcl); break;
            case 0x017C: _wait();cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x017D: _wait();_fetch(); break;
            
            //  C5: push RP2 (M:3 T:11)
            // -- M2
            case 0x017E: _mwrite(--cpu->sp,cpu->b); break;
            case 0x017F: _wait(); break;
            // -- M3
            case 0x0180: _mwrite(--cpu->sp,cpu->c); break;
            case 0x0181: _wait(); break;
            // -- OVERLAP
            case 0x0182: _wait();_fetch(); break;
            
            //  C6: ALU n (M:2 T:7)
            // -- M2
            case 0x0183: _wait();_mread(cpu->pc++); break;
            case 0x0184: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x0185: _wait();_z80_add8(cpu,cpu->dlatch);_fetch(); break;
            
            //  C7: rst Y*8 (M:3 T:11)
            // -- M2
            case 0x0186: _mwrite(--cpu->sp,cpu->pch); break;
            case 0x0187: _wait(); break;
            // -- M3
            case 0x0188: _mwrite(--cpu->sp,cpu->pcl); break;
            case 0x0189: _wait();cpu->wz=0x00;cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x018A: _wait();_fetch(); break;
            
            //  C8: ret CC (M:4 T:11)
            // -- M2 (generic)
            case 0x018B: if(!_cc_z){_z80_skip(cpu,4,7,1);}; break;
            // -- M3
            case 0x018C: _wait();_mread(cpu->sp++); break;
            case 0x018D: cpu->wzl=_gd(); break;
            // -- M4
            case 0x018E: _wait();_mread(cpu->sp++); break;
            case 0x018F: cpu->wzh=_gd();cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x0190: _wait();_fetch(); break;
            
            //  C9: ret (M:3 T:10)
            // -- M2
            case 0x0191: _wait();_mread(cpu->sp++); break;
            case 0x0192: cpu->wzl=_gd(); break;
            // -- M3
            case 0x0193: _wait();_mread(cpu->sp++); break;
            case 0x0194: cpu->wzh=_gd();cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x0195: _wait();_fetch(); break;
            
            //  CA: jp CC,nn (M:3 T:10)
            // -- M2
            case 0x0196: _wait();_mread(cpu->pc++); break;
            case 0x0197: cpu->wzl=_gd(); break;
            // -- M3
            case 0x0198: _wait();_mread(cpu->pc++); break;
            case 0x0199: cpu->wzh=_gd();if(_cc_z){cpu->pc=cpu->wz;}; break;
            // -- OVERLAP
            case 0x019A: _wait();_fetch(); break;
            
            //  CB: cb prefix (M:1 T:4)
            // -- OVERLAP
            case 0x019B: _wait();_fetch_cb(); break;
            
            //  CC: call CC,nn (M:6 T:17)
            // -- M2
            case 0x019C: _wait();_mread(cpu->pc++); break;
            case 0x019D: cpu->wzl=_gd(); break;
            // -- M3
            case 0x019E: _wait();_mread(cpu->pc++); break;
            case 0x019F: cpu->wzh=_gd();if (!_cc_z){_z80_skip(cpu,5,9,2);}; break;
            // -- M4 (generic)
            case 0x01A0:  break;
            // -- M5
            case 0x01A1: _mwrite(--cpu->sp,cpu->pch); break;
            case 0x01A2: _wait(); break;
            // -- M6
            case 0x01A3: _mwrite(--cpu->sp,cpu->pcl); break;
            case 0x01A4: _wait();cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x01A5: _wait();_fetch(); break;
            
            //  CD: call nn (M:5 T:17)
            // -- M2
            case 0x01A6: _wait();_mread(cpu->pc++); break;
            case 0x01A7: cpu->wzl=_gd(); break;
            // -- M3
            case 0x01A8: _wait();_mread(cpu->pc++); break;
            case 0x01A9: cpu->wzh=_gd(); break;
            // -- M4
            case 0x01AA: _mwrite(--cpu->sp,cpu->pch); break;
            case 0x01AB: _wait(); break;
            // -- M5
            case 0x01AC: _mwrite(--cpu->sp,cpu->pcl); break;
            case 0x01AD: _wait();cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x01AE: _wait();_fetch(); break;
            
            //  CE: ALU n (M:2 T:7)
            // -- M2
            case 0x01AF: _wait();_mread(cpu->pc++); break;
            case 0x01B0: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x01B1: _wait();_z80_adc8(cpu,cpu->dlatch);_fetch(); break;
            
            //  CF: rst Y*8 (M:3 T:11)
            // -- M2
            case 0x01B2: _mwrite(--cpu->sp,cpu->pch); break;
            case 0x01B3: _wait(); break;
            // -- M3
            case 0x01B4: _mwrite(--cpu->sp,cpu->pcl); break;
            case 0x01B5: _wait();cpu->wz=0x08;cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x01B6: _wait();_fetch(); break;
            
            //  D0: ret CC (M:4 T:11)
            // -- M2 (generic)
            case 0x01B7: if(!_cc_nc){_z80_skip(cpu,4,7,1);}; break;
            // -- M3
            case 0x01B8: _wait();_mread(cpu->sp++); break;
            case 0x01B9: cpu->wzl=_gd(); break;
            // -- M4
            case 0x01BA: _wait();_mread(cpu->sp++); break;
            case 0x01BB: cpu->wzh=_gd();cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x01BC: _wait();_fetch(); break;
            
            //  D1: pop RP2 (M:3 T:10)
            // -- M2
            case 0x01BD: _wait();_mread(cpu->sp++); break;
            case 0x01BE: cpu->e=_gd(); break;
            // -- M3
            case 0x01BF: _wait();_mread(cpu->sp++); break;
            case 0x01C0: cpu->d=_gd(); break;
            // -- OVERLAP
            case 0x01C1: _wait();_fetch(); break;
            
            //  D2: jp CC,nn (M:3 T:10)
            // -- M2
            case 0x01C2: _wait();_mread(cpu->pc++); break;
            case 0x01C3: cpu->wzl=_gd(); break;
            // -- M3
            case 0x01C4: _wait();_mread(cpu->pc++); break;
            case 0x01C5: cpu->wzh=_gd();if(_cc_nc){cpu->pc=cpu->wz;}; break;
            // -- OVERLAP
            case 0x01C6: _wait();_fetch(); break;
            
            //  D3: out (n),a (M:3 T:11)
            // -- M2
            case 0x01C7: _wait();_mread(cpu->pc++); break;
            case 0x01C8: cpu->wzl=_gd();cpu->wzh=cpu->a; break;
            // -- M3 (iowrite)
            case 0x01C9: _iowrite(cpu->wz,cpu->a); break;
            case 0x01CA: _wait();cpu->wzl++; break;
            // -- OVERLAP
            case 0x01CB: _wait();_fetch(); break;
            
            //  D4: call CC,nn (M:6 T:17)
            // -- M2
            case 0x01CC: _wait();_mread(cpu->pc++); break;
            case 0x01CD: cpu->wzl=_gd(); break;
            // -- M3
            case 0x01CE: _wait();_mread(cpu->pc++); break;
            case 0x01CF: cpu->wzh=_gd();if (!_cc_nc){_z80_skip(cpu,5,9,2);}; break;
            // -- M4 (generic)
            case 0x01D0:  break;
            // -- M5
            case 0x01D1: _mwrite(--cpu->sp,cpu->pch); break;
            case 0x01D2: _wait(); break;
            // -- M6
            case 0x01D3: _mwrite(--cpu->sp,cpu->pcl); break;
            case 0x01D4: _wait();cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x01D5: _wait();_fetch(); break;
            
            //  D5: push RP2 (M:3 T:11)
            // -- M2
            case 0x01D6: _mwrite(--cpu->sp,cpu->d); break;
            case 0x01D7: _wait(); break;
            // -- M3
            case 0x01D8: _mwrite(--cpu->sp,cpu->e); break;
            case 0x01D9: _wait(); break;
            // -- OVERLAP
            case 0x01DA: _wait();_fetch(); break;
            
            //  D6: ALU n (M:2 T:7)
            // -- M2
            case 0x01DB: _wait();_mread(cpu->pc++); break;
            case 0x01DC: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x01DD: _wait();_z80_sub8(cpu,cpu->dlatch);_fetch(); break;
            
            //  D7: rst Y*8 (M:3 T:11)
            // -- M2
            case 0x01DE: _mwrite(--cpu->sp,cpu->pch); break;
            case 0x01DF: _wait(); break;
            // -- M3
            case 0x01E0: _mwrite(--cpu->sp,cpu->pcl); break;
            case 0x01E1: _wait();cpu->wz=0x10;cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x01E2: _wait();_fetch(); break;
            
            //  D8: ret CC (M:4 T:11)
            // -- M2 (generic)
            case 0x01E3: if(!_cc_c){_z80_skip(cpu,4,7,1);}; break;
            // -- M3
            case 0x01E4: _wait();_mread(cpu->sp++); break;
            case 0x01E5: cpu->wzl=_gd(); break;
            // -- M4
            case 0x01E6: _wait();_mread(cpu->sp++); break;
            case 0x01E7: cpu->wzh=_gd();cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x01E8: _wait();_fetch(); break;
            
            //  D9: exx (M:1 T:4)
            // -- OVERLAP
            case 0x01E9: _wait();_z80_exx(cpu);_fetch(); break;
            
            //  DA: jp CC,nn (M:3 T:10)
            // -- M2
            case 0x01EA: _wait();_mread(cpu->pc++); break;
            case 0x01EB: cpu->wzl=_gd(); break;
            // -- M3
            case 0x01EC: _wait();_mread(cpu->pc++); break;
            case 0x01ED: cpu->wzh=_gd();if(_cc_c){cpu->pc=cpu->wz;}; break;
            // -- OVERLAP
            case 0x01EE: _wait();_fetch(); break;
            
            //  DB: in a,(n) (M:3 T:11)
            // -- M2
            case 0x01EF: _wait();_mread(cpu->pc++); break;
            case 0x01F0: cpu->wzl=_gd();cpu->wzh=cpu->a; break;
            // -- M3 (ioread)
            case 0x01F1: _wait();_ioread(cpu->wz++); break;
            case 0x01F2: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x01F3: _wait();cpu->a=cpu->dlatch;_fetch(); break;
            
            //  DC: call CC,nn (M:6 T:17)
            // -- M2
            case 0x01F4: _wait();_mread(cpu->pc++); break;
            case 0x01F5: cpu->wzl=_gd(); break;
            // -- M3
            case 0x01F6: _wait();_mread(cpu->pc++); break;
            case 0x01F7: cpu->wzh=_gd();if (!_cc_c){_z80_skip(cpu,5,9,2);}; break;
            // -- M4 (generic)
            case 0x01F8:  break;
            // -- M5
            case 0x01F9: _mwrite(--cpu->sp,cpu->pch); break;
            case 0x01FA: _wait(); break;
            // -- M6
            case 0x01FB: _mwrite(--cpu->sp,cpu->pcl); break;
            case 0x01FC: _wait();cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x01FD: _wait();_fetch(); break;
            
            //  DD: dd prefix (M:1 T:4)
            // -- OVERLAP
            case 0x01FE: _wait();_fetch_dd(); break;
            
            //  DE: ALU n (M:2 T:7)
            // -- M2
            case 0x01FF: _wait();_mread(cpu->pc++); break;
            case 0x0200: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x0201: _wait();_z80_sbc8(cpu,cpu->dlatch);_fetch(); break;
            
            //  DF: rst Y*8 (M:3 T:11)
            // -- M2
            case 0x0202: _mwrite(--cpu->sp,cpu->pch); break;
            case 0x0203: _wait(); break;
            // -- M3
            case 0x0204: _mwrite(--cpu->sp,cpu->pcl); break;
            case 0x0205: _wait();cpu->wz=0x18;cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x0206: _wait();_fetch(); break;
            
            //  E0: ret CC (M:4 T:11)
            // -- M2 (generic)
            case 0x0207: if(!_cc_po){_z80_skip(cpu,4,7,1);}; break;
            // -- M3
            case 0x0208: _wait();_mread(cpu->sp++); break;
            case 0x0209: cpu->wzl=_gd(); break;
            // -- M4
            case 0x020A: _wait();_mread(cpu->sp++); break;
            case 0x020B: cpu->wzh=_gd();cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x020C: _wait();_fetch(); break;
            
            //  E1: pop RP2 (M:3 T:10)
            // -- M2
            case 0x020D: _wait();_mread(cpu->sp++); break;
            case 0x020E: cpu->hlx[cpu->hlx_idx].l=_gd(); break;
            // -- M3
            case 0x020F: _wait();_mread(cpu->sp++); break;
            case 0x0210: cpu->hlx[cpu->hlx_idx].h=_gd(); break;
            // -- OVERLAP
            case 0x0211: _wait();_fetch(); break;
            
            //  E2: jp CC,nn (M:3 T:10)
            // -- M2
            case 0x0212: _wait();_mread(cpu->pc++); break;
            case 0x0213: cpu->wzl=_gd(); break;
            // -- M3
            case 0x0214: _wait();_mread(cpu->pc++); break;
            case 0x0215: cpu->wzh=_gd();if(_cc_po){cpu->pc=cpu->wz;}; break;
            // -- OVERLAP
            case 0x0216: _wait();_fetch(); break;
            
            //  E3: ex (sp),hl (M:5 T:19)
            // -- M2
            case 0x0217: _wait();_mread(cpu->sp); break;
            case 0x0218: cpu->wzl=_gd(); break;
            // -- M3
            case 0x0219: _wait();_mread(cpu->sp+1); break;
            case 0x021A: cpu->wzh=_gd(); break;
            // -- M4
            case 0x021B: _mwrite(cpu->sp+1,cpu->hlx[cpu->hlx_idx].h); break;
            case 0x021C: _wait(); break;
            // -- M5
            case 0x021D: _mwrite(cpu->sp,cpu->hlx[cpu->hlx_idx].l); break;
            case 0x021E: _wait();cpu->hlx[cpu->hlx_idx].hl=cpu->wz; break;
            // -- OVERLAP
            case 0x021F: _wait();_fetch(); break;
            
            //  E4: call CC,nn (M:6 T:17)
            // -- M2
            case 0x0220: _wait();_mread(cpu->pc++); break;
            case 0x0221: cpu->wzl=_gd(); break;
            // -- M3
            case 0x0222: _wait();_mread(cpu->pc++); break;
            case 0x0223: cpu->wzh=_gd();if (!_cc_po){_z80_skip(cpu,5,9,2);}; break;
            // -- M4 (generic)
            case 0x0224:  break;
            // -- M5
            case 0x0225: _mwrite(--cpu->sp,cpu->pch); break;
            case 0x0226: _wait(); break;
            // -- M6
            case 0x0227: _mwrite(--cpu->sp,cpu->pcl); break;
            case 0x0228: _wait();cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x0229: _wait();_fetch(); break;
            
            //  E5: push RP2 (M:3 T:11)
            // -- M2
            case 0x022A: _mwrite(--cpu->sp,cpu->hlx[cpu->hlx_idx].h); break;
            case 0x022B: _wait(); break;
            // -- M3
            case 0x022C: _mwrite(--cpu->sp,cpu->hlx[cpu->hlx_idx].l); break;
            case 0x022D: _wait(); break;
            // -- OVERLAP
            case 0x022E: _wait();_fetch(); break;
            
            //  E6: ALU n (M:2 T:7)
            // -- M2
            case 0x022F: _wait();_mread(cpu->pc++); break;
            case 0x0230: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x0231: _wait();_z80_and8(cpu,cpu->dlatch);_fetch(); break;
            
            //  E7: rst Y*8 (M:3 T:11)
            // -- M2
            case 0x0232: _mwrite(--cpu->sp,cpu->pch); break;
            case 0x0233: _wait(); break;
            // -- M3
            case 0x0234: _mwrite(--cpu->sp,cpu->pcl); break;
            case 0x0235: _wait();cpu->wz=0x20;cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x0236: _wait();_fetch(); break;
            
            //  E8: ret CC (M:4 T:11)
            // -- M2 (generic)
            case 0x0237: if(!_cc_pe){_z80_skip(cpu,4,7,1);}; break;
            // -- M3
            case 0x0238: _wait();_mread(cpu->sp++); break;
            case 0x0239: cpu->wzl=_gd(); break;
            // -- M4
            case 0x023A: _wait();_mread(cpu->sp++); break;
            case 0x023B: cpu->wzh=_gd();cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x023C: _wait();_fetch(); break;
            
            //  E9: jp hl (M:1 T:4)
            // -- OVERLAP
            case 0x023D: _wait();cpu->pc=cpu->hlx[cpu->hlx_idx].hl;_fetch(); break;
            
            //  EA: jp CC,nn (M:3 T:10)
            // -- M2
            case 0x023E: _wait();_mread(cpu->pc++); break;
            case 0x023F: cpu->wzl=_gd(); break;
            // -- M3
            case 0x0240: _wait();_mread(cpu->pc++); break;
            case 0x0241: cpu->wzh=_gd();if(_cc_pe){cpu->pc=cpu->wz;}; break;
            // -- OVERLAP
            case 0x0242: _wait();_fetch(); break;
            
            //  EB: ex de,hl (M:1 T:4)
            // -- OVERLAP
            case 0x0243: _wait();_z80_ex_de_hl(cpu);_fetch(); break;
            
            //  EC: call CC,nn (M:6 T:17)
            // -- M2
            case 0x0244: _wait();_mread(cpu->pc++); break;
            case 0x0245: cpu->wzl=_gd(); break;
            // -- M3
            case 0x0246: _wait();_mread(cpu->pc++); break;
            case 0x0247: cpu->wzh=_gd();if (!_cc_pe){_z80_skip(cpu,5,9,2);}; break;
            // -- M4 (generic)
            case 0x0248:  break;
            // -- M5
            case 0x0249: _mwrite(--cpu->sp,cpu->pch); break;
            case 0x024A: _wait(); break;
            // -- M6
            case 0x024B: _mwrite(--cpu->sp,cpu->pcl); break;
            case 0x024C: _wait();cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x024D: _wait();_fetch(); break;
            
            //  ED: ed prefix (M:1 T:4)
            // -- OVERLAP
            case 0x024E: _wait();_fetch_ed(); break;
            
            //  EE: ALU n (M:2 T:7)
            // -- M2
            case 0x024F: _wait();_mread(cpu->pc++); break;
            case 0x0250: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x0251: _wait();_z80_xor8(cpu,cpu->dlatch);_fetch(); break;
            
            //  EF: rst Y*8 (M:3 T:11)
            // -- M2
            case 0x0252: _mwrite(--cpu->sp,cpu->pch); break;
            case 0x0253: _wait(); break;
            // -- M3
            case 0x0254: _mwrite(--cpu->sp,cpu->pcl); break;
            case 0x0255: _wait();cpu->wz=0x28;cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x0256: _wait();_fetch(); break;
            
            //  F0: ret CC (M:4 T:11)
            // -- M2 (generic)
            case 0x0257: if(!_cc_p){_z80_skip(cpu,4,7,1);}; break;
            // -- M3
            case 0x0258: _wait();_mread(cpu->sp++); break;
            case 0x0259: cpu->wzl=_gd(); break;
            // -- M4
            case 0x025A: _wait();_mread(cpu->sp++); break;
            case 0x025B: cpu->wzh=_gd();cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x025C: _wait();_fetch(); break;
            
            //  F1: pop RP2 (M:3 T:10)
            // -- M2
            case 0x025D: _wait();_mread(cpu->sp++); break;
            case 0x025E: cpu->f=_gd(); break;
            // -- M3
            case 0x025F: _wait();_mread(cpu->sp++); break;
            case 0x0260: cpu->a=_gd(); break;
            // -- OVERLAP
            case 0x0261: _wait();_fetch(); break;
            
            //  F2: jp CC,nn (M:3 T:10)
            // -- M2
            case 0x0262: _wait();_mread(cpu->pc++); break;
            case 0x0263: cpu->wzl=_gd(); break;
            // -- M3
            case 0x0264: _wait();_mread(cpu->pc++); break;
            case 0x0265: cpu->wzh=_gd();if(_cc_p){cpu->pc=cpu->wz;}; break;
            // -- OVERLAP
            case 0x0266: _wait();_fetch(); break;
            
            //  F3: di (M:1 T:4)
            // -- OVERLAP
            case 0x0267: _wait();cpu->iff1=cpu->iff2=false;;_fetch(); break;
            
            //  F4: call CC,nn (M:6 T:17)
            // -- M2
            case 0x0268: _wait();_mread(cpu->pc++); break;
            case 0x0269: cpu->wzl=_gd(); break;
            // -- M3
            case 0x026A: _wait();_mread(cpu->pc++); break;
            case 0x026B: cpu->wzh=_gd();if (!_cc_p){_z80_skip(cpu,5,9,2);}; break;
            // -- M4 (generic)
            case 0x026C:  break;
            // -- M5
            case 0x026D: _mwrite(--cpu->sp,cpu->pch); break;
            case 0x026E: _wait(); break;
            // -- M6
            case 0x026F: _mwrite(--cpu->sp,cpu->pcl); break;
            case 0x0270: _wait();cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x0271: _wait();_fetch(); break;
            
            //  F5: push RP2 (M:3 T:11)
            // -- M2
            case 0x0272: _mwrite(--cpu->sp,cpu->a); break;
            case 0x0273: _wait(); break;
            // -- M3
            case 0x0274: _mwrite(--cpu->sp,cpu->f); break;
            case 0x0275: _wait(); break;
            // -- OVERLAP
            case 0x0276: _wait();_fetch(); break;
            
            //  F6: ALU n (M:2 T:7)
            // -- M2
            case 0x0277: _wait();_mread(cpu->pc++); break;
            case 0x0278: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x0279: _wait();_z80_or8(cpu,cpu->dlatch);_fetch(); break;
            
            //  F7: rst Y*8 (M:3 T:11)
            // -- M2
            case 0x027A: _mwrite(--cpu->sp,cpu->pch); break;
            case 0x027B: _wait(); break;
            // -- M3
            case 0x027C: _mwrite(--cpu->sp,cpu->pcl); break;
            case 0x027D: _wait();cpu->wz=0x30;cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x027E: _wait();_fetch(); break;
            
            //  F8: ret CC (M:4 T:11)
            // -- M2 (generic)
            case 0x027F: if(!_cc_m){_z80_skip(cpu,4,7,1);}; break;
            // -- M3
            case 0x0280: _wait();_mread(cpu->sp++); break;
            case 0x0281: cpu->wzl=_gd(); break;
            // -- M4
            case 0x0282: _wait();_mread(cpu->sp++); break;
            case 0x0283: cpu->wzh=_gd();cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x0284: _wait();_fetch(); break;
            
            //  F9: ld sp,hl (M:2 T:6)
            // -- M2 (generic)
            case 0x0285: cpu->sp=cpu->hlx[cpu->hlx_idx].hl; break;
            // -- OVERLAP
            case 0x0286: _wait();_fetch(); break;
            
            //  FA: jp CC,nn (M:3 T:10)
            // -- M2
            case 0x0287: _wait();_mread(cpu->pc++); break;
            case 0x0288: cpu->wzl=_gd(); break;
            // -- M3
            case 0x0289: _wait();_mread(cpu->pc++); break;
            case 0x028A: cpu->wzh=_gd();if(_cc_m){cpu->pc=cpu->wz;}; break;
            // -- OVERLAP
            case 0x028B: _wait();_fetch(); break;
            
            //  FB: ei (M:1 T:4)
            // -- OVERLAP
            case 0x028C: _wait();cpu->iff1=cpu->iff2=false;_fetch();cpu->iff1=cpu->iff2=true; break;
            
            //  FC: call CC,nn (M:6 T:17)
            // -- M2
            case 0x028D: _wait();_mread(cpu->pc++); break;
            case 0x028E: cpu->wzl=_gd(); break;
            // -- M3
            case 0x028F: _wait();_mread(cpu->pc++); break;
            case 0x0290: cpu->wzh=_gd();if (!_cc_m){_z80_skip(cpu,5,9,2);}; break;
            // -- M4 (generic)
            case 0x0291:  break;
            // -- M5
            case 0x0292: _mwrite(--cpu->sp,cpu->pch); break;
            case 0x0293: _wait(); break;
            // -- M6
            case 0x0294: _mwrite(--cpu->sp,cpu->pcl); break;
            case 0x0295: _wait();cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x0296: _wait();_fetch(); break;
            
            //  FD: fd prefix (M:1 T:4)
            // -- OVERLAP
            case 0x0297: _wait();_fetch_fd(); break;
            
            //  FE: ALU n (M:2 T:7)
            // -- M2
            case 0x0298: _wait();_mread(cpu->pc++); break;
            case 0x0299: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x029A: _wait();_z80_cp8(cpu,cpu->dlatch);_fetch(); break;
            
            //  FF: rst Y*8 (M:3 T:11)
            // -- M2
            case 0x029B: _mwrite(--cpu->sp,cpu->pch); break;
            case 0x029C: _wait(); break;
            // -- M3
            case 0x029D: _mwrite(--cpu->sp,cpu->pcl); break;
            case 0x029E: _wait();cpu->wz=0x38;cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x029F: _wait();_fetch(); break;
            
            // ED 00: ed nop (M:1 T:4)
            // -- OVERLAP
            case 0x02A0: _wait();_fetch(); break;
            
            // ED 40: in RY,(c) (M:2 T:8)
            // -- M2 (ioread)
            case 0x02A1: _wait();_ioread(cpu->bc); break;
            case 0x02A2: cpu->dlatch=_gd();cpu->wz=cpu->bc+1; break;
            // -- OVERLAP
            case 0x02A3: _wait();cpu->b=_z80_in(cpu,cpu->dlatch);_fetch(); break;
            
            // ED 41: out (c),RY (M:2 T:8)
            // -- M2 (iowrite)
            case 0x02A4: _iowrite(cpu->bc,cpu->b); break;
            case 0x02A5: _wait();cpu->wz=cpu->bc+1; break;
            // -- OVERLAP
            case 0x02A6: _wait();_fetch(); break;
            
            // ED 42: sbc hl,RP (M:2 T:11)
            // -- M2 (generic)
            case 0x02A7: _z80_sbc16(cpu,cpu->bc); break;
            // -- OVERLAP
            case 0x02A8: _wait();_fetch(); break;
            
            // ED 43: ld (nn),RP (M:5 T:16)
            // -- M2
            case 0x02A9: _wait();_mread(cpu->pc++); break;
            case 0x02AA: cpu->wzl=_gd(); break;
            // -- M3
            case 0x02AB: _wait();_mread(cpu->pc++); break;
            case 0x02AC: cpu->wzh=_gd(); break;
            // -- M4
            case 0x02AD: _mwrite(cpu->wz++,cpu->c); break;
            case 0x02AE: _wait(); break;
            // -- M5
            case 0x02AF: _mwrite(cpu->wz,cpu->b); break;
            case 0x02B0: _wait(); break;
            // -- OVERLAP
            case 0x02B1: _wait();_fetch(); break;
            
            // ED 44: neg (M:1 T:4)
            // -- OVERLAP
            case 0x02B2: _wait();_z80_neg8(cpu);_fetch(); break;
            
            // ED 45: reti/retn (M:3 T:10)
            // -- M2
            case 0x02B3: _wait();_mread(cpu->sp++); break;
            case 0x02B4: cpu->wzl=_gd();pins|=Z80_RETI; break;
            // -- M3
            case 0x02B5: _wait();_mread(cpu->sp++); break;
            case 0x02B6: cpu->wzh=_gd();cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x02B7: _wait();_fetch();cpu->iff1=cpu->iff2; break;
            
            // ED 46: im IMY (M:1 T:4)
            // -- OVERLAP
            case 0x02B8: _wait();cpu->im=0;_fetch(); break;
            
            // ED 47: ld i,a (M:1 T:5)
            // -- OVERLAP
            case 0x02B9: _wait();cpu->i=cpu->a;_fetch(); break;
            
            // ED 48: in RY,(c) (M:2 T:8)
            // -- M2 (ioread)
            case 0x02BA: _wait();_ioread(cpu->bc); break;
            case 0x02BB: cpu->dlatch=_gd();cpu->wz=cpu->bc+1; break;
            // -- OVERLAP
            case 0x02BC: _wait();cpu->c=_z80_in(cpu,cpu->dlatch);_fetch(); break;
            
            // ED 49: out (c),RY (M:2 T:8)
            // -- M2 (iowrite)
            case 0x02BD: _iowrite(cpu->bc,cpu->c); break;
            case 0x02BE: _wait();cpu->wz=cpu->bc+1; break;
            // -- OVERLAP
            case 0x02BF: _wait();_fetch(); break;
            
            // ED 4A: adc hl,RP (M:2 T:11)
            // -- M2 (generic)
            case 0x02C0: _z80_adc16(cpu,cpu->bc); break;
            // -- OVERLAP
            case 0x02C1: _wait();_fetch(); break;
            
            // ED 4B: ld RP,(nn) (M:5 T:16)
            // -- M2
            case 0x02C2: _wait();_mread(cpu->pc++); break;
            case 0x02C3: cpu->wzl=_gd(); break;
            // -- M3
            case 0x02C4: _wait();_mread(cpu->pc++); break;
            case 0x02C5: cpu->wzh=_gd(); break;
            // -- M4
            case 0x02C6: _wait();_mread(cpu->wz++); break;
            case 0x02C7: cpu->c=_gd(); break;
            // -- M5
            case 0x02C8: _wait();_mread(cpu->wz); break;
            case 0x02C9: cpu->b=_gd(); break;
            // -- OVERLAP
            case 0x02CA: _wait();_fetch(); break;
            
            // ED 4E: im IMY (M:1 T:4)
            // -- OVERLAP
            case 0x02CB: _wait();cpu->im=0;_fetch(); break;
            
            // ED 4F: ld r,a (M:1 T:5)
            // -- OVERLAP
            case 0x02CC: _wait();cpu->r=cpu->a;_fetch(); break;
            
            // ED 50: in RY,(c) (M:2 T:8)
            // -- M2 (ioread)
            case 0x02CD: _wait();_ioread(cpu->bc); break;
            case 0x02CE: cpu->dlatch=_gd();cpu->wz=cpu->bc+1; break;
            // -- OVERLAP
            case 0x02CF: _wait();cpu->d=_z80_in(cpu,cpu->dlatch);_fetch(); break;
            
            // ED 51: out (c),RY (M:2 T:8)
            // -- M2 (iowrite)
            case 0x02D0: _iowrite(cpu->bc,cpu->d); break;
            case 0x02D1: _wait();cpu->wz=cpu->bc+1; break;
            // -- OVERLAP
            case 0x02D2: _wait();_fetch(); break;
            
            // ED 52: sbc hl,RP (M:2 T:11)
            // -- M2 (generic)
            case 0x02D3: _z80_sbc16(cpu,cpu->de); break;
            // -- OVERLAP
            case 0x02D4: _wait();_fetch(); break;
            
            // ED 53: ld (nn),RP (M:5 T:16)
            // -- M2
            case 0x02D5: _wait();_mread(cpu->pc++); break;
            case 0x02D6: cpu->wzl=_gd(); break;
            // -- M3
            case 0x02D7: _wait();_mread(cpu->pc++); break;
            case 0x02D8: cpu->wzh=_gd(); break;
            // -- M4
            case 0x02D9: _mwrite(cpu->wz++,cpu->e); break;
            case 0x02DA: _wait(); break;
            // -- M5
            case 0x02DB: _mwrite(cpu->wz,cpu->d); break;
            case 0x02DC: _wait(); break;
            // -- OVERLAP
            case 0x02DD: _wait();_fetch(); break;
            
            // ED 56: im IMY (M:1 T:4)
            // -- OVERLAP
            case 0x02DE: _wait();cpu->im=1;_fetch(); break;
            
            // ED 57: ld a,i (M:1 T:5)
            // -- OVERLAP
            case 0x02DF: _wait();cpu->a=cpu->i;cpu->f=_z80_sziff2_flags(cpu, cpu->i);_fetch(); break;
            
            // ED 58: in RY,(c) (M:2 T:8)
            // -- M2 (ioread)
            case 0x02E0: _wait();_ioread(cpu->bc); break;
            case 0x02E1: cpu->dlatch=_gd();cpu->wz=cpu->bc+1; break;
            // -- OVERLAP
            case 0x02E2: _wait();cpu->e=_z80_in(cpu,cpu->dlatch);_fetch(); break;
            
            // ED 59: out (c),RY (M:2 T:8)
            // -- M2 (iowrite)
            case 0x02E3: _iowrite(cpu->bc,cpu->e); break;
            case 0x02E4: _wait();cpu->wz=cpu->bc+1; break;
            // -- OVERLAP
            case 0x02E5: _wait();_fetch(); break;
            
            // ED 5A: adc hl,RP (M:2 T:11)
            // -- M2 (generic)
            case 0x02E6: _z80_adc16(cpu,cpu->de); break;
            // -- OVERLAP
            case 0x02E7: _wait();_fetch(); break;
            
            // ED 5B: ld RP,(nn) (M:5 T:16)
            // -- M2
            case 0x02E8: _wait();_mread(cpu->pc++); break;
            case 0x02E9: cpu->wzl=_gd(); break;
            // -- M3
            case 0x02EA: _wait();_mread(cpu->pc++); break;
            case 0x02EB: cpu->wzh=_gd(); break;
            // -- M4
            case 0x02EC: _wait();_mread(cpu->wz++); break;
            case 0x02ED: cpu->e=_gd(); break;
            // -- M5
            case 0x02EE: _wait();_mread(cpu->wz); break;
            case 0x02EF: cpu->d=_gd(); break;
            // -- OVERLAP
            case 0x02F0: _wait();_fetch(); break;
            
            // ED 5E: im IMY (M:1 T:4)
            // -- OVERLAP
            case 0x02F1: _wait();cpu->im=2;_fetch(); break;
            
            // ED 5F: ld a,r (M:1 T:5)
            // -- OVERLAP
            case 0x02F2: _wait();cpu->a=cpu->r;cpu->f=_z80_sziff2_flags(cpu, cpu->r);_fetch(); break;
            
            // ED 60: in RY,(c) (M:2 T:8)
            // -- M2 (ioread)
            case 0x02F3: _wait();_ioread(cpu->bc); break;
            case 0x02F4: cpu->dlatch=_gd();cpu->wz=cpu->bc+1; break;
            // -- OVERLAP
            case 0x02F5: _wait();cpu->hlx[cpu->hlx_idx].h=_z80_in(cpu,cpu->dlatch);_fetch(); break;
            
            // ED 61: out (c),RY (M:2 T:8)
            // -- M2 (iowrite)
            case 0x02F6: _iowrite(cpu->bc,cpu->hlx[cpu->hlx_idx].h); break;
            case 0x02F7: _wait();cpu->wz=cpu->bc+1; break;
            // -- OVERLAP
            case 0x02F8: _wait();_fetch(); break;
            
            // ED 62: sbc hl,RP (M:2 T:11)
            // -- M2 (generic)
            case 0x02F9: _z80_sbc16(cpu,cpu->hl); break;
            // -- OVERLAP
            case 0x02FA: _wait();_fetch(); break;
            
            // ED 63: ld (nn),RP (M:5 T:16)
            // -- M2
            case 0x02FB: _wait();_mread(cpu->pc++); break;
            case 0x02FC: cpu->wzl=_gd(); break;
            // -- M3
            case 0x02FD: _wait();_mread(cpu->pc++); break;
            case 0x02FE: cpu->wzh=_gd(); break;
            // -- M4
            case 0x02FF: _mwrite(cpu->wz++,cpu->l); break;
            case 0x0300: _wait(); break;
            // -- M5
            case 0x0301: _mwrite(cpu->wz,cpu->h); break;
            case 0x0302: _wait(); break;
            // -- OVERLAP
            case 0x0303: _wait();_fetch(); break;
            
            // ED 66: im IMY (M:1 T:4)
            // -- OVERLAP
            case 0x0304: _wait();cpu->im=0;_fetch(); break;
            
            // ED 67: rrd (M:4 T:14)
            // -- M2
            case 0x0305: _wait();_mread(cpu->hl); break;
            case 0x0306: cpu->dlatch=_gd(); break;
            // -- M3 (generic)
            case 0x0307: cpu->dlatch=_z80_rrd(cpu,cpu->dlatch); break;
            // -- M4
            case 0x0308: _mwrite(cpu->hl,cpu->dlatch); break;
            case 0x0309: _wait();cpu->wz=cpu->hl+1; break;
            // -- OVERLAP
            case 0x030A: _wait();_fetch(); break;
            
            // ED 68: in RY,(c) (M:2 T:8)
            // -- M2 (ioread)
            case 0x030B: _wait();_ioread(cpu->bc); break;
            case 0x030C: cpu->dlatch=_gd();cpu->wz=cpu->bc+1; break;
            // -- OVERLAP
            case 0x030D: _wait();cpu->hlx[cpu->hlx_idx].l=_z80_in(cpu,cpu->dlatch);_fetch(); break;
            
            // ED 69: out (c),RY (M:2 T:8)
            // -- M2 (iowrite)
            case 0x030E: _iowrite(cpu->bc,cpu->hlx[cpu->hlx_idx].l); break;
            case 0x030F: _wait();cpu->wz=cpu->bc+1; break;
            // -- OVERLAP
            case 0x0310: _wait();_fetch(); break;
            
            // ED 6A: adc hl,RP (M:2 T:11)
            // -- M2 (generic)
            case 0x0311: _z80_adc16(cpu,cpu->hl); break;
            // -- OVERLAP
            case 0x0312: _wait();_fetch(); break;
            
            // ED 6B: ld RP,(nn) (M:5 T:16)
            // -- M2
            case 0x0313: _wait();_mread(cpu->pc++); break;
            case 0x0314: cpu->wzl=_gd(); break;
            // -- M3
            case 0x0315: _wait();_mread(cpu->pc++); break;
            case 0x0316: cpu->wzh=_gd(); break;
            // -- M4
            case 0x0317: _wait();_mread(cpu->wz++); break;
            case 0x0318: cpu->l=_gd(); break;
            // -- M5
            case 0x0319: _wait();_mread(cpu->wz); break;
            case 0x031A: cpu->h=_gd(); break;
            // -- OVERLAP
            case 0x031B: _wait();_fetch(); break;
            
            // ED 6E: im IMY (M:1 T:4)
            // -- OVERLAP
            case 0x031C: _wait();cpu->im=0;_fetch(); break;
            
            // ED 6F: rld (M:4 T:14)
            // -- M2
            case 0x031D: _wait();_mread(cpu->hl); break;
            case 0x031E: cpu->dlatch=_gd(); break;
            // -- M3 (generic)
            case 0x031F: cpu->dlatch=_z80_rld(cpu,cpu->dlatch); break;
            // -- M4
            case 0x0320: _mwrite(cpu->hl,cpu->dlatch); break;
            case 0x0321: _wait();cpu->wz=cpu->hl+1; break;
            // -- OVERLAP
            case 0x0322: _wait();_fetch(); break;
            
            // ED 70: in (c) (M:2 T:8)
            // -- M2 (ioread)
            case 0x0323: _wait();_ioread(cpu->bc); break;
            case 0x0324: cpu->dlatch=_gd();cpu->wz=cpu->bc+1; break;
            // -- OVERLAP
            case 0x0325: _wait();_z80_in(cpu,cpu->dlatch);_fetch(); break;
            
            // ED 71: out (c),0 (M:2 T:8)
            // -- M2 (iowrite)
            case 0x0326: _iowrite(cpu->bc,0); break;
            case 0x0327: _wait();cpu->wz=cpu->bc+1; break;
            // -- OVERLAP
            case 0x0328: _wait();_fetch(); break;
            
            // ED 72: sbc hl,RP (M:2 T:11)
            // -- M2 (generic)
            case 0x0329: _z80_sbc16(cpu,cpu->sp); break;
            // -- OVERLAP
            case 0x032A: _wait();_fetch(); break;
            
            // ED 73: ld (nn),RP (M:5 T:16)
            // -- M2
            case 0x032B: _wait();_mread(cpu->pc++); break;
            case 0x032C: cpu->wzl=_gd(); break;
            // -- M3
            case 0x032D: _wait();_mread(cpu->pc++); break;
            case 0x032E: cpu->wzh=_gd(); break;
            // -- M4
            case 0x032F: _mwrite(cpu->wz++,cpu->spl); break;
            case 0x0330: _wait(); break;
            // -- M5
            case 0x0331: _mwrite(cpu->wz,cpu->sph); break;
            case 0x0332: _wait(); break;
            // -- OVERLAP
            case 0x0333: _wait();_fetch(); break;
            
            // ED 76: im IMY (M:1 T:4)
            // -- OVERLAP
            case 0x0334: _wait();cpu->im=1;_fetch(); break;
            
            // ED 78: in RY,(c) (M:2 T:8)
            // -- M2 (ioread)
            case 0x0335: _wait();_ioread(cpu->bc); break;
            case 0x0336: cpu->dlatch=_gd();cpu->wz=cpu->bc+1; break;
            // -- OVERLAP
            case 0x0337: _wait();cpu->a=_z80_in(cpu,cpu->dlatch);_fetch(); break;
            
            // ED 79: out (c),RY (M:2 T:8)
            // -- M2 (iowrite)
            case 0x0338: _iowrite(cpu->bc,cpu->a); break;
            case 0x0339: _wait();cpu->wz=cpu->bc+1; break;
            // -- OVERLAP
            case 0x033A: _wait();_fetch(); break;
            
            // ED 7A: adc hl,RP (M:2 T:11)
            // -- M2 (generic)
            case 0x033B: _z80_adc16(cpu,cpu->sp); break;
            // -- OVERLAP
            case 0x033C: _wait();_fetch(); break;
            
            // ED 7B: ld RP,(nn) (M:5 T:16)
            // -- M2
            case 0x033D: _wait();_mread(cpu->pc++); break;
            case 0x033E: cpu->wzl=_gd(); break;
            // -- M3
            case 0x033F: _wait();_mread(cpu->pc++); break;
            case 0x0340: cpu->wzh=_gd(); break;
            // -- M4
            case 0x0341: _wait();_mread(cpu->wz++); break;
            case 0x0342: cpu->spl=_gd(); break;
            // -- M5
            case 0x0343: _wait();_mread(cpu->wz); break;
            case 0x0344: cpu->sph=_gd(); break;
            // -- OVERLAP
            case 0x0345: _wait();_fetch(); break;
            
            // ED 7E: im IMY (M:1 T:4)
            // -- OVERLAP
            case 0x0346: _wait();cpu->im=2;_fetch(); break;
            
            // ED A0: ldi (M:4 T:12)
            // -- M2
            case 0x0347: _wait();_mread(cpu->hl++); break;
            case 0x0348: cpu->dlatch=_gd(); break;
            // -- M3
            case 0x0349: _mwrite(cpu->de++,cpu->dlatch); break;
            case 0x034A: _wait(); break;
            // -- M4 (generic)
            case 0x034B: _z80_ldi_ldd(cpu,cpu->dlatch); break;
            // -- OVERLAP
            case 0x034C: _wait();_fetch(); break;
            
            // ED A1: cpi (M:3 T:12)
            // -- M2
            case 0x034D: _wait();_mread(cpu->hl++); break;
            case 0x034E: cpu->dlatch=_gd(); break;
            // -- M3 (generic)
            case 0x034F: cpu->wz++;_z80_cpi_cpd(cpu,cpu->dlatch); break;
            // -- OVERLAP
            case 0x0350: _wait();_fetch(); break;
            
            // ED A2: ini (M:3 T:12)
            // -- M2 (ioread)
            case 0x0351: _wait();_ioread(cpu->bc); break;
            case 0x0352: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;cpu->b--;; break;
            // -- M3
            case 0x0353: _mwrite(cpu->hl++,cpu->dlatch); break;
            case 0x0354: _wait();_z80_ini_ind(cpu,cpu->dlatch,cpu->c+1); break;
            // -- OVERLAP
            case 0x0355: _wait();_fetch(); break;
            
            // ED A3: outi (M:3 T:12)
            // -- M2
            case 0x0356: _wait();_mread(cpu->hl++); break;
            case 0x0357: cpu->dlatch=_gd();cpu->b--; break;
            // -- M3 (iowrite)
            case 0x0358: _iowrite(cpu->bc,cpu->dlatch); break;
            case 0x0359: _wait();cpu->wz=cpu->bc+1;_z80_outi_outd(cpu,cpu->dlatch); break;
            // -- OVERLAP
            case 0x035A: _wait();_fetch(); break;
            
            // ED A8: ldd (M:4 T:12)
            // -- M2
            case 0x035B: _wait();_mread(cpu->hl--); break;
            case 0x035C: cpu->dlatch=_gd(); break;
            // -- M3
            case 0x035D: _mwrite(cpu->de--,cpu->dlatch); break;
            case 0x035E: _wait(); break;
            // -- M4 (generic)
            case 0x035F: _z80_ldi_ldd(cpu,cpu->dlatch); break;
            // -- OVERLAP
            case 0x0360: _wait();_fetch(); break;
            
            // ED A9: cpd (M:3 T:12)
            // -- M2
            case 0x0361: _wait();_mread(cpu->hl--); break;
            case 0x0362: cpu->dlatch=_gd(); break;
            // -- M3 (generic)
            case 0x0363: cpu->wz--;_z80_cpi_cpd(cpu,cpu->dlatch); break;
            // -- OVERLAP
            case 0x0364: _wait();_fetch(); break;
            
            // ED AA: ind (M:3 T:12)
            // -- M2 (ioread)
            case 0x0365: _wait();_ioread(cpu->bc); break;
            case 0x0366: cpu->dlatch=_gd();cpu->wz=cpu->bc-1;cpu->b--;; break;
            // -- M3
            case 0x0367: _mwrite(cpu->hl--,cpu->dlatch); break;
            case 0x0368: _wait();_z80_ini_ind(cpu,cpu->dlatch,cpu->c-1); break;
            // -- OVERLAP
            case 0x0369: _wait();_fetch(); break;
            
            // ED AB: outd (M:3 T:12)
            // -- M2
            case 0x036A: _wait();_mread(cpu->hl--); break;
            case 0x036B: cpu->dlatch=_gd();cpu->b--; break;
            // -- M3 (iowrite)
            case 0x036C: _iowrite(cpu->bc,cpu->dlatch); break;
            case 0x036D: _wait();cpu->wz=cpu->bc-1;_z80_outi_outd(cpu,cpu->dlatch); break;
            // -- OVERLAP
            case 0x036E: _wait();_fetch(); break;
            
            // ED B0: ldir (M:5 T:17)
            // -- M2
            case 0x036F: _wait();_mread(cpu->hl++); break;
            case 0x0370: cpu->dlatch=_gd(); break;
            // -- M3
            case 0x0371: _mwrite(cpu->de++,cpu->dlatch); break;
            case 0x0372: _wait(); break;
            // -- M4 (generic)
            case 0x0373: if(!_z80_ldi_ldd(cpu,cpu->dlatch)){_z80_skip(cpu,1,7,2);}; break;
            // -- M5 (generic)
            case 0x0374: cpu->wz=--cpu->pc;--cpu->pc;; break;
            // -- OVERLAP
            case 0x0375: _wait();_fetch(); break;
            
            // ED B1: cpir (M:4 T:17)
            // -- M2
            case 0x0376: _wait();_mread(cpu->hl++); break;
            case 0x0377: cpu->dlatch=_gd(); break;
            // -- M3 (generic)
            case 0x0378: cpu->wz++;if(!_z80_cpi_cpd(cpu,cpu->dlatch)){_z80_skip(cpu,1,7,2);}; break;
            // -- M4 (generic)
            case 0x0379: cpu->wz=--cpu->pc;--cpu->pc; break;
            // -- OVERLAP
            case 0x037A: _wait();_fetch(); break;
            
            // ED B2: inir (M:4 T:17)
            // -- M2 (ioread)
            case 0x037B: _wait();_ioread(cpu->bc); break;
            case 0x037C: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;cpu->b--;; break;
            // -- M3
            case 0x037D: _mwrite(cpu->hl++,cpu->dlatch); break;
            case 0x037E: _wait();if (!_z80_ini_ind(cpu,cpu->dlatch,cpu->c+1)){_z80_skip(cpu,1,6,1);}; break;
            // -- M4 (generic)
            case 0x037F: cpu->wz=--cpu->pc;--cpu->pc; break;
            // -- OVERLAP
            case 0x0380: _wait();_fetch(); break;
            
            // ED B3: otir (M:4 T:17)
            // -- M2
            case 0x0381: _wait();_mread(cpu->hl++); break;
            case 0x0382: cpu->dlatch=_gd();cpu->b--; break;
            // -- M3 (iowrite)
            case 0x0383: _iowrite(cpu->bc,cpu->dlatch); break;
            case 0x0384: _wait();cpu->wz=cpu->bc+1;if(!_z80_outi_outd(cpu,cpu->dlatch)){_z80_skip(cpu,1,6,1);}; break;
            // -- M4 (generic)
            case 0x0385: cpu->wz=--cpu->pc;--cpu->pc; break;
            // -- OVERLAP
            case 0x0386: _wait();_fetch(); break;
            
            // ED B8: lddr (M:5 T:17)
            // -- M2
            case 0x0387: _wait();_mread(cpu->hl--); break;
            case 0x0388: cpu->dlatch=_gd(); break;
            // -- M3
            case 0x0389: _mwrite(cpu->de--,cpu->dlatch); break;
            case 0x038A: _wait(); break;
            // -- M4 (generic)
            case 0x038B: if(!_z80_ldi_ldd(cpu,cpu->dlatch)){_z80_skip(cpu,1,7,2);}; break;
            // -- M5 (generic)
            case 0x038C: cpu->wz=--cpu->pc;--cpu->pc;; break;
            // -- OVERLAP
            case 0x038D: _wait();_fetch(); break;
            
            // ED B9: cpdr (M:4 T:17)
            // -- M2
            case 0x038E: _wait();_mread(cpu->hl--); break;
            case 0x038F: cpu->dlatch=_gd(); break;
            // -- M3 (generic)
            case 0x0390: cpu->wz--;if(!_z80_cpi_cpd(cpu,cpu->dlatch)){_z80_skip(cpu,1,7,2);}; break;
            // -- M4 (generic)
            case 0x0391: cpu->wz=--cpu->pc;--cpu->pc; break;
            // -- OVERLAP
            case 0x0392: _wait();_fetch(); break;
            
            // ED BA: indr (M:4 T:17)
            // -- M2 (ioread)
            case 0x0393: _wait();_ioread(cpu->bc); break;
            case 0x0394: cpu->dlatch=_gd();cpu->wz=cpu->bc-1;cpu->b--;; break;
            // -- M3
            case 0x0395: _mwrite(cpu->hl--,cpu->dlatch); break;
            case 0x0396: _wait();if (!_z80_ini_ind(cpu,cpu->dlatch,cpu->c-1)){_z80_skip(cpu,1,6,1);}; break;
            // -- M4 (generic)
            case 0x0397: cpu->wz=--cpu->pc;--cpu->pc; break;
            // -- OVERLAP
            case 0x0398: _wait();_fetch(); break;
            
            // ED BB: otdr (M:4 T:17)
            // -- M2
            case 0x0399: _wait();_mread(cpu->hl--); break;
            case 0x039A: cpu->dlatch=_gd();cpu->b--; break;
            // -- M3 (iowrite)
            case 0x039B: _iowrite(cpu->bc,cpu->dlatch); break;
            case 0x039C: _wait();cpu->wz=cpu->bc-1;if(!_z80_outi_outd(cpu,cpu->dlatch)){_z80_skip(cpu,1,6,1);}; break;
            // -- M4 (generic)
            case 0x039D: cpu->wz=--cpu->pc;--cpu->pc; break;
            // -- OVERLAP
            case 0x039E: _wait();_fetch(); break;
            
            // CB 00: cb (M:1 T:4)
            // -- OVERLAP
            case 0x039F: _wait();{uint8_t z=cpu->opcode&7;_z80_cb_action(cpu,z,z);};_fetch(); break;
            
            // CB 00: cbhl (M:3 T:11)
            // -- M2
            case 0x03A0: _wait();_mread(cpu->hl); break;
            case 0x03A1: cpu->dlatch=_gd();if(!_z80_cb_action(cpu,6,6)){_z80_skip(cpu,2,6,3);}; break;
            // -- M3
            case 0x03A2: _mwrite(cpu->hl,cpu->dlatch); break;
            case 0x03A3: _wait(); break;
            // -- OVERLAP
            case 0x03A4: _wait();_fetch(); break;
            
            // CB 00: ddfdcb (M:5 T:18)
            // -- M2 (generic)
            case 0x03A5: _z80_ddfdcb_addr(cpu, pins); break;
            // -- M3
            case 0x03A6: _wait();_mread(cpu->pc++); break;
            case 0x03A7: cpu->dlatch=_gd();_z80_ddfdcb_opcode(cpu,cpu->dlatch); break;
            // -- M4
            case 0x03A8: _wait();_mread(cpu->addr); break;
            case 0x03A9: cpu->dlatch=_gd();if(!_z80_cb_action(cpu,6,cpu->opcode&7)){_z80_skip(cpu,2,6,3);}; break;
            // -- M5
            case 0x03AA: _mwrite(cpu->addr,cpu->dlatch); break;
            case 0x03AB: _wait(); break;
            // -- OVERLAP
            case 0x03AC: _wait();_fetch(); break;
            
            //  00: int_im0 (M:5 T:9)
            // -- M2 (generic)
            case 0x03AD: pins=_z80_int012_step0(cpu,pins); break;
            // -- M3 (generic)
            case 0x03AE: _wait();pins=_z80_int012_step1(cpu,pins); break;
            // -- M4 (generic)
            case 0x03AF: pins=_z80_int0_step2(cpu,pins); break;
            // -- M5 (generic)
            case 0x03B0: pins=_z80_int0_step3(cpu,pins); break;
            // -- OVERLAP
            case 0x03B1: _wait();_fetch(); break;
            
            //  00: int_im1 (M:6 T:16)
            // -- M2 (generic)
            case 0x03B2: pins=_z80_int012_step0(cpu,pins); break;
            // -- M3 (generic)
            case 0x03B3: _wait();pins=_z80_int012_step1(cpu,pins); break;
            // -- M4 (generic)
            case 0x03B4: pins=_z80_refresh(cpu,pins); break;
            // -- M5
            case 0x03B5: _mwrite(--cpu->sp,cpu->pch); break;
            case 0x03B6: _wait(); break;
            // -- M6
            case 0x03B7: _mwrite(--cpu->sp,cpu->pcl); break;
            case 0x03B8: _wait();cpu->wz=cpu->pc=0x0038; break;
            // -- OVERLAP
            case 0x03B9: _wait();_fetch(); break;
            
            //  00: int_im2 (M:9 T:22)
            // -- M2 (generic)
            case 0x03BA: pins=_z80_int012_step0(cpu,pins); break;
            // -- M3 (generic)
            case 0x03BB: _wait();pins=_z80_int012_step1(cpu,pins); break;
            // -- M4 (generic)
            case 0x03BC: cpu->dlatch=_z80_get_db(pins); break;
            // -- M5 (generic)
            case 0x03BD: pins=_z80_refresh(cpu,pins); break;
            // -- M6
            case 0x03BE: _mwrite(--cpu->sp,cpu->pch); break;
            case 0x03BF: _wait(); break;
            // -- M7
            case 0x03C0: _mwrite(--cpu->sp,cpu->pcl); break;
            case 0x03C1: _wait();cpu->wzl=cpu->dlatch;cpu->wzh=cpu->i;; break;
            // -- M8
            case 0x03C2: _wait();_mread(cpu->wz++); break;
            case 0x03C3: cpu->dlatch=_gd(); break;
            // -- M9
            case 0x03C4: _wait();_mread(cpu->wz); break;
            case 0x03C5: cpu->wzh=_gd();cpu->wzl=cpu->dlatch;cpu->pc=cpu->wz; break;
            // -- OVERLAP
            case 0x03C6: _wait();_fetch(); break;
            
            //  00: nmi (M:5 T:14)
            // -- M2 (generic)
            case 0x03C7: pins=_z80_nmi_step0(cpu,pins); break;
            // -- M3 (generic)
            case 0x03C8: pins=_z80_refresh(cpu,pins); break;
            // -- M4
            case 0x03C9: _mwrite(--cpu->sp,cpu->pch); break;
            case 0x03CA: _wait(); break;
            // -- M5
            case 0x03CB: _mwrite(--cpu->sp,cpu->pcl); break;
            case 0x03CC: _wait();cpu->wz=cpu->pc=0x0066; break;
            // -- OVERLAP
            case 0x03CD: _wait();_fetch(); break;

        }
        cpu->op.step += 1;
    }
    // advance the decode pipeline by one tcycle
    cpu->op.pip >>= 1;

    // track NMI 0 => 1 edge and current INT pin state, this will track the
    // relevant interrupt status up to the last instruction cycle and will
    // be checked in the first M1 cycle (during _fetch)
track_int_bits: {
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
