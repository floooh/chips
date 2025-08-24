#pragma once
/*#
    # ui_kc85.h

    Integrated debugging UI for kc85.h

    Do this:
    ~~~C
    #define CHIPS_UI_IMPL
    ~~~
    before you include this file in *one* C++ file to create the
    implementation.

    Define the KC85 type to build before including this header (both the
    declaration and implementation):

        CHIPS_KC85_TYPE_2
        CHIPS_KC85_TYPE_3
        CHIPS_KC85_TYPE_4

    Optionally provide the following macros with your own implementation

    ~~~C
    CHIPS_ASSERT(c)
    ~~~
        your own assert macro (default: assert(c))

    Include the following headers (and their depenencies) before including
    ui_kc85.h both for the declaration and implementation.

    - kc85.h
    - mem.h
    - ui_chip.h
    - ui_settings.h
    - ui_util.h
    - ui_z80.h
    - ui_z80ctc.h
    - ui_z80pio.h
    - ui_kc85sys.h
    - ui_audio.h
    - ui_display.h
    - ui_dasm.h
    - ui_dbg.h
    - ui_memedit.h
    - ui_memmap.h
    - ui_snapshot.h

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

#if !(defined(CHIPS_KC85_TYPE_2) || defined(CHIPS_KC85_TYPE_3) || defined(CHIPS_KC85_TYPE_4))
#error "Please define one of CHIPS_KC85_TYPE_2, CHIPS_KC85_TYPE_3 or CHIPS_KC85_TYPE_4 before including kc85.h!"
#endif

#ifdef __cplusplus
extern "C" {
#endif

// general callback type for rebooting to different configs
typedef void (*ui_kc85_boot_t)(kc85_t* sys);

typedef struct {
    kc85_t* kc85;
    ui_kc85_boot_t boot_cb;                 // user-provided callback to reboot
    ui_inject_t inject;
    ui_dbg_texture_callbacks_t dbg_texture; // user-provided texture create/update/destroy callbacks
    ui_dbg_debug_callbacks_t dbg_debug;     // user-provided debugger callbacks
    ui_dbg_keys_desc_t dbg_keys;            // user-defined hotkeys for ui_dbg_t
    ui_snapshot_desc_t snapshot;            // snapshot system creation params
} ui_kc85_desc_t;

typedef struct {
    ui_display_frame_t display;
} ui_kc85_frame_t;

typedef struct {
    kc85_t* kc85;
    ui_kc85_boot_t boot_cb;
    ui_inject_t inject;
    ui_z80_t cpu;
    ui_z80pio_t pio;
    ui_z80ctc_t ctc;
    ui_kc85sys_t sys;
    ui_audio_t audio;
    ui_display_t display;
    ui_memmap_t memmap;
    ui_memedit_t memedit[4];
    ui_dasm_t dasm[4];
    ui_dbg_t dbg;
    ui_snapshot_t snapshot;
} ui_kc85_t;

void ui_kc85_init(ui_kc85_t* ui, const ui_kc85_desc_t* desc);
void ui_kc85_discard(ui_kc85_t* ui);
void ui_kc85_draw(ui_kc85_t* ui, const ui_kc85_frame_t* frame);
chips_debug_t ui_kc85_get_debug(ui_kc85_t* ui);
void ui_kc85_save_settings(ui_kc85_t* ui, ui_settings_t* settings);
void ui_kc85_load_settings(ui_kc85_t* ui, const ui_settings_t* settings);

#ifdef __cplusplus
} /* extern "C" */
#endif

/*-- IMPLEMENTATION (include in C++ source) ----------------------------------*/
#ifdef CHIPS_UI_IMPL
#ifndef __cplusplus
#error "implementation must be compiled as C++"
#endif
#include <string.h> // memset
#ifndef CHIPS_ASSERT
    #include <assert.h>
    #define CHIPS_ASSERT(c) assert(c)
#endif
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-field-initializers"
#endif

static void _ui_kc85_draw_menu(ui_kc85_t* ui) {
    CHIPS_ASSERT(ui && ui->kc85 && ui->boot_cb);
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("System")) {
            ui_snapshot_menus(&ui->snapshot);
            if (ImGui::MenuItem("Reset")) {
                kc85_reset(ui->kc85);
                ui_dbg_reset(&ui->dbg);
            }
            if (ImGui::MenuItem("Cold Boot")) {
                ui->boot_cb(ui->kc85);
                ui_dbg_reboot(&ui->dbg);
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Hardware")) {
            ImGui::MenuItem("Memory Map", 0, &ui->memmap.open);
            ImGui::MenuItem("System State", 0, &ui->sys.open);
            ImGui::MenuItem("Audio Output", 0, &ui->audio.open);
            ImGui::MenuItem("Display", 0, &ui->display.open);
            ImGui::MenuItem("Z80 CPU", 0, &ui->cpu.open);
            ImGui::MenuItem("Z80 PIO", 0, &ui->pio.open);
            ImGui::MenuItem("Z80 CTC", 0, &ui->ctc.open);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Debug")) {
            ImGui::MenuItem("CPU Debugger", 0, &ui->dbg.ui.open);
            ImGui::MenuItem("Breakpoints", 0, &ui->dbg.ui.breakpoints.open);
            ImGui::MenuItem("Stopwatch", 0, &ui->dbg.ui.stopwatch.open);
            ImGui::MenuItem("Execution History", 0, &ui->dbg.ui.history.open);
            ImGui::MenuItem("Memory Heatmap", 0, &ui->dbg.ui.heatmap.open);
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
            ImGui::MenuItem("Scan Commands (TODO)");
            ImGui::EndMenu();
        }
        if (ui->inject.menu_cb) {
            ui->inject.menu_cb();
        }
        ui_util_options_menu();
        ImGui::EndMainMenuBar();
    }
}

static void _ui_kc85_update_memmap(ui_kc85_t* ui) {
    CHIPS_ASSERT(ui && ui->kc85);
    const uint64_t pio_pins = ui->kc85->pio_pins;
    ui_memmap_reset(&ui->memmap);
    #if defined(CHIPS_KC85_TYPE_2)
        /* KC85/2 memory map */
        ui_memmap_layer(&ui->memmap, "System");
            ui_memmap_region(&ui->memmap, "RAM0", 0x0000, 0x4000, 0 != (pio_pins & KC85_PIO_RAM));
            ui_memmap_region(&ui->memmap, "IRM", 0x8000, 0x4000, 0 != (pio_pins & KC85_PIO_IRM));
            ui_memmap_region(&ui->memmap, "CAOS ROM 1", 0xE000, 0x0800, 0 != (pio_pins & KC85_PIO_CAOS_ROM));
            ui_memmap_region(&ui->memmap, "CAOS ROM 2", 0xF000, 0x0800, 0 != (pio_pins & KC85_PIO_CAOS_ROM));
    #elif defined(CHIPS_KC85_TYPE_3)
        /* KC85/3 memory map */
        ui_memmap_layer(&ui->memmap, "System");
            ui_memmap_region(&ui->memmap, "RAM0", 0x0000, 0x4000, 0 != (pio_pins & KC85_PIO_RAM));
            ui_memmap_region(&ui->memmap, "IRM", 0x8000, 0x4000, 0 != (pio_pins & KC85_PIO_IRM));
            ui_memmap_region(&ui->memmap, "BASIC ROM", 0xC000, 0x2000, 0 != (pio_pins & KC85_PIO_BASIC_ROM));
            ui_memmap_region(&ui->memmap, "CAOS ROM", 0xE000, 0x2000, 0 != (pio_pins & KC85_PIO_CAOS_ROM));
    #else
        /* KC85/4 memory map */
        const uint8_t io86  = ui->kc85->io86;
        const uint8_t io84  = ui->kc85->io84;
        ui_memmap_layer(&ui->memmap, "System 0");
            ui_memmap_region(&ui->memmap, "RAM0", 0x0000, 0x4000, 0 != (pio_pins & KC85_PIO_RAM));
            ui_memmap_region(&ui->memmap, "RAM4", 0x4000, 0x4000, 0 != (io86 & KC85_IO86_RAM4));
            ui_memmap_region(&ui->memmap, "IRM0 PIXELS", 0x8000, 0x2800, (0 != (pio_pins & KC85_PIO_IRM)) && (0 == (io84 & 6)));
            ui_memmap_region(&ui->memmap, "IRM0", 0xA800, 0x1800, 0 != (pio_pins & KC85_PIO_IRM));
            ui_memmap_region(&ui->memmap, "CAOS ROM E", 0xE000, 0x2000, 0 != (pio_pins & KC85_PIO_CAOS_ROM));
        ui_memmap_layer(&ui->memmap, "System 1");
            ui_memmap_region(&ui->memmap, "IRM0 COLORS", 0x8000, 0x2800, (0 != (pio_pins & KC85_PIO_IRM)) && (2 == (io84 & 6)));
            ui_memmap_region(&ui->memmap, "CAOS ROM C", 0xC000, 0x1000, 0 != (io86 & KC85_IO86_CAOS_ROM_C));
        ui_memmap_layer(&ui->memmap, "System 2");
            ui_memmap_region(&ui->memmap, "IRM1 PIXELS", 0x8000, 0x2800, (0 != (pio_pins & KC85_PIO_IRM)) && (4 == (io84 & 6)));
            ui_memmap_region(&ui->memmap, "BASIC ROM", 0xC000, 0x2000, 0 != (pio_pins & KC85_PIO_BASIC_ROM));
        ui_memmap_layer(&ui->memmap, "System 3");
            ui_memmap_region(&ui->memmap, "IRM1 COLORS", 0x8000, 0x2800, (0 != (pio_pins & KC85_PIO_IRM)) && (6 == (io84 & 6)));
        ui_memmap_layer(&ui->memmap, "System 4");
            ui_memmap_region(&ui->memmap, "RAM8 BANK0", 0x8000, 0x4000, (0 != (pio_pins & KC85_PIO_RAM8)) && (0 == (io84 & KC85_IO84_SEL_RAM8)));
        ui_memmap_layer(&ui->memmap, "System 5");
            ui_memmap_region(&ui->memmap, "RAM8 BANK1", 0x8000, 0x4000, (0 != (pio_pins & KC85_PIO_RAM8)) && (0 != (io84 & KC85_IO84_SEL_RAM8)));
    #endif
    for (size_t i = 0; i < KC85_EXP_NUM_SLOTS; i++) {
        const uint8_t slot_addr = ui->kc85->exp.slot[i].addr;
        ui_memmap_layer(&ui->memmap, slot_addr == 0x08 ? "Slot 08" : "Slot 0C");
        if (kc85_slot_occupied(ui->kc85, slot_addr)) {
            ui_memmap_region(&ui->memmap,
                kc85_slot_mod_name(ui->kc85, slot_addr),
                kc85_slot_cpu_addr(ui->kc85, slot_addr),
                kc85_slot_mod_size(ui->kc85, slot_addr),
                kc85_slot_ctrl(ui->kc85, slot_addr) & 1);
        }
    }
}

static const ui_chip_pin_t _ui_kc85_cpu_pins[] = {
    { "D0",     0,      Z80_D0 },
    { "D1",     1,      Z80_D1 },
    { "D2",     2,      Z80_D2 },
    { "D3",     3,      Z80_D3 },
    { "D4",     4,      Z80_D4 },
    { "D5",     5,      Z80_D5 },
    { "D6",     6,      Z80_D6 },
    { "D7",     7,      Z80_D7 },
    { "M1",     8,      Z80_M1 },
    { "MREQ",   9,      Z80_MREQ },
    { "IORQ",   10,     Z80_IORQ },
    { "RD",     11,     Z80_RD },
    { "WR",     12,     Z80_WR },
    { "RFSH",   13,     Z80_RFSH },
    { "HALT",   14,     Z80_HALT },
    { "INT",    15,     Z80_INT },
    { "NMI",    16,     Z80_NMI },
    { "WAIT",   17,     Z80_WAIT },
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

static const ui_chip_pin_t _ui_kc85_pio_pins[] = {
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

static const ui_chip_pin_t _ui_kc85_ctc_pins[] = {
    { "D0",     0,      Z80_D0 },
    { "D1",     1,      Z80_D1 },
    { "D2",     2,      Z80_D2 },
    { "D3",     3,      Z80_D3 },
    { "D4",     4,      Z80_D4 },
    { "D5",     5,      Z80_D5 },
    { "D6",     6,      Z80_D6 },
    { "D7",     7,      Z80_D7 },
    { "CE",     9,      Z80CTC_CE },
    { "CS0",    10,     Z80CTC_CS0 },
    { "CS1",    11,     Z80CTC_CS1 },
    { "M1",     12,     Z80CTC_M1 },
    { "IORQ",   13,     Z80CTC_IORQ },
    { "RD",     14,     Z80CTC_RD },
    { "INT",    15,     Z80CTC_INT },
    { "CT0",    16,     Z80CTC_CLKTRG0 },
    { "ZT0",    17,     Z80CTC_ZCTO0 },
    { "CT1",    19,     Z80CTC_CLKTRG1 },
    { "ZT1",    20,     Z80CTC_ZCTO1 },
    { "CT2",    22,     Z80CTC_CLKTRG2 },
    { "ZT2",    23,     Z80CTC_ZCTO2 },
    { "CT3",    25,     Z80CTC_CLKTRG3 },
};

static uint8_t _ui_kc85_mem_read(int layer, uint16_t addr, void* user_data) {
    CHIPS_ASSERT(user_data);
    kc85_t* kc85 = (kc85_t*) user_data;
    if (layer == 0) {
        return mem_rd(&kc85->mem, addr);
    } else if ((layer >= 4) && (layer < 8)) {
        // IRM access
        if ((addr >= 0x8000) && (addr < 0xC000)) {
            return kc85->ram[KC85_IRM0_PAGE + (layer-4)][addr - 0x8000];
        } else {
            return 0xFF;
        }
    } else {
        // Motherboard, SLOT 08, SLOT 0C
        return mem_layer_rd(&kc85->mem, layer-1, addr);
    }
}

static void _ui_kc85_mem_write(int layer, uint16_t addr, uint8_t data, void* user_data) {
    CHIPS_ASSERT(user_data);
    kc85_t* kc85 = (kc85_t*) user_data;
    if (layer == 0) {
        mem_wr(&kc85->mem, addr, data);
    } else if ((layer >= 4) && (layer < 8)) {
        // IRM access
        if ((addr >= 0x8000) && (addr < 0xC000)) {
            kc85->ram[KC85_IRM0_PAGE + (layer-4)][addr - 0x8000] = data;
        }
    } else {
        mem_layer_wr(&kc85->mem, layer-1, addr, data);
    }
}

void ui_kc85_init(ui_kc85_t* ui, const ui_kc85_desc_t* ui_desc) {
    CHIPS_ASSERT(ui && ui_desc);
    CHIPS_ASSERT(ui_desc->kc85);
    CHIPS_ASSERT(ui_desc->boot_cb);
    ui->kc85 = ui_desc->kc85;
    ui->boot_cb = ui_desc->boot_cb;
    ui->inject = ui_desc->inject;
    ui_snapshot_init(&ui->snapshot, &ui_desc->snapshot);
    int x = 20, y = 20, dx = 10, dy = 10;
    {
        ui_dbg_desc_t desc = {0};
        desc.title = "CPU Debugger";
        desc.x = x;
        desc.y = y;
        desc.z80 = &ui->kc85->cpu;
        desc.freq_hz = KC85_FREQUENCY;
        desc.scanline_ticks = KC85_SCANLINE_TICKS;
        desc.frame_ticks = KC85_SCANLINE_TICKS * KC85_NUM_SCANLINES;
        desc.read_cb = _ui_kc85_mem_read;
        desc.texture_cbs = ui_desc->dbg_texture;
        desc.debug_cbs = ui_desc->dbg_debug;
        desc.keys = ui_desc->dbg_keys;
        desc.user_data = ui->kc85;
        ui_dbg_init(&ui->dbg, &desc);
    }
    x += dx; y += dy;
    {
        ui_z80_desc_t desc = {0};
        desc.title = "Z80 CPU";
        desc.cpu = &ui->kc85->cpu;
        desc.x = x;
        desc.y = y;
        UI_CHIP_INIT_DESC(&desc.chip_desc, "Z80\nCPU", 36, _ui_kc85_cpu_pins);
        ui_z80_init(&ui->cpu, &desc);
    }
    x += dx; y += dy;
    {
        ui_z80pio_desc_t desc = {0};
        desc.title = "Z80 PIO";
        desc.pio = &ui->kc85->pio;
        desc.x = x;
        desc.y = y;
        UI_CHIP_INIT_DESC(&desc.chip_desc, "Z80\nPIO", 40, _ui_kc85_pio_pins);
        ui_z80pio_init(&ui->pio, &desc);
    }
    x += dx; y += dy;
    {
        ui_z80ctc_desc_t desc = {0};
        desc.title = "Z80 CTC";
        desc.ctc = &ui->kc85->ctc;
        desc.x = x;
        desc.y = y;
        UI_CHIP_INIT_DESC(&desc.chip_desc, "Z80\nCTC", 32, _ui_kc85_ctc_pins);
        ui_z80ctc_init(&ui->ctc, &desc);
    }
    x += dx; y += dy;
    {
        ui_kc85sys_desc_t desc = {0};
        desc.title = "System State";
        desc.kc85 = ui->kc85;
        desc.x = x;
        desc.y = y;
        ui_kc85sys_init(&ui->sys, &desc);
    }
    x += dx; y += dy;
    {
        ui_audio_desc_t desc = {0};
        desc.title = "Audio Output";
        desc.sample_buffer = ui->kc85->audio.sample_buffer;
        desc.num_samples = ui->kc85->audio.num_samples;
        desc.x = x;
        desc.y = y;
        ui_audio_init(&ui->audio, &desc);
    }
    x += dx; y += dy;
    {
        ui_display_desc_t desc = {0};
        desc.title = "Display";
        desc.x = x;
        desc.y = y;
        ui_display_init(&ui->display, &desc);
    }
    x += dx; y += dy;
    {
        ui_memedit_desc_t desc = {0};
        desc.layers[0] = "CPU Mapped";
        desc.layers[1] = "Motherboard";
        desc.layers[2] = "Slot 08";
        desc.layers[3] = "Slot 0C";
        #if defined(CHIPS_KC85_TYPE_4)
            desc.layers[4] = "IRM 0 Pixels";
            desc.layers[5] = "IRM 0 Colors";
            desc.layers[6] = "IRM 1 Pixels";
            desc.layers[7] = "IRM 1 Colors";
        #endif
        desc.read_cb = _ui_kc85_mem_read;
        desc.write_cb = _ui_kc85_mem_write;
        desc.user_data = ui->kc85;
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
        desc.layers[0] = "CPU Mapped";
        desc.layers[1] = "Motherboard";
        desc.layers[2] = "Slot 08";
        desc.layers[3] = "Slot 0C";
        #if defined(CHIPS_KC85_TYPE_4)
            desc.layers[4] = "IRM 0 Pixels";
            desc.layers[5] = "IRM 0 Colors";
            desc.layers[6] = "IRM 1 Pixels";
            desc.layers[7] = "IRM 1 Colors";
        #endif
        desc.cpu_type = UI_DASM_CPUTYPE_Z80;
        desc.start_addr = 0xF000;
        desc.read_cb = _ui_kc85_mem_read;
        desc.user_data = ui->kc85;
        static const char* titles[4] = { "Disassembler #1", "Disassembler #2", "Disassembler #2", "Dissassembler #3" };
        for (int i = 0; i < 4; i++) {
            desc.title = titles[i]; desc.x = x; desc.y = y;
            ui_dasm_init(&ui->dasm[i], &desc);
            x += dx; y += dy;
        }
    }
}

void ui_kc85_discard(ui_kc85_t* ui) {
    CHIPS_ASSERT(ui && ui->kc85);
    ui->kc85 = 0;
    ui_z80_discard(&ui->cpu);
    ui_z80pio_discard(&ui->pio);
    ui_z80ctc_discard(&ui->ctc);
    ui_kc85sys_discard(&ui->sys);
    ui_audio_discard(&ui->audio);
    ui_display_discard(&ui->display);
    ui_memmap_discard(&ui->memmap);
    for (int i = 0; i < 4; i++) {
        ui_memedit_discard(&ui->memedit[i]);
        ui_dasm_discard(&ui->dasm[i]);
    }
    ui_dbg_discard(&ui->dbg);
}

void ui_kc85_draw(ui_kc85_t* ui, const ui_kc85_frame_t* frame) {
    CHIPS_ASSERT(ui && ui->kc85 && frame);
    _ui_kc85_draw_menu(ui);
    if (ui->memmap.open) {
        _ui_kc85_update_memmap(ui);
    }
    ui_audio_draw(&ui->audio, ui->kc85->audio.sample_pos);
    ui_display_draw(&ui->display, &frame->display);
    ui_z80_draw(&ui->cpu);
    ui_z80pio_draw(&ui->pio);
    ui_z80ctc_draw(&ui->ctc);
    ui_memmap_draw(&ui->memmap);
    ui_kc85sys_draw(&ui->sys);
    for (int i = 0; i < 4; i++) {
        ui_memedit_draw(&ui->memedit[i]);
        ui_dasm_draw(&ui->dasm[i]);
    }
    ui_dbg_draw(&ui->dbg);
}

chips_debug_t ui_kc85_get_debug(ui_kc85_t* ui) {
    chips_debug_t res = {};
    res.callback.func = (chips_debug_func_t)ui_dbg_tick;
    res.callback.user_data = &ui->dbg;
    res.stopped = &ui->dbg.dbg.stopped;
    return res;
}

void ui_kc85_save_settings(ui_kc85_t* ui, ui_settings_t* settings) {
    CHIPS_ASSERT(ui && settings);
    ui_z80_save_settings(&ui->cpu, settings);
    ui_z80pio_save_settings(&ui->pio, settings);
    ui_z80ctc_save_settings(&ui->ctc, settings);
    ui_kc85sys_save_settings(&ui->sys, settings);
    ui_audio_save_settings(&ui->audio, settings);
    ui_display_save_settings(&ui->display, settings);
    ui_memmap_save_settings(&ui->memmap, settings);
    for (int i = 0; i < 4; i++) {
        ui_memedit_save_settings(&ui->memedit[i], settings);
    }
    for (int i = 0; i < 4; i++) {
        ui_dasm_save_settings(&ui->dasm[i], settings);
    }
    ui_dbg_save_settings(&ui->dbg, settings);
}

void ui_kc85_load_settings(ui_kc85_t* ui, const ui_settings_t* settings) {
    CHIPS_ASSERT(ui && settings);
    ui_z80_load_settings(&ui->cpu, settings);
    ui_z80pio_load_settings(&ui->pio, settings);
    ui_z80ctc_load_settings(&ui->ctc, settings);
    ui_kc85sys_load_settings(&ui->sys, settings);
    ui_audio_load_settings(&ui->audio, settings);
    ui_display_load_settings(&ui->display, settings);
    ui_memmap_load_settings(&ui->memmap, settings);
    for (int i = 0; i < 4; i++) {
        ui_memedit_load_settings(&ui->memedit[i], settings);
    }
    for (int i = 0; i < 4; i++) {
        ui_dasm_load_settings(&ui->dasm[i], settings);
    }
    ui_dbg_load_settings(&ui->dbg, settings);
}
#ifdef __clang__
#pragma clang diagnostic pop
#endif
#endif /* CHIPS_UI_IMPL */
