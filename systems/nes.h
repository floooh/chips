#pragma once
/*#
    # nes.h

    A NES emulator in a C header

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
    - chips/m6502.h
    - chips/r2c02.h

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

// NES emulator state
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

typedef struct {
    uint8_t (*read_prg)(uint16_t addr, void* user_data);
    void* user_data;
} nes_mapper_t;

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

typedef struct {
    m6502_t cpu;
    r2c02_t ppu;
    bool request_nmi;
    
    chips_debug_t debug;

    mem_t mem;
    mem_t ppu_mem;
    uint8_t ram[0x800];             // 2KB
    uint8_t ppu_ram[0x1000];        // 4KB
    uint8_t character_ram[0x2000];  // 8KB
    uint16_t ppu_name_table[4];
    
    nes_mapper_t mapper;
    struct {
        nes_cartridge_header header;
        uint8_t rom[0x8000];
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
#define _OAMDMA     (0x4014)

static uint8_t _ppu_read(uint16_t addr, void* user_data);
static uint8_t _ppu_read_palette(uint8_t address, void* user_data);
static void _ppu_write(uint16_t address, uint8_t data, void* user_data);
static void _ppu_scanline_irq(void* user_data);
static void _ppu_vblank(void* user_data);
static void _ppu_set_pixel(int x, int y, uint8_t color, void* user_data);
static uint64_t _nes_tick(nes_t* sys, uint64_t pins);
static uint8_t mapper0_read_prg(uint16_t addr, void* user_data);

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
        .scanline_irq = _ppu_scanline_irq,
        .read_palette = _ppu_read_palette,
        .vblank_callback = _ppu_vblank,
        .set_pixel = _ppu_set_pixel,
        .user_data = sys,
    });

    mem_init(&sys->mem);
    // TODO: too bad, I can't use 4 pages of 0x800 size like this:
    //mem_map_ram(&sys->mem, 0, 0x0000, 0x0800, sys->ram);
    //mem_map_ram(&sys->mem, 0, 0x0800, 0x0800, sys->ram);
    //mem_map_ram(&sys->mem, 0, 0x1000, 0x0800, sys->ram);
    //mem_map_ram(&sys->mem, 0, 0x1800, 0x0800, sys->ram);
    mem_map_ram(&sys->mem, 0, 0x0000, 0x2000, sys->ram);
    mem_map_ram(&sys->mem, 1, 0x8000, 0x8000, sys->cart.rom);

    mem_init(&sys->ppu_mem);
    // TODO: it should be size: 0x800
    mem_map_ram(&sys->ppu_mem, 0, 0x2000, 0x1000, sys->ppu_ram);
    mem_map_ram(&sys->ppu_mem, 0, 0x3000, 0x1000, sys->ppu_ram);
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
        case 1:
            sys->controller[0].left = 1;
            break;
        case 2:
            sys->controller[0].right = 1;
            break;
        case 3:
            sys->controller[0].down = 1;
            break;
        case 4:
            sys->controller[0].up = 1;
            break;
        case 5:
            sys->controller[0].start = 1;
            break;
        case 6:
            sys->controller[0].a = 1;
            break;
        case 7:
            sys->controller[0].b = 1;
            break;
        case 8:
            sys->controller[0].select = 1;
            break;
    }
}

void nes_key_up(nes_t* sys, int value) {
    switch(value) {
        case 1:
            sys->controller[0].left = 0;
            break;
        case 2:
            sys->controller[0].right = 0;
            break;
        case 3:
            sys->controller[0].down = 0;
            break;
        case 4:
            sys->controller[0].up = 0;
            break;
        case 5:
            sys->controller[0].start = 0;
            break;
        case 6:
            sys->controller[0].a = 0;
            break;
        case 7:
            sys->controller[0].b = 0;
            break;
        case 8:
            sys->controller[0].select = 0;
            break;
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
    if(hdr->trainer || hdr->sram_avail || hdr->vram_expansion || hdr->mapper_low || hdr->prg_page_count > 2 || hdr->tile_page_count > 1)
        return false;

    size_t prg_size = hdr->prg_page_count * 0x4000;
    memcpy(&sys->cart.header, hdr, sizeof(nes_cartridge_header));
    memcpy(&sys->cart.rom, data.ptr + sizeof(nes_cartridge_header), prg_size);
    if(hdr->tile_page_count > 0) {
        memcpy(&sys->character_ram, data.ptr + sizeof(nes_cartridge_header) + prg_size, 0x2000);
    }

    const uint8_t mapper = hdr->mapper_low | (hdr->mapper_hi << 4);
    CHIPS_ASSERT(mapper == 0);
    
    if(hdr->mirror_mode) {
        // vertical mirroring
        sys->ppu_name_table[0] = sys->ppu_name_table[2] = 0x2000;
        sys->ppu_name_table[1] = sys->ppu_name_table[3] = 0x2400;
    } else {
        // horizontal mirroring
        sys->ppu_name_table[0] = sys->ppu_name_table[1] = 0x2000;
        sys->ppu_name_table[2] = sys->ppu_name_table[3] = 0x2400;
    }
    
    sys->mapper = (nes_mapper_t){
        .user_data = (void*)sys,
        .read_prg = mapper0_read_prg
    };

    mem_unmap_layer(&sys->mem, 0);
    if(hdr->prg_page_count == 1) {
        mem_map_ram(&sys->mem, 0, 0x8000, 0x4000, sys->cart.rom);
        mem_map_ram(&sys->mem, 0, 0xC000, 0x4000, sys->cart.rom);
    } else {
        mem_map_ram(&sys->mem, 0, 0x8000, 0x8000, sys->cart.rom);
    }

    nes_reset(sys);
    return true;
}

static uint8_t _ppu_read(uint16_t addr, void* user_data) {
    nes_t* sys = (nes_t*)user_data;
    CHIPS_ASSERT(sys && sys->valid);
    if (addr < 0x2000) {
        return sys->character_ram[addr];
    }
    if (addr < 0x3f00) {
        const uint16_t index = addr & 0x3ff;
        // Name tables upto 0x3000, then mirrored upto 3eff
        uint16_t normalizedAddr = addr;
        if (addr >= 0x3000)
            normalizedAddr -= 0x1000;

        if (normalizedAddr < 0x2400)      //NT0
            normalizedAddr = sys->ppu_name_table[0] + index;
        else if (normalizedAddr < 0x2800) //NT1
            normalizedAddr = sys->ppu_name_table[1] + index;
        else if (normalizedAddr < 0x2c00) //NT2
            normalizedAddr = sys->ppu_name_table[2] + index;
        else
            normalizedAddr = sys->ppu_name_table[3] + index;
        return mem_rd(&sys->ppu_mem, normalizedAddr);
    }
    if (addr < 0x4000) {
        uint16_t normalizedAddr = addr & 0x1f;
        // Addresses $3F10/$3F14/$3F18/$3F1C are mirrors of $3F00/$3F04/$3F08/$3F0C
        if (normalizedAddr >= 0x10 && addr % 4 == 0) {
            normalizedAddr = normalizedAddr & 0xf;
        }
        return mem_rd(&sys->ppu_mem, 0x3f00 + normalizedAddr);
    }
    return 0;
}

static void _ppu_write(uint16_t addr, uint8_t data, void* user_data) {
    nes_t* sys = (nes_t*)user_data;
    CHIPS_ASSERT(sys && sys->valid);
    if (addr < 0x2000) {
        sys->character_ram[addr] = data;
    }
    else if (addr < 0x3f00) {
        const uint16_t index = addr & 0x3ff;
        // Name tables upto 0x3000, then mirrored upto 3eff
        uint16_t normalizedAddr = addr;
        if (addr >= 0x3000)
            normalizedAddr -= 0x1000;
        if (normalizedAddr < 0x2400)      //NT0
            normalizedAddr = sys->ppu_name_table[0] + index;
        else if (normalizedAddr < 0x2800) //NT1
            normalizedAddr = sys->ppu_name_table[1] + index;
        else if (normalizedAddr < 0x2c00) //NT2
            normalizedAddr = sys->ppu_name_table[2] + index;
        else
            normalizedAddr = sys->ppu_name_table[3] + index;
        mem_wr(&sys->ppu_mem, normalizedAddr, data);
    }
    else if (addr < 0x4000) {
        uint16_t normalizedAddr = addr & 0x1f;
        // Addresses $3F10/$3F14/$3F18/$3F1C are mirrors of $3F00/$3F04/$3F08/$3F0C
        if (normalizedAddr >= 0x10 && addr % 4 == 0) {
            normalizedAddr = normalizedAddr & 0xf;
        }
        mem_wr(&sys->ppu_mem, 0x3f00 + normalizedAddr, data);
   }
}

static void _ppu_scanline_irq(void* user_data) {
    (void)user_data;
    // TODO: _ppu_scanline_irq
}

static uint8_t _ppu_read_palette(uint8_t addr, void* user_data) {
    nes_t* sys = (nes_t*)user_data;
    CHIPS_ASSERT(sys && sys->valid);
    uint16_t normalizedAddr = addr & 0x1f;
    // Addresses $3F10/$3F14/$3F18/$3F1C are mirrors of $3F00/$3F04/$3F08/$3F0C
    if (normalizedAddr >= 0x10 && addr % 4 == 0) {
        normalizedAddr = normalizedAddr & 0xf;
    }
    return mem_rd(&sys->ppu_mem, 0x3f00 + normalizedAddr);
}

static void _ppu_vblank(void* user_data) {
    nes_t* sys = (nes_t*)user_data;
    CHIPS_ASSERT(sys && sys->valid);
    sys->request_nmi = true;
}

static void _ppu_set_pixel(int x, int y, uint8_t color, void* user_data) {
    nes_t* sys = (nes_t*)user_data;
    CHIPS_ASSERT(sys && sys->valid);
    sys->fb[x+y*256] = color;
}

static uint64_t _nes_tick(nes_t* sys, uint64_t pins) {
    if(sys->request_nmi) {
        pins |= M6502_NMI;
        sys->request_nmi = false;
    }
    pins = m6502_tick(&sys->cpu, pins);
    
    //printf("PC: %04X\n", sys->cpu.PC);
    // extract 16-bit address from pin mask
    uint16_t addr = M6502_GET_ADDR(pins);
    // perform memory access
    if (pins & M6502_RW) {
        // a memory read
        if(addr < 0x2000) {
            M6502_SET_DATA(pins, sys->ram[addr & 0x7ff]);
        } else if (addr >= 0x4016 && addr <= 0x4017) {
            M6502_SET_DATA(pins, (sys->controller_state[addr & 0x0001] & 0x80) ? 1 : 0);
            sys->controller_state[addr & 0x0001] <<= 1;
        } else if (addr < 0x4020) {
            if (addr < 0x4000) //PPU registers, mirrored
                addr = addr & 0x2007;
            if(addr == _PPUSTATUS) {
                M6502_SET_DATA(pins, r2c02_status(&sys->ppu));
                pins &= ~M6502_NMI;
            } else if(addr == _PPUDATA) {
                M6502_SET_DATA(pins, r2c02_data(&sys->ppu));
            } else if(addr == _OAMDATA) {
                M6502_SET_DATA(pins, r2c02_oam_data(&sys->ppu, addr));
            }
        }
        else if (addr < 0x8000) {
            // TODO:
        }
        else if (sys->mapper.read_prg) {
            M6502_SET_DATA(pins, sys->mapper.read_prg((addr - 0x8000)& 0x3fff, sys->mapper.user_data));
        }
    }
    else {
        // a memory write
        uint8_t data = M6502_GET_DATA(pins);
        if(addr < 0x2000) {
            sys->ram[addr & 0x7ff] = data;
        }
        else if (addr >= 0x4016 && addr <= 0x4017) {
            sys->controller_state[addr & 0x0001] = sys->controller[addr & 0x0001].value;
        }
        else if (addr < 0x4020) {
            if (addr < 0x4000) //PPU registers, mirrored
                addr = addr & 0x2007;
            if(addr == _PPUCTRL) {
                r2c02_control(&sys->ppu, data);
            } else if(addr == _PPUMASK) {
                r2c02_set_mask(&sys->ppu, data);
            } else if(addr == _OAMADDR) {
                sys->ppu.sprite_data_address = data;
            } else if(addr == _PPUADDR) {
                r2c02_set_sata_address(&sys->ppu, data);
            } else if(addr == _PPUSCROL) {
                r2c02_set_scroll(&sys->ppu, data);
            } else if(addr == _PPUDATA) {
                r2c02_set_data(&sys->ppu, data);
            } else if(addr == _OAMDMA) {
                // TODO:
                uint8_t* page_ptr = &sys->ram[addr & 0x7ff];
                if (page_ptr)
                {
                    memcpy(sys->ppu.sprite_memory + sys->ppu.sprite_data_address, page_ptr, 256 - sys->ppu.sprite_data_address);
                    if (sys->ppu.sprite_data_address)
                        memcpy(sys->ppu.sprite_memory, page_ptr + (256 - sys->ppu.sprite_data_address), sys->ppu.sprite_data_address);
                }
            } else if(addr == _OAMDATA) {
                sys->ppu.sprite_memory[sys->ppu.sprite_data_address++] = data;
            }
        }
    }

    r2c02_tick(&sys->ppu, pins);
    r2c02_tick(&sys->ppu, pins);
    r2c02_tick(&sys->ppu, pins);

    return pins;
}

static uint8_t mapper0_read_prg(uint16_t addr, void* user_data) {
    nes_t* sys = (nes_t*)user_data;
    CHIPS_ASSERT(sys && sys->valid);
    return sys->cart.rom[addr];
}

#endif /* CHIPS_IMPL */
