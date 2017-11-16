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
typedef struct {
    /* main register set */
    union {
        uint8_t r8[8];   /* to access with register index, flip bit 0 */
        uint16_t r16[4];
        struct {
            /* this assumes we're on a little-endian CPU */
            union { uint16_t BC; struct { uint8_t C, B; }; };
            union { uint16_t DE; struct { uint8_t E, D; }; };
            union { uint16_t HL; struct { uint8_t L, H; }; };
            union { uint16_t AF; struct { uint8_t F, A; }; };
        };
    };
    /* alternate register set */
    uint16_t BC_, DE_, HL_, AF_;
    /* undocumented WZ and shadow WZ */
    uint16_t WZ, WZ_;
    /* index registers */
    union { uint16_t IX; struct { uint8_t IXL, IXH; }; };
    union { uint16_t IY; struct { uint8_t IYL, IYH; }; };
    /* stack pointer */
    uint16_t SP;
    /* program counter */
    uint16_t PC;
    /* interrupt vector */
    uint8_t I;
    /* memory refresh */
    uint8_t R;
    /* control pins */
    uint16_t ctrl;
    /* address bus pins */
    uint16_t addr;
    /* data bus pins */
    uint8_t data;

    /* interrupt mode IM0, IM1 or IM2 */
    uint8_t im;
    /* interrupt enable flip-flips */
    bool imm1, imm2;

    /* tick function and context data */
    void (*tick_func)(z80* cpu, void* context);

} z80;

typedef struct {
    void* tick_context;
    void (*tick_func)(z80* cpu, void* context);
} z80_desc;

/* initialize a new z80 instance */
extern void z80_init(z80* cpu, z80_desc* desc);
/* execute the next instruction, return number of time cycles */
extern uint32_t z80_step(z80* cpu, z80_desc* desc);
/* execute instructions for up to 'ticks' time cycles, return executed time cycles */
extern uint32_t z80_run(z80* cpu, uint32_t ticks);
/* set one or more pins to active state */
extern void z80_on(z80* cpu, uint16_t pins);
/* set one or more pins to cleared state */
extern void z80_off(z80* cpu, uint16_t pins);


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

#define _Z80_SZ(val) ((val&0xFF)?(val&Z80_SF):Z80_ZF)
#define _Z80_SZYXCH(acc,val,res) (_Z80_SZ(res)|(res&(Z80_YF|Z80_XF))|((res>>8)&Z80_CF)|((acc^val^res)&Z80_HF))
#define _Z80_ADD_FLAGS(acc,val,res) (_Z80_SZYXCH(acc,val,res)|((((val^acc^0x80)&(val^res))>>5)&Z80_VF))
#define _Z80_SUB_FLAGS(acc,val,res) (Z80_NF|_Z80_SZYXCH(acc,val,res)|((((val^acc)&(res^acc))>>5)&Z80_VF))
#define _Z80_CP_FLAGS(acc,val,res) (Z80_NF|(_Z80_SZ(res)|(val&(Z80_YF|Z80_XF))|((res>>8)&Z80_CF)|((acc^val^res)&Z80_HF))|((((val^acc)&(res^acc))>>5)&Z80_VF)) 

/*-- ALU functions -----------------------------------------------------------*/
static void _z80_add8(z80* cpu, uint8_t val) {
    int res = cpu->A + val;
    cpu->F = _Z80_ADD_FLAGS(cpu->A, val, res);
    cpu->A = (uint8_t) res;
}

static void _z80_adc8(z80* cpu, uint8_t val) {
    int res = cpu->A + val + (cpu->F & Z80_CF);
    cpu->F = _Z80_ADD_FLAGS(cpu->A, val, res);
    cpu->A = (uint8_t) res;
}

static void _z80_sub8(z80* cpu, uint8_t val) {
    int res = (int)cpu->A - (int)val;
    cpu->F = _Z80_SUB_FLAGS(cpu->A, val, res);
    cpu->A = (uint8_t) res;
}

static void _z80_sbc8(z80* cpu, uint8_t val) {
    int res = (int)cpu->A - (int)val - (cpu->F & Z80_CF);
    cpu->F = _Z80_SUB_FLAGS(cpu->A, val, res);
    cpu->A = (uint8_t) res;
}

static void _z80_cp8(z80* cpu, uint8_t val) {
    /* NOTE: XF|YF are set from val, not from result */
    int res = (int)cpu->A - (int)val;
    cpu->F = _Z80_CP_FLAGS(cpu->A, val, res);
}

static void _z80_neg8(z80* cpu) {
    uint8_t val = cpu->A;
    cpu->A = 0;
    _z80_sub(cpu, val);
}

void z80_init(z80* cpu, z80_desc* desc) {
    CHIPS_ASSERT(cpu);
    CHIPS_ASSERT(desc);
    CHIPS_ASSERT(desc->tick_func);
    memset(cpu, 0, sizeof(z80);
    cpu->tick_func = desc->tick_func;
    cpu->tick_context = desc->tick_context;
}

uint32_t z80_step(z80* cpu) {
    // FIXME
    return 0;
}

uint32_t z80_run(z80* cpu, uint32_t t) {
    // FIXME
    return 0;
}

void z80_on(z80* cpu, uint16_t pins) {
    // FIXME
}

void z80_off(z80* cpu, uint16_t pins) {
    // FIXME
}

#endif /* CHIPS_IMPL */

#ifdef __cplusplus
} /* extern "C" */
#endif
