#pragma once
/*#
    # ui_ay38910.h

    Debug visualization for AY-3-8910 sound chip.

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
        - ay38910.h
        - ui_chip.h

    Include the following headers before including the *implementation*:
        - imgui.h
        - ay38910.h
        - ui_chip.h
        - ui_util.h

    All string data provided to the ui_ay38910_init() must remain alive until
    until ui_ay38910_discard() is called!

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

/* setup parameters for ui_ay38910_init()
    NOTE: all string data must remain alive until ui_ay38910_discard()!
*/
typedef struct {
    const char* title;          /* window title */
    ay38910_t* ay;              /* pointer to ay38910_t instance to track */
    int x, y;                   /* initial window pos */
    int w, h;                   /* initial window size or zero for default size */
    bool open;                  /* initial open state */
    ui_chip_desc_t chip_desc;   /* chip visualization desc */
} ui_ay38910_desc_t;

typedef struct {
    const char* title;
    ay38910_t* ay;
    float init_x, init_y;
    float init_w, init_h;
    bool open;
    bool valid;
    ui_chip_t chip;
} ui_ay38910_t;

void ui_ay38910_init(ui_ay38910_t* win, const ui_ay38910_desc_t* desc);
void ui_ay38910_discard(ui_ay38910_t* win);
void ui_ay38910_draw(ui_ay38910_t* win);

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

void ui_ay38910_init(ui_ay38910_t* win, const ui_ay38910_desc_t* desc) {
    CHIPS_ASSERT(win && desc);
    CHIPS_ASSERT(desc->title);
    CHIPS_ASSERT(desc->ay);
    memset(win, 0, sizeof(ui_ay38910_t));
    win->title = desc->title;
    win->ay = desc->ay;
    win->init_x = (float) desc->x;
    win->init_y = (float) desc->y;
    win->init_w = (float) ((desc->w == 0) ? 440 : desc->w);
    win->init_h = (float) ((desc->h == 0) ? 370 : desc->h);
    win->open = desc->open;
    win->valid = true;
    ui_chip_init(&win->chip, &desc->chip_desc);
}

void ui_ay38910_discard(ui_ay38910_t* win) {
    CHIPS_ASSERT(win && win->valid);
    win->valid = false;
}

static void _ui_ay38910_draw_state(ui_ay38910_t* win) {
    ay38910_t* ay = win->ay;
    ImGui::Columns(4, "##ay_channels", false);
    ImGui::SetColumnWidth(0, 96);
    ImGui::SetColumnWidth(1, 40);
    ImGui::SetColumnWidth(2, 40);
    ImGui::SetColumnWidth(3, 40);
    ImGui::NextColumn();
    ImGui::Text("ChnA"); ImGui::NextColumn();
    ImGui::Text("ChnB"); ImGui::NextColumn();
    ImGui::Text("ChnC"); ImGui::NextColumn();
    ImGui::Separator();
    ImGui::Text("Tone Period"); ImGui::NextColumn();
    for (int i = 0; i < 3; i++) {
        ImGui::Text("%04X", ay->tone[i].period); ImGui::NextColumn();
    }
    ImGui::Text("Tone Count"); ImGui::NextColumn();
    for (int i = 0; i < 3; i++) {
        ImGui::Text("%04X", ay->tone[i].counter); ImGui::NextColumn();
    }
    ImGui::Text("Tone Bit"); ImGui::NextColumn();
    for (int i = 0; i < 3; i++) {
        ImGui::Text("%s", ay->tone[i].bit ? "ON":"OFF"); ImGui::NextColumn();
    }
    ImGui::Text("Volume"); ImGui::NextColumn();
    for (int i = 0; i < 3; i++) {
        ImGui::Text("%X", ay->reg[AY38910_REG_AMP_A + i] & 0x0F); ImGui::NextColumn();
    }
    ImGui::Text("Ampl Ctrl"); ImGui::NextColumn();
    for (int i = 0; i < 3; i++) {
        const uint8_t v = ay->reg[AY38910_REG_AMP_A + i] & 0x10;
        ImGui::Text("%s", v ? "ENV":"VOL"); ImGui::NextColumn();
    }
    ImGui::Text("Mix Tone"); ImGui::NextColumn();
    for (int i = 0; i < 3; i++) {
        const uint8_t v = (ay->enable >> i) & 1;
        ImGui::Text("%s", v ? "OFF":"ON"); ImGui::NextColumn();
    }
    ImGui::Text("Mix Noise"); ImGui::NextColumn();
    for (int i = 0; i < 3; i++) {
        const uint8_t v = (ay->enable >> (i+3)) & 1;
        ImGui::Text("%s", v ? "OFF":"ON"); ImGui::NextColumn();
    }
    ImGui::Columns();
    ImGui::Separator();
    ImGui::Text("Noise Period  %02X (reg:%02X)", ay->noise.period, ay->reg[AY38910_REG_PERIOD_NOISE]);
    ImGui::Text("Noise Count   %02X", ay->noise.counter);
    ImGui::Text("Noise Rand    %05X", ay->noise.rng);
    ImGui::Text("Noise Bit     %s", ay->noise.bit ? "ON":"OFF");
    ImGui::Separator();
    ImGui::Text("Env Period    %04X (reg:%04X)", ay->env.period, (ay->reg[AY38910_REG_ENV_PERIOD_COARSE]<<8)|ay->reg[AY38910_REG_ENV_PERIOD_FINE]);
    ImGui::Text("Env Count     %04X", ay->env.counter);
    ImGui::Text("Env Ampl      %02X", ay->env.shape_state);
    ImGui::Separator();
    const int num_ports = (ay->type==AY38910_TYPE_8910) ? 2 : ((ay->type == AY38910_TYPE_8912) ? 1 : 0);
    const int max_ports = 2;
    ImGui::Columns(max_ports + 1, "##ay_ports", false);
    ImGui::SetColumnWidth(0, 96);
    ImGui::SetColumnWidth(1, 60);
    ImGui::SetColumnWidth(2, 60);
    ImGui::NextColumn();
    ImGui::Text("PortA"); ImGui::NextColumn();
    ImGui::Text("PortB"); ImGui::NextColumn();
    ImGui::Text("In/Out Dir"); ImGui::NextColumn();
    int i;
    for (i = 0; i < num_ports; i++) {
        const uint8_t v = (ay->enable >> (i+6)) & 1;
        ImGui::Text("%s", v ? "OUT":"IN"); ImGui::NextColumn();
    }
    for (; i < max_ports; i++) {
        ImGui::Text("-"); ImGui::NextColumn();
    }
    ImGui::Text("Data Store"); ImGui::NextColumn();
    for (i = 0; i < num_ports; i++) {
        ImGui::Text("%02X", (i == 0) ? ay->port_a : ay->port_b); ImGui::NextColumn();
    }
    for (; i < max_ports; i++) {
        ImGui::Text("-"); ImGui::NextColumn();
    }
}

void ui_ay38910_draw(ui_ay38910_t* win) {
    CHIPS_ASSERT(win && win->valid && win->title && win->ay);
    if (!win->open) {
        return;
    }
    ImGui::SetNextWindowPos(ImVec2(win->init_x, win->init_y), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(win->init_w, win->init_h), ImGuiCond_Once);
    if (ImGui::Begin(win->title, &win->open)) {
        ImGui::BeginChild("##ay_chip", ImVec2(176, 0), true);
        ui_chip_draw(&win->chip, win->ay->pins);
        ImGui::EndChild();
        ImGui::SameLine();
        ImGui::BeginChild("##ay_state", ImVec2(0, 0), true);
        _ui_ay38910_draw_state(win);
        ImGui::EndChild();
    }
    ImGui::End();
}
#endif /* CHIPS_IMPL */
