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
    - the RESET pin is currently not tested, call the z80_reset() 
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

    http://github.com/floooh/yakc

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

/*--- address bus pins ---*/
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

/*--- data bus pins ------*/
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
    z80_tick_t tick_cb;         /* tick callback */
    void* user_data;            /* optional user data for tick callback */
} z80_desc_t;

/* Z80 CPU state */
typedef struct {
    z80_tick_t tick_cb;
    uint64_t bc_de_hl_fa;
    uint64_t bc_de_hl_fa_;
    uint64_t wz_ix_iy_sp;
    uint64_t im_ir_pc_bits;     
    uint64_t pins;              /* only for debug inspection */
    void* user_data;
    z80_trap_t trap_cb;
    void* trap_user_data;
    int trap_id;                /* != 0 if a trap has been hit */
} z80_t;

/* initialize a new z80 instance */
void z80_init(z80_t* cpu, const z80_desc_t* desc);
/* reset an existing z80 instance */
void z80_reset(z80_t* cpu);
/* set optional trap callback function */
void z80_trap_cb(z80_t* cpu, z80_trap_t trap_cb, void* trap_user_data);
/* execute instructions for at least 'ticks', but at least one, return executed ticks */
uint32_t z80_exec(z80_t* cpu, uint32_t ticks);
/* return false if z80_exec() returned in the middle of an extended instruction */
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

/* register locations in register banks */
#define _A (0)
#define _F (8)
#define _L (16)
#define _H (24)
#define _E (32)
#define _D (40)
#define _C (48)
#define _B (56)
#define _FA (0)
#define _HL (16)
#define _DE (32)
#define _BC (48)
#define _SP (0)
#define _IY (16)
#define _IX (32)
#define _WZ (48)
#define _PC (16)
#define _IR (32)
#define _R  (32)
#define _I  (40)
#define _IM (48)
#define _USE_IX (0)
#define _USE_IY (1)
#define _IFF1 (2)
#define _IFF2 (3)
#define _EI   (4)
#define _BIT_USE_IX (1ULL<<_USE_IX)
#define _BIT_USE_IY (1ULL<<_USE_IY)
#define _BIT_IFF1   (1ULL<<_IFF1)
#define _BIT_IFF2   (1ULL<<_IFF2)
#define _BIT_EI     (1ULL<<_EI)
#define _BITS_USE_IXIY  (_BIT_USE_IX|_BIT_USE_IY)

/* set 8-bit immediate value in 64-bit register bank */
#define _S8(bank,shift,val) bank=(((bank)&~(0xFFULL<<(shift)))|(((val)&0xFFULL)<<(shift)))
/* extract 8-bit value from 64-bit register bank */
#define _G8(bank,shift) (((bank)>>(shift))&0xFFULL)
/* set 16-bit immediate value in 64-bit register bank */
#define _S16(bank,shift,val) bank=((bank&~(0xFFFFULL<<(shift)))|(((val)&0xFFFFULL)<<(shift)))
/* extract 16-bit value from 64-bit register bank */
#define _G16(bank,shift) (((bank)>>(shift))&0xFFFFULL)
/* set a single bit value in 64-bit register mask */
#define _S1(bank,shift,val) bank=(((bank)&~(1ULL<<(shift)))|(((val)&1ULL)<<(shift)))
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
/* invoke tick callback (with wait state detection) */
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
#define _IMM8(data) _MR(pc++,data);
/* read 16-bit immediate value (also update WZ register) */
#define _IMM16(data) {uint8_t w,z;_MR(pc++,z);_MR(pc++,w);data=(w<<8)|z;_S_WZ(data);} 
/* true if current op is an indexed op */
#define _IDX() (0!=(r2&_BITS_USE_IXIY))
/* generate effective address for (HL), (IX+d), (IY+d) */
#define _ADDR(addr,ext_ticks) {addr=_G16(ws,_HL);if(_IDX()){int8_t d;_MR(pc++,d);addr+=d;_S_WZ(addr);_T(ext_ticks);}}
/* helper macro to bump R register */
#define _BUMPR() d8=_G8(r2,_R);d8=(d8&0x80)|((d8+1)&0x7F);_S8(r2,_R,d8)
/* a normal opcode fetch, bump R */
#define _FETCH(op) {_SA(pc++);_TWM(4,Z80_M1|Z80_MREQ|Z80_RD);op=_GD();_BUMPR();}
/* special opcode fetch for CB prefix, only bump R if not a DD/FD+CB 'double prefix' op */
#define _FETCH_CB(op) {_SA(pc++);_TWM(4,Z80_M1|Z80_MREQ|Z80_RD);op=_GD();if(!_IDX()){_BUMPR();}}
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
#define _SZIFF2_FLAGS(val) ((_G_F()&Z80_CF)|_SZ(val)|(val&(Z80_YF|Z80_XF))|((r2&_BIT_IFF2)?Z80_PF:0))

/* register setter/getter shortcut macros */
#define _S_A(val)  _S8(ws,_A,val)
#define _S_F(val)  _S8(ws,_F,val)
#define _S_L(val)  _S8(ws,_L,val)
#define _S_H(val)  _S8(ws,_H,val)
#define _S_E(val)  _S8(ws,_E,val)
#define _S_D(val)  _S8(ws,_D,val)
#define _S_C(val)  _S8(ws,_C,val)
#define _S_B(val)  _S8(ws,_B,val)
#define _S_FA(val) _S16(ws,_FA,val)
#define _S_HL(val) _S16(ws,_HL,val)
#define _S_DE(val) _S16(ws,_DE,val)
#define _S_BC(val) _S16(ws,_BC,val)
#define _S_WZ(val) _S16(r1,_WZ,val)
#define _S_IX(val) _S16(r1,_IX,val)
#define _S_IY(val) _S16(r1,_IY,val)
#define _S_SP(val) _S16(r1,_SP,val)
#define _S_IM(val) _S8(r2,_IM,val)
#define _S_I(val)  _S8(r2,_I,val)
#define _S_R(val)  _S8(r2,_R,val)
#define _S_IR(val) _S16(r2,_IR,val)
#define _S_PC(val) _S16(r2,_PC,val)
#define _G_A()  _G8(ws,_A)
#define _G_F()  _G8(ws,_F)
#define _G_L()  _G8(ws,_L)
#define _G_H()  _G8(ws,_H)
#define _G_E()  _G8(ws,_E)
#define _G_D()  _G8(ws,_D)
#define _G_C()  _G8(ws,_C)
#define _G_B()  _G8(ws,_B)
#define _G_FA() _G16(ws,_FA)
#define _G_HL() _G16(ws,_HL)
#define _G_DE() _G16(ws,_DE)
#define _G_BC() _G16(ws,_BC)
#define _G_WZ() _G16(r1,_WZ)
#define _G_IX() _G16(r1,_IX)
#define _G_IY() _G16(r1,_IY)
#define _G_SP() _G16(r1,_SP)
#define _G_IM() _G8(r2,_IM)
#define _G_I()  _G8(r2,_I)
#define _G_R()  _G8(r2,_R)
#define _G_IR() _G16(r2,_IR)
#define _G_PC() _G16(r2,_PC)

/* register access functions */
void z80_set_a(z80_t* cpu, uint8_t v)         { _S8(cpu->bc_de_hl_fa,_A,v); }
void z80_set_f(z80_t* cpu, uint8_t v)         { _S8(cpu->bc_de_hl_fa,_F,v); }
void z80_set_l(z80_t* cpu, uint8_t v)         { _S8(cpu->bc_de_hl_fa,_L,v); }
void z80_set_h(z80_t* cpu, uint8_t v)         { _S8(cpu->bc_de_hl_fa,_H,v); }
void z80_set_e(z80_t* cpu, uint8_t v)         { _S8(cpu->bc_de_hl_fa,_E,v); }
void z80_set_d(z80_t* cpu, uint8_t v)         { _S8(cpu->bc_de_hl_fa,_D,v); }
void z80_set_c(z80_t* cpu, uint8_t v)         { _S8(cpu->bc_de_hl_fa,_C,v); }
void z80_set_b(z80_t* cpu, uint8_t v)         { _S8(cpu->bc_de_hl_fa,_B,v); }
void z80_set_af(z80_t* cpu, uint16_t v)       { _S16(cpu->bc_de_hl_fa,_FA,((v<<8)&0xFF00)|((v>>8)&0x00FF)); }
void z80_set_fa(z80_t* cpu, uint16_t v)       { _S16(cpu->bc_de_hl_fa,_FA,v); }
void z80_set_hl(z80_t* cpu, uint16_t v)       { _S16(cpu->bc_de_hl_fa,_HL,v); }
void z80_set_de(z80_t* cpu, uint16_t v)       { _S16(cpu->bc_de_hl_fa,_DE,v); }
void z80_set_bc(z80_t* cpu, uint16_t v)       { _S16(cpu->bc_de_hl_fa,_BC,v); }
void z80_set_fa_(z80_t* cpu, uint16_t v)      { _S16(cpu->bc_de_hl_fa_,_FA,v); }
void z80_set_af_(z80_t* cpu, uint16_t v)      { _S16(cpu->bc_de_hl_fa_,_FA,((v<<8)&0xFF00)|((v>>8)&0x00FF)); }
void z80_set_hl_(z80_t* cpu, uint16_t v)      { _S16(cpu->bc_de_hl_fa_,_HL,v); }
void z80_set_de_(z80_t* cpu, uint16_t v)      { _S16(cpu->bc_de_hl_fa_,_DE,v); }
void z80_set_bc_(z80_t* cpu, uint16_t v)      { _S16(cpu->bc_de_hl_fa_,_BC,v); }
void z80_set_sp(z80_t* cpu, uint16_t v)       { _S16(cpu->wz_ix_iy_sp,_SP,v); }
void z80_set_iy(z80_t* cpu, uint16_t v)       { _S16(cpu->wz_ix_iy_sp,_IY,v); }
void z80_set_ix(z80_t* cpu, uint16_t v)       { _S16(cpu->wz_ix_iy_sp,_IX,v); }
void z80_set_wz(z80_t* cpu, uint16_t v)       { _S16(cpu->wz_ix_iy_sp,_WZ,v); }
void z80_set_pc(z80_t* cpu, uint16_t v)       { _S16(cpu->im_ir_pc_bits,_PC,v); }
void z80_set_ir(z80_t* cpu, uint16_t v)       { _S16(cpu->im_ir_pc_bits,_IR,v); }
void z80_set_i(z80_t* cpu, uint8_t v)         { _S8(cpu->im_ir_pc_bits,_I,v); }
void z80_set_r(z80_t* cpu, uint8_t v)         { _S8(cpu->im_ir_pc_bits,_R,v); }
void z80_set_im(z80_t* cpu, uint8_t v)        { _S8(cpu->im_ir_pc_bits,_IM,v); }
void z80_set_iff1(z80_t* cpu, bool b)         { _S1(cpu->im_ir_pc_bits,_IFF1,(b?1:0)); }
void z80_set_iff2(z80_t* cpu, bool b)         { _S1(cpu->im_ir_pc_bits,_IFF2,(b?1:0)); }
void z80_set_ei_pending(z80_t* cpu, bool b)   { _S1(cpu->im_ir_pc_bits,_EI,(b?1:0)); }
uint8_t z80_a(z80_t* cpu)         { return _G8(cpu->bc_de_hl_fa,_A); }
uint8_t z80_f(z80_t* cpu)         { return _G8(cpu->bc_de_hl_fa,_F); }
uint8_t z80_l(z80_t* cpu)         { return _G8(cpu->bc_de_hl_fa,_L); }
uint8_t z80_h(z80_t* cpu)         { return _G8(cpu->bc_de_hl_fa,_H); }
uint8_t z80_e(z80_t* cpu)         { return _G8(cpu->bc_de_hl_fa,_E); }
uint8_t z80_d(z80_t* cpu)         { return _G8(cpu->bc_de_hl_fa,_D); }
uint8_t z80_c(z80_t* cpu)         { return _G8(cpu->bc_de_hl_fa,_C); }
uint8_t z80_b(z80_t* cpu)         { return _G8(cpu->bc_de_hl_fa,_B); }
uint16_t z80_fa(z80_t* cpu)       { return _G16(cpu->bc_de_hl_fa,_FA); }
uint16_t z80_af(z80_t* cpu)       { uint16_t d16=_G16(cpu->bc_de_hl_fa,_FA); return (d16<<8)|(d16>>8); }
uint16_t z80_hl(z80_t* cpu)       { return _G16(cpu->bc_de_hl_fa,_HL); }
uint16_t z80_de(z80_t* cpu)       { return _G16(cpu->bc_de_hl_fa,_DE); }
uint16_t z80_bc(z80_t* cpu)       { return _G16(cpu->bc_de_hl_fa,_BC); }
uint16_t z80_fa_(z80_t* cpu)      { return _G16(cpu->bc_de_hl_fa_,_FA); }
uint16_t z80_af_(z80_t* cpu)      { uint16_t d16=_G16(cpu->bc_de_hl_fa_,_FA); return (d16<<8)|(d16>>8); }
uint16_t z80_hl_(z80_t* cpu)      { return _G16(cpu->bc_de_hl_fa_,_HL); }
uint16_t z80_de_(z80_t* cpu)      { return _G16(cpu->bc_de_hl_fa_,_DE); }
uint16_t z80_bc_(z80_t* cpu)      { return _G16(cpu->bc_de_hl_fa_,_BC); }
uint16_t z80_sp(z80_t* cpu)       { return _G16(cpu->wz_ix_iy_sp,_SP); }
uint16_t z80_iy(z80_t* cpu)       { return _G16(cpu->wz_ix_iy_sp,_IY); }
uint16_t z80_ix(z80_t* cpu)       { return _G16(cpu->wz_ix_iy_sp,_IX); }
uint16_t z80_wz(z80_t* cpu)       { return _G16(cpu->wz_ix_iy_sp,_WZ); }
uint16_t z80_pc(z80_t* cpu)       { return _G16(cpu->im_ir_pc_bits,_PC); }
uint16_t z80_ir(z80_t* cpu)       { return _G16(cpu->im_ir_pc_bits,_IR); }
uint8_t z80_i(z80_t* cpu)         { return _G8(cpu->im_ir_pc_bits,_I); }
uint8_t z80_r(z80_t* cpu)         { return _G8(cpu->im_ir_pc_bits,_R); }
uint8_t z80_im(z80_t* cpu)        { return _G8(cpu->im_ir_pc_bits,_IM); }
bool z80_iff1(z80_t* cpu)         { return 0 != (cpu->im_ir_pc_bits & _BIT_IFF1); }
bool z80_iff2(z80_t* cpu)         { return 0 != (cpu->im_ir_pc_bits & _BIT_IFF2); }
bool z80_ei_pending(z80_t* cpu)   { return 0 != (cpu->im_ir_pc_bits & _BIT_EI); }

void z80_init(z80_t* cpu, const z80_desc_t* desc) {
    CHIPS_ASSERT(_FA == 0);
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
    cpu->bc_de_hl_fa = cpu->bc_de_hl_fa_ = 0xFFFFFFFFFFFFFFFFULL;
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
    cpu->im_ir_pc_bits &= ~(_BIT_EI|_BIT_USE_IX|_BIT_USE_IY);
}

void z80_trap_cb(z80_t* cpu, z80_trap_t trap_cb, void* trap_user_data) {
    CHIPS_ASSERT(cpu);
    cpu->trap_cb = trap_cb;
    cpu->trap_user_data = trap_user_data;
}

bool z80_opdone(z80_t* cpu) {
    return 0 == (cpu->im_ir_pc_bits & _BITS_USE_IXIY);
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

/* DAA instruction */
static inline uint64_t _z80_daa(uint64_t ws) {
    uint8_t a = _G8(ws,_A);
    uint8_t v = a;
    uint8_t f = _G8(ws,_F);
    if (f & Z80_NF) {
        if (((a & 0xF)>0x9) || (f & Z80_HF)) {
            v -= 0x06;
        }
        if ((a > 0x99) || (f & Z80_CF)) {
            v -= 0x60;
        }
    }
    else {
        if (((a & 0xF)>0x9) || (f & Z80_HF)) {
            v += 0x06;
        }
        if ((a > 0x99) || (f & Z80_CF)) {
            v += 0x60;
        }
    }
    f &= Z80_CF|Z80_NF;
    f |= (a>0x99) ? Z80_CF : 0;
    f |= (a ^ v) & Z80_HF;
    f |= _z80_szp[v];
    _S8(ws,_A,v);
    _S8(ws,_F,f);
    return ws;
}

/* get 'working set' register bank with renamed HL <=> IX/IY */
static inline uint64_t _z80_map_regs(uint64_t r0, uint64_t r1, uint64_t r2) {
    uint64_t ws = r0;
    if (r2 & _BIT_USE_IX) {
        ws = (ws & ~(0xFFFFULL<<_HL)) | (((r1>>_IX)<<_HL) & (0xFFFFULL<<_HL));
    }
    else if (r2 & _BIT_USE_IY) {
        ws = (ws & ~(0xFFFFULL<<_HL)) | (((r1>>_IY)<<_HL) & (0xFFFFULL<<_HL));
    }
    return ws;
}

/* write HL <=> IX/IY register-renamed working set back to actual register banks */
static inline uint64_t _z80_flush_r0(uint64_t ws, uint64_t r0, uint64_t map_bits) {
    if (map_bits & _BITS_USE_IXIY) {
        r0 = (r0 & (0xFFFFULL<<_HL)) | (ws & ~(0xFFFFULL<<_HL));
    }
    else {
        r0 = ws;
    }
    return r0;
}

static inline uint64_t _z80_flush_r1(uint64_t ws, uint64_t r1, uint64_t map_bits) {
    if (map_bits & _BIT_USE_IX) {
        r1 = (r1 & ~(0xFFFFULL<<_IX)) | (((ws>>_HL)<<_IX) & (0xFFFFULL<<_IX));
    }
    else if (map_bits & _BIT_USE_IY) {
        r1 = (r1 & ~(0xFFFFULL<<_IY)) | (((ws>>_HL)<<_IY) & (0xFFFFULL<<_IY));
    }
    return r1;
}

/* instruction decoder */
uint32_t z80_exec(z80_t* cpu, uint32_t num_ticks) {
    cpu->trap_id = 0;
    uint64_t r0 = cpu->bc_de_hl_fa;
    uint64_t r1 = cpu->wz_ix_iy_sp;
    uint64_t r2 = cpu->im_ir_pc_bits;
    uint64_t r3 = cpu->bc_de_hl_fa_;
    uint64_t ws = _z80_map_regs(r0, r1, r2);
    uint64_t map_bits = r2 & _BITS_USE_IXIY;
    uint64_t pins = cpu->pins;
    const z80_tick_t tick = cpu->tick_cb;
    const z80_trap_t trap = cpu->trap_cb;
    void* ud = cpu->user_data;
    uint32_t ticks = 0;
    uint8_t op = 0, d8 = 0;
    uint16_t addr = 0, d16 = 0;
    uint16_t pc = _G_PC();
    uint64_t pre_pins = pins;
    do {
        /* fetch next opcode byte */
        _FETCH(op)
        /* special case ED-prefixed instruction: cancel effect of DD/FD prefix */
        if (op == 0xED) {
            map_bits &= ~_BITS_USE_IXIY;
        }
        /* handle HL <=> IX/IY renaming for indexed ops */
        if (map_bits != (r2 & _BITS_USE_IXIY)) {
            const uint64_t old_map_bits = r2 & _BITS_USE_IXIY;
            r0 = _z80_flush_r0(ws, r0, old_map_bits);
            r1 = _z80_flush_r1(ws, r1, old_map_bits);
            r2 = (r2 & ~_BITS_USE_IXIY) | map_bits;
            ws = _z80_map_regs(r0, r1, r2);
        }
        /* decode instruction */
        switch (op) {
            case 0x0:/*NOP*/ break;
            case 0x1:/*LD BC,nn*/_IMM16(d16);_S_BC(d16);break;
            case 0x2:/*LD (BC),A*/addr=_G_BC();d8=_G_A();_MW(addr++,d8);_S_WZ((d8<<8)|(addr&0x00FF));break;
            case 0x3:/*INC BC*/_T(2);_S_BC(_G_BC()+1);break;
            case 0x4:/*INC B*/d8=_G_B();{uint8_t r=d8+1;uint8_t f=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x80){f|=Z80_VF;}_S_F(f|(_G_F()&Z80_CF));d8=r;}_S_B(d8);break;
            case 0x5:/*DEC B*/d8=_G_B();{uint8_t r=d8-1;uint8_t f=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x7F){f|=Z80_VF;}_S_F(f|(_G_F()&Z80_CF));d8=r;}_S_B(d8);break;
            case 0x6:/*LD B,n*/_IMM8(d8);_S_B(d8);break;
            case 0x7:/*RLCA*/{uint8_t a=_G_A();uint8_t f=_G_F();uint8_t r=(a<<1)|(a>>7);f=((a>>7)&Z80_CF)|(f&(Z80_SF|Z80_ZF|Z80_PF))|(r&(Z80_YF|Z80_XF));_S_A(r);_S_F(f);}break;
            case 0x8:/*EX AF,AF'*/{r0=_z80_flush_r0(ws,r0,r2);uint16_t fa=_G16(r0,_FA);uint16_t fa_=_G16(r3,_FA);_S16(r0,_FA,fa_);_S16(r3,_FA,fa);ws=_z80_map_regs(r0,r1,r2);}break;
            case 0x9:/*ADD HL,BC*/{uint16_t acc=_G_HL();_S_WZ(acc+1);d16=_G_BC();uint32_t r=acc+d16;_S_HL(r);uint8_t f=_G_F()&(Z80_SF|Z80_ZF|Z80_VF);f|=((acc^r^d16)>>8)&Z80_HF;f|=((r>>16)&Z80_CF)|((r>>8)&(Z80_YF|Z80_XF));_S_F(f);_T(7);}break;
            case 0xa:/*LD A,(BC)*/addr=_G_BC();_MR(addr++,d8);_S_A(d8);_S_WZ(addr);break;
            case 0xb:/*DEC BC*/_T(2);_S_BC(_G_BC()-1);break;
            case 0xc:/*INC C*/d8=_G_C();{uint8_t r=d8+1;uint8_t f=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x80){f|=Z80_VF;}_S_F(f|(_G_F()&Z80_CF));d8=r;}_S_C(d8);break;
            case 0xd:/*DEC C*/d8=_G_C();{uint8_t r=d8-1;uint8_t f=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x7F){f|=Z80_VF;}_S_F(f|(_G_F()&Z80_CF));d8=r;}_S_C(d8);break;
            case 0xe:/*LD C,n*/_IMM8(d8);_S_C(d8);break;
            case 0xf:/*RRCA*/{uint8_t a=_G_A();uint8_t f=_G_F();uint8_t r=(a>>1)|(a<<7);f=(a&Z80_CF)|(f&(Z80_SF|Z80_ZF|Z80_PF))|(r&(Z80_YF|Z80_XF));_S_A(r);_S_F(f);}break;
            case 0x10:/*DJNZ*/{_T(1);int8_t d;_IMM8(d);d8=_G_B()-1;_S_B(d8);if(d8>0){pc+=d;_S_WZ(pc);_T(5);}}break;
            case 0x11:/*LD DE,nn*/_IMM16(d16);_S_DE(d16);break;
            case 0x12:/*LD (DE),A*/addr=_G_DE();d8=_G_A();_MW(addr++,d8);_S_WZ((d8<<8)|(addr&0x00FF));break;
            case 0x13:/*INC DE*/_T(2);_S_DE(_G_DE()+1);break;
            case 0x14:/*INC D*/d8=_G_D();{uint8_t r=d8+1;uint8_t f=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x80){f|=Z80_VF;}_S_F(f|(_G_F()&Z80_CF));d8=r;}_S_D(d8);break;
            case 0x15:/*DEC D*/d8=_G_D();{uint8_t r=d8-1;uint8_t f=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x7F){f|=Z80_VF;}_S_F(f|(_G_F()&Z80_CF));d8=r;}_S_D(d8);break;
            case 0x16:/*LD D,n*/_IMM8(d8);_S_D(d8);break;
            case 0x17:/*RLA*/{uint8_t a=_G_A();uint8_t f=_G_F();uint8_t r=(a<<1)|(f&Z80_CF);f=((a>>7)&Z80_CF)|(f&(Z80_SF|Z80_ZF|Z80_PF))|(r&(Z80_YF|Z80_XF));_S_A(r);_S_F(f);}break;
            case 0x18:/*JR d*/{int8_t d;_IMM8(d);pc+=d;_S_WZ(pc);_T(5);}break;
            case 0x19:/*ADD HL,DE*/{uint16_t acc=_G_HL();_S_WZ(acc+1);d16=_G_DE();uint32_t r=acc+d16;_S_HL(r);uint8_t f=_G_F()&(Z80_SF|Z80_ZF|Z80_VF);f|=((acc^r^d16)>>8)&Z80_HF;f|=((r>>16)&Z80_CF)|((r>>8)&(Z80_YF|Z80_XF));_S_F(f);_T(7);}break;
            case 0x1a:/*LD A,(DE)*/addr=_G_DE();_MR(addr++,d8);_S_A(d8);_S_WZ(addr);break;
            case 0x1b:/*DEC DE*/_T(2);_S_DE(_G_DE()-1);break;
            case 0x1c:/*INC E*/d8=_G_E();{uint8_t r=d8+1;uint8_t f=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x80){f|=Z80_VF;}_S_F(f|(_G_F()&Z80_CF));d8=r;}_S_E(d8);break;
            case 0x1d:/*DEC E*/d8=_G_E();{uint8_t r=d8-1;uint8_t f=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x7F){f|=Z80_VF;}_S_F(f|(_G_F()&Z80_CF));d8=r;}_S_E(d8);break;
            case 0x1e:/*LD E,n*/_IMM8(d8);_S_E(d8);break;
            case 0x1f:/*RRA*/{uint8_t a=_G_A();uint8_t f=_G_F();uint8_t r=(a>>1)|((f&Z80_CF)<<7);f=(a&Z80_CF)|(f&(Z80_SF|Z80_ZF|Z80_PF))|(r&(Z80_YF|Z80_XF));_S_A(r);_S_F(f);}break;
            case 0x20:/*JR NZ,d*/{int8_t d;_IMM8(d);if(!(_G_F()&Z80_ZF)){pc+=d;_S_WZ(pc);_T(5);}}break;
            case 0x21:/*LD HL,nn*/_IMM16(d16);_S_HL(d16);break;
            case 0x22:/*LD (nn),HL*/_IMM16(addr);_MW(addr++,_G_L());_MW(addr,_G_H());_S_WZ(addr);break;
            case 0x23:/*INC HL*/_T(2);_S_HL(_G_HL()+1);break;
            case 0x24:/*INC H*/d8=_G_H();{uint8_t r=d8+1;uint8_t f=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x80){f|=Z80_VF;}_S_F(f|(_G_F()&Z80_CF));d8=r;}_S_H(d8);break;
            case 0x25:/*DEC H*/d8=_G_H();{uint8_t r=d8-1;uint8_t f=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x7F){f|=Z80_VF;}_S_F(f|(_G_F()&Z80_CF));d8=r;}_S_H(d8);break;
            case 0x26:/*LD H,n*/_IMM8(d8);_S_H(d8);break;
            case 0x27:/*DAA*/ws=_z80_daa(ws);break;
            case 0x28:/*JR Z,d*/{int8_t d;_IMM8(d);if((_G_F()&Z80_ZF)){pc+=d;_S_WZ(pc);_T(5);}}break;
            case 0x29:/*ADD HL,HL*/{uint16_t acc=_G_HL();_S_WZ(acc+1);d16=_G_HL();uint32_t r=acc+d16;_S_HL(r);uint8_t f=_G_F()&(Z80_SF|Z80_ZF|Z80_VF);f|=((acc^r^d16)>>8)&Z80_HF;f|=((r>>16)&Z80_CF)|((r>>8)&(Z80_YF|Z80_XF));_S_F(f);_T(7);}break;
            case 0x2a:/*LD HL,(nn)*/_IMM16(addr);_MR(addr++,d8);_S_L(d8);_MR(addr,d8);_S_H(d8);_S_WZ(addr);break;
            case 0x2b:/*DEC HL*/_T(2);_S_HL(_G_HL()-1);break;
            case 0x2c:/*INC L*/d8=_G_L();{uint8_t r=d8+1;uint8_t f=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x80){f|=Z80_VF;}_S_F(f|(_G_F()&Z80_CF));d8=r;}_S_L(d8);break;
            case 0x2d:/*DEC L*/d8=_G_L();{uint8_t r=d8-1;uint8_t f=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x7F){f|=Z80_VF;}_S_F(f|(_G_F()&Z80_CF));d8=r;}_S_L(d8);break;
            case 0x2e:/*LD L,n*/_IMM8(d8);_S_L(d8);break;
            case 0x2f:/*CPL*/{uint8_t a=_G_A()^0xFF;_S_A(a);uint8_t f=_G_F();f=(f&(Z80_SF|Z80_ZF|Z80_PF|Z80_CF))|Z80_HF|Z80_NF|(a&(Z80_YF|Z80_XF));_S_F(f);}break;
            case 0x30:/*JR NC,d*/{int8_t d;_IMM8(d);if(!(_G_F()&Z80_CF)){pc+=d;_S_WZ(pc);_T(5);}}break;
            case 0x31:/*LD SP,nn*/_IMM16(d16);_S_SP(d16);break;
            case 0x32:/*LD (nn),A*/_IMM16(addr);d8=_G_A();_MW(addr++,d8);_S_WZ((d8<<8)|(addr&0x00FF));break;
            case 0x33:/*INC SP*/_T(2);_S_SP(_G_SP()+1);break;
            case 0x34:/*INC (HL/IX+d/IY+d)*/_ADDR(addr,5);_T(1);_MR(addr,d8);{uint8_t r=d8+1;uint8_t f=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x80){f|=Z80_VF;}_S_F(f|(_G_F()&Z80_CF));d8=r;}_MW(addr,d8);break;
            case 0x35:/*DEC (HL/IX+d/IY+d)*/_ADDR(addr,5);_T(1);_MR(addr,d8);{uint8_t r=d8-1;uint8_t f=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x7F){f|=Z80_VF;}_S_F(f|(_G_F()&Z80_CF));d8=r;}_MW(addr,d8);break;
            case 0x36:/*LD (HL/IX+d/IY+d),n*/_ADDR(addr,2);_IMM8(d8);_MW(addr,d8);break;
            case 0x37:/*SCF*/{uint8_t a=_G_A();uint8_t f=_G_F();f=(f&(Z80_SF|Z80_ZF|Z80_PF|Z80_CF))|Z80_CF|(a&(Z80_YF|Z80_XF));_S_F(f);}break;
            case 0x38:/*JR C,d*/{int8_t d;_IMM8(d);if((_G_F()&Z80_CF)){pc+=d;_S_WZ(pc);_T(5);}}break;
            case 0x39:/*ADD HL,SP*/{uint16_t acc=_G_HL();_S_WZ(acc+1);d16=_G_SP();uint32_t r=acc+d16;_S_HL(r);uint8_t f=_G_F()&(Z80_SF|Z80_ZF|Z80_VF);f|=((acc^r^d16)>>8)&Z80_HF;f|=((r>>16)&Z80_CF)|((r>>8)&(Z80_YF|Z80_XF));_S_F(f);_T(7);}break;
            case 0x3a:/*LD A,(nn)*/_IMM16(addr);_MR(addr++,d8);_S_A(d8);_S_WZ(addr);break;
            case 0x3b:/*DEC SP*/_T(2);_S_SP(_G_SP()-1);break;
            case 0x3c:/*INC A*/d8=_G_A();{uint8_t r=d8+1;uint8_t f=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x80){f|=Z80_VF;}_S_F(f|(_G_F()&Z80_CF));d8=r;}_S_A(d8);break;
            case 0x3d:/*DEC A*/d8=_G_A();{uint8_t r=d8-1;uint8_t f=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x7F){f|=Z80_VF;}_S_F(f|(_G_F()&Z80_CF));d8=r;}_S_A(d8);break;
            case 0x3e:/*LD A,n*/_IMM8(d8);_S_A(d8);break;
            case 0x3f:/*CCF*/{uint8_t a=_G_A();uint8_t f=_G_F();f=((f&(Z80_SF|Z80_ZF|Z80_PF|Z80_CF))|((f&Z80_CF)<<4)|(a&(Z80_YF|Z80_XF)))^Z80_CF;_S_F(f);}break;
            case 0x40:/*LD B,B*/_S_B(_G_B());break;
            case 0x41:/*LD B,C*/_S_B(_G_C());break;
            case 0x42:/*LD B,D*/_S_B(_G_D());break;
            case 0x43:/*LD B,E*/_S_B(_G_E());break;
            case 0x44:/*LD B,H*/_S_B(_G_H());break;
            case 0x45:/*LD B,L*/_S_B(_G_L());break;
            case 0x46:/*LD B,(HL/IX+d/IY+d)*/_ADDR(addr,5);_MR(addr,d8);_S_B(d8);break;
            case 0x47:/*LD B,A*/_S_B(_G_A());break;
            case 0x48:/*LD C,B*/_S_C(_G_B());break;
            case 0x49:/*LD C,C*/_S_C(_G_C());break;
            case 0x4a:/*LD C,D*/_S_C(_G_D());break;
            case 0x4b:/*LD C,E*/_S_C(_G_E());break;
            case 0x4c:/*LD C,H*/_S_C(_G_H());break;
            case 0x4d:/*LD C,L*/_S_C(_G_L());break;
            case 0x4e:/*LD C,(HL/IX+d/IY+d)*/_ADDR(addr,5);_MR(addr,d8);_S_C(d8);break;
            case 0x4f:/*LD C,A*/_S_C(_G_A());break;
            case 0x50:/*LD D,B*/_S_D(_G_B());break;
            case 0x51:/*LD D,C*/_S_D(_G_C());break;
            case 0x52:/*LD D,D*/_S_D(_G_D());break;
            case 0x53:/*LD D,E*/_S_D(_G_E());break;
            case 0x54:/*LD D,H*/_S_D(_G_H());break;
            case 0x55:/*LD D,L*/_S_D(_G_L());break;
            case 0x56:/*LD D,(HL/IX+d/IY+d)*/_ADDR(addr,5);_MR(addr,d8);_S_D(d8);break;
            case 0x57:/*LD D,A*/_S_D(_G_A());break;
            case 0x58:/*LD E,B*/_S_E(_G_B());break;
            case 0x59:/*LD E,C*/_S_E(_G_C());break;
            case 0x5a:/*LD E,D*/_S_E(_G_D());break;
            case 0x5b:/*LD E,E*/_S_E(_G_E());break;
            case 0x5c:/*LD E,H*/_S_E(_G_H());break;
            case 0x5d:/*LD E,L*/_S_E(_G_L());break;
            case 0x5e:/*LD E,(HL/IX+d/IY+d)*/_ADDR(addr,5);_MR(addr,d8);_S_E(d8);break;
            case 0x5f:/*LD E,A*/_S_E(_G_A());break;
            case 0x60:/*LD H,B*/_S_H(_G_B());break;
            case 0x61:/*LD H,C*/_S_H(_G_C());break;
            case 0x62:/*LD H,D*/_S_H(_G_D());break;
            case 0x63:/*LD H,E*/_S_H(_G_E());break;
            case 0x64:/*LD H,H*/_S_H(_G_H());break;
            case 0x65:/*LD H,L*/_S_H(_G_L());break;
            case 0x66:/*LD H,(HL/IX+d/IY+d)*/_ADDR(addr,5);_MR(addr,d8);if(_IDX()){_S8(r0,_H,d8);}else{_S_H(d8);}break;
            case 0x67:/*LD H,A*/_S_H(_G_A());break;
            case 0x68:/*LD L,B*/_S_L(_G_B());break;
            case 0x69:/*LD L,C*/_S_L(_G_C());break;
            case 0x6a:/*LD L,D*/_S_L(_G_D());break;
            case 0x6b:/*LD L,E*/_S_L(_G_E());break;
            case 0x6c:/*LD L,H*/_S_L(_G_H());break;
            case 0x6d:/*LD L,L*/_S_L(_G_L());break;
            case 0x6e:/*LD L,(HL/IX+d/IY+d)*/_ADDR(addr,5);_MR(addr,d8);if(_IDX()){_S8(r0,_L,d8);}else{_S_L(d8);}break;
            case 0x6f:/*LD L,A*/_S_L(_G_A());break;
            case 0x70:/*LD (HL/IX+d/IY+d),B*/d8=_G_B();_ADDR(addr,5);_MW(addr,d8);break;
            case 0x71:/*LD (HL/IX+d/IY+d),C*/d8=_G_C();_ADDR(addr,5);_MW(addr,d8);break;
            case 0x72:/*LD (HL/IX+d/IY+d),D*/d8=_G_D();_ADDR(addr,5);_MW(addr,d8);break;
            case 0x73:/*LD (HL/IX+d/IY+d),E*/d8=_G_E();_ADDR(addr,5);_MW(addr,d8);break;
            case 0x74:/*LD (HL/IX+d/IY+d),H*/d8=_IDX()?_G8(r0,_H):_G_H();_ADDR(addr,5);_MW(addr,d8);break;
            case 0x75:/*LD (HL/IX+d/IY+d),L*/d8=_IDX()?_G8(r0,_L):_G_L();_ADDR(addr,5);_MW(addr,d8);break;
            case 0x76:/*HALT*/pins|=Z80_HALT;pc--;break;
            case 0x77:/*LD (HL/IX+d/IY+d),A*/d8=_G_A();_ADDR(addr,5);_MW(addr,d8);break;
            case 0x78:/*LD A,B*/_S_A(_G_B());break;
            case 0x79:/*LD A,C*/_S_A(_G_C());break;
            case 0x7a:/*LD A,D*/_S_A(_G_D());break;
            case 0x7b:/*LD A,E*/_S_A(_G_E());break;
            case 0x7c:/*LD A,H*/_S_A(_G_H());break;
            case 0x7d:/*LD A,L*/_S_A(_G_L());break;
            case 0x7e:/*LD A,(HL/IX+d/IY+d)*/_ADDR(addr,5);_MR(addr,d8);_S_A(d8);break;
            case 0x7f:/*LD A,A*/_S_A(_G_A());break;
            case 0x80:/*ADD B*/d8=_G_B();{uint8_t acc=_G_A();uint32_t res=acc+d8;_S_F(_ADD_FLAGS(acc,d8,res));_S_A(res);}break;
            case 0x81:/*ADD C*/d8=_G_C();{uint8_t acc=_G_A();uint32_t res=acc+d8;_S_F(_ADD_FLAGS(acc,d8,res));_S_A(res);}break;
            case 0x82:/*ADD D*/d8=_G_D();{uint8_t acc=_G_A();uint32_t res=acc+d8;_S_F(_ADD_FLAGS(acc,d8,res));_S_A(res);}break;
            case 0x83:/*ADD E*/d8=_G_E();{uint8_t acc=_G_A();uint32_t res=acc+d8;_S_F(_ADD_FLAGS(acc,d8,res));_S_A(res);}break;
            case 0x84:/*ADD H*/d8=_G_H();{uint8_t acc=_G_A();uint32_t res=acc+d8;_S_F(_ADD_FLAGS(acc,d8,res));_S_A(res);}break;
            case 0x85:/*ADD L*/d8=_G_L();{uint8_t acc=_G_A();uint32_t res=acc+d8;_S_F(_ADD_FLAGS(acc,d8,res));_S_A(res);}break;
            case 0x86:/*ADD,(HL/IX+d/IY+d)*/_ADDR(addr,5);_MR(addr,d8);{uint8_t acc=_G_A();uint32_t res=acc+d8;_S_F(_ADD_FLAGS(acc,d8,res));_S_A(res);}break;
            case 0x87:/*ADD A*/d8=_G_A();{uint8_t acc=_G_A();uint32_t res=acc+d8;_S_F(_ADD_FLAGS(acc,d8,res));_S_A(res);}break;
            case 0x88:/*ADC B*/d8=_G_B();{uint8_t acc=_G_A();uint32_t res=acc+d8+(_G_F()&Z80_CF);_S_F(_ADD_FLAGS(acc,d8,res));_S_A(res);}break;
            case 0x89:/*ADC C*/d8=_G_C();{uint8_t acc=_G_A();uint32_t res=acc+d8+(_G_F()&Z80_CF);_S_F(_ADD_FLAGS(acc,d8,res));_S_A(res);}break;
            case 0x8a:/*ADC D*/d8=_G_D();{uint8_t acc=_G_A();uint32_t res=acc+d8+(_G_F()&Z80_CF);_S_F(_ADD_FLAGS(acc,d8,res));_S_A(res);}break;
            case 0x8b:/*ADC E*/d8=_G_E();{uint8_t acc=_G_A();uint32_t res=acc+d8+(_G_F()&Z80_CF);_S_F(_ADD_FLAGS(acc,d8,res));_S_A(res);}break;
            case 0x8c:/*ADC H*/d8=_G_H();{uint8_t acc=_G_A();uint32_t res=acc+d8+(_G_F()&Z80_CF);_S_F(_ADD_FLAGS(acc,d8,res));_S_A(res);}break;
            case 0x8d:/*ADC L*/d8=_G_L();{uint8_t acc=_G_A();uint32_t res=acc+d8+(_G_F()&Z80_CF);_S_F(_ADD_FLAGS(acc,d8,res));_S_A(res);}break;
            case 0x8e:/*ADC,(HL/IX+d/IY+d)*/_ADDR(addr,5);_MR(addr,d8);{uint8_t acc=_G_A();uint32_t res=acc+d8+(_G_F()&Z80_CF);_S_F(_ADD_FLAGS(acc,d8,res));_S_A(res);}break;
            case 0x8f:/*ADC A*/d8=_G_A();{uint8_t acc=_G_A();uint32_t res=acc+d8+(_G_F()&Z80_CF);_S_F(_ADD_FLAGS(acc,d8,res));_S_A(res);}break;
            case 0x90:/*SUB B*/d8=_G_B();{uint8_t acc=_G_A();uint32_t res=(uint32_t)((int)acc-(int)d8);_S_F(_SUB_FLAGS(acc,d8,res));_S_A(res);}break;
            case 0x91:/*SUB C*/d8=_G_C();{uint8_t acc=_G_A();uint32_t res=(uint32_t)((int)acc-(int)d8);_S_F(_SUB_FLAGS(acc,d8,res));_S_A(res);}break;
            case 0x92:/*SUB D*/d8=_G_D();{uint8_t acc=_G_A();uint32_t res=(uint32_t)((int)acc-(int)d8);_S_F(_SUB_FLAGS(acc,d8,res));_S_A(res);}break;
            case 0x93:/*SUB E*/d8=_G_E();{uint8_t acc=_G_A();uint32_t res=(uint32_t)((int)acc-(int)d8);_S_F(_SUB_FLAGS(acc,d8,res));_S_A(res);}break;
            case 0x94:/*SUB H*/d8=_G_H();{uint8_t acc=_G_A();uint32_t res=(uint32_t)((int)acc-(int)d8);_S_F(_SUB_FLAGS(acc,d8,res));_S_A(res);}break;
            case 0x95:/*SUB L*/d8=_G_L();{uint8_t acc=_G_A();uint32_t res=(uint32_t)((int)acc-(int)d8);_S_F(_SUB_FLAGS(acc,d8,res));_S_A(res);}break;
            case 0x96:/*SUB,(HL/IX+d/IY+d)*/_ADDR(addr,5);_MR(addr,d8);{uint8_t acc=_G_A();uint32_t res=(uint32_t)((int)acc-(int)d8);_S_F(_SUB_FLAGS(acc,d8,res));_S_A(res);}break;
            case 0x97:/*SUB A*/d8=_G_A();{uint8_t acc=_G_A();uint32_t res=(uint32_t)((int)acc-(int)d8);_S_F(_SUB_FLAGS(acc,d8,res));_S_A(res);}break;
            case 0x98:/*SBC B*/d8=_G_B();{uint8_t acc=_G_A();uint32_t res=(uint32_t)((int)acc-(int)d8-(_G_F()&Z80_CF));_S_F(_SUB_FLAGS(acc,d8,res));_S_A(res);}break;
            case 0x99:/*SBC C*/d8=_G_C();{uint8_t acc=_G_A();uint32_t res=(uint32_t)((int)acc-(int)d8-(_G_F()&Z80_CF));_S_F(_SUB_FLAGS(acc,d8,res));_S_A(res);}break;
            case 0x9a:/*SBC D*/d8=_G_D();{uint8_t acc=_G_A();uint32_t res=(uint32_t)((int)acc-(int)d8-(_G_F()&Z80_CF));_S_F(_SUB_FLAGS(acc,d8,res));_S_A(res);}break;
            case 0x9b:/*SBC E*/d8=_G_E();{uint8_t acc=_G_A();uint32_t res=(uint32_t)((int)acc-(int)d8-(_G_F()&Z80_CF));_S_F(_SUB_FLAGS(acc,d8,res));_S_A(res);}break;
            case 0x9c:/*SBC H*/d8=_G_H();{uint8_t acc=_G_A();uint32_t res=(uint32_t)((int)acc-(int)d8-(_G_F()&Z80_CF));_S_F(_SUB_FLAGS(acc,d8,res));_S_A(res);}break;
            case 0x9d:/*SBC L*/d8=_G_L();{uint8_t acc=_G_A();uint32_t res=(uint32_t)((int)acc-(int)d8-(_G_F()&Z80_CF));_S_F(_SUB_FLAGS(acc,d8,res));_S_A(res);}break;
            case 0x9e:/*SBC,(HL/IX+d/IY+d)*/_ADDR(addr,5);_MR(addr,d8);{uint8_t acc=_G_A();uint32_t res=(uint32_t)((int)acc-(int)d8-(_G_F()&Z80_CF));_S_F(_SUB_FLAGS(acc,d8,res));_S_A(res);}break;
            case 0x9f:/*SBC A*/d8=_G_A();{uint8_t acc=_G_A();uint32_t res=(uint32_t)((int)acc-(int)d8-(_G_F()&Z80_CF));_S_F(_SUB_FLAGS(acc,d8,res));_S_A(res);}break;
            case 0xa0:/*AND B*/d8=_G_B();{d8&=_G_A();_S_F(_z80_szp[d8]|Z80_HF);_S_A(d8);}break;
            case 0xa1:/*AND C*/d8=_G_C();{d8&=_G_A();_S_F(_z80_szp[d8]|Z80_HF);_S_A(d8);}break;
            case 0xa2:/*AND D*/d8=_G_D();{d8&=_G_A();_S_F(_z80_szp[d8]|Z80_HF);_S_A(d8);}break;
            case 0xa3:/*AND E*/d8=_G_E();{d8&=_G_A();_S_F(_z80_szp[d8]|Z80_HF);_S_A(d8);}break;
            case 0xa4:/*AND H*/d8=_G_H();{d8&=_G_A();_S_F(_z80_szp[d8]|Z80_HF);_S_A(d8);}break;
            case 0xa5:/*AND L*/d8=_G_L();{d8&=_G_A();_S_F(_z80_szp[d8]|Z80_HF);_S_A(d8);}break;
            case 0xa6:/*AND,(HL/IX+d/IY+d)*/_ADDR(addr,5);_MR(addr,d8);{d8&=_G_A();_S_F(_z80_szp[d8]|Z80_HF);_S_A(d8);}break;
            case 0xa7:/*AND A*/d8=_G_A();{d8&=_G_A();_S_F(_z80_szp[d8]|Z80_HF);_S_A(d8);}break;
            case 0xa8:/*XOR B*/d8=_G_B();{d8^=_G_A();_S_F(_z80_szp[d8]);_S_A(d8);}break;
            case 0xa9:/*XOR C*/d8=_G_C();{d8^=_G_A();_S_F(_z80_szp[d8]);_S_A(d8);}break;
            case 0xaa:/*XOR D*/d8=_G_D();{d8^=_G_A();_S_F(_z80_szp[d8]);_S_A(d8);}break;
            case 0xab:/*XOR E*/d8=_G_E();{d8^=_G_A();_S_F(_z80_szp[d8]);_S_A(d8);}break;
            case 0xac:/*XOR H*/d8=_G_H();{d8^=_G_A();_S_F(_z80_szp[d8]);_S_A(d8);}break;
            case 0xad:/*XOR L*/d8=_G_L();{d8^=_G_A();_S_F(_z80_szp[d8]);_S_A(d8);}break;
            case 0xae:/*XOR,(HL/IX+d/IY+d)*/_ADDR(addr,5);_MR(addr,d8);{d8^=_G_A();_S_F(_z80_szp[d8]);_S_A(d8);}break;
            case 0xaf:/*XOR A*/d8=_G_A();{d8^=_G_A();_S_F(_z80_szp[d8]);_S_A(d8);}break;
            case 0xb0:/*OR B*/d8=_G_B();{d8|=_G_A();_S_F(_z80_szp[d8]);_S_A(d8);}break;
            case 0xb1:/*OR C*/d8=_G_C();{d8|=_G_A();_S_F(_z80_szp[d8]);_S_A(d8);}break;
            case 0xb2:/*OR D*/d8=_G_D();{d8|=_G_A();_S_F(_z80_szp[d8]);_S_A(d8);}break;
            case 0xb3:/*OR E*/d8=_G_E();{d8|=_G_A();_S_F(_z80_szp[d8]);_S_A(d8);}break;
            case 0xb4:/*OR H*/d8=_G_H();{d8|=_G_A();_S_F(_z80_szp[d8]);_S_A(d8);}break;
            case 0xb5:/*OR L*/d8=_G_L();{d8|=_G_A();_S_F(_z80_szp[d8]);_S_A(d8);}break;
            case 0xb6:/*OR,(HL/IX+d/IY+d)*/_ADDR(addr,5);_MR(addr,d8);{d8|=_G_A();_S_F(_z80_szp[d8]);_S_A(d8);}break;
            case 0xb7:/*OR A*/d8=_G_A();{d8|=_G_A();_S_F(_z80_szp[d8]);_S_A(d8);}break;
            case 0xb8:/*CP B*/d8=_G_B();{uint8_t acc=_G_A();int32_t res=(uint32_t)((int)acc-(int)d8);_S_F(_CP_FLAGS(acc,d8,res));}break;
            case 0xb9:/*CP C*/d8=_G_C();{uint8_t acc=_G_A();int32_t res=(uint32_t)((int)acc-(int)d8);_S_F(_CP_FLAGS(acc,d8,res));}break;
            case 0xba:/*CP D*/d8=_G_D();{uint8_t acc=_G_A();int32_t res=(uint32_t)((int)acc-(int)d8);_S_F(_CP_FLAGS(acc,d8,res));}break;
            case 0xbb:/*CP E*/d8=_G_E();{uint8_t acc=_G_A();int32_t res=(uint32_t)((int)acc-(int)d8);_S_F(_CP_FLAGS(acc,d8,res));}break;
            case 0xbc:/*CP H*/d8=_G_H();{uint8_t acc=_G_A();int32_t res=(uint32_t)((int)acc-(int)d8);_S_F(_CP_FLAGS(acc,d8,res));}break;
            case 0xbd:/*CP L*/d8=_G_L();{uint8_t acc=_G_A();int32_t res=(uint32_t)((int)acc-(int)d8);_S_F(_CP_FLAGS(acc,d8,res));}break;
            case 0xbe:/*CP,(HL/IX+d/IY+d)*/_ADDR(addr,5);_MR(addr,d8);{uint8_t acc=_G_A();int32_t res=(uint32_t)((int)acc-(int)d8);_S_F(_CP_FLAGS(acc,d8,res));}break;
            case 0xbf:/*CP A*/d8=_G_A();{uint8_t acc=_G_A();int32_t res=(uint32_t)((int)acc-(int)d8);_S_F(_CP_FLAGS(acc,d8,res));}break;
            case 0xc0:/*RET NZ*/_T(1);if (!(_G_F()&Z80_ZF)){uint8_t w,z;d16=_G_SP();_MR(d16++,z);_MR(d16++,w);_S_SP(d16);pc=(w<<8)|z;_S_WZ(pc);}break;
            case 0xc1:/*POP BC*/addr=_G_SP();_MR(addr++,d8);d16=d8;_MR(addr++,d8);d16|=d8<<8;_S_BC(d16);_S_SP(addr);break;
            case 0xc2:/*JP NZ,nn*/_IMM16(addr);if(!(_G_F()&Z80_ZF)){pc=addr;}break;
            case 0xc3:/*JP nn*/_IMM16(pc);break;
            case 0xc4:/*CALL NZ,nn*/_IMM16(addr);if(!(_G_F()&Z80_ZF)){_T(1);uint16_t sp=_G_SP();_MW(--sp,pc>>8);_MW(--sp,pc);_S_SP(sp);pc=addr;}break;
            case 0xc5:/*PUSH BC*/_T(1);addr=_G_SP();d16=_G_BC();_MW(--addr,d16>>8);_MW(--addr,d16);_S_SP(addr);break;
            case 0xc6:/*ADD n*/_IMM8(d8);{uint8_t acc=_G_A();uint32_t res=acc+d8;_S_F(_ADD_FLAGS(acc,d8,res));_S_A(res);}break;
            case 0xc7:/*RST 0x0*/_T(1);d16= _G_SP();_MW(--d16, pc>>8);_MW(--d16, pc);_S_SP(d16);pc=0x0;_S_WZ(pc);break;
            case 0xc8:/*RET Z*/_T(1);if ((_G_F()&Z80_ZF)){uint8_t w,z;d16=_G_SP();_MR(d16++,z);_MR(d16++,w);_S_SP(d16);pc=(w<<8)|z;_S_WZ(pc);}break;
            case 0xc9:/*RET*/d16=_G_SP();_MR(d16++,d8);pc=d8;_MR(d16++,d8);pc|=d8<<8;_S_SP(d16);_S_WZ(pc);break;
            case 0xca:/*JP Z,nn*/_IMM16(addr);if((_G_F()&Z80_ZF)){pc=addr;}break;
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
                const int rz = (7-z)<<3;
                /* load the operand (for indexed ops, always from memory!) */
                if ((z == 6) || _IDX()) {
                  _T(1);
                  addr = _G_HL();
                  if (_IDX()) {
                    _T(1);
                    addr += d;
                    _S_WZ(addr);
                  }
                  _MR(addr,d8);
                }
                else {
                  /* simple non-indexed, non-(HL): load register value */
                  d8 = _G8(ws,rz);
                }
                uint8_t f = _G_F();
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
                      f |= (_G_WZ()>>8) & (Z80_YF|Z80_XF);
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
                      _S8(r0,rz,r);
                    }
                    else {
                      _S8(ws,rz,r);
                    }
                  }
                }
                _S_F(f);
            }
            break;
            case 0xcc:/*CALL Z,nn*/_IMM16(addr);if((_G_F()&Z80_ZF)){_T(1);uint16_t sp=_G_SP();_MW(--sp,pc>>8);_MW(--sp,pc);_S_SP(sp);pc=addr;}break;
            case 0xcd:/*CALL nn*/_IMM16(addr);_T(1);d16=_G_SP();_MW(--d16,pc>>8);_MW(--d16,pc);_S_SP(d16);pc=addr;break;
            case 0xce:/*ADC n*/_IMM8(d8);{uint8_t acc=_G_A();uint32_t res=acc+d8+(_G_F()&Z80_CF);_S_F(_ADD_FLAGS(acc,d8,res));_S_A(res);}break;
            case 0xcf:/*RST 0x8*/_T(1);d16= _G_SP();_MW(--d16, pc>>8);_MW(--d16, pc);_S_SP(d16);pc=0x8;_S_WZ(pc);break;
            case 0xd0:/*RET NC*/_T(1);if (!(_G_F()&Z80_CF)){uint8_t w,z;d16=_G_SP();_MR(d16++,z);_MR(d16++,w);_S_SP(d16);pc=(w<<8)|z;_S_WZ(pc);}break;
            case 0xd1:/*POP DE*/addr=_G_SP();_MR(addr++,d8);d16=d8;_MR(addr++,d8);d16|=d8<<8;_S_DE(d16);_S_SP(addr);break;
            case 0xd2:/*JP NC,nn*/_IMM16(addr);if(!(_G_F()&Z80_CF)){pc=addr;}break;
            case 0xd3:/*OUT (n),A*/{_IMM8(d8);uint8_t a=_G_A();addr=(a<<8)|d8;_OUT(addr,a);_S_WZ((addr&0xFF00)|((addr+1)&0x00FF));}break;
            case 0xd4:/*CALL NC,nn*/_IMM16(addr);if(!(_G_F()&Z80_CF)){_T(1);uint16_t sp=_G_SP();_MW(--sp,pc>>8);_MW(--sp,pc);_S_SP(sp);pc=addr;}break;
            case 0xd5:/*PUSH DE*/_T(1);addr=_G_SP();d16=_G_DE();_MW(--addr,d16>>8);_MW(--addr,d16);_S_SP(addr);break;
            case 0xd6:/*SUB n*/_IMM8(d8);{uint8_t acc=_G_A();uint32_t res=(uint32_t)((int)acc-(int)d8);_S_F(_SUB_FLAGS(acc,d8,res));_S_A(res);}break;
            case 0xd7:/*RST 0x10*/_T(1);d16= _G_SP();_MW(--d16, pc>>8);_MW(--d16, pc);_S_SP(d16);pc=0x10;_S_WZ(pc);break;
            case 0xd8:/*RET C*/_T(1);if ((_G_F()&Z80_CF)){uint8_t w,z;d16=_G_SP();_MR(d16++,z);_MR(d16++,w);_S_SP(d16);pc=(w<<8)|z;_S_WZ(pc);}break;
            case 0xd9:/*EXX*/{r0=_z80_flush_r0(ws,r0,r2);const uint64_t rx=r3;r3=(r3&0xffff)|(r0&0xffffffffffff0000);r0=(r0&0xffff)|(rx&0xffffffffffff0000);ws=_z80_map_regs(r0, r1, r2);}break;
            case 0xda:/*JP C,nn*/_IMM16(addr);if((_G_F()&Z80_CF)){pc=addr;}break;
            case 0xdb:/*IN A,(n)*/{_IMM8(d8);uint8_t a=_G_A();addr=(a<<8)|d8;_IN(addr++,a);_S_A(a);_S_WZ(addr);}break;
            case 0xdc:/*CALL C,nn*/_IMM16(addr);if((_G_F()&Z80_CF)){_T(1);uint16_t sp=_G_SP();_MW(--sp,pc>>8);_MW(--sp,pc);_S_SP(sp);pc=addr;}break;
            case 0xdd:/*DD prefix*/map_bits|=_BIT_USE_IX;continue;break;
            case 0xde:/*SBC n*/_IMM8(d8);{uint8_t acc=_G_A();uint32_t res=(uint32_t)((int)acc-(int)d8-(_G_F()&Z80_CF));_S_F(_SUB_FLAGS(acc,d8,res));_S_A(res);}break;
            case 0xdf:/*RST 0x18*/_T(1);d16= _G_SP();_MW(--d16, pc>>8);_MW(--d16, pc);_S_SP(d16);pc=0x18;_S_WZ(pc);break;
            case 0xe0:/*RET PO*/_T(1);if (!(_G_F()&Z80_PF)){uint8_t w,z;d16=_G_SP();_MR(d16++,z);_MR(d16++,w);_S_SP(d16);pc=(w<<8)|z;_S_WZ(pc);}break;
            case 0xe1:/*POP HL*/addr=_G_SP();_MR(addr++,d8);d16=d8;_MR(addr++,d8);d16|=d8<<8;_S_HL(d16);_S_SP(addr);break;
            case 0xe2:/*JP PO,nn*/_IMM16(addr);if(!(_G_F()&Z80_PF)){pc=addr;}break;
            case 0xe3:/*EX (SP),HL*/{_T(3);addr=_G_SP();d16=_G_HL();uint8_t l,h;_MR(addr,l);_MR(addr+1,h);_MW(addr,d16);_MW(addr+1,d16>>8);d16=(h<<8)|l;_S_HL(d16);_S_WZ(d16);}break;
            case 0xe4:/*CALL PO,nn*/_IMM16(addr);if(!(_G_F()&Z80_PF)){_T(1);uint16_t sp=_G_SP();_MW(--sp,pc>>8);_MW(--sp,pc);_S_SP(sp);pc=addr;}break;
            case 0xe5:/*PUSH HL*/_T(1);addr=_G_SP();d16=_G_HL();_MW(--addr,d16>>8);_MW(--addr,d16);_S_SP(addr);break;
            case 0xe6:/*AND n*/_IMM8(d8);{d8&=_G_A();_S_F(_z80_szp[d8]|Z80_HF);_S_A(d8);}break;
            case 0xe7:/*RST 0x20*/_T(1);d16= _G_SP();_MW(--d16, pc>>8);_MW(--d16, pc);_S_SP(d16);pc=0x20;_S_WZ(pc);break;
            case 0xe8:/*RET PE*/_T(1);if ((_G_F()&Z80_PF)){uint8_t w,z;d16=_G_SP();_MR(d16++,z);_MR(d16++,w);_S_SP(d16);pc=(w<<8)|z;_S_WZ(pc);}break;
            case 0xe9:/*JP HL*/pc=_G_HL();break;
            case 0xea:/*JP PE,nn*/_IMM16(addr);if((_G_F()&Z80_PF)){pc=addr;}break;
            case 0xeb:/*EX DE,HL*/{r0=_z80_flush_r0(ws,r0,r2);uint16_t de=_G16(r0,_DE);uint16_t hl=_G16(r0,_HL);_S16(r0,_DE,hl);_S16(r0,_HL,de);ws=_z80_map_regs(r0,r1,r2);}break;
            case 0xec:/*CALL PE,nn*/_IMM16(addr);if((_G_F()&Z80_PF)){_T(1);uint16_t sp=_G_SP();_MW(--sp,pc>>8);_MW(--sp,pc);_S_SP(sp);pc=addr;}break;
            case 0xED: {
                _FETCH(op);
                switch(op) {
                    case 0x40:/*IN B,(C)*/{addr=_G_BC();_IN(addr++,d8);_S_WZ(addr);uint8_t f=(_G_F()&Z80_CF)|_z80_szp[d8];_S8(ws,_F,f);_S_B(d8);}break;
                    case 0x41:/*OUT (C),B*/addr=_G_BC();_OUT(addr++,_G_B());_S_WZ(addr);break;
                    case 0x42:/*SBC HL,BC*/{uint16_t acc=_G_HL();_S_WZ(acc+1);d16=_G_BC();uint32_t r=acc-d16-(_G_F()&Z80_CF);uint8_t f=Z80_NF|(((d16^acc)&(acc^r)&0x8000)>>13);_S_HL(r);f|=((acc^r^d16)>>8) & Z80_HF;f|=(r>>16)&Z80_CF;f|=(r>>8)&(Z80_SF|Z80_YF|Z80_XF);f|=(r&0xFFFF)?0:Z80_ZF;_S_F(f);_T(7);}break;
                    case 0x43:/*LD (nn),BC*/_IMM16(addr);d16=_G_BC();_MW(addr++,d16&0xFF);_MW(addr,d16>>8);_S_WZ(addr);break;
                    case 0x44:/*NEG*/d8=_G_A();_S_A(0);{uint8_t acc=_G_A();uint32_t res=(uint32_t)((int)acc-(int)d8);_S_F(_SUB_FLAGS(acc,d8,res));_S_A(res);}break;
                    case 0x45:/*RETN*/pins|=Z80_RETI;d16=_G_SP();_MR(d16++,d8);pc=d8;_MR(d16++,d8);pc|=d8<<8;_S_SP(d16);_S_WZ(pc);if (r2&_BIT_IFF2){r2|=_BIT_IFF1;}else{r2&=~_BIT_IFF1;}break;
                    case 0x46:/*IM 0*/_S_IM(0);break;
                    case 0x47:/*LD I,A*/_T(1);_S_I(_G_A());break;
                    case 0x48:/*IN C,(C)*/{addr=_G_BC();_IN(addr++,d8);_S_WZ(addr);uint8_t f=(_G_F()&Z80_CF)|_z80_szp[d8];_S8(ws,_F,f);_S_C(d8);}break;
                    case 0x49:/*OUT (C),C*/addr=_G_BC();_OUT(addr++,_G_C());_S_WZ(addr);break;
                    case 0x4a:/*ADC HL,BC*/{uint16_t acc=_G_HL();_S_WZ(acc+1);d16=_G_BC();uint32_t r=acc+d16+(_G_F()&Z80_CF);_S_HL(r);uint8_t f=((d16^acc^0x8000)&(d16^r)&0x8000)>>13;f|=((acc^r^d16)>>8)&Z80_HF;f|=(r>>16)&Z80_CF;f|=(r>>8)&(Z80_SF|Z80_YF|Z80_XF);f|=(r&0xFFFF)?0:Z80_ZF;_S_F(f);_T(7);}break;
                    case 0x4b:/*LD BC,(nn)*/_IMM16(addr);_MR(addr++,d8);d16=d8;_MR(addr,d8);d16|=d8<<8;_S_BC(d16);_S_WZ(addr);break;
                    case 0x4c:/*NEG*/d8=_G_A();_S_A(0);{uint8_t acc=_G_A();uint32_t res=(uint32_t)((int)acc-(int)d8);_S_F(_SUB_FLAGS(acc,d8,res));_S_A(res);}break;
                    case 0x4d:/*RETI*/pins|=Z80_RETI;d16=_G_SP();_MR(d16++,d8);pc=d8;_MR(d16++,d8);pc|=d8<<8;_S_SP(d16);_S_WZ(pc);if (r2&_BIT_IFF2){r2|=_BIT_IFF1;}else{r2&=~_BIT_IFF1;}break;
                    case 0x4e:/*IM 0*/_S_IM(0);break;
                    case 0x4f:/*LD R,A*/_T(1);_S_R(_G_A());break;
                    case 0x50:/*IN D,(C)*/{addr=_G_BC();_IN(addr++,d8);_S_WZ(addr);uint8_t f=(_G_F()&Z80_CF)|_z80_szp[d8];_S8(ws,_F,f);_S_D(d8);}break;
                    case 0x51:/*OUT (C),D*/addr=_G_BC();_OUT(addr++,_G_D());_S_WZ(addr);break;
                    case 0x52:/*SBC HL,DE*/{uint16_t acc=_G_HL();_S_WZ(acc+1);d16=_G_DE();uint32_t r=acc-d16-(_G_F()&Z80_CF);uint8_t f=Z80_NF|(((d16^acc)&(acc^r)&0x8000)>>13);_S_HL(r);f|=((acc^r^d16)>>8) & Z80_HF;f|=(r>>16)&Z80_CF;f|=(r>>8)&(Z80_SF|Z80_YF|Z80_XF);f|=(r&0xFFFF)?0:Z80_ZF;_S_F(f);_T(7);}break;
                    case 0x53:/*LD (nn),DE*/_IMM16(addr);d16=_G_DE();_MW(addr++,d16&0xFF);_MW(addr,d16>>8);_S_WZ(addr);break;
                    case 0x54:/*NEG*/d8=_G_A();_S_A(0);{uint8_t acc=_G_A();uint32_t res=(uint32_t)((int)acc-(int)d8);_S_F(_SUB_FLAGS(acc,d8,res));_S_A(res);}break;
                    case 0x55:/*RETN*/pins|=Z80_RETI;d16=_G_SP();_MR(d16++,d8);pc=d8;_MR(d16++,d8);pc|=d8<<8;_S_SP(d16);_S_WZ(pc);if (r2&_BIT_IFF2){r2|=_BIT_IFF1;}else{r2&=~_BIT_IFF1;}break;
                    case 0x56:/*IM 1*/_S_IM(1);break;
                    case 0x57:/*LD A,I*/_T(1);d8=_G_I();_S_A(d8);_S_F(_SZIFF2_FLAGS(d8));break;
                    case 0x58:/*IN E,(C)*/{addr=_G_BC();_IN(addr++,d8);_S_WZ(addr);uint8_t f=(_G_F()&Z80_CF)|_z80_szp[d8];_S8(ws,_F,f);_S_E(d8);}break;
                    case 0x59:/*OUT (C),E*/addr=_G_BC();_OUT(addr++,_G_E());_S_WZ(addr);break;
                    case 0x5a:/*ADC HL,DE*/{uint16_t acc=_G_HL();_S_WZ(acc+1);d16=_G_DE();uint32_t r=acc+d16+(_G_F()&Z80_CF);_S_HL(r);uint8_t f=((d16^acc^0x8000)&(d16^r)&0x8000)>>13;f|=((acc^r^d16)>>8)&Z80_HF;f|=(r>>16)&Z80_CF;f|=(r>>8)&(Z80_SF|Z80_YF|Z80_XF);f|=(r&0xFFFF)?0:Z80_ZF;_S_F(f);_T(7);}break;
                    case 0x5b:/*LD DE,(nn)*/_IMM16(addr);_MR(addr++,d8);d16=d8;_MR(addr,d8);d16|=d8<<8;_S_DE(d16);_S_WZ(addr);break;
                    case 0x5c:/*NEG*/d8=_G_A();_S_A(0);{uint8_t acc=_G_A();uint32_t res=(uint32_t)((int)acc-(int)d8);_S_F(_SUB_FLAGS(acc,d8,res));_S_A(res);}break;
                    case 0x5d:/*RETN*/pins|=Z80_RETI;d16=_G_SP();_MR(d16++,d8);pc=d8;_MR(d16++,d8);pc|=d8<<8;_S_SP(d16);_S_WZ(pc);if (r2&_BIT_IFF2){r2|=_BIT_IFF1;}else{r2&=~_BIT_IFF1;}break;
                    case 0x5e:/*IM 2*/_S_IM(2);break;
                    case 0x5f:/*LD A,R*/_T(1);d8=_G_R();_S_A(d8);_S_F(_SZIFF2_FLAGS(d8));break;
                    case 0x60:/*IN H,(C)*/{addr=_G_BC();_IN(addr++,d8);_S_WZ(addr);uint8_t f=(_G_F()&Z80_CF)|_z80_szp[d8];_S8(ws,_F,f);_S_H(d8);}break;
                    case 0x61:/*OUT (C),H*/addr=_G_BC();_OUT(addr++,_G_H());_S_WZ(addr);break;
                    case 0x62:/*SBC HL,HL*/{uint16_t acc=_G_HL();_S_WZ(acc+1);d16=_G_HL();uint32_t r=acc-d16-(_G_F()&Z80_CF);uint8_t f=Z80_NF|(((d16^acc)&(acc^r)&0x8000)>>13);_S_HL(r);f|=((acc^r^d16)>>8) & Z80_HF;f|=(r>>16)&Z80_CF;f|=(r>>8)&(Z80_SF|Z80_YF|Z80_XF);f|=(r&0xFFFF)?0:Z80_ZF;_S_F(f);_T(7);}break;
                    case 0x63:/*LD (nn),HL*/_IMM16(addr);d16=_G_HL();_MW(addr++,d16&0xFF);_MW(addr,d16>>8);_S_WZ(addr);break;
                    case 0x64:/*NEG*/d8=_G_A();_S_A(0);{uint8_t acc=_G_A();uint32_t res=(uint32_t)((int)acc-(int)d8);_S_F(_SUB_FLAGS(acc,d8,res));_S_A(res);}break;
                    case 0x65:/*RETN*/pins|=Z80_RETI;d16=_G_SP();_MR(d16++,d8);pc=d8;_MR(d16++,d8);pc|=d8<<8;_S_SP(d16);_S_WZ(pc);if (r2&_BIT_IFF2){r2|=_BIT_IFF1;}else{r2&=~_BIT_IFF1;}break;
                    case 0x66:/*IM 0*/_S_IM(0);break;
                    case 0x67:/*RRD*/{addr=_G_HL();uint8_t a=_G_A();_MR(addr,d8);uint8_t l=a&0x0F;a=(a&0xF0)|(d8&0x0F);_S_A(a);d8=(d8>>4)|(l<<4);_MW(addr++,d8);_S_WZ(addr);_S_F((_G_F()&Z80_CF)|_z80_szp[a]);_T(4);}break;
                    case 0x68:/*IN L,(C)*/{addr=_G_BC();_IN(addr++,d8);_S_WZ(addr);uint8_t f=(_G_F()&Z80_CF)|_z80_szp[d8];_S8(ws,_F,f);_S_L(d8);}break;
                    case 0x69:/*OUT (C),L*/addr=_G_BC();_OUT(addr++,_G_L());_S_WZ(addr);break;
                    case 0x6a:/*ADC HL,HL*/{uint16_t acc=_G_HL();_S_WZ(acc+1);d16=_G_HL();uint32_t r=acc+d16+(_G_F()&Z80_CF);_S_HL(r);uint8_t f=((d16^acc^0x8000)&(d16^r)&0x8000)>>13;f|=((acc^r^d16)>>8)&Z80_HF;f|=(r>>16)&Z80_CF;f|=(r>>8)&(Z80_SF|Z80_YF|Z80_XF);f|=(r&0xFFFF)?0:Z80_ZF;_S_F(f);_T(7);}break;
                    case 0x6b:/*LD HL,(nn)*/_IMM16(addr);_MR(addr++,d8);d16=d8;_MR(addr,d8);d16|=d8<<8;_S_HL(d16);_S_WZ(addr);break;
                    case 0x6c:/*NEG*/d8=_G_A();_S_A(0);{uint8_t acc=_G_A();uint32_t res=(uint32_t)((int)acc-(int)d8);_S_F(_SUB_FLAGS(acc,d8,res));_S_A(res);}break;
                    case 0x6d:/*RETN*/pins|=Z80_RETI;d16=_G_SP();_MR(d16++,d8);pc=d8;_MR(d16++,d8);pc|=d8<<8;_S_SP(d16);_S_WZ(pc);if (r2&_BIT_IFF2){r2|=_BIT_IFF1;}else{r2&=~_BIT_IFF1;}break;
                    case 0x6e:/*IM 0*/_S_IM(0);break;
                    case 0x6f:/*RLD*/{addr=_G_HL();uint8_t a=_G_A();_MR(addr,d8);uint8_t l=a&0x0F;a=(a&0xF0)|(d8>>4);_S_A(a);d8=(d8<<4)|l;_MW(addr++,d8);_S_WZ(addr);_S_F((_G_F()&Z80_CF)|_z80_szp[a]);_T(4);}break;
                    case 0x70:/*IN HL,(C)*/{addr=_G_BC();_IN(addr++,d8);_S_WZ(addr);uint8_t f=(_G_F()&Z80_CF)|_z80_szp[d8];_S8(ws,_F,f);}break;
                    case 0x71:/*OUT (C),HL*/addr=_G_BC();_OUT(addr++,0);_S_WZ(addr);break;
                    case 0x72:/*SBC HL,SP*/{uint16_t acc=_G_HL();_S_WZ(acc+1);d16=_G_SP();uint32_t r=acc-d16-(_G_F()&Z80_CF);uint8_t f=Z80_NF|(((d16^acc)&(acc^r)&0x8000)>>13);_S_HL(r);f|=((acc^r^d16)>>8) & Z80_HF;f|=(r>>16)&Z80_CF;f|=(r>>8)&(Z80_SF|Z80_YF|Z80_XF);f|=(r&0xFFFF)?0:Z80_ZF;_S_F(f);_T(7);}break;
                    case 0x73:/*LD (nn),SP*/_IMM16(addr);d16=_G_SP();_MW(addr++,d16&0xFF);_MW(addr,d16>>8);_S_WZ(addr);break;
                    case 0x74:/*NEG*/d8=_G_A();_S_A(0);{uint8_t acc=_G_A();uint32_t res=(uint32_t)((int)acc-(int)d8);_S_F(_SUB_FLAGS(acc,d8,res));_S_A(res);}break;
                    case 0x75:/*RETN*/pins|=Z80_RETI;d16=_G_SP();_MR(d16++,d8);pc=d8;_MR(d16++,d8);pc|=d8<<8;_S_SP(d16);_S_WZ(pc);if (r2&_BIT_IFF2){r2|=_BIT_IFF1;}else{r2&=~_BIT_IFF1;}break;
                    case 0x76:/*IM 1*/_S_IM(1);break;
                    case 0x77:/*NOP (ED)*/ break;
                    case 0x78:/*IN A,(C)*/{addr=_G_BC();_IN(addr++,d8);_S_WZ(addr);uint8_t f=(_G_F()&Z80_CF)|_z80_szp[d8];_S8(ws,_F,f);_S_A(d8);}break;
                    case 0x79:/*OUT (C),A*/addr=_G_BC();_OUT(addr++,_G_A());_S_WZ(addr);break;
                    case 0x7a:/*ADC HL,SP*/{uint16_t acc=_G_HL();_S_WZ(acc+1);d16=_G_SP();uint32_t r=acc+d16+(_G_F()&Z80_CF);_S_HL(r);uint8_t f=((d16^acc^0x8000)&(d16^r)&0x8000)>>13;f|=((acc^r^d16)>>8)&Z80_HF;f|=(r>>16)&Z80_CF;f|=(r>>8)&(Z80_SF|Z80_YF|Z80_XF);f|=(r&0xFFFF)?0:Z80_ZF;_S_F(f);_T(7);}break;
                    case 0x7b:/*LD SP,(nn)*/_IMM16(addr);_MR(addr++,d8);d16=d8;_MR(addr,d8);d16|=d8<<8;_S_SP(d16);_S_WZ(addr);break;
                    case 0x7c:/*NEG*/d8=_G_A();_S_A(0);{uint8_t acc=_G_A();uint32_t res=(uint32_t)((int)acc-(int)d8);_S_F(_SUB_FLAGS(acc,d8,res));_S_A(res);}break;
                    case 0x7d:/*RETN*/pins|=Z80_RETI;d16=_G_SP();_MR(d16++,d8);pc=d8;_MR(d16++,d8);pc|=d8<<8;_S_SP(d16);_S_WZ(pc);if (r2&_BIT_IFF2){r2|=_BIT_IFF1;}else{r2&=~_BIT_IFF1;}break;
                    case 0x7e:/*IM 2*/_S_IM(2);break;
                    case 0x7f:/*NOP (ED)*/ break;
                    case 0xa0:/*LDI*/{uint16_t hl=_G_HL();uint16_t de=_G_DE();_MR(hl,d8);_MW(de,d8);hl++;de++;_S_HL(hl);_S_DE(de);_T(2);d8+=_G_A();uint8_t f=_G_F()&(Z80_SF|Z80_ZF|Z80_CF);if(d8&0x02){f|=Z80_YF;}if(d8&0x08){f|=Z80_XF;}uint16_t bc=_G_BC();bc--;_S_BC(bc);if(bc){f|=Z80_VF;}_S_F(f);}break;
                    case 0xa1:/*CPI*/{uint16_t hl = _G_HL();_MR(hl,d8);uint16_t wz = _G_WZ();hl++;wz++;_S_WZ(wz);_S_HL(hl);_T(5);int r=((int)_G_A())-d8;uint8_t f=(_G_F()&Z80_CF)|Z80_NF|_SZ(r);if((r&0x0F)>(_G_A()&0x0F)){f|=Z80_HF;r--;}if(r&0x02){f|=Z80_YF;}if(r&0x08){f|=Z80_XF;}uint16_t bc=_G_BC();bc--;_S_BC(bc);if(bc){f|=Z80_VF;}_S8(ws,_F,f);}break;
                    case 0xa2:/*INI*/{_T(1);addr=_G_BC();uint16_t hl=_G_HL();_IN(addr,d8);_MW(hl,d8);uint8_t b=_G_B();uint8_t c=_G_C();b--;addr++;hl++;c++;_S_B(b);_S_HL(hl);_S_WZ(addr);uint8_t f=(b?(b&Z80_SF):Z80_ZF)|(b&(Z80_XF|Z80_YF));if(d8&Z80_SF){f|=Z80_NF;}uint32_t t=(uint32_t)(c&0xFF)+d8;if(t&0x100){f|=Z80_HF|Z80_CF;}f|=_z80_szp[((uint8_t)(t&0x07))^b]&Z80_PF;_S_F(f);}break;
                    case 0xa3:/*OUTI*/{_T(1);uint16_t hl=_G_HL();_MR(hl,d8);uint8_t b=_G_B();b--;_S_B(b);addr=_G_BC();_OUT(addr,d8);addr++; hl++;_S_HL(hl);_S_WZ(addr);uint8_t f=(b?(b&Z80_SF):Z80_ZF)|(b&(Z80_XF|Z80_YF));if(d8&Z80_SF){f|=Z80_NF;}uint32_t t=(uint32_t)_G_L()+(uint32_t)d8;if (t&0x0100){f|=Z80_HF|Z80_CF;}f|=_z80_szp[((uint8_t)(t&0x07))^b]&Z80_PF;_S_F(f);}break;
                    case 0xa8:/*LDD*/{uint16_t hl=_G_HL();uint16_t de=_G_DE();_MR(hl,d8);_MW(de,d8);hl--;de--;_S_HL(hl);_S_DE(de);_T(2);d8+=_G_A();uint8_t f=_G_F()&(Z80_SF|Z80_ZF|Z80_CF);if(d8&0x02){f|=Z80_YF;}if(d8&0x08){f|=Z80_XF;}uint16_t bc=_G_BC();bc--;_S_BC(bc);if(bc){f|=Z80_VF;}_S_F(f);}break;
                    case 0xa9:/*CPD*/{uint16_t hl = _G_HL();_MR(hl,d8);uint16_t wz = _G_WZ();hl--;wz--;_S_WZ(wz);_S_HL(hl);_T(5);int r=((int)_G_A())-d8;uint8_t f=(_G_F()&Z80_CF)|Z80_NF|_SZ(r);if((r&0x0F)>(_G_A()&0x0F)){f|=Z80_HF;r--;}if(r&0x02){f|=Z80_YF;}if(r&0x08){f|=Z80_XF;}uint16_t bc=_G_BC();bc--;_S_BC(bc);if(bc){f|=Z80_VF;}_S8(ws,_F,f);}break;
                    case 0xaa:/*IND*/{_T(1);addr=_G_BC();uint16_t hl=_G_HL();_IN(addr,d8);_MW(hl,d8);uint8_t b=_G_B();uint8_t c=_G_C();b--;addr--;hl--;c--;_S_B(b);_S_HL(hl);_S_WZ(addr);uint8_t f=(b?(b&Z80_SF):Z80_ZF)|(b&(Z80_XF|Z80_YF));if(d8&Z80_SF){f|=Z80_NF;}uint32_t t=(uint32_t)(c&0xFF)+d8;if(t&0x100){f|=Z80_HF|Z80_CF;}f|=_z80_szp[((uint8_t)(t&0x07))^b]&Z80_PF;_S_F(f);}break;
                    case 0xab:/*OUTD*/{_T(1);uint16_t hl=_G_HL();_MR(hl,d8);uint8_t b=_G_B();b--;_S_B(b);addr=_G_BC();_OUT(addr,d8);addr--;hl--;_S_HL(hl);_S_WZ(addr);uint8_t f=(b?(b&Z80_SF):Z80_ZF)|(b&(Z80_XF|Z80_YF));if(d8&Z80_SF){f|=Z80_NF;}uint32_t t=(uint32_t)_G_L()+(uint32_t)d8;if (t&0x0100){f|=Z80_HF|Z80_CF;}f|=_z80_szp[((uint8_t)(t&0x07))^b]&Z80_PF;_S_F(f);}break;
                    case 0xb0:/*LDIR*/{uint16_t hl=_G_HL();uint16_t de=_G_DE();_MR(hl,d8);_MW(de,d8);hl++;de++;_S_HL(hl);_S_DE(de);_T(2);d8+=_G_A();uint8_t f=_G_F()&(Z80_SF|Z80_ZF|Z80_CF);if(d8&0x02){f|=Z80_YF;}if(d8&0x08){f|=Z80_XF;}uint16_t bc=_G_BC();bc--;_S_BC(bc);if(bc){f|=Z80_VF;}_S_F(f);if(bc){pc-=2;_S_WZ(pc+1);_T(5);}}break;
                    case 0xb1:/*CPIR*/{uint16_t hl = _G_HL();_MR(hl,d8);uint16_t wz = _G_WZ();hl++;wz++;_S_WZ(wz);_S_HL(hl);_T(5);int r=((int)_G_A())-d8;uint8_t f=(_G_F()&Z80_CF)|Z80_NF|_SZ(r);if((r&0x0F)>(_G_A()&0x0F)){f|=Z80_HF;r--;}if(r&0x02){f|=Z80_YF;}if(r&0x08){f|=Z80_XF;}uint16_t bc=_G_BC();bc--;_S_BC(bc);if(bc){f|=Z80_VF;}_S8(ws,_F,f);if(bc&&!(f&Z80_ZF)){pc-=2;_S_WZ(pc+1);_T(5);}}break;
                    case 0xb2:/*INIR*/{_T(1);addr=_G_BC();uint16_t hl=_G_HL();_IN(addr,d8);_MW(hl,d8);uint8_t b=_G_B();uint8_t c=_G_C();b--;addr++;hl++;c++;_S_B(b);_S_HL(hl);_S_WZ(addr);uint8_t f=(b?(b&Z80_SF):Z80_ZF)|(b&(Z80_XF|Z80_YF));if(d8&Z80_SF){f|=Z80_NF;}uint32_t t=(uint32_t)(c&0xFF)+d8;if(t&0x100){f|=Z80_HF|Z80_CF;}f|=_z80_szp[((uint8_t)(t&0x07))^b]&Z80_PF;_S_F(f);if(b){pc-=2;_T(5);}}break;
                    case 0xb3:/*OTIR*/{_T(1);uint16_t hl=_G_HL();_MR(hl,d8);uint8_t b=_G_B();b--;_S_B(b);addr=_G_BC();_OUT(addr,d8);addr++; hl++;_S_HL(hl);_S_WZ(addr);uint8_t f=(b?(b&Z80_SF):Z80_ZF)|(b&(Z80_XF|Z80_YF));if(d8&Z80_SF){f|=Z80_NF;}uint32_t t=(uint32_t)_G_L()+(uint32_t)d8;if (t&0x0100){f|=Z80_HF|Z80_CF;}f|=_z80_szp[((uint8_t)(t&0x07))^b]&Z80_PF;_S_F(f);if(b){pc-=2;_T(5);}}break;
                    case 0xb8:/*LDDR*/{uint16_t hl=_G_HL();uint16_t de=_G_DE();_MR(hl,d8);_MW(de,d8);hl--;de--;_S_HL(hl);_S_DE(de);_T(2);d8+=_G_A();uint8_t f=_G_F()&(Z80_SF|Z80_ZF|Z80_CF);if(d8&0x02){f|=Z80_YF;}if(d8&0x08){f|=Z80_XF;}uint16_t bc=_G_BC();bc--;_S_BC(bc);if(bc){f|=Z80_VF;}_S_F(f);if(bc){pc-=2;_S_WZ(pc+1);_T(5);}}break;
                    case 0xb9:/*CPDR*/{uint16_t hl = _G_HL();_MR(hl,d8);uint16_t wz = _G_WZ();hl--;wz--;_S_WZ(wz);_S_HL(hl);_T(5);int r=((int)_G_A())-d8;uint8_t f=(_G_F()&Z80_CF)|Z80_NF|_SZ(r);if((r&0x0F)>(_G_A()&0x0F)){f|=Z80_HF;r--;}if(r&0x02){f|=Z80_YF;}if(r&0x08){f|=Z80_XF;}uint16_t bc=_G_BC();bc--;_S_BC(bc);if(bc){f|=Z80_VF;}_S8(ws,_F,f);if(bc&&!(f&Z80_ZF)){pc-=2;_S_WZ(pc+1);_T(5);}}break;
                    case 0xba:/*INDR*/{_T(1);addr=_G_BC();uint16_t hl=_G_HL();_IN(addr,d8);_MW(hl,d8);uint8_t b=_G_B();uint8_t c=_G_C();b--;addr--;hl--;c--;_S_B(b);_S_HL(hl);_S_WZ(addr);uint8_t f=(b?(b&Z80_SF):Z80_ZF)|(b&(Z80_XF|Z80_YF));if(d8&Z80_SF){f|=Z80_NF;}uint32_t t=(uint32_t)(c&0xFF)+d8;if(t&0x100){f|=Z80_HF|Z80_CF;}f|=_z80_szp[((uint8_t)(t&0x07))^b]&Z80_PF;_S_F(f);if(b){pc-=2;_T(5);}}break;
                    case 0xbb:/*OTDR*/{_T(1);uint16_t hl=_G_HL();_MR(hl,d8);uint8_t b=_G_B();b--;_S_B(b);addr=_G_BC();_OUT(addr,d8);addr--;hl--;_S_HL(hl);_S_WZ(addr);uint8_t f=(b?(b&Z80_SF):Z80_ZF)|(b&(Z80_XF|Z80_YF));if(d8&Z80_SF){f|=Z80_NF;}uint32_t t=(uint32_t)_G_L()+(uint32_t)d8;if (t&0x0100){f|=Z80_HF|Z80_CF;}f|=_z80_szp[((uint8_t)(t&0x07))^b]&Z80_PF;_S_F(f);if(b){pc-=2;_T(5);}}break;
                    default: break;
                }
            }
            break;
            case 0xee:/*XOR n*/_IMM8(d8);{d8^=_G_A();_S_F(_z80_szp[d8]);_S_A(d8);}break;
            case 0xef:/*RST 0x28*/_T(1);d16= _G_SP();_MW(--d16, pc>>8);_MW(--d16, pc);_S_SP(d16);pc=0x28;_S_WZ(pc);break;
            case 0xf0:/*RET P*/_T(1);if (!(_G_F()&Z80_SF)){uint8_t w,z;d16=_G_SP();_MR(d16++,z);_MR(d16++,w);_S_SP(d16);pc=(w<<8)|z;_S_WZ(pc);}break;
            case 0xf1:/*POP FA*/addr=_G_SP();_MR(addr++,d8);d16=d8<<8;_MR(addr++,d8);d16|=d8;_S_FA(d16);_S_SP(addr);break;
            case 0xf2:/*JP P,nn*/_IMM16(addr);if(!(_G_F()&Z80_SF)){pc=addr;}break;
            case 0xf3:/*DI*/r2&=~(_BIT_IFF1|_BIT_IFF2);break;
            case 0xf4:/*CALL P,nn*/_IMM16(addr);if(!(_G_F()&Z80_SF)){_T(1);uint16_t sp=_G_SP();_MW(--sp,pc>>8);_MW(--sp,pc);_S_SP(sp);pc=addr;}break;
            case 0xf5:/*PUSH FA*/_T(1);addr=_G_SP();d16=_G_FA();_MW(--addr,d16);_MW(--addr,d16>>8);_S_SP(addr);break;
            case 0xf6:/*OR n*/_IMM8(d8);{d8|=_G_A();_S_F(_z80_szp[d8]);_S_A(d8);}break;
            case 0xf7:/*RST 0x30*/_T(1);d16= _G_SP();_MW(--d16, pc>>8);_MW(--d16, pc);_S_SP(d16);pc=0x30;_S_WZ(pc);break;
            case 0xf8:/*RET M*/_T(1);if ((_G_F()&Z80_SF)){uint8_t w,z;d16=_G_SP();_MR(d16++,z);_MR(d16++,w);_S_SP(d16);pc=(w<<8)|z;_S_WZ(pc);}break;
            case 0xf9:/*LD SP,HL*/_T(2);_S_SP(_G_HL());break;
            case 0xfa:/*JP M,nn*/_IMM16(addr);if((_G_F()&Z80_SF)){pc=addr;}break;
            case 0xfb:/*EI*/r2=(r2&~(_BIT_IFF1|_BIT_IFF2))|_BIT_EI;break;
            case 0xfc:/*CALL M,nn*/_IMM16(addr);if((_G_F()&Z80_SF)){_T(1);uint16_t sp=_G_SP();_MW(--sp,pc>>8);_MW(--sp,pc);_S_SP(sp);pc=addr;}break;
            case 0xfd:/*FD prefix*/map_bits|=_BIT_USE_IY;continue;break;
            case 0xfe:/*CP n*/_IMM8(d8);{uint8_t acc=_G_A();int32_t res=(uint32_t)((int)acc-(int)d8);_S_F(_CP_FLAGS(acc,d8,res));}break;
            case 0xff:/*RST 0x38*/_T(1);d16= _G_SP();_MW(--d16, pc>>8);_MW(--d16, pc);_S_SP(d16);pc=0x38;_S_WZ(pc);break;

        }
        /* check for interrupt request */
        bool nmi = 0 != ((pins & (pre_pins ^ pins)) & Z80_NMI);
        bool irq = (pins & Z80_INT) && (r2 & _BIT_IFF1);
        if (nmi || irq) {
            /* clear IFF flags (disables interrupt) */
            r2 &= ~_BIT_IFF1;
            if (pins & Z80_INT) {
                r2 &= ~_BIT_IFF2;
            }
            /* if in HALT state, continue */
            if (pins & Z80_HALT) {
                pins &= ~Z80_HALT;
                pc++;
            }
            /* put PC on address bus */
            _SA(pc);
            if (nmi) { /* non-maskable interrupt? */

                /* a no-op 5 tick opcode fetch */
                _TWM(5,Z80_M1|Z80_MREQ|Z80_RD);_BUMPR();
                /* put PC on stack */
                uint16_t sp = _G_SP();
                _MW(--sp,pc>>8);
                _MW(--sp,pc);
                _S_SP(sp);
                /* jump to address 0x0066 */
                pc = 0x0066;
                _S_WZ(pc);
            }
            else { /* maskable interrupt */

                /* interrupt acknowledge machine cycle, interrupt 
                   controller is expected to put interrupt vector low byte
                   on address bus
                */
                _TWM(4,Z80_M1|Z80_IORQ);
                const uint8_t int_vec = _GD();
                _BUMPR();
                _T(2);
                switch (_G_IM()) {
                    case 0: /* interrupt mode 0 not supported */
                        break;
                    case 1:
                        {
                            /* interrupt mode 1: 
                                - put PC on stack
                                - load address 0x0038 into PC
                            */
                            uint16_t sp = _G_SP();
                            _MW(--sp,pc>>8);
                            _MW(--sp,pc);
                            _S_SP(sp);
                            pc = 0x0038;
                            _S_WZ(pc);
                        }
                        break;
                    case 2:
                        {
                            /* interrupt mode 2:
                                - put PC on stack
                                - build interrupt vector address
                                - load address of interrupt service routine from
                                  interrupt vector and load into PC
                            */
                            uint16_t sp = _G_SP();
                            _MW(--sp,pc>>8);
                            _MW(--sp,pc);
                            _S_SP(sp);
                            addr = (_G_I()<<8) | (int_vec & 0xFE);
                            uint8_t z,w;
                            _MR(addr++,z);
                            _MR(addr,w);
                            pc = (w<<8)|z;
                            _S_WZ(pc);
                        }
                        break;
                }
            }
        }
        /* call track evaluation callback if set */
        if (trap) {
            int trap_id = trap(pc,ticks,pins,cpu->trap_user_data);
            if (trap_id) {
                cpu->trap_id=trap_id;
                break;
            }
        }
        /* clear state bits for next instruction */
        map_bits &= ~_BITS_USE_IXIY;
        pins &= ~Z80_INT;
        /* delay-enable interrupt flags */
        if (r2 & _BIT_EI) {
            r2 &= ~_BIT_EI;
            r2 |= (_BIT_IFF1 | _BIT_IFF2);
        }
        pre_pins = pins;
    } while (ticks < num_ticks);
    /* flush local state back to persistent CPU state before leaving */
    _S_PC(pc);
    r0 = _z80_flush_r0(ws, r0, r2);
    r1 = _z80_flush_r1(ws, r1, r2);
    r2 = (r2 & ~_BITS_USE_IXIY) | map_bits;
    cpu->bc_de_hl_fa = r0;
    cpu->wz_ix_iy_sp = r1;
    cpu->im_ir_pc_bits = r2;
    cpu->bc_de_hl_fa_ = r3;
    cpu->pins = pins;
    return ticks;
}

#undef _A
#undef _F
#undef _L
#undef _H
#undef _E
#undef _D
#undef _C
#undef _B
#undef _FA
#undef _HL
#undef _DE
#undef _BC
#undef _SP
#undef _WZ
#undef _PC
#undef _IR
#undef _R 
#undef _I 
#undef _IX
#undef _IY
#undef _IM
#undef _IFF1
#undef _IFF2
#undef _EI  
#undef _USE_IX
#undef _USE_IY
#undef _BIT_IFF1
#undef _BIT_IFF2
#undef _BIT_EI  
#undef _BIT_USE_IX
#undef _BIT_USE_IY
#undef _BITS_USE_IXIY
#undef _S8
#undef _G8
#undef _S16
#undef _G16
#undef _S1
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
#undef _S_A
#undef _S_F
#undef _S_L
#undef _S_E
#undef _S_D
#undef _S_C
#undef _S_B
#undef _S_FA
#undef _S_HL
#undef _S_DE
#undef _S_BC
#undef _S_WZ
#undef _S_IX
#undef _S_IY
#undef _S_SP
#undef _S_IM
#undef _S_I
#undef _S_R
#undef _S_I
#undef _G_A
#undef _G_F
#undef _G_L
#undef _G_E
#undef _G_D
#undef _G_C
#undef _G_B
#undef _G_FA 
#undef _G_HL 
#undef _G_DE 
#undef _G_BC 
#undef _G_WZ 
#undef _G_IX 
#undef _G_IY 
#undef _G_SP 
#undef _G_IM
#undef _G_I
#undef _G_R
#undef _G_IR 
#undef _G_PC 

#endif /* CHIPS_IMPL */
