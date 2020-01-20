/*#
    # ui_i81255.h

    Debug visualization for i8255.h

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
        - i8255.h
        - ui_chip.h

    Include the following headers before including the *implementation*:
        - imgui.h
        - i8255.h
        - ui_chip.h
        - ui_util.h

    All string data provided to the ui_i8255_init() must remain alive until
    until ui_i8255_discard() is called!

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

/* setup parameters for ui_i8255_init()
    NOTE: all string data must remain alive until ui_i8255_discard() is called!
*/
typedef struct {
    const char* title;          /* window title */
    i8255_t* i8255;             /* pointer to i8255_t instance to track */
    int x, y;                   /* initial window position */
    int w, h;                   /* initial window size or 0 for default size */
    bool open;                  /* initial open state */
    ui_chip_desc_t chip_desc;   /* chip visualization desc */
} ui_i8255_desc_t;

typedef struct {
    const char* title;
    i8255_t* i8255;
    float init_x, init_y;
    float init_w, init_h;
    bool open;
    bool valid;
    ui_chip_t chip;
} ui_i8255_t;

void ui_i8255_init(ui_i8255_t* win, const ui_i8255_desc_t* desc);
void ui_i8255_discard(ui_i8255_t* win);
void ui_i8255_draw(ui_i8255_t* win);

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

void ui_i8255_init(ui_i8255_t* win, const ui_i8255_desc_t* desc) {
    CHIPS_ASSERT(win && desc);
    CHIPS_ASSERT(desc->title);
    CHIPS_ASSERT(desc->i8255);
    memset(win, 0, sizeof(ui_i8255_t));
    win->title = desc->title;
    win->i8255 = desc->i8255;
    win->init_x = (float) desc->x;
    win->init_y = (float) desc->y;
    win->init_w = (float) ((desc->w == 0) ? 440 : desc->w);
    win->init_h = (float) ((desc->h == 0) ? 370 : desc->h);
    win->open = desc->open;
    win->valid = true;
    ui_chip_init(&win->chip, &desc->chip_desc);
}

void ui_i8255_discard(ui_i8255_t* win) {
    CHIPS_ASSERT(win && win->valid);
    win->valid = false;
}

static void _ui_i8255_draw_state(ui_i8255_t* win) {
    i8255_t* ppi = win->i8255;
    ImGui::Columns(5, "##ppi_ports", false);
    ImGui::SetColumnWidth(0, 64);
    ImGui::SetColumnWidth(1, 32);
    ImGui::SetColumnWidth(2, 32);
    ImGui::SetColumnWidth(3, 32);
    ImGui::SetColumnWidth(4, 32);
    ImGui::NextColumn();
    ImGui::Text("A"); ImGui::NextColumn();
    ImGui::Text("B"); ImGui::NextColumn();
    ImGui::Text("CHI"); ImGui::NextColumn();
    ImGui::Text("CLO"); ImGui::NextColumn();
    ImGui::Separator();
    ImGui::Text("Mode"); ImGui::NextColumn();
    ImGui::Text("%d", (ppi->control & I8255_CTRL_ACHI_MODE) >> 5); ImGui::NextColumn();
    ImGui::Text("%d", (ppi->control & I8255_CTRL_BCLO_MODE) >> 2); ImGui::NextColumn();
    ImGui::Text("%d", (ppi->control & I8255_CTRL_ACHI_MODE) >> 5); ImGui::NextColumn();
    ImGui::Text("%d", (ppi->control & I8255_CTRL_BCLO_MODE) >> 2); ImGui::NextColumn();
    ImGui::Text("In/Out"); ImGui::NextColumn();
    ImGui::Text("%s", ((ppi->control & I8255_CTRL_A) == I8255_CTRL_A_INPUT) ? "IN":"OUT"); ImGui::NextColumn();
    ImGui::Text("%s", ((ppi->control & I8255_CTRL_B) == I8255_CTRL_B_INPUT) ? "IN":"OUT"); ImGui::NextColumn();
    ImGui::Text("%s", ((ppi->control & I8255_CTRL_CHI) == I8255_CTRL_CHI_INPUT) ? "IN":"OUT"); ImGui::NextColumn();
    ImGui::Text("%s", ((ppi->control & I8255_CTRL_CLO) == I8255_CTRL_CLO_INPUT) ? "IN":"OUT"); ImGui::NextColumn();
    ImGui::Text("Output"); ImGui::NextColumn();
    ImGui::Text("%02X", ppi->pa.outp); ImGui::NextColumn();
    ImGui::Text("%02X", ppi->pb.outp); ImGui::NextColumn();
    ImGui::Text("%X", ppi->pc.outp >> 4); ImGui::NextColumn();
    ImGui::Text("%X", ppi->pc.outp & 0xF); ImGui::NextColumn();
    ImGui::Columns(); ImGui::Separator();
    ImGui::Text("Control: %02X", ppi->control);
}

void ui_i8255_draw(ui_i8255_t* win) {
    CHIPS_ASSERT(win && win->valid && win->title && win->i8255);
    if (!win->open) {
        return;
    }
    ImGui::SetNextWindowPos(ImVec2(win->init_x, win->init_y), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(win->init_w, win->init_h), ImGuiCond_Once);
    if (ImGui::Begin(win->title, &win->open)) {
        ImGui::BeginChild("##i8255_chip", ImVec2(176, 0), true);
        ui_chip_draw(&win->chip, win->i8255->pins);
        ImGui::EndChild();
        ImGui::SameLine();
        ImGui::BeginChild("##i8255_state", ImVec2(0, 0), true);
        _ui_i8255_draw_state(win);
        ImGui::EndChild();
    }
    ImGui::End();
}
#endif
