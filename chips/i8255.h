#pragma once
/*
    i8255.h -- emulates the Intel 8255 PPI chip

    Do this:
        #define CHIPS_IMPL
    before you include this file in *one* C or C++ file to create the
    implementation.

    Optionally provide the following macros with your own implementation

        CHIPS_ASSERT(c)     -- your own assert macro (default: assert(c))

    EMULATED PINS:

                  +-----------+
            CS -->|           |<-> PA0
            RD -->|           |...
            WR -->|   i8255   |<-> PA7
            A0 -->|           |
            A1 -->|           |<-> PB0
                  |           |...
            D0 <->|           |<-> PB7
               ...|           |
            D7 <->|           |<-> PC0
                  |           |...
                  |           |<-> PC7
                  +-----------+

    NOT IMPLEMENTED:
        - mode 1 (strobed input/output)
        - mode 2 (bi-directional bus)
        - interrupts
        - input latches

    FIXME: documentation

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
*/
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
    Pin definitions.

    Pin locations 0 to 39 are reserved for the CPU. Data bus pins D0..D7
    are shared directly with the CPU.

    Control pin operations (XX: active, --: inactive, ..: don't care)

    | A1 | A0 | RD | WR | CS |
    +----+----+----+----+----+
    | -- | -- | XX | -- | XX |  read: port A -> data bus
    | -- | XX | XX | -- | XX |  read: port B -> data bus
    | XX | -- | XX | -- | XX |  read: port C -> data bus
    | XX | XX | XX | -- | XX |  read: control word -> data bus
    | -- | -- | -- | XX | XX |  write: data bus -> port A
    | -- | XX | -- | XX | XX |  write: data bus -> port B
    | XX | -- | -- | XX | XX |  write: data bus -> port C
    | XX | XX | -- | XX | XX |  write: data bus -> control

    Invoke those operations via to i8255_iorq() function.
*/

// control pins shared with Z80
#define I8255_PIN_RD    (27)    // read from PPI, shared with Z80_RD!
#define I8255_PIN_WR    (28)    // write to PPI, shared with Z80_WR!

// i8255 control pins
#define I8255_PIN_CS    (40)    // chip-select, i8255 responds to RW/WR when this is active

// register addressing pins same as address bus pins A0 and A1
#define I8255_PIN_A0    (0)
#define I8255_PIN_A1    (1)

// data bus pins shared with CPU
#define I8255_PIN_D0    (16)
#define I8255_PIN_D1    (17)
#define I8255_PIN_D2    (18)
#define I8255_PIN_D3    (19)
#define I8255_PIN_D4    (20)
#define I8255_PIN_D5    (21)
#define I8255_PIN_D6    (22)
#define I8255_PIN_D7    (23)

// port A pins
#define I8255_PIN_PA0  (48)
#define I8255_PIN_PA1  (49)
#define I8255_PIN_PA2  (50)
#define I8255_PIN_PA3  (51)
#define I8255_PIN_PA4  (52)
#define I8255_PIN_PA5  (53)
#define I8255_PIN_PA6  (54)
#define I8255_PIN_PA7  (55)

// port B pins
#define I8255_PIN_PB0  (56)
#define I8255_PIN_PB1  (57)
#define I8255_PIN_PB2  (58)
#define I8255_PIN_PB3  (59)
#define I8255_PIN_PB4  (60)
#define I8255_PIN_PB5  (61)
#define I8255_PIN_PB6  (62)
#define I8255_PIN_PB7  (63)

/* port C pins, NOTE: these overlap with address bus pins A8..A15!
   don't forget to clear upper 8 address bus pins when reusing the
   CPU pin mask for the i8255!
*/
#define I8255_PIN_PC0   (8)
#define I8255_PIN_PC1   (9)
#define I8255_PIN_PC2   (10)
#define I8255_PIN_PC3   (11)
#define I8255_PIN_PC4   (12)
#define I8255_PIN_PC5   (13)
#define I8255_PIN_PC6   (14)
#define I8255_PIN_PC7   (15)

// pin bit masks
#define I8255_RD        (1ULL<<I8255_PIN_RD)
#define I8255_WR        (1ULL<<I8255_PIN_WR)
#define I8255_CS        (1ULL<<I8255_PIN_CS)
#define I8255_A0        (1ULL<<I8255_PIN_A0)
#define I8255_A1        (1ULL<<I8255_PIN_A1)
#define I8255_D0        (1ULL<<I8255_PIN_D0)
#define I8255_D1        (1ULL<<I8255_PIN_D1)
#define I8255_D2        (1ULL<<I8255_PIN_D2)
#define I8255_D3        (1ULL<<I8255_PIN_D3)
#define I8255_D4        (1ULL<<I8255_PIN_D4)
#define I8255_D5        (1ULL<<I8255_PIN_D5)
#define I8255_D6        (1ULL<<I8255_PIN_D6)
#define I8255_D7        (1ULL<<I8255_PIN_D7)
#define I8255_PA0       (1ULL<<I8255_PIN_PA0)
#define I8255_PA1       (1ULL<<I8255_PIN_PA1)
#define I8255_PA2       (1ULL<<I8255_PIN_PA2)
#define I8255_PA3       (1ULL<<I8255_PIN_PA3)
#define I8255_PA4       (1ULL<<I8255_PIN_PA4)
#define I8255_PA5       (1ULL<<I8255_PIN_PA5)
#define I8255_PA6       (1ULL<<I8255_PIN_PA6)
#define I8255_PA7       (1ULL<<I8255_PIN_PA7)
#define I8255_PB0       (1ULL<<I8255_PIN_PB0)
#define I8255_PB1       (1ULL<<I8255_PIN_PB1)
#define I8255_PB2       (1ULL<<I8255_PIN_PB2)
#define I8255_PB3       (1ULL<<I8255_PIN_PB3)
#define I8255_PB4       (1ULL<<I8255_PIN_PB4)
#define I8255_PB5       (1ULL<<I8255_PIN_PB5)
#define I8255_PB6       (1ULL<<I8255_PIN_PB6)
#define I8255_PB7       (1ULL<<I8255_PIN_PB7)
#define I8255_PC0       (1ULL<<I8255_PIN_PC0)
#define I8255_PC1       (1ULL<<I8255_PIN_PC1)
#define I8255_PC2       (1ULL<<I8255_PIN_PC2)
#define I8255_PC3       (1ULL<<I8255_PIN_PC3)
#define I8255_PC4       (1ULL<<I8255_PIN_PC4)
#define I8255_PC5       (1ULL<<I8255_PIN_PC5)
#define I8255_PC6       (1ULL<<I8255_PIN_PC6)
#define I8255_PC7       (1ULL<<I8255_PIN_PC7)
#define I8255_PA_PINS   (I8255_PA0|I8255_PA1|I8255_PA2|I8255_PA3|I8255_PA4|I8255_PA5|I8255_PA6|I8255_PA7)
#define I8255_PB_PINS   (I8255_PB0|I8255_PB1|I8255_PB2|I8255_PB3|I8255_PB4|I8255_PB5|I8255_PB6|I8255_PB7)
#define I8255_PC_PINS   (I8255_PC0|I8255_PC1|I8255_PC2|I8255_PC3|I8255_PC4|I8255_PC5|I8255_PC6|I8255_PC7)
#define I8255_PORT_PINS (I8255_PA_PINS|I8255_PB_PINS|I8255_PC_PINS)

/*
    Control word bits

    MODE SELECT (bit 7: 1)

    | C7 | C6 | C5 | C4 | C3 | C2 | C1 | C0 |

    C0..C2: GROUP B control bits:
        C0: port C (lower) in/out:  0=output, 1=input
        C1: port B in/out:          0=output, 1=input
        C2: mode select:            0=mode0 (basic in/out), 1=mode1 (strobed in/out)

    C3..C6: GROUP A control bits:
        C3: port C (upper) in/out:  0=output, 1=input
        C4: port A in/out:          0=output, 1=input
        C5+C6: mode select:         00=mode0 (basic in/out)
                                    01=mode1 (strobed in/out)
                                    1x=mode2 (bi-directional bus)

    C7: 1 for 'mode select'

    INTERRUPT CONTROL (bit 7: 0)

    Interrupt handling is currently not implemented
*/

/* mode select or interrupt control */
#define I8255_CTRL_CONTROL          (1<<7)
#define I8255_CTRL_CONTROL_MODE     (1<<7)
#define I8255_CTRL_CONTROL_BIT      (0)

/* port C (lower) input/output select */
#define I8255_CTRL_CLO              (1<<0)
#define I8255_CTRL_CLO_INPUT        (1<<0)
#define I8255_CTRL_CLO_OUTPUT       (0)

/* port B input/output select */
#define I8255_CTRL_B                (1<<1)
#define I8255_CTRL_B_INPUT          (1<<1)
#define I8255_CTRL_B_OUTPUT         (0)

/* group B mode select */
#define I8255_CTRL_BCLO_MODE        (1<<2)
#define I8255_CTRL_BCLO_MODE_0      (0)
#define I8255_CTRL_BCLO_MODE_1      (1<<2)

/* port C (upper) input/output select */
#define I8255_CTRL_CHI              (1<<3)
#define I8255_CTRL_CHI_INPUT        (1<<3)
#define I8255_CTRL_CHI_OUTPUT       (0)

/* port A input/output select */
#define I8255_CTRL_A                (1<<4)
#define I8255_CTRL_A_INPUT          (1<<4)
#define I8255_CTRL_A_OUTPUT         (0)

/* group A mode select */
#define I8255_CTRL_ACHI_MODE        ((1<<6)|(1<<5))
#define I8255_CTRL_ACHI_MODE_0      (0)
#define I8255_CTRL_ACHI_MODE_1      (1<<5)
/* otherwise mode 2 */

/* set/reset bit (for I8255_CTRL_CONTROL_BIT) */
#define I8255_CTRL_BIT              (1<<0)
#define I8255_CTRL_BIT_SET          (1<<0)
#define I8255_CTRL_BIT_RESET        (0)

/* i8255 port state */
typedef struct {
    uint8_t outp;   /* output latch (FIXME: input latch) */
} i8255_port_t;

/* i8255 state */
typedef struct {
    i8255_port_t pa, pb, pc;
    uint8_t control;
    uint64_t pins;
} i8255_t;

/* extract 8-bit data bus from 64-bit pins */
#define I8255_GET_DATA(p) ((uint8_t)((p)>>16))
/* merge 8-bit data bus value into 64-bit pins */
#define I8255_SET_DATA(p,d) {p=((p)&~0xFF0000ULL)|(((d)<<16)&0xFF0000ULL);}
/* extract port pins */
#define I8255_GET_PA(p) ((uint8_t)((p)>>48))
#define I8255_GET_PB(p) ((uint8_t)((p)>>56))
#define I8255_GET_PC(p) ((uint8_t)((p)>>8))
/* set port pins into pin mask */
#define I8255_SET_PA(p,a) {p=((p)&0xFF00FFFFFFFFFFFFULL)|(((a)&0xFFULL)<<48);}
#define I8255_SET_PB(p,b) {p=((p)&0x00FFFFFFFFFFFFFFULL)|(((b)&0xFFULL)<<56);}
#define I8255_SET_PC(p,c) {p=((p)&0xFFFFFFFFFFFF00FFULL)|(((c)&0xFFULL)<<8);}
#define I8255_SET_PCHI(p,c) {p=((p)&0xFFFFFFFFFFFF0FFFULL)|(((c)&0xF0ULL)<<8);}
#define I8255_SET_PCLO(p,c) {p=((p)&0xFFFFFFFFFFFFF0FFULL)|(((c)&0x0FULL)<<8);}

/* initialize a new i8255_t instance */
void i8255_init(i8255_t* ppi);
/* reset i8255_t instance */
void i8255_reset(i8255_t* ppi);
/* tick i8255_t instance */
uint64_t i8255_tick(i8255_t* ppi, uint64_t pins);

#ifdef __cplusplus
} /* extern "C" */
#endif

/*--- IMPLEMENTATION ---------------------------------------------------------*/
#ifdef CHIPS_IMPL
#include <string.h>
#ifndef CHIPS_ASSERT
    #include <assert.h>
    #define CHIPS_ASSERT(c) assert(c)
#endif

void i8255_init(i8255_t* ppi) {
    CHIPS_ASSERT(ppi);
    memset(ppi, 0, sizeof(*ppi));
    i8255_reset(ppi);
}

void i8255_reset(i8255_t* ppi) {
    CHIPS_ASSERT(ppi);
    /* set all ports to input */
    ppi->control = I8255_CTRL_CONTROL_MODE|
                   I8255_CTRL_CLO_INPUT|
                   I8255_CTRL_CHI_INPUT|
                   I8255_CTRL_B_INPUT|
                   I8255_CTRL_A_INPUT;
    ppi->pa.outp = 0;
}

/* write a value to the PPI */
static void _i8255_write(i8255_t* ppi, uint64_t pins) {
    uint8_t data = I8255_GET_DATA(pins);
    switch (pins & (I8255_A0|I8255_A1)) {
        case 0: /* write to port A */
            if ((ppi->control & I8255_CTRL_A) == I8255_CTRL_A_OUTPUT) {
                ppi->pa.outp = data;
            }
            break;
        case 1: /* write to port B */
            if ((ppi->control & I8255_CTRL_B) == I8255_CTRL_B_OUTPUT) {
                ppi->pb.outp = data;
            }
            break;
        case 2: /* write to port C */
            ppi->pc.outp = data;
            break;
        case 3: /* control operation*/
            if ((data & I8255_CTRL_CONTROL) == I8255_CTRL_CONTROL_MODE) {
                /* set port mode */
                ppi->control = data;
                ppi->pa.outp = 0;
                ppi->pb.outp = 0;
                ppi->pc.outp = 0;
            }
            else {
                /* set/clear single bit in port C */
                const uint8_t mask = 1<<((data>>1)&7);
                if ((data & I8255_CTRL_BIT) == I8255_CTRL_BIT_SET) {
                    ppi->pc.outp |= mask;
                }
                else {
                    ppi->pc.outp &= ~mask;
                }
            }
            break;
    }
}

/* read a value from the PPI */
static uint64_t _i8255_read(i8255_t* ppi, uint64_t pins) {
    uint8_t data = 0xFF;
    switch (pins & (I8255_A0|I8255_A1)) {
        case 0: /* read from port A */
            if ((ppi->control & I8255_CTRL_A) == I8255_CTRL_A_OUTPUT) {
                data = ppi->pa.outp;
            }
            else {
                data = I8255_GET_PA(pins);
            }
            break;
        case 1: /* read from port B */
            if ((ppi->control & I8255_CTRL_B) == I8255_CTRL_B_OUTPUT) {
                data = ppi->pb.outp;
            }
            else {
                data = I8255_GET_PB(pins);
            }
            break;
        case 2: /* read from port C */
            data = I8255_GET_PC(pins);
            if ((ppi->control & I8255_CTRL_CHI) == I8255_CTRL_CHI_OUTPUT) {
                data = (data & 0x0F) | (ppi->pc.outp & 0xF0);
            }
            if ((ppi->control & I8255_CTRL_CLO) == I8255_CTRL_CLO_OUTPUT) {
                data = (data & 0xF0) | (ppi->pc.outp & 0x0F);
            }
            break;
        case 3: /* read control word */
            data = ppi->control;
            break;
    }
    I8255_SET_DATA(pins, data);
    return pins;
}

static uint64_t _i8255_write_port_pins(i8255_t* ppi, uint64_t pins) {
    if ((ppi->control & I8255_CTRL_A_OUTPUT) == I8255_CTRL_A_OUTPUT) {
        I8255_SET_PA(pins, ppi->pa.outp);
    }
    if ((ppi->control & I8255_CTRL_B_OUTPUT) == I8255_CTRL_B_OUTPUT) {
        I8255_SET_PB(pins, ppi->pb.outp);
    }
    if ((ppi->control & I8255_CTRL_CHI) == I8255_CTRL_CHI_OUTPUT) {
        I8255_SET_PCHI(pins, ppi->pc.outp);
    }
    if ((ppi->control & I8255_CTRL_CLO) == I8255_CTRL_CLO_OUTPUT) {
        I8255_SET_PCLO(pins, ppi->pc.outp);
    }
    return pins;
}

uint64_t i8255_tick(i8255_t* ppi, uint64_t pins) {
    CHIPS_ASSERT(ppi);
    if (pins & I8255_CS) {
        if (pins & I8255_RD) {
            pins = _i8255_read(ppi, pins);
        }
        else if (pins & I8255_WR) {
            _i8255_write(ppi, pins);
        }
    }
    pins = _i8255_write_port_pins(ppi, pins);
    ppi->pins = pins;
    return pins;
}

#endif /* CHIPS_IMPL */
