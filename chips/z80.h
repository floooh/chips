#pragma once
/*#
    # z80.h

    Header-only Z80 CPU emulator written in C.

    Project repo: https://github.com/floooh/chips/

    NOTE: this file is code-generated from z80.template.h and z80_gen.py
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
        your own assert macro (default: assert(c))

    ## Emulated Pins
    ***********************************
    *           +-----------+         *
    * M1    <---|           |---> A0  *
    * MREQ  <---|           |---> A1  *
    * IORQ  <---|           |---> A2  *
    * RD    <---|           |---> ..  *
    * WR    <---|    Z80    |---> A15 *
    * HALT  <---|           |         *
    * WAIT  --->|           |<--> D0  *
    * INT   --->|           |<--> D1  *
    *           |           |<--> ... *
    *           |           |<--> D7  *
    *           +-----------+         *
    ***********************************

    ## Not Emulated
    - refresh cycles (RFSH pin)
    - interrupt mode 0
    - bus request/acknowledge (BUSRQ and BUSAK pins)
    - the RESET pin is currently not checked, call the z80_reset() 
      function instead

    ## Functions
    ~~~C
    void z80_init(z80_t* cpu, const z80_desc_t* desc)
    ~~~
        Initializes a new Z80 CPU instance. The z80_desc_t struct
        provides initialization attributes:
            ~~~C
            typedef struct {
                z80_tick_t tick_cb; // the CPU tick callback
                void* user_data;    // user data arg handed to callbacks
            } z80_desc_t;
            ~~~
        The tick_cb function will be called from inside z80_exec().

    ~~~C
    void z80_reset(z80_t* cpu)
    ~~~
        Resets the Z80 CPU instance. 

    ~~~C
    uint32_t z80_exec(z80_t* cpu, uint32_t num_ticks)
    ~~~
        Starts executing instructions until num_ticks is reached or the PC
        hits a trap, returns the number of executed ticks. The number of
        executed ticks will be greater or equal to num_ticks (unless a trap
        has been hit), because complete instructions will be executed. During
        execution the tick callback will be invoked one or multiple times
        (usually once per machine cycle, but also for 'filler ticks' or wait
        state ticks). To check if a trap has been hit, test whether 
        z80_t.trap_id != 0
        NOTE: the z80_exec() function may return in the 'middle' of an
        DD/FD extended instruction (right after the prefix byte). If this
        is the case, z80_opdone() will return false.

    ~~~C
    bool z80_opdone(z80_t* cpu)
    ~~~
        Return true if z80_exec() has returned at the end of an instruction,
        and false if the CPU is in the middle of a DD/FD prefix instruction.

    ~~~C
    void z80_set_x(z80_t* cpu, uint8_t val)
    void z80_set_xx(z80_t* cpu, uint16_t val)
    uint8_t z80_x(z80_t* cpu)
    uint16_t z80_xx(z80_t* cpu)
    ~~~
        Set and get Z80 registers and flags.

    ~~~C
    void z80_trap_cb(z80_t* cpu, z80_trap_t trap_cb)
    ~~~
        Set an optional trap callback. If this is set it will be invoked
        at the end of an instruction with the current PC (which points
        to the start of the next instruction). The trap callback should
        return a non-zero value if the execution loop should exit. The
        returned value will also be written to z80_t.trap_id.
        Set a null ptr as trap callback disables the trap checking.
        To get the current trap callback, simply access z80_t.trap_cb directly.

    ## Macros
    ~~~C
    Z80_SET_ADDR(pins, addr)
    ~~~
        set 16-bit address bus pins in 64-bit pin mask

    ~~~C
    Z80_GET_ADDR(pins)
    ~~~
        extract 16-bit address bus value from 64-bit pin mask

    ~~~C
    Z80_SET_DATA(pins, data)
    ~~~
        set 8-bit data bus pins in 64-bit pin mask

    ~~~C
    Z80_GET_DATA(pins)
    ~~~
        extract 8-bit data bus value from 64-bit pin mask

    ~~~C
    Z80_MAKE_PINS(ctrl, addr, data)
    ~~~
        build 64-bit pin mask from control-, address- and data-pins

    ~~~C
    Z80_DAISYCHAIN_BEGIN(pins)
    ~~~
        used in tick function at start of interrupt daisy-chain block

    ~~~C
    Z80_DAISYCHAIN_END(pins)
    ~~~
        used in tick function at end of interrupt daisy-chain block

    ## The Tick Callback 

    The tick function is called for one or multiple time cycles
    and connects the Z80 to the outside world. Usually one call
    of the tick function corresponds to one machine cycle, 
    but this is not always the case. The tick functions takes
    2 arguments:

    - num_ticks: the number of time cycles for this callback invocation
    - pins: a 64-bit integer with CPU pins (address- and data-bus pins,
        and control pins)

    A simplest-possible tick callback which just performs memory read/write
    operations on a 64kByte byte array looks like this:

    ~~~C
    uint8_t mem[1<<16] = { 0 };
    uint64_t tick(int num_ticks, uint64_t pins, void* user_data) {
        if (pins & Z80_MREQ) {
            if (pins & Z80_RD) {
                Z80_SET_DATA(pins, mem[Z80_GET_ADDR(pins)]);
            }
            else if (pins & Z80_WR) {
                mem[Z80_GET_ADDR(pins)] = Z80_GET_DATA(pins);
            }
        }
        else if (pins & Z80_IORQ) {
            // FIXME: perform device I/O
        }
        return pins;
    }
    ~~~

    The tick callback inspects the pins, performs the requested actions
    (memory read/write and input/output), modifies the pin bitmask
    with requests for the CPU (inject wait states, or request an
    interrupt), and finally returns the pin bitmask back to the 
    CPU emulation.

    The following pin bitmasks are relevant for the tick callback:

    - **MREQ|RD**: This is a memory read cycle, the tick callback must 
      put the byte at the memory address indicated by the address
      bus pins A0..A15 (bits 0..15) into the data bus 
      pins (D0..D7). If the M1 pin is also set, then this
      is an opcode fetch machine cycle (4 clock ticks), otherwise
      this is a normal memory read machine cycle (3 clock ticks)
    - **MREQ|WR**: This is a memory write machine cycle, the tick
      callback must write the byte in the data bus pins (D0..D7)
      to the memory location in the address bus pins (A0..A15). 
      A memory write machine cycle is 3 clock-ticks.
    - **IORQ|RD**: This is a device-input machine cycle, the 16-bit
      port number is in the address bus pins (A0..A15), and the
      tick callback must write the input-byte to the data bus
      pins (D0..D7). An input machine cycle is 4 clock-ticks.
    - **IORQ|WR**: This is a device-output machine cycle, the data
      bus pins (D0..D7) contains the byte to be output
      at the port in the address-bus pins (A0..A15). An output
      machine cycle is 4 cycles.

    Interrupt handling requires to inspect and set additional
    pins, more on that below.

    To inject wait states, execute the additional cycles in the
    CPU tick callback, and set the number of wait states
    with the Z80_SET_WAIT() macro on the returned CPU pins.
    Up to 7 wait states can be injected per machine cycle.

    Note that not all calls to the tick callback have one
    of the above pin bit patterns set. The CPU may need
    to execute filler- or processing ticks which are
    not memory-, IO- or interrupt-handling operations.

    This may happen in the following situations:
    - opcode fetch machine cycles are always a single callback
      invocation of 4 cycles with the M1|MREQ|RD pins set, however
      in a real Z80, some opcode fetch machine cycles are 5 or 6
      cycles long, in this case, the tick callback will be called
      again without control pins set and a tick count of 1 or 2
    - some instructions require additional processing ticks which
      are not memory- or IO-operations, in this case the tick
      callback may be called for with any number of ticks, but
      without activated control pins

    ## Interrupt Handling

    The interrupt 'daisy chain protocol' is entirely implemented
    in the tick callback (usually the actual interrupt daisy chain
    handling happens in the Z80 chip-family emulators, but the
    tick callback needs to invoke their interrupt handling functions).

    An interrupt request/acknowledge cycle for (most common)
    interrupt mode 2 looks like this:

    - an interrupt is requested from inside the tick callback by
      setting the INT pin in any tick callback invocation (the 
      INT pins remains active until the end of the current instruction)
    - the CPU emulator checks the INT pin at the end of the current
      instruction, if the INT pin is active, an interrupt-request/acknowledge
      machine cycle is executed which results in additional tick
      callback invocations to perform the interrupt-acknowledge protocol
    - the interrupt-controller device with pending interrupt scans the
      pin bits for M1|IORQ during the tick callback, and if active,
      places the interrupt vector low-byte on the data bus pins
    - back in the CPU emulation, the interrupt request is completed by
      constructing the complete 16-bit interrupt vector, reading the
      address of the interrupt service routine from there, and setting
      the PC register to that address (the next executed instruction
      is then the first instruction of the interrupt service routine)

    There are 2 virtual pins for the interrupt daisy chain protocol:

    - **IEIO** (Interrupt Enable In/Out): This combines the IEI and IEO 
      pins found on Z80 chip-family members and is used to disable
      interrupts for lower-priority interrupt controllers in the
      daisy chain if a higher priority device is currently negotiating
      interrupt handling with the CPU. The IEIO pin always starts
      in active state at the start of the daisy chain, and is handed
      from one interrupt controller to the next in order of 
      daisy-chain priority. The first interrupt controller with
      active interrupt handling clears the IEIO bit, which prevent
      the 'downstream' lower-priority interrupt controllers from
      issuing interrupt requests.
    - **RETI** (Return From Interrupt): The virtual RETI pin is set
      by the CPU when it decodes a RETI instruction. This is scanned
      by the interrupt controller which is currently 'under service'
      to enable interrupt handling for lower-priority devices
      in the daisy chain. In a real Z80 system this the interrupt
      controller chips perform their own simple instruction decoding
      to detect RETI instructions.

    The CPU tick callback is the heart of emulation, for complete
    tick callback examples check the system emulators:
    
    https://github.com/floooh/chips/tree/master/systems

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

/*--- callback function typedefs ---*/
typedef uint64_t (*z80_tick_t)(int num_ticks, uint64_t pins, void* user_data);
typedef int (*z80_trap_t)(uint16_t pc, int ticks, uint64_t pins, void* trap_user_data);

/*--- address lines ---*/
#define Z80_A0  (1ULL<<0)
#define Z80_A1  (1ULL<<1)
#define Z80_A2  (1ULL<<2)
#define Z80_A3  (1ULL<<3)
#define Z80_A4  (1ULL<<4)
#define Z80_A5  (1ULL<<5)
#define Z80_A6  (1ULL<<6)
#define Z80_A7  (1ULL<<7)
#define Z80_A8  (1ULL<<8)
#define Z80_A9  (1ULL<<9)
#define Z80_A10 (1ULL<<10)
#define Z80_A11 (1ULL<<11)
#define Z80_A12 (1ULL<<12)
#define Z80_A13 (1ULL<<13)
#define Z80_A14 (1ULL<<14)
#define Z80_A15 (1ULL<<15)

/*--- data lines ------*/
#define Z80_D0  (1ULL<<16)
#define Z80_D1  (1ULL<<17)
#define Z80_D2  (1ULL<<18)
#define Z80_D3  (1ULL<<19)
#define Z80_D4  (1ULL<<20)
#define Z80_D5  (1ULL<<21)
#define Z80_D6  (1ULL<<22)
#define Z80_D7  (1ULL<<23)

/*--- control pins ---*/

/* system control pins */
#define  Z80_M1    (1ULL<<24)       /* machine cycle 1 */
#define  Z80_MREQ  (1ULL<<25)       /* memory request */
#define  Z80_IORQ  (1ULL<<26)       /* input/output request */
#define  Z80_RD    (1ULL<<27)       /* read */
#define  Z80_WR    (1ULL<<28)       /* write */
#define  Z80_CTRL_MASK (Z80_M1|Z80_MREQ|Z80_IORQ|Z80_RD|Z80_WR)

/* CPU control pins */
#define  Z80_HALT  (1ULL<<29)       /* halt state */
#define  Z80_INT   (1ULL<<30)       /* interrupt request */
#define  Z80_NMI   (1ULL<<31)       /* non-maskable interrupt */
#define  Z80_BUSREQ (1ULL<<32)      /* bus request */
#define  Z80_BUSACK (1ULL<<33)      /* bus acknowledge */

/* up to 7 wait states can be injected per machine cycle */
#define Z80_WAIT0   (1ULL<<34)
#define Z80_WAIT1   (1ULL<<35)
#define Z80_WAIT2   (1ULL<<36)
#define Z80_WAIT_SHIFT (34)
#define Z80_WAIT_MASK (Z80_WAIT0|Z80_WAIT1|Z80_WAIT2)

/* interrupt-related 'virtual pins', these don't exist on the Z80 */
#define Z80_IEIO    (1ULL<<37)      /* unified daisy chain 'Interrupt Enable In+Out' */
#define Z80_RETI    (1ULL<<38)      /* cpu has decoded a RETI instruction */

/* bit mask for all CPU bus pins */
#define Z80_PIN_MASK ((1ULL<<40)-1)

/*--- status indicator flags ---*/
#define Z80_CF (1<<0)           /* carry */
#define Z80_NF (1<<1)           /* add/subtract */
#define Z80_VF (1<<2)           /* parity/overflow */
#define Z80_PF Z80_VF
#define Z80_XF (1<<3)           /* undocumented bit 3 */
#define Z80_HF (1<<4)           /* half carry */
#define Z80_YF (1<<5)           /* undocumented bit 5 */
#define Z80_ZF (1<<6)           /* zero */
#define Z80_SF (1<<7)           /* sign */

/* initialization attributes */
typedef struct {
    z80_tick_t tick_cb;
    void* user_data;
} z80_desc_t;

/* state bits (z80_reg_t.bits) */
#define Z80_BIT_IX      (1<<0)      /* IX prefix active */
#define Z80_BIT_IY      (1<<1)      /* IY prefix active */
#define Z80_BIT_IFF1    (1<<2)      /* interrupt-enabled bit 1 */
#define Z80_BIT_IFF2    (1<<3)      /* interrupt-enabled bit 2 */
#define Z80_BIT_EI      (1<<4)      /* enable interrupts in next instruction */

/* Z80 register bank */
typedef struct {
    uint16_t pc;
    uint8_t a, f;
    uint8_t l, h;
    uint8_t e, d;
    uint8_t c, b;
    uint8_t il, ih;     /* during decoding, contains HL, IX or IY depending on current prefix */
    uint16_t wz;        /* internal memptr register */
    uint16_t sp;
    uint16_t ix;
    uint16_t iy;
    uint8_t i, r;       /* interrupt vector base and refresh register */
    uint16_t fa_;       /* shadow registers (AF', HL', ...) */
    uint16_t hl_;
    uint16_t de_;
    uint16_t bc_;
    uint8_t im;         /* interrupt mode (0, 1, 2) */
    uint8_t bits;       /* state bits (Z80_BIT_*) */
} z80_reg_t;

/* Z80 CPU state */
typedef struct {
    z80_tick_t tick_cb;     /* tick callback */
    z80_reg_t reg;          /* register bank */
    uint64_t pins;          /* last pin state (for debug inspection) */
    void* user_data;        /* userdata for tick callback */
    z80_trap_t trap_cb;     /* trap evaluation callback */
    void* trap_user_data;   /* userdata for trap callback */
    int trap_id;            /* != 0 if last z80_exec() hit a trap */
} z80_t;

/* initialize a new z80 instance */
void z80_init(z80_t* cpu, const z80_desc_t* desc);
/* reset an existing z80 instance */
void z80_reset(z80_t* cpu);
/* set optional trap callback function */
void z80_trap_cb(z80_t* cpu, z80_trap_t trap_cb, void* trap_user_data);
/* execute instructions for at least 'ticks', but at least one, return executed ticks */
uint32_t z80_exec(z80_t* cpu, uint32_t ticks);
/* return false if z80_exec() returned in the middle of an extended intruction */
bool z80_opdone(z80_t* cpu);

/* register access functions */
void z80_set_a(z80_t* cpu, uint8_t v);
void z80_set_f(z80_t* cpu, uint8_t v);
void z80_set_l(z80_t* cpu, uint8_t v);
void z80_set_h(z80_t* cpu, uint8_t v);
void z80_set_e(z80_t* cpu, uint8_t v);
void z80_set_d(z80_t* cpu, uint8_t v);
void z80_set_c(z80_t* cpu, uint8_t v);
void z80_set_b(z80_t* cpu, uint8_t v);
void z80_set_fa(z80_t* cpu, uint16_t v);
void z80_set_af(z80_t* cpi, uint16_t v);
void z80_set_hl(z80_t* cpu, uint16_t v);
void z80_set_de(z80_t* cpu, uint16_t v);
void z80_set_bc(z80_t* cpu, uint16_t v);
void z80_set_fa_(z80_t* cpu, uint16_t v);
void z80_set_af_(z80_t* cpi, uint16_t v);
void z80_set_hl_(z80_t* cpu, uint16_t v);
void z80_set_de_(z80_t* cpu, uint16_t v);
void z80_set_bc_(z80_t* cpu, uint16_t v);
void z80_set_pc(z80_t* cpu, uint16_t v);
void z80_set_wz(z80_t* cpu, uint16_t v);
void z80_set_sp(z80_t* cpu, uint16_t v);
void z80_set_i(z80_t* cpu, uint8_t v);
void z80_set_r(z80_t* cpu, uint8_t v);
void z80_set_ix(z80_t* cpu, uint16_t v);
void z80_set_iy(z80_t* cpu, uint16_t v);
void z80_set_im(z80_t* cpu, uint8_t v);
void z80_set_iff1(z80_t* cpu, bool b);
void z80_set_iff2(z80_t* cpu, bool b);
void z80_set_ei_pending(z80_t* cpu, bool b);

uint8_t z80_a(z80_t* cpu);
uint8_t z80_f(z80_t* cpu);
uint8_t z80_l(z80_t* cpu);
uint8_t z80_h(z80_t* cpu);
uint8_t z80_e(z80_t* cpu);
uint8_t z80_d(z80_t* cpu);
uint8_t z80_c(z80_t* cpu);
uint8_t z80_b(z80_t* cpu);
uint16_t z80_fa(z80_t* cpu);
uint16_t z80_af(z80_t* cpu);
uint16_t z80_hl(z80_t* cpu);
uint16_t z80_de(z80_t* cpu);
uint16_t z80_bc(z80_t* cpu);
uint16_t z80_fa_(z80_t* cpu);
uint16_t z80_af_(z80_t* cpu);
uint16_t z80_hl_(z80_t* cpu);
uint16_t z80_de_(z80_t* cpu);
uint16_t z80_bc_(z80_t* cpu);
uint16_t z80_pc(z80_t* cpu);
uint16_t z80_wz(z80_t* cpu);
uint16_t z80_sp(z80_t* cpu);
uint16_t z80_ir(z80_t* cpu);
uint8_t z80_i(z80_t* cpu);
uint8_t z80_r(z80_t* cpu);
uint16_t z80_ix(z80_t* cpu);
uint16_t z80_iy(z80_t* cpu);
uint8_t z80_im(z80_t* cpu);
bool z80_iff1(z80_t* cpu);
bool z80_iff2(z80_t* cpu);
bool z80_ei_pending(z80_t* cpu);

/* helper macro to start interrupt handling in tick callback */
#define Z80_DAISYCHAIN_BEGIN(pins) if (pins&Z80_M1) { pins|=Z80_IEIO;
/* helper macro to end interrupt handling in tick callback */
#define Z80_DAISYCHAIN_END(pins) pins&=~Z80_RETI; }
/* return a pin mask with control-pins, address and data bus */
#define Z80_MAKE_PINS(ctrl, addr, data) ((ctrl)|(((data)<<16)&0xFF0000ULL)|((addr)&0xFFFFULL))
/* extract 16-bit address bus from 64-bit pins */
#define Z80_GET_ADDR(p) ((uint16_t)(p&0xFFFFULL))
/* merge 16-bit address bus value into 64-bit pins */
#define Z80_SET_ADDR(p,a) {p=((p&~0xFFFFULL)|((a)&0xFFFFULL));}
/* extract 8-bit data bus from 64-bit pins */
#define Z80_GET_DATA(p) ((uint8_t)((p&0xFF0000ULL)>>16))
/* merge 8-bit data bus value into 64-bit pins */
#define Z80_SET_DATA(p,d) {p=((p&~0xFF0000ULL)|(((d)<<16)&0xFF0000ULL));}
/* extract number of wait states from pin mask */
#define Z80_GET_WAIT(p) ((p&Z80_WAIT_MASK)>>Z80_WAIT_SHIFT)
/* set up to 7 wait states in pin mask */
#define Z80_SET_WAIT(p,w) {p=((p&~Z80_WAIT_MASK)|((((uint64_t)w)<<Z80_WAIT_SHIFT)&Z80_WAIT_MASK));}

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
void z80_set_a(z80_t* cpu, uint8_t v)         { cpu->reg.a = v; }
void z80_set_f(z80_t* cpu, uint8_t v)         { cpu->reg.f = v; }
void z80_set_l(z80_t* cpu, uint8_t v)         { cpu->reg.l = v; }
void z80_set_h(z80_t* cpu, uint8_t v)         { cpu->reg.h = v; }
void z80_set_e(z80_t* cpu, uint8_t v)         { cpu->reg.e = v; }
void z80_set_d(z80_t* cpu, uint8_t v)         { cpu->reg.d = v; }
void z80_set_c(z80_t* cpu, uint8_t v)         { cpu->reg.c = v; }
void z80_set_b(z80_t* cpu, uint8_t v)         { cpu->reg.b = v; }
void z80_set_af(z80_t* cpu, uint16_t v)       { cpu->reg.f = v; cpu->reg.a = v>>8; }
void z80_set_fa(z80_t* cpu, uint16_t v)       { cpu->reg.f = v>>8; cpu->reg.a = v; }
void z80_set_hl(z80_t* cpu, uint16_t v)       { cpu->reg.l = v; cpu->reg.h = v>>8; }
void z80_set_de(z80_t* cpu, uint16_t v)       { cpu->reg.e = v; cpu->reg.d = v>>8; }
void z80_set_bc(z80_t* cpu, uint16_t v)       { cpu->reg.c = v; cpu->reg.b = v>>8; }
void z80_set_fa_(z80_t* cpu, uint16_t v)      { cpu->reg.fa_ = v; };
void z80_set_af_(z80_t* cpu, uint16_t v)      { cpu->reg.fa_ = (v<<8)|(v>>8); }
void z80_set_hl_(z80_t* cpu, uint16_t v)      { cpu->reg.hl_ = v; }
void z80_set_de_(z80_t* cpu, uint16_t v)      { cpu->reg.de_ = v; }
void z80_set_bc_(z80_t* cpu, uint16_t v)      { cpu->reg.bc_ = v; }
void z80_set_sp(z80_t* cpu, uint16_t v)       { cpu->reg.sp = v; }
void z80_set_iy(z80_t* cpu, uint16_t v)       { cpu->reg.iy = v; }
void z80_set_ix(z80_t* cpu, uint16_t v)       { cpu->reg.ix = v; }
void z80_set_wz(z80_t* cpu, uint16_t v)       { cpu->reg.wz = v; }
void z80_set_pc(z80_t* cpu, uint16_t v)       { cpu->reg.pc = v; }
void z80_set_ir(z80_t* cpu, uint16_t v)       { cpu->reg.i = v>>8; cpu->reg.r = v; }
void z80_set_i(z80_t* cpu, uint8_t v)         { cpu->reg.i = v; }
void z80_set_r(z80_t* cpu, uint8_t v)         { cpu->reg.r = v; }
void z80_set_im(z80_t* cpu, uint8_t v)        { cpu->reg.im = v; }
void z80_set_iff1(z80_t* cpu, bool b)         { if (b) { cpu->reg.bits |= Z80_BIT_IFF1; } else { cpu->reg.bits &= ~Z80_BIT_IFF1; } }
void z80_set_iff2(z80_t* cpu, bool b)         { if (b) { cpu->reg.bits |= Z80_BIT_IFF2; } else { cpu->reg.bits &= ~Z80_BIT_IFF2; } }
void z80_set_ei_pending(z80_t* cpu, bool b)   { if (b) { cpu->reg.bits |= Z80_BIT_EI; } else { cpu->reg.bits &= ~Z80_BIT_EI; } }
uint8_t z80_a(z80_t* cpu)         { return cpu->reg.a; }
uint8_t z80_f(z80_t* cpu)         { return cpu->reg.f; }
uint8_t z80_l(z80_t* cpu)         { return cpu->reg.l; }
uint8_t z80_h(z80_t* cpu)         { return cpu->reg.h; }
uint8_t z80_e(z80_t* cpu)         { return cpu->reg.e; }
uint8_t z80_d(z80_t* cpu)         { return cpu->reg.d; }
uint8_t z80_c(z80_t* cpu)         { return cpu->reg.c; }
uint8_t z80_b(z80_t* cpu)         { return cpu->reg.b; }
uint16_t z80_fa(z80_t* cpu)       { return (cpu->reg.f<<8)|cpu->reg.a; }
uint16_t z80_af(z80_t* cpu)       { return (cpu->reg.a<<8)|cpu->reg.f; }
uint16_t z80_hl(z80_t* cpu)       { return (cpu->reg.h<<8)|cpu->reg.l; }
uint16_t z80_de(z80_t* cpu)       { return (cpu->reg.d<<8)|cpu->reg.e; }
uint16_t z80_bc(z80_t* cpu)       { return (cpu->reg.b<<8)|cpu->reg.c; }
uint16_t z80_fa_(z80_t* cpu)      { return cpu->reg.fa_; }
uint16_t z80_af_(z80_t* cpu)      { return (cpu->reg.fa_<<8)|(cpu->reg.fa_>>8); }
uint16_t z80_hl_(z80_t* cpu)      { return cpu->reg.hl_; }
uint16_t z80_de_(z80_t* cpu)      { return cpu->reg.de_; }
uint16_t z80_bc_(z80_t* cpu)      { return cpu->reg.bc_; }
uint16_t z80_sp(z80_t* cpu)       { return cpu->reg.sp; }
uint16_t z80_iy(z80_t* cpu)       { return cpu->reg.iy; }
uint16_t z80_ix(z80_t* cpu)       { return cpu->reg.ix; }
uint16_t z80_wz(z80_t* cpu)       { return cpu->reg.wz; }
uint16_t z80_pc(z80_t* cpu)       { return cpu->reg.pc; }
uint16_t z80_ir(z80_t* cpu)       { return (cpu->reg.i<<8)|cpu->reg.r; }
uint8_t z80_i(z80_t* cpu)         { return cpu->reg.i; }
uint8_t z80_r(z80_t* cpu)         { return cpu->reg.r; }
uint8_t z80_im(z80_t* cpu)        { return cpu->reg.im; }
bool z80_iff1(z80_t* cpu)         { return 0 != (cpu->reg.bits & Z80_BIT_IFF1); }
bool z80_iff2(z80_t* cpu)         { return 0 != (cpu->reg.bits & Z80_BIT_IFF2); }
bool z80_ei_pending(z80_t* cpu)   { return 0 != (cpu->reg.bits & Z80_BIT_EI); }

void z80_init(z80_t* cpu, const z80_desc_t* desc) {
    CHIPS_ASSERT(cpu && desc);
    CHIPS_ASSERT(desc->tick_cb);
    memset(cpu, 0, sizeof(*cpu));
    z80_reset(cpu);
    cpu->tick_cb = desc->tick_cb;
    cpu->user_data = desc->user_data;
}

void z80_reset(z80_t* cpu) {
    CHIPS_ASSERT(cpu);
    /* set AF to 0xFFFF, all other regs are undefined, set to 0xFFFF to */
    z80_set_bc(cpu, 0xFFFF); z80_set_bc_(cpu, 0xFFFF);
    z80_set_de(cpu, 0xFFFF); z80_set_de_(cpu, 0xFFFF);
    z80_set_hl(cpu, 0xFFFF); z80_set_hl_(cpu, 0xFFFF);
    z80_set_af(cpu, 0xFFFF); z80_set_af_(cpu, 0xFFFF);
    z80_set_ix(cpu, 0xFFFF);
    z80_set_iy(cpu, 0xFFFF);
    z80_set_wz(cpu, 0xFFFF);
    /* set SP to 0xFFFF, PC to 0x0000 */
    z80_set_sp(cpu, 0xFFFF);
    z80_set_pc(cpu, 0x0000);
    /* IFF1 and IFF2 are off */
    z80_set_iff1(cpu, false);
    z80_set_iff2(cpu, false);
    /* IM is set to 0 */
    z80_set_im(cpu, 0);
    /* after power-on or reset, R is set to 0 (see z80-documented.pdf) */
    z80_set_ir(cpu, 0x0000);
    cpu->reg.bits &= ~(Z80_BIT_EI|Z80_BIT_IX|Z80_BIT_IY);
}

void z80_trap_cb(z80_t* cpu, z80_trap_t trap_cb, void* trap_user_data) {
    CHIPS_ASSERT(cpu);
    cpu->trap_cb = trap_cb;
    cpu->trap_user_data = trap_user_data;
}

bool z80_opdone(z80_t* cpu) {
    return 0 == (cpu->reg.bits & (Z80_BIT_IX|Z80_BIT_IY));
}

/* sign+zero+parity lookup table */
static uint8_t _z80_szp[256] = {
  0x44,0x00,0x00,0x04,0x00,0x04,0x04,0x00,0x08,0x0c,0x0c,0x08,0x0c,0x08,0x08,0x0c,
  0x00,0x04,0x04,0x00,0x04,0x00,0x00,0x04,0x0c,0x08,0x08,0x0c,0x08,0x0c,0x0c,0x08,
  0x20,0x24,0x24,0x20,0x24,0x20,0x20,0x24,0x2c,0x28,0x28,0x2c,0x28,0x2c,0x2c,0x28,
  0x24,0x20,0x20,0x24,0x20,0x24,0x24,0x20,0x28,0x2c,0x2c,0x28,0x2c,0x28,0x28,0x2c,
  0x00,0x04,0x04,0x00,0x04,0x00,0x00,0x04,0x0c,0x08,0x08,0x0c,0x08,0x0c,0x0c,0x08,
  0x04,0x00,0x00,0x04,0x00,0x04,0x04,0x00,0x08,0x0c,0x0c,0x08,0x0c,0x08,0x08,0x0c,
  0x24,0x20,0x20,0x24,0x20,0x24,0x24,0x20,0x28,0x2c,0x2c,0x28,0x2c,0x28,0x28,0x2c,
  0x20,0x24,0x24,0x20,0x24,0x20,0x20,0x24,0x2c,0x28,0x28,0x2c,0x28,0x2c,0x2c,0x28,
  0x80,0x84,0x84,0x80,0x84,0x80,0x80,0x84,0x8c,0x88,0x88,0x8c,0x88,0x8c,0x8c,0x88,
  0x84,0x80,0x80,0x84,0x80,0x84,0x84,0x80,0x88,0x8c,0x8c,0x88,0x8c,0x88,0x88,0x8c,
  0xa4,0xa0,0xa0,0xa4,0xa0,0xa4,0xa4,0xa0,0xa8,0xac,0xac,0xa8,0xac,0xa8,0xa8,0xac,
  0xa0,0xa4,0xa4,0xa0,0xa4,0xa0,0xa0,0xa4,0xac,0xa8,0xa8,0xac,0xa8,0xac,0xac,0xa8,
  0x84,0x80,0x80,0x84,0x80,0x84,0x84,0x80,0x88,0x8c,0x8c,0x88,0x8c,0x88,0x88,0x8c,
  0x80,0x84,0x84,0x80,0x84,0x80,0x80,0x84,0x8c,0x88,0x88,0x8c,0x88,0x8c,0x8c,0x88,
  0xa0,0xa4,0xa4,0xa0,0xa4,0xa0,0xa0,0xa4,0xac,0xa8,0xa8,0xac,0xa8,0xac,0xac,0xa8,
  0xa4,0xa0,0xa0,0xa4,0xa0,0xa4,0xa4,0xa0,0xa8,0xac,0xac,0xa8,0xac,0xa8,0xa8,0xac,
};

/* load the virtual IHL register pair with IX, IY or HL */
static inline void _z80_load_ihl(z80_reg_t* c, uint8_t map_bits) {
    if (map_bits & Z80_BIT_IX) {
        c->il=c->ix; c->ih=c->ix>>8;
    }
    else if (map_bits & Z80_BIT_IY) {
        c->il=c->iy; c->ih=c->iy>>8;
    }
    else {
        c->il=c->l; c->ih=c->h;
    }
}

/* flush the virtual IHL register back to IX, IY or HL */
static inline void _z80_flush_ihl(z80_reg_t* c, uint8_t map_bits) {
    if (map_bits & Z80_BIT_IX) {
        c->ix=(c->ih<<8)|c->il;
    }
    else if (map_bits & Z80_BIT_IY) {
        c->iy=(c->ih<<8)|c->il;
    }
    else {
        c->l=c->il; c->h=c->ih;
    }
}

/* implementation for the DAA instruction */
static inline void _z80_daa(z80_reg_t* c) {
    uint8_t v = c->a;
    if (c->f & Z80_NF) {
        if (((c->a & 0xF)>0x9) || (c->f & Z80_HF)) {
            v -= 0x06;
        }
        if ((c->a > 0x99) || (c->f & Z80_CF)) {
            v -= 0x60;
        }
    }
    else {
        if (((c->a & 0xF)>0x9) || (c->f & Z80_HF)) {
            v += 0x06;
        }
        if ((c->a > 0x99) || (c->f & Z80_CF)) {
            v += 0x60;
        }
    }
    c->f &= Z80_CF|Z80_NF;
    c->f |= (c->a>0x99) ? Z80_CF : 0;
    c->f |= (c->a ^ v) & Z80_HF;
    c->f |= _z80_szp[v];
    c->a = v;
}

/* set 16-bit address bus pins */
#define _SA(addr) pins=(pins&~0xFFFFULL)|((addr)&0xFFFFULL)
/* set 16-bit address bus and 8-bit data bus pins */
#define _SAD(addr,data) pins=(pins&~0xFFFFFFULL)|((((data)&0xFFULL)<<16)&0xFF0000ULL)|((addr)&0xFFFFULL)
/* get 8-bit data bus value from pins */
#define _GD() ((uint8_t)((pins&0xFF0000ULL)>>16))
/* invoke 'filler tick' without control pins set */
#define _T(num) pins=tick(num,(pins&~Z80_CTRL_MASK),ud);ticks+=num
/* invoke tick callback with pins mask */
#define _TM(num,mask) pins=tick(num,(pins&~(Z80_CTRL_MASK))|(mask),ud);ticks+=num
/* invoke tick callback (with wait state detecion) */
#define _TWM(num,mask) pins=tick(num,(pins&~(Z80_WAIT_MASK|Z80_CTRL_MASK))|(mask),ud);ticks+=num+Z80_GET_WAIT(pins)
/* memory read machine cycle */
#define _MR(addr,data) _SA(addr);_TWM(3,Z80_MREQ|Z80_RD);data=_GD()
/* memory write machine cycle */
#define _MW(addr,data) _SAD(addr,data);_TWM(3,Z80_MREQ|Z80_WR)
/* input machine cycle */
#define _IN(addr,data) _SA(addr);_TWM(4,Z80_IORQ|Z80_RD);data=_GD()
/* output machine cycle */
#define _OUT(addr,data) _SAD(addr,data);_TWM(4,Z80_IORQ|Z80_WR);
/* read 8-bit immediate value */
#define _IMM8(data) _MR(c.pc++,data);
/* read 16-bit immediate value (also update WZ register) */
#define _IMM16(data) {uint8_t w,z;_MR(c.pc++,z);_MR(c.pc++,w);data=c.wz=(w<<8)|z;} 
/* true if current op is an indexed op */
#define _IDX() (0!=(c.bits&(Z80_BIT_IX|Z80_BIT_IY)))
/* generate effective address for (HL), (IX+d), (IY+d) */
#define _ADDR(ext_ticks) {addr=(c.ih<<8)|c.il;if(_IDX()){int8_t d;_MR(c.pc++,d);addr+=d;c.wz=addr;_T(ext_ticks);}}
/* helper macro to bump R register */
#define _BUMPR() c.r=(c.r&0x80)|((c.r+1)&0x7F);
/* a normal opcode fetch, bump R */
#define _FETCH(op) {_SA(c.pc++);_TWM(4,Z80_M1|Z80_MREQ|Z80_RD);op=_GD();_BUMPR();}
/* special opcode fetch for CB prefix, only bump R if not a DD/FD+CB 'double prefix' op */
#define _FETCH_CB(op) {_SA(c.pc++);_TWM(4,Z80_M1|Z80_MREQ|Z80_RD);op=_GD();if(!_IDX()){_BUMPR();}}
/* evaluate S+Z flags */
#define _SZ(val) ((val&0xFF)?(val&Z80_SF):Z80_ZF)
/* evaluate SZYXCH flags */
#define _SZYXCH(acc,val,res) (_SZ(res)|(res&(Z80_YF|Z80_XF))|((res>>8)&Z80_CF)|((acc^val^res)&Z80_HF))
/* evaluate flags for 8-bit adds */
#define _ADD_FLAGS(acc,val,res) (_SZYXCH(acc,val,res)|((((val^acc^0x80)&(val^res))>>5)&Z80_VF))
/* evaluate flags for 8-bit subs */
#define _SUB_FLAGS(acc,val,res) (Z80_NF|_SZYXCH(acc,val,res)|((((val^acc)&(res^acc))>>5)&Z80_VF))
/* evaluate flags for 8-bit compare */
#define _CP_FLAGS(acc,val,res) (Z80_NF|(_SZ(res)|(val&(Z80_YF|Z80_XF))|((res>>8)&Z80_CF)|((acc^val^res)&Z80_HF))|((((val^acc)&(res^acc))>>5)&Z80_VF))
/* evaluate flags for LD A,I and LD A,R */
#define _SZIFF2_FLAGS(val) ((c.f&Z80_CF)|_SZ(val)|(val&(Z80_YF|Z80_XF))|((c.bits&Z80_BIT_IFF2)?Z80_PF:0))

uint32_t z80_exec(z80_t* cpu, uint32_t num_ticks) {
    cpu->trap_id = 0;
    z80_reg_t c = cpu->reg;
    _z80_load_ihl(&c, c.bits);
    uint8_t map_bits = c.bits & (Z80_BIT_IX|Z80_BIT_IY);
    uint64_t pins = cpu->pins;
    uint64_t pre_pins = pins;
    const z80_tick_t tick = cpu->tick_cb;
    const z80_trap_t trap = cpu->trap_cb;
    void* ud = cpu->user_data;
    uint32_t ticks = 0;
    uint16_t addr = 0;
    uint16_t d16 = 0;
    uint8_t d8 = 0;
    uint8_t op = 0;
    do {
        _FETCH(op);
        /* ED prefix cancels DD and FD prefix */
        if (op == 0xED) {
            map_bits &= ~(Z80_BIT_IX|Z80_BIT_IY);
        }
        /* IX/IY <=> HL mapping for DD/FD prefixed ops */
        if (map_bits != (c.bits & (Z80_BIT_IX|Z80_BIT_IY))) {
            _z80_flush_ihl(&c, c.bits);
            _z80_load_ihl(&c, map_bits);
            c.bits = (c.bits & ~(Z80_BIT_IX|Z80_BIT_IY)) | map_bits;
        }
        /* code-generated instruction decoder */
        switch (op) {
            case 0x0:/*NOP*/ break;
            case 0x1:/*LD BC,nn*/_IMM16(d16);c.b=d16>>8;c.c=d16;break;
            case 0x2:/*LD (BC),A*/addr=(c.b<<8)|c.c;_MW(addr++,c.a);c.wz=(c.a<<8)|(addr&0x00FF);break;
            case 0x3:/*INC BC*/_T(2);d16=(c.b<<8)|c.c;d16++;c.b=d16>>8;c.c=d16;break;
            case 0x4:/*INC B*/d8=c.b;{uint8_t r=d8+1;c.f&=Z80_CF;c.f|=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x80){c.f|=Z80_VF;}d8=r;}c.b=d8;break;
            case 0x5:/*DEC B*/d8=c.b;{uint8_t r=d8-1;c.f&=Z80_CF;c.f|=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x7F){c.f|=Z80_VF;}d8=r;}c.b=d8;break;
            case 0x6:/*LD B,n*/_IMM8(c.b);break;
            case 0x7:/*RLCA*/{uint8_t r=(c.a<<1)|(c.a>>7);c.f=((c.a>>7)&Z80_CF)|(c.f&(Z80_SF|Z80_ZF|Z80_PF))|(r&(Z80_YF|Z80_XF));c.a=r;}break;
            case 0x8:/*EX AF,AF'*/{uint16_t fa=(c.f<<8)|c.a;c.f=c.fa_>>8;c.a=c.fa_;c.fa_=fa;}break;
            case 0x9:/*ADD HL/IX/IY,BC*/{uint16_t acc=(c.ih<<8)|c.il;c.wz=acc+1;d16=(c.b<<8)|c.c;uint32_t r=acc+d16;c.ih=r>>8;c.il=r;c.f&=(Z80_SF|Z80_ZF|Z80_VF);c.f|=((acc^r^d16)>>8)&Z80_HF;c.f|=((r>>16)&Z80_CF)|((r>>8)&(Z80_YF|Z80_XF));_T(7);}break;
            case 0xa:/*LD A,(BC)*/addr=(c.b<<8)|c.c;_MR(addr++,c.a);c.wz=addr;break;
            case 0xb:/*DEC BC*/_T(2);d16=(c.b<<8)|c.c;d16--;c.b=d16>>8;c.c=d16;break;
            case 0xc:/*INC C*/d8=c.c;{uint8_t r=d8+1;c.f&=Z80_CF;c.f|=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x80){c.f|=Z80_VF;}d8=r;}c.c=d8;break;
            case 0xd:/*DEC C*/d8=c.c;{uint8_t r=d8-1;c.f&=Z80_CF;c.f|=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x7F){c.f|=Z80_VF;}d8=r;}c.c=d8;break;
            case 0xe:/*LD C,n*/_IMM8(c.c);break;
            case 0xf:/*RRCA*/{uint8_t r=(c.a>>1)|(c.a<<7);c.f=(c.a&Z80_CF)|(c.f&(Z80_SF|Z80_ZF|Z80_PF))|(r&(Z80_YF|Z80_XF));c.a=r;}break;
            case 0x10:/*DJNZ*/{_T(1);int8_t d;_IMM8(d);if(--c.b>0){c.pc+=d;c.wz=c.pc;_T(5);}}break;
            case 0x11:/*LD DE,nn*/_IMM16(d16);c.d=d16>>8;c.e=d16;break;
            case 0x12:/*LD (DE),A*/addr=(c.d<<8)|c.e;_MW(addr++,c.a);c.wz=(c.a<<8)|(addr&0x00FF);break;
            case 0x13:/*INC DE*/_T(2);d16=(c.d<<8)|c.e;d16++;c.d=d16>>8;c.e=d16;break;
            case 0x14:/*INC D*/d8=c.d;{uint8_t r=d8+1;c.f&=Z80_CF;c.f|=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x80){c.f|=Z80_VF;}d8=r;}c.d=d8;break;
            case 0x15:/*DEC D*/d8=c.d;{uint8_t r=d8-1;c.f&=Z80_CF;c.f|=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x7F){c.f|=Z80_VF;}d8=r;}c.d=d8;break;
            case 0x16:/*LD D,n*/_IMM8(c.d);break;
            case 0x17:/*RLA*/{uint8_t r=(c.a<<1)|(c.f&Z80_CF);c.f=((c.a>>7)&Z80_CF)|(c.f&(Z80_SF|Z80_ZF|Z80_PF))|(r&(Z80_YF|Z80_XF));c.a=r;}break;
            case 0x18:/*JR d*/{int8_t d;_IMM8(d);c.pc+=d;c.wz=c.pc;_T(5);}break;
            case 0x19:/*ADD HL/IX/IY,DE*/{uint16_t acc=(c.ih<<8)|c.il;c.wz=acc+1;d16=(c.d<<8)|c.e;uint32_t r=acc+d16;c.ih=r>>8;c.il=r;c.f&=(Z80_SF|Z80_ZF|Z80_VF);c.f|=((acc^r^d16)>>8)&Z80_HF;c.f|=((r>>16)&Z80_CF)|((r>>8)&(Z80_YF|Z80_XF));_T(7);}break;
            case 0x1a:/*LD A,(DE)*/addr=(c.d<<8)|c.e;_MR(addr++,c.a);c.wz=addr;break;
            case 0x1b:/*DEC DE*/_T(2);d16=(c.d<<8)|c.e;d16--;c.d=d16>>8;c.e=d16;break;
            case 0x1c:/*INC E*/d8=c.e;{uint8_t r=d8+1;c.f&=Z80_CF;c.f|=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x80){c.f|=Z80_VF;}d8=r;}c.e=d8;break;
            case 0x1d:/*DEC E*/d8=c.e;{uint8_t r=d8-1;c.f&=Z80_CF;c.f|=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x7F){c.f|=Z80_VF;}d8=r;}c.e=d8;break;
            case 0x1e:/*LD E,n*/_IMM8(c.e);break;
            case 0x1f:/*RRA*/{uint8_t r=(c.a>>1)|((c.f&Z80_CF)<<7);c.f=(c.a&Z80_CF)|(c.f&(Z80_SF|Z80_ZF|Z80_PF))|(r&(Z80_YF|Z80_XF));c.a=r;}break;
            case 0x20:/*JR NZ,d*/{int8_t d;_IMM8(d);if(!(c.f&Z80_ZF)){c.pc+=d;c.wz=c.pc;_T(5);}}break;
            case 0x21:/*LD HL/IX/IY,nn*/_IMM16(d16);c.ih=d16>>8;c.il=d16;break;
            case 0x22:/*LD (nn),HL/IX/IY*/_IMM16(addr);_MW(addr++,c.il);_MW(addr,c.ih);c.wz=addr;break;
            case 0x23:/*INC HL/IX/IY*/_T(2);d16=(c.ih<<8)|c.il;d16++;c.ih=d16>>8;c.il=d16;break;
            case 0x24:/*INC H*/d8=c.ih;{uint8_t r=d8+1;c.f&=Z80_CF;c.f|=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x80){c.f|=Z80_VF;}d8=r;}c.ih=d8;break;
            case 0x25:/*DEC H*/d8=c.ih;{uint8_t r=d8-1;c.f&=Z80_CF;c.f|=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x7F){c.f|=Z80_VF;}d8=r;}c.ih=d8;break;
            case 0x26:/*LD H,n*/_IMM8(c.ih);break;
            case 0x27:/*DAA*/_z80_daa(&c);break;
            case 0x28:/*JR Z,d*/{int8_t d;_IMM8(d);if((c.f&Z80_ZF)){c.pc+=d;c.wz=c.pc;_T(5);}}break;
            case 0x29:/*ADD HL/IX/IY,HL/IX/IY*/{uint16_t acc=(c.ih<<8)|c.il;c.wz=acc+1;d16=(c.ih<<8)|c.il;uint32_t r=acc+d16;c.ih=r>>8;c.il=r;c.f&=(Z80_SF|Z80_ZF|Z80_VF);c.f|=((acc^r^d16)>>8)&Z80_HF;c.f|=((r>>16)&Z80_CF)|((r>>8)&(Z80_YF|Z80_XF));_T(7);}break;
            case 0x2a:/*LD HL/IX/IY,(nn)*/_IMM16(addr);_MR(addr++,c.il);_MR(addr,c.ih);c.wz=addr;break;
            case 0x2b:/*DEC HL/IX/IY*/_T(2);d16=(c.ih<<8)|c.il;d16--;c.ih=d16>>8;c.il=d16;break;
            case 0x2c:/*INC L*/d8=c.il;{uint8_t r=d8+1;c.f&=Z80_CF;c.f|=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x80){c.f|=Z80_VF;}d8=r;}c.il=d8;break;
            case 0x2d:/*DEC L*/d8=c.il;{uint8_t r=d8-1;c.f&=Z80_CF;c.f|=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x7F){c.f|=Z80_VF;}d8=r;}c.il=d8;break;
            case 0x2e:/*LD L,n*/_IMM8(c.il);break;
            case 0x2f:/*CPL*/c.a^=0xFF;c.f=(c.f&(Z80_SF|Z80_ZF|Z80_PF|Z80_CF))|Z80_HF|Z80_NF|(c.a&(Z80_YF|Z80_XF));break;
            case 0x30:/*JR NC,d*/{int8_t d;_IMM8(d);if(!(c.f&Z80_CF)){c.pc+=d;c.wz=c.pc;_T(5);}}break;
            case 0x31:/*LD SP,nn*/_IMM16(d16);c.sp=d16;break;
            case 0x32:/*LD (nn),A*/_IMM16(addr);_MW(addr++,c.a);c.wz=(c.a<<8)|(addr&0x00FF);break;
            case 0x33:/*INC SP*/_T(2);d16=c.sp;d16++;c.sp=d16;break;
            case 0x34:/*INC (HL/IX+d/IY+d)*/_ADDR(5);_T(1);_MR(addr,d8);{uint8_t r=d8+1;c.f&=Z80_CF;c.f|=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x80){c.f|=Z80_VF;}d8=r;}_MW(addr,d8);break;
            case 0x35:/*DEC (HL/IX+d/IY+d)*/_ADDR(5);_T(1);_MR(addr,d8);{uint8_t r=d8-1;c.f&=Z80_CF;c.f|=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x7F){c.f|=Z80_VF;}d8=r;}_MW(addr,d8);break;
            case 0x36:/*LD (HL/IX+d/IY+d),n*/_ADDR(2);_IMM8(d8);_MW(addr,d8);break;
            case 0x37:/*SCF*/c.f=(c.f&(Z80_SF|Z80_ZF|Z80_PF|Z80_CF))|Z80_CF|(c.a&(Z80_YF|Z80_XF));break;
            case 0x38:/*JR C,d*/{int8_t d;_IMM8(d);if((c.f&Z80_CF)){c.pc+=d;c.wz=c.pc;_T(5);}}break;
            case 0x39:/*ADD HL/IX/IY,SP*/{uint16_t acc=(c.ih<<8)|c.il;c.wz=acc+1;d16=c.sp;uint32_t r=acc+d16;c.ih=r>>8;c.il=r;c.f&=(Z80_SF|Z80_ZF|Z80_VF);c.f|=((acc^r^d16)>>8)&Z80_HF;c.f|=((r>>16)&Z80_CF)|((r>>8)&(Z80_YF|Z80_XF));_T(7);}break;
            case 0x3a:/*LD A,(nn)*/_IMM16(addr);_MR(addr++,c.a);c.wz=addr;break;
            case 0x3b:/*DEC SP*/_T(2);d16=c.sp;d16--;c.sp=d16;break;
            case 0x3c:/*INC A*/d8=c.a;{uint8_t r=d8+1;c.f&=Z80_CF;c.f|=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x80){c.f|=Z80_VF;}d8=r;}c.a=d8;break;
            case 0x3d:/*DEC A*/d8=c.a;{uint8_t r=d8-1;c.f&=Z80_CF;c.f|=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x7F){c.f|=Z80_VF;}d8=r;}c.a=d8;break;
            case 0x3e:/*LD A,n*/_IMM8(c.a);break;
            case 0x3f:/*CCF*/c.f=((c.f&(Z80_SF|Z80_ZF|Z80_PF|Z80_CF))|((c.f&Z80_CF)<<4)|(c.a&(Z80_YF|Z80_XF)))^Z80_CF;break;
            case 0x40:/*LD B,B*/c.b=c.b;break;
            case 0x41:/*LD B,C*/c.b=c.c;break;
            case 0x42:/*LD B,D*/c.b=c.d;break;
            case 0x43:/*LD B,E*/c.b=c.e;break;
            case 0x44:/*LD B,H*/c.b=c.ih;break;
            case 0x45:/*LD B,L*/c.b=c.il;break;
            case 0x46:/*LD B,(HL/IX+d/IY+d)*/_ADDR(5);_MR(addr,c.b);break;
            case 0x47:/*LD B,A*/c.b=c.a;break;
            case 0x48:/*LD C,B*/c.c=c.b;break;
            case 0x49:/*LD C,C*/c.c=c.c;break;
            case 0x4a:/*LD C,D*/c.c=c.d;break;
            case 0x4b:/*LD C,E*/c.c=c.e;break;
            case 0x4c:/*LD C,H*/c.c=c.ih;break;
            case 0x4d:/*LD C,L*/c.c=c.il;break;
            case 0x4e:/*LD C,(HL/IX+d/IY+d)*/_ADDR(5);_MR(addr,c.c);break;
            case 0x4f:/*LD C,A*/c.c=c.a;break;
            case 0x50:/*LD D,B*/c.d=c.b;break;
            case 0x51:/*LD D,C*/c.d=c.c;break;
            case 0x52:/*LD D,D*/c.d=c.d;break;
            case 0x53:/*LD D,E*/c.d=c.e;break;
            case 0x54:/*LD D,H*/c.d=c.ih;break;
            case 0x55:/*LD D,L*/c.d=c.il;break;
            case 0x56:/*LD D,(HL/IX+d/IY+d)*/_ADDR(5);_MR(addr,c.d);break;
            case 0x57:/*LD D,A*/c.d=c.a;break;
            case 0x58:/*LD E,B*/c.e=c.b;break;
            case 0x59:/*LD E,C*/c.e=c.c;break;
            case 0x5a:/*LD E,D*/c.e=c.d;break;
            case 0x5b:/*LD E,E*/c.e=c.e;break;
            case 0x5c:/*LD E,H*/c.e=c.ih;break;
            case 0x5d:/*LD E,L*/c.e=c.il;break;
            case 0x5e:/*LD E,(HL/IX+d/IY+d)*/_ADDR(5);_MR(addr,c.e);break;
            case 0x5f:/*LD E,A*/c.e=c.a;break;
            case 0x60:/*LD H,B*/c.ih=c.b;break;
            case 0x61:/*LD H,C*/c.ih=c.c;break;
            case 0x62:/*LD H,D*/c.ih=c.d;break;
            case 0x63:/*LD H,E*/c.ih=c.e;break;
            case 0x64:/*LD H,H*/c.ih=c.ih;break;
            case 0x65:/*LD H,L*/c.ih=c.il;break;
            case 0x66:/*LD H,(HL/IX+d/IY+d)*/_ADDR(5);if(_IDX()){_MR(addr,c.h);}else{_MR(addr,c.ih);}break;
            case 0x67:/*LD H,A*/c.ih=c.a;break;
            case 0x68:/*LD L,B*/c.il=c.b;break;
            case 0x69:/*LD L,C*/c.il=c.c;break;
            case 0x6a:/*LD L,D*/c.il=c.d;break;
            case 0x6b:/*LD L,E*/c.il=c.e;break;
            case 0x6c:/*LD L,H*/c.il=c.ih;break;
            case 0x6d:/*LD L,L*/c.il=c.il;break;
            case 0x6e:/*LD L,(HL/IX+d/IY+d)*/_ADDR(5);if(_IDX()){_MR(addr,c.l);}else{_MR(addr,c.il);}break;
            case 0x6f:/*LD L,A*/c.il=c.a;break;
            case 0x70:/*LD (HL/IX+d/IY+d),B*/_ADDR(5);_MW(addr,c.b);break;
            case 0x71:/*LD (HL/IX+d/IY+d),C*/_ADDR(5);_MW(addr,c.c);break;
            case 0x72:/*LD (HL/IX+d/IY+d),D*/_ADDR(5);_MW(addr,c.d);break;
            case 0x73:/*LD (HL/IX+d/IY+d),E*/_ADDR(5);_MW(addr,c.e);break;
            case 0x74:/*LD (HL/IX+d/IY+d),H*/_ADDR(5);if(_IDX()){_MW(addr,c.h);}else{_MW(addr,c.ih);}break;
            case 0x75:/*LD (HL/IX+d/IY+d),L*/_ADDR(5);if(_IDX()){_MW(addr,c.l);}else{_MW(addr,c.il);}break;
            case 0x76:/*HALT*/pins|=Z80_HALT;c.pc--;break;
            case 0x77:/*LD (HL/IX+d/IY+d),A*/_ADDR(5);_MW(addr,c.a);break;
            case 0x78:/*LD A,B*/c.a=c.b;break;
            case 0x79:/*LD A,C*/c.a=c.c;break;
            case 0x7a:/*LD A,D*/c.a=c.d;break;
            case 0x7b:/*LD A,E*/c.a=c.e;break;
            case 0x7c:/*LD A,H*/c.a=c.ih;break;
            case 0x7d:/*LD A,L*/c.a=c.il;break;
            case 0x7e:/*LD A,(HL/IX+d/IY+d)*/_ADDR(5);_MR(addr,c.a);break;
            case 0x7f:/*LD A,A*/c.a=c.a;break;
            case 0x80:/*ADD B*/{uint32_t res=c.a+c.b;c.f=_ADD_FLAGS(c.a,c.b,res);c.a=res;}break;
            case 0x81:/*ADD C*/{uint32_t res=c.a+c.c;c.f=_ADD_FLAGS(c.a,c.c,res);c.a=res;}break;
            case 0x82:/*ADD D*/{uint32_t res=c.a+c.d;c.f=_ADD_FLAGS(c.a,c.d,res);c.a=res;}break;
            case 0x83:/*ADD E*/{uint32_t res=c.a+c.e;c.f=_ADD_FLAGS(c.a,c.e,res);c.a=res;}break;
            case 0x84:/*ADD H*/{uint32_t res=c.a+c.ih;c.f=_ADD_FLAGS(c.a,c.ih,res);c.a=res;}break;
            case 0x85:/*ADD L*/{uint32_t res=c.a+c.il;c.f=_ADD_FLAGS(c.a,c.il,res);c.a=res;}break;
            case 0x86:/*ADD,(HL/IX+d/IY+d)*/_ADDR(5);_MR(addr,d8);{uint32_t res=c.a+d8;c.f=_ADD_FLAGS(c.a,d8,res);c.a=res;}break;
            case 0x87:/*ADD A*/{uint32_t res=c.a+c.a;c.f=_ADD_FLAGS(c.a,c.a,res);c.a=res;}break;
            case 0x88:/*ADC B*/{uint32_t res=c.a+c.b+(c.f&Z80_CF);c.f=_ADD_FLAGS(c.a,c.b,res);c.a=res;}break;
            case 0x89:/*ADC C*/{uint32_t res=c.a+c.c+(c.f&Z80_CF);c.f=_ADD_FLAGS(c.a,c.c,res);c.a=res;}break;
            case 0x8a:/*ADC D*/{uint32_t res=c.a+c.d+(c.f&Z80_CF);c.f=_ADD_FLAGS(c.a,c.d,res);c.a=res;}break;
            case 0x8b:/*ADC E*/{uint32_t res=c.a+c.e+(c.f&Z80_CF);c.f=_ADD_FLAGS(c.a,c.e,res);c.a=res;}break;
            case 0x8c:/*ADC H*/{uint32_t res=c.a+c.ih+(c.f&Z80_CF);c.f=_ADD_FLAGS(c.a,c.ih,res);c.a=res;}break;
            case 0x8d:/*ADC L*/{uint32_t res=c.a+c.il+(c.f&Z80_CF);c.f=_ADD_FLAGS(c.a,c.il,res);c.a=res;}break;
            case 0x8e:/*ADC,(HL/IX+d/IY+d)*/_ADDR(5);_MR(addr,d8);{uint32_t res=c.a+d8+(c.f&Z80_CF);c.f=_ADD_FLAGS(c.a,d8,res);c.a=res;}break;
            case 0x8f:/*ADC A*/{uint32_t res=c.a+c.a+(c.f&Z80_CF);c.f=_ADD_FLAGS(c.a,c.a,res);c.a=res;}break;
            case 0x90:/*SUB B*/{uint32_t res=(uint32_t)((int)c.a-(int)c.b);c.f=_SUB_FLAGS(c.a,c.b,res);c.a=res;}break;
            case 0x91:/*SUB C*/{uint32_t res=(uint32_t)((int)c.a-(int)c.c);c.f=_SUB_FLAGS(c.a,c.c,res);c.a=res;}break;
            case 0x92:/*SUB D*/{uint32_t res=(uint32_t)((int)c.a-(int)c.d);c.f=_SUB_FLAGS(c.a,c.d,res);c.a=res;}break;
            case 0x93:/*SUB E*/{uint32_t res=(uint32_t)((int)c.a-(int)c.e);c.f=_SUB_FLAGS(c.a,c.e,res);c.a=res;}break;
            case 0x94:/*SUB H*/{uint32_t res=(uint32_t)((int)c.a-(int)c.ih);c.f=_SUB_FLAGS(c.a,c.ih,res);c.a=res;}break;
            case 0x95:/*SUB L*/{uint32_t res=(uint32_t)((int)c.a-(int)c.il);c.f=_SUB_FLAGS(c.a,c.il,res);c.a=res;}break;
            case 0x96:/*SUB,(HL/IX+d/IY+d)*/_ADDR(5);_MR(addr,d8);{uint32_t res=(uint32_t)((int)c.a-(int)d8);c.f=_SUB_FLAGS(c.a,d8,res);c.a=res;}break;
            case 0x97:/*SUB A*/{uint32_t res=(uint32_t)((int)c.a-(int)c.a);c.f=_SUB_FLAGS(c.a,c.a,res);c.a=res;}break;
            case 0x98:/*SBC B*/{uint32_t res=(uint32_t)((int)c.a-(int)c.b-(c.f&Z80_CF));c.f=_SUB_FLAGS(c.a,c.b,res);c.a=res;}break;
            case 0x99:/*SBC C*/{uint32_t res=(uint32_t)((int)c.a-(int)c.c-(c.f&Z80_CF));c.f=_SUB_FLAGS(c.a,c.c,res);c.a=res;}break;
            case 0x9a:/*SBC D*/{uint32_t res=(uint32_t)((int)c.a-(int)c.d-(c.f&Z80_CF));c.f=_SUB_FLAGS(c.a,c.d,res);c.a=res;}break;
            case 0x9b:/*SBC E*/{uint32_t res=(uint32_t)((int)c.a-(int)c.e-(c.f&Z80_CF));c.f=_SUB_FLAGS(c.a,c.e,res);c.a=res;}break;
            case 0x9c:/*SBC H*/{uint32_t res=(uint32_t)((int)c.a-(int)c.ih-(c.f&Z80_CF));c.f=_SUB_FLAGS(c.a,c.ih,res);c.a=res;}break;
            case 0x9d:/*SBC L*/{uint32_t res=(uint32_t)((int)c.a-(int)c.il-(c.f&Z80_CF));c.f=_SUB_FLAGS(c.a,c.il,res);c.a=res;}break;
            case 0x9e:/*SBC,(HL/IX+d/IY+d)*/_ADDR(5);_MR(addr,d8);{uint32_t res=(uint32_t)((int)c.a-(int)d8-(c.f&Z80_CF));c.f=_SUB_FLAGS(c.a,d8,res);c.a=res;}break;
            case 0x9f:/*SBC A*/{uint32_t res=(uint32_t)((int)c.a-(int)c.a-(c.f&Z80_CF));c.f=_SUB_FLAGS(c.a,c.a,res);c.a=res;}break;
            case 0xa0:/*AND B*/{c.a&=c.b;c.f=_z80_szp[c.a]|Z80_HF;}break;
            case 0xa1:/*AND C*/{c.a&=c.c;c.f=_z80_szp[c.a]|Z80_HF;}break;
            case 0xa2:/*AND D*/{c.a&=c.d;c.f=_z80_szp[c.a]|Z80_HF;}break;
            case 0xa3:/*AND E*/{c.a&=c.e;c.f=_z80_szp[c.a]|Z80_HF;}break;
            case 0xa4:/*AND H*/{c.a&=c.ih;c.f=_z80_szp[c.a]|Z80_HF;}break;
            case 0xa5:/*AND L*/{c.a&=c.il;c.f=_z80_szp[c.a]|Z80_HF;}break;
            case 0xa6:/*AND,(HL/IX+d/IY+d)*/_ADDR(5);_MR(addr,d8);{c.a&=d8;c.f=_z80_szp[c.a]|Z80_HF;}break;
            case 0xa7:/*AND A*/{c.a&=c.a;c.f=_z80_szp[c.a]|Z80_HF;}break;
            case 0xa8:/*XOR B*/{c.a^=c.b;c.f=_z80_szp[c.a];}break;
            case 0xa9:/*XOR C*/{c.a^=c.c;c.f=_z80_szp[c.a];}break;
            case 0xaa:/*XOR D*/{c.a^=c.d;c.f=_z80_szp[c.a];}break;
            case 0xab:/*XOR E*/{c.a^=c.e;c.f=_z80_szp[c.a];}break;
            case 0xac:/*XOR H*/{c.a^=c.ih;c.f=_z80_szp[c.a];}break;
            case 0xad:/*XOR L*/{c.a^=c.il;c.f=_z80_szp[c.a];}break;
            case 0xae:/*XOR,(HL/IX+d/IY+d)*/_ADDR(5);_MR(addr,d8);{c.a^=d8;c.f=_z80_szp[c.a];}break;
            case 0xaf:/*XOR A*/{c.a^=c.a;c.f=_z80_szp[c.a];}break;
            case 0xb0:/*OR B*/{c.a|=c.b;c.f=_z80_szp[c.a];}break;
            case 0xb1:/*OR C*/{c.a|=c.c;c.f=_z80_szp[c.a];}break;
            case 0xb2:/*OR D*/{c.a|=c.d;c.f=_z80_szp[c.a];}break;
            case 0xb3:/*OR E*/{c.a|=c.e;c.f=_z80_szp[c.a];}break;
            case 0xb4:/*OR H*/{c.a|=c.ih;c.f=_z80_szp[c.a];}break;
            case 0xb5:/*OR L*/{c.a|=c.il;c.f=_z80_szp[c.a];}break;
            case 0xb6:/*OR,(HL/IX+d/IY+d)*/_ADDR(5);_MR(addr,d8);{c.a|=d8;c.f=_z80_szp[c.a];}break;
            case 0xb7:/*OR A*/{c.a|=c.a;c.f=_z80_szp[c.a];}break;
            case 0xb8:/*CP B*/{int32_t res=(uint32_t)((int)c.a-(int)c.b);c.f=_CP_FLAGS(c.a,c.b,res);}break;
            case 0xb9:/*CP C*/{int32_t res=(uint32_t)((int)c.a-(int)c.c);c.f=_CP_FLAGS(c.a,c.c,res);}break;
            case 0xba:/*CP D*/{int32_t res=(uint32_t)((int)c.a-(int)c.d);c.f=_CP_FLAGS(c.a,c.d,res);}break;
            case 0xbb:/*CP E*/{int32_t res=(uint32_t)((int)c.a-(int)c.e);c.f=_CP_FLAGS(c.a,c.e,res);}break;
            case 0xbc:/*CP H*/{int32_t res=(uint32_t)((int)c.a-(int)c.ih);c.f=_CP_FLAGS(c.a,c.ih,res);}break;
            case 0xbd:/*CP L*/{int32_t res=(uint32_t)((int)c.a-(int)c.il);c.f=_CP_FLAGS(c.a,c.il,res);}break;
            case 0xbe:/*CP,(HL/IX+d/IY+d)*/_ADDR(5);_MR(addr,d8);{int32_t res=(uint32_t)((int)c.a-(int)d8);c.f=_CP_FLAGS(c.a,d8,res);}break;
            case 0xbf:/*CP A*/{int32_t res=(uint32_t)((int)c.a-(int)c.a);c.f=_CP_FLAGS(c.a,c.a,res);}break;
            case 0xc0:/*RET NZ*/_T(1);if (!(c.f&Z80_ZF)){uint8_t w,z;_MR(c.sp++,z);_MR(c.sp++,w);c.pc=c.wz=(w<<8)|z;}break;
            case 0xc1:/*POP BC*/_MR(c.sp++,c.c);_MR(c.sp++,c.b);break;
            case 0xc2:/*JP NZ,nn*/_IMM16(addr);if(!(c.f&Z80_ZF)){c.pc=addr;}break;
            case 0xc3:/*JP nn*/_IMM16(c.pc);break;
            case 0xc4:/*CALL NZ,nn*/_IMM16(addr);if(!(c.f&Z80_ZF)){_T(1);_MW(--c.sp,c.pc>>8);_MW(--c.sp,c.pc);c.pc=addr;}break;
            case 0xc5:/*PUSH BC*/_T(1);_MW(--c.sp,c.b);_MW(--c.sp,c.c);break;
            case 0xc6:/*ADD n*/_IMM8(d8);{uint32_t res=c.a+d8;c.f=_ADD_FLAGS(c.a,d8,res);c.a=res;}break;
            case 0xc7:/*RST 0x0*/_T(1);_MW(--c.sp,c.pc>>8);_MW(--c.sp,c.pc);c.pc=c.wz=0x0;break;
            case 0xc8:/*RET Z*/_T(1);if ((c.f&Z80_ZF)){uint8_t w,z;_MR(c.sp++,z);_MR(c.sp++,w);c.pc=c.wz=(w<<8)|z;}break;
            case 0xc9:/*RET*/_MR(c.sp++,d8);c.pc=d8;_MR(c.sp++,d8);c.pc|=d8<<8;c.wz=c.pc;break;
            case 0xca:/*JP Z,nn*/_IMM16(addr);if((c.f&Z80_ZF)){c.pc=addr;}break;
            case 0xCB: {
                /* special handling for undocumented DD/FD+CB double prefix instructions,
                 these always load the value from memory (IX+d),
                 and write the value back, even for normal
                 "register" instructions
                 see: http://www.baltazarstudios.com/files/ddcb.html
                */
                /* load the d offset for indexed instructions */
                int8_t d;
                if (_IDX()) { _IMM8(d); } else { d=0; }
                /* fetch opcode without memory refresh and incrementint R */
                _FETCH_CB(op);
                const uint8_t x = op>>6;
                const uint8_t y = (op>>3)&7;
                const uint8_t z = op&7;
                /* load the operand (for indexed ops, always from memory!) */
                if ((z == 6) || _IDX()) {
                    _T(1);
                    addr = (c.ih<<8)|c.il;
                    if (_IDX()) {
                        _T(1);
                        addr += d;
                        c.wz = addr;
                    }
                    _MR(addr,d8);
                }
                else {
                    /* simple non-indexed, non-(HL): load register value */
                    switch (z) {
                        case 0: d8 = c.b; break;
                        case 1: d8 = c.c; break;
                        case 2: d8 = c.d; break;
                        case 3: d8 = c.e; break;
                        case 4: d8 = c.ih; break;
                        case 5: d8 = c.il; break;
                        case 7: d8 = c.a; break;
                    }
                }
                uint8_t f = c.f;
                uint8_t r;
                switch (x) {
                    case 0:
                        /* rot/shift */
                        switch (y) {
                            case 0: /*RLC*/ r=d8<<1|d8>>7; f=_z80_szp[r]|(d8>>7&Z80_CF); break;
                            case 1: /*RRC*/ r=d8>>1|d8<<7; f=_z80_szp[r]|(d8&Z80_CF); break;
                            case 2: /*RL */ r=d8<<1|(f&Z80_CF); f=_z80_szp[r]|(d8>>7&Z80_CF); break;
                            case 3: /*RR */ r=d8>>1|((f&Z80_CF)<<7); f=_z80_szp[r]|(d8&Z80_CF); break;
                            case 4: /*SLA*/ r=d8<<1; f=_z80_szp[r]|(d8>>7&Z80_CF); break;
                            case 5: /*SRA*/ r=d8>>1|(d8&0x80); f=_z80_szp[r]|(d8&Z80_CF); break;
                            case 6: /*SLL*/ r=d8<<1|1; f=_z80_szp[r]|(d8>>7&Z80_CF); break;
                            case 7: /*SRL*/ r=d8>>1; f=_z80_szp[r]|(d8&Z80_CF); break;
                        }
                        break;
                    case 1:
                        /* BIT (bit test) */
                        r = d8 & (1<<y);
                        f = (f&Z80_CF) | Z80_HF | (r?(r&Z80_SF):(Z80_ZF|Z80_PF));
                        if ((z == 6) || _IDX()) {
                            f |= (c.wz>>8) & (Z80_YF|Z80_XF);
                        }
                        else {
                            f |= d8 & (Z80_YF|Z80_XF);
                        }
                        break;
                    case 2:
                        /* RES (bit clear) */
                        r = d8 & ~(1<<y);
                        break;
                    case 3:
                        /* SET (bit set) */
                        r = d8 | (1<<y);
                        break;
                }
                if (x != 1) {
                    /* write result back */
                    if ((z == 6) || _IDX()) {
                        /* (HL), (IX+d), (IY+d): write back to memory, for extended ops,
                           even when the op is actually a register op
                        */
                        _MW(addr,r);
                    }
                    if (z != 6) {
                        /* write result back to register (special case for indexed + H/L! */
                        if (_IDX() && ((z==4)||(z==5))) {
                            if (z == 4) { c.h = r; }
                            else        { c.l = r; }
                        }
                        else {
                            switch (z) {
                                case 0: c.b = r; break;
                                case 1: c.c = r; break;
                                case 2: c.d = r; break;
                                case 3: c.e = r; break;
                                case 4: c.ih = r; break;
                                case 5: c.il = r; break;
                                case 7: c.a = r; break;
                            }
                        }
                    }
                }
                c.f = f;
            }
            break;
            case 0xcc:/*CALL Z,nn*/_IMM16(addr);if((c.f&Z80_ZF)){_T(1);_MW(--c.sp,c.pc>>8);_MW(--c.sp,c.pc);c.pc=addr;}break;
            case 0xcd:/*CALL nn*/_IMM16(addr);_T(1);_MW(--c.sp,c.pc>>8);_MW(--c.sp,c.pc);c.pc=addr;break;
            case 0xce:/*ADC n*/_IMM8(d8);{uint32_t res=c.a+d8+(c.f&Z80_CF);c.f=_ADD_FLAGS(c.a,d8,res);c.a=res;}break;
            case 0xcf:/*RST 0x8*/_T(1);_MW(--c.sp,c.pc>>8);_MW(--c.sp,c.pc);c.pc=c.wz=0x8;break;
            case 0xd0:/*RET NC*/_T(1);if (!(c.f&Z80_CF)){uint8_t w,z;_MR(c.sp++,z);_MR(c.sp++,w);c.pc=c.wz=(w<<8)|z;}break;
            case 0xd1:/*POP DE*/_MR(c.sp++,c.e);_MR(c.sp++,c.d);break;
            case 0xd2:/*JP NC,nn*/_IMM16(addr);if(!(c.f&Z80_CF)){c.pc=addr;}break;
            case 0xd3:/*OUT (n),A*/{_IMM8(d8);addr=(c.a<<8)|d8;_OUT(addr,c.a);c.wz=(addr&0xFF00)|((addr+1)&0x00FF);}break;
            case 0xd4:/*CALL NC,nn*/_IMM16(addr);if(!(c.f&Z80_CF)){_T(1);_MW(--c.sp,c.pc>>8);_MW(--c.sp,c.pc);c.pc=addr;}break;
            case 0xd5:/*PUSH DE*/_T(1);_MW(--c.sp,c.d);_MW(--c.sp,c.e);break;
            case 0xd6:/*SUB n*/_IMM8(d8);{uint32_t res=(uint32_t)((int)c.a-(int)d8);c.f=_SUB_FLAGS(c.a,d8,res);c.a=res;}break;
            case 0xd7:/*RST 0x10*/_T(1);_MW(--c.sp,c.pc>>8);_MW(--c.sp,c.pc);c.pc=c.wz=0x10;break;
            case 0xd8:/*RET C*/_T(1);if ((c.f&Z80_CF)){uint8_t w,z;_MR(c.sp++,z);_MR(c.sp++,w);c.pc=c.wz=(w<<8)|z;}break;
            case 0xd9:/*EXX*/{_z80_flush_ihl(&c,c.bits);uint16_t bc=(c.b<<8)|c.c;uint16_t de=(c.d<<8)|c.e;uint16_t hl=(c.h<<8)|c.l;c.b=c.bc_>>8;c.c=c.bc_;c.d=c.de_>>8;c.e=c.de_;c.h=c.hl_>>8;c.l=c.hl_;c.bc_=bc;c.de_=de;c.hl_=hl;_z80_load_ihl(&c,c.bits);}break;
            case 0xda:/*JP C,nn*/_IMM16(addr);if((c.f&Z80_CF)){c.pc=addr;}break;
            case 0xdb:/*IN A,(n)*/{_IMM8(d8);addr=(c.a<<8)|d8;_IN(addr++,c.a);c.wz=addr;}break;
            case 0xdc:/*CALL C,nn*/_IMM16(addr);if((c.f&Z80_CF)){_T(1);_MW(--c.sp,c.pc>>8);_MW(--c.sp,c.pc);c.pc=addr;}break;
            case 0xdd:/*DD prefix*/map_bits|=Z80_BIT_IX;continue;break;
            case 0xde:/*SBC n*/_IMM8(d8);{uint32_t res=(uint32_t)((int)c.a-(int)d8-(c.f&Z80_CF));c.f=_SUB_FLAGS(c.a,d8,res);c.a=res;}break;
            case 0xdf:/*RST 0x18*/_T(1);_MW(--c.sp,c.pc>>8);_MW(--c.sp,c.pc);c.pc=c.wz=0x18;break;
            case 0xe0:/*RET PO*/_T(1);if (!(c.f&Z80_PF)){uint8_t w,z;_MR(c.sp++,z);_MR(c.sp++,w);c.pc=c.wz=(w<<8)|z;}break;
            case 0xe1:/*POP HL/IX/IY*/_MR(c.sp++,c.il);_MR(c.sp++,c.ih);break;
            case 0xe2:/*JP PO,nn*/_IMM16(addr);if(!(c.f&Z80_PF)){c.pc=addr;}break;
            case 0xe3:/*EX (SP),HL/IX/IY*/{_T(3);addr=c.sp;uint8_t l,h;_MR(addr,l);_MR(addr+1,h);_MW(addr,c.il);_MW(addr+1,c.ih);c.ih=h;c.il=l;c.wz=(h<<8)|l;}break;
            case 0xe4:/*CALL PO,nn*/_IMM16(addr);if(!(c.f&Z80_PF)){_T(1);_MW(--c.sp,c.pc>>8);_MW(--c.sp,c.pc);c.pc=addr;}break;
            case 0xe5:/*PUSH HL/IX/IY*/_T(1);_MW(--c.sp,c.ih);_MW(--c.sp,c.il);break;
            case 0xe6:/*AND n*/_IMM8(d8);{c.a&=d8;c.f=_z80_szp[c.a]|Z80_HF;}break;
            case 0xe7:/*RST 0x20*/_T(1);_MW(--c.sp,c.pc>>8);_MW(--c.sp,c.pc);c.pc=c.wz=0x20;break;
            case 0xe8:/*RET PE*/_T(1);if ((c.f&Z80_PF)){uint8_t w,z;_MR(c.sp++,z);_MR(c.sp++,w);c.pc=c.wz=(w<<8)|z;}break;
            case 0xe9:/*JP HL/IX/IY*/c.pc=(c.ih<<8)|c.il;break;
            case 0xea:/*JP PE,nn*/_IMM16(addr);if((c.f&Z80_PF)){c.pc=addr;}break;
            case 0xeb:/*EX DE,HL*/{_z80_flush_ihl(&c,c.bits);c.e^=c.l;c.l^=c.e;c.e^=c.l;c.d^=c.h;c.h^=c.d;c.d^=c.h;_z80_load_ihl(&c,c.bits);}break;
            case 0xec:/*CALL PE,nn*/_IMM16(addr);if((c.f&Z80_PF)){_T(1);_MW(--c.sp,c.pc>>8);_MW(--c.sp,c.pc);c.pc=addr;}break;
            case 0xED: {
                _FETCH(op);
                switch(op) {
                    case 0x40:/*IN B,(C)*/{addr=(c.b<<8)|c.c;_IN(addr++,d8);c.wz=addr;c.f=(c.f&Z80_CF)|_z80_szp[d8];c.b=d8;}break;
                    case 0x41:/*OUT (C),B*/addr=(c.b<<8)|c.c;_OUT(addr++,c.b);c.wz=addr;break;
                    case 0x42:/*SBC HL,BC*/{uint16_t acc=(c.ih<<8)|c.il;c.wz=acc+1;d16=(c.b<<8)|c.c;uint32_t r=acc-d16-(c.f&Z80_CF);c.ih=r>>8;c.il=r;c.f=Z80_NF|(((d16^acc)&(acc^r)&0x8000)>>13);c.f|=((acc^r^d16)>>8) & Z80_HF;c.f|=(r>>16)&Z80_CF;c.f|=(r>>8)&(Z80_SF|Z80_YF|Z80_XF);c.f|=(r&0xFFFF)?0:Z80_ZF;_T(7);}break;
                    case 0x43:/*LD (nn),BC*/_IMM16(addr);d16=(c.b<<8)|c.c;_MW(addr++,d16&0xFF);_MW(addr,d16>>8);c.wz=addr;break;
                    case 0x44:/*NEG*/d8=c.a;c.a=0;{uint32_t res=(uint32_t)((int)c.a-(int)d8);c.f=_SUB_FLAGS(c.a,d8,res);c.a=res;}break;
                    case 0x45:/*RETN*/pins|=Z80_RETI;_MR(c.sp++,d8);c.pc=d8;_MR(c.sp++,d8);c.pc|=d8<<8;c.wz=c.pc;if(c.bits&Z80_BIT_IFF2){c.bits|=Z80_BIT_IFF1;}else{c.bits&=~Z80_BIT_IFF1;}break;
                    case 0x46:/*IM 0*/c.im=0;break;
                    case 0x47:/*LD I,A*/_T(1);c.i=c.a;break;
                    case 0x48:/*IN C,(C)*/{addr=(c.b<<8)|c.c;_IN(addr++,d8);c.wz=addr;c.f=(c.f&Z80_CF)|_z80_szp[d8];c.c=d8;}break;
                    case 0x49:/*OUT (C),C*/addr=(c.b<<8)|c.c;_OUT(addr++,c.c);c.wz=addr;break;
                    case 0x4a:/*ADC HL,BC*/{uint16_t acc=(c.ih<<8)|c.il;c.wz=acc+1;d16=(c.b<<8)|c.c;uint32_t r=acc+d16+(c.f&Z80_CF);c.ih=r>>8;c.il=r;c.f=((d16^acc^0x8000)&(d16^r)&0x8000)>>13;c.f|=((acc^r^d16)>>8)&Z80_HF;c.f|=(r>>16)&Z80_CF;c.f|=(r>>8)&(Z80_SF|Z80_YF|Z80_XF);c.f|=(r&0xFFFF)?0:Z80_ZF;_T(7);}break;
                    case 0x4b:/*LD BC,(nn)*/_IMM16(addr);_MR(addr++,d8);d16=d8;_MR(addr,d8);d16|=d8<<8;c.b=d16>>8;c.c=d16;c.wz=addr;break;
                    case 0x4c:/*NEG*/d8=c.a;c.a=0;{uint32_t res=(uint32_t)((int)c.a-(int)d8);c.f=_SUB_FLAGS(c.a,d8,res);c.a=res;}break;
                    case 0x4d:/*RETI*/pins|=Z80_RETI;_MR(c.sp++,d8);c.pc=d8;_MR(c.sp++,d8);c.pc|=d8<<8;c.wz=c.pc;if(c.bits&Z80_BIT_IFF2){c.bits|=Z80_BIT_IFF1;}else{c.bits&=~Z80_BIT_IFF1;}break;
                    case 0x4e:/*IM 0*/c.im=0;break;
                    case 0x4f:/*LD R,A*/_T(1);c.r=c.a;break;
                    case 0x50:/*IN D,(C)*/{addr=(c.b<<8)|c.c;_IN(addr++,d8);c.wz=addr;c.f=(c.f&Z80_CF)|_z80_szp[d8];c.d=d8;}break;
                    case 0x51:/*OUT (C),D*/addr=(c.b<<8)|c.c;_OUT(addr++,c.d);c.wz=addr;break;
                    case 0x52:/*SBC HL,DE*/{uint16_t acc=(c.ih<<8)|c.il;c.wz=acc+1;d16=(c.d<<8)|c.e;uint32_t r=acc-d16-(c.f&Z80_CF);c.ih=r>>8;c.il=r;c.f=Z80_NF|(((d16^acc)&(acc^r)&0x8000)>>13);c.f|=((acc^r^d16)>>8) & Z80_HF;c.f|=(r>>16)&Z80_CF;c.f|=(r>>8)&(Z80_SF|Z80_YF|Z80_XF);c.f|=(r&0xFFFF)?0:Z80_ZF;_T(7);}break;
                    case 0x53:/*LD (nn),DE*/_IMM16(addr);d16=(c.d<<8)|c.e;_MW(addr++,d16&0xFF);_MW(addr,d16>>8);c.wz=addr;break;
                    case 0x54:/*NEG*/d8=c.a;c.a=0;{uint32_t res=(uint32_t)((int)c.a-(int)d8);c.f=_SUB_FLAGS(c.a,d8,res);c.a=res;}break;
                    case 0x55:/*RETN*/pins|=Z80_RETI;_MR(c.sp++,d8);c.pc=d8;_MR(c.sp++,d8);c.pc|=d8<<8;c.wz=c.pc;if(c.bits&Z80_BIT_IFF2){c.bits|=Z80_BIT_IFF1;}else{c.bits&=~Z80_BIT_IFF1;}break;
                    case 0x56:/*IM 1*/c.im=1;break;
                    case 0x57:/*LD A,I*/_T(1);c.a=c.i;c.f=_SZIFF2_FLAGS(c.a);break;
                    case 0x58:/*IN E,(C)*/{addr=(c.b<<8)|c.c;_IN(addr++,d8);c.wz=addr;c.f=(c.f&Z80_CF)|_z80_szp[d8];c.e=d8;}break;
                    case 0x59:/*OUT (C),E*/addr=(c.b<<8)|c.c;_OUT(addr++,c.e);c.wz=addr;break;
                    case 0x5a:/*ADC HL,DE*/{uint16_t acc=(c.ih<<8)|c.il;c.wz=acc+1;d16=(c.d<<8)|c.e;uint32_t r=acc+d16+(c.f&Z80_CF);c.ih=r>>8;c.il=r;c.f=((d16^acc^0x8000)&(d16^r)&0x8000)>>13;c.f|=((acc^r^d16)>>8)&Z80_HF;c.f|=(r>>16)&Z80_CF;c.f|=(r>>8)&(Z80_SF|Z80_YF|Z80_XF);c.f|=(r&0xFFFF)?0:Z80_ZF;_T(7);}break;
                    case 0x5b:/*LD DE,(nn)*/_IMM16(addr);_MR(addr++,d8);d16=d8;_MR(addr,d8);d16|=d8<<8;c.d=d16>>8;c.e=d16;c.wz=addr;break;
                    case 0x5c:/*NEG*/d8=c.a;c.a=0;{uint32_t res=(uint32_t)((int)c.a-(int)d8);c.f=_SUB_FLAGS(c.a,d8,res);c.a=res;}break;
                    case 0x5d:/*RETN*/pins|=Z80_RETI;_MR(c.sp++,d8);c.pc=d8;_MR(c.sp++,d8);c.pc|=d8<<8;c.wz=c.pc;if(c.bits&Z80_BIT_IFF2){c.bits|=Z80_BIT_IFF1;}else{c.bits&=~Z80_BIT_IFF1;}break;
                    case 0x5e:/*IM 2*/c.im=2;break;
                    case 0x5f:/*LD A,R*/_T(1);c.a=c.r;c.f=_SZIFF2_FLAGS(c.a);break;
                    case 0x60:/*IN H,(C)*/{addr=(c.b<<8)|c.c;_IN(addr++,d8);c.wz=addr;c.f=(c.f&Z80_CF)|_z80_szp[d8];c.ih=d8;}break;
                    case 0x61:/*OUT (C),H*/addr=(c.b<<8)|c.c;_OUT(addr++,c.ih);c.wz=addr;break;
                    case 0x62:/*SBC HL,HL/IX/IY*/{uint16_t acc=(c.ih<<8)|c.il;c.wz=acc+1;d16=(c.ih<<8)|c.il;uint32_t r=acc-d16-(c.f&Z80_CF);c.ih=r>>8;c.il=r;c.f=Z80_NF|(((d16^acc)&(acc^r)&0x8000)>>13);c.f|=((acc^r^d16)>>8) & Z80_HF;c.f|=(r>>16)&Z80_CF;c.f|=(r>>8)&(Z80_SF|Z80_YF|Z80_XF);c.f|=(r&0xFFFF)?0:Z80_ZF;_T(7);}break;
                    case 0x63:/*LD (nn),HL/IX/IY*/_IMM16(addr);d16=(c.ih<<8)|c.il;_MW(addr++,d16&0xFF);_MW(addr,d16>>8);c.wz=addr;break;
                    case 0x64:/*NEG*/d8=c.a;c.a=0;{uint32_t res=(uint32_t)((int)c.a-(int)d8);c.f=_SUB_FLAGS(c.a,d8,res);c.a=res;}break;
                    case 0x65:/*RETN*/pins|=Z80_RETI;_MR(c.sp++,d8);c.pc=d8;_MR(c.sp++,d8);c.pc|=d8<<8;c.wz=c.pc;if(c.bits&Z80_BIT_IFF2){c.bits|=Z80_BIT_IFF1;}else{c.bits&=~Z80_BIT_IFF1;}break;
                    case 0x66:/*IM 0*/c.im=0;break;
                    case 0x67:/*RRD*/{addr=(c.ih<<8)|c.il;_MR(addr,d8);uint8_t l=c.a&0x0F;c.a=(c.a&0xF0)|(d8&0x0F);d8=(d8>>4)|(l<<4);_MW(addr++,d8);c.wz=addr;c.f=(c.f&Z80_CF)|_z80_szp[c.a];_T(4);}break;
                    case 0x68:/*IN L,(C)*/{addr=(c.b<<8)|c.c;_IN(addr++,d8);c.wz=addr;c.f=(c.f&Z80_CF)|_z80_szp[d8];c.il=d8;}break;
                    case 0x69:/*OUT (C),L*/addr=(c.b<<8)|c.c;_OUT(addr++,c.il);c.wz=addr;break;
                    case 0x6a:/*ADC HL,HL/IX/IY*/{uint16_t acc=(c.ih<<8)|c.il;c.wz=acc+1;d16=(c.ih<<8)|c.il;uint32_t r=acc+d16+(c.f&Z80_CF);c.ih=r>>8;c.il=r;c.f=((d16^acc^0x8000)&(d16^r)&0x8000)>>13;c.f|=((acc^r^d16)>>8)&Z80_HF;c.f|=(r>>16)&Z80_CF;c.f|=(r>>8)&(Z80_SF|Z80_YF|Z80_XF);c.f|=(r&0xFFFF)?0:Z80_ZF;_T(7);}break;
                    case 0x6b:/*LD HL/IX/IY,(nn)*/_IMM16(addr);_MR(addr++,d8);d16=d8;_MR(addr,d8);d16|=d8<<8;c.ih=d16>>8;c.il=d16;c.wz=addr;break;
                    case 0x6c:/*NEG*/d8=c.a;c.a=0;{uint32_t res=(uint32_t)((int)c.a-(int)d8);c.f=_SUB_FLAGS(c.a,d8,res);c.a=res;}break;
                    case 0x6d:/*RETN*/pins|=Z80_RETI;_MR(c.sp++,d8);c.pc=d8;_MR(c.sp++,d8);c.pc|=d8<<8;c.wz=c.pc;if(c.bits&Z80_BIT_IFF2){c.bits|=Z80_BIT_IFF1;}else{c.bits&=~Z80_BIT_IFF1;}break;
                    case 0x6e:/*IM 0*/c.im=0;break;
                    case 0x6f:/*RLD*/{addr=(c.ih<<8)|c.il;_MR(addr,d8);uint8_t l=c.a&0x0F;c.a=(c.a&0xF0)|(d8>>4);d8=(d8<<4)|l;_MW(addr++,d8);c.wz=addr;c.f=(c.f&Z80_CF)|_z80_szp[c.a];_T(4);}break;
                    case 0x70:/*IN F,(C)*/{addr=(c.b<<8)|c.c;_IN(addr++,d8);c.wz=addr;c.f=(c.f&Z80_CF)|_z80_szp[d8];}break;
                    case 0x71:/*OUT (C),F*/addr=(c.b<<8)|c.c;_OUT(addr++,0);c.wz=addr;break;
                    case 0x72:/*SBC HL,SP*/{uint16_t acc=(c.ih<<8)|c.il;c.wz=acc+1;d16=c.sp;uint32_t r=acc-d16-(c.f&Z80_CF);c.ih=r>>8;c.il=r;c.f=Z80_NF|(((d16^acc)&(acc^r)&0x8000)>>13);c.f|=((acc^r^d16)>>8) & Z80_HF;c.f|=(r>>16)&Z80_CF;c.f|=(r>>8)&(Z80_SF|Z80_YF|Z80_XF);c.f|=(r&0xFFFF)?0:Z80_ZF;_T(7);}break;
                    case 0x73:/*LD (nn),SP*/_IMM16(addr);d16=c.sp;_MW(addr++,d16&0xFF);_MW(addr,d16>>8);c.wz=addr;break;
                    case 0x74:/*NEG*/d8=c.a;c.a=0;{uint32_t res=(uint32_t)((int)c.a-(int)d8);c.f=_SUB_FLAGS(c.a,d8,res);c.a=res;}break;
                    case 0x75:/*RETN*/pins|=Z80_RETI;_MR(c.sp++,d8);c.pc=d8;_MR(c.sp++,d8);c.pc|=d8<<8;c.wz=c.pc;if(c.bits&Z80_BIT_IFF2){c.bits|=Z80_BIT_IFF1;}else{c.bits&=~Z80_BIT_IFF1;}break;
                    case 0x76:/*IM 1*/c.im=1;break;
                    case 0x77:/*NOP (ED)*/ break;
                    case 0x78:/*IN A,(C)*/{addr=(c.b<<8)|c.c;_IN(addr++,d8);c.wz=addr;c.f=(c.f&Z80_CF)|_z80_szp[d8];c.a=d8;}break;
                    case 0x79:/*OUT (C),A*/addr=(c.b<<8)|c.c;_OUT(addr++,c.a);c.wz=addr;break;
                    case 0x7a:/*ADC HL,SP*/{uint16_t acc=(c.ih<<8)|c.il;c.wz=acc+1;d16=c.sp;uint32_t r=acc+d16+(c.f&Z80_CF);c.ih=r>>8;c.il=r;c.f=((d16^acc^0x8000)&(d16^r)&0x8000)>>13;c.f|=((acc^r^d16)>>8)&Z80_HF;c.f|=(r>>16)&Z80_CF;c.f|=(r>>8)&(Z80_SF|Z80_YF|Z80_XF);c.f|=(r&0xFFFF)?0:Z80_ZF;_T(7);}break;
                    case 0x7b:/*LD SP,(nn)*/_IMM16(addr);_MR(addr++,d8);d16=d8;_MR(addr,d8);d16|=d8<<8;c.sp=d16;c.wz=addr;break;
                    case 0x7c:/*NEG*/d8=c.a;c.a=0;{uint32_t res=(uint32_t)((int)c.a-(int)d8);c.f=_SUB_FLAGS(c.a,d8,res);c.a=res;}break;
                    case 0x7d:/*RETN*/pins|=Z80_RETI;_MR(c.sp++,d8);c.pc=d8;_MR(c.sp++,d8);c.pc|=d8<<8;c.wz=c.pc;if(c.bits&Z80_BIT_IFF2){c.bits|=Z80_BIT_IFF1;}else{c.bits&=~Z80_BIT_IFF1;}break;
                    case 0x7e:/*IM 2*/c.im=2;break;
                    case 0x7f:/*NOP (ED)*/ break;
                    case 0xa0:/*LDI*/{uint16_t hl=(c.ih<<8)|c.il;uint16_t de=(c.d<<8)|c.e;_MR(hl,d8);_MW(de,d8);hl++;de++;c.ih=hl>>8;c.il=hl;c.d=de>>8;c.e=de;_T(2);d8+=c.a;c.f &=(Z80_SF|Z80_ZF|Z80_CF);if(d8&0x02){c.f|=Z80_YF;}if(d8&0x08){c.f|=Z80_XF;}uint16_t bc=(c.b<<8)|c.c;bc--;c.b=bc>>8;c.c=bc;if(bc){c.f|=Z80_VF;}}break;
                    case 0xa1:/*CPI*/{uint16_t hl = (c.ih<<8)|c.il;_MR(hl,d8);hl++;c.wz++;c.ih=hl>>8;c.il=hl;_T(5);int r=(int)c.a-d8;c.f=(c.f&Z80_CF)|Z80_NF|_SZ(r);if((r&0x0F)>(c.a&0x0F)){c.f|=Z80_HF;r--;}if(r&0x02){c.f|=Z80_YF;}if(r&0x08){c.f|=Z80_XF;}uint16_t bc=(c.b<<8)|c.c;bc--;c.b=bc>>8;c.c=bc;if(bc){c.f|=Z80_VF;}}break;
                    case 0xa2:/*INI*/{_T(1);addr=(c.b<<8)|c.c;uint16_t hl=(c.ih<<8)|c.il;_IN(addr,d8);_MW(hl,d8);uint8_t rc=c.c;c.b--;addr++;hl++;rc++;c.ih=hl>>8; c.il=hl;c.wz=addr;c.f=(c.b?(c.b&Z80_SF):Z80_ZF)|(c.b&(Z80_XF|Z80_YF));if(d8&Z80_SF){c.f|=Z80_NF;}uint32_t t=(uint32_t)(rc&0xFF)+d8;if(t&0x100){c.f|=Z80_HF|Z80_CF;}c.f|=_z80_szp[((uint8_t)(t&0x07))^c.b]&Z80_PF;}break;
                    case 0xa3:/*OUTI*/{_T(1);uint16_t hl=(c.ih<<8)|c.il;_MR(hl,d8);c.b--;addr=(c.b<<8)|c.c;_OUT(addr,d8);addr++; hl++;c.ih=hl>>8;c.il=hl;c.wz=addr;c.f=(c.b?(c.b&Z80_SF):Z80_ZF)|(c.b&(Z80_XF|Z80_YF));if(d8&Z80_SF){c.f|=Z80_NF;}uint32_t t=(uint32_t)c.il+(uint32_t)d8;if (t&0x0100){c.f|=Z80_HF|Z80_CF;}c.f|=_z80_szp[((uint8_t)(t&0x07))^c.b]&Z80_PF;}break;
                    case 0xa8:/*LDD*/{uint16_t hl=(c.ih<<8)|c.il;uint16_t de=(c.d<<8)|c.e;_MR(hl,d8);_MW(de,d8);hl--;de--;c.ih=hl>>8;c.il=hl;c.d=de>>8;c.e=de;_T(2);d8+=c.a;c.f &=(Z80_SF|Z80_ZF|Z80_CF);if(d8&0x02){c.f|=Z80_YF;}if(d8&0x08){c.f|=Z80_XF;}uint16_t bc=(c.b<<8)|c.c;bc--;c.b=bc>>8;c.c=bc;if(bc){c.f|=Z80_VF;}}break;
                    case 0xa9:/*CPD*/{uint16_t hl = (c.ih<<8)|c.il;_MR(hl,d8);hl--;c.wz--;c.ih=hl>>8;c.il=hl;_T(5);int r=(int)c.a-d8;c.f=(c.f&Z80_CF)|Z80_NF|_SZ(r);if((r&0x0F)>(c.a&0x0F)){c.f|=Z80_HF;r--;}if(r&0x02){c.f|=Z80_YF;}if(r&0x08){c.f|=Z80_XF;}uint16_t bc=(c.b<<8)|c.c;bc--;c.b=bc>>8;c.c=bc;if(bc){c.f|=Z80_VF;}}break;
                    case 0xaa:/*IND*/{_T(1);addr=(c.b<<8)|c.c;uint16_t hl=(c.ih<<8)|c.il;_IN(addr,d8);_MW(hl,d8);uint8_t rc=c.c;c.b--;addr--;hl--;rc--;c.ih=hl>>8; c.il=hl;c.wz=addr;c.f=(c.b?(c.b&Z80_SF):Z80_ZF)|(c.b&(Z80_XF|Z80_YF));if(d8&Z80_SF){c.f|=Z80_NF;}uint32_t t=(uint32_t)(rc&0xFF)+d8;if(t&0x100){c.f|=Z80_HF|Z80_CF;}c.f|=_z80_szp[((uint8_t)(t&0x07))^c.b]&Z80_PF;}break;
                    case 0xab:/*OUTD*/{_T(1);uint16_t hl=(c.ih<<8)|c.il;_MR(hl,d8);c.b--;addr=(c.b<<8)|c.c;_OUT(addr,d8);addr--;hl--;c.ih=hl>>8;c.il=hl;c.wz=addr;c.f=(c.b?(c.b&Z80_SF):Z80_ZF)|(c.b&(Z80_XF|Z80_YF));if(d8&Z80_SF){c.f|=Z80_NF;}uint32_t t=(uint32_t)c.il+(uint32_t)d8;if (t&0x0100){c.f|=Z80_HF|Z80_CF;}c.f|=_z80_szp[((uint8_t)(t&0x07))^c.b]&Z80_PF;}break;
                    case 0xb0:/*LDIR*/{uint16_t hl=(c.ih<<8)|c.il;uint16_t de=(c.d<<8)|c.e;_MR(hl,d8);_MW(de,d8);hl++;de++;c.ih=hl>>8;c.il=hl;c.d=de>>8;c.e=de;_T(2);d8+=c.a;c.f &=(Z80_SF|Z80_ZF|Z80_CF);if(d8&0x02){c.f|=Z80_YF;}if(d8&0x08){c.f|=Z80_XF;}uint16_t bc=(c.b<<8)|c.c;bc--;c.b=bc>>8;c.c=bc;if(bc){c.f|=Z80_VF;}if(bc){c.pc-=2;c.wz=c.pc+1;_T(5);}}break;
                    case 0xb1:/*CPIR*/{uint16_t hl = (c.ih<<8)|c.il;_MR(hl,d8);hl++;c.wz++;c.ih=hl>>8;c.il=hl;_T(5);int r=(int)c.a-d8;c.f=(c.f&Z80_CF)|Z80_NF|_SZ(r);if((r&0x0F)>(c.a&0x0F)){c.f|=Z80_HF;r--;}if(r&0x02){c.f|=Z80_YF;}if(r&0x08){c.f|=Z80_XF;}uint16_t bc=(c.b<<8)|c.c;bc--;c.b=bc>>8;c.c=bc;if(bc){c.f|=Z80_VF;}if(bc&&!(c.f&Z80_ZF)){c.pc-=2;c.wz=c.pc+1;_T(5);}}break;
                    case 0xb2:/*INIR*/{_T(1);addr=(c.b<<8)|c.c;uint16_t hl=(c.ih<<8)|c.il;_IN(addr,d8);_MW(hl,d8);uint8_t rc=c.c;c.b--;addr++;hl++;rc++;c.ih=hl>>8; c.il=hl;c.wz=addr;c.f=(c.b?(c.b&Z80_SF):Z80_ZF)|(c.b&(Z80_XF|Z80_YF));if(d8&Z80_SF){c.f|=Z80_NF;}uint32_t t=(uint32_t)(rc&0xFF)+d8;if(t&0x100){c.f|=Z80_HF|Z80_CF;}c.f|=_z80_szp[((uint8_t)(t&0x07))^c.b]&Z80_PF;if(c.b){c.pc-=2;_T(5);}}break;
                    case 0xb3:/*OTIR*/{_T(1);uint16_t hl=(c.ih<<8)|c.il;_MR(hl,d8);c.b--;addr=(c.b<<8)|c.c;_OUT(addr,d8);addr++; hl++;c.ih=hl>>8;c.il=hl;c.wz=addr;c.f=(c.b?(c.b&Z80_SF):Z80_ZF)|(c.b&(Z80_XF|Z80_YF));if(d8&Z80_SF){c.f|=Z80_NF;}uint32_t t=(uint32_t)c.il+(uint32_t)d8;if (t&0x0100){c.f|=Z80_HF|Z80_CF;}c.f|=_z80_szp[((uint8_t)(t&0x07))^c.b]&Z80_PF;if(c.b){c.pc-=2;_T(5);}}break;
                    case 0xb8:/*LDDR*/{uint16_t hl=(c.ih<<8)|c.il;uint16_t de=(c.d<<8)|c.e;_MR(hl,d8);_MW(de,d8);hl--;de--;c.ih=hl>>8;c.il=hl;c.d=de>>8;c.e=de;_T(2);d8+=c.a;c.f &=(Z80_SF|Z80_ZF|Z80_CF);if(d8&0x02){c.f|=Z80_YF;}if(d8&0x08){c.f|=Z80_XF;}uint16_t bc=(c.b<<8)|c.c;bc--;c.b=bc>>8;c.c=bc;if(bc){c.f|=Z80_VF;}if(bc){c.pc-=2;c.wz=c.pc+1;_T(5);}}break;
                    case 0xb9:/*CPDR*/{uint16_t hl = (c.ih<<8)|c.il;_MR(hl,d8);hl--;c.wz--;c.ih=hl>>8;c.il=hl;_T(5);int r=(int)c.a-d8;c.f=(c.f&Z80_CF)|Z80_NF|_SZ(r);if((r&0x0F)>(c.a&0x0F)){c.f|=Z80_HF;r--;}if(r&0x02){c.f|=Z80_YF;}if(r&0x08){c.f|=Z80_XF;}uint16_t bc=(c.b<<8)|c.c;bc--;c.b=bc>>8;c.c=bc;if(bc){c.f|=Z80_VF;}if(bc&&!(c.f&Z80_ZF)){c.pc-=2;c.wz=c.pc+1;_T(5);}}break;
                    case 0xba:/*INDR*/{_T(1);addr=(c.b<<8)|c.c;uint16_t hl=(c.ih<<8)|c.il;_IN(addr,d8);_MW(hl,d8);uint8_t rc=c.c;c.b--;addr--;hl--;rc--;c.ih=hl>>8; c.il=hl;c.wz=addr;c.f=(c.b?(c.b&Z80_SF):Z80_ZF)|(c.b&(Z80_XF|Z80_YF));if(d8&Z80_SF){c.f|=Z80_NF;}uint32_t t=(uint32_t)(rc&0xFF)+d8;if(t&0x100){c.f|=Z80_HF|Z80_CF;}c.f|=_z80_szp[((uint8_t)(t&0x07))^c.b]&Z80_PF;if(c.b){c.pc-=2;_T(5);}}break;
                    case 0xbb:/*OTDR*/{_T(1);uint16_t hl=(c.ih<<8)|c.il;_MR(hl,d8);c.b--;addr=(c.b<<8)|c.c;_OUT(addr,d8);addr--;hl--;c.ih=hl>>8;c.il=hl;c.wz=addr;c.f=(c.b?(c.b&Z80_SF):Z80_ZF)|(c.b&(Z80_XF|Z80_YF));if(d8&Z80_SF){c.f|=Z80_NF;}uint32_t t=(uint32_t)c.il+(uint32_t)d8;if (t&0x0100){c.f|=Z80_HF|Z80_CF;}c.f|=_z80_szp[((uint8_t)(t&0x07))^c.b]&Z80_PF;if(c.b){c.pc-=2;_T(5);}}break;
                    default: break;
                }
            }
            break;
            case 0xee:/*XOR n*/_IMM8(d8);{c.a^=d8;c.f=_z80_szp[c.a];}break;
            case 0xef:/*RST 0x28*/_T(1);_MW(--c.sp,c.pc>>8);_MW(--c.sp,c.pc);c.pc=c.wz=0x28;break;
            case 0xf0:/*RET P*/_T(1);if (!(c.f&Z80_SF)){uint8_t w,z;_MR(c.sp++,z);_MR(c.sp++,w);c.pc=c.wz=(w<<8)|z;}break;
            case 0xf1:/*POP AF*/_MR(c.sp++,c.f);_MR(c.sp++,c.a);break;
            case 0xf2:/*JP P,nn*/_IMM16(addr);if(!(c.f&Z80_SF)){c.pc=addr;}break;
            case 0xf3:/*DI*/c.bits&=~(Z80_BIT_IFF1|Z80_BIT_IFF2);break;
            case 0xf4:/*CALL P,nn*/_IMM16(addr);if(!(c.f&Z80_SF)){_T(1);_MW(--c.sp,c.pc>>8);_MW(--c.sp,c.pc);c.pc=addr;}break;
            case 0xf5:/*PUSH AF*/_T(1);_MW(--c.sp,c.a);_MW(--c.sp,c.f);break;
            case 0xf6:/*OR n*/_IMM8(d8);{c.a|=d8;c.f=_z80_szp[c.a];}break;
            case 0xf7:/*RST 0x30*/_T(1);_MW(--c.sp,c.pc>>8);_MW(--c.sp,c.pc);c.pc=c.wz=0x30;break;
            case 0xf8:/*RET M*/_T(1);if ((c.f&Z80_SF)){uint8_t w,z;_MR(c.sp++,z);_MR(c.sp++,w);c.pc=c.wz=(w<<8)|z;}break;
            case 0xf9:/*LD SP,HL/IX/IY*/_T(2);c.sp=(c.ih<<8)|c.il;break;
            case 0xfa:/*JP M,nn*/_IMM16(addr);if((c.f&Z80_SF)){c.pc=addr;}break;
            case 0xfb:/*EI*/c.bits=(c.bits&~(Z80_BIT_IFF1|Z80_BIT_IFF2))|Z80_BIT_EI;break;
            case 0xfc:/*CALL M,nn*/_IMM16(addr);if((c.f&Z80_SF)){_T(1);_MW(--c.sp,c.pc>>8);_MW(--c.sp,c.pc);c.pc=addr;}break;
            case 0xfd:/*FD prefix*/map_bits|=Z80_BIT_IY;continue;break;
            case 0xfe:/*CP n*/_IMM8(d8);{int32_t res=(uint32_t)((int)c.a-(int)d8);c.f=_CP_FLAGS(c.a,d8,res);}break;
            case 0xff:/*RST 0x38*/_T(1);_MW(--c.sp,c.pc>>8);_MW(--c.sp,c.pc);c.pc=c.wz=0x38;break;

            default: break;
        }
        /* interrupt handling */
        bool nmi = (pins & (pre_pins ^ pins)) & Z80_NMI;
        bool irq = ((pins & (Z80_INT|Z80_BUSREQ)) == Z80_INT) && (c.bits & Z80_BIT_IFF1);
        if (nmi || irq) {
            c.bits &= ~Z80_BIT_IFF1;
            /* a regular interrupt also clears the IFF2 flag */
            if (!nmi) {
                c.bits &= ~Z80_BIT_IFF2;
            }
            /* leave HALT state (if currently in HALT) */
            if (pins & Z80_HALT) {
                pins &= ~Z80_HALT;
                c.pc++;
            }
            /* put PC on address bus */
            _SA(c.pc);
            if (nmi) {
                /* non-maskable interrupt:
                    - fetch and discard the next instruction byte
                    - store PC on stack
                    - load PC and WZ with 0x0066
                */
                _TWM(5,Z80_M1|Z80_MREQ|Z80_RD); _BUMPR();
                _MW(--c.sp,c.pc>>8);
                _MW(--c.sp,c.pc);
                c.pc = c.wz = 0x0066;
            }
            else {
                /* maskable interrupt: load interrupt vector from peripheral (even if not used) */
                _TWM(4,Z80_M1|Z80_IORQ); _BUMPR();
                const uint8_t int_vec = _GD();
                _T(2);
                uint16_t addr;
                uint8_t w, z;
                switch (c.im) {
                    /* interrupt mode 0: not supported */
                    case 0:
                        break;
                    /* interrupt mode 1: put PC on stack, load PC and WZ with 0x0038 */
                    case 1:
                        _MW(--c.sp,c.pc>>8);
                        _MW(--c.sp,c.pc);
                        c.pc = c.wz = 0x0038;
                        break;
                    /* interrupt mode 2: put PC on stack, load PC and WZ from interrupt-vector-table */
                    case 2:
                        _MW(--c.sp,c.pc>>8);
                        _MW(--c.sp,c.pc);
                        addr = (c.i<<8) | (int_vec & 0xFE);
                        _MR(addr++,z);
                        _MR(addr,w);
                        c.pc = c.wz = (w<<8)|z;
                        break;
                }
            }
        }
        /* if we arrive here we are at the end of a DD/FD prefixed instruction,
           clear the prefix bits for the next instruction
        */
        map_bits &= ~(Z80_BIT_IX|Z80_BIT_IY);

        /* invoke optional user-trap callback */
        if (trap) {
            int trap_id = trap(c.pc, ticks, pins, cpu->trap_user_data);
            if (trap_id) {
                cpu->trap_id=trap_id;
                break;
            }
        }

        /* clear the interrupt pin state */
        pins &= ~Z80_INT;

        /* delay-enable IFF flags */
        if (c.bits & Z80_BIT_EI) {
            c.bits &= ~Z80_BIT_EI;
            c.bits |= (Z80_BIT_IFF1 | Z80_BIT_IFF2);
        }
        pre_pins = pins;
    } while (ticks < num_ticks);
    _z80_flush_ihl(&c, c.bits);
    c.bits = (c.bits & ~(Z80_BIT_IX|Z80_BIT_IY)) | map_bits;
    cpu->reg = c;
    cpu->pins = pins;
    return ticks;
}

#undef _SA
#undef _SAD
#undef _GD
#undef _T
#undef _TM
#undef _TWM
#undef _MR
#undef _MW
#undef _IN
#undef _OUT
#undef _IMM8
#undef _IMM16
#undef _ADDR
#undef _BUMPR
#undef _FETCH
#undef _FETCH_CB
#undef _SZ
#undef _SZYXCH
#undef _ADD_FLAGS
#undef _SUB_FLAGS
#undef _CP_FLAGS
#undef _SZIFF2_FLAGS

#endif /* CHIPS_IMPL */
