#pragma once
/*
    z80pio.h -- emulates the Z80 PIO (Parallel Input/Output)

                  +-----------+
            D0 <->|           |<-> A0
            .. <->|           |<-> ..
            D7 <->|           |<-> A7
         BASEL -->|           |--> ARDY
         CDSEL -->|           |<-- ASTB
            CE -->|    Z80    |
            M1 -->|    PIO    |<-> B0
          IORQ -->|           |<-> ..
            RD -->|           |<-> B7
           INT <--|           |--> BRDY
           IEI -->|           |<-- BSTB
           IEO <--|           |
                  +-----------+

    FIXME: pins emulation, ready/strobe, interrupts, bidirectional
*/
#include <stdint.h>
#include <stdbool.h>

#ifdef __cpluscplus
extern "C" {
}
#endif

/*
    Pin definitions. 
    
    All pin locations from 0 to 36 are shared with the CPU. Chip-type
    specific pins start at position 40. This enables efficient bus-sharing
    with the CPU and other Z80-family chips.

    Thus the Z80 PIO pin layout is as follows:

    0..15:      address bus A0..A15 (not connected)
    16..23:     data bus D0..D7
    24..36:     CPU pins (some of those shared directly with PIO)
    40..46:     PIO-specific pins

    FIXME: it probably makes sense to also directly share the IEI/IEO
    interrupt daisy chain pins that way
*/

/* control pins directly shared with CPU */
#define Z80PIO_M1       (1UL<<24)   /* CPU Machine Cycle One, same as Z80_M1 */
#define Z80PIO_IORQ     (1UL<<26)   /* IO Request from CPU, same as Z80_IORQ */
#define Z80PIO_RD       (1UL<<27)   /* Read Cycle Status from CPU, same as Z80_RD */
#define Z80PIO_INT      (1UL<<32)   /* Interrupt Request, same as Z80_INT */

/* Z80 interrupt daisy chain shared pins */
#define Z80PIO_IEI      (1UL<<37)   /* Interrupt Enable In */
#define Z80PIO_IEO      (1UL<<38)   /* Interrupt Enable Out */

/* PIO specific pins start at bit 40 */
#define Z80PIO_CE       (1UL<<40)   /* Chip Enable */
#define Z80PIO_BASEL    (1UL<<41)   /* Port A/B Select (inactive: A, active: B) */
#define Z80PIO_CDSEL    (1UL<<42)   /* Control/Data Select (inactive: data, active: control) */
#define Z80PIO_ARDY     (1UL<<43)   /* Port A Ready */
#define Z80PIO_BRDY     (1UL<<44)   /* Port B Ready */
#define Z80PIO_ASTB     (1UL<<45)   /* Port A Strobe */
#define Z80PIO_BSTB     (1UL<<46)   /* Port B Strobe */

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
    bool int_enabled;       /* definitive interrupt enabled flag */
    bool int_pending;       /* interrupt is pending */
    bool expect_io_select;  /* next control word will be io_select */
    bool expect_int_mask;   /* next control word will be  mask */
} z80pio_port;

/*
    Z80 PIO state
*/
typedef struct {
    /* port A and B register sets */
    z80pio_port PORT[Z80PIO_NUM_PORTS];
    /* currently in reset state? (until a control word is received) */
    bool reset_active;
    /* port-input callback */
    uint8_t (*in_cb)(int port_id);
    /* port-output callback */
    void (*out_cb)(int port_id, uint8_t data);
} z80pio;

/*
    Z80 PIO initialization struct, defines callbacks to read
    or write data on the PIOs I/O ports.
*/
typedef struct {
    /* input on port A or B requested */
    uint8_t (*in_cb)(int port_id);
    /* output to port A or B */
    void (*out_cb)(int port_id, uint8_t val);
} z80pio_desc;

/* extract 8-bit data bus from 64-bit pins */
#define Z80PIO_DATA(p) ((uint8_t)(p>>16))
/* merge 8-bit data bus value into 64-bit pins */
#define Z80PIO_SET_DATA(p,d) {p=((p&~0xFF0000)|((d&0xFF)<<16));}

/* initialize a new z80pio instance */
extern void z80pio_init(z80pio* pio, z80pio_desc* desc);
/* reset an existing z80pio instance */
extern void z80pio_reset(z80pio* pio);
/* per-tick function, reads or writes data/control bytes from/to PIO controlled by pins */
uint64_t z80pio_tick(z80pio* pio, uint64_t pins);

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
    z80pio_init

    Call this once to initialize a new Z80 PIO instance, this will
    clear the z80pio structure and go into reset state.
*/
void z80pio_init(z80pio* pio, z80pio_desc* desc) {
    CHIPS_ASSERT(pio);
    memset(pio, 0, sizeof(z80pio));
    pio->out_cb = desc->out_cb;
    pio->in_cb = desc->in_cb;
    z80pio_reset(pio);
}

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
void z80pio_reset(z80pio* pio) {
    CHIPS_ASSERT(pio);
    for (int p = 0; p < Z80PIO_NUM_PORTS; p++) {
        pio->PORT[p].mode = Z80PIO_MODE_INPUT;
        pio->PORT[p].output = 0;
        pio->PORT[p].io_select = 0;
        pio->PORT[p].int_control &= ~Z80PIO_INTCTRL_EI;
        pio->PORT[p].int_mask = 0xFF;
        pio->PORT[p].int_enabled = false;
        pio->PORT[p].int_pending = false;
        pio->PORT[p].expect_int_mask = false;
        pio->PORT[p].expect_io_select = false;
    }
    pio->reset_active = true;
}

/* new control word received from CPU */
static void _z80pio_write_ctrl(z80pio* pio, int port_id, uint8_t data) {
    CHIPS_ASSERT((port_id >= 0) && (port_id < Z80PIO_NUM_PORTS));
    pio->reset_active = false;
    z80pio_port* p = &pio->PORT[port_id];
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
                if (pio->out_cb) {
                    pio->out_cb(port_id, p->output);
                }
            }
            else if (p->mode == Z80PIO_MODE_BITCONTROL) {
                /* next control word is the io_select mask */
                p->expect_io_select = true;
                /* temporarly disable interrupt until io_select mask written */
                p->int_enabled = false;
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
                p->int_pending = false;
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
static uint8_t _z80pio_read_ctrl(z80pio* pio) {
    /* I haven't found documentation about what is
       returned when reading the control word, this
       is what MAME does
    */
    return (pio->PORT[Z80PIO_PORT_A].int_control & 0xC0) |
           (pio->PORT[Z80PIO_PORT_B].int_control >> 4);
}

/* new data word received from CPU */
static void _z80pio_write_data(z80pio* pio, int port_id, uint8_t data) {
    CHIPS_ASSERT((port_id >= 0) && (port_id < Z80PIO_NUM_PORTS));
    z80pio_port* p = &pio->PORT[port_id];
    switch (p->mode) {
        case Z80PIO_MODE_OUTPUT:
            p->output = data;
            if (pio->out_cb) {
                pio->out_cb(port_id, data);
            }
            break;
        case Z80PIO_MODE_INPUT:
            p->output = data;
            break;
        case Z80PIO_MODE_BIDIRECTIONAL:
            // FIXME
            break;
        case Z80PIO_MODE_BITCONTROL:
            p->output = data;
            if (pio->out_cb) {
                pio->out_cb(port_id, p->io_select | (p->output & ~p->io_select));
            }
            break;
    }
}

/* read port data back to CPU */
static uint8_t _z80pio_read_data(z80pio* pio, int port_id) {
    CHIPS_ASSERT((port_id >= 0) && (port_id < Z80PIO_NUM_PORTS));
    z80pio_port* p = &pio->PORT[port_id];
    uint8_t data = 0xFF;
    switch (p->mode) {
        case Z80PIO_MODE_OUTPUT:
            data = p->output;
            break;
        case Z80PIO_MODE_INPUT:
            if (pio->in_cb) {
                p->input = pio->in_cb(port_id);
            }
            data = p->input;
            break;
        case Z80PIO_MODE_BIDIRECTIONAL:
            // FIXME
            break;
        case Z80PIO_MODE_BITCONTROL:
            if (pio->in_cb) {
                p->input = pio->in_cb(port_id);
            }
            data = (p->input & p->io_select) | (p->output & ~p->io_select);
            break;
    }
    return data;
}

/*
    z80pio_tick

    Per-tick function, before calling this function the pins argument
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
uint64_t z80pio_tick(z80pio* pio, uint64_t pins) {
    if ((pins & (Z80PIO_CE|Z80PIO_IORQ)) == (Z80PIO_CE|Z80PIO_IORQ)) {
        const int port_id = (pins & Z80PIO_BASEL) ? Z80PIO_PORT_B : Z80PIO_PORT_A;
        if (pins & Z80PIO_RD) {
            /* read mode */
            uint8_t data;
            if (pins & Z80PIO_CDSEL) {
                /* select control mode */
                data = _z80pio_read_ctrl(pio);
            }
            else {
                /* select data mode */
                data = _z80pio_read_data(pio, port_id);
            }
            Z80PIO_SET_DATA(pins, data);
        }
        else {
            /* write mode */
            const uint8_t data = Z80PIO_DATA(pins);
            if (pins & Z80PIO_CDSEL) {
                /* select control mode */
                _z80pio_write_ctrl(pio, port_id, data);
            }
            else {
                /* select data mode */
                _z80pio_write_data(pio, port_id, data);
            }
        }
    }
    return pins;
}
#endif /* CHIPS_IMPL */

#ifdef __cplusplus
} /* extern "C" */
#endif
