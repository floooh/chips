#pragma once
/*#
    # ui_z1013.h

    Integrated debugging UI for z1013.h

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
    ui_1013.h both for the declaration and implementation.

    - z1013.h
    - mem.h
    - ui_chip.h
    - ui_util.h
    - ui_z80.h
    - ui_z80pio.h
    - ui_dbg.h
    - ui_dasm.h
    - ui_memedit.h
    - ui_memmap.h

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
typedef void (*ui_z1013_boot_t)(z1013_t* sys, z1013_type_t type);

typedef struct {
    z1013_t* z1013;
    ui_z1013_boot_t boot_cb; /* user-provided callback to reboot to different config */
    ui_dbg_create_texture_t create_texture_cb;      /* texture creation callback for ui_dbg_t */
    ui_dbg_update_texture_t update_texture_cb;      /* texture update callback for ui_dbg_t */
    ui_dbg_destroy_texture_t destroy_texture_cb;    /* texture destruction callback for ui_dbg_t */
    ui_dbg_keydesc_t dbg_keys;          /* user-defined hotkeys for ui_dbg_t */
} ui_z1013_desc_t;

typedef struct {
    z1013_t* z1013;
    ui_z1013_boot_t boot_cb;
    ui_z80_t cpu;
    ui_z80pio_t pio;
    ui_memmap_t memmap;
    ui_memedit_t memedit[4];
    ui_dasm_t dasm[4];
    ui_dbg_t dbg;
} ui_z1013_t;

void ui_z1013_init(ui_z1013_t* ui, const ui_z1013_desc_t* desc);
void ui_z1013_discard(ui_z1013_t* ui);
void ui_z1013_draw(ui_z1013_t* ui, double time_ms);
bool ui_z1013_before_exec(ui_z1013_t* ui);
void ui_z1013_after_exec(ui_z1013_t* ui);

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

static void _ui_z1013_draw_menu(ui_z1013_t* ui, double time_ms) {
    CHIPS_ASSERT(ui && ui->z1013 && ui->boot_cb);
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("System")) {
            if (ImGui::MenuItem("Reset")) {
                z1013_reset(ui->z1013);
                ui_dbg_reset(&ui->dbg);
            }
            if (ImGui::MenuItem("Z1013.01", 0, ui->z1013->type == Z1013_TYPE_01)) {
                ui->boot_cb(ui->z1013, Z1013_TYPE_01);
                ui_dbg_reboot(&ui->dbg);
            }
            if (ImGui::MenuItem("Z1013.16", 0, ui->z1013->type == Z1013_TYPE_16)) {
                ui->boot_cb(ui->z1013, Z1013_TYPE_16);
                ui_dbg_reboot(&ui->dbg);
            }
            if (ImGui::MenuItem("Z1013.64", 0, ui->z1013->type == Z1013_TYPE_64)) {
                ui->boot_cb(ui->z1013, Z1013_TYPE_64);
                ui_dbg_reboot(&ui->dbg);
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Hardware")) {
            ImGui::MenuItem("Memory Map", 0, &ui->memmap.open);
            ImGui::MenuItem("Z80 CPU", 0, &ui->cpu.open);
            ImGui::MenuItem("Z80 PIO", 0, &ui->pio.open);
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

static void _ui_z1013_update_memmap(ui_z1013_t* ui) {
    CHIPS_ASSERT(ui && ui->z1013);
    ui_memmap_reset(&ui->memmap);
    ui_memmap_layer(&ui->memmap, "System");
    if ((Z1013_TYPE_01 == ui->z1013->type) || (Z1013_TYPE_16 == ui->z1013->type)) {
        /* Z1013/01 + /16 memory map */
        ui_memmap_region(&ui->memmap, "RAM", 0x0000, 0x4000, true);
        ui_memmap_region(&ui->memmap, "VIDEO", 0xEC00, 0x0400, true);
        ui_memmap_region(&ui->memmap, "ROM", 0xF000, 0x0800, true);
    }
    else {
        /* Z1013/64 memory map */
        ui_memmap_region(&ui->memmap, "RAM0", 0x0000, 0xEC00, true);
        ui_memmap_region(&ui->memmap, "VIDEO", 0xEC00, 0x0400, true);
        ui_memmap_region(&ui->memmap, "ROM", 0xF000, 0x0800, true);
        ui_memmap_region(&ui->memmap, "RAM1", 0xF800, 0x0800, true);
    }
}

static const ui_chip_pin_t _ui_z1013_cpu_pins[] = {
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

static const ui_chip_pin_t _ui_z1013_pio_pins[] = {
    { "D0",     0,      Z80_D0 },
    { "D1",     1,      Z80_D1 },
    { "D2",     2,      Z80_D2 },
    { "D3",     3,      Z80_D3 },
    { "D4",     4,      Z80_D4 },
    { "D5",     5,      Z80_D5 },
    { "D6",     6,      Z80_D6 },
    { "D7",     7,      Z80_D7 },
    { "CE",     9,      Z80PIO_CE },
    { "BASEL",  10,     Z80PIO_BASEL },
    { "CDSEL",  11,     Z80PIO_CDSEL },
    { "M1",     12,     Z80PIO_M1 },
    { "IORQ",   13,     Z80PIO_IORQ },
    { "RD",     14,     Z80PIO_RD },
    { "INT",    15,     Z80PIO_INT },
    { "ARDY",   20,     Z80PIO_ARDY },
    { "ASTB",   21,     Z80PIO_ASTB },
    { "PA0",    22,     Z80PIO_PA0 },
    { "PA1",    23,     Z80PIO_PA1 },
    { "PA2",    24,     Z80PIO_PA2 },
    { "PA3",    25,     Z80PIO_PA3 },
    { "PA4",    26,     Z80PIO_PA4 },
    { "PA5",    27,     Z80PIO_PA5 },
    { "PA6",    28,     Z80PIO_PA6 },
    { "PA7",    29,     Z80PIO_PA7 },
    { "BRDY",   30,     Z80PIO_ARDY },
    { "BSTB",   31,     Z80PIO_ASTB },
    { "PB0",    32,     Z80PIO_PB0 },
    { "PB1",    33,     Z80PIO_PB1 },
    { "PB2",    34,     Z80PIO_PB2 },
    { "PB3",    35,     Z80PIO_PB3 },
    { "PB4",    36,     Z80PIO_PB4 },
    { "PB5",    37,     Z80PIO_PB5 },
    { "PB6",    38,     Z80PIO_PB6 },
    { "PB7",    39,     Z80PIO_PB7 },
};

static uint8_t _ui_z1013_mem_read(int layer, uint16_t addr, void* user_data) {
    (void)layer;
    CHIPS_ASSERT(user_data);
    z1013_t* z1013 = (z1013_t*) user_data;
    return mem_rd(&z1013->mem, addr);
}

void _ui_z1013_mem_write(int layer, uint16_t addr, uint8_t data, void* user_data) {
    (void)layer;
    CHIPS_ASSERT(user_data);
    z1013_t* z1013 = (z1013_t*) user_data;
    mem_wr(&z1013->mem, addr, data);
}

void ui_z1013_init(ui_z1013_t* ui, const ui_z1013_desc_t* ui_desc) {
    CHIPS_ASSERT(ui && ui_desc);
    CHIPS_ASSERT(ui_desc->z1013);
    CHIPS_ASSERT(ui_desc->boot_cb);
    CHIPS_ASSERT(ui_desc->create_texture_cb && ui_desc->update_texture_cb && ui_desc->destroy_texture_cb);
    ui->z1013 = ui_desc->z1013;
    ui->boot_cb = ui_desc->boot_cb;
    int x = 20, y = 20, dx = 10, dy = 10;
    {
        ui_dbg_desc_t desc = {0};
        desc.title = "CPU Debugger";
        desc.x = x;
        desc.y = y;
        desc.z80 = &ui->z1013->cpu;
        desc.read_cb = _ui_z1013_mem_read;
        desc.create_texture_cb = ui_desc->create_texture_cb;
        desc.update_texture_cb = ui_desc->update_texture_cb;
        desc.destroy_texture_cb = ui_desc->destroy_texture_cb;
        desc.keys = ui_desc->dbg_keys;
        desc.user_data = ui->z1013;
        ui_dbg_init(&ui->dbg, &desc);
    }
    {
        ui_z80_desc_t desc = {0};
        desc.title = "Z80 CPU";
        desc.cpu = &ui->z1013->cpu;
        desc.x = x;
        desc.y = y;
        UI_CHIP_INIT_DESC(&desc.chip_desc, "Z80\nCPU", 36, _ui_z1013_cpu_pins);
        ui_z80_init(&ui->cpu, &desc);
    }
    x += dx; y += dy;
    {
        ui_z80pio_desc_t desc = {0};
        desc.title = "Z80 PIO";
        desc.pio = &ui->z1013->pio;
        desc.x = x;
        desc.y = y;
        UI_CHIP_INIT_DESC(&desc.chip_desc, "Z80\nPIO", 40, _ui_z1013_pio_pins);
        ui_z80pio_init(&ui->pio, &desc);
    }
    x += dx; y += dy;
    {
        ui_memedit_desc_t desc = {0};
        desc.layers[0] = "System";
        desc.read_cb = _ui_z1013_mem_read;
        desc.write_cb = _ui_z1013_mem_write;
        desc.user_data = ui->z1013;
        static const char* titles[] = { "Memory Editor #1", "Memory Editor #2", "Memory Editor #3", "Memory Editor #4" };
        for (int i = 0; i < 4; i++) {
            desc.title = titles[i]; desc.x = x; desc.y = y;
            ui_memedit_init(&ui->memedit[i], &desc);
            x += dx; y += dy;
        }
    }
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
        desc.layers[0] = "System";
        desc.cpu_type = UI_DASM_CPUTYPE_Z80;
        desc.start_addr = 0xF000;
        desc.read_cb = _ui_z1013_mem_read;
        desc.user_data = ui->z1013;
        static const char* titles[4] = { "Disassembler #1", "Disassembler #2", "Disassembler #2", "Dissassembler #3" };
        for (int i = 0; i < 4; i++) {
            desc.title = titles[i]; desc.x = x; desc.y = y;
            ui_dasm_init(&ui->dasm[i], &desc);
            x += dx; y += dy;
        }
    }
}

void ui_z1013_discard(ui_z1013_t* ui) {
    CHIPS_ASSERT(ui && ui->z1013);
    ui->z1013 = 0;
    ui_z80_discard(&ui->cpu);
    ui_z80pio_discard(&ui->pio);
    ui_memmap_discard(&ui->memmap);
    for (int i = 0; i < 4; i++) {
        ui_memedit_discard(&ui->memedit[i]);
        ui_dasm_discard(&ui->dasm[i]);
    }
    ui_dbg_discard(&ui->dbg);
}

void ui_z1013_draw(ui_z1013_t* ui, double time_ms) {
    CHIPS_ASSERT(ui && ui->z1013);
    _ui_z1013_draw_menu(ui, time_ms);
    if (ui->memmap.open) {
        _ui_z1013_update_memmap(ui);
    }
    ui_z80_draw(&ui->cpu);
    ui_z80pio_draw(&ui->pio);
    ui_memmap_draw(&ui->memmap);
    for (int i = 0; i < 4; i++) {
        ui_memedit_draw(&ui->memedit[i]);
        ui_dasm_draw(&ui->dasm[i]);
    }
    ui_dbg_draw(&ui->dbg);
}

bool ui_z1013_before_exec(ui_z1013_t* ui) {
    CHIPS_ASSERT(ui && ui->z1013);
    return ui_dbg_before_exec(&ui->dbg);
}

void ui_z1013_after_exec(ui_z1013_t* ui) {
    CHIPS_ASSERT(ui && ui->z1013);
    ui_dbg_after_exec(&ui->dbg);
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif
#endif /* CHIPS_IMPL */
