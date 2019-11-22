#pragma once
/*#
    # m6502x.h

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

    ## Notes

    Stored here for later referencer:

    - https://www.pagetable.com/?p=39
    
    Maybe it makes sense to rewrite the code-generation python script with
    a real 'decode' ROM?

    ## Functions
    ~~~C
    void m6502_init(m6502_t* cpu, const m6502_desc_t* desc)
    ~~~
        Initialize a m6502_t instance, the desc structure provides initialization
        attributes:
            ~~~C
            typedef struct {
                m6502_tick_t tick_cb;       // the CPU tick callback
                bool bcd_disabled;          // set to true if BCD mode is disabled
                m6510_in_t in_cb;           // optional port IO input callback (only on m6510)
                m6510_out_t out_cb;         // optional port IO output callback (only on m6510)
                uint8_t m6510_io_pullup;    // IO port bits that are 1 when reading
                uint8_t m6510_io_floating;  // unconnected IO port pins
                void* user_data;            // optional user-data for callbacks
            } m6502_desc_t;
            ~~~

        To emulate a m6510 you must provide port IO callbacks in _in_cb_ and _out_cb_,
        and should initialize the m6510_io_pullup and m6510_io_floating members.

    ~~~C
    void m6502_reset(m6502_t* cpu)
    ~~~
        Reset the m6502 instance.

    ~~~C
    uint32_t m6502_exec(m6502_t* cpu, uint32_t ticks)
    ~~~
        Execute instructions until the requested number of _ticks_ is reached,
        or a trap has been hit. Return number of executed cycles. To check if a trap
        has been hit, test the m6502_t.trap_id member on >= 0. 
        During execution the tick callback will be called for each clock cycle
        with the current CPU pin bitmask. The tick callback function must inspect
        the pin bitmask, perform memory requests and if necessary update the
        data bus pins. Finally the tick callback returns the (optionally
        modified) pin bitmask. If the tick callback sets the RDY pin (1),
        and the current tick is a read-access, the CPU will loop until the
        RDY pin is (0). The RDY pin is automatically cleared before the
        tick function is called for a read-access.

    ~~~C
    uint64_t m6510_iorq(m6502_t* cpu, uint64_t pins)
    ~~~
        For the m6510, call this function from inside the tick callback when the
        CPU wants to access the special memory location 0 and 1 (these are mapped
        to the IO port control registers of the m6510). m6510_iorq() may call the
        input/output callback functions provided in m6510_init().

    ~~~C
    void m6502_trap_cb(m6502_t* cpu, m6502_trap_t trap_cb)
    ~~~
        Set an optional trap callback. If this is set it will be invoked
        at the end of an instruction with the current PC (which points
        to the start of the next instruction). The trap callback should
        return a non-zero value if the execution loop should exit. The
        returned value will also be written to m6502_t.trap_id.
        Set a null ptr as trap callback disables the trap checking.
        To get the current trap callback, simply access m6502_t.trap_cb directly.

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

/* address lines */
#define M6502X_A0  (1ULL<<0)
#define M6502X_A1  (1ULL<<1)
#define M6502X_A2  (1ULL<<2)
#define M6502X_A3  (1ULL<<3)
#define M6502X_A4  (1ULL<<4)
#define M6502X_A5  (1ULL<<5)
#define M6502X_A6  (1ULL<<6)
#define M6502X_A7  (1ULL<<7)
#define M6502X_A8  (1ULL<<8)
#define M6502X_A9  (1ULL<<9)
#define M6502X_A10 (1ULL<<10)
#define M6502X_A11 (1ULL<<11)
#define M6502X_A12 (1ULL<<12)
#define M6502X_A13 (1ULL<<13)
#define M6502X_A14 (1ULL<<14)
#define M6502X_A15 (1ULL<<15)

/*--- data lines ------*/
#define M6502X_D0  (1ULL<<16)
#define M6502X_D1  (1ULL<<17)
#define M6502X_D2  (1ULL<<18)
#define M6502X_D3  (1ULL<<19)
#define M6502X_D4  (1ULL<<20)
#define M6502X_D5  (1ULL<<21)
#define M6502X_D6  (1ULL<<22)
#define M6502X_D7  (1ULL<<23)

/*--- control pins ---*/
#define M6502X_RW    (1ULL<<24)
#define M6502X_SYNC  (1ULL<<25)
#define M6502X_IRQ   (1ULL<<26)
#define M6502X_NMI   (1ULL<<27)
#define M6502X_RDY   (1ULL<<28)
#define M6502X_RES   (1ULL<<29)
#define M6510X_AEC   (1ULL<<30)

/*--- m6510 specific port pins ---*/
#define M6510X_P0    (1ULL<<32)
#define M6510X_P1    (1ULL<<33)
#define M6510X_P2    (1ULL<<34)
#define M6510X_P3    (1ULL<<35)
#define M6510X_P4    (1ULL<<36)
#define M6510X_P5    (1ULL<<37)
#define M6510X_PORT_BITS (M6510X_P0|M6510X_P1|M6510X_P2|M6510X_P3|M6510X_P4|M6510X_P5)

/* bit mask for all CPU pins (up to bit pos 40) */
#define M6502X_PIN_MASK ((1ULL<<40)-1)

/*--- status indicator flags ---*/
#define M6502X_CF (1<<0)   /* carry */
#define M6502X_ZF (1<<1)   /* zero */
#define M6502X_IF (1<<2)   /* IRQ disable */
#define M6502X_DF (1<<3)   /* decimal mode */
#define M6502X_BF (1<<4)   /* BRK command */
#define M6502X_XF (1<<5)   /* unused */
#define M6502X_VF (1<<6)   /* overflow */
#define M6502X_NF (1<<7)   /* negative */

/* the desc structure provided to m6502_init() */
typedef struct {
    bool bcd_disabled;      /* set to true if BCD mode is disabled */
} m6502x_desc_t;

/* M6502 CPU state */
typedef struct {
    uint16_t IR;
    uint16_t PC;
    uint16_t AD;        /* ADL/ADH internal register */
    uint8_t A,X,Y,S,P;
    uint64_t PINS;
    uint8_t irq_pip;
    uint8_t brk_irq;
    uint8_t bcd_enabled;
} m6502x_t;

/* initialize a new m6502 instance */
void m6502x_init(m6502x_t* cpu, const m6502x_desc_t* desc);
/* execute one tick */
uint64_t m6502x_tick(m6502x_t* cpu, uint64_t pins);

/* register access functions */
void m6502x_set_a(m6502x_t* cpu, uint8_t v);
void m6502x_set_x(m6502x_t* cpu, uint8_t v);
void m6502x_set_y(m6502x_t* cpu, uint8_t v);
void m6502x_set_s(m6502x_t* cpu, uint8_t v);
void m6502x_set_p(m6502x_t* cpu, uint8_t v);
void m6502x_set_pc(m6502x_t* cpu, uint16_t v);

uint8_t m6502x_a(m6502x_t* cpu);
uint8_t m6502x_x(m6502x_t* cpu);
uint8_t m6502x_y(m6502x_t* cpu);
uint8_t m6502x_s(m6502x_t* cpu);
uint8_t m6502x_p(m6502x_t* cpu);
uint16_t m6502x_pc(m6502x_t* cpu);

/* extract 16-bit address bus from 64-bit pins */
#define M6502X_GET_ADDR(p) ((uint16_t)(p&0xFFFFULL))
/* merge 16-bit address bus value into 64-bit pins */
#define M6502X_SET_ADDR(p,a) {p=((p&~0xFFFFULL)|((a)&0xFFFFULL));}
/* extract 8-bit data bus from 64-bit pins */
#define M6502X_GET_DATA(p) ((uint8_t)((p&0xFF0000ULL)>>16))
/* merge 8-bit data bus value into 64-bit pins */
#define M6502X_SET_DATA(p,d) {p=(((p)&~0xFF0000ULL)|(((d)<<16)&0xFF0000ULL));}
/* return a pin mask with control-pins, address and data bus */
#define M6502X_MAKE_PINS(ctrl, addr, data) ((ctrl)|(((data)<<16)&0xFF0000ULL)|((addr)&0xFFFFULL))

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
void m6502x_set_a(m6502x_t* cpu, uint8_t v) { cpu->A = v; }
void m6502x_set_x(m6502x_t* cpu, uint8_t v) { cpu->X = v; }
void m6502x_set_y(m6502x_t* cpu, uint8_t v) { cpu->Y = v; }
void m6502x_set_s(m6502x_t* cpu, uint8_t v) { cpu->S = v; }
void m6502x_set_p(m6502x_t* cpu, uint8_t v) { cpu->P = v; }
void m6502x_set_pc(m6502x_t* cpu, uint16_t v) { cpu->PC = v; }
uint8_t m6502x_a(m6502x_t* cpu) { return cpu->A; }
uint8_t m6502x_x(m6502x_t* cpu) { return cpu->X; }
uint8_t m6502x_y(m6502x_t* cpu) { return cpu->Y; }
uint8_t m6502x_s(m6502x_t* cpu) { return cpu->S; }
uint8_t m6502x_p(m6502x_t* cpu) { return cpu->P; }
uint16_t m6502x_pc(m6502x_t* cpu) { return cpu->PC; }

/* helper macros and functions for code-generated instruction decoder */
#define _M6502X_NZ(p,v) ((p&~(M6502X_NF|M6502X_ZF))|((v&0xFF)?(v&M6502X_NF):M6502X_ZF))

static inline void _m6502x_adc(m6502x_t* cpu, uint8_t val) {
    if (cpu->bcd_enabled && (cpu->P & M6502X_DF)) {
        /* decimal mode (credit goes to MAME) */
        uint8_t c = cpu->P & M6502X_CF ? 1 : 0;
        cpu->P &= ~(M6502X_NF|M6502X_VF|M6502X_ZF|M6502X_CF);
        uint8_t al = (cpu->A & 0x0F) + (val & 0x0F) + c;
        if (al > 9) {
            al += 6;
        }
        uint8_t ah = (cpu->A >> 4) + (val >> 4) + (al > 0x0F);
        if (0 == (uint8_t)(cpu->A + val + c)) {
            cpu->P |= M6502X_ZF;
        }
        else if (ah & 0x08) {
            cpu->P |= M6502X_NF;
        }
        if (~(cpu->A^val) & (cpu->A^(ah<<4)) & 0x80) {
            cpu->P |= M6502X_VF;
        }
        if (ah > 9) {
            ah += 6;
        }
        if (ah > 15) {
            cpu->P |= M6502X_CF;
        }
        cpu->A = (ah<<4) | (al & 0x0F);
    }
    else {
        /* default mode */
        uint16_t sum = cpu->A + val + (cpu->P & M6502X_CF ? 1:0);
        cpu->P &= ~(M6502X_VF|M6502X_CF);
        cpu->P = _M6502X_NZ(cpu->P,sum);
        if (~(cpu->A^val) & (cpu->A^sum) & 0x80) {
            cpu->P |= M6502X_VF;
        }
        if (sum & 0xFF00) {
            cpu->P |= M6502X_CF;
        }
        cpu->A = sum & 0xFF;
    }    
}

static inline void _m6502x_sbc(m6502x_t* cpu, uint8_t val) {
    if (cpu->bcd_enabled && (cpu->P & M6502X_DF)) {
        /* decimal mode (credit goes to MAME) */
        uint8_t c = cpu->P & M6502X_CF ? 0 : 1;
        cpu->P &= ~(M6502X_NF|M6502X_VF|M6502X_ZF|M6502X_CF);
        uint16_t diff = cpu->A - val - c;
        uint8_t al = (cpu->A & 0x0F) - (val & 0x0F) - c;
        if ((int8_t)al < 0) {
            al -= 6;
        }
        uint8_t ah = (cpu->A>>4) - (val>>4) - ((int8_t)al < 0);
        if (0 == (uint8_t)diff) {
            cpu->P |= M6502X_ZF;
        }
        else if (diff & 0x80) {
            cpu->P |= M6502X_NF;
        }
        if ((cpu->A^val) & (cpu->A^diff) & 0x80) {
            cpu->P |= M6502X_VF;
        }
        if (!(diff & 0xFF00)) {
            cpu->P |= M6502X_CF;
        }
        if (ah & 0x80) {
            ah -= 6;
        }
        cpu->A = (ah<<4) | (al & 0x0F);
    }
    else {
        /* default mode */
        uint16_t diff = cpu->A - val - (cpu->P & M6502X_CF ? 0 : 1);
        cpu->P &= ~(M6502X_VF|M6502X_CF);
        cpu->P = _M6502X_NZ(cpu->P, (uint8_t)diff);
        if ((cpu->A^val) & (cpu->A^diff) & 0x80) {
            cpu->P |= M6502X_VF;
        }
        if (!(diff & 0xFF00)) {
            cpu->P |= M6502X_CF;
        }
        cpu->A = diff & 0xFF;
    }
}

static inline void _m6502x_cmp(m6502x_t* cpu, uint8_t r, uint8_t v) {
    uint16_t t = r - v;
    cpu->P = (_M6502X_NZ(cpu->P, (uint8_t)t) & ~M6502X_CF) | ((t & 0xFF00) ? 0:M6502X_CF);
}

static inline uint8_t _m6502x_asl(m6502x_t* cpu, uint8_t v) {
    cpu->P = (_M6502X_NZ(cpu->P, v<<1) & ~M6502X_CF) | ((v & 0x80) ? M6502X_CF:0);
    return v<<1;
}

static inline uint8_t _m6502x_lsr(m6502x_t* cpu, uint8_t v) {
    cpu->P = (_M6502X_NZ(cpu->P, v>>1) & ~M6502X_CF) | ((v & 0x01) ? M6502X_CF:0);
    return v>>1;
}

static inline uint8_t _m6502x_rol(m6502x_t* cpu, uint8_t v) {
    bool carry = cpu->P & M6502X_CF;
    cpu->P &= ~(M6502X_NF|M6502X_ZF|M6502X_CF);
    if (v & 0x80) {
        cpu->P |= M6502X_CF;
    }
    v <<= 1;
    if (carry) {
        v |= 1;
    }
    cpu->P = _M6502X_NZ(cpu->P, v);
    return v;
}

static inline uint8_t _m6502x_ror(m6502x_t* cpu, uint8_t v) {
    bool carry = cpu->P & M6502X_CF;
    cpu->P &= ~(M6502X_NF|M6502X_ZF|M6502X_CF);
    if (v & 1) {
        cpu->P |= M6502X_CF;
    }
    v >>= 1;
    if (carry) {
        v |= 0x80;
    }
    cpu->P = _M6502X_NZ(cpu->P, v);
    return v;
}

static inline void _m6502x_bit(m6502x_t* cpu, uint8_t v) {
    uint8_t t = cpu->A & v;
    cpu->P &= ~(M6502X_NF|M6502X_VF|M6502X_ZF);
    if (!t) {
        cpu->P |= M6502X_ZF;
    }
    cpu->P |= t & (M6502X_NF|M6502X_VF);
}

static inline void _m6502x_arr(m6502x_t* cpu) {
    /* undocumented, unreliable ARR instruction, but this is tested
       by the Wolfgang Lorenz C64 test suite
       implementation taken from MAME
    */
    if (cpu->bcd_enabled && (cpu->P & M6502X_DF)) {
        bool c = cpu->P & M6502X_CF;
        cpu->P &= ~(M6502X_NF|M6502X_VF|M6502X_ZF|M6502X_CF);
        uint8_t a = cpu->A>>1;
        if (c) {
            a |= 0x80;
        }
        cpu->P = _M6502X_NZ(cpu->P,a);
        if ((a ^ cpu->A) & 0x40) {
            cpu->P |= M6502X_VF;
        }
        if ((cpu->A & 0xF) >= 5) {
            a = ((a + 6) & 0xF) | (a & 0xF0);
        }
        if ((cpu->A & 0xF0) >= 0x50) {
            a += 0x60;
            cpu->P |= M6502X_CF;
        }
        cpu->A = a;
    }
    else {
        bool c = cpu->P & M6502X_CF;
        cpu->P &= ~(M6502X_NF|M6502X_VF|M6502X_ZF|M6502X_CF);
        cpu->A >>= 1;
        if (c) {
            cpu->A |= 0x80;
        }
        cpu->P = _M6502X_NZ(cpu->P,cpu->A);
        if (cpu->A & 0x40) {
            cpu->P |= M6502X_VF|M6502X_CF;
        }
        if (cpu->A & 0x20) {
            cpu->P ^= M6502X_VF;
        }
    }
}

/* undocumented SBX instruction: 
    AND X register with accumulator and store result in X register, then
    subtract byte from X register (without borrow) where the
    subtract works like a CMP instruction
*/
static inline void _m6502x_sbx(m6502x_t* cpu, uint8_t v) {
    uint16_t t = (cpu->A & cpu->X) - v;
    cpu->P = _M6502X_NZ(cpu->P, t) & ~M6502X_CF;
    if (!(t & 0xFF00)) {
        cpu->P |= M6502X_CF;
    }
    cpu->X = (uint8_t)t;
}
#undef _M6502X_NZ

void m6502x_init(m6502x_t* c, const m6502x_desc_t* desc) {
    CHIPS_ASSERT(c && desc);
    memset(c, 0, sizeof(*c));
    c->PINS = M6502X_RW;
    c->P = M6502X_BF|M6502X_IF|M6502X_XF|M6502X_ZF;
    c->S = 0xBD;
    c->bcd_enabled = !desc->bcd_disabled;
}

/* set 16-bit address in 64-bit pin mask */
#define _SA(addr) pins=(pins&~0xFFFF)|((addr)&0xFFFFULL)
/* extract 16-bit addess from pin mask */
#define _GA() ((uint16_t)(pins&0xFFFFULL))
/* set 16-bit address and 8-bit data in 64-bit pin mask */
#define _SAD(addr,data) pins=(pins&~0xFFFFFF)|((((data)&0xFF)<<16)&0xFF0000ULL)|((addr)&0xFFFFULL)
/* fetch next opcode byte */
#define _FETCH() _SA(c->PC++);_ON(M6502X_SYNC);
/* set 8-bit data in 64-bit pin mask */
#define _SD(data) pins=((pins&~0xFF0000ULL)|(((data&0xFF)<<16)&0xFF0000ULL))
/* extract 8-bit data from 64-bit pin mask */
#define _GD() ((uint8_t)((pins&0xFF0000ULL)>>16))
/* enable control pins */
#define _ON(m) pins|=(m)
/* disable control pins */
#define _OFF(m) pins&=~(m)
/* a memory read tick */
#define _RD() _ON(M6502X_RW);
/* a memory write tick */
#define _WR() _OFF(M6502X_RW);
/* set N and Z flags depending on value */
#define _NZ(v) c->P=((c->P&~(M6502X_NF|M6502X_ZF))|((v&0xFF)?(v&M6502X_NF):M6502X_ZF))

uint64_t m6502x_tick(m6502x_t* c, uint64_t pins) {
    c->irq_pip <<= 1;
    if (pins & (M6502X_SYNC|M6502X_IRQ|M6502X_NMI|M6502X_RDY|M6502X_RES)) {
        if (pins & M6502X_SYNC) {
            // load new instruction into 'instruction register' and restart tick counter
            c->IR = _GD()<<3;
            _OFF(M6502X_SYNC);
            // FIXME: check for reset, this is a bit non-trivial, since setting
            // the RESET pin to active starts a BRK instruction each tick(?)
            // until the RESET pin goes inactive, only then the RESET is
            // allowed to execute

            // if interrupt was requested, force a BRK instruction
            if ((c->irq_pip & 4) && (0 == (c->P & M6502X_IF))) {
                c->IR = 0;
                c->brk_irq = true;
                c->PC--;
            }
            else {
                c->brk_irq = false;
            }
        }
        // check for interrupt
        if (pins & M6502X_IRQ) {
            c->irq_pip |= 1;
        }
    }
    // reads are default, writes a special
    _RD();
    switch (c->IR++) {
$decode_block
    }
    c->PINS = pins;
    return pins;
}
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
