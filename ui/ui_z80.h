#pragma once
/*#
    # ui_z80.h

    Debug visualization UI of Z80 CPU.

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
        - z80.h
        - ui_chip.h

    Include the following headers before including the *implementation*:
        - imgui.h
        - z80.h
        - ui_chip.h
        - ui_util.h

    All strings provided to ui_z80_init() must remain alive until
    ui_z80_discard() is called!

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

/* setup parameters for ui_z80_init()
    NOTE: all string data must remain alive until ui_z80_discard()!
*/
typedef struct {
    const char* title;          /* window title */
    z80_t* cpu;                 /* pointer to CPU to track */
    int x, y;                   /* initial window pos */
    int w, h;                   /* initial window size, or 0 for default size */
    bool open;                  /* initial open state */
    ui_chip_desc_t chip_desc;   /* chip visualization desc */
} ui_z80_desc_t;

typedef struct {
    const char* title;
    z80_t* cpu;
    float init_x, init_y;
    float init_w, init_h;
    bool open;
    bool valid;
    ui_chip_t chip;
} ui_z80_t;

void ui_z80_init(ui_z80_t* win, const ui_z80_desc_t* desc);
void ui_z80_discard(ui_z80_t* win);
void ui_z80_draw(ui_z80_t* win);

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

void ui_z80_init(ui_z80_t* win, const ui_z80_desc_t* desc) {
    CHIPS_ASSERT(win && desc);
    CHIPS_ASSERT(desc->title);
    CHIPS_ASSERT(desc->cpu);
    memset(win, 0, sizeof(ui_z80_t));
    win->title = desc->title;
    win->cpu = desc->cpu;
    win->init_x = (float) desc->x;
    win->init_y = (float) desc->y;
    win->init_w = (float) ((desc->w == 0) ? 360 : desc->w);
    win->init_h = (float) ((desc->h == 0) ? 340 : desc->h);
    win->open = desc->open;
    win->valid = true;
    ui_chip_init(&win->chip, &desc->chip_desc);
}

void ui_z80_discard(ui_z80_t* win) {
    CHIPS_ASSERT(win && win->valid);
    win->valid = false;
}

static void _ui_z80_regs(ui_z80_t* win) {
    z80_t* cpu = win->cpu;
    ImGui::Text("AF: %04X  AF': %04X", z80_af(cpu), z80_af_(cpu));
    ImGui::Text("BC: %04X  BC': %04X", z80_bc(cpu), z80_bc_(cpu));
    ImGui::Text("DE: %04X  DE': %04X", z80_de(cpu), z80_de_(cpu));
    ImGui::Text("HL: %04X  HL': %04X", z80_hl(cpu), z80_hl_(cpu));
    ImGui::Separator();
    ImGui::Text("IX: %04X  IY:  %04X", z80_ix(cpu), z80_iy(cpu));
    ImGui::Text("PC: %04X  SP:  %04X", z80_pc(cpu), z80_sp(cpu));
    ImGui::Text("IR: %04X  WZ:  %04X", z80_ir(cpu), z80_wz(cpu));
    ImGui::Text("IM: %02X", z80_im(cpu));
    ImGui::Separator();
    const uint8_t f = z80_f(cpu);
    char f_str[9] = {
        (f & Z80_SF) ? 'S':'-',
        (f & Z80_ZF) ? 'Z':'-',
        (f & Z80_YF) ? 'X':'-',
        (f & Z80_HF) ? 'H':'-',
        (f & Z80_XF) ? 'Y':'-',
        (f & Z80_VF) ? 'V':'-',
        (f & Z80_NF) ? 'N':'-',
        (f & Z80_CF) ? 'C':'-',
        0,
    };
    ImGui::Text("Flags: %s", f_str);
    ImGui::Text("IFF1:  %s", z80_iff1(cpu)?"ON":"OFF");
    ImGui::Text("IFF2:  %s", z80_iff2(cpu)?"ON":"OFF");
    ImGui::Separator();
    ImGui::Text("Addr:  %04X", Z80_GET_ADDR(cpu->pins));
    ImGui::Text("Data:  %02X", Z80_GET_DATA(cpu->pins));
    ImGui::Text("Wait:  %d", (int)Z80_GET_WAIT(cpu->pins));
}

void ui_z80_draw(ui_z80_t* win) {
    CHIPS_ASSERT(win && win->valid && win->title && win->cpu);
    if (!win->open) {
        return;
    }
    ImGui::SetNextWindowPos(ImVec2(win->init_x, win->init_y), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(win->init_w, win->init_h), ImGuiCond_Once);
    if (ImGui::Begin(win->title, &win->open)) {
        ImGui::BeginChild("##z80_chip", ImVec2(176, 0), true);
        ui_chip_draw(&win->chip, win->cpu->pins);
        ImGui::EndChild();
        ImGui::SameLine();
        ImGui::BeginChild("##z80_regs", ImVec2(0, 0), true);
        _ui_z80_regs(win);
        ImGui::EndChild();
    }
    ImGui::End();
}
#endif /* CHIPS_IMPL */
