#pragma once
/*#
    # lc80.h

    LC80 emulator in a C header.

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

    You need to include the following headers before including lc80.h:

    - chips/z80.h
    - chips/z80ctc.h
    - chips/z80pio.h
    - chips/beeper.h
    - chips/kbd.h
    - chips/clk.h

    ## The LC80

    The LC80 (**L**ern **C**omputer 80, Learning Computer 80) was an East
    German Z80 based computer used for education and simple controller tasks.

    It didn't have a proper keyboard and couldn't be attached to a TV,
    instead it had a pocket-calculator-derived mini-keyboard for hexadecimal
    numbers and a 6-digit LED display planted right on the open motherboard.

    The hardware looks like this:

    - 1x U880 CPU @ 900 kHz (equiv Z80)
    - 2x U855 (equiv Z80-PIO)
    - 1x U857 (equiv Z80-CTC)
    - 1x U505D (16kBit / 2kByte ROM == 2 KByte ROM)
    - 2..8x U214D (1024 x 4 bit SRAM, only default config of 1KB RAM emulated)
    - 0..3x K573RF2 (Soviet 2KByte EPROM, equiv Intel 2716, not emulated)
    - 2x DS8205 3-to-8 decoder used for address decoding (equiv LS138)
    - 3x VQE23 2-digits LED display block (equiv ???)

    TODO: more details about the hardware and emulator
        
    ## zlib/libpng license

    Copyright (c) 2019 Andre Weissflog
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

/* key codes (for lc80_key(), lc80_key_down(), lc80_key_up() */
#define LC80_KEY_0      ('0')
#define LC80_KEY_1      ('1')
#define LC80_KEY_2      ('2')
#define LC80_KEY_3      ('3')
#define LC80_KEY_4      ('4')
#define LC80_KEY_5      ('5')
#define LC80_KEY_6      ('6')
#define LC80_KEY_7      ('7')
#define LC80_KEY_8      ('8')
#define LC80_KEY_9      ('9')
#define LC80_KEY_A      ('a')
#define LC80_KEY_B      ('b')
#define LC80_KEY_C      ('c')
#define LC80_KEY_D      ('d')
#define LC80_KEY_E      ('e')
#define LC80_KEY_F      ('f')
#define LC80_KEY_RES    (0x01)
#define LC80_KEY_ADR    (0x0B)
#define LC80_KEY_DAT    (0x0A)
#define LC80_KEY_PLUS   (0x09)
#define LC80_KEY_MINUS  (0x08)
#define LC80_KEY_NMI    (0x0D)
#define LC80_KEY_ST     (0x02)
#define LC80_KEY_LD     (0x03)
#define LC80_KEY_EX     (0x20)

/* U505D ROM chip pins */
#define LC80_U505_A0     (1ULL<<0)
#define LC80_U505_A1     (1ULL<<1)
#define LC80_U505_A2     (1ULL<<2)
#define LC80_U505_A3     (1ULL<<3)
#define LC80_U505_A4     (1ULL<<4)
#define LC80_U505_A5     (1ULL<<5)
#define LC80_U505_A6     (1ULL<<6)
#define LC80_U505_A7     (1ULL<<7)
#define LC80_U505_A8     (1ULL<<8)
#define LC80_U505_A9     (1ULL<<9)
#define LC80_U505_A10    (1ULL<<10)
#define LC80_U505_D0     (1ULL<<16)
#define LC80_U505_D1     (1ULL<<17)
#define LC80_U505_D2     (1ULL<<18)
#define LC80_U505_D3     (1ULL<<19)
#define LC80_U505_D4     (1ULL<<20)
#define LC80_U505_D5     (1ULL<<21)
#define LC80_U505_D6     (1ULL<<22)
#define LC80_U505_D7     (1ULL<<23)
#define LC80_U505_CS     (1ULL<<24)

/* U214 RAM chip pins */
#define LC80_U214_A0     (1ULL<<0)
#define LC80_U214_A1     (1ULL<<1)
#define LC80_U214_A2     (1ULL<<2)
#define LC80_U214_A3     (1ULL<<3)
#define LC80_U214_A4     (1ULL<<4)
#define LC80_U214_A5     (1ULL<<5)
#define LC80_U214_A6     (1ULL<<6)
#define LC80_U214_A7     (1ULL<<7)
#define LC80_U214_A8     (1ULL<<8)
#define LC80_U214_A9     (1ULL<<9)
#define LC80_U214_D0     (1ULL<<16)
#define LC80_U214_D1     (1ULL<<17)
#define LC80_U214_D2     (1ULL<<18)
#define LC80_U214_D3     (1ULL<<19)
#define LC80_U214_CS     (1ULL<<24)
#define LC80_U214_WR     (1ULL<<28)     /* same as Z80_WR */

/* DS8205 / LS138 3-to-8 decoder pins */
#define LC80_DS8205_Y0   (1<<0)      /* output pins */
#define LC80_DS8205_Y1   (1<<1)
#define LC80_DS8205_Y2   (1<<2)
#define LC80_DS8205_Y3   (1<<3)
#define LC80_DS8205_Y4   (1<<4)
#define LC80_DS8205_Y5   (1<<5)
#define LC80_DS8205_Y6   (1<<6)
#define LC80_DS8205_Y7   (1<<7)
#define LC80_DS8205_A    (1<<8)      /* select inputs */
#define LC80_DS8205_B    (1<<9)
#define LC80_DS8205_C    (1<<10)
#define LC80_DS8205_G1   (1<<11)     /* enable inputs */
#define LC80_DS8205_G2A  (1<<12)
#define LC80_DS8205_G2B  (1<<13)
#define LC80_DS8205_SELECT  (LC80_DS8205_A|LC80_DS8205_B|LC80_DS8205_C)
#define LC80_DS8205_INPUTS  (LC80_DS8205_A|LC80_DS8205_B|LC80_DS8205_C|LC80_DS8205_G1|LC80_DS8205_G2A|LC80_DS8205_G2B)
#define LC80_DS8205_OUTPUTS (0xFF)

/* VQE23 2-digit LED state */
#define LC80_VQE23_A1    (1ULL<<0)
#define LC80_VQE23_B1    (1ULL<<1)
#define LC80_VQE23_C1    (1ULL<<2)
#define LC80_VQE23_D1    (1ULL<<3)
#define LC80_VQE23_E1    (1ULL<<4)
#define LC80_VQE23_F1    (1ULL<<5)
#define LC80_VQE23_G1    (1ULL<<6)
#define LC80_VQE23_P1    (1ULL<<7)
#define LC80_VQE23_A2    (1ULL<<8)
#define LC80_VQE23_B2    (1ULL<<9)
#define LC80_VQE23_C2    (1ULL<<10)
#define LC80_VQE23_D2    (1ULL<<11)
#define LC80_VQE23_E2    (1ULL<<12)
#define LC80_VQE23_F2    (1ULL<<13)
#define LC80_VQE23_G2    (1ULL<<14)
#define LC80_VQE23_P2    (1ULL<<15)
#define LC80_VQE23_K1    (1ULL<<16)
#define LC80_VQE23_K2    (1ULL<<17)

typedef void (*lc80_audio_callback_t)(const float* samples, int num_samples, void* user_data);
#define LC80_MAX_AUDIO_SAMPLES (1024)
#define LC80_DEFAULT_AUDIO_SAMPLES (128)

/* config parameters for lc80_init() */
typedef struct {
    /* optional userdata pointer for callbacks */
    void* user_data;

    /* audio output config (if you don't want audio, set audio_cb to zero) */
    lc80_audio_callback_t audio_cb;     /* called when audio_num_samples are ready */
    int audio_num_samples;              /* default is LC80_DEFAULT_AUDIO_SAMPLES */
    int audio_sample_rate;              /* playback sample rate, default is 44100 */
    float audio_volume;                 /* audio volume (0.0 .. 1.0), default is 0.3 */

    /* ROM image (must be single 2KByte image) */
    const void* rom_ptr;
    int rom_size;
} lc80_desc_t;

/* LC80 emulator state */
typedef struct {
    bool valid;

    z80_t cpu;
    z80ctc_t ctc;
    z80pio_t pio_sys;
    z80pio_t pio_usr;
    uint32_t vqe23[3];          /* state of the 2-digit LED modules */
    uint32_t u505;              /* pin state of the 2 U505D ROM chips */
    uint32_t u214[2];           /* pin state of the 2 U214D RAM chips */
    uint32_t ds8205[2];         /* pin state of the 2 DS8205 3-to-8 decoders (equiv LS138) */
    bool reset;
    bool nmi;

    beeper_t beeper;
    clk_t clk;
    kbd_t kbd;

    void* user_data;
    lc80_audio_callback_t audio_cb;
    int num_samples;
    int sample_pos;
    float sample_buffer[LC80_MAX_AUDIO_SAMPLES];

    uint8_t ram[0x0400];
    uint8_t rom[0x0800];
} lc80_t;

void lc80_init(lc80_t* sys, const lc80_desc_t* desc);
void lc80_discard(lc80_t* sys);
void lc80_reset(lc80_t* sys);
void lc80_exec(lc80_t* sys, uint32_t micro_seconds);
void lc80_key_down(lc80_t* sys, int key_code);
void lc80_key_up(lc80_t* sys, int key_code);
void lc80_key(lc80_t* sys, int key_code);       /* down + up */

#ifdef __cplusplus
} /* extern "C" */
#endif

/*-- IMPLEMENTATION ----------------------------------------------------------*/
#ifdef CHIPS_IMPL
#include <string.h> /* memcpy, memset */
#ifndef CHIPS_ASSERT
    #include <assert.h>
    #define CHIPS_ASSERT(c) assert(c)
#endif

#define _LC80_DEFAULT(val,def) (((val) != 0) ? (val) : (def));

static uint64_t _lc80_tick(int num, uint64_t pins, void* user_data);
static uint8_t _lc80_pio_sys_in(int port_id, void* user_data);
static void _lc80_pio_sys_out(int port_id, uint8_t data, void* user_data);
static uint8_t _lc80_pio_usr_in(int port_id, void* user_data);
static void _lc80_pio_usr_out(int port_id, uint8_t data, void* user_data);

void lc80_init(lc80_t* sys, const lc80_desc_t* desc) {
    CHIPS_ASSERT(sys && desc);
    
    memset(sys, 0, sizeof(lc80_t));
    sys->valid = true;
    sys->user_data = desc->user_data;
    
    CHIPS_ASSERT(desc->rom_ptr && (desc->rom_size == sizeof(sys->rom)));
    memcpy(sys->rom, desc->rom_ptr, sizeof(sys->rom));

    /* system clock is 900 kHz */
    const uint32_t freq_hz = 900000;
    clk_init(&sys->clk, freq_hz);
    z80ctc_init(&sys->ctc);

    z80_desc_t cpu_desc;
    memset(&cpu_desc, 0, sizeof(cpu_desc));
    cpu_desc.tick_cb = _lc80_tick;
    cpu_desc.user_data = sys;
    z80_init(&sys->cpu, &cpu_desc);

    z80pio_desc_t pio_desc;
    memset(&pio_desc, 0, sizeof(pio_desc));
    pio_desc.in_cb = _lc80_pio_sys_in;
    pio_desc.out_cb = _lc80_pio_sys_out;
    pio_desc.user_data = sys;
    z80pio_init(&sys->pio_sys, &pio_desc);
    pio_desc.in_cb = _lc80_pio_usr_in;
    pio_desc.out_cb = _lc80_pio_usr_out;
    z80pio_init(&sys->pio_usr, &pio_desc);

    for (int i = 0; i < 3; i++) {
        sys->vqe23[i] = 0x0000FFFF;
    }

    sys->audio_cb = desc->audio_cb;
    sys->num_samples = _LC80_DEFAULT(desc->audio_num_samples, LC80_DEFAULT_AUDIO_SAMPLES);
    const int audio_hz = _LC80_DEFAULT(desc->audio_sample_rate, 44100);
    const float audio_vol = _LC80_DEFAULT(desc->audio_volume, 0.3f);
    beeper_init(&sys->beeper, freq_hz, audio_hz, audio_vol);

    /* keyboard matrix:
          0   1   2   3   4   5
        +---+---+---+---+---+---+   PIO (USR) Port B
      0 |   | 3 | 7 | B | F | - |-> 4
        +---+---+---+---+---+---+
      1 |LD | 2 | + | E | A | 6 |-> 5
        +---+---+---+---+---+---+
      2 |ST | 1 | 5 | 9 | D |DAT|-> 6
        +---+---+---+---+---+---+
      3 |EX | 0 | 4 | 8 | C |ADR|-> 7
        +---+---+---+---+---+---+
          ^   ^   ^   ^   ^   ^
          2   3   4   5   6   7
             PIO (SYS) Port B
    */
    int kbd_matrix[4][6] = {
        { 0,            LC80_KEY_3, LC80_KEY_7,     LC80_KEY_B, LC80_KEY_F, LC80_KEY_MINUS },
        { LC80_KEY_LD,  LC80_KEY_2, LC80_KEY_PLUS,  LC80_KEY_E, LC80_KEY_A, LC80_KEY_6 },
        { LC80_KEY_ST,  LC80_KEY_1, LC80_KEY_5,     LC80_KEY_9, LC80_KEY_D, LC80_KEY_DAT },
        { LC80_KEY_EX,  LC80_KEY_0, LC80_KEY_4,     LC80_KEY_8, LC80_KEY_C, LC80_KEY_ADR }
    };
    kbd_init(&sys->kbd, 2);
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 6; col++) {
            int key = kbd_matrix[row][col];
            if (0 != key) {
                kbd_register_key(&sys->kbd, key, col, row, 0);
            }
        }
    }

    /* execution starts at 0x0000 */
    z80_set_pc(&sys->cpu, 0x0000);
}

void lc80_discard(lc80_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    sys->valid = false;
}

void lc80_reset(lc80_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    sys->reset = false;
    sys->nmi = false;
    z80_reset(&sys->cpu);
    z80ctc_reset(&sys->ctc);
    z80pio_reset(&sys->pio_sys);
    z80pio_reset(&sys->pio_usr);
    beeper_reset(&sys->beeper);
    z80_set_pc(&sys->cpu, 0x0000);
}

void lc80_exec(lc80_t* sys, uint32_t micro_seconds) {
    CHIPS_ASSERT(sys && sys->valid);
    uint32_t ticks_to_run = clk_ticks_to_run(&sys->clk, micro_seconds);
    uint32_t ticks_executed = z80_exec(&sys->cpu, ticks_to_run);
    clk_ticks_executed(&sys->clk, ticks_executed);
    kbd_update(&sys->kbd, micro_seconds);
    if (sys->nmi) {
        sys->nmi = false;
    }
    if (sys->reset) {
        lc80_reset(sys);
    }
}

void lc80_key_down(lc80_t* sys, int key_code) {
    CHIPS_ASSERT(sys && sys->valid);
    switch (key_code) {
        case LC80_KEY_RES: sys->reset = true; break;
        case LC80_KEY_NMI: sys->nmi = true; break;
        default: kbd_key_down(&sys->kbd, key_code);
    }
}

void lc80_key_up(lc80_t* sys, int key_code) {
    CHIPS_ASSERT(sys && sys->valid);
    switch (key_code) {
        case LC80_KEY_RES:
        case LC80_KEY_NMI:
            /* not a bug */
            break;
        default:
            kbd_key_up(&sys->kbd, key_code);
            break;
    }
}

void lc80_key(lc80_t* sys, int key_code) {
    CHIPS_ASSERT(sys && sys->valid);
    lc80_key_down(sys, key_code);
    lc80_key_up(sys, key_code);
}

#define _LC80_HI(pins,mask) (0!=(pins&mask))
#define _LC80_LO(pins,mask) (0==(pins&mask))

/* DS8205 (LS138) 3-to-8 decoder */
static inline uint32_t _lc80_ds8205_tick(uint32_t inp) {
    /*
        enable = G1 && !G2A && !G2B

        Select pins are active high.

        Output pins are active high (real hardware active-low)
    */
    uint32_t outp = inp & LC80_DS8205_INPUTS;
    if ((inp & (LC80_DS8205_G1|LC80_DS8205_G2A|LC80_DS8205_G2B)) == LC80_DS8205_G1) {
        /* outputs enabled */
        uint8_t data = (1 << ((inp & LC80_DS8205_SELECT)>>8));
        outp |= data;
    }
    return outp;
}

/* LC80 CPU tick callback */
uint64_t _lc80_tick(int num_ticks, uint64_t pins, void* user_data) {
    lc80_t* sys = (lc80_t*) user_data;

    /* Address decoding via the two DS8205 3-to-8 decoders (LS138 clones)

        This is a bit of an "active-hi/lo" mess, all chip emulator
        pin masks are "active means bit is set" regardless of
        active-hi/lo in the real hardware.

        On the LC-80, all CPU control pins are inverted, so they
        actually look like active-high to the system.

        We'll treat the DS8205 Enable pins like the real hardware,
        but the Select and Output both as "bit set if active".
    */
    uint32_t d209 = LC80_DS8205_C;  /* C input is always high, but connected to
                                       a switch to optionally switch to a user-
                                       provided set of EEPROMS
                                    */
    if (_LC80_HI(pins, Z80_A11))    { d209 |= LC80_DS8205_A; }
    if (_LC80_HI(pins, Z80_A12))    { d209 |= LC80_DS8205_B; }
    if (_LC80_LO(pins, Z80_A13))    { d209 |= LC80_DS8205_G1; }
    if (_LC80_LO(pins, Z80_MREQ))   { d209 |= LC80_DS8205_G2A; }
    d209 = _lc80_ds8205_tick(d209);
    sys->ds8205[0] = d209;

    uint32_t d210 = LC80_DS8205_C|LC80_DS8205_G1;
    if (_LC80_HI(pins, Z80_A10))    { d210 |= LC80_DS8205_A; }
    if (_LC80_HI(pins, Z80_A11))    { d210 |= LC80_DS8205_B; }
    if (_LC80_LO(pins, Z80_MREQ))   { d210 |= LC80_DS8205_G2A; }
    if (_LC80_LO(pins, Z80_A13))    { d210 |= LC80_DS8205_G2B; }
    d210 = _lc80_ds8205_tick(d210);
    sys->ds8205[1] = d210;

    /* ROM access? */
    if (d209 & LC80_DS8205_Y4) {
        /* Y4 output of D209 selects D202 (ROM1) */
        uint16_t addr = pins & 0x7FF;
        uint8_t data = sys->rom[addr];
        Z80_SET_DATA(pins, data);
        sys->u505 = LC80_U505_CS | (data << 16) | addr;
    }
    else {
        sys->u505 = (pins & 0xFF07FF);
    }

    /* RAM access?

        D210 output Y4 selected RAM bank 0
             outputs Y5..Y7 select the RAM banks 1..3, bit those
             are not socketed in a standard LC80
    */
    if (d210 & LC80_DS8205_Y4) {
        /* Y4 output of D209 selects RAM bank 0 (D204 and D205, each 4 bit of data) */
        uint16_t addr = pins & 0x3FF;
        uint8_t data = 0xFF;
        if (pins & Z80_WR) {
            /* RAM write access */
            data = Z80_GET_DATA(pins);
            sys->ram[addr] = data;
        }
        else if (pins & Z80_RD) {
            /* RAM read access */
            data = sys->ram[addr];
            Z80_SET_DATA(pins, data);
        }
        sys->u214[0] = LC80_U214_CS | (pins & Z80_WR) | addr | (((data>>0) & 0x0F)<<16);
        sys->u214[1] = LC80_U214_CS | (pins & Z80_WR) | addr | (((data>>4) & 0x0F)<<16);
    }
    else {
        sys->u214[0] = pins & (Z80_WR | 0x0F03FF);
        sys->u214[1] = (pins & (Z80_WR | 0x0003FF)) | ((pins & 0xF00000)>>4);
    }

    /* System PIO */
    uint64_t pio_sys_pins = pins;
    if (0 == (pins & Z80_A3)) { pio_sys_pins |= Z80PIO_CE; }
    if (pins & Z80_A0) { pio_sys_pins |= Z80PIO_BASEL; }
    if (pins & Z80_A1) { pio_sys_pins |= Z80PIO_CDSEL; }
    pins = z80pio_iorq(&sys->pio_sys, pio_sys_pins) & Z80_PIN_MASK;

    /* User PIO */
    uint64_t pio_usr_pins = pins;
    if (0 == (pins & Z80_A2)) { pio_usr_pins |= Z80PIO_CE; }
    if (pins & Z80_A0) { pio_usr_pins |= Z80PIO_BASEL; }
    if (pins & Z80_A1) { pio_usr_pins |= Z80PIO_CDSEL; }
    pins = z80pio_iorq(&sys->pio_usr, pio_usr_pins) & Z80_PIN_MASK;

    /* CTC */
    uint64_t ctc_pins = pins;
    if (0 == (pins & Z80_A4)) { ctc_pins |= Z80CTC_CE; };
    if (pins & Z80_A0) { ctc_pins |= Z80CTC_CS0; }
    if (pins & Z80_A1) { ctc_pins |= Z80CTC_CS1; }
    pins = z80ctc_iorq(&sys->ctc, ctc_pins) & Z80_PIN_MASK;

    /* tick CTC and handle beeper */
    for (int i = 0; i < num_ticks; i++) {
        pins = z80ctc_tick(&sys->ctc, pins);
        if (beeper_tick(&sys->beeper)) {
            /* new audio sample ready */
            sys->sample_buffer[sys->sample_pos++] = sys->beeper.sample;
            if (sys->sample_pos == sys->num_samples) {
                if (sys->audio_cb) {
                    sys->audio_cb(sys->sample_buffer, sys->num_samples, sys->user_data);
                }
                sys->sample_pos = 0;
            }
        }
    }

    /* interrupt daisychain priority is: CTC => User PIO => System PIO */
    Z80_DAISYCHAIN_BEGIN(pins)
    {
        pins = z80ctc_int(&sys->ctc, pins);
        pins = z80pio_int(&sys->pio_usr, pins);
        pins = z80pio_int(&sys->pio_sys, pins);
    }
    Z80_DAISYCHAIN_END(pins);

    /* NMI? */
    if (sys->nmi) {
        pins |= Z80_NMI;
    }
    else {
        pins &= ~Z80_NMI;
    }

    return (pins & Z80_PIN_MASK);
}

/* update a VQE23 2-digit LED display element */
static uint32_t _lc80_vqe23_write(uint32_t vqe23, int digit, uint8_t data) {
    if (0 == digit) {
        vqe23 &= ~0xFF;
        if (data & (1<<2)) { vqe23 |= LC80_VQE23_A1; }
        if (data & (1<<0)) { vqe23 |= LC80_VQE23_B1; }
        if (data & (1<<5)) { vqe23 |= LC80_VQE23_C1; }
        if (data & (1<<7)) { vqe23 |= LC80_VQE23_D1; }
        if (data & (1<<6)) { vqe23 |= LC80_VQE23_E1; }
        if (data & (1<<1)) { vqe23 |= LC80_VQE23_F1; }
        if (data & (1<<3)) { vqe23 |= LC80_VQE23_G1; }
        if (data & (1<<4)) { vqe23 |= LC80_VQE23_P1; }
    }
    else {
        vqe23 &= ~0xFF00;
        if (data & (1<<2)) { vqe23 |= LC80_VQE23_A2; }
        if (data & (1<<0)) { vqe23 |= LC80_VQE23_B2; }
        if (data & (1<<5)) { vqe23 |= LC80_VQE23_C2; }
        if (data & (1<<7)) { vqe23 |= LC80_VQE23_D2; }
        if (data & (1<<6)) { vqe23 |= LC80_VQE23_E2; }
        if (data & (1<<1)) { vqe23 |= LC80_VQE23_F2; }
        if (data & (1<<3)) { vqe23 |= LC80_VQE23_G2; }
        if (data & (1<<4)) { vqe23 |= LC80_VQE23_P2; }
    }
    return vqe23;
}

uint8_t _lc80_pio_sys_in(int port_id, void* user_data) {
    (void)port_id;
    (void)user_data;
    // FIXME: TAPE IN
    return 0xFF;
}

void _lc80_pio_sys_out(int port_id, uint8_t data, void* user_data) {
    lc80_t* sys = (lc80_t*) user_data;
    if (port_id == 1) {
        /* TAPE OUT */
        beeper_set(&sys->beeper, 0 == (data & (1<<1)));

        /* LED display update

            Bits 2..7 of System PIO Port B select the digit,
            and all 8 bits of System PIO Port A select
            the 7 segments and decimal point.
        */
        const uint8_t outp = sys->pio_sys.port[Z80PIO_PORT_A].output;
        if (0 == (data & (1<<2))) {
            sys->vqe23[0] = _lc80_vqe23_write(sys->vqe23[0], 0, outp);
            sys->vqe23[0] |= LC80_VQE23_K1;
        }
        else {
            sys->vqe23[0] &= ~LC80_VQE23_K1;
        }
        if (0 == (data & (1<<3))) {
            sys->vqe23[0] = _lc80_vqe23_write(sys->vqe23[0], 1, outp);
            sys->vqe23[0] |= LC80_VQE23_K2;
        }
        else {
            sys->vqe23[0] &= ~LC80_VQE23_K2;
        }
        if (0 == (data & (1<<4))) {
            sys->vqe23[1] = _lc80_vqe23_write(sys->vqe23[1], 0, outp);
            sys->vqe23[1] |= LC80_VQE23_K1;
        }
        else {
            sys->vqe23[1] &= ~LC80_VQE23_K1;
        }
        if (0 == (data & (1<<5))) {
            sys->vqe23[1] = _lc80_vqe23_write(sys->vqe23[1], 1, outp);
            sys->vqe23[1] |= LC80_VQE23_K2;
        }
        else {
            sys->vqe23[1] &= ~LC80_VQE23_K2;
        }
        if (0 == (data & (1<<6))) {
            sys->vqe23[2] = _lc80_vqe23_write(sys->vqe23[2], 0, outp);
            sys->vqe23[2] |= LC80_VQE23_K1;
        }
        else {
            sys->vqe23[2] &= ~LC80_VQE23_K1;
        }
        if (0 == (data & (1<<7))) {
            sys->vqe23[2] = _lc80_vqe23_write(sys->vqe23[2], 1, outp);
            sys->vqe23[2] |= LC80_VQE23_K2;
        }
        else {
            sys->vqe23[2] &= ~LC80_VQE23_K2;
        }
        /* bits 2..7 of port B also double as input to the keyboard matrix */
        uint8_t kbd_columns = ~(data >> 2) & 0x3F;
        kbd_set_active_columns(&sys->kbd, kbd_columns);
    }
}

uint8_t _lc80_pio_usr_in(int port_id, void* user_data) {
    lc80_t* sys = (lc80_t*) user_data;
    /* bits 4..7 of port B are keyboard matrix lines */
    if (port_id == 1) {
        uint8_t kbd_lines = ~(kbd_scan_lines(&sys->kbd)<<4);
        return kbd_lines;
    }
    else {
        return 0xFF;
    }
}

void _lc80_pio_usr_out(int port_id, uint8_t data, void* user_data) {
    // FIXME
    (void)port_id;
    (void)data;
    (void)user_data;
}

#endif /* CHIPS_IMPL */
