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
        - ui_util.h

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

#define UI_MEMMAP_MAX_LAYERS (16)
#define UI_MEMMAP_MAX_REGIONS (16)

/* describes a memory region in a layer */
typedef struct {
    const char* name;                   /* region name (must be a static string) */
    uint16_t addr;                      /* start address */
    int len;                            /* length in bytes (up to 0x10000) */
    bool on;                            /* on or off? */
} ui_memmap_region_t;

/* describes a memory layer */
typedef struct {
    const char* name;           /* layer name (must be a static string) */
    int num_regions;
    ui_memmap_region_t regions[UI_MEMMAP_MAX_REGIONS];  /* memory regions in layer */
} ui_memmap_layer_t;

/* setup params for ui_memmap_init() */
typedef struct {
    const char* title;  /* window title */
    int x, y;           /* initial window pos */
    int w, h;           /* initial window size, or 0 for default size */
    bool open;          /* initial open state */
} ui_memmap_desc_t;

/* ui_memmap window state */
typedef struct {
    const char* title;
    float init_x, init_y;
    float init_w, init_h;
    float layer_height;
    float left_padding;
    bool open;
    bool valid;
    int num_layers;
    ui_memmap_layer_t layers[UI_MEMMAP_MAX_LAYERS];
} ui_memmap_t;

void ui_memmap_init(ui_memmap_t* win, const ui_memmap_desc_t* desc);
void ui_memmap_discard(ui_memmap_t* win);
void ui_memmap_draw(ui_memmap_t* win);

/* reset/clear memory map description */
void ui_memmap_reset(ui_memmap_t* win);
/* add a layer to the memory map description (call after ui_memmap_reset) */
void ui_memmap_layer(ui_memmap_t* win, const char* name);
/* add a region in the last added layer of the memory map description */
void ui_memmap_region(ui_memmap_t* win, const char* name, uint16_t addr, int len, bool on);

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

static void _ui_memmap_draw_grid(ui_memmap_t* win, const ImVec2& canvas_pos, const ImVec2& canvas_area) {
    ImDrawList* l = ImGui::GetWindowDrawList();
    const ImU32 grid_color = ui_util_color(ImGuiCol_Text);
    const float y1 = canvas_pos.y + canvas_area.y - win->layer_height;
    /* line rulers */
    if (canvas_area.x > win->left_padding) {
        static const char* addr[5] = { "0000", "4000", "8000", "C000", "FFFF" };
        const float glyph_width = ImGui::CalcTextSize("X").x;
        const float x0 = canvas_pos.x + win->left_padding;
        float dx = (float)(int)((canvas_area.x - win->left_padding) / 4.0f);
        const float y0 = canvas_pos.y;
        const float y1 = canvas_pos.y + canvas_area.y + 4.0f - win->layer_height;
        float x = x0;
        for (int i = 0; i < 5; i++, x += dx) {
            l->AddLine(ImVec2(x, y0), ImVec2(x, y1), grid_color);
            const float addr_x = (i == 4) ? x - 4.0f*glyph_width : x;
            l->AddText(ImVec2(addr_x, y1), grid_color, addr[i]);
        }
        const ImVec2 p0(canvas_pos.x + win->left_padding, y1);
        const ImVec2 p1(x0 + 4*dx, p0.y);
        l->AddLine(p0, p1, grid_color);
    }
    /* layer names to the left */
    ImVec2 text_pos(canvas_pos.x, (y1 - win->layer_height + 6));
    for (int i = 0; i < win->num_layers; i++) {
        l->AddText(text_pos, grid_color, win->layers[i].name);
        text_pos.y -= win->layer_height;
    }
}

static void _ui_memmap_draw_region(ui_memmap_t* win, const ImVec2& pos, float width, const ui_memmap_region_t& reg) {
    const float alpha = ImGui::GetStyle().Alpha;
    const ImU32 on_color = ImColor(0.0f, 0.75f, 0.0f, alpha);
    const ImU32 off_color = ImColor(0.0f, 0.25f, 0.0f, alpha);
    ImU32 color = reg.on ? on_color : off_color;
    uint32_t addr = reg.addr;
    uint32_t end_addr = reg.addr + reg.len;
    if (end_addr > 0x10000) {
        /* wraparound */
        ImVec2 a(pos.x, pos.y);
        ImVec2 b((pos.x + (((end_addr & 0xFFFF) * width) / 0x10000))-2, (pos.y + win->layer_height)-2);
        ImGui::GetWindowDrawList()->AddRectFilled(a, b, color);
        if (ImGui::IsMouseHoveringRect(a, b)) {
            ImGui::SetTooltip("%s (%04X..%04X)", reg.name, 0, (end_addr&0xFFFF)-1);
        }
        end_addr = 0x10000;
    }
    ImVec2 a((pos.x + ((addr * width) / 0x10000)), pos.y);
    ImVec2 b((pos.x + ((end_addr * width) / 0x10000))-2, (pos.y + win->layer_height)-2);
    ImGui::GetWindowDrawList()->AddRectFilled(a, b, color);
    if (ImGui::IsMouseHoveringRect(a, b)) {
        ImGui::SetTooltip("%s (%04X..%04X)", reg.name, addr, end_addr-1);
    }
}

static void _ui_memmap_draw_regions(ui_memmap_t* win, const ImVec2& canvas_pos, const ImVec2& canvas_area) {
    ImVec2 pos(canvas_pos.x + win->left_padding, canvas_pos.y + canvas_area.y + 4 - 2*win->layer_height);
    for (int li = 0; li < win->num_layers; li++, pos.y -= win->layer_height) {
        for (int ri = 0; ri < win->layers[li].num_regions; ri++) {
            const ui_memmap_region_t& reg = win->layers[li].regions[ri];
            if (reg.name) {
                _ui_memmap_draw_region(win, pos, canvas_area.x - win->left_padding, reg);
            }
        }
    }
}

void ui_memmap_init(ui_memmap_t* win, const ui_memmap_desc_t* desc) {
    CHIPS_ASSERT(win && desc);
    CHIPS_ASSERT(desc->title);
    memset(win, 0, sizeof(ui_memmap_t));
    win->title = desc->title;
    win->init_x = (float) desc->x;
    win->init_y = (float) desc->y;
    win->init_w = (float) ((desc->w == 0) ? 400 : desc->w);
    win->init_h = (float) ((desc->h == 0) ? 40 : desc->h);
    win->open = desc->open;
    win->left_padding = 80.0f;;
    win->layer_height = 20.0f;
    win->valid = true;
}

void ui_memmap_discard(ui_memmap_t* win) {
    CHIPS_ASSERT(win && win->valid);
    win->valid = false;
}

void ui_memmap_draw(ui_memmap_t* win) {
    CHIPS_ASSERT(win && win->valid && win->title);
    if (!win->open) {
        return;
    }
    const float min_height = 40.0f + ((win->num_layers + 1.0f) * win->layer_height);
    ImGui::SetNextWindowPos(ImVec2(win->init_x, win->init_y), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(win->init_w, win->init_h), ImGuiCond_Once);
    ImGui::SetNextWindowSizeConstraints(ImVec2(120.0f, min_height), ImVec2(FLT_MAX,FLT_MAX));
    if (ImGui::Begin(win->title, &win->open)) {
        const ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
        const ImVec2 canvas_area = ImGui::GetContentRegionAvail();
        _ui_memmap_draw_regions(win, canvas_pos, canvas_area);
        _ui_memmap_draw_grid(win, canvas_pos, canvas_area);
    }
    ImGui::End();
}

void ui_memmap_reset(ui_memmap_t* win) {
    CHIPS_ASSERT(win && win->valid);
    win->num_layers = 0;
    memset(win->layers, 0, sizeof(win->layers));
}

void ui_memmap_layer(ui_memmap_t* win, const char* name) {
    CHIPS_ASSERT(win && win->valid);
    CHIPS_ASSERT((win->num_layers >= 0) && (win->num_layers < UI_MEMMAP_MAX_LAYERS));
    CHIPS_ASSERT(name);
    win->layers[win->num_layers++].name = name;
}

void ui_memmap_region(ui_memmap_t* win, const char* name, uint16_t addr, int len, bool on) {
    CHIPS_ASSERT(win && win->valid);
    CHIPS_ASSERT((win->num_layers > 0) && (win->num_layers <= UI_MEMMAP_MAX_LAYERS));
    CHIPS_ASSERT((len >= 0) && (len <= 0x10000));
    CHIPS_ASSERT(name);
    ui_memmap_layer_t& layer = win->layers[win->num_layers-1];
    CHIPS_ASSERT((layer.num_regions >= 0) && (layer.num_regions < UI_MEMMAP_MAX_REGIONS));
    ui_memmap_region_t& reg = layer.regions[layer.num_regions++];
    reg.name = name;
    reg.addr = addr;
    reg.len = len;
    reg.on = on;
}

#endif /* CHIPS_IMPL */
