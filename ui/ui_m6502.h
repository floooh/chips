#pragma once
/*#
    # ui_m6502.h

    Debug visualization UI for m6502.h

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

    Include the following headers before the including the *declaration*:
        - m6502.h
        - ui_chip.h

    Include the following headers before including the *implementation*:
        - imgui.h
        - m6502.h
        - ui_chip.h
        - ui_util.h

    All strings provided to ui_m6502_init() must remain alive until
    ui_m6502_discard() is called!

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

/* setup parameters for ui_m6502_init()
    NOTE: all string data must remain alive until ui_m6502_discard()!
*/
typedef struct {
    const char* title;          /* window title */
    m6502_t* cpu;               /* m6502_t instance to track */
    int x, y;                   /* initial window position */
    int w, h;                   /* initial window width and height */
    bool open;                  /* initial open state */
    ui_chip_desc_t chip_desc;   /* chips visualization desc */
} ui_m6502_desc_t;

typedef struct {
    const char* title;
    m6502_t* cpu;
    float init_x, init_y;
    float init_w, init_h;
    bool open;
    bool valid;
    ui_chip_t chip;
} ui_m6502_t;

void ui_m6502_init(ui_m6502_t* win, const ui_m6502_desc_t* desc);
void ui_m6502_discard(ui_m6502_t* win);
void ui_m6502_draw(ui_m6502_t* win);

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

void ui_m6502_init(ui_m6502_t* win, const ui_m6502_desc_t* desc) {
    CHIPS_ASSERT(win && desc);
    CHIPS_ASSERT(desc->title);
    CHIPS_ASSERT(desc->cpu);
    memset(win, 0, sizeof(ui_m6502_t));
    win->title = desc->title;
    win->cpu = desc->cpu;
    win->init_x = (float) desc->x;
    win->init_y = (float) desc->y;
    win->init_w = (float) ((desc->w == 0) ? 360 : desc->w);
    win->init_h = (float) ((desc->h == 0) ? 320 : desc->h);
    win->open = desc->open;
    win->valid = true;
    ui_chip_init(&win->chip, &desc->chip_desc);
}

void ui_m6502_discard(ui_m6502_t* win) {
    CHIPS_ASSERT(win && win->valid);
    win->valid = false;
}

static void _ui_m6502_regs(ui_m6502_t* win) {
    m6502_t* cpu = win->cpu;
    ImGui::Text("A:  %02X", cpu->state.A);
    ImGui::Text("X:  %02X", cpu->state.X);
    ImGui::Text("Y:  %02X", cpu->state.Y);
    ImGui::Text("S:  %02X", cpu->state.S);
    const uint8_t f = cpu->state.P;
    char f_str[9] = {
        (f & M6502_NF) ? 'N':'-',
        (f & M6502_VF) ? 'V':'-',
        (f & M6502_XF) ? 'X':'-',
        (f & M6502_BF) ? 'B':'-',
        (f & M6502_DF) ? 'D':'-',
        (f & M6502_IF) ? 'I':'-',
        (f & M6502_ZF) ? 'Z':'-',
        (f & M6502_CF) ? 'C':'-',
        0
    };
    ImGui::Text("P:  %02X %s", f, f_str);
    ImGui::Text("PC: %04X", cpu->state.PC);
    ImGui::Separator();
    ImGui::Text("6510 I/O Port:");
    ui_util_b8("  DDR:    ", cpu->io_ddr);
    ui_util_b8("  Input:  ", cpu->io_inp);
    ui_util_b8("  Output: ", cpu->io_out);
    ui_util_b8("  Drive:  ", cpu->io_drive);
    ui_util_b8("  Float:  ", cpu->io_floating);
    ui_util_b8("  Pullup: ", cpu->io_pullup);
    ui_util_b8("  Pins:   ", cpu->io_pins);
    ImGui::Separator();
    ImGui::Text("BCD: %s", cpu->state.bcd_enabled ? "enabled":"disabled");
}

void ui_m6502_draw(ui_m6502_t* win) {
    CHIPS_ASSERT(win && win->valid && win->cpu);
    if (!win->open) {
        return;
    }
    ImGui::SetNextWindowPos(ImVec2(win->init_x, win->init_y), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(win->init_w, win->init_h), ImGuiCond_Once);
    if (ImGui::Begin(win->title, &win->open)) {
        ImGui::BeginChild("##m6502_chip", ImVec2(176, 0), true);
        ui_chip_draw(&win->chip, win->cpu->state.PINS);
        ImGui::EndChild();
        ImGui::SameLine();
        ImGui::BeginChild("##m6502_regs", ImVec2(0, 0), true);
        _ui_m6502_regs(win);
        ImGui::EndChild();
    }
    ImGui::End();
}
#endif /* CHIPS_IMPL */
