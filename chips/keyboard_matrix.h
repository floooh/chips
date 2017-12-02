#pragma once
/*
    keyboard_matrix.h -- keyboard matrix helpers


    A keyboard_matrix instance maps key codes to the cross-sections of
    an up to 12x12 keyboard matrix with up to 4 shift keys.

        C0  C1  C1  C2  C3 ... C12
         /   /   /   /   /       |
    L0---+---+---+---+---+-....--+
         |   /   /   /   /       /
    L1---+---+---+---+---+-....--+
         .   .   .   .   .       .
    L12--+---+---+---+---+-....--+

    First register host-system key codes with the keyboard matrix (how are keys
    positioned in the matrix) by calling kbd_register_shift() and
    kbd_register_key().

    Feed 'host system key presses' into the keyboard_matrix instance
    by calling kbd_key_down() and kbd_key_up(). Some emulated systems
    took quite long to scan a key press, so the keyboard matrix has
    a 'sticky count', which may prolong the key press visible to the
    emulated system if the host-system key press was too short.

    Call the function kbd_test_lines() to check the current state of the
    keyboard matrix.
*/
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define KBD_MAX_COLUMNS (12)
#define KBD_MAX_LINES (12)
#define KBD_MAX_SHIFT_KEYS (4)
#define KBD_MAX_KEYS (256)
#define KBD_MAX_PRESSED_KEYS (4)

/* desc structure for kbd_setup() */
typedef struct {
    /* the number of kbd_update() ticks a key will at least remain down, default is 2 */
    int sticky_count;
} keyboard_matrix_desc;

/* a pressed-key state */
typedef struct {
    /* key code of the pressed key */
    int key;
    /* mask bit layout is 8-bits shift, and 12-bits each columns and lines */
    /* |SSSSSSSS|CCCCCCCCCCCC|LLLLLLLLLLLL| */
    uint32_t mask;
    /* the frame-count when the key was pressed down */
    uint32_t pressed_frame;
    /* the frame-count when the key was released, 0 if not yet released */
    uint32_t released_frame;
} key_state;

/* keyboard matrix state */
typedef struct {
    /* current frame counter, bumped by kbd_update() */
    uint32_t frame_count;
    /* number of frames a key will at least remain pressed */
    int sticky_count;
    /* map key ASCII code to shift/column/line bits */
    uint32_t key_masks[KBD_MAX_KEYS];
    /* column/line bits of up to 8 shift keys */
    uint32_t shift_masks[KBD_MAX_SHIFT_KEYS];
    /* currently pressed keys (bitmask==0 is empty slot) */
    key_state key_buffer[KBD_MAX_PRESSED_KEYS];
} keyboard_matrix;

/* initialize a keyboard matrix instance */
extern void kbd_init(keyboard_matrix* kbd, keyboard_matrix_desc* desc);
/* update keyboard matrix state (releases sticky keys), usually call once per frame */
extern void kbd_update(keyboard_matrix* kbd);
/* register a shift key, shift_layer 0 is reserved! */
extern void kbd_register_shift(keyboard_matrix* kbd, int shift_layer, int column, int line);
/* register a key */
extern void kbg_register_key(keyboard_matrix* kbd, int key, int column, int line, int shift_layer);
/* add a key to the pressed-key buffer */
extern void kbd_key_down(keyboard_matrix* kbd, int key);
/* remove a key from the pressed-key buffer */
extern void kbd_key_up(keyboard_matrix* kbd, int key);
/* activate columns and return which lines are lit */
extern uint16_t kbd_test_lines(keyboard_matrix* kbd, uint16_t column_mask);

/*-- IMPLEMENTATION ----------------------------------------------------------*/
#ifdef CHIPS_IMPL
#include <string.h>
#ifndef CHIPS_DEBUG
    #ifdef _DEBUG
        #define CHIPS_DEBUG
    #endif
#endif
#ifndef CHIPS_ASSERT
    #include <assert.h>
    #define CHIPS_ASSERT(c) assert(c)
#endif

void kbd_init(keyboard_matrix* kbd, keyboard_matrix_desc* desc) {
    CHIPS_ASSERT(kbd && desc);
    memset(kbd, 0, sizeof(keyboard_matrix));
    kbd->frame_count = 1;
    kbd->sticky_count = (desc->sticky_count == 0) ? 2 : desc->sticky_count;
}

void kbd_update(keyboard_matrix* kbd) {
    CHIPS_ASSERT(kbd);
    kbd->frame_count++;
    /* check for sticky keys that should be released */
    for (int i = 0; i < KBD_MAX_PRESSED_KEYS; i++) {
        key_state* k = &kbd->key_buffer[i];
        if (k->released_frame != 0) {
            if (kbd->frame_count > (k->pressed_frame + kbd->sticky_count)) {
                k->mask = 0;
                k->key = 0;
                k->pressed_frame = 0;
                k->released_frame = 0;
            }
        }
    }
}

void kbd_register_shift(keyboard_matrix* kbd, int shift_layer, int column, int line) {
    CHIPS_ASSERT(kbd);
    CHIPS_ASSERT((column >= 0) && (column < KBD_MAX_COLUMNS));
    CHIPS_ASSERT((line >= 0) && (line < KBD_MAX_LINES));
    CHIPS_ASSERT((shift_layer >= 1) && (shift_layer < (KBD_MAX_SHIFT_KEYS+1)));
    const int shift_index = shift_layer - 1;
    kbd->shift_masks[shift_index] = (1<<(shift_index+KBD_MAX_COLUMNS+KBD_MAX_LINES)) | (1<<(column+KBD_MAX_COLUMNS)) | (1<<line);
}

void kbd_register_key(keyboard_matrix* kbd, int key, int column, int line, uint8_t shift_mask) {
    CHIPS_ASSERT(kbd);
    CHIPS_ASSERT((key >= 0) && (key < KBD_MAX_KEYS));
    CHIPS_ASSERT((column >= 0) && (column < KBD_MAX_COLUMNS));
    CHIPS_ASSERT((line >= 0) && (line < KBD_MAX_LINES));
    kbd->key_masks[key] = (shift_mask << (KBD_MAX_COLUMNS+KBD_MAX_LINES)) | (1<<(column+KBD_MAX_COLUMNS)) | (1<<line);
}

void kbd_key_down(keyboard_matrix* kbd, int key) {
    CHIPS_ASSERT(kbd && (key >= 0) && (key < KBD_MAX_KEYS));
    /* find a free keybuffer slot */
    for (int i = 0; i < KBD_MAX_PRESSED_KEYS; i++) {
        key_state* k = &kbd->key_buffer[i];
        if (0 == k->mask) {
            k->key = key;
            k->mask = kbd->key_masks[key];
            k->pressed_frame = kbd->frame_count;
            k->released_frame = 0;
            return;
        }
    }
}

void kbd_key_up(keyboard_matrix* kbd, int key) {
    CHIPS_ASSERT(kbd && (key >= 0) && (key < KBD_MAX_KEYS));
    /* find the key in the keybuffer, just set released_frame */
    for (int i = 0; i < KBD_MAX_PRESSED_KEYS; i++) {
        key_state* k = &kbd->key_buffer[i];
        if (key == k->key) {
            k->released_frame = kbd->frame_count;
        }
    }
}

/* extract column bits from a 32-bit key mask */
static uint16_t _kbd_columns(uint32_t key_mask) {
    return (key_mask>>KBD_MAX_COLUMNS) & ((1<<KBD_MAX_COLUMNS)-1);
}

/* extract line bits from a 32-bit key mask */
static uint16_t _kbd_lines(uint32_t key_mask) {
    return key_mask & ((1<<KBD_MAX_LINES)-1);
}

/* extract shift mask bits from a 32-bit key mask */
static uint32_t _kbd_shift(uint32_t key_mask) {
    return key_mask & ((1<<KBD_MAX_SHIFT_KEYS)-1)<<(KBD_MAX_COLUMNS+KBD_MAX_LINES);
}

/* scan keyboard matrix */
uint16_t kbd_test_lines(keyboard_matrix* kbd, uint16_t column_mask) {
    CHIPS_ASSERT(kbd);
    uint16_t line_bits = 0;
    for (int key_index = 0; key_index < KBD_MAX_PRESSED_KEYS; key_index++) {
        const uint32_t key_mask = kbd->key_buffer[key_index].mask;
        if (key_mask) {
            const uint16_t key_col_mask = _kbd_columns(key_mask);
            if ((key_col_mask & column_mask) == key_col_mask) {
                line_bits |= _kbd_lines(key_mask);
            }
            const uint32_t key_shift_mask = _kbd_shift(key_mask);
            if (key_shift_mask) {
                for (int shift_index = 0; shift_index < KBD_MAX_SHIFT_KEYS; shift_index++) {
                    const uint32_t shift_mask = kbd->shift_masks[shift_index];
                    if (shift_mask & key_shift_mask) {
                        const uint16_t shift_col_mask = _kbd_columns(shift_mask);
                        if ((shift_col_mask & column_mask) == (shift_col_mask)) {
                            line_bits |= _kbd_lines(shift_mask);
                        }
                    }
                }
            }
        }
    }
    return line_bits;
}
#endif /* CHIPS_IMPL */

#ifdef __cplusplus
} /* extern "C" */
#endif
