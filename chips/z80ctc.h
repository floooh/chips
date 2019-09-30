#pragma once
/*#
    # z80ctc.h
    
    Header-only emulator for the Z80 CTC (Counter/Timer Channels)
    written in C.

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

    ## Emulated Pins:

    ***************************************
    *          +-----------+              *
    *   D0 <-->|           |<--- CLKTRG0  *
    *   .. <-->|           |---> ZCTO0    *
    *   D7 <-->|           |<--- CLKTRG1  *
    *   CE --->|           |---> ZCTO1    *
    *  CS0 --->|           |<--- CLKTRG2  *
    *  CS1 --->|    Z80    |---> ZCTO2    *
    *   M1 --->|    CTC    |<--- CLKTRG3  *
    * IORQ --->|           |              *
    *   RD --->|           |              *
    *  INT <---|           |              *
    *  IEI --->|           |              *
    *  IEO <---|           |              *
    *          +-----------+              *
    ***************************************

    ## Inaccuracies:

    - the spec says "After initialization, channels may be reprogrammed at
      any time. If updated control and time constant words are written to a
      channel during the count operation, the count continues to zero before
      the new time constant is loaded into the counter". The current
      implementation doesn't behave like this, instead it behaves like MAME.
      Needs more research!

    ## Functions:
    ~~~C
    void z80ctc_init(z80ctc_t* ctc)
    ~~~
        Initializes a new Z80 CTC instance and puts it into the reset state.

    ~~~C
    void z80ctc_reset(z80ctc_t* ctc)
    ~~~
        Puts the CTC instance into the reset state.

    ~~~C
    uint64_t z80ctc_iorq(z80ctc_t* ctc, uint64_t pins)
    ~~~
        Call this when the CPU wants to perform an IO request on the CTC.
        Takes a pin mask as input, and returns a potentially modified
        pin mask.

        The following input pins are read function:

        - **M1**:       must be unset
        - **IORQ|CE**:  must both be set
        - **RD**:       must be set to read data from the CTC
        - **WR**:       must be set to write data to the CTC
        - **CS0|CS1**:  selects the CTC channels 0..3
        - **D0..D7**:   the data to write to the CTC

        The following output pins are potentially modified:

        - **D0..D7**:   the data read from the CTC

    ~~~C
    uint64_t z80ctc_tick(z80ctc_t* ctc, uint64_t pins)
    ~~~
        Perform a tick on the CTC, this function must be called
        for every CPU tick. The CTC will check for external trigger
        events (via the CLKTRG pins), update counters, and set the
        ZCTO pins when counters reach zero.

        The following input pins are read:

        - **CLKTRG0..CLKTRG3**: used to inject an internal trigger/counter
          event into the CTC
        
        The following output pins are potentially modified:

        - **ZCTO0..ZCTO2**: set when the channels 0..2 are in counter
          mode and the countdown reaches 0

    ~~~C
    uint64_t z80ctc_int(z80ctc_t* ctc, uint64_t pins)
    ~~~
        Handle the daisychain interrupt protocol. See the z80.h
        header for details.

    ## Macros

    ~~~C
    Z80CTC_SET_DATA(p,d)
    ~~~
        set 8-bit data bus pins in 64-bit pin mask (identical with
        Z80_SET_DATA() from the z80.h header)

    ~~~C
    Z80CTC_GET_DATA(p)
    ~~~
        extract 8-bit data bus value from 64-bit pin mask (identical with
        Z80_GET_DATA() from the z80.h header)

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

/* control pins directly shared with CPU */
#define Z80CTC_M1       (1ULL<<24)   /* CPU Machine Cycle One (same as Z80_M1) */
#define Z80CTC_IORQ     (1ULL<<26)   /* CPU IO Request (same as Z80_IORQ) */
#define Z80CTC_RD       (1ULL<<27)   /* CPU Read Cycle Status (same as Z80_RD) */
#define Z80CTC_INT      (1ULL<<30)   /* Interrupt Request (same as Z80_INT) */
#define Z80CTC_RESET    (1ULL<<31)   /* put CTC into reset state (same as Z80_RESET) */

/* Z80 interrupt daisy chain shared pins */
#define Z80CTC_IEIO     (1ULL<<37)   /* combined Interrupt Enable In/Out (same as Z80_IEIO) */
#define Z80CTC_RETI     (1ULL<<38)   /* CPU has decoded a RETI instruction (same as Z80_RETI) */

/* CTC specific pins starting at bit 40 */
#define Z80CTC_CE       (1ULL<<40)   /* Chip Enable */
#define Z80CTC_CS0      (1ULL<<41)   /* Channel Select Bit 0 */
#define Z80CTC_CS1      (1ULL<<42)   /* Channel Select Bit 1 */
#define Z80CTC_CLKTRG0  (1ULL<<43)   /* Clock/Timer Trigger 0 */
#define Z80CTC_CLKTRG1  (1ULL<<44)   /* Clock/Timer Trigger 1 */
#define Z80CTC_CLKTRG2  (1ULL<<45)   /* Clock/Timer Trigger 2 */
#define Z80CTC_CLKTRG3  (1ULL<<46)   /* Clock/Timer Trigger 3 */
#define Z80CTC_ZCTO0    (1ULL<<47)   /* Zero Count/Timeout 0 */
#define Z80CTC_ZCTO1    (1ULL<<48)   /* Zero Count/Timeout 1 */
#define Z80CTC_ZCTO2    (1ULL<<49)   /* Zero Count/Timeout 2 */

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

/* interrupt state bits */
#define Z80CTC_INT_NEEDED           (1<<0)  /* interrupt request needed */
#define Z80CTC_INT_REQUESTED        (1<<1)  /* interrupt request issued, waiting for ack from CPU */
#define Z80CTC_INT_SERVICING        (1<<2)  /* interrupt was acknowledged, now servicing */

/*
    Z80 CTC channel state
*/
typedef struct {
    uint8_t control;        /* Z80CTC_CTRL_xxx */
    uint8_t constant;
    uint8_t down_counter;
    uint8_t prescaler;
    uint8_t int_vector;
    /* optimization helpers */
    bool trigger_edge;
    bool waiting_for_trigger;
    bool ext_trigger;
    uint8_t prescaler_mask;
    uint8_t int_state;
} z80ctc_channel_t;

#define Z80CTC_NUM_CHANNELS (4)

/*
    Z80 CTC state 
*/
typedef struct {
    z80ctc_channel_t chn[Z80CTC_NUM_CHANNELS];
    uint64_t pins;
} z80ctc_t;

/* extract 8-bit data bus from 64-bit pins */
#define Z80CTC_GET_DATA(p) ((uint8_t)(p>>16))
/* merge 8-bit data bus value into 64-bit pins */
#define Z80CTC_SET_DATA(p,d) {p=((p&~0xFF0000)|((d&0xFF)<<16));}

/* initialize a new Z80 CTC instance */
void z80ctc_init(z80ctc_t* ctc);
/* reset an existing Z80 CTC instance */
void z80ctc_reset(z80ctc_t* ctc);
/* perform an IORQ machine cycle */
uint64_t z80ctc_iorq(z80ctc_t* ctc, uint64_t pins);

/*
    Internal inline function!

    called when the downcounter reaches zero, request interrupt,
    trigger ZCTO pin and reload downcounter
*/
static inline uint64_t _z80ctc_counter_zero(z80ctc_t* ctc, z80ctc_channel_t* chn, uint64_t pins, int chn_id) {
    /* down counter has reached zero, trigger interrupt and ZCTO pin */
    if (chn->control & Z80CTC_CTRL_EI) {
        /* interrupt enabled, request an interrupt */
        chn->int_state |= Z80CTC_INT_NEEDED;
    }
    /* last channel doesn't have a ZCTO pin */
    if (chn_id < 4) {
        /* set the zcto pin */
        pins |= Z80CTC_ZCTO0<<chn_id;
        ctc->pins = pins;
    }
    /* reload the down counter */
    chn->down_counter = chn->constant;
    return pins;
}

/* 
    Internal inline function!

    Issue an 'active edge' on a channel, this happens when a CLKTRG pin
    is triggered, or when reprogramming the Z80CTC_CTRL_EDGE control bit.

    This results in:
    - if the channel is in timer mode and waiting for trigger,
      the waiting flag is cleared and timing starts
    - if the channel is in counter mode, the counter decrements
*/
static inline uint64_t _z80ctc_active_edge(z80ctc_t* ctc, z80ctc_channel_t* chn, uint64_t pins, int chn_id) {
    if ((chn->control & Z80CTC_CTRL_MODE) == Z80CTC_CTRL_MODE_COUNTER) {
        /* counter mode */
        if (0 == --chn->down_counter) {
            pins = _z80ctc_counter_zero(ctc, chn, pins, chn_id);
        }
    }
    else if (chn->waiting_for_trigger) {
        /* timer mode and waiting for trigger? */
        chn->waiting_for_trigger = false;
        chn->down_counter = chn->constant;
    }
    return pins;
}

/* perform a tick on the CTC, handles counters and timers */
static inline uint64_t z80ctc_tick(z80ctc_t* ctc, uint64_t pins) {
    pins &= ~(Z80CTC_ZCTO0|Z80CTC_ZCTO1|Z80CTC_ZCTO2);
    for (int chn_id = 0; chn_id < Z80CTC_NUM_CHANNELS; chn_id++) {
        z80ctc_channel_t* chn = &ctc->chn[chn_id];

        /* check if externally triggered */
        if (chn->waiting_for_trigger || (chn->control & Z80CTC_CTRL_MODE) == Z80CTC_CTRL_MODE_COUNTER) {
            bool trg = 0 != (pins & (Z80CTC_CLKTRG0<<chn_id));
            if (trg != chn->ext_trigger) {
                chn->ext_trigger = trg;
                /* rising/falling edge trigger */
                if (chn->trigger_edge == trg) {
                    pins = _z80ctc_active_edge(ctc, chn, pins, chn_id);
                }
            }
        }
        else if ((chn->control & (Z80CTC_CTRL_MODE|Z80CTC_CTRL_RESET|Z80CTC_CTRL_CONST_FOLLOWS)) == Z80CTC_CTRL_MODE_TIMER) {
            /* handle timer mode downcounting */
            if (0 == ((--chn->prescaler) & chn->prescaler_mask)) {
                /* prescaler has reached zero, tick the down counter */
                if (0 == --chn->down_counter) {
                    pins = _z80ctc_counter_zero(ctc, chn, pins, chn_id);
                }
            }
        }
    }
    return pins;
}

/* call this once per machine cycle to handle the interrupt daisy chain */
static inline uint64_t z80ctc_int(z80ctc_t* ctc, uint64_t pins) {
    for (int i = 0; i < Z80CTC_NUM_CHANNELS; i++) {
        z80ctc_channel_t* chn = &ctc->chn[i];
        /*
            - set status of IEO pin depending on IEI pin and current
              channel's interrupt request/acknowledge status, this
              'ripples' to the next channel and downstream interrupt
              controllers

            - the IEO pin will be set to inactive (interrupt disabled)
              when: (1) the IEI pin is inactive, or (2) the IEI pin is
              active and and an interrupt has been requested

            - if an interrupt has been requested but not ackowledged by
              the CPU because interrupts are disabled, the RETI state
              must be passed to downstream devices. If a RETI is 
              received in the interrupt-requested state, the IEIO
              pin will be set to active, so that downstream devices
              get a chance to decode the RETI
        */

        /* if any higher priority device in the daisy chain has cleared
           the IEIO pin, skip interrupt handling
        */
        if ((pins & Z80CTC_IEIO) && (0 != chn->int_state)) {
            /* check if if the CPU has decoded a RETI */
            if (pins & Z80CTC_RETI) {
                /* if we're the device that's currently under service by
                   the CPU, keep interrupts enabled for downstream devices and
                   clear our interrupt state (this is basically the
                   'HELP' logic described in the PIO and CTC manuals
                */
                if (chn->int_state & Z80CTC_INT_SERVICING) {
                    chn->int_state = 0;
                }
                /* if we are *NOT* the device currently under service, this
                   means we have an interrupt request pending but the CPU
                   denied the request (because interruprs were disabled)
                */
            }
            /* need to request interrupt? */
            if (chn->int_state & Z80CTC_INT_NEEDED) {
                chn->int_state &= ~Z80CTC_INT_NEEDED;
                chn->int_state |= Z80CTC_INT_REQUESTED;
            }
            /* need to place interrupt vector on data bus? */
            if ((pins & (Z80CTC_IORQ|Z80CTC_M1)) == (Z80CTC_IORQ|Z80CTC_M1)) {
                /* CPU has acknowledged the interrupt, place interrupt
                   vector on data bus
                */
                Z80CTC_SET_DATA(pins, chn->int_vector);
                chn->int_state &= ~Z80CTC_INT_REQUESTED;
                chn->int_state |= Z80CTC_INT_SERVICING;
            }
            /* disable interrupts for downstream devices? */
            if (0 != chn->int_state) {
                pins &= ~Z80CTC_IEIO;
            }
            /* set Z80_INT pin state during INT_REQUESTED */
            if (chn->int_state & Z80CTC_INT_REQUESTED) {
                pins |= Z80CTC_INT;
                ctc->pins = pins;
            }
        }
    }
    return pins;
}

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

/*
    z80ctc_init

    Call this once to initialize a new Z80 CTC instance, this will
    clear the z80ctc struct and go into a reset state.
*/
void z80ctc_init(z80ctc_t* ctc) {
    CHIPS_ASSERT(ctc);
    memset(ctc, 0, sizeof(*ctc));
    z80ctc_reset(ctc);
}

/*
    z80ctc_reset

    Puts the Z80 CTC into the reset state.
*/
void z80ctc_reset(z80ctc_t* ctc) {
    CHIPS_ASSERT(ctc);
    for (int i = 0; i < Z80CTC_NUM_CHANNELS; i++) {
        z80ctc_channel_t* chn = &ctc->chn[i];
        chn->control = Z80CTC_CTRL_RESET;
        chn->constant = 0;
        chn->down_counter = 0;
        chn->waiting_for_trigger = false;
        chn->trigger_edge = false;
        chn->prescaler_mask = 0x0F;
        chn->int_state = 0;
    }
}

/* write to CTC channel */
uint64_t _z80ctc_write(z80ctc_t* ctc, uint64_t pins, int chn_id, uint8_t data) {
    z80ctc_channel_t* chn = &ctc->chn[chn_id];
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
        chn->trigger_edge = (data & Z80CTC_CTRL_EDGE) == Z80CTC_CTRL_EDGE_RISING;
        if ((chn->control & Z80CTC_CTRL_PRESCALER) == Z80CTC_CTRL_PRESCALER_16) {
            chn->prescaler_mask = 0x0F;
        }
        else {
            chn->prescaler_mask = 0xFF;
        }

        /* changing the Trigger Slope trigger an 'active edge' */
        if ((old_ctrl & Z80CTC_CTRL_EDGE) != (chn->control & Z80CTC_CTRL_EDGE)) {
            pins = _z80ctc_active_edge(ctc, chn, pins, chn_id);
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
uint8_t _z80ctc_read(z80ctc_t* ctc, int chn_id) {
    return ctc->chn[chn_id].down_counter;
}

/* perform an IORQ machine cycle */
uint64_t z80ctc_iorq(z80ctc_t* ctc, uint64_t pins) {
    if ((pins & (Z80CTC_CE|Z80CTC_IORQ|Z80CTC_M1)) == (Z80CTC_CE|Z80CTC_IORQ)) {
        const int chn_id = (pins / Z80CTC_CS0) & 3;
        if (pins & Z80CTC_RD) {
            const uint8_t data = _z80ctc_read(ctc, chn_id);
            Z80CTC_SET_DATA(pins, data);
        }
        else {
            const uint8_t data = Z80CTC_GET_DATA(pins);
            pins = _z80ctc_write(ctc, pins, chn_id, data);
        }
        ctc->pins = pins;
    }
    else {
        ctc->pins = (ctc->pins & ~(Z80CTC_CE|Z80CTC_M1|Z80CTC_IORQ|Z80CTC_RD)) | (pins & (Z80CTC_CE|Z80CTC_M1|Z80CTC_IORQ|Z80CTC_RD));
    }
    return pins;
}

#endif /* CHIPS_IMPL */
