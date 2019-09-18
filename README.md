# chips

[![Build Status](https://github.com/floooh/chips/workflows/build_and_test/badge.svg)](https://github.com/floooh/chips/actions)

A toolbox of 8-bit chip-emulators, helper code and complete embeddable 
system emulators in dependency-free C headers (a subset of C99 that
compiles on gcc, clang and cl.exe).

Tests and example code is in a separate repo: https://github.com/floooh/chips-test

The example emulators, compiled to WebAssembly: https://floooh.github.io/tiny8bit/

For schematics, manuals and research material, see: https://github.com/floooh/emu-info

## What's New

* **05-Aug-2019**:
    - The Z80 and 6502 CPU emulators are now each in a single header instead
    of being split into a manually written "outer header" which includes
    another code-generated header with the instruction decoder.
    No functional changes (I tried a variation of the Z80 emulator which goes
    back to separate byte registers in a struct instead of merging the
    registers into 64-bit integers, this saved a couple KBytes code size in
    WASM but was about 10% slower so I discarded that experiment)

* **31-Dec-2018**: 
    - A complete set of debugging UI headers using Dear ImGui has been added,
    each chip emulator has a window which visualizes the pin- and
    internal-state, and there are helper windows which implement a memory
    editor, memory "heatmap" (visualize read/write/execute operations),
    disassembler and CPU step debugger. Finally there are 'integration
    headers' which implement an entire UI for an emulated system. Note that
    the implementation part of the UI headers needs to be compiled as C++,
    the 'public API' of the headers are callable from C though.
    - The CPU emulators (z80.h and m6502.h) have new trap handling. Instead
    of predefined "slots", a trap evaluation callback is now installed, which
    is called at the end of each CPU instruction. This is used extensively by
    the new debugging UIs to keep track of CPU operations and breakpoint
    support.
    - The Amstrad CPC emulation has gained floppy disc loading support, and
    the video system precision has been improved (many modern graphics demos
    at least work now instead of having completely broken rendering, but
    there's still more to be done).
    - Loading local files via drag'n'drop has been improved in the
    WebAssembly version, all emulators can now properly detect and load all
    supported file formats via drag'n'drop.

* **23-Jul-2018**: all chip emulators with callbacks now have an extra
```user_data``` argument in the callbacks which is provided in the init
function, this makes the chip emulators a bit more flexible when more than
one emulator of the same type is used in a program

## System Emulators

The directory ```systems``` contains a number of header-only 8-bit
computer emulators which can be embedded into applications that
provide keyboard input, and render the emulator's generated
video- and audio-output.

Note that accuracy of the system emulators varies quite a lot,
and is mainly defined by what games, demos and tests have been
used for testing and improving the emulation.

The following system emulators are provided:

### KC85/2, /3 and /4

An East German Z80-based computer with 320x256 color graphics, beeper
sound a powerful expansion slot system and (for its time) innovative
operating system. The KC85/2 family was designed and built
by VEB Mikroelektronik Mühlhausen between 1984 and 1989.

### Z9001 (aka KC85/1), KC87

Another East German 8-bitter created by Robotron Dresden.
This was a more conventional, less innovative design compared 
to the KC85/2, both in hardware and software. The Z9001 family
only had 40x20 ASCII pseudo-graphics display with optional
color support.

### Z1013

This was the most simple and cheapest Z80-based computer built
in Eastern Germany that still resembled a 'proper' computer which
could be attached to a TV. It was also the only Eastern German computer
that was available as assemble-youself-kit for the general public.

### ZX Spectrum 48k and 128

The Sinclair ZX Spectrum is supported as the original 48k model and
the improved 128 model with a proper sound chip (the AY-3-8912, which was 
also used in the Amstrad CPC).

### CPC 464, 6128 and KC Compact

FIXME

### C64

FIXME

### Acorn Atom

FIXME

## Chip Emulators

### Z80 CPU (chips/z80.h)

The Zilog Z80 CPU.

- tick-callback with CPU-pin bitmask, called with machine-cycle or single-tick granularity
- fast switch-case instruction decoder generated via python script
- up to 7 wait states can be injected per machine cycle by setting WAIT pins in 
  the CPU tick callback
- all undocumented instructions supported
- internal WZ register and undocumented XF and YF flags supported
- support for interrupt-priority handling (daisy chain) with help from the tick callback
- runs the ZEXDOC and ZEXALL tests
- runs the CPU test of the FUSE ZX Spectrum emulator, with the following exceptions:
  - the state of the XF/YF undocumented flags is ignored for indirect BIT
  test instructions, FUSE doesn't agree here with ZEXALL and I think ZEXALL
  is right (the state of the XF/YF flags depends on the current state of the
  internal WZ register)
  - FUSE assumes that the PC after a HALT instruction has been incremented,
  while the chips Z80 emulator doesn't incrmenent the PC, this shouldn't make
  any difference though
- properly handles sequences of DD/FD prefix bytes
- flexible trap callback for hooking in debuggers and "native code" handlers
- NOT IMPLEMENTED/TODO:
    - interrupt mode 0
    - refresh cycle in second half of opcode fetch machine cycle
    - bus request/acknowledge (BUSRQ/BUSAK pins)

### Z80 PIO (chips/z80pio.h)

The Zilog Z80 Parallel Input/Output controller.

- programmed via Z80-compatible chip-pin bitmask
- two callbacks for passive port A/B input/output
- write-function for active port A/B input (may trigger interrupt)
- can act as interrupt controller in a Z80 interrupt-daisy-chain
- NOT IMPLEMENTED:
    - bidirectional mode

### Z80 CTC (chips/z80ctc.h)

The Zilog Z80 Counter/Timer Channels.

- programmed via Z80-compatible chip-pin bitmask
- emulates the CLK/TRG and ZC/TO input/output pins
- can act as interrupt controller in a Z80 interrupt-daisy-chain

### MOS 6502 CPU (chips/m6502.h)

The MOS Technology 6502 CPU.

- single tick-callback with CPU-pin bitmask, called with tick-granularity
- fast switch-case instruction decoder generated via python script
- emulates all(?) quirks (like redundant and 'junk' read/write cycles, variable cycle counts in some addressing modes, page boundary wrap-around in indirect jump, etc...), mostly verified via visual6502.org
- emulates the known and useful 'documented-undocumented' opcodes (like LAX, SAX, DCP, ...)
- decimal mode implemented, can be disabled
- same powerful trap callback as the Z80 emulator
- test coverage:
    - **NESTEST**: completely working (this runs through all documented, and most 'common'
      undocumented instructions but doesn't test decimal mode)
    - **Wolfgang Lorenz C64 Test Suite** (CPU tests):
      - _adc*_: OK
      - _alrb, arrb, ancb_: OK
      - _and*_: OK
      - _aneb_: **FAIL**
      - _asl*_: OK
      - _aso*_: OK
      - _axs*_: OK
      - _b*r_ (branches): OK
      - _bit*_: OK
      - _branchwrap_: OK
      - _brkn_: OK
      - _cl*_, se*_ (set/clear flags): OK
      - _cmp*_: OK
      - _cpuport_: OK (m6510 CPU port)
      - _cputiming_: **FAIL** (opcodes 5C and 7C show 'clocks: 8 right: 9')
      - _dcm*_: OK
      - _dec*, dexn, deyn_: OK
      - _eor*_: OK
      - _inc*, inxn, inyn_: OK
      - _ins*_: OK
      - _inxn, inyn_: OK
      - _irq_: **FAIL**
      - _jmpi, jmpw, jsrw_: OK
      - _lasay_: **FAIL**
      - _lax*_: OK
      - _lda*_: OK
      - _ldx*, ldy*_: OK
      - _lse*_: OK
      - _lsr*_: OK
      - _lxab_: **FAIL**
      - _nmi_: **FAIL**
      - _nop*_: OK
      - _ora*_: OK
      - ...

### AY-3-8910 (chips/ay38910.h)

The General Instrument AY-3-8910 sound generator chip and its low-cost variants
AY-3-8912 and AY-3-8913 (the 3 variants only differ in the number of provided
I/O ports, the 8910 has 2 ports, the 8912 has 1, and the 8913 has none.

- programmable via chip-pin bitmask
- separate tick function called from CPU tick callback, returns true when a new audio sample is ready

### MC6845 (chips/mc6845.h)

Motorola 6845 video address generator and variants.

- programmable via chip-pin bitmask
- tick function which generates hsync, vsync, display-enable, 13-bit 
  memory-address and 5-bit row-address, returned as pin-mask
- NOT IMPLEMENTED/TODO:
    - interlace mode
    - the cursor pin
    - the light-pen functionality
- NOTE: emulation quality is "ok" for most Amstrad graphics demos, but more
improvements are needed

### MC6847 (chips/mc6847.h)

Motorola 6847 video display generator.

- programmable via chip-pin bitmask
- tick function which directly generates a RGBA8 framebuffer
- memory-fetch callback called from the tick function, this returns
  a complete pin-mask and can be used to set the mode-select input pins
  (this is used in the Acorn Atom for instance, which directly connects
  data bus pins to MC6847 mode-select pins)

### i8255 PPI (chips/i8255.h)

Intel 8255 Programmable Peripheral Interface

- programmable via chip-pin mask
- NOT IMPLEMENTED / TODO:
    - Mode 1 (strobed input/output)
    - Mode 2 (bi-directional bus)
    - interrupt generation

### MOS 6522 VIA (chips/m6522.h)

MOS Technology 6522 Versatile Interface Adapter.

Currently this just contains the minimal required functionality to make
some games on the Acorn Atom work (basically just timers, and even those
or likely not correct). 

### MOS 6526 CIA (chips/m6526.h)

MOS Technology 6526 Complex Interface Adapter

(Work In Progress)

- **Wolfgang Lorenz C64 Test Suite Status**:
  - _cia1pb6, cia1pb7, cia2pb6, cia2pb7_: OK
  - _cia1ta, cia1tb, cia2ta, cia2tb_: **FAIL** (but improved, doesn't fail immediately)
  - _cia1tab_: OK
  - _cia1tb123, cia2tb123_: OK
  - _cntdef, cnto2_: OK (but note that CNT pin is not emulated, it's always high)
  - _flipos_: OK
  - _icr01_: OK
  - _imr_: OK
  - _loadth_: OK
  - _oneshot_: OK
- **NOT IMPLEMENTED:**
  - time-of-day features
  - serial port
  - PC pin
  - CNT pin is always high

### MOS 6569 VIC-II for PAL-B (chips/m6569.h)

MOS Technology 6569 Video Interface Chip VIC-II (FIXME: needs more info)

### MOS 6581 SID (chips/m6581.h)

The C64 sound chip (FIXME: needs more info)

### AM40010 Amstrad CPC Gate Array Emulation (chips/am40010.h)

This emulated the Amstrad CPC gate array chip, and also integrates the PAL
chip for bankswitching in the 6128. 

### UPD765 Floppy Disc Controller (chips/upd765.h)

This is a basic emulation of the UPD765 floppy controller. Currently only
features required by the Amstrad CPC are implemented.

## Helper Code

### Memory (chips/mem.h)

Helper code for 8-bit home computer memory systems.

- map 16-bit address ranges to host memory with 1-KByte page granularity
- memory pages can be mapped as RAM, ROM or RAM-behind-ROM via separate read/write pointers
- up to 4 'mapping layers' to simplify bank-switching or memory expansion systems

### Keyboard Matrix (chips/kbd.h)

Helper code for keyboard matrix typically found in 8-bit home computers.

- map host system key- or ASCII-codes to keyboard matrix locations
- up to 12x12 matrix size
- up to 4 configurable modifier keys (for shift keys)
- internal pressed-key-buffer for simultaneously pressed host system keys
- configurable 'sticky count' to lengthen short host-system key presses to
give the emulated system enough time to scan the keyboard matrix state

### Sound Beeper (chips/beeper.h)

A square-wave beeper found in many simple home computers.

- toggle beeper state on/off from CPU tick callback
- tick function returns true when new audio-sample is ready

### Clock (chips/clk.h)

Helper function to convert a clock frequency in Hz to a number of ticks,
and to keep track of the 'left over' ticks from one frame to the next.

### Floppy Disc Drive (chips/fdd.h)

A basic floppy disc drive emulator, currently only basic functionality
as needed by the Amstrad CPC emulation.

### Amstrad CPC Disk Image Loader (chips/fdd_cpc.h)

This is an extension-header for fdd.h which adds support to read
an Amstrad CPC .DSK disk image file and "insert" it into the Floppy Disc Drive
(fdd.h).

