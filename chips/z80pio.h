#pragma once
/*#
    # z80pio.h

    Header-only emulator for the Z80 PIO (Parallel Input/Output)
    written in C.

    Do this:
    ~~~C
    #define CHIPS_IMPL
    ~~~
    before you include this file in *one* C file to create the
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

    ## Functions:
    ~~~C
    void z80pio_init(z80pio_t* pio)
    ~~~
        Initialize a new Z80 PIO instance and enter the reset state.

    ~~~C
    void z80pio_reset(z80pio_t* pio)
    ~~~
        Reset the Z80 PIO instance.

        The reset state performs the following functions:
        1. Both port mask registers are reset to inhibit all port data bits.
        2. Port data bus lines are set to a high impedance state and the Ready
           "handshake" signals are inactive (low) Mode 1 is automatically selected.
        3. The vector address registers are NOT reset.
        4. Both port interrupt enable flip flops are reset
        5. Both port output registers are reset.

    ~~~C
    uint64_t z80pio_tick(z80pio_t* pio, uint64_t pins)
    ~~~
        Tick a Z80 PIO instance, call this for every CPU tick. Depending on the input
        pin mask and internal state, the PIO will perform IO requests from the CPU,
        check any input port conditions that might trigger, perform the
        interrupt daisychain protocol and returns a potentially modified
        pin mask.

        The PIO reads the following pins when an IO request should be performed:

        - **Z80PIO_CE|Z80PIO_IORQ**: performs an IO request
        - **Z80PIO_RD, Z80PIO_WR**: read or write direction for IO requests
        - **Z80PIO_BASEL**: select port A or B
        - **Z80PIO_CDSEL**: access control or data byte
        - **Z80PIO_D0..Z80PIO_D7**: the data byte for IO write requests

        The A/B port pins may trigger an interrupt depending on the current
        PIO configuration:

        - **Z80PIO_PA0..PA7, Z80PIO_PB0..PB7**

        The following output are potentially modified:

        - **Z80PIO_D0..Z80PIO_D7**: the resulting data byte of IO read requests
        - **Z80PIO_INT**: if the PIO wants to request an interrupt
        - **Z80PIO_PA0..PA7, Z80PIO_PB0..PB7**: may be modified depending on the
          current PIO configuration
        - **Z80PIO_IEIO**: enable or disable interrupts for daisychain downstream chips
#*/
/*
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

/*
    Pin definitions.

    All pin locations from 0 to 39 are shared with the CPU. Chip-type
    specific pins start at position 40. This enables efficient bus-sharing
    with the CPU and other Z80-family chips.

    Thus the Z80 PIO pin layout is as follows:

    0..15:      address bus A0..A15 (not connected)
    16..23:     data bus D0..D7
    24..36:     CPU pins (some of those shared directly with PIO)
    37..39      'virtual' interrupt system pins
    40..63      PIO-specific pins
*/

/* control pins directly shared with CPU */
#define Z80PIO_PIN_M1    (24)      /* CPU Machine Cycle One, same as Z80_M1 */
#define Z80PIO_PIN_IORQ  (26)      /* IO Request from CPU, same as Z80_IORQ */
#define Z80PIO_PIN_RD    (27)      /* Read Cycle Status from CPU, same as Z80_RD */
#define Z80PIO_PIN_INT   (30)      /* Interrupt Request, same as Z80_INT */

/* Z80 interrupt daisy chain shared pins */
#define Z80PIO_PIN_IEIO  (37)      /* combined Interrupt Enable In/Out (same as Z80_IEIO) */
#define Z80PIO_PIN_RETI  (38)      /* CPU has decoded a RETI instruction (same as Z80_RETI) */

/* PIO specific pins start at bit 40 */
#define Z80PIO_PIN_CE    (40)      /* Chip Enable */
#define Z80PIO_PIN_BASEL (41)      /* Port A/B Select (inactive: A, active: B) */
#define Z80PIO_PIN_CDSEL (42)      /* Control/Data Select (inactive: data, active: control) */
#define Z80PIO_PIN_ARDY  (43)      /* Port A Ready */
#define Z80PIO_PIN_BRDY  (44)      /* Port B Ready */
#define Z80PIO_PIN_ASTB  (45)      /* Port A Strobe */
#define Z80PIO_PIN_BSTB  (46)      /* Port B Strobe */

/* A/B 8-bit port pins */
#define Z80PIO_PIN_PA0   (48)
#define Z80PIO_PIN_PA1   (49)
#define Z80PIO_PIN_PA2   (50)
#define Z80PIO_PIN_PA3   (51)
#define Z80PIO_PIN_PA4   (52)
#define Z80PIO_PIN_PA5   (53)
#define Z80PIO_PIN_PA6   (54)
#define Z80PIO_PIN_PA7   (55)

#define Z80PIO_PIN_PB0   (56)
#define Z80PIO_PIN_PB1   (57)
#define Z80PIO_PIN_PB2   (58)
#define Z80PIO_PIN_PB3   (59)
#define Z80PIO_PIN_PB4   (60)
#define Z80PIO_PIN_PB5   (61)
#define Z80PIO_PIN_PB6   (62)
#define Z80PIO_PIN_PB7   (63)

// pin bit masks
#define Z80PIO_M1       (1ULL<<Z80PIO_PIN_M1)
#define Z80PIO_IORQ     (1ULL<<Z80PIO_PIN_IORQ)
#define Z80PIO_RD       (1ULL<<Z80PIO_PIN_RD)
#define Z80PIO_INT      (1ULL<<Z80PIO_PIN_INT)
#define Z80PIO_IEIO     (1ULL<<Z80PIO_PIN_IEIO)
#define Z80PIO_RETI     (1ULL<<Z80PIO_PIN_RETI)
#define Z80PIO_CE       (1ULL<<Z80PIO_PIN_CE)
#define Z80PIO_BASEL    (1ULL<<Z80PIO_PIN_BASEL)
#define Z80PIO_CDSEL    (1ULL<<Z80PIO_PIN_CDSEL)
#define Z80PIO_ARDY     (1ULL<<Z80PIO_PIN_ARDY)
#define Z80PIO_BRDY     (1ULL<<Z80PIO_PIN_BRDY)
#define Z80PIO_ASTB     (1ULL<<Z80PIO_PIN_ASTB)
#define Z80PIO_BSTB     (1ULL<<Z80PIO_PIN_BSTB)
#define Z80PIO_PA0      (1ULL<<Z80PIO_PIN_PA0)
#define Z80PIO_PA1      (1ULL<<Z80PIO_PIN_PA1)
#define Z80PIO_PA2      (1ULL<<Z80PIO_PIN_PA2)
#define Z80PIO_PA3      (1ULL<<Z80PIO_PIN_PA3)
#define Z80PIO_PA4      (1ULL<<Z80PIO_PIN_PA4)
#define Z80PIO_PA5      (1ULL<<Z80PIO_PIN_PA5)
#define Z80PIO_PA6      (1ULL<<Z80PIO_PIN_PA6)
#define Z80PIO_PA7      (1ULL<<Z80PIO_PIN_PA7)
#define Z80PIO_PB0      (1ULL<<Z80PIO_PIN_PB0)
#define Z80PIO_PB1      (1ULL<<Z80PIO_PIN_PB1)
#define Z80PIO_PB2      (1ULL<<Z80PIO_PIN_PB2)
#define Z80PIO_PB3      (1ULL<<Z80PIO_PIN_PB3)
#define Z80PIO_PB4      (1ULL<<Z80PIO_PIN_PB4)
#define Z80PIO_PB5      (1ULL<<Z80PIO_PIN_PB5)
#define Z80PIO_PB6      (1ULL<<Z80PIO_PIN_PB6)
#define Z80PIO_PB7      (1ULL<<Z80PIO_PIN_PB7)

/* Port Names */
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
#define Z80PIO_INT_SERVICED (1<<2)

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

/* Z80 PIO state. */
typedef struct {
    z80pio_port_t port[Z80PIO_NUM_PORTS];
    bool reset_active;  /* currently in reset state? (until a control word is received) */
    uint64_t pins;      /* last pin state (useful for debugging) */
} z80pio_t;

/* extract 8-bit data bus from 64-bit pins */
#define Z80PIO_GET_DATA(p) ((uint8_t)(((p)>>16)&0xFF))
/* merge 8-bit data bus value into 64-bit pins */
#define Z80PIO_SET_DATA(p,d) {p=((p)&~0xFF0000ULL)|(((d)<<16)&0xFF0000ULL);}
/* set 8-bit port A data on 64-bit pin mask */
#define Z80PIO_SET_PA(p,d) {p=(((p)&~0x00FF000000000000ULL)|((((uint64_t)(d))&0xFFULL)<<48));}
/* set 8-bit port B data on 64-bit pin mask */
#define Z80PIO_SET_PB(p,d) {p=(((p)&~0xFF00000000000000ULL)|((((uint64_t)(d))&0xFFULL)<<56));}
/* set both port A and B */
#define Z80PIO_SET_PAB(p,a,b) {p=((p)&~0xFFFF000000000000ULL)|((((uint64_t)(b))&0xFFULL)<<56)|((((uint64_t)(a))&0xFFULL)<<48);}
/* extract port A data */
#define Z80PIO_GET_PA(p) ((uint8_t)((p)>>48))
/* extract port B data */
#define Z80PIO_GET_PB(p) ((uint8_t)((p)>>56))
/* get port data from pins by port index */
#define Z80PIO_GET_PORT(p,pid) ((uint8_t)(p>>(48+(pid)*8)))

/* initialize a new Z80 PIO instance */
void z80pio_init(z80pio_t* pio);
/* reset a Z80 PIO instance */
void z80pio_reset(z80pio_t* pio);
/* tick the Z80 PIO instance */
uint64_t z80pio_tick(z80pio_t* pio, uint64_t pins);

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

#if defined(__GNUC__)
#define _Z80PIO_UNREACHABLE __builtin_unreachable()
#elif defined(_MSC_VER)
#define _Z80PIO_UNREACHABLE __assume(0)
#else
#define _Z80PIO_UNREACHABLE
#endif

void z80pio_init(z80pio_t* pio) {
    CHIPS_ASSERT(pio);
    memset(pio, 0, sizeof(z80pio_t));
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
            if (p->mode == Z80PIO_MODE_BITCONTROL) {
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
                p->int_state &= ~Z80PIO_INT_NEEDED;
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
    uint8_t data = (pio->port[Z80PIO_PORT_A].int_control & 0xC0) |
                   (pio->port[Z80PIO_PORT_B].int_control >> 4);
    return data;
}

/* new data word received from CPU */
void _z80pio_write_data(z80pio_t* pio, int port_id, uint8_t data) {
    // FIXME: in OUTPUT mode, the ARDY/BRDY pins must be set here
    CHIPS_ASSERT((port_id >= 0) && (port_id < Z80PIO_NUM_PORTS));
    z80pio_port_t* p = &pio->port[port_id];
    switch (p->mode) {
        case Z80PIO_MODE_OUTPUT:
            p->output = data;
            break;
        case Z80PIO_MODE_INPUT:
            p->output = data;
            break;
        case Z80PIO_MODE_BIDIRECTIONAL:
            // FIXME
            break;
        case Z80PIO_MODE_BITCONTROL:
            p->output = data;
            break;
        default:
            _Z80PIO_UNREACHABLE;
            break;
    }
}

/* read port data back to CPU */
uint8_t _z80pio_read_data(z80pio_t* pio, int port_id) {
    CHIPS_ASSERT((port_id >= 0) && (port_id < Z80PIO_NUM_PORTS));
    z80pio_port_t* p = &pio->port[port_id];
    switch (p->mode) {
        case Z80PIO_MODE_OUTPUT:
            return p->output;
        case Z80PIO_MODE_INPUT:
            return p->input;
        case Z80PIO_MODE_BIDIRECTIONAL:
            return 0xFF;
        case Z80PIO_MODE_BITCONTROL:
            return (p->input & p->io_select) | (p->output & ~p->io_select);
        default:
            _Z80PIO_UNREACHABLE;
            break;
    }
}

// set the PA and PB bits into the pin mask
uint64_t _z80pio_set_port_output_pins(z80pio_t* pio, uint64_t pins) {
    for (int port_id = 0; port_id < Z80PIO_NUM_PORTS; port_id++) {
        z80pio_port_t* p = &pio->port[port_id];
        uint8_t data;
        switch (p->mode) {
            case Z80PIO_MODE_OUTPUT:
                data = p->output;
                break;
            case Z80PIO_MODE_INPUT:
                data = 0xFF;
                break;
            case Z80PIO_MODE_BIDIRECTIONAL:
                // FIXME!
                data = 0xFF;
                break;
            case Z80PIO_MODE_BITCONTROL:
                // set input bits to 1
                data = p->io_select | (p->output & ~p->io_select);
                break;
            default:
                _Z80PIO_UNREACHABLE;
                break;
        }
        if (port_id == 0) {
            Z80PIO_SET_PA(pins, data);
        }
        else {
            Z80PIO_SET_PB(pins, data);
        }
    }
    return pins;
}

// handle an IO request from the CPU
uint64_t _z80pio_iorq(z80pio_t* pio, uint64_t pins) {
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
        uint8_t data = Z80PIO_GET_DATA(pins);
        if (pins & Z80PIO_CDSEL) {
            _z80pio_write_ctrl(pio, port_id, data);
        }
        else {
            _z80pio_write_data(pio, port_id, data);
        }
    }
    return pins;
}

// handle the interrupt daisy chain protocol
uint64_t _z80pio_int(z80pio_t* pio, uint64_t pins) {
    for (int i = 0; i < Z80PIO_NUM_PORTS; i++) {
        z80pio_port_t* p = &pio->port[i];
        // on RETI, only the highest priority interrupt that's currently being
        // serviced resets its state so that IEIO enables interrupt handling
        // on downstream devices, this must be allowed to happen even if a higher
        // priority device has entered interrupt handling
        if ((pins & Z80PIO_RETI) && (p->int_state & Z80PIO_INT_SERVICED)) {
            p->int_state &= ~Z80PIO_INT_SERVICED;
            pins &= ~Z80PIO_RETI;
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
        if ((p->int_state != 0) && (pins & Z80PIO_IEIO)) {
            // inhibit interrupt handling on downstream devices for the
            // entire duration of interrupt servicing
            pins &= ~Z80PIO_IEIO;
            // set INT pint active until the CPU acknowledges the interrupt
            if (p->int_state & Z80PIO_INT_NEEDED) {
                pins |= Z80PIO_INT;
                p->int_state = (p->int_state & ~Z80PIO_INT_NEEDED) | Z80PIO_INT_REQUESTED;
            }
            // interrupt ackowledge from CPU (M1|IORQ): put interrupt vector
            // on data bus, clear INT pin and go into "serviced" state.
            if ((p->int_state & Z80PIO_INT_REQUESTED) && ((pins & (Z80PIO_IORQ|Z80PIO_M1)) == (Z80PIO_IORQ|Z80PIO_M1))) {
                Z80PIO_SET_DATA(pins, p->int_vector);
                p->int_state = (p->int_state & ~Z80PIO_INT_REQUESTED) | Z80PIO_INT_SERVICED;
                pins &= ~Z80PIO_INT;
            }
        }
    }
    return pins;
}

void _z80pio_read_port_inputs(z80pio_t* pio, uint64_t pins) {
    for (int i = 0; i < Z80PIO_NUM_PORTS; i++) {
        z80pio_port_t* p = &pio->port[i];
        uint8_t data = (i == 0) ? Z80PIO_GET_PA(pins) : Z80PIO_GET_PB(pins);

        // this only needs to be evaluated if either the port input
        // or port state might have changed
        if ((data != p->input) || (pins & Z80PIO_CE)) {
            if (Z80PIO_MODE_INPUT == p->mode) {
                // FIXME: strobe/ready handshake and interrupt!
                p->input = data;
            }
            else if (Z80PIO_MODE_BITCONTROL == p->mode) {
                p->input = data;
                uint8_t val = (p->input & p->io_select) | (p->output & ~p->io_select);

                // check interrupt condition (FIXME: this is very expensive)
                uint8_t mask = ~p->int_mask;
                bool match = false;
                val &= mask;

                const uint8_t ictrl = p->int_control & 0x60;
                if ((ictrl == 0) && (val != mask)) match = true;
                else if ((ictrl == 0x20) && (val != 0)) match = true;
                else if ((ictrl == 0x40) && (val == 0)) match = true;
                else if ((ictrl == 0x60) && (val == mask)) match = true;
                if (!p->bctrl_match && match && p->int_enabled) {
                    p->int_state |= Z80PIO_INT_NEEDED;
                }
                p->bctrl_match = match;
            }
        }
    }
}

uint64_t z80pio_tick(z80pio_t* pio, uint64_t pins) {
    /*
        - OUTPUT MODE: On CPU write, the bus data is written to the output
          register, and the ARDY/BRDY pins must be set until the ASTB/BSTB pins
          changes from active to inactive. Strobe active=>inactive also an INT
          if the interrupt enable flip-flop is set and this device has the
          highest priority.

        - INPUT MODE (FIXME): When ASTB/BSTB goes active, data is loaded into the port's
          input register. When ASTB/BSTB then goes from active to inactive, an
          INT is generated is interrupt enable is set and this is the highest
          priority interrupt device. ARDY/BRDY goes active on ASTB/BSTB going
          inactive, and remains active until the CPU reads the input data.

        - BIDIRECTIONAL MODE: FIXME

        - BIT MODE: no handshake pins (ARDY/BRDY, ASTB/BSTB) are used. A CPU write
          cycle latches the data into the output register. On a CPU read cycle,
          the data returned to the CPU will be composed of output register data
          from those port data lines assigned as outputs and input register data
          from those port data lines assigned as inputs. The input register will
          contain data which was present immediately prior to the falling edge of RD.
          An interrupt will be generated if interrupts from the port are enabled and
          the data on the port data lines satisfy the logical equation defined by
          the 8-bit mask and 2-bit mask control registers
    */
    if ((pins & (Z80PIO_CE|Z80PIO_IORQ|Z80PIO_M1)) == (Z80PIO_CE|Z80PIO_IORQ)) {
        pins = _z80pio_iorq(pio, pins);
    }
    _z80pio_read_port_inputs(pio, pins);
    pins = _z80pio_set_port_output_pins(pio, pins);
    pins = _z80pio_int(pio, pins);
    pio->pins = pins;
    return pins;
}

#endif /* CHIPS_IMPL */
