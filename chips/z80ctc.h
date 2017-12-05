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

    FIXME: the spec says "After initialization, channels may be reprogrammed
    at any time. If updated control and time constant words are written
    to a channel during the count operation, the count continues to
    zero before the new time constant is loaded into the counter".

    The current implementation doesn't behave like this, instead it behaves
    like MAME. Needs more research!

    FIXME: interrupts
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
    37..40      'virtual' interrupt system pins
    44..53      CTC-specific pins
*/

/* control pins directly shared with CPU */
#define Z80CTC_M1       (1ULL<<24)   /* CPU Machine Cycle One (same as Z80_M1) */
#define Z80CTC_IORQ     (1ULL<<26)   /* CPU IO Request (same as Z80_IORQ) */
#define Z80CTC_RD       (1ULL<<27)   /* CPU Read Cycle Status (same as Z80_RD) */
#define Z80CTC_INT      (1ULL<<32)   /* Interrupt Request (same as Z80_INT) */
#define Z80CTC_RESET    (1ULL<<34)   /* put CTC into reset state (same as Z80_RESET) */

/* Z80 interrupt daisy chain shared pins */
#define Z80CTC_IEI      (1ULL<<37)   /* Interrupt Enable In (same as Z80PIO_IEI) */
#define Z80CTC_IEO      (1ULL<<38)   /* Interrupt Enable Out (same as Z80PIO_IEO) */

/* CTC specific pins starting at bit 40 */
#define Z80CTC_CE       (1ULL<<44)   /* Chip Enable */
#define Z80CTC_CS0_PIN  (45)
#define Z80CTC_CS0      (1ULL<<45)   /* Channel Select Bit 0 */
#define Z80CTC_CS1      (1ULL<<46)   /* Channel Select Bit 1 */
#define Z80CTC_CLKTRG0  (1ULL<<47)   /* Clock/Timer Trigger 0 */
#define Z80CTC_CLKTRG1  (1ULL<<48)   /* Clock/Timer Trigger 1 */
#define Z80CTC_CLKTRG2  (1ULL<<49)   /* Clock/Timer Trigger 2 */
#define Z80CTC_CLKTRG3  (1ULL<<50)   /* Clock/Timer Trigger 3 */
#define Z80CTC_ZCTO0    (1ULL<<51)   /* Zero Count/Timeout 0 */
#define Z80CTC_ZCTO1    (1ULL<<52)   /* Zero Count/Timeout 1 */
#define Z80CTC_ZCTO2    (1ULL<<53)   /* Zero Count/Timeout 2 */

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
#define Z80CTC_CTRL_EDGE_RISING     (1<<4)
#define Z80CTC_CTRL_EDGE_FALLING    (0)

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
} z80ctc;

/* extract 8-bit data bus from 64-bit pins */
#define Z80CTC_DATA(p) ((uint8_t)(p>>16))
/* merge 8-bit data bus value into 64-bit pins */
#define Z80CTC_SET_DATA(p,d) {p=((p&~0xFF0000)|((d&0xFF)<<16));}

/* initialize a new Z80 CTC instance */
extern void z80ctc_init(z80ctc* ctc);
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
void z80ctc_init(z80ctc* ctc) {
    CHIPS_ASSERT(ctc);
    memset(ctc, 0, sizeof(z80ctc));
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
        chn->down_counter = 0;
        chn->waiting_for_trigger = false;
    }
}

/*
    called when the downcounter reaches zero, request interrupt,
    trigger ZCTO pin and reload downcounter
*/
uint64_t _z80ctc_counter_zero(z80ctc_channel* chn, uint64_t pins, int chn_id) {
    /* down counter has reached zero, trigger interrupt and ZCTO pin */
    if (chn->control & Z80CTC_CTRL_EI) {
        /* interrupt enabled */
        /* FIXME: request interrupt */
    }
    /* last channel doesn't have a ZCTO pin */
    if (chn_id < 4) {
        /* set the zcto pin */
        pins |= Z80CTC_ZCTO0<<chn_id;
    }
    /* reload the down counter */
    chn->down_counter = chn->constant;
    return pins;
}

/* Issue an 'active edge' on a channel, this happens when a CLKTRG pin
   is triggered, or when reprogramming the Z80CTC_CTRL_EDGE control bit.

   This results in:
   - if the channel is in timer mode and waiting for trigger,
     the waiting flag is cleared and timing starts
   - if the channel is in counter mode, the counter decrements
*/
uint64_t _z80ctc_active_edge(z80ctc* ctc, uint64_t pins, int chn_id) {
    z80ctc_channel* chn = &ctc->chn[chn_id];
    if ((chn->control & Z80CTC_CTRL_MODE) == Z80CTC_CTRL_MODE_COUNTER) {
        /* counter mode */
        if (0 == --chn->down_counter) {
            pins = _z80ctc_counter_zero(chn, pins, chn_id);
        }
    }
    else {
        /* timer mode and waiting for trigger? */
        if (chn->waiting_for_trigger) {
            chn->waiting_for_trigger = false;
            chn->down_counter = chn->constant;
        }
    }
    return pins;
}

/* tick a single channel */
uint64_t _z80ctc_tick_channel(z80ctc* ctc, uint64_t pins, int chn_id) {
    z80ctc_channel* chn = &ctc->chn[chn_id];
    /* last channel doesn't have a ZCTO pin */
    if (chn_id < 4) {
        pins &= ~(Z80CTC_ZCTO0<<chn_id);
    }
    /* check if externally triggered */
    bool trg = 0 != (pins & (Z80CTC_CLKTRG0<<chn_id));
    bool triggered = false;
    if (trg != chn->ext_trigger) {
        chn->ext_trigger = trg;
        /* rising/falling edge trigger */
        if ((((chn->control & Z80CTC_CTRL_EDGE) == Z80CTC_CTRL_EDGE_RISING) && trg) ||
            (((chn->control & Z80CTC_CTRL_EDGE) == Z80CTC_CTRL_EDGE_FALLING) && !trg))
        {
            triggered = true;
        }
    }

    /* if triggered, may need to update the counter or clear the wait_for_trigger flag */
    if (triggered) {
        pins = _z80ctc_active_edge(ctc, pins, chn_id);
    }

    /* handle timer mode downcounting */
    if ((chn->control & Z80CTC_CTRL_MODE) == Z80CTC_CTRL_MODE_TIMER) {
        if (chn->waiting_for_trigger || (chn->control & (Z80CTC_CTRL_RESET|Z80CTC_CTRL_CONST_FOLLOWS))) {
            return pins;
        }
        /* decrement the prescaler and tick the down counter every
           16 or 256 prescaler ticks
        */
        uint8_t p = --chn->prescaler;
        if ((chn->control & Z80CTC_CTRL_PRESCALER) == Z80CTC_CTRL_PRESCALER_16) {
            p &= 0x0F;
        }
        if (0 == p) {
            /* prescaler has reached zero, tick the down counter */
            if (0 == --chn->down_counter) {
                pins = _z80ctc_counter_zero(chn, pins, chn_id);
            }
        }
    }
    return pins;
}

/* write to CTC channel */
uint64_t _z80ctc_write(z80ctc* ctc, uint64_t pins, int chn_id, uint8_t data) {
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
        else {
            chn->down_counter = chn->constant;
        }
    }
    else if (data & Z80CTC_CTRL_CONTROL) {
        /* a control word */
        const uint8_t old_ctrl = chn->control;
        chn->control = data;

        /* changing the Trigger Slope trigger an 'active edge' */
        if ((old_ctrl & Z80CTC_CTRL_EDGE) != (chn->control & Z80CTC_CTRL_EDGE)) {
            pins = _z80ctc_active_edge(ctc, pins, chn_id);
        }
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
    return pins;
}

/* read from CTC channel */
uint8_t _z80ctc_read(z80ctc* ctc, int chn_id) {
    return ctc->chn[chn_id].down_counter;
}

/* tick the CTC, this must be called for each CPU tick */
uint64_t z80ctc_tick(z80ctc* ctc, uint64_t pins) {
    /* tick all channels */
    for (int i = 0; i < Z80CTC_NUM_CHANNELS; i++) {
        pins = _z80ctc_tick_channel(ctc, pins, i);
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
            pins = _z80ctc_write(ctc, pins, chn_id, data);
        }
    }
    return pins;
}
#endif /* CHIPS_IMPL */

#ifdef __cplusplus
} /* extern "C" */
#endif
