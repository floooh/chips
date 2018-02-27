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

/* control register bits */
#define M6526_CRA_START                 (1<<0)
#define M6526_CRA_START_START           (1<<0)  /* start timer A (reset on underflow in oneshot mode) */
#define M6526_CRA_START_STOP            (0)     /* stop timer A */

#define M6526_CRA_PBON                  (1<<1)
#define M6526_CRA_PBON_PB6ON            (1<<1)  /* timer A output appears on PB6 */
#define M6526_CRA_PBON_PB6OFF           (0)     /* PB6 normal operation */

#define M6526_CRA_OUTMODE               (1<<2)
#define M6526_CRA_OUTMODE_TOGGLE        (1<<2)
#define M6526_CRA_OUTMODE_PULSE         (0)

#define M6526_CRA_RUNMODE               (1<<3)
#define M6526_CRA_RUNMODE_ONESHOT       (1<<3)
#define M6526_CRA_RUNMODE_CONTINUOUS    (0)

#define M6526_CRA_FORCE_LOAD            (1<<4)  /* strobe only, no latch, read will always return 0 */

#define M6526_CRA_INMODE                (1<<5)
#define M6526_CRA_INMODE_CNT            (1<<5)  /* timer A counts positive CNT pin transitions */
#define M6526_CRA_INMODE_CLK            (0)     /* timer A counts clock ticks */

#define M6526_CRA_SPMODE                (1<<6)
#define M6526_CRA_SPMODE_OUTPUT         (1<<6)
#define M6526_CRA_SPMODE_INPUT          (0)

#define M6526_CRA_TODIN                 (1<<7)
#define M6526_CRA_TODIN_50HZ            (1<<7)
#define M6526_CRA_TODIN_60HZ            (0)

#define M6526_CRB_START                 (1<<0)
#define M6526_CRB_START_START           (1<<0)  /* start timer B (reset on underflow in oneshot mode) */
#define M6526_CRB_START_STOP            (0)     /* stop timer B */

#define M6526_CRB_PBON                  (1<<1)
#define M6526_CRB_PBON_PB7ON            (1<<1)  /* timer B output appears on PB7 */
#define M6526_CRB_PBON_PB7OFF           (0)     /* PB7 normal operation */

#define M6526_CRB_OUTMODE               (1<<2)
#define M6526_CRB_OUTMODE_TOGGLE        (1<<2)
#define M6526_CRB_OUTMODE_PULSE         (0)

#define M6526_CRB_RUNMODE               (1<<3)
#define M6526_CRB_RUNMODE_ONESHOT       (1<<3)
#define M6526_CRB_RUNMODE_CONTINUOUS    (0)

#define M6526_CRB_FORCE_LOAD            (1<<4)  /* strobe only, no latch, read will always return 0 */

#define M6526_CRB_INMODE                ((1<<6)|(1<<5))
#define M6526_CRB_INMODE_CLK            (0)     /* timer B counts clock ticks */
#define M6526_CRB_INMODE_CNT            (1<<5)  /* timer B counts positive CNT pin transitions */
#define M6526_CRB_INMODE_TA             (1<<6)  /* timer B counts timer A underflow pulses */
#define M6526_CRB_INMODE_CNTTA          ((1<<6)|(1<<5)) /* timer B counts timer A underflow pulses while CNT is high */

#define M6526_CRB_ALARM                 (1<<7)
#define M6526_CRB_ALARM_TOD             (1<<7)  /* writing to TOD registers sets alarm */
#define M6526_CRB_ALARM_ALARM           (0)     /* writing to TOD registers sets TOD clock */

/* port in/out callbacks */
#define M6526_PORT_A (0)
#define M6526_PORT_B (1)
typedef uint8_t (*m6526_in_t)(int port_id);
typedef void (*m6526_out_t)(int port_id, uint8_t data);

/* m6526 state */
typedef struct {
    uint8_t out_a, in_a, ddr_a;
    uint8_t out_b, in_b, ddr_b;
    uint16_t ta_latch, tb_latch;
    uint16_t ta_counter, tb_counter;
    uint8_t icr, cra, crb;
    bool ta_active, tb_active;
    bool ta_pb6, tb_pb7;
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

void m6526_init(m6526_t* c, m6526_in_t in_cb, m6526_out_t out_cb) {
    CHIPS_ASSERT(c);
    memset(c, 0, sizeof(*c));
    c->in_cb = in_cb;
    c->out_cb = out_cb;
    c->ta_latch = 0xFFFF;
    c->tb_latch = 0xFFFF;
}

void m6526_reset(m6526_t* c) {
    CHIPS_ASSERT(c);
    c->out_a = c->in_a = c->ddr_a = 0;
    c->out_b = c->in_b = c->ddr_b = 0;
    c->ta_latch = c->tb_latch = 0xFFFF;
    c->ta_counter = c->tb_counter = 0;
    c->icr = c->cra = c->crb = 0;
    c->ta_active = c->tb_active = false;
    c->ta_pb6 = c->tb_pb7 = false;
}

static uint8_t _m6526_read(m6526_t* c, uint8_t addr) {

    return 0xFF;
}

static void _m6526_write(m6526_t* c, uint8_t addr, uint8_t data) {

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


#endif /* CHIPS_IMPL */

#ifdef __cplusplus
} /* extern "C" */
#endif
