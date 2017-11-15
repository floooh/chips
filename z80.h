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
        uint8_t r[8];   /* to access with register index, flip bit 0 */
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
} z80;

#ifdef __cplusplus
} /* extern "C" */
#endif
