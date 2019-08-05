#pragma once
/*#
    # ui_memedit.h

    Memory viewer/editor UI using Dear ImGui.

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

#ifdef CHIPS_IMPL
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
    MemoryEditor* ed;
    bool open;
    bool valid;
} ui_memedit_t;

/* NOTE: win MUST be zero-initialized already, allocates memory via new() */
void ui_memedit_init(ui_memedit_t* win, const ui_memedit_desc_t* desc);
/* frees memory via delete() */
void ui_memedit_discard(ui_memedit_t* win);
void ui_memedit_draw(ui_memedit_t* win);

#ifdef __cplusplus
} /* extern "C" */
#endif

/*-- IMPLEMENTATION (include in C++ source) ----------------------------------*/
#ifdef CHIPS_IMPL
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

struct MemoryEditor
{
    typedef unsigned char u8;

    // Settings
    bool            Open;                                   // = true   // set to false when DrawWindow() was closed. ignore if not using DrawWindow
    bool            ReadOnly;                               // = false  // set to true to disable any editing
    int             Rows;                                   // = 16     //
    bool            OptShowAscii;                           // = true   //
    bool            OptShowHexII;                           // = false  //
    bool            OptGreyOutZeroes;                       // = true   //
    int             OptMidRowsCount;                        // = 8      // set to 0 to disable extra spacing between every mid-rows
    int             OptAddrDigitsCount;                     // = 0      // number of addr digits to display (default calculated based on maximum displayed addr)
    ImU32           HighlightColor;                         //          // color of highlight
    u8              (*ReadFn)(u8* data, size_t off);        // = NULL   // optional handler to read bytes
    void            (*WriteFn)(u8* data, size_t off, u8 d); // = NULL   // optional handler to write bytes
    bool            (*HighlightFn)(u8* data, size_t off);   // = NULL   // optional handler to return Highlight property (to support non-contiguous highlighting)

    /*--- BEGIN ui_memedit.h changes ---*/
    int NumLayers;
    int CurLayer;
    const char* Layers[UI_MEMEDIT_MAX_LAYERS];
    /*--- END ui_memedit.h changes ---*/

    // State/Internals
    bool            ContentsWidthChanged;
    size_t          DataEditingAddr;
    bool            DataEditingTakeFocus;
    char            DataInputBuf[32];
    char            AddrInputBuf[32];
    size_t          GotoAddr;
    size_t          HighlightMin, HighlightMax;

    MemoryEditor()
    {
        // Settings
        Open = true;
        ReadOnly = false;
        Rows = 16;
        OptShowAscii = true;
        OptShowHexII = false;
        OptGreyOutZeroes = true;
        OptMidRowsCount = 8;
        OptAddrDigitsCount = 0;
        HighlightColor = IM_COL32(255, 255, 255, 40);
        ReadFn = NULL;
        WriteFn = NULL;
        HighlightFn = NULL;

        // State/Internals
        ContentsWidthChanged = false;
        DataEditingAddr = (size_t)-1;
        DataEditingTakeFocus = false;
        memset(DataInputBuf, 0, sizeof(DataInputBuf));
        memset(AddrInputBuf, 0, sizeof(AddrInputBuf));
        GotoAddr = (size_t)-1;
        HighlightMin = HighlightMax = (size_t)-1;

        /*--- BEGIN ui_memedit.h changes ---*/
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
        float   SpacingBetweenMidRows;
        float   PosHexStart;
        float   PosHexEnd;
        float   PosAsciiStart;
        float   PosAsciiEnd;
        float   WindowWidth;
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
        s.SpacingBetweenMidRows = (float)(int)(s.HexCellWidth * 0.25f); // Every OptMidRowsCount columns we add a bit of extra spacing
        s.PosHexStart = (s.AddrDigitsCount + 2) * s.GlyphWidth;
        s.PosHexEnd = s.PosHexStart + (s.HexCellWidth * Rows);
        s.PosAsciiStart = s.PosAsciiEnd = s.PosHexEnd;
        if (OptShowAscii)
        {
            s.PosAsciiStart = s.PosHexEnd + s.GlyphWidth * 1;
            if (OptMidRowsCount > 0)
                s.PosAsciiStart += ((Rows + OptMidRowsCount - 1) / OptMidRowsCount) * s.SpacingBetweenMidRows;
            s.PosAsciiEnd = s.PosAsciiStart + Rows * s.GlyphWidth;
        }
        s.WindowWidth = s.PosAsciiEnd + style.ScrollbarSize + style.WindowPadding.x * 2 + s.GlyphWidth;
    }

#ifdef _MSC_VER
#define _PRISizeT   "IX"
#else
#define _PRISizeT   "zX"
#endif

    // Standalone Memory Editor window
    void DrawWindow(const char* title, u8* mem_data, size_t mem_size, size_t base_display_addr = 0x0000)
    {
        Sizes s;
        CalcSizes(s, mem_size, base_display_addr);
        ImGui::SetNextWindowSizeConstraints(ImVec2(0.0f, 0.0f), ImVec2(s.WindowWidth, FLT_MAX));

        Open = true;
        if (ImGui::Begin(title, &Open, ImGuiWindowFlags_NoScrollbar))
        {
            if (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows) && ImGui::IsMouseClicked(1))
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
    void DrawContents(u8* mem_data, size_t mem_size, size_t base_display_addr = 0x0000)
    {
        Sizes s;
        CalcSizes(s, mem_size, base_display_addr);
        if (0 == AddrInputBuf[0]) {
            sprintf(AddrInputBuf, "%0*" _PRISizeT, s.AddrDigitsCount, base_display_addr);
        }
        ImGuiStyle& style = ImGui::GetStyle();

        const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing(); // 1 separator, 1 input text
        ImGui::BeginChild("##scrolling", ImVec2(0, -footer_height_to_reserve));
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

        const int line_total_count = (int)((mem_size + Rows - 1) / Rows);
        ImGuiListClipper clipper(line_total_count, s.LineHeight);
        const size_t visible_start_addr = clipper.DisplayStart * Rows;
        const size_t visible_end_addr = clipper.DisplayEnd * Rows;

        bool data_next = false;

        if (ReadOnly || DataEditingAddr >= mem_size)
            DataEditingAddr = (size_t)-1;

        size_t data_editing_addr_backup = DataEditingAddr;
        size_t data_editing_addr_next = (size_t)-1;
        if (DataEditingAddr != (size_t)-1)
        {
            // Move cursor but only apply on next frame so scrolling with be synchronized (because currently we can't change the scrolling while the window is being rendered)
            if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_UpArrow)) && DataEditingAddr >= (size_t)Rows)          { data_editing_addr_next = DataEditingAddr - Rows; DataEditingTakeFocus = true; }
            else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_DownArrow)) && DataEditingAddr < mem_size - Rows) { data_editing_addr_next = DataEditingAddr + Rows; DataEditingTakeFocus = true; }
            else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_LeftArrow)) && DataEditingAddr > 0)               { data_editing_addr_next = DataEditingAddr - 1; DataEditingTakeFocus = true; }
            else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_RightArrow)) && DataEditingAddr < mem_size - 1)   { data_editing_addr_next = DataEditingAddr + 1; DataEditingTakeFocus = true; }
        }
        if (data_editing_addr_next != (size_t)-1 && (data_editing_addr_next / Rows) != (data_editing_addr_backup / Rows))
        {
            // Track cursor movements
            const int scroll_offset = ((int)(data_editing_addr_next / Rows) - (int)(data_editing_addr_backup / Rows));
            const bool scroll_desired = (scroll_offset < 0 && data_editing_addr_next < visible_start_addr + Rows * 2) || (scroll_offset > 0 && data_editing_addr_next > visible_end_addr - Rows * 2);
            if (scroll_desired)
                ImGui::SetScrollY(ImGui::GetScrollY() + scroll_offset * s.LineHeight);
        }

        // Draw vertical separator
        ImVec2 window_pos = ImGui::GetWindowPos();
        if (OptShowAscii)
            draw_list->AddLine(ImVec2(window_pos.x + s.PosAsciiStart - s.GlyphWidth, window_pos.y), ImVec2(window_pos.x + s.PosAsciiStart - s.GlyphWidth, window_pos.y + 9999), ImGui::GetColorU32(ImGuiCol_Border));

        const ImU32 color_text = ImGui::GetColorU32(ImGuiCol_Text);
        const ImU32 color_disabled = OptGreyOutZeroes ? ImGui::GetColorU32(ImGuiCol_TextDisabled) : color_text;

        for (int line_i = clipper.DisplayStart; line_i < clipper.DisplayEnd; line_i++) // display only visible lines
        {
            size_t addr = (size_t)(line_i * Rows);
            ImGui::Text("%0*" _PRISizeT ": ", s.AddrDigitsCount, base_display_addr + addr);

            // Draw Hexadecimal
            for (int n = 0; n < Rows && addr < mem_size; n++, addr++)
            {
                float byte_pos_x = s.PosHexStart + s.HexCellWidth * n;
                if (OptMidRowsCount > 0)
                    byte_pos_x += (n / OptMidRowsCount) * s.SpacingBetweenMidRows;
                ImGui::SameLine(byte_pos_x);

                // Draw highlight
                if ((addr >= HighlightMin && addr < HighlightMax) || (HighlightFn && HighlightFn(mem_data, addr)))
                {
                    ImVec2 pos = ImGui::GetCursorScreenPos();
                    float highlight_width = s.GlyphWidth * 2;
                    bool is_next_byte_highlighted =  (addr + 1 < mem_size) && ((HighlightMax != (size_t)-1 && addr + 1 < HighlightMax) || (HighlightFn && HighlightFn(mem_data, addr + 1)));
                    if (is_next_byte_highlighted || (n + 1 == Rows))
                    {
                        highlight_width = s.HexCellWidth;
                        if (OptMidRowsCount > 0 && n > 0 && (n + 1) < Rows && ((n + 1) % OptMidRowsCount) == 0)
                            highlight_width += s.SpacingBetweenMidRows;
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
                        ImGui::SetKeyboardFocusHere();
                        ImGui::CaptureKeyboardFromApp(true);
                        sprintf(AddrInputBuf, "%0*" _PRISizeT, s.AddrDigitsCount, base_display_addr + addr);
                        sprintf(DataInputBuf, "%02X", ReadFn ? ReadFn(mem_data, addr) : mem_data[addr]);
                    }
                    ImGui::PushItemWidth(s.GlyphWidth * 2);
                    struct UserData
                    {
                        // FIXME: We should have a way to retrieve the text edit cursor position more easily in the API, this is rather tedious. This is such a ugly mess we may be better off not using InputText() at all here.
                        static int Callback(ImGuiTextEditCallbackData* data)
                        {
                            UserData* user_data = (UserData*)data->UserData;
                            if (!data->HasSelection())
                                user_data->CursorPos = data->CursorPos;
                            if (data->SelectionStart == 0 && data->SelectionEnd == data->BufTextLen)
                            {
                                // When not editing a byte, always rewrite its content (this is a bit tricky, since InputText technically "owns" the master copy of the buffer we edit it in there)
                                data->DeleteChars(0, data->BufTextLen);
                                data->InsertChars(0, user_data->CurrentBufOverwrite);
                                data->SelectionStart = 0;
                                data->SelectionEnd = data->CursorPos = 2;
                            }
                            return 0;
                        }
                        char   CurrentBufOverwrite[3];  // Input
                        int    CursorPos;               // Output
                    };
                    UserData user_data;
                    user_data.CursorPos = -1;
                    sprintf(user_data.CurrentBufOverwrite, "%02X", ReadFn ? ReadFn(mem_data, addr) : mem_data[addr]);
                    ImGuiInputTextFlags flags = ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_NoHorizontalScroll | ImGuiInputTextFlags_AlwaysInsertMode | ImGuiInputTextFlags_CallbackAlways;
                    if (ImGui::InputText("##data", DataInputBuf, 32, flags, UserData::Callback, &user_data))
                        data_write = data_next = true;
                    else if (!DataEditingTakeFocus && !ImGui::IsItemActive())
                        DataEditingAddr = data_editing_addr_next = (size_t)-1;
                    DataEditingTakeFocus = false;
                    ImGui::PopItemWidth();
                    if (user_data.CursorPos >= 2)
                        data_write = data_next = true;
                    if (data_editing_addr_next != (size_t)-1)
                        data_write = data_next = false;
                    int data_input_value;
                    if (data_write && sscanf(DataInputBuf, "%X", &data_input_value) == 1)
                    {
                        if (WriteFn)
                            WriteFn(mem_data, addr, (u8)data_input_value);
                        else
                            mem_data[addr] = (u8)data_input_value;
                    }
                    ImGui::PopID();
                }
                else
                {
                    // NB: The trailing space is not visible but ensure there's no gap that the mouse cannot click on.
                    u8 b = ReadFn ? ReadFn(mem_data, addr) : mem_data[addr];

                    if (OptShowHexII)
                    {
                        if ((b >= 32 && b < 128))
                            ImGui::Text(".%c ", b);
                        else if (b == 0xFF && OptGreyOutZeroes)
                            ImGui::TextDisabled("## ");
                        else if (b == 0x00)
                            ImGui::Text("   ");
                        else
                            ImGui::Text("%02X ", b);
                    }
                    else
                    {
                        if (b == 0 && OptGreyOutZeroes)
                            ImGui::TextDisabled("00 ");
                        else
                            ImGui::Text("%02X ", b);
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
                addr = line_i * Rows;
                ImGui::PushID(line_i);
                if (ImGui::InvisibleButton("ascii", ImVec2(s.PosAsciiEnd - s.PosAsciiStart, s.LineHeight)))
                {
                    DataEditingAddr = addr + (size_t)((ImGui::GetIO().MousePos.x - pos.x) / s.GlyphWidth);
                    DataEditingTakeFocus = true;
                }
                ImGui::PopID();
                for (int n = 0; n < Rows && addr < mem_size; n++, addr++)
                {
                    if (addr == DataEditingAddr)
                    {
                        draw_list->AddRectFilled(pos, ImVec2(pos.x + s.GlyphWidth, pos.y + s.LineHeight), ImGui::GetColorU32(ImGuiCol_FrameBg));
                        draw_list->AddRectFilled(pos, ImVec2(pos.x + s.GlyphWidth, pos.y + s.LineHeight), ImGui::GetColorU32(ImGuiCol_TextSelectedBg));
                    }
                    unsigned char c = ReadFn ? ReadFn(mem_data, addr) : mem_data[addr];
                    char display_c = (c < 32 || c >= 128) ? '.' : c;
                    draw_list->AddText(pos, (display_c == '.') ? color_disabled : color_text, &display_c, &display_c + 1);
                    pos.x += s.GlyphWidth;
                }
            }
        }
        clipper.End();
        ImGui::PopStyleVar(2);
        ImGui::EndChild();

        if (data_next && DataEditingAddr < mem_size)
        {
            DataEditingAddr = DataEditingAddr + 1;
            DataEditingTakeFocus = true;
        }
        else if (data_editing_addr_next != (size_t)-1)
        {
            DataEditingAddr = data_editing_addr_next;
        }

        ImGui::Separator();

        // Options menu
        if (ImGui::Button("Options"))
            ImGui::OpenPopup("context");
        if (ImGui::BeginPopup("context"))
        {
            ImGui::PushItemWidth(56);
            if (ImGui::DragInt("##rows", &Rows, 0.2f, 4, 32, "%.0f rows")) ContentsWidthChanged = true;
            ImGui::PopItemWidth();
            ImGui::Checkbox("Show HexII", &OptShowHexII);
            if (ImGui::Checkbox("Show Ascii", &OptShowAscii)) ContentsWidthChanged = true;
            ImGui::Checkbox("Grey out zeroes", &OptGreyOutZeroes);
            ImGui::EndPopup();
        }

        ImGui::SameLine();
        ImGui::PushItemWidth((s.AddrDigitsCount + 1) * s.GlyphWidth + style.FramePadding.x * 2.0f);
        if (ImGui::InputText("##addr", AddrInputBuf, 32, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_EnterReturnsTrue))
        {
            size_t goto_addr;
            if (sscanf(AddrInputBuf, "%" _PRISizeT, &goto_addr) == 1)
            {
                GotoAddr = goto_addr - base_display_addr;
                HighlightMin = HighlightMax = (size_t)-1;
            }
        }
        ImGui::PopItemWidth();

        /*--- BEGIN ui_memedit.h changes */
        ImGui::SameLine();
        ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth());
        ImGui::Combo("##layer", &CurLayer, Layers, NumLayers);
        ImGui::PopItemWidth();
        /*--- END ui_memedit.h changes */

        if (GotoAddr != (size_t)-1)
        {
            if (GotoAddr < mem_size)
            {
                ImGui::BeginChild("##scrolling");
                ImGui::SetScrollFromPosY(ImGui::GetCursorStartPos().y + (GotoAddr / Rows) * ImGui::GetTextLineHeight());
                ImGui::EndChild();
                DataEditingAddr = GotoAddr;
                DataEditingTakeFocus = true;
            }
            GotoAddr = (size_t)-1;
        }

        // Notify the main window of our ideal child content size (FIXME: we are missing an API to get the contents size from the child)
        ImGui::SetCursorPosX(s.WindowWidth);
    }
};
#undef _PRISizeT
/*== end of imgui_memory_editor.h ============================================*/

/* ImGui MemoryEditor read/write callbacks */
static uint8_t _ui_memedit_readfn(uint8_t* ptr, size_t off) {
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
    win->open = desc->open;
    win->ed = new MemoryEditor;
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
    ImGui::SetNextWindowPos(ImVec2(win->init_x, win->init_y), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(win->init_w, win->init_h), ImGuiCond_Once);
    win->ed->DrawWindow(win->title, (uint8_t*)win, (1<<16));
    win->open = win->ed->Open;
}
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#endif /* CHIPS_IMPL */
