#pragma once
/*#
    # c1530.h

    The Commodore datasette tape drive in a header for loading .TAP files.

    Do this:
    ~~~C
    #define CHIPS_IMPL
    ~~~
    before you include this file in *one* C or C++ file to create the
    implementation.

    Optionally provide the following macros with your own implementation
    ~~~C
    CHIPS_ASSERT(c)
    ~~~
        your own assert macro (default: assert(c))

    You need to include the following headers before including c64.h:

    ## Howto

    The C1530 can be used with system emulators with support for the
    Commodore cassette port (so far vic20.h and c64.h). The system
    emulators expose the cassette port through a byte called 'cas_port'. The
    C1530 emulators 'connects' to this cassette port through a pointer
    to this cas_port byte, so that this byte becomes shared between
    the computer emulator and the C1530 emulator.

    To setup a c1530_t instance, call c1530_init() and provide a pointer
    to the shared cassette port byte of the computer system:

    ~~~C
    c1530_init(&c1530, &(c1530_desc_t){
        .cas_port = &c64.cas_port
    });
    ~~~

    For each computer system tick, call the c1530_tick() function once too:

    ~~~C
    for (uint32_t ticks = 0; ticks < num_ticks; ticks++) {
        c64_tick(&c64);
        c1530_tick(&c1530);
    }
    ~~~

    Use the following functions to insert and remove a tape, with a .TAP
    file loading into memory, or check if a tape is inserted:

    ~~~C
    bool c1530_insert_tape(c1530_t* sys, const uint8_t* ptr, int num_bytes);
    void c1530_remove_tape(c1530_t* sys);
    bool c1530_tape_inserted(c1530_t* sys);
    ~~~

    Call the following functions to control the tape motor (press the Play
    or Stop buttons):

    ~~~C
    void c1530_play(c1530_t* sys);
    void c1530_stop(c1530_t* sys);
    bool c1530_is_motor_on(c1530_t* sys);
    ~~~

    The motor may also be switched on/off by the computer system through
    the cassette port's MOTOR pin.

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
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* casette port bits, same as C64_CASPORT_* */
#define C1530_CASPORT_MOTOR   (1<<0)
#define C1530_CASPORT_READ    (1<<1)
#define C1530_CASPORT_WRITE   (1<<2)
#define C1530_CASPORT_SENSE   (1<<3)

/* max size of a cassette tape image */
#define C1530_MAX_TAPE_SIZE (512*1024)

/* config params for c1530_init() */
typedef struct {
    /* pointer to a the C64's cassette port byte */
    uint8_t* cas_port;
} c1530_desc_t;

/* 1530 drive state */
typedef struct {
    uint8_t* cas_port;  /* pointer to shared C64 cassette port state */
    bool valid;         /* true between c1530_init() and c1530_discard() */
    uint32_t size;      /* tape_size > 0: a tape is inserted */
    uint32_t pos;
    uint32_t pulse_count;
    uint8_t buf[C1530_MAX_TAPE_SIZE];
} c1530_t;

/* initialize a c1530_t instance */
void c1530_init(c1530_t* sys, const c1530_desc_t* desc);
/* discard a c1530_t instance */
void c1530_discard(c1530_t* sys);
/* reset a c1530_t instance */
void c1530_reset(c1530_t* sys);
/* tick the tape drive */
void c1530_tick(c1530_t* sys);
/* insert a tape file */
bool c1530_insert_tape(c1530_t* sys, chips_range_t data);
/* remove tape file */
void c1530_remove_tape(c1530_t* sys);
/* return true if a tape is currently inserted */
bool c1530_tape_inserted(c1530_t* sys);
/* start the tape (press the Play button) */
void c1530_play(c1530_t* sys);
/* stop the tape (unpress the Play button */
void c1530_stop(c1530_t* sys);
/* return true if tape motor is on */
bool c1530_is_motor_on(c1530_t* sys);
// prepare c1530_t snapshot for saving
void c1530_snapshot_onsave(c1530_t* snapshot);
// fixup c1530_t snapshot after loading
void c1530_snapshot_onload(c1530_t* snapshot, c1530_t* sys);

#ifdef __cplusplus
} /* extern "C" */
#endif

/*-- IMPLEMENTATION ----------------------------------------------------------*/
#ifdef CHIPS_IMPL
#include <string.h> /* memcpy, memset */
#ifndef CHIPS_ASSERT
    #include <assert.h>
    #define CHIPS_ASSERT(c) assert(c)
#endif

void c1530_init(c1530_t* sys, const c1530_desc_t* desc) {
    CHIPS_ASSERT(sys && desc);
    CHIPS_ASSERT(desc->cas_port);
    memset(sys, 0, sizeof(c1530_t));
    sys->valid = true;
    sys->cas_port = desc->cas_port;
}

void c1530_discard(c1530_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    sys->valid = false;
}

void c1530_reset(c1530_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    sys->cas_port = 0;
    sys->size = 0;
    sys->pos = 0;
    sys->pulse_count = 0;
}

/* C64 TAP file header */
typedef struct {
    uint8_t signature[12];  /* "C64-TAPE-RAW" */
    uint8_t version;        /* 0x00 or 0x01 */
    uint8_t pad[3];         /* reserved */
    uint32_t size;          /* size of the following data */
} _c1530_tap_header;

bool c1530_insert_tape(c1530_t* sys, chips_range_t data) {
    CHIPS_ASSERT(sys && sys->valid && data.ptr && (data.size > 0));
    c1530_remove_tape(sys);
    if (data.size <= sizeof(_c1530_tap_header)) {
        return false;
    }
    const uint8_t* ptr = (uint8_t*) data.ptr;
    const _c1530_tap_header* hdr = (const _c1530_tap_header*) ptr;
    ptr += sizeof(_c1530_tap_header);
    const uint8_t sig[12] = { 'C','6','4','-','T','A','P','E','-','R','A','W'};
    for (size_t i = 0; i < 12; i++) {
        if (sig[i] != hdr->signature[i]) {
            return false;
        }
    }
    if (1 != hdr->version) {
        return false;
    }
    if (data.size < (hdr->size + sizeof(_c1530_tap_header))) {
        return false;
    }
    if (data.size > sizeof(sys->buf)) {
        return false;
    }
    memcpy(sys->buf, ptr, hdr->size);
    sys->size = hdr->size;
    sys->pos = 0;
    sys->pulse_count = 0;
    return true;
}

void c1530_play(c1530_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    /* motor on, play button down */
    *sys->cas_port &= ~(C1530_CASPORT_MOTOR|C1530_CASPORT_SENSE);
}

void c1530_stop(c1530_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    /* motor off, play button up */
    *sys->cas_port |= (C1530_CASPORT_MOTOR|C1530_CASPORT_SENSE);
}

bool c1530_is_motor_on(c1530_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    return 0 == (*sys->cas_port & C1530_CASPORT_MOTOR);
}

void c1530_remove_tape(c1530_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    c1530_stop(sys);
    sys->size = 0;
    sys->pos = 0;
    sys->pulse_count = 0;
}

bool c1530_tape_inserted(c1530_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    return sys->size > 0;
}

void c1530_tick(c1530_t* sys) {
    CHIPS_ASSERT(sys && sys->valid);
    *sys->cas_port &= ~C1530_CASPORT_READ;
    if (c1530_is_motor_on(sys) && (sys->size > 0) && (sys->pos <= sys->size)) {
        if (sys->pulse_count == 0) {
            uint8_t val = sys->buf[sys->pos++];
            if (val == 0) {
                uint8_t s[3];
                for (int i = 0; i < 3; i++) {
                    s[i] = sys->buf[sys->pos++];
                }
                sys->pulse_count = (s[2]<<16) | (s[1]<<8) | s[0];
            }
            else {
                sys->pulse_count = val * 8;
            }
            *sys->cas_port |= C1530_CASPORT_READ;
        }
        else {
            sys->pulse_count--;
        }
    }
}

void c1530_snapshot_onsave(c1530_t* snapshot) {
    CHIPS_ASSERT(snapshot);
    snapshot->cas_port = 0;
}

void c1530_snapshot_onload(c1530_t* snapshot, c1530_t* sys) {
    CHIPS_ASSERT(snapshot && sys);
    snapshot->cas_port = sys->cas_port;
}

#endif /* CHIPS_IMPL */
