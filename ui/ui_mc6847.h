#pragma once
/*#
    # ui_mc6847.h

    Debug visualization for mc6847.h

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
        - mc6847.h
        - ui_chip.h

    Include the following headers before including the *implementation*:
        - imgui.h
        - mc6847.h
        - ui_chip.h
        - ui_util.h

    All string data provided to ui_mc6847_init() must remain alive until
    until ui_mc6847_discard() is called!

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

/* setup parameters for ui_mc6847_init()
    NOTE: all string data must remain alive until ui_mc6847_discard()!
*/
typedef struct {
    const char* title;          /* window title */
    mc6847_t* mc6847;           /* pointer to mc6847_t instance to track */
    int x, y;                   /* initial window pos */
    int w, h;                   /* initial window size, or 0 for default size */
    bool open;                  /* initial open state */
    ui_chip_desc_t chip_desc;   /* chip visualization desc */
} ui_mc6847_desc_t;

typedef struct {
    const char* title;
    mc6847_t* mc6847;
    float init_x, init_y;
    float init_w, init_h;
    bool open;
    bool valid;
    ui_chip_t chip;
} ui_mc6847_t;

void ui_mc6847_init(ui_mc6847_t* win, const ui_mc6847_desc_t* desc);
void ui_mc6847_discard(ui_mc6847_t* win);
void ui_mc6847_draw(ui_mc6847_t* win);

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

void ui_mc6847_init(ui_mc6847_t* win, const ui_mc6847_desc_t* desc) {
    CHIPS_ASSERT(win && desc);
    CHIPS_ASSERT(desc->title);
    CHIPS_ASSERT(desc->mc6847);
    memset(win, 0, sizeof(ui_mc6847_t));
    win->title = desc->title;
    win->mc6847 = desc->mc6847;
    win->init_x = (float) desc->x;
    win->init_y = (float) desc->y;
    win->init_w = (float) ((desc->w == 0) ? 348 : desc->w);
    win->init_h = (float) ((desc->h == 0) ? 360 : desc->h);
    win->open = desc->open;
    win->valid = true;
    ui_chip_init(&win->chip, &desc->chip_desc);
}

void ui_mc6847_discard(ui_mc6847_t* win) {
    CHIPS_ASSERT(win && win->valid);
    win->valid = false;
}

static void _ui_mc6847_draw_hwcolors(ui_mc6847_t* win) {
    ImGui::Text("Gfx Mode Colors:");
    ImVec4 c;
    const ImVec2 size(18,18);
    for (int i = 0; i < 8; i++) {
        c = ImColor(win->mc6847->palette[i]);
        ImGui::PushID(i);
        ImGui::ColorButton("##gm_color", c, ImGuiColorEditFlags_NoAlpha, size);
        ImGui::PopID();
        if (((i+1) % 4) != 0) {
            ImGui::SameLine();
        }
    }
    ImGui::Separator();
    ImGui::Text("Text Mode Colors:");
    ImVec4 tm_green       = ImColor(win->mc6847->alnum_green);
    ImVec4 tm_dark_green  = ImColor(win->mc6847->alnum_dark_green);
    ImVec4 tm_orange      = ImColor(win->mc6847->alnum_orange);
    ImVec4 tm_dark_orange = ImColor(win->mc6847->alnum_dark_orange);
    ImGui::ColorButton("##tm_green", tm_green, ImGuiColorEditFlags_NoAlpha, size); ImGui::SameLine();
    ImGui::ColorButton("##tm_dark_green", tm_dark_green, ImGuiColorEditFlags_NoAlpha, size); ImGui::SameLine();
    ImGui::ColorButton("##tm_orange", tm_orange, ImGuiColorEditFlags_NoAlpha, size); ImGui::SameLine();
    ImGui::ColorButton("##tm_dark_orange", tm_dark_orange, ImGuiColorEditFlags_NoAlpha, size);
}

static void _ui_mc6847_draw_values(ui_mc6847_t* win) {
    ImGui::Text("H Period:     %4d", win->mc6847->h_period);
    ImGui::Text("H Sync Start: %4d", win->mc6847->h_sync_start);
    ImGui::Text("H Sync End:   %4d", win->mc6847->h_sync_end);
    ImGui::Text("H Counter:    %4d", win->mc6847->h_count);
    ImGui::Text("Line Counter: %4d", win->mc6847->l_count);
}

void ui_mc6847_draw(ui_mc6847_t* win) {
    CHIPS_ASSERT(win && win->valid);
    if (!win->open) {
        return;
    }
    ImGui::SetNextWindowPos(ImVec2(win->init_x, win->init_y), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(win->init_w, win->init_h), ImGuiCond_Once);
    if (ImGui::Begin(win->title, &win->open)) {
        ImGui::BeginChild("##chip", ImVec2(176, 0), true);
        ui_chip_draw(&win->chip, win->mc6847->pins);
        ImGui::EndChild();
        ImGui::SameLine();
        ImGui::BeginChild("##state", ImVec2(0, 0), true);
        _ui_mc6847_draw_hwcolors(win);
        ImGui::Separator();
        _ui_mc6847_draw_values(win);
        ImGui::EndChild();
    }
    ImGui::End();
}
#endif /* CHIPS_IMPL */
