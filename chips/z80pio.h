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

/*
    Pins are grouped in a single uint64_t. The layout is as follows:

    0..7:       CPU DATA BUS
    8..15:      PORT A DATA
    16..23:     PORT B DATA
    24..36:     CONTROL PINS
*/

/* PIO control pins */
#define Z80PIO_BASEL    (1<<24)     /* port B or A select (set: B, clear: A) */
#define Z80PIO_CDSEL    (1<<25)     /* control or data select (set: control, clear: data) */
#define Z80PIO_CE       (1<<26)     /* chip enable */
#define Z80PIO_M1       (1<<27)     /* machine cycle one from CPU */
#define Z80PIO_IORQ     (1<<28)     /* input/output request from CPU */
#define Z80PIO_RD       (1<<29)     /* read cycle status from CPU */

/* interrupt control pins */
#define Z80PIO_INT      (1<<30)     /* interrupt request */
#define Z80PIO_IEI      (1<<31)     /* interrupt enable in */
#define Z80PIO_IEO      (1<<32)     /* interrupt enable out */

/* port A status pins */
#define Z80PIO_ASTB     (1<<33)     /* port A strobe pulse from peripheral device */
#define Z80PIO_ARDY     (1<<34)     /* register A ready */

/* port B status pins */
#define Z80PIO_BSTB     (1<<35)     /* port B strobe pulse from peripheral device */
#define Z80PIO_BRDY     (1<<36)     /* register B ready */

