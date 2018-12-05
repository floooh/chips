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

/* i8255 control pins */
#define I8255_CS    (1ULL<<40)     /* chip-select, i8255 responds to RW/WR when this is active */
#define I8255_RD    (1ULL<<41)     /* read from PPI */
#define I8255_WR    (1ULL<<42)     /* write to PPI */
#define I8255_A0    (1ULL<<43)     /* address bit 0 */
#define I8255_A1    (1ULL<<44)     /* address bit 1 */

/* data bus pins shared with CPU */
#define I8255_D0  (1ULL<<16)
#define I8255_D1  (1ULL<<17)
#define I8255_D2  (1ULL<<18)
#define I8255_D3  (1ULL<<19)
#define I8255_D4  (1ULL<<20)
#define I8255_D5  (1ULL<<21)
#define I8255_D6  (1ULL<<22)
#define I8255_D7  (1ULL<<23)

/* NOTE the Port I/O pins are only visible in the 'pins' member,
   they are not accepted as input, and are not returned as output
   of the i8255_iorq() function! Usually the pins member
   is used for debug visualization, since the port I/O is handled
   through callback functions.
*/
#define I8255_PA0       (1ULL<<0)
#define I8255_PA1       (1ULL<<1)
#define I8255_PA2       (1ULL<<2)
#define I8255_PA3       (1ULL<<3)
#define I8255_PA4       (1ULL<<4)
#define I8255_PA5       (1ULL<<5)
#define I8255_PA6       (1ULL<<6)
#define I8255_PA7       (1ULL<<7)

#define I8255_PB0       (1ULL<<8)
#define I8255_PB1       (1ULL<<9)
#define I8255_PB2       (1ULL<<10)
#define I8255_PB3       (1ULL<<11)
#define I8255_PB4       (1ULL<<12)
#define I8255_PB5       (1ULL<<13)
#define I8255_PB6       (1ULL<<14)
#define I8255_PB7       (1ULL<<15)

#define I8255_PC0       (1ULL<<48)
#define I8255_PC1       (1ULL<<49)
#define I8255_PC2       (1ULL<<50)
#define I8255_PC3       (1ULL<<51)
#define I8255_PC4       (1ULL<<52)
#define I8255_PC5       (1ULL<<53)
#define I8255_PC6       (1ULL<<54)
#define I8255_PC7       (1ULL<<55)

#define I8255_PA_BITS   (0x00000000000000FFULL)
#define I8255_PB_BITS   (0x000000000000FF00ULL)
#define I8255_PC_BITS   (0x00FF000000000000ULL)
#define I8255_PORT_BITS (I8255_PA_BITS|I8255_PB_BITS|I8255_PC_BITS)

/* port names */
#define I8255_PORT_A    (0)
#define I8255_PORT_B    (1)
#define I8255_PORT_C    (2)
#define I8255_NUM_PORTS (3)

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
#define I8255_CTRL_CONTROL               (1<<7)
#define I8255_CTRL_CONTROL_MODE          (1<<7)
#define I8255_CTRL_CONTROL_BIT           (0)

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

/* callbacks for input/output on ports */
typedef uint8_t (*i8255_in_t)(int port_id, void* user_data);
typedef uint64_t (*i8255_out_t)(int port_id, uint64_t pins, uint8_t data, void* user_data);

/* initialization parameters */
typedef struct {
    i8255_in_t in_cb;       /* port-input callback */
    i8255_out_t out_cb;     /* port-output callback */
    void* user_data;        /* optional user-data for callbacks */
} i8255_desc_t;

/* i8255 state */
typedef struct {
    /* port output latches */
    uint8_t output[I8255_NUM_PORTS];
    /* control word */
    uint8_t control;
    /* last input or output value on port */
    uint8_t port[I8255_NUM_PORTS];
    /* last pin state in i8255_iorq(), with PA/PB/PC pins merged in */
    uint64_t pins;
    /* port-input callback */
    i8255_in_t in_cb;
    /* port-output callback */
    i8255_out_t out_cb;
    /* optional user-data for callbacks */
    void* user_data;
} i8255_t;

/* extract 8-bit data bus from 64-bit pins */
#define I8255_GET_DATA(p) ((uint8_t)(p>>16))
/* merge 8-bit data bus value into 64-bit pins */
#define I8255_SET_DATA(p,d) {p=((p&~0xFF0000)|((d&0xFF)<<16));}

/*
    i8255_init

    This initializes a new i8255 instance, which will clear
    the i8255_t struct and put the chip into its reset state.

    ppi     -- pointer to a i8255_t instance
    in_cb   -- function to be called when input on a port is needed
    out_cb  -- function to be called when output on a port is performed
*/
void i8255_init(i8255_t* ppi, const i8255_desc_t* desc);

/*
    i8255_reset

    Puts the i8255 into the reset state. Clears the control word register
    and puts all ports into input mode.
*/
void i8255_reset(i8255_t* ppi);

/*
    i8255_iorq

    Performs a read or write operation on the PPI. The pins 
    CS|RD|WR define whether this is a read or write operation,
    the address pins A0|A1 define the port number, or
    whether the control word is affected. If this is a
    write operation, the data bus pins must contain the value
    to be written.

    Calling this function may cause invocation of the in/out
    callback function, and the data bus pins may be modified
    (if this is a read operation).
*/
uint64_t i8255_iorq(i8255_t* ppi, uint64_t pins);

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

void i8255_init(i8255_t* ppi, const i8255_desc_t* desc) {
    CHIPS_ASSERT(ppi && desc && desc->in_cb && desc->out_cb);
    memset(ppi, 0, sizeof(*ppi));
    ppi->in_cb = desc->in_cb;
    ppi->out_cb = desc->out_cb;
    ppi->user_data = desc->user_data;
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
    for (int i = 0; i < I8255_NUM_PORTS; i++) {
        ppi->output[i] = 0;
        ppi->port[i] = 0xFF;
    }
}

/* handle output on port A or B */
static inline uint64_t _i8255_out_a(i8255_t* ppi, uint64_t pins, uint8_t data) {
    ppi->output[I8255_PORT_A] = data;
    ppi->port[I8255_PORT_A] = data;
    return ppi->out_cb(I8255_PORT_A, pins, data, ppi->user_data);
}

static inline uint64_t _i8255_out_b(i8255_t* ppi, uint64_t pins, uint8_t data) {
    ppi->output[I8255_PORT_B] = data;
    ppi->port[I8255_PORT_B] = data;
    return ppi->out_cb(I8255_PORT_B, pins, data, ppi->user_data);
}

/* handle output on port C (which is split in upper/lower half) */
static inline uint64_t _i8255_out_c(i8255_t* ppi, uint64_t pins) {
    uint8_t mask = 0, data = 0;
    if ((ppi->control & I8255_CTRL_CHI) == I8255_CTRL_CHI_OUTPUT) {
        mask |= 0xF0;
    }
    else {
        data |= 0xF0;   /* in input mode, return all bits as set */
    }
    if ((ppi->control & I8255_CTRL_CLO) == I8255_CTRL_CLO_OUTPUT) {
        mask |= 0x0F;
    }
    else {
        data |= 0x0F;
    }
    data |= ppi->output[I8255_PORT_C] & mask;
    ppi->port[I8255_PORT_C] = (ppi->port[I8255_PORT_C] & ~mask) | (data & mask);
    pins = ppi->out_cb(I8255_PORT_C, pins, data, ppi->user_data);
    return pins;
}

/* handle input from port A or B */
static inline uint8_t _i8255_in_a(i8255_t* ppi) {
    uint8_t data;
    if ((ppi->control & I8255_CTRL_A) == I8255_CTRL_A_OUTPUT) {
        data = ppi->output[I8255_PORT_A];
    }
    else {
        data = ppi->in_cb(I8255_PORT_A, ppi->user_data);
    }
    ppi->port[I8255_PORT_A] = data;
    return data;
}

static inline uint8_t _i8255_in_b(i8255_t* ppi) {
    uint8_t data;
    if ((ppi->control & I8255_CTRL_B) == I8255_CTRL_B_OUTPUT) {
        data = ppi->output[I8255_PORT_B];
    }
    else {
        data = ppi->in_cb(I8255_PORT_B, ppi->user_data);
    }
    ppi->port[I8255_PORT_B] = data;
    return data;
}

/* handle input on port C (which is split in upper and lower half) */
static inline uint8_t _i8255_in_c(i8255_t* ppi) {
    uint8_t mask = 0, data = 0;
    if ((ppi->control & I8255_CTRL_CHI) == I8255_CTRL_CHI_OUTPUT) {
        /* read data from output latch */
        data |= ppi->output[I8255_PORT_C] & 0xF0;
    }
    else {
        /* read data from port */
        mask |= 0xF0;
    }
    if ((ppi->control & I8255_CTRL_CLO) == I8255_CTRL_CLO_OUTPUT) {
        /* read data from output latch */
        data |= ppi->output[I8255_PORT_C] & 0x0F;
    }
    else {
        /* read data from port */
        mask |= 0x0F;
    }
    if (0 != mask) {
        data |= ppi->in_cb(I8255_PORT_C, ppi->user_data);
    }
    ppi->port[I8255_PORT_C] = (ppi->port[I8255_PORT_C] & ~mask) | (data & mask);
    return data;
}

/* set new control word */
static uint64_t _i8255_select_mode(i8255_t* ppi, uint64_t pins, uint8_t data) {
    ppi->control = data;
    for (int i = 0; i < I8255_NUM_PORTS; i++) {
        ppi->output[i] = 0;
        ppi->port[i] = 0;
    }
    if ((ppi->control & I8255_CTRL_A) == I8255_CTRL_A_OUTPUT) {
        pins = _i8255_out_a(ppi, pins, 0);
    }
    else {
        pins = _i8255_out_a(ppi, pins, 0xFF);
    }
    if ((ppi->control & I8255_CTRL_B) == I8255_CTRL_B_OUTPUT) {
        pins = _i8255_out_b(ppi, pins, 0);
    }
    else {
        pins = _i8255_out_b(ppi, pins, 0xFF);
    }
    pins = _i8255_out_c(ppi, pins);
    return pins;
}

/* write a value to the PPI */
static uint64_t _i8255_write(i8255_t* ppi, uint64_t pins, uint8_t data) {
    switch (pins & (I8255_A0|I8255_A1)) {
        case 0: /* write to port A */
            if ((ppi->control & I8255_CTRL_A) == I8255_CTRL_A_OUTPUT) {
                pins = _i8255_out_a(ppi, pins, data);
            }
            break;
        case I8255_A0: /* write to port B */
            if ((ppi->control & I8255_CTRL_B) == I8255_CTRL_B_OUTPUT) {
                pins = _i8255_out_b(ppi, pins, data);
            }
            break;
        case I8255_A1: /* write to port C */
            ppi->output[I8255_PORT_C] = data;
            pins = _i8255_out_c(ppi, pins);
            break;
        case (I8255_A0|I8255_A1): /* control operation*/
            if ((data & I8255_CTRL_CONTROL) == I8255_CTRL_CONTROL_MODE) {
                /* set port mode */
                pins = _i8255_select_mode(ppi, pins, data);
            }
            else {
                /* set/clear single bit in port C */
                const uint8_t mask = 1<<((data>>1)&7);
                if ((data & I8255_CTRL_BIT) == I8255_CTRL_BIT_SET) {
                    /* set bit */
                    ppi->output[I8255_PORT_C] |= mask;
                }
                else {
                    /* clear bit */
                    ppi->output[I8255_PORT_C] &= ~mask;
                }
                /* FIXME: interrupts, buffer full flags */
                pins = _i8255_out_c(ppi, pins);
            }
            break;
    }
    return pins;
}

/* read a value from the PPI */
static uint8_t _i8255_read(i8255_t* ppi, uint64_t pins) {
    uint8_t data = 0xFF;
    switch (pins & (I8255_A0|I8255_A1)) {
        case 0: /* read from port A */
            data = _i8255_in_a(ppi);
            break;
        case I8255_A0: /* read from port B */
            data = _i8255_in_b(ppi);
            break;
        case I8255_A1: /* read from port C */
            data = _i8255_in_c(ppi);
            break;
        case (I8255_A0|I8255_A1): /* read control word */
            data = ppi->control;
            break;
    }
    return data;
}

uint64_t i8255_iorq(i8255_t* ppi, uint64_t pins) {
    CHIPS_ASSERT(ppi);
    if (pins & I8255_CS) {
        if (pins & I8255_RD) {
            /* read from PPI */
            const uint8_t data = _i8255_read(ppi, pins);
            I8255_SET_DATA(pins, data);
        }
        else if (pins & I8255_WR) {
            /* write to PPI */
            const uint8_t data = I8255_GET_DATA(pins);
            pins = _i8255_write(ppi, pins, data);
        }
        ppi->pins = (pins & ~I8255_PORT_BITS) |
                    (ppi->port[I8255_PORT_A] * I8255_PA0) |
                    (ppi->port[I8255_PORT_B] * I8255_PB0) |
                    (ppi->port[I8255_PORT_C] * I8255_PC0);
    }
    return pins;
}

#endif /* CHIPS_IMPL */
