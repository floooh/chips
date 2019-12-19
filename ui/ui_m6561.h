#pragma once
/*#
    # ui_m6561.h

    Debug visualization for m6561.h (VIC-I)

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
        - m6561.h
        - ui_chip.h

    Include the following headers before including the *implementation*:
        - imgui.h
        - m6561.h
        - ui_util.h
        - ui_chip.h

    All strings provided to ui_m6561_init() must remain alive until
    ui_m6561_discard() is called!

    ## zlib/libpng license

    Copyright (c) 2019 Andre Weissflog
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

/* setup parameters for ui_m6561_init()
    NOTE: all string data must remain alive until ui_m6561_discard()!
*/
typedef struct ui_m6561_desc_t {
    const char* title;          /* window title */
    m6561_t* vic;               /* pointer to m6561_t instance to track */
    int x, y;                   /* initial window pos */
    int w, h;                   /* initial window size (default size if 0) */
    bool open;                  /* initial open state */
    ui_chip_desc_t chip_desc;   /* chip visualization desc */
} ui_m6561_desc_t;

typedef struct ui_m6561_t {
    const char* title;
    m6561_t* vic;
    float init_x, init_y;
    float init_w, init_h;
    bool open;
    bool valid;
    ui_chip_t chip;
} ui_m6561_t;

void ui_m6561_init(ui_m6561_t* win, const ui_m6561_desc_t* desc);
void ui_m6561_discard(ui_m6561_t* win);
void ui_m6561_draw(ui_m6561_t* win);

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

void ui_m6561_init(ui_m6561_t* win, const ui_m6561_desc_t* desc) {
    CHIPS_ASSERT(win && desc);
    CHIPS_ASSERT(desc->title);
    CHIPS_ASSERT(desc->vic);
    memset(win, 0, sizeof(ui_m6561_t));
    win->title = desc->title;
    win->vic = desc->vic;
    win->init_x = (float) desc->x;
    win->init_y = (float) desc->y;
    win->init_w = (float) ((desc->w == 0) ? 496 : desc->w);
    win->init_h = (float) ((desc->h == 0) ? 416 : desc->h);
    win->open = desc->open;
    win->valid = true;
    ui_chip_init(&win->chip, &desc->chip_desc);
}

void ui_m6561_discard(ui_m6561_t* win) {
    CHIPS_ASSERT(win && win->valid);
    win->valid = false;
}

static void _ui_m6561_draw_hwcolors(ui_m6561_t* win) {
    if (ImGui::CollapsingHeader("Hardware Colors")) {
        ImVec4 c;
        const ImVec2 size(18,18);
        for (int i = 0; i < 16; i++) {
            c = ImColor(m6561_color(i));
            ImGui::PushID(i);
            ImGui::ColorButton("##hw_color", c, ImGuiColorEditFlags_NoAlpha, size);
            ImGui::PopID();
            if (((i+1) % 8) != 0) {
                ImGui::SameLine();
            }
        }
    }
}

static void _ui_m6561_draw_registers(const ui_m6561_t* win) {
    if (ImGui::CollapsingHeader("Registers")) {
        for (int i = 0; i < M6561_NUM_REGS; i++) {
            ImGui::Text("CR%X: %02X", i, win->vic->regs[i]);
        }
    }
}

static void _ui_m6561_draw_raster_unit(const ui_m6561_t* win) {
    if (ImGui::CollapsingHeader("Raster Unit")) {
        const m6561_raster_unit_t* rs = &win->vic->rs;
        ImGui::Text("h_count: %d", rs->h_count);
        ImGui::Text("v_count: %d", rs->v_count);
        ImGui::Text("vm_index: %d", rs->vm_index);
        ImGui::Text("vm_base:  %d", rs->vm_base);
        ImGui::Text("cell_count: %d", rs->cell_count);
        ImGui::Text("row_count:  %d", rs->row_count);
        ImGui::Text("h_disp_start: %d", rs->h_disp_start);
        ImGui::Text("h_disp_end:   %d", rs->h_disp_end);
        ImGui::Text("v_disp_start: %d", rs->v_disp_start);
        ImGui::Text("v_disp_end:   %d", rs->v_disp_end);
        ImGui::Text("cell_height:  %d", rs->cell_height);
        ImGui::Text("border:  %d", rs->border);
        ImGui::Text("fetch disable: %d", rs->fetch_disable);
    }
}

static void _ui_m6561_draw_crt(const ui_m6561_t* win) {
    if (ImGui::CollapsingHeader("CRT")) {
        const m6561_crt_t* crt = &win->vic->crt;
        ImGui::Text("x: %3d  y: %3d", crt->x, crt->y);
        ImGui::Text("vis x(%d=>%d) y(%d=>%d)", crt->vis_x0, crt->vis_x1, crt->vis_y0, crt->vis_y1);
    }
}

static void _ui_m6561_tint_framebuffer(ui_m6561_t* win) {
    uint32_t* ptr = win->vic->crt.rgba8_buffer;
    if (ptr) {
        const int num = m6561_display_width(win->vic) * m6561_display_height(win->vic);
        for (int i = 0; i < num; i++) {
            ptr[i] = ~ptr[i] | 0xFF0000F0;
        }
    }
}

void ui_m6561_draw(ui_m6561_t* win) {
    CHIPS_ASSERT(win && win->valid);
    if (!win->open) {
        return;
    }
    ImGui::SetNextWindowPos(ImVec2(win->init_x, win->init_y), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(win->init_w, win->init_h), ImGuiCond_Once);
    if (ImGui::Begin(win->title, &win->open)) {
        ImGui::BeginChild("##m6561_chip", ImVec2(176, 0), true);
        ui_chip_draw(&win->chip, win->vic->pins);
        ImGui::EndChild();
        ImGui::SameLine();
        ImGui::BeginChild("##m6561_state", ImVec2(0, 0), true);
        ImGui::Checkbox("Debug Visualization", &win->vic->debug_vis);
        if (ImGui::Button("Tint Framebuffer")) {
            _ui_m6561_tint_framebuffer(win);
        }
        _ui_m6561_draw_hwcolors(win);
        _ui_m6561_draw_registers(win);
        _ui_m6561_draw_raster_unit(win);
        _ui_m6561_draw_crt(win);
        ImGui::EndChild();
    }
    ImGui::End();
}

#endif /* CHIPS_IMPL */
