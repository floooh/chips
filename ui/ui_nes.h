#pragma once
/*#
    # ui_nes.h

    Integrated debugging UI for nes.h

    Do this:
    ~~~C
    #define CHIPS_UI_IMPL
    ~~~
    before you include this file in *one* C++ file to create the
    implementation.

    Optionally provide the following macros with your own implementation

    ~~~C
    CHIPS_ASSERT(c)
    ~~~
        your own assert macro (default: assert(c))

    Include the following headers (and their depenencies) before including
    ui_nes.h both for the declaration and implementation.

    - nes.h
    - mem.h
    - ui_chip.h
    - ui_util.h
    - ui_m6502.h
    - ui_audio.h
    - ui_dasm.h
    - ui_dbg.h
    - ui_memedit.h
    - ui_memmap.h
    - ui_kbd.h

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

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    nes_t* nes;
    ui_dbg_texture_callbacks_t dbg_texture;     // debug texture create/update/destroy callbacks
    ui_dbg_keys_desc_t dbg_keys;                // user-defined hotkeys for ui_dbg_t
} ui_nes_desc_t;

typedef struct {
    nes_t* nes;
    ui_m6502_t cpu;
    ui_memedit_t memedit[4];
    ui_memedit_t ppu_memedit[4];
    ui_memedit_t sprite_memedit[4];
    ui_memedit_t picture_memedit[4];
    ui_dasm_t dasm[4];
    ui_dbg_t dbg;
} ui_nes_t;

void ui_nes_init(ui_nes_t* ui, const ui_nes_desc_t* desc);
void ui_nes_discard(ui_nes_t* ui);
void ui_nes_draw(ui_nes_t* ui);
chips_debug_t ui_nes_get_debug(ui_nes_t* ui);

#ifdef __cplusplus
} /* extern "C" */
#endif

/*-- IMPLEMENTATION (include in C++ source) ----------------------------------*/
#ifdef CHIPS_UI_IMPL
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

static void _ui_nes_draw_menu(ui_nes_t* ui) {
    CHIPS_ASSERT(ui && ui->nes);
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("System")) {
            if (ImGui::MenuItem("Reset")) {
                nes_reset(ui->nes);
                ui_dbg_reset(&ui->dbg);
            }
            if (ImGui::MenuItem("Cold Boot")) {
                ui_dbg_reboot(&ui->dbg);
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Hardware")) {
            ImGui::MenuItem("MOS 6502 (CPU)", 0, &ui->cpu.open);
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
            if (ImGui::BeginMenu("PPU Memory Editor")) {
                ImGui::MenuItem("Window #1", 0, &ui->ppu_memedit[0].open);
                ImGui::MenuItem("Window #2", 0, &ui->ppu_memedit[1].open);
                ImGui::MenuItem("Window #3", 0, &ui->ppu_memedit[2].open);
                ImGui::MenuItem("Window #4", 0, &ui->ppu_memedit[3].open);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Sprite Memory Editor")) {
                ImGui::MenuItem("Window #1", 0, &ui->sprite_memedit[0].open);
                ImGui::MenuItem("Window #2", 0, &ui->sprite_memedit[1].open);
                ImGui::MenuItem("Window #3", 0, &ui->sprite_memedit[2].open);
                ImGui::MenuItem("Window #4", 0, &ui->sprite_memedit[3].open);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Picture Memory Editor")) {
                ImGui::MenuItem("Window #1", 0, &ui->picture_memedit[0].open);
                ImGui::MenuItem("Window #2", 0, &ui->picture_memedit[1].open);
                ImGui::MenuItem("Window #3", 0, &ui->picture_memedit[2].open);
                ImGui::MenuItem("Window #4", 0, &ui->picture_memedit[3].open);
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
        ui_util_options_menu();
        ImGui::EndMainMenuBar();
    }
}

static const ui_chip_pin_t _ui_nes_cpu_pins[] = {
    { "D0",     0,      M6502_D0 },
    { "D1",     1,      M6502_D1 },
    { "D2",     2,      M6502_D2 },
    { "D3",     3,      M6502_D3 },
    { "D4",     4,      M6502_D4 },
    { "D5",     5,      M6502_D5 },
    { "D6",     6,      M6502_D6 },
    { "D7",     7,      M6502_D7 },
    { "RW",     9,      M6502_RW },
    { "SYNC",   10,     M6502_SYNC },
    { "IRQ",    11,     M6502_IRQ },
    { "NMI",    12,     M6502_NMI },
    { "RDY",    13,     M6502_RDY },
    { "RES",    14,     M6502_RES },
    { "A0",     16,     M6502_A0 },
    { "A1",     17,     M6502_A1 },
    { "A2",     18,     M6502_A2 },
    { "A3",     19,     M6502_A3 },
    { "A4",     20,     M6502_A4 },
    { "A5",     21,     M6502_A5 },
    { "A6",     22,     M6502_A6 },
    { "A7",     23,     M6502_A7 },
    { "A8",     24,     M6502_A8 },
    { "A9",     25,     M6502_A9 },
    { "A10",    26,     M6502_A10 },
    { "A11",    27,     M6502_A11 },
    { "A12",    28,     M6502_A12 },
    { "A13",    29,     M6502_A13 },
    { "A14",    30,     M6502_A14 },
    { "A15",    31,     M6502_A15 },
};

static uint8_t _ui_nes_mem_read(int layer, uint16_t addr, void* user_data) {
    (void)layer;
    CHIPS_ASSERT(user_data);
    ui_nes_t* ui_nes = (ui_nes_t*) user_data;
    nes_t* nes = ui_nes->nes;
    if (addr < 0x2000) {
        /* CPU mapped RAM layer */
        return nes->ram[addr & 0x7ff];
    }
    if (addr >= 0x4016 && addr <= 0x4017) {
        return nes->controller_state[addr & 0x0001];
    }
    if (addr < 0x4020) {
        // TODO:
        return 0xff;
    }
    if (addr < 0x8000) {
        // TODO:
        return 0xff;
    }
    if (addr < 0xc000) {
        return nes->cart.rom[addr - 0x8000];
    } else {
        if(nes->cart.header.prg_page_count == 1) {
            return nes->cart.rom[addr - 0xc000];
        } else {
            return nes->cart.rom[addr - 0x8000];
        }
    }
    return 0xff;
}

static void _ui_nes_mem_write(int layer, uint16_t addr, uint8_t data, void* user_data) {
    (void)layer;
    CHIPS_ASSERT(user_data);
    ui_nes_t* ui_nes = (ui_nes_t*) user_data;
    nes_t* nes = ui_nes->nes;
    if (addr < 0x2000) {
        nes->ram[addr & 0x7ff] = data;
    } else if (addr >= 0x4016 && addr <= 0x4017) {
        nes->controller_state[addr & 0x0001] = data;
    } else if (addr < 0x4020) {
        // TODO:
    } else if (addr < 0x8000) {
        // TODO:
    } else if (addr < 0xc000) {
        nes->cart.rom[addr - 0x8000] = data;
    } else if(nes->cart.header.prg_page_count == 1) {
            nes->cart.rom[addr - 0xc000] = data;
    } else {
        nes->cart.rom[addr - 0x8000] = data;
    }
}

static uint8_t _ui_nes_ppu_mem_read(int layer, uint16_t addr, void* user_data) {
    (void)layer;
    CHIPS_ASSERT(user_data);
    ui_nes_t* ui_nes = (ui_nes_t*) user_data;
    nes_t* nes = ui_nes->nes;
    if (addr < 0x2000) {
        return nes->character_ram[addr];
    }
    if (addr >= 0x2000 && addr < 0x3f00) {
        const uint16_t index = addr & 0x3ff;
        // Name tables upto 0x3000, then mirrored upto 3eff
        uint16_t normalizedAddr = addr;
        if (addr >= 0x3000)
            normalizedAddr -= 0x1000;

        if (normalizedAddr < 0x2400)      //NT0
            normalizedAddr = nes->ppu_name_table[0] + index;
        else if (normalizedAddr < 0x2800) //NT1
            normalizedAddr = nes->ppu_name_table[1] + index;
        else if (normalizedAddr < 0x2c00) //NT2
            normalizedAddr = nes->ppu_name_table[2] + index;
        else
            normalizedAddr = nes->ppu_name_table[3] + index;
        return nes->ppu_ram[normalizedAddr-0x2000];
    }
    if (addr >= 0x3f00 && addr < 0x4000) {
        uint16_t normalizedAddr = addr & 0x1f;
        // Addresses $3F10/$3F14/$3F18/$3F1C are mirrors of $3F00/$3F04/$3F08/$3F0C
        if (normalizedAddr >= 0x10 && addr % 4 == 0) {
            normalizedAddr = normalizedAddr & 0xf;
        }
        return nes->ppu_pal_ram[normalizedAddr];
    }
    return 0xFF;
}

static void _ui_nes_ppu_mem_write(int layer, uint16_t addr, uint8_t data, void* user_data) {
    (void)layer;
    CHIPS_ASSERT(user_data);
    ui_nes_t* ui_nes = (ui_nes_t*) user_data;
    nes_t* nes = ui_nes->nes;
    if (addr < 0x2000) {
        nes->character_ram[addr] = data;
    }
    else if (addr >= 0x2000 && addr < 0x3f00) {
        const uint16_t index = addr & 0x3ff;
        // Name tables upto 0x3000, then mirrored upto 3eff
        uint16_t normalizedAddr = addr;
        if (addr >= 0x3000)
            normalizedAddr -= 0x1000;

        if (normalizedAddr < 0x2400)      //NT0
            normalizedAddr = nes->ppu_name_table[0] + index;
        else if (normalizedAddr < 0x2800) //NT1
            normalizedAddr = nes->ppu_name_table[1] + index;
        else if (normalizedAddr < 0x2c00) //NT2
            normalizedAddr = nes->ppu_name_table[2] + index;
        else
            normalizedAddr = nes->ppu_name_table[3] + index;
        nes->ppu_ram[normalizedAddr-0x2000] = data;
    }
    if (addr >= 0x3f00 && addr < 0x4000) {
        uint16_t normalizedAddr = addr & 0x1f;
        // Addresses $3F10/$3F14/$3F18/$3F1C are mirrors of $3F00/$3F04/$3F08/$3F0C
        if (normalizedAddr >= 0x10 && addr % 4 == 0) {
            normalizedAddr = normalizedAddr & 0xf;
        }
        nes->ppu_pal_ram[normalizedAddr] = data;
    }
}

static uint8_t _ui_nes_sprite_mem_read(int layer, uint16_t addr, void* user_data) {
    (void)layer;
    CHIPS_ASSERT(user_data);
    ui_nes_t* ui_nes = (ui_nes_t*) user_data;
    nes_t* nes = ui_nes->nes;
    if (addr >= 0 && addr < 64*4 ) {
        return nes->ppu.sprite_memory[addr];
    }
    return 0xFF;
}

static void _ui_nes_sprite_mem_write(int layer, uint16_t addr, uint8_t data, void* user_data) {
    (void)layer;
    CHIPS_ASSERT(user_data);
    ui_nes_t* ui_nes = (ui_nes_t*) user_data;
    nes_t* nes = ui_nes->nes;
    if (addr >= 0 && addr < 64*4 ) {
        nes->ppu.sprite_memory[addr] = data;
    }
}

static uint8_t _ui_nes_picture_mem_read(int layer, uint16_t addr, void* user_data) {
    (void)layer;
    CHIPS_ASSERT(user_data);
    ui_nes_t* ui_nes = (ui_nes_t*) user_data;
    nes_t* nes = ui_nes->nes;
    if (addr >= 0 && addr < 256*240 ) {
        return nes->ppu.picture_buffer[addr];
    }
    return 0xFF;
}

static void _ui_nes_picture_mem_write(int layer, uint16_t addr, uint8_t data, void* user_data) {
    (void)layer;
    CHIPS_ASSERT(user_data);
    ui_nes_t* ui_nes = (ui_nes_t*) user_data;
    nes_t* nes = ui_nes->nes;
    if (addr >= 0 && addr < 256*240 ) {
        nes->ppu.picture_buffer[addr] = data;
    }
}

static void _ui_nes_draw_hw_colors(ui_nes_t* ui) {
    ImGui::Text("Hardware Colors:");
    const ImVec2 size(18,18);
    for (int i = 0; i < 64; i++) {
        ImGui::PushID(i);
        ImGui::ColorButton("##hw_color", ImColor(ppu_palette[i]), ImGuiColorEditFlags_NoAlpha, size);
        ImGui::PopID();
        if (((i+1) % 16) != 0) {
            ImGui::SameLine();
        }
    }

    nes_t* nes = ui->nes;
    ImGui::Text("Background palette 0:");
    uint16_t addr = 0x0;
    for (int i = 0; i < 4; i++) {
        ImGui::PushID(i);
        uint8_t pal_index = nes->ppu_pal_ram[addr + i];
        ImGui::ColorButton("##hw_color", ImColor(ppu_palette[pal_index]), ImGuiColorEditFlags_NoAlpha, size);
        ImGui::PopID();
        if (((i+1) % 4) != 0) {
            ImGui::SameLine();
        }
    }

    ImGui::Text("Background palette 1:");
    addr = 0x4;
    for (int i = 0; i < 4; i++) {
        ImGui::PushID(i);
        uint8_t pal_index = nes->ppu_pal_ram[addr + i];
        ImGui::ColorButton("##hw_color", ImColor(ppu_palette[pal_index]), ImGuiColorEditFlags_NoAlpha, size);
        ImGui::PopID();
        if (((i+1) % 4) != 0) {
            ImGui::SameLine();
        }
    }

    ImGui::Text("Background palette 2:");
    addr = 0x8;
    for (int i = 0; i < 4; i++) {
        ImGui::PushID(i);
        uint8_t pal_index = nes->ppu_pal_ram[addr + i];
        ImGui::ColorButton("##hw_color", ImColor(ppu_palette[pal_index]), ImGuiColorEditFlags_NoAlpha, size);
        ImGui::PopID();
        if (((i+1) % 4) != 0) {
            ImGui::SameLine();
        }
    }

    ImGui::Text("Background palette 3:");
    addr = 0xc;
    for (int i = 0; i < 4; i++) {
        ImGui::PushID(i);
        uint8_t pal_index = nes->ppu_pal_ram[addr + i];
        ImGui::ColorButton("##hw_color", ImColor(ppu_palette[pal_index]), ImGuiColorEditFlags_NoAlpha, size);
        ImGui::PopID();
        if (((i+1) % 4) != 0) {
            ImGui::SameLine();
        }
    }
}

void ui_nes_init(ui_nes_t* ui, const ui_nes_desc_t* ui_desc) {
    CHIPS_ASSERT(ui && ui_desc);
    CHIPS_ASSERT(ui_desc->nes);
    ui->nes = ui_desc->nes;
    int x = 20, y = 20, dx = 10, dy = 10;
    {
        ui_dbg_desc_t desc = {0};
        desc.title = "CPU Debugger";
        desc.x = x;
        desc.y = y;
        desc.m6502 = &ui->nes->cpu;
        desc.read_cb = _ui_nes_mem_read;
        desc.texture_cbs = ui_desc->dbg_texture;
        desc.keys = ui_desc->dbg_keys;
        desc.user_data = ui;
        ui_dbg_init(&ui->dbg, &desc);
    }
    x += dx; y += dy;
    {
        ui_m6502_desc_t desc = {0};
        desc.title = "MOS 6502";
        desc.cpu = &ui->nes->cpu;
        desc.x = x;
        desc.y = y;
        UI_CHIP_INIT_DESC(&desc.chip_desc, "6502", 32, _ui_nes_cpu_pins);
        ui_m6502_init(&ui->cpu, &desc);
    }
    x += dx; y += dy;
    {
        ui_memedit_desc_t desc = {0};
        desc.layers[0] = "System";
        desc.read_cb = _ui_nes_mem_read;
        desc.write_cb = _ui_nes_mem_write;
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
        ui_memedit_desc_t desc = {.mem_size = 0x4000};
        desc.layers[0] = "PPU";
        desc.read_cb = _ui_nes_ppu_mem_read;
        desc.write_cb = _ui_nes_ppu_mem_write;
        desc.user_data = ui;
        static const char* titles[] = { "PPU Memory Editor #1", "PPU Memory Editor #2", "PPU Memory Editor #3", "PPU Memory Editor #4" };
        for (int i = 0; i < 4; i++) {
            desc.title = titles[i]; desc.x = x; desc.y = y;
            ui_memedit_init(&ui->ppu_memedit[i], &desc);
            x += dx; y += dy;
        }
    }
    x += dx; y += dy;
    {
        ui_memedit_desc_t desc = {.mem_size = 0x100};
        desc.layers[0] = "Sprite";
        desc.read_cb = _ui_nes_sprite_mem_read;
        desc.write_cb = _ui_nes_sprite_mem_write;
        desc.user_data = ui;
        static const char* titles[] = { "Sprite Memory Editor #1", "Sprite Memory Editor #2", "Sprite Memory Editor #3", "Sprite Memory Editor #4" };
        for (int i = 0; i < 4; i++) {
            desc.title = titles[i]; desc.x = x; desc.y = y;
            ui_memedit_init(&ui->sprite_memedit[i], &desc);
            x += dx; y += dy;
        }
    }
    x += dx; y += dy;
    {
        ui_memedit_desc_t desc = {.mem_size = 256*240};
        desc.layers[0] = "Picture";
        desc.read_cb = _ui_nes_picture_mem_read;
        desc.write_cb = _ui_nes_picture_mem_write;
        desc.user_data = ui;
        static const char* titles[] = { "Picture Memory Editor #1", "Picture Memory Editor #2", "Picture Memory Editor #3", "Picture Memory Editor #4" };
        for (int i = 0; i < 4; i++) {
            desc.title = titles[i]; desc.x = x; desc.y = y;
            ui_memedit_init(&ui->picture_memedit[i], &desc);
            x += dx; y += dy;
        }
    }
    x += dx; y += dy;
    {
        ui_dasm_desc_t desc = {0};
        desc.layers[0] = "System";
        desc.cpu_type = UI_DASM_CPUTYPE_M6502;
        desc.user_data = ui->nes;
        static const char* titles[4] = { "Disassembler #1", "Disassembler #2", "Disassembler #2", "Dissassembler #3" };
        for (int i = 0; i < 4; i++) {
            desc.title = titles[i]; desc.x = x; desc.y = y;
            ui_dasm_init(&ui->dasm[i], &desc);
            x += dx; y += dy;
        }
    }
}

void ui_nes_discard(ui_nes_t* ui) {
    CHIPS_ASSERT(ui && ui->nes);
    ui->nes = 0;
    ui_m6502_discard(&ui->cpu);
    for (int i = 0; i < 4; i++) {
        ui_memedit_discard(&ui->memedit[i]);
        ui_memedit_discard(&ui->ppu_memedit[i]);
        ui_memedit_discard(&ui->sprite_memedit[i]);
        ui_memedit_discard(&ui->picture_memedit[i]);
        ui_dasm_discard(&ui->dasm[i]);
    }
    ui_dbg_discard(&ui->dbg);
}

void ui_nes_draw(ui_nes_t* ui) {
    CHIPS_ASSERT(ui && ui->nes);
    _ui_nes_draw_menu(ui);
    ui_m6502_draw(&ui->cpu);
    for (int i = 0; i < 4; i++) {
        ui_memedit_draw(&ui->memedit[i]);
        ui_memedit_draw(&ui->ppu_memedit[i]);
        ui_memedit_draw(&ui->sprite_memedit[i]);
        ui_memedit_draw(&ui->picture_memedit[i]);
        ui_dasm_draw(&ui->dasm[i]);
    }
    _ui_nes_draw_hw_colors(ui);
    ui_dbg_draw(&ui->dbg);
}

chips_debug_t ui_nes_get_debug(ui_nes_t* ui) {
    CHIPS_ASSERT(ui);
    chips_debug_t res = {};
    res.callback.func = (chips_debug_func_t)ui_dbg_tick;
    res.callback.user_data = &ui->dbg;
    res.stopped = &ui->dbg.dbg.stopped;
    return res;
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif
#endif /* CHIPS_UI_IMPL */
