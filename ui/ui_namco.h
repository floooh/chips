#pragma once
/*#
    # ui_namco.h

    Integrated debugging UI for namco.h (Pacman / Pengo arcade hardware)

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

    Before including the implementation, define NAMCO_PACMAN or NAMCO_PENGO
    to select the hardware configuration.

    Include the following headers (and their depenencies) before including
    ui_namco.h both for the declaration and implementation:

    - namco.h
    - mem.h
    - ui_chip.h
    - ui_util.h
    - ui_z80.h
    - ui_audio.h
    - ui_dasm.h
    - ui_dbg.h
    - ui_memedit.h
    - ui_memmap.h

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

typedef struct {
    namco_t* sys;
    ui_dbg_create_texture_t create_texture_cb;      /* texture creation callback for ui_dbg_t */
    ui_dbg_update_texture_t update_texture_cb;      /* texture update callback for ui_dbg_t */
    ui_dbg_destroy_texture_t destroy_texture_cb;    /* texture destruction callback for ui_dbg_t */
    ui_dbg_keydesc_t dbg_keys;                      /* user-defined hotkeys for ui_dbg_t */
} ui_namco_desc_t;

typedef struct {
    namco_t* sys;
    ui_z80_t cpu;
    ui_dbg_t dbg;
    ui_audio_t audio;
    ui_memmap_t memmap;
    ui_memedit_t memedit[4];
    ui_dasm_t dasm[4];
} ui_namco_t;

void ui_namco_init(ui_namco_t* ui, const ui_namco_desc_t* desc);
void ui_namco_discard(ui_namco_t* ui);
void ui_namco_draw(ui_namco_t* ui, double time_ms);
bool ui_namco_before_exec(ui_namco_t* ui);
void ui_namco_after_exec(ui_namco_t* ui);

#ifdef __cplusplus
} /* extern "C" */
#endif

/*-- IMPLEMENTATION (include in C++ source) ----------------------------------*/
#ifdef CHIPS_IMPL
#if !defined(NAMCO_PACMAN) && !defined(NAMCO_PENGO)
#error "Please define NAMCO_PACMAN or NAMCO_PENGO before including the implementation"
#endif
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

static const ui_chip_pin_t _ui_namco_cpu_pins[] = {
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

#define _UI_NAMCO_MEMLAYER_MAIN     (0)
#define _UI_NAMCO_MEMLAYER_GFX      (1)
#define _UI_NAMCO_MEMLAYER_PROM     (2)
#define _UI_NAMCO_MEMLAYER_SOUND    (3)
#define _UI_NAMCO_NUM_MEMLAYERS     (4)
static const char* _ui_namco_memlayer_names[_UI_NAMCO_NUM_MEMLAYERS] = {
    "Main", "Gfx", "PROM", "Sound"
};

static uint8_t _ui_namco_mem_read(int layer, uint16_t addr, void* user_data) {
    const ui_namco_t* ui = (const ui_namco_t*) user_data;
    CHIPS_ASSERT(ui && ui->sys);
    switch (layer) {
        case _UI_NAMCO_MEMLAYER_MAIN:
            return mem_rd(&ui->sys->mem, addr);
        case _UI_NAMCO_MEMLAYER_GFX:
            return (addr < sizeof(ui->sys->rom_gfx)) ? ui->sys->rom_gfx[addr] : 0xFF;
        case _UI_NAMCO_MEMLAYER_PROM:
            return (addr < sizeof(ui->sys->rom_prom)) ? ui->sys->rom_prom[addr] : 0xFF;
        case _UI_NAMCO_MEMLAYER_SOUND:
            return (addr < sizeof(ui->sys->sound.rom)) ? ui->sys->sound.rom[addr/0x0100][addr&0x00FF] : 0xFF;
        default:
            return 0xFF;
    }
}

static void _ui_namco_mem_write(int layer, uint16_t addr, uint8_t data, void* user_data) {
    ui_namco_t* ui = (ui_namco_t*) user_data;
    CHIPS_ASSERT(ui && ui->sys);
    switch (layer) {
        case _UI_NAMCO_MEMLAYER_MAIN:
            mem_wr(&ui->sys->mem, addr, data);
            break;
        case _UI_NAMCO_MEMLAYER_GFX:
            if (addr < sizeof(ui->sys->rom_gfx)) {
                ui->sys->rom_gfx[addr] = data;
            }
            break;
        case _UI_NAMCO_MEMLAYER_PROM:
            if (addr < sizeof(ui->sys->rom_prom)) {
                ui->sys->rom_prom[addr] = data;
            }
            break;
        case _UI_NAMCO_MEMLAYER_SOUND:
            if (addr < sizeof(ui->sys->sound.rom)) {
                ui->sys->sound.rom[addr/0x0100][addr&0x00FF] = data;
            }
            break;
        default:
            break;
    }
}

void ui_namco_init(ui_namco_t* ui, const ui_namco_desc_t* ui_desc) {
    CHIPS_ASSERT(ui && ui_desc);
    CHIPS_ASSERT(ui_desc->sys);
    memset(ui, 0, sizeof(ui_namco_t));
    ui->sys = ui_desc->sys;
    int x = 20, y = 20, dx = 10, dy = 10;
    {
        ui_dbg_desc_t desc = {0};
        desc.title = "CPU Debugger";
        desc.x = x;
        desc.y = y;
        desc.z80 = &ui->sys->cpu;
        desc.read_cb = _ui_namco_mem_read;
        desc.read_layer = _UI_NAMCO_MEMLAYER_MAIN;
        desc.create_texture_cb = ui_desc->create_texture_cb;
        desc.update_texture_cb = ui_desc->update_texture_cb;
        desc.destroy_texture_cb = ui_desc->destroy_texture_cb;
        desc.keys = ui_desc->dbg_keys;
        desc.user_data = ui;
        ui_dbg_init(&ui->dbg, &desc);
    }
    x += dx; y += dy;
    {
        ui_z80_desc_t desc = {0};
        desc.title = "Z80 CPU (Main)";
        desc.cpu = &ui->sys->cpu;
        desc.x = x;
        desc.y = y;
        UI_CHIP_INIT_DESC(&desc.chip_desc, "Z80\nCPU", 36, _ui_namco_cpu_pins);
        ui_z80_init(&ui->cpu, &desc);
    }
    x += dx; y += dy;
    {
        ui_audio_desc_t desc = {0};
        desc.title = "Audio Output";
        desc.sample_buffer = ui->sys->sound.sample_buffer;
        desc.num_samples = ui->sys->sound.num_samples;
        desc.x = x;
        desc.y = y;
        ui_audio_init(&ui->audio, &desc);
    }
    x += dx; y += dy;
    {
        ui_memedit_desc_t desc = {0};
        for (int i = 0; i < _UI_NAMCO_NUM_MEMLAYERS; i++) {
            desc.layers[i] = _ui_namco_memlayer_names[i];
        }
        desc.read_cb = _ui_namco_mem_read;
        desc.write_cb = _ui_namco_mem_write;
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
        ui_memmap_layer(&ui->memmap, "Main");
            #if defined(NAMCO_PACMAN)
                ui_memmap_region(&ui->memmap, "ROM", 0x0000, 0x4000, true);
                ui_memmap_region(&ui->memmap, "Video RAM", 0x4000, 0x0400, true);
                ui_memmap_region(&ui->memmap, "Color RAM", 0x4400, 0x0400, true);
                ui_memmap_region(&ui->memmap, "RAM", 0x4C00, 0x0400, true);
            #endif
    }
    x += dx; y += dy;
    {
        ui_dasm_desc_t desc = {0};
        for (int i = 0; i < 2; i++) {
            desc.layers[i] = _ui_namco_memlayer_names[i];
        }
        desc.cpu_type = UI_DASM_CPUTYPE_Z80;
        desc.start_addr = 0x0000;
        desc.read_cb = _ui_namco_mem_read;
        desc.user_data = ui;
        static const char* titles[4] = { "Disassembler #1", "Disassembler #2", "Disassembler #2", "Dissassembler #3" };
        for (int i = 0; i < 4; i++) {
            desc.title = titles[i]; desc.x = x; desc.y = y;
            ui_dasm_init(&ui->dasm[i], &desc);
            x += dx; y += dy;
        }
    }
}

void ui_namco_discard(ui_namco_t* ui) {
    CHIPS_ASSERT(ui && ui->sys);
    ui_dbg_discard(&ui->dbg);
    ui_memmap_discard(&ui->memmap);
    for (int i = 0; i < 4; i++) {
        ui_memedit_discard(&ui->memedit[i]);
        ui_dasm_discard(&ui->dasm[i]);
    }
    ui_z80_discard(&ui->cpu);
}


static void _ui_namco_draw_menu(ui_namco_t* ui, double exec_time) {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("System")) {
            if (ImGui::MenuItem("Reboot")) {
                namco_reset(ui->sys);
                ui_dbg_reboot(&ui->dbg);
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Hardware")) {
            ImGui::MenuItem("Memory Map", 0, &ui->memmap.open);
            ImGui::MenuItem("Audio Output", 0, &ui->audio.open);
            ImGui::MenuItem("Z80", 0, &ui->cpu.open);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Debug")) {
            ImGui::MenuItem("CPU Debugger", 0, &ui->dbg.ui.open);
            ImGui::MenuItem("Breakpoints", 0, &ui->dbg.ui.show_breakpoints);
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
        ui_util_options_menu(exec_time, ui->dbg.dbg.stopped);
        ImGui::EndMainMenuBar();
    }
}

void ui_namco_draw(ui_namco_t* ui, double time_ms) {
    CHIPS_ASSERT(ui && ui->sys);
    _ui_namco_draw_menu(ui, time_ms);
    ui_memmap_draw(&ui->memmap);
    ui_audio_draw(&ui->audio, ui->sys->sound.sample_pos);
    ui_dbg_draw(&ui->dbg);
    ui_z80_draw(&ui->cpu);
    for (int i = 0; i < 4; i++) {
        ui_memedit_draw(&ui->memedit[i]);
        ui_dasm_draw(&ui->dasm[i]);
    }
}

bool ui_namco_before_exec(ui_namco_t* ui) {
    CHIPS_ASSERT(ui && ui->sys);
    return ui_dbg_before_exec(&ui->dbg);
}

void ui_namco_after_exec(ui_namco_t* ui) {
    CHIPS_ASSERT(ui && ui->sys);
    ui_dbg_after_exec(&ui->dbg);
}

#endif /* CHIPS_IMPL */
