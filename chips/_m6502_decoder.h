/* machine generated, don't edit! */
/* set 16-bit address in 64-bit pin mask*/
#define _SA(addr) pins=(pins&~0xFFFF)|((addr)&0xFFFFULL)
/* set 16-bit address and 8-bit data in 64-bit pin mask */
#define _SAD(addr,data) pins=(pins&~0xFFFFFF)|((((data)&0xFF)<<16)&0xFF0000ULL)|((addr)&0xFFFFULL)
/* set 8-bit data in 64-bit pin mask */
#define _SD(data) pins=((pins&~0xFF0000ULL)|(((data&0xFF)<<16)&0xFF0000ULL))
/* extract 8-bit data from 64-bit pin mask */
#define _GD() ((uint8_t)((pins&0xFF0000ULL)>>16))
/* enable control pins */
#define _ON(m) pins|=(m)
/* disable control pins */
#define _OFF(m) pins&=~(m)
/* execute a tick */
#define _T() pins=tick(pins,ud);ticks++;
/* a memory read tick */
#define _RD() _ON(M6502_RW);do{_OFF(M6502_RDY);_T();}while(pins&M6502_RDY);
/* a memory write tick */
#define _WR() _OFF(M6502_RW);_T()
/* implied addressing mode, this still puts the PC on the address bus */
#define _A_IMP() _SA(c.PC)
/* immediate addressing mode */
#define _A_IMM() _SA(c.PC++)
/* zero-page addressing mode */
#define _A_ZER() _SA(c.PC++);_RD();a=_GD();_SA(a)
/* zero page + X addressing mode */
#define _A_ZPX() _SA(c.PC++);_RD();a=_GD();_SA(a);_RD();a=(a+c.X)&0x00FF;_SA(a)
/* zero page + Y addressing mode */
#define _A_ZPY() _SA(c.PC++);_RD();a=_GD();_SA(a);_RD();a=(a+c.Y)&0x00FF;_SA(a)
/* absolute addressing mode */
#define _A_ABS() _SA(c.PC++);_RD();l=_GD();_SA(c.PC++);_RD();h=_GD();a=(h<<8)|l;_SA(a)
/* absolute+X addressing mode for read-only instructions, early out if no page boundary is crossed */
#define _A_ABX_R() _SA(c.PC++);_RD();t=_GD()+c.X;_SA(c.PC++);_RD();a=(_GD()<<8)|(t&0xFF);_SA(a);if((t&0xFF00)!=0){_RD();a=(a&0xFF00)+t;_SA(a);}
/* absolute+X addressing mode for read/write instructions */
#define _A_ABX_W() _SA(c.PC++);_RD();t=_GD()+c.X;_SA(c.PC++);_RD();a=(_GD()<<8)|(t&0xFF);_SA(a);_RD();a=(a&0xFF00)+t;_SA(a)
/* absolute+Y addressing mode for read-only instructions, early out if no page boundary is crossed */
#define _A_ABY_R() _SA(c.PC++);_RD();t=_GD()+c.Y;_SA(c.PC++);_RD();a=(_GD()<<8)|(t&0xFF);_SA(a);if((t&0xFF00)!=0){_RD();a=(a&0xFF00)+t;_SA(a);}
/* absolute+Y addressing mode for read/write instructions */
#define _A_ABY_W() _SA(c.PC++);_RD();t=_GD()+c.Y;_SA(c.PC++);_RD();a=(_GD()<<8)|(t&0xFF);_SA(a);_RD();a=(a&0xFF00)+t;_SA(a)
/* (zp,X) indexed indirect addressing mode */
#define _A_IDX() _SA(c.PC++);_RD();a=_GD();_SA(a);_RD();a=(a+c.X)&0xFF;_SA(a);_RD();t=_GD();a=(a+1)&0xFF;_SA(a);_RD();a=(_GD()<<8)|t;_SA(a);
/* (zp),Y indirect indexed addressing mode for read-only instructions, early out if no page boundary crossed */
#define _A_IDY_R() _SA(c.PC++);_RD();a=_GD();_SA(a);_RD();t=_GD()+c.Y;a=(a+1)&0xFF;_SA(a);_RD();a=(_GD()<<8)|(t&0xFF);_SA(a);if((t&0xFF00)!=0){_RD();a=(a&0xFF00)+t;_SA(a);}
/* (zp),Y indirect indexed addressing mode for read/write instructions */
#define _A_IDY_W() _SA(c.PC++);_RD();a=_GD();_SA(a);_RD();t=_GD()+c.Y;a=(a+1)&0xFF;_SA(a);_RD();a=(_GD()<<8)|(t&0xFF);_SA(a);_RD();a=(a&0xFF00)+t;_SA(a)
/* set N and Z flags depending on value */
#define _NZ(v) c.P=((c.P&~(M6502_NF|M6502_ZF))|((v&0xFF)?(v&M6502_NF):M6502_ZF))

uint32_t m6502_exec(m6502_t* cpu, uint32_t num_ticks) {
  m6502_state_t c = cpu->state;
  int trap_id = -1;
  uint8_t l, h;
  uint16_t a, t;
  uint32_t ticks = 0;
  uint64_t pins = c.PINS;
  const m6502_tick_t tick = cpu->tick;
  void* ud = cpu->user_data;
  do {
    uint64_t pre_pins = pins;
    _OFF(M6502_IRQ|M6502_NMI);
    /* fetch opcode */
    _SA(c.PC++);_ON(M6502_SYNC);_RD();_OFF(M6502_SYNC);
    /* store 'delayed IRQ response' flag state */
    c.pi = c.P;
    const uint8_t opcode = _GD();
    switch (opcode) {
      case 0x0:/*BRK */_A_IMP();_RD();c.PC++;_SAD(0x0100|c.S--,c.PC>>8);_WR();_SAD(0x0100|c.S--,c.PC);_WR();_SAD(0x0100|c.S--,c.P|M6502_BF);_WR();_SA(0xFFFE);_RD();l=_GD();_SA(0xFFFF);_RD();h=_GD();c.PC=(h<<8)|l;c.P|=M6502_IF;break;
      case 0x1:/*ORA (zp,X)*/_A_IDX();_RD();c.A|=_GD();_NZ(c.A);break;
      case 0x2:/*INVALID*/break;
      case 0x3:/*SLO (zp,X) (undoc)*/_A_IDX();_RD();_WR();l=_GD();c.P=(c.P&~M6502_CF)|((l&0x80)?M6502_CF:0);l<<=1;_NZ(l);_SD(l);_WR();c.A|=l;_NZ(c.A);break;
      case 0x4:/*NOP zp (undoc)*/_A_ZER();_RD();break;
      case 0x5:/*ORA zp*/_A_ZER();_RD();c.A|=_GD();_NZ(c.A);break;
      case 0x6:/*ASL zp*/_A_ZER();_RD();_WR();l=_GD();c.P=(c.P&~M6502_CF)|((l&0x80)?M6502_CF:0);l<<=1;_NZ(l);_SD(l);_WR();break;
      case 0x7:/*SLO zp (undoc)*/_A_ZER();_RD();_WR();l=_GD();c.P=(c.P&~M6502_CF)|((l&0x80)?M6502_CF:0);l<<=1;_NZ(l);_SD(l);_WR();c.A|=l;_NZ(c.A);break;
      case 0x8:/*PHP */_A_IMP();_RD();_SAD(0x0100|c.S--,c.P|M6502_BF);_WR();break;
      case 0x9:/*ORA #*/_A_IMM();_RD();c.A|=_GD();_NZ(c.A);break;
      case 0xa:/*ASLA */_A_IMP();_RD();c.P=(c.P&~M6502_CF)|((c.A&0x80)?M6502_CF:0);c.A<<=1;_NZ(c.A);break;
      case 0xb:/*ANC # (undoc)*/_A_IMM();_RD();c.A&=_GD();_NZ(c.A);if(c.A&0x80){c.P|=M6502_CF;}else{c.P&=~M6502_CF;}break;
      case 0xc:/*NOP abs (undoc)*/_A_ABS();_RD();break;
      case 0xd:/*ORA abs*/_A_ABS();_RD();c.A|=_GD();_NZ(c.A);break;
      case 0xe:/*ASL abs*/_A_ABS();_RD();_WR();l=_GD();c.P=(c.P&~M6502_CF)|((l&0x80)?M6502_CF:0);l<<=1;_NZ(l);_SD(l);_WR();break;
      case 0xf:/*SLO abs (undoc)*/_A_ABS();_RD();_WR();l=_GD();c.P=(c.P&~M6502_CF)|((l&0x80)?M6502_CF:0);l<<=1;_NZ(l);_SD(l);_WR();c.A|=l;_NZ(c.A);break;
      case 0x10:/*BPL #*/_A_IMM();_RD();if((c.P&0x80)==0x0){_RD();t=c.PC+(int8_t)_GD();if((t&0xFF00)!=(c.PC&0xFF00)){_RD();}c.PC=t;}break;
      case 0x11:/*ORA (zp),Y*/_A_IDY_R();_RD();c.A|=_GD();_NZ(c.A);break;
      case 0x12:/*INVALID*/break;
      case 0x13:/*SLO (zp),Y (undoc)*/_A_IDY_W();_RD();_WR();l=_GD();c.P=(c.P&~M6502_CF)|((l&0x80)?M6502_CF:0);l<<=1;_NZ(l);_SD(l);_WR();c.A|=l;_NZ(c.A);break;
      case 0x14:/*NOP zp,X (undoc)*/_A_ZPX();_RD();break;
      case 0x15:/*ORA zp,X*/_A_ZPX();_RD();c.A|=_GD();_NZ(c.A);break;
      case 0x16:/*ASL zp,X*/_A_ZPX();_RD();_WR();l=_GD();c.P=(c.P&~M6502_CF)|((l&0x80)?M6502_CF:0);l<<=1;_NZ(l);_SD(l);_WR();break;
      case 0x17:/*SLO zp,X (undoc)*/_A_ZPX();_RD();_WR();l=_GD();c.P=(c.P&~M6502_CF)|((l&0x80)?M6502_CF:0);l<<=1;_NZ(l);_SD(l);_WR();c.A|=l;_NZ(c.A);break;
      case 0x18:/*CLC */_A_IMP();_RD();c.P&=~0x1;break;
      case 0x19:/*ORA abs,Y*/_A_ABY_R();_RD();c.A|=_GD();_NZ(c.A);break;
      case 0x1a:/*NOP  (undoc)*/_A_IMP();_RD();break;
      case 0x1b:/*SLO abs,Y (undoc)*/_A_ABY_W();_RD();_WR();l=_GD();c.P=(c.P&~M6502_CF)|((l&0x80)?M6502_CF:0);l<<=1;_NZ(l);_SD(l);_WR();c.A|=l;_NZ(c.A);break;
      case 0x1c:/*NOP abs,X (undoc)*/_A_ABX_R();_RD();break;
      case 0x1d:/*ORA abs,X*/_A_ABX_R();_RD();c.A|=_GD();_NZ(c.A);break;
      case 0x1e:/*ASL abs,X*/_A_ABX_W();_RD();_WR();l=_GD();c.P=(c.P&~M6502_CF)|((l&0x80)?M6502_CF:0);l<<=1;_NZ(l);_SD(l);_WR();break;
      case 0x1f:/*SLO abs,X (undoc)*/_A_ABX_W();_RD();_WR();l=_GD();c.P=(c.P&~M6502_CF)|((l&0x80)?M6502_CF:0);l<<=1;_NZ(l);_SD(l);_WR();c.A|=l;_NZ(c.A);break;
      case 0x20:/*JSR */_SA(c.PC++);_RD();l=_GD();_SA(0x0100|c.S);_RD();_SAD(0x0100|c.S--,c.PC>>8);_WR();_SAD(0x0100|c.S--,c.PC);_WR();_SA(c.PC);_RD();h=_GD();c.PC=(h<<8)|l;break;
      case 0x21:/*AND (zp,X)*/_A_IDX();_RD();c.A&=_GD();_NZ(c.A);break;
      case 0x22:/*INVALID*/break;
      case 0x23:/*RLA (zp,X) (undoc)*/_A_IDX();_RD();_WR();l=_GD();{bool carry=c.P&M6502_CF;c.P&=~(M6502_NF|M6502_ZF|M6502_CF);if(l&0x80){c.P|=M6502_CF;}l<<=1;if(carry){l|=0x01;}_NZ(l);}_SD(l);_WR();c.A&=l;_NZ(c.A);break;
      case 0x24:/*BIT zp*/_A_ZER();_RD();l=_GD();h=c.A&l;c.P&=~(M6502_NF|M6502_VF|M6502_ZF);if(!h){c.P|=M6502_ZF;}c.P|=l&(M6502_NF|M6502_VF);break;
      case 0x25:/*AND zp*/_A_ZER();_RD();c.A&=_GD();_NZ(c.A);break;
      case 0x26:/*ROL zp*/_A_ZER();_RD();_WR();l=_GD();{bool carry=c.P&M6502_CF;c.P&=~(M6502_NF|M6502_ZF|M6502_CF);if(l&0x80){c.P|=M6502_CF;}l<<=1;if(carry){l|=0x01;}_NZ(l);}_SD(l);_WR();break;
      case 0x27:/*RLA zp (undoc)*/_A_ZER();_RD();_WR();l=_GD();{bool carry=c.P&M6502_CF;c.P&=~(M6502_NF|M6502_ZF|M6502_CF);if(l&0x80){c.P|=M6502_CF;}l<<=1;if(carry){l|=0x01;}_NZ(l);}_SD(l);_WR();c.A&=l;_NZ(c.A);break;
      case 0x28:/*PLP */_A_IMP();_RD();_SA(0x0100|c.S++);_RD();_SA(0x0100|c.S);_RD();c.P=(_GD()&~M6502_BF)|M6502_XF;break;
      case 0x29:/*AND #*/_A_IMM();_RD();c.A&=_GD();_NZ(c.A);break;
      case 0x2a:/*ROLA */_A_IMP();_RD();{bool carry=c.P&M6502_CF;c.P&=~(M6502_NF|M6502_ZF|M6502_CF);if(c.A&0x80){c.P|=M6502_CF;}c.A<<=1;if(carry){c.A|=0x01;}_NZ(c.A);}break;
      case 0x2b:/*ANC # (undoc)*/_A_IMM();_RD();c.A&=_GD();_NZ(c.A);if(c.A&0x80){c.P|=M6502_CF;}else{c.P&=~M6502_CF;}break;
      case 0x2c:/*BIT abs*/_A_ABS();_RD();l=_GD();h=c.A&l;c.P&=~(M6502_NF|M6502_VF|M6502_ZF);if(!h){c.P|=M6502_ZF;}c.P|=l&(M6502_NF|M6502_VF);break;
      case 0x2d:/*AND abs*/_A_ABS();_RD();c.A&=_GD();_NZ(c.A);break;
      case 0x2e:/*ROL abs*/_A_ABS();_RD();_WR();l=_GD();{bool carry=c.P&M6502_CF;c.P&=~(M6502_NF|M6502_ZF|M6502_CF);if(l&0x80){c.P|=M6502_CF;}l<<=1;if(carry){l|=0x01;}_NZ(l);}_SD(l);_WR();break;
      case 0x2f:/*RLA abs (undoc)*/_A_ABS();_RD();_WR();l=_GD();{bool carry=c.P&M6502_CF;c.P&=~(M6502_NF|M6502_ZF|M6502_CF);if(l&0x80){c.P|=M6502_CF;}l<<=1;if(carry){l|=0x01;}_NZ(l);}_SD(l);_WR();c.A&=l;_NZ(c.A);break;
      case 0x30:/*BMI #*/_A_IMM();_RD();if((c.P&0x80)==0x80){_RD();t=c.PC+(int8_t)_GD();if((t&0xFF00)!=(c.PC&0xFF00)){_RD();}c.PC=t;}break;
      case 0x31:/*AND (zp),Y*/_A_IDY_R();_RD();c.A&=_GD();_NZ(c.A);break;
      case 0x32:/*INVALID*/break;
      case 0x33:/*RLA (zp),Y (undoc)*/_A_IDY_W();_RD();_WR();l=_GD();{bool carry=c.P&M6502_CF;c.P&=~(M6502_NF|M6502_ZF|M6502_CF);if(l&0x80){c.P|=M6502_CF;}l<<=1;if(carry){l|=0x01;}_NZ(l);}_SD(l);_WR();c.A&=l;_NZ(c.A);break;
      case 0x34:/*NOP zp,X (undoc)*/_A_ZPX();_RD();break;
      case 0x35:/*AND zp,X*/_A_ZPX();_RD();c.A&=_GD();_NZ(c.A);break;
      case 0x36:/*ROL zp,X*/_A_ZPX();_RD();_WR();l=_GD();{bool carry=c.P&M6502_CF;c.P&=~(M6502_NF|M6502_ZF|M6502_CF);if(l&0x80){c.P|=M6502_CF;}l<<=1;if(carry){l|=0x01;}_NZ(l);}_SD(l);_WR();break;
      case 0x37:/*RLA zp,X (undoc)*/_A_ZPX();_RD();_WR();l=_GD();{bool carry=c.P&M6502_CF;c.P&=~(M6502_NF|M6502_ZF|M6502_CF);if(l&0x80){c.P|=M6502_CF;}l<<=1;if(carry){l|=0x01;}_NZ(l);}_SD(l);_WR();c.A&=l;_NZ(c.A);break;
      case 0x38:/*SEC */_A_IMP();_RD();c.P|=0x1;break;
      case 0x39:/*AND abs,Y*/_A_ABY_R();_RD();c.A&=_GD();_NZ(c.A);break;
      case 0x3a:/*NOP  (undoc)*/_A_IMP();_RD();break;
      case 0x3b:/*RLA abs,Y (undoc)*/_A_ABY_W();_RD();_WR();l=_GD();{bool carry=c.P&M6502_CF;c.P&=~(M6502_NF|M6502_ZF|M6502_CF);if(l&0x80){c.P|=M6502_CF;}l<<=1;if(carry){l|=0x01;}_NZ(l);}_SD(l);_WR();c.A&=l;_NZ(c.A);break;
      case 0x3c:/*NOP abs,X (undoc)*/_A_ABX_R();_RD();break;
      case 0x3d:/*AND abs,X*/_A_ABX_R();_RD();c.A&=_GD();_NZ(c.A);break;
      case 0x3e:/*ROL abs,X*/_A_ABX_W();_RD();_WR();l=_GD();{bool carry=c.P&M6502_CF;c.P&=~(M6502_NF|M6502_ZF|M6502_CF);if(l&0x80){c.P|=M6502_CF;}l<<=1;if(carry){l|=0x01;}_NZ(l);}_SD(l);_WR();break;
      case 0x3f:/*RLA abs,X (undoc)*/_A_ABX_W();_RD();_WR();l=_GD();{bool carry=c.P&M6502_CF;c.P&=~(M6502_NF|M6502_ZF|M6502_CF);if(l&0x80){c.P|=M6502_CF;}l<<=1;if(carry){l|=0x01;}_NZ(l);}_SD(l);_WR();c.A&=l;_NZ(c.A);break;
      case 0x40:/*RTI */_A_IMP();_RD();_SA(0x0100|c.S++);_RD();_SA(0x0100|c.S++);_RD();c.P=(_GD()&~M6502_BF)|M6502_XF;_SA(0x0100|c.S++);_RD();l=_GD();_SA(0x0100|c.S);_RD();h=_GD();c.PC=(h<<8)|l;c.pi=c.P;break;
      case 0x41:/*EOR (zp,X)*/_A_IDX();_RD();c.A^=_GD();_NZ(c.A);break;
      case 0x42:/*INVALID*/break;
      case 0x43:/*SRE (zp,X) (undoc)*/_A_IDX();_RD();_WR();l=_GD();c.P=(c.P&~M6502_CF)|((l&0x01)?M6502_CF:0);l>>=1;_NZ(l);_SD(l);_WR();c.A^=l;_NZ(c.A);break;
      case 0x44:/*NOP zp (undoc)*/_A_ZER();_RD();break;
      case 0x45:/*EOR zp*/_A_ZER();_RD();c.A^=_GD();_NZ(c.A);break;
      case 0x46:/*LSR zp*/_A_ZER();_RD();_WR();l=_GD();c.P=(c.P&~M6502_CF)|((l&0x01)?M6502_CF:0);l>>=1;_NZ(l);_SD(l);_WR();break;
      case 0x47:/*SRE zp (undoc)*/_A_ZER();_RD();_WR();l=_GD();c.P=(c.P&~M6502_CF)|((l&0x01)?M6502_CF:0);l>>=1;_NZ(l);_SD(l);_WR();c.A^=l;_NZ(c.A);break;
      case 0x48:/*PHA */_A_IMP();_RD();_SAD(0x0100|c.S--,c.A);_WR();break;
      case 0x49:/*EOR #*/_A_IMM();_RD();c.A^=_GD();_NZ(c.A);break;
      case 0x4a:/*LSRA */_A_IMP();_RD();c.P=(c.P&~M6502_CF)|((c.A&0x01)?M6502_CF:0);c.A>>=1;_NZ(c.A);break;
      case 0x4b:/*ASR # (undoc)*/_A_IMM();_RD();c.A&=_GD();c.P=(c.P&~M6502_CF)|((c.A&0x01)?M6502_CF:0);c.A>>=1;_NZ(c.A);break;
      case 0x4c:/*JMP */_SA(c.PC++);_RD();l=_GD();_SA(c.PC++);_RD();h=_GD();c.PC=(h<<8)|l;break;
      case 0x4d:/*EOR abs*/_A_ABS();_RD();c.A^=_GD();_NZ(c.A);break;
      case 0x4e:/*LSR abs*/_A_ABS();_RD();_WR();l=_GD();c.P=(c.P&~M6502_CF)|((l&0x01)?M6502_CF:0);l>>=1;_NZ(l);_SD(l);_WR();break;
      case 0x4f:/*SRE abs (undoc)*/_A_ABS();_RD();_WR();l=_GD();c.P=(c.P&~M6502_CF)|((l&0x01)?M6502_CF:0);l>>=1;_NZ(l);_SD(l);_WR();c.A^=l;_NZ(c.A);break;
      case 0x50:/*BVC #*/_A_IMM();_RD();if((c.P&0x40)==0x0){_RD();t=c.PC+(int8_t)_GD();if((t&0xFF00)!=(c.PC&0xFF00)){_RD();}c.PC=t;}break;
      case 0x51:/*EOR (zp),Y*/_A_IDY_R();_RD();c.A^=_GD();_NZ(c.A);break;
      case 0x52:/*INVALID*/break;
      case 0x53:/*SRE (zp),Y (undoc)*/_A_IDY_W();_RD();_WR();l=_GD();c.P=(c.P&~M6502_CF)|((l&0x01)?M6502_CF:0);l>>=1;_NZ(l);_SD(l);_WR();c.A^=l;_NZ(c.A);break;
      case 0x54:/*NOP zp,X (undoc)*/_A_ZPX();_RD();break;
      case 0x55:/*EOR zp,X*/_A_ZPX();_RD();c.A^=_GD();_NZ(c.A);break;
      case 0x56:/*LSR zp,X*/_A_ZPX();_RD();_WR();l=_GD();c.P=(c.P&~M6502_CF)|((l&0x01)?M6502_CF:0);l>>=1;_NZ(l);_SD(l);_WR();break;
      case 0x57:/*SRE zp,X (undoc)*/_A_ZPX();_RD();_WR();l=_GD();c.P=(c.P&~M6502_CF)|((l&0x01)?M6502_CF:0);l>>=1;_NZ(l);_SD(l);_WR();c.A^=l;_NZ(c.A);break;
      case 0x58:/*CLI */_A_IMP();_RD();c.P&=~0x4;break;
      case 0x59:/*EOR abs,Y*/_A_ABY_R();_RD();c.A^=_GD();_NZ(c.A);break;
      case 0x5a:/*NOP  (undoc)*/_A_IMP();_RD();break;
      case 0x5b:/*SRE abs,Y (undoc)*/_A_ABY_W();_RD();_WR();l=_GD();c.P=(c.P&~M6502_CF)|((l&0x01)?M6502_CF:0);l>>=1;_NZ(l);_SD(l);_WR();c.A^=l;_NZ(c.A);break;
      case 0x5c:/*NOP abs (undoc)*/_A_ABS();_RD();break;
      case 0x5d:/*EOR abs,X*/_A_ABX_R();_RD();c.A^=_GD();_NZ(c.A);break;
      case 0x5e:/*LSR abs,X*/_A_ABX_W();_RD();_WR();l=_GD();c.P=(c.P&~M6502_CF)|((l&0x01)?M6502_CF:0);l>>=1;_NZ(l);_SD(l);_WR();break;
      case 0x5f:/*SRE abs,X (undoc)*/_A_ABX_W();_RD();_WR();l=_GD();c.P=(c.P&~M6502_CF)|((l&0x01)?M6502_CF:0);l>>=1;_NZ(l);_SD(l);_WR();c.A^=l;_NZ(c.A);break;
      case 0x60:/*RTS */_A_IMP();_RD();_SA(0x0100|c.S++);_RD();_SA(0x0100|c.S++);_RD();l=_GD();_SA(0x0100|c.S);_RD();h=_GD();c.PC=(h<<8)|l;_SA(c.PC++);_RD();break;
      case 0x61:/*ADC (zp,X)*/_A_IDX();_RD();_m6502_adc(&c,_GD());break;
      case 0x62:/*INVALID*/break;
      case 0x63:/*RRA (zp,X) (undoc)*/_A_IDX();_RD();_WR();l=_GD();{bool carry=c.P&M6502_CF;c.P&=~(M6502_NF|M6502_ZF|M6502_CF);if(l&0x01){c.P|=M6502_CF;}l>>=1;if(carry){l|=0x80;}_NZ(l);}_SD(l);_WR();_m6502_adc(&c,l);break;
      case 0x64:/*NOP zp (undoc)*/_A_ZER();_RD();break;
      case 0x65:/*ADC zp*/_A_ZER();_RD();_m6502_adc(&c,_GD());break;
      case 0x66:/*ROR zp*/_A_ZER();_RD();_WR();l=_GD();{bool carry=c.P&M6502_CF;c.P&=~(M6502_NF|M6502_ZF|M6502_CF);if(l&0x01){c.P|=M6502_CF;}l>>=1;if(carry){l|=0x80;}_NZ(l);}_SD(l);_WR();break;
      case 0x67:/*RRA zp (undoc)*/_A_ZER();_RD();_WR();l=_GD();{bool carry=c.P&M6502_CF;c.P&=~(M6502_NF|M6502_ZF|M6502_CF);if(l&0x01){c.P|=M6502_CF;}l>>=1;if(carry){l|=0x80;}_NZ(l);}_SD(l);_WR();_m6502_adc(&c,l);break;
      case 0x68:/*PLA */_A_IMP();_RD();_SA(0x0100|c.S++);_RD();_SA(0x0100|c.S);_RD();c.A=_GD();_NZ(c.A);break;
      case 0x69:/*ADC #*/_A_IMM();_RD();_m6502_adc(&c,_GD());break;
      case 0x6a:/*RORA */_A_IMP();_RD();{bool carry=c.P&M6502_CF;c.P&=~(M6502_NF|M6502_ZF|M6502_CF);if(c.A&0x01){c.P|=M6502_CF;}c.A>>=1;if(carry){c.A|=0x80;}_NZ(c.A);}break;
      case 0x6b:/*ARR # (undoc)*/_A_IMM();_RD();c.A&=_GD();_m6502_arr(&c);break;
      case 0x6c:/*JMPI */_SA(c.PC++);_RD();l=_GD();_SA(c.PC++);_RD();h=_GD();a=(h<<8)|l;_SA(a);_RD();l=_GD();a=(a&0xFF00)|((a+1)&0x00FF);_SA(a);_RD();h=_GD();c.PC=(h<<8)|l;break;
      case 0x6d:/*ADC abs*/_A_ABS();_RD();_m6502_adc(&c,_GD());break;
      case 0x6e:/*ROR abs*/_A_ABS();_RD();_WR();l=_GD();{bool carry=c.P&M6502_CF;c.P&=~(M6502_NF|M6502_ZF|M6502_CF);if(l&0x01){c.P|=M6502_CF;}l>>=1;if(carry){l|=0x80;}_NZ(l);}_SD(l);_WR();break;
      case 0x6f:/*RRA abs (undoc)*/_A_ABS();_RD();_WR();l=_GD();{bool carry=c.P&M6502_CF;c.P&=~(M6502_NF|M6502_ZF|M6502_CF);if(l&0x01){c.P|=M6502_CF;}l>>=1;if(carry){l|=0x80;}_NZ(l);}_SD(l);_WR();_m6502_adc(&c,l);break;
      case 0x70:/*BVS #*/_A_IMM();_RD();if((c.P&0x40)==0x40){_RD();t=c.PC+(int8_t)_GD();if((t&0xFF00)!=(c.PC&0xFF00)){_RD();}c.PC=t;}break;
      case 0x71:/*ADC (zp),Y*/_A_IDY_R();_RD();_m6502_adc(&c,_GD());break;
      case 0x72:/*INVALID*/break;
      case 0x73:/*RRA (zp),Y (undoc)*/_A_IDY_W();_RD();_WR();l=_GD();{bool carry=c.P&M6502_CF;c.P&=~(M6502_NF|M6502_ZF|M6502_CF);if(l&0x01){c.P|=M6502_CF;}l>>=1;if(carry){l|=0x80;}_NZ(l);}_SD(l);_WR();_m6502_adc(&c,l);break;
      case 0x74:/*NOP zp,X (undoc)*/_A_ZPX();_RD();break;
      case 0x75:/*ADC zp,X*/_A_ZPX();_RD();_m6502_adc(&c,_GD());break;
      case 0x76:/*ROR zp,X*/_A_ZPX();_RD();_WR();l=_GD();{bool carry=c.P&M6502_CF;c.P&=~(M6502_NF|M6502_ZF|M6502_CF);if(l&0x01){c.P|=M6502_CF;}l>>=1;if(carry){l|=0x80;}_NZ(l);}_SD(l);_WR();break;
      case 0x77:/*RRA zp,X (undoc)*/_A_ZPX();_RD();_WR();l=_GD();{bool carry=c.P&M6502_CF;c.P&=~(M6502_NF|M6502_ZF|M6502_CF);if(l&0x01){c.P|=M6502_CF;}l>>=1;if(carry){l|=0x80;}_NZ(l);}_SD(l);_WR();_m6502_adc(&c,l);break;
      case 0x78:/*SEI */_A_IMP();_RD();c.P|=0x4;break;
      case 0x79:/*ADC abs,Y*/_A_ABY_R();_RD();_m6502_adc(&c,_GD());break;
      case 0x7a:/*NOP  (undoc)*/_A_IMP();_RD();break;
      case 0x7b:/*RRA abs,Y (undoc)*/_A_ABY_W();_RD();_WR();l=_GD();{bool carry=c.P&M6502_CF;c.P&=~(M6502_NF|M6502_ZF|M6502_CF);if(l&0x01){c.P|=M6502_CF;}l>>=1;if(carry){l|=0x80;}_NZ(l);}_SD(l);_WR();_m6502_adc(&c,l);break;
      case 0x7c:/*NOP abs (undoc)*/_A_ABS();_RD();break;
      case 0x7d:/*ADC abs,X*/_A_ABX_R();_RD();_m6502_adc(&c,_GD());break;
      case 0x7e:/*ROR abs,X*/_A_ABX_W();_RD();_WR();l=_GD();{bool carry=c.P&M6502_CF;c.P&=~(M6502_NF|M6502_ZF|M6502_CF);if(l&0x01){c.P|=M6502_CF;}l>>=1;if(carry){l|=0x80;}_NZ(l);}_SD(l);_WR();break;
      case 0x7f:/*RRA abs,X (undoc)*/_A_ABX_W();_RD();_WR();l=_GD();{bool carry=c.P&M6502_CF;c.P&=~(M6502_NF|M6502_ZF|M6502_CF);if(l&0x01){c.P|=M6502_CF;}l>>=1;if(carry){l|=0x80;}_NZ(l);}_SD(l);_WR();_m6502_adc(&c,l);break;
      case 0x80:/*NOP # (undoc)*/_A_IMM();_RD();break;
      case 0x81:/*STA (zp,X)*/_A_IDX();_SD(c.A);_WR();break;
      case 0x82:/*NOP # (undoc)*/_A_IMM();_RD();break;
      case 0x83:/*SAX (zp,X) (undoc)*/_A_IDX();_SD(c.A&c.X);_WR();break;
      case 0x84:/*STY zp*/_A_ZER();_SD(c.Y);_WR();break;
      case 0x85:/*STA zp*/_A_ZER();_SD(c.A);_WR();break;
      case 0x86:/*STX zp*/_A_ZER();_SD(c.X);_WR();break;
      case 0x87:/*SAX zp (undoc)*/_A_ZER();_SD(c.A&c.X);_WR();break;
      case 0x88:/*DEY */_A_IMP();_RD();c.Y--;_NZ(c.Y);break;
      case 0x89:/*NOP # (undoc)*/_A_IMM();_RD();break;
      case 0x8a:/*TXA */_A_IMP();_RD();c.A=c.X;_NZ(c.A);break;
      case 0x8b:/*ANE # (undoc)*/_A_IMM();_RD();l=_GD();c.A&=l&c.X;_NZ(c.A);break;
      case 0x8c:/*STY abs*/_A_ABS();_SD(c.Y);_WR();break;
      case 0x8d:/*STA abs*/_A_ABS();_SD(c.A);_WR();break;
      case 0x8e:/*STX abs*/_A_ABS();_SD(c.X);_WR();break;
      case 0x8f:/*SAX abs (undoc)*/_A_ABS();_SD(c.A&c.X);_WR();break;
      case 0x90:/*BCC #*/_A_IMM();_RD();if((c.P&0x1)==0x0){_RD();t=c.PC+(int8_t)_GD();if((t&0xFF00)!=(c.PC&0xFF00)){_RD();}c.PC=t;}break;
      case 0x91:/*STA (zp),Y*/_A_IDY_W();_SD(c.A);_WR();break;
      case 0x92:/*INVALID*/break;
      case 0x93:/*SHA (not impl) (zp),Y (undoc)*/_A_IDY_W();_RD();break;
      case 0x94:/*STY zp,X*/_A_ZPX();_SD(c.Y);_WR();break;
      case 0x95:/*STA zp,X*/_A_ZPX();_SD(c.A);_WR();break;
      case 0x96:/*STX zp,Y*/_A_ZPY();_SD(c.X);_WR();break;
      case 0x97:/*SAX zp,Y (undoc)*/_A_ZPY();_SD(c.A&c.X);_WR();break;
      case 0x98:/*TYA */_A_IMP();_RD();c.A=c.Y;_NZ(c.A);break;
      case 0x99:/*STA abs,Y*/_A_ABY_W();_SD(c.A);_WR();break;
      case 0x9a:/*TXS */_A_IMP();_RD();c.S=c.X;break;
      case 0x9b:/*SHS (not impl) abs,Y (undoc)*/_A_ABY_W();_RD();break;
      case 0x9c:/*SHY (not impl) abs,X (undoc)*/_A_ABX_W();_RD();break;
      case 0x9d:/*STA abs,X*/_A_ABX_W();_SD(c.A);_WR();break;
      case 0x9e:/*SHX (not impl) abs,Y (undoc)*/_A_ABY_W();_RD();break;
      case 0x9f:/*SHA (not impl) abs,Y (undoc)*/_A_ABY_W();_RD();break;
      case 0xa0:/*LDY #*/_A_IMM();_RD();c.Y=_GD();_NZ(c.Y);break;
      case 0xa1:/*LDA (zp,X)*/_A_IDX();_RD();c.A=_GD();_NZ(c.A);break;
      case 0xa2:/*LDX #*/_A_IMM();_RD();c.X=_GD();_NZ(c.X);break;
      case 0xa3:/*LAX (zp,X) (undoc)*/_A_IDX();_RD();c.A=c.X=_GD();_NZ(c.A);break;
      case 0xa4:/*LDY zp*/_A_ZER();_RD();c.Y=_GD();_NZ(c.Y);break;
      case 0xa5:/*LDA zp*/_A_ZER();_RD();c.A=_GD();_NZ(c.A);break;
      case 0xa6:/*LDX zp*/_A_ZER();_RD();c.X=_GD();_NZ(c.X);break;
      case 0xa7:/*LAX zp (undoc)*/_A_ZER();_RD();c.A=c.X=_GD();_NZ(c.A);break;
      case 0xa8:/*TAY */_A_IMP();_RD();c.Y=c.A;_NZ(c.Y);break;
      case 0xa9:/*LDA #*/_A_IMM();_RD();c.A=_GD();_NZ(c.A);break;
      case 0xaa:/*TAX */_A_IMP();_RD();c.X=c.A;_NZ(c.X);break;
      case 0xab:/*LXA # (undoc)*/_A_IMM();_RD();c.A&=_GD();c.X=c.A;_NZ(c.A);break;
      case 0xac:/*LDY abs*/_A_ABS();_RD();c.Y=_GD();_NZ(c.Y);break;
      case 0xad:/*LDA abs*/_A_ABS();_RD();c.A=_GD();_NZ(c.A);break;
      case 0xae:/*LDX abs*/_A_ABS();_RD();c.X=_GD();_NZ(c.X);break;
      case 0xaf:/*LAX abs (undoc)*/_A_ABS();_RD();c.A=c.X=_GD();_NZ(c.A);break;
      case 0xb0:/*BCS #*/_A_IMM();_RD();if((c.P&0x1)==0x1){_RD();t=c.PC+(int8_t)_GD();if((t&0xFF00)!=(c.PC&0xFF00)){_RD();}c.PC=t;}break;
      case 0xb1:/*LDA (zp),Y*/_A_IDY_R();_RD();c.A=_GD();_NZ(c.A);break;
      case 0xb2:/*INVALID*/break;
      case 0xb3:/*LAX (zp),Y (undoc)*/_A_IDY_R();_RD();c.A=c.X=_GD();_NZ(c.A);break;
      case 0xb4:/*LDY zp,X*/_A_ZPX();_RD();c.Y=_GD();_NZ(c.Y);break;
      case 0xb5:/*LDA zp,X*/_A_ZPX();_RD();c.A=_GD();_NZ(c.A);break;
      case 0xb6:/*LDX zp,Y*/_A_ZPY();_RD();c.X=_GD();_NZ(c.X);break;
      case 0xb7:/*LAX zp,Y (undoc)*/_A_ZPY();_RD();c.A=c.X=_GD();_NZ(c.A);break;
      case 0xb8:/*CLV */_A_IMP();_RD();c.P&=~0x40;break;
      case 0xb9:/*LDA abs,Y*/_A_ABY_R();_RD();c.A=_GD();_NZ(c.A);break;
      case 0xba:/*TSX */_A_IMP();_RD();c.X=c.S;_NZ(c.X);break;
      case 0xbb:/*LAS (not impl) abs,Y (undoc)*/_A_ABY_R();_RD();break;
      case 0xbc:/*LDY abs,X*/_A_ABX_R();_RD();c.Y=_GD();_NZ(c.Y);break;
      case 0xbd:/*LDA abs,X*/_A_ABX_R();_RD();c.A=_GD();_NZ(c.A);break;
      case 0xbe:/*LDX abs,Y*/_A_ABY_R();_RD();c.X=_GD();_NZ(c.X);break;
      case 0xbf:/*LAX abs,Y (undoc)*/_A_ABY_R();_RD();c.A=c.X=_GD();_NZ(c.A);break;
      case 0xc0:/*CPY #*/_A_IMM();_RD();l=_GD();t=c.Y-l;_NZ((uint8_t)t)&~M6502_CF;if(!(t&0xFF00)){c.P|=M6502_CF;}break;
      case 0xc1:/*CMP (zp,X)*/_A_IDX();_RD();l=_GD();t=c.A-l;_NZ((uint8_t)t)&~M6502_CF;if(!(t&0xFF00)){c.P|=M6502_CF;}break;
      case 0xc2:/*NOP # (undoc)*/_A_IMM();_RD();break;
      case 0xc3:/*DCP (zp,X) (undoc)*/_A_IDX();_RD();_WR();l=_GD();l--;_NZ(l);_SD(l);_WR();t=c.A-l;_NZ((uint8_t)t)&~M6502_CF;if(!(t&0xFF00)){c.P|=M6502_CF;}break;
      case 0xc4:/*CPY zp*/_A_ZER();_RD();l=_GD();t=c.Y-l;_NZ((uint8_t)t)&~M6502_CF;if(!(t&0xFF00)){c.P|=M6502_CF;}break;
      case 0xc5:/*CMP zp*/_A_ZER();_RD();l=_GD();t=c.A-l;_NZ((uint8_t)t)&~M6502_CF;if(!(t&0xFF00)){c.P|=M6502_CF;}break;
      case 0xc6:/*DEC zp*/_A_ZER();_RD();l=_GD();_WR();l--;_NZ(l);_SD(l);_WR();break;
      case 0xc7:/*DCP zp (undoc)*/_A_ZER();_RD();_WR();l=_GD();l--;_NZ(l);_SD(l);_WR();t=c.A-l;_NZ((uint8_t)t)&~M6502_CF;if(!(t&0xFF00)){c.P|=M6502_CF;}break;
      case 0xc8:/*INY */_A_IMP();_RD();c.Y++;_NZ(c.Y);break;
      case 0xc9:/*CMP #*/_A_IMM();_RD();l=_GD();t=c.A-l;_NZ((uint8_t)t)&~M6502_CF;if(!(t&0xFF00)){c.P|=M6502_CF;}break;
      case 0xca:/*DEX */_A_IMP();_RD();c.X--;_NZ(c.X);break;
      case 0xcb:/*SBX (not impl) # (undoc)*/_A_IMM();_RD();break;
      case 0xcc:/*CPY abs*/_A_ABS();_RD();l=_GD();t=c.Y-l;_NZ((uint8_t)t)&~M6502_CF;if(!(t&0xFF00)){c.P|=M6502_CF;}break;
      case 0xcd:/*CMP abs*/_A_ABS();_RD();l=_GD();t=c.A-l;_NZ((uint8_t)t)&~M6502_CF;if(!(t&0xFF00)){c.P|=M6502_CF;}break;
      case 0xce:/*DEC abs*/_A_ABS();_RD();l=_GD();_WR();l--;_NZ(l);_SD(l);_WR();break;
      case 0xcf:/*DCP abs (undoc)*/_A_ABS();_RD();_WR();l=_GD();l--;_NZ(l);_SD(l);_WR();t=c.A-l;_NZ((uint8_t)t)&~M6502_CF;if(!(t&0xFF00)){c.P|=M6502_CF;}break;
      case 0xd0:/*BNE #*/_A_IMM();_RD();if((c.P&0x2)==0x0){_RD();t=c.PC+(int8_t)_GD();if((t&0xFF00)!=(c.PC&0xFF00)){_RD();}c.PC=t;}break;
      case 0xd1:/*CMP (zp),Y*/_A_IDY_R();_RD();l=_GD();t=c.A-l;_NZ((uint8_t)t)&~M6502_CF;if(!(t&0xFF00)){c.P|=M6502_CF;}break;
      case 0xd2:/*INVALID*/break;
      case 0xd3:/*DCP (zp),Y (undoc)*/_A_IDY_W();_RD();_WR();l=_GD();l--;_NZ(l);_SD(l);_WR();t=c.A-l;_NZ((uint8_t)t)&~M6502_CF;if(!(t&0xFF00)){c.P|=M6502_CF;}break;
      case 0xd4:/*NOP zp,X (undoc)*/_A_ZPX();_RD();break;
      case 0xd5:/*CMP zp,X*/_A_ZPX();_RD();l=_GD();t=c.A-l;_NZ((uint8_t)t)&~M6502_CF;if(!(t&0xFF00)){c.P|=M6502_CF;}break;
      case 0xd6:/*DEC zp,X*/_A_ZPX();_RD();l=_GD();_WR();l--;_NZ(l);_SD(l);_WR();break;
      case 0xd7:/*DCP zp,X (undoc)*/_A_ZPX();_RD();_WR();l=_GD();l--;_NZ(l);_SD(l);_WR();t=c.A-l;_NZ((uint8_t)t)&~M6502_CF;if(!(t&0xFF00)){c.P|=M6502_CF;}break;
      case 0xd8:/*CLD */_A_IMP();_RD();c.P&=~0x8;break;
      case 0xd9:/*CMP abs,Y*/_A_ABY_R();_RD();l=_GD();t=c.A-l;_NZ((uint8_t)t)&~M6502_CF;if(!(t&0xFF00)){c.P|=M6502_CF;}break;
      case 0xda:/*NOP  (undoc)*/_A_IMP();_RD();break;
      case 0xdb:/*DCP abs,Y (undoc)*/_A_ABY_W();_RD();_WR();l=_GD();l--;_NZ(l);_SD(l);_WR();t=c.A-l;_NZ((uint8_t)t)&~M6502_CF;if(!(t&0xFF00)){c.P|=M6502_CF;}break;
      case 0xdc:/*NOP abs,X (undoc)*/_A_ABX_R();_RD();break;
      case 0xdd:/*CMP abs,X*/_A_ABX_R();_RD();l=_GD();t=c.A-l;_NZ((uint8_t)t)&~M6502_CF;if(!(t&0xFF00)){c.P|=M6502_CF;}break;
      case 0xde:/*DEC abs,X*/_A_ABX_W();_RD();l=_GD();_WR();l--;_NZ(l);_SD(l);_WR();break;
      case 0xdf:/*DCP abs,X (undoc)*/_A_ABX_W();_RD();_WR();l=_GD();l--;_NZ(l);_SD(l);_WR();t=c.A-l;_NZ((uint8_t)t)&~M6502_CF;if(!(t&0xFF00)){c.P|=M6502_CF;}break;
      case 0xe0:/*CPX #*/_A_IMM();_RD();l=_GD();t=c.X-l;_NZ((uint8_t)t)&~M6502_CF;if(!(t&0xFF00)){c.P|=M6502_CF;}break;
      case 0xe1:/*SBC (zp,X)*/_A_IDX();_RD();_m6502_sbc(&c,_GD());break;
      case 0xe2:/*NOP # (undoc)*/_A_IMM();_RD();break;
      case 0xe3:/*ISB (zp,X) (undoc)*/_A_IDX();_RD();_WR();l=_GD();l++;_SD(l);_WR();_m6502_sbc(&c,l);break;
      case 0xe4:/*CPX zp*/_A_ZER();_RD();l=_GD();t=c.X-l;_NZ((uint8_t)t)&~M6502_CF;if(!(t&0xFF00)){c.P|=M6502_CF;}break;
      case 0xe5:/*SBC zp*/_A_ZER();_RD();_m6502_sbc(&c,_GD());break;
      case 0xe6:/*INC zp*/_A_ZER();_RD();l=_GD();_WR();l++;_NZ(l);_SD(l);_WR();break;
      case 0xe7:/*ISB zp (undoc)*/_A_ZER();_RD();_WR();l=_GD();l++;_SD(l);_WR();_m6502_sbc(&c,l);break;
      case 0xe8:/*INX */_A_IMP();_RD();c.X++;_NZ(c.X);break;
      case 0xe9:/*SBC #*/_A_IMM();_RD();_m6502_sbc(&c,_GD());break;
      case 0xea:/*NOP */_A_IMP();_RD();break;
      case 0xeb:/*SBC # (undoc)*/_A_IMM();_RD();_m6502_sbc(&c,_GD());break;
      case 0xec:/*CPX abs*/_A_ABS();_RD();l=_GD();t=c.X-l;_NZ((uint8_t)t)&~M6502_CF;if(!(t&0xFF00)){c.P|=M6502_CF;}break;
      case 0xed:/*SBC abs*/_A_ABS();_RD();_m6502_sbc(&c,_GD());break;
      case 0xee:/*INC abs*/_A_ABS();_RD();l=_GD();_WR();l++;_NZ(l);_SD(l);_WR();break;
      case 0xef:/*ISB abs (undoc)*/_A_ABS();_RD();_WR();l=_GD();l++;_SD(l);_WR();_m6502_sbc(&c,l);break;
      case 0xf0:/*BEQ #*/_A_IMM();_RD();if((c.P&0x2)==0x2){_RD();t=c.PC+(int8_t)_GD();if((t&0xFF00)!=(c.PC&0xFF00)){_RD();}c.PC=t;}break;
      case 0xf1:/*SBC (zp),Y*/_A_IDY_R();_RD();_m6502_sbc(&c,_GD());break;
      case 0xf2:/*INVALID*/break;
      case 0xf3:/*ISB (zp),Y (undoc)*/_A_IDY_W();_RD();_WR();l=_GD();l++;_SD(l);_WR();_m6502_sbc(&c,l);break;
      case 0xf4:/*NOP zp,X (undoc)*/_A_ZPX();_RD();break;
      case 0xf5:/*SBC zp,X*/_A_ZPX();_RD();_m6502_sbc(&c,_GD());break;
      case 0xf6:/*INC zp,X*/_A_ZPX();_RD();l=_GD();_WR();l++;_NZ(l);_SD(l);_WR();break;
      case 0xf7:/*ISB zp,X (undoc)*/_A_ZPX();_RD();_WR();l=_GD();l++;_SD(l);_WR();_m6502_sbc(&c,l);break;
      case 0xf8:/*SED */_A_IMP();_RD();c.P|=0x8;break;
      case 0xf9:/*SBC abs,Y*/_A_ABY_R();_RD();_m6502_sbc(&c,_GD());break;
      case 0xfa:/*NOP  (undoc)*/_A_IMP();_RD();break;
      case 0xfb:/*ISB abs,Y (undoc)*/_A_ABY_W();_RD();_WR();l=_GD();l++;_SD(l);_WR();_m6502_sbc(&c,l);break;
      case 0xfc:/*NOP abs,X (undoc)*/_A_ABX_R();_RD();break;
      case 0xfd:/*SBC abs,X*/_A_ABX_R();_RD();_m6502_sbc(&c,_GD());break;
      case 0xfe:/*INC abs,X*/_A_ABX_W();_RD();l=_GD();_WR();l++;_NZ(l);_SD(l);_WR();break;
      case 0xff:/*ISB abs,X (undoc)*/_A_ABX_W();_RD();_WR();l=_GD();l++;_SD(l);_WR();_m6502_sbc(&c,l);break;
    }
    /* edge detection for NMI pin */
    bool nmi = 0 != ((pins & (pre_pins ^ pins)) & M6502_NMI);
    /* check for interrupt request */
    if (nmi || ((pins & M6502_IRQ) && !(c.pi & M6502_IF))) {
      /* execute a slightly modified BRK instruction, do NOT increment PC! */
      _SA(c.PC);_ON(M6502_SYNC);_RD();_OFF(M6502_SYNC);
      _SA(c.PC); _RD();
      _SAD(0x0100|c.S--, c.PC>>8); _WR();
      _SAD(0x0100|c.S--, c.PC); _WR();
      _SAD(0x0100|c.S--, c.P&~M6502_BF); _WR();
      if (pins & M6502_NMI) {
        _SA(0xFFFA); _RD(); l=_GD();
        c.P |= M6502_IF;
        _SA(0xFFFB); _RD(); h=_GD();
      }
      else {
        _SA(0xFFFE); _RD(); l=_GD();
        c.P |= M6502_IF;
        _SA(0xFFFF); _RD(); h=_GD();
      }
      c.PC = (h<<8)|l;
    }
    for (int i=0; i<M6502_MAX_NUM_TRAPS; i++) {
      if (cpu->trap_valid[i] && (c.PC==cpu->trap_addr[i])) {
        trap_id=i;
      }
    }
  } while ((ticks < num_ticks) && (trap_id < 0));
  c.PINS = pins;
  cpu->state = c;
  cpu->trap_id = trap_id;
  return ticks;
}
#undef _SA
#undef _SAD
#undef _GD
#undef _ON
#undef _OFF
#undef _T
#undef _RD
#undef _WR
#undef _A_IMP
#undef _A_IMM
#undef _A_ZER
#undef _A_ZPX
#undef _A_ZPY
#undef _A_ABS
#undef _A_ABX_R
#undef _A_ABX_W
#undef _A_ABY_R
#undef _A_ABY_W
#undef _A_IDX
#undef _A_IDY_R
#undef _A_IDY_W
#undef _NZ
