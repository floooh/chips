#pragma once
/*#
    # ui_m6522.h

    Debug visualization UI for m6522.h

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
        - m6522.h
        - ui_chip.h

    Include the following headers before including the *implementation*:
        - imgui.h
        - m6522.h
        - ui_chip.h
        - ui_util.h

    All strings provided to ui_m6522_init() must remain alive until
    ui_m6522_discard() is called!

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

/* setup parameters for ui_m6522_init()
    NOTE: all string data must remain alive until ui_m6522_discard()!
*/
typedef struct ui_m6522_desc_t {
    const char* title;          /* window title */
    m6522_t* via;               /* m6522_t instance to track */
    uint16_t regs_base;         /* register bank base address (e.g. 9110 or 9120 on VIC-20) */
    int x, y;                   /* initial window pos */
    int w, h;                   /* initial window size (or 0 for default size) */
    bool open;                  /* initial window open state */
    ui_chip_desc_t chip_desc;   /* chip visualization desc */
} ui_m6522_desc_t;

typedef struct ui_m6522_t {
    const char* title;
    m6522_t* via;
    uint16_t regs_base;
    float init_x, init_y;
    float init_w, init_h;
    bool open;
    bool valid;
    ui_chip_t chip;
} ui_m6522_t;

void ui_m6522_init(ui_m6522_t* win, const ui_m6522_desc_t* desc);
void ui_m6522_discard(ui_m6522_t* win);
void ui_m6522_draw(ui_m6522_t* win);

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

void ui_m6522_init(ui_m6522_t* win, const ui_m6522_desc_t* desc) {
    CHIPS_ASSERT(win && desc);
    CHIPS_ASSERT(desc->title);
    CHIPS_ASSERT(desc->via);
    memset(win, 0, sizeof(ui_m6522_t));
    win->title = desc->title;
    win->via = desc->via;
    win->regs_base = desc->regs_base;
    win->init_x = (float) desc->x;
    win->init_y = (float) desc->y;
    win->init_w = (float) ((desc->w == 0) ? 440 : desc->w);
    win->init_h = (float) ((desc->h == 0) ? 380 : desc->h);
    win->open = desc->open;
    win->valid = true;
    ui_chip_init(&win->chip, &desc->chip_desc);
}

void ui_m6522_discard(ui_m6522_t* win) {
    CHIPS_ASSERT(win && win->valid);
    win->valid = false;
}

static void _ui_m6522_draw_registers(ui_m6522_t* win) {
    if (ImGui::CollapsingHeader("Registers", ImGuiTreeNodeFlags_DefaultOpen)) {
        const m6522_t* via = win->via;
        uint16_t rb = win->regs_base;
        ImGui::Text("RB I/O   ($%04X/%5d): %02X/%02X", rb+0, rb+0, via->pb.inpr, via->pb.outr);
        ImGui::Text("RA I/O   ($%04X/%5d): %02X/%02X", rb+1, rb+1, via->pa.inpr, via->pa.outr);
        ImGui::Text("DDRB     ($%04X/%5d): %02X", rb+2, rb+2, via->pb.ddr);
        ImGui::Text("DDRA     ($%04X/%5d): %02X", rb+3, rb+3, via->pa.ddr);
        ImGui::Text("T1CL     ($%04X/%5d): %02X", rb+4, rb+4, via->t1.counter & 0xFF);
        ImGui::Text("T1CH     ($%04X/%5d): %02X", rb+5, rb+5, (via->t1.counter>>8) & 0xFF);
        ImGui::Text("T1LL     ($%04X/%5d): %02X", rb+6, rb+6, via->t1.latch & 0xFF);
        ImGui::Text("T1LH     ($%04X/%5d): %02X", rb+7, rb+7, (via->t1.latch>>8) & 0xFF);
        ImGui::Text("T2CL C/L ($%04X/%5d): %02X/%02X", rb+8, rb+8, via->t2.counter & 0xFF, via->t2.latch & 0xFF);
        ImGui::Text("T2CH C/L ($%04X/%5d): %02X/%02X", rb+9, rb+9, (via->t2.counter>>8) & 0xFF, (via->t2.latch>>8) & 0xFF);
        ImGui::Text("SR       ($%04X/%5d): ??", rb+10, rb+10);
        ImGui::Text("ACR      ($%04X/%5d): %02X", rb+11, rb+11, via->acr);
        ImGui::Text("PCR      ($%04X/%5d): %02X", rb+12, rb+12, via->pcr);
        ImGui::Text("IFR      ($%04X/%5d): %02X", rb+13, rb+13, via->intr.ifr);
        ImGui::Text("IER      ($%04X/%5d): %02X", rb+14, rb+14, via->intr.ier);
        ImGui::Text("RA_N I/O ($%04X/%5d): %02X/%02X", rb+15, rb+15, via->pa.inpr, via->pa.outr);
    }
}

static void _ui_m6522_draw_ports(ui_m6522_t* win) {
    if (ImGui::CollapsingHeader("Ports", ImGuiTreeNodeFlags_DefaultOpen)) {
        const m6522_t* via = win->via;
        ui_util_b8("PA DDR:  ", via->pa.ddr);  ImGui::SameLine(); ImGui::Text("(%02X)", via->pa.ddr);
        ui_util_b8("PA Inp:  ", via->pa.inpr); ImGui::SameLine(); ImGui::Text("(%02X)", via->pa.inpr);
        ui_util_b8("PA Out:  ", via->pa.outr); ImGui::SameLine(); ImGui::Text("(%02X)", via->pa.outr);
        ui_util_b8("PA Pins: ", via->pa.pins); ImGui::SameLine(); ImGui::Text("(%02X)", via->pa.pins);
        ImGui::Text("PA C1:   in=%s, out=%s", via->pa.c1_in?"ON ":"OFF", via->pa.c1_out?"ON ":"OFF");
        ImGui::Text("PA C2:   in=%s, out=%s", via->pa.c2_in?"ON ":"OFF", via->pa.c2_out?"ON ":"OFF");
        ImGui::Separator();
        ui_util_b8("PB DDR:  ", via->pb.ddr);  ImGui::SameLine(); ImGui::Text("(%02X)", via->pb.ddr);
        ui_util_b8("PB Inp:  ", via->pb.inpr); ImGui::SameLine(); ImGui::Text("(%02X)", via->pb.inpr);
        ui_util_b8("PB Out:  ", via->pb.outr); ImGui::SameLine(); ImGui::Text("(%02X)", via->pb.outr);
        ui_util_b8("PB Pins: ", via->pb.pins); ImGui::SameLine(); ImGui::Text("(%02X)", via->pb.pins);
        ImGui::Text("PB C1:   in=%s, out=%s", via->pb.c1_in?"ON ":"OFF", via->pb.c1_out?"ON ":"OFF");
        ImGui::Text("PB C2:   in=%s, out=%s", via->pb.c2_in?"ON ":"OFF", via->pb.c2_out?"ON ":"OFF");
    }
}

static void _ui_m6522_draw_counters(ui_m6522_t* win) {
    if (ImGui::CollapsingHeader("Counters", ImGuiTreeNodeFlags_DefaultOpen)) {
        const m6522_t* via = win->via;
        ImGui::Text("T1 Count: %04X", via->t1.counter);
        ImGui::Text("T1 Latch: %04X", via->t1.latch);
        ImGui::Text("T1 Out:   %s", via->t1.t_out ? "ON":"OFF");
        ImGui::Text("T1 Bit:   %s", via->t1.t_bit ? "ON":"OFF");
        ImGui::Separator();
        ImGui::Text("T2 Count: %04X", via->t2.counter);
        ImGui::Text("T2 Latch: %04X", via->t2.latch);
        ImGui::Text("T2 Out:   %s", via->t2.t_out ? "ON":"OFF");
        ImGui::Text("T2 Bit:   %s", via->t2.t_bit ? "ON":"OFF");
    }
}

static void _ui_m6522_draw_int_ctrl(ui_m6522_t* win) {
    if (ImGui::CollapsingHeader("Control & Interrupts", ImGuiTreeNodeFlags_DefaultOpen)) {
        const m6522_t* via = win->via;
        ImGui::Text("ACR: %02X\n", via->acr);
        ImGui::Text("  PA Latch:      %s", M6522_ACR_PA_LATCH_ENABLE(via) ? "Enabled":"Disabled");
        ImGui::Text("  PB Latch:      %s", M6522_ACR_PB_LATCH_ENABLE(via) ? "Enabled":"Disabled");
        const char* shift_ctrl = "??";
        if (M6522_ACR_SR_DISABLED(via))         { shift_ctrl = "Disabled"; }
        else if (M6522_ACR_SI_T2_CONTROL(via))  { shift_ctrl = "In by T2 Ctrl"; }
        else if (M6522_ACR_SI_O2_CONTROL(via))  { shift_ctrl = "In by Clock"; }
        else if (M6522_ACR_SI_EXT_CONTROL(via)) { shift_ctrl = "In by Pulse"; }
        else if (M6522_ACR_SO_T2_RATE(via))     { shift_ctrl = "Out by T2 Rate"; }
        else if (M6522_ACR_SO_T2_CONTROL(via))  { shift_ctrl = "Out by T2 Ctrl"; }
        else if (M6522_ACR_SO_O2_CONTROL(via))  { shift_ctrl = "Out by Clock"; }
        else if (M6522_ACR_SO_EXT_CONTROL(via)) { shift_ctrl = "Out by Pulse"; }
        ImGui::Text("  Shift Control: %s", shift_ctrl);
        ImGui::Text("  T2 Control:    %s", M6522_ACR_T2_COUNT_PB6(via) ? "PB6 Pulses":"One Shot");
        ImGui::Text("  T1 Mode:       %s", M6522_ACR_T1_CONTINUOUS(via) ? "Continuous":"One Shot");
        ImGui::Text("  T1 set PB7:    %s", M6522_ACR_T1_SET_PB7(via) ? "Enabled":"Disabled");
        ImGui::Text("PCR: %02X\n", via->pcr);
        ImGui::Text("  CA1 Ctrl: %s", M6522_PCR_CA1_LOW_TO_HIGH(via)? "IRQ on Rising":"IRQ on Falling");
        const char* ca2_ctrl = "??";
        switch ((via->pcr>>1) & 7) {
            case 0: ca2_ctrl = "In, IRQ on Falling"; break;
            case 1: ca2_ctrl = "In, Indep IRQ on Falling"; break;
            case 2: ca2_ctrl = "In, IRQ on Rising"; break;
            case 3: ca2_ctrl = "In, Indep IRQ on Rising"; break;
            case 4: ca2_ctrl = "Handshake Out"; break;
            case 5: ca2_ctrl = "Pulse Out"; break;
            case 6: ca2_ctrl = "Manual Out, Low"; break;
            case 7: ca2_ctrl = "Manual Out, High"; break;
        }
        ImGui::Text("  CA2 Ctrl: %s", ca2_ctrl);
        ImGui::Text("  CB1 Ctrl: %s", M6522_PCR_CB1_LOW_TO_HIGH(via)? "IRQ on Rising":"IRQ on Falling");
        const char* cb2_ctrl = "??";
        switch ((via->pcr>>5) & 7) {
            case 0: cb2_ctrl = "In, IRQ on Falling"; break;
            case 1: cb2_ctrl = "In, Indep IRQ on Falling"; break;
            case 2: cb2_ctrl = "In, IRQ on Rising"; break;
            case 3: cb2_ctrl = "In, Indep IRQ on Rising"; break;
            case 4: cb2_ctrl = "Handshake Out"; break;
            case 5: cb2_ctrl = "Pulse Out"; break;
            case 6: cb2_ctrl = "Manual Out, Low"; break;
            case 7: cb2_ctrl = "Manual Out, High"; break;
        }
        ImGui::Text("  CB2 Ctrl: %s", cb2_ctrl);
        ImGui::Text("IFR/IER: %02X/%02X", via->intr.ifr, via->intr.ier);
        ImGui::Text("  CA2: %s (%s)", (via->intr.ifr & M6522_IRQ_CA2) ? "ON ":"OFF", (via->intr.ier & M6522_IRQ_CA2) ? "Enabled":"Disabled");
        ImGui::Text("  CA1: %s (%s)", (via->intr.ifr & M6522_IRQ_CA1) ? "ON ":"OFF", (via->intr.ier & M6522_IRQ_CA1) ? "Enabled":"Disabled");
        ImGui::Text("  SR:  %s (%s)", (via->intr.ifr & M6522_IRQ_SR) ? "ON ":"OFF", (via->intr.ier & M6522_IRQ_SR) ? "Enabled":"Disabled");
        ImGui::Text("  CB2: %s (%s)", (via->intr.ifr & M6522_IRQ_CB2) ? "ON ":"OFF", (via->intr.ier & M6522_IRQ_CB2) ? "Enabled":"Disabled");
        ImGui::Text("  CB1: %s (%s)", (via->intr.ifr & M6522_IRQ_CB1) ? "ON ":"OFF", (via->intr.ier & M6522_IRQ_CB1) ? "Enabled":"Disabled");
        ImGui::Text("  T2:  %s (%s)", (via->intr.ifr & M6522_IRQ_T2) ? "ON ":"OFF", (via->intr.ier & M6522_IRQ_T2) ? "Enabled":"Disabled");
        ImGui::Text("  T1:  %s (%s)", (via->intr.ifr & M6522_IRQ_T1) ? "ON ":"OFF", (via->intr.ier & M6522_IRQ_T1) ? "Enabled":"Disabled");
        ImGui::Text("  ANY: %s",      (via->intr.ifr & M6522_IRQ_ANY) ? "ON ":"OFF");
    }
}

void ui_m6522_draw(ui_m6522_t* win) {
    CHIPS_ASSERT(win && win->valid && win->via);
    if (!win->open) {
        return;
    }
    ImGui::SetNextWindowPos(ImVec2(win->init_x, win->init_y), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(win->init_w, win->init_h), ImGuiCond_Once);
    if (ImGui::Begin(win->title, &win->open)) {
        ImGui::BeginChild("##m6522_chip", ImVec2(176, 0), true);
        ui_chip_draw(&win->chip, win->via->pins);
        ImGui::EndChild();
        ImGui::SameLine();
        ImGui::BeginChild("##m6522_state", ImVec2(0, 0), true);
        _ui_m6522_draw_registers(win);
        _ui_m6522_draw_ports(win);
        _ui_m6522_draw_counters(win);
        _ui_m6522_draw_int_ctrl(win);
        ImGui::EndChild();
    }
    ImGui::End();
}
#endif /* CHIPS_IMPL */
