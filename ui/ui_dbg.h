#pragma once
/*#
    # ui_dbg.h

    CPU debugger UI.

    Do this:
    ~~~C
    #define CHIPS_IMPL
    ~~~
    before you include this file in *one* C++ file to create the 
    implementation.

    Select the supported CPUs with the following macros (at least
    one must be defined):

    UI_DBG_USE_Z80
    UI_DBG_USE_M6502

    Optionally provide the following macros with your own implementation
    
    ~~~C
    CHIPS_ASSERT(c)
    ~~~
        your own assert macro (default: assert(c))

    You need to include the following headers before including the
    *implementation*:

        - imgui.h
        - ui_util.h
        - z80.h         (only if UI_DBG_USE_Z80 is defined)
        - z80dasm.h     (only if UI_DBG_USE_Z80 is defined)
        - m6502.h       (only if UI_DBG_USE_M6502 is defined)
        - m6502dasm.h   (only if UI_DASM_USE_M6502 is defined)

    All strings provided to ui_dbg_init() must remain alive until
    ui_dbg_discard() is called!

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

#define UI_DBG_MAX_HISTORYITEMS (128)
#define UI_DBG_MAX_BREAKPOINTS (32)
#define UI_DBG_MAX_LAYERS (16)
#define UI_DBG_BASE_TRAPID (128)        /* first CPU trap-id used by debugger */

/* standard breakpoint types */
enum {
    UI_DBG_BREAKTYPE_STEP = 0,  /* single-stepping active */
    UI_DBG_BREAKTYPE_EXEC,  /* break on executed address */
    UI_DBG_BREAKTYPE_READ,  /* break on memory read from address */
    UI_DBG_BREAKTYPE_WRITE, /* break on memory write to address */
    UI_DBG_BREAKTYPE_IN,    /* break on IO input from port (Z80 only) */
    UI_DBG_BREAKTYPE_OUT,   /* break on IO output to port (Z80 only) */
    UI_DBG_BREAKTYPE_BYTE,  /* break on a specific 8-bit value at address */
    UI_DBG_BREAKTYPE_WORD,  /* break on a specific 16-bit value at address */
    UI_DBG_BREAKTYPE_IRQ,   /* break on maskable interrupt */
    UI_DBG_BREAKTYPE_NMI,   /* break on non-maskable interrupt */

    UI_DBG_BREAKTYPE_USER,  /* user-defined types start here */
};

/* a breakpoint description */
typedef struct ui_dbg_breakpoint_t {
    int type;           /* UI_DBG_BREAKTYPE_* */
    bool enabled;
    uint16_t addr;
    int val[4];
} ui_dbg_breakpoint_t;

/* callback for reading a byte from memory */
typedef uint8_t (*ui_dbg_read_t)(int layer, uint16_t addr, void* user_data);
/* callback for evaluating breakpoints, return breakpoint index, or -1 */
typedef int (*ui_dbg_break_t)(ui_dbg_breakpoint_t* first, int num, uint16_t pc, uint64_t pins, int ticks, void* user_data);

typedef struct ui_dbg_desc_t {
    const char* title;          /* window title */
    #ifdef UI_DBG_USE_Z80
    z80_t* z80;                 /* Z80 CPU to track */
    #endif
    #ifdef UI_DBG_USE_M6502
    m6502_t* m6502i             /* 6502 CPU to track */
    #endif
    ui_dbg_read_t read_cb;      /* callback to read memory */
    ui_dbg_break_t break_cb;    /* callback for evaluating breakpoints */
    void* user_data;            /* user data for callbacks */
    const char* layers[UI_DBG_MAX_LAYERS];   /* memory system layer names */
    int x, y;                   /* initial window pos */
    int w, h;                   /* initial window size, or 0 for default size */
    bool open;                  /* initial open state */
} ui_dbg_desc_t;

/* an execution history item */
typedef struct ui_dbg_historyitem_t {
    uint64_t pins;
    int ticks;
    uint16_t pc;
} ui_dbg_historyitem_t;

/* an execution history ring buffer */
typedef struct ui_dbg_history_t {
    int head;
    int tail;
    ui_dbg_historyitem_t items[UI_DBG_MAX_HISTORYITEMS];
} ui_dbg_history_t;

/* disassembler state */
typedef struct ui_dbg_dasm_t {
    int cur_layer;
    int num_layers;
    const char* layers[UI_DBG_MAX_LAYERS];
    int str_pos;
    char str_buf[UI_DASM_MAX_STRLEN];
    int bin_pos;
    uint8_t bin_buf[UI_DASM_MAX_BINLEN];
} ui_dbg_dasm_t;

typedef struct ui_dbg_t {
    const char* title;
    bool valid;
    bool open;
    float init_x, init_y;
    float init_w, init_h;
    #ifdef UI_DBG_USE_Z80
    z80_t* z80;
    #endif
    #ifdef UI_DBG_USE_M6502
    m6502_t* m6502;
    #endif
    ui_dbg_read_t read_cb;
    ui_dbg_break_t break_cb;
    void* user_data;
    ui_dbg_dasm_t dasm;
    ui_dbg_history_t history;
} ui_dbg_t;

void ui_dbg_init(ui_dbg_t* win, ui_dbg_desc_t* desc);
void ui_dbg_discard(ui_dbg_t* win);
void ui_dbg_draw(ui_dbg_t* win);

#ifdef __cplusplus
} /* extern "C" */
#endif

/*-- IMPLEMENTATION ----------------------------------------------------------*/
#ifdef CHIPS_IMPL
#include <string.h>
#ifndef CHIPS_ASSERT
    #include <assert.h>
    #define CHIPS_ASSERT(c) assert(c)
#endif
#if !defined(UI_DBG_USE_Z80) && !defined(UI_DBG_USE_M6502)
#error "please define UI_DBG_USE_Z80 and/or UI_DBG_USE_M6502"
#endif

static void _ui_dbg_history_init(ui_dbg_history_t* hist, ui_dbg_desc_t* desc) {
    /* nothing to do here, since hist is already zero-initialized */
}

static void _ui_dbg_dasm_init(ui_dbg_dasm_t* dasm, ui_dbg_desc_t* desc) {
    /* assuming dasm is already zero-initialized */
    for (int i = 0; i < UI_DBG_MAX_LAYERS; i++) {
        if (desc->layers[i]) {
            dasm->num_layers++;
            dasm->layers[i] = desc->layers[i];
        }
        else {
            break;
        }
    }
}

void ui_dbg_init(ui_dbg_t* win, ui_dbg_desc_t* desc) {
    CHIPS_ASSERT(win && desc);
    CHIPS_ASSERT(desc->title);
    CHIPS_ASSERT(desc->read_cb && desc->break_cb);
    #if defined(UI_DBG_USE_Z80) &&  defined(UI_DBG_USE_M6502)
        CHIPS_ASSERT((desc->z80 || desc->m6502) && !(desc->z80 && desc->m6502));
    #elif defined(UI_DBG_USE_Z80)
        CHIPS_ASSERT(desc->z80);
    #else
        CHIPS_ASSERT(desc->m6502);
    #endif
    memset(win, 0, sizeof(ui_dbg_t));
    win->title = desc->title;
    win->open = desc->open;
    win->valid = true;
    win->init_x = (float) desc->x;
    win->init_y = (float) desc->y;
    win->init_w = (float) ((desc->w == 0) ? 400 : desc->w);
    win->init_h = (float) ((desc->h == 0) ? 256 : desc->h);
    #if defined(UI_DBG_USE_Z80)
        win->z80 = desc->z80;
    #endif
    #if defined(UI_DBG_USE_M6502)
        win->m6502 = desc->m6502;
    #endif
    win->read_cb = desc->read_cb;
    win->break_cb = desc->break_cb;
    win->user_data = desc->user_data;
    _ui_dbg_dasm_init(&win->dasm, desc);
    _ui_dbg_history_init(&win->history, desc);
}

void ui_dbg_discard(ui_dbg_t* win) {
    CHIPS_ASSERT(win && win->valid);
    win->valid = false;
}

void ui_dbg_draw(ui_dbg_t* win) {
    CHIPS_ASSERT(win && win->valid && win->title);
    if (!win->open) {
        return;
    }
    ImGui::SetNextWindowPos(ImVec2(win->init_x, win->init_y), ImGuiSetCond_Once);
    ImGui::SetNextWindowSize(ImVec2(win->init_w, win->init_h), ImGuiSetCond_Once);
    if (ImGui::Begin(win->title, &win->open)) {
        ImGui::Text("FIXME");
    }
    ImGui::End();
}
#endif /* CHIPS_IMPL */
