#pragma once
/*#
    # ui_atom.h

    Integrated debugging UI for atom.h

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
    ui_atom.h both for the declaration and implementation.

    - atom.h
    - mem.h
    - ui_chip.h
    - ui_util.h
    - ui_m6502.h
    - ui_mc6847.h
    - ui_i8255.h
    - ui_m6522.h
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

/* reboot callback */
typedef void (*ui_atom_boot_cb)(atom_t* sys);

typedef struct {
    atom_t* atom;
    ui_atom_boot_cb boot_cb;
    ui_dbg_create_texture_t create_texture_cb;      /* texture creation callback for ui_dbg_t */
    ui_dbg_update_texture_t update_texture_cb;      /* texture update callback for ui_dbg_t */
    ui_dbg_destroy_texture_t destroy_texture_cb;    /* texture destruction callback for ui_dbg_t */
    ui_dbg_keydesc_t dbg_keys;          /* user-defined hotkeys for ui_dbg_t */
} ui_atom_desc_t;

typedef struct {
    atom_t* atom;
    ui_atom_boot_cb boot_cb;
    ui_m6502_t cpu;
    ui_i8255_t ppi;
    ui_m6522_t via;
    ui_mc6847_t vdg;
    ui_audio_t audio;
    ui_kbd_t kbd;
    ui_memmap_t memmap;
    ui_memedit_t memedit[4];
    ui_dasm_t dasm[4];
    ui_dbg_t dbg;
} ui_atom_t;

void ui_atom_init(ui_atom_t* ui, const ui_atom_desc_t* desc);
void ui_atom_discard(ui_atom_t* ui);
void ui_atom_draw(ui_atom_t* ui, double time_ms);
void ui_atom_exec(ui_atom_t* ui, uint32_t frame_time_us);

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

static void _ui_atom_draw_menu(ui_atom_t* ui, double time_ms) {
    CHIPS_ASSERT(ui && ui->atom && ui->boot_cb);
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("System")) {
            if (ImGui::MenuItem("Reset")) {
                atom_reset(ui->atom);
                ui_dbg_reset(&ui->dbg);
            }
            if (ImGui::MenuItem("Cold Boot")) {
                ui->boot_cb(ui->atom);
                ui_dbg_reboot(&ui->dbg);
            }
            if (ImGui::BeginMenu("Joystick")) {
                if (ImGui::MenuItem("None", 0, (ui->atom->joystick_type == ATOM_JOYSTICKTYPE_NONE))) {
                    ui->atom->joystick_type = ATOM_JOYSTICKTYPE_NONE;
                }
                if (ImGui::MenuItem("MMC", 0, (ui->atom->joystick_type == ATOM_JOYSTICKTYPE_MMC))) {
                    ui->atom->joystick_type = ATOM_JOYSTICKTYPE_MMC;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Hardware")) {
            ImGui::MenuItem("Memory Map", 0, &ui->memmap.open);
            ImGui::MenuItem("Keyboard Matrix", 0, &ui->kbd.open);
            ImGui::MenuItem("Audio Output", 0, &ui->audio.open);
            ImGui::MenuItem("MOS 6502 (CPU)", 0, &ui->cpu.open);
            ImGui::MenuItem("MOS 6522 (VIA)", 0, &ui->via.open);
            ImGui::MenuItem("MC6847 (VDG)", 0, &ui->vdg.open);
            ImGui::MenuItem("i8255 (PPI)", 0, &ui->ppi.open);
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

static uint8_t _ui_atom_mem_read(int layer, uint16_t addr, void* user_data) {
    CHIPS_ASSERT(user_data);
    (void)layer;
    atom_t* atom = (atom_t*) user_data;
    return mem_rd(&atom->mem, addr);
}

static void _ui_atom_mem_write(int layer, uint16_t addr, uint8_t data, void* user_data) {
    CHIPS_ASSERT(user_data);
    (void)layer;
    atom_t* atom = (atom_t*) user_data;
    mem_wr(&atom->mem, addr, data);
}

static const ui_chip_pin_t _ui_atom_cpu_pins[] = {
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

static const ui_chip_pin_t _ui_atom_ppi_pins[] = {
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

static const ui_chip_pin_t _ui_atom_via_pins[] = {
    { "D0",     0,      M6522_D0 },
    { "D1",     1,      M6522_D1 },
    { "D2",     2,      M6522_D2 },
    { "D3",     3,      M6522_D3 },
    { "D4",     4,      M6522_D4 },
    { "D5",     5,      M6522_D5 },
    { "D6",     6,      M6522_D6 },
    { "D7",     7,      M6522_D7 },
    { "RS0",    9,      M6522_RS0 },
    { "RS1",    10,     M6522_RS1 },
    { "RS2",    11,     M6522_RS2 },
    { "RS3",    12,     M6522_RS3 },
    { "RW",     14,     M6522_RW },
    { "CS1",    15,     M6522_CS1 },
    { "CS2",    16,     M6522_CS2 },
    { "PA0",    20,     M6522_PA0 },
    { "PA1",    21,     M6522_PA1 },
    { "PA2",    22,     M6522_PA2 },
    { "PA3",    23,     M6522_PA3 },
    { "PA4",    24,     M6522_PA4 },
    { "PA5",    25,     M6522_PA5 },
    { "PA6",    26,     M6522_PA6 },
    { "PA7",    27,     M6522_PA7 },
    { "CA1",    28,     M6522_CA1 },
    { "CA2",    29,     M6522_CA2 },
    { "PB0",    30,     M6522_PB0 },
    { "PB1",    31,     M6522_PB1 },
    { "PB2",    32,     M6522_PB2 },
    { "PB3",    33,     M6522_PB3 },
    { "PB4",    34,     M6522_PB4 },
    { "PB5",    35,     M6522_PB5 },
    { "PB6",    36,     M6522_PB6 },
    { "PB7",    37,     M6522_PB7 },
    { "CB1",    38,     M6522_CB1 },
    { "CB2",    39,     M6522_CB2 },
};

static const ui_chip_pin_t _ui_atom_vdg_pins[] = {
    { "D0",     0,      MC6847_D0 },
    { "D1",     1,      MC6847_D1 },
    { "D2",     2,      MC6847_D2 },
    { "D3",     3,      MC6847_D3 },
    { "D4",     4,      MC6847_D4 },
    { "D5",     5,      MC6847_D5 },
    { "D6",     6,      MC6847_D6 },
    { "D7",     7,      MC6847_D7 },
    { "A/G",    9,      MC6847_AG },
    { "A/S",    10,     MC6847_AS },
    { "I/X",    11,     MC6847_INTEXT },
    { "INV",    12,     MC6847_INV },
    { "CSS",    13,     MC6847_CSS },
    { "GM0",    14,     MC6847_GM0 },
    { "GM1",    15,     MC6847_GM0 },
    { "GM2",    16,     MC6847_GM0 },
    { "GM3",    17,     MC6847_GM0 },
    { "A0",     18,     MC6847_A0 },
    { "A1",     19,     MC6847_A1 },
    { "A2",     20,     MC6847_A2 },
    { "A3",     21,     MC6847_A3 },
    { "A4",     22,     MC6847_A4 },
    { "A5",     23,     MC6847_A5 },
    { "A6",     24,     MC6847_A6 },
    { "A7",     25,     MC6847_A7 },
    { "A8",     26,     MC6847_A8 },
    { "A9",     27,     MC6847_A9 },
    { "A10",    28,     MC6847_A10 },
    { "A11",    29,     MC6847_A11 },
    { "A12",    30,     MC6847_A12 },
    { "FS",     32,     MC6847_FS },
    { "HS",     33,     MC6847_HS },
    { "RP",     34,     MC6847_RP }
};

void ui_atom_init(ui_atom_t* ui, const ui_atom_desc_t* ui_desc) {
    CHIPS_ASSERT(ui && ui_desc);
    CHIPS_ASSERT(ui_desc->atom);
    CHIPS_ASSERT(ui_desc->boot_cb);
    ui->atom = ui_desc->atom;
    ui->boot_cb = ui_desc->boot_cb;
    int x = 20, y = 20, dx = 10, dy = 10;
    {
        ui_dbg_desc_t desc = {0};
        desc.title = "CPU Debugger";
        desc.x = x;
        desc.y = y;
        desc.m6502 = &ui->atom->cpu;
        desc.read_cb = _ui_atom_mem_read;
        desc.create_texture_cb = ui_desc->create_texture_cb;
        desc.update_texture_cb = ui_desc->update_texture_cb;
        desc.destroy_texture_cb = ui_desc->destroy_texture_cb;
        desc.keys = ui_desc->dbg_keys;
        desc.user_data = ui->atom;
        ui_dbg_init(&ui->dbg, &desc);
    }
    x += dx; y += dy;
    {
        ui_m6502_desc_t desc = {0};
        desc.title = "MOS 6502";
        desc.cpu = &ui->atom->cpu;
        desc.x = x;
        desc.y = y;
        UI_CHIP_INIT_DESC(&desc.chip_desc, "6502", 32, _ui_atom_cpu_pins);
        ui_m6502_init(&ui->cpu, &desc);
    }
    x += dx; y += dy;
    {
        ui_m6522_desc_t desc = {0};
        desc.title = "MOS 6522";
        desc.via = &ui->atom->via;
        desc.regs_base = 0xB800;
        desc.x = x;
        desc.y = y;
        UI_CHIP_INIT_DESC(&desc.chip_desc, "6522", 40, _ui_atom_via_pins);
        ui_m6522_init(&ui->via, &desc);
    }
    x += dx; y += dy;
    {
        ui_mc6847_desc_t desc = {0};
        desc.title = "MC6847";
        desc.mc6847 = &ui->atom->vdg;
        desc.x = x;
        desc.y = y;
        UI_CHIP_INIT_DESC(&desc.chip_desc, "6847", 36, _ui_atom_vdg_pins);
        ui_mc6847_init(&ui->vdg, &desc);
    }
    x += dx; y += dy;
    {
        ui_i8255_desc_t desc = {0};
        desc.title = "i8255";
        desc.i8255 = &ui->atom->ppi;
        desc.x = x;
        desc.y = y;
        UI_CHIP_INIT_DESC(&desc.chip_desc, "i8255", 40, _ui_atom_ppi_pins);
        ui_i8255_init(&ui->ppi, &desc);
    }
    x += dx; y += dy;
    {
        ui_audio_desc_t desc = {0};
        desc.title = "Audio Output";
        desc.sample_buffer = ui->atom->sample_buffer;
        desc.num_samples = ui->atom->num_samples;
        desc.x = x;
        desc.y = y;
        ui_audio_init(&ui->audio, &desc);
    }
    x += dx; y += dy;
    {
        ui_kbd_desc_t desc = {0};
        desc.title = "Keyboard Matrix";
        desc.kbd = &ui->atom->kbd;
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
        desc.layers[0] = "System";
        desc.read_cb = _ui_atom_mem_read;
        desc.write_cb = _ui_atom_mem_write;
        desc.user_data = ui->atom;
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
        /* the memory map is static */
        ui_memmap_layer(&ui->memmap, "System");
        ui_memmap_region(&ui->memmap, "RAM", 0x0000, 0x3000, true);
        ui_memmap_region(&ui->memmap, "EXT RAM", 0x3000, 0x5000, true);
        ui_memmap_region(&ui->memmap, "VIDEO RAM", 0x8000, 0x2000, true);
        ui_memmap_region(&ui->memmap, "IO AREA", 0xB000, 0x1000, true);
        ui_memmap_region(&ui->memmap, "BASIC ROM 0", 0xC000, 0x1000, true);
        ui_memmap_region(&ui->memmap, "FP ROM", 0xD000, 0x1000, true);
        ui_memmap_region(&ui->memmap, "DOS ROM", 0xE000, 0x1000, true);
        ui_memmap_region(&ui->memmap, "BASIC ROM 1", 0xF000, 0x1000, true);
    }
    x += dx; y += dy;
    {
        ui_dasm_desc_t desc = {0};
        desc.layers[0] = "System";
        desc.cpu_type = UI_DASM_CPUTYPE_M6502;
        desc.start_addr = mem_rd16(&ui->atom->mem, 0xFFFC);
        desc.read_cb = _ui_atom_mem_read;
        desc.user_data = ui->atom;
        static const char* titles[4] = { "Disassembler #1", "Disassembler #2", "Disassembler #2", "Dissassembler #3" };
        for (int i = 0; i < 4; i++) {
            desc.title = titles[i]; desc.x = x; desc.y = y;
            ui_dasm_init(&ui->dasm[i], &desc);
            x += dx; y += dy;
        }
    }
}

void ui_atom_discard(ui_atom_t* ui) {
    CHIPS_ASSERT(ui && ui->atom);
    ui->atom = 0;
    ui_m6502_discard(&ui->cpu);
    ui_m6522_discard(&ui->via);
    ui_mc6847_discard(&ui->vdg);
    ui_i8255_discard(&ui->ppi);
    ui_kbd_discard(&ui->kbd);
    ui_audio_discard(&ui->audio);
    ui_memmap_discard(&ui->memmap);
    for (int i = 0; i < 4; i++) {
        ui_memedit_discard(&ui->memedit[i]);
        ui_dasm_discard(&ui->dasm[i]);
    }
    ui_dbg_discard(&ui->dbg);
}

void ui_atom_draw(ui_atom_t* ui, double time_ms) {
    CHIPS_ASSERT(ui && ui->atom);
    _ui_atom_draw_menu(ui, time_ms);
    ui_audio_draw(&ui->audio, ui->atom->sample_pos);
    ui_kbd_draw(&ui->kbd);
    ui_m6502_draw(&ui->cpu);
    ui_m6522_draw(&ui->via);
    ui_mc6847_draw(&ui->vdg);
    ui_i8255_draw(&ui->ppi);
    ui_memmap_draw(&ui->memmap);
    for (int i = 0; i < 4; i++) {
        ui_memedit_draw(&ui->memedit[i]);
        ui_dasm_draw(&ui->dasm[i]);
    }
    ui_dbg_draw(&ui->dbg);
}

void ui_atom_exec(ui_atom_t* ui, uint32_t frame_time_us) {
    CHIPS_ASSERT(ui && ui->atom);
    uint32_t ticks_to_run = clk_us_to_ticks(ATOM_FREQUENCY, frame_time_us);
    atom_t* atom = ui->atom;
    for (uint32_t i = 0; (i < ticks_to_run) && (!ui->dbg.dbg.stopped); i++) {
        atom_tick(ui->atom);
        ui_dbg_tick(&ui->dbg, atom->pins);
    }
    kbd_update(&ui->atom->kbd, frame_time_us);
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif
#endif /* CHIPS_IMPL */
