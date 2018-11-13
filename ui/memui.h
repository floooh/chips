#pragma once
/*#
    # memui.h

    Memory viewer/editor UI.

    Do this:
    ~~~C
    #define CHIPS_IMPL
    ~~~
    before you include this file in *one* C or C++ file to create the 
    implementation.

    Optionally provide the following macros with your own implementation
    
    ~~~C
    CHIPS_ASSERT(c)
    ~~~
        your own assert macro (default: assert(c))

    You need to include the following headers before including the
    *implementation*:

        - imgui.h
        - ui/thirdparty/imgui_memory_editor.h

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

#ifndef CHIPS_IMPL
typedef void MemoryEditor;
#endif

#define MEMUI_MAX_LAYERS (8)

/* callbacks for reading and writing bytes */
typedef uint8_t (*memui_read_t)(int layer, uint16_t addr, void* user_data);
typedef void (*memui_write_t)(int layer, uint16_t addr, uint8_t data, void* user_data);

/* setup parameters for memui_init()

    NOTE: all strings must be static!
*/
typedef struct {
    const char* window_title;
    const char* layers[MEMUI_MAX_LAYERS];
    memui_read_t read_cb;
    memui_write_t write_cb;
    void* user_data;
    bool read_only;
} memui_desc_t;

typedef struct {
    const char* window_title;
    const char* layers[MEMUI_MAX_LAYERS];
    memui_read_t read_cb;
    memui_write_t write_cb;
    void* user_data;
    int num_layers;
    int cur_layer;
    bool open;
    MemoryEditor* ed;
} memui_t;

/* initialize a new memui window */
void memui_init(memui_t* win, memui_desc_t* desc);
/* discard a memui window (frees memory) */
void memui_discard(memui_t* win);
/* open the memui window */
void memui_open(memui_t* win);
/* close the memui window */
void memui_close(memui_t* win);
/* toggle visibility */
void memui_toggle(memui_t* win);
/* return true if memui window is open */
bool memui_isopen(memui_t* win);
/* draw the memui window */
void memui_draw(memui_t* win);

#ifdef __cplusplus
} /* extern "C" */
#endif

/*-- IMPLEMENTATION (include in C++ source) ----------------------------------*/
#ifdef CHIPS_IMPL
#ifndef __cplusplus
#error "memui.h implementation must be compiled as C++"
#endif
#include <string.h>
#ifndef CHIPS_DEBUG
    #ifdef _DEBUG
        #define CHIPS_DEBUG
    #endif
#endif
#ifndef CHIPS_ASSERT
    #include <assert.h>
    #define CHIPS_ASSERT(c) assert(c)
#endif

/* ImGui MemoryEditor read/write callbacks */
static uint8_t _memui_readfn(uint8_t* ptr, size_t off) {
    /* we'll treat the "data ptr" as "user data" */
    //const memui_t* win = (memui_t*) ptr;
    return 0;
}

static void _memui_writefn(uint8_t* ptr, size_t off, uint8_t val) {
    /* we'll treat the "data ptr" as "user data" */
    //const memui_t* win = (memui_t*) ptr;
}

void memui_init(memui_t* win, memui_desc_t* desc) {
    CHIPS_ASSERT(win && desc);
    CHIPS_ASSERT(desc->window_title);
    memset(win, 0, sizeof(memui_t));
    win->window_title = desc->window_title;
    win->read_cb = desc->read_cb;
    win->write_cb = desc->write_cb;
    win->user_data = desc->user_data;
    for (int i = 0; i < MEMUI_MAX_LAYERS; i++) {
        if (desc->layers[i]) {
            win->layers[i] = desc->layers[i];
            win->num_layers++;
        }
        else {
            break;
        }
    }
    win->ed = new MemoryEditor;
    win->ed->ReadOnly = desc->read_only;
    win->ed->ReadFn = _memui_readfn;
    win->ed->WriteFn = _memui_writefn;
}

void memui_discard(memui_t* win) {
    CHIPS_ASSERT(win && win->ed);
    delete win->ed; win->ed = nullptr;
}

void memui_open(memui_t* win) {
    CHIPS_ASSERT(win);
    win->open = true;
}

void memui_close(memui_t* win) {
    CHIPS_ASSERT(win);
    win->open = false;
}

void memui_toggle(memui_t* win) {
    CHIPS_ASSERT(win);
    win->open = !win->open;
}

bool memui_isopen(memui_t* win) {
    CHIPS_ASSERT(win);
    return win->open;
}

void memui_draw(memui_t* win) {
    // FIXME
}

#endif /* CHIPS_IMPL */
