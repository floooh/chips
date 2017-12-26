#-------------------------------------------------------------------------------
#   m6502_decoder.py
#   Generate instruction decoder for m6502.h emulator.
#-------------------------------------------------------------------------------
import sys

# tab width for generated code
TabWidth = 2

# the output path
OutPath = '../chips/_m6502_decoder.h'

# the output file handle
Out = None

#-------------------------------------------------------------------------------
#   output a src line
#
def l(s) :
    Out.write(s+'\n')

#-------------------------------------------------------------------------------
def write_defines():
    l('/* set 16-bit address in 64-bit pin mask*/')
    l('#define _SA(addr) pins=(pins&~0xFFFF)|((addr)&0xFFFFULL)')
    l('/* extract 8-bit data from 64-bit pin mask */')
    l('#define _GD() ((uint8_t)((pins&0xFF0000ULL)>>16))')
    l('/* enable control pins */')
    l('#define _ON(m) pins|=(m)')
    l('/* disable control pins */')
    l('#define _OFF(m) pins&=~(m)')
    l('/* execute a tick */')
    l('#define _T() pins=tick(pins);ticks++')

#-------------------------------------------------------------------------------
def write_undefines():
    l('#undef _SA')
    l('#undef _GD')
    l('#undef _ON')
    l('#undef _OFF')
    l('#undef _T')

#-------------------------------------------------------------------------------
def write_header():
    l("/* machine generated, don't edit! */")
    write_defines()
    l('uint32_t m6502_exec(m6502_t* cpu, uint32_t num_ticks) {')
    l('  m6502_t c = *cpu;')
    l('  uint32_t ticks = 0;')
    l('  uint64_t pins = c.PINS;')
    l('  const m6502_tick_t tick = c.tick;')
    l('  uint8_t opcode;')
    l('  do {')

#-------------------------------------------------------------------------------
def write_footer():
    l('  } while ((ticks < num_ticks) && ((pins & c.break_mask)==0));')
    l('  c.PINS = pins;')
    l('  *cpu = c;')
    l('  return ticks;')
    l('}')
    write_undefines()

#-------------------------------------------------------------------------------
#   execution starts here
#
Out = open(OutPath, 'w')
write_header()

# loop over all instruction bytes
for i in range(0, 256):
    pass
write_footer()
Out.close()
