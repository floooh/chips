#pragma once
/*#
    # ui_m6569.h

    Debug visualization UI for m6569.h (VIC-II)

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
        - m6569.h
        - ui_chip.h

    Include the following headers before including the *implementation*:
        - imgui.h
        - m6569.h
        - ui_util.h
        - ui_chip.h

    All strings provided to ui_m6569_init() must remain alive until
    ui_m6569_discard() is called!

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

/* setup parameters for ui_m6569_init()
    NOTE: all string data must remain alive until ui_m6569_discard()!
*/
typedef struct ui_m6569_desc_t {
    const char* title;          /* window title */
    m6569_t* vic;               /* pointer to m6569_t instance to track */
    int x, y;                   /* initial window pos */
    int w, h;                   /* initial window size (default size if 0) */
    bool open;                  /* initial open state */
    ui_chip_desc_t chip_desc;   /* chip visualization desc */
} ui_m6569_desc_t;

typedef struct ui_m6569_t {
    const char* title;
    m6569_t* vic;
    float init_x, init_y;
    float init_w, init_h;
    bool open;
    bool valid;
    ui_chip_t chip;
} ui_m6569_t;

void ui_m6569_init(ui_m6569_t* win, const ui_m6569_desc_t* desc);
void ui_m6569_discard(ui_m6569_t* win);
void ui_m6569_draw(ui_m6569_t* win);

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

void ui_m6569_init(ui_m6569_t* win, const ui_m6569_desc_t* desc) {
    CHIPS_ASSERT(win && desc);
    CHIPS_ASSERT(desc->title);
    CHIPS_ASSERT(desc->vic);
    memset(win, 0, sizeof(ui_m6569_t));
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

void ui_m6569_discard(ui_m6569_t* win) {
    CHIPS_ASSERT(win && win->valid);
    win->valid = false;
}

static void _ui_m6569_draw_hwcolors(ui_m6569_t* win) {
    if (ImGui::CollapsingHeader("Hardware Colors")) {
        ImVec4 c;
        const ImVec2 size(18,18);
        for (int i = 0; i < 16; i++) {
            c = ImColor(m6569_color(i));
            ImGui::PushID(i);
            ImGui::ColorButton("##hw_color", c, ImGuiColorEditFlags_NoAlpha, size);
            ImGui::PopID();
            if (((i+1) % 8) != 0) {
                ImGui::SameLine();
            }
        }
    }
}

static void _ui_m6569_regcolor(const char* label, uint8_t val) {
    ImGui::Text("%s%X", label, val); ImGui::SameLine();
    ImGui::ColorButton("##regclr", ImColor(m6569_color(val&0xF)), ImGuiColorEditFlags_NoAlpha, ImVec2(12,12));
}

static void _ui_m6569_draw_registers(ui_m6569_t* win) {
    m6569_t* vic = win->vic;
    if (ImGui::CollapsingHeader("Registers")) {
        for (int i = 0; i < M6569_NUM_MOBS; i++) {
            ImGui::Text("m%dx:%02X  m%dy:%02X", i, vic->reg.mxy[i][0], i, vic->reg.mxy[i][1]);
            if (((i+1) % 2) != 0) {
                ImGui::SameLine(); ImGui::Text("  "); ImGui::SameLine();
            }
        }
        ui_util_b8("mx8:", vic->reg.mx8); ImGui::SameLine();
        ui_util_b8("me:", vic->reg.me);
        ui_util_b8("mye:", vic->reg.mye); ImGui::SameLine();
        ui_util_b8("mxe:", vic->reg.mxe);
        ui_util_b8("mdp:", vic->reg.mdp); ImGui::SameLine();
        ui_util_b8("mmc:", vic->reg.mmc);
        ui_util_b8("mcm:", vic->reg.mcm); ImGui::SameLine();
        ui_util_b8("mcd:", vic->reg.mcd);
        ui_util_b8("ctrl1:", vic->reg.ctrl_1);
            ImGui::Text("  rst8:%d", (0 != (vic->reg.ctrl_1 & M6569_CTRL1_RST8)) ? 1:0); ImGui::SameLine();
            ImGui::Text("ecm:%s", (0 != (vic->reg.ctrl_1 & M6569_CTRL1_ECM)) ? "ON ":"OFF"); ImGui::SameLine();
            ImGui::Text("bmm:%s", (0 != (vic->reg.ctrl_1 & M6569_CTRL1_BMM)) ? "ON ":"OFF");
            ImGui::Text("  den:%s", (0 != (vic->reg.ctrl_1 & M6569_CTRL1_DEN)) ? "ON ":"OFF"); ImGui::SameLine();
            ImGui::Text("rsel:%s", (0 != (vic->reg.ctrl_1 & M6569_CTRL1_RSEL)) ? "ON ":"OFF"); ImGui::SameLine();
            ImGui::Text("yscroll:%d", vic->reg.ctrl_1 & M6569_CTRL1_YSCROLL);
        ui_util_b8("ctrl2:", vic->reg.ctrl_2);
            ImGui::Text("  res:%s", (0 != (vic->reg.ctrl_2 & M6569_CTRL2_RES)) ? "ON ":"OFF"); ImGui::SameLine();
            ImGui::Text("mcm:%s", (0 != (vic->reg.ctrl_2 & M6569_CTRL2_MCM)) ? "ON ":"OFF"); ImGui::SameLine();
            ImGui::Text("csel:%s", (0 != (vic->reg.ctrl_2 & M6569_CTRL2_CSEL)) ? "ON ":"OFF"); ImGui::SameLine();
            ImGui::Text("xscroll:%d", vic->reg.ctrl_2 & M6569_CTRL2_XSCROLL);
        ImGui::Text("raster:%02X  lpx:%02X  lpy:%02X", vic->reg.raster, vic->reg.lightpen_xy[0], vic->reg.lightpen_xy[1]);
        ui_util_b8("memptrs:", vic->reg.mem_ptrs);
            ImGui::Text("  vm:%04X", (((vic->reg.mem_ptrs>>4)&0xF)<<10)); ImGui::SameLine();
            ImGui::Text("cb:%04X", (((vic->reg.mem_ptrs>>1)&0x7)<<11));
        ui_util_b8("int latch:", vic->reg.int_latch); ImGui::SameLine();
        ui_util_b8("mask:", vic->reg.int_mask);
            ImGui::Text("  IRQ:%s", (0 != (vic->reg.int_latch & M6569_INT_IRQ)) ? "ON ":"OFF");
            ImGui::Text("  ILP:%s", (0 != (vic->reg.int_latch & M6569_INT_ILP)) ? "ON ":"OFF"); ImGui::SameLine();
            ImGui::Text("IMMC:%s", (0 != (vic->reg.int_latch & M6569_INT_IMMC)) ? "ON ":"OFF"); ImGui::SameLine();
            ImGui::Text("IMBC:%s", (0 != (vic->reg.int_latch & M6569_INT_IMBC)) ? "ON ":"OFF"); ImGui::SameLine();
            ImGui::Text("IRST:%s", (0 != (vic->reg.int_latch & M6569_INT_IRST)) ? "ON ":"OFF");
            ImGui::Text("  ELP:%s", (0 != (vic->reg.int_mask & M6569_INT_ELP)) ? "ON ":"OFF"); ImGui::SameLine();
            ImGui::Text("EMMC:%s", (0 != (vic->reg.int_mask & M6569_INT_EMMC)) ? "ON ":"OFF"); ImGui::SameLine();
            ImGui::Text("EMBC:%s", (0 != (vic->reg.int_mask & M6569_INT_EMBC)) ? "ON ":"OFF"); ImGui::SameLine();
            ImGui::Text("ERST:%s", (0 != (vic->reg.int_mask & M6569_INT_ERST)) ? "ON ":"OFF");
        _ui_m6569_regcolor("ec:", vic->reg.ec);
        _ui_m6569_regcolor("bc0:", vic->reg.bc[0]); ImGui::SameLine();
        _ui_m6569_regcolor("bc1:", vic->reg.bc[1]); ImGui::SameLine();
        _ui_m6569_regcolor("bc2:", vic->reg.bc[2]); ImGui::SameLine();
        _ui_m6569_regcolor("bc3:", vic->reg.bc[3]);
        _ui_m6569_regcolor("mc0:", vic->reg.mc[0]); ImGui::SameLine();
        _ui_m6569_regcolor("mc1:", vic->reg.mc[1]); ImGui::SameLine();
        _ui_m6569_regcolor("mc2:", vic->reg.mc[2]); ImGui::SameLine();
        _ui_m6569_regcolor("mc3:", vic->reg.mc[3]);
        _ui_m6569_regcolor("mm0:", vic->reg.mm[0]); ImGui::SameLine();
        _ui_m6569_regcolor("mm1:", vic->reg.mm[1]);
    }
}

static void _ui_m6569_draw_raster_unit(ui_m6569_t* win) {
    if (ImGui::CollapsingHeader("Raster Unit")) {
        ImGui::Text("FIXME!");
    }
}

static void _ui_m6569_draw_memory_unit(ui_m6569_t* win) {
    if (ImGui::CollapsingHeader("Memory Unit")) {
        ImGui::Text("FIXME!");
    }
}

static void _ui_m6569_draw_video_matrix(ui_m6569_t* win) {
    if (ImGui::CollapsingHeader("Video Matrix")) {
        ImGui::Text("FIXME!");
    }
}

static void _ui_m6569_draw_border_unit(ui_m6569_t* win) {
    if (ImGui::CollapsingHeader("Border Unit")) {
        ImGui::Text("FIXME!");
    }
}

static void _ui_m6569_draw_graphics_unit(ui_m6569_t* win) {
    if (ImGui::CollapsingHeader("Graphics Unit")) {
        ImGui::Text("FIXME!");
    }
}

static void _ui_m6569_draw_sprite_units(ui_m6569_t* win) {
    static const char* su_names[8] = {
        "Sprite Unit 0", "Sprite Unit 1", "Sprite Unit 2", "Sprite Unit 3",
        "Sprite Unit 4", "Sprite Unit 5", "Sprite Unit 6", "Sprite Unit 7",
    };
    for (int i = 0; i < 8; i++) {
        if (ImGui::CollapsingHeader(su_names[i])) {
            ImGui::Text("FIXME!");
        }
    }
}

void ui_m6569_draw(ui_m6569_t* win) {
    CHIPS_ASSERT(win && win->valid);
    if (!win->open) {
        return;
    }
    ImGui::SetNextWindowPos(ImVec2(win->init_x, win->init_y), ImGuiSetCond_Once);
    ImGui::SetNextWindowSize(ImVec2(win->init_w, win->init_h), ImGuiSetCond_Once);
    if (ImGui::Begin(win->title, &win->open)) {
        ImGui::BeginChild("##m6569_chip", ImVec2(176, 0), true);
        ui_chip_draw(&win->chip, win->vic->pins);
        ImGui::EndChild();
        ImGui::SameLine();
        ImGui::BeginChild("##m6569_state", ImVec2(0, 0), true);
        ImGui::Checkbox("Debug Visualization (FIXME)", &win->vic->debug_vis);
        _ui_m6569_draw_hwcolors(win);
        _ui_m6569_draw_registers(win);
        _ui_m6569_draw_raster_unit(win);
        _ui_m6569_draw_memory_unit(win);
        _ui_m6569_draw_video_matrix(win);
        _ui_m6569_draw_border_unit(win);
        _ui_m6569_draw_graphics_unit(win);
        _ui_m6569_draw_sprite_units(win);
        ImGui::EndChild();
    }
    ImGui::End();
}

#endif /* CHIPS_IMPL */
