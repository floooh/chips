#pragma once
/*#
    # ui_z80pio.h

    Debug visualization UI for the Z80 PIO.

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

    You need to include the following headers before including the
    *implementation*:

        - imgui.h
        - z80pio.h
        - ui_util.h

    All strings provided to ui_z80pio_init() must remain alive until
    ui_z80pio_discard() is called!

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

/* setup parameeters for ui_z80pio_init()

    NOTE: all string data must remain alive until ui_z80pio_discard()!
*/
typedef struct {
    const char* title;  /* window title */
    z80pio_t* pio;      /* pointer to PIO to track */
    int x, y;           /* initial window position */
} ui_z80pio_desc_t;


typedef struct {
    const char* title;
    z80pio_t* pio;
    int init_x, init_y;
    bool open;
    bool valid;
} ui_z80pio_t;

void ui_z80pio_init(ui_z80pio_t* win, ui_z80pio_desc_t* desc);
void ui_z80pio_discard(ui_z80pio_t* win);
void ui_z80pio_open(ui_z80pio_t* win);
void ui_z80pio_close(ui_z80pio_t* win);
void ui_z80pio_toggle(ui_z80pio_t* win);
bool ui_z80pio_isopen(ui_z80pio_t* win);
void ui_z80pio_draw(ui_z80pio_t* win);

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

void ui_z80pio_init(ui_z80pio_t* win, ui_z80pio_desc_t* desc) {
    CHIPS_ASSERT(win && desc);
    CHIPS_ASSERT(desc->title);
    CHIPS_ASSERT(desc->pio);
    memset(win, 0, sizeof(ui_z80pio_t));
    win->title = desc->title;
    win->pio = desc->pio;
    win->init_x = desc->x;
    win->init_y = desc->y;
    win->valid = true;
}

void ui_z80pio_discard(ui_z80pio_t* win) {
    CHIPS_ASSERT(win && win->valid);
    win->valid = false;
}

void ui_z80pio_open(ui_z80pio_t* win) {
    CHIPS_ASSERT(win && win->valid);
    win->open = true;
}

void ui_z80pio_close(ui_z80pio_t* win) {
    CHIPS_ASSERT(win && win->valid);
    win->open = false;
}

void ui_z80pio_toggle(ui_z80pio_t* win) {
    CHIPS_ASSERT(win && win->valid);
    win->open = !win->open;
}

bool ui_z80pio_isopen(ui_z80pio_t* win) {
    CHIPS_ASSERT(win && win->valid);
    return win->open;
}

static const char* _ui_z80pio_mode_str(uint8_t mode) {
    switch (mode) {
        case 0: return "OUTPUT";
        case 1: return "INPUT";
        case 2: return "BIDIRECTIONAL";
        case 3: return "BITCONTROL";
        default: return "INVALID";
    }
}

static void _ui_z80pio_port(ui_z80pio_t* win, int port_id) {
    z80pio_port_t* p = &win->pio->port[port_id];
    ui_util_u8("Mode:         ", p->mode); ImGui::SameLine(); ImGui::Text("(%s)", _ui_z80pio_mode_str(p->mode));
    ui_util_u8("Output:       ", p->output);
    ui_util_u8("Input:        ", p->input);
    ui_util_b8("IO Select:    ", p->io_select);
    ui_util_u8("INT Control:  ", p->int_control);
    ImGui::Text("    Enabled:  %s", (p->int_control & Z80PIO_INTCTRL_EI) ? "ENABLED":"DISABLED");
    ImGui::Text("    And/Or:   %s", (p->int_control & Z80PIO_INTCTRL_ANDOR) ? "AND":"OR");
    ImGui::Text("    High/Low: %s", (p->int_control & Z80PIO_INTCTRL_HILO) ? "HIGH":"LOW");
    ui_util_u8("INT Vector:   ", p->int_vector);
    ui_util_b8("INT Mask:     ", p->int_mask);
    const char* exp = "???";
    if (p->expect_int_mask) exp = "MASK";
    else if (p->expect_io_select) exp = "IOSELECT";
    else exp = "ANY";
    ImGui::Text("Expect:       %s", exp);
}

void ui_z80pio_draw(ui_z80pio_t* win) {
    CHIPS_ASSERT(win && win->valid && win->pio);
    if (!win->open) {
        return;
    }
    ImGui::SetNextWindowPos(ImVec2(win->init_x, win->init_y), ImGuiSetCond_Once);
    ImGui::SetNextWindowSize(ImVec2(220, 420), ImGuiSetCond_Once);
    if (ImGui::Begin(win->title, &win->open)) {
        if (ImGui::CollapsingHeader("PIO Port A", "#pio_a", true, true)) {
            _ui_z80pio_port(win, Z80PIO_PORT_A);
        }
        if (ImGui::CollapsingHeader("PIO Port B", "#pio_b", true, true)) {
            _ui_z80pio_port(win, Z80PIO_PORT_B);
        }
    }
    ImGui::End();
}

} /* extern "C" */
#endif

