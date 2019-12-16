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

/* ACR control bits */
#define M6522_ACR_LATCH_A           (1<<0)
#define M6522_ACR_LATCH_B           (1<<1)
#define M6522_ACR_SHIFT_DISABLE     (0)
/* FIXME: shift ops bits 2,3,4 */
#define M6522_ACR_T2_COUNT          (1<<5)
#define M6522_ACR_T1_CONT_INT       (1<<6)
#define M6522_ACR_T1_PB7            (1<<7)

/* port in/out callbacks */
#define M6522_PORT_A (0)
#define M6522_PORT_B (1)
typedef uint8_t (*m6522_in_t)(int port_id, void* user_data);
typedef void (*m6522_out_t)(int port_id, uint8_t data, void* user_data);

/* I/O port state */
typedef struct {
    uint8_t inp;
    uint8_t out;
    uint8_t latch;
    uint8_t ddr;
    uint8_t port;
} m6522_port_t;

/* timer state */
typedef struct {
    uint16_t latch;     /* 16-bit initial value latch */
    uint16_t counter;   /* 16-bit counter */
    bool t_bit;         /* toggles between true and false when counter underflows */
    bool t_out;         /* true for 1 cycle when counter underflow */
    uint8_t pip_count;      /* 2-cycle delay pipeline, output is 'counter active' */
    uint8_t pip_oneshot;    /* 1-cycle delay pipeline, output is 'oneshot mode active' */
    uint8_t pip_load;       /* 1-cycle delay pipeline, output is 'force load' */
} m6522_timer_t;

/* interrupt state (same as m6522_int_t) */
typedef struct {
    uint8_t imr;            /* interrupt mask */
    uint8_t imr1;           /* one cycle delay for imr updates */
    uint8_t icr;            /* interrupt control register */
    uint8_t pip_irq;        /* 1-cycle delay pipeline to request irq */
    uint8_t pip_read_icr;   /* timer B bug: remember reads from ICR */
    uint8_t cacb_edge;      /* last state of ca/cb bits to detect edge */
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
    uint64_t pins;
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
void m6522_tick(m6522_t* m6522);

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
    p->inp = 0;
    p->out = 0;
    p->latch = 0;
    p->ddr = 0;
    p->port = 0;
}

static void _m6522_init_timer(m6522_timer_t* t) {
    t->latch = 0xFFFF;
    t->counter = 0;
    t->t_bit = 0;
    t->t_out = 0;
    t->pip_count = 0;
    t->pip_oneshot = 0;
    t->pip_load = 0;
}

static void _m6522_init_interrupt(m6522_int_t* intr) {
    intr->imr = 0;
    intr->imr1 = 0;
    intr->icr = 0;
    intr->pip_irq = 0;
    intr->pip_read_icr = 0;
    intr->cacb_edge = 0;
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
void m6522_reset(m6522_t* m6522) {
    CHIPS_ASSERT(m6522);
    _m6522_init_port(&c->pa);
    _m6522_init_port(&c->pb);
    _m6522_init_timer(&c->t1);
    _m6522_init_timer(&c->t2);
    _m6522_init_interrupt(&c->intr);
    c->acr = 0;
    c->pcr = 0;
    c->pins = 0;
}

/*--- delay-pipeline macros and functions ---*/
/* set a new state at pipeline pos */
#define _M6522_PIP_SET(pip,pos,state) {if(state){pip|=(1<<pos);}else{pip&=~(1<<pos);}}
/* test pipeline state, pos 0 is the 'output bit' */
#define _M6522_PIP_TEST(pip,pos) (0!=(pip&(1<<pos)))
/* step a pipeline */
#define _M6522_PIP_STEP(pip) (pip>>=1)

#define _M6522_ACR_T1_PB7(c)         ((c->acr & (1<<7)) != 0)
#define _M6522_ACR_T1_CONT_INT(c)    ((c->acr & (1<<6)) != 0)
#define _M6522_ACR_LATCH_B(c)        ((c->acr & (1<<1)) != 0)
#define _M6522_ACR_LATCH_A(c)        ((c->acr & (1<<0)) != 0)

/*--- port implementation ---*/
static inline uint8_t _m6522_merge_pb7(m6522_t* c, uint8_t data) {
    if (M6522_ACR_T1_PB7(c)) {
        data &= ~(1<<7);
        if (c->t1.t_out) {
            data |= (1<<7);
        }
    }
    return data;
}

static inline void _m6522_update_pa(m6522_t* c) {
    uint8_t data = (c->pa.out & c->pa.ddr) | ~c->pa.ddr;
    c->out_cb(M6522_PORT_A, data, c->user_data);
    c->pa.port = data;
}

static inline void _m6522_update_pb(m6522_t* c) {
    uint8_t data = (c->pb.out & c->pb.ddr) | ~c->pb.ddr;
    data = _m6522_merge_pb7(c, data);
    c->out_cb(M6522_PORT_B, data, c->user_data);
    c->pb.port = data;
}

static inline uint8_t _m6522_read_pa(m6522_t* c) {
    c->pa.inp = c->in_cb(M6522_PORT_A, c->user_data);
    uint8_t data = (c->pa.inp & ~c->pa.ddr) | (c->pa.out & c->pa.ddr);
    c->pa.port = data;
    return data;
}

static inline uint8_t _m6522_read_pb(m6522_t* c) {
    c->pb.inp = c->in_cb(M6522_PORT_B, c->user_data);
    uint8_t data = (c->pb.inp & ~c->pb.ddr) | (c->pb.out & c->pb.ddr);
    data = _m6522_merge_pb7(c, data);
    c->pb.port = data;
    return data;
}

void m6522_tick(m6522_t* m6522) {
    if (m6522->t1-- == 0) {
        if (_M6522_CHECK_ACR_T1_CONT_INT()) {
            /* continuous, reload counter */
            m6522->t1_pb7 = !m6522->t1_pb7;
            m6522->t1 = (m6522->t1lh<<8)|m6522->t1ll;
        }
        else {
            /* one-shot, stop counting */
            m6522->t1_pb7 = 1;
            m6522->t1_active = false;
        }
    }
    if (_M6522_CHECK_ACR_T1_PB7()) {
        _m6522_out_b(m6522);
        M6522_SET_PB(m6522->pins, m6522->port_b);
    }
    /* FIXME: interrupt */
    if (m6522->t2-- == 0) {
        /* FIXME: implement delay and interrupt */
        m6522->t2_active = false;
    }
}

static void _m6522_write(m6522_t* c, uint8_t addr, uint8_t data) {
    switch (addr) {
        case M6522_REG_RB:
            c->pb.out = data;
            _m6522_update_pb(c);
            /* FIXME: clear PB interrupt */
            /* FIXME: handshake */
            break;
        
        case M6522_REG_RA:
            c->pa.out = data;
            _m6522_update_pa(c);
            /* FIXME: clear PA interrupt */
            /* FIXME: handshake */
            break;
        
        case M6522_REG_RA_NOH:
            c->pa.out = data;
            _m6522_update_pa(c);
            break;
        
        case M6522_REG_DDRB:
            c->pb.ddr = data;
            _m6522_update_pb(c);
            break;
    
        case M6522_REG_DDRA:
            c->pa.ddr = data;
            _m6522_update_pa(c);
            break;

        case M6522_REG_T1CL:
        case M6522_REG_T1LL:
            m6522->t1ll = data;
            break;

        case M6522_REG_T1LH:
            m6522->t1lh = data;
            /* FIXME: clear interrupt */
            break;

        case M6522_REG_T1CH:
            m6522->t1lh = data;
            m6522->t1 = (m6522->t1lh<<8) | m6522->t1ll;
            m6522->t1_pb7 = 0;
            if (_M6522_CHECK_ACR_T1_PB7()) {
                _m6522_out_b(m6522);
            }
            m6522->t1_active = true;
            break;

        case M6522_REG_T2CL:
            m6522->t2ll = data;
            break;

        case M6522_REG_T2CH:
            /* FIXME: clear interrupt */
            m6522->t2lh = data;
            m6522->t2 = (m6522->t2lh<<8) | m6522->t2ll;
            m6522->t2_active = true;
            break;

        case M6522_REG_SR:
            /* FIXME */
            break;

        case M6522_REG_PCR:
            m6522->pcr = data;
            /* FIXME: CA2, CB2 */
            break;

        case M6522_REG_ACR:
            m6522->acr = data;
            _m6522_out_b(m6522);
            /* FIXME: shift timer */
            if (_M6522_CHECK_ACR_T1_CONT_INT()) {
                m6522->t1_active = true;
            }
            break;

        case M6522_REG_IER:
            /* FIXME */
            break;

        case M6522_REG_IFR:
            /* FIXME */
            break;
    }
}

static uint8_t _m6522_read(m6522_t* c, uint8_t addr) {
    uint8_t data = 0;
    switch (addr) {
        case M6522_REG_RB:
            if (_M6522_ACR_LATCH_B(c)) {
                data = c->pb.inp;
            }
            else {
                data = _m6522_read_pb(c);
            }
            /* FIXME: clear interrupt */
            break;

        case M6522_REG_RA:
            if (_M6522_ACR_LATCH_A(c)) {
                data = c->pa.inp;
            }
            else {
                data = _m6522_read_pa(c);
            }
            /* FIXME: clear interrupt */
            break;
        
        case M6522_REG_RA_NOH:
            if (_M6522_ACR_LATCH_A(c)) {
                data = c->pa.inp;
            }
            else {
                data = _m6522_read_pa(c);
            }
            break;

        case M6522_REG_DDRB:
            data = c->pb.ddr;
            break;
        
        case M6522_REG_DDRA:
            data = c->pa.ddr;
            break;

        case M6522_REG_T1CL:
            /* FIXME: clear interrupt */
            data = (uint8_t) m6522->t1;
            break;

        case M6522_REG_T1CH:
            data = (uint8_t)(m6522->t1>>8);
            break;

        case M6522_REG_T1LL:
            data = m6522->t1ll;
            break;

        case M6522_REG_T1LH:
            data = m6522->t1lh;
            break;

        case M6522_REG_T2CL:
            /* FIXME: clear interrupt */
            data = (uint8_t) m6522->t2;
            break;

        case M6522_REG_T2CH:
            data = (uint8_t)(m6522->t2>>8);
            break;

        case M6522_REG_SR:
            /* FIXME */
            break;

        case M6522_REG_PCR:
            data = m6522->pcr;
            break;

        case M6522_REG_ACR:
            data = m6522->acr;
            break;

        case M6522_REG_IER:
            /* FIXME */
            break;

        case M6522_REG_IFR:
            /* FIXME */
            break;
    }
    return data;
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
        c->pins = pins;
    }
    return pins;
}

#endif /* CHIPS_IMPL */
