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

This is just a short overview, more detailed per-emulator documentation can
be found in the respictive headers. The emulation quality differs quite a
bit, but will improve over time as I'm adding support for more tests, demos
and games.

## Embedding Guide

In general 

