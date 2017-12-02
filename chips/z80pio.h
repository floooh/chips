#pragma once
/*
    z80pio.h -- emulates the Z80 PIO

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
    /* the PIO pins */
    uint64_t PINS; 
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
    init-desc structure, communication with devices
    is done with callbacks, not passively through pins
    (at least for now)
*/
typedef struct {
    /* input on port A or B requested */
    uint8_t (*in_cb)(int port_id);
    /* output to port A or B */
    void (*out_cb)(int port_id, uint8_t val);
} z80pio_desc;

/* initialize a new z80pio instance */
extern void z80pio_init(z80pio* pio, z80pio_desc* desc);
/* reset an existing z80pio instance */
extern void z80pio_reset(z80pio* pio);
/* write control byte (port_id is Z80PIO_PORT_A or Z80PIO_PORT_B) */
void z80pio_write_ctrl(z80pio* pio, int port_id, uint8_t val);
/* read control byte (this returns the same result for both ports) */
uint8_t z80pio_read_ctrl(z80pio* pio);
/* write data byte */
void z80pio_write_data(z80pio* pio, int port_id, uint8_t val);
/* read data byte */
uint8_t z80pio_read_data(z80pio* pio, int port_id);

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
    void z80pio_init(z80pio* pio);

    Calls this once to initialize a new Z80 PIO instance, this will
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
    void z80pio_reset(z80pio* pio);

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
void z80pio_write_ctrl(z80pio* pio, int port_id, uint8_t data) {
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
uint8_t z80pio_read_ctrl(z80pio* pio) {
    /* I haven't found documentation about what is
       returned when reading the control word, this
       is what MAME does
    */
    return (pio->PORT[Z80PIO_PORT_A].int_control & 0xC0) |
           (pio->PORT[Z80PIO_PORT_B].int_control >> 4);
}

/* new data word received from CPU */
void z80pio_write_data(z80pio* pio, int port_id, uint8_t data) {
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
uint8_t z80pio_read_data(z80pio* pio, int port_id) {
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
#endif /* CHIPS_IMPL */

#ifdef __cplusplus
} /* extern "C" */
#endif
