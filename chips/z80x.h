#pragma once
/*
    FIXME
*/
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// address pins
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

// data pins
#define Z80_D0  (1ULL<<16)
#define Z80_D1  (1ULL<<17)
#define Z80_D2  (1ULL<<18)
#define Z80_D3  (1ULL<<19)
#define Z80_D4  (1ULL<<20)
#define Z80_D5  (1ULL<<21)
#define Z80_D6  (1ULL<<22)
#define Z80_D7  (1ULL<<23)

// 
#define Z80_M1    (1ULL<<24)        // machine cycle 1
#define Z80_MREQ  (1ULL<<25)        // memory request
#define Z80_IORQ  (1ULL<<26)        // input/output request
#define Z80_RD    (1ULL<<27)        // read
#define Z80_WR    (1ULL<<28)        // write
#define Z80_HALT  (1ULL<<29)        // halt state
#define Z80_INT   (1ULL<<30)        // interrupt request
#define Z80_RES   (1ULL<<31)        // reset requested
#define Z80_NMI   (1ULL<<32)        // non-maskable interrupt
#define Z80_WAIT  (1ULL<<33)        // wait requested
#define Z80_RFSH  (1ULL<<34)        // refresh

// virtual pins (for interrupt daisy chain protocol)
#define Z80_IEIO    (1ULL<<37)      // unified daisy chain 'Interrupt Enable In+Out'
#define Z80_RETI    (1ULL<<38)      // cpu has decoded a RETI instruction

#define Z80_CTRL_PIN_MASK (Z80_M1|Z80_MREQ|Z80_IORQ|Z80_RD|Z80_WR|Z80_RFSH)
#define Z80_PIN_MASK ((1ULL<<40)-1)

// status flags
#define Z80_CF (1<<0)           // carry
#define Z80_NF (1<<1)           // add/subtract
#define Z80_VF (1<<2)           // parity/overflow
#define Z80_PF Z80_VF
#define Z80_XF (1<<3)           // undocumented bit 3
#define Z80_HF (1<<4)           // half carry
#define Z80_YF (1<<5)           // undocumented bit 5
#define Z80_ZF (1<<6)           // zero
#define Z80_SF (1<<7)           // sign

// machine cycle execution pipeline bits (TODO: explain this stuff)
#define Z80_PIP_BIT_STEP        (1ULL<<0)  // step the instruction decoder forward
#define Z80_PIP_BIT_WAIT        (1ULL<<32) // sample the wait pin

#define Z80_PIP_BITS (Z80_PIP_BIT_STEP|Z80_PIP_BIT_WAIT)

#define Z80_PIP_MASK_STEP       (0xFFFFFFFFULL)
#define Z80_PIP_MASK_WAIT       (0xFFFFFFFF00000000ULL)

// CPU state
typedef struct {
    uint64_t pins;      // last stored pin state
    uint64_t pip;       // execution pipeline
    uint16_t ir;        // instruction register with extra bits for next 'active' tcycle
    uint16_t pc;        // program counter
    uint8_t f, a, c, b, e, d, l, h;
    uint16_t wz;
    uint16_t sp;
    uint16_t ix;
    uint16_t iy;
    uint8_t i;
    uint8_t r;
    uint8_t im;
    uint8_t af2, bc2, de2, hl2; // shadow register bank
    uint8_t dlatch;     // temporary store for data bus value
} z80_t;

// initialize a new Z80 instance and return initial pin mask
uint64_t z80_init(z80_t* cpu);
// execute one tick, return new pin mask
uint64_t z80_tick(z80_t* cpu, uint64_t pins);

#ifdef __cplusplus
} // extern C
#endif

//-- IMPLEMENTATION ------------------------------------------------------------
#ifdef CHIPS_IMPL
#include <string.h> // memset
#ifndef CHIPS_ASSERT
#include <assert.h>
#define CHIPS_ASSERT(c) assert(c)
#endif

static const uint64_t z80_pip_table[256] = {
    // nop (M:1 T:4)
    0x000000010000000A,
    // ld bc,nn (M:1 T:4)
    0x000000010000000A,
    // ld (bc),a (M:1 T:4)
    0x000000010000000A,
    // inc bc (M:1 T:4)
    0x000000010000000A,
    // inc b (M:1 T:4)
    0x000000010000000A,
    // dec b (M:1 T:4)
    0x000000010000000A,
    // ld b,n (M:1 T:4)
    0x000000010000000A,
    // rlca (M:1 T:4)
    0x000000010000000A,
    // ex af,af' (M:1 T:4)
    0x000000010000000A,
    // add hl,bc (M:1 T:4)
    0x000000010000000A,
    // ld a,(bc) (M:1 T:4)
    0x000000010000000A,
    // dec bc (M:1 T:4)
    0x000000010000000A,
    // inc c (M:1 T:4)
    0x000000010000000A,
    // dec c (M:1 T:4)
    0x000000010000000A,
    // ld c,n (M:1 T:4)
    0x000000010000000A,
    // rrca (M:1 T:4)
    0x000000010000000A,
    // djnz d (M:1 T:4)
    0x000000010000000A,
    // ld de,nn (M:1 T:4)
    0x000000010000000A,
    // ld (de),a (M:1 T:4)
    0x000000010000000A,
    // inc de (M:1 T:4)
    0x000000010000000A,
    // inc d (M:1 T:4)
    0x000000010000000A,
    // dec d (M:1 T:4)
    0x000000010000000A,
    // ld d,n (M:1 T:4)
    0x000000010000000A,
    // rla (M:1 T:4)
    0x000000010000000A,
    // jr d (M:1 T:4)
    0x000000010000000A,
    // add hl,de (M:1 T:4)
    0x000000010000000A,
    // ld a,(de) (M:1 T:4)
    0x000000010000000A,
    // dec de (M:1 T:4)
    0x000000010000000A,
    // inc e (M:1 T:4)
    0x000000010000000A,
    // dec e (M:1 T:4)
    0x000000010000000A,
    // ld e,n (M:1 T:4)
    0x000000010000000A,
    // rra (M:1 T:4)
    0x000000010000000A,
    // jr nz,d (M:1 T:4)
    0x000000010000000A,
    // ld hl,nn (M:1 T:4)
    0x000000010000000A,
    // ld (nn),hl (M:1 T:4)
    0x000000010000000A,
    // inc hl (M:1 T:4)
    0x000000010000000A,
    // inc h (M:1 T:4)
    0x000000010000000A,
    // dec h (M:1 T:4)
    0x000000010000000A,
    // ld h,n (M:1 T:4)
    0x000000010000000A,
    // daa (M:1 T:4)
    0x000000010000000A,
    // jr z,d (M:1 T:4)
    0x000000010000000A,
    // add hl,hl (M:1 T:4)
    0x000000010000000A,
    // ld hl,(nn) (M:1 T:4)
    0x000000010000000A,
    // dec hl (M:1 T:4)
    0x000000010000000A,
    // inc l (M:1 T:4)
    0x000000010000000A,
    // dec l (M:1 T:4)
    0x000000010000000A,
    // ld l,n (M:1 T:4)
    0x000000010000000A,
    // cpl (M:1 T:4)
    0x000000010000000A,
    // jr nc,d (M:1 T:4)
    0x000000010000000A,
    // ld sp,nn (M:1 T:4)
    0x000000010000000A,
    // ld (nn),a (M:1 T:4)
    0x000000010000000A,
    // inc sp (M:1 T:4)
    0x000000010000000A,
    // inc (hl) (M:1 T:4)
    0x000000010000000A,
    // dec (hl) (M:1 T:4)
    0x000000010000000A,
    // ld (hl),n (M:1 T:4)
    0x000000010000000A,
    // scf (M:1 T:4)
    0x000000010000000A,
    // jr c,d (M:1 T:4)
    0x000000010000000A,
    // add hl,sp (M:1 T:4)
    0x000000010000000A,
    // ld a,(nn) (M:1 T:4)
    0x000000010000000A,
    // dec sp (M:1 T:4)
    0x000000010000000A,
    // inc a (M:1 T:4)
    0x000000010000000A,
    // dec a (M:1 T:4)
    0x000000010000000A,
    // ld a,n (M:1 T:4)
    0x000000010000000A,
    // ccf (M:1 T:4)
    0x000000010000000A,
    // ld b,b (M:1 T:4)
    0x000000010000000A,
    // ld c,b (M:1 T:4)
    0x000000010000000A,
    // ld d,b (M:1 T:4)
    0x000000010000000A,
    // ld e,b (M:1 T:4)
    0x000000010000000A,
    // ld h,b (M:1 T:4)
    0x000000010000000A,
    // ld l,b (M:1 T:4)
    0x000000010000000A,
    // ld b,(hl) (M:2 T:7)
    0x000000110000005A,
    // ld a,b (M:1 T:4)
    0x000000010000000A,
    // ld b,c (M:1 T:4)
    0x000000010000000A,
    // ld c,c (M:1 T:4)
    0x000000010000000A,
    // ld d,c (M:1 T:4)
    0x000000010000000A,
    // ld e,c (M:1 T:4)
    0x000000010000000A,
    // ld h,c (M:1 T:4)
    0x000000010000000A,
    // ld l,c (M:1 T:4)
    0x000000010000000A,
    // ld c,(hl) (M:2 T:7)
    0x000000110000005A,
    // ld a,c (M:1 T:4)
    0x000000010000000A,
    // ld b,d (M:1 T:4)
    0x000000010000000A,
    // ld c,d (M:1 T:4)
    0x000000010000000A,
    // ld d,d (M:1 T:4)
    0x000000010000000A,
    // ld e,d (M:1 T:4)
    0x000000010000000A,
    // ld h,d (M:1 T:4)
    0x000000010000000A,
    // ld l,d (M:1 T:4)
    0x000000010000000A,
    // ld d,(hl) (M:2 T:7)
    0x000000110000005A,
    // ld a,d (M:1 T:4)
    0x000000010000000A,
    // ld b,e (M:1 T:4)
    0x000000010000000A,
    // ld c,e (M:1 T:4)
    0x000000010000000A,
    // ld d,e (M:1 T:4)
    0x000000010000000A,
    // ld e,e (M:1 T:4)
    0x000000010000000A,
    // ld h,e (M:1 T:4)
    0x000000010000000A,
    // ld l,e (M:1 T:4)
    0x000000010000000A,
    // ld e,(hl) (M:2 T:7)
    0x000000110000005A,
    // ld a,e (M:1 T:4)
    0x000000010000000A,
    // ld b,h (M:1 T:4)
    0x000000010000000A,
    // ld c,h (M:1 T:4)
    0x000000010000000A,
    // ld d,h (M:1 T:4)
    0x000000010000000A,
    // ld e,h (M:1 T:4)
    0x000000010000000A,
    // ld h,h (M:1 T:4)
    0x000000010000000A,
    // ld l,h (M:1 T:4)
    0x000000010000000A,
    // ld h,(hl) (M:2 T:7)
    0x000000110000005A,
    // ld a,h (M:1 T:4)
    0x000000010000000A,
    // ld b,l (M:1 T:4)
    0x000000010000000A,
    // ld c,l (M:1 T:4)
    0x000000010000000A,
    // ld d,l (M:1 T:4)
    0x000000010000000A,
    // ld e,l (M:1 T:4)
    0x000000010000000A,
    // ld h,l (M:1 T:4)
    0x000000010000000A,
    // ld l,l (M:1 T:4)
    0x000000010000000A,
    // ld l,(hl) (M:2 T:7)
    0x000000110000005A,
    // ld a,l (M:1 T:4)
    0x000000010000000A,
    // ld (hl),b (M:2 T:7)
    0x0000001100000052,
    // ld (hl),c (M:2 T:7)
    0x0000001100000052,
    // ld (hl),d (M:2 T:7)
    0x0000001100000052,
    // ld (hl),e (M:2 T:7)
    0x0000001100000052,
    // ld (hl),h (M:2 T:7)
    0x0000001100000052,
    // ld (hl),l (M:2 T:7)
    0x0000001100000052,
    // halt (M:1 T:4)
    0x000000010000000A,
    // ld (hl),a (M:2 T:7)
    0x0000001100000052,
    // ld b,a (M:1 T:4)
    0x000000010000000A,
    // ld c,a (M:1 T:4)
    0x000000010000000A,
    // ld d,a (M:1 T:4)
    0x000000010000000A,
    // ld e,a (M:1 T:4)
    0x000000010000000A,
    // ld h,a (M:1 T:4)
    0x000000010000000A,
    // ld l,a (M:1 T:4)
    0x000000010000000A,
    // ld a,(hl) (M:2 T:7)
    0x000000110000005A,
    // ld a,a (M:1 T:4)
    0x000000010000000A,
    // add b (M:1 T:4)
    0x000000010000000A,
    // add c (M:1 T:4)
    0x000000010000000A,
    // add d (M:1 T:4)
    0x000000010000000A,
    // add e (M:1 T:4)
    0x000000010000000A,
    // add h (M:1 T:4)
    0x000000010000000A,
    // add l (M:1 T:4)
    0x000000010000000A,
    // add (hl) (M:2 T:7)
    0x000000110000005A,
    // add a (M:1 T:4)
    0x000000010000000A,
    // adc b (M:1 T:4)
    0x000000010000000A,
    // adc c (M:1 T:4)
    0x000000010000000A,
    // adc d (M:1 T:4)
    0x000000010000000A,
    // adc e (M:1 T:4)
    0x000000010000000A,
    // adc h (M:1 T:4)
    0x000000010000000A,
    // adc l (M:1 T:4)
    0x000000010000000A,
    // adc (hl) (M:2 T:7)
    0x000000110000005A,
    // adc a (M:1 T:4)
    0x000000010000000A,
    // sub b (M:1 T:4)
    0x000000010000000A,
    // sub c (M:1 T:4)
    0x000000010000000A,
    // sub d (M:1 T:4)
    0x000000010000000A,
    // sub e (M:1 T:4)
    0x000000010000000A,
    // sub h (M:1 T:4)
    0x000000010000000A,
    // sub l (M:1 T:4)
    0x000000010000000A,
    // sub (hl) (M:2 T:7)
    0x000000110000005A,
    // sub a (M:1 T:4)
    0x000000010000000A,
    // sbc b (M:1 T:4)
    0x000000010000000A,
    // sbc c (M:1 T:4)
    0x000000010000000A,
    // sbc d (M:1 T:4)
    0x000000010000000A,
    // sbc e (M:1 T:4)
    0x000000010000000A,
    // sbc h (M:1 T:4)
    0x000000010000000A,
    // sbc l (M:1 T:4)
    0x000000010000000A,
    // sbc (hl) (M:2 T:7)
    0x000000110000005A,
    // sbc a (M:1 T:4)
    0x000000010000000A,
    // and b (M:1 T:4)
    0x000000010000000A,
    // and c (M:1 T:4)
    0x000000010000000A,
    // and d (M:1 T:4)
    0x000000010000000A,
    // and e (M:1 T:4)
    0x000000010000000A,
    // and h (M:1 T:4)
    0x000000010000000A,
    // and l (M:1 T:4)
    0x000000010000000A,
    // and (hl) (M:2 T:7)
    0x000000110000005A,
    // and a (M:1 T:4)
    0x000000010000000A,
    // xor b (M:1 T:4)
    0x000000010000000A,
    // xor c (M:1 T:4)
    0x000000010000000A,
    // xor d (M:1 T:4)
    0x000000010000000A,
    // xor e (M:1 T:4)
    0x000000010000000A,
    // xor h (M:1 T:4)
    0x000000010000000A,
    // xor l (M:1 T:4)
    0x000000010000000A,
    // xor (hl) (M:2 T:7)
    0x000000110000005A,
    // xor a (M:1 T:4)
    0x000000010000000A,
    // or b (M:1 T:4)
    0x000000010000000A,
    // or c (M:1 T:4)
    0x000000010000000A,
    // or d (M:1 T:4)
    0x000000010000000A,
    // or e (M:1 T:4)
    0x000000010000000A,
    // or h (M:1 T:4)
    0x000000010000000A,
    // or l (M:1 T:4)
    0x000000010000000A,
    // or (hl) (M:2 T:7)
    0x000000110000005A,
    // or a (M:1 T:4)
    0x000000010000000A,
    // cp b (M:1 T:4)
    0x000000010000000A,
    // cp c (M:1 T:4)
    0x000000010000000A,
    // cp d (M:1 T:4)
    0x000000010000000A,
    // cp e (M:1 T:4)
    0x000000010000000A,
    // cp h (M:1 T:4)
    0x000000010000000A,
    // cp l (M:1 T:4)
    0x000000010000000A,
    // cp (hl) (M:2 T:7)
    0x000000110000005A,
    // cp a (M:1 T:4)
    0x000000010000000A,
    // ret nz (M:1 T:4)
    0x000000010000000A,
    // pop bc2 (M:1 T:4)
    0x000000010000000A,
    // jp nz,nn (M:1 T:4)
    0x000000010000000A,
    // jp nn (M:1 T:4)
    0x000000010000000A,
    // call nz,nn (M:1 T:4)
    0x000000010000000A,
    // push bc2 (M:1 T:4)
    0x000000010000000A,
    // add n (M:1 T:4)
    0x000000010000000A,
    // rst 0h (M:1 T:4)
    0x000000010000000A,
    // ret z (M:1 T:4)
    0x000000010000000A,
    // ret (M:1 T:4)
    0x000000010000000A,
    // jp z,nn (M:1 T:4)
    0x000000010000000A,
    // cb prefix (M:1 T:4)
    0x000000010000000A,
    // call z,nn (M:1 T:4)
    0x000000010000000A,
    // call nn (M:1 T:4)
    0x000000010000000A,
    // adc n (M:1 T:4)
    0x000000010000000A,
    // rst 8h (M:1 T:4)
    0x000000010000000A,
    // ret nc (M:1 T:4)
    0x000000010000000A,
    // pop de2 (M:1 T:4)
    0x000000010000000A,
    // jp nc,nn (M:1 T:4)
    0x000000010000000A,
    // out (n),a (M:1 T:4)
    0x000000010000000A,
    // call nc,nn (M:1 T:4)
    0x000000010000000A,
    // push de2 (M:1 T:4)
    0x000000010000000A,
    // sub n (M:1 T:4)
    0x000000010000000A,
    // rst 10h (M:1 T:4)
    0x000000010000000A,
    // ret c (M:1 T:4)
    0x000000010000000A,
    // exx (M:1 T:4)
    0x000000010000000A,
    // jp c,nn (M:1 T:4)
    0x000000010000000A,
    // in a,(n) (M:1 T:4)
    0x000000010000000A,
    // call c,nn (M:1 T:4)
    0x000000010000000A,
    // dd prefix (M:1 T:4)
    0x000000010000000A,
    // sbc n (M:1 T:4)
    0x000000010000000A,
    // rst 18h (M:1 T:4)
    0x000000010000000A,
    // ret po (M:1 T:4)
    0x000000010000000A,
    // pop hl2 (M:1 T:4)
    0x000000010000000A,
    // jp po,nn (M:1 T:4)
    0x000000010000000A,
    // ex (sp),hl (M:1 T:4)
    0x000000010000000A,
    // call po,nn (M:1 T:4)
    0x000000010000000A,
    // push hl2 (M:1 T:4)
    0x000000010000000A,
    // and n (M:1 T:4)
    0x000000010000000A,
    // rst 20h (M:1 T:4)
    0x000000010000000A,
    // ret pe (M:1 T:4)
    0x000000010000000A,
    // jp hl (M:1 T:4)
    0x000000010000000A,
    // jp pe,nn (M:1 T:4)
    0x000000010000000A,
    // ex de,hl (M:1 T:4)
    0x000000010000000A,
    // call pe,nn (M:1 T:4)
    0x000000010000000A,
    // ed prefix (M:1 T:4)
    0x000000010000000A,
    // xor n (M:1 T:4)
    0x000000010000000A,
    // rst 28h (M:1 T:4)
    0x000000010000000A,
    // ret p (M:1 T:4)
    0x000000010000000A,
    // pop sp2 (M:1 T:4)
    0x000000010000000A,
    // jp p,nn (M:1 T:4)
    0x000000010000000A,
    // di (M:1 T:4)
    0x000000010000000A,
    // call p,nn (M:1 T:4)
    0x000000010000000A,
    // push sp2 (M:1 T:4)
    0x000000010000000A,
    // or n (M:1 T:4)
    0x000000010000000A,
    // rst 30h (M:1 T:4)
    0x000000010000000A,
    // ret m (M:1 T:4)
    0x000000010000000A,
    // ld sp,hl (M:1 T:4)
    0x000000010000000A,
    // jp m,nn (M:1 T:4)
    0x000000010000000A,
    // ei (M:1 T:4)
    0x000000010000000A,
    // call m,nn (M:1 T:4)
    0x000000010000000A,
    // fd prefix (M:1 T:4)
    0x000000010000000A,
    // cp n (M:1 T:4)
    0x000000010000000A,
    // rst 38h (M:1 T:4)
    0x000000010000000A,

};

uint64_t z80_init(z80_t* cpu) {
    CHIPS_ASSERT(cpu);
    memset(cpu, 0, sizeof(cpu));
    // initial state according to visualz80
    cpu->f = cpu->a = cpu->c = cpu->b = 0x55;
    cpu->e = cpu->d = cpu->l = cpu->h = 0x55f;
    cpu->wz = cpu->sp = cpu->ix = cpu->ix = 0x5555;
    cpu->af2 = cpu->bc2 = cpu->de2 = cpu->hl2 = 0x5555;
    // FIXME: iff1/2 disabled, initial value of IM???
}

static inline uint64_t z80_set_ab(uint64_t pins, uint16_t ab) {
    return (pins & ~0xFFFF) | ab;
}

static inline uint64_t z80_set_ab_x(uint64_t pins, uint16_t ab, uint64_t x) {
    return (pins & ~0xFFFF) | ab | x;
}

static inline uint64_t z80_set_ab_db(uint64_t pins, uint16_t ab, uint8_t db) {
    return (pins & ~0xFFFFFF) | (db<<16) | ab;
}

static inline uint64_t z80_set_ab_db_x(uint64_t pins, uint16_t ab, uint8_t db, uint64_t x) {
    return (pins & ~0xFFFFFF) | (db<<16) | ab | x;
}

static inline uint8_t z80_get_db(uint64_t pins) {
    return pins>>16;
}

// register helper macros
#define _gaf()      ((uint16_t)(cpu->f<<8)|cpu->a)
#define _gbc()      ((uint16_t)(cpu->b<<8)|cpu->c)
#define _gde()      ((uint16_t)(cpu->d<<8)|cpu->e)
#define _ghl()      ((uint16_t)(cpu->h<<8)|cpu->l)
#define _gsp()      (cpu->sp)
#define _saf(af)    {cpu->f=af>>8;cpu->a=af}
#define _sbc(bc)    {cpu->b=bc>>8;cpu->c=bc}
#define _sde(de)    {cpu->d=de>>8;cpu->e=de}
#define _shl(hl)    {cpu->h=hl>>8;cpu->l=hl}
#define _ssp(sp)    {cpu->sp=sp;}

// pin helper macros
#define _sa(ab)             pins=z80_set_ab(pins,ab)
#define _sax(ab,x)          pins=z80_set_ab_x(pins,ab,x)
#define _sad(ab,d)          pins=z80_set_ab_db(pins,ab,d)
#define _sadx(ab,d,x)       pins=z80_set_ab_db_x(pins,ab,d,x)
#define _gd()               z80_get_db(pins)

// high level helper macros
#define _fetch()    _sax(cpu->pc++,Z80_M1|Z80_MREQ|Z80_RD)
#define _rfsh()     _sax(cpu->r,Z80_MREQ|Z80_RFSH);cpu->r=(cpu->r&0x80)|((cpu->r+1)&0x7F)
#define _mr(ab)     _sax(ab,Z80_MREQ|Z80_RD)
#define _mw(ab,d)   _sadx(ab,d,Z80_MREQ|Z80_WR)
#define _ior(ab)    _sax(ab,Z80_IORQ|Z80_RD)
#define _iow(ab,d)  _sadx(ab,d,Z80_IORQ|Z80_WR)

uint64_t z80_tick(z80_t* cpu, uint64_t pins) {
    uint64_t pip = cpu->pip;
    // wait cycle?
    if ((pip & Z80_PIP_BIT_WAIT) && (pins & Z80_WAIT)) {
        cpu->pins = pins & ~Z80_CTRL_PIN_MASK;
        return pins;
    }
    // check if new opcode must be loaded from data bus
    if ((pins & (Z80_M1|Z80_MREQ|Z80_RD)) == (Z80_M1|Z80_MREQ|Z80_RD)) {
        uint8_t opcode = _gd();
        cpu->ir = opcode<<3;
        pip = z80_pip_table[opcode];
    }
    // process the next 'active' tcycle
    pins &= ~Z80_CTRL_PIN_MASK;
    if (pip & Z80_PIP_BIT_STEP) {
        switch (cpu->ir++) {
        
        // nop (M:1 T:4)
        // -- M1
        case (0x00<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x00<<3)|1: _fetch(); break;
        
        // ld bc,nn (M:1 T:4)
        // -- M1
        case (0x01<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x01<<3)|1: _fetch(); break;
        
        // ld (bc),a (M:1 T:4)
        // -- M1
        case (0x02<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x02<<3)|1: _fetch(); break;
        
        // inc bc (M:1 T:4)
        // -- M1
        case (0x03<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x03<<3)|1: _fetch(); break;
        
        // inc b (M:1 T:4)
        // -- M1
        case (0x04<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x04<<3)|1: _fetch(); break;
        
        // dec b (M:1 T:4)
        // -- M1
        case (0x05<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x05<<3)|1: _fetch(); break;
        
        // ld b,n (M:1 T:4)
        // -- M1
        case (0x06<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x06<<3)|1: _fetch(); break;
        
        // rlca (M:1 T:4)
        // -- M1
        case (0x07<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x07<<3)|1: _fetch(); break;
        
        // ex af,af' (M:1 T:4)
        // -- M1
        case (0x08<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x08<<3)|1: _fetch(); break;
        
        // add hl,bc (M:1 T:4)
        // -- M1
        case (0x09<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x09<<3)|1: _fetch(); break;
        
        // ld a,(bc) (M:1 T:4)
        // -- M1
        case (0x0A<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x0A<<3)|1: _fetch(); break;
        
        // dec bc (M:1 T:4)
        // -- M1
        case (0x0B<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x0B<<3)|1: _fetch(); break;
        
        // inc c (M:1 T:4)
        // -- M1
        case (0x0C<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x0C<<3)|1: _fetch(); break;
        
        // dec c (M:1 T:4)
        // -- M1
        case (0x0D<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x0D<<3)|1: _fetch(); break;
        
        // ld c,n (M:1 T:4)
        // -- M1
        case (0x0E<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x0E<<3)|1: _fetch(); break;
        
        // rrca (M:1 T:4)
        // -- M1
        case (0x0F<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x0F<<3)|1: _fetch(); break;
        
        // djnz d (M:1 T:4)
        // -- M1
        case (0x10<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x10<<3)|1: _fetch(); break;
        
        // ld de,nn (M:1 T:4)
        // -- M1
        case (0x11<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x11<<3)|1: _fetch(); break;
        
        // ld (de),a (M:1 T:4)
        // -- M1
        case (0x12<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x12<<3)|1: _fetch(); break;
        
        // inc de (M:1 T:4)
        // -- M1
        case (0x13<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x13<<3)|1: _fetch(); break;
        
        // inc d (M:1 T:4)
        // -- M1
        case (0x14<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x14<<3)|1: _fetch(); break;
        
        // dec d (M:1 T:4)
        // -- M1
        case (0x15<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x15<<3)|1: _fetch(); break;
        
        // ld d,n (M:1 T:4)
        // -- M1
        case (0x16<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x16<<3)|1: _fetch(); break;
        
        // rla (M:1 T:4)
        // -- M1
        case (0x17<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x17<<3)|1: _fetch(); break;
        
        // jr d (M:1 T:4)
        // -- M1
        case (0x18<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x18<<3)|1: _fetch(); break;
        
        // add hl,de (M:1 T:4)
        // -- M1
        case (0x19<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x19<<3)|1: _fetch(); break;
        
        // ld a,(de) (M:1 T:4)
        // -- M1
        case (0x1A<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x1A<<3)|1: _fetch(); break;
        
        // dec de (M:1 T:4)
        // -- M1
        case (0x1B<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x1B<<3)|1: _fetch(); break;
        
        // inc e (M:1 T:4)
        // -- M1
        case (0x1C<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x1C<<3)|1: _fetch(); break;
        
        // dec e (M:1 T:4)
        // -- M1
        case (0x1D<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x1D<<3)|1: _fetch(); break;
        
        // ld e,n (M:1 T:4)
        // -- M1
        case (0x1E<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x1E<<3)|1: _fetch(); break;
        
        // rra (M:1 T:4)
        // -- M1
        case (0x1F<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x1F<<3)|1: _fetch(); break;
        
        // jr nz,d (M:1 T:4)
        // -- M1
        case (0x20<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x20<<3)|1: _fetch(); break;
        
        // ld hl,nn (M:1 T:4)
        // -- M1
        case (0x21<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x21<<3)|1: _fetch(); break;
        
        // ld (nn),hl (M:1 T:4)
        // -- M1
        case (0x22<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x22<<3)|1: _fetch(); break;
        
        // inc hl (M:1 T:4)
        // -- M1
        case (0x23<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x23<<3)|1: _fetch(); break;
        
        // inc h (M:1 T:4)
        // -- M1
        case (0x24<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x24<<3)|1: _fetch(); break;
        
        // dec h (M:1 T:4)
        // -- M1
        case (0x25<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x25<<3)|1: _fetch(); break;
        
        // ld h,n (M:1 T:4)
        // -- M1
        case (0x26<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x26<<3)|1: _fetch(); break;
        
        // daa (M:1 T:4)
        // -- M1
        case (0x27<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x27<<3)|1: _fetch(); break;
        
        // jr z,d (M:1 T:4)
        // -- M1
        case (0x28<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x28<<3)|1: _fetch(); break;
        
        // add hl,hl (M:1 T:4)
        // -- M1
        case (0x29<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x29<<3)|1: _fetch(); break;
        
        // ld hl,(nn) (M:1 T:4)
        // -- M1
        case (0x2A<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x2A<<3)|1: _fetch(); break;
        
        // dec hl (M:1 T:4)
        // -- M1
        case (0x2B<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x2B<<3)|1: _fetch(); break;
        
        // inc l (M:1 T:4)
        // -- M1
        case (0x2C<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x2C<<3)|1: _fetch(); break;
        
        // dec l (M:1 T:4)
        // -- M1
        case (0x2D<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x2D<<3)|1: _fetch(); break;
        
        // ld l,n (M:1 T:4)
        // -- M1
        case (0x2E<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x2E<<3)|1: _fetch(); break;
        
        // cpl (M:1 T:4)
        // -- M1
        case (0x2F<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x2F<<3)|1: _fetch(); break;
        
        // jr nc,d (M:1 T:4)
        // -- M1
        case (0x30<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x30<<3)|1: _fetch(); break;
        
        // ld sp,nn (M:1 T:4)
        // -- M1
        case (0x31<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x31<<3)|1: _fetch(); break;
        
        // ld (nn),a (M:1 T:4)
        // -- M1
        case (0x32<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x32<<3)|1: _fetch(); break;
        
        // inc sp (M:1 T:4)
        // -- M1
        case (0x33<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x33<<3)|1: _fetch(); break;
        
        // inc (hl) (M:1 T:4)
        // -- M1
        case (0x34<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x34<<3)|1: _fetch(); break;
        
        // dec (hl) (M:1 T:4)
        // -- M1
        case (0x35<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x35<<3)|1: _fetch(); break;
        
        // ld (hl),n (M:1 T:4)
        // -- M1
        case (0x36<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x36<<3)|1: _fetch(); break;
        
        // scf (M:1 T:4)
        // -- M1
        case (0x37<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x37<<3)|1: _fetch(); break;
        
        // jr c,d (M:1 T:4)
        // -- M1
        case (0x38<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x38<<3)|1: _fetch(); break;
        
        // add hl,sp (M:1 T:4)
        // -- M1
        case (0x39<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x39<<3)|1: _fetch(); break;
        
        // ld a,(nn) (M:1 T:4)
        // -- M1
        case (0x3A<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x3A<<3)|1: _fetch(); break;
        
        // dec sp (M:1 T:4)
        // -- M1
        case (0x3B<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x3B<<3)|1: _fetch(); break;
        
        // inc a (M:1 T:4)
        // -- M1
        case (0x3C<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x3C<<3)|1: _fetch(); break;
        
        // dec a (M:1 T:4)
        // -- M1
        case (0x3D<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x3D<<3)|1: _fetch(); break;
        
        // ld a,n (M:1 T:4)
        // -- M1
        case (0x3E<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x3E<<3)|1: _fetch(); break;
        
        // ccf (M:1 T:4)
        // -- M1
        case (0x3F<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x3F<<3)|1: _fetch(); break;
        
        // ld b,b (M:1 T:4)
        // -- M1
        case (0x40<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x40<<3)|1: cpu->b=cpu->b;_fetch(); break;
        
        // ld c,b (M:1 T:4)
        // -- M1
        case (0x41<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x41<<3)|1: cpu->b=cpu->c;_fetch(); break;
        
        // ld d,b (M:1 T:4)
        // -- M1
        case (0x42<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x42<<3)|1: cpu->b=cpu->d;_fetch(); break;
        
        // ld e,b (M:1 T:4)
        // -- M1
        case (0x43<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x43<<3)|1: cpu->b=cpu->e;_fetch(); break;
        
        // ld h,b (M:1 T:4)
        // -- M1
        case (0x44<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x44<<3)|1: cpu->b=cpu->h;_fetch(); break;
        
        // ld l,b (M:1 T:4)
        // -- M1
        case (0x45<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x45<<3)|1: cpu->b=cpu->l;_fetch(); break;
        
        // ld b,(hl) (M:2 T:7)
        // -- M1
        case (0x46<<3)|0: _rfsh(); break;
        // -- M2
        case (0x46<<3)|1: _mr(_ghl()); break;
        case (0x46<<3)|2: cpu->b=_gd(); break;
        // -- OVERLAP
        case (0x46<<3)|3: _fetch(); break;
        
        // ld a,b (M:1 T:4)
        // -- M1
        case (0x47<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x47<<3)|1: cpu->b=cpu->a;_fetch(); break;
        
        // ld b,c (M:1 T:4)
        // -- M1
        case (0x48<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x48<<3)|1: cpu->c=cpu->b;_fetch(); break;
        
        // ld c,c (M:1 T:4)
        // -- M1
        case (0x49<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x49<<3)|1: cpu->c=cpu->c;_fetch(); break;
        
        // ld d,c (M:1 T:4)
        // -- M1
        case (0x4A<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x4A<<3)|1: cpu->c=cpu->d;_fetch(); break;
        
        // ld e,c (M:1 T:4)
        // -- M1
        case (0x4B<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x4B<<3)|1: cpu->c=cpu->e;_fetch(); break;
        
        // ld h,c (M:1 T:4)
        // -- M1
        case (0x4C<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x4C<<3)|1: cpu->c=cpu->h;_fetch(); break;
        
        // ld l,c (M:1 T:4)
        // -- M1
        case (0x4D<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x4D<<3)|1: cpu->c=cpu->l;_fetch(); break;
        
        // ld c,(hl) (M:2 T:7)
        // -- M1
        case (0x4E<<3)|0: _rfsh(); break;
        // -- M2
        case (0x4E<<3)|1: _mr(_ghl()); break;
        case (0x4E<<3)|2: cpu->c=_gd(); break;
        // -- OVERLAP
        case (0x4E<<3)|3: _fetch(); break;
        
        // ld a,c (M:1 T:4)
        // -- M1
        case (0x4F<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x4F<<3)|1: cpu->c=cpu->a;_fetch(); break;
        
        // ld b,d (M:1 T:4)
        // -- M1
        case (0x50<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x50<<3)|1: cpu->d=cpu->b;_fetch(); break;
        
        // ld c,d (M:1 T:4)
        // -- M1
        case (0x51<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x51<<3)|1: cpu->d=cpu->c;_fetch(); break;
        
        // ld d,d (M:1 T:4)
        // -- M1
        case (0x52<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x52<<3)|1: cpu->d=cpu->d;_fetch(); break;
        
        // ld e,d (M:1 T:4)
        // -- M1
        case (0x53<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x53<<3)|1: cpu->d=cpu->e;_fetch(); break;
        
        // ld h,d (M:1 T:4)
        // -- M1
        case (0x54<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x54<<3)|1: cpu->d=cpu->h;_fetch(); break;
        
        // ld l,d (M:1 T:4)
        // -- M1
        case (0x55<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x55<<3)|1: cpu->d=cpu->l;_fetch(); break;
        
        // ld d,(hl) (M:2 T:7)
        // -- M1
        case (0x56<<3)|0: _rfsh(); break;
        // -- M2
        case (0x56<<3)|1: _mr(_ghl()); break;
        case (0x56<<3)|2: cpu->d=_gd(); break;
        // -- OVERLAP
        case (0x56<<3)|3: _fetch(); break;
        
        // ld a,d (M:1 T:4)
        // -- M1
        case (0x57<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x57<<3)|1: cpu->d=cpu->a;_fetch(); break;
        
        // ld b,e (M:1 T:4)
        // -- M1
        case (0x58<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x58<<3)|1: cpu->e=cpu->b;_fetch(); break;
        
        // ld c,e (M:1 T:4)
        // -- M1
        case (0x59<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x59<<3)|1: cpu->e=cpu->c;_fetch(); break;
        
        // ld d,e (M:1 T:4)
        // -- M1
        case (0x5A<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x5A<<3)|1: cpu->e=cpu->d;_fetch(); break;
        
        // ld e,e (M:1 T:4)
        // -- M1
        case (0x5B<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x5B<<3)|1: cpu->e=cpu->e;_fetch(); break;
        
        // ld h,e (M:1 T:4)
        // -- M1
        case (0x5C<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x5C<<3)|1: cpu->e=cpu->h;_fetch(); break;
        
        // ld l,e (M:1 T:4)
        // -- M1
        case (0x5D<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x5D<<3)|1: cpu->e=cpu->l;_fetch(); break;
        
        // ld e,(hl) (M:2 T:7)
        // -- M1
        case (0x5E<<3)|0: _rfsh(); break;
        // -- M2
        case (0x5E<<3)|1: _mr(_ghl()); break;
        case (0x5E<<3)|2: cpu->e=_gd(); break;
        // -- OVERLAP
        case (0x5E<<3)|3: _fetch(); break;
        
        // ld a,e (M:1 T:4)
        // -- M1
        case (0x5F<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x5F<<3)|1: cpu->e=cpu->a;_fetch(); break;
        
        // ld b,h (M:1 T:4)
        // -- M1
        case (0x60<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x60<<3)|1: cpu->h=cpu->b;_fetch(); break;
        
        // ld c,h (M:1 T:4)
        // -- M1
        case (0x61<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x61<<3)|1: cpu->h=cpu->c;_fetch(); break;
        
        // ld d,h (M:1 T:4)
        // -- M1
        case (0x62<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x62<<3)|1: cpu->h=cpu->d;_fetch(); break;
        
        // ld e,h (M:1 T:4)
        // -- M1
        case (0x63<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x63<<3)|1: cpu->h=cpu->e;_fetch(); break;
        
        // ld h,h (M:1 T:4)
        // -- M1
        case (0x64<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x64<<3)|1: cpu->h=cpu->h;_fetch(); break;
        
        // ld l,h (M:1 T:4)
        // -- M1
        case (0x65<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x65<<3)|1: cpu->h=cpu->l;_fetch(); break;
        
        // ld h,(hl) (M:2 T:7)
        // -- M1
        case (0x66<<3)|0: _rfsh(); break;
        // -- M2
        case (0x66<<3)|1: _mr(_ghl()); break;
        case (0x66<<3)|2: cpu->h=_gd(); break;
        // -- OVERLAP
        case (0x66<<3)|3: _fetch(); break;
        
        // ld a,h (M:1 T:4)
        // -- M1
        case (0x67<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x67<<3)|1: cpu->h=cpu->a;_fetch(); break;
        
        // ld b,l (M:1 T:4)
        // -- M1
        case (0x68<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x68<<3)|1: cpu->l=cpu->b;_fetch(); break;
        
        // ld c,l (M:1 T:4)
        // -- M1
        case (0x69<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x69<<3)|1: cpu->l=cpu->c;_fetch(); break;
        
        // ld d,l (M:1 T:4)
        // -- M1
        case (0x6A<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x6A<<3)|1: cpu->l=cpu->d;_fetch(); break;
        
        // ld e,l (M:1 T:4)
        // -- M1
        case (0x6B<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x6B<<3)|1: cpu->l=cpu->e;_fetch(); break;
        
        // ld h,l (M:1 T:4)
        // -- M1
        case (0x6C<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x6C<<3)|1: cpu->l=cpu->h;_fetch(); break;
        
        // ld l,l (M:1 T:4)
        // -- M1
        case (0x6D<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x6D<<3)|1: cpu->l=cpu->l;_fetch(); break;
        
        // ld l,(hl) (M:2 T:7)
        // -- M1
        case (0x6E<<3)|0: _rfsh(); break;
        // -- M2
        case (0x6E<<3)|1: _mr(_ghl()); break;
        case (0x6E<<3)|2: cpu->l=_gd(); break;
        // -- OVERLAP
        case (0x6E<<3)|3: _fetch(); break;
        
        // ld a,l (M:1 T:4)
        // -- M1
        case (0x6F<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x6F<<3)|1: cpu->l=cpu->a;_fetch(); break;
        
        // ld (hl),b (M:2 T:7)
        // -- M1
        case (0x70<<3)|0: _rfsh(); break;
        // -- M2
        case (0x70<<3)|1: _mw(0xFFFF,0xFF)/*FIXME: address and data!*/; break;
        // -- OVERLAP
        case (0x70<<3)|2: _fetch(); break;
        
        // ld (hl),c (M:2 T:7)
        // -- M1
        case (0x71<<3)|0: _rfsh(); break;
        // -- M2
        case (0x71<<3)|1: _mw(0xFFFF,0xFF)/*FIXME: address and data!*/; break;
        // -- OVERLAP
        case (0x71<<3)|2: _fetch(); break;
        
        // ld (hl),d (M:2 T:7)
        // -- M1
        case (0x72<<3)|0: _rfsh(); break;
        // -- M2
        case (0x72<<3)|1: _mw(0xFFFF,0xFF)/*FIXME: address and data!*/; break;
        // -- OVERLAP
        case (0x72<<3)|2: _fetch(); break;
        
        // ld (hl),e (M:2 T:7)
        // -- M1
        case (0x73<<3)|0: _rfsh(); break;
        // -- M2
        case (0x73<<3)|1: _mw(0xFFFF,0xFF)/*FIXME: address and data!*/; break;
        // -- OVERLAP
        case (0x73<<3)|2: _fetch(); break;
        
        // ld (hl),h (M:2 T:7)
        // -- M1
        case (0x74<<3)|0: _rfsh(); break;
        // -- M2
        case (0x74<<3)|1: _mw(0xFFFF,0xFF)/*FIXME: address and data!*/; break;
        // -- OVERLAP
        case (0x74<<3)|2: _fetch(); break;
        
        // ld (hl),l (M:2 T:7)
        // -- M1
        case (0x75<<3)|0: _rfsh(); break;
        // -- M2
        case (0x75<<3)|1: _mw(0xFFFF,0xFF)/*FIXME: address and data!*/; break;
        // -- OVERLAP
        case (0x75<<3)|2: _fetch(); break;
        
        // halt (M:1 T:4)
        // -- M1
        case (0x76<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x76<<3)|1: z80_halt(cpu);_fetch(); break;
        
        // ld (hl),a (M:2 T:7)
        // -- M1
        case (0x77<<3)|0: _rfsh(); break;
        // -- M2
        case (0x77<<3)|1: _mw(0xFFFF,0xFF)/*FIXME: address and data!*/; break;
        // -- OVERLAP
        case (0x77<<3)|2: _fetch(); break;
        
        // ld b,a (M:1 T:4)
        // -- M1
        case (0x78<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x78<<3)|1: cpu->a=cpu->b;_fetch(); break;
        
        // ld c,a (M:1 T:4)
        // -- M1
        case (0x79<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x79<<3)|1: cpu->a=cpu->c;_fetch(); break;
        
        // ld d,a (M:1 T:4)
        // -- M1
        case (0x7A<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x7A<<3)|1: cpu->a=cpu->d;_fetch(); break;
        
        // ld e,a (M:1 T:4)
        // -- M1
        case (0x7B<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x7B<<3)|1: cpu->a=cpu->e;_fetch(); break;
        
        // ld h,a (M:1 T:4)
        // -- M1
        case (0x7C<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x7C<<3)|1: cpu->a=cpu->h;_fetch(); break;
        
        // ld l,a (M:1 T:4)
        // -- M1
        case (0x7D<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x7D<<3)|1: cpu->a=cpu->l;_fetch(); break;
        
        // ld a,(hl) (M:2 T:7)
        // -- M1
        case (0x7E<<3)|0: _rfsh(); break;
        // -- M2
        case (0x7E<<3)|1: _mr(_ghl()); break;
        case (0x7E<<3)|2: cpu->a=_gd(); break;
        // -- OVERLAP
        case (0x7E<<3)|3: _fetch(); break;
        
        // ld a,a (M:1 T:4)
        // -- M1
        case (0x7F<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x7F<<3)|1: cpu->a=cpu->a;_fetch(); break;
        
        // add b (M:1 T:4)
        // -- M1
        case (0x80<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x80<<3)|1: z80_add(cpu,cpu->b);_fetch(); break;
        
        // add c (M:1 T:4)
        // -- M1
        case (0x81<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x81<<3)|1: z80_add(cpu,cpu->c);_fetch(); break;
        
        // add d (M:1 T:4)
        // -- M1
        case (0x82<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x82<<3)|1: z80_add(cpu,cpu->d);_fetch(); break;
        
        // add e (M:1 T:4)
        // -- M1
        case (0x83<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x83<<3)|1: z80_add(cpu,cpu->e);_fetch(); break;
        
        // add h (M:1 T:4)
        // -- M1
        case (0x84<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x84<<3)|1: z80_add(cpu,cpu->h);_fetch(); break;
        
        // add l (M:1 T:4)
        // -- M1
        case (0x85<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x85<<3)|1: z80_add(cpu,cpu->l);_fetch(); break;
        
        // add (hl) (M:2 T:7)
        // -- M1
        case (0x86<<3)|0: _rfsh(); break;
        // -- M2
        case (0x86<<3)|1: _mr(_ghl()); break;
        case (0x86<<3)|2: cpu->dlatch=_gd(); break;
        // -- OVERLAP
        case (0x86<<3)|3: z80_add(cpu,cpu->dlatch);_fetch(); break;
        
        // add a (M:1 T:4)
        // -- M1
        case (0x87<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x87<<3)|1: z80_add(cpu,cpu->a);_fetch(); break;
        
        // adc b (M:1 T:4)
        // -- M1
        case (0x88<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x88<<3)|1: z80_adc(cpu,cpu->b);_fetch(); break;
        
        // adc c (M:1 T:4)
        // -- M1
        case (0x89<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x89<<3)|1: z80_adc(cpu,cpu->c);_fetch(); break;
        
        // adc d (M:1 T:4)
        // -- M1
        case (0x8A<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x8A<<3)|1: z80_adc(cpu,cpu->d);_fetch(); break;
        
        // adc e (M:1 T:4)
        // -- M1
        case (0x8B<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x8B<<3)|1: z80_adc(cpu,cpu->e);_fetch(); break;
        
        // adc h (M:1 T:4)
        // -- M1
        case (0x8C<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x8C<<3)|1: z80_adc(cpu,cpu->h);_fetch(); break;
        
        // adc l (M:1 T:4)
        // -- M1
        case (0x8D<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x8D<<3)|1: z80_adc(cpu,cpu->l);_fetch(); break;
        
        // adc (hl) (M:2 T:7)
        // -- M1
        case (0x8E<<3)|0: _rfsh(); break;
        // -- M2
        case (0x8E<<3)|1: _mr(_ghl()); break;
        case (0x8E<<3)|2: cpu->dlatch=_gd(); break;
        // -- OVERLAP
        case (0x8E<<3)|3: z80_adc(cpu,cpu->dlatch);_fetch(); break;
        
        // adc a (M:1 T:4)
        // -- M1
        case (0x8F<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x8F<<3)|1: z80_adc(cpu,cpu->a);_fetch(); break;
        
        // sub b (M:1 T:4)
        // -- M1
        case (0x90<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x90<<3)|1: z80_sub(cpu,cpu->b);_fetch(); break;
        
        // sub c (M:1 T:4)
        // -- M1
        case (0x91<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x91<<3)|1: z80_sub(cpu,cpu->c);_fetch(); break;
        
        // sub d (M:1 T:4)
        // -- M1
        case (0x92<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x92<<3)|1: z80_sub(cpu,cpu->d);_fetch(); break;
        
        // sub e (M:1 T:4)
        // -- M1
        case (0x93<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x93<<3)|1: z80_sub(cpu,cpu->e);_fetch(); break;
        
        // sub h (M:1 T:4)
        // -- M1
        case (0x94<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x94<<3)|1: z80_sub(cpu,cpu->h);_fetch(); break;
        
        // sub l (M:1 T:4)
        // -- M1
        case (0x95<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x95<<3)|1: z80_sub(cpu,cpu->l);_fetch(); break;
        
        // sub (hl) (M:2 T:7)
        // -- M1
        case (0x96<<3)|0: _rfsh(); break;
        // -- M2
        case (0x96<<3)|1: _mr(_ghl()); break;
        case (0x96<<3)|2: cpu->dlatch=_gd(); break;
        // -- OVERLAP
        case (0x96<<3)|3: z80_sub(cpu,cpu->dlatch);_fetch(); break;
        
        // sub a (M:1 T:4)
        // -- M1
        case (0x97<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x97<<3)|1: z80_sub(cpu,cpu->a);_fetch(); break;
        
        // sbc b (M:1 T:4)
        // -- M1
        case (0x98<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x98<<3)|1: z80_sbc(cpu,cpu->b);_fetch(); break;
        
        // sbc c (M:1 T:4)
        // -- M1
        case (0x99<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x99<<3)|1: z80_sbc(cpu,cpu->c);_fetch(); break;
        
        // sbc d (M:1 T:4)
        // -- M1
        case (0x9A<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x9A<<3)|1: z80_sbc(cpu,cpu->d);_fetch(); break;
        
        // sbc e (M:1 T:4)
        // -- M1
        case (0x9B<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x9B<<3)|1: z80_sbc(cpu,cpu->e);_fetch(); break;
        
        // sbc h (M:1 T:4)
        // -- M1
        case (0x9C<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x9C<<3)|1: z80_sbc(cpu,cpu->h);_fetch(); break;
        
        // sbc l (M:1 T:4)
        // -- M1
        case (0x9D<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x9D<<3)|1: z80_sbc(cpu,cpu->l);_fetch(); break;
        
        // sbc (hl) (M:2 T:7)
        // -- M1
        case (0x9E<<3)|0: _rfsh(); break;
        // -- M2
        case (0x9E<<3)|1: _mr(_ghl()); break;
        case (0x9E<<3)|2: cpu->dlatch=_gd(); break;
        // -- OVERLAP
        case (0x9E<<3)|3: z80_sbc(cpu,cpu->dlatch);_fetch(); break;
        
        // sbc a (M:1 T:4)
        // -- M1
        case (0x9F<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0x9F<<3)|1: z80_sbc(cpu,cpu->a);_fetch(); break;
        
        // and b (M:1 T:4)
        // -- M1
        case (0xA0<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xA0<<3)|1: z80_and(cpu,cpu->b);_fetch(); break;
        
        // and c (M:1 T:4)
        // -- M1
        case (0xA1<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xA1<<3)|1: z80_and(cpu,cpu->c);_fetch(); break;
        
        // and d (M:1 T:4)
        // -- M1
        case (0xA2<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xA2<<3)|1: z80_and(cpu,cpu->d);_fetch(); break;
        
        // and e (M:1 T:4)
        // -- M1
        case (0xA3<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xA3<<3)|1: z80_and(cpu,cpu->e);_fetch(); break;
        
        // and h (M:1 T:4)
        // -- M1
        case (0xA4<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xA4<<3)|1: z80_and(cpu,cpu->h);_fetch(); break;
        
        // and l (M:1 T:4)
        // -- M1
        case (0xA5<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xA5<<3)|1: z80_and(cpu,cpu->l);_fetch(); break;
        
        // and (hl) (M:2 T:7)
        // -- M1
        case (0xA6<<3)|0: _rfsh(); break;
        // -- M2
        case (0xA6<<3)|1: _mr(_ghl()); break;
        case (0xA6<<3)|2: cpu->dlatch=_gd(); break;
        // -- OVERLAP
        case (0xA6<<3)|3: z80_and(cpu,cpu->dlatch);_fetch(); break;
        
        // and a (M:1 T:4)
        // -- M1
        case (0xA7<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xA7<<3)|1: z80_and(cpu,cpu->a);_fetch(); break;
        
        // xor b (M:1 T:4)
        // -- M1
        case (0xA8<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xA8<<3)|1: z80_xor(cpu,cpu->b);_fetch(); break;
        
        // xor c (M:1 T:4)
        // -- M1
        case (0xA9<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xA9<<3)|1: z80_xor(cpu,cpu->c);_fetch(); break;
        
        // xor d (M:1 T:4)
        // -- M1
        case (0xAA<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xAA<<3)|1: z80_xor(cpu,cpu->d);_fetch(); break;
        
        // xor e (M:1 T:4)
        // -- M1
        case (0xAB<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xAB<<3)|1: z80_xor(cpu,cpu->e);_fetch(); break;
        
        // xor h (M:1 T:4)
        // -- M1
        case (0xAC<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xAC<<3)|1: z80_xor(cpu,cpu->h);_fetch(); break;
        
        // xor l (M:1 T:4)
        // -- M1
        case (0xAD<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xAD<<3)|1: z80_xor(cpu,cpu->l);_fetch(); break;
        
        // xor (hl) (M:2 T:7)
        // -- M1
        case (0xAE<<3)|0: _rfsh(); break;
        // -- M2
        case (0xAE<<3)|1: _mr(_ghl()); break;
        case (0xAE<<3)|2: cpu->dlatch=_gd(); break;
        // -- OVERLAP
        case (0xAE<<3)|3: z80_xor(cpu,cpu->dlatch);_fetch(); break;
        
        // xor a (M:1 T:4)
        // -- M1
        case (0xAF<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xAF<<3)|1: z80_xor(cpu,cpu->a);_fetch(); break;
        
        // or b (M:1 T:4)
        // -- M1
        case (0xB0<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xB0<<3)|1: z80_or(cpu,cpu->b);_fetch(); break;
        
        // or c (M:1 T:4)
        // -- M1
        case (0xB1<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xB1<<3)|1: z80_or(cpu,cpu->c);_fetch(); break;
        
        // or d (M:1 T:4)
        // -- M1
        case (0xB2<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xB2<<3)|1: z80_or(cpu,cpu->d);_fetch(); break;
        
        // or e (M:1 T:4)
        // -- M1
        case (0xB3<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xB3<<3)|1: z80_or(cpu,cpu->e);_fetch(); break;
        
        // or h (M:1 T:4)
        // -- M1
        case (0xB4<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xB4<<3)|1: z80_or(cpu,cpu->h);_fetch(); break;
        
        // or l (M:1 T:4)
        // -- M1
        case (0xB5<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xB5<<3)|1: z80_or(cpu,cpu->l);_fetch(); break;
        
        // or (hl) (M:2 T:7)
        // -- M1
        case (0xB6<<3)|0: _rfsh(); break;
        // -- M2
        case (0xB6<<3)|1: _mr(_ghl()); break;
        case (0xB6<<3)|2: cpu->dlatch=_gd(); break;
        // -- OVERLAP
        case (0xB6<<3)|3: z80_or(cpu,cpu->dlatch);_fetch(); break;
        
        // or a (M:1 T:4)
        // -- M1
        case (0xB7<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xB7<<3)|1: z80_or(cpu,cpu->a);_fetch(); break;
        
        // cp b (M:1 T:4)
        // -- M1
        case (0xB8<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xB8<<3)|1: z80_cp(cpu,cpu->b);_fetch(); break;
        
        // cp c (M:1 T:4)
        // -- M1
        case (0xB9<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xB9<<3)|1: z80_cp(cpu,cpu->c);_fetch(); break;
        
        // cp d (M:1 T:4)
        // -- M1
        case (0xBA<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xBA<<3)|1: z80_cp(cpu,cpu->d);_fetch(); break;
        
        // cp e (M:1 T:4)
        // -- M1
        case (0xBB<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xBB<<3)|1: z80_cp(cpu,cpu->e);_fetch(); break;
        
        // cp h (M:1 T:4)
        // -- M1
        case (0xBC<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xBC<<3)|1: z80_cp(cpu,cpu->h);_fetch(); break;
        
        // cp l (M:1 T:4)
        // -- M1
        case (0xBD<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xBD<<3)|1: z80_cp(cpu,cpu->l);_fetch(); break;
        
        // cp (hl) (M:2 T:7)
        // -- M1
        case (0xBE<<3)|0: _rfsh(); break;
        // -- M2
        case (0xBE<<3)|1: _mr(_ghl()); break;
        case (0xBE<<3)|2: cpu->dlatch=_gd(); break;
        // -- OVERLAP
        case (0xBE<<3)|3: z80_cp(cpu,cpu->dlatch);_fetch(); break;
        
        // cp a (M:1 T:4)
        // -- M1
        case (0xBF<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xBF<<3)|1: z80_cp(cpu,cpu->a);_fetch(); break;
        
        // ret nz (M:1 T:4)
        // -- M1
        case (0xC0<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xC0<<3)|1: _fetch(); break;
        
        // pop bc2 (M:1 T:4)
        // -- M1
        case (0xC1<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xC1<<3)|1: _fetch(); break;
        
        // jp nz,nn (M:1 T:4)
        // -- M1
        case (0xC2<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xC2<<3)|1: _fetch(); break;
        
        // jp nn (M:1 T:4)
        // -- M1
        case (0xC3<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xC3<<3)|1: _fetch(); break;
        
        // call nz,nn (M:1 T:4)
        // -- M1
        case (0xC4<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xC4<<3)|1: _fetch(); break;
        
        // push bc2 (M:1 T:4)
        // -- M1
        case (0xC5<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xC5<<3)|1: _fetch(); break;
        
        // add n (M:1 T:4)
        // -- M1
        case (0xC6<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xC6<<3)|1: _fetch(); break;
        
        // rst 0h (M:1 T:4)
        // -- M1
        case (0xC7<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xC7<<3)|1: _fetch(); break;
        
        // ret z (M:1 T:4)
        // -- M1
        case (0xC8<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xC8<<3)|1: _fetch(); break;
        
        // ret (M:1 T:4)
        // -- M1
        case (0xC9<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xC9<<3)|1: _fetch(); break;
        
        // jp z,nn (M:1 T:4)
        // -- M1
        case (0xCA<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xCA<<3)|1: _fetch(); break;
        
        // cb prefix (M:1 T:4)
        // -- M1
        case (0xCB<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xCB<<3)|1: _fetch(); break;
        
        // call z,nn (M:1 T:4)
        // -- M1
        case (0xCC<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xCC<<3)|1: _fetch(); break;
        
        // call nn (M:1 T:4)
        // -- M1
        case (0xCD<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xCD<<3)|1: _fetch(); break;
        
        // adc n (M:1 T:4)
        // -- M1
        case (0xCE<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xCE<<3)|1: _fetch(); break;
        
        // rst 8h (M:1 T:4)
        // -- M1
        case (0xCF<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xCF<<3)|1: _fetch(); break;
        
        // ret nc (M:1 T:4)
        // -- M1
        case (0xD0<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xD0<<3)|1: _fetch(); break;
        
        // pop de2 (M:1 T:4)
        // -- M1
        case (0xD1<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xD1<<3)|1: _fetch(); break;
        
        // jp nc,nn (M:1 T:4)
        // -- M1
        case (0xD2<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xD2<<3)|1: _fetch(); break;
        
        // out (n),a (M:1 T:4)
        // -- M1
        case (0xD3<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xD3<<3)|1: _fetch(); break;
        
        // call nc,nn (M:1 T:4)
        // -- M1
        case (0xD4<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xD4<<3)|1: _fetch(); break;
        
        // push de2 (M:1 T:4)
        // -- M1
        case (0xD5<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xD5<<3)|1: _fetch(); break;
        
        // sub n (M:1 T:4)
        // -- M1
        case (0xD6<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xD6<<3)|1: _fetch(); break;
        
        // rst 10h (M:1 T:4)
        // -- M1
        case (0xD7<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xD7<<3)|1: _fetch(); break;
        
        // ret c (M:1 T:4)
        // -- M1
        case (0xD8<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xD8<<3)|1: _fetch(); break;
        
        // exx (M:1 T:4)
        // -- M1
        case (0xD9<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xD9<<3)|1: _fetch(); break;
        
        // jp c,nn (M:1 T:4)
        // -- M1
        case (0xDA<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xDA<<3)|1: _fetch(); break;
        
        // in a,(n) (M:1 T:4)
        // -- M1
        case (0xDB<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xDB<<3)|1: _fetch(); break;
        
        // call c,nn (M:1 T:4)
        // -- M1
        case (0xDC<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xDC<<3)|1: _fetch(); break;
        
        // dd prefix (M:1 T:4)
        // -- M1
        case (0xDD<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xDD<<3)|1: _fetch(); break;
        
        // sbc n (M:1 T:4)
        // -- M1
        case (0xDE<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xDE<<3)|1: _fetch(); break;
        
        // rst 18h (M:1 T:4)
        // -- M1
        case (0xDF<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xDF<<3)|1: _fetch(); break;
        
        // ret po (M:1 T:4)
        // -- M1
        case (0xE0<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xE0<<3)|1: _fetch(); break;
        
        // pop hl2 (M:1 T:4)
        // -- M1
        case (0xE1<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xE1<<3)|1: _fetch(); break;
        
        // jp po,nn (M:1 T:4)
        // -- M1
        case (0xE2<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xE2<<3)|1: _fetch(); break;
        
        // ex (sp),hl (M:1 T:4)
        // -- M1
        case (0xE3<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xE3<<3)|1: _fetch(); break;
        
        // call po,nn (M:1 T:4)
        // -- M1
        case (0xE4<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xE4<<3)|1: _fetch(); break;
        
        // push hl2 (M:1 T:4)
        // -- M1
        case (0xE5<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xE5<<3)|1: _fetch(); break;
        
        // and n (M:1 T:4)
        // -- M1
        case (0xE6<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xE6<<3)|1: _fetch(); break;
        
        // rst 20h (M:1 T:4)
        // -- M1
        case (0xE7<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xE7<<3)|1: _fetch(); break;
        
        // ret pe (M:1 T:4)
        // -- M1
        case (0xE8<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xE8<<3)|1: _fetch(); break;
        
        // jp hl (M:1 T:4)
        // -- M1
        case (0xE9<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xE9<<3)|1: _fetch(); break;
        
        // jp pe,nn (M:1 T:4)
        // -- M1
        case (0xEA<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xEA<<3)|1: _fetch(); break;
        
        // ex de,hl (M:1 T:4)
        // -- M1
        case (0xEB<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xEB<<3)|1: _fetch(); break;
        
        // call pe,nn (M:1 T:4)
        // -- M1
        case (0xEC<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xEC<<3)|1: _fetch(); break;
        
        // ed prefix (M:1 T:4)
        // -- M1
        case (0xED<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xED<<3)|1: _fetch(); break;
        
        // xor n (M:1 T:4)
        // -- M1
        case (0xEE<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xEE<<3)|1: _fetch(); break;
        
        // rst 28h (M:1 T:4)
        // -- M1
        case (0xEF<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xEF<<3)|1: _fetch(); break;
        
        // ret p (M:1 T:4)
        // -- M1
        case (0xF0<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xF0<<3)|1: _fetch(); break;
        
        // pop sp2 (M:1 T:4)
        // -- M1
        case (0xF1<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xF1<<3)|1: _fetch(); break;
        
        // jp p,nn (M:1 T:4)
        // -- M1
        case (0xF2<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xF2<<3)|1: _fetch(); break;
        
        // di (M:1 T:4)
        // -- M1
        case (0xF3<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xF3<<3)|1: _fetch(); break;
        
        // call p,nn (M:1 T:4)
        // -- M1
        case (0xF4<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xF4<<3)|1: _fetch(); break;
        
        // push sp2 (M:1 T:4)
        // -- M1
        case (0xF5<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xF5<<3)|1: _fetch(); break;
        
        // or n (M:1 T:4)
        // -- M1
        case (0xF6<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xF6<<3)|1: _fetch(); break;
        
        // rst 30h (M:1 T:4)
        // -- M1
        case (0xF7<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xF7<<3)|1: _fetch(); break;
        
        // ret m (M:1 T:4)
        // -- M1
        case (0xF8<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xF8<<3)|1: _fetch(); break;
        
        // ld sp,hl (M:1 T:4)
        // -- M1
        case (0xF9<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xF9<<3)|1: _fetch(); break;
        
        // jp m,nn (M:1 T:4)
        // -- M1
        case (0xFA<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xFA<<3)|1: _fetch(); break;
        
        // ei (M:1 T:4)
        // -- M1
        case (0xFB<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xFB<<3)|1: _fetch(); break;
        
        // call m,nn (M:1 T:4)
        // -- M1
        case (0xFC<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xFC<<3)|1: _fetch(); break;
        
        // fd prefix (M:1 T:4)
        // -- M1
        case (0xFD<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xFD<<3)|1: _fetch(); break;
        
        // cp n (M:1 T:4)
        // -- M1
        case (0xFE<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xFE<<3)|1: _fetch(); break;
        
        // rst 38h (M:1 T:4)
        // -- M1
        case (0xFF<<3)|0: _rfsh(); break;
        // -- OVERLAP
        case (0xFF<<3)|1: _fetch(); break;

        }
    }
    cpu->pip = (pip & ~Z80_PIP_BITS) >> 1;
    cpu->pins = pins;
    return pins;
}

#undef _gaf
#undef _gbc
#undef _gde
#undef _ghl
#undef _gsp
#undef _saf
#undef _sbc
#undef _sde
#undef _shl
#undef _ssp
#undef _sa
#undef _sax
#undef _sad
#undef _sadx
#undef _gd
#undef _fetch
#undef _rfsh
#undef _mr
#undef _mw
#undef _ior
#undef _iow

#endif // CHIPS_IMPL