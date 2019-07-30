#pragma once
/*#
    # ui_am40010.h

    Amstrad gate array debug UI.

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
        - am40010.h
        - ui_chip.h

    Include the following headers before including the *implementation*:
        - imgui.h
        - am40010.h
        - ui_chip.h
        - ui_util.h

    All string data provided to the ui_am40010_init() must remain alive until
    until ui_am40010_discard() is called!

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

/* setup parameters for ui_am40010_init()
    NOTE: all string data must remain alive until ui_i8255_discard() is called!
*/
typedef struct ui_am40010_desc_t {
    const char* title;          /* window title */
    am40010_t* am40010;         /* pointer to am40010_t instance to track */
    int x, y;                   /* initial window position */
    int w, h;                   /* initial window size, or 0 for default size */
    bool open;                  /* initial open state */
    ui_chip_desc_t chip_desc;   /* chips visualization desc */
} ui_am40010_desc_t;

typedef struct ui_am40010_t {
    const char* title;
    am40010_t* am40010;
    float init_x, init_y;
    float init_w, init_h;
    bool open;
    bool valid;
    ui_chip_t chip;
} ui_am40010_t;

void ui_am40010_init(ui_am40010_t* win, ui_am40010_desc_t* desc);
void ui_am40010_discard(ui_am40010_t* win);
void ui_am40010_draw(ui_am40010_t* win);

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

void ui_am40010_init(ui_am40010_t* win, ui_am40010_desc_t* desc) {
    CHIPS_ASSERT(win && desc);
    CHIPS_ASSERT(desc->title);
    CHIPS_ASSERT(desc->am40010);
    memset(win, 0, sizeof(ui_am40010_t));
    win->title = desc->title;
    win->am40010 = desc->am40010;
    win->init_x = (float) desc->x;
    win->init_y = (float) desc->y;
    win->init_w = (float) ((desc->w == 0) ? 440 : desc->w);
    win->init_h = (float) ((desc->h == 0) ? 300 : desc->h);
    win->open = desc->open;
    win->valid = true;
    ui_chip_init(&win->chip, &desc->chip_desc);
}

void ui_am40010_discard(ui_am40010_t* win) {
    CHIPS_ASSERT(win && win->valid);
    win->valid = false;
}

static void _ui_am40010_draw_hw_colors(ui_am40010_t* win) {
    am40010_colors_t* c = &win->am40010->colors;
    ImGui::Text("Hardware Colors:");
    const ImVec2 size(18,18);
    for (int i = 0; i < 32; i++) {
        ImGui::PushID(i);
        ImGui::ColorButton("##hw_color", ImColor(c->hw_rgba8[i]), ImGuiColorEditFlags_NoAlpha, size);
        ImGui::PopID();
        if (((i+1) % 8) != 0) {
            ImGui::SameLine();
        }
    }
}

static void _ui_am40010_draw_ink_colors(ui_am40010_t* win) {
    am40010_colors_t* c = &win->am40010->colors;
    ImGui::Text("Ink Colors:");
    const ImVec2 size(18,18);
    for (int i = 0; i < 16; i++) {
        ImGui::PushID(128 + i);
        ImGui::ColorButton("##ink_color", ImColor(c->ink_rgba8[i]), ImGuiColorEditFlags_NoAlpha, size);
        ImGui::PopID();
        if (((i+1) % 8) != 0) {
            ImGui::SameLine();
        }
    }
}

static void _ui_am40010_draw_border_color(ui_am40010_t* win) {
    am40010_colors_t* c = &win->am40010->colors;
    ImGui::Text("Border Color:");
    const ImVec2 size(18,18);
    ImGui::ColorButton("##brd_color", ImColor(c->border_rgba8), ImGuiColorEditFlags_NoAlpha, size);
}

static void _ui_am40010_draw_registers(ui_am40010_t* win) {
    am40010_registers_t* r = &win->am40010->regs;
    ImGui::Text("INKSEL %02X", r->inksel);
    ImGui::Text("BORDER %02X", r->border);
    ImGui::Text("INK   "); ImGui::SameLine();
    for (int i = 0; i < 16; i++) {
        ImGui::Text("%02X", r->ink[i]);
        if (((i+1) % 8) != 0) {
            ImGui::SameLine();
        }
        else if (i < 15) {
            ImGui::Text("      "); ImGui::SameLine();
        }
    }
    ImGui::Text("CONFIG %02X", r->config);
    ImGui::Text("  Mode     %d", r->config & 3);
    ImGui::Text("  LoROM    %s", (r->config & AM40010_CONFIG_LROMEN) ? "ON":"OFF");
    ImGui::Text("  HiROM    %s", (r->config & AM40010_CONFIG_HROMEN) ? "ON":"OFF");
    ImGui::Text("  IRQRes   %s", (r->config & AM40010_CONFIG_IRQRESET) ? "ON":"OFF");
}

static void _ui_am40010_draw_sync_irq(ui_am40010_t* win) {
    am40010_video_t* v = &win->am40010->video;
    ImGui::Text("Mode    %d", v->mode);
    ImGui::Text("IntCnt  %02X", v->intcnt);
    ImGui::Text("HSCount %02X", v->hscount);
    ImGui::Text("ClkCnt  %02X", v->clkcnt);
    ImGui::Text("Sync    %s", v->sync ? "ON":"OFF");
    ImGui::Text("IRQ     %s", v->intr ? "ON":"OFF");
}

static void _ui_am40010_draw_video(ui_am40010_t* win) {
    am40010_crt_t* crt = &win->am40010->crt;
    uint64_t crtc_pins = win->am40010->crtc_pins;
    ImGui::Text("h_pos %X", crt->h_pos);
    ImGui::Text("v_pos %X", crt->v_pos);
    const uint16_t addr = ((crtc_pins & 0x3000) << 2) |     /* MA13,MA12 */
                          ((crtc_pins & 0x3FF) << 1) |      /* MA9..MA0 */
                          (((crtc_pins>>48) & 7) << 11);    /* RA0..RA2 */
    ImGui::Text("addr  %04X", addr);
}

static void _ui_am40010_tint_framebuffer(ui_am40010_t* win) {
    const int num = AM40010_DBG_DISPLAY_WIDTH * AM40010_DBG_DISPLAY_HEIGHT;
    uint32_t* ptr = win->am40010->rgba8_buffer;
    for (int i = 0; i < num; i++) {
        ptr[i] = ~ptr[i] | 0xFF0000F0;
    }
}

static void _ui_am40010_draw_state(ui_am40010_t* win) {
    ImGui::Checkbox("Debug Visualization", &win->am40010->dbg_vis);
    if (ImGui::Button("Tint Framebuffer")) {
        _ui_am40010_tint_framebuffer(win);
    }
    if (ImGui::CollapsingHeader("Colors", ImGuiTreeNodeFlags_DefaultOpen)) {
        _ui_am40010_draw_hw_colors(win);
        _ui_am40010_draw_ink_colors(win);
        _ui_am40010_draw_border_color(win);
    }
    if (ImGui::CollapsingHeader("Registers", ImGuiTreeNodeFlags_DefaultOpen)) {
        _ui_am40010_draw_registers(win);
    }
    if (ImGui::CollapsingHeader("Sync & IRQ", ImGuiTreeNodeFlags_DefaultOpen)) {
        _ui_am40010_draw_sync_irq(win);
    }
    if (ImGui::CollapsingHeader("Display", ImGuiTreeNodeFlags_DefaultOpen)) {
        _ui_am40010_draw_video(win);
    }
}

void ui_am40010_draw(ui_am40010_t* win) {
    CHIPS_ASSERT(win && win->valid && win->title && win->am40010);
    if (!win->open) {
        return;
    }
    ImGui::SetNextWindowPos(ImVec2(win->init_x, win->init_y), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(win->init_w, win->init_h), ImGuiCond_Once);
    if (ImGui::Begin(win->title, &win->open)) {
        ImGui::BeginChild("##chip", ImVec2(176, 0), true);
        ui_chip_draw(&win->chip, win->am40010->pins);
        ImGui::EndChild();
        ImGui::SameLine();
        ImGui::BeginChild("##state", ImVec2(0, 0), true);
        _ui_am40010_draw_state(win);
        ImGui::EndChild();
    }
    ImGui::End();
}

#endif /* CHIPS_IMPL */
