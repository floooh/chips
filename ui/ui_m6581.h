#pragma once
/*#
    # ui_m6581.h

    Debug visualization for m6581.h (SID).

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
        - m6581.h
        - ui_chip.h

    Include the following headers before including the *implementation*:
        - imgui.h
        - m6581.h
        - ui_chip.h
        - ui_util.h

    All strings provided to ui_m6581_init() must remain alive until
    ui_m6581_discard() is called!

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

/* setup parameters for ui_m6581_init()
    NOTE: all string data must remain alive until ui_m6581_discard()!
*/
typedef struct ui_m6581_desc_t {
    const char* title;          /* window title */
    m6581_t* sid;               /* pointer to m6581_t instance to track */
    int x, y;                   /* initial window position */
    int w, h;                   /* initial window size (or default size of 0) */
    bool open;                  /* initial window open state */
    ui_chip_desc_t chip_desc;   /* chip visualization desc */
} ui_m6581_desc_t;

typedef struct ui_m6581_t {
    const char* title;
    m6581_t* sid;
    float init_x, init_y;
    float init_w, init_h;
    bool open;
    bool valid;
    ui_chip_t chip;
} ui_m6581_t;

void ui_m6581_init(ui_m6581_t* win, const ui_m6581_desc_t* desc);
void ui_m6581_discard(ui_m6581_t* win);
void ui_m6581_draw(ui_m6581_t* win);

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

void ui_m6581_init(ui_m6581_t* win, const ui_m6581_desc_t* desc) {
    CHIPS_ASSERT(win && desc);
    CHIPS_ASSERT(desc->title);
    CHIPS_ASSERT(desc->sid);
    memset(win, 0, sizeof(ui_m6581_t));
    win->title = desc->title;
    win->sid = desc->sid;
    win->init_x = (float) desc->x;
    win->init_y = (float) desc->y;
    win->init_w = (float) ((desc->w == 0) ? 496 : desc->w);
    win->init_h = (float) ((desc->h == 0) ? 410 : desc->h);
    win->open = desc->open;
    win->valid = true;
    ui_chip_init(&win->chip, &desc->chip_desc);
}

void ui_m6581_discard(ui_m6581_t* win) {
    CHIPS_ASSERT(win && win->valid);
    win->valid = false;
}

static void _ui_m6581_draw_state(ui_m6581_t* win) {
    m6581_t* sid = win->sid;
    const float cw0 = 96.0f;
    const float cw = 64.0f;
    ImGui::Columns(4, "##sid_channels", false);
    ImGui::SetColumnWidth(0, cw0);
    ImGui::SetColumnWidth(1, cw);
    ImGui::SetColumnWidth(2, cw);
    ImGui::SetColumnWidth(3, cw);
    ImGui::Columns();
    if (ImGui::CollapsingHeader("Wave Generator", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Columns(4, "##sid_channels", false);
        ImGui::NextColumn();
        ImGui::Text("Chn0"); ImGui::NextColumn();
        ImGui::Text("Chn1"); ImGui::NextColumn();
        ImGui::Text("Chn2"); ImGui::NextColumn();
        ImGui::Separator();
        ImGui::Text("Muted"); ImGui::NextColumn();
        for (int i = 0; i < 3; i++) {
            ImGui::Text("%s", sid->voice[i].muted ? "YES":"NO"); ImGui::NextColumn();
        }
        ImGui::Text("Frequency"); ImGui::NextColumn();
        for (int i = 0; i < 3; i++) {
            ImGui::Text("%04X", sid->voice[i].freq); ImGui::NextColumn();
        }
        ImGui::Text("Pulse Width"); ImGui::NextColumn();
        for (int i = 0; i < 3; i++) {
            ImGui::Text("%04X", sid->voice[i].pulse_width); ImGui::NextColumn();
        }
        ImGui::Text("Accum"); ImGui::NextColumn();
        for (int i = 0; i < 3; i++) {
            ImGui::Text("%06X", sid->voice[i].wav_accum); ImGui::NextColumn();
        }
        ImGui::Text("Noise Shift"); ImGui::NextColumn();
        for (int i = 0; i < 3; i++) {
            ImGui::Text("%06X", sid->voice[i].noise_shift); ImGui::NextColumn();
        }
        ImGui::Text("Sync"); ImGui::NextColumn();
        for (int i = 0; i < 3; i++) {
            ImGui::Text("%s", sid->voice[i].sync ? "ON":"OFF"); ImGui::NextColumn();
        }
        ImGui::Text("Output"); ImGui::NextColumn();
        for (int i = 0; i < 3; i++) {
            ImGui::Text("%04X", sid->voice[i].wav_output); ImGui::NextColumn();
        }
        ImGui::Text("Control"); ImGui::NextColumn();
        for (int i = 0; i < 3; i++) {
            ui_util_b8("", sid->voice[i].ctrl); ImGui::NextColumn();
        }
        ImGui::Text(":GATE"); ImGui::NextColumn();
        for (int i = 0; i < 3; i++) {
            ImGui::Text("%s", (sid->voice[i].ctrl & M6581_CTRL_GATE) ? "ON":"OFF"); ImGui::NextColumn();
        }
        ImGui::Text(":SYNC"); ImGui::NextColumn();
        for (int i = 0; i < 3; i++) {
            ImGui::Text("%s", (sid->voice[i].ctrl & M6581_CTRL_SYNC) ? "ON":"OFF"); ImGui::NextColumn();
        }
        ImGui::Text(":RINGMOD"); ImGui::NextColumn();
        for (int i = 0; i < 3; i++) {
            ImGui::Text("%s", (sid->voice[i].ctrl & M6581_CTRL_RINGMOD) ? "ON":"OFF"); ImGui::NextColumn();
        }
        ImGui::Text(":TEST"); ImGui::NextColumn();
        for (int i = 0; i < 3; i++) {
            ImGui::Text("%s", (sid->voice[i].ctrl & M6581_CTRL_TEST) ? "ON":"OFF"); ImGui::NextColumn();
        }
        ImGui::Text(":TRIANGLE"); ImGui::NextColumn();
        for (int i = 0; i < 3; i++) {
            ImGui::Text("%s", (sid->voice[i].ctrl & M6581_CTRL_TRIANGLE) ? "ON":"OFF"); ImGui::NextColumn();
        }
        ImGui::Text(":SAWTOOTH"); ImGui::NextColumn();
        for (int i = 0; i < 3; i++) {
            ImGui::Text("%s", (sid->voice[i].ctrl & M6581_CTRL_SAWTOOTH) ? "ON":"OFF"); ImGui::NextColumn();
        }
        ImGui::Text(":PULSE"); ImGui::NextColumn();
        for (int i = 0; i < 3; i++) {
            ImGui::Text("%s", (sid->voice[i].ctrl & M6581_CTRL_PULSE) ? "ON":"OFF"); ImGui::NextColumn();
        }
        ImGui::Text(":NOISE"); ImGui::NextColumn();
        for (int i = 0; i < 3; i++) {
            ImGui::Text("%s", (sid->voice[i].ctrl & M6581_CTRL_NOISE) ? "ON":"OFF"); ImGui::NextColumn();
        }
    }
    ImGui::Columns();
    if (ImGui::CollapsingHeader("Envelope Generator")) {
        ImGui::Columns(4, "##sid_channels", false);
        ImGui::NextColumn();
        ImGui::Text("Chn0"); ImGui::NextColumn();
        ImGui::Text("Chn1"); ImGui::NextColumn();
        ImGui::Text("Chn2"); ImGui::NextColumn();
        ImGui::Separator();
        ImGui::Text("State"); ImGui::NextColumn();
        for (int i = 0; i < 3; i++) {
            const char* state;
            switch (sid->voice[i].env_state) {
                case M6581_ENV_FROZEN: state = "frozen"; break;
                case M6581_ENV_ATTACK: state = "attack"; break;
                case M6581_ENV_DECAY:  state = "decay"; break;
                case M6581_ENV_RELEASE: state = "release"; break;
                default: state = "???"; break;
            }
            ImGui::Text("%s", state); ImGui::NextColumn();
        }
        ImGui::Text("Attack"); ImGui::NextColumn();
        for (int i = 0; i < 3; i++) {
            ImGui::Text("%X", sid->voice[i].env_attack_add); ImGui::NextColumn();
        }
        ImGui::Text("Decay"); ImGui::NextColumn();
        for (int i = 0; i < 3; i++) {
            ImGui::Text("%X", sid->voice[i].env_decay_sub); ImGui::NextColumn();
        }
        ImGui::Text("Sustain"); ImGui::NextColumn();
        for (int i = 0; i < 3; i++) {
            ImGui::Text("%04X", sid->voice[i].env_sustain_level); ImGui::NextColumn();
        }
        ImGui::Text("Release"); ImGui::NextColumn();
        for (int i = 0; i < 3; i++) {
            ImGui::Text("%X", sid->voice[i].env_release_sub); ImGui::NextColumn();
        }
        ImGui::Text("Cur Level"); ImGui::NextColumn();
        for (int i = 0; i < 3; i++) {
            ImGui::Text("%02X", sid->voice[i].env_cur_level); ImGui::NextColumn();
        }
        ImGui::Text("Counter"); ImGui::NextColumn();
        for (int i = 0; i < 3; i++) {
            ImGui::Text("%04X", sid->voice[i].env_counter); ImGui::NextColumn();
        }
        ImGui::Text("Exp Counter"); ImGui::NextColumn();
        for (int i = 0; i < 3; i++) {
            ImGui::Text("%02X", sid->voice[i].env_exp_counter); ImGui::NextColumn();
        } 
        ImGui::Text("Counter Cmp"); ImGui::NextColumn();
        for (int i = 0; i < 3; i++) {
            ImGui::Text("%X", sid->voice[i].env_counter_compare); ImGui::NextColumn();
        }
    }
    ImGui::Columns();
    if (ImGui::CollapsingHeader("Filter")) {
        ImGui::Text("Voices        %s %s %s", 
            (0 != (sid->filter.voices & 1)) ? "ON ":"OFF",
            (0 != (sid->filter.voices & 2)) ? "ON ":"OFF",
            (0 != (sid->filter.voices & 4)) ? "ON ":"OFF");
        ImGui::Text("Cutoff        %04X", sid->filter.cutoff);
        ImGui::Text("Resonance     %X", sid->filter.resonance);
        ImGui::Text("Volume        %X", sid->filter.volume);
        ImGui::Text("Mode          %X", sid->filter.mode);
        ImGui::Text(" :LOWPASS     %s", (0 != (sid->filter.mode & M6581_FILTER_LP)) ? "ON":"OFF");
        ImGui::Text(" :BANDPASS    %s", (0 != (sid->filter.mode & M6581_FILTER_BP)) ? "ON":"OFF");
        ImGui::Text(" :HIGHPASS    %s", (0 != (sid->filter.mode & M6581_FILTER_HP)) ? "ON":"OFF");
        ImGui::Text(" :3OFF        %s", (0 != (sid->filter.mode & M6581_FILTER_3OFF)) ? "ON":"OFF");
        ImGui::Text("w0            %X", sid->filter.w0);
        ImGui::Text("v_lp          %X", sid->filter.v_lp);
        ImGui::Text("v_bp          %X", sid->filter.v_bp);
        ImGui::Text("v_hp          %X", sid->filter.v_hp);
    }
}

void ui_m6581_draw(ui_m6581_t* win) {
    CHIPS_ASSERT(win && win->valid);
    if (!win->open) {
        return;
    }
    ImGui::SetNextWindowPos(ImVec2(win->init_x, win->init_y), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(win->init_w, win->init_h), ImGuiCond_Once);
    if (ImGui::Begin(win->title, &win->open)) {
        ImGui::BeginChild("##m6581_chip", ImVec2(176, 0), true);
        ui_chip_draw(&win->chip, win->sid->pins);
        ImGui::EndChild();
        ImGui::SameLine();
        ImGui::BeginChild("##m6581_state", ImVec2(0, 0), true);
        _ui_m6581_draw_state(win);
        ImGui::EndChild();
    }
    ImGui::End();
}

#endif /* CHIPS_IMPL */
