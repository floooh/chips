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
        - PAL not implemented
        - only mappers 0, 2, 3, 7 & 66 are implemented
        - there is no audio
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

// NES emulator state
typedef struct {
    m6502_t cpu;
    r2c02_t ppu;
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

uint8_t nes_ppu_read(nes_t* nes, uint16_t addr);
void nes_ppu_write(nes_t* nes, uint16_t address, uint8_t data);
uint8_t nes_mem_read(nes_t* sys, uint16_t addr);
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

void nes_init(nes_t* sys, const nes_desc_t* desc) {
    CHIPS_ASSERT(sys && desc);
    if (desc->debug.callback.func) { CHIPS_ASSERT(desc->debug.stopped); }
    memset(sys, 0, sizeof(nes_t));
    sys->valid = true;
    sys->debug = desc->debug;
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

void nes_discard(nes_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    sys->valid = false;
}

void nes_reset(nes_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    sys->pins |= M6502_RES;
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

uint8_t nes_mem_read(nes_t* sys, uint16_t addr) {
    if(addr < 0x2000) {
        return sys->ram[addr & 0x7ff];
    } else if (addr >= 0x4016 && addr <= 0x4017) {
        uint8_t data = (sys->controller_state[addr & 0x0001] & 0x80) ? 1 : 0;
        sys->controller_state[addr & 0x0001] <<= 1;
        return data;
    } else if (addr < 0x4020) {
        if (addr < 0x4000) //PPU registers, mirrored
            addr = addr & 0x2007;
        return r2c02_read(&sys->ppu, addr-0x2000);
    } else if (addr < 0x6000) {
        // TODO: Expansion ROM
        return 0xFF;
    } else if (addr < 0x8000) {
        return sys->extended_ram[addr - 0x6000];
    } else {
        return  sys->cart.mapper.read_prg(addr, sys);
    }
}

void nes_mem_write(nes_t* sys, uint16_t addr, uint8_t data) {
    if(addr < 0x2000) {
        sys->ram[addr & 0x7ff] = data;
    } else if (addr < 0x4000) {
        //PPU registers, mirrored
        addr = addr & 0x0007;
        r2c02_write(&sys->ppu, addr, data);
    } else if(addr == 0x4014) {
        // OAMDMA
        uint16_t page = data << 8;
        if (page < 0x2000) {
            uint8_t* page_ptr = sys->ram + (page & 0x7ff);
            memcpy(sys->ppu.sprite_memory + sys->ppu.sprite_data_address, page_ptr, 256 - sys->ppu.sprite_data_address);
            if (sys->ppu.sprite_data_address)
                memcpy(sys->ppu.sprite_memory, page_ptr + (256 - sys->ppu.sprite_data_address), sys->ppu.sprite_data_address);
        }
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

static uint64_t _nes_tick(nes_t* sys, uint64_t pins) {
    if(sys->ppu.request_nmi) {
        pins |= M6502_NMI;
        sys->ppu.request_nmi = false;
    }
    pins = m6502_tick(&sys->cpu, pins);
    pins &= ~M6502_NMI;

    // extract 16-bit address from pin mask
    uint16_t addr = M6502_GET_ADDR(pins);
    // perform memory access
    if (pins & M6502_RW) {
        // a memory read
        M6502_SET_DATA(pins, nes_mem_read(sys, addr));
    }
    else {
        // a memory write
        nes_mem_write(sys, addr, M6502_GET_DATA(pins));
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
    bool supported = false;
    memset(&sys->cart.mapper, 0, sizeof(sys->cart.mapper));
    sys->cart.mapper = (nes_mapper_t){
        .read_prg = _nes_read_prg0,
        .write_prg = _nes_write_prg0,
        .read_chr = _nes_read_chr0,
        .write_chr = _nes_write_chr0,
    };
    switch(mapper_num) {
        case 0:
            supported = true;
            break;
        case 2:
            sys->cart.mapper = (nes_mapper_t) {
                .read_prg = _nes_read_prg2,
                .write_prg = _nes_write_prg2,
                .read_chr = _nes_read_chr0,
                .write_chr = _nes_write_chr0,
            };
            supported = true;
            break;
        case 3:
            sys->cart.mapper = (nes_mapper_t) {
                .read_prg = _nes_read_prg3,
                .write_prg = _nes_write_prg3,
                .read_chr = _nes_read_chr3,
                .write_chr = _nes_write_chr0,
            };
            supported = true;
            break;
        case 7:
            sys->cart.mapper = (nes_mapper_t) {
                .mirroring = OneScreenLower,
                .read_prg = _nes_read_prg7,
                .write_prg = _nes_write_prg7,
                .read_chr = _nes_read_chr7,
                .write_chr = _nes_write_chr7,
            };
            supported = true;
            break;
        case 66:
            sys->cart.mapper = (nes_mapper_t) {
                .read_prg = _nes_read_prg66,
                .write_prg = _nes_write_prg66,
                .read_chr = _nes_read_chr66,
                .write_chr = _nes_write_chr0,
            };
            supported = true;
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
