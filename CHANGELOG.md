## What's New

* **02-Jan-2024**:

  - Added a new 'web api' (a set of Javascript functions which allow controlling
    the integrated debugger) for the emualators:
      - KC85/2, /3 and /4
      - C64
      - CPC
  - Debugger: added a 'stopwatch' window to count cycles between two breakpoints
  - am40010.h: video ram data is now loaded into an internal 2-byte latch
    in two steps at a 2 MHz interval

  Also check out this new VSCode extension: https://marketplace.visualstudio.com/items?itemName=floooh.vscode-kcide

* **10-Jan-2023**: A big code cleanup session affecting all emulators,
  and some minor emulation improvements:

  - C11 is now required to compile the headers (because of stdalign.h usage)
  - The KC85/4 emulation now implements the mysterious 'trück signal'
    (which is used to synchronize the two audio flip-flops). Clearing
    bit #0 on PIO port B forces both audio flip-flops to low state which
    allows to put the audio flip-flops into a known state (the intended
    function was to avoid that the two audio channels cancel each other
    out, but more recently the feature has been used to play sampled audio).
    Many thanks to dOc.K of the Moods Plateau demo group for the rubber ducking
    support and sample player code for testing, this was extremely helpful!
  - Another audio related problem in the KC85 emulation has been fixed
    (or rather, the Beeper emulaton used by the KC85 emulators): Forcing
    the beeper to a high state, and modulating the output waveform through
    the volume didn't work and resulted in silence. This bug wouldn't
    manifest itself with regular 'oscillating' audio output, but it broke
    sampled audio playback.
  - The video decoding on some systems has been slightly optimized
    (ZX Spectrum, KC85/2..4, Z9001 and Z1013)
  - All emulators now have a snapshot feature, mainly useful for debugging.
    Snapshot files are *not* intended as interchange file formats, since even
    small changes to the emulator implementation will make old snapshot files
    invalid. The feature simply works by taking a copy of the emulator's state
    in host memory, which is quite trivial since the entire emulator state
    lives in a single nested data structure, but this is also the reason why
    snapshot files are not stable, so use the feature at your own risk.
  - There's now a new header 'chips/chips_common.h' with shared structs and
    functions, this header now needs to be included before any other chip
    or system header.
  - pointer/size pairs have been replaced now with the common ```chips_range_t```
    struct
  - The video decoding has changed drastically:
    - The host framebuffer is now part of the emulator state and no longer provided
      as a ptr/size pair from the outside. That way, snapshots automatically contain
      a screenshot of the current emulator state.
    - The (now internal) host framebuffer is now generally 2^N bytes wide which
      simplifies address computation in the emulator's video decoding. The host
      framebuffer rendering code is expected to only render a visible sub-area
      out of the host framebuffer.
    - The various getter functions for quering information about the display
      have been replaced with a single function which returns a ```chips_display_info_t```
      struct, containing all information needed to render the emulator's
      framebuffer (framebuffer dimension, bytes per pixel, pointer to the internal
      framebuffer, the visible 'screen rectangle', an optional color palette,
      and a boolean whether the display is in portrait mode)
    - For most emulators (all except Bombjack), the host framebuffers pixel
      format has changed from RGBA8 (4 bytes per pixel) to R8 (1 byte per
      pixel) where each 8-bit pixel is an index into a static color palette
      which is provided separately. This reduced per-tick work in the emulator
      (by getting rid of 8 to 16 color palette lookups and writing less data
      per tick) and allows to move part of the video decoding (the color palette
      resolution) into a pixel shader running on the GPU. The only exception
      is Bombjack. This still generates an RGBA8 host framebuffer because the 'hardware
      palette' of the Bombjack arcade machine is 12-bits wide (4096 colors)
      and thus doesn't fit into a 256-entry color palette.
  - A potential buffer overflow in Bombjack has been fixed (under rare circumstances,
    the sprite rendering routine could write outside of the host framebuffer - this
    has been fixed, and in debug mode an assert has been added in the inner sprite
    decoder loop to catch any regressions) - in the previous 'host system wrapper'
    of Bombjack this problem wouldn't have resulted in a segfault, because the
    externally provided framebuffer memory block had plenty of 'scribble space'
    past the end of the emulator's framebuffer area.
  - A somewhat annoying input problem has been fixed in Bombjack: when the debugging
    UI was active, the coin input trigger would become unresponsive on host machines
    with a display refresh rate >60Hz (such as M1 Macs). The reason for this was
    pretty dumb and embarrassing, and thus doesn't require a detailed explanation ;P

* **16-Dec-2021**: An entirely new 'cycle-stepped' Z80 emulator, along with
  various improvements mainly in the Amstrad CPC emulation. Accompanying
  blog post upcoming. A snapshot of the old emulator is under the git tag
  **pre-cycle-stepped-z80** both in the [chip](https://github.com/floooh/chips/tags)
  and [chips-test](https://github.com/floooh/chips-test/tags) repo. PS: blog post is here:
  https://floooh.github.io/2021/12/17/cycle-stepped-z80.html

* **14-May-2020**: A small breaking change in kbd.h: the function ```kbd_update()```
    now takes a new argument ```uint32_t frame_time_us``` which is the
    current frame time (duration) in microseconds. This is necessary to
    make the sticky-key handling frame rate independent. The ```sticky_frames```
    initialization parameter in ```kbd_init()``` remains unchanged. This
    is the number of **60Hz frames** a key press should remain sticky.

* **20-Jan-2020**: The i8255 and MC6847 chips emulations have been changed
    to a 'tick-only API', continuing the 'API streamlining' that started
    with the 6522 VIA chip. The Atom system emulation has been updated
    accordingly, and the minimal necessary changes to the CPC emulation
    have been added (but only as a quick hack, the other CPC support
    chips haven't had their API updated yet).

* **15-Jan-2020**: The CIA (m6526.h), VIC-II (m6569.h), SID (m6581.h) chip
    emulators have merged their *_iorq() functions for reading
    and writing chip registers into the regular *_tick() functions,
    and the C64 emulation in systems/c64.h has been updated accordingly
    (this API change started with the chips in the vic20.h and will continue for
    the other chip emulators).

* **03-Jan-2020**: Another VIA- and VIC-20 related update:
    - The VIA (m6522.h) API has been simplified: the separate m6522_iorq()
      function to read and write chip registers has been merged into
      the m6522_tick() function, and all IO callbacks have been removed.
      Instead of handling the VIA IO ports through callbacks, the port input
      pins are now set before calling m6522_tick(),
      and the port outputs are inspected in the pin mask returned by
      m6522_tick(). Similar, if chip registers should be read or written,
      the chip-select and RW pin must be set on the input pin mask
      for m6522_tick(). This 'API streamlining' makes writing system
      tick functions more straightforward. System schematics now translate
      more directly into code which sets and inspects pin
      bit masks, instead of handling the address decoding, register
      reads/write, and IO through completely different code (such as
      IO callbacks).
    - The same API change (merging the iorq function into the tick function
      has been implemented for the VIC emulation (m6561.h). In the future
      the other chip emulations will follow too. See the ```_vic20_tick()```
      function in the ```systems/vic20.h``` header for a code example of how
      the new VIA and VIC APIs are used in a system's tick function.
    - The VIA emulation is now more feature complete and accurate, but
      not yet complete:
        - the entire shift-register functionality is not implemented
        - timers and interrupts are not cycle accurate in some situations
    - The VIC-20 emulation is now good enough to support TAP-file loading
      through c1530.h datassette emulation (this depends mostly on somewhat
      accurate VIA timers and interrupts). Also, the first modern demo-scene
      demos are now running in the VIC-20 emulation, although with glitches
      here and there.

* **24-Dec-2019**: A small "inbetween merge" because the feature branch I
    was working on became too unfocused: The plan was to add 1541 floppy
    support to the C64 emulation, but I soon realized that the VIA emulation
    would essentially need to be rewritten for this. To help getting the
    VIA emulation right, I started with a VIC-20 system emulation. Here's
    what's new:
    - a new VIC-20 system emulation, still quite WIP
    - started to rewrite the 6522 VIA emulation from scratch, it's still
      very WIP, but works "at least as good" as the previous implementation
    - a new VIC-I PAL (MOS 6561) emulation (used by the VIC-20)
    - moved the datasette emulation out of c64.h into its own header
      (c1530.h), for attaching peripheral devices the c64.h emulation
      now emulates the interface ports (IEC and CASPORT) instead, which
      the peripheral device emulations "connect to"
    - started with a 1541 floppy drive emulation in the header c1541.h,
      not functional yet
    - various optimizations in kbd.h with the goal to make the
      frequently called keyboard matrix scanning functions so cheap that
      they can be called in each emulated tick
    - various other minor cleanups and optimizations in m6526.h (CIA)
      and m6569.h (VIC-II)

    I will experiment next with more radical changes to the VIA emulation, idea
    is to merge the currently separate m6522_iorq() function (which handles
    reads and writes to the VIA registers) into the regular m6522_tick() function.
    If those experiments are successful, other chips will use the same
    model in the future, starting with the IO/timer chips (m6526, i8255,
    z80pio and z80ctc).

* **13-Dec-2019**: The new 'cycle-stepped' 6502/6510 emulator has been merged
    to master. The new emulator has a slightly different programming model,
    please see the updated [header
    documentation](https://github.com/floooh/chips/blob/master/chips/m6502.h) and
    [this blog
    post](https://floooh.github.io/2019/12/13/cycle-stepped-6502.html). I have
    created a [git tag](https://github.com/floooh/chips/tree/old-m6502) which
    preserves the previous emulator. All the 6502-based system emulators on the
    [Tiny Emulators page](https://floooh.github.io/tiny8bit/) have been updated
    with the new cycle-stepped 6502.

* **14-Oct-2019**: Improvements to the 6502 and C64 emulation:
    - All tests of the Wolfgang Lorenz test suite are passing now, except:
        - irq and nmi: the timing for interrupt requests is slightly off,
        this is most likely because the 6502 emulation currently doesn't
        delay interrupt handling to the end of the next instruction under
        some circumstances
        - cia1ta, cia1tb, cia2ta, cia2tb: these are marked as "under
        construction" in the test suite's readme, so I assume it's normal
        that they are failing(?)
    - With the remaining tests all passing, this means:
        - all unintended and unstable 6502 instructions are now supported as
        tested by the Wolfgang Lorenz test suite
        - all instruction clock cycles are now correct (previously two
        unintended NOP instructions were one clock tick off because they used
        the wrong addressing mode)
    - Some code cleanup and (very minor) optimizations in the 6569 (VIC-II)
      emulation, and improved the raster interrupt timing which was slightly
      off.

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
