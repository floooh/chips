#pragma once
/*#
    # ui_fdd.h

    Debug visualization for fdd.h (floppy disc drive)

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
        - fdd.h
        - ui_chip.h

    Include the following headers before including the *implementation*:
        - imgui.h
        - fdd.h
        - ui_util.h

    All string data provided to the ui_fdd_init() must remain alive until
    until ui_fdd_discard() is called!

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

/* setup parameters for ui_fdd_init()
    NOTE: all string data must remain alive until ui_i8255_discard() is called!
*/
typedef struct ui_fdd_desc_t {
    const char* title;      /* window title */
    fdd_t* fdd;             /* pointer to fdd_t instance to track */
    int x, y;               /* initial window position */
    int w, h;               /* initial window size, or 0 for default size */
    bool open;              /* initial window open state */
} ui_fdd_desc_t;

typedef struct ui_fdd_t {
    const char* title;
    fdd_t* fdd;
    float init_x, init_y;
    float init_w, init_h;
    bool open;
    bool valid;
} ui_fdd_t;

void ui_fdd_init(ui_fdd_t* win, ui_fdd_desc_t* desc);
void ui_fdd_discard(ui_fdd_t* win);
void ui_fdd_draw(ui_fdd_t* win);

#ifdef __cplusplus
} /* extern "C" */
#endif

/*-- IMPLEMENTATION (include in C++ source) ----------------------------------*/
#ifdef CHIPS_IMPL
#ifndef __cplusplus
#error "implementation must be compiled as C++"
#endif
#include <string.h> /* memset */
#include <ctype.h>  /* isalnum */
#ifndef CHIPS_ASSERT
    #include <assert.h>
    #define CHIPS_ASSERT(c) assert(c)
#endif

void ui_fdd_init(ui_fdd_t* win, ui_fdd_desc_t* desc) {
    CHIPS_ASSERT(win && desc);
    CHIPS_ASSERT(desc->title);
    CHIPS_ASSERT(desc->fdd);
    memset(win, 0, sizeof(ui_fdd_t));
    win->title = desc->title;
    win->fdd = desc->fdd;
    win->init_x = (float) desc->x;
    win->init_y = (float) desc->y;
    win->init_w = (float) ((desc->w == 0) ? 540 : desc->w);
    win->init_h = (float) ((desc->h == 0) ? 320 : desc->h);
    win->open = desc->open;
    win->valid = true;
}

void ui_fdd_discard(ui_fdd_t* win) {
    CHIPS_ASSERT(win && win->valid);
    win->valid = false;
}

void ui_fdd_draw(ui_fdd_t* win) {
    CHIPS_ASSERT(win && win->valid && win->title);
    if (!win->open) {
        return;
    }
    ImGui::SetNextWindowPos(ImVec2(win->init_x, win->init_y), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(win->init_w, win->init_h), ImGuiCond_Once);
    if (ImGui::Begin(win->title, &win->open)) {
        if (ImGui::CollapsingHeader("Drive Status", ImGuiTreeNodeFlags_DefaultOpen)) {
            fdd_t* fdd = win->fdd;
            ImGui::Text("Motor: %s", fdd->motor_on ? "ON":"OFF");
            ImGui::Text("Disc:  %s", fdd->has_disc ? "YES":"NO");
            ImGui::Text("Current Side:   %d", fdd->cur_side);
            ImGui::Text("Current Track:  %d", fdd->cur_track_index);
            ImGui::Text("Current Sector: %d", fdd->cur_sector_index);
            ImGui::Text("Pos in Sector:  %d", fdd->cur_sector_pos);
        }
        if (ImGui::CollapsingHeader("Disc Status", ImGuiTreeNodeFlags_DefaultOpen)) {
            if (win->fdd->has_disc) {
                fdd_disc_t* disc = &win->fdd->disc;
                ImGui::Text("Formatted:       %s", disc->formatted ? "YES":"NO");
                ImGui::Text("Write Protected: %s", disc->write_protected ? "YES":"NO");
                ImGui::Text("Sides:  %d", disc->num_sides);
                ImGui::Text("Tracks: %d", disc->num_tracks);
                for (int side = 0; side < disc->num_sides; side++) {
                    if (ImGui::CollapsingHeader(side == 0 ? "Side 1":"Side 2", ImGuiTreeNodeFlags_DefaultOpen)) {
                        for (int track_index = 0; track_index < disc->num_tracks; track_index++) {
                            const int bytes_per_line = 16;
                            char buf[64];
                            snprintf(buf, sizeof(buf), "Track %d", track_index);
                            ImGui::Text(" "); ImGui::SameLine();
                            if (ImGui::CollapsingHeader(buf)) {
                                fdd_track_t* track = &disc->tracks[side][track_index];
                                for (int sector_index = 0; sector_index < track->num_sectors; sector_index++) {
                                    fdd_sector_t* sec = &track->sectors[sector_index];
                                    ImGui::Text("  "); ImGui::SameLine();
                                    snprintf(buf, sizeof(buf), "Track %d / Sector %d", track_index, sector_index);
                                    if (ImGui::CollapsingHeader(buf)) {
                                        ImGui::Text("C:%02X H:%02X R:%02X N:%02X ST1:%02X ST2:%02X",
                                            sec->info.upd765.c,
                                            sec->info.upd765.h,
                                            sec->info.upd765.r,
                                            sec->info.upd765.n,
                                            sec->info.upd765.st1,
                                            sec->info.upd765.st2);
                                        int i = 0;
                                        while (i < sec->data_size) {
                                            int j = 0;
                                            ImGui::Text("%04X:", i); ImGui::SameLine();
                                            for (; (j < bytes_per_line) && (i < sec->data_size); j++, i++) {
                                                uint8_t val = win->fdd->data[i+sec->data_offset];
                                                if (isalnum((int)val)) {
                                                    buf[j] = val;
                                                }
                                                else {
                                                    buf[j] = '.';
                                                }
                                                ImGui::Text("%02X", val);
                                                ImGui::SameLine();
                                            }
                                            buf[j] = 0;
                                            ImGui::Text("  %s", buf);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            else {
                ImGui::Text("No disc in drive.");
            }
        }
    }
    ImGui::End();
}
#endif /* CHIPS_IMPL */
