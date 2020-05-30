#pragma once
/*#
    # ui_vic20.h

    Integrated debugging UI for vic20.h

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
    ui_c64.h both for the declaration and implementation.

    - vic20.h
    - c1530.h
    - mem.h
    - ui_c1530.h
    - ui_chip.h
    - ui_util.h
    - ui_m6502.h
    - ui_m6522.h
    - ui_m6561.h
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
typedef void (*ui_vic20_boot_cb)(vic20_t* sys);

/* setup params for ui_c64_init() */
typedef struct {
    vic20_t* vic20;             /* pointer to vic20_t instance to track */
    ui_vic20_boot_cb boot_cb;   /* reboot callback function */
    ui_dbg_create_texture_t create_texture_cb;      /* texture creation callback for ui_dbg_t */
    ui_dbg_update_texture_t update_texture_cb;      /* texture update callback for ui_dbg_t */
    ui_dbg_destroy_texture_t destroy_texture_cb;    /* texture destruction callback for ui_dbg_t */
    ui_dbg_keydesc_t dbg_keys;          /* user-defined hotkeys for ui_dbg_t */
} ui_vic20_desc_t;

typedef struct {
    vic20_t* vic20;
    int dbg_scanline;
    ui_c1530_t c1530;
    ui_vic20_boot_cb boot_cb;
    ui_m6502_t cpu;
    ui_m6522_t via[2];
    ui_m6561_t vic;
    ui_audio_t audio;
    ui_kbd_t kbd;
    ui_memmap_t memmap;
    ui_memedit_t memedit[4];
    ui_dasm_t dasm[4];
    ui_dbg_t dbg;
    bool system_window_open;
} ui_vic20_t;

void ui_vic20_init(ui_vic20_t* ui, const ui_vic20_desc_t* desc);
void ui_vic20_discard(ui_vic20_t* ui);
void ui_vic20_draw(ui_vic20_t* ui, double time_ms);
void ui_vic20_exec(ui_vic20_t* ui, uint32_t frame_time_us);

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

static void _ui_vic20_draw_menu(ui_vic20_t* ui, double time_ms) {
    CHIPS_ASSERT(ui && ui->vic20 && ui->boot_cb);
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("System")) {
            if (ImGui::MenuItem("Reset")) {
                vic20_reset(ui->vic20);
                ui_dbg_reset(&ui->dbg);
            }
            if (ImGui::MenuItem("Remove Cartridge")) {
                vic20_remove_rom_cartridge(ui->vic20);
            }
            if (ImGui::MenuItem("Cold Boot")) {
                ui->boot_cb(ui->vic20);
                ui_dbg_reboot(&ui->dbg);
            }

            if (ImGui::BeginMenu("Joystick")) {
                if (ImGui::MenuItem("None", 0, ui->vic20->joystick_type == VIC20_JOYSTICKTYPE_NONE)) {
                    ui->vic20->joystick_type = VIC20_JOYSTICKTYPE_NONE;
                }
                if (ImGui::MenuItem("Digital", 0, ui->vic20->joystick_type == VIC20_JOYSTICKTYPE_DIGITAL)) {
                    ui->vic20->joystick_type = VIC20_JOYSTICKTYPE_DIGITAL;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Hardware")) {
            ImGui::MenuItem("System", 0, &ui->system_window_open);
            ImGui::MenuItem("Memory Map", 0, &ui->memmap.open);
            ImGui::MenuItem("Keyboard Matrix", 0, &ui->kbd.open);
            ImGui::MenuItem("Audio Output", 0, &ui->audio.open);
            ImGui::MenuItem("MOS 6502 (CPU)", 0, &ui->cpu.open);
            ImGui::MenuItem("MOS 6522 #1 (VIA)", 0, &ui->via[0].open);
            ImGui::MenuItem("MOS 6522 #2 (VIA)", 0, &ui->via[1].open);
            ImGui::MenuItem("MOS 6561 (VIC-I)", 0, &ui->vic.open);
            if (ui->c1530.valid) {
                ImGui::MenuItem("C1530 (Datassette)", 0, &ui->c1530.open);
            }
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

/* keep disassembler layer at the start */
#define _UI_VIC20_MEMLAYER_CPU    (0)     /* CPU visible mapping */
#define _UI_VIC20_MEMLAYER_VIC    (1)     /* VIC visible mapping */
#define _UI_VIC20_MEMLAYER_COLOR  (2)     /* special static color RAM */
#define _UI_VIC20_CODELAYER_NUM   (1)     /* number of valid layers for disassembler */
#define _UI_VIC20_MEMLAYER_NUM    (3)

static const char* _ui_vic20_memlayer_names[_UI_VIC20_MEMLAYER_NUM] = {
    "CPU Mapped", "VIC Mapped", "Color RAM"
};

static uint8_t _ui_vic20_mem_read(int layer, uint16_t addr, void* user_data) {
    CHIPS_ASSERT(user_data);
    ui_vic20_t* ui = (ui_vic20_t*) user_data;
    vic20_t* vic20 = ui->vic20;
    switch (layer) {
        case _UI_VIC20_MEMLAYER_CPU:
            return mem_rd(&vic20->mem_cpu, addr);
        case _UI_VIC20_MEMLAYER_VIC:
            return mem_rd(&vic20->mem_vic, addr);
        case _UI_VIC20_MEMLAYER_COLOR:
            /* static COLOR RAM */
            return vic20->color_ram[addr & 0x3FF];
        default:
            return 0xFF;
    }
}

static void _ui_vic20_mem_write(int layer, uint16_t addr, uint8_t data, void* user_data) {
    CHIPS_ASSERT(user_data);
    ui_vic20_t* ui = (ui_vic20_t*) user_data;
    vic20_t* vic20 = ui->vic20;
    switch (layer) {
        case _UI_VIC20_MEMLAYER_CPU:
            mem_wr(&vic20->mem_cpu, addr, data);
            break;
        case _UI_VIC20_MEMLAYER_VIC:
            mem_wr(&vic20->mem_vic, addr, data);
            break;
        case _UI_VIC20_MEMLAYER_COLOR:
            /* static COLOR RAM */
            vic20->color_ram[addr & 0x3FF] = data;
            break;
    }
}

static void _ui_vic20_update_memmap(ui_vic20_t* ui) {
    CHIPS_ASSERT(ui && ui->vic20);
    const vic20_memory_config_t cfg = ui->vic20->mem_config;
    ui_memmap_reset(&ui->memmap);
    ui_memmap_layer(&ui->memmap, "SYS");
        ui_memmap_region(&ui->memmap, "RAM0",  0x0000, 0x0400, true);
        ui_memmap_region(&ui->memmap, "RAM3K", 0x0400, 0x0C00, cfg == VIC20_MEMCONFIG_MAX);
        ui_memmap_region(&ui->memmap, "RAM1",  0x1000, 0x1000, true);
        ui_memmap_region(&ui->memmap, "EXP1",  0x2000, 0x2000, cfg >= VIC20_MEMCONFIG_8K);
        ui_memmap_region(&ui->memmap, "EXP2",  0x4000, 0x2000, cfg >= VIC20_MEMCONFIG_16K);
        ui_memmap_region(&ui->memmap, "EXP3",  0x6000, 0x2000, cfg >= VIC20_MEMCONFIG_24K);
        ui_memmap_region(&ui->memmap, "CHAR",   0x8000, 0x1000, true);
        ui_memmap_region(&ui->memmap, "IO",     0x9000, 0x0200, true);
        // FIXME: color ram at variable address
        ui_memmap_region(&ui->memmap, "COLOR",  0x9400, 0x0800, true);
        ui_memmap_region(&ui->memmap, "EXP4",   0xA000, 0x2000, cfg >= VIC20_MEMCONFIG_32K);
        ui_memmap_region(&ui->memmap, "BASIC",  0xC000, 0x2000, true);
        ui_memmap_region(&ui->memmap, "KERNAL", 0xE000, 0x2000, true);
}

static int _ui_vic20_eval_bp(ui_dbg_t* dbg_win, uint16_t pc, int ticks, uint64_t pins, void* user_data) {
    (void)pc; (void)ticks; (void)pins;
    CHIPS_ASSERT(user_data);
    ui_vic20_t* ui = (ui_vic20_t*) user_data;
    vic20_t* vic20 = ui->vic20;
    int scanline = vic20->vic.rs.v_count;
    int trap_id = 0;
    for (int i = 0; (i < dbg_win->dbg.num_breakpoints) && (trap_id == 0); i++) {
        const ui_dbg_breakpoint_t* bp = &dbg_win->dbg.breakpoints[i];
        if (bp->enabled) {
            switch (bp->type) {
                /* scanline number */
                case UI_DBG_BREAKTYPE_USER+0:
                    if ((ui->dbg_scanline != scanline) && (scanline == bp->val)) {
                        trap_id = UI_DBG_BP_BASE_TRAPID + i;
                    }
                    break;
                /* next scanline */
                case UI_DBG_BREAKTYPE_USER+1:
                    if (ui->dbg_scanline != scanline) {
                        trap_id = UI_DBG_BP_BASE_TRAPID + i;
                    }
                    break;
                /* next frame */
                case UI_DBG_BREAKTYPE_USER+2:
                    if ((ui->dbg_scanline != scanline) && (scanline == 0)) {
                        trap_id = UI_DBG_BP_BASE_TRAPID + i;
                    }
                    break;
            }
        }
    }
    ui->dbg_scanline = scanline;
    return trap_id;
}

static const ui_chip_pin_t _ui_vic20_cpu_pins[] = {
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
    { "RDY",    11,     M6502_RDY },
    { "IRQ",    12,     M6502_IRQ },
    { "NMI",    13,     M6502_NMI },
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

// FIXME
static const ui_chip_pin_t _ui_vic20_via_pins[] = {
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
    { "IRQ",    17,     M6522_IRQ },
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

static const ui_chip_pin_t _ui_vic20_vic_pins[] = {
    { "DB0",    0,      M6561_D0 },
    { "DB1",    1,      M6561_D1 },
    { "DB2",    2,      M6561_D2 },
    { "DB3",    3,      M6561_D3 },
    { "DB4",    4,      M6561_D4 },
    { "DB5",    5,      M6561_D5 },
    { "DB6",    6,      M6561_D6 },
    { "DB7",    7,      M6561_D7 },
    { "RW",     9,      M6561_RW },
    { "A0",     14,     M6561_A0 },
    { "A1",     15,     M6561_A1 },
    { "A2",     16,     M6561_A2 },
    { "A3",     17,     M6561_A3 },
    { "A4",     18,     M6561_A4 },
    { "A5",     19,     M6561_A5 },
    { "A6",     20,     M6561_A6 },
    { "A7",     21,     M6561_A7 },
    { "A8",     22,     M6561_A8 },
    { "A9",     23,     M6561_A9 },
    { "A10",    24,     M6561_A10 },
    { "A11",    25,     M6561_A11 },
    { "A12",    26,     M6561_A12 },
    { "A13",    27,     M6561_A13 }
};

void ui_vic20_init(ui_vic20_t* ui, const ui_vic20_desc_t* ui_desc) {
    CHIPS_ASSERT(ui && ui_desc);
    CHIPS_ASSERT(ui_desc->vic20);
    CHIPS_ASSERT(ui_desc->boot_cb);
    ui->vic20 = ui_desc->vic20;
    ui->boot_cb = ui_desc->boot_cb;
    int x = 20, y = 20, dx = 10, dy = 10;
    {
        ui_dbg_desc_t desc = {0};
        desc.title = "CPU Debugger";
        desc.x = x;
        desc.y = y;
        desc.m6502 = &ui->vic20->cpu;
        desc.read_cb = _ui_vic20_mem_read;
        desc.break_cb = _ui_vic20_eval_bp;
        desc.create_texture_cb = ui_desc->create_texture_cb;
        desc.update_texture_cb = ui_desc->update_texture_cb;
        desc.destroy_texture_cb = ui_desc->destroy_texture_cb;
        desc.keys = ui_desc->dbg_keys;
        desc.user_data = ui;
        /* custom breakpoint types */
        desc.user_breaktypes[0].label = "Scanline at";
        desc.user_breaktypes[0].show_val16 = true;
        desc.user_breaktypes[1].label = "Next Scanline";
        desc.user_breaktypes[2].label = "Next Frame";
        ui_dbg_init(&ui->dbg, &desc);
    }
    if (ui->vic20->c1530.valid) {
        x += dx; y += dy;
        ui_c1530_desc_t desc = {0};
        desc.title = "C1530 Datassette";
        desc.x = x;
        desc.y = y;
        desc.c1530 = &ui->vic20->c1530;
        ui_c1530_init(&ui->c1530, &desc);
    }
    x += dx; y += dy;
    {
        ui_m6502_desc_t desc = {0};
        desc.title = "MOS 6502";
        desc.cpu = &ui->vic20->cpu;
        desc.x = x;
        desc.y = y;
        desc.h = 390;
        UI_CHIP_INIT_DESC(&desc.chip_desc, "6502", 32, _ui_vic20_cpu_pins);
        ui_m6502_init(&ui->cpu, &desc);
    }
    x += dx; y += dy;
    {
        ui_m6522_desc_t desc = {0};
        desc.title = "MOS 6522 #1 (VIA)";
        desc.via = &ui->vic20->via_1;
        desc.regs_base = 0x9110;
        desc.x = x;
        desc.y = y;
        UI_CHIP_INIT_DESC(&desc.chip_desc, "6522", 40, _ui_vic20_via_pins);
        ui_m6522_init(&ui->via[0], &desc);
        x += dx; y += dy;
        desc.title = "MOS 6522 #2 (VIA)";
        desc.via = &ui->vic20->via_2;
        desc.regs_base = 0x9120;
        desc.x = x;
        desc.y = y;
        ui_m6522_init(&ui->via[1], &desc);
    }
    x += dx; y += dy;
    {
        ui_m6561_desc_t desc = {0};
        desc.title = "MOS 6561 (VIC-I)";
        desc.vic = &ui->vic20->vic;
        desc.regs_base = 0x9000;
        desc.x = x;
        desc.y = y;
        UI_CHIP_INIT_DESC(&desc.chip_desc, "6561", 28, _ui_vic20_vic_pins);
        ui_m6561_init(&ui->vic, &desc);
    }
    x += dx; y += dy;
    {
        ui_audio_desc_t desc = {0};
        desc.title = "Audio Output";
        desc.sample_buffer = ui->vic20->sample_buffer;
        desc.num_samples = ui->vic20->num_samples;
        desc.x = x;
        desc.y = y;
        ui_audio_init(&ui->audio, &desc);
    }
    x += dx; y += dy;
    {
        ui_kbd_desc_t desc = {0};
        desc.title = "Keyboard Matrix";
        desc.kbd = &ui->vic20->kbd;
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
        for (int i = 0; i < _UI_VIC20_MEMLAYER_NUM; i++) {
            desc.layers[i] = _ui_vic20_memlayer_names[i];
        }
        desc.read_cb = _ui_vic20_mem_read;
        desc.write_cb = _ui_vic20_mem_write;
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
        for (int i = 0; i < _UI_VIC20_CODELAYER_NUM; i++) {
            desc.layers[i] = _ui_vic20_memlayer_names[i];
        }
        desc.cpu_type = UI_DASM_CPUTYPE_M6502;
        desc.start_addr = mem_rd16(&ui->vic20->mem_cpu, 0xFFFC);
        desc.read_cb = _ui_vic20_mem_read;
        desc.user_data = ui;
        static const char* titles[4] = { "Disassembler #1", "Disassembler #2", "Disassembler #2", "Dissassembler #3" };
        for (int i = 0; i < 4; i++) {
            desc.title = titles[i]; desc.x = x; desc.y = y;
            ui_dasm_init(&ui->dasm[i], &desc);
            x += dx; y += dy;
        }
    }
}

void ui_vic20_discard(ui_vic20_t* ui) {
    CHIPS_ASSERT(ui && ui->vic20);
    ui->vic20 = 0;
    if (ui->c1530.valid) {
        ui_c1530_discard(&ui->c1530);
    }
    ui_m6502_discard(&ui->cpu);
    ui_m6522_discard(&ui->via[0]);
    ui_m6522_discard(&ui->via[1]);
    ui_m6561_discard(&ui->vic);
    ui_kbd_discard(&ui->kbd);
    ui_audio_discard(&ui->audio);
    ui_memmap_discard(&ui->memmap);
    for (int i = 0; i < 4; i++) {
        ui_memedit_discard(&ui->memedit[i]);
        ui_dasm_discard(&ui->dasm[i]);
    }
    ui_dbg_discard(&ui->dbg);
}

void ui_vic20_draw_system(ui_vic20_t* ui) {
    if (!ui->system_window_open) {
        return;
    }
    vic20_t* sys = ui->vic20;
    ImGui::SetNextWindowSize({ 200, 250}, ImGuiCond_Once);
    if (ImGui::Begin("VIC-20 System", &ui->system_window_open)) {
        const char* mem_config = "???";
        switch (sys->mem_config) {
            case VIC20_MEMCONFIG_STANDARD:  mem_config = "standard"; break;
            case VIC20_MEMCONFIG_8K:        mem_config = "+8K RAM"; break;
            case VIC20_MEMCONFIG_16K:       mem_config = "+16K RAM"; break;
            case VIC20_MEMCONFIG_24K:       mem_config = "+24K RAM"; break;
            case VIC20_MEMCONFIG_32K:       mem_config = "+32K RAM"; break;
            case VIC20_MEMCONFIG_MAX:       mem_config = "MAX RAM"; break;
        }
        ImGui::Text("Memory Config: %s", mem_config);
        if (ImGui::CollapsingHeader("Cassette Port", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("MOTOR: %d", (sys->cas_port & VIC20_CASPORT_MOTOR) ? 1:0);
            ImGui::Text("WRITE: %d", (sys->cas_port & VIC20_CASPORT_WRITE) ? 1:0);
            ImGui::Text("READ:  %d", (sys->cas_port & VIC20_CASPORT_READ) ? 1:0);
            ImGui::Text("SENSE: %d", (sys->cas_port & VIC20_CASPORT_SENSE) ? 1:0);
        }
        if (ImGui::CollapsingHeader("IEC Port", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("RESET: %d", (sys->iec_port & VIC20_IECPORT_RESET) ? 1:0);
            ImGui::Text("SRQIN: %d", (sys->iec_port & VIC20_IECPORT_SRQIN) ? 1:0);
            ImGui::Text("DATA:  %d", (sys->iec_port & VIC20_IECPORT_DATA) ? 1:0);
            ImGui::Text("CLK:   %d", (sys->iec_port & VIC20_IECPORT_CLK) ? 1:0);
            ImGui::Text("ATN:   %d", (sys->iec_port & VIC20_IECPORT_ATN) ? 1:0);
        }
    }
    ImGui::End();
}

void ui_vic20_draw(ui_vic20_t* ui, double time_ms) {
    CHIPS_ASSERT(ui && ui->vic20);
    _ui_vic20_draw_menu(ui, time_ms);
    if (ui->memmap.open) {
        _ui_vic20_update_memmap(ui);
    }
    ui_vic20_draw_system(ui);
    if (ui->c1530.valid) {
        ui_c1530_draw(&ui->c1530);
    }
    ui_audio_draw(&ui->audio, ui->vic20->sample_pos);
    ui_kbd_draw(&ui->kbd);
    ui_m6502_draw(&ui->cpu);
    ui_m6522_draw(&ui->via[0]);
    ui_m6522_draw(&ui->via[1]);
    ui_m6561_draw(&ui->vic);
    ui_memmap_draw(&ui->memmap);
    for (int i = 0; i < 4; i++) {
        ui_memedit_draw(&ui->memedit[i]);
        ui_dasm_draw(&ui->dasm[i]);
    }
    ui_dbg_draw(&ui->dbg);
}

void ui_vic20_exec(ui_vic20_t* ui, uint32_t frame_time_us) {
    CHIPS_ASSERT(ui && ui->vic20);
    uint32_t ticks_to_run = clk_us_to_ticks(VIC20_FREQUENCY, frame_time_us);
    vic20_t* vic20 = ui->vic20;
    for (uint32_t i = 0; (i < ticks_to_run) && (!ui->dbg.dbg.stopped); i++) {
        vic20_tick(vic20);
        ui_dbg_tick(&ui->dbg, vic20->pins);
    }
    kbd_update(&ui->vic20->kbd, frame_time_us);
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif
#endif /* CHIPS_IMPL */



