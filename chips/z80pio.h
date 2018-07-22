#pragma once
/*#
    # z80pio.h

    Header-only emulator for the Z80 PIO (Parallel Input/Output)
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

    ## Emulated pins:

    *************************************
    *           +-----------+           *
    *    D0 <-->|           |<--> A0    *
    *    .. <-->|           |<--> ..    *
    *    D7 <-->|           |<--> A7    *
    * BASEL --->|           |---> ARDY  *
    * CDSEL --->|           |<--- ASTB  *
    *    CE --->|    Z80    |           *
    *    M1 --->|    PIO    |<--> B0    *
    *  IORQ --->|           |<--> ..    *
    *    RD --->|           |<--> B7    *
    *   INT <---|           |---> BRDY  *
    *   IEI --->|           |<--- BSTB  *
    *   IEO <---|           |           *
    *           +-----------+           *
    *************************************

    ## Not Emulated
        - bidirectional mode

    !!! Note FIXME
        function and usage documentation

    ## MIT License

    Copyright (c) 2017 Andre Weissflog

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

/*
    Pin definitions. 
    
    All pin locations from 0 to 39 are shared with the CPU. Chip-type
    specific pins start at position 44. This enables efficient bus-sharing
    with the CPU and other Z80-family chips.

    Thus the Z80 PIO pin layout is as follows:

    0..15:      address bus A0..A15 (not connected)
    16..23:     data bus D0..D7
    24..36:     CPU pins (some of those shared directly with PIO)
    37..39      'virtual' interrupt system pins
    44..50      PIO-specific pins
*/

/* control pins directly shared with CPU */
#define Z80PIO_M1       (1ULL<<24)      /* CPU Machine Cycle One, same as Z80_M1 */
#define Z80PIO_IORQ     (1ULL<<26)      /* IO Request from CPU, same as Z80_IORQ */
#define Z80PIO_RD       (1ULL<<27)      /* Read Cycle Status from CPU, same as Z80_RD */
#define Z80PIO_INT      (1ULL<<30)      /* Interrupt Request, same as Z80_INT */

/* Z80 interrupt daisy chain shared pins */
#define Z80PIO_IEIO     (1ULL<<37)      /* combined Interrupt Enable In/Out (same as Z80_IEIO) */
#define Z80PIO_RETI     (1ULL<<38)      /* CPU has decoded a RETI instruction (same as Z80_RETI) */

/* PIO specific pins start at bit 40 */
#define Z80PIO_CE       (1ULL<<44)      /* Chip Enable */
#define Z80PIO_BASEL    (1ULL<<45)      /* Port A/B Select (inactive: A, active: B) */
#define Z80PIO_CDSEL    (1ULL<<46)      /* Control/Data Select (inactive: data, active: control) */
#define Z80PIO_ARDY     (1ULL<<47)      /* Port A Ready */
#define Z80PIO_BRDY     (1ULL<<48)      /* Port B Ready */
#define Z80PIO_ASTB     (1ULL<<49)      /* Port A Strobe */
#define Z80PIO_BSTB     (1ULL<<40)      /* Port B Strobe */

/* FIXME: Port A/B 8-bit port pins? these are currently not needed because
   port in/out is handled through callback functions
*/

/*
    Port Names
*/
#define Z80PIO_PORT_A       (0)
#define Z80PIO_PORT_B       (1)
#define Z80PIO_NUM_PORTS    (2)

/*
    Operating Modes

    The operating mode of a port is established by writing a control word
    to the PIO in the following format:
 
     D7 D6 D5 D4 D3 D2 D1 D0
    |M1|M0| x| x| 1| 1| 1| 1|

    D7,D6   are the mode word bits
    D3..D0  set to 1111 to indicate 'Set Mode'
*/
#define Z80PIO_MODE_OUTPUT           (0)
#define Z80PIO_MODE_INPUT            (1)
#define Z80PIO_MODE_BIDIRECTIONAL    (2)
#define Z80PIO_MODE_BITCONTROL       (3) /* only port A */

/*
    Interrupt control word bits.

     D7 D6 D5 D4 D3 D2 D1 D0
    |EI|AO|HL|MF| 0| 1| 1| 1|

    D7 (EI)             interrupt enabled (1=enabled, 0=disabled)
    D6 (AND/OR)         logical operation during port monitoring (only Mode 3, AND=1, OR=0)
    D5 (HIGH/LOW)       port data polarity during port monitoring (only Mode 3)
    D4 (MASK FOLLOWS)   if set, the next control word are the port monitoring mask (only Mode 3)

    (*) if an interrupt is pending when the enable flag is set, it will then be 
        enabled on the onto the CPU interrupt request line
    (*) setting bit D4 during any mode of operation will cause any pending
        interrupt to be reset

    The interrupt enable flip-flop of a port may be set or reset
    without modifying the rest of the interrupt control word
    by the following command:

     D7 D6 D5 D4 D3 D2 D1 D0
    |EI| x| x| x| 0| 0| 1| 1|
*/
#define Z80PIO_INTCTRL_EI           (1<<7)
#define Z80PIO_INTCTRL_ANDOR        (1<<6)
#define Z80PIO_INTCTRL_HILO         (1<<5)
#define Z80PIO_INTCTRL_MASK_FOLLOWS (1<<4)

/* Interrupt Handling State */
#define Z80PIO_INT_NEEDED (1<<0)
#define Z80PIO_INT_REQUESTED (1<<1)
#define Z80PIO_INT_SERVICING (1<<2)

/*
    I/O port registers
*/
typedef struct {
    uint8_t input;          /* 8-bit data input register */
    uint8_t output;         /* 8-bit data output register */
    uint8_t mode;           /* 2-bit mode control register (Z80PIO_MODE_*) */
    uint8_t io_select;      /* 8-bit input/output select register */
    uint8_t int_vector;     /* 8-bit interrupt vector */
    uint8_t int_control;    /* interrupt control word (Z80PIO_INTCTRL_*) */
    uint8_t int_mask;       /* 8-bit interrupt control mask */
    uint8_t int_state;      /* current interrupt handling state */
    bool int_enabled;       /* definitive interrupt enabled flag */
    bool expect_io_select;  /* next control word will be io_select */
    bool expect_int_mask;   /* next control word will be  mask */
    bool bctrl_match;       /* bitcontrol logic equation result */
} z80pio_port_t;

/*  
    FIXME: the out callback should take the current pin mask 
    as input arg and return value to allow the callback
    to modify the CPU pins (e.g. to request an interrupr)
*/
typedef uint8_t (*z80pio_in_t)(int port_id, void* user_data);
typedef void (*z80pio_out_t)(int port_id, uint8_t data, void* user_data);

/* Z80 PIO initialization parameters */
typedef struct {
    z80pio_in_t in_cb;      /* port-input callback */
    z80pio_out_t out_cb;    /* port-output callback */
    void* user_data;        /* user-data handed to callbacks */
} z80pio_desc_t;

/* Z80 PIO state. */
typedef struct {
    /* port A and B register sets */
    z80pio_port_t port[Z80PIO_NUM_PORTS];
    /* currently in reset state? (until a control word is received) */
    bool reset_active;
    /* port-input callback */
    z80pio_in_t in_cb;
    /* port-output callback */
    z80pio_out_t out_cb;
    /* user-data for callbacks */
    void* user_data;
} z80pio_t;

/* extract 8-bit data bus from 64-bit pins */
#define Z80PIO_GET_DATA(p) ((uint8_t)(p>>16))
/* merge 8-bit data bus value into 64-bit pins */
#define Z80PIO_SET_DATA(p,d) {p=((p&~0xFF0000)|((d&0xFF)<<16));}

/*
    z80pio_init

    Call this once to initialize a new Z80 PIO instance, this will
    clear the z80pio structure and go into reset state:

    pio     -- pointer to a z80pio_t instance
    desc    -- pointer to a z80pio_desc_t structure

*/
extern void z80pio_init(z80pio_t* pio, z80pio_desc_t* desc);

/*
    z80pio_reset

    The Z80-PIO automatically enters a reset state when power is applied.
    The reset state performs the following functions:
    1) Both port mask registers are reset to inhibit all port data bits.
    2) Port data bus lines are set to a high impedance state and the
       Ready "handshake" signals are inactive (low) Mode 1 is automatically
       selected.
    3) The vector address registers are NOT reset.
    4) Both port interrupt enable flip flops are reset
    5) Both port output registers are reset.
*/
extern void z80pio_reset(z80pio_t* pio);

/*
    z80pio_iorq

    IORQ machine cycle function, before calling this function the pins argument
    must be prepared with the functions the PIO should control. Usually
    the PIO control pins Z80PIO_BASEL and Z80PIO_CDSEL are connected
    to two address bus pins, forming 4 consecutive port addresses
    for PIO data/control operations on port A or B:

    ... get shared pins from CPU (ADDR, DATA, IORQ, RD, M1, INT)
    uint64_t pio_pins = cpu_pins & Z80_PIN_MASK
    ... if the PIO needs to perform a task, set the chip-enable pin:
    pio_pins |= Z80PIO_CE
    ... select the control/data operation:
    if (control) pio_pins |= Z80PIO_CDSEL; (else: data operation)
    ... select port A or B
    if (port_b) pio_pins |= Z80PIO_BASEL; (else: port A)
    ... tick the PIO, and merge any results back into CPU pins
    cpu_pins = z80pio_tick(&pio, pio_pins) & Z80_PIN_MASK;
*/
extern uint64_t z80pio_iorq(z80pio_t* pio, uint64_t pins);

/* write value to a PIO port, this may trigger an interrupt */
void z80pio_write_port(z80pio_t* pio, int port_id, uint8_t data);
/* call this once per machine cycle to handle the interrupt daisy chain */
static inline uint64_t z80pio_int(z80pio_t* pio, uint64_t pins) {
    for (int i = 0; i < Z80PIO_NUM_PORTS; i++) {
        z80pio_port_t* p = &pio->port[i];
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
        if ((pins & Z80PIO_IEIO) && (0 != p->int_state)) {
            /* check if if the CPU has decoded a RETI */
            if (pins & Z80PIO_RETI) {
                /* if we're the device that's currently under service by
                   the CPU, keep interrupts enabled for downstream devices and
                   clear our interrupt state (this is basically the
                   'HELP' logic described in the PIO and CTC manuals
                */
                if (p->int_state & Z80PIO_INT_SERVICING) {
                    p->int_state = 0;
                }
                /* if we are *NOT* the device currently under service, this
                   means we have an interrupt request pending but the CPU
                   denied the request (because interruprs were disabled)
                */
            }
            /* need to request interrupt? */
            if (p->int_state & Z80PIO_INT_NEEDED) {
                pins |= Z80PIO_INT;
                p->int_state &= ~Z80PIO_INT_NEEDED;
                p->int_state |= Z80PIO_INT_REQUESTED;
            }
            /* need to place interrupt vector on data bus? */
            if ((pins & (Z80PIO_IORQ|Z80PIO_M1)) == (Z80PIO_IORQ|Z80PIO_M1)) {
                /* CPU has acknowledged the interrupt, place interrupt
                   vector on data bus
                */
                Z80PIO_SET_DATA(pins, p->int_vector);
                p->int_state &= ~Z80PIO_INT_REQUESTED;
                p->int_state |= Z80PIO_INT_SERVICING;
            }
            /* disable interrupts for downstream devices? */
            if (0 != p->int_state) {
                pins &= ~Z80PIO_IEIO;
            }
        }
    }
    return pins;
}

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

void z80pio_init(z80pio_t* pio, z80pio_desc_t* desc) {
    CHIPS_ASSERT(pio && desc && desc->in_cb && desc->out_cb);
    memset(pio, 0, sizeof(*pio));
    pio->out_cb = desc->out_cb;
    pio->in_cb = desc->in_cb;
    pio->user_data = desc->user_data;
    z80pio_reset(pio);
}

void z80pio_reset(z80pio_t* pio) {
    CHIPS_ASSERT(pio);
    for (int p = 0; p < Z80PIO_NUM_PORTS; p++) {
        pio->port[p].mode = Z80PIO_MODE_INPUT;
        pio->port[p].output = 0;
        pio->port[p].io_select = 0;
        pio->port[p].int_control &= ~Z80PIO_INTCTRL_EI;
        pio->port[p].int_mask = 0xFF;
        pio->port[p].int_enabled = false;
        pio->port[p].expect_int_mask = false;
        pio->port[p].expect_io_select = false;
        pio->port[p].bctrl_match = false;
        pio->port[p].int_state = 0;
    }
    pio->reset_active = true;
}

/* new control word received from CPU */
void _z80pio_write_ctrl(z80pio_t* pio, int port_id, uint8_t data) {
    CHIPS_ASSERT((port_id >= 0) && (port_id < Z80PIO_NUM_PORTS));
    pio->reset_active = false;
    z80pio_port_t* p = &pio->port[port_id];
    if (p->expect_io_select) {
        /* followup io_select mask */
        p->io_select = data;
        p->int_enabled = (p->int_control & Z80PIO_INTCTRL_EI) ? true:false;
        p->expect_io_select = false;
    } 
    else if (p->expect_int_mask) {
        /* followup interrupt mask */
        p->int_mask = data;
        p->int_enabled = (p->int_control & Z80PIO_INTCTRL_EI) ? true:false;
        p->expect_int_mask = false;
    }
    else {
        const uint8_t ctrl = data & 0x0F;
        if ((ctrl & 1) == 0) {
            /* set interrupt vector */
            p->int_vector = data;
            /* according to MAME setting the interrupt vector
               also enables interrupts, but this doesn't seem to
               be mentioned in the spec
            */
            p->int_control |= Z80PIO_INTCTRL_EI;
            p->int_enabled = true;
        }
        else if (ctrl == 0x0F) {
            /* set operating mode (Z80PIO_MODE_*) */
            p->mode = data>>6;
            if (p->mode == Z80PIO_MODE_OUTPUT) {
                /* make output visible */
                pio->out_cb(port_id, p->output, pio->user_data);
            }
            else if (p->mode == Z80PIO_MODE_BITCONTROL) {
                /* next control word is the io_select mask */
                p->expect_io_select = true;
                /* temporarly disable interrupt until io_select mask written */
                p->int_enabled = false;
                p->bctrl_match = false;
            }
        }
        else if (ctrl == 0x07) {
            /* set interrupt control word (Z80PIO_INTCTRL_*) */
            p->int_control = data & 0xF0;
            if (data & Z80PIO_INTCTRL_MASK_FOLLOWS) {
                /* next control word is the interrupt control mask */
                p->expect_int_mask = true;
                /* temporarly disable interrupt until mask is written */
                p->int_enabled = false;
                /* reset pending interrupt */
                p->int_state = 0;
                p->bctrl_match = false;
            }
            else {
                p->int_enabled = (p->int_control & Z80PIO_INTCTRL_EI) ? true:false;
            }
        }
        else if (ctrl == 0x03) {
            /* only set interrupt enable bit */
            p->int_control = (data & Z80PIO_INTCTRL_EI)|(p->int_control & ~Z80PIO_INTCTRL_EI);
            p->int_enabled = (p->int_control & Z80PIO_INTCTRL_EI) ? true:false;
        }
    }
}

/* read control word back to CPU */
uint8_t _z80pio_read_ctrl(z80pio_t* pio) {
    /* I haven't found documentation about what is
       returned when reading the control word, this
       is what MAME does
    */
    return (pio->port[Z80PIO_PORT_A].int_control & 0xC0) |
           (pio->port[Z80PIO_PORT_B].int_control >> 4);
}

/* new data word received from CPU */
void _z80pio_write_data(z80pio_t* pio, int port_id, uint8_t data) {
    CHIPS_ASSERT((port_id >= 0) && (port_id < Z80PIO_NUM_PORTS));
    z80pio_port_t* p = &pio->port[port_id];
    switch (p->mode) {
        case Z80PIO_MODE_OUTPUT:
            p->output = data;
            pio->out_cb(port_id, data, pio->user_data);
            break;
        case Z80PIO_MODE_INPUT:
            p->output = data;
            break;
        case Z80PIO_MODE_BIDIRECTIONAL:
            // FIXME
            break;
        case Z80PIO_MODE_BITCONTROL:
            p->output = data;
            pio->out_cb(port_id, p->io_select | (p->output & ~p->io_select), pio->user_data);
            break;
    }
}

/* read port data back to CPU */
uint8_t _z80pio_read_data(z80pio_t* pio, int port_id) {
    CHIPS_ASSERT((port_id >= 0) && (port_id < Z80PIO_NUM_PORTS));
    z80pio_port_t* p = &pio->port[port_id];
    uint8_t data = 0xFF;
    switch (p->mode) {
        case Z80PIO_MODE_OUTPUT:
            data = p->output;
            break;
        case Z80PIO_MODE_INPUT:
            p->input = pio->in_cb(port_id, pio->user_data);
            data = p->input;
            break;
        case Z80PIO_MODE_BIDIRECTIONAL:
            // FIXME
            break;
        case Z80PIO_MODE_BITCONTROL:
            p->input = pio->in_cb(port_id, pio->user_data);
            data = (p->input & p->io_select) | (p->output & ~p->io_select);
            break;
    }
    return data;
}

uint64_t z80pio_iorq(z80pio_t* pio, uint64_t pins) {
    if ((pins & (Z80PIO_CE|Z80PIO_IORQ|Z80PIO_M1)) == (Z80PIO_CE|Z80PIO_IORQ)) {
        const int port_id = (pins & Z80PIO_BASEL) ? Z80PIO_PORT_B : Z80PIO_PORT_A;
        if (pins & Z80PIO_RD) {
            uint8_t data;
            if (pins & Z80PIO_CDSEL) {
                data = _z80pio_read_ctrl(pio);
            }
            else {
                data = _z80pio_read_data(pio, port_id);
            }
            Z80PIO_SET_DATA(pins, data);
        }
        else {
            const uint8_t data = Z80PIO_GET_DATA(pins);
            if (pins & Z80PIO_CDSEL) {
                _z80pio_write_ctrl(pio, port_id, data);
            }
            else {
                _z80pio_write_data(pio, port_id, data);
            }
        }
    }
    return pins;
}

/* write value to a port, this may trigger an interrupt */
void z80pio_write_port(z80pio_t* pio, int port_id, uint8_t data) {
    CHIPS_ASSERT(pio && (port_id >= 0) && (port_id < Z80PIO_NUM_PORTS));
    z80pio_port_t* p = &pio->port[port_id];
    if (Z80PIO_MODE_BITCONTROL == p->mode) {
        p->input = data;
        uint8_t val = (p->input & p->io_select) | (p->output & ~p->io_select);
        uint8_t mask = ~p->int_mask;
        bool match = false;
        val &= mask;

        const uint8_t ictrl = p->int_control & 0x60;
        if ((ictrl == 0) && (val != mask)) match = true;
        else if ((ictrl == 0x20) && (val != 0)) match = true;
        else if ((ictrl == 0x40) && (val == 0)) match = true;
        else if ((ictrl == 0x60) && (val == mask)) match = true;
        if (!p->bctrl_match && match && (p->int_control & 0x80)) {
            p->int_state |= Z80PIO_INT_NEEDED;
        }
        p->bctrl_match = match;
    }
}

#endif /* CHIPS_IMPL */

#ifdef __cplusplus
} /* extern "C" */
#endif
