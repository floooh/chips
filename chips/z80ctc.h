#pragma once
/*
    z80ctc.h    -- emulates the Z80 CTC (Counter/Timer Channels)

                  +-----------+
            D0 <->|           |<-- CLK/TRG0
            .. <->|           |--> ZC/TO0
            D7 <->|           |<-- CLK/TRG1
            CE -->|           |--> ZC/TO1
           CS0 -->|           |<-- CLK/TRG2
           CS1 -->|    Z80    |--> ZC/TO2
            M1 -->|    PIO    |<-- CLK/TRG3
          IORQ -->|           |
            RD -->|           |<-- RESET
           INT <--|           |
           IEI -->|           |
           IEO <--|           |
                  +-----------+
*/
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
    Pin definitions.

    All pin locations from 0 to 36 are shared with the CPU. Chip-type
    specific pins start at position 40. This enables efficient bus-sharing
    with the CPU and other Z80-family chips.

    The Z80 CTC pin layout is as follows:

    0..16       address bus A0..A15 (not connected)
    16..23      data bus D0..D7
    24..36      CPU pins (some shared directly with CTC)
    40..        CTC-specific pins
*/

/* control pins directly shared with CPU */
#define Z80CTC_M1       (1UL<<24)   /* CPU Machine Cycle One (same as Z80_M1) */
#define Z80CTC_IORQ     (1UL<<26)   /* CPU IO Request (same as Z80_IORQ) */
#define Z80CTC_RD       (1UL<<27)   /* CPU Read Cycle Status (same as Z80_RD) */
#define Z80CTC_INT      (1UL<<32)   /* Interrupt Request (same as Z80_INT) */
#define Z80CTC_RESET    (1UL<<34)   /* put CTC into reset state (same as Z80_RESET) */

/* Z80 interrupt daisy chain shared pins */
#define Z80CTC_IEI      (1UL<<37)   /* Interrupt Enable In (same as Z80PIO_IEI) */
#define Z80CTC_IEO      (1UL<<38)   /* Interrupt Enable Out (same as Z80PIO_IEO) */

/* CTC specific pins starting at bit 40 */
#define Z80CTC_CE       (1UL<<40)   /* Chip Enable */
#define Z80CTC_CS0_PIN  (41)
#define Z80CTC_CS0      (1UL<<41)   /* Channel Select Bit 0 */
#define Z80CTC_CS1      (1UL<<42)   /* Channel Select Bit 1 */
#define Z80CTC_CLKTRG0  (1UL<<43)   /* Clock/Timer Trigger 0 */
#define Z80CTC_CLKTRG1  (1UL<<44)   /* Clock/Timer Trigger 1 */
#define Z80CTC_CLKTRG2  (1UL<<45)   /* Clock/Timer Trigger 2 */
#define Z80CTC_CLKTRG3  (1UL<<46)   /* Clock/Timer Trigger 3 */
#define Z80CTC_ZCTO0    (1UL<<47)   /* Zero Count/Timeout 0 */
#define Z80CTC_ZCTO1    (1UL<<48)   /* Zero Count/Timeout 1 */
#define Z80CTC_ZCTO2    (1UL<<49)   /* Zero Count/Timeout 2 */

/*
    Z80 CTC control register bits
*/
#define Z80CTC_CTRL_EI              (1<<7)  /* 1: interrupt enabled, 0: interrupt disabled */

#define Z80CTC_CTRL_MODE            (1<<6)  /* 1: counter mode, 0: timer mode */
#define Z80CTC_CTRL_MODE_COUNTER    (1<<6)
#define Z80CTC_CTRL_MODE_TIMER      (0)

#define Z80CTC_CTRL_PRESCALER       (1<<5)  /* 1: prescale value 256, 0: prescaler value 16 */
#define Z80CTC_CTRL_PRESCALER_256   (1<<5)
#define Z80CTC_CTRL_PRESCALER_16    (0)

#define Z80CTC_CTRL_EDGE            (1<<4)  /* 1: rising edge, 0: falling edge */
#define Z80CTC_CTRL_TRIGGER         (1<<3)  /* 1: CLK/TRG pulse starts timer, 0: trigger when time constant loaded */
#define Z80CTC_CTRL_TRIGGER_WAIT    (1<<3)
#define Z80CTC_CTRL_TRIGGER_AUTO    (0)

#define Z80CTC_CTRL_CONST_FOLLOWS   (1<<2)  /* 1: time constant follows, 0: no time constant follows */
#define Z80CTC_CTRL_RESET           (1<<1)  /* 1: software reset, 0: continue operation */
#define Z80CTC_CTRL_CONTROL         (1<<0)  /* 1: control, 0: vector */
#define Z80CTC_CTRL_VECTOR          (0)

/*
    Z80 CTC channel state
*/
typedef struct {
    uint8_t control;        /* Z80CTC_CTRL_xxx */
    uint8_t constant;
    uint8_t down_counter;
    uint8_t prescaler;
    uint8_t int_vector;
    bool waiting_for_trigger;
    bool ext_trigger;
} z80ctc_channel;

#define Z80CTC_NUM_CHANNELS (4)

/*
    Z80 CTC state 
*/
typedef struct {
    z80ctc_channel chn[Z80CTC_NUM_CHANNELS];
    void (*zcto_cb)(int chn_id);
} z80ctc;

/*
    Z80 CTC initialization struct. Defines callbacks for the
    ZCTO0..3 output pins.
*/
typedef struct {
    void (*zcto_cb)(int chn_id);
} z80ctc_desc;

/* extract 8-bit data bus from 64-bit pins */
#define Z80CTC_DATA(p) ((uint8_t)(p>>16))
/* merge 8-bit data bus value into 64-bit pins */
#define Z80CTC_SET_DATA(p,d) {p=((p&~0xFF0000)|((d&0xFF)<<16));}

/* initialize a new Z80 CTC instance */
extern void z80ctc_init(z80ctc* ctc, z80ctc_desc* desc);
/* reset an existing Z80 CTC instance */
extern void z80ctc_reset(z80ctc* ctc);
/* per-tick function, call this always, not only when CTC is reprogrammed! */
extern uint64_t z80ctc_tick(z80ctc* ctc, uint64_t pins);

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

/*
    z80ctc_init

    Call this once to initialize a new Z80 CTC instance, this will
    clear the z80ctc struct and go into a reset state.
*/
void z80ctc_init(z80ctc* ctc, z80ctc_desc* desc) {
    CHIPS_ASSERT(ctc);
    memset(ctc, 0, sizeof(z80ctc));
    ctc->zcto_cb = desc->zcto_cb;
    z80ctc_reset(ctc);
}

/*
    z80ctc_reset

    Puts the Z80 CTC into the reset state.
*/
void z80ctc_reset(z80ctc* ctc) {
    CHIPS_ASSERT(ctc);
    for (int i = 0; i < Z80CTC_NUM_CHANNELS; i++) {
        z80ctc_channel* chn = &ctc->chn[i];
        chn->control = Z80CTC_CTRL_RESET;
        chn->constant = 0;
        chn->down_counter =0;
        chn->waiting_for_trigger = false;
    }
}

/* tick a single channel in timer mode */
void _z80ctc_tick_timer(z80ctc* ctc, uint64_t pins, int chn_id) {
    z80ctc_channel* chn = &ctc->chn[chn_id];

    // FIXME: check if the channel's CLKTRG has changed (against
    // RISING/FALLING egde flag:
    //  - when in timer mode and wait_for_trigger is set, clear
    //    the wait_for_trigger flag
    //  - when in counter mode, decrement the counter without
    //    prescale, when at zero, int-request and cb
    bool trg = 0 != (pins & Z80CTC_CLKTRG0<<chn_id);
    if (trg != chn->ext_trigger) {
        chn->ext_trigger = trg;
        // FIXME check rising/falling egde
        // FIXME: handle wait_for_trigger if in timer mode
    }
    // FIXME: counter vs timer mode

    /* if not in timer mode, nothing happens */
    if ((chn->control & Z80CTC_CTRL_MODE) == Z80CTC_CTRL_MODE_COUNTER) {
        return;
    }
    /* if reset active or waiting for constant, nothing happens */
    if (chn->control & (Z80CTC_CTRL_RESET|Z80CTC_CTRL_CONST_FOLLOWS)) {
        return;
    }
    /* if waiting for trigger, nothing happens */
    if (chn->waiting_for_trigger) {
        return;
    }

    /* decrement the prescaler and tick the down counter 
       every 16 or 256 prescaler ticks
    */
    uint8_t p = --chn->prescaler;
    if ((chn->control & Z80CTC_CTRL_PRESCALER) == Z80CTC_CTRL_PRESCALER_16) {
        p &= 0x0F;
    }
    if (0 == p) {
        /* prescaler has reached zero, tick the down counter */
        if (0 == --chn->down_counter) {
            /* down counter has reached zero, trigger interrupt and ZCTO pin */
            if (chn->control & Z80CTC_CTRL_EI) {
                /* interrupt enabled */
                /* FIXME: request interrupt */
            }
            if (ctc->zcto_cb) {
                ctc->zcto_cb(chn_id);
            }
            /* reload the down counter */
            chn->down_counter = chn->constant;
        }
    }
}

/* write to CTC channel */
void _z80ctc_write(z80ctc* ctc, int chn_id, uint8_t data) {
    z80ctc_channel* chn = &ctc->chn[chn_id];
    if (chn->control & Z80CTC_CTRL_CONST_FOLLOWS) {
        /* timer constant following control word */
        chn->control &= ~(Z80CTC_CTRL_CONST_FOLLOWS|Z80CTC_CTRL_RESET);
        chn->constant = data;
        if ((chn->control & Z80CTC_CTRL_MODE) == Z80CTC_CTRL_MODE_TIMER) {
            if ((chn->control & Z80CTC_CTRL_TRIGGER) == Z80CTC_CTRL_TRIGGER_WAIT) {
                chn->waiting_for_trigger = true;
            }
            else {
                chn->down_counter = chn->constant;
            }
        }
    }
    else if (data & Z80CTC_CTRL_CONTROL) {
        /* a control word */
        chn->control = data;
    }
    else {
        /* the interrupt vector for the entire CTC must be written
           to channel 0, the vectors for the following channels 
           are then computed from the base vector plus 2 bytes per channel
        */
        if (0 == chn_id) {
            for (int i = 0; i < Z80CTC_NUM_CHANNELS; i++) {
                ctc->chn[i].int_vector = (data & 0xF8) + 2*i;
            }
        }
    }
}

/* read from CTC channel */
uint8_t _z80ctc_read(z80ctc* ctc, int chn_id) {
    return ctc->chn[chn_id].down_counter;
}

/* tick the CTC, this must be called for each CPU tick */
uint64_t z80ctc_tick(z80ctc* ctc, uint64_t pins) {
    /* tick all channels */
    for (int i = 0; i < Z80CTC_NUM_CHANNELS; i++) {
        _z80ctc_tick_timer(ctc, pins, i);
    }
    /* read or write the CTC? */
    if ((pins & (Z80CTC_CE|Z80CTC_IORQ)) == (Z80CTC_CE|Z80CTC_IORQ)) {
        const int chn_id = (pins>>Z80CTC_CS0_PIN) & 3;
        if (pins & Z80CTC_RD) {
            /* read mode */
            const uint8_t data = _z80ctc_read(ctc, chn_id);
            Z80CTC_SET_DATA(pins, data);
        }
        else {
            /* write mode */
            const uint8_t data = Z80CTC_DATA(pins);
            _z80ctc_write(ctc, chn_id, data);
        }
    }
    return pins;
}
#endif /* CHIPS_IMPL */

#ifdef __cplusplus
} /* extern "C" */
#endif
