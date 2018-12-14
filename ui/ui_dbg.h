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
        - m6502dasm.h   (only if UI_DBG_USE_M6502 is defined)

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

#define UI_DBG_MAX_HISTORYITEMS (16)                     /* must be 2^N */
#define UI_DBG_HISTORY_MASK (UI_DBG_MAX_HISTORYITEMS-1) /* history ringbuffer index mask */
#define UI_DBG_MAX_BREAKPOINTS (32)
#define UI_DBG_MAX_LAYERS (16)
#define UI_DBG_STEP_TRAPID (128)                    /* special trap id when step-mode active */
#define UI_DBG_BASE_TRAPID (UI_DBG_STEP_TRAPID+1)   /* first CPU trap-id used for breakpoints */
#define UI_DBG_MAX_STRLEN (32)
#define UI_DBG_MAX_BINLEN (16)
#define UI_DBG_NUM_LINES (128)

/* standard breakpoint types */
enum {
    UI_DBG_BREAKTYPE_STEP = 0,  /* single-stepping active */
    UI_DBG_BREAKTYPE_EXEC,  /* break on executed address */
    UI_DBG_BREAKTYPE_BYTE,  /* break on a specific 8-bit value at address */
    UI_DBG_BREAKTYPE_WORD,  /* break on a specific 16-bit value at address */
    UI_DBG_BREAKTYPE_IRQ,   /* break on maskable interrupt */
    UI_DBG_BREAKTYPE_NMI,   /* break on non-maskable interrupt */

    UI_DBG_BREAKTYPE_USER,  /* user-defined types start here */
};

/* current step mode */
enum {
    UI_DBG_STEPMODE_NONE = 0,
    UI_DBG_STEPMODE_INTO,
    UI_DBG_STEPMODE_OVER,
    UI_DBG_STEPMODE_OUT
};

/* a breakpoint description */
typedef struct ui_dbg_breakpoint_t {
    int type;           /* UI_DBG_BREAKTYPE_* */
    bool enabled;
    uint16_t addr;
    int val;
} ui_dbg_breakpoint_t;

/* callback for reading a byte from memory */
typedef uint8_t (*ui_dbg_read_t)(int layer, uint16_t addr, void* user_data);
/* callback for evaluating breakpoints, return breakpoint index, or -1 */
typedef int (*ui_dbg_break_t)(ui_dbg_breakpoint_t* first, int num, uint16_t pc, uint64_t pins, int ticks, void* user_data);
/* a callback to create a dynamic-update RGBA8 UI texture, needs to return an ImTextureID handle */
typedef void* (*ui_dbg_create_texture_t)(int w, int h);
/* callback to update a UI texture with new data */
typedef void (*ui_dbg_update_texture_t)(void* tex_handle, void* data, int data_byte_size);
/* callback to destroy a UI texture */
typedef void (*ui_dbg_destroy_texture_t)(void* tex_handle);

typedef struct ui_dbg_desc_t {
    const char* title;          /* window title */
    #ifdef UI_DBG_USE_Z80
    z80_t* z80;                 /* Z80 CPU to track */
    #endif
    #ifdef UI_DBG_USE_M6502
    m6502_t* m6502;             /* 6502 CPU to track */
    #endif
    ui_dbg_read_t read_cb;      /* callback to read memory */
    ui_dbg_break_t break_cb;    /* callback for evaluating breakpoints */
    ui_dbg_create_texture_t create_texture_cb;      /* callback to create UI texture */
    ui_dbg_update_texture_t update_texture_cb;      /* callback to update UI texture */
    ui_dbg_destroy_texture_t destroy_texture_cb;    /* callback to destroy UI texture */
    void* user_data;            /* user data for callbacks */
    const char* layers[UI_DBG_MAX_LAYERS];   /* memory system layer names */
    int x, y;                   /* initial window pos */
    int w, h;                   /* initial window size, or 0 for default size */
    bool open;                  /* initial open state */
} ui_dbg_desc_t;

/* an execution history item */
typedef struct ui_dbg_historyitem_t {
    uint64_t pins;
    uint32_t frame_id;
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
    uint16_t cur_addr;
    int str_pos;
    char str_buf[UI_DBG_MAX_STRLEN];
    int bin_pos;
    uint8_t bin_buf[UI_DBG_MAX_BINLEN];
} ui_dbg_dasm_t;

/* debugger state */
typedef struct ui_dbg_state_t {
    #ifdef UI_DBG_USE_Z80
    z80_t* z80;
    z80_trap_t z80_trap_cb;
    void* z80_trap_ud;
    #endif
    #ifdef UI_DBG_USE_M6502
    m6502_t* m6502;
    m6502_trap_t m6502_trap_cb;
    void* m6502_trap_ud;
    #endif
    uint32_t frame_id;
    bool stopped;
    int step_mode;
    uint16_t trap_pc;
    uint16_t stepover_pc;
    int num_breakpoints;
    ui_dbg_breakpoint_t breakpoints[UI_DBG_MAX_BREAKPOINTS];
} ui_dbg_state_t;

/* UI state variables */
typedef struct ui_dbg_uistate_t {
    const char* title;
    bool open;
    float init_x, init_y;
    float init_w, init_h;
    bool show_heatmap;
    bool show_regs;
    bool show_buttons;
    bool show_breakpoints;
    bool show_history;
    bool show_bytes;
    bool show_ticks;
} ui_dbg_uistate_t;

typedef struct ui_dbg_heatmapitem_t {
    uint32_t exec_count;
    uint32_t write_count;
    uint32_t read_count;
} ui_dbg_heapmap_item_t;

typedef struct ui_dbg_heatmap_t {
    void* texture;
    bool show_ops, show_reads, show_writes;
    int scale;
    int cur_y;
    ui_dbg_heatmapitem_t items[1<<16];     /* execution counter map */
    uint32_t pixels[1<<16];    /* execution counters converted to pixel data */
} ui_dbg_heatmap_t;

typedef struct ui_dbg_t {
    bool valid;
    ui_dbg_read_t read_cb;
    ui_dbg_break_t break_cb;
    ui_dbg_create_texture_t create_texture_cb;
    ui_dbg_update_texture_t update_texture_cb;
    ui_dbg_destroy_texture_t destroy_texture_cb;
    void* user_data;
    ui_dbg_state_t dbg;
    ui_dbg_uistate_t ui;
    ui_dbg_dasm_t dasm;
    ui_dbg_history_t history;
    ui_dbg_heatmap_t heatmap;
} ui_dbg_t;

void ui_dbg_init(ui_dbg_t* win, ui_dbg_desc_t* desc);
void ui_dbg_discard(ui_dbg_t* win);
void ui_dbg_draw(ui_dbg_t* win);

/* call before executing system ticks, don't tick if function returns false */
bool ui_dbg_before_exec(ui_dbg_t* win);
/* call after executing system ticks */
void ui_dbg_after_exec(ui_dbg_t* win);

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

/*== DISASSEMBLER ============================================================*/
static void _ui_dbg_dasm_init(ui_dbg_t* win, ui_dbg_desc_t* desc) {
    /* assuming dasm is already zero-initialized */
    for (int i = 0; i < UI_DBG_MAX_LAYERS; i++) {
        if (desc->layers[i]) {
            win->dasm.num_layers++;
            win->dasm.layers[i] = desc->layers[i];
        }
        else {
            break;
        }
    }
}

/* disassembler callback to fetch the next instruction byte */
static uint8_t _ui_dbg_dasm_in_cb(void* user_data) {
    ui_dbg_t* win = (ui_dbg_t*) user_data;
    uint8_t val = win->read_cb(win->dasm.cur_layer, win->dasm.cur_addr++, win->user_data);
    if (win->dasm.bin_pos < UI_DBG_MAX_BINLEN) {
        win->dasm.bin_buf[win->dasm.bin_pos++] = val;
    }
    return val;
}

/* disassembler callback to output a character */
static void _ui_dbg_dasm_out_cb(char c, void* user_data) {
    ui_dbg_t* win = (ui_dbg_t*) user_data;
    if ((win->dasm.str_pos+1) < UI_DBG_MAX_STRLEN) {
        win->dasm.str_buf[win->dasm.str_pos++] = c;
        win->dasm.str_buf[win->dasm.str_pos] = 0;
    }
}

/* disassemble the next instruction */
static uint16_t _ui_dbg_disasm(ui_dbg_t* win, uint16_t pc) {
    win->dasm.cur_addr = pc;
    win->dasm.str_pos = 0;
    win->dasm.bin_pos = 0;
    #if defined(UI_DBG_USE_Z80)
    z80dasm_op(pc, _ui_dbg_dasm_in_cb, _ui_dbg_dasm_out_cb, win);
    #else
    m6502dasm_op(pc, _ui_dbg_dasm_in_cb, _ui_dbg_dasm_out_cb, win);
    #endif
    return win->dasm.cur_addr;
}

/*== DEBUGGER STATE ==========================================================*/
static void _ui_dbg_dbgstate_init(ui_dbg_t* win, ui_dbg_desc_t* desc) {
    ui_dbg_state_t* dbg = &win->dbg;
    #if defined(UI_DBG_USE_Z80) &&  defined(UI_DBG_USE_M6502)
        CHIPS_ASSERT((desc->z80 || desc->m6502) && !(desc->z80 && desc->m6502));
        dbg->z80 = desc->z80;
        dbg->m6502 = desc->m6502;
    #elif defined(UI_DBG_USE_Z80)
        CHIPS_ASSERT(desc->z80);
        dbg->z80 = desc->z80;
    #else
        CHIPS_ASSERT(desc->m6502);
        dbg->m6502 = desc->m6502;
    #endif
}

static uint16_t _ui_dbg_get_pc(ui_dbg_t* win) {
    #if defined(UI_DBG_USE_Z80)
    if (win->dbg.z80) {
        return z80_pc(win->dbg.z80);
    }
    #endif
    #if defined(UI_DBG_USE_M6502)
    if (win->dbg.m6502) {
        return m6502_pc(win->dbg.m6502);
    }
    #endif
    return 0;
}

static void _ui_dbg_break(ui_dbg_t* win) {
    win->dbg.stopped = true;
    win->dbg.step_mode = UI_DBG_STEPMODE_NONE;
}

static void _ui_dbg_continue(ui_dbg_t* win) {
    win->dbg.stopped = false;
    win->dbg.step_mode = UI_DBG_STEPMODE_NONE;
}

static void _ui_dbg_step_into(ui_dbg_t* win) {
    win->dbg.stopped = false;
    win->dbg.step_mode = UI_DBG_STEPMODE_INTO;
    win->dbg.trap_pc = _ui_dbg_get_pc(win);
}

static void _ui_dbg_step_over(ui_dbg_t* win) {
    // FIXME: only set to step-over mode when the instruction at PC
    // is a CALL or conditional jump, otherwise set to step-into!
    win->dbg.stopped = false;
    win->dbg.step_mode = UI_DBG_STEPMODE_OVER;
    win->dbg.trap_pc = _ui_dbg_get_pc(win);
    win->dbg.stepover_pc = _ui_dbg_disasm(win, win->dbg.trap_pc);
}

static void _ui_dbg_step_out(ui_dbg_t* win) {
    win->dbg.stopped = false;
    win->dbg.step_mode = UI_DBG_STEPMODE_OUT;
    win->dbg.trap_pc = _ui_dbg_get_pc(win);
}

/*== HISTORY =================================================================*/
static void _ui_dbg_history_init(ui_dbg_t* win, ui_dbg_desc_t* desc) {
    /* nothing to do here, since hist is already zero-initialized */
}

static void _ui_dbg_history_clear(ui_dbg_t* win) {
    ui_dbg_history_t* h = &win->history;
    h->head = 0;
    h->tail = 0;
}

static void _ui_dbg_history_add(ui_dbg_t* win, uint16_t pc, int ticks, uint64_t pins) {
    ui_dbg_history_t* h = &win->history;
    h->items[h->head].pins = pins;
    h->items[h->head].frame_id = win->dbg.frame_id;
    h->items[h->head].ticks = ticks;
    h->items[h->head].pc = pc;
    h->head = (h->head + 1) & UI_DBG_HISTORY_MASK;
    if (h->head == h->tail) {
        h->tail = (h->tail + 1) & UI_DBG_HISTORY_MASK;
    }
}

/* return number of items in history */
static int _ui_dbg_history_num(ui_dbg_t* win) {
    const ui_dbg_history_t* h = &win->history;
    if (h->head >= h->tail) {
        return h->head - h->tail;
    }
    else {
        return (h->head + UI_DBG_MAX_HISTORYITEMS) - h->tail;
    }
}

/* get history item at index */
static ui_dbg_historyitem_t* _ui_dbg_history_item(ui_dbg_t* win, int index) {
    CHIPS_ASSERT((index >= 0) && (index < _ui_dbg_history_num(win)));
    ui_dbg_history_t* h = &win->history;
    const int rb_index = (h->tail + index) & UI_DBG_HISTORY_MASK;
    return &(h->items[rb_index]);
}

/* get instruction ticks from item in history */
static int _ui_dbg_history_ticks(ui_dbg_t* win, int index) {
    if ((index >= 0) && ((index + 1) < _ui_dbg_history_num(win))) {
        const ui_dbg_historyitem_t* item0 = _ui_dbg_history_item(win, index);
        const ui_dbg_historyitem_t* item1 = _ui_dbg_history_item(win, index + 1);
        if (item0->frame_id == item1->frame_id) {
            return item1->ticks - item0->ticks;
        }
        else {
            return item1->ticks;
        }
    }
    else {
        return 0;
    }
}

/*== BREAKPOINTS =============================================================*/

/* breakpoint evaluation callback, this is installed as CPU trap callback when needed */
static int _ui_dbg_bp_eval(uint16_t pc, int ticks, uint64_t pins, void* user_data) {
    ui_dbg_t* win = (ui_dbg_t*) user_data;
    int trap_id = 0;
    if (win->dbg.step_mode != UI_DBG_STEPMODE_NONE) {
        switch (win->dbg.step_mode) {
            case UI_DBG_STEPMODE_INTO:
                if (pc != win->dbg.trap_pc) {
                    trap_id = UI_DBG_STEP_TRAPID;
                }
                break;
            case UI_DBG_STEPMODE_OVER:
                if (pc == win->dbg.stepover_pc) {
                    trap_id = UI_DBG_STEP_TRAPID;
                }
                break;
            case UI_DBG_STEPMODE_OUT:
                trap_id = UI_DBG_STEP_TRAPID;
                break;
        }
    }
    else {
        /* check breakpoints */
        for (int i = 0; (i < win->dbg.num_breakpoints) && (trap_id == 0); i++) {
            const ui_dbg_breakpoint_t* bp = &win->dbg.breakpoints[i];
            if (bp->enabled) {
                switch (bp->type) {
                    case UI_DBG_BREAKTYPE_EXEC:
                        if (pc == bp->addr) {
                            trap_id = UI_DBG_BASE_TRAPID + i;
                        }
                        break;
                }
            }
        }
    }
    /* update execution history and heatmap */
    if (pc != win->dbg.trap_pc) {
        _ui_dbg_history_add(win, pc, ticks, pins);
        win->heatmap.items[pc].exec_count++;
    }
    #if defined(UI_DBG_USE_Z80)
    if ((pins & Z80_CTRL_MASK) == (Z80_MREQ|Z80_RD)) {
        win->heatmap.items[Z80_GET_ADDR(pins)].read_count++;
    }
    else if ((pins & Z80_CTRL_MASK) == (Z80_MREQ|Z80_WR)) {
        win->heatmap.items[Z80_GET_ADDR(pins)].write_count++;
    }
    #endif
    #if defined(UI_DBG_USE_M6502)

    #endif
    win->dbg.trap_pc = pc;
    return trap_id;
}

static bool _ui_dbg_bp_add(ui_dbg_t* win, int type, uint16_t addr, int val) {
    if (win->dbg.num_breakpoints < UI_DBG_MAX_BREAKPOINTS) {
        ui_dbg_breakpoint_t* bp = &win->dbg.breakpoints[win->dbg.num_breakpoints++];
        bp->type = type;
        bp->addr = addr;
        bp->val = val;
        bp->enabled = true;
        return true;
    }
    else {
        /* no more breakpoint slots */
        return false;
    }
}

/* find breakpoint index by type and address, return -1 if not found */
static int _ui_dbg_bp_find(ui_dbg_t* win, int type, uint16_t addr) {
    for (int i = 0; i < win->dbg.num_breakpoints; i++) {
        const ui_dbg_breakpoint_t* bp = &win->dbg.breakpoints[i];
        if (bp->type == type && bp->addr == addr) {
            return i;
        }
    }
    return -1;
}

/* remove breakpoint by index */
static void _ui_dbg_bp_rem(ui_dbg_t* win, int index) {
    if ((win->dbg.num_breakpoints > 0) && (index >= 0) && (index < win->dbg.num_breakpoints)) {
        for (int i = index; i < (win->dbg.num_breakpoints - 1); i++) {
            win->dbg.breakpoints[i] = win->dbg.breakpoints[i+1];
        }
        win->dbg.num_breakpoints--;
    }
}

/* add a new breakpoint, or remove existing one */
static void _ui_dbg_bp_toggle(ui_dbg_t* win, int type, uint16_t addr) {
    int index = _ui_dbg_bp_find(win, type, addr);
    if (index >= 0) {
        /* breakpoint already exists, remove */
        _ui_dbg_bp_rem(win, index);
    }
    else {
        /* breakpoint doesn't exist, add a new one */
        _ui_dbg_bp_add(win, type, addr, 0);
    }
}

/* return true if breakpoint is enabled, false is disabled, or index out of bounds */
static bool _ui_dbg_bp_enabled(ui_dbg_t* win, int index) {
    if ((index >= 0) && (index < win->dbg.num_breakpoints)) {
        const ui_dbg_breakpoint_t* bp = &win->dbg.breakpoints[index];
        return bp->enabled;
    }
    return false;
}

/*== HEATMAP =================================================================*/
static void _ui_dbg_heatmap_init(ui_dbg_t* win, ui_dbg_desc_t* desc) {
    win->heatmap.texture = win->create_texture_cb(256, 256);
    win->heatmap.show_ops = win->heatmap.show_reads = win->heatmap.show_writes = true;
    win->heatmap.scale = 1;
}

static void _ui_dbg_heatmap_discard(ui_dbg_t* win) {
    win->destroy_texture_cb(win->heatmap.texture);
}

static void _ui_dbg_heatmap_clear(ui_dbg_t* win) {
    memset(win->heatmap.items, 0, sizeof(win->heatmap.items));
}

static void _ui_dbg_heatmap_update(ui_dbg_t* win) {
    int y0 = win->heatmap.cur_y;
    int y1 = win->heatmap.cur_y + 32;
    win->heatmap.cur_y = (y0 + 32) & 255;
    for (int y = y0; y < y1; y++) {
        for (int x = 0; x < 256; x++) {
            int i = y * 256 + x;
            uint32_t p = 0;
            if (win->heatmap.show_ops && (win->heatmap.items[i].exec_count > 0)) {
                uint32_t r = 0x60 + (win->heatmap.items[i].exec_count>>8);
                if (r > 0xFF) { r = 0xFF; }
                p |= 0xFF000000 | r;
            }
            if (win->heatmap.show_writes && (win->heatmap.items[i].write_count > 0)) {
                uint32_t g = 0x60 + (win->heatmap.items[i].write_count>>8);
                if (g > 0xFF) { g = 0xFF; }
                p |= 0xFF000000 | (g<<8);
            }
            if (win->heatmap.show_reads && (win->heatmap.items[i].read_count > 0)) {
                uint32_t b = 0x60 + (win->heatmap.items[i].read_count>>8);
                if (b > 0xFF) { b = 0xFF; }
                p |= 0xFF000000 | (b<<16);
            }
            win->heatmap.pixels[i] = p;
        }
    }
    win->update_texture_cb(win->heatmap.texture, win->heatmap.pixels, 256*256*4);
}

static void _ui_dbg_heatmap_draw(ui_dbg_t* win) {
    if (!win->ui.show_heatmap) {
        return;
    }
    _ui_dbg_heatmap_update(win);
    ImGui::SetNextWindowPos(ImVec2(30, 30), ImGuiSetCond_Once);
    ImGui::SetNextWindowSize(ImVec2(288, 356), ImGuiSetCond_Once);
    if (ImGui::Begin("Memory Heatmap", &win->ui.show_heatmap)) {
        if (ImGui::Button("Clear")) {
            _ui_dbg_heatmap_clear(win);
        }
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, 0xFF0000FF);
        ImGui::Checkbox("Ops", &win->heatmap.show_ops); ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, 0xFFFF0000);
        ImGui::Checkbox("Reads", &win->heatmap.show_reads); ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, 0xFF00FF00);
        ImGui::Checkbox("Writes", &win->heatmap.show_writes);
        ImGui::PopStyleColor(3);
        ImGui::SliderInt("Scale", &win->heatmap.scale, 1, 8);
        ImGui::BeginChild("##tex", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);
        ImVec2 screen_pos = ImGui::GetCursorScreenPos();
        ImVec2 mouse_pos = ImGui::GetMousePos();
        ImGui::Image(win->heatmap.texture, ImVec2(256*win->heatmap.scale, 256*win->heatmap.scale), ImVec2(0, 0), ImVec2(1, 1));
        if (ImGui::IsItemHovered()) {
            int x = (int)((mouse_pos.x - screen_pos.x) / win->heatmap.scale);
            int y = (int)((mouse_pos.y - screen_pos.y) / win->heatmap.scale);
            uint16_t addr = y * 256 + x;
            if (win->heatmap.items[addr].exec_count > 0) {
                /* this is an instruction */
                _ui_dbg_disasm(win, addr);
                ImGui::SetTooltip("%04X: %s", addr, win->dasm.str_buf);
            }
            else {
                ImGui::SetTooltip("%04X: %02X %02X %02X %02X", addr,
                    win->read_cb(win->dasm.cur_layer, addr, win->user_data),
                    win->read_cb(win->dasm.cur_layer, addr + 1, win->user_data),
                    win->read_cb(win->dasm.cur_layer, addr + 2, win->user_data),
                    win->read_cb(win->dasm.cur_layer, addr + 3, win->user_data));
            }
        }
        ImGui::EndChild();
    }
    ImGui::End();
}

/*== UI HELPERS ==============================================================*/
static void _ui_dbg_uistate_init(ui_dbg_t* win, ui_dbg_desc_t* desc) {
    ui_dbg_uistate_t* ui = &win->ui;
    ui->title = desc->title;
    ui->open = desc->open;
    ui->init_x = (float) desc->x;
    ui->init_y = (float) desc->y;
    ui->init_w = (float) ((desc->w == 0) ? 460 : desc->w);
    ui->init_h = (float) ((desc->h == 0) ? 460 : desc->h);
    ui->show_regs = true;
    ui->show_buttons = true;
    ui->show_bytes = true;
    ui->show_ticks = true;
    ui->show_history = false;
    ui->show_breakpoints = false;
}

static void _ui_dbg_draw_menu(ui_dbg_t* win) {
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Debug")) {
            ImGui::MenuItem("Continue", "F5");
            ImGui::MenuItem("Step Over", "F6");
            ImGui::MenuItem("Step Into", "F7");
            ImGui::MenuItem("Step Out", "F8");
            if (ImGui::BeginMenu("Stop")) {
                ImGui::MenuItem("Stop Now");
                ImGui::MenuItem("Stop at IRQ");
                ImGui::MenuItem("Stop at NMI");
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Run")) {
                ImGui::MenuItem("Run to IRQ");
                ImGui::MenuItem("Run to NMI");
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Breakpoints")) {
            ImGui::MenuItem("Toggle Breakpoint", "F9");
            ImGui::MenuItem("Add Breakpoint...");
            ImGui::MenuItem("Enable All Breakpoints");
            ImGui::MenuItem("Disable All Breakpoints");
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Show")) {
            ImGui::MenuItem("Memory Heatmap", 0, &win->ui.show_heatmap);
            ImGui::MenuItem("Registers", 0, &win->ui.show_regs);
            ImGui::MenuItem("Button Bar", 0, &win->ui.show_buttons);
            ImGui::MenuItem("Breakpoints", 0, &win->ui.show_breakpoints);
            ImGui::MenuItem("History", 0, &win->ui.show_history);
            ImGui::MenuItem("Opcode Bytes", 0, &win->ui.show_bytes);
            ImGui::MenuItem("Opcode Ticks", 0, &win->ui.show_ticks);
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
}

void _ui_dbg_draw_regs(ui_dbg_t* win) {
    if (!win->ui.show_regs) {
        return;
    }
    #if defined(UI_DBG_USE_Z80)
    if (win->dbg.z80) {
        const float h = 3*ImGui::GetFrameHeightWithSpacing();
        ImGui::BeginChild("##regs", ImVec2(0, h), false);
        z80_t* c = win->dbg.z80;
        ImGui::Columns(6, "##reg_columns", false);
        for (int i = 0; i < 6; i++) {
            ImGui::SetColumnWidth(i, 72);
        }
        z80_set_af(c, ui_util_input_u16("AF", z80_af(c))); ImGui::NextColumn();
        z80_set_bc(c, ui_util_input_u16("BC", z80_bc(c))); ImGui::NextColumn();
        z80_set_de(c, ui_util_input_u16("DE", z80_de(c))); ImGui::NextColumn();
        z80_set_hl(c, ui_util_input_u16("HL", z80_hl(c))); ImGui::NextColumn();
        z80_set_hl(c, ui_util_input_u16("WZ", z80_hl(c))); ImGui::NextColumn();
        z80_set_im(c, ui_util_input_u8("IM", z80_im(c))); ImGui::NextColumn();
        z80_set_af_(c, ui_util_input_u16("AF'", z80_af_(c))); ImGui::NextColumn();
        z80_set_bc_(c, ui_util_input_u16("BC'", z80_bc_(c))); ImGui::NextColumn();
        z80_set_de_(c, ui_util_input_u16("DE'", z80_de_(c))); ImGui::NextColumn();
        z80_set_hl_(c, ui_util_input_u16("HL'", z80_hl_(c))); ImGui::NextColumn();
        z80_set_i(c, ui_util_input_u8("I", z80_i(c))); ImGui::NextColumn();
        ImGui::AlignTextToFramePadding();
        if (z80_iff1(c)) { ImGui::Text("IFF1"); }
        else             { ImGui::TextDisabled("IFF1"); }
        ImGui::SameLine();
        if (z80_iff2(c)) { ImGui::Text("IFF2"); }
        else             { ImGui::TextDisabled("IFF2"); }
        ImGui::NextColumn();
        z80_set_ix(c, ui_util_input_u16("IX", z80_ix(c))); ImGui::NextColumn();
        z80_set_iy(c, ui_util_input_u16("IY", z80_iy(c))); ImGui::NextColumn();
        z80_set_sp(c, ui_util_input_u16("SP", z80_sp(c))); ImGui::NextColumn();
        z80_set_pc(c, ui_util_input_u16("PC", z80_pc(c))); ImGui::NextColumn();
        z80_set_r(c, ui_util_input_u8("R", z80_r(c))); ImGui::NextColumn();
        const uint8_t f = z80_f(c);
        char f_str[9] = {
            (f & Z80_SF) ? 'S':'-',
            (f & Z80_ZF) ? 'Z':'-',
            (f & Z80_YF) ? 'X':'-',
            (f & Z80_HF) ? 'H':'-',
            (f & Z80_XF) ? 'Y':'-',
            (f & Z80_VF) ? 'V':'-',
            (f & Z80_NF) ? 'N':'-',
            (f & Z80_CF) ? 'C':'-',
            0,
        };
        ImGui::AlignTextToFramePadding();
        ImGui::Text("%s", f_str);
        ImGui::EndChild();
    }
    #else
    if (win->m6502) {

    }
    #endif
    ImGui::Separator();
}

static void _ui_dbg_draw_buttons(ui_dbg_t* win) {
    if (!win->ui.show_buttons) {
        return;
    }
    if (win->dbg.stopped) {
        if (ImGui::Button("Continue")) {
            _ui_dbg_continue(win);
        }
        ImGui::SameLine();
        if (ImGui::Button("Step")) {
            _ui_dbg_step_into(win);
        }
        ImGui::SameLine();
        if (ImGui::Button("Over")) {
            _ui_dbg_step_over(win);
        }
        ImGui::SameLine();
        if (ImGui::Button("Out")) {
            _ui_dbg_step_out(win);
        }
        ImGui::SameLine();
        if (ImGui::Button(">IRQ")) {
            _ui_dbg_continue(win);
        }
        ImGui::SameLine();
        if (ImGui::Button(">NMI")) {
            _ui_dbg_continue(win);
        }
    }
    else {
        if (ImGui::Button("Break")) {
            _ui_dbg_break(win);
        }
    }
    ImGui::Separator();
}

static void _ui_dbg_draw_main(ui_dbg_t* win) {
    ImGui::BeginChild("##main", ImVec2(0, 0), false);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0,0));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0,0));
    const float line_height = ImGui::GetTextLineHeight();
    const float glyph_width = ImGui::CalcTextSize("F").x;
    const float cell_width = 3 * glyph_width;
    const int hist_num_lines = _ui_dbg_history_num(win);
    const int all_num_lines = hist_num_lines + UI_DBG_NUM_LINES;
    ImGuiListClipper clipper(all_num_lines, line_height);

    const ImU32 bp_enabled_color = 0xFF0000FF;
    const ImU32 bp_disabled_color = 0xFF000088;
    const ImU32 pc_color = 0xFF00FFFF;
    const ImU32 brd_color = 0xFF000000;

    /* visible items */
    uint16_t addr = _ui_dbg_get_pc(win);
    for (int line_i = 0; line_i < clipper.DisplayEnd; line_i++) {
        if (line_i < hist_num_lines) {
            /* take address from history */
            addr = _ui_dbg_history_item(win, line_i)->pc;
        }
        const uint16_t op_addr = addr;
        addr = _ui_dbg_disasm(win, addr);

        /* check if above visible scroll area */
        if (line_i < clipper.DisplayStart) {
            continue;
        }

        bool in_history = (line_i + 1) < hist_num_lines;
        if (in_history) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        }
        else {
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_Text]);
        }

        /* inside visible scroll area */
        const int num_bytes = addr - op_addr;

        /* column for breakpoint and step-cursor */
        ImVec2 pos = ImGui::GetCursorScreenPos();
        pos.y += 1;
        const float lh2 = (float)(int)(line_height/2);
        ImGui::PushID(line_i);
        if (ImGui::InvisibleButton("##bp", ImVec2(16, line_height))) {
            /* add or remove execution breakpoint */
            _ui_dbg_bp_toggle(win, UI_DBG_BREAKTYPE_EXEC, op_addr);
        }
        ImGui::PopID();
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const ImVec2 mid(pos.x + 6, pos.y + lh2);
        const int bp_index = _ui_dbg_bp_find(win, UI_DBG_BREAKTYPE_EXEC, op_addr);
        if (bp_index >= 0) {
            /* an execution breakpoint exists for this address */
            ImU32 bp_color = _ui_dbg_bp_enabled(win, bp_index) ? bp_enabled_color : bp_disabled_color;
            dl->AddCircleFilled(mid, 6, bp_color);
            dl->AddCircle(mid, 6, brd_color);
        }
        else if (ImGui::IsItemHovered()) {
            dl->AddCircle(mid, 6, bp_enabled_color);
        }
        /* current PC/step cursor */
        if ((op_addr == _ui_dbg_get_pc(win)) && !in_history) {
            const ImVec2 a(pos.x + 2, pos.y);
            const ImVec2 b(pos.x + 12, pos.y + lh2);
            const ImVec2 c(pos.x + 2, pos.y + line_height);
            dl->AddTriangleFilled(a, b, c, pc_color);
            dl->AddTriangle(a, b, c, brd_color);
        }
        ImGui::SameLine();

        /* address */
        ImGui::Text("%04X:   ", op_addr);
        ImGui::SameLine();

        /* instruction bytes (optional) */
        float x = ImGui::GetCursorPosX();
        if (win->ui.show_bytes) {
            for (int n = 0; n < num_bytes; n++) {
                ImGui::SameLine(x + cell_width*n);
                ImGui::Text("%02X ", win->dasm.bin_buf[n]);
            }
            x += cell_width * 4;
        }

        /* disassembled instruction */
        x += glyph_width * 4;
        ImGui::SameLine(x);
        ImGui::Text("%s", win->dasm.str_buf);

        /* tick count */
        if (in_history && win->ui.show_ticks) {
            x += glyph_width * 20; 
            ImGui::SameLine(x);
            int ticks = _ui_dbg_history_ticks(win, line_i);
            ImGui::Text("%d", ticks);
        }

        ImGui::PopStyleColor();
    }
    clipper.End();
    ImGui::PopStyleVar(2);

    ImGui::EndChild();
}

static void _ui_dbg_dbgwin_draw(ui_dbg_t* win) {
    if (!win->ui.open) {
        return;
    }
    ImGui::SetNextWindowPos(ImVec2(win->ui.init_x, win->ui.init_y), ImGuiSetCond_Once);
    ImGui::SetNextWindowSize(ImVec2(win->ui.init_w, win->ui.init_h), ImGuiSetCond_Once);
    if (ImGui::Begin(win->ui.title, &win->ui.open, ImGuiWindowFlags_MenuBar)) {
        _ui_dbg_draw_menu(win);
        _ui_dbg_draw_regs(win);
        _ui_dbg_draw_buttons(win);
        _ui_dbg_draw_main(win);
    }
    ImGui::End();
}

/*== PUBLIC FUNCTIONS ========================================================*/
void ui_dbg_init(ui_dbg_t* win, ui_dbg_desc_t* desc) {
    CHIPS_ASSERT(win && desc);
    CHIPS_ASSERT(desc->title);
    CHIPS_ASSERT(desc->read_cb && desc->break_cb);
    CHIPS_ASSERT(desc->create_texture_cb && desc->update_texture_cb && desc->destroy_texture_cb);
    memset(win, 0, sizeof(ui_dbg_t));
    win->valid = true;
    win->read_cb = desc->read_cb;
    win->break_cb = desc->break_cb;
    win->create_texture_cb = desc->create_texture_cb;
    win->update_texture_cb = desc->update_texture_cb;
    win->destroy_texture_cb = desc->destroy_texture_cb;
    win->user_data = desc->user_data;
    _ui_dbg_dbgstate_init(win, desc);
    _ui_dbg_uistate_init(win, desc);
    _ui_dbg_dasm_init(win, desc);
    _ui_dbg_history_init(win, desc);
    _ui_dbg_heatmap_init(win, desc);
}

void ui_dbg_discard(ui_dbg_t* win) {
    CHIPS_ASSERT(win && win->valid);
    _ui_dbg_heatmap_discard(win);
    win->valid = false;
}

bool ui_dbg_before_exec(ui_dbg_t* win) {
    CHIPS_ASSERT(win && win->valid);
    /* only install trap callback if our window is actually open */
    if (!(win->ui.open || win->ui.show_heatmap)) {
        _ui_dbg_history_clear(win);
        return true;
    }
    if (!win->dbg.stopped) {
        win->dbg.frame_id++;
        #if defined(UI_DBG_USE_Z80)
            if (win->dbg.z80) {
                win->dbg.z80_trap_cb = win->dbg.z80->trap_cb;
                win->dbg.z80_trap_ud = win->dbg.z80->trap_user_data;
                z80_trap_cb(win->dbg.z80, _ui_dbg_bp_eval, win);
            }
        #endif
        #if defined(UI_DBG_USE_M6502)
            if (win->dbg.m6502) {
                win->dbg.m6502_trap_cb = win->dbg.m6502->trap_cb;
                win->dbg.m6502_trap_ud = win->dbg.m6502->trap_user_data;
                m6502_trap_cb(win->dbg.m6502, _ui_dbg_eval, win);
            }
        #endif
    }
    return !win->dbg.stopped;
}

void ui_dbg_after_exec(ui_dbg_t* win) {
    CHIPS_ASSERT(win && win->valid);
    /* uninstall our trap callback, but only if it hasn't been overwritten */
    int trap_id = 0;
    #if defined(UI_DBG_USE_Z80)
        if (win->dbg.z80 && (win->dbg.z80->trap_cb == _ui_dbg_bp_eval)) {
            z80_trap_cb(win->dbg.z80, win->dbg.z80_trap_cb, win->dbg.z80_trap_ud);
        }
        win->dbg.z80_trap_cb = 0;
        win->dbg.z80_trap_ud = 0;
        trap_id = win->dbg.z80->trap_id;
    #endif
    #if defined(UI_DBG_USE_M6502)
        if (win->dbg.m6502 && (win->dbg.m6502->trap_cb == _ui_dbg_bp_eval)) {
            m6502_trap_cb(win->dbg.m6502, win->dbg.m6502_trap_cb, win->dbg.m6502_trap_ud);
        }
        win->dbg.m6502_trap_cb = 0;
        trap_id = win->dbg.m6502->trap_id;
    #endif
    if (trap_id) {
        win->dbg.stopped = true;
        win->dbg.step_mode = UI_DBG_STEPMODE_NONE;
    }
}

void ui_dbg_draw(ui_dbg_t* win) {
    CHIPS_ASSERT(win && win->valid && win->ui.title);
    if (!(win->ui.open || win->ui.show_heatmap)) {
        return;
    }
    _ui_dbg_dbgwin_draw(win);
    _ui_dbg_heatmap_draw(win);
}
#endif /* CHIPS_IMPL */
