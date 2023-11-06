#pragma once
/*#
    FIXME FIXME FIXME

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
    uint64_t z80ctc_tick(z80ctc_t* ctc, uint64_t pins)
    ~~~
        Perform a tick on the CTC, this function must be called
        for every CPU tick. Depending on the input pin mask and the current
        CTC chip state, the CTC will perform IO requests from the CPU, check
        for external trigger events, perform the interrupt daisychain protocol,
        and return a potentially modified pin mask.

        The CTC reads the following pins when an IO request should be performed:

        - **Z80CTC_CE|Z80CTC_IORQ**: performs an IO request
        - **Z80CTC_RD, Z80CTC_WR**: read or write direction for IO requests
        - **Z80CTC_CS0..Z80CTC_CS1**: selects the CTC channel 0..3 for IO requests
        - **Z80CTC_D0..Z80CTC_D7**: the data byte for IO write requests

        The following input pins can be used to inject external trigger/counter
        events into the CTC:

        - **Z80CTC_CLKTRG0..Z80CTC_CLKTRG3**

        The following virtual pins are read as part of the interrupt daisychain
        protocol:

        - **Z80CTC_IEIO**: combined IEI/IEO pins, used to enable or disable interrupt
          handling in "downstream chips"
        - **Z80CTC_RETI**: virtual pin which is set when the CPU has decoded a RETI/RETN
          instruction

        The following output pins are potentially modified:

        - **Z80CTC_D0..Z80CTC_D7**: the resulting data byte of IO read requests
        - **Z80CTC_INT**: if the CTC wants to request an interrupt
        - **Z80CTC_ZCTO0..ZCTO2**: when the channels 0..2 are in counter mode and the countdown reaches 0
        - **Z80CTC_IEIO**: enable or disable interrupts for daisychain downstream chips
#*/
/*
    zlib/libpng license

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
*/
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// control pins directly shared with CPU
#define Z80CTC_BIT_M1       (24)   // CPU Machine Cycle One (same as Z80_M1) */
#define Z80CTC_BIT_IORQ     (26)   // CPU IO Request (same as Z80_IORQ) */
#define Z80CTC_BIT_RD       (27)   // CPU Read Cycle Status (same as Z80_RD) */
#define Z80CTC_BIT_INT      (30)   // Interrupt Request (same as Z80_INT) */
#define Z80CTC_BIT_RESET    (31)   // put CTC into reset state (same as Z80_RESET) */

// Z80 interrupt daisy chain shared pins
#define Z80CTC_BIT_IEIO     (37)   // combined Interrupt Enable In/Out (same as Z80_IEIO) */
#define Z80CTC_BIT_RETI     (38)   // CPU has decoded a RETI instruction (same as Z80_RETI) */

// CTC specific pins starting at bit 40
#define Z80CTC_BIT_CE       (40)   // Chip Enable
#define Z80CTC_BIT_CS0      (41)   // Channel Select Bit 0
#define Z80CTC_BIT_CS1      (42)   // Channel Select Bit 1
#define Z80CTC_BIT_CLKTRG0  (43)   // Clock/Timer Trigger 0
#define Z80CTC_BIT_CLKTRG1  (44)   // Clock/Timer Trigger 1
#define Z80CTC_BIT_CLKTRG2  (45)   // Clock/Timer Trigger 2
#define Z80CTC_BIT_CLKTRG3  (46)   // Clock/Timer Trigger 3
#define Z80CTC_BIT_ZCTO0    (47)   // Zero Count/Timeout 0
#define Z80CTC_BIT_ZCTO1    (48)   // Zero Count/Timeout 1
#define Z80CTC_BIT_ZCTO2    (49)   // Zero Count/Timeout 2

#define Z80CTC_M1       (1ULL<<Z80CTC_BIT_M1)
#define Z80CTC_IORQ     (1ULL<<Z80CTC_BIT_IORQ)
#define Z80CTC_RD       (1ULL<<Z80CTC_BIT_RD)
#define Z80CTC_INT      (1ULL<<Z80CTC_BIT_INT)
#define Z80CTC_RESET    (1ULL<<Z80CTC_BIT_RESET)
#define Z80CTC_IEIO     (1ULL<<Z80CTC_BIT_IEIO)
#define Z80CTC_RETI     (1ULL<<Z80CTC_BIT_RETI)
#define Z80CTC_CE       (1ULL<<Z80CTC_BIT_CE)
#define Z80CTC_CS0      (1ULL<<Z80CTC_BIT_CS0)
#define Z80CTC_CS1      (1ULL<<Z80CTC_BIT_CS1)
#define Z80CTC_CLKTRG0  (1ULL<<Z80CTC_BIT_CLKTRG0)
#define Z80CTC_CLKTRG1  (1ULL<<Z80CTC_BIT_CLKTRG1)
#define Z80CTC_CLKTRG2  (1ULL<<Z80CTC_BIT_CLKTRG2)
#define Z80CTC_CLKTRG3  (1ULL<<Z80CTC_BIT_CLKTRG3)
#define Z80CTC_ZCTO0    (1ULL<<Z80CTC_BIT_ZCTO0)
#define Z80CTC_ZCTO1    (1ULL<<Z80CTC_BIT_ZCTO1)
#define Z80CTC_ZCTO2    (1ULL<<Z80CTC_BIT_ZCTO2)

// Z80 CTC control register bits
#define Z80CTC_CTRL_EI              (1<<7)  // 1: interrupt enabled, 0: interrupt disabled

#define Z80CTC_CTRL_MODE            (1<<6)  // 1: counter mode, 0: timer mode
#define Z80CTC_CTRL_MODE_COUNTER    (1<<6)
#define Z80CTC_CTRL_MODE_TIMER      (0)

#define Z80CTC_CTRL_PRESCALER       (1<<5)  // 1: prescale value 256, 0: prescaler value 16
#define Z80CTC_CTRL_PRESCALER_256   (1<<5)
#define Z80CTC_CTRL_PRESCALER_16    (0)

#define Z80CTC_CTRL_EDGE            (1<<4)  // 1: rising edge, 0: falling edge
#define Z80CTC_CTRL_EDGE_RISING     (1<<4)
#define Z80CTC_CTRL_EDGE_FALLING    (0)

#define Z80CTC_CTRL_TRIGGER         (1<<3)  // 1: CLK/TRG pulse starts timer, 0: trigger when time constant loaded
#define Z80CTC_CTRL_TRIGGER_WAIT    (1<<3)
#define Z80CTC_CTRL_TRIGGER_AUTO    (0)

#define Z80CTC_CTRL_CONST_FOLLOWS   (1<<2)  // 1: time constant follows, 0: no time constant follows
#define Z80CTC_CTRL_RESET           (1<<1)  // 1: software reset, 0: continue operation
#define Z80CTC_CTRL_CONTROL         (1<<0)  // 1: control, 0: vector
#define Z80CTC_CTRL_VECTOR          (0)

// interrupt state bits
#define Z80CTC_INT_NEEDED           (1<<0)
#define Z80CTC_INT_REQUESTED        (1<<1)
#define Z80CTC_INT_SERVICED         (1<<2)

#define Z80CTC_NUM_CHANNELS (4)

typedef uint8_t u8x4 __attribute__((ext_vector_type(4)));

typedef struct {
    uint8_t control[Z80CTC_NUM_CHANNELS];   // Z80CTC_CTRL_xxx
    u8x4 prescaler;
    u8x4 prescaler_mask;
    u8x4 constant;
    u8x4 counter;
    u8x4 active_edge_counter_sub;
    uint8_t int_vector[Z80CTC_NUM_CHANNELS];
    // optimization helpers
    bool trigger_edge[Z80CTC_NUM_CHANNELS];
    bool waiting_for_trigger[Z80CTC_NUM_CHANNELS];
    bool ext_trigger[Z80CTC_NUM_CHANNELS];
    uint8_t int_state[Z80CTC_NUM_CHANNELS];
    uint64_t pins;
} z80ctc_t;

// extract 8-bit data bus from 64-bit pins
#define Z80CTC_GET_DATA(p) ((uint8_t)(((p)>>16)&0xFF))
// merge 8-bit data bus value into 64-bit pins
#define Z80CTC_SET_DATA(p,d) {p=((p)&~0xFF0000ULL)|(((d)<<16)&0xFF0000ULL);}

// initialize a new Z80 CTC instance
void z80ctc_init(z80ctc_t* ctc);
// reset the Z80 CTC instance
void z80ctc_reset(z80ctc_t* ctc);
// tick the CTC instance
uint64_t z80ctc_tick(z80ctc_t* ctc, uint64_t pins);

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
        ctc->control[i] = Z80CTC_CTRL_RESET;
        ctc->waiting_for_trigger[i] = false;
        ctc->trigger_edge[i] = false;
        ctc->int_state[i] = 0;
    }
    ctc->prescaler_mask = 0x0F;
    ctc->constant = 0;
    ctc->counter = 0;
}

/*
    Issue an 'active edge' on a channel, this happens when a CLKTRG pin
    is triggered, or when reprogramming the Z80CTC_CTRL_EDGE control bit.

    This results in:
    - if the channel is in timer mode and waiting for trigger,
      the waiting flag is cleared and timing starts
    - if the channel is in counter mode, the counter decrements
*/
static void _z80ctc_active_edge(z80ctc_t* ctc, int chn) {
    if ((ctc->control[chn] & Z80CTC_CTRL_MODE) == Z80CTC_CTRL_MODE_COUNTER) {
        // counter mode
        ctc->active_edge_counter_sub[chn] = 1;
    } else if (ctc->waiting_for_trigger[chn]) {
        // timer mode and waiting for trigger?
        ctc->waiting_for_trigger[chn] = false;
        ctc->counter[chn] = ctc->constant[chn];
    }
}

// write to CTC channel
static void _z80ctc_write(z80ctc_t* ctc, int chn, uint8_t data) {
    if (ctc->control[chn] & Z80CTC_CTRL_CONST_FOLLOWS) {
        // timer constant following control word
        ctc->control[chn] &= ~(Z80CTC_CTRL_CONST_FOLLOWS|Z80CTC_CTRL_RESET);
        ctc->constant[chn] = data;
        if ((ctc->control[chn] & Z80CTC_CTRL_MODE) == Z80CTC_CTRL_MODE_TIMER) {
            if ((ctc->control[chn] & Z80CTC_CTRL_TRIGGER) == Z80CTC_CTRL_TRIGGER_WAIT) {
                ctc->waiting_for_trigger[chn] = true;
            } else {
                ctc->counter[chn] = ctc->constant[chn];
            }
        } else {
            ctc->counter[chn] = ctc->constant[chn];
        }
    } else if (data & Z80CTC_CTRL_CONTROL) {
        // a control word
        const uint8_t old_ctrl = ctc->control[chn];
        ctc->control[chn] = data;
        ctc->trigger_edge[chn] = (data & Z80CTC_CTRL_EDGE) == Z80CTC_CTRL_EDGE_RISING;
        if ((ctc->control[chn] & Z80CTC_CTRL_PRESCALER) == Z80CTC_CTRL_PRESCALER_16) {
            ctc->prescaler_mask[chn] = 0x0F;
        } else {
            ctc->prescaler_mask[chn] = 0xFF;
        }

        // changing the Trigger Slope triggers an 'active edge'
        if ((old_ctrl & Z80CTC_CTRL_EDGE) != (ctc->control[chn] & Z80CTC_CTRL_EDGE)) {
            _z80ctc_active_edge(ctc, chn);
        }
    } else {
        /* the interrupt vector for the entire CTC must be written
           to channel 0, the vectors for the following channels
           are then computed from the base vector plus 2 bytes per channel
        */
        if (0 == chn) {
            for (int i = 0; i < Z80CTC_NUM_CHANNELS; i++) {
                ctc->int_vector[i] = (data & 0xF8) + 2*i;
            }
        }
    }
}

// perform an CPU IO request on the CTC
static uint64_t _z80ctc_iorq(z80ctc_t* ctc, uint64_t pins) {
    const int chn = (pins / Z80CTC_CS0) & 3;
    if (pins & Z80CTC_RD) {
        const uint8_t data = ctc->counter[chn];
        Z80CTC_SET_DATA(pins, data);
    } else {
        const uint8_t data = Z80CTC_GET_DATA(pins);
        _z80ctc_write(ctc, chn, data);
    }
    return pins;
}

// internal tick function
static uint64_t _z80ctc_tick(z80ctc_t* ctc, uint64_t pins) {
    pins &= ~(Z80CTC_ZCTO0|Z80CTC_ZCTO1|Z80CTC_ZCTO2);
    u8x4 prescaler_sub = 0;
    for (int chn = 0; chn < Z80CTC_NUM_CHANNELS; chn++) {
        // check if externally triggered
        if (ctc->waiting_for_trigger[chn] || (ctc->control[chn] & Z80CTC_CTRL_MODE) == Z80CTC_CTRL_MODE_COUNTER) {
            bool trg = 0 != (pins & (Z80CTC_CLKTRG0<<chn));
            if (trg != ctc->ext_trigger[chn]) {
                ctc->ext_trigger[chn] = trg;
                /* rising/falling edge trigger */
                if (ctc->trigger_edge[chn] == trg) {
                    _z80ctc_active_edge(ctc, chn);
                }
            }
        } else if ((ctc->control[chn] & (Z80CTC_CTRL_MODE|Z80CTC_CTRL_RESET|Z80CTC_CTRL_CONST_FOLLOWS)) == Z80CTC_CTRL_MODE_TIMER) {
            // handle timer mode downcounting
            prescaler_sub[chn] = 1;
        }
    }

    ctc->prescaler = (ctc->prescaler - prescaler_sub) & ctc->prescaler_mask;
    u8x4 counter_sub = ctc->active_edge_counter_sub | (((u8x4)(ctc->prescaler == 0)) & 1 & prescaler_sub);
    ctc->counter = ctc->counter - counter_sub;
    for (int chn = 0; chn < Z80CTC_NUM_CHANNELS; chn++) {
        if (counter_sub[chn]) {
            if (0 == ctc->counter[chn]) {
                // down counter has reached zero, trigger interrupt and ZCTO pin
                if (ctc->control[chn] & Z80CTC_CTRL_EI) {
                    // interrupt enabled, request an interrupt
                    ctc->int_state[chn] |= Z80CTC_INT_NEEDED;
                }
                // last channel doesn't have a ZCTO pin
                if (chn < 3) {
                    // set the zcto pin
                    pins |= Z80CTC_ZCTO0<<chn;
                }
                // reload the down counter
                ctc->counter[chn] = ctc->constant[chn];
            }
        }
    }
    return pins;
}

// interrupt daisy chain handling
static uint64_t _z80ctc_int(z80ctc_t* ctc, uint64_t pins) {
    for (int chn = 0; chn < Z80CTC_NUM_CHANNELS; chn++) {
        // on RETI, only the highest priority interrupt that's currently being
        // serviced resets its state so that IEIO enables interrupt handling
        // on downstream devices, this must be allowed to happen even if a higher
        // priority device has entered interrupt handling
        if ((pins & Z80CTC_RETI) && (ctc->int_state[chn] & Z80CTC_INT_SERVICED)) {
            ctc->int_state[chn] &= ~Z80CTC_INT_SERVICED;
            pins &= ~Z80CTC_RETI;
        }

        /*
            Also see: https://github.com/floooh/emu-info/blob/master/z80/z80-interrupts.pdf

            Especially the timing Figure 7 and Figure 7 timing diagrams!

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

            - NOT IMPLEMENTED: "All channels are inhibited from changing
              their interrupt request status when M1 is active - about two
              clock cycles earlier than IORQ".
        */
        if ((ctc->int_state[chn] != 0) && (pins & Z80CTC_IEIO)) {
            // inhibit interrupt handling on downstream devices for the
            // entire duration of interrupt servicing
            pins &= ~Z80CTC_IEIO;
            // set INT pint active until the CPU acknowledges the interrupt
            if (ctc->int_state[chn] & Z80CTC_INT_NEEDED) {
                pins |= Z80CTC_INT;
                ctc->int_state[chn] = (ctc->int_state[chn] & ~Z80CTC_INT_NEEDED) | Z80CTC_INT_REQUESTED;
            }
            // interrupt ackowledge from CPU (M1|IORQ): put interrupt vector
            // on data bus, clear INT pin and go into "serviced" state.
            if ((ctc->int_state[chn] & Z80CTC_INT_REQUESTED) && ((pins & (Z80CTC_IORQ|Z80CTC_M1)) == (Z80CTC_IORQ|Z80CTC_M1))) {
                Z80CTC_SET_DATA(pins, ctc->int_vector[chn]);
                ctc->int_state[chn] = (ctc->int_state[chn] & ~Z80CTC_INT_REQUESTED) | Z80CTC_INT_SERVICED;
                pins &= ~Z80CTC_INT;
            }
        }
    }
    return pins;
}

uint64_t z80ctc_tick(z80ctc_t* ctc, uint64_t pins) {
    ctc->active_edge_counter_sub = 0;
    if ((pins & (Z80CTC_CE|Z80CTC_IORQ|Z80CTC_M1)) == (Z80CTC_CE|Z80CTC_IORQ)) {
        pins = _z80ctc_iorq(ctc, pins);
    }
    pins = _z80ctc_tick(ctc, pins);
    pins = _z80ctc_int(ctc, pins);
    ctc->pins = pins;
    return pins;
}

#endif /* CHIPS_IMPL */
