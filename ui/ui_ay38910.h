#pragma once
/*#
    # ui_ay38910.h

    Debug visualization for AY-3-8910 sound chip.

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
        - ay38910.h
        - ui_chip.h

    Include the following headers before including the *implementation*:
        - imgui.h
        - ay38910.h
        - ui_chip.h
        - ui_util.h

    All string data provided to the ui_ay38910_init() must remain alive until
    until ui_ay38910_discard() is called!

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

/* setup parameters for ui_ay38910_init()
    NOTE: all string data must remain alive until ui_z80_discard()!
*/
typedef struct {
    const char* title;          /* window title */
    ay38910_t* ay;              /* pointer to ay38910_t instance to track */
    int x, y;                   /* initial window pos */
    int w, h;               /* initial window width and height */
    bool open;                  /* initial open state */
    ui_chip_desc_t chip_desc;   /* chip visualization desc */
} ui_ay38910_desc_t;

typedef struct {
    const char* title;
    ay38910_t* ay;
    int init_x, init_y;
    int init_w, init_h;
    bool open;
    bool valid;
    ui_chip_t chip;
} ui_ay38910_t;

void ui_ay38910_init(ui_ay38910_t* win, const ui_ay38910_desc_t* desc);
void ui_ay38910_discard(ui_ay38910_t* win);
void ui_ay38190_draw(ui_ay38910_t* win);

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

void ui_ay38910_init(ui_ay38910_t* win, const ui_ay38910_desc_t* desc) {
    CHIPS_ASSERT(win && desc);
    CHIPS_ASSERT(desc->title);
    CHIPS_ASSERT(desc->ay);
    memset(win, 0, sizeof(ui_ay38910_t));
    win->title = desc->title;
    win->ay = desc->ay;
    win->init_x = desc->x;
    win->init_y = desc->y;
    win->init_w = desc->w;
    win->init_h = desc->h;
    win->open = desc->open;
    win->valid = true;
    ui_chip_init(&win->chip, &desc->chip_desc);
}

void ui_ay38910_discard(ui_ay38910_t* win) {
    CHIPS_ASSERT(win && win->valid);
    win->valid = false;
}

void ui_ay38190_draw(ui_ay38910_t* win) {
    CHIPS_ASSERT(win && win->valid && win->ay);
    if (!win->open) {
        return;
    }
    ImGui::SetNextWindowPos(ImVec2(win->init_x, win->init_y), ImGuiSetCond_Once);
    ImGui::SetNextWindowSize(ImVec2(win->init_w, win->init_h), ImGuiSetCond_Once);
    if (ImGui::Begin(win->title, &win->open)) {
        ImGui::BeginChild("##ay_chip", ImVec2(176, 0), true);
        ui_chip_draw(&win->chip, win->ay->pins);
        ImGui::EndChild();
    }
    ImGui::End();
}
#endif /* CHIPS_IMPL */
