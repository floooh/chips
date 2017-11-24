// machine generated, do not edit!
static uint32_t _z80_op(z80* c, uint32_t ticks) {
  void(*tick)(z80*) = c->tick;
  uint8_t opcode; int8_t d; uint16_t a; uint8_t v; uint8_t l; uint8_t h;
  /*>fetch*/{c->CTRL|=Z80_M1;c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;opcode=c->DATA;c->CTRL&=~(Z80_M1|Z80_MREQ|Z80_RD);c->R=(c->R&0x80)|((c->R+1)&0x7F);c->CTRL|=Z80_RFSH;tick(c);ticks++;c->CTRL|=Z80_MREQ;c->ADDR=c->IR;tick(c);ticks++;c->CTRL&=~(Z80_RFSH|Z80_MREQ);}/*<fetch*/
  switch (opcode) {
    case 0x0:  return ticks; //NOP
    case 0x1: _IMM16(); c->BC=c->WZ;return ticks; //LD BC,nn
    case 0x2: c->WZ=c->BC;/*>wr*/c->ADDR=c->WZ++;tick(c);ticks++;c->CTRL|=(Z80_MREQ|Z80_WR);c->DATA=c->A;tick(c);ticks++;c->CTRL&=~(Z80_MREQ|Z80_WR);tick(c);ticks++;/*wr<*/;c->W=c->A;return ticks; //LD (BC),A
    case 0x3: tick(c);ticks++;tick(c);ticks++;c->BC++;return ticks; //INC BC
    case 0x4: c->B=_z80_inc(c,c->B);return ticks; //INC B
    case 0x5: c->B=_z80_dec(c,c->B);return ticks; //DEC B
    case 0x6: /*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;c->B=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/return ticks; //LD B,n
    case 0x7: _z80_rlca(c);return ticks; //RLCA
    case 0x8: _SWP16(c->AF,c->AF_);return ticks; //EX AF,AF'
    case 0x9: c->HL=_z80_add16(c,c->HL,c->BC);tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;return ticks; //ADD HL,BC
    case 0xa: c->WZ=c->BC;/*>rd*/c->ADDR=c->WZ++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;c->A=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;return ticks; //LD A,(BC)
    case 0xb: tick(c);ticks++;tick(c);ticks++;c->BC--;return ticks; //DEC BC
    case 0xc: c->C=_z80_inc(c,c->C);return ticks; //INC C
    case 0xd: c->C=_z80_dec(c,c->C);return ticks; //DEC C
    case 0xe: /*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;c->C=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/return ticks; //LD C,n
    case 0xf: _z80_rrca(c);return ticks; //RRCA
    case 0x10: ticks=_z80_djnz(c,tick,ticks);return ticks; //DJNZ
    case 0x11: _IMM16(); c->DE=c->WZ;return ticks; //LD DE,nn
    case 0x12: c->WZ=c->DE;/*>wr*/c->ADDR=c->WZ++;tick(c);ticks++;c->CTRL|=(Z80_MREQ|Z80_WR);c->DATA=c->A;tick(c);ticks++;c->CTRL&=~(Z80_MREQ|Z80_WR);tick(c);ticks++;/*wr<*/;c->W=c->A;return ticks; //LD (DE),A
    case 0x13: tick(c);ticks++;tick(c);ticks++;c->DE++;return ticks; //INC DE
    case 0x14: c->D=_z80_inc(c,c->D);return ticks; //INC D
    case 0x15: c->D=_z80_dec(c,c->D);return ticks; //DEC D
    case 0x16: /*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;c->D=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/return ticks; //LD D,n
    case 0x17: _z80_rla(c);return ticks; //RLA
    case 0x18: ticks=_z80_jr(c,tick,ticks);return ticks; //JR d
    case 0x19: c->HL=_z80_add16(c,c->HL,c->DE);tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;return ticks; //ADD HL,DE
    case 0x1a: c->WZ=c->DE;/*>rd*/c->ADDR=c->WZ++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;c->A=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;return ticks; //LD A,(DE)
    case 0x1b: tick(c);ticks++;tick(c);ticks++;c->DE--;return ticks; //DEC DE
    case 0x1c: c->E=_z80_inc(c,c->E);return ticks; //INC E
    case 0x1d: c->E=_z80_dec(c,c->E);return ticks; //DEC E
    case 0x1e: /*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;c->E=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/return ticks; //LD E,n
    case 0x1f: _z80_rra(c);return ticks; //RRA
    case 0x20: ticks=_z80_jr_cc(c,!(c->F&Z80_ZF),tick,ticks);return ticks; //JR NZ,d
    case 0x21: _IMM16(); c->HL=c->WZ;return ticks; //LD HL,nn
    case 0x22: _IMM16();/*>wr*/c->ADDR=c->WZ++;tick(c);ticks++;c->CTRL|=(Z80_MREQ|Z80_WR);c->DATA=(uint8_t)c->HL;tick(c);ticks++;c->CTRL&=~(Z80_MREQ|Z80_WR);tick(c);ticks++;/*wr<*//*>wr*/c->ADDR=c->WZ;tick(c);ticks++;c->CTRL|=(Z80_MREQ|Z80_WR);c->DATA=(uint8_t)(c->HL>>8);tick(c);ticks++;c->CTRL&=~(Z80_MREQ|Z80_WR);tick(c);ticks++;/*wr<*/return ticks; //LD (nn),HL
    case 0x23: tick(c);ticks++;tick(c);ticks++;c->HL++;return ticks; //INC HL
    case 0x24: c->H=_z80_inc(c,c->H);return ticks; //INC H
    case 0x25: c->H=_z80_dec(c,c->H);return ticks; //DEC H
    case 0x26: /*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;c->H=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/return ticks; //LD H,n
    case 0x27: _z80_daa(c);return ticks; //DAA
    case 0x28: ticks=_z80_jr_cc(c,(c->F&Z80_ZF),tick,ticks);return ticks; //JR Z,d
    case 0x29: c->HL=_z80_add16(c,c->HL,c->HL);tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;return ticks; //ADD HL,HL
    case 0x2a: _IMM16();/*>rd*/c->ADDR=c->WZ++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;l=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*//*>rd*/c->ADDR=c->WZ;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;h=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/c->HL=(h<<8)|l;return ticks; //LD HL,(nn)
    case 0x2b: tick(c);ticks++;tick(c);ticks++;c->HL--;return ticks; //DEC HL
    case 0x2c: c->L=_z80_inc(c,c->L);return ticks; //INC L
    case 0x2d: c->L=_z80_dec(c,c->L);return ticks; //DEC L
    case 0x2e: /*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;c->L=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/return ticks; //LD L,n
    case 0x2f: _z80_cpl(c);return ticks; //CPL
    case 0x30: ticks=_z80_jr_cc(c,!(c->F&Z80_CF),tick,ticks);return ticks; //JR NC,d
    case 0x31: _IMM16(); c->SP=c->WZ;return ticks; //LD SP,nn
    case 0x32: _IMM16();/*>wr*/c->ADDR=c->WZ++;tick(c);ticks++;c->CTRL|=(Z80_MREQ|Z80_WR);c->DATA=c->A;tick(c);ticks++;c->CTRL&=~(Z80_MREQ|Z80_WR);tick(c);ticks++;/*wr<*/;c->W=c->A;return ticks; //LD (nn),A
    case 0x33: tick(c);ticks++;tick(c);ticks++;c->SP++;return ticks; //INC SP
    case 0x34: a=c->HL;tick(c);ticks++;/*>rd*/c->ADDR=a;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;v=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*//*>wr*/c->ADDR=a;tick(c);ticks++;c->CTRL|=(Z80_MREQ|Z80_WR);c->DATA=_z80_inc(c,v);tick(c);ticks++;c->CTRL&=~(Z80_MREQ|Z80_WR);tick(c);ticks++;/*wr<*/return ticks; //INC (c->HL)
    case 0x35: a=c->HL;tick(c);ticks++;/*>rd*/c->ADDR=a;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;v=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*//*>wr*/c->ADDR=a;tick(c);ticks++;c->CTRL|=(Z80_MREQ|Z80_WR);c->DATA=_z80_dec(c,v);tick(c);ticks++;c->CTRL&=~(Z80_MREQ|Z80_WR);tick(c);ticks++;/*wr<*/return ticks; //DEC (c->HL)
    case 0x36: a=c->HL;/*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;v=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*//*>wr*/c->ADDR=a;tick(c);ticks++;c->CTRL|=(Z80_MREQ|Z80_WR);c->DATA=v;tick(c);ticks++;c->CTRL&=~(Z80_MREQ|Z80_WR);tick(c);ticks++;/*wr<*/return ticks; //LD (c->HL),n
    case 0x37: _z80_scf(c);return ticks; //SCF
    case 0x38: ticks=_z80_jr_cc(c,(c->F&Z80_CF),tick,ticks);return ticks; //JR C,d
    case 0x39: c->HL=_z80_add16(c,c->HL,c->SP);tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;return ticks; //ADD HL,SP
    case 0x3a: _IMM16();/*>rd*/c->ADDR=c->WZ++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;c->A=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;return ticks; //LD A,(nn)
    case 0x3b: tick(c);ticks++;tick(c);ticks++;c->SP--;return ticks; //DEC SP
    case 0x3c: c->A=_z80_inc(c,c->A);return ticks; //INC A
    case 0x3d: c->A=_z80_dec(c,c->A);return ticks; //DEC A
    case 0x3e: /*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;c->A=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/return ticks; //LD A,n
    case 0x3f: _z80_ccf(c);return ticks; //CCF
    case 0x40: c->B=c->B;return ticks; //LD B,B
    case 0x41: c->B=c->C;return ticks; //LD B,C
    case 0x42: c->B=c->D;return ticks; //LD B,D
    case 0x43: c->B=c->E;return ticks; //LD B,E
    case 0x44: c->B=c->H;return ticks; //LD B,H
    case 0x45: c->B=c->L;return ticks; //LD B,L
    case 0x46: {a=c->HL;/*>rd*/c->ADDR=a;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;c->B=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/}return ticks; //LD B,(c->HL)
    case 0x47: c->B=c->A;return ticks; //LD B,A
    case 0x48: c->C=c->B;return ticks; //LD C,B
    case 0x49: c->C=c->C;return ticks; //LD C,C
    case 0x4a: c->C=c->D;return ticks; //LD C,D
    case 0x4b: c->C=c->E;return ticks; //LD C,E
    case 0x4c: c->C=c->H;return ticks; //LD C,H
    case 0x4d: c->C=c->L;return ticks; //LD C,L
    case 0x4e: {a=c->HL;/*>rd*/c->ADDR=a;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;c->C=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/}return ticks; //LD C,(c->HL)
    case 0x4f: c->C=c->A;return ticks; //LD C,A
    case 0x50: c->D=c->B;return ticks; //LD D,B
    case 0x51: c->D=c->C;return ticks; //LD D,C
    case 0x52: c->D=c->D;return ticks; //LD D,D
    case 0x53: c->D=c->E;return ticks; //LD D,E
    case 0x54: c->D=c->H;return ticks; //LD D,H
    case 0x55: c->D=c->L;return ticks; //LD D,L
    case 0x56: {a=c->HL;/*>rd*/c->ADDR=a;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;c->D=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/}return ticks; //LD D,(c->HL)
    case 0x57: c->D=c->A;return ticks; //LD D,A
    case 0x58: c->E=c->B;return ticks; //LD E,B
    case 0x59: c->E=c->C;return ticks; //LD E,C
    case 0x5a: c->E=c->D;return ticks; //LD E,D
    case 0x5b: c->E=c->E;return ticks; //LD E,E
    case 0x5c: c->E=c->H;return ticks; //LD E,H
    case 0x5d: c->E=c->L;return ticks; //LD E,L
    case 0x5e: {a=c->HL;/*>rd*/c->ADDR=a;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;c->E=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/}return ticks; //LD E,(c->HL)
    case 0x5f: c->E=c->A;return ticks; //LD E,A
    case 0x60: c->H=c->B;return ticks; //LD H,B
    case 0x61: c->H=c->C;return ticks; //LD H,C
    case 0x62: c->H=c->D;return ticks; //LD H,D
    case 0x63: c->H=c->E;return ticks; //LD H,E
    case 0x64: c->H=c->H;return ticks; //LD H,H
    case 0x65: c->H=c->L;return ticks; //LD H,L
    case 0x66: {a=c->HL;/*>rd*/c->ADDR=a;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;c->H=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/}return ticks; //LD H,(c->HL)
    case 0x67: c->H=c->A;return ticks; //LD H,A
    case 0x68: c->L=c->B;return ticks; //LD L,B
    case 0x69: c->L=c->C;return ticks; //LD L,C
    case 0x6a: c->L=c->D;return ticks; //LD L,D
    case 0x6b: c->L=c->E;return ticks; //LD L,E
    case 0x6c: c->L=c->H;return ticks; //LD L,H
    case 0x6d: c->L=c->L;return ticks; //LD L,L
    case 0x6e: {a=c->HL;/*>rd*/c->ADDR=a;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;c->L=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/}return ticks; //LD L,(c->HL)
    case 0x6f: c->L=c->A;return ticks; //LD L,A
    case 0x70: {a=c->HL;/*>wr*/c->ADDR=a;tick(c);ticks++;c->CTRL|=(Z80_MREQ|Z80_WR);c->DATA=c->B;tick(c);ticks++;c->CTRL&=~(Z80_MREQ|Z80_WR);tick(c);ticks++;/*wr<*/;}return ticks; //LD (c->HL),B
    case 0x71: {a=c->HL;/*>wr*/c->ADDR=a;tick(c);ticks++;c->CTRL|=(Z80_MREQ|Z80_WR);c->DATA=c->C;tick(c);ticks++;c->CTRL&=~(Z80_MREQ|Z80_WR);tick(c);ticks++;/*wr<*/;}return ticks; //LD (c->HL),C
    case 0x72: {a=c->HL;/*>wr*/c->ADDR=a;tick(c);ticks++;c->CTRL|=(Z80_MREQ|Z80_WR);c->DATA=c->D;tick(c);ticks++;c->CTRL&=~(Z80_MREQ|Z80_WR);tick(c);ticks++;/*wr<*/;}return ticks; //LD (c->HL),D
    case 0x73: {a=c->HL;/*>wr*/c->ADDR=a;tick(c);ticks++;c->CTRL|=(Z80_MREQ|Z80_WR);c->DATA=c->E;tick(c);ticks++;c->CTRL&=~(Z80_MREQ|Z80_WR);tick(c);ticks++;/*wr<*/;}return ticks; //LD (c->HL),E
    case 0x74: {a=c->HL;/*>wr*/c->ADDR=a;tick(c);ticks++;c->CTRL|=(Z80_MREQ|Z80_WR);c->DATA=c->H;tick(c);ticks++;c->CTRL&=~(Z80_MREQ|Z80_WR);tick(c);ticks++;/*wr<*/;}return ticks; //LD (c->HL),H
    case 0x75: {a=c->HL;/*>wr*/c->ADDR=a;tick(c);ticks++;c->CTRL|=(Z80_MREQ|Z80_WR);c->DATA=c->L;tick(c);ticks++;c->CTRL&=~(Z80_MREQ|Z80_WR);tick(c);ticks++;/*wr<*/;}return ticks; //LD (c->HL),L
    case 0x76: _z80_halt(c);return ticks; //HALT
    case 0x77: {a=c->HL;/*>wr*/c->ADDR=a;tick(c);ticks++;c->CTRL|=(Z80_MREQ|Z80_WR);c->DATA=c->A;tick(c);ticks++;c->CTRL&=~(Z80_MREQ|Z80_WR);tick(c);ticks++;/*wr<*/;}return ticks; //LD (c->HL),A
    case 0x78: c->A=c->B;return ticks; //LD A,B
    case 0x79: c->A=c->C;return ticks; //LD A,C
    case 0x7a: c->A=c->D;return ticks; //LD A,D
    case 0x7b: c->A=c->E;return ticks; //LD A,E
    case 0x7c: c->A=c->H;return ticks; //LD A,H
    case 0x7d: c->A=c->L;return ticks; //LD A,L
    case 0x7e: {a=c->HL;/*>rd*/c->ADDR=a;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;c->A=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/}return ticks; //LD A,(c->HL)
    case 0x7f: c->A=c->A;return ticks; //LD A,A
    case 0x80: _z80_add(c,c->B);return ticks; //ADD B
    case 0x81: _z80_add(c,c->C);return ticks; //ADD C
    case 0x82: _z80_add(c,c->D);return ticks; //ADD D
    case 0x83: _z80_add(c,c->E);return ticks; //ADD E
    case 0x84: _z80_add(c,c->H);return ticks; //ADD H
    case 0x85: _z80_add(c,c->L);return ticks; //ADD L
    case 0x86: a=c->HL;/*>rd*/c->ADDR=a;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;v=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/_z80_add(c,v);return ticks; //ADD (c->HL)
    case 0x87: _z80_add(c,c->A);return ticks; //ADD A
    case 0x88: _z80_adc(c,c->B);return ticks; //ADC B
    case 0x89: _z80_adc(c,c->C);return ticks; //ADC C
    case 0x8a: _z80_adc(c,c->D);return ticks; //ADC D
    case 0x8b: _z80_adc(c,c->E);return ticks; //ADC E
    case 0x8c: _z80_adc(c,c->H);return ticks; //ADC H
    case 0x8d: _z80_adc(c,c->L);return ticks; //ADC L
    case 0x8e: a=c->HL;/*>rd*/c->ADDR=a;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;v=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/_z80_adc(c,v);return ticks; //ADC (c->HL)
    case 0x8f: _z80_adc(c,c->A);return ticks; //ADC A
    case 0x90: _z80_sub(c,c->B);return ticks; //SUB B
    case 0x91: _z80_sub(c,c->C);return ticks; //SUB C
    case 0x92: _z80_sub(c,c->D);return ticks; //SUB D
    case 0x93: _z80_sub(c,c->E);return ticks; //SUB E
    case 0x94: _z80_sub(c,c->H);return ticks; //SUB H
    case 0x95: _z80_sub(c,c->L);return ticks; //SUB L
    case 0x96: a=c->HL;/*>rd*/c->ADDR=a;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;v=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/_z80_sub(c,v);return ticks; //SUB (c->HL)
    case 0x97: _z80_sub(c,c->A);return ticks; //SUB A
    case 0x98: _z80_sbc(c,c->B);return ticks; //SBC B
    case 0x99: _z80_sbc(c,c->C);return ticks; //SBC C
    case 0x9a: _z80_sbc(c,c->D);return ticks; //SBC D
    case 0x9b: _z80_sbc(c,c->E);return ticks; //SBC E
    case 0x9c: _z80_sbc(c,c->H);return ticks; //SBC H
    case 0x9d: _z80_sbc(c,c->L);return ticks; //SBC L
    case 0x9e: a=c->HL;/*>rd*/c->ADDR=a;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;v=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/_z80_sbc(c,v);return ticks; //SBC (c->HL)
    case 0x9f: _z80_sbc(c,c->A);return ticks; //SBC A
    case 0xa0: _z80_and(c,c->B);return ticks; //AND B
    case 0xa1: _z80_and(c,c->C);return ticks; //AND C
    case 0xa2: _z80_and(c,c->D);return ticks; //AND D
    case 0xa3: _z80_and(c,c->E);return ticks; //AND E
    case 0xa4: _z80_and(c,c->H);return ticks; //AND H
    case 0xa5: _z80_and(c,c->L);return ticks; //AND L
    case 0xa6: a=c->HL;/*>rd*/c->ADDR=a;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;v=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/_z80_and(c,v);return ticks; //AND (c->HL)
    case 0xa7: _z80_and(c,c->A);return ticks; //AND A
    case 0xa8: _z80_xor(c,c->B);return ticks; //XOR B
    case 0xa9: _z80_xor(c,c->C);return ticks; //XOR C
    case 0xaa: _z80_xor(c,c->D);return ticks; //XOR D
    case 0xab: _z80_xor(c,c->E);return ticks; //XOR E
    case 0xac: _z80_xor(c,c->H);return ticks; //XOR H
    case 0xad: _z80_xor(c,c->L);return ticks; //XOR L
    case 0xae: a=c->HL;/*>rd*/c->ADDR=a;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;v=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/_z80_xor(c,v);return ticks; //XOR (c->HL)
    case 0xaf: _z80_xor(c,c->A);return ticks; //XOR A
    case 0xb0: _z80_or(c,c->B);return ticks; //OR B
    case 0xb1: _z80_or(c,c->C);return ticks; //OR C
    case 0xb2: _z80_or(c,c->D);return ticks; //OR D
    case 0xb3: _z80_or(c,c->E);return ticks; //OR E
    case 0xb4: _z80_or(c,c->H);return ticks; //OR H
    case 0xb5: _z80_or(c,c->L);return ticks; //OR L
    case 0xb6: a=c->HL;/*>rd*/c->ADDR=a;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;v=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/_z80_or(c,v);return ticks; //OR (c->HL)
    case 0xb7: _z80_or(c,c->A);return ticks; //OR A
    case 0xb8: _z80_cp(c,c->B);return ticks; //CP B
    case 0xb9: _z80_cp(c,c->C);return ticks; //CP C
    case 0xba: _z80_cp(c,c->D);return ticks; //CP D
    case 0xbb: _z80_cp(c,c->E);return ticks; //CP E
    case 0xbc: _z80_cp(c,c->H);return ticks; //CP H
    case 0xbd: _z80_cp(c,c->L);return ticks; //CP L
    case 0xbe: a=c->HL;/*>rd*/c->ADDR=a;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;v=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/_z80_cp(c,v);return ticks; //CP (c->HL)
    case 0xbf: _z80_cp(c,c->A);return ticks; //CP A
    case 0xc0: ticks=_z80_retcc(c,!(c->F&Z80_ZF),tick,ticks);return ticks; //RET NZ
    case 0xc1: /*>rd*/c->ADDR=c->SP++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;l=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*//*>rd*/c->ADDR=c->SP++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;h=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;c->BC=(h<<8)|l;return ticks; //POP BC
    case 0xc2: _IMM16(); if (!(c->F&Z80_ZF)) { c->PC=c->WZ; }return ticks; //JP NZ,nn
    case 0xc3: _IMM16(); c->PC=c->WZ;return ticks; //JP nn
    case 0xc4: ticks=_z80_callcc(c,!(c->F&Z80_ZF),tick,ticks);return ticks; //CALL NZ,nn
    case 0xc5: _T();_WR(--c->SP,(uint8_t)(c->BC>>8)); _WR(--c->SP,(uint8_t)c->BC);return ticks; //PUSH BC
    case 0xc6: {uint8_t v;_RD(c->PC++,v);_z80_add(c,v);}return ticks; //ADD n
    case 0xc7: ticks=_z80_rst(c,0x0,tick,ticks);return ticks; //RST 0x0
    case 0xc8: ticks=_z80_retcc(c,(c->F&Z80_ZF),tick,ticks);return ticks; //RET Z
    case 0xc9: ticks=_z80_ret(c,tick,ticks);return ticks; //RET
    case 0xca: _IMM16(); if ((c->F&Z80_ZF)) { c->PC=c->WZ; }return ticks; //JP Z,nn
    case 0xcb:
      /*>fetch*/{c->CTRL|=Z80_M1;c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;opcode=c->DATA;c->CTRL&=~(Z80_M1|Z80_MREQ|Z80_RD);c->R=(c->R&0x80)|((c->R+1)&0x7F);c->CTRL|=Z80_RFSH;tick(c);ticks++;c->CTRL|=Z80_MREQ;c->ADDR=c->IR;tick(c);ticks++;c->CTRL&=~(Z80_RFSH|Z80_MREQ);}/*<fetch*/
      switch (opcode) {
        case 0x0: c->B=_z80_rlc(c,c->B);return ticks; //RLC B
        case 0x1: c->C=_z80_rlc(c,c->C);return ticks; //RLC C
        case 0x2: c->D=_z80_rlc(c,c->D);return ticks; //RLC D
        case 0x3: c->E=_z80_rlc(c,c->E);return ticks; //RLC E
        case 0x4: c->H=_z80_rlc(c,c->H);return ticks; //RLC H
        case 0x5: c->L=_z80_rlc(c,c->L);return ticks; //RLC L
        case 0x6: { a=c->HL;;_T();uint8_t v;_RD(a,v);_WR(a,_z80_rlc(c,v)); }return ticks; //RLC (c->HL)
        case 0x7: c->A=_z80_rlc(c,c->A);return ticks; //RLC A
        case 0x8: c->B=_z80_rrc(c,c->B);return ticks; //RRC B
        case 0x9: c->C=_z80_rrc(c,c->C);return ticks; //RRC C
        case 0xa: c->D=_z80_rrc(c,c->D);return ticks; //RRC D
        case 0xb: c->E=_z80_rrc(c,c->E);return ticks; //RRC E
        case 0xc: c->H=_z80_rrc(c,c->H);return ticks; //RRC H
        case 0xd: c->L=_z80_rrc(c,c->L);return ticks; //RRC L
        case 0xe: { a=c->HL;;_T();uint8_t v;_RD(a,v);_WR(a,_z80_rrc(c,v)); }return ticks; //RRC (c->HL)
        case 0xf: c->A=_z80_rrc(c,c->A);return ticks; //RRC A
        case 0x10: c->B=_z80_rl(c,c->B);return ticks; //RL B
        case 0x11: c->C=_z80_rl(c,c->C);return ticks; //RL C
        case 0x12: c->D=_z80_rl(c,c->D);return ticks; //RL D
        case 0x13: c->E=_z80_rl(c,c->E);return ticks; //RL E
        case 0x14: c->H=_z80_rl(c,c->H);return ticks; //RL H
        case 0x15: c->L=_z80_rl(c,c->L);return ticks; //RL L
        case 0x16: { a=c->HL;;_T();uint8_t v;_RD(a,v);_WR(a,_z80_rl(c,v)); }return ticks; //RL (c->HL)
        case 0x17: c->A=_z80_rl(c,c->A);return ticks; //RL A
        case 0x18: c->B=_z80_rr(c,c->B);return ticks; //RR B
        case 0x19: c->C=_z80_rr(c,c->C);return ticks; //RR C
        case 0x1a: c->D=_z80_rr(c,c->D);return ticks; //RR D
        case 0x1b: c->E=_z80_rr(c,c->E);return ticks; //RR E
        case 0x1c: c->H=_z80_rr(c,c->H);return ticks; //RR H
        case 0x1d: c->L=_z80_rr(c,c->L);return ticks; //RR L
        case 0x1e: { a=c->HL;;_T();uint8_t v;_RD(a,v);_WR(a,_z80_rr(c,v)); }return ticks; //RR (c->HL)
        case 0x1f: c->A=_z80_rr(c,c->A);return ticks; //RR A
        case 0x20: c->B=_z80_sla(c,c->B);return ticks; //SLA B
        case 0x21: c->C=_z80_sla(c,c->C);return ticks; //SLA C
        case 0x22: c->D=_z80_sla(c,c->D);return ticks; //SLA D
        case 0x23: c->E=_z80_sla(c,c->E);return ticks; //SLA E
        case 0x24: c->H=_z80_sla(c,c->H);return ticks; //SLA H
        case 0x25: c->L=_z80_sla(c,c->L);return ticks; //SLA L
        case 0x26: { a=c->HL;;_T();uint8_t v;_RD(a,v);_WR(a,_z80_sla(c,v)); }return ticks; //SLA (c->HL)
        case 0x27: c->A=_z80_sla(c,c->A);return ticks; //SLA A
        case 0x28: c->B=_z80_sra(c,c->B);return ticks; //SRA B
        case 0x29: c->C=_z80_sra(c,c->C);return ticks; //SRA C
        case 0x2a: c->D=_z80_sra(c,c->D);return ticks; //SRA D
        case 0x2b: c->E=_z80_sra(c,c->E);return ticks; //SRA E
        case 0x2c: c->H=_z80_sra(c,c->H);return ticks; //SRA H
        case 0x2d: c->L=_z80_sra(c,c->L);return ticks; //SRA L
        case 0x2e: { a=c->HL;;_T();uint8_t v;_RD(a,v);_WR(a,_z80_sra(c,v)); }return ticks; //SRA (c->HL)
        case 0x2f: c->A=_z80_sra(c,c->A);return ticks; //SRA A
        case 0x30: c->B=_z80_sll(c,c->B);return ticks; //SLL B
        case 0x31: c->C=_z80_sll(c,c->C);return ticks; //SLL C
        case 0x32: c->D=_z80_sll(c,c->D);return ticks; //SLL D
        case 0x33: c->E=_z80_sll(c,c->E);return ticks; //SLL E
        case 0x34: c->H=_z80_sll(c,c->H);return ticks; //SLL H
        case 0x35: c->L=_z80_sll(c,c->L);return ticks; //SLL L
        case 0x36: { a=c->HL;;_T();uint8_t v;_RD(a,v);_WR(a,_z80_sll(c,v)); }return ticks; //SLL (c->HL)
        case 0x37: c->A=_z80_sll(c,c->A);return ticks; //SLL A
        case 0x38: c->B=_z80_srl(c,c->B);return ticks; //SRL B
        case 0x39: c->C=_z80_srl(c,c->C);return ticks; //SRL C
        case 0x3a: c->D=_z80_srl(c,c->D);return ticks; //SRL D
        case 0x3b: c->E=_z80_srl(c,c->E);return ticks; //SRL E
        case 0x3c: c->H=_z80_srl(c,c->H);return ticks; //SRL H
        case 0x3d: c->L=_z80_srl(c,c->L);return ticks; //SRL L
        case 0x3e: { a=c->HL;;_T();uint8_t v;_RD(a,v);_WR(a,_z80_srl(c,v)); }return ticks; //SRL (c->HL)
        case 0x3f: c->A=_z80_srl(c,c->A);return ticks; //SRL A
        case 0x40: _z80_bit(c,c->B,0x1);return ticks; //BIT 0,B
        case 0x41: _z80_bit(c,c->C,0x1);return ticks; //BIT 0,C
        case 0x42: _z80_bit(c,c->D,0x1);return ticks; //BIT 0,D
        case 0x43: _z80_bit(c,c->E,0x1);return ticks; //BIT 0,E
        case 0x44: _z80_bit(c,c->H,0x1);return ticks; //BIT 0,H
        case 0x45: _z80_bit(c,c->L,0x1);return ticks; //BIT 0,L
        case 0x46: { a=c->HL;;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x1); }return ticks; //BIT 0,(c->HL)
        case 0x47: _z80_bit(c,c->A,0x1);return ticks; //BIT 0,A
        case 0x48: _z80_bit(c,c->B,0x2);return ticks; //BIT 1,B
        case 0x49: _z80_bit(c,c->C,0x2);return ticks; //BIT 1,C
        case 0x4a: _z80_bit(c,c->D,0x2);return ticks; //BIT 1,D
        case 0x4b: _z80_bit(c,c->E,0x2);return ticks; //BIT 1,E
        case 0x4c: _z80_bit(c,c->H,0x2);return ticks; //BIT 1,H
        case 0x4d: _z80_bit(c,c->L,0x2);return ticks; //BIT 1,L
        case 0x4e: { a=c->HL;;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x2); }return ticks; //BIT 1,(c->HL)
        case 0x4f: _z80_bit(c,c->A,0x2);return ticks; //BIT 1,A
        case 0x50: _z80_bit(c,c->B,0x4);return ticks; //BIT 2,B
        case 0x51: _z80_bit(c,c->C,0x4);return ticks; //BIT 2,C
        case 0x52: _z80_bit(c,c->D,0x4);return ticks; //BIT 2,D
        case 0x53: _z80_bit(c,c->E,0x4);return ticks; //BIT 2,E
        case 0x54: _z80_bit(c,c->H,0x4);return ticks; //BIT 2,H
        case 0x55: _z80_bit(c,c->L,0x4);return ticks; //BIT 2,L
        case 0x56: { a=c->HL;;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x4); }return ticks; //BIT 2,(c->HL)
        case 0x57: _z80_bit(c,c->A,0x4);return ticks; //BIT 2,A
        case 0x58: _z80_bit(c,c->B,0x8);return ticks; //BIT 3,B
        case 0x59: _z80_bit(c,c->C,0x8);return ticks; //BIT 3,C
        case 0x5a: _z80_bit(c,c->D,0x8);return ticks; //BIT 3,D
        case 0x5b: _z80_bit(c,c->E,0x8);return ticks; //BIT 3,E
        case 0x5c: _z80_bit(c,c->H,0x8);return ticks; //BIT 3,H
        case 0x5d: _z80_bit(c,c->L,0x8);return ticks; //BIT 3,L
        case 0x5e: { a=c->HL;;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x8); }return ticks; //BIT 3,(c->HL)
        case 0x5f: _z80_bit(c,c->A,0x8);return ticks; //BIT 3,A
        case 0x60: _z80_bit(c,c->B,0x10);return ticks; //BIT 4,B
        case 0x61: _z80_bit(c,c->C,0x10);return ticks; //BIT 4,C
        case 0x62: _z80_bit(c,c->D,0x10);return ticks; //BIT 4,D
        case 0x63: _z80_bit(c,c->E,0x10);return ticks; //BIT 4,E
        case 0x64: _z80_bit(c,c->H,0x10);return ticks; //BIT 4,H
        case 0x65: _z80_bit(c,c->L,0x10);return ticks; //BIT 4,L
        case 0x66: { a=c->HL;;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x10); }return ticks; //BIT 4,(c->HL)
        case 0x67: _z80_bit(c,c->A,0x10);return ticks; //BIT 4,A
        case 0x68: _z80_bit(c,c->B,0x20);return ticks; //BIT 5,B
        case 0x69: _z80_bit(c,c->C,0x20);return ticks; //BIT 5,C
        case 0x6a: _z80_bit(c,c->D,0x20);return ticks; //BIT 5,D
        case 0x6b: _z80_bit(c,c->E,0x20);return ticks; //BIT 5,E
        case 0x6c: _z80_bit(c,c->H,0x20);return ticks; //BIT 5,H
        case 0x6d: _z80_bit(c,c->L,0x20);return ticks; //BIT 5,L
        case 0x6e: { a=c->HL;;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x20); }return ticks; //BIT 5,(c->HL)
        case 0x6f: _z80_bit(c,c->A,0x20);return ticks; //BIT 5,A
        case 0x70: _z80_bit(c,c->B,0x40);return ticks; //BIT 6,B
        case 0x71: _z80_bit(c,c->C,0x40);return ticks; //BIT 6,C
        case 0x72: _z80_bit(c,c->D,0x40);return ticks; //BIT 6,D
        case 0x73: _z80_bit(c,c->E,0x40);return ticks; //BIT 6,E
        case 0x74: _z80_bit(c,c->H,0x40);return ticks; //BIT 6,H
        case 0x75: _z80_bit(c,c->L,0x40);return ticks; //BIT 6,L
        case 0x76: { a=c->HL;;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x40); }return ticks; //BIT 6,(c->HL)
        case 0x77: _z80_bit(c,c->A,0x40);return ticks; //BIT 6,A
        case 0x78: _z80_bit(c,c->B,0x80);return ticks; //BIT 7,B
        case 0x79: _z80_bit(c,c->C,0x80);return ticks; //BIT 7,C
        case 0x7a: _z80_bit(c,c->D,0x80);return ticks; //BIT 7,D
        case 0x7b: _z80_bit(c,c->E,0x80);return ticks; //BIT 7,E
        case 0x7c: _z80_bit(c,c->H,0x80);return ticks; //BIT 7,H
        case 0x7d: _z80_bit(c,c->L,0x80);return ticks; //BIT 7,L
        case 0x7e: { a=c->HL;;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x80); }return ticks; //BIT 7,(c->HL)
        case 0x7f: _z80_bit(c,c->A,0x80);return ticks; //BIT 7,A
        case 0x80: c->B&=~0x1;return ticks; //RES 0,B
        case 0x81: c->C&=~0x1;return ticks; //RES 0,C
        case 0x82: c->D&=~0x1;return ticks; //RES 0,D
        case 0x83: c->E&=~0x1;return ticks; //RES 0,E
        case 0x84: c->H&=~0x1;return ticks; //RES 0,H
        case 0x85: c->L&=~0x1;return ticks; //RES 0,L
        case 0x86: { a=c->HL;;_T();uint8_t v;_RD(a,v);_WR(a,v&~0x1); }return ticks; //RES 0,(c->HL)
        case 0x87: c->A&=~0x1;return ticks; //RES 0,A
        case 0x88: c->B&=~0x2;return ticks; //RES 1,B
        case 0x89: c->C&=~0x2;return ticks; //RES 1,C
        case 0x8a: c->D&=~0x2;return ticks; //RES 1,D
        case 0x8b: c->E&=~0x2;return ticks; //RES 1,E
        case 0x8c: c->H&=~0x2;return ticks; //RES 1,H
        case 0x8d: c->L&=~0x2;return ticks; //RES 1,L
        case 0x8e: { a=c->HL;;_T();uint8_t v;_RD(a,v);_WR(a,v&~0x2); }return ticks; //RES 1,(c->HL)
        case 0x8f: c->A&=~0x2;return ticks; //RES 1,A
        case 0x90: c->B&=~0x4;return ticks; //RES 2,B
        case 0x91: c->C&=~0x4;return ticks; //RES 2,C
        case 0x92: c->D&=~0x4;return ticks; //RES 2,D
        case 0x93: c->E&=~0x4;return ticks; //RES 2,E
        case 0x94: c->H&=~0x4;return ticks; //RES 2,H
        case 0x95: c->L&=~0x4;return ticks; //RES 2,L
        case 0x96: { a=c->HL;;_T();uint8_t v;_RD(a,v);_WR(a,v&~0x4); }return ticks; //RES 2,(c->HL)
        case 0x97: c->A&=~0x4;return ticks; //RES 2,A
        case 0x98: c->B&=~0x8;return ticks; //RES 3,B
        case 0x99: c->C&=~0x8;return ticks; //RES 3,C
        case 0x9a: c->D&=~0x8;return ticks; //RES 3,D
        case 0x9b: c->E&=~0x8;return ticks; //RES 3,E
        case 0x9c: c->H&=~0x8;return ticks; //RES 3,H
        case 0x9d: c->L&=~0x8;return ticks; //RES 3,L
        case 0x9e: { a=c->HL;;_T();uint8_t v;_RD(a,v);_WR(a,v&~0x8); }return ticks; //RES 3,(c->HL)
        case 0x9f: c->A&=~0x8;return ticks; //RES 3,A
        case 0xa0: c->B&=~0x10;return ticks; //RES 4,B
        case 0xa1: c->C&=~0x10;return ticks; //RES 4,C
        case 0xa2: c->D&=~0x10;return ticks; //RES 4,D
        case 0xa3: c->E&=~0x10;return ticks; //RES 4,E
        case 0xa4: c->H&=~0x10;return ticks; //RES 4,H
        case 0xa5: c->L&=~0x10;return ticks; //RES 4,L
        case 0xa6: { a=c->HL;;_T();uint8_t v;_RD(a,v);_WR(a,v&~0x10); }return ticks; //RES 4,(c->HL)
        case 0xa7: c->A&=~0x10;return ticks; //RES 4,A
        case 0xa8: c->B&=~0x20;return ticks; //RES 5,B
        case 0xa9: c->C&=~0x20;return ticks; //RES 5,C
        case 0xaa: c->D&=~0x20;return ticks; //RES 5,D
        case 0xab: c->E&=~0x20;return ticks; //RES 5,E
        case 0xac: c->H&=~0x20;return ticks; //RES 5,H
        case 0xad: c->L&=~0x20;return ticks; //RES 5,L
        case 0xae: { a=c->HL;;_T();uint8_t v;_RD(a,v);_WR(a,v&~0x20); }return ticks; //RES 5,(c->HL)
        case 0xaf: c->A&=~0x20;return ticks; //RES 5,A
        case 0xb0: c->B&=~0x40;return ticks; //RES 6,B
        case 0xb1: c->C&=~0x40;return ticks; //RES 6,C
        case 0xb2: c->D&=~0x40;return ticks; //RES 6,D
        case 0xb3: c->E&=~0x40;return ticks; //RES 6,E
        case 0xb4: c->H&=~0x40;return ticks; //RES 6,H
        case 0xb5: c->L&=~0x40;return ticks; //RES 6,L
        case 0xb6: { a=c->HL;;_T();uint8_t v;_RD(a,v);_WR(a,v&~0x40); }return ticks; //RES 6,(c->HL)
        case 0xb7: c->A&=~0x40;return ticks; //RES 6,A
        case 0xb8: c->B&=~0x80;return ticks; //RES 7,B
        case 0xb9: c->C&=~0x80;return ticks; //RES 7,C
        case 0xba: c->D&=~0x80;return ticks; //RES 7,D
        case 0xbb: c->E&=~0x80;return ticks; //RES 7,E
        case 0xbc: c->H&=~0x80;return ticks; //RES 7,H
        case 0xbd: c->L&=~0x80;return ticks; //RES 7,L
        case 0xbe: { a=c->HL;;_T();uint8_t v;_RD(a,v);_WR(a,v&~0x80); }return ticks; //RES 7,(c->HL)
        case 0xbf: c->A&=~0x80;return ticks; //RES 7,A
        case 0xc0: c->B|=0x1;return ticks; //SET 0,B
        case 0xc1: c->C|=0x1;return ticks; //SET 0,C
        case 0xc2: c->D|=0x1;return ticks; //SET 0,D
        case 0xc3: c->E|=0x1;return ticks; //SET 0,E
        case 0xc4: c->H|=0x1;return ticks; //SET 0,H
        case 0xc5: c->L|=0x1;return ticks; //SET 0,L
        case 0xc6: { a=c->HL;;_T();uint8_t v;_RD(a,v);_WR(a,v|0x1);}return ticks; //SET 0,(c->HL)
        case 0xc7: c->A|=0x1;return ticks; //SET 0,A
        case 0xc8: c->B|=0x2;return ticks; //SET 1,B
        case 0xc9: c->C|=0x2;return ticks; //SET 1,C
        case 0xca: c->D|=0x2;return ticks; //SET 1,D
        case 0xcb: c->E|=0x2;return ticks; //SET 1,E
        case 0xcc: c->H|=0x2;return ticks; //SET 1,H
        case 0xcd: c->L|=0x2;return ticks; //SET 1,L
        case 0xce: { a=c->HL;;_T();uint8_t v;_RD(a,v);_WR(a,v|0x2);}return ticks; //SET 1,(c->HL)
        case 0xcf: c->A|=0x2;return ticks; //SET 1,A
        case 0xd0: c->B|=0x4;return ticks; //SET 2,B
        case 0xd1: c->C|=0x4;return ticks; //SET 2,C
        case 0xd2: c->D|=0x4;return ticks; //SET 2,D
        case 0xd3: c->E|=0x4;return ticks; //SET 2,E
        case 0xd4: c->H|=0x4;return ticks; //SET 2,H
        case 0xd5: c->L|=0x4;return ticks; //SET 2,L
        case 0xd6: { a=c->HL;;_T();uint8_t v;_RD(a,v);_WR(a,v|0x4);}return ticks; //SET 2,(c->HL)
        case 0xd7: c->A|=0x4;return ticks; //SET 2,A
        case 0xd8: c->B|=0x8;return ticks; //SET 3,B
        case 0xd9: c->C|=0x8;return ticks; //SET 3,C
        case 0xda: c->D|=0x8;return ticks; //SET 3,D
        case 0xdb: c->E|=0x8;return ticks; //SET 3,E
        case 0xdc: c->H|=0x8;return ticks; //SET 3,H
        case 0xdd: c->L|=0x8;return ticks; //SET 3,L
        case 0xde: { a=c->HL;;_T();uint8_t v;_RD(a,v);_WR(a,v|0x8);}return ticks; //SET 3,(c->HL)
        case 0xdf: c->A|=0x8;return ticks; //SET 3,A
        case 0xe0: c->B|=0x10;return ticks; //SET 4,B
        case 0xe1: c->C|=0x10;return ticks; //SET 4,C
        case 0xe2: c->D|=0x10;return ticks; //SET 4,D
        case 0xe3: c->E|=0x10;return ticks; //SET 4,E
        case 0xe4: c->H|=0x10;return ticks; //SET 4,H
        case 0xe5: c->L|=0x10;return ticks; //SET 4,L
        case 0xe6: { a=c->HL;;_T();uint8_t v;_RD(a,v);_WR(a,v|0x10);}return ticks; //SET 4,(c->HL)
        case 0xe7: c->A|=0x10;return ticks; //SET 4,A
        case 0xe8: c->B|=0x20;return ticks; //SET 5,B
        case 0xe9: c->C|=0x20;return ticks; //SET 5,C
        case 0xea: c->D|=0x20;return ticks; //SET 5,D
        case 0xeb: c->E|=0x20;return ticks; //SET 5,E
        case 0xec: c->H|=0x20;return ticks; //SET 5,H
        case 0xed: c->L|=0x20;return ticks; //SET 5,L
        case 0xee: { a=c->HL;;_T();uint8_t v;_RD(a,v);_WR(a,v|0x20);}return ticks; //SET 5,(c->HL)
        case 0xef: c->A|=0x20;return ticks; //SET 5,A
        case 0xf0: c->B|=0x40;return ticks; //SET 6,B
        case 0xf1: c->C|=0x40;return ticks; //SET 6,C
        case 0xf2: c->D|=0x40;return ticks; //SET 6,D
        case 0xf3: c->E|=0x40;return ticks; //SET 6,E
        case 0xf4: c->H|=0x40;return ticks; //SET 6,H
        case 0xf5: c->L|=0x40;return ticks; //SET 6,L
        case 0xf6: { a=c->HL;;_T();uint8_t v;_RD(a,v);_WR(a,v|0x40);}return ticks; //SET 6,(c->HL)
        case 0xf7: c->A|=0x40;return ticks; //SET 6,A
        case 0xf8: c->B|=0x80;return ticks; //SET 7,B
        case 0xf9: c->C|=0x80;return ticks; //SET 7,C
        case 0xfa: c->D|=0x80;return ticks; //SET 7,D
        case 0xfb: c->E|=0x80;return ticks; //SET 7,E
        case 0xfc: c->H|=0x80;return ticks; //SET 7,H
        case 0xfd: c->L|=0x80;return ticks; //SET 7,L
        case 0xfe: { a=c->HL;;_T();uint8_t v;_RD(a,v);_WR(a,v|0x80);}return ticks; //SET 7,(c->HL)
        case 0xff: c->A|=0x80;return ticks; //SET 7,A
        default: return ticks;
      }
      break;
    case 0xcc: ticks=_z80_callcc(c,(c->F&Z80_ZF),tick,ticks);return ticks; //CALL Z,nn
    case 0xcd: ticks=_z80_call(c,tick,ticks);return ticks; //CALL nn
    case 0xce: {uint8_t v;_RD(c->PC++,v);_z80_adc(c,v);}return ticks; //ADC n
    case 0xcf: ticks=_z80_rst(c,0x8,tick,ticks);return ticks; //RST 0x8
    case 0xd0: ticks=_z80_retcc(c,!(c->F&Z80_CF),tick,ticks);return ticks; //RET NC
    case 0xd1: /*>rd*/c->ADDR=c->SP++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;l=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*//*>rd*/c->ADDR=c->SP++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;h=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;c->DE=(h<<8)|l;return ticks; //POP DE
    case 0xd2: _IMM16(); if (!(c->F&Z80_CF)) { c->PC=c->WZ; }return ticks; //JP NC,nn
    case 0xd3: {uint8_t v;_RD(c->PC++,v);c->WZ=((c->A<<8)|v);_OUT(c->WZ,c->A);c->Z++;}return ticks; //OUT (n),A
    case 0xd4: ticks=_z80_callcc(c,!(c->F&Z80_CF),tick,ticks);return ticks; //CALL NC,nn
    case 0xd5: _T();_WR(--c->SP,(uint8_t)(c->DE>>8)); _WR(--c->SP,(uint8_t)c->DE);return ticks; //PUSH DE
    case 0xd6: {uint8_t v;_RD(c->PC++,v);_z80_sub(c,v);}return ticks; //SUB n
    case 0xd7: ticks=_z80_rst(c,0x10,tick,ticks);return ticks; //RST 0x10
    case 0xd8: ticks=_z80_retcc(c,(c->F&Z80_CF),tick,ticks);return ticks; //RET C
    case 0xd9: _SWP16(c->BC,c->BC_);_SWP16(c->DE,c->DE_);_SWP16(c->HL,c->HL_);return ticks; //EXX
    case 0xda: _IMM16(); if ((c->F&Z80_CF)) { c->PC=c->WZ; }return ticks; //JP C,nn
    case 0xdb: {uint8_t v;_RD(c->PC++,v);c->WZ=((c->A<<8)|v);_IN(c->WZ++,c->A);}return ticks; //IN A,(n)
    case 0xdc: ticks=_z80_callcc(c,(c->F&Z80_CF),tick,ticks);return ticks; //CALL C,nn
    case 0xdd:
      /*>fetch*/{c->CTRL|=Z80_M1;c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;opcode=c->DATA;c->CTRL&=~(Z80_M1|Z80_MREQ|Z80_RD);c->R=(c->R&0x80)|((c->R+1)&0x7F);c->CTRL|=Z80_RFSH;tick(c);ticks++;c->CTRL|=Z80_MREQ;c->ADDR=c->IR;tick(c);ticks++;c->CTRL&=~(Z80_RFSH|Z80_MREQ);}/*<fetch*/
      switch (opcode) {
        case 0x0:  return ticks; //NOP
        case 0x1: _IMM16(); c->BC=c->WZ;return ticks; //LD BC,nn
        case 0x2: c->WZ=c->BC;/*>wr*/c->ADDR=c->WZ++;tick(c);ticks++;c->CTRL|=(Z80_MREQ|Z80_WR);c->DATA=c->A;tick(c);ticks++;c->CTRL&=~(Z80_MREQ|Z80_WR);tick(c);ticks++;/*wr<*/;c->W=c->A;return ticks; //LD (BC),A
        case 0x3: tick(c);ticks++;tick(c);ticks++;c->BC++;return ticks; //INC BC
        case 0x4: c->B=_z80_inc(c,c->B);return ticks; //INC B
        case 0x5: c->B=_z80_dec(c,c->B);return ticks; //DEC B
        case 0x6: /*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;c->B=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/return ticks; //LD B,n
        case 0x7: _z80_rlca(c);return ticks; //RLCA
        case 0x8: _SWP16(c->AF,c->AF_);return ticks; //EX AF,AF'
        case 0x9: c->IX=_z80_add16(c,c->IX,c->BC);tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;return ticks; //ADD IX,BC
        case 0xa: c->WZ=c->BC;/*>rd*/c->ADDR=c->WZ++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;c->A=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;return ticks; //LD A,(BC)
        case 0xb: tick(c);ticks++;tick(c);ticks++;c->BC--;return ticks; //DEC BC
        case 0xc: c->C=_z80_inc(c,c->C);return ticks; //INC C
        case 0xd: c->C=_z80_dec(c,c->C);return ticks; //DEC C
        case 0xe: /*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;c->C=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/return ticks; //LD C,n
        case 0xf: _z80_rrca(c);return ticks; //RRCA
        case 0x10: ticks=_z80_djnz(c,tick,ticks);return ticks; //DJNZ
        case 0x11: _IMM16(); c->DE=c->WZ;return ticks; //LD DE,nn
        case 0x12: c->WZ=c->DE;/*>wr*/c->ADDR=c->WZ++;tick(c);ticks++;c->CTRL|=(Z80_MREQ|Z80_WR);c->DATA=c->A;tick(c);ticks++;c->CTRL&=~(Z80_MREQ|Z80_WR);tick(c);ticks++;/*wr<*/;c->W=c->A;return ticks; //LD (DE),A
        case 0x13: tick(c);ticks++;tick(c);ticks++;c->DE++;return ticks; //INC DE
        case 0x14: c->D=_z80_inc(c,c->D);return ticks; //INC D
        case 0x15: c->D=_z80_dec(c,c->D);return ticks; //DEC D
        case 0x16: /*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;c->D=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/return ticks; //LD D,n
        case 0x17: _z80_rla(c);return ticks; //RLA
        case 0x18: ticks=_z80_jr(c,tick,ticks);return ticks; //JR d
        case 0x19: c->IX=_z80_add16(c,c->IX,c->DE);tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;return ticks; //ADD IX,DE
        case 0x1a: c->WZ=c->DE;/*>rd*/c->ADDR=c->WZ++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;c->A=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;return ticks; //LD A,(DE)
        case 0x1b: tick(c);ticks++;tick(c);ticks++;c->DE--;return ticks; //DEC DE
        case 0x1c: c->E=_z80_inc(c,c->E);return ticks; //INC E
        case 0x1d: c->E=_z80_dec(c,c->E);return ticks; //DEC E
        case 0x1e: /*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;c->E=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/return ticks; //LD E,n
        case 0x1f: _z80_rra(c);return ticks; //RRA
        case 0x20: ticks=_z80_jr_cc(c,!(c->F&Z80_ZF),tick,ticks);return ticks; //JR NZ,d
        case 0x21: _IMM16(); c->IX=c->WZ;return ticks; //LD IX,nn
        case 0x22: _IMM16();/*>wr*/c->ADDR=c->WZ++;tick(c);ticks++;c->CTRL|=(Z80_MREQ|Z80_WR);c->DATA=(uint8_t)c->IX;tick(c);ticks++;c->CTRL&=~(Z80_MREQ|Z80_WR);tick(c);ticks++;/*wr<*//*>wr*/c->ADDR=c->WZ;tick(c);ticks++;c->CTRL|=(Z80_MREQ|Z80_WR);c->DATA=(uint8_t)(c->IX>>8);tick(c);ticks++;c->CTRL&=~(Z80_MREQ|Z80_WR);tick(c);ticks++;/*wr<*/return ticks; //LD (nn),IX
        case 0x23: tick(c);ticks++;tick(c);ticks++;c->IX++;return ticks; //INC IX
        case 0x24: c->IXH=_z80_inc(c,c->IXH);return ticks; //INC IXH
        case 0x25: c->IXH=_z80_dec(c,c->IXH);return ticks; //DEC IXH
        case 0x26: /*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;c->IXH=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/return ticks; //LD IXH,n
        case 0x27: _z80_daa(c);return ticks; //DAA
        case 0x28: ticks=_z80_jr_cc(c,(c->F&Z80_ZF),tick,ticks);return ticks; //JR Z,d
        case 0x29: c->IX=_z80_add16(c,c->IX,c->IX);tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;return ticks; //ADD IX,IX
        case 0x2a: _IMM16();/*>rd*/c->ADDR=c->WZ++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;l=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*//*>rd*/c->ADDR=c->WZ;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;h=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/c->IX=(h<<8)|l;return ticks; //LD IX,(nn)
        case 0x2b: tick(c);ticks++;tick(c);ticks++;c->IX--;return ticks; //DEC IX
        case 0x2c: c->IXL=_z80_inc(c,c->IXL);return ticks; //INC IXL
        case 0x2d: c->IXL=_z80_dec(c,c->IXL);return ticks; //DEC IXL
        case 0x2e: /*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;c->IXL=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/return ticks; //LD IXL,n
        case 0x2f: _z80_cpl(c);return ticks; //CPL
        case 0x30: ticks=_z80_jr_cc(c,!(c->F&Z80_CF),tick,ticks);return ticks; //JR NC,d
        case 0x31: _IMM16(); c->SP=c->WZ;return ticks; //LD SP,nn
        case 0x32: _IMM16();/*>wr*/c->ADDR=c->WZ++;tick(c);ticks++;c->CTRL|=(Z80_MREQ|Z80_WR);c->DATA=c->A;tick(c);ticks++;c->CTRL&=~(Z80_MREQ|Z80_WR);tick(c);ticks++;/*wr<*/;c->W=c->A;return ticks; //LD (nn),A
        case 0x33: tick(c);ticks++;tick(c);ticks++;c->SP++;return ticks; //INC SP
        case 0x34: /*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;d=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;a=c->WZ=c->IX+d;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;/*>rd*/c->ADDR=a;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;v=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*//*>wr*/c->ADDR=a;tick(c);ticks++;c->CTRL|=(Z80_MREQ|Z80_WR);c->DATA=_z80_inc(c,v);tick(c);ticks++;c->CTRL&=~(Z80_MREQ|Z80_WR);tick(c);ticks++;/*wr<*/return ticks; //INC (c->IX+d)
        case 0x35: /*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;d=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;a=c->WZ=c->IX+d;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;/*>rd*/c->ADDR=a;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;v=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*//*>wr*/c->ADDR=a;tick(c);ticks++;c->CTRL|=(Z80_MREQ|Z80_WR);c->DATA=_z80_dec(c,v);tick(c);ticks++;c->CTRL&=~(Z80_MREQ|Z80_WR);tick(c);ticks++;/*wr<*/return ticks; //DEC (c->IX+d)
        case 0x36: /*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;d=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;a=c->WZ=c->IX+d;tick(c);ticks++;tick(c);ticks++;/*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;v=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*//*>wr*/c->ADDR=a;tick(c);ticks++;c->CTRL|=(Z80_MREQ|Z80_WR);c->DATA=v;tick(c);ticks++;c->CTRL&=~(Z80_MREQ|Z80_WR);tick(c);ticks++;/*wr<*/return ticks; //LD (c->IX+d),n
        case 0x37: _z80_scf(c);return ticks; //SCF
        case 0x38: ticks=_z80_jr_cc(c,(c->F&Z80_CF),tick,ticks);return ticks; //JR C,d
        case 0x39: c->IX=_z80_add16(c,c->IX,c->SP);tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;return ticks; //ADD IX,SP
        case 0x3a: _IMM16();/*>rd*/c->ADDR=c->WZ++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;c->A=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;return ticks; //LD A,(nn)
        case 0x3b: tick(c);ticks++;tick(c);ticks++;c->SP--;return ticks; //DEC SP
        case 0x3c: c->A=_z80_inc(c,c->A);return ticks; //INC A
        case 0x3d: c->A=_z80_dec(c,c->A);return ticks; //DEC A
        case 0x3e: /*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;c->A=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/return ticks; //LD A,n
        case 0x3f: _z80_ccf(c);return ticks; //CCF
        case 0x40: c->B=c->B;return ticks; //LD B,B
        case 0x41: c->B=c->C;return ticks; //LD B,C
        case 0x42: c->B=c->D;return ticks; //LD B,D
        case 0x43: c->B=c->E;return ticks; //LD B,E
        case 0x44: c->B=c->IXH;return ticks; //LD B,IXH
        case 0x45: c->B=c->IXL;return ticks; //LD B,IXL
        case 0x46: {/*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;d=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;a=c->WZ=c->IX+d;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;/*>rd*/c->ADDR=a;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;c->B=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/}return ticks; //LD B,(c->IX+d)
        case 0x47: c->B=c->A;return ticks; //LD B,A
        case 0x48: c->C=c->B;return ticks; //LD C,B
        case 0x49: c->C=c->C;return ticks; //LD C,C
        case 0x4a: c->C=c->D;return ticks; //LD C,D
        case 0x4b: c->C=c->E;return ticks; //LD C,E
        case 0x4c: c->C=c->IXH;return ticks; //LD C,IXH
        case 0x4d: c->C=c->IXL;return ticks; //LD C,IXL
        case 0x4e: {/*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;d=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;a=c->WZ=c->IX+d;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;/*>rd*/c->ADDR=a;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;c->C=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/}return ticks; //LD C,(c->IX+d)
        case 0x4f: c->C=c->A;return ticks; //LD C,A
        case 0x50: c->D=c->B;return ticks; //LD D,B
        case 0x51: c->D=c->C;return ticks; //LD D,C
        case 0x52: c->D=c->D;return ticks; //LD D,D
        case 0x53: c->D=c->E;return ticks; //LD D,E
        case 0x54: c->D=c->IXH;return ticks; //LD D,IXH
        case 0x55: c->D=c->IXL;return ticks; //LD D,IXL
        case 0x56: {/*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;d=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;a=c->WZ=c->IX+d;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;/*>rd*/c->ADDR=a;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;c->D=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/}return ticks; //LD D,(c->IX+d)
        case 0x57: c->D=c->A;return ticks; //LD D,A
        case 0x58: c->E=c->B;return ticks; //LD E,B
        case 0x59: c->E=c->C;return ticks; //LD E,C
        case 0x5a: c->E=c->D;return ticks; //LD E,D
        case 0x5b: c->E=c->E;return ticks; //LD E,E
        case 0x5c: c->E=c->IXH;return ticks; //LD E,IXH
        case 0x5d: c->E=c->IXL;return ticks; //LD E,IXL
        case 0x5e: {/*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;d=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;a=c->WZ=c->IX+d;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;/*>rd*/c->ADDR=a;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;c->E=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/}return ticks; //LD E,(c->IX+d)
        case 0x5f: c->E=c->A;return ticks; //LD E,A
        case 0x60: c->IXH=c->B;return ticks; //LD IXH,B
        case 0x61: c->IXH=c->C;return ticks; //LD IXH,C
        case 0x62: c->IXH=c->D;return ticks; //LD IXH,D
        case 0x63: c->IXH=c->E;return ticks; //LD IXH,E
        case 0x64: c->IXH=c->IXH;return ticks; //LD IXH,IXH
        case 0x65: c->IXH=c->IXL;return ticks; //LD IXH,IXL
        case 0x66: {/*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;d=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;a=c->WZ=c->IX+d;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;/*>rd*/c->ADDR=a;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;c->H=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/}return ticks; //LD H,(c->IX+d)
        case 0x67: c->IXH=c->A;return ticks; //LD IXH,A
        case 0x68: c->IXL=c->B;return ticks; //LD IXL,B
        case 0x69: c->IXL=c->C;return ticks; //LD IXL,C
        case 0x6a: c->IXL=c->D;return ticks; //LD IXL,D
        case 0x6b: c->IXL=c->E;return ticks; //LD IXL,E
        case 0x6c: c->IXL=c->IXH;return ticks; //LD IXL,IXH
        case 0x6d: c->IXL=c->IXL;return ticks; //LD IXL,IXL
        case 0x6e: {/*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;d=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;a=c->WZ=c->IX+d;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;/*>rd*/c->ADDR=a;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;c->L=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/}return ticks; //LD L,(c->IX+d)
        case 0x6f: c->IXL=c->A;return ticks; //LD IXL,A
        case 0x70: {/*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;d=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;a=c->WZ=c->IX+d;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;/*>wr*/c->ADDR=a;tick(c);ticks++;c->CTRL|=(Z80_MREQ|Z80_WR);c->DATA=c->B;tick(c);ticks++;c->CTRL&=~(Z80_MREQ|Z80_WR);tick(c);ticks++;/*wr<*/;}return ticks; //LD (c->IX+d),B
        case 0x71: {/*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;d=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;a=c->WZ=c->IX+d;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;/*>wr*/c->ADDR=a;tick(c);ticks++;c->CTRL|=(Z80_MREQ|Z80_WR);c->DATA=c->C;tick(c);ticks++;c->CTRL&=~(Z80_MREQ|Z80_WR);tick(c);ticks++;/*wr<*/;}return ticks; //LD (c->IX+d),C
        case 0x72: {/*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;d=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;a=c->WZ=c->IX+d;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;/*>wr*/c->ADDR=a;tick(c);ticks++;c->CTRL|=(Z80_MREQ|Z80_WR);c->DATA=c->D;tick(c);ticks++;c->CTRL&=~(Z80_MREQ|Z80_WR);tick(c);ticks++;/*wr<*/;}return ticks; //LD (c->IX+d),D
        case 0x73: {/*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;d=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;a=c->WZ=c->IX+d;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;/*>wr*/c->ADDR=a;tick(c);ticks++;c->CTRL|=(Z80_MREQ|Z80_WR);c->DATA=c->E;tick(c);ticks++;c->CTRL&=~(Z80_MREQ|Z80_WR);tick(c);ticks++;/*wr<*/;}return ticks; //LD (c->IX+d),E
        case 0x74: {/*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;d=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;a=c->WZ=c->IX+d;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;/*>wr*/c->ADDR=a;tick(c);ticks++;c->CTRL|=(Z80_MREQ|Z80_WR);c->DATA=c->H;tick(c);ticks++;c->CTRL&=~(Z80_MREQ|Z80_WR);tick(c);ticks++;/*wr<*/;}return ticks; //LD (c->IX+d),H
        case 0x75: {/*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;d=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;a=c->WZ=c->IX+d;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;/*>wr*/c->ADDR=a;tick(c);ticks++;c->CTRL|=(Z80_MREQ|Z80_WR);c->DATA=c->L;tick(c);ticks++;c->CTRL&=~(Z80_MREQ|Z80_WR);tick(c);ticks++;/*wr<*/;}return ticks; //LD (c->IX+d),L
        case 0x76: _z80_halt(c);return ticks; //HALT
        case 0x77: {/*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;d=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;a=c->WZ=c->IX+d;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;/*>wr*/c->ADDR=a;tick(c);ticks++;c->CTRL|=(Z80_MREQ|Z80_WR);c->DATA=c->A;tick(c);ticks++;c->CTRL&=~(Z80_MREQ|Z80_WR);tick(c);ticks++;/*wr<*/;}return ticks; //LD (c->IX+d),A
        case 0x78: c->A=c->B;return ticks; //LD A,B
        case 0x79: c->A=c->C;return ticks; //LD A,C
        case 0x7a: c->A=c->D;return ticks; //LD A,D
        case 0x7b: c->A=c->E;return ticks; //LD A,E
        case 0x7c: c->A=c->IXH;return ticks; //LD A,IXH
        case 0x7d: c->A=c->IXL;return ticks; //LD A,IXL
        case 0x7e: {/*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;d=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;a=c->WZ=c->IX+d;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;/*>rd*/c->ADDR=a;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;c->A=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/}return ticks; //LD A,(c->IX+d)
        case 0x7f: c->A=c->A;return ticks; //LD A,A
        case 0x80: _z80_add(c,c->B);return ticks; //ADD B
        case 0x81: _z80_add(c,c->C);return ticks; //ADD C
        case 0x82: _z80_add(c,c->D);return ticks; //ADD D
        case 0x83: _z80_add(c,c->E);return ticks; //ADD E
        case 0x84: _z80_add(c,c->IXH);return ticks; //ADD IXH
        case 0x85: _z80_add(c,c->IXL);return ticks; //ADD IXL
        case 0x86: /*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;d=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;a=c->WZ=c->IX+d;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;/*>rd*/c->ADDR=a;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;v=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/_z80_add(c,v);return ticks; //ADD (c->IX+d)
        case 0x87: _z80_add(c,c->A);return ticks; //ADD A
        case 0x88: _z80_adc(c,c->B);return ticks; //ADC B
        case 0x89: _z80_adc(c,c->C);return ticks; //ADC C
        case 0x8a: _z80_adc(c,c->D);return ticks; //ADC D
        case 0x8b: _z80_adc(c,c->E);return ticks; //ADC E
        case 0x8c: _z80_adc(c,c->IXH);return ticks; //ADC IXH
        case 0x8d: _z80_adc(c,c->IXL);return ticks; //ADC IXL
        case 0x8e: /*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;d=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;a=c->WZ=c->IX+d;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;/*>rd*/c->ADDR=a;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;v=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/_z80_adc(c,v);return ticks; //ADC (c->IX+d)
        case 0x8f: _z80_adc(c,c->A);return ticks; //ADC A
        case 0x90: _z80_sub(c,c->B);return ticks; //SUB B
        case 0x91: _z80_sub(c,c->C);return ticks; //SUB C
        case 0x92: _z80_sub(c,c->D);return ticks; //SUB D
        case 0x93: _z80_sub(c,c->E);return ticks; //SUB E
        case 0x94: _z80_sub(c,c->IXH);return ticks; //SUB IXH
        case 0x95: _z80_sub(c,c->IXL);return ticks; //SUB IXL
        case 0x96: /*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;d=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;a=c->WZ=c->IX+d;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;/*>rd*/c->ADDR=a;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;v=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/_z80_sub(c,v);return ticks; //SUB (c->IX+d)
        case 0x97: _z80_sub(c,c->A);return ticks; //SUB A
        case 0x98: _z80_sbc(c,c->B);return ticks; //SBC B
        case 0x99: _z80_sbc(c,c->C);return ticks; //SBC C
        case 0x9a: _z80_sbc(c,c->D);return ticks; //SBC D
        case 0x9b: _z80_sbc(c,c->E);return ticks; //SBC E
        case 0x9c: _z80_sbc(c,c->IXH);return ticks; //SBC IXH
        case 0x9d: _z80_sbc(c,c->IXL);return ticks; //SBC IXL
        case 0x9e: /*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;d=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;a=c->WZ=c->IX+d;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;/*>rd*/c->ADDR=a;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;v=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/_z80_sbc(c,v);return ticks; //SBC (c->IX+d)
        case 0x9f: _z80_sbc(c,c->A);return ticks; //SBC A
        case 0xa0: _z80_and(c,c->B);return ticks; //AND B
        case 0xa1: _z80_and(c,c->C);return ticks; //AND C
        case 0xa2: _z80_and(c,c->D);return ticks; //AND D
        case 0xa3: _z80_and(c,c->E);return ticks; //AND E
        case 0xa4: _z80_and(c,c->IXH);return ticks; //AND IXH
        case 0xa5: _z80_and(c,c->IXL);return ticks; //AND IXL
        case 0xa6: /*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;d=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;a=c->WZ=c->IX+d;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;/*>rd*/c->ADDR=a;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;v=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/_z80_and(c,v);return ticks; //AND (c->IX+d)
        case 0xa7: _z80_and(c,c->A);return ticks; //AND A
        case 0xa8: _z80_xor(c,c->B);return ticks; //XOR B
        case 0xa9: _z80_xor(c,c->C);return ticks; //XOR C
        case 0xaa: _z80_xor(c,c->D);return ticks; //XOR D
        case 0xab: _z80_xor(c,c->E);return ticks; //XOR E
        case 0xac: _z80_xor(c,c->IXH);return ticks; //XOR IXH
        case 0xad: _z80_xor(c,c->IXL);return ticks; //XOR IXL
        case 0xae: /*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;d=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;a=c->WZ=c->IX+d;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;/*>rd*/c->ADDR=a;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;v=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/_z80_xor(c,v);return ticks; //XOR (c->IX+d)
        case 0xaf: _z80_xor(c,c->A);return ticks; //XOR A
        case 0xb0: _z80_or(c,c->B);return ticks; //OR B
        case 0xb1: _z80_or(c,c->C);return ticks; //OR C
        case 0xb2: _z80_or(c,c->D);return ticks; //OR D
        case 0xb3: _z80_or(c,c->E);return ticks; //OR E
        case 0xb4: _z80_or(c,c->IXH);return ticks; //OR IXH
        case 0xb5: _z80_or(c,c->IXL);return ticks; //OR IXL
        case 0xb6: /*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;d=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;a=c->WZ=c->IX+d;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;/*>rd*/c->ADDR=a;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;v=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/_z80_or(c,v);return ticks; //OR (c->IX+d)
        case 0xb7: _z80_or(c,c->A);return ticks; //OR A
        case 0xb8: _z80_cp(c,c->B);return ticks; //CP B
        case 0xb9: _z80_cp(c,c->C);return ticks; //CP C
        case 0xba: _z80_cp(c,c->D);return ticks; //CP D
        case 0xbb: _z80_cp(c,c->E);return ticks; //CP E
        case 0xbc: _z80_cp(c,c->IXH);return ticks; //CP IXH
        case 0xbd: _z80_cp(c,c->IXL);return ticks; //CP IXL
        case 0xbe: /*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;d=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;a=c->WZ=c->IX+d;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;/*>rd*/c->ADDR=a;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;v=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/_z80_cp(c,v);return ticks; //CP (c->IX+d)
        case 0xbf: _z80_cp(c,c->A);return ticks; //CP A
        case 0xc0: ticks=_z80_retcc(c,!(c->F&Z80_ZF),tick,ticks);return ticks; //RET NZ
        case 0xc1: /*>rd*/c->ADDR=c->SP++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;l=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*//*>rd*/c->ADDR=c->SP++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;h=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;c->BC=(h<<8)|l;return ticks; //POP BC
        case 0xc2: _IMM16(); if (!(c->F&Z80_ZF)) { c->PC=c->WZ; }return ticks; //JP NZ,nn
        case 0xc3: _IMM16(); c->PC=c->WZ;return ticks; //JP nn
        case 0xc4: ticks=_z80_callcc(c,!(c->F&Z80_ZF),tick,ticks);return ticks; //CALL NZ,nn
        case 0xc5: _T();_WR(--c->SP,(uint8_t)(c->BC>>8)); _WR(--c->SP,(uint8_t)c->BC);return ticks; //PUSH BC
        case 0xc6: {uint8_t v;_RD(c->PC++,v);_z80_add(c,v);}return ticks; //ADD n
        case 0xc7: ticks=_z80_rst(c,0x0,tick,ticks);return ticks; //RST 0x0
        case 0xc8: ticks=_z80_retcc(c,(c->F&Z80_ZF),tick,ticks);return ticks; //RET Z
        case 0xc9: ticks=_z80_ret(c,tick,ticks);return ticks; //RET
        case 0xca: _IMM16(); if ((c->F&Z80_ZF)) { c->PC=c->WZ; }return ticks; //JP Z,nn
        case 0xcb:
          _RD(c->PC++, d);
          /*>fetch*/{c->CTRL|=Z80_M1;c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;opcode=c->DATA;c->CTRL&=~(Z80_M1|Z80_MREQ|Z80_RD);c->CTRL|=Z80_RFSH;tick(c);ticks++;c->CTRL|=Z80_MREQ;c->ADDR=c->IR;tick(c);ticks++;c->CTRL&=~(Z80_RFSH|Z80_MREQ);}/*<fetch*/
          switch (opcode) {
            case 0x0: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->B=_z80_rlc(c,v);_WR(a,c->B); }return ticks; //RLC (c->IX+d),B
            case 0x1: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->C=_z80_rlc(c,v);_WR(a,c->C); }return ticks; //RLC (c->IX+d),C
            case 0x2: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->D=_z80_rlc(c,v);_WR(a,c->D); }return ticks; //RLC (c->IX+d),D
            case 0x3: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->E=_z80_rlc(c,v);_WR(a,c->E); }return ticks; //RLC (c->IX+d),E
            case 0x4: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->H=_z80_rlc(c,v);_WR(a,c->H); }return ticks; //RLC (c->IX+d),H
            case 0x5: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->L=_z80_rlc(c,v);_WR(a,c->L); }return ticks; //RLC (c->IX+d),L
            case 0x6: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_WR(a,_z80_rlc(c,v)); }return ticks; //RLC (c->IX+d)
            case 0x7: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->A=_z80_rlc(c,v);_WR(a,c->A); }return ticks; //RLC (c->IX+d),A
            case 0x8: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->B=_z80_rrc(c,v);_WR(a,c->B); }return ticks; //RRC (c->IX+d),B
            case 0x9: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->C=_z80_rrc(c,v);_WR(a,c->C); }return ticks; //RRC (c->IX+d),C
            case 0xa: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->D=_z80_rrc(c,v);_WR(a,c->D); }return ticks; //RRC (c->IX+d),D
            case 0xb: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->E=_z80_rrc(c,v);_WR(a,c->E); }return ticks; //RRC (c->IX+d),E
            case 0xc: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->H=_z80_rrc(c,v);_WR(a,c->H); }return ticks; //RRC (c->IX+d),H
            case 0xd: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->L=_z80_rrc(c,v);_WR(a,c->L); }return ticks; //RRC (c->IX+d),L
            case 0xe: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_WR(a,_z80_rrc(c,v)); }return ticks; //RRC (c->IX+d)
            case 0xf: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->A=_z80_rrc(c,v);_WR(a,c->A); }return ticks; //RRC (c->IX+d),A
            case 0x10: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->B=_z80_rl(c,v);_WR(a,c->B); }return ticks; //RL (c->IX+d),B
            case 0x11: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->C=_z80_rl(c,v);_WR(a,c->C); }return ticks; //RL (c->IX+d),C
            case 0x12: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->D=_z80_rl(c,v);_WR(a,c->D); }return ticks; //RL (c->IX+d),D
            case 0x13: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->E=_z80_rl(c,v);_WR(a,c->E); }return ticks; //RL (c->IX+d),E
            case 0x14: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->H=_z80_rl(c,v);_WR(a,c->H); }return ticks; //RL (c->IX+d),H
            case 0x15: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->L=_z80_rl(c,v);_WR(a,c->L); }return ticks; //RL (c->IX+d),L
            case 0x16: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_WR(a,_z80_rl(c,v)); }return ticks; //RL (c->IX+d)
            case 0x17: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->A=_z80_rl(c,v);_WR(a,c->A); }return ticks; //RL (c->IX+d),A
            case 0x18: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->B=_z80_rr(c,v);_WR(a,c->B); }return ticks; //RR (c->IX+d),B
            case 0x19: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->C=_z80_rr(c,v);_WR(a,c->C); }return ticks; //RR (c->IX+d),C
            case 0x1a: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->D=_z80_rr(c,v);_WR(a,c->D); }return ticks; //RR (c->IX+d),D
            case 0x1b: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->E=_z80_rr(c,v);_WR(a,c->E); }return ticks; //RR (c->IX+d),E
            case 0x1c: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->H=_z80_rr(c,v);_WR(a,c->H); }return ticks; //RR (c->IX+d),H
            case 0x1d: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->L=_z80_rr(c,v);_WR(a,c->L); }return ticks; //RR (c->IX+d),L
            case 0x1e: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_WR(a,_z80_rr(c,v)); }return ticks; //RR (c->IX+d)
            case 0x1f: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->A=_z80_rr(c,v);_WR(a,c->A); }return ticks; //RR (c->IX+d),A
            case 0x20: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->B=_z80_sla(c,v);_WR(a,c->B); }return ticks; //SLA (c->IX+d),B
            case 0x21: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->C=_z80_sla(c,v);_WR(a,c->C); }return ticks; //SLA (c->IX+d),C
            case 0x22: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->D=_z80_sla(c,v);_WR(a,c->D); }return ticks; //SLA (c->IX+d),D
            case 0x23: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->E=_z80_sla(c,v);_WR(a,c->E); }return ticks; //SLA (c->IX+d),E
            case 0x24: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->H=_z80_sla(c,v);_WR(a,c->H); }return ticks; //SLA (c->IX+d),H
            case 0x25: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->L=_z80_sla(c,v);_WR(a,c->L); }return ticks; //SLA (c->IX+d),L
            case 0x26: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_WR(a,_z80_sla(c,v)); }return ticks; //SLA (c->IX+d)
            case 0x27: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->A=_z80_sla(c,v);_WR(a,c->A); }return ticks; //SLA (c->IX+d),A
            case 0x28: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->B=_z80_sra(c,v);_WR(a,c->B); }return ticks; //SRA (c->IX+d),B
            case 0x29: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->C=_z80_sra(c,v);_WR(a,c->C); }return ticks; //SRA (c->IX+d),C
            case 0x2a: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->D=_z80_sra(c,v);_WR(a,c->D); }return ticks; //SRA (c->IX+d),D
            case 0x2b: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->E=_z80_sra(c,v);_WR(a,c->E); }return ticks; //SRA (c->IX+d),E
            case 0x2c: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->H=_z80_sra(c,v);_WR(a,c->H); }return ticks; //SRA (c->IX+d),H
            case 0x2d: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->L=_z80_sra(c,v);_WR(a,c->L); }return ticks; //SRA (c->IX+d),L
            case 0x2e: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_WR(a,_z80_sra(c,v)); }return ticks; //SRA (c->IX+d)
            case 0x2f: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->A=_z80_sra(c,v);_WR(a,c->A); }return ticks; //SRA (c->IX+d),A
            case 0x30: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->B=_z80_sll(c,v);_WR(a,c->B); }return ticks; //SLL (c->IX+d),B
            case 0x31: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->C=_z80_sll(c,v);_WR(a,c->C); }return ticks; //SLL (c->IX+d),C
            case 0x32: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->D=_z80_sll(c,v);_WR(a,c->D); }return ticks; //SLL (c->IX+d),D
            case 0x33: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->E=_z80_sll(c,v);_WR(a,c->E); }return ticks; //SLL (c->IX+d),E
            case 0x34: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->H=_z80_sll(c,v);_WR(a,c->H); }return ticks; //SLL (c->IX+d),H
            case 0x35: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->L=_z80_sll(c,v);_WR(a,c->L); }return ticks; //SLL (c->IX+d),L
            case 0x36: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_WR(a,_z80_sll(c,v)); }return ticks; //SLL (c->IX+d)
            case 0x37: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->A=_z80_sll(c,v);_WR(a,c->A); }return ticks; //SLL (c->IX+d),A
            case 0x38: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->B=_z80_srl(c,v);_WR(a,c->B); }return ticks; //SRL (c->IX+d),B
            case 0x39: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->C=_z80_srl(c,v);_WR(a,c->C); }return ticks; //SRL (c->IX+d),C
            case 0x3a: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->D=_z80_srl(c,v);_WR(a,c->D); }return ticks; //SRL (c->IX+d),D
            case 0x3b: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->E=_z80_srl(c,v);_WR(a,c->E); }return ticks; //SRL (c->IX+d),E
            case 0x3c: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->H=_z80_srl(c,v);_WR(a,c->H); }return ticks; //SRL (c->IX+d),H
            case 0x3d: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->L=_z80_srl(c,v);_WR(a,c->L); }return ticks; //SRL (c->IX+d),L
            case 0x3e: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_WR(a,_z80_srl(c,v)); }return ticks; //SRL (c->IX+d)
            case 0x3f: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->A=_z80_srl(c,v);_WR(a,c->A); }return ticks; //SRL (c->IX+d),A
            case 0x40: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x1); }return ticks; //BIT 0,(c->IX+d)
            case 0x41: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x1); }return ticks; //BIT 0,(c->IX+d)
            case 0x42: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x1); }return ticks; //BIT 0,(c->IX+d)
            case 0x43: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x1); }return ticks; //BIT 0,(c->IX+d)
            case 0x44: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x1); }return ticks; //BIT 0,(c->IX+d)
            case 0x45: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x1); }return ticks; //BIT 0,(c->IX+d)
            case 0x46: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x1); }return ticks; //BIT 0,(c->IX+d)
            case 0x47: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x1); }return ticks; //BIT 0,(c->IX+d)
            case 0x48: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x2); }return ticks; //BIT 1,(c->IX+d)
            case 0x49: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x2); }return ticks; //BIT 1,(c->IX+d)
            case 0x4a: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x2); }return ticks; //BIT 1,(c->IX+d)
            case 0x4b: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x2); }return ticks; //BIT 1,(c->IX+d)
            case 0x4c: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x2); }return ticks; //BIT 1,(c->IX+d)
            case 0x4d: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x2); }return ticks; //BIT 1,(c->IX+d)
            case 0x4e: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x2); }return ticks; //BIT 1,(c->IX+d)
            case 0x4f: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x2); }return ticks; //BIT 1,(c->IX+d)
            case 0x50: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x4); }return ticks; //BIT 2,(c->IX+d)
            case 0x51: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x4); }return ticks; //BIT 2,(c->IX+d)
            case 0x52: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x4); }return ticks; //BIT 2,(c->IX+d)
            case 0x53: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x4); }return ticks; //BIT 2,(c->IX+d)
            case 0x54: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x4); }return ticks; //BIT 2,(c->IX+d)
            case 0x55: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x4); }return ticks; //BIT 2,(c->IX+d)
            case 0x56: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x4); }return ticks; //BIT 2,(c->IX+d)
            case 0x57: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x4); }return ticks; //BIT 2,(c->IX+d)
            case 0x58: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x8); }return ticks; //BIT 3,(c->IX+d)
            case 0x59: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x8); }return ticks; //BIT 3,(c->IX+d)
            case 0x5a: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x8); }return ticks; //BIT 3,(c->IX+d)
            case 0x5b: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x8); }return ticks; //BIT 3,(c->IX+d)
            case 0x5c: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x8); }return ticks; //BIT 3,(c->IX+d)
            case 0x5d: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x8); }return ticks; //BIT 3,(c->IX+d)
            case 0x5e: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x8); }return ticks; //BIT 3,(c->IX+d)
            case 0x5f: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x8); }return ticks; //BIT 3,(c->IX+d)
            case 0x60: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x10); }return ticks; //BIT 4,(c->IX+d)
            case 0x61: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x10); }return ticks; //BIT 4,(c->IX+d)
            case 0x62: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x10); }return ticks; //BIT 4,(c->IX+d)
            case 0x63: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x10); }return ticks; //BIT 4,(c->IX+d)
            case 0x64: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x10); }return ticks; //BIT 4,(c->IX+d)
            case 0x65: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x10); }return ticks; //BIT 4,(c->IX+d)
            case 0x66: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x10); }return ticks; //BIT 4,(c->IX+d)
            case 0x67: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x10); }return ticks; //BIT 4,(c->IX+d)
            case 0x68: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x20); }return ticks; //BIT 5,(c->IX+d)
            case 0x69: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x20); }return ticks; //BIT 5,(c->IX+d)
            case 0x6a: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x20); }return ticks; //BIT 5,(c->IX+d)
            case 0x6b: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x20); }return ticks; //BIT 5,(c->IX+d)
            case 0x6c: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x20); }return ticks; //BIT 5,(c->IX+d)
            case 0x6d: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x20); }return ticks; //BIT 5,(c->IX+d)
            case 0x6e: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x20); }return ticks; //BIT 5,(c->IX+d)
            case 0x6f: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x20); }return ticks; //BIT 5,(c->IX+d)
            case 0x70: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x40); }return ticks; //BIT 6,(c->IX+d)
            case 0x71: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x40); }return ticks; //BIT 6,(c->IX+d)
            case 0x72: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x40); }return ticks; //BIT 6,(c->IX+d)
            case 0x73: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x40); }return ticks; //BIT 6,(c->IX+d)
            case 0x74: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x40); }return ticks; //BIT 6,(c->IX+d)
            case 0x75: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x40); }return ticks; //BIT 6,(c->IX+d)
            case 0x76: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x40); }return ticks; //BIT 6,(c->IX+d)
            case 0x77: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x40); }return ticks; //BIT 6,(c->IX+d)
            case 0x78: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x80); }return ticks; //BIT 7,(c->IX+d)
            case 0x79: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x80); }return ticks; //BIT 7,(c->IX+d)
            case 0x7a: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x80); }return ticks; //BIT 7,(c->IX+d)
            case 0x7b: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x80); }return ticks; //BIT 7,(c->IX+d)
            case 0x7c: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x80); }return ticks; //BIT 7,(c->IX+d)
            case 0x7d: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x80); }return ticks; //BIT 7,(c->IX+d)
            case 0x7e: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x80); }return ticks; //BIT 7,(c->IX+d)
            case 0x7f: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x80); }return ticks; //BIT 7,(c->IX+d)
            case 0x80: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->B=v&~0x1;_WR(a,c->B); }return ticks; //RES 0,(c->IX+d),B
            case 0x81: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->C=v&~0x1;_WR(a,c->C); }return ticks; //RES 0,(c->IX+d),C
            case 0x82: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->D=v&~0x1;_WR(a,c->D); }return ticks; //RES 0,(c->IX+d),D
            case 0x83: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->E=v&~0x1;_WR(a,c->E); }return ticks; //RES 0,(c->IX+d),E
            case 0x84: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->H=v&~0x1;_WR(a,c->H); }return ticks; //RES 0,(c->IX+d),H
            case 0x85: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->L=v&~0x1;_WR(a,c->L); }return ticks; //RES 0,(c->IX+d),L
            case 0x86: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_WR(a,v&~0x1); }return ticks; //RES 0,(c->IX+d)
            case 0x87: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->A=v&~0x1;_WR(a,c->A); }return ticks; //RES 0,(c->IX+d),A
            case 0x88: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->B=v&~0x2;_WR(a,c->B); }return ticks; //RES 1,(c->IX+d),B
            case 0x89: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->C=v&~0x2;_WR(a,c->C); }return ticks; //RES 1,(c->IX+d),C
            case 0x8a: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->D=v&~0x2;_WR(a,c->D); }return ticks; //RES 1,(c->IX+d),D
            case 0x8b: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->E=v&~0x2;_WR(a,c->E); }return ticks; //RES 1,(c->IX+d),E
            case 0x8c: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->H=v&~0x2;_WR(a,c->H); }return ticks; //RES 1,(c->IX+d),H
            case 0x8d: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->L=v&~0x2;_WR(a,c->L); }return ticks; //RES 1,(c->IX+d),L
            case 0x8e: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_WR(a,v&~0x2); }return ticks; //RES 1,(c->IX+d)
            case 0x8f: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->A=v&~0x2;_WR(a,c->A); }return ticks; //RES 1,(c->IX+d),A
            case 0x90: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->B=v&~0x4;_WR(a,c->B); }return ticks; //RES 2,(c->IX+d),B
            case 0x91: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->C=v&~0x4;_WR(a,c->C); }return ticks; //RES 2,(c->IX+d),C
            case 0x92: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->D=v&~0x4;_WR(a,c->D); }return ticks; //RES 2,(c->IX+d),D
            case 0x93: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->E=v&~0x4;_WR(a,c->E); }return ticks; //RES 2,(c->IX+d),E
            case 0x94: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->H=v&~0x4;_WR(a,c->H); }return ticks; //RES 2,(c->IX+d),H
            case 0x95: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->L=v&~0x4;_WR(a,c->L); }return ticks; //RES 2,(c->IX+d),L
            case 0x96: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_WR(a,v&~0x4); }return ticks; //RES 2,(c->IX+d)
            case 0x97: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->A=v&~0x4;_WR(a,c->A); }return ticks; //RES 2,(c->IX+d),A
            case 0x98: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->B=v&~0x8;_WR(a,c->B); }return ticks; //RES 3,(c->IX+d),B
            case 0x99: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->C=v&~0x8;_WR(a,c->C); }return ticks; //RES 3,(c->IX+d),C
            case 0x9a: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->D=v&~0x8;_WR(a,c->D); }return ticks; //RES 3,(c->IX+d),D
            case 0x9b: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->E=v&~0x8;_WR(a,c->E); }return ticks; //RES 3,(c->IX+d),E
            case 0x9c: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->H=v&~0x8;_WR(a,c->H); }return ticks; //RES 3,(c->IX+d),H
            case 0x9d: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->L=v&~0x8;_WR(a,c->L); }return ticks; //RES 3,(c->IX+d),L
            case 0x9e: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_WR(a,v&~0x8); }return ticks; //RES 3,(c->IX+d)
            case 0x9f: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->A=v&~0x8;_WR(a,c->A); }return ticks; //RES 3,(c->IX+d),A
            case 0xa0: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->B=v&~0x10;_WR(a,c->B); }return ticks; //RES 4,(c->IX+d),B
            case 0xa1: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->C=v&~0x10;_WR(a,c->C); }return ticks; //RES 4,(c->IX+d),C
            case 0xa2: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->D=v&~0x10;_WR(a,c->D); }return ticks; //RES 4,(c->IX+d),D
            case 0xa3: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->E=v&~0x10;_WR(a,c->E); }return ticks; //RES 4,(c->IX+d),E
            case 0xa4: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->H=v&~0x10;_WR(a,c->H); }return ticks; //RES 4,(c->IX+d),H
            case 0xa5: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->L=v&~0x10;_WR(a,c->L); }return ticks; //RES 4,(c->IX+d),L
            case 0xa6: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_WR(a,v&~0x10); }return ticks; //RES 4,(c->IX+d)
            case 0xa7: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->A=v&~0x10;_WR(a,c->A); }return ticks; //RES 4,(c->IX+d),A
            case 0xa8: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->B=v&~0x20;_WR(a,c->B); }return ticks; //RES 5,(c->IX+d),B
            case 0xa9: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->C=v&~0x20;_WR(a,c->C); }return ticks; //RES 5,(c->IX+d),C
            case 0xaa: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->D=v&~0x20;_WR(a,c->D); }return ticks; //RES 5,(c->IX+d),D
            case 0xab: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->E=v&~0x20;_WR(a,c->E); }return ticks; //RES 5,(c->IX+d),E
            case 0xac: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->H=v&~0x20;_WR(a,c->H); }return ticks; //RES 5,(c->IX+d),H
            case 0xad: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->L=v&~0x20;_WR(a,c->L); }return ticks; //RES 5,(c->IX+d),L
            case 0xae: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_WR(a,v&~0x20); }return ticks; //RES 5,(c->IX+d)
            case 0xaf: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->A=v&~0x20;_WR(a,c->A); }return ticks; //RES 5,(c->IX+d),A
            case 0xb0: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->B=v&~0x40;_WR(a,c->B); }return ticks; //RES 6,(c->IX+d),B
            case 0xb1: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->C=v&~0x40;_WR(a,c->C); }return ticks; //RES 6,(c->IX+d),C
            case 0xb2: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->D=v&~0x40;_WR(a,c->D); }return ticks; //RES 6,(c->IX+d),D
            case 0xb3: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->E=v&~0x40;_WR(a,c->E); }return ticks; //RES 6,(c->IX+d),E
            case 0xb4: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->H=v&~0x40;_WR(a,c->H); }return ticks; //RES 6,(c->IX+d),H
            case 0xb5: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->L=v&~0x40;_WR(a,c->L); }return ticks; //RES 6,(c->IX+d),L
            case 0xb6: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_WR(a,v&~0x40); }return ticks; //RES 6,(c->IX+d)
            case 0xb7: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->A=v&~0x40;_WR(a,c->A); }return ticks; //RES 6,(c->IX+d),A
            case 0xb8: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->B=v&~0x80;_WR(a,c->B); }return ticks; //RES 7,(c->IX+d),B
            case 0xb9: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->C=v&~0x80;_WR(a,c->C); }return ticks; //RES 7,(c->IX+d),C
            case 0xba: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->D=v&~0x80;_WR(a,c->D); }return ticks; //RES 7,(c->IX+d),D
            case 0xbb: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->E=v&~0x80;_WR(a,c->E); }return ticks; //RES 7,(c->IX+d),E
            case 0xbc: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->H=v&~0x80;_WR(a,c->H); }return ticks; //RES 7,(c->IX+d),H
            case 0xbd: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->L=v&~0x80;_WR(a,c->L); }return ticks; //RES 7,(c->IX+d),L
            case 0xbe: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_WR(a,v&~0x80); }return ticks; //RES 7,(c->IX+d)
            case 0xbf: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->A=v&~0x80;_WR(a,c->A); }return ticks; //RES 7,(c->IX+d),A
            case 0xc0: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->B=v|0x1;_WR(a,c->B);}return ticks; //SET 0,(c->IX+d),B
            case 0xc1: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->C=v|0x1;_WR(a,c->C);}return ticks; //SET 0,(c->IX+d),C
            case 0xc2: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->D=v|0x1;_WR(a,c->D);}return ticks; //SET 0,(c->IX+d),D
            case 0xc3: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->E=v|0x1;_WR(a,c->E);}return ticks; //SET 0,(c->IX+d),E
            case 0xc4: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->H=v|0x1;_WR(a,c->IXH);}return ticks; //SET 0,(c->IX+d),H
            case 0xc5: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->L=v|0x1;_WR(a,c->IXL);}return ticks; //SET 0,(c->IX+d),L
            case 0xc6: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_WR(a,v|0x1);}return ticks; //SET 0,(c->IX+d)
            case 0xc7: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->A=v|0x1;_WR(a,c->A);}return ticks; //SET 0,(c->IX+d),A
            case 0xc8: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->B=v|0x2;_WR(a,c->B);}return ticks; //SET 1,(c->IX+d),B
            case 0xc9: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->C=v|0x2;_WR(a,c->C);}return ticks; //SET 1,(c->IX+d),C
            case 0xca: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->D=v|0x2;_WR(a,c->D);}return ticks; //SET 1,(c->IX+d),D
            case 0xcb: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->E=v|0x2;_WR(a,c->E);}return ticks; //SET 1,(c->IX+d),E
            case 0xcc: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->H=v|0x2;_WR(a,c->IXH);}return ticks; //SET 1,(c->IX+d),H
            case 0xcd: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->L=v|0x2;_WR(a,c->IXL);}return ticks; //SET 1,(c->IX+d),L
            case 0xce: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_WR(a,v|0x2);}return ticks; //SET 1,(c->IX+d)
            case 0xcf: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->A=v|0x2;_WR(a,c->A);}return ticks; //SET 1,(c->IX+d),A
            case 0xd0: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->B=v|0x4;_WR(a,c->B);}return ticks; //SET 2,(c->IX+d),B
            case 0xd1: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->C=v|0x4;_WR(a,c->C);}return ticks; //SET 2,(c->IX+d),C
            case 0xd2: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->D=v|0x4;_WR(a,c->D);}return ticks; //SET 2,(c->IX+d),D
            case 0xd3: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->E=v|0x4;_WR(a,c->E);}return ticks; //SET 2,(c->IX+d),E
            case 0xd4: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->H=v|0x4;_WR(a,c->IXH);}return ticks; //SET 2,(c->IX+d),H
            case 0xd5: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->L=v|0x4;_WR(a,c->IXL);}return ticks; //SET 2,(c->IX+d),L
            case 0xd6: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_WR(a,v|0x4);}return ticks; //SET 2,(c->IX+d)
            case 0xd7: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->A=v|0x4;_WR(a,c->A);}return ticks; //SET 2,(c->IX+d),A
            case 0xd8: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->B=v|0x8;_WR(a,c->B);}return ticks; //SET 3,(c->IX+d),B
            case 0xd9: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->C=v|0x8;_WR(a,c->C);}return ticks; //SET 3,(c->IX+d),C
            case 0xda: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->D=v|0x8;_WR(a,c->D);}return ticks; //SET 3,(c->IX+d),D
            case 0xdb: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->E=v|0x8;_WR(a,c->E);}return ticks; //SET 3,(c->IX+d),E
            case 0xdc: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->H=v|0x8;_WR(a,c->IXH);}return ticks; //SET 3,(c->IX+d),H
            case 0xdd: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->L=v|0x8;_WR(a,c->IXL);}return ticks; //SET 3,(c->IX+d),L
            case 0xde: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_WR(a,v|0x8);}return ticks; //SET 3,(c->IX+d)
            case 0xdf: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->A=v|0x8;_WR(a,c->A);}return ticks; //SET 3,(c->IX+d),A
            case 0xe0: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->B=v|0x10;_WR(a,c->B);}return ticks; //SET 4,(c->IX+d),B
            case 0xe1: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->C=v|0x10;_WR(a,c->C);}return ticks; //SET 4,(c->IX+d),C
            case 0xe2: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->D=v|0x10;_WR(a,c->D);}return ticks; //SET 4,(c->IX+d),D
            case 0xe3: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->E=v|0x10;_WR(a,c->E);}return ticks; //SET 4,(c->IX+d),E
            case 0xe4: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->H=v|0x10;_WR(a,c->IXH);}return ticks; //SET 4,(c->IX+d),H
            case 0xe5: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->L=v|0x10;_WR(a,c->IXL);}return ticks; //SET 4,(c->IX+d),L
            case 0xe6: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_WR(a,v|0x10);}return ticks; //SET 4,(c->IX+d)
            case 0xe7: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->A=v|0x10;_WR(a,c->A);}return ticks; //SET 4,(c->IX+d),A
            case 0xe8: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->B=v|0x20;_WR(a,c->B);}return ticks; //SET 5,(c->IX+d),B
            case 0xe9: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->C=v|0x20;_WR(a,c->C);}return ticks; //SET 5,(c->IX+d),C
            case 0xea: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->D=v|0x20;_WR(a,c->D);}return ticks; //SET 5,(c->IX+d),D
            case 0xeb: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->E=v|0x20;_WR(a,c->E);}return ticks; //SET 5,(c->IX+d),E
            case 0xec: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->H=v|0x20;_WR(a,c->IXH);}return ticks; //SET 5,(c->IX+d),H
            case 0xed: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->L=v|0x20;_WR(a,c->IXL);}return ticks; //SET 5,(c->IX+d),L
            case 0xee: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_WR(a,v|0x20);}return ticks; //SET 5,(c->IX+d)
            case 0xef: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->A=v|0x20;_WR(a,c->A);}return ticks; //SET 5,(c->IX+d),A
            case 0xf0: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->B=v|0x40;_WR(a,c->B);}return ticks; //SET 6,(c->IX+d),B
            case 0xf1: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->C=v|0x40;_WR(a,c->C);}return ticks; //SET 6,(c->IX+d),C
            case 0xf2: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->D=v|0x40;_WR(a,c->D);}return ticks; //SET 6,(c->IX+d),D
            case 0xf3: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->E=v|0x40;_WR(a,c->E);}return ticks; //SET 6,(c->IX+d),E
            case 0xf4: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->H=v|0x40;_WR(a,c->IXH);}return ticks; //SET 6,(c->IX+d),H
            case 0xf5: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->L=v|0x40;_WR(a,c->IXL);}return ticks; //SET 6,(c->IX+d),L
            case 0xf6: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_WR(a,v|0x40);}return ticks; //SET 6,(c->IX+d)
            case 0xf7: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->A=v|0x40;_WR(a,c->A);}return ticks; //SET 6,(c->IX+d),A
            case 0xf8: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->B=v|0x80;_WR(a,c->B);}return ticks; //SET 7,(c->IX+d),B
            case 0xf9: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->C=v|0x80;_WR(a,c->C);}return ticks; //SET 7,(c->IX+d),C
            case 0xfa: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->D=v|0x80;_WR(a,c->D);}return ticks; //SET 7,(c->IX+d),D
            case 0xfb: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->E=v|0x80;_WR(a,c->E);}return ticks; //SET 7,(c->IX+d),E
            case 0xfc: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->H=v|0x80;_WR(a,c->IXH);}return ticks; //SET 7,(c->IX+d),H
            case 0xfd: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->L=v|0x80;_WR(a,c->IXL);}return ticks; //SET 7,(c->IX+d),L
            case 0xfe: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_WR(a,v|0x80);}return ticks; //SET 7,(c->IX+d)
            case 0xff: { a=c->WZ=c->IX+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->A=v|0x80;_WR(a,c->A);}return ticks; //SET 7,(c->IX+d),A
            default: return ticks;
          }
          break;
        case 0xcc: ticks=_z80_callcc(c,(c->F&Z80_ZF),tick,ticks);return ticks; //CALL Z,nn
        case 0xcd: ticks=_z80_call(c,tick,ticks);return ticks; //CALL nn
        case 0xce: {uint8_t v;_RD(c->PC++,v);_z80_adc(c,v);}return ticks; //ADC n
        case 0xcf: ticks=_z80_rst(c,0x8,tick,ticks);return ticks; //RST 0x8
        case 0xd0: ticks=_z80_retcc(c,!(c->F&Z80_CF),tick,ticks);return ticks; //RET NC
        case 0xd1: /*>rd*/c->ADDR=c->SP++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;l=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*//*>rd*/c->ADDR=c->SP++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;h=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;c->DE=(h<<8)|l;return ticks; //POP DE
        case 0xd2: _IMM16(); if (!(c->F&Z80_CF)) { c->PC=c->WZ; }return ticks; //JP NC,nn
        case 0xd3: {uint8_t v;_RD(c->PC++,v);c->WZ=((c->A<<8)|v);_OUT(c->WZ,c->A);c->Z++;}return ticks; //OUT (n),A
        case 0xd4: ticks=_z80_callcc(c,!(c->F&Z80_CF),tick,ticks);return ticks; //CALL NC,nn
        case 0xd5: _T();_WR(--c->SP,(uint8_t)(c->DE>>8)); _WR(--c->SP,(uint8_t)c->DE);return ticks; //PUSH DE
        case 0xd6: {uint8_t v;_RD(c->PC++,v);_z80_sub(c,v);}return ticks; //SUB n
        case 0xd7: ticks=_z80_rst(c,0x10,tick,ticks);return ticks; //RST 0x10
        case 0xd8: ticks=_z80_retcc(c,(c->F&Z80_CF),tick,ticks);return ticks; //RET C
        case 0xd9: _SWP16(c->BC,c->BC_);_SWP16(c->DE,c->DE_);_SWP16(c->HL,c->HL_);return ticks; //EXX
        case 0xda: _IMM16(); if ((c->F&Z80_CF)) { c->PC=c->WZ; }return ticks; //JP C,nn
        case 0xdb: {uint8_t v;_RD(c->PC++,v);c->WZ=((c->A<<8)|v);_IN(c->WZ++,c->A);}return ticks; //IN A,(n)
        case 0xdc: ticks=_z80_callcc(c,(c->F&Z80_CF),tick,ticks);return ticks; //CALL C,nn
        case 0xde: {uint8_t v;_RD(c->PC++,v);_z80_sbc(c,v);}return ticks; //SBC n
        case 0xdf: ticks=_z80_rst(c,0x18,tick,ticks);return ticks; //RST 0x18
        case 0xe0: ticks=_z80_retcc(c,!(c->F&Z80_PF),tick,ticks);return ticks; //RET PO
        case 0xe1: /*>rd*/c->ADDR=c->SP++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;l=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*//*>rd*/c->ADDR=c->SP++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;h=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;c->IX=(h<<8)|l;return ticks; //POP IX
        case 0xe2: _IMM16(); if (!(c->F&Z80_PF)) { c->PC=c->WZ; }return ticks; //JP PO,nn
        case 0xe3: _T();_RD(c->SP,c->Z);_RD(c->SP+1,c->W);_WR(c->SP,(uint8_t)c->IX);_WR(c->SP+1,(uint8_t)(c->IX>>8));c->IX=c->WZ;_T();_T();return ticks; //EX (SP),IX
        case 0xe4: ticks=_z80_callcc(c,!(c->F&Z80_PF),tick,ticks);return ticks; //CALL PO,nn
        case 0xe5: _T();_WR(--c->SP,(uint8_t)(c->IX>>8)); _WR(--c->SP,(uint8_t)c->IX);return ticks; //PUSH IX
        case 0xe6: {uint8_t v;_RD(c->PC++,v);_z80_and(c,v);}return ticks; //AND n
        case 0xe7: ticks=_z80_rst(c,0x20,tick,ticks);return ticks; //RST 0x20
        case 0xe8: ticks=_z80_retcc(c,(c->F&Z80_PF),tick,ticks);return ticks; //RET PE
        case 0xe9: c->PC=c->IX;return ticks; //JP IX
        case 0xea: _IMM16(); if ((c->F&Z80_PF)) { c->PC=c->WZ; }return ticks; //JP PE,nn
        case 0xeb: _SWP16(c->DE,c->HL);return ticks; //EX DE,HL
        case 0xec: ticks=_z80_callcc(c,(c->F&Z80_PF),tick,ticks);return ticks; //CALL PE,nn
        case 0xee: {uint8_t v;_RD(c->PC++,v);_z80_xor(c,v);}return ticks; //XOR n
        case 0xef: ticks=_z80_rst(c,0x28,tick,ticks);return ticks; //RST 0x28
        case 0xf0: ticks=_z80_retcc(c,!(c->F&Z80_SF),tick,ticks);return ticks; //RET P
        case 0xf1: /*>rd*/c->ADDR=c->SP++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;l=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*//*>rd*/c->ADDR=c->SP++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;h=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;c->AF=(h<<8)|l;return ticks; //POP AF
        case 0xf2: _IMM16(); if (!(c->F&Z80_SF)) { c->PC=c->WZ; }return ticks; //JP P,nn
        case 0xf3: _z80_di(c);return ticks; //DI
        case 0xf4: ticks=_z80_callcc(c,!(c->F&Z80_SF),tick,ticks);return ticks; //CALL P,nn
        case 0xf5: _T();_WR(--c->SP,(uint8_t)(c->AF>>8)); _WR(--c->SP,(uint8_t)c->AF);return ticks; //PUSH AF
        case 0xf6: {uint8_t v;_RD(c->PC++,v);_z80_or(c,v);}return ticks; //OR n
        case 0xf7: ticks=_z80_rst(c,0x30,tick,ticks);return ticks; //RST 0x30
        case 0xf8: ticks=_z80_retcc(c,(c->F&Z80_SF),tick,ticks);return ticks; //RET M
        case 0xf9: tick(c);ticks++;tick(c);ticks++;c->SP=c->IX;return ticks; //LD SP,IX
        case 0xfa: _IMM16(); if ((c->F&Z80_SF)) { c->PC=c->WZ; }return ticks; //JP M,nn
        case 0xfb: _z80_ei(c);return ticks; //EI
        case 0xfc: ticks=_z80_callcc(c,(c->F&Z80_SF),tick,ticks);return ticks; //CALL M,nn
        case 0xfe: {uint8_t v;_RD(c->PC++,v);_z80_cp(c,v);}return ticks; //CP n
        case 0xff: ticks=_z80_rst(c,0x38,tick,ticks);return ticks; //RST 0x38
        default: return ticks;
      }
      break;
    case 0xde: {uint8_t v;_RD(c->PC++,v);_z80_sbc(c,v);}return ticks; //SBC n
    case 0xdf: ticks=_z80_rst(c,0x18,tick,ticks);return ticks; //RST 0x18
    case 0xe0: ticks=_z80_retcc(c,!(c->F&Z80_PF),tick,ticks);return ticks; //RET PO
    case 0xe1: /*>rd*/c->ADDR=c->SP++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;l=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*//*>rd*/c->ADDR=c->SP++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;h=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;c->HL=(h<<8)|l;return ticks; //POP HL
    case 0xe2: _IMM16(); if (!(c->F&Z80_PF)) { c->PC=c->WZ; }return ticks; //JP PO,nn
    case 0xe3: _T();_RD(c->SP,c->Z);_RD(c->SP+1,c->W);_WR(c->SP,(uint8_t)c->HL);_WR(c->SP+1,(uint8_t)(c->HL>>8));c->HL=c->WZ;_T();_T();return ticks; //EX (SP),HL
    case 0xe4: ticks=_z80_callcc(c,!(c->F&Z80_PF),tick,ticks);return ticks; //CALL PO,nn
    case 0xe5: _T();_WR(--c->SP,(uint8_t)(c->HL>>8)); _WR(--c->SP,(uint8_t)c->HL);return ticks; //PUSH HL
    case 0xe6: {uint8_t v;_RD(c->PC++,v);_z80_and(c,v);}return ticks; //AND n
    case 0xe7: ticks=_z80_rst(c,0x20,tick,ticks);return ticks; //RST 0x20
    case 0xe8: ticks=_z80_retcc(c,(c->F&Z80_PF),tick,ticks);return ticks; //RET PE
    case 0xe9: c->PC=c->HL;return ticks; //JP HL
    case 0xea: _IMM16(); if ((c->F&Z80_PF)) { c->PC=c->WZ; }return ticks; //JP PE,nn
    case 0xeb: _SWP16(c->DE,c->HL);return ticks; //EX DE,HL
    case 0xec: ticks=_z80_callcc(c,(c->F&Z80_PF),tick,ticks);return ticks; //CALL PE,nn
    case 0xed:
      /*>fetch*/{c->CTRL|=Z80_M1;c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;opcode=c->DATA;c->CTRL&=~(Z80_M1|Z80_MREQ|Z80_RD);c->R=(c->R&0x80)|((c->R+1)&0x7F);c->CTRL|=Z80_RFSH;tick(c);ticks++;c->CTRL|=Z80_MREQ;c->ADDR=c->IR;tick(c);ticks++;c->CTRL&=~(Z80_RFSH|Z80_MREQ);}/*<fetch*/
      switch (opcode) {
        case 0x40: c->WZ=c->BC;_IN(c->WZ++,c->B);c->F=c->szp[c->B]|(c->F&Z80_CF);return ticks; //IN B,(C)
        case 0x41: c->WZ=c->BC;_OUT(c->WZ++,c->B);return ticks; //OUT (C),B
        case 0x42: c->HL=_z80_sbc16(c,c->HL,c->BC);_T();_T();_T();_T();_T();_T();_T();return ticks; //SBC HL,BC
        case 0x43: _IMM16();_WR(c->WZ++,(uint8_t)c->BC);_WR(c->WZ,(uint8_t)(c->BC>>8));return ticks; //LD (nn),BC
        case 0x44: _z80_neg(c);return ticks; //NEG
        case 0x46: c->IM=0;return ticks; //IM 0
        case 0x47: _T(); c->I=c->A;return ticks; //LD I,A
        case 0x48: c->WZ=c->BC;_IN(c->WZ++,c->C);c->F=c->szp[c->C]|(c->F&Z80_CF);return ticks; //IN C,(C)
        case 0x49: c->WZ=c->BC;_OUT(c->WZ++,c->C);return ticks; //OUT (C),C
        case 0x4a: c->HL=_z80_adc16(c,c->HL,c->BC);_T();_T();_T();_T();_T();_T();_T();return ticks; //ADC HL,BC
        case 0x4b: {_IMM16();uint8_t l;_RD(c->WZ++,l);uint8_t h;_RD(c->WZ,h);c->BC=(h<<8)|l;}return ticks; //LD BC,(nn)
        case 0x4c: _z80_neg(c);return ticks; //NEG
        case 0x4d: _z80_reti(c);return ticks; //RETI
        case 0x4e: c->IM=0;return ticks; //IM 0
        case 0x4f: _T(); c->R=c->A;return ticks; //LD R,A
        case 0x50: c->WZ=c->BC;_IN(c->WZ++,c->D);c->F=c->szp[c->D]|(c->F&Z80_CF);return ticks; //IN D,(C)
        case 0x51: c->WZ=c->BC;_OUT(c->WZ++,c->D);return ticks; //OUT (C),D
        case 0x52: c->HL=_z80_sbc16(c,c->HL,c->DE);_T();_T();_T();_T();_T();_T();_T();return ticks; //SBC HL,DE
        case 0x53: _IMM16();_WR(c->WZ++,(uint8_t)c->DE);_WR(c->WZ,(uint8_t)(c->DE>>8));return ticks; //LD (nn),DE
        case 0x54: _z80_neg(c);return ticks; //NEG
        case 0x56: c->IM=1;return ticks; //IM 1
        case 0x57: _T(); c->A=c->I; c->F=_z80_sziff2(c,c->I)|(c->F&Z80_CF);return ticks; //LD A,I
        case 0x58: c->WZ=c->BC;_IN(c->WZ++,c->E);c->F=c->szp[c->E]|(c->F&Z80_CF);return ticks; //IN E,(C)
        case 0x59: c->WZ=c->BC;_OUT(c->WZ++,c->E);return ticks; //OUT (C),E
        case 0x5a: c->HL=_z80_adc16(c,c->HL,c->DE);_T();_T();_T();_T();_T();_T();_T();return ticks; //ADC HL,DE
        case 0x5b: {_IMM16();uint8_t l;_RD(c->WZ++,l);uint8_t h;_RD(c->WZ,h);c->DE=(h<<8)|l;}return ticks; //LD DE,(nn)
        case 0x5c: _z80_neg(c);return ticks; //NEG
        case 0x5e: c->IM=2;return ticks; //IM 2
        case 0x5f: _T(); c->A=c->R; c->F=_z80_sziff2(c,c->R)|(c->F&Z80_CF);return ticks; //LD A,R
        case 0x60: c->WZ=c->BC;_IN(c->WZ++,c->H);c->F=c->szp[c->H]|(c->F&Z80_CF);return ticks; //IN H,(C)
        case 0x61: c->WZ=c->BC;_OUT(c->WZ++,c->H);return ticks; //OUT (C),H
        case 0x62: c->HL=_z80_sbc16(c,c->HL,c->HL);_T();_T();_T();_T();_T();_T();_T();return ticks; //SBC HL,HL
        case 0x63: _IMM16();_WR(c->WZ++,(uint8_t)c->HL);_WR(c->WZ,(uint8_t)(c->HL>>8));return ticks; //LD (nn),HL
        case 0x64: _z80_neg(c);return ticks; //NEG
        case 0x66: c->IM=0;return ticks; //IM 0
        case 0x67: ticks=_z80_rrd(c,tick,ticks);return ticks; //RRD
        case 0x68: c->WZ=c->BC;_IN(c->WZ++,c->L);c->F=c->szp[c->L]|(c->F&Z80_CF);return ticks; //IN L,(C)
        case 0x69: c->WZ=c->BC;_OUT(c->WZ++,c->L);return ticks; //OUT (C),L
        case 0x6a: c->HL=_z80_adc16(c,c->HL,c->HL);_T();_T();_T();_T();_T();_T();_T();return ticks; //ADC HL,HL
        case 0x6b: {_IMM16();uint8_t l;_RD(c->WZ++,l);uint8_t h;_RD(c->WZ,h);c->HL=(h<<8)|l;}return ticks; //LD HL,(nn)
        case 0x6c: _z80_neg(c);return ticks; //NEG
        case 0x6e: c->IM=0;return ticks; //IM 0
        case 0x6f: ticks=_z80_rld(c,tick,ticks);return ticks; //RLD
        case 0x70: c->WZ=c->BC;uint8_t v; _IN(c->WZ++,v);c->F=c->szp[v]|(c->F&Z80_CF);return ticks; //IN (C)

        case 0x72: c->HL=_z80_sbc16(c,c->HL,c->SP);_T();_T();_T();_T();_T();_T();_T();return ticks; //SBC HL,SP
        case 0x73: _IMM16();_WR(c->WZ++,(uint8_t)c->SP);_WR(c->WZ,(uint8_t)(c->SP>>8));return ticks; //LD (nn),SP
        case 0x74: _z80_neg(c);return ticks; //NEG
        case 0x76: c->IM=1;return ticks; //IM 1
        case 0x77:  return ticks; //NOP (ED)
        case 0x78: c->WZ=c->BC;_IN(c->WZ++,c->A);c->F=c->szp[c->A]|(c->F&Z80_CF);return ticks; //IN A,(C)
        case 0x79: c->WZ=c->BC;_OUT(c->WZ++,c->A);return ticks; //OUT (C),A
        case 0x7a: c->HL=_z80_adc16(c,c->HL,c->SP);_T();_T();_T();_T();_T();_T();_T();return ticks; //ADC HL,SP
        case 0x7b: {_IMM16();uint8_t l;_RD(c->WZ++,l);uint8_t h;_RD(c->WZ,h);c->SP=(h<<8)|l;}return ticks; //LD SP,(nn)
        case 0x7c: _z80_neg(c);return ticks; //NEG
        case 0x7e: c->IM=2;return ticks; //IM 2
        case 0x7f:  return ticks; //NOP (ED)
        case 0xa0: ticks=_z80_ldi(c,tick,ticks);return ticks; //LDI
        case 0xa1: ticks=_z80_cpi(c,tick,ticks);return ticks; //CPI
        case 0xa2: ticks=_z80_ini(c,tick,ticks);return ticks; //INI
        case 0xa3: ticks=_z80_outi(c,tick,ticks);return ticks; //OUTI
        case 0xa8: ticks=_z80_ldd(c,tick,ticks);return ticks; //LDD
        case 0xa9: ticks=_z80_cpd(c,tick,ticks);return ticks; //CPD
        case 0xaa: ticks=_z80_ind(c,tick,ticks);return ticks; //IND
        case 0xab: ticks=_z80_outd(c,tick,ticks);return ticks; //OUTD
        case 0xb0: ticks=_z80_ldir(c,tick,ticks);return ticks; //LDIR
        case 0xb1: ticks=_z80_cpir(c,tick,ticks);return ticks; //CPIR
        case 0xb2: ticks=_z80_inir(c,tick,ticks);return ticks; //INIR
        case 0xb3: ticks=_z80_otir(c,tick,ticks);return ticks; //OTIR
        case 0xb8: ticks=_z80_lddr(c,tick,ticks);return ticks; //LDDR
        case 0xb9: ticks=_z80_cpdr(c,tick,ticks);return ticks; //CPDR
        case 0xba: ticks=_z80_indr(c,tick,ticks);return ticks; //INDR
        case 0xbb: ticks=_z80_otdr(c,tick,ticks);return ticks; //OTDR
        default: return ticks;
      }
      break;
    case 0xee: {uint8_t v;_RD(c->PC++,v);_z80_xor(c,v);}return ticks; //XOR n
    case 0xef: ticks=_z80_rst(c,0x28,tick,ticks);return ticks; //RST 0x28
    case 0xf0: ticks=_z80_retcc(c,!(c->F&Z80_SF),tick,ticks);return ticks; //RET P
    case 0xf1: /*>rd*/c->ADDR=c->SP++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;l=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*//*>rd*/c->ADDR=c->SP++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;h=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;c->AF=(h<<8)|l;return ticks; //POP AF
    case 0xf2: _IMM16(); if (!(c->F&Z80_SF)) { c->PC=c->WZ; }return ticks; //JP P,nn
    case 0xf3: _z80_di(c);return ticks; //DI
    case 0xf4: ticks=_z80_callcc(c,!(c->F&Z80_SF),tick,ticks);return ticks; //CALL P,nn
    case 0xf5: _T();_WR(--c->SP,(uint8_t)(c->AF>>8)); _WR(--c->SP,(uint8_t)c->AF);return ticks; //PUSH AF
    case 0xf6: {uint8_t v;_RD(c->PC++,v);_z80_or(c,v);}return ticks; //OR n
    case 0xf7: ticks=_z80_rst(c,0x30,tick,ticks);return ticks; //RST 0x30
    case 0xf8: ticks=_z80_retcc(c,(c->F&Z80_SF),tick,ticks);return ticks; //RET M
    case 0xf9: tick(c);ticks++;tick(c);ticks++;c->SP=c->HL;return ticks; //LD SP,HL
    case 0xfa: _IMM16(); if ((c->F&Z80_SF)) { c->PC=c->WZ; }return ticks; //JP M,nn
    case 0xfb: _z80_ei(c);return ticks; //EI
    case 0xfc: ticks=_z80_callcc(c,(c->F&Z80_SF),tick,ticks);return ticks; //CALL M,nn
    case 0xfd:
      /*>fetch*/{c->CTRL|=Z80_M1;c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;opcode=c->DATA;c->CTRL&=~(Z80_M1|Z80_MREQ|Z80_RD);c->R=(c->R&0x80)|((c->R+1)&0x7F);c->CTRL|=Z80_RFSH;tick(c);ticks++;c->CTRL|=Z80_MREQ;c->ADDR=c->IR;tick(c);ticks++;c->CTRL&=~(Z80_RFSH|Z80_MREQ);}/*<fetch*/
      switch (opcode) {
        case 0x0:  return ticks; //NOP
        case 0x1: _IMM16(); c->BC=c->WZ;return ticks; //LD BC,nn
        case 0x2: c->WZ=c->BC;/*>wr*/c->ADDR=c->WZ++;tick(c);ticks++;c->CTRL|=(Z80_MREQ|Z80_WR);c->DATA=c->A;tick(c);ticks++;c->CTRL&=~(Z80_MREQ|Z80_WR);tick(c);ticks++;/*wr<*/;c->W=c->A;return ticks; //LD (BC),A
        case 0x3: tick(c);ticks++;tick(c);ticks++;c->BC++;return ticks; //INC BC
        case 0x4: c->B=_z80_inc(c,c->B);return ticks; //INC B
        case 0x5: c->B=_z80_dec(c,c->B);return ticks; //DEC B
        case 0x6: /*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;c->B=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/return ticks; //LD B,n
        case 0x7: _z80_rlca(c);return ticks; //RLCA
        case 0x8: _SWP16(c->AF,c->AF_);return ticks; //EX AF,AF'
        case 0x9: c->IY=_z80_add16(c,c->IY,c->BC);tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;return ticks; //ADD IY,BC
        case 0xa: c->WZ=c->BC;/*>rd*/c->ADDR=c->WZ++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;c->A=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;return ticks; //LD A,(BC)
        case 0xb: tick(c);ticks++;tick(c);ticks++;c->BC--;return ticks; //DEC BC
        case 0xc: c->C=_z80_inc(c,c->C);return ticks; //INC C
        case 0xd: c->C=_z80_dec(c,c->C);return ticks; //DEC C
        case 0xe: /*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;c->C=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/return ticks; //LD C,n
        case 0xf: _z80_rrca(c);return ticks; //RRCA
        case 0x10: ticks=_z80_djnz(c,tick,ticks);return ticks; //DJNZ
        case 0x11: _IMM16(); c->DE=c->WZ;return ticks; //LD DE,nn
        case 0x12: c->WZ=c->DE;/*>wr*/c->ADDR=c->WZ++;tick(c);ticks++;c->CTRL|=(Z80_MREQ|Z80_WR);c->DATA=c->A;tick(c);ticks++;c->CTRL&=~(Z80_MREQ|Z80_WR);tick(c);ticks++;/*wr<*/;c->W=c->A;return ticks; //LD (DE),A
        case 0x13: tick(c);ticks++;tick(c);ticks++;c->DE++;return ticks; //INC DE
        case 0x14: c->D=_z80_inc(c,c->D);return ticks; //INC D
        case 0x15: c->D=_z80_dec(c,c->D);return ticks; //DEC D
        case 0x16: /*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;c->D=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/return ticks; //LD D,n
        case 0x17: _z80_rla(c);return ticks; //RLA
        case 0x18: ticks=_z80_jr(c,tick,ticks);return ticks; //JR d
        case 0x19: c->IY=_z80_add16(c,c->IY,c->DE);tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;return ticks; //ADD IY,DE
        case 0x1a: c->WZ=c->DE;/*>rd*/c->ADDR=c->WZ++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;c->A=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;return ticks; //LD A,(DE)
        case 0x1b: tick(c);ticks++;tick(c);ticks++;c->DE--;return ticks; //DEC DE
        case 0x1c: c->E=_z80_inc(c,c->E);return ticks; //INC E
        case 0x1d: c->E=_z80_dec(c,c->E);return ticks; //DEC E
        case 0x1e: /*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;c->E=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/return ticks; //LD E,n
        case 0x1f: _z80_rra(c);return ticks; //RRA
        case 0x20: ticks=_z80_jr_cc(c,!(c->F&Z80_ZF),tick,ticks);return ticks; //JR NZ,d
        case 0x21: _IMM16(); c->IY=c->WZ;return ticks; //LD IY,nn
        case 0x22: _IMM16();/*>wr*/c->ADDR=c->WZ++;tick(c);ticks++;c->CTRL|=(Z80_MREQ|Z80_WR);c->DATA=(uint8_t)c->IY;tick(c);ticks++;c->CTRL&=~(Z80_MREQ|Z80_WR);tick(c);ticks++;/*wr<*//*>wr*/c->ADDR=c->WZ;tick(c);ticks++;c->CTRL|=(Z80_MREQ|Z80_WR);c->DATA=(uint8_t)(c->IY>>8);tick(c);ticks++;c->CTRL&=~(Z80_MREQ|Z80_WR);tick(c);ticks++;/*wr<*/return ticks; //LD (nn),IY
        case 0x23: tick(c);ticks++;tick(c);ticks++;c->IY++;return ticks; //INC IY
        case 0x24: c->IYH=_z80_inc(c,c->IYH);return ticks; //INC IYH
        case 0x25: c->IYH=_z80_dec(c,c->IYH);return ticks; //DEC IYH
        case 0x26: /*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;c->IYH=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/return ticks; //LD IYH,n
        case 0x27: _z80_daa(c);return ticks; //DAA
        case 0x28: ticks=_z80_jr_cc(c,(c->F&Z80_ZF),tick,ticks);return ticks; //JR Z,d
        case 0x29: c->IY=_z80_add16(c,c->IY,c->IY);tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;return ticks; //ADD IY,IY
        case 0x2a: _IMM16();/*>rd*/c->ADDR=c->WZ++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;l=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*//*>rd*/c->ADDR=c->WZ;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;h=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/c->IY=(h<<8)|l;return ticks; //LD IY,(nn)
        case 0x2b: tick(c);ticks++;tick(c);ticks++;c->IY--;return ticks; //DEC IY
        case 0x2c: c->IYL=_z80_inc(c,c->IYL);return ticks; //INC IYL
        case 0x2d: c->IYL=_z80_dec(c,c->IYL);return ticks; //DEC IYL
        case 0x2e: /*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;c->IYL=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/return ticks; //LD IYL,n
        case 0x2f: _z80_cpl(c);return ticks; //CPL
        case 0x30: ticks=_z80_jr_cc(c,!(c->F&Z80_CF),tick,ticks);return ticks; //JR NC,d
        case 0x31: _IMM16(); c->SP=c->WZ;return ticks; //LD SP,nn
        case 0x32: _IMM16();/*>wr*/c->ADDR=c->WZ++;tick(c);ticks++;c->CTRL|=(Z80_MREQ|Z80_WR);c->DATA=c->A;tick(c);ticks++;c->CTRL&=~(Z80_MREQ|Z80_WR);tick(c);ticks++;/*wr<*/;c->W=c->A;return ticks; //LD (nn),A
        case 0x33: tick(c);ticks++;tick(c);ticks++;c->SP++;return ticks; //INC SP
        case 0x34: /*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;d=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;a=c->WZ=c->IY+d;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;/*>rd*/c->ADDR=a;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;v=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*//*>wr*/c->ADDR=a;tick(c);ticks++;c->CTRL|=(Z80_MREQ|Z80_WR);c->DATA=_z80_inc(c,v);tick(c);ticks++;c->CTRL&=~(Z80_MREQ|Z80_WR);tick(c);ticks++;/*wr<*/return ticks; //INC (c->IY+d)
        case 0x35: /*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;d=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;a=c->WZ=c->IY+d;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;/*>rd*/c->ADDR=a;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;v=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*//*>wr*/c->ADDR=a;tick(c);ticks++;c->CTRL|=(Z80_MREQ|Z80_WR);c->DATA=_z80_dec(c,v);tick(c);ticks++;c->CTRL&=~(Z80_MREQ|Z80_WR);tick(c);ticks++;/*wr<*/return ticks; //DEC (c->IY+d)
        case 0x36: /*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;d=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;a=c->WZ=c->IY+d;tick(c);ticks++;tick(c);ticks++;/*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;v=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*//*>wr*/c->ADDR=a;tick(c);ticks++;c->CTRL|=(Z80_MREQ|Z80_WR);c->DATA=v;tick(c);ticks++;c->CTRL&=~(Z80_MREQ|Z80_WR);tick(c);ticks++;/*wr<*/return ticks; //LD (c->IY+d),n
        case 0x37: _z80_scf(c);return ticks; //SCF
        case 0x38: ticks=_z80_jr_cc(c,(c->F&Z80_CF),tick,ticks);return ticks; //JR C,d
        case 0x39: c->IY=_z80_add16(c,c->IY,c->SP);tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;return ticks; //ADD IY,SP
        case 0x3a: _IMM16();/*>rd*/c->ADDR=c->WZ++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;c->A=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;return ticks; //LD A,(nn)
        case 0x3b: tick(c);ticks++;tick(c);ticks++;c->SP--;return ticks; //DEC SP
        case 0x3c: c->A=_z80_inc(c,c->A);return ticks; //INC A
        case 0x3d: c->A=_z80_dec(c,c->A);return ticks; //DEC A
        case 0x3e: /*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;c->A=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/return ticks; //LD A,n
        case 0x3f: _z80_ccf(c);return ticks; //CCF
        case 0x40: c->B=c->B;return ticks; //LD B,B
        case 0x41: c->B=c->C;return ticks; //LD B,C
        case 0x42: c->B=c->D;return ticks; //LD B,D
        case 0x43: c->B=c->E;return ticks; //LD B,E
        case 0x44: c->B=c->IYH;return ticks; //LD B,IYH
        case 0x45: c->B=c->IYL;return ticks; //LD B,IYL
        case 0x46: {/*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;d=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;a=c->WZ=c->IY+d;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;/*>rd*/c->ADDR=a;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;c->B=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/}return ticks; //LD B,(c->IY+d)
        case 0x47: c->B=c->A;return ticks; //LD B,A
        case 0x48: c->C=c->B;return ticks; //LD C,B
        case 0x49: c->C=c->C;return ticks; //LD C,C
        case 0x4a: c->C=c->D;return ticks; //LD C,D
        case 0x4b: c->C=c->E;return ticks; //LD C,E
        case 0x4c: c->C=c->IYH;return ticks; //LD C,IYH
        case 0x4d: c->C=c->IYL;return ticks; //LD C,IYL
        case 0x4e: {/*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;d=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;a=c->WZ=c->IY+d;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;/*>rd*/c->ADDR=a;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;c->C=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/}return ticks; //LD C,(c->IY+d)
        case 0x4f: c->C=c->A;return ticks; //LD C,A
        case 0x50: c->D=c->B;return ticks; //LD D,B
        case 0x51: c->D=c->C;return ticks; //LD D,C
        case 0x52: c->D=c->D;return ticks; //LD D,D
        case 0x53: c->D=c->E;return ticks; //LD D,E
        case 0x54: c->D=c->IYH;return ticks; //LD D,IYH
        case 0x55: c->D=c->IYL;return ticks; //LD D,IYL
        case 0x56: {/*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;d=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;a=c->WZ=c->IY+d;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;/*>rd*/c->ADDR=a;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;c->D=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/}return ticks; //LD D,(c->IY+d)
        case 0x57: c->D=c->A;return ticks; //LD D,A
        case 0x58: c->E=c->B;return ticks; //LD E,B
        case 0x59: c->E=c->C;return ticks; //LD E,C
        case 0x5a: c->E=c->D;return ticks; //LD E,D
        case 0x5b: c->E=c->E;return ticks; //LD E,E
        case 0x5c: c->E=c->IYH;return ticks; //LD E,IYH
        case 0x5d: c->E=c->IYL;return ticks; //LD E,IYL
        case 0x5e: {/*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;d=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;a=c->WZ=c->IY+d;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;/*>rd*/c->ADDR=a;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;c->E=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/}return ticks; //LD E,(c->IY+d)
        case 0x5f: c->E=c->A;return ticks; //LD E,A
        case 0x60: c->IYH=c->B;return ticks; //LD IYH,B
        case 0x61: c->IYH=c->C;return ticks; //LD IYH,C
        case 0x62: c->IYH=c->D;return ticks; //LD IYH,D
        case 0x63: c->IYH=c->E;return ticks; //LD IYH,E
        case 0x64: c->IYH=c->IYH;return ticks; //LD IYH,IYH
        case 0x65: c->IYH=c->IYL;return ticks; //LD IYH,IYL
        case 0x66: {/*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;d=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;a=c->WZ=c->IY+d;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;/*>rd*/c->ADDR=a;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;c->H=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/}return ticks; //LD H,(c->IY+d)
        case 0x67: c->IYH=c->A;return ticks; //LD IYH,A
        case 0x68: c->IYL=c->B;return ticks; //LD IYL,B
        case 0x69: c->IYL=c->C;return ticks; //LD IYL,C
        case 0x6a: c->IYL=c->D;return ticks; //LD IYL,D
        case 0x6b: c->IYL=c->E;return ticks; //LD IYL,E
        case 0x6c: c->IYL=c->IYH;return ticks; //LD IYL,IYH
        case 0x6d: c->IYL=c->IYL;return ticks; //LD IYL,IYL
        case 0x6e: {/*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;d=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;a=c->WZ=c->IY+d;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;/*>rd*/c->ADDR=a;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;c->L=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/}return ticks; //LD L,(c->IY+d)
        case 0x6f: c->IYL=c->A;return ticks; //LD IYL,A
        case 0x70: {/*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;d=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;a=c->WZ=c->IY+d;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;/*>wr*/c->ADDR=a;tick(c);ticks++;c->CTRL|=(Z80_MREQ|Z80_WR);c->DATA=c->B;tick(c);ticks++;c->CTRL&=~(Z80_MREQ|Z80_WR);tick(c);ticks++;/*wr<*/;}return ticks; //LD (c->IY+d),B
        case 0x71: {/*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;d=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;a=c->WZ=c->IY+d;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;/*>wr*/c->ADDR=a;tick(c);ticks++;c->CTRL|=(Z80_MREQ|Z80_WR);c->DATA=c->C;tick(c);ticks++;c->CTRL&=~(Z80_MREQ|Z80_WR);tick(c);ticks++;/*wr<*/;}return ticks; //LD (c->IY+d),C
        case 0x72: {/*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;d=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;a=c->WZ=c->IY+d;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;/*>wr*/c->ADDR=a;tick(c);ticks++;c->CTRL|=(Z80_MREQ|Z80_WR);c->DATA=c->D;tick(c);ticks++;c->CTRL&=~(Z80_MREQ|Z80_WR);tick(c);ticks++;/*wr<*/;}return ticks; //LD (c->IY+d),D
        case 0x73: {/*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;d=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;a=c->WZ=c->IY+d;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;/*>wr*/c->ADDR=a;tick(c);ticks++;c->CTRL|=(Z80_MREQ|Z80_WR);c->DATA=c->E;tick(c);ticks++;c->CTRL&=~(Z80_MREQ|Z80_WR);tick(c);ticks++;/*wr<*/;}return ticks; //LD (c->IY+d),E
        case 0x74: {/*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;d=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;a=c->WZ=c->IY+d;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;/*>wr*/c->ADDR=a;tick(c);ticks++;c->CTRL|=(Z80_MREQ|Z80_WR);c->DATA=c->H;tick(c);ticks++;c->CTRL&=~(Z80_MREQ|Z80_WR);tick(c);ticks++;/*wr<*/;}return ticks; //LD (c->IY+d),H
        case 0x75: {/*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;d=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;a=c->WZ=c->IY+d;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;/*>wr*/c->ADDR=a;tick(c);ticks++;c->CTRL|=(Z80_MREQ|Z80_WR);c->DATA=c->L;tick(c);ticks++;c->CTRL&=~(Z80_MREQ|Z80_WR);tick(c);ticks++;/*wr<*/;}return ticks; //LD (c->IY+d),L
        case 0x76: _z80_halt(c);return ticks; //HALT
        case 0x77: {/*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;d=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;a=c->WZ=c->IY+d;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;/*>wr*/c->ADDR=a;tick(c);ticks++;c->CTRL|=(Z80_MREQ|Z80_WR);c->DATA=c->A;tick(c);ticks++;c->CTRL&=~(Z80_MREQ|Z80_WR);tick(c);ticks++;/*wr<*/;}return ticks; //LD (c->IY+d),A
        case 0x78: c->A=c->B;return ticks; //LD A,B
        case 0x79: c->A=c->C;return ticks; //LD A,C
        case 0x7a: c->A=c->D;return ticks; //LD A,D
        case 0x7b: c->A=c->E;return ticks; //LD A,E
        case 0x7c: c->A=c->IYH;return ticks; //LD A,IYH
        case 0x7d: c->A=c->IYL;return ticks; //LD A,IYL
        case 0x7e: {/*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;d=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;a=c->WZ=c->IY+d;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;/*>rd*/c->ADDR=a;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;c->A=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/}return ticks; //LD A,(c->IY+d)
        case 0x7f: c->A=c->A;return ticks; //LD A,A
        case 0x80: _z80_add(c,c->B);return ticks; //ADD B
        case 0x81: _z80_add(c,c->C);return ticks; //ADD C
        case 0x82: _z80_add(c,c->D);return ticks; //ADD D
        case 0x83: _z80_add(c,c->E);return ticks; //ADD E
        case 0x84: _z80_add(c,c->IYH);return ticks; //ADD IYH
        case 0x85: _z80_add(c,c->IYL);return ticks; //ADD IYL
        case 0x86: /*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;d=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;a=c->WZ=c->IY+d;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;/*>rd*/c->ADDR=a;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;v=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/_z80_add(c,v);return ticks; //ADD (c->IY+d)
        case 0x87: _z80_add(c,c->A);return ticks; //ADD A
        case 0x88: _z80_adc(c,c->B);return ticks; //ADC B
        case 0x89: _z80_adc(c,c->C);return ticks; //ADC C
        case 0x8a: _z80_adc(c,c->D);return ticks; //ADC D
        case 0x8b: _z80_adc(c,c->E);return ticks; //ADC E
        case 0x8c: _z80_adc(c,c->IYH);return ticks; //ADC IYH
        case 0x8d: _z80_adc(c,c->IYL);return ticks; //ADC IYL
        case 0x8e: /*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;d=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;a=c->WZ=c->IY+d;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;/*>rd*/c->ADDR=a;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;v=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/_z80_adc(c,v);return ticks; //ADC (c->IY+d)
        case 0x8f: _z80_adc(c,c->A);return ticks; //ADC A
        case 0x90: _z80_sub(c,c->B);return ticks; //SUB B
        case 0x91: _z80_sub(c,c->C);return ticks; //SUB C
        case 0x92: _z80_sub(c,c->D);return ticks; //SUB D
        case 0x93: _z80_sub(c,c->E);return ticks; //SUB E
        case 0x94: _z80_sub(c,c->IYH);return ticks; //SUB IYH
        case 0x95: _z80_sub(c,c->IYL);return ticks; //SUB IYL
        case 0x96: /*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;d=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;a=c->WZ=c->IY+d;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;/*>rd*/c->ADDR=a;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;v=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/_z80_sub(c,v);return ticks; //SUB (c->IY+d)
        case 0x97: _z80_sub(c,c->A);return ticks; //SUB A
        case 0x98: _z80_sbc(c,c->B);return ticks; //SBC B
        case 0x99: _z80_sbc(c,c->C);return ticks; //SBC C
        case 0x9a: _z80_sbc(c,c->D);return ticks; //SBC D
        case 0x9b: _z80_sbc(c,c->E);return ticks; //SBC E
        case 0x9c: _z80_sbc(c,c->IYH);return ticks; //SBC IYH
        case 0x9d: _z80_sbc(c,c->IYL);return ticks; //SBC IYL
        case 0x9e: /*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;d=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;a=c->WZ=c->IY+d;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;/*>rd*/c->ADDR=a;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;v=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/_z80_sbc(c,v);return ticks; //SBC (c->IY+d)
        case 0x9f: _z80_sbc(c,c->A);return ticks; //SBC A
        case 0xa0: _z80_and(c,c->B);return ticks; //AND B
        case 0xa1: _z80_and(c,c->C);return ticks; //AND C
        case 0xa2: _z80_and(c,c->D);return ticks; //AND D
        case 0xa3: _z80_and(c,c->E);return ticks; //AND E
        case 0xa4: _z80_and(c,c->IYH);return ticks; //AND IYH
        case 0xa5: _z80_and(c,c->IYL);return ticks; //AND IYL
        case 0xa6: /*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;d=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;a=c->WZ=c->IY+d;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;/*>rd*/c->ADDR=a;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;v=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/_z80_and(c,v);return ticks; //AND (c->IY+d)
        case 0xa7: _z80_and(c,c->A);return ticks; //AND A
        case 0xa8: _z80_xor(c,c->B);return ticks; //XOR B
        case 0xa9: _z80_xor(c,c->C);return ticks; //XOR C
        case 0xaa: _z80_xor(c,c->D);return ticks; //XOR D
        case 0xab: _z80_xor(c,c->E);return ticks; //XOR E
        case 0xac: _z80_xor(c,c->IYH);return ticks; //XOR IYH
        case 0xad: _z80_xor(c,c->IYL);return ticks; //XOR IYL
        case 0xae: /*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;d=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;a=c->WZ=c->IY+d;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;/*>rd*/c->ADDR=a;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;v=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/_z80_xor(c,v);return ticks; //XOR (c->IY+d)
        case 0xaf: _z80_xor(c,c->A);return ticks; //XOR A
        case 0xb0: _z80_or(c,c->B);return ticks; //OR B
        case 0xb1: _z80_or(c,c->C);return ticks; //OR C
        case 0xb2: _z80_or(c,c->D);return ticks; //OR D
        case 0xb3: _z80_or(c,c->E);return ticks; //OR E
        case 0xb4: _z80_or(c,c->IYH);return ticks; //OR IYH
        case 0xb5: _z80_or(c,c->IYL);return ticks; //OR IYL
        case 0xb6: /*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;d=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;a=c->WZ=c->IY+d;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;/*>rd*/c->ADDR=a;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;v=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/_z80_or(c,v);return ticks; //OR (c->IY+d)
        case 0xb7: _z80_or(c,c->A);return ticks; //OR A
        case 0xb8: _z80_cp(c,c->B);return ticks; //CP B
        case 0xb9: _z80_cp(c,c->C);return ticks; //CP C
        case 0xba: _z80_cp(c,c->D);return ticks; //CP D
        case 0xbb: _z80_cp(c,c->E);return ticks; //CP E
        case 0xbc: _z80_cp(c,c->IYH);return ticks; //CP IYH
        case 0xbd: _z80_cp(c,c->IYL);return ticks; //CP IYL
        case 0xbe: /*>rd*/c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;d=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;a=c->WZ=c->IY+d;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;tick(c);ticks++;/*>rd*/c->ADDR=a;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;v=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/_z80_cp(c,v);return ticks; //CP (c->IY+d)
        case 0xbf: _z80_cp(c,c->A);return ticks; //CP A
        case 0xc0: ticks=_z80_retcc(c,!(c->F&Z80_ZF),tick,ticks);return ticks; //RET NZ
        case 0xc1: /*>rd*/c->ADDR=c->SP++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;l=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*//*>rd*/c->ADDR=c->SP++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;h=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;c->BC=(h<<8)|l;return ticks; //POP BC
        case 0xc2: _IMM16(); if (!(c->F&Z80_ZF)) { c->PC=c->WZ; }return ticks; //JP NZ,nn
        case 0xc3: _IMM16(); c->PC=c->WZ;return ticks; //JP nn
        case 0xc4: ticks=_z80_callcc(c,!(c->F&Z80_ZF),tick,ticks);return ticks; //CALL NZ,nn
        case 0xc5: _T();_WR(--c->SP,(uint8_t)(c->BC>>8)); _WR(--c->SP,(uint8_t)c->BC);return ticks; //PUSH BC
        case 0xc6: {uint8_t v;_RD(c->PC++,v);_z80_add(c,v);}return ticks; //ADD n
        case 0xc7: ticks=_z80_rst(c,0x0,tick,ticks);return ticks; //RST 0x0
        case 0xc8: ticks=_z80_retcc(c,(c->F&Z80_ZF),tick,ticks);return ticks; //RET Z
        case 0xc9: ticks=_z80_ret(c,tick,ticks);return ticks; //RET
        case 0xca: _IMM16(); if ((c->F&Z80_ZF)) { c->PC=c->WZ; }return ticks; //JP Z,nn
        case 0xcb:
          _RD(c->PC++, d);
          /*>fetch*/{c->CTRL|=Z80_M1;c->ADDR=c->PC++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;opcode=c->DATA;c->CTRL&=~(Z80_M1|Z80_MREQ|Z80_RD);c->CTRL|=Z80_RFSH;tick(c);ticks++;c->CTRL|=Z80_MREQ;c->ADDR=c->IR;tick(c);ticks++;c->CTRL&=~(Z80_RFSH|Z80_MREQ);}/*<fetch*/
          switch (opcode) {
            case 0x0: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->B=_z80_rlc(c,v);_WR(a,c->B); }return ticks; //RLC (c->IY+d),B
            case 0x1: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->C=_z80_rlc(c,v);_WR(a,c->C); }return ticks; //RLC (c->IY+d),C
            case 0x2: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->D=_z80_rlc(c,v);_WR(a,c->D); }return ticks; //RLC (c->IY+d),D
            case 0x3: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->E=_z80_rlc(c,v);_WR(a,c->E); }return ticks; //RLC (c->IY+d),E
            case 0x4: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->H=_z80_rlc(c,v);_WR(a,c->H); }return ticks; //RLC (c->IY+d),H
            case 0x5: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->L=_z80_rlc(c,v);_WR(a,c->L); }return ticks; //RLC (c->IY+d),L
            case 0x6: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_WR(a,_z80_rlc(c,v)); }return ticks; //RLC (c->IY+d)
            case 0x7: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->A=_z80_rlc(c,v);_WR(a,c->A); }return ticks; //RLC (c->IY+d),A
            case 0x8: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->B=_z80_rrc(c,v);_WR(a,c->B); }return ticks; //RRC (c->IY+d),B
            case 0x9: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->C=_z80_rrc(c,v);_WR(a,c->C); }return ticks; //RRC (c->IY+d),C
            case 0xa: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->D=_z80_rrc(c,v);_WR(a,c->D); }return ticks; //RRC (c->IY+d),D
            case 0xb: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->E=_z80_rrc(c,v);_WR(a,c->E); }return ticks; //RRC (c->IY+d),E
            case 0xc: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->H=_z80_rrc(c,v);_WR(a,c->H); }return ticks; //RRC (c->IY+d),H
            case 0xd: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->L=_z80_rrc(c,v);_WR(a,c->L); }return ticks; //RRC (c->IY+d),L
            case 0xe: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_WR(a,_z80_rrc(c,v)); }return ticks; //RRC (c->IY+d)
            case 0xf: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->A=_z80_rrc(c,v);_WR(a,c->A); }return ticks; //RRC (c->IY+d),A
            case 0x10: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->B=_z80_rl(c,v);_WR(a,c->B); }return ticks; //RL (c->IY+d),B
            case 0x11: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->C=_z80_rl(c,v);_WR(a,c->C); }return ticks; //RL (c->IY+d),C
            case 0x12: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->D=_z80_rl(c,v);_WR(a,c->D); }return ticks; //RL (c->IY+d),D
            case 0x13: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->E=_z80_rl(c,v);_WR(a,c->E); }return ticks; //RL (c->IY+d),E
            case 0x14: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->H=_z80_rl(c,v);_WR(a,c->H); }return ticks; //RL (c->IY+d),H
            case 0x15: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->L=_z80_rl(c,v);_WR(a,c->L); }return ticks; //RL (c->IY+d),L
            case 0x16: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_WR(a,_z80_rl(c,v)); }return ticks; //RL (c->IY+d)
            case 0x17: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->A=_z80_rl(c,v);_WR(a,c->A); }return ticks; //RL (c->IY+d),A
            case 0x18: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->B=_z80_rr(c,v);_WR(a,c->B); }return ticks; //RR (c->IY+d),B
            case 0x19: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->C=_z80_rr(c,v);_WR(a,c->C); }return ticks; //RR (c->IY+d),C
            case 0x1a: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->D=_z80_rr(c,v);_WR(a,c->D); }return ticks; //RR (c->IY+d),D
            case 0x1b: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->E=_z80_rr(c,v);_WR(a,c->E); }return ticks; //RR (c->IY+d),E
            case 0x1c: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->H=_z80_rr(c,v);_WR(a,c->H); }return ticks; //RR (c->IY+d),H
            case 0x1d: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->L=_z80_rr(c,v);_WR(a,c->L); }return ticks; //RR (c->IY+d),L
            case 0x1e: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_WR(a,_z80_rr(c,v)); }return ticks; //RR (c->IY+d)
            case 0x1f: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->A=_z80_rr(c,v);_WR(a,c->A); }return ticks; //RR (c->IY+d),A
            case 0x20: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->B=_z80_sla(c,v);_WR(a,c->B); }return ticks; //SLA (c->IY+d),B
            case 0x21: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->C=_z80_sla(c,v);_WR(a,c->C); }return ticks; //SLA (c->IY+d),C
            case 0x22: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->D=_z80_sla(c,v);_WR(a,c->D); }return ticks; //SLA (c->IY+d),D
            case 0x23: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->E=_z80_sla(c,v);_WR(a,c->E); }return ticks; //SLA (c->IY+d),E
            case 0x24: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->H=_z80_sla(c,v);_WR(a,c->H); }return ticks; //SLA (c->IY+d),H
            case 0x25: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->L=_z80_sla(c,v);_WR(a,c->L); }return ticks; //SLA (c->IY+d),L
            case 0x26: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_WR(a,_z80_sla(c,v)); }return ticks; //SLA (c->IY+d)
            case 0x27: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->A=_z80_sla(c,v);_WR(a,c->A); }return ticks; //SLA (c->IY+d),A
            case 0x28: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->B=_z80_sra(c,v);_WR(a,c->B); }return ticks; //SRA (c->IY+d),B
            case 0x29: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->C=_z80_sra(c,v);_WR(a,c->C); }return ticks; //SRA (c->IY+d),C
            case 0x2a: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->D=_z80_sra(c,v);_WR(a,c->D); }return ticks; //SRA (c->IY+d),D
            case 0x2b: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->E=_z80_sra(c,v);_WR(a,c->E); }return ticks; //SRA (c->IY+d),E
            case 0x2c: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->H=_z80_sra(c,v);_WR(a,c->H); }return ticks; //SRA (c->IY+d),H
            case 0x2d: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->L=_z80_sra(c,v);_WR(a,c->L); }return ticks; //SRA (c->IY+d),L
            case 0x2e: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_WR(a,_z80_sra(c,v)); }return ticks; //SRA (c->IY+d)
            case 0x2f: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->A=_z80_sra(c,v);_WR(a,c->A); }return ticks; //SRA (c->IY+d),A
            case 0x30: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->B=_z80_sll(c,v);_WR(a,c->B); }return ticks; //SLL (c->IY+d),B
            case 0x31: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->C=_z80_sll(c,v);_WR(a,c->C); }return ticks; //SLL (c->IY+d),C
            case 0x32: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->D=_z80_sll(c,v);_WR(a,c->D); }return ticks; //SLL (c->IY+d),D
            case 0x33: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->E=_z80_sll(c,v);_WR(a,c->E); }return ticks; //SLL (c->IY+d),E
            case 0x34: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->H=_z80_sll(c,v);_WR(a,c->H); }return ticks; //SLL (c->IY+d),H
            case 0x35: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->L=_z80_sll(c,v);_WR(a,c->L); }return ticks; //SLL (c->IY+d),L
            case 0x36: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_WR(a,_z80_sll(c,v)); }return ticks; //SLL (c->IY+d)
            case 0x37: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->A=_z80_sll(c,v);_WR(a,c->A); }return ticks; //SLL (c->IY+d),A
            case 0x38: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->B=_z80_srl(c,v);_WR(a,c->B); }return ticks; //SRL (c->IY+d),B
            case 0x39: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->C=_z80_srl(c,v);_WR(a,c->C); }return ticks; //SRL (c->IY+d),C
            case 0x3a: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->D=_z80_srl(c,v);_WR(a,c->D); }return ticks; //SRL (c->IY+d),D
            case 0x3b: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->E=_z80_srl(c,v);_WR(a,c->E); }return ticks; //SRL (c->IY+d),E
            case 0x3c: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->H=_z80_srl(c,v);_WR(a,c->H); }return ticks; //SRL (c->IY+d),H
            case 0x3d: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->L=_z80_srl(c,v);_WR(a,c->L); }return ticks; //SRL (c->IY+d),L
            case 0x3e: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_WR(a,_z80_srl(c,v)); }return ticks; //SRL (c->IY+d)
            case 0x3f: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->A=_z80_srl(c,v);_WR(a,c->A); }return ticks; //SRL (c->IY+d),A
            case 0x40: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x1); }return ticks; //BIT 0,(c->IY+d)
            case 0x41: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x1); }return ticks; //BIT 0,(c->IY+d)
            case 0x42: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x1); }return ticks; //BIT 0,(c->IY+d)
            case 0x43: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x1); }return ticks; //BIT 0,(c->IY+d)
            case 0x44: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x1); }return ticks; //BIT 0,(c->IY+d)
            case 0x45: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x1); }return ticks; //BIT 0,(c->IY+d)
            case 0x46: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x1); }return ticks; //BIT 0,(c->IY+d)
            case 0x47: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x1); }return ticks; //BIT 0,(c->IY+d)
            case 0x48: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x2); }return ticks; //BIT 1,(c->IY+d)
            case 0x49: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x2); }return ticks; //BIT 1,(c->IY+d)
            case 0x4a: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x2); }return ticks; //BIT 1,(c->IY+d)
            case 0x4b: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x2); }return ticks; //BIT 1,(c->IY+d)
            case 0x4c: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x2); }return ticks; //BIT 1,(c->IY+d)
            case 0x4d: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x2); }return ticks; //BIT 1,(c->IY+d)
            case 0x4e: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x2); }return ticks; //BIT 1,(c->IY+d)
            case 0x4f: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x2); }return ticks; //BIT 1,(c->IY+d)
            case 0x50: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x4); }return ticks; //BIT 2,(c->IY+d)
            case 0x51: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x4); }return ticks; //BIT 2,(c->IY+d)
            case 0x52: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x4); }return ticks; //BIT 2,(c->IY+d)
            case 0x53: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x4); }return ticks; //BIT 2,(c->IY+d)
            case 0x54: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x4); }return ticks; //BIT 2,(c->IY+d)
            case 0x55: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x4); }return ticks; //BIT 2,(c->IY+d)
            case 0x56: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x4); }return ticks; //BIT 2,(c->IY+d)
            case 0x57: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x4); }return ticks; //BIT 2,(c->IY+d)
            case 0x58: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x8); }return ticks; //BIT 3,(c->IY+d)
            case 0x59: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x8); }return ticks; //BIT 3,(c->IY+d)
            case 0x5a: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x8); }return ticks; //BIT 3,(c->IY+d)
            case 0x5b: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x8); }return ticks; //BIT 3,(c->IY+d)
            case 0x5c: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x8); }return ticks; //BIT 3,(c->IY+d)
            case 0x5d: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x8); }return ticks; //BIT 3,(c->IY+d)
            case 0x5e: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x8); }return ticks; //BIT 3,(c->IY+d)
            case 0x5f: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x8); }return ticks; //BIT 3,(c->IY+d)
            case 0x60: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x10); }return ticks; //BIT 4,(c->IY+d)
            case 0x61: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x10); }return ticks; //BIT 4,(c->IY+d)
            case 0x62: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x10); }return ticks; //BIT 4,(c->IY+d)
            case 0x63: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x10); }return ticks; //BIT 4,(c->IY+d)
            case 0x64: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x10); }return ticks; //BIT 4,(c->IY+d)
            case 0x65: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x10); }return ticks; //BIT 4,(c->IY+d)
            case 0x66: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x10); }return ticks; //BIT 4,(c->IY+d)
            case 0x67: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x10); }return ticks; //BIT 4,(c->IY+d)
            case 0x68: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x20); }return ticks; //BIT 5,(c->IY+d)
            case 0x69: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x20); }return ticks; //BIT 5,(c->IY+d)
            case 0x6a: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x20); }return ticks; //BIT 5,(c->IY+d)
            case 0x6b: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x20); }return ticks; //BIT 5,(c->IY+d)
            case 0x6c: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x20); }return ticks; //BIT 5,(c->IY+d)
            case 0x6d: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x20); }return ticks; //BIT 5,(c->IY+d)
            case 0x6e: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x20); }return ticks; //BIT 5,(c->IY+d)
            case 0x6f: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x20); }return ticks; //BIT 5,(c->IY+d)
            case 0x70: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x40); }return ticks; //BIT 6,(c->IY+d)
            case 0x71: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x40); }return ticks; //BIT 6,(c->IY+d)
            case 0x72: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x40); }return ticks; //BIT 6,(c->IY+d)
            case 0x73: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x40); }return ticks; //BIT 6,(c->IY+d)
            case 0x74: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x40); }return ticks; //BIT 6,(c->IY+d)
            case 0x75: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x40); }return ticks; //BIT 6,(c->IY+d)
            case 0x76: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x40); }return ticks; //BIT 6,(c->IY+d)
            case 0x77: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x40); }return ticks; //BIT 6,(c->IY+d)
            case 0x78: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x80); }return ticks; //BIT 7,(c->IY+d)
            case 0x79: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x80); }return ticks; //BIT 7,(c->IY+d)
            case 0x7a: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x80); }return ticks; //BIT 7,(c->IY+d)
            case 0x7b: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x80); }return ticks; //BIT 7,(c->IY+d)
            case 0x7c: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x80); }return ticks; //BIT 7,(c->IY+d)
            case 0x7d: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x80); }return ticks; //BIT 7,(c->IY+d)
            case 0x7e: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x80); }return ticks; //BIT 7,(c->IY+d)
            case 0x7f: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x80); }return ticks; //BIT 7,(c->IY+d)
            case 0x80: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->B=v&~0x1;_WR(a,c->B); }return ticks; //RES 0,(c->IY+d),B
            case 0x81: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->C=v&~0x1;_WR(a,c->C); }return ticks; //RES 0,(c->IY+d),C
            case 0x82: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->D=v&~0x1;_WR(a,c->D); }return ticks; //RES 0,(c->IY+d),D
            case 0x83: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->E=v&~0x1;_WR(a,c->E); }return ticks; //RES 0,(c->IY+d),E
            case 0x84: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->H=v&~0x1;_WR(a,c->H); }return ticks; //RES 0,(c->IY+d),H
            case 0x85: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->L=v&~0x1;_WR(a,c->L); }return ticks; //RES 0,(c->IY+d),L
            case 0x86: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_WR(a,v&~0x1); }return ticks; //RES 0,(c->IY+d)
            case 0x87: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->A=v&~0x1;_WR(a,c->A); }return ticks; //RES 0,(c->IY+d),A
            case 0x88: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->B=v&~0x2;_WR(a,c->B); }return ticks; //RES 1,(c->IY+d),B
            case 0x89: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->C=v&~0x2;_WR(a,c->C); }return ticks; //RES 1,(c->IY+d),C
            case 0x8a: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->D=v&~0x2;_WR(a,c->D); }return ticks; //RES 1,(c->IY+d),D
            case 0x8b: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->E=v&~0x2;_WR(a,c->E); }return ticks; //RES 1,(c->IY+d),E
            case 0x8c: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->H=v&~0x2;_WR(a,c->H); }return ticks; //RES 1,(c->IY+d),H
            case 0x8d: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->L=v&~0x2;_WR(a,c->L); }return ticks; //RES 1,(c->IY+d),L
            case 0x8e: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_WR(a,v&~0x2); }return ticks; //RES 1,(c->IY+d)
            case 0x8f: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->A=v&~0x2;_WR(a,c->A); }return ticks; //RES 1,(c->IY+d),A
            case 0x90: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->B=v&~0x4;_WR(a,c->B); }return ticks; //RES 2,(c->IY+d),B
            case 0x91: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->C=v&~0x4;_WR(a,c->C); }return ticks; //RES 2,(c->IY+d),C
            case 0x92: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->D=v&~0x4;_WR(a,c->D); }return ticks; //RES 2,(c->IY+d),D
            case 0x93: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->E=v&~0x4;_WR(a,c->E); }return ticks; //RES 2,(c->IY+d),E
            case 0x94: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->H=v&~0x4;_WR(a,c->H); }return ticks; //RES 2,(c->IY+d),H
            case 0x95: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->L=v&~0x4;_WR(a,c->L); }return ticks; //RES 2,(c->IY+d),L
            case 0x96: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_WR(a,v&~0x4); }return ticks; //RES 2,(c->IY+d)
            case 0x97: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->A=v&~0x4;_WR(a,c->A); }return ticks; //RES 2,(c->IY+d),A
            case 0x98: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->B=v&~0x8;_WR(a,c->B); }return ticks; //RES 3,(c->IY+d),B
            case 0x99: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->C=v&~0x8;_WR(a,c->C); }return ticks; //RES 3,(c->IY+d),C
            case 0x9a: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->D=v&~0x8;_WR(a,c->D); }return ticks; //RES 3,(c->IY+d),D
            case 0x9b: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->E=v&~0x8;_WR(a,c->E); }return ticks; //RES 3,(c->IY+d),E
            case 0x9c: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->H=v&~0x8;_WR(a,c->H); }return ticks; //RES 3,(c->IY+d),H
            case 0x9d: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->L=v&~0x8;_WR(a,c->L); }return ticks; //RES 3,(c->IY+d),L
            case 0x9e: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_WR(a,v&~0x8); }return ticks; //RES 3,(c->IY+d)
            case 0x9f: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->A=v&~0x8;_WR(a,c->A); }return ticks; //RES 3,(c->IY+d),A
            case 0xa0: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->B=v&~0x10;_WR(a,c->B); }return ticks; //RES 4,(c->IY+d),B
            case 0xa1: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->C=v&~0x10;_WR(a,c->C); }return ticks; //RES 4,(c->IY+d),C
            case 0xa2: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->D=v&~0x10;_WR(a,c->D); }return ticks; //RES 4,(c->IY+d),D
            case 0xa3: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->E=v&~0x10;_WR(a,c->E); }return ticks; //RES 4,(c->IY+d),E
            case 0xa4: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->H=v&~0x10;_WR(a,c->H); }return ticks; //RES 4,(c->IY+d),H
            case 0xa5: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->L=v&~0x10;_WR(a,c->L); }return ticks; //RES 4,(c->IY+d),L
            case 0xa6: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_WR(a,v&~0x10); }return ticks; //RES 4,(c->IY+d)
            case 0xa7: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->A=v&~0x10;_WR(a,c->A); }return ticks; //RES 4,(c->IY+d),A
            case 0xa8: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->B=v&~0x20;_WR(a,c->B); }return ticks; //RES 5,(c->IY+d),B
            case 0xa9: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->C=v&~0x20;_WR(a,c->C); }return ticks; //RES 5,(c->IY+d),C
            case 0xaa: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->D=v&~0x20;_WR(a,c->D); }return ticks; //RES 5,(c->IY+d),D
            case 0xab: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->E=v&~0x20;_WR(a,c->E); }return ticks; //RES 5,(c->IY+d),E
            case 0xac: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->H=v&~0x20;_WR(a,c->H); }return ticks; //RES 5,(c->IY+d),H
            case 0xad: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->L=v&~0x20;_WR(a,c->L); }return ticks; //RES 5,(c->IY+d),L
            case 0xae: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_WR(a,v&~0x20); }return ticks; //RES 5,(c->IY+d)
            case 0xaf: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->A=v&~0x20;_WR(a,c->A); }return ticks; //RES 5,(c->IY+d),A
            case 0xb0: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->B=v&~0x40;_WR(a,c->B); }return ticks; //RES 6,(c->IY+d),B
            case 0xb1: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->C=v&~0x40;_WR(a,c->C); }return ticks; //RES 6,(c->IY+d),C
            case 0xb2: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->D=v&~0x40;_WR(a,c->D); }return ticks; //RES 6,(c->IY+d),D
            case 0xb3: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->E=v&~0x40;_WR(a,c->E); }return ticks; //RES 6,(c->IY+d),E
            case 0xb4: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->H=v&~0x40;_WR(a,c->H); }return ticks; //RES 6,(c->IY+d),H
            case 0xb5: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->L=v&~0x40;_WR(a,c->L); }return ticks; //RES 6,(c->IY+d),L
            case 0xb6: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_WR(a,v&~0x40); }return ticks; //RES 6,(c->IY+d)
            case 0xb7: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->A=v&~0x40;_WR(a,c->A); }return ticks; //RES 6,(c->IY+d),A
            case 0xb8: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->B=v&~0x80;_WR(a,c->B); }return ticks; //RES 7,(c->IY+d),B
            case 0xb9: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->C=v&~0x80;_WR(a,c->C); }return ticks; //RES 7,(c->IY+d),C
            case 0xba: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->D=v&~0x80;_WR(a,c->D); }return ticks; //RES 7,(c->IY+d),D
            case 0xbb: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->E=v&~0x80;_WR(a,c->E); }return ticks; //RES 7,(c->IY+d),E
            case 0xbc: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->H=v&~0x80;_WR(a,c->H); }return ticks; //RES 7,(c->IY+d),H
            case 0xbd: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->L=v&~0x80;_WR(a,c->L); }return ticks; //RES 7,(c->IY+d),L
            case 0xbe: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_WR(a,v&~0x80); }return ticks; //RES 7,(c->IY+d)
            case 0xbf: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->A=v&~0x80;_WR(a,c->A); }return ticks; //RES 7,(c->IY+d),A
            case 0xc0: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->B=v|0x1;_WR(a,c->B);}return ticks; //SET 0,(c->IY+d),B
            case 0xc1: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->C=v|0x1;_WR(a,c->C);}return ticks; //SET 0,(c->IY+d),C
            case 0xc2: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->D=v|0x1;_WR(a,c->D);}return ticks; //SET 0,(c->IY+d),D
            case 0xc3: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->E=v|0x1;_WR(a,c->E);}return ticks; //SET 0,(c->IY+d),E
            case 0xc4: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->H=v|0x1;_WR(a,c->IYH);}return ticks; //SET 0,(c->IY+d),H
            case 0xc5: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->L=v|0x1;_WR(a,c->IYL);}return ticks; //SET 0,(c->IY+d),L
            case 0xc6: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_WR(a,v|0x1);}return ticks; //SET 0,(c->IY+d)
            case 0xc7: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->A=v|0x1;_WR(a,c->A);}return ticks; //SET 0,(c->IY+d),A
            case 0xc8: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->B=v|0x2;_WR(a,c->B);}return ticks; //SET 1,(c->IY+d),B
            case 0xc9: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->C=v|0x2;_WR(a,c->C);}return ticks; //SET 1,(c->IY+d),C
            case 0xca: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->D=v|0x2;_WR(a,c->D);}return ticks; //SET 1,(c->IY+d),D
            case 0xcb: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->E=v|0x2;_WR(a,c->E);}return ticks; //SET 1,(c->IY+d),E
            case 0xcc: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->H=v|0x2;_WR(a,c->IYH);}return ticks; //SET 1,(c->IY+d),H
            case 0xcd: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->L=v|0x2;_WR(a,c->IYL);}return ticks; //SET 1,(c->IY+d),L
            case 0xce: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_WR(a,v|0x2);}return ticks; //SET 1,(c->IY+d)
            case 0xcf: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->A=v|0x2;_WR(a,c->A);}return ticks; //SET 1,(c->IY+d),A
            case 0xd0: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->B=v|0x4;_WR(a,c->B);}return ticks; //SET 2,(c->IY+d),B
            case 0xd1: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->C=v|0x4;_WR(a,c->C);}return ticks; //SET 2,(c->IY+d),C
            case 0xd2: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->D=v|0x4;_WR(a,c->D);}return ticks; //SET 2,(c->IY+d),D
            case 0xd3: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->E=v|0x4;_WR(a,c->E);}return ticks; //SET 2,(c->IY+d),E
            case 0xd4: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->H=v|0x4;_WR(a,c->IYH);}return ticks; //SET 2,(c->IY+d),H
            case 0xd5: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->L=v|0x4;_WR(a,c->IYL);}return ticks; //SET 2,(c->IY+d),L
            case 0xd6: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_WR(a,v|0x4);}return ticks; //SET 2,(c->IY+d)
            case 0xd7: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->A=v|0x4;_WR(a,c->A);}return ticks; //SET 2,(c->IY+d),A
            case 0xd8: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->B=v|0x8;_WR(a,c->B);}return ticks; //SET 3,(c->IY+d),B
            case 0xd9: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->C=v|0x8;_WR(a,c->C);}return ticks; //SET 3,(c->IY+d),C
            case 0xda: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->D=v|0x8;_WR(a,c->D);}return ticks; //SET 3,(c->IY+d),D
            case 0xdb: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->E=v|0x8;_WR(a,c->E);}return ticks; //SET 3,(c->IY+d),E
            case 0xdc: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->H=v|0x8;_WR(a,c->IYH);}return ticks; //SET 3,(c->IY+d),H
            case 0xdd: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->L=v|0x8;_WR(a,c->IYL);}return ticks; //SET 3,(c->IY+d),L
            case 0xde: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_WR(a,v|0x8);}return ticks; //SET 3,(c->IY+d)
            case 0xdf: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->A=v|0x8;_WR(a,c->A);}return ticks; //SET 3,(c->IY+d),A
            case 0xe0: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->B=v|0x10;_WR(a,c->B);}return ticks; //SET 4,(c->IY+d),B
            case 0xe1: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->C=v|0x10;_WR(a,c->C);}return ticks; //SET 4,(c->IY+d),C
            case 0xe2: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->D=v|0x10;_WR(a,c->D);}return ticks; //SET 4,(c->IY+d),D
            case 0xe3: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->E=v|0x10;_WR(a,c->E);}return ticks; //SET 4,(c->IY+d),E
            case 0xe4: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->H=v|0x10;_WR(a,c->IYH);}return ticks; //SET 4,(c->IY+d),H
            case 0xe5: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->L=v|0x10;_WR(a,c->IYL);}return ticks; //SET 4,(c->IY+d),L
            case 0xe6: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_WR(a,v|0x10);}return ticks; //SET 4,(c->IY+d)
            case 0xe7: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->A=v|0x10;_WR(a,c->A);}return ticks; //SET 4,(c->IY+d),A
            case 0xe8: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->B=v|0x20;_WR(a,c->B);}return ticks; //SET 5,(c->IY+d),B
            case 0xe9: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->C=v|0x20;_WR(a,c->C);}return ticks; //SET 5,(c->IY+d),C
            case 0xea: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->D=v|0x20;_WR(a,c->D);}return ticks; //SET 5,(c->IY+d),D
            case 0xeb: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->E=v|0x20;_WR(a,c->E);}return ticks; //SET 5,(c->IY+d),E
            case 0xec: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->H=v|0x20;_WR(a,c->IYH);}return ticks; //SET 5,(c->IY+d),H
            case 0xed: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->L=v|0x20;_WR(a,c->IYL);}return ticks; //SET 5,(c->IY+d),L
            case 0xee: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_WR(a,v|0x20);}return ticks; //SET 5,(c->IY+d)
            case 0xef: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->A=v|0x20;_WR(a,c->A);}return ticks; //SET 5,(c->IY+d),A
            case 0xf0: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->B=v|0x40;_WR(a,c->B);}return ticks; //SET 6,(c->IY+d),B
            case 0xf1: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->C=v|0x40;_WR(a,c->C);}return ticks; //SET 6,(c->IY+d),C
            case 0xf2: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->D=v|0x40;_WR(a,c->D);}return ticks; //SET 6,(c->IY+d),D
            case 0xf3: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->E=v|0x40;_WR(a,c->E);}return ticks; //SET 6,(c->IY+d),E
            case 0xf4: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->H=v|0x40;_WR(a,c->IYH);}return ticks; //SET 6,(c->IY+d),H
            case 0xf5: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->L=v|0x40;_WR(a,c->IYL);}return ticks; //SET 6,(c->IY+d),L
            case 0xf6: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_WR(a,v|0x40);}return ticks; //SET 6,(c->IY+d)
            case 0xf7: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->A=v|0x40;_WR(a,c->A);}return ticks; //SET 6,(c->IY+d),A
            case 0xf8: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->B=v|0x80;_WR(a,c->B);}return ticks; //SET 7,(c->IY+d),B
            case 0xf9: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->C=v|0x80;_WR(a,c->C);}return ticks; //SET 7,(c->IY+d),C
            case 0xfa: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->D=v|0x80;_WR(a,c->D);}return ticks; //SET 7,(c->IY+d),D
            case 0xfb: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->E=v|0x80;_WR(a,c->E);}return ticks; //SET 7,(c->IY+d),E
            case 0xfc: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->H=v|0x80;_WR(a,c->IYH);}return ticks; //SET 7,(c->IY+d),H
            case 0xfd: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->L=v|0x80;_WR(a,c->IYL);}return ticks; //SET 7,(c->IY+d),L
            case 0xfe: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);_WR(a,v|0x80);}return ticks; //SET 7,(c->IY+d)
            case 0xff: { a=c->WZ=c->IY+d;;tick(c);ticks++;_T();uint8_t v;_RD(a,v);c->A=v|0x80;_WR(a,c->A);}return ticks; //SET 7,(c->IY+d),A
            default: return ticks;
          }
          break;
        case 0xcc: ticks=_z80_callcc(c,(c->F&Z80_ZF),tick,ticks);return ticks; //CALL Z,nn
        case 0xcd: ticks=_z80_call(c,tick,ticks);return ticks; //CALL nn
        case 0xce: {uint8_t v;_RD(c->PC++,v);_z80_adc(c,v);}return ticks; //ADC n
        case 0xcf: ticks=_z80_rst(c,0x8,tick,ticks);return ticks; //RST 0x8
        case 0xd0: ticks=_z80_retcc(c,!(c->F&Z80_CF),tick,ticks);return ticks; //RET NC
        case 0xd1: /*>rd*/c->ADDR=c->SP++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;l=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*//*>rd*/c->ADDR=c->SP++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;h=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;c->DE=(h<<8)|l;return ticks; //POP DE
        case 0xd2: _IMM16(); if (!(c->F&Z80_CF)) { c->PC=c->WZ; }return ticks; //JP NC,nn
        case 0xd3: {uint8_t v;_RD(c->PC++,v);c->WZ=((c->A<<8)|v);_OUT(c->WZ,c->A);c->Z++;}return ticks; //OUT (n),A
        case 0xd4: ticks=_z80_callcc(c,!(c->F&Z80_CF),tick,ticks);return ticks; //CALL NC,nn
        case 0xd5: _T();_WR(--c->SP,(uint8_t)(c->DE>>8)); _WR(--c->SP,(uint8_t)c->DE);return ticks; //PUSH DE
        case 0xd6: {uint8_t v;_RD(c->PC++,v);_z80_sub(c,v);}return ticks; //SUB n
        case 0xd7: ticks=_z80_rst(c,0x10,tick,ticks);return ticks; //RST 0x10
        case 0xd8: ticks=_z80_retcc(c,(c->F&Z80_CF),tick,ticks);return ticks; //RET C
        case 0xd9: _SWP16(c->BC,c->BC_);_SWP16(c->DE,c->DE_);_SWP16(c->HL,c->HL_);return ticks; //EXX
        case 0xda: _IMM16(); if ((c->F&Z80_CF)) { c->PC=c->WZ; }return ticks; //JP C,nn
        case 0xdb: {uint8_t v;_RD(c->PC++,v);c->WZ=((c->A<<8)|v);_IN(c->WZ++,c->A);}return ticks; //IN A,(n)
        case 0xdc: ticks=_z80_callcc(c,(c->F&Z80_CF),tick,ticks);return ticks; //CALL C,nn
        case 0xde: {uint8_t v;_RD(c->PC++,v);_z80_sbc(c,v);}return ticks; //SBC n
        case 0xdf: ticks=_z80_rst(c,0x18,tick,ticks);return ticks; //RST 0x18
        case 0xe0: ticks=_z80_retcc(c,!(c->F&Z80_PF),tick,ticks);return ticks; //RET PO
        case 0xe1: /*>rd*/c->ADDR=c->SP++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;l=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*//*>rd*/c->ADDR=c->SP++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;h=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;c->IY=(h<<8)|l;return ticks; //POP IY
        case 0xe2: _IMM16(); if (!(c->F&Z80_PF)) { c->PC=c->WZ; }return ticks; //JP PO,nn
        case 0xe3: _T();_RD(c->SP,c->Z);_RD(c->SP+1,c->W);_WR(c->SP,(uint8_t)c->IY);_WR(c->SP+1,(uint8_t)(c->IY>>8));c->IY=c->WZ;_T();_T();return ticks; //EX (SP),IY
        case 0xe4: ticks=_z80_callcc(c,!(c->F&Z80_PF),tick,ticks);return ticks; //CALL PO,nn
        case 0xe5: _T();_WR(--c->SP,(uint8_t)(c->IY>>8)); _WR(--c->SP,(uint8_t)c->IY);return ticks; //PUSH IY
        case 0xe6: {uint8_t v;_RD(c->PC++,v);_z80_and(c,v);}return ticks; //AND n
        case 0xe7: ticks=_z80_rst(c,0x20,tick,ticks);return ticks; //RST 0x20
        case 0xe8: ticks=_z80_retcc(c,(c->F&Z80_PF),tick,ticks);return ticks; //RET PE
        case 0xe9: c->PC=c->IY;return ticks; //JP IY
        case 0xea: _IMM16(); if ((c->F&Z80_PF)) { c->PC=c->WZ; }return ticks; //JP PE,nn
        case 0xeb: _SWP16(c->DE,c->HL);return ticks; //EX DE,HL
        case 0xec: ticks=_z80_callcc(c,(c->F&Z80_PF),tick,ticks);return ticks; //CALL PE,nn
        case 0xee: {uint8_t v;_RD(c->PC++,v);_z80_xor(c,v);}return ticks; //XOR n
        case 0xef: ticks=_z80_rst(c,0x28,tick,ticks);return ticks; //RST 0x28
        case 0xf0: ticks=_z80_retcc(c,!(c->F&Z80_SF),tick,ticks);return ticks; //RET P
        case 0xf1: /*>rd*/c->ADDR=c->SP++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;l=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*//*>rd*/c->ADDR=c->SP++;tick(c);ticks++;c->CTRL|=Z80_MREQ|Z80_RD;tick(c);ticks++;h=c->DATA;c->CTRL&=~(Z80_MREQ|Z80_RD);tick(c);ticks++;/*<rd*/;c->AF=(h<<8)|l;return ticks; //POP AF
        case 0xf2: _IMM16(); if (!(c->F&Z80_SF)) { c->PC=c->WZ; }return ticks; //JP P,nn
        case 0xf3: _z80_di(c);return ticks; //DI
        case 0xf4: ticks=_z80_callcc(c,!(c->F&Z80_SF),tick,ticks);return ticks; //CALL P,nn
        case 0xf5: _T();_WR(--c->SP,(uint8_t)(c->AF>>8)); _WR(--c->SP,(uint8_t)c->AF);return ticks; //PUSH AF
        case 0xf6: {uint8_t v;_RD(c->PC++,v);_z80_or(c,v);}return ticks; //OR n
        case 0xf7: ticks=_z80_rst(c,0x30,tick,ticks);return ticks; //RST 0x30
        case 0xf8: ticks=_z80_retcc(c,(c->F&Z80_SF),tick,ticks);return ticks; //RET M
        case 0xf9: tick(c);ticks++;tick(c);ticks++;c->SP=c->IY;return ticks; //LD SP,IY
        case 0xfa: _IMM16(); if ((c->F&Z80_SF)) { c->PC=c->WZ; }return ticks; //JP M,nn
        case 0xfb: _z80_ei(c);return ticks; //EI
        case 0xfc: ticks=_z80_callcc(c,(c->F&Z80_SF),tick,ticks);return ticks; //CALL M,nn
        case 0xfe: {uint8_t v;_RD(c->PC++,v);_z80_cp(c,v);}return ticks; //CP n
        case 0xff: ticks=_z80_rst(c,0x38,tick,ticks);return ticks; //RST 0x38
        default: return ticks;
      }
      break;
    case 0xfe: {uint8_t v;_RD(c->PC++,v);_z80_cp(c,v);}return ticks; //CP n
    case 0xff: ticks=_z80_rst(c,0x38,tick,ticks);return ticks; //RST 0x38
    default: return ticks;
  }
  return ticks;
}
