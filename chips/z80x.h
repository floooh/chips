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
    uint64_t step;      // current decoder step
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

typedef struct {
    uint64_t pip;   // decode pipeline
    uint64_t step;  // the first case-branch of the instruction
} z80_opdesc_t;

static const z80_opdesc_t z80_opdesc_table[256] = {
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
    // ld b,n (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x000C },
    // rlca (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x000E },
    // ex af,af' (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0010 },
    // add hl,bc (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0012 },
    // ld a,(bc) (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0014 },
    // dec bc (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0016 },
    // inc c (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0018 },
    // dec c (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x001A },
    // ld c,n (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x001C },
    // rrca (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x001E },
    // djnz d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0020 },
    // ld de,nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0022 },
    // ld (de),a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0024 },
    // inc de (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0026 },
    // inc d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0028 },
    // dec d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x002A },
    // ld d,n (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x002C },
    // rla (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x002E },
    // jr d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0030 },
    // add hl,de (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0032 },
    // ld a,(de) (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0034 },
    // dec de (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0036 },
    // inc e (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0038 },
    // dec e (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x003A },
    // ld e,n (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x003C },
    // rra (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x003E },
    // jr nz,d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0040 },
    // ld hl,nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0042 },
    // ld (nn),hl (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0044 },
    // inc hl (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0046 },
    // inc h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0048 },
    // dec h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x004A },
    // ld h,n (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x004C },
    // daa (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x004E },
    // jr z,d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0050 },
    // add hl,hl (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0052 },
    // ld hl,(nn) (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0054 },
    // dec hl (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0056 },
    // inc l (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0058 },
    // dec l (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x005A },
    // ld l,n (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x005C },
    // cpl (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x005E },
    // jr nc,d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0060 },
    // ld sp,nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0062 },
    // ld (nn),a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0064 },
    // inc sp (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0066 },
    // inc (hl) (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0068 },
    // dec (hl) (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x006A },
    // ld (hl),n (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x006C },
    // scf (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x006E },
    // jr c,d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0070 },
    // add hl,sp (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0072 },
    // ld a,(nn) (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0074 },
    // dec sp (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0076 },
    // inc a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0078 },
    // dec a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x007A },
    // ld a,n (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x007C },
    // ccf (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x007E },
    // ld b,b (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0080 },
    // ld c,b (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0082 },
    // ld d,b (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0084 },
    // ld e,b (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0086 },
    // ld h,b (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0088 },
    // ld l,b (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x008A },
    // ld b,(hl) (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x008C },
    // ld a,b (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0090 },
    // ld b,c (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0092 },
    // ld c,c (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0094 },
    // ld d,c (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0096 },
    // ld e,c (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0098 },
    // ld h,c (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x009A },
    // ld l,c (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x009C },
    // ld c,(hl) (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x009E },
    // ld a,c (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00A2 },
    // ld b,d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00A4 },
    // ld c,d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00A6 },
    // ld d,d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00A8 },
    // ld e,d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00AA },
    // ld h,d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00AC },
    // ld l,d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00AE },
    // ld d,(hl) (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x00B0 },
    // ld a,d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00B4 },
    // ld b,e (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00B6 },
    // ld c,e (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00B8 },
    // ld d,e (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00BA },
    // ld e,e (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00BC },
    // ld h,e (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00BE },
    // ld l,e (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00C0 },
    // ld e,(hl) (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x00C2 },
    // ld a,e (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00C6 },
    // ld b,h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00C8 },
    // ld c,h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00CA },
    // ld d,h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00CC },
    // ld e,h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00CE },
    // ld h,h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00D0 },
    // ld l,h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00D2 },
    // ld h,(hl) (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x00D4 },
    // ld a,h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00D8 },
    // ld b,l (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00DA },
    // ld c,l (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00DC },
    // ld d,l (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00DE },
    // ld e,l (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00E0 },
    // ld h,l (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00E2 },
    // ld l,l (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00E4 },
    // ld l,(hl) (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x00E6 },
    // ld a,l (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00EA },
    // ld (hl),b (M:2 T:7 steps:3)
    { 0x0000001100000052, 0x00EC },
    // ld (hl),c (M:2 T:7 steps:3)
    { 0x0000001100000052, 0x00EF },
    // ld (hl),d (M:2 T:7 steps:3)
    { 0x0000001100000052, 0x00F2 },
    // ld (hl),e (M:2 T:7 steps:3)
    { 0x0000001100000052, 0x00F5 },
    // ld (hl),h (M:2 T:7 steps:3)
    { 0x0000001100000052, 0x00F8 },
    // ld (hl),l (M:2 T:7 steps:3)
    { 0x0000001100000052, 0x00FB },
    // halt (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x00FE },
    // ld (hl),a (M:2 T:7 steps:3)
    { 0x0000001100000052, 0x0100 },
    // ld b,a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0103 },
    // ld c,a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0105 },
    // ld d,a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0107 },
    // ld e,a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0109 },
    // ld h,a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x010B },
    // ld l,a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x010D },
    // ld a,(hl) (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x010F },
    // ld a,a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0113 },
    // add b (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0115 },
    // add c (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0117 },
    // add d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0119 },
    // add e (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x011B },
    // add h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x011D },
    // add l (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x011F },
    // add (hl) (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x0121 },
    // add a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0125 },
    // adc b (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0127 },
    // adc c (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0129 },
    // adc d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x012B },
    // adc e (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x012D },
    // adc h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x012F },
    // adc l (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0131 },
    // adc (hl) (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x0133 },
    // adc a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0137 },
    // sub b (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0139 },
    // sub c (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x013B },
    // sub d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x013D },
    // sub e (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x013F },
    // sub h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0141 },
    // sub l (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0143 },
    // sub (hl) (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x0145 },
    // sub a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0149 },
    // sbc b (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x014B },
    // sbc c (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x014D },
    // sbc d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x014F },
    // sbc e (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0151 },
    // sbc h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0153 },
    // sbc l (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0155 },
    // sbc (hl) (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x0157 },
    // sbc a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x015B },
    // and b (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x015D },
    // and c (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x015F },
    // and d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0161 },
    // and e (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0163 },
    // and h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0165 },
    // and l (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0167 },
    // and (hl) (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x0169 },
    // and a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x016D },
    // xor b (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x016F },
    // xor c (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0171 },
    // xor d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0173 },
    // xor e (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0175 },
    // xor h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0177 },
    // xor l (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0179 },
    // xor (hl) (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x017B },
    // xor a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x017F },
    // or b (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0181 },
    // or c (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0183 },
    // or d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0185 },
    // or e (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0187 },
    // or h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0189 },
    // or l (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x018B },
    // or (hl) (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x018D },
    // or a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0191 },
    // cp b (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0193 },
    // cp c (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0195 },
    // cp d (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0197 },
    // cp e (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0199 },
    // cp h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x019B },
    // cp l (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x019D },
    // cp (hl) (M:2 T:7 steps:4)
    { 0x000000110000005A, 0x019F },
    // cp a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01A3 },
    // ret nz (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01A5 },
    // pop bc2 (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01A7 },
    // jp nz,nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01A9 },
    // jp nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01AB },
    // call nz,nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01AD },
    // push bc2 (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01AF },
    // add n (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01B1 },
    // rst 0h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01B3 },
    // ret z (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01B5 },
    // ret (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01B7 },
    // jp z,nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01B9 },
    // cb prefix (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01BB },
    // call z,nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01BD },
    // call nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01BF },
    // adc n (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01C1 },
    // rst 8h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01C3 },
    // ret nc (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01C5 },
    // pop de2 (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01C7 },
    // jp nc,nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01C9 },
    // out (n),a (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01CB },
    // call nc,nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01CD },
    // push de2 (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01CF },
    // sub n (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01D1 },
    // rst 10h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01D3 },
    // ret c (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01D5 },
    // exx (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01D7 },
    // jp c,nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01D9 },
    // in a,(n) (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01DB },
    // call c,nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01DD },
    // dd prefix (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01DF },
    // sbc n (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01E1 },
    // rst 18h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01E3 },
    // ret po (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01E5 },
    // pop hl2 (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01E7 },
    // jp po,nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01E9 },
    // ex (sp),hl (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01EB },
    // call po,nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01ED },
    // push hl2 (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01EF },
    // and n (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01F1 },
    // rst 20h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01F3 },
    // ret pe (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01F5 },
    // jp hl (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01F7 },
    // jp pe,nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01F9 },
    // ex de,hl (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01FB },
    // call pe,nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01FD },
    // ed prefix (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x01FF },
    // xor n (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0201 },
    // rst 28h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0203 },
    // ret p (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0205 },
    // pop sp2 (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0207 },
    // jp p,nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0209 },
    // di (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x020B },
    // call p,nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x020D },
    // push sp2 (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x020F },
    // or n (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0211 },
    // rst 30h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0213 },
    // ret m (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0215 },
    // ld sp,hl (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0217 },
    // jp m,nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0219 },
    // ei (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x021B },
    // call m,nn (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x021D },
    // fd prefix (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x021F },
    // cp n (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0221 },
    // rst 38h (M:1 T:4 steps:2)
    { 0x000000010000000A, 0x0223 },

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
        pip = z80_opdesc_table[opcode].pip;
        cpu->step = z80_opdesc_table[opcode].step;
    }
    // process the next 'active' tcycle
    pins &= ~Z80_CTRL_PIN_MASK;
    if (pip & Z80_PIP_BIT_STEP) {
        switch (cpu->step++) {
            
            // nop (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0000: _rfsh(); break;
            // -- OVERLAP
            case 0x0001: _fetch(); break;
            
            // ld bc,nn (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0002: _rfsh(); break;
            // -- OVERLAP
            case 0x0003: _fetch(); break;
            
            // ld (bc),a (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0004: _rfsh(); break;
            // -- OVERLAP
            case 0x0005: _fetch(); break;
            
            // inc bc (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0006: _rfsh(); break;
            // -- OVERLAP
            case 0x0007: _fetch(); break;
            
            // inc b (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0008: _rfsh(); break;
            // -- OVERLAP
            case 0x0009: _fetch(); break;
            
            // dec b (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x000A: _rfsh(); break;
            // -- OVERLAP
            case 0x000B: _fetch(); break;
            
            // ld b,n (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x000C: _rfsh(); break;
            // -- OVERLAP
            case 0x000D: _fetch(); break;
            
            // rlca (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x000E: _rfsh(); break;
            // -- OVERLAP
            case 0x000F: _fetch(); break;
            
            // ex af,af' (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0010: _rfsh(); break;
            // -- OVERLAP
            case 0x0011: _fetch(); break;
            
            // add hl,bc (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0012: _rfsh(); break;
            // -- OVERLAP
            case 0x0013: _fetch(); break;
            
            // ld a,(bc) (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0014: _rfsh(); break;
            // -- OVERLAP
            case 0x0015: _fetch(); break;
            
            // dec bc (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0016: _rfsh(); break;
            // -- OVERLAP
            case 0x0017: _fetch(); break;
            
            // inc c (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0018: _rfsh(); break;
            // -- OVERLAP
            case 0x0019: _fetch(); break;
            
            // dec c (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x001A: _rfsh(); break;
            // -- OVERLAP
            case 0x001B: _fetch(); break;
            
            // ld c,n (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x001C: _rfsh(); break;
            // -- OVERLAP
            case 0x001D: _fetch(); break;
            
            // rrca (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x001E: _rfsh(); break;
            // -- OVERLAP
            case 0x001F: _fetch(); break;
            
            // djnz d (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0020: _rfsh(); break;
            // -- OVERLAP
            case 0x0021: _fetch(); break;
            
            // ld de,nn (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0022: _rfsh(); break;
            // -- OVERLAP
            case 0x0023: _fetch(); break;
            
            // ld (de),a (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0024: _rfsh(); break;
            // -- OVERLAP
            case 0x0025: _fetch(); break;
            
            // inc de (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0026: _rfsh(); break;
            // -- OVERLAP
            case 0x0027: _fetch(); break;
            
            // inc d (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0028: _rfsh(); break;
            // -- OVERLAP
            case 0x0029: _fetch(); break;
            
            // dec d (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x002A: _rfsh(); break;
            // -- OVERLAP
            case 0x002B: _fetch(); break;
            
            // ld d,n (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x002C: _rfsh(); break;
            // -- OVERLAP
            case 0x002D: _fetch(); break;
            
            // rla (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x002E: _rfsh(); break;
            // -- OVERLAP
            case 0x002F: _fetch(); break;
            
            // jr d (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0030: _rfsh(); break;
            // -- OVERLAP
            case 0x0031: _fetch(); break;
            
            // add hl,de (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0032: _rfsh(); break;
            // -- OVERLAP
            case 0x0033: _fetch(); break;
            
            // ld a,(de) (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0034: _rfsh(); break;
            // -- OVERLAP
            case 0x0035: _fetch(); break;
            
            // dec de (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0036: _rfsh(); break;
            // -- OVERLAP
            case 0x0037: _fetch(); break;
            
            // inc e (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0038: _rfsh(); break;
            // -- OVERLAP
            case 0x0039: _fetch(); break;
            
            // dec e (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x003A: _rfsh(); break;
            // -- OVERLAP
            case 0x003B: _fetch(); break;
            
            // ld e,n (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x003C: _rfsh(); break;
            // -- OVERLAP
            case 0x003D: _fetch(); break;
            
            // rra (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x003E: _rfsh(); break;
            // -- OVERLAP
            case 0x003F: _fetch(); break;
            
            // jr nz,d (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0040: _rfsh(); break;
            // -- OVERLAP
            case 0x0041: _fetch(); break;
            
            // ld hl,nn (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0042: _rfsh(); break;
            // -- OVERLAP
            case 0x0043: _fetch(); break;
            
            // ld (nn),hl (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0044: _rfsh(); break;
            // -- OVERLAP
            case 0x0045: _fetch(); break;
            
            // inc hl (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0046: _rfsh(); break;
            // -- OVERLAP
            case 0x0047: _fetch(); break;
            
            // inc h (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0048: _rfsh(); break;
            // -- OVERLAP
            case 0x0049: _fetch(); break;
            
            // dec h (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x004A: _rfsh(); break;
            // -- OVERLAP
            case 0x004B: _fetch(); break;
            
            // ld h,n (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x004C: _rfsh(); break;
            // -- OVERLAP
            case 0x004D: _fetch(); break;
            
            // daa (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x004E: _rfsh(); break;
            // -- OVERLAP
            case 0x004F: _fetch(); break;
            
            // jr z,d (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0050: _rfsh(); break;
            // -- OVERLAP
            case 0x0051: _fetch(); break;
            
            // add hl,hl (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0052: _rfsh(); break;
            // -- OVERLAP
            case 0x0053: _fetch(); break;
            
            // ld hl,(nn) (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0054: _rfsh(); break;
            // -- OVERLAP
            case 0x0055: _fetch(); break;
            
            // dec hl (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0056: _rfsh(); break;
            // -- OVERLAP
            case 0x0057: _fetch(); break;
            
            // inc l (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0058: _rfsh(); break;
            // -- OVERLAP
            case 0x0059: _fetch(); break;
            
            // dec l (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x005A: _rfsh(); break;
            // -- OVERLAP
            case 0x005B: _fetch(); break;
            
            // ld l,n (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x005C: _rfsh(); break;
            // -- OVERLAP
            case 0x005D: _fetch(); break;
            
            // cpl (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x005E: _rfsh(); break;
            // -- OVERLAP
            case 0x005F: _fetch(); break;
            
            // jr nc,d (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0060: _rfsh(); break;
            // -- OVERLAP
            case 0x0061: _fetch(); break;
            
            // ld sp,nn (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0062: _rfsh(); break;
            // -- OVERLAP
            case 0x0063: _fetch(); break;
            
            // ld (nn),a (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0064: _rfsh(); break;
            // -- OVERLAP
            case 0x0065: _fetch(); break;
            
            // inc sp (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0066: _rfsh(); break;
            // -- OVERLAP
            case 0x0067: _fetch(); break;
            
            // inc (hl) (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0068: _rfsh(); break;
            // -- OVERLAP
            case 0x0069: _fetch(); break;
            
            // dec (hl) (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x006A: _rfsh(); break;
            // -- OVERLAP
            case 0x006B: _fetch(); break;
            
            // ld (hl),n (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x006C: _rfsh(); break;
            // -- OVERLAP
            case 0x006D: _fetch(); break;
            
            // scf (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x006E: _rfsh(); break;
            // -- OVERLAP
            case 0x006F: _fetch(); break;
            
            // jr c,d (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0070: _rfsh(); break;
            // -- OVERLAP
            case 0x0071: _fetch(); break;
            
            // add hl,sp (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0072: _rfsh(); break;
            // -- OVERLAP
            case 0x0073: _fetch(); break;
            
            // ld a,(nn) (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0074: _rfsh(); break;
            // -- OVERLAP
            case 0x0075: _fetch(); break;
            
            // dec sp (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0076: _rfsh(); break;
            // -- OVERLAP
            case 0x0077: _fetch(); break;
            
            // inc a (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0078: _rfsh(); break;
            // -- OVERLAP
            case 0x0079: _fetch(); break;
            
            // dec a (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x007A: _rfsh(); break;
            // -- OVERLAP
            case 0x007B: _fetch(); break;
            
            // ld a,n (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x007C: _rfsh(); break;
            // -- OVERLAP
            case 0x007D: _fetch(); break;
            
            // ccf (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x007E: _rfsh(); break;
            // -- OVERLAP
            case 0x007F: _fetch(); break;
            
            // ld b,b (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0080: _rfsh(); break;
            // -- OVERLAP
            case 0x0081: cpu->b=cpu->b;_fetch(); break;
            
            // ld c,b (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0082: _rfsh(); break;
            // -- OVERLAP
            case 0x0083: cpu->b=cpu->c;_fetch(); break;
            
            // ld d,b (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0084: _rfsh(); break;
            // -- OVERLAP
            case 0x0085: cpu->b=cpu->d;_fetch(); break;
            
            // ld e,b (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0086: _rfsh(); break;
            // -- OVERLAP
            case 0x0087: cpu->b=cpu->e;_fetch(); break;
            
            // ld h,b (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0088: _rfsh(); break;
            // -- OVERLAP
            case 0x0089: cpu->b=cpu->h;_fetch(); break;
            
            // ld l,b (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x008A: _rfsh(); break;
            // -- OVERLAP
            case 0x008B: cpu->b=cpu->l;_fetch(); break;
            
            // ld b,(hl) (M:2 T:7)
            // -- M1 (fetch/rfsh)
            case 0x008C: _rfsh(); break;
            // -- M2 (mread)
            case 0x008D: _mr(_ghl()); break;
            case 0x008E: cpu->b=_gd(); break;
            // -- OVERLAP
            case 0x008F: _fetch(); break;
            
            // ld a,b (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0090: _rfsh(); break;
            // -- OVERLAP
            case 0x0091: cpu->b=cpu->a;_fetch(); break;
            
            // ld b,c (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0092: _rfsh(); break;
            // -- OVERLAP
            case 0x0093: cpu->c=cpu->b;_fetch(); break;
            
            // ld c,c (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0094: _rfsh(); break;
            // -- OVERLAP
            case 0x0095: cpu->c=cpu->c;_fetch(); break;
            
            // ld d,c (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0096: _rfsh(); break;
            // -- OVERLAP
            case 0x0097: cpu->c=cpu->d;_fetch(); break;
            
            // ld e,c (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0098: _rfsh(); break;
            // -- OVERLAP
            case 0x0099: cpu->c=cpu->e;_fetch(); break;
            
            // ld h,c (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x009A: _rfsh(); break;
            // -- OVERLAP
            case 0x009B: cpu->c=cpu->h;_fetch(); break;
            
            // ld l,c (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x009C: _rfsh(); break;
            // -- OVERLAP
            case 0x009D: cpu->c=cpu->l;_fetch(); break;
            
            // ld c,(hl) (M:2 T:7)
            // -- M1 (fetch/rfsh)
            case 0x009E: _rfsh(); break;
            // -- M2 (mread)
            case 0x009F: _mr(_ghl()); break;
            case 0x00A0: cpu->c=_gd(); break;
            // -- OVERLAP
            case 0x00A1: _fetch(); break;
            
            // ld a,c (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x00A2: _rfsh(); break;
            // -- OVERLAP
            case 0x00A3: cpu->c=cpu->a;_fetch(); break;
            
            // ld b,d (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x00A4: _rfsh(); break;
            // -- OVERLAP
            case 0x00A5: cpu->d=cpu->b;_fetch(); break;
            
            // ld c,d (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x00A6: _rfsh(); break;
            // -- OVERLAP
            case 0x00A7: cpu->d=cpu->c;_fetch(); break;
            
            // ld d,d (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x00A8: _rfsh(); break;
            // -- OVERLAP
            case 0x00A9: cpu->d=cpu->d;_fetch(); break;
            
            // ld e,d (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x00AA: _rfsh(); break;
            // -- OVERLAP
            case 0x00AB: cpu->d=cpu->e;_fetch(); break;
            
            // ld h,d (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x00AC: _rfsh(); break;
            // -- OVERLAP
            case 0x00AD: cpu->d=cpu->h;_fetch(); break;
            
            // ld l,d (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x00AE: _rfsh(); break;
            // -- OVERLAP
            case 0x00AF: cpu->d=cpu->l;_fetch(); break;
            
            // ld d,(hl) (M:2 T:7)
            // -- M1 (fetch/rfsh)
            case 0x00B0: _rfsh(); break;
            // -- M2 (mread)
            case 0x00B1: _mr(_ghl()); break;
            case 0x00B2: cpu->d=_gd(); break;
            // -- OVERLAP
            case 0x00B3: _fetch(); break;
            
            // ld a,d (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x00B4: _rfsh(); break;
            // -- OVERLAP
            case 0x00B5: cpu->d=cpu->a;_fetch(); break;
            
            // ld b,e (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x00B6: _rfsh(); break;
            // -- OVERLAP
            case 0x00B7: cpu->e=cpu->b;_fetch(); break;
            
            // ld c,e (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x00B8: _rfsh(); break;
            // -- OVERLAP
            case 0x00B9: cpu->e=cpu->c;_fetch(); break;
            
            // ld d,e (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x00BA: _rfsh(); break;
            // -- OVERLAP
            case 0x00BB: cpu->e=cpu->d;_fetch(); break;
            
            // ld e,e (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x00BC: _rfsh(); break;
            // -- OVERLAP
            case 0x00BD: cpu->e=cpu->e;_fetch(); break;
            
            // ld h,e (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x00BE: _rfsh(); break;
            // -- OVERLAP
            case 0x00BF: cpu->e=cpu->h;_fetch(); break;
            
            // ld l,e (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x00C0: _rfsh(); break;
            // -- OVERLAP
            case 0x00C1: cpu->e=cpu->l;_fetch(); break;
            
            // ld e,(hl) (M:2 T:7)
            // -- M1 (fetch/rfsh)
            case 0x00C2: _rfsh(); break;
            // -- M2 (mread)
            case 0x00C3: _mr(_ghl()); break;
            case 0x00C4: cpu->e=_gd(); break;
            // -- OVERLAP
            case 0x00C5: _fetch(); break;
            
            // ld a,e (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x00C6: _rfsh(); break;
            // -- OVERLAP
            case 0x00C7: cpu->e=cpu->a;_fetch(); break;
            
            // ld b,h (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x00C8: _rfsh(); break;
            // -- OVERLAP
            case 0x00C9: cpu->h=cpu->b;_fetch(); break;
            
            // ld c,h (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x00CA: _rfsh(); break;
            // -- OVERLAP
            case 0x00CB: cpu->h=cpu->c;_fetch(); break;
            
            // ld d,h (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x00CC: _rfsh(); break;
            // -- OVERLAP
            case 0x00CD: cpu->h=cpu->d;_fetch(); break;
            
            // ld e,h (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x00CE: _rfsh(); break;
            // -- OVERLAP
            case 0x00CF: cpu->h=cpu->e;_fetch(); break;
            
            // ld h,h (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x00D0: _rfsh(); break;
            // -- OVERLAP
            case 0x00D1: cpu->h=cpu->h;_fetch(); break;
            
            // ld l,h (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x00D2: _rfsh(); break;
            // -- OVERLAP
            case 0x00D3: cpu->h=cpu->l;_fetch(); break;
            
            // ld h,(hl) (M:2 T:7)
            // -- M1 (fetch/rfsh)
            case 0x00D4: _rfsh(); break;
            // -- M2 (mread)
            case 0x00D5: _mr(_ghl()); break;
            case 0x00D6: cpu->h=_gd(); break;
            // -- OVERLAP
            case 0x00D7: _fetch(); break;
            
            // ld a,h (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x00D8: _rfsh(); break;
            // -- OVERLAP
            case 0x00D9: cpu->h=cpu->a;_fetch(); break;
            
            // ld b,l (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x00DA: _rfsh(); break;
            // -- OVERLAP
            case 0x00DB: cpu->l=cpu->b;_fetch(); break;
            
            // ld c,l (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x00DC: _rfsh(); break;
            // -- OVERLAP
            case 0x00DD: cpu->l=cpu->c;_fetch(); break;
            
            // ld d,l (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x00DE: _rfsh(); break;
            // -- OVERLAP
            case 0x00DF: cpu->l=cpu->d;_fetch(); break;
            
            // ld e,l (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x00E0: _rfsh(); break;
            // -- OVERLAP
            case 0x00E1: cpu->l=cpu->e;_fetch(); break;
            
            // ld h,l (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x00E2: _rfsh(); break;
            // -- OVERLAP
            case 0x00E3: cpu->l=cpu->h;_fetch(); break;
            
            // ld l,l (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x00E4: _rfsh(); break;
            // -- OVERLAP
            case 0x00E5: cpu->l=cpu->l;_fetch(); break;
            
            // ld l,(hl) (M:2 T:7)
            // -- M1 (fetch/rfsh)
            case 0x00E6: _rfsh(); break;
            // -- M2 (mread)
            case 0x00E7: _mr(_ghl()); break;
            case 0x00E8: cpu->l=_gd(); break;
            // -- OVERLAP
            case 0x00E9: _fetch(); break;
            
            // ld a,l (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x00EA: _rfsh(); break;
            // -- OVERLAP
            case 0x00EB: cpu->l=cpu->a;_fetch(); break;
            
            // ld (hl),b (M:2 T:7)
            // -- M1 (fetch/rfsh)
            case 0x00EC: _rfsh(); break;
            // -- M2 (mwrite)
            case 0x00ED: _mw(0xFFFF,0xFF)/*FIXME: address and data!*/; break;
            // -- OVERLAP
            case 0x00EE: _fetch(); break;
            
            // ld (hl),c (M:2 T:7)
            // -- M1 (fetch/rfsh)
            case 0x00EF: _rfsh(); break;
            // -- M2 (mwrite)
            case 0x00F0: _mw(0xFFFF,0xFF)/*FIXME: address and data!*/; break;
            // -- OVERLAP
            case 0x00F1: _fetch(); break;
            
            // ld (hl),d (M:2 T:7)
            // -- M1 (fetch/rfsh)
            case 0x00F2: _rfsh(); break;
            // -- M2 (mwrite)
            case 0x00F3: _mw(0xFFFF,0xFF)/*FIXME: address and data!*/; break;
            // -- OVERLAP
            case 0x00F4: _fetch(); break;
            
            // ld (hl),e (M:2 T:7)
            // -- M1 (fetch/rfsh)
            case 0x00F5: _rfsh(); break;
            // -- M2 (mwrite)
            case 0x00F6: _mw(0xFFFF,0xFF)/*FIXME: address and data!*/; break;
            // -- OVERLAP
            case 0x00F7: _fetch(); break;
            
            // ld (hl),h (M:2 T:7)
            // -- M1 (fetch/rfsh)
            case 0x00F8: _rfsh(); break;
            // -- M2 (mwrite)
            case 0x00F9: _mw(0xFFFF,0xFF)/*FIXME: address and data!*/; break;
            // -- OVERLAP
            case 0x00FA: _fetch(); break;
            
            // ld (hl),l (M:2 T:7)
            // -- M1 (fetch/rfsh)
            case 0x00FB: _rfsh(); break;
            // -- M2 (mwrite)
            case 0x00FC: _mw(0xFFFF,0xFF)/*FIXME: address and data!*/; break;
            // -- OVERLAP
            case 0x00FD: _fetch(); break;
            
            // halt (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x00FE: _rfsh(); break;
            // -- OVERLAP
            case 0x00FF: z80_halt(cpu);_fetch(); break;
            
            // ld (hl),a (M:2 T:7)
            // -- M1 (fetch/rfsh)
            case 0x0100: _rfsh(); break;
            // -- M2 (mwrite)
            case 0x0101: _mw(0xFFFF,0xFF)/*FIXME: address and data!*/; break;
            // -- OVERLAP
            case 0x0102: _fetch(); break;
            
            // ld b,a (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0103: _rfsh(); break;
            // -- OVERLAP
            case 0x0104: cpu->a=cpu->b;_fetch(); break;
            
            // ld c,a (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0105: _rfsh(); break;
            // -- OVERLAP
            case 0x0106: cpu->a=cpu->c;_fetch(); break;
            
            // ld d,a (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0107: _rfsh(); break;
            // -- OVERLAP
            case 0x0108: cpu->a=cpu->d;_fetch(); break;
            
            // ld e,a (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0109: _rfsh(); break;
            // -- OVERLAP
            case 0x010A: cpu->a=cpu->e;_fetch(); break;
            
            // ld h,a (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x010B: _rfsh(); break;
            // -- OVERLAP
            case 0x010C: cpu->a=cpu->h;_fetch(); break;
            
            // ld l,a (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x010D: _rfsh(); break;
            // -- OVERLAP
            case 0x010E: cpu->a=cpu->l;_fetch(); break;
            
            // ld a,(hl) (M:2 T:7)
            // -- M1 (fetch/rfsh)
            case 0x010F: _rfsh(); break;
            // -- M2 (mread)
            case 0x0110: _mr(_ghl()); break;
            case 0x0111: cpu->a=_gd(); break;
            // -- OVERLAP
            case 0x0112: _fetch(); break;
            
            // ld a,a (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0113: _rfsh(); break;
            // -- OVERLAP
            case 0x0114: cpu->a=cpu->a;_fetch(); break;
            
            // add b (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0115: _rfsh(); break;
            // -- OVERLAP
            case 0x0116: z80_add(cpu,cpu->b);_fetch(); break;
            
            // add c (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0117: _rfsh(); break;
            // -- OVERLAP
            case 0x0118: z80_add(cpu,cpu->c);_fetch(); break;
            
            // add d (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0119: _rfsh(); break;
            // -- OVERLAP
            case 0x011A: z80_add(cpu,cpu->d);_fetch(); break;
            
            // add e (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x011B: _rfsh(); break;
            // -- OVERLAP
            case 0x011C: z80_add(cpu,cpu->e);_fetch(); break;
            
            // add h (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x011D: _rfsh(); break;
            // -- OVERLAP
            case 0x011E: z80_add(cpu,cpu->h);_fetch(); break;
            
            // add l (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x011F: _rfsh(); break;
            // -- OVERLAP
            case 0x0120: z80_add(cpu,cpu->l);_fetch(); break;
            
            // add (hl) (M:2 T:7)
            // -- M1 (fetch/rfsh)
            case 0x0121: _rfsh(); break;
            // -- M2 (mread)
            case 0x0122: _mr(_ghl()); break;
            case 0x0123: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x0124: z80_add(cpu,cpu->dlatch);_fetch(); break;
            
            // add a (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0125: _rfsh(); break;
            // -- OVERLAP
            case 0x0126: z80_add(cpu,cpu->a);_fetch(); break;
            
            // adc b (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0127: _rfsh(); break;
            // -- OVERLAP
            case 0x0128: z80_adc(cpu,cpu->b);_fetch(); break;
            
            // adc c (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0129: _rfsh(); break;
            // -- OVERLAP
            case 0x012A: z80_adc(cpu,cpu->c);_fetch(); break;
            
            // adc d (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x012B: _rfsh(); break;
            // -- OVERLAP
            case 0x012C: z80_adc(cpu,cpu->d);_fetch(); break;
            
            // adc e (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x012D: _rfsh(); break;
            // -- OVERLAP
            case 0x012E: z80_adc(cpu,cpu->e);_fetch(); break;
            
            // adc h (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x012F: _rfsh(); break;
            // -- OVERLAP
            case 0x0130: z80_adc(cpu,cpu->h);_fetch(); break;
            
            // adc l (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0131: _rfsh(); break;
            // -- OVERLAP
            case 0x0132: z80_adc(cpu,cpu->l);_fetch(); break;
            
            // adc (hl) (M:2 T:7)
            // -- M1 (fetch/rfsh)
            case 0x0133: _rfsh(); break;
            // -- M2 (mread)
            case 0x0134: _mr(_ghl()); break;
            case 0x0135: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x0136: z80_adc(cpu,cpu->dlatch);_fetch(); break;
            
            // adc a (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0137: _rfsh(); break;
            // -- OVERLAP
            case 0x0138: z80_adc(cpu,cpu->a);_fetch(); break;
            
            // sub b (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0139: _rfsh(); break;
            // -- OVERLAP
            case 0x013A: z80_sub(cpu,cpu->b);_fetch(); break;
            
            // sub c (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x013B: _rfsh(); break;
            // -- OVERLAP
            case 0x013C: z80_sub(cpu,cpu->c);_fetch(); break;
            
            // sub d (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x013D: _rfsh(); break;
            // -- OVERLAP
            case 0x013E: z80_sub(cpu,cpu->d);_fetch(); break;
            
            // sub e (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x013F: _rfsh(); break;
            // -- OVERLAP
            case 0x0140: z80_sub(cpu,cpu->e);_fetch(); break;
            
            // sub h (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0141: _rfsh(); break;
            // -- OVERLAP
            case 0x0142: z80_sub(cpu,cpu->h);_fetch(); break;
            
            // sub l (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0143: _rfsh(); break;
            // -- OVERLAP
            case 0x0144: z80_sub(cpu,cpu->l);_fetch(); break;
            
            // sub (hl) (M:2 T:7)
            // -- M1 (fetch/rfsh)
            case 0x0145: _rfsh(); break;
            // -- M2 (mread)
            case 0x0146: _mr(_ghl()); break;
            case 0x0147: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x0148: z80_sub(cpu,cpu->dlatch);_fetch(); break;
            
            // sub a (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0149: _rfsh(); break;
            // -- OVERLAP
            case 0x014A: z80_sub(cpu,cpu->a);_fetch(); break;
            
            // sbc b (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x014B: _rfsh(); break;
            // -- OVERLAP
            case 0x014C: z80_sbc(cpu,cpu->b);_fetch(); break;
            
            // sbc c (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x014D: _rfsh(); break;
            // -- OVERLAP
            case 0x014E: z80_sbc(cpu,cpu->c);_fetch(); break;
            
            // sbc d (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x014F: _rfsh(); break;
            // -- OVERLAP
            case 0x0150: z80_sbc(cpu,cpu->d);_fetch(); break;
            
            // sbc e (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0151: _rfsh(); break;
            // -- OVERLAP
            case 0x0152: z80_sbc(cpu,cpu->e);_fetch(); break;
            
            // sbc h (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0153: _rfsh(); break;
            // -- OVERLAP
            case 0x0154: z80_sbc(cpu,cpu->h);_fetch(); break;
            
            // sbc l (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0155: _rfsh(); break;
            // -- OVERLAP
            case 0x0156: z80_sbc(cpu,cpu->l);_fetch(); break;
            
            // sbc (hl) (M:2 T:7)
            // -- M1 (fetch/rfsh)
            case 0x0157: _rfsh(); break;
            // -- M2 (mread)
            case 0x0158: _mr(_ghl()); break;
            case 0x0159: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x015A: z80_sbc(cpu,cpu->dlatch);_fetch(); break;
            
            // sbc a (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x015B: _rfsh(); break;
            // -- OVERLAP
            case 0x015C: z80_sbc(cpu,cpu->a);_fetch(); break;
            
            // and b (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x015D: _rfsh(); break;
            // -- OVERLAP
            case 0x015E: z80_and(cpu,cpu->b);_fetch(); break;
            
            // and c (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x015F: _rfsh(); break;
            // -- OVERLAP
            case 0x0160: z80_and(cpu,cpu->c);_fetch(); break;
            
            // and d (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0161: _rfsh(); break;
            // -- OVERLAP
            case 0x0162: z80_and(cpu,cpu->d);_fetch(); break;
            
            // and e (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0163: _rfsh(); break;
            // -- OVERLAP
            case 0x0164: z80_and(cpu,cpu->e);_fetch(); break;
            
            // and h (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0165: _rfsh(); break;
            // -- OVERLAP
            case 0x0166: z80_and(cpu,cpu->h);_fetch(); break;
            
            // and l (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0167: _rfsh(); break;
            // -- OVERLAP
            case 0x0168: z80_and(cpu,cpu->l);_fetch(); break;
            
            // and (hl) (M:2 T:7)
            // -- M1 (fetch/rfsh)
            case 0x0169: _rfsh(); break;
            // -- M2 (mread)
            case 0x016A: _mr(_ghl()); break;
            case 0x016B: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x016C: z80_and(cpu,cpu->dlatch);_fetch(); break;
            
            // and a (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x016D: _rfsh(); break;
            // -- OVERLAP
            case 0x016E: z80_and(cpu,cpu->a);_fetch(); break;
            
            // xor b (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x016F: _rfsh(); break;
            // -- OVERLAP
            case 0x0170: z80_xor(cpu,cpu->b);_fetch(); break;
            
            // xor c (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0171: _rfsh(); break;
            // -- OVERLAP
            case 0x0172: z80_xor(cpu,cpu->c);_fetch(); break;
            
            // xor d (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0173: _rfsh(); break;
            // -- OVERLAP
            case 0x0174: z80_xor(cpu,cpu->d);_fetch(); break;
            
            // xor e (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0175: _rfsh(); break;
            // -- OVERLAP
            case 0x0176: z80_xor(cpu,cpu->e);_fetch(); break;
            
            // xor h (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0177: _rfsh(); break;
            // -- OVERLAP
            case 0x0178: z80_xor(cpu,cpu->h);_fetch(); break;
            
            // xor l (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0179: _rfsh(); break;
            // -- OVERLAP
            case 0x017A: z80_xor(cpu,cpu->l);_fetch(); break;
            
            // xor (hl) (M:2 T:7)
            // -- M1 (fetch/rfsh)
            case 0x017B: _rfsh(); break;
            // -- M2 (mread)
            case 0x017C: _mr(_ghl()); break;
            case 0x017D: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x017E: z80_xor(cpu,cpu->dlatch);_fetch(); break;
            
            // xor a (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x017F: _rfsh(); break;
            // -- OVERLAP
            case 0x0180: z80_xor(cpu,cpu->a);_fetch(); break;
            
            // or b (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0181: _rfsh(); break;
            // -- OVERLAP
            case 0x0182: z80_or(cpu,cpu->b);_fetch(); break;
            
            // or c (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0183: _rfsh(); break;
            // -- OVERLAP
            case 0x0184: z80_or(cpu,cpu->c);_fetch(); break;
            
            // or d (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0185: _rfsh(); break;
            // -- OVERLAP
            case 0x0186: z80_or(cpu,cpu->d);_fetch(); break;
            
            // or e (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0187: _rfsh(); break;
            // -- OVERLAP
            case 0x0188: z80_or(cpu,cpu->e);_fetch(); break;
            
            // or h (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0189: _rfsh(); break;
            // -- OVERLAP
            case 0x018A: z80_or(cpu,cpu->h);_fetch(); break;
            
            // or l (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x018B: _rfsh(); break;
            // -- OVERLAP
            case 0x018C: z80_or(cpu,cpu->l);_fetch(); break;
            
            // or (hl) (M:2 T:7)
            // -- M1 (fetch/rfsh)
            case 0x018D: _rfsh(); break;
            // -- M2 (mread)
            case 0x018E: _mr(_ghl()); break;
            case 0x018F: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x0190: z80_or(cpu,cpu->dlatch);_fetch(); break;
            
            // or a (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0191: _rfsh(); break;
            // -- OVERLAP
            case 0x0192: z80_or(cpu,cpu->a);_fetch(); break;
            
            // cp b (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0193: _rfsh(); break;
            // -- OVERLAP
            case 0x0194: z80_cp(cpu,cpu->b);_fetch(); break;
            
            // cp c (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0195: _rfsh(); break;
            // -- OVERLAP
            case 0x0196: z80_cp(cpu,cpu->c);_fetch(); break;
            
            // cp d (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0197: _rfsh(); break;
            // -- OVERLAP
            case 0x0198: z80_cp(cpu,cpu->d);_fetch(); break;
            
            // cp e (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0199: _rfsh(); break;
            // -- OVERLAP
            case 0x019A: z80_cp(cpu,cpu->e);_fetch(); break;
            
            // cp h (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x019B: _rfsh(); break;
            // -- OVERLAP
            case 0x019C: z80_cp(cpu,cpu->h);_fetch(); break;
            
            // cp l (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x019D: _rfsh(); break;
            // -- OVERLAP
            case 0x019E: z80_cp(cpu,cpu->l);_fetch(); break;
            
            // cp (hl) (M:2 T:7)
            // -- M1 (fetch/rfsh)
            case 0x019F: _rfsh(); break;
            // -- M2 (mread)
            case 0x01A0: _mr(_ghl()); break;
            case 0x01A1: cpu->dlatch=_gd(); break;
            // -- OVERLAP
            case 0x01A2: z80_cp(cpu,cpu->dlatch);_fetch(); break;
            
            // cp a (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x01A3: _rfsh(); break;
            // -- OVERLAP
            case 0x01A4: z80_cp(cpu,cpu->a);_fetch(); break;
            
            // ret nz (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x01A5: _rfsh(); break;
            // -- OVERLAP
            case 0x01A6: _fetch(); break;
            
            // pop bc2 (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x01A7: _rfsh(); break;
            // -- OVERLAP
            case 0x01A8: _fetch(); break;
            
            // jp nz,nn (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x01A9: _rfsh(); break;
            // -- OVERLAP
            case 0x01AA: _fetch(); break;
            
            // jp nn (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x01AB: _rfsh(); break;
            // -- OVERLAP
            case 0x01AC: _fetch(); break;
            
            // call nz,nn (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x01AD: _rfsh(); break;
            // -- OVERLAP
            case 0x01AE: _fetch(); break;
            
            // push bc2 (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x01AF: _rfsh(); break;
            // -- OVERLAP
            case 0x01B0: _fetch(); break;
            
            // add n (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x01B1: _rfsh(); break;
            // -- OVERLAP
            case 0x01B2: _fetch(); break;
            
            // rst 0h (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x01B3: _rfsh(); break;
            // -- OVERLAP
            case 0x01B4: _fetch(); break;
            
            // ret z (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x01B5: _rfsh(); break;
            // -- OVERLAP
            case 0x01B6: _fetch(); break;
            
            // ret (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x01B7: _rfsh(); break;
            // -- OVERLAP
            case 0x01B8: _fetch(); break;
            
            // jp z,nn (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x01B9: _rfsh(); break;
            // -- OVERLAP
            case 0x01BA: _fetch(); break;
            
            // cb prefix (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x01BB: _rfsh(); break;
            // -- OVERLAP
            case 0x01BC: _fetch(); break;
            
            // call z,nn (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x01BD: _rfsh(); break;
            // -- OVERLAP
            case 0x01BE: _fetch(); break;
            
            // call nn (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x01BF: _rfsh(); break;
            // -- OVERLAP
            case 0x01C0: _fetch(); break;
            
            // adc n (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x01C1: _rfsh(); break;
            // -- OVERLAP
            case 0x01C2: _fetch(); break;
            
            // rst 8h (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x01C3: _rfsh(); break;
            // -- OVERLAP
            case 0x01C4: _fetch(); break;
            
            // ret nc (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x01C5: _rfsh(); break;
            // -- OVERLAP
            case 0x01C6: _fetch(); break;
            
            // pop de2 (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x01C7: _rfsh(); break;
            // -- OVERLAP
            case 0x01C8: _fetch(); break;
            
            // jp nc,nn (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x01C9: _rfsh(); break;
            // -- OVERLAP
            case 0x01CA: _fetch(); break;
            
            // out (n),a (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x01CB: _rfsh(); break;
            // -- OVERLAP
            case 0x01CC: _fetch(); break;
            
            // call nc,nn (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x01CD: _rfsh(); break;
            // -- OVERLAP
            case 0x01CE: _fetch(); break;
            
            // push de2 (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x01CF: _rfsh(); break;
            // -- OVERLAP
            case 0x01D0: _fetch(); break;
            
            // sub n (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x01D1: _rfsh(); break;
            // -- OVERLAP
            case 0x01D2: _fetch(); break;
            
            // rst 10h (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x01D3: _rfsh(); break;
            // -- OVERLAP
            case 0x01D4: _fetch(); break;
            
            // ret c (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x01D5: _rfsh(); break;
            // -- OVERLAP
            case 0x01D6: _fetch(); break;
            
            // exx (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x01D7: _rfsh(); break;
            // -- OVERLAP
            case 0x01D8: _fetch(); break;
            
            // jp c,nn (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x01D9: _rfsh(); break;
            // -- OVERLAP
            case 0x01DA: _fetch(); break;
            
            // in a,(n) (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x01DB: _rfsh(); break;
            // -- OVERLAP
            case 0x01DC: _fetch(); break;
            
            // call c,nn (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x01DD: _rfsh(); break;
            // -- OVERLAP
            case 0x01DE: _fetch(); break;
            
            // dd prefix (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x01DF: _rfsh(); break;
            // -- OVERLAP
            case 0x01E0: _fetch(); break;
            
            // sbc n (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x01E1: _rfsh(); break;
            // -- OVERLAP
            case 0x01E2: _fetch(); break;
            
            // rst 18h (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x01E3: _rfsh(); break;
            // -- OVERLAP
            case 0x01E4: _fetch(); break;
            
            // ret po (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x01E5: _rfsh(); break;
            // -- OVERLAP
            case 0x01E6: _fetch(); break;
            
            // pop hl2 (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x01E7: _rfsh(); break;
            // -- OVERLAP
            case 0x01E8: _fetch(); break;
            
            // jp po,nn (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x01E9: _rfsh(); break;
            // -- OVERLAP
            case 0x01EA: _fetch(); break;
            
            // ex (sp),hl (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x01EB: _rfsh(); break;
            // -- OVERLAP
            case 0x01EC: _fetch(); break;
            
            // call po,nn (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x01ED: _rfsh(); break;
            // -- OVERLAP
            case 0x01EE: _fetch(); break;
            
            // push hl2 (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x01EF: _rfsh(); break;
            // -- OVERLAP
            case 0x01F0: _fetch(); break;
            
            // and n (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x01F1: _rfsh(); break;
            // -- OVERLAP
            case 0x01F2: _fetch(); break;
            
            // rst 20h (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x01F3: _rfsh(); break;
            // -- OVERLAP
            case 0x01F4: _fetch(); break;
            
            // ret pe (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x01F5: _rfsh(); break;
            // -- OVERLAP
            case 0x01F6: _fetch(); break;
            
            // jp hl (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x01F7: _rfsh(); break;
            // -- OVERLAP
            case 0x01F8: _fetch(); break;
            
            // jp pe,nn (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x01F9: _rfsh(); break;
            // -- OVERLAP
            case 0x01FA: _fetch(); break;
            
            // ex de,hl (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x01FB: _rfsh(); break;
            // -- OVERLAP
            case 0x01FC: _fetch(); break;
            
            // call pe,nn (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x01FD: _rfsh(); break;
            // -- OVERLAP
            case 0x01FE: _fetch(); break;
            
            // ed prefix (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x01FF: _rfsh(); break;
            // -- OVERLAP
            case 0x0200: _fetch(); break;
            
            // xor n (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0201: _rfsh(); break;
            // -- OVERLAP
            case 0x0202: _fetch(); break;
            
            // rst 28h (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0203: _rfsh(); break;
            // -- OVERLAP
            case 0x0204: _fetch(); break;
            
            // ret p (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0205: _rfsh(); break;
            // -- OVERLAP
            case 0x0206: _fetch(); break;
            
            // pop sp2 (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0207: _rfsh(); break;
            // -- OVERLAP
            case 0x0208: _fetch(); break;
            
            // jp p,nn (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0209: _rfsh(); break;
            // -- OVERLAP
            case 0x020A: _fetch(); break;
            
            // di (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x020B: _rfsh(); break;
            // -- OVERLAP
            case 0x020C: _fetch(); break;
            
            // call p,nn (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x020D: _rfsh(); break;
            // -- OVERLAP
            case 0x020E: _fetch(); break;
            
            // push sp2 (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x020F: _rfsh(); break;
            // -- OVERLAP
            case 0x0210: _fetch(); break;
            
            // or n (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0211: _rfsh(); break;
            // -- OVERLAP
            case 0x0212: _fetch(); break;
            
            // rst 30h (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0213: _rfsh(); break;
            // -- OVERLAP
            case 0x0214: _fetch(); break;
            
            // ret m (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0215: _rfsh(); break;
            // -- OVERLAP
            case 0x0216: _fetch(); break;
            
            // ld sp,hl (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0217: _rfsh(); break;
            // -- OVERLAP
            case 0x0218: _fetch(); break;
            
            // jp m,nn (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0219: _rfsh(); break;
            // -- OVERLAP
            case 0x021A: _fetch(); break;
            
            // ei (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x021B: _rfsh(); break;
            // -- OVERLAP
            case 0x021C: _fetch(); break;
            
            // call m,nn (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x021D: _rfsh(); break;
            // -- OVERLAP
            case 0x021E: _fetch(); break;
            
            // fd prefix (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x021F: _rfsh(); break;
            // -- OVERLAP
            case 0x0220: _fetch(); break;
            
            // cp n (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0221: _rfsh(); break;
            // -- OVERLAP
            case 0x0222: _fetch(); break;
            
            // rst 38h (M:1 T:4)
            // -- M1 (fetch/rfsh)
            case 0x0223: _rfsh(); break;
            // -- OVERLAP
            case 0x0224: _fetch(); break;

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