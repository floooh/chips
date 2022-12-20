#pragma once
/*#
    # ui_snapshots.h

    Snapshot UI helpers.

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

    You need to include the following headers before including the
    *implementation*:

        - imgui.h

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
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UI_SNAPSHOT_MAX_SLOTS (8)

// callback function to save snapshot to a numbered slot
typedef void (*ui_snapshot_save_t)(size_t slot_index);
// callback function to load snapshot from numbered slot
typedef bool (*ui_snapshot_load_t)(size_t slot_index);

typedef struct {
    void* texture;  // an abstract texture object as ImTextureID
    int width;      // width of texture in pixels
    int height;     // height of texture in pixels
} ui_snapshot_screenshot_t;

// describe a snapshot slot to the ui
typedef struct {
    ui_snapshot_screenshot_t screenshot;
} ui_snapshot_slot_info_t;

// a snapshot slot
typedef struct {
    bool valid;
    ui_snapshot_slot_info_t info;
} ui_snapshot_slot_t;

// initialization parameters
typedef struct {
    ui_snapshot_save_t save_cb;
    ui_snapshot_load_t load_cb;
    void* empty_slot_texture;
} ui_snapshot_desc_t;

// snapshot system state
typedef struct {
    ui_snapshot_save_t save_cb;
    ui_snapshot_load_t load_cb;
    ui_snapshot_slot_t slots[UI_SNAPSHOT_MAX_SLOTS];
} ui_snapshot_t;

// initialize the snapshot instance
void ui_snapshot_init(ui_snapshot_t* state, const ui_snapshot_desc_t* desc);
// inject snap menu UI
void ui_snapshot_menus(ui_snapshot_t* state);
// called from UI when a snapshot should be saved
void ui_snapshot_save_slot(ui_snapshot_t* state, size_t slot_index);
// called from UI when a snapshot should be loaded
bool ui_snapshot_load_slot(ui_snapshot_t* state, size_t slot_index);
// update snapshot info, returns previous slot info (usually called from within save callback)
ui_snapshot_slot_info_t ui_snapshot_update_slot_info(ui_snapshot_t* state, size_t slot_index, const ui_snapshot_slot_info_t* slot_info);

#ifdef __cplusplus
} // extern "C"
#endif

//-- IMPLEMENTATION ------------------------------------------------------------
#ifdef CHIPS_UI_IMPL
#include <string.h>
#ifndef CHIPS_ASSERT
    #include <assert.h>
    #define CHIPS_ASSERT(c) assert(c)
#endif

void ui_snapshot_init(ui_snapshot_t* state, const ui_snapshot_desc_t* desc) {
    CHIPS_ASSERT(state && desc);
    CHIPS_ASSERT(desc->save_cb && desc->load_cb);
    memset(state, 0, sizeof(ui_snapshot_t));
    state->save_cb = desc->save_cb;
    state->load_cb = desc->load_cb;
    for (size_t i = 0; i < UI_SNAPSHOT_MAX_SLOTS; i++) {
        state->slots[i].info.screenshot.texture = desc->empty_slot_texture;
    }
}

void ui_snapshot_menus(ui_snapshot_t* state) {
    CHIPS_ASSERT(state);
    if (ImGui::BeginMenu("Save Snapshot")) {
        for (size_t slot_index = 0; slot_index < UI_SNAPSHOT_MAX_SLOTS; slot_index++) {
            ImTextureID screenshot = state->slots[slot_index].info.screenshot.texture;
            if (screenshot) {
                ImGui::PushID(slot_index);
                const ImVec2 pos = ImGui::GetCursorPos();
                bool pressed = ImGui::Selectable("##savesnapshot", false, 0, (ImVec2){ 128.0f, 96.0f });
                ImGui::SetCursorPos(pos);
                ImGui::Image(screenshot, (ImVec2){ 128.0f, 96.0f });
                if (pressed) {
                    ui_snapshot_save_slot(state, slot_index);
                }
                if ((slot_index + 1) & 1) {
                    ImGui::SameLine();
                }
                ImGui::PopID();
            }
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Load Snapshot")) {
        for (size_t slot_index = 0; slot_index < UI_SNAPSHOT_MAX_SLOTS; slot_index++) {
            if (state->slots[slot_index].valid) {
                ImTextureID screenshot = state->slots[slot_index].info.screenshot.texture;
                if (screenshot) {
                    ImGui::PushID(slot_index);
                    const ImVec2 pos = ImGui::GetCursorPos();
                    bool pressed = ImGui::Selectable("##loadsnapshot", false, 0, (ImVec2){ 128.0f, 96.0f });
                    ImGui::SetCursorPos(pos);
                    ImGui::Image(screenshot, (ImVec2){ 128.0f, 96.0f });
                    if (pressed) {
                        ui_snapshot_load_slot(state, slot_index);
                    }
                    if ((slot_index + 1) & 1) {
                        ImGui::SameLine();
                    }
                    ImGui::PopID();
                }
            }
        }
        ImGui::EndMenu();
    }
}

void ui_snapshot_save_slot(ui_snapshot_t* state, size_t slot_index) {
    CHIPS_ASSERT(slot_index < UI_SNAPSHOT_MAX_SLOTS);
    state->save_cb(slot_index);
}

bool ui_snapshot_load_slot(ui_snapshot_t* state, size_t slot_index) {
    CHIPS_ASSERT(state);
    CHIPS_ASSERT(slot_index < UI_SNAPSHOT_MAX_SLOTS);
    if (state->slots[slot_index].valid) {
        return state->load_cb(slot_index);
    }
    else {
        return false;
    }
}

ui_snapshot_slot_info_t ui_snapshot_update_slot_info(ui_snapshot_t* state, size_t slot_index, const ui_snapshot_slot_info_t* slot_info) {
    CHIPS_ASSERT(state && slot_info);
    CHIPS_ASSERT(slot_index < UI_SNAPSHOT_MAX_SLOTS);
    ui_snapshot_slot_info_t prev_info = {};
    if (state->slots[slot_index].valid) {
        prev_info = state->slots[slot_index].info;
    }
    state->slots[slot_index].valid = true;
    state->slots[slot_index].info = *slot_info;
    return prev_info;
}
#endif