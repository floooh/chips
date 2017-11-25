#pragma once
/*
    z80.h -- Z80 CPU emulator

             +-----------+
    M1      -|           |- A0
    MREQ    -|           |- A1
    IORQ    -|           |- A2
    RD      -|           |- ...
    WR      -|           |- A15
    RFSH    -|           |
    HALT    -|           |
    WAIT    -|    Z80    |- D0
    INT     -|           |- D1
    NMI     -|           |- D2
    RESET   -|           |- ...
    BUSREQ  -|           |- D7
    BUSACK  -|           |
    CLK     -|           |
    +5V     -|           |
    GND     -|           |
             +-----------+

    Decoding Z80 instructions: http://z80.info/decoding.htm
*/
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* status indicator flags */
typedef enum {
    Z80_CF = (1<<0),        /* carry */
    Z80_NF = (1<<1),        /* add/subtract */
    Z80_VF = (1<<2),        /* parity/overflow */
    Z80_PF = Z80_VF,
    Z80_XF = (1<<3),        /* undocumented bit 3 */
    Z80_HF = (1<<4),        /* half carry */
    Z80_YF = (1<<5),        /* undocumented bit 5 */
    Z80_ZF = (1<<6),        /* zero */
    Z80_SF = (1<<7),        /* sign */
} z80_flags;

/* pin functions */
typedef enum {
    /* system control pins */
    Z80_M1    = (1<<0),          /* machine cycle 1 */
    Z80_MREQ  = (1<<1),          /* memory request */
    Z80_IORQ  = (1<<2),          /* input/output request */
    Z80_RD    = (1<<3),          /* read */
    Z80_WR    = (1<<4),          /* write */
    Z80_RFSH  = (1<<5),          /* refresh */

    /* CPU control pins */
    Z80_HALT  = (1<<6),          /* halt state */
    Z80_WAIT  = (1<<7),          /* wait state */
    Z80_INT   = (1<<8),          /* interrupt request */
    Z80_NMI   = (1<<9),          /* non-maskable interrupt */
    Z80_RESET = (1<<10),         /* reset */

    /* CPU bus control pins */
    Z80_BUSREQ = (1<<11),        /* bus request */
    Z80_BUSACK = (1<<12),        /* bus acknowledge */
} z80_pins;

/* Z80 CPU state */
typedef struct _z80 z80;
typedef struct _z80 {
    /* control pins (see z80_pins enum) */
    uint16_t CTRL;
    /* 16-bit address bus */
    uint16_t ADDR;
    /* 8-bit data bus */
    uint8_t DATA;
    /* program counter */
    uint16_t PC;
    /* NOTE: union layout assumes little-endian CPU */
    union { uint16_t AF; struct { uint8_t F, A; }; };
    union { uint16_t HL; struct { uint8_t L, H; }; };
    union { uint16_t IX; struct { uint8_t IXL, IXH; }; };
    union { uint16_t IY; struct { uint8_t IYL, IYH; }; };
    union { uint16_t BC; struct { uint8_t C, B; }; };
    union { uint16_t DE; struct { uint8_t E, D; }; };
    union { uint16_t WZ; struct { uint8_t Z, W; }; };
    union { uint16_t IR; struct { uint8_t R, I; }; };
    /* alternate register set (there is no WZ' register!) */
    uint16_t BC_, DE_, HL_, AF_;
    /* stack pointer */
    uint16_t SP;
    /* interrupt mode (0, 1 or 2) */
    uint8_t IM;
    /* interrupt enable bits */
    bool IFF1, IFF2;

    /* enable-interrupt pending for start of next instruction */
    bool ei_pending;

    /* tick function and context data */
    void (*tick)(z80* cpu);
    /* user-provided context pointer */
    void* context;
    /* flag lookup table for SZP flag combinations */
    uint8_t szp[256];
} z80;

typedef struct {
    void* tick_context;
    void (*tick_func)(z80* cpu);
} z80_desc;

/* initialize a new z80 instance */
extern void z80_init(z80* cpu, z80_desc* desc);
/* reset an existing z80 instance */
extern void z80_reset(z80* cpu);
/* execute the next instruction, return number of time cycles */
extern uint32_t z80_step(z80* cpu);
/* execute instructions for up to 'ticks' time cycles, return executed time cycles */
extern uint32_t z80_run(z80* cpu, uint32_t ticks);

/*-- IMPLEMENTATION ----------------------------------------------------------*/
#ifdef CHIPS_IMPL
#include <string.h>
#ifndef CHIPS_DEBUG
    #ifdef _DEBUG
        #define CHIPS_DEBUG
    #endif
#endif
#ifndef CHIPS_ASSERT
    #include <assert.h>
    #define CHIPS_ASSERT(c) assert(c)
#endif

#ifdef _SZ
#undef _SZ
#endif
#ifdef _SZYXCH
#undef _SZYXCH
#endif
#ifdef _ADD_FLAGS
#undef _ADD_FLAGS
#endif
#ifdef _SUB_FLAGS
#undef _SUB_FLAGS
#endif
#ifdef _CP_FLAGS
#undef _CP_FLAGS
#endif
#ifdef _ON
#undef _ON
#endif
#ifdef _OFF
#undef _OFF
#endif
#ifdef _T
#undef _T
#endif
#ifdef _RD
#undef _RD
#endif
#ifdef _RDS
#undef _RDS
#endif
#ifdef _WR
#undef _WR
#endif
#ifndef _IN
#undef _IN
#endif
#ifndef _OUT
#undef _OUT
#endif
#ifdef _SWP16
#undef _SWP16
#endif
#define _SZ(val) ((val&0xFF)?(val&Z80_SF):Z80_ZF)
#define _SZYXCH(acc,val,res) (_SZ(res)|(res&(Z80_YF|Z80_XF))|((res>>8)&Z80_CF)|((acc^val^res)&Z80_HF))
#define _ADD_FLAGS(acc,val,res) (_SZYXCH(acc,val,res)|((((val^acc^0x80)&(val^res))>>5)&Z80_VF))
#define _SUB_FLAGS(acc,val,res) (Z80_NF|_SZYXCH(acc,val,res)|((((val^acc)&(res^acc))>>5)&Z80_VF))
#define _CP_FLAGS(acc,val,res) (Z80_NF|(_SZ(res)|(val&(Z80_YF|Z80_XF))|((res>>8)&Z80_CF)|((acc^val^res)&Z80_HF))|((((val^acc)&(res^acc))>>5)&Z80_VF))
#define _ON(m) {c->CTRL|=(m);}
#define _OFF(m) {c->CTRL&=~(m);}
#define _T() {tick(c);ticks++;}
#define _SWP16(a,b) {uint16_t tmp=a;a=b;b=tmp;}

#define _RD(addr,res){\
    c->ADDR=addr;\
    _T();\
    _ON(Z80_MREQ|Z80_RD);\
    _T();\
    _OFF(Z80_MREQ|Z80_RD);\
    _T();\
    res=c->DATA;}
/*
    a memory write machine cycle, place 16-bit address into ADDR, place 8-bit
    value into DATA, and then memory[ADDR] = DATA

              T1   T2   T3
    --------+----+----+----+
    CLK     |--**|--**|--**|
    A15-A0  |   MEM ADDR   |
    MREQ    |   *|****|**  |
    RD      |    |    |    |
    WR      |    |  **|**  |
    D7-D0   |   X|XXXX|XXXX|
    WAIT    |    | -- |    |
*/
#define _WR(addr,data){\
    c->ADDR=addr;\
    _T();\
    _ON(Z80_MREQ|Z80_WR);\
    c->DATA=data;\
    _T();\
    _OFF(Z80_MREQ|Z80_WR);\
    _T();}

/*
    an IO input machine cycle, place device address in ADDR, read byte into DATA

              T1   T2   TW   T3
    --------+----+----+----+----+
    CLK     |--**|--**|--**|--**|
    A15-A0  |     PORT ADDR     |
    IORQ    |    |****|****|**  |
    RD      |    |****|****|**  |
    WR      |    |    |    |    |
    D7-D0   |    |    |    | X  |
    WAIT    |    |    | -- |    |

    NOTE: the IORQ|RD pins will already be switched off at the beginning
    of TW, so that IO devices don't need to do double work.
*/
#define _IN(addr,res){\
    c->ADDR=addr;\
    _T();\
    _ON(Z80_IORQ|Z80_RD);\
    _T();\
    _OFF(Z80_IORQ|Z80_RD);\
    _T();\
    _T();\
    res=c->DATA;}

/*
    an IO output machine cycle, place device address in ADDR and data in DATA

              T1   T2   TW   T3
    --------+----+----+----+----+
    CLK     |--**|--**|--**|--**|
    A15-A0  |     PORT ADDR     |
    IORQ    |    |****|****|**  |
    RD      |    |    |    |    |
    WR      |    |****|****|**  |
    D7-D0   |  XX|XXXX|XXXX|XXXX|
    WAIT    |    |    | -- |    |

    NOTE: the IORQ|WR pins will already be switched off at the beginning
    of TW, so that IO devices don't need to do double work.
*/
#define _OUT(addr, data){\
    c->ADDR=addr;\
    _T();\
    _ON(Z80_IORQ|Z80_WR);\
    c->DATA=data;\
    _T();\
    _OFF(Z80_IORQ|Z80_WR);\
    _T();\
    _T();}

/*-- ALU FUNCTIONS -----------------------------------------------------------*/
static void _z80_add(z80* c, uint8_t val) {
    int res = c->A + val;
    c->F = _ADD_FLAGS(c->A, val, res);
    c->A = (uint8_t) res;
}

static void _z80_adc(z80* c, uint8_t val) {
    int res = c->A + val + (c->F & Z80_CF);
    c->F = _ADD_FLAGS(c->A, val, res);
    c->A = (uint8_t) res;
}

static void _z80_sub(z80* c, uint8_t val) {
    int res = (int)c->A - (int)val;
    c->F = _SUB_FLAGS(c->A, val, res);
    c->A = (uint8_t) res;
}

static void _z80_sbc(z80* c, uint8_t val) {
    int res = (int)c->A - (int)val - (c->F & Z80_CF);
    c->F = _SUB_FLAGS(c->A, val, res);
    c->A = (uint8_t) res;
}

static void _z80_cp(z80* c, uint8_t val) {
    /* NOTE: XF|YF are set from val, not from result */
    int res = (int)c->A - (int)val;
    c->F = _CP_FLAGS(c->A, val, res);
}

static void _z80_and(z80* c, uint8_t val) {
    c->A &= val;
    c->F = c->szp[c->A]|Z80_HF;
}

static void _z80_or(z80* c, uint8_t val) {
    c->A |= val;
    c->F = c->szp[c->A];
}

static void _z80_xor(z80* c, uint8_t val) {
    c->A ^= val;
    c->F = c->szp[c->A];
}

static void _z80_neg(z80* c) {
    uint8_t val = c->A;
    c->A = 0;
    _z80_sub(c, val);
}

static uint8_t _z80_inc(z80* c, uint8_t val) {
    uint8_t r = val + 1;
    uint8_t f = _SZ(r)|(r&(Z80_XF|Z80_YF))|((r^val)&Z80_HF);
    if (r == 0x80) f |= Z80_VF;
    c->F = f | (c->F & Z80_CF);
    return r;
}

static uint8_t _z80_dec(z80* c, uint8_t val) {
    uint8_t r = val - 1;
    uint8_t f = Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^val)&Z80_HF);
    if (r == 0x7F) f |= Z80_VF;
    c->F = f | (c->F & Z80_CF);
    return r;
}

/*-- ROTATE AND SHIFT FUNCTIONS ----------------------------------------------*/
static void _z80_rlca(z80* c) {
    uint8_t r = c->A<<1 | c->A>>7;
    c->F = (c->A>>7 & Z80_CF)|(c->F & (Z80_SF|Z80_ZF|Z80_PF))|(r & (Z80_XF|Z80_YF));
    c->A = r;
}

static void _z80_rrca(z80* c) {
    uint8_t r = c->A>>1 | c->A<<7;
    c->F = (c->A & Z80_CF)|(c->F & (Z80_SF|Z80_ZF|Z80_PF))|(r & (Z80_YF|Z80_XF));
    c->A = r;
}

static void _z80_rla(z80* c) {
    uint8_t r = c->A<<1 | (c->F & Z80_CF);
    c->F = (c->A>>7 & Z80_CF) | (c->F & (Z80_SF|Z80_ZF|Z80_PF)) | (r & (Z80_YF|Z80_XF));
    c->A = r;
}

static void _z80_rra(z80* c) {
    uint8_t r = c->A>>1 | ((c->F & Z80_CF)<<7);
    c->F = (c->A & Z80_CF) | (c->F & (Z80_SF|Z80_ZF|Z80_PF)) | (r & (Z80_YF|Z80_XF));
    c->A = r;
}

static uint8_t _z80_rlc(z80* c, uint8_t val) {
    uint8_t r = val<<1 | val>>7;
    c->F = c->szp[r] | (val>>7 & Z80_CF);
    return r;
}

static uint8_t _z80_rrc(z80* c, uint8_t val) {
    uint8_t r = val>>1 | val<<7;
    c->F = c->szp[r] | (val & Z80_CF);
    return r;
}

static uint8_t _z80_rl(z80* c, uint8_t val) {
    uint8_t r = val<<1 | (c->F & Z80_CF);
    c->F = (val>>7 & Z80_CF) | c->szp[r];
    return r;
}

static uint8_t _z80_rr(z80* c, uint8_t val) {
    uint8_t r = val>>1 | ((c->F & Z80_CF)<<7);
    c->F = (val & Z80_CF) | c->szp[r];
    return r;
}

static uint8_t _z80_sla(z80* c, uint8_t val) {
    uint8_t r = val<<1;
    c->F = (val>>7 & Z80_CF) | c->szp[r];
    return r;
}

static uint8_t _z80_sra(z80* c, uint8_t val) {
    uint8_t r = val>>1 | (val & 0x80);
    c->F = (val & Z80_CF) | c->szp[r];
    return r;
}

static uint8_t _z80_sll(z80* c, uint8_t val) {
    // undocument! sll8 is identical with sla8 but inserts a 1 into the LSB
    uint8_t r = (val<<1) | 1;
    c->F = (val>>7 & Z80_CF) | c->szp[r];
    return r;
}

static uint8_t _z80_srl(z80* c, uint8_t val) {
    uint8_t r = val>>1;
    c->F = (val & Z80_CF) | c->szp[r];
    return r;
}

/*-- CONTROL FLOW FUNCTIONS --------------------------------------------------*/
static uint32_t _z80_djnz(z80* c, void(*tick)(z80*), uint32_t ticks) {
    _T();
    int8_t d; _RD(c->PC++, d);
    if (--c->B > 0) {
        c->WZ = c->PC = c->PC + d;
        _T(); _T(); _T(); _T(); _T();
    }
    return ticks;
}

static uint32_t _z80_jr(z80* c, void(*tick)(z80*), uint32_t ticks) {
    int8_t d; _RD(c->PC++, d);
    c->WZ = c->PC + d;
    c->PC = c->WZ;
    _T(); _T(); _T(); _T(); _T();
    return ticks;
}

static uint32_t _z80_jr_cc(z80* c, bool cond, void(*tick)(z80*), uint32_t ticks) {
    int8_t d; _RD(c->PC++, d);
    if (cond) {
        c->WZ = c->PC = c->PC + d;
        _T(); _T(); _T(); _T(); _T();
    }
    return ticks;
}

static uint32_t _z80_ret(z80* c, void(*tick)(z80*), uint32_t ticks) {
    _RD(c->SP++, c->Z);
    _RD(c->SP++, c->W);
    c->PC = c->WZ;
    return ticks;
}

static uint32_t _z80_retcc(z80* c, bool cond, void(*tick)(z80*), uint32_t ticks) {
    _T();
    if (cond) {
        _RD(c->SP++, c->Z);
        _RD(c->SP++, c->W);
        c->PC = c->WZ;
    }
    return ticks;
}

/*-- MISC FUNCTIONS ----------------------------------------------------------*/
static uint16_t _z80_add16(z80* c, uint16_t acc, uint16_t val) {
    c->WZ = acc+1;
    uint32_t res = acc + val;
    // flag computation taken from MAME
    c->F = (c->F & (Z80_SF|Z80_ZF|Z80_VF)) |
           (((acc^res^val)>>8)&Z80_HF)|
           ((res>>16) & Z80_CF) | ((res >> 8) & (Z80_YF|Z80_XF));
    return (uint16_t)res;
}

static uint16_t _z80_adc16(z80* c, uint16_t acc, uint16_t val) {
    c->WZ = acc+1;
    uint32_t res = acc + val + (c->F & Z80_CF);
    // flag computation taken from MAME
    c->F = (((acc^res^val)>>8)&Z80_HF) |
           ((res>>16)&Z80_CF) |
           ((res>>8)&(Z80_SF|Z80_YF|Z80_XF)) |
           ((res & 0xFFFF) ? 0 : Z80_ZF) |
           (((val^acc^0x8000) & (val^res)&0x8000)>>13);
    return res;
}

static uint16_t _z80_sbc16(z80* c, uint16_t acc, uint16_t val) {
    c->WZ = acc+1;
    uint32_t res = acc - val - (c->F & Z80_CF);
    // flag computation taken from MAME
    c->F = (((acc^res^val)>>8)&Z80_HF) | Z80_NF |
           ((res>>16)&Z80_CF) |
           ((res>>8) & (Z80_SF|Z80_YF|Z80_XF)) |
           ((res & 0xFFFF) ? 0 : Z80_ZF) |
           (((val^acc) & (acc^res)&0x8000)>>13);
    return res;
}

static void _z80_halt(z80* c) {
    _ON(Z80_HALT);
    c->PC--;
}

static void _z80_di(z80* c) {
    c->IFF1 = false;
    c->IFF2 = false;
}

static void _z80_ei(z80* c) {
    c->ei_pending = true;
}

static void _z80_reti(z80* c) {
    // FIXME
}

static uint8_t _z80_sziff2(z80* c, uint8_t val) {
    uint8_t f = _SZ(val);
    f |= (val & (Z80_YF|Z80_XF));
    if (c->IFF2) f |= Z80_PF;
    return f;
}

static void _z80_daa(z80* c) {
    /* from MAME and http://www.z80.info/zip/z80-documented.pdf */
    uint8_t val = c->A;
    if (c->F & Z80_NF) {
        if (((c->A & 0xF) > 0x9) || (c->F & Z80_HF)) {
            val -= 0x06;
        }
        if ((c->A > 0x99) || (c->F & Z80_CF)) {
            val -= 0x60;
        }
    }
    else {
        if (((c->A & 0xF) > 0x9) || (c->F & Z80_HF)) {
            val += 0x06;
        }
        if ((c->A > 0x99) || (c->F & Z80_CF)) {
            val += 0x60;
        }
    }
    c->F &= Z80_CF|Z80_NF;
    c->F |= (c->A > 0x99) ? Z80_CF:0;
    c->F |= (c->A^val) & Z80_HF;
    c->F |= c->szp[val];
    c->A = val;
}

static void _z80_cpl(z80* c) {
    c->A ^= 0xFF;
    c->F = (c->F&(Z80_SF|Z80_ZF|Z80_PF|Z80_CF))|Z80_HF|Z80_NF|(c->A&(Z80_YF|Z80_XF));
}

static void _z80_scf(z80* c) {
    c->F = (c->F&(Z80_SF|Z80_ZF|Z80_YF|Z80_XF|Z80_PF))|Z80_CF|(c->A&(Z80_YF|Z80_XF));
}

static void _z80_ccf(z80* c) {
    c->F = ((c->F&(Z80_SF|Z80_ZF|Z80_YF|Z80_XF|Z80_PF|Z80_CF))|((c->F&Z80_CF)<<4)|(c->A&(Z80_YF|Z80_XF)))^Z80_CF;
}

static uint32_t _z80_rst(z80* c, uint8_t vec, void(*tick)(z80*), uint32_t ticks) {
    _WR(--c->SP, (uint8_t)c->PC<<8);
    _WR(--c->SP, (uint8_t)c->PC);
    c->WZ = c->PC = (uint16_t) vec;
    return ticks;
}

/*-- INSTRUCTION DECODER ----------------------------------------------------*/
#include "_z80_opcodes.h"

/*-- PUBLIC FUNCTIONS --------------------------------------------------------*/
void z80_init(z80* c, z80_desc* desc) {
    CHIPS_ASSERT(c);
    CHIPS_ASSERT(desc);
    CHIPS_ASSERT(desc->tick_func);
    memset(c, 0, sizeof(z80));
    z80_reset(c);
    c->tick = desc->tick_func;
    c->context = desc->tick_context;
    /* init SZP flags table */
    for (int val = 0; val < 256; val++) {
        int p = 0;
        for (int i = 0; i < 8; i++) {
            if (val & (1<<i)) p++;
        }
        uint8_t f = val ? (val & Z80_SF) : Z80_ZF;
        f |= (val & (Z80_YF|Z80_XF));   // undocumented flag bits 3 and 5
        f |= p & 1 ? 0 : Z80_PF;
        c->szp[val] = f;
    }
}

void z80_reset(z80* c) {
    CHIPS_ASSERT(c);
    /* AF and SP are set to 0xFFFF */
    c->AF = c->SP = 0xFFFF;
    /* PC is set to 0x0000 */
    c->PC = 0x0000;
    /* IFF1 and IFF2 are off */
    c->IFF1 = c->IFF2 = false;
    /* IM is set to 0 */
    c->IM = 0;
    /* all other registers are undefined, set them to 0xFF */
    c->BC = c->DE = c->HL = 0xFFFF;
    c->IX = c->IY = 0xFFFF;
    c->BC_ = c->DE_ = c->HL_ = c->AF_ = 0xFFFF;
    c->WZ = c->IR = 0xFFFF;
    /* after power-on or reset, R is set to 0 (see z80-documented.pdf) */
    c->R = 0;
    c->ei_pending = false;
}

uint32_t z80_step(z80* c) {
    if (c->ei_pending) {
        c->IFF1 = c->IFF2 = true;
        c->ei_pending = false;
    }
    return _z80_op(c, 0);
}

uint32_t z80_run(z80* c, uint32_t t) {
    // FIXME
    return 0;
}

#undef _SZ
#undef _SZYXCH
#undef _ADD_FLAGS
#undef _SUB_FLAGS
#undef _CP_FLAGS
#undef _ON
#undef _OFF
#undef _T
#undef _RD
#undef _RDS
#undef _WR
#undef _IN
#undef _OUT
#undef _SWP16
#endif /* CHIPS_IMPL */

#ifdef __cplusplus
} /* extern "C" */
#endif
