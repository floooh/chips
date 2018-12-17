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

#define UI_DBG_MAX_BREAKPOINTS (32)
#define UI_DBG_STEP_TRAPID (128)                    /* special trap id when step-mode active */
#define UI_DBG_BASE_TRAPID (UI_DBG_STEP_TRAPID+1)   /* first CPU trap-id used for breakpoints */
#define UI_DBG_MAX_STRLEN (32)
#define UI_DBG_MAX_BINLEN (16)
#define UI_DBG_NUM_LINES (256)
#define UI_DBG_NUM_BACKTRACE_LINES (UI_DBG_NUM_LINES/2)

/* breakpoint types */
enum {
    UI_DBG_BREAKTYPE_EXEC = 0,      /* break on executed address */
    UI_DBG_BREAKTYPE_BYTE,          /* break on a specific 8-bit value at address */
    UI_DBG_BREAKTYPE_WORD,          /* break on a specific 16-bit value at address */
    UI_DBG_BREAKTYPE_IRQ,           /* break on maskable interrupt */
    UI_DBG_BREAKTYPE_NMI,           /* break on non-maskable interrupt */
};

/* breakpoint conditions */
enum {
    UI_DBG_BREAKCOND_EQUAL = 0,
    UI_DBG_BREAKCOND_NONEQUAL,
    UI_DBG_BREAKCOND_GREATER,
    UI_DBG_BREAKCOND_LESS,
    UI_DBG_BREAKCOND_GREATER_EQUAL,
    UI_DBG_BREAKCOND_LESS_EQUAL,
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
    int cond;           /* UI_DBG_BREAKCOND_* */
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

/* user-defined hotkeys (all strings must be static) */
typedef struct ui_dbg_keydesc_t {
    int continue_keycode;
    int break_keycode;
    int step_over_keycode;
    int step_into_keycode;
    int step_out_keycode;
    int toggle_breakpoint_keycode;
    const char* continue_name;
    const char* break_name;
    const char* step_over_name;
    const char* step_into_name;
    const char* step_out_name;
    const char* toggle_breakpoint_name;
} ui_dbg_keydesc_t;

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
    int x, y;                   /* initial window pos */
    int w, h;                   /* initial window size, or 0 for default size */
    bool open;                  /* initial open state */
    ui_dbg_keydesc_t keys;      /* user-defined hotkeys */
} ui_dbg_desc_t;

/* disassembler state */
typedef struct ui_dbg_dasm_t {
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

    bool stopped;
    int step_mode;

    bool install_trap_cb;       /* whether to install the trap callback */
    uint32_t frame_id;          /* used in trap callback to detect when a new frame has started */
    uint32_t trap_frame_id;
    uint16_t trap_pc;           /* last PC in CPU trap callback */
    int trap_ticks;             /* last tick count in CPU trap callback */
    uint16_t stepover_pc;
    int delete_breakpoint_index;
    int num_breakpoints;
    ui_dbg_breakpoint_t breakpoints[UI_DBG_MAX_BREAKPOINTS];
} ui_dbg_state_t;

/* a displayed line */
typedef struct ui_dbg_line_t {
    uint16_t addr;
    uint8_t val;
} ui_dbg_line_t;

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
    bool show_bytes;
    bool show_ticks;
    bool request_scroll;
    ui_dbg_keydesc_t keys;
    ui_dbg_line_t line_array[UI_DBG_NUM_LINES];
} ui_dbg_uistate_t;

typedef struct ui_dbg_heatmapitem_t {
    uint16_t op_count;      /* set for first instruction byte */
    uint16_t op_start;      /* set for followup instruction bytes */
    uint16_t write_count;
    uint16_t read_count;
    uint16_t ticks;
} ui_dbg_heapmap_item_t;

typedef struct ui_dbg_heatmap_t {
    void* texture;
    bool show_ops, show_reads, show_writes;
    int scale;
    int cur_y;
    bool popup_addr_valid;
    uint16_t popup_addr;
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

/*== GENERAL HELPERS =========================================================*/
static inline uint8_t _ui_dbg_read_byte(ui_dbg_t* win, uint16_t addr) {
    return win->read_cb(0, addr, win->user_data);
}

static inline uint16_t _ui_dbg_read_word(ui_dbg_t* win, uint16_t addr) {
    uint8_t l = win->read_cb(0, addr, win->user_data);
    uint8_t h = win->read_cb(0, addr+1, win->user_data);
    return (uint16_t) (h<<8)|l;
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

/* disassembler callback to fetch the next instruction byte */
static uint8_t _ui_dbg_dasm_in_cb(void* user_data) {
    ui_dbg_t* win = (ui_dbg_t*) user_data;
    uint8_t val = _ui_dbg_read_byte(win, win->dasm.cur_addr++);
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

/* disassemble the an instruction, but only return the length of the instruction */
static uint16_t _ui_dbg_disasm_len(ui_dbg_t* win, uint16_t pc) {
    win->dasm.cur_addr = pc;
    win->dasm.str_pos = 0;
    win->dasm.bin_pos = 0;
    #if defined(UI_DBG_USE_Z80)
    uint16_t next_pc = z80dasm_op(pc, _ui_dbg_dasm_in_cb, 0, win);
    #else
    uint16_t next_pc = m6502dasm_op(pc, _ui_dbg_dasm_in_cb, 0, win);
    #endif
    return next_pc - pc;
}


static void _ui_dbg_break(ui_dbg_t* win) {
    win->dbg.stopped = true;
    win->dbg.step_mode = UI_DBG_STEPMODE_NONE;
    win->ui.request_scroll = true;
}

static void _ui_dbg_continue(ui_dbg_t* win) {
    win->dbg.stopped = false;
    win->dbg.step_mode = UI_DBG_STEPMODE_NONE;
}

static void _ui_dbg_step_into(ui_dbg_t* win) {
    win->dbg.stopped = false;
    win->dbg.step_mode = UI_DBG_STEPMODE_INTO;
    win->dbg.trap_pc = _ui_dbg_get_pc(win);
    win->ui.request_scroll = true;
}

static void _ui_dbg_step_over(ui_dbg_t* win) {
    // FIXME: only set to step-over mode when the instruction at PC
    // is a CALL or conditional jump, otherwise set to step-into!
    win->dbg.stopped = false;
    win->dbg.step_mode = UI_DBG_STEPMODE_OVER;
    win->dbg.trap_pc = _ui_dbg_get_pc(win);
    win->dbg.stepover_pc = _ui_dbg_disasm(win, win->dbg.trap_pc);
    win->ui.request_scroll = true;
}

static void _ui_dbg_step_out(ui_dbg_t* win) {
    win->dbg.stopped = false;
    win->dbg.step_mode = UI_DBG_STEPMODE_OUT;
    win->dbg.trap_pc = _ui_dbg_get_pc(win);
    win->ui.request_scroll = true;
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
    win->dbg.install_trap_cb = true;
    win->dbg.delete_breakpoint_index = -1;
}

/*== BREAKPOINTS =============================================================*/

/* breakpoint evaluation callback, this is installed as CPU trap callback when needed */
static int _ui_dbg_bp_eval(uint16_t pc, int ticks, uint64_t pins, void* user_data) {
    ui_dbg_t* win = (ui_dbg_t*) user_data;
    int trap_id = 0;
    if (win->dbg.step_mode != UI_DBG_STEPMODE_NONE) {
        switch (win->dbg.step_mode) {
            case UI_DBG_STEPMODE_INTO:
                /* stop when PC has changed */
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

                    case UI_DBG_BREAKTYPE_BYTE:
                        {
                            int val = (int) _ui_dbg_read_byte(win, bp->addr);
                            bool b = false;
                            switch (bp->cond) {
                                case UI_DBG_BREAKCOND_EQUAL:            b = val == bp->val; break;
                                case UI_DBG_BREAKCOND_NONEQUAL:         b = val != bp->val; break;
                                case UI_DBG_BREAKCOND_GREATER:          b = val > bp->val; break;
                                case UI_DBG_BREAKCOND_LESS:             b = val < bp->val; break;
                                case UI_DBG_BREAKCOND_GREATER_EQUAL:    b = val >= bp->val; break;
                                case UI_DBG_BREAKCOND_LESS_EQUAL:       b = val <= bp->val; break;
                            }
                            if (b) {
                                trap_id = UI_DBG_BASE_TRAPID + i;
                            }
                        }
                        break;

                    case UI_DBG_BREAKTYPE_WORD:
                        {
                            uint16_t val = (int) _ui_dbg_read_word(win, bp->addr);
                            bool b = false;
                            switch (bp->cond) {
                                case UI_DBG_BREAKCOND_EQUAL:            b = val == bp->val; break;
                                case UI_DBG_BREAKCOND_NONEQUAL:         b = val != bp->val; break;
                                case UI_DBG_BREAKCOND_GREATER:          b = val > bp->val; break;
                                case UI_DBG_BREAKCOND_LESS:             b = val < bp->val; break;
                                case UI_DBG_BREAKCOND_GREATER_EQUAL:    b = val >= bp->val; break;
                                case UI_DBG_BREAKCOND_LESS_EQUAL:       b = val <= bp->val; break;
                            }
                            if (b) {
                                trap_id = UI_DBG_BASE_TRAPID + i;
                            }
                        }
                        break;

                    case UI_DBG_BREAKTYPE_IRQ:
                        #if defined UI_DBG_USE_Z80
                        if (pins & Z80_INT) {
                            trap_id = UI_DBG_BASE_TRAPID + i;
                        }
                        #endif
                        #if defined UI_DBG_USE_M6502
                        if (pins & M6502_INT) {
                            trap_id = UI_DBG_BASE_TRAPID + i;
                        }
                        #endif
                        break;

                    case UI_DBG_BREAKTYPE_NMI:
                        #if defined UI_DBG_USE_Z80
                        if (pins & Z80_NMI) {
                            trap_id = UI_DBG_BASE_TRAPID + i;
                        }
                        #endif
                        #if defined UI_DBG_USE_M6502
                        if (pins & M6502_NMI) {
                            trap_id = UI_DBG_BASE_TRAPID + i;
                        }
                        #endif
                        break;
                }
            }
        }
    }
    /* update execution heatmap */
    if (pc != win->dbg.trap_pc) {
        if (win->heatmap.items[pc].op_count < 0xFFFF) {
            win->heatmap.items[pc].op_count++;
        }
        int op_len = _ui_dbg_disasm_len(win, pc);
        for (int i = 1; i < op_len; i++) {
            win->heatmap.items[(pc + i) & 0xFFFF].op_start = pc;
        }
        /* update last instruction's ticks */
        if (win->dbg.trap_frame_id == win->dbg.frame_id) {
            win->heatmap.items[win->dbg.trap_pc].ticks = ticks - win->dbg.trap_ticks;
        }
        else {
            win->heatmap.items[win->dbg.trap_pc].ticks = ticks;
        }
    }
    #if defined(UI_DBG_USE_Z80)
    if ((pins & Z80_CTRL_MASK) == (Z80_MREQ|Z80_RD)) {
        const uint16_t addr = Z80_GET_ADDR(pins);
        if (win->heatmap.items[addr].read_count < 0xFFFF) {
            win->heatmap.items[addr].read_count++;
        }
    }
    else if ((pins & Z80_CTRL_MASK) == (Z80_MREQ|Z80_WR)) {
        const uint16_t addr = Z80_GET_ADDR(pins);
        if (win->heatmap.items[addr].write_count < 0xFFFF) {
            win->heatmap.items[addr].write_count++;
        }
    }
    #endif
    #if defined(UI_DBG_USE_M6502)
    #error "FIXME: M6502"
    #endif
    win->dbg.trap_pc = pc;
    win->dbg.trap_frame_id = win->dbg.frame_id;
    win->dbg.trap_ticks = ticks;
    return trap_id;
}

/* add an execution breakpoint */
static bool _ui_dbg_bp_add_exec(ui_dbg_t* win, bool enabled, uint16_t addr) {
    if (win->dbg.num_breakpoints < UI_DBG_MAX_BREAKPOINTS) {
        ui_dbg_breakpoint_t* bp = &win->dbg.breakpoints[win->dbg.num_breakpoints++];
        bp->type = UI_DBG_BREAKTYPE_EXEC;
        bp->cond = UI_DBG_BREAKCOND_EQUAL;
        bp->addr = addr;
        bp->val = 0;
        bp->enabled = enabled;
        return true;
    }
    else {
        /* no more breakpoint slots */
        return false;
    }
}

/* add a memory breakpoint (bytes) */
static bool _ui_dbg_bp_add_byte(ui_dbg_t* win, bool enabled, uint16_t addr) {
    if (win->dbg.num_breakpoints < UI_DBG_MAX_BREAKPOINTS) {
        ui_dbg_breakpoint_t* bp = &win->dbg.breakpoints[win->dbg.num_breakpoints++];
        bp->type = UI_DBG_BREAKTYPE_BYTE;
        bp->cond = UI_DBG_BREAKCOND_EQUAL;
        bp->addr = addr;
        bp->val = _ui_dbg_read_byte(win, addr);
        bp->enabled = enabled;
        return true;
    }
    else {
        /* no more breakpoint slots */
        return false;
    }
}

/* add a memory breakpoint (word) */
static bool _ui_dbg_bp_add_word(ui_dbg_t* win, bool enabled, uint16_t addr) {
    if (win->dbg.num_breakpoints < UI_DBG_MAX_BREAKPOINTS) {
        ui_dbg_breakpoint_t* bp = &win->dbg.breakpoints[win->dbg.num_breakpoints++];
        bp->type = UI_DBG_BREAKTYPE_WORD;
        bp->cond = UI_DBG_BREAKCOND_EQUAL;
        bp->addr = addr;
        bp->val = _ui_dbg_read_word(win, addr);
        bp->enabled = enabled;
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

/* delete breakpoint by index */
static void _ui_dbg_bp_del(ui_dbg_t* win, int index) {
    if ((win->dbg.num_breakpoints > 0) && (index >= 0) && (index < win->dbg.num_breakpoints)) {
        for (int i = index; i < (win->dbg.num_breakpoints - 1); i++) {
            win->dbg.breakpoints[i] = win->dbg.breakpoints[i+1];
        }
        win->dbg.num_breakpoints--;
    }
}

/* add a new breakpoint, or remove existing one */
static void _ui_dbg_bp_toggle_exec(ui_dbg_t* win, uint16_t addr) {
    int index = _ui_dbg_bp_find(win, UI_DBG_BREAKTYPE_EXEC, addr);
    if (index >= 0) {
        /* breakpoint already exists, remove */
        _ui_dbg_bp_del(win, index);
    }
    else {
        /* breakpoint doesn't exist, add a new one */
        _ui_dbg_bp_add_exec(win, true, addr);
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

/* disable all breakpoints */
static void _ui_dbg_bp_disable_all(ui_dbg_t* win) {
    for (int i = 0; i < win->dbg.num_breakpoints; i++) {
        win->dbg.breakpoints[i].enabled = false;
    }
}

/* enable all breakpoints */
static void _ui_dbg_bp_enable_all(ui_dbg_t* win) {
    for (int i = 0; i < win->dbg.num_breakpoints; i++) {
        win->dbg.breakpoints[i].enabled = true;
    }
}

/* delete all breakpoints */
static void _ui_dbg_bp_delete_all(ui_dbg_t* win) {
    win->dbg.num_breakpoints = 0;
}

/* draw the "Delete all breakpoints" popup modal */
static void _ui_dbg_bp_draw_delete_all_modal(ui_dbg_t* win, const char* title) {
    if (ImGui::BeginPopupModal(title, 0, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Delete all breakpoints?");
        ImGui::Separator();
        if (ImGui::Button("Ok", ImVec2(120, 0))) {
            _ui_dbg_bp_delete_all(win);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

/* draw the breakpoint list window */
static void _ui_dbg_bp_draw(ui_dbg_t* win) {
    if (!win->ui.show_breakpoints) {
        return;
    }
    ImGui::SetNextWindowPos(ImVec2(win->ui.init_x + 30, win->ui.init_y + 30), ImGuiSetCond_Once);
    ImGui::SetNextWindowSize(ImVec2(-1, 256), ImGuiSetCond_Once);
    if (ImGui::Begin("Breakpoints", &win->ui.show_breakpoints)) {
        bool scroll_down = false;
        if (ImGui::Button("Add..")) {
            _ui_dbg_bp_add_exec(win, false, _ui_dbg_get_pc(win));
            scroll_down = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Disable All")) {
            _ui_dbg_bp_disable_all(win);
        }
        ImGui::SameLine();
        if (ImGui::Button("Enable All")) {
            _ui_dbg_bp_enable_all(win);
        }
        ImGui::SameLine();
        if (ImGui::Button("Delete All")) {
            ImGui::OpenPopup("Delete All?");
        }
        _ui_dbg_bp_draw_delete_all_modal(win, "Delete All?");
        int del_bp_index = -1;
        ImGui::Separator();
        ImGui::BeginChild("##bp_list", ImVec2(0, 0), false);
        for (int i = 0; i < win->dbg.num_breakpoints; i++) {
            ImGui::PushID(i);
            ui_dbg_breakpoint_t* bp = &win->dbg.breakpoints[i];
            ImGui::Checkbox("##enabled", &bp->enabled); ImGui::SameLine();
            if (ImGui::IsItemHovered()) {
                if (bp->enabled) {
                    ImGui::SetTooltip("Disable");
                }
                else {
                    ImGui::SetTooltip("Enable");
                }
            }
            ImGui::SameLine();
            bool upd_val = false;
            ImGui::PushItemWidth(84);
            if (ImGui::Combo("##type", &bp->type, "Break at\0Byte at\0Word at\0IRQ\0NMI\0")) {
                upd_val = true;
            }
            ImGui::PopItemWidth();
            if ((bp->type != UI_DBG_BREAKTYPE_IRQ) && (bp->type != UI_DBG_BREAKTYPE_NMI)) {
                ImGui::SameLine();
                uint16_t old_addr = bp->addr;
                bp->addr = ui_util_input_u16("##addr", old_addr);
                if (upd_val || (old_addr != bp->addr)) {
                    /* if breakpoint type or address has changed, update the breakpoint's value from memory */
                    if (bp->type == UI_DBG_BREAKTYPE_BYTE) {
                        bp->val = (int) _ui_dbg_read_byte(win, bp->addr);
                    }
                    else {
                        bp->val = (int) _ui_dbg_read_word(win, bp->addr);
                    }
                }
                if ((bp->type == UI_DBG_BREAKTYPE_BYTE) || (bp->type == UI_DBG_BREAKTYPE_WORD)) {
                    ImGui::SameLine();
                    ImGui::PushItemWidth(42);
                    ImGui::Combo("##cond", &bp->cond,"==\0!=\0>\0<\0>=\0<=\0");
                    ImGui::PopItemWidth();
                    ImGui::SameLine();
                    if (bp->type == UI_DBG_BREAKTYPE_BYTE) {
                        bp->val = (int) ui_util_input_u8("##byte", (uint8_t)bp->val);
                    }
                    else {
                        bp->val = (int) ui_util_input_u16("##word", (uint16_t)bp->val);
                    }
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Del")) {
                del_bp_index = i;
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Delete");
            }
            ImGui::PopID();
        }
        if (del_bp_index != -1) {
            ImGui::OpenPopup("Delete?");
            win->dbg.delete_breakpoint_index = del_bp_index;
        }
        if ((win->dbg.delete_breakpoint_index >= 0) && ImGui::BeginPopupModal("Delete?", 0, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Delete breakpoint at %04X?", win->dbg.breakpoints[win->dbg.delete_breakpoint_index].addr);
            ImGui::Separator();
            if (ImGui::Button("Ok", ImVec2(120, 0))) {
                _ui_dbg_bp_del(win, win->dbg.delete_breakpoint_index);
                ImGui::CloseCurrentPopup();
                win->dbg.delete_breakpoint_index = -1;
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
                win->dbg.delete_breakpoint_index = -1;
            }
            ImGui::EndPopup();
        }
        if (scroll_down) {
            ImGui::SetScrollHereY(1.0f);
        }
        ImGui::EndChild();
    }
    ImGui::End();
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

static void _ui_dbg_heatmap_clear_all(ui_dbg_t* win) {
    memset(win->heatmap.items, 0, sizeof(win->heatmap.items));
}

static void _ui_dbg_heatmap_clear_rw(ui_dbg_t* win) {
    for (int i = 0; i < (1<<16); i++) {
        win->heatmap.items[i].read_count = 0;
        win->heatmap.items[i].write_count = 0;
    }
}

static void _ui_dbg_heatmap_update(ui_dbg_t* win) {
    const int frame_chunk_height = 64;
    int y0 = win->heatmap.cur_y;
    int y1 = win->heatmap.cur_y + frame_chunk_height;
    win->heatmap.cur_y = (y0 + frame_chunk_height) & 255;
    for (int y = y0; y < y1; y++) {
        for (int x = 0; x < 256; x++) {
            int i = y * 256 + x;
            uint32_t p = 0;
            if (_ui_dbg_get_pc(win) == i) {
                p |= 0xFF00FFFF;
            }
            if (win->heatmap.show_ops && (win->heatmap.items[i].op_count > 0)) {
                uint32_t r = 0x60 + (win->heatmap.items[i].op_count>>8);
                if (r > 0xFF) { r = 0xFF; }
                p |= 0xFF000000 | r;
            }
            if (win->heatmap.show_ops && (win->heatmap.items[i].op_start != 0)) {
                /* opcode followup byte */
                uint32_t r = 0x60 + (win->heatmap.items[win->heatmap.items[i].op_start].op_count>>8);
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
    ImGui::SetNextWindowPos(ImVec2(win->ui.init_x + 60, win->ui.init_y + 60), ImGuiSetCond_Once);
    ImGui::SetNextWindowSize(ImVec2(288, 356), ImGuiSetCond_Once);
    if (ImGui::Begin("Memory Heatmap", &win->ui.show_heatmap)) {
        if (ImGui::Button("Clear All")) {
            _ui_dbg_heatmap_clear_all(win);
        }
        ImGui::SameLine();
        if (ImGui::Button("Clear R/W")) {
            _ui_dbg_heatmap_clear_rw(win);
        }
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, 0xFF0000FF);
        ImGui::Checkbox("OP", &win->heatmap.show_ops); ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, 0xFFFF0000);
        ImGui::Checkbox("R", &win->heatmap.show_reads); ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, 0xFF00FF00);
        ImGui::Checkbox("W", &win->heatmap.show_writes);
        ImGui::PopStyleColor(3);
        ImGui::SliderInt("Scale", &win->heatmap.scale, 1, 8);
        ImGui::BeginChild("##tex", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);
        ImVec2 screen_pos = ImGui::GetCursorScreenPos();
        ImVec2 mouse_pos = ImGui::GetMousePos();
        ImGui::Image(win->heatmap.texture, ImVec2(256*win->heatmap.scale, 256*win->heatmap.scale), ImVec2(0, 0), ImVec2(1, 1));
        int x = (int)((mouse_pos.x - screen_pos.x) / win->heatmap.scale);
        int y = (int)((mouse_pos.y - screen_pos.y) / win->heatmap.scale);
        uint16_t addr = y * 256 + x;
        if (win->heatmap.items[addr].op_start != 0) {
            /* address is actually an opcode followup byte, reset to start of instruction */
            addr = win->heatmap.items[addr].op_start;
        }
        if (ImGui::IsItemHovered()) {
            if (win->heatmap.items[addr].op_count > 0) {
                _ui_dbg_disasm(win, addr);
                ImGui::SetTooltip("%04X: %s (ticks: %d)\n(right-click for options)",
                    addr, win->dasm.str_buf, win->heatmap.items[addr].ticks);
            }
            else {
                ImGui::SetTooltip("%04X: %02X %02X %02X %02X\n(right-click for options)", addr,
                    _ui_dbg_read_byte(win, addr),
                    _ui_dbg_read_byte(win, addr+1),
                    _ui_dbg_read_byte(win, addr+2),
                    _ui_dbg_read_byte(win, addr+3));
            }
        }
        if (ImGui::BeginPopupContextItem("##popup")) {
            if (!win->heatmap.popup_addr_valid) {
                win->heatmap.popup_addr_valid = true;
                win->heatmap.popup_addr = addr;
            }
            ImGui::Text("Address: %04X", win->heatmap.popup_addr);
            ImGui::Separator();
            if (ImGui::Selectable("Add Exec Breakpoint")) {
                if (-1 == _ui_dbg_bp_find(win, UI_DBG_BREAKTYPE_EXEC, win->heatmap.popup_addr)) {
                    _ui_dbg_bp_add_exec(win, true, win->heatmap.popup_addr);
                }
            }
            if (ImGui::Selectable("Add Byte Breakpoint")) {
                if (-1 == _ui_dbg_bp_find(win, UI_DBG_BREAKTYPE_BYTE, win->heatmap.popup_addr)) {
                    _ui_dbg_bp_add_byte(win, false, win->heatmap.popup_addr);
                    win->ui.show_breakpoints = true;
                    ImGui::SetWindowFocus("Breakpoints");
                }
            }
            if (ImGui::Selectable("Add Word Breakpoint")) {
                if (-1 == _ui_dbg_bp_find(win, UI_DBG_BREAKTYPE_WORD, win->heatmap.popup_addr)) {
                    _ui_dbg_bp_add_word(win, false, win->heatmap.popup_addr);
                    win->ui.show_breakpoints = true;
                    ImGui::SetWindowFocus("Breakpoints");
                }
            }
            ImGui::EndPopup();
        }
        else {
            win->heatmap.popup_addr_valid = false;
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
    ui->show_breakpoints = false;
    ui->keys = desc->keys;
}

static void _ui_dbg_draw_menu(ui_dbg_t* win) {
    bool delete_all_bp = false;
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Debug")) {
            if (ImGui::MenuItem("Break", win->ui.keys.break_name, false, !win->dbg.stopped)) {
                _ui_dbg_break(win);
            }
            if (ImGui::MenuItem("Continue", win->ui.keys.continue_name, false, win->dbg.stopped)) {
                _ui_dbg_continue(win);
            }
            if (ImGui::MenuItem("Step Over", win->ui.keys.step_over_name, false, win->dbg.stopped)) {
                _ui_dbg_step_over(win);
            }
            if (ImGui::MenuItem("Step Into", win->ui.keys.step_into_name, false, win->dbg.stopped)) {
                _ui_dbg_step_into(win);
            }
            if (ImGui::MenuItem("Step Out", win->ui.keys.step_out_name, false, win->dbg.stopped)) {
                _ui_dbg_step_out(win);
            }
            ImGui::MenuItem("Install CPU Debug Hook", 0, &win->dbg.install_trap_cb);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Breakpoints")) {
            ImGui::MenuItem("Breakpoint Window", 0, &win->ui.show_breakpoints);
            if (ImGui::MenuItem("Toggle Breakpoint", "F9")) {
                _ui_dbg_bp_toggle_exec(win, _ui_dbg_get_pc(win));
            }
            if (ImGui::MenuItem("Add Breakpoint..")) {
                _ui_dbg_bp_add_exec(win, false, _ui_dbg_get_pc(win));
                win->ui.show_breakpoints = true;
                ImGui::SetWindowFocus("Breakpoints");
            }
            if (ImGui::MenuItem("Enable All")) {
                _ui_dbg_bp_enable_all(win);
            }
            if (ImGui::MenuItem("Disable All")) {
                _ui_dbg_bp_disable_all(win);
            }
            if (ImGui::MenuItem("Delete All")) {
                delete_all_bp = true;
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Show")) {
            ImGui::MenuItem("Memory Heatmap", 0, &win->ui.show_heatmap);
            ImGui::MenuItem("Registers", 0, &win->ui.show_regs);
            ImGui::MenuItem("Button Bar", 0, &win->ui.show_buttons);
            ImGui::MenuItem("Breakpoints", 0, &win->ui.show_breakpoints);
            ImGui::MenuItem("Opcode Bytes", 0, &win->ui.show_bytes);
            ImGui::MenuItem("Opcode Ticks", 0, &win->ui.show_ticks);
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
    if (delete_all_bp) {
        ImGui::OpenPopup("Delete All?");
    }
    _ui_dbg_bp_draw_delete_all_modal(win, "Delete All?");
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

/* handle keyboard input, the debug window must be focused for hotkeys to work! */
static void _ui_dbg_handle_input(ui_dbg_t* win) {
    /* unused hotkeys are defined as 0 and will never be triggered */
    if (win->dbg.stopped) {
        if (ImGui::IsKeyPressed(win->ui.keys.continue_keycode)) {
            _ui_dbg_continue(win);
        }
        if (ImGui::IsKeyPressed(win->ui.keys.step_over_keycode)) {
            _ui_dbg_step_over(win);
        }
        if (ImGui::IsKeyPressed(win->ui.keys.step_into_keycode)) {
            _ui_dbg_step_into(win);
        }
        if (ImGui::IsKeyPressed(win->ui.keys.step_out_keycode)) {
            _ui_dbg_step_out(win);
        }
    }
    else {
        if (ImGui::IsKeyPressed(win->ui.keys.break_keycode)) {
            _ui_dbg_break(win);
        }
    }
    if (ImGui::IsKeyPressed(win->ui.keys.toggle_breakpoint_keycode)) {
        _ui_dbg_bp_toggle_exec(win, _ui_dbg_get_pc(win));
    }
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

/* this updates the line array currently visualized by the disassembler
   listing, this only happens when the PC is outside the visible
   area 
*/
static void _ui_dbg_update_line_array(ui_dbg_t* win, uint16_t addr) {
    /* one half is backtraced from current PC, the other half is
       'forward tracked' from current PC
    */
    uint16_t bt_addr = addr;
    int i;
    for (i = 0; i < UI_DBG_NUM_BACKTRACE_LINES; i++) {
        bt_addr -= 1;
        if (win->heatmap.items[bt_addr].op_start) {
            bt_addr = win->heatmap.items[bt_addr].op_start;
        }
        int bt_index = UI_DBG_NUM_BACKTRACE_LINES - i - 1;
        win->ui.line_array[bt_index].addr = bt_addr;
        win->ui.line_array[bt_index].val = _ui_dbg_read_byte(win, bt_addr);
    }
    for (; i < UI_DBG_NUM_LINES; i++) {
        win->ui.line_array[i].addr = addr;
        addr = _ui_dbg_disasm(win, addr);
        win->ui.line_array[i].val = win->dasm.bin_buf[0];
    }
}

/* check if the address is outside the line_array (with safe area at front and back) */
static bool _ui_dbg_addr_inside_line_array(ui_dbg_t* win, uint16_t addr) {
    uint16_t first_addr = win->ui.line_array[16].addr;
    uint16_t last_addr = win->ui.line_array[UI_DBG_NUM_LINES-16].addr;
    if (first_addr == last_addr) {
        return false;
    }
    else if (first_addr < last_addr) {
        return (first_addr <= addr) && (addr <= last_addr);
    }
    else {
        /* address wraps around in line_addrs array */
        return (first_addr <= addr) || (addr <= last_addr);
    }
}

/* update the line array if necessary (content doesn't match anymore, or PC is outside) */
static bool _ui_dbg_line_array_needs_update(ui_dbg_t* win, uint16_t addr) {
    /* first check if the current address is outside the line array */
    if (!_ui_dbg_addr_inside_line_array(win, addr)) {
        return true;
    }
    /* if address is inside, check if the memory content is still the same */
    for (int i = 0; i < UI_DBG_NUM_LINES; i++) {
        if (win->ui.line_array[i].val != _ui_dbg_read_byte(win, win->ui.line_array[i].addr)) {
            return true;
        }
    }
    /* all ok, not dirty */
    return false;
}

static void _ui_dbg_draw_main(ui_dbg_t* win) {
    const float line_height = ImGui::GetTextLineHeight();
    ImGui::SetNextWindowContentSize(ImVec2(0, UI_DBG_NUM_LINES * line_height));
    ImGui::BeginChild("##main", ImGui::GetContentRegionAvail(), false);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0,0));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0,0));
    const float glyph_width = ImGui::CalcTextSize("F").x;
    const float cell_width = 3 * glyph_width;

    const ImU32 bp_enabled_color = 0xFF0000FF;
    const ImU32 bp_disabled_color = 0xFF000088;
    const ImU32 pc_color = 0xFF00FFFF;
    const ImU32 brd_color = 0xFF000000;

    /* update the line array if PC is outside or its content has become outdated */
    const uint16_t pc = _ui_dbg_get_pc(win);
    bool force_scroll = false;
    if (_ui_dbg_line_array_needs_update(win, pc)) {
        _ui_dbg_update_line_array(win, pc);
        force_scroll = true;
    }

    /* make sure the PC line is visible, but only when not stopped or stepping */
    const int safe_lines = 5;
    ImGuiListClipper clipper(UI_DBG_NUM_LINES, line_height);
    for (int line_i = 0; line_i < UI_DBG_NUM_LINES; line_i++) {
        uint16_t addr = win->ui.line_array[line_i].addr;
        bool in_safe_area = (line_i >= (clipper.DisplayStart+safe_lines)) && (line_i <= (clipper.DisplayEnd-safe_lines));
        bool is_pc_line = (addr == pc);
        if (is_pc_line &&
                (force_scroll ||
                (!in_safe_area && win->ui.request_scroll) ||
                (!in_safe_area && !win->dbg.stopped)))
        {
            win->ui.request_scroll = false;
            int scroll_to_line = line_i - safe_lines - 2;
            if (scroll_to_line < 0) {
                scroll_to_line = 0;
            }
            ImGui::SetScrollY(scroll_to_line * line_height);
        }
        if (is_pc_line) {
            break;
        }
    }

    /* draw the disassembly */
    for (int line_i = 0; line_i < UI_DBG_NUM_LINES; line_i++) {
        bool visible_line = (line_i >= clipper.DisplayStart) && (line_i < clipper.DisplayEnd);
        uint16_t addr = win->ui.line_array[line_i].addr;
        bool is_pc_line = (addr == pc);
        bool show_dasm = (line_i >= UI_DBG_NUM_BACKTRACE_LINES) || (win->heatmap.items[addr].op_count > 0);
        const uint16_t start_addr = addr;
        if (show_dasm) {
            addr = _ui_dbg_disasm(win, addr);
        }
        else {
            addr++;
        }
        const int num_bytes = addr - start_addr;

        /* skip rendering if not in visible area */
        if (!visible_line) {
            continue;
        }

        /* show data bytes or potential but not verified instructions as dimmed */
        if ((win->heatmap.items[start_addr].op_count > 0) || (start_addr == pc)) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_Text]);
        }
        else {
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        }

        /* column for breakpoint and step-cursor */
        ImVec2 pos = ImGui::GetCursorScreenPos();
        pos.y += 1;
        const float lh2 = (float)(int)(line_height/2);
        ImGui::PushID(line_i);
        if (ImGui::InvisibleButton("##bp", ImVec2(16, line_height))) {
            /* add or remove execution breakpoint */
            _ui_dbg_bp_toggle_exec(win, start_addr);
        }
        ImGui::PopID();
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const ImVec2 mid(pos.x + 7, pos.y + lh2);
        const int bp_index = _ui_dbg_bp_find(win, UI_DBG_BREAKTYPE_EXEC, start_addr);
        if (bp_index >= 0) {
            /* an execution breakpoint exists for this address */
            ImU32 bp_color = _ui_dbg_bp_enabled(win, bp_index) ? bp_enabled_color : bp_disabled_color;
            dl->AddCircleFilled(mid, 7, bp_color);
            dl->AddCircle(mid, 7, brd_color);
        }
        else if (ImGui::IsItemHovered()) {
            dl->AddCircle(mid, 7, bp_enabled_color);
        }
        /* current PC/step cursor */
        if (is_pc_line) {
            const ImVec2 a(pos.x + 2, pos.y);
            const ImVec2 b(pos.x + 12, pos.y + lh2);
            const ImVec2 c(pos.x + 2, pos.y + line_height);
            dl->AddTriangleFilled(a, b, c, pc_color);
            dl->AddTriangle(a, b, c, brd_color);
        }
        ImGui::SameLine();

        /* address */
        ImGui::Text("%04X:   ", start_addr);
        ImGui::SameLine();

        /* instruction bytes (optional) */
        float x = ImGui::GetCursorPosX();
        if (win->ui.show_bytes) {
            for (int n = 0; n < num_bytes; n++) {
                ImGui::SameLine(x + cell_width*n);
                uint8_t val;
                if (show_dasm) {
                    val = win->dasm.bin_buf[n];
                }
                else {
                    val = _ui_dbg_read_byte(win, start_addr+n);
                }
                ImGui::Text("%02X ", val);
            }
            x += cell_width * 4;
        }

        /* disassembled instruction */
        x += glyph_width * 4;
        ImGui::SameLine(x);
        if (show_dasm) {
            ImGui::Text("%s", win->dasm.str_buf);
        }
        else {
            ImGui::Text("???");
        }

        /* tick count */
        if (win->ui.show_ticks) {
            int ticks = win->heatmap.items[start_addr].ticks;
            x += glyph_width * 20; 
            ImGui::SameLine(x);
            if (ticks > 0) {
                ImGui::Text("%d", ticks);
            }
            else {
                ImGui::TextDisabled("?");
            }
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
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) {
            ImGui::CaptureKeyboardFromApp();
            _ui_dbg_handle_input(win);
        }
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
    _ui_dbg_heatmap_init(win, desc);
}

void ui_dbg_discard(ui_dbg_t* win) {
    CHIPS_ASSERT(win && win->valid);
    _ui_dbg_heatmap_discard(win);
    win->valid = false;
}

bool ui_dbg_before_exec(ui_dbg_t* win) {
    CHIPS_ASSERT(win && win->valid);
    if (win->dbg.install_trap_cb) {
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
        return !win->dbg.stopped;
    }
    else {
        return true;
    }
}

void ui_dbg_after_exec(ui_dbg_t* win) {
    CHIPS_ASSERT(win && win->valid);
    /* uninstall our trap callback, but only if it hasn't been overwritten */
    int trap_id = 0;
    #if defined(UI_DBG_USE_Z80)
        if (win->dbg.z80) {
            if (win->dbg.z80->trap_cb == _ui_dbg_bp_eval) {
                z80_trap_cb(win->dbg.z80, win->dbg.z80_trap_cb, win->dbg.z80_trap_ud);
            }
            win->dbg.z80_trap_cb = 0;
            win->dbg.z80_trap_ud = 0;
            trap_id = win->dbg.z80->trap_id;
        }
    #endif
    #if defined(UI_DBG_USE_M6502)
        if (win->dbg.m6502) {
            if (win->dbg.m6502->trap_cb == _ui_dbg_bp_eval) {
                m6502_trap_cb(win->dbg.m6502, win->dbg.m6502_trap_cb, win->dbg.m6502_trap_ud);
            }
            win->dbg.m6502_trap_cb = 0;
            win->dbg.m6502_trap_ud = 0;
            trap_id = win->dbg.m6502->trap_id;
        }
    #endif
    if (trap_id) {
        win->dbg.stopped = true;
        win->dbg.step_mode = UI_DBG_STEPMODE_NONE;
        ImGui::SetWindowFocus(win->ui.title);
        win->ui.open = true;
    }
}

void ui_dbg_draw(ui_dbg_t* win) {
    CHIPS_ASSERT(win && win->valid && win->ui.title);
    if (!(win->ui.open || win->ui.show_heatmap || win->ui.show_breakpoints)) {
        return;
    }
    _ui_dbg_dbgwin_draw(win);
    _ui_dbg_heatmap_draw(win);
    _ui_dbg_bp_draw(win);
}
#endif /* CHIPS_IMPL */
