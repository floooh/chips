#pragma once
/*#
    # ui_upd765.h

    Debug visualization for upd765.h

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
        - upd765.h
        - ui_chip.h

    Include the following headers before including the *implementation*:
        - imgui.h
        - upd765.h
        - ui_chip.h
        - ui_util.h

    All string data provided to ui_upd765_init() must remain alive until
    until ui_upd765_discard() is called!

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

/* setup params for ui_upd765_init()
    NOTE: all string data must remain alive until ui_upd765_discard()
*/
typedef struct ui_upd765_desc_t {
    const char* title;          /* window title */
    upd765_t* upd765;           /* pointer to upd765_t instance to track */
    int x, y;                   /* initial window pos */
    int w, h;                   /* initial window size, or 0 for default size */
    bool open;                  /* initial open state */
    ui_chip_desc_t chip_desc;   /* chip visualization desc */
} ui_upd765_desc_t;

typedef struct ui_upd765_t {
    const char* title;
    upd765_t* upd765;
    float init_x, init_y;
    float init_w, init_h;
    bool open;
    bool valid;
    ui_chip_t chip;
} ui_upd765_t;

void ui_upd765_init(ui_upd765_t* win, const ui_upd765_desc_t* desc);
void ui_upd765_discard(ui_upd765_t* win);
void ui_upd765_draw(ui_upd765_t* win);

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

void ui_upd765_init(ui_upd765_t* win, ui_upd765_desc_t* desc) {
    CHIPS_ASSERT(win && desc);
    CHIPS_ASSERT(desc->title);
    CHIPS_ASSERT(desc->upd765);
    memset(win, 0, sizeof(ui_upd765_t));
    win->title = desc->title;
    win->upd765 = desc->upd765;
    win->init_x = (float) desc->x;
    win->init_y = (float) desc->y;
    win->init_w = (float) ((desc->w == 0) ? 496 : desc->w);
    win->init_h = (float) ((desc->h == 0) ? 400 : desc->h);
    win->open = desc->open;
    win->valid = true;
    ui_chip_init(&win->chip, &desc->chip_desc);
}

void ui_upd765_discard(ui_upd765_t* win) {
    CHIPS_ASSERT(win && win->valid);
    win->valid = false;
}

static const char* _ui_upd765_phase_str(int phase) {
    switch (phase) {
        case UPD765_PHASE_IDLE:     return "IDLE";
        case UPD765_PHASE_COMMAND:  return "COMMAND";
        case UPD765_PHASE_EXEC:     return "EXEC";
        case UPD765_PHASE_RESULT:   return "RESULT";
        default:                    return "???";
    }
}

static const char* _ui_upd765_cmd_str(int cmd) {
    switch (cmd) {
        case UPD765_CMD_INVALID:                return "INVALID";
        case UPD765_CMD_READ_DATA:              return "READ DATA";
        case UPD765_CMD_READ_DELETED_DATA:      return "READ DEL DATA";
        case UPD765_CMD_WRITE_DATA:             return "WRITE DATA";
        case UPD765_CMD_WRITE_DELETED_DATA:     return "WRITE DEL DATA";
        case UPD765_CMD_READ_A_TRACK:           return "READ A TRACK";
        case UPD765_CMD_READ_ID:                return "READ ID";
        case UPD765_CMD_FORMAT_A_TRACK:         return "FORMAT TRACK";
        case UPD765_CMD_SCAN_EQUAL:             return "SCAN EQUAL";
        case UPD765_CMD_SCAN_LOW_OR_EQUAL:      return "SCAN LO/EQUAL";
        case UPD765_CMD_SCAN_HIGH_OR_EQUAL:     return "SCAN HI/EQUAL";
        case UPD765_CMD_RECALIBRATE:            return "RECALIBRATE";
        case UPD765_CMD_SENSE_INTERRUPT_STATUS: return "SENSE INTR";
        case UPD765_CMD_SPECIFY:                return "SPECIFY";
        case UPD765_CMD_SENSE_DRIVE_STATUS:     return "SENSE DRIVE";
        case UPD765_CMD_SEEK:                   return "SEEK";
        default: return "???";
    }
}

static void _ui_upd765_draw_state(ui_upd765_t* win) {
    ImGui::Text("Phase  %s", _ui_upd765_phase_str(win->upd765->phase));
    ImGui::Text("Cmd    %s", _ui_upd765_cmd_str(win->upd765->cmd));
    ImGui::Text("Fifo  "); ImGui::SameLine();
    for (int i = 0; i < win->upd765->fifo_num; i++) {
        ImGui::Text("%02X", win->upd765->fifo[i]);
        if (i < (win->upd765->fifo_num-1)) {
            ImGui::SameLine();
        }
    }
    {
        /* last status */
        const uint8_t s = win->upd765->status;
        ui_util_b8("Status ", s);
        ImGui::Text("      "); ImGui::SameLine();
        const char* str[8] = { "D0B", "D1B", "D2B", "D3B", "CB", "EXM", "DIO", "RQM" };
        for (int i = 0; i < 8; i++) {
            int bit = 7-i;
            if ((s & (1<<bit)) != 0) {
                ImGui::Text("%s", str[bit]);
            }
            else {
                ImGui::TextDisabled("%s", str[bit]);
            }
            if (i < 7) {
                ImGui::SameLine();
            }
        }
    }
    ImGui::Separator();
    {
        /* status registers */
        const char* str[4][8] = {
            { "US0", "US1", "HD", "NR", "EC", "SE", "R0", "R1" },
            { "MA", "NW", "ND", "D3", "OR", "DE", "--", "EN "},
            { "MD", "BC", "SN", "SH", "WC", "DD", "CM", "--" },
            { "US0", "US1", "HD", "TS", "T0", "RY", "WP", "FT" }
        };
        const char* st0_res_str[4] = { "NT", "AT", "IC", "ATRM" };
        for (int sti = 0; sti < 4; sti++) {
            char buf[16];
            snprintf(buf, sizeof(buf), "ST%d    ", sti);
            const uint8_t s = win->upd765->st[sti];
            ui_util_b8(buf, s);
            ImGui::Text("      "); ImGui::SameLine();
            for (int i = 0; i < 8; i++) {
                /* special case bit 6 and 7 of st0 */
                if ((sti == 0) && (i >= 6)) {
                    if (i == 6) {
                        ImGui::Text("%s", st0_res_str[(s>>6) & 3]);
                    }
                }
                else {
                    int bit = 7-i;
                    if ((s & (1<<bit)) != 0) {
                        ImGui::Text("%s", str[sti][bit]);
                    }
                    else {
                        ImGui::TextDisabled("%s", str[sti][bit]);
                    }
                    if (i < 7) {
                        ImGui::SameLine();
                    }
                }
            }
        }
    }
    ImGui::Separator();
    {
        /* sector info */
        const upd765_sectorinfo_t* si = &win->upd765->sector_info;
        ImGui::Text("Physical Track: %X", si->physical_track);
        ImGui::Text("Cylinder:  %02X", si->c);
        ImGui::Text("Head Addr: %02X", si->h);
        ImGui::Text("Record:    %02X", si->r);
        ImGui::Text("Number:    %02X", si->n);
        ImGui::Text("ST1:       %02X", si->st1);
        ImGui::Text("ST2:       %02X", si->st2);
    }
}

void ui_upd765_draw(ui_upd765_t* win) {
    CHIPS_ASSERT(win && win->valid && win->title);
    if (!win->open) {
        return;
    }
    ImGui::SetNextWindowPos(ImVec2(win->init_x, win->init_y), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(win->init_w, win->init_h), ImGuiCond_Once);
    if (ImGui::Begin(win->title, &win->open)) {
        ImGui::BeginChild("##chip", ImVec2(176, 0), true);
        ui_chip_draw(&win->chip, win->upd765->pins);
        ImGui::EndChild();
        ImGui::SameLine();
        ImGui::BeginChild("##state", ImVec2(0, 0), true);
        _ui_upd765_draw_state(win);
        ImGui::EndChild();
    }
    ImGui::End();
}
#endif /* CHIPS_IMPL */
