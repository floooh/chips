# chips

[![Build Status](https://github.com/floooh/chips/workflows/build_and_test/badge.svg)](https://github.com/floooh/chips/actions)

A toolbox of 8-bit chip-emulators, helper code and complete embeddable
system emulators in dependency-free C headers (a subset of C99 that
compiles on gcc, clang and cl.exe).

Tests and example code is in a separate repo: https://github.com/floooh/chips-test

The example emulators, compiled to WebAssembly: https://floooh.github.io/tiny8bit/

For schematics, manuals and research material, see: https://github.com/floooh/emu-info

The USP of the chip emulators is that they communicate with the outside world through
a 'pin bit mask': A 'tick' function takes an uint64_t as input where the bits
represent the chip's in/out pins, the tick function inspects the pin
bits, computes one tick, and returns a (potentially modified) pin bit mask.

A complete emulated computer then more or less just wires those chip emulators
together just like on a breadboard.

In reality, most emulators are not quite as 'pure' (as this would affect performance
too much or complicate the emulation): some chip emulators have a small number
of callback functions and the adress decoding in the system emulators often
take shortcuts instead of simulating the actual address decoding chips
(with one exception: the lc80 emulator).
