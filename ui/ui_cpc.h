#pragma once
/*#
    # ui_cpc.h

    Integrated debugging UI for cpc.h

    Do this:
    ~~~C
    #define CHIPS_IMPL
    ~~~
    before you include this file in *one* C++ file to create the 
    implementation.

    Optionally provide the following macros with your own implementation
    
    ~~~C
    CHIPS_ASSERT(c)
    ~~~
        your own assert macro (default: assert(c))

    Include the following headers (and their depenencies) before including
    ui_cpc.h both for the declaration and implementation.

    - cpc.h
    - mem.h
    - ui_chip.h
    - ui_util.h
    - ui_z80.h
    - ui_i8255.h
    - ui_mc6845.h
    - ui_upd765.h
    - ui_ay38910.h
    - ui_upd765.h
    - ui_am40010.h
    - ui_fdd.h
    - ui_audio.h
    - ui_dasm.h
    - ui_dbg.h
    - ui_memedit.h
    - ui_memmap.h
    - ui_kbd.h

    ## zlib/libpng license

    Copyright (c) 2018 Andre Weissflog
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

/* general callback type for rebooting to different configs */
typedef void (*ui_cpc_boot_t)(cpc_t* sys, cpc_type_t type);

typedef struct {
    cpc_t* cpc;
    ui_cpc_boot_t boot_cb; /* user-provided callback to reboot to different config */
    ui_dbg_create_texture_t create_texture_cb;      /* texture creation callback for ui_dbg_t */
    ui_dbg_update_texture_t update_texture_cb;      /* texture update callback for ui_dbg_t */
    ui_dbg_destroy_texture_t destroy_texture_cb;    /* texture destruction callback for ui_dbg_t */
    ui_dbg_keydesc_t dbg_keys;          /* user-defined hotkeys for ui_dbg_t */
} ui_cpc_desc_t;

typedef struct {
    cpc_t* cpc;
    int dbg_scanline;
    bool dbg_vsync;
    ui_cpc_boot_t boot_cb;
    ui_z80_t cpu;
    ui_ay38910_t psg;
    ui_mc6845_t vdc;
    ui_am40010_t ga;
    ui_i8255_t ppi;
    ui_upd765_t upd;
    ui_audio_t audio;
    ui_fdd_t fdd;
    ui_kbd_t kbd;
    ui_memmap_t memmap;
    ui_memedit_t memedit[4];
    ui_dasm_t dasm[4];
    ui_dbg_t dbg;
} ui_cpc_t;

void ui_cpc_init(ui_cpc_t* ui, const ui_cpc_desc_t* desc);
void ui_cpc_discard(ui_cpc_t* ui);
void ui_cpc_draw(ui_cpc_t* ui, double time_ms);
bool ui_cpc_before_exec(ui_cpc_t* ui);
void ui_cpc_after_exec(ui_cpc_t* ui);

#ifdef __cplusplus
} /* extern "C" */
#endif

/*-- IMPLEMENTATION (include in C++ source) ----------------------------------*/
#ifdef CHIPS_IMPL
#ifndef __cplusplus
#error "implementation must be compiled as C++"
#endif
#include <string.h> /* memset */
#ifndef CHIPS_ASSERT
    #include <assert.h>
    #define CHIPS_ASSERT(c) assert(c)
#endif
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-field-initializers"
#endif

static void _ui_cpc_draw_menu(ui_cpc_t* ui, double time_ms) {
    CHIPS_ASSERT(ui && ui->cpc && ui->boot_cb);
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("System")) {
            if (ImGui::MenuItem("Reset")) {
                cpc_reset(ui->cpc);
                ui_dbg_reset(&ui->dbg);
            }
            if (ImGui::MenuItem("CPC 464", 0, ui->cpc->type == CPC_TYPE_464)) {
                ui->boot_cb(ui->cpc, CPC_TYPE_464);
                ui_dbg_reboot(&ui->dbg);
            }
            if (ImGui::MenuItem("CPC 6128", 0, ui->cpc->type == CPC_TYPE_6128)) {
                ui->boot_cb(ui->cpc, CPC_TYPE_6128);
                ui_dbg_reboot(&ui->dbg);
            }
            if (ImGui::MenuItem("KC Compact", 0, ui->cpc->type == CPC_TYPE_KCCOMPACT)) {
                ui->boot_cb(ui->cpc, CPC_TYPE_KCCOMPACT);
                ui_dbg_reboot(&ui->dbg);
            }
            if (ImGui::MenuItem("Joystick", 0, ui->cpc->joystick_type != CPC_JOYSTICK_NONE)) {
                if (ui->cpc->joystick_type == CPC_JOYSTICK_NONE) {
                    ui->cpc->joystick_type = CPC_JOYSTICK_DIGITAL;
                }
                else {
                    ui->cpc->joystick_type = CPC_JOYSTICK_NONE;
                }
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Hardware")) {
            ImGui::MenuItem("Memory Map", 0, &ui->memmap.open);
            ImGui::MenuItem("Keyboard Matrix", 0, &ui->kbd.open);
            ImGui::MenuItem("Audio Output", 0, &ui->audio.open);
            ImGui::MenuItem("Z80 (CPU)", 0, &ui->cpu.open);
            ImGui::MenuItem("AY-3-8912 (PSG)", 0, &ui->psg.open);
            ImGui::MenuItem("MC6845 (CRTC)", 0, &ui->vdc.open);
            ImGui::MenuItem("AM40010 (Gate Array)", 0, &ui->ga.open);
            ImGui::MenuItem("i8255 (PPI)", 0, &ui->ppi.open);
            ImGui::MenuItem("uPD765 (FDC)", 0, &ui->upd.open);
            ImGui::MenuItem("Floppy Drive", 0, &ui->fdd.open);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Debug")) {
            ImGui::MenuItem("CPU Debugger", 0, &ui->dbg.ui.open);
            ImGui::MenuItem("Breakpoints", 0, &ui->dbg.ui.show_breakpoints);
            ImGui::MenuItem("Execution History", 0, &ui->dbg.ui.show_history);
            ImGui::MenuItem("Memory Heatmap", 0, &ui->dbg.ui.show_heatmap);
            if (ImGui::BeginMenu("Memory Editor")) {
                ImGui::MenuItem("Window #1", 0, &ui->memedit[0].open);
                ImGui::MenuItem("Window #2", 0, &ui->memedit[1].open);
                ImGui::MenuItem("Window #3", 0, &ui->memedit[2].open);
                ImGui::MenuItem("Window #4", 0, &ui->memedit[3].open);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Disassembler")) {
                ImGui::MenuItem("Window #1", 0, &ui->dasm[0].open);
                ImGui::MenuItem("Window #2", 0, &ui->dasm[1].open);
                ImGui::MenuItem("Window #3", 0, &ui->dasm[2].open);
                ImGui::MenuItem("Window #4", 0, &ui->dasm[3].open);
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        ui_util_options_menu(time_ms, ui->dbg.dbg.stopped);
        ImGui::EndMainMenuBar();
    }
}

#define _UI_CPC_MEMLAYER_CPU      (0)
#define _UI_CPC_MEMLAYER_GA       (1)
#define _UI_CPC_MEMLAYER_ROMS     (2)
#define _UI_CPC_MEMLAYER_AMSDOS   (3)
#define _UI_CPC_MEMLAYER_RAM0     (4)
#define _UI_CPC_MEMLAYER_RAM1     (5)
#define _UI_CPC_MEMLAYER_RAM2     (6)
#define _UI_CPC_MEMLAYER_RAM3     (7)
#define _UI_CPC_MEMLAYER_RAM4     (8)
#define _UI_CPC_MEMLAYER_RAM5     (9)
#define _UI_CPC_MEMLAYER_RAM6     (10)
#define _UI_CPC_MEMLAYER_RAM7     (11)
#define _UI_CPC_MEMLAYER_NUM      (12)

static int _ui_cpc_ram_config[8][4] = {
    { 0, 1, 2, 3 },
    { 0, 1, 2, 7 },
    { 4, 5, 6, 7 },
    { 0, 3, 2, 7 },
    { 0, 4, 2, 3 },
    { 0, 5, 2, 3 },
    { 0, 6, 2, 3 },
    { 0, 7, 2, 3 }
};

static const char* _ui_cpc_ram_name[8] = {
    "RAM 0", "RAM 1", "RAM 2", "RAM 3", "RAM 4", "RAM 5", "RAM 6", "RAM 7"
};
static const char* _ui_cpc_ram_banks[8] = {
    "RAM Bank 0", "RAM Bank 1", "RAM Bank 2", "RAM Bank 3", "RAM Bank 4", "RAM Bank 5", "RAM Bank 6", "RAM Bank 7"
};

static const char* _ui_cpc_memlayer_names[_UI_CPC_MEMLAYER_NUM] = {
    "CPU Mapped", "Gate Array", "OS ROMS", "AMSDOS ROM", "RAM 0", "RAM 1", "RAM 2", "RAM 3", "RAM 4", "RAM 5", "RAM 6", "RAM 7"
};

static void _ui_cpc_update_memmap(ui_cpc_t* ui) {
    CHIPS_ASSERT(ui && ui->cpc);
    const cpc_t* cpc = ui->cpc;
    ui_memmap_reset(&ui->memmap);
    const uint8_t rom_enable = cpc->ga.regs.config;
    if ((cpc->type == CPC_TYPE_464) || (cpc->type == CPC_TYPE_KCCOMPACT)) {
        ui_memmap_layer(&ui->memmap, "ROM");
            ui_memmap_region(&ui->memmap, "Lower ROM (OS)", 0x0000, 0x4000, !(rom_enable & AM40010_CONFIG_LROMEN));
            ui_memmap_region(&ui->memmap, "Upper ROM (BASIC)", 0xC000, 0x4000, !(rom_enable & AM40010_CONFIG_HROMEN));
        ui_memmap_layer(&ui->memmap, "RAM");
            ui_memmap_region(&ui->memmap, "RAM 0", 0x0000, 0x4000, true);
            ui_memmap_region(&ui->memmap, "RAM 1", 0x4000, 0x4000, true);
            ui_memmap_region(&ui->memmap, "RAM 2", 0x8000, 0x4000, true);
            ui_memmap_region(&ui->memmap, "RAM 3 (Screen)", 0xC000, 0x4000, true);
    }
    else {
        const uint8_t ram_config_index = cpc->ga.ram_config & 7;
        const uint8_t rom_select = cpc->ga.rom_select;
        ui_memmap_layer(&ui->memmap, "ROM Layer 0");
            ui_memmap_region(&ui->memmap, "OS ROM", 0x0000, 0x4000, !(rom_enable & AM40010_CONFIG_LROMEN));
            ui_memmap_region(&ui->memmap, "BASIC ROM", 0xC000, 0x4000, !(rom_enable & AM40010_CONFIG_HROMEN) && (rom_select != 7));
        ui_memmap_layer(&ui->memmap, "ROM Layer 1");
            ui_memmap_region(&ui->memmap, "AMSDOS ROM", 0xC000, 0x4000, !(rom_enable & AM40010_CONFIG_HROMEN)  && (rom_select == 7));
        for (int bank = 0; bank < 8; bank++) {
            ui_memmap_layer(&ui->memmap, _ui_cpc_ram_banks[bank]);
            bool bank_active = false;
            for (int slot = 0; slot < 4; slot++) {
                if (bank == _ui_cpc_ram_config[ram_config_index][slot]) {
                    ui_memmap_region(&ui->memmap, _ui_cpc_ram_name[bank], 0x4000*slot, 0x4000, true);
                    bank_active = true;
                }
            }
            if (!bank_active) {
                ui_memmap_region(&ui->memmap, _ui_cpc_ram_name[bank], 0x0000, 0x4000, false);
            }
        }
    }
}

static uint8_t* _ui_cpc_memptr(cpc_t* cpc, int layer, uint16_t addr) {
    CHIPS_ASSERT((layer >= _UI_CPC_MEMLAYER_GA) && (layer < _UI_CPC_MEMLAYER_NUM));
    if (layer == _UI_CPC_MEMLAYER_GA) {
        uint8_t* ram = &cpc->ram[0][0];
        return ram + addr;
    }
    else if (layer == _UI_CPC_MEMLAYER_ROMS) {
        if (addr < 0x4000) {
            return &cpc->rom_os[addr];
        }
        else if (addr >= 0xC000) {
            return &cpc->rom_basic[addr - 0xC000];
        }
        else {
            return 0;
        }
    }
    else if (layer == _UI_CPC_MEMLAYER_AMSDOS) {
        if ((CPC_TYPE_6128 == cpc->type) && (addr >= 0xC000)) {
            return &cpc->rom_amsdos[addr - 0xC000];
        }
        else {
            return 0;
        }
    }
    else {
        /* one of the 7 RAM layers */
        CHIPS_ASSERT((layer >= _UI_CPC_MEMLAYER_RAM0) && (layer <= _UI_CPC_MEMLAYER_RAM7));
        const int ram_config_index = (CPC_TYPE_6128 == cpc->type) ? (cpc->ga.ram_config & 7) : 0;
        const int ram_bank = layer - _UI_CPC_MEMLAYER_RAM0;
        bool ram_mapped = false;
        for (int i = 0; i < 4; i++) {
            if (ram_bank == _ui_cpc_ram_config[ram_config_index][i]) {
                const uint16_t start = 0x4000 * i;
                const uint32_t end = start + 0x4000;
                ram_mapped = true;
                if ((addr >= start) && (addr < end)) {
                    return &cpc->ram[ram_bank][addr - start];
                }
            }
        }
        if (!ram_mapped && (CPC_TYPE_6128 != cpc->type)) {
            /* if the RAM bank is not currently mapped to a CPU visible address,
                just use start address zero, this will throw off disassemblers
                though
            */
            if (addr < 0x4000) {
                return &cpc->ram[ram_bank][addr];
            }
        }
    }
    /* fallthrough: address isn't mapped to physical RAM */
    return 0;
}

static uint8_t _ui_cpc_mem_read(int layer, uint16_t addr, void* user_data) {
    CHIPS_ASSERT(user_data);
    ui_cpc_t* ui_cpc = (ui_cpc_t*) user_data;
    cpc_t* cpc = ui_cpc->cpc;
    if (layer == _UI_CPC_MEMLAYER_CPU) {
        /* CPU mapped RAM layer */
        return mem_rd(&cpc->mem, addr);
    }
    else {
        uint8_t* ptr = _ui_cpc_memptr(cpc, layer, addr);
        if (ptr) {
            return *ptr;
        }
        else {
            return 0xFF;
        }
    }
}

static void _ui_cpc_mem_write(int layer, uint16_t addr, uint8_t data, void* user_data) {
    CHIPS_ASSERT(user_data);
    ui_cpc_t* ui_cpc = (ui_cpc_t*) user_data;
    cpc_t* cpc = ui_cpc->cpc;
    if (layer == _UI_CPC_MEMLAYER_CPU) {
        mem_wr(&cpc->mem, addr, data);
    }
    else {
        uint8_t* ptr = _ui_cpc_memptr(cpc, layer, addr);
        if (ptr) {
            *ptr = data;
        }
    }
}

static int _ui_cpc_eval_bp(ui_dbg_t* dbg_win, uint16_t pc, int ticks, uint64_t pins, void* user_data) {
    (void)pc;
    (void)ticks;
    (void)pins;
    CHIPS_ASSERT(user_data);
    ui_cpc_t* ui_cpc = (ui_cpc_t*) user_data;
    cpc_t* cpc = ui_cpc->cpc;
    int scanline = cpc->ga.crt.v_pos;
    bool vsync = cpc->crtc.vs;
    int trap_id = 0;
    for (int i = 0; (i < dbg_win->dbg.num_breakpoints) && (trap_id == 0); i++) {
        const ui_dbg_breakpoint_t* bp = &dbg_win->dbg.breakpoints[i];
        if (bp->enabled) {
            switch (bp->type) {
                /* scanline number */
                case UI_DBG_BREAKTYPE_USER+0:
                    if ((ui_cpc->dbg_scanline != scanline) && (scanline == bp->val)) {
                        trap_id = UI_DBG_BP_BASE_TRAPID + i;
                    }
                    break;
                /* new scanline */
                case UI_DBG_BREAKTYPE_USER+1:
                    if (ui_cpc->dbg_scanline != scanline) {
                        trap_id = UI_DBG_BP_BASE_TRAPID + i;
                    }
                    break;
                /* new frame */
                case UI_DBG_BREAKTYPE_USER+2:
                    if ((ui_cpc->dbg_scanline != scanline) && (scanline == 0)) {
                        trap_id = UI_DBG_BP_BASE_TRAPID + i;
                    }
                    break;
                /* VSYNC */
                case UI_DBG_BREAKTYPE_USER+3:
                    if (vsync && !ui_cpc->dbg_vsync) {
                        trap_id = UI_DBG_BP_BASE_TRAPID + i;
                    }
                    break;
            }
        }
    }
    ui_cpc->dbg_scanline = scanline;
    ui_cpc->dbg_vsync = vsync;
    return trap_id;
}

static const ui_chip_pin_t _ui_cpc_cpu_pins[] = {
    { "D0",     0,      Z80_D0 },
    { "D1",     1,      Z80_D1 },
    { "D2",     2,      Z80_D2 },
    { "D3",     3,      Z80_D3 },
    { "D4",     4,      Z80_D4 },
    { "D5",     5,      Z80_D5 },
    { "D6",     6,      Z80_D6 },
    { "D7",     7,      Z80_D7 },
    { "M1",     9,      Z80_M1 },
    { "MREQ",   10,     Z80_MREQ },
    { "IORQ",   11,     Z80_IORQ },
    { "RD",     12,     Z80_RD },
    { "WR",     13,     Z80_WR },
    { "HALT",   14,     Z80_HALT },
    { "INT",    15,     Z80_INT },
    { "NMI",    16,     Z80_NMI },
    { "WAIT",   17,     Z80_WAIT_MASK },
    { "A0",     18,     Z80_A0 },
    { "A1",     19,     Z80_A1 },
    { "A2",     20,     Z80_A2 },
    { "A3",     21,     Z80_A3 },
    { "A4",     22,     Z80_A4 },
    { "A5",     23,     Z80_A5 },
    { "A6",     24,     Z80_A6 },
    { "A7",     25,     Z80_A7 },
    { "A8",     26,     Z80_A8 },
    { "A9",     27,     Z80_A9 },
    { "A10",    28,     Z80_A10 },
    { "A11",    29,     Z80_A11 },
    { "A12",    30,     Z80_A12 },
    { "A13",    31,     Z80_A13 },
    { "A14",    32,     Z80_A14 },
    { "A15",    33,     Z80_A15 },
};

static const ui_chip_pin_t _ui_cpc_psg_pins[] = {
    { "DA0",  0, AY38910_DA0 },
    { "DA1",  1, AY38910_DA1 },
    { "DA2",  2, AY38910_DA2 },
    { "DA3",  3, AY38910_DA3 },
    { "DA4",  4, AY38910_DA4 },
    { "DA5",  5, AY38910_DA5 },
    { "DA6",  6, AY38910_DA6 },
    { "DA7",  7, AY38910_DA7 },
    { "BDIR", 9, AY38910_BDIR },
    { "BC1",  10, AY38910_BC1 },
    { "IOA0", 11, AY38910_IOA0 },
    { "IOA1", 12, AY38910_IOA1 },
    { "IOA2", 13, AY38910_IOA2 },
    { "IOA3", 14, AY38910_IOA3 },
    { "IOA4", 15, AY38910_IOA4 },
    { "IOA5", 16, AY38910_IOA5 },
    { "IOA6", 17, AY38910_IOA6 },
    { "IOA7", 18, AY38910_IOA7 },
};

static const ui_chip_pin_t _ui_cpc_vdc_pins[] = {
    { "D0",     0,      MC6845_D0 },
    { "D1",     1,      MC6845_D1 },
    { "D2",     2,      MC6845_D2 },
    { "D3",     3,      MC6845_D3 },
    { "D4",     4,      MC6845_D4 },
    { "D5",     5,      MC6845_D5 },
    { "D6",     6,      MC6845_D6 },
    { "D7",     7,      MC6845_D7 },
    { "CS",     9,      MC6845_CS },
    { "RS",    10,      MC6845_RS },
    { "RW",    11,      MC6845_RW },
    { "DE",    13,      MC6845_DE },
    { "VS",    14,      MC6845_VS },
    { "HS",    15,      MC6845_HS },
    { "MA0",   20,      MC6845_MA0 },
    { "MA1",   21,      MC6845_MA1 },
    { "MA2",   22,      MC6845_MA2 },
    { "MA3",   23,      MC6845_MA3 },
    { "MA4",   24,      MC6845_MA4 },
    { "MA5",   25,      MC6845_MA5 },
    { "MA6",   26,      MC6845_MA6 },
    { "MA7",   27,      MC6845_MA7 },
    { "MA8",   28,      MC6845_MA8 },
    { "MA9",   29,      MC6845_MA9 },
    { "MA10",  30,      MC6845_MA10 },
    { "MA11",  31,      MC6845_MA11 },
    { "MA12",  32,      MC6845_MA12 },
    { "MA13",  33,      MC6845_MA13 },
    { "RA0",   35,      MC6845_RA0 },
    { "RA1",   36,      MC6845_RA1 },
    { "RA2",   37,      MC6845_RA2 },
    { "RA3",   38,      MC6845_RA3 },
    { "RA4",   39,      MC6845_RA4 },
};

static const ui_chip_pin_t _ui_cpc_ppi_pins[] = {
    { "D0",     0,      I8255_D0 },
    { "D1",     1,      I8255_D1 },
    { "D2",     2,      I8255_D2 },
    { "D3",     3,      I8255_D3 },
    { "D4",     4,      I8255_D4 },
    { "D5",     5,      I8255_D5 },
    { "D6",     6,      I8255_D6 },
    { "D7",     7,      I8255_D7 },

    { "CS",     9,      I8255_CS },
    { "RD",    10,      I8255_RD },
    { "WR",    11,      I8255_WR },
    { "A0",    12,      I8255_A0 },
    { "A1",    13,      I8255_A1 },

    { "PC0",   16,      I8255_PC0 },
    { "PC1",   17,      I8255_PC1 },
    { "PC2",   18,      I8255_PC2 },
    { "PC3",   19,      I8255_PC3 },

    { "PA0",   20,      I8255_PA0 },
    { "PA1",   21,      I8255_PA1 },
    { "PA2",   22,      I8255_PA2 },
    { "PA3",   23,      I8255_PA3 },
    { "PA4",   24,      I8255_PA4 },
    { "PA5",   25,      I8255_PA5 },
    { "PA6",   26,      I8255_PA6 },
    { "PA7",   27,      I8255_PA7 },

    { "PB0",   28,      I8255_PB0 },
    { "PB1",   29,      I8255_PB1 },
    { "PB2",   30,      I8255_PB2 },
    { "PB3",   31,      I8255_PB3 },
    { "PB4",   32,      I8255_PB4 },
    { "PB5",   33,      I8255_PB5 },
    { "PB6",   34,      I8255_PB6 },
    { "PB7",   35,      I8255_PB7 },

    { "PC4",   36,      I8255_PC4 },
    { "PC5",   37,      I8255_PC5 },
    { "PC6",   38,      I8255_PC6 },
    { "PC7",   39,      I8255_PC7 },
};

static const ui_chip_pin_t _ui_cpc_upd_pins[] = {
    { "A0",     0,      I8255_A0 },
    { "CS",     2,      I8255_CS },
    { "RD",     3,      I8255_RD },
    { "WR",     4,      I8255_WR },

    { "D0",     8,      I8255_D0 },
    { "D1",     9,      I8255_D1 },
    { "D2",     10,     I8255_D2 },
    { "D3",     11,     I8255_D3 },
    { "D4",     12,     I8255_D4 },
    { "D5",     13,     I8255_D5 },
    { "D6",     14,     I8255_D6 },
    { "D7",     15,     I8255_D7 },
};

static const ui_chip_pin_t _ui_cpc_ga_pins[] = {
    { "D0",     0,      AM40010_D0 },
    { "D1",     1,      AM40010_D1 },
    { "D2",     2,      AM40010_D2 },
    { "D3",     3,      AM40010_D3 },
    { "D4",     4,      AM40010_D4 },
    { "D5",     5,      AM40010_D5 },
    { "D6",     6,      AM40010_D6 },
    { "D7",     7,      AM40010_D7 },

    { "M1",     9,      AM40010_M1 },
    { "MREQ",   10,     AM40010_MREQ },
    { "IORQ",   11,     AM40010_IORQ },
    { "RD",     12,     AM40010_RD },
    { "WR",     13,     AM40010_WR },

    { "A13",    14,     AM40010_A13 },
    { "A14",    15,     AM40010_A14 },
    { "A15",    16,     AM40010_A15 },

    { "DE",     18,     AM40010_VS },
    { "HS",     19,     AM40010_HS },
    { "VS",     20,     AM40010_VS },

    { "INT",    22,     AM40010_INT },
    { "READY",  23,     AM40010_READY },
    { "SYNC",   24,     AM40010_SYNC }
};

void ui_cpc_init(ui_cpc_t* ui, const ui_cpc_desc_t* ui_desc) {
    CHIPS_ASSERT(ui && ui_desc);
    CHIPS_ASSERT(ui_desc->cpc);
    CHIPS_ASSERT(ui_desc->boot_cb);
    ui->cpc = ui_desc->cpc;
    ui->boot_cb = ui_desc->boot_cb;
    int x = 20, y = 20, dx = 10, dy = 10;
    {
        ui_dbg_desc_t desc = {0};
        desc.title = "CPU Debugger";
        desc.x = x;
        desc.y = y;
        desc.z80 = &ui->cpc->cpu;
        desc.read_cb = _ui_cpc_mem_read;
        desc.break_cb = _ui_cpc_eval_bp;
        desc.create_texture_cb = ui_desc->create_texture_cb;
        desc.update_texture_cb = ui_desc->update_texture_cb;
        desc.destroy_texture_cb = ui_desc->destroy_texture_cb;
        desc.keys = ui_desc->dbg_keys;
        desc.user_data = ui;
        /* custom breakpoint types */
        desc.user_breaktypes[0].label = "Scanline at";
        desc.user_breaktypes[0].show_val16 = true;
        desc.user_breaktypes[1].label = "New Scanline";
        desc.user_breaktypes[2].label = "New Frame";
        desc.user_breaktypes[3].label = "VSYNC";
        ui_dbg_init(&ui->dbg, &desc);
    }
    x += dx; y += dy;
    {
        ui_z80_desc_t desc = {0};
        desc.title = "Z80 CPU";
        desc.cpu = &ui->cpc->cpu;
        desc.x = x;
        desc.y = y;
        UI_CHIP_INIT_DESC(&desc.chip_desc, "Z80\nCPU", 36, _ui_cpc_cpu_pins);
        ui_z80_init(&ui->cpu, &desc);
    }
    x += dx; y += dy;
    {
        ui_ay38910_desc_t desc = {0};
        desc.title = "AY-3-8912";
        desc.ay = &ui->cpc->psg;
        desc.x = x;
        desc.y = y;
        UI_CHIP_INIT_DESC(&desc.chip_desc, "8912", 22, _ui_cpc_psg_pins);
        ui_ay38910_init(&ui->psg, &desc);
    }
    x += dx; y += dy;
    {
        ui_mc6845_desc_t desc = {0};
        desc.title = "MC6845";
        desc.mc6845 = &ui->cpc->crtc;
        desc.x = x;
        desc.y = y;
        UI_CHIP_INIT_DESC(&desc.chip_desc, "6845", 40, _ui_cpc_vdc_pins);
        ui_mc6845_init(&ui->vdc, &desc);
    }
    x += dx, y += dy;
    {
        ui_am40010_desc_t desc = {0};
        desc.title = "AM40010 + PAL";
        desc.am40010 = &ui->cpc->ga;
        desc.x = x;
        desc.y = y;
        UI_CHIP_INIT_DESC(&desc.chip_desc, "40010", 28, _ui_cpc_ga_pins);
        ui_am40010_init(&ui->ga, &desc);
    }
    x += dx; y += dy;
    {
        ui_i8255_desc_t desc = {0};
        desc.title = "i8255";
        desc.i8255 = &ui->cpc->ppi;
        desc.x = x;
        desc.y = y;
        UI_CHIP_INIT_DESC(&desc.chip_desc, "i8255", 40, _ui_cpc_ppi_pins);
        ui_i8255_init(&ui->ppi, &desc);
    }
    x += dx; y += dy;
    {
        ui_upd765_desc_t desc = {0};
        desc.title = "uPD765";
        desc.upd765 = &ui->cpc->fdc;
        desc.x = x;
        desc.y = y;
        UI_CHIP_INIT_DESC(&desc.chip_desc, "765", 16, _ui_cpc_upd_pins);
        ui_upd765_init(&ui->upd, &desc);
    }
    x += dx; y += dy;
    {
        ui_audio_desc_t desc = {0};
        desc.title = "Audio Output";
        desc.sample_buffer = ui->cpc->sample_buffer;
        desc.num_samples = ui->cpc->num_samples;
        desc.x = x;
        desc.y = y;
        ui_audio_init(&ui->audio, &desc);
    }
    x += dx; y += dy;
    {
        ui_fdd_desc_t desc = {0};
        desc.title = "Floppy Disk Drive";
        desc.fdd = &ui->cpc->fdd;
        desc.x = x;
        desc.y = y;
        ui_fdd_init(&ui->fdd, &desc);
    }
    x += dx; y += dy;
    {
        ui_kbd_desc_t desc = {0};
        desc.title = "Keyboard Matrix";
        desc.kbd = &ui->cpc->kbd;
        desc.layers[0] = "None";
        desc.layers[1] = "Shift";
        desc.layers[2] = "Ctrl";
        desc.x = x;
        desc.y = y;
        ui_kbd_init(&ui->kbd, &desc);
    }
    x += dx; y += dy;
    {
        ui_memedit_desc_t desc = {0};
        for (int i = 0; i < _UI_CPC_MEMLAYER_NUM; i++) {
            desc.layers[i] = _ui_cpc_memlayer_names[i];
        }
        desc.read_cb = _ui_cpc_mem_read;
        desc.write_cb = _ui_cpc_mem_write;
        desc.user_data = ui;
        static const char* titles[] = { "Memory Editor #1", "Memory Editor #2", "Memory Editor #3", "Memory Editor #4" };
        for (int i = 0; i < 4; i++) {
            desc.title = titles[i]; desc.x = x; desc.y = y;
            ui_memedit_init(&ui->memedit[i], &desc);
            x += dx; y += dy;
        }
    }
    x += dx; y += dy;
    {
        ui_memmap_desc_t desc = {0};
        desc.title = "Memory Map";
        desc.x = x;
        desc.y = y;
        ui_memmap_init(&ui->memmap, &desc);
    }
    x += dx; y += dy;
    {
        ui_dasm_desc_t desc = {0};
        for (int i = 0; i < _UI_CPC_MEMLAYER_NUM; i++) {
            desc.layers[i] = _ui_cpc_memlayer_names[i];
        }
        desc.cpu_type = UI_DASM_CPUTYPE_Z80;
        desc.start_addr = 0x0000;
        desc.read_cb = _ui_cpc_mem_read;
        desc.user_data = ui;
        static const char* titles[4] = { "Disassembler #1", "Disassembler #2", "Disassembler #2", "Dissassembler #3" };
        for (int i = 0; i < 4; i++) {
            desc.title = titles[i]; desc.x = x; desc.y = y;
            ui_dasm_init(&ui->dasm[i], &desc);
            x += dx; y += dy;
        }
    }
}

void ui_cpc_discard(ui_cpc_t* ui) {
    CHIPS_ASSERT(ui && ui->cpc);
    ui->cpc = 0;
    ui_z80_discard(&ui->cpu);
    ui_i8255_discard(&ui->ppi);
    ui_upd765_discard(&ui->upd);
    ui_ay38910_discard(&ui->psg);
    ui_mc6845_discard(&ui->vdc);
    ui_am40010_discard(&ui->ga);
    ui_kbd_discard(&ui->kbd);
    ui_audio_discard(&ui->audio);
    ui_fdd_discard(&ui->fdd);
    ui_memmap_discard(&ui->memmap);
    for (int i = 0; i < 4; i++) {
        ui_memedit_discard(&ui->memedit[i]);
        ui_dasm_discard(&ui->dasm[i]);
    }
    ui_dbg_discard(&ui->dbg);
}

void ui_cpc_draw(ui_cpc_t* ui, double time_ms) {
    CHIPS_ASSERT(ui && ui->cpc);
    _ui_cpc_draw_menu(ui, time_ms);
    if (ui->memmap.open) {
        _ui_cpc_update_memmap(ui);
    }
    ui_audio_draw(&ui->audio, ui->cpc->sample_pos);
    ui_fdd_draw(&ui->fdd);
    ui_kbd_draw(&ui->kbd);
    ui_z80_draw(&ui->cpu);
    ui_ay38910_draw(&ui->psg);
    ui_mc6845_draw(&ui->vdc);
    ui_am40010_draw(&ui->ga);
    ui_i8255_draw(&ui->ppi);
    ui_upd765_draw(&ui->upd);
    ui_memmap_draw(&ui->memmap);
    for (int i = 0; i < 4; i++) {
        ui_memedit_draw(&ui->memedit[i]);
        ui_dasm_draw(&ui->dasm[i]);
    }
    ui_dbg_draw(&ui->dbg);
}

bool ui_cpc_before_exec(ui_cpc_t* ui) {
    CHIPS_ASSERT(ui && ui->cpc);
    return ui_dbg_before_exec(&ui->dbg);
}

void ui_cpc_after_exec(ui_cpc_t* ui) {
    CHIPS_ASSERT(ui && ui->cpc);
    ui_dbg_after_exec(&ui->dbg);
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif
#endif /* CHIPS_IMPL */
