#pragma once
/*#
    # nes.h

    A NES emulator in a C header.

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

    You need to include the following headers before including nes.h:

    - chips/chips_common.h
    - chips/clk.h
    - chips/m6502.h
    - chips/r2c02.h

    ## Warning

    This emulator is not fully implemented:
        - PAL, Dendy not implemented
        - only mappers 0, 1, 2, 3, 7 & 66 are implemented
        - audio is limited to pulses 1 & 2 and noise channels (triangle and DMC channels are not implemented)
        - only the standard NES controller is supported

    ## zlib/libpng license

        Copyright (c) 2023 Scemino
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
#include <stddef.h>
#include <stdalign.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NES_DEFAULT_AUDIO_SAMPLE_RATE (44100)
#define NES_DEFAULT_AUDIO_SAMPLES (128)     // default number of samples in internal sample buffer
#define NES_MAX_AUDIO_SAMPLES (1024)        // max number of audio samples in internal sample buffer

// bump when nes_t memory layout changes
#define NES_SNAPSHOT_VERSION (0x0001)

// pad mask bits
#define NES_PAD_RIGHT (1<<0)
#define NES_PAD_LEFT  (1<<1)
#define NES_PAD_DOWN  (1<<2)
#define NES_PAD_UP    (1<<3)
#define NES_PAD_START (1<<4)
#define NES_PAD_SEL   (1<<5)
#define NES_PAD_B     (1<<6)
#define NES_PAD_A     (1<<7)

typedef struct {
    char magic[4];

    uint8_t prg_page_count;       // 16 KB page size
    uint8_t tile_page_count;      // 8 KB page size
    bool mirror_mode : 1;       // 0 - vertical arrangement (horizontal mirror)
                                // 1 - horizontal arrangement (vertical mirror)
    bool sram_avail : 1;        // battery-backed Save RAM
    bool trainer : 1;           // contains a trainer ROM
    bool vram_expansion : 1;    // 0 - two screen nametable set
                                // 1 - four screen nametable set
    uint8_t mapper_low : 4;       // low nibble of mapper number
    uint8_t reserved_0 : 4;
    uint8_t mapper_hi : 4;        // high nibble of mapper number
    uint8_t sram_page_count;      // 8 KB page size
    uint8_t reserved_1[7];
} nes_cartridge_header;

// configuration parameters for nes_init()
typedef struct {
    chips_audio_desc_t audio;
    chips_debug_t debug;
} nes_desc_t;

typedef union {
    struct {
        uint8_t right:  1;
        uint8_t left:   1;
        uint8_t down:   1;
        uint8_t up:     1;
        uint8_t start:  1;
        uint8_t select: 1;
        uint8_t b:      1;
        uint8_t a:      1;
    };
    uint8_t value;
} controller_t;

typedef enum {
    Horizontal  = 0,
    Vertical    = 1,
    FourScreen  = 8,
    OneScreenLower,
    OneScreenHigher,
} name_table_mirroring_t;

typedef struct {
    uint8_t (*read_prg)(uint16_t address, void* user_data);
    void (*write_prg)(uint16_t address, uint8_t value, void* user_data);
    uint8_t (*read_chr)(uint16_t address, void* user_data);
    void (*write_chr)(uint16_t addr, uint8_t data, void* user_data);

    union {
        struct {
            uint8_t chr_bank_sel4[2];
            uint8_t chr_bank_sel8;

            uint8_t prg_bank_sel16[2];
            uint8_t prg_bank_sel32;

            uint8_t load_reg;
            uint8_t load_reg_count;
            uint8_t ctrl_reg;
        } data1;
        struct {
            uint8_t select_prg;
        } data2;
        struct {
            uint8_t select_chr;
        } data3;
        struct {
            uint8_t prg_bank;
        } data7;
        struct {
            uint8_t prg_bank;
            uint8_t chr_bank;
        } data66;
    };
    name_table_mirroring_t mirroring;
} nes_mapper_t;

typedef struct {
    uint16_t timer;
    uint16_t reload;
    uint8_t output;
    uint32_t sequence;
    uint32_t new_sequence;
} apu_sequencer_t;

typedef struct {
    bool start;
    bool disable;
    uint16_t divider_count;
    uint16_t volume;
    uint16_t output;
    uint16_t decay_count;
} apu_envelope_t;

typedef struct {
    bool enabled;
    bool down;
    bool reload;
    uint8_t shift;
    uint8_t timer;
    uint8_t period;
    uint16_t change;
    bool mute;
} apu_sweeper_t;

typedef struct {
    double frequency;
    double duty_cycle;
    double amplitude;
} pulse_t;

typedef struct {
    uint32_t clock_counter;
    uint32_t frame_clock_counter;
    double global_time;
    double audio_time;
    double audio_time_per_system_sample;
    double audio_time_per_nes_clock;
    float audio_sample;

    struct {
        apu_sequencer_t seq;
        apu_envelope_t env;
        apu_sweeper_t sweeper;
        pulse_t pulse;
        uint8_t len_counter;
        bool enable;
        bool halt;
        double output;
    } pulse[2];
    struct {
        apu_sequencer_t seq;
        apu_envelope_t env;
        uint8_t len_counter;
        bool enable;
        bool halt;
        double output;
    } noise;
} apu_t;

// NES emulator state
typedef struct {
    m6502_t cpu;
    r2c02_t ppu;
    uint16_t dma_wait;

    chips_debug_t debug;

    uint8_t ram[0x800];             // 2KB
    uint8_t extended_ram[0x2000];   // 8KB

    uint8_t ppu_ram[0x1000];        // 4KB
    uint8_t ppu_pal_ram[0x20];      // 32B
    uint16_t ppu_name_table[4];

    struct {
        nes_cartridge_header header;
        nes_mapper_t mapper;
        uint8_t character_ram[0x20000]; // 128KB
        uint8_t rom[0x40000];           // 256KB
    } cart;

    struct {
        uint32_t sample_rate;
        chips_audio_callback_t callback;
        int num_samples;
        int sample_pos;
        float sample_buffer[NES_MAX_AUDIO_SAMPLES];
    } audio;
    apu_t apu;

    controller_t controller[2];
    uint8_t controller_state[2];

    uint64_t pins;
    bool valid;

    alignas(64) uint8_t fb[PPU_FRAMEBUFFER_SIZE_BYTES];
} nes_t;

// initialize a new NES instance
void nes_init(nes_t* nes, const nes_desc_t* desc);
// reset a NES instance
void nes_reset(nes_t* nes);
// discard a NES instance
void nes_discard(nes_t* nes);
// get display requirements and framebuffer content, may be called with nullptr
chips_display_info_t nes_display_info(nes_t* nes);
// run NES instance for given amount of micro_seconds, returns number of ticks executed
uint32_t nes_exec(nes_t* nes, uint32_t micro_seconds);
void nes_key_down(nes_t* nes, int value);
void nes_key_up(nes_t* nes, int value);
// set pad mask (combination of NES_PAD_*)
void nes_pad(nes_t* sys, uint8_t mask);
// get current pad bitmask state
uint8_t nes_pad_mask(nes_t* sys);
// return true if a cartridge is currently inserted
bool nes_cartridge_inserted(nes_t* nes);
// remove current cartridge
void nes_remove_cartridge(nes_t* nes);

uint8_t nes_ppu_read(nes_t* nes, uint16_t addr);
void nes_ppu_write(nes_t* nes, uint16_t address, uint8_t data);
uint8_t nes_mem_read(nes_t* sys, uint16_t addr, bool read_only);
void nes_mem_write(nes_t* sys, uint16_t addr, uint8_t data);

#ifdef __cplusplus
} // extern "C"
#endif

/*-- IMPLEMENTATION ----------------------------------------------------------*/
#ifdef CHIPS_IMPL
#include <string.h>
#ifndef CHIPS_ASSERT
    #include <assert.h>
    #define CHIPS_ASSERT(c) assert(c)
#endif

#define _NES_FREQUENCY            (1789773)              // clock frequency in Hz

#define _PPUCTRL    (0x2000)
#define _PPUMASK    (0x2001)
#define _PPUSTATUS  (0x2002)
#define _OAMADDR    (0x2003)
#define _OAMDATA    (0x2004)
#define _PPUSCROL   (0x2005)
#define _PPUADDR    (0x2006)
#define _PPUDATA    (0x2007)

static const uint8_t length_table[] = {
    10, 254, 20,  2, 40,  4, 80,  6,
    160,   8, 60, 10, 14, 12, 26, 14,
    12,  16, 24, 18, 48, 20, 96, 22,
    192,  24, 72, 26, 16, 28, 32, 30
};

static uint8_t _ppu_read(uint16_t addr, void* user_data);
static void _ppu_write(uint16_t address, uint8_t data, void* user_data);
static void _ppu_set_pixels(uint8_t* buffer, void* user_data);
static uint64_t _nes_tick(nes_t* sys, uint64_t pins);
static bool _nes_use_mapper(nes_t* sys, uint8_t mapper_num);
static void _nes_mirroring(nes_t* sys);

static uint8_t _nes_read_prg0(uint16_t addr, void* user_data);
static void _nes_write_prg0(uint16_t addr, uint8_t value, void* user_data);
static uint8_t _nes_read_chr0(uint16_t addr, void* user_data);
static void _nes_write_chr0(uint16_t addr, uint8_t data, void* user_data);

static uint8_t _nes_read_prg1(uint16_t addr, void* user_data);
static void _nes_write_prg1(uint16_t addr, uint8_t value, void* user_data);
static uint8_t _nes_read_chr1(uint16_t addr, void* user_data);

static uint8_t _nes_read_prg2(uint16_t addr, void* user_data);
static void _nes_write_prg2(uint16_t addr, uint8_t value, void* user_data);

static uint8_t _nes_read_prg3(uint16_t addr, void* user_data);
static void _nes_write_prg3(uint16_t addr, uint8_t value, void* user_data);
static uint8_t _nes_read_chr3(uint16_t addr, void* user_data);

static uint8_t _nes_read_prg7(uint16_t addr, void* user_data);
static void _nes_write_prg7(uint16_t addr, uint8_t value, void* user_data);
static uint8_t _nes_read_chr7(uint16_t addr, void* user_data);
static void _nes_write_chr7(uint16_t addr, uint8_t data, void* user_data);

static uint8_t _nes_read_prg66(uint16_t addr, void* user_data);
static void _nes_write_prg66(uint16_t addr, uint8_t value, void* user_data);
static uint8_t _nes_read_chr66(uint16_t addr, void* user_data);

uint8_t nes_ppu_read(nes_t* nes, uint16_t address) {
    return _ppu_read(address, nes);
}

void nes_ppu_write(nes_t* nes, uint16_t address, uint8_t data) {
    _ppu_write(address, data, nes);
}

#define _NES_DEFAULT(val,def) (((val) != 0) ? (val) : (def))

void nes_init(nes_t* sys, const nes_desc_t* desc) {
    CHIPS_ASSERT(sys && desc);
    if (desc->debug.callback.func) { CHIPS_ASSERT(desc->debug.stopped); }
    memset(sys, 0, sizeof(nes_t));
    sys->valid = true;
    sys->debug = desc->debug;
    sys->audio.callback = desc->audio.callback;
    sys->audio.num_samples = _NES_DEFAULT(desc->audio.num_samples, NES_DEFAULT_AUDIO_SAMPLES);
    sys->audio.sample_rate = _NES_DEFAULT(desc->audio.sample_rate, NES_DEFAULT_AUDIO_SAMPLE_RATE);
    CHIPS_ASSERT(sys->audio.num_samples <= NES_MAX_AUDIO_SAMPLES);
 	sys->apu.audio_time_per_system_sample = 1.0 / (double)sys->audio.sample_rate;
	sys->apu.audio_time_per_nes_clock = 1.0 / (double)_NES_FREQUENCY; // APU Clock Frequency

    // initialize the CPU
    sys->pins = m6502_init(&sys->cpu, &(m6502_desc_t){
        .bcd_disabled = true
    });
    // initialize the PPU
    r2c02_init(&sys->ppu, &(r2c02_desc_t){
        .read = _ppu_read,
        .write = _ppu_write,
        .set_pixels = _ppu_set_pixels,
        .user_data = sys,
    });
    _nes_use_mapper(sys, 0);
}

bool nes_cartridge_inserted(nes_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    return sys->cart.header.magic[0];
}

void nes_remove_cartridge(nes_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    memset(&sys->cart, 0, sizeof(sys->cart));
    memset(&sys->ram, 0, sizeof(sys->ram));
    memset(&sys->extended_ram, 0, sizeof(sys->extended_ram));
    memset(&sys->ram, 0, sizeof(sys->ram));
    memset(&sys->ppu_ram, 0, sizeof(sys->ppu_ram));
    memset(&sys->ppu_pal_ram, 0, sizeof(sys->ppu_pal_ram));
    memset(&sys->ppu_name_table, 0, sizeof(sys->ppu_name_table));
    _nes_use_mapper(sys, 0);
    nes_reset(sys);
}

void nes_discard(nes_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    sys->valid = false;
}

void nes_reset(nes_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    sys->pins |= M6502_RES|M6502_SYNC;
    sys->apu.noise.seq.sequence = 0xdbdb;
    r2c02_reset(&sys->ppu);
}

uint32_t nes_exec(nes_t* sys, uint32_t micro_seconds) {
    CHIPS_ASSERT(sys && sys->valid);
    const uint32_t num_ticks = clk_us_to_ticks(_NES_FREQUENCY, micro_seconds);
    uint64_t pins = sys->pins;
    if (0 == sys->debug.callback.func) {
        // run without debug hook
        for (uint32_t tick = 0; tick < num_ticks; tick++) {
            pins = _nes_tick(sys, pins);
        }
    } else {
        // run with debug hook
        for (uint32_t tick = 0; (tick < num_ticks) && !(*sys->debug.stopped); tick++) {
            pins = _nes_tick(sys, pins);
            sys->debug.callback.func(sys->debug.callback.user_data, pins);
        }
    }
    sys->pins = pins;
    return num_ticks;
}

void nes_key_down(nes_t* sys, int value) {
    switch(value) {
        case 1: sys->controller[0].left =   1; break;
        case 2: sys->controller[0].right =  1; break;
        case 3: sys->controller[0].down =   1; break;
        case 4: sys->controller[0].up =     1; break;
        case 5: sys->controller[0].start =  1; break;
        case 6: sys->controller[0].a =      1; break;
        case 7: sys->controller[0].b =      1; break;
        case 8: sys->controller[0].select = 1; break;
    }
}

void nes_key_up(nes_t* sys, int value) {
    switch(value) {
        case 1: sys->controller[0].left =   0; break;
        case 2: sys->controller[0].right =  0; break;
        case 3: sys->controller[0].down =   0; break;
        case 4: sys->controller[0].up =     0; break;
        case 5: sys->controller[0].start =  0; break;
        case 6: sys->controller[0].a =      0; break;
        case 7: sys->controller[0].b =      0; break;
        case 8: sys->controller[0].select = 0; break;
    }
}

void nes_pad(nes_t* sys, uint8_t mask) {
    CHIPS_ASSERT(sys && sys->valid);
    sys->controller[0].value = mask;
}

uint8_t nes_pad_mask(nes_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    return sys->controller[0].value;
}

chips_display_info_t nes_display_info(nes_t* sys) {
    const chips_display_info_t res = {
        .frame = {
            .dim = {
                .width = PPU_DISPLAY_WIDTH,
                .height = PPU_DISPLAY_HEIGHT,
            },
            .bytes_per_pixel = 1,
            .buffer = {
                .ptr = sys ? sys->fb : 0,
                .size = PPU_FRAMEBUFFER_SIZE_BYTES,
            }
        },
        .screen = {
            .x = 0,
            .y = 0,
            .width = PPU_DISPLAY_WIDTH,
            .height = PPU_DISPLAY_HEIGHT,
        },
        .palette = {
            .ptr = (void*)ppu_palette,
            .size = sizeof(ppu_palette)
        }
    };
    return res;
}

bool nes_insert_cart(nes_t* sys, chips_range_t data) {
    CHIPS_ASSERT(sys && sys->valid);
    if (data.size <= sizeof(nes_cartridge_header)) {
        return false;
    }
    const nes_cartridge_header* hdr = (nes_cartridge_header*) data.ptr;
    if(strncmp(hdr->magic, "NES\x1A", 4))
        return false;

    // not supported
    const uint8_t mapper_num = hdr->mapper_low | (hdr->mapper_hi << 4);
    if(hdr->trainer || hdr->vram_expansion || hdr->prg_page_count > 16 || hdr->tile_page_count > 16)
        return false;

    // read PRG-ROM (16KB banks)
    const size_t prg_size = hdr->prg_page_count * 0x4000;
    memcpy(&sys->cart.header, hdr, sizeof(nes_cartridge_header));
    memcpy(sys->cart.rom, data.ptr + sizeof(nes_cartridge_header), prg_size);

    // read CHR-ROM (8KB banks)
    if(hdr->tile_page_count > 0) {
        memcpy(sys->cart.character_ram, data.ptr + sizeof(nes_cartridge_header) + prg_size, hdr->tile_page_count * 0x2000);
    }

    if(_nes_use_mapper(sys, mapper_num)) {
        nes_reset(sys);
        return true;
    }
    return false;
}

static uint8_t _ppu_read(uint16_t addr, void* user_data) {
    nes_t* sys = (nes_t*)user_data;
    CHIPS_ASSERT(sys && sys->valid);
    if (addr < 0x2000) {
        return sys->cart.mapper.read_chr(addr, sys);
    } else if(addr < 0x3f00) {
        const uint16_t index = addr & 0x3ff;
        // Name tables upto 0x3000, then mirrored upto 3eff
        uint16_t normalizedAddr = addr;
        if (addr >= 0x3000)
            normalizedAddr -= 0x1000;
        if(sys->ppu_name_table[0] >= 0x2c00) {
            return sys->cart.mapper.read_chr(normalizedAddr, sys);
        } else
        {
            if (normalizedAddr < 0x2400)      //NT0
                normalizedAddr = sys->ppu_name_table[0] + index;
            else if (normalizedAddr < 0x2800) //NT1
                normalizedAddr = sys->ppu_name_table[1] + index;
            else if (normalizedAddr < 0x2c00) //NT2
                normalizedAddr = sys->ppu_name_table[2] + index;
            else
                normalizedAddr = sys->ppu_name_table[3] + index;
            return sys->ppu_ram[normalizedAddr-0x2000];
        }
    } else if(addr <= 0x3f0c && addr % 4 == 0) {
        return sys->ppu_pal_ram[0];
    } else if (addr < 0x4000) {
        uint16_t normalizedAddr = addr & 0x1f;
        // Addresses $3F10/$3F14/$3F18/$3F1C are mirrors of $3F00/$3F04/$3F08/$3F0C
        if (normalizedAddr >= 0x10 && addr % 4 == 0) {
            normalizedAddr &= 0xf;
        }
        return sys->ppu_pal_ram[normalizedAddr];
    }
    return 0;
}

static void _ppu_write(uint16_t addr, uint8_t data, void* user_data) {
    nes_t* sys = (nes_t*)user_data;
    CHIPS_ASSERT(sys && sys->valid);
    if (addr < 0x2000) {
        sys->cart.mapper.write_chr(addr, data, sys);
    } else if (addr < 0x3f00) {
        const uint16_t index = addr & 0x3ff;
        // Name tables upto 0x3000, then mirrored upto 3eff
        uint16_t normalizedAddr = addr;
        if (addr >= 0x3000)
            normalizedAddr -= 0x1000;
        if(sys->ppu_name_table[0] >= 0x2c00) {
            sys->cart.mapper.write_chr(normalizedAddr, data, sys);
        } else
        {
            if (normalizedAddr < 0x2400)      //NT0
                normalizedAddr = sys->ppu_name_table[0] + index;
            else if (normalizedAddr < 0x2800) //NT1
                normalizedAddr = sys->ppu_name_table[1] + index;
            else if (normalizedAddr < 0x2c00) //NT2
                normalizedAddr = sys->ppu_name_table[2] + index;
            else
                normalizedAddr = sys->ppu_name_table[3] + index;
            sys->ppu_ram[normalizedAddr-0x2000] = data;
        }
    } else if (addr < 0x4000) {
        uint16_t normalizedAddr = addr & 0x1f;
        // Addresses $3F10/$3F14/$3F18/$3F1C are mirrors of $3F00/$3F04/$3F08/$3F0C
        if (normalizedAddr >= 0x10 && addr % 4 == 0) {
            normalizedAddr &= 0xf;
        }
        sys->ppu_pal_ram[normalizedAddr] = data;
   }
}

static void _ppu_set_pixels(uint8_t* buffer, void* user_data) {
    nes_t* sys = (nes_t*)user_data;
    CHIPS_ASSERT(sys && sys->valid);
    memcpy(sys->fb, buffer, 256*240);
}

uint8_t nes_mem_read(nes_t* sys, uint16_t addr, bool read_only) {
    if(addr < 0x2000) {
        return sys->ram[addr & 0x7ff];
    } else if (addr >= 0x4016 && addr <= 0x4017) {
        uint8_t data = (sys->controller_state[addr & 0x0001] & 0x80) ? 1 : 0;
        sys->controller_state[addr & 0x0001] <<= 1;
        return data;
    } else if (addr < 0x4020) {
        if (addr < 0x4000) //PPU registers, mirrored
            addr = addr & 0x2007;
        return r2c02_read(&sys->ppu, addr-0x2000, read_only);
    } else if (addr < 0x6000) {
        // TODO: Expansion ROM
        return 0xFF;
    } else if (addr < 0x8000) {
        return sys->extended_ram[addr - 0x6000];
    } else {
        return sys->cart.mapper.read_prg(addr, sys);
    }
}

void nes_mem_write(nes_t* sys, uint16_t addr, uint8_t data) {
    if(addr < 0x2000) {
        sys->ram[addr & 0x7ff] = data;
    } else if (addr < 0x4000) {
        //PPU registers, mirrored
        addr = addr & 0x0007;
        r2c02_write(&sys->ppu, addr, data);
    } else if(addr == 0x4000) {
        switch ((data & 0xc0) >> 6) {
        case 0x00: sys->apu.pulse[0].seq.new_sequence = 0b01000000; sys->apu.pulse[0].pulse.duty_cycle = 0.125; break;
        case 0x01: sys->apu.pulse[0].seq.new_sequence = 0b01100000; sys->apu.pulse[0].pulse.duty_cycle = 0.250; break;
        case 0x02: sys->apu.pulse[0].seq.new_sequence = 0b01111000; sys->apu.pulse[0].pulse.duty_cycle = 0.500; break;
        case 0x03: sys->apu.pulse[0].seq.new_sequence = 0b10011111; sys->apu.pulse[0].pulse.duty_cycle = 0.750; break;
        }
        sys->apu.pulse[0].seq.sequence = sys->apu.pulse[0].seq.new_sequence;
        sys->apu.pulse[0].halt = (data & 0x20);
        sys->apu.pulse[0].env.volume = (data & 0x0f);
		sys->apu.pulse[0].env.disable = (data & 0x10);
    } else if(addr == 0x4001) {
        sys->apu.pulse[0].sweeper.enabled = data & 0x80;
		sys->apu.pulse[0].sweeper.period = (data & 0x70) >> 4;
		sys->apu.pulse[0].sweeper.down = data & 0x08;
		sys->apu.pulse[0].sweeper.shift = data & 0x07;
		sys->apu.pulse[0].sweeper.reload = true;
    } else if(addr == 0x4002) {
        sys->apu.pulse[0].seq.reload = (sys->apu.pulse[0].seq.reload & 0xff00) | data;
    } else if(addr == 0x4003) {
        sys->apu.pulse[0].seq.reload = (uint16_t)((data & 0x07)) << 8 | (sys->apu.pulse[0].seq.reload & 0x00ff);
        sys->apu.pulse[0].seq.timer = sys->apu.pulse[0].seq.reload;
        sys->apu.pulse[0].seq.sequence = sys->apu.pulse[0].seq.new_sequence;
        sys->apu.pulse[0].len_counter = length_table[(data & 0xf8) >> 3];
        sys->apu.pulse[0].env.start = true;
    } else if(addr == 0x4004) {
        switch ((data & 0xc0) >> 6) {
        case 0x00: sys->apu.pulse[1].seq.new_sequence = 0b01000000; sys->apu.pulse[1].pulse.duty_cycle = 0.125; break;
        case 0x01: sys->apu.pulse[1].seq.new_sequence = 0b01100000; sys->apu.pulse[1].pulse.duty_cycle = 0.250; break;
        case 0x02: sys->apu.pulse[1].seq.new_sequence = 0b01111000; sys->apu.pulse[1].pulse.duty_cycle = 0.500; break;
        case 0x03: sys->apu.pulse[1].seq.new_sequence = 0b10011111; sys->apu.pulse[1].pulse.duty_cycle = 0.750; break;
        }
        sys->apu.pulse[1].seq.sequence = sys->apu.pulse[1].seq.new_sequence;
        sys->apu.pulse[1].halt = (data & 0x20);
        sys->apu.pulse[1].env.volume = (data & 0x0f);
		sys->apu.pulse[1].env.disable = (data & 0x10);
    } else if(addr == 0x4005) {
        sys->apu.pulse[1].sweeper.enabled = data & 0x80;
		sys->apu.pulse[1].sweeper.period = (data & 0x70) >> 4;
		sys->apu.pulse[1].sweeper.down = data & 0x08;
		sys->apu.pulse[1].sweeper.shift = data & 0x07;
		sys->apu.pulse[1].sweeper.reload = true;
    } else if(addr == 0x4006) {
        sys->apu.pulse[1].seq.reload = (sys->apu.pulse[1].seq.reload & 0xff00) | data;
    } else if(addr == 0x4007) {
        sys->apu.pulse[1].seq.reload = (uint16_t)((data & 0x07)) << 8 | (sys->apu.pulse[1].seq.reload & 0x00ff);
        sys->apu.pulse[1].seq.timer = sys->apu.pulse[1].seq.reload;
        sys->apu.pulse[1].seq.sequence = sys->apu.pulse[1].seq.new_sequence;
        sys->apu.pulse[1].len_counter = length_table[(data & 0xf8) >> 3];
        sys->apu.pulse[1].env.start = true;
    } else if(addr == 0x400c) {
        sys->apu.noise.env.volume = (data & 0x0F);
        sys->apu.noise.env.disable = (data & 0x10);
		sys->apu.noise.halt = (data & 0x20);
    } else if(addr == 0x400e) {
		switch (data & 0x0f) {
		case 0x00: sys->apu.noise.seq.reload = 0; break;
		case 0x01: sys->apu.noise.seq.reload = 4; break;
		case 0x02: sys->apu.noise.seq.reload = 8; break;
		case 0x03: sys->apu.noise.seq.reload = 16; break;
		case 0x04: sys->apu.noise.seq.reload = 32; break;
		case 0x05: sys->apu.noise.seq.reload = 64; break;
		case 0x06: sys->apu.noise.seq.reload = 96; break;
		case 0x07: sys->apu.noise.seq.reload = 128; break;
		case 0x08: sys->apu.noise.seq.reload = 160; break;
		case 0x09: sys->apu.noise.seq.reload = 202; break;
		case 0x0A: sys->apu.noise.seq.reload = 254; break;
		case 0x0B: sys->apu.noise.seq.reload = 380; break;
		case 0x0C: sys->apu.noise.seq.reload = 508; break;
		case 0x0D: sys->apu.noise.seq.reload = 1016; break;
		case 0x0E: sys->apu.noise.seq.reload = 2034; break;
		case 0x0F: sys->apu.noise.seq.reload = 4068; break;
		}
    } else if(addr == 0x400f) {
        sys->apu.pulse[0].env.start = true;
        sys->apu.pulse[1].env.start = true;
        sys->apu.noise.env.start = true;
		sys->apu.noise.len_counter = length_table[(data & 0xf8) >> 3];
    } else if(addr == 0x4014) {
        sys->dma_wait = 256;
        // OAMDMA
        uint16_t page = data << 8;
        if (page < 0x2000) {
            uint8_t* page_ptr = sys->ram + (page & 0x7ff);
            memcpy(sys->ppu.oam.reg + sys->ppu.sprite_data_address, page_ptr, 256 - sys->ppu.sprite_data_address);
            if (sys->ppu.sprite_data_address)
                memcpy(sys->ppu.oam.reg, page_ptr + (256 - sys->ppu.sprite_data_address), sys->ppu.sprite_data_address);
        }
    } else if(addr == 0x4015) {
        sys->apu.pulse[0].enable = data & 1;
        sys->apu.pulse[1].enable = data & 2;
        sys->apu.noise.enable = data & 0x04;
    } else if (addr >= 0x4016 && addr <= 0x4017) {
        sys->controller_state[addr & 0x0001] = sys->controller[addr & 0x0001].value;
    } else if (addr < 0x6000) {
        // TODO: Expansion ROM
    } else if (addr < 0x8000) {
        sys->extended_ram[addr - 0x6000] = data;
    } else {
        sys->cart.mapper.write_prg(addr, data, sys);
    }
}

bool nes_load_snapshot(nes_t* sys, uint32_t version, nes_t* src) {
    CHIPS_ASSERT(sys && src);
    if (version != NES_SNAPSHOT_VERSION) {
        return false;
    }
    static nes_t im;
    im = *src;
    chips_debug_snapshot_onload(&im.debug, &sys->debug);
    chips_audio_callback_snapshot_onload(&im.audio.callback, &sys->audio.callback);
    *sys = im;
    return true;
}

uint32_t nes_save_snapshot(nes_t* sys, nes_t* dst) {
    CHIPS_ASSERT(sys && dst);
    *dst = *sys;
    chips_debug_snapshot_onsave(&dst->debug);
    chips_audio_callback_snapshot_onsave(&dst->audio.callback);
    return NES_SNAPSHOT_VERSION;
}

static inline double _approx_sin(double t) {
    double j = t * 0.15915;
    j = j - (int)j;
    return 20.785 * j * (j - 0.5) * (j - 1.0);
}

static double _pulse_sample(pulse_t* pulse, double t) {
    double a = 0;
    double b = 0;
    double p = pulse->duty_cycle * 2.0 * M_PI;

    const double harmonics = 20;
    for (double n = 1; n < harmonics; n++) {
        double c = n * pulse->frequency * 2.0 * M_PI * t;
        a += -_approx_sin(c) / n;
        b += -_approx_sin(c - p * n) / n;
    }

    return (2.0 * pulse->amplitude / M_PI) * (a - b);
}

static void _apu_env_clock(apu_envelope_t* env, bool loop) {
    if (!env->start) {
        if (env->divider_count == 0) {
            env->divider_count = env->volume;

            if (env->decay_count == 0) {
                if (loop) env->decay_count = 15;
            }
            else
                env->decay_count--;
        }
        else
            env->divider_count--;
    } else {
        env->start = false;
        env->decay_count = 15;
        env->divider_count = env->volume;
    }

    env->output = env->disable ? env->volume : env->decay_count;
}

static void _apu_sweeper_track(apu_sweeper_t* sweeper, uint16_t target) {
    if (sweeper->enabled) {
        sweeper->change = target >> sweeper->shift;
        sweeper->mute = (target < 8) || (target > 0x7FF);
    }
}

static bool _apu_sweeper_clock(apu_sweeper_t* sweeper, uint16_t* target, bool channel) {
    bool changed = false;
    if (sweeper->timer == 0 && sweeper->enabled && sweeper->shift > 0 && !sweeper->mute) {
        if (*target >= 8 && sweeper->change < 0x07FF) {
            if (sweeper->down) {
                *target -= sweeper->change - channel;
            } else {
                *target += sweeper->change;
            }
            changed = true;
        }
    }

    if (sweeper->timer == 0 || sweeper->reload) {
        sweeper->timer = sweeper->period;
        sweeper->reload = false;
    } else {
       sweeper-> timer--;
    }

    sweeper->mute = (*target < 8) || (*target > 0x7FF);

    return changed;
}

static void _apu_len_counter_clock(bool enable, uint8_t* counter, bool halt) {
    if (!enable)
        *counter = 0;
    else if (*counter > 0 && !halt)
        (*counter)--;
}

typedef uint32_t (*_apu_seq_func)(uint32_t s);

static uint32_t _apu_pulse_seq(uint32_t s) {
    s = ((s & 0x0001) << 7) | ((s & 0x00fe) >> 1);
    return s;
}

static uint32_t _apu_noise_seq(uint32_t s) {
    s = (((s & 0x0001) ^ ((s & 0x0002) >> 1)) << 14) | ((s & 0x7fff) >> 1);
    return s;
}

static uint8_t _apu_seq_clock(apu_sequencer_t* seq, bool enable, _apu_seq_func func) {
    if (enable) {
        seq->timer--;
        if (seq->timer == 0xffff) {
            seq->timer = seq->reload;
            seq->sequence = func(seq->sequence);
            seq->output = seq->sequence & 0x00000001;
        }
    }
    return seq->output;
}

static bool _apu_tick(apu_t* sys) {
    bool quarter_frame_clock = false;
    bool half_frame_clock = false;
    bool audio_sample_ready = false;

    if (sys->clock_counter % 2 == 0) {
        sys->frame_clock_counter++;

        // 4-Step Sequence Mode
        switch (sys->frame_clock_counter) {
            case 3729:
                quarter_frame_clock = true;
                break;
            case 7457:
                quarter_frame_clock = true;
                half_frame_clock = true;
                break;
            case 11186:
                quarter_frame_clock = true;
                break;
            case 14916:
                quarter_frame_clock = true;
                half_frame_clock = true;
                sys->frame_clock_counter = 0;
                break;
        }

        // Quater frame "beats" adjust the volume envelope
        if (quarter_frame_clock) {
            _apu_env_clock(&sys->pulse[0].env, sys->pulse[0].halt);
            _apu_env_clock(&sys->pulse[1].env, sys->pulse[1].halt);
            _apu_env_clock(&sys->noise.env, sys->noise.halt);
        }

        // Half frame "beats" adjust the note length and
        // frequency sweepers
        if (half_frame_clock) {
            _apu_len_counter_clock(sys->pulse[0].enable, &sys->pulse[0].len_counter, sys->pulse[0].halt);
            _apu_sweeper_clock(&sys->pulse[0].sweeper, &sys->pulse[0].seq.reload, 0);

            _apu_len_counter_clock(sys->pulse[1].enable, &sys->pulse[1].len_counter, sys->pulse[1].halt);
            _apu_sweeper_clock(&sys->pulse[1].sweeper, &sys->pulse[1].seq.reload, 0);

            _apu_len_counter_clock(sys->noise.enable, &sys->noise.len_counter, sys->noise.halt);
        }

        // pulse 1
        _apu_seq_clock(&sys->pulse[0].seq, sys->pulse[0].enable, _apu_pulse_seq);
        sys->pulse[0].pulse.frequency = (double)_NES_FREQUENCY / (16.0 * (double)(sys->pulse[0].seq.reload + 1));
        sys->pulse[0].pulse.amplitude = (double)(sys->pulse[0].env.output - 1) / 16.0;
        float pulse1_sample = (float)(_pulse_sample(&sys->pulse[0].pulse, sys->global_time));

        if (sys->pulse[0].len_counter > 0 && sys->pulse[0].seq.timer >= 8 && !sys->pulse[0].sweeper.mute && sys->pulse[0].env.output > 2)
            sys->pulse[0].output += (pulse1_sample - sys->pulse[0].output) * 0.5;
        else
            sys->pulse[0].output = 0;

        if (!sys->pulse[0].enable) sys->pulse[0].output = 0;

        // pulse 2
        _apu_seq_clock(&sys->pulse[1].seq, sys->pulse[1].enable, _apu_pulse_seq);
        sys->pulse[1].pulse.frequency = (double)_NES_FREQUENCY / (16.0 * (double)(sys->pulse[1].seq.reload + 1));
        sys->pulse[1].pulse.amplitude = (double)(sys->pulse[1].env.output - 1) / 16.0;
        float pulse2_sample = (float)(_pulse_sample(&sys->pulse[1].pulse, sys->global_time));

        if (sys->pulse[1].len_counter > 0 && sys->pulse[1].seq.timer >= 8 && !sys->pulse[1].sweeper.mute && sys->pulse[1].env.output > 2)
            sys->pulse[1].output += (pulse2_sample - sys->pulse[1].output) * 0.5;
        else
            sys->pulse[1].output = 0;

        if (!sys->pulse[1].enable) sys->pulse[1].output = 0;

        // noise
        _apu_seq_clock(&sys->noise.seq, sys->noise.enable, _apu_noise_seq);
        if (sys->noise.len_counter > 0 && sys->noise.seq.timer >= 8) {
			sys->noise.output = (double)sys->noise.seq.output * ((double)(sys->noise.env.output-1) / 16.0);
		}
        if (!sys->noise.enable) sys->noise.output = 0;
    }

    // Frequency sweepers change at high frequency
	_apu_sweeper_track(&sys->pulse[0].sweeper, sys->pulse[0].seq.reload);
	_apu_sweeper_track(&sys->pulse[1].sweeper, sys->pulse[1].seq.reload);

    // mix
    sys->audio_sample =
        ((1.0 * sys->pulse[0].output) - 0.8) * 0.1 +
        ((1.0 * sys->pulse[1].output) - 0.8) * 0.1 +
        ((2.0 * (sys->noise.output - 0.5))) * 0.1;

    // Synchronising with Audio
    if (sys->audio_time >= sys->audio_time_per_system_sample) {
        sys->audio_time -= sys->audio_time_per_system_sample;
        audio_sample_ready = true;
    }

    sys->clock_counter++;
    sys->global_time += sys->audio_time_per_nes_clock;
    sys->audio_time += sys->audio_time_per_nes_clock;

    return audio_sample_ready;
}

static uint64_t _nes_tick(nes_t* sys, uint64_t pins) {
    if(sys->ppu.request_nmi) {
        pins |= M6502_NMI;
        sys->ppu.request_nmi = false;
    }
    if(sys->ppu.request_irq) {
        pins |= M6502_IRQ;
        sys->ppu.request_irq = false;
    }
    if(sys->dma_wait) {
        sys->dma_wait--;
    } else {
        pins = m6502_tick(&sys->cpu, pins);
        pins &= ~M6502_NMI;

        // extract 16-bit address from pin mask
        uint16_t addr = M6502_GET_ADDR(pins);
        // perform memory access
        if (pins & M6502_RW) {
            // a memory read
            M6502_SET_DATA(pins, nes_mem_read(sys, addr, false));
        }
        else {
            // a memory write
            nes_mem_write(sys, addr, M6502_GET_DATA(pins));
        }
    }

    // tick the sound chip...
    if(_apu_tick(&sys->apu)) {
        // new sound sample ready
        sys->audio.sample_buffer[sys->audio.sample_pos++] = sys->apu.audio_sample;
        if (sys->audio.sample_pos == sys->audio.num_samples) {
            if (sys->audio.callback.func) {
                // new sample packet is ready
                sys->audio.callback.func(sys->audio.sample_buffer, sys->audio.num_samples, sys->audio.callback.user_data);
            }
            sys->audio.sample_pos = 0;
        }
    }
    r2c02_tick(&sys->ppu, pins);
    r2c02_tick(&sys->ppu, pins);
    r2c02_tick(&sys->ppu, pins);

    return pins;
}

// *************************
// ********* MAPPERS *******
// *************************
static bool _nes_use_mapper(nes_t* sys, uint8_t mapper_num) {
    bool supported = true;
    memset(&sys->cart.mapper, 0, sizeof(sys->cart.mapper));
    sys->cart.mapper = (nes_mapper_t){
        .read_prg = _nes_read_prg0,
        .write_prg = _nes_write_prg0,
        .read_chr = _nes_read_chr0,
        .write_chr = _nes_write_chr0,
    };
    switch(mapper_num) {
        case 0:
            break;
        case 1:
            sys->cart.mapper = (nes_mapper_t) {
                .data1 = {
                    .ctrl_reg = 0x1c,
                    .prg_bank_sel16[1] = sys->cart.header.prg_page_count - 1,
                },
                .read_prg = _nes_read_prg1,
                .write_prg = _nes_write_prg1,
                .read_chr = _nes_read_chr1,
                .write_chr = _nes_write_chr0,
            };
            break;
        case 2:
            sys->cart.mapper = (nes_mapper_t) {
                .read_prg = _nes_read_prg2,
                .write_prg = _nes_write_prg2,
                .read_chr = _nes_read_chr0,
                .write_chr = _nes_write_chr0,
            };
            break;
        case 3:
            sys->cart.mapper = (nes_mapper_t) {
                .read_prg = _nes_read_prg3,
                .write_prg = _nes_write_prg3,
                .read_chr = _nes_read_chr3,
                .write_chr = _nes_write_chr0,
            };
            break;
        case 7:
            sys->cart.mapper = (nes_mapper_t) {
                .mirroring = OneScreenLower,
                .read_prg = _nes_read_prg7,
                .write_prg = _nes_write_prg7,
                .read_chr = _nes_read_chr7,
                .write_chr = _nes_write_chr7,
            };
            break;
        case 66:
            sys->cart.mapper = (nes_mapper_t) {
                .read_prg = _nes_read_prg66,
                .write_prg = _nes_write_prg66,
                .read_chr = _nes_read_chr66,
                .write_chr = _nes_write_chr0,
            };
            break;
        default:
            supported = false;
            break;
    }
    sys->cart.mapper.mirroring = sys->cart.header.mirror_mode ? Vertical : Horizontal;
    _nes_mirroring(sys);
    return supported;
}

static void _nes_mirroring(nes_t* sys) {
    switch (sys->cart.mapper.mirroring) {
        case Horizontal:
            sys->ppu_name_table[0] = sys->ppu_name_table[1] = 0x2000;
            sys->ppu_name_table[2] = sys->ppu_name_table[3] = 0x2400;
            break;
        case Vertical:
            sys->ppu_name_table[0] = sys->ppu_name_table[2] = 0x2000;
            sys->ppu_name_table[1] = sys->ppu_name_table[3] = 0x2400;
            break;
        case OneScreenLower:
            sys->ppu_name_table[0] = sys->ppu_name_table[1] = sys->ppu_name_table[2] = sys->ppu_name_table[3] = 0x2000;
            break;
        case OneScreenHigher:
            sys->ppu_name_table[0] = sys->ppu_name_table[1] = sys->ppu_name_table[2] = sys->ppu_name_table[3] = 0x2400;
            break;
        case FourScreen:
            sys->ppu_name_table[0] = 0x2800;
            break;
        default:
            sys->ppu_name_table[0] = sys->ppu_name_table[1] = sys->ppu_name_table[2] = sys->ppu_name_table[3] = 0x2000;
    }
}

// ********* MAPPER 0 **************

static uint8_t _nes_read_prg0(uint16_t addr, void* user_data) {
    nes_t* sys = (nes_t*)user_data;
    CHIPS_ASSERT(sys && sys->valid);
    addr -= 0x8000;
    if(sys->cart.header.prg_page_count == 1) {
        addr &= 0x3fff;
    }
    return sys->cart.rom[addr];
}

static void _nes_write_prg0(uint16_t addr, uint8_t value, void* user_data) {
    (void)addr; (void)value; (void)user_data;
}

static uint8_t _nes_read_chr0(uint16_t addr, void* user_data) {
    nes_t* sys = (nes_t*)user_data;
    CHIPS_ASSERT(sys && sys->valid);
    return sys->cart.character_ram[addr];
}

static void _nes_write_chr0(uint16_t addr, uint8_t data, void* user_data) {
    nes_t* sys = (nes_t*)user_data;
    CHIPS_ASSERT(sys && sys->valid);
    sys->cart.character_ram[addr] = data;
}

// ********* MAPPER 1 **************

static uint8_t _nes_read_prg1(uint16_t addr, void* user_data) {
    nes_t* sys = (nes_t*)user_data;
    CHIPS_ASSERT(sys && sys->valid);
    uint32_t mapped_addr;
    if (sys->cart.mapper.data1.ctrl_reg & 0b01000) {
        // 16K Mode
        int sel = (addr < 0xc000) ? 0 : 1;
        mapped_addr = sys->cart.mapper.data1.prg_bank_sel16[sel] * 0x4000 + (addr & 0x3fff);
    } else {
        // 32K Mode
        mapped_addr = sys->cart.mapper.data1.prg_bank_sel32 * 0x8000 + (addr & 0x7FFF);
    }
    return sys->cart.rom[mapped_addr];
}

static uint8_t _nes_read_chr1(uint16_t addr, void* user_data) {
    nes_t* sys = (nes_t*)user_data;
    CHIPS_ASSERT(sys && sys->valid);
    if (sys->cart.header.tile_page_count != 0) {
        if (sys->cart.mapper.data1.ctrl_reg & 0x10) {
            // 4K CHR Bank Mode
            int sel = (addr < 0x1000) ? 0 : 1;
            addr = sys->cart.mapper.data1.chr_bank_sel4[sel] * 0x1000 + (addr & 0x0fff);
        } else {
            // 8K CHR Bank Mode
            addr = sys->cart.mapper.data1.chr_bank_sel8 * 0x2000 + (addr & 0x1FFF);
        }
    }
    return sys->cart.character_ram[addr];
}

static void _nes_write_prg1(uint16_t addr, uint8_t data, void* user_data) {
    nes_t* sys = (nes_t*)user_data;
    CHIPS_ASSERT(sys && sys->valid);
    if(data & 0x80) {
        sys->cart.mapper.data1.load_reg = 0x00;
        sys->cart.mapper.data1.load_reg_count = 0;
        sys->cart.mapper.data1.ctrl_reg = sys->cart.mapper.data1.ctrl_reg | 0x0c;
    } else {
        sys->cart.mapper.data1.load_reg >>= 1;
        sys->cart.mapper.data1.load_reg |= (data & 0x01) << 4;
        sys->cart.mapper.data1.load_reg_count++;
        if (sys->cart.mapper.data1.load_reg_count == 5) {
            uint8_t tgt_reg = (addr >> 13) & 0x03;

            switch (tgt_reg) {
                case 0:
                    // 0x8000 - 0x9FFF: Set Control Register
                    sys->cart.mapper.data1.ctrl_reg = sys->cart.mapper.data1.load_reg & 0x1f;

                    switch (sys->cart.mapper.data1.ctrl_reg & 0x03)
                    {
                    case 0: sys->cart.mapper.mirroring = OneScreenLower; break;
                    case 1: sys->cart.mapper.mirroring = OneScreenHigher; break;
                    case 2: sys->cart.mapper.mirroring = Vertical;     break;
                    case 3: sys->cart.mapper.mirroring = Horizontal;   break;
                    }
                    break;
                case 1:
                    // 0xA000 - 0xBFFF: Set CHR Bank Lo
                    if (sys->cart.mapper.data1.ctrl_reg & 0b10000) {
                        // 4K CHR Bank at PPU 0x0000
                        sys->cart.mapper.data1.chr_bank_sel4[0] = sys->cart.mapper.data1.load_reg & 0x1f;
                    } else {
                        // 8K CHR Bank at PPU 0x0000
                        sys->cart.mapper.data1.chr_bank_sel8 = sys->cart.mapper.data1.load_reg & 0x1e;
                    }
                    break;
                case 2:
                    // 0xC000 - 0xDFFF: Set CHR Bank Hi
                    if (sys->cart.mapper.data1.ctrl_reg & 0b10000) {
                        // 4K CHR Bank at PPU 0x1000
                        sys->cart.mapper.data1.chr_bank_sel4[1] = sys->cart.mapper.data1.load_reg & 0x1f;
                    }
                    break;
                case 3: {
                    // 0xE000 - 0xFFFF: Configure PRG Banks
                    uint8_t prg_mode = (sys->cart.mapper.data1.ctrl_reg >> 2) & 0x03;

                    if (prg_mode == 0 || prg_mode == 1) {
                        // Set 32K PRG Bank at CPU 0x8000
                        sys->cart.mapper.data1.prg_bank_sel32 = (sys->cart.mapper.data1.load_reg & 0x0e) >> 1;
                    } else if (prg_mode == 2) {
                        // Fix 16KB PRG Bank at CPU 0x8000 to First Bank
                        sys->cart.mapper.data1.prg_bank_sel16[0] = 0;
                        // Set 16KB PRG Bank at CPU 0xC000
                        sys->cart.mapper.data1.prg_bank_sel16[1] = sys->cart.mapper.data1.load_reg & 0x0f;
                    } else if (prg_mode == 3) {
                        // Set 16KB PRG Bank at CPU 0x8000
                        sys->cart.mapper.data1.prg_bank_sel16[0] = sys->cart.mapper.data1.load_reg & 0x0f;
                        // Fix 16KB PRG Bank at CPU 0xC000 to Last Bank
                        sys->cart.mapper.data1.prg_bank_sel16[1] = sys->cart.header.prg_page_count - 1;
                    }
                } break;
            }

            // reset load register
            sys->cart.mapper.data1.load_reg = 0x00;
            sys->cart.mapper.data1.load_reg_count = 0;
        }
    }
}

// ********* MAPPER 2 **************

static uint8_t _nes_read_prg2(uint16_t addr, void* user_data) {
    nes_t* sys = (nes_t*)user_data;
    CHIPS_ASSERT(sys && sys->valid);
    uint16_t page;
    if(addr < 0xc000) {
        // CPU $8000-$BFFF: 16 KB switchable PRG ROM bank
        page = sys->cart.mapper.data2.select_prg;
    } else {
        // CPU $C000-$FFFF: 16 KB PRG ROM bank, fixed to the last bank
        page = sys->cart.header.prg_page_count - 1;
    }
    return sys->cart.rom[(page << 14) + (addr & 0x3fff)];
}

static void _nes_write_prg2(uint16_t addr, uint8_t value, void* user_data) {
    (void)addr;
    nes_t* sys = (nes_t*)user_data;
    CHIPS_ASSERT(sys && sys->valid);
    CHIPS_ASSERT(value < sys->cart.header.prg_page_count);
    sys->cart.mapper.data2.select_prg = value;
}

// ********* MAPPER 3 **************

static uint8_t _nes_read_prg3(uint16_t addr, void* user_data) {
    nes_t* sys = (nes_t*)user_data;
    CHIPS_ASSERT(sys && sys->valid);
    if (sys->cart.header.prg_page_count == 2)
        return sys->cart.rom[addr - 0x8000];
    else if (sys->cart.header.prg_page_count == 1)
        return sys->cart.rom[(addr - 0x8000) & 0x3fff];
    else
        return 0;
}

static void _nes_write_prg3(uint16_t addr, uint8_t value, void* user_data) {
    (void)addr;
    nes_t* sys = (nes_t*)user_data;
    CHIPS_ASSERT(sys && sys->valid);
    sys->cart.mapper.data3.select_chr = value & 0x3;
}

static uint8_t _nes_read_chr3(uint16_t addr, void* user_data) {
    nes_t* sys = (nes_t*)user_data;
    CHIPS_ASSERT(sys && sys->valid);
    return sys->cart.character_ram[addr | (sys->cart.mapper.data3.select_chr << 13)];
}

// ********* MAPPER 7 **************

static uint8_t _nes_read_prg7(uint16_t addr, void* user_data) {
    nes_t* sys = (nes_t*)user_data;
    CHIPS_ASSERT(sys && sys->valid);
    if(addr >= 0x8000) {
        return sys->cart.rom[(sys->cart.mapper.data7.prg_bank * 0x8000) + (addr & 0x7fff)];
    } else {
        return 0;
    }
}

static void _nes_write_prg7(uint16_t addr, uint8_t data, void* user_data) {
    nes_t* sys = (nes_t*)user_data;
    CHIPS_ASSERT(sys && sys->valid);
    if(addr >= 0x8000) {
        sys->cart.mapper.data7.prg_bank = (data & 0x07);
        sys->cart.mapper.mirroring = (data & 0x10) ? OneScreenHigher : OneScreenLower;
        _nes_mirroring(sys);
    }
}

static uint8_t _nes_read_chr7(uint16_t addr, void* user_data) {
    nes_t* sys = (nes_t*)user_data;
    CHIPS_ASSERT(sys && sys->valid);
    if(addr < 0x2000) {
        return sys->cart.character_ram[addr];
    }
    return 0;
}

static void _nes_write_chr7(uint16_t addr, uint8_t data, void* user_data) {
    nes_t* sys = (nes_t*)user_data;
    CHIPS_ASSERT(sys && sys->valid);
    if(addr < 0x2000) {
        sys->cart.character_ram[addr] = data;
    }
}

// ********* MAPPER 66 **************

static uint8_t _nes_read_prg66(uint16_t addr, void* user_data) {
    nes_t* sys = (nes_t*)user_data;
    CHIPS_ASSERT(sys && sys->valid);
    if(addr >= 0x8000) {
        return sys->cart.rom[(sys->cart.mapper.data66.prg_bank * 0x8000) + (addr & 0x7fff)];
    } else {
        return 0;
    }
}

static void _nes_write_prg66(uint16_t addr, uint8_t data, void* user_data) {
    nes_t* sys = (nes_t*)user_data;
    CHIPS_ASSERT(sys && sys->valid);
    if(addr >= 0x8000) {
        sys->cart.mapper.data66.prg_bank = ((data & 0x30) >> 4);
        sys->cart.mapper.data66.chr_bank = (data & 0x3);
        sys->cart.mapper.mirroring = Vertical;
        _nes_mirroring(sys);
    }
}

static uint8_t _nes_read_chr66(uint16_t addr, void* user_data) {
    nes_t* sys = (nes_t*)user_data;
    CHIPS_ASSERT(sys && sys->valid);
    if(addr < 0x2000) {
        return sys->cart.character_ram[(sys->cart.mapper.data66.chr_bank * 0x2000) + addr];
    }
    return 0;
}

#endif /* CHIPS_IMPL */
