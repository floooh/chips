#pragma once
/*#
    # ui_kc85sys.h

    Debug visualization for the KC85 system state.

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

    All strings provided to ui_kc85sys_init() must remain alive until
    ui_kc85sys_discard() is called!

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

/* setup params for ui_kc85_init()

    NOTE: all strings must remain alive until ui_kc85sys_discard();
*/
typedef struct {
    const char* title;      /* window title */
    kc85_t* kc85;           /* pointer to kc85_t instance to track */
    int x, y;               /* initial position */
    int w, h;               /* initial window size or 0 for default size */
    bool open;              /* initial open state */
} ui_kc85sys_desc_t;

typedef struct {
    const char* title;
    kc85_t* kc85;
    float init_x, init_y;
    float init_w, init_h;
    bool open;
    bool valid;
} ui_kc85sys_t;

void ui_kc85sys_init(ui_kc85sys_t* win, const ui_kc85sys_desc_t* desc);
void ui_kc85sys_discard(ui_kc85sys_t* win);
void ui_kc85sys_draw(ui_kc85sys_t* win);

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

void ui_kc85sys_init(ui_kc85sys_t* win, const ui_kc85sys_desc_t* desc) {
    CHIPS_ASSERT(win && desc);
    CHIPS_ASSERT(desc->title);
    CHIPS_ASSERT(desc->kc85);
    memset(win, 0, sizeof(ui_kc85sys_t));
    win->title = desc->title;
    win->kc85 = desc->kc85;
    win->init_x = (float) desc->x;
    win->init_y = (float) desc->y;
    win->init_w = (float) ((desc->w == 0) ? 200 : desc->w);
    win->init_h = (float) ((desc->h == 0) ? 400 : desc->h);
    win->open = desc->open;
    win->valid = true;
}

void ui_kc85sys_discard(ui_kc85sys_t* win) {
    CHIPS_ASSERT(win && win->valid);
    win->valid = false;
}

void ui_kc85sys_draw(ui_kc85sys_t* win) {
    CHIPS_ASSERT(win && win->valid && win->title && win->kc85);
    if (!win->open) {
        return;
    }
    ImGui::SetNextWindowPos(ImVec2(win->init_x, win->init_y), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(win->init_w, win->init_h), ImGuiCond_Once);
    if (ImGui::Begin(win->title, &win->open)) {
        if (ImGui::CollapsingHeader("Port 88h (PIO A)", ImGuiTreeNodeFlags_DefaultOpen)) {
            const uint8_t v = win->kc85->pio_a;
            ImGui::Text("0: CAOS ROM E    %s", (v & KC85_PIO_A_CAOS_ROM) ? "ON":"OFF");
            ImGui::Text("1: RAM0          %s", (v & KC85_PIO_A_RAM) ? "ON":"OFF");
            ImGui::Text("2: IRM           %s", (v & KC85_PIO_A_IRM) ? "ON":"OFF");
            ImGui::Text("3: RAM0          %s", (v & KC85_PIO_A_RAM_RO) ? "R/W":"R/O");
            ImGui::Text("4: unused");
            ImGui::Text("5: Tape LED      %s", (v & KC85_PIO_A_TAPE_LED) ? "ON":"OFF");
            ImGui::Text("6: Tape Motor    %s", (v & KC85_PIO_A_TAPE_MOTOR) ? "ON":"OFF");
            if (KC85_TYPE_2 == win->kc85->type) {
                ImGui::Text("7: unused");
            }
            else {
                ImGui::Text("7: BASIC ROM     %s", (v & KC85_PIO_A_BASIC_ROM) ? "ON":"OFF");
            }
        }
        if (ImGui::CollapsingHeader("Port 89h (PIO B)", ImGuiTreeNodeFlags_DefaultOpen)) {
            const uint8_t v = win->kc85->pio_b;
            ImGui::Text("0..4: Volume     %02Xh", (v & 0x1F));
            if (KC85_TYPE_4 == win->kc85->type) {
                ImGui::Text("5: RAM8          %s", (v & KC85_PIO_B_RAM8) ? "ON":"OFF");
                ImGui::Text("6: RAM8          %s", (v & KC85_PIO_B_RAM8_RO) ? "R/W":"R/O");
            }
            else {
                ImGui::Text("5..6: unused");
            }
            ImGui::Text("7: Blinking      %s", (v & KC85_PIO_B_BLINK_ENABLED) ? "ON":"OFF");
        }
        if (KC85_TYPE_4 == win->kc85->type) {
            if (ImGui::CollapsingHeader("Port 84h", ImGuiTreeNodeFlags_DefaultOpen)) {
                const uint8_t v = win->kc85->io84;
                ImGui::Text("0: Show image    %d", (v & KC85_IO84_SEL_VIEW_IMG) ? 0:1);
                ImGui::Text("1: Access        %s", (v & KC85_IO84_SEL_CPU_COLOR) ? "PIXELS":"COLORS");
                ImGui::Text("2: Access image  %d", (v & KC85_IO84_SEL_CPU_IMG) ? 0:1);
                ImGui::Text("3: Hicolor mode  %s", (v & KC85_IO84_HICOLOR) ? "OFF":"ON");
                ImGui::Text("4: RAM8 block    %d", (v & KC85_IO84_SEL_RAM8) ? 0:1);
                ImGui::Text("5: RAM8 ???      %d", (v & KC85_IO84_BLOCKSEL_RAM8) ? 0:1);
                ImGui::Text("6..7: unused");
            }
            if (ImGui::CollapsingHeader("Port 86h", ImGuiTreeNodeFlags_DefaultOpen)) {
                const uint8_t v = win->kc85->io86;
                ImGui::Text("0: RAM4          %s", (v & KC85_IO86_RAM4) ? "ON":"OFF");
                ImGui::Text("1: RAM4          %s", (v & KC85_IO86_RAM4_RO) ? "R/W":"R/O");
                ImGui::Text("2..6: unused");
                ImGui::Text("7: CAOS ROM C    %s", (v & KC85_IO86_CAOS_ROM_C) ? "ON":"OFF");
            }
        }
        if (ImGui::CollapsingHeader("Display", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("Vert Count:    %d", win->kc85->v_count);
            ImGui::Text("Hori Tick:     %d", win->kc85->h_tick);
        }
    }
    ImGui::End();
}
#endif /* CHIPS_IMPL */
