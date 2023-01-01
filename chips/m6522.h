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
    *   RS0 --->|           |<--- CA1   *
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

    In each system tick, call the m6522_tick() function, this takes
    an input pin mask, and returns a (potentially modified) output
    pin mask.

    Depending on the emulated system, the I/O and control pins
    PA0..PA7, PB0..PB7, CA1, CA2, CB1 and CB2 must be set as needed
    in the input pin mask (these are often connected to the keyboard
    matrix or peripheral devices).

    If the CPU wants to read or write VIA registers, set the CS1 pin
    to 1 (keep CS2 at 0), and set the RW pin depending on whether it's
    a register read (RW=1 means read, RW=0 means write, just like
    on the M6502 CPU), and the RS0..RS3 register select pins
    (usually identical with the shared address bus pins A0..A4).

    Note that the pin positions for RS0..RS3 and RW are shared with the
    respective M6502 pins.

    On return m6522_tick() returns a modified pin mask where the following
    pins might have changed state:

    - the IRQ pin (same bit position as M6502_IRQ)
    - the port A I/O pins PA0..PA7
    - the port A control pins CA1 and CA2
    - the port B I/O pins PB0..PB7
    - the port B control pins CB1 and CB2
    - data bus pins D0..D7 if this was a register read function.

    For an example VIA ticking code, checkout the _vic20_tick() function
    in systems/vic20.h

    To reset a m6522_t instance, call m6522_reset():

    ~~~C
    m6522_reset(&sys->via);
    ~~~

    ## LINKS

    On timer behaviour when hitting zero:

    http://forum.6502.org/viewtopic.php?f=4&t=2901

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

// register select same as lower 4 shared address bus bits
#define M6522_PIN_RS0       (0)
#define M6522_PIN_RS1       (1)
#define M6522_PIN_RS2       (2)
#define M6522_PIN_RS3       (3)

// data bus pins shared with CPU
#define M6522_PIN_D0        (16)
#define M6522_PIN_D1        (17)
#define M6522_PIN_D2        (18)
#define M6522_PIN_D3        (19)
#define M6522_PIN_D4        (20)
#define M6522_PIN_D5        (21)
#define M6522_PIN_D6        (22)
#define M6522_PIN_D7        (23)

// control pins shared with CPU
#define M6522_PIN_RW        (24)      // same as M6502_RW
#define M6522_PIN_IRQ       (26)      // same as M6502_IRQ

// control pins
#define M6522_PIN_CS1       (40)      // chip-select 1, to select: CS1 high, CS2 low
#define M6522_PIN_CS2       (41)      // chip-select 2

#define M6522_PIN_CA1       (42)      // peripheral A control 1
#define M6522_PIN_CA2       (43)      // peripheral A control 2
#define M6522_PIN_CB1       (44)      // peripheral B control 1
#define M6522_PIN_CB2       (45)      // peripheral B control 2

// peripheral A port
#define M6522_PIN_PA0       (48)
#define M6522_PIN_PA1       (49)
#define M6522_PIN_PA2       (50)
#define M6522_PIN_PA3       (51)
#define M6522_PIN_PA4       (52)
#define M6522_PIN_PA5       (53)
#define M6522_PIN_PA6       (54)
#define M6522_PIN_PA7       (55)

// peripheral B port
#define M6522_PIN_PB0       (56)
#define M6522_PIN_PB1       (57)
#define M6522_PIN_PB2       (58)
#define M6522_PIN_PB3       (59)
#define M6522_PIN_PB4       (60)
#define M6522_PIN_PB5       (61)
#define M6522_PIN_PB6       (62)
#define M6522_PIN_PB7       (63)

// pin bit masks
#define M6522_RS0       (1ULL<<M6522_PIN_RS0)
#define M6522_RS1       (1ULL<<M6522_PIN_RS1)
#define M6522_RS2       (1ULL<<M6522_PIN_RS2)
#define M6522_RS3       (1ULL<<M6522_PIN_RS3)
#define M6522_RS_PINS   (0x0FULL)
#define M6522_D0        (1ULL<<M6522_PIN_D0)
#define M6522_D1        (1ULL<<M6522_PIN_D1)
#define M6522_D2        (1ULL<<M6522_PIN_D2)
#define M6522_D3        (1ULL<<M6522_PIN_D3)
#define M6522_D4        (1ULL<<M6522_PIN_D4)
#define M6522_D5        (1ULL<<M6522_PIN_D5)
#define M6522_D6        (1ULL<<M6522_PIN_D6)
#define M6522_D7        (1ULL<<M6522_PIN_D7)
#define M6522_DB_PINS   (0xFF0000ULL)
#define M6522_RW        (1ULL<<M6522_PIN_RW)
#define M6522_IRQ       (1ULL<<M6522_PIN_IRQ)
#define M6522_CS1       (1ULL<<M6522_PIN_CS1)
#define M6522_CS2       (1ULL<<M6522_PIN_CS2)
#define M6522_CA1       (1ULL<<M6522_PIN_CA1)
#define M6522_CA2       (1ULL<<M6522_PIN_CA2)
#define M6522_CB1       (1ULL<<M6522_PIN_CB1)
#define M6522_CB2       (1ULL<<M6522_PIN_CB2)
#define M6522_CA_PINS   (M6522_CA1|M6522_CA2)
#define M6522_CB_PINS   (M6522_CB1|M6522_CB2)
#define M6522_PA0       (1ULL<<M6522_PIN_PA0)
#define M6522_PA1       (1ULL<<M6522_PIN_PA1)
#define M6522_PA2       (1ULL<<M6522_PIN_PA2)
#define M6522_PA3       (1ULL<<M6522_PIN_PA3)
#define M6522_PA4       (1ULL<<M6522_PIN_PA4)
#define M6522_PA5       (1ULL<<M6522_PIN_PA5)
#define M6522_PA6       (1ULL<<M6522_PIN_PA6)
#define M6522_PA7       (1ULL<<M6522_PIN_PA7)
#define M6522_PA_PINS   (M6522_PA0|M6522_PA1|M6522_PA2|M6522_PA3|M6522_PA4|M6522_PA5|M6522_PA6|M6522_PA7)
#define M6522_PB0       (1ULL<<M6522_PIN_PB0)
#define M6522_PB1       (1ULL<<M6522_PIN_PB1)
#define M6522_PB2       (1ULL<<M6522_PIN_PB2)
#define M6522_PB3       (1ULL<<M6522_PIN_PB3)
#define M6522_PB4       (1ULL<<M6522_PIN_PB4)
#define M6522_PB5       (1ULL<<M6522_PIN_PB5)
#define M6522_PB6       (1ULL<<M6522_PIN_PB6)
#define M6522_PB7       (1ULL<<M6522_PIN_PB7)
#define M6522_PB_PINS   (M6522_PB0|M6522_PB1|M6522_PB2|M6522_PB3|M6522_PB4|M6522_PB5|M6522_PB6|M6522_PB7)

// register indices
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

// PCR test macros (MAME naming)
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

// ACR test macros (MAME naming)
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

// interrupt bits
#define M6522_IRQ_CA2      (1<<0)
#define M6522_IRQ_CA1      (1<<1)
#define M6522_IRQ_SR       (1<<2)
#define M6522_IRQ_CB2      (1<<3)
#define M6522_IRQ_CB1      (1<<4)
#define M6522_IRQ_T2       (1<<5)
#define M6522_IRQ_T1       (1<<6)
#define M6522_IRQ_ANY      (1<<7)

// delay-pipeline bit offsets
#define M6522_PIP_TIMER_COUNT   (0)
#define M6522_PIP_TIMER_LOAD    (8)
#define M6522_PIP_IRQ           (0)

// I/O port state
typedef struct {
    uint8_t inpr;
    uint8_t outr;
    uint8_t ddr;
    uint8_t pins;
    bool c1_in;
    bool c1_out;
    bool c1_triggered;
    bool c2_in;
    bool c2_out;
    bool c2_triggered;
} m6522_port_t;

// timer state
typedef struct {
    uint16_t latch;     /* 16-bit initial value latch, NOTE: T2 only has an 8-bit latch */
    uint16_t counter;   /* 16-bit counter */
    bool t_bit;         /* toggles between true and false when counter underflows */
    bool t_out;         /* true for 1 cycle when counter underflow */
    /* merged delay-pipelines:
        2-cycle 'counter active':   bits 0..7
        1-cycle 'force load':       bits 8..16
    */
    uint16_t pip;
} m6522_timer_t;

// interrupt state (same as m6522_int_t)
typedef struct {
    uint8_t ier;            /* interrupt enable register */
    uint8_t ifr;            /* interrupt flag register */
    uint16_t pip;
} m6522_int_t;

// m6522 state
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

// extract 8-bit data bus from 64-bit pins
#define M6522_GET_DATA(p) ((uint8_t)((p)>>16))
// merge 8-bit data bus value into 64-bit pins
#define M6522_SET_DATA(p,d) {p=(((p)&~0xFF0000ULL)|(((d)<<16)&0xFF0000ULL));}
// extract port A pins
#define M6522_GET_PA(p) ((uint8_t)((p)>>48))
// extract port B pins
#define M6522_GET_PB(p) ((uint8_t)((p)>>56))
// merge port A pins into pin mask
#define M6522_SET_PA(p,a) {p=((p)&0xFF00FFFFFFFFFFFFULL)|(((a)&0xFFULL)<<48);}
// merge port B pins into pin mask
#define M6522_SET_PB(p,b) {p=((p)&0x00FFFFFFFFFFFFFFULL)|(((b)&0xFFULL)<<56);}
// merge port A and B pins into pin mask
#define M6522_SET_PAB(p,a,b) {p=((p)&0x0000FFFFFFFFFFFFULL)|(((a)&0xFFULL)<<48)|(((b)&0xFFULL)<<56);}

// initialize a new 6522 instance
void m6522_init(m6522_t* m6522);
// reset an existing 6522 instance
void m6522_reset(m6522_t* m6522);
// tick the m6522
uint64_t m6522_tick(m6522_t* m6522, uint64_t pins);

#ifdef __cplusplus
} // extern "C"
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
    p->c1_in = false;
    p->c1_out = true;
    p->c1_triggered = false;
    p->c2_in = false;
    p->c2_out = true;
    p->c2_triggered = false;
}

static void _m6522_init_timer(m6522_timer_t* t, bool is_reset) {
    /* counters and latches are not initialized at reset */
    if (!is_reset) {
        t->latch = 0xFFFF;
        t->counter = 0;
        t->t_bit = false;
    }
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
    _m6522_init_timer(&c->t1, false);
    _m6522_init_timer(&c->t2, false);
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
    _m6522_init_timer(&c->t1, true);
    _m6522_init_timer(&c->t2, true);
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
static inline void _m6522_read_port_pins(m6522_t* c, uint64_t pins) {
    /* check CA1/CA2/CB1/CB2 triggered */
    bool new_ca1 = 0 != (pins & M6522_CA1);
    bool new_ca2 = 0 != (pins & M6522_CA2);
    bool new_cb1 = 0 != (pins & M6522_CB1);
    bool new_cb2 = 0 != (pins & M6522_CB2);
    c->pa.c1_triggered = (c->pa.c1_in != new_ca1) && ((new_ca1 && M6522_PCR_CA1_LOW_TO_HIGH(c)) || (!new_ca1 && M6522_PCR_CA1_HIGH_TO_LOW(c)));
    c->pa.c2_triggered = (c->pa.c2_in != new_ca2) && ((new_ca2 && M6522_PCR_CA2_LOW_TO_HIGH(c)) || (!new_ca2 && M6522_PCR_CA2_HIGH_TO_LOW(c)));
    c->pb.c1_triggered = (c->pb.c1_in != new_cb1) && ((new_cb1 && M6522_PCR_CB1_LOW_TO_HIGH(c)) || (!new_ca1 && M6522_PCR_CB1_HIGH_TO_LOW(c)));
    c->pb.c2_triggered = (c->pb.c2_in != new_cb2) && ((new_cb2 && M6522_PCR_CB2_LOW_TO_HIGH(c)) || (!new_ca2 && M6522_PCR_CB2_HIGH_TO_LOW(c)));
    c->pa.c1_in = new_ca1;
    c->pa.c2_in = new_cb2;
    c->pb.c1_in = new_cb1;
    c->pb.c2_in = new_cb2;

    /* with latching enabled, only update input register when CA1 / CB1 goes active */
    if (M6522_ACR_PA_LATCH_ENABLE(c)) {
        if (c->pa.c1_triggered) {
            c->pa.inpr = M6522_GET_PA(pins);
        }
    }
    else {
        c->pa.inpr = M6522_GET_PA(pins);
    }
    if (M6522_ACR_PB_LATCH_ENABLE(c)) {
        if (c->pb.c1_triggered) {
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
    /* NOTE: CA1 actually is an input-only pin */
    pins &= ~(M6522_CA1|M6522_CA2|M6522_CB1|M6522_CB2);
    if (c->pa.c1_out) {
        pins |= M6522_CA1;
    }
    if (c->pa.c2_out) {
        pins |= M6522_CA2;
    }
    if (c->pb.c1_out) {
        pins |= M6522_CB1;
    }
    if (c->pb.c2_out) {
        pins |= M6522_CB2;
    }
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

/*
    On timer behaviour:

    http://forum.6502.org/viewtopic.php?f=4&t=2901

    (essentially: T1 is always reloaded from latch, both in continuous
    and oneshot mode, while T2 is never reloaded)
*/
static void _m6522_tick_t1(m6522_t* c) {
    m6522_timer_t* t = &c->t1;

    /* decrement counter? */
    if (_M6522_PIP_TEST(t->pip, M6522_PIP_TIMER_COUNT, 0)) {
        t->counter--;
    }

    /* timer underflow? */
    t->t_out = (0xFFFF == t->counter);
    if (t->t_out) {
        /* continuous or oneshot mode? */
        if (M6522_ACR_T1_CONTINUOUS(c)) {
            t->t_bit = !t->t_bit;
            /* trigger T1 interrupt on each underflow */
            _m6522_set_intr(c, M6522_IRQ_T1);
        }
        else {
            if (!t->t_bit) {
                /* trigger T1 only once */
                _m6522_set_intr(c, M6522_IRQ_T1);
                t->t_bit = true;
            }
        }
        /* reload T1 from latch on each underflow,
           this happens both in oneshot and continous mode
        */
        _M6522_PIP_SET(t->pip, M6522_PIP_TIMER_LOAD, 1);
    }

    /* reload timer from latch? */
    if (_M6522_PIP_TEST(t->pip, M6522_PIP_TIMER_LOAD, 0)) {
        t->counter = t->latch;
    }
}


static void _m6522_tick_t2(m6522_t* c, uint64_t pins) {
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

    /* underflow? */
    t->t_out = (0xFFFF == t->counter);
    if (t->t_out) {
        /* t2 is always oneshot */
        if (!t->t_bit) {
            /* FIXME: 6526-style "Timer B Bug"? */
            _m6522_set_intr(c, M6522_IRQ_T2);
            t->t_bit = true;
        }
        /* NOTE: T2 never reloads from latch on hitting zero */
    }
}

static void _m6522_tick_pipeline(m6522_t* c) {
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

static void _m6522_update_cab(m6522_t* c) {
    if (c->pa.c1_triggered) {
        _m6522_set_intr(c, M6522_IRQ_CA1);
        if (M6522_PCR_CA2_AUTO_HS(c)) {
            c->pa.c2_out = true;
        }
    }
    if (c->pa.c2_triggered && M6522_PCR_CA2_INPUT(c)) {
        _m6522_set_intr(c, M6522_IRQ_CA2);
    }
    if (c->pb.c1_triggered) {
        _m6522_set_intr(c, M6522_IRQ_CB1);
        if (M6522_PCR_CB2_AUTO_HS(c)) {
            c->pb.c2_out = true;
        }
    }
    // FIXME: shift-in/out on CB1
    if (c->pb.c2_triggered && M6522_PCR_CB2_INPUT(c)) {
        _m6522_set_intr(c, M6522_IRQ_CB2);
    }
}

static uint64_t _m6522_update_irq(m6522_t* c, uint64_t pins) {

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

/* perform a tick */
static uint64_t _m6522_tick(m6522_t* c, uint64_t pins) {
    _m6522_read_port_pins(c, pins);
    _m6522_update_cab(c);
    _m6522_tick_t1(c);
    _m6522_tick_t2(c, pins);
    pins = _m6522_update_irq(c, pins);
    pins = _m6522_write_port_pins(c, pins);
    _m6522_tick_pipeline(c);
    return pins;
}

/* read a register */
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
            if (M6522_PCR_CA2_PULSE_OUTPUT(c) || M6522_PCR_CA2_AUTO_HS(c)) {
                c->pa.c2_out = false;
            }
            if (M6522_PCR_CA2_PULSE_OUTPUT(c)) {
                /* FIXME: pulse output delay pipeline */
            }
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

/* write a register */
static void _m6522_write(m6522_t* c, uint8_t addr, uint8_t data) {
    switch (addr) {
        case M6522_REG_RB:
            c->pb.outr = data;
            _m6522_clear_pb_intr(c);
            if (M6522_PCR_CB2_AUTO_HS(c)) {
                c->pb.c2_out = false;
            }
            break;

        case M6522_REG_RA:
            c->pa.outr = data;
            _m6522_clear_pa_intr(c);
            if (M6522_PCR_CA2_PULSE_OUTPUT(c) || M6522_PCR_CA2_AUTO_HS(c)) {
                c->pa.c2_out = false;
            }
            if (M6522_PCR_CA2_PULSE_OUTPUT(c)) {
                /* FIXME: pulse output delay pipeline */
            }
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
            c->t1.counter = c->t1.latch;
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
            c->t2.counter = c->t2.latch;
            break;

        case M6522_REG_SR:
            /* FIXME */
            break;

        case M6522_REG_ACR:
            c->acr = data;
            /* FIXME: shift timer */
            /*
            if (M6522_ACR_T1_CONTINUOUS(c)) {
                // FIXME: continuous counter delay?
                _M6522_PIP_CLR(c->t1.pip, M6522_PIP_TIMER_COUNT, 0);
                _M6522_PIP_CLR(c->t1.pip, M6522_PIP_TIMER_COUNT, 1);
            }
            */
            /* FIXME(?) this properly transitions T2 from counting PB6 to clock counter mode */
            if (!M6522_ACR_T2_COUNT_PB6(c)) {
                _M6522_PIP_CLR(c->t2.pip, M6522_PIP_TIMER_COUNT, 0)
            }
            break;

        case M6522_REG_PCR:
            c->pcr = data;
            if (M6522_PCR_CA2_FIX_OUTPUT(c)) {
                c->pa.c2_out = M6522_PCR_CA2_OUTPUT_LEVEL(c);
            }
            if (M6522_PCR_CB2_FIX_OUTPUT(c)) {
                c->pb.c2_out = M6522_PCR_CB2_OUTPUT_LEVEL(c);
            }
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

uint64_t m6522_tick(m6522_t* c, uint64_t pins) {
    if ((pins & (M6522_CS1|M6522_CS2)) == M6522_CS1) {
        uint8_t addr = pins & M6522_RS_PINS;
        if (pins & M6522_RW) {
            uint8_t data = _m6522_read(c, addr);
            M6522_SET_DATA(pins, data);
        }
        else {
            uint8_t data = M6522_GET_DATA(pins);
            _m6522_write(c, addr, data);
        }
    }
    /* FIXME: move tick above read/write? */
    pins = _m6522_tick(c, pins);
    c->pins = pins;
    return pins;
}

#endif /* CHIPS_IMPL */
