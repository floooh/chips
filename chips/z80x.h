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

static inline void _z80_skip(z80_t* cpu, int steps) {
    cpu->op.step += steps;
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
#define _Z80_OPSTATE_SLOT_CB        (512)
#define _Z80_OPSTATE_SLOT_CBHL      (512+1)
#define _Z80_OPSTATE_SLOT_DDFDCB    (512+2)
#define _Z80_OPSTATE_SLOT_INT_IM0   (512+3)
#define _Z80_OPSTATE_SLOT_INT_IM1   (512+4)
#define _Z80_OPSTATE_SLOT_INT_IM2   (512+5)
#define _Z80_OPSTATE_SLOT_NMI       (512+6)

#define _Z80_OPSTATE_NUM_SPECIAL_OPS (7)

static const z80_opstate_t _z80_opstate_table[2*256 + _Z80_OPSTATE_NUM_SPECIAL_OPS] = {
    {   21, 0 },  //  00: nop (M:1 T:4 steps:1)
    {   22, 0 },  //  01: ld bc,nn (M:3 T:10 steps:7)
    {   29, 0 },  //  02: ld (bc),a (M:2 T:7 steps:4)
    {   33, 0 },  //  03: inc bc (M:2 T:6 steps:3)
    {   36, 0 },  //  04: inc b (M:1 T:4 steps:1)
    {   37, 0 },  //  05: dec b (M:1 T:4 steps:1)
    {   38, _Z80_OPSTATE_FLAGS_IMM8 },  //  06: ld b,n (M:2 T:7 steps:4)
    {   42, 0 },  //  07: rlca (M:1 T:4 steps:1)
    {   43, 0 },  //  08: ex af,af' (M:1 T:4 steps:1)
    {   44, 0 },  //  09: add hl,bc (M:2 T:11 steps:8)
    {   52, 0 },  //  0A: ld a,(bc) (M:2 T:7 steps:4)
    {   56, 0 },  //  0B: dec bc (M:2 T:6 steps:3)
    {   59, 0 },  //  0C: inc c (M:1 T:4 steps:1)
    {   60, 0 },  //  0D: dec c (M:1 T:4 steps:1)
    {   61, _Z80_OPSTATE_FLAGS_IMM8 },  //  0E: ld c,n (M:2 T:7 steps:4)
    {   65, 0 },  //  0F: rrca (M:1 T:4 steps:1)
    {   66, 0 },  //  10: djnz d (M:4 T:13 steps:10)
    {   76, 0 },  //  11: ld de,nn (M:3 T:10 steps:7)
    {   83, 0 },  //  12: ld (de),a (M:2 T:7 steps:4)
    {   87, 0 },  //  13: inc de (M:2 T:6 steps:3)
    {   90, 0 },  //  14: inc d (M:1 T:4 steps:1)
    {   91, 0 },  //  15: dec d (M:1 T:4 steps:1)
    {   92, _Z80_OPSTATE_FLAGS_IMM8 },  //  16: ld d,n (M:2 T:7 steps:4)
    {   96, 0 },  //  17: rla (M:1 T:4 steps:1)
    {   97, 0 },  //  18: jr d (M:3 T:12 steps:9)
    {  106, 0 },  //  19: add hl,de (M:2 T:11 steps:8)
    {  114, 0 },  //  1A: ld a,(de) (M:2 T:7 steps:4)
    {  118, 0 },  //  1B: dec de (M:2 T:6 steps:3)
    {  121, 0 },  //  1C: inc e (M:1 T:4 steps:1)
    {  122, 0 },  //  1D: dec e (M:1 T:4 steps:1)
    {  123, _Z80_OPSTATE_FLAGS_IMM8 },  //  1E: ld e,n (M:2 T:7 steps:4)
    {  127, 0 },  //  1F: rra (M:1 T:4 steps:1)
    {  128, 0 },  //  20: jr nz,d (M:3 T:12 steps:9)
    {  137, 0 },  //  21: ld hl,nn (M:3 T:10 steps:7)
    {  144, 0 },  //  22: ld (nn),hl (M:5 T:16 steps:13)
    {  157, 0 },  //  23: inc hl (M:2 T:6 steps:3)
    {  160, 0 },  //  24: inc h (M:1 T:4 steps:1)
    {  161, 0 },  //  25: dec h (M:1 T:4 steps:1)
    {  162, _Z80_OPSTATE_FLAGS_IMM8 },  //  26: ld h,n (M:2 T:7 steps:4)
    {  166, 0 },  //  27: daa (M:1 T:4 steps:1)
    {  167, 0 },  //  28: jr z,d (M:3 T:12 steps:9)
    {  176, 0 },  //  29: add hl,hl (M:2 T:11 steps:8)
    {  184, 0 },  //  2A: ld hl,(nn) (M:5 T:16 steps:13)
    {  197, 0 },  //  2B: dec hl (M:2 T:6 steps:3)
    {  200, 0 },  //  2C: inc l (M:1 T:4 steps:1)
    {  201, 0 },  //  2D: dec l (M:1 T:4 steps:1)
    {  202, _Z80_OPSTATE_FLAGS_IMM8 },  //  2E: ld l,n (M:2 T:7 steps:4)
    {  206, 0 },  //  2F: cpl (M:1 T:4 steps:1)
    {  207, 0 },  //  30: jr nc,d (M:3 T:12 steps:9)
    {  216, 0 },  //  31: ld sp,nn (M:3 T:10 steps:7)
    {  223, 0 },  //  32: ld (nn),a (M:4 T:13 steps:10)
    {  233, 0 },  //  33: inc sp (M:2 T:6 steps:3)
    {  236, _Z80_OPSTATE_FLAGS_INDIRECT },  //  34: inc (hl) (M:3 T:11 steps:8)
    {  244, _Z80_OPSTATE_FLAGS_INDIRECT },  //  35: dec (hl) (M:3 T:11 steps:8)
    {  252, _Z80_OPSTATE_FLAGS_INDIRECT|_Z80_OPSTATE_FLAGS_IMM8 },  //  36: ld (hl),n (M:3 T:10 steps:7)
    {  259, 0 },  //  37: scf (M:1 T:4 steps:1)
    {  260, 0 },  //  38: jr c,d (M:3 T:12 steps:9)
    {  269, 0 },  //  39: add hl,sp (M:2 T:11 steps:8)
    {  277, 0 },  //  3A: ld a,(nn) (M:4 T:13 steps:10)
    {  287, 0 },  //  3B: dec sp (M:2 T:6 steps:3)
    {  290, 0 },  //  3C: inc a (M:1 T:4 steps:1)
    {  291, 0 },  //  3D: dec a (M:1 T:4 steps:1)
    {  292, _Z80_OPSTATE_FLAGS_IMM8 },  //  3E: ld a,n (M:2 T:7 steps:4)
    {  296, 0 },  //  3F: ccf (M:1 T:4 steps:1)
    {  297, 0 },  //  40: ld b,b (M:1 T:4 steps:1)
    {  298, 0 },  //  41: ld b,c (M:1 T:4 steps:1)
    {  299, 0 },  //  42: ld b,d (M:1 T:4 steps:1)
    {  300, 0 },  //  43: ld b,e (M:1 T:4 steps:1)
    {  301, 0 },  //  44: ld b,h (M:1 T:4 steps:1)
    {  302, 0 },  //  45: ld b,l (M:1 T:4 steps:1)
    {  303, _Z80_OPSTATE_FLAGS_INDIRECT },  //  46: ld b,(hl) (M:2 T:7 steps:4)
    {  307, 0 },  //  47: ld b,a (M:1 T:4 steps:1)
    {  308, 0 },  //  48: ld c,b (M:1 T:4 steps:1)
    {  309, 0 },  //  49: ld c,c (M:1 T:4 steps:1)
    {  310, 0 },  //  4A: ld c,d (M:1 T:4 steps:1)
    {  311, 0 },  //  4B: ld c,e (M:1 T:4 steps:1)
    {  312, 0 },  //  4C: ld c,h (M:1 T:4 steps:1)
    {  313, 0 },  //  4D: ld c,l (M:1 T:4 steps:1)
    {  314, _Z80_OPSTATE_FLAGS_INDIRECT },  //  4E: ld c,(hl) (M:2 T:7 steps:4)
    {  318, 0 },  //  4F: ld c,a (M:1 T:4 steps:1)
    {  319, 0 },  //  50: ld d,b (M:1 T:4 steps:1)
    {  320, 0 },  //  51: ld d,c (M:1 T:4 steps:1)
    {  321, 0 },  //  52: ld d,d (M:1 T:4 steps:1)
    {  322, 0 },  //  53: ld d,e (M:1 T:4 steps:1)
    {  323, 0 },  //  54: ld d,h (M:1 T:4 steps:1)
    {  324, 0 },  //  55: ld d,l (M:1 T:4 steps:1)
    {  325, _Z80_OPSTATE_FLAGS_INDIRECT },  //  56: ld d,(hl) (M:2 T:7 steps:4)
    {  329, 0 },  //  57: ld d,a (M:1 T:4 steps:1)
    {  330, 0 },  //  58: ld e,b (M:1 T:4 steps:1)
    {  331, 0 },  //  59: ld e,c (M:1 T:4 steps:1)
    {  332, 0 },  //  5A: ld e,d (M:1 T:4 steps:1)
    {  333, 0 },  //  5B: ld e,e (M:1 T:4 steps:1)
    {  334, 0 },  //  5C: ld e,h (M:1 T:4 steps:1)
    {  335, 0 },  //  5D: ld e,l (M:1 T:4 steps:1)
    {  336, _Z80_OPSTATE_FLAGS_INDIRECT },  //  5E: ld e,(hl) (M:2 T:7 steps:4)
    {  340, 0 },  //  5F: ld e,a (M:1 T:4 steps:1)
    {  341, 0 },  //  60: ld h,b (M:1 T:4 steps:1)
    {  342, 0 },  //  61: ld h,c (M:1 T:4 steps:1)
    {  343, 0 },  //  62: ld h,d (M:1 T:4 steps:1)
    {  344, 0 },  //  63: ld h,e (M:1 T:4 steps:1)
    {  345, 0 },  //  64: ld h,h (M:1 T:4 steps:1)
    {  346, 0 },  //  65: ld h,l (M:1 T:4 steps:1)
    {  347, _Z80_OPSTATE_FLAGS_INDIRECT },  //  66: ld h,(hl) (M:2 T:7 steps:4)
    {  351, 0 },  //  67: ld h,a (M:1 T:4 steps:1)
    {  352, 0 },  //  68: ld l,b (M:1 T:4 steps:1)
    {  353, 0 },  //  69: ld l,c (M:1 T:4 steps:1)
    {  354, 0 },  //  6A: ld l,d (M:1 T:4 steps:1)
    {  355, 0 },  //  6B: ld l,e (M:1 T:4 steps:1)
    {  356, 0 },  //  6C: ld l,h (M:1 T:4 steps:1)
    {  357, 0 },  //  6D: ld l,l (M:1 T:4 steps:1)
    {  358, _Z80_OPSTATE_FLAGS_INDIRECT },  //  6E: ld l,(hl) (M:2 T:7 steps:4)
    {  362, 0 },  //  6F: ld l,a (M:1 T:4 steps:1)
    {  363, _Z80_OPSTATE_FLAGS_INDIRECT },  //  70: ld (hl),b (M:2 T:7 steps:4)
    {  367, _Z80_OPSTATE_FLAGS_INDIRECT },  //  71: ld (hl),c (M:2 T:7 steps:4)
    {  371, _Z80_OPSTATE_FLAGS_INDIRECT },  //  72: ld (hl),d (M:2 T:7 steps:4)
    {  375, _Z80_OPSTATE_FLAGS_INDIRECT },  //  73: ld (hl),e (M:2 T:7 steps:4)
    {  379, _Z80_OPSTATE_FLAGS_INDIRECT },  //  74: ld (hl),h (M:2 T:7 steps:4)
    {  383, _Z80_OPSTATE_FLAGS_INDIRECT },  //  75: ld (hl),l (M:2 T:7 steps:4)
    {  387, 0 },  //  76: halt (M:1 T:4 steps:1)
    {  388, _Z80_OPSTATE_FLAGS_INDIRECT },  //  77: ld (hl),a (M:2 T:7 steps:4)
    {  392, 0 },  //  78: ld a,b (M:1 T:4 steps:1)
    {  393, 0 },  //  79: ld a,c (M:1 T:4 steps:1)
    {  394, 0 },  //  7A: ld a,d (M:1 T:4 steps:1)
    {  395, 0 },  //  7B: ld a,e (M:1 T:4 steps:1)
    {  396, 0 },  //  7C: ld a,h (M:1 T:4 steps:1)
    {  397, 0 },  //  7D: ld a,l (M:1 T:4 steps:1)
    {  398, _Z80_OPSTATE_FLAGS_INDIRECT },  //  7E: ld a,(hl) (M:2 T:7 steps:4)
    {  402, 0 },  //  7F: ld a,a (M:1 T:4 steps:1)
    {  403, 0 },  //  80: add b (M:1 T:4 steps:1)
    {  404, 0 },  //  81: add c (M:1 T:4 steps:1)
    {  405, 0 },  //  82: add d (M:1 T:4 steps:1)
    {  406, 0 },  //  83: add e (M:1 T:4 steps:1)
    {  407, 0 },  //  84: add h (M:1 T:4 steps:1)
    {  408, 0 },  //  85: add l (M:1 T:4 steps:1)
    {  409, _Z80_OPSTATE_FLAGS_INDIRECT },  //  86: add (hl) (M:2 T:7 steps:4)
    {  413, 0 },  //  87: add a (M:1 T:4 steps:1)
    {  414, 0 },  //  88: adc b (M:1 T:4 steps:1)
    {  415, 0 },  //  89: adc c (M:1 T:4 steps:1)
    {  416, 0 },  //  8A: adc d (M:1 T:4 steps:1)
    {  417, 0 },  //  8B: adc e (M:1 T:4 steps:1)
    {  418, 0 },  //  8C: adc h (M:1 T:4 steps:1)
    {  419, 0 },  //  8D: adc l (M:1 T:4 steps:1)
    {  420, _Z80_OPSTATE_FLAGS_INDIRECT },  //  8E: adc (hl) (M:2 T:7 steps:4)
    {  424, 0 },  //  8F: adc a (M:1 T:4 steps:1)
    {  425, 0 },  //  90: sub b (M:1 T:4 steps:1)
    {  426, 0 },  //  91: sub c (M:1 T:4 steps:1)
    {  427, 0 },  //  92: sub d (M:1 T:4 steps:1)
    {  428, 0 },  //  93: sub e (M:1 T:4 steps:1)
    {  429, 0 },  //  94: sub h (M:1 T:4 steps:1)
    {  430, 0 },  //  95: sub l (M:1 T:4 steps:1)
    {  431, _Z80_OPSTATE_FLAGS_INDIRECT },  //  96: sub (hl) (M:2 T:7 steps:4)
    {  435, 0 },  //  97: sub a (M:1 T:4 steps:1)
    {  436, 0 },  //  98: sbc b (M:1 T:4 steps:1)
    {  437, 0 },  //  99: sbc c (M:1 T:4 steps:1)
    {  438, 0 },  //  9A: sbc d (M:1 T:4 steps:1)
    {  439, 0 },  //  9B: sbc e (M:1 T:4 steps:1)
    {  440, 0 },  //  9C: sbc h (M:1 T:4 steps:1)
    {  441, 0 },  //  9D: sbc l (M:1 T:4 steps:1)
    {  442, _Z80_OPSTATE_FLAGS_INDIRECT },  //  9E: sbc (hl) (M:2 T:7 steps:4)
    {  446, 0 },  //  9F: sbc a (M:1 T:4 steps:1)
    {  447, 0 },  //  A0: and b (M:1 T:4 steps:1)
    {  448, 0 },  //  A1: and c (M:1 T:4 steps:1)
    {  449, 0 },  //  A2: and d (M:1 T:4 steps:1)
    {  450, 0 },  //  A3: and e (M:1 T:4 steps:1)
    {  451, 0 },  //  A4: and h (M:1 T:4 steps:1)
    {  452, 0 },  //  A5: and l (M:1 T:4 steps:1)
    {  453, _Z80_OPSTATE_FLAGS_INDIRECT },  //  A6: and (hl) (M:2 T:7 steps:4)
    {  457, 0 },  //  A7: and a (M:1 T:4 steps:1)
    {  458, 0 },  //  A8: xor b (M:1 T:4 steps:1)
    {  459, 0 },  //  A9: xor c (M:1 T:4 steps:1)
    {  460, 0 },  //  AA: xor d (M:1 T:4 steps:1)
    {  461, 0 },  //  AB: xor e (M:1 T:4 steps:1)
    {  462, 0 },  //  AC: xor h (M:1 T:4 steps:1)
    {  463, 0 },  //  AD: xor l (M:1 T:4 steps:1)
    {  464, _Z80_OPSTATE_FLAGS_INDIRECT },  //  AE: xor (hl) (M:2 T:7 steps:4)
    {  468, 0 },  //  AF: xor a (M:1 T:4 steps:1)
    {  469, 0 },  //  B0: or b (M:1 T:4 steps:1)
    {  470, 0 },  //  B1: or c (M:1 T:4 steps:1)
    {  471, 0 },  //  B2: or d (M:1 T:4 steps:1)
    {  472, 0 },  //  B3: or e (M:1 T:4 steps:1)
    {  473, 0 },  //  B4: or h (M:1 T:4 steps:1)
    {  474, 0 },  //  B5: or l (M:1 T:4 steps:1)
    {  475, _Z80_OPSTATE_FLAGS_INDIRECT },  //  B6: or (hl) (M:2 T:7 steps:4)
    {  479, 0 },  //  B7: or a (M:1 T:4 steps:1)
    {  480, 0 },  //  B8: cp b (M:1 T:4 steps:1)
    {  481, 0 },  //  B9: cp c (M:1 T:4 steps:1)
    {  482, 0 },  //  BA: cp d (M:1 T:4 steps:1)
    {  483, 0 },  //  BB: cp e (M:1 T:4 steps:1)
    {  484, 0 },  //  BC: cp h (M:1 T:4 steps:1)
    {  485, 0 },  //  BD: cp l (M:1 T:4 steps:1)
    {  486, _Z80_OPSTATE_FLAGS_INDIRECT },  //  BE: cp (hl) (M:2 T:7 steps:4)
    {  490, 0 },  //  BF: cp a (M:1 T:4 steps:1)
    {  491, 0 },  //  C0: ret nz (M:4 T:11 steps:8)
    {  499, 0 },  //  C1: pop bc (M:3 T:10 steps:7)
    {  506, 0 },  //  C2: jp nz,nn (M:3 T:10 steps:7)
    {  513, 0 },  //  C3: jp nn (M:3 T:10 steps:7)
    {  520, 0 },  //  C4: call nz,nn (M:6 T:17 steps:14)
    {  534, 0 },  //  C5: push bc (M:4 T:11 steps:8)
    {  542, _Z80_OPSTATE_FLAGS_IMM8 },  //  C6: add n (M:2 T:7 steps:4)
    {  546, 0 },  //  C7: rst 0h (M:4 T:11 steps:8)
    {  554, 0 },  //  C8: ret z (M:4 T:11 steps:8)
    {  562, 0 },  //  C9: ret (M:3 T:10 steps:7)
    {  569, 0 },  //  CA: jp z,nn (M:3 T:10 steps:7)
    {  576, 0 },  //  CB: cb prefix (M:1 T:4 steps:1)
    {  577, 0 },  //  CC: call z,nn (M:6 T:17 steps:14)
    {  591, 0 },  //  CD: call nn (M:5 T:17 steps:14)
    {  605, _Z80_OPSTATE_FLAGS_IMM8 },  //  CE: adc n (M:2 T:7 steps:4)
    {  609, 0 },  //  CF: rst 8h (M:4 T:11 steps:8)
    {  617, 0 },  //  D0: ret nc (M:4 T:11 steps:8)
    {  625, 0 },  //  D1: pop de (M:3 T:10 steps:7)
    {  632, 0 },  //  D2: jp nc,nn (M:3 T:10 steps:7)
    {  639, 0 },  //  D3: out (n),a (M:3 T:11 steps:8)
    {  647, 0 },  //  D4: call nc,nn (M:6 T:17 steps:14)
    {  661, 0 },  //  D5: push de (M:4 T:11 steps:8)
    {  669, _Z80_OPSTATE_FLAGS_IMM8 },  //  D6: sub n (M:2 T:7 steps:4)
    {  673, 0 },  //  D7: rst 10h (M:4 T:11 steps:8)
    {  681, 0 },  //  D8: ret c (M:4 T:11 steps:8)
    {  689, 0 },  //  D9: exx (M:1 T:4 steps:1)
    {  690, 0 },  //  DA: jp c,nn (M:3 T:10 steps:7)
    {  697, 0 },  //  DB: in a,(n) (M:3 T:11 steps:8)
    {  705, 0 },  //  DC: call c,nn (M:6 T:17 steps:14)
    {  719, 0 },  //  DD: dd prefix (M:1 T:4 steps:1)
    {  720, _Z80_OPSTATE_FLAGS_IMM8 },  //  DE: sbc n (M:2 T:7 steps:4)
    {  724, 0 },  //  DF: rst 18h (M:4 T:11 steps:8)
    {  732, 0 },  //  E0: ret po (M:4 T:11 steps:8)
    {  740, 0 },  //  E1: pop hl (M:3 T:10 steps:7)
    {  747, 0 },  //  E2: jp po,nn (M:3 T:10 steps:7)
    {  754, 0 },  //  E3: ex (sp),hl (M:5 T:19 steps:16)
    {  770, 0 },  //  E4: call po,nn (M:6 T:17 steps:14)
    {  784, 0 },  //  E5: push hl (M:4 T:11 steps:8)
    {  792, _Z80_OPSTATE_FLAGS_IMM8 },  //  E6: and n (M:2 T:7 steps:4)
    {  796, 0 },  //  E7: rst 20h (M:4 T:11 steps:8)
    {  804, 0 },  //  E8: ret pe (M:4 T:11 steps:8)
    {  812, 0 },  //  E9: jp hl (M:1 T:4 steps:1)
    {  813, 0 },  //  EA: jp pe,nn (M:3 T:10 steps:7)
    {  820, 0 },  //  EB: ex de,hl (M:1 T:4 steps:1)
    {  821, 0 },  //  EC: call pe,nn (M:6 T:17 steps:14)
    {  835, 0 },  //  ED: ed prefix (M:1 T:4 steps:1)
    {  836, _Z80_OPSTATE_FLAGS_IMM8 },  //  EE: xor n (M:2 T:7 steps:4)
    {  840, 0 },  //  EF: rst 28h (M:4 T:11 steps:8)
    {  848, 0 },  //  F0: ret p (M:4 T:11 steps:8)
    {  856, 0 },  //  F1: pop af (M:3 T:10 steps:7)
    {  863, 0 },  //  F2: jp p,nn (M:3 T:10 steps:7)
    {  870, 0 },  //  F3: di (M:1 T:4 steps:1)
    {  871, 0 },  //  F4: call p,nn (M:6 T:17 steps:14)
    {  885, 0 },  //  F5: push af (M:4 T:11 steps:8)
    {  893, _Z80_OPSTATE_FLAGS_IMM8 },  //  F6: or n (M:2 T:7 steps:4)
    {  897, 0 },  //  F7: rst 30h (M:4 T:11 steps:8)
    {  905, 0 },  //  F8: ret m (M:4 T:11 steps:8)
    {  913, 0 },  //  F9: ld sp,hl (M:2 T:6 steps:3)
    {  916, 0 },  //  FA: jp m,nn (M:3 T:10 steps:7)
    {  923, 0 },  //  FB: ei (M:1 T:4 steps:1)
    {  924, 0 },  //  FC: call m,nn (M:6 T:17 steps:14)
    {  938, 0 },  //  FD: fd prefix (M:1 T:4 steps:1)
    {  939, _Z80_OPSTATE_FLAGS_IMM8 },  //  FE: cp n (M:2 T:7 steps:4)
    {  943, 0 },  //  FF: rst 38h (M:4 T:11 steps:8)
    {  951, 0 },  // ED 00: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 01: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 02: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 03: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 04: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 05: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 06: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 07: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 08: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 09: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 0A: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 0B: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 0C: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 0D: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 0E: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 0F: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 10: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 11: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 12: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 13: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 14: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 15: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 16: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 17: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 18: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 19: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 1A: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 1B: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 1C: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 1D: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 1E: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 1F: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 20: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 21: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 22: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 23: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 24: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 25: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 26: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 27: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 28: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 29: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 2A: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 2B: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 2C: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 2D: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 2E: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 2F: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 30: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 31: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 32: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 33: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 34: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 35: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 36: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 37: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 38: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 39: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 3A: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 3B: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 3C: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 3D: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 3E: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 3F: ed nop (M:1 T:4 steps:1)
    {  952, 0 },  // ED 40: in b,(c) (M:2 T:8 steps:5)
    {  957, 0 },  // ED 41: out (c),b (M:2 T:8 steps:5)
    {  962, 0 },  // ED 42: sbc hl,bc (M:2 T:11 steps:8)
    {  970, 0 },  // ED 43: ld (nn),bc (M:5 T:16 steps:13)
    {  983, 0 },  // ED 44: neg (M:1 T:4 steps:1)
    {  984, 0 },  // ED 45: reti/retn (M:3 T:10 steps:7)
    {  991, 0 },  // ED 46: im 0 (M:1 T:4 steps:1)
    {  992, 0 },  // ED 47: ld i,a (M:2 T:5 steps:2)
    {  994, 0 },  // ED 48: in c,(c) (M:2 T:8 steps:5)
    {  999, 0 },  // ED 49: out (c),c (M:2 T:8 steps:5)
    { 1004, 0 },  // ED 4A: adc hl,bc (M:2 T:11 steps:8)
    { 1012, 0 },  // ED 4B: ld bc,(nn) (M:5 T:16 steps:13)
    {  983, 0 },  // ED 4C: neg (M:1 T:4 steps:1)
    {  984, 0 },  // ED 4D: reti/retn (M:3 T:10 steps:7)
    { 1025, 0 },  // ED 4E: im 0 (M:1 T:4 steps:1)
    { 1026, 0 },  // ED 4F: ld r,a (M:2 T:5 steps:2)
    { 1028, 0 },  // ED 50: in d,(c) (M:2 T:8 steps:5)
    { 1033, 0 },  // ED 51: out (c),d (M:2 T:8 steps:5)
    { 1038, 0 },  // ED 52: sbc hl,de (M:2 T:11 steps:8)
    { 1046, 0 },  // ED 53: ld (nn),de (M:5 T:16 steps:13)
    {  983, 0 },  // ED 54: neg (M:1 T:4 steps:1)
    {  984, 0 },  // ED 55: reti/retn (M:3 T:10 steps:7)
    { 1059, 0 },  // ED 56: im 1 (M:1 T:4 steps:1)
    { 1060, 0 },  // ED 57: ld a,i (M:2 T:5 steps:2)
    { 1062, 0 },  // ED 58: in e,(c) (M:2 T:8 steps:5)
    { 1067, 0 },  // ED 59: out (c),e (M:2 T:8 steps:5)
    { 1072, 0 },  // ED 5A: adc hl,de (M:2 T:11 steps:8)
    { 1080, 0 },  // ED 5B: ld de,(nn) (M:5 T:16 steps:13)
    {  983, 0 },  // ED 5C: neg (M:1 T:4 steps:1)
    {  984, 0 },  // ED 5D: reti/retn (M:3 T:10 steps:7)
    { 1093, 0 },  // ED 5E: im 2 (M:1 T:4 steps:1)
    { 1094, 0 },  // ED 5F: ld a,r (M:2 T:5 steps:2)
    { 1096, 0 },  // ED 60: in h,(c) (M:2 T:8 steps:5)
    { 1101, 0 },  // ED 61: out (c),h (M:2 T:8 steps:5)
    { 1106, 0 },  // ED 62: sbc hl,hl (M:2 T:11 steps:8)
    { 1114, 0 },  // ED 63: ld (nn),hl (M:5 T:16 steps:13)
    {  983, 0 },  // ED 64: neg (M:1 T:4 steps:1)
    {  984, 0 },  // ED 65: reti/retn (M:3 T:10 steps:7)
    { 1127, 0 },  // ED 66: im 0 (M:1 T:4 steps:1)
    { 1128, 0 },  // ED 67: rrd (M:4 T:14 steps:11)
    { 1139, 0 },  // ED 68: in l,(c) (M:2 T:8 steps:5)
    { 1144, 0 },  // ED 69: out (c),l (M:2 T:8 steps:5)
    { 1149, 0 },  // ED 6A: adc hl,hl (M:2 T:11 steps:8)
    { 1157, 0 },  // ED 6B: ld hl,(nn) (M:5 T:16 steps:13)
    {  983, 0 },  // ED 6C: neg (M:1 T:4 steps:1)
    {  984, 0 },  // ED 6D: reti/retn (M:3 T:10 steps:7)
    { 1170, 0 },  // ED 6E: im 0 (M:1 T:4 steps:1)
    { 1171, 0 },  // ED 6F: rld (M:4 T:14 steps:11)
    { 1182, 0 },  // ED 70: in (c) (M:2 T:8 steps:5)
    { 1187, 0 },  // ED 71: out (c),0 (M:2 T:8 steps:5)
    { 1192, 0 },  // ED 72: sbc hl,sp (M:2 T:11 steps:8)
    { 1200, 0 },  // ED 73: ld (nn),sp (M:5 T:16 steps:13)
    {  983, 0 },  // ED 74: neg (M:1 T:4 steps:1)
    {  984, 0 },  // ED 75: reti/retn (M:3 T:10 steps:7)
    { 1213, 0 },  // ED 76: im 1 (M:1 T:4 steps:1)
    {  951, 0 },  // ED 77: ed nop (M:1 T:4 steps:1)
    { 1214, 0 },  // ED 78: in a,(c) (M:2 T:8 steps:5)
    { 1219, 0 },  // ED 79: out (c),a (M:2 T:8 steps:5)
    { 1224, 0 },  // ED 7A: adc hl,sp (M:2 T:11 steps:8)
    { 1232, 0 },  // ED 7B: ld sp,(nn) (M:5 T:16 steps:13)
    {  983, 0 },  // ED 7C: neg (M:1 T:4 steps:1)
    {  984, 0 },  // ED 7D: reti/retn (M:3 T:10 steps:7)
    { 1245, 0 },  // ED 7E: im 2 (M:1 T:4 steps:1)
    {  951, 0 },  // ED 7F: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 80: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 81: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 82: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 83: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 84: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 85: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 86: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 87: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 88: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 89: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 8A: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 8B: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 8C: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 8D: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 8E: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 8F: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 90: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 91: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 92: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 93: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 94: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 95: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 96: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 97: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 98: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 99: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 9A: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 9B: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 9C: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 9D: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 9E: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED 9F: ed nop (M:1 T:4 steps:1)
    { 1246, 0 },  // ED A0: ldi (M:4 T:12 steps:9)
    { 1255, 0 },  // ED A1: cpi (M:3 T:12 steps:9)
    { 1264, 0 },  // ED A2: ini (M:4 T:12 steps:9)
    { 1273, 0 },  // ED A3: outi (M:4 T:12 steps:9)
    {  951, 0 },  // ED A4: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED A5: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED A6: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED A7: ed nop (M:1 T:4 steps:1)
    { 1282, 0 },  // ED A8: ldd (M:4 T:12 steps:9)
    { 1291, 0 },  // ED A9: cpd (M:3 T:12 steps:9)
    { 1300, 0 },  // ED AA: ind (M:4 T:12 steps:9)
    { 1309, 0 },  // ED AB: outd (M:4 T:12 steps:9)
    {  951, 0 },  // ED AC: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED AD: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED AE: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED AF: ed nop (M:1 T:4 steps:1)
    { 1318, 0 },  // ED B0: ldir (M:5 T:17 steps:14)
    { 1332, 0 },  // ED B1: cpir (M:4 T:17 steps:14)
    { 1346, 0 },  // ED B2: inir (M:5 T:17 steps:14)
    { 1360, 0 },  // ED B3: otir (M:5 T:17 steps:14)
    {  951, 0 },  // ED B4: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED B5: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED B6: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED B7: ed nop (M:1 T:4 steps:1)
    { 1374, 0 },  // ED B8: lddr (M:5 T:17 steps:14)
    { 1388, 0 },  // ED B9: cpdr (M:4 T:17 steps:14)
    { 1402, 0 },  // ED BA: indr (M:5 T:17 steps:14)
    { 1416, 0 },  // ED BB: otdr (M:5 T:17 steps:14)
    {  951, 0 },  // ED BC: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED BD: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED BE: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED BF: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED C0: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED C1: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED C2: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED C3: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED C4: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED C5: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED C6: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED C7: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED C8: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED C9: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED CA: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED CB: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED CC: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED CD: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED CE: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED CF: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED D0: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED D1: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED D2: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED D3: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED D4: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED D5: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED D6: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED D7: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED D8: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED D9: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED DA: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED DB: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED DC: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED DD: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED DE: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED DF: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED E0: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED E1: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED E2: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED E3: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED E4: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED E5: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED E6: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED E7: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED E8: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED E9: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED EA: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED EB: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED EC: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED ED: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED EE: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED EF: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED F0: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED F1: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED F2: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED F3: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED F4: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED F5: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED F6: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED F7: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED F8: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED F9: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED FA: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED FB: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED FC: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED FD: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED FE: ed nop (M:1 T:4 steps:1)
    {  951, 0 },  // ED FF: ed nop (M:1 T:4 steps:1)
    { 1430, 0 },  // CB 00: cb (M:1 T:4 steps:1)
    { 1431, 0 },  // CB 01: cbhl (M:3 T:11 steps:8)
    { 1439, 0 },  // CB 02: ddfdcb (M:6 T:18 steps:15)
    { 1454, 0 },  //  03: int_im0 (M:5 T:9 steps:6)
    { 1460, 0 },  //  04: int_im1 (M:7 T:16 steps:13)
    { 1473, 0 },  //  05: int_im2 (M:9 T:22 steps:19)
    { 1492, 0 },  //  06: nmi (M:5 T:14 steps:11)

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
        // NOTE: PC is *not* incremented!
        pins = _z80_set_ab_x(pins, cpu->pc, Z80_M1|Z80_MREQ|Z80_RD);
    }
    else if ((cpu->int_bits & Z80_INT) && cpu->iff1) {
        // maskable interrupts start with a special M1 machine cycle which
        // doesn't fetch the next opcode, but instead activate the
        // pins M1|IOQR to request a special byte which is handled differently
        // depending on interrupt mode
        cpu->op = _z80_opstate_table[_Z80_OPSTATE_SLOT_INT_IM0 + cpu->im];
        // NOTE: PC is not incremented, and no pins are activated here
    }
    else {
        // no interrupt, continue with next opcode
        cpu->op.step = 0xFFFF;
        pins = _z80_set_ab_x(pins, cpu->pc++, Z80_M1|Z80_MREQ|Z80_RD);
    }
    cpu->prefix_state = 0;
    cpu->int_bits = 0;
    return pins;
}

static inline uint64_t _z80_fetch_prefix(z80_t* cpu, uint64_t pins, uint8_t prefix) {
    // reset the decoder to continue at step 0
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
                // set pins for a regular read machine cycle to read d-offset
                pins = _z80_set_ab_x(pins, cpu->pc++, Z80_MREQ|Z80_RD);
            }
            else {
                // this is a regular CB-prefixed instruction, continue
                // execution on a special fetch machine cycle which doesn't
                // handle DD/FD prefix and then branches either to the
                // special CB or CBHL decoder block
                cpu->op.step = 19 - 1; // => step 19
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
        // M1/T2: shared fetch machine cycle for all opcodes
        case 0: _wait(); cpu->opcode = _gd(); break;
        // M1/T3: refresh cycle
        case 1: pins = _z80_refresh(cpu, pins); break;
        // M1/T4: branch to instruction 'payload'
        case 2: {
            cpu->op = _z80_opstate_table[cpu->opcode + cpu->prefix_offset];
            // if this is a (HL)/(IX+d)/(IY+d) instruction, insert
            // d-load cycle if needed and compute effective address
            if (cpu->op.flags & _Z80_OPSTATE_FLAGS_INDIRECT) {
                cpu->addr = cpu->hlx[cpu->hlx_idx].hl;
                if (cpu->hlx_idx != _Z80_MAP_HL) {
                    if (cpu->op.flags & _Z80_OPSTATE_FLAGS_IMM8) {
                        // special case: if this is indirect+immediate (which is
                        // just LD (HL),n, then the immediate-load is 'hidden' within
                        // the 8-tcycle d-offset computation)
                        cpu->op.step = 10;  // continue at step 11
                    }
                    else {
                        // regular (IX+d)/(IY+d) instruction
                        cpu->op.step = 2;   // continue at step 3
                    }
                }
            }
        } break;
        //=== optional d-loading cycle for (IX+d), (IY+d)
        //--- mread
        case 3: break;
        case 4: _wait();_mread(cpu->pc++); break;
        case 5: cpu->addr += (int8_t)_gd(); cpu->wz = cpu->addr; break;
        //--- filler ticks
        case 6: break;
        case 7: break;
        case 8: break;
        case 9: break;
        case 10: {
            // branch to original instruction
            cpu->op = _z80_opstate_table[cpu->opcode];
        } break;
        //=== special case d-loading cycle for (IX+d),n where the immediate load
        //    is hidden in the d-cycle load
        //--- mread for d offset
        case 11: break;
        case 12: _wait();_mread(cpu->pc++); break;
        case 13: cpu->addr += (int8_t)_gd(); cpu->wz = cpu->addr; break;
        //--- mread for n
        case 14: break;
        case 15: _wait();_mread(cpu->pc++); break;
        case 16: cpu->dlatch=_gd(); break;
        //--- filler tick
        case 17: break;
        case 18: {
            // branch to ld (hl),n and skip the original mread cycle for loading 'n'
            cpu->op = _z80_opstate_table[cpu->opcode];
            cpu->op.step += 3;
        } break;
        //=== special opcode fetch machine cycle for CB-prefixed instructions
        case 19: _wait(); cpu->opcode = _gd(); break;
        case 20: pins = _z80_refresh(cpu, pins); break;
        case 21: {
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
        // -- overlapped
        case   22: _fetch();break;
        
        //  01: ld bc,nn (M:3 T:10)
        // -- mread
        case   23: break;
        case   24: _wait();_mread(cpu->pc++);break;
        case   25: cpu->c=_gd();break;
        // -- mread
        case   26: break;
        case   27: _wait();_mread(cpu->pc++);break;
        case   28: cpu->b=_gd();break;
        // -- overlapped
        case   29: _fetch();break;
        
        //  02: ld (bc),a (M:2 T:7)
        // -- mwrite
        case   30: break;
        case   31: _wait();_mwrite(cpu->bc,cpu->a);cpu->wzl=cpu->c+1;cpu->wzh=cpu->a;;break;
        case   32: break;
        // -- overlapped
        case   33: _fetch();break;
        
        //  03: inc bc (M:2 T:6)
        // -- generic
        case   34: cpu->bc++;break;
        case   35: break;
        // -- overlapped
        case   36: _fetch();break;
        
        //  04: inc b (M:1 T:4)
        // -- overlapped
        case   37: cpu->b=_z80_inc8(cpu,cpu->b);_fetch();break;
        
        //  05: dec b (M:1 T:4)
        // -- overlapped
        case   38: cpu->b=_z80_dec8(cpu,cpu->b);_fetch();break;
        
        //  06: ld b,n (M:2 T:7)
        // -- mread
        case   39: break;
        case   40: _wait();_mread(cpu->pc++);break;
        case   41: cpu->b=_gd();break;
        // -- overlapped
        case   42: _fetch();break;
        
        //  07: rlca (M:1 T:4)
        // -- overlapped
        case   43: _z80_rlca(cpu);_fetch();break;
        
        //  08: ex af,af' (M:1 T:4)
        // -- overlapped
        case   44: _z80_ex_af_af2(cpu);_fetch();break;
        
        //  09: add hl,bc (M:2 T:11)
        // -- generic
        case   45: _z80_add16(cpu,cpu->bc);break;
        case   46: break;
        case   47: break;
        case   48: break;
        case   49: break;
        case   50: break;
        case   51: break;
        // -- overlapped
        case   52: _fetch();break;
        
        //  0A: ld a,(bc) (M:2 T:7)
        // -- mread
        case   53: break;
        case   54: _wait();_mread(cpu->bc);break;
        case   55: cpu->a=_gd();cpu->wz=cpu->bc+1;break;
        // -- overlapped
        case   56: _fetch();break;
        
        //  0B: dec bc (M:2 T:6)
        // -- generic
        case   57: cpu->bc--;break;
        case   58: break;
        // -- overlapped
        case   59: _fetch();break;
        
        //  0C: inc c (M:1 T:4)
        // -- overlapped
        case   60: cpu->c=_z80_inc8(cpu,cpu->c);_fetch();break;
        
        //  0D: dec c (M:1 T:4)
        // -- overlapped
        case   61: cpu->c=_z80_dec8(cpu,cpu->c);_fetch();break;
        
        //  0E: ld c,n (M:2 T:7)
        // -- mread
        case   62: break;
        case   63: _wait();_mread(cpu->pc++);break;
        case   64: cpu->c=_gd();break;
        // -- overlapped
        case   65: _fetch();break;
        
        //  0F: rrca (M:1 T:4)
        // -- overlapped
        case   66: _z80_rrca(cpu);_fetch();break;
        
        //  10: djnz d (M:4 T:13)
        // -- generic
        case   67: break;
        // -- mread
        case   68: break;
        case   69: _wait();_mread(cpu->pc++);break;
        case   70: cpu->dlatch=_gd();if(--cpu->b==0){_z80_skip(cpu,5);};break;
        // -- generic
        case   71: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;;break;
        case   72: break;
        case   73: break;
        case   74: break;
        case   75: break;
        // -- overlapped
        case   76: _fetch();break;
        
        //  11: ld de,nn (M:3 T:10)
        // -- mread
        case   77: break;
        case   78: _wait();_mread(cpu->pc++);break;
        case   79: cpu->e=_gd();break;
        // -- mread
        case   80: break;
        case   81: _wait();_mread(cpu->pc++);break;
        case   82: cpu->d=_gd();break;
        // -- overlapped
        case   83: _fetch();break;
        
        //  12: ld (de),a (M:2 T:7)
        // -- mwrite
        case   84: break;
        case   85: _wait();_mwrite(cpu->de,cpu->a);cpu->wzl=cpu->e+1;cpu->wzh=cpu->a;;break;
        case   86: break;
        // -- overlapped
        case   87: _fetch();break;
        
        //  13: inc de (M:2 T:6)
        // -- generic
        case   88: cpu->de++;break;
        case   89: break;
        // -- overlapped
        case   90: _fetch();break;
        
        //  14: inc d (M:1 T:4)
        // -- overlapped
        case   91: cpu->d=_z80_inc8(cpu,cpu->d);_fetch();break;
        
        //  15: dec d (M:1 T:4)
        // -- overlapped
        case   92: cpu->d=_z80_dec8(cpu,cpu->d);_fetch();break;
        
        //  16: ld d,n (M:2 T:7)
        // -- mread
        case   93: break;
        case   94: _wait();_mread(cpu->pc++);break;
        case   95: cpu->d=_gd();break;
        // -- overlapped
        case   96: _fetch();break;
        
        //  17: rla (M:1 T:4)
        // -- overlapped
        case   97: _z80_rla(cpu);_fetch();break;
        
        //  18: jr d (M:3 T:12)
        // -- mread
        case   98: break;
        case   99: _wait();_mread(cpu->pc++);break;
        case  100: cpu->dlatch=_gd();break;
        // -- generic
        case  101: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;;break;
        case  102: break;
        case  103: break;
        case  104: break;
        case  105: break;
        // -- overlapped
        case  106: _fetch();break;
        
        //  19: add hl,de (M:2 T:11)
        // -- generic
        case  107: _z80_add16(cpu,cpu->de);break;
        case  108: break;
        case  109: break;
        case  110: break;
        case  111: break;
        case  112: break;
        case  113: break;
        // -- overlapped
        case  114: _fetch();break;
        
        //  1A: ld a,(de) (M:2 T:7)
        // -- mread
        case  115: break;
        case  116: _wait();_mread(cpu->de);break;
        case  117: cpu->a=_gd();cpu->wz=cpu->de+1;break;
        // -- overlapped
        case  118: _fetch();break;
        
        //  1B: dec de (M:2 T:6)
        // -- generic
        case  119: cpu->de--;break;
        case  120: break;
        // -- overlapped
        case  121: _fetch();break;
        
        //  1C: inc e (M:1 T:4)
        // -- overlapped
        case  122: cpu->e=_z80_inc8(cpu,cpu->e);_fetch();break;
        
        //  1D: dec e (M:1 T:4)
        // -- overlapped
        case  123: cpu->e=_z80_dec8(cpu,cpu->e);_fetch();break;
        
        //  1E: ld e,n (M:2 T:7)
        // -- mread
        case  124: break;
        case  125: _wait();_mread(cpu->pc++);break;
        case  126: cpu->e=_gd();break;
        // -- overlapped
        case  127: _fetch();break;
        
        //  1F: rra (M:1 T:4)
        // -- overlapped
        case  128: _z80_rra(cpu);_fetch();break;
        
        //  20: jr nz,d (M:3 T:12)
        // -- mread
        case  129: break;
        case  130: _wait();_mread(cpu->pc++);break;
        case  131: cpu->dlatch=_gd();if(!(_cc_nz)){_z80_skip(cpu,5);};break;
        // -- generic
        case  132: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;;break;
        case  133: break;
        case  134: break;
        case  135: break;
        case  136: break;
        // -- overlapped
        case  137: _fetch();break;
        
        //  21: ld hl,nn (M:3 T:10)
        // -- mread
        case  138: break;
        case  139: _wait();_mread(cpu->pc++);break;
        case  140: cpu->hlx[cpu->hlx_idx].l=_gd();break;
        // -- mread
        case  141: break;
        case  142: _wait();_mread(cpu->pc++);break;
        case  143: cpu->hlx[cpu->hlx_idx].h=_gd();break;
        // -- overlapped
        case  144: _fetch();break;
        
        //  22: ld (nn),hl (M:5 T:16)
        // -- mread
        case  145: break;
        case  146: _wait();_mread(cpu->pc++);break;
        case  147: cpu->wzl=_gd();break;
        // -- mread
        case  148: break;
        case  149: _wait();_mread(cpu->pc++);break;
        case  150: cpu->wzh=_gd();break;
        // -- mwrite
        case  151: break;
        case  152: _wait();_mwrite(cpu->wz++,cpu->hlx[cpu->hlx_idx].l);;break;
        case  153: break;
        // -- mwrite
        case  154: break;
        case  155: _wait();_mwrite(cpu->wz,cpu->hlx[cpu->hlx_idx].h);;break;
        case  156: break;
        // -- overlapped
        case  157: _fetch();break;
        
        //  23: inc hl (M:2 T:6)
        // -- generic
        case  158: cpu->hlx[cpu->hlx_idx].hl++;break;
        case  159: break;
        // -- overlapped
        case  160: _fetch();break;
        
        //  24: inc h (M:1 T:4)
        // -- overlapped
        case  161: cpu->hlx[cpu->hlx_idx].h=_z80_inc8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch();break;
        
        //  25: dec h (M:1 T:4)
        // -- overlapped
        case  162: cpu->hlx[cpu->hlx_idx].h=_z80_dec8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch();break;
        
        //  26: ld h,n (M:2 T:7)
        // -- mread
        case  163: break;
        case  164: _wait();_mread(cpu->pc++);break;
        case  165: cpu->hlx[cpu->hlx_idx].h=_gd();break;
        // -- overlapped
        case  166: _fetch();break;
        
        //  27: daa (M:1 T:4)
        // -- overlapped
        case  167: _z80_daa(cpu);_fetch();break;
        
        //  28: jr z,d (M:3 T:12)
        // -- mread
        case  168: break;
        case  169: _wait();_mread(cpu->pc++);break;
        case  170: cpu->dlatch=_gd();if(!(_cc_z)){_z80_skip(cpu,5);};break;
        // -- generic
        case  171: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;;break;
        case  172: break;
        case  173: break;
        case  174: break;
        case  175: break;
        // -- overlapped
        case  176: _fetch();break;
        
        //  29: add hl,hl (M:2 T:11)
        // -- generic
        case  177: _z80_add16(cpu,cpu->hlx[cpu->hlx_idx].hl);break;
        case  178: break;
        case  179: break;
        case  180: break;
        case  181: break;
        case  182: break;
        case  183: break;
        // -- overlapped
        case  184: _fetch();break;
        
        //  2A: ld hl,(nn) (M:5 T:16)
        // -- mread
        case  185: break;
        case  186: _wait();_mread(cpu->pc++);break;
        case  187: cpu->wzl=_gd();break;
        // -- mread
        case  188: break;
        case  189: _wait();_mread(cpu->pc++);break;
        case  190: cpu->wzh=_gd();break;
        // -- mread
        case  191: break;
        case  192: _wait();_mread(cpu->wz++);break;
        case  193: cpu->hlx[cpu->hlx_idx].l=_gd();break;
        // -- mread
        case  194: break;
        case  195: _wait();_mread(cpu->wz);break;
        case  196: cpu->hlx[cpu->hlx_idx].h=_gd();break;
        // -- overlapped
        case  197: _fetch();break;
        
        //  2B: dec hl (M:2 T:6)
        // -- generic
        case  198: cpu->hlx[cpu->hlx_idx].hl--;break;
        case  199: break;
        // -- overlapped
        case  200: _fetch();break;
        
        //  2C: inc l (M:1 T:4)
        // -- overlapped
        case  201: cpu->hlx[cpu->hlx_idx].l=_z80_inc8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch();break;
        
        //  2D: dec l (M:1 T:4)
        // -- overlapped
        case  202: cpu->hlx[cpu->hlx_idx].l=_z80_dec8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch();break;
        
        //  2E: ld l,n (M:2 T:7)
        // -- mread
        case  203: break;
        case  204: _wait();_mread(cpu->pc++);break;
        case  205: cpu->hlx[cpu->hlx_idx].l=_gd();break;
        // -- overlapped
        case  206: _fetch();break;
        
        //  2F: cpl (M:1 T:4)
        // -- overlapped
        case  207: _z80_cpl(cpu);_fetch();break;
        
        //  30: jr nc,d (M:3 T:12)
        // -- mread
        case  208: break;
        case  209: _wait();_mread(cpu->pc++);break;
        case  210: cpu->dlatch=_gd();if(!(_cc_nc)){_z80_skip(cpu,5);};break;
        // -- generic
        case  211: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;;break;
        case  212: break;
        case  213: break;
        case  214: break;
        case  215: break;
        // -- overlapped
        case  216: _fetch();break;
        
        //  31: ld sp,nn (M:3 T:10)
        // -- mread
        case  217: break;
        case  218: _wait();_mread(cpu->pc++);break;
        case  219: cpu->spl=_gd();break;
        // -- mread
        case  220: break;
        case  221: _wait();_mread(cpu->pc++);break;
        case  222: cpu->sph=_gd();break;
        // -- overlapped
        case  223: _fetch();break;
        
        //  32: ld (nn),a (M:4 T:13)
        // -- mread
        case  224: break;
        case  225: _wait();_mread(cpu->pc++);break;
        case  226: cpu->wzl=_gd();break;
        // -- mread
        case  227: break;
        case  228: _wait();_mread(cpu->pc++);break;
        case  229: cpu->wzh=_gd();break;
        // -- mwrite
        case  230: break;
        case  231: _wait();_mwrite(cpu->wz++,cpu->a);cpu->wzh=cpu->a;;break;
        case  232: break;
        // -- overlapped
        case  233: _fetch();break;
        
        //  33: inc sp (M:2 T:6)
        // -- generic
        case  234: cpu->sp++;break;
        case  235: break;
        // -- overlapped
        case  236: _fetch();break;
        
        //  34: inc (hl) (M:3 T:11)
        // -- mread
        case  237: break;
        case  238: _wait();_mread(cpu->addr);break;
        case  239: cpu->dlatch=_gd();cpu->dlatch=_z80_inc8(cpu,cpu->dlatch);break;
        case  240: break;
        // -- mwrite
        case  241: break;
        case  242: _wait();_mwrite(cpu->addr,cpu->dlatch);;break;
        case  243: break;
        // -- overlapped
        case  244: _fetch();break;
        
        //  35: dec (hl) (M:3 T:11)
        // -- mread
        case  245: break;
        case  246: _wait();_mread(cpu->addr);break;
        case  247: cpu->dlatch=_gd();cpu->dlatch=_z80_dec8(cpu,cpu->dlatch);break;
        case  248: break;
        // -- mwrite
        case  249: break;
        case  250: _wait();_mwrite(cpu->addr,cpu->dlatch);;break;
        case  251: break;
        // -- overlapped
        case  252: _fetch();break;
        
        //  36: ld (hl),n (M:3 T:10)
        // -- mread
        case  253: break;
        case  254: _wait();_mread(cpu->pc++);break;
        case  255: cpu->dlatch=_gd();break;
        // -- mwrite
        case  256: break;
        case  257: _wait();_mwrite(cpu->addr,cpu->dlatch);;break;
        case  258: break;
        // -- overlapped
        case  259: _fetch();break;
        
        //  37: scf (M:1 T:4)
        // -- overlapped
        case  260: _z80_scf(cpu);_fetch();break;
        
        //  38: jr c,d (M:3 T:12)
        // -- mread
        case  261: break;
        case  262: _wait();_mread(cpu->pc++);break;
        case  263: cpu->dlatch=_gd();if(!(_cc_c)){_z80_skip(cpu,5);};break;
        // -- generic
        case  264: cpu->pc+=(int8_t)cpu->dlatch;cpu->wz=cpu->pc;;break;
        case  265: break;
        case  266: break;
        case  267: break;
        case  268: break;
        // -- overlapped
        case  269: _fetch();break;
        
        //  39: add hl,sp (M:2 T:11)
        // -- generic
        case  270: _z80_add16(cpu,cpu->sp);break;
        case  271: break;
        case  272: break;
        case  273: break;
        case  274: break;
        case  275: break;
        case  276: break;
        // -- overlapped
        case  277: _fetch();break;
        
        //  3A: ld a,(nn) (M:4 T:13)
        // -- mread
        case  278: break;
        case  279: _wait();_mread(cpu->pc++);break;
        case  280: cpu->wzl=_gd();break;
        // -- mread
        case  281: break;
        case  282: _wait();_mread(cpu->pc++);break;
        case  283: cpu->wzh=_gd();break;
        // -- mread
        case  284: break;
        case  285: _wait();_mread(cpu->wz++);break;
        case  286: cpu->a=_gd();break;
        // -- overlapped
        case  287: _fetch();break;
        
        //  3B: dec sp (M:2 T:6)
        // -- generic
        case  288: cpu->sp--;break;
        case  289: break;
        // -- overlapped
        case  290: _fetch();break;
        
        //  3C: inc a (M:1 T:4)
        // -- overlapped
        case  291: cpu->a=_z80_inc8(cpu,cpu->a);_fetch();break;
        
        //  3D: dec a (M:1 T:4)
        // -- overlapped
        case  292: cpu->a=_z80_dec8(cpu,cpu->a);_fetch();break;
        
        //  3E: ld a,n (M:2 T:7)
        // -- mread
        case  293: break;
        case  294: _wait();_mread(cpu->pc++);break;
        case  295: cpu->a=_gd();break;
        // -- overlapped
        case  296: _fetch();break;
        
        //  3F: ccf (M:1 T:4)
        // -- overlapped
        case  297: _z80_ccf(cpu);_fetch();break;
        
        //  40: ld b,b (M:1 T:4)
        // -- overlapped
        case  298: cpu->b=cpu->b;_fetch();break;
        
        //  41: ld b,c (M:1 T:4)
        // -- overlapped
        case  299: cpu->b=cpu->c;_fetch();break;
        
        //  42: ld b,d (M:1 T:4)
        // -- overlapped
        case  300: cpu->b=cpu->d;_fetch();break;
        
        //  43: ld b,e (M:1 T:4)
        // -- overlapped
        case  301: cpu->b=cpu->e;_fetch();break;
        
        //  44: ld b,h (M:1 T:4)
        // -- overlapped
        case  302: cpu->b=cpu->hlx[cpu->hlx_idx].h;_fetch();break;
        
        //  45: ld b,l (M:1 T:4)
        // -- overlapped
        case  303: cpu->b=cpu->hlx[cpu->hlx_idx].l;_fetch();break;
        
        //  46: ld b,(hl) (M:2 T:7)
        // -- mread
        case  304: break;
        case  305: _wait();_mread(cpu->addr);break;
        case  306: cpu->b=_gd();break;
        // -- overlapped
        case  307: _fetch();break;
        
        //  47: ld b,a (M:1 T:4)
        // -- overlapped
        case  308: cpu->b=cpu->a;_fetch();break;
        
        //  48: ld c,b (M:1 T:4)
        // -- overlapped
        case  309: cpu->c=cpu->b;_fetch();break;
        
        //  49: ld c,c (M:1 T:4)
        // -- overlapped
        case  310: cpu->c=cpu->c;_fetch();break;
        
        //  4A: ld c,d (M:1 T:4)
        // -- overlapped
        case  311: cpu->c=cpu->d;_fetch();break;
        
        //  4B: ld c,e (M:1 T:4)
        // -- overlapped
        case  312: cpu->c=cpu->e;_fetch();break;
        
        //  4C: ld c,h (M:1 T:4)
        // -- overlapped
        case  313: cpu->c=cpu->hlx[cpu->hlx_idx].h;_fetch();break;
        
        //  4D: ld c,l (M:1 T:4)
        // -- overlapped
        case  314: cpu->c=cpu->hlx[cpu->hlx_idx].l;_fetch();break;
        
        //  4E: ld c,(hl) (M:2 T:7)
        // -- mread
        case  315: break;
        case  316: _wait();_mread(cpu->addr);break;
        case  317: cpu->c=_gd();break;
        // -- overlapped
        case  318: _fetch();break;
        
        //  4F: ld c,a (M:1 T:4)
        // -- overlapped
        case  319: cpu->c=cpu->a;_fetch();break;
        
        //  50: ld d,b (M:1 T:4)
        // -- overlapped
        case  320: cpu->d=cpu->b;_fetch();break;
        
        //  51: ld d,c (M:1 T:4)
        // -- overlapped
        case  321: cpu->d=cpu->c;_fetch();break;
        
        //  52: ld d,d (M:1 T:4)
        // -- overlapped
        case  322: cpu->d=cpu->d;_fetch();break;
        
        //  53: ld d,e (M:1 T:4)
        // -- overlapped
        case  323: cpu->d=cpu->e;_fetch();break;
        
        //  54: ld d,h (M:1 T:4)
        // -- overlapped
        case  324: cpu->d=cpu->hlx[cpu->hlx_idx].h;_fetch();break;
        
        //  55: ld d,l (M:1 T:4)
        // -- overlapped
        case  325: cpu->d=cpu->hlx[cpu->hlx_idx].l;_fetch();break;
        
        //  56: ld d,(hl) (M:2 T:7)
        // -- mread
        case  326: break;
        case  327: _wait();_mread(cpu->addr);break;
        case  328: cpu->d=_gd();break;
        // -- overlapped
        case  329: _fetch();break;
        
        //  57: ld d,a (M:1 T:4)
        // -- overlapped
        case  330: cpu->d=cpu->a;_fetch();break;
        
        //  58: ld e,b (M:1 T:4)
        // -- overlapped
        case  331: cpu->e=cpu->b;_fetch();break;
        
        //  59: ld e,c (M:1 T:4)
        // -- overlapped
        case  332: cpu->e=cpu->c;_fetch();break;
        
        //  5A: ld e,d (M:1 T:4)
        // -- overlapped
        case  333: cpu->e=cpu->d;_fetch();break;
        
        //  5B: ld e,e (M:1 T:4)
        // -- overlapped
        case  334: cpu->e=cpu->e;_fetch();break;
        
        //  5C: ld e,h (M:1 T:4)
        // -- overlapped
        case  335: cpu->e=cpu->hlx[cpu->hlx_idx].h;_fetch();break;
        
        //  5D: ld e,l (M:1 T:4)
        // -- overlapped
        case  336: cpu->e=cpu->hlx[cpu->hlx_idx].l;_fetch();break;
        
        //  5E: ld e,(hl) (M:2 T:7)
        // -- mread
        case  337: break;
        case  338: _wait();_mread(cpu->addr);break;
        case  339: cpu->e=_gd();break;
        // -- overlapped
        case  340: _fetch();break;
        
        //  5F: ld e,a (M:1 T:4)
        // -- overlapped
        case  341: cpu->e=cpu->a;_fetch();break;
        
        //  60: ld h,b (M:1 T:4)
        // -- overlapped
        case  342: cpu->hlx[cpu->hlx_idx].h=cpu->b;_fetch();break;
        
        //  61: ld h,c (M:1 T:4)
        // -- overlapped
        case  343: cpu->hlx[cpu->hlx_idx].h=cpu->c;_fetch();break;
        
        //  62: ld h,d (M:1 T:4)
        // -- overlapped
        case  344: cpu->hlx[cpu->hlx_idx].h=cpu->d;_fetch();break;
        
        //  63: ld h,e (M:1 T:4)
        // -- overlapped
        case  345: cpu->hlx[cpu->hlx_idx].h=cpu->e;_fetch();break;
        
        //  64: ld h,h (M:1 T:4)
        // -- overlapped
        case  346: cpu->hlx[cpu->hlx_idx].h=cpu->hlx[cpu->hlx_idx].h;_fetch();break;
        
        //  65: ld h,l (M:1 T:4)
        // -- overlapped
        case  347: cpu->hlx[cpu->hlx_idx].h=cpu->hlx[cpu->hlx_idx].l;_fetch();break;
        
        //  66: ld h,(hl) (M:2 T:7)
        // -- mread
        case  348: break;
        case  349: _wait();_mread(cpu->addr);break;
        case  350: cpu->h=_gd();break;
        // -- overlapped
        case  351: _fetch();break;
        
        //  67: ld h,a (M:1 T:4)
        // -- overlapped
        case  352: cpu->hlx[cpu->hlx_idx].h=cpu->a;_fetch();break;
        
        //  68: ld l,b (M:1 T:4)
        // -- overlapped
        case  353: cpu->hlx[cpu->hlx_idx].l=cpu->b;_fetch();break;
        
        //  69: ld l,c (M:1 T:4)
        // -- overlapped
        case  354: cpu->hlx[cpu->hlx_idx].l=cpu->c;_fetch();break;
        
        //  6A: ld l,d (M:1 T:4)
        // -- overlapped
        case  355: cpu->hlx[cpu->hlx_idx].l=cpu->d;_fetch();break;
        
        //  6B: ld l,e (M:1 T:4)
        // -- overlapped
        case  356: cpu->hlx[cpu->hlx_idx].l=cpu->e;_fetch();break;
        
        //  6C: ld l,h (M:1 T:4)
        // -- overlapped
        case  357: cpu->hlx[cpu->hlx_idx].l=cpu->hlx[cpu->hlx_idx].h;_fetch();break;
        
        //  6D: ld l,l (M:1 T:4)
        // -- overlapped
        case  358: cpu->hlx[cpu->hlx_idx].l=cpu->hlx[cpu->hlx_idx].l;_fetch();break;
        
        //  6E: ld l,(hl) (M:2 T:7)
        // -- mread
        case  359: break;
        case  360: _wait();_mread(cpu->addr);break;
        case  361: cpu->l=_gd();break;
        // -- overlapped
        case  362: _fetch();break;
        
        //  6F: ld l,a (M:1 T:4)
        // -- overlapped
        case  363: cpu->hlx[cpu->hlx_idx].l=cpu->a;_fetch();break;
        
        //  70: ld (hl),b (M:2 T:7)
        // -- mwrite
        case  364: break;
        case  365: _wait();_mwrite(cpu->addr,cpu->b);;break;
        case  366: break;
        // -- overlapped
        case  367: _fetch();break;
        
        //  71: ld (hl),c (M:2 T:7)
        // -- mwrite
        case  368: break;
        case  369: _wait();_mwrite(cpu->addr,cpu->c);;break;
        case  370: break;
        // -- overlapped
        case  371: _fetch();break;
        
        //  72: ld (hl),d (M:2 T:7)
        // -- mwrite
        case  372: break;
        case  373: _wait();_mwrite(cpu->addr,cpu->d);;break;
        case  374: break;
        // -- overlapped
        case  375: _fetch();break;
        
        //  73: ld (hl),e (M:2 T:7)
        // -- mwrite
        case  376: break;
        case  377: _wait();_mwrite(cpu->addr,cpu->e);;break;
        case  378: break;
        // -- overlapped
        case  379: _fetch();break;
        
        //  74: ld (hl),h (M:2 T:7)
        // -- mwrite
        case  380: break;
        case  381: _wait();_mwrite(cpu->addr,cpu->h);;break;
        case  382: break;
        // -- overlapped
        case  383: _fetch();break;
        
        //  75: ld (hl),l (M:2 T:7)
        // -- mwrite
        case  384: break;
        case  385: _wait();_mwrite(cpu->addr,cpu->l);;break;
        case  386: break;
        // -- overlapped
        case  387: _fetch();break;
        
        //  76: halt (M:1 T:4)
        // -- overlapped
        case  388: pins=_z80_halt(cpu,pins);_fetch();break;
        
        //  77: ld (hl),a (M:2 T:7)
        // -- mwrite
        case  389: break;
        case  390: _wait();_mwrite(cpu->addr,cpu->a);;break;
        case  391: break;
        // -- overlapped
        case  392: _fetch();break;
        
        //  78: ld a,b (M:1 T:4)
        // -- overlapped
        case  393: cpu->a=cpu->b;_fetch();break;
        
        //  79: ld a,c (M:1 T:4)
        // -- overlapped
        case  394: cpu->a=cpu->c;_fetch();break;
        
        //  7A: ld a,d (M:1 T:4)
        // -- overlapped
        case  395: cpu->a=cpu->d;_fetch();break;
        
        //  7B: ld a,e (M:1 T:4)
        // -- overlapped
        case  396: cpu->a=cpu->e;_fetch();break;
        
        //  7C: ld a,h (M:1 T:4)
        // -- overlapped
        case  397: cpu->a=cpu->hlx[cpu->hlx_idx].h;_fetch();break;
        
        //  7D: ld a,l (M:1 T:4)
        // -- overlapped
        case  398: cpu->a=cpu->hlx[cpu->hlx_idx].l;_fetch();break;
        
        //  7E: ld a,(hl) (M:2 T:7)
        // -- mread
        case  399: break;
        case  400: _wait();_mread(cpu->addr);break;
        case  401: cpu->a=_gd();break;
        // -- overlapped
        case  402: _fetch();break;
        
        //  7F: ld a,a (M:1 T:4)
        // -- overlapped
        case  403: cpu->a=cpu->a;_fetch();break;
        
        //  80: add b (M:1 T:4)
        // -- overlapped
        case  404: _z80_add8(cpu,cpu->b);_fetch();break;
        
        //  81: add c (M:1 T:4)
        // -- overlapped
        case  405: _z80_add8(cpu,cpu->c);_fetch();break;
        
        //  82: add d (M:1 T:4)
        // -- overlapped
        case  406: _z80_add8(cpu,cpu->d);_fetch();break;
        
        //  83: add e (M:1 T:4)
        // -- overlapped
        case  407: _z80_add8(cpu,cpu->e);_fetch();break;
        
        //  84: add h (M:1 T:4)
        // -- overlapped
        case  408: _z80_add8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch();break;
        
        //  85: add l (M:1 T:4)
        // -- overlapped
        case  409: _z80_add8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch();break;
        
        //  86: add (hl) (M:2 T:7)
        // -- mread
        case  410: break;
        case  411: _wait();_mread(cpu->addr);break;
        case  412: cpu->dlatch=_gd();break;
        // -- overlapped
        case  413: _z80_add8(cpu,cpu->dlatch);_fetch();break;
        
        //  87: add a (M:1 T:4)
        // -- overlapped
        case  414: _z80_add8(cpu,cpu->a);_fetch();break;
        
        //  88: adc b (M:1 T:4)
        // -- overlapped
        case  415: _z80_adc8(cpu,cpu->b);_fetch();break;
        
        //  89: adc c (M:1 T:4)
        // -- overlapped
        case  416: _z80_adc8(cpu,cpu->c);_fetch();break;
        
        //  8A: adc d (M:1 T:4)
        // -- overlapped
        case  417: _z80_adc8(cpu,cpu->d);_fetch();break;
        
        //  8B: adc e (M:1 T:4)
        // -- overlapped
        case  418: _z80_adc8(cpu,cpu->e);_fetch();break;
        
        //  8C: adc h (M:1 T:4)
        // -- overlapped
        case  419: _z80_adc8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch();break;
        
        //  8D: adc l (M:1 T:4)
        // -- overlapped
        case  420: _z80_adc8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch();break;
        
        //  8E: adc (hl) (M:2 T:7)
        // -- mread
        case  421: break;
        case  422: _wait();_mread(cpu->addr);break;
        case  423: cpu->dlatch=_gd();break;
        // -- overlapped
        case  424: _z80_adc8(cpu,cpu->dlatch);_fetch();break;
        
        //  8F: adc a (M:1 T:4)
        // -- overlapped
        case  425: _z80_adc8(cpu,cpu->a);_fetch();break;
        
        //  90: sub b (M:1 T:4)
        // -- overlapped
        case  426: _z80_sub8(cpu,cpu->b);_fetch();break;
        
        //  91: sub c (M:1 T:4)
        // -- overlapped
        case  427: _z80_sub8(cpu,cpu->c);_fetch();break;
        
        //  92: sub d (M:1 T:4)
        // -- overlapped
        case  428: _z80_sub8(cpu,cpu->d);_fetch();break;
        
        //  93: sub e (M:1 T:4)
        // -- overlapped
        case  429: _z80_sub8(cpu,cpu->e);_fetch();break;
        
        //  94: sub h (M:1 T:4)
        // -- overlapped
        case  430: _z80_sub8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch();break;
        
        //  95: sub l (M:1 T:4)
        // -- overlapped
        case  431: _z80_sub8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch();break;
        
        //  96: sub (hl) (M:2 T:7)
        // -- mread
        case  432: break;
        case  433: _wait();_mread(cpu->addr);break;
        case  434: cpu->dlatch=_gd();break;
        // -- overlapped
        case  435: _z80_sub8(cpu,cpu->dlatch);_fetch();break;
        
        //  97: sub a (M:1 T:4)
        // -- overlapped
        case  436: _z80_sub8(cpu,cpu->a);_fetch();break;
        
        //  98: sbc b (M:1 T:4)
        // -- overlapped
        case  437: _z80_sbc8(cpu,cpu->b);_fetch();break;
        
        //  99: sbc c (M:1 T:4)
        // -- overlapped
        case  438: _z80_sbc8(cpu,cpu->c);_fetch();break;
        
        //  9A: sbc d (M:1 T:4)
        // -- overlapped
        case  439: _z80_sbc8(cpu,cpu->d);_fetch();break;
        
        //  9B: sbc e (M:1 T:4)
        // -- overlapped
        case  440: _z80_sbc8(cpu,cpu->e);_fetch();break;
        
        //  9C: sbc h (M:1 T:4)
        // -- overlapped
        case  441: _z80_sbc8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch();break;
        
        //  9D: sbc l (M:1 T:4)
        // -- overlapped
        case  442: _z80_sbc8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch();break;
        
        //  9E: sbc (hl) (M:2 T:7)
        // -- mread
        case  443: break;
        case  444: _wait();_mread(cpu->addr);break;
        case  445: cpu->dlatch=_gd();break;
        // -- overlapped
        case  446: _z80_sbc8(cpu,cpu->dlatch);_fetch();break;
        
        //  9F: sbc a (M:1 T:4)
        // -- overlapped
        case  447: _z80_sbc8(cpu,cpu->a);_fetch();break;
        
        //  A0: and b (M:1 T:4)
        // -- overlapped
        case  448: _z80_and8(cpu,cpu->b);_fetch();break;
        
        //  A1: and c (M:1 T:4)
        // -- overlapped
        case  449: _z80_and8(cpu,cpu->c);_fetch();break;
        
        //  A2: and d (M:1 T:4)
        // -- overlapped
        case  450: _z80_and8(cpu,cpu->d);_fetch();break;
        
        //  A3: and e (M:1 T:4)
        // -- overlapped
        case  451: _z80_and8(cpu,cpu->e);_fetch();break;
        
        //  A4: and h (M:1 T:4)
        // -- overlapped
        case  452: _z80_and8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch();break;
        
        //  A5: and l (M:1 T:4)
        // -- overlapped
        case  453: _z80_and8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch();break;
        
        //  A6: and (hl) (M:2 T:7)
        // -- mread
        case  454: break;
        case  455: _wait();_mread(cpu->addr);break;
        case  456: cpu->dlatch=_gd();break;
        // -- overlapped
        case  457: _z80_and8(cpu,cpu->dlatch);_fetch();break;
        
        //  A7: and a (M:1 T:4)
        // -- overlapped
        case  458: _z80_and8(cpu,cpu->a);_fetch();break;
        
        //  A8: xor b (M:1 T:4)
        // -- overlapped
        case  459: _z80_xor8(cpu,cpu->b);_fetch();break;
        
        //  A9: xor c (M:1 T:4)
        // -- overlapped
        case  460: _z80_xor8(cpu,cpu->c);_fetch();break;
        
        //  AA: xor d (M:1 T:4)
        // -- overlapped
        case  461: _z80_xor8(cpu,cpu->d);_fetch();break;
        
        //  AB: xor e (M:1 T:4)
        // -- overlapped
        case  462: _z80_xor8(cpu,cpu->e);_fetch();break;
        
        //  AC: xor h (M:1 T:4)
        // -- overlapped
        case  463: _z80_xor8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch();break;
        
        //  AD: xor l (M:1 T:4)
        // -- overlapped
        case  464: _z80_xor8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch();break;
        
        //  AE: xor (hl) (M:2 T:7)
        // -- mread
        case  465: break;
        case  466: _wait();_mread(cpu->addr);break;
        case  467: cpu->dlatch=_gd();break;
        // -- overlapped
        case  468: _z80_xor8(cpu,cpu->dlatch);_fetch();break;
        
        //  AF: xor a (M:1 T:4)
        // -- overlapped
        case  469: _z80_xor8(cpu,cpu->a);_fetch();break;
        
        //  B0: or b (M:1 T:4)
        // -- overlapped
        case  470: _z80_or8(cpu,cpu->b);_fetch();break;
        
        //  B1: or c (M:1 T:4)
        // -- overlapped
        case  471: _z80_or8(cpu,cpu->c);_fetch();break;
        
        //  B2: or d (M:1 T:4)
        // -- overlapped
        case  472: _z80_or8(cpu,cpu->d);_fetch();break;
        
        //  B3: or e (M:1 T:4)
        // -- overlapped
        case  473: _z80_or8(cpu,cpu->e);_fetch();break;
        
        //  B4: or h (M:1 T:4)
        // -- overlapped
        case  474: _z80_or8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch();break;
        
        //  B5: or l (M:1 T:4)
        // -- overlapped
        case  475: _z80_or8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch();break;
        
        //  B6: or (hl) (M:2 T:7)
        // -- mread
        case  476: break;
        case  477: _wait();_mread(cpu->addr);break;
        case  478: cpu->dlatch=_gd();break;
        // -- overlapped
        case  479: _z80_or8(cpu,cpu->dlatch);_fetch();break;
        
        //  B7: or a (M:1 T:4)
        // -- overlapped
        case  480: _z80_or8(cpu,cpu->a);_fetch();break;
        
        //  B8: cp b (M:1 T:4)
        // -- overlapped
        case  481: _z80_cp8(cpu,cpu->b);_fetch();break;
        
        //  B9: cp c (M:1 T:4)
        // -- overlapped
        case  482: _z80_cp8(cpu,cpu->c);_fetch();break;
        
        //  BA: cp d (M:1 T:4)
        // -- overlapped
        case  483: _z80_cp8(cpu,cpu->d);_fetch();break;
        
        //  BB: cp e (M:1 T:4)
        // -- overlapped
        case  484: _z80_cp8(cpu,cpu->e);_fetch();break;
        
        //  BC: cp h (M:1 T:4)
        // -- overlapped
        case  485: _z80_cp8(cpu,cpu->hlx[cpu->hlx_idx].h);_fetch();break;
        
        //  BD: cp l (M:1 T:4)
        // -- overlapped
        case  486: _z80_cp8(cpu,cpu->hlx[cpu->hlx_idx].l);_fetch();break;
        
        //  BE: cp (hl) (M:2 T:7)
        // -- mread
        case  487: break;
        case  488: _wait();_mread(cpu->addr);break;
        case  489: cpu->dlatch=_gd();break;
        // -- overlapped
        case  490: _z80_cp8(cpu,cpu->dlatch);_fetch();break;
        
        //  BF: cp a (M:1 T:4)
        // -- overlapped
        case  491: _z80_cp8(cpu,cpu->a);_fetch();break;
        
        //  C0: ret nz (M:4 T:11)
        // -- generic
        case  492: if(!_cc_nz){_z80_skip(cpu,6);};break;
        // -- mread
        case  493: break;
        case  494: _wait();_mread(cpu->sp++);break;
        case  495: cpu->wzl=_gd();break;
        // -- mread
        case  496: break;
        case  497: _wait();_mread(cpu->sp++);break;
        case  498: cpu->wzh=_gd();cpu->pc=cpu->wz;break;
        // -- overlapped
        case  499: _fetch();break;
        
        //  C1: pop bc (M:3 T:10)
        // -- mread
        case  500: break;
        case  501: _wait();_mread(cpu->sp++);break;
        case  502: cpu->c=_gd();break;
        // -- mread
        case  503: break;
        case  504: _wait();_mread(cpu->sp++);break;
        case  505: cpu->b=_gd();break;
        // -- overlapped
        case  506: _fetch();break;
        
        //  C2: jp nz,nn (M:3 T:10)
        // -- mread
        case  507: break;
        case  508: _wait();_mread(cpu->pc++);break;
        case  509: cpu->wzl=_gd();break;
        // -- mread
        case  510: break;
        case  511: _wait();_mread(cpu->pc++);break;
        case  512: cpu->wzh=_gd();if(_cc_nz){cpu->pc=cpu->wz;};break;
        // -- overlapped
        case  513: _fetch();break;
        
        //  C3: jp nn (M:3 T:10)
        // -- mread
        case  514: break;
        case  515: _wait();_mread(cpu->pc++);break;
        case  516: cpu->wzl=_gd();break;
        // -- mread
        case  517: break;
        case  518: _wait();_mread(cpu->pc++);break;
        case  519: cpu->wzh=_gd();cpu->pc=cpu->wz;break;
        // -- overlapped
        case  520: _fetch();break;
        
        //  C4: call nz,nn (M:6 T:17)
        // -- mread
        case  521: break;
        case  522: _wait();_mread(cpu->pc++);break;
        case  523: cpu->wzl=_gd();break;
        // -- mread
        case  524: break;
        case  525: _wait();_mread(cpu->pc++);break;
        case  526: cpu->wzh=_gd();if (!_cc_nz){_z80_skip(cpu,7);};break;
        // -- generic
        case  527: break;
        // -- mwrite
        case  528: break;
        case  529: _wait();_mwrite(--cpu->sp,cpu->pch);;break;
        case  530: break;
        // -- mwrite
        case  531: break;
        case  532: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz;;break;
        case  533: break;
        // -- overlapped
        case  534: _fetch();break;
        
        //  C5: push bc (M:4 T:11)
        // -- generic
        case  535: break;
        // -- mwrite
        case  536: break;
        case  537: _wait();_mwrite(--cpu->sp,cpu->b);;break;
        case  538: break;
        // -- mwrite
        case  539: break;
        case  540: _wait();_mwrite(--cpu->sp,cpu->c);;break;
        case  541: break;
        // -- overlapped
        case  542: _fetch();break;
        
        //  C6: add n (M:2 T:7)
        // -- mread
        case  543: break;
        case  544: _wait();_mread(cpu->pc++);break;
        case  545: cpu->dlatch=_gd();break;
        // -- overlapped
        case  546: _z80_add8(cpu,cpu->dlatch);_fetch();break;
        
        //  C7: rst 0h (M:4 T:11)
        // -- generic
        case  547: break;
        // -- mwrite
        case  548: break;
        case  549: _wait();_mwrite(--cpu->sp,cpu->pch);;break;
        case  550: break;
        // -- mwrite
        case  551: break;
        case  552: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x00;cpu->pc=cpu->wz;;break;
        case  553: break;
        // -- overlapped
        case  554: _fetch();break;
        
        //  C8: ret z (M:4 T:11)
        // -- generic
        case  555: if(!_cc_z){_z80_skip(cpu,6);};break;
        // -- mread
        case  556: break;
        case  557: _wait();_mread(cpu->sp++);break;
        case  558: cpu->wzl=_gd();break;
        // -- mread
        case  559: break;
        case  560: _wait();_mread(cpu->sp++);break;
        case  561: cpu->wzh=_gd();cpu->pc=cpu->wz;break;
        // -- overlapped
        case  562: _fetch();break;
        
        //  C9: ret (M:3 T:10)
        // -- mread
        case  563: break;
        case  564: _wait();_mread(cpu->sp++);break;
        case  565: cpu->wzl=_gd();break;
        // -- mread
        case  566: break;
        case  567: _wait();_mread(cpu->sp++);break;
        case  568: cpu->wzh=_gd();cpu->pc=cpu->wz;break;
        // -- overlapped
        case  569: _fetch();break;
        
        //  CA: jp z,nn (M:3 T:10)
        // -- mread
        case  570: break;
        case  571: _wait();_mread(cpu->pc++);break;
        case  572: cpu->wzl=_gd();break;
        // -- mread
        case  573: break;
        case  574: _wait();_mread(cpu->pc++);break;
        case  575: cpu->wzh=_gd();if(_cc_z){cpu->pc=cpu->wz;};break;
        // -- overlapped
        case  576: _fetch();break;
        
        //  CB: cb prefix (M:1 T:4)
        // -- overlapped
        case  577: _fetch_cb();break;
        
        //  CC: call z,nn (M:6 T:17)
        // -- mread
        case  578: break;
        case  579: _wait();_mread(cpu->pc++);break;
        case  580: cpu->wzl=_gd();break;
        // -- mread
        case  581: break;
        case  582: _wait();_mread(cpu->pc++);break;
        case  583: cpu->wzh=_gd();if (!_cc_z){_z80_skip(cpu,7);};break;
        // -- generic
        case  584: break;
        // -- mwrite
        case  585: break;
        case  586: _wait();_mwrite(--cpu->sp,cpu->pch);;break;
        case  587: break;
        // -- mwrite
        case  588: break;
        case  589: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz;;break;
        case  590: break;
        // -- overlapped
        case  591: _fetch();break;
        
        //  CD: call nn (M:5 T:17)
        // -- mread
        case  592: break;
        case  593: _wait();_mread(cpu->pc++);break;
        case  594: cpu->wzl=_gd();break;
        // -- mread
        case  595: break;
        case  596: _wait();_mread(cpu->pc++);break;
        case  597: cpu->wzh=_gd();break;
        case  598: break;
        // -- mwrite
        case  599: break;
        case  600: _wait();_mwrite(--cpu->sp,cpu->pch);;break;
        case  601: break;
        // -- mwrite
        case  602: break;
        case  603: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz;;break;
        case  604: break;
        // -- overlapped
        case  605: _fetch();break;
        
        //  CE: adc n (M:2 T:7)
        // -- mread
        case  606: break;
        case  607: _wait();_mread(cpu->pc++);break;
        case  608: cpu->dlatch=_gd();break;
        // -- overlapped
        case  609: _z80_adc8(cpu,cpu->dlatch);_fetch();break;
        
        //  CF: rst 8h (M:4 T:11)
        // -- generic
        case  610: break;
        // -- mwrite
        case  611: break;
        case  612: _wait();_mwrite(--cpu->sp,cpu->pch);;break;
        case  613: break;
        // -- mwrite
        case  614: break;
        case  615: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x08;cpu->pc=cpu->wz;;break;
        case  616: break;
        // -- overlapped
        case  617: _fetch();break;
        
        //  D0: ret nc (M:4 T:11)
        // -- generic
        case  618: if(!_cc_nc){_z80_skip(cpu,6);};break;
        // -- mread
        case  619: break;
        case  620: _wait();_mread(cpu->sp++);break;
        case  621: cpu->wzl=_gd();break;
        // -- mread
        case  622: break;
        case  623: _wait();_mread(cpu->sp++);break;
        case  624: cpu->wzh=_gd();cpu->pc=cpu->wz;break;
        // -- overlapped
        case  625: _fetch();break;
        
        //  D1: pop de (M:3 T:10)
        // -- mread
        case  626: break;
        case  627: _wait();_mread(cpu->sp++);break;
        case  628: cpu->e=_gd();break;
        // -- mread
        case  629: break;
        case  630: _wait();_mread(cpu->sp++);break;
        case  631: cpu->d=_gd();break;
        // -- overlapped
        case  632: _fetch();break;
        
        //  D2: jp nc,nn (M:3 T:10)
        // -- mread
        case  633: break;
        case  634: _wait();_mread(cpu->pc++);break;
        case  635: cpu->wzl=_gd();break;
        // -- mread
        case  636: break;
        case  637: _wait();_mread(cpu->pc++);break;
        case  638: cpu->wzh=_gd();if(_cc_nc){cpu->pc=cpu->wz;};break;
        // -- overlapped
        case  639: _fetch();break;
        
        //  D3: out (n),a (M:3 T:11)
        // -- mread
        case  640: break;
        case  641: _wait();_mread(cpu->pc++);break;
        case  642: cpu->wzl=_gd();cpu->wzh=cpu->a;break;
        // -- iowrite
        case  643: break;
        case  644: _iowrite(cpu->wz,cpu->a);break;
        case  645: _wait();cpu->wzl++;;break;
        case  646: break;
        // -- overlapped
        case  647: _fetch();break;
        
        //  D4: call nc,nn (M:6 T:17)
        // -- mread
        case  648: break;
        case  649: _wait();_mread(cpu->pc++);break;
        case  650: cpu->wzl=_gd();break;
        // -- mread
        case  651: break;
        case  652: _wait();_mread(cpu->pc++);break;
        case  653: cpu->wzh=_gd();if (!_cc_nc){_z80_skip(cpu,7);};break;
        // -- generic
        case  654: break;
        // -- mwrite
        case  655: break;
        case  656: _wait();_mwrite(--cpu->sp,cpu->pch);;break;
        case  657: break;
        // -- mwrite
        case  658: break;
        case  659: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz;;break;
        case  660: break;
        // -- overlapped
        case  661: _fetch();break;
        
        //  D5: push de (M:4 T:11)
        // -- generic
        case  662: break;
        // -- mwrite
        case  663: break;
        case  664: _wait();_mwrite(--cpu->sp,cpu->d);;break;
        case  665: break;
        // -- mwrite
        case  666: break;
        case  667: _wait();_mwrite(--cpu->sp,cpu->e);;break;
        case  668: break;
        // -- overlapped
        case  669: _fetch();break;
        
        //  D6: sub n (M:2 T:7)
        // -- mread
        case  670: break;
        case  671: _wait();_mread(cpu->pc++);break;
        case  672: cpu->dlatch=_gd();break;
        // -- overlapped
        case  673: _z80_sub8(cpu,cpu->dlatch);_fetch();break;
        
        //  D7: rst 10h (M:4 T:11)
        // -- generic
        case  674: break;
        // -- mwrite
        case  675: break;
        case  676: _wait();_mwrite(--cpu->sp,cpu->pch);;break;
        case  677: break;
        // -- mwrite
        case  678: break;
        case  679: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x10;cpu->pc=cpu->wz;;break;
        case  680: break;
        // -- overlapped
        case  681: _fetch();break;
        
        //  D8: ret c (M:4 T:11)
        // -- generic
        case  682: if(!_cc_c){_z80_skip(cpu,6);};break;
        // -- mread
        case  683: break;
        case  684: _wait();_mread(cpu->sp++);break;
        case  685: cpu->wzl=_gd();break;
        // -- mread
        case  686: break;
        case  687: _wait();_mread(cpu->sp++);break;
        case  688: cpu->wzh=_gd();cpu->pc=cpu->wz;break;
        // -- overlapped
        case  689: _fetch();break;
        
        //  D9: exx (M:1 T:4)
        // -- overlapped
        case  690: _z80_exx(cpu);_fetch();break;
        
        //  DA: jp c,nn (M:3 T:10)
        // -- mread
        case  691: break;
        case  692: _wait();_mread(cpu->pc++);break;
        case  693: cpu->wzl=_gd();break;
        // -- mread
        case  694: break;
        case  695: _wait();_mread(cpu->pc++);break;
        case  696: cpu->wzh=_gd();if(_cc_c){cpu->pc=cpu->wz;};break;
        // -- overlapped
        case  697: _fetch();break;
        
        //  DB: in a,(n) (M:3 T:11)
        // -- mread
        case  698: break;
        case  699: _wait();_mread(cpu->pc++);break;
        case  700: cpu->wzl=_gd();cpu->wzh=cpu->a;break;
        // -- ioread
        case  701: break;
        case  702: break;
        case  703: _wait();_ioread(cpu->wz++);break;
        case  704: cpu->dlatch=_gd();break;
        // -- overlapped
        case  705: cpu->a=cpu->dlatch;_fetch();break;
        
        //  DC: call c,nn (M:6 T:17)
        // -- mread
        case  706: break;
        case  707: _wait();_mread(cpu->pc++);break;
        case  708: cpu->wzl=_gd();break;
        // -- mread
        case  709: break;
        case  710: _wait();_mread(cpu->pc++);break;
        case  711: cpu->wzh=_gd();if (!_cc_c){_z80_skip(cpu,7);};break;
        // -- generic
        case  712: break;
        // -- mwrite
        case  713: break;
        case  714: _wait();_mwrite(--cpu->sp,cpu->pch);;break;
        case  715: break;
        // -- mwrite
        case  716: break;
        case  717: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz;;break;
        case  718: break;
        // -- overlapped
        case  719: _fetch();break;
        
        //  DD: dd prefix (M:1 T:4)
        // -- overlapped
        case  720: _fetch_dd();break;
        
        //  DE: sbc n (M:2 T:7)
        // -- mread
        case  721: break;
        case  722: _wait();_mread(cpu->pc++);break;
        case  723: cpu->dlatch=_gd();break;
        // -- overlapped
        case  724: _z80_sbc8(cpu,cpu->dlatch);_fetch();break;
        
        //  DF: rst 18h (M:4 T:11)
        // -- generic
        case  725: break;
        // -- mwrite
        case  726: break;
        case  727: _wait();_mwrite(--cpu->sp,cpu->pch);;break;
        case  728: break;
        // -- mwrite
        case  729: break;
        case  730: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x18;cpu->pc=cpu->wz;;break;
        case  731: break;
        // -- overlapped
        case  732: _fetch();break;
        
        //  E0: ret po (M:4 T:11)
        // -- generic
        case  733: if(!_cc_po){_z80_skip(cpu,6);};break;
        // -- mread
        case  734: break;
        case  735: _wait();_mread(cpu->sp++);break;
        case  736: cpu->wzl=_gd();break;
        // -- mread
        case  737: break;
        case  738: _wait();_mread(cpu->sp++);break;
        case  739: cpu->wzh=_gd();cpu->pc=cpu->wz;break;
        // -- overlapped
        case  740: _fetch();break;
        
        //  E1: pop hl (M:3 T:10)
        // -- mread
        case  741: break;
        case  742: _wait();_mread(cpu->sp++);break;
        case  743: cpu->hlx[cpu->hlx_idx].l=_gd();break;
        // -- mread
        case  744: break;
        case  745: _wait();_mread(cpu->sp++);break;
        case  746: cpu->hlx[cpu->hlx_idx].h=_gd();break;
        // -- overlapped
        case  747: _fetch();break;
        
        //  E2: jp po,nn (M:3 T:10)
        // -- mread
        case  748: break;
        case  749: _wait();_mread(cpu->pc++);break;
        case  750: cpu->wzl=_gd();break;
        // -- mread
        case  751: break;
        case  752: _wait();_mread(cpu->pc++);break;
        case  753: cpu->wzh=_gd();if(_cc_po){cpu->pc=cpu->wz;};break;
        // -- overlapped
        case  754: _fetch();break;
        
        //  E3: ex (sp),hl (M:5 T:19)
        // -- mread
        case  755: break;
        case  756: _wait();_mread(cpu->sp);break;
        case  757: cpu->wzl=_gd();break;
        // -- mread
        case  758: break;
        case  759: _wait();_mread(cpu->sp+1);break;
        case  760: cpu->wzh=_gd();break;
        case  761: break;
        // -- mwrite
        case  762: break;
        case  763: _wait();_mwrite(cpu->sp+1,cpu->hlx[cpu->hlx_idx].h);;break;
        case  764: break;
        // -- mwrite
        case  765: break;
        case  766: _wait();_mwrite(cpu->sp,cpu->hlx[cpu->hlx_idx].l);cpu->hlx[cpu->hlx_idx].hl=cpu->wz;;break;
        case  767: break;
        case  768: break;
        case  769: break;
        // -- overlapped
        case  770: _fetch();break;
        
        //  E4: call po,nn (M:6 T:17)
        // -- mread
        case  771: break;
        case  772: _wait();_mread(cpu->pc++);break;
        case  773: cpu->wzl=_gd();break;
        // -- mread
        case  774: break;
        case  775: _wait();_mread(cpu->pc++);break;
        case  776: cpu->wzh=_gd();if (!_cc_po){_z80_skip(cpu,7);};break;
        // -- generic
        case  777: break;
        // -- mwrite
        case  778: break;
        case  779: _wait();_mwrite(--cpu->sp,cpu->pch);;break;
        case  780: break;
        // -- mwrite
        case  781: break;
        case  782: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz;;break;
        case  783: break;
        // -- overlapped
        case  784: _fetch();break;
        
        //  E5: push hl (M:4 T:11)
        // -- generic
        case  785: break;
        // -- mwrite
        case  786: break;
        case  787: _wait();_mwrite(--cpu->sp,cpu->hlx[cpu->hlx_idx].h);;break;
        case  788: break;
        // -- mwrite
        case  789: break;
        case  790: _wait();_mwrite(--cpu->sp,cpu->hlx[cpu->hlx_idx].l);;break;
        case  791: break;
        // -- overlapped
        case  792: _fetch();break;
        
        //  E6: and n (M:2 T:7)
        // -- mread
        case  793: break;
        case  794: _wait();_mread(cpu->pc++);break;
        case  795: cpu->dlatch=_gd();break;
        // -- overlapped
        case  796: _z80_and8(cpu,cpu->dlatch);_fetch();break;
        
        //  E7: rst 20h (M:4 T:11)
        // -- generic
        case  797: break;
        // -- mwrite
        case  798: break;
        case  799: _wait();_mwrite(--cpu->sp,cpu->pch);;break;
        case  800: break;
        // -- mwrite
        case  801: break;
        case  802: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x20;cpu->pc=cpu->wz;;break;
        case  803: break;
        // -- overlapped
        case  804: _fetch();break;
        
        //  E8: ret pe (M:4 T:11)
        // -- generic
        case  805: if(!_cc_pe){_z80_skip(cpu,6);};break;
        // -- mread
        case  806: break;
        case  807: _wait();_mread(cpu->sp++);break;
        case  808: cpu->wzl=_gd();break;
        // -- mread
        case  809: break;
        case  810: _wait();_mread(cpu->sp++);break;
        case  811: cpu->wzh=_gd();cpu->pc=cpu->wz;break;
        // -- overlapped
        case  812: _fetch();break;
        
        //  E9: jp hl (M:1 T:4)
        // -- overlapped
        case  813: cpu->pc=cpu->hlx[cpu->hlx_idx].hl;_fetch();break;
        
        //  EA: jp pe,nn (M:3 T:10)
        // -- mread
        case  814: break;
        case  815: _wait();_mread(cpu->pc++);break;
        case  816: cpu->wzl=_gd();break;
        // -- mread
        case  817: break;
        case  818: _wait();_mread(cpu->pc++);break;
        case  819: cpu->wzh=_gd();if(_cc_pe){cpu->pc=cpu->wz;};break;
        // -- overlapped
        case  820: _fetch();break;
        
        //  EB: ex de,hl (M:1 T:4)
        // -- overlapped
        case  821: _z80_ex_de_hl(cpu);_fetch();break;
        
        //  EC: call pe,nn (M:6 T:17)
        // -- mread
        case  822: break;
        case  823: _wait();_mread(cpu->pc++);break;
        case  824: cpu->wzl=_gd();break;
        // -- mread
        case  825: break;
        case  826: _wait();_mread(cpu->pc++);break;
        case  827: cpu->wzh=_gd();if (!_cc_pe){_z80_skip(cpu,7);};break;
        // -- generic
        case  828: break;
        // -- mwrite
        case  829: break;
        case  830: _wait();_mwrite(--cpu->sp,cpu->pch);;break;
        case  831: break;
        // -- mwrite
        case  832: break;
        case  833: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz;;break;
        case  834: break;
        // -- overlapped
        case  835: _fetch();break;
        
        //  ED: ed prefix (M:1 T:4)
        // -- overlapped
        case  836: _fetch_ed();break;
        
        //  EE: xor n (M:2 T:7)
        // -- mread
        case  837: break;
        case  838: _wait();_mread(cpu->pc++);break;
        case  839: cpu->dlatch=_gd();break;
        // -- overlapped
        case  840: _z80_xor8(cpu,cpu->dlatch);_fetch();break;
        
        //  EF: rst 28h (M:4 T:11)
        // -- generic
        case  841: break;
        // -- mwrite
        case  842: break;
        case  843: _wait();_mwrite(--cpu->sp,cpu->pch);;break;
        case  844: break;
        // -- mwrite
        case  845: break;
        case  846: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x28;cpu->pc=cpu->wz;;break;
        case  847: break;
        // -- overlapped
        case  848: _fetch();break;
        
        //  F0: ret p (M:4 T:11)
        // -- generic
        case  849: if(!_cc_p){_z80_skip(cpu,6);};break;
        // -- mread
        case  850: break;
        case  851: _wait();_mread(cpu->sp++);break;
        case  852: cpu->wzl=_gd();break;
        // -- mread
        case  853: break;
        case  854: _wait();_mread(cpu->sp++);break;
        case  855: cpu->wzh=_gd();cpu->pc=cpu->wz;break;
        // -- overlapped
        case  856: _fetch();break;
        
        //  F1: pop af (M:3 T:10)
        // -- mread
        case  857: break;
        case  858: _wait();_mread(cpu->sp++);break;
        case  859: cpu->f=_gd();break;
        // -- mread
        case  860: break;
        case  861: _wait();_mread(cpu->sp++);break;
        case  862: cpu->a=_gd();break;
        // -- overlapped
        case  863: _fetch();break;
        
        //  F2: jp p,nn (M:3 T:10)
        // -- mread
        case  864: break;
        case  865: _wait();_mread(cpu->pc++);break;
        case  866: cpu->wzl=_gd();break;
        // -- mread
        case  867: break;
        case  868: _wait();_mread(cpu->pc++);break;
        case  869: cpu->wzh=_gd();if(_cc_p){cpu->pc=cpu->wz;};break;
        // -- overlapped
        case  870: _fetch();break;
        
        //  F3: di (M:1 T:4)
        // -- overlapped
        case  871: cpu->iff1=cpu->iff2=false;;_fetch();break;
        
        //  F4: call p,nn (M:6 T:17)
        // -- mread
        case  872: break;
        case  873: _wait();_mread(cpu->pc++);break;
        case  874: cpu->wzl=_gd();break;
        // -- mread
        case  875: break;
        case  876: _wait();_mread(cpu->pc++);break;
        case  877: cpu->wzh=_gd();if (!_cc_p){_z80_skip(cpu,7);};break;
        // -- generic
        case  878: break;
        // -- mwrite
        case  879: break;
        case  880: _wait();_mwrite(--cpu->sp,cpu->pch);;break;
        case  881: break;
        // -- mwrite
        case  882: break;
        case  883: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz;;break;
        case  884: break;
        // -- overlapped
        case  885: _fetch();break;
        
        //  F5: push af (M:4 T:11)
        // -- generic
        case  886: break;
        // -- mwrite
        case  887: break;
        case  888: _wait();_mwrite(--cpu->sp,cpu->a);;break;
        case  889: break;
        // -- mwrite
        case  890: break;
        case  891: _wait();_mwrite(--cpu->sp,cpu->f);;break;
        case  892: break;
        // -- overlapped
        case  893: _fetch();break;
        
        //  F6: or n (M:2 T:7)
        // -- mread
        case  894: break;
        case  895: _wait();_mread(cpu->pc++);break;
        case  896: cpu->dlatch=_gd();break;
        // -- overlapped
        case  897: _z80_or8(cpu,cpu->dlatch);_fetch();break;
        
        //  F7: rst 30h (M:4 T:11)
        // -- generic
        case  898: break;
        // -- mwrite
        case  899: break;
        case  900: _wait();_mwrite(--cpu->sp,cpu->pch);;break;
        case  901: break;
        // -- mwrite
        case  902: break;
        case  903: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x30;cpu->pc=cpu->wz;;break;
        case  904: break;
        // -- overlapped
        case  905: _fetch();break;
        
        //  F8: ret m (M:4 T:11)
        // -- generic
        case  906: if(!_cc_m){_z80_skip(cpu,6);};break;
        // -- mread
        case  907: break;
        case  908: _wait();_mread(cpu->sp++);break;
        case  909: cpu->wzl=_gd();break;
        // -- mread
        case  910: break;
        case  911: _wait();_mread(cpu->sp++);break;
        case  912: cpu->wzh=_gd();cpu->pc=cpu->wz;break;
        // -- overlapped
        case  913: _fetch();break;
        
        //  F9: ld sp,hl (M:2 T:6)
        // -- generic
        case  914: cpu->sp=cpu->hlx[cpu->hlx_idx].hl;break;
        case  915: break;
        // -- overlapped
        case  916: _fetch();break;
        
        //  FA: jp m,nn (M:3 T:10)
        // -- mread
        case  917: break;
        case  918: _wait();_mread(cpu->pc++);break;
        case  919: cpu->wzl=_gd();break;
        // -- mread
        case  920: break;
        case  921: _wait();_mread(cpu->pc++);break;
        case  922: cpu->wzh=_gd();if(_cc_m){cpu->pc=cpu->wz;};break;
        // -- overlapped
        case  923: _fetch();break;
        
        //  FB: ei (M:1 T:4)
        // -- overlapped
        case  924: cpu->iff1=cpu->iff2=false;_fetch();cpu->iff1=cpu->iff2=true;break;
        
        //  FC: call m,nn (M:6 T:17)
        // -- mread
        case  925: break;
        case  926: _wait();_mread(cpu->pc++);break;
        case  927: cpu->wzl=_gd();break;
        // -- mread
        case  928: break;
        case  929: _wait();_mread(cpu->pc++);break;
        case  930: cpu->wzh=_gd();if (!_cc_m){_z80_skip(cpu,7);};break;
        // -- generic
        case  931: break;
        // -- mwrite
        case  932: break;
        case  933: _wait();_mwrite(--cpu->sp,cpu->pch);;break;
        case  934: break;
        // -- mwrite
        case  935: break;
        case  936: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->pc=cpu->wz;;break;
        case  937: break;
        // -- overlapped
        case  938: _fetch();break;
        
        //  FD: fd prefix (M:1 T:4)
        // -- overlapped
        case  939: _fetch_fd();break;
        
        //  FE: cp n (M:2 T:7)
        // -- mread
        case  940: break;
        case  941: _wait();_mread(cpu->pc++);break;
        case  942: cpu->dlatch=_gd();break;
        // -- overlapped
        case  943: _z80_cp8(cpu,cpu->dlatch);_fetch();break;
        
        //  FF: rst 38h (M:4 T:11)
        // -- generic
        case  944: break;
        // -- mwrite
        case  945: break;
        case  946: _wait();_mwrite(--cpu->sp,cpu->pch);;break;
        case  947: break;
        // -- mwrite
        case  948: break;
        case  949: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=0x38;cpu->pc=cpu->wz;;break;
        case  950: break;
        // -- overlapped
        case  951: _fetch();break;
        
        // ED 00: ed nop (M:1 T:4)
        // -- overlapped
        case  952: _fetch();break;
        
        // ED 40: in b,(c) (M:2 T:8)
        // -- ioread
        case  953: break;
        case  954: break;
        case  955: _wait();_ioread(cpu->bc);break;
        case  956: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;break;
        // -- overlapped
        case  957: cpu->b=_z80_in(cpu,cpu->dlatch);_fetch();break;
        
        // ED 41: out (c),b (M:2 T:8)
        // -- iowrite
        case  958: break;
        case  959: _iowrite(cpu->bc,cpu->b);break;
        case  960: _wait();cpu->wz=cpu->bc+1;;break;
        case  961: break;
        // -- overlapped
        case  962: _fetch();break;
        
        // ED 42: sbc hl,bc (M:2 T:11)
        // -- generic
        case  963: _z80_sbc16(cpu,cpu->bc);break;
        case  964: break;
        case  965: break;
        case  966: break;
        case  967: break;
        case  968: break;
        case  969: break;
        // -- overlapped
        case  970: _fetch();break;
        
        // ED 43: ld (nn),bc (M:5 T:16)
        // -- mread
        case  971: break;
        case  972: _wait();_mread(cpu->pc++);break;
        case  973: cpu->wzl=_gd();break;
        // -- mread
        case  974: break;
        case  975: _wait();_mread(cpu->pc++);break;
        case  976: cpu->wzh=_gd();break;
        // -- mwrite
        case  977: break;
        case  978: _wait();_mwrite(cpu->wz++,cpu->c);;break;
        case  979: break;
        // -- mwrite
        case  980: break;
        case  981: _wait();_mwrite(cpu->wz,cpu->b);;break;
        case  982: break;
        // -- overlapped
        case  983: _fetch();break;
        
        // ED 44: neg (M:1 T:4)
        // -- overlapped
        case  984: _z80_neg8(cpu);_fetch();break;
        
        // ED 45: reti/retn (M:3 T:10)
        // -- mread
        case  985: break;
        case  986: _wait();_mread(cpu->sp++);break;
        case  987: cpu->wzl=_gd();pins|=Z80_RETI;break;
        // -- mread
        case  988: break;
        case  989: _wait();_mread(cpu->sp++);break;
        case  990: cpu->wzh=_gd();cpu->pc=cpu->wz;break;
        // -- overlapped
        case  991: _fetch();cpu->iff1=cpu->iff2;break;
        
        // ED 46: im 0 (M:1 T:4)
        // -- overlapped
        case  992: cpu->im=0;_fetch();break;
        
        // ED 47: ld i,a (M:2 T:5)
        // -- generic
        case  993: break;
        // -- overlapped
        case  994: cpu->i=cpu->a;_fetch();break;
        
        // ED 48: in c,(c) (M:2 T:8)
        // -- ioread
        case  995: break;
        case  996: break;
        case  997: _wait();_ioread(cpu->bc);break;
        case  998: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;break;
        // -- overlapped
        case  999: cpu->c=_z80_in(cpu,cpu->dlatch);_fetch();break;
        
        // ED 49: out (c),c (M:2 T:8)
        // -- iowrite
        case 1000: break;
        case 1001: _iowrite(cpu->bc,cpu->c);break;
        case 1002: _wait();cpu->wz=cpu->bc+1;;break;
        case 1003: break;
        // -- overlapped
        case 1004: _fetch();break;
        
        // ED 4A: adc hl,bc (M:2 T:11)
        // -- generic
        case 1005: _z80_adc16(cpu,cpu->bc);break;
        case 1006: break;
        case 1007: break;
        case 1008: break;
        case 1009: break;
        case 1010: break;
        case 1011: break;
        // -- overlapped
        case 1012: _fetch();break;
        
        // ED 4B: ld bc,(nn) (M:5 T:16)
        // -- mread
        case 1013: break;
        case 1014: _wait();_mread(cpu->pc++);break;
        case 1015: cpu->wzl=_gd();break;
        // -- mread
        case 1016: break;
        case 1017: _wait();_mread(cpu->pc++);break;
        case 1018: cpu->wzh=_gd();break;
        // -- mread
        case 1019: break;
        case 1020: _wait();_mread(cpu->wz++);break;
        case 1021: cpu->c=_gd();break;
        // -- mread
        case 1022: break;
        case 1023: _wait();_mread(cpu->wz);break;
        case 1024: cpu->b=_gd();break;
        // -- overlapped
        case 1025: _fetch();break;
        
        // ED 4E: im 0 (M:1 T:4)
        // -- overlapped
        case 1026: cpu->im=0;_fetch();break;
        
        // ED 4F: ld r,a (M:2 T:5)
        // -- generic
        case 1027: break;
        // -- overlapped
        case 1028: cpu->r=cpu->a;_fetch();break;
        
        // ED 50: in d,(c) (M:2 T:8)
        // -- ioread
        case 1029: break;
        case 1030: break;
        case 1031: _wait();_ioread(cpu->bc);break;
        case 1032: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;break;
        // -- overlapped
        case 1033: cpu->d=_z80_in(cpu,cpu->dlatch);_fetch();break;
        
        // ED 51: out (c),d (M:2 T:8)
        // -- iowrite
        case 1034: break;
        case 1035: _iowrite(cpu->bc,cpu->d);break;
        case 1036: _wait();cpu->wz=cpu->bc+1;;break;
        case 1037: break;
        // -- overlapped
        case 1038: _fetch();break;
        
        // ED 52: sbc hl,de (M:2 T:11)
        // -- generic
        case 1039: _z80_sbc16(cpu,cpu->de);break;
        case 1040: break;
        case 1041: break;
        case 1042: break;
        case 1043: break;
        case 1044: break;
        case 1045: break;
        // -- overlapped
        case 1046: _fetch();break;
        
        // ED 53: ld (nn),de (M:5 T:16)
        // -- mread
        case 1047: break;
        case 1048: _wait();_mread(cpu->pc++);break;
        case 1049: cpu->wzl=_gd();break;
        // -- mread
        case 1050: break;
        case 1051: _wait();_mread(cpu->pc++);break;
        case 1052: cpu->wzh=_gd();break;
        // -- mwrite
        case 1053: break;
        case 1054: _wait();_mwrite(cpu->wz++,cpu->e);;break;
        case 1055: break;
        // -- mwrite
        case 1056: break;
        case 1057: _wait();_mwrite(cpu->wz,cpu->d);;break;
        case 1058: break;
        // -- overlapped
        case 1059: _fetch();break;
        
        // ED 56: im 1 (M:1 T:4)
        // -- overlapped
        case 1060: cpu->im=1;_fetch();break;
        
        // ED 57: ld a,i (M:2 T:5)
        // -- generic
        case 1061: break;
        // -- overlapped
        case 1062: cpu->a=cpu->i;cpu->f=_z80_sziff2_flags(cpu, cpu->i);_fetch();break;
        
        // ED 58: in e,(c) (M:2 T:8)
        // -- ioread
        case 1063: break;
        case 1064: break;
        case 1065: _wait();_ioread(cpu->bc);break;
        case 1066: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;break;
        // -- overlapped
        case 1067: cpu->e=_z80_in(cpu,cpu->dlatch);_fetch();break;
        
        // ED 59: out (c),e (M:2 T:8)
        // -- iowrite
        case 1068: break;
        case 1069: _iowrite(cpu->bc,cpu->e);break;
        case 1070: _wait();cpu->wz=cpu->bc+1;;break;
        case 1071: break;
        // -- overlapped
        case 1072: _fetch();break;
        
        // ED 5A: adc hl,de (M:2 T:11)
        // -- generic
        case 1073: _z80_adc16(cpu,cpu->de);break;
        case 1074: break;
        case 1075: break;
        case 1076: break;
        case 1077: break;
        case 1078: break;
        case 1079: break;
        // -- overlapped
        case 1080: _fetch();break;
        
        // ED 5B: ld de,(nn) (M:5 T:16)
        // -- mread
        case 1081: break;
        case 1082: _wait();_mread(cpu->pc++);break;
        case 1083: cpu->wzl=_gd();break;
        // -- mread
        case 1084: break;
        case 1085: _wait();_mread(cpu->pc++);break;
        case 1086: cpu->wzh=_gd();break;
        // -- mread
        case 1087: break;
        case 1088: _wait();_mread(cpu->wz++);break;
        case 1089: cpu->e=_gd();break;
        // -- mread
        case 1090: break;
        case 1091: _wait();_mread(cpu->wz);break;
        case 1092: cpu->d=_gd();break;
        // -- overlapped
        case 1093: _fetch();break;
        
        // ED 5E: im 2 (M:1 T:4)
        // -- overlapped
        case 1094: cpu->im=2;_fetch();break;
        
        // ED 5F: ld a,r (M:2 T:5)
        // -- generic
        case 1095: break;
        // -- overlapped
        case 1096: cpu->a=cpu->r;cpu->f=_z80_sziff2_flags(cpu, cpu->r);_fetch();break;
        
        // ED 60: in h,(c) (M:2 T:8)
        // -- ioread
        case 1097: break;
        case 1098: break;
        case 1099: _wait();_ioread(cpu->bc);break;
        case 1100: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;break;
        // -- overlapped
        case 1101: cpu->hlx[cpu->hlx_idx].h=_z80_in(cpu,cpu->dlatch);_fetch();break;
        
        // ED 61: out (c),h (M:2 T:8)
        // -- iowrite
        case 1102: break;
        case 1103: _iowrite(cpu->bc,cpu->hlx[cpu->hlx_idx].h);break;
        case 1104: _wait();cpu->wz=cpu->bc+1;;break;
        case 1105: break;
        // -- overlapped
        case 1106: _fetch();break;
        
        // ED 62: sbc hl,hl (M:2 T:11)
        // -- generic
        case 1107: _z80_sbc16(cpu,cpu->hl);break;
        case 1108: break;
        case 1109: break;
        case 1110: break;
        case 1111: break;
        case 1112: break;
        case 1113: break;
        // -- overlapped
        case 1114: _fetch();break;
        
        // ED 63: ld (nn),hl (M:5 T:16)
        // -- mread
        case 1115: break;
        case 1116: _wait();_mread(cpu->pc++);break;
        case 1117: cpu->wzl=_gd();break;
        // -- mread
        case 1118: break;
        case 1119: _wait();_mread(cpu->pc++);break;
        case 1120: cpu->wzh=_gd();break;
        // -- mwrite
        case 1121: break;
        case 1122: _wait();_mwrite(cpu->wz++,cpu->l);;break;
        case 1123: break;
        // -- mwrite
        case 1124: break;
        case 1125: _wait();_mwrite(cpu->wz,cpu->h);;break;
        case 1126: break;
        // -- overlapped
        case 1127: _fetch();break;
        
        // ED 66: im 0 (M:1 T:4)
        // -- overlapped
        case 1128: cpu->im=0;_fetch();break;
        
        // ED 67: rrd (M:4 T:14)
        // -- mread
        case 1129: break;
        case 1130: _wait();_mread(cpu->hl);break;
        case 1131: cpu->dlatch=_gd();break;
        // -- generic
        case 1132: cpu->dlatch=_z80_rrd(cpu,cpu->dlatch);break;
        case 1133: break;
        case 1134: break;
        case 1135: break;
        // -- mwrite
        case 1136: break;
        case 1137: _wait();_mwrite(cpu->hl,cpu->dlatch);cpu->wz=cpu->hl+1;;break;
        case 1138: break;
        // -- overlapped
        case 1139: _fetch();break;
        
        // ED 68: in l,(c) (M:2 T:8)
        // -- ioread
        case 1140: break;
        case 1141: break;
        case 1142: _wait();_ioread(cpu->bc);break;
        case 1143: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;break;
        // -- overlapped
        case 1144: cpu->hlx[cpu->hlx_idx].l=_z80_in(cpu,cpu->dlatch);_fetch();break;
        
        // ED 69: out (c),l (M:2 T:8)
        // -- iowrite
        case 1145: break;
        case 1146: _iowrite(cpu->bc,cpu->hlx[cpu->hlx_idx].l);break;
        case 1147: _wait();cpu->wz=cpu->bc+1;;break;
        case 1148: break;
        // -- overlapped
        case 1149: _fetch();break;
        
        // ED 6A: adc hl,hl (M:2 T:11)
        // -- generic
        case 1150: _z80_adc16(cpu,cpu->hl);break;
        case 1151: break;
        case 1152: break;
        case 1153: break;
        case 1154: break;
        case 1155: break;
        case 1156: break;
        // -- overlapped
        case 1157: _fetch();break;
        
        // ED 6B: ld hl,(nn) (M:5 T:16)
        // -- mread
        case 1158: break;
        case 1159: _wait();_mread(cpu->pc++);break;
        case 1160: cpu->wzl=_gd();break;
        // -- mread
        case 1161: break;
        case 1162: _wait();_mread(cpu->pc++);break;
        case 1163: cpu->wzh=_gd();break;
        // -- mread
        case 1164: break;
        case 1165: _wait();_mread(cpu->wz++);break;
        case 1166: cpu->l=_gd();break;
        // -- mread
        case 1167: break;
        case 1168: _wait();_mread(cpu->wz);break;
        case 1169: cpu->h=_gd();break;
        // -- overlapped
        case 1170: _fetch();break;
        
        // ED 6E: im 0 (M:1 T:4)
        // -- overlapped
        case 1171: cpu->im=0;_fetch();break;
        
        // ED 6F: rld (M:4 T:14)
        // -- mread
        case 1172: break;
        case 1173: _wait();_mread(cpu->hl);break;
        case 1174: cpu->dlatch=_gd();break;
        // -- generic
        case 1175: cpu->dlatch=_z80_rld(cpu,cpu->dlatch);break;
        case 1176: break;
        case 1177: break;
        case 1178: break;
        // -- mwrite
        case 1179: break;
        case 1180: _wait();_mwrite(cpu->hl,cpu->dlatch);cpu->wz=cpu->hl+1;;break;
        case 1181: break;
        // -- overlapped
        case 1182: _fetch();break;
        
        // ED 70: in (c) (M:2 T:8)
        // -- ioread
        case 1183: break;
        case 1184: break;
        case 1185: _wait();_ioread(cpu->bc);break;
        case 1186: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;break;
        // -- overlapped
        case 1187: _z80_in(cpu,cpu->dlatch);_fetch();break;
        
        // ED 71: out (c),0 (M:2 T:8)
        // -- iowrite
        case 1188: break;
        case 1189: _iowrite(cpu->bc,0);break;
        case 1190: _wait();cpu->wz=cpu->bc+1;;break;
        case 1191: break;
        // -- overlapped
        case 1192: _fetch();break;
        
        // ED 72: sbc hl,sp (M:2 T:11)
        // -- generic
        case 1193: _z80_sbc16(cpu,cpu->sp);break;
        case 1194: break;
        case 1195: break;
        case 1196: break;
        case 1197: break;
        case 1198: break;
        case 1199: break;
        // -- overlapped
        case 1200: _fetch();break;
        
        // ED 73: ld (nn),sp (M:5 T:16)
        // -- mread
        case 1201: break;
        case 1202: _wait();_mread(cpu->pc++);break;
        case 1203: cpu->wzl=_gd();break;
        // -- mread
        case 1204: break;
        case 1205: _wait();_mread(cpu->pc++);break;
        case 1206: cpu->wzh=_gd();break;
        // -- mwrite
        case 1207: break;
        case 1208: _wait();_mwrite(cpu->wz++,cpu->spl);;break;
        case 1209: break;
        // -- mwrite
        case 1210: break;
        case 1211: _wait();_mwrite(cpu->wz,cpu->sph);;break;
        case 1212: break;
        // -- overlapped
        case 1213: _fetch();break;
        
        // ED 76: im 1 (M:1 T:4)
        // -- overlapped
        case 1214: cpu->im=1;_fetch();break;
        
        // ED 78: in a,(c) (M:2 T:8)
        // -- ioread
        case 1215: break;
        case 1216: break;
        case 1217: _wait();_ioread(cpu->bc);break;
        case 1218: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;break;
        // -- overlapped
        case 1219: cpu->a=_z80_in(cpu,cpu->dlatch);_fetch();break;
        
        // ED 79: out (c),a (M:2 T:8)
        // -- iowrite
        case 1220: break;
        case 1221: _iowrite(cpu->bc,cpu->a);break;
        case 1222: _wait();cpu->wz=cpu->bc+1;;break;
        case 1223: break;
        // -- overlapped
        case 1224: _fetch();break;
        
        // ED 7A: adc hl,sp (M:2 T:11)
        // -- generic
        case 1225: _z80_adc16(cpu,cpu->sp);break;
        case 1226: break;
        case 1227: break;
        case 1228: break;
        case 1229: break;
        case 1230: break;
        case 1231: break;
        // -- overlapped
        case 1232: _fetch();break;
        
        // ED 7B: ld sp,(nn) (M:5 T:16)
        // -- mread
        case 1233: break;
        case 1234: _wait();_mread(cpu->pc++);break;
        case 1235: cpu->wzl=_gd();break;
        // -- mread
        case 1236: break;
        case 1237: _wait();_mread(cpu->pc++);break;
        case 1238: cpu->wzh=_gd();break;
        // -- mread
        case 1239: break;
        case 1240: _wait();_mread(cpu->wz++);break;
        case 1241: cpu->spl=_gd();break;
        // -- mread
        case 1242: break;
        case 1243: _wait();_mread(cpu->wz);break;
        case 1244: cpu->sph=_gd();break;
        // -- overlapped
        case 1245: _fetch();break;
        
        // ED 7E: im 2 (M:1 T:4)
        // -- overlapped
        case 1246: cpu->im=2;_fetch();break;
        
        // ED A0: ldi (M:4 T:12)
        // -- mread
        case 1247: break;
        case 1248: _wait();_mread(cpu->hl++);break;
        case 1249: cpu->dlatch=_gd();break;
        // -- mwrite
        case 1250: break;
        case 1251: _wait();_mwrite(cpu->de++,cpu->dlatch);;break;
        case 1252: break;
        // -- generic
        case 1253: _z80_ldi_ldd(cpu,cpu->dlatch);break;
        case 1254: break;
        // -- overlapped
        case 1255: _fetch();break;
        
        // ED A1: cpi (M:3 T:12)
        // -- mread
        case 1256: break;
        case 1257: _wait();_mread(cpu->hl++);break;
        case 1258: cpu->dlatch=_gd();break;
        // -- generic
        case 1259: cpu->wz++;_z80_cpi_cpd(cpu,cpu->dlatch);break;
        case 1260: break;
        case 1261: break;
        case 1262: break;
        case 1263: break;
        // -- overlapped
        case 1264: _fetch();break;
        
        // ED A2: ini (M:4 T:12)
        // -- generic
        case 1265: break;
        // -- ioread
        case 1266: break;
        case 1267: break;
        case 1268: _wait();_ioread(cpu->bc);break;
        case 1269: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;cpu->b--;;break;
        // -- mwrite
        case 1270: break;
        case 1271: _wait();_mwrite(cpu->hl++,cpu->dlatch);_z80_ini_ind(cpu,cpu->dlatch,cpu->c+1);;break;
        case 1272: break;
        // -- overlapped
        case 1273: _fetch();break;
        
        // ED A3: outi (M:4 T:12)
        // -- generic
        case 1274: break;
        // -- mread
        case 1275: break;
        case 1276: _wait();_mread(cpu->hl++);break;
        case 1277: cpu->dlatch=_gd();cpu->b--;break;
        // -- iowrite
        case 1278: break;
        case 1279: _iowrite(cpu->bc,cpu->dlatch);break;
        case 1280: _wait();cpu->wz=cpu->bc+1;_z80_outi_outd(cpu,cpu->dlatch);;break;
        case 1281: break;
        // -- overlapped
        case 1282: _fetch();break;
        
        // ED A8: ldd (M:4 T:12)
        // -- mread
        case 1283: break;
        case 1284: _wait();_mread(cpu->hl--);break;
        case 1285: cpu->dlatch=_gd();break;
        // -- mwrite
        case 1286: break;
        case 1287: _wait();_mwrite(cpu->de--,cpu->dlatch);;break;
        case 1288: break;
        // -- generic
        case 1289: _z80_ldi_ldd(cpu,cpu->dlatch);break;
        case 1290: break;
        // -- overlapped
        case 1291: _fetch();break;
        
        // ED A9: cpd (M:3 T:12)
        // -- mread
        case 1292: break;
        case 1293: _wait();_mread(cpu->hl--);break;
        case 1294: cpu->dlatch=_gd();break;
        // -- generic
        case 1295: cpu->wz--;_z80_cpi_cpd(cpu,cpu->dlatch);break;
        case 1296: break;
        case 1297: break;
        case 1298: break;
        case 1299: break;
        // -- overlapped
        case 1300: _fetch();break;
        
        // ED AA: ind (M:4 T:12)
        // -- generic
        case 1301: break;
        // -- ioread
        case 1302: break;
        case 1303: break;
        case 1304: _wait();_ioread(cpu->bc);break;
        case 1305: cpu->dlatch=_gd();cpu->wz=cpu->bc-1;cpu->b--;;break;
        // -- mwrite
        case 1306: break;
        case 1307: _wait();_mwrite(cpu->hl--,cpu->dlatch);_z80_ini_ind(cpu,cpu->dlatch,cpu->c-1);;break;
        case 1308: break;
        // -- overlapped
        case 1309: _fetch();break;
        
        // ED AB: outd (M:4 T:12)
        // -- generic
        case 1310: break;
        // -- mread
        case 1311: break;
        case 1312: _wait();_mread(cpu->hl--);break;
        case 1313: cpu->dlatch=_gd();cpu->b--;break;
        // -- iowrite
        case 1314: break;
        case 1315: _iowrite(cpu->bc,cpu->dlatch);break;
        case 1316: _wait();cpu->wz=cpu->bc-1;_z80_outi_outd(cpu,cpu->dlatch);;break;
        case 1317: break;
        // -- overlapped
        case 1318: _fetch();break;
        
        // ED B0: ldir (M:5 T:17)
        // -- mread
        case 1319: break;
        case 1320: _wait();_mread(cpu->hl++);break;
        case 1321: cpu->dlatch=_gd();break;
        // -- mwrite
        case 1322: break;
        case 1323: _wait();_mwrite(cpu->de++,cpu->dlatch);;break;
        case 1324: break;
        // -- generic
        case 1325: if(!_z80_ldi_ldd(cpu,cpu->dlatch)){_z80_skip(cpu,5);};break;
        case 1326: break;
        // -- generic
        case 1327: cpu->wz=--cpu->pc;--cpu->pc;;break;
        case 1328: break;
        case 1329: break;
        case 1330: break;
        case 1331: break;
        // -- overlapped
        case 1332: _fetch();break;
        
        // ED B1: cpir (M:4 T:17)
        // -- mread
        case 1333: break;
        case 1334: _wait();_mread(cpu->hl++);break;
        case 1335: cpu->dlatch=_gd();break;
        // -- generic
        case 1336: cpu->wz++;if(!_z80_cpi_cpd(cpu,cpu->dlatch)){_z80_skip(cpu,5);};break;
        case 1337: break;
        case 1338: break;
        case 1339: break;
        case 1340: break;
        // -- generic
        case 1341: cpu->wz=--cpu->pc;--cpu->pc;break;
        case 1342: break;
        case 1343: break;
        case 1344: break;
        case 1345: break;
        // -- overlapped
        case 1346: _fetch();break;
        
        // ED B2: inir (M:5 T:17)
        // -- generic
        case 1347: break;
        // -- ioread
        case 1348: break;
        case 1349: break;
        case 1350: _wait();_ioread(cpu->bc);break;
        case 1351: cpu->dlatch=_gd();cpu->wz=cpu->bc+1;cpu->b--;;break;
        // -- mwrite
        case 1352: break;
        case 1353: _wait();_mwrite(cpu->hl++,cpu->dlatch);if (!_z80_ini_ind(cpu,cpu->dlatch,cpu->c+1)){_z80_skip(cpu,5);};;break;
        case 1354: break;
        // -- generic
        case 1355: cpu->wz=--cpu->pc;--cpu->pc;break;
        case 1356: break;
        case 1357: break;
        case 1358: break;
        case 1359: break;
        // -- overlapped
        case 1360: _fetch();break;
        
        // ED B3: otir (M:5 T:17)
        // -- generic
        case 1361: break;
        // -- mread
        case 1362: break;
        case 1363: _wait();_mread(cpu->hl++);break;
        case 1364: cpu->dlatch=_gd();cpu->b--;break;
        // -- iowrite
        case 1365: break;
        case 1366: _iowrite(cpu->bc,cpu->dlatch);break;
        case 1367: _wait();cpu->wz=cpu->bc+1;if(!_z80_outi_outd(cpu,cpu->dlatch)){_z80_skip(cpu,5);};;break;
        case 1368: break;
        // -- generic
        case 1369: cpu->wz=--cpu->pc;--cpu->pc;break;
        case 1370: break;
        case 1371: break;
        case 1372: break;
        case 1373: break;
        // -- overlapped
        case 1374: _fetch();break;
        
        // ED B8: lddr (M:5 T:17)
        // -- mread
        case 1375: break;
        case 1376: _wait();_mread(cpu->hl--);break;
        case 1377: cpu->dlatch=_gd();break;
        // -- mwrite
        case 1378: break;
        case 1379: _wait();_mwrite(cpu->de--,cpu->dlatch);;break;
        case 1380: break;
        // -- generic
        case 1381: if(!_z80_ldi_ldd(cpu,cpu->dlatch)){_z80_skip(cpu,5);};break;
        case 1382: break;
        // -- generic
        case 1383: cpu->wz=--cpu->pc;--cpu->pc;;break;
        case 1384: break;
        case 1385: break;
        case 1386: break;
        case 1387: break;
        // -- overlapped
        case 1388: _fetch();break;
        
        // ED B9: cpdr (M:4 T:17)
        // -- mread
        case 1389: break;
        case 1390: _wait();_mread(cpu->hl--);break;
        case 1391: cpu->dlatch=_gd();break;
        // -- generic
        case 1392: cpu->wz--;if(!_z80_cpi_cpd(cpu,cpu->dlatch)){_z80_skip(cpu,5);};break;
        case 1393: break;
        case 1394: break;
        case 1395: break;
        case 1396: break;
        // -- generic
        case 1397: cpu->wz=--cpu->pc;--cpu->pc;break;
        case 1398: break;
        case 1399: break;
        case 1400: break;
        case 1401: break;
        // -- overlapped
        case 1402: _fetch();break;
        
        // ED BA: indr (M:5 T:17)
        // -- generic
        case 1403: break;
        // -- ioread
        case 1404: break;
        case 1405: break;
        case 1406: _wait();_ioread(cpu->bc);break;
        case 1407: cpu->dlatch=_gd();cpu->wz=cpu->bc-1;cpu->b--;;break;
        // -- mwrite
        case 1408: break;
        case 1409: _wait();_mwrite(cpu->hl--,cpu->dlatch);if (!_z80_ini_ind(cpu,cpu->dlatch,cpu->c-1)){_z80_skip(cpu,5);};;break;
        case 1410: break;
        // -- generic
        case 1411: cpu->wz=--cpu->pc;--cpu->pc;break;
        case 1412: break;
        case 1413: break;
        case 1414: break;
        case 1415: break;
        // -- overlapped
        case 1416: _fetch();break;
        
        // ED BB: otdr (M:5 T:17)
        // -- generic
        case 1417: break;
        // -- mread
        case 1418: break;
        case 1419: _wait();_mread(cpu->hl--);break;
        case 1420: cpu->dlatch=_gd();cpu->b--;break;
        // -- iowrite
        case 1421: break;
        case 1422: _iowrite(cpu->bc,cpu->dlatch);break;
        case 1423: _wait();cpu->wz=cpu->bc-1;if(!_z80_outi_outd(cpu,cpu->dlatch)){_z80_skip(cpu,5);};;break;
        case 1424: break;
        // -- generic
        case 1425: cpu->wz=--cpu->pc;--cpu->pc;break;
        case 1426: break;
        case 1427: break;
        case 1428: break;
        case 1429: break;
        // -- overlapped
        case 1430: _fetch();break;
        
        // CB 00: cb (M:1 T:4)
        // -- overlapped
        case 1431: {uint8_t z=cpu->opcode&7;_z80_cb_action(cpu,z,z);};_fetch();break;
        
        // CB 00: cbhl (M:3 T:11)
        // -- mread
        case 1432: break;
        case 1433: _wait();_mread(cpu->hl);break;
        case 1434: cpu->dlatch=_gd();if(!_z80_cb_action(cpu,6,6)){_z80_skip(cpu,3);};break;
        case 1435: break;
        // -- mwrite
        case 1436: break;
        case 1437: _wait();_mwrite(cpu->hl,cpu->dlatch);;break;
        case 1438: break;
        // -- overlapped
        case 1439: _fetch();break;
        
        // CB 00: ddfdcb (M:6 T:18)
        // -- generic
        case 1440: _wait();;break;
        // -- generic
        case 1441: _z80_ddfdcb_addr(cpu, pins);break;
        // -- mread
        case 1442: break;
        case 1443: _wait();_mread(cpu->pc++);break;
        case 1444: cpu->dlatch=_gd();_z80_ddfdcb_opcode(cpu,cpu->dlatch);break;
        case 1445: break;
        case 1446: break;
        // -- mread
        case 1447: break;
        case 1448: _wait();_mread(cpu->addr);break;
        case 1449: cpu->dlatch=_gd();if(!_z80_cb_action(cpu,6,cpu->opcode&7)){_z80_skip(cpu,3);};break;
        case 1450: break;
        // -- mwrite
        case 1451: break;
        case 1452: _wait();_mwrite(cpu->addr,cpu->dlatch);;break;
        case 1453: break;
        // -- overlapped
        case 1454: _fetch();break;
        
        //  00: int_im0 (M:5 T:9)
        // -- generic
        case 1455: pins=_z80_int012_step0(cpu,pins);break;
        // -- generic
        case 1456: pins=_z80_int012_step1(cpu,pins);break;
        // -- generic
        case 1457: _wait();pins=_z80_int0_step2(cpu,pins);break;
        // -- generic
        case 1458: pins=_z80_int0_step3(cpu,pins);break;
        case 1459: break;
        // -- overlapped
        case 1460: _fetch();break;
        
        //  00: int_im1 (M:7 T:16)
        // -- generic
        case 1461: pins=_z80_int012_step0(cpu,pins);break;
        // -- generic
        case 1462: pins=_z80_int012_step1(cpu,pins);break;
        // -- generic
        case 1463: _wait();;break;
        // -- generic
        case 1464: pins=_z80_refresh(cpu,pins);break;
        case 1465: break;
        case 1466: break;
        // -- mwrite
        case 1467: break;
        case 1468: _wait();_mwrite(--cpu->sp,cpu->pch);;break;
        case 1469: break;
        // -- mwrite
        case 1470: break;
        case 1471: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=cpu->pc=0x0038;;break;
        case 1472: break;
        // -- overlapped
        case 1473: _fetch();break;
        
        //  00: int_im2 (M:9 T:22)
        // -- generic
        case 1474: pins=_z80_int012_step0(cpu,pins);break;
        // -- generic
        case 1475: pins=_z80_int012_step1(cpu,pins);break;
        // -- generic
        case 1476: _wait();cpu->dlatch=_z80_get_db(pins);break;
        // -- generic
        case 1477: pins=_z80_refresh(cpu,pins);break;
        case 1478: break;
        case 1479: break;
        // -- mwrite
        case 1480: break;
        case 1481: _wait();_mwrite(--cpu->sp,cpu->pch);;break;
        case 1482: break;
        // -- mwrite
        case 1483: break;
        case 1484: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wzl=cpu->dlatch;cpu->wzh=cpu->i;;;break;
        case 1485: break;
        // -- mread
        case 1486: break;
        case 1487: _wait();_mread(cpu->wz++);break;
        case 1488: cpu->dlatch=_gd();break;
        // -- mread
        case 1489: break;
        case 1490: _wait();_mread(cpu->wz);break;
        case 1491: cpu->wzh=_gd();cpu->wzl=cpu->dlatch;cpu->pc=cpu->wz;break;
        // -- overlapped
        case 1492: _fetch();break;
        
        //  00: nmi (M:5 T:14)
        // -- generic
        case 1493: pins=_z80_nmi_step0(cpu,pins);break;
        // -- generic
        case 1494: pins=_z80_refresh(cpu,pins);break;
        case 1495: break;
        case 1496: break;
        // -- mwrite
        case 1497: break;
        case 1498: _wait();_mwrite(--cpu->sp,cpu->pch);;break;
        case 1499: break;
        // -- mwrite
        case 1500: break;
        case 1501: _wait();_mwrite(--cpu->sp,cpu->pcl);cpu->wz=cpu->pc=0x0066;;break;
        case 1502: break;
        // -- overlapped
        case 1503: _fetch();break;

    }
    cpu->op.step += 1;
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
