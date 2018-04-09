/* machine generated, do not edit! */
static uint8_t _z80_szp[256] = {
  0x44,0x00,0x00,0x04,0x00,0x04,0x04,0x00,0x08,0x0c,0x0c,0x08,0x0c,0x08,0x08,0x0c,
  0x00,0x04,0x04,0x00,0x04,0x00,0x00,0x04,0x0c,0x08,0x08,0x0c,0x08,0x0c,0x0c,0x08,
  0x20,0x24,0x24,0x20,0x24,0x20,0x20,0x24,0x2c,0x28,0x28,0x2c,0x28,0x2c,0x2c,0x28,
  0x24,0x20,0x20,0x24,0x20,0x24,0x24,0x20,0x28,0x2c,0x2c,0x28,0x2c,0x28,0x28,0x2c,
  0x00,0x04,0x04,0x00,0x04,0x00,0x00,0x04,0x0c,0x08,0x08,0x0c,0x08,0x0c,0x0c,0x08,
  0x04,0x00,0x00,0x04,0x00,0x04,0x04,0x00,0x08,0x0c,0x0c,0x08,0x0c,0x08,0x08,0x0c,
  0x24,0x20,0x20,0x24,0x20,0x24,0x24,0x20,0x28,0x2c,0x2c,0x28,0x2c,0x28,0x28,0x2c,
  0x20,0x24,0x24,0x20,0x24,0x20,0x20,0x24,0x2c,0x28,0x28,0x2c,0x28,0x2c,0x2c,0x28,
  0x80,0x84,0x84,0x80,0x84,0x80,0x80,0x84,0x8c,0x88,0x88,0x8c,0x88,0x8c,0x8c,0x88,
  0x84,0x80,0x80,0x84,0x80,0x84,0x84,0x80,0x88,0x8c,0x8c,0x88,0x8c,0x88,0x88,0x8c,
  0xa4,0xa0,0xa0,0xa4,0xa0,0xa4,0xa4,0xa0,0xa8,0xac,0xac,0xa8,0xac,0xa8,0xa8,0xac,
  0xa0,0xa4,0xa4,0xa0,0xa4,0xa0,0xa0,0xa4,0xac,0xa8,0xa8,0xac,0xa8,0xac,0xac,0xa8,
  0x84,0x80,0x80,0x84,0x80,0x84,0x84,0x80,0x88,0x8c,0x8c,0x88,0x8c,0x88,0x88,0x8c,
  0x80,0x84,0x84,0x80,0x84,0x80,0x80,0x84,0x8c,0x88,0x88,0x8c,0x88,0x8c,0x8c,0x88,
  0xa0,0xa4,0xa4,0xa0,0xa4,0xa0,0xa0,0xa4,0xac,0xa8,0xa8,0xac,0xa8,0xac,0xac,0xa8,
  0xa4,0xa0,0xa0,0xa4,0xa0,0xa4,0xa4,0xa0,0xa8,0xac,0xac,0xa8,0xac,0xa8,0xa8,0xac,
};
/* set 16-bit address in 64-bit pin mask*/
#define _SA(addr) pins=(pins&~0xFFFF)|((addr)&0xFFFFULL)
/* set 16-bit address and 8-bit data in 64-bit pin mask */
#define _SAD(addr,data) pins=(pins&~0xFFFFFF)|((((data)&0xFF)<<16)&0xFF0000ULL)|((addr)&0xFFFFULL)
/* extract 8-bit data from 64-bit pin mask */
#define _GD() ((uint8_t)((pins&0xFF0000ULL)>>16))
/* enable control pins */
#define _ON(m) pins|=(m)
/* disable control pins */
#define _OFF(m) pins&=~(m)
/* execute a number of ticks without wait-state detection */
#define _T(num) pins=tick(num,pins);ticks+=num
/* execute a number of ticks with wait-state detection */
#define _TW(num) pins&=~Z80_WAIT_MASK;pins=tick(num,pins);ticks+=num+Z80_GET_WAIT(pins);
/* a memory read machine cycle (3 ticks with wait-state detection) */
#define _MR(addr,data) _SA(addr);_ON(Z80_MREQ|Z80_RD);_TW(3);_OFF(Z80_MREQ|Z80_RD);data=_GD()
/* a memory write machine cycle (3 ticks with wait-state detection) */
#define _MW(addr,data) _SAD(addr,data);_ON(Z80_MREQ|Z80_WR);_TW(3);_OFF(Z80_MREQ|Z80_WR)
/* an input machine cycle (4 ticks with wait-state detection) */
#define _IN(addr,data) _SA(addr);_ON(Z80_IORQ|Z80_RD);_TW(4);_OFF(Z80_IORQ|Z80_RD);data=_GD()
/* an output machine cycle (4 ticks with wait-state detection) */
#define _OUT(addr,data) _SAD(addr,data);_ON(Z80_IORQ|Z80_WR);_TW(4);_OFF(Z80_IORQ|Z80_WR)
/* an opcode fetch machine cycle (4 ticks with wait-state detection, no refresh cycle emulated, bump R) */
#define _FETCH(op) _ON(Z80_M1|Z80_MREQ|Z80_RD);_SA(c.PC++);_TW(4);_OFF(Z80_M1|Z80_MREQ|Z80_RD);op=_GD();c.R=(c.R&0x80)|((c.R+1)&0x7F)
/* a special opcode fetch for DD/FD+CB instructions without incrementing R */
#define _FETCH_CB(op) _ON(Z80_M1|Z80_MREQ|Z80_RD);_SA(c.PC++);_TW(4);_OFF(Z80_M1|Z80_MREQ|Z80_RD);op=_GD()
/* a 16-bit immediate load from (PC) into WZ */
#define _IMM16() {uint8_t w,z;_MR(c.PC++,z);_MR(c.PC++,w);c.WZ=(w<<8)|z;}
/* evaluate the S and Z flags */
#define _SZ(val) ((val&0xFF)?(val&Z80_SF):Z80_ZF)
/* evaluate the S,Z,Y,X,C and H flags */
#define _SZYXCH(acc,val,res) (_SZ(res)|(res&(Z80_YF|Z80_XF))|((res>>8)&Z80_CF)|((acc^val^res)&Z80_HF))
/* evaluate flags for ADD and ADC */
#define _ADD_FLAGS(acc,val,res) (_SZYXCH(acc,val,res)|((((val^acc^0x80)&(val^res))>>5)&Z80_VF))
/* evaluate flags for SUB and SBC */
#define _SUB_FLAGS(acc,val,res) (Z80_NF|_SZYXCH(acc,val,res)|((((val^acc)&(res^acc))>>5)&Z80_VF))
/* evaluate flags for CP */
#define _CP_FLAGS(acc,val,res) (Z80_NF|(_SZ(res)|(val&(Z80_YF|Z80_XF))|((res>>8)&Z80_CF)|((acc^val^res)&Z80_HF))|((((val^acc)&(res^acc))>>5)&Z80_VF))

uint32_t z80_exec(z80_t* cpu, uint32_t num_ticks) {
  z80_state_t c = cpu->state;
  const z80_tick_t tick = cpu->tick;
  uint64_t pins = cpu->pins;
  int trap_id = -1;
  uint32_t ticks = 0;
  uint8_t opcode; uint16_t a; uint8_t v; uint8_t f;
  do {
    _OFF(Z80_INT);
    if (c.ei_pending) { c.IFF1=c.IFF2=true; c.ei_pending=false; }
    {
    _FETCH(opcode);
    switch (opcode) {
      case 0x0:/*NOP*/ break;
      case 0x1:/*LD BC,nn*/_IMM16();c.BC=c.WZ;break;
      case 0x2:/*LD (BC),A*/c.WZ=c.BC;_MW(c.WZ++,c.A);;c.WZ=(c.WZ&0x00FF)|(c.A<<8);break;
      case 0x3:/*INC BC*/_T(2);c.BC++;break;
      case 0x4:/*INC B*/{uint8_t r=c.B+1;f=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^c.B)&Z80_HF);if(r==0x80){f|=Z80_VF;}c.F=f|(c.F&Z80_CF);c.B=r;}break;
      case 0x5:/*DEC B*/{uint8_t r=c.B-1;f=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^c.B)&Z80_HF);if(r==0x7F){f|=Z80_VF;}c.F=f|(c.F&Z80_CF);c.B=r;}break;
      case 0x6:/*LD B,n*/_MR(c.PC++,c.B);break;
      case 0x7:/*RLCA*/{uint8_t r=c.A<<1|c.A>>7;c.F=(c.A>>7&Z80_CF)|(c.F&(Z80_SF|Z80_ZF|Z80_PF))|(r&(Z80_XF|Z80_YF));c.A=r;}break;
      case 0x8:/*EX AF,AF'*/{uint16_t tmp=c.AF;c.AF=c.AF_;c.AF_=tmp;}break;
      case 0x9:/*ADD HL,BC*/{c.WZ=c.HL+1;uint32_t r=c.HL+c.BC;c.F=(c.F&(Z80_SF|Z80_ZF|Z80_VF))|(((c.HL^r^c.BC)>>8)&Z80_HF)|((r>>16)&Z80_CF)|((r>>8)&(Z80_YF|Z80_XF));c.HL=r;}_T(7);break;
      case 0xa:/*LD A,(BC)*/c.WZ=c.BC;_MR(c.WZ++,c.A);;break;
      case 0xb:/*DEC BC*/_T(2);c.BC--;break;
      case 0xc:/*INC C*/{uint8_t r=c.C+1;f=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^c.C)&Z80_HF);if(r==0x80){f|=Z80_VF;}c.F=f|(c.F&Z80_CF);c.C=r;}break;
      case 0xd:/*DEC C*/{uint8_t r=c.C-1;f=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^c.C)&Z80_HF);if(r==0x7F){f|=Z80_VF;}c.F=f|(c.F&Z80_CF);c.C=r;}break;
      case 0xe:/*LD C,n*/_MR(c.PC++,c.C);break;
      case 0xf:/*RRCA*/{uint8_t r=c.A>>1|c.A<<7;c.F=(c.A&Z80_CF)|(c.F&(Z80_SF|Z80_ZF|Z80_PF))|(r&(Z80_YF|Z80_XF));c.A=r;}break;
      case 0x10:/*DJNZ*/_T(1);{int8_t d;_MR(c.PC++,d);if(--c.B>0){c.WZ=c.PC=c.PC+d;_T(5);}}break;
      case 0x11:/*LD DE,nn*/_IMM16();c.DE=c.WZ;break;
      case 0x12:/*LD (DE),A*/c.WZ=c.DE;_MW(c.WZ++,c.A);;c.WZ=(c.WZ&0x00FF)|(c.A<<8);break;
      case 0x13:/*INC DE*/_T(2);c.DE++;break;
      case 0x14:/*INC D*/{uint8_t r=c.D+1;f=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^c.D)&Z80_HF);if(r==0x80){f|=Z80_VF;}c.F=f|(c.F&Z80_CF);c.D=r;}break;
      case 0x15:/*DEC D*/{uint8_t r=c.D-1;f=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^c.D)&Z80_HF);if(r==0x7F){f|=Z80_VF;}c.F=f|(c.F&Z80_CF);c.D=r;}break;
      case 0x16:/*LD D,n*/_MR(c.PC++,c.D);break;
      case 0x17:/*RLA*/{uint8_t r=c.A<<1|(c.F&Z80_CF);c.F=(c.A>>7&Z80_CF)|(c.F&(Z80_SF|Z80_ZF|Z80_PF))|(r&(Z80_YF|Z80_XF));c.A=r;}break;
      case 0x18:/*JR d*/{int8_t d;_MR(c.PC++,d);c.WZ=c.PC=c.PC+d;}_T(5);break;
      case 0x19:/*ADD HL,DE*/{c.WZ=c.HL+1;uint32_t r=c.HL+c.DE;c.F=(c.F&(Z80_SF|Z80_ZF|Z80_VF))|(((c.HL^r^c.DE)>>8)&Z80_HF)|((r>>16)&Z80_CF)|((r>>8)&(Z80_YF|Z80_XF));c.HL=r;}_T(7);break;
      case 0x1a:/*LD A,(DE)*/c.WZ=c.DE;_MR(c.WZ++,c.A);;break;
      case 0x1b:/*DEC DE*/_T(2);c.DE--;break;
      case 0x1c:/*INC E*/{uint8_t r=c.E+1;f=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^c.E)&Z80_HF);if(r==0x80){f|=Z80_VF;}c.F=f|(c.F&Z80_CF);c.E=r;}break;
      case 0x1d:/*DEC E*/{uint8_t r=c.E-1;f=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^c.E)&Z80_HF);if(r==0x7F){f|=Z80_VF;}c.F=f|(c.F&Z80_CF);c.E=r;}break;
      case 0x1e:/*LD E,n*/_MR(c.PC++,c.E);break;
      case 0x1f:/*RRA*/{uint8_t r=c.A>>1|((c.F&Z80_CF)<<7);c.F=(c.A&Z80_CF)|(c.F&(Z80_SF|Z80_ZF|Z80_PF))|(r&(Z80_YF|Z80_XF));c.A=r;}break;
      case 0x20:/*JR NZ,d*/{int8_t d;_MR(c.PC++,d);if(!(c.F&Z80_ZF)){c.WZ=c.PC=c.PC+d;_T(5);}}break;
      case 0x21:/*LD HL,nn*/_IMM16();c.HL=c.WZ;break;
      case 0x22:/*LD (nn),HL*/_IMM16();_MW(c.WZ++,(uint8_t)c.HL);_MW(c.WZ,(uint8_t)(c.HL>>8));break;
      case 0x23:/*INC HL*/_T(2);c.HL++;break;
      case 0x24:/*INC H*/{uint8_t r=c.H+1;f=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^c.H)&Z80_HF);if(r==0x80){f|=Z80_VF;}c.F=f|(c.F&Z80_CF);c.H=r;}break;
      case 0x25:/*DEC H*/{uint8_t r=c.H-1;f=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^c.H)&Z80_HF);if(r==0x7F){f|=Z80_VF;}c.F=f|(c.F&Z80_CF);c.H=r;}break;
      case 0x26:/*LD H,n*/_MR(c.PC++,c.H);break;
      case 0x27:/*DAA*/v=c.A;if(c.F&Z80_NF){if(((c.A&0xF)>0x9)||(c.F&Z80_HF)){v-=0x06;}if((c.A>0x99)||(c.F&Z80_CF)){v-=0x60;}}else{if(((c.A&0xF)>0x9)||(c.F&Z80_HF)){v+=0x06;}if((c.A>0x99)||(c.F&Z80_CF)){v+=0x60;}}c.F&=Z80_CF|Z80_NF;c.F|=(c.A>0x99)?Z80_CF:0;c.F|=(c.A^v)&Z80_HF;c.F|=_z80_szp[v];c.A=v;break;
      case 0x28:/*JR Z,d*/{int8_t d;_MR(c.PC++,d);if((c.F&Z80_ZF)){c.WZ=c.PC=c.PC+d;_T(5);}}break;
      case 0x29:/*ADD HL,HL*/{c.WZ=c.HL+1;uint32_t r=c.HL+c.HL;c.F=(c.F&(Z80_SF|Z80_ZF|Z80_VF))|(((c.HL^r^c.HL)>>8)&Z80_HF)|((r>>16)&Z80_CF)|((r>>8)&(Z80_YF|Z80_XF));c.HL=r;}_T(7);break;
      case 0x2a:/*LD HL,(nn)*/_IMM16();{uint8_t l,h;_MR(c.WZ++,l);_MR(c.WZ,h);c.HL=(h<<8)|l;}break;
      case 0x2b:/*DEC HL*/_T(2);c.HL--;break;
      case 0x2c:/*INC L*/{uint8_t r=c.L+1;f=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^c.L)&Z80_HF);if(r==0x80){f|=Z80_VF;}c.F=f|(c.F&Z80_CF);c.L=r;}break;
      case 0x2d:/*DEC L*/{uint8_t r=c.L-1;f=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^c.L)&Z80_HF);if(r==0x7F){f|=Z80_VF;}c.F=f|(c.F&Z80_CF);c.L=r;}break;
      case 0x2e:/*LD L,n*/_MR(c.PC++,c.L);break;
      case 0x2f:/*CPL*/c.A^=0xFF;c.F=(c.F&(Z80_SF|Z80_ZF|Z80_PF|Z80_CF))|Z80_HF|Z80_NF|(c.A&(Z80_YF|Z80_XF));break;
      case 0x30:/*JR NC,d*/{int8_t d;_MR(c.PC++,d);if(!(c.F&Z80_CF)){c.WZ=c.PC=c.PC+d;_T(5);}}break;
      case 0x31:/*LD SP,nn*/_IMM16();c.SP=c.WZ;break;
      case 0x32:/*LD (nn),A*/_IMM16();_MW(c.WZ++,c.A);;c.WZ=(c.WZ&0x00FF)|(c.A<<8);break;
      case 0x33:/*INC SP*/_T(2);c.SP++;break;
      case 0x34:/*INC (HL)*/a=c.HL;_T(1);_MR(a,v);{uint8_t r=v+1;f=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^v)&Z80_HF);if(r==0x80){f|=Z80_VF;}c.F=f|(c.F&Z80_CF);v=r;}_MW(a,v);break;
      case 0x35:/*DEC (HL)*/a=c.HL;_T(1);_MR(a,v);{uint8_t r=v-1;f=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^v)&Z80_HF);if(r==0x7F){f|=Z80_VF;}c.F=f|(c.F&Z80_CF);v=r;}_MW(a,v);break;
      case 0x36:/*LD (HL),n*/a=c.HL;_MR(c.PC++,v);_MW(a,v);break;
      case 0x37:/*SCF*/c.F=(c.F&(Z80_SF|Z80_ZF|Z80_YF|Z80_XF|Z80_PF))|Z80_CF|(c.A&(Z80_YF|Z80_XF));break;
      case 0x38:/*JR C,d*/{int8_t d;_MR(c.PC++,d);if((c.F&Z80_CF)){c.WZ=c.PC=c.PC+d;_T(5);}}break;
      case 0x39:/*ADD HL,SP*/{c.WZ=c.HL+1;uint32_t r=c.HL+c.SP;c.F=(c.F&(Z80_SF|Z80_ZF|Z80_VF))|(((c.HL^r^c.SP)>>8)&Z80_HF)|((r>>16)&Z80_CF)|((r>>8)&(Z80_YF|Z80_XF));c.HL=r;}_T(7);break;
      case 0x3a:/*LD A,(nn)*/_IMM16();_MR(c.WZ++,c.A);;break;
      case 0x3b:/*DEC SP*/_T(2);c.SP--;break;
      case 0x3c:/*INC A*/{uint8_t r=c.A+1;f=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^c.A)&Z80_HF);if(r==0x80){f|=Z80_VF;}c.F=f|(c.F&Z80_CF);c.A=r;}break;
      case 0x3d:/*DEC A*/{uint8_t r=c.A-1;f=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^c.A)&Z80_HF);if(r==0x7F){f|=Z80_VF;}c.F=f|(c.F&Z80_CF);c.A=r;}break;
      case 0x3e:/*LD A,n*/_MR(c.PC++,c.A);break;
      case 0x3f:/*CCF*/c.F=((c.F&(Z80_SF|Z80_ZF|Z80_YF|Z80_XF|Z80_PF|Z80_CF))|((c.F&Z80_CF)<<4)|(c.A&(Z80_YF|Z80_XF)))^Z80_CF;break;
      case 0x40:/*LD B,B*/c.B=c.B;break;
      case 0x41:/*LD B,C*/c.B=c.C;break;
      case 0x42:/*LD B,D*/c.B=c.D;break;
      case 0x43:/*LD B,E*/c.B=c.E;break;
      case 0x44:/*LD B,H*/c.B=c.H;break;
      case 0x45:/*LD B,L*/c.B=c.L;break;
      case 0x46:/*LD B,(HL)*/a=c.HL;_MR(a,c.B);break;
      case 0x47:/*LD B,A*/c.B=c.A;break;
      case 0x48:/*LD C,B*/c.C=c.B;break;
      case 0x49:/*LD C,C*/c.C=c.C;break;
      case 0x4a:/*LD C,D*/c.C=c.D;break;
      case 0x4b:/*LD C,E*/c.C=c.E;break;
      case 0x4c:/*LD C,H*/c.C=c.H;break;
      case 0x4d:/*LD C,L*/c.C=c.L;break;
      case 0x4e:/*LD C,(HL)*/a=c.HL;_MR(a,c.C);break;
      case 0x4f:/*LD C,A*/c.C=c.A;break;
      case 0x50:/*LD D,B*/c.D=c.B;break;
      case 0x51:/*LD D,C*/c.D=c.C;break;
      case 0x52:/*LD D,D*/c.D=c.D;break;
      case 0x53:/*LD D,E*/c.D=c.E;break;
      case 0x54:/*LD D,H*/c.D=c.H;break;
      case 0x55:/*LD D,L*/c.D=c.L;break;
      case 0x56:/*LD D,(HL)*/a=c.HL;_MR(a,c.D);break;
      case 0x57:/*LD D,A*/c.D=c.A;break;
      case 0x58:/*LD E,B*/c.E=c.B;break;
      case 0x59:/*LD E,C*/c.E=c.C;break;
      case 0x5a:/*LD E,D*/c.E=c.D;break;
      case 0x5b:/*LD E,E*/c.E=c.E;break;
      case 0x5c:/*LD E,H*/c.E=c.H;break;
      case 0x5d:/*LD E,L*/c.E=c.L;break;
      case 0x5e:/*LD E,(HL)*/a=c.HL;_MR(a,c.E);break;
      case 0x5f:/*LD E,A*/c.E=c.A;break;
      case 0x60:/*LD H,B*/c.H=c.B;break;
      case 0x61:/*LD H,C*/c.H=c.C;break;
      case 0x62:/*LD H,D*/c.H=c.D;break;
      case 0x63:/*LD H,E*/c.H=c.E;break;
      case 0x64:/*LD H,H*/c.H=c.H;break;
      case 0x65:/*LD H,L*/c.H=c.L;break;
      case 0x66:/*LD H,(HL)*/a=c.HL;_MR(a,c.H);break;
      case 0x67:/*LD H,A*/c.H=c.A;break;
      case 0x68:/*LD L,B*/c.L=c.B;break;
      case 0x69:/*LD L,C*/c.L=c.C;break;
      case 0x6a:/*LD L,D*/c.L=c.D;break;
      case 0x6b:/*LD L,E*/c.L=c.E;break;
      case 0x6c:/*LD L,H*/c.L=c.H;break;
      case 0x6d:/*LD L,L*/c.L=c.L;break;
      case 0x6e:/*LD L,(HL)*/a=c.HL;_MR(a,c.L);break;
      case 0x6f:/*LD L,A*/c.L=c.A;break;
      case 0x70:/*LD (HL),B*/a=c.HL;_MW(a,c.B);break;
      case 0x71:/*LD (HL),C*/a=c.HL;_MW(a,c.C);break;
      case 0x72:/*LD (HL),D*/a=c.HL;_MW(a,c.D);break;
      case 0x73:/*LD (HL),E*/a=c.HL;_MW(a,c.E);break;
      case 0x74:/*LD (HL),H*/a=c.HL;_MW(a,c.H);break;
      case 0x75:/*LD (HL),L*/a=c.HL;_MW(a,c.L);break;
      case 0x76:/*HALT*/_ON(Z80_HALT);c.PC--;break;
      case 0x77:/*LD (HL),A*/a=c.HL;_MW(a,c.A);break;
      case 0x78:/*LD A,B*/c.A=c.B;break;
      case 0x79:/*LD A,C*/c.A=c.C;break;
      case 0x7a:/*LD A,D*/c.A=c.D;break;
      case 0x7b:/*LD A,E*/c.A=c.E;break;
      case 0x7c:/*LD A,H*/c.A=c.H;break;
      case 0x7d:/*LD A,L*/c.A=c.L;break;
      case 0x7e:/*LD A,(HL)*/a=c.HL;_MR(a,c.A);break;
      case 0x7f:/*LD A,A*/c.A=c.A;break;
      case 0x80:/*ADD B*/{int res=c.A+c.B;c.F=_ADD_FLAGS(c.A,c.B,res);c.A=(uint8_t)res;}break;
      case 0x81:/*ADD C*/{int res=c.A+c.C;c.F=_ADD_FLAGS(c.A,c.C,res);c.A=(uint8_t)res;}break;
      case 0x82:/*ADD D*/{int res=c.A+c.D;c.F=_ADD_FLAGS(c.A,c.D,res);c.A=(uint8_t)res;}break;
      case 0x83:/*ADD E*/{int res=c.A+c.E;c.F=_ADD_FLAGS(c.A,c.E,res);c.A=(uint8_t)res;}break;
      case 0x84:/*ADD H*/{int res=c.A+c.H;c.F=_ADD_FLAGS(c.A,c.H,res);c.A=(uint8_t)res;}break;
      case 0x85:/*ADD L*/{int res=c.A+c.L;c.F=_ADD_FLAGS(c.A,c.L,res);c.A=(uint8_t)res;}break;
      case 0x86:/*ADD (HL)*/a=c.HL;_MR(a,v);{int res=c.A+v;c.F=_ADD_FLAGS(c.A,v,res);c.A=(uint8_t)res;}break;
      case 0x87:/*ADD A*/{int res=c.A+c.A;c.F=_ADD_FLAGS(c.A,c.A,res);c.A=(uint8_t)res;}break;
      case 0x88:/*ADC B*/{int res=c.A+c.B+(c.F&Z80_CF);c.F=_ADD_FLAGS(c.A,c.B,res);c.A=(uint8_t)res;}break;
      case 0x89:/*ADC C*/{int res=c.A+c.C+(c.F&Z80_CF);c.F=_ADD_FLAGS(c.A,c.C,res);c.A=(uint8_t)res;}break;
      case 0x8a:/*ADC D*/{int res=c.A+c.D+(c.F&Z80_CF);c.F=_ADD_FLAGS(c.A,c.D,res);c.A=(uint8_t)res;}break;
      case 0x8b:/*ADC E*/{int res=c.A+c.E+(c.F&Z80_CF);c.F=_ADD_FLAGS(c.A,c.E,res);c.A=(uint8_t)res;}break;
      case 0x8c:/*ADC H*/{int res=c.A+c.H+(c.F&Z80_CF);c.F=_ADD_FLAGS(c.A,c.H,res);c.A=(uint8_t)res;}break;
      case 0x8d:/*ADC L*/{int res=c.A+c.L+(c.F&Z80_CF);c.F=_ADD_FLAGS(c.A,c.L,res);c.A=(uint8_t)res;}break;
      case 0x8e:/*ADC (HL)*/a=c.HL;_MR(a,v);{int res=c.A+v+(c.F&Z80_CF);c.F=_ADD_FLAGS(c.A,v,res);c.A=(uint8_t)res;}break;
      case 0x8f:/*ADC A*/{int res=c.A+c.A+(c.F&Z80_CF);c.F=_ADD_FLAGS(c.A,c.A,res);c.A=(uint8_t)res;}break;
      case 0x90:/*SUB B*/{int res=(int)c.A-(int)c.B;c.F=_SUB_FLAGS(c.A,c.B,res);c.A=(uint8_t)res;}break;
      case 0x91:/*SUB C*/{int res=(int)c.A-(int)c.C;c.F=_SUB_FLAGS(c.A,c.C,res);c.A=(uint8_t)res;}break;
      case 0x92:/*SUB D*/{int res=(int)c.A-(int)c.D;c.F=_SUB_FLAGS(c.A,c.D,res);c.A=(uint8_t)res;}break;
      case 0x93:/*SUB E*/{int res=(int)c.A-(int)c.E;c.F=_SUB_FLAGS(c.A,c.E,res);c.A=(uint8_t)res;}break;
      case 0x94:/*SUB H*/{int res=(int)c.A-(int)c.H;c.F=_SUB_FLAGS(c.A,c.H,res);c.A=(uint8_t)res;}break;
      case 0x95:/*SUB L*/{int res=(int)c.A-(int)c.L;c.F=_SUB_FLAGS(c.A,c.L,res);c.A=(uint8_t)res;}break;
      case 0x96:/*SUB (HL)*/a=c.HL;_MR(a,v);{int res=(int)c.A-(int)v;c.F=_SUB_FLAGS(c.A,v,res);c.A=(uint8_t)res;}break;
      case 0x97:/*SUB A*/{int res=(int)c.A-(int)c.A;c.F=_SUB_FLAGS(c.A,c.A,res);c.A=(uint8_t)res;}break;
      case 0x98:/*SBC B*/{int res=(int)c.A-(int)c.B-(c.F&Z80_CF);c.F=_SUB_FLAGS(c.A,c.B,res);c.A=(uint8_t)res;}break;
      case 0x99:/*SBC C*/{int res=(int)c.A-(int)c.C-(c.F&Z80_CF);c.F=_SUB_FLAGS(c.A,c.C,res);c.A=(uint8_t)res;}break;
      case 0x9a:/*SBC D*/{int res=(int)c.A-(int)c.D-(c.F&Z80_CF);c.F=_SUB_FLAGS(c.A,c.D,res);c.A=(uint8_t)res;}break;
      case 0x9b:/*SBC E*/{int res=(int)c.A-(int)c.E-(c.F&Z80_CF);c.F=_SUB_FLAGS(c.A,c.E,res);c.A=(uint8_t)res;}break;
      case 0x9c:/*SBC H*/{int res=(int)c.A-(int)c.H-(c.F&Z80_CF);c.F=_SUB_FLAGS(c.A,c.H,res);c.A=(uint8_t)res;}break;
      case 0x9d:/*SBC L*/{int res=(int)c.A-(int)c.L-(c.F&Z80_CF);c.F=_SUB_FLAGS(c.A,c.L,res);c.A=(uint8_t)res;}break;
      case 0x9e:/*SBC (HL)*/a=c.HL;_MR(a,v);{int res=(int)c.A-(int)v-(c.F&Z80_CF);c.F=_SUB_FLAGS(c.A,v,res);c.A=(uint8_t)res;}break;
      case 0x9f:/*SBC A*/{int res=(int)c.A-(int)c.A-(c.F&Z80_CF);c.F=_SUB_FLAGS(c.A,c.A,res);c.A=(uint8_t)res;}break;
      case 0xa0:/*AND B*/c.A&=c.B;c.F=_z80_szp[c.A]|Z80_HF;break;
      case 0xa1:/*AND C*/c.A&=c.C;c.F=_z80_szp[c.A]|Z80_HF;break;
      case 0xa2:/*AND D*/c.A&=c.D;c.F=_z80_szp[c.A]|Z80_HF;break;
      case 0xa3:/*AND E*/c.A&=c.E;c.F=_z80_szp[c.A]|Z80_HF;break;
      case 0xa4:/*AND H*/c.A&=c.H;c.F=_z80_szp[c.A]|Z80_HF;break;
      case 0xa5:/*AND L*/c.A&=c.L;c.F=_z80_szp[c.A]|Z80_HF;break;
      case 0xa6:/*AND (HL)*/a=c.HL;_MR(a,v);c.A&=v;c.F=_z80_szp[c.A]|Z80_HF;break;
      case 0xa7:/*AND A*/c.A&=c.A;c.F=_z80_szp[c.A]|Z80_HF;break;
      case 0xa8:/*XOR B*/c.A^=c.B;c.F=_z80_szp[c.A];break;
      case 0xa9:/*XOR C*/c.A^=c.C;c.F=_z80_szp[c.A];break;
      case 0xaa:/*XOR D*/c.A^=c.D;c.F=_z80_szp[c.A];break;
      case 0xab:/*XOR E*/c.A^=c.E;c.F=_z80_szp[c.A];break;
      case 0xac:/*XOR H*/c.A^=c.H;c.F=_z80_szp[c.A];break;
      case 0xad:/*XOR L*/c.A^=c.L;c.F=_z80_szp[c.A];break;
      case 0xae:/*XOR (HL)*/a=c.HL;_MR(a,v);c.A^=v;c.F=_z80_szp[c.A];break;
      case 0xaf:/*XOR A*/c.A^=c.A;c.F=_z80_szp[c.A];break;
      case 0xb0:/*OR B*/c.A|=c.B;c.F=_z80_szp[c.A];break;
      case 0xb1:/*OR C*/c.A|=c.C;c.F=_z80_szp[c.A];break;
      case 0xb2:/*OR D*/c.A|=c.D;c.F=_z80_szp[c.A];break;
      case 0xb3:/*OR E*/c.A|=c.E;c.F=_z80_szp[c.A];break;
      case 0xb4:/*OR H*/c.A|=c.H;c.F=_z80_szp[c.A];break;
      case 0xb5:/*OR L*/c.A|=c.L;c.F=_z80_szp[c.A];break;
      case 0xb6:/*OR (HL)*/a=c.HL;_MR(a,v);c.A|=v;c.F=_z80_szp[c.A];break;
      case 0xb7:/*OR A*/c.A|=c.A;c.F=_z80_szp[c.A];break;
      case 0xb8:/*CP B*/{int res=(int)c.A-(int)c.B;c.F=_CP_FLAGS(c.A,c.B,res);}break;
      case 0xb9:/*CP C*/{int res=(int)c.A-(int)c.C;c.F=_CP_FLAGS(c.A,c.C,res);}break;
      case 0xba:/*CP D*/{int res=(int)c.A-(int)c.D;c.F=_CP_FLAGS(c.A,c.D,res);}break;
      case 0xbb:/*CP E*/{int res=(int)c.A-(int)c.E;c.F=_CP_FLAGS(c.A,c.E,res);}break;
      case 0xbc:/*CP H*/{int res=(int)c.A-(int)c.H;c.F=_CP_FLAGS(c.A,c.H,res);}break;
      case 0xbd:/*CP L*/{int res=(int)c.A-(int)c.L;c.F=_CP_FLAGS(c.A,c.L,res);}break;
      case 0xbe:/*CP (HL)*/a=c.HL;_MR(a,v);{int res=(int)c.A-(int)v;c.F=_CP_FLAGS(c.A,v,res);}break;
      case 0xbf:/*CP A*/{int res=(int)c.A-(int)c.A;c.F=_CP_FLAGS(c.A,c.A,res);}break;
      case 0xc0:/*RET NZ*/_T(1);if(!(c.F&Z80_ZF)){uint8_t w,z;_MR(c.SP++,z);_MR(c.SP++,w);c.PC=c.WZ=(w<<8)|z;}break;
      case 0xc1:/*POP BC*/{uint8_t l,h;_MR(c.SP++,l);_MR(c.SP++,h);c.BC=(h<<8)|l;}break;
      case 0xc2:/*JP NZ,nn*/_IMM16();if (!(c.F&Z80_ZF)) { c.PC=c.WZ; }break;
      case 0xc3:/*JP nn*/_IMM16();c.PC=c.WZ;break;
      case 0xc4:/*CALL NZ,nn*/_IMM16();if (!(c.F&Z80_ZF)){_T(1);_MW(--c.SP,(uint8_t)(c.PC>>8));_MW(--c.SP,(uint8_t)c.PC);c.PC=c.WZ;}break;
      case 0xc5:/*PUSH BC*/_T(1);_MW(--c.SP,(uint8_t)(c.BC>>8));_MW(--c.SP,(uint8_t)c.BC);break;
      case 0xc6:/*ADD n*/_MR(c.PC++,v);{int res=c.A+v;c.F=_ADD_FLAGS(c.A,v,res);c.A=(uint8_t)res;}break;
      case 0xc7:/*RST 0x0*/_MW(--c.SP,(uint8_t)(c.PC>>8));_MW(--c.SP,(uint8_t)c.PC);c.WZ=c.PC=(uint16_t)0x0;break;
      case 0xc8:/*RET Z*/_T(1);if((c.F&Z80_ZF)){uint8_t w,z;_MR(c.SP++,z);_MR(c.SP++,w);c.PC=c.WZ=(w<<8)|z;}break;
      case 0xc9:/*RET*/{uint8_t w,z;_MR(c.SP++,z);_MR(c.SP++,w);c.PC=c.WZ=(w<<8)|z;}break;
      case 0xca:/*JP Z,nn*/_IMM16();if ((c.F&Z80_ZF)) { c.PC=c.WZ; }break;
      case 0xcb:
        {
        _FETCH(opcode);
        switch (opcode) {
          default:
              {
                uint8_t* vptr;
                switch (opcode&7) {
                  case 0: vptr=&c.B; break;
                  case 1: vptr=&c.C; break;
                  case 2: vptr=&c.D; break;
                  case 3: vptr=&c.E; break;
                  case 4: vptr=&c.H; break;
                  case 5: vptr=&c.L; break;
                  case 6: vptr=0; break;
                  case 7: vptr=&c.A; break;
                }
                uint8_t y=(opcode>>3)&7;
                switch (opcode>>6) {
                  case 0:
                    /* ROT n,r */
                    if (vptr) {
                      switch (y) {
                        case 0:/*RLC r*/{uint8_t r=*vptr<<1|*vptr>>7;c.F=_z80_szp[r]|(*vptr>>7&Z80_CF);*vptr=r;}break;
                        case 1:/*RRC r*/{uint8_t r=*vptr>>1|*vptr<<7;c.F=_z80_szp[r]|(*vptr&Z80_CF);*vptr=r;}break;
                        case 2:/*RL  r*/{uint8_t r=*vptr<<1|(c.F&Z80_CF);c.F=(*vptr>>7&Z80_CF)|_z80_szp[r];*vptr=r;}break;
                        case 3:/*RR  r*/{uint8_t r=*vptr>>1|((c.F&Z80_CF)<<7);c.F=(*vptr&Z80_CF)|_z80_szp[r];*vptr=r;}break;
                        case 4:/*SLA r*/{uint8_t r=*vptr<<1;c.F=(*vptr>>7&Z80_CF)|_z80_szp[r];*vptr=r;}break;
                        case 5:/*SRA r*/{uint8_t r=*vptr>>1|(*vptr&0x80);c.F=(*vptr&Z80_CF)|_z80_szp[r];*vptr=r;}break;
                        case 6:/*SLL r*/{uint8_t r=(*vptr<<1)|1;c.F=(*vptr>>7&Z80_CF)|_z80_szp[r];*vptr=r;}break;
                        case 7:/*SRL r*/{uint8_t r=*vptr>>1;c.F=(*vptr&Z80_CF)|_z80_szp[r];*vptr=r;}break;
                      }
                    } else {
                      switch (y) {
                        case 0:/*RLC (HL)*/a=c.HL;_T(1);_MR(a,v);{uint8_t r=v<<1|v>>7;c.F=_z80_szp[r]|(v>>7&Z80_CF);v=r;}_MW(a,v);break;
                        case 1:/*RRC (HL)*/a=c.HL;_T(1);_MR(a,v);{uint8_t r=v>>1|v<<7;c.F=_z80_szp[r]|(v&Z80_CF);v=r;}_MW(a,v);break;
                        case 2:/*RL  (HL)*/a=c.HL;_T(1);_MR(a,v);{uint8_t r=v<<1|(c.F&Z80_CF);c.F=(v>>7&Z80_CF)|_z80_szp[r];v=r;}_MW(a,v);break;
                        case 3:/*RR  (HL)*/a=c.HL;_T(1);_MR(a,v);{uint8_t r=v>>1|((c.F & Z80_CF)<<7);c.F=(v&Z80_CF)|_z80_szp[r];v=r;}_MW(a,v);break;
                        case 4:/*SLA (HL)*/a=c.HL;_T(1);_MR(a,v);{uint8_t r=v<<1;c.F=(v>>7&Z80_CF)|_z80_szp[r];v=r;}_MW(a,v);break;
                        case 5:/*SRA (HL)*/a=c.HL;_T(1);_MR(a,v);{uint8_t r=v>>1|(v&0x80);c.F=(v&Z80_CF)|_z80_szp[r];v=r;}_MW(a,v);break;
                        case 6:/*SLL (HL)*/a=c.HL;_T(1);_MR(a,v);{uint8_t r=(v<<1)|1;c.F=(v>>7&Z80_CF)|_z80_szp[r];v=r;}_MW(a,v);break;
                        case 7:/*SRL (HL)*/a=c.HL;_T(1);_MR(a,v);{uint8_t r=v>>1;c.F=(v&Z80_CF)|_z80_szp[r];v=r;}_MW(a,v);break;
                      }
                    }
                    break;
                  case 1:
                    /* BIT n,r */
                    if (vptr) {
                      v=*vptr&(1<<y);f=Z80_HF|(v?(v&Z80_SF):(Z80_ZF|Z80_PF))|(*vptr&(Z80_YF|Z80_XF));c.F=f|(c.F&Z80_CF);
                    } else {
                      a=c.HL;_T(1);_MR(a,v);v&=(1<<y);f=Z80_HF|(v?(v&Z80_SF):(Z80_ZF|Z80_PF))|((c.WZ>>8)&(Z80_YF|Z80_XF));c.F=f|(c.F&Z80_CF);
                    }
                    break;
                  case 2:
                    /* RES n,r */
                    if (vptr) {
                      *vptr&=~(1<<y);
                    } else {
                      a=c.HL;_T(1);_MR(a,v);_MW(a,v&~(1<<y));
                    }
                    break;
                  case 3:
                    /* RES n,r */
                    if (vptr) {
                      *vptr|=(1<<y);
                    } else {
                      a=c.HL;_T(1);_MR(a,v);_MW(a,v|(1<<y));
                    }
                    break;
                }
              }
            break;
        } }
        break;
      case 0xcc:/*CALL Z,nn*/_IMM16();if ((c.F&Z80_ZF)){_T(1);_MW(--c.SP,(uint8_t)(c.PC>>8));_MW(--c.SP,(uint8_t)c.PC);c.PC=c.WZ;}break;
      case 0xcd:/*CALL nn*/_IMM16();_T(1);_MW(--c.SP,(uint8_t)(c.PC>>8));_MW(--c.SP,(uint8_t)c.PC);c.PC=c.WZ;break;
      case 0xce:/*ADC n*/_MR(c.PC++,v);{int res=c.A+v+(c.F&Z80_CF);c.F=_ADD_FLAGS(c.A,v,res);c.A=(uint8_t)res;}break;
      case 0xcf:/*RST 0x8*/_MW(--c.SP,(uint8_t)(c.PC>>8));_MW(--c.SP,(uint8_t)c.PC);c.WZ=c.PC=(uint16_t)0x8;break;
      case 0xd0:/*RET NC*/_T(1);if(!(c.F&Z80_CF)){uint8_t w,z;_MR(c.SP++,z);_MR(c.SP++,w);c.PC=c.WZ=(w<<8)|z;}break;
      case 0xd1:/*POP DE*/{uint8_t l,h;_MR(c.SP++,l);_MR(c.SP++,h);c.DE=(h<<8)|l;}break;
      case 0xd2:/*JP NC,nn*/_IMM16();if (!(c.F&Z80_CF)) { c.PC=c.WZ; }break;
      case 0xd3:/*OUT (n),A*/_MR(c.PC++,v);c.WZ=((c.A<<8)|v);_OUT(c.WZ,c.A);{uint8_t z=(uint8_t)c.WZ;z++;c.WZ=(c.WZ&0xFF00)|z;}break;
      case 0xd4:/*CALL NC,nn*/_IMM16();if (!(c.F&Z80_CF)){_T(1);_MW(--c.SP,(uint8_t)(c.PC>>8));_MW(--c.SP,(uint8_t)c.PC);c.PC=c.WZ;}break;
      case 0xd5:/*PUSH DE*/_T(1);_MW(--c.SP,(uint8_t)(c.DE>>8));_MW(--c.SP,(uint8_t)c.DE);break;
      case 0xd6:/*SUB n*/_MR(c.PC++,v);{int res=(int)c.A-(int)v;c.F=_SUB_FLAGS(c.A,v,res);c.A=(uint8_t)res;}break;
      case 0xd7:/*RST 0x10*/_MW(--c.SP,(uint8_t)(c.PC>>8));_MW(--c.SP,(uint8_t)c.PC);c.WZ=c.PC=(uint16_t)0x10;break;
      case 0xd8:/*RET C*/_T(1);if((c.F&Z80_CF)){uint8_t w,z;_MR(c.SP++,z);_MR(c.SP++,w);c.PC=c.WZ=(w<<8)|z;}break;
      case 0xd9:/*EXX*/{uint16_t tmp=c.BC;c.BC=c.BC_;c.BC_=tmp;}{uint16_t tmp=c.DE;c.DE=c.DE_;c.DE_=tmp;}{uint16_t tmp=c.HL;c.HL=c.HL_;c.HL_=tmp;}break;
      case 0xda:/*JP C,nn*/_IMM16();if ((c.F&Z80_CF)) { c.PC=c.WZ; }break;
      case 0xdb:/*IN A,(n)*/_MR(c.PC++,v);c.WZ=((c.A<<8)|v);_IN(c.WZ++,c.A);break;
      case 0xdc:/*CALL C,nn*/_IMM16();if ((c.F&Z80_CF)){_T(1);_MW(--c.SP,(uint8_t)(c.PC>>8));_MW(--c.SP,(uint8_t)c.PC);c.PC=c.WZ;}break;
      case 0xdd:
        {
        _FETCH(opcode);
        switch (opcode) {
          case 0x0:/*NOP*/ break;
          case 0x1:/*LD BC,nn*/_IMM16();c.BC=c.WZ;break;
          case 0x2:/*LD (BC),A*/c.WZ=c.BC;_MW(c.WZ++,c.A);;c.WZ=(c.WZ&0x00FF)|(c.A<<8);break;
          case 0x3:/*INC BC*/_T(2);c.BC++;break;
          case 0x4:/*INC B*/{uint8_t r=c.B+1;f=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^c.B)&Z80_HF);if(r==0x80){f|=Z80_VF;}c.F=f|(c.F&Z80_CF);c.B=r;}break;
          case 0x5:/*DEC B*/{uint8_t r=c.B-1;f=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^c.B)&Z80_HF);if(r==0x7F){f|=Z80_VF;}c.F=f|(c.F&Z80_CF);c.B=r;}break;
          case 0x6:/*LD B,n*/_MR(c.PC++,c.B);break;
          case 0x7:/*RLCA*/{uint8_t r=c.A<<1|c.A>>7;c.F=(c.A>>7&Z80_CF)|(c.F&(Z80_SF|Z80_ZF|Z80_PF))|(r&(Z80_XF|Z80_YF));c.A=r;}break;
          case 0x8:/*EX AF,AF'*/{uint16_t tmp=c.AF;c.AF=c.AF_;c.AF_=tmp;}break;
          case 0x9:/*ADD IX,BC*/{c.WZ=c.IX+1;uint32_t r=c.IX+c.BC;c.F=(c.F&(Z80_SF|Z80_ZF|Z80_VF))|(((c.IX^r^c.BC)>>8)&Z80_HF)|((r>>16)&Z80_CF)|((r>>8)&(Z80_YF|Z80_XF));c.IX=r;}_T(7);break;
          case 0xa:/*LD A,(BC)*/c.WZ=c.BC;_MR(c.WZ++,c.A);;break;
          case 0xb:/*DEC BC*/_T(2);c.BC--;break;
          case 0xc:/*INC C*/{uint8_t r=c.C+1;f=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^c.C)&Z80_HF);if(r==0x80){f|=Z80_VF;}c.F=f|(c.F&Z80_CF);c.C=r;}break;
          case 0xd:/*DEC C*/{uint8_t r=c.C-1;f=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^c.C)&Z80_HF);if(r==0x7F){f|=Z80_VF;}c.F=f|(c.F&Z80_CF);c.C=r;}break;
          case 0xe:/*LD C,n*/_MR(c.PC++,c.C);break;
          case 0xf:/*RRCA*/{uint8_t r=c.A>>1|c.A<<7;c.F=(c.A&Z80_CF)|(c.F&(Z80_SF|Z80_ZF|Z80_PF))|(r&(Z80_YF|Z80_XF));c.A=r;}break;
          case 0x10:/*DJNZ*/_T(1);{int8_t d;_MR(c.PC++,d);if(--c.B>0){c.WZ=c.PC=c.PC+d;_T(5);}}break;
          case 0x11:/*LD DE,nn*/_IMM16();c.DE=c.WZ;break;
          case 0x12:/*LD (DE),A*/c.WZ=c.DE;_MW(c.WZ++,c.A);;c.WZ=(c.WZ&0x00FF)|(c.A<<8);break;
          case 0x13:/*INC DE*/_T(2);c.DE++;break;
          case 0x14:/*INC D*/{uint8_t r=c.D+1;f=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^c.D)&Z80_HF);if(r==0x80){f|=Z80_VF;}c.F=f|(c.F&Z80_CF);c.D=r;}break;
          case 0x15:/*DEC D*/{uint8_t r=c.D-1;f=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^c.D)&Z80_HF);if(r==0x7F){f|=Z80_VF;}c.F=f|(c.F&Z80_CF);c.D=r;}break;
          case 0x16:/*LD D,n*/_MR(c.PC++,c.D);break;
          case 0x17:/*RLA*/{uint8_t r=c.A<<1|(c.F&Z80_CF);c.F=(c.A>>7&Z80_CF)|(c.F&(Z80_SF|Z80_ZF|Z80_PF))|(r&(Z80_YF|Z80_XF));c.A=r;}break;
          case 0x18:/*JR d*/{int8_t d;_MR(c.PC++,d);c.WZ=c.PC=c.PC+d;}_T(5);break;
          case 0x19:/*ADD IX,DE*/{c.WZ=c.IX+1;uint32_t r=c.IX+c.DE;c.F=(c.F&(Z80_SF|Z80_ZF|Z80_VF))|(((c.IX^r^c.DE)>>8)&Z80_HF)|((r>>16)&Z80_CF)|((r>>8)&(Z80_YF|Z80_XF));c.IX=r;}_T(7);break;
          case 0x1a:/*LD A,(DE)*/c.WZ=c.DE;_MR(c.WZ++,c.A);;break;
          case 0x1b:/*DEC DE*/_T(2);c.DE--;break;
          case 0x1c:/*INC E*/{uint8_t r=c.E+1;f=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^c.E)&Z80_HF);if(r==0x80){f|=Z80_VF;}c.F=f|(c.F&Z80_CF);c.E=r;}break;
          case 0x1d:/*DEC E*/{uint8_t r=c.E-1;f=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^c.E)&Z80_HF);if(r==0x7F){f|=Z80_VF;}c.F=f|(c.F&Z80_CF);c.E=r;}break;
          case 0x1e:/*LD E,n*/_MR(c.PC++,c.E);break;
          case 0x1f:/*RRA*/{uint8_t r=c.A>>1|((c.F&Z80_CF)<<7);c.F=(c.A&Z80_CF)|(c.F&(Z80_SF|Z80_ZF|Z80_PF))|(r&(Z80_YF|Z80_XF));c.A=r;}break;
          case 0x20:/*JR NZ,d*/{int8_t d;_MR(c.PC++,d);if(!(c.F&Z80_ZF)){c.WZ=c.PC=c.PC+d;_T(5);}}break;
          case 0x21:/*LD IX,nn*/_IMM16();c.IX=c.WZ;break;
          case 0x22:/*LD (nn),IX*/_IMM16();_MW(c.WZ++,(uint8_t)c.IX);_MW(c.WZ,(uint8_t)(c.IX>>8));break;
          case 0x23:/*INC IX*/_T(2);c.IX++;break;
          case 0x24:/*INC IXH*/{uint8_t r=c.IXH+1;f=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^c.IXH)&Z80_HF);if(r==0x80){f|=Z80_VF;}c.F=f|(c.F&Z80_CF);c.IXH=r;}break;
          case 0x25:/*DEC IXH*/{uint8_t r=c.IXH-1;f=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^c.IXH)&Z80_HF);if(r==0x7F){f|=Z80_VF;}c.F=f|(c.F&Z80_CF);c.IXH=r;}break;
          case 0x26:/*LD IXH,n*/_MR(c.PC++,c.IXH);break;
          case 0x27:/*DAA*/v=c.A;if(c.F&Z80_NF){if(((c.A&0xF)>0x9)||(c.F&Z80_HF)){v-=0x06;}if((c.A>0x99)||(c.F&Z80_CF)){v-=0x60;}}else{if(((c.A&0xF)>0x9)||(c.F&Z80_HF)){v+=0x06;}if((c.A>0x99)||(c.F&Z80_CF)){v+=0x60;}}c.F&=Z80_CF|Z80_NF;c.F|=(c.A>0x99)?Z80_CF:0;c.F|=(c.A^v)&Z80_HF;c.F|=_z80_szp[v];c.A=v;break;
          case 0x28:/*JR Z,d*/{int8_t d;_MR(c.PC++,d);if((c.F&Z80_ZF)){c.WZ=c.PC=c.PC+d;_T(5);}}break;
          case 0x29:/*ADD IX,IX*/{c.WZ=c.IX+1;uint32_t r=c.IX+c.IX;c.F=(c.F&(Z80_SF|Z80_ZF|Z80_VF))|(((c.IX^r^c.IX)>>8)&Z80_HF)|((r>>16)&Z80_CF)|((r>>8)&(Z80_YF|Z80_XF));c.IX=r;}_T(7);break;
          case 0x2a:/*LD IX,(nn)*/_IMM16();{uint8_t l,h;_MR(c.WZ++,l);_MR(c.WZ,h);c.IX=(h<<8)|l;}break;
          case 0x2b:/*DEC IX*/_T(2);c.IX--;break;
          case 0x2c:/*INC IXL*/{uint8_t r=c.IXL+1;f=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^c.IXL)&Z80_HF);if(r==0x80){f|=Z80_VF;}c.F=f|(c.F&Z80_CF);c.IXL=r;}break;
          case 0x2d:/*DEC IXL*/{uint8_t r=c.IXL-1;f=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^c.IXL)&Z80_HF);if(r==0x7F){f|=Z80_VF;}c.F=f|(c.F&Z80_CF);c.IXL=r;}break;
          case 0x2e:/*LD IXL,n*/_MR(c.PC++,c.IXL);break;
          case 0x2f:/*CPL*/c.A^=0xFF;c.F=(c.F&(Z80_SF|Z80_ZF|Z80_PF|Z80_CF))|Z80_HF|Z80_NF|(c.A&(Z80_YF|Z80_XF));break;
          case 0x30:/*JR NC,d*/{int8_t d;_MR(c.PC++,d);if(!(c.F&Z80_CF)){c.WZ=c.PC=c.PC+d;_T(5);}}break;
          case 0x31:/*LD SP,nn*/_IMM16();c.SP=c.WZ;break;
          case 0x32:/*LD (nn),A*/_IMM16();_MW(c.WZ++,c.A);;c.WZ=(c.WZ&0x00FF)|(c.A<<8);break;
          case 0x33:/*INC SP*/_T(2);c.SP++;break;
          case 0x34:/*INC (IX+d)*/{int8_t d;_MR(c.PC++,d);;a=c.WZ=c.IX+d;}_T(5);_T(1);_MR(a,v);{uint8_t r=v+1;f=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^v)&Z80_HF);if(r==0x80){f|=Z80_VF;}c.F=f|(c.F&Z80_CF);v=r;}_MW(a,v);break;
          case 0x35:/*DEC (IX+d)*/{int8_t d;_MR(c.PC++,d);;a=c.WZ=c.IX+d;}_T(5);_T(1);_MR(a,v);{uint8_t r=v-1;f=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^v)&Z80_HF);if(r==0x7F){f|=Z80_VF;}c.F=f|(c.F&Z80_CF);v=r;}_MW(a,v);break;
          case 0x36:/*LD (IX+d),n*/{int8_t d;_MR(c.PC++,d);;a=c.WZ=c.IX+d;}_T(2);_MR(c.PC++,v);_MW(a,v);break;
          case 0x37:/*SCF*/c.F=(c.F&(Z80_SF|Z80_ZF|Z80_YF|Z80_XF|Z80_PF))|Z80_CF|(c.A&(Z80_YF|Z80_XF));break;
          case 0x38:/*JR C,d*/{int8_t d;_MR(c.PC++,d);if((c.F&Z80_CF)){c.WZ=c.PC=c.PC+d;_T(5);}}break;
          case 0x39:/*ADD IX,SP*/{c.WZ=c.IX+1;uint32_t r=c.IX+c.SP;c.F=(c.F&(Z80_SF|Z80_ZF|Z80_VF))|(((c.IX^r^c.SP)>>8)&Z80_HF)|((r>>16)&Z80_CF)|((r>>8)&(Z80_YF|Z80_XF));c.IX=r;}_T(7);break;
          case 0x3a:/*LD A,(nn)*/_IMM16();_MR(c.WZ++,c.A);;break;
          case 0x3b:/*DEC SP*/_T(2);c.SP--;break;
          case 0x3c:/*INC A*/{uint8_t r=c.A+1;f=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^c.A)&Z80_HF);if(r==0x80){f|=Z80_VF;}c.F=f|(c.F&Z80_CF);c.A=r;}break;
          case 0x3d:/*DEC A*/{uint8_t r=c.A-1;f=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^c.A)&Z80_HF);if(r==0x7F){f|=Z80_VF;}c.F=f|(c.F&Z80_CF);c.A=r;}break;
          case 0x3e:/*LD A,n*/_MR(c.PC++,c.A);break;
          case 0x3f:/*CCF*/c.F=((c.F&(Z80_SF|Z80_ZF|Z80_YF|Z80_XF|Z80_PF|Z80_CF))|((c.F&Z80_CF)<<4)|(c.A&(Z80_YF|Z80_XF)))^Z80_CF;break;
          case 0x40:/*LD B,B*/c.B=c.B;break;
          case 0x41:/*LD B,C*/c.B=c.C;break;
          case 0x42:/*LD B,D*/c.B=c.D;break;
          case 0x43:/*LD B,E*/c.B=c.E;break;
          case 0x44:/*LD B,IXH*/c.B=c.IXH;break;
          case 0x45:/*LD B,IXL*/c.B=c.IXL;break;
          case 0x46:/*LD B,(IX+d)*/{int8_t d;_MR(c.PC++,d);;a=c.WZ=c.IX+d;}_T(5);_MR(a,c.B);break;
          case 0x47:/*LD B,A*/c.B=c.A;break;
          case 0x48:/*LD C,B*/c.C=c.B;break;
          case 0x49:/*LD C,C*/c.C=c.C;break;
          case 0x4a:/*LD C,D*/c.C=c.D;break;
          case 0x4b:/*LD C,E*/c.C=c.E;break;
          case 0x4c:/*LD C,IXH*/c.C=c.IXH;break;
          case 0x4d:/*LD C,IXL*/c.C=c.IXL;break;
          case 0x4e:/*LD C,(IX+d)*/{int8_t d;_MR(c.PC++,d);;a=c.WZ=c.IX+d;}_T(5);_MR(a,c.C);break;
          case 0x4f:/*LD C,A*/c.C=c.A;break;
          case 0x50:/*LD D,B*/c.D=c.B;break;
          case 0x51:/*LD D,C*/c.D=c.C;break;
          case 0x52:/*LD D,D*/c.D=c.D;break;
          case 0x53:/*LD D,E*/c.D=c.E;break;
          case 0x54:/*LD D,IXH*/c.D=c.IXH;break;
          case 0x55:/*LD D,IXL*/c.D=c.IXL;break;
          case 0x56:/*LD D,(IX+d)*/{int8_t d;_MR(c.PC++,d);;a=c.WZ=c.IX+d;}_T(5);_MR(a,c.D);break;
          case 0x57:/*LD D,A*/c.D=c.A;break;
          case 0x58:/*LD E,B*/c.E=c.B;break;
          case 0x59:/*LD E,C*/c.E=c.C;break;
          case 0x5a:/*LD E,D*/c.E=c.D;break;
          case 0x5b:/*LD E,E*/c.E=c.E;break;
          case 0x5c:/*LD E,IXH*/c.E=c.IXH;break;
          case 0x5d:/*LD E,IXL*/c.E=c.IXL;break;
          case 0x5e:/*LD E,(IX+d)*/{int8_t d;_MR(c.PC++,d);;a=c.WZ=c.IX+d;}_T(5);_MR(a,c.E);break;
          case 0x5f:/*LD E,A*/c.E=c.A;break;
          case 0x60:/*LD IXH,B*/c.IXH=c.B;break;
          case 0x61:/*LD IXH,C*/c.IXH=c.C;break;
          case 0x62:/*LD IXH,D*/c.IXH=c.D;break;
          case 0x63:/*LD IXH,E*/c.IXH=c.E;break;
          case 0x64:/*LD IXH,IXH*/c.IXH=c.IXH;break;
          case 0x65:/*LD IXH,IXL*/c.IXH=c.IXL;break;
          case 0x66:/*LD H,(IX+d)*/{int8_t d;_MR(c.PC++,d);;a=c.WZ=c.IX+d;}_T(5);_MR(a,c.H);break;
          case 0x67:/*LD IXH,A*/c.IXH=c.A;break;
          case 0x68:/*LD IXL,B*/c.IXL=c.B;break;
          case 0x69:/*LD IXL,C*/c.IXL=c.C;break;
          case 0x6a:/*LD IXL,D*/c.IXL=c.D;break;
          case 0x6b:/*LD IXL,E*/c.IXL=c.E;break;
          case 0x6c:/*LD IXL,IXH*/c.IXL=c.IXH;break;
          case 0x6d:/*LD IXL,IXL*/c.IXL=c.IXL;break;
          case 0x6e:/*LD L,(IX+d)*/{int8_t d;_MR(c.PC++,d);;a=c.WZ=c.IX+d;}_T(5);_MR(a,c.L);break;
          case 0x6f:/*LD IXL,A*/c.IXL=c.A;break;
          case 0x70:/*LD (IX+d),B*/{int8_t d;_MR(c.PC++,d);;a=c.WZ=c.IX+d;}_T(5);_MW(a,c.B);break;
          case 0x71:/*LD (IX+d),C*/{int8_t d;_MR(c.PC++,d);;a=c.WZ=c.IX+d;}_T(5);_MW(a,c.C);break;
          case 0x72:/*LD (IX+d),D*/{int8_t d;_MR(c.PC++,d);;a=c.WZ=c.IX+d;}_T(5);_MW(a,c.D);break;
          case 0x73:/*LD (IX+d),E*/{int8_t d;_MR(c.PC++,d);;a=c.WZ=c.IX+d;}_T(5);_MW(a,c.E);break;
          case 0x74:/*LD (IX+d),H*/{int8_t d;_MR(c.PC++,d);;a=c.WZ=c.IX+d;}_T(5);_MW(a,c.H);break;
          case 0x75:/*LD (IX+d),L*/{int8_t d;_MR(c.PC++,d);;a=c.WZ=c.IX+d;}_T(5);_MW(a,c.L);break;
          case 0x76:/*HALT*/_ON(Z80_HALT);c.PC--;break;
          case 0x77:/*LD (IX+d),A*/{int8_t d;_MR(c.PC++,d);;a=c.WZ=c.IX+d;}_T(5);_MW(a,c.A);break;
          case 0x78:/*LD A,B*/c.A=c.B;break;
          case 0x79:/*LD A,C*/c.A=c.C;break;
          case 0x7a:/*LD A,D*/c.A=c.D;break;
          case 0x7b:/*LD A,E*/c.A=c.E;break;
          case 0x7c:/*LD A,IXH*/c.A=c.IXH;break;
          case 0x7d:/*LD A,IXL*/c.A=c.IXL;break;
          case 0x7e:/*LD A,(IX+d)*/{int8_t d;_MR(c.PC++,d);;a=c.WZ=c.IX+d;}_T(5);_MR(a,c.A);break;
          case 0x7f:/*LD A,A*/c.A=c.A;break;
          case 0x80:/*ADD B*/{int res=c.A+c.B;c.F=_ADD_FLAGS(c.A,c.B,res);c.A=(uint8_t)res;}break;
          case 0x81:/*ADD C*/{int res=c.A+c.C;c.F=_ADD_FLAGS(c.A,c.C,res);c.A=(uint8_t)res;}break;
          case 0x82:/*ADD D*/{int res=c.A+c.D;c.F=_ADD_FLAGS(c.A,c.D,res);c.A=(uint8_t)res;}break;
          case 0x83:/*ADD E*/{int res=c.A+c.E;c.F=_ADD_FLAGS(c.A,c.E,res);c.A=(uint8_t)res;}break;
          case 0x84:/*ADD IXH*/{int res=c.A+c.IXH;c.F=_ADD_FLAGS(c.A,c.IXH,res);c.A=(uint8_t)res;}break;
          case 0x85:/*ADD IXL*/{int res=c.A+c.IXL;c.F=_ADD_FLAGS(c.A,c.IXL,res);c.A=(uint8_t)res;}break;
          case 0x86:/*ADD (IX+d)*/{int8_t d;_MR(c.PC++,d);;a=c.WZ=c.IX+d;}_T(5);_MR(a,v);{int res=c.A+v;c.F=_ADD_FLAGS(c.A,v,res);c.A=(uint8_t)res;}break;
          case 0x87:/*ADD A*/{int res=c.A+c.A;c.F=_ADD_FLAGS(c.A,c.A,res);c.A=(uint8_t)res;}break;
          case 0x88:/*ADC B*/{int res=c.A+c.B+(c.F&Z80_CF);c.F=_ADD_FLAGS(c.A,c.B,res);c.A=(uint8_t)res;}break;
          case 0x89:/*ADC C*/{int res=c.A+c.C+(c.F&Z80_CF);c.F=_ADD_FLAGS(c.A,c.C,res);c.A=(uint8_t)res;}break;
          case 0x8a:/*ADC D*/{int res=c.A+c.D+(c.F&Z80_CF);c.F=_ADD_FLAGS(c.A,c.D,res);c.A=(uint8_t)res;}break;
          case 0x8b:/*ADC E*/{int res=c.A+c.E+(c.F&Z80_CF);c.F=_ADD_FLAGS(c.A,c.E,res);c.A=(uint8_t)res;}break;
          case 0x8c:/*ADC IXH*/{int res=c.A+c.IXH+(c.F&Z80_CF);c.F=_ADD_FLAGS(c.A,c.IXH,res);c.A=(uint8_t)res;}break;
          case 0x8d:/*ADC IXL*/{int res=c.A+c.IXL+(c.F&Z80_CF);c.F=_ADD_FLAGS(c.A,c.IXL,res);c.A=(uint8_t)res;}break;
          case 0x8e:/*ADC (IX+d)*/{int8_t d;_MR(c.PC++,d);;a=c.WZ=c.IX+d;}_T(5);_MR(a,v);{int res=c.A+v+(c.F&Z80_CF);c.F=_ADD_FLAGS(c.A,v,res);c.A=(uint8_t)res;}break;
          case 0x8f:/*ADC A*/{int res=c.A+c.A+(c.F&Z80_CF);c.F=_ADD_FLAGS(c.A,c.A,res);c.A=(uint8_t)res;}break;
          case 0x90:/*SUB B*/{int res=(int)c.A-(int)c.B;c.F=_SUB_FLAGS(c.A,c.B,res);c.A=(uint8_t)res;}break;
          case 0x91:/*SUB C*/{int res=(int)c.A-(int)c.C;c.F=_SUB_FLAGS(c.A,c.C,res);c.A=(uint8_t)res;}break;
          case 0x92:/*SUB D*/{int res=(int)c.A-(int)c.D;c.F=_SUB_FLAGS(c.A,c.D,res);c.A=(uint8_t)res;}break;
          case 0x93:/*SUB E*/{int res=(int)c.A-(int)c.E;c.F=_SUB_FLAGS(c.A,c.E,res);c.A=(uint8_t)res;}break;
          case 0x94:/*SUB IXH*/{int res=(int)c.A-(int)c.IXH;c.F=_SUB_FLAGS(c.A,c.IXH,res);c.A=(uint8_t)res;}break;
          case 0x95:/*SUB IXL*/{int res=(int)c.A-(int)c.IXL;c.F=_SUB_FLAGS(c.A,c.IXL,res);c.A=(uint8_t)res;}break;
          case 0x96:/*SUB (IX+d)*/{int8_t d;_MR(c.PC++,d);;a=c.WZ=c.IX+d;}_T(5);_MR(a,v);{int res=(int)c.A-(int)v;c.F=_SUB_FLAGS(c.A,v,res);c.A=(uint8_t)res;}break;
          case 0x97:/*SUB A*/{int res=(int)c.A-(int)c.A;c.F=_SUB_FLAGS(c.A,c.A,res);c.A=(uint8_t)res;}break;
          case 0x98:/*SBC B*/{int res=(int)c.A-(int)c.B-(c.F&Z80_CF);c.F=_SUB_FLAGS(c.A,c.B,res);c.A=(uint8_t)res;}break;
          case 0x99:/*SBC C*/{int res=(int)c.A-(int)c.C-(c.F&Z80_CF);c.F=_SUB_FLAGS(c.A,c.C,res);c.A=(uint8_t)res;}break;
          case 0x9a:/*SBC D*/{int res=(int)c.A-(int)c.D-(c.F&Z80_CF);c.F=_SUB_FLAGS(c.A,c.D,res);c.A=(uint8_t)res;}break;
          case 0x9b:/*SBC E*/{int res=(int)c.A-(int)c.E-(c.F&Z80_CF);c.F=_SUB_FLAGS(c.A,c.E,res);c.A=(uint8_t)res;}break;
          case 0x9c:/*SBC IXH*/{int res=(int)c.A-(int)c.IXH-(c.F&Z80_CF);c.F=_SUB_FLAGS(c.A,c.IXH,res);c.A=(uint8_t)res;}break;
          case 0x9d:/*SBC IXL*/{int res=(int)c.A-(int)c.IXL-(c.F&Z80_CF);c.F=_SUB_FLAGS(c.A,c.IXL,res);c.A=(uint8_t)res;}break;
          case 0x9e:/*SBC (IX+d)*/{int8_t d;_MR(c.PC++,d);;a=c.WZ=c.IX+d;}_T(5);_MR(a,v);{int res=(int)c.A-(int)v-(c.F&Z80_CF);c.F=_SUB_FLAGS(c.A,v,res);c.A=(uint8_t)res;}break;
          case 0x9f:/*SBC A*/{int res=(int)c.A-(int)c.A-(c.F&Z80_CF);c.F=_SUB_FLAGS(c.A,c.A,res);c.A=(uint8_t)res;}break;
          case 0xa0:/*AND B*/c.A&=c.B;c.F=_z80_szp[c.A]|Z80_HF;break;
          case 0xa1:/*AND C*/c.A&=c.C;c.F=_z80_szp[c.A]|Z80_HF;break;
          case 0xa2:/*AND D*/c.A&=c.D;c.F=_z80_szp[c.A]|Z80_HF;break;
          case 0xa3:/*AND E*/c.A&=c.E;c.F=_z80_szp[c.A]|Z80_HF;break;
          case 0xa4:/*AND IXH*/c.A&=c.IXH;c.F=_z80_szp[c.A]|Z80_HF;break;
          case 0xa5:/*AND IXL*/c.A&=c.IXL;c.F=_z80_szp[c.A]|Z80_HF;break;
          case 0xa6:/*AND (IX+d)*/{int8_t d;_MR(c.PC++,d);;a=c.WZ=c.IX+d;}_T(5);_MR(a,v);c.A&=v;c.F=_z80_szp[c.A]|Z80_HF;break;
          case 0xa7:/*AND A*/c.A&=c.A;c.F=_z80_szp[c.A]|Z80_HF;break;
          case 0xa8:/*XOR B*/c.A^=c.B;c.F=_z80_szp[c.A];break;
          case 0xa9:/*XOR C*/c.A^=c.C;c.F=_z80_szp[c.A];break;
          case 0xaa:/*XOR D*/c.A^=c.D;c.F=_z80_szp[c.A];break;
          case 0xab:/*XOR E*/c.A^=c.E;c.F=_z80_szp[c.A];break;
          case 0xac:/*XOR IXH*/c.A^=c.IXH;c.F=_z80_szp[c.A];break;
          case 0xad:/*XOR IXL*/c.A^=c.IXL;c.F=_z80_szp[c.A];break;
          case 0xae:/*XOR (IX+d)*/{int8_t d;_MR(c.PC++,d);;a=c.WZ=c.IX+d;}_T(5);_MR(a,v);c.A^=v;c.F=_z80_szp[c.A];break;
          case 0xaf:/*XOR A*/c.A^=c.A;c.F=_z80_szp[c.A];break;
          case 0xb0:/*OR B*/c.A|=c.B;c.F=_z80_szp[c.A];break;
          case 0xb1:/*OR C*/c.A|=c.C;c.F=_z80_szp[c.A];break;
          case 0xb2:/*OR D*/c.A|=c.D;c.F=_z80_szp[c.A];break;
          case 0xb3:/*OR E*/c.A|=c.E;c.F=_z80_szp[c.A];break;
          case 0xb4:/*OR IXH*/c.A|=c.IXH;c.F=_z80_szp[c.A];break;
          case 0xb5:/*OR IXL*/c.A|=c.IXL;c.F=_z80_szp[c.A];break;
          case 0xb6:/*OR (IX+d)*/{int8_t d;_MR(c.PC++,d);;a=c.WZ=c.IX+d;}_T(5);_MR(a,v);c.A|=v;c.F=_z80_szp[c.A];break;
          case 0xb7:/*OR A*/c.A|=c.A;c.F=_z80_szp[c.A];break;
          case 0xb8:/*CP B*/{int res=(int)c.A-(int)c.B;c.F=_CP_FLAGS(c.A,c.B,res);}break;
          case 0xb9:/*CP C*/{int res=(int)c.A-(int)c.C;c.F=_CP_FLAGS(c.A,c.C,res);}break;
          case 0xba:/*CP D*/{int res=(int)c.A-(int)c.D;c.F=_CP_FLAGS(c.A,c.D,res);}break;
          case 0xbb:/*CP E*/{int res=(int)c.A-(int)c.E;c.F=_CP_FLAGS(c.A,c.E,res);}break;
          case 0xbc:/*CP IXH*/{int res=(int)c.A-(int)c.IXH;c.F=_CP_FLAGS(c.A,c.IXH,res);}break;
          case 0xbd:/*CP IXL*/{int res=(int)c.A-(int)c.IXL;c.F=_CP_FLAGS(c.A,c.IXL,res);}break;
          case 0xbe:/*CP (IX+d)*/{int8_t d;_MR(c.PC++,d);;a=c.WZ=c.IX+d;}_T(5);_MR(a,v);{int res=(int)c.A-(int)v;c.F=_CP_FLAGS(c.A,v,res);}break;
          case 0xbf:/*CP A*/{int res=(int)c.A-(int)c.A;c.F=_CP_FLAGS(c.A,c.A,res);}break;
          case 0xc0:/*RET NZ*/_T(1);if(!(c.F&Z80_ZF)){uint8_t w,z;_MR(c.SP++,z);_MR(c.SP++,w);c.PC=c.WZ=(w<<8)|z;}break;
          case 0xc1:/*POP BC*/{uint8_t l,h;_MR(c.SP++,l);_MR(c.SP++,h);c.BC=(h<<8)|l;}break;
          case 0xc2:/*JP NZ,nn*/_IMM16();if (!(c.F&Z80_ZF)) { c.PC=c.WZ; }break;
          case 0xc3:/*JP nn*/_IMM16();c.PC=c.WZ;break;
          case 0xc4:/*CALL NZ,nn*/_IMM16();if (!(c.F&Z80_ZF)){_T(1);_MW(--c.SP,(uint8_t)(c.PC>>8));_MW(--c.SP,(uint8_t)c.PC);c.PC=c.WZ;}break;
          case 0xc5:/*PUSH BC*/_T(1);_MW(--c.SP,(uint8_t)(c.BC>>8));_MW(--c.SP,(uint8_t)c.BC);break;
          case 0xc6:/*ADD n*/_MR(c.PC++,v);{int res=c.A+v;c.F=_ADD_FLAGS(c.A,v,res);c.A=(uint8_t)res;}break;
          case 0xc7:/*RST 0x0*/_MW(--c.SP,(uint8_t)(c.PC>>8));_MW(--c.SP,(uint8_t)c.PC);c.WZ=c.PC=(uint16_t)0x0;break;
          case 0xc8:/*RET Z*/_T(1);if((c.F&Z80_ZF)){uint8_t w,z;_MR(c.SP++,z);_MR(c.SP++,w);c.PC=c.WZ=(w<<8)|z;}break;
          case 0xc9:/*RET*/{uint8_t w,z;_MR(c.SP++,z);_MR(c.SP++,w);c.PC=c.WZ=(w<<8)|z;}break;
          case 0xca:/*JP Z,nn*/_IMM16();if ((c.F&Z80_ZF)) { c.PC=c.WZ; }break;
          case 0xcb:
            {
            int8_t d;_MR(c.PC++,d);
            _FETCH_CB(opcode);
            switch (opcode) {
              default:
                  {
                    uint8_t* vptr;
                    switch (opcode&7) {
                      case 0: vptr=&c.B; break;
                      case 1: vptr=&c.C; break;
                      case 2: vptr=&c.D; break;
                      case 3: vptr=&c.E; break;
                      case 4: vptr=&c.H; break;
                      case 5: vptr=&c.L; break;
                      case 6: vptr=0; break;
                      case 7: vptr=&c.A; break;
                    }
                    uint8_t y=(opcode>>3)&7;
                    switch (opcode>>6) {
                      case 0:
                        /* ROT n,r */
                        if (vptr) {
                          switch (y) {
                            case 0:/*RLC (IX+d),r*/a=c.WZ=c.IX+d;_T(2);_MR(a,v);{uint8_t r=v<<1|v>>7;c.F=_z80_szp[r]|(v>>7&Z80_CF);v=r;}*vptr=v;_MW(a,v);break;
                            case 1:/*RRC (IX+d),r*/a=c.WZ=c.IX+d;_T(2);_MR(a,v);{uint8_t r=v>>1|v<<7;c.F=_z80_szp[r]|(v&Z80_CF);v=r;}*vptr=v;_MW(a,v);break;
                            case 2:/*RL  (IX+d),r*/a=c.WZ=c.IX+d;_T(2);_MR(a,v);{uint8_t r=v<<1|(c.F&Z80_CF);c.F=(v>>7&Z80_CF)|_z80_szp[r];v=r;}*vptr=v;_MW(a,v);break;
                            case 3:/*RR  (IX+d),r*/a=c.WZ=c.IX+d;_T(2);_MR(a,v);{uint8_t r=v>>1|((c.F & Z80_CF)<<7);c.F=(v&Z80_CF)|_z80_szp[r];v=r;}*vptr=v;_MW(a,v);break;
                            case 4:/*SLA (IX+d),r*/a=c.WZ=c.IX+d;_T(2);_MR(a,v);{uint8_t r=v<<1;c.F=(v>>7&Z80_CF)|_z80_szp[r];v=r;}*vptr=v;_MW(a,v);break;
                            case 5:/*SRA (IX+d),r*/a=c.WZ=c.IX+d;_T(2);_MR(a,v);{uint8_t r=v>>1|(v&0x80);c.F=(v&Z80_CF)|_z80_szp[r];v=r;}*vptr=v;_MW(a,v);break;
                            case 6:/*SLL (IX+d),r*/a=c.WZ=c.IX+d;_T(2);_MR(a,v);{uint8_t r=(v<<1)|1;c.F=(v>>7&Z80_CF)|_z80_szp[r];v=r;}*vptr=v;_MW(a,v);break;
                            case 7:/*SRL (IX+d),r*/a=c.WZ=c.IX+d;_T(2);_MR(a,v);{uint8_t r=v>>1;c.F=(v&Z80_CF)|_z80_szp[r];v=r;}*vptr=v;_MW(a,v);break;
                          }
                        } else {
                          switch (y) {
                            case 0:/*RLC (IX+d)*/a=c.WZ=c.IX+d;_T(2);_MR(a,v);{uint8_t r=v<<1|v>>7;c.F=_z80_szp[r]|(v>>7&Z80_CF);v=r;}_MW(a,v);break;
                            case 1:/*RRC (IX+d)*/a=c.WZ=c.IX+d;_T(2);_MR(a,v);{uint8_t r=v>>1|v<<7;c.F=_z80_szp[r]|(v&Z80_CF);v=r;}_MW(a,v);break;
                            case 2:/*RL  (IX+d)*/a=c.WZ=c.IX+d;_T(2);_MR(a,v);{uint8_t r=v<<1|(c.F&Z80_CF);c.F=(v>>7&Z80_CF)|_z80_szp[r];v=r;}_MW(a,v);break;
                            case 3:/*RR  (IX+d)*/a=c.WZ=c.IX+d;_T(2);_MR(a,v);{uint8_t r=v>>1|((c.F & Z80_CF)<<7);c.F=(v&Z80_CF)|_z80_szp[r];v=r;}_MW(a,v);break;
                            case 4:/*SLA (IX+d)*/a=c.WZ=c.IX+d;_T(2);_MR(a,v);{uint8_t r=v<<1;c.F=(v>>7&Z80_CF)|_z80_szp[r];v=r;}_MW(a,v);break;
                            case 5:/*SRA (IX+d)*/a=c.WZ=c.IX+d;_T(2);_MR(a,v);{uint8_t r=v>>1|(v&0x80);c.F=(v&Z80_CF)|_z80_szp[r];v=r;}_MW(a,v);break;
                            case 6:/*SLL (IX+d)*/a=c.WZ=c.IX+d;_T(2);_MR(a,v);{uint8_t r=(v<<1)|1;c.F=(v>>7&Z80_CF)|_z80_szp[r];v=r;}_MW(a,v);break;
                            case 7:/*SRL (IX+d)*/a=c.WZ=c.IX+d;_T(2);_MR(a,v);{uint8_t r=v>>1;c.F=(v&Z80_CF)|_z80_szp[r];v=r;}_MW(a,v);break;
                          }
                        }
                        break;
                      case 1:
                        /* BIT n,(IX|IY+d) */
                        a=c.WZ=c.IX+d;_T(2);_MR(a,v);v&=(1<<y);f=Z80_HF|(v?(v&Z80_SF):(Z80_ZF|Z80_PF))|((c.WZ>>8)&(Z80_YF|Z80_XF));c.F=f|(c.F&Z80_CF);
                        break;
                      case 2:
                        if (vptr) {
                          /* RES n,(IX|IY+d),r (undocumented) */
                          a=c.WZ=c.IX+d;_T(2);_MR(a,v);*vptr=v&~(1<<y);_MW(a,*vptr);
                        } else {
                          /* RES n,(IX|IY+d) */
                          a=c.WZ=c.IX+d;_T(2);_MR(a,v);_MW(a,v&~(1<<y));
                        }
                        break;
                      case 3:
                        if (vptr) {
                          /* SET n,(IX|IY+d),r (undocumented) */
                          a=c.WZ=c.IX+d;_T(2);_MR(a,v);*vptr=v|(1<<y);_MW(a,*vptr);
                        } else {
                          /* RES n,(IX|IY+d) */
                          a=c.WZ=c.IX+d;_T(2);_MR(a,v);_MW(a,v|(1<<y));
                        }
                        break;
                    }
                  }
                break;
            } }
            break;
          case 0xcc:/*CALL Z,nn*/_IMM16();if ((c.F&Z80_ZF)){_T(1);_MW(--c.SP,(uint8_t)(c.PC>>8));_MW(--c.SP,(uint8_t)c.PC);c.PC=c.WZ;}break;
          case 0xcd:/*CALL nn*/_IMM16();_T(1);_MW(--c.SP,(uint8_t)(c.PC>>8));_MW(--c.SP,(uint8_t)c.PC);c.PC=c.WZ;break;
          case 0xce:/*ADC n*/_MR(c.PC++,v);{int res=c.A+v+(c.F&Z80_CF);c.F=_ADD_FLAGS(c.A,v,res);c.A=(uint8_t)res;}break;
          case 0xcf:/*RST 0x8*/_MW(--c.SP,(uint8_t)(c.PC>>8));_MW(--c.SP,(uint8_t)c.PC);c.WZ=c.PC=(uint16_t)0x8;break;
          case 0xd0:/*RET NC*/_T(1);if(!(c.F&Z80_CF)){uint8_t w,z;_MR(c.SP++,z);_MR(c.SP++,w);c.PC=c.WZ=(w<<8)|z;}break;
          case 0xd1:/*POP DE*/{uint8_t l,h;_MR(c.SP++,l);_MR(c.SP++,h);c.DE=(h<<8)|l;}break;
          case 0xd2:/*JP NC,nn*/_IMM16();if (!(c.F&Z80_CF)) { c.PC=c.WZ; }break;
          case 0xd3:/*OUT (n),A*/_MR(c.PC++,v);c.WZ=((c.A<<8)|v);_OUT(c.WZ,c.A);{uint8_t z=(uint8_t)c.WZ;z++;c.WZ=(c.WZ&0xFF00)|z;}break;
          case 0xd4:/*CALL NC,nn*/_IMM16();if (!(c.F&Z80_CF)){_T(1);_MW(--c.SP,(uint8_t)(c.PC>>8));_MW(--c.SP,(uint8_t)c.PC);c.PC=c.WZ;}break;
          case 0xd5:/*PUSH DE*/_T(1);_MW(--c.SP,(uint8_t)(c.DE>>8));_MW(--c.SP,(uint8_t)c.DE);break;
          case 0xd6:/*SUB n*/_MR(c.PC++,v);{int res=(int)c.A-(int)v;c.F=_SUB_FLAGS(c.A,v,res);c.A=(uint8_t)res;}break;
          case 0xd7:/*RST 0x10*/_MW(--c.SP,(uint8_t)(c.PC>>8));_MW(--c.SP,(uint8_t)c.PC);c.WZ=c.PC=(uint16_t)0x10;break;
          case 0xd8:/*RET C*/_T(1);if((c.F&Z80_CF)){uint8_t w,z;_MR(c.SP++,z);_MR(c.SP++,w);c.PC=c.WZ=(w<<8)|z;}break;
          case 0xd9:/*EXX*/{uint16_t tmp=c.BC;c.BC=c.BC_;c.BC_=tmp;}{uint16_t tmp=c.DE;c.DE=c.DE_;c.DE_=tmp;}{uint16_t tmp=c.HL;c.HL=c.HL_;c.HL_=tmp;}break;
          case 0xda:/*JP C,nn*/_IMM16();if ((c.F&Z80_CF)) { c.PC=c.WZ; }break;
          case 0xdb:/*IN A,(n)*/_MR(c.PC++,v);c.WZ=((c.A<<8)|v);_IN(c.WZ++,c.A);break;
          case 0xdc:/*CALL C,nn*/_IMM16();if ((c.F&Z80_CF)){_T(1);_MW(--c.SP,(uint8_t)(c.PC>>8));_MW(--c.SP,(uint8_t)c.PC);c.PC=c.WZ;}break;
          case 0xde:/*SBC n*/_MR(c.PC++,v);{int res=(int)c.A-(int)v-(c.F&Z80_CF);c.F=_SUB_FLAGS(c.A,v,res);c.A=(uint8_t)res;}break;
          case 0xdf:/*RST 0x18*/_MW(--c.SP,(uint8_t)(c.PC>>8));_MW(--c.SP,(uint8_t)c.PC);c.WZ=c.PC=(uint16_t)0x18;break;
          case 0xe0:/*RET PO*/_T(1);if(!(c.F&Z80_PF)){uint8_t w,z;_MR(c.SP++,z);_MR(c.SP++,w);c.PC=c.WZ=(w<<8)|z;}break;
          case 0xe1:/*POP IX*/{uint8_t l,h;_MR(c.SP++,l);_MR(c.SP++,h);c.IX=(h<<8)|l;}break;
          case 0xe2:/*JP PO,nn*/_IMM16();if (!(c.F&Z80_PF)) { c.PC=c.WZ; }break;
          case 0xe3:/*EX (SP),IX*/_T(1);{uint8_t w,z;_MR(c.SP,z);_MR(c.SP+1,w);_MW(c.SP,(uint8_t)c.IX);_MW(c.SP+1,(uint8_t)(c.IX>>8));c.IX=c.WZ=(w<<8)|z;}_T(2);break;
          case 0xe4:/*CALL PO,nn*/_IMM16();if (!(c.F&Z80_PF)){_T(1);_MW(--c.SP,(uint8_t)(c.PC>>8));_MW(--c.SP,(uint8_t)c.PC);c.PC=c.WZ;}break;
          case 0xe5:/*PUSH IX*/_T(1);_MW(--c.SP,(uint8_t)(c.IX>>8));_MW(--c.SP,(uint8_t)c.IX);break;
          case 0xe6:/*AND n*/_MR(c.PC++,v);c.A&=v;c.F=_z80_szp[c.A]|Z80_HF;break;
          case 0xe7:/*RST 0x20*/_MW(--c.SP,(uint8_t)(c.PC>>8));_MW(--c.SP,(uint8_t)c.PC);c.WZ=c.PC=(uint16_t)0x20;break;
          case 0xe8:/*RET PE*/_T(1);if((c.F&Z80_PF)){uint8_t w,z;_MR(c.SP++,z);_MR(c.SP++,w);c.PC=c.WZ=(w<<8)|z;}break;
          case 0xe9:/*JP IX*/c.PC=c.IX;break;
          case 0xea:/*JP PE,nn*/_IMM16();if ((c.F&Z80_PF)) { c.PC=c.WZ; }break;
          case 0xeb:/*EX DE,HL*/{uint16_t tmp=c.DE;c.DE=c.HL;c.HL=tmp;}break;
          case 0xec:/*CALL PE,nn*/_IMM16();if ((c.F&Z80_PF)){_T(1);_MW(--c.SP,(uint8_t)(c.PC>>8));_MW(--c.SP,(uint8_t)c.PC);c.PC=c.WZ;}break;
          case 0xee:/*XOR n*/_MR(c.PC++,v);c.A^=v;c.F=_z80_szp[c.A];break;
          case 0xef:/*RST 0x28*/_MW(--c.SP,(uint8_t)(c.PC>>8));_MW(--c.SP,(uint8_t)c.PC);c.WZ=c.PC=(uint16_t)0x28;break;
          case 0xf0:/*RET P*/_T(1);if(!(c.F&Z80_SF)){uint8_t w,z;_MR(c.SP++,z);_MR(c.SP++,w);c.PC=c.WZ=(w<<8)|z;}break;
          case 0xf1:/*POP AF*/{uint8_t l,h;_MR(c.SP++,l);_MR(c.SP++,h);c.AF=(h<<8)|l;}break;
          case 0xf2:/*JP P,nn*/_IMM16();if (!(c.F&Z80_SF)) { c.PC=c.WZ; }break;
          case 0xf3:/*DI*/c.IFF1=c.IFF2=false;break;
          case 0xf4:/*CALL P,nn*/_IMM16();if (!(c.F&Z80_SF)){_T(1);_MW(--c.SP,(uint8_t)(c.PC>>8));_MW(--c.SP,(uint8_t)c.PC);c.PC=c.WZ;}break;
          case 0xf5:/*PUSH AF*/_T(1);_MW(--c.SP,(uint8_t)(c.AF>>8));_MW(--c.SP,(uint8_t)c.AF);break;
          case 0xf6:/*OR n*/_MR(c.PC++,v);c.A|=v;c.F=_z80_szp[c.A];break;
          case 0xf7:/*RST 0x30*/_MW(--c.SP,(uint8_t)(c.PC>>8));_MW(--c.SP,(uint8_t)c.PC);c.WZ=c.PC=(uint16_t)0x30;break;
          case 0xf8:/*RET M*/_T(1);if((c.F&Z80_SF)){uint8_t w,z;_MR(c.SP++,z);_MR(c.SP++,w);c.PC=c.WZ=(w<<8)|z;}break;
          case 0xf9:/*LD SP,IX*/_T(2);c.SP=c.IX;break;
          case 0xfa:/*JP M,nn*/_IMM16();if ((c.F&Z80_SF)) { c.PC=c.WZ; }break;
          case 0xfb:/*EI*/c.ei_pending=true;break;
          case 0xfc:/*CALL M,nn*/_IMM16();if ((c.F&Z80_SF)){_T(1);_MW(--c.SP,(uint8_t)(c.PC>>8));_MW(--c.SP,(uint8_t)c.PC);c.PC=c.WZ;}break;
          case 0xfe:/*CP n*/_MR(c.PC++,v);{int res=(int)c.A-(int)v;c.F=_CP_FLAGS(c.A,v,res);}break;
          case 0xff:/*RST 0x38*/_MW(--c.SP,(uint8_t)(c.PC>>8));_MW(--c.SP,(uint8_t)c.PC);c.WZ=c.PC=(uint16_t)0x38;break;
          default:
            break;
        } }
        break;
      case 0xde:/*SBC n*/_MR(c.PC++,v);{int res=(int)c.A-(int)v-(c.F&Z80_CF);c.F=_SUB_FLAGS(c.A,v,res);c.A=(uint8_t)res;}break;
      case 0xdf:/*RST 0x18*/_MW(--c.SP,(uint8_t)(c.PC>>8));_MW(--c.SP,(uint8_t)c.PC);c.WZ=c.PC=(uint16_t)0x18;break;
      case 0xe0:/*RET PO*/_T(1);if(!(c.F&Z80_PF)){uint8_t w,z;_MR(c.SP++,z);_MR(c.SP++,w);c.PC=c.WZ=(w<<8)|z;}break;
      case 0xe1:/*POP HL*/{uint8_t l,h;_MR(c.SP++,l);_MR(c.SP++,h);c.HL=(h<<8)|l;}break;
      case 0xe2:/*JP PO,nn*/_IMM16();if (!(c.F&Z80_PF)) { c.PC=c.WZ; }break;
      case 0xe3:/*EX (SP),HL*/_T(1);{uint8_t w,z;_MR(c.SP,z);_MR(c.SP+1,w);_MW(c.SP,(uint8_t)c.HL);_MW(c.SP+1,(uint8_t)(c.HL>>8));c.HL=c.WZ=(w<<8)|z;}_T(2);break;
      case 0xe4:/*CALL PO,nn*/_IMM16();if (!(c.F&Z80_PF)){_T(1);_MW(--c.SP,(uint8_t)(c.PC>>8));_MW(--c.SP,(uint8_t)c.PC);c.PC=c.WZ;}break;
      case 0xe5:/*PUSH HL*/_T(1);_MW(--c.SP,(uint8_t)(c.HL>>8));_MW(--c.SP,(uint8_t)c.HL);break;
      case 0xe6:/*AND n*/_MR(c.PC++,v);c.A&=v;c.F=_z80_szp[c.A]|Z80_HF;break;
      case 0xe7:/*RST 0x20*/_MW(--c.SP,(uint8_t)(c.PC>>8));_MW(--c.SP,(uint8_t)c.PC);c.WZ=c.PC=(uint16_t)0x20;break;
      case 0xe8:/*RET PE*/_T(1);if((c.F&Z80_PF)){uint8_t w,z;_MR(c.SP++,z);_MR(c.SP++,w);c.PC=c.WZ=(w<<8)|z;}break;
      case 0xe9:/*JP HL*/c.PC=c.HL;break;
      case 0xea:/*JP PE,nn*/_IMM16();if ((c.F&Z80_PF)) { c.PC=c.WZ; }break;
      case 0xeb:/*EX DE,HL*/{uint16_t tmp=c.DE;c.DE=c.HL;c.HL=tmp;}break;
      case 0xec:/*CALL PE,nn*/_IMM16();if ((c.F&Z80_PF)){_T(1);_MW(--c.SP,(uint8_t)(c.PC>>8));_MW(--c.SP,(uint8_t)c.PC);c.PC=c.WZ;}break;
      case 0xed:
        {
        _FETCH(opcode);
        switch (opcode) {
          case 0x40:/*IN B,(C)*/c.WZ=c.BC;_IN(c.WZ++,c.B);c.F=_z80_szp[c.B]|(c.F&Z80_CF);break;
          case 0x41:/*OUT (C),B*/c.WZ=c.BC;_OUT(c.WZ++,c.B);break;
          case 0x42:/*SBC HL,BC*/{c.WZ=c.HL+1;uint32_t r=c.HL-c.BC-(c.F&Z80_CF);c.F=(((c.HL^r^c.BC)>>8)&Z80_HF)|Z80_NF|((r>>16)&Z80_CF)|((r>>8)&(Z80_SF|Z80_YF|Z80_XF))|((r&0xFFFF)?0:Z80_ZF)|(((c.BC^c.HL)&(c.HL^r)&0x8000)>>13);c.HL=r;}_T(7);break;
          case 0x43:/*LD (nn),BC*/_IMM16();_MW(c.WZ++,(uint8_t)c.BC);_MW(c.WZ,(uint8_t)(c.BC>>8));break;
          case 0x44:/*NEG*/v=c.A;c.A=0;{int res=(int)c.A-(int)v;c.F=_SUB_FLAGS(c.A,v,res);c.A=(uint8_t)res;}break;
          case 0x46:/*IM 0*/c.IM=0;break;
          case 0x47:/*LD I,A*/_T(1);c.I=c.A;break;
          case 0x48:/*IN C,(C)*/c.WZ=c.BC;_IN(c.WZ++,c.C);c.F=_z80_szp[c.C]|(c.F&Z80_CF);break;
          case 0x49:/*OUT (C),C*/c.WZ=c.BC;_OUT(c.WZ++,c.C);break;
          case 0x4a:/*ADC HL,BC*/{c.WZ=c.HL+1;uint32_t r=c.HL+c.BC+(c.F&Z80_CF);c.F=(((c.HL^r^c.BC)>>8)&Z80_HF)|((r>>16)&Z80_CF)|((r>>8)&(Z80_SF|Z80_YF|Z80_XF))|((r&0xFFFF)?0:Z80_ZF)|(((c.BC^c.HL^0x8000)&(c.BC^r)&0x8000)>>13);c.HL=r;}_T(7);break;
          case 0x4b:/*LD BC,(nn)*/_IMM16();{uint8_t l,h;_MR(c.WZ++,l);_MR(c.WZ,h);c.BC=(h<<8)|l;}break;
          case 0x4c:/*NEG*/v=c.A;c.A=0;{int res=(int)c.A-(int)v;c.F=_SUB_FLAGS(c.A,v,res);c.A=(uint8_t)res;}break;
          case 0x4d:/*RETI*/{uint8_t w,z;_ON(Z80_RETI);_MR(c.SP++,z);_MR(c.SP++,w);c.PC=c.WZ=(w<<8)|z;}break;
          case 0x4e:/*IM 0*/c.IM=0;break;
          case 0x4f:/*LD R,A*/_T(1);c.R=c.A;break;
          case 0x50:/*IN D,(C)*/c.WZ=c.BC;_IN(c.WZ++,c.D);c.F=_z80_szp[c.D]|(c.F&Z80_CF);break;
          case 0x51:/*OUT (C),D*/c.WZ=c.BC;_OUT(c.WZ++,c.D);break;
          case 0x52:/*SBC HL,DE*/{c.WZ=c.HL+1;uint32_t r=c.HL-c.DE-(c.F&Z80_CF);c.F=(((c.HL^r^c.DE)>>8)&Z80_HF)|Z80_NF|((r>>16)&Z80_CF)|((r>>8)&(Z80_SF|Z80_YF|Z80_XF))|((r&0xFFFF)?0:Z80_ZF)|(((c.DE^c.HL)&(c.HL^r)&0x8000)>>13);c.HL=r;}_T(7);break;
          case 0x53:/*LD (nn),DE*/_IMM16();_MW(c.WZ++,(uint8_t)c.DE);_MW(c.WZ,(uint8_t)(c.DE>>8));break;
          case 0x54:/*NEG*/v=c.A;c.A=0;{int res=(int)c.A-(int)v;c.F=_SUB_FLAGS(c.A,v,res);c.A=(uint8_t)res;}break;
          case 0x56:/*IM 1*/c.IM=1;break;
          case 0x57:/*LD A,I*/_T(1);c.A=c.I; c.F=(_SZ(c.I)|(c.I&(Z80_YF|Z80_XF))|(c.IFF2?Z80_PF:0))|(c.F&Z80_CF);break;
          case 0x58:/*IN E,(C)*/c.WZ=c.BC;_IN(c.WZ++,c.E);c.F=_z80_szp[c.E]|(c.F&Z80_CF);break;
          case 0x59:/*OUT (C),E*/c.WZ=c.BC;_OUT(c.WZ++,c.E);break;
          case 0x5a:/*ADC HL,DE*/{c.WZ=c.HL+1;uint32_t r=c.HL+c.DE+(c.F&Z80_CF);c.F=(((c.HL^r^c.DE)>>8)&Z80_HF)|((r>>16)&Z80_CF)|((r>>8)&(Z80_SF|Z80_YF|Z80_XF))|((r&0xFFFF)?0:Z80_ZF)|(((c.DE^c.HL^0x8000)&(c.DE^r)&0x8000)>>13);c.HL=r;}_T(7);break;
          case 0x5b:/*LD DE,(nn)*/_IMM16();{uint8_t l,h;_MR(c.WZ++,l);_MR(c.WZ,h);c.DE=(h<<8)|l;}break;
          case 0x5c:/*NEG*/v=c.A;c.A=0;{int res=(int)c.A-(int)v;c.F=_SUB_FLAGS(c.A,v,res);c.A=(uint8_t)res;}break;
          case 0x5e:/*IM 2*/c.IM=2;break;
          case 0x5f:/*LD A,R*/_T(1);c.A=c.R; c.F=(_SZ(c.R)|(c.R&(Z80_YF|Z80_XF))|(c.IFF2?Z80_PF:0))|(c.F&Z80_CF);break;
          case 0x60:/*IN H,(C)*/c.WZ=c.BC;_IN(c.WZ++,c.H);c.F=_z80_szp[c.H]|(c.F&Z80_CF);break;
          case 0x61:/*OUT (C),H*/c.WZ=c.BC;_OUT(c.WZ++,c.H);break;
          case 0x62:/*SBC HL,HL*/{c.WZ=c.HL+1;uint32_t r=c.HL-c.HL-(c.F&Z80_CF);c.F=(((c.HL^r^c.HL)>>8)&Z80_HF)|Z80_NF|((r>>16)&Z80_CF)|((r>>8)&(Z80_SF|Z80_YF|Z80_XF))|((r&0xFFFF)?0:Z80_ZF)|(((c.HL^c.HL)&(c.HL^r)&0x8000)>>13);c.HL=r;}_T(7);break;
          case 0x63:/*LD (nn),HL*/_IMM16();_MW(c.WZ++,(uint8_t)c.HL);_MW(c.WZ,(uint8_t)(c.HL>>8));break;
          case 0x64:/*NEG*/v=c.A;c.A=0;{int res=(int)c.A-(int)v;c.F=_SUB_FLAGS(c.A,v,res);c.A=(uint8_t)res;}break;
          case 0x66:/*IM 0*/c.IM=0;break;
          case 0x67:/*RRD*/{uint8_t l,v;c.WZ=c.HL;_MR(c.WZ++,v);l=c.A&0x0F;c.A=(c.A&0xF0)|(v&0x0F);v=(v>>4)|(l<<4);_T(4);_MW(c.HL,v);c.F=_z80_szp[c.A]|(c.F&Z80_CF);}break;
          case 0x68:/*IN L,(C)*/c.WZ=c.BC;_IN(c.WZ++,c.L);c.F=_z80_szp[c.L]|(c.F&Z80_CF);break;
          case 0x69:/*OUT (C),L*/c.WZ=c.BC;_OUT(c.WZ++,c.L);break;
          case 0x6a:/*ADC HL,HL*/{c.WZ=c.HL+1;uint32_t r=c.HL+c.HL+(c.F&Z80_CF);c.F=(((c.HL^r^c.HL)>>8)&Z80_HF)|((r>>16)&Z80_CF)|((r>>8)&(Z80_SF|Z80_YF|Z80_XF))|((r&0xFFFF)?0:Z80_ZF)|(((c.HL^c.HL^0x8000)&(c.HL^r)&0x8000)>>13);c.HL=r;}_T(7);break;
          case 0x6b:/*LD HL,(nn)*/_IMM16();{uint8_t l,h;_MR(c.WZ++,l);_MR(c.WZ,h);c.HL=(h<<8)|l;}break;
          case 0x6c:/*NEG*/v=c.A;c.A=0;{int res=(int)c.A-(int)v;c.F=_SUB_FLAGS(c.A,v,res);c.A=(uint8_t)res;}break;
          case 0x6e:/*IM 0*/c.IM=0;break;
          case 0x6f:/*RLD*/{uint8_t l,v;c.WZ=c.HL;_MR(c.WZ++,v);l=c.A&0x0F;c.A=(c.A&0xF0)|(v>>4);v=(v<<4)|l;_T(4);_MW(c.HL,v);c.F=_z80_szp[c.A]|(c.F&Z80_CF);}break;
          case 0x70:/*IN (C)*/c.WZ=c.BC;_IN(c.WZ++,v);c.F=_z80_szp[v]|(c.F&Z80_CF);break;
          case 0x71:/*???*/c.WZ=c.BC;_OUT(c.WZ++,0);break;
          case 0x72:/*SBC HL,SP*/{c.WZ=c.HL+1;uint32_t r=c.HL-c.SP-(c.F&Z80_CF);c.F=(((c.HL^r^c.SP)>>8)&Z80_HF)|Z80_NF|((r>>16)&Z80_CF)|((r>>8)&(Z80_SF|Z80_YF|Z80_XF))|((r&0xFFFF)?0:Z80_ZF)|(((c.SP^c.HL)&(c.HL^r)&0x8000)>>13);c.HL=r;}_T(7);break;
          case 0x73:/*LD (nn),SP*/_IMM16();_MW(c.WZ++,(uint8_t)c.SP);_MW(c.WZ,(uint8_t)(c.SP>>8));break;
          case 0x74:/*NEG*/v=c.A;c.A=0;{int res=(int)c.A-(int)v;c.F=_SUB_FLAGS(c.A,v,res);c.A=(uint8_t)res;}break;
          case 0x76:/*IM 1*/c.IM=1;break;
          case 0x77:/*NOP (ED)*/ break;
          case 0x78:/*IN A,(C)*/c.WZ=c.BC;_IN(c.WZ++,c.A);c.F=_z80_szp[c.A]|(c.F&Z80_CF);break;
          case 0x79:/*OUT (C),A*/c.WZ=c.BC;_OUT(c.WZ++,c.A);break;
          case 0x7a:/*ADC HL,SP*/{c.WZ=c.HL+1;uint32_t r=c.HL+c.SP+(c.F&Z80_CF);c.F=(((c.HL^r^c.SP)>>8)&Z80_HF)|((r>>16)&Z80_CF)|((r>>8)&(Z80_SF|Z80_YF|Z80_XF))|((r&0xFFFF)?0:Z80_ZF)|(((c.SP^c.HL^0x8000)&(c.SP^r)&0x8000)>>13);c.HL=r;}_T(7);break;
          case 0x7b:/*LD SP,(nn)*/_IMM16();{uint8_t l,h;_MR(c.WZ++,l);_MR(c.WZ,h);c.SP=(h<<8)|l;}break;
          case 0x7c:/*NEG*/v=c.A;c.A=0;{int res=(int)c.A-(int)v;c.F=_SUB_FLAGS(c.A,v,res);c.A=(uint8_t)res;}break;
          case 0x7e:/*IM 2*/c.IM=2;break;
          case 0x7f:/*NOP (ED)*/ break;
          case 0xa0:/*LDI*/_MR(c.HL,v);_MW(c.DE,v);_T(2);v+=c.A;f=c.F&(Z80_SF|Z80_ZF|Z80_CF);if(v&0x02){f|=Z80_YF;}if(v&0x08){f|=Z80_XF;}c.HL++;c.DE++;c.BC--;if(c.BC){f|=Z80_VF;}c.F=f;break;
          case 0xa1:/*CPI*/_MR(c.HL,v);_T(5);{int r=(int)c.A-v;f=Z80_NF|(c.F&Z80_CF)|_SZ(r);if((r&0x0F)>(c.A&0x0F)){f|=Z80_HF;r--;}if(r&0x02){f|=Z80_YF;}if(r&0x08){f|=Z80_XF;}}c.WZ++;c.HL++;c.BC--;if(c.BC){f|=Z80_VF;}c.F=f;break;
          case 0xa2:/*INI*/_T(1);c.WZ=c.BC;_IN(c.WZ++,v);c.B--;_MW(c.HL++,v);f=c.B?(c.B&Z80_SF):Z80_ZF;if(v&Z80_SF){f|=Z80_NF;}{uint32_t t=(uint32_t)((c.C+(+1))&0xFF)+(uint32_t)v;if(t&0x100){f|=(Z80_HF|Z80_CF);}f|=_z80_szp[((uint8_t)(t&0x07))^c.B]&Z80_PF;}c.F=f;break;
          case 0xa3:/*OUTI*/_T(1);_MR(c.HL++,v);c.B--;c.WZ=c.BC;_OUT(c.WZ++,v);f=c.B?(c.B&Z80_SF):Z80_ZF;if(v&Z80_SF){f|=Z80_NF;}{uint32_t t=(uint32_t)c.L+(uint32_t)v;if(t&0x100){f|=(Z80_HF|Z80_CF);}f|=_z80_szp[((uint8_t)(t&0x07))^c.B]&Z80_PF;}c.F=f;break;
          case 0xa8:/*LDD*/_MR(c.HL,v);_MW(c.DE,v);_T(2);v+=c.A;f=c.F&(Z80_SF|Z80_ZF|Z80_CF);if(v&0x02){f|=Z80_YF;}if(v&0x08){f|=Z80_XF;}c.HL--;c.DE--;c.BC--;if(c.BC){f|=Z80_VF;}c.F=f;break;
          case 0xa9:/*CPD*/_MR(c.HL,v);{int r=(int)c.A-v;_T(5);f=Z80_NF|(c.F&Z80_CF)|_SZ(r);if((r&0x0F)>(c.A&0x0F)){f|=Z80_HF;r--;}if(r&0x02){f|=Z80_YF;}if(r&0x08){f|=Z80_XF;}}c.WZ--;c.HL--;c.BC--;if(c.BC){f|=Z80_VF;}c.F=f;break;
          case 0xaa:/*IND*/_T(1);c.WZ=c.BC;_IN(c.WZ--,v);c.B--;_MW(c.HL--,v);f=c.B?(c.B&Z80_SF):Z80_ZF;if(v&Z80_SF){f|=Z80_NF;}{uint32_t t=(uint32_t)((c.C+(-1))&0xFF)+(uint32_t)v;if(t&0x100){f|=(Z80_HF|Z80_CF);}f|=_z80_szp[((uint8_t)(t&0x07))^c.B]&Z80_PF;}c.F=f;break;
          case 0xab:/*OUTD*/_T(1);_MR(c.HL--,v);c.B--;c.WZ=c.BC;_OUT(c.WZ--,v);f=c.B?(c.B&Z80_SF):Z80_ZF;if(v&Z80_SF){f|=Z80_NF;}{uint32_t t=(uint32_t)c.L+(uint32_t)v;if(t&0x100){f|=(Z80_HF|Z80_CF);}f|=_z80_szp[((uint8_t)(t&0x07))^c.B]&Z80_PF;}c.F=f;break;
          case 0xb0:/*LDIR*/_MR(c.HL,v);_MW(c.DE,v);_T(2);v+=c.A;f=c.F&(Z80_SF|Z80_ZF|Z80_CF);if(v&0x02){f|=Z80_YF;}if(v&0x08){f|=Z80_XF;}c.HL++;c.DE++;c.BC--;if(c.BC){f|=Z80_VF;}c.F=f;if(c.BC){c.PC-=2;c.WZ=c.PC+1;_T(5);}break;
          case 0xb1:/*CPIR*/_MR(c.HL,v);_T(5);{int r=(int)c.A-v;f=Z80_NF|(c.F&Z80_CF)|_SZ(r);if((r&0x0F)>(c.A&0x0F)){f|=Z80_HF;r--;}if(r&0x02){f|=Z80_YF;}if(r&0x08){f|=Z80_XF;}}c.WZ++;c.HL++;c.BC--;if(c.BC){f|=Z80_VF;}c.F=f;if(c.BC&&!(c.F&Z80_ZF)){c.PC-=2;c.WZ=c.PC+1;_T(5);}break;
          case 0xb2:/*INIR*/_T(1);c.WZ=c.BC;_IN(c.WZ++,v);c.B--;_MW(c.HL++,v);f=c.B?(c.B&Z80_SF):Z80_ZF;if(v&Z80_SF){f|=Z80_NF;}{uint32_t t=(uint32_t)((c.C+(+1))&0xFF)+(uint32_t)v;if(t&0x100){f|=(Z80_HF|Z80_CF);}f|=_z80_szp[((uint8_t)(t&0x07))^c.B]&Z80_PF;}c.F=f;if(c.B){c.PC-=2;_T(5);}break;
          case 0xb3:/*OTIR*/_T(1);_MR(c.HL++,v);c.B--;c.WZ=c.BC;_OUT(c.WZ++,v);f=c.B?(c.B&Z80_SF):Z80_ZF;if(v&Z80_SF){f|=Z80_NF;}{uint32_t t=(uint32_t)c.L+(uint32_t)v;if(t&0x100){f|=(Z80_HF|Z80_CF);}f|=_z80_szp[((uint8_t)(t&0x07))^c.B]&Z80_PF;}c.F=f;if(c.B){c.PC-=2;_T(5);}break;
          case 0xb8:/*LDDR*/_MR(c.HL,v);_MW(c.DE,v);_T(2);v+=c.A;f=c.F&(Z80_SF|Z80_ZF|Z80_CF);if(v&0x02){f|=Z80_YF;}if(v&0x08){f|=Z80_XF;}c.HL--;c.DE--;c.BC--;if(c.BC){f|=Z80_VF;}c.F=f;if(c.BC){c.PC-=2;c.WZ=c.PC+1;_T(5);}break;
          case 0xb9:/*CPDR*/_MR(c.HL,v);{int r=(int)c.A-v;_T(5);f=Z80_NF|(c.F&Z80_CF)|_SZ(r);if((r&0x0F)>(c.A&0x0F)){f|=Z80_HF;r--;}if(r&0x02){f|=Z80_YF;}if(r&0x08){f|=Z80_XF;}}c.WZ--;c.HL--;c.BC--;if(c.BC){f|=Z80_VF;}c.F=f;if((c.BC)&&!(c.F&Z80_ZF)){c.PC-=2;c.WZ=c.PC+1;_T(5);}break;
          case 0xba:/*INDR*/_T(1);c.WZ=c.BC;_IN(c.WZ--,v);c.B--;_MW(c.HL--,v);f=c.B?(c.B&Z80_SF):Z80_ZF;if(v&Z80_SF){f|=Z80_NF;}{uint32_t t=(uint32_t)((c.C+(-1))&0xFF)+(uint32_t)v;if(t&0x100){f|=(Z80_HF|Z80_CF);}f|=_z80_szp[((uint8_t)(t&0x07))^c.B]&Z80_PF;}c.F=f;if(c.B){c.PC-=2;_T(5);}break;
          case 0xbb:/*OTDR*/_T(1);_MR(c.HL--,v);c.B--;c.WZ=c.BC;_OUT(c.WZ--,v);f=c.B?(c.B&Z80_SF):Z80_ZF;if(v&Z80_SF){f|=Z80_NF;}{uint32_t t=(uint32_t)c.L+(uint32_t)v;if(t&0x100){f|=(Z80_HF|Z80_CF);}f|=_z80_szp[((uint8_t)(t&0x07))^c.B]&Z80_PF;}c.F=f;if(c.B){c.PC-=2;_T(5);}break;
          default:
            break;
        } }
        break;
      case 0xee:/*XOR n*/_MR(c.PC++,v);c.A^=v;c.F=_z80_szp[c.A];break;
      case 0xef:/*RST 0x28*/_MW(--c.SP,(uint8_t)(c.PC>>8));_MW(--c.SP,(uint8_t)c.PC);c.WZ=c.PC=(uint16_t)0x28;break;
      case 0xf0:/*RET P*/_T(1);if(!(c.F&Z80_SF)){uint8_t w,z;_MR(c.SP++,z);_MR(c.SP++,w);c.PC=c.WZ=(w<<8)|z;}break;
      case 0xf1:/*POP AF*/{uint8_t l,h;_MR(c.SP++,l);_MR(c.SP++,h);c.AF=(h<<8)|l;}break;
      case 0xf2:/*JP P,nn*/_IMM16();if (!(c.F&Z80_SF)) { c.PC=c.WZ; }break;
      case 0xf3:/*DI*/c.IFF1=c.IFF2=false;break;
      case 0xf4:/*CALL P,nn*/_IMM16();if (!(c.F&Z80_SF)){_T(1);_MW(--c.SP,(uint8_t)(c.PC>>8));_MW(--c.SP,(uint8_t)c.PC);c.PC=c.WZ;}break;
      case 0xf5:/*PUSH AF*/_T(1);_MW(--c.SP,(uint8_t)(c.AF>>8));_MW(--c.SP,(uint8_t)c.AF);break;
      case 0xf6:/*OR n*/_MR(c.PC++,v);c.A|=v;c.F=_z80_szp[c.A];break;
      case 0xf7:/*RST 0x30*/_MW(--c.SP,(uint8_t)(c.PC>>8));_MW(--c.SP,(uint8_t)c.PC);c.WZ=c.PC=(uint16_t)0x30;break;
      case 0xf8:/*RET M*/_T(1);if((c.F&Z80_SF)){uint8_t w,z;_MR(c.SP++,z);_MR(c.SP++,w);c.PC=c.WZ=(w<<8)|z;}break;
      case 0xf9:/*LD SP,HL*/_T(2);c.SP=c.HL;break;
      case 0xfa:/*JP M,nn*/_IMM16();if ((c.F&Z80_SF)) { c.PC=c.WZ; }break;
      case 0xfb:/*EI*/c.ei_pending=true;break;
      case 0xfc:/*CALL M,nn*/_IMM16();if ((c.F&Z80_SF)){_T(1);_MW(--c.SP,(uint8_t)(c.PC>>8));_MW(--c.SP,(uint8_t)c.PC);c.PC=c.WZ;}break;
      case 0xfd:
        {
        _FETCH(opcode);
        switch (opcode) {
          case 0x0:/*NOP*/ break;
          case 0x1:/*LD BC,nn*/_IMM16();c.BC=c.WZ;break;
          case 0x2:/*LD (BC),A*/c.WZ=c.BC;_MW(c.WZ++,c.A);;c.WZ=(c.WZ&0x00FF)|(c.A<<8);break;
          case 0x3:/*INC BC*/_T(2);c.BC++;break;
          case 0x4:/*INC B*/{uint8_t r=c.B+1;f=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^c.B)&Z80_HF);if(r==0x80){f|=Z80_VF;}c.F=f|(c.F&Z80_CF);c.B=r;}break;
          case 0x5:/*DEC B*/{uint8_t r=c.B-1;f=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^c.B)&Z80_HF);if(r==0x7F){f|=Z80_VF;}c.F=f|(c.F&Z80_CF);c.B=r;}break;
          case 0x6:/*LD B,n*/_MR(c.PC++,c.B);break;
          case 0x7:/*RLCA*/{uint8_t r=c.A<<1|c.A>>7;c.F=(c.A>>7&Z80_CF)|(c.F&(Z80_SF|Z80_ZF|Z80_PF))|(r&(Z80_XF|Z80_YF));c.A=r;}break;
          case 0x8:/*EX AF,AF'*/{uint16_t tmp=c.AF;c.AF=c.AF_;c.AF_=tmp;}break;
          case 0x9:/*ADD IY,BC*/{c.WZ=c.IY+1;uint32_t r=c.IY+c.BC;c.F=(c.F&(Z80_SF|Z80_ZF|Z80_VF))|(((c.IY^r^c.BC)>>8)&Z80_HF)|((r>>16)&Z80_CF)|((r>>8)&(Z80_YF|Z80_XF));c.IY=r;}_T(7);break;
          case 0xa:/*LD A,(BC)*/c.WZ=c.BC;_MR(c.WZ++,c.A);;break;
          case 0xb:/*DEC BC*/_T(2);c.BC--;break;
          case 0xc:/*INC C*/{uint8_t r=c.C+1;f=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^c.C)&Z80_HF);if(r==0x80){f|=Z80_VF;}c.F=f|(c.F&Z80_CF);c.C=r;}break;
          case 0xd:/*DEC C*/{uint8_t r=c.C-1;f=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^c.C)&Z80_HF);if(r==0x7F){f|=Z80_VF;}c.F=f|(c.F&Z80_CF);c.C=r;}break;
          case 0xe:/*LD C,n*/_MR(c.PC++,c.C);break;
          case 0xf:/*RRCA*/{uint8_t r=c.A>>1|c.A<<7;c.F=(c.A&Z80_CF)|(c.F&(Z80_SF|Z80_ZF|Z80_PF))|(r&(Z80_YF|Z80_XF));c.A=r;}break;
          case 0x10:/*DJNZ*/_T(1);{int8_t d;_MR(c.PC++,d);if(--c.B>0){c.WZ=c.PC=c.PC+d;_T(5);}}break;
          case 0x11:/*LD DE,nn*/_IMM16();c.DE=c.WZ;break;
          case 0x12:/*LD (DE),A*/c.WZ=c.DE;_MW(c.WZ++,c.A);;c.WZ=(c.WZ&0x00FF)|(c.A<<8);break;
          case 0x13:/*INC DE*/_T(2);c.DE++;break;
          case 0x14:/*INC D*/{uint8_t r=c.D+1;f=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^c.D)&Z80_HF);if(r==0x80){f|=Z80_VF;}c.F=f|(c.F&Z80_CF);c.D=r;}break;
          case 0x15:/*DEC D*/{uint8_t r=c.D-1;f=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^c.D)&Z80_HF);if(r==0x7F){f|=Z80_VF;}c.F=f|(c.F&Z80_CF);c.D=r;}break;
          case 0x16:/*LD D,n*/_MR(c.PC++,c.D);break;
          case 0x17:/*RLA*/{uint8_t r=c.A<<1|(c.F&Z80_CF);c.F=(c.A>>7&Z80_CF)|(c.F&(Z80_SF|Z80_ZF|Z80_PF))|(r&(Z80_YF|Z80_XF));c.A=r;}break;
          case 0x18:/*JR d*/{int8_t d;_MR(c.PC++,d);c.WZ=c.PC=c.PC+d;}_T(5);break;
          case 0x19:/*ADD IY,DE*/{c.WZ=c.IY+1;uint32_t r=c.IY+c.DE;c.F=(c.F&(Z80_SF|Z80_ZF|Z80_VF))|(((c.IY^r^c.DE)>>8)&Z80_HF)|((r>>16)&Z80_CF)|((r>>8)&(Z80_YF|Z80_XF));c.IY=r;}_T(7);break;
          case 0x1a:/*LD A,(DE)*/c.WZ=c.DE;_MR(c.WZ++,c.A);;break;
          case 0x1b:/*DEC DE*/_T(2);c.DE--;break;
          case 0x1c:/*INC E*/{uint8_t r=c.E+1;f=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^c.E)&Z80_HF);if(r==0x80){f|=Z80_VF;}c.F=f|(c.F&Z80_CF);c.E=r;}break;
          case 0x1d:/*DEC E*/{uint8_t r=c.E-1;f=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^c.E)&Z80_HF);if(r==0x7F){f|=Z80_VF;}c.F=f|(c.F&Z80_CF);c.E=r;}break;
          case 0x1e:/*LD E,n*/_MR(c.PC++,c.E);break;
          case 0x1f:/*RRA*/{uint8_t r=c.A>>1|((c.F&Z80_CF)<<7);c.F=(c.A&Z80_CF)|(c.F&(Z80_SF|Z80_ZF|Z80_PF))|(r&(Z80_YF|Z80_XF));c.A=r;}break;
          case 0x20:/*JR NZ,d*/{int8_t d;_MR(c.PC++,d);if(!(c.F&Z80_ZF)){c.WZ=c.PC=c.PC+d;_T(5);}}break;
          case 0x21:/*LD IY,nn*/_IMM16();c.IY=c.WZ;break;
          case 0x22:/*LD (nn),IY*/_IMM16();_MW(c.WZ++,(uint8_t)c.IY);_MW(c.WZ,(uint8_t)(c.IY>>8));break;
          case 0x23:/*INC IY*/_T(2);c.IY++;break;
          case 0x24:/*INC IYH*/{uint8_t r=c.IYH+1;f=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^c.IYH)&Z80_HF);if(r==0x80){f|=Z80_VF;}c.F=f|(c.F&Z80_CF);c.IYH=r;}break;
          case 0x25:/*DEC IYH*/{uint8_t r=c.IYH-1;f=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^c.IYH)&Z80_HF);if(r==0x7F){f|=Z80_VF;}c.F=f|(c.F&Z80_CF);c.IYH=r;}break;
          case 0x26:/*LD IYH,n*/_MR(c.PC++,c.IYH);break;
          case 0x27:/*DAA*/v=c.A;if(c.F&Z80_NF){if(((c.A&0xF)>0x9)||(c.F&Z80_HF)){v-=0x06;}if((c.A>0x99)||(c.F&Z80_CF)){v-=0x60;}}else{if(((c.A&0xF)>0x9)||(c.F&Z80_HF)){v+=0x06;}if((c.A>0x99)||(c.F&Z80_CF)){v+=0x60;}}c.F&=Z80_CF|Z80_NF;c.F|=(c.A>0x99)?Z80_CF:0;c.F|=(c.A^v)&Z80_HF;c.F|=_z80_szp[v];c.A=v;break;
          case 0x28:/*JR Z,d*/{int8_t d;_MR(c.PC++,d);if((c.F&Z80_ZF)){c.WZ=c.PC=c.PC+d;_T(5);}}break;
          case 0x29:/*ADD IY,IY*/{c.WZ=c.IY+1;uint32_t r=c.IY+c.IY;c.F=(c.F&(Z80_SF|Z80_ZF|Z80_VF))|(((c.IY^r^c.IY)>>8)&Z80_HF)|((r>>16)&Z80_CF)|((r>>8)&(Z80_YF|Z80_XF));c.IY=r;}_T(7);break;
          case 0x2a:/*LD IY,(nn)*/_IMM16();{uint8_t l,h;_MR(c.WZ++,l);_MR(c.WZ,h);c.IY=(h<<8)|l;}break;
          case 0x2b:/*DEC IY*/_T(2);c.IY--;break;
          case 0x2c:/*INC IYL*/{uint8_t r=c.IYL+1;f=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^c.IYL)&Z80_HF);if(r==0x80){f|=Z80_VF;}c.F=f|(c.F&Z80_CF);c.IYL=r;}break;
          case 0x2d:/*DEC IYL*/{uint8_t r=c.IYL-1;f=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^c.IYL)&Z80_HF);if(r==0x7F){f|=Z80_VF;}c.F=f|(c.F&Z80_CF);c.IYL=r;}break;
          case 0x2e:/*LD IYL,n*/_MR(c.PC++,c.IYL);break;
          case 0x2f:/*CPL*/c.A^=0xFF;c.F=(c.F&(Z80_SF|Z80_ZF|Z80_PF|Z80_CF))|Z80_HF|Z80_NF|(c.A&(Z80_YF|Z80_XF));break;
          case 0x30:/*JR NC,d*/{int8_t d;_MR(c.PC++,d);if(!(c.F&Z80_CF)){c.WZ=c.PC=c.PC+d;_T(5);}}break;
          case 0x31:/*LD SP,nn*/_IMM16();c.SP=c.WZ;break;
          case 0x32:/*LD (nn),A*/_IMM16();_MW(c.WZ++,c.A);;c.WZ=(c.WZ&0x00FF)|(c.A<<8);break;
          case 0x33:/*INC SP*/_T(2);c.SP++;break;
          case 0x34:/*INC (IY+d)*/{int8_t d;_MR(c.PC++,d);;a=c.WZ=c.IY+d;}_T(5);_T(1);_MR(a,v);{uint8_t r=v+1;f=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^v)&Z80_HF);if(r==0x80){f|=Z80_VF;}c.F=f|(c.F&Z80_CF);v=r;}_MW(a,v);break;
          case 0x35:/*DEC (IY+d)*/{int8_t d;_MR(c.PC++,d);;a=c.WZ=c.IY+d;}_T(5);_T(1);_MR(a,v);{uint8_t r=v-1;f=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^v)&Z80_HF);if(r==0x7F){f|=Z80_VF;}c.F=f|(c.F&Z80_CF);v=r;}_MW(a,v);break;
          case 0x36:/*LD (IY+d),n*/{int8_t d;_MR(c.PC++,d);;a=c.WZ=c.IY+d;}_T(2);_MR(c.PC++,v);_MW(a,v);break;
          case 0x37:/*SCF*/c.F=(c.F&(Z80_SF|Z80_ZF|Z80_YF|Z80_XF|Z80_PF))|Z80_CF|(c.A&(Z80_YF|Z80_XF));break;
          case 0x38:/*JR C,d*/{int8_t d;_MR(c.PC++,d);if((c.F&Z80_CF)){c.WZ=c.PC=c.PC+d;_T(5);}}break;
          case 0x39:/*ADD IY,SP*/{c.WZ=c.IY+1;uint32_t r=c.IY+c.SP;c.F=(c.F&(Z80_SF|Z80_ZF|Z80_VF))|(((c.IY^r^c.SP)>>8)&Z80_HF)|((r>>16)&Z80_CF)|((r>>8)&(Z80_YF|Z80_XF));c.IY=r;}_T(7);break;
          case 0x3a:/*LD A,(nn)*/_IMM16();_MR(c.WZ++,c.A);;break;
          case 0x3b:/*DEC SP*/_T(2);c.SP--;break;
          case 0x3c:/*INC A*/{uint8_t r=c.A+1;f=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^c.A)&Z80_HF);if(r==0x80){f|=Z80_VF;}c.F=f|(c.F&Z80_CF);c.A=r;}break;
          case 0x3d:/*DEC A*/{uint8_t r=c.A-1;f=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^c.A)&Z80_HF);if(r==0x7F){f|=Z80_VF;}c.F=f|(c.F&Z80_CF);c.A=r;}break;
          case 0x3e:/*LD A,n*/_MR(c.PC++,c.A);break;
          case 0x3f:/*CCF*/c.F=((c.F&(Z80_SF|Z80_ZF|Z80_YF|Z80_XF|Z80_PF|Z80_CF))|((c.F&Z80_CF)<<4)|(c.A&(Z80_YF|Z80_XF)))^Z80_CF;break;
          case 0x40:/*LD B,B*/c.B=c.B;break;
          case 0x41:/*LD B,C*/c.B=c.C;break;
          case 0x42:/*LD B,D*/c.B=c.D;break;
          case 0x43:/*LD B,E*/c.B=c.E;break;
          case 0x44:/*LD B,IYH*/c.B=c.IYH;break;
          case 0x45:/*LD B,IYL*/c.B=c.IYL;break;
          case 0x46:/*LD B,(IY+d)*/{int8_t d;_MR(c.PC++,d);;a=c.WZ=c.IY+d;}_T(5);_MR(a,c.B);break;
          case 0x47:/*LD B,A*/c.B=c.A;break;
          case 0x48:/*LD C,B*/c.C=c.B;break;
          case 0x49:/*LD C,C*/c.C=c.C;break;
          case 0x4a:/*LD C,D*/c.C=c.D;break;
          case 0x4b:/*LD C,E*/c.C=c.E;break;
          case 0x4c:/*LD C,IYH*/c.C=c.IYH;break;
          case 0x4d:/*LD C,IYL*/c.C=c.IYL;break;
          case 0x4e:/*LD C,(IY+d)*/{int8_t d;_MR(c.PC++,d);;a=c.WZ=c.IY+d;}_T(5);_MR(a,c.C);break;
          case 0x4f:/*LD C,A*/c.C=c.A;break;
          case 0x50:/*LD D,B*/c.D=c.B;break;
          case 0x51:/*LD D,C*/c.D=c.C;break;
          case 0x52:/*LD D,D*/c.D=c.D;break;
          case 0x53:/*LD D,E*/c.D=c.E;break;
          case 0x54:/*LD D,IYH*/c.D=c.IYH;break;
          case 0x55:/*LD D,IYL*/c.D=c.IYL;break;
          case 0x56:/*LD D,(IY+d)*/{int8_t d;_MR(c.PC++,d);;a=c.WZ=c.IY+d;}_T(5);_MR(a,c.D);break;
          case 0x57:/*LD D,A*/c.D=c.A;break;
          case 0x58:/*LD E,B*/c.E=c.B;break;
          case 0x59:/*LD E,C*/c.E=c.C;break;
          case 0x5a:/*LD E,D*/c.E=c.D;break;
          case 0x5b:/*LD E,E*/c.E=c.E;break;
          case 0x5c:/*LD E,IYH*/c.E=c.IYH;break;
          case 0x5d:/*LD E,IYL*/c.E=c.IYL;break;
          case 0x5e:/*LD E,(IY+d)*/{int8_t d;_MR(c.PC++,d);;a=c.WZ=c.IY+d;}_T(5);_MR(a,c.E);break;
          case 0x5f:/*LD E,A*/c.E=c.A;break;
          case 0x60:/*LD IYH,B*/c.IYH=c.B;break;
          case 0x61:/*LD IYH,C*/c.IYH=c.C;break;
          case 0x62:/*LD IYH,D*/c.IYH=c.D;break;
          case 0x63:/*LD IYH,E*/c.IYH=c.E;break;
          case 0x64:/*LD IYH,IYH*/c.IYH=c.IYH;break;
          case 0x65:/*LD IYH,IYL*/c.IYH=c.IYL;break;
          case 0x66:/*LD H,(IY+d)*/{int8_t d;_MR(c.PC++,d);;a=c.WZ=c.IY+d;}_T(5);_MR(a,c.H);break;
          case 0x67:/*LD IYH,A*/c.IYH=c.A;break;
          case 0x68:/*LD IYL,B*/c.IYL=c.B;break;
          case 0x69:/*LD IYL,C*/c.IYL=c.C;break;
          case 0x6a:/*LD IYL,D*/c.IYL=c.D;break;
          case 0x6b:/*LD IYL,E*/c.IYL=c.E;break;
          case 0x6c:/*LD IYL,IYH*/c.IYL=c.IYH;break;
          case 0x6d:/*LD IYL,IYL*/c.IYL=c.IYL;break;
          case 0x6e:/*LD L,(IY+d)*/{int8_t d;_MR(c.PC++,d);;a=c.WZ=c.IY+d;}_T(5);_MR(a,c.L);break;
          case 0x6f:/*LD IYL,A*/c.IYL=c.A;break;
          case 0x70:/*LD (IY+d),B*/{int8_t d;_MR(c.PC++,d);;a=c.WZ=c.IY+d;}_T(5);_MW(a,c.B);break;
          case 0x71:/*LD (IY+d),C*/{int8_t d;_MR(c.PC++,d);;a=c.WZ=c.IY+d;}_T(5);_MW(a,c.C);break;
          case 0x72:/*LD (IY+d),D*/{int8_t d;_MR(c.PC++,d);;a=c.WZ=c.IY+d;}_T(5);_MW(a,c.D);break;
          case 0x73:/*LD (IY+d),E*/{int8_t d;_MR(c.PC++,d);;a=c.WZ=c.IY+d;}_T(5);_MW(a,c.E);break;
          case 0x74:/*LD (IY+d),H*/{int8_t d;_MR(c.PC++,d);;a=c.WZ=c.IY+d;}_T(5);_MW(a,c.H);break;
          case 0x75:/*LD (IY+d),L*/{int8_t d;_MR(c.PC++,d);;a=c.WZ=c.IY+d;}_T(5);_MW(a,c.L);break;
          case 0x76:/*HALT*/_ON(Z80_HALT);c.PC--;break;
          case 0x77:/*LD (IY+d),A*/{int8_t d;_MR(c.PC++,d);;a=c.WZ=c.IY+d;}_T(5);_MW(a,c.A);break;
          case 0x78:/*LD A,B*/c.A=c.B;break;
          case 0x79:/*LD A,C*/c.A=c.C;break;
          case 0x7a:/*LD A,D*/c.A=c.D;break;
          case 0x7b:/*LD A,E*/c.A=c.E;break;
          case 0x7c:/*LD A,IYH*/c.A=c.IYH;break;
          case 0x7d:/*LD A,IYL*/c.A=c.IYL;break;
          case 0x7e:/*LD A,(IY+d)*/{int8_t d;_MR(c.PC++,d);;a=c.WZ=c.IY+d;}_T(5);_MR(a,c.A);break;
          case 0x7f:/*LD A,A*/c.A=c.A;break;
          case 0x80:/*ADD B*/{int res=c.A+c.B;c.F=_ADD_FLAGS(c.A,c.B,res);c.A=(uint8_t)res;}break;
          case 0x81:/*ADD C*/{int res=c.A+c.C;c.F=_ADD_FLAGS(c.A,c.C,res);c.A=(uint8_t)res;}break;
          case 0x82:/*ADD D*/{int res=c.A+c.D;c.F=_ADD_FLAGS(c.A,c.D,res);c.A=(uint8_t)res;}break;
          case 0x83:/*ADD E*/{int res=c.A+c.E;c.F=_ADD_FLAGS(c.A,c.E,res);c.A=(uint8_t)res;}break;
          case 0x84:/*ADD IYH*/{int res=c.A+c.IYH;c.F=_ADD_FLAGS(c.A,c.IYH,res);c.A=(uint8_t)res;}break;
          case 0x85:/*ADD IYL*/{int res=c.A+c.IYL;c.F=_ADD_FLAGS(c.A,c.IYL,res);c.A=(uint8_t)res;}break;
          case 0x86:/*ADD (IY+d)*/{int8_t d;_MR(c.PC++,d);;a=c.WZ=c.IY+d;}_T(5);_MR(a,v);{int res=c.A+v;c.F=_ADD_FLAGS(c.A,v,res);c.A=(uint8_t)res;}break;
          case 0x87:/*ADD A*/{int res=c.A+c.A;c.F=_ADD_FLAGS(c.A,c.A,res);c.A=(uint8_t)res;}break;
          case 0x88:/*ADC B*/{int res=c.A+c.B+(c.F&Z80_CF);c.F=_ADD_FLAGS(c.A,c.B,res);c.A=(uint8_t)res;}break;
          case 0x89:/*ADC C*/{int res=c.A+c.C+(c.F&Z80_CF);c.F=_ADD_FLAGS(c.A,c.C,res);c.A=(uint8_t)res;}break;
          case 0x8a:/*ADC D*/{int res=c.A+c.D+(c.F&Z80_CF);c.F=_ADD_FLAGS(c.A,c.D,res);c.A=(uint8_t)res;}break;
          case 0x8b:/*ADC E*/{int res=c.A+c.E+(c.F&Z80_CF);c.F=_ADD_FLAGS(c.A,c.E,res);c.A=(uint8_t)res;}break;
          case 0x8c:/*ADC IYH*/{int res=c.A+c.IYH+(c.F&Z80_CF);c.F=_ADD_FLAGS(c.A,c.IYH,res);c.A=(uint8_t)res;}break;
          case 0x8d:/*ADC IYL*/{int res=c.A+c.IYL+(c.F&Z80_CF);c.F=_ADD_FLAGS(c.A,c.IYL,res);c.A=(uint8_t)res;}break;
          case 0x8e:/*ADC (IY+d)*/{int8_t d;_MR(c.PC++,d);;a=c.WZ=c.IY+d;}_T(5);_MR(a,v);{int res=c.A+v+(c.F&Z80_CF);c.F=_ADD_FLAGS(c.A,v,res);c.A=(uint8_t)res;}break;
          case 0x8f:/*ADC A*/{int res=c.A+c.A+(c.F&Z80_CF);c.F=_ADD_FLAGS(c.A,c.A,res);c.A=(uint8_t)res;}break;
          case 0x90:/*SUB B*/{int res=(int)c.A-(int)c.B;c.F=_SUB_FLAGS(c.A,c.B,res);c.A=(uint8_t)res;}break;
          case 0x91:/*SUB C*/{int res=(int)c.A-(int)c.C;c.F=_SUB_FLAGS(c.A,c.C,res);c.A=(uint8_t)res;}break;
          case 0x92:/*SUB D*/{int res=(int)c.A-(int)c.D;c.F=_SUB_FLAGS(c.A,c.D,res);c.A=(uint8_t)res;}break;
          case 0x93:/*SUB E*/{int res=(int)c.A-(int)c.E;c.F=_SUB_FLAGS(c.A,c.E,res);c.A=(uint8_t)res;}break;
          case 0x94:/*SUB IYH*/{int res=(int)c.A-(int)c.IYH;c.F=_SUB_FLAGS(c.A,c.IYH,res);c.A=(uint8_t)res;}break;
          case 0x95:/*SUB IYL*/{int res=(int)c.A-(int)c.IYL;c.F=_SUB_FLAGS(c.A,c.IYL,res);c.A=(uint8_t)res;}break;
          case 0x96:/*SUB (IY+d)*/{int8_t d;_MR(c.PC++,d);;a=c.WZ=c.IY+d;}_T(5);_MR(a,v);{int res=(int)c.A-(int)v;c.F=_SUB_FLAGS(c.A,v,res);c.A=(uint8_t)res;}break;
          case 0x97:/*SUB A*/{int res=(int)c.A-(int)c.A;c.F=_SUB_FLAGS(c.A,c.A,res);c.A=(uint8_t)res;}break;
          case 0x98:/*SBC B*/{int res=(int)c.A-(int)c.B-(c.F&Z80_CF);c.F=_SUB_FLAGS(c.A,c.B,res);c.A=(uint8_t)res;}break;
          case 0x99:/*SBC C*/{int res=(int)c.A-(int)c.C-(c.F&Z80_CF);c.F=_SUB_FLAGS(c.A,c.C,res);c.A=(uint8_t)res;}break;
          case 0x9a:/*SBC D*/{int res=(int)c.A-(int)c.D-(c.F&Z80_CF);c.F=_SUB_FLAGS(c.A,c.D,res);c.A=(uint8_t)res;}break;
          case 0x9b:/*SBC E*/{int res=(int)c.A-(int)c.E-(c.F&Z80_CF);c.F=_SUB_FLAGS(c.A,c.E,res);c.A=(uint8_t)res;}break;
          case 0x9c:/*SBC IYH*/{int res=(int)c.A-(int)c.IYH-(c.F&Z80_CF);c.F=_SUB_FLAGS(c.A,c.IYH,res);c.A=(uint8_t)res;}break;
          case 0x9d:/*SBC IYL*/{int res=(int)c.A-(int)c.IYL-(c.F&Z80_CF);c.F=_SUB_FLAGS(c.A,c.IYL,res);c.A=(uint8_t)res;}break;
          case 0x9e:/*SBC (IY+d)*/{int8_t d;_MR(c.PC++,d);;a=c.WZ=c.IY+d;}_T(5);_MR(a,v);{int res=(int)c.A-(int)v-(c.F&Z80_CF);c.F=_SUB_FLAGS(c.A,v,res);c.A=(uint8_t)res;}break;
          case 0x9f:/*SBC A*/{int res=(int)c.A-(int)c.A-(c.F&Z80_CF);c.F=_SUB_FLAGS(c.A,c.A,res);c.A=(uint8_t)res;}break;
          case 0xa0:/*AND B*/c.A&=c.B;c.F=_z80_szp[c.A]|Z80_HF;break;
          case 0xa1:/*AND C*/c.A&=c.C;c.F=_z80_szp[c.A]|Z80_HF;break;
          case 0xa2:/*AND D*/c.A&=c.D;c.F=_z80_szp[c.A]|Z80_HF;break;
          case 0xa3:/*AND E*/c.A&=c.E;c.F=_z80_szp[c.A]|Z80_HF;break;
          case 0xa4:/*AND IYH*/c.A&=c.IYH;c.F=_z80_szp[c.A]|Z80_HF;break;
          case 0xa5:/*AND IYL*/c.A&=c.IYL;c.F=_z80_szp[c.A]|Z80_HF;break;
          case 0xa6:/*AND (IY+d)*/{int8_t d;_MR(c.PC++,d);;a=c.WZ=c.IY+d;}_T(5);_MR(a,v);c.A&=v;c.F=_z80_szp[c.A]|Z80_HF;break;
          case 0xa7:/*AND A*/c.A&=c.A;c.F=_z80_szp[c.A]|Z80_HF;break;
          case 0xa8:/*XOR B*/c.A^=c.B;c.F=_z80_szp[c.A];break;
          case 0xa9:/*XOR C*/c.A^=c.C;c.F=_z80_szp[c.A];break;
          case 0xaa:/*XOR D*/c.A^=c.D;c.F=_z80_szp[c.A];break;
          case 0xab:/*XOR E*/c.A^=c.E;c.F=_z80_szp[c.A];break;
          case 0xac:/*XOR IYH*/c.A^=c.IYH;c.F=_z80_szp[c.A];break;
          case 0xad:/*XOR IYL*/c.A^=c.IYL;c.F=_z80_szp[c.A];break;
          case 0xae:/*XOR (IY+d)*/{int8_t d;_MR(c.PC++,d);;a=c.WZ=c.IY+d;}_T(5);_MR(a,v);c.A^=v;c.F=_z80_szp[c.A];break;
          case 0xaf:/*XOR A*/c.A^=c.A;c.F=_z80_szp[c.A];break;
          case 0xb0:/*OR B*/c.A|=c.B;c.F=_z80_szp[c.A];break;
          case 0xb1:/*OR C*/c.A|=c.C;c.F=_z80_szp[c.A];break;
          case 0xb2:/*OR D*/c.A|=c.D;c.F=_z80_szp[c.A];break;
          case 0xb3:/*OR E*/c.A|=c.E;c.F=_z80_szp[c.A];break;
          case 0xb4:/*OR IYH*/c.A|=c.IYH;c.F=_z80_szp[c.A];break;
          case 0xb5:/*OR IYL*/c.A|=c.IYL;c.F=_z80_szp[c.A];break;
          case 0xb6:/*OR (IY+d)*/{int8_t d;_MR(c.PC++,d);;a=c.WZ=c.IY+d;}_T(5);_MR(a,v);c.A|=v;c.F=_z80_szp[c.A];break;
          case 0xb7:/*OR A*/c.A|=c.A;c.F=_z80_szp[c.A];break;
          case 0xb8:/*CP B*/{int res=(int)c.A-(int)c.B;c.F=_CP_FLAGS(c.A,c.B,res);}break;
          case 0xb9:/*CP C*/{int res=(int)c.A-(int)c.C;c.F=_CP_FLAGS(c.A,c.C,res);}break;
          case 0xba:/*CP D*/{int res=(int)c.A-(int)c.D;c.F=_CP_FLAGS(c.A,c.D,res);}break;
          case 0xbb:/*CP E*/{int res=(int)c.A-(int)c.E;c.F=_CP_FLAGS(c.A,c.E,res);}break;
          case 0xbc:/*CP IYH*/{int res=(int)c.A-(int)c.IYH;c.F=_CP_FLAGS(c.A,c.IYH,res);}break;
          case 0xbd:/*CP IYL*/{int res=(int)c.A-(int)c.IYL;c.F=_CP_FLAGS(c.A,c.IYL,res);}break;
          case 0xbe:/*CP (IY+d)*/{int8_t d;_MR(c.PC++,d);;a=c.WZ=c.IY+d;}_T(5);_MR(a,v);{int res=(int)c.A-(int)v;c.F=_CP_FLAGS(c.A,v,res);}break;
          case 0xbf:/*CP A*/{int res=(int)c.A-(int)c.A;c.F=_CP_FLAGS(c.A,c.A,res);}break;
          case 0xc0:/*RET NZ*/_T(1);if(!(c.F&Z80_ZF)){uint8_t w,z;_MR(c.SP++,z);_MR(c.SP++,w);c.PC=c.WZ=(w<<8)|z;}break;
          case 0xc1:/*POP BC*/{uint8_t l,h;_MR(c.SP++,l);_MR(c.SP++,h);c.BC=(h<<8)|l;}break;
          case 0xc2:/*JP NZ,nn*/_IMM16();if (!(c.F&Z80_ZF)) { c.PC=c.WZ; }break;
          case 0xc3:/*JP nn*/_IMM16();c.PC=c.WZ;break;
          case 0xc4:/*CALL NZ,nn*/_IMM16();if (!(c.F&Z80_ZF)){_T(1);_MW(--c.SP,(uint8_t)(c.PC>>8));_MW(--c.SP,(uint8_t)c.PC);c.PC=c.WZ;}break;
          case 0xc5:/*PUSH BC*/_T(1);_MW(--c.SP,(uint8_t)(c.BC>>8));_MW(--c.SP,(uint8_t)c.BC);break;
          case 0xc6:/*ADD n*/_MR(c.PC++,v);{int res=c.A+v;c.F=_ADD_FLAGS(c.A,v,res);c.A=(uint8_t)res;}break;
          case 0xc7:/*RST 0x0*/_MW(--c.SP,(uint8_t)(c.PC>>8));_MW(--c.SP,(uint8_t)c.PC);c.WZ=c.PC=(uint16_t)0x0;break;
          case 0xc8:/*RET Z*/_T(1);if((c.F&Z80_ZF)){uint8_t w,z;_MR(c.SP++,z);_MR(c.SP++,w);c.PC=c.WZ=(w<<8)|z;}break;
          case 0xc9:/*RET*/{uint8_t w,z;_MR(c.SP++,z);_MR(c.SP++,w);c.PC=c.WZ=(w<<8)|z;}break;
          case 0xca:/*JP Z,nn*/_IMM16();if ((c.F&Z80_ZF)) { c.PC=c.WZ; }break;
          case 0xcb:
            {
            int8_t d;_MR(c.PC++,d);
            _FETCH_CB(opcode);
            switch (opcode) {
              default:
                  {
                    uint8_t* vptr;
                    switch (opcode&7) {
                      case 0: vptr=&c.B; break;
                      case 1: vptr=&c.C; break;
                      case 2: vptr=&c.D; break;
                      case 3: vptr=&c.E; break;
                      case 4: vptr=&c.H; break;
                      case 5: vptr=&c.L; break;
                      case 6: vptr=0; break;
                      case 7: vptr=&c.A; break;
                    }
                    uint8_t y=(opcode>>3)&7;
                    switch (opcode>>6) {
                      case 0:
                        /* ROT n,r */
                        if (vptr) {
                          switch (y) {
                            case 0:/*RLC (IY+d),r*/a=c.WZ=c.IY+d;_T(2);_MR(a,v);{uint8_t r=v<<1|v>>7;c.F=_z80_szp[r]|(v>>7&Z80_CF);v=r;}*vptr=v;_MW(a,v);break;
                            case 1:/*RRC (IY+d),r*/a=c.WZ=c.IY+d;_T(2);_MR(a,v);{uint8_t r=v>>1|v<<7;c.F=_z80_szp[r]|(v&Z80_CF);v=r;}*vptr=v;_MW(a,v);break;
                            case 2:/*RL  (IY+d),r*/a=c.WZ=c.IY+d;_T(2);_MR(a,v);{uint8_t r=v<<1|(c.F&Z80_CF);c.F=(v>>7&Z80_CF)|_z80_szp[r];v=r;}*vptr=v;_MW(a,v);break;
                            case 3:/*RR  (IY+d),r*/a=c.WZ=c.IY+d;_T(2);_MR(a,v);{uint8_t r=v>>1|((c.F & Z80_CF)<<7);c.F=(v&Z80_CF)|_z80_szp[r];v=r;}*vptr=v;_MW(a,v);break;
                            case 4:/*SLA (IY+d),r*/a=c.WZ=c.IY+d;_T(2);_MR(a,v);{uint8_t r=v<<1;c.F=(v>>7&Z80_CF)|_z80_szp[r];v=r;}*vptr=v;_MW(a,v);break;
                            case 5:/*SRA (IY+d),r*/a=c.WZ=c.IY+d;_T(2);_MR(a,v);{uint8_t r=v>>1|(v&0x80);c.F=(v&Z80_CF)|_z80_szp[r];v=r;}*vptr=v;_MW(a,v);break;
                            case 6:/*SLL (IY+d),r*/a=c.WZ=c.IY+d;_T(2);_MR(a,v);{uint8_t r=(v<<1)|1;c.F=(v>>7&Z80_CF)|_z80_szp[r];v=r;}*vptr=v;_MW(a,v);break;
                            case 7:/*SRL (IY+d),r*/a=c.WZ=c.IY+d;_T(2);_MR(a,v);{uint8_t r=v>>1;c.F=(v&Z80_CF)|_z80_szp[r];v=r;}*vptr=v;_MW(a,v);break;
                          }
                        } else {
                          switch (y) {
                            case 0:/*RLC (IY+d)*/a=c.WZ=c.IY+d;_T(2);_MR(a,v);{uint8_t r=v<<1|v>>7;c.F=_z80_szp[r]|(v>>7&Z80_CF);v=r;}_MW(a,v);break;
                            case 1:/*RRC (IY+d)*/a=c.WZ=c.IY+d;_T(2);_MR(a,v);{uint8_t r=v>>1|v<<7;c.F=_z80_szp[r]|(v&Z80_CF);v=r;}_MW(a,v);break;
                            case 2:/*RL  (IY+d)*/a=c.WZ=c.IY+d;_T(2);_MR(a,v);{uint8_t r=v<<1|(c.F&Z80_CF);c.F=(v>>7&Z80_CF)|_z80_szp[r];v=r;}_MW(a,v);break;
                            case 3:/*RR  (IY+d)*/a=c.WZ=c.IY+d;_T(2);_MR(a,v);{uint8_t r=v>>1|((c.F & Z80_CF)<<7);c.F=(v&Z80_CF)|_z80_szp[r];v=r;}_MW(a,v);break;
                            case 4:/*SLA (IY+d)*/a=c.WZ=c.IY+d;_T(2);_MR(a,v);{uint8_t r=v<<1;c.F=(v>>7&Z80_CF)|_z80_szp[r];v=r;}_MW(a,v);break;
                            case 5:/*SRA (IY+d)*/a=c.WZ=c.IY+d;_T(2);_MR(a,v);{uint8_t r=v>>1|(v&0x80);c.F=(v&Z80_CF)|_z80_szp[r];v=r;}_MW(a,v);break;
                            case 6:/*SLL (IY+d)*/a=c.WZ=c.IY+d;_T(2);_MR(a,v);{uint8_t r=(v<<1)|1;c.F=(v>>7&Z80_CF)|_z80_szp[r];v=r;}_MW(a,v);break;
                            case 7:/*SRL (IY+d)*/a=c.WZ=c.IY+d;_T(2);_MR(a,v);{uint8_t r=v>>1;c.F=(v&Z80_CF)|_z80_szp[r];v=r;}_MW(a,v);break;
                          }
                        }
                        break;
                      case 1:
                        /* BIT n,(IX|IY+d) */
                        a=c.WZ=c.IY+d;_T(2);_MR(a,v);v&=(1<<y);f=Z80_HF|(v?(v&Z80_SF):(Z80_ZF|Z80_PF))|((c.WZ>>8)&(Z80_YF|Z80_XF));c.F=f|(c.F&Z80_CF);
                        break;
                      case 2:
                        if (vptr) {
                          /* RES n,(IX|IY+d),r (undocumented) */
                          a=c.WZ=c.IY+d;_T(2);_MR(a,v);*vptr=v&~(1<<y);_MW(a,*vptr);
                        } else {
                          /* RES n,(IX|IY+d) */
                          a=c.WZ=c.IY+d;_T(2);_MR(a,v);_MW(a,v&~(1<<y));
                        }
                        break;
                      case 3:
                        if (vptr) {
                          /* SET n,(IX|IY+d),r (undocumented) */
                          a=c.WZ=c.IY+d;_T(2);_MR(a,v);*vptr=v|(1<<y);_MW(a,*vptr);
                        } else {
                          /* RES n,(IX|IY+d) */
                          a=c.WZ=c.IY+d;_T(2);_MR(a,v);_MW(a,v|(1<<y));
                        }
                        break;
                    }
                  }
                break;
            } }
            break;
          case 0xcc:/*CALL Z,nn*/_IMM16();if ((c.F&Z80_ZF)){_T(1);_MW(--c.SP,(uint8_t)(c.PC>>8));_MW(--c.SP,(uint8_t)c.PC);c.PC=c.WZ;}break;
          case 0xcd:/*CALL nn*/_IMM16();_T(1);_MW(--c.SP,(uint8_t)(c.PC>>8));_MW(--c.SP,(uint8_t)c.PC);c.PC=c.WZ;break;
          case 0xce:/*ADC n*/_MR(c.PC++,v);{int res=c.A+v+(c.F&Z80_CF);c.F=_ADD_FLAGS(c.A,v,res);c.A=(uint8_t)res;}break;
          case 0xcf:/*RST 0x8*/_MW(--c.SP,(uint8_t)(c.PC>>8));_MW(--c.SP,(uint8_t)c.PC);c.WZ=c.PC=(uint16_t)0x8;break;
          case 0xd0:/*RET NC*/_T(1);if(!(c.F&Z80_CF)){uint8_t w,z;_MR(c.SP++,z);_MR(c.SP++,w);c.PC=c.WZ=(w<<8)|z;}break;
          case 0xd1:/*POP DE*/{uint8_t l,h;_MR(c.SP++,l);_MR(c.SP++,h);c.DE=(h<<8)|l;}break;
          case 0xd2:/*JP NC,nn*/_IMM16();if (!(c.F&Z80_CF)) { c.PC=c.WZ; }break;
          case 0xd3:/*OUT (n),A*/_MR(c.PC++,v);c.WZ=((c.A<<8)|v);_OUT(c.WZ,c.A);{uint8_t z=(uint8_t)c.WZ;z++;c.WZ=(c.WZ&0xFF00)|z;}break;
          case 0xd4:/*CALL NC,nn*/_IMM16();if (!(c.F&Z80_CF)){_T(1);_MW(--c.SP,(uint8_t)(c.PC>>8));_MW(--c.SP,(uint8_t)c.PC);c.PC=c.WZ;}break;
          case 0xd5:/*PUSH DE*/_T(1);_MW(--c.SP,(uint8_t)(c.DE>>8));_MW(--c.SP,(uint8_t)c.DE);break;
          case 0xd6:/*SUB n*/_MR(c.PC++,v);{int res=(int)c.A-(int)v;c.F=_SUB_FLAGS(c.A,v,res);c.A=(uint8_t)res;}break;
          case 0xd7:/*RST 0x10*/_MW(--c.SP,(uint8_t)(c.PC>>8));_MW(--c.SP,(uint8_t)c.PC);c.WZ=c.PC=(uint16_t)0x10;break;
          case 0xd8:/*RET C*/_T(1);if((c.F&Z80_CF)){uint8_t w,z;_MR(c.SP++,z);_MR(c.SP++,w);c.PC=c.WZ=(w<<8)|z;}break;
          case 0xd9:/*EXX*/{uint16_t tmp=c.BC;c.BC=c.BC_;c.BC_=tmp;}{uint16_t tmp=c.DE;c.DE=c.DE_;c.DE_=tmp;}{uint16_t tmp=c.HL;c.HL=c.HL_;c.HL_=tmp;}break;
          case 0xda:/*JP C,nn*/_IMM16();if ((c.F&Z80_CF)) { c.PC=c.WZ; }break;
          case 0xdb:/*IN A,(n)*/_MR(c.PC++,v);c.WZ=((c.A<<8)|v);_IN(c.WZ++,c.A);break;
          case 0xdc:/*CALL C,nn*/_IMM16();if ((c.F&Z80_CF)){_T(1);_MW(--c.SP,(uint8_t)(c.PC>>8));_MW(--c.SP,(uint8_t)c.PC);c.PC=c.WZ;}break;
          case 0xde:/*SBC n*/_MR(c.PC++,v);{int res=(int)c.A-(int)v-(c.F&Z80_CF);c.F=_SUB_FLAGS(c.A,v,res);c.A=(uint8_t)res;}break;
          case 0xdf:/*RST 0x18*/_MW(--c.SP,(uint8_t)(c.PC>>8));_MW(--c.SP,(uint8_t)c.PC);c.WZ=c.PC=(uint16_t)0x18;break;
          case 0xe0:/*RET PO*/_T(1);if(!(c.F&Z80_PF)){uint8_t w,z;_MR(c.SP++,z);_MR(c.SP++,w);c.PC=c.WZ=(w<<8)|z;}break;
          case 0xe1:/*POP IY*/{uint8_t l,h;_MR(c.SP++,l);_MR(c.SP++,h);c.IY=(h<<8)|l;}break;
          case 0xe2:/*JP PO,nn*/_IMM16();if (!(c.F&Z80_PF)) { c.PC=c.WZ; }break;
          case 0xe3:/*EX (SP),IY*/_T(1);{uint8_t w,z;_MR(c.SP,z);_MR(c.SP+1,w);_MW(c.SP,(uint8_t)c.IY);_MW(c.SP+1,(uint8_t)(c.IY>>8));c.IY=c.WZ=(w<<8)|z;}_T(2);break;
          case 0xe4:/*CALL PO,nn*/_IMM16();if (!(c.F&Z80_PF)){_T(1);_MW(--c.SP,(uint8_t)(c.PC>>8));_MW(--c.SP,(uint8_t)c.PC);c.PC=c.WZ;}break;
          case 0xe5:/*PUSH IY*/_T(1);_MW(--c.SP,(uint8_t)(c.IY>>8));_MW(--c.SP,(uint8_t)c.IY);break;
          case 0xe6:/*AND n*/_MR(c.PC++,v);c.A&=v;c.F=_z80_szp[c.A]|Z80_HF;break;
          case 0xe7:/*RST 0x20*/_MW(--c.SP,(uint8_t)(c.PC>>8));_MW(--c.SP,(uint8_t)c.PC);c.WZ=c.PC=(uint16_t)0x20;break;
          case 0xe8:/*RET PE*/_T(1);if((c.F&Z80_PF)){uint8_t w,z;_MR(c.SP++,z);_MR(c.SP++,w);c.PC=c.WZ=(w<<8)|z;}break;
          case 0xe9:/*JP IY*/c.PC=c.IY;break;
          case 0xea:/*JP PE,nn*/_IMM16();if ((c.F&Z80_PF)) { c.PC=c.WZ; }break;
          case 0xeb:/*EX DE,HL*/{uint16_t tmp=c.DE;c.DE=c.HL;c.HL=tmp;}break;
          case 0xec:/*CALL PE,nn*/_IMM16();if ((c.F&Z80_PF)){_T(1);_MW(--c.SP,(uint8_t)(c.PC>>8));_MW(--c.SP,(uint8_t)c.PC);c.PC=c.WZ;}break;
          case 0xee:/*XOR n*/_MR(c.PC++,v);c.A^=v;c.F=_z80_szp[c.A];break;
          case 0xef:/*RST 0x28*/_MW(--c.SP,(uint8_t)(c.PC>>8));_MW(--c.SP,(uint8_t)c.PC);c.WZ=c.PC=(uint16_t)0x28;break;
          case 0xf0:/*RET P*/_T(1);if(!(c.F&Z80_SF)){uint8_t w,z;_MR(c.SP++,z);_MR(c.SP++,w);c.PC=c.WZ=(w<<8)|z;}break;
          case 0xf1:/*POP AF*/{uint8_t l,h;_MR(c.SP++,l);_MR(c.SP++,h);c.AF=(h<<8)|l;}break;
          case 0xf2:/*JP P,nn*/_IMM16();if (!(c.F&Z80_SF)) { c.PC=c.WZ; }break;
          case 0xf3:/*DI*/c.IFF1=c.IFF2=false;break;
          case 0xf4:/*CALL P,nn*/_IMM16();if (!(c.F&Z80_SF)){_T(1);_MW(--c.SP,(uint8_t)(c.PC>>8));_MW(--c.SP,(uint8_t)c.PC);c.PC=c.WZ;}break;
          case 0xf5:/*PUSH AF*/_T(1);_MW(--c.SP,(uint8_t)(c.AF>>8));_MW(--c.SP,(uint8_t)c.AF);break;
          case 0xf6:/*OR n*/_MR(c.PC++,v);c.A|=v;c.F=_z80_szp[c.A];break;
          case 0xf7:/*RST 0x30*/_MW(--c.SP,(uint8_t)(c.PC>>8));_MW(--c.SP,(uint8_t)c.PC);c.WZ=c.PC=(uint16_t)0x30;break;
          case 0xf8:/*RET M*/_T(1);if((c.F&Z80_SF)){uint8_t w,z;_MR(c.SP++,z);_MR(c.SP++,w);c.PC=c.WZ=(w<<8)|z;}break;
          case 0xf9:/*LD SP,IY*/_T(2);c.SP=c.IY;break;
          case 0xfa:/*JP M,nn*/_IMM16();if ((c.F&Z80_SF)) { c.PC=c.WZ; }break;
          case 0xfb:/*EI*/c.ei_pending=true;break;
          case 0xfc:/*CALL M,nn*/_IMM16();if ((c.F&Z80_SF)){_T(1);_MW(--c.SP,(uint8_t)(c.PC>>8));_MW(--c.SP,(uint8_t)c.PC);c.PC=c.WZ;}break;
          case 0xfe:/*CP n*/_MR(c.PC++,v);{int res=(int)c.A-(int)v;c.F=_CP_FLAGS(c.A,v,res);}break;
          case 0xff:/*RST 0x38*/_MW(--c.SP,(uint8_t)(c.PC>>8));_MW(--c.SP,(uint8_t)c.PC);c.WZ=c.PC=(uint16_t)0x38;break;
          default:
            break;
        } }
        break;
      case 0xfe:/*CP n*/_MR(c.PC++,v);{int res=(int)c.A-(int)v;c.F=_CP_FLAGS(c.A,v,res);}break;
      case 0xff:/*RST 0x38*/_MW(--c.SP,(uint8_t)(c.PC>>8));_MW(--c.SP,(uint8_t)c.PC);c.WZ=c.PC=(uint16_t)0x38;break;
      default:
        break;
    } }
    if (((pins & (Z80_INT|Z80_BUSREQ))==Z80_INT) && c.IFF1) {
      c.IFF1=c.IFF2=false;
      if (pins & Z80_HALT) { pins &= ~Z80_HALT; c.PC++; }
      _ON(Z80_M1|Z80_IORQ);
      _SA(c.PC);
      _TW(4);
      const uint8_t int_vec=_GD();
      _OFF(Z80_M1|Z80_IORQ);
      c.R=(c.R&0x80)|((c.R+1)&0x7F);
      _T(2);
      if (c.IM==1) {
        _MW(--c.SP,(uint8_t)(c.PC>>8));
        _MW(--c.SP,(uint8_t)(c.PC));
        c.PC=c.WZ=0x0038;
      }
      else if (c.IM==2) {
        _MW(--c.SP,(uint8_t)(c.PC>>8));
        _MW(--c.SP,(uint8_t)(c.PC));
        a=(c.I<<8)|(int_vec&0xFE);
        {
          uint8_t w,z;
          _MR(a++,z);
          _MR(a,w);
          c.PC=c.WZ=(w<<8)|z;
        }
      } else {
        /*CHIPS_ASSERT(false);*/
      }
    }
    for (int i=0; i<Z80_MAX_NUM_TRAPS; i++) {
      if (cpu->trap_valid[i] && (c.PC==cpu->trap_addr[i])) {
        if (cpu->trap_func[i]) {
          if (cpu->trap_func[i]()) { trap_id=i; }
        } else {
          trap_id=i;
        }
      }
    }
  } while ((ticks < num_ticks) && (trap_id < 0));
  cpu->state = c;
  cpu->pins = pins;
  cpu->trap_id = trap_id;
  return ticks;
}
#undef _SA
#undef _SAD
#undef _GD
#undef _ON
#undef _OFF
#undef _T
#undef _TW
#undef _MR
#undef _MW
#undef _IN
#undef _OUT
#undef _FETCH
#undef _FETCH_CB
#undef _IMM16
#undef _ADD_FLAGS
#undef _SUB_FLAGS
#undef _CP_FLAGS
