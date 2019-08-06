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
#define M6502_A0  (1ULL<<0)
#define M6502_A1  (1ULL<<1)
#define M6502_A2  (1ULL<<2)
#define M6502_A3  (1ULL<<3)
#define M6502_A4  (1ULL<<4)
#define M6502_A5  (1ULL<<5)
#define M6502_A6  (1ULL<<6)
#define M6502_A7  (1ULL<<7)
#define M6502_A8  (1ULL<<8)
#define M6502_A9  (1ULL<<9)
#define M6502_A10 (1ULL<<10)
#define M6502_A11 (1ULL<<11)
#define M6502_A12 (1ULL<<12)
#define M6502_A13 (1ULL<<13)
#define M6502_A14 (1ULL<<14)
#define M6502_A15 (1ULL<<15)

/*--- data lines ------*/
#define M6502_D0  (1ULL<<16)
#define M6502_D1  (1ULL<<17)
#define M6502_D2  (1ULL<<18)
#define M6502_D3  (1ULL<<19)
#define M6502_D4  (1ULL<<20)
#define M6502_D5  (1ULL<<21)
#define M6502_D6  (1ULL<<22)
#define M6502_D7  (1ULL<<23)

/*--- control pins ---*/
#define M6502_RW    (1ULL<<24)
#define M6502_SYNC  (1ULL<<25)
#define M6502_IRQ   (1ULL<<26)
#define M6502_NMI   (1ULL<<27)
#define M6502_RDY   (1ULL<<28)
#define M6510_AEC   (1ULL<<29)

/*--- m6510 specific port pins ---*/
#define M6510_P0    (1ULL<<32)
#define M6510_P1    (1ULL<<33)
#define M6510_P2    (1ULL<<34)
#define M6510_P3    (1ULL<<35)
#define M6510_P4    (1ULL<<36)
#define M6510_P5    (1ULL<<37)
#define M6510_PORT_BITS (M6510_P0|M6510_P1|M6510_P2|M6510_P3|M6510_P4|M6510_P5)

/* bit mask for all CPU pins (up to bit pos 40) */
#define M6502_PIN_MASK ((1ULL<<40)-1)

/*--- status indicator flags ---*/
#define M6502_CF (1<<0)   /* carry */
#define M6502_ZF (1<<1)   /* zero */
#define M6502_IF (1<<2)   /* IRQ disable */
#define M6502_DF (1<<3)   /* decimal mode */
#define M6502_BF (1<<4)   /* BRK command */
#define M6502_XF (1<<5)   /* unused */
#define M6502_VF (1<<6)   /* overflow */
#define M6502_NF (1<<7)   /* negative */

/* callback function typedefs */
typedef uint64_t (*m6502_tick_t)(uint64_t pins, void* user_data);
typedef int (*m6502_trap_t)(uint16_t pc, int ticks, uint64_t pins, void* user_data);
typedef void (*m6510_out_t)(uint8_t data, void* user_data);
typedef uint8_t (*m6510_in_t)(void* user_data);

/* the desc structure provided to m6502_init() */
typedef struct {
    m6502_tick_t tick_cb;   /* the CPU tick callback */
    void* user_data;        /* optional callback user data */
    bool bcd_disabled;      /* set to true if BCD mode is disabled */
    m6510_in_t in_cb;       /* optional port IO input callback (only on m6510) */
    m6510_out_t out_cb;     /* optional port IO output callback (only on m6510) */
    uint8_t m6510_io_pullup;    /* IO port bits that are 1 when reading */
    uint8_t m6510_io_floating;  /* unconnected IO port pins */
} m6502_desc_t;

/* mutable tick state */
typedef struct {
    uint64_t PINS;
    uint8_t A,X,Y,S,P;      /* 8-bit registers */
    uint16_t PC;            /* 16-bit program counter */
    /* state of interrupt enable flag at the time when the IRQ pin is sampled,
       this is used to implement 'delayed IRQ response'
       (see: https://wiki.nesdev.com/w/index.php/CPU_interrupts)
    */
    uint8_t pi;
    bool bcd_enabled;       /* this is actually not mutable but needed when ticking */
} m6502_state_t;

/* M6502 CPU state */
typedef struct {
    m6502_state_t state;
    m6502_tick_t tick_cb;
    m6502_trap_t trap_cb;
    void* user_data;
    void* trap_user_data;
    int trap_id;        /* index of trap hit (-1 if no trap) */

    /* the m6510 IO port stuff */
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

/* initialize a new m6502 instance */
void m6502_init(m6502_t* cpu, const m6502_desc_t* desc);
/* reset an existing m6502 instance */
void m6502_reset(m6502_t* cpu);
/* set a trap callback function */
void m6502_trap_cb(m6502_t* cpu, m6502_trap_t trap_cb, void* trap_user_data);
/* execute instruction for at least 'ticks' or trap hit, return number of executed ticks */
uint32_t m6502_exec(m6502_t* cpu, uint32_t ticks);
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
void m6502_set_a(m6502_t* cpu, uint8_t v) { cpu->state.A = v; }
void m6502_set_x(m6502_t* cpu, uint8_t v) { cpu->state.X = v; }
void m6502_set_y(m6502_t* cpu, uint8_t v) { cpu->state.Y = v; }
void m6502_set_s(m6502_t* cpu, uint8_t v) { cpu->state.S = v; }
void m6502_set_p(m6502_t* cpu, uint8_t v) { cpu->state.P = v; }
void m6502_set_pc(m6502_t* cpu, uint16_t v) { cpu->state.PC = v; }
uint8_t m6502_a(m6502_t* cpu) { return cpu->state.A; }
uint8_t m6502_x(m6502_t* cpu) { return cpu->state.X; }
uint8_t m6502_y(m6502_t* cpu) { return cpu->state.Y; }
uint8_t m6502_s(m6502_t* cpu) { return cpu->state.S; }
uint8_t m6502_p(m6502_t* cpu) { return cpu->state.P; }
uint16_t m6502_pc(m6502_t* cpu) { return cpu->state.PC; }

/* helper macros and functions for code-generated instruction decoder */
#define _M6502_NZ(p,v) ((p&~(M6502_NF|M6502_ZF))|((v&0xFF)?(v&M6502_NF):M6502_ZF))

static inline void _m6502_adc(m6502_state_t* cpu, uint8_t val) {
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

static inline void _m6502_sbc(m6502_state_t* cpu, uint8_t val) {
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

static inline void _m6502_arr(m6502_state_t* cpu) {
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
#undef _M6502_NZ

void m6502_init(m6502_t* c, const m6502_desc_t* desc) {
    CHIPS_ASSERT(c && desc);
    CHIPS_ASSERT(desc->tick_cb);
    memset(c, 0, sizeof(*c));
    c->tick_cb = desc->tick_cb;
    c->user_data = desc->user_data;
    c->state.PINS = M6502_RW;
    c->state.P = M6502_IF|M6502_XF;
    c->state.S = 0xFD;
    c->state.bcd_enabled = !desc->bcd_disabled;
    c->in_cb = desc->in_cb;
    c->out_cb = desc->out_cb;
    c->io_pullup = desc->m6510_io_pullup;
    c->io_floating = desc->m6510_io_floating;
    c->trap_id = -1;
}

void m6502_reset(m6502_t* c) {
    CHIPS_ASSERT(c);
    c->state.P = M6502_IF|M6502_XF;
    c->state.S = 0xFD;
    c->state.PINS = M6502_RW;
    /* load reset vector from 0xFFFD into PC */
    uint8_t l = M6502_GET_DATA(c->tick_cb(M6502_MAKE_PINS(M6502_RW, 0xFFFC, 0x00), c->user_data));
    uint8_t h = M6502_GET_DATA(c->tick_cb(M6502_MAKE_PINS(M6502_RW, 0xFFFD, 0x00), c->user_data));
    c->state.PC = (h<<8)|l;
    c->io_ddr = 0;
    c->io_out = 0;
    c->io_inp = 0;
    c->io_pins = 0;
}

void m6502_trap_cb(m6502_t* c, m6502_trap_t trap_cb, void* trap_user_data) {
    CHIPS_ASSERT(c);
    c->trap_cb = trap_cb;
    c->trap_user_data = trap_user_data;
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

/* set 16-bit address in 64-bit pin mask*/
#define _SA(addr) pins=(pins&~0xFFFF)|((addr)&0xFFFFULL)
/* set 16-bit address and 8-bit data in 64-bit pin mask */
#define _SAD(addr,data) pins=(pins&~0xFFFFFF)|((((data)&0xFF)<<16)&0xFF0000ULL)|((addr)&0xFFFFULL)
/* set 8-bit data in 64-bit pin mask */
#define _SD(data) pins=((pins&~0xFF0000ULL)|(((data&0xFF)<<16)&0xFF0000ULL))
/* extract 8-bit data from 64-bit pin mask */
#define _GD() ((uint8_t)((pins&0xFF0000ULL)>>16))
/* enable control pins */
#define _ON(m) pins|=(m)
/* disable control pins */
#define _OFF(m) pins&=~(m)
/* execute a tick */
#define _T() pins=tick(pins,ud);ticks++;
/* a memory read tick */
#define _RD() _ON(M6502_RW);do{_OFF(M6502_RDY);_T();}while(pins&M6502_RDY);
/* a memory write tick */
#define _WR() _OFF(M6502_RW);_T()
/* implied addressing mode, this still puts the PC on the address bus */
#define _A_IMP() _SA(c.PC)
/* immediate addressing mode */
#define _A_IMM() _SA(c.PC++)
/* zero-page addressing mode */
#define _A_ZER() _SA(c.PC++);_RD();a=_GD();_SA(a)
/* zero page + X addressing mode */
#define _A_ZPX() _SA(c.PC++);_RD();a=_GD();_SA(a);_RD();a=(a+c.X)&0x00FF;_SA(a)
/* zero page + Y addressing mode */
#define _A_ZPY() _SA(c.PC++);_RD();a=_GD();_SA(a);_RD();a=(a+c.Y)&0x00FF;_SA(a)
/* absolute addressing mode */
#define _A_ABS() _SA(c.PC++);_RD();l=_GD();_SA(c.PC++);_RD();h=_GD();a=(h<<8)|l;_SA(a)
/* absolute+X addressing mode for read-only instructions, early out if no page boundary is crossed */
#define _A_ABX_R() _SA(c.PC++);_RD();t=_GD()+c.X;_SA(c.PC++);_RD();a=(_GD()<<8)|(t&0xFF);_SA(a);if((t&0xFF00)!=0){_RD();a=(a&0xFF00)+t;_SA(a);}
/* absolute+X addressing mode for read/write instructions */
#define _A_ABX_W() _SA(c.PC++);_RD();t=_GD()+c.X;_SA(c.PC++);_RD();a=(_GD()<<8)|(t&0xFF);_SA(a);_RD();a=(a&0xFF00)+t;_SA(a)
/* absolute+Y addressing mode for read-only instructions, early out if no page boundary is crossed */
#define _A_ABY_R() _SA(c.PC++);_RD();t=_GD()+c.Y;_SA(c.PC++);_RD();a=(_GD()<<8)|(t&0xFF);_SA(a);if((t&0xFF00)!=0){_RD();a=(a&0xFF00)+t;_SA(a);}
/* absolute+Y addressing mode for read/write instructions */
#define _A_ABY_W() _SA(c.PC++);_RD();t=_GD()+c.Y;_SA(c.PC++);_RD();a=(_GD()<<8)|(t&0xFF);_SA(a);_RD();a=(a&0xFF00)+t;_SA(a)
/* (zp,X) indexed indirect addressing mode */
#define _A_IDX() _SA(c.PC++);_RD();a=_GD();_SA(a);_RD();a=(a+c.X)&0xFF;_SA(a);_RD();t=_GD();a=(a+1)&0xFF;_SA(a);_RD();a=(_GD()<<8)|t;_SA(a);
/* (zp),Y indirect indexed addressing mode for read-only instructions, early out if no page boundary crossed */
#define _A_IDY_R() _SA(c.PC++);_RD();a=_GD();_SA(a);_RD();t=_GD()+c.Y;a=(a+1)&0xFF;_SA(a);_RD();a=(_GD()<<8)|(t&0xFF);_SA(a);if((t&0xFF00)!=0){_RD();a=(a&0xFF00)+t;_SA(a);}
/* (zp),Y indirect indexed addressing mode for read/write instructions */
#define _A_IDY_W() _SA(c.PC++);_RD();a=_GD();_SA(a);_RD();t=_GD()+c.Y;a=(a+1)&0xFF;_SA(a);_RD();a=(_GD()<<8)|(t&0xFF);_SA(a);_RD();a=(a&0xFF00)+t;_SA(a)
/* set N and Z flags depending on value */
#define _NZ(v) c.P=((c.P&~(M6502_NF|M6502_ZF))|((v&0xFF)?(v&M6502_NF):M6502_ZF))

uint32_t m6502_exec(m6502_t* cpu, uint32_t num_ticks) {
    cpu->trap_id = 0;
    m6502_state_t c = cpu->state;
    uint8_t l, h;
    uint16_t a, t;
    uint32_t ticks = 0;
    uint64_t pins = c.PINS;
    const m6502_tick_t tick = cpu->tick_cb;
    const m6502_trap_t trap = cpu->trap_cb;
    void* ud = cpu->user_data;
    do {
        uint64_t pre_pins = pins;
        _OFF(M6502_IRQ|M6502_NMI);
        /* fetch opcode */
        _SA(c.PC++);_ON(M6502_SYNC);_RD();_OFF(M6502_SYNC);
        /* store 'delayed IRQ response' flag state */
        c.pi = c.P;
        const uint8_t opcode = _GD();
        /* code-generated decode block */
        switch (opcode) {
            case 0x0:/*BRK */_A_IMP();_RD();c.PC++;_SAD(0x0100|c.S--,c.PC>>8);_WR();_SAD(0x0100|c.S--,c.PC);_WR();_SAD(0x0100|c.S--,c.P|M6502_BF);_WR();_SA(0xFFFE);_RD();l=_GD();_SA(0xFFFF);_RD();h=_GD();c.PC=(h<<8)|l;c.P|=M6502_IF;break;
            case 0x1:/*ORA (zp,X)*/_A_IDX();_RD();c.A|=_GD();_NZ(c.A);break;
            case 0x2:/*INVALID*/break;
            case 0x3:/*SLO (zp,X) (undoc)*/_A_IDX();_RD();_WR();l=_GD();c.P=(c.P&~M6502_CF)|((l&0x80)?M6502_CF:0);l<<=1;_NZ(l);_SD(l);_WR();c.A|=l;_NZ(c.A);break;
            case 0x4:/*NOP zp (undoc)*/_A_ZER();_RD();break;
            case 0x5:/*ORA zp*/_A_ZER();_RD();c.A|=_GD();_NZ(c.A);break;
            case 0x6:/*ASL zp*/_A_ZER();_RD();_WR();l=_GD();c.P=(c.P&~M6502_CF)|((l&0x80)?M6502_CF:0);l<<=1;_NZ(l);_SD(l);_WR();break;
            case 0x7:/*SLO zp (undoc)*/_A_ZER();_RD();_WR();l=_GD();c.P=(c.P&~M6502_CF)|((l&0x80)?M6502_CF:0);l<<=1;_NZ(l);_SD(l);_WR();c.A|=l;_NZ(c.A);break;
            case 0x8:/*PHP */_A_IMP();_RD();_SAD(0x0100|c.S--,c.P|M6502_BF);_WR();break;
            case 0x9:/*ORA #*/_A_IMM();_RD();c.A|=_GD();_NZ(c.A);break;
            case 0xa:/*ASLA */_A_IMP();_RD();c.P=(c.P&~M6502_CF)|((c.A&0x80)?M6502_CF:0);c.A<<=1;_NZ(c.A);break;
            case 0xb:/*ANC # (undoc)*/_A_IMM();_RD();c.A&=_GD();_NZ(c.A);if(c.A&0x80){c.P|=M6502_CF;}else{c.P&=~M6502_CF;}break;
            case 0xc:/*NOP abs (undoc)*/_A_ABS();_RD();break;
            case 0xd:/*ORA abs*/_A_ABS();_RD();c.A|=_GD();_NZ(c.A);break;
            case 0xe:/*ASL abs*/_A_ABS();_RD();_WR();l=_GD();c.P=(c.P&~M6502_CF)|((l&0x80)?M6502_CF:0);l<<=1;_NZ(l);_SD(l);_WR();break;
            case 0xf:/*SLO abs (undoc)*/_A_ABS();_RD();_WR();l=_GD();c.P=(c.P&~M6502_CF)|((l&0x80)?M6502_CF:0);l<<=1;_NZ(l);_SD(l);_WR();c.A|=l;_NZ(c.A);break;
            case 0x10:/*BPL #*/_A_IMM();_RD();if((c.P&0x80)==0x0){_RD();t=c.PC+(int8_t)_GD();if((t&0xFF00)!=(c.PC&0xFF00)){_RD();}c.PC=t;}break;
            case 0x11:/*ORA (zp),Y*/_A_IDY_R();_RD();c.A|=_GD();_NZ(c.A);break;
            case 0x12:/*INVALID*/break;
            case 0x13:/*SLO (zp),Y (undoc)*/_A_IDY_W();_RD();_WR();l=_GD();c.P=(c.P&~M6502_CF)|((l&0x80)?M6502_CF:0);l<<=1;_NZ(l);_SD(l);_WR();c.A|=l;_NZ(c.A);break;
            case 0x14:/*NOP zp,X (undoc)*/_A_ZPX();_RD();break;
            case 0x15:/*ORA zp,X*/_A_ZPX();_RD();c.A|=_GD();_NZ(c.A);break;
            case 0x16:/*ASL zp,X*/_A_ZPX();_RD();_WR();l=_GD();c.P=(c.P&~M6502_CF)|((l&0x80)?M6502_CF:0);l<<=1;_NZ(l);_SD(l);_WR();break;
            case 0x17:/*SLO zp,X (undoc)*/_A_ZPX();_RD();_WR();l=_GD();c.P=(c.P&~M6502_CF)|((l&0x80)?M6502_CF:0);l<<=1;_NZ(l);_SD(l);_WR();c.A|=l;_NZ(c.A);break;
            case 0x18:/*CLC */_A_IMP();_RD();c.P&=~0x1;break;
            case 0x19:/*ORA abs,Y*/_A_ABY_R();_RD();c.A|=_GD();_NZ(c.A);break;
            case 0x1a:/*NOP  (undoc)*/_A_IMP();_RD();break;
            case 0x1b:/*SLO abs,Y (undoc)*/_A_ABY_W();_RD();_WR();l=_GD();c.P=(c.P&~M6502_CF)|((l&0x80)?M6502_CF:0);l<<=1;_NZ(l);_SD(l);_WR();c.A|=l;_NZ(c.A);break;
            case 0x1c:/*NOP abs,X (undoc)*/_A_ABX_R();_RD();break;
            case 0x1d:/*ORA abs,X*/_A_ABX_R();_RD();c.A|=_GD();_NZ(c.A);break;
            case 0x1e:/*ASL abs,X*/_A_ABX_W();_RD();_WR();l=_GD();c.P=(c.P&~M6502_CF)|((l&0x80)?M6502_CF:0);l<<=1;_NZ(l);_SD(l);_WR();break;
            case 0x1f:/*SLO abs,X (undoc)*/_A_ABX_W();_RD();_WR();l=_GD();c.P=(c.P&~M6502_CF)|((l&0x80)?M6502_CF:0);l<<=1;_NZ(l);_SD(l);_WR();c.A|=l;_NZ(c.A);break;
            case 0x20:/*JSR */_SA(c.PC++);_RD();l=_GD();_SA(0x0100|c.S);_RD();_SAD(0x0100|c.S--,c.PC>>8);_WR();_SAD(0x0100|c.S--,c.PC);_WR();_SA(c.PC);_RD();h=_GD();c.PC=(h<<8)|l;break;
            case 0x21:/*AND (zp,X)*/_A_IDX();_RD();c.A&=_GD();_NZ(c.A);break;
            case 0x22:/*INVALID*/break;
            case 0x23:/*RLA (zp,X) (undoc)*/_A_IDX();_RD();_WR();l=_GD();{bool carry=c.P&M6502_CF;c.P&=~(M6502_NF|M6502_ZF|M6502_CF);if(l&0x80){c.P|=M6502_CF;}l<<=1;if(carry){l|=0x01;}_NZ(l);}_SD(l);_WR();c.A&=l;_NZ(c.A);break;
            case 0x24:/*BIT zp*/_A_ZER();_RD();l=_GD();h=c.A&l;c.P&=~(M6502_NF|M6502_VF|M6502_ZF);if(!h){c.P|=M6502_ZF;}c.P|=l&(M6502_NF|M6502_VF);break;
            case 0x25:/*AND zp*/_A_ZER();_RD();c.A&=_GD();_NZ(c.A);break;
            case 0x26:/*ROL zp*/_A_ZER();_RD();_WR();l=_GD();{bool carry=c.P&M6502_CF;c.P&=~(M6502_NF|M6502_ZF|M6502_CF);if(l&0x80){c.P|=M6502_CF;}l<<=1;if(carry){l|=0x01;}_NZ(l);}_SD(l);_WR();break;
            case 0x27:/*RLA zp (undoc)*/_A_ZER();_RD();_WR();l=_GD();{bool carry=c.P&M6502_CF;c.P&=~(M6502_NF|M6502_ZF|M6502_CF);if(l&0x80){c.P|=M6502_CF;}l<<=1;if(carry){l|=0x01;}_NZ(l);}_SD(l);_WR();c.A&=l;_NZ(c.A);break;
            case 0x28:/*PLP */_A_IMP();_RD();_SA(0x0100|c.S++);_RD();_SA(0x0100|c.S);_RD();c.P=(_GD()&~M6502_BF)|M6502_XF;break;
            case 0x29:/*AND #*/_A_IMM();_RD();c.A&=_GD();_NZ(c.A);break;
            case 0x2a:/*ROLA */_A_IMP();_RD();{bool carry=c.P&M6502_CF;c.P&=~(M6502_NF|M6502_ZF|M6502_CF);if(c.A&0x80){c.P|=M6502_CF;}c.A<<=1;if(carry){c.A|=0x01;}_NZ(c.A);}break;
            case 0x2b:/*ANC # (undoc)*/_A_IMM();_RD();c.A&=_GD();_NZ(c.A);if(c.A&0x80){c.P|=M6502_CF;}else{c.P&=~M6502_CF;}break;
            case 0x2c:/*BIT abs*/_A_ABS();_RD();l=_GD();h=c.A&l;c.P&=~(M6502_NF|M6502_VF|M6502_ZF);if(!h){c.P|=M6502_ZF;}c.P|=l&(M6502_NF|M6502_VF);break;
            case 0x2d:/*AND abs*/_A_ABS();_RD();c.A&=_GD();_NZ(c.A);break;
            case 0x2e:/*ROL abs*/_A_ABS();_RD();_WR();l=_GD();{bool carry=c.P&M6502_CF;c.P&=~(M6502_NF|M6502_ZF|M6502_CF);if(l&0x80){c.P|=M6502_CF;}l<<=1;if(carry){l|=0x01;}_NZ(l);}_SD(l);_WR();break;
            case 0x2f:/*RLA abs (undoc)*/_A_ABS();_RD();_WR();l=_GD();{bool carry=c.P&M6502_CF;c.P&=~(M6502_NF|M6502_ZF|M6502_CF);if(l&0x80){c.P|=M6502_CF;}l<<=1;if(carry){l|=0x01;}_NZ(l);}_SD(l);_WR();c.A&=l;_NZ(c.A);break;
            case 0x30:/*BMI #*/_A_IMM();_RD();if((c.P&0x80)==0x80){_RD();t=c.PC+(int8_t)_GD();if((t&0xFF00)!=(c.PC&0xFF00)){_RD();}c.PC=t;}break;
            case 0x31:/*AND (zp),Y*/_A_IDY_R();_RD();c.A&=_GD();_NZ(c.A);break;
            case 0x32:/*INVALID*/break;
            case 0x33:/*RLA (zp),Y (undoc)*/_A_IDY_W();_RD();_WR();l=_GD();{bool carry=c.P&M6502_CF;c.P&=~(M6502_NF|M6502_ZF|M6502_CF);if(l&0x80){c.P|=M6502_CF;}l<<=1;if(carry){l|=0x01;}_NZ(l);}_SD(l);_WR();c.A&=l;_NZ(c.A);break;
            case 0x34:/*NOP zp,X (undoc)*/_A_ZPX();_RD();break;
            case 0x35:/*AND zp,X*/_A_ZPX();_RD();c.A&=_GD();_NZ(c.A);break;
            case 0x36:/*ROL zp,X*/_A_ZPX();_RD();_WR();l=_GD();{bool carry=c.P&M6502_CF;c.P&=~(M6502_NF|M6502_ZF|M6502_CF);if(l&0x80){c.P|=M6502_CF;}l<<=1;if(carry){l|=0x01;}_NZ(l);}_SD(l);_WR();break;
            case 0x37:/*RLA zp,X (undoc)*/_A_ZPX();_RD();_WR();l=_GD();{bool carry=c.P&M6502_CF;c.P&=~(M6502_NF|M6502_ZF|M6502_CF);if(l&0x80){c.P|=M6502_CF;}l<<=1;if(carry){l|=0x01;}_NZ(l);}_SD(l);_WR();c.A&=l;_NZ(c.A);break;
            case 0x38:/*SEC */_A_IMP();_RD();c.P|=0x1;break;
            case 0x39:/*AND abs,Y*/_A_ABY_R();_RD();c.A&=_GD();_NZ(c.A);break;
            case 0x3a:/*NOP  (undoc)*/_A_IMP();_RD();break;
            case 0x3b:/*RLA abs,Y (undoc)*/_A_ABY_W();_RD();_WR();l=_GD();{bool carry=c.P&M6502_CF;c.P&=~(M6502_NF|M6502_ZF|M6502_CF);if(l&0x80){c.P|=M6502_CF;}l<<=1;if(carry){l|=0x01;}_NZ(l);}_SD(l);_WR();c.A&=l;_NZ(c.A);break;
            case 0x3c:/*NOP abs,X (undoc)*/_A_ABX_R();_RD();break;
            case 0x3d:/*AND abs,X*/_A_ABX_R();_RD();c.A&=_GD();_NZ(c.A);break;
            case 0x3e:/*ROL abs,X*/_A_ABX_W();_RD();_WR();l=_GD();{bool carry=c.P&M6502_CF;c.P&=~(M6502_NF|M6502_ZF|M6502_CF);if(l&0x80){c.P|=M6502_CF;}l<<=1;if(carry){l|=0x01;}_NZ(l);}_SD(l);_WR();break;
            case 0x3f:/*RLA abs,X (undoc)*/_A_ABX_W();_RD();_WR();l=_GD();{bool carry=c.P&M6502_CF;c.P&=~(M6502_NF|M6502_ZF|M6502_CF);if(l&0x80){c.P|=M6502_CF;}l<<=1;if(carry){l|=0x01;}_NZ(l);}_SD(l);_WR();c.A&=l;_NZ(c.A);break;
            case 0x40:/*RTI */_A_IMP();_RD();_SA(0x0100|c.S++);_RD();_SA(0x0100|c.S++);_RD();c.P=(_GD()&~M6502_BF)|M6502_XF;_SA(0x0100|c.S++);_RD();l=_GD();_SA(0x0100|c.S);_RD();h=_GD();c.PC=(h<<8)|l;c.pi=c.P;break;
            case 0x41:/*EOR (zp,X)*/_A_IDX();_RD();c.A^=_GD();_NZ(c.A);break;
            case 0x42:/*INVALID*/break;
            case 0x43:/*SRE (zp,X) (undoc)*/_A_IDX();_RD();_WR();l=_GD();c.P=(c.P&~M6502_CF)|((l&0x01)?M6502_CF:0);l>>=1;_NZ(l);_SD(l);_WR();c.A^=l;_NZ(c.A);break;
            case 0x44:/*NOP zp (undoc)*/_A_ZER();_RD();break;
            case 0x45:/*EOR zp*/_A_ZER();_RD();c.A^=_GD();_NZ(c.A);break;
            case 0x46:/*LSR zp*/_A_ZER();_RD();_WR();l=_GD();c.P=(c.P&~M6502_CF)|((l&0x01)?M6502_CF:0);l>>=1;_NZ(l);_SD(l);_WR();break;
            case 0x47:/*SRE zp (undoc)*/_A_ZER();_RD();_WR();l=_GD();c.P=(c.P&~M6502_CF)|((l&0x01)?M6502_CF:0);l>>=1;_NZ(l);_SD(l);_WR();c.A^=l;_NZ(c.A);break;
            case 0x48:/*PHA */_A_IMP();_RD();_SAD(0x0100|c.S--,c.A);_WR();break;
            case 0x49:/*EOR #*/_A_IMM();_RD();c.A^=_GD();_NZ(c.A);break;
            case 0x4a:/*LSRA */_A_IMP();_RD();c.P=(c.P&~M6502_CF)|((c.A&0x01)?M6502_CF:0);c.A>>=1;_NZ(c.A);break;
            case 0x4b:/*ASR # (undoc)*/_A_IMM();_RD();c.A&=_GD();c.P=(c.P&~M6502_CF)|((c.A&0x01)?M6502_CF:0);c.A>>=1;_NZ(c.A);break;
            case 0x4c:/*JMP */_SA(c.PC++);_RD();l=_GD();_SA(c.PC++);_RD();h=_GD();c.PC=(h<<8)|l;break;
            case 0x4d:/*EOR abs*/_A_ABS();_RD();c.A^=_GD();_NZ(c.A);break;
            case 0x4e:/*LSR abs*/_A_ABS();_RD();_WR();l=_GD();c.P=(c.P&~M6502_CF)|((l&0x01)?M6502_CF:0);l>>=1;_NZ(l);_SD(l);_WR();break;
            case 0x4f:/*SRE abs (undoc)*/_A_ABS();_RD();_WR();l=_GD();c.P=(c.P&~M6502_CF)|((l&0x01)?M6502_CF:0);l>>=1;_NZ(l);_SD(l);_WR();c.A^=l;_NZ(c.A);break;
            case 0x50:/*BVC #*/_A_IMM();_RD();if((c.P&0x40)==0x0){_RD();t=c.PC+(int8_t)_GD();if((t&0xFF00)!=(c.PC&0xFF00)){_RD();}c.PC=t;}break;
            case 0x51:/*EOR (zp),Y*/_A_IDY_R();_RD();c.A^=_GD();_NZ(c.A);break;
            case 0x52:/*INVALID*/break;
            case 0x53:/*SRE (zp),Y (undoc)*/_A_IDY_W();_RD();_WR();l=_GD();c.P=(c.P&~M6502_CF)|((l&0x01)?M6502_CF:0);l>>=1;_NZ(l);_SD(l);_WR();c.A^=l;_NZ(c.A);break;
            case 0x54:/*NOP zp,X (undoc)*/_A_ZPX();_RD();break;
            case 0x55:/*EOR zp,X*/_A_ZPX();_RD();c.A^=_GD();_NZ(c.A);break;
            case 0x56:/*LSR zp,X*/_A_ZPX();_RD();_WR();l=_GD();c.P=(c.P&~M6502_CF)|((l&0x01)?M6502_CF:0);l>>=1;_NZ(l);_SD(l);_WR();break;
            case 0x57:/*SRE zp,X (undoc)*/_A_ZPX();_RD();_WR();l=_GD();c.P=(c.P&~M6502_CF)|((l&0x01)?M6502_CF:0);l>>=1;_NZ(l);_SD(l);_WR();c.A^=l;_NZ(c.A);break;
            case 0x58:/*CLI */_A_IMP();_RD();c.P&=~0x4;break;
            case 0x59:/*EOR abs,Y*/_A_ABY_R();_RD();c.A^=_GD();_NZ(c.A);break;
            case 0x5a:/*NOP  (undoc)*/_A_IMP();_RD();break;
            case 0x5b:/*SRE abs,Y (undoc)*/_A_ABY_W();_RD();_WR();l=_GD();c.P=(c.P&~M6502_CF)|((l&0x01)?M6502_CF:0);l>>=1;_NZ(l);_SD(l);_WR();c.A^=l;_NZ(c.A);break;
            case 0x5c:/*NOP abs (undoc)*/_A_ABS();_RD();break;
            case 0x5d:/*EOR abs,X*/_A_ABX_R();_RD();c.A^=_GD();_NZ(c.A);break;
            case 0x5e:/*LSR abs,X*/_A_ABX_W();_RD();_WR();l=_GD();c.P=(c.P&~M6502_CF)|((l&0x01)?M6502_CF:0);l>>=1;_NZ(l);_SD(l);_WR();break;
            case 0x5f:/*SRE abs,X (undoc)*/_A_ABX_W();_RD();_WR();l=_GD();c.P=(c.P&~M6502_CF)|((l&0x01)?M6502_CF:0);l>>=1;_NZ(l);_SD(l);_WR();c.A^=l;_NZ(c.A);break;
            case 0x60:/*RTS */_A_IMP();_RD();_SA(0x0100|c.S++);_RD();_SA(0x0100|c.S++);_RD();l=_GD();_SA(0x0100|c.S);_RD();h=_GD();c.PC=(h<<8)|l;_SA(c.PC++);_RD();break;
            case 0x61:/*ADC (zp,X)*/_A_IDX();_RD();_m6502_adc(&c,_GD());break;
            case 0x62:/*INVALID*/break;
            case 0x63:/*RRA (zp,X) (undoc)*/_A_IDX();_RD();_WR();l=_GD();{bool carry=c.P&M6502_CF;c.P&=~(M6502_NF|M6502_ZF|M6502_CF);if(l&0x01){c.P|=M6502_CF;}l>>=1;if(carry){l|=0x80;}_NZ(l);}_SD(l);_WR();_m6502_adc(&c,l);break;
            case 0x64:/*NOP zp (undoc)*/_A_ZER();_RD();break;
            case 0x65:/*ADC zp*/_A_ZER();_RD();_m6502_adc(&c,_GD());break;
            case 0x66:/*ROR zp*/_A_ZER();_RD();_WR();l=_GD();{bool carry=c.P&M6502_CF;c.P&=~(M6502_NF|M6502_ZF|M6502_CF);if(l&0x01){c.P|=M6502_CF;}l>>=1;if(carry){l|=0x80;}_NZ(l);}_SD(l);_WR();break;
            case 0x67:/*RRA zp (undoc)*/_A_ZER();_RD();_WR();l=_GD();{bool carry=c.P&M6502_CF;c.P&=~(M6502_NF|M6502_ZF|M6502_CF);if(l&0x01){c.P|=M6502_CF;}l>>=1;if(carry){l|=0x80;}_NZ(l);}_SD(l);_WR();_m6502_adc(&c,l);break;
            case 0x68:/*PLA */_A_IMP();_RD();_SA(0x0100|c.S++);_RD();_SA(0x0100|c.S);_RD();c.A=_GD();_NZ(c.A);break;
            case 0x69:/*ADC #*/_A_IMM();_RD();_m6502_adc(&c,_GD());break;
            case 0x6a:/*RORA */_A_IMP();_RD();{bool carry=c.P&M6502_CF;c.P&=~(M6502_NF|M6502_ZF|M6502_CF);if(c.A&0x01){c.P|=M6502_CF;}c.A>>=1;if(carry){c.A|=0x80;}_NZ(c.A);}break;
            case 0x6b:/*ARR # (undoc)*/_A_IMM();_RD();c.A&=_GD();_m6502_arr(&c);break;
            case 0x6c:/*JMPI */_SA(c.PC++);_RD();l=_GD();_SA(c.PC++);_RD();h=_GD();a=(h<<8)|l;_SA(a);_RD();l=_GD();a=(a&0xFF00)|((a+1)&0x00FF);_SA(a);_RD();h=_GD();c.PC=(h<<8)|l;break;
            case 0x6d:/*ADC abs*/_A_ABS();_RD();_m6502_adc(&c,_GD());break;
            case 0x6e:/*ROR abs*/_A_ABS();_RD();_WR();l=_GD();{bool carry=c.P&M6502_CF;c.P&=~(M6502_NF|M6502_ZF|M6502_CF);if(l&0x01){c.P|=M6502_CF;}l>>=1;if(carry){l|=0x80;}_NZ(l);}_SD(l);_WR();break;
            case 0x6f:/*RRA abs (undoc)*/_A_ABS();_RD();_WR();l=_GD();{bool carry=c.P&M6502_CF;c.P&=~(M6502_NF|M6502_ZF|M6502_CF);if(l&0x01){c.P|=M6502_CF;}l>>=1;if(carry){l|=0x80;}_NZ(l);}_SD(l);_WR();_m6502_adc(&c,l);break;
            case 0x70:/*BVS #*/_A_IMM();_RD();if((c.P&0x40)==0x40){_RD();t=c.PC+(int8_t)_GD();if((t&0xFF00)!=(c.PC&0xFF00)){_RD();}c.PC=t;}break;
            case 0x71:/*ADC (zp),Y*/_A_IDY_R();_RD();_m6502_adc(&c,_GD());break;
            case 0x72:/*INVALID*/break;
            case 0x73:/*RRA (zp),Y (undoc)*/_A_IDY_W();_RD();_WR();l=_GD();{bool carry=c.P&M6502_CF;c.P&=~(M6502_NF|M6502_ZF|M6502_CF);if(l&0x01){c.P|=M6502_CF;}l>>=1;if(carry){l|=0x80;}_NZ(l);}_SD(l);_WR();_m6502_adc(&c,l);break;
            case 0x74:/*NOP zp,X (undoc)*/_A_ZPX();_RD();break;
            case 0x75:/*ADC zp,X*/_A_ZPX();_RD();_m6502_adc(&c,_GD());break;
            case 0x76:/*ROR zp,X*/_A_ZPX();_RD();_WR();l=_GD();{bool carry=c.P&M6502_CF;c.P&=~(M6502_NF|M6502_ZF|M6502_CF);if(l&0x01){c.P|=M6502_CF;}l>>=1;if(carry){l|=0x80;}_NZ(l);}_SD(l);_WR();break;
            case 0x77:/*RRA zp,X (undoc)*/_A_ZPX();_RD();_WR();l=_GD();{bool carry=c.P&M6502_CF;c.P&=~(M6502_NF|M6502_ZF|M6502_CF);if(l&0x01){c.P|=M6502_CF;}l>>=1;if(carry){l|=0x80;}_NZ(l);}_SD(l);_WR();_m6502_adc(&c,l);break;
            case 0x78:/*SEI */_A_IMP();_RD();c.P|=0x4;break;
            case 0x79:/*ADC abs,Y*/_A_ABY_R();_RD();_m6502_adc(&c,_GD());break;
            case 0x7a:/*NOP  (undoc)*/_A_IMP();_RD();break;
            case 0x7b:/*RRA abs,Y (undoc)*/_A_ABY_W();_RD();_WR();l=_GD();{bool carry=c.P&M6502_CF;c.P&=~(M6502_NF|M6502_ZF|M6502_CF);if(l&0x01){c.P|=M6502_CF;}l>>=1;if(carry){l|=0x80;}_NZ(l);}_SD(l);_WR();_m6502_adc(&c,l);break;
            case 0x7c:/*NOP abs (undoc)*/_A_ABS();_RD();break;
            case 0x7d:/*ADC abs,X*/_A_ABX_R();_RD();_m6502_adc(&c,_GD());break;
            case 0x7e:/*ROR abs,X*/_A_ABX_W();_RD();_WR();l=_GD();{bool carry=c.P&M6502_CF;c.P&=~(M6502_NF|M6502_ZF|M6502_CF);if(l&0x01){c.P|=M6502_CF;}l>>=1;if(carry){l|=0x80;}_NZ(l);}_SD(l);_WR();break;
            case 0x7f:/*RRA abs,X (undoc)*/_A_ABX_W();_RD();_WR();l=_GD();{bool carry=c.P&M6502_CF;c.P&=~(M6502_NF|M6502_ZF|M6502_CF);if(l&0x01){c.P|=M6502_CF;}l>>=1;if(carry){l|=0x80;}_NZ(l);}_SD(l);_WR();_m6502_adc(&c,l);break;
            case 0x80:/*NOP # (undoc)*/_A_IMM();_RD();break;
            case 0x81:/*STA (zp,X)*/_A_IDX();_SD(c.A);_WR();break;
            case 0x82:/*NOP # (undoc)*/_A_IMM();_RD();break;
            case 0x83:/*SAX (zp,X) (undoc)*/_A_IDX();_SD(c.A&c.X);_WR();break;
            case 0x84:/*STY zp*/_A_ZER();_SD(c.Y);_WR();break;
            case 0x85:/*STA zp*/_A_ZER();_SD(c.A);_WR();break;
            case 0x86:/*STX zp*/_A_ZER();_SD(c.X);_WR();break;
            case 0x87:/*SAX zp (undoc)*/_A_ZER();_SD(c.A&c.X);_WR();break;
            case 0x88:/*DEY */_A_IMP();_RD();c.Y--;_NZ(c.Y);break;
            case 0x89:/*NOP # (undoc)*/_A_IMM();_RD();break;
            case 0x8a:/*TXA */_A_IMP();_RD();c.A=c.X;_NZ(c.A);break;
            case 0x8b:/*ANE # (undoc)*/_A_IMM();_RD();l=_GD();c.A&=l&c.X;_NZ(c.A);break;
            case 0x8c:/*STY abs*/_A_ABS();_SD(c.Y);_WR();break;
            case 0x8d:/*STA abs*/_A_ABS();_SD(c.A);_WR();break;
            case 0x8e:/*STX abs*/_A_ABS();_SD(c.X);_WR();break;
            case 0x8f:/*SAX abs (undoc)*/_A_ABS();_SD(c.A&c.X);_WR();break;
            case 0x90:/*BCC #*/_A_IMM();_RD();if((c.P&0x1)==0x0){_RD();t=c.PC+(int8_t)_GD();if((t&0xFF00)!=(c.PC&0xFF00)){_RD();}c.PC=t;}break;
            case 0x91:/*STA (zp),Y*/_A_IDY_W();_SD(c.A);_WR();break;
            case 0x92:/*INVALID*/break;
            case 0x93:/*SHA (not impl) (zp),Y (undoc)*/_A_IDY_W();_RD();break;
            case 0x94:/*STY zp,X*/_A_ZPX();_SD(c.Y);_WR();break;
            case 0x95:/*STA zp,X*/_A_ZPX();_SD(c.A);_WR();break;
            case 0x96:/*STX zp,Y*/_A_ZPY();_SD(c.X);_WR();break;
            case 0x97:/*SAX zp,Y (undoc)*/_A_ZPY();_SD(c.A&c.X);_WR();break;
            case 0x98:/*TYA */_A_IMP();_RD();c.A=c.Y;_NZ(c.A);break;
            case 0x99:/*STA abs,Y*/_A_ABY_W();_SD(c.A);_WR();break;
            case 0x9a:/*TXS */_A_IMP();_RD();c.S=c.X;break;
            case 0x9b:/*SHS (not impl) abs,Y (undoc)*/_A_ABY_W();_RD();break;
            case 0x9c:/*SHY (not impl) abs,X (undoc)*/_A_ABX_W();_RD();break;
            case 0x9d:/*STA abs,X*/_A_ABX_W();_SD(c.A);_WR();break;
            case 0x9e:/*SHX (not impl) abs,Y (undoc)*/_A_ABY_W();_RD();break;
            case 0x9f:/*SHA (not impl) abs,Y (undoc)*/_A_ABY_W();_RD();break;
            case 0xa0:/*LDY #*/_A_IMM();_RD();c.Y=_GD();_NZ(c.Y);break;
            case 0xa1:/*LDA (zp,X)*/_A_IDX();_RD();c.A=_GD();_NZ(c.A);break;
            case 0xa2:/*LDX #*/_A_IMM();_RD();c.X=_GD();_NZ(c.X);break;
            case 0xa3:/*LAX (zp,X) (undoc)*/_A_IDX();_RD();c.A=c.X=_GD();_NZ(c.A);break;
            case 0xa4:/*LDY zp*/_A_ZER();_RD();c.Y=_GD();_NZ(c.Y);break;
            case 0xa5:/*LDA zp*/_A_ZER();_RD();c.A=_GD();_NZ(c.A);break;
            case 0xa6:/*LDX zp*/_A_ZER();_RD();c.X=_GD();_NZ(c.X);break;
            case 0xa7:/*LAX zp (undoc)*/_A_ZER();_RD();c.A=c.X=_GD();_NZ(c.A);break;
            case 0xa8:/*TAY */_A_IMP();_RD();c.Y=c.A;_NZ(c.Y);break;
            case 0xa9:/*LDA #*/_A_IMM();_RD();c.A=_GD();_NZ(c.A);break;
            case 0xaa:/*TAX */_A_IMP();_RD();c.X=c.A;_NZ(c.X);break;
            case 0xab:/*LXA # (undoc)*/_A_IMM();_RD();c.A&=_GD();c.X=c.A;_NZ(c.A);break;
            case 0xac:/*LDY abs*/_A_ABS();_RD();c.Y=_GD();_NZ(c.Y);break;
            case 0xad:/*LDA abs*/_A_ABS();_RD();c.A=_GD();_NZ(c.A);break;
            case 0xae:/*LDX abs*/_A_ABS();_RD();c.X=_GD();_NZ(c.X);break;
            case 0xaf:/*LAX abs (undoc)*/_A_ABS();_RD();c.A=c.X=_GD();_NZ(c.A);break;
            case 0xb0:/*BCS #*/_A_IMM();_RD();if((c.P&0x1)==0x1){_RD();t=c.PC+(int8_t)_GD();if((t&0xFF00)!=(c.PC&0xFF00)){_RD();}c.PC=t;}break;
            case 0xb1:/*LDA (zp),Y*/_A_IDY_R();_RD();c.A=_GD();_NZ(c.A);break;
            case 0xb2:/*INVALID*/break;
            case 0xb3:/*LAX (zp),Y (undoc)*/_A_IDY_R();_RD();c.A=c.X=_GD();_NZ(c.A);break;
            case 0xb4:/*LDY zp,X*/_A_ZPX();_RD();c.Y=_GD();_NZ(c.Y);break;
            case 0xb5:/*LDA zp,X*/_A_ZPX();_RD();c.A=_GD();_NZ(c.A);break;
            case 0xb6:/*LDX zp,Y*/_A_ZPY();_RD();c.X=_GD();_NZ(c.X);break;
            case 0xb7:/*LAX zp,Y (undoc)*/_A_ZPY();_RD();c.A=c.X=_GD();_NZ(c.A);break;
            case 0xb8:/*CLV */_A_IMP();_RD();c.P&=~0x40;break;
            case 0xb9:/*LDA abs,Y*/_A_ABY_R();_RD();c.A=_GD();_NZ(c.A);break;
            case 0xba:/*TSX */_A_IMP();_RD();c.X=c.S;_NZ(c.X);break;
            case 0xbb:/*LAS (not impl) abs,Y (undoc)*/_A_ABY_R();_RD();break;
            case 0xbc:/*LDY abs,X*/_A_ABX_R();_RD();c.Y=_GD();_NZ(c.Y);break;
            case 0xbd:/*LDA abs,X*/_A_ABX_R();_RD();c.A=_GD();_NZ(c.A);break;
            case 0xbe:/*LDX abs,Y*/_A_ABY_R();_RD();c.X=_GD();_NZ(c.X);break;
            case 0xbf:/*LAX abs,Y (undoc)*/_A_ABY_R();_RD();c.A=c.X=_GD();_NZ(c.A);break;
            case 0xc0:/*CPY #*/_A_IMM();_RD();l=_GD();t=c.Y-l;_NZ((uint8_t)t)&~M6502_CF;if(!(t&0xFF00)){c.P|=M6502_CF;}break;
            case 0xc1:/*CMP (zp,X)*/_A_IDX();_RD();l=_GD();t=c.A-l;_NZ((uint8_t)t)&~M6502_CF;if(!(t&0xFF00)){c.P|=M6502_CF;}break;
            case 0xc2:/*NOP # (undoc)*/_A_IMM();_RD();break;
            case 0xc3:/*DCP (zp,X) (undoc)*/_A_IDX();_RD();_WR();l=_GD();l--;_NZ(l);_SD(l);_WR();t=c.A-l;_NZ((uint8_t)t)&~M6502_CF;if(!(t&0xFF00)){c.P|=M6502_CF;}break;
            case 0xc4:/*CPY zp*/_A_ZER();_RD();l=_GD();t=c.Y-l;_NZ((uint8_t)t)&~M6502_CF;if(!(t&0xFF00)){c.P|=M6502_CF;}break;
            case 0xc5:/*CMP zp*/_A_ZER();_RD();l=_GD();t=c.A-l;_NZ((uint8_t)t)&~M6502_CF;if(!(t&0xFF00)){c.P|=M6502_CF;}break;
            case 0xc6:/*DEC zp*/_A_ZER();_RD();l=_GD();_WR();l--;_NZ(l);_SD(l);_WR();break;
            case 0xc7:/*DCP zp (undoc)*/_A_ZER();_RD();_WR();l=_GD();l--;_NZ(l);_SD(l);_WR();t=c.A-l;_NZ((uint8_t)t)&~M6502_CF;if(!(t&0xFF00)){c.P|=M6502_CF;}break;
            case 0xc8:/*INY */_A_IMP();_RD();c.Y++;_NZ(c.Y);break;
            case 0xc9:/*CMP #*/_A_IMM();_RD();l=_GD();t=c.A-l;_NZ((uint8_t)t)&~M6502_CF;if(!(t&0xFF00)){c.P|=M6502_CF;}break;
            case 0xca:/*DEX */_A_IMP();_RD();c.X--;_NZ(c.X);break;
            case 0xcb:/*SBX (not impl) # (undoc)*/_A_IMM();_RD();break;
            case 0xcc:/*CPY abs*/_A_ABS();_RD();l=_GD();t=c.Y-l;_NZ((uint8_t)t)&~M6502_CF;if(!(t&0xFF00)){c.P|=M6502_CF;}break;
            case 0xcd:/*CMP abs*/_A_ABS();_RD();l=_GD();t=c.A-l;_NZ((uint8_t)t)&~M6502_CF;if(!(t&0xFF00)){c.P|=M6502_CF;}break;
            case 0xce:/*DEC abs*/_A_ABS();_RD();l=_GD();_WR();l--;_NZ(l);_SD(l);_WR();break;
            case 0xcf:/*DCP abs (undoc)*/_A_ABS();_RD();_WR();l=_GD();l--;_NZ(l);_SD(l);_WR();t=c.A-l;_NZ((uint8_t)t)&~M6502_CF;if(!(t&0xFF00)){c.P|=M6502_CF;}break;
            case 0xd0:/*BNE #*/_A_IMM();_RD();if((c.P&0x2)==0x0){_RD();t=c.PC+(int8_t)_GD();if((t&0xFF00)!=(c.PC&0xFF00)){_RD();}c.PC=t;}break;
            case 0xd1:/*CMP (zp),Y*/_A_IDY_R();_RD();l=_GD();t=c.A-l;_NZ((uint8_t)t)&~M6502_CF;if(!(t&0xFF00)){c.P|=M6502_CF;}break;
            case 0xd2:/*INVALID*/break;
            case 0xd3:/*DCP (zp),Y (undoc)*/_A_IDY_W();_RD();_WR();l=_GD();l--;_NZ(l);_SD(l);_WR();t=c.A-l;_NZ((uint8_t)t)&~M6502_CF;if(!(t&0xFF00)){c.P|=M6502_CF;}break;
            case 0xd4:/*NOP zp,X (undoc)*/_A_ZPX();_RD();break;
            case 0xd5:/*CMP zp,X*/_A_ZPX();_RD();l=_GD();t=c.A-l;_NZ((uint8_t)t)&~M6502_CF;if(!(t&0xFF00)){c.P|=M6502_CF;}break;
            case 0xd6:/*DEC zp,X*/_A_ZPX();_RD();l=_GD();_WR();l--;_NZ(l);_SD(l);_WR();break;
            case 0xd7:/*DCP zp,X (undoc)*/_A_ZPX();_RD();_WR();l=_GD();l--;_NZ(l);_SD(l);_WR();t=c.A-l;_NZ((uint8_t)t)&~M6502_CF;if(!(t&0xFF00)){c.P|=M6502_CF;}break;
            case 0xd8:/*CLD */_A_IMP();_RD();c.P&=~0x8;break;
            case 0xd9:/*CMP abs,Y*/_A_ABY_R();_RD();l=_GD();t=c.A-l;_NZ((uint8_t)t)&~M6502_CF;if(!(t&0xFF00)){c.P|=M6502_CF;}break;
            case 0xda:/*NOP  (undoc)*/_A_IMP();_RD();break;
            case 0xdb:/*DCP abs,Y (undoc)*/_A_ABY_W();_RD();_WR();l=_GD();l--;_NZ(l);_SD(l);_WR();t=c.A-l;_NZ((uint8_t)t)&~M6502_CF;if(!(t&0xFF00)){c.P|=M6502_CF;}break;
            case 0xdc:/*NOP abs,X (undoc)*/_A_ABX_R();_RD();break;
            case 0xdd:/*CMP abs,X*/_A_ABX_R();_RD();l=_GD();t=c.A-l;_NZ((uint8_t)t)&~M6502_CF;if(!(t&0xFF00)){c.P|=M6502_CF;}break;
            case 0xde:/*DEC abs,X*/_A_ABX_W();_RD();l=_GD();_WR();l--;_NZ(l);_SD(l);_WR();break;
            case 0xdf:/*DCP abs,X (undoc)*/_A_ABX_W();_RD();_WR();l=_GD();l--;_NZ(l);_SD(l);_WR();t=c.A-l;_NZ((uint8_t)t)&~M6502_CF;if(!(t&0xFF00)){c.P|=M6502_CF;}break;
            case 0xe0:/*CPX #*/_A_IMM();_RD();l=_GD();t=c.X-l;_NZ((uint8_t)t)&~M6502_CF;if(!(t&0xFF00)){c.P|=M6502_CF;}break;
            case 0xe1:/*SBC (zp,X)*/_A_IDX();_RD();_m6502_sbc(&c,_GD());break;
            case 0xe2:/*NOP # (undoc)*/_A_IMM();_RD();break;
            case 0xe3:/*ISB (zp,X) (undoc)*/_A_IDX();_RD();_WR();l=_GD();l++;_SD(l);_WR();_m6502_sbc(&c,l);break;
            case 0xe4:/*CPX zp*/_A_ZER();_RD();l=_GD();t=c.X-l;_NZ((uint8_t)t)&~M6502_CF;if(!(t&0xFF00)){c.P|=M6502_CF;}break;
            case 0xe5:/*SBC zp*/_A_ZER();_RD();_m6502_sbc(&c,_GD());break;
            case 0xe6:/*INC zp*/_A_ZER();_RD();l=_GD();_WR();l++;_NZ(l);_SD(l);_WR();break;
            case 0xe7:/*ISB zp (undoc)*/_A_ZER();_RD();_WR();l=_GD();l++;_SD(l);_WR();_m6502_sbc(&c,l);break;
            case 0xe8:/*INX */_A_IMP();_RD();c.X++;_NZ(c.X);break;
            case 0xe9:/*SBC #*/_A_IMM();_RD();_m6502_sbc(&c,_GD());break;
            case 0xea:/*NOP */_A_IMP();_RD();break;
            case 0xeb:/*SBC # (undoc)*/_A_IMM();_RD();_m6502_sbc(&c,_GD());break;
            case 0xec:/*CPX abs*/_A_ABS();_RD();l=_GD();t=c.X-l;_NZ((uint8_t)t)&~M6502_CF;if(!(t&0xFF00)){c.P|=M6502_CF;}break;
            case 0xed:/*SBC abs*/_A_ABS();_RD();_m6502_sbc(&c,_GD());break;
            case 0xee:/*INC abs*/_A_ABS();_RD();l=_GD();_WR();l++;_NZ(l);_SD(l);_WR();break;
            case 0xef:/*ISB abs (undoc)*/_A_ABS();_RD();_WR();l=_GD();l++;_SD(l);_WR();_m6502_sbc(&c,l);break;
            case 0xf0:/*BEQ #*/_A_IMM();_RD();if((c.P&0x2)==0x2){_RD();t=c.PC+(int8_t)_GD();if((t&0xFF00)!=(c.PC&0xFF00)){_RD();}c.PC=t;}break;
            case 0xf1:/*SBC (zp),Y*/_A_IDY_R();_RD();_m6502_sbc(&c,_GD());break;
            case 0xf2:/*INVALID*/break;
            case 0xf3:/*ISB (zp),Y (undoc)*/_A_IDY_W();_RD();_WR();l=_GD();l++;_SD(l);_WR();_m6502_sbc(&c,l);break;
            case 0xf4:/*NOP zp,X (undoc)*/_A_ZPX();_RD();break;
            case 0xf5:/*SBC zp,X*/_A_ZPX();_RD();_m6502_sbc(&c,_GD());break;
            case 0xf6:/*INC zp,X*/_A_ZPX();_RD();l=_GD();_WR();l++;_NZ(l);_SD(l);_WR();break;
            case 0xf7:/*ISB zp,X (undoc)*/_A_ZPX();_RD();_WR();l=_GD();l++;_SD(l);_WR();_m6502_sbc(&c,l);break;
            case 0xf8:/*SED */_A_IMP();_RD();c.P|=0x8;break;
            case 0xf9:/*SBC abs,Y*/_A_ABY_R();_RD();_m6502_sbc(&c,_GD());break;
            case 0xfa:/*NOP  (undoc)*/_A_IMP();_RD();break;
            case 0xfb:/*ISB abs,Y (undoc)*/_A_ABY_W();_RD();_WR();l=_GD();l++;_SD(l);_WR();_m6502_sbc(&c,l);break;
            case 0xfc:/*NOP abs,X (undoc)*/_A_ABX_R();_RD();break;
            case 0xfd:/*SBC abs,X*/_A_ABX_R();_RD();_m6502_sbc(&c,_GD());break;
            case 0xfe:/*INC abs,X*/_A_ABX_W();_RD();l=_GD();_WR();l++;_NZ(l);_SD(l);_WR();break;
            case 0xff:/*ISB abs,X (undoc)*/_A_ABX_W();_RD();_WR();l=_GD();l++;_SD(l);_WR();_m6502_sbc(&c,l);break;

        }
        /* edge detection for NMI pin */
        bool nmi = 0 != ((pins & (pre_pins ^ pins)) & M6502_NMI);
        /* check for interrupt request */
        if (nmi || ((pins & M6502_IRQ) && !(c.pi & M6502_IF))) {
            /* execute a slightly modified BRK instruction, do NOT increment PC! */
            _SA(c.PC);_ON(M6502_SYNC);_RD();_OFF(M6502_SYNC);
            _SA(c.PC); _RD();
            _SAD(0x0100|c.S--, c.PC>>8); _WR();
            _SAD(0x0100|c.S--, c.PC); _WR();
            _SAD(0x0100|c.S--, c.P&~M6502_BF); _WR();
            if (pins & M6502_NMI) {
                _SA(0xFFFA); _RD(); l=_GD();
                c.P |= M6502_IF;
                _SA(0xFFFB); _RD(); h=_GD();
            }
            else {
                _SA(0xFFFE); _RD(); l=_GD();
                c.P |= M6502_IF;
                _SA(0xFFFF); _RD(); h=_GD();
            }
            c.PC = (h<<8)|l;
        }
        if (trap) {
            int trap_id=trap(c.PC,ticks,pins,cpu->trap_user_data);
            if (trap_id) {
                cpu->trap_id=trap_id;
                break;
            }
        }
    } while (ticks < num_ticks);
    M6510_SET_PORT(pins, cpu->io_pins);
    c.PINS = pins;
    cpu->state = c;
    return ticks;
}
#undef _SA
#undef _SAD
#undef _GD
#undef _ON
#undef _OFF
#undef _T
#undef _RD
#undef _WR
#undef _A_IMP
#undef _A_IMM
#undef _A_ZER
#undef _A_ZPX
#undef _A_ZPY
#undef _A_ABS
#undef _A_ABX_R
#undef _A_ABX_W
#undef _A_ABY_R
#undef _A_ABY_W
#undef _A_IDX
#undef _A_IDY_R
#undef _A_IDY_W
#undef _NZ
#endif /* CHIPS_IMPL */
