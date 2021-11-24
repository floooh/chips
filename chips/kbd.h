#pragma once
/*
    kbd.h -- keyboard matrix helpers

    Do this:
        #define CHIPS_IMPL
    before you include this file in *one* C or C++ file to create the 
    implementation.

    Optionally provide the following macros with your own implementation
    
        CHIPS_ASSERT(c)     -- your own assert macro (default: assert(c))

    OVERVIEW

    A kbd_t instance maps key codes to the cross-sections of
    an up to 12x12 keyboard matrix with up to 4 modifier keys (shift, ctrl, ...)

        C0  C1  C1  C2  C3 ... C12
         /   /   /   /   /       |
    L0---+---+---+---+---+-....--+
         |   /   /   /   /       /
    L1---+---+---+---+---+-....--+
         .   .   .   .   .       .
    L12--+---+---+---+---+-....--+

    First register host-system key codes with the keyboard matrix (where are keys
    positioned in the matrix) by calling kbd_register_modifier() and
    kbd_register_key().

    Feed 'host system key presses' into the keyboard_matrix instance
    by calling kbd_key_down() and kbd_key_up(). Some emulated systems
    took quite long to scan a key press, so the keyboard matrix has
    a 'sticky count', which may prolong the key press visible to the
    emulated system if the host-system key press was too short.

    Call the function kbd_update() once per frame, this will keep track
    of the 'sticky count' and auto-release sticky key presses which
    have expired.

    There's two ways to chech the state of the keyboard matrix:

    (1) Call the functions kbd_test_lines() or kbd_test_columns(), these
    take a bit mask of active columns or lines as input, and return a bit mask
    of the associated lines or columns which have become active in turn.

    (2) Sometimes the input bit mask is not available in the place where the
    keyboard scanout needs to happen. In this case you can store
    an active input mask by calling either kbd_set_active_columns() or
    kbd_set_active_lines() first, and then somewhere else, call
    kbd_scan_lines() or kbd_scan_columns() to get the resulting scanned
    bit mask.

    All functions for testing/scanning the keyboard matrix state are 'fast'
    because of internal state caching, most processing happens in the
    less frequently called functions kbd_update(), kbd_key_down() and
    kbd_key_up().

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

#ifdef __cplusplus
extern "C" {
#endif

#define KBD_MAX_COLUMNS (12)
#define KBD_MAX_LINES (12)
#define KBD_MAX_MOD_KEYS (4)
#define KBD_MAX_KEYS (256)
#define KBD_MAX_PRESSED_KEYS (4)

// a pressed-key state
typedef struct {
    // key code of the pressed key
    int key;
    // mask bit layout is 8-bits modifier, and 12-bits each columns and lines
    // |SSSSSSSS|CCCCCCCCCCCC|LLLLLLLLLLLL|
    uint32_t mask;
    // timestamp when the key was pressed down
    uint64_t pressed_time;
    // true if the key has been released
    bool released;
} key_state_t;

// keyboard matrix state
typedef struct {
    // current time stamp, bumped by kbd_update()
    uint64_t cur_time;
    // number of frames a key will at least remain pressed
    uint32_t sticky_time;
    // currently active columns
    uint16_t active_columns;
    // currently active lines
    uint16_t active_lines;
    // map key ASCII code to modifier/column/line bits
    uint32_t key_masks[KBD_MAX_KEYS];
    // column/line bits for modifier keys
    uint32_t mod_masks[KBD_MAX_MOD_KEYS];
    // currently pressed keys (bitmask==0 is empty slot)
    key_state_t key_buffer[KBD_MAX_PRESSED_KEYS];
    // active column/line masks, updated when key pressed state changes
    uint16_t scanout_column_masks[KBD_MAX_LINES];
    uint16_t scanout_line_masks[KBD_MAX_COLUMNS];
    // last cached column / scanout combinations
    uint16_t cur_column_mask;
    uint16_t cur_scanout_line_mask;
    uint16_t cur_line_mask;
    uint16_t cur_scanout_column_mask;
} kbd_t;

// initialize a keyboard matrix instance, provide key-sticky duration in number of 60Hz frames
void kbd_init(kbd_t* kbd, int sticky_frames);
// update keyboard matrix state (releases sticky keys), call once per frame with frame time in micro-seconds
void kbd_update(kbd_t* kbd, uint32_t frame_time_us);
// register a modifier key, layers are from 0 to KBD_MAX_MOD_KEYS-1
void kbd_register_modifier(kbd_t* kbd, int layer, int column, int line);
// register a modifier key where the modifier is mapped to an entire keyboard line
void kbd_register_modifier_line(kbd_t* kbd, int layer, int line);
// register a modifier key where the modifier is mapped to an entire keyboard column
void kbd_register_modifier_column(kbd_t* kbd, int layer, int column);
// register a key
void kbd_register_key(kbd_t* kbd, int key, int column, int line, int mod_mask);
// add a key to the pressed-key buffer
void kbd_key_down(kbd_t* kbd, int key);
// remove a key from the pressed-key buffer
void kbd_key_up(kbd_t* kbd, int key);
// test keyboard matrix against a column bitmask and return lit lines
uint16_t kbd_test_lines(kbd_t* kbd, uint16_t column_mask);
// test keyboard matrix against a line bitmask and return lit columns
uint16_t kbd_test_columns(kbd_t* kbd, uint16_t line_mask);
// set active column mask (use together with kbd_scan_lines
static inline void kbd_set_active_columns(kbd_t* kbd, uint16_t column_mask) {
    kbd->active_columns = column_mask;
}
// scan active lines (used together with kbd_set_active_columns
static inline uint16_t kbd_scan_lines(kbd_t* kbd) {
    return kbd_test_lines(kbd, kbd->active_columns);
}
// set active lines mask (use together with kbd_scan_columns
static inline void kbd_set_active_lines(kbd_t* kbd, uint16_t line_mask) {
    kbd->active_lines = line_mask;
}
// scan active columns (used together with kbd_set_active_lines
static inline uint16_t kbd_scan_columns(kbd_t* kbd) {
    return kbd_test_columns(kbd, kbd->active_lines);
}

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

/*
    kbd_init(kbd_t* kbd, int sticky_count)

    Initialize a kbd instance:

    kbd             -- pointer to kbd_t instance
    sticky_frames   -- number of 60Hz frames a key will at least remain
                       pressed even when calling kbd_key_up()
                       to give emulated system enough time to sample the
                       keyboard matrix
*/
void kbd_init(kbd_t* kbd, int sticky_frames) {
    CHIPS_ASSERT(kbd);
    *kbd = (kbd_t){
        .sticky_time = sticky_frames * 16667
    };
}

void kbd_register_modifier(kbd_t* kbd, int layer, int column, int line) {
    CHIPS_ASSERT(kbd);
    CHIPS_ASSERT((column >= 0) && (column < KBD_MAX_COLUMNS));
    CHIPS_ASSERT((line >= 0) && (line < KBD_MAX_LINES));
    CHIPS_ASSERT((layer >= 0) && (layer < KBD_MAX_MOD_KEYS));
    kbd->mod_masks[layer] = (1<<(layer+KBD_MAX_COLUMNS+KBD_MAX_LINES)) | (1<<(column+KBD_MAX_LINES)) | (1<<line);
}

void kbd_register_modifier_line(kbd_t* kbd, int layer, int line) {
    CHIPS_ASSERT(kbd);
    CHIPS_ASSERT((line >= 0) && (line < KBD_MAX_LINES));
    CHIPS_ASSERT((layer >= 0) && (layer < KBD_MAX_MOD_KEYS));
    kbd->mod_masks[layer] = (1<<(layer+KBD_MAX_COLUMNS+KBD_MAX_LINES)) | (1<<line);
}

void kbd_register_modifier_column(kbd_t* kbd, int layer, int column) {
    CHIPS_ASSERT(kbd);
    CHIPS_ASSERT((column >= 0) && (column < KBD_MAX_COLUMNS));
    CHIPS_ASSERT((layer >= 0) && (layer < KBD_MAX_MOD_KEYS));
    kbd->mod_masks[layer] = (1<<(layer+KBD_MAX_COLUMNS+KBD_MAX_LINES)) | (1<<(column+KBD_MAX_LINES));
}

void kbd_register_key(kbd_t* kbd, int key, int column, int line, int mod_mask) {
    CHIPS_ASSERT(kbd);
    CHIPS_ASSERT((key >= 0) && (key < KBD_MAX_KEYS));
    CHIPS_ASSERT((column >= 0) && (column < KBD_MAX_COLUMNS));
    CHIPS_ASSERT((line >= 0) && (line < KBD_MAX_LINES));
    kbd->key_masks[key] = (mod_mask << (KBD_MAX_COLUMNS+KBD_MAX_LINES)) | (1<<(column+KBD_MAX_LINES)) | (1<<line);
}

// extract column bits from a 32-bit key mask
static uint16_t _kbd_columns(uint32_t key_mask) {
    return (key_mask>>KBD_MAX_LINES) & ((1<<KBD_MAX_COLUMNS)-1);
}

// extract line bits from a 32-bit key mask
static uint16_t _kbd_lines(uint32_t key_mask) {
    return key_mask & ((1<<KBD_MAX_LINES)-1);
}

// extract modifier mask bits from a 32-bit key mask
static uint32_t _kbd_mod(uint32_t key_mask) {
    return key_mask & ((1<<KBD_MAX_MOD_KEYS)-1)<<(KBD_MAX_COLUMNS+KBD_MAX_LINES);
}

// internal function to scan keyboard lines by column mask, SLOW!
static uint16_t _kbd_test_lines(kbd_t* kbd, uint16_t column_mask) {
    CHIPS_ASSERT(kbd);
    uint16_t line_bits = 0;
    for (int key_index = 0; key_index < KBD_MAX_PRESSED_KEYS; key_index++) {
        const uint32_t key_mask = kbd->key_buffer[key_index].mask;
        if (key_mask) {
            const uint16_t key_col_mask = _kbd_columns(key_mask);
            if ((key_col_mask & column_mask) == key_col_mask) {
                line_bits |= _kbd_lines(key_mask);
            }
            const uint32_t key_mod_mask = _kbd_mod(key_mask);
            if (key_mod_mask) {
                for (int mod_index = 0; mod_index < KBD_MAX_MOD_KEYS; mod_index++) {
                    const uint32_t mod_mask = kbd->mod_masks[mod_index];
                    if (mod_mask & key_mod_mask) {
                        const uint16_t mod_col_mask = _kbd_columns(mod_mask);
                        if (mod_col_mask) {
                            if ((mod_col_mask & column_mask) == mod_col_mask) {
                                line_bits |= _kbd_lines(mod_mask);
                            }
                        }
                        else {
                            line_bits |= _kbd_lines(mod_mask);
                        }
                    }
                }
            }
        }
    }
    return line_bits;
}

// internal function to scan keyboard rows by column mask, SLOW!
static uint16_t _kbd_test_columns(kbd_t* kbd, uint16_t line_mask) {
    CHIPS_ASSERT(kbd);
    uint16_t column_bits = 0;
    for (int key_index = 0; key_index < KBD_MAX_PRESSED_KEYS; key_index++) {
        const uint32_t key_mask = kbd->key_buffer[key_index].mask;
        if (key_mask) {
            const uint16_t key_line_mask = _kbd_lines(key_mask);
            if ((key_line_mask & line_mask) == key_line_mask) {
                column_bits |= _kbd_columns(key_mask);
            }
            const uint32_t key_mod_mask = _kbd_mod(key_mask);
            if (key_mod_mask) {
                for (int mod_index = 0; mod_index < KBD_MAX_MOD_KEYS; mod_index++) {
                    const uint32_t mod_mask = kbd->mod_masks[mod_index];
                    if (mod_mask & key_mod_mask) {
                        const uint16_t mod_line_mask = _kbd_lines(mod_mask);
                        if (mod_line_mask) {
                            if ((mod_line_mask & line_mask) == mod_line_mask) {
                                column_bits |= _kbd_columns(mod_mask);
                            }
                        }
                        else {
                            column_bits |= _kbd_columns(mod_mask);
                        }
                    }
                }
            }
        }
    }
    return column_bits;
}

// update the scanout column- and line-masks, SLOW!
static void _kbd_update_scanout_masks(kbd_t* kbd) {
    for (int line = 0; line < KBD_MAX_LINES; line++) {
        kbd->scanout_column_masks[line] = _kbd_test_columns(kbd, (1<<line));
    }
    for (int col = 0; col < KBD_MAX_COLUMNS; col++) {
        kbd->scanout_line_masks[col] = _kbd_test_lines(kbd, (1<<col));
    }
    kbd->cur_column_mask = 0;
    kbd->cur_scanout_line_mask = 0;
    kbd->cur_line_mask = 0;
    kbd->cur_scanout_column_mask = 0;
}

void kbd_update(kbd_t* kbd, uint32_t frame_time_us) {
    CHIPS_ASSERT(kbd);
    // check for sticky keys that should be released
    for (int i = 0; i < KBD_MAX_PRESSED_KEYS; i++) {
        key_state_t* k = &kbd->key_buffer[i];
        if (k->released) {
            // properly handle cur_time wraparound
            if ((kbd->cur_time < k->pressed_time) ||
                (kbd->cur_time > (k->pressed_time + kbd->sticky_time))) {
                k->mask = 0;
                k->key = 0;
                k->pressed_time = 0;
                k->released = false;
            }
        }
    }
    kbd->cur_time += frame_time_us;
    _kbd_update_scanout_masks(kbd);
}

void kbd_key_down(kbd_t* kbd, int key) {
    CHIPS_ASSERT(kbd && (key >= 0) && (key < KBD_MAX_KEYS));
    /* first check if the key is already in the key buffer,
       if yes, just update its pressed-frame and return,
       otherwise find a new, free keybuffer slot
    */
    for (int i = 0; i < KBD_MAX_PRESSED_KEYS; i++) {
        key_state_t* k = &kbd->key_buffer[i];
        if (k->key == key) {
            k->pressed_time = kbd->cur_time;
            _kbd_update_scanout_masks(kbd);
            return;
        }
    }
    for (int i = 0; i < KBD_MAX_PRESSED_KEYS; i++) {
        key_state_t* k = &kbd->key_buffer[i];
        if (0 == k->mask) {
            k->key = key;
            k->mask = kbd->key_masks[key];
            k->pressed_time = kbd->cur_time;
            k->released = false;
            _kbd_update_scanout_masks(kbd);
            return;
        }
    }
}

void kbd_key_up(kbd_t* kbd, int key) {
    CHIPS_ASSERT(kbd && (key >= 0) && (key < KBD_MAX_KEYS));
    // find the key in the keybuffer, just set released_frame
    for (int i = 0; i < KBD_MAX_PRESSED_KEYS; i++) {
        key_state_t* k = &kbd->key_buffer[i];
        if (key == k->key) {
            k->released = true;
        }
    }
    _kbd_update_scanout_masks(kbd);
}

// scan keyboard matrix lines by column mask
uint16_t kbd_test_lines(kbd_t* kbd, uint16_t column_mask) {
    if (column_mask != kbd->cur_column_mask) {
        uint16_t m = 0;
        for (int col = 0; col < KBD_MAX_COLUMNS; col++) {
            if (column_mask & (1<<col)) {
                m |= kbd->scanout_line_masks[col];
            }
        }
        kbd->cur_scanout_line_mask = m;
        kbd->cur_column_mask = column_mask;
    }
    return kbd->cur_scanout_line_mask;
}

// scan keyboard matrix lines by column mask
uint16_t kbd_test_columns(kbd_t* kbd, uint16_t line_mask) {
    if (line_mask != kbd->cur_line_mask) {
        uint16_t m = 0;
        for (int line = 0; line < KBD_MAX_LINES; line++) {
            if (line_mask & (1<<line)) {
                m |= kbd->scanout_column_masks[line];
            }
        }
        kbd->cur_scanout_column_mask = m;
        kbd->cur_line_mask = line_mask;
    }
    return kbd->cur_scanout_column_mask;
}

#endif /* CHIPS_IMPL */
