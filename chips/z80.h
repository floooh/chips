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
    /* NOTE: union layout assumes little-endian CPU */
    union { uint16_t BC; struct { uint8_t C, B; }; };
    union { uint16_t DE; struct { uint8_t E, D; }; };
    union { uint16_t HL; struct { uint8_t L, H; }; };
    union { uint16_t AF; struct { uint8_t F, A; }; };
    union { uint16_t IX; struct { uint8_t IXL, IXH; }; };
    union { uint16_t IY; struct { uint8_t IYL, IYH; }; };
    union { uint16_t WZ; struct { uint8_t Z, W; }; };
    union { uint16_t IR; struct { uint8_t R, I; }; };
    uint16_t BC_, DE_, HL_, AF_, WZ_;
    uint16_t PC;
    uint16_t SP;
    uint16_t CTRL;      /* control pins */
    uint16_t ADDR;      /* address pins */
    uint8_t DATA;       /* data pins */
    uint8_t IM;
    bool IFF1, IFF2;

    uint8_t opcode;     /* last opcode read with _z80_fetch() */
    bool ei_pending;
    uint32_t ticks;

    /* tick function and context data */
    void (*tick)(z80* cpu);
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
/* execute the next instruction, return number of time cycles */
extern uint32_t z80_step(z80* cpu);
/* execute instructions for up to 'ticks' time cycles, return executed time cycles */
extern uint32_t z80_run(z80* cpu, uint32_t ticks);
/* set one or more pins to active state */
extern void z80_on(z80* cpu, uint16_t pins);
/* set one or more pins to cleared state */
extern void z80_off(z80* cpu, uint16_t pins);
/* test if any control pin is active */
extern bool z80_any(z80* cpu, uint16_t pins);
/* test if all control pins are active */
extern bool z80_all(z80* cpu, uint16_t pins);

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
#ifndef _IMM16
#undef _IMM16
#endif
#ifndef _INVALID_OPCODE
#undef _INVALID_OPCODE
#endif
#define _SZ(val) ((val&0xFF)?(val&Z80_SF):Z80_ZF)
#define _SZYXCH(acc,val,res) (_SZ(res)|(res&(Z80_YF|Z80_XF))|((res>>8)&Z80_CF)|((acc^val^res)&Z80_HF))
#define _ADD_FLAGS(acc,val,res) (_SZYXCH(acc,val,res)|((((val^acc^0x80)&(val^res))>>5)&Z80_VF))
#define _SUB_FLAGS(acc,val,res) (Z80_NF|_SZYXCH(acc,val,res)|((((val^acc)&(res^acc))>>5)&Z80_VF))
#define _CP_FLAGS(acc,val,res) (Z80_NF|(_SZ(res)|(val&(Z80_YF|Z80_XF))|((res>>8)&Z80_CF)|((acc^val^res)&Z80_HF))|((((val^acc)&(res^acc))>>5)&Z80_VF))
#define _ON(m) { c->CTRL |= (m); }
#define _OFF(m) { c->CTRL &= ~(m); }
#define _T() { c->tick(c); c->ticks++; }
#define _WR(a,r) _z80_write(c, a, r)
#define _RD(a) _z80_read(c, a)
#define _OUT(a,r) _z80_out(c, a, r)
#define _IN(a) _z80_in(c, a)
#define _SWP16(a,b) { uint16_t tmp=a; a=b; b=tmp; }
#define _IMM16() c->Z=_RD(c->PC++);c->W=_RD(c->PC++);
#define _INVALID_OPCODE(n) CHIPS_ASSERT(false);

/*
    instruction fetch machine cycle (M1)
              T1   T2   T3   T4
    --------+----+----+----+----+
    CLK     |--**|--**|--**|--**|
    A15-A0  |   PC    | REFRESH |
    MREQ    |   *|****|  **|**  |
    RD      |   *|****|    |    |
    WAIT    |    | -- |    |    |
    M1      |****|****|    |    |
    D7-D0   |    |   X|    |    |
    RFSH    |    |    |****|****|

    Result is the fetched opcode in z80.opcode and z80.DATA
*/
static uint8_t _z80_fetch(z80* c) {
    /*--- T1 ---*/
    _ON(Z80_M1);
    c->ADDR = c->PC++;
    _T();
    /*--- T2 ---*/
    _ON(Z80_MREQ|Z80_RD);
    _T();
    uint8_t opcode = c->DATA;
    c->R = (c->R&0x80)|((c->R+1)&0x7F);   /* update R */
    /*--- T3 ---*/
    _OFF(Z80_M1|Z80_MREQ|Z80_RD);
    _ON(Z80_RFSH);
    c->ADDR = c->IR;
    _T();
    /*--- T4 ---*/
    _ON(Z80_MREQ);
    _T();
    _OFF(Z80_RFSH|Z80_MREQ);
    return opcode;
}

/*
    a memory read cycle, place address in ADDR, read byte into DATA

              T1   T2   T3
    --------+----+----+----+
    CLK     |--**|--**|--**|
    A15-A0  |   MEM ADDR   |
    MREQ    |   *|****|*** |
    RD      |   *|****|*** |
    WR      |    |    |    |
    D7-D0   |    |    | X  |
    WAIT    |    | -- |    |
*/
static uint8_t _z80_read(z80* c, uint16_t addr) {
    /*--- T1 ---*/
    c->ADDR = addr;
    _T();
    /*--- T2 ---*/
    _ON(Z80_MREQ|Z80_RD);
    _T();    /* tick callback must read memory here */
    /*--- T3 ---*/
    _OFF(Z80_MREQ|Z80_RD);
    _T();
    return c->DATA;
}

/*
    a memory write cycle, place 16-bit address into ADDR, place 8-bit
    value into DATA, and then memory[ADDR] = DATA

              T1   T2   T3
    --------+----+----+----+
    CLK     |--**|--**|--**|
    A15-A0  |   MEM ADDR   |
    MREQ    |   *|****|*** |
    RD      |    |    |    |
    WR      |    |  **|*** |
    D7-D0   |   X|XXXX|XXXX|
    WAIT    |    | -- |    |
*/
static void _z80_write(z80* c, uint16_t addr, uint8_t data) {
    /*--- T1 ---*/
    c->ADDR = addr;
    _T();
    /*--- T2 ---*/
    _ON(Z80_MREQ|Z80_WR);
    c->DATA = data;
    _T();    /* tick callback must write memory here */
    /*--- T3 ---*/
    _OFF(Z80_MREQ|Z80_WR);
    _T();
}

static uint8_t _z80_in(z80* c, uint16_t addr) {
    // FIXME!
    return 0;
}

static void _z80_out(z80* c, uint16_t addr, uint8_t data) {
    // FIXME!
}

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

static void _z80_rrd(z80* c) {
    c->WZ = c->HL;
    uint8_t x = _RD(c->WZ++);
    uint8_t tmp = c->A & 0xF;           // store A low nibble
    c->A = (c->A & 0xF0) | (x & 0x0F);  // move (HL) low nibble to A low nibble
    x = (x >> 4) | (tmp << 4);          // move A low nibble to (HL) high nibble, and (HL) high nibble to (HL) low nibble
    _T(); _T(); _T(); _T();
    _WR(c->HL, x);
    c->F = c->szp[c->A] | (c->F & Z80_CF);
}

static void _z80_rld(z80* c) {
    c->WZ = c->HL;
    uint8_t x = _RD(c->WZ++);
    uint8_t tmp = c->A & 0xF;             // store A low nibble
    c->A = (c->A & 0xF0) | (x>>4);      // move (HL) high nibble into A low nibble
    x = (x<<4) | tmp;                       // move (HL) low to high nibble, move A low nibble to (HL) low nibble
    _T(); _T(); _T(); _T();
    _WR(c->HL, x);
    c->F = c->szp[c->A] | (c->F & Z80_CF);
}

/*-- BLOCK FUNCTIONS ---------------------------------------------------------*/
static void _z80_ldi(z80* c) {
    uint8_t val = _RD(c->HL);
    _WR(c->DE, val);
    _T(); _T();
    val += c->A;
    uint8_t f = c->F & (Z80_SF|Z80_ZF|Z80_CF);
    if (val & 0x02) f |= Z80_YF;
    if (val & 0x08) f |= Z80_XF;
    c->HL++;
    c->DE++;
    c->BC--;
    if (c->BC) {
        f |= Z80_VF;
    }
    c->F = f;
}

static void _z80_ldd(z80* c) {
    uint8_t val = _RD(c->HL);
    _WR(c->DE, val);
    _T(); _T();
    val += c->A;
    uint8_t f = c->F & (Z80_SF|Z80_ZF|Z80_CF);
    if (val & 0x02) f |= Z80_YF;
    if (val & 0x08) f |= Z80_XF;
    c->HL--;
    c->DE--;
    c->BC--;
    if (c->BC) {
        f |= Z80_VF;
    }
    c->F = f;
}

static void _z80_ldir(z80* c) {
    _z80_ldi(c);
    if (c->BC != 0) {
        c->PC -= 2;
        c->WZ = c->PC + 1;
        _T(); _T(); _T(); _T(); _T();
    }
}

static void _z80_lddr(z80* c) {
    _z80_ldd(c);
    if (c->BC != 0) {
        c->PC -= 2;
        c->WZ = c->PC + 1;
        _T(); _T(); _T(); _T(); _T();
    }
}

static void _z80_cpi(z80* c) {
    int r = (int)c->A - (int)_RD(c->HL);
    _T(); _T(); _T(); _T(); _T();
    uint8_t f = Z80_NF | (c->F & Z80_CF) | _SZ(r);
    if ((r & 0xF) > (c->A & 0xF)) {
        f |= Z80_HF;
        r--;
    }
    if (r & 0x02) f |= Z80_YF;
    if (r & 0x08) f |= Z80_XF;
    c->WZ++;
    c->HL++;
    c->BC--;
    if (c->BC) {
        f |= Z80_VF;
    }
    c->F = f;
}

static void _z80_cpd(z80* c) {
    int r = (int)c->A - (int)_RD(c->HL);
    _T(); _T(); _T(); _T(); _T();
    uint8_t f = Z80_NF | (c->F & Z80_CF) | _SZ(r);
    if ((r & 0xF) > (c->A & 0xF)) {
        f |= Z80_HF;
        r--;
    }
    if (r & 0x02) f |= Z80_YF;
    if (r & 0x08) f |= Z80_XF;
    c->WZ--;
    c->HL--;
    c->BC--;
    if (c->BC) {
        f |= Z80_VF;
    }
    c->F = f;
}

static void _z80_cpir(z80* c) {
    _z80_cpi(c);
    if ((c->BC != 0) && !(c->F & Z80_ZF)) {
        c->PC -= 2;
        c->WZ = c->PC + 1;    /* FIXME: is this correct (see memptr_eng.txt) */
        _T(); _T(); _T(); _T(); _T();
    }
}

static void _z80_cpdr(z80* c) {
    _z80_cpd(c);
    if ((c->BC != 0) && !(c->F & Z80_ZF)) {
        c->PC -= 2;
        c->WZ = c->PC + 1;
        _T(); _T(); _T(); _T(); _T();
    }
}

static void _z80_ini(z80* c) {
    // FIXME
}

static void _z80_ind(z80* c) {
    // FIXME
}

static void _z80_inir(z80* c) {
    // FIXME
}

static void _z80_indr(z80* c) {
    // FIXME
}

static void _z80_outi(z80* c) {
    // FIXME
}

static void _z80_outd(z80* c) {
    // FIXME
}

static void _z80_otir(z80* c) {
    // FIXME
}

static void _z80_otdr(z80* c) {
    // FIXME
}

/*-- CONTROL FLOW FUNCTIONS --------------------------------------------------*/
static void _z80_djnz(z80* c) {
    _T();
    int8_t d = (int8_t) _RD(c->PC++);
    if (--c->B > 0) {
        c->WZ = c->PC = c->PC + d;
        _T(); _T(); _T(); _T(); _T();
    }
}

static void _z80_jr(z80* c) {
    int8_t d = (int8_t) _RD(c->PC++);
    c->WZ = c->PC + d;
    c->PC = c->WZ;
    _T(); _T(); _T(); _T(); _T();
}

static void _z80_jr_cc(z80* c, bool cond) {
    int8_t d = (int8_t) _RD(c->PC++);
    if (cond) {
        c->WZ = c->PC = c->PC + d;
        _T(); _T(); _T(); _T(); _T();
    }
}

static void _z80_call(z80* c) {
    _IMM16();
    _T();
    _WR(--c->SP, (uint8_t)(c->PC>>8));
    _WR(--c->SP, (uint8_t)c->PC);
    c->PC=c->WZ;
}

static void _z80_ret(z80* c) {
    c->Z = _RD(c->SP++);
    c->W = _RD(c->SP++);
    c->PC = c->WZ;
}

static void _z80_callcc(z80* c, bool cond) {
    _IMM16();
    if (cond) {
        _T();
        _WR(--c->SP, (uint8_t)(c->PC>>8));
        _WR(--c->SP, (uint8_t)c->PC);
        c->PC = c->WZ;
    }
}

static void _z80_retcc(z80* c, bool cond) {
    _T();
    if (cond) {
        c->Z = _RD(c->SP++);
        c->W = _RD(c->SP++);
        c->PC = c->WZ;
    }
}

/*-- BIT MANIPULATION FUNCTIONS ----------------------------------------------*/
static void _z80_bit(z80* c, uint8_t val, uint8_t mask) {
    uint8_t r = val & mask;
    uint8_t f = Z80_HF | (r ? (r & Z80_SF) : (Z80_ZF|Z80_PF));
    f |= (val & (Z80_YF|Z80_XF));
    c->F = f | (c->F & Z80_CF);
}

static void _z80_ibit(z80* c, uint8_t val, uint8_t mask) {
    // this is the version for the BIT instruction for (HL), (IX+d), (IY+d),
    // these set the undocumented YF and XF flags from high byte of HL+1
    // or IX/IY+d
    uint8_t r = val & mask;
    uint8_t f = Z80_HF | (r ? (r & Z80_SF) : (Z80_ZF|Z80_PF));
    f |= (c->W & (Z80_YF|Z80_XF));
    c->F = f | (c->F & Z80_CF);
}
/*-- MISC FUNCTIONS ----------------------------------------------------------*/
static uint16_t _z80_add16(z80* c, uint16_t val0, uint16_t val1) {
    // FIXME
    return 0;
}

static uint16_t _z80_adc16(z80* c, uint16_t val0, uint16_t val1) {
    // FIXME
    return 0;
}

static uint16_t _z80_sbc16(z80* c, uint16_t val0, uint16_t val1) {
    // FIXME
    return 0;
}

static uint16_t _z80_exsp(z80* c, uint16_t val) {
    // FIXME!
    return 0;
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

static void _z80_rst(z80* c, uint8_t vec) {
    _WR(--c->SP, (uint8_t)c->PC<<8);
    _WR(--c->SP, (uint8_t)c->PC);
    c->WZ = c->PC = (uint16_t) vec;
}

/*-- INSTRUCTION DECODER ----------------------------------------------------*/
#include "_z80_opcodes.h"

/*-- PUBLIC FUNCTIONS --------------------------------------------------------*/
void z80_init(z80* c, z80_desc* desc) {
    CHIPS_ASSERT(c);
    CHIPS_ASSERT(desc);
    CHIPS_ASSERT(desc->tick_func);
    memset(c, 0, sizeof(z80));
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

uint32_t z80_step(z80* c) {
    c->ticks = 0;
    if (c->ei_pending) {
        c->IFF1 = c->IFF2 = true;
        c->ei_pending = false;
    }
    _z80_op(c);
    return c->ticks;
}

uint32_t z80_run(z80* c, uint32_t t) {
    // FIXME
    return 0;
}

/* control pin functions */
void z80_on(z80* c, uint16_t pins) {
    c->CTRL |= pins;
}
void z80_off(z80* c, uint16_t pins) {
    c->CTRL &= ~pins;
}
bool z80_any(z80* c, uint16_t pins) {
    return (c->CTRL & pins) != 0;
}
bool z80_all(z80* c, uint16_t pins) {
    return (c->CTRL & pins) == pins;
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
#undef _WR
#undef _IN
#undef _OUT
#undef _SWP16
#undef _IMM16
#undef _INVALID_OPCODE
#endif /* CHIPS_IMPL */

#ifdef __cplusplus
} /* extern "C" */
#endif
