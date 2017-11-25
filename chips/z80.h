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

#endif /* CHIPS_IMPL */

#ifdef __cplusplus
} /* extern "C" */
#endif
