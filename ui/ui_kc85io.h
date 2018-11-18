#pragma once
/*#
    # ui_kc85io.h

    Debug visualization for the KC85 IO ports.

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
        - kc85.h

    Include the following headers before including the *implementation*:
        - imgui.h
        - kc85.h

    All strings provided to ui_kc85io_init() must remain alive until
    ui_kc85io_discard() is called!

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

/* setup params for ui_kc85io_init()

    NOTE: all strings must remain alive until ui_kc85io_discard();
*/
typedef struct {
    const char* title;      /* window title */
    kc85_t* kc85;           /* pointer to kc85_t instance to track */
    int x, y;               /* initial position */
} ui_kc85io_desc_t;

typedef struct {
    const char* title;
    kc85_t* kc85;
    int init_x, init_y;
    bool open;
    bool valid;
} ui_kc85io_t;

void ui_kc85io_init(ui_kc85io_t* win, ui_kc85io_desc_t* desc);
void ui_kc85io_discard(ui_kc85io_t* win);
void ui_kc85io_open(ui_kc85io_t* win);
void ui_kc85io_close(ui_kc85io_t* win);
void ui_kc85io_toggle(ui_kc85io_t* win);
bool ui_kc85io_isopen(ui_kc85io_t* win);
void ui_kc85io_draw(ui_kc85io_t* win);

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

void ui_kc85io_init(ui_kc85io_t* win, ui_kc85io_desc_t* desc) {
    CHIPS_ASSERT(win && desc);
    CHIPS_ASSERT(desc->title);
    CHIPS_ASSERT(desc->kc85);
    memset(win, 0, sizeof(ui_kc85io_t));
    win->title = desc->title;
    win->kc85 = desc->kc85;
    win->init_x = desc->x;
    win->init_y = desc->y;
    win->valid = true;
}

void ui_kc85io_discard(ui_kc85io_t* win) {
    CHIPS_ASSERT(win && win->valid);
    win->valid = false;
}

void ui_kc85io_open(ui_kc85io_t* win) {
    CHIPS_ASSERT(win && win->valid);
    win->open = true;
}

void ui_kc85io_close(ui_kc85io_t* win) {
    CHIPS_ASSERT(win && win->valid);
    win->open = false;
}

void ui_kc85io_toggle(ui_kc85io_t* win) {
    CHIPS_ASSERT(win && win->valid);
    win->open = !win->open;
}

bool ui_kc85io_isopen(ui_kc85io_t* win) {
    CHIPS_ASSERT(win && win->valid);
    return win->open;
}

void ui_kc85io_draw(ui_kc85io_t* win) {
    CHIPS_ASSERT(win && win->valid && win->kc85);
    if (!win->open) {
        return;
    }
    ImGui::SetNextWindowPos(ImVec2(win->init_x, win->init_y), ImGuiSetCond_Once);
    ImGui::SetNextWindowSize(ImVec2(200, 200), ImGuiSetCond_Once);
    if (ImGui::Begin(win->title, &win->open)) {
        ImGui::Text("FIXME!");
    }
    ImGui::End();
}
#endif /* CHIPS_IMPL */
