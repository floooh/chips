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
*/
#include <stdint.h>
#include <stdbool.h>

#ifdef __cpluscplus
extern "C" {
}
#endif

/*
    Pins are grouped in a single uint64_t. The layout is as follows:

    0..7:       CPU DATA BUS
    8..15:      PORT A DATA
    16..23:     PORT B DATA
    24..36:     CONTROL PINS

    NOTE that the pins that are usually directly connected to CPU
    control pins share the same bit positions.
*/

/* PIO control pins */
#define Z80PIO_M1       (1UL<<24)     /* machine cycle one from CPU (same as Z80_M1) */
#define Z80PIO_CE       (1UL<<25)     /* chip enable */
#define Z80PIO_IORQ     (1UL<<26)     /* input/output request from CPU (same as Z80_IORQ) */
#define Z80PIO_RD       (1UL<<27)     /* read cycle status from CPU (same as Z80_RD) */
#define Z80PIO_BASEL    (1UL<<28)     /* port B or A select (set: B, clear: A) */
#define Z80PIO_CDSEL    (1UL<<29)     /* control or data select (set: control, clear: data) */

/* interrupt control pins */
#define Z80PIO_IEI      (1UL<<30)     /* interrupt enable in */
#define Z80PIO_IEO      (1UL<<31)     /* interrupt enable out */
#define Z80PIO_INT      (1UL<<32)     /* interrupt request (same Z80_INT) */

/* port A status pins */
#define Z80PIO_ASTB     (1UL<<33)     /* port A strobe pulse from peripheral device */
#define Z80PIO_ARDY     (1UL<<34)     /* register A ready */

/* port B status pins */
#define Z80PIO_BSTB     (1UL<<35)     /* port B strobe pulse from peripheral device */
#define Z80PIO_BRDY     (1UL<<36)     /* register B ready */

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
} z80pio;

/* initialize a new z80pio instance */
extern void z80pio_init(z80pio* pio);
/* reset an existing z80pio instance */
extern void z80pio_reset(z80pio* pio);
/* execute a tick on the z80pio instance */
extern void z80pio_tick(z80pio* pio);

/* set one or several port bits to active */
#define Z80PIO_ON(pins,bits) {pins=(pins&~bits)|bits;}
/* set one or several port bits to inactive */
#define Z80PIO_OFF(pins,bits) {pins&=~bits;}
/* extract data bus pins D0..D7 */
#define Z80PIO_GET_DATA(pins) ((uint8_t)pins)
/* set data bus pins D0..D7 */
#define Z80PIO_SET_DATA(pins,data) {pins=(pins&~0xFF)|(data&0xFF);}
/* extract port A or B pins A0..A7 or B0..B7 */
#define Z80PIO_GET_PORT(pins,port) ((uint8_t)(pins>>(8+port*8)))
/* set port A or B pins A0..A7 or B0..B7 */
#define Z80PIO_SET_PORT(pins,port,data) {pins=(pins&~(0xFF<<(8+port*8)))|(((data&0xFF)<<(8+port*8)));}

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
void z80pio_init(z80pio* pio) {
    CHIPS_ASSERT(pio);
    memset(pio, 0, sizeof(z80pio));
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
    Z80PIO_OFF(pio->PINS, Z80PIO_ARDY);
    Z80PIO_OFF(pio->PINS, Z80PIO_BRDY);
    for (int p = 0; p < Z80PIO_NUM_PORTS; p++) {
        Z80PIO_SET_PORT(pio->PINS, p, 0x00);
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

/* get port A or B from Z80PIO_ABSEL pin */
static int _z80pio_select_port(z80pio* pio, uint64_t pins) {
    return (pins & Z80PIO_BASEL) ? Z80PIO_PORT_B : Z80PIO_PORT_A;
}

/* new control word received from CPU */
static void _z80pio_write_control(z80pio* pio) {
    pio->reset_active = false;
    int port = _z80pio_select_port(pio, pio->PINS);
    z80pio_port* p = &pio->PORT[port];
    const uint8_t val = Z80PIO_GET_DATA(pio->PINS);
    if (p->expect_io_select) {
        /* followup io_select mask */
        p->io_select = val;
        p->int_enabled = (p->int_control & Z80PIO_INTCTRL_EI) ? true:false;
        p->expect_io_select = false;
    } 
    else if (p->expect_int_mask) {
        /* followup interrupt mask */
        p->int_mask = val;
        p->int_enabled = (p->int_control & Z80PIO_INTCTRL_EI) ? true:false;
        p->expect_int_mask = false;
    }
    else {
        const uint8_t ctrl = val & 0x0F;
        if ((ctrl & 1) == 0) {
            /* set interrupt vector */
            p->int_vector = val;
            /* according to MAME setting the interrupt vector
               also enables interrupts, but this doesn't seem to
               be mentioned in the spec
            */
            p->int_control |= Z80PIO_INTCTRL_EI;
            p->int_enabled = true;
        }
        else if (ctrl == 0x0F) {
            /* set operating mode (Z80PIO_MODE_*) */
            p->mode = val>>6;
            if (p->mode == Z80PIO_MODE_OUTPUT) {
                /* make output register visible on pins */
                Z80PIO_SET_PORT(pio->PINS,port,p->output);
                Z80PIO_ON(pio->PINS,Z80PIO_ARDY);
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
            p->int_control = val & 0xF0;
            if (val & Z80PIO_INTCTRL_MASK_FOLLOWS) {
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
            p->int_control = (val & Z80PIO_INTCTRL_EI)|(p->int_control & ~Z80PIO_INTCTRL_EI);
            p->int_enabled = (p->int_control & Z80PIO_INTCTRL_EI) ? true:false;
        }
    }
}

/* read control word back to CPU */
static uint8_t _z80pio_read_control(z80pio* pio) {
    /* I haven't found documentation about what is
       returned when reading the control word, this
       is what MAME does
    */
    return (pio->PORT[Z80PIO_PORT_A].int_control & 0xC0) |
           (pio->PORT[Z80PIO_PORT_B].int_control >> 4);
}

/* new data word received from CPU */
static void _z80pio_write_data(z80pio* pio) {

}

/* read port data back to CPU */
static uint8_t _z80pio_read_data(z80pio* pio) {
    // FIXME
    return 0xFF;
}

/* interrupt acknowledge from CPU */
static void _z80pio_interrupt_acknowledge(z80pio* pio) {
    // FIXME!
}

/*
    void z80pio_tick(z80pio* pio)

    The tick function is usually called from within the Z80 CPU tick
    callback after output pin state of the CPU has been transferred to
    input pins of the PIO.
*/
void z80pio_tick(z80pio* pio) {
    if ((pio->PINS & (Z80PIO_IORQ|Z80PIO_CE)) == (Z80PIO_IORQ|Z80PIO_CE)) {
        if (pio->PINS & Z80PIO_M1) {
            _z80pio_interrupt_acknowledge(pio);
        }
        if (pio->PINS & Z80PIO_RD) {
            /* PIO -> CPU */
            if (pio->PINS & Z80PIO_CDSEL) {
                uint8_t data = _z80pio_read_control(pio);
                Z80PIO_SET_DATA(pio->PINS, data);
            }
            else {
                uint8_t data = _z80pio_read_data(pio);
                Z80PIO_SET_DATA(pio->PINS, data);
            }
        }
        else {
            /* CPU -> PIO */
            if (pio->PINS & Z80PIO_CDSEL) {
                /* control word from CPU */
                _z80pio_write_control(pio);
            }
            else {
                /* data word to CPU */
                _z80pio_write_data(pio);
            }
        }
    }
}

#endif /* CHIPS_IMPL */

#ifdef __cplusplus
} /* extern "C" */
#endif
