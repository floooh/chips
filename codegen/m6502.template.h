#pragma once
/*#
    # m6502.h

    MOS Technology 6502 / 6510 CPU emulator.

    Project repo: https://github.com/floooh/chips/
    
    NOTE: this file is code-generated from m6502.template.h and m6502_gen.py
    in the 'codegen' directory.

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

    ## Emulated Pins

    ***********************************
    *           +-----------+         *
    *   IRQ --->|           |---> A0  *
    *   NMI --->|           |...      *
    *    RDY--->|           |---> A15 *
    *    RES--->|           |         *
    *    RW <---|           |         *
    *  SYNC <---|           |         *
    *           |           |<--> D0  *
    *   (P0)<-->|           |...      *
    *        ...|           |<--> D7  *
    *   (P5)<-->|           |         *
    *           +-----------+         *
    ***********************************

    The input/output P0..P5 pins only exist on the m6510.

    If the RDY pin is active (1) the CPU will loop on the next read
    access until the pin goes inactive.

    ## Overview

    m6502.h implements a cycle-stepped 6502/6510 CPU emulator, meaning
    that the emulation state can be ticked forward in clock cycles instead
    of full instructions.

    To initialize the emulator, fill out a m6502_desc_t structure with
    initialization parameters and then call m6502_init(). 

        ~~~C
        typedef struct {
            bool bcd_disabled;          // set to true if BCD mode is disabled
            m6510_in_t in_cb;           // only m6510: port IO input callback
            m6510_out_t out_cb;         // only m6510: port IO output callback
            uint8_t m6510_io_pullup;    // only m6510: IO port bits that are 1 when reading
            uint8_t m6510_io_floating;  // only m6510: unconnected IO port pins
            void* m6510_user_data;      // only m6510: optional in/out callback user data
         } m6502_desc_t;
         ~~~

    At the end of m6502_init(), the CPU emulation will be at the start of
    RESET state, and the first 7 ticks will execute the reset sequence
    (loading the reset vector at address 0xFFFC and continuing execution
    there.

    m6502_init() will return a 64-bit pin mask which must be the input argument
    to the first call of m6502_tick().

    To execute instructions, call m6502_tick() in a loop. m6502_tick() takes
    a 64-bit pin mask as input, executes one clock tick, and returns
    a modified pin mask.

    After executing one tick, the pin mask must be inspected, a memory read
    or write operation must be performed, and the modified pin mask must be
    used for the next call to m6502_tick(). This 64-bit pin mask is how
    the CPU emulation communicates with the outside world.

    The simplest-possible execution loop would look like this:

        ~~~C
        // setup 64 kBytes of memory
        uint8_t mem[1<<16] = { ... };
        // initialize the CPU
        m6502_t cpu;
        uint64_t pins = m6502_init(&cpu, &(m6502_desc_t){...});
        while (...) {
            // run the CPU emulation for one tick
            pins = m6502_tick(&cpu, pins);
            // extract 16-bit address from pin mask
            const uint16_t addr = M6502_GET_ADDR(pins);
            // perform memory access
            if (pins & M6502_RW) {
                // a memory read
                M6502_SET_DATA(pins, mem[addr]);
            }
            else {
                // a memory write
                mem[addr] = M6502_GET_DATA(pins);
            }
        }
        ~~~

    To start a reset sequence, set the M6502_RES bit in the pin mask and
    continue calling the m6502_tick() function. At the start of the next
    instruction, the CPU will initiate the 7-tick reset sequence. You do NOT
    need to clear the M6502_RES bit, this will be cleared when the reset
    sequence starts.

    To request an interrupt, set the M6502_IRQ or M6502_NMI bits in the pin
    mask and continue calling the tick function. The interrupt sequence
    will be initiated at the end of the current or next instruction
    (depending on the exact cycle the interrupt pin has been set).
    
    Unlike the M6502_RES pin, you are also responsible for clearing the
    interrupt pins (typically, the interrupt lines are cleared by the chip
    which requested the interrupt once the CPU reads a chip's interrupt
    status register to check which chip requested the interrupt).

    To find out whether a new instruction is about to start, check if the
    M6502_SYNC pin is set.

    To "goto" a random address at any time, a 'prefetch' like this is
    necessary (this basically simulates a normal instruction fetch from
    address 'next_pc'). This is usually only needed in "trap code" which
    intercepts operating system calls, executes some native code to emulate
    the operating system call, and then continue execution somewhere else:

        ~~~C
        pins = M6502_SYNC;
        M6502_SET_ADDR(pins, next_pc);
        M6502_SET_DATA(pins, mem[next_pc]);
        m6502_set_pc(next_pc);
        ~~~~

    ## Functions
    ~~~C
    uint64_t m6502_init(m6502_t* cpu, const m6502_desc_t* desc)
    ~~~
        Initialize a m6502_t instance, the desc structure provides
        initialization attributes:
            ~~~C
            typedef struct {
                bool bcd_disabled;              // set to true if BCD mode is disabled
                m6510_in_t m6510_in_cb;         // m6510 only: optional port IO input callback
                m6510_out_t m6510_out_cb;       // m6510 only: optional port IO output callback
                void* m6510_user_data;          // m6510 only: optional callback user data
                uint8_t m6510_io_pullup;        // m6510 only: IO port bits that are 1 when reading
                uint8_t m6510_io_floating;      // m6510 only: unconnected IO port pins
            } m6502_desc_t;
            ~~~

        To emulate a m6510 you must provide port IO callbacks in m6510_in_cb
        and m6510_out_cb, and should initialize the m6510_io_pullup and
        m6510_io_floating members to indicate which of the IO pins are
        connected or hardwired to a 1-state.

    ~~~C
    uint64_t m6502_tick(m6502_t* cpu, uint64_t pins)
    ~~~
        Tick the CPU for one clock cycle. The 'pins' argument and return value
        is the current state of the CPU pins used to communicate with the
        outside world (see the Overview section above for details).

    ~~~C
    uint64_t m6510_iorq(m6502_t* cpu, uint64_t pins)
    ~~~
        For the 6510, call this function after the tick callback when memory
        access to the special addresses 0 and 1 are requested. m6510_iorq()
        may call the input/output callback functions provided in m6502_desc_t.

    ~~~C
    void m6502_set_x(m6502_t* cpu, uint8_t val)
    void m6502_set_xx(m6502_t* cpu, uint16_t val)
    uint8_t m6502_x(m6502_t* cpu)
    uint16_t m6502_xx(m6502_t* cpu)
    ~~~
        Set and get 6502 registers and flags.


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

/* address bus pins */
#define M6502_A0    (1ULL<<0)
#define M6502_A1    (1ULL<<1)
#define M6502_A2    (1ULL<<2)
#define M6502_A3    (1ULL<<3)
#define M6502_A4    (1ULL<<4)
#define M6502_A5    (1ULL<<5)
#define M6502_A6    (1ULL<<6)
#define M6502_A7    (1ULL<<7)
#define M6502_A8    (1ULL<<8)
#define M6502_A9    (1ULL<<9)
#define M6502_A10   (1ULL<<10)
#define M6502_A11   (1ULL<<11)
#define M6502_A12   (1ULL<<12)
#define M6502_A13   (1ULL<<13)
#define M6502_A14   (1ULL<<14)
#define M6502_A15   (1ULL<<15)

/* data bus pins */
#define M6502_D0    (1ULL<<16)
#define M6502_D1    (1ULL<<17)
#define M6502_D2    (1ULL<<18)
#define M6502_D3    (1ULL<<19)
#define M6502_D4    (1ULL<<20)
#define M6502_D5    (1ULL<<21)
#define M6502_D6    (1ULL<<22)
#define M6502_D7    (1ULL<<23)

/* control pins */
#define M6502_RW    (1ULL<<24)      /* out: memory read or write access */
#define M6502_SYNC  (1ULL<<25)      /* out: start of a new instruction */
#define M6502_IRQ   (1ULL<<26)      /* in: maskable interrupt requested */
#define M6502_NMI   (1ULL<<27)      /* in: non-maskable interrupt requested */
#define M6502_RDY   (1ULL<<28)      /* in: freeze execution at next read cycle */
#define M6510_AEC   (1ULL<<29)      /* in, m6510 only, put bus lines into tristate mode, not implemented */
#define M6502_RES   (1ULL<<30)      /* request RESET */

/* m6510 IO port pins */
#define M6510_P0    (1ULL<<32)
#define M6510_P1    (1ULL<<33)
#define M6510_P2    (1ULL<<34)
#define M6510_P3    (1ULL<<35)
#define M6510_P4    (1ULL<<36)
#define M6510_P5    (1ULL<<37)
#define M6510_PORT_BITS (M6510_P0|M6510_P1|M6510_P2|M6510_P3|M6510_P4|M6510_P5)

/* bit mask for all CPU pins (up to bit pos 40) */
#define M6502_PIN_MASK ((1ULL<<40)-1)

/* status indicator flags */
#define M6502_CF    (1<<0)  /* carry */
#define M6502_ZF    (1<<1)  /* zero */
#define M6502_IF    (1<<2)  /* IRQ disable */
#define M6502_DF    (1<<3)  /* decimal mode */
#define M6502_BF    (1<<4)  /* BRK command */
#define M6502_XF    (1<<5)  /* unused */
#define M6502_VF    (1<<6)  /* overflow */
#define M6502_NF    (1<<7)  /* negative */

/* internal BRK state flags */
#define M6502_BRK_IRQ   (1<<0)  /* IRQ was triggered */
#define M6502_BRK_NMI   (1<<1)  /* NMI was triggered */
#define M6502_BRK_RESET (1<<2)  /* RES was triggered */

/* m6510 IO port callback prototypes */
typedef void (*m6510_out_t)(uint8_t data, void* user_data);
typedef uint8_t (*m6510_in_t)(void* user_data);

/* the desc structure provided to m6502_init() */
typedef struct {
    bool bcd_disabled;              /* set to true if BCD mode is disabled */
    m6510_in_t m6510_in_cb;         /* optional port IO input callback (only on m6510) */
    m6510_out_t m6510_out_cb;       /* optional port IO output callback (only on m6510) */
    void* m6510_user_data;          /* optional callback user data */
    uint8_t m6510_io_pullup;        /* IO port bits that are 1 when reading */
    uint8_t m6510_io_floating;      /* unconnected IO port pins */
} m6502_desc_t;

/* CPU state */
typedef struct {
    uint16_t IR;        /* internal instruction register */
    uint16_t PC;        /* internal program counter register */
    uint16_t AD;        /* ADL/ADH internal register */
    uint8_t A,X,Y,S,P;  /* regular registers */
    uint64_t PINS;      /* last stored pin state (do NOT modify) */
    uint16_t irq_pip;
    uint16_t nmi_pip;
    uint8_t brk_flags;  /* M6502_BRK_* */
    uint8_t bcd_enabled;
    /* 6510 IO port state */
    void* user_data;
    m6510_in_t in_cb;
    m6510_out_t out_cb;
    uint8_t io_ddr;     /* 1: output, 0: input */
    uint8_t io_inp;     /* last port input */
    uint8_t io_out;     /* last port output */
    uint8_t io_pins;    /* current state of IO pins (combined input/output) */
    uint8_t io_pullup;
    uint8_t io_floating;
    uint8_t io_drive;
} m6502_t;

/* initialize a new m6502 instance and return initial pin mask */
uint64_t m6502_init(m6502_t* cpu, const m6502_desc_t* desc);
/* execute one tick */
uint64_t m6502_tick(m6502_t* cpu, uint64_t pins);
/* perform m6510 port IO (only call this if M6510_CHECK_IO(pins) is true) */
uint64_t m6510_iorq(m6502_t* cpu, uint64_t pins);

/* register access functions */
void m6502_set_a(m6502_t* cpu, uint8_t v);
void m6502_set_x(m6502_t* cpu, uint8_t v);
void m6502_set_y(m6502_t* cpu, uint8_t v);
void m6502_set_s(m6502_t* cpu, uint8_t v);
void m6502_set_p(m6502_t* cpu, uint8_t v);
void m6502_set_pc(m6502_t* cpu, uint16_t v);
uint8_t m6502_a(m6502_t* cpu);
uint8_t m6502_x(m6502_t* cpu);
uint8_t m6502_y(m6502_t* cpu);
uint8_t m6502_s(m6502_t* cpu);
uint8_t m6502_p(m6502_t* cpu);
uint16_t m6502_pc(m6502_t* cpu);

/* extract 16-bit address bus from 64-bit pins */
#define M6502_GET_ADDR(p) ((uint16_t)(p&0xFFFFULL))
/* merge 16-bit address bus value into 64-bit pins */
#define M6502_SET_ADDR(p,a) {p=((p&~0xFFFFULL)|((a)&0xFFFFULL));}
/* extract 8-bit data bus from 64-bit pins */
#define M6502_GET_DATA(p) ((uint8_t)((p&0xFF0000ULL)>>16))
/* merge 8-bit data bus value into 64-bit pins */
#define M6502_SET_DATA(p,d) {p=(((p)&~0xFF0000ULL)|(((d)<<16)&0xFF0000ULL));}
/* copy data bus value from other pin mask */
#define M6502_COPY_DATA(p0,p1) (((p0)&~0xFF0000ULL)|((p1)&0xFF0000ULL))
/* return a pin mask with control-pins, address and data bus */
#define M6502_MAKE_PINS(ctrl, addr, data) ((ctrl)|(((data)<<16)&0xFF0000ULL)|((addr)&0xFFFFULL))
/* set the port bits on the 64-bit pin mask */
#define M6510_SET_PORT(p,d) {p=(((p)&~M6510_PORT_BITS)|((((uint64_t)d)<<32)&M6510_PORT_BITS));}
/* M6510: check for IO port access to address 0 or 1 */
#define M6510_CHECK_IO(p) ((p&0xFFFEULL)==0)

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

/* register access functions */
void m6502_set_a(m6502_t* cpu, uint8_t v) { cpu->A = v; }
void m6502_set_x(m6502_t* cpu, uint8_t v) { cpu->X = v; }
void m6502_set_y(m6502_t* cpu, uint8_t v) { cpu->Y = v; }
void m6502_set_s(m6502_t* cpu, uint8_t v) { cpu->S = v; }
void m6502_set_p(m6502_t* cpu, uint8_t v) { cpu->P = v; }
void m6502_set_pc(m6502_t* cpu, uint16_t v) { cpu->PC = v; }
uint8_t m6502_a(m6502_t* cpu) { return cpu->A; }
uint8_t m6502_x(m6502_t* cpu) { return cpu->X; }
uint8_t m6502_y(m6502_t* cpu) { return cpu->Y; }
uint8_t m6502_s(m6502_t* cpu) { return cpu->S; }
uint8_t m6502_p(m6502_t* cpu) { return cpu->P; }
uint16_t m6502_pc(m6502_t* cpu) { return cpu->PC; }

/* helper macros and functions for code-generated instruction decoder */
#define _M6502_NZ(p,v) ((p&~(M6502_NF|M6502_ZF))|((v&0xFF)?(v&M6502_NF):M6502_ZF))

static inline void _m6502_adc(m6502_t* cpu, uint8_t val) {
    if (cpu->bcd_enabled && (cpu->P & M6502_DF)) {
        /* decimal mode (credit goes to MAME) */
        uint8_t c = cpu->P & M6502_CF ? 1 : 0;
        cpu->P &= ~(M6502_NF|M6502_VF|M6502_ZF|M6502_CF);
        uint8_t al = (cpu->A & 0x0F) + (val & 0x0F) + c;
        if (al > 9) {
            al += 6;
        }
        uint8_t ah = (cpu->A >> 4) + (val >> 4) + (al > 0x0F);
        if (0 == (uint8_t)(cpu->A + val + c)) {
            cpu->P |= M6502_ZF;
        }
        else if (ah & 0x08) {
            cpu->P |= M6502_NF;
        }
        if (~(cpu->A^val) & (cpu->A^(ah<<4)) & 0x80) {
            cpu->P |= M6502_VF;
        }
        if (ah > 9) {
            ah += 6;
        }
        if (ah > 15) {
            cpu->P |= M6502_CF;
        }
        cpu->A = (ah<<4) | (al & 0x0F);
    }
    else {
        /* default mode */
        uint16_t sum = cpu->A + val + (cpu->P & M6502_CF ? 1:0);
        cpu->P &= ~(M6502_VF|M6502_CF);
        cpu->P = _M6502_NZ(cpu->P,sum);
        if (~(cpu->A^val) & (cpu->A^sum) & 0x80) {
            cpu->P |= M6502_VF;
        }
        if (sum & 0xFF00) {
            cpu->P |= M6502_CF;
        }
        cpu->A = sum & 0xFF;
    }    
}

static inline void _m6502_sbc(m6502_t* cpu, uint8_t val) {
    if (cpu->bcd_enabled && (cpu->P & M6502_DF)) {
        /* decimal mode (credit goes to MAME) */
        uint8_t c = cpu->P & M6502_CF ? 0 : 1;
        cpu->P &= ~(M6502_NF|M6502_VF|M6502_ZF|M6502_CF);
        uint16_t diff = cpu->A - val - c;
        uint8_t al = (cpu->A & 0x0F) - (val & 0x0F) - c;
        if ((int8_t)al < 0) {
            al -= 6;
        }
        uint8_t ah = (cpu->A>>4) - (val>>4) - ((int8_t)al < 0);
        if (0 == (uint8_t)diff) {
            cpu->P |= M6502_ZF;
        }
        else if (diff & 0x80) {
            cpu->P |= M6502_NF;
        }
        if ((cpu->A^val) & (cpu->A^diff) & 0x80) {
            cpu->P |= M6502_VF;
        }
        if (!(diff & 0xFF00)) {
            cpu->P |= M6502_CF;
        }
        if (ah & 0x80) {
            ah -= 6;
        }
        cpu->A = (ah<<4) | (al & 0x0F);
    }
    else {
        /* default mode */
        uint16_t diff = cpu->A - val - (cpu->P & M6502_CF ? 0 : 1);
        cpu->P &= ~(M6502_VF|M6502_CF);
        cpu->P = _M6502_NZ(cpu->P, (uint8_t)diff);
        if ((cpu->A^val) & (cpu->A^diff) & 0x80) {
            cpu->P |= M6502_VF;
        }
        if (!(diff & 0xFF00)) {
            cpu->P |= M6502_CF;
        }
        cpu->A = diff & 0xFF;
    }
}

static inline void _m6502_cmp(m6502_t* cpu, uint8_t r, uint8_t v) {
    uint16_t t = r - v;
    cpu->P = (_M6502_NZ(cpu->P, (uint8_t)t) & ~M6502_CF) | ((t & 0xFF00) ? 0:M6502_CF);
}

static inline uint8_t _m6502_asl(m6502_t* cpu, uint8_t v) {
    cpu->P = (_M6502_NZ(cpu->P, v<<1) & ~M6502_CF) | ((v & 0x80) ? M6502_CF:0);
    return v<<1;
}

static inline uint8_t _m6502_lsr(m6502_t* cpu, uint8_t v) {
    cpu->P = (_M6502_NZ(cpu->P, v>>1) & ~M6502_CF) | ((v & 0x01) ? M6502_CF:0);
    return v>>1;
}

static inline uint8_t _m6502_rol(m6502_t* cpu, uint8_t v) {
    bool carry = cpu->P & M6502_CF;
    cpu->P &= ~(M6502_NF|M6502_ZF|M6502_CF);
    if (v & 0x80) {
        cpu->P |= M6502_CF;
    }
    v <<= 1;
    if (carry) {
        v |= 1;
    }
    cpu->P = _M6502_NZ(cpu->P, v);
    return v;
}

static inline uint8_t _m6502_ror(m6502_t* cpu, uint8_t v) {
    bool carry = cpu->P & M6502_CF;
    cpu->P &= ~(M6502_NF|M6502_ZF|M6502_CF);
    if (v & 1) {
        cpu->P |= M6502_CF;
    }
    v >>= 1;
    if (carry) {
        v |= 0x80;
    }
    cpu->P = _M6502_NZ(cpu->P, v);
    return v;
}

static inline void _m6502_bit(m6502_t* cpu, uint8_t v) {
    uint8_t t = cpu->A & v;
    cpu->P &= ~(M6502_NF|M6502_VF|M6502_ZF);
    if (!t) {
        cpu->P |= M6502_ZF;
    }
    cpu->P |= v & (M6502_NF|M6502_VF);
}

static inline void _m6502_arr(m6502_t* cpu) {
    /* undocumented, unreliable ARR instruction, but this is tested
       by the Wolfgang Lorenz C64 test suite
       implementation taken from MAME
    */
    if (cpu->bcd_enabled && (cpu->P & M6502_DF)) {
        bool c = cpu->P & M6502_CF;
        cpu->P &= ~(M6502_NF|M6502_VF|M6502_ZF|M6502_CF);
        uint8_t a = cpu->A>>1;
        if (c) {
            a |= 0x80;
        }
        cpu->P = _M6502_NZ(cpu->P,a);
        if ((a ^ cpu->A) & 0x40) {
            cpu->P |= M6502_VF;
        }
        if ((cpu->A & 0xF) >= 5) {
            a = ((a + 6) & 0xF) | (a & 0xF0);
        }
        if ((cpu->A & 0xF0) >= 0x50) {
            a += 0x60;
            cpu->P |= M6502_CF;
        }
        cpu->A = a;
    }
    else {
        bool c = cpu->P & M6502_CF;
        cpu->P &= ~(M6502_NF|M6502_VF|M6502_ZF|M6502_CF);
        cpu->A >>= 1;
        if (c) {
            cpu->A |= 0x80;
        }
        cpu->P = _M6502_NZ(cpu->P,cpu->A);
        if (cpu->A & 0x40) {
            cpu->P |= M6502_VF|M6502_CF;
        }
        if (cpu->A & 0x20) {
            cpu->P ^= M6502_VF;
        }
    }
}

/* undocumented SBX instruction: 
    AND X register with accumulator and store result in X register, then
    subtract byte from X register (without borrow) where the
    subtract works like a CMP instruction
*/
static inline void _m6502_sbx(m6502_t* cpu, uint8_t v) {
    uint16_t t = (cpu->A & cpu->X) - v;
    cpu->P = _M6502_NZ(cpu->P, t) & ~M6502_CF;
    if (!(t & 0xFF00)) {
        cpu->P |= M6502_CF;
    }
    cpu->X = (uint8_t)t;
}
#undef _M6502_NZ

uint64_t m6502_init(m6502_t* c, const m6502_desc_t* desc) {
    CHIPS_ASSERT(c && desc);
    memset(c, 0, sizeof(*c));
    c->P = M6502_ZF;
    c->bcd_enabled = !desc->bcd_disabled;
    c->PINS = M6502_RW | M6502_SYNC | M6502_RES;
    c->in_cb = desc->m6510_in_cb;
    c->out_cb = desc->m6510_out_cb;
    c->user_data = desc->m6510_user_data;
    c->io_pullup = desc->m6510_io_pullup;
    c->io_floating = desc->m6510_io_floating;
    return c->PINS;
}

/* only call this when accessing address 0 or 1 (M6510_CHECK_IO(pins) evaluates to true) */
uint64_t m6510_iorq(m6502_t* c, uint64_t pins) {
    CHIPS_ASSERT(c->in_cb && c->out_cb);
    if ((pins & M6502_A0) == 0) {
        /* address 0: access to data direction register */
        if (pins & M6502_RW) {
            /* read IO direction bits */
            M6502_SET_DATA(pins, c->io_ddr);
        }
        else {
            /* write IO direction bits and update outside world */
            c->io_ddr = M6502_GET_DATA(pins);
            c->io_drive = (c->io_out & c->io_ddr) | (c->io_drive & ~c->io_ddr);
            c->out_cb((c->io_out & c->io_ddr) | (c->io_pullup & ~c->io_ddr), c->user_data);
            c->io_pins = (c->io_out & c->io_ddr) | (c->io_inp & ~c->io_ddr);
        }
    }
    else {
        /* address 1: perform I/O */
        if (pins & M6502_RW) {
            /* an input operation */
            c->io_inp = c->in_cb(c->user_data);
            uint8_t val = ((c->io_inp | (c->io_floating & c->io_drive)) & ~c->io_ddr) | (c->io_out & c->io_ddr);
            M6502_SET_DATA(pins, val);
        }
        else {
            /* an output operation */
            c->io_out = M6502_GET_DATA(pins);
            c->io_drive = (c->io_out & c->io_ddr) | (c->io_drive & ~c->io_ddr);
            c->out_cb((c->io_out & c->io_ddr) | (c->io_pullup & ~c->io_ddr), c->user_data);
        }
        c->io_pins = (c->io_out & c->io_ddr) | (c->io_inp & ~c->io_ddr);
    }
    return pins;
}

/* set 16-bit address in 64-bit pin mask */
#define _SA(addr) pins=(pins&~0xFFFF)|((addr)&0xFFFFULL)
/* extract 16-bit addess from pin mask */
#define _GA() ((uint16_t)(pins&0xFFFFULL))
/* set 16-bit address and 8-bit data in 64-bit pin mask */
#define _SAD(addr,data) pins=(pins&~0xFFFFFF)|((((data)&0xFF)<<16)&0xFF0000ULL)|((addr)&0xFFFFULL)
/* fetch next opcode byte */
#define _FETCH() _SA(c->PC);_ON(M6502_SYNC);
/* set 8-bit data in 64-bit pin mask */
#define _SD(data) pins=((pins&~0xFF0000ULL)|(((data&0xFF)<<16)&0xFF0000ULL))
/* extract 8-bit data from 64-bit pin mask */
#define _GD() ((uint8_t)((pins&0xFF0000ULL)>>16))
/* enable control pins */
#define _ON(m) pins|=(m)
/* disable control pins */
#define _OFF(m) pins&=~(m)
/* a memory read tick */
#define _RD() _ON(M6502_RW);
/* a memory write tick */
#define _WR() _OFF(M6502_RW);
/* set N and Z flags depending on value */
#define _NZ(v) c->P=((c->P&~(M6502_NF|M6502_ZF))|((v&0xFF)?(v&M6502_NF):M6502_ZF))

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4244)   /* conversion from 'uint16_t' to 'uint8_t', possible loss of data */
#endif

uint64_t m6502_tick(m6502_t* c, uint64_t pins) {
    if (pins & (M6502_SYNC|M6502_IRQ|M6502_NMI|M6502_RDY|M6502_RES)) {
        // interrupt detection also works in RDY phases, but only NMI is "sticky"
        
        // NMI is edge-triggered
        if (0 != ((pins & (pins ^ c->PINS)) & M6502_NMI)) {
            c->nmi_pip |= 1;
        }
        // IRQ test is level triggered
        if ((pins & M6502_IRQ) && (0 == (c->P & M6502_IF))) {
            c->irq_pip |= 1;
        }
        
        // RDY pin is only checked during read cycles
        if ((pins & (M6502_RW|M6502_RDY)) == (M6502_RW|M6502_RDY)) {
            M6510_SET_PORT(pins, c->io_pins);
            c->PINS = pins;
            c->irq_pip <<= 1;
            return pins;
        }
        if (pins & M6502_SYNC) {
            // load new instruction into 'instruction register' and restart tick counter
            c->IR = _GD()<<3;
            _OFF(M6502_SYNC);
            
            // check IRQ, NMI and RES state
            //  - IRQ is level-triggered and must be active in the full cycle
            //    before SYNC
            //  - NMI is edge-triggered, and the change must have happened in
            //    any cycle before SYNC
            //  - RES behaves slightly different than on a real 6502, we go
            //    into RES state as soon as the pin goes active, from there
            //    on, behaviour is 'standard'
            if (0 != (c->irq_pip & 4)) {
                c->brk_flags |= M6502_BRK_IRQ;
            }
            if (0 != (c->nmi_pip & 0xFFFC)) {
                c->brk_flags |= M6502_BRK_NMI;
            }
            if (0 != (pins & M6502_RES)) {
                c->brk_flags |= M6502_BRK_RESET;
                c->io_ddr = 0;
                c->io_out = 0;
                c->io_inp = 0;
                c->io_pins = 0;
            }
            c->irq_pip &= 3;
            c->nmi_pip &= 3;

            // if interrupt or reset was requested, force a BRK instruction
            if (c->brk_flags) {
                c->IR = 0;
                c->P &= ~M6502_BF;
                pins &= ~M6502_RES;
            }
            else {
                c->PC++;
            }
        }
    }
    // reads are default, writes are special
    _RD();
    switch (c->IR++) {
$decode_block
    }
    M6510_SET_PORT(pins, c->io_pins);
    c->PINS = pins;
    c->irq_pip <<= 1;
    c->nmi_pip <<= 1;
    return pins;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#undef _SA
#undef _SAD
#undef _FETCH
#undef _SD
#undef _GD
#undef _ON
#undef _OFF
#undef _RD
#undef _WR
#undef _NZ
#endif /* CHIPS_IMPL */
