#pragma once
/*#
    # ui_dasm.h

    Disassembler UI using Dear ImGui.

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

    You need to include the following headers before including the
    *implementation*:

        - imgui.h
        - ui_util.h
        - z80dasm.h

    All strings provided to ui_dasm_init() must remain alive until
    ui_dasm_discard() is called!

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

#define UI_DASM_MAX_LAYERS (8)
#define UI_DASM_MAX_STRLEN (32)
#define UI_DASM_NUM_LINES (256)

/* callback for reading a byte from memory */
typedef uint8_t (*ui_dasm_read_t)(int layer, uint16_t addr, void* user_data);

/* setup parameters for ui_dasm_init()

    NOTE: all strings must be static!
*/
typedef struct {
    const char* title;  /* window title */
    const char* layers[UI_DASM_MAX_LAYERS];   /* memory system layer names */
    ui_dasm_read_t read_cb;
    void* user_data;
    int x, y, w, h;     /* initial window pos and size */
} ui_dasm_desc_t;

typedef struct {
    const char* title;
    ui_dasm_read_t read_cb;
    int cur_layer;
    int num_layers;
    const char* layers[UI_DASM_MAX_LAYERS];
    void* user_data;
    int init_x, init_y;
    int init_w, init_h;
    bool open;
    bool valid;
    uint16_t start_addr;
    uint16_t cur_addr;
    int str_pos;
    char str_buf[UI_DASM_MAX_STRLEN];
} ui_dasm_t;

/* initialize a new window */
void ui_dasm_init(ui_dasm_t* win, ui_dasm_desc_t* desc);
/* discard a window */
void ui_dasm_discard(ui_dasm_t* win);
/* open a window */
void ui_dasm_open(ui_dasm_t* win);
/* close a window */
void ui_dasm_close(ui_dasm_t* win);
/* toggle window visibility */
void ui_dasm_toggle(ui_dasm_t* win);
/* return true if window is open */
bool ui_dasm_isopen(ui_dasm_t* win);
/* draw the window */
void ui_dasm_draw(ui_dasm_t* win);

#ifdef __cplusplus
} /* extern "C" */
#endif

/*-- IMPLEMENTATION (include in C++ source) ----------------------------------*/
#ifdef CHIPS_IMPL
#ifndef __cplusplus
#error "implementation must be compiled as C++"
#endif
#include <string.h> /* memset */
#include <stdio.h>  /* sscanf, sprintf (ImGui memory editor) */
#ifndef CHIPS_DEBUG
    #ifdef _DEBUG
        #define CHIPS_DEBUG
    #endif
#endif
#ifndef CHIPS_ASSERT
    #include <assert.h>
    #define CHIPS_ASSERT(c) assert(c)
#endif

void ui_dasm_init(ui_dasm_t* win, ui_dasm_desc_t* desc) {
    CHIPS_ASSERT(win && desc);
    CHIPS_ASSERT(desc->title);
    memset(win, 0, sizeof(ui_dasm_t));
    win->title = desc->title;
    win->read_cb = desc->read_cb;
    win->user_data = desc->user_data;
    win->init_x = desc->x;
    win->init_y = desc->y;
    win->init_w = desc->w;
    win->init_h = desc->h;
    for (int i = 0; i < UI_DASM_MAX_LAYERS; i++) {
        if (desc->layers[i]) {
            win->num_layers++;
            win->layers[i] = desc->layers[i];
        }
        else {
            break;
        }
    }
    win->valid = true;
}

void ui_dasm_discard(ui_dasm_t* win) {
    CHIPS_ASSERT(win && win->valid);
    win->valid = false;
}

void ui_dasm_open(ui_dasm_t* win) {
    CHIPS_ASSERT(win && win->valid);
    win->open = true;
}

void ui_dasm_close(ui_dasm_t* win) {
    CHIPS_ASSERT(win && win->valid);
    win->open = false;
}

void ui_dasm_toggle(ui_dasm_t* win) {
    CHIPS_ASSERT(win && win->valid);
    win->open = !win->open;
}

bool ui_dasm_isopen(ui_dasm_t* win) {
    CHIPS_ASSERT(win && win->valid);
    return win->open;
}

static uint8_t _ui_dasm_in_cb(void* user_data) {
    ui_dasm_t* win = (ui_dasm_t*) user_data;
    return win->read_cb(win->cur_layer, win->cur_addr++, win->user_data);
}

static void _ui_dasm_out_cb(char c, void* user_data) {
    ui_dasm_t* win = (ui_dasm_t*) user_data;
    if ((win->str_pos+1) < UI_DASM_MAX_STRLEN) {
        win->str_buf[win->str_pos++] = c;
        win->str_buf[win->str_pos] = 0;
    }
}

static void _ui_dasm_disasm(ui_dasm_t* win) {
    win->str_pos = 0;
    z80dasm_op(win->cur_addr, _ui_dasm_in_cb, _ui_dasm_out_cb, win);
}

void ui_dasm_draw(ui_dasm_t* win) {
    CHIPS_ASSERT(win && win->valid && win->title);
    if (!win->open) {
        return;
    }

    ImGui::SetNextWindowPos(ImVec2(win->init_x, win->init_y), ImGuiSetCond_Once);
    ImGui::SetNextWindowSize(ImVec2(win->init_w, win->init_h), ImGuiSetCond_Once);
    if (ImGui::Begin(win->title, &win->open)) {
        ImGui::BeginChild("##scrolling", ImVec2(0, -(4+ImGui::GetFrameHeightWithSpacing())));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0,0));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0,0));
        const float line_height = ImGui::GetTextLineHeight();
        const float glyph_width = ImGui::CalcTextSize("F").x;
        const float cell_width = 3 * glyph_width;
        ImGuiListClipper clipper(UI_DASM_NUM_LINES, line_height);

        /* skip hidden lines */
        win->cur_addr = win->start_addr;
        for (int line_i = 0; (line_i < clipper.DisplayStart) && (line_i < UI_DASM_NUM_LINES); line_i++) {
            _ui_dasm_disasm(win);
        }

        /* visible items */
        for (int line_i = clipper.DisplayStart; line_i < clipper.DisplayEnd; line_i++) {
            const int op_addr = (int)win->cur_addr;
            _ui_dasm_disasm(win);
            const int num_bytes = (int)win->cur_addr - op_addr;

            /* address */
            ImGui::Text("%04X: ", op_addr);
            ImGui::SameLine();

            /* instruction bytes */
            const float line_start_x = ImGui::GetCursorPosX();
            for (int n = 0; n < num_bytes; n++) {
                ImGui::SameLine(line_start_x + cell_width * n);
                uint8_t val = win->read_cb(win->cur_layer, op_addr+n, win->user_data);
                ImGui::Text("%02X ", val);
            }

            /* disassembled instruction */
            ImGui::SameLine(line_start_x + cell_width * 4 + glyph_width * 2);
            ImGui::Text("%s", win->str_buf);
        }
        clipper.End();
        ImGui::PopStyleVar(2);
        ImGui::EndChild();

        ImGui::Separator();
        win->start_addr = ui_util_hex16("##addr", win->start_addr);
        ImGui::SameLine();
        ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth());
        ImGui::Combo("##layer", &win->cur_layer, win->layers, win->num_layers);
        ImGui::PopItemWidth();
    }
    ImGui::End();
}
#endif /* CHIPS_IMPL */
