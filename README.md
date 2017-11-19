# chips

A toolbox of chips and helper code to write 8-bit emulators as
dependency-free C-headers.

*WORK IN PROGRESS*

Tests and example code is in a separate repo: https://github.com/floooh/chips-test

This is the meaty stuff from the YAKC emulator, rewritten from
C++ in C, moved into its own project.

What's planned:

- 8-bit memory with bank-mapping and ram-behind-rom
- Z80 CPU (improved from YAKC, with proper cycle emulation)
- Z80 PIO
- Z80 CTC
- Z80 interrupt controller daisy chain
- MOS 6502 CPU
- MOS 6522 VIA
- i8255 PPI
- MC6845
- MC6847
- AY-3-8910
