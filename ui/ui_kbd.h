#pragma once
/*#
    # ui_kbd.h

    Keyboard matrix visualization (for kbd.h).

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
        - kbd.h

    Include the following headers before including the *implementation*:
        - imgui.h
        - kbd.h

    All string data provided to the ui_kbd_init() must remain alive until
    until ui_kbd_discard() is called!

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

#define UI_KBD_MAX_LAYERS (KBD_MAX_MOD_KEYS+1)

/* setup params for ui_kbd_init()
    NOTE that string data must remain alive until ui_kbd_discard()
*/
typedef struct {
    const char* title;      /* window title */
    const char* layers[UI_KBD_MAX_LAYERS]; /* modifier keys / matrix layers (at least 1) */
    kbd_t* kbd;             /* kbd_t instance to track */
    int x, y;               /* initial window position */
    bool open;              /* initial open state */
} ui_kbd_desc_t;

typedef struct {
    const char* title;
    kbd_t* kbd;
    int cur_layer;
    int num_layers;
    const char* layers[UI_KBD_MAX_LAYERS];
    float init_x, init_y;
    float left_padding, top_padding;
    float cell_width, cell_height;
    int num_columns, num_lines;
    uint32_t last_key_mask;
    int last_key_frame_count;
    bool open;
    bool valid;
    int keymap[UI_KBD_MAX_LAYERS][KBD_MAX_COLUMNS][KBD_MAX_LINES];
} ui_kbd_t;

void ui_kbd_init(ui_kbd_t* win, const ui_kbd_desc_t* desc);
void ui_kbd_discard(ui_kbd_t* win);
void ui_kbd_draw(ui_kbd_t* win);

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

/* extract line, column and modifier bits from key mask */
static inline uint32_t _ui_kbd_line_bits(uint32_t m) {
    return m & ((1<<KBD_MAX_LINES)-1);
}

static inline uint32_t _ui_kbd_column_bits(uint32_t m) {
    return (m>>KBD_MAX_LINES) & ((1<<KBD_MAX_COLUMNS)-1);
}

static inline uint32_t _ui_kbd_mod_bits(uint32_t m) {
    return (m >> (KBD_MAX_COLUMNS+KBD_MAX_LINES)) & ((1<<KBD_MAX_MOD_KEYS)-1);
}

static inline int _ui_kbd_line(uint32_t line_mask) {
    CHIPS_ASSERT(line_mask != 0);
    for (int i = 0; i < KBD_MAX_LINES; i++) {
        if (line_mask & (1<<i)) {
            return i;
        }
    }
    return 0;
}

static inline int _ui_kbd_column(uint32_t col_mask) {
    CHIPS_ASSERT(col_mask != 0);
    for (int i = 0; i < KBD_MAX_LINES; i++) {
        if (col_mask & (1<<i)) {
            return i;
        }
    }
    return 0;
}

void ui_kbd_init(ui_kbd_t* win, const ui_kbd_desc_t* desc) {
    CHIPS_ASSERT(win && desc);
    CHIPS_ASSERT(desc->title);
    CHIPS_ASSERT(desc->kbd);
    CHIPS_ASSERT(desc->layers[0]);
    memset(win, 0, sizeof(ui_kbd_t));
    win->title = desc->title;
    win->kbd = desc->kbd;
    for (; win->num_layers < UI_KBD_MAX_LAYERS; win->num_layers++) {
        if (desc->layers[win->num_layers]) {
            win->layers[win->num_layers] = desc->layers[win->num_layers];
        }
        else {
            break;
        }
    }
    win->init_x = (float) desc->x;
    win->init_y = (float) desc->y;
    win->left_padding = 32.0f;
    win->top_padding = 20.0f;
    win->cell_width = 32.0f;
    win->cell_height = 32.0f;
    win->open = desc->open;

    /* get matrix size */
    uint32_t key_bits = 0;
    for (int i = 0; i < KBD_MAX_KEYS; i++) {
        key_bits |= win->kbd->key_masks[i];
    }
    uint32_t line_bits = _ui_kbd_line_bits(key_bits);
    uint32_t column_bits = _ui_kbd_column_bits(key_bits);
    for (int i = 0; i < 16; i++) {
        if (column_bits & (1<<i)) {
            win->num_columns = i+1;
        }
        if (line_bits & (1<<i)) {
            win->num_lines = i+1;
        }
    }

    /* create a keymap of printable key names */
    for (int key = 0; key < KBD_MAX_KEYS; key++) {
        key_bits = win->kbd->key_masks[key];
        if (key_bits == 0) {
            continue;
        }
        line_bits = _ui_kbd_line_bits(key_bits);
        column_bits = _ui_kbd_column_bits(key_bits);
        uint32_t mod_bits = _ui_kbd_mod_bits(key_bits);
        if ((line_bits != 0) && (column_bits != 0)) {
            int line = _ui_kbd_line(line_bits);
            int column = _ui_kbd_column(column_bits);
            CHIPS_ASSERT((line >= 0) && (line < KBD_MAX_LINES));
            CHIPS_ASSERT((column >= 0) && (column < KBD_MAX_COLUMNS));
            if (mod_bits == 0) {
                win->keymap[0][column][line] = key;
            }
            else {
                for (int mod = 0; mod < KBD_MAX_MOD_KEYS; mod++) {
                    if (mod_bits & (1<<mod)) {
                        win->keymap[mod+1][column][line] = key;
                    }
                }
            }
        }
    }
    win->valid = true;
}

void ui_kbd_discard(ui_kbd_t* win) {
    CHIPS_ASSERT(win && win->valid);
    win->valid = false;
}

static void _ui_kbd_draw_plane_combo(ui_kbd_t* win) {
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Modifier:");
    ImGui::SameLine();
    ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth());
    ImGui::Combo("##mod_keys", &win->cur_layer, win->layers, win->num_layers);
    ImGui::PopItemWidth();
}

static void _ui_kbd_draw_matrix(ui_kbd_t* win, const ImVec2& canvas_pos, const ImVec2& canvas_size, uint32_t mask) {
    const uint32_t line_bits = _ui_kbd_line_bits(mask);
    const uint32_t column_bits = _ui_kbd_column_bits(mask);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    const float dx = win->cell_width;
    const float dy = win->cell_height;
    const float x0 = canvas_pos.x + win->left_padding;
    const float y0 = canvas_pos.y + win->top_padding;
    const float x1 = x0 + win->num_columns * dx;
    const float y1 = y0 + win->num_lines * dy;
    const float tdy = ImGui::GetTextLineHeight();
    const float tdx = -10.0f;
    const ImU32 grid_color = ui_util_color(ImGuiCol_Text);
    const ImU32 down_color = ImColor(1.0f, 0.0f, 0.0f, ImGui::GetStyle().Alpha);
    char buf[32];
    float y = y0;
    for (int l=0; l<win->num_lines; l++, y+=dy) {
        snprintf(buf, sizeof(buf), "%d", l);
        const uint32_t down = line_bits & (1<<l);
        dl->AddLine(ImVec2(x0, y+dy), ImVec2(x1, y+dy), down?down_color:grid_color, down?2.0f:1.0f);
        dl->AddText(ImVec2(x0, y+dy-tdy), grid_color, buf);
    }
    float x = x0;
    for (int c=0; c<win->num_columns; c++, x+=dx) {
        snprintf(buf, sizeof(buf), "%d", c);
        const uint32_t down = column_bits & (1<<c);
        dl->AddLine(ImVec2(x+dx, y0), ImVec2(x+dx, y1), down?down_color:grid_color, down?2.0f:1.0f);
        dl->AddText(ImVec2(x+dx+tdx, y0), grid_color, buf);
    }
    y = y0;
    buf[1] = 0;
    for (int l=0; l<win->num_lines; l++, y+=dy) {
        x = x0;
        for (int c=0; c<win->num_columns; c++, x+=dx) {
            const bool down = (line_bits & (1<<l)) && (column_bits & (1<<c));
            if (down) {
                dl->AddCircleFilled(ImVec2(x+dx,y+dy), 4.0f, down_color, 12);
            }
            int key = win->keymap[win->cur_layer][c][l];
            /* FIXME: special keys (arrow, enter, ...) */
            if ((key > 32) && (key < 128)) {
                buf[0] = key;
                dl->AddText(ImVec2(x+dx+tdx, y+dy-tdy), grid_color, buf);
            }
        }
    }
}

void ui_kbd_draw(ui_kbd_t* win) {
    CHIPS_ASSERT(win && win->valid && win->title && win->kbd);
    if (!win->open) {
        return;
    }
    ImGui::SetNextWindowPos(ImVec2(win->init_x, win->init_y), ImGuiCond_Once);
    const float min_w = win->num_columns * win->cell_width + win->left_padding + 40.0f;
    const float min_h = win->num_lines * win->cell_height + win->top_padding + 64.0f;
    ImGui::SetNextWindowSize(ImVec2(min_w, min_h), ImGuiCond_Once);
    ImGui::SetNextWindowSizeConstraints(ImVec2(min_w, min_h), ImVec2(FLT_MAX, FLT_MAX));
    if (ImGui::Begin(win->title, &win->open)) {
        _ui_kbd_draw_plane_combo(win);
        const ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
        const ImVec2 canvas_size = ImGui::GetContentRegionAvail();
        uint32_t mask = 0;
        for (int i = 0; i < KBD_MAX_PRESSED_KEYS; i++) {
            mask |= win->kbd->key_buffer[i].mask;
        }
        if (mask != 0) {
            win->last_key_frame_count = 0;
            win->last_key_mask = mask;
        }
        if (win->last_key_mask && (win->last_key_frame_count++ > 30)) {
            win->last_key_mask = 0;
        }
        _ui_kbd_draw_matrix(win, canvas_pos, canvas_size, win->last_key_mask);
    }
    ImGui::End();
}
#endif /* CHIPS_IMPL */
