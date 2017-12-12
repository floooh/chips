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

/*
    The tick function is called for one or multiple time cycles
    connects the Z80 to the outside world (usually one call
    of the tick function corresponds to one machine cycle, 
    but this is not always the case). The CPU pins
    (control pins, data bus and address bus) are communicated
    as a single 64-bit integer. The tick callback function
    must inspect the pins, and modify the pin state 
    accordingly:

    - if MREQ|RD is set, this is a memory read cycle, the tick
      callback must read the memory at the location indicated by
      the address bus, and set the data bus bits with the memory content
    - if MREQ|WR is set, this is a memory write cycle, the tick
      callback must change the memory content at the location of
      the address bus to the value of the data bus pins
    - if IORQ|RD is set, this is a device input cycle, the 16-bit
      port number is on the address bus (usually only the lower 8-bit
      of this are used), and the tick callback must set the data bus
      pins to the port input value
    - if IORQ|WR is set, this is a device output cycle, the 16-bit
      port number is on the address bus, and the 8-bit output value
      is on the data bus
    - to inject a wait state, set the WAIT pin in the tick callback,
      note that the WAIT pin is only checked during read or write
      cycles
    - to request an interrupt, set the INT pin, note that the 
      state of the interrupt pin is tested at the end of an instruction

    The pin-layout of the 64-bit integer is as follows:

    - bits 0..15:   address bus
    - bits 16..23:  data bus
    - bits 24..36:  control pins
    - bits 37..40:  interrupt system 'virtual pins'
*/
typedef uint64_t (*tick_callback)(int num_ticks, uint64_t pins);

/*--- address lines ---*/
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

/*--- data lines ------*/
#define Z80_D0  (1ULL<<16)
#define Z80_D1  (1ULL<<17)
#define Z80_D2  (1ULL<<18)
#define Z80_D3  (1ULL<<19)
#define Z80_D4  (1ULL<<20)
#define Z80_D5  (1ULL<<21)
#define Z80_D6  (1ULL<<22)
#define Z80_D7  (1ULL<<23)

/*--- pin functions ---*/

/* system control pins */
#define  Z80_M1    (1ULL<<24)       /* machine cycle 1 */
#define  Z80_MREQ  (1ULL<<25)       /* memory request */
#define  Z80_IORQ  (1ULL<<26)       /* input/output request */
#define  Z80_RD    (1ULL<<27)       /* read */
#define  Z80_WR    (1ULL<<28)       /* write */
#define  Z80_RFSH  (1ULL<<29)       /* refresh */

/* CPU control pins */
#define  Z80_HALT  (1ULL<<30)       /* halt state */
#define  Z80_WAIT  (1ULL<<31)       /* wait state */
#define  Z80_INT   (1ULL<<32)       /* interrupt request */
#define  Z80_NMI   (1ULL<<33)       /* non-maskable interrupt */
#define  Z80_RESET (1ULL<<34)       /* reset */

/* CPU bus control pins */
#define  Z80_BUSREQ (1ULL<<35)      /* bus request */
#define  Z80_BUSACK (1ULL<<36)      /* bus acknowledge */

/* interrupt-related 'virtual pins', these don't exist on the Z80 */
#define Z80_IEIO    (1ULL<<37)      /* unified daisy chain 'Interrupt Enable In+Out' */
#define Z80_RETI    (1ULL<<38)      /* cpu has decoded a RETI instruction */

/* bit mask for all CPU bus pins */
#define Z80_PIN_MASK ((1ULL<<40)-1)

/*--- status indicator flags ---*/
#define Z80_CF (1<<0)           /* carry */
#define Z80_NF (1<<1)           /* add/subtract */
#define Z80_VF (1<<2)           /* parity/overflow */
#define Z80_PF Z80_VF
#define Z80_XF (1<<3)           /* undocumented bit 3 */
#define Z80_HF (1<<4)           /* half carry */
#define Z80_YF (1<<5)           /* undocumented bit 5 */
#define Z80_ZF (1<<6)           /* zero */
#define Z80_SF (1<<7)           /* sign */

/* Z80 CPU state */
typedef struct {
    /* tick function */
    tick_callback tick;
    /* the CPU pins (control, address and data) */
    uint64_t PINS;
    /* program counter */
    uint16_t PC;
    /* memptr */
    uint16_t WZ;
    /* NOTE: union layout assumes little-endian CPU */
    union { uint16_t AF; struct { uint8_t F, A; }; };
    union { uint16_t HL; struct { uint8_t L, H; }; };
    union { uint16_t IX; struct { uint8_t IXL, IXH; }; };
    union { uint16_t IY; struct { uint8_t IYL, IYH; }; };
    union { uint16_t BC; struct { uint8_t C, B; }; };
    union { uint16_t DE; struct { uint8_t E, D; }; };
    union { uint16_t IR; struct { uint8_t R, I; }; };
    /* alternate register set */
    uint16_t BC_, DE_, HL_, AF_;
    /* stack pointer */
    uint16_t SP;
    /* interrupt mode (0, 1 or 2) */
    uint8_t IM;
    /* interrupt enable bits */
    bool IFF1, IFF2;

    /* enable-interrupt pending for start of next instruction */
    bool ei_pending;
} z80;

typedef struct {
    tick_callback tick_cb;
} z80_desc;

/* initialize a new z80 instance */
extern void z80_init(z80* cpu, z80_desc* desc);
/* reset an existing z80 instance */
extern void z80_reset(z80* cpu);
/* execute the next instruction, return number of time cycles */
extern uint32_t z80_step(z80* cpu);
/* execute instructions for (at least) given number of ticks, return executed ticks */
extern uint32_t z80_run(z80* cpu, uint32_t ticks);

/* extract 16-bit address bus from 64-bit pins */
#define Z80_ADDR(p) (p&0xFFFFULL)
/* merge 16-bit address bus value into 64-bit pins */
#define Z80_SET_ADDR(p,a) {p=((p&~0xFFFFULL)|(a&0xFFFFULL));}
/* extract 8-bit data bus from 64-bit pins */
#define Z80_DATA(p) ((p&0xFF0000ULL)>>16)
/* merge 8-bit data bus value into 64-bit pins */
#define Z80_SET_DATA(p,d) {p=((p&~0xFF0000ULL)|((d<<16)&0xFF0000ULL));}

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

#include "_z80_opcodes.h"

void z80_init(z80* c, z80_desc* desc) {
    CHIPS_ASSERT(c);
    CHIPS_ASSERT(desc);
    CHIPS_ASSERT(desc->tick_cb);
    memset(c, 0, sizeof(z80));
    z80_reset(c);
    c->tick = desc->tick_cb;
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
    c->WZ = 0xFFFF;
    /* after power-on or reset, R is set to 0 (see z80-documented.pdf) */
    c->IR = 0;
    c->ei_pending = false;
}

uint32_t z80_run(z80* __restrict c, uint32_t t) {
    uint32_t ticks_executed = 0;
    while (ticks_executed < t) {
        ticks_executed += z80_step(c);
    }
    return ticks_executed;
}

#endif /* CHIPS_IMPL */

#ifdef __cplusplus
} /* extern "C" */
#endif
