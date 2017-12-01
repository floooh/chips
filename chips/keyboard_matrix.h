#pragma once
/*
    keyboard_matrix.h -- keyboard matrix helpers
*/
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define KBD_MAX_COLUMNS (16)
#define KBD_MAX_LINES (16)
#define KBD_MAX_KEYS (256)
#define KBD_MAX_SHIFT_KEYS (8)
#define KBD_MAX_PRESSED_KEYS (4)

/* desc structure for kbd_setup() */
typedef struct {
    /* the number of kbd_update() ticks a key will at least remain down */
    int sticky_count;
} keyboard_matrix_desc;

/* a pressed-key state */
typedef struct {
    /* key code of the pressed key */
    int key;
    /* column- and line bits of the key, zero means 'empty slot' */
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
    /* map key ASCII code to column/line bits */
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
/* get current keyboard matrix state of all pressed keys (column<<16|lines) */
extern uint32_t kbd_get_bits(keyboard_matrix* kbd);

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
    CHIPS_ASSERT((shift_layer >= 0) && (shift_layer < KBD_MAX_SHIFT_KEYS));
    CHIPS_ASSERT((column >= 0) && (column < KBD_MAX_COLUMNS));
    CHIPS_ASSERT((line >= 0) && (line < KBD_MAX_LINES));
    kbd->shift_masks[shift_layer] = (1<<(column+16)) | (1<<line);
}

void kbd_register_key(keyboard_matrix* kbd, int key, int column, int line, int shift_layer) {
    CHIPS_ASSERT(kbd);
    CHIPS_ASSERT((key >= 0) && (key < KBD_MAX_KEYS));
    CHIPS_ASSERT((column >= 0) && (column < KBD_MAX_COLUMNS));
    CHIPS_ASSERT((line >= 0) && (line < KBD_MAX_LINES));
    CHIPS_ASSERT((shift_layer >= 0) && (shift_layer < KBD_MAX_SHIFT_KEYS));
    kbd->key_masks[key] = (1<<(column+16)) | (1<<line) | kbd->shift_masks[shift_layer];
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

uint32_t kbd_get_bits(keyboard_matrix* kbd) {
    CHIPS_ASSERT(kbd);
    uint32_t bits = 0;
    for (int i = 0; i < KBD_MAX_PRESSED_KEYS; i++) {
        bits |= kbd->key_buffer[i].mask;
    }
    return bits;
}
#endif /* CHIPS_IMPL */

#ifdef __cplusplus
} /* extern "C" */
#endif
