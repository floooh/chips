#pragma once
/*#
    # ui_z80pio.h

    Debug visualization UI for the Z80 PIO.

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
        - z80pio.h
        - ui_chip.h

    Include the following headers before including the *implementation*:
        - imgui.h
        - z80pio.h
        - ui_chip.h

    All strings provided to ui_z80pio_init() must remain alive until
    ui_z80pio_discard() is called!

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

/* setup parameters for ui_z80pio_init()

    NOTE: all string data must remain alive until ui_z80pio_discard()!
*/
typedef struct {
    const char* title;      /* window title */
    z80pio_t* pio;          /* pointer to PIO to track */
    int x, y;               /* initial window position */
    int w, h;               /* initial window size, or 0 for default size */
    bool open;              /* initial open state */
    ui_chip_desc_t chip_desc;   /* chip visualization desc */
} ui_z80pio_desc_t;

typedef struct {
    const char* title;
    z80pio_t* pio;
    float init_x, init_y;
    float init_w, init_h;
    bool open;
    bool valid;
    ui_chip_t chip;
} ui_z80pio_t;

void ui_z80pio_init(ui_z80pio_t* win, const ui_z80pio_desc_t* desc);
void ui_z80pio_discard(ui_z80pio_t* win);
void ui_z80pio_draw(ui_z80pio_t* win);

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

void ui_z80pio_init(ui_z80pio_t* win, const ui_z80pio_desc_t* desc) {
    CHIPS_ASSERT(win && desc);
    CHIPS_ASSERT(desc->title);
    CHIPS_ASSERT(desc->pio);
    memset(win, 0, sizeof(ui_z80pio_t));
    win->title = desc->title;
    win->pio = desc->pio;
    win->init_x = (float) desc->x;
    win->init_y = (float) desc->y;
    win->init_w = (float) ((desc->w == 0) ? 360 : desc->w);
    win->init_h = (float) ((desc->h == 0) ? 364 : desc->h);
    win->open = desc->open;
    win->valid = true;
    ui_chip_init(&win->chip, &desc->chip_desc);
}

void ui_z80pio_discard(ui_z80pio_t* win) {
    CHIPS_ASSERT(win && win->valid);
    win->valid = false;
}

static const char* _ui_z80pio_mode_str(uint8_t mode) {
    switch (mode) {
        case 0: return "OUT";
        case 1: return "INP";
        case 2: return "BDIR";
        case 3: return "BITC";
        default: return "INVALID";
    }
}

static void _ui_z80pio_ports(ui_z80pio_t* win) {
    const z80pio_t* pio = win->pio;

    ImGui::Columns(3, "##pio_columns", false);
    ImGui::SetColumnWidth(0, 80);
    ImGui::SetColumnWidth(1, 40);
    ImGui::SetColumnWidth(2, 40);
    ImGui::NextColumn();
    ImGui::Text("PA"); ImGui::NextColumn();
    ImGui::Text("PB"); ImGui::NextColumn();
    ImGui::Separator();

    ImGui::Text("Mode"); ImGui::NextColumn();
    for (int i = 0; i < 2; i++) {
        ImGui::Text("%s", _ui_z80pio_mode_str(pio->port[i].mode)); ImGui::NextColumn();
    }
    ImGui::Text("Output"); ImGui::NextColumn();
    for (int i = 0; i < 2; i++) {
        ImGui::Text("%02X", pio->port[i].output); ImGui::NextColumn();
    }
    ImGui::Text("Input"); ImGui::NextColumn();
    for (int i = 0; i < 2; i++) {
        ImGui::Text("%02X", pio->port[i].input); ImGui::NextColumn();
    }
    ImGui::Text("IO Select"); ImGui::NextColumn();
    for (int i = 0; i < 2; i++) {
        ImGui::Text("%02X", pio->port[i].io_select); ImGui::NextColumn();
    }
    ImGui::Text("INT Ctrl"); ImGui::NextColumn();
    for (int i = 0; i < 2; i++) {
        ImGui::Text("%02X", pio->port[i].int_control); ImGui::NextColumn();
    }
    ImGui::Text("  ei/di"); ImGui::NextColumn();
    for (int i = 0; i < 2; i++) {
        ImGui::Text("%s", pio->port[i].int_control & Z80PIO_INTCTRL_EI ? "EI":"DI"); ImGui::NextColumn();
    }
    ImGui::Text("  and/or"); ImGui::NextColumn();
    for (int i = 0; i < 2; i++) {
        ImGui::Text("%s", pio->port[i].int_control & Z80PIO_INTCTRL_ANDOR ? "AND":"OR"); ImGui::NextColumn();
    }
    ImGui::Text("  hi/lo"); ImGui::NextColumn();
    for (int i = 0; i < 2; i++) {
        ImGui::Text("%s", pio->port[i].int_control & Z80PIO_INTCTRL_HILO ? "HI":"LO"); ImGui::NextColumn();
    }
    ImGui::Text("INT Vec"); ImGui::NextColumn();
    for (int i = 0; i < 2; i++) {
        ImGui::Text("%02X", pio->port[i].int_vector); ImGui::NextColumn();
    }
    ImGui::Text("INT Mask"); ImGui::NextColumn();
    for (int i = 0; i < 2; i++) {
        ImGui::Text("%02X", pio->port[i].int_mask); ImGui::NextColumn();
    }
}

void ui_z80pio_draw(ui_z80pio_t* win) {
    CHIPS_ASSERT(win && win->valid && win->title && win->pio);
    if (!win->open) {
        return;
    }
    ImGui::SetNextWindowPos(ImVec2(win->init_x, win->init_y), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(win->init_w, win->init_h), ImGuiCond_Once);
    if (ImGui::Begin(win->title, &win->open)) {
        ImGui::BeginChild("##pio_chip", ImVec2(176, 0), true);
        ui_chip_draw(&win->chip, win->pio->pins);
        ImGui::EndChild();
        ImGui::SameLine();
        ImGui::BeginChild("##pio_vals", ImVec2(0, 0), true);
        _ui_z80pio_ports(win);
        ImGui::EndChild();
    }
    ImGui::End();
}

} /* extern "C" */
#endif

