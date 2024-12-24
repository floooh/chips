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
    chips_dim_t dim;            // framebuffer width/height
    chips_rect_t screen;        // visible area
    chips_dim_t pixel_aspect;   // pixel aspect
    bool portrait;
    bool origin_top_left;
} ui_display_frame_t;

typedef struct {
    const char* title;
    float init_x, init_y;
    float init_w, init_h;
    bool open;
    bool last_open;
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

typedef struct {
    ImVec2 v[4];
} ui_display_quad_t;

void ui_display_init(ui_display_t* win, const ui_display_desc_t* desc) {
    CHIPS_ASSERT(win && desc);
    CHIPS_ASSERT(desc->title);
    memset(win, 0, sizeof(ui_display_t));
    win->title = desc->title;
    win->init_x = (float) desc->x;
    win->init_y = (float) desc->y;
    win->init_w = (float) ((desc->w == 0) ? 320 + 20 : desc->w);
    win->init_h = (float) ((desc->h == 0) ? 256 + 20 : desc->h);
    win->open = win->last_open = desc->open;
    win->valid = true;
}

void ui_display_discard(ui_display_t* win) {
    CHIPS_ASSERT(win && win->valid);
    win->valid = false;
}

static ui_display_quad_t ui_display_uv_quad(bool origin_top_left, bool portrait) {
    ui_display_quad_t res = {};
    res.v[0] = { 0, 0 };
    res.v[1] = { 1, 0 };
    res.v[2] = { 1, 1 };
    res.v[3] = { 0, 1 };
    if (origin_top_left) {
        res.v[0].y = res.v[1].y = 1;
        res.v[2].y = res.v[3].y = 0;
    }
    if (portrait) {
        ImVec2 v3 = res.v[3];
        res.v[3] = res.v[2];
        res.v[2] = res.v[1];
        res.v[1] = res.v[0];
        res.v[0] = v3;
    }
    return res;
}

static ui_display_quad_t ui_display_pos_quad(ImVec2 dim, ImVec2 aspect) {
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const ImVec2 region = ImGui::GetContentRegionAvail();
    float cw = region.x;
    if (cw < 1.0f) {
        cw = 1.0f;
    }
    float ch = region.y;
    if (ch < 1.0f) {
        ch = 1.0f;
    }
    const float canvas_aspect = cw / ch;
    const float view_aspect = (dim.x * aspect.x) / (dim.y * aspect.y);
    float vp_x, vp_y, vp_w, vp_h;
    if (view_aspect < canvas_aspect) {
        vp_y = pos.y;
        vp_h = ch;
        vp_w = ch * view_aspect;
        vp_x = pos.x + (cw - vp_w) * 0.5f;
    } else {
        vp_x = pos.x;
        vp_w = cw;
        vp_h = cw / view_aspect;
        vp_y = pos.y + (ch - vp_h) * 0.5f;
    }
    const float x0 = vp_x;
    const float y0 = vp_y;
    const float x1 = vp_x + vp_w;
    const float y1 = vp_y + vp_h;
    ui_display_quad_t res = {};
    res.v[0] = { x0, y0 };
    res.v[1] = { x1, y0 };
    res.v[2] = { x1, y1 };
    res.v[3] = { x0, y1 };
    return res;
}

void ui_display_draw(ui_display_t* win, const ui_display_frame_t* frame) {
    CHIPS_ASSERT(win && frame && win->valid && win->title);
    ui_util_handle_window_open_dirty(&win->open, &win->last_open);
    if (!win->open) {
        return;
    }
    const ImVec2 dim = { (float)frame->screen.width, (float)frame->screen.height };
    const ImVec2 pixel_aspect = { (float)frame->pixel_aspect.width, (float)frame->pixel_aspect.height };
    ImGui::SetNextWindowPos({win->init_x, win->init_y}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize({win->init_w, win->init_h}, ImGuiCond_FirstUseEver);
    if (ImGui::Begin(win->title, &win->open, ImGuiWindowFlags_HorizontalScrollbar|ImGuiWindowFlags_NoNav)) {
        // need to render the image via ImDrawList because we need to specify 4 uv coords
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const ui_display_quad_t p = ui_display_pos_quad(dim, pixel_aspect);
        const ui_display_quad_t uv = ui_display_uv_quad(frame->origin_top_left, frame->portrait);
        dl->AddImageQuad(frame->tex, p.v[0], p.v[1], p.v[2], p.v[3], uv.v[0], uv.v[1], uv.v[2], uv.v[3], 0xFFFFFFFF);
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
