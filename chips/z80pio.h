#pragma once
/*
    z80pio.h -- emulates the Z80 PIO

                  +-----------+
            D0 <->|           |<-> A0
            .. <->|           |<-> ..
            D7 <->|           |<-> A7
         BASEL -->|           |--> ARDY
         CDSEL -->|           |<-- ASTB
            CE -->|           |
            M1 -->|           |<-> B0
          IORQ -->|    Z80    |<-> ..
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
*/

/* PIO control pins */
#define Z80PIO_BASEL    (1UL<<24)     /* port B or A select (set: B, clear: A) */
#define Z80PIO_CDSEL    (1UL<<25)     /* control or data select (set: control, clear: data) */
#define Z80PIO_CE       (1UL<<26)     /* chip enable */
#define Z80PIO_M1       (1UL<<27)     /* machine cycle one from CPU */
#define Z80PIO_IORQ     (1UL<<28)     /* input/output request from CPU */
#define Z80PIO_RD       (1UL<<29)     /* read cycle status from CPU */

/* interrupt control pins */
#define Z80PIO_INT      (1UL<<30)     /* interrupt request */
#define Z80PIO_IEI      (1UL<<31)     /* interrupt enable in */
#define Z80PIO_IEO      (1UL<<32)     /* interrupt enable out */

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
#define Z80PIO_MODE_CONTROL          (3) /* only port A */

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
    uint8_t mask;           /* 8-bit mask register */
    uint8_t io_select;      /* 8-bit input/output select register */
    uint8_t mask_control;   /* 2-bit mask control register */
    uint8_t int_vector;     /* 8-bit interrupt vector */
    uint8_t int_control;    /* interrupt control word (Z80PIO_INTCTRL_*) */
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
    bool in_reset_state;
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
#define Z80PIO_DATA(pins) ((uint8_t)pins)
/* set data bus pins D0..D7 */
#define Z80PIO_SET_DATA(pins,data) {pins=(pins&~0xFF)|(data&0xFF);}
/* extract port A or B pins A0..A7 or B0..B7 */
#define Z80PIO_PORT_DATA(pins,port) ((uint8_t)(pins>>(8+port*8)))
/* set port A or B pins A0..A7 or B0..B7 */
#define Z80PIO_SET_PORT_DATA(pins,port,data) {pins=(pins&~(0xFF<<(8+port*8)))|(((data&0xFF)<<(8+port*8)));}

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
    Calls this once to initialize a new Z80 PIO instance, this will
    clear the z80pio structure and go into reset state.
*/
void z80pio_init(z80pio* pio) {
    CHIPS_ASSERT(pio);
    memset(pio, 0, sizeof(z80pio));
    z80pio_reset(pio);
}

/*
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
        Z80PIO_SET_PORT_DATA(pio->PINS, p, 0x00);
        pio->PORT[p].mask = 0;
        pio->PORT[p].mode = Z80PIO_MODE_INPUT;
        pio->PORT[p].output = 0;
        pio->PORT[p].int_control &= ~Z80PIO_INTCTRL_EI;
    }
    pio->in_reset_state = true;
}
#endif /* CHIPS_IMPL */

#ifdef __cplusplus
} /* extern "C" */
#endif
