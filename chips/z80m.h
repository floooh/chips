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

#define Z80M_MAX_NUM_TRAPS (8)

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
    uint64_t ir_pc_wz_sp;
    /* control bits,IM,IY,IX */
    uint64_t bits_im_iy_ix;
    /* last pin state (only for debug inspection) */
    uint64_t pins;
    void* user_data;
    int trap_id;
    bool trap_valid[Z80M_MAX_NUM_TRAPS];
    uint16_t trap_addr[Z80M_MAX_NUM_TRAPS];
    z80m_trapfunc_t trap_func[Z80M_MAX_NUM_TRAPS];
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
/* bank 2 */
#define _SP (0)
#define _WZ (16)
#define _PC (32)
#define _IR (48)
#define _R  (48)
#define _I  (56)
/* bank 3 */
#define _IX (16)
#define _IY (32)
#define _IM (48)
#define _IFF1 (0)
#define _IFF2 (1)
#define _EI   (2)
#define _USE_IX (3)
#define _USE_IY (4)
#define _BIT_IFF1   (1ULL<<_IFF1)
#define _BIT_IFF2   (1ULL<<_IFF2)
#define _BIT_EI     (1ULL<<_EI)
#define _BIT_USE_IX (1ULL<<_USE_IX)
#define _BIT_USE_IY (1ULL<<_USE_IY)
#define _BITS_MAP_REGS (_BIT_USE_IX|_BIT_USE_IY)

/* set 8-bit immediate value in 64-bit register bank */
#define _S8(bank,shift,val)    bank=(((bank)&~(0xFFULL<<(shift)))|(((val)&0xFFULL)<<(shift)))
/* extract 8-bit value from 64-bit register bank */
#define _G8(bank,shift)         (((bank)>>(shift))&0xFFULL)
/* set 16-bit immediate value in 64-bit register bank */
#define _S16(bank,shift,val)   bank=((bank&~(0xFFFFULL<<(shift)))|(((val)&0xFFFFULL)<<(shift)))
/* extract 16-bit value from 64-bit register bank */
#define _G16(bank,shift)        (((bank)>>(shift))&0xFFFFULL)
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
#define _IMM16(data) {uint8_t w,z;_MR(pc++,z);_MR(pc++,w);data=(w<<8)|z;_S16(r1,_WZ,data);} 
/* generate effective address for (HL), (IX+d), (IY+d) */
#define _ADDR(addr,ext_ticks) {addr=_G16(ws,_HL);if (r2&(_BIT_USE_IX|_BIT_USE_IY)){int8_t d;_MR(pc++,d);addr+=d;_S16(r1,_WZ,addr);_T(ext_ticks);}}
/* a normal opcode fetch, bump R */
#define _FETCH(op) {_SA(pc++);_ON(Z80M_M1|Z80M_MREQ|Z80M_RD);_TW(4);_OFF(Z80M_M1|Z80M_MREQ|Z80M_RD);op=_GD();d8=_G8(r1,_R);d8=(d8&0x80)|((d8+1)&0x7F);_S8(r1,_R,d8);}

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
void z80m_set_pc(z80m_t* cpu, uint16_t v)       { _S16(cpu->ir_pc_wz_sp,_PC,v); }
void z80m_set_wz(z80m_t* cpu, uint16_t v)       { _S16(cpu->ir_pc_wz_sp,_WZ,v); }
void z80m_set_sp(z80m_t* cpu, uint16_t v)       { _S16(cpu->ir_pc_wz_sp,_SP,v); }
void z80m_set_ir(z80m_t* cpu, uint16_t v)       { _S16(cpu->ir_pc_wz_sp,_IR,v); }
void z80m_set_i(z80m_t* cpu, uint8_t v)         { _S8(cpu->ir_pc_wz_sp,_I,v); }
void z80m_set_r(z80m_t* cpu, uint8_t v)         { _S8(cpu->ir_pc_wz_sp,_R,v); }
void z80m_set_ix(z80m_t* cpu, uint16_t v)       { _S16(cpu->bits_im_iy_ix,_IX,v); }
void z80m_set_iy(z80m_t* cpu, uint16_t v)       { _S16(cpu->bits_im_iy_ix,_IY,v); }
void z80m_set_im(z80m_t* cpu, uint8_t v)        { _S8(cpu->bits_im_iy_ix,_IM,v); }
void z80m_set_iff1(z80m_t* cpu, bool b)         { _S1(cpu->bits_im_iy_ix,_IFF1,(b?1:0)); }
void z80m_set_iff2(z80m_t* cpu, bool b)         { _S1(cpu->bits_im_iy_ix,_IFF2,(b?1:0)); }
void z80m_set_ei_pending(z80m_t* cpu, bool b)   { _S1(cpu->bits_im_iy_ix,_EI,(b?1:0)); }
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
uint16_t z80m_pc(z80m_t* cpu)       { return _G16(cpu->ir_pc_wz_sp,_PC); }
uint16_t z80m_wz(z80m_t* cpu)       { return _G16(cpu->ir_pc_wz_sp,_WZ); }
uint16_t z80m_sp(z80m_t* cpu)       { return _G16(cpu->ir_pc_wz_sp,_SP); }
uint16_t z80m_ir(z80m_t* cpu)       { return _G16(cpu->ir_pc_wz_sp,_IR); }
uint8_t z80m_i(z80m_t* cpu)         { return _G8(cpu->ir_pc_wz_sp,_I); }
uint8_t z80m_r(z80m_t* cpu)         { return _G8(cpu->ir_pc_wz_sp,_R); }
uint16_t z80m_ix(z80m_t* cpu)       { return _G16(cpu->bits_im_iy_ix,_IX); }
uint16_t z80m_iy(z80m_t* cpu)       { return _G16(cpu->bits_im_iy_ix,_IY); }
uint8_t z80m_im(z80m_t* cpu)        { return _G8(cpu->bits_im_iy_ix,_IM); }
bool z80m_iff1(z80m_t* cpu)         { return 0 != (cpu->bits_im_iy_ix & _BIT_IFF1); }
bool z80m_iff2(z80m_t* cpu)         { return 0 != (cpu->bits_im_iy_ix & _BIT_IFF2); }
bool z80m_ei_pending(z80m_t* cpu)   { return 0 != (cpu->bits_im_iy_ix & _BIT_EI); }

void z80m_init(z80m_t* cpu, z80m_desc_t* desc) {
    CHIPS_ASSERT(cpu && desc);
    CHIPS_ASSERT(desc->tick_cb);
    memset(cpu, 0, sizeof(*cpu));
    z80m_reset(cpu);
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
    cpu->bits_im_iy_ix &= ~(_BIT_EI|_BIT_USE_IX|_BIT_USE_IY);
}

bool z80m_opdone(z80m_t* cpu) {
    return 0 == (cpu->bits_im_iy_ix & (_BIT_USE_IX|_BIT_USE_IY));
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
static inline uint64_t _z80m_add8(uint64_t bank, uint8_t val) {
    uint8_t acc = _G8(bank,_A);
    uint32_t res = acc + val;
    _S8(bank,_F,_z80m_add_flags(acc,val,res));
    _S8(bank,_A,res);
    return bank;
}

static inline uint64_t _z80m_adc8(uint64_t bank, uint8_t val) {
    uint8_t acc = _G8(bank,_A);
    uint32_t res = acc + val + (_G8(bank,_F) & Z80M_CF);
    _S8(bank,_F,_z80m_add_flags(acc,val,res));
    _S8(bank,_A,res);
    return bank;
}

static inline uint64_t _z80m_sub8(uint64_t bank, uint8_t val) {
    uint8_t acc = _G8(bank,_A);
    uint32_t res = (uint32_t) ((int)acc - (int)val);
    _S8(bank,_F,_z80m_sub_flags(acc,val,res));
    _S8(bank,_A,res);
    return bank;
}

static inline uint64_t _z80m_sbc8(uint64_t bank, uint8_t val) {
    uint8_t acc = _G8(bank,_A);
    uint32_t res = (uint32_t) ((int)acc - (int)val - (_G8(bank,_F) & Z80M_CF));
    _S8(bank,_F,_z80m_sub_flags(acc,val,res));
    _S8(bank,_A,res);
    return bank;
}

static inline uint64_t _z80m_and8(uint64_t bank, uint8_t val) {
    val &= _G8(bank,_A);
    _S8(bank,_F,_z80m_szp[val]|Z80M_HF);
    _S8(bank,_A,val);
    return bank;
}

static inline uint64_t _z80m_xor8(uint64_t bank, uint8_t val) {
    val ^= _G8(bank,_A);
    _S8(bank,_F,_z80m_szp[val]);
    _S8(bank,_A,val);
    return bank;
}

static inline uint64_t _z80m_or8(uint64_t bank, uint8_t val) {
    val |= _G8(bank,_A);
    _S8(bank,_F,_z80m_szp[val]);
    _S8(bank,_A,val);
    return bank;
}

static inline uint64_t _z80m_cp8(uint64_t bank, uint8_t val) {
    uint8_t acc = _G8(bank,_A);
    uint32_t res = (uint32_t) ((int)acc - (int)val);
    _S8(bank,_F,_z80m_cp_flags(acc,val,res));
    return bank;
}

static inline uint64_t _z80m_alu8(uint8_t type, uint64_t bank, uint8_t val) {
    switch (type) {
        case 0: return _z80m_add8(bank,val); break;
        case 1: return _z80m_adc8(bank,val); break;
        case 2: return _z80m_sub8(bank,val); break;
        case 3: return _z80m_sbc8(bank,val); break;
        case 4: return _z80m_and8(bank,val); break;
        case 5: return _z80m_xor8(bank,val); break;
        case 6: return _z80m_or8(bank,val); break;
        case 7: return _z80m_cp8(bank,val); break;
    }
    /* can't happen */
    return bank;
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

/* manage the virtual 'working set' register bank */
static inline uint64_t _z80m_map_regs(uint64_t r0, uint64_t r2) {
    uint64_t ws = r0;
    if (r2 & _BIT_USE_IX) {
        ws = (ws & ~(0xFFFFULL<<_HL)) | (((r2>>_IX) & 0xFFFF)<<_HL);
    }
    else if (r2 & _BIT_USE_IY) {
        ws = (ws & ~(0xFFFFULL<<_HL)) | (((r2>>_IY) & 0xFFFF)<<_HL);
    }
    return ws;
}

static inline uint64_t _z80m_flush_r0(uint64_t ws, uint64_t r0, uint64_t map_bits) {
    r0 = (r0 & (0xFFFFULL<<_HL)) | (ws & ~(0xFFFFULL<<_HL));
    if (!(map_bits & (_BIT_USE_IX|_BIT_USE_IY))) {
        r0 = (r0 & ~(0xFFFFULL<<_HL)) | (ws & (0xFFFFULL<<_HL));
    }
    return r0;
}

static inline uint64_t _z80m_flush_r2(uint64_t ws, uint64_t r2, uint64_t map_bits) {
    if (map_bits & _BIT_USE_IX) {
        r2 = (r2 & ~(0xFFFFULL<<_IX)) | (((ws>>_HL) & 0xFFFFULL)<<_IX);
    }
    else if (map_bits & _BIT_USE_IY) {
        r2 = (r2 & ~(0xFFFFULL<<_IY)) | (((ws>>_HL) & 0xFFFFULL)<<_IY);
    }
    return r2;
}

/* instruction decoder */
uint32_t z80m_exec(z80m_t* cpu, uint32_t num_ticks) {
    uint64_t r0 = cpu->bc_de_hl_fa;
    uint64_t r1 = cpu->ir_pc_wz_sp;
    uint64_t r2 = cpu->bits_im_iy_ix;
    uint64_t r3 = cpu->bc_de_hl_fa_;
    uint64_t ws = _z80m_map_regs(r0, r2);
    uint64_t map_bits = r2 & _BITS_MAP_REGS;
    uint64_t pins = cpu->pins;
    const z80m_tick_t tick = cpu->tick;
    void* ud = cpu->user_data;
    int trap_id = -1;
    uint32_t ticks = 0;
    uint8_t op, d8;
    uint16_t addr, d16;
    uint16_t pc = _G16(r1,_PC);
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
            r2 = _z80m_flush_r2(ws, r2, old_map_bits);
            r2 = (r2 & ~_BITS_MAP_REGS) | map_bits; 
            ws = _z80m_map_regs(r0, r2);
        }

        /* split opcode into bitgroups
            |xx|yyy|zzz|
            |xx|pp|q|zzz|
        */
        const uint8_t x = op>>6;
        const uint8_t y = (op>>3)&7;
        const uint8_t z = op&7;
        const uint8_t p = y>>1;
        const uint8_t q = y&1;
        const int ry = (7-y)<<3;
        const int rz = (7-z)<<3;
        const int rp = (3-p)<<4;

        /*=== BLOCK 1: 8-bit loads and HALT ==================================*/
        if (x == 1) {
            if (y == 6) {
                if (z == 6) {
                    /* special case; HALT */
                    _ON(Z80M_HALT);
                    pc--;
                }
                else  {
                    /* LD (HL),r; LD (IX+d),r; LD (IY+d),r */
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
                }
            }
            else {
                if (z == 6) {
                    /* LD r,(HL); LD r,(IX+d); LD r,(IY+d) */
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
                }
                else { 
                    /* LD r,s */
                    d8=_G8(ws,rz);
                    _S8(ws,ry,d8);
                }
            }
        }
        /*=== BLOCK 2: 8-bit ALU instructions ================================*/
        else if (x == 2) {
            if (z == 6) { _ADDR(addr,5); _MR(addr,d8); } /* ALU (HL); ALU (IX+d); ALU (IY+d) */
            else        { d8 = _G8(ws,rz); } /* ALU r */
            ws = _z80m_alu8(y,ws,d8);
        }
        /*=== BLOCK 0: misc instructions =====================================*/
        else if (x == 0) {
            switch (z) {
                case 0:
                    switch (y) {
                        case 0:     /* NOP */
                            break;
                        case 1:     /* EX AF,AF' */
                            r0 = _z80m_flush_r0(ws, r0, r2);
                            uint16_t fa = _G16(r0,_FA);
                            uint16_t fa_ = _G16(r3, _FA);
                            _S16(r0,_FA,fa_);
                            _S16(r3,_FA,fa);
                            ws = _z80m_map_regs(r0, r2);
                            break;
                        case 2:     /* DJNZ d */
                            assert(false);
                            break;
                        case 3:     /* JR d */
                            assert(false);
                            break;
                        default:     /* JR cc,d */
                            assert(false);
                            break;
                    }
                    break;
                case 1:
                    if (q == 0) {
                        /* 16-bit immediate loads (AF => SP)*/
                        _IMM16(d16);
                        if (_FA==rp) { _S16(r1,_SP,d16); } /* LD SP,nn */
                        else         { _S16(ws,rp,d16); } /* LD HL,nn; LD DE,nn; LD BC,nn */
                    }
                    else {
                        /* ADD HL,rr; ADD IX,rr; ADD IY,rr */
                        assert(false);
                    }
                    break;
                case 2:
                    /* indirect loads */
                    switch (p) { /* get effective address (BC),(DE) or (nn) */
                        case 0:     addr=_G16(ws,_BC); break;
                        case 1:     addr=_G16(ws,_DE); break;
                        default:    _IMM16(addr); break;
                    }
                    if (q == 0) { /* store */
                        if (p == 2) { /* LD (nn),HL; LD (nn),IX; LD (nn),IY, WZ=addr++ */
                            _MW(addr++,_G8(ws,_L)); _MW(addr,_G8(ws,_H)); _S16(r1,_WZ,addr);
                        }
                        else { /* LD (BC),A; LD (DE),A; LD (nn),A; W=A,L=addr++ */
                            d8=_G8(ws,_A); _MW(addr++,d8); _S16(r1,_WZ,((d8<<8)|(addr&0x00FF)));
                        }
                    }
                    else { /* load */
                        if (p == 2) { /* LD HL,(nn); LD IX,(nn); LD IY,(nn) */
                            _MR(addr++,d8); _S8(ws,_L,d8); _MR(addr,d8); _S8(ws,_H,d8); _S16(r1,_WZ,addr);
                        }
                        else {  /* LD A,(BC); LD A,(DE); LD A,(nn); W=addr++ */
                            _MR(addr++,d8); _S8(ws,_A,d8); _S16(r1,_WZ,addr);
                        }
                    }
                    break;
                case 3:
                    /* 16-bit INC/DEC */
                    _T(2);
                    if (rp==_FA) { d16=_G16(r1,_SP); }
                    else         { d16=_G16(ws,rp); }
                    d16 = d16 + (q ? -1 : +1);
                    if (rp==_FA) { _S16(r1,_SP,d16); }
                    else         { _S16(ws,rp,d16); }
                    break;
                case 4: /* 8-bit INC (HL); INC (IX+d); INC (IY+d); INC r */
                case 5: /* 8-bit DEC (HL); DEC (IX+d); DEC (IY+d); DEC r */
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
                case 6:
                    if (y == 6) { _ADDR(addr,2); _IMM8(d8); _MW(addr,d8); } /* LD (HL),n; LD (IX+d),n; LD (IY+d),n */
                    else        { _IMM8(d8); _S8(ws,ry,d8); } /* LD r,n */
                    break;
                case 7:
                    /* misc ops on A and F */
                    switch (y) {
                        case 0: ws=_z80m_rlca(ws); break;
                        case 1: ws=_z80m_rrca(ws); break;
                        case 2: ws=_z80m_rla(ws); break;
                        case 3: ws=_z80m_rra(ws); break;
                        default:
                            assert(false);
                            break;
                    }
                    break;
            }
        }
        /*=== BLOCK 3: misc and extended ops =================================*/
        else {
            switch (z) {
                case 0:
                    /* RET cc */
                    assert(false);
                    break;
                case 1:
                    /* POP + misc */
                    if (q == 0) {
                        /* POP BC,DE,HL,IX,IY */
                        addr = _G16(r1,_SP);
                        uint8_t l,h;
                        _MR(addr++,l); _MR(addr++,h);
                        d16 = (rp==_FA) ? ((l<<8)|h) : ((h<<8)|l);
                        _S16(ws,rp,d16); _S16(r1,_SP,addr);
                    }
                    else switch (p) {
                        case 0: /* RET */
                            assert(false);
                            break;
                        case 1: /* EXX */
                            r0 = _z80m_flush_r0(ws, r0, r2);
                            const uint64_t rx = r3;
                            r3 = (r3 & 0xFFFF) | (r0 & 0xFFFFFFFFFFFF0000);
                            r0 = (r0 & 0xFFFF) | (rx & 0xFFFFFFFFFFFF0000);
                            ws = _z80m_map_regs(r0, r2);
                            break;
                        case 2: /* JP (HL) */
                            assert(false);
                            break;
                        case 3: /* LD SP,HL; LD SP,IX; LD SP,IY */
                            _T(2); _S16(r1,_SP,_G16(ws,_HL));
                            break;
                    }
                    break;
                case 2:
                    /* JP cc,nn */
                    assert(false);
                    break;
                case 3:
                    /* misc ops */
                    switch (y) {
                        case 0:
                            /* JP nn */
                            assert(false);
                            break;
                        case 1:
                            /* CB prefix */
                            assert(false);
                            break;
                        case 2:
                            /* OUT (n),A */
                            assert(false);
                            break;
                        case 3:
                            /* IN A(n) */
                            assert(false);
                            break;
                        case 4:
                            /* EX SP,(HL/IX/IY) */
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
                                _S16(r1,_WZ,d16);
                            }
                            break;
                        case 5:
                            /* EX DE,HL */
                            {
                                r0 = _z80m_flush_r0(ws, r0, r2);
                                uint16_t de = _G16(r0,_DE);
                                uint16_t hl = _G16(r0,_HL);
                                _S16(r0,_DE,hl);
                                _S16(r0,_HL,de);
                                ws = _z80m_map_regs(r0, r2);
                            }
                            break;
                        case 6:
                            /* DI */
                            assert(false);
                            break;
                        case 7:
                            /* EI */
                            assert(false);
                            break;
                    }
                    break;
                case 4:
                    /* CALL cc,nn */
                    assert(false);
                    break;
                case 5:
                    /* PUSH, CALL, DD,ED,FD prefixes */
                    if (q == 0) {
                        /* PUSH BC,DE,HL,IX,IY,AF */
                        _T(1);
                        addr = _G16(r1,_SP);
                        d16 = _G16(ws,rp);
                        if (rp==_FA) {
                            d16 = (d16>>8) | (d16<<8);
                        }
                        _MW(--addr,d16>>8); _MW(--addr,d16);
                        _S16(r1,_SP,addr);
                    }
                    else switch (p) {
                        case 0: assert(false); break;               /* CALL nnnn */
                        case 1: map_bits |= _BIT_USE_IX; continue;  /* DD prefix (map IX into HL slot) */
                        case 2: {   /* ED prefix */
                                _FETCH(op);
                                const uint8_t x = op>>6;
                                const uint8_t y = (op>>3)&7;
                                const uint8_t z = op&7;
                                const uint8_t p = y>>1;
                                const uint8_t q = y&1;
                                const int rp = (3-p)<<4;
                                if (x == 2) {
                                    /* block instructions (LDIR, etc...) */
                                    assert(false);
                                }
                                else if (x == 1) {
                                    /* misc ED ops */
                                    switch (z) {
                                        case 0: /* IN r,(C) */
                                            assert(false);
                                            break;
                                        case 1: /* OUT (C),r */
                                            assert(false);
                                            break;
                                        case 2: /* SBC/ADC HL,rr */
                                            assert(false);
                                            break;
                                        case 3: /* LD (nn),rr; LD rr,(nn) */
                                            _IMM16(addr);
                                            if (q == 0) { /* LD (nn),rr */
                                                if (rp == _FA) { d16=_G16(r1, _SP); }
                                                else           { d16=_G16(ws, rp);  }
                                                _MW(addr++, d16 & 0xFF);
                                                _MW(addr, d16 >> 8);
                                                _S16(r1, _WZ, addr);
                                            }
                                            else {  /* LD rr,(nn) */
                                                uint8_t l,h; _MR(addr++,l); _MR(addr,h); d16 = (h<<8)|l;
                                                if (rp == _FA) { _S16(r1, _SP, d16); }
                                                else           { _S16(ws, rp, d16); }
                                                _S16(r1, _WZ, addr);
                                            }
                                            break;
                                        case 4: /* NEG */
                                            assert(false);
                                            break;
                                        case 5: /* RETN, RETI */
                                            assert(false);
                                            break;
                                        case 6: /* IM */
                                            assert(false);
                                            break;
                                        case 7: /* misc ops with R,I,A */
                                            switch (y) {
                                                case 0: /* LD I,A */
                                                    _T(1); d8=_G8(ws,_A); _S8(r1,_I,d8);
                                                    break;
                                                case 1: /* LD R,A */
                                                    _T(1); d8=_G8(ws,_A); _S8(r1,_R,d8);
                                                    break;
                                                case 2: /* LD A,I */
                                                    _T(1); d8=_G8(r1,_I); _S8(ws,_A,d8);
                                                    _S8(ws,_F,_z80m_sziff2_flags(ws, r2, d8));
                                                    break;
                                                case 3: /* LD A,R */
                                                    _T(1); d8=_G8(r1,_R); _S8(ws,_A,d8);
                                                    _S8(ws,_F,_z80m_sziff2_flags(ws, r2, d8));
                                                    break;
                                                case 4: /* RRD */
                                                    assert(false);
                                                    break;
                                                case 5: /* RLD */
                                                    assert(false);
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
                        case 3: map_bits |= _BIT_USE_IY; continue;  /* FD prefix (map IY into HL slot ) */
                    }
                    break;
                case 6:
                    /* ALU n */
                    _MR(pc++,d8); ws=_z80m_alu8(y,ws,d8);
                    break;
                case 7:
                    /* RST */
                    assert(false);
                    break;
            }
        }
        map_bits &= ~(_BIT_USE_IX|_BIT_USE_IY);
    } while ((ticks < num_ticks) && (trap_id < 0));
    _S16(r1,_PC,pc);
    {
        uint64_t old_map_bits = r2 & _BITS_MAP_REGS;
        r0 = _z80m_flush_r0(ws, r0, old_map_bits);
        r2 = _z80m_flush_r2(ws, r2, old_map_bits);
    }
    r2 = (r2 & ~_BITS_MAP_REGS) | map_bits;
    cpu->bc_de_hl_fa = r0;
    cpu->ir_pc_wz_sp = r1;
    cpu->bits_im_iy_ix = r2;
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
#undef _PC
#undef _WZ
#undef _SP
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
#endif /* CHIPS_IMPL */
