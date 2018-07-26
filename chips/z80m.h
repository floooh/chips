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
#define Z80M_WAIT_MASK (Z80_WAIT0|Z80_WAIT1|Z80_WAIT2)

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

/* register access macros */
#define Z80M_MSK_A      (0x00000000000000FFULL)
#define Z80M_MSK_F      (0x000000000000FF00ULL)
#define Z80M_MSK_L      (0x0000000000FF0000ULL)
#define Z80M_MSK_H      (0x00000000FF000000ULL)
#define Z80M_MSK_E      (0x000000FF00000000ULL)
#define Z80M_MSK_D      (0x0000FF0000000000ULL)
#define Z80M_MSK_C      (0x00FF000000000000ULL)
#define Z80M_MSK_B      (0xFF00000000000000ULL)
#define Z80M_MSK_FA     (0x000000000000FFFFULL)
#define Z80M_MSK_HL     (0x00000000FFFF0000ULL)
#define Z80M_MSK_DE     (0x0000FFFF00000000ULL)
#define Z80M_MSK_BC     (0xFFFF000000000000ULL)
#define Z80M_MSK_PC     (0x000000000000FFFFULL)
#define Z80M_MSK_WZ     (0x00000000FFFF0000ULL)
#define Z80M_MSK_SP     (0x0000FFFF00000000ULL)
#define Z80M_MSK_IR     (0xFFFF000000000000ULL)
#define Z80M_MSK_IX     (0x000000000000FFFFULL)
#define Z80M_MSK_IY     (0x00000000FFFF0000ULL)
#define Z80M_MSK_IM     (0x000000FF00000000ULL)
#define Z80M_MSK_IFF1   (0x0000010000000000ULL)
#define Z80M_MSK_IFF2   (0x0000020000000000ULL)
#define Z80M_MSK_EI     (0x0000040000000000ULL)

#define Z80M_INV_A      (0xFFFFFFFFFFFFFF00ULL)
#define Z80M_INV_F      (0xFFFFFFFFFFFF00FFULL)
#define Z80M_INV_L      (0xFFFFFFFFFF00FFFFULL)
#define Z80M_INV_H      (0xFFFFFFFF00FFFFFFULL)
#define Z80M_INV_E      (0xFFFFFF00FFFFFFFFULL)
#define Z80M_INV_D      (0xFFFF00FFFFFFFFFFULL)
#define Z80M_INV_C      (0xFF00FFFFFFFFFFFFULL)
#define Z80M_INV_B      (0x00FFFFFFFFFFFFFFULL)
#define Z80M_INV_FA     (0xFFFFFFFFFFFF0000ULL)
#define Z80M_INV_HL     (0xFFFFFFFF0000FFFFULL)
#define Z80M_INV_DE     (0xFFFF0000FFFFFFFFULL)
#define Z80M_INV_BC     (0x0000FFFFFFFFFFFFULL)
#define Z80M_INV_PC     (0xFFFFFFFFFFFF0000ULL)
#define Z80M_INV_WZ     (0xFFFFFFFF0000FFFFULL)
#define Z80M_INV_SP     (0xFFFF0000FFFFFFFFULL)
#define Z80M_INV_IR     (0x0000FFFFFFFFFFFFULL)
#define Z80M_INV_IX     (0xFFFFFFFFFFFF0000ULL)
#define Z80M_INV_IY     (0xFFFFFFFF0000FFFFULL)
#define Z80M_INV_IM     (0xFFFFFF00FFFFFFFFULL)
#define Z80M_INV_IFF1   (0xFFFFFEFFFFFFFFFFULL)
#define Z80M_INV_IFF2   (0xFFFFFDFFFFFFFFFFULL)
#define Z80M_INV_EI     (0xFFFFFBFFFFFFFFFFULL)

#define Z80M_SFT_A      (0)
#define Z80M_SFT_F      (8)
#define Z80M_SFT_L      (16)
#define Z80M_SFT_H      (24)
#define Z80M_SFT_E      (32)
#define Z80M_SFT_D      (40)
#define Z80M_SFT_C      (48)
#define Z80M_SFT_B      (56)
#define Z80M_SFT_FA     (0)
#define Z80M_SFT_HL     (16)
#define Z80M_SFT_DE     (32)
#define Z80M_SFT_BC     (48)
#define Z80M_SFT_PC     (0)
#define Z80M_SFT_WZ     (16)
#define Z80M_SFT_SP     (32)
#define Z80M_SFT_IR     (48)
#define Z80M_SFT_IX     (0)
#define Z80M_SFT_IY     (16)
#define Z80M_SFT_IM     (32)
#define Z80M_SFT_IFF1   (40)
#define Z80M_SFT_IFF2   (41)
#define Z80M_SFT_EI     (42)

#define Z80M_SET_A(bank,val)    (((bank)&Z80M_INV_A)|((val&0xFFULL)<<Z80M_SFT_A))
#define Z80M_SET_F(bank,val)    (((bank)&Z80M_INV_F)|((val&0xFFULL)<<Z80M_SFT_F))
#define Z80M_SET_L(bank,val)    (((bank)&Z80M_INV_L)|((val&0xFFULL)<<Z80M_SFT_L))
#define Z80M_SET_H(bank,val)    (((bank)&Z80M_INV_H)|((val&0xFFULL)<<Z80M_SFT_H))
#define Z80M_SET_E(bank,val)    (((bank)&Z80M_INV_E)|((val&0xFFULL)<<Z80M_SFT_E))
#define Z80M_SET_D(bank,val)    (((bank)&Z80M_INV_D)|((val&0xFFULL)<<Z80M_SFT_D))
#define Z80M_SET_C(bank,val)    (((bank)&Z80M_INV_C)|((val&0xFFULL)<<Z80M_SFT_C))
#define Z80M_SET_B(bank,val)    (((bank)&Z80M_INV_B)|((val&0xFFULL)<<Z80M_SFT_B))
#define Z80M_SET_FA(bank,val)   (((bank)&Z80M_INV_FA)|((val&0xFFFFULL)<<Z80M_SFT_FA))
#define Z80M_SET_HL(bank,val)   (((bank)&Z80M_INV_HL)|((val&0xFFFFULL)<<Z80M_SFT_HL))
#define Z80M_SET_DE(bank,val)   (((bank)&Z80M_INV_DE)|((val&0xFFFFULL)<<Z80M_SFT_DE))
#define Z80M_SET_BC(bank,val)   (((bank)&Z80M_INV_BC)|((val&0xFFFFULL)<<Z80M_SFT_BC))
#define Z80M_SET_PC(bank,val)   (((bank)&Z80M_INV_PC)|((val&0xFFFFULL)<<Z80M_SFT_PC))
#define Z80M_SET_WZ(bank,val)   (((bank)&Z80M_INV_WZ)|((val&0xFFFFULL)<<Z80M_SFT_WZ))
#define Z80M_SET_SP(bank,val)   (((bank)&Z80M_INV_SP)|((val&0xFFFFULL)<<Z80M_SFT_SP))
#define Z80M_SET_IR(bank,val)   (((bank)&Z80M_INV_IR)|((val&0xFFFFULL)<<Z80M_SFT_IR))
#define Z80M_SET_IX(bank,val)   (((bank)&Z80M_INV_IX)|((val&0xFFFFULL)<<Z80M_SFT_IX))
#define Z80M_SET_IY(bank,val)   (((bank)&Z80M_INV_IY)|((val&0xFFFFULL)<<Z80M_SFT_IY))
#define Z80M_SET_IM(bank,val)   (((bank)&Z80M_INV_IM)|((val&0xFFFFULL)<<Z80M_SFT_IM))
#define Z80M_SET_IFF1(bank,val) (((bank)&Z80M_INV_IFF1)|((val&1ULL)<<Z80M_SFT_IFF1))
#define Z80M_SET_IFF2(bank,val) (((bank)&Z80M_INV_IFF2)|((val&1ULL)<<Z80M_SFT_IFF2))
#define Z80M_SET_EI(bank,val)   (((bank)&Z80M_INV_EI)|((val&1ULL)<<Z80M_SFT_EI))

#define Z80M_A(bank)    (((bank)>>Z80M_SFT_A)&0xFF)
#define Z80M_F(bank)    (((bank)>>Z80M_SFT_F)&0xFF)
#define Z80M_L(bank)    (((bank)>>Z80M_SFT_L)&0xFF)
#define Z80M_H(bank)    (((bank)>>Z80M_SFT_H)&0xFF)
#define Z80M_E(bank)    (((bank)>>Z80M_SFT_E)&0xFF)
#define Z80M_D(bank)    (((bank)>>Z80M_SFT_D)&0xFF)
#define Z80M_C(bank)    (((bank)>>Z80M_SFT_C)&0xFF)
#define Z80M_B(bank)    (((bank)>>Z80M_SFT_B)&0xFF)
#define Z80M_FA(bank)   (((bank)>>Z80M_SFT_FA)&0xFFFF)
#define Z80M_HL(bank)   (((bank)>>Z80M_SFT_HL)&0xFFFF)
#define Z80M_DE(bank)   (((bank)>>Z80M_SFT_DE)&0xFFFF)
#define Z80M_BC(bank)   (((bank)>>Z80M_SFT_BC)&0xFFFF)   
#define Z80M_PC(bank)   (((bank)>>Z80M_SFT_PC)&0xFFFF)
#define Z80M_WZ(bank)   (((bank)>>Z80M_SFT_WZ)&0xFFFF)
#define Z80M_SP(bank)   (((bank)>>Z80M_SFT_SP)&0xFFFF)
#define Z80M_IR(bank)   (((bank)>>Z80M_SFT_IR)&0xFFFF)
#define Z80M_IX(bank)   (((bank)>>Z80M_SFT_IX)&0xFFFF)
#define Z80M_IY(bank)   (((bank)>>Z80M_SFT_IY)&0xFFFF)
#define Z80M_IM(bank)   (((bank)>>Z80M_SFT_IM)&0xFF)
#define Z80M_IFF1(bank) (((bank)>>Z80M_SFT_IFF1)&1)
#define Z80M_IFF2(bank) (((bank)>>Z80M_SFT_IFF2)&1)
#define Z80M_EI(bank)   (((bank)>>Z80M_SFT_EI)&1)

/* initialization attributes */
typedef struct {
    z80m_tick_t tick_cb;
    void* user_data;
} z80m_desc_t;

/* Z80 mutable tick state */
typedef struct {
    /* main register bank (BC, DE, HL, FA) */
    uint64_t bcdehlfa;   /* B:63..56 C:55..48 D:47..40 E:39..32 H:31..24 L:23..16: F:15..8, A:7..0 */
    /* shadow register bank (BC', DE', HL', FA') */
    uint64_t bcdehlfa_;
    /* IR,SP,WZ,PC */
    uint64_t irspwzpc;
    /* EI-pending,IFF1,IFF2,IM,IY,IX */
    uint64_t ifimiyix;
} z80m_state_t;

/* Z80 CPU state */
typedef struct {
    z80m_tick_t tick;
    z80m_state_t state;
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
static inline void z80m_set_a(z80m_t* cpu, uint8_t v)   { cpu->state.bcdehlfa = Z80M_SET_A(cpu->state.bcdehlfa, v); }
static inline void z80m_set_f(z80m_t* cpu, uint8_t v)   { cpu->state.bcdehlfa = Z80M_SET_F(cpu->state.bcdehlfa, v); }
static inline void z80m_set_l(z80m_t* cpu, uint8_t v)   { cpu->state.bcdehlfa = Z80M_SET_L(cpu->state.bcdehlfa, v); }
static inline void z80m_set_h(z80m_t* cpu, uint8_t v)   { cpu->state.bcdehlfa = Z80M_SET_H(cpu->state.bcdehlfa, v); }
static inline void z80m_set_e(z80m_t* cpu, uint8_t v)   { cpu->state.bcdehlfa = Z80M_SET_E(cpu->state.bcdehlfa, v); }
static inline void z80m_set_d(z80m_t* cpu, uint8_t v)   { cpu->state.bcdehlfa = Z80M_SET_D(cpu->state.bcdehlfa, v); }
static inline void z80m_set_c(z80m_t* cpu, uint8_t v)   { cpu->state.bcdehlfa = Z80M_SET_C(cpu->state.bcdehlfa, v); }
static inline void z80m_set_b(z80m_t* cpu, uint8_t v)   { cpu->state.bcdehlfa = Z80M_SET_B(cpu->state.bcdehlfa, v); }
static inline void z80m_set_fa(z80m_t* cpu, uint16_t v) { cpu->state.bcdehlfa = Z80M_SET_FA(cpu->state.bcdehlfa, v); }
static inline void z80m_set_hl(z80m_t* cpu, uint16_t v) { cpu->state.bcdehlfa = Z80M_SET_HL(cpu->state.bcdehlfa, v); }
static inline void z80m_set_de(z80m_t* cpu, uint16_t v) { cpu->state.bcdehlfa = Z80M_SET_DE(cpu->state.bcdehlfa, v); }
static inline void z80m_set_bc(z80m_t* cpu, uint16_t v) { cpu->state.bcdehlfa = Z80M_SET_BC(cpu->state.bcdehlfa, v); }
static inline void z80m_set_fa_(z80m_t* cpu, uint16_t v) { cpu->state.bcdehlfa_ = Z80M_SET_FA(cpu->state.bcdehlfa_, v); }
static inline void z80m_set_hl_(z80m_t* cpu, uint16_t v) { cpu->state.bcdehlfa_ = Z80M_SET_HL(cpu->state.bcdehlfa_, v); }
static inline void z80m_set_de_(z80m_t* cpu, uint16_t v) { cpu->state.bcdehlfa_ = Z80M_SET_DE(cpu->state.bcdehlfa_, v); }
static inline void z80m_set_bc_(z80m_t* cpu, uint16_t v) { cpu->state.bcdehlfa_ = Z80M_SET_BC(cpu->state.bcdehlfa_, v); }
static inline void z80m_set_pc(z80m_t* cpu, uint16_t v) { cpu->state.irspwzpc = Z80M_SET_PC(cpu->state.irspwzpc, v); }
static inline void z80m_set_wz(z80m_t* cpu, uint16_t v) { cpu->state.irspwzpc = Z80M_SET_WZ(cpu->state.irspwzpc, v); }
static inline void z80m_set_sp(z80m_t* cpu, uint16_t v) { cpu->state.irspwzpc = Z80M_SET_SP(cpu->state.irspwzpc, v); }
static inline void z80m_set_ir(z80m_t* cpu, uint16_t v) { cpu->state.irspwzpc = Z80M_SET_IR(cpu->state.irspwzpc, v); }
static inline void z80m_set_ix(z80m_t* cpu, uint16_t v) { cpu->state.ifimiyix = Z80M_SET_IX(cpu->state.ifimiyix, v); }
static inline void z80m_set_iy(z80m_t* cpu, uint16_t v) { cpu->state.ifimiyix = Z80M_SET_IY(cpu->state.ifimiyix, v); }
static inline void z80m_set_im(z80m_t* cpu, uint8_t v)  { cpu->state.ifimiyix = Z80M_SET_IM(cpu->state.ifimiyix, v); }
static inline void z80m_set_iff1(z80m_t* cpu, bool b)   { cpu->state.ifimiyix = Z80M_SET_IFF1(cpu->state.ifimiyix, (b?1:0)); }
static inline void z80m_set_iff2(z80m_t* cpu, bool b)   { cpu->state.ifimiyix = Z80M_SET_IFF2(cpu->state.ifimiyix, (b?1:0)); }
static inline void z80m_set_ei_pending(z80m_t* cpu, bool b) { cpu->state.ifimiyix = Z80M_SET_EI(cpu->state.ifimiyix, (b?1:0)); }

static inline uint8_t z80m_a(z80m_t* cpu)   { return Z80M_A(cpu->state.bcdehlfa); }
static inline uint8_t z80m_f(z80m_t* cpu)   { return Z80M_F(cpu->state.bcdehlfa); }
static inline uint8_t z80m_l(z80m_t* cpu)   { return Z80M_L(cpu->state.bcdehlfa); }
static inline uint8_t z80m_h(z80m_t* cpu)   { return Z80M_H(cpu->state.bcdehlfa); }
static inline uint8_t z80m_e(z80m_t* cpu)   { return Z80M_E(cpu->state.bcdehlfa); }
static inline uint8_t z80m_d(z80m_t* cpu)   { return Z80M_D(cpu->state.bcdehlfa); }
static inline uint8_t z80m_c(z80m_t* cpu)   { return Z80M_C(cpu->state.bcdehlfa); }
static inline uint8_t z80m_b(z80m_t* cpu)   { return Z80M_B(cpu->state.bcdehlfa); }
static inline uint16_t z80m_fa(z80m_t* cpu) { return Z80M_FA(cpu->state.bcdehlfa); }
static inline uint16_t z80m_hl(z80m_t* cpu) { return Z80M_HL(cpu->state.bcdehlfa); }
static inline uint16_t z80m_de(z80m_t* cpu) { return Z80M_DE(cpu->state.bcdehlfa); }
static inline uint16_t z80m_bc(z80m_t* cpu) { return Z80M_BC(cpu->state.bcdehlfa); }
static inline uint16_t z80m_fa_(z80m_t* cpu) { return Z80M_FA(cpu->state.bcdehlfa_); }
static inline uint16_t z80m_hl_(z80m_t* cpu) { return Z80M_HL(cpu->state.bcdehlfa_); }
static inline uint16_t z80m_de_(z80m_t* cpu) { return Z80M_DE(cpu->state.bcdehlfa_); }
static inline uint16_t z80m_bc_(z80m_t* cpu) { return Z80M_BC(cpu->state.bcdehlfa_); }
static inline uint16_t z80m_pc(z80m_t* cpu) { return Z80M_PC(cpu->state.irspwzpc); }
static inline uint16_t z80m_wz(z80m_t* cpu) { return Z80M_WZ(cpu->state.irspwzpc); }
static inline uint16_t z80m_sp(z80m_t* cpu) { return Z80M_SP(cpu->state.irspwzpc); }
static inline uint16_t z80m_ir(z80m_t* cpu) { return Z80M_IR(cpu->state.irspwzpc); }
static inline uint16_t z80m_ix(z80m_t* cpu) { return Z80M_IX(cpu->state.ifimiyix); }
static inline uint16_t z80m_iy(z80m_t* cpu) { return Z80M_IY(cpu->state.ifimiyix); }
static inline uint8_t z80m_im(z80m_t* cpu)  { return Z80M_IM(cpu->state.ifimiyix); }
static inline bool z80m_iff1(z80m_t* cpu)   { return 0 != Z80M_IFF1(cpu->state.ifimiyix); }
static inline bool z80m_iff2(z80m_t* cpu)   { return 0 != Z80M_IFF2(cpu->state.ifimiyix); }
static inline bool z80m_ei_pending(z80m_t* cpu)     { return 0 != Z80M_EI(cpu->state.ifimiyix); }

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
    cpu->state.bcdehlfa = cpu->state.bcdehlfa_ = 0xFFFFFFFFFFFFFFFFULL;
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
    z80m_set_ei_pending(cpu, false);
}

#endif /* CHIPS_IMPL */
