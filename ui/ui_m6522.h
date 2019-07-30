#pragma once
/*#
    # ui_m6522.h

    Debug visualization UI for m6522.h

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
        - m6522.h
        - ui_chip.h

    Include the following headers before including the *implementation*:
        - imgui.h
        - m6522.h
        - ui_chip.h
        - ui_util.h

    All strings provided to ui_m6522_init() must remain alive until
    ui_m6522_discard() is called!

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

/* setup parameters for ui_m6522_init()
    NOTE: all string data must remain alive until ui_m6522_discard()!
*/
typedef struct ui_m6522_desc_t {
    const char* title;          /* window title */
    m6522_t* via;               /* m6522_t instance to track */
    int x, y;                   /* initial window pos */
    int w, h;                   /* initial window size (or 0 for default size) */
    bool open;                  /* initial window open state */
    ui_chip_desc_t chip_desc;   /* chip visualization desc */
} ui_m6522_desc_t;

typedef struct ui_m6522_t {
    const char* title;
    m6522_t* via;
    float init_x, init_y;
    float init_w, init_h;
    bool open;
    bool valid;
    ui_chip_t chip;
} ui_m6522_t;

void ui_m6522_init(ui_m6522_t* win, const ui_m6522_desc_t* desc);
void ui_m6522_discard(ui_m6522_t* win);
void ui_m6522_draw(ui_m6522_t* win);

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

void ui_m6522_init(ui_m6522_t* win, const ui_m6522_desc_t* desc) {
    CHIPS_ASSERT(win && desc);
    CHIPS_ASSERT(desc->title);
    CHIPS_ASSERT(desc->via);
    memset(win, 0, sizeof(ui_m6522_t));
    win->title = desc->title;
    win->via = desc->via;
    win->init_x = (float) desc->x;
    win->init_y = (float) desc->y;
    win->init_w = (float) ((desc->w == 0) ? 420 : desc->w);
    win->init_h = (float) ((desc->h == 0) ? 380 : desc->h);
    win->open = desc->open;
    win->valid = true;
    ui_chip_init(&win->chip, &desc->chip_desc);
}

void ui_m6522_discard(ui_m6522_t* win) {
    CHIPS_ASSERT(win && win->valid);
    win->valid = false;
}

static void _ui_m6522_draw_state(ui_m6522_t* win) {
    const m6522_t* via = win->via;
    ImGui::Columns(3, "##via_columns", false);
    ImGui::SetColumnWidth(0, 64);
    ImGui::SetColumnWidth(1, 80);
    ImGui::SetColumnWidth(2, 80);
    ImGui::NextColumn();
    ImGui::Text("Port A"); ImGui::NextColumn();
    ImGui::Text("Port B"); ImGui::NextColumn();
    ImGui::Separator();
    ImGui::Text("DDR"); ImGui::NextColumn();
    ui_util_b8("", via->ddr_a); ImGui::NextColumn();
    ui_util_b8("", via->ddr_b); ImGui::NextColumn();
    ImGui::Text("Output"); ImGui::NextColumn();
    ui_util_b8("", via->out_a); ImGui::NextColumn();
    ui_util_b8("", via->out_b); ImGui::NextColumn();
    ImGui::Text("Input"); ImGui::NextColumn();
    ui_util_b8("", via->in_a); ImGui::NextColumn();
    ui_util_b8("", via->in_b); ImGui::NextColumn();
    ImGui::Text("Pins"); ImGui::NextColumn();
    ui_util_b8("", via->port_a); ImGui::NextColumn();
    ui_util_b8("", via->port_b); ImGui::NextColumn();
    ImGui::Separator();
    ImGui::NextColumn();
    ImGui::Text("Timer 1"); ImGui::NextColumn();
    ImGui::Text("Timer 2"); ImGui::NextColumn();
    ImGui::Separator();
    ImGui::Text("Active"); ImGui::NextColumn();
    ImGui::Text("%s", via->t1_active ? "YES":"NO"); ImGui::NextColumn();
    ImGui::Text("%s", via->t2_active ? "YES":"NO"); ImGui::NextColumn();
    ImGui::Text("Latch"); ImGui::NextColumn();
    ImGui::Text("%02X%02X", via->t1lh, via->t1lh); ImGui::NextColumn();
    ImGui::Text("%02X%02X", via->t2lh, via->t2lh); ImGui::NextColumn();
    ImGui::Text("Counter"); ImGui::NextColumn();
    ImGui::Text("%04X", via->t1); ImGui::NextColumn();
    ImGui::Text("%04X", via->t2); ImGui::NextColumn();
    ImGui::Separator();
    ImGui::Columns();
    ImGui::Text("ACR      %02X", via->acr);
    ImGui::Text("PCR      %02X", via->pcr);
}

void ui_m6522_draw(ui_m6522_t* win) {
    CHIPS_ASSERT(win && win->valid && win->via);
    if (!win->open) {
        return;
    }
    ImGui::SetNextWindowPos(ImVec2(win->init_x, win->init_y), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(win->init_w, win->init_h), ImGuiCond_Once);
    if (ImGui::Begin(win->title, &win->open)) {
        ImGui::BeginChild("##m6522_chip", ImVec2(176, 0), true);
        ui_chip_draw(&win->chip, win->via->pins);
        ImGui::EndChild();
        ImGui::SameLine();
        ImGui::BeginChild("##m6522_state", ImVec2(0, 0), true);
        _ui_m6522_draw_state(win);
        ImGui::EndChild();
    }
    ImGui::End();
}
#endif /* CHIPS_IMPL */
