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
    UI_MEMMAP_REGIONSTATE_INACTIVE,     /* not currently active */
    UI_MEMMAP_REGIONSTATE_ACTIVE,       /* active and CPU-visible */
    UI_MEMMAP_REGIONSTATE_HIDDEN,       /* active, but hidden behind a higher-priority region */
} ui_memmap_regionstate_t;

/* describes a memory region in a layer */
typedef struct {
    const char* name;                   /* region name (must be a static string) */
    ui_memmap_regionstate_t state;      /* current mapping-state of the region */
    uint16_t addr;                      /* start address */
    uint16_t len;                       /* length in bytes */
    const char* desc;                   /* description (for tooltip, must be a static string!) */
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
void ui_memmap_draw(ui_memmap_t* win, ui_memmap_state_t* state);

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

void ui_memmap_init(ui_memmap_t* win, ui_memmap_desc_t* desc) {
    CHIPS_ASSERT(win && desc);
    CHIPS_ASSERT(desc->title);
    memset(win, 0, sizeof(ui_memmap_t));
    win->title = desc->title;
    win->init_x = desc->x;
    win->init_y = desc->y;
    win->init_w = desc->w;
    win->init_h = desc->h;
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

void ui_memmap_draw(ui_memmap_t* win, ui_memmap_state_t* state) {
    CHIPS_ASSERT(win && win->valid);
    CHIPS_ASSERT(state);
    if (!win->open) {
        return;
    }
    ImGui::SetNextWindowPos(ImVec2(win->init_x, win->init_y), ImGuiSetCond_Once);
    ImGui::SetNextWindowSize(ImVec2(win->init_w, win->init_h), ImGuiSetCond_Once);
    if (ImGui::Begin(win->title, &win->open)) {
        // FIXME
    }
    ImGui::End();
}

#endif /* CHIPS_IMPL */
