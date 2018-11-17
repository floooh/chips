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

#ifdef __cplusplus
} /* extern "C" */
#endif

/*-- IMPLEMENTATION (include in C++ source) ----------------------------------*/
#ifdef CHIPS_IMPL
#ifndef __cplusplus
#error "implementation must be compiled as C++"
#endif
#include <string.h> /* memset */
#include <stdio.h>  /* sscanf */

uint16_t ui_util_input_u16(const char* label, uint16_t val) {
    static char buf[5];
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
    ImGui::Text("%s", label); ImGui::SameLine(); ImGui::Text("%s", str);
}

#endif /* CHIPS_IMPL */
