#pragma once
/*#
    # ui_m6581.h

    Debug visualization for m6581.h (SID).

    Do this:
    ~~~C
    #define CHIPS_UI_IMPL
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
        - ui_settings.h

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
    bool last_open;
    bool valid;
    ui_chip_t chip;
} ui_m6581_t;

void ui_m6581_init(ui_m6581_t* win, const ui_m6581_desc_t* desc);
void ui_m6581_discard(ui_m6581_t* win);
void ui_m6581_draw(ui_m6581_t* win);
void ui_m6581_save_settings(ui_m6581_t* win, ui_settings_t* settings);
void ui_m6581_load_settings(ui_m6581_t* win, const ui_settings_t* settings);

#ifdef __cplusplus
} /* extern "C" */
#endif

/*-- IMPLEMENTATION (include in C++ source) ----------------------------------*/
#ifdef CHIPS_UI_IMPL
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
    win->open = win->last_open = desc->open;
    win->valid = true;
    ui_chip_init(&win->chip, &desc->chip_desc);
}

void ui_m6581_discard(ui_m6581_t* win) {
    CHIPS_ASSERT(win && win->valid);
    win->valid = false;
}

static void _ui_m6581_draw_state(ui_m6581_t* win) {
    m6581_t* sid = win->sid;
    const float cw0 = 84.0f;
    const float cw = 56.0f;
    if (ImGui::CollapsingHeader("Wave Generator", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGui::BeginTable("##sid_channels", 4)) {
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, cw0);
            ImGui::TableSetupColumn("Chn0", ImGuiTableColumnFlags_WidthFixed, cw);
            ImGui::TableSetupColumn("Chn2", ImGuiTableColumnFlags_WidthFixed, cw);
            ImGui::TableSetupColumn("Chn3", ImGuiTableColumnFlags_WidthFixed, cw);
            ImGui::TableHeadersRow();
            ImGui::TableNextColumn();
            ImGui::Text("Muted"); ImGui::TableNextColumn();
            for (int i = 0; i < 3; i++) {
                ImGui::Text("%s", sid->voice[i].muted ? "YES":"NO"); ImGui::TableNextColumn();
            }
            ImGui::Text("Frequency"); ImGui::TableNextColumn();
            for (int i = 0; i < 3; i++) {
                ImGui::Text("%04X", sid->voice[i].freq); ImGui::TableNextColumn();
            }
            ImGui::Text("Pulse Width"); ImGui::TableNextColumn();
            for (int i = 0; i < 3; i++) {
                ImGui::Text("%04X", sid->voice[i].pulse_width); ImGui::TableNextColumn();
            }
            ImGui::Text("Accum"); ImGui::TableNextColumn();
            for (int i = 0; i < 3; i++) {
                ImGui::Text("%06X", sid->voice[i].wav_accum); ImGui::TableNextColumn();
            }
            ImGui::Text("Noise Shift"); ImGui::TableNextColumn();
            for (int i = 0; i < 3; i++) {
                ImGui::Text("%06X", sid->voice[i].noise_shift); ImGui::TableNextColumn();
            }
            ImGui::Text("Sync"); ImGui::TableNextColumn();
            for (int i = 0; i < 3; i++) {
                ImGui::Text("%s", sid->voice[i].sync ? "ON":"OFF"); ImGui::TableNextColumn();
            }
            ImGui::Text("Output"); ImGui::TableNextColumn();
            for (int i = 0; i < 3; i++) {
                ImGui::Text("%04X", sid->voice[i].wav_output); ImGui::TableNextColumn();
            }
            ImGui::Text("Control"); ImGui::TableNextColumn();
            for (int i = 0; i < 3; i++) {
                ui_util_b8("", sid->voice[i].ctrl); ImGui::TableNextColumn();
            }
            ImGui::Text("  GATE"); ImGui::TableNextColumn();
            for (int i = 0; i < 3; i++) {
                ImGui::Text("%s", (sid->voice[i].ctrl & M6581_CTRL_GATE) ? "ON":"OFF"); ImGui::TableNextColumn();
            }
            ImGui::Text("  SYNC"); ImGui::TableNextColumn();
            for (int i = 0; i < 3; i++) {
                ImGui::Text("%s", (sid->voice[i].ctrl & M6581_CTRL_SYNC) ? "ON":"OFF"); ImGui::TableNextColumn();
            }
            ImGui::Text("  RINGMOD"); ImGui::TableNextColumn();
            for (int i = 0; i < 3; i++) {
                ImGui::Text("%s", (sid->voice[i].ctrl & M6581_CTRL_RINGMOD) ? "ON":"OFF"); ImGui::TableNextColumn();
            }
            ImGui::Text("  TEST"); ImGui::TableNextColumn();
            for (int i = 0; i < 3; i++) {
                ImGui::Text("%s", (sid->voice[i].ctrl & M6581_CTRL_TEST) ? "ON":"OFF"); ImGui::TableNextColumn();
            }
            ImGui::Text("  TRIANGLE"); ImGui::TableNextColumn();
            for (int i = 0; i < 3; i++) {
                ImGui::Text("%s", (sid->voice[i].ctrl & M6581_CTRL_TRIANGLE) ? "ON":"OFF"); ImGui::TableNextColumn();
            }
            ImGui::Text("  SAWTOOTH"); ImGui::TableNextColumn();
            for (int i = 0; i < 3; i++) {
                ImGui::Text("%s", (sid->voice[i].ctrl & M6581_CTRL_SAWTOOTH) ? "ON":"OFF"); ImGui::TableNextColumn();
            }
            ImGui::Text("  PULSE"); ImGui::TableNextColumn();
            for (int i = 0; i < 3; i++) {
                ImGui::Text("%s", (sid->voice[i].ctrl & M6581_CTRL_PULSE) ? "ON":"OFF"); ImGui::TableNextColumn();
            }
            ImGui::Text("  NOISE"); ImGui::TableNextColumn();
            for (int i = 0; i < 3; i++) {
                ImGui::Text("%s", (sid->voice[i].ctrl & M6581_CTRL_NOISE) ? "ON":"OFF"); ImGui::TableNextColumn();
            }
            ImGui::EndTable();
        }
    }
    if (ImGui::CollapsingHeader("Envelope Generator")) {
        if (ImGui::BeginTable("##sid_env", 4)) {
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, cw0);
            ImGui::TableSetupColumn("Chn0", ImGuiTableColumnFlags_WidthFixed, cw);
            ImGui::TableSetupColumn("Chn2", ImGuiTableColumnFlags_WidthFixed, cw);
            ImGui::TableSetupColumn("Chn3", ImGuiTableColumnFlags_WidthFixed, cw);
            ImGui::TableHeadersRow();
            ImGui::TableNextColumn();
            ImGui::Text("State"); ImGui::TableNextColumn();
            for (int i = 0; i < 3; i++) {
                const char* state;
                switch (sid->voice[i].env_state) {
                    case M6581_ENV_FROZEN: state = "frozen"; break;
                    case M6581_ENV_ATTACK: state = "attack"; break;
                    case M6581_ENV_DECAY:  state = "decay"; break;
                    case M6581_ENV_RELEASE: state = "release"; break;
                    default: state = "???"; break;
                }
                ImGui::Text("%s", state); ImGui::TableNextColumn();
            }
            ImGui::Text("Attack"); ImGui::TableNextColumn();
            for (int i = 0; i < 3; i++) {
                ImGui::Text("%X", sid->voice[i].env_attack_add); ImGui::TableNextColumn();
            }
            ImGui::Text("Decay"); ImGui::TableNextColumn();
            for (int i = 0; i < 3; i++) {
                ImGui::Text("%X", sid->voice[i].env_decay_sub); ImGui::TableNextColumn();
            }
            ImGui::Text("Sustain"); ImGui::TableNextColumn();
            for (int i = 0; i < 3; i++) {
                ImGui::Text("%04X", sid->voice[i].env_sustain_level); ImGui::TableNextColumn();
            }
            ImGui::Text("Release"); ImGui::TableNextColumn();
            for (int i = 0; i < 3; i++) {
                ImGui::Text("%X", sid->voice[i].env_release_sub); ImGui::TableNextColumn();
            }
            ImGui::Text("Cur Level"); ImGui::TableNextColumn();
            for (int i = 0; i < 3; i++) {
                ImGui::Text("%02X", sid->voice[i].env_cur_level); ImGui::TableNextColumn();
            }
            ImGui::Text("Counter"); ImGui::TableNextColumn();
            for (int i = 0; i < 3; i++) {
                ImGui::Text("%04X", sid->voice[i].env_counter); ImGui::TableNextColumn();
            }
            ImGui::Text("Exp Counter"); ImGui::TableNextColumn();
            for (int i = 0; i < 3; i++) {
                ImGui::Text("%02X", sid->voice[i].env_exp_counter); ImGui::TableNextColumn();
            }
            ImGui::Text("Counter Cmp"); ImGui::TableNextColumn();
            for (int i = 0; i < 3; i++) {
                ImGui::Text("%X", sid->voice[i].env_counter_compare); ImGui::TableNextColumn();
            }
            ImGui::EndTable();
        }
    }
    if (ImGui::CollapsingHeader("Filter")) {
        ImGui::Text("Voices        %s %s %s",
            (0 != (sid->filter.voices & 1)) ? "ON ":"OFF",
            (0 != (sid->filter.voices & 2)) ? "ON ":"OFF",
            (0 != (sid->filter.voices & 4)) ? "ON ":"OFF");
        ImGui::Text("Cutoff        %04X", sid->filter.cutoff);
        ImGui::Text("Resonance     %X", sid->filter.resonance);
        ImGui::Text("Volume        %X", sid->filter.volume);
        ImGui::Text("Mode          %X", sid->filter.mode);
        ImGui::Text("  LOWPASS     %s", (0 != (sid->filter.mode & M6581_FILTER_LP)) ? "ON":"OFF");
        ImGui::Text("  BANDPASS    %s", (0 != (sid->filter.mode & M6581_FILTER_BP)) ? "ON":"OFF");
        ImGui::Text("  HIGHPASS    %s", (0 != (sid->filter.mode & M6581_FILTER_HP)) ? "ON":"OFF");
        ImGui::Text("  3OFF        %s", (0 != (sid->filter.mode & M6581_FILTER_3OFF)) ? "ON":"OFF");
        ImGui::Text("w0            %X", sid->filter.w0);
        ImGui::Text("v_lp          %X", sid->filter.v_lp);
        ImGui::Text("v_bp          %X", sid->filter.v_bp);
        ImGui::Text("v_hp          %X", sid->filter.v_hp);
    }
}

void ui_m6581_draw(ui_m6581_t* win) {
    CHIPS_ASSERT(win && win->valid);
    ui_util_handle_window_open_dirty(&win->open, &win->last_open);
    if (!win->open) {
        return;
    }
    ImGui::SetNextWindowPos(ImVec2(win->init_x, win->init_y), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(win->init_w, win->init_h), ImGuiCond_FirstUseEver);
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

void ui_m6581_save_settings(ui_m6581_t* win, ui_settings_t* settings) {
    CHIPS_ASSERT(win && settings);
    ui_settings_add(settings, win->title, win->open);
}

void ui_m6581_load_settings(ui_m6581_t* win, const ui_settings_t* settings) {
    CHIPS_ASSERT(win && settings);
    win->open = ui_settings_isopen(settings, win->title);
}
#endif /* CHIPS_UI_IMPL */
