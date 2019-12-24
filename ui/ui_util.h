#pragma once
/*#
    # ui_util.h

    UI helper functions. Include this header before most other Chips UI headers.

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

/* draw an 16-bit hex text input field */
uint16_t ui_util_input_u16(const char* label, uint16_t val);
/* draw an 8-bit hex text input field */
uint8_t ui_util_input_u8(const char* label, uint8_t val);
/* draw an 8-bit hex label/value text */
void ui_util_u8(const char* label, uint8_t val);
/* draw a 16-bit hex label/value text */
void ui_util_u16(const char* label, uint16_t val);
/* draw an 8-bit binary label/value text */
void ui_util_b8(const char* label, uint8_t val);
/* draw a 24-bit binary label/value text */
void ui_util_b24(const char* label, uint32_t val);
/* draw a 32-bit binary label/value text */
void ui_util_b32(const char* label, uint32_t val);
/* get an ImGui style color (ImGuiCol_*) with overall alpha applied */
uint32_t ui_util_color(int imgui_color);
/* inject the common options menu */
void ui_util_options_menu(double time_ms);

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
#include <stdio.h>  /* sscanf */

uint16_t ui_util_input_u16(const char* label, uint16_t val) {
    char buf[5];
    for (int i = 0; i < 4; i++) {
        buf[i] = "0123456789ABCDEF"[val>>((3-i)*4) & 0xF];
    }
    buf[4] = 0;
    const int flags = ImGuiInputTextFlags_CharsHexadecimal|
        ImGuiInputTextFlags_CharsUppercase|
        ImGuiInputTextFlags_EnterReturnsTrue;
    ImGui::PushItemWidth(38);
    if (ImGui::InputText(label, buf, sizeof(buf), flags)) {
        int res;
        if (sscanf(buf, "%X", &res) == 1) {
            val = (uint16_t) res;
        }
    }
    ImGui::PopItemWidth();
    return val;
}

uint8_t ui_util_input_u8(const char* label, uint8_t val) {
    char buf[3];
    for (int i = 0; i < 2; i++) {
        buf[i] = "0123456789ABCDEF"[val>>((1-i)*4) & 0xF];
    }
    buf[2] = 0;
    const int flags = ImGuiInputTextFlags_CharsHexadecimal|
        ImGuiInputTextFlags_CharsUppercase|
        ImGuiInputTextFlags_EnterReturnsTrue;
    ImGui::PushItemWidth(22);
    if (ImGui::InputText(label, buf, sizeof(buf), flags)) {
        int res;
        if (sscanf(buf, "%X", &res) == 1) {
            val = (uint8_t) res;
        }
    }
    ImGui::PopItemWidth();
    return val;
}

void ui_util_u8(const char* label, uint8_t val) {
    ImGui::Text("%s", label); ImGui::SameLine(); ImGui::Text("%02X", val);
}

void ui_util_u8(const char* label, uint16_t val) {
    ImGui::Text("%s", label); ImGui::SameLine(); ImGui::Text("%04X", val);
}

void ui_util_b8(const char* label, uint8_t val) {
    char str[9];
    for (int i = 0; i < 8; i++) {
        str[i] = (val & (1<<(7-i))) ? '1':'0';
    }
    str[8] = 0;
    ImGui::Text("%s%s", label, str);
}

void ui_util_b24(const char* label, uint32_t val) {
    char str[25];
    for (int i = 0; i < 24; i++) {
        str[i] = (val & (1<<(23-i))) ? '1':'0';
    }
    str[24] = 0;
    ImGui::Text("%s%s", label, str);
}

void ui_util_b32(const char* label, uint32_t val) {
    char str[33];
    for (int i = 0; i < 32; i++) {
        str[i] = (val & (1<<(31-i))) ? '1':'0';
    }
    str[32] = 0;
    ImGui::Text("%s%s", label, str);
}

uint32_t ui_util_color(int imgui_color) {
    CHIPS_ASSERT((imgui_color >= 0) && (imgui_color < ImGuiCol_COUNT));
    const ImGuiStyle& style = ImGui::GetStyle();
    ImVec4 c = style.Colors[imgui_color];
    c.w *= style.Alpha;
    return ImColor(c);
}

void ui_util_options_menu(double time_ms, bool stopped) {
    if (ImGui::BeginMenu("Options")) {
        ImGui::SliderFloat("UI Alpha", &ImGui::GetStyle().Alpha, 0.1f, 1.0f);
        ImGui::SliderFloat("BG Alpha", &ImGui::GetStyle().Colors[ImGuiCol_WindowBg].w, 0.1f, 1.0f);
        static int theme = 0;
        if (ImGui::RadioButton("Dark Theme", &theme, 0)) {
            ImGui::StyleColorsDark();
        }
        if (ImGui::RadioButton("Light Theme", &theme, 1)) {
            ImGui::StyleColorsLight();
        }
        if (ImGui::RadioButton("Classic Theme", &theme, 2)) {
            ImGui::StyleColorsClassic();
        }
        ImGui::EndMenu();
    }
    ImGui::SameLine(ImGui::GetWindowWidth() - 120);
    if (stopped) {
        ImGui::Text("emu: stopped");
    }
    else {
        ImGui::Text("emu: %.2fms", time_ms);
    }
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
#endif /* CHIPS_IMPL */
