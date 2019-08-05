#pragma once
/*#
    # ui_z80ctc.h

    Debug visualization UI for the Z80 CTC.

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
        - z80ctc.h
        - ui_chip.h

    Include the following headers before including the *implementation*:
        - imgui.h
        - z80ctc.h
        - ui_chip.h

    All strings provided to ui_z80ctc_init() must remain alive until
    ui_z80ctc_discard() is called!

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

/* setup parameters for ui_z80ctc_init()

    NOTE: all string data must remain alive until ui_z80ctc_discard()!
*/
typedef struct {
    const char* title;          /* window title */
    z80ctc_t* ctc;              /* pointer to CTC to track */
    int x, y;                   /* initial window position */
    int w, h;                   /* initial window size, or 0 for default size */
    bool open;                  /* initial open state */
    ui_chip_desc_t chip_desc;   /* chips visualization desc */
} ui_z80ctc_desc_t;

typedef struct {
    const char* title;
    z80ctc_t* ctc;
    float init_x, init_y;
    float init_w, init_h;
    bool open;
    bool valid;
    ui_chip_t chip;
} ui_z80ctc_t;

void ui_z80ctc_init(ui_z80ctc_t* win, const ui_z80ctc_desc_t* desc);
void ui_z80ctc_discard(ui_z80ctc_t* win);
void ui_z80ctc_draw(ui_z80ctc_t* win);

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

void ui_z80ctc_init(ui_z80ctc_t* win, const ui_z80ctc_desc_t* desc) {
    CHIPS_ASSERT(win && desc);
    CHIPS_ASSERT(desc->title);
    CHIPS_ASSERT(desc->ctc);
    memset(win, 0, sizeof(ui_z80ctc_t));
    win->title = desc->title;
    win->ctc = desc->ctc;
    win->init_x = (float) desc->x;
    win->init_y = (float) desc->y;
    win->init_w = (float) ((desc->w == 0) ? 460 : desc->w);
    win->init_h = (float) ((desc->h == 0) ? 300 : desc->h);
    win->open = desc->open;
    win->valid = true;
    ui_chip_init(&win->chip, &desc->chip_desc);
}

void ui_z80ctc_discard(ui_z80ctc_t* win) {
    CHIPS_ASSERT(win && win->valid);
    win->valid = false;
}

static void _ui_z80ctc_channels(ui_z80ctc_t* win) {
    const z80ctc_t* ctc = win->ctc;

    ImGui::Columns(5, "##ctc_columns", false);
    ImGui::SetColumnWidth(0, 80);
    ImGui::SetColumnWidth(1, 40);
    ImGui::SetColumnWidth(2, 40);
    ImGui::SetColumnWidth(3, 40);
    ImGui::SetColumnWidth(4, 40);
    ImGui::NextColumn();
    ImGui::Text("Chn0"); ImGui::NextColumn();
    ImGui::Text("Chn1"); ImGui::NextColumn();
    ImGui::Text("Chn2"); ImGui::NextColumn();
    ImGui::Text("Chn3"); ImGui::NextColumn();
    ImGui::Separator();

    ImGui::Text("Constant"); ImGui::NextColumn();
    for (int i = 0; i < 4; i++) {
        ImGui::Text("%02X", ctc->chn[i].constant); ImGui::NextColumn();
    }
    ImGui::Text("Counter"); ImGui::NextColumn();
    for (int i = 0; i < 4; i++) {
        ImGui::Text("%02X", ctc->chn[i].down_counter); ImGui::NextColumn();
    }
    ImGui::Text("INT Vec"); ImGui::NextColumn();
    for (int i = 0; i < 4; i++) {
        ImGui::Text("%02X", ctc->chn[i].int_vector); ImGui::NextColumn();
    }
    ImGui::Text("Control"); ImGui::NextColumn();
    for (int i = 0; i < 4; i++) {
        ImGui::Text("%02X", ctc->chn[i].control); ImGui::NextColumn();
    }
    ImGui::Text("       INT"); ImGui::NextColumn();
    for (int i = 0; i < 4; i++) {
        ImGui::Text("%s", (ctc->chn[i].control & Z80CTC_CTRL_EI) ? "EI":"DI"); ImGui::NextColumn();
    }
    ImGui::Text("      MODE"); ImGui::NextColumn();
    for (int i = 0; i < 4; i++) {
        ImGui::Text("%s", (ctc->chn[i].control & Z80CTC_CTRL_MODE) ? "CTR":"TMR"); ImGui::NextColumn();
    }
    ImGui::Text("  PRESCALE"); ImGui::NextColumn();
    for (int i = 0; i < 4; i++) {
        ImGui::Text("%s", (ctc->chn[i].control & Z80CTC_CTRL_PRESCALER) ? "256":"16"); ImGui::NextColumn();
    }
    ImGui::Text("      EDGE"); ImGui::NextColumn();
    for (int i = 0; i < 4; i++) {
        ImGui::Text("%s", (ctc->chn[i].control & Z80CTC_CTRL_EDGE) ? "RISE":"FALL"); ImGui::NextColumn();
    }
    ImGui::Text("   TRIGGER"); ImGui::NextColumn();
    for (int i = 0; i < 4; i++) {
        ImGui::Text("%s", (ctc->chn[i].control & Z80CTC_CTRL_TRIGGER) ? "PULS":"AUTO"); ImGui::NextColumn();
    }
    ImGui::Text("  CONSTANT"); ImGui::NextColumn();
    for (int i = 0; i < 4; i++) {
        ImGui::Text("%s", (ctc->chn[i].control & Z80CTC_CTRL_CONST_FOLLOWS) ? "FLWS":"NONE"); ImGui::NextColumn();
    }
    ImGui::Text("     RESET"); ImGui::NextColumn();
    for (int i = 0; i < 4; i++) {
        ImGui::Text("%s", (ctc->chn[i].control & Z80CTC_CTRL_RESET) ? "ON":"OFF"); ImGui::NextColumn();
    }
    ImGui::Text("   CONTROL"); ImGui::NextColumn();
    for (int i = 0; i < 4; i++) {
        ImGui::Text("%s", (ctc->chn[i].control & Z80CTC_CTRL_CONTROL) ? "WRD":"VEC"); ImGui::NextColumn();
    }
}

void ui_z80ctc_draw(ui_z80ctc_t* win) {
    CHIPS_ASSERT(win && win->valid && win->title && win->ctc);
    if (!win->open) {
        return;
    }
    ImGui::SetNextWindowPos(ImVec2(win->init_x, win->init_y), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(win->init_w, win->init_h), ImGuiCond_Once);
    if (ImGui::Begin(win->title, &win->open)) {
        ImGui::BeginChild("##ctc_chip", ImVec2(176, 0), true);
        ui_chip_draw(&win->chip, win->ctc->pins);
        ImGui::EndChild();
        ImGui::SameLine();
        ImGui::BeginChild("##ctc_vals", ImVec2(0, 0), true);
        _ui_z80ctc_channels(win);
        ImGui::EndChild();
    }
    ImGui::End();
}

#endif /* CHIPS_IMPL */
