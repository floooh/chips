#pragma once
/*
    sys_common.h

    Common data types for chips system headers.

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
*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void* ptr;
    size_t size;
} chips_range_t;

typedef struct {
    int width, height;
} chips_dim_t;

typedef struct {
    int x, y, width, height;
} chips_rect_t;

typedef struct {
    struct {
        chips_dim_t dim;        // framebuffer dimensions in pixels
        chips_range_t buffer;
        size_t bytes_per_pixel; // 1 or 4
    } frame;
    chips_rect_t screen;
    chips_range_t palette;
    bool portrait;
} chips_display_info_t;

typedef struct {
    void (*func)(const float* samples, int num_samples, void* user_data);
    void* user_data;
} chips_audio_callback_t;

typedef void (*chips_debug_func_t)(void* user_data, uint64_t pins);
typedef struct {
    struct {
        chips_debug_func_t func;
        void* user_data;
    } callback;
    bool* stopped;
} chips_debug_t;

typedef struct {
    chips_audio_callback_t callback;
    int num_samples;
    int sample_rate;
    float volume;
} chips_audio_desc_t;

// prepare chips_audio_t snapshot for saving
void chips_audio_callback_snapshot_onsave(chips_audio_callback_t* snapshot);
// fixup chips_audio_t snapshot after loading
void chips_audio_callback_snapshot_onload(chips_audio_callback_t* snapshot, chips_audio_callback_t* sys);
// prepare chips_debut_t snapshot for saving
void chips_debug_snapshot_onsave(chips_debug_t* snapshot);
// fixup chips_debug_t snapshot after loading
void chips_debug_snapshot_onload(chips_debug_t* snapshot, chips_debug_t* sys);

#ifdef __cplusplus
} // extern "C"
#endif

/*--- IMPLEMENTATION ---------------------------------------------------------*/
#ifdef CHIPS_IMPL

void chips_audio_callback_snapshot_onsave(chips_audio_callback_t* snapshot) {
    snapshot->func = 0;
    snapshot->user_data = 0;
}

void chips_audio_callback_snapshot_onload(chips_audio_callback_t* snapshot, chips_audio_callback_t* sys) {
    snapshot->func = sys->func;
    snapshot->user_data = sys->user_data;
}

void chips_debug_snapshot_onsave(chips_debug_t* snapshot) {
    snapshot->callback.func = 0;
    snapshot->callback.user_data = 0;
    snapshot->stopped = 0;
}

void chips_debug_snapshot_onload(chips_debug_t* snapshot, chips_debug_t* sys) {
    snapshot->callback.func = sys->callback.func;
    snapshot->callback.user_data = sys->callback.user_data;
    snapshot->stopped = sys->stopped;
}

#endif // CHIPS_IMPL
