#pragma once
/*#
    # ui_memedit.h

    Memory viewer/editor UI using Dear ImGui.

    Do this:
    ~~~C
    #define CHIPS_UI_IMPL
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

    All strings provided to ui_memedit_init() must remain alive until
    ui_memedit_discard() is called!

    Includes a (slightly extended) version of imgui_memory_editor.h:

    https://github.com/ocornut/imgui_club/blob/master/imgui_memory_editor/imgui_memory_editor.h

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

#ifdef CHIPS_UI_IMPL
struct MemoryEditor;
#else
typedef void MemoryEditor;
#endif

#define UI_MEMEDIT_MAX_LAYERS (16)

/* callbacks for reading and writing bytes */
typedef uint8_t (*ui_memedit_read_t)(int layer, uint16_t addr, void* user_data);
typedef void (*ui_memedit_write_t)(int layer, uint16_t addr, uint8_t data, void* user_data);

/* setup parameters for ui_memedit_init()

    NOTE: all strings must be static!
*/
typedef struct {
    const char* title;  /* window title */
    const char* layers[UI_MEMEDIT_MAX_LAYERS];   /* memory system layer names */
    ui_memedit_read_t read_cb;
    ui_memedit_write_t write_cb;
    size_t max_addr;
    int num_cols;       /* initial number of cols, default is 16 */
    bool hide_ascii;    /* initially hide the ASCII column */
    bool hide_options;  /* hide the Options dropdown */
    bool hide_addr_input;   /* hide the address input field */
    void* user_data;
    int x, y;           /* initial window pos */
    int w, h;           /* initial window size, or 0 for default size */
    bool open;          /* initial open state */
} ui_memedit_desc_t;

typedef struct {
    const char* title;
    ui_memedit_read_t read_cb;
    ui_memedit_write_t write_cb;
    void* user_data;
    float init_x, init_y;
    float init_w, init_h;
    size_t max_addr;
    MemoryEditor* ed;
    bool open;
    bool valid;
} ui_memedit_t;

/* NOTE: win MUST be zero-initialized already, allocates memory via new() */
void ui_memedit_init(ui_memedit_t* win, const ui_memedit_desc_t* desc);
/* frees memory via delete() */
void ui_memedit_discard(ui_memedit_t* win);
void ui_memedit_draw_content(ui_memedit_t* win);
void ui_memedit_draw(ui_memedit_t* win);

#ifdef __cplusplus
} /* extern "C" */
#endif

/*-- IMPLEMENTATION (include in C++ source) ----------------------------------*/
#ifdef CHIPS_UI_IMPL
#ifndef __cplusplus
#error "implementation must be compiled as C++"
#endif
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4996)   /* sscanf */
#endif
#include <string.h> /* memset */
#include <stdio.h>  /* sscanf, sprintf (ImGui memory editor) */
#ifndef CHIPS_ASSERT
    #include <assert.h>
    #define CHIPS_ASSERT(c) assert(c)
#endif

/*== imgui_memory_editor.h (with additional layer dropdown) ==================*/

// Mini memory editor for Dear ImGui (to embed in your game/tools)
// Animated GIF: https://twitter.com/ocornut/status/894242704317530112
// Get latest version at http://www.github.com/ocornut/imgui_club
//
// You can adjust the keyboard repeat delay/rate in ImGuiIO.
// The code assume a mono-space font for simplicity! If you don't use the default font, use ImGui::PushFont()/PopFont() to switch to a mono-space font before caling this.
//
// Usage:
//   static MemoryEditor mem_edit_1;                                            // store your state somewhere
//   mem_edit_1.DrawWindow("Memory Editor", mem_block, mem_block_size, 0x0000); // create a window and draw memory editor (if you already have a window, use DrawContents())
//
// Usage:
//   static MemoryEditor mem_edit_2;
//   ImGui::Begin("MyWindow")
//   mem_edit_2.DrawContents(this, sizeof(*this), (size_t)this);
//   ImGui::End();
//
// Changelog:
// - v0.10: initial version
// - v0.11: always refresh active text input with the latest byte from source memory if it's not being edited.
// - v0.12: added OptMidRowsCount to allow extra spacing every XX rows.
// - v0.13: added optional ReadFn/WriteFn handlers to access memory via a function. various warning fixes for 64-bits.
// - v0.14: added GotoAddr member, added GotoAddrAndHighlight() and highlighting. fixed minor scrollbar glitch when resizing.
// - v0.15: added maximum window width. minor optimization.
// - v0.16: added OptGreyOutZeroes option. various sizing fixes when resizing using the "Rows" drag.
// - v0.17: added HighlightFn handler for optional non-contiguous highlighting.
// - v0.18: fixes for displaying 64-bits addresses, fixed mouse click gaps introduced in recent changes, cursor tracking scrolling fixes.
// - v0.19: fixed auto-focus of next byte leaving WantCaptureKeyboard=false for one frame. we now capture the keyboard during that transition.
// - v0.20: added options menu. added OptShowAscii checkbox. added optional HexII display. split Draw() in DrawWindow()/DrawContents(). fixing glyph width. refactoring/cleaning code.
// - v0.21: fixes for using DrawContents() in our own window. fixed HexII to actually be useful and not on the wrong side.
// - v0.22: clicking Ascii view select the byte in the Hex view. Ascii view highlight selection.
// - v0.23: fixed right-arrow triggering a byte write
//
// Todo/Bugs:
// - Arrows are being sent to the InputText() about to disappear which for LeftArrow makes the text cursor appear at position 1 for one frame.

#ifdef _MSC_VER
#define _PRISizeT   "I"
#define ImSnprintf  _snprintf
#else
#define _PRISizeT   "z"
#define ImSnprintf  snprintf
#endif

struct MemoryEditor
{
    enum DataFormat
    {
        DataFormat_Bin = 0,
        DataFormat_Dec = 1,
        DataFormat_Hex = 2,
        DataFormat_COUNT
    };

    /*--- BEGIN ui_memedit.h changes ---*/
    int NumLayers;
    int CurLayer;
    const char* Layers[UI_MEMEDIT_MAX_LAYERS];
    bool OptShowAddrInput;      // = true
    /*--- END ui_memedit.h changes ---*/

    // Settings
    bool            Open;                                       // = true   // set to false when DrawWindow() was closed. ignore if not using DrawWindow().
    bool            ReadOnly;                                   // = false  // disable any editing.
    int             Cols;                                       // = 16     // number of columns to display.
    bool            OptShowOptions;                             // = true   // display options button/context menu. when disabled, options will be locked unless you provide your own UI for them.
    bool            OptShowDataPreview;                         // = false  // display a footer previewing the decimal/binary/hex/float representation of the currently selected bytes.
    bool            OptShowHexII;                               // = false  // display values in HexII representation instead of regular hexadecimal: hide null/zero bytes, ascii values as ".X".
    bool            OptShowAscii;                               // = true   // display ASCII representation on the right side.
    bool            OptGreyOutZeroes;                           // = true   // display null/zero bytes using the TextDisabled color.
    bool            OptUpperCaseHex;                            // = true   // display hexadecimal values as "FF" instead of "ff".
    int             OptMidColsCount;                            // = 8      // set to 0 to disable extra spacing between every mid-cols.
    int             OptAddrDigitsCount;                         // = 0      // number of addr digits to display (default calculated based on maximum displayed addr).
    float           OptFooterExtraHeight;                       // = 0      // space to reserve at the bottom of the widget to add custom widgets
    ImU32           HighlightColor;                             //          // background color of highlighted bytes.
    ImU8            (*ReadFn)(const ImU8* data, size_t off);    // = 0      // optional handler to read bytes.
    void            (*WriteFn)(ImU8* data, size_t off, ImU8 d); // = 0      // optional handler to write bytes.
    bool            (*HighlightFn)(const ImU8* data, size_t off);//= 0      // optional handler to return Highlight property (to support non-contiguous highlighting).

    // [Internal State]
    bool            ContentsWidthChanged;
    size_t          DataPreviewAddr;
    size_t          DataEditingAddr;
    bool            DataEditingTakeFocus;
    char            DataInputBuf[32];
    char            AddrInputBuf[32];
    size_t          GotoAddr;
    size_t          HighlightMin, HighlightMax;
    int             PreviewEndianess;
    ImGuiDataType   PreviewDataType;

    MemoryEditor()
    {
        // Settings
        Open = true;
        ReadOnly = false;
        Cols = 16;
        OptShowOptions = true;
        OptShowDataPreview = false;
        OptShowHexII = false;
        OptShowAscii = true;
        OptGreyOutZeroes = true;
        OptUpperCaseHex = true;
        OptMidColsCount = 8;
        OptAddrDigitsCount = 0;
        OptFooterExtraHeight = 0.0f;
        HighlightColor = IM_COL32(255, 255, 255, 50);
        ReadFn = NULL;
        WriteFn = NULL;
        HighlightFn = NULL;

        // State/Internals
        ContentsWidthChanged = false;
        DataPreviewAddr = DataEditingAddr = (size_t)-1;
        DataEditingTakeFocus = false;
        memset(DataInputBuf, 0, sizeof(DataInputBuf));
        memset(AddrInputBuf, 0, sizeof(AddrInputBuf));
        GotoAddr = (size_t)-1;
        HighlightMin = HighlightMax = (size_t)-1;
        PreviewEndianess = 0;
        PreviewDataType = ImGuiDataType_S32;

        /*--- BEGIN ui_memedit.h changes ---*/
        OptShowAddrInput = true;
        NumLayers = 0;
        CurLayer = 0;
        for (int i = 0; i < UI_MEMEDIT_MAX_LAYERS; i++) {
            Layers[i] = nullptr;
        }
        /*--- END ui_memedit.h changes ---*/
    }

    void GotoAddrAndHighlight(size_t addr_min, size_t addr_max)
    {
        GotoAddr = addr_min;
        HighlightMin = addr_min;
        HighlightMax = addr_max;
    }

    struct Sizes
    {
        int     AddrDigitsCount;
        float   LineHeight;
        float   GlyphWidth;
        float   HexCellWidth;
        float   SpacingBetweenMidCols;
        float   PosHexStart;
        float   PosHexEnd;
        float   PosAsciiStart;
        float   PosAsciiEnd;
        float   WindowWidth;

        Sizes() { memset(this, 0, sizeof(*this)); }
    };

    void CalcSizes(Sizes& s, size_t mem_size, size_t base_display_addr)
    {
        ImGuiStyle& style = ImGui::GetStyle();
        s.AddrDigitsCount = OptAddrDigitsCount;
        if (s.AddrDigitsCount == 0)
            for (size_t n = base_display_addr + mem_size - 1; n > 0; n >>= 4)
                s.AddrDigitsCount++;
        s.LineHeight = ImGui::GetTextLineHeight();
        s.GlyphWidth = ImGui::CalcTextSize("F").x + 1;                  // We assume the font is mono-space
        s.HexCellWidth = (float)(int)(s.GlyphWidth * 2.5f);             // "FF " we include trailing space in the width to easily catch clicks everywhere
        s.SpacingBetweenMidCols = (float)(int)(s.HexCellWidth * 0.25f); // Every OptMidColsCount columns we add a bit of extra spacing
        s.PosHexStart = (s.AddrDigitsCount + 2) * s.GlyphWidth;
        s.PosHexEnd = s.PosHexStart + (s.HexCellWidth * Cols);
        s.PosAsciiStart = s.PosAsciiEnd = s.PosHexEnd;
        if (OptShowAscii)
        {
            s.PosAsciiStart = s.PosHexEnd + s.GlyphWidth * 1;
            if (OptMidColsCount > 0)
                s.PosAsciiStart += (float)((Cols + OptMidColsCount - 1) / OptMidColsCount) * s.SpacingBetweenMidCols;
            s.PosAsciiEnd = s.PosAsciiStart + Cols * s.GlyphWidth;
        }
        s.WindowWidth = s.PosAsciiEnd + style.ScrollbarSize + style.WindowPadding.x * 2 + s.GlyphWidth;
    }

    // Standalone Memory Editor window
    void DrawWindow(const char* title, void* mem_data, size_t mem_size, size_t base_display_addr = 0x0000)
    {
        Sizes s;
        CalcSizes(s, mem_size, base_display_addr);
        ImGui::SetNextWindowSize(ImVec2(s.WindowWidth, s.WindowWidth * 0.60f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSizeConstraints(ImVec2(0.0f, 0.0f), ImVec2(s.WindowWidth, FLT_MAX));

        Open = true;
        if (ImGui::Begin(title, &Open, ImGuiWindowFlags_NoScrollbar))
        {
            if (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows) && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
                ImGui::OpenPopup("context");
            DrawContents(mem_data, mem_size, base_display_addr);
            if (ContentsWidthChanged)
            {
                CalcSizes(s, mem_size, base_display_addr);
                ImGui::SetWindowSize(ImVec2(s.WindowWidth, ImGui::GetWindowSize().y));
            }
        }
        ImGui::End();
    }

    // Memory Editor contents only
    void DrawContents(void* mem_data_void, size_t mem_size, size_t base_display_addr = 0x0000)
    {
        if (Cols < 1)
            Cols = 1;

        ImU8* mem_data = (ImU8*)mem_data_void;
        Sizes s;
        CalcSizes(s, mem_size, base_display_addr);
        ImGuiStyle& style = ImGui::GetStyle();

        // We begin into our scrolling region with the 'ImGuiWindowFlags_NoMove' in order to prevent click from moving the window.
        // This is used as a facility since our main click detection code doesn't assign an ActiveId so the click would normally be caught as a window-move.
        const float height_separator = style.ItemSpacing.y;
        float footer_height = OptFooterExtraHeight;
        if (OptShowOptions)
            footer_height += height_separator + ImGui::GetFrameHeightWithSpacing() * 1;
        if (OptShowDataPreview)
            footer_height += height_separator + ImGui::GetFrameHeightWithSpacing() * 1 + ImGui::GetTextLineHeightWithSpacing() * 3;
        ImGui::BeginChild("##scrolling", ImVec2(0, -footer_height), false, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNav);
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

        // We are not really using the clipper API correctly here, because we rely on visible_start_addr/visible_end_addr for our scrolling function.
        const int line_total_count = (int)((mem_size + Cols - 1) / Cols);
        ImGuiListClipper clipper;
        clipper.Begin(line_total_count, s.LineHeight);

        bool data_next = false;

        if (ReadOnly || DataEditingAddr >= mem_size)
            DataEditingAddr = (size_t)-1;
        if (DataPreviewAddr >= mem_size)
            DataPreviewAddr = (size_t)-1;

        size_t preview_data_type_size = OptShowDataPreview ? DataTypeGetSize(PreviewDataType) : 0;

        size_t data_editing_addr_next = (size_t)-1;
        if (DataEditingAddr != (size_t)-1)
        {
            // Move cursor but only apply on next frame so scrolling with be synchronized (because currently we can't change the scrolling while the window is being rendered)
            if (ImGui::IsKeyPressed(ImGuiKey_UpArrow) && (ptrdiff_t)DataEditingAddr >= (ptrdiff_t)Cols)                     { data_editing_addr_next = DataEditingAddr - Cols; }
            else if (ImGui::IsKeyPressed(ImGuiKey_DownArrow) && (ptrdiff_t)DataEditingAddr < (ptrdiff_t)mem_size - Cols)    { data_editing_addr_next = DataEditingAddr + Cols; }
            else if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow) && (ptrdiff_t)DataEditingAddr > (ptrdiff_t)0)                  { data_editing_addr_next = DataEditingAddr - 1; }
            else if (ImGui::IsKeyPressed(ImGuiKey_RightArrow) && (ptrdiff_t)DataEditingAddr < (ptrdiff_t)mem_size - 1)      { data_editing_addr_next = DataEditingAddr + 1; }
        }

        // Draw vertical separator
        ImVec2 window_pos = ImGui::GetWindowPos();
        if (OptShowAscii)
            draw_list->AddLine(ImVec2(window_pos.x + s.PosAsciiStart - s.GlyphWidth, window_pos.y), ImVec2(window_pos.x + s.PosAsciiStart - s.GlyphWidth, window_pos.y + 9999), ImGui::GetColorU32(ImGuiCol_Border));

        const ImU32 color_text = ImGui::GetColorU32(ImGuiCol_Text);
        const ImU32 color_disabled = OptGreyOutZeroes ? ImGui::GetColorU32(ImGuiCol_TextDisabled) : color_text;

        const char* format_address = OptUpperCaseHex ? "%0*" _PRISizeT "X: " : "%0*" _PRISizeT "x: ";
        const char* format_data = OptUpperCaseHex ? "%0*" _PRISizeT "X" : "%0*" _PRISizeT "x";
        const char* format_byte = OptUpperCaseHex ? "%02X" : "%02x";
        const char* format_byte_space = OptUpperCaseHex ? "%02X " : "%02x ";

        while (clipper.Step())
            for (int line_i = clipper.DisplayStart; line_i < clipper.DisplayEnd; line_i++) // display only visible lines
            {
                size_t addr = (size_t)(line_i * Cols);
                ImGui::Text(format_address, s.AddrDigitsCount, base_display_addr + addr);

                // Draw Hexadecimal
                for (int n = 0; n < Cols && addr < mem_size; n++, addr++)
                {
                    float byte_pos_x = s.PosHexStart + s.HexCellWidth * n;
                    if (OptMidColsCount > 0)
                        byte_pos_x += (float)(n / OptMidColsCount) * s.SpacingBetweenMidCols;
                    ImGui::SameLine(byte_pos_x);

                    // Draw highlight
                    bool is_highlight_from_user_range = (addr >= HighlightMin && addr < HighlightMax);
                    bool is_highlight_from_user_func = (HighlightFn && HighlightFn(mem_data, addr));
                    bool is_highlight_from_preview = (addr >= DataPreviewAddr && addr < DataPreviewAddr + preview_data_type_size);
                    if (is_highlight_from_user_range || is_highlight_from_user_func || is_highlight_from_preview)
                    {
                        ImVec2 pos = ImGui::GetCursorScreenPos();
                        float highlight_width = s.GlyphWidth * 2;
                        bool is_next_byte_highlighted = (addr + 1 < mem_size) && ((HighlightMax != (size_t)-1 && addr + 1 < HighlightMax) || (HighlightFn && HighlightFn(mem_data, addr + 1)));
                        if (is_next_byte_highlighted || (n + 1 == Cols))
                        {
                            highlight_width = s.HexCellWidth;
                            if (OptMidColsCount > 0 && n > 0 && (n + 1) < Cols && ((n + 1) % OptMidColsCount) == 0)
                                highlight_width += s.SpacingBetweenMidCols;
                        }
                        draw_list->AddRectFilled(pos, ImVec2(pos.x + highlight_width, pos.y + s.LineHeight), HighlightColor);
                    }

                    if (DataEditingAddr == addr)
                    {
                        // Display text input on current byte
                        bool data_write = false;
                        ImGui::PushID((void*)addr);
                        if (DataEditingTakeFocus)
                        {
                            ImGui::SetKeyboardFocusHere(0);
                            ImSnprintf(AddrInputBuf, sizeof(AddrInputBuf), format_data, s.AddrDigitsCount, base_display_addr + addr);
                            ImSnprintf(DataInputBuf, sizeof(DataInputBuf), format_byte, ReadFn ? ReadFn(mem_data, addr) : mem_data[addr]);
                        }
                        struct UserData
                        {
                            // FIXME: We should have a way to retrieve the text edit cursor position more easily in the API, this is rather tedious. This is such a ugly mess we may be better off not using InputText() at all here.
                            static int Callback(ImGuiInputTextCallbackData* data)
                            {
                                UserData* user_data = (UserData*)data->UserData;
                                if (!data->HasSelection())
                                    user_data->CursorPos = data->CursorPos;
                                if (data->SelectionStart == 0 && data->SelectionEnd == data->BufTextLen)
                                {
                                    // When not editing a byte, always refresh its InputText content pulled from underlying memory data
                                    // (this is a bit tricky, since InputText technically "owns" the master copy of the buffer we edit it in there)
                                    data->DeleteChars(0, data->BufTextLen);
                                    data->InsertChars(0, user_data->CurrentBufOverwrite);
                                    data->SelectionStart = 0;
                                    data->SelectionEnd = 2;
                                    data->CursorPos = 0;
                                }
                                return 0;
                            }
                            char   CurrentBufOverwrite[3];  // Input
                            int    CursorPos;               // Output
                        };
                        UserData user_data;
                        user_data.CursorPos = -1;
                        ImSnprintf(user_data.CurrentBufOverwrite, sizeof(user_data.CurrentBufOverwrite), format_byte, ReadFn ? ReadFn(mem_data, addr) : mem_data[addr]);
                        ImGuiInputTextFlags flags = ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_NoHorizontalScroll | ImGuiInputTextFlags_CallbackAlways;
#if IMGUI_VERSION_NUM >= 18104
                        flags |= ImGuiInputTextFlags_AlwaysOverwrite;
#else
                        flags |= ImGuiInputTextFlags_AlwaysInsertMode;
#endif
                        ImGui::SetNextItemWidth(s.GlyphWidth * 2);
                        if (ImGui::InputText("##data", DataInputBuf, IM_ARRAYSIZE(DataInputBuf), flags, UserData::Callback, &user_data))
                            data_write = data_next = true;
                        else if (!DataEditingTakeFocus && !ImGui::IsItemActive())
                            DataEditingAddr = data_editing_addr_next = (size_t)-1;
                        DataEditingTakeFocus = false;
                        if (user_data.CursorPos >= 2)
                            data_write = data_next = true;
                        if (data_editing_addr_next != (size_t)-1)
                            data_write = data_next = false;
                        unsigned int data_input_value = 0;
                        if (data_write && sscanf(DataInputBuf, "%X", &data_input_value) == 1)
                        {
                            if (WriteFn)
                                WriteFn(mem_data, addr, (ImU8)data_input_value);
                            else
                                mem_data[addr] = (ImU8)data_input_value;
                        }
                        ImGui::PopID();
                    }
                    else
                    {
                        // NB: The trailing space is not visible but ensure there's no gap that the mouse cannot click on.
                        ImU8 b = ReadFn ? ReadFn(mem_data, addr) : mem_data[addr];

                        if (OptShowHexII)
                        {
                            if ((b >= 32 && b < 128))
                                ImGui::Text(".%c ", b);
                            else if (b == 0xFF && OptGreyOutZeroes)
                                ImGui::TextDisabled("## ");
                            else if (b == 0x00)
                                ImGui::Text("   ");
                            else
                                ImGui::Text(format_byte_space, b);
                        }
                        else
                        {
                            if (b == 0 && OptGreyOutZeroes)
                                ImGui::TextDisabled("00 ");
                            else
                                ImGui::Text(format_byte_space, b);
                        }
                        if (!ReadOnly && ImGui::IsItemHovered() && ImGui::IsMouseClicked(0))
                        {
                            DataEditingTakeFocus = true;
                            data_editing_addr_next = addr;
                        }
                    }
                }

                if (OptShowAscii)
                {
                    // Draw ASCII values
                    ImGui::SameLine(s.PosAsciiStart);
                    ImVec2 pos = ImGui::GetCursorScreenPos();
                    addr = line_i * Cols;
                    ImGui::PushID(line_i);
                    if (ImGui::InvisibleButton("ascii", ImVec2(s.PosAsciiEnd - s.PosAsciiStart, s.LineHeight)))
                    {
                        DataEditingAddr = DataPreviewAddr = addr + (size_t)((ImGui::GetIO().MousePos.x - pos.x) / s.GlyphWidth);
                        DataEditingTakeFocus = true;
                    }
                    ImGui::PopID();
                    for (int n = 0; n < Cols && addr < mem_size; n++, addr++)
                    {
                        if (addr == DataEditingAddr)
                        {
                            draw_list->AddRectFilled(pos, ImVec2(pos.x + s.GlyphWidth, pos.y + s.LineHeight), ImGui::GetColorU32(ImGuiCol_FrameBg));
                            draw_list->AddRectFilled(pos, ImVec2(pos.x + s.GlyphWidth, pos.y + s.LineHeight), ImGui::GetColorU32(ImGuiCol_TextSelectedBg));
                        }
                        unsigned char c = ReadFn ? ReadFn(mem_data, addr) : mem_data[addr];
                        char display_c = (c < 32 || c >= 128) ? '.' : c;
                        draw_list->AddText(pos, (display_c == c) ? color_text : color_disabled, &display_c, &display_c + 1);
                        pos.x += s.GlyphWidth;
                    }
                }
            }
        ImGui::PopStyleVar(2);
        ImGui::EndChild();

        // Notify the main window of our ideal child content size (FIXME: we are missing an API to get the contents size from the child)
        ImGui::SetCursorPosX(s.WindowWidth);

        if (data_next && DataEditingAddr + 1 < mem_size)
        {
            DataEditingAddr = DataPreviewAddr = DataEditingAddr + 1;
            DataEditingTakeFocus = true;
        }
        else if (data_editing_addr_next != (size_t)-1)
        {
            DataEditingAddr = DataPreviewAddr = data_editing_addr_next;
            DataEditingTakeFocus = true;
        }

        const bool lock_show_data_preview = OptShowDataPreview;
        if (OptShowOptions)
        {
            ImGui::Separator();
            DrawOptionsLine(s, mem_data, mem_size, base_display_addr);
        }

        if (lock_show_data_preview)
        {
            ImGui::Separator();
            DrawPreviewLine(s, mem_data, mem_size, base_display_addr);
        }
    }

    void DrawOptionsLine(const Sizes& s, void* mem_data, size_t mem_size, size_t base_display_addr)
    {
        IM_UNUSED(mem_data);
        ImGuiStyle& style = ImGui::GetStyle();
        const char* format_range = OptUpperCaseHex ? "Range %0*" _PRISizeT "X..%0*" _PRISizeT "X" : "Range %0*" _PRISizeT "x..%0*" _PRISizeT "x";

        // Options menu
        if (ImGui::Button("Options"))
            ImGui::OpenPopup("context");
        if (ImGui::BeginPopup("context"))
        {
            ImGui::SetNextItemWidth(s.GlyphWidth * 7 + style.FramePadding.x * 2.0f);
            if (ImGui::DragInt("##cols", &Cols, 0.2f, 4, 32, "%d cols")) { ContentsWidthChanged = true; if (Cols < 1) Cols = 1; }
            ImGui::Checkbox("Show Data Preview", &OptShowDataPreview);
            ImGui::Checkbox("Show HexII", &OptShowHexII);
            if (ImGui::Checkbox("Show Ascii", &OptShowAscii)) { ContentsWidthChanged = true; }
            ImGui::Checkbox("Grey out zeroes", &OptGreyOutZeroes);
            ImGui::Checkbox("Uppercase Hex", &OptUpperCaseHex);

            ImGui::EndPopup();
        }

        /*--- BEGIN ui_memedit.h changes ---*/
        if (OptShowAddrInput) {
        /*--- END ui_memedit.h changes ---*/
            ImGui::SameLine();
            ImGui::Text(format_range, s.AddrDigitsCount, base_display_addr, s.AddrDigitsCount, base_display_addr + mem_size - 1);
            ImGui::SameLine();
            ImGui::SetNextItemWidth((s.AddrDigitsCount + 1) * s.GlyphWidth + style.FramePadding.x * 2.0f);
            if (ImGui::InputText("##addr", AddrInputBuf, IM_ARRAYSIZE(AddrInputBuf), ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_EnterReturnsTrue))
            {
                size_t goto_addr;
                if (sscanf(AddrInputBuf, "%" _PRISizeT "X", &goto_addr) == 1)
                {
                    GotoAddr = goto_addr - base_display_addr;
                    HighlightMin = HighlightMax = (size_t)-1;
                }
            }
        /*--- BEGIN ui_memedit.h changes ---*/
        }
        /*--- END ui_memedit.h changes ---*/

        /*--- BEGIN ui_memedit.h changes */
        if (NumLayers > 1) {
            ImGui::SameLine();
            ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
            ImGui::Combo("##layer", &CurLayer, Layers, NumLayers);
            ImGui::PopItemWidth();
        }
        /*--- END ui_memedit.h changes */

        if (GotoAddr != (size_t)-1)
        {
            if (GotoAddr < mem_size)
            {
                ImGui::BeginChild("##scrolling");
                ImGui::SetScrollFromPosY(ImGui::GetCursorStartPos().y + (GotoAddr / Cols) * ImGui::GetTextLineHeight());
                ImGui::EndChild();
                DataEditingAddr = DataPreviewAddr = GotoAddr;
                DataEditingTakeFocus = true;
            }
            GotoAddr = (size_t)-1;
        }
    }

    void DrawPreviewLine(const Sizes& s, void* mem_data_void, size_t mem_size, size_t base_display_addr)
    {
        IM_UNUSED(base_display_addr);
        ImU8* mem_data = (ImU8*)mem_data_void;
        ImGuiStyle& style = ImGui::GetStyle();
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Preview as:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth((s.GlyphWidth * 10.0f) + style.FramePadding.x * 2.0f + style.ItemInnerSpacing.x);
        if (ImGui::BeginCombo("##combo_type", DataTypeGetDesc(PreviewDataType), ImGuiComboFlags_HeightLargest))
        {
            for (int n = 0; n < ImGuiDataType_COUNT; n++)
                if (ImGui::Selectable(DataTypeGetDesc((ImGuiDataType)n), PreviewDataType == n))
                    PreviewDataType = (ImGuiDataType)n;
            ImGui::EndCombo();
        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth((s.GlyphWidth * 6.0f) + style.FramePadding.x * 2.0f + style.ItemInnerSpacing.x);
        ImGui::Combo("##combo_endianess", &PreviewEndianess, "LE\0BE\0\0");

        char buf[128] = "";
        float x = s.GlyphWidth * 6.0f;
        bool has_value = DataPreviewAddr != (size_t)-1;
        if (has_value)
            DrawPreviewData(DataPreviewAddr, mem_data, mem_size, PreviewDataType, DataFormat_Dec, buf, (size_t)IM_ARRAYSIZE(buf));
        ImGui::Text("Dec"); ImGui::SameLine(x); ImGui::TextUnformatted(has_value ? buf : "N/A");
        if (has_value)
            DrawPreviewData(DataPreviewAddr, mem_data, mem_size, PreviewDataType, DataFormat_Hex, buf, (size_t)IM_ARRAYSIZE(buf));
        ImGui::Text("Hex"); ImGui::SameLine(x); ImGui::TextUnformatted(has_value ? buf : "N/A");
        if (has_value)
            DrawPreviewData(DataPreviewAddr, mem_data, mem_size, PreviewDataType, DataFormat_Bin, buf, (size_t)IM_ARRAYSIZE(buf));
        buf[IM_ARRAYSIZE(buf) - 1] = 0;
        ImGui::Text("Bin"); ImGui::SameLine(x); ImGui::TextUnformatted(has_value ? buf : "N/A");
    }

    // Utilities for Data Preview
    const char* DataTypeGetDesc(ImGuiDataType data_type) const
    {
        const char* descs[] = { "Int8", "Uint8", "Int16", "Uint16", "Int32", "Uint32", "Int64", "Uint64", "Float", "Double" };
        IM_ASSERT(data_type >= 0 && data_type < ImGuiDataType_COUNT);
        return descs[data_type];
    }

    size_t DataTypeGetSize(ImGuiDataType data_type) const
    {
        const size_t sizes[] = { 1, 1, 2, 2, 4, 4, 8, 8, sizeof(float), sizeof(double) };
        IM_ASSERT(data_type >= 0 && data_type < ImGuiDataType_COUNT);
        return sizes[data_type];
    }

    const char* DataFormatGetDesc(DataFormat data_format) const
    {
        const char* descs[] = { "Bin", "Dec", "Hex" };
        IM_ASSERT(data_format >= 0 && data_format < DataFormat_COUNT);
        return descs[data_format];
    }

    bool IsBigEndian() const
    {
        uint16_t x = 1;
        char c[2];
        memcpy(c, &x, 2);
        return c[0] != 0;
    }

    static void* EndianessCopyBigEndian(void* _dst, void* _src, size_t s, int is_little_endian)
    {
        if (is_little_endian)
        {
            uint8_t* dst = (uint8_t*)_dst;
            uint8_t* src = (uint8_t*)_src + s - 1;
            for (int i = 0, n = (int)s; i < n; ++i)
                memcpy(dst++, src--, 1);
            return _dst;
        }
        else
        {
            return memcpy(_dst, _src, s);
        }
    }

    static void* EndianessCopyLittleEndian(void* _dst, void* _src, size_t s, int is_little_endian)
    {
        if (is_little_endian)
        {
            return memcpy(_dst, _src, s);
        }
        else
        {
            uint8_t* dst = (uint8_t*)_dst;
            uint8_t* src = (uint8_t*)_src + s - 1;
            for (int i = 0, n = (int)s; i < n; ++i)
                memcpy(dst++, src--, 1);
            return _dst;
        }
    }

    void* EndianessCopy(void* dst, void* src, size_t size) const
    {
        static void* (*fp)(void*, void*, size_t, int) = NULL;
        if (fp == NULL)
            fp = IsBigEndian() ? EndianessCopyBigEndian : EndianessCopyLittleEndian;
        return fp(dst, src, size, PreviewEndianess);
    }

    const char* FormatBinary(const uint8_t* buf, int width) const
    {
        IM_ASSERT(width <= 64);
        size_t out_n = 0;
        static char out_buf[64 + 8 + 1];
        int n = width / 8;
        for (int j = n - 1; j >= 0; --j)
        {
            for (int i = 0; i < 8; ++i)
                out_buf[out_n++] = (buf[j] & (1 << (7 - i))) ? '1' : '0';
            out_buf[out_n++] = ' ';
        }
        IM_ASSERT(out_n < IM_ARRAYSIZE(out_buf));
        out_buf[out_n] = 0;
        return out_buf;
    }

    // [Internal]
    void DrawPreviewData(size_t addr, const ImU8* mem_data, size_t mem_size, ImGuiDataType data_type, DataFormat data_format, char* out_buf, size_t out_buf_size) const
    {
        uint8_t buf[8];
        size_t elem_size = DataTypeGetSize(data_type);
        size_t size = addr + elem_size > mem_size ? mem_size - addr : elem_size;
        if (ReadFn)
            for (int i = 0, n = (int)size; i < n; ++i)
                buf[i] = ReadFn(mem_data, addr + i);
        else
            memcpy(buf, mem_data + addr, size);

        if (data_format == DataFormat_Bin)
        {
            uint8_t binbuf[8];
            EndianessCopy(binbuf, buf, size);
            ImSnprintf(out_buf, out_buf_size, "%s", FormatBinary(binbuf, (int)size * 8));
            return;
        }

        out_buf[0] = 0;
        switch (data_type)
        {
        case ImGuiDataType_S8:
        {
            int8_t int8 = 0;
            EndianessCopy(&int8, buf, size);
            if (data_format == DataFormat_Dec) { ImSnprintf(out_buf, out_buf_size, "%hhd", int8); return; }
            if (data_format == DataFormat_Hex) { ImSnprintf(out_buf, out_buf_size, "0x%02x", int8 & 0xFF); return; }
            break;
        }
        case ImGuiDataType_U8:
        {
            uint8_t uint8 = 0;
            EndianessCopy(&uint8, buf, size);
            if (data_format == DataFormat_Dec) { ImSnprintf(out_buf, out_buf_size, "%hhu", uint8); return; }
            if (data_format == DataFormat_Hex) { ImSnprintf(out_buf, out_buf_size, "0x%02x", uint8 & 0XFF); return; }
            break;
        }
        case ImGuiDataType_S16:
        {
            int16_t int16 = 0;
            EndianessCopy(&int16, buf, size);
            if (data_format == DataFormat_Dec) { ImSnprintf(out_buf, out_buf_size, "%hd", int16); return; }
            if (data_format == DataFormat_Hex) { ImSnprintf(out_buf, out_buf_size, "0x%04x", int16 & 0xFFFF); return; }
            break;
        }
        case ImGuiDataType_U16:
        {
            uint16_t uint16 = 0;
            EndianessCopy(&uint16, buf, size);
            if (data_format == DataFormat_Dec) { ImSnprintf(out_buf, out_buf_size, "%hu", uint16); return; }
            if (data_format == DataFormat_Hex) { ImSnprintf(out_buf, out_buf_size, "0x%04x", uint16 & 0xFFFF); return; }
            break;
        }
        case ImGuiDataType_S32:
        {
            int32_t int32 = 0;
            EndianessCopy(&int32, buf, size);
            if (data_format == DataFormat_Dec) { ImSnprintf(out_buf, out_buf_size, "%d", int32); return; }
            if (data_format == DataFormat_Hex) { ImSnprintf(out_buf, out_buf_size, "0x%08x", int32); return; }
            break;
        }
        case ImGuiDataType_U32:
        {
            uint32_t uint32 = 0;
            EndianessCopy(&uint32, buf, size);
            if (data_format == DataFormat_Dec) { ImSnprintf(out_buf, out_buf_size, "%u", uint32); return; }
            if (data_format == DataFormat_Hex) { ImSnprintf(out_buf, out_buf_size, "0x%08x", uint32); return; }
            break;
        }
        case ImGuiDataType_S64:
        {
            int64_t int64 = 0;
            EndianessCopy(&int64, buf, size);
            if (data_format == DataFormat_Dec) { ImSnprintf(out_buf, out_buf_size, "%lld", (long long)int64); return; }
            if (data_format == DataFormat_Hex) { ImSnprintf(out_buf, out_buf_size, "0x%016llx", (long long)int64); return; }
            break;
        }
        case ImGuiDataType_U64:
        {
            uint64_t uint64 = 0;
            EndianessCopy(&uint64, buf, size);
            if (data_format == DataFormat_Dec) { ImSnprintf(out_buf, out_buf_size, "%llu", (long long)uint64); return; }
            if (data_format == DataFormat_Hex) { ImSnprintf(out_buf, out_buf_size, "0x%016llx", (long long)uint64); return; }
            break;
        }
        case ImGuiDataType_Float:
        {
            float float32 = 0.0f;
            EndianessCopy(&float32, buf, size);
            if (data_format == DataFormat_Dec) { ImSnprintf(out_buf, out_buf_size, "%f", float32); return; }
            if (data_format == DataFormat_Hex) { ImSnprintf(out_buf, out_buf_size, "%a", float32); return; }
            break;
        }
        case ImGuiDataType_Double:
        {
            double float64 = 0.0;
            EndianessCopy(&float64, buf, size);
            if (data_format == DataFormat_Dec) { ImSnprintf(out_buf, out_buf_size, "%f", float64); return; }
            if (data_format == DataFormat_Hex) { ImSnprintf(out_buf, out_buf_size, "%a", float64); return; }
            break;
        }
        case ImGuiDataType_COUNT:
            break;
        } // Switch
        IM_ASSERT(0); // Shouldn't reach
    }
};
/*== end of imgui_memory_editor.h ============================================*/

/* ImGui MemoryEditor read/write callbacks */
static uint8_t _ui_memedit_readfn(const uint8_t* ptr, size_t off) {
    /* we'll treat the "data ptr" as "user data" */
    const ui_memedit_t* win = (ui_memedit_t*) ptr;
    CHIPS_ASSERT(win && win->ed);
    if (win->read_cb) {
        return win->read_cb(win->ed->CurLayer, (uint16_t)off, win->user_data);
    }
    else {
        return 0;
    }
}

static void _ui_memedit_writefn(uint8_t* ptr, size_t off, uint8_t val) {
    /* we'll treat the "data ptr" as "user data" */
    const ui_memedit_t* win = (ui_memedit_t*) ptr;
    CHIPS_ASSERT(win && win->ed);
    if (win->write_cb) {
        win->write_cb(win->ed->CurLayer, (uint16_t)off, val, win->user_data);
    }
}

void ui_memedit_init(ui_memedit_t* win, const ui_memedit_desc_t* desc) {
    CHIPS_ASSERT(win && desc);
    CHIPS_ASSERT(desc->title);
    CHIPS_ASSERT(0 == win->ed);
    memset(win, 0, sizeof(ui_memedit_t));
    win->title = desc->title;
    win->read_cb = desc->read_cb;
    win->write_cb = desc->write_cb;
    win->user_data = desc->user_data;
    win->init_x = (float) desc->x;
    win->init_y = (float) desc->y;
    win->init_w = (float) ((desc->w == 0) ? 512 : desc->w);
    win->init_h = (float) ((desc->h == 0) ? 120 : desc->h);
    win->max_addr = (desc->max_addr == 0) ? (1<<16) : desc->max_addr;
    win->open = desc->open;
    win->ed = new MemoryEditor;
    win->ed->Cols = (desc->num_cols == 0) ? win->ed->Cols : desc->num_cols;
    win->ed->OptShowOptions = !desc->hide_options;
    win->ed->OptShowAddrInput = !desc->hide_addr_input;
    win->ed->OptShowAscii = !desc->hide_ascii;
    win->ed->Open = win->open;
    win->ed->ReadFn = _ui_memedit_readfn;
    win->ed->WriteFn = _ui_memedit_writefn;
    win->ed->OptAddrDigitsCount = 4;
    for (int i = 0; i < UI_MEMEDIT_MAX_LAYERS; i++) {
        if (desc->layers[i]) {
            win->ed->NumLayers++;
            win->ed->Layers[i] = desc->layers[i];
        }
        else {
            break;
        }
    }
    win->valid = true;
}

void ui_memedit_discard(ui_memedit_t* win) {
    CHIPS_ASSERT(win && win->ed && win->valid);
    delete win->ed; win->ed = nullptr;
    win->valid = false;
}

void ui_memedit_draw(ui_memedit_t* win) {
    CHIPS_ASSERT(win && win->ed && win->title && win->valid);
    win->ed->Open = win->open;
    if (!win->ed->Open) {
        return;
    }
    ImGui::SetNextWindowPos(ImVec2(win->init_x, win->init_y), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(win->init_w, win->init_h), ImGuiCond_FirstUseEver);
    win->ed->DrawWindow(win->title, (uint8_t*)win, win->max_addr);
    win->open = win->ed->Open;
}

void ui_memedit_draw_content(ui_memedit_t* win) {
    CHIPS_ASSERT(win && win->valid);
    win->ed->DrawContents((uint8_t*)win, win->max_addr);
}
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#endif /* CHIPS_UI_IMPL */
