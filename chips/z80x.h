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

// pin access helper macros
#define Z80_GET_ADDR(p) ((uint16_t)(p&0xFFFF))
#define Z80_SET_ADDR(p,a) {p=(p&~0xFFFF)|((a)&0xFFFF);}
#define Z80_GET_DATA(p) ((uint8_t)((p>>16)&0xFF))
#define Z80_SET_DATA(p,d) {p=(p&~0xFF0000)|((d<<16)&0xFF0000);}

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

typedef struct {
    uint64_t pip;   // the op's decode pipeline
    uint64_t step;  // first or current decoder switch-case branch step
} z80_opstate_t;

// CPU state
typedef struct {
    uint64_t pins;      // last stored pin state
    z80_opstate_t op;   // the currently active op
    uint16_t pc;        // program counter
    uint8_t f, a, c, b, e, d, l, h;
    uint8_t dlatch;     // temporary store for data bus value
    uint16_t wz;
    uint16_t sp;
    uint16_t ix;
    uint16_t iy;
    uint8_t i;
    uint8_t r;
    uint8_t im;
    uint16_t af2, bc2, de2, hl2; // shadow register bank
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

uint64_t z80_init(z80_t* cpu) {
    CHIPS_ASSERT(cpu);
    memset(cpu, 0, sizeof(z80_t));
    // initial state according to visualz80
    cpu->f = cpu->a = cpu->c = cpu->b = 0x55;
    cpu->e = cpu->d = cpu->l = cpu->h = 0x55;
    cpu->wz = cpu->sp = cpu->ix = cpu->iy = 0x5555;
    cpu->af2 = cpu->bc2 = cpu->de2 = cpu->hl2 = 0x5555;
    // FIXME: iff1/2 disabled, initial value of IM???

    // return bit mask which causes the CPU to execute one
    // NOP in order to 'ignore' instruction processing
    return Z80_M1|Z80_MREQ|Z80_RD;
}

static inline void z80_halt(z80_t* cpu) {
    // FIXME
    (void)cpu;
}

static inline void z80_add(z80_t* cpu, uint8_t val) {
    // FIXME
    (void)cpu; (void)val;
}

static inline void z80_adc(z80_t* cpu, uint8_t val) {
    // FIXME
    (void)cpu; (void)val;
}

static inline void z80_sub(z80_t* cpu, uint8_t val) {
    // FIXME
    (void)cpu; (void)val;
}

static inline void z80_sbc(z80_t* cpu, uint8_t val) {
    // FIXME
    (void)cpu; (void)val;
}

static inline void z80_and(z80_t* cpu, uint8_t val) {
    // FIXME
    (void)cpu; (void)val;
}

static inline void z80_xor(z80_t* cpu, uint8_t val) {
    // FIXME
    (void)cpu; (void)val;
}

static inline void z80_or(z80_t* cpu, uint8_t val) {
    // FIXME
    (void)cpu; (void)val;
}

static inline void z80_cp(z80_t* cpu, uint8_t val) {
    // FIXME
    (void)cpu; (void)val;
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

static const z80_opstate_t z80_opstate_table[256] = {
    // nop (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0000 },
    // ld bc,nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0002 },
    // ld (bc),a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0004 },
    // inc bc (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0006 },
    // inc b (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0008 },
    // dec b (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x000A },
    // ld b,n (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x000C },
    // rlca (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0010 },
    // ex af,af' (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0012 },
    // add hl,bc (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0014 },
    // ld a,(bc) (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0016 },
    // dec bc (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0018 },
    // inc c (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x001A },
    // dec c (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x001C },
    // ld c,n (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x001E },
    // rrca (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0022 },
    // djnz d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0024 },
    // ld de,nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0026 },
    // ld (de),a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0028 },
    // inc de (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x002A },
    // inc d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x002C },
    // dec d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x002E },
    // ld d,n (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x0030 },
    // rla (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0034 },
    // jr d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0036 },
    // add hl,de (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0038 },
    // ld a,(de) (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x003A },
    // dec de (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x003C },
    // inc e (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x003E },
    // dec e (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0040 },
    // ld e,n (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x0042 },
    // rra (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0046 },
    // jr nz,d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0048 },
    // ld hl,nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x004A },
    // ld (nn),hl (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x004C },
    // inc hl (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x004E },
    // inc h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0050 },
    // dec h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0052 },
    // ld h,n (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x0054 },
    // daa (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0058 },
    // jr z,d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x005A },
    // add hl,hl (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x005C },
    // ld hl,(nn) (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x005E },
    // dec hl (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0060 },
    // inc l (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0062 },
    // dec l (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0064 },
    // ld l,n (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x0066 },
    // cpl (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x006A },
    // jr nc,d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x006C },
    // ld sp,nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x006E },
    // ld (nn),a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0070 },
    // inc sp (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0072 },
    // inc (hl) (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0074 },
    // dec (hl) (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0076 },
    // ld (HL),n (M:3 T:10 steps:5)
    { 0x000000910000029A, 0x0078 },
    // scf (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x007D },
    // jr c,d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x007F },
    // add hl,sp (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0081 },
    // ld a,(nn) (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0083 },
    // dec sp (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0085 },
    // inc a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0087 },
    // dec a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0089 },
    // ld a,n (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x008B },
    // ccf (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x008F },
    // ld b,b (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0091 },
    // ld c,b (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0093 },
    // ld d,b (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0095 },
    // ld e,b (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0097 },
    // ld h,b (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0099 },
    // ld l,b (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x009B },
    // ld b,(hl) (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x009D },
    // ld a,b (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00A1 },
    // ld b,c (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00A3 },
    // ld c,c (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00A5 },
    // ld d,c (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00A7 },
    // ld e,c (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00A9 },
    // ld h,c (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00AB },
    // ld l,c (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00AD },
    // ld c,(hl) (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x00AF },
    // ld a,c (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00B3 },
    // ld b,d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00B5 },
    // ld c,d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00B7 },
    // ld d,d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00B9 },
    // ld e,d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00BB },
    // ld h,d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00BD },
    // ld l,d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00BF },
    // ld d,(hl) (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x00C1 },
    // ld a,d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00C5 },
    // ld b,e (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00C7 },
    // ld c,e (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00C9 },
    // ld d,e (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00CB },
    // ld e,e (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00CD },
    // ld h,e (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00CF },
    // ld l,e (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00D1 },
    // ld e,(hl) (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x00D3 },
    // ld a,e (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00D7 },
    // ld b,h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00D9 },
    // ld c,h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00DB },
    // ld d,h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00DD },
    // ld e,h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00DF },
    // ld h,h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00E1 },
    // ld l,h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00E3 },
    // ld h,(hl) (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x00E5 },
    // ld a,h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00E9 },
    // ld b,l (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00EB },
    // ld c,l (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00ED },
    // ld d,l (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00EF },
    // ld e,l (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00F1 },
    // ld h,l (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00F3 },
    // ld l,l (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00F5 },
    // ld l,(hl) (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x00F7 },
    // ld a,l (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00FB },
    // ld (hl),b (M:2 T:7 steps:3)
    { 0x0000001100000052, 0x00FD },
    // ld (hl),c (M:2 T:7 steps:3)
    { 0x0000001100000052, 0x0100 },
    // ld (hl),d (M:2 T:7 steps:3)
    { 0x0000001100000052, 0x0103 },
    // ld (hl),e (M:2 T:7 steps:3)
    { 0x0000001100000052, 0x0106 },
    // ld (hl),h (M:2 T:7 steps:3)
    { 0x0000001100000052, 0x0109 },
    // ld (hl),l (M:2 T:7 steps:3)
    { 0x0000001100000052, 0x010C },
    // halt (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x010F },
    // ld (hl),a (M:2 T:7 steps:3)
    { 0x0000001100000052, 0x0111 },
    // ld b,a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0114 },
    // ld c,a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0116 },
    // ld d,a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0118 },
    // ld e,a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x011A },
    // ld h,a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x011C },
    // ld l,a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x011E },
    // ld a,(hl) (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x0120 },
    // ld a,a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0124 },
    // add b (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0126 },
    // add c (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0128 },
    // add d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x012A },
    // add e (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x012C },
    // add h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x012E },
    // add l (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0130 },
    // add (hl) (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x0132 },
    // add a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0136 },
    // adc b (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0138 },
    // adc c (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x013A },
    // adc d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x013C },
    // adc e (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x013E },
    // adc h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0140 },
    // adc l (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0142 },
    // adc (hl) (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x0144 },
    // adc a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0148 },
    // sub b (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x014A },
    // sub c (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x014C },
    // sub d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x014E },
    // sub e (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0150 },
    // sub h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0152 },
    // sub l (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0154 },
    // sub (hl) (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x0156 },
    // sub a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x015A },
    // sbc b (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x015C },
    // sbc c (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x015E },
    // sbc d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0160 },
    // sbc e (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0162 },
    // sbc h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0164 },
    // sbc l (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0166 },
    // sbc (hl) (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x0168 },
    // sbc a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x016C },
    // and b (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x016E },
    // and c (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0170 },
    // and d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0172 },
    // and e (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0174 },
    // and h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0176 },
    // and l (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0178 },
    // and (hl) (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x017A },
    // and a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x017E },
    // xor b (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0180 },
    // xor c (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0182 },
    // xor d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0184 },
    // xor e (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0186 },
    // xor h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0188 },
    // xor l (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x018A },
    // xor (hl) (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x018C },
    // xor a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0190 },
    // or b (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0192 },
    // or c (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0194 },
    // or d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0196 },
    // or e (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0198 },
    // or h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x019A },
    // or l (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x019C },
    // or (hl) (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x019E },
    // or a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01A2 },
    // cp b (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01A4 },
    // cp c (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01A6 },
    // cp d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01A8 },
    // cp e (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01AA },
    // cp h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01AC },
    // cp l (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01AE },
    // cp (hl) (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x01B0 },
    // cp a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01B4 },
    // ret nz (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01B6 },
    // pop bc2 (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01B8 },
    // jp nz,nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01BA },
    // jp nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01BC },
    // call nz,nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01BE },
    // push bc2 (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01C0 },
    // add n (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01C2 },
    // rst 0h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01C4 },
    // ret z (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01C6 },
    // ret (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01C8 },
    // jp z,nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01CA },
    // cb prefix (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01CC },
    // call z,nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01CE },
    // call nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01D0 },
    // adc n (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01D2 },
    // rst 8h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01D4 },
    // ret nc (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01D6 },
    // pop de2 (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01D8 },
    // jp nc,nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01DA },
    // out (n),a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01DC },
    // call nc,nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01DE },
    // push de2 (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01E0 },
    // sub n (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01E2 },
    // rst 10h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01E4 },
    // ret c (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01E6 },
    // exx (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01E8 },
    // jp c,nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01EA },
    // in a,(n) (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01EC },
    // call c,nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01EE },
    // dd prefix (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01F0 },
    // sbc n (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01F2 },
    // rst 18h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01F4 },
    // ret po (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01F6 },
    // pop hl2 (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01F8 },
    // jp po,nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01FA },
    // ex (sp),hl (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01FC },
    // call po,nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01FE },
    // push hl2 (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0200 },
    // and n (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0202 },
    // rst 20h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0204 },
    // ret pe (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0206 },
    // jp hl (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0208 },
    // jp pe,nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x020A },
    // ex de,hl (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x020C },
    // call pe,nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x020E },
    // ed prefix (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0210 },
    // xor n (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0212 },
    // rst 28h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0214 },
    // ret p (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0216 },
    // pop sp2 (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0218 },
    // jp p,nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x021A },
    // di (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x021C },
    // call p,nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x021E },
    // push sp2 (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0220 },
    // or n (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0222 },
    // rst 30h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0224 },
    // ret m (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0226 },
    // ld sp,hl (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0228 },
    // jp m,nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x022A },
    // ei (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x022C },
    // call m,nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x022E },
    // fd prefix (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0230 },
    // cp n (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0232 },
    // rst 38h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0234 },

};

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
#define _fetch()        _sax(cpu->pc++,Z80_M1|Z80_MREQ|Z80_RD)
#define _rfsh()         _sax(cpu->r,Z80_MREQ|Z80_RFSH);cpu->r=(cpu->r&0x80)|((cpu->r+1)&0x7F)
#define _mread(ab)      _sax(ab,Z80_MREQ|Z80_RD)
#define _mwrite(ab,d)   _sadx(ab,d,Z80_MREQ|Z80_WR)
#define _ioread(ab)     _sax(ab,Z80_IORQ|Z80_RD)
#define _iowrite(ab,d)  _sadx(ab,d,Z80_IORQ|Z80_WR)

uint64_t z80_tick(z80_t* cpu, uint64_t pins) {
    // wait cycle? (wait pin sampling only happens in specific tcycles)
    if ((cpu->op.pip & Z80_PIP_BIT_WAIT) && (pins & Z80_WAIT)) {
        cpu->pins = pins & ~Z80_CTRL_PIN_MASK;
        return pins;
    }
    // load next instruction opcode from data bus
    if ((pins & (Z80_M1|Z80_MREQ|Z80_RD)) == (Z80_M1|Z80_MREQ|Z80_RD)) {
        uint8_t opcode = _gd();
        cpu->op = z80_opstate_table[opcode];
    }
    // process the next active tcycle
    pins &= ~Z80_CTRL_PIN_MASK;
    if (cpu->op.pip & Z80_PIP_BIT_STEP) {
        switch (cpu->op.step++) {
            
            // nop (M:1 T:4)
            // -- M1
            case 0x0000: _rfsh(); break;
            // -- OVERLAP
            case 0x0001: _fetch(); break;
            
            // ld bc,nn (M:1 T:4)
            // -- M1
            case 0x0002: _rfsh(); break;
            // -- OVERLAP
            case 0x0003: _fetch(); break;
            
            // ld (bc),a (M:1 T:4)
            // -- M1
            case 0x0004: _rfsh(); break;
            // -- OVERLAP
            case 0x0005: _fetch(); break;
            
            // inc bc (M:1 T:4)
            // -- M1
            case 0x0006: _rfsh(); break;
            // -- OVERLAP
            case 0x0007: _fetch(); break;
            
            // inc b (M:1 T:4)
            // -- M1
            case 0x0008: _rfsh(); break;
            // -- OVERLAP
            case 0x0009: _fetch(); break;
            
            // dec b (M:1 T:4)
            // -- M1
            case 0x000A: _rfsh(); break;
            // -- OVERLAP
            case 0x000B: _fetch(); break;
            
            // ld b,n (M:2 T:7)
            // -- M1
            case 0x000C: _rfsh(); break;
            // -- M2
            case 0x000D: _mread(cpu->pc++); break;
            case 0x000E: cpu->b=_gd(); break;
            // -- OVERLAP
            case 0x000F: _fetch(); break;
            
            // rlca (M:1 T:4)
            // -- M1
            case 0x0010: _rfsh(); break;
            // -- OVERLAP
            case 0x0011: _fetch(); break;
            
            // ex af,af' (M:1 T:4)
            // -- M1
            case 0x0012: _rfsh(); break;
            // -- OVERLAP
            case 0x0013: _fetch(); break;
            
            // add hl,bc (M:1 T:4)
            // -- M1
            case 0x0014: _rfsh(); break;
            // -- OVERLAP
            case 0x0015: _fetch(); break;
            
            // ld a,(bc) (M:1 T:4)
            // -- M1
            case 0x0016: _rfsh(); break;
            // -- OVERLAP
            case 0x0017: _fetch(); break;
            
            // dec bc (M:1 T:4)
            // -- M1
            case 0x0018: _rfsh(); break;
            // -- OVERLAP
            case 0x0019: _fetch(); break;
            
            // inc c (M:1 T:4)
            // -- M1
            case 0x001A: _rfsh(); break;
            // -- OVERLAP
            case 0x001B: _fetch(); break;
            
            // dec c (M:1 T:4)
            // -- M1
            case 0x001C: _rfsh(); break;
            // -- OVERLAP
            case 0x001D: _fetch(); break;
            
            // ld c,n (M:2 T:7)
            // -- M1
            case 0x001E: _rfsh(); break;
            // -- M2
            case 0x001F: _mread(cpu->pc++); break;
            case 0x0020: cpu->c=_gd(); break;
            // -- OVERLAP
            case 0x0021: _fetch(); break;
            
            // rrca (M:1 T:4)
            // -- M1
            case 0x0022: _rfsh(); break;
            // -- OVERLAP
            case 0x0023: _fetch(); break;
            
            // djnz d (M:1 T:4)
            // -- M1
            case 0x0024: _rfsh(); break;
            // -- OVERLAP
            case 0x0025: _fetch(); break;
            
            // ld de,nn (M:1 T:4)
            // -- M1
            case 0x0026: _rfsh(); break;
            // -- OVERLAP
            case 0x0027: _fetch(); break;
            
            // ld (de),a (M:1 T:4)
            // -- M1
            case 0x0028: _rfsh(); break;
            // -- OVERLAP
            case 0x0029: _fetch(); break;
            
            // inc de (M:1 T:4)
            // -- M1
            case 0x002A: _rfsh(); break;
            // -- OVERLAP
            case 0x002B: _fetch(); break;
            
            // inc d (M:1 T:4)
            // -- M1
            case 0x002C: _rfsh(); break;
            // -- OVERLAP
            case 0x002D: _fetch(); break;
            
            // dec d (M:1 T:4)
            // -- M1
            case 0x002E: _rfsh(); break;
            // -- OVERLAP
            case 0x002F: _fetch(); break;
            
            // ld d,n (M:2 T:7)
            // -- M1
            case 0x0030: _rfsh(); break;
            // -- M2
            case 0x0031: _mread(cpu->pc++); break;
            case 0x0032: cpu->d=_gd(); break;
            // -- OVERLAP
            case 0x0033: _fetch(); break;
            
            // rla (M:1 T:4)
            // -- M1
            case 0x0034: _rfsh(); break;
            // -- OVERLAP
            case 0x0035: _fetch(); break;
            
            // jr d (M:1 T:4)
            // -- M1
            case 0x0036: _rfsh(); break;
            // -- OVERLAP
            case 0x0037: _fetch(); break;
            
            // add hl,de (M:1 T:4)
            // -- M1
            case 0x0038: _rfsh(); break;
            // -- OVERLAP
            case 0x0039: _fetch(); break;
            
            // ld a,(de) (M:1 T:4)
            // -- M1
            case 0x003A: _rfsh(); break;
            // -- OVERLAP
            case 0x003B: _fetch(); break;
            
            // dec de (M:1 T:4)
            // -- M1
            case 0x003C: _rfsh(); break;
            // -- OVERLAP
            case 0x003D: _fetch(); break;
            
            // inc e (M:1 T:4)
            // -- M1
            case 0x003E: _rfsh(); break;
            // -- OVERLAP
            case 0x003F: _fetch(); break;
            
            // dec e (M:1 T:4)
            // -- M1
            case 0x0040: _rfsh(); break;
            // -- OVERLAP
            case 0x0041: _fetch(); break;
            
            // ld e,n (M:2 T:7)
            // -- M1
            case 0x0042: _rfsh(); break;
            // -- M2
            case 0x0043: _mread(cpu->pc++); break;
            case 0x0044: cpu->e=_gd(); break;
            // -- OVERLAP
            case 0x0045: _fetch(); break;
            
            // rra (M:1 T:4)
            // -- M1
            case 0x0046: _rfsh(); break;
            // -- OVERLAP
            case 0x0047: _fetch(); break;
            
            // jr nz,d (M:1 T:4)
            // -- M1
            case 0x0048: _rfsh(); break;
            // -- OVERLAP
            case 0x0049: _fetch(); break;
            
            // ld hl,nn (M:1 T:4)
            // -- M1
            case 0x004A: _rfsh(); break;
            // -- OVERLAP
            case 0x004B: _fetch(); break;
            
            // ld (nn),hl (M:1 T:4)
            // -- M1
            case 0x004C: _rfsh(); break;
            // -- OVERLAP
            case 0x004D: _fetch(); break;
            
            // inc hl (M:1 T:4)
            // -- M1
            case 0x004E: _rfsh(); break;
            // -- OVERLAP
            case 0x004F: _fetch(); break;
            
            // inc h (M:1 T:4)
            // -- M1
            case 0x0050: _rfsh(); break;
            // -- OVERLAP
            case 0x0051: _fetch(); break;
            
            // dec h (M:1 T:4)
            // -- M1
            case 0x0052: _rfsh(); break;
            // -- OVERLAP
            case 0x0053: _fetch(); break;
            
            // ld h,n (M:2 T:7)
            // -- M1
            case 0x0054: _rfsh(); break;
            // -- M2
            case 0x0055: _mread(cpu->pc++); break;
            case 0x0056: cpu->h=_gd(); break;
            // -- OVERLAP
            case 0x0057: _fetch(); break;
            
            // daa (M:1 T:4)
            // -- M1
            case 0x0058: _rfsh(); break;
            // -- OVERLAP
            case 0x0059: _fetch(); break;
            
            // jr z,d (M:1 T:4)
            // -- M1
            case 0x005A: _rfsh(); break;
            // -- OVERLAP
            case 0x005B: _fetch(); break;
            
            // add hl,hl (M:1 T:4)
            // -- M1
            case 0x005C: _rfsh(); break;
            // -- OVERLAP
            case 0x005D: _fetch(); break;
            
            // ld hl,(nn) (M:1 T:4)
            // -- M1
            case 0x005E: _rfsh(); break;
            // -- OVERLAP
            case 0x005F: _fetch(); break;
            
            // dec hl (M:1 T:4)
            // -- M1
            case 0x0060: _rfsh(); break;
            // -- OVERLAP
            case 0x0061: _fetch(); break;
            
            // inc l (M:1 T:4)
            // -- M1
            case 0x0062: _rfsh(); break;
            // -- OVERLAP
            case 0x0063: _fetch(); break;
            
            // dec l (M:1 T:4)
            // -- M1
            case 0x0064: _rfsh(); break;
            // -- OVERLAP
            case 0x0065: _fetch(); break;
            
            // ld l,n (M:2 T:7)
            // -- M1
            case 0x0066: _rfsh(); break;
            // -- M2
            case 0x0067: _mread(cpu->pc++); break;
            case 0x0068: cpu->l=_gd(); break;
            // -- OVERLAP
            case 0x0069: _fetch(); break;
            
            // cpl (M:1 T:4)
            // -- M1
            case 0x006A: _rfsh(); break;
            // -- OVERLAP
            case 0x006B: _fetch(); break;
            
            // jr nc,d (M:1 T:4)
            // -- M1
            case 0x006C: _rfsh(); break;
            // -- OVERLAP
            case 0x006D: _fetch(); break;
            
            // ld sp,nn (M:1 T:4)
            // -- M1
            case 0x006E: _rfsh(); break;
            // -- OVERLAP
            case 0x006F: _fetch(); break;
            
            // ld (nn),a (M:1 T:4)
            // -- M1
            case 0x0070: _rfsh(); break;
            // -- OVERLAP
            case 0x0071: _fetch(); break;
            
            // inc sp (M:1 T:4)
            // -- M1
            case 0x0072: _rfsh(); break;
            // -- OVERLAP
            case 0x0073: _fetch(); break;
            
            // inc (hl) (M:1 T:4)
            // -- M1
            case 0x0074: _rfsh(); break;
            // -- OVERLAP
            case 0x0075: _fetch(); break;
            
            // dec (hl) (M:1 T:4)
            // -- M1
            case 0x0076: _rfsh(); break;
            // -- OVERLAP
            case 0x0077: _fetch(); break;
            
            // ld (HL),n (M:3 T:10)
            // -- M1
            case 0x0078: _rfsh(); break;
            // -- M2
            case 0x0079: _mread(cpu->pc++); break;
            case 0x007A: cpu->dlatch=_gd(); break;
            // -- M3
            case 0x007B: _mwrite(0xFFFF,0xFF)/*FIXME: address and data!*/; break;
            // -- OVERLAP
            case 0x007C: _fetch(); break;
            
            // scf (M:1 T:4)
            // -- M1
            case 0x007D: _rfsh(); break;
            // -- OVERLAP
            case 0x007E: _fetch(); break;
            
            // jr c,d (M:1 T:4)
            // -- M1
            case 0x007F: _rfsh(); break;
            // -- OVERLAP
            case 0x0080: _fetch(); break;
            
            // add hl,sp (M:1 T:4)
            // -- M1
            case 0x0081: _rfsh(); break;
            // -- OVERLAP
            case 0x0082: _fetch(); break;
            
            // ld a,(nn) (M:1 T:4)
            // -- M1
            case 0x0083: _rfsh(); break;
            // -- OVERLAP
            case 0x0084: _fetch(); break;
            
            // dec sp (M:1 T:4)
            // -- M1
            case 0x0085: _rfsh(); break;
            // -- OVERLAP
            case 0x0086: _fetch(); break;
            
            // inc a (M:1 T:4)
            // -- M1
            case 0x0087: _rfsh(); break;
            // -- OVERLAP
            case 0x0088: _fetch(); break;
            
            // dec a (M:1 T:4)
            // -- M1
            case 0x0089: _rfsh(); break;
            // -- OVERLAP
            case 0x008A: _fetch(); break;
            
            // ld a,n (M:2 T:7)
            // -- M1
            case 0x008B: _rfsh(); break;
            // -- M2
            case 0x008C: _mread(cpu->pc++); break;
            case 0x008D: cpu->a=_gd(); break;
            // -- OVERLAP
            case 0x008E: _fetch(); break;
            
            // ccf (M:1 T:4)
            // -- M1
            case 0x008F: _rfsh(); break;
            // -- OVERLAP
            case 0x0090: _fetch(); break;
            
            // ld b,b (M:1 T:4)
            // -- M1
            case 0x0091: _rfsh(); break;
            // -- OVERLAP
            case 0x0092: cpu->b=cpu->b;_fetch(); break;
            
            // ld c,b (M:1 T:4)
            // -- M1
            case 0x0093: _rfsh(); break;
            // -- OVERLAP
            case 0x0094: cpu->c=cpu->b;_fetch(); break;
            
            // ld d,b (M:1 T:4)
            // -- M1
            case 0x0095: _rfsh(); break;
            // -- OVERLAP
            case 0x0096: cpu->d=cpu->b;_fetch(); break;
            
            // ld e,b (M:1 T:4)
            // -- M1
            case 0x0097: _rfsh(); break;
            // -- OVERLAP
            case 0x0098: cpu->e=cpu->b;_fetch(); break;
            
            // ld h,b (M:1 T:4)
            // -- M1
            case 0x0099: _rfsh(); break;
            // -- OVERLAP
            case 0x009A: cpu->h=cpu->b;_fetch(); break;
            
            // ld l,b (M:1 T:4)
            // -- M1
            case 0x009B: _rfsh(); break;
            // -- OVERLAP
            case 0x009C: cpu->l=cpu->b;_fetch(); break;
            
            // ld b,(hl) (M:2 T:7)
            // -- M1
            case 0x009D: _rfsh(); break;
            // -- M2
            case 0x009E: _mread(_ghl()); break;
            case 0x009F: cpu->b=_gd(); break;
            // -- OVERLAP
            case 0x00A0: _fetch(); break;
            
            // ld a,b (M:1 T:4)
            // -- M1
            case 0x00A1: _rfsh(); break;
            // -- OVERLAP
            case 0x00A2: cpu->a=cpu->b;_fetch(); break;
            
            // ld b,c (M:1 T:4)
            // -- M1
            case 0x00A3: _rfsh(); break;
            // -- OVERLAP
            case 0x00A4: cpu->b=cpu->c;_fetch(); break;
            
            // ld c,c (M:1 T:4)
            // -- M1
            case 0x00A5: _rfsh(); break;
            // -- OVERLAP
            case 0x00A6: cpu->c=cpu->c;_fetch(); break;
            
            // ld d,c (M:1 T:4)
            // -- M1
            case 0x00A7: _rfsh(); break;
            // -- OVERLAP
            case 0x00A8: cpu->d=cpu->c;_fetch(); break;
            
            // ld e,c (M:1 T:4)
            // -- M1
            case 0x00A9: _rfsh(); break;
            // -- OVERLAP
            case 0x00AA: cpu->e=cpu->c;_fetch(); break;
            
            // ld h,c (M:1 T:4)
            // -- M1
            case 0x00AB: _rfsh(); break;
            // -- OVERLAP
            case 0x00AC: cpu->h=cpu->c;_fetch(); break;
            
            // ld l,c (M:1 T:4)
            // -- M1
            case 0x00AD: _rfsh(); break;
            // -- OVERLAP
            case 0x00AE: cpu->l=cpu->c;_fetch(); break;
            
            // ld c,(hl) (M:2 T:7)
            // -- M1
            case 0x00AF: _rfsh(); break;
            // -- M2
            case 0x00B0: _mread(_ghl()); break;
            case 0x00B1: cpu->c=_gd(); break;
            // -- OVERLAP
            case 0x00B2: _fetch(); break;
            
            // ld a,c (M:1 T:4)
            // -- M1
            case 0x00B3: _rfsh(); break;
            // -- OVERLAP
            case 0x00B4: cpu->a=cpu->c;_fetch(); break;
            
            // ld b,d (M:1 T:4)
            // -- M1
            case 0x00B5: _rfsh(); break;
            // -- OVERLAP
            case 0x00B6: cpu->b=cpu->d;_fetch(); break;
            
            // ld c,d (M:1 T:4)
            // -- M1
            case 0x00B7: _rfsh(); break;
            // -- OVERLAP
            case 0x00B8: cpu->c=cpu->d;_fetch(); break;
            
            // ld d,d (M:1 T:4)
            // -- M1
            case 0x00B9: _rfsh(); break;
            // -- OVERLAP
            case 0x00BA: cpu->d=cpu->d;_fetch(); break;
            
            // ld e,d (M:1 T:4)
            // -- M1
            case 0x00BB: _rfsh(); break;
            // -- OVERLAP
            case 0x00BC: cpu->e=cpu->d;_fetch(); break;
            
            // ld h,d (M:1 T:4)
            // -- M1
            case 0x00BD: _rfsh(); break;
            // -- OVERLAP
            case 0x00BE: cpu->h=cpu->d;_fetch(); break;
            
            // ld l,d (M:1 T:4)
            // -- M1
            case 0x00BF: _rfsh(); break;
            // -- OVERLAP
            case 0x00C0: cpu->l=cpu->d;_fetch(); break;
            
            // ld d,(hl) (M:2 T:7)
            // -- M1
            case 0x00C1: _rfsh(); break;
            // -- M2
            case 0x00C2: _mread(_ghl()); break;
            case 0x00C3: cpu->d=_gd(); break;
            // -- OVERLAP
            case 0x00C4: _fetch(); break;
            
            // ld a,d (M:1 T:4)
            // -- M1
            case 0x00C5: _rfsh(); break;
            // -- OVERLAP
            case 0x00C6: cpu->a=cpu->d;_fetch(); break;
            
            // ld b,e (M:1 T:4)
            // -- M1
            case 0x00C7: _rfsh(); break;
            // -- OVERLAP
            case 0x00C8: cpu->b=cpu->e;_fetch(); break;
            
            // ld c,e (M:1 T:4)
            // -- M1
            case 0x00C9: _rfsh(); break;
            // -- OVERLAP
            case 0x00CA: cpu->c=cpu->e;_fetch(); break;
            
            // ld d,e (M:1 T:4)
            // -- M1
            case 0x00CB: _rfsh(); break;
            // -- OVERLAP
            case 0x00CC: cpu->d=cpu->e;_fetch(); break;
            
            // ld e,e (M:1 T:4)
            // -- M1
            case 0x00CD: _rfsh(); break;
            // -- OVERLAP
            case 0x00CE: cpu->e=cpu->e;_fetch(); break;
            
            // ld h,e (M:1 T:4)
            // -- M1
            case 0x00CF: _rfsh(); break;
            // -- OVERLAP
            case 0x00D0: cpu->h=cpu->e;_fetch(); break;
            
            // ld l,e (M:1 T:4)
            // -- M1
            case 0x00D1: _rfsh(); break;
            // -- OVERLAP
            case 0x00D2: cpu->l=cpu->e;_fetch(); break;
            
            // ld e,(hl) (M:2 T:7)
            // -- M1
            case 0x00D3: _rfsh(); break;
            // -- M2
            case 0x00D4: _mread(_ghl()); break;
            case 0x00D5: cpu->e=_gd(); break;
            // -- OVERLAP
            case 0x00D6: _fetch(); break;
            
            // ld a,e (M:1 T:4)
            // -- M1
            case 0x00D7: _rfsh(); break;
            // -- OVERLAP
            case 0x00D8: cpu->a=cpu->e;_fetch(); break;
            
            // ld b,h (M:1 T:4)
            // -- M1
            case 0x00D9: _rfsh(); break;
            // -- OVERLAP
            case 0x00DA: cpu->b=cpu->h;_fetch(); break;
            
            // ld c,h (M:1 T:4)
            // -- M1
            case 0x00DB: _rfsh(); break;
            // -- OVERLAP
            case 0x00DC: cpu->c=cpu->h;_fetch(); break;
            
            // ld d,h (M:1 T:4)
            // -- M1
            case 0x00DD: _rfsh(); break;
            // -- OVERLAP
            case 0x00DE: cpu->d=cpu->h;_fetch(); break;
            
            // ld e,h (M:1 T:4)
            // -- M1
            case 0x00DF: _rfsh(); break;
            // -- OVERLAP
            case 0x00E0: cpu->e=cpu->h;_fetch(); break;
            
            // ld h,h (M:1 T:4)
            // -- M1
            case 0x00E1: _rfsh(); break;
            // -- OVERLAP
            case 0x00E2: cpu->h=cpu->h;_fetch(); break;
            
            // ld l,h (M:1 T:4)
            // -- M1
            case 0x00E3: _rfsh(); break;
            // -- OVERLAP
            case 0x00E4: cpu->l=cpu->h;_fetch(); break;
            
            // ld h,(hl) (M:2 T:7)
            // -- M1
            case 0x00E5: _rfsh(); break;
            // -- M2
            case 0x00E6: _mread(_ghl()); break;
            case 0x00E7: cpu->h=_gd(); break;
            // -- OVERLAP
            case 0x00E8: _fetch(); break;
            
            // ld a,h (M:1 T:4)
            // -- M1
            case 0x00E9: _rfsh(); break;
            // -- OVERLAP
            case 0x00EA: cpu->a=cpu->h;_fetch(); break;
            
            // ld b,l (M:1 T:4)
            // -- M1
            case 0x00EB: _rfsh(); break;
            // -- OVERLAP
            case 0x00EC: cpu->b=cpu->l;_fetch(); break;
            
            // ld c,l (M:1 T:4)
            // -- M1
            case 0x00ED: _rfsh(); break;
            // -- OVERLAP
            case 0x00EE: cpu->c=cpu->l;_fetch(); break;
            
            // ld d,l (M:1 T:4)
            // -- M1
            case 0x00EF: _rfsh(); break;
            // -- OVERLAP
            case 0x00F0: cpu->d=cpu->l;_fetch(); break;
            
            // ld e,l (M:1 T:4)
            // -- M1
            case 0x00F1: _rfsh(); break;
            // -- OVERLAP
            case 0x00F2: cpu->e=cpu->l;_fetch(); break;
            
            // ld h,l (M:1 T:4)
            // -- M1
            case 0x00F3: _rfsh(); break;
            // -- OVERLAP
            case 0x00F4: cpu->h=cpu->l;_fetch(); break;
            
            // ld l,l (M:1 T:4)
            // -- M1
            case 0x00F5: _rfsh(); break;
            // -- OVERLAP
            case 0x00F6: cpu->l=cpu->l;_fetch(); break;
            
            // ld l,(hl) (M:2 T:7)
            // -- M1
            case 0x00F7: _rfsh(); break;
            // -- M2
            case 0x00F8: _mread(_ghl()); break;
            case 0x00F9: cpu->l=_gd(); break;
            // -- OVERLAP
            case 0x00FA: _fetch(); break;
            
            // ld a,l (M:1 T:4)
            // -- M1
            case 0x00FB: _rfsh(); break;
            // -- OVERLAP
            case 0x00FC: cpu->a=cpu->l;_fetch(); break;
            
            // ld (hl),b (M:2 T:7)
            // -- M1
            case 0x00FD: _rfsh(); break;
            // -- M2
            case 0x00FE: _mwrite(0xFFFF,0xFF)/*FIXME: address and data!*/; break;
            // -- OVERLAP
            case 0x00FF: _fetch(); break;
            
            // ld (hl),c (M:2 T:7)
            // -- M1
            case 0x0100: _rfsh(); break;
            // -- M2
            case 0x0101: _mwrite(0xFFFF,0xFF)/*FIXME: address and data!*/; break;
            // -- OVERLAP
            case 0x0102: _fetch(); break;
            
            // ld (hl),d (M:2 T:7)
            // -- M1
            case 0x0103: _rfsh(); break;
            // -- M2
            case 0x0104: _mwrite(0xFFFF,0xFF)/*FIXME: address and data!*/; break;
            // -- OVERLAP
            case 0x0105: _fetch(); break;
            
            // ld (hl),e (M:2 T:7)
            // -- M1
            case 0x0106: _rfsh(); break;
            // -- M2
            case 0x0107: _mwrite(0xFFFF,0xFF)/*FIXME: address and data!*/; break;
            // -- OVERLAP
            case 0x0108: _fetch(); break;
            
            // ld (hl),h (M:2 T:7)
            // -- M1
            case 0x0109: _rfsh(); break;
            // -- M2
            case 0x010A: _mwrite(0xFFFF,0xFF)/*FIXME: address and data!*/; break;
            // -- OVERLAP
            case 0x010B: _fetch(); break;
            
            // ld (hl),l (M:2 T:7)
            // -- M1
            case 0x010C: _rfsh(); break;
            // -- M2
            case 0x010D: _mwrite(0xFFFF,0xFF)/*FIXME: address and data!*/; break;
            // -- OVERLAP
            case 0x010E: _fetch(); break;
            
            // halt (M:1 T:4)
            // -- M1
            case 0x010F: _rfsh(); break;
            // -- OVERLAP
            case 0x0110: z80_halt(cpu);_fetch(); break;
            
            // ld (hl),a (M:2 T:7)
            // -- M1
            case 0x0111: _rfsh(); break;
            // -- M2
            case 0x0112: _mwrite(0xFFFF,0xFF)/*FIXME: address and data!*/; break;
            // -- OVERLAP
            case 0x0113: _fetch(); break;
            
            // ld b,a (M:1 T:4)
            // -- M1
            case 0x0114: _rfsh(); break;
            // -- OVERLAP
            case 0x0115: cpu->b=cpu->a;_fetch(); break;
            
            // ld c,a (M:1 T:4)
            // -- M1
            case 0x0116: _rfsh(); break;
            // -- OVERLAP
            case 0x0117: cpu->c=cpu->a;_fetch(); break;
            
            // ld d,a (M:1 T:4)
            // -- M1
            case 0x0118: _rfsh(); break;
            // -- OVERLAP
            case 0x0119: cpu->d=cpu->a;_fetch(); break;
            
            // ld e,a (M:1 T:4)
            // -- M1
            case 0x011A: _rfsh(); break;
            // -- OVERLAP
            case 0x011B: cpu->e=cpu->a;_fetch(); break;
            
            // ld h,a (M:1 T:4)
            // -- M1
            case 0x011C: _rfsh(); break;
            // -- OVERLAP
            case 0x011D: cpu->h=cpu->a;_fetch(); break;
            
            // ld l,a (M:1 T:4)
            // -- M1
            case 0x011E: _rfsh(); break;
            // -- OVERLAP
            case 0x011F: cpu->l=cpu->a;_fetch(); break;
            
            // ld a,(hl) (M:2 T:7)
            // -- M1
            case 0x0120: _rfsh(); break;
            // -- M2
            case 0x0121: _mread(_ghl()); break;
            case 0x0122: cpu->a=_gd(); break;
            // -- OVERLAP
            case 0x0123: _fetch(); break;
            
            // ld a,a (M:1 T:4)
            // -- M1
            case 0x0124: _rfsh(); break;
            // -- OVERLAP
            case 0x0125: cpu->a=cpu->a;_fetch(); break;
            
            // add b (M:1 T:4)
            // -- M1
            case 0x0126: _rfsh(); break;
            // -- OVERLAP
            case 0x0127: z80_add(cpu,cpu->b);_fetch(); break;
            
            // add c (M:1 T:4)
            // -- M1
            case 0x0128: _rfsh(); break;
            // -- OVERLAP
            case 0x0129: z80_add(cpu,cpu->c);_fetch(); break;
            
            // add d (M:1 T:4)
            // -- M1
            case 0x012A: _rfsh(); break;
            // -- OVERLAP
            case 0x012B: z80_add(cpu,cpu->d);_fetch(); break;
            
            // add e (M:1 T:4)
            // -- M1
            case 0x012C: _rfsh(); break;
            // -- OVERLAP
            case 0x012D: z80_add(cpu,cpu->e);_fetch(); break;
            
            // add h (M:1 T:4)
            // -- M1
            case 0x012E: _rfsh(); break;
            // -- OVERLAP
            case 0x012F: z80_add(cpu,cpu->h);_fetch(); break;
            
            // add l (M:1 T:4)
            // -- M1
            case 0x0130: _rfsh(); break;
            // -- OVERLAP
            case 0x0131: z80_add(cpu,cpu->l);_fetch(); break;
            
            // add (hl) (M:2 T:7)
            // -- M1
            case 0x0132: _rfsh(); break;
            // -- M2
            case 0x0133: _mread(_ghl()); break;
            case 0x0134: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x0135: z80_add(cpu,cpu->dlatch);_fetch(); break;
            
            // add a (M:1 T:4)
            // -- M1
            case 0x0136: _rfsh(); break;
            // -- OVERLAP
            case 0x0137: z80_add(cpu,cpu->a);_fetch(); break;
            
            // adc b (M:1 T:4)
            // -- M1
            case 0x0138: _rfsh(); break;
            // -- OVERLAP
            case 0x0139: z80_adc(cpu,cpu->b);_fetch(); break;
            
            // adc c (M:1 T:4)
            // -- M1
            case 0x013A: _rfsh(); break;
            // -- OVERLAP
            case 0x013B: z80_adc(cpu,cpu->c);_fetch(); break;
            
            // adc d (M:1 T:4)
            // -- M1
            case 0x013C: _rfsh(); break;
            // -- OVERLAP
            case 0x013D: z80_adc(cpu,cpu->d);_fetch(); break;
            
            // adc e (M:1 T:4)
            // -- M1
            case 0x013E: _rfsh(); break;
            // -- OVERLAP
            case 0x013F: z80_adc(cpu,cpu->e);_fetch(); break;
            
            // adc h (M:1 T:4)
            // -- M1
            case 0x0140: _rfsh(); break;
            // -- OVERLAP
            case 0x0141: z80_adc(cpu,cpu->h);_fetch(); break;
            
            // adc l (M:1 T:4)
            // -- M1
            case 0x0142: _rfsh(); break;
            // -- OVERLAP
            case 0x0143: z80_adc(cpu,cpu->l);_fetch(); break;
            
            // adc (hl) (M:2 T:7)
            // -- M1
            case 0x0144: _rfsh(); break;
            // -- M2
            case 0x0145: _mread(_ghl()); break;
            case 0x0146: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x0147: z80_adc(cpu,cpu->dlatch);_fetch(); break;
            
            // adc a (M:1 T:4)
            // -- M1
            case 0x0148: _rfsh(); break;
            // -- OVERLAP
            case 0x0149: z80_adc(cpu,cpu->a);_fetch(); break;
            
            // sub b (M:1 T:4)
            // -- M1
            case 0x014A: _rfsh(); break;
            // -- OVERLAP
            case 0x014B: z80_sub(cpu,cpu->b);_fetch(); break;
            
            // sub c (M:1 T:4)
            // -- M1
            case 0x014C: _rfsh(); break;
            // -- OVERLAP
            case 0x014D: z80_sub(cpu,cpu->c);_fetch(); break;
            
            // sub d (M:1 T:4)
            // -- M1
            case 0x014E: _rfsh(); break;
            // -- OVERLAP
            case 0x014F: z80_sub(cpu,cpu->d);_fetch(); break;
            
            // sub e (M:1 T:4)
            // -- M1
            case 0x0150: _rfsh(); break;
            // -- OVERLAP
            case 0x0151: z80_sub(cpu,cpu->e);_fetch(); break;
            
            // sub h (M:1 T:4)
            // -- M1
            case 0x0152: _rfsh(); break;
            // -- OVERLAP
            case 0x0153: z80_sub(cpu,cpu->h);_fetch(); break;
            
            // sub l (M:1 T:4)
            // -- M1
            case 0x0154: _rfsh(); break;
            // -- OVERLAP
            case 0x0155: z80_sub(cpu,cpu->l);_fetch(); break;
            
            // sub (hl) (M:2 T:7)
            // -- M1
            case 0x0156: _rfsh(); break;
            // -- M2
            case 0x0157: _mread(_ghl()); break;
            case 0x0158: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x0159: z80_sub(cpu,cpu->dlatch);_fetch(); break;
            
            // sub a (M:1 T:4)
            // -- M1
            case 0x015A: _rfsh(); break;
            // -- OVERLAP
            case 0x015B: z80_sub(cpu,cpu->a);_fetch(); break;
            
            // sbc b (M:1 T:4)
            // -- M1
            case 0x015C: _rfsh(); break;
            // -- OVERLAP
            case 0x015D: z80_sbc(cpu,cpu->b);_fetch(); break;
            
            // sbc c (M:1 T:4)
            // -- M1
            case 0x015E: _rfsh(); break;
            // -- OVERLAP
            case 0x015F: z80_sbc(cpu,cpu->c);_fetch(); break;
            
            // sbc d (M:1 T:4)
            // -- M1
            case 0x0160: _rfsh(); break;
            // -- OVERLAP
            case 0x0161: z80_sbc(cpu,cpu->d);_fetch(); break;
            
            // sbc e (M:1 T:4)
            // -- M1
            case 0x0162: _rfsh(); break;
            // -- OVERLAP
            case 0x0163: z80_sbc(cpu,cpu->e);_fetch(); break;
            
            // sbc h (M:1 T:4)
            // -- M1
            case 0x0164: _rfsh(); break;
            // -- OVERLAP
            case 0x0165: z80_sbc(cpu,cpu->h);_fetch(); break;
            
            // sbc l (M:1 T:4)
            // -- M1
            case 0x0166: _rfsh(); break;
            // -- OVERLAP
            case 0x0167: z80_sbc(cpu,cpu->l);_fetch(); break;
            
            // sbc (hl) (M:2 T:7)
            // -- M1
            case 0x0168: _rfsh(); break;
            // -- M2
            case 0x0169: _mread(_ghl()); break;
            case 0x016A: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x016B: z80_sbc(cpu,cpu->dlatch);_fetch(); break;
            
            // sbc a (M:1 T:4)
            // -- M1
            case 0x016C: _rfsh(); break;
            // -- OVERLAP
            case 0x016D: z80_sbc(cpu,cpu->a);_fetch(); break;
            
            // and b (M:1 T:4)
            // -- M1
            case 0x016E: _rfsh(); break;
            // -- OVERLAP
            case 0x016F: z80_and(cpu,cpu->b);_fetch(); break;
            
            // and c (M:1 T:4)
            // -- M1
            case 0x0170: _rfsh(); break;
            // -- OVERLAP
            case 0x0171: z80_and(cpu,cpu->c);_fetch(); break;
            
            // and d (M:1 T:4)
            // -- M1
            case 0x0172: _rfsh(); break;
            // -- OVERLAP
            case 0x0173: z80_and(cpu,cpu->d);_fetch(); break;
            
            // and e (M:1 T:4)
            // -- M1
            case 0x0174: _rfsh(); break;
            // -- OVERLAP
            case 0x0175: z80_and(cpu,cpu->e);_fetch(); break;
            
            // and h (M:1 T:4)
            // -- M1
            case 0x0176: _rfsh(); break;
            // -- OVERLAP
            case 0x0177: z80_and(cpu,cpu->h);_fetch(); break;
            
            // and l (M:1 T:4)
            // -- M1
            case 0x0178: _rfsh(); break;
            // -- OVERLAP
            case 0x0179: z80_and(cpu,cpu->l);_fetch(); break;
            
            // and (hl) (M:2 T:7)
            // -- M1
            case 0x017A: _rfsh(); break;
            // -- M2
            case 0x017B: _mread(_ghl()); break;
            case 0x017C: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x017D: z80_and(cpu,cpu->dlatch);_fetch(); break;
            
            // and a (M:1 T:4)
            // -- M1
            case 0x017E: _rfsh(); break;
            // -- OVERLAP
            case 0x017F: z80_and(cpu,cpu->a);_fetch(); break;
            
            // xor b (M:1 T:4)
            // -- M1
            case 0x0180: _rfsh(); break;
            // -- OVERLAP
            case 0x0181: z80_xor(cpu,cpu->b);_fetch(); break;
            
            // xor c (M:1 T:4)
            // -- M1
            case 0x0182: _rfsh(); break;
            // -- OVERLAP
            case 0x0183: z80_xor(cpu,cpu->c);_fetch(); break;
            
            // xor d (M:1 T:4)
            // -- M1
            case 0x0184: _rfsh(); break;
            // -- OVERLAP
            case 0x0185: z80_xor(cpu,cpu->d);_fetch(); break;
            
            // xor e (M:1 T:4)
            // -- M1
            case 0x0186: _rfsh(); break;
            // -- OVERLAP
            case 0x0187: z80_xor(cpu,cpu->e);_fetch(); break;
            
            // xor h (M:1 T:4)
            // -- M1
            case 0x0188: _rfsh(); break;
            // -- OVERLAP
            case 0x0189: z80_xor(cpu,cpu->h);_fetch(); break;
            
            // xor l (M:1 T:4)
            // -- M1
            case 0x018A: _rfsh(); break;
            // -- OVERLAP
            case 0x018B: z80_xor(cpu,cpu->l);_fetch(); break;
            
            // xor (hl) (M:2 T:7)
            // -- M1
            case 0x018C: _rfsh(); break;
            // -- M2
            case 0x018D: _mread(_ghl()); break;
            case 0x018E: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x018F: z80_xor(cpu,cpu->dlatch);_fetch(); break;
            
            // xor a (M:1 T:4)
            // -- M1
            case 0x0190: _rfsh(); break;
            // -- OVERLAP
            case 0x0191: z80_xor(cpu,cpu->a);_fetch(); break;
            
            // or b (M:1 T:4)
            // -- M1
            case 0x0192: _rfsh(); break;
            // -- OVERLAP
            case 0x0193: z80_or(cpu,cpu->b);_fetch(); break;
            
            // or c (M:1 T:4)
            // -- M1
            case 0x0194: _rfsh(); break;
            // -- OVERLAP
            case 0x0195: z80_or(cpu,cpu->c);_fetch(); break;
            
            // or d (M:1 T:4)
            // -- M1
            case 0x0196: _rfsh(); break;
            // -- OVERLAP
            case 0x0197: z80_or(cpu,cpu->d);_fetch(); break;
            
            // or e (M:1 T:4)
            // -- M1
            case 0x0198: _rfsh(); break;
            // -- OVERLAP
            case 0x0199: z80_or(cpu,cpu->e);_fetch(); break;
            
            // or h (M:1 T:4)
            // -- M1
            case 0x019A: _rfsh(); break;
            // -- OVERLAP
            case 0x019B: z80_or(cpu,cpu->h);_fetch(); break;
            
            // or l (M:1 T:4)
            // -- M1
            case 0x019C: _rfsh(); break;
            // -- OVERLAP
            case 0x019D: z80_or(cpu,cpu->l);_fetch(); break;
            
            // or (hl) (M:2 T:7)
            // -- M1
            case 0x019E: _rfsh(); break;
            // -- M2
            case 0x019F: _mread(_ghl()); break;
            case 0x01A0: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x01A1: z80_or(cpu,cpu->dlatch);_fetch(); break;
            
            // or a (M:1 T:4)
            // -- M1
            case 0x01A2: _rfsh(); break;
            // -- OVERLAP
            case 0x01A3: z80_or(cpu,cpu->a);_fetch(); break;
            
            // cp b (M:1 T:4)
            // -- M1
            case 0x01A4: _rfsh(); break;
            // -- OVERLAP
            case 0x01A5: z80_cp(cpu,cpu->b);_fetch(); break;
            
            // cp c (M:1 T:4)
            // -- M1
            case 0x01A6: _rfsh(); break;
            // -- OVERLAP
            case 0x01A7: z80_cp(cpu,cpu->c);_fetch(); break;
            
            // cp d (M:1 T:4)
            // -- M1
            case 0x01A8: _rfsh(); break;
            // -- OVERLAP
            case 0x01A9: z80_cp(cpu,cpu->d);_fetch(); break;
            
            // cp e (M:1 T:4)
            // -- M1
            case 0x01AA: _rfsh(); break;
            // -- OVERLAP
            case 0x01AB: z80_cp(cpu,cpu->e);_fetch(); break;
            
            // cp h (M:1 T:4)
            // -- M1
            case 0x01AC: _rfsh(); break;
            // -- OVERLAP
            case 0x01AD: z80_cp(cpu,cpu->h);_fetch(); break;
            
            // cp l (M:1 T:4)
            // -- M1
            case 0x01AE: _rfsh(); break;
            // -- OVERLAP
            case 0x01AF: z80_cp(cpu,cpu->l);_fetch(); break;
            
            // cp (hl) (M:2 T:7)
            // -- M1
            case 0x01B0: _rfsh(); break;
            // -- M2
            case 0x01B1: _mread(_ghl()); break;
            case 0x01B2: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x01B3: z80_cp(cpu,cpu->dlatch);_fetch(); break;
            
            // cp a (M:1 T:4)
            // -- M1
            case 0x01B4: _rfsh(); break;
            // -- OVERLAP
            case 0x01B5: z80_cp(cpu,cpu->a);_fetch(); break;
            
            // ret nz (M:1 T:4)
            // -- M1
            case 0x01B6: _rfsh(); break;
            // -- OVERLAP
            case 0x01B7: _fetch(); break;
            
            // pop bc2 (M:1 T:4)
            // -- M1
            case 0x01B8: _rfsh(); break;
            // -- OVERLAP
            case 0x01B9: _fetch(); break;
            
            // jp nz,nn (M:1 T:4)
            // -- M1
            case 0x01BA: _rfsh(); break;
            // -- OVERLAP
            case 0x01BB: _fetch(); break;
            
            // jp nn (M:1 T:4)
            // -- M1
            case 0x01BC: _rfsh(); break;
            // -- OVERLAP
            case 0x01BD: _fetch(); break;
            
            // call nz,nn (M:1 T:4)
            // -- M1
            case 0x01BE: _rfsh(); break;
            // -- OVERLAP
            case 0x01BF: _fetch(); break;
            
            // push bc2 (M:1 T:4)
            // -- M1
            case 0x01C0: _rfsh(); break;
            // -- OVERLAP
            case 0x01C1: _fetch(); break;
            
            // add n (M:1 T:4)
            // -- M1
            case 0x01C2: _rfsh(); break;
            // -- OVERLAP
            case 0x01C3: _fetch(); break;
            
            // rst 0h (M:1 T:4)
            // -- M1
            case 0x01C4: _rfsh(); break;
            // -- OVERLAP
            case 0x01C5: _fetch(); break;
            
            // ret z (M:1 T:4)
            // -- M1
            case 0x01C6: _rfsh(); break;
            // -- OVERLAP
            case 0x01C7: _fetch(); break;
            
            // ret (M:1 T:4)
            // -- M1
            case 0x01C8: _rfsh(); break;
            // -- OVERLAP
            case 0x01C9: _fetch(); break;
            
            // jp z,nn (M:1 T:4)
            // -- M1
            case 0x01CA: _rfsh(); break;
            // -- OVERLAP
            case 0x01CB: _fetch(); break;
            
            // cb prefix (M:1 T:4)
            // -- M1
            case 0x01CC: _rfsh(); break;
            // -- OVERLAP
            case 0x01CD: _fetch(); break;
            
            // call z,nn (M:1 T:4)
            // -- M1
            case 0x01CE: _rfsh(); break;
            // -- OVERLAP
            case 0x01CF: _fetch(); break;
            
            // call nn (M:1 T:4)
            // -- M1
            case 0x01D0: _rfsh(); break;
            // -- OVERLAP
            case 0x01D1: _fetch(); break;
            
            // adc n (M:1 T:4)
            // -- M1
            case 0x01D2: _rfsh(); break;
            // -- OVERLAP
            case 0x01D3: _fetch(); break;
            
            // rst 8h (M:1 T:4)
            // -- M1
            case 0x01D4: _rfsh(); break;
            // -- OVERLAP
            case 0x01D5: _fetch(); break;
            
            // ret nc (M:1 T:4)
            // -- M1
            case 0x01D6: _rfsh(); break;
            // -- OVERLAP
            case 0x01D7: _fetch(); break;
            
            // pop de2 (M:1 T:4)
            // -- M1
            case 0x01D8: _rfsh(); break;
            // -- OVERLAP
            case 0x01D9: _fetch(); break;
            
            // jp nc,nn (M:1 T:4)
            // -- M1
            case 0x01DA: _rfsh(); break;
            // -- OVERLAP
            case 0x01DB: _fetch(); break;
            
            // out (n),a (M:1 T:4)
            // -- M1
            case 0x01DC: _rfsh(); break;
            // -- OVERLAP
            case 0x01DD: _fetch(); break;
            
            // call nc,nn (M:1 T:4)
            // -- M1
            case 0x01DE: _rfsh(); break;
            // -- OVERLAP
            case 0x01DF: _fetch(); break;
            
            // push de2 (M:1 T:4)
            // -- M1
            case 0x01E0: _rfsh(); break;
            // -- OVERLAP
            case 0x01E1: _fetch(); break;
            
            // sub n (M:1 T:4)
            // -- M1
            case 0x01E2: _rfsh(); break;
            // -- OVERLAP
            case 0x01E3: _fetch(); break;
            
            // rst 10h (M:1 T:4)
            // -- M1
            case 0x01E4: _rfsh(); break;
            // -- OVERLAP
            case 0x01E5: _fetch(); break;
            
            // ret c (M:1 T:4)
            // -- M1
            case 0x01E6: _rfsh(); break;
            // -- OVERLAP
            case 0x01E7: _fetch(); break;
            
            // exx (M:1 T:4)
            // -- M1
            case 0x01E8: _rfsh(); break;
            // -- OVERLAP
            case 0x01E9: _fetch(); break;
            
            // jp c,nn (M:1 T:4)
            // -- M1
            case 0x01EA: _rfsh(); break;
            // -- OVERLAP
            case 0x01EB: _fetch(); break;
            
            // in a,(n) (M:1 T:4)
            // -- M1
            case 0x01EC: _rfsh(); break;
            // -- OVERLAP
            case 0x01ED: _fetch(); break;
            
            // call c,nn (M:1 T:4)
            // -- M1
            case 0x01EE: _rfsh(); break;
            // -- OVERLAP
            case 0x01EF: _fetch(); break;
            
            // dd prefix (M:1 T:4)
            // -- M1
            case 0x01F0: _rfsh(); break;
            // -- OVERLAP
            case 0x01F1: _fetch(); break;
            
            // sbc n (M:1 T:4)
            // -- M1
            case 0x01F2: _rfsh(); break;
            // -- OVERLAP
            case 0x01F3: _fetch(); break;
            
            // rst 18h (M:1 T:4)
            // -- M1
            case 0x01F4: _rfsh(); break;
            // -- OVERLAP
            case 0x01F5: _fetch(); break;
            
            // ret po (M:1 T:4)
            // -- M1
            case 0x01F6: _rfsh(); break;
            // -- OVERLAP
            case 0x01F7: _fetch(); break;
            
            // pop hl2 (M:1 T:4)
            // -- M1
            case 0x01F8: _rfsh(); break;
            // -- OVERLAP
            case 0x01F9: _fetch(); break;
            
            // jp po,nn (M:1 T:4)
            // -- M1
            case 0x01FA: _rfsh(); break;
            // -- OVERLAP
            case 0x01FB: _fetch(); break;
            
            // ex (sp),hl (M:1 T:4)
            // -- M1
            case 0x01FC: _rfsh(); break;
            // -- OVERLAP
            case 0x01FD: _fetch(); break;
            
            // call po,nn (M:1 T:4)
            // -- M1
            case 0x01FE: _rfsh(); break;
            // -- OVERLAP
            case 0x01FF: _fetch(); break;
            
            // push hl2 (M:1 T:4)
            // -- M1
            case 0x0200: _rfsh(); break;
            // -- OVERLAP
            case 0x0201: _fetch(); break;
            
            // and n (M:1 T:4)
            // -- M1
            case 0x0202: _rfsh(); break;
            // -- OVERLAP
            case 0x0203: _fetch(); break;
            
            // rst 20h (M:1 T:4)
            // -- M1
            case 0x0204: _rfsh(); break;
            // -- OVERLAP
            case 0x0205: _fetch(); break;
            
            // ret pe (M:1 T:4)
            // -- M1
            case 0x0206: _rfsh(); break;
            // -- OVERLAP
            case 0x0207: _fetch(); break;
            
            // jp hl (M:1 T:4)
            // -- M1
            case 0x0208: _rfsh(); break;
            // -- OVERLAP
            case 0x0209: _fetch(); break;
            
            // jp pe,nn (M:1 T:4)
            // -- M1
            case 0x020A: _rfsh(); break;
            // -- OVERLAP
            case 0x020B: _fetch(); break;
            
            // ex de,hl (M:1 T:4)
            // -- M1
            case 0x020C: _rfsh(); break;
            // -- OVERLAP
            case 0x020D: _fetch(); break;
            
            // call pe,nn (M:1 T:4)
            // -- M1
            case 0x020E: _rfsh(); break;
            // -- OVERLAP
            case 0x020F: _fetch(); break;
            
            // ed prefix (M:1 T:4)
            // -- M1
            case 0x0210: _rfsh(); break;
            // -- OVERLAP
            case 0x0211: _fetch(); break;
            
            // xor n (M:1 T:4)
            // -- M1
            case 0x0212: _rfsh(); break;
            // -- OVERLAP
            case 0x0213: _fetch(); break;
            
            // rst 28h (M:1 T:4)
            // -- M1
            case 0x0214: _rfsh(); break;
            // -- OVERLAP
            case 0x0215: _fetch(); break;
            
            // ret p (M:1 T:4)
            // -- M1
            case 0x0216: _rfsh(); break;
            // -- OVERLAP
            case 0x0217: _fetch(); break;
            
            // pop sp2 (M:1 T:4)
            // -- M1
            case 0x0218: _rfsh(); break;
            // -- OVERLAP
            case 0x0219: _fetch(); break;
            
            // jp p,nn (M:1 T:4)
            // -- M1
            case 0x021A: _rfsh(); break;
            // -- OVERLAP
            case 0x021B: _fetch(); break;
            
            // di (M:1 T:4)
            // -- M1
            case 0x021C: _rfsh(); break;
            // -- OVERLAP
            case 0x021D: _fetch(); break;
            
            // call p,nn (M:1 T:4)
            // -- M1
            case 0x021E: _rfsh(); break;
            // -- OVERLAP
            case 0x021F: _fetch(); break;
            
            // push sp2 (M:1 T:4)
            // -- M1
            case 0x0220: _rfsh(); break;
            // -- OVERLAP
            case 0x0221: _fetch(); break;
            
            // or n (M:1 T:4)
            // -- M1
            case 0x0222: _rfsh(); break;
            // -- OVERLAP
            case 0x0223: _fetch(); break;
            
            // rst 30h (M:1 T:4)
            // -- M1
            case 0x0224: _rfsh(); break;
            // -- OVERLAP
            case 0x0225: _fetch(); break;
            
            // ret m (M:1 T:4)
            // -- M1
            case 0x0226: _rfsh(); break;
            // -- OVERLAP
            case 0x0227: _fetch(); break;
            
            // ld sp,hl (M:1 T:4)
            // -- M1
            case 0x0228: _rfsh(); break;
            // -- OVERLAP
            case 0x0229: _fetch(); break;
            
            // jp m,nn (M:1 T:4)
            // -- M1
            case 0x022A: _rfsh(); break;
            // -- OVERLAP
            case 0x022B: _fetch(); break;
            
            // ei (M:1 T:4)
            // -- M1
            case 0x022C: _rfsh(); break;
            // -- OVERLAP
            case 0x022D: _fetch(); break;
            
            // call m,nn (M:1 T:4)
            // -- M1
            case 0x022E: _rfsh(); break;
            // -- OVERLAP
            case 0x022F: _fetch(); break;
            
            // fd prefix (M:1 T:4)
            // -- M1
            case 0x0230: _rfsh(); break;
            // -- OVERLAP
            case 0x0231: _fetch(); break;
            
            // cp n (M:1 T:4)
            // -- M1
            case 0x0232: _rfsh(); break;
            // -- OVERLAP
            case 0x0233: _fetch(); break;
            
            // rst 38h (M:1 T:4)
            // -- M1
            case 0x0234: _rfsh(); break;
            // -- OVERLAP
            case 0x0235: _fetch(); break;

        }
    }
    // advance the decode pipeline by one tcycle
    cpu->op.pip = (cpu->op.pip & ~Z80_PIP_BITS) >> 1;
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
#undef _mread
#undef _mwrite
#undef _ioread
#undef _iowrite

#endif // CHIPS_IMPL