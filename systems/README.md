# Embeddable 8-bit Emulators

This directory contains header-only 8-bit home computer emulators
written in C (more specifically, a subset of C99 that compiles
on gcc, clang and cl.exe both in C and C++ mode).

The emulators don't contain any platform-specific code and
are easy to embed into bigger applications.

## Emulated systems:

- **z1013.h**: the Robotron Z1013 in the following variants:
    - *Z1013.01*: the original Z1013 with 1 MHz Z80 CPU, 16 KB RAM, "old" operating system ROM and 8x4 keyboard matrix with 4 shift keys
    - *Z1013.16*: the modernised Z1013 with 2 MHz CPU, 16 KB RAM, "new" operating system ROM and an 8x8 keyboard matrix for 'industry standard' keyboards
    - *Z1013.64*: same as the Z1013.16 model, but with 64 KByte RAM
- **z9001.h**: the Robotron Z9001 aka KC85/1 in 2 versions:
    - *Z9001*: the original Z9001 which was later renamed to KC85/1 with black-and-white display, a 16 KByte RAM extension (for 32 KByte RAM overall), and an optional BASIC module
    - *KC87*: the improved KC87 with color module, 48 KByte RAM and builtin BASIC ROM
- **zx.h**: the ZX Spectrum in 2 variants:
    - *ZX Spectrum 48k*: 48 KB RAM and beeper sound
    - *ZX Spectrum 128*: the improved Spectrum with 128 KB RAM and an additional AY-3-8912 sound chip
- **cpc.h**: the Amstrad CPC in 3 variants:
    - *CPC 464*: the original 64 KByte Amstrad CPC
    - *CPC 6128*: the improved CPC with 128 KB RAM
    - *KC Compact*: an East German CPC clone (the emulator only differs in color palette and ROM, it doesn't emulate the gate-array 'hardware-emulation' in the KC Compact
- **atom.h**: an *Acorn Atom* with modern extensions (32 KB RAM + 8 KB video memory, a rudimentary VIA 6522 emulation and MMC joystick support)
- (TODO) **c64.h**: a *C64* emulator with PAL display
- (TODO) **kc85.h**: an emulator for 3 KC85 models from VEB Mikroelektronik Mühlhausen:
    - *KC85/2*: the original KC85 model with 16 KB RAM, CAOS 2.2 and no integrated BASIC
    - *KC85/3*: an incremental improvement with CAOS 3.1 and integrated BASIC ROM
    - *KC85/4*: a vastly improved model with 128 KB RAM, higher color resolution and double buffered display 

The accuracy of emulation differs quite a bit, please refer to the embedded documentation
in the header files for details.

## General properties of the emulator headers

- All headers are *STB-style* with a *declaration part* that's always
parsed, and an *implementation part* that's only parsed when ```CHIPS_IMPL```
is defined in one source file before including the headers
- The declaration part only includes the following CRT headers:
    - stdint.h (for integer typedefs, uint32_t, etc...)
    - stdbool.h (for the bool typedef)
- The implementation part will additionally include:
    - string.h (for memcpy and memset)
    - assert.h (unless the CHIPS_ASSERT macro is overridden with your own assert implementation)
- Except for memset() and memcpy(), no C runtime functions are called in the headers (most importantly, there's no memory allocation or file I/O happening).

## Embedding Guide

The following embedding guide will use the **cpc.h** header (Amstrad CPC) as an
example, other emulators only differ in details which are explained in the
emulator-specific documentation embedded in the respective headers.

In general, you provide:

- system ROM dumps in memory
- keyboard input events
- a memory chunk used as shared RGBA8 framebuffer
- a callback function which receives small packets of audio samples
- optional snapshot-, tape- or disk-images in memory to load into the emulator

The emulator provides:

- a continuously updated RGBA8 video image in the shared framebuffer
- audio data as a stream of 32-bit float samples in the range -1.0..1.0

The embedding app will normally do the following per 60Hz-frame:

- tell the emulator to execute the right number of ticks for the frame duration
- provide keyboard input
- render the framebuffer

While 'ticking', the emulator will call your audio callback whenever it has
finished generating a new packet of (usually) 128 audio samples 

## Platform Abstraction

The emulator headers themselves don't care how you're getting keyboard input
from the host platform,
or how rendering and audio playback happens, this must be implemented by your
embedding code.

The example embedding uses the following portability headers for this:

- [sokol_app.h](https://github.com/floooh/sokol/blob/master/sokol_app.h): as cross-platform application-wrapper (app entry, window- and 3D-context creation, and input)
- [sokol_gfx.h](https://github.com/floooh/sokol/blob/master/sokol_gfx.h): to copy the emulator's framebuffer into a texture and render it through a 3D-API (using GL, D3D11 or Metal)
- [sokol_audio.h](https://github.com/floooh/sokol/blob/master/sokol_audio.h): to stream the generated audio samples into a platform-specific audio backend
- [sokol_time.h](https://github.com/floooh/sokol/blob/master/sokol_time.h): to measure the elapsed time between host-system frames

## The ```common``` helper-function library

The above low-level platform-abstraction-headers are further wrapped into a small library called
*common* which all example emulators link against, consisting of the following
headers:

- **args.h**: argument parser for cmdline and URL args
- **clock.h**: measure frame time and count frames
- **fs.h**: load files into memory via fopen() or HTTP
- **gfx.h**: small wrapper over sokol_gfx.h to render the emulator framebuffer
- **keybuf.h**: split a string into a sequence of key strokes to send into the emulator

Any function names in the following code starting with *args_*, *clock_*, *fs_*, *gfx_* and *keybuf_* are part of the common-library and implemented in these headers.

It's important to understand that these functions are neither required, nor
part of the **chips** project. While your own embedding code most likely will
implement similar helper functions, their specific APIs and implementations might look
entirely different!

### Step 1: Header includes and app-skeleton

First let's include the right headers:

```c
#include "common/common.h"
#define CHIPS_IMPL
#include "chips/z80.h"
#include "chips/ay38910.h"
#include "chips/i8255.h"
#include "chips/mc6845.h"
#include "chips/crt.h"
#include "chips/clk.h"
#include "chips/kbd.h"
#include "chips/mem.h"
#include "systems/cpc.h"
#include "roms/cpc-roms.h"
```

```common/common.h``` includes the helper function headers (the stuff
mentioned above starting with *args_*, *clock_*, *fs_*, *gfx_* and
*keybuf_*).

Next are the chips-headers that are needed by **cpc.h**. The list of required
headers can be looked up in the embedded documentation in the system emulator
headers. The reason why **cpc.h** doesn't include those headers directly is
that this gives you more freedom where you put those headers in your own
project.

The ```#define CHIPS_IMPL``` tells the headers that the implementation
part should be included. Again, this is a detail that's very specific to the
example emulators, in your own code you might decide to place the header implementations
into another source file.

**cpc.h** needs the following chip emulator headers:

- **chips/z80.h**: the Zilog Z80 CPU emulator
- **chips/ay38910.h**: the General Instrument AY-3-8910 sound-chip emulator
(the CPC actually uses the AY-3-8912 which only has one IO port, this is
supported as a subtype by the ay38910.h emulator)
- **chips/i8255**: the Intel 8255 'Programmable Peripheral Interface' chip,
this is used on the CPC for interfacing the keyboard, sound chip and tape-deck
- **chips/mc6845.h**: the Motorola MC6845 video address generator chip, this
creates the timing- and address-signals for the CPC custom gate array chip,
which in turn generates the video signal

A number of 'subsystem headers' are required which don't emulate
a specific microchip, but instead offer generic functionality needed by
most 8-bit home computer emulators:

- **chips/crt.h**: this provides beam-position- and timing-information for a
PAL- or NTSC cathode-ray-tube
- **chips/clk.h**: converts a duration into a number of ticks, and keeps track of
'overflow ticks' (when the CPU has executed more ticks than requested, those
overflow-ticks must be taken into account when ticking the CPU next, otherwise
the emulated system would run slightly faster than intended)
- **chips/kbd.h**: helper functions to manage a generic keyboard matrix
 (the CPC has a fairly straight-forward 10x8 keyboard matrix)
- **chips/mem.h**: implements a memory subsystem to map 16-bit emulator
addresses to host system addresses using page-tables, and access the
memory as RAM, ROM or RAM-behind-ROM.

After the chips-headers, the actual CPC emulator header **systems/cpc.h** is
included, and after that the ROM images (I have converted the actual ROM
dumps to embedded C arrays using a python code generator script, of course
you're free to provide the ROM images in different ways, as long as they're
in memory when **cpc_init()** is called.

The CPC emulator state is in a single global variable:

```cpp
cpc_t cpc;
```

You're also free put this variable into a heap allocation or on the stack,
but for stack variables be aware that the emulator state can be up to few
hundred KBytes because the memory, ROM images and tape/disk drive buffers are
part of the emulator state. On some platforms this may be too big for a
stack variable.

Next the empty application skeleton, this is using the **sokol_app.h**
header, it will look different on other platform wrappers (like SDL):

```cpp
sapp_desc sokol_main(int argc, char* argv[]) {
    ...
}

void app_init(void) {
    ...
}

void app_frame(void) {
    ...
}

void app_input(const sapp_event* event) {
    ...
}

void app_cleanup(void) {
    ...
}
```

Now let's implement those functions one by one:

## sokol_main()

The **sokol_main()** function is called by sokol_app.h right after the
application is started, and should only do minimal initialization work
(like parsing command line arguments). The main job of sokol_main()
is to return a **sapp_desc** struct with initialization attributes
for sokol_app.h:

```cpp
sapp_desc sokol_main(int argc, char* argv[]) {
    args_init(argc, argv);
    return (sapp_desc) {
        .init_cb = app_init,
        .frame_cb = app_frame,
        .event_cb = app_input,
        .cleanup_cb = app_cleanup,
        .width = CPC_DISPLAY_WIDTH,
        .height = 2 * CPC_DISPLAY_HEIGHT,
        .window_title = "CPC 6128",
    };
}
```

The call to **args_init()** initializes our own little argument-parsing 
helper module that's part of the **common** library.

The returned **sapp_desc** struct tells sokol_app.h what functions it should
call for initialization (**app_init()**), per-frame work (**app_frame()**),
notifying the application about input events (**app_input()**), and
cleanup/shutdown (**app_cleanup()**). The next three attributes **width**,
**height** and **window_title** describe the application window (note that the CPC
emulator has non-square pixels since its highest pixel resolution is
640x200, that's why the vertical size of the application window is doubled).

After **sokol_main()** returns, sokol_app.h will create a window and 3D-API
context, and then proceed by calling the initialization callback function
(which in our case is **app_init()**):

## app_init()

This is where all the one-time-initialization happens. We're doing a bit more here
than what a minimal emulator would need.

First some more subsystem initialization:

```cpp
void app_init(void) {
    gfx_init(&(gfx_desc_t) {
        .fb_width = CPC_DISPLAY_WIDTH,
        .fb_height = CPC_DISPLAY_HEIGHT,
        .aspect_y = 2
    });
    clock_init();
    saudio_setup(&(saudio_desc){0});
    fs_init();
    ...
}
```

**gfx_init()** is part of our little **common** helper lib and initializes 
everything necessary to render a textured fullscreen rectangle via
sokol_gfx.h.

**clock_init()** is also part of the **common** helper lib, the clock functions
measure the current frame time, and keep track of a counter which is 
incremented each frame.

**saudio_setup()** is a direct call to the sokol_audio.h initialization
function. sokol_audio.h will be used to play the audio samples generated by
the emulator.

And finally **fs_init()** (also part of the **common** lib) initializes 
some file loading helper functions.

Next we'll check if a file should be loaded into the emulator. The CPC
emulator supports two types of file loading: snapshot files
which can be loaded immediately (with the extensions .sna and .bin), 
and tape files (with the extension .tap) which are loaded through the
tape drive emulation and take longer to load:

```cpp
    if (args_has("file")) {
        fs_load_file(args_string("file"));
    }
    if (args_has("tape")) {
        fs_load_file(args_string("tape"));
    }
```
This basically means: if there's a cmdline argument of the form ```file=path```, 
or a command line argument ```tape=path```, load the file at _path_ into
memory.

After that some more command line options to configure the emulated CPC:

```cpp
    cpc_type_t type = CPC_TYPE_6128;
    if (args_has("type")) {
        if (args_string_compare("type", "cpc464")) {
            type = CPC_TYPE_464;
        }
        else if (args_string_compare("type", "kccompact")) {
            type = CPC_TYPE_KCCOMPACT;
        }
    }
    cpc_joystick_t joy_type = CPC_JOYSTICK_NONE;
    if (args_has("joystick")) {
        joy_type = CPC_JOYSTICK_DIGITAL;
    }
```

The CPC emulation currently supports three models: the original *CPC464*,
the *CPC6128*, and the more exotic *KC Compact* (an East German CPC clone).
These can be selected with the command line options ```type=cpc464```, ```type=kccompact```, and by default, the CPC 6128 will be selected.

The CPC's digital joystick emulation can be activated with ```joystick=*```.
With activated joystick emulation, the key codes for left/right/up/down/space
are not interpreted as key presses, but as joystick buttons.

Other emulators (for instance the ZX Spectrum) may support different
joystick types here.

Finally the CPC is initialized with a call to **cpc_init()**:

```cpp
    cpc_init(&cpc, &(cpc_desc_t){
        .type = type,
        .joystick_type = joy_type,
        .pixel_buffer = gfx_framebuffer(),
        .pixel_buffer_size = gfx_framebuffer_size(),
        .audio_cb = push_audio,
        .audio_sample_rate = saudio_sample_rate(),
        .rom_464_os = dump_cpc464_os,
        .rom_464_os_size = sizeof(dump_cpc464_os),
        .rom_464_basic = dump_cpc464_basic,
        .rom_464_basic_size = sizeof(dump_cpc464_basic),
        .rom_6128_os = dump_cpc6128_os,
        .rom_6128_os_size = sizeof(dump_cpc6128_os),
        .rom_6128_basic = dump_cpc6128_basic,
        .rom_6128_basic_size = sizeof(dump_cpc6128_basic),
        .rom_6128_amsdos = dump_cpc6128_amsdos,
        .rom_6128_amsdos_size = sizeof(dump_cpc6128_amsdos),
        .rom_kcc_os = dump_kcc_os,
        .rom_kcc_os_size = sizeof(dump_kcc_os),
        .rom_kcc_basic = dump_kcc_bas,
        .rom_kcc_basic_size = sizeof(dump_kcc_bas)
    });
```

That's quite a mouthful so let's look at it in detail:

First the CPC model and joystick type as defined by command line args, not much to see here:
```cpp
        .type = type,
        .joystick_type = joy_type,
```

Next, the emulator needs to know about the location and size of the shared
memory chunk that's used as display frame buffer:

```cpp
        .pixel_buffer = gfx_framebuffer(),
        .pixel_buffer_size = gfx_framebuffer_size(),
```

In the example embedding, the gfx_* functions in the **common** lib manage
this chunk of memory, and I have created two functions to get a pointer to,
and the size in bytes of the this memory chunk. The cpc_init() function
will make sure that the provided size is sufficient for the CPC display through 
an assert-check.

```cpp
        .audio_cb = push_audio,
        .audio_sample_rate = saudio_sample_rate(),
```

This configures the audio output. The emulator needs a pointer to a callback
function which copies packets of audio samples generated by the
emulator into the audio backend, and the actual sample rate of the audio backend
(it's important that the emulator generates audio samples at the right
sample rate, otherwise audio glitches would occur).

The **push_audio()** callback function looks like this:

```cpp
static void push_audio(const float* samples, int num_samples, void* user_data) {
    saudio_push(samples, num_samples);
}
```
This simply forwards the audio samples to sokol_audio.h.

Finally the emulator needs to know the location and size of all required ROM images:

```cpp
        .rom_464_os = dump_cpc464_os,
        .rom_464_os_size = sizeof(dump_cpc464_os),
        .rom_464_basic = dump_cpc464_basic,
        .rom_464_basic_size = sizeof(dump_cpc464_basic),
        .rom_6128_os = dump_cpc6128_os,
        .rom_6128_os_size = sizeof(dump_cpc6128_os),
        .rom_6128_basic = dump_cpc6128_basic,
        .rom_6128_basic_size = sizeof(dump_cpc6128_basic),
        .rom_6128_amsdos = dump_cpc6128_amsdos,
        .rom_6128_amsdos_size = sizeof(dump_cpc6128_amsdos),
        .rom_kcc_os = dump_kcc_os,
        .rom_kcc_os_size = sizeof(dump_kcc_os),
        .rom_kcc_basic = dump_kcc_bas,
        .rom_kcc_basic_size = sizeof(dump_kcc_bas)
```

For each ROM image, a pointer and size (in bytes) must be provided. You
only need to provide the ROM images for the actually emulated model,
but since this is only decided during runtime in our example embedding
we simply provide all the images. The emulator will copy the required
images during cpc_init() into internal buffers, so the ROM image
memory can be thrown away after cpc_init() returns.

And that is all for initialization, on to the per-frame work:

## app_frame()

The per-frame callback at least needs to tick the emulator, and render
the generated video output image. But in our case a couple more things
are happening:

```cpp
void app_frame(void) {
    cpc_exec(&cpc, clock_frame_time());
    gfx_draw();
    /* load a data file? */
    if (fs_ptr() && clock_frame_count() > 120) {
        if (args_has("file")) {
            /* load the file data as a quickload-snapshot (SNA or BIN format)*/
            cpc_quickload(&cpc, fs_ptr(), fs_size());
        }
        else {
            /* load the file data via the tape-emulation (TAP format) */
            if (cpc_insert_tape(&cpc, fs_ptr(), fs_size())) {
                /* issue the right commands to start loading from tape */
                keybuf_put(0, 20, "|tape\nrun\"\n\n");
            }
        }
        fs_free();
    }
    uint8_t key_code;
    if (0 != (key_code = keybuf_get())) {
        cpc_key_down(&cpc, key_code);
        cpc_key_up(&cpc, key_code);
    }
}
```

First the most important call: ticking the emulator:

```cpp
    cpc_exec(&cpc, clock_frame_time());
    ...
```

The call to **clock_frame_time()** returns the measured frame time of the
last frame in microseconds, 'rounded' to either the duration of a 60Hz or 30Hz
frame (16667 or 33333 microseconds). This might seem a bit strange but
fixes a couple of minor issues:

- the emulator won't suffer from micro-stuttering (actually it's not quite
that easy since the CPC is a PAL machine which updates its display at 50Hz,
but typical modern computers update their display at 60Hz, so there will
always be a small amount of stuttering and screen tearing)
- if the application is suspended for a long time (for instance when running
in a web browser, and the browser tab is switched to the background),
the first frame after bringing the tab back to the front will be clipped to
33 milliseconds duration, that way the emulator won't try to 'catch up'
several seconds or even minutes of idle time, which would freeze
the entire application for a while)

The function **cpc_exec()** will now run the emulation for the required number of
emulator ticks (in the case of a CPC, which is running at 4 MHz, and a 
60 Hz frame rate, this would result in around 4000000 / 60 = 66667 ticks
per frame. 

During the call to cpc_exec() the emulator will generate new audio samples
and decode new pixels into the shared framebuffer memory chunk.

If enough audio samples have been created to fill an audio-packet (usually
128 samples), the audio callback function will be called from inside
cpc_exec().

After cpc_exec() returns, it's time to update the display:

```cpp
    ...
    gfx_draw();
    ...
```
This will copy the current framebuffer content into a texture
and render the texture as a 'fullscreen rectangle' through the 
*gfx_* helper functions in the **common** library.

This is already enough code for the emulator to boot up and display an image,
but keyboard input doesn't work yet, and there's no way to load data.

First let's complete the per-frame callback with support for loading
game snapshots and tape files:

```cpp
void app_frame(void) {
    cpc_exec(&cpc, clock_frame_time());
    gfx_draw();
    if (fs_ptr() && clock_frame_count() > 120) {
        if (args_has("file")) {
            /* load the file data as a quickload-snapshot (SNA or BIN format)*/
            cpc_quickload(&cpc, fs_ptr(), fs_size());
        }
        else {
            /* load the file data via the tape-emulation (TAP format) */
            if (cpc_insert_tape(&cpc, fs_ptr(), fs_size())) {
                /* issue the right commands to start loading from tape */
                keybuf_put(0, 20, "|tape\nrun\"\n\n");
            }
        }
        fs_free();
    }
    uint8_t key_code;
    if (0 != (key_code = keybuf_get())) {
        cpc_key_down(&cpc, key_code);
        cpc_key_up(&cpc, key_code);
    }
}
```

The following code block checks if file data is available, and at least 120
frames have passed (this is enough time for the CPC to boot up), and when
both conditions are met, the file is loaded into the emulator, either as 
a snapshot via **cpc_quickload()**, or as a tape file via **cpc_insert_tape()**:

```cpp
    if (fs_ptr() && clock_frame_count() > 120) {
        if (args_has("file")) {
            /* load the file data as a quickload-snapshot (SNA or BIN format)*/
            cpc_quickload(&cpc, fs_ptr(), fs_size());
        }
        else {
            /* load the file data via the tape-emulation (TAP format) */
            if (cpc_insert_tape(&cpc, fs_ptr(), fs_size())) {
                /* issue the right commands to start loading from tape */
                keybuf_put(0, 20, "|tape\nrun\"\n\n");
            }
        }
        fs_free();
    }
```

Snapshot files (provided with the ```file=path``` command line arg) are loaded
and started immediately (this is the call to the function **cpc_quickload()**),
tape files however need a bit more work:

Instead of 'loading' the file immediately, we only insert the data into
a 'virtual tape deck' with the function **cpc_insert_tape()**, and after
that send keystrokes to the emulator to start loading the
data from tape (these are the commands that a user would normally
enter by hand):

```
|TAPE[Enter]
RUN"[Enter]
[Enter]
```

This is where our little keystroke-helper lib comes in which is part of the
**common** library: this takes a string, and 'plays it back' with some frames
delay between each key stroke to allow the emulator enough time to scan the
keyboard:

```cpp
    uint8_t key_code;
    if (0 != (key_code = keybuf_get())) {
        cpc_key_down(&cpc, key_code);
        cpc_key_up(&cpc, key_code);
    }
```

Whenever the function keybuf_get() returns a non-null key code, the key-down
and key-up event will be sent to the emulator, so the following commands
appear on the CPC display as if typed in by the user:

```
|TAPE[Enter]
RUN"[Enter]
[Enter]
```

And that's all for the per-frame stuff. On to keyboard input!

## app_input()

The callback function app_input() is invoked by sokol_app.h whenever an
input event happens. We are only interested in keyboard events:

```cpp
void app_input(const sapp_event* event) {
    const bool shift = event->modifiers & SAPP_MODIFIER_SHIFT;
    switch (event->type) {
        int c;
        case SAPP_EVENTTYPE_CHAR:
            c = (int) event->char_code;
            if ((c > 0x20) && (c < KBD_MAX_KEYS)) {
                cpc_key_down(&cpc, c);
                cpc_key_up(&cpc, c);
            }
            break;
        case SAPP_EVENTTYPE_KEY_DOWN:
        case SAPP_EVENTTYPE_KEY_UP:
            switch (event->key_code) {
                case SAPP_KEYCODE_SPACE:        c = 0x20; break;
                case SAPP_KEYCODE_LEFT:         c = 0x08; break;
                case SAPP_KEYCODE_RIGHT:        c = 0x09; break;
                case SAPP_KEYCODE_DOWN:         c = 0x0A; break;
                case SAPP_KEYCODE_UP:           c = 0x0B; break;
                case SAPP_KEYCODE_ENTER:        c = 0x0D; break;
                case SAPP_KEYCODE_LEFT_SHIFT:   c = 0x02; break;
                case SAPP_KEYCODE_BACKSPACE:    c = shift ? 0x0C : 0x01; break; // 0x0C: clear screen
                case SAPP_KEYCODE_ESCAPE:       c = shift ? 0x13 : 0x03; break; // 0x13: break
                case SAPP_KEYCODE_F1:           c = 0xF1; break;
                case SAPP_KEYCODE_F2:           c = 0xF2; break;
                case SAPP_KEYCODE_F3:           c = 0xF3; break;
                case SAPP_KEYCODE_F4:           c = 0xF4; break;
                case SAPP_KEYCODE_F5:           c = 0xF5; break;
                case SAPP_KEYCODE_F6:           c = 0xF6; break;
                case SAPP_KEYCODE_F7:           c = 0xF7; break;
                case SAPP_KEYCODE_F8:           c = 0xF8; break;
                case SAPP_KEYCODE_F9:           c = 0xF9; break;
                case SAPP_KEYCODE_F10:          c = 0xFA; break;
                case SAPP_KEYCODE_F11:          c = 0xFB; break;
                case SAPP_KEYCODE_F12:          c = 0xFC; break;
                default:                        c = 0; break;
            }
            if (c) {
                if (event->type == SAPP_EVENTTYPE_KEY_DOWN) {
                    cpc_key_down(&cpc, c);
                }
                else {
                    cpc_key_up(&cpc, c);
                }
            }
            break;
        default:
            break;
    }
}
```

This function looks a little bit different for each emulator. In general, key
codes are communicated as 7-bit ASCII, with some special cases (e.g. function
keys are represented as hex codes 0xF1 to 0xFF). 

You might wonder why there are two code blocks, one for SAPP_EVENTTYPE_CHAR,
and one for SAPP_EVENTTYPE_KEY_DOWN/UP, the reason for this is mainly to
handle international keyboard layouts:

The SAPP_EVENTTYPE_CHAR returns an UNICODE character code that's independent
from the actual keyboard layout, so the character that's printed on a user's
keyboard is also the character that's sent to the emulator. The downside is
that you can't get up/down information with CHAR events (since some
characters might be composed from several keystrokes), that's why the handler
code for CHAR events sends both a key-down and key-up event for an entire key
stroke, this is good for text input, but bad for controlling games.

ASCII codes below 32 (0x20 hex) are not sent by the CHAR event, but need
to be handled through the "raw" key up/down event. The advantage is that
for these raw key events, separate up/down events are sent, so these can
be forwarded as-is to the emulator (this is useful for joystick emulation, 
and that's also the reason why the Space key is handled as a raw key event).

And that's pretty much it, a fully functioning emulator with very little
code :)

Only the cleanup function is left:

## app_cleanup()

This is called by sokol_app.h when the user exits the application by pressing
the window's close button:

```cpp
void app_cleanup(void) {
    cpc_discard(&cpc);
    saudio_shutdown();
    gfx_shutdown();
}
```
For simple application this cleanup procedure isn't even needed, and when
running as WebAssembly in the browser, this function won't be called at 
all (since when the user closes a browser tab, or quits the browser, a webpage
will simply 'vanish').
