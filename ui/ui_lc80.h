#pragma once
/*#
    # ui_lc80.h

    UI for lc80.h.

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

    Include the following headers (and their depenencies) before including
    ui_lc80.h both for the declaration and implementation.

    - lc80.h
    - mem.h
    - ui_chip.h
    - ui_util.h
    - ui_z80.h
    - ui_z80pio.h
    - ui_z80ctc.h
    - ui_audio.h
    - ui_kbd.h
    - ui_dbg.h
    - ui_dasm.h
    - ui_memedit.h

    ## zlib/libpng license

    Copyright (c) 2019 Andre Weissflog
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

/* general callback type for rebooting to different configs */
typedef void (*ui_lc80_boot_t)(lc80_t* sys);

typedef struct {
    lc80_t* sys;
    ui_lc80_boot_t boot_cb; /* user-provided callback to reboot to different config */
    ui_dbg_create_texture_t create_texture_cb;      /* texture creation callback for ui_dbg_t */
    ui_dbg_update_texture_t update_texture_cb;      /* texture update callback for ui_dbg_t */
    ui_dbg_destroy_texture_t destroy_texture_cb;    /* texture destruction callback for ui_dbg_t */
    ui_dbg_keydesc_t dbg_keys;          /* user-defined hotkeys for ui_dbg_t */
} ui_lc80_desc_t;

typedef struct {
    ui_chip_t chip;
    struct {
        float x, y;
    } pos;
} ui_lc80_chip_t;

#define UI_LC80_MAX_PIN_CONNECTIONS (12)

typedef struct {
    void* imgui_font;
    lc80_t* sys;
    ui_lc80_boot_t boot_cb;
    struct {
        ui_z80_t cpu;
        ui_z80pio_t pio_sys;
        ui_z80pio_t pio_usr;
        ui_z80ctc_t ctc;
        ui_audio_t audio;
        ui_kbd_t kbd;
        ui_memedit_t memedit[4];
        ui_dasm_t dasm[4];
        ui_dbg_t dbg;
    } win;
    struct {
        ui_lc80_chip_t cpu;
        ui_lc80_chip_t pio_sys;
        ui_lc80_chip_t pio_usr;
        ui_lc80_chip_t ctc;
        ui_lc80_chip_t u505;
        ui_lc80_chip_t u214[2];
        ui_lc80_chip_t ds8205[2];
        ui_lc80_chip_t vqe23[3];
    } mb;
    int wire_hovered_num;
    bool wire_active;
    float wire_hovered_pos[UI_LC80_MAX_PIN_CONNECTIONS][6][2];
} ui_lc80_t;

void ui_lc80_init(ui_lc80_t* ui, const ui_lc80_desc_t* desc);
void ui_lc80_discard(ui_lc80_t* ui);
void ui_lc80_draw(ui_lc80_t* ui, double time_ms);
bool ui_lc80_before_exec(ui_lc80_t* ui);
void ui_lc80_after_exec(ui_lc80_t* ui);

#ifdef __cplusplus
} /* extern "C" */
#endif

/*-- IMPLEMENTATION (include in C++ source) ----------------------------------*/
#ifdef CHIPS_IMPL
#ifndef __cplusplus
#error "implementation must be compiled as C++"
#endif
#include <string.h> /* memset */
#include <math.h> /* maxf */
#ifndef CHIPS_ASSERT
    #include <assert.h>
    #define CHIPS_ASSERT(c) assert(c)
#endif
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-field-initializers"
#endif

struct _ui_lc80_mb_config {
    ImU32 wire_color_on = 0xFFFFFFFF;
    ImU32 wire_color_off = 0xFF777777;
    struct {
        float digit_padding = 14.0f;
        float segment_length = 24.0f;
        float segment_thickness = 8.0f;
        float dot_offset_x = 22.0f;
        float dot_offset_y = 24.0f;
        float dot_radius = 5.0f;
        float led_radius = 10.0f;
        ImU32 color_on = 0xFF00FF00;
        ImU32 color_off = 0xFF004400;
        ImU32 color_outline = 0xFF002200;
        ImU32 color_background = 0xFF002200;
    } display;
};

enum _ui_lc80_wire_type_t {
    _UI_LC80_WIRETYPE_ADDR,
    _UI_LC80_WIRETYPE_DATA,
    _UI_LC80_WIRETYPE_CTRL,
    _UI_LC80_WIRETYPE_SELECT,
    _UI_LC80_WIRETYPE_DISPLAY,
};

/* set ImGui screen cursor pos */
static void _ui_lc80_set_pos(float x, float y) {
    ImVec2 disp_size = ImGui::GetIO().DisplaySize;
    if (x < 0) {
        x = disp_size.x + x;
    }
    if (y < 0) {
        y = disp_size.y + y;
    }
    ImGui::SetCursorScreenPos(ImVec2(x, y));
}

/*
    Draw a 7+1 segment digit

     ---
    |   |
     ---
    |   |
     --- x
*/

static void _ui_lc80_draw_seg_hori(ImDrawList* l, const ImVec2& p, bool on, const _ui_lc80_mb_config& conf) {
    const float dx = conf.display.segment_length * 0.5f;
    const float dy = conf.display.segment_thickness * 0.5f;
    ImVec2 points[6] = {
        ImVec2((p.x - dx),      p.y),
        ImVec2((p.x - dx) + dy, p.y - dy),
        ImVec2((p.x + dx) - dy, p.y - dy),
        ImVec2((p.x + dx),      p.y),
        ImVec2((p.x + dx) - dy, p.y + dy),
        ImVec2((p.x - dx) + dy, p.y + dy)
    };
    l->AddConvexPolyFilled(points, 6, on ? conf.display.color_on : conf.display.color_off);
    l->AddPolyline(points, 6, conf.display.color_outline, true, 1.0f);
}

static void _ui_lc80_draw_seg_vert(ImDrawList* l, const ImVec2& p, bool on, const _ui_lc80_mb_config& conf) {
    const float dx = conf.display.segment_thickness * 0.5f;
    const float dy = conf.display.segment_length * 0.5f;
    ImVec2 points[6] = {
        ImVec2(p.x,      (p.y - dy)),
        ImVec2(p.x + dx, (p.y - dy) + dx),
        ImVec2(p.x + dx, (p.y + dy) - dx),
        ImVec2(p.x,      (p.y + dy)),
        ImVec2(p.x - dx, (p.y + dy) - dx),
        ImVec2(p.x - dx, (p.y - dy) + dx)
    };
    l->AddConvexPolyFilled(points, 6, on ? conf.display.color_on : conf.display.color_off);
    l->AddPolyline(points, 6, conf.display.color_outline, true, 1.0f);
}

static void _ui_lc80_draw_vqe23_segments(ImDrawList* l, const ImVec2& p, bool enabled, uint8_t segs, const _ui_lc80_mb_config& conf) {
    const float seg_l = conf.display.segment_length;
    const float seg_lh = seg_l * 0.5f;
    const float dot_x = conf.display.dot_offset_x;
    const float dot_y = conf.display.dot_offset_y;
    const float dot_r = conf.display.dot_radius;
    const ImU32 dot_color = (0 == (segs & 0x80)) ? conf.display.color_on : conf.display.color_off;
    _ui_lc80_draw_seg_hori(l, ImVec2(p.x, p.y-seg_l), enabled && (0==(segs&0x01)), conf);
    _ui_lc80_draw_seg_hori(l, p, enabled && (0==(segs&0x40)), conf);
    _ui_lc80_draw_seg_hori(l, ImVec2(p.x, p.y+seg_l), enabled && (0==(segs&0x08)), conf);
    _ui_lc80_draw_seg_vert(l, ImVec2(p.x - seg_lh, p.y - seg_lh), enabled && (0==(segs&0x20)), conf);
    _ui_lc80_draw_seg_vert(l, ImVec2(p.x + seg_lh, p.y - seg_lh), enabled && (0==(segs&0x02)), conf);
    _ui_lc80_draw_seg_vert(l, ImVec2(p.x - seg_lh, p.y + seg_lh), enabled && (0==(segs&0x10)), conf);
    _ui_lc80_draw_seg_vert(l, ImVec2(p.x + seg_lh, p.y + seg_lh), enabled && (0==(segs&0x04)), conf);
    l->AddCircleFilled(ImVec2(p.x + dot_x, p.y + dot_y), dot_r, dot_color);
    l->AddCircle(ImVec2(p.x + dot_x, p.y + dot_y), dot_r, conf.display.color_outline);
    l->AddCircleFilled(ImVec2(p.x - dot_x, p.y - dot_y), dot_r, conf.display.color_off);
    l->AddCircle(ImVec2(p.x - dot_x, p.y - dot_y), dot_r, conf.display.color_outline);
}

static void _ui_lc80_draw_vqe23_pins(ImDrawList* l, ui_chip_t* c, float x, float y, uint32_t pins) {
    for (int i = 0; i < c->num_slots; i++) {
        const ui_chip_pin_t* pin = &c->pins[i];
        if (!pin->name) {
            break;
        }
        ui_chip_vec2_t pos = ui_chip_pin_pos(c, i, x, y);
        float x0 = pos.x - c->pin_width * 0.5f;
        float y0 = pos.y - c->pin_height * 0.5f;
        float x1 = x0 + c->pin_width;
        float y1 = y0 + c->pin_height;
        l->AddRectFilled(ImVec2(x0, y0), ImVec2(x1, y1), (pins & pin->mask) ? 0xFFFFFFFF : 0xFF888888);
    }
}

static void _ui_lc80_draw_vqe23(ImDrawList* l, ui_chip_t* c, float x, float y, uint32_t pins, const _ui_lc80_mb_config& conf) {
    const float dx = conf.display.segment_length * 0.5f + conf.display.digit_padding;
    const ImVec2 p0(x - c->chip_width * 0.5f, y - c->chip_height * 0.5f);
    const ImVec2 p1(x + c->chip_width * 0.5f, y + c->chip_height * 0.5f);
    l->AddRectFilled(p0, p1, conf.display.color_background);
    _ui_lc80_draw_vqe23_segments(l, ImVec2(x - dx, y), true, pins>>8, conf);
    _ui_lc80_draw_vqe23_segments(l, ImVec2(x + dx, y), true, pins&0xFF, conf);
    _ui_lc80_draw_vqe23_pins(l, c, x, y, pins);
}

static ImVec2 _ui_lc80_draw_tape_led(ImDrawList* l, ImVec2 pos, bool on, const _ui_lc80_mb_config& conf) {
    l->AddCircleFilled(pos, conf.display.led_radius, on ? conf.display.color_on : conf.display.color_off);
    return ImVec2(pos.x + 2 * conf.display.led_radius, pos.y);
}

static ImVec2 _ui_lc80_draw_halt_led(ImDrawList* l, ImVec2 pos, bool on, const _ui_lc80_mb_config& conf) {
    ImU32 color_on = 0xFF0000CC;
    ImU32 color_off = 0xFF888888;
    l->AddCircleFilled(pos, conf.display.led_radius, on ? color_on : color_off);
    return ImVec2(pos.x + 2 * conf.display.led_radius, pos.y);
}

static void _ui_lc80_draw_display(ui_lc80_t* ui, const ImVec2 p, const _ui_lc80_mb_config conf) {
    CHIPS_ASSERT(ui);
    ImDrawList* l = ImGui::GetWindowDrawList();
    _ui_lc80_draw_tape_led(l, ImVec2(p.x+16.0f, p.y+24.0f), 0==(ui->sys->pio_sys.port[Z80PIO_PORT_B].output&2), conf);
    _ui_lc80_draw_halt_led(l, ImVec2(p.x+16.0f, p.y+48.0f), 0!=(ui->sys->cpu.pins & Z80_HALT), conf);
    ImVec2 disp_pos(p.x + 84.0f, p.y + 48.0f);
    _ui_lc80_draw_vqe23(l, &ui->mb.vqe23[2].chip, ui->mb.vqe23[2].pos.x, ui->mb.vqe23[2].pos.y, ui->sys->vqe23[2], conf);
    _ui_lc80_draw_vqe23(l, &ui->mb.vqe23[1].chip, ui->mb.vqe23[1].pos.x, ui->mb.vqe23[1].pos.y, ui->sys->vqe23[1], conf);
    _ui_lc80_draw_vqe23(l, &ui->mb.vqe23[0].chip, ui->mb.vqe23[0].pos.x, ui->mb.vqe23[0].pos.y, ui->sys->vqe23[0], conf);
    _ui_lc80_set_pos(p.x, p.y + 104.0f);
}

static bool _ui_lc80_btn(const char* text, ImU32 fg_color, ImU32 bg_color) {
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, 10.0f));
    ImGui::PushStyleColor(ImGuiCol_Button, bg_color);
    ImGui::PushStyleColor(ImGuiCol_Text, fg_color);
    bool res = ImGui::Button(text, ImVec2(48.0f, 32.0f));
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(2);
    return res;
}

static void _ui_lc80_draw_keyboard(ui_lc80_t* ui, const ImVec2& pos) {
    CHIPS_ASSERT(ui);
    _ui_lc80_set_pos(pos.x, pos.y);
    ImGui::BeginChild("keyboard", ImVec2(380, 330));
    ImGui::Indent(48.0f);
    const ImU32 bg_red = 0xFF0000AA;
    const ImU32 bg_black = 0xFF444444;
    const ImU32 bg_white = 0xFFBBBBBB;
    const ImU32 black = 0xFF000000;
    const ImU32 white = 0xFFFFFFFF;
    /* Row 1 */
    if (_ui_lc80_btn("RES", white, bg_red)) {
        lc80_key(ui->sys, LC80_KEY_RES);
    }
    ImGui::SameLine();
    if (_ui_lc80_btn("ADR", white, bg_black)) {
        lc80_key(ui->sys, LC80_KEY_ADR);
    }
    ImGui::SameLine();
    if (_ui_lc80_btn("DAT", white, bg_black)) {
        lc80_key(ui->sys, LC80_KEY_DAT);
    }
    ImGui::SameLine();
    if (_ui_lc80_btn("+", white, bg_black)) {
        lc80_key(ui->sys, LC80_KEY_PLUS);
    }
    ImGui::SameLine();
    if (_ui_lc80_btn("-", white, bg_black)) {
        lc80_key(ui->sys, LC80_KEY_MINUS);
    }
    /* Row 2 */
    if (_ui_lc80_btn("NMI", white, bg_red)) {
        lc80_key(ui->sys, LC80_KEY_NMI);
    }
    ImGui::SameLine();
    if (_ui_lc80_btn("C", black, bg_white)) {
        lc80_key(ui->sys, LC80_KEY_C);
    }
    ImGui::SameLine();
    if (_ui_lc80_btn("D", black, bg_white)) {
        lc80_key(ui->sys, LC80_KEY_D);
    }
    ImGui::SameLine();
    if (_ui_lc80_btn("E", black, bg_white)) {
        lc80_key(ui->sys, LC80_KEY_E);
    }
    ImGui::SameLine();
    if (_ui_lc80_btn("F", black, bg_white)) {
        lc80_key(ui->sys, LC80_KEY_F);
    }
    /* Row 3 */
    if (_ui_lc80_btn("ST", white, bg_black)) {
        lc80_key(ui->sys, LC80_KEY_ST);
    }
    ImGui::SameLine();
    if (_ui_lc80_btn("8", black, bg_white)) {
        lc80_key(ui->sys, LC80_KEY_8);
    }
    ImGui::SameLine();
    if (_ui_lc80_btn("9", black, bg_white)) {
        lc80_key(ui->sys, LC80_KEY_9);
    }
    ImGui::SameLine();
    if (_ui_lc80_btn("A", black, bg_white)) {
        lc80_key(ui->sys, LC80_KEY_A);
    }
    ImGui::SameLine();
    if (_ui_lc80_btn("B", black, bg_white)) {
        lc80_key(ui->sys, LC80_KEY_B);
    }
    /* Row 4 */
    if (_ui_lc80_btn("LD", white, bg_black)) {
        lc80_key(ui->sys, LC80_KEY_LD);
    }
    ImGui::SameLine();
    if (_ui_lc80_btn("4", black, bg_white)) {
        lc80_key(ui->sys, LC80_KEY_4);
    }
    ImGui::SameLine();
    if (_ui_lc80_btn("5", black, bg_white)) {
        lc80_key(ui->sys, LC80_KEY_5);
    }
    ImGui::SameLine();
    if (_ui_lc80_btn("6", black, bg_white)) {
        lc80_key(ui->sys, LC80_KEY_6);
    }
    ImGui::SameLine();
    if (_ui_lc80_btn("7", black, bg_white)) {
        lc80_key(ui->sys, LC80_KEY_7);
    }
    /* Row 5 */
    if (_ui_lc80_btn("EX", white, bg_black)) {
        lc80_key(ui->sys, LC80_KEY_EX);
    }
    ImGui::SameLine();
    if (_ui_lc80_btn("0", black, bg_white)) {
        lc80_key(ui->sys, LC80_KEY_0);
    }
    ImGui::SameLine();
    if (_ui_lc80_btn("1", black, bg_white)) {
        lc80_key(ui->sys, LC80_KEY_1);
    }
    ImGui::SameLine();
    if (_ui_lc80_btn("2", black, bg_white)) {
        lc80_key(ui->sys, LC80_KEY_2);
    }
    ImGui::SameLine();
    if (_ui_lc80_btn("3", black, bg_white)) {
        lc80_key(ui->sys, LC80_KEY_3);
    }
    ImGui::Unindent();
    ImGui::EndChild();
}

static ui_chip_vec2_t _ui_lc80_pin_pos(ui_lc80_chip_t* c, uint64_t pin_mask) {
    return ui_chip_pinmask_pos(&c->chip, pin_mask, c->pos.x, c->pos.y);
}

static bool _ui_lc80_pin_hovered(ui_lc80_chip_t* c, uint64_t pin_mask) {
    ui_chip_vec2_t pin_pos = _ui_lc80_pin_pos(c, pin_mask);
    const float w = fmaxf(c->chip.pin_slot_dist, c->chip.pin_width) * 0.5f;
    const float h = fmaxf(c->chip.pin_slot_dist, c->chip.pin_height) * 0.5f;
    const float x0 = pin_pos.x - w;
    const float x1 = pin_pos.x + w;
    const float y0 = pin_pos.y - h;
    const float y1 = pin_pos.y + h;
    const float mx = ImGui::GetIO().MousePos.x;
    const float my = ImGui::GetIO().MousePos.y;
    return (mx > x0) && (mx < x1) && (my > y0) && (my < y1);
}

static void _ui_lc80_draw_wire_pos(ui_lc80_t* ui,
                                   float x0, float y0,
                                   float x1, float y1,
                                   float dx0,
                                   float y,
                                   float dx1,
                                   _ui_lc80_wire_type_t wire_type,
                                   bool active,
                                   bool hovered)
{
    ImVec2 points[] = {
        { x0,       y0 },
        { x0 + dx0, y0 },
        { x0 + dx0, (y == -1) ? y0 : y },
        { x1 + dx1, (y == -1) ? y0 : y },
        { x1 + dx1, y1 },
        { x1,       y1 }
    };
    ImU32 color;
    switch (wire_type) {
        case _UI_LC80_WIRETYPE_ADDR:    color = active ? 0xFF0000FF : 0xFF000088; break;
        case _UI_LC80_WIRETYPE_DATA:    color = active ? 0xFF00FF00 : 0xFF008800; break;
        case _UI_LC80_WIRETYPE_SELECT:  color = active ? 0xFF00FFFF : 0xFF008888; break;
        case _UI_LC80_WIRETYPE_DISPLAY: color = active ? 0xFFFFFF00 : 0xFFCCCC00; break;
        default: color = active ? 0xFFFFFFFF : 0xFF888888; break;
    }
    ImDrawList* l = ImGui::GetWindowDrawList();
    ImDrawListFlags f = l->Flags;
    l->Flags = ImDrawListFlags_None;
    l->AddPolyline(points, 6, color, false, 1.0f);
    l->Flags = f;
    if (hovered) {
        CHIPS_ASSERT(ui->wire_hovered_num < UI_LC80_MAX_PIN_CONNECTIONS);
        ui->wire_active = active;
        for (int i = 0; i < 6; i++) {
            ui->wire_hovered_pos[ui->wire_hovered_num][i][0] = points[i].x;
            ui->wire_hovered_pos[ui->wire_hovered_num][i][1] = points[i].y;
        }
        ui->wire_hovered_num++;
    }
}

static void _ui_lc80_draw_wire(ui_lc80_t* ui,
                               ui_lc80_chip_t* from_chip,
                               ui_lc80_chip_t* to_chip,
                               uint64_t from_pin,
                               uint64_t to_pin,
                               float dx0,
                               float y,
                               float dx1,
                               _ui_lc80_wire_type_t wire_type,
                               bool active)
{
    ui_chip_vec2_t p0 = _ui_lc80_pin_pos(from_chip, from_pin);
    ui_chip_vec2_t p1 = _ui_lc80_pin_pos(to_chip, to_pin);
    bool hovered = _ui_lc80_pin_hovered(from_chip, from_pin) | _ui_lc80_pin_hovered(to_chip, to_pin);
    _ui_lc80_draw_wire_pos(ui, p0.x, p0.y, p1.x, p1.y, dx0, y, dx1, wire_type, active, hovered);
}

static void _ui_lc80_draw_hovered_wire(ui_lc80_t* ui) {
    ImU32 color = ui->wire_active ? 0xFFFF00FF : 0xFFCC00CC;
    ImDrawList* l = ImGui::GetWindowDrawList();
    for (int i = 0; i < ui->wire_hovered_num; i++) {
        l->AddPolyline((ImVec2*)ui->wire_hovered_pos[i], 6, color, false, 5.0f);
    }
    ui->wire_hovered_num = 0;
}

static void _ui_lc80_draw_ctrl_wires(ui_lc80_t* ui, const _ui_lc80_mb_config& c) {
    (void)c;
    auto& mb = ui->mb;
    for (int i = 0; i < 8; i++) {
        _ui_lc80_wire_type_t type = _UI_LC80_WIRETYPE_CTRL;
        uint64_t cpu_pin = Z80_M1<<i;
        bool active = 0 != (ui->sys->cpu.pins & cpu_pin);
        float x0 = -10.0f - i*4;
        float y = 300.0f + i*4;
        if (cpu_pin == Z80_M1) {
            float x1 = -20;
            _ui_lc80_draw_wire(ui, &mb.cpu, &mb.ctc,     cpu_pin, Z80CTC_M1, x0, y, x1, type, active);
            _ui_lc80_draw_wire(ui, &mb.cpu, &mb.pio_sys, cpu_pin, Z80PIO_M1, x0, y, x1, type, active);
            _ui_lc80_draw_wire(ui, &mb.cpu, &mb.pio_usr, cpu_pin, Z80PIO_M1, x0, y, x1, type, active);
        }
        else if (cpu_pin == Z80_MREQ) {
            float x1 = -30;
            _ui_lc80_draw_wire(ui, &mb.cpu, &mb.ds8205[0], cpu_pin, LC80_DS8205_G2A, x0, y, x1, type, active);
            _ui_lc80_draw_wire(ui, &mb.cpu, &mb.ds8205[1], cpu_pin, LC80_DS8205_G2A, x0, y, x1, type, active);
        }
        else if (cpu_pin == Z80_IORQ) {
            float x1 = -24;
            _ui_lc80_draw_wire(ui, &mb.cpu, &mb.ctc,     cpu_pin, Z80CTC_IORQ, x0, y, x1, type, active);
            _ui_lc80_draw_wire(ui, &mb.cpu, &mb.pio_sys, cpu_pin, Z80PIO_IORQ, x0, y, x1, type, active);
            _ui_lc80_draw_wire(ui, &mb.cpu, &mb.pio_usr, cpu_pin, Z80PIO_IORQ, x0, y, x1, type, active);
        }
        else if (cpu_pin == Z80_RD) {
            const float x1 = -28;
            _ui_lc80_draw_wire(ui, &mb.cpu, &mb.ctc,     cpu_pin, Z80CTC_RD, x0, y, x1, type, active);
            _ui_lc80_draw_wire(ui, &mb.cpu, &mb.pio_sys, cpu_pin, Z80PIO_RD, x0, y, x1, type, active);
            _ui_lc80_draw_wire(ui, &mb.cpu, &mb.pio_usr, cpu_pin, Z80PIO_RD, x0, y, x1, type, active);
        }
        else if (cpu_pin == Z80_WR) {
            const float x1 = 20;
            _ui_lc80_draw_wire(ui, &mb.cpu, &mb.u214[0], cpu_pin, LC80_U214_WR, x0, y, x1, type, active);
            _ui_lc80_draw_wire(ui, &mb.cpu, &mb.u214[1], cpu_pin, LC80_U214_WR, x0, y, x1, type, active);
        }
        else if (cpu_pin == Z80_INT) {
            const float x1 = -12;
            _ui_lc80_draw_wire(ui, &mb.cpu, &mb.ctc,     cpu_pin, Z80CTC_INT, x0, y, x1, type, active);
            _ui_lc80_draw_wire(ui, &mb.cpu, &mb.pio_sys, cpu_pin, Z80PIO_INT, x0, y, x1, type, active);
            _ui_lc80_draw_wire(ui, &mb.cpu, &mb.pio_usr, cpu_pin, Z80PIO_INT, x0, y, x1, type, active);
        }
    }
}

static void _ui_lc80_draw_data_bus(ui_lc80_t* ui, const _ui_lc80_mb_config& c) {
    (void)c;
    auto& mb = ui->mb;
    for (int i = 0; i < 8; i++) {
        _ui_lc80_wire_type_t type = _UI_LC80_WIRETYPE_DATA;
        bool active = 0 != (ui->sys->cpu.pins & (Z80_D0<<i));
        const float x0 = -70.0f + i*4;
        const float x1 = -36.0f + i*4;
        const float x2 = 10.0f + i*4;
        const float x3 = 10.0f+ (i-4)*4;
        const float y0 = 40.0f + i * 4;
        const float y1 = 660.0f - i * 4;
        _ui_lc80_draw_wire(ui, &mb.cpu, &mb.ctc,     Z80_D0<<i, Z80_D0<<i, x0, y0, x1, type, active);
        _ui_lc80_draw_wire(ui, &mb.cpu, &mb.pio_sys, Z80_D0<<i, Z80_D0<<i, x0, y0, x1, type, active);
        _ui_lc80_draw_wire(ui, &mb.cpu, &mb.pio_usr, Z80_D0<<i, Z80_D0<<i, x0, y0, x1, type, active);
        _ui_lc80_draw_wire(ui, &mb.cpu, &mb.u505,    Z80_D0<<i, LC80_U505_D0<<i, x0, y1, x2, type, active);
        if (i < 4) {
            _ui_lc80_draw_wire(ui, &mb.cpu, &mb.u214[0], Z80_D0<<i, LC80_U214_D0<<i, x0, y1, x2, type, active);
        }
        else {
            _ui_lc80_draw_wire(ui, &mb.cpu, &mb.u214[1], Z80_D0<<i, LC80_U214_D0<<(i-4), x0, y1, x3, type, active);
        }
    }
}

static void _ui_lc80_draw_addr_bus(ui_lc80_t* ui, const _ui_lc80_mb_config& c) {
    (void)c;
    auto& mb = ui->mb;
    for (int i = 0; i < 16; i++) {
        _ui_lc80_wire_type_t type = _UI_LC80_WIRETYPE_ADDR;
        bool active = 0 != (ui->sys->cpu.pins & (Z80_A0<<i));
        const float x0 = 10.0f + i*4;
        const float y = 340.0f + i*4;
        const float x1 = -48.0f + i*4;
        if (i < 11) {
            _ui_lc80_draw_wire(ui, &mb.cpu, &mb.u505, Z80_A0<<i, LC80_U505_A0<<i, x0, y, x1, type, active);
        }
        if (i < 10) {
            _ui_lc80_draw_wire(ui, &mb.cpu, &mb.u214[0], Z80_A0<<i, LC80_U214_A0<<i, x0, y, x1, type, active);
            _ui_lc80_draw_wire(ui, &mb.cpu, &mb.u214[1], Z80_A0<<i, LC80_U214_A0<<i, x0, y, x1, type, active);
        }
        if (i == 0) {
            _ui_lc80_draw_wire(ui, &mb.cpu, &mb.pio_sys, Z80_A0<<i, Z80PIO_BASEL, x0, y, -44, type, active);
            _ui_lc80_draw_wire(ui, &mb.cpu, &mb.pio_usr, Z80_A0<<i, Z80PIO_BASEL, x0, y, -44, type, active);
            _ui_lc80_draw_wire(ui, &mb.cpu, &mb.ctc,     Z80_A0<<i, Z80CTC_CS0, x0, y, -44, type, active);
        }
        if (i == 1) {
            _ui_lc80_draw_wire(ui, &mb.cpu, &mb.pio_sys, Z80_A0<<i, Z80PIO_CDSEL, x0, y, -48, type, active);
            _ui_lc80_draw_wire(ui, &mb.cpu, &mb.pio_usr, Z80_A0<<i, Z80PIO_CDSEL, x0, y, -48, type, active);
            _ui_lc80_draw_wire(ui, &mb.cpu, &mb.ctc,     Z80_A0<<i, Z80CTC_CS1, x0, y, -48, type, active);
        }
        if (i == 2) {
            _ui_lc80_draw_wire(ui, &mb.cpu, &mb.pio_usr, Z80_A0<<i, Z80PIO_CE, x0, y, -40, type, active);
        }
        if (i == 3) {
            _ui_lc80_draw_wire(ui, &mb.cpu, &mb.pio_sys, Z80_A0<<i, Z80PIO_CE, x0, y, -40, type, active);
        }
        if (i == 4) {
            _ui_lc80_draw_wire(ui, &mb.cpu, &mb.ctc, Z80_A0<<i, Z80CTC_CE, x0, y, -40, type, active);
        }
        const float ds_x1 = 30.0f - i*4;
        if (i == 10) {
            _ui_lc80_draw_wire(ui, &mb.cpu, &mb.ds8205[1], Z80_A10, LC80_DS8205_A, x0, y, ds_x1, type, active);
        }
        if (i == 11) {
            _ui_lc80_draw_wire(ui, &mb.cpu, &mb.ds8205[0], Z80_A11, LC80_DS8205_A, x0, y, ds_x1, type, active);
            _ui_lc80_draw_wire(ui, &mb.cpu, &mb.ds8205[1], Z80_A11, LC80_DS8205_B, x0, y, ds_x1, type, active);
        }
        if (i == 12) {
            _ui_lc80_draw_wire(ui, &mb.cpu, &mb.ds8205[0], Z80_A12, LC80_DS8205_B, x0, y, ds_x1, type, active);
        }
        if (i == 13) {
            _ui_lc80_draw_wire(ui, &mb.cpu, &mb.ds8205[0], Z80_A13, LC80_DS8205_G1, x0, y, ds_x1, type, active);
            _ui_lc80_draw_wire(ui, &mb.cpu, &mb.ds8205[1], Z80_A13, LC80_DS8205_G2B, x0, y, ds_x1, type, active);
        }
    }
}

static void _ui_lc80_draw_chip_select(ui_lc80_t* ui, const _ui_lc80_mb_config& c) {
    (void)c;
    auto& mb = ui->mb;
    _ui_lc80_wire_type_t type = _UI_LC80_WIRETYPE_SELECT;
    {
        bool active = 0 != (ui->sys->ds8205[0] & LC80_DS8205_Y4);
        _ui_lc80_draw_wire(ui, &mb.ds8205[0], &mb.u505, LC80_DS8205_Y4, LC80_U505_CS, 10, 450, 10, type, active);
    }
    {
        bool active = 0 != (ui->sys->ds8205[1] & LC80_DS8205_Y4);
        _ui_lc80_draw_wire(ui, &mb.ds8205[1], &mb.u214[0], LC80_DS8205_Y4, LC80_U214_CS, 14, 454, 10, type, active);
        _ui_lc80_draw_wire(ui, &mb.ds8205[1], &mb.u214[1], LC80_DS8205_Y4, LC80_U214_CS, 14, 454, 10, type, active);
    }
}

static void _ui_lc80_draw_display_wires(ui_lc80_t* ui, const _ui_lc80_mb_config& c) {
    (void)c;
    auto& mb = ui->mb;
    for (int i = 0; i < 8; i++) {
        _ui_lc80_wire_type_t type = _UI_LC80_WIRETYPE_DISPLAY;
        uint64_t pio_pin_mask = Z80PIO_PA0<<i;
        bool active = 0 != (ui->sys->pio_sys.pins & pio_pin_mask);
        float x0 = 72.0f - i*4;
        float y  = 360.0f - i*4;
        for (int j = 0; j < 3; j++) {
            _ui_lc80_draw_wire(ui, &mb.pio_sys, &mb.vqe23[j], pio_pin_mask, LC80_VQE23_A1<<i, x0, y, 0, type, active);
            _ui_lc80_draw_wire(ui, &mb.pio_sys, &mb.vqe23[j], pio_pin_mask, LC80_VQE23_A2<<i, x0, y, 0, type, active);
        }
    }
    for (int i = 0; i < 6; i++) {
        _ui_lc80_wire_type_t type = _UI_LC80_WIRETYPE_SELECT;
        uint64_t pio_pin_mask = Z80PIO_PB2<<i;
        uint64_t vqe23_pin_mask = (0 == (i & 1)) ? LC80_VQE23_K1 : LC80_VQE23_K2;
        int vqe23_index = i/2;
        bool active = 0 != (ui->sys->pio_sys.pins & pio_pin_mask);
        _ui_lc80_draw_wire(ui, &mb.pio_sys, &mb.vqe23[vqe23_index], pio_pin_mask, vqe23_pin_mask, 34.0f-i*4, 316.0f-i*4, 0, type, active);
    }
}

static void _ui_lc80_draw_led_wires(ui_lc80_t* ui, const _ui_lc80_mb_config& c) {
    (void)c;
    auto& mb = ui->mb;
    ui_chip_vec2_t p0 = _ui_lc80_pin_pos(&mb.pio_sys, Z80PIO_PB1);
    ui_chip_vec2_t p1 = { 656, 384 };
    bool active = 0 != (ui->sys->pio_sys.pins & Z80PIO_PB1);
    bool hovered = _ui_lc80_pin_hovered(&mb.pio_sys, Z80PIO_PB1);
    _ui_lc80_draw_wire_pos(ui, p0.x, p0.y, p1.x, p1.y, 38, 320, 0, _UI_LC80_WIRETYPE_SELECT, active, hovered);
}

static void _ui_lc80_draw_motherboard(ui_lc80_t* ui) {
    ImVec2 p0 = { 0, 18 };
    ImVec2 p1 = ImGui::GetIO().DisplaySize;
    p1.y -= p0.y;
    ImGui::SetNextWindowPos(p0, ImGuiCond_Always);
    ImGui::SetNextWindowSize(p1, ImGuiCond_Always);
    const ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoBringToFrontOnFocus;
    if (ImGui::Begin("LC-80", nullptr, flags)) {
        auto& mb = ui->mb;
        _ui_lc80_mb_config c;
        _ui_lc80_draw_ctrl_wires(ui, c);
        _ui_lc80_draw_data_bus(ui, c);
        _ui_lc80_draw_addr_bus(ui, c);
        _ui_lc80_draw_chip_select(ui, c);
        _ui_lc80_draw_display_wires(ui, c);
        _ui_lc80_draw_led_wires(ui, c);
        _ui_lc80_draw_hovered_wire(ui);
        _ui_lc80_draw_keyboard(ui, ImVec2(640, 460));
        _ui_lc80_draw_display(ui, ImVec2(640, 360), c);
        ui_chip_draw_at(&mb.cpu.chip, ui->sys->cpu.pins, mb.cpu.pos.x, mb.cpu.pos.y);
        ui_chip_draw_at(&mb.ctc.chip, ui->sys->ctc.pins, mb.ctc.pos.x, mb.ctc.pos.y);
        ui_chip_draw_at(&mb.pio_usr.chip, ui->sys->pio_usr.pins, mb.pio_usr.pos.x, mb.pio_usr.pos.y);
        ui_chip_draw_at(&mb.pio_sys.chip, ui->sys->pio_sys.pins, mb.pio_sys.pos.x, mb.pio_sys.pos.y);
        ui_chip_draw_at(&mb.u505.chip, ui->sys->u505, mb.u505.pos.x, mb.u505.pos.y);
        for (int i = 0; i < 2; i++) {
            ui_chip_draw_at(&mb.u214[i].chip, ui->sys->u214[i], mb.u214[i].pos.x, mb.u214[i].pos.y);
            ui_chip_draw_at(&mb.ds8205[i].chip, ui->sys->ds8205[i], mb.ds8205[i].pos.x, mb.ds8205[i].pos.y);
        }
    }
    ImGui::End();
}

static void _ui_lc80_draw_menu(ui_lc80_t* ui, double time_ms) {
    CHIPS_ASSERT(ui && ui->sys && ui->boot_cb);
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("System")) {
            if (ImGui::MenuItem("Reset")) {
                lc80_reset(ui->sys);
                ui_dbg_reset(&ui->win.dbg);
            }
            if (ImGui::MenuItem("Cold Boot")) {
                ui->boot_cb(ui->sys);
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Hardware")) {
            ImGui::MenuItem("Keyboard Matrix", 0, &ui->win.kbd.open);
            ImGui::MenuItem("Audio Output", 0, &ui->win.audio.open);
            ImGui::MenuItem("Z80 CPU", 0, &ui->win.cpu.open);
            ImGui::MenuItem("Z80 PIO (SYS)", 0, &ui->win.pio_sys.open);
            ImGui::MenuItem("Z80 PIO (USR)", 0, &ui->win.pio_usr.open);
            ImGui::MenuItem("Z80 CTC", 0, &ui->win.ctc.open);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Debug")) {
            ImGui::MenuItem("CPU Debugger", 0, &ui->win.dbg.ui.open);
            ImGui::MenuItem("Breakpoints", 0, &ui->win.dbg.ui.show_breakpoints);
            ImGui::MenuItem("Execution History", 0, &ui->win.dbg.ui.show_history);
            ImGui::MenuItem("Memory Heatmap", 0, &ui->win.dbg.ui.show_heatmap);
            if (ImGui::BeginMenu("Memory Editor")) {
                ImGui::MenuItem("Window #1", 0, &ui->win.memedit[0].open);
                ImGui::MenuItem("Window #2", 0, &ui->win.memedit[1].open);
                ImGui::MenuItem("Window #3", 0, &ui->win.memedit[2].open);
                ImGui::MenuItem("Window #4", 0, &ui->win.memedit[3].open);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Disassembler")) {
                ImGui::MenuItem("Window #1", 0, &ui->win.dasm[0].open);
                ImGui::MenuItem("Window #2", 0, &ui->win.dasm[1].open);
                ImGui::MenuItem("Window #3", 0, &ui->win.dasm[2].open);
                ImGui::MenuItem("Window #4", 0, &ui->win.dasm[3].open);
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        ui_util_options_menu(time_ms, ui->win.dbg.dbg.stopped);
        ImGui::EndMainMenuBar();
    }
}

static void _ui_lc80_draw_windows(ui_lc80_t* ui) {
    ui_audio_draw(&ui->win.audio, ui->sys->sample_pos);
    ui_kbd_draw(&ui->win.kbd);
    ui_z80_draw(&ui->win.cpu);
    ui_z80pio_draw(&ui->win.pio_sys);
    ui_z80pio_draw(&ui->win.pio_usr);
    ui_z80ctc_draw(&ui->win.ctc);
    for (int i = 0; i < 4; i++) {
        ui_memedit_draw(&ui->win.memedit[i]);
        ui_dasm_draw(&ui->win.dasm[i]);
    }
    ui_dbg_draw(&ui->win.dbg);
}

static const ui_chip_pin_t _ui_lc80_cpu_pins[] = {
    { "D0",     0,      Z80_D0 },
    { "D1",     1,      Z80_D1 },
    { "D2",     2,      Z80_D2 },
    { "D3",     3,      Z80_D3 },
    { "D4",     4,      Z80_D4 },
    { "D5",     5,      Z80_D5 },
    { "D6",     6,      Z80_D6 },
    { "D7",     7,      Z80_D7 },
    { "M1",     9,      Z80_M1 },
    { "MREQ",   10,     Z80_MREQ },
    { "IORQ",   11,     Z80_IORQ },
    { "RD",     12,     Z80_RD },
    { "WR",     13,     Z80_WR },
    { "HALT",   14,     Z80_HALT },
    { "INT",    15,     Z80_INT },
    { "NMI",    16,     Z80_NMI },
    { "WAIT",   17,     Z80_WAIT_MASK },
    { "A0",     18,     Z80_A0 },
    { "A1",     19,     Z80_A1 },
    { "A2",     20,     Z80_A2 },
    { "A3",     21,     Z80_A3 },
    { "A4",     22,     Z80_A4 },
    { "A5",     23,     Z80_A5 },
    { "A6",     24,     Z80_A6 },
    { "A7",     25,     Z80_A7 },
    { "A8",     26,     Z80_A8 },
    { "A9",     27,     Z80_A9 },
    { "A10",    28,     Z80_A10 },
    { "A11",    29,     Z80_A11 },
    { "A12",    30,     Z80_A12 },
    { "A13",    31,     Z80_A13 },
    { "A14",    32,     Z80_A14 },
    { "A15",    33,     Z80_A15 },
};

static const ui_chip_pin_t _ui_lc80_pio_pins[] = {
    { "D0",     0,      Z80_D0 },
    { "D1",     1,      Z80_D1 },
    { "D2",     2,      Z80_D2 },
    { "D3",     3,      Z80_D3 },
    { "D4",     4,      Z80_D4 },
    { "D5",     5,      Z80_D5 },
    { "D6",     6,      Z80_D6 },
    { "D7",     7,      Z80_D7 },
    { "CE",     9,      Z80PIO_CE },
    { "BASEL",  10,     Z80PIO_BASEL },
    { "CDSEL",  11,     Z80PIO_CDSEL },
    { "M1",     12,     Z80PIO_M1 },
    { "IORQ",   13,     Z80PIO_IORQ },
    { "RD",     14,     Z80PIO_RD },
    { "INT",    15,     Z80PIO_INT },
    { "ARDY",   20,     Z80PIO_ARDY },
    { "ASTB",   21,     Z80PIO_ASTB },
    { "PA0",    22,     Z80PIO_PA0 },
    { "PA1",    23,     Z80PIO_PA1 },
    { "PA2",    24,     Z80PIO_PA2 },
    { "PA3",    25,     Z80PIO_PA3 },
    { "PA4",    26,     Z80PIO_PA4 },
    { "PA5",    27,     Z80PIO_PA5 },
    { "PA6",    28,     Z80PIO_PA6 },
    { "PA7",    29,     Z80PIO_PA7 },
    { "BRDY",   30,     Z80PIO_ARDY },
    { "BSTB",   31,     Z80PIO_ASTB },
    { "PB0",    32,     Z80PIO_PB0 },
    { "PB1",    33,     Z80PIO_PB1 },
    { "PB2",    34,     Z80PIO_PB2 },
    { "PB3",    35,     Z80PIO_PB3 },
    { "PB4",    36,     Z80PIO_PB4 },
    { "PB5",    37,     Z80PIO_PB5 },
    { "PB6",    38,     Z80PIO_PB6 },
    { "PB7",    39,     Z80PIO_PB7 },
};

static const ui_chip_pin_t _ui_lc80_ctc_pins[] = {
    { "D0",     0,      Z80_D0 },
    { "D1",     1,      Z80_D1 },
    { "D2",     2,      Z80_D2 },
    { "D3",     3,      Z80_D3 },
    { "D4",     4,      Z80_D4 },
    { "D5",     5,      Z80_D5 },
    { "D6",     6,      Z80_D6 },
    { "D7",     7,      Z80_D7 },
    { "CE",     9,      Z80CTC_CE },
    { "CS0",    10,     Z80CTC_CS0 },
    { "CS1",    11,     Z80CTC_CS1 },
    { "M1",     12,     Z80CTC_M1 },
    { "IORQ",   13,     Z80CTC_IORQ },
    { "RD",     14,     Z80CTC_RD },
    { "INT",    15,     Z80CTC_INT },
    { "CT0",    16,     Z80CTC_CLKTRG0 },
    { "ZT0",    17,     Z80CTC_ZCTO0 },
    { "CT1",    19,     Z80CTC_CLKTRG1 },
    { "ZT1",    20,     Z80CTC_ZCTO1 },
    { "CT2",    22,     Z80CTC_CLKTRG2 },
    { "ZT2",    23,     Z80CTC_ZCTO2 },
    { "CT3",    25,     Z80CTC_CLKTRG3 },
};

static const ui_chip_pin_t _ui_lc80_u505_pins[] = {
    { "A0",     0,      LC80_U505_A0 },
    { "A1",     1,      LC80_U505_A1 },
    { "A2",     2,      LC80_U505_A2 },
    { "A3",     3,      LC80_U505_A3 },
    { "A4",     4,      LC80_U505_A4 },
    { "A5",     5,      LC80_U505_A5 },
    { "A6",     6,      LC80_U505_A6 },
    { "A7",     7,      LC80_U505_A7 },
    { "A8",     8,      LC80_U505_A8 },
    { "A9",     9,      LC80_U505_A9 },
    { "A10",    10,     LC80_U505_A10 },
    { "CS",     11,     LC80_U505_CS },
    { "D0",     13,     LC80_U505_D0 },
    { "D1",     14,     LC80_U505_D1 },
    { "D2",     15,     LC80_U505_D2 },
    { "D3",     16,     LC80_U505_D3 },
    { "D4",     17,     LC80_U505_D4 },
    { "D5",     18,     LC80_U505_D5 },
    { "D6",     19,     LC80_U505_D6 },
    { "D7",     20,     LC80_U505_D7 },
};

static const ui_chip_pin_t _ui_lc80_u214_pins[] = {
    { "A0",     0,      LC80_U214_A0 },
    { "A1",     1,      LC80_U214_A1 },
    { "A2",     2,      LC80_U214_A2 },
    { "A3",     3,      LC80_U214_A3 },
    { "A4",     4,      LC80_U214_A4 },
    { "A5",     5,      LC80_U214_A5 },
    { "A6",     6,      LC80_U214_A6 },
    { "A7",     7,      LC80_U214_A7 },
    { "A8",     8,      LC80_U214_A8 },
    { "A9",     9,      LC80_U214_A9 },
    { "CS",     10,     LC80_U214_CS },
    { "WR",     11,     LC80_U214_WR },
    { "D0",     13,     LC80_U214_D0 },
    { "D1",     14,     LC80_U214_D1 },
    { "D2",     15,     LC80_U214_D2 },
    { "D3",     16,     LC80_U214_D3 },
};

static const ui_chip_pin_t _ui_lc80_ds8205_pins[] = {
    { "A",      0,      LC80_DS8205_A },
    { "B",      1,      LC80_DS8205_B },
    { "C",      2,      LC80_DS8205_C },
    { "G1",     4,      LC80_DS8205_G1 },
    { "G2A",    5,      LC80_DS8205_G2A },
    { "G2B",    6,      LC80_DS8205_G2B },
    { "Y0",     8,      LC80_DS8205_Y0 },
    { "Y1",     9,      LC80_DS8205_Y1 },
    { "Y2",     10,     LC80_DS8205_Y2 },
    { "Y3",     11,     LC80_DS8205_Y3 },
    { "Y4",     12,     LC80_DS8205_Y4 },
    { "Y5",     13,     LC80_DS8205_Y5 },
    { "Y6",     14,     LC80_DS8205_Y6 },
    { "Y7",     15,     LC80_DS8205_Y7 },
};

static const ui_chip_pin_t _ui_lc80_vqe23_pins[] = {
    { "A1",    13,      LC80_VQE23_A1 },
    { "B1",    14,      LC80_VQE23_B1 },
    { "C1",    15,      LC80_VQE23_C1 },
    { "D1",    16,      LC80_VQE23_D1 },
    { "E1",    17,      LC80_VQE23_E1 },
    { "F1",    18,      LC80_VQE23_F1 },
    { "G1",    19,      LC80_VQE23_G1 },
    { "P1",    20,      LC80_VQE23_P1 },
    { "K1",    22,      LC80_VQE23_K1 },
    { "A2",     0,      LC80_VQE23_A2 },
    { "B2",     1,      LC80_VQE23_B2 },
    { "C2",     2,      LC80_VQE23_C2 },
    { "D2",     3,      LC80_VQE23_D2 },
    { "E2",     4,      LC80_VQE23_E2 },
    { "F2",     5,      LC80_VQE23_F2 },
    { "G2",     6,      LC80_VQE23_G2 },
    { "P2",     7,      LC80_VQE23_P2 },
    { "K2",     9,      LC80_VQE23_K2 }
};

static void _ui_lc80_config_chip_desc(ui_chip_desc_t* desc, int width) {
    desc->pin_slot_dist = 10;
    desc->pin_width = 8;
    desc->pin_height = 8;
    desc->chip_width = width;
    desc->pin_names_inside = true;
    desc->name_outside = true;
}

static void _ui_lc80_init_motherboard(ui_lc80_t* ui) {
    auto& mb = ui->mb;
    const int ic_width = 80;
    const int ic_width_slim = 48;

    ui_chip_desc_t cpu_desc;
    UI_CHIP_INIT_DESC(&cpu_desc, "Z80 CPU", 36, _ui_lc80_cpu_pins);
    _ui_lc80_config_chip_desc(&cpu_desc, ic_width);
    ui_chip_init(&mb.cpu.chip, &cpu_desc);
    mb.cpu.pos = { 128, 200 };

    ui_chip_desc_t ds8205_desc;
    UI_CHIP_INIT_DESC(&ds8205_desc, "DS8205", 16, _ui_lc80_ds8205_pins);
    _ui_lc80_config_chip_desc(&ds8205_desc, ic_width_slim);
    ui_chip_init(&ui->mb.ds8205[0].chip, &ds8205_desc);
    ui_chip_init(&ui->mb.ds8205[1].chip, &ds8205_desc);
    ui->mb.ds8205[0].pos = { 128, 480 };
    ui->mb.ds8205[1].pos = { 128, 580 };

    ui_chip_desc_t ctc_desc;
    UI_CHIP_INIT_DESC(&ctc_desc, "Z80 CTC", 32, _ui_lc80_ctc_pins);
    _ui_lc80_config_chip_desc(&ctc_desc, ic_width);
    ui_chip_init(&ui->mb.ctc.chip, &ctc_desc);
    ui->mb.ctc.pos = { 370, 190 };

    ui_chip_desc_t pio_desc;
    UI_CHIP_INIT_DESC(&pio_desc, "Z80 PIO (SYS)", 40, _ui_lc80_pio_pins);
    _ui_lc80_config_chip_desc(&pio_desc, ic_width);
    ui_chip_init(&mb.pio_sys.chip, &pio_desc);
    mb.pio_sys.pos = { 690, 190 };
    pio_desc.name = "Z80 PIO (USR)";
    ui_chip_init(&mb.pio_usr.chip, &pio_desc);
    mb.pio_usr.pos = { 530, 190 };

    ui_chip_desc_t u505_desc;
    UI_CHIP_INIT_DESC(&u505_desc, "U505 (ROM)", 22, _ui_lc80_u505_pins);
    _ui_lc80_config_chip_desc(&u505_desc, ic_width_slim);
    ui_chip_init(&ui->mb.u505.chip, &u505_desc);
    ui->mb.u505.pos = { 300, 530 };

    ui_chip_desc_t u214_desc;
    UI_CHIP_INIT_DESC(&u214_desc, "U214 (RAM)", 20, _ui_lc80_u214_pins);
    _ui_lc80_config_chip_desc(&u214_desc, ic_width_slim);
    ui_chip_init(&ui->mb.u214[0].chip, &u214_desc);
    ui_chip_init(&ui->mb.u214[1].chip, &u214_desc);
    ui->mb.u214[0].pos = { 420, 530 };
    ui->mb.u214[1].pos = { 560, 530 };

    _ui_lc80_mb_config mb_conf;
    ui_chip_desc_t vqe23_desc;
    UI_CHIP_INIT_DESC(&vqe23_desc, "VQE23", 23, _ui_lc80_vqe23_pins);
    vqe23_desc.num_slots = 0;
    vqe23_desc.num_slots_top = 23;
    vqe23_desc.chip_width = (int) (2 * mb_conf.display.segment_length + 4 * mb_conf.display.digit_padding);
    vqe23_desc.chip_height = (int) (2 * mb_conf.display.segment_length + 2 * mb_conf.display.digit_padding);
    vqe23_desc.pin_slot_dist = 4;
    vqe23_desc.pin_width = 3;
    vqe23_desc.pin_height = 8;
    ui_chip_init(&ui->mb.vqe23[0].chip, &vqe23_desc);
    ui_chip_init(&ui->mb.vqe23[1].chip, &vqe23_desc);
    ui_chip_init(&ui->mb.vqe23[2].chip, &vqe23_desc);
    ui->mb.vqe23[0].pos = { 940, 408 };
    ui->mb.vqe23[1].pos = { 832, 408 };
    ui->mb.vqe23[2].pos = { 724, 408 };
}

static uint8_t _ui_lc80_mem_read(int layer, uint16_t addr, void* user_data) {
    (void)layer;
    CHIPS_ASSERT(user_data);
    lc80_t* sys = (lc80_t*) user_data;
    if (addr < 0x0800) {
        return sys->rom[addr & 0x07FF];
    }
    else if ((addr >= 0x2000) && (addr < 0x2400)) {
        return sys->ram[addr & 0x3FF];
    }
    else {
        return 0xFF;
    }
}

void _ui_lc80_mem_write(int layer, uint16_t addr, uint8_t data, void* user_data) {
    (void)layer;
    CHIPS_ASSERT(user_data);
    lc80_t* sys = (lc80_t*) user_data;
    if (addr < 0x0800) {
        sys->rom[addr & 0x07FF] = data;
    }
    else if ((addr >= 0x2000) && (addr < 0x2400)) {
        sys->ram[addr & 0x3FF] = data;
    }
}

static void _ui_lc80_init_windows(ui_lc80_t* ui, const ui_lc80_desc_t* ui_desc) {
    int x = 20, y = 20, dx = 10, dy = 10;
    {
        ui_dbg_desc_t desc = {0};
        desc.title = "CPU Debugger";
        desc.x = x;
        desc.y = y;
        desc.z80 = &ui->sys->cpu;
        desc.read_cb = _ui_lc80_mem_read;
        desc.create_texture_cb = ui_desc->create_texture_cb;
        desc.update_texture_cb = ui_desc->update_texture_cb;
        desc.destroy_texture_cb = ui_desc->destroy_texture_cb;
        desc.keys = ui_desc->dbg_keys;
        desc.user_data = ui->sys;
        ui_dbg_init(&ui->win.dbg, &desc);
    }
    x += dx; y += dy;
    {
        ui_z80_desc_t desc = {0};
        desc.title = "Z80 CPU";
        desc.cpu = &ui->sys->cpu;
        desc.x = x;
        desc.y = y;
        UI_CHIP_INIT_DESC(&desc.chip_desc, "Z80\nCPU", 36, _ui_lc80_cpu_pins);
        ui_z80_init(&ui->win.cpu, &desc);
    }
    x += dx; y += dy;
    {
        ui_z80pio_desc_t desc = {0};
        desc.title = "Z80 PIO (SYS)";
        desc.pio = &ui->sys->pio_sys;
        desc.x = x;
        desc.y = y;
        UI_CHIP_INIT_DESC(&desc.chip_desc, "Z80\nPIO", 40, _ui_lc80_pio_pins);
        ui_z80pio_init(&ui->win.pio_sys, &desc);
        x += dx; y += dy;
        desc.title = "Z80 PIO (USR)";
        desc.pio = &ui->sys->pio_usr;
        ui_z80pio_init(&ui->win.pio_usr, &desc);
    }
    x += dx; y += dy;
    {
        ui_z80ctc_desc_t desc = {0};
        desc.title = "Z80 CTC";
        desc.ctc = &ui->sys->ctc;
        desc.x = x;
        desc.y = y;
        UI_CHIP_INIT_DESC(&desc.chip_desc, "Z80\nCTC", 32, _ui_lc80_ctc_pins);
        ui_z80ctc_init(&ui->win.ctc, &desc);
    }
    x += dx; y += dy;
    {
        ui_audio_desc_t desc = {0};
        desc.title = "Audio Output";
        desc.sample_buffer = ui->sys->sample_buffer;
        desc.num_samples = ui->sys->num_samples;
        desc.x = x;
        desc.y = y;
        ui_audio_init(&ui->win.audio, &desc);
    }
    x += dx; y += dy;
    {
        ui_kbd_desc_t desc = {0};
        desc.title = "Keyboard Matrix";
        desc.kbd = &ui->sys->kbd;
        desc.layers[0] = "None";
        desc.x = x;
        desc.y = y;
        ui_kbd_init(&ui->win.kbd, &desc);
    }
    x += dx; y += dy;
    {
        ui_memedit_desc_t desc = {0};
        desc.layers[0] = "System";
        desc.read_cb = _ui_lc80_mem_read;
        desc.write_cb = _ui_lc80_mem_write;
        desc.user_data = ui->sys;
        static const char* titles[] = { "Memory Editor #1", "Memory Editor #2", "Memory Editor #3", "Memory Editor #4" };
        for (int i = 0; i < 4; i++) {
            desc.title = titles[i]; desc.x = x; desc.y = y;
            ui_memedit_init(&ui->win.memedit[i], &desc);
            x += dx; y += dy;
        }
    }
    x += dx; y += dy;
    {
        ui_dasm_desc_t desc = {0};
        desc.layers[0] = "System";
        desc.cpu_type = UI_DASM_CPUTYPE_Z80;
        desc.read_cb = _ui_lc80_mem_read;
        desc.user_data = ui->sys;
        static const char* titles[4] = { "Disassembler #1", "Disassembler #2", "Disassembler #2", "Dissassembler #3" };
        for (int i = 0; i < 4; i++) {
            desc.title = titles[i]; desc.x = x; desc.y = y;
            ui_dasm_init(&ui->win.dasm[i], &desc);
            x += dx; y += dy;
        }
    }
}

static void _ui_lc80_discard_windows(ui_lc80_t* ui) {
    ui_z80_discard(&ui->win.cpu);
    ui_z80pio_discard(&ui->win.pio_sys);
    ui_z80pio_discard(&ui->win.pio_usr);
    ui_z80ctc_discard(&ui->win.ctc);
    ui_audio_discard(&ui->win.audio);
    ui_kbd_discard(&ui->win.kbd);
    for (int i = 0; i < 4; i++) {
        ui_memedit_discard(&ui->win.memedit[i]);
        ui_dasm_discard(&ui->win.dasm[i]);
    }
    ui_dbg_discard(&ui->win.dbg);
}

void ui_lc80_init(ui_lc80_t* ui, const ui_lc80_desc_t* ui_desc) {
    CHIPS_ASSERT(ui && ui_desc);
    CHIPS_ASSERT(ui_desc->sys);
    CHIPS_ASSERT(ui_desc->boot_cb);
    ui->sys = ui_desc->sys;
    ui->boot_cb = ui_desc->boot_cb;
    _ui_lc80_init_windows(ui, ui_desc);
    _ui_lc80_init_motherboard(ui);
}

void ui_lc80_discard(ui_lc80_t* ui) {
    CHIPS_ASSERT(ui && ui->sys);
    ui->sys = 0;
    _ui_lc80_discard_windows(ui);
}

void ui_lc80_draw(ui_lc80_t* ui, double time_ms) {
    CHIPS_ASSERT(ui && ui->sys);
    _ui_lc80_draw_menu(ui, time_ms);
    _ui_lc80_draw_motherboard(ui);
    _ui_lc80_draw_windows(ui);
}

bool ui_lc80_before_exec(ui_lc80_t* ui) {
    CHIPS_ASSERT(ui && ui->sys);
    return ui_dbg_before_exec(&ui->win.dbg);
}

void ui_lc80_after_exec(ui_lc80_t* ui) {
    CHIPS_ASSERT(ui && ui->sys);
    ui_dbg_after_exec(&ui->win.dbg);
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif
#endif /* CHIPS_IMPL */
