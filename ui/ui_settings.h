#pragma once
/*#
    # ui_settings.h

    Persistent settings helper.

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

    Copyright (c) 2024 Andre Weissflog
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

#define UI_SETTINGS_MAX_SLOTS (32)
#define UI_SETTINGS_MAX_STRING_LENGTH (128)

typedef struct {
    char buf[UI_SETTINGS_MAX_STRING_LENGTH];
} ui_settings_str_t;

typedef struct {
    ui_settings_str_t window_title;
    bool open;
} ui_settings_slot_t;

typedef struct {
    int num_slots;
    ui_settings_slot_t slots[UI_SETTINGS_MAX_SLOTS];
} ui_settings_t;

// initialize settings instance
void ui_settings_init(ui_settings_t* state);
// add a settings item
bool ui_settings_add(ui_settings_t* state, const char* window_title, bool open);
// find slot index, return -1 if not found
int ui_settings_find_slot_index(ui_settings_t* state, const char* window_title);
// check window open settings flag
bool ui_settings_isopen(const ui_settings_t* state, const char* window_title);

#ifdef __cplusplus
} // extern "C"
#endif

//-- IMPLEMENTATION ------------------------------------------------------------
#ifdef CHIPS_UI_IMPL
#include <stdio.h>  // snprintf
#ifndef CHIPS_ASSERT
    #include <assert.h>
    #define CHIPS_ASSERT(c) assert(c)
#endif

void ui_settings_init(ui_settings_t* state) {
    CHIPS_ASSERT(state);
    memset(state, 0, sizeof(*state));
}

static ui_settings_str_t ui_settings_make_str(const char* str) {
    ui_settings_str_t res = {0};
    snprintf(res.buf, sizeof(res.buf), "%s", str);
    return res;
}

bool ui_settings_add(ui_settings_t* state, const char* window_title, bool open) {
    CHIPS_ASSERT(state);
    CHIPS_ASSERT(window_title);
    if (state->num_slots >= UI_SETTINGS_MAX_SLOTS) {
        return false;
    }
    if (strlen(window_title) >= (UI_SETTINGS_MAX_STRING_LENGTH - 1)) {
        return false;
    }
    ui_settings_slot_t* slot = &state->slots[state->num_slots++];
    slot->window_title = ui_settings_make_str(window_title);
    slot->open = open;
    return true;
}

int ui_settings_find_slot_index(const ui_settings_t* state, const char* window_title) {
    CHIPS_ASSERT(state && window_title);
    for (int i = 0; i < state->num_slots; i++) {
        const ui_settings_slot_t* slot = &state->slots[i];
        CHIPS_ASSERT(slot->window_title.buf);
        if (strcmp(window_title, slot->window_title.buf) == 0) {
            return i;
        }
    }
    return -1;
}

bool ui_settings_isopen(const ui_settings_t* state, const char* window_title) {
    CHIPS_ASSERT(state && window_title);
    int slot_index = ui_settings_find_slot_index(state, window_title);
    if (slot_index != -1) {
        return state->slots[slot_index].open;
    }
    return false;
}
#endif // CHIPS_UI_IMPL
