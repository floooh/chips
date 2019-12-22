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
    *   CA1 --->|           |<--- RS0   *
    *   CA2 --->|           |...        *
    *   CB1 --->|           |<--- RS3   *
    *   CB2 --->|           |           *
    *           |           |<--> PA0   *
    *   CS1 --->|           |...        *
    *   CS2 --->|           |<--> PA7   *
    *           |           |           *
    *    RW --->|   m6522   |           *
    * RESET --->|           |<--> PB0   *
    *   IRQ <---|           |...        *
    *           |           |<--> PB7   *
    *           |           |           *
    *           |           |<--> D0    *
    *           |           |...        *
    *           |           |<--> D7    *
    *           +-----------+           *
    *************************************
    
    ## NOT EMULATED

    Currently this just contains the minimal required functionality to make
    some games on the Acorn Atom work (basically just timers, and even those
    or likely not correct). 

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
#define M6522_RS0   (1ULL<<0)
#define M6522_RS1   (1ULL<<1)
#define M6522_RS2   (1ULL<<2)
#define M6522_RS3   (1ULL<<3)
#define M6522_RS    (M6522_RS3|M6522_RS2|M6522_RS1|M6522_RS0)

/* data bus pins shared with CPU */
#define M6522_D0    (1ULL<<16)
#define M6522_D1    (1ULL<<17)
#define M6522_D2    (1ULL<<18)
#define M6522_D3    (1ULL<<19)
#define M6522_D4    (1ULL<<20)
#define M6522_D5    (1ULL<<21)
#define M6522_D6    (1ULL<<22)
#define M6522_D7    (1ULL<<23)

/* control pins shared with CPU */
#define M6522_RW    (1ULL<<24)      /* same as M6502_RW */
#define M6522_IRQ   (1ULL<<26)      /* same as M6502_IRQ */

/* control pins */
#define M6522_CS1   (1ULL<<40)      /* chip-select 1, to select: CS1 high, CS2 low */
#define M6522_CS2   (1ULL<<41)      /* chip-select 2 */
#define M6522_CA1   (1ULL<<42)      /* peripheral A control 1 */
#define M6522_CA2   (1ULL<<43)      /* peripheral A control 2 */
#define M6522_CB1   (1ULL<<44)      /* peripheral B control 1 */
#define M6522_CB2   (1ULL<<45)      /* peripheral B control 2 */

/* peripheral A port */
#define M6522_PA0   (1ULL<<48)
#define M6522_PA1   (1ULL<<49)
#define M6522_PA2   (1ULL<<50)
#define M6522_PA3   (1ULL<<51)
#define M6522_PA4   (1ULL<<52)
#define M6522_PA5   (1ULL<<53)
#define M6522_PA6   (1ULL<<54)
#define M6522_PA7   (1ULL<<55)

/* peripheral B port */
#define M6522_PB0   (1ULL<<56)
#define M6522_PB1   (1ULL<<57)
#define M6522_PB2   (1ULL<<58)
#define M6522_PB3   (1ULL<<59)
#define M6522_PB4   (1ULL<<60)
#define M6522_PB5   (1ULL<<61)
#define M6522_PB6   (1ULL<<62)
#define M6522_PB7   (1ULL<<63)

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

#define M6522_NUM_PORTS (2)

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

/* ACT test macros (MAME naming) */
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

/* port in/out callbacks */
#define M6522_PORT_A (0)
#define M6522_PORT_B (1)
typedef uint8_t (*m6522_in_t)(int port_id, void* user_data);
typedef void (*m6522_out_t)(int port_id, uint8_t data, void* user_data);

/* I/O port state */
typedef struct {
    uint8_t inpr;
    uint8_t outr;
    uint8_t ddr;
    uint8_t port;
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

/* m6522 initialization parameters */
typedef struct {
    m6522_in_t in_cb;
    m6522_out_t out_cb;
    void* user_data;
} m6522_desc_t;

/* m6522 state */
typedef struct {
    m6522_port_t pa;
    m6522_port_t pb;
    m6522_timer_t t1;
    m6522_timer_t t2;
    m6522_int_t intr;
    uint8_t acr;        /* auxilary control register */
    uint8_t pcr;        /* peripheral control register */
    uint64_t tick_pins; /* pin state after last m6522_tick() */
    uint64_t iorq_pins; /* pin state after last m6522_iorq() */
    m6522_in_t in_cb;
    m6522_out_t out_cb;
    void* user_data;
} m6522_t;

/* extract 8-bit data bus from 64-bit pins */
#define M6522_GET_DATA(p) ((uint8_t)(p>>16))
/* merge 8-bit data bus value into 64-bit pins */
#define M6522_SET_DATA(p,d) {p=((p&~0xFF0000)|(((d)&0xFF)<<16));}
/* merge 4-bit address into 64-bit pins */
#define M6522_SET_ADDR(p,d) {p=((p&~0xF)|((d)&0xF));}
/* merge port A pins into pin mask */
#define M6522_SET_PA(p,a) {p=(p&0xFF00FFFFFFFFFFFFULL)|(((a)&0xFFULL)<<48);}
/* merge port B pins into pin mask */
#define M6522_SET_PB(p,b) {p=(p&0x00FFFFFFFFFFFFFFULL)|(((b)&0xFFULL)<<56);}
/* merge port A and B pins into pin mask */
#define M6522_SET_PAB(p,a,b) {p=(p&0x0000FFFFFFFFFFFFULL)|(((a)&0xFFULL)<<48)|(((b)&0xFFULL)<<56);}

/* initialize a new 6522 instance */
void m6522_init(m6522_t* m6522, const m6522_desc_t* desc);
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
    p->port = 0;
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

void m6522_init(m6522_t* c, const m6522_desc_t* desc) {
    CHIPS_ASSERT(c && desc && desc->in_cb && desc->out_cb);
    memset(c, 0, sizeof(*c));
    c->in_cb = desc->in_cb;
    c->out_cb = desc->out_cb;
    c->user_data = desc->user_data;
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
    c->tick_pins = 0;
    c->iorq_pins = 0;
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
static inline uint8_t _m6522_merge_pb7(m6522_t* c, uint8_t data) {
    if (M6522_ACR_T1_SET_PB7(c)) {
        data &= ~(1<<7);
        if (c->t1.t_bit) {
            data |= (1<<7);
        }
    }
    return data;
}

static inline void _m6522_output_pa(m6522_t* c) {
    uint8_t data = (c->pa.outr & c->pa.ddr) | ~c->pa.ddr;
    c->out_cb(M6522_PORT_A, data, c->user_data);
    c->pa.port = data;
}

static inline void _m6522_output_pb(m6522_t* c) {
    uint8_t data = (c->pb.outr & c->pb.ddr) | ~c->pb.ddr;
    data = _m6522_merge_pb7(c, data);
    c->out_cb(M6522_PORT_B, data, c->user_data);
    c->pb.port = data;
}

static inline uint8_t _m6522_input_pa(m6522_t* c) {
    c->pa.inpr = c->in_cb(M6522_PORT_A, c->user_data);
    uint8_t data = (c->pa.inpr & ~c->pa.ddr) | (c->pa.outr & c->pa.ddr);
    c->pa.port = data;
    return data;
}

static inline uint8_t _m6522_input_pb(m6522_t* c) {
    c->pb.inpr = c->in_cb(M6522_PORT_B, c->user_data);
    uint8_t data = (c->pb.inpr & ~c->pb.ddr) | (c->pb.outr & c->pb.ddr);
    data = _m6522_merge_pb7(c, data);
    c->pb.port = data;
    return data;
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

/* timer tick functions, also see m6526.h */
void _m6522_tick_t1(m6522_t* c, uint64_t pins) {
    m6522_timer_t* t = &c->t1;

    /* decrement counter? */
    if (_M6522_PIP_TEST(t->pip, M6522_PIP_TIMER_COUNT, 0)) {
        t->counter--;
    }

    /* timer underflow? */
    bool old_t_out = t->t_out;
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
    if (old_t_out != t->t_out) {
        _m6522_output_pb(c);
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
        if (M6522_PB6 & (~pins & (pins ^ c->tick_pins))) {
            t->counter--;
        }
    }
    else if (_M6522_PIP_TEST(t->pip, M6522_PIP_TIMER_COUNT, 0)) {
        t->counter--;
    }

    /* timer underflow? note that T2 simply keeps on counting, it will not reload
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

void _m6522_update_irq(m6522_t* c, uint64_t pins) {
    /* FIXME interrupts for CA1, CA2, CB1, CB2, SR */

    /* main interrupt bit (delayed by pip) */
    if (_M6522_PIP_TEST(c->intr.pip, M6522_PIP_IRQ, 0)) {
        c->intr.ifr |= (1<<7);
    }
}

uint64_t m6522_tick(m6522_t* c, uint64_t pins) {
    _m6522_tick_t1(c, pins);
    _m6522_tick_t2(c, pins);
    _m6522_update_irq(c, pins);
    _m6522_tick_pipeline(c);
    if (0 != (c->intr.ifr & (1<<7))) {
        pins |= M6522_IRQ;
        c->iorq_pins |= M6522_IRQ;
    }
    else {
        pins &= ~M6522_IRQ;
        c->iorq_pins &= ~M6522_IRQ;
    }
    c->tick_pins = pins;
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
                data = _m6522_input_pb(c);
            }
            _m6522_clear_pb_intr(c);
            break;

        case M6522_REG_RA:
            if (M6522_ACR_PA_LATCH_ENABLE(c)) {
                data = c->pa.inpr;
            }
            else {
                data = _m6522_input_pa(c);
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
                data = _m6522_input_pa(c);
            }
            break;
    }
    return data;
}

static void _m6522_write(m6522_t* c, uint8_t addr, uint8_t data) {
    switch (addr) {
        case M6522_REG_RB:
            c->pb.outr = data;
            _m6522_output_pb(c);
            _m6522_clear_pb_intr(c);
            /* FIXME: handshake */
            break;
        
        case M6522_REG_RA:
            c->pa.outr = data;
            _m6522_output_pa(c);
            _m6522_clear_pa_intr(c);
            /* FIXME: handshake */
            /* FIXME: pulse output */
            break;
        
        case M6522_REG_DDRB:
            c->pb.ddr = data;
            _m6522_output_pb(c);
            break;
    
        case M6522_REG_DDRA:
            c->pa.ddr = data;
            _m6522_output_pa(c);
            break;

        case M6522_REG_T1CL:
        case M6522_REG_T1LL:
            c->t1.latch = (c->t1.latch & 0xFF00) | data;
            break;

        case M6522_REG_T1CH:
            c->t1.latch = (data << 8) | (c->t1.latch & 0x00FF);
            _m6522_clear_intr(c, M6522_IRQ_T1);
            c->t1.t_bit = false;
            if (M6522_ACR_T1_SET_PB7(c)) {
                _m6522_output_pb(c);
            }
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
            _m6522_output_pb(c);
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
            _m6522_output_pa(c);
            break;
    }
}

uint64_t m6522_iorq(m6522_t* c, uint64_t pins) {
    if ((pins & (M6522_CS1|M6522_CS2)) == M6522_CS1) {
        uint8_t addr = pins & M6522_RS;
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
        M6522_SET_PAB(pins, c->pa.port, c->pb.port);
        c->iorq_pins = pins;
    }
    return pins;
}

#endif /* CHIPS_IMPL */
