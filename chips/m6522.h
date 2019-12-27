#pragma once
/*#
    # m6522.h

    Header-only MOS 6522 VIA emulator written in C.

    Do this:
    ~~~C
    #define CHIPS_IMPL
    ~~~
    before you include this file in *one* C or C++ file to create the 
    implementation.

    Optionally provide the following macros with your own implementation
    
    ~~~C
    CHIPS_ASSERT(c)
    ~~~
        your own assert macro (default: assert(c))

    ## Emulated Pins
    *************************************
    *           +-----------+           *
    *   RS0 --->|           |<--> CA1   *
    *        ...|           |<--> CA2   *
    *   RS3 --->|           |           *
    *           |           |<--> PA0   *
    *   CS1 --->|           |...        *
    *   CS2 --->|           |<--> PA7   *
    *           |           |           *
    *    D0 <-->|           |           *
    *        ...|   m6522   |<--> CB1   *
    *    D7 <-->|           |<--> CB2   *
    *           |           |           *
    *    RW --->|           |<--> PB0   *
    *   IRQ <---|           |...        *
    *           |           |<--> PB7   *
    *           |           |           *
    *           +-----------+           *
    *************************************
    
    ## How to use

    Call m6522_init() to initialize a new m6522_t instance (note that
    there is no m6522_desc_t struct:

    ~~~C
    m6522_t via;
    m6522_init(&via);
    ~~~

    In your emulated system's tick function, call m6522_tick() in
    each emulated tick, and m6522_iorq() when VIA registers are
    written to or read from.

    The m6522_tick() function inspects the following pins:

    - the port A I/O pins PA0..PA7
    - the port A control pins CA1 and CA2
    - the port B I/O pins PB0..PB7
    - the port B control pins CB1 and CB2

    On return m6522_tick() returns a modified pin mask where the following
    pins might have changed state:

    - the IRQ pin
    - the port A I/O pins PA0..PA7
    - the port A control pins CA1 and CA2
    - the port B I/O pins PB0..PB7
    - the port B control pins CB1 and CB2

    Here's a simple example of m6522_tick() where the VIA chip acts as
    a keyboard controller for a 8x8 "active-low" keyboard matrix.

    All VIA port A pins have been configured as inputs and are connected
    to the keyboard matrix lines, and all VIA port B are set as
    outputs and are connected to the keyboard matrix columns. The keyboard
    handler code in the operating system would write to the port B register
    and then read from the port A register.

    The m6522_tick() function call would look like this:

    ~~~C
    // extract shared CPU/VIA pins and initialize the port A
    // input pins with the current keyboard matrix row status
    uint64_t via_pins = cpu_pins & M6502_PINS;
    uint8_t via_pa = ~kbd_scan_lines(&sys->kbd);
    M6522_SET_PA(via_pins, via_pa);
    // call the VIA tick function
    via_pins = m6522_tick(&sys->via, via_pins);
    // forward the PB output pins to the keyboard matrix for the next tick
    uint8_t kbd_cols = ~M6522_GET_PB(via_pins);
    kbs_set_active_columns(&sys->kbd, kbd_cols);
    // while at it, forward the IRQ pin state to the cpu pins
    cpu_pins = (cpu_pins & ~M6502_IRQ) | (via_pins & M6522_IRQ);
    ~~~

    Elsewhere in the same tick function, call m6522_iorq() when VIA registers
    must be read or written.

    The relevant pins for the m6522_iorq() call are:

    - register select pins RS0..RS3, these are shared with the
      first 4 address bus pins, since that's the usual configuration
    - the chip select pins CS1 and CS2 (CS1 must be active and CS2
      inactive for the chip to respond)
    - the RW pin (usually connected to the CPU's RW pin) to select
      between a register write and register read
    - the data bus pins DB0..DB7, these are usually shared with the
      CPU's data bus pins.

    The m6522_iorq() function returns a new pin mask where only the
    data bus pins DB0..DB7 might have changed (in case of a register
    read access).

    Since most pin positions are shared with the CPU the m6522_iorq()
    call usually looks very simple. For instance here's how it looks for
    the VIA-2 on a VIC-20 computer:

    ~~~C
    // address decoding
    const uint16_t addr = M6502_GET_ADDR(cpu_pins);
    if ((addr & 0xFC00) == 0x9000) {
        // we're in the system's memory-mapped IO area, VIA-2
        // is selected when the A5 address pin is active:
        if (addr & M6502_A5) {
            // build shared pin mask for VIA and set the CS1 pin
            uint64_t via_pins = (cpu_pins & M6502_PIN_MASK) | M6522_CS1;
            // the returned pin mask can be assigned back to the CPU pins:
            cpu_pins = m6522_iorq(&sys->via_2, via_pins) & M6502_PIN_MASK;
        }
    }
    ~~~

    Finally, to reset a m6522_t instance, call m6522_reset():

    ~~~C
    m6522_reset(&sys->via);
    ~~~

    ## zlib/libpng license

    Copyright (c) 2018 Andre Weissflog
    This software is provided 'as-is', without any express or implied warranty.
    In no event will the authors be held liable for any damages arising from the
    use of this software.
    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:
        1. The origin of this software must not be misrepresented; you must not
        claim that you wrote the original software. If you use this software in a
        product, an acknowledgment in the product documentation would be
        appreciated but is not required.
        2. Altered source versions must be plainly marked as such, and must not
        be misrepresented as being the original software.
        3. This notice may not be removed or altered from any source
        distribution. 
#*/
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* register select same as lower 4 shared address bus bits */
#define M6522_RS0       (1ULL<<0)
#define M6522_RS1       (1ULL<<1)
#define M6522_RS2       (1ULL<<2)
#define M6522_RS3       (1ULL<<3)
#define M6522_RS_PINS   (0x0FULL)

/* data bus pins shared with CPU */
#define M6522_D0        (1ULL<<16)
#define M6522_D1        (1ULL<<17)
#define M6522_D2        (1ULL<<18)
#define M6522_D3        (1ULL<<19)
#define M6522_D4        (1ULL<<20)
#define M6522_D5        (1ULL<<21)
#define M6522_D6        (1ULL<<22)
#define M6522_D7        (1ULL<<23)
#define M6522_DB_PINS   (0xFF0000ULL)

/* control pins shared with CPU */
#define M6522_RW        (1ULL<<24)      /* same as M6502_RW */
#define M6522_IRQ       (1ULL<<26)      /* same as M6502_IRQ */

/* control pins */
#define M6522_CS1       (1ULL<<40)      /* chip-select 1, to select: CS1 high, CS2 low */
#define M6522_CS2       (1ULL<<41)      /* chip-select 2 */

#define M6522_CA1       (1ULL<<42)      /* peripheral A control 1 */
#define M6522_CA2       (1ULL<<43)      /* peripheral A control 2 */
#define M6522_CB1       (1ULL<<44)      /* peripheral B control 1 */
#define M6522_CB2       (1ULL<<45)      /* peripheral B control 2 */
#define M6522_CA_PINS   (M6522_CA1|M6522_CA2)
#define M6522_CB_PINS   (M6522_CB1|M6522_CB2)

/* peripheral A port */
#define M6522_PA0       (1ULL<<48)
#define M6522_PA1       (1ULL<<49)
#define M6522_PA2       (1ULL<<50)
#define M6522_PA3       (1ULL<<51)
#define M6522_PA4       (1ULL<<52)
#define M6522_PA5       (1ULL<<53)
#define M6522_PA6       (1ULL<<54)
#define M6522_PA7       (1ULL<<55)
#define M6522_PA_PINS   (M6522_PA0|M6522_PA1|M6522_PA2|M6522_PA3|M6522_PA4|M6522_PA5|M6522_PA6|M6522_PA7)

/* peripheral B port */
#define M6522_PB0       (1ULL<<56)
#define M6522_PB1       (1ULL<<57)
#define M6522_PB2       (1ULL<<58)
#define M6522_PB3       (1ULL<<59)
#define M6522_PB4       (1ULL<<60)
#define M6522_PB5       (1ULL<<61)
#define M6522_PB6       (1ULL<<62)
#define M6522_PB7       (1ULL<<63)
#define M6522_PB_PINS   (M6522_PB0|M6522_PB1|M6522_PB2|M6522_PB3|M6522_PB4|M6522_PB5|M6522_PB6|M6522_PB7)

/* register indices */
#define M6522_REG_RB        (0)     /* input/output register B */
#define M6522_REG_RA        (1)     /* input/output register A */
#define M6522_REG_DDRB      (2)     /* data direction B */
#define M6522_REG_DDRA      (3)     /* data direction A */
#define M6522_REG_T1CL      (4)     /* T1 low-order latch / counter */
#define M6522_REG_T1CH      (5)     /* T1 high-order counter */
#define M6522_REG_T1LL      (6)     /* T1 low-order latches */
#define M6522_REG_T1LH      (7)     /* T1 high-order latches */
#define M6522_REG_T2CL      (8)     /* T2 low-order latch / counter */
#define M6522_REG_T2CH      (9)     /* T2 high-order counter */
#define M6522_REG_SR        (10)    /* shift register */
#define M6522_REG_ACR       (11)    /* auxiliary control register */
#define M6522_REG_PCR       (12)    /* peripheral control register */
#define M6522_REG_IFR       (13)    /* interrupt flag register */
#define M6522_REG_IER       (14)    /* interrupt enable register */
#define M6522_REG_RA_NOH    (15)    /* input/output A without handshake */
#define M6522_NUM_REGS      (16)

/* PCR test macros (MAME naming) */
#define M6522_PCR_CA1_LOW_TO_HIGH(c)   (c->pcr & 0x01)
#define M6522_PCR_CA1_HIGH_TO_LOW(c)   (!(c->pcr & 0x01))
#define M6522_PCR_CB1_LOW_TO_HIGH(c)   (c->pcr & 0x10)
#define M6522_PCR_CB1_HIGH_TO_LOW(c)   (!(c->pcr & 0x10))
#define M6522_PCR_CA2_INPUT(c)         (!(c->pcr & 0x08))
#define M6522_PCR_CA2_LOW_TO_HIGH(c)   ((c->pcr & 0x0c) == 0x04)
#define M6522_PCR_CA2_HIGH_TO_LOW(c)   ((c->pcr & 0x0c) == 0x00)
#define M6522_PCR_CA2_IND_IRQ(c)       ((c->pcr & 0x0a) == 0x02)
#define M6522_PCR_CA2_OUTPUT(c)        (c->pcr & 0x08)
#define M6522_PCR_CA2_AUTO_HS(c)       ((c->pcr & 0x0c) == 0x08)
#define M6522_PCR_CA2_HS_OUTPUT(c)     ((c->pcr & 0x0e) == 0x08)
#define M6522_PCR_CA2_PULSE_OUTPUT(c)  ((c->pcr & 0x0e) == 0x0a)
#define M6522_PCR_CA2_FIX_OUTPUT(c)    ((c->pcr & 0x0c) == 0x0c)
#define M6522_PCR_CA2_OUTPUT_LEVEL(c)  ((c->pcr & 0x02) >> 1)
#define M6522_PCR_CB2_INPUT(c)         (!(c->pcr & 0x80))
#define M6522_PCR_CB2_LOW_TO_HIGH(c)   ((c->pcr & 0xc0) == 0x40)
#define M6522_PCR_CB2_HIGH_TO_LOW(c)   ((c->pcr & 0xc0) == 0x00)
#define M6522_PCR_CB2_IND_IRQ(c)       ((c->pcr & 0xa0) == 0x20)
#define M6522_PCR_CB2_OUTPUT(c)        (c->pcr & 0x80)
#define M6522_PCR_CB2_AUTO_HS(c)       ((c->pcr & 0xc0) == 0x80)
#define M6522_PCR_CB2_HS_OUTPUT(c)     ((c->pcr & 0xe0) == 0x80)
#define M6522_PCR_CB2_PULSE_OUTPUT(c)  ((c->pcr & 0xe0) == 0xa0)
#define M6522_PCR_CB2_FIX_OUTPUT(c)    ((c->pcr & 0xc0) == 0xc0)
#define M6522_PCR_CB2_OUTPUT_LEVEL(c)  ((c->pcr & 0x20) >> 5)

/* ACR test macros (MAME naming) */
#define M6522_ACR_PA_LATCH_ENABLE(c)      (c->acr & 0x01)
#define M6522_ACR_PB_LATCH_ENABLE(c)      (c->acr & 0x02)
#define M6522_ACR_SR_DISABLED(c)          (!(c->acr & 0x1c))
#define M6522_ACR_SI_T2_CONTROL(c)        ((c->acr & 0x1c) == 0x04)
#define M6522_ACR_SI_O2_CONTROL(c)        ((c->acr & 0x1c) == 0x08)
#define M6522_ACR_SI_EXT_CONTROL(c)       ((c->acr & 0x1c) == 0x0c)
#define M6522_ACR_SO_T2_RATE(c)           ((c->acr & 0x1c) == 0x10)
#define M6522_ACR_SO_T2_CONTROL(c)        ((c->acr & 0x1c) == 0x14)
#define M6522_ACR_SO_O2_CONTROL(c)        ((c->acr & 0x1c) == 0x18)
#define M6522_ACR_SO_EXT_CONTROL(c)       ((c->acr & 0x1c) == 0x1c)
#define M6522_ACR_T1_SET_PB7(c)           (c->acr & 0x80)
#define M6522_ACR_T1_CONTINUOUS(c)        (c->acr & 0x40)
#define M6522_ACR_T2_COUNT_PB6(c)         (c->acr & 0x20)

/* interrupt bits */
#define M6522_IRQ_CA2      (1<<0)
#define M6522_IRQ_CA1      (1<<1)
#define M6522_IRQ_SR       (1<<2)
#define M6522_IRQ_CB2      (1<<3)
#define M6522_IRQ_CB1      (1<<4)
#define M6522_IRQ_T2       (1<<5)
#define M6522_IRQ_T1       (1<<6)
#define M6522_IRQ_ANY      (1<<7)

/* delay-pipeline bit offsets */
#define M6522_PIP_TIMER_COUNT   (0)
#define M6522_PIP_TIMER_LOAD    (8)
#define M6522_PIP_IRQ           (0)

/* I/O port state */
typedef struct {
    uint8_t inpr;
    uint8_t outr;
    uint8_t ddr;
    uint8_t pins;
} m6522_port_t;

/* timer state */
typedef struct {
    uint16_t latch;     /* 16-bit initial value latch */
    uint16_t counter;   /* 16-bit counter */
    bool t_bit;         /* toggles between true and false when counter underflows */
    bool t_out;         /* true for 1 cycle when counter underflow */
    /* merged delay-pipelines:
        2-cycle 'counter active':   bits 0..7
        1-cycle 'force load':       bits 8..16
    */
    uint16_t pip;
} m6522_timer_t;

/* interrupt state (same as m6522_int_t) */
typedef struct {
    uint8_t ier;            /* interrupt enable register */
    uint8_t ifr;            /* interrupt flag register */
    uint16_t pip;
} m6522_int_t;

/* m6522 state */
typedef struct {
    m6522_port_t pa;
    m6522_port_t pb;
    m6522_timer_t t1;
    m6522_timer_t t2;
    m6522_int_t intr;
    uint8_t acr;        /* auxilary control register */
    uint8_t pcr;        /* peripheral control register */
    uint64_t pins;
} m6522_t;

/* extract 8-bit data bus from 64-bit pins */
#define M6522_GET_DATA(p) ((uint8_t)(p>>16))
/* merge 8-bit data bus value into 64-bit pins */
#define M6522_SET_DATA(p,d) {p=((p&~0xFF0000)|(((d)&0xFF)<<16));}
/* extract port A pins */
#define M6522_GET_PA(p) ((uint8_t)(p>>48))
/* extract port B pins */
#define M6522_GET_PB(p) ((uint8_t)(p>>56))
/* merge port A pins into pin mask */
#define M6522_SET_PA(p,a) {p=(p&0xFF00FFFFFFFFFFFFULL)|(((a)&0xFFULL)<<48);}
/* merge port B pins into pin mask */
#define M6522_SET_PB(p,b) {p=(p&0x00FFFFFFFFFFFFFFULL)|(((b)&0xFFULL)<<56);}
/* merge port A and B pins into pin mask */
#define M6522_SET_PAB(p,a,b) {p=(p&0x0000FFFFFFFFFFFFULL)|(((a)&0xFFULL)<<48)|(((b)&0xFFULL)<<56);}

/* initialize a new 6522 instance */
void m6522_init(m6522_t* m6522);
/* reset an existing 6522 instance */
void m6522_reset(m6522_t* m6522);
/* perform an IO request */
uint64_t m6522_iorq(m6522_t* m6522, uint64_t pins);
/* tick the m6522 */
uint64_t m6522_tick(m6522_t* m6522, uint64_t pins);

#ifdef __cplusplus
} /* extern "C" */
#endif

/*-- IMPLEMENTATION ----------------------------------------------------------*/
#ifdef CHIPS_IMPL
#include <string.h>
#ifndef CHIPS_ASSERT
    #include <assert.h>
    #define CHIPS_ASSERT(c) assert(c)
#endif

static void _m6522_init_port(m6522_port_t* p) {
    p->inpr = 0xFF;
    p->outr = 0;
    p->ddr = 0;
    p->pins = 0;
}

static void _m6522_init_timer(m6522_timer_t* t) {
    t->latch = 0xFFFF;
    t->counter = 0;
    t->t_bit = false;
    t->t_out = false;
    t->pip = 0;
}

static void _m6522_init_interrupt(m6522_int_t* intr) {
    intr->ier = 0;
    intr->ifr = 0;
    intr->pip = 0;
}

void m6522_init(m6522_t* c) {
    CHIPS_ASSERT(c);
    memset(c, 0, sizeof(*c));
    _m6522_init_port(&c->pa);
    _m6522_init_port(&c->pb);
    _m6522_init_timer(&c->t1);
    _m6522_init_timer(&c->t2);
    _m6522_init_interrupt(&c->intr);
    c->acr = 0;
    c->pcr = 0;
    c->t1.latch = 0xFFFF;
    c->t2.latch = 0xFFFF;
}

/*
    "The RESET input clears all internal registers to logic 0, 
    (except T1, T2 and SR). This places all peripheral interface lines
    in the input state, disables the timers, shift registers etc. and 
    disables interrupting from the chip"
*/
void m6522_reset(m6522_t* c) {
    CHIPS_ASSERT(c);
    _m6522_init_port(&c->pa);
    _m6522_init_port(&c->pb);
    _m6522_init_timer(&c->t1);
    _m6522_init_timer(&c->t2);
    _m6522_init_interrupt(&c->intr);
    c->acr = 0;
    c->pcr = 0;
    c->pins = 0;
}

/*--- delay-pipeline macros ---*/
/* set or clear a new state at pipeline pos */
#define _M6522_PIP_SET(pip,offset,pos) {pip|=(1<<(offset+pos));}
#define _M6522_PIP_CLR(pip,offset,pos) {pip&=~(1<<(offset+pos));}
/* reset an entire pipeline */
#define _M6522_PIP_RESET(pip,offset) {pip&=~(0xFF<<offset);}
/* test pipeline state, pos 0 is the 'output bit' */
#define _M6522_PIP_TEST(pip,offset,pos) (0!=(pip&(1<<(offset+pos))))

/*--- port implementation ---*/
static inline bool _m6522_ca1_triggered(m6522_t* c, uint64_t pins) {
    if (M6522_PCR_CA1_LOW_TO_HIGH(c)) {
        /* check for CA1 rising edge */
        return (M6522_CA1 & (pins & (pins ^ c->pins)));
    }
    else {
        /* check for CA1 falling edge */
        return (M6522_CA1 & (~pins & (pins ^ c->pins)));
    }
}

static inline bool _m6522_cb1_triggered(m6522_t* c, uint64_t pins) {
    if (M6522_PCR_CB1_LOW_TO_HIGH(c)) {
        /* check for CA1 rising edge */
        return (M6522_CB1 & (pins & (pins ^ c->pins)));
    }
    else {
        /* check for CA1 falling edge */
        return (M6522_CB1 & (~pins & (pins ^ c->pins)));
    }
}

static inline void _m6522_read_port_pins(m6522_t* c, uint64_t pins) {
    /* with latching enabled, only update input register when CA1 / CB1 goes active */
    if (M6522_ACR_PA_LATCH_ENABLE(c)) {
        if (_m6522_ca1_triggered(c, pins)) {
            c->pa.inpr = M6522_GET_PA(pins);
        }
    }
    else {
        c->pa.inpr = M6522_GET_PA(pins);
    }
    if (M6522_ACR_PB_LATCH_ENABLE(c)) {
        if (_m6522_cb1_triggered(c, pins)) {
            c->pb.inpr = M6522_GET_PB(pins);
        }
    }
    else {
        c->pb.inpr = M6522_GET_PB(pins);
    }
}

static inline uint8_t _m6522_merge_pb7(m6522_t* c, uint8_t data) {
    if (M6522_ACR_T1_SET_PB7(c)) {
        data &= ~(1<<7);
        if (c->t1.t_bit) {
            data |= (1<<7);
        }
    }
    return data;
}

static inline uint64_t _m6522_write_port_pins(m6522_t* c, uint64_t pins) {
    c->pa.pins = (c->pa.inpr & ~c->pa.ddr) | (c->pa.outr & c->pa.ddr);
    c->pb.pins = _m6522_merge_pb7(c, (c->pb.inpr & ~c->pb.ddr) | (c->pb.outr & c->pb.ddr));
    M6522_SET_PAB(pins, c->pa.pins, c->pb.pins);
    return pins;
}

static inline void _m6522_set_intr(m6522_t* c, uint8_t data) {
    c->intr.ifr |= data;
}

static inline void _m6522_clear_intr(m6522_t* c, uint8_t data) {
    c->intr.ifr &= ~data;
    /* clear main interrupt flag? */
    if (0 == (c->intr.ifr & c->intr.ier & 0x7F)) {
        c->intr.ifr &= 0x7F;
        /* cancel any interrupts in the delay pipeline */
        _M6522_PIP_RESET(c->intr.pip, M6522_PIP_IRQ);
    }
}

static inline void _m6522_clear_pa_intr(m6522_t* c) {
    _m6522_clear_intr(c, M6522_IRQ_CA1 | (M6522_PCR_CA2_IND_IRQ(c) ? 0 : M6522_IRQ_CA2));
}

static inline void _m6522_clear_pb_intr(m6522_t* c) {
    _m6522_clear_intr(c, M6522_IRQ_CB1 | (M6522_PCR_CB2_IND_IRQ(c) ? 0 : M6522_IRQ_CB2));
}

static inline void _m6522_write_ier(m6522_t* c, uint8_t data) {
    if (data & 0x80) {
        c->intr.ier |= data & 0x7F;
    }
    else {
        c->intr.ier &= ~(data & 0x7F);
    }
}

static inline void _m6522_write_ifr(m6522_t* c, uint8_t data) {
    if (data & M6522_IRQ_ANY) {
        data = 0x7F;
    }
    _m6522_clear_intr(c, data);
}

void _m6522_tick_t1(m6522_t* c, uint64_t pins) {
    m6522_timer_t* t = &c->t1;

    /* decrement counter? */
    if (_M6522_PIP_TEST(t->pip, M6522_PIP_TIMER_COUNT, 0)) {
        t->counter--;
    }

    /* timer underflow? */
    t->t_out = (0 == t->counter) && _M6522_PIP_TEST(t->pip, M6522_PIP_TIMER_COUNT, 1);
    if (t->t_out) {
        /* continuous or oneshot mode? */
        if (M6522_ACR_T1_CONTINUOUS(c)) {
            /* continuous */
            t->t_bit = !t->t_bit;
            /* trigger T1 interrupt on each underflow */
            _m6522_set_intr(c, M6522_IRQ_T1);
            /* reload T1 from latch */
            _M6522_PIP_SET(t->pip, M6522_PIP_TIMER_LOAD, 0);
        }
        else {
            /* oneshot */
            if (!t->t_bit) {
                /* trigger T1 only once */
                _m6522_set_intr(c, M6522_IRQ_T1);
                t->t_bit = true;
            }
        }
    }

    /* reload timer from latch? */
    if (_M6522_PIP_TEST(t->pip, M6522_PIP_TIMER_LOAD, 0)) {
        t->counter = t->latch;
        _M6522_PIP_CLR(t->pip, M6522_PIP_TIMER_COUNT, 1);
    }
}


void _m6522_tick_t2(m6522_t* c, uint64_t pins) {
    m6522_timer_t* t = &c->t2;

    /* either decrement on PB6, or on tick */
    if (M6522_ACR_T2_COUNT_PB6(c)) {
        /* count falling edge of PB6 */
        if (M6522_PB6 & (~pins & (pins ^ c->pins))) {
            t->counter--;
        }
    }
    else if (_M6522_PIP_TEST(t->pip, M6522_PIP_TIMER_COUNT, 0)) {
        t->counter--;
    }

    /* timer underflow? note that T2 simply keeps counting, it will not reload
        from its latch on underflow, loading from latch only happens once when the timer
        is set up with a new value
    */
    t->t_out = (0 == t->counter) && _M6522_PIP_TEST(t->pip, M6522_PIP_TIMER_COUNT, 1);
    if (t->t_out) {
        /* t2 is always oneshot */
        if (!t->t_bit) {
            /* FIXME: 6526-style "Timer B Bug"? */
            _m6522_set_intr(c, M6522_IRQ_T2);
            t->t_bit = true;
        }
    }

    /* reload timer from latch? this only happens when T2 is
        explicitly loaded, not on wrap-around
     */
    if (_M6522_PIP_TEST(t->pip, M6522_PIP_TIMER_LOAD, 0)) {
        t->counter = t->latch;
        _M6522_PIP_CLR(t->pip, M6522_PIP_TIMER_COUNT, 1);
    }
}

void _m6522_tick_pipeline(m6522_t* c) {
    /* feed counter pipelines, both counters are always counting */
    _M6522_PIP_SET(c->t1.pip, M6522_PIP_TIMER_COUNT, 2);
    _M6522_PIP_SET(c->t2.pip, M6522_PIP_TIMER_COUNT, 2);

    /* interrupt pipeline */
    if (c->intr.ifr & c->intr.ier) {
        _M6522_PIP_SET(c->intr.pip, M6522_PIP_IRQ, 1);
    }

    /* tick pipelines forward */
    c->t1.pip = (c->t1.pip >> 1) & 0x7F7F;
    c->t2.pip = (c->t2.pip >> 1) & 0x7F7F;
    c->intr.pip = (c->intr.pip >> 1) & 0x7F7F;
}

uint64_t _m6522_update_irq(m6522_t* c, uint64_t pins) {
    /* FIXME interrupts for CA1, CA2, CB1, CB2, SR */

    /* main interrupt bit (delayed by pip) */
    if (_M6522_PIP_TEST(c->intr.pip, M6522_PIP_IRQ, 0)) {
        c->intr.ifr |= (1<<7);
    }

    /* merge IRQ bit */
    if (0 != (c->intr.ifr & (1<<7))) {
        pins |= M6522_IRQ;
    }
    else {
        pins &= ~M6522_IRQ;
    }
    return pins;
}

#define _M6522_TICK_PIN_MASK (M6522_IRQ|M6522_PA_PINS|M6522_PB_PINS|M6522_CA_PINS|M6522_CB_PINS)
uint64_t m6522_tick(m6522_t* c, uint64_t pins) {
    /* process input pins */
    _m6522_read_port_pins(c, pins);

    /* tick timers */
    _m6522_tick_t1(c, pins);
    _m6522_tick_t2(c, pins);

    /* update interrupt bits */
    pins = _m6522_update_irq(c, pins);

    /* merge port output pins */
    pins = _m6522_write_port_pins(c, pins);

    /* tick internal delay-pipelines forward */
    _m6522_tick_pipeline(c);

    c->pins = (c->pins & ~_M6522_TICK_PIN_MASK) | (pins & _M6522_TICK_PIN_MASK);
    return pins;
}

static uint8_t _m6522_read(m6522_t* c, uint8_t addr) {
    uint8_t data = 0;
    switch (addr) {
        case M6522_REG_RB:
            if (M6522_ACR_PB_LATCH_ENABLE(c)) {
                data = c->pb.inpr;
            }
            else {
                data = c->pb.pins;
            }
            _m6522_clear_pb_intr(c);
            break;

        case M6522_REG_RA:
            if (M6522_ACR_PA_LATCH_ENABLE(c)) {
                data = c->pa.inpr;
            }
            else {
                data = c->pa.pins;
            }
            _m6522_clear_pa_intr(c);
            /* FIXME: handshake */
            /* FIXME: pulse output */
            break;
        
        case M6522_REG_DDRB:
            data = c->pb.ddr;
            break;
        
        case M6522_REG_DDRA:
            data = c->pa.ddr;
            break;

        case M6522_REG_T1CL:
            data = c->t1.counter & 0xFF;
            _m6522_clear_intr(c, M6522_IRQ_T1);
            break;

        case M6522_REG_T1CH:
            data = c->t1.counter >> 8;
            break;

        case M6522_REG_T1LL:
            data = c->t1.latch & 0xFF;
            break;

        case M6522_REG_T1LH:
            data = c->t1.latch >> 8;
            break;

        case M6522_REG_T2CL:
            data = c->t2.counter & 0xFF;
            _m6522_clear_intr(c, M6522_IRQ_T2);
            break;

        case M6522_REG_T2CH:
            data = c->t2.counter >> 8;
            break;

        case M6522_REG_SR:
            /* FIXME */
            break;

        case M6522_REG_ACR:
            data = c->acr;
            break;

        case M6522_REG_PCR:
            data = c->pcr;
            break;

        case M6522_REG_IFR:
            data = c->intr.ifr;
            break;

        case M6522_REG_IER:
            data = c->intr.ier | 0x80;
            break;

        case M6522_REG_RA_NOH:
            if (M6522_ACR_PA_LATCH_ENABLE(c)) {
                data = c->pa.inpr;
            }
            else {
                data = c->pa.pins;
            }
            break;
    }
    return data;
}

static void _m6522_write(m6522_t* c, uint8_t addr, uint8_t data) {
    switch (addr) {
        case M6522_REG_RB:
            c->pb.outr = data;
            _m6522_clear_pb_intr(c);
            /* FIXME: handshake */
            break;
        
        case M6522_REG_RA:
            c->pa.outr = data;
            _m6522_clear_pa_intr(c);
            /* FIXME: handshake */
            /* FIXME: pulse output */
            break;
        
        case M6522_REG_DDRB:
            c->pb.ddr = data;
            break;
    
        case M6522_REG_DDRA:
            c->pa.ddr = data;
            break;

        case M6522_REG_T1CL:
        case M6522_REG_T1LL:
            c->t1.latch = (c->t1.latch & 0xFF00) | data;
            break;

        case M6522_REG_T1CH:
            c->t1.latch = (data << 8) | (c->t1.latch & 0x00FF);
            _m6522_clear_intr(c, M6522_IRQ_T1);
            c->t1.t_bit = false;
            _M6522_PIP_SET(c->t1.pip, M6522_PIP_TIMER_LOAD, 1);
            break;

        case M6522_REG_T1LH:
            c->t1.latch = (data << 8) | (c->t1.latch & 0x00FF);
            _m6522_clear_intr(c, M6522_IRQ_T1);
            break;

        case M6522_REG_T2CL:
            c->t2.latch = (c->t2.latch & 0xFF00) | data;
            break;

        case M6522_REG_T2CH:
            c->t2.latch = (data << 8) | (c->t2.latch & 0x00FF);
            _m6522_clear_intr(c, M6522_IRQ_T2);
            c->t2.t_bit = false;
            _M6522_PIP_SET(c->t2.pip, M6522_PIP_TIMER_LOAD, 1);
            break;

        case M6522_REG_SR:
            /* FIXME */
            break;

        case M6522_REG_ACR:
            c->acr = data;
            /* FIXME: shift timer */
            if (M6522_ACR_T1_CONTINUOUS(c)) {
                /* add counter delay */
                _M6522_PIP_CLR(c->t1.pip, M6522_PIP_TIMER_COUNT, 0);
                _M6522_PIP_CLR(c->t1.pip, M6522_PIP_TIMER_COUNT, 1);
            }
            break;

        case M6522_REG_PCR:
            c->pcr = data;
            /* FIXME: CA2, CB2 */
            break;

        case M6522_REG_IFR:
            _m6522_write_ifr(c, data);
            break;

        case M6522_REG_IER:
            _m6522_write_ier(c, data);
            break;

        case M6522_REG_RA_NOH:
            c->pa.outr = data;
            break;
    }
}

#define _M6522_IORQ_PIN_MASK (M6522_RS_PINS|M6522_DB_PINS|M6522_RW|M6522_CS1|M6522_CS2)
uint64_t m6522_iorq(m6522_t* c, uint64_t pins) {
    if ((pins & (M6522_CS1|M6522_CS2)) == M6522_CS1) {
        uint8_t addr = pins & M6522_RS_PINS;
        if (pins & M6522_RW) {
            /* a read operation */
            uint8_t data = _m6522_read(c, addr);
            M6522_SET_DATA(pins, data);
        }
        else {
            /* a write operation */
            uint8_t data = M6522_GET_DATA(pins);
            _m6522_write(c, addr, data);
        }
        c->pins = (c->pins & ~_M6522_IORQ_PIN_MASK) | (pins & _M6522_IORQ_PIN_MASK);
    }
    return pins;
}

#endif /* CHIPS_IMPL */
