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

    - handshake (FLAG and PC pin)
    - time of day clock
    - serial port
    - no external counter trigger via CNT pin
    - there are various "delay-pipelines" in the chip for counters and
      interrupts, these are currently not implemented!

    ## LINKS:
    - https://ist.uwaterloo.ca/~schepers/MJK/cia6526.html

    TODO: Documentation
    
    ## MIT License

    Copyright (c) 2018 Andre Weissflog

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
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
#define M6522_IRQ   (1ULL<<26)      /* same as M6502_IRQ */

/* chip-specific control pins */
#define M6526_CS    (1ULL<<40)
#define M6526_FLAG  (1ULL<<41)
#define M6526_PC    (1ULL<<42)
#define M6526_SP    (1ULL<<43)
#define M6526_TOD   (1ULL<<44)
#define M6526_CNT   (1ULL<<45)

/* port A in/out pins */
#define M6526_PA0   (1ULL<<48)
#define M6526_PA1   (1ULL<<49)
#define M6526_PA2   (1ULL<<50)
#define M6526_PA3   (1ULL<<51)
#define M6526_PA4   (1ULL<<52)
#define M6526_PA5   (1ULL<<53)
#define M6526_PA6   (1ULL<<54)
#define M6526_PA7   (1ULL<<55)

/* port B in/out pins */
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
typedef uint8_t (*m6526_in_t)(int port_id);
typedef void (*m6526_out_t)(int port_id, uint8_t data);

/* I/O port state */
typedef struct {
    uint8_t reg;    /* port register */
    uint8_t ddr;    /* data direction register */
    uint8_t inp;    /* input latch */
} m6526_port_t;

/* timer state */
typedef struct {
    uint16_t latch;     /* 16-bit initial value latch */
    uint16_t counter;   /* 16-bit counter */
    uint8_t cr;         /* control register */
    bool t_bit;         /* toggles between true and false when counter underflows */
    bool t_out;         /* set to true for 1 cycle when counter underflow */
    uint8_t pip_count;      /* 2-cycle delay pipeline, output is 'counter active' */
    uint8_t pip_oneshot;    /* 1-cycle delay pipeline, output is 'oneshot mode active' */
    uint8_t pip_load;       /* 1-cycle delay pipeline, output is 'force load' */
} m6526_timer_t;

/* m6526 state */
typedef struct {
    m6526_port_t pa;
    m6526_port_t pb;
    m6526_timer_t ta;
    m6526_timer_t tb;
    uint8_t icr_mask, icr_data;
    bool irq;
    m6526_in_t in_cb;
    m6526_out_t out_cb;
} m6526_t;

/* extract 8-bit data bus from 64-bit pins */
#define M6526_GET_DATA(p) ((uint8_t)(p>>16))
/* merge 8-bit data bus value into 64-bit pins */
#define M6526_SET_DATA(p,d) {p=((p&~0xFF0000)|((d&0xFF)<<16));}
/* merge 4-bit register-select address into 64-bit pins */
#define M6526_SET_ADDR(p,d) {p=((p&~0xF)|(d&0xF));}

/* initialize a new m6526_t instance */
extern void m6526_init(m6526_t* c, m6526_in_t in_cb, m6526_out_t out_cb);
/* reset an existing m6526_t instance */
extern void m6526_reset(m6526_t* c);
/* perform an IO request */
extern uint64_t m6526_iorq(m6526_t* c, uint64_t pins);
/* tick the m6526_t instance, this may trigger the IRQ pin */
extern uint64_t m6526_tick(m6526_t* c, uint64_t pins);

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

static void _m6526_init_port(m6526_port_t* p) {
    p->reg = 0;
    p->ddr = 0;
    p->inp = 0;
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

void m6526_init(m6526_t* c, m6526_in_t in_cb, m6526_out_t out_cb) {
    CHIPS_ASSERT(c && in_cb && out_cb);
    memset(c, 0, sizeof(*c));
    c->in_cb = in_cb;
    c->out_cb = out_cb;
    _m6526_init_port(&c->pa);
    _m6526_init_port(&c->pb);
    _m6526_init_timer(&c->ta);
    _m6526_init_timer(&c->tb);
    c->ta.latch = 0xFFFF;
    c->tb.latch = 0xFFFF;
}

void m6526_reset(m6526_t* c) {
    CHIPS_ASSERT(c);
    _m6526_init_port(&c->pa);
    _m6526_init_port(&c->pb);
    _m6526_init_timer(&c->ta);
    _m6526_init_timer(&c->tb);
    c->icr_mask = c->icr_data = 0;
    c->irq = false;
}

/*--- control-register utility functions */
static bool _m6526_timer_start(uint8_t cr) {
    return cr & (1<<0);
}

static bool _m6526_pbon(uint8_t cr) {
    return cr & (1<<1);
}

static bool _m6526_outmode_toggle(uint8_t cr) {
    return cr & (1<<2);
}

static bool _m6526_runmode_oneshot(uint8_t cr) {
    return cr & (1<<3);
}

static bool _m6526_ta_inmode_count(uint8_t cra) {
    return cra & (1<<5);
}

/*--- delay-pipeline functions */
static void _m6526_pip_set(uint8_t* pip, int pos, bool state) {
    /* push a new state into the pipeline at delay-position */
    if (state) {
        *pip |= (1<<pos);
    }
    else {
        *pip &= ~(1<<pos);
    }
}

static bool _m6526_pip_pop(uint8_t* pip) {
    /* pop current state from pipeline and advance the pipeline */
    bool state = *pip & 1;
    *pip >>= 1;
    return state;
}

static bool _m6526_pip_peek(uint8_t* pip, int pos) {
    /* take a peek at any pipeline slot */
    return *pip & (1<<pos);
}

/*--- timer implementation ---*/

/* generic timer tick, doesn't handle counter pipeline input
   (since this is different for timer A and B)
   check here for the details: https://ist.uwaterloo.ca/~schepers/MJK/cia6526.html
*/
static void _m6526_timer_tick(m6526_timer_t* t) {
    /* if timer is active, decrement the counter */
    if (_m6526_pip_pop(&t->pip_count)) {
        t->counter--;
    }

    /* timer output and toggle-state */
    t->t_out = (0 == t->counter) && _m6526_pip_peek(&t->pip_count, 1);
    if (t->t_out) {
        t->t_bit = !t->t_bit;
    }

    /* push one-shot state into the oneshot-pipeline */
    bool oneshot_active_now = _m6526_runmode_oneshot(t->cr);
    _m6526_pip_set(&t->pip_oneshot, 1, oneshot_active_now);

    /* clear start flag if oneshot and 0 reached */
    bool oneshot_active_pip = _m6526_pip_pop(&t->pip_oneshot);
    if (t->t_out && (oneshot_active_now || oneshot_active_pip)) {
        t->cr &= ~(1<<0);
    }

    /* reload from latch? */
    bool load_active_pip = _m6526_pip_pop(&t->pip_load);
    if (t->t_out || load_active_pip) {
        t->counter = t->latch;
        _m6526_pip_set(&t->pip_count, 2, false);
    }
}

static void _m6526_tick_ta(m6526_t* c) {
    /* push timer-active state into the count-pipeline
       FIXME: CNT pin counting is not implemented
    */
    bool timer_active_now = !_m6526_ta_inmode_count(c->ta.cr);
    timer_active_now &= _m6526_timer_start(c->ta.cr);
    _m6526_pip_set(&c->ta.pip_count, 2, timer_active_now);
    _m6526_timer_tick(&c->ta);
}

static void _m6526_tick_tb(m6526_t* c) {
    /* push timer-active state into the count-pipeline
       FIXME: CNT pin counting not implemented
       FIXME: underflow from time A not yet implemented!
    */
    bool timer_active_now = !_m6526_ta_inmode_count(c->tb.cr);
    timer_active_now &= _m6526_timer_start(c->tb.cr);
    _m6526_pip_set(&c->tb.pip_count, 2, timer_active_now);
    _m6526_timer_tick(&c->tb);
}

/*--- port implementation ---*/
static uint8_t _m6526_merge_pb67(m6526_t* c, uint8_t data) {
    /* merge timer state bits into data byte */
    if (_m6526_pbon(c->ta.cr)) {
        data &= ~(1<<6);
        if (_m6526_outmode_toggle(c->ta.cr)) {
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
    if (_m6526_pbon(c->tb.cr)) {
        data &= ~(1<<7);
        if (_m6526_outmode_toggle(c->tb.cr)) {
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

static void _m6526_write_pa(m6526_t* c, uint8_t data) {
    c->pa.reg = data;
    data |= c->pa.inp & ~c->pa.ddr;
    c->out_cb(M6526_PORT_A, data);
}

static void _m6526_write_pb(m6526_t* c, uint8_t data) {
    c->pb.reg = data;
    data |= c->pb.inp & ~c->pb.ddr;
    data = _m6526_merge_pb67(c, data);
    c->out_cb(M6526_PORT_B, data);
}

static uint8_t _m6526_read_pa(m6526_t* c) {
    /* datasheet: "On a READ, the PR reflects the information present
       on the actual port pins (PA0-PA7, PB0-PB7) for both input and output bits.
    */
    /* FIXME: MAME has a special case when ddra is 0xFF, but this is not
       mentioned anywhere?
    */
    /* the input callback should put a 1 into all unconnected pins */
    c->pa.inp = c->in_cb(M6526_PORT_A);
    return c->pa.inp;
}

static uint8_t _m6526_read_pb(m6526_t* c) {
    uint8_t data = c->in_cb(M6526_PORT_B);
    c->pb.inp = data;
    data = _m6526_merge_pb67(c, data);
    return data;
}

static void _m6526_write_cr(m6526_t* c, m6526_timer_t* t, uint8_t data) {
    /* bit 4 (force load) isn't stored, so we need to handle this here right away,
       there's a 1 cycle delay for force-load
    */
    bool force_load = data & (1<<4);
    _m6526_pip_set(&t->pip_load, 1, force_load);

    /* if the start bit goes from 0 to 1, set the current toggle-bit-state to 1 */
    if (!_m6526_timer_start(t->cr) && _m6526_timer_start(data)) {
        t->t_bit = true;
    }

    /* bit 4 (force load) isn't stored */
    t->cr = data & ~(1<<4);
    /* state of port B output might be changed because of PB6/PB7 */
    _m6526_write_pb(c, c->pb.reg);
}

static void _m6526_write(m6526_t* c, uint8_t addr, uint8_t data) {
    switch (addr) {
        case M6526_REG_PRA:
            _m6526_write_pa(c, data);
            break;
        case M6526_REG_PRB:
            _m6526_write_pb(c, data);
            break;
        case M6526_REG_DDRA:
            c->pa.ddr = data;
            _m6526_write_pa(c, c->pa.reg);
            break;
        case M6526_REG_DDRB:
            c->pb.ddr = data;
            _m6526_write_pb(c, c->pb.reg);
            break;
        case M6526_REG_TALO:
            c->ta.latch = (c->ta.latch & 0xFF00) | data;
            break;
        case M6526_REG_TAHI:
            c->ta.latch = (c->ta.latch & 0x00FF) | (data<<8);
            break;
        case M6526_REG_TBLO:
            c->tb.latch = (c->tb.latch & 0xFF00) | data;
            break;
        case M6526_REG_TBHI:
            c->tb.latch = (c->tb.latch & 0x00FF) | (data<<8);
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
        case M6526_REG_CRA:
            data = c->ta.cr;
            break;
        case M6526_REG_CRB:
            data = c->tb.cr;
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
    }
    return pins;
}

uint64_t m6526_tick(m6526_t* c, uint64_t pins) {
    _m6526_tick_ta(c);
    _m6526_tick_tb(c);
    // FIXME: interrupt!
    return pins;
}

#endif /* CHIPS_IMPL */

#ifdef __cplusplus
} /* extern "C" */
#endif
