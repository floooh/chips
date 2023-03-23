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
    int x, y;
    int w, h;
    bool open;
    ui_dbg_texture_callbacks_t texture_cbs;
    void* tex_pattern_tables[2];
    void* tex_name_tables;
    void* tex_sprites;
    int pattern_pal_index;
    uint32_t pixel_buffer[512*512];
    uint32_t pixel_buffer2[256*256];
    uint32_t pixel_buffer3[64*64];
} ui_nes_video_t;

typedef struct {
    nes_t* nes;
    ui_m6502_t cpu;
    ui_audio_t audio;
    ui_memedit_t memedit[4];
    ui_memedit_t ppu_memedit[4];
    ui_memedit_t sprite_memedit[4];
    ui_memedit_t oam_memedit[4];
    ui_dasm_t dasm[4];
    ui_nes_video_t video;
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
            ImGui::MenuItem("Audio Output", 0, &ui->audio.open);
            ImGui::MenuItem("MOS 6502 (CPU)", 0, &ui->cpu.open);
            ImGui::MenuItem("Video Hardware", 0, &ui->video.open);
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
            if (ImGui::BeginMenu("OAM Memory Editor")) {
                ImGui::MenuItem("Window #1", 0, &ui->oam_memedit[0].open);
                ImGui::MenuItem("Window #2", 0, &ui->oam_memedit[1].open);
                ImGui::MenuItem("Window #3", 0, &ui->oam_memedit[2].open);
                ImGui::MenuItem("Window #4", 0, &ui->oam_memedit[3].open);
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
    return nes_mem_read(nes, addr, true);
}

static void _ui_nes_mem_write(int layer, uint16_t addr, uint8_t data, void* user_data) {
    (void)layer;
    CHIPS_ASSERT(user_data);
    ui_nes_t* ui_nes = (ui_nes_t*) user_data;
    nes_t* nes = ui_nes->nes;
    nes_ppu_write(nes, addr, data);
}

static uint8_t _ui_nes_ppu_mem_read(int layer, uint16_t addr, void* user_data) {
    (void)layer;
    CHIPS_ASSERT(user_data);
    ui_nes_t* ui_nes = (ui_nes_t*) user_data;
    nes_t* nes = ui_nes->nes;
    return nes_ppu_read(nes, addr);
}

static void _ui_nes_ppu_mem_write(int layer, uint16_t addr, uint8_t data, void* user_data) {
    (void)layer;
    CHIPS_ASSERT(user_data);
    ui_nes_t* ui_nes = (ui_nes_t*) user_data;
    nes_t* nes = ui_nes->nes;
    nes_ppu_write(nes, addr, data);
}

static uint8_t _ui_nes_sprite_mem_read(int layer, uint16_t addr, void* user_data) {
    (void)layer;
    CHIPS_ASSERT(user_data);
    ui_nes_t* ui_nes = (ui_nes_t*) user_data;
    nes_t* nes = ui_nes->nes;
    if (addr >= 0 && addr < 64*4 ) {
        return nes->ppu.oam.reg[addr];
    }
    return 0xFF;
}

static void _ui_nes_sprite_mem_write(int layer, uint16_t addr, uint8_t data, void* user_data) {
    (void)layer;
    CHIPS_ASSERT(user_data);
    ui_nes_t* ui_nes = (ui_nes_t*) user_data;
    nes_t* nes = ui_nes->nes;
    if (addr >= 0 && addr < 64*4 ) {
        nes->ppu.oam.reg[addr] = data;
    }
}

static uint8_t _ui_nes_oam_mem_read(int layer, uint16_t addr, void* user_data) {
    (void)layer;
    CHIPS_ASSERT(user_data);
    ui_nes_t* ui_nes = (ui_nes_t*) user_data;
    nes_t* nes = ui_nes->nes;
    if (addr >= 0 && addr < 64*4 ) {
        return nes->ppu.oam.reg[addr];
    }
    return 0xFF;
}

static void _ui_nes_oam_mem_write(int layer, uint16_t addr, uint8_t data, void* user_data) {
    (void)layer;
    CHIPS_ASSERT(user_data);
    ui_nes_t* ui_nes = (ui_nes_t*) user_data;
    nes_t* nes = ui_nes->nes;
    if (addr >= 0 && addr < 64*4 ) {
        nes->ppu.oam.reg[addr] = data;
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
        ui_audio_desc_t desc = {0};
        desc.title = "Audio Output";
        desc.sample_buffer = ui->nes->audio.sample_buffer;
        desc.num_samples = ui->nes->audio.num_samples;
        desc.x = x;
        desc.y = y;
        ui_audio_init(&ui->audio, &desc);
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
        ui_memedit_desc_t desc = {.mem_size = 64*4};
        desc.layers[0] = "OAM";
        desc.read_cb = _ui_nes_oam_mem_read;
        desc.write_cb = _ui_nes_oam_mem_write;
        desc.user_data = ui;
        static const char* titles[] = { "OAM Memory Editor #1", "OAM Memory Editor #2", "OAM Memory Editor #3", "OAM Memory Editor #4" };
        for (int i = 0; i < 4; i++) {
            desc.title = titles[i]; desc.x = x; desc.y = y;
            ui_memedit_init(&ui->oam_memedit[i], &desc);
            x += dx; y += dy;
        }
    }
    x += dx; y += dy;
    {
        ui_dasm_desc_t desc = {0};
        desc.layers[0] = "System";
        desc.read_cb = _ui_nes_mem_read;
        desc.cpu_type = UI_DASM_CPUTYPE_M6502;
        desc.user_data = ui;
        static const char* titles[4] = { "Disassembler #1", "Disassembler #2", "Disassembler #2", "Dissassembler #3" };
        for (int i = 0; i < 4; i++) {
            desc.title = titles[i]; desc.x = x; desc.y = y;
            ui_dasm_init(&ui->dasm[i], &desc);
            x += dx; y += dy;
        }
    }
    {
        ui->video.texture_cbs = ui_desc->dbg_texture;
        ui->video.x = 10;
        ui->video.y = 20;
        ui->video.w = 450;
        ui->video.h = 568;
        ui->video.tex_pattern_tables[0] = ui->video.texture_cbs.create_cb(256, 256);
        ui->video.tex_pattern_tables[1] = ui->video.texture_cbs.create_cb(256, 256);
        ui->video.tex_name_tables = ui->video.texture_cbs.create_cb(512, 512);
        ui->video.tex_sprites = ui->video.texture_cbs.create_cb(64, 64);
    }
}

void ui_nes_discard(ui_nes_t* ui) {
    CHIPS_ASSERT(ui && ui->nes);
    ui->video.texture_cbs.destroy_cb(ui->video.tex_pattern_tables[0]);
    ui->video.texture_cbs.destroy_cb(ui->video.tex_pattern_tables[1]);
    ui->video.texture_cbs.destroy_cb(ui->video.tex_name_tables);
    ui->video.texture_cbs.destroy_cb(ui->video.tex_sprites);
    ui_m6502_discard(&ui->cpu);
    ui_audio_discard(&ui->audio);
    for (int i = 0; i < 4; i++) {
        ui_memedit_discard(&ui->memedit[i]);
        ui_memedit_discard(&ui->ppu_memedit[i]);
        ui_memedit_discard(&ui->sprite_memedit[i]);
        ui_memedit_discard(&ui->oam_memedit[i]);
        ui_dasm_discard(&ui->dasm[i]);
    }
    ui_dbg_discard(&ui->dbg);
}

static inline uint16_t _nes_pal_addr(uint8_t pal_type, uint8_t pal_index) {
    CHIPS_ASSERT(pal_index >= 0);
    CHIPS_ASSERT(pal_index < 4);
    return 0x3f00 + (pal_type * 0x10) + (pal_index << 2);
}

static void _ui_nes_decode_pattern_tile(ui_nes_t* ui, uint8_t table_nr, uint8_t pal_type, uint8_t tile_index, uint32_t* dst, int w, int pal_index) {
    uint16_t tile_addr = 0x1000 * table_nr + (tile_index << 4);
    uint16_t pal_addr = _nes_pal_addr(pal_type, pal_index);
    for(int py = 0; py < 8; py++) {
        uint8_t bp1_byte = _ui_nes_ppu_mem_read(0, tile_addr, ui);
        uint8_t bp2_byte = _ui_nes_ppu_mem_read(0, tile_addr + 8, ui);
        uint8_t x_shift = 7;
        for(int px = 0; px < 8; px++) {
            uint8_t col_index = (((bp2_byte >> x_shift) & 1) << 1) | ((bp1_byte >> x_shift) & 1);
            CHIPS_ASSERT(col_index >= 0);
            CHIPS_ASSERT(col_index < 4);
            uint8_t p_index = _ui_nes_ppu_mem_read(0, pal_addr + col_index, ui);
            *dst = ppu_palette[p_index];
            dst++;
            x_shift--;
        }
        dst += (w - 8);
        tile_addr++;
    }
}

static void _ui_nes_decode_pattern_table(ui_nes_t* ui, uint8_t pal_type, uint8_t pattern_table_nr, int pal_index) {
    uint32_t* dst = ui->video.pixel_buffer2;
    int tile_index = 0;
    for(int py = 0; py < 16; py++) {
        for(int px = 0; px < 16; px++) {
            int dst_offset = (py << 11) + (px << 3);
            _ui_nes_decode_pattern_tile(ui, pattern_table_nr, pal_type, tile_index, dst+dst_offset, 256, pal_index);
            tile_index++;
        }
    }
}

inline static uint16_t _ui_nes_tile_address(uint8_t table_nr) {
    return 0x2000 + 0x400 * table_nr;
}


inline static uint16_t _ui_nes_att_address(int x, int y, uint8_t table_nr) {
    return 0x23c0 + (table_nr * 0x400) + (y / 4) * 8 + (x / 4);
}

inline static uint8_t _ui_nes_table_nr(int tile_x, int tile_y) {
    return (tile_x / 32) + ((tile_y / 32) << 1);
}

static uint8_t _ui_nes_attributes(ui_nes_t* ui, int tile_x, int tile_y, uint8_t table_nr) {
    CHIPS_ASSERT(tile_x >= 0 && tile_x < 32);
    CHIPS_ASSERT(tile_y >= 0 && tile_y < 32);
    CHIPS_ASSERT(table_nr >= 0 && table_nr < 4);
    uint16_t addr = _ui_nes_att_address(tile_x, tile_y, table_nr);
    return _ui_nes_ppu_mem_read(0, addr, ui);
}

static uint8_t _ui_nes_pal_index(ui_nes_t* ui, int tile_x, int tile_y, uint8_t table_nr) {
    uint8_t att = _ui_nes_attributes(ui, tile_x, tile_y, table_nr);
    uint8_t oam_shift = ((tile_x % 2) + ((tile_y % 2) << 1)) << 1;
    uint8_t pal_index = (att >> oam_shift) & 3;
    return pal_index;
}

static void _ui_nes_decode_name_table(ui_nes_t* ui, int x, int y) {
    uint8_t table_nr = (y << 1) + x;
    uint16_t addr = _ui_nes_tile_address(table_nr);
    uint32_t* dst = ui->video.pixel_buffer;
    for(int tile_y = 0; tile_y < 30; tile_y++) {
        for(int tile_x = 0; tile_x < 32; tile_x++) {
            uint8_t tile_index = _ui_nes_ppu_mem_read(0, addr, ui);
            int dst_offset = (tile_x << 3) + (tile_y * 8 * 512) + (x * 256) + (y * 512 * 256);
            uint8_t att = _ui_nes_attributes(ui, tile_x, tile_y, table_nr);
            int att_shift = ((addr >> 4) & 4) | (addr & 2);
            uint8_t pal_index = (att >> att_shift) & 3;
            _ui_nes_decode_pattern_tile(ui, 1, 0, tile_index, dst + dst_offset, 512, pal_index);
            addr++;
        }
    }
}

static void _ui_nes_update_pattern_table(ui_nes_t* ui, int table_nr, int pal_index) {
    _ui_nes_decode_pattern_table(ui, table_nr, 0, pal_index);
    ui->video.texture_cbs.update_cb(ui->video.tex_pattern_tables[table_nr], ui->video.pixel_buffer2, 256*256*sizeof(uint32_t));
}

static void _ui_nes_update_names_tables(ui_nes_t* ui) {
    for (int y = 0; y < 2; y++) {
        for (int x = 0; x < 2; x++) {
            _ui_nes_decode_name_table(ui, x, y);
        }
    }
    ui->video.texture_cbs.update_cb(ui->video.tex_name_tables, ui->video.pixel_buffer, 512*512*sizeof(uint32_t));
}

static void _ui_nes_decode_sprite(ui_nes_t* ui, int sprite_x, int sprite_y, int pattern_table) {
    uint8_t* p = ui->nes->ppu.oam.reg + sprite_x*4+sprite_y*32;
    uint8_t tile_index = p[1];
    uint8_t pal_index = p[2] & 3;
    int dst_offset = sprite_x*8+(sprite_y*8*8*8);
    uint32_t* dst = ui->video.pixel_buffer3 + dst_offset;
    _ui_nes_decode_pattern_tile(ui, pattern_table, 1, tile_index, dst, 64, pal_index);
}

static void _ui_nes_update_sprites(ui_nes_t* ui) {
    uint8_t table_nr = (_ui_nes_ppu_mem_read(0, 0x2000, ui) >> 3) & 1;
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            _ui_nes_decode_sprite(ui, x, y, table_nr);
        }
    }
    ui->video.texture_cbs.update_cb(ui->video.tex_sprites, ui->video.pixel_buffer3, 64*64*sizeof(uint32_t));
}

static void _ui_nes_draw_video(ui_nes_t* ui) {
    if (!ui->video.open) {
        return;
    }
    ImGui::SetNextWindowPos(ImVec2((float)ui->video.x, (float)ui->video.y), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2((float)ui->video.w, (float)ui->video.h), ImGuiCond_Once);
    if (ImGui::Begin("Video Hardware", &ui->video.open)) {
        const ImVec2 size(18, 18);
        if (ImGui::CollapsingHeader("Palette", ImGuiTreeNodeFlags_DefaultOpen)) {
            for (int row = 0; row < 4; row++) {
                for (int col = 0; col < 16; col++) {
                    int pal_index = col + row * 16;
                    ImGui::PushID(pal_index);
                    ImGui::ColorButton("##hw_color", ImColor(ppu_palette[pal_index]), ImGuiColorEditFlags_NoAlpha, size);
                    ImGui::PopID();
                    if (col != 15) {
                        ImGui::SameLine();
                    }
                }
            }
            {
                ImGui::Text("Palette 0");
                uint16_t addr = 0x3f00;
                for (int i = 0; i < 4; i++) {
                    ImGui::PushID(i);
                    uint8_t pal_index = _ui_nes_ppu_mem_read(0, addr + i, ui);
                    ImGui::ColorButton("##hw_p0color1", ImColor(ppu_palette[pal_index]), ImGuiColorEditFlags_NoAlpha, size);
                    ImGui::PopID();
                    if(i != 3) {
                        ImGui::SameLine();
                    }
                }
                addr = 0x3f10;
                for (int i = 0; i < 4; i++) {
                    ImGui::PushID(i);
                    uint8_t pal_index = _ui_nes_ppu_mem_read(0, addr + i, ui);
                    ImGui::ColorButton("##hw_p0color2", ImColor(ppu_palette[pal_index]), ImGuiColorEditFlags_NoAlpha, size);
                    ImGui::PopID();
                    if(i != 3) {
                        ImGui::SameLine();
                    }
                }
            }
            {
                ImGui::Text("Palette 1");
                uint16_t addr = 0x3f04;
                for (int i = 0; i < 4; i++) {
                    ImGui::PushID(i);
                    uint8_t pal_index = _ui_nes_ppu_mem_read(0, addr + i, ui);
                    ImGui::ColorButton("##hw_p1color1", ImColor(ppu_palette[pal_index]), ImGuiColorEditFlags_NoAlpha, size);
                    ImGui::PopID();
                    if(i != 3) {
                        ImGui::SameLine();
                    }
                }
                addr = 0x3f14;
                for (int i = 0; i < 4; i++) {
                    ImGui::PushID(i);
                    uint8_t pal_index = _ui_nes_ppu_mem_read(0, addr + i, ui);
                    ImGui::ColorButton("##hw_p1color2", ImColor(ppu_palette[pal_index]), ImGuiColorEditFlags_NoAlpha, size);
                    ImGui::PopID();
                    if(i != 3) {
                        ImGui::SameLine();
                    }
                }
            }
            {
                ImGui::Text("Palette 2");
                uint16_t addr = 0x3f08;
                for (int i = 0; i < 4; i++) {
                    ImGui::PushID(i);
                    uint8_t pal_index = _ui_nes_ppu_mem_read(0, addr + i, ui);
                    ImGui::ColorButton("##hw_p2color1", ImColor(ppu_palette[pal_index]), ImGuiColorEditFlags_NoAlpha, size);
                    ImGui::PopID();
                    if(i != 3) {
                        ImGui::SameLine();
                    }
                }
                addr = 0x3f18;
                for (int i = 0; i < 4; i++) {
                    ImGui::PushID(i);
                    uint8_t pal_index = _ui_nes_ppu_mem_read(0, addr + i, ui);
                    ImGui::ColorButton("##hw_p2color2", ImColor(ppu_palette[pal_index]), ImGuiColorEditFlags_NoAlpha, size);
                    ImGui::PopID();
                    if(i != 3) {
                        ImGui::SameLine();
                    }
                }
            }
            {
                ImGui::Text("Palette 3");
                uint16_t addr = 0x3f0c;
                for (int i = 0; i < 4; i++) {
                    ImGui::PushID(i);
                    uint8_t pal_index = _ui_nes_ppu_mem_read(0, addr + i, ui);
                    ImGui::ColorButton("##hw_p3color1", ImColor(ppu_palette[pal_index]), ImGuiColorEditFlags_NoAlpha, size);
                    ImGui::PopID();
                    if(i != 3) {
                        ImGui::SameLine();
                    }
                }
                addr = 0x3f1c;
                for (int i = 0; i < 4; i++) {
                    ImGui::PushID(i);
                    uint8_t pal_index = _ui_nes_ppu_mem_read(0, addr + i, ui);
                    ImGui::ColorButton("##hw_p3color2", ImColor(ppu_palette[pal_index]), ImGuiColorEditFlags_NoAlpha, size);
                    ImGui::PopID();
                    if(i != 3) {
                        ImGui::SameLine();
                    }
                }
            }
        }
        if (ImGui::CollapsingHeader("Pattern tables", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::SliderInt("Palette #", &ui->video.pattern_pal_index, 0, 3);
            const ImVec2 p = ImGui::GetCursorPos();
            _ui_nes_update_pattern_table(ui, 0, ui->video.pattern_pal_index);
            ImGui::Image(ui->video.tex_pattern_tables[0], ImVec2(512, 512));
            // HACK: Why do I need to do this to position correctly the next image ?
            ImGui::SetCursorPos(ImVec2(p.x, p.y + 264));
            _ui_nes_update_pattern_table(ui, 1, ui->video.pattern_pal_index);
            ImGui::Image(ui->video.tex_pattern_tables[1], ImVec2(512, 512));
            ImGui::SetCursorPos(ImVec2(p.x, p.y + 528));
        }
        if (ImGui::CollapsingHeader("Name tables", ImGuiTreeNodeFlags_DefaultOpen)) {
            _ui_nes_update_names_tables(ui);
            ImVec2 screen_pos = ImGui::GetCursorScreenPos();
            ImVec2 mouse_pos = ImGui::GetMousePos();
            const ImVec2 p = ImGui::GetCursorPos();
            ImGui::Image(ui->video.tex_name_tables, ImVec2(512, 512));
            ImGui::SetCursorPos(ImVec2(p.x, p.y + 520));
            int tile_x = (int)(mouse_pos.x - screen_pos.x)>>3;
            int tile_y = (int)(mouse_pos.y - screen_pos.y)>>3;
            if (ImGui::IsItemHovered()) {
                uint8_t table_nr = _ui_nes_table_nr(tile_x, tile_y);
                tile_x %= 32;
                tile_y %= 32;
                uint8_t att = _ui_nes_attributes(ui, tile_x, tile_y, table_nr);
                uint16_t att_addr = _ui_nes_att_address(tile_x, tile_y, table_nr);
                uint8_t pal_index = _ui_nes_pal_index(ui,tile_x, tile_y, table_nr);
                uint16_t pal_addr = _nes_pal_addr(0, pal_index);
                uint8_t pattern = tile_x + (tile_y << 5);
                uint16_t pattern_addr = _ui_nes_tile_address(table_nr) + pattern;
                uint8_t tile_index = _ui_nes_ppu_mem_read(0, pattern_addr, ui);
                uint16_t tile_addr = 0x1000 * table_nr + (tile_index << 4);
                ImGui::SetTooltip("x: %d y: %d\naddr: %x\ntable: %u\natt: %02x\natt_addr: %x\npal_addr: %x\ntile_index: %x\ntile_addr: %x", tile_x, tile_y, pattern_addr, table_nr, att, att_addr, pal_addr, tile_index, tile_addr);
            }
        }
        if (ImGui::CollapsingHeader("Sprites", ImGuiTreeNodeFlags_DefaultOpen)) {
            _ui_nes_update_sprites(ui);
            ImGui::Image(ui->video.tex_sprites, ImVec2(128, 128));
        }
    }
    ImGui::End();
}

void ui_nes_draw(ui_nes_t* ui) {
    CHIPS_ASSERT(ui && ui->nes);
    _ui_nes_draw_menu(ui);
    ui_m6502_draw(&ui->cpu);
    ui_audio_draw(&ui->audio, ui->nes->audio.sample_pos);
    for (int i = 0; i < 4; i++) {
        ui_memedit_draw(&ui->memedit[i]);
        ui_memedit_draw(&ui->ppu_memedit[i]);
        ui_memedit_draw(&ui->sprite_memedit[i]);
        ui_memedit_draw(&ui->oam_memedit[i]);
        ui_dasm_draw(&ui->dasm[i]);
    }
    ui_dbg_draw(&ui->dbg);
    _ui_nes_draw_video(ui);
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
