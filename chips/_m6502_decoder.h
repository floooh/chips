/* machine generated, don't edit! */
/* set 16-bit address in 64-bit pin mask*/
#define _SA(addr) pins=(pins&~0xFFFF)|((addr)&0xFFFFULL)
/* extract 8-bit data from 64-bit pin mask */
#define _GD() ((uint8_t)((pins&0xFF0000ULL)>>16))
/* enable control pins */
#define _ON(m) pins|=(m)
/* disable control pins */
#define _OFF(m) pins&=~(m)
/* execute a tick */
#define _T() pins=tick(pins);ticks++
uint32_t m6502_exec(m6502_t* cpu, uint32_t num_ticks) {
  m6502_t c = *cpu;
  uint32_t ticks = 0;
  uint64_t pins = c.PINS;
  const m6502_tick_t tick = c.tick;
  uint8_t opcode;
  do {
  } while ((ticks < num_ticks) && ((pins & c.break_mask)==0));
  c.PINS = pins;
  *cpu = c;
  return ticks;
}
#undef _SA
#undef _GD
#undef _ON
#undef _OFF
#undef _T
