#pragma once
/*#
    # ui_memmap.h

    Generic home computer memory map visualization UI.

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

    All strings provided to ui_memmap_init() must remain alive until
    ui_memmap_discard() is called!

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

#define UI_MEMMAP_MAX_LAYERS (8)
#define UI_MEMMAP_MAX_REGIONS (16)

/* memory region states */
typedef enum {
    UI_MEMMAP_REGIONSTATE_OFF,          /* not currently active */
    UI_MEMMAP_REGIONSTATE_ACTIVE,       /* active and CPU-visible */
    UI_MEMMAP_REGIONSTATE_HIDDEN,       /* active, but hidden behind a higher-priority region */
} ui_memmap_regionstate_t;

/* describes a memory region in a layer */
typedef struct {
    const char* name;                   /* region name (must be a static string) */
    ui_memmap_regionstate_t state;      /* current mapping-state of the region */
    uint16_t addr;                      /* start address */
    uint16_t len;                       /* length in bytes */
} ui_memmap_region_t;

/* describes a memory layer */
typedef struct {
    const char* name;           /* layer name (must be a static string) */
    ui_memmap_region_t regions[UI_MEMMAP_MAX_REGIONS];  /* memory regions in layer */
} ui_memmap_layer_t;

/* describes the entire memory mapping state */
typedef struct {
    ui_memmap_layer_t layers[UI_MEMMAP_MAX_LAYERS];
} ui_memmap_state_t;

/* setup params for ui_memmap_init() */
typedef struct {
    const char* title;  /* window title */
    int x, y, w, h;     /* initial window pos and size */
} ui_memmap_desc_t;

/* ui_memmap window state */
typedef struct {
    const char* title;
    int init_x, init_y;
    int init_w, init_h;
    int layer_height;
    int left_padding;
    uint32_t grid_color;
    uint32_t off_color;
    uint32_t active_color;
    uint32_t hidden_color;
    bool open;
    bool valid;
} ui_memmap_t;

/* initialize a new window */
void ui_memmap_init(ui_memmap_t* win, ui_memmap_desc_t* desc);
/* discard a window */
void ui_memmap_discard(ui_memmap_t* win);
/* open a window */
void ui_memmap_open(ui_memmap_t* win);
/* close a window */
void ui_memmap_close(ui_memmap_t* win);
/* toggle window visibility */
void ui_memmap_toggle(ui_memmap_t* win);
/* return true if window is open */
bool ui_memmap_isopen(ui_memmap_t* win);
/* draw the window */
void ui_memmap_draw(ui_memmap_t* win, const ui_memmap_state_t* state);

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

static const int _ui_memmap_left_padding = 80;
static const int _ui_memmap_layer_height = 20;

static int _ui_memmap_num_layers(const ui_memmap_state_t* state) {
    int num_layers = 0;
    for (int i = 0; i < UI_MEMMAP_MAX_LAYERS; i++) {
        if (state->layers[i].name) {
            num_layers++;
        }
        else {
            break;
        }
    }
    return num_layers;
}

static void _ui_memmap_draw_grid(ui_memmap_t* win, const ui_memmap_state_t* state, const ImVec2& canvas_pos, const ImVec2& canvas_area) {
    ImDrawList* l = ImGui::GetWindowDrawList();
    const int y1 = canvas_pos.y + canvas_area.y - win->layer_height;
    /* line rulers */
    if (canvas_area.x > win->left_padding) {
        static const char* addr[5] = { "0000", "4000", "8000", "C000", "FFFF" };
        const float glyph_width = ImGui::CalcTextSize("X").x;
        const int x0 = canvas_pos.x + win->left_padding;
        int dx = (canvas_area.x - win->left_padding) / 4;
        const int y0 = canvas_pos.y;
        const int y1 = canvas_pos.y + canvas_area.y + 4 - win->layer_height;
        for (int i = 0, x = x0; i < 5; i++, x += dx) {
            l->AddLine(ImVec2(x, y0), ImVec2(x, y1), win->grid_color);
            const float addr_x = (i == 4) ? x - 4*glyph_width : x;
            l->AddText(ImVec2(addr_x, y1), win->grid_color, addr[i]);
        }
        const ImVec2 p0(canvas_pos.x + win->left_padding, y1);
        const ImVec2 p1(x0 + 4*dx, p0.y);
        l->AddLine(p0, p1, win->grid_color);
    }
    /* layer names to the left */
    const int num_layers = _ui_memmap_num_layers(state);
    ImVec2 text_pos(canvas_pos.x, y1 - win->layer_height);
    for (int i = 0; i < num_layers; i++) {
        l->AddText(text_pos, win->grid_color, state->layers[i].name);
        text_pos.y -= win->layer_height;
    }
}

static void _ui_memmap_draw_region(ui_memmap_t* win, const ImVec2& pos, float width, const ui_memmap_region_t& reg) {
    ImU32 color = 0xFFFFFFFF;
    switch (reg.state) {
        case UI_MEMMAP_REGIONSTATE_OFF:     color = win->off_color; break;
        case UI_MEMMAP_REGIONSTATE_ACTIVE:  color = win->active_color; break;
        case UI_MEMMAP_REGIONSTATE_HIDDEN:  color = win->hidden_color; break;
    }
    ImVec2 a((pos.x + ((reg.addr * width) / 0x10000)), pos.y);
    ImVec2 b((pos.x + (((reg.addr+reg.len) * width) / 0x10000))-2, (pos.y + win->layer_height)-2);
    ImGui::GetWindowDrawList()->AddRectFilled(a, b, color);
    if (ImGui::IsMouseHoveringRect(a, b)) {
        ImGui::SetTooltip("%04X..%04X: %s", reg.addr, reg.addr+reg.len-1, reg.name);
    }
}

static void _ui_memmap_draw_regions(ui_memmap_t* win, const ui_memmap_state_t* state, const ImVec2& canvas_pos, const ImVec2& canvas_area) {
    const int num_layers = _ui_memmap_num_layers(state);
    ImVec2 pos(canvas_pos.x + win->left_padding, canvas_pos.y + canvas_area.y + 4 - 2*win->layer_height);
    for (int li = 0; li < num_layers; li++, pos.y -= win->layer_height) {
        for (int ri = 0; ri < UI_MEMMAP_MAX_REGIONS; ri++) {
            const ui_memmap_region_t& reg = state->layers[li].regions[ri];
            if (reg.name) {
                _ui_memmap_draw_region(win, pos, canvas_area.x - win->left_padding, reg);
            }
        }
    }
}

void ui_memmap_init(ui_memmap_t* win, ui_memmap_desc_t* desc) {
    CHIPS_ASSERT(win && desc);
    CHIPS_ASSERT(desc->title);
    memset(win, 0, sizeof(ui_memmap_t));
    win->title = desc->title;
    win->init_x = desc->x;
    win->init_y = desc->y;
    win->init_w = desc->w;
    win->init_h = desc->h;
    win->left_padding = 80;
    win->layer_height = 20;
    win->grid_color = 0xFFAAAAAA;
    win->off_color = 0xFF222222;
    win->active_color = 0xFF00CC00;
    win->hidden_color = 0xFF006600;
    win->valid = true;
}

void ui_memmap_discard(ui_memmap_t* win) {
    CHIPS_ASSERT(win && win->valid);
    win->valid = false;
}

void ui_memmap_open(ui_memmap_t* win) {
    CHIPS_ASSERT(win && win->valid);
    win->open = true;
}

void ui_memmap_close(ui_memmap_t* win) {
    CHIPS_ASSERT(win && win->valid);
    win->open = false;
}

void ui_memmap_toggle(ui_memmap_t* win) {
    CHIPS_ASSERT(win && win->valid);
    win->open = !win->open;
}

bool ui_memmap_isopen(ui_memmap_t* win) {
    CHIPS_ASSERT(win && win->valid);
    return win->open;
}

void ui_memmap_draw(ui_memmap_t* win, const ui_memmap_state_t* state) {
    CHIPS_ASSERT(win && win->valid);
    CHIPS_ASSERT(state);
    if (!win->open) {
        return;
    }
    const int min_height = 40 + (_ui_memmap_num_layers(state)+1) * win->layer_height;
    ImGui::SetNextWindowPos(ImVec2(win->init_x, win->init_y), ImGuiSetCond_Once);
    ImGui::SetNextWindowSize(ImVec2(win->init_w, win->init_h), ImGuiSetCond_Once);
    ImGui::SetNextWindowSizeConstraints(ImVec2(120, min_height), ImVec2(FLT_MAX,FLT_MAX));
    if (ImGui::Begin(win->title, &win->open)) {
        const ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
        const ImVec2 canvas_area = ImGui::GetContentRegionAvail();
        _ui_memmap_draw_regions(win, state, canvas_pos, canvas_area);
        _ui_memmap_draw_grid(win, state, canvas_pos, canvas_area);
    }
    ImGui::End();
}

#endif /* CHIPS_IMPL */
