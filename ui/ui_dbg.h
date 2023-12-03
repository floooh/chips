#pragma once
/*#
    # ui_dbg.h

    CPU debugger UI.

    Do this:
    ~~~C
    #define CHIPS_UI_IMPL
    ~~~
    before you include this file in *one* C++ file to create the
    implementation.

    Select the supported CPUs with the following macros (define one
    or the other, but not both):

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

#if !defined(UI_DBG_USE_Z80) && !defined(UI_DBG_USE_M6502)
#error "please define UI_DBG_USE_Z80 or UI_DBG_USE_M6502"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* NOTE: keep all MAX and NUM values 2^N */
#define UI_DBG_MAX_BREAKPOINTS (32)
#define UI_DBG_MAX_USER_BREAKTYPES (8)  /* max number of user breakpoint types */
#define UI_DBG_STEP_TRAPID (128)        /* special trap id when step-mode active */
#define UI_DBG_BP_BASE_TRAPID (UI_DBG_STEP_TRAPID+1)   /* first CPU trap-id used for breakpoints */
#define UI_DBG_NUM_LINES (256)
#define UI_DBG_NUM_BACKTRACE_LINES (UI_DBG_NUM_LINES/2)
#define UI_DBG_NUM_HISTORY_ITEMS (256)

/* breakpoint types */
enum {
    UI_DBG_BREAKTYPE_EXEC,      /* break on executed address */
    UI_DBG_BREAKTYPE_BYTE,      /* break on a specific 8-bit value at address */
    UI_DBG_BREAKTYPE_WORD,      /* break on a specific 16-bit value at address */
    UI_DBG_BREAKTYPE_IRQ,       /* break on maskable interrupt */
    UI_DBG_BREAKTYPE_NMI,       /* break on non-maskable interrupt */
    #if defined(UI_DBG_USE_Z80)
        UI_DBG_BREAKTYPE_OUT,   /* break on a Z80 out operation */
        UI_DBG_BREAKTYPE_IN,    /* break on a Z80 in operation */
    #endif
    UI_DBG_BREAKTYPE_USER,      /* user breakpoint types start here */
};
#define UI_DBG_MAX_BREAKTYPES (UI_DBG_BREAKTYPE_USER + UI_DBG_MAX_USER_BREAKTYPES)

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
    UI_DBG_STEPMODE_TICK,
};

enum {
    UI_DBG_STOP_REASON_UNKNOWN = 0,
    UI_DBG_STOP_REASON_BREAK = 1,
    UI_DBG_STOP_REASON_BREAKPOINT = 2,
    UI_DBG_STOP_REASON_STEP = 3,
};

/* a breakpoint description */
typedef struct ui_dbg_breakpoint_t {
    int type;           /* UI_DBG_BREAKTYPE_* */
    int cond;           /* UI_DBG_BREAKCOND_* */
    bool enabled;
    uint16_t addr;
    int val;
} ui_dbg_breakpoint_t;

/* breakpoint type description */
typedef struct ui_dbg_user_breaktype_t {
    const char* label;          /* human readable name */
    const char* val_label;      /* value field label or 0 for no label */
    bool show_addr;             /* show the address field ? */
    bool show_cmp;              /* show the comparision dropdown? */
    bool show_val8;             /* show the value field as byte? */
    bool show_val16;            /* show the value field as word? */
} ui_dbg_breaktype_t;

/* forward decl */
struct ui_dbg_t;
/* callback for reading a byte from memory */
typedef uint8_t (*ui_dbg_read_t)(int layer, uint16_t addr, void* user_data);
/* callback for evaluating uer breakpoints, return breakpoint index, or -1 */
typedef int (*ui_dbg_user_break_t)(struct ui_dbg_t* win, int trap_id, uint64_t pins, void* user_data);
/* a callback to create a dynamic-update RGBA8 UI texture, needs to return an ImTextureID handle */
typedef void* (*ui_dbg_create_texture_t)(int w, int h);
/* callback to update a UI texture with new data */
typedef void (*ui_dbg_update_texture_t)(void* tex_handle, void* data, int data_byte_size);
/* callback to destroy a UI texture */
typedef void (*ui_dbg_destroy_texture_t)(void* tex_handle);
/* callback when emulator has stopped at an address (stop_reason is UI_DBG_STOP_REASON_XXX) */
typedef void (*ui_dbg_stopped_t)(int stop_reason, uint16_t addr);
/* callback when emulator has continued after stopped state */
typedef void (*ui_dbg_continued_t)(void);

/* user-defined hotkeys (all strings must be static) */
typedef struct ui_dbg_key_desc_t {
    int keycode;    // ImGuiKey_*
    const char* name;
} ui_dbg_key_desc_t;

typedef struct ui_dbg_keys_desc_t {
    ui_dbg_key_desc_t cont;
    ui_dbg_key_desc_t stop;
    ui_dbg_key_desc_t step_over;
    ui_dbg_key_desc_t step_into;
    ui_dbg_key_desc_t step_tick;
    ui_dbg_key_desc_t toggle_breakpoint;
} ui_dbg_keys_desc_t;

typedef struct ui_dbg_texture_callbacks_t {
    ui_dbg_create_texture_t create_cb;      // callback to create UI texture
    ui_dbg_update_texture_t update_cb;      // callback to update UI texture
    ui_dbg_destroy_texture_t destroy_cb;    // callback to destroy UI texture
} ui_dbg_texture_callbacks_t;

typedef struct ui_dbg_debug_callbacks_t {
    ui_dbg_stopped_t stopped_cb;
    ui_dbg_continued_t continued_cb;
} ui_dbg_debug_callbacks_t;

typedef struct ui_dbg_desc_t {
    const char* title;          /* window title */
    #if defined(UI_DBG_USE_Z80)
    z80_t* z80;                 /* Z80 CPU to track */
    #elif defined(UI_DBG_USE_M6502)
    m6502_t* m6502;             /* 6502 CPU to track */
    #endif
    ui_dbg_read_t read_cb;          /* callback to read memory */
    int read_layer;                 /* layer argument for read_cb */
    ui_dbg_user_break_t break_cb;   /* optional user-breakpoint evaluation callback */
    ui_dbg_texture_callbacks_t texture_cbs;
    ui_dbg_debug_callbacks_t debug_cbs;
    void* user_data;            /* user data for callbacks */
    int x, y;                   /* initial window pos */
    int w, h;                   /* initial window size, or 0 for default size */
    bool open;                  /* initial open state */
    ui_dbg_keys_desc_t keys;      /* user-defined hotkeys */
    ui_dbg_breaktype_t user_breaktypes[UI_DBG_MAX_USER_BREAKTYPES];  /* user-defined breakpoint types */
} ui_dbg_desc_t;

/* debugger state */
typedef struct ui_dbg_state_t {
    #if defined(UI_DBG_USE_Z80)
    z80_t* z80;
    #elif defined(UI_DBG_USE_M6502)
    m6502_t* m6502;
    #endif
    bool stopped;
    bool external_debugger_connected;
    int step_mode;
    uint64_t last_tick_pins;    // cpu pins in last tick
    uint32_t frame_id;          // used in trap callback to detect when a new frame has started
    uint32_t cur_op_ticks;
    uint16_t cur_op_pc;         // PC of current instruction
    uint16_t stepover_pc;
    int last_trap_id;           // can be used to identify breakpoint which caused trap
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
    bool show_history;
    bool request_scroll;
    ui_dbg_keys_desc_t keys;
    ui_dbg_line_t line_array[UI_DBG_NUM_LINES];
    int num_breaktypes;
    ui_dbg_breaktype_t breaktypes[UI_DBG_MAX_BREAKTYPES];
    const char* breaktype_combo_labels[UI_DBG_MAX_BREAKTYPES];
} ui_dbg_uistate_t;

enum {
    UI_DBG_HEATMAP_ITEM_OPCODE = (1<<0),
    UI_DBG_HEATMAP_ITEM_WRITE  = (1<<1),
    UI_DBG_HEATMAP_ITEM_READ   = (1<<2),
};

typedef struct ui_dbg_heatmapitem_t {
    uint8_t state;  // UI_DBG_HEATMAP_ITEM_xxx
    uint8_t ticks;  // instruction tick count
} ui_dbg_heatmapitem_t;

typedef struct ui_dbg_heatmap_t {
    int tex_width, tex_height;
    int tex_width_uicombo_state;
    int next_tex_width;
    void* texture;
    bool show_ops, show_reads, show_writes;
    int autoclear_interval; /* 0: no autoclear */
    int scale;
    int cur_y;
    bool popup_addr_valid;
    uint16_t popup_addr;
    ui_dbg_heatmapitem_t items[1<<16];     /* execution counter map */
    uint32_t pixels[1<<16];    /* execution counters converted to pixel data */
} ui_dbg_heatmap_t;

typedef struct ui_dbg_history_t {
    uint16_t pc[UI_DBG_NUM_HISTORY_ITEMS];
    uint16_t pos;
} ui_dbg_history_t;

enum {
    UI_DBG_DASM_LINE_MAX_BYTES = 8,
    UI_DBG_DASM_LINE_MAX_CHARS = 32,
};

typedef struct ui_dbg_dasm_line_t {
    uint16_t addr;
    uint8_t num_bytes;
    uint8_t num_chars;
    uint8_t bytes[UI_DBG_DASM_LINE_MAX_BYTES];
    char chars[UI_DBG_DASM_LINE_MAX_CHARS];
} ui_dbg_dasm_line_t;

typedef struct ui_dbg_dasm_request_t {
    uint16_t addr;                  // base address
    int offset_lines;               // offset in number of ops/lines, may be negative
    int num_lines;                  // number of lines to disassemble
    ui_dbg_dasm_line_t* out_lines;  // pointer to output ops, must have at least num_ops entries
} ui_dbg_dasm_request_t;

typedef struct ui_dbg_t {
    bool valid;
    ui_dbg_read_t read_cb;
    int read_layer;
    ui_dbg_user_break_t break_cb;
    ui_dbg_texture_callbacks_t texture_cbs;
    ui_dbg_debug_callbacks_t debug_cbs;
    void* user_data;
    ui_dbg_dasm_line_t dasm_line;
    ui_dbg_state_t dbg;
    ui_dbg_uistate_t ui;
    ui_dbg_heatmap_t heatmap;
    ui_dbg_history_t history;
} ui_dbg_t;

/* initialize a new ui_dbg_t instance */
void ui_dbg_init(ui_dbg_t* win, ui_dbg_desc_t* desc);
/* discard ui_dbg_t instance */
void ui_dbg_discard(ui_dbg_t* win);
/* notify ui_dbg that an external debugger has connected (may change some behaviour) */
void ui_dbg_external_debugger_connected(ui_dbg_t* win);
/* notify ui_dbg that an external debugger has disconnected (clears breakpoints and continues) */
void ui_dbg_external_debugger_disconnected(ui_dbg_t* win);
/* render the ui_dbg_t UIs */
void ui_dbg_draw(ui_dbg_t* win);
/* call after ticking the system */
void ui_dbg_tick(ui_dbg_t* win, uint64_t pins);
/* call when resetting the emulated machine (re-initializes some data structures) */
void ui_dbg_reset(ui_dbg_t* win);
/* call when rebooting the emulated machine (re-initializes some data structures) */
void ui_dbg_reboot(ui_dbg_t* win);
/* set an execution breakpoint at address */
void ui_dbg_add_breakpoint(ui_dbg_t* win, uint16_t addr);
/* clear an execution breakpoint at address */
void ui_dbg_remove_breakpoint(ui_dbg_t* win, uint16_t addr);
/* pause/stop execution */
void ui_dbg_break(ui_dbg_t* win);
/* continue execution */
void ui_dbg_continue(ui_dbg_t* win, bool invoke_continued_cb);
/* return true if the debugger is currently stopped */
bool ui_dbg_stopped(ui_dbg_t* win);
/* perform a debugger step-next (step over) */
void ui_dbg_step_next(ui_dbg_t* win);
/* peform a debugger step-into */
void ui_dbg_step_into(ui_dbg_t* win);
/* request a disassembly at start address */
void ui_dbg_disassemble(ui_dbg_t* win, const ui_dbg_dasm_request_t* request);

#ifdef __cplusplus
} /* extern "C" */
#endif

/*-- IMPLEMENTATION ----------------------------------------------------------*/
#ifdef CHIPS_UI_IMPL
#include <string.h>
#ifndef CHIPS_ASSERT
    #include <assert.h>
    #define CHIPS_ASSERT(c) assert(c)
#endif

/*== GENERAL HELPERS =========================================================*/
static inline const char* _ui_dbg_str_or_def(const char* str, const char* def) {
    if (str) {
        return str;
    } else {
        return def;
    }
}

static inline uint8_t _ui_dbg_read_byte(ui_dbg_t* win, uint16_t addr) {
    return win->read_cb(win->read_layer, addr, win->user_data);
}

static inline uint16_t _ui_dbg_read_word(ui_dbg_t* win, uint16_t addr) {
    uint8_t l = win->read_cb(win->read_layer, addr, win->user_data);
    uint8_t h = win->read_cb(win->read_layer, addr+1, win->user_data);
    return (uint16_t) (h<<8)|l;
}

static inline uint16_t _ui_dbg_get_pc(ui_dbg_t* win) {
    return win->dbg.cur_op_pc;
}

/* disassembler callback to fetch the next instruction byte */
static uint8_t _ui_dbg_dasm_in_cb(void* user_data) {
    ui_dbg_t* win = (ui_dbg_t*) user_data;
    uint8_t val = _ui_dbg_read_byte(win, win->dasm_line.addr++);
    if (win->dasm_line.num_bytes < UI_DBG_DASM_LINE_MAX_BYTES) {
        win->dasm_line.bytes[win->dasm_line.num_bytes++] = val;
    }
    return val;
}

/* disassembler callback to output a character */
static void _ui_dbg_dasm_out_cb(char c, void* user_data) {
    ui_dbg_t* win = (ui_dbg_t*) user_data;
    if ((win->dasm_line.num_chars + 1) < UI_DBG_DASM_LINE_MAX_CHARS) {
        win->dasm_line.chars[win->dasm_line.num_chars++] = c;
        win->dasm_line.chars[win->dasm_line.num_chars] = 0;
    }
}

// disassemble instruction at address
static inline uint16_t _ui_dbg_disasm(ui_dbg_t* win, uint16_t addr) {
    memset(&win->dasm_line, 0, sizeof(win->dasm_line));
    win->dasm_line.addr = addr;
    #if defined(UI_DBG_USE_Z80)
        z80dasm_op(addr, _ui_dbg_dasm_in_cb, _ui_dbg_dasm_out_cb, win);
    #elif defined(UI_DBG_USE_M6502)
        m6502dasm_op(addr, _ui_dbg_dasm_in_cb, _ui_dbg_dasm_out_cb, win);
    #endif
    uint16_t next_addr = win->dasm_line.addr;
    win->dasm_line.addr = addr;
    return next_addr;
}

/* disassemble the an instruction, but only return the length of the instruction */
/* check if the an instruction is a 'step over' op */
static bool _ui_dbg_is_stepover_op(uint8_t opcode) {
    #if defined(UI_DBG_USE_Z80)
        switch (opcode) {
            /* CALL nnnn */
            case 0xCD:
            /* CALL cc,nnnn */
            case 0xDC: case 0xFC: case 0xD4: case 0xC4:
            case 0xF4: case 0xEC: case 0xE4: case 0xCC:
            /* DJNZ d */
            case 0x10:
                return true;
            default:
                return false;
        }
    #elif defined(UI_DBG_USE_M6502)
        /* on 6502, only JSR qualifies */
        return opcode == 0x20;
    #endif
}

/* check if an instruction is a control-flow op */
static bool _ui_dbg_is_controlflow_op(uint8_t opcode0, uint8_t opcode1) {
    #if defined(UI_DBG_USE_Z80)
        switch (opcode0) {
            /* CALL nnnn */
            case 0xCD:
            /* CALL cc,nnnn */
            case 0xDC: case 0xFC: case 0xD4: case 0xC4:
            case 0xF4: case 0xEC: case 0xE4: case 0xCC:
            /* JP nnnn */
            case 0xC3:
            /* JP cc,nnnn */
            case 0xDA: case 0xFA: case 0xD2: case 0xC2:
            case 0xF2: case 0xEA: case 0xE2: case 0xCA:
            /* DJNZ d */
            case 0x10:
            /* JR d */
            case 0x18:
            /* JR cc,d */
            case 0x38: case 0x30: case 0x20: case 0x28:
            /* RST */
            case 0xC7:  case 0xCF: case 0xD7: case 0xDF:
            case 0xE7:  case 0xEF: case 0xF7: case 0xFF:
            /* HALT */
            case 0x76:
            /* RET */
            case 0xC9:
            /* RET cc */
            case 0xC0: case 0xC8: case 0xD0: case 0xD8:
            case 0xE0: case 0xE8: case 0xF0: case 0xF8:
            /* JP (HL) */
            case 0xE9:
                return true;
            /* RETN, RETI */
            case 0xED:
                switch (opcode1) {
                    case 0x45: case 0x55: case 0x65: case 0x75:
                    case 0x4D: case 0x5D: case 0x6D: case 0x7D:
                        return true;
                    default:
                        return false;
                }
                break;
            /* JP (IX), JP (IY) */
            case 0xDD: case 0xFD:
                return opcode1 == 0xE9;
            default:
                return false;
        }
    #elif defined(UI_DBG_USE_M6502)
        (void)opcode1;
        switch (opcode0) {
            /* BRK */
            case 0x00:
            /* JSR/JMP abs */
            case 0x20: case 0x4C:
            /* JMP ind */
            case 0x6C:
            /* relative branch */
            case 0x10: case 0x30: case 0x50: case 0x70:
            case 0x90: case 0xB0: case 0xD0: case 0xF0:
                return true;
            /* RTI */
            case 0x40:
            /* RTS */
            case 0x60:
                return true;
            default:
                return false;
        }
    #endif
}

static void _ui_dbg_break(ui_dbg_t* win) {
    win->dbg.stopped = true;
    win->dbg.step_mode = UI_DBG_STEPMODE_NONE;
    win->ui.request_scroll = true;
    if (win->debug_cbs.stopped_cb) {
        win->debug_cbs.stopped_cb(UI_DBG_STOP_REASON_BREAK, win->dbg.cur_op_pc);
    }
}

static void _ui_dbg_continue(ui_dbg_t* win, bool invoke_continue_cb) {
    win->dbg.stopped = false;
    win->dbg.step_mode = UI_DBG_STEPMODE_NONE;
    if (invoke_continue_cb && win->debug_cbs.continued_cb) {
        win->debug_cbs.continued_cb();
    }
}

static void _ui_dbg_step_into(ui_dbg_t* win) {
    win->dbg.stopped = false;
    win->dbg.step_mode = UI_DBG_STEPMODE_INTO;
    win->ui.request_scroll = true;
}

static void _ui_dbg_step_over(ui_dbg_t* win) {
    win->dbg.stopped = false;
    win->ui.request_scroll = true;
    uint16_t next_pc = _ui_dbg_disasm(win, _ui_dbg_get_pc(win));
    if (_ui_dbg_is_stepover_op(win->dasm_line.bytes[0])) {
        win->dbg.step_mode = UI_DBG_STEPMODE_OVER;
        win->dbg.stepover_pc = next_pc;
    } else {
        win->dbg.step_mode = UI_DBG_STEPMODE_INTO;
    }
}

static void _ui_dbg_step_tick(ui_dbg_t* win) {
    win->dbg.stopped = false;
    win->dbg.step_mode = UI_DBG_STEPMODE_TICK;
    win->ui.request_scroll = true;
}

/*== HISTORY =================================================================*/
static void _ui_dbg_history_reset(ui_dbg_t* win) {
    memset(&win->history, 0, sizeof(win->history));
}

static void _ui_dbg_history_reboot(ui_dbg_t* win) {
    _ui_dbg_history_reset(win);
}

static void _ui_dbg_history_push(ui_dbg_t* win, uint16_t pc) {
    win->history.pc[win->history.pos] = pc;
    win->history.pos = (win->history.pos + 1) & (UI_DBG_NUM_HISTORY_ITEMS-1);
}

static uint16_t _ui_dbg_history_get(ui_dbg_t* win, uint16_t rel_pos) {
    uint16_t index = (win->history.pos - rel_pos - 1) & (UI_DBG_NUM_HISTORY_ITEMS-1);
    return win->history.pc[index];
}

static void _ui_dbg_history_draw(ui_dbg_t* win) {
    if (!win->ui.show_history) {
        return;
    }
    ImGui::SetNextWindowPos(ImVec2(win->ui.init_x + win->ui.init_w, win->ui.init_y + 64), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(win->ui.init_w, 376), ImGuiCond_Once);
    if (ImGui::Begin("Execution History", &win->ui.show_history)) {
        const float line_height = ImGui::GetTextLineHeight();
        ImGui::SetNextWindowContentSize(ImVec2(0, UI_DBG_NUM_HISTORY_ITEMS * line_height));
        ImGui::BeginChild("##main", ImGui::GetContentRegionAvail(), false);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0,0));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0,0));
        const float glyph_width = ImGui::CalcTextSize("F").x;
        const float cell_width = 3 * glyph_width;

        ImGuiListClipper clipper;
        clipper.Begin(UI_DBG_NUM_LINES, line_height);
        clipper.Step();
        for (int line_i = 0; line_i < UI_DBG_NUM_LINES; line_i++) {

            /* skip rendering if not in visible area */
            bool visible_line = (line_i >= clipper.DisplayStart) && (line_i < clipper.DisplayEnd);
            if (!visible_line) {
                continue;
            }

            /* get history PC */
            uint16_t pc = _ui_dbg_history_get(win, line_i);
            uint16_t addr = _ui_dbg_disasm(win, pc);
            const int num_bytes = addr - pc;

            /* address */
            if (0 == line_i) {
                ImGui::Text("cur> %04X:   ", pc);
            } else {
                ImGui::Text("%4d %04X:   ", -line_i, pc);
            }
            ImGui::SameLine();

            /* instruction bytes (optional) */
            float x = ImGui::GetCursorPosX();
            if (win->ui.show_bytes) {
                for (int n = 0; n < num_bytes; n++) {
                    ImGui::SameLine(x + cell_width * n);
                    uint8_t val = win->dasm_line.bytes[n];
                    ImGui::Text("%02X ", val);
                }
                x += cell_width * 4;
            }

            /* disassembled instruction */
            x += glyph_width * 4;
            ImGui::SameLine(x);
            ImGui::Text("%s", win->dasm_line.bytes);

            /* tick count */
            x += glyph_width * 17;
            if (win->ui.show_ticks) {
                int ticks = win->heatmap.items[pc].ticks;
                ImGui::SameLine(x);
                ImGui::Text("%d", ticks);
            }
        }
        clipper.End();
        ImGui::PopStyleVar(2);
        ImGui::EndChild();
    }
    ImGui::End();
}


/*== DEBUGGER STATE ==========================================================*/
static void _ui_dbg_dbgstate_init(ui_dbg_t* win, ui_dbg_desc_t* desc) {
    ui_dbg_state_t* dbg = &win->dbg;
    #if defined(UI_DBG_USE_Z80)
        CHIPS_ASSERT(desc->z80);
        dbg->z80 = desc->z80;
    #elif defined(UI_DBG_USE_M6502)
        CHIPS_ASSERT(desc->m6502);
        dbg->m6502 = desc->m6502;
    #endif
    dbg->delete_breakpoint_index = -1;
}

static void _ui_dbg_dbgstate_reset(ui_dbg_t* win) {
    ui_dbg_state_t* dbg = &win->dbg;
    dbg->stopped = false;
    dbg->step_mode = UI_DBG_STEPMODE_NONE;
    dbg->cur_op_pc = 0;
    dbg->last_trap_id = 0;
}

static void _ui_dbg_dbgstate_reboot(ui_dbg_t* win) {
    _ui_dbg_dbgstate_reset(win);
}

// evaluate per-opcode breakpoints, called at the start of a new instrucion
static int _ui_dbg_eval_op_breakpoints(ui_dbg_t* win, int trap_id, uint16_t pc) {
    if (win->dbg.step_mode != UI_DBG_STEPMODE_NONE) {
        switch (win->dbg.step_mode) {
            case UI_DBG_STEPMODE_INTO:
                trap_id = UI_DBG_STEP_TRAPID;
                break;
            case UI_DBG_STEPMODE_OVER:
                #if defined(UI_DBG_USE_Z80)
                if (pc == win->dbg.stepover_pc) {
                    trap_id = UI_DBG_STEP_TRAPID;
                }
                #elif defined(UI_DBG_USE_M6502)
                if (pc == win->dbg.stepover_pc) {
                    trap_id = UI_DBG_STEP_TRAPID;
                }
                #endif
                break;
        }
    } else {
        for (int i = 0; (i < win->dbg.num_breakpoints) && (trap_id == 0); i++) {
            const ui_dbg_breakpoint_t* bp = &win->dbg.breakpoints[i];
            if (bp->enabled) {
                switch (bp->type) {
                    case UI_DBG_BREAKTYPE_EXEC:
                        if (pc == bp->addr) {
                            trap_id = UI_DBG_BP_BASE_TRAPID + i;
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
                                trap_id = UI_DBG_BP_BASE_TRAPID + i;
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
                                trap_id = UI_DBG_BP_BASE_TRAPID + i;
                            }
                        }
                        break;
                }
            }
        }
    }
    return trap_id;
}

//  evaluate per-tick breakpoints, only call this if is dbg.step_mode is UI_DBG_STEPMODE_NONE!
static int _ui_dbg_eval_tick_breakpoints(ui_dbg_t* win, int trap_id, uint64_t pins) {
    uint64_t rising_pins = pins & (pins ^ win->dbg.last_tick_pins);
    for (int i = 0; (i < win->dbg.num_breakpoints) && (trap_id == 0); i++) {
        const ui_dbg_breakpoint_t* bp = &win->dbg.breakpoints[i];
        if (bp->enabled) {
            switch (bp->type) {
                case UI_DBG_BREAKTYPE_IRQ:
                    #if defined(UI_DBG_USE_Z80)
                        if (Z80_INT & rising_pins) {
                            trap_id = UI_DBG_BP_BASE_TRAPID + i;
                        }
                    #elif defined(UI_DBG_USE_M6502)
                        if (M6502_IRQ & rising_pins) {
                            trap_id = UI_DBG_BP_BASE_TRAPID + i;
                        }
                    #endif
                    break;

                case UI_DBG_BREAKTYPE_NMI:
                    #if defined(UI_DBG_USE_Z80)
                        if (Z80_NMI & rising_pins) {
                            trap_id = UI_DBG_BP_BASE_TRAPID + i;
                        }
                    #elif defined(UI_DBG_USE_M6502)
                        if (M6502_NMI & rising_pins) {
                            trap_id = UI_DBG_BP_BASE_TRAPID + i;
                        }
                    #endif
                    break;

                #if defined(UI_DBG_USE_Z80)
                case UI_DBG_BREAKTYPE_OUT:
                    if ((pins & Z80_CTRL_PIN_MASK) == (Z80_IORQ|Z80_WR)) {
                        const uint16_t mask = bp->val;
                        if ((Z80_GET_ADDR(pins) & mask) == (bp->addr & mask)) {
                            trap_id = UI_DBG_BP_BASE_TRAPID + i;
                        }
                    }
                    break;

                case UI_DBG_BREAKTYPE_IN:
                    if ((pins & Z80_CTRL_PIN_MASK) == (Z80_IORQ|Z80_RD)) {
                        const uint16_t mask = bp->val;
                        if ((Z80_GET_ADDR(pins) & mask) == (bp->addr & mask)) {
                            trap_id = UI_DBG_BP_BASE_TRAPID + i;
                        }
                    }
                    break;
                #endif
            }
        }
    }

    // call optional user-breakpoint evaluation callback
    if ((0 == trap_id) && win->break_cb) {
        trap_id = win->break_cb(win, trap_id, pins, win->user_data);
    }
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
    } else {
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
    } else {
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
    } else {
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
    } else {
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
    ImGui::SetNextWindowPos(ImVec2(win->ui.init_x + win->ui.init_w, win->ui.init_y), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(-1, 256), ImGuiCond_Once);
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
            CHIPS_ASSERT((bp->type >= 0) && (bp->type < UI_DBG_MAX_BREAKTYPES));
            /* visualize the current breakpoint */
            bool bp_active = (win->dbg.last_trap_id >= UI_DBG_BP_BASE_TRAPID) &&
                             ((win->dbg.last_trap_id - UI_DBG_BP_BASE_TRAPID) == i);
            if (bp_active) {
                ImGui::PushStyleColor(ImGuiCol_CheckMark, 0xFF0000FF);
            }
            ImGui::Checkbox("##enabled", &bp->enabled); ImGui::SameLine();
            if (bp_active) {
                ImGui::PopStyleColor();
            }
            if (ImGui::IsItemHovered()) {
                if (bp->enabled) {
                    ImGui::SetTooltip("Disable Breakpoint");
                } else {
                    ImGui::SetTooltip("Enable Breakpoint");
                }
            }
            ImGui::SameLine();
            bool upd_val = false;
            ImGui::PushItemWidth(112);
            if (ImGui::Combo("##type", &bp->type, win->ui.breaktype_combo_labels, win->ui.num_breaktypes)) {
                upd_val = true;
            }
            ui_dbg_breaktype_t* bt = &win->ui.breaktypes[bp->type];
            CHIPS_ASSERT(bt->label);
            ImGui::PopItemWidth();
            if (bt->show_addr) {
                ImGui::SameLine();
                uint16_t old_addr = bp->addr;
                bp->addr = ui_util_input_u16("##addr", old_addr);
                if (upd_val || (old_addr != bp->addr)) {
                    /* if breakpoint type or address has changed, update the breakpoint's value from memory */
                    switch (bp->type) {
                        case UI_DBG_BREAKTYPE_BYTE:
                            bp->val = (int) _ui_dbg_read_byte(win, bp->addr);
                            break;
                        case UI_DBG_BREAKTYPE_WORD:
                            bp->val = (int) _ui_dbg_read_word(win, bp->addr);
                            break;
                        #if defined(UI_DBG_USE_Z80)
                        case UI_DBG_BREAKTYPE_OUT:
                        case UI_DBG_BREAKTYPE_IN:
                            bp->val = 0x00FF;
                            break;
                        #endif
                        default:
                            bp->val = 0;
                            break;
                    }
                }
            }
            if (bt->show_cmp) {
                ImGui::SameLine();
                ImGui::PushItemWidth(42);
                ImGui::Combo("##cond", &bp->cond,"==\0!=\0>\0<\0>=\0<=\0");
                ImGui::PopItemWidth();
            }
            if (bt->show_val8 || bt->show_val16) {
                if (bt->val_label) {
                    ImGui::SameLine();
                    ImGui::Text("%s", bt->val_label);
                }
                if (bt->show_val8) {
                    ImGui::SameLine();
                    bp->val = (int) ui_util_input_u8("##byte", (uint8_t)bp->val);
                } else {
                    ImGui::SameLine();
                    bp->val = (int) ui_util_input_u16("##word", (uint16_t)bp->val);
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
static void _ui_dbg_heatmap_init(ui_dbg_t* win) {
    win->heatmap.tex_width = 256;
    win->heatmap.tex_height = 256;
    win->heatmap.tex_width_uicombo_state = 4;
    win->heatmap.next_tex_width = win->heatmap.tex_width;
    win->heatmap.texture = win->texture_cbs.create_cb(win->heatmap.tex_width, win->heatmap.tex_height);
    win->heatmap.show_ops = win->heatmap.show_reads = win->heatmap.show_writes = true;
    win->heatmap.scale = 1;
    win->heatmap.autoclear_interval = 0;  /* 0 means: no autoclear */
}

static void _ui_dbg_heatmap_discard(ui_dbg_t* win) {
    win->texture_cbs.destroy_cb(win->heatmap.texture);
}

static void _ui_dbg_heatmap_update_texture_size(ui_dbg_t* win, int new_width) {
    CHIPS_ASSERT(((1<<16) % new_width) == 0);
    if (new_width != win->heatmap.tex_width) {
        win->heatmap.tex_width = new_width;
        win->heatmap.tex_height = (1<<16) / new_width;
        win->texture_cbs.destroy_cb(win->heatmap.texture);
        win->heatmap.texture = win->texture_cbs.create_cb(win->heatmap.tex_width, win->heatmap.tex_height);
    }
}

static void _ui_dbg_heatmap_reset(ui_dbg_t* win) {
    win->heatmap.popup_addr_valid = false;
    win->heatmap.popup_addr = 0;
    memset(win->heatmap.items, 0, sizeof(win->heatmap.items));
}

static void _ui_dbg_heatmap_reboot(ui_dbg_t* win) {
    _ui_dbg_heatmap_reset(win);
}

static void _ui_dbg_heatmap_clear_all(ui_dbg_t* win) {
    memset(win->heatmap.items, 0, sizeof(win->heatmap.items));
}

static void _ui_dbg_heatmap_clear_rw(ui_dbg_t* win) {
    for (int i = 0; i < (1<<16); i++) {
        win->heatmap.items[i].state &= ~(UI_DBG_HEATMAP_ITEM_READ|UI_DBG_HEATMAP_ITEM_WRITE);
    }
}

static void _ui_dbg_heatmap_record_op(ui_dbg_t* win, uint16_t pc) {
    // record per-op heatmap events
    win->heatmap.items[pc].state |= UI_DBG_HEATMAP_ITEM_OPCODE;
    // update last instruction's ticks
    win->heatmap.items[win->dbg.cur_op_pc].ticks = win->dbg.cur_op_ticks;
}

static void _ui_dbg_heatmap_record_tick(ui_dbg_t* win, uint64_t pins) {
    #if defined(UI_DBG_USE_Z80)
        if ((pins & Z80_CTRL_PIN_MASK) == (Z80_MREQ|Z80_RD)) {
            const uint16_t addr = Z80_GET_ADDR(pins);
            win->heatmap.items[addr].state |= UI_DBG_HEATMAP_ITEM_READ;
        } else if ((pins & Z80_CTRL_PIN_MASK) == (Z80_MREQ|Z80_WR)) {
            const uint16_t addr = Z80_GET_ADDR(pins);
            win->heatmap.items[addr].state |= UI_DBG_HEATMAP_ITEM_WRITE;
        }
    #elif defined(UI_DBG_USE_M6502)
        const uint16_t addr = M6502_GET_ADDR(pins);
        if (0 != (pins & M6502_RW)) {
            win->heatmap.items[addr].state |= UI_DBG_HEATMAP_ITEM_READ;
        } else {
            win->heatmap.items[addr].state |= UI_DBG_HEATMAP_ITEM_WRITE;
        }
    #endif
}

static inline bool _ui_dbg_heatmap_is_opcode(ui_dbg_t* win, uint16_t addr) {
    return 0 != (win->heatmap.items[addr].state & UI_DBG_HEATMAP_ITEM_OPCODE);
}

static inline bool _ui_dbg_heatmap_is_read(ui_dbg_t* win, uint16_t addr) {
    return 0 != (win->heatmap.items[addr].state & UI_DBG_HEATMAP_ITEM_READ);
}

static inline bool _ui_dbg_heatmap_is_write(ui_dbg_t* win, uint16_t addr) {
    return 0 != (win->heatmap.items[addr].state & UI_DBG_HEATMAP_ITEM_WRITE);
}

static void _ui_dbg_heatmap_update(ui_dbg_t* win) {
    const int frame_chunk_height = 64;
    const int y0 = win->heatmap.cur_y;
    const int y1 = win->heatmap.cur_y + frame_chunk_height;
    win->heatmap.cur_y = (y0 + frame_chunk_height) & 255;
    for (int y = y0; y < y1; y++) {
        for (int x = 0; x < 256; x++) {
            const int i = y * 256 + x;
            uint32_t p = 0;
            if (_ui_dbg_get_pc(win) == i) {
                p |= 0xFF00FFFF;
            }
            if (win->heatmap.show_ops && _ui_dbg_heatmap_is_opcode(win, (uint16_t)i)) {
                p |= 0xFF0000FF;
            }
            if (win->heatmap.show_writes && _ui_dbg_heatmap_is_write(win, (uint16_t)i)) {
                p |= 0xFF008800;
            }
            if (win->heatmap.show_reads && _ui_dbg_heatmap_is_read(win, (uint16_t)i)) {
                p |= 0xFF880000;
            }
            win->heatmap.pixels[i] = p;
        }
    }
    win->texture_cbs.update_cb(win->heatmap.texture, win->heatmap.pixels, 256*256*4);
}

static void _ui_dbg_heatmap_draw(ui_dbg_t* win) {
    if (!win->ui.show_heatmap) {
        return;
    }
    ui_dbg_heatmap_t* hm = &win->heatmap;

    if (hm->next_tex_width != hm->tex_width) {
        _ui_dbg_heatmap_update_texture_size(win, hm->next_tex_width);
    }
    if (hm->autoclear_interval > 0) {
        if ((win->dbg.frame_id % hm->autoclear_interval) == 0) {
            _ui_dbg_heatmap_clear_all(win);
        }
    }
    _ui_dbg_heatmap_update(win);
    ImGui::SetNextWindowPos(ImVec2(win->ui.init_x + win->ui.init_w, win->ui.init_y + 128), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(292, 400), ImGuiCond_Once);
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
        ImGui::Checkbox("OP", &hm->show_ops); ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, 0xFFFF0000);
        ImGui::Checkbox("R", &hm->show_reads); ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, 0xFF00FF00);
        ImGui::Checkbox("W", &hm->show_writes);
        ImGui::PopStyleColor(3);
        if (ImGui::Combo("Size", &hm->tex_width_uicombo_state,
            "16 x 4096 bytes\0"
            "32 x 2048 bytes\0"
            "64 x 1024 bytes\0"
            "128 x 512 bytes\0"
            "256 x 256 bytes\0"
            "512 x 128 bytes\0\0"))
        {
            hm->next_tex_width = 1<<(hm->tex_width_uicombo_state+4);
        }
        ImGui::SliderInt("Scale", &hm->scale, 1, 8);
        ImGui::SliderInt("Auto Clear", &hm->autoclear_interval, 0, 32);
        if (ImGui::IsItemHovered() && (hm->autoclear_interval == 0)) {
            ImGui::SetTooltip("Slide to >0 to automatically\nclear every Nth frames.");
        }
        ImGui::BeginChild("##tex", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);
        ImVec2 screen_pos = ImGui::GetCursorScreenPos();
        ImVec2 mouse_pos = ImGui::GetMousePos();
        float w = (float) (hm->scale * hm->tex_width);
        float h = (float) (hm->scale * hm->tex_height);
        ImGui::Image(win->heatmap.texture, ImVec2(w, h), ImVec2(0, 0), ImVec2(1, 1));
        int x = (int)((mouse_pos.x - screen_pos.x) / hm->scale);
        int y = (int)((mouse_pos.y - screen_pos.y) / hm->scale);
        uint16_t addr = y * hm->tex_width + x;
        if (ImGui::IsItemHovered()) {
            if (_ui_dbg_heatmap_is_opcode(win, addr)) {
                _ui_dbg_disasm(win, addr);
                ImGui::SetTooltip("%04X: %s (ticks: %d)\n(right-click for options)",
                    addr, win->dasm_line.chars, hm->items[addr].ticks);
            } else {
                ImGui::SetTooltip("%04X: %02X %02X %02X %02X\n(right-click for options)", addr,
                    _ui_dbg_read_byte(win, addr),
                    _ui_dbg_read_byte(win, addr+1),
                    _ui_dbg_read_byte(win, addr+2),
                    _ui_dbg_read_byte(win, addr+3));
            }
        }
        if (ImGui::BeginPopupContextItem("##popup")) {
            if (!hm->popup_addr_valid) {
                hm->popup_addr_valid = true;
                hm->popup_addr = addr;
            }
            ImGui::Text("Address: %04X", hm->popup_addr);
            ImGui::Separator();
            if (ImGui::Selectable("Add Exec Breakpoint")) {
                if (-1 == _ui_dbg_bp_find(win, UI_DBG_BREAKTYPE_EXEC, hm->popup_addr)) {
                    _ui_dbg_bp_add_exec(win, true, hm->popup_addr);
                }
            }
            if (ImGui::Selectable("Add Byte Breakpoint")) {
                if (-1 == _ui_dbg_bp_find(win, UI_DBG_BREAKTYPE_BYTE, hm->popup_addr)) {
                    _ui_dbg_bp_add_byte(win, false, hm->popup_addr);
                    win->ui.show_breakpoints = true;
                    ImGui::SetWindowFocus("Breakpoints");
                }
            }
            if (ImGui::Selectable("Add Word Breakpoint")) {
                if (-1 == _ui_dbg_bp_find(win, UI_DBG_BREAKTYPE_WORD, hm->popup_addr)) {
                    _ui_dbg_bp_add_word(win, false, hm->popup_addr);
                    win->ui.show_breakpoints = true;
                    ImGui::SetWindowFocus("Breakpoints");
                }
            }
            ImGui::EndPopup();
        } else {
            hm->popup_addr_valid = false;
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
    ui->init_w = (float) ((desc->w == 0) ? 380 : desc->w);
    ui->init_h = (float) ((desc->h == 0) ? 440 : desc->h);
    ui->show_regs = true;
    ui->show_buttons = true;
    ui->show_bytes = true;
    ui->show_ticks = true;
    ui->show_history = false;
    ui->show_breakpoints = false;
    ui->keys = desc->keys;
    int i = 0;
    for (; i < UI_DBG_BREAKTYPE_USER; i++) {
        ui_dbg_breaktype_t* bt = &(ui->breaktypes[i]);
        switch (i) {
            case UI_DBG_BREAKTYPE_EXEC:
                bt->label = "Break at";
                bt->show_addr = true;
                break;
            case UI_DBG_BREAKTYPE_BYTE:
                bt->label = "Byte at";
                bt->show_addr = bt->show_cmp = bt->show_val8 = true;
                break;
            case UI_DBG_BREAKTYPE_WORD:
                bt->label = "Word at";
                bt->show_addr = bt->show_cmp = bt->show_val16 = true;
                break;
            case UI_DBG_BREAKTYPE_IRQ:
                bt->label = "IRQ";
                break;
            case UI_DBG_BREAKTYPE_NMI:
                bt->label = "NMI";
                break;
            #if defined(UI_DBG_USE_Z80)
            case UI_DBG_BREAKTYPE_OUT:
                bt->label = "OUT at";
                bt->show_addr = bt->show_val16 = true;
                bt->val_label = "portmask";
                break;
            case UI_DBG_BREAKTYPE_IN:
                bt->label = "IN at";
                bt->show_addr = bt->show_val16 = true;
                bt->val_label = "portmask";
                break;
            #endif
        }
        ui->breaktype_combo_labels[i] = bt->label;
    }
    int j = 0;
    for (; j < UI_DBG_MAX_USER_BREAKTYPES; j++, i++) {
        if (desc->user_breaktypes[j].label) {
            ui->breaktypes[i] = desc->user_breaktypes[j];
            ui->breaktype_combo_labels[i] = ui->breaktypes[i].label;
        } else {
            break;
        }
    }
    ui->num_breaktypes = i;
}

static void _ui_dbg_uistate_reset(ui_dbg_t* win) {
    memset(win->ui.line_array, 0, sizeof(win->ui.line_array));
}

static void _ui_dbg_uistate_reboot(ui_dbg_t* win) {
    _ui_dbg_uistate_reset(win);
}

static void _ui_dbg_draw_menu(ui_dbg_t* win) {
    bool delete_all_bp = false;
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Debug")) {
            if (ImGui::MenuItem("Break", win->ui.keys.stop.name, false, !win->dbg.stopped)) {
                _ui_dbg_break(win);
            }
            if (ImGui::MenuItem("Continue", win->ui.keys.cont.name, false, win->dbg.stopped)) {
                _ui_dbg_continue(win, true);
            }
            if (ImGui::MenuItem("Step Over", win->ui.keys.step_over.name, false, win->dbg.stopped)) {
                _ui_dbg_step_over(win);
            }
            if (ImGui::MenuItem("Step Into", win->ui.keys.step_into.name, false, win->dbg.stopped)) {
                _ui_dbg_step_into(win);
            }
            if (ImGui::MenuItem("Tick", win->ui.keys.step_tick.name, false, win->dbg.stopped)) {
                _ui_dbg_step_tick(win);
            }
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
            ImGui::MenuItem("Execution History", 0, &win->ui.show_history);
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
        const float h = 4*ImGui::GetFrameHeightWithSpacing();
        ImGui::BeginChild("##regs", ImVec2(0, h), false);
        z80_t* c = win->dbg.z80;
        ImGui::Columns(5, "##reg_columns", false);
        for (int i = 0; i < 5; i++) {
            ImGui::SetColumnWidth(i, 72);
        }
        c->af  = ui_util_input_u16("AF", c->af); ImGui::NextColumn();
        c->bc  = ui_util_input_u16("BC", c->bc); ImGui::NextColumn();
        c->de  = ui_util_input_u16("DE", c->de); ImGui::NextColumn();
        c->hl  = ui_util_input_u16("HL", c->hl); ImGui::NextColumn();
        c->wz  = ui_util_input_u16("WZ", c->wz); ImGui::NextColumn();
        c->af2 = ui_util_input_u16("AF'", c->af2); ImGui::NextColumn();
        c->bc2 = ui_util_input_u16("BC'", c->bc2); ImGui::NextColumn();
        c->de2 = ui_util_input_u16("DE'", c->de2); ImGui::NextColumn();
        c->hl2 = ui_util_input_u16("HL'", c->hl2); ImGui::NextColumn();
        c->i   = ui_util_input_u8("I", c->i); ImGui::NextColumn();
        c->ix  = ui_util_input_u16("IX", c->ix); ImGui::NextColumn();
        c->iy  = ui_util_input_u16("IY", c->iy); ImGui::NextColumn();
        c->sp  = ui_util_input_u16("SP", c->sp); ImGui::NextColumn();
        c->pc  = ui_util_input_u16("PC", c->pc); ImGui::NextColumn();
        c->r   = ui_util_input_u8("R", c->r); ImGui::NextColumn();
        c->im  = ui_util_input_u8("IM", c->im); ImGui::SameLine(); ImGui::NextColumn();
        ImGui::AlignTextToFramePadding();
        if (c->iff1) { ImGui::Text("IFF1"); }
        else         { ImGui::TextDisabled("IFF1"); }
        ImGui::NextColumn();
        ImGui::AlignTextToFramePadding();
        if (c->iff2) { ImGui::Text("IFF2"); }
        else         { ImGui::TextDisabled("IFF2"); }
        ImGui::NextColumn();
        char f_str[9] = {
            (c->f & Z80_SF) ? 'S':'-',
            (c->f & Z80_ZF) ? 'Z':'-',
            (c->f & Z80_YF) ? 'Y':'-',
            (c->f & Z80_HF) ? 'H':'-',
            (c->f & Z80_XF) ? 'X':'-',
            (c->f & Z80_VF) ? 'V':'-',
            (c->f & Z80_NF) ? 'N':'-',
            (c->f & Z80_CF) ? 'C':'-',
            0,
        };
        ImGui::AlignTextToFramePadding();
        ImGui::Text("%s", f_str);
        ImGui::EndChild();
    #elif defined(UI_DBG_USE_M6502)
        const float h = 1*ImGui::GetFrameHeightWithSpacing();
        ImGui::BeginChild("##regs", ImVec2(0, h), false);
        m6502_t* c = win->dbg.m6502;
        ImGui::Columns(7, "##reg_columns", false);
        for (int i = 0; i < 5; i++) {
            ImGui::SetColumnWidth(i, 44);
        }
        ImGui::SetColumnWidth(5, 64);
        ImGui::SetColumnWidth(6, 72);
        m6502_set_a(c, ui_util_input_u8("A", m6502_a(c))); ImGui::NextColumn();
        m6502_set_x(c, ui_util_input_u8("X", m6502_x(c))); ImGui::NextColumn();
        m6502_set_y(c, ui_util_input_u8("Y", m6502_y(c))); ImGui::NextColumn();
        m6502_set_s(c, ui_util_input_u8("S", m6502_s(c))); ImGui::NextColumn();
        m6502_set_p(c, ui_util_input_u8("P", m6502_p(c))); ImGui::NextColumn();
        m6502_set_pc(c, ui_util_input_u16("PC", m6502_pc(c))); ImGui::NextColumn();
        const uint8_t p = m6502_p(c);
        char p_str[9] = {
            (p & M6502_NF) ? 'N':'-',
            (p & M6502_VF) ? 'V':'-',
            (p & M6502_XF) ? 'X':'-',
            (p & M6502_BF) ? 'B':'-',
            (p & M6502_DF) ? 'D':'-',
            (p & M6502_IF) ? 'I':'-',
            (p & M6502_ZF) ? 'Z':'-',
            (p & M6502_CF) ? 'C':'-',
            0,
        };
        ImGui::AlignTextToFramePadding();
        ImGui::Text("%s", p_str);
        ImGui::EndChild();
    #endif
    ImGui::Separator();
}

/* handle keyboard input, the debug window must be focused for hotkeys to work! */
static void _ui_dbg_handle_input(ui_dbg_t* win) {
    /* unused hotkeys are defined as 0 and will never be triggered */
    if (win->dbg.stopped) {
        if (0 != win->ui.keys.cont.keycode) {
            if (ImGui::IsKeyPressed((ImGuiKey)win->ui.keys.cont.keycode)) {
                _ui_dbg_continue(win, true);
            }
        }
        if (0 != win->ui.keys.step_over.keycode) {
            if (ImGui::IsKeyPressed((ImGuiKey)win->ui.keys.step_over.keycode)) {
                _ui_dbg_step_over(win);
            }
        }
        if (0 != win->ui.keys.step_into.keycode) {
            if (ImGui::IsKeyPressed((ImGuiKey)win->ui.keys.step_into.keycode)) {
                _ui_dbg_step_into(win);
            }
        }
        if (0 != win->ui.keys.step_tick.keycode) {
            if (ImGui::IsKeyPressed((ImGuiKey)win->ui.keys.step_tick.keycode)) {
                _ui_dbg_step_tick(win);
            }
        }
    } else {
        if (ImGui::IsKeyPressed((ImGuiKey)win->ui.keys.stop.keycode)) {
            _ui_dbg_break(win);
        }
    }
    if (ImGui::IsKeyPressed((ImGuiKey)win->ui.keys.toggle_breakpoint.keycode)) {
        _ui_dbg_bp_toggle_exec(win, _ui_dbg_get_pc(win));
    }
}

static void _ui_dbg_draw_buttons(ui_dbg_t* win) {
    if (!win->ui.show_buttons) {
        return;
    }
    char str[32];
    if (win->dbg.stopped || (win->dbg.step_mode != UI_DBG_STEPMODE_NONE)) {
        snprintf(str, sizeof(str), "Continue (%s)", _ui_dbg_str_or_def(win->ui.keys.cont.name, "-"));
        if (ImGui::Button(str)) {
            _ui_dbg_continue(win, true);
        }
        ImGui::SameLine();
        snprintf(str, sizeof(str), "Over (%s)", _ui_dbg_str_or_def(win->ui.keys.step_over.name, "-"));
        if (ImGui::Button(str)) {
            _ui_dbg_step_over(win);
        }
        ImGui::SameLine();
        snprintf(str, sizeof(str), "Into (%s)", _ui_dbg_str_or_def(win->ui.keys.step_into.name, "-"));
        if (ImGui::Button(str)) {
            _ui_dbg_step_into(win);
        }
        ImGui::SameLine();
        snprintf(str, sizeof(str), "Tick (%s)", _ui_dbg_str_or_def(win->ui.keys.step_tick.name, "-"));
        if (ImGui::Button(str)) {
            _ui_dbg_step_tick(win);
        }
    } else {
        snprintf(str, sizeof(str), "Break (%s)", _ui_dbg_str_or_def(win->ui.keys.stop.name, "-"));
        if (ImGui::Button(str)) {
            _ui_dbg_break(win);
        }
    }
    ImGui::Separator();
}

/* helper function for backward scanning disassembly, tries to find
   a known op in the previous 4 bytes, returns true if a known op
   was found

*/
typedef struct {
    bool is_known_op;
    uint16_t addr;
} _ui_dbg_disasm_backscan_result_t;

static _ui_dbg_disasm_backscan_result_t _ui_dbg_disasm_backscan(ui_dbg_t* win, uint16_t addr) {
    bool is_known_op = false;
    uint16_t bs_addr = addr - 1;
    uint16_t scan_addr = bs_addr;
    for (int i = 0; i < 4; i++, scan_addr--) {
        if (_ui_dbg_heatmap_is_opcode(win, scan_addr)) {
            // Z80: prefixed instruction?
            #if defined(UI_DBG_USE_Z80)
                uint16_t prev_addr = scan_addr - 1;
                if (_ui_dbg_heatmap_is_opcode(win, prev_addr)) {
                    uint8_t maybe_prefix = _ui_dbg_read_byte(win, prev_addr);
                    if ((maybe_prefix == 0xCB) || (maybe_prefix == 0xDD) || (maybe_prefix == 0xED) || (maybe_prefix == 0xFD)) {
                        scan_addr = prev_addr;
                        // NOTE: we don't need a separate code path to check for double prefix FD/DD CB, since
                        // in such a case the CB opcode byte will be the regular opcode byte followed by
                        // a no-op 8-bit immediate value
                    }
                }
            #endif
            // found an op start, if any unknown bytes had been skipped, ignore the op
            // (it will be found again in the next iteration)
            _ui_dbg_disasm(win, scan_addr);
            if ((int)(win->dasm_line.num_bytes - 1) == i) {
                // ok, no gap bytes, break with 'found_op' status
                bs_addr = scan_addr;
                is_known_op = true;
            }
            break;
        }
    }
    return { is_known_op, bs_addr };
}

/* this updates the line array currently visualized by the disassembler
   listing, this only happens when the PC is outside the visible
   area or when the memory content 'under' the line array changes
*/
static void _ui_dbg_update_line_array(ui_dbg_t* win, uint16_t addr) {
    /* one half is backtraced from current PC, the other half is
       'forward tracked' from current PC
    */
    uint16_t bs_addr = addr;
    int line_idx;
    for (line_idx = 0; line_idx < UI_DBG_NUM_BACKTRACE_LINES; line_idx++) {
        // scan backwards for op start in blocks of 4 bytes (== max length of an instruction)
        bs_addr = _ui_dbg_disasm_backscan(win, bs_addr).addr;
        const int bs_index = UI_DBG_NUM_BACKTRACE_LINES - line_idx - 1;
        win->ui.line_array[bs_index].addr = bs_addr;
        win->ui.line_array[bs_index].val = _ui_dbg_read_byte(win, bs_addr);
    }
    for (; line_idx < UI_DBG_NUM_LINES; line_idx++) {
        win->ui.line_array[line_idx].addr = addr;
        addr = _ui_dbg_disasm(win, addr);
        win->ui.line_array[line_idx].val = win->dasm_line.bytes[0];
    }
}

/* check if the address is outside the line_array
    NOTE that all backtraced lines are also considered
    "outside the line array", this is for the case
    where the PC jumps back into the backtraced array,
    so that instructions after the PC are "speculatively
    disassembled" even if those addresses haven't been
    executed before.
*/
static bool _ui_dbg_addr_inside_line_array(ui_dbg_t* win, uint16_t addr) {
    uint16_t first_addr = win->ui.line_array[UI_DBG_NUM_BACKTRACE_LINES].addr;
    uint16_t last_addr = win->ui.line_array[UI_DBG_NUM_LINES-16].addr;
    if (first_addr == last_addr) {
        return false;
    } else if (first_addr < last_addr) {
        return (first_addr <= addr) && (addr <= last_addr);
    } else {
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
    const ImU32 ctrlflow_color = 0x44888888;

    /* update the line array if PC is outside or its content has become outdated */
    const uint16_t pc = _ui_dbg_get_pc(win);
    bool force_scroll = false;
    if (_ui_dbg_line_array_needs_update(win, pc)) {
        _ui_dbg_update_line_array(win, pc);
        force_scroll = true;
    }

    /* make sure the PC line is visible, but only when not stopped or stepping */
    const int safe_lines = 5;
    ImGuiListClipper clipper;
    clipper.Begin(UI_DBG_NUM_LINES, line_height);
    clipper.Step();
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
        bool show_dasm = (line_i >= UI_DBG_NUM_BACKTRACE_LINES) || _ui_dbg_heatmap_is_opcode(win, addr);
        const uint16_t start_addr = addr;
        if (show_dasm) {
            addr = _ui_dbg_disasm(win, addr);
        } else {
            addr++;
        }
        const int num_bytes = addr - start_addr;

        /* skip rendering if not in visible area */
        if (!visible_line) {
            continue;
        }

        /* show data bytes or potential but not verified instructions as dimmed */
        if (_ui_dbg_heatmap_is_opcode(win, start_addr) || (start_addr == pc)) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_Text]);
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        }

        /* column for breakpoint and step-cursor */
        ImVec2 pos = ImGui::GetCursorScreenPos();
        const float lh2 = (float)(int)(line_height/2);
        const float base_y = pos.y + line_height;
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
        } else if (ImGui::IsItemHovered()) {
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
        /* draw a separation line (for PC, breakpoint or control-flow ops) */
        if (is_pc_line) {
            dl->AddLine(ImVec2(pos.x+16,base_y), ImVec2(pos.x+2048,base_y), pc_color);
        } else if (bp_index >= 0) {
            dl->AddLine(ImVec2(pos.x+16,base_y), ImVec2(pos.x+2048,base_y), bp_disabled_color);
        } else if (show_dasm && _ui_dbg_is_controlflow_op(win->dasm_line.bytes[0], win->dasm_line.bytes[1])) {
            dl->AddLine(ImVec2(pos.x+16,base_y), ImVec2(pos.x+2048,base_y), ctrlflow_color);
        }

        /* address */
        ImGui::SameLine();
        ImGui::Text("%04X:   ", start_addr);
        ImGui::SameLine();

        /* instruction bytes (optional) */
        float x = ImGui::GetCursorPosX();
        if (win->ui.show_bytes) {
            for (int n = 0; n < num_bytes; n++) {
                ImGui::SameLine(x + cell_width*n);
                uint8_t val;
                if (show_dasm) {
                    val = win->dasm_line.bytes[n];
                } else {
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
            ImGui::Text("%s", win->dasm_line.chars);
        } else {
            ImGui::Text(" ");
        }

        /* tick count */
        x += glyph_width * (is_pc_line ? 18:20);
        if (win->ui.show_ticks) {
            int ticks = win->heatmap.items[start_addr].ticks;
            ImGui::SameLine(x);
            if (ticks > 0) {
                if (is_pc_line) {
                    ImGui::Text("%2d/%d", win->dbg.cur_op_ticks, ticks);
                } else {
                    ImGui::Text(" %d", ticks);
                }
            } else if (show_dasm) {
                if (is_pc_line) {
                    ImGui::Text("%2d/?", win->dbg.cur_op_ticks);
                } else {
                    ImGui::Text("?");
                }
            } else {
                ImGui::Text(" ");
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
    ImGui::SetNextWindowPos(ImVec2(win->ui.init_x, win->ui.init_y), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(win->ui.init_w, win->ui.init_h), ImGuiCond_Once);
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
    CHIPS_ASSERT(desc->read_cb);
    CHIPS_ASSERT(desc->texture_cbs.create_cb && desc->texture_cbs.update_cb && desc->texture_cbs.destroy_cb);
    memset(win, 0, sizeof(ui_dbg_t));
    win->valid = true;
    win->read_cb = desc->read_cb;
    win->read_layer = desc->read_layer;
    win->break_cb = desc->break_cb;
    win->texture_cbs = desc->texture_cbs;
    win->debug_cbs = desc->debug_cbs;
    win->user_data = desc->user_data;
    _ui_dbg_dbgstate_init(win, desc);
    _ui_dbg_uistate_init(win, desc);
    _ui_dbg_heatmap_init(win);
}

void ui_dbg_discard(ui_dbg_t* win) {
    CHIPS_ASSERT(win && win->valid);
    _ui_dbg_heatmap_discard(win);
    win->valid = false;
}

void ui_dbg_reset(ui_dbg_t* win) {
    CHIPS_ASSERT(win && win->valid);
    _ui_dbg_dbgstate_reset(win);
    _ui_dbg_uistate_reset(win);
    _ui_dbg_heatmap_reset(win);
    _ui_dbg_history_reset(win);
}

void ui_dbg_reboot(ui_dbg_t* win) {
    CHIPS_ASSERT(win && win->valid);
    _ui_dbg_dbgstate_reboot(win);
    _ui_dbg_uistate_reboot(win);
    _ui_dbg_heatmap_reboot(win);
    _ui_dbg_history_reboot(win);
}

void ui_dbg_tick(ui_dbg_t* win, uint64_t pins) {
    int trap_id = 0;
    if (win->dbg.step_mode == UI_DBG_STEPMODE_TICK) {
        trap_id = UI_DBG_STEP_TRAPID;
    }
    // evaluate per-op breakpoints
    #if defined(UI_DBG_USE_M6502)
        const bool new_op = pins & M6502_SYNC;
    #elif defined(UI_DBG_USE_Z80)
        const bool new_op = z80_opdone(win->dbg.z80);
    #endif
    if (new_op) {
        const uint16_t pc = pins & 0xFFFF;
        trap_id = _ui_dbg_eval_op_breakpoints(win, trap_id, pc);
        _ui_dbg_heatmap_record_op(win, pc);
        _ui_dbg_history_push(win, pc);
        win->dbg.cur_op_ticks = 0;
        win->dbg.cur_op_pc = pc;
    }
    if (win->dbg.step_mode == UI_DBG_STEPMODE_NONE) {
        trap_id = _ui_dbg_eval_tick_breakpoints(win, trap_id, pins);
    }
    _ui_dbg_heatmap_record_tick(win, pins);
    win->dbg.cur_op_ticks++;
    win->dbg.last_tick_pins = pins;

    if (trap_id >= UI_DBG_STEP_TRAPID) {
        win->dbg.stopped = true;
        if (win->debug_cbs.stopped_cb) {
            int stop_reason = (win->dbg.step_mode == UI_DBG_STEPMODE_NONE) ? UI_DBG_STOP_REASON_BREAKPOINT : UI_DBG_STOP_REASON_STEP;
            win->debug_cbs.stopped_cb(stop_reason, win->dbg.cur_op_pc);
        }
        win->dbg.step_mode = UI_DBG_STEPMODE_NONE;
        if (!win->dbg.external_debugger_connected) {
            ImGui::SetWindowFocus(win->ui.title);
            win->ui.open = true;
        }
    }
    win->dbg.last_trap_id = trap_id;
}

void ui_dbg_draw(ui_dbg_t* win) {
    CHIPS_ASSERT(win && win->valid && win->ui.title);
    win->dbg.frame_id++;
    if (!(win->ui.open || win->ui.show_heatmap || win->ui.show_breakpoints || win->ui.show_history)) {
        return;
    }
    _ui_dbg_dbgwin_draw(win);
    _ui_dbg_heatmap_draw(win);
    _ui_dbg_history_draw(win);
    _ui_dbg_bp_draw(win);
}

void ui_dbg_external_debugger_connected(ui_dbg_t* win) {
    CHIPS_ASSERT(win && win->valid);
    win->dbg.external_debugger_connected = true;
}

void ui_dbg_external_debugger_disconnected(ui_dbg_t* win) {
    CHIPS_ASSERT(win && win->valid);
    win->dbg.external_debugger_connected = false;
    // delete all breakpoints and continue execution (in case of stopped)
    _ui_dbg_bp_delete_all(win);
    _ui_dbg_continue(win, false);
}

void ui_dbg_add_breakpoint(ui_dbg_t* win, uint16_t addr) {
    CHIPS_ASSERT(win && win->valid && win->ui.title);
    int index = _ui_dbg_bp_find(win, UI_DBG_BREAKTYPE_EXEC, addr);
    if (index < 0) {
        _ui_dbg_bp_add_exec(win, true, addr);
    }
}

void ui_dbg_remove_breakpoint(ui_dbg_t* win, uint16_t addr) {
    CHIPS_ASSERT(win && win->valid && win->ui.title);
    int index = _ui_dbg_bp_find(win, UI_DBG_BREAKTYPE_EXEC, addr);
    if (index >= 0) {
        _ui_dbg_bp_del(win, index);
    }
}

void ui_dbg_break(ui_dbg_t* win) {
    CHIPS_ASSERT(win && win->valid);
    _ui_dbg_break(win);
}

void ui_dbg_continue(ui_dbg_t* win, bool invoke_continued_cb) {
    CHIPS_ASSERT(win && win->valid);
    _ui_dbg_continue(win, invoke_continued_cb);
}

bool ui_dbg_stopped(ui_dbg_t* win) {
    CHIPS_ASSERT(win && win->valid);
    return win->dbg.stopped;
}

void ui_dbg_step_next(ui_dbg_t* win) {
    CHIPS_ASSERT(win && win->valid);
    _ui_dbg_step_over(win);
}

void ui_dbg_step_into(ui_dbg_t* win) {
    CHIPS_ASSERT(win && win->valid);
    _ui_dbg_step_into(win);
}

void ui_dbg_disassemble(ui_dbg_t* win, const ui_dbg_dasm_request_t* request) {
    CHIPS_ASSERT(win && win->valid);
    CHIPS_ASSERT(request);
    CHIPS_ASSERT(request->out_lines);
    CHIPS_ASSERT(request->num_lines > 0);

    // NOTE: this code uses the same strategy as _ui_dbg_update_line_array()
    // it will first scan backward and look for known instructions
    // in the heatmap, and then scan forward
    uint16_t bs_addr = request->addr;
    int line_idx = 0;
    // optional backwards scan
    if (request->offset_lines < 0) {
        const int num_backtrace_lines = -request->offset_lines;
        for (; line_idx < num_backtrace_lines; line_idx++) {
            // scan backwards for op start in block of 4 bytes (== max length of instruction)
            const _ui_dbg_disasm_backscan_result_t res = _ui_dbg_disasm_backscan(win, bs_addr);
            bs_addr = res.addr;
            const int bs_line_idx = num_backtrace_lines - line_idx - 1;
            if (res.is_known_op) {
                _ui_dbg_disasm(win, bs_addr);
                request->out_lines[bs_line_idx] = win->dasm_line;
            } else {
                uint8_t byte = _ui_dbg_read_byte(win, bs_addr);
                ui_dbg_dasm_line_t* l = &request->out_lines[bs_line_idx];
                memset(l, 0, sizeof(ui_dbg_dasm_line_t));
                l->addr = bs_addr;
                l->num_bytes = 1;
                l->bytes[0] = byte;
                l->num_chars = 3;
                l->chars[0] = '?';
                l->chars[1] = '?';
                l->chars[2] = '?';
            }
        }
    }

    uint16_t fwd_addr = request->addr;
    // if the offset is > 0, skip disassembled instructions
    if (request->offset_lines > 0) {
        for (int i = 0; i < request->offset_lines; i++) {
            fwd_addr = _ui_dbg_disasm(win, fwd_addr);
        }
    }
    for (; line_idx < request->num_lines; line_idx++) {
        fwd_addr = _ui_dbg_disasm(win, fwd_addr);
        request->out_lines[line_idx] = win->dasm_line;
    }
}

#endif /* CHIPS_UI_IMPL */
