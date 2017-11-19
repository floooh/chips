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
    union {
        /* NOTE: union layout assumes little-endian host CPU */
        uint8_t r8[14];
        uint16_t r16[7];
        struct {
            union { uint16_t BC; struct { uint8_t C, B; }; };
            union { uint16_t DE; struct { uint8_t E, D; }; };
            union { uint16_t HL; struct { uint8_t L, H; }; };
            union { uint16_t FA; struct { uint8_t A, F; }; };
            union { uint16_t IX; struct { uint8_t IXL, IXH; }; };
            union { uint16_t IY; struct { uint8_t IYL, IYH; }; };
            uint16_t SP;
        };
    };
    int ri8[8];         /* register indirection indices into r8 */
    int ri16af[4];      /* register indirection indices into r16, with AF */
    int ri16sp[4];      /* register indirection indices into r16, with SP */
    union { uint16_t WZ; struct { uint8_t Z, W; }; };
    union { uint16_t IR; struct { uint8_t R, I; }; };
    uint16_t BC_, DE_, HL_, FA_, WZ_;
    uint16_t PC;
    uint16_t CTRL;      /* control pins */
    uint16_t ADDR;      /* address pins */
    uint8_t DATA;       /* data pins */

    uint8_t opcode;     /* last opcode read with _z80_fetch() */
    uint8_t im;
    bool ddfd;          /* IX/IY indexed opcode active */
    bool iff1, iff2;
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
#ifdef _TICK
#undef _TICK
#endif
#ifdef _TICKS
#undef _TICKS
#endif
#ifdef _EXTICKS
#undef _EXTICKS
#endif
#ifdef _READ
#undef _READ
#endif
#ifdef _SWAP16
#undef _SWAP16
#endif
#define _SZ(val) ((val&0xFF)?(val&Z80_SF):Z80_ZF)
#define _SZYXCH(acc,val,res) (_SZ(res)|(res&(Z80_YF|Z80_XF))|((res>>8)&Z80_CF)|((acc^val^res)&Z80_HF))
#define _ADD_FLAGS(acc,val,res) (_SZYXCH(acc,val,res)|((((val^acc^0x80)&(val^res))>>5)&Z80_VF))
#define _SUB_FLAGS(acc,val,res) (Z80_NF|_SZYXCH(acc,val,res)|((((val^acc)&(res^acc))>>5)&Z80_VF))
#define _CP_FLAGS(acc,val,res) (Z80_NF|(_SZ(res)|(val&(Z80_YF|Z80_XF))|((res>>8)&Z80_CF)|((acc^val^res)&Z80_HF))|((((val^acc)&(res^acc))>>5)&Z80_VF))
#define _ON(m) { cpu->CTRL |= (m); }
#define _OFF(m) { cpu->CTRL &= ~(m); }
#define _TICK() { cpu->tick(cpu); cpu->ticks++; }
#define _TICKS(n) { for (int i=0; i<n; i++) { cpu->tick(cpu); }; cpu->ticks+=n; }
#define _EXTICKS(n) { if (cpu->ddfd) { for (int i=0; i<n; i++) { cpu->tick(cpu); }; cpu->ticks+=n; } }
#define _WRITE(a,r) _z80_write(cpu, a, r)
#define _READ(a) _z80_read(cpu, a)
#define _SWAP16(a,b) { uint16_t tmp=a; a=b; b=tmp; }

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
static void _z80_fetch(z80* cpu) {
    /*--- T1 ---*/
    _ON(Z80_M1);
    cpu->ADDR = cpu->PC++;
    _TICK();
    /*--- T2 ---*/
    _ON(Z80_MREQ|Z80_RD);
    _TICK();
    cpu->opcode = cpu->DATA;
    cpu->R = (cpu->R&0x80)|((cpu->R+1)&0x7F);   /* update R */
    /*--- T3 ---*/
    _OFF(Z80_M1|Z80_MREQ|Z80_RD);
    _ON(Z80_RFSH);
    cpu->ADDR = cpu->IR;
    _TICK();
    /*--- T4 ---*/
    _ON(Z80_MREQ);
    _TICK();
    _OFF(Z80_RFSH|Z80_MREQ);
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
static uint8_t _z80_read(z80* cpu, uint16_t addr) {
    /*--- T1 ---*/
    cpu->ADDR = addr;
    _TICK();
    /*--- T2 ---*/
    _ON(Z80_MREQ|Z80_RD);
    _TICK();    /* tick callback must read memory here */
    /*--- T3 ---*/
    _OFF(Z80_MREQ|Z80_RD);
    _TICK();
    return cpu->DATA;
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
static void _z80_write(z80* cpu, uint16_t addr, uint8_t data) {
    /*--- T1 ---*/
    cpu->ADDR = addr;
    _TICK();
    /*--- T2 ---*/
    _ON(Z80_MREQ|Z80_WR);
    cpu->DATA = data;
    _TICK();    /* tick callback must write memory here */
    /*--- T3 ---*/
    _OFF(Z80_MREQ|Z80_WR);
    _TICK();
}

/*-- ALU FUNCTIONS -----------------------------------------------------------*/
static void _z80_add8(z80* cpu, uint8_t val) {
    int res = cpu->A + val;
    cpu->F = _ADD_FLAGS(cpu->A, val, res);
    cpu->A = (uint8_t) res;
}

static void _z80_adc8(z80* cpu, uint8_t val) {
    int res = cpu->A + val + (cpu->F & Z80_CF);
    cpu->F = _ADD_FLAGS(cpu->A, val, res);
    cpu->A = (uint8_t) res;
}

static void _z80_sub8(z80* cpu, uint8_t val) {
    int res = (int)cpu->A - (int)val;
    cpu->F = _SUB_FLAGS(cpu->A, val, res);
    cpu->A = (uint8_t) res;
}

static void _z80_sbc8(z80* cpu, uint8_t val) {
    int res = (int)cpu->A - (int)val - (cpu->F & Z80_CF);
    cpu->F = _SUB_FLAGS(cpu->A, val, res);
    cpu->A = (uint8_t) res;
}

static void _z80_cp8(z80* cpu, uint8_t val) {
    /* NOTE: XF|YF are set from val, not from result */
    int res = (int)cpu->A - (int)val;
    cpu->F = _CP_FLAGS(cpu->A, val, res);
}

static void _z80_and8(z80* cpu, uint8_t val) {
    cpu->A &= val;
    cpu->F = cpu->szp[cpu->A]|Z80_HF;
}

static void _z80_or8(z80* cpu, uint8_t val) {
    cpu->A |= val;
    cpu->F = cpu->szp[cpu->A];
}

static void _z80_xor8(z80* cpu, uint8_t val) {
    cpu->A ^= val;
    cpu->F = cpu->szp[cpu->A];
}

static void _z80_neg8(z80* cpu) {
    uint8_t val = cpu->A;
    cpu->A = 0;
    _z80_sub8(cpu, val);
}

static uint8_t _z80_inc8(z80* cpu, uint8_t val) {
    uint8_t r = val + 1;
    uint8_t f = _SZ(r)|(r&(Z80_XF|Z80_YF))|((r^val)&Z80_HF);
    if (r == 0x80) f |= Z80_VF;
    cpu->F = f | (cpu->F & Z80_CF);
    return r;
}

static uint8_t _z80_dec8(z80* cpu, uint8_t val) {
    uint8_t r = val - 1;
    uint8_t f = Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^val)&Z80_HF);
    if (r == 0x7F) f |= Z80_VF;
    cpu->F = f | (cpu->F & Z80_CF);
    return r;
}

/*-- ROTATE AND SHIFT FUNCTIONS ----------------------------------------------*/
static void _z80_rlca(z80* cpu) {
    uint8_t r = cpu->A<<1 | cpu->A>>7;
    cpu->F = (cpu->A>>7 & Z80_CF)|(cpu->F & (Z80_SF|Z80_ZF|Z80_PF))|(r & (Z80_XF|Z80_YF));
    cpu->A = r;
}

static void _z80_rrca(z80* cpu) {
    uint8_t r = cpu->A>>1 | cpu->A<<7;
    cpu->F = (cpu->A & Z80_CF)|(cpu->F & (Z80_SF|Z80_ZF|Z80_PF))|(r & (Z80_YF|Z80_XF));
    cpu->A = r;
}

static void _z80_rla(z80* cpu) {
    uint8_t r = cpu->A<<1 | (cpu->F & Z80_CF);
    cpu->F = (cpu->A>>7 & Z80_CF) | (cpu->F & (Z80_SF|Z80_ZF|Z80_PF)) | (r & (Z80_YF|Z80_XF));
    cpu->A = r;
}

static void _z80_rra(z80* cpu) {
    uint8_t r = cpu->A>>1 | ((cpu->F & Z80_CF)<<7);
    cpu->F = (cpu->A & Z80_CF) | (cpu->F & (Z80_SF|Z80_ZF|Z80_PF)) | (r & (Z80_YF|Z80_XF));
    cpu->A = r;
}

static uint8_t _z80_rlc(z80* cpu, uint8_t val) {
    uint8_t r = val<<1 | val>>7;
    cpu->F = cpu->szp[r] | (val>>7 & Z80_CF);
    return r;
}

static uint8_t _z80_rrc(z80* cpu, uint8_t val) {
    uint8_t r = val>>1 | val<<7;
    cpu->F = cpu->szp[r] | (val & Z80_CF);
    return r;
}

static uint8_t _z80_rl(z80* cpu, uint8_t val) {
    uint8_t r = val<<1 | (cpu->F & Z80_CF);
    cpu->F = (val>>7 & Z80_CF) | cpu->szp[r];
    return r;
}

static uint8_t _z80_rr(z80* cpu, uint8_t val) {
    uint8_t r = val>>1 | ((cpu->F & Z80_CF)<<7);
    cpu->F = (val & Z80_CF) | cpu->szp[r];
    return r;
}

static uint8_t _z80_sla(z80* cpu, uint8_t val) {
    uint8_t r = val<<1;
    cpu->F = (val>>7 & Z80_CF) | cpu->szp[r];
    return r;
}

static uint8_t _z80_sra(z80* cpu, uint8_t val) {
    uint8_t r = val>>1 | (val & 0x80);
    cpu->F = (val & Z80_CF) | cpu->szp[r];
    return r;
}

static uint8_t _z80_sll(z80* cpu, uint8_t val) {
    // undocument! sll8 is identical with sla8 but inserts a 1 into the LSB
    uint8_t r = (val<<1) | 1;
    cpu->F = (val>>7 & Z80_CF) | cpu->szp[r];
    return r;
}

static uint8_t _z80_srl(z80* cpu, uint8_t val) {
    uint8_t r = val>>1;
    cpu->F = (val & Z80_CF) | cpu->szp[r];
    return r;
}

static void _z80_rrd(z80* cpu) {
    cpu->WZ = cpu->HL;
    uint8_t x = _READ(cpu->WZ++);
    uint8_t tmp = cpu->A & 0xF;             // store A low nibble
    cpu->A = (cpu->A & 0xF0) | (x & 0x0F);  // move (HL) low nibble to A low nibble
    x = (x >> 4) | (tmp << 4);              // move A low nibble to (HL) high nibble, and (HL) high nibble to (HL) low nibble
    _TICKS(4);
    _WRITE(cpu->HL, x);
    cpu->F = cpu->szp[cpu->A] | (cpu->F & Z80_CF);
}

static void _z80_rld(z80* cpu) {
    cpu->WZ = cpu->HL;
    uint8_t x = _READ(cpu->WZ++);
    uint8_t tmp = cpu->A & 0xF;             // store A low nibble
    cpu->A = (cpu->A & 0xF0) | (x>>4);      // move (HL) high nibble into A low nibble
    x = (x<<4) | tmp;                       // move (HL) low to high nibble, move A low nibble to (HL) low nibble
    _TICKS(4);
    _WRITE(cpu->HL, x);
    cpu->F = cpu->szp[cpu->A] | (cpu->F & Z80_CF);
}

/*-- BLOCK FUNCTIONS ---------------------------------------------------------*/
static void _z80_ldi(z80* cpu) {
    uint8_t val = _READ(cpu->HL);
    _WRITE(cpu->DE, val);
    _TICKS(2);
    val += cpu->A;
    uint8_t f = cpu->F & (Z80_SF|Z80_ZF|Z80_CF);
    if (val & 0x02) f |= Z80_YF;
    if (val & 0x08) f |= Z80_XF;
    cpu->HL++;
    cpu->DE++;
    cpu->BC--;
    if (cpu->BC) {
        f |= Z80_VF;
    }
    cpu->F = f;
}

static void _z80_ldd(z80* cpu) {
    uint8_t val = _READ(cpu->HL);
    _WRITE(cpu->DE, val);
    _TICKS(2);
    val += cpu->A;
    uint8_t f = cpu->F & (Z80_SF|Z80_ZF|Z80_CF);
    if (val & 0x02) f |= Z80_YF;
    if (val & 0x08) f |= Z80_XF;
    cpu->HL--;
    cpu->DE--;
    cpu->BC--;
    if (cpu->BC) {
        f |= Z80_VF;
    }
    cpu->F = f;
}

static void _z80_ldir(z80* cpu) {
    _z80_ldi(cpu);
    if (cpu->BC != 0) {
        cpu->PC -= 2;
        cpu->WZ = cpu->PC + 1;
        _TICKS(5);
    }
}

static void _z80_lddr(z80* cpu) {
    _z80_ldd(cpu);
    if (cpu->BC != 0) {
        cpu->PC -= 2;
        cpu->WZ = cpu->PC + 1;
        _TICKS(5);
    }
}

static void _z80_cpi(z80* cpu) {
    int r = (int)cpu->A - (int)_READ(cpu->HL);
    _TICKS(5);
    uint8_t f = Z80_NF | (cpu->F & Z80_CF) | _SZ(r);
    if ((r & 0xF) > (cpu->A & 0xF)) {
        f |= Z80_HF;
        r--;
    }
    if (r & 0x02) f |= Z80_YF;
    if (r & 0x08) f |= Z80_XF;
    cpu->WZ++;
    cpu->HL++;
    cpu->BC--;
    if (cpu->BC) {
        f |= Z80_VF;
    }
    cpu->F = f;
}

static void _z80_cpd(z80* cpu) {
    int r = (int)cpu->A - (int)_READ(cpu->HL);
    _TICKS(5);
    uint8_t f = Z80_NF | (cpu->F & Z80_CF) | _SZ(r);
    if ((r & 0xF) > (cpu->A & 0xF)) {
        f |= Z80_HF;
        r--;
    }
    if (r & 0x02) f |= Z80_YF;
    if (r & 0x08) f |= Z80_XF;
    cpu->WZ--;
    cpu->HL--;
    cpu->BC--;
    if (cpu->BC) {
        f |= Z80_VF;
    }
    cpu->F = f;
}

static void _z80_cpir(z80* cpu) {
    _z80_cpi(cpu);
    if ((cpu->BC != 0) && !(cpu->F & Z80_ZF)) {
        cpu->PC -= 2;
        cpu->WZ = cpu->PC + 1;    /* FIXME: is this correct (see memptr_eng.txt) */
        _TICKS(5);
    }
}

static void _z80_cpdr(z80* cpu) {
    _z80_cpd(cpu);
    if ((cpu->BC != 0) && !(cpu->F & Z80_ZF)) {
        cpu->PC -= 2;
        cpu->WZ = cpu->PC + 1;
        _TICKS(5);
    }
}

static void _z80_ini(z80* cpu) {
    // FIXME
}

static void _z80_ind(z80* cpu) {
    // FIXME
}

static void _z80_inir(z80* cpu) {
    // FIXME
}

static void _z80_indr(z80* cpu) {
    // FIXME
}

static void _z80_outi(z80* cpu) {
    // FIXME
}

static void _z80_outd(z80* cpu) {
    // FIXME
}

static void _z80_otir(z80* cpu) {
    // FIXME
}

static void _z80_otdr(z80* cpu) {
    // FIXME
}

/*-- CONTROL FLOW FUNCTIONS --------------------------------------------------*/
static bool _z80_cond(z80* cpu, uint8_t cc) {
    /* condition code flag check */
    switch (cc) {
        case 0: return !(cpu->F & Z80_ZF);      /* NZ */
        case 1: return  (cpu->F & Z80_ZF);      /* Z */
        case 2: return !(cpu->F & Z80_CF);      /* NC */
        case 3: return  (cpu->F & Z80_CF);      /* C */
        case 4: return !(cpu->F & Z80_PF);      /* PO */
        case 5: return  (cpu->F & Z80_PF);      /* PE */
        case 6: return !(cpu->F & Z80_SF);      /* P */
        case 7: return  (cpu->F & Z80_SF);      /* M */
    }
    return false; /* can't happen */
}

static void _z80_djnz(z80* cpu) {
    _TICK();
    int8_t d = (int8_t) _READ(cpu->PC++);
    if (--cpu->B > 0) {
        cpu->WZ = cpu->PC = cpu->PC + d;
        _TICKS(5);
    }
}

static void _z80_jp(z80* cpu) {
    cpu->Z = _READ(cpu->PC++);
    cpu->W = _READ(cpu->PC);
    cpu->PC = cpu->WZ;
}

static void _z80_jpcc(z80* cpu, uint8_t cond) {
    cpu->Z = _READ(cpu->PC++);
    cpu->W = _READ(cpu->PC++);
    if (_z80_cond(cpu, cond)) {
        cpu->PC = cpu->WZ;
    }
}

static void _z80_jrcc(z80* cpu, uint8_t cond) {
    int8_t d = (int8_t) _READ(cpu->PC++);
    if (_z80_cond(cpu, cond-4)) {
        cpu->WZ = cpu->PC = cpu->PC + d;
        _TICKS(5);
    }
}

static void _z80_call(z80* cpu) {
    cpu->Z = _READ(cpu->PC++);
    cpu->W = _READ(cpu->PC++);
    _TICK();
    _WRITE(--cpu->SP, (uint8_t)(cpu->PC>>8));
    _WRITE(--cpu->SP, (uint8_t)cpu->PC);
    cpu->PC=cpu->WZ;
}

static void _z80_ret(z80* cpu) {
    cpu->Z = _READ(cpu->SP++);
    cpu->W = _READ(cpu->SP++);
    cpu->PC = cpu->WZ;
}

static void _z80_callcc(z80* cpu, uint8_t cond) {
    cpu->Z = _READ(cpu->PC++);
    cpu->W = _READ(cpu->PC++);
    if (_z80_cond(cpu, cond)) {
        _TICK();
        _WRITE(--cpu->SP, (uint8_t)(cpu->PC>>8));
        _WRITE(--cpu->SP, (uint8_t)cpu->PC);
        cpu->PC = cpu->WZ;
    }
}

static void _z80_retcc(z80* cpu, uint8_t cond) {
    _TICK();
    if (_z80_cond(cpu, cond)) {
        cpu->Z = _READ(cpu->SP++);
        cpu->W = _READ(cpu->SP++);
        cpu->PC = cpu->WZ;
    }
}

/*-- MISC FUNCTIONS ----------------------------------------------------------*/
static void _z80_halt(z80* cpu) {
    _ON(Z80_HALT);
    cpu->PC--;
}

static void _z80_di(z80* cpu) {
    cpu->iff1 = false;
    cpu->iff2 = false;
}

static void _z80_ei(z80* cpu) {
    cpu->ei_pending = true;
}

static void _z80_im(z80* cpu, uint8_t y) {
    /* mapping table: 0->0; 1->0; 2->1; 3->2; 4->0; 5->0; 6->1; 7->2 */
    cpu->im = (y&3)==0 ? 0 : (y&3)-1;
}

static uint8_t _z80_sziff2(z80* cpu, uint8_t val) {
    uint8_t f = _SZ(val);
    f |= (val & (Z80_YF|Z80_XF));
    if (cpu->iff2) f |= Z80_PF;
    return f;
}

static void _z80_dda(z80* cpu) {
    /* from MAME and http://www.z80.info/zip/z80-documented.pdf */
    uint8_t val = cpu->A;
    if (cpu->F & Z80_NF) {
        if (((cpu->A & 0xF) > 0x9) || (cpu->F & Z80_HF)) {
            val -= 0x06;
        }
        if ((cpu->A > 0x99) || (cpu->F & Z80_CF)) {
            val -= 0x60;
        }
    }
    else {
        if (((cpu->A & 0xF) > 0x9) || (cpu->F & Z80_HF)) {
            val += 0x06;
        }
        if ((cpu->A > 0x99) || (cpu->F & Z80_CF)) {
            val += 0x60;
        }
    }
    cpu->F &= Z80_CF|Z80_NF;
    cpu->F |= (cpu->A > 0x99) ? Z80_CF:0;
    cpu->F |= (cpu->A^val) & Z80_HF;
    cpu->F |= cpu->szp[val];
    cpu->A = val;
}

static void _z80_cpl(z80* cpu) {
    cpu->A ^= 0xFF;
    cpu->F = (cpu->F&(Z80_SF|Z80_ZF|Z80_PF|Z80_CF))|Z80_HF|Z80_NF|(cpu->A&(Z80_YF|Z80_XF));
}

static void _z80_scf(z80* cpu) {
    cpu->F = (cpu->F&(Z80_SF|Z80_ZF|Z80_YF|Z80_XF|Z80_PF))|Z80_CF|(cpu->A&(Z80_YF|Z80_XF));
}

static void _z80_ccf(z80* cpu) {
    cpu->F = ((cpu->F&(Z80_SF|Z80_ZF|Z80_YF|Z80_XF|Z80_PF|Z80_CF))|((cpu->F&Z80_CF)<<4)|(cpu->A&(Z80_YF|Z80_XF)))^Z80_CF;
}

/*-- DECODE HELPERS ----------------------------------------------------------*/
static uint8_t _z80_rot(z80* cpu, uint8_t val, uint8_t type) {
    /* select rotate/shift operation */
    switch (type) {
        case 0: return _z80_rlc(cpu, val);
        case 1: return _z80_rrc(cpu, val);
        case 2: return _z80_rl(cpu, val);
        case 3: return _z80_rr(cpu, val);
        case 4: return _z80_sla(cpu, val);
        case 5: return _z80_sra(cpu, val);
        case 6: return _z80_sll(cpu, val);
        case 7: return _z80_srl(cpu, val);
    }
    return 0;   /* can't happen */
}

static uint16_t _z80_addr(z80* cpu) {
    /* get effective address (HL, or IX+d, or IY+d), updates WZ if indexed */
    if (cpu->ddfd) {
        /* indexed instruction IX+d or IY+d */
        int8_t d = (int8_t) _READ(cpu->PC++);
        cpu->WZ = cpu->r16[cpu->ri16sp[2]] + d;
        return cpu->WZ;
    }
    else {
        return cpu->HL;
    }
}

static void _z80_patch_ix(z80* cpu) {
    /* patch HL register indirection index to IX */
    cpu->ri8[4] = 9;    /* L = IXL */
    cpu->ri8[5] = 8;    /* H = IXH */
    cpu->ri16af[2] = 4; /* HL = IX */
    cpu->ri16sp[2] = 4; /* HL = IX */
    cpu->ddfd = true;
}

static void _z80_patch_iy(z80* cpu) {
    /* patch HL register indirection index to IY */
    cpu->ri8[4] = 11;   /* L = IYL */
    cpu->ri8[5] = 10;   /* H = IYH */
    cpu->ri16af[2] = 5; /* HL = IY */
    cpu->ri16sp[2] = 5; /* HL = IY */
    cpu->ddfd = true;
}

static void _z80_unpatch_ixiy(z80* cpu) {
    cpu->ri8[4] = 5;    /* L = L */
    cpu->ri8[5] = 4;    /* H = H */
    cpu->ri16af[2] = 2; /* HL = HL */
    cpu->ri16sp[2] = 2; /* HL = HL */
    cpu->ddfd = false;
}

/*-- INSTRUCTION DECODERS ----------------------------------------------------*/

/* CB prefix instruction decoder */
static void _z80_cb_op(z80* cpu) {
    /*
        Split opcode into bit groups:
        |xx|yyy|zzz|
    */
    const uint8_t op = cpu->opcode;
    const uint8_t x = op>>6;
    const uint8_t y = (op>>3) & 7;
    const uint8_t z = op & 7;
    if (x == 0) {
        if (z == 6) {
            /* ROT (HL); ROT (IX+d); ROT (IY+d) */
            uint16_t addr = _z80_addr(cpu);
            _EXTICKS(5);
            _WRITE(addr, _z80_rot(cpu, _READ(addr), y));
            return;
        }
        else if (cpu->ddfd) {
            /* undocumented: ROT (IX+d),r; ROT (IY+d),r */
            // FIXME
        }
        else {
            /* ROT r */
            cpu->r8[cpu->ri8[z]] = _z80_rot(cpu, cpu->r8[cpu->ri8[z]], y);
            return;
        }
    }
    else if (x == 1) {

    }
    else if (x == 2) {

    }
    else if (x == 3) {

    }
    assert(false);
}

/* ED prefix instruction decoder */
static void _z80_ed_op(z80* cpu) {
    /*
        Split opcode into bit groups:
        |xx|yyy|zzz|
        |xx|ppq|zzz|
    */
    const uint8_t op = cpu->opcode;
    const uint8_t x = op>>6;
    const uint8_t y = (op>>3) & 7;
    const uint8_t z = op & 7;
    const uint8_t p = y>>1;
    const uint8_t q = y & 1;

    if (x == 2) {
        /* block instructions */
        switch (z) {
            case 0:
                switch (y) {
                    case 4: _z80_ldi(cpu); return;
                    case 5: _z80_ldd(cpu); return;
                    case 6: _z80_ldir(cpu); return;
                    case 7: _z80_lddr(cpu); return;
                }
                break;
            case 1:
                switch (y) {
                    case 4: _z80_cpi(cpu); return;
                    case 5: _z80_cpd(cpu); return;
                    case 6: _z80_cpir(cpu); return;
                    case 7: _z80_cpdr(cpu); return;
                }
                break;
            case 2:
                switch (y) {
                    case 4: _z80_ini(cpu); return;
                    case 5: _z80_ind(cpu); return;
                    case 6: _z80_inir(cpu); return;
                    case 7: _z80_indr(cpu); return;

                }
                break;
            case 3:
                switch (y) {
                    case 4: _z80_outi(cpu); return;
                    case 5: _z80_outd(cpu); return;
                    case 6: _z80_otir(cpu); return;
                    case 7: _z80_otdr(cpu); return;
                }
                break;
        }
    }
    else if (x == 1) {
        /* misc ED ops */
        switch (z) {
            case 0: /* IN r,(C) */ break;
            case 1: /* OUT (C),r */ break;
            case 2: /* SBC/ADC HL,rr */ break;
            case 3: /* 16-bit immediate load/store */
                cpu->Z = _READ(cpu->PC++);
                cpu->W = _READ(cpu->PC++);
                if (q == 0) {
                    /* LD (nn),dd */
                    uint16_t val = cpu->r16[cpu->ri16sp[p]];
                    _WRITE(cpu->WZ++, (uint8_t)val);
                    _WRITE(cpu->WZ, (uint8_t)(val>>8));
                }
                else {
                    /* LD dd,(nn) */
                    uint8_t l = _READ(cpu->WZ++);
                    uint8_t h = _READ(cpu->WZ);
                    cpu->r16[cpu->ri16sp[p]] = (h<<8) | l;
                }
                return;
            case 4: /* NEG */
                _z80_neg8(cpu);
                return;
            case 5: /* RETN; RETI */
            case 6: /* IM m */
                _z80_im(cpu, y);
                return;
            case 7:
                switch (y) {
                    case 0: /* LD I,A */
                        cpu->I=cpu->A; _TICK();
                        return;
                    case 1: /* LD R,A */
                        cpu->R=cpu->A; _TICK();
                        return;
                    case 2: /* LD A,I */
                        cpu->A=cpu->I; cpu->F=_z80_sziff2(cpu,cpu->I)|(cpu->F&Z80_CF);
                        _TICK();
                        return;
                    case 3: /* LD A,R */
                        /* FIXME: "If an interrupt occurs during execution of this instruction, the parity flag contains a 0." */
                        cpu->A=cpu->R; cpu->F=_z80_sziff2(cpu,cpu->R)|(cpu->F&Z80_CF);
                        _TICK();
                        return;
                    case 4: /* RRD */
                        _z80_rrd(cpu);
                        return;
                    case 5: /* RLD */
                        _z80_rld(cpu);
                        return;
                    case 6: /* NOP (ED) */ return;
                    case 7: /* NOP (ED) */ return;
                }
                break;
        }
    }
    assert(false);
}

/* main instruction decoder */
static void _z80_op(z80* cpu) {
    /*
        Split opcode into bit groups:
        |xx|yyy|zzz|
        |xx|ppq|zzz|
    */
    const uint8_t op = cpu->opcode;
    const uint8_t x = op>>6;
    const uint8_t y = (op>>3) & 7;
    const uint8_t z = op & 7;
    const uint8_t p = y>>1;
    const uint8_t q = y & 1;
    if (x == 1) {
        /* block 1: 8-bit loads and HALT */
        if (y == 6) {
            if (z == 6) {
                /* special case: LD HL,HL is HALT */
                _z80_halt(cpu); return;
            }
            else {
                /* LD (HL),r; LD (IX+d),r; LD (IY+d),r */
                /* direct r8 access is not a bug, since H/L must be accessed in indexed ops */
                uint16_t addr = _z80_addr(cpu);
                _EXTICKS(5);
                _WRITE(addr, cpu->r8[z^1]); return;
            }
        }
        else {
            /* LD r,(HL); LD r,(IX+d); LD r,(IY+d); LD r,r' */
            /* direct r8 access is not a bug, since H/L must be accessed in indexed ops */
            if (z == 6) {
                uint16_t addr = _z80_addr(cpu);
                _EXTICKS(5);
                cpu->r8[cpu->ri8[y]] = _READ(addr);
            }
            else {
                cpu->r8[cpu->ri8[y]] = cpu->r8[cpu->ri8[z]];
            }
            return;
        }
    }
    else if (x == 2) {
        /* block 2: 8-bit register ALU instructions */
        uint8_t val;
        if (z == 6) {
            uint16_t addr = _z80_addr(cpu);
            _EXTICKS(5);
            val = _READ(addr);
        }
        else {
            val = cpu->r8[cpu->ri8[z]];
        }
        switch (y) {
            case 0: _z80_add8(cpu, val); return;
            case 1: _z80_adc8(cpu, val); return;
            case 2: _z80_sub8(cpu, val); return;
            case 3: _z80_sbc8(cpu, val); return;
            case 4: _z80_and8(cpu, val); return;
            case 5: _z80_xor8(cpu, val); return;
            case 6: _z80_or8(cpu, val); return;
            case 7: _z80_cp8(cpu, val); return;
        }
    }
    else if (x == 0) {
        /* block 0: misc instructions */
        switch (z) {
            case 0:
                switch (y) {
                    case 0: /* NOP */
                        return;
                    case 1: /* EX AF,AF' */ break;
                    case 2: /* DJNZ */
                        _z80_djnz(cpu);
                        return;
                    case 3: /* JR d */ break;
                    default: /* JR cc,d */
                        _z80_jrcc(cpu, y);
                        return;
                }
                break;
            case 1:
                if (q == 0) {
                    /* 16-bit immediate loads */
                    cpu->Z=_READ(cpu->PC++); cpu->W=_READ(cpu->PC++);
                    cpu->r16[cpu->ri16sp[p]]=cpu->WZ;
                    return;
                }
                else {
                    /* ADD HL,rr; ADD IX,rr; ADD IY,rr */
                }
                break;
            case 2:
                /* indirect loads */
                switch (y) {
                    case 0: /* LD (BC),A */
                        cpu->WZ=cpu->BC; _WRITE(cpu->WZ++,cpu->A); cpu->W=cpu->A;
                        return;
                    case 1: /* LD A,(BC) */
                        cpu->WZ=cpu->BC; cpu->A=_READ(cpu->WZ++);
                        return;
                    case 2: /* LD (DE),A */
                        cpu->WZ=cpu->DE; _WRITE(cpu->WZ++,cpu->A); cpu->W=cpu->A;
                        return;
                    case 3: /* LD A,(DE) */
                        cpu->WZ=cpu->DE; cpu->A=_READ(cpu->WZ++);
                        return;
                    case 4: /* LD (nn),HL|IX|IY */
                        cpu->Z=_READ(cpu->PC++); cpu->W=_READ(cpu->PC++);
                        _WRITE(cpu->WZ++,cpu->r8[cpu->ri8[5]]); _WRITE(cpu->WZ,cpu->r8[cpu->ri8[4]]);
                        return;
                    case 5: /* LD HL|IX|IY,(nn) */
                        cpu->Z=_READ(cpu->PC++); cpu->W=_READ(cpu->PC++);
                        cpu->r8[cpu->ri8[5]]=_READ(cpu->WZ++); cpu->r8[cpu->ri8[4]]=_READ(cpu->WZ);
                        return;
                    case 6: /* LD (nn),A */
                        cpu->Z=_READ(cpu->PC++); cpu->W=_READ(cpu->PC++);
                        _WRITE(cpu->WZ++,cpu->A); cpu->W=cpu->A;
                        return;
                    case 7: /* LD A,(nn) */
                        cpu->Z=_READ(cpu->PC++); cpu->W=_READ(cpu->PC++);
                        cpu->A=_READ(cpu->WZ++);
                        return;
                }
                break;
            case 3:
                /* 16-bit INC,DEC */
                break;
            case 4:
                /* INC (HL); INC (IX+d); INC (IY+d); INC r */
                if (y == 6) {
                    uint16_t addr = _z80_addr(cpu);
                    _EXTICKS(5);
                    _WRITE(addr, _z80_inc8(cpu, _READ(addr)));
                }
                else {
                    cpu->r8[cpu->ri8[y]] = _z80_inc8(cpu, cpu->r8[cpu->ri8[y]]);
                }
                return;
            case 5:
                /* DEC (HL); DEC (IX+d); DEC (IY+d); DEC r */
                if (y == 6) {
                    uint16_t addr = _z80_addr(cpu);
                    _EXTICKS(5);
                    _WRITE(addr, _z80_dec8(cpu, _READ(addr)));
                }
                else {
                    cpu->r8[cpu->ri8[y]] = _z80_dec8(cpu, cpu->r8[cpu->ri8[y]]);
                }
                return;
            case 6:
                if (y == 6) {
                    /* LD (HL),n; LD (IX+d),n; LD (IY+d),n */
                    uint16_t addr = _z80_addr(cpu);
                    _EXTICKS(2);
                    _READ(cpu->PC++);
                    _WRITE(addr, cpu->DATA);
                    return;
                }
                else {
                    /* LD r,n */
                    _READ(cpu->PC++); cpu->r8[cpu->ri8[y]]=cpu->DATA; return;
                }
                break;
            case 7:
                /* misc ops on A and F */
                switch (y) {
                    case 0: _z80_rlca(cpu); return;
                    case 1: _z80_rrca(cpu); return;
                    case 2: _z80_rla(cpu); return;
                    case 3: _z80_rra(cpu); return;
                    case 4: _z80_dda(cpu); return;
                    case 5: _z80_cpl(cpu); return;
                    case 6: _z80_scf(cpu); return;
                    case 7: _z80_ccf(cpu); return;
                }
                break;
        }
    }
    else if (x == 3) {
        /* block 3: misc and extended instructions */
        switch (z) {
            case 0: /* RET cc */
                _z80_retcc(cpu, y);
                return;
            case 1: /* POP + misc */
                if (q == 0) {
                    /* POP BC/DE/HL/IX/IY/AF */
                }
                else switch (p) {
                    case 0: /* RET */
                        _z80_ret(cpu);
                        return;
                    case 1: /* EXX */ break;
                    case 2: /* JP (HL); JP (IX); JP (IY) */ break;
                    case 3: /* LD SP,HL; LD SP,IX; LD SP,IY */
                        _TICKS(2);
                        cpu->SP = cpu->r16[cpu->ri16sp[2]];
                        return;
                }
                break;
            case 2: /* JP cc,nn */
                _z80_jpcc(cpu, y);
                return;
            case 3:
                /* misc ops and CB prefix */
                switch (y) {
                    case 0: /* JP nn */
                        _z80_jp(cpu);
                        return;
                    case 1: /* CB prefix */
                        _z80_fetch(cpu);
                        _z80_cb_op(cpu);
                        return;
                    case 2: /* OUT (n),A */ break;
                    case 3: /* IN A,(n) */ break;
                    case 4: /* EX (SP),HL; EX (SP),IX; EX (SP),IY */
                    case 5: /* EX DE,HL */
                        _SWAP16(cpu->DE, cpu->HL);
                        return;
                    case 6: /* DI */
                        _z80_di(cpu);
                        return;
                    case 7: /* EI */
                        _z80_ei(cpu);
                        return;
                }
                break;
            case 4: /* CALL cc,nn */
                _z80_callcc(cpu, y);
                return;
            case 5:
                /* PUSH, CALL, extended */
                if (q == 0) {
                    /* PUSH BC/DE/HL/IX/IY/AF */
                    // FIXME
                }
                else switch (p) {
                    case 0: /* CALL nn */
                        _z80_call(cpu);
                        return;
                    case 1: /* DD prefix */
                        if (!cpu->ddfd) {   /* avoid infinite recursion */
                            _z80_patch_ix(cpu);
                            _z80_fetch(cpu);
                            _z80_op(cpu);
                            _z80_unpatch_ixiy(cpu);
                        }
                        return;
                    case 2: /* ED prefix */
                        _z80_fetch(cpu);
                        _z80_ed_op(cpu);
                        return;
                    case 3: /* FD prefix */
                        if (!cpu->ddfd) {   /* avoid infinite recursion */
                            _z80_patch_iy(cpu);
                            _z80_fetch(cpu);
                            _z80_op(cpu);
                            _z80_unpatch_ixiy(cpu);
                        }
                        return;
                }
                break;
            case 6:
                /* ALU n */
                {
                    uint8_t val = _READ(cpu->PC++);
                    switch (y) {
                        case 0: _z80_add8(cpu, val); return;
                        case 1: _z80_adc8(cpu, val); return;
                        case 2: _z80_sub8(cpu, val); return;
                        case 3: _z80_sbc8(cpu, val); return;
                        case 4: _z80_and8(cpu, val); return;
                        case 5: _z80_xor8(cpu, val); return;
                        case 6: _z80_or8(cpu, val); return;
                        case 7: _z80_cp8(cpu, val); return;
                    }
                }
                break;
        }
    }
    /* unhandled/invalid instruction */
    assert(false);
}

void z80_init(z80* cpu, z80_desc* desc) {
    CHIPS_ASSERT(cpu);
    CHIPS_ASSERT(desc);
    CHIPS_ASSERT(desc->tick_func);
    memset(cpu, 0, sizeof(z80));
    cpu->tick = desc->tick_func;
    cpu->context = desc->tick_context;
    for (int i = 0; i < 8; i++) {
        cpu->ri8[i] = i^1;
    }
    for (int i = 0; i < 4; i++) {
        cpu->ri16af[i] = i;
        cpu->ri16sp[i] = i;
    }
    cpu->ri16sp[3] = 6; /* index 6 is SP */
    /* init SZP flags table */
    for (int val = 0; val < 256; val++) {
        int p = 0;
        for (int i = 0; i < 8; i++) {
            if (val & (1<<i)) p++;
        }
        uint8_t f = val ? (val & Z80_SF) : Z80_ZF;
        f |= (val & (Z80_YF|Z80_XF));   // undocumented flag bits 3 and 5
        f |= p & 1 ? 0 : Z80_PF;
        cpu->szp[val] = f;
    }
}

uint32_t z80_step(z80* cpu) {
    CHIPS_ASSERT(0 == cpu->ddfd);
    cpu->ticks = 0;
    if (cpu->ei_pending) {
        cpu->iff1 = cpu->iff2 = true;
        cpu->ei_pending = false;
    }
    _z80_fetch(cpu);
    _z80_op(cpu);
    return cpu->ticks;
}

uint32_t z80_run(z80* cpu, uint32_t t) {
    // FIXME
    return 0;
}

/* control pin functions */
void z80_on(z80* cpu, uint16_t pins) {
    cpu->CTRL |= pins;
}
void z80_off(z80* cpu, uint16_t pins) {
    cpu->CTRL &= ~pins;
}
bool z80_any(z80* cpu, uint16_t pins) {
    return (cpu->CTRL & pins) != 0;
}
bool z80_all(z80* cpu, uint16_t pins) {
    return (cpu->CTRL & pins) == pins;
}

#undef _SZ
#undef _SZYXCH
#undef _ADD_FLAGS
#undef _SUB_FLAGS
#undef _CP_FLAGS
#undef _ON
#undef _OFF
#undef _TICK
#undef _TICKS
#undef _READ
#undef _WRITE
#undef _SWAP16
#endif /* CHIPS_IMPL */

#ifdef __cplusplus
} /* extern "C" */
#endif
