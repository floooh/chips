This directory contains code-generation python scripts which will generate the
z80.h and m6502.h headers.

In a bash compatible shell run:

```sh
./z80_gen.sh
./m6502_gen.sh
```

This will run Python3 inside a virtual environment and read/write the `chips/z80.h` and
`chips/m6502.h` headers.
