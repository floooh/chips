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
    /* OP,SP,WZ,PC */
    uint64_t ir_sp_wz_pc;
    /* EI-pending,IFF1,IFF2,IM,IY,IX */
    uint64_t eif_im_iy_ix;
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
extern void z80m_set_ei_pending(z80m_t* cpu, bool b);

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
extern bool z80m_ei_pending(z80m_t* cpu);

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
#define _PC (0)
#define _WZ (16)
#define _SP (32)
#define _IR (48)
#define _R  (48)
#define _I  (56)
/* bank 3 */
#define _IX (0)
#define _IY (16)
#define _IM (32)
#define _IFF1 (40)
#define _IFF2 (41)
#define _EI   (42)
#define _BIT_IFF1 (1ULL<<_IFF1)
#define _BIT_IFF2 (1ULL<<_IFF2)
#define _BIT_EI   (1ULL<<_EI)

/* set 8-bit immediate value in 64-bit register bank */
#define _SI8(bank,shift,val)    ((bank&~(0xFFULL<<shift))|((val&0xFFULL)<<shift))
/* extract 8-bit value from 64-bit register bank */
#define _G8(bank,shift)         ((bank>>shift)&0xFFULL)
/* set 16-bit immediate value in 64-bit register bank */
#define _SI16(bank,shift,val)   ((bank&~(0xFFFFUL<<shift))|((val&0xFFFFULL)<<shift))
/* extract 16-bit value from 64-bit register bank */
#define _G16(bank,shift)        ((bank>>shift)&0xFFFFULL)
/* set a single bit value in 64-bit register mask */
#define _SI1(bank,shift,val)    ((bank&~(1ULL<<shift))|((val&1ULL)<<shift))
/* set 16-bit address bus pins */
#define _SA(addr) pins=(pins&~0xFFFFULL)|((addr)&0xFFFFULL)
/* set 16-bit address bus and 8-bit data bus pins */
#define _SAD(addr,data) pins=(pins&~0xFFFFFFULL)|((((data)&0xFFULL)<<16)&0xFF0000ULL)|((addr)&0xFFFFULL)
/* get 8-bit data bus value from pins */
#define _GD() ((uint8_t)((pins&0xFF0000ULL)>>16))

#define _ON(m) pins|=(m)
#define _OFF(m) pins&=~(m)
#define _T(num) pins=tick(num,pins,ud);ticks+=num
#define _TW(num) pins&=~Z80M_WAIT_MASK;pins=tick(num,pins,ud);ticks+=num+Z80M_GET_WAIT(pins);
#define _MR(addr,data) _SA(addr);_ON(Z80M_MREQ|Z80M_RD);_TW(3);_OFF(Z80M_MREQ|Z80M_RD);data=_GD()
#define _MW(addr,data) _SAD(addr,data);_ON(Z80M_MREQ|Z80M_WR);_TW(3);_OFF(Z80M_MREQ|Z80M_WR)
#define _IN(addr,data) _SA(addr);_ON(Z80M_IORQ|Z80M_RD);_TW(4);_OFF(Z80M_IORQ|Z80M_RD);data=_GD()
#define _OUT(addr,data) _SAD(addr,data);_ON(Z80M_IORQ|Z80M_WR);_TW(4);_OFF(Z80M_IORQ|Z80M_WR)

/* register access functions */
void z80m_set_a(z80m_t* cpu, uint8_t v)         { cpu->bc_de_hl_fa = _SI8(cpu->bc_de_hl_fa,_A,v); }
void z80m_set_f(z80m_t* cpu, uint8_t v)         { cpu->bc_de_hl_fa = _SI8(cpu->bc_de_hl_fa,_F,v); }
void z80m_set_l(z80m_t* cpu, uint8_t v)         { cpu->bc_de_hl_fa = _SI8(cpu->bc_de_hl_fa,_L,v); }
void z80m_set_h(z80m_t* cpu, uint8_t v)         { cpu->bc_de_hl_fa = _SI8(cpu->bc_de_hl_fa,_H,v); }
void z80m_set_e(z80m_t* cpu, uint8_t v)         { cpu->bc_de_hl_fa = _SI8(cpu->bc_de_hl_fa,_E,v); }
void z80m_set_d(z80m_t* cpu, uint8_t v)         { cpu->bc_de_hl_fa = _SI8(cpu->bc_de_hl_fa,_D,v); }
void z80m_set_c(z80m_t* cpu, uint8_t v)         { cpu->bc_de_hl_fa = _SI8(cpu->bc_de_hl_fa,_C,v); }
void z80m_set_b(z80m_t* cpu, uint8_t v)         { cpu->bc_de_hl_fa = _SI8(cpu->bc_de_hl_fa,_B,v); }
void z80m_set_fa(z80m_t* cpu, uint16_t v)       { cpu->bc_de_hl_fa = _SI16(cpu->bc_de_hl_fa,_FA,v); }
void z80m_set_hl(z80m_t* cpu, uint16_t v)       { cpu->bc_de_hl_fa = _SI16(cpu->bc_de_hl_fa,_HL,v); }
void z80m_set_de(z80m_t* cpu, uint16_t v)       { cpu->bc_de_hl_fa = _SI16(cpu->bc_de_hl_fa,_DE,v); }
void z80m_set_bc(z80m_t* cpu, uint16_t v)       { cpu->bc_de_hl_fa = _SI16(cpu->bc_de_hl_fa,_BC,v); }
void z80m_set_fa_(z80m_t* cpu, uint16_t v)      { cpu->bc_de_hl_fa_ = _SI16(cpu->bc_de_hl_fa_,_FA,v); }
void z80m_set_hl_(z80m_t* cpu, uint16_t v)      { cpu->bc_de_hl_fa_ = _SI16(cpu->bc_de_hl_fa_,_HL,v); }
void z80m_set_de_(z80m_t* cpu, uint16_t v)      { cpu->bc_de_hl_fa_ = _SI16(cpu->bc_de_hl_fa_,_DE,v); }
void z80m_set_bc_(z80m_t* cpu, uint16_t v)      { cpu->bc_de_hl_fa_ = _SI16(cpu->bc_de_hl_fa_,_BC,v); }
void z80m_set_pc(z80m_t* cpu, uint16_t v)       { cpu->ir_sp_wz_pc = _SI16(cpu->ir_sp_wz_pc,_PC,v); }
void z80m_set_wz(z80m_t* cpu, uint16_t v)       { cpu->ir_sp_wz_pc = _SI16(cpu->ir_sp_wz_pc,_WZ,v); }
void z80m_set_sp(z80m_t* cpu, uint16_t v)       { cpu->ir_sp_wz_pc = _SI16(cpu->ir_sp_wz_pc,_SP,v); }
void z80m_set_ir(z80m_t* cpu, uint16_t v)       { cpu->ir_sp_wz_pc = _SI16(cpu->ir_sp_wz_pc,_IR,v); }
void z80m_set_i(z80m_t* cpu, uint8_t v)         { cpu->ir_sp_wz_pc = _SI8(cpu->ir_sp_wz_pc,_I,v); }
void z80m_set_r(z80m_t* cpu, uint8_t v)         { cpu->ir_sp_wz_pc = _SI8(cpu->ir_sp_wz_pc,_R,v); }
void z80m_set_ix(z80m_t* cpu, uint16_t v)       { cpu->eif_im_iy_ix = _SI16(cpu->eif_im_iy_ix,_IX,v); }
void z80m_set_iy(z80m_t* cpu, uint16_t v)       { cpu->eif_im_iy_ix = _SI16(cpu->eif_im_iy_ix,_IY,v); }
void z80m_set_im(z80m_t* cpu, uint8_t v)        { cpu->eif_im_iy_ix = _SI8(cpu->eif_im_iy_ix,_IM,v); }
void z80m_set_iff1(z80m_t* cpu, bool b)         { cpu->eif_im_iy_ix = _SI1(cpu->eif_im_iy_ix,_IFF1,(b?1:0)); }
void z80m_set_iff2(z80m_t* cpu, bool b)         { cpu->eif_im_iy_ix = _SI1(cpu->eif_im_iy_ix,_IFF2,(b?1:0)); }
void z80m_set_ei_pending(z80m_t* cpu, bool b)   { cpu->eif_im_iy_ix = _SI1(cpu->eif_im_iy_ix,_EI,(b?1:0)); }
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
uint16_t z80m_pc(z80m_t* cpu)       { return _G16(cpu->ir_sp_wz_pc,_PC); }
uint16_t z80m_wz(z80m_t* cpu)       { return _G16(cpu->ir_sp_wz_pc,_WZ); }
uint16_t z80m_sp(z80m_t* cpu)       { return _G16(cpu->ir_sp_wz_pc,_SP); }
uint16_t z80m_ir(z80m_t* cpu)       { return _G16(cpu->ir_sp_wz_pc,_IR); }
uint8_t z80m_i(z80m_t* cpu)         { return _G8(cpu->ir_sp_wz_pc,_I); }
uint8_t z80m_r(z80m_t* cpu)         { return _G8(cpu->ir_sp_wz_pc,_R); }
uint16_t z80m_ix(z80m_t* cpu)       { return _G16(cpu->eif_im_iy_ix,_IX); }
uint16_t z80m_iy(z80m_t* cpu)       { return _G16(cpu->eif_im_iy_ix,_IY); }
uint8_t z80m_im(z80m_t* cpu)        { return _G8(cpu->eif_im_iy_ix,_IM); }
bool z80m_iff1(z80m_t* cpu)         { return 0 != (cpu->eif_im_iy_ix & _BIT_IFF1); }
bool z80m_iff2(z80m_t* cpu)         { return 0 != (cpu->eif_im_iy_ix & _BIT_IFF2); }
bool z80m_ei_pending(z80m_t* cpu)   { return 0 != (cpu->eif_im_iy_ix & _BIT_EI); }

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
    z80m_set_i(cpu, 0x00);
    z80m_set_r(cpu, 0x00);
    z80m_set_ei_pending(cpu, false);
}

/* instruction decoder */
uint32_t z80m_exec(z80m_t* cpu, uint32_t num_ticks) {
    uint64_t r0 = cpu->bc_de_hl_fa;
    uint64_t r1 = cpu->ir_sp_wz_pc;
    uint64_t r2 = cpu->eif_im_iy_ix;
    uint64_t pins = cpu->pins;
    const z80m_tick_t tick = cpu->tick;
    void* ud = cpu->user_data;
    int trap_id = -1;
    uint32_t ticks = 0;
    do {
        /* switch off interrupt flag */
        _OFF(Z80M_INT);
        /* delay-enable interrupt flags */
        if (r2 & _BIT_EI) {
            r2 &= ~_BIT_EI;
            r2 |= (_BIT_IFF1 | _BIT_IFF2);
        }
        /* fetch opcode (4 cycles) */
        uint8_t op;
        {
            _ON(Z80M_M1|Z80M_MREQ|Z80M_RD);
            uint16_t pc = _G16(r1,_PC);
            _SA(pc++);
            r1 = _SI16(r1,_PC,pc);
            _TW(4);
            _OFF(Z80M_M1|Z80M_MREQ|Z80M_RD);
            op = _GD();
            uint8_t r = _G8(r1,_R);
            r = (r&0x80)|((r+1)&0x7F);
            r1 = _SI8(r1,_R,r);
        }
    } while ((ticks < num_ticks) && (trap_id < 0));
    cpu->bc_de_hl_fa = r0;
    cpu->ir_sp_wz_pc = r1;
    cpu->eif_im_iy_ix = r2;
    cpu->pins = pins;
    cpu->trap_id = trap_id;
    return ticks;
}

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
