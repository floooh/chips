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
    l('/* set 16-bit address and 8-bit data in 64-bit pin mask */')
    l('#define _SAD(addr,data) pins=(pins&~0xFFFFFF)|(((data)<<16)&0xFF0000ULL)|((addr)&0xFFFFULL)')
    l('/* extract 8-bit data from 64-bit pin mask */')
    l('#define _GD() ((uint8_t)((pins&0xFF0000ULL)>>16))')
    l('/* enable control pins */')
    l('#define _ON(m) pins|=(m)')
    l('/* disable control pins */')
    l('#define _OFF(m) pins&=~(m)')
    l('/* execute a tick */')
    l('#define _T() pins=tick(pins);ticks++')
    l('/* a memory read tick */')
    l('#define _RD() _ON(M6502_RW);_T();')
    l('/* a memory write tick */')
    l('#define _WR() _OFF(M6502_RW);_T()')

#-------------------------------------------------------------------------------
def write_undefines():
    l('#undef _SA')
    l('#undef _SAD')
    l('#undef _GD')
    l('#undef _ON')
    l('#undef _OFF')
    l('#undef _T')
    l('#undef _RD')
    l('#undef _WR')

#-------------------------------------------------------------------------------
def write_interrupt_handling():
    l('    /* check for interrupt request */')
    l('    if ((pins & M6502_NMI) || ((pins & M6502_IRQ) && !(c.P & M6502_IF))) {')
    l('      /* execute a slightly modified BRK instruction */')
    l('      _RD();')
    l('      _SAD(0x0100|c.S--, c.PC>>8); _WR();')
    l('      _SAD(0x0100|c.S--, c.PC); _WR();')
    l('      _SAD(0x0100|c.S--, c.P&~M6502_BF); _WR();')
    l('      if (pins & M6502_NMI) {')
    l('        _SA(0xFFFA); _RD(); l=_GD();')
    l('        _SA(0xFFFB); _RD(); h=_GD();')
    l('      }')
    l('      else {')
    l('        _SA(0xFFFE); _RD(); l=_GD();')
    l('        _SA(0xFFFF); _RD(); h=_GD();')
    l('      }')
    l('      c.PC = (h<<8)|l;')
    l('      c.P |= M6502_IF;')
    l('      pins &= ~(M6502_IRQ|M6502_NMI);')
    l('    }')

#-------------------------------------------------------------------------------
def write_header():
    l("/* machine generated, don't edit! */")
    write_defines()
    l('uint32_t m6502_exec(m6502_t* cpu, uint32_t num_ticks) {')
    l('  m6502_t c = *cpu;')
    l('  uint8_t l, h;')
    l('  uint32_t ticks = 0;')
    l('  uint64_t pins = c.PINS;')
    l('  const m6502_tick_t tick = c.tick;')
    l('  do {')
    l('    /* fetch opcode */')
    l('    _SA(c.PC++);_ON(M6502_SYNC);_RD();_OFF(M6502_SYNC);')
    l('    const uint8_t opcode = _GD();')
    l('    switch (opcode) {')

#-------------------------------------------------------------------------------
def write_footer():
    l('    }')
    write_interrupt_handling()
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
