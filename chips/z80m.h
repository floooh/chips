#pragma once
/*#
    # z80m.h

    Alternative Z80 implementation focusing on minimal code size.

    WIP!
*/
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*--- callback function typedefs ---*/
typedef uint64_t (*z80m_tick_t)(int num_ticks, uint64_t pins, void* user_data);
typedef bool (*z80m_trapfunc_t)(void* user_data);

/*--- address lines ---*/
#define Z80M_A0  (1ULL<<0)
#define Z80M_A1  (1ULL<<1)
#define Z80M_A2  (1ULL<<2)
#define Z80M_A3  (1ULL<<3)
#define Z80M_A4  (1ULL<<4)
#define Z80M_A5  (1ULL<<5)
#define Z80M_A6  (1ULL<<6)
#define Z80M_A7  (1ULL<<7)
#define Z80M_A8  (1ULL<<8)
#define Z80M_A9  (1ULL<<9)
#define Z80M_A10 (1ULL<<10)
#define Z80M_A11 (1ULL<<11)
#define Z80M_A12 (1ULL<<12)
#define Z80M_A13 (1ULL<<13)
#define Z80M_A14 (1ULL<<14)
#define Z80M_A15 (1ULL<<15)

/*--- data lines ------*/
#define Z80M_D0  (1ULL<<16)
#define Z80M_D1  (1ULL<<17)
#define Z80M_D2  (1ULL<<18)
#define Z80M_D3  (1ULL<<19)
#define Z80M_D4  (1ULL<<20)
#define Z80M_D5  (1ULL<<21)
#define Z80M_D6  (1ULL<<22)
#define Z80M_D7  (1ULL<<23)

/*--- control pins ---*/

/* system control pins */
#define  Z80M_M1    (1ULL<<24)       /* machine cycle 1 */
#define  Z80M_MREQ  (1ULL<<25)       /* memory request */
#define  Z80M_IORQ  (1ULL<<26)       /* input/output request */
#define  Z80M_RD    (1ULL<<27)       /* read */
#define  Z80M_WR    (1ULL<<28)       /* write */

/* CPU control pins */
#define  Z80M_HALT  (1ULL<<29)       /* halt state */
#define  Z80M_INT   (1ULL<<30)       /* interrupt request */
#define  Z80M_RESET (1ULL<<31)       /* reset */
#define  Z80M_BUSREQ (1ULL<<32)      /* bus request */
#define  Z80M_BUSACK (1ULL<<33)      /* bus acknowledge */

/* up to 7 wait states can be injected per machine cycle */
#define Z80M_WAIT0   (1ULL<<34)
#define Z80M_WAIT1   (1ULL<<35)
#define Z80M_WAIT2   (1ULL<<36)
#define Z80M_WAIT_SHIFT (34)
#define Z80M_WAIT_MASK (Z80M_WAIT0|Z80M_WAIT1|Z80M_WAIT2)

/* interrupt-related 'virtual pins', these don't exist on the Z80 */
#define Z80M_IEIO    (1ULL<<37)      /* unified daisy chain 'Interrupt Enable In+Out' */
#define Z80M_RETI    (1ULL<<38)      /* cpu has decoded a RETI instruction */

/* bit mask for all CPU bus pins */
#define Z80M_PIN_MASK ((1ULL<<40)-1)

/*--- status indicator flags ---*/
#define Z80M_CF (1<<0)           /* carry */
#define Z80M_NF (1<<1)           /* add/subtract */
#define Z80M_VF (1<<2)           /* parity/overflow */
#define Z80M_PF Z80M_VF
#define Z80M_XF (1<<3)           /* undocumented bit 3 */
#define Z80M_HF (1<<4)           /* half carry */
#define Z80M_YF (1<<5)           /* undocumented bit 5 */
#define Z80M_ZF (1<<6)           /* zero */
#define Z80M_SF (1<<7)           /* sign */

#define Z80M_MAX_NUM_TRAPS (4)

/* initialization attributes */
typedef struct {
    z80m_tick_t tick_cb;
    void* user_data;
} z80m_desc_t;

/* Z80 CPU state */
typedef struct {
    /* tick callback */
    z80m_tick_t tick;
    /* main register bank (BC, DE, HL, FA) */
    uint64_t bc_de_hl_fa;   /* B:63..56 C:55..48 D:47..40 E:39..32 H:31..24 L:23..16: F:15..8, A:7..0 */
    /* shadow register bank (BC', DE', HL', FA') */
    uint64_t bc_de_hl_fa_;
    /* IR,WZ,SP,PC */
    uint64_t wz_ix_iy_sp;
    /* control bits,IM,IY,IX */
    uint64_t im_ir_pc_bits;
    /* last pin state (only for debug inspection) */
    uint64_t pins;
    void* user_data;
    int trap_id;
    uint64_t trap_addr;
} z80m_t;

/* initialize a new z80 instance */
extern void z80m_init(z80m_t* cpu, z80m_desc_t* desc);
/* reset an existing z80 instance */
extern void z80m_reset(z80m_t* cpu);
/* set a trap point */
extern void z80m_set_trap(z80m_t* cpu, int trap_id, uint16_t addr);
/* clear a trap point */
extern void z80m_clear_trap(z80m_t* cpu, int trap_id);
/* return true if a trap is valid */
extern bool z80m_has_trap(z80m_t* cpu, int trap_id);
/* execute instructions for at least 'ticks', but at least one, return executed ticks */
extern uint32_t z80m_exec(z80m_t* cpu, uint32_t ticks);
/* return false if z80m_exec() returned in the middle of an extended intruction */
extern bool z80m_opdone(z80m_t* cpu);

/* register access functions */
extern void z80m_set_a(z80m_t* cpu, uint8_t v);
extern void z80m_set_f(z80m_t* cpu, uint8_t v);
extern void z80m_set_l(z80m_t* cpu, uint8_t v);
extern void z80m_set_h(z80m_t* cpu, uint8_t v);
extern void z80m_set_e(z80m_t* cpu, uint8_t v);
extern void z80m_set_d(z80m_t* cpu, uint8_t v);
extern void z80m_set_c(z80m_t* cpu, uint8_t v);
extern void z80m_set_b(z80m_t* cpu, uint8_t v);
extern void z80m_set_fa(z80m_t* cpu, uint16_t v);
extern void z80m_set_hl(z80m_t* cpu, uint16_t v);
extern void z80m_set_de(z80m_t* cpu, uint16_t v);
extern void z80m_set_bc(z80m_t* cpu, uint16_t v);
extern void z80m_set_fa_(z80m_t* cpu, uint16_t v);
extern void z80m_set_hl_(z80m_t* cpu, uint16_t v);
extern void z80m_set_de_(z80m_t* cpu, uint16_t v);
extern void z80m_set_bc_(z80m_t* cpu, uint16_t v);
extern void z80m_set_pc(z80m_t* cpu, uint16_t v);
extern void z80m_set_wz(z80m_t* cpu, uint16_t v);
extern void z80m_set_sp(z80m_t* cpu, uint16_t v);
extern void z80m_set_i(z80m_t* cpu, uint8_t v);
extern void z80m_set_r(z80m_t* cpu, uint8_t v);
extern void z80m_set_ix(z80m_t* cpu, uint16_t v);
extern void z80m_set_iy(z80m_t* cpu, uint16_t v);
extern void z80m_set_im(z80m_t* cpu, uint8_t v);
extern void z80m_set_iff1(z80m_t* cpu, bool b);
extern void z80m_set_iff2(z80m_t* cpu, bool b);

extern uint8_t z80m_a(z80m_t* cpu);
extern uint8_t z80m_f(z80m_t* cpu);
extern uint8_t z80m_l(z80m_t* cpu);
extern uint8_t z80m_h(z80m_t* cpu);
extern uint8_t z80m_e(z80m_t* cpu);
extern uint8_t z80m_d(z80m_t* cpu);
extern uint8_t z80m_c(z80m_t* cpu);
extern uint8_t z80m_b(z80m_t* cpu);
extern uint16_t z80m_fa(z80m_t* cpu);
extern uint16_t z80m_hl(z80m_t* cpu);
extern uint16_t z80m_de(z80m_t* cpu);
extern uint16_t z80m_bc(z80m_t* cpu);
extern uint16_t z80m_fa_(z80m_t* cpu);
extern uint16_t z80m_hl_(z80m_t* cpu);
extern uint16_t z80m_de_(z80m_t* cpu);
extern uint16_t z80m_bc_(z80m_t* cpu);
extern uint16_t z80m_pc(z80m_t* cpu);
extern uint16_t z80m_wz(z80m_t* cpu);
extern uint16_t z80m_sp(z80m_t* cpu);
extern uint8_t z80m_i(z80m_t* cpu);
extern uint8_t z80m_r(z80m_t* cpu);
extern uint16_t z80m_ix(z80m_t* cpu);
extern uint16_t z80m_iy(z80m_t* cpu);
extern uint8_t z80m_im(z80m_t* cpu);
extern bool z80m_iff1(z80m_t* cpu);
extern bool z80m_iff2(z80m_t* cpu);

/* helper macro to start interrupt handling in tick callback */
#define Z80M_DAISYCHAIN_BEGIN(pins) if (pins&Z80M_M1) { pins|=Z80M_IEIO;
/* helper macro to end interrupt handling in tick callback */
#define Z80M_DAISYCHAIN_END(pins) pins&=~Z80M_RETI; }
/* return a pin mask with control-pins, address and data bus */
#define Z80M_MAKE_PINS(ctrl, addr, data) ((ctrl)|(((data)<<16)&0xFF0000ULL)|((addr)&0xFFFFULL))
/* extract 16-bit address bus from 64-bit pins */
#define Z80M_GET_ADDR(p) ((uint16_t)(p&0xFFFFULL))
/* merge 16-bit address bus value into 64-bit pins */
#define Z80M_SET_ADDR(p,a) {p=((p&~0xFFFFULL)|((a)&0xFFFFULL));}
/* extract 8-bit data bus from 64-bit pins */
#define Z80M_GET_DATA(p) ((uint8_t)((p&0xFF0000ULL)>>16))
/* merge 8-bit data bus value into 64-bit pins */
#define Z80M_SET_DATA(p,d) {p=((p&~0xFF0000ULL)|(((d)<<16)&0xFF0000ULL));}
/* extract number of wait states from pin mask */
#define Z80M_GET_WAIT(p) ((p&Z80M_WAIT_MASK)>>Z80M_WAIT_SHIFT)
/* set up to 7 wait states in pin mask */
#define Z80M_SET_WAIT(p,w) {p=((p&~Z80M_WAIT_MASK)|((((uint64_t)w)<<Z80M_WAIT_SHIFT)&Z80M_WAIT_MASK));}

#ifdef __cplusplus
} /* extern "C" */
#endif

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

/* register locations in register banks */
/* bank 0  and bank 1*/
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
#define _BITS_MAP_REGS (_BIT_USE_IX|_BIT_USE_IY)

/* set 8-bit immediate value in 64-bit register bank */
#define _S8(bank,shift,val)    bank=(((bank)&~(0xFFULL<<(shift)))|(((val)&0xFFULL)<<(shift)))
/* extract 8-bit value from 64-bit register bank */
#define _G8(bank,shift)         (((bank)>>(shift))&0xFFULL)
/* set 16-bit immediate value in 64-bit register bank */
#define _S16(bank,shift,val)   bank=((bank&~(0xFFFFULL<<(shift)))|(((val)&0xFFFFULL)<<(shift)))
/* special set WZ macro */
#define _SWZ(val) _S16(r1,_WZ,val)
/* extract 16-bit value from 64-bit register bank */
#define _G16(bank,shift)        (((bank)>>(shift))&0xFFFFULL)
/* special get WZ macro */
#define _GWZ() _G16(r1,_WZ)
/* set a single bit value in 64-bit register mask */
#define _S1(bank,shift,val)    bank=(((bank)&~(1ULL<<(shift)))|(((val)&1ULL)<<(shift)))
/* set 16-bit address bus pins */
#define _SA(addr) pins=(pins&~0xFFFFULL)|((addr)&0xFFFFULL)
/* set 16-bit address bus and 8-bit data bus pins */
#define _SAD(addr,data) pins=(pins&~0xFFFFFFULL)|((((data)&0xFFULL)<<16)&0xFF0000ULL)|((addr)&0xFFFFULL)
/* get 8-bit data bus value from pins */
#define _GD() ((uint8_t)((pins&0xFF0000ULL)>>16))
/* enable CPU control pins */
#define _ON(m) pins|=(m)
/* disable CPU control pins */
#define _OFF(m) pins&=~(m)
/* invoke tick callback (without wait state detection) */
#define _T(num) pins=tick(num,pins,ud);ticks+=num
/* invoke tick callback (with wait state detecion) */
#define _TW(num) pins&=~Z80M_WAIT_MASK;pins=tick(num,pins,ud);ticks+=num+Z80M_GET_WAIT(pins);
/* memory read machine cycle */
#define _MR(addr,data) _SA(addr);_ON(Z80M_MREQ|Z80M_RD);_TW(3);_OFF(Z80M_MREQ|Z80M_RD);data=_GD()
/* memory write machine cycle */
#define _MW(addr,data) _SAD(addr,data);_ON(Z80M_MREQ|Z80M_WR);_TW(3);_OFF(Z80M_MREQ|Z80M_WR)
/* input machine cycle */
#define _IN(addr,data) _SA(addr);_ON(Z80M_IORQ|Z80M_RD);_TW(4);_OFF(Z80M_IORQ|Z80M_RD);data=_GD()
/* output machine cycle */
#define _OUT(addr,data) _SAD(addr,data);_ON(Z80M_IORQ|Z80M_WR);_TW(4);_OFF(Z80M_IORQ|Z80M_WR)
/* read 8-bit immediate value */
#define _IMM8(data) _MR(pc++,data);
/* read 16-bit immediate value (also update WZ register) */
#define _IMM16(data) {uint8_t w,z;_MR(pc++,z);_MR(pc++,w);data=(w<<8)|z;_SWZ(data);} 
/* generate effective address for (HL), (IX+d), (IY+d) */
#define _ADDR(addr,ext_ticks) {addr=_G16(ws,_HL);if(r2&(_BIT_USE_IX|_BIT_USE_IY)){int8_t d;_MR(pc++,d);addr+=d;_SWZ(addr);_T(ext_ticks);}}
/* helper macro to bump R register */
#define _BUMPR() d8=_G8(r2,_R);d8=(d8&0x80)|((d8+1)&0x7F);_S8(r2,_R,d8)
/* a normal opcode fetch, bump R */
#define _FETCH(op) {_SA(pc++);_ON(Z80M_M1|Z80M_MREQ|Z80M_RD);_TW(4);_OFF(Z80M_M1|Z80M_MREQ|Z80M_RD);op=_GD();_BUMPR();}
/* special opcode fetch for CB prefix, only bump R if not a DD/FD+CB 'double prefix' op */
#define _FETCH_CB(op) {_SA(pc++);_ON(Z80M_M1|Z80M_MREQ|Z80M_RD);_TW(4);_OFF(Z80M_M1|Z80M_MREQ|Z80M_RD);op=_GD();if(0==(r2&(_BIT_USE_IX|_BIT_USE_IY))){_BUMPR();}}

/* register access functions */
void z80m_set_a(z80m_t* cpu, uint8_t v)         { _S8(cpu->bc_de_hl_fa,_A,v); }
void z80m_set_f(z80m_t* cpu, uint8_t v)         { _S8(cpu->bc_de_hl_fa,_F,v); }
void z80m_set_l(z80m_t* cpu, uint8_t v)         { _S8(cpu->bc_de_hl_fa,_L,v); }
void z80m_set_h(z80m_t* cpu, uint8_t v)         { _S8(cpu->bc_de_hl_fa,_H,v); }
void z80m_set_e(z80m_t* cpu, uint8_t v)         { _S8(cpu->bc_de_hl_fa,_E,v); }
void z80m_set_d(z80m_t* cpu, uint8_t v)         { _S8(cpu->bc_de_hl_fa,_D,v); }
void z80m_set_c(z80m_t* cpu, uint8_t v)         { _S8(cpu->bc_de_hl_fa,_C,v); }
void z80m_set_b(z80m_t* cpu, uint8_t v)         { _S8(cpu->bc_de_hl_fa,_B,v); }
void z80m_set_fa(z80m_t* cpu, uint16_t v)       { _S16(cpu->bc_de_hl_fa,_FA,v); }
void z80m_set_hl(z80m_t* cpu, uint16_t v)       { _S16(cpu->bc_de_hl_fa,_HL,v); }
void z80m_set_de(z80m_t* cpu, uint16_t v)       { _S16(cpu->bc_de_hl_fa,_DE,v); }
void z80m_set_bc(z80m_t* cpu, uint16_t v)       { _S16(cpu->bc_de_hl_fa,_BC,v); }
void z80m_set_fa_(z80m_t* cpu, uint16_t v)      { _S16(cpu->bc_de_hl_fa_,_FA,v); }
void z80m_set_hl_(z80m_t* cpu, uint16_t v)      { _S16(cpu->bc_de_hl_fa_,_HL,v); }
void z80m_set_de_(z80m_t* cpu, uint16_t v)      { _S16(cpu->bc_de_hl_fa_,_DE,v); }
void z80m_set_bc_(z80m_t* cpu, uint16_t v)      { _S16(cpu->bc_de_hl_fa_,_BC,v); }
void z80m_set_sp(z80m_t* cpu, uint16_t v)       { _S16(cpu->wz_ix_iy_sp,_SP,v); }
void z80m_set_iy(z80m_t* cpu, uint16_t v)       { _S16(cpu->wz_ix_iy_sp,_IY,v); }
void z80m_set_ix(z80m_t* cpu, uint16_t v)       { _S16(cpu->wz_ix_iy_sp,_IX,v); }
void z80m_set_wz(z80m_t* cpu, uint16_t v)       { _S16(cpu->wz_ix_iy_sp,_WZ,v); }
void z80m_set_pc(z80m_t* cpu, uint16_t v)       { _S16(cpu->im_ir_pc_bits,_PC,v); }
void z80m_set_ir(z80m_t* cpu, uint16_t v)       { _S16(cpu->im_ir_pc_bits,_IR,v); }
void z80m_set_i(z80m_t* cpu, uint8_t v)         { _S8(cpu->im_ir_pc_bits,_I,v); }
void z80m_set_r(z80m_t* cpu, uint8_t v)         { _S8(cpu->im_ir_pc_bits,_R,v); }
void z80m_set_im(z80m_t* cpu, uint8_t v)        { _S8(cpu->im_ir_pc_bits,_IM,v); }
void z80m_set_iff1(z80m_t* cpu, bool b)         { _S1(cpu->im_ir_pc_bits,_IFF1,(b?1:0)); }
void z80m_set_iff2(z80m_t* cpu, bool b)         { _S1(cpu->im_ir_pc_bits,_IFF2,(b?1:0)); }
void z80m_set_ei_pending(z80m_t* cpu, bool b)   { _S1(cpu->im_ir_pc_bits,_EI,(b?1:0)); }
uint8_t z80m_a(z80m_t* cpu)         { return _G8(cpu->bc_de_hl_fa,_A); }
uint8_t z80m_f(z80m_t* cpu)         { return _G8(cpu->bc_de_hl_fa,_F); }
uint8_t z80m_l(z80m_t* cpu)         { return _G8(cpu->bc_de_hl_fa,_L); }
uint8_t z80m_h(z80m_t* cpu)         { return _G8(cpu->bc_de_hl_fa,_H); }
uint8_t z80m_e(z80m_t* cpu)         { return _G8(cpu->bc_de_hl_fa,_E); }
uint8_t z80m_d(z80m_t* cpu)         { return _G8(cpu->bc_de_hl_fa,_D); }
uint8_t z80m_c(z80m_t* cpu)         { return _G8(cpu->bc_de_hl_fa,_C); }
uint8_t z80m_b(z80m_t* cpu)         { return _G8(cpu->bc_de_hl_fa,_B); }
uint16_t z80m_fa(z80m_t* cpu)       { return _G16(cpu->bc_de_hl_fa,_FA); }
uint16_t z80m_hl(z80m_t* cpu)       { return _G16(cpu->bc_de_hl_fa,_HL); }
uint16_t z80m_de(z80m_t* cpu)       { return _G16(cpu->bc_de_hl_fa,_DE); }
uint16_t z80m_bc(z80m_t* cpu)       { return _G16(cpu->bc_de_hl_fa,_BC); }
uint16_t z80m_fa_(z80m_t* cpu)      { return _G16(cpu->bc_de_hl_fa_,_FA); }
uint16_t z80m_hl_(z80m_t* cpu)      { return _G16(cpu->bc_de_hl_fa_,_HL); }
uint16_t z80m_de_(z80m_t* cpu)      { return _G16(cpu->bc_de_hl_fa_,_DE); }
uint16_t z80m_bc_(z80m_t* cpu)      { return _G16(cpu->bc_de_hl_fa_,_BC); }
uint16_t z80m_sp(z80m_t* cpu)       { return _G16(cpu->wz_ix_iy_sp,_SP); }
uint16_t z80m_iy(z80m_t* cpu)       { return _G16(cpu->wz_ix_iy_sp,_IY); }
uint16_t z80m_ix(z80m_t* cpu)       { return _G16(cpu->wz_ix_iy_sp,_IX); }
uint16_t z80m_wz(z80m_t* cpu)       { return _G16(cpu->wz_ix_iy_sp,_WZ); }
uint16_t z80m_pc(z80m_t* cpu)       { return _G16(cpu->im_ir_pc_bits,_PC); }
uint16_t z80m_ir(z80m_t* cpu)       { return _G16(cpu->im_ir_pc_bits,_IR); }
uint8_t z80m_i(z80m_t* cpu)         { return _G8(cpu->im_ir_pc_bits,_I); }
uint8_t z80m_r(z80m_t* cpu)         { return _G8(cpu->im_ir_pc_bits,_R); }
uint8_t z80m_im(z80m_t* cpu)        { return _G8(cpu->im_ir_pc_bits,_IM); }
bool z80m_iff1(z80m_t* cpu)         { return 0 != (cpu->im_ir_pc_bits & _BIT_IFF1); }
bool z80m_iff2(z80m_t* cpu)         { return 0 != (cpu->im_ir_pc_bits & _BIT_IFF2); }
bool z80m_ei_pending(z80m_t* cpu)   { return 0 != (cpu->im_ir_pc_bits & _BIT_EI); }

void z80m_init(z80m_t* cpu, z80m_desc_t* desc) {
    CHIPS_ASSERT(_FA == 0);
    CHIPS_ASSERT(cpu && desc);
    CHIPS_ASSERT(desc->tick_cb);
    memset(cpu, 0, sizeof(*cpu));
    z80m_reset(cpu);
    cpu->trap_addr = 0xFFFFFFFFFFFFFFFF;
    cpu->tick = desc->tick_cb;
    cpu->user_data = desc->user_data;
}

void z80m_reset(z80m_t* cpu) {
    CHIPS_ASSERT(cpu);
    /* set AF to 0xFFFF, all other regs are undefined, set to 0xFFFF to */
    cpu->bc_de_hl_fa = cpu->bc_de_hl_fa_ = 0xFFFFFFFFFFFFFFFFULL;
    z80m_set_ix(cpu, 0xFFFF);
    z80m_set_iy(cpu, 0xFFFF);
    z80m_set_wz(cpu, 0xFFFF);
    /* set SP to 0xFFFF, PC to 0x0000 */
    z80m_set_sp(cpu, 0xFFFF);
    z80m_set_pc(cpu, 0x0000);
    /* IFF1 and IFF2 are off */
    z80m_set_iff1(cpu, false);
    z80m_set_iff2(cpu, false);
    /* IM is set to 0 */
    z80m_set_im(cpu, 0);
    /* after power-on or reset, R is set to 0 (see z80-documented.pdf) */
    z80m_set_ir(cpu, 0x0000);
    cpu->im_ir_pc_bits &= ~(_BIT_EI|_BIT_USE_IX|_BIT_USE_IY);
}

void z80m_set_trap(z80m_t* cpu, int trap_id, uint16_t addr) {
    CHIPS_ASSERT(cpu && (trap_id >= 0) && (trap_id < Z80M_MAX_NUM_TRAPS));
    cpu->trap_addr &= ~(0xFFFFULL<<(trap_id<<4));
    cpu->trap_addr |= addr<<(trap_id<<4);
}

void z80m_clear_trap(z80m_t* cpu, int trap_id) {
    CHIPS_ASSERT(cpu && (trap_id >= 0) && (trap_id < Z80M_MAX_NUM_TRAPS));
    cpu->trap_addr |= 0xFFFFULL<<(trap_id<<4);
}

bool z80m_has_trap(z80m_t* cpu, int trap_id) {
    CHIPS_ASSERT(cpu && (trap_id >= 0) && (trap_id < Z80M_MAX_NUM_TRAPS));
    return (cpu->trap_addr>>(trap_id<<4) & 0xFFFF) != 0xFFFF;
}

bool z80m_opdone(z80m_t* cpu) {
    return 0 == (cpu->im_ir_pc_bits & (_BIT_USE_IX|_BIT_USE_IY));
}

/* flags evaluation */
static inline uint8_t _z80m_sz(uint8_t val) {
    return val ? (val & Z80M_SF) : Z80M_ZF;
}

static inline uint8_t _z80m_szyxch(uint8_t acc, uint8_t val, uint32_t res) {
    return _z80m_sz(res)|(res&(Z80M_YF|Z80M_XF))|((res>>8)&Z80M_CF)|((acc^val^res)&Z80M_HF);
}

static inline uint8_t _z80m_add_flags(uint8_t acc, uint8_t val, uint32_t res) {
    return _z80m_szyxch(acc,val,res)|((((val^acc^0x80)&(val^res))>>5)&Z80M_VF);
}

static inline uint8_t _z80m_sub_flags(uint8_t acc, uint8_t val, uint32_t res) {
    return Z80M_NF|_z80m_szyxch(acc,val,res)|((((val^acc)&(res^acc))>>5)&Z80M_VF);
}

static inline uint8_t _z80m_cp_flags(uint8_t acc, uint8_t val, uint32_t res) {
    return Z80M_NF|(_z80m_sz(res)|(val&(Z80M_YF|Z80M_XF))|((res>>8)&Z80M_CF)|((acc^val^res)&Z80M_HF))|((((val^acc)&(res^acc))>>5)&Z80M_VF);
}

static inline uint8_t _z80m_sziff2_flags(uint64_t ws, uint64_t r2, uint8_t val) {
    uint8_t f = _G8(ws,_F) & Z80M_CF;
    f |= _z80m_sz(val)|(val&(Z80M_YF|Z80M_XF))|((r2 & _BIT_IFF2) ? Z80M_PF : 0);
    return f;
}

/* sign+zero+parity lookup table */
static uint8_t _z80m_szp[256] = {
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

/* ALU functions */
static inline uint64_t _z80m_add8(uint64_t ws, uint8_t val) {
    uint8_t acc = _G8(ws,_A);
    uint32_t res = acc + val;
    _S8(ws,_F,_z80m_add_flags(acc,val,res));
    _S8(ws,_A,res);
    return ws;
}

static inline uint64_t _z80m_adc8(uint64_t ws, uint8_t val) {
    uint8_t acc = _G8(ws,_A);
    uint32_t res = acc + val + (_G8(ws,_F) & Z80M_CF);
    _S8(ws,_F,_z80m_add_flags(acc,val,res));
    _S8(ws,_A,res);
    return ws;
}

static inline uint64_t _z80m_sub8(uint64_t ws, uint8_t val) {
    uint8_t acc = _G8(ws,_A);
    uint32_t res = (uint32_t) ((int)acc - (int)val);
    _S8(ws,_F,_z80m_sub_flags(acc,val,res));
    _S8(ws,_A,res);
    return ws;
}

static inline uint64_t _z80m_neg8(uint64_t ws) {
    uint8_t val = _G8(ws,_A);
    _S8(ws,_A,0);
    return _z80m_sub8(ws,val);
}

static inline uint64_t _z80m_sbc8(uint64_t ws, uint8_t val) {
    uint8_t acc = _G8(ws,_A);
    uint32_t res = (uint32_t) ((int)acc - (int)val - (_G8(ws,_F) & Z80M_CF));
    _S8(ws,_F,_z80m_sub_flags(acc,val,res));
    _S8(ws,_A,res);
    return ws;
}

static inline uint64_t _z80m_and8(uint64_t ws, uint8_t val) {
    val &= _G8(ws,_A);
    _S8(ws,_F,_z80m_szp[val]|Z80M_HF);
    _S8(ws,_A,val);
    return ws;
}

static inline uint64_t _z80m_xor8(uint64_t ws, uint8_t val) {
    val ^= _G8(ws,_A);
    _S8(ws,_F,_z80m_szp[val]);
    _S8(ws,_A,val);
    return ws;
}

static inline uint64_t _z80m_or8(uint64_t ws, uint8_t val) {
    val |= _G8(ws,_A);
    _S8(ws,_F,_z80m_szp[val]);
    _S8(ws,_A,val);
    return ws;
}

static inline uint64_t _z80m_cp8(uint64_t ws, uint8_t val) {
    uint8_t acc = _G8(ws,_A);
    uint32_t res = (uint32_t) ((int)acc - (int)val);
    _S8(ws,_F,_z80m_cp_flags(acc,val,res));
    return ws;
}

static inline uint64_t _z80m_alu8(uint8_t type, uint64_t ws, uint8_t val) {
    switch (type) {
        case 0: return _z80m_add8(ws,val); break;
        case 1: return _z80m_adc8(ws,val); break;
        case 2: return _z80m_sub8(ws,val); break;
        case 3: return _z80m_sbc8(ws,val); break;
        case 4: return _z80m_and8(ws,val); break;
        case 5: return _z80m_xor8(ws,val); break;
        case 6: return _z80m_or8(ws,val); break;
        case 7: return _z80m_cp8(ws,val); break;
    }
    /* can't happen */
    return ws;
}

static inline uint64_t _z80m_daa(uint64_t ws) {
    uint8_t a = _G8(ws,_A);
    uint8_t v = a;
    uint8_t f = _G8(ws,_F);
    if (f & Z80M_NF) {
        if (((a & 0xF)>0x9) || (f & Z80M_HF)) {
            v -= 0x06;
        }
        if ((a > 0x99) || (f & Z80M_CF)) {
            v -= 0x60;
        }
    }
    else {
        if (((a & 0xF)>0x9) || (f & Z80M_HF)) {
            v += 0x06;
        }
        if ((a > 0x99) || (f & Z80M_CF)) {
            v += 0x60;
        }
    }
    f &= Z80M_CF|Z80M_NF;
    f |= (a>0x99) ? Z80M_CF : 0;
    f |= (a ^ v) & Z80M_HF;
    f |= _z80m_szp[v];
    _S8(ws,_A,v);
    _S8(ws,_F,f);
    return ws;
}

static inline uint64_t _z80m_cpl(uint64_t ws) {
    uint8_t a = _G8(ws,_A) ^ 0xFF;
    _S8(ws,_A,a);
    uint8_t f = _G8(ws,_F);
    f = (f & (Z80M_SF|Z80M_ZF|Z80M_PF|Z80M_CF)) | Z80M_HF | Z80M_NF | (a & (Z80M_YF|Z80M_XF));
    _S8(ws,_F,f);
    return ws;
}

static inline uint64_t _z80m_scf(uint64_t ws) {
    uint8_t a = _G8(ws,_A);
    uint8_t f = _G8(ws,_F);
    f = (f & (Z80M_SF|Z80M_ZF|Z80M_PF|Z80M_CF)) | Z80M_CF | (a & (Z80M_YF|Z80M_XF));
    _S8(ws,_F,f);
    return ws;
}

static inline uint64_t _z80m_ccf(uint64_t ws) {
    uint8_t a = _G8(ws,_A);
    uint8_t f = _G8(ws,_F);
    f = ((f & (Z80M_SF|Z80M_ZF|Z80M_YF|Z80M_XF|Z80M_PF|Z80M_CF)) | ((f & Z80M_CF)<<4) | (a & (Z80M_YF|Z80M_XF))) ^ Z80M_CF;
    _S8(ws,_F,f);
    return ws;
}

static inline uint64_t _z80m_rlca(uint64_t ws) {
    uint8_t a = _G8(ws,_A);
    uint8_t f = _G8(ws,_F);
    uint8_t r = (a<<1) | (a>>7);
    f = ((a>>7) & Z80M_CF) | (f & (Z80M_SF|Z80M_ZF|Z80M_PF)) | (r & (Z80M_YF|Z80M_XF));
    _S8(ws,_A,r);
    _S8(ws,_F,f);
    return ws;
}

static inline uint64_t _z80m_rrca(uint64_t ws) {
    uint8_t a = _G8(ws,_A);
    uint8_t f = _G8(ws,_F);
    uint8_t r = (a>>1) | (a<<7);
    f = (a & Z80M_CF) | (f & (Z80M_SF|Z80M_ZF|Z80M_PF)) | (r & (Z80M_YF|Z80M_XF));
    _S8(ws,_A,r);
    _S8(ws,_F,f);
    return ws;
}

static inline uint64_t _z80m_rla(uint64_t ws) {
    uint8_t a = _G8(ws,_A);
    uint8_t f = _G8(ws,_F);
    uint8_t r = (a<<1) | (f & Z80M_CF);
    f = ((a>>7) & Z80M_CF) | (f & (Z80M_SF|Z80M_ZF|Z80M_PF)) | (r & (Z80M_YF|Z80M_XF));
    _S8(ws,_A,r);
    _S8(ws,_F,f);
    return ws;
}

static inline uint64_t _z80m_rra(uint64_t ws) {
    uint8_t a = _G8(ws,_A);
    uint8_t f = _G8(ws,_F);
    uint8_t r = (a>>1) | ((f & Z80M_CF)<<7);
    f = (a & Z80M_CF) | (f & (Z80M_SF|Z80M_ZF|Z80M_PF)) | (r & (Z80M_YF|Z80M_XF));
    _S8(ws,_A,r);
    _S8(ws,_F,f);
    return ws;
}

static inline bool _z80m_cond(uint64_t ws, uint8_t cc) {
    const uint8_t f = _G8(ws,_F);
    bool res = false;
    switch (cc>>1) {
        case 0: res = f & Z80M_ZF; break;   /* NZ,Z */
        case 1: res = f & Z80M_CF; break;   /* NC,C */
        case 2: res = f & Z80M_PF; break;   /* PO,PE */
        case 3: res = f & Z80M_SF; break;   /* P,M */
    }
    if (!(cc & 1)) {
        res = !res;
    }
    return res;
}

/* manage the virtual 'working set' register bank */
static inline uint64_t _z80m_map_regs(uint64_t r0, uint64_t r1, uint64_t r2) {
    uint64_t ws = r0;
    if (r2 & _BIT_USE_IX) {
        ws = (ws & ~(0xFFFFULL<<_HL)) | (((r1>>_IX)<<_HL) & (0xFFFFULL<<_HL));
    }
    else if (r2 & _BIT_USE_IY) {
        ws = (ws & ~(0xFFFFULL<<_HL)) | (((r1>>_IY)<<_HL) & (0xFFFFULL<<_IY));
    }
    return ws;
}

static inline uint64_t _z80m_flush_r0(uint64_t ws, uint64_t r0, uint64_t map_bits) {
    if (map_bits & (_BIT_USE_IX|_BIT_USE_IY)) {
        r0 = (r0 & (0xFFFFULL<<_HL)) | (ws & ~(0xFFFFULL<<_HL));
    }
    else {
        r0 = ws;
    }
    return r0;
}

static inline uint64_t _z80m_flush_r1(uint64_t ws, uint64_t r1, uint64_t map_bits) {
    if (map_bits & _BIT_USE_IX) {
        r1 = (r1 & ~(0xFFFFULL<<_IX)) | (((ws>>_HL)<<_IX) & (0xFFFFULL<<_IX));
    }
    else if (map_bits & _BIT_USE_IY) {
        r1 = (r1 & ~(0xFFFFULL<<_IY)) | (((ws>>_HL)<<_IY) & (0xFFFFULL<<_IY));
    }
    return r1;
}

/* instruction decoder */
uint32_t z80m_exec(z80m_t* cpu, uint32_t num_ticks) {
    uint64_t r0 = cpu->bc_de_hl_fa;
    uint64_t r1 = cpu->wz_ix_iy_sp;
    uint64_t r2 = cpu->im_ir_pc_bits;
    uint64_t r3 = cpu->bc_de_hl_fa_;
    uint64_t ws = _z80m_map_regs(r0, r1, r2);
    uint64_t map_bits = r2 & _BITS_MAP_REGS;
    uint64_t pins = cpu->pins;
    const uint64_t trap_addr = cpu->trap_addr;
    const z80m_tick_t tick = cpu->tick;
    void* ud = cpu->user_data;
    int trap_id = -1;
    uint32_t ticks = 0;
    uint8_t op, d8;
    uint16_t addr, d16;
    uint16_t pc = _G16(r2,_PC);
    do {
        /* switch off interrupt flag */
        _OFF(Z80M_INT);
        /* delay-enable interrupt flags */
        if (r2 & _BIT_EI) {
            r2 &= ~_BIT_EI;
            r2 |= (_BIT_IFF1 | _BIT_IFF2);
        }
        /* fetch opcode machine cycle, bump R register (4 cycles) */
        _FETCH(op)

        /* ED prefix resets the IX/IY mapping flags (this may happen for invalid
            opcode sequences like FF ED, DD ED etc..)
        */
        if (op == 0xED) { 
            map_bits &= ~(_BIT_USE_IX|_BIT_USE_IY);
        }

        /* flush and update the working set if register mapping has changed */
        if (map_bits != (r2 & _BITS_MAP_REGS)) {
            const uint64_t old_map_bits = r2 & _BITS_MAP_REGS;
            r0 = _z80m_flush_r0(ws, r0, old_map_bits);
            r1 = _z80m_flush_r1(ws, r1, old_map_bits);
            r2 = (r2 & ~_BITS_MAP_REGS) | map_bits; 
            ws = _z80m_map_regs(r0, r1, r2);
        }

        /* split opcode into bitgroups
            |xx|yyy|zzz|
            |xx|pp|q|zzz|
        */
        const uint8_t y = (op>>3)&7;
        const uint8_t z = op&7;
        const uint8_t p = y>>1;
        const uint8_t q = y&1;
        const int ry = (7-y)<<3;
        const int rz = (7-z)<<3;
        const int rp = (3-p)<<4;

        switch (op) {
        /*=== BLOCK 2: 8-bit ALU instructions ================================*/
            /* LD (HL),r; LD (IX+d),r; LD (IY+d),r */
            case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x77:
                if ((map_bits & (_BIT_USE_IX|_BIT_USE_IY)) && ((z>>1) == 2)) {
                    /* special case LD (IX/IY+d),H; LD (IX/IY+d),L, these need to
                        store the original H/L registers, not IXH/IXL
                    */
                    d8 = _G8(r0, rz);
                }
                else {
                    d8=_G8(ws,rz);
                }
                _ADDR(addr,5);
                _MW(addr,d8); 
                break;
            /* special case HALT */
            case 0x76:
                _ON(Z80M_HALT); pc--;
                break;
            /* LD r,s */
            case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x47:
            case 0x48: case 0x49: case 0x4A: case 0x4B: case 0x4C: case 0x4D: case 0x4F:
            case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x57:
            case 0x58: case 0x59: case 0x5A: case 0x5B: case 0x5C: case 0x5D: case 0x5F:
            case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x67:
            case 0x68: case 0x69: case 0x6A: case 0x6B: case 0x6C: case 0x6D: case 0x6F:
            case 0x78: case 0x79: case 0x7A: case 0x7B: case 0x7C: case 0x7D: case 0x7F:
                d8=_G8(ws,rz);
                _S8(ws,ry,d8);
                break;
            /* LD r,(HL); LD r,(IX+d); LD r,(IY+d) */
            case 0x46: case 0x4E: case 0x56: case 0x5E: case 0x66: case 0x6E: case 0x7E:
                _ADDR(addr,5);
                _MR(addr,d8); 
                if ((map_bits & (_BIT_USE_IX|_BIT_USE_IY)) && ((y>>1) == 2)) {
                    /* special case LD H,(IX/IY+d), LD L,(IX/IY+d), these need to
                    access the original H/L registers, not IXH/IXL
                    */
                    _S8(r0,ry,d8);
                }
                else {
                    _S8(ws,ry,d8);
                }
                break;
        /*=== BLOCK 2: 8-bit ALU instructions ================================*/
            /* ALU (HL); ALU (IX+d); ALU (IY+d) */
            case 0x86: case 0x8E: case 0x96: case 0x9E: case 0xA6: case 0xAE: case 0xB6: case 0xBE:
                _ADDR(addr,5); _MR(addr,d8);
                ws = _z80m_alu8(y,ws,d8);
                break;
            /* ALU r */
            case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x87:
            case 0x88: case 0x89: case 0x8A: case 0x8B: case 0x8C: case 0x8D: case 0x8F:
            case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x97:
            case 0x98: case 0x99: case 0x9A: case 0x9B: case 0x9C: case 0x9D: case 0x9F:
            case 0xA0: case 0xA1: case 0xA2: case 0xA3: case 0xA4: case 0xA5: case 0xA7:
            case 0xA8: case 0xA9: case 0xAA: case 0xAB: case 0xAC: case 0xAD: case 0xAF:
            case 0xB0: case 0xB1: case 0xB2: case 0xB3: case 0xB4: case 0xB5: case 0xB7:
            case 0xB8: case 0xB9: case 0xBA: case 0xBB: case 0xBC: case 0xBD: case 0xBF:
                ws = _z80m_alu8(y,ws,_G8(ws,rz));
                break;
        /*=== BLOCK 0: misc instructions =====================================*/
            /* NOP */
            case 0x00: 
                break;
            /* EX AF,AF' */
            case 0x08:
                r0 = _z80m_flush_r0(ws, r0, r2);
                uint16_t fa = _G16(r0,_FA);
                uint16_t fa_ = _G16(r3, _FA);
                _S16(r0,_FA,fa_);
                _S16(r3,_FA,fa);
                ws = _z80m_map_regs(r0, r1, r2);
                break;
            /* DJNZ */
            case 0x10:
                {
                    _T(1);
                    int8_t d; _IMM8(d);
                    d8 = _G8(ws,_B) - 1;
                    _S8(ws,_B,d8);
                    if (d8 > 0) {
                        pc += d;
                        _SWZ(pc);
                        _T(5);
                    }
                }
                break;
            /* JR d */
            case 0x18:
                {
                    int8_t d; _IMM8(d); pc += d;
                    _SWZ(pc);
                    _T(5);
                }
                break;
            /* JR cc,d */
            case 0x20: case 0x28: case 0x30: case 0x38:
                {
                    int8_t d; _IMM8(d);
                    if (_z80m_cond(ws,y-4)) {
                        pc += d;
                        _SWZ(pc);
                        _T(5);
                    }
                }
                break;
            /* 16-bit immediate loads (AF => SP)*/
            case 0x01: case 0x11: case 0x21: case 0x31:
                _IMM16(d16);
                if (_FA==rp) { _S16(r1,_SP,d16); } /* LD SP,nn */
                else         { _S16(ws,rp,d16); } /* LD HL,nn; LD DE,nn; LD BC,nn */
                break;
            /* ADD HL,rr; ADD IX,rr; ADD IY,rr */
            case 0x09: case 0x19: case 0x29: case 0x39:
                {
                    uint16_t acc = _G16(ws,_HL);
                    _SWZ(acc+1);
                    if (_FA==rp) { d16 = _G16(r1,_SP); }  /* ADD HL,SP */
                    else         { d16 = _G16(ws,rp); }   /* ADD HL,dd */
                    uint32_t r = acc + d16;
                    _S16(ws,_HL,r);
                    uint8_t f = _G8(ws,_F) & (Z80M_SF|Z80M_ZF|Z80M_VF);
                    f |= ((acc^r^d16)>>8) & Z80M_HF;
                    f |= ((r>>16) & Z80M_CF)|((r>>8)&(Z80M_YF|Z80M_XF));
                    _S8(ws,_F,f);
                    _T(7);
                }
                break;
            /* indirect loads */
            case 0x02: case 0x0A: case 0x12: case 0x1A: case 0x22: case 0x2A: case 0x32: case 0x3A:
                switch (p) { /* get effective address (BC),(DE) or (nn) */
                    case 0:     addr=_G16(ws,_BC); break;
                    case 1:     addr=_G16(ws,_DE); break;
                    default:    _IMM16(addr); break;
                }
                if (q == 0) { /* store */
                    if (p == 2) { /* LD (nn),HL; LD (nn),IX; LD (nn),IY, WZ=addr++ */
                        _MW(addr++,_G8(ws,_L)); _MW(addr,_G8(ws,_H)); _SWZ(addr);
                    }
                    else { /* LD (BC),A; LD (DE),A; LD (nn),A; W=A,L=addr++ */
                        d8=_G8(ws,_A); _MW(addr++,d8); _SWZ(((d8<<8)|(addr&0x00FF)));
                    }
                }
                else { /* load */
                    if (p == 2) { /* LD HL,(nn); LD IX,(nn); LD IY,(nn) */
                        _MR(addr++,d8); _S8(ws,_L,d8); _MR(addr,d8); _S8(ws,_H,d8); _SWZ(addr);
                    }
                    else {  /* LD A,(BC); LD A,(DE); LD A,(nn); W=addr++ */
                        _MR(addr++,d8); _S8(ws,_A,d8); _SWZ(addr);
                    }
                }
                break;
            /* 16-bit INC/DEC */
            case 0x03: case 0x0B: case 0x13: case 0x1B: case 0x23: case 0x2B: case 0x33: case 0x3B:
                _T(2);
                if (rp==_FA) { d16=_G16(r1,_SP); }
                else         { d16=_G16(ws,rp); }
                d16 = d16 + (q ? -1 : +1);
                if (rp==_FA) { _S16(r1,_SP,d16); }
                else         { _S16(ws,rp,d16); }
                break;
            /* 8-bit INC (HL); INC (IX+d); INC (IY+d); INC r */
            /* 8-bit DEC (HL); DEC (IX+d); DEC (IY+d); DEC r */
            case 0x04: case 0x05: case 0x0C: case 0x0D: case 0x14: case 0x15: case 0x1C: case 0x1D:
            case 0x24: case 0x25: case 0x2C: case 0x2D: case 0x34: case 0x35: case 0x3C: case 0x3D:
                {
                    if (y == 6) { _ADDR(addr,5); _MR(addr,d8); _T(1); }
                    else        { d8 = _G8(ws,ry); }
                    uint8_t r = d8 + ((z & 1) ? -1 : +1);
                    if (y == 6) { _MW(addr,r); }
                    else        { _S8(ws,ry,r); }
                    uint8_t f = _G8(ws,_F) & Z80M_CF;
                    f |= _z80m_sz(r)|(r&(Z80M_XF|Z80M_YF))|((r^d8)&Z80M_HF);
                    if (z & 1) {
                        f |= Z80M_NF;
                        if (r == 0x7F) { f |= Z80M_VF; }
                    }
                    else {
                        if (r == 0x80) { f |= Z80M_VF; }
                    }
                    _S8(ws,_F,f);
                }
                break;
            /* LD (HL),n; LD (IX+d),n; LD (IY+d),n */
            case 0x36:
                _ADDR(addr,2); _IMM8(d8); _MW(addr,d8);
                break;
            /* LD r,n */
            case 0x06: case 0x0E: case 0x16: case 0x1E: case 0x26: case 0x2E: case 0x3E:
                _IMM8(d8); _S8(ws,ry,d8);
                break;
            /* misc ops on A and F */
            case 0x07: ws=_z80m_rlca(ws); break;
            case 0x0F: ws=_z80m_rrca(ws); break;
            case 0x17: ws=_z80m_rla(ws); break;
            case 0x1F: ws=_z80m_rra(ws); break;
            case 0x27: ws=_z80m_daa(ws); break;
            case 0x2F: ws=_z80m_cpl(ws); break;
            case 0x37: ws=_z80m_scf(ws); break;
            case 0x3F: ws=_z80m_ccf(ws); break;
        /*=== BLOCK 3: misc and extended ops =================================*/
            /* RET cc */
            case 0xC0: case 0xC8: case 0xD0: case 0xD8: case 0xE0: case 0xE8: case 0xF0: case 0xF8:
                _T(1);
                if (_z80m_cond(ws,y)) {
                    uint8_t w,z;
                    uint16_t sp = _G16(r1,_SP);
                    _MR(sp++,z);
                    _MR(sp++,w);
                    _S16(r1,_SP,sp);
                    pc = (w<<8)|z;
                    _SWZ(pc);
                }
                break;
            /* POP BC,DE,HL,AF,IX,IY */
            case 0xC1: case 0xD1: case 0xE1: case 0xF1:
                {
                    addr = _G16(r1,_SP);
                    uint8_t l,h;
                    _MR(addr++,l); _MR(addr++,h);
                    d16 = (rp==_FA) ? ((l<<8)|h) : ((h<<8)|l);
                    _S16(ws,rp,d16); _S16(r1,_SP,addr);
                }
                break;
            /* RET */
            case 0xC9:
                {
                    uint8_t w,z;
                    uint16_t sp = _G16(r1,_SP);
                    _MR(sp++,z);
                    _MR(sp++,w);
                    _S16(r1,_SP,sp);
                    pc = (w<<8)|z;
                    _SWZ(pc);
                }
                break;
            /* EXX */
            case 0xD9:
                r0 = _z80m_flush_r0(ws, r0, r2);
                const uint64_t rx = r3;
                r3 = (r3 & 0xFFFF) | (r0 & 0xFFFFFFFFFFFF0000);
                r0 = (r0 & 0xFFFF) | (rx & 0xFFFFFFFFFFFF0000);
                ws = _z80m_map_regs(r0, r1, r2);
                break;
            /* JP (HL), JP (IX), JP (IY) */
            case 0xE9:
                pc = _G16(ws,_HL);
                break;
            /* LD SP,HL; LD SP,IX; LD SP,IY */
            case 0xF9:
                _T(2); _S16(r1,_SP,_G16(ws,_HL));
                break;
            /* JP cc,nn */
            case 0xC2: case 0xCA: case 0xD2: case 0xDA: case 0xE2: case 0xEA: case 0xF2: case 0xFA:
                _IMM16(addr);
                if (_z80m_cond(ws,y)) {
                    pc = addr;
                }
                break;
            /* JP nn */
            case 0xC3:
                _IMM16(pc);
                break;
            /* CB prefix */
            case 0xCB:
                {
                    /* special handling for undocumented DD/FD+CB double prefix instructions,
                        these always load the value from memory (IX+d),
                        and write the value back, even for normal
                        'register' instructions
                        see: http://www.baltazarstudios.com/files/ddcb.html
                    */
                    /* load the d offset for indexed instructions */
                    int8_t d = 0;
                    if (r2 & (_BIT_USE_IX|_BIT_USE_IY)) {
                        _IMM8(d);
                    }
                    /* fetch opcode without memory refresh and incrementint R */
                    _FETCH_CB(op);
                    const uint8_t x = op>>6;
                    const uint8_t y = (op>>3)&7;
                    const uint8_t z = op&7;
                    const int rz = (7-z)<<3;
                    /* load the operand (for indexed ops, always from memory!) */
                    if ((z == 6) || (r2 & (_BIT_USE_IX|_BIT_USE_IY))) {
                        _T(1);
                        addr = _G16(ws,_HL);
                        if (r2 & (_BIT_USE_IX|_BIT_USE_IY)) {
                            _T(1);
                            addr += d;
                            _SWZ(addr);
                        }
                        _MR(addr,d8);
                    }
                    else {
                        /* simple non-indexed, non-(HL): load register value */
                        d8 = _G8(ws,rz);
                    }
                    uint8_t f = _G8(ws,_F);
                    uint8_t r;
                    switch (x) {
                        case 0:
                            /* rot/shift */
                            switch (y) {
                                case 0: /*RLC*/ r=d8<<1|d8>>7; f=_z80m_szp[r]|(d8>>7&Z80M_CF); break;
                                case 1: /*RRC*/ r=d8>>1|d8<<7; f=_z80m_szp[r]|(d8&Z80M_CF); break;
                                case 2: /*RL */ r=d8<<1|(f&Z80M_CF); f=_z80m_szp[r]|(d8>>7&Z80M_CF); break;
                                case 3: /*RR */ r=d8>>1|((f&Z80M_CF)<<7); f=_z80m_szp[r]|(d8&Z80M_CF); break;
                                case 4: /*SLA*/ r=d8<<1; f=_z80m_szp[r]|(d8>>7&Z80M_CF); break;
                                case 5: /*SRA*/ r=d8>>1|(d8&0x80); f=_z80m_szp[r]|(d8&Z80M_CF); break;
                                case 6: /*SLL*/ r=d8<<1|1; f=_z80m_szp[r]|(d8>>7&Z80M_CF); break;
                                case 7: /*SRL*/ r=d8>>1; f=_z80m_szp[r]|(d8&Z80M_CF); break;
                            }
                            break;
                        case 1:
                            /* BIT (bit test) */
                            r = d8 & (1<<y); 
                            f = (f&Z80M_CF) | Z80M_HF | (r?(r&Z80M_SF):(Z80M_ZF|Z80M_PF));
                            if (z == 6) {
                                f |= (_GWZ()>>8) & (Z80M_YF|Z80M_XF);
                            }
                            else {
                                f |= d8 & (Z80M_YF|Z80M_XF);
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
                        if ((z == 6) || (r2 & (_BIT_USE_IX|_BIT_USE_IY))) {
                            /* (HL), (IX+d), (IY+d): write back to memory, for extended ops,
                                even when the op is actually a register op
                            */
                            _MW(addr,r);
                        }
                        if (z != 6) {
                            /* write result back to register */
                            _S8(ws,rz,r);
                        }
                    }
                    _S8(ws,_F,f);
                }
                break;
            /* OUT (n),A */
            case 0xD3:
                {
                    _IMM8(d8);
                    uint8_t a = _G8(ws,_A);
                    addr = (a<<8)|d8;
                    _OUT(addr,a);
                    /* special WZ computation, only bump Z */
                    addr = (addr & 0xFF00) | ((addr+1) & 0x00FF);
                    _SWZ(addr);
                }
                break;
            /* IN A(n) */
            case 0xDB:
                {
                    _IMM8(d8);
                    uint8_t a = _G8(ws,_A);
                    addr = (a<<8)|d8;
                    _IN(addr++,a);
                    _S8(ws,_A,a);
                    _SWZ(addr);
                }
                break;
            /* EX SP,(HL/IX/IY) */
            case 0xE3:
                {
                    _T(3);
                    addr = _G16(r1,_SP);
                    d16 = _G16(ws,_HL);
                    uint8_t l,h;
                    _MR(addr,l);
                    _MR(addr+1,h);
                    _MW(addr,d16);
                    _MW(addr+1,d16>>8);
                    d16 = (h<<8)|l;
                    _S16(ws,_HL,d16);
                    _SWZ(d16);
                }
                break;
            /* EX DE,HL */
            case 0xEB:
                {
                    r0 = _z80m_flush_r0(ws, r0, r2);
                    uint16_t de = _G16(r0,_DE);
                    uint16_t hl = _G16(r0,_HL);
                    _S16(r0,_DE,hl);
                    _S16(r0,_HL,de);
                    ws = _z80m_map_regs(r0, r1, r2);
                }
                break;
            /* DI */
            case 0xF3:
                r2 &= ~(_BIT_IFF1|_BIT_IFF2);
                break;
            /* EI (ints enabled at start of next op) */
            case 0xFB:
                r2 |= _BIT_EI;
                break;
            /* CALL cc,nn */
            case 0xC4: case 0xCC: case 0xD4: case 0xDC: case 0xE4: case 0xEC: case 0xF4: case 0xFC:
                _IMM16(addr);
                if (_z80m_cond(ws,y)) {
                    _T(1);
                    uint16_t sp = _G16(r1,_SP);
                    _MW(--sp, pc>>8);
                    _MW(--sp, pc);
                    _S16(r1,_SP,sp);
                    pc = addr;
                }
                break;
            /* PUSH BC,DE,HL,IX,IY,AF */
            case 0xC5: case 0xD5: case 0xE5: case 0xF5:
                _T(1);
                addr = _G16(r1,_SP);
                d16 = _G16(ws,rp);
                if (rp==_FA) {
                    d16 = (d16>>8) | (d16<<8);
                }
                _MW(--addr,d16>>8); _MW(--addr,d16);
                _S16(r1,_SP,addr);
                break;
            /* CALL nn */
            case 0xCD:
                {
                    _IMM16(addr);
                    _T(1);
                    uint16_t sp = _G16(r1,_SP);
                    _MW(--sp, pc>>8);
                    _MW(--sp, pc);
                    _S16(r1,_SP,sp);
                    pc = addr;
                }
                break;
            /* DD prefix (maps IX into HL slot, don't handle interrupt) */
            case 0xDD:
                map_bits |= _BIT_USE_IX;
                continue;
            /* FD prefix (maps IY into HL slot, don't handle interrupt) */
            case 0xFD:
                map_bits |= _BIT_USE_IY;
                continue;
            /* ALU n */
            case 0xC6: case 0xCE: case 0xD6: case 0xDE: case 0xE6: case 0xEE: case 0xF6: case 0xFE:
                _IMM8(d8); ws=_z80m_alu8(y,ws,d8);
                break;
            /* RST */
            case 0xC7: case 0xCF: case 0xD7: case 0xDF: case 0xE7: case 0xEF: case 0xF7: case 0xFF:
                {
                    uint16_t sp = _G16(r1,_SP);
                    _MW(--sp, pc>>8);
                    _MW(--sp, pc);
                    _S16(r1,_SP,sp);
                    pc = y * 8;
                    _SWZ(pc);
                }
                break;
            /* ED prefix */
            case 0xED:
                {
                    _FETCH(op);
                    const uint8_t x = op>>6;
                    const uint8_t y = (op>>3)&7;
                    const uint8_t z = op&7;
                    const uint8_t p = y>>1;
                    const uint8_t q = y&1;
                    const int rp = (3-p)<<4;
                    if (x == 2) {
                        /* block instructions (LDIR, etc...) */
                        if (y >= 4) {
                            switch (z) {
                                case 0: /* LDI, LDD, LDIR, LDDR */
                                    {
                                        uint16_t hl = _G16(ws,_HL);
                                        uint16_t de = _G16(ws,_DE);
                                        _MR(hl,d8);
                                        _MW(de,d8);
                                        if (y & 1) { hl--; de--; }
                                        else       { hl++; de++; }
                                        _S16(ws,_HL,hl);
                                        _S16(ws,_DE,de);
                                        _T(2);
                                        d8 += _G8(ws,_A);
                                        uint8_t f = _G8(ws,_F) & (Z80M_SF|Z80M_ZF|Z80M_CF);
                                        if (d8 & 0x02) { f |= Z80M_YF; }
                                        if (d8 & 0x08) { f |= Z80M_XF; }
                                        uint16_t bc = _G16(ws,_BC);
                                        bc--;
                                        _S16(ws,_BC,bc);
                                        if (bc) { f |= Z80M_VF; }
                                        _S8(ws,_F,f);
                                        if (y >= 6) {
                                            /* LDIR/LDDR */
                                            if (bc) {
                                                pc -= 2;
                                                _SWZ(pc+1);
                                                _T(5);
                                            }
                                        }
                                    }
                                    break;
                                case 1: /* CPI, CPD, CPIR, CPDR */
                                    {
                                        uint16_t hl = _G16(ws,_HL);
                                        _MR(hl,d8);
                                        uint16_t wz = _GWZ();
                                        if (y & 1) { hl--; wz--; }
                                        else       { hl++; wz++; }
                                        _SWZ(wz);
                                        _S16(ws,_HL,hl);
                                        _T(5);
                                        int r = ((int)_G8(ws,_A)) - d8;
                                        uint8_t f = (_G8(ws,_F) & Z80M_CF) | Z80M_NF | _z80m_sz(r);
                                        if ((r & 0x0F) > (_G8(ws,_A) & 0x0F)) {
                                            f |= Z80M_HF;
                                            r--;
                                        }
                                        if (r & 0x02) { f |= Z80M_YF; }
                                        if (r & 0x08) { f |= Z80M_XF; }
                                        uint16_t bc = _G16(ws,_BC);
                                        bc--;
                                        _S16(ws,_BC,bc);
                                        if (bc) { f |= Z80M_VF; }
                                        _S8(ws,_F,f);
                                        if (y >= 6) {
                                            /* CPIR/CPDR */
                                            if (bc && !(f & Z80M_ZF)) {
                                                pc -= 2;
                                                _SWZ(pc+1);
                                                _T(5);
                                            }
                                        }
                                    }
                                    break;
                                case 2: /* INI, IND, INIR, INDR */
                                    {
                                        _T(1);
                                        addr = _G16(ws,_BC);
                                        uint16_t hl = _G16(ws,_HL);
                                        _IN(addr,d8);
                                        _MW(hl,d8);
                                        uint8_t b = _G8(ws,_B);
                                        uint8_t c = _G8(ws,_C);
                                        b--;
                                        if (y & 1) { addr--; hl--; c--; }
                                        else       { addr++; hl++; c++; }
                                        _S8(ws,_B,b);
                                        _S16(ws,_HL,hl);
                                        _SWZ(addr);
                                        uint8_t f = b ? (b & Z80M_SF) : Z80M_ZF;
                                        if (d8 & Z80M_SF) { f |= Z80M_NF; }
                                        uint32_t t = (uint32_t) (c & 0xFF) + d8;
                                        if (t & 0x100) { f |= Z80M_HF|Z80M_CF; }
                                        f |= _z80m_szp[((uint8_t)(t & 0x07)) ^ b] & Z80M_PF;
                                        _S8(ws,_F,f);
                                        if (y >= 6) {
                                            /* INIR,INDR */
                                            if (b) {
                                                pc -= 2;
                                                _T(5);
                                            }
                                        }
                                    }
                                    break;
                                case 3: /* OUTI, OUTD, OTIR, OTDR */
                                    {
                                        _T(1);
                                        uint16_t hl = _G16(ws,_HL);
                                        _MR(hl,d8);
                                        uint8_t b = _G8(ws,_B);
                                        b--;
                                        _S8(ws,_B,b);
                                        addr = _G16(ws,_BC);
                                        _OUT(addr,d8);
                                        if (y & 1) { addr--; hl--; }
                                        else       { addr++; hl++; }
                                        _S16(ws,_HL,hl);
                                        _SWZ(addr);
                                        uint8_t f = b ? (b & Z80M_SF) : Z80M_ZF;
                                        if (d8 & Z80M_SF) { f |= Z80M_NF; }
                                        uint32_t t = (uint32_t)_G8(ws,_L) + (uint32_t)d8;
                                        if (t & 0x0100) { f |= Z80M_HF|Z80M_CF; }
                                        f |= _z80m_szp[((uint8_t)(t & 0x07)) ^ b] & Z80M_PF;
                                        _S8(ws,_F,f);
                                        if (y >= 6) {
                                            /* INIR,INDR */
                                            if (b) {
                                                pc -= 2;
                                                _T(5);
                                            }
                                        }
                                    }
                                    break;
                            }
                        }
                    }
                    else if (x == 1) {
                        const int ry = (7-y)<<3;
                        /* misc ED ops */
                        switch (z) {
                            case 0: /* IN r,(C) */
                                {
                                    addr = _G16(ws,_BC);
                                    _IN(addr++,d8);
                                    _SWZ(addr);
                                    uint8_t f = (_G8(ws,_F) & Z80M_CF) | _z80m_szp[d8];
                                    _S8(ws,_F,f);
                                    /* handle undocumented special case IN F,(C): 
                                        only set flags, don't store result
                                    */
                                    if (ry != _F) {
                                        _S8(ws,ry,d8);
                                    }
                                }
                                break;
                            case 1: /* OUT (C),r */
                                addr = _G16(ws,_BC);
                                d8 = (ry == _F) ? 0 : _G8(ws,ry);
                                _OUT(addr++,d8);
                                _SWZ(addr);
                                break;
                            case 2: /* SBC/ADC HL,rr */
                                {
                                    uint16_t acc = _G16(ws,_HL);
                                    _SWZ(acc+1);
                                    if (_FA==rp) { d16 = _G16(r1,_SP); }  /* ADD HL,SP */
                                    else         { d16 = _G16(ws,rp); }   /* ADD HL,dd */
                                    uint32_t r;
                                    uint8_t f;
                                    if (q == 0) {
                                        /* SBC HL,rr */
                                        r = acc - d16 - (_G8(ws,_F) & Z80M_CF);
                                        f = Z80M_NF | (((d16^acc)&(acc^r)&0x8000)>>13);
                                    }
                                    else {
                                        /* ADC HL,rr */
                                        r = acc + d16 + (_G8(ws,_F) & Z80M_CF);
                                        f = ((d16^acc^0x8000)&(d16^r)&0x8000)>>13;
                                    }
                                    _S16(ws,_HL,r);
                                    f |= ((acc^r^d16)>>8) & Z80M_HF;
                                    f |= (r>>16) & Z80M_CF;
                                    f |= (r>>8) & (Z80M_SF|Z80M_YF|Z80M_XF);
                                    f |= (r & 0xFFFF) ? 0 : Z80M_ZF;
                                    _S8(ws,_F,f);
                                    _T(7);
                                }
                                break;
                            case 3: /* LD (nn),rr; LD rr,(nn) */
                                _IMM16(addr);
                                if (q == 0) { /* LD (nn),rr */
                                    if (rp == _FA) { d16=_G16(r1, _SP); }
                                    else           { d16=_G16(ws, rp);  }
                                    _MW(addr++, d16 & 0xFF);
                                    _MW(addr, d16 >> 8);
                                    _SWZ(addr);
                                }
                                else {  /* LD rr,(nn) */
                                    uint8_t l,h; _MR(addr++,l); _MR(addr,h); d16 = (h<<8)|l;
                                    if (rp == _FA) { _S16(r1, _SP, d16); }
                                    else           { _S16(ws, rp, d16); }
                                    _SWZ(addr);
                                }
                                break;
                            case 4: /* NEG */
                                ws = _z80m_neg8(ws);
                                break;
                            case 5: /* RETN, RETI */
                                assert(false);
                                break;
                            case 6: /* IM */
                                d8 = ((y&3) == 0) ? 0 : (y&3)-1;
                                _S8(r2,_IM,d8);
                                break;
                            case 7: /* misc ops with R,I,A */
                                switch (y) {
                                    case 0: /* LD I,A */
                                        _T(1); d8=_G8(ws,_A); _S8(r2,_I,d8);
                                        break;
                                    case 1: /* LD R,A */
                                        _T(1); d8=_G8(ws,_A); _S8(r2,_R,d8);
                                        break;
                                    case 2: /* LD A,I */
                                        _T(1); d8=_G8(r2,_I); _S8(ws,_A,d8);
                                        _S8(ws,_F,_z80m_sziff2_flags(ws, r2, d8));
                                        break;
                                    case 3: /* LD A,R */
                                        _T(1); d8=_G8(r2,_R); _S8(ws,_A,d8);
                                        _S8(ws,_F,_z80m_sziff2_flags(ws, r2, d8));
                                        break;
                                    case 4: { /* RRD */
                                            addr = _G16(ws,_HL);
                                            uint8_t a = _G8(ws,_A);
                                            _MR(addr, d8);
                                            uint8_t l = a & 0x0F;
                                            a = (a & 0xF0) | (d8 & 0x0F);
                                            _S8(ws,_A,a);
                                            d8 = (d8>>4) | (l<<4);
                                            _MW(addr++,d8);
                                            _SWZ(addr);
                                            _S8(ws,_F,(_G8(ws,_F) & Z80M_CF) | _z80m_szp[a]);
                                            _T(4);
                                        }
                                        break;
                                    case 5: {  /* RLD */
                                            addr = _G16(ws,_HL);
                                            uint8_t a = _G8(ws,_A);
                                            _MR(addr, d8);
                                            uint8_t l = a & 0x0F;
                                            a = (a & 0xF0) | (d8>>4);
                                            _S8(ws,_A,a);
                                            d8 = (d8<<4) | l;
                                            _MW(addr++,d8);
                                            _SWZ(addr);
                                            _S8(ws,_F,(_G8(ws,_F) & Z80M_CF) | _z80m_szp[a]);
                                            _T(4);
                                        }
                                        break;
                                    default: /* 8-cycle NOP */
                                        break;
                                }
                                break;
                        }
                    }
                    else {
                        /* everything else is a 8-cycle NOP */
                    }
                }
                break;
        }
        /* check for and handle interrupt */
        if (((pins & (Z80M_INT|Z80M_BUSREQ))==Z80M_INT) && (r2 & _BIT_IFF1)) {
            r2 &= ~(_BIT_IFF1|_BIT_IFF2);
            if (pins & Z80M_HALT) {
                pins &= ~Z80M_HALT;
                pc++;
            }
            _ON(Z80M_M1|Z80M_IORQ);
            _SA(pc);
            _TW(4);
            const uint8_t int_vec = _GD();
            _OFF(Z80M_M1|Z80M_IORQ);
            _BUMPR();
            _T(2);
            uint16_t sp = _G16(r1,_SP);
            _MW(--sp,pc>>8);
            _MW(--sp,pc);
            _S16(r1,_SP,sp);
            switch (_G8(r2,_IM)) {
                case 0:
                    /* IM 0 not supported */
                    break;
                case 1:
                    pc = 0x0038;
                    break;
                case 2:
                    {
                        addr = _G8(r1,_I) | (int_vec & 0xFE);
                        uint8_t z,w;
                        _MR(addr++,z);
                        _MR(addr,w);
                        pc = (w<<8)|z;
                    }
                    break;
            }
            _SWZ(pc);
        }
        map_bits &= ~(_BIT_USE_IX|_BIT_USE_IY);
        /* check traps */
        if (trap_addr != 0xFFFFFFFFFFFFFFFF) {
            uint64_t ta = trap_addr;
            for (int i = 0; i < Z80M_MAX_NUM_TRAPS; i++) {
                ta >>= 16;
                if (((ta & 0xFFFF) == pc) && (pc != 0xFFFF)) {
                    trap_id = i;
                    break;
                }
            }
        }
    } while ((ticks < num_ticks) && (trap_id < 0));
    _S16(r2,_PC,pc);
    {
        uint64_t old_map_bits = r2 & _BITS_MAP_REGS;
        r0 = _z80m_flush_r0(ws, r0, old_map_bits);
        r1 = _z80m_flush_r1(ws, r1, old_map_bits);
    }
    r2 = (r2 & ~_BITS_MAP_REGS) | map_bits;
    cpu->bc_de_hl_fa = r0;
    cpu->wz_ix_iy_sp = r1;
    cpu->im_ir_pc_bits = r2;
    cpu->bc_de_hl_fa_ = r3;
    cpu->pins = pins;
    cpu->trap_id = trap_id;
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
#undef _BITS_MAP_REGS
#undef _S8
#undef _G8
#undef _S16
#undef _G16
#undef _S1
#undef _SA
#undef _SAD
#undef _GD
#undef _ON
#undef _OFF
#undef _T
#undef _TW
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

#endif /* CHIPS_IMPL */
