#pragma once
/*#
    # ui_cpc_ga.h

    Debug visualization for the CPC gate array.
    
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

    Include the following headers (and their depenencies) before the declaration:

    - cpc.h

    Include the following header before the implementation:

    - imgui.h
    - ui_util.h
    
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

/* setup parameters for ui_cpc_ga_init()
    NOTE: all string data must remain alive until ui_cpc_ga_discard() is called!
*/
typedef struct {
    const char* title;      /* window title */
    cpc_t* cpc;             /* pointer to cpc instance to track */
    int x, y;               /* initial window position */
    int w, h;               /* initial window size or zero for default size */
    bool open;              /* initial open state */
} ui_cpc_ga_desc_t;

typedef struct {
    const char* title;
    cpc_t* cpc;
    float init_x, init_y;
    float init_w, init_h;
    bool open;
    bool valid;
} ui_cpc_ga_t;

void ui_cpc_ga_init(ui_cpc_ga_t* win, const ui_cpc_ga_desc_t* desc);
void ui_cpc_ga_discard(ui_cpc_ga_t* win);
void ui_cpc_ga_draw(ui_cpc_ga_t* win);

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

void ui_cpc_ga_init(ui_cpc_ga_t* win, const ui_cpc_ga_desc_t* desc) {
    CHIPS_ASSERT(win && desc);
    CHIPS_ASSERT(desc->title);
    CHIPS_ASSERT(desc->cpc);
    memset(win, 0, sizeof(ui_cpc_ga_t));
    win->title = desc->title;
    win->cpc = desc->cpc;
    win->init_x = (float) desc->x;
    win->init_y = (float) desc->y;
    win->init_w = (float) ((desc->w == 0) ? 428 : desc->w);
    win->init_h = (float) ((desc->h == 0) ? 428 : desc->h);
    win->open = desc->open;
    win->valid = true;
}

void ui_cpc_ga_discard(ui_cpc_ga_t* win) {
    CHIPS_ASSERT(win && win->valid);
    win->valid = false;
}

static void _ui_cpc_ga_draw_hw_colors(ui_cpc_ga_t* win) {
    ImGui::Text("Hardware Colors:");
    ImVec4 c;
    const ImVec2 size(18,18);
    for (int i = 0; i < 32; i++) {
        c = ImColor(win->cpc->ga.colors[i]);
        ImGui::PushID(i);
        ImGui::ColorButton("##hw_color", c, ImGuiColorEditFlags_NoAlpha, size);
        ImGui::PopID();
        if (((i+1) % 16) != 0) {
            ImGui::SameLine();
        }
    }
}

static void _ui_cpc_ga_draw_palette(ui_cpc_ga_t* win) {
    ImGui::Text("Palette Colors:");
    const ImVec2 size(18,18);
    ImVec4 c;
    for (int i = 0; i < 16; i++) {
        c = ImColor(win->cpc->ga.palette[i]);
        ImGui::PushID(128 + i);
        ImGui::ColorButton("##pal_color", c, ImGuiColorEditFlags_NoAlpha, size);
        ImGui::PopID();
        if (((i+1) % 16) != 0) {
            ImGui::SameLine();
        }
    }
}

static void _ui_cpc_ga_draw_border(ui_cpc_ga_t* win) {
    ImGui::Text("Border Colors:");
    const ImVec2 size(18,18);
    const ImVec4 c = ImColor(win->cpc->ga.border_color);
    ImGui::ColorButton("##brd_color", c, ImGuiColorEditFlags_NoAlpha, size);
}

static const char* _ui_cpc_ga_video_modes[4] = {
    "160 x 200 @ 16 color",
    "320 x 200 @ 4 colors",
    "640 x 200 @ 2 colors",
    "160 x 200 @ 4 colors"    
};

void ui_cpc_ga_draw(ui_cpc_ga_t* win) {
    CHIPS_ASSERT(win && win->valid && win->title && win->cpc);
    if (!win->open) {
        return;
    }
    ImGui::SetNextWindowPos(ImVec2(win->init_x, win->init_y), ImGuiSetCond_Once);
    ImGui::SetNextWindowSize(ImVec2(win->init_w, win->init_h), ImGuiSetCond_Once);
    if (ImGui::Begin(win->title, &win->open)) {
        cpc_t* cpc = win->cpc;
        ImGui::Checkbox("Debug Visualization", &cpc->video_debug_enabled);
        ImGui::Separator();
        _ui_cpc_ga_draw_hw_colors(win);
        ImGui::Separator();
        _ui_cpc_ga_draw_palette(win);
        ImGui::Separator();
        _ui_cpc_ga_draw_border(win);
        ImGui::Separator();
        ImGui::Text("Video Mode: %d (%s)", cpc->ga.video_mode, _ui_cpc_ga_video_modes[cpc->ga.video_mode % 3]);
        ImGui::Text("Sync: %s  IRQ: %s", cpc->ga.sync ? "ON ":"OFF", cpc->ga.intr ? "ON":"OFF");
        ImGui::Separator();
        ImGui::Text("HSync:");
        ImGui::Text("  Counter: %d", cpc->ga.hsync_counter);
        ImGui::Text("  HSync IRQ Counter: %d", cpc->ga.hsync_irq_counter);
        ImGui::Text("  HSync After VSync Counter: %d\n", cpc->ga.hsync_after_vsync_counter);
        ImGui::Text("  HSync Delay Counter: %d\n", cpc->ga.hsync_delay_counter);
        ImGui::Separator();
        ImGui::Text("CRT:");
        ImGui::Text("  Sync    H: %s  V: %s", cpc->crt.h_sync?"ON ":"OFF", cpc->crt.v_sync?"ON ":"OFF");
        ImGui::Text("  Blank   H: %s  V: %s", cpc->crt.h_blank?"ON ":"OFF", cpc->crt.v_blank?"ON":"OFF");
        ImGui::Text("  Pos     H: %3d  V: %3d", cpc->crt.h_pos, cpc->crt.v_pos);
        ImGui::Text("  Retrace H: %3d  V: %3d", cpc->crt.h_retrace, cpc->crt.v_retrace);
    }
    ImGui::End();
}

#endif /* CHIPS_IMPL */
