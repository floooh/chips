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
    bool open;                  /* initial open state */
    ui_chip_desc_t chip_desc;   /* chip visualization desc */
} ui_i8255_desc_t;

typedef struct {
    const char* title;
    i8255_t* i8255;
    int init_x, init_y;
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

void ui_i8255_init(ui_i8255_t* win, ui_i8255_desc_t* desc) {
    CHIPS_ASSERT(win && desc);
    CHIPS_ASSERT(desc->title);
    CHIPS_ASSERT(desc->i8255);
    memset(win, 0, sizeof(ui_i8255_t));
    win->title = desc->title;
    win->i8255 = desc->i8255;
    win->init_x = desc->x;
    win->init_y = desc->y;
    win->valid = true;
    ui_chip_init(&win->chip, &desc->chip_desc);
}

void ui_i8255_discard(ui_i8255_t* win) {
    CHIPS_ASSERT(win && win->valid);
    win->valid = false;
}

static void _ui_i8255_draw_state(ui_i8255_t* win) {
    ImGui::Text("FIXME!");
}

void ui_i8255_draw(ui_i8255_t* win) {
    CHIPS_ASSERT(win && win->valid && win->i8255);
    if (!win->open) {
        return;
    }
    ImGui::SetNextWindowPos(ImVec2(win->init_x, win->init_y), ImGuiSetCond_Once);
    ImGui::SetNextWindowSize(ImVec2(440, 370), ImGuiSetCond_Once);
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
