#pragma once
/*#
    # ui_c1530.h

    Debugging UI for the c1530.h datassette drive emulation.

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

    Include the following headers (and their dependencies) before including
    ui_c1530.h both for the declaration and implementation:

    - c1530.h

    ## zlib/libpng license

    Copyright (c) 2020 Andre Weissflog
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

/* setup params for ui_c1530_init() */
typedef struct {
    const char* title;      /* window title */
    c1530_t* c1530;         /* pointer to c1530_t instance */
    int x, y;               /* initial window position */
    int w, h;               /* initial window size, or 0 for default size */
    bool open;              /* initial open state */
} ui_c1530_desc_t;

typedef struct {
    const char* title;
    c1530_t* c1530;
    float init_x, init_y;
    float init_w, init_h;
    bool open;
    bool valid;
} ui_c1530_t;

void ui_c1530_init(ui_c1530_t* win, const ui_c1530_desc_t* desc);
void ui_c1530_discard(ui_c1530_t* win);
void ui_c1530_draw(ui_c1530_t* win);

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

void ui_c1530_init(ui_c1530_t* win, const ui_c1530_desc_t* desc) {
    CHIPS_ASSERT(win && desc);
    CHIPS_ASSERT(desc->title);
    CHIPS_ASSERT(desc->c1530);
    memset(win, 0, sizeof(*win));
    win->title = desc->title;
    win->c1530 = desc->c1530;
    win->init_x = (float) desc->x;
    win->init_y = (float) desc->y;
    win->init_w = (float) ((desc->w == 0) ? 200 : desc->w);
    win->init_h = (float) ((desc->h == 0) ? 220 : desc->h);
    win->open = desc->open;
    win->valid = true;
}

void ui_c1530_discard(ui_c1530_t* win) {
    CHIPS_ASSERT(win && win->valid);
    win->valid = false;
    win->c1530 = 0;
}

void ui_c1530_draw(ui_c1530_t* win) {
    CHIPS_ASSERT(win && win->valid);
    if (!win->open) {
        return;
    }
    ImGui::SetNextWindowPos({win->init_x, win->init_y}, ImGuiCond_Once);
    ImGui::SetNextWindowSize({win->init_w, win->init_h}, ImGuiCond_Once);
    if (ImGui::Begin(win->title, &win->open)) {
        c1530_t* sys = win->c1530;
        CHIPS_ASSERT(sys && sys->cas_port);
        if (ImGui::CollapsingHeader("Cassette Port", ImGuiTreeNodeFlags_DefaultOpen)) {
            uint8_t cas_port = *sys->cas_port;
            ImGui::Text("MOTOR: %d", (cas_port & VIC20_CASPORT_MOTOR) ? 1:0);
            ImGui::Text("WRITE: %d", (cas_port & VIC20_CASPORT_WRITE) ? 1:0);
            ImGui::Text("READ:  %d", (cas_port & VIC20_CASPORT_READ) ? 1:0);
            ImGui::Text("SENSE: %d", (cas_port & VIC20_CASPORT_SENSE) ? 1:0);
        }
        if (ImGui::CollapsingHeader("Drive Status", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("Motor: %s", c1530_is_motor_on(sys) ? "ON":"OFF");
            ImGui::Text("Tape:  %s", c1530_tape_inserted(sys) ? "INSERTED":"NONE");
            ImGui::Text("Pos:   %d/%d", sys->pos, sys->size);
            ImGui::Text("Pulse Count: %d", sys->pulse_count);
        }
    }
    ImGui::End();
}

#endif /* CHIPS_IMPL */
