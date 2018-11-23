#pragma once
/*#
    # ui_kc85.h

    Integrated debugging UI for KC85/2../4.

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
    ui_kc85.h both for the declaration and implementation.

    - kc85.h
    - mem.h
    - ui_chip.h
    - ui_util.h
    - ui_z80.h
    - ui_z80ctc.h
    - ui_z80pio.h
    - ui_kc85sys.h
    - ui_audio.h
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
typedef void (*ui_kc85_boot_t)(kc85_t* sys, kc85_type_t type);

typedef struct {
    kc85_t* kc85;
    ui_kc85_boot_t boot_cb; /* user-provided callback to reboot to different config */
} ui_kc85_desc_t;

typedef struct {
    kc85_t* kc85;
    ui_kc85_boot_t boot_cb;
    ui_z80_t cpu;
    ui_z80pio_t pio;
    ui_z80ctc_t ctc;
    ui_kc85sys_t sys;
    ui_audio_t audio;
    ui_memedit_t memedit;
    ui_memmap_t memmap;
    ui_dasm_t dasm;
} ui_kc85_t;

void ui_kc85_init(ui_kc85_t* ui, const ui_kc85_desc_t* desc);
void ui_kc85_discard(ui_kc85_t* ui);
void ui_kc85_draw(ui_kc85_t* ui, double time_ms);

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
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-field-initializers"

static void _ui_kc85_draw_menu(ui_kc85_t* ui, double time_ms) {
    CHIPS_ASSERT(ui && ui->kc85);
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("System")) {
            if (ImGui::MenuItem("Reset")) {
                kc85_reset(ui->kc85);
            }
            if (ImGui::MenuItem("KC85/2")) {
                ui->boot_cb(ui->kc85, KC85_TYPE_2);
            }
            if (ImGui::MenuItem("KC85/3")) {
                ui->boot_cb(ui->kc85, KC85_TYPE_3);
            }
            if (ImGui::MenuItem("KC85/4")) {
                ui->boot_cb(ui->kc85, KC85_TYPE_4);
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Hardware")) {
            ImGui::MenuItem("Memory Map", 0, &ui->memmap.open);
            ImGui::MenuItem("System State", 0, &ui->sys.open);
            ImGui::MenuItem("Audio Output", 0, &ui->audio.open);
            ImGui::MenuItem("Z80 CPU", 0, &ui->cpu.open);
            ImGui::MenuItem("Z80 PIO", 0, &ui->pio.open);
            ImGui::MenuItem("Z80 CTC", 0, &ui->ctc.open);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Debug")) {
            ImGui::MenuItem("Memory Editor", 0, &ui->memedit.open);
            ImGui::MenuItem("Disassembler", 0, &ui->dasm.open);
            ImGui::MenuItem("CPU Debugger (TODO)");
            ImGui::MenuItem("Scan Commands (TODO)");
            ImGui::EndMenu();
        }
        ImGui::SameLine(ImGui::GetWindowWidth() - 120);
        ImGui::Text("emu: %.2fms", time_ms);
        ImGui::EndMainMenuBar();
    }
}

static void _ui_kc85_update_memmap(ui_kc85_t* ui) {
    CHIPS_ASSERT(ui && ui->kc85);
    const uint8_t pio_a = ui->kc85->pio_a;
    const uint8_t pio_b = ui->kc85->pio_b;
    const uint8_t io86  = ui->kc85->io86;
    const uint8_t io84  = ui->kc85->io84;
    ui_memmap_reset(&ui->memmap);
    if (KC85_TYPE_2 == ui->kc85->type) {
        /* KC85/2 memory map */
        ui_memmap_layer(&ui->memmap, "System");
            ui_memmap_region(&ui->memmap, "RAM0", 0x0000, 0x4000, pio_a & KC85_PIO_A_RAM);
            ui_memmap_region(&ui->memmap, "IRM", 0x8000, 0x4000, pio_a & KC85_PIO_A_IRM);
            ui_memmap_region(&ui->memmap, "CAOS ROM 1", 0xE000, 0x0800, pio_a & KC85_PIO_A_CAOS_ROM);
            ui_memmap_region(&ui->memmap, "CAOS ROM 2", 0xF000, 0x0800, pio_a & KC85_PIO_A_CAOS_ROM);
    }
    else if (KC85_TYPE_3 == ui->kc85->type) {
        /* KC85/3 memory map */
        ui_memmap_layer(&ui->memmap, "System");
            ui_memmap_region(&ui->memmap, "RAM0", 0x0000, 0x4000, pio_a & KC85_PIO_A_RAM);
            ui_memmap_region(&ui->memmap, "IRM", 0x8000, 0x4000, pio_a & KC85_PIO_A_IRM);
            ui_memmap_region(&ui->memmap, "BASIC ROM", 0xC000, 0x2000, pio_a & KC85_PIO_A_BASIC_ROM);
            ui_memmap_region(&ui->memmap, "CAOS ROM", 0xE000, 0x2000, pio_a & KC85_PIO_A_CAOS_ROM);
    }
    else {
        /* KC85/4 memory map */
        ui_memmap_layer(&ui->memmap, "System 0");
            ui_memmap_region(&ui->memmap, "RAM0", 0x0000, 0x4000, pio_a & KC85_PIO_A_RAM);
            ui_memmap_region(&ui->memmap, "RAM4", 0x4000, 0x4000, io86 & KC85_IO86_RAM4);
            ui_memmap_region(&ui->memmap, "IRM0 PIXELS", 0x8000, 0x2800, (pio_a & KC85_PIO_A_IRM) && ((io84 & 6)==0));
            ui_memmap_region(&ui->memmap, "IRM0", 0xA800, 0x1800, pio_a & KC85_PIO_A_IRM);
            ui_memmap_region(&ui->memmap, "CAOS ROM E", 0xE000, 0x2000, pio_a & KC85_PIO_A_CAOS_ROM);
        ui_memmap_layer(&ui->memmap, "System 1");
            ui_memmap_region(&ui->memmap, "IRM0 COLORS", 0x8000, 0x2800, (pio_a & KC85_PIO_A_IRM) && ((io84 & 6)==2));
            ui_memmap_region(&ui->memmap, "CAOS ROM C", 0xC000, 0x1000, io86 & KC85_IO86_CAOS_ROM_C);
        ui_memmap_layer(&ui->memmap, "System 2");
            ui_memmap_region(&ui->memmap, "IRM1 PIXELS", 0x8000, 0x2800, (pio_a & KC85_PIO_A_IRM) && ((io84 & 6)==4));
            ui_memmap_region(&ui->memmap, "BASIC ROM", 0xC000, 0x2000, pio_a & KC85_PIO_A_BASIC_ROM);
        ui_memmap_layer(&ui->memmap, "System 3");
            ui_memmap_region(&ui->memmap, "IRM1 COLORS", 0x8000, 0x2800, (pio_a & KC85_PIO_A_IRM) && ((io84 & 6)==6));
        ui_memmap_layer(&ui->memmap, "System 4");
            ui_memmap_region(&ui->memmap, "RAM8 BANK0", 0x8000, 0x4000, (pio_b & KC85_PIO_B_RAM8) && !(io84 & KC85_IO84_SEL_RAM8));
        ui_memmap_layer(&ui->memmap, "System 5");
            ui_memmap_region(&ui->memmap, "RAM8 BANK1", 0x8000, 0x4000, (pio_b & KC85_PIO_B_RAM8) && (io84 & KC85_IO84_SEL_RAM8));
    }
    for (int i = 0; i < KC85_NUM_SLOTS; i++) {
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
    { 0, 0, 0 }
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
    { 0, 0, 0 }
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
    { 0, 0, 0 }
};

static uint8_t _ui_kc85_mem_read(int layer, uint16_t addr, void* user_data) {
    CHIPS_ASSERT(user_data);
    kc85_t* kc85 = (kc85_t*) user_data;
    if (layer == 0) {
        return mem_rd(&kc85->mem, addr);
    }
    else {
        return mem_layer_rd(&kc85->mem, layer-1, addr);
    }
}

static void _ui_kc85_mem_write(int layer, uint16_t addr, uint8_t data, void* user_data) {
    CHIPS_ASSERT(user_data);
    kc85_t* kc85 = (kc85_t*) user_data;
    if (layer == 0) {
        mem_wr(&kc85->mem, addr, data);
    }
    else {
        mem_layer_wr(&kc85->mem, layer-1, addr, data);
    }
}

void ui_kc85_init(ui_kc85_t* ui, const ui_kc85_desc_t* desc) {
    CHIPS_ASSERT(ui && desc);
    CHIPS_ASSERT(desc->kc85);
    CHIPS_ASSERT(desc->boot_cb);
    ui->kc85 = desc->kc85;
    ui->boot_cb = desc->boot_cb;
    {
        ui_z80_desc_t desc = {0};
        desc.title = "Z80 CPU";
        desc.cpu = &ui->kc85->cpu;
        desc.x = 40;
        desc.y = 60;
        ui_chip_init_chip_desc(&desc.chip_desc, "Z80\nCPU", 36, _ui_kc85_cpu_pins);
        ui_z80_init(&ui->cpu, &desc);
    }
    {
        ui_z80pio_desc_t desc = {0};
        desc.title = "Z80 PIO";
        desc.pio = &ui->kc85->pio;
        desc.x = 40;
        desc.y = 60;
        ui_chip_init_chip_desc(&desc.chip_desc, "Z80\nPIO", 40, _ui_kc85_pio_pins);
        ui_z80pio_init(&ui->pio, &desc);
    }
    {
        ui_z80ctc_desc_t desc = {0};
        desc.title = "Z80 CTC";
        desc.ctc = &ui->kc85->ctc;
        desc.x = 40;
        desc.y = 60;
        ui_chip_init_chip_desc(&desc.chip_desc, "Z80\nCTC", 32, _ui_kc85_ctc_pins);
        ui_z80ctc_init(&ui->ctc, &desc);
    }
    {
        ui_kc85sys_desc_t desc = {0};
        desc.title = "System State";
        desc.kc85 = ui->kc85;
        desc.x = 40;
        desc.y = 60;
        ui_kc85sys_init(&ui->sys, &desc);
    }
    {
        ui_audio_desc_t desc = {0};
        desc.title = "Audio Output";
        desc.sample_buffer = ui->kc85->sample_buffer;
        desc.num_samples = ui->kc85->num_samples;
        desc.x = 40;
        desc.y = 60;
        ui_audio_init(&ui->audio, &desc);
    }
    {
        ui_memedit_desc_t desc = {0};
        desc.title = "Memory Editor";
        desc.layers[0] = "CPU Mapped";
        desc.layers[1] = "Motherboard";
        desc.layers[2] = "Slot 08";
        desc.layers[3] = "Slot 0C";
        desc.read_cb = _ui_kc85_mem_read;
        desc.write_cb = _ui_kc85_mem_write;
        desc.user_data = ui->kc85;
        desc.x = 20;
        desc.y = 40;
        desc.h = 120;
        ui_memedit_init(&ui->memedit, &desc);
    }
    {
        ui_memmap_desc_t desc = {0};
        desc.title = "Memory Map";
        desc.x = 30;
        desc.y = 50;
        desc.w = 400;
        desc.h = 64;
        ui_memmap_init(&ui->memmap, &desc);
    }
    {
        ui_dasm_desc_t desc = {0};
        desc.title = "Disassembler";
        desc.layers[0] = "CPU Mapped";
        desc.layers[1] = "Motherboard";
        desc.layers[2] = "Slot 08";
        desc.layers[3] = "Slot 0C";
        desc.start_addr = 0xF000;
        desc.read_cb = _ui_kc85_mem_read;
        desc.user_data = ui->kc85;
        desc.x = 40;
        desc.y = 60;
        desc.w = 400;
        desc.h = 256;
        ui_dasm_init(&ui->dasm, &desc);
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
    ui_memedit_discard(&ui->memedit);
    ui_memmap_discard(&ui->memmap);
    ui_dasm_discard(&ui->dasm);
}

void ui_kc85_draw(ui_kc85_t* ui, double time_ms) {
    CHIPS_ASSERT(ui && ui->kc85);
    _ui_kc85_draw_menu(ui, time_ms);
    if (ui->memmap.open) {
        _ui_kc85_update_memmap(ui);
    }
    ui_audio_draw(&ui->audio, ui->kc85->sample_pos);
    ui_z80_draw(&ui->cpu);
    ui_z80pio_draw(&ui->pio);
    ui_z80ctc_draw(&ui->ctc);
    ui_memedit_draw(&ui->memedit);
    ui_memmap_draw(&ui->memmap);
    ui_dasm_draw(&ui->dasm);
    ui_kc85sys_draw(&ui->sys);
}

#pragma clang diagnostic pop
#endif /* CHIPS_IMPL */
