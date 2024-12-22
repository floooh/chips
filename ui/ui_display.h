#pragma once
/*#
    # ui_display.h

    Display the emulator framebuffer in a window.

    Do this:
    ~~~C
    #define CHIPS_UI_IMPL
    ~~~
    before you include this file in *one* C++ file to create the
    implementation.

    Optionally provide the following macros with your own implementation

    ~~~C
    CHIPS_ASSERT(c)
    ~~~
        your own assert macro (default: assert(c))

    Include the following headers before including the *implementation*:
        - imgui.h
        - ui_util.h
        - ui_settings.h
        - chips_common.h

    ## zlib/libpng license

    Copyright (c) 2024 Andre Weissflog
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

typedef struct {
    const char* title;          // window title
    int x, y;                   // initial window position
    int w, h;                   // initial window size of 0 for default size
    bool open;                  // initial open state
} ui_display_desc_t;

typedef struct {
    ui_texture_t tex;
    bool portrait;
    bool origin_top_left;
} ui_display_frame_t;

typedef struct {
    const char* title;
    float init_x, init_y;
    float init_w, init_h;
    bool open;
    bool valid;
} ui_display_t;

void ui_display_init(ui_display_t* win, const ui_display_desc_t* desc);
void ui_display_discard(ui_display_t* win);
void ui_display_draw(ui_display_t* win, const ui_display_frame_t* frame);
void ui_display_save_settings(ui_display_t* win, ui_settings_t* settings);
void ui_display_load_settings(ui_display_t* win, const ui_settings_t* settings);

#ifdef __cplusplus
} // extern "C"
#endif

/*-- IMPLEMENTATION (include in C++ source) ----------------------------------*/
#ifdef CHIPS_UI_IMPL
#ifndef __cplusplus
#error "implementation must be compiled as C++"
#endif
#include <string.h> /* memset */
#ifndef CHIPS_ASSERT
    #include <assert.h>
    #define CHIPS_ASSERT(c) assert(c)
#endif

void ui_display_init(ui_display_t* win, const ui_display_desc_t* desc) {
    CHIPS_ASSERT(win && desc);
    CHIPS_ASSERT(desc->title);
    memset(win, 0, sizeof(ui_display_t));
    win->title = desc->title;
    win->init_x = (float) desc->x;
    win->init_y = (float) desc->y;
    win->init_w = (float) ((desc->w == 0) ? 320 : desc->w);
    win->init_h = (float) ((desc->h == 0) ? 256 : desc->h);
    win->open = desc->open;
    win->valid = true;
}

void ui_display_discard(ui_display_t* win) {
    CHIPS_ASSERT(win && win->valid);
    win->valid = false;
}

void ui_display_draw(ui_display_t* win, const ui_display_frame_t* frame) {
    CHIPS_ASSERT(win && frame && win->valid && win->title);
    if (!win->open) {
        return;
    }
    ImGui::SetNextWindowPos({win->init_x, win->init_y}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize({win->init_w, win->init_h}, ImGuiCond_FirstUseEver);
    if (ImGui::Begin(win->title, &win->open, ImGuiWindowFlags_HorizontalScrollbar)) {
        // need to render the image via ImDrawList because we need to specific 4 uv coords
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const ImVec2 pos = ImGui::GetCursorScreenPos();
        const ImVec2 region = ImGui::GetContentRegionAvail();
        const ImVec2 p0 = pos;
        const ImVec2 p1 = { pos.x + region.x, pos.y + region.y };
        const ImVec2 p[4] = {
            { p0.x, p0.y },
            { p1.x, p0.y },
            { p1.x, p1.y },
            { p0.x, p1.y },
        };
        ImVec2 uv[4] = {
            { 0, 0 },
            { 1, 0 },
            { 1, 1 },
            { 0, 1 },
        };
        if (frame->origin_top_left) {
            uv[0].y = uv[1].y = 1;
            uv[2].y = uv[3].y = 0;
        }
        if (frame->portrait) {
            ImVec2 uv0 = uv[0];
            uv[0] = uv[1];
            uv[1] = uv[2];
            uv[2] = uv[3];
            uv[3] = uv0;
        }
        dl->AddImageQuad(frame->tex, p[0], p[1], p[2], p[3], uv[0], uv[1], uv[2], uv[3], 0xFFFFFFFF);
    }
    ImGui::End();
}

void ui_display_save_settings(ui_display_t* win, ui_settings_t* settings) {
    CHIPS_ASSERT(win && settings);
    ui_settings_add(settings, win->title, win->open);
}

void ui_display_load_settings(ui_display_t* win, const ui_settings_t* settings) {
    CHIPS_ASSERT(win && settings);
    win->open = ui_settings_isopen(settings, win->title);
}

#endif // CHIPS_UI_IMPL
