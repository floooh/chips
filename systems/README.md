# Header-only, embeddable 8-bit emulators written in C

Emulated systems:

- ```z1013.h```: the Robotron Z1013 in the following variants:
    - ```Z1013.01```: the original Z1013 with 1 MHz Z80 CPU, 16 KB RAM, "old" operating system ROM and 8x4 keyboard matrix with 4 shift keys
    - ```Z1013.16```: the modernised Z1013 with 2 MHz CPU, 16 KB RAM, "new" operating system ROM and an 8x8 keyboard matrix for 'industry standard' keyboards
    - ```Z1013.64```: same as the Z1013.16 model, but with 64 KByte RAM
- ```z9001.h```: the Robotron Z9001 aka KC85/1 in 2 versions:
    - ```Z9001```: the original Z9001 which was later renamed to KC85/1 with black-and-white display, a 16 KByte RAM extension (for 32 KByte RAM overall), and an optional BASIC module
    - ```KC87```: the improved KC87 with color module, 48 KByte RAM and builtin BASIC ROM
- ```zx.h```: the ZX Spectrum in 2 variants:
    - ```ZX Spectrum 48k```: 48 KB RAM and beeper sound
    - ```ZX Spectrum 128```: the improved Spectrum with 128 KB RAM and an additional AY-3-8912 sound chip
- ```cpc.h```: the Amstrad CPC in 3 variants:
    - ```CPC 464```: the original 64 KByte Amstrad CPC
    - ```CPC 6128```: the improved CPC with 128 KB RAM
    - ```KC Compact```: an East German CPC clone (the emulator only differs in color palette and ROM, it doesn't emulate gate array 'hardware-emulation' in the KC Compact
- ```atom.h```: an ```Acorn Atom``` with modern extensions (32 KB RAM + 8 KB video memory, a rudinmentary VIA 6522 emulation and MMC joystick support)
- ```c64.h```: a ```C64``` emulator with PAL display

This is just a short overview, more detailed per-emulator documentation can
be found in the respective headers. The emulation quality differs quite a
bit, but will improve over time as I'm adding support for more tests, demos
and games.

## General properties of the emulator headers

- all headers are "STB-style" with a declaration part that's always
parsed, and an implemented part that's only parsed when ```CHIPS_IMPL```
is defined in one source file
- the declaration part only includes the following headers: ```stdint.h```, ```stdbool.h```, ```string.h```
- the implementation part will additionally include ```assert.h```, unless you define your own CHIPS_ASSERT macro
- the following CRT header features are used:
    - uint8_t, uint16_t, uint32_t, uint64_t, bool
    - memset(), memcpy()
    - assert() (unless CHIPS_ASSERT is overridden)
- the system emulators depend on the chip emulators in the ```chips``` directory, but you need to include the right headers before including the system emulator header (this gives you the freedom to place the headers wherever you want in your own project directory hierarchy)
- the headers will never take ownership of memory you provide, any memory handed
into the emulator via pointers can be scribbled over or released immediately
after a function with pointer arguments returns

## Embedding Guide

You provide:

- ROM dumps in memory
- keyboard input
- a memory chunk which the emulator will use as a linear RGBA8 framebuffer
- a callback function which receives small packets of audio 

The emulator provides:
- the video display as a linear RGBA8 framebuffer
- audio data as a stream of 32-bit float samples in the range -1.0..1.0

Normally your embedding app does the following things per 60 Hz frame:
- tell the emulator to execute 16.667 milliseconds worth of 'ticks'
- call the emulator's key up/down functions to communicate keyboard input to the emulator
- render the RGBA8 framebuffer

Independently from the per-frame work above, the emulator will call your
audio callback whenever it has finished generating a new packet of 
audio samples (by default one packet is 128 samples, but this number
is tweakable).

Following is a walkthrough of the Tiny Emulator example for the
Amstrad CPC in the [chips-test repository](https://github.com/floooh/chips-test).

## Embedding Walkthrough

(with the Amstrad CPC as example, the other emulators only differ
in details like game snapshot fileformats etc...)

The example embedding uses the following dependencies:

- [sokol_app.h](https://github.com/floooh/sokol/blob/master/sokol_app.h): as cross-platform application-wrapper (app entry, window- and 3D-context creation, and input)
- [sokol_gfx.h](https://github.com/floooh/sokol/blob/master/sokol_gfx.h): to copy the emulator's framebuffer into a texture and render it through a 3D-API (using GL, D3D11 or Metal)
- [sokol_audio.h](https://github.com/floooh/sokol/blob/master/sokol_audio.h): to stream the generated audio samples into a platform-specific audio backend
- [sokol_time.h](https://github.com/floooh/sokol/blob/master/sokol_time.h): to measure the elapsed time between host-system frames

## The ```common``` helper function library

The above low-level headers are further wrapped into a small ```common``` library which all emulators link against, consisting 
of the following headers and helper functions:

- ```args.h```: argument parser for command line args (when compiled as native OSX/Windows/Linux exe), or from URL args (when compiled for WebAssembly). Arguments must have the form ```key=value```
    - ```void args_init(int argc, char* argv[])```: initialize the args parser
    - ```void args_has(const char* key)```: check if a named argument is present
    - ```const char* args_string(const char* key)```: return a value string by key, or an empty string if the arg doesn't exist
    - ```bool args_string_compare(const char* key, const char* value)```: return true if an argument exists, and has the expected value
    - ```bool args_bool(const char* key)```: return true if an arg exists and has the value "true", "yes" or "on"
- ```clock.h```: measure frame time and count frames
    - ```void clock_init(void)```: initialize/reset all values
    - ```double clock_frame_time(void)```: return the last frame's duration in seconds, this will be "rouneded" to either 0.016666..
    or 0.033333.. seconds to ensure a smooth framerate and skip long pauses (for instance when the emulator's webpage
    tab was switched to the background)
    - ```uint32_t clock_frame_count(void)```: returns the number of frames since clock_init() (or rather the number of
    times the clock_frame_time() function was called)
- ```fs.h```: file loading support via fopen() of HTTP, only one file is loaded at a time, the maximum file size is currently limited
to 256 KBytes
    - ```fs_init(void)```: initialize the file loader
    - ```fs_load_file(const char* path)```: load a file synchronously via fopen() (not available in WebAssembly)
    - ```fs_load_mem(const uint8_t* ptr, uint32_t size)```: load a file from data in memory
    - ```uint32_t fs_size(void)```: returns the size of the currently loaded file in bytes, or 0 if no file is loaded
    - ```uint8_t* fs_ptr(void)```: returns a pointer to the loaded file data, or 0 if no file is loaded
    - ```void fs_free(void)```: unloads the currently loaded file
- ```gfx.h```: wrapper functions for rendering the emulator display via sokol_gfx.h:
    - ```gfx_init(int fb_width, int fb_height, int aspect_scale_x, int aspect_scale_y)```: initialize the rendering system with the pixel dimensions of the current emulator's framebuffer, if the emulator system has square pixels, both aspect_scale_x and aspect_scale_y should be set to one, the only current exception is the Amstrad CPC, which has 'half-width' pixels
    - ```gfx_draw(void)```: renders the emulator framebuffer memory (called once per frame)
    - ```gfx_shutdown(void)```: shuts down sokol_gfx.h
    - ```uint8_t rgba8_buffer[]```: the memory chunk used as a framebuffer
- ```keybuf.h```: split a string into sequence of key strokes, useful to send 'terminal commands' to the emulator
    - ```keybuf_put(int start_delay, int key_delay, const char* text)```: setup the string ```text``` for playback after ```start_delay``` frames, with ```key_delay``` frames between key strokes
    - ```uint8_t keybuf_get(void)```: call this once per frame, this will either return the next character from the string setup with ```keybuf_put()```, or 0 if this is a 'delay frame', or the end of the string had been reached

### Step 1: Header includes and app-skeleton

### 




