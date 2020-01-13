/*#
    # ui_m6526.h

    Debug visualization UI for m6526.h

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
        - m6526.h
        - ui_chip.h

    Include the following headers before including the *implementation*:
        - imgui.h
        - m6526.h
        - ui_chip.h
        - ui_util.h

    All strings provided to ui_m6526_init() must remain alive until
    ui_m6526_discard() is called!

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

/* setup parameters for ui_m6526_init()
    NOTE: all string data must remain alive until ui_m6526_discard()!
*/
typedef struct ui_m6526_desc_t {
    const char* title;          /* window title */
    m6526_t* cia;               /* m6526_t instance to track */
    int x, y;                   /* initial window position */
    int w, h;                   /* initial window size (or default size if 0) */
    bool open;                  /* initial open state */
    ui_chip_desc_t chip_desc;   /* chip visualization desc */
} ui_m6526_desc_t;

typedef struct ui_m6526_t {
    const char* title;
    m6526_t* cia;
    float init_x, init_y;
    float init_w, init_h;
    bool open;
    bool valid;
    ui_chip_t chip;
} ui_m6526_t;

void ui_m6526_init(ui_m6526_t* win, const ui_m6526_desc_t* desc);
void ui_m6526_discard(ui_m6526_t* win);
void ui_m6526_draw(ui_m6526_t* win);

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

void ui_m6526_init(ui_m6526_t* win, const ui_m6526_desc_t* desc) {
    CHIPS_ASSERT(win && desc);
    CHIPS_ASSERT(desc->title);
    CHIPS_ASSERT(desc->cia);
    memset(win, 0, sizeof(ui_m6526_t));
    win->title = desc->title;
    win->cia = desc->cia;
    win->init_x = (float) desc->x;
    win->init_y = (float) desc->y;
    win->init_w = (float) ((desc->w == 0) ? 420 : desc->w);
    win->init_h = (float) ((desc->h == 0) ? 460 : desc->h);
    win->open = desc->open;
    win->valid = true;
    ui_chip_init(&win->chip, &desc->chip_desc);
}

void ui_m6526_discard(ui_m6526_t* win) {
    CHIPS_ASSERT(win && win->valid);
    win->valid = false;
}

static void _ui_m6526_draw_state(ui_m6526_t* win) {
    const m6526_t* cia = win->cia;
    ImGui::Columns(3, "##cia_columns", false);
    ImGui::SetColumnWidth(0, 64);
    ImGui::SetColumnWidth(1, 80);
    ImGui::SetColumnWidth(2, 80);

    /* ports */
    ImGui::NextColumn();
    ImGui::Text("Port A"); ImGui::NextColumn();
    ImGui::Text("Port B"); ImGui::NextColumn();
    ImGui::Separator();
    ImGui::Text("DDR"); ImGui::NextColumn();
    ui_util_b8("", cia->pa.ddr); ImGui::NextColumn();
    ui_util_b8("", cia->pb.ddr); ImGui::NextColumn();
    ImGui::Text("Reg"); ImGui::NextColumn();
    ui_util_b8("", cia->pa.reg); ImGui::NextColumn();
    ui_util_b8("", cia->pb.reg); ImGui::NextColumn();
    ImGui::Text("Inp"); ImGui::NextColumn();
    ui_util_b8("", cia->pa.inp); ImGui::NextColumn();
    ui_util_b8("", cia->pb.inp); ImGui::NextColumn();
    ImGui::Text("Pins"); ImGui::NextColumn();
    ui_util_b8("", cia->pa.pins); ImGui::NextColumn();
    ui_util_b8("", cia->pb.pins); ImGui::NextColumn();

    /* timers */
    ImGui::Separator();
    ImGui::NextColumn();
    ImGui::Text("Timer A"); ImGui::NextColumn();
    ImGui::Text("Timer B"); ImGui::NextColumn();
    ImGui::Separator();
    ImGui::Text("Latch"); ImGui::NextColumn();
    ImGui::Text("%04X", cia->ta.latch); ImGui::NextColumn();
    ImGui::Text("%04X", cia->tb.latch); ImGui::NextColumn();
    ImGui::Text("Counter"); ImGui::NextColumn();
    ImGui::Text("%04X", cia->ta.counter); ImGui::NextColumn();
    ImGui::Text("%04X", cia->tb.counter); ImGui::NextColumn();
    ImGui::Text("Control"); ImGui::NextColumn();
    ui_util_b8("", cia->ta.cr); ImGui::NextColumn();
    ui_util_b8("", cia->tb.cr); ImGui::NextColumn();
    ImGui::Text(":START"); ImGui::NextColumn();
    ImGui::Text("%s", M6526_TIMER_STARTED(cia->ta.cr) ? "STARTED":"STOP"); ImGui::NextColumn();
    ImGui::Text("%s", M6526_TIMER_STARTED(cia->tb.cr) ? "STARTED":"STOP"); ImGui::NextColumn();
    ImGui::Text(":PBON"); ImGui::NextColumn();
    ImGui::Text("%s", M6526_PBON(cia->ta.cr) ? "PB6":"---"); ImGui::NextColumn();
    ImGui::Text("%s", M6526_PBON(cia->tb.cr) ? "PB7":"---"); ImGui::NextColumn();
    ImGui::Text(":OUTMODE"); ImGui::NextColumn();
    ImGui::Text("%s", M6526_OUTMODE_TOGGLE(cia->ta.cr) ? "TOGGLE":"PULSE"); ImGui::NextColumn();
    ImGui::Text("%s", M6526_OUTMODE_TOGGLE(cia->tb.cr) ? "TOGGLE":"PULSE"); ImGui::NextColumn();
    ImGui::Text(":RUNMODE"); ImGui::NextColumn();
    ImGui::Text("%s", M6526_RUNMODE_ONESHOT(cia->ta.cr) ? "ONESHOT":"CONT"); ImGui::NextColumn();
    ImGui::Text("%s", M6526_RUNMODE_ONESHOT(cia->tb.cr) ? "ONESHOT":"CONT"); ImGui::NextColumn();
    ImGui::Text(":INMODE"); ImGui::NextColumn();
    ImGui::Text("%s", M6526_TA_INMODE_PHI2(cia->ta.cr) ? "PHI2":"CNT"); ImGui::NextColumn();
    if (M6526_TB_INMODE_PHI2(cia->tb.cr)) {
        ImGui::Text("PHI2");
    }
    else if (M6526_TB_INMODE_CNT(cia->tb.cr)) {
        ImGui::Text("CNT");
    }
    else if (M6526_TB_INMODE_TA(cia->tb.cr)) {
        ImGui::Text("TA");
    }
    else if (M6526_TB_INMODE_TACNT(cia->tb.cr)) {
        ImGui::Text("TACNT");
    }
    ImGui::NextColumn();
    ImGui::Text(":SPMODE"); ImGui::NextColumn();
    ImGui::Text("%s", M6526_TA_SPMODE_OUTPUT(cia->ta.cr) ? "OUTPUT":"INPUT"); ImGui::NextColumn();
    ImGui::Text("---"); ImGui::NextColumn();
    ImGui::Text(":TODIN"); ImGui::NextColumn();
    ImGui::Text("%s", M6526_TA_TODIN_50HZ(cia->ta.cr) ? "50HZ":"60HZ"); ImGui::NextColumn();
    ImGui::Text("---"); ImGui::NextColumn();
    ImGui::Text(":ALARM"); ImGui::NextColumn();
    ImGui::Text("%s", M6526_TB_ALARM_ALARM(cia->ta.cr) ? "ALARM":"CLOCK"); ImGui::NextColumn();
    ImGui::Text("---"); ImGui::NextColumn();
    ImGui::Text("Bit"); ImGui::NextColumn();
    ImGui::Text("%s", cia->ta.t_bit ? "ON":"OFF"); ImGui::NextColumn();
    ImGui::Text("%s", cia->tb.t_bit ? "ON":"OFF"); ImGui::NextColumn();
    ImGui::Text("Out"); ImGui::NextColumn();
    ImGui::Text("%s", cia->ta.t_out ? "ON":"OFF"); ImGui::NextColumn();
    ImGui::Text("%s", cia->tb.t_out ? "ON":"OFF"); ImGui::NextColumn();

    /* interrupt */
    ImGui::Columns();
    ImGui::Separator();
    ImGui::Text("Interrupt");
    ImGui::Separator();
    ui_util_b8("Mask     ", cia->intr.imr);
    ui_util_b8("Control  ", cia->intr.icr);

}

void ui_m6526_draw(ui_m6526_t* win) {
    CHIPS_ASSERT(win && win->valid);
    if (!win->open) {
        return;
    }
    ImGui::SetNextWindowPos(ImVec2(win->init_x, win->init_y), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(win->init_w, win->init_h), ImGuiCond_Once);
    if (ImGui::Begin(win->title, &win->open)) {
        ImGui::BeginChild("##m6526_chip", ImVec2(176, 0), true);
        ui_chip_draw(&win->chip, win->cia->pins);
        ImGui::EndChild();
        ImGui::SameLine();
        ImGui::BeginChild("##m6526_state", ImVec2(0, 0), true);
        _ui_m6526_draw_state(win);
        ImGui::EndChild();
    }
    ImGui::End();
}
#endif /* CHIPS_IMPL */
