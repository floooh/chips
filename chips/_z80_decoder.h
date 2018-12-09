uint32_t z80_exec(z80_t* cpu, uint32_t num_ticks) {
  uint8_t B=cpu->b, C=cpu->c;
  uint8_t D=cpu->d, E=cpu->e;
  uint8_t H=cpu->h, L=cpu->l;
  uint8_t A=cpu->a, F=cpu->f;
  uint8_t IXH=cpu->ixh, IXL=cpu->ixl;
  uint8_t IYH=cpu->iyh, IYL=cpu->iyl;
  uint16_t BC_=cpu->bc_, DE_=cpu->de_, HL_=cpu->hl_, AF_=cpu->af_;
  uint16_t WZ=cpu->wz, SP=cpu->sp, PC=cpu->pc;
  uint8_t IM=cpu->im, I=cpu->i, R=r;
  bool IFF1=cpu->iff1, IFF2=cpu->iff2, EI_PENDING=cpu->ei->pending;
  uint64_t pins = cpu->pins;
  const uint64_t trap_addr = cpu->trap_addr;
  const z80_tick_t tick = cpu->tick;
  void* ud = cpu->user_data;
  int trap_id = -1;
  uint32_t ticks = 0;
  uint8_t op, d8;
  uint16_t addr, d16;
  uint64_t pre_pins = pins;
  do {
    _FETCH(op)
    switch (op) {
      case 0x0:/*NOP*/ break;
      case 0x1:/*LD BC,nn*/_IMM16(d16);_S_BC(d16);break;
      case 0x2:/*LD (BC),A*/addr=_G_BC();d8=_G_A();_MW(addr++,d8);_S_WZ((d8<<8)|(addr&0x00FF));break;
      case 0x3:/*INC BC*/_T(2);_S_BC(_G_BC()+1);break;
      case 0x4:/*INC B*/d8=_G_B();{uint8_t r=d8+1;uint8_t f=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x80){f|=Z80_VF;}F=f|(F&Z80_CF);d8=r;}_S_B(d8);break;
      case 0x5:/*DEC B*/d8=_G_B();{uint8_t r=d8-1;uint8_t f=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x7F){f|=Z80_VF;}F=f|(F&Z80_CF);d8=r;}_S_B(d8);break;
      case 0x6:/*LD B,n*/_IMM8(d8);_S_B(d8);break;
      case 0x7:/*RLCA*/ws=_z80_rlca(ws);break;
      case 0x8:/*EX AF,AF'*/{uint16_t tmp=(F<<8)|A;A=_FA&0xFF;F=_FA>>8;FA_=tmp;}break;
      case 0x9:/*ADD HL,BC*/{uint16_t acc=(H<<8)|L;WZ=acc+1;d16=(B<<8)|C;uint32_t r=acc+d16;H=r>>8;L=r;uint8_t f=F&(Z80_SF|Z80_ZF|Z80_VF);f|=((acc^r^d16)>>8)&Z80_HF;f|=((r>>16)&Z80_CF)|((r>>8)&(Z80_YF|Z80_XF));F=f;_T(7);}break;
      case 0xa:/*LD A,(BC)*/addr=_G_BC();_MR(addr++,d8);_S_A(d8);_S_WZ(addr);break;
      case 0xb:/*DEC BC*/_T(2);_S_BC(_G_BC()-1);break;
      case 0xc:/*INC C*/d8=_G_C();{uint8_t r=d8+1;uint8_t f=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x80){f|=Z80_VF;}F=f|(F&Z80_CF);d8=r;}_S_C(d8);break;
      case 0xd:/*DEC C*/d8=_G_C();{uint8_t r=d8-1;uint8_t f=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x7F){f|=Z80_VF;}F=f|(F&Z80_CF);d8=r;}_S_C(d8);break;
      case 0xe:/*LD C,n*/_IMM8(d8);_S_C(d8);break;
      case 0xf:/*RRCA*/ws=_z80_rrca(ws);break;
      case 0x10:/*DJNZ*/{_T(1);int8_t d;_IMM8(d);B--;if(B>0){PC+=d;WZ=PC;_T(5);}}break;
      case 0x11:/*LD DE,nn*/_IMM16(d16);_S_DE(d16);break;
      case 0x12:/*LD (DE),A*/addr=_G_DE();d8=_G_A();_MW(addr++,d8);_S_WZ((d8<<8)|(addr&0x00FF));break;
      case 0x13:/*INC DE*/_T(2);_S_DE(_G_DE()+1);break;
      case 0x14:/*INC D*/d8=_G_D();{uint8_t r=d8+1;uint8_t f=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x80){f|=Z80_VF;}F=f|(F&Z80_CF);d8=r;}_S_D(d8);break;
      case 0x15:/*DEC D*/d8=_G_D();{uint8_t r=d8-1;uint8_t f=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x7F){f|=Z80_VF;}F=f|(F&Z80_CF);d8=r;}_S_D(d8);break;
      case 0x16:/*LD D,n*/_IMM8(d8);_S_D(d8);break;
      case 0x17:/*RLA*/ws=_z80_rla(ws);break;
      case 0x18:/*JR d*/{int8_t d;_IMM8(d);PC+=d;WZ=PC;_T(5);}break;
      case 0x19:/*ADD HL,DE*/{uint16_t acc=(H<<8)|L;WZ=acc+1;d16=(D<<8)|E;uint32_t r=acc+d16;H=r>>8;L=r;uint8_t f=F&(Z80_SF|Z80_ZF|Z80_VF);f|=((acc^r^d16)>>8)&Z80_HF;f|=((r>>16)&Z80_CF)|((r>>8)&(Z80_YF|Z80_XF));F=f;_T(7);}break;
      case 0x1a:/*LD A,(DE)*/addr=_G_DE();_MR(addr++,d8);_S_A(d8);_S_WZ(addr);break;
      case 0x1b:/*DEC DE*/_T(2);_S_DE(_G_DE()-1);break;
      case 0x1c:/*INC E*/d8=_G_E();{uint8_t r=d8+1;uint8_t f=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x80){f|=Z80_VF;}F=f|(F&Z80_CF);d8=r;}_S_E(d8);break;
      case 0x1d:/*DEC E*/d8=_G_E();{uint8_t r=d8-1;uint8_t f=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x7F){f|=Z80_VF;}F=f|(F&Z80_CF);d8=r;}_S_E(d8);break;
      case 0x1e:/*LD E,n*/_IMM8(d8);_S_E(d8);break;
      case 0x1f:/*RRA*/ws=_z80_rra(ws);break;
      case 0x20:/*JR NZ,d*/{int8_t d;_IMM8(d);if(!(_G_F()&Z80_ZF)){PC+=d;WZ=PC;_T(5);}}break;
      case 0x21:/*LD HL,nn*/_IMM16(d16);_S_HL(d16);break;
      case 0x22:/*LD (nn),HL*/_IMM16(addr);_MW(addr++,_G_L());_MW(addr,_G_H());_S_WZ(addr);break;
      case 0x23:/*INC HL*/_T(2);_S_HL(_G_HL()+1);break;
      case 0x24:/*INC H*/d8=_G_H();{uint8_t r=d8+1;uint8_t f=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x80){f|=Z80_VF;}F=f|(F&Z80_CF);d8=r;}_S_H(d8);break;
      case 0x25:/*DEC H*/d8=_G_H();{uint8_t r=d8-1;uint8_t f=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x7F){f|=Z80_VF;}F=f|(F&Z80_CF);d8=r;}_S_H(d8);break;
      case 0x26:/*LD H,n*/_IMM8(d8);_S_H(d8);break;
      case 0x27:/*DAA*/ws=_z80_daa(ws);break;
      case 0x28:/*JR Z,d*/{int8_t d;_IMM8(d);if((_G_F()&Z80_ZF)){PC+=d;WZ=PC;_T(5);}}break;
      case 0x29:/*ADD HL,HL*/{uint16_t acc=(H<<8)|L;WZ=acc+1;d16=(H<<8)|L;uint32_t r=acc+d16;H=r>>8;L=r;uint8_t f=F&(Z80_SF|Z80_ZF|Z80_VF);f|=((acc^r^d16)>>8)&Z80_HF;f|=((r>>16)&Z80_CF)|((r>>8)&(Z80_YF|Z80_XF));F=f;_T(7);}break;
      case 0x2a:/*LD HL,(nn)*/_IMM16(addr);_MR(addr++,d8);_S_L(d8);_MR(addr,d8);_S_H(d8);_S_WZ(addr);break;
      case 0x2b:/*DEC HL*/_T(2);_S_HL(_G_HL()-1);break;
      case 0x2c:/*INC L*/d8=_G_L();{uint8_t r=d8+1;uint8_t f=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x80){f|=Z80_VF;}F=f|(F&Z80_CF);d8=r;}_S_L(d8);break;
      case 0x2d:/*DEC L*/d8=_G_L();{uint8_t r=d8-1;uint8_t f=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x7F){f|=Z80_VF;}F=f|(F&Z80_CF);d8=r;}_S_L(d8);break;
      case 0x2e:/*LD L,n*/_IMM8(d8);_S_L(d8);break;
      case 0x2f:/*CPL*/ws=_z80_cpl(ws);break;
      case 0x30:/*JR NC,d*/{int8_t d;_IMM8(d);if(!(_G_F()&Z80_CF)){PC+=d;WZ=PC;_T(5);}}break;
      case 0x31:/*LD SP,nn*/_IMM16(d16);_S_SP(d16);break;
      case 0x32:/*LD (nn),A*/_IMM16(addr);d8=_G_A();_MW(addr++,d8);_S_WZ((d8<<8)|(addr&0x00FF));break;
      case 0x33:/*INC SP*/_T(2);_S_SP(_G_SP()+1);break;
      case 0x34:/*INC (HL/IX+d/IY+d)*/addr=(H<<8)|L;_T(1);_MR(addr,d8);{uint8_t r=d8+1;uint8_t f=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x80){f|=Z80_VF;}F=f|(F&Z80_CF);d8=r;}_MW(addr,d8);break;
      case 0x35:/*DEC (HL/IX+d/IY+d)*/addr=(H<<8)|L;_T(1);_MR(addr,d8);{uint8_t r=d8-1;uint8_t f=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x7F){f|=Z80_VF;}F=f|(F&Z80_CF);d8=r;}_MW(addr,d8);break;
      case 0x36:/*LD (HL/IX+d/IY+d),n*/addr=(H<<8)|L;_IMM8(d8);_MW(addr,d8);break;
      case 0x37:/*SCF*/ws=_z80_scf(ws);break;
      case 0x38:/*JR C,d*/{int8_t d;_IMM8(d);if((_G_F()&Z80_CF)){PC+=d;WZ=PC;_T(5);}}break;
      case 0x39:/*ADD HL,SP*/{uint16_t acc=(H<<8)|L;WZ=acc+1;d16=(S<<8)|P;uint32_t r=acc+d16;H=r>>8;L=r;uint8_t f=F&(Z80_SF|Z80_ZF|Z80_VF);f|=((acc^r^d16)>>8)&Z80_HF;f|=((r>>16)&Z80_CF)|((r>>8)&(Z80_YF|Z80_XF));F=f;_T(7);}break;
      case 0x3a:/*LD A,(nn)*/_IMM16(addr);_MR(addr++,d8);_S_A(d8);_S_WZ(addr);break;
      case 0x3b:/*DEC SP*/_T(2);_S_SP(_G_SP()-1);break;
      case 0x3c:/*INC A*/d8=_G_A();{uint8_t r=d8+1;uint8_t f=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x80){f|=Z80_VF;}F=f|(F&Z80_CF);d8=r;}_S_A(d8);break;
      case 0x3d:/*DEC A*/d8=_G_A();{uint8_t r=d8-1;uint8_t f=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x7F){f|=Z80_VF;}F=f|(F&Z80_CF);d8=r;}_S_A(d8);break;
      case 0x3e:/*LD A,n*/_IMM8(d8);_S_A(d8);break;
      case 0x3f:/*CCF*/ws=_z80_ccf(ws);break;
      case 0x40:/*LD B,B*/B=B;break;
      case 0x41:/*LD B,C*/B=C;break;
      case 0x42:/*LD B,D*/B=D;break;
      case 0x43:/*LD B,E*/B=E;break;
      case 0x44:/*LD B,H*/B=H;break;
      case 0x45:/*LD B,L*/B=L;break;
      case 0x46:/*LD B,(HL)*/addr=(H<<8)|L;_MR(addr,B);break;
      case 0x47:/*LD B,A*/B=A;break;
      case 0x48:/*LD C,B*/C=B;break;
      case 0x49:/*LD C,C*/C=C;break;
      case 0x4a:/*LD C,D*/C=D;break;
      case 0x4b:/*LD C,E*/C=E;break;
      case 0x4c:/*LD C,H*/C=H;break;
      case 0x4d:/*LD C,L*/C=L;break;
      case 0x4e:/*LD C,(HL)*/addr=(H<<8)|L;_MR(addr,C);break;
      case 0x4f:/*LD C,A*/C=A;break;
      case 0x50:/*LD D,B*/D=B;break;
      case 0x51:/*LD D,C*/D=C;break;
      case 0x52:/*LD D,D*/D=D;break;
      case 0x53:/*LD D,E*/D=E;break;
      case 0x54:/*LD D,H*/D=H;break;
      case 0x55:/*LD D,L*/D=L;break;
      case 0x56:/*LD D,(HL)*/addr=(H<<8)|L;_MR(addr,D);break;
      case 0x57:/*LD D,A*/D=A;break;
      case 0x58:/*LD E,B*/E=B;break;
      case 0x59:/*LD E,C*/E=C;break;
      case 0x5a:/*LD E,D*/E=D;break;
      case 0x5b:/*LD E,E*/E=E;break;
      case 0x5c:/*LD E,H*/E=H;break;
      case 0x5d:/*LD E,L*/E=L;break;
      case 0x5e:/*LD E,(HL)*/addr=(H<<8)|L;_MR(addr,E);break;
      case 0x5f:/*LD E,A*/E=A;break;
      case 0x60:/*LD H,B*/H=B;break;
      case 0x61:/*LD H,C*/H=C;break;
      case 0x62:/*LD H,D*/H=D;break;
      case 0x63:/*LD H,E*/H=E;break;
      case 0x64:/*LD H,H*/H=H;break;
      case 0x65:/*LD H,L*/H=L;break;
      case 0x66:/*LD H,(HL)*/addr=(H<<8)|L;_MR(addr,H);break;
      case 0x67:/*LD H,A*/H=A;break;
      case 0x68:/*LD L,B*/L=B;break;
      case 0x69:/*LD L,C*/L=C;break;
      case 0x6a:/*LD L,D*/L=D;break;
      case 0x6b:/*LD L,E*/L=E;break;
      case 0x6c:/*LD L,H*/L=H;break;
      case 0x6d:/*LD L,L*/L=L;break;
      case 0x6e:/*LD L,(HL)*/addr=(H<<8)|L;_MR(addr,L);break;
      case 0x6f:/*LD L,A*/L=A;break;
      case 0x70:/*LD (HL),B*/addr=(H<<8)|L;_MW(addr,B);break;
      case 0x71:/*LD (HL),C*/addr=(H<<8)|L;_MW(addr,C);break;
      case 0x72:/*LD (HL),D*/addr=(H<<8)|L;_MW(addr,D);break;
      case 0x73:/*LD (HL),E*/addr=(H<<8)|L;_MW(addr,E);break;
      case 0x74:/*LD (HL),H*/addr=(H<<8)|L;_MW(addr,H);break;
      case 0x75:/*LD (HL),L*/addr=(H<<8)|L;_MW(addr,L);break;
      case 0x76:/*HALT*/pins|=Z80_HALT;PC--;break;
      case 0x77:/*LD (HL),A*/addr=(H<<8)|L;_MW(addr,A);break;
      case 0x78:/*LD A,B*/A=B;break;
      case 0x79:/*LD A,C*/A=C;break;
      case 0x7a:/*LD A,D*/A=D;break;
      case 0x7b:/*LD A,E*/A=E;break;
      case 0x7c:/*LD A,H*/A=H;break;
      case 0x7d:/*LD A,L*/A=L;break;
      case 0x7e:/*LD A,(HL)*/addr=(H<<8)|L;_MR(addr,A);break;
      case 0x7f:/*LD A,A*/A=A;break;
      case 0x80:/*ADD B*/d8=B;{uint32_t res=A+d8;F=_ADD_FLAGS(A,d8,res);A=res;}break;
      case 0x81:/*ADD C*/d8=C;{uint32_t res=A+d8;F=_ADD_FLAGS(A,d8,res);A=res;}break;
      case 0x82:/*ADD D*/d8=D;{uint32_t res=A+d8;F=_ADD_FLAGS(A,d8,res);A=res;}break;
      case 0x83:/*ADD E*/d8=E;{uint32_t res=A+d8;F=_ADD_FLAGS(A,d8,res);A=res;}break;
      case 0x84:/*ADD H*/d8=H;{uint32_t res=A+d8;F=_ADD_FLAGS(A,d8,res);A=res;}break;
      case 0x85:/*ADD L*/d8=L;{uint32_t res=A+d8;F=_ADD_FLAGS(A,d8,res);A=res;}break;
      case 0x86:/*ADD,(HL)*/addr=(H<<8)|L;_MR(addr,d8);{uint32_t res=A+d8;F=_ADD_FLAGS(A,d8,res);A=res;}break;
      case 0x87:/*ADD A*/d8=A;{uint32_t res=A+d8;F=_ADD_FLAGS(A,d8,res);A=res;}break;
      case 0x88:/*ADC B*/d8=B;{uint32_t res=A+d8+(F&Z80_CF);F=_ADD_FLAGS(A,d8,res);A=res;}break;
      case 0x89:/*ADC C*/d8=C;{uint32_t res=A+d8+(F&Z80_CF);F=_ADD_FLAGS(A,d8,res);A=res;}break;
      case 0x8a:/*ADC D*/d8=D;{uint32_t res=A+d8+(F&Z80_CF);F=_ADD_FLAGS(A,d8,res);A=res;}break;
      case 0x8b:/*ADC E*/d8=E;{uint32_t res=A+d8+(F&Z80_CF);F=_ADD_FLAGS(A,d8,res);A=res;}break;
      case 0x8c:/*ADC H*/d8=H;{uint32_t res=A+d8+(F&Z80_CF);F=_ADD_FLAGS(A,d8,res);A=res;}break;
      case 0x8d:/*ADC L*/d8=L;{uint32_t res=A+d8+(F&Z80_CF);F=_ADD_FLAGS(A,d8,res);A=res;}break;
      case 0x8e:/*ADC,(HL)*/addr=(H<<8)|L;_MR(addr,d8);{uint32_t res=A+d8+(F&Z80_CF);F=_ADD_FLAGS(A,d8,res);A=res;}break;
      case 0x8f:/*ADC A*/d8=A;{uint32_t res=A+d8+(F&Z80_CF);F=_ADD_FLAGS(A,d8,res);A=res;}break;
      case 0x90:/*SUB B*/d8=B;{uint32_t res=(uint32_t)((int)A-(int)d8);F=_SUB_FLAGS(A,d8,res);A=res;}break;
      case 0x91:/*SUB C*/d8=C;{uint32_t res=(uint32_t)((int)A-(int)d8);F=_SUB_FLAGS(A,d8,res);A=res;}break;
      case 0x92:/*SUB D*/d8=D;{uint32_t res=(uint32_t)((int)A-(int)d8);F=_SUB_FLAGS(A,d8,res);A=res;}break;
      case 0x93:/*SUB E*/d8=E;{uint32_t res=(uint32_t)((int)A-(int)d8);F=_SUB_FLAGS(A,d8,res);A=res;}break;
      case 0x94:/*SUB H*/d8=H;{uint32_t res=(uint32_t)((int)A-(int)d8);F=_SUB_FLAGS(A,d8,res);A=res;}break;
      case 0x95:/*SUB L*/d8=L;{uint32_t res=(uint32_t)((int)A-(int)d8);F=_SUB_FLAGS(A,d8,res);A=res;}break;
      case 0x96:/*SUB,(HL)*/addr=(H<<8)|L;_MR(addr,d8);{uint32_t res=(uint32_t)((int)A-(int)d8);F=_SUB_FLAGS(A,d8,res);A=res;}break;
      case 0x97:/*SUB A*/d8=A;{uint32_t res=(uint32_t)((int)A-(int)d8);F=_SUB_FLAGS(A,d8,res);A=res;}break;
      case 0x98:/*SBC B*/d8=B;{uint32_t res=(uint32_t)((int)A-(int)d8-(F&Z80_CF));F=_SUB_FLAGS(A,d8,res));A=res;}break;
      case 0x99:/*SBC C*/d8=C;{uint32_t res=(uint32_t)((int)A-(int)d8-(F&Z80_CF));F=_SUB_FLAGS(A,d8,res));A=res;}break;
      case 0x9a:/*SBC D*/d8=D;{uint32_t res=(uint32_t)((int)A-(int)d8-(F&Z80_CF));F=_SUB_FLAGS(A,d8,res));A=res;}break;
      case 0x9b:/*SBC E*/d8=E;{uint32_t res=(uint32_t)((int)A-(int)d8-(F&Z80_CF));F=_SUB_FLAGS(A,d8,res));A=res;}break;
      case 0x9c:/*SBC H*/d8=H;{uint32_t res=(uint32_t)((int)A-(int)d8-(F&Z80_CF));F=_SUB_FLAGS(A,d8,res));A=res;}break;
      case 0x9d:/*SBC L*/d8=L;{uint32_t res=(uint32_t)((int)A-(int)d8-(F&Z80_CF));F=_SUB_FLAGS(A,d8,res));A=res;}break;
      case 0x9e:/*SBC,(HL)*/addr=(H<<8)|L;_MR(addr,d8);{uint32_t res=(uint32_t)((int)A-(int)d8-(F&Z80_CF));F=_SUB_FLAGS(A,d8,res));A=res;}break;
      case 0x9f:/*SBC A*/d8=A;{uint32_t res=(uint32_t)((int)A-(int)d8-(F&Z80_CF));F=_SUB_FLAGS(A,d8,res));A=res;}break;
      case 0xa0:/*AND B*/d8=B;{A&=d8;F=_z80_szp[A]|Z80_HF;}break;
      case 0xa1:/*AND C*/d8=C;{A&=d8;F=_z80_szp[A]|Z80_HF;}break;
      case 0xa2:/*AND D*/d8=D;{A&=d8;F=_z80_szp[A]|Z80_HF;}break;
      case 0xa3:/*AND E*/d8=E;{A&=d8;F=_z80_szp[A]|Z80_HF;}break;
      case 0xa4:/*AND H*/d8=H;{A&=d8;F=_z80_szp[A]|Z80_HF;}break;
      case 0xa5:/*AND L*/d8=L;{A&=d8;F=_z80_szp[A]|Z80_HF;}break;
      case 0xa6:/*AND,(HL)*/addr=(H<<8)|L;_MR(addr,d8);{A&=d8;F=_z80_szp[A]|Z80_HF;}break;
      case 0xa7:/*AND A*/d8=A;{A&=d8;F=_z80_szp[A]|Z80_HF;}break;
      case 0xa8:/*XOR B*/d8=B;{A^=d8^;F=_z80_szp[A];}break;
      case 0xa9:/*XOR C*/d8=C;{A^=d8^;F=_z80_szp[A];}break;
      case 0xaa:/*XOR D*/d8=D;{A^=d8^;F=_z80_szp[A];}break;
      case 0xab:/*XOR E*/d8=E;{A^=d8^;F=_z80_szp[A];}break;
      case 0xac:/*XOR H*/d8=H;{A^=d8^;F=_z80_szp[A];}break;
      case 0xad:/*XOR L*/d8=L;{A^=d8^;F=_z80_szp[A];}break;
      case 0xae:/*XOR,(HL)*/addr=(H<<8)|L;_MR(addr,d8);{A^=d8^;F=_z80_szp[A];}break;
      case 0xaf:/*XOR A*/d8=A;{A^=d8^;F=_z80_szp[A];}break;
      case 0xb0:/*OR B*/d8=B;{A|=d8;F=_z80_szp[A];}break;
      case 0xb1:/*OR C*/d8=C;{A|=d8;F=_z80_szp[A];}break;
      case 0xb2:/*OR D*/d8=D;{A|=d8;F=_z80_szp[A];}break;
      case 0xb3:/*OR E*/d8=E;{A|=d8;F=_z80_szp[A];}break;
      case 0xb4:/*OR H*/d8=H;{A|=d8;F=_z80_szp[A];}break;
      case 0xb5:/*OR L*/d8=L;{A|=d8;F=_z80_szp[A];}break;
      case 0xb6:/*OR,(HL)*/addr=(H<<8)|L;_MR(addr,d8);{A|=d8;F=_z80_szp[A];}break;
      case 0xb7:/*OR A*/d8=A;{A|=d8;F=_z80_szp[A];}break;
      case 0xb8:/*CP B*/d8=B;{int32_t res=(uint32_t)((int)A-(int)d8);F=_CP_FLAGS(A,d8,res));}break;
      case 0xb9:/*CP C*/d8=C;{int32_t res=(uint32_t)((int)A-(int)d8);F=_CP_FLAGS(A,d8,res));}break;
      case 0xba:/*CP D*/d8=D;{int32_t res=(uint32_t)((int)A-(int)d8);F=_CP_FLAGS(A,d8,res));}break;
      case 0xbb:/*CP E*/d8=E;{int32_t res=(uint32_t)((int)A-(int)d8);F=_CP_FLAGS(A,d8,res));}break;
      case 0xbc:/*CP H*/d8=H;{int32_t res=(uint32_t)((int)A-(int)d8);F=_CP_FLAGS(A,d8,res));}break;
      case 0xbd:/*CP L*/d8=L;{int32_t res=(uint32_t)((int)A-(int)d8);F=_CP_FLAGS(A,d8,res));}break;
      case 0xbe:/*CP,(HL)*/addr=(H<<8)|L;_MR(addr,d8);{int32_t res=(uint32_t)((int)A-(int)d8);F=_CP_FLAGS(A,d8,res));}break;
      case 0xbf:/*CP A*/d8=A;{int32_t res=(uint32_t)((int)A-(int)d8);F=_CP_FLAGS(A,d8,res));}break;
      case 0xc0:/*RET NZ*/_T(1);if (!(_G_F()&Z80_ZF)){uint8_t w,z;_MR(SP++,z);_MR(SP++,w);PC=(w<<8)|z;WZ=PC;}break;
      case 0xc1:/*POP BC*/_MR(SP++,B);_MR(SP++,C);break;
      case 0xc2:/*JP NZ,nn*/_IMM16(addr);if(!(_G_F()&Z80_ZF)){pc=addr;}break;
      case 0xc3:/*JP nn*/_IMM16(pc);break;
      case 0xc4:/*CALL NZ,nn*/_IMM16(addr);if(!(_G_F()&Z80_ZF)){_T(1);_MW(--SP,PC>>8);_MW(--SP,PC);PC=addr;}break;
      case 0xc5:/*PUSH BC*/_T(1);_MW(--SP,B);_MW(--SP,C);break;
      case 0xc6:/*ADD n*/_IMM8(d8);{uint32_t res=A+d8;F=_ADD_FLAGS(A,d8,res);A=res;}break;
      case 0xc7:/*RST 0x0*/_T(1);_MW(--SP,PC>>8);_MW(--SP,PC);PC=WZ=0x0;break;
      case 0xc8:/*RET Z*/_T(1);if ((_G_F()&Z80_ZF)){uint8_t w,z;_MR(SP++,z);_MR(SP++,w);PC=(w<<8)|z;WZ=PC;}break;
      case 0xc9:/*RET*/_MR(SP++,d8);PC=d8;_MR(SP++,d8);PC|=d8<<8;WZ=PC;break;
      case 0xca:/*JP Z,nn*/_IMM16(addr);if((_G_F()&Z80_ZF)){pc=addr;}break;
      case 0xCB: {
        /* special handling for undocumented DD/FD+CB double prefix instructions,
         these always load the value from memory (IX+d),
         and write the value back, even for normal
         "register" instructions
         see: http://www.baltazarstudios.com/files/ddcb.html
        */
        /* load the d offset for indexed instructions */
        int8_t d;
        if (_IDX()) { _IMM8(d); } else { d=0; }
        /* fetch opcode without memory refresh and incrementint R */
        _FETCH_CB(op);
        const uint8_t x = op>>6;
        const uint8_t y = (op>>3)&7;
        const uint8_t z = op&7;
        const int rz = (7-z)<<3;
        /* load the operand (for indexed ops, always from memory!) */
        if ((z == 6) || _IDX()) {
          _T(1);
          addr = _G_HL();
          if (_IDX()) {
            _T(1);
            addr += d;
            _S_WZ(addr);
          }
          _MR(addr,d8);
        }
        else {
          /* simple non-indexed, non-(HL): load register value */
          d8 = _G8(ws,rz);
        }
        uint8_t f = _G_F();
        uint8_t r;
        switch (x) {
          case 0:
             /* rot/shift */
             switch (y) {
               case 0: /*RLC*/ r=d8<<1|d8>>7; f=_z80_szp[r]|(d8>>7&Z80_CF); break;
               case 1: /*RRC*/ r=d8>>1|d8<<7; f=_z80_szp[r]|(d8&Z80_CF); break;
               case 2: /*RL */ r=d8<<1|(f&Z80_CF); f=_z80_szp[r]|(d8>>7&Z80_CF); break;
               case 3: /*RR */ r=d8>>1|((f&Z80_CF)<<7); f=_z80_szp[r]|(d8&Z80_CF); break;
               case 4: /*SLA*/ r=d8<<1; f=_z80_szp[r]|(d8>>7&Z80_CF); break;
               case 5: /*SRA*/ r=d8>>1|(d8&0x80); f=_z80_szp[r]|(d8&Z80_CF); break;
               case 6: /*SLL*/ r=d8<<1|1; f=_z80_szp[r]|(d8>>7&Z80_CF); break;
               case 7: /*SRL*/ r=d8>>1; f=_z80_szp[r]|(d8&Z80_CF); break;
             }
             break;
          case 1:
            /* BIT (bit test) */
            r = d8 & (1<<y);
            f = (f&Z80_CF) | Z80_HF | (r?(r&Z80_SF):(Z80_ZF|Z80_PF));
            if ((z == 6) || _IDX()) {
              f |= (_G_WZ()>>8) & (Z80_YF|Z80_XF);
            }
            else {
              f |= d8 & (Z80_YF|Z80_XF);
            }
            break;
          case 2:
            /* RES (bit clear) */
            r = d8 & ~(1<<y);
            break;
          case 3:
            /* SET (bit set) */
            r = d8 | (1<<y);
            break;
        }
        if (x != 1) {
          /* write result back */
          if ((z == 6) || _IDX()) {
            /* (HL), (IX+d), (IY+d): write back to memory, for extended ops,
               even when the op is actually a register op
            */
            _MW(addr,r);
          }
          if (z != 6) {
            /* write result back to register (special case for indexed + H/L! */
            if (_IDX() && ((z==4)||(z==5))) {
              _S8(r0,rz,r);
            }
            else {
              _S8(ws,rz,r);
            }
          }
        }
        _S_F(f);
      }
      break;
      case 0xcc:/*CALL Z,nn*/_IMM16(addr);if((_G_F()&Z80_ZF)){_T(1);_MW(--SP,PC>>8);_MW(--SP,PC);PC=addr;}break;
      case 0xcd:/*CALL nn*/_IMM16(addr);_T(1);_MW(--SP,PC>>8);_MW(--SP,PC);PC=addr;break;
      case 0xce:/*ADC n*/_IMM8(d8);{uint32_t res=A+d8+(F&Z80_CF);F=_ADD_FLAGS(A,d8,res);A=res;}break;
      case 0xcf:/*RST 0x8*/_T(1);_MW(--SP,PC>>8);_MW(--SP,PC);PC=WZ=0x8;break;
      case 0xd0:/*RET NC*/_T(1);if (!(_G_F()&Z80_CF)){uint8_t w,z;_MR(SP++,z);_MR(SP++,w);PC=(w<<8)|z;WZ=PC;}break;
      case 0xd1:/*POP DE*/_MR(SP++,D);_MR(SP++,E);break;
      case 0xd2:/*JP NC,nn*/_IMM16(addr);if(!(_G_F()&Z80_CF)){pc=addr;}break;
      case 0xd3:/*OUT (n),A*/{_IMM8(d8);addr=(A<<8)|d8;_OUT(addr,A);WZ=(addr&0xFF00)|((addr+1)&0x00FF);}break;
      case 0xd4:/*CALL NC,nn*/_IMM16(addr);if(!(_G_F()&Z80_CF)){_T(1);_MW(--SP,PC>>8);_MW(--SP,PC);PC=addr;}break;
      case 0xd5:/*PUSH DE*/_T(1);_MW(--SP,D);_MW(--SP,E);break;
      case 0xd6:/*SUB n*/_IMM8(d8);{uint32_t res=(uint32_t)((int)A-(int)d8);F=_SUB_FLAGS(A,d8,res);A=res;}break;
      case 0xd7:/*RST 0x10*/_T(1);_MW(--SP,PC>>8);_MW(--SP,PC);PC=WZ=0x10;break;
      case 0xd8:/*RET C*/_T(1);if ((_G_F()&Z80_CF)){uint8_t w,z;_MR(SP++,z);_MR(SP++,w);PC=(w<<8)|z;WZ=PC;}break;
      case 0xd9:/*EXX*/{uint16_t hl=(H<<8)|L;uint16_t de=(D<<8)|E;uint16_t bc=(B<<8)|C;H=HL_>>8;L=HL_;HL_=hl;D=DE_>>8;E=DE_;DE_=de;B=BC_>>8;C=BC_;BC_=bc;}break;
      case 0xda:/*JP C,nn*/_IMM16(addr);if((_G_F()&Z80_CF)){pc=addr;}break;
      case 0xdb:/*IN A,(n)*/{_IMM8(d8);WZ=(A<<8)|d8;_IN(WZ++,A);}break;
      case 0xdc:/*CALL C,nn*/_IMM16(addr);if((_G_F()&Z80_CF)){_T(1);_MW(--SP,PC>>8);_MW(--SP,PC);PC=addr;}break;
      case 0xdd:/*DD prefix*/map_bits|=_BIT_USE_IX;continue;break;
      case 0xde:/*SBC n*/_IMM8(d8);{uint32_t res=(uint32_t)((int)A-(int)d8-(F&Z80_CF));F=_SUB_FLAGS(A,d8,res));A=res;}break;
      case 0xdf:/*RST 0x18*/_T(1);_MW(--SP,PC>>8);_MW(--SP,PC);PC=WZ=0x18;break;
      case 0xe0:/*RET PO*/_T(1);if (!(_G_F()&Z80_PF)){uint8_t w,z;_MR(SP++,z);_MR(SP++,w);PC=(w<<8)|z;WZ=PC;}break;
      case 0xe1:/*POP HL*/_MR(SP++,H);_MR(SP++,L);break;
      case 0xe2:/*JP PO,nn*/_IMM16(addr);if(!(_G_F()&Z80_PF)){pc=addr;}break;
      case 0xe3:/*EX (SP),HL*/{_T(3);uint8_t l,h;_MR(SP,l);_MR(SP+1,h);_MW(SP,L);_MW(SP+1,H);HL=WZ=(h<<8)|l;}break;
      case 0xe4:/*CALL PO,nn*/_IMM16(addr);if(!(_G_F()&Z80_PF)){_T(1);_MW(--SP,PC>>8);_MW(--SP,PC);PC=addr;}break;
      case 0xe5:/*PUSH HL*/_T(1);_MW(--SP,H);_MW(--SP,L);break;
      case 0xe6:/*AND n*/_IMM8(d8);{A&=d8;F=_z80_szp[A]|Z80_HF;}break;
      case 0xe7:/*RST 0x20*/_T(1);_MW(--SP,PC>>8);_MW(--SP,PC);PC=WZ=0x20;break;
      case 0xe8:/*RET PE*/_T(1);if ((_G_F()&Z80_PF)){uint8_t w,z;_MR(SP++,z);_MR(SP++,w);PC=(w<<8)|z;WZ=PC;}break;
      case 0xe9:/*JP HL*/pc=_G_HL();break;
      case 0xea:/*JP PE,nn*/_IMM16(addr);if((_G_F()&Z80_PF)){pc=addr;}break;
      case 0xeb:/*EX DE,HL*/DE=DE^HL;HL=HL^DE;DE=DE^HL;break;
      case 0xec:/*CALL PE,nn*/_IMM16(addr);if((_G_F()&Z80_PF)){_T(1);_MW(--SP,PC>>8);_MW(--SP,PC);PC=addr;}break;
      case 0xED: {
        _FETCH(op);
        switch(op) {
          case 0x40:/*IN B,(C)*/{WZ=(B<<8)|C;_IN(WZ++,d8);F=(F&Z80_CF)|_z80_szp[d8];B=d8;}break;
          case 0x41:/*OUT (C),B*/WZ=(B<<8)|C;_OUT(WZ++,B);break;
          case 0x42:/*SBC HL,BC*/{uint16_t acc=(H<<8)|L;WZ=acc+1;d16=(B<<8)|C;uint32_t r=acc-d16-(F&Z80_CF);uint8_t f=Z80_NF|(((d16^acc)&(acc^r)&0x8000)>>13);H=r>>8;L=r;f|=((acc^r^d16)>>8) & Z80_HF;f|=(r>>16)&Z80_CF;f|=(r>>8)&(Z80_SF|Z80_YF|Z80_XF);f|=(r&0xFFFF)?0:Z80_ZF;F=f;_T(7);}break;
          case 0x43:/*LD (nn),BC*/_IMM16(addr);_MW(addr++,C);_MW(addr,B);_S_WZ(addr);break;
          case 0x44:/*NEG*/d8=A;A=0;{uint32_t res=(uint32_t)((int)A-(int)d8);F=_SUB_FLAGS(A,d8,res);A=res;}break;
          case 0x45:/*RETN*/pins|=Z80_RETI;_MR(SP++,d8);PC=d8;_MR(SP++,d8);PC|=d8<<8;WZ=PC;IFF1=IFF2;break;
          case 0x46:/*IM 0*/_S_IM(0);break;
          case 0x47:/*LD I,A*/_T(1);_S_I(_G_A());break;
          case 0x48:/*IN C,(C)*/{WZ=(B<<8)|C;_IN(WZ++,d8);F=(F&Z80_CF)|_z80_szp[d8];C=d8;}break;
          case 0x49:/*OUT (C),C*/WZ=(B<<8)|C;_OUT(WZ++,C);break;
          case 0x4a:/*ADC HL,BC*/{uint16_t acc=(H<<8)|L;WZ=acc+1;d16=(B<<8)|C;uint32_t r=acc+d16+(F&Z80_CF);H=r>>8;L=r;uint8_t f=((d16^acc^0x8000)&(d16^r)&0x8000)>>13;f|=((acc^r^d16)>>8)&Z80_HF;f|=(r>>16)&Z80_CF;f|=(r>>8)&(Z80_SF|Z80_YF|Z80_XF);f|=(r&0xFFFF)?0:Z80_ZF;F=f;_T(7);}break;
          case 0x4b:/*LD BC,(nn)*/_IMM16(addr);_MR(addr++,C);_MR(addr,B);_S_WZ(addr);break;
          case 0x4c:/*NEG*/d8=A;A=0;{uint32_t res=(uint32_t)((int)A-(int)d8);F=_SUB_FLAGS(A,d8,res);A=res;}break;
          case 0x4d:/*RETI*/pins|=Z80_RETI;_MR(SP++,d8);PC=d8;_MR(SP++,d8);PC|=d8<<8;WZ=PC;IFF1=IFF2;break;
          case 0x4e:/*IM 0*/_S_IM(0);break;
          case 0x4f:/*LD R,A*/_T(1);_S_R(_G_A());break;
          case 0x50:/*IN D,(C)*/{WZ=(B<<8)|C;_IN(WZ++,d8);F=(F&Z80_CF)|_z80_szp[d8];D=d8;}break;
          case 0x51:/*OUT (C),D*/WZ=(B<<8)|C;_OUT(WZ++,D);break;
          case 0x52:/*SBC HL,DE*/{uint16_t acc=(H<<8)|L;WZ=acc+1;d16=(D<<8)|E;uint32_t r=acc-d16-(F&Z80_CF);uint8_t f=Z80_NF|(((d16^acc)&(acc^r)&0x8000)>>13);H=r>>8;L=r;f|=((acc^r^d16)>>8) & Z80_HF;f|=(r>>16)&Z80_CF;f|=(r>>8)&(Z80_SF|Z80_YF|Z80_XF);f|=(r&0xFFFF)?0:Z80_ZF;F=f;_T(7);}break;
          case 0x53:/*LD (nn),DE*/_IMM16(addr);_MW(addr++,E);_MW(addr,D);_S_WZ(addr);break;
          case 0x54:/*NEG*/d8=A;A=0;{uint32_t res=(uint32_t)((int)A-(int)d8);F=_SUB_FLAGS(A,d8,res);A=res;}break;
          case 0x55:/*RETN*/pins|=Z80_RETI;_MR(SP++,d8);PC=d8;_MR(SP++,d8);PC|=d8<<8;WZ=PC;IFF1=IFF2;break;
          case 0x56:/*IM 1*/_S_IM(1);break;
          case 0x57:/*LD A,I*/_T(1);d8=_G_I();_S_A(d8);_S_F(_SZIFF2_FLAGS(d8));break;
          case 0x58:/*IN E,(C)*/{WZ=(B<<8)|C;_IN(WZ++,d8);F=(F&Z80_CF)|_z80_szp[d8];E=d8;}break;
          case 0x59:/*OUT (C),E*/WZ=(B<<8)|C;_OUT(WZ++,E);break;
          case 0x5a:/*ADC HL,DE*/{uint16_t acc=(H<<8)|L;WZ=acc+1;d16=(D<<8)|E;uint32_t r=acc+d16+(F&Z80_CF);H=r>>8;L=r;uint8_t f=((d16^acc^0x8000)&(d16^r)&0x8000)>>13;f|=((acc^r^d16)>>8)&Z80_HF;f|=(r>>16)&Z80_CF;f|=(r>>8)&(Z80_SF|Z80_YF|Z80_XF);f|=(r&0xFFFF)?0:Z80_ZF;F=f;_T(7);}break;
          case 0x5b:/*LD DE,(nn)*/_IMM16(addr);_MR(addr++,E);_MR(addr,D);_S_WZ(addr);break;
          case 0x5c:/*NEG*/d8=A;A=0;{uint32_t res=(uint32_t)((int)A-(int)d8);F=_SUB_FLAGS(A,d8,res);A=res;}break;
          case 0x5d:/*RETN*/pins|=Z80_RETI;_MR(SP++,d8);PC=d8;_MR(SP++,d8);PC|=d8<<8;WZ=PC;IFF1=IFF2;break;
          case 0x5e:/*IM 2*/_S_IM(2);break;
          case 0x5f:/*LD A,R*/_T(1);d8=_G_R();_S_A(d8);_S_F(_SZIFF2_FLAGS(d8));break;
          case 0x60:/*IN H,(C)*/{WZ=(B<<8)|C;_IN(WZ++,d8);F=(F&Z80_CF)|_z80_szp[d8];H=d8;}break;
          case 0x61:/*OUT (C),H*/WZ=(B<<8)|C;_OUT(WZ++,H);break;
          case 0x62:/*SBC HL,HL*/{uint16_t acc=(H<<8)|L;WZ=acc+1;d16=(H<<8)|L;uint32_t r=acc-d16-(F&Z80_CF);uint8_t f=Z80_NF|(((d16^acc)&(acc^r)&0x8000)>>13);H=r>>8;L=r;f|=((acc^r^d16)>>8) & Z80_HF;f|=(r>>16)&Z80_CF;f|=(r>>8)&(Z80_SF|Z80_YF|Z80_XF);f|=(r&0xFFFF)?0:Z80_ZF;F=f;_T(7);}break;
          case 0x63:/*LD (nn),HL*/_IMM16(addr);_MW(addr++,L);_MW(addr,H);_S_WZ(addr);break;
          case 0x64:/*NEG*/d8=A;A=0;{uint32_t res=(uint32_t)((int)A-(int)d8);F=_SUB_FLAGS(A,d8,res);A=res;}break;
          case 0x65:/*RETN*/pins|=Z80_RETI;_MR(SP++,d8);PC=d8;_MR(SP++,d8);PC|=d8<<8;WZ=PC;IFF1=IFF2;break;
          case 0x66:/*IM 0*/_S_IM(0);break;
          case 0x67:/*RRD*/{WZ=(H<<8)|L;_MR(WZ,d8);uint8_t l=A&0x0F;A=(A&0xF0)|(d8&0x0F);d8=(d8>>4)|(l<<4);_MW(WZ++,d8);F=(F&Z80_CF)|_z80_szp[A]);_T(4);}break;
          case 0x68:/*IN L,(C)*/{WZ=(B<<8)|C;_IN(WZ++,d8);F=(F&Z80_CF)|_z80_szp[d8];L=d8;}break;
          case 0x69:/*OUT (C),L*/WZ=(B<<8)|C;_OUT(WZ++,L);break;
          case 0x6a:/*ADC HL,HL*/{uint16_t acc=(H<<8)|L;WZ=acc+1;d16=(H<<8)|L;uint32_t r=acc+d16+(F&Z80_CF);H=r>>8;L=r;uint8_t f=((d16^acc^0x8000)&(d16^r)&0x8000)>>13;f|=((acc^r^d16)>>8)&Z80_HF;f|=(r>>16)&Z80_CF;f|=(r>>8)&(Z80_SF|Z80_YF|Z80_XF);f|=(r&0xFFFF)?0:Z80_ZF;F=f;_T(7);}break;
          case 0x6b:/*LD HL,(nn)*/_IMM16(addr);_MR(addr++,L);_MR(addr,H);_S_WZ(addr);break;
          case 0x6c:/*NEG*/d8=A;A=0;{uint32_t res=(uint32_t)((int)A-(int)d8);F=_SUB_FLAGS(A,d8,res);A=res;}break;
          case 0x6d:/*RETN*/pins|=Z80_RETI;_MR(SP++,d8);PC=d8;_MR(SP++,d8);PC|=d8<<8;WZ=PC;IFF1=IFF2;break;
          case 0x6e:/*IM 0*/_S_IM(0);break;
          case 0x6f:/*RLD*/{WZ=(H<<8)|L;_MR(WZ,d8);uint8_t l=A&0x0F;A=(A&0xF0)|(d8>>4);d8=(d8<<4)|l;_MW(WZ++,d8);F=(F&Z80_CF)|_z80_szp[A]);_T(4);}break;
          case 0x70:/*IN HL,(C)*/{WZ=(B<<8)|C;_IN(WZ++,d8);F=(F&Z80_CF)|_z80_szp[d8];}break;
          case 0x71:/*OUT (C),HL*/WZ=(B<<8)|C;_OUT(WZ++,0);break;
          case 0x72:/*SBC HL,SP*/{uint16_t acc=(H<<8)|L;WZ=acc+1;d16=(S<<8)|P;uint32_t r=acc-d16-(F&Z80_CF);uint8_t f=Z80_NF|(((d16^acc)&(acc^r)&0x8000)>>13);H=r>>8;L=r;f|=((acc^r^d16)>>8) & Z80_HF;f|=(r>>16)&Z80_CF;f|=(r>>8)&(Z80_SF|Z80_YF|Z80_XF);f|=(r&0xFFFF)?0:Z80_ZF;F=f;_T(7);}break;
          case 0x73:/*LD (nn),SP*/_IMM16(addr);_MW(addr++,P);_MW(addr,S);_S_WZ(addr);break;
          case 0x74:/*NEG*/d8=A;A=0;{uint32_t res=(uint32_t)((int)A-(int)d8);F=_SUB_FLAGS(A,d8,res);A=res;}break;
          case 0x75:/*RETN*/pins|=Z80_RETI;_MR(SP++,d8);PC=d8;_MR(SP++,d8);PC|=d8<<8;WZ=PC;IFF1=IFF2;break;
          case 0x76:/*IM 1*/_S_IM(1);break;
          case 0x77:/*NOP (ED)*/ break;
          case 0x78:/*IN A,(C)*/{WZ=(B<<8)|C;_IN(WZ++,d8);F=(F&Z80_CF)|_z80_szp[d8];A=d8;}break;
          case 0x79:/*OUT (C),A*/WZ=(B<<8)|C;_OUT(WZ++,A);break;
          case 0x7a:/*ADC HL,SP*/{uint16_t acc=(H<<8)|L;WZ=acc+1;d16=(S<<8)|P;uint32_t r=acc+d16+(F&Z80_CF);H=r>>8;L=r;uint8_t f=((d16^acc^0x8000)&(d16^r)&0x8000)>>13;f|=((acc^r^d16)>>8)&Z80_HF;f|=(r>>16)&Z80_CF;f|=(r>>8)&(Z80_SF|Z80_YF|Z80_XF);f|=(r&0xFFFF)?0:Z80_ZF;F=f;_T(7);}break;
          case 0x7b:/*LD SP,(nn)*/_IMM16(addr);_MR(addr++,P);_MR(addr,S);_S_WZ(addr);break;
          case 0x7c:/*NEG*/d8=A;A=0;{uint32_t res=(uint32_t)((int)A-(int)d8);F=_SUB_FLAGS(A,d8,res);A=res;}break;
          case 0x7d:/*RETN*/pins|=Z80_RETI;_MR(SP++,d8);PC=d8;_MR(SP++,d8);PC|=d8<<8;WZ=PC;IFF1=IFF2;break;
          case 0x7e:/*IM 2*/_S_IM(2);break;
          case 0x7f:/*NOP (ED)*/ break;
          case 0xa0:/*LDI*/{uint16_t hl=(H<<8)|L;uint16_t de=(D<<8)|E;_MR(hl,d8);_MW(de,d8);hl++;de++;H=hl>>8;L=hl;D=de>>8;E=de;_T(2);d8+=A;F&=Z80_SF|Z80_ZF|Z80_CF;if(d8&0x02){F|=Z80_YF;}if(d8&0x08){F|=Z80_XF;}uint16_t bc=(B<<8)|C;bc--;B=bc>>8;C=bc;if(bc){F|=Z80_VF;}}break;
          case 0xa1:/*CPI*/{uint16_t hl=(H<<8)|L;_MR(hl,d8);hl++;WZ++;H=hl>>8;L=hl;_T(5);int r=((int)A)-d8;F=(F&Z80_CF)|Z80_NF|_SZ(r);if((r&0x0F)>(A&0x0F)){F|=Z80_HF;r--;}if(r&0x02){F|=Z80_YF;}if(r&0x08){F|=Z80_XF;}uint16_t bc=(B<<8)|C;bc--;B=bc>>8;C=bc;if(bc){F|=Z80_VF;}}break;
          case 0xa2:/*INI*/{_T(1);WZ=(B<<8)|C;uint16_t hl=(H<<8)|L;_IN(WZ,d8);_MW(hl,d8);B--;WZ++;hl++;C++;H=hl>>8;L=hl;F=(B?(B&Z80_SF):Z80_ZF)|(B&(Z80_XF|Z80_YF));if(d8&Z80_SF){F|=Z80_NF;}uint32_t t=(uint32_t)(C+d8);if(t&0x100){F|=Z80_HF|Z80_CF;}F|=_z80_szp[((uint8_t)(t&0x07))^B]&Z80_PF;}break;
          case 0xa3:/*OUTI*/{_T(1);uint16_t hl=(H<<8)|L;_MR(hl,d8);B--;WZ=(B<<8)|C;_OUT(WZ,d8);WZ++; hl++;H=hl>>8;L=hl;F=(B?(B&Z80_SF):Z80_ZF)|(B&(Z80_XF|Z80_YF));if(d8&Z80_SF){F|=Z80_NF;}uint32_t t=(uint32_t)(L+d8);if (t&0x0100){F|=Z80_HF|Z80_CF;}F|=_z80_szp[((uint8_t)(t&0x07))^B]&Z80_PF;}break;
          case 0xa8:/*LDD*/{uint16_t hl=(H<<8)|L;uint16_t de=(D<<8)|E;_MR(hl,d8);_MW(de,d8);hl--;de--;H=hl>>8;L=hl;D=de>>8;E=de;_T(2);d8+=A;F&=Z80_SF|Z80_ZF|Z80_CF;if(d8&0x02){F|=Z80_YF;}if(d8&0x08){F|=Z80_XF;}uint16_t bc=(B<<8)|C;bc--;B=bc>>8;C=bc;if(bc){F|=Z80_VF;}}break;
          case 0xa9:/*CPD*/{uint16_t hl=(H<<8)|L;_MR(hl,d8);hl--;WZ--;H=hl>>8;L=hl;_T(5);int r=((int)A)-d8;F=(F&Z80_CF)|Z80_NF|_SZ(r);if((r&0x0F)>(A&0x0F)){F|=Z80_HF;r--;}if(r&0x02){F|=Z80_YF;}if(r&0x08){F|=Z80_XF;}uint16_t bc=(B<<8)|C;bc--;B=bc>>8;C=bc;if(bc){F|=Z80_VF;}}break;
          case 0xaa:/*IND*/{_T(1);WZ=(B<<8)|C;uint16_t hl=(H<<8)|L;_IN(WZ,d8);_MW(hl,d8);B--;WZ--;hl--;C--;H=hl>>8;L=hl;F=(B?(B&Z80_SF):Z80_ZF)|(B&(Z80_XF|Z80_YF));if(d8&Z80_SF){F|=Z80_NF;}uint32_t t=(uint32_t)(C+d8);if(t&0x100){F|=Z80_HF|Z80_CF;}F|=_z80_szp[((uint8_t)(t&0x07))^B]&Z80_PF;}break;
          case 0xab:/*OUTD*/{_T(1);uint16_t hl=(H<<8)|L;_MR(hl,d8);B--;WZ=(B<<8)|C;_OUT(WZ,d8);WZ--;hl--;H=hl>>8;L=hl;F=(B?(B&Z80_SF):Z80_ZF)|(B&(Z80_XF|Z80_YF));if(d8&Z80_SF){F|=Z80_NF;}uint32_t t=(uint32_t)(L+d8);if (t&0x0100){F|=Z80_HF|Z80_CF;}F|=_z80_szp[((uint8_t)(t&0x07))^B]&Z80_PF;}break;
          case 0xb0:/*LDIR*/{uint16_t hl=(H<<8)|L;uint16_t de=(D<<8)|E;_MR(hl,d8);_MW(de,d8);hl++;de++;H=hl>>8;L=hl;D=de>>8;E=de;_T(2);d8+=A;F&=Z80_SF|Z80_ZF|Z80_CF;if(d8&0x02){F|=Z80_YF;}if(d8&0x08){F|=Z80_XF;}uint16_t bc=(B<<8)|C;bc--;B=bc>>8;C=bc;if(bc){F|=Z80_VF;}if(bc){PC-=2;WZ=PC+1;_T(5);}}break;
          case 0xb1:/*CPIR*/{uint16_t hl=(H<<8)|L;_MR(hl,d8);hl++;WZ++;H=hl>>8;L=hl;_T(5);int r=((int)A)-d8;F=(F&Z80_CF)|Z80_NF|_SZ(r);if((r&0x0F)>(A&0x0F)){F|=Z80_HF;r--;}if(r&0x02){F|=Z80_YF;}if(r&0x08){F|=Z80_XF;}uint16_t bc=(B<<8)|C;bc--;B=bc>>8;C=bc;if(bc){F|=Z80_VF;}if(bc&&!(F&Z80_ZF)){PC-=2;WZ=PC+1;_T(5);}}break;
          case 0xb2:/*INIR*/{_T(1);WZ=(B<<8)|C;uint16_t hl=(H<<8)|L;_IN(WZ,d8);_MW(hl,d8);B--;WZ++;hl++;C++;H=hl>>8;L=hl;F=(B?(B&Z80_SF):Z80_ZF)|(B&(Z80_XF|Z80_YF));if(d8&Z80_SF){F|=Z80_NF;}uint32_t t=(uint32_t)(C+d8);if(t&0x100){F|=Z80_HF|Z80_CF;}F|=_z80_szp[((uint8_t)(t&0x07))^B]&Z80_PF;if(B){PC-=2;_T(5);}}break;
          case 0xb3:/*OTIR*/{_T(1);uint16_t hl=(H<<8)|L;_MR(hl,d8);B--;WZ=(B<<8)|C;_OUT(WZ,d8);WZ++; hl++;H=hl>>8;L=hl;F=(B?(B&Z80_SF):Z80_ZF)|(B&(Z80_XF|Z80_YF));if(d8&Z80_SF){F|=Z80_NF;}uint32_t t=(uint32_t)(L+d8);if (t&0x0100){F|=Z80_HF|Z80_CF;}F|=_z80_szp[((uint8_t)(t&0x07))^B]&Z80_PF;if(B){PC-=2;_T(5);}}break;
          case 0xb8:/*LDDR*/{uint16_t hl=(H<<8)|L;uint16_t de=(D<<8)|E;_MR(hl,d8);_MW(de,d8);hl--;de--;H=hl>>8;L=hl;D=de>>8;E=de;_T(2);d8+=A;F&=Z80_SF|Z80_ZF|Z80_CF;if(d8&0x02){F|=Z80_YF;}if(d8&0x08){F|=Z80_XF;}uint16_t bc=(B<<8)|C;bc--;B=bc>>8;C=bc;if(bc){F|=Z80_VF;}if(bc){PC-=2;WZ=PC+1;_T(5);}}break;
          case 0xb9:/*CPDR*/{uint16_t hl=(H<<8)|L;_MR(hl,d8);hl--;WZ--;H=hl>>8;L=hl;_T(5);int r=((int)A)-d8;F=(F&Z80_CF)|Z80_NF|_SZ(r);if((r&0x0F)>(A&0x0F)){F|=Z80_HF;r--;}if(r&0x02){F|=Z80_YF;}if(r&0x08){F|=Z80_XF;}uint16_t bc=(B<<8)|C;bc--;B=bc>>8;C=bc;if(bc){F|=Z80_VF;}if(bc&&!(F&Z80_ZF)){PC-=2;WZ=PC+1;_T(5);}}break;
          case 0xba:/*INDR*/{_T(1);WZ=(B<<8)|C;uint16_t hl=(H<<8)|L;_IN(WZ,d8);_MW(hl,d8);B--;WZ--;hl--;C--;H=hl>>8;L=hl;F=(B?(B&Z80_SF):Z80_ZF)|(B&(Z80_XF|Z80_YF));if(d8&Z80_SF){F|=Z80_NF;}uint32_t t=(uint32_t)(C+d8);if(t&0x100){F|=Z80_HF|Z80_CF;}F|=_z80_szp[((uint8_t)(t&0x07))^B]&Z80_PF;if(B){PC-=2;_T(5);}}break;
          case 0xbb:/*OTDR*/{_T(1);uint16_t hl=(H<<8)|L;_MR(hl,d8);B--;WZ=(B<<8)|C;_OUT(WZ,d8);WZ--;hl--;H=hl>>8;L=hl;F=(B?(B&Z80_SF):Z80_ZF)|(B&(Z80_XF|Z80_YF));if(d8&Z80_SF){F|=Z80_NF;}uint32_t t=(uint32_t)(L+d8);if (t&0x0100){F|=Z80_HF|Z80_CF;}F|=_z80_szp[((uint8_t)(t&0x07))^B]&Z80_PF;if(B){PC-=2;_T(5);}}break;
          default: break;
        }
      }
      break;
      case 0xee:/*XOR n*/_IMM8(d8);{A^=d8^;F=_z80_szp[A];}break;
      case 0xef:/*RST 0x28*/_T(1);_MW(--SP,PC>>8);_MW(--SP,PC);PC=WZ=0x28;break;
      case 0xf0:/*RET P*/_T(1);if (!(_G_F()&Z80_SF)){uint8_t w,z;_MR(SP++,z);_MR(SP++,w);PC=(w<<8)|z;WZ=PC;}break;
      case 0xf1:/*POP FA*/_MR(SP++,F);_MR(SP++,A);break;
      case 0xf2:/*JP P,nn*/_IMM16(addr);if(!(_G_F()&Z80_SF)){pc=addr;}break;
      case 0xf3:/*DI*/IFF1=IFF2=false;break;
      case 0xf4:/*CALL P,nn*/_IMM16(addr);if(!(_G_F()&Z80_SF)){_T(1);_MW(--SP,PC>>8);_MW(--SP,PC);PC=addr;}break;
      case 0xf5:/*PUSH FA*/_T(1);_MW(--SP,F);_MW(--SP,A);break;
      case 0xf6:/*OR n*/_IMM8(d8);{A|=d8;F=_z80_szp[A];}break;
      case 0xf7:/*RST 0x30*/_T(1);_MW(--SP,PC>>8);_MW(--SP,PC);PC=WZ=0x30;break;
      case 0xf8:/*RET M*/_T(1);if ((_G_F()&Z80_SF)){uint8_t w,z;_MR(SP++,z);_MR(SP++,w);PC=(w<<8)|z;WZ=PC;}break;
      case 0xf9:/*LD SP,HL*/_T(2);_S_SP(_G_HL());break;
      case 0xfa:/*JP M,nn*/_IMM16(addr);if((_G_F()&Z80_SF)){pc=addr;}break;
      case 0xfb:/*EI*/IFF1=IFF2=false;EI_PENDING=true;break;
      case 0xfc:/*CALL M,nn*/_IMM16(addr);if((_G_F()&Z80_SF)){_T(1);_MW(--SP,PC>>8);_MW(--SP,PC);PC=addr;}break;
      case 0xfd:/*FD prefix*/map_bits|=_BIT_USE_IY;continue;break;
      case 0xfe:/*CP n*/_IMM8(d8);{int32_t res=(uint32_t)((int)A-(int)d8);F=_CP_FLAGS(A,d8,res));}break;
      case 0xff:/*RST 0x38*/_T(1);_MW(--SP,PC>>8);_MW(--SP,PC);PC=WZ=0x38;break;
      default: break;
    }
    bool nmi = 0 != ((pins & (pre_pins ^ pins)) & Z80_NMI);
    if (nmi || (((pins & (Z80_INT|Z80_BUSREQ))==Z80_INT) && IFF1)) {
      IFF1=false;
      if (pins & Z80_INT) {
        IFF2=false;
      }
      if (pins & Z80_HALT) {
        pins &= ~Z80_HALT;
        PC++;
      }
      _SA(PC);
      if (nmi) {
        _TWM(5,Z80_M1|Z80_MREQ|Z80_RD);_BUMPR();
        _MW(--SP,PC>>8);
        _MW(--SP,PC);
        PC=WZ=0x0066;
      }
      else {
        _TWM(4,Z80_M1|Z80_IORQ);
        const uint8_t int_vec=_GD();
        _BUMPR();
        _T(2);
        switch (IM) {
          case 0:
            break;
          case 1:
            {
              _MW(--SP,PC>>8);
              _MW(--SP,PC);
              PC=WZ=0x0038;
            }
            break;
          case 2:
            {
              _MW(--SP,PC>>8);
              _MW(--SP,PC);
              addr = (I<<8) | (int_vec & 0xFE);
              uint8_t z,w;
              _MR(addr++,z);
              _MR(addr,w);
              PC=WZ=(w<<8)|z;
            }
            break;
        }
       }
    }
    pins&=~Z80_INT;
    /* delay-enable interrupt flags */
    if (EI_PENDING) {
      EI_PENDING=false;
      IFF1=IFF2=true;
    }
    if (trap_addr != 0xFFFFFFFFFFFFFFFF) {
      uint64_t ta = trap_addr;
      for (int i = 0; i < Z80_MAX_NUM_TRAPS; i++) {
        if (((ta & 0xFFFF) == pc) && (pc != 0xFFFF)) {
          trap_id = i;
          break;
        }
        ta >>= 16;
      }
    }
    pre_pins = pins;
  } while ((ticks < num_ticks) && (trap_id < 0));
  cpu->b=B; cpu->c=C;
  cpu->d=D; cpu->e=E;
  cpu->h=H; cpu->l=L;
  cpu->a=A; cpu->f=F;
  cpu->ixh=IXH; cpu->ixl=IXL;
  cpu->iyh=IYH; cpu->iyl=IYL;
  cpu->bc_=BC_; cpu->de_=DE_; cpu->hl_=HL_; cpu->af_=AF_;
  cpu->wz=WZ; cpu->sp=SP; cpu->pc=PC;
  cpu->im=IM; cpu->i=I; cpu->r=R;
  cpu->iff1=IFF1; cpu->iff2=IFF2; cpu->ei_pending=EI_PENDING;
  cpu->pins = pins;
  cpu->trap_id = trap_id;
  return ticks;
}
