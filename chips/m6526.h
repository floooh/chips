#pragma once
/*#
    # m6526.h

    MOS Technology 6526 Complex Interface Adapter (CIA)

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
    
    ## Emulated Pins

    ************************************
    *           +-----------+          *
    *    CS --->|           |<--- FLAG *
    *    RW --->|           |---> PC   *
    *   RES --->|           |---> SP   *
    *   IRQ <---|           |<--- TOD  *
    *           |           |<--- CNT  *
    *           |           |          *
    *   RS0 --->|   M6526   |<--> PA0  *
    *   RS1 --->|           |...       *
    *   RS2 --->|           |<--> PA7  *
    *   RS3 --->|           |          *
    *           |           |<--> PB0  *
    *   DB0 --->|           |...       *
    *        ...|           |<--> PB7  *
    *   DB7 --->|           |          *
    *           +-----------+          *
    ************************************

    ## NOT IMPLEMENTED:

    - PC pin
    - time of day clock
    - serial port
    - no external counter trigger via CNT pin

    ## LINKS:
    - https://ist.uwaterloo.ca/~schepers/MJK/cia6526.html
    - https://ist.uwaterloo.ca/~schepers/MJK/cia6526.html

    TODO: Documentation
    
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
#define M6526_RS0   (1ULL<<0)
#define M6526_RS1   (1ULL<<1)
#define M6526_RS2   (1ULL<<2)
#define M6526_RS3   (1ULL<<3)
#define M6526_RS    (M6526_RS3|M6526_RS2|M6526_RS1|M6526_RS0)

/* data bus pins shared with CPU */
#define M6526_D0    (1ULL<<16)
#define M6526_D1    (1ULL<<17)
#define M6526_D2    (1ULL<<18)
#define M6526_D3    (1ULL<<19)
#define M6526_D4    (1ULL<<20)
#define M6526_D5    (1ULL<<21)
#define M6526_D6    (1ULL<<22)
#define M6526_D7    (1ULL<<23)

/* control pins shared with CPU */
#define M6526_RW    (1ULL<<24)      /* same as M6502_RW */
#define M6526_IRQ   (1ULL<<26)      /* same as M6502_IRQ */

/* chip-specific control pins */
#define M6526_CS    (1ULL<<40)
#define M6526_FLAG  (1ULL<<41)
#define M6526_PC    (1ULL<<42)
#define M6526_SP    (1ULL<<43)
#define M6526_TOD   (1ULL<<44)
#define M6526_CNT   (1ULL<<45)

/* port I/O pins */
#define M6526_PA0   (1ULL<<48)
#define M6526_PA1   (1ULL<<49)
#define M6526_PA2   (1ULL<<50)
#define M6526_PA3   (1ULL<<51)
#define M6526_PA4   (1ULL<<52)
#define M6526_PA5   (1ULL<<53)
#define M6526_PA6   (1ULL<<54)
#define M6526_PA7   (1ULL<<55)

#define M6526_PB0   (1ULL<<56)
#define M6526_PB1   (1ULL<<57)
#define M6526_PB2   (1ULL<<58)
#define M6526_PB3   (1ULL<<59)
#define M6526_PB4   (1ULL<<60)
#define M6526_PB5   (1ULL<<61)
#define M6526_PB6   (1ULL<<62)
#define M6526_PB7   (1ULL<<63)

/* register indices */
#define M6526_REG_PRA       (0)     /* peripheral data reg A */
#define M6526_REG_PRB       (1)     /* peripheral data reg B */
#define M6526_REG_DDRA      (2)     /* data direction reg A */
#define M6526_REG_DDRB      (3)     /* data direction reg B */
#define M6526_REG_TALO      (4)     /* timer A low register */
#define M6526_REG_TAHI      (5)     /* timer A high register */
#define M6526_REG_TBLO      (6)     /* timer B low register */
#define M6526_REG_TBHI      (7)     /* timer B high register */
#define M6526_REG_TOD10TH   (8)     /* 10ths of seconds register */
#define M6526_REG_TODSEC    (9)     /* seconds register */
#define M6526_REG_TODMIN    (10)    /* minutes register */
#define M6526_REG_TODHR     (11)    /* hours am/pm register */
#define M6526_REG_SDR       (12)    /* serial data register */
#define M6526_REG_ICR       (13)    /* interrupt control register */
#define M6526_REG_CRA       (14)    /* control register A */
#define M6526_REG_CRB       (15)    /* control register B */

/* port in/out callbacks */
#define M6526_PORT_A (0)
#define M6526_PORT_B (1)
typedef uint8_t (*m6526_in_t)(int port_id, void* user_data);
typedef void (*m6526_out_t)(int port_id, uint8_t data, void* user_data);

/*--- control-register flag testing macros ---*/
#define M6526_TIMER_STARTED(cr)     (0!=((cr)&(1<<0)))
#define M6526_PBON(cr)              (0!=((cr)&(1<<1)))
#define M6526_OUTMODE_TOGGLE(cr)    (0!=((cr)&(1<<2)))
#define M6526_RUNMODE_ONESHOT(cr)   (0!=((cr)&(1<<3)))
#define M6526_FORCE_LOAD(cr)        (0!=((cr)&(1<<4)))
#define M6526_TA_INMODE_PHI2(cra)   (0==((cra)&(1<<5)))
#define M6526_TA_SPMODE_OUTPUT(cra) (0!=((cra)&(1<<6)))
#define M6526_TA_TODIN_50HZ(cra)    (0!=((cra)&(1<<7)))
#define M6526_TB_INMODE_PHI2(crb)   (0==((crb)&((1<<6)|(1<<5))))
#define M6526_TB_INMODE_CNT(crb)    ((1<<5)==((crb)&((1<<6)|(1<<5))))
#define M6526_TB_INMODE_TA(crb)     ((1<<6)==((crb)&((1<<6)|(1<<5))))
#define M6526_TB_INMODE_TACNT(crb)  (((1<<6)|(1<<5))==((crb)&((1<<6)|(1<<5))))
#define M6526_TB_ALARM_ALARM(crb)   (0!=((crb)&(1<<7)))

/* m6526 initialization parameters */
typedef struct {
    m6526_in_t in_cb;
    m6526_out_t out_cb;
    void* user_data;
} m6526_desc_t;

/* I/O port state */
typedef struct {
    uint8_t reg;        /* port register */
    uint8_t ddr;        /* data direction register */
    uint8_t inp;        /* input latch */
    uint8_t out;        /* last output value */
    uint8_t port;       /* current port pin state */
} m6526_port_t;

/* timer state */
typedef struct {
    uint16_t latch;     /* 16-bit initial value latch */
    uint16_t counter;   /* 16-bit counter */
    uint8_t cr;         /* control register */
    bool t_bit;         /* toggles between true and false when counter underflows */
    bool t_out;         /* true for 1 cycle when counter underflow */
    uint8_t pip_count;      /* 2-cycle delay pipeline, output is 'counter active' */
    uint8_t pip_oneshot;    /* 1-cycle delay pipeline, output is 'oneshot mode active' */
    uint8_t pip_load;       /* 1-cycle delay pipeline, output is 'force load' */
} m6526_timer_t;

/* interrupt state */
typedef struct {
    uint8_t imr;        /* interrupt mask */
    uint8_t imr1;       /* one cycle delay for imr updates */
    uint8_t icr;        /* interrupt control register */
    uint8_t pip_irq;    /* 1-cycle delay pipeline to request irq */
    bool flag;          /* last state of flag bit, to detect edge */
} m6526_int_t;

/* m6526 state */
typedef struct {
    m6526_port_t pa;
    m6526_port_t pb;
    m6526_timer_t ta;
    m6526_timer_t tb;
    m6526_int_t intr;
    uint64_t pins;
    m6526_in_t in_cb;
    m6526_out_t out_cb;
    void* user_data;
} m6526_t;

/* extract 8-bit data bus from 64-bit pins */
#define M6526_GET_DATA(p) ((uint8_t)(p>>16))
/* merge 8-bit data bus value into 64-bit pins */
#define M6526_SET_DATA(p,d) {p=((p&~0xFF0000)|((d&0xFF)<<16));}
/* merge 4-bit register-select address into 64-bit pins */
#define M6526_SET_ADDR(p,d) {p=((p&~0xF)|(d&0xF));}
/* set port A/B pins in bitmask */
#define M6526_SET_PAB(p,a,b) {p=(p&0x0000FFFFFFFFFFFFULL)|(((a)&0xFFULL)<<48)|(((b)&0xFFULL)<<56);}

/* initialize a new m6526_t instance */
void m6526_init(m6526_t* c, const m6526_desc_t* desc);
/* reset an existing m6526_t instance */
void m6526_reset(m6526_t* c);
/* perform an IO request */
uint64_t m6526_iorq(m6526_t* c, uint64_t pins);
/* tick the m6526_t instance, return true if interrupt requested */
uint64_t m6526_tick(m6526_t* c, uint64_t pins);

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

static void _m6526_init_port(m6526_port_t* p) {
    p->reg = 0;
    p->ddr = 0;
    p->inp = 0;
    p->out = 0xFF;
    p->port = 0;
}

static void _m6526_init_timer(m6526_timer_t* t) {
    t->latch = 0xFFFF;
    t->counter = 0;
    t->cr = 0;
    t->t_bit = 0;
    t->t_out = 0;
    t->pip_count = 0;
    t->pip_oneshot = 0;
    t->pip_load = 0;
}

static void _m6526_init_interrupt(m6526_int_t* intr) {
    intr->imr = 0;
    intr->imr1 = 0;
    intr->icr = 0;
    intr->pip_irq = 0;
    intr->flag = false;
}

void m6526_init(m6526_t* c, const m6526_desc_t* desc) {
    CHIPS_ASSERT(c && desc && desc->in_cb && desc->out_cb);
    memset(c, 0, sizeof(*c));
    c->in_cb = desc->in_cb;
    c->out_cb = desc->out_cb;
    c->user_data = desc->user_data;
    _m6526_init_port(&c->pa);
    _m6526_init_port(&c->pb);
    _m6526_init_timer(&c->ta);
    _m6526_init_timer(&c->tb);
    _m6526_init_interrupt(&c->intr);
    c->ta.latch = 0xFFFF;
    c->tb.latch = 0xFFFF;
}

void m6526_reset(m6526_t* c) {
    CHIPS_ASSERT(c);
    _m6526_init_port(&c->pa);
    _m6526_init_port(&c->pb);
    _m6526_init_timer(&c->ta);
    _m6526_init_timer(&c->tb);
    _m6526_init_interrupt(&c->intr);
    c->pins = 0;
}

/*--- delay-pipeline macros and functions ---*/
/* set a new state at pipeline pos */
#define _M6526_PIP_SET(pip,pos,state) {if(state){pip|=(1<<pos);}else{pip&=~(1<<pos);}}
/* test pipeline state, pos 0 is the 'output bit' */
#define _M6526_PIP_TEST(pip,pos) (0!=(pip&(1<<pos)))
/* step a pipeline */
#define _M6526_PIP_STEP(pip) (pip>>=1)

/*--- port implementation ---*/
static inline uint8_t _m6526_merge_pb67(m6526_t* c, uint8_t data) {
    /* merge timer state bits into data byte */
    if (M6526_PBON(c->ta.cr)) {
        data &= ~(1<<6);
        if (M6526_OUTMODE_TOGGLE(c->ta.cr)) {
            /* timer A toggle state into bit 6 */
            if (c->ta.t_bit) {
                data |= (1<<6);
            }
        }
        else {
            /* timer A output state into bit 6 */
            if (c->ta.t_out) {
                data |= (1<<6);
            }
        }
    }
    if (M6526_PBON(c->tb.cr)) {
        data &= ~(1<<7);
        if (M6526_OUTMODE_TOGGLE(c->tb.cr)) {
            /* timer B toggle state into bit 7 */
            if (c->tb.t_bit) {
                data |= (1<<7);
            }
        }
        else {
            /* timer B output state into bit 7 */
            if (c->tb.t_out) {
                data |= (1<<7);
            }
        }
    }
    return data;
}

static inline void _m6526_update_pa(m6526_t* c) {
    /* FIXME: is this correct? (using reg without ddr mask) */
    uint8_t data = c->pa.reg;
    data |= c->pa.inp & ~c->pa.ddr;
    if (data != c->pa.out) {
        c->pa.out = data;
        c->out_cb(M6526_PORT_A, data, c->user_data);
    }
    c->pa.port = (c->pa.out & c->pa.ddr) | (c->pa.inp & ~c->pa.ddr);
}

static inline void _m6526_update_pb(m6526_t* c) {
    /* FIXME: is this correct? (using reg without ddr mask) */
    uint8_t data = c->pb.reg;
    data |= c->pb.inp & ~c->pb.ddr;
    data = _m6526_merge_pb67(c, data);
    if (data != c->pb.out) {
        c->pb.out = data;
        c->out_cb(M6526_PORT_B, data, c->user_data);
    }
    c->pb.port = (c->pb.out & c->pb.ddr) | (c->pb.inp & ~c->pb.ddr);
}

static inline uint8_t _m6526_read_pa(m6526_t* c) {
    /* datasheet: "On a READ, the PR reflects the information present
       on the actual port pins (PA0-PA7, PB0-PB7) for both input and output bits.
    */
    /* the input callback should put a 1 into all unconnected pins */
    if (c->pa.ddr != 0xFF) {
        c->pa.inp = (c->in_cb(M6526_PORT_A, c->user_data) & ~c->pa.ddr) | (c->pa.reg & c->pa.ddr);
    }
    else {
        c->pa.inp = c->in_cb(M6526_PORT_A, c->user_data) & c->pa.reg;
    }
    c->pa.port = (c->pa.out & c->pa.ddr) | (c->pa.inp & ~c->pa.ddr);
    return c->pa.inp;
}

static uint8_t _m6526_read_pb(m6526_t* c) {
    uint8_t data;
    if (c->pb.ddr != 0xFF) {
        data = (c->in_cb(M6526_PORT_B, c->user_data) & ~c->pb.ddr) | (c->pb.reg & c->pb.ddr);
    }
    else {
        data = c->in_cb(M6526_PORT_B, c->user_data) & c->pb.reg;
    }
    c->pb.inp = data;
    data = _m6526_merge_pb67(c, data);
    c->pb.port = (c->pb.out & c->pb.ddr) | (c->pb.inp & ~c->pb.ddr);
    return data;
}

/*--- interrupt implementation ---*/
static void _m6526_write_icr(m6526_t* c, uint8_t data) {
    /* from datasheet: When writing to the MASK register, if bit 7 (SET/CLEAR)
       of data written is a ZERO, any mask bit written with a on will be cleared,
       while those mask bits written with a zero will be unaffected. If bit 7
       of the data written is a ONE, any mask bit written with a one will be set,
       while those mask bits written with a zero will be unaffected.
    */
    if (data & (1<<7)) {
        c->intr.imr1 |= (data & 0x1F);
    }
    else {
        c->intr.imr1 &= ~(data & 0x1F);
    }
}

static uint8_t _m6526_read_icr(m6526_t* c) {
    /* the icr register is cleared after reading, this will also cause the
       IRQ line to go inactive, also the irq 1-cycle-delay pipeline is
       set to cleared state.
       see Figure 5 https://ist.uwaterloo.ca/~schepers/MJK/cia6526.html
    */
    uint8_t data = c->intr.icr;
    c->intr.icr = 0;
    /* cancel an interrupt pending in the pipeline */
    c->intr.pip_irq = 0;
    return data;
}

static void _m6526_update_irq(m6526_t* c, uint64_t pins) {
    /* see Figure 5 https://ist.uwaterloo.ca/~schepers/MJK/cia6526.html */

    /* timer A underflow interrupt? */
    if (c->ta.t_out) {
        c->intr.icr |= (1<<0);
    }
    /* timer B underflow interrupt flag? */
    if (c->tb.t_out) {
        c->intr.icr |= (1<<1);
    }
    /* check for FLAG pin trigger */
    if ((pins & M6526_FLAG) && (!c->intr.flag)) {
        c->intr.icr |= (1<<4);
    }
    c->intr.flag = 0 != (pins & M6526_FLAG);

    /* FIXME: ALARM, SP interrupt conditions */

    /* handle main interrupt bit */
    if (_M6526_PIP_TEST(c->intr.pip_irq, 0)) {
        c->intr.icr |= (1<<7);
    }
}

/*--- timer implementation ---*/

/* generic timer tick, doesn't handle counter pipeline input
   (since this is different for timer A and B)
   check here for the details: https://ist.uwaterloo.ca/~schepers/MJK/cia6526.html
*/
static void _m6526_tick_timer(m6526_timer_t* t) {
    /* decrement counter? */
    if (_M6526_PIP_TEST(t->pip_count,0)) {
        t->counter--;
    }

    /* timer undeflow? */
    t->t_out = (0 == t->counter) && _M6526_PIP_TEST(t->pip_count,1);
    if (t->t_out) {
        t->t_bit = !t->t_bit;
        /* reset started flag if in one-shot mode */
        if (M6526_RUNMODE_ONESHOT(t->cr) || _M6526_PIP_TEST(t->pip_oneshot,0)) {
            t->cr &= ~(1<<0);
        }
        _M6526_PIP_SET(t->pip_load, 0, true);
    }

    /* reload counter from latch? */
    if (_M6526_PIP_TEST(t->pip_load, 0)) {
        t->counter = t->latch;
        _M6526_PIP_SET(t->pip_count,1,false);
    }
}

static void _m6526_tick_pipeline(m6526_t* c) {
    /* timer A counter pipeline (FIXME: CNT) */
    if (M6526_TA_INMODE_PHI2(c->ta.cr)) {
        _M6526_PIP_SET(c->ta.pip_count, 2, true);
    }
    if (!M6526_TIMER_STARTED(c->ta.cr)) {
        _M6526_PIP_SET(c->ta.pip_count, 2, false);
    }
    _M6526_PIP_STEP(c->ta.pip_count);
    /* timer A load-from-latch pipeline */
    if (M6526_FORCE_LOAD(c->ta.cr)) {
        _M6526_PIP_SET(c->ta.pip_load, 1, true);
        c->ta.cr &= ~(1<<4);
    }
    _M6526_PIP_STEP(c->ta.pip_load);
    /* timer A oneshot pipeline */
    if (M6526_RUNMODE_ONESHOT(c->ta.cr)) {
        _M6526_PIP_SET(c->ta.pip_oneshot, 1, true);
    }    
    _M6526_PIP_STEP(c->ta.pip_oneshot);

    /* timer B counter pipeline (FIMXE: CNT) */
    bool tb_active = false;
    if (M6526_TB_INMODE_PHI2(c->tb.cr)) {
        tb_active = true;
    }
    else if (M6526_TB_INMODE_TA(c->tb.cr)) {
        tb_active = c->ta.t_out;
    }
    else if (M6526_TB_INMODE_CNT(c->tb.cr)) {
        tb_active = false;   // FIXME: CNT always high for now
    }
    else if (M6526_TB_INMODE_TACNT(c->tb.cr)) {
        tb_active = c->ta.t_out; // FIXME: CNT always high for now
    }
    _M6526_PIP_SET(c->tb.pip_count, 2, tb_active);
    if (!M6526_TIMER_STARTED(c->tb.cr)) {
        _M6526_PIP_SET(c->tb.pip_count, 2, false);
    }
    _M6526_PIP_STEP(c->tb.pip_count);

    /* timer B load-from-latch pipeline */
    if (M6526_FORCE_LOAD(c->tb.cr)) {
        _M6526_PIP_SET(c->tb.pip_load, 1, true);
        c->tb.cr &= ~(1<<4);
    }
    _M6526_PIP_STEP(c->tb.pip_load);

    /* timer B oneshot pipeline */
    if (M6526_RUNMODE_ONESHOT(c->tb.cr)) {
        _M6526_PIP_SET(c->tb.pip_oneshot, 1, true);
    }    
    _M6526_PIP_STEP(c->tb.pip_oneshot);

    /* interrupt pipeline */
    if (c->intr.icr & c->intr.imr) {
        _M6526_PIP_SET(c->intr.pip_irq, 1, true);
    }
    _M6526_PIP_STEP(c->intr.pip_irq);
    c->intr.imr = c->intr.imr1;
}

uint64_t m6526_tick(m6526_t* c, uint64_t pins) {
    _m6526_tick_timer(&c->ta);
    _m6526_tick_timer(&c->tb);
    _m6526_update_pb(c);    /* state of PB6/PB7 might have changed */
    _m6526_update_irq(c, pins);
    _m6526_tick_pipeline(c);
    if (0 != (c->intr.icr & (1<<7))) {
        pins |= M6526_IRQ;
        c->pins |= M6526_IRQ;
    }
    else {
        pins &= ~M6526_IRQ;
        c->pins &= ~M6526_IRQ;
    }
    return pins;
}

static void _m6526_write_cr(m6526_t* c, m6526_timer_t* t, uint8_t data) {

    /* if the start bit goes from 0 to 1, set the current toggle-bit-state to 1 */
    if (!M6526_TIMER_STARTED(t->cr) && M6526_TIMER_STARTED(data)) {
        t->t_bit = true;
    }
    t->cr = data;

    /* state of port B output might be changed because of PB6/PB7 */
    _m6526_update_pb(c);
}

static void _m6526_write(m6526_t* c, uint8_t addr, uint8_t data) {
    switch (addr) {
        case M6526_REG_PRA:
            c->pa.reg = data;
            _m6526_update_pa(c);
            break;
        case M6526_REG_PRB:
            c->pb.reg = data;
            _m6526_update_pb(c);
            break;
        case M6526_REG_DDRA:
            c->pa.ddr = data;
            _m6526_update_pa(c);
            break;
        case M6526_REG_DDRB:
            c->pb.ddr = data;
            _m6526_update_pb(c);
            break;
        case M6526_REG_TALO:
            c->ta.latch = (c->ta.latch & 0xFF00) | data;
            break;
        case M6526_REG_TAHI:
            c->ta.latch = (data<<8) | (c->ta.latch & 0x00FF);
            /* if timer is not running, writing hi-byte load counter form latch */
            if (!M6526_TIMER_STARTED(c->ta.cr)) {
                _M6526_PIP_SET(c->ta.pip_load, 1, true);
            }
            break;
        case M6526_REG_TBLO:
            c->tb.latch = (c->tb.latch & 0xFF00) | data;
            break;
        case M6526_REG_TBHI:
            c->tb.latch = (data<<8) | (c->tb.latch & 0x00FF);
            /* if timer is not running, writing hi-byte writes latch */
            if (!M6526_TIMER_STARTED(c->tb.cr)) {
                _M6526_PIP_SET(c->tb.pip_load, 1, true);
            }
            break;
        case M6526_REG_ICR:
            _m6526_write_icr(c, data);
            break;
        case M6526_REG_CRA:
            _m6526_write_cr(c, &c->ta, data);
            break;
        case M6526_REG_CRB:
            _m6526_write_cr(c, &c->tb, data);
            break;
    }
}

static uint8_t _m6526_read(m6526_t* c, uint8_t addr) {
    uint8_t data = 0xFF;
    switch (addr) {
        case M6526_REG_PRA:
            data = _m6526_read_pa(c);
            break;
        case M6526_REG_PRB:
            data = _m6526_read_pb(c);
            break;
        case M6526_REG_DDRA:
            data = c->pa.ddr;
            break;
        case M6526_REG_DDRB:
            data = c->pb.ddr;
            break;
        case M6526_REG_TALO:
            data = c->ta.counter & 0xFF;
            break;
        case M6526_REG_TAHI:
            data = c->ta.counter >> 8;
            break;
        case M6526_REG_TBLO:
            data = c->tb.counter & 0xFF;
            break;
        case M6526_REG_TBHI:
            data = c->tb.counter >> 8;
            break;
        case M6526_REG_ICR:
            data = _m6526_read_icr(c);
            break;
        case M6526_REG_CRA:
            /* force-load bit always returns zero */
            data = c->ta.cr & ~(1<<4);
            break;
        case M6526_REG_CRB:
            /* force-load bit always returns zero */
            data = c->tb.cr & ~(1<<4);
            break;
    }
    return data;
}

uint64_t m6526_iorq(m6526_t* c, uint64_t pins) {
    if (pins & M6526_CS) {
        uint8_t addr = pins & M6526_RS;
        if (pins & M6526_RW) {
            /* a read request */
            uint8_t data = _m6526_read(c, addr);
            M6526_SET_DATA(pins, data);
        }
        else {
            /* a write request */
            uint8_t data = M6526_GET_DATA(pins);
            _m6526_write(c, addr, data);
        }
        M6526_SET_PAB(pins, c->pa.port, c->pb.port);
        c->pins = pins;
    }
    return pins;
}

#endif /* CHIPS_IMPL */
