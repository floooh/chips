#pragma once
/*#
    # z80pio.h

    FIXME FIXME FIXME!

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
    uint64_t z80pio_iorq(z80pio_t* pio, uint64_t pins)
    ~~~~
        Performs an IO request on the PIO. Call this from the CPU
        tick callback when the CPU executes an IO machine cycle
        on a port address assigned to the PIO. This takes the
        state of PIO pins as input (control pins and data bus), and 
        returns a pin mask with new data-bus state.

        The following input pins are read by the function:

        - **M1**:       must be unset (otherwise this would be an interrupt request cycle)
        - **IORQ|CE**:  must be set ('chip-enable')
        - **RD**:       set for read/input operations
        - **WR**:       set for write/output operations
        - **CDSEL**:    set to read/write control byte, unset to read/write data byte
        - **BASEL**:    set to select port B, unset to select port A
        - **D0..D7**:   control- or data-byte to write into PIO

        The following output pins may be modified by the function:

        - **D0..D7**:   control- or data-byte read from PIO

    ~~~C
    void z80pio_write_port(z80pio_t* pio, int port_id, uint8_t data)
    ~~~
        Actively write a byte to PIO port A or B. This may trigger an interrupt.

    ~~~C
    uint64_t z80pio_int(z80pio_t* pio, uint64_t pins)
    ~~~
        Handle the daisy-chain interrupt protocol. See the z80.h header
        for details.

    ## Macros

    ~~~C
    Z80PIO_SET_DATA(p,d)
    ~~~
        set 8-bit data bus pins in 64-bit pin mask (identical with
        Z80_SET_DATA() from the z80.h header)

    ~~~C
    Z80PIO_GET_DATA(p)
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
#define Z80PIO_M1       (1ULL<<24)      /* CPU Machine Cycle One, same as Z80_M1 */
#define Z80PIO_IORQ     (1ULL<<26)      /* IO Request from CPU, same as Z80_IORQ */
#define Z80PIO_RD       (1ULL<<27)      /* Read Cycle Status from CPU, same as Z80_RD */
#define Z80PIO_INT      (1ULL<<30)      /* Interrupt Request, same as Z80_INT */

/* Z80 interrupt daisy chain shared pins */
#define Z80PIO_IEIO     (1ULL<<37)      /* combined Interrupt Enable In/Out (same as Z80_IEIO) */
#define Z80PIO_RETI     (1ULL<<38)      /* CPU has decoded a RETI instruction (same as Z80_RETI) */

/* PIO specific pins start at bit 40 */
#define Z80PIO_CE       (1ULL<<40)      /* Chip Enable */
#define Z80PIO_BASEL    (1ULL<<41)      /* Port A/B Select (inactive: A, active: B) */
#define Z80PIO_CDSEL    (1ULL<<42)      /* Control/Data Select (inactive: data, active: control) */
#define Z80PIO_ARDY     (1ULL<<43)      /* Port A Ready */
#define Z80PIO_BRDY     (1ULL<<44)      /* Port B Ready */
#define Z80PIO_ASTB     (1ULL<<45)      /* Port A Strobe */
#define Z80PIO_BSTB     (1ULL<<46)      /* Port B Strobe */

/* A/B 8-bit port pins */
#define Z80PIO_PA0      (1ULL<<48)
#define Z80PIO_PA1      (1ULL<<49)
#define Z80PIO_PA2      (1ULL<<50)
#define Z80PIO_PA3      (1ULL<<51)
#define Z80PIO_PA4      (1ULL<<52)
#define Z80PIO_PA5      (1ULL<<53)
#define Z80PIO_PA6      (1ULL<<54)
#define Z80PIO_PA7      (1ULL<<55)

#define Z80PIO_PB0      (1ULL<<56)
#define Z80PIO_PB1      (1ULL<<57)
#define Z80PIO_PB2      (1ULL<<58)
#define Z80PIO_PB3      (1ULL<<59)
#define Z80PIO_PB4      (1ULL<<60)
#define Z80PIO_PB5      (1ULL<<61)
#define Z80PIO_PB6      (1ULL<<62)
#define Z80PIO_PB7      (1ULL<<63)

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
#define Z80PIO_GET_DATA(p) ((uint8_t)(p>>16))
/* merge 8-bit data bus value into 64-bit pins */
#define Z80PIO_SET_DATA(p,d) {p=((p&~0xFF0000)|((d&0xFF)<<16));}
/* set 8-bit port A data on 64-bit pin mask */
#define Z80PIO_SET_PA(p,d) {p=((p&~0x00FF000000000000ULL)|((((uint64_t)d)&0xFFULL)<<48));}
/* set 8-bit port B data on 64-bit pin mask */
#define Z80PIO_SET_PB(p,d) {p=((p&~0xFF00000000000000ULL)|((((uint64_t)d)&0xFFULL)<<56));}
/* set both port A and B */
#define Z80PIO_SET_PAB(p,a,b) {p=(p&~0xFFFF000000000000ULL)|((((uint64_t)b)&0xFFULL)<<56)|((((uint64_t)a)&0xFFULL)<<48);}
/* extract port A data */
#define Z80PIO_GET_PA(p) ((uint8_t)(p>>48))
/* extract port B data */
#define Z80PIO_GET_PB(p) ((uint8_t)(p>>56))
/* get port data from pins by port index */
#define Z80PIO_GET_PORT(p,pid) ((uint8_t)(p>>(48+pid*8)))

/* initialize a new Z80 PIO instance */
void z80pio_init(z80pio_t* pio);
/* reset a Z80 PIO instance */
void z80pio_reset(z80pio_t* pio);
/* tick the Z80 PIO instance */
uint64_t z80pio_tick(z80pio_t* pio, uint64_t pins);
/* write value to a PIO port, this may trigger an interrupt */
void z80pio_write_port(z80pio_t* pio, int port_id, uint8_t data);

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
    }
}

/* read port data back to CPU */
uint8_t _z80pio_read_data(z80pio_t* pio, int port_id, uint64_t pins) {
    CHIPS_ASSERT((port_id >= 0) && (port_id < Z80PIO_NUM_PORTS));
    z80pio_port_t* p = &pio->port[port_id];
    uint8_t data = 0xFF;
    switch (p->mode) {
        case Z80PIO_MODE_OUTPUT:
            data = p->output;
            break;
        case Z80PIO_MODE_INPUT:
            // FIXME: in INPUT MODE, the input register is only updated
            // on ASTB/BSTB!!!
            p->input = data = Z80PIO_GET_PORT(pins,port_id);
            break;
        case Z80PIO_MODE_BIDIRECTIONAL:
            // FIXME
            break;
        case Z80PIO_MODE_BITCONTROL:
            p->input = Z80PIO_GET_PORT(pins,port_id);
            data = (p->input & p->io_select) | (p->output & ~p->io_select);
            break;
    }
    return data;
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
            data = _z80pio_read_data(pio, port_id, pins);
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
        if ((p->int_state & Z80PIO_INT_NEEDED) && (pins & Z80PIO_IEIO)) {
            // disable interrupt handling for downstream devices
            pins &= ~Z80PIO_IEIO;
            // request interrupt
            pins |= Z80PIO_INT;
            // answer the CPU's M1|IORQ cycle (put interrupt vector on data bus)
            if ((pins & (Z80PIO_IORQ|Z80PIO_M1)) == (Z80PIO_IORQ|Z80PIO_M1)) {
                Z80PIO_SET_DATA(pins, p->int_vector);
                p->int_state &= ~Z80PIO_INT_NEEDED;
                pins &= ~Z80PIO_INT;
            }
        }
        // RETI is visible to all daisy chain devices, no matter of IEIO state
// FIXME: hmm there isn't anything to do here on RETI now???
//        if (pins & Z80PIO_RETI) {
//            p->int_state &= ~Z80PIO_INT_SERVICING;
//        }
    }
    return pins;
}

uint64_t z80pio_tick(z80pio_t* pio, uint64_t pins) {
    /*  FIXME:

        - OUTPUT MODE: On CPU write, the bus data is written to the output
          register (already happens in _z80pio_write_data()), and the ARDY/BRDY
          pins must be set until the ASTB/BSTB pins changes from active to inactive.
          Strobe active=>inactive also an INT if the interrupt enable flip-flop
          is set and this device has the highest priority.

        - INPUT MODE: When ASTB/BSTB goes active, data is loaded into the port's
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
    pins = _z80pio_set_port_output_pins(pio, pins);
    pins = _z80pio_int(pio, pins);

    pio->pins = pins;
    return pins;
}

/* write value to a port, this may trigger an interrupt */
void z80pio_write_port(z80pio_t* pio, int port_id, uint8_t data) {
    CHIPS_ASSERT(pio && (port_id >= 0) && (port_id < Z80PIO_NUM_PORTS));
    z80pio_port_t* p = &pio->port[port_id];
    if (Z80PIO_MODE_BITCONTROL == p->mode) {
        p->input = data;
        uint8_t val = (p->input & p->io_select) | (p->output & ~p->io_select);
        //p->port = val;
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
