uint32_t z80_exec(z80_t* cpu, uint32_t num_ticks) {
  uint64_t r0 = cpu->bc_de_hl_fa;
  uint64_t r1 = cpu->wz_ix_iy_sp;
  uint64_t r2 = cpu->im_ir_pc_bits;
  uint64_t r3 = cpu->bc_de_hl_fa_;
  uint64_t ws = _z80_map_regs(r0, r1, r2);
  uint64_t map_bits = r2 & _BITS_MAP_REGS;
  uint64_t pins = cpu->pins;
  const uint64_t trap_addr = cpu->trap_addr;
  const z80_tick_t tick = cpu->tick;
  void* ud = cpu->user_data;
  int trap_id = -1;
  uint32_t ticks = 0;
  uint8_t op, d8;
  uint16_t addr, d16;
  uint16_t pc = _G_PC();
  uint64_t pre_pins = pins;
  do {
    _FETCH(op)
    if (op == 0xED) {
      map_bits &= ~(_BIT_USE_IX|_BIT_USE_IY);
    }
    if (map_bits != (r2 & _BITS_MAP_REGS)) {
      const uint64_t old_map_bits = r2 & _BITS_MAP_REGS;
      r0 = _z80_flush_r0(ws, r0, old_map_bits);
      r1 = _z80_flush_r1(ws, r1, old_map_bits);
      r2 = (r2 & ~_BITS_MAP_REGS) | map_bits;
      ws = _z80_map_regs(r0, r1, r2);
    }
    switch (op) {
      case 0x0:/*NOP*/ break;
      case 0x1:/*LD BC,nn*/_IMM16(d16);_S_BC(d16);break;
      case 0x2:/*LD (BC),A*/addr=_G_BC();d8=_G_A();_MW(addr++,d8);_S_WZ((d8<<8)|(addr&0x00FF));break;
      case 0x3:/*INC BC*/_T(2);_S_BC(_G_BC()+1);break;
      case 0x4:/*INC B*/d8=_G_B();{uint8_t r=d8+1;uint8_t f=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x80){f|=Z80_VF;}_S_F(f|(_G_F()&Z80_CF));d8=r;}_S_B(d8);break;
      case 0x5:/*DEC B*/d8=_G_B();{uint8_t r=d8-1;uint8_t f=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x7F){f|=Z80_VF;}_S_F(f|(_G_F()&Z80_CF));d8=r;}_S_B(d8);break;
      case 0x6:/*LD B,n*/_IMM8(d8);_S_B(d8);break;
      case 0x7:/*RLCA*/ws=_z80_rlca(ws);break;
      case 0x8:/*EX AF,AF'*/{r0=_z80_flush_r0(ws,r0,r2);uint16_t fa=_G16(r0,_FA);uint16_t fa_=_G16(r3,_FA);_S16(r0,_FA,fa_);_S16(r3,_FA,fa);ws=_z80_map_regs(r0,r1,r2);}break;
      case 0x9:/*ADD HL,BC*/{uint16_t acc=_G_HL();_S_WZ(acc+1);d16=_G_BC();uint32_t r=acc+d16;_S_HL(r);uint8_t f=_G_F()&(Z80_SF|Z80_ZF|Z80_VF);f|=((acc^r^d16)>>8)&Z80_HF;f|=((r>>16)&Z80_CF)|((r>>8)&(Z80_YF|Z80_XF));_S_F(f);_T(7);}break;
      case 0xa:/*LD A,(BC)*/addr=_G_BC();_MR(addr++,d8);_S_A(d8);_S_WZ(addr);break;
      case 0xb:/*DEC BC*/_T(2);_S_BC(_G_BC()-1);break;
      case 0xc:/*INC C*/d8=_G_C();{uint8_t r=d8+1;uint8_t f=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x80){f|=Z80_VF;}_S_F(f|(_G_F()&Z80_CF));d8=r;}_S_C(d8);break;
      case 0xd:/*DEC C*/d8=_G_C();{uint8_t r=d8-1;uint8_t f=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x7F){f|=Z80_VF;}_S_F(f|(_G_F()&Z80_CF));d8=r;}_S_C(d8);break;
      case 0xe:/*LD C,n*/_IMM8(d8);_S_C(d8);break;
      case 0xf:/*RRCA*/ws=_z80_rrca(ws);break;
      case 0x10:/*DJNZ*/{_T(1);int8_t d;_IMM8(d);d8=_G_B()-1;_S_B(d8);if(d8>0){pc+=d;_S_WZ(pc);_T(5);}}break;
      case 0x11:/*LD DE,nn*/_IMM16(d16);_S_DE(d16);break;
      case 0x12:/*LD (DE),A*/addr=_G_DE();d8=_G_A();_MW(addr++,d8);_S_WZ((d8<<8)|(addr&0x00FF));break;
      case 0x13:/*INC DE*/_T(2);_S_DE(_G_DE()+1);break;
      case 0x14:/*INC D*/d8=_G_D();{uint8_t r=d8+1;uint8_t f=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x80){f|=Z80_VF;}_S_F(f|(_G_F()&Z80_CF));d8=r;}_S_D(d8);break;
      case 0x15:/*DEC D*/d8=_G_D();{uint8_t r=d8-1;uint8_t f=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x7F){f|=Z80_VF;}_S_F(f|(_G_F()&Z80_CF));d8=r;}_S_D(d8);break;
      case 0x16:/*LD D,n*/_IMM8(d8);_S_D(d8);break;
      case 0x17:/*RLA*/ws=_z80_rla(ws);break;
      case 0x18:/*JR d*/{int8_t d;_IMM8(d);pc+=d;_S_WZ(pc);_T(5);}break;
      case 0x19:/*ADD HL,DE*/{uint16_t acc=_G_HL();_S_WZ(acc+1);d16=_G_DE();uint32_t r=acc+d16;_S_HL(r);uint8_t f=_G_F()&(Z80_SF|Z80_ZF|Z80_VF);f|=((acc^r^d16)>>8)&Z80_HF;f|=((r>>16)&Z80_CF)|((r>>8)&(Z80_YF|Z80_XF));_S_F(f);_T(7);}break;
      case 0x1a:/*LD A,(DE)*/addr=_G_DE();_MR(addr++,d8);_S_A(d8);_S_WZ(addr);break;
      case 0x1b:/*DEC DE*/_T(2);_S_DE(_G_DE()-1);break;
      case 0x1c:/*INC E*/d8=_G_E();{uint8_t r=d8+1;uint8_t f=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x80){f|=Z80_VF;}_S_F(f|(_G_F()&Z80_CF));d8=r;}_S_E(d8);break;
      case 0x1d:/*DEC E*/d8=_G_E();{uint8_t r=d8-1;uint8_t f=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x7F){f|=Z80_VF;}_S_F(f|(_G_F()&Z80_CF));d8=r;}_S_E(d8);break;
      case 0x1e:/*LD E,n*/_IMM8(d8);_S_E(d8);break;
      case 0x1f:/*RRA*/ws=_z80_rra(ws);break;
      case 0x20:/*JR NZ,d*/{int8_t d;_IMM8(d);if(!(_G_F()&Z80_ZF)){pc+=d;_S_WZ(pc);_T(5);}}break;
      case 0x21:/*LD HL,nn*/_IMM16(d16);_S_HL(d16);break;
      case 0x22:/*LD (nn),HL*/_IMM16(addr);_MW(addr++,_G_L());_MW(addr,_G_H());_S_WZ(addr);break;
      case 0x23:/*INC HL*/_T(2);_S_HL(_G_HL()+1);break;
      case 0x24:/*INC H*/d8=_G_H();{uint8_t r=d8+1;uint8_t f=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x80){f|=Z80_VF;}_S_F(f|(_G_F()&Z80_CF));d8=r;}_S_H(d8);break;
      case 0x25:/*DEC H*/d8=_G_H();{uint8_t r=d8-1;uint8_t f=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x7F){f|=Z80_VF;}_S_F(f|(_G_F()&Z80_CF));d8=r;}_S_H(d8);break;
      case 0x26:/*LD H,n*/_IMM8(d8);_S_H(d8);break;
      case 0x27:/*DAA*/ws=_z80_daa(ws);break;
      case 0x28:/*JR Z,d*/{int8_t d;_IMM8(d);if((_G_F()&Z80_ZF)){pc+=d;_S_WZ(pc);_T(5);}}break;
      case 0x29:/*ADD HL,HL*/{uint16_t acc=_G_HL();_S_WZ(acc+1);d16=_G_HL();uint32_t r=acc+d16;_S_HL(r);uint8_t f=_G_F()&(Z80_SF|Z80_ZF|Z80_VF);f|=((acc^r^d16)>>8)&Z80_HF;f|=((r>>16)&Z80_CF)|((r>>8)&(Z80_YF|Z80_XF));_S_F(f);_T(7);}break;
      case 0x2a:/*LD HL,(nn)*/_IMM16(addr);_MR(addr++,d8);_S_L(d8);_MR(addr,d8);_S_H(d8);_S_WZ(addr);break;
      case 0x2b:/*DEC HL*/_T(2);_S_HL(_G_HL()-1);break;
      case 0x2c:/*INC L*/d8=_G_L();{uint8_t r=d8+1;uint8_t f=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x80){f|=Z80_VF;}_S_F(f|(_G_F()&Z80_CF));d8=r;}_S_L(d8);break;
      case 0x2d:/*DEC L*/d8=_G_L();{uint8_t r=d8-1;uint8_t f=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x7F){f|=Z80_VF;}_S_F(f|(_G_F()&Z80_CF));d8=r;}_S_L(d8);break;
      case 0x2e:/*LD L,n*/_IMM8(d8);_S_L(d8);break;
      case 0x2f:/*CPL*/ws=_z80_cpl(ws);break;
      case 0x30:/*JR NC,d*/{int8_t d;_IMM8(d);if(!(_G_F()&Z80_CF)){pc+=d;_S_WZ(pc);_T(5);}}break;
      case 0x31:/*LD SP,nn*/_IMM16(d16);_S_SP(d16);break;
      case 0x32:/*LD (nn),A*/_IMM16(addr);d8=_G_A();_MW(addr++,d8);_S_WZ((d8<<8)|(addr&0x00FF));break;
      case 0x33:/*INC SP*/_T(2);_S_SP(_G_SP()+1);break;
      case 0x34:/*INC (HL/IX+d/IY+d)*/_ADDR(addr,5);_T(1);_MR(addr,d8);{uint8_t r=d8+1;uint8_t f=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x80){f|=Z80_VF;}_S_F(f|(_G_F()&Z80_CF));d8=r;}_MW(addr,d8);break;
      case 0x35:/*DEC (HL/IX+d/IY+d)*/_ADDR(addr,5);_T(1);_MR(addr,d8);{uint8_t r=d8-1;uint8_t f=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x7F){f|=Z80_VF;}_S_F(f|(_G_F()&Z80_CF));d8=r;}_MW(addr,d8);break;
      case 0x36:/*LD (HL/IX+d/IY+d),n*/_ADDR(addr,2);_IMM8(d8);_MW(addr,d8);break;
      case 0x37:/*SCF*/ws=_z80_scf(ws);break;
      case 0x38:/*JR C,d*/{int8_t d;_IMM8(d);if((_G_F()&Z80_CF)){pc+=d;_S_WZ(pc);_T(5);}}break;
      case 0x39:/*ADD HL,SP*/{uint16_t acc=_G_HL();_S_WZ(acc+1);d16=_G_SP();uint32_t r=acc+d16;_S_HL(r);uint8_t f=_G_F()&(Z80_SF|Z80_ZF|Z80_VF);f|=((acc^r^d16)>>8)&Z80_HF;f|=((r>>16)&Z80_CF)|((r>>8)&(Z80_YF|Z80_XF));_S_F(f);_T(7);}break;
      case 0x3a:/*LD A,(nn)*/_IMM16(addr);_MR(addr++,d8);_S_A(d8);_S_WZ(addr);break;
      case 0x3b:/*DEC SP*/_T(2);_S_SP(_G_SP()-1);break;
      case 0x3c:/*INC A*/d8=_G_A();{uint8_t r=d8+1;uint8_t f=_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x80){f|=Z80_VF;}_S_F(f|(_G_F()&Z80_CF));d8=r;}_S_A(d8);break;
      case 0x3d:/*DEC A*/d8=_G_A();{uint8_t r=d8-1;uint8_t f=Z80_NF|_SZ(r)|(r&(Z80_XF|Z80_YF))|((r^d8)&Z80_HF);if(r==0x7F){f|=Z80_VF;}_S_F(f|(_G_F()&Z80_CF));d8=r;}_S_A(d8);break;
      case 0x3e:/*LD A,n*/_IMM8(d8);_S_A(d8);break;
      case 0x3f:/*CCF*/ws=_z80_ccf(ws);break;
      case 0x40:/*LD B,B*/_S_B(_G_B());break;
      case 0x41:/*LD B,C*/_S_B(_G_C());break;
      case 0x42:/*LD B,D*/_S_B(_G_D());break;
      case 0x43:/*LD B,E*/_S_B(_G_E());break;
      case 0x44:/*LD B,H*/_S_B(_G_H());break;
      case 0x45:/*LD B,L*/_S_B(_G_L());break;
      case 0x46:/*LD B,(HL/IX+d/IY+d)*/_ADDR(addr,5);_MR(addr,d8);_S_B(d8);break;
      case 0x47:/*LD B,A*/_S_B(_G_A());break;
      case 0x48:/*LD C,B*/_S_C(_G_B());break;
      case 0x49:/*LD C,C*/_S_C(_G_C());break;
      case 0x4a:/*LD C,D*/_S_C(_G_D());break;
      case 0x4b:/*LD C,E*/_S_C(_G_E());break;
      case 0x4c:/*LD C,H*/_S_C(_G_H());break;
      case 0x4d:/*LD C,L*/_S_C(_G_L());break;
      case 0x4e:/*LD C,(HL/IX+d/IY+d)*/_ADDR(addr,5);_MR(addr,d8);_S_C(d8);break;
      case 0x4f:/*LD C,A*/_S_C(_G_A());break;
      case 0x50:/*LD D,B*/_S_D(_G_B());break;
      case 0x51:/*LD D,C*/_S_D(_G_C());break;
      case 0x52:/*LD D,D*/_S_D(_G_D());break;
      case 0x53:/*LD D,E*/_S_D(_G_E());break;
      case 0x54:/*LD D,H*/_S_D(_G_H());break;
      case 0x55:/*LD D,L*/_S_D(_G_L());break;
      case 0x56:/*LD D,(HL/IX+d/IY+d)*/_ADDR(addr,5);_MR(addr,d8);_S_D(d8);break;
      case 0x57:/*LD D,A*/_S_D(_G_A());break;
      case 0x58:/*LD E,B*/_S_E(_G_B());break;
      case 0x59:/*LD E,C*/_S_E(_G_C());break;
      case 0x5a:/*LD E,D*/_S_E(_G_D());break;
      case 0x5b:/*LD E,E*/_S_E(_G_E());break;
      case 0x5c:/*LD E,H*/_S_E(_G_H());break;
      case 0x5d:/*LD E,L*/_S_E(_G_L());break;
      case 0x5e:/*LD E,(HL/IX+d/IY+d)*/_ADDR(addr,5);_MR(addr,d8);_S_E(d8);break;
      case 0x5f:/*LD E,A*/_S_E(_G_A());break;
      case 0x60:/*LD H,B*/_S_H(_G_B());break;
      case 0x61:/*LD H,C*/_S_H(_G_C());break;
      case 0x62:/*LD H,D*/_S_H(_G_D());break;
      case 0x63:/*LD H,E*/_S_H(_G_E());break;
      case 0x64:/*LD H,H*/_S_H(_G_H());break;
      case 0x65:/*LD H,L*/_S_H(_G_L());break;
      case 0x66:/*LD H,(HL/IX+d/IY+d)*/_ADDR(addr,5);_MR(addr,d8);if(_IDX()){_S8(r0,_H,d8);}else{_S_H(d8);}break;
      case 0x67:/*LD H,A*/_S_H(_G_A());break;
      case 0x68:/*LD L,B*/_S_L(_G_B());break;
      case 0x69:/*LD L,C*/_S_L(_G_C());break;
      case 0x6a:/*LD L,D*/_S_L(_G_D());break;
      case 0x6b:/*LD L,E*/_S_L(_G_E());break;
      case 0x6c:/*LD L,H*/_S_L(_G_H());break;
      case 0x6d:/*LD L,L*/_S_L(_G_L());break;
      case 0x6e:/*LD L,(HL/IX+d/IY+d)*/_ADDR(addr,5);_MR(addr,d8);if(_IDX()){_S8(r0,_L,d8);}else{_S_L(d8);}break;
      case 0x6f:/*LD L,A*/_S_L(_G_A());break;
      case 0x70:/*LD (HL/IX+d/IY+d),B*/d8=_G_B();_ADDR(addr,5);_MW(addr,d8);break;
      case 0x71:/*LD (HL/IX+d/IY+d),C*/d8=_G_C();_ADDR(addr,5);_MW(addr,d8);break;
      case 0x72:/*LD (HL/IX+d/IY+d),D*/d8=_G_D();_ADDR(addr,5);_MW(addr,d8);break;
      case 0x73:/*LD (HL/IX+d/IY+d),E*/d8=_G_E();_ADDR(addr,5);_MW(addr,d8);break;
      case 0x74:/*LD (HL/IX+d/IY+d),H*/d8=_IDX()?_G8(r0,_H):_G_H();_ADDR(addr,5);_MW(addr,d8);break;
      case 0x75:/*LD (HL/IX+d/IY+d),L*/d8=_IDX()?_G8(r0,_L):_G_L();_ADDR(addr,5);_MW(addr,d8);break;
      case 0x76:/*HALT*/pins|=Z80_HALT;pc--;break;
      case 0x77:/*LD (HL/IX+d/IY+d),A*/d8=_G_A();_ADDR(addr,5);_MW(addr,d8);break;
      case 0x78:/*LD A,B*/_S_A(_G_B());break;
      case 0x79:/*LD A,C*/_S_A(_G_C());break;
      case 0x7a:/*LD A,D*/_S_A(_G_D());break;
      case 0x7b:/*LD A,E*/_S_A(_G_E());break;
      case 0x7c:/*LD A,H*/_S_A(_G_H());break;
      case 0x7d:/*LD A,L*/_S_A(_G_L());break;
      case 0x7e:/*LD A,(HL/IX+d/IY+d)*/_ADDR(addr,5);_MR(addr,d8);_S_A(d8);break;
      case 0x7f:/*LD A,A*/_S_A(_G_A());break;
      case 0x80:/*ADD B*/d8=_G_B();{uint8_t acc=_G_A();uint32_t res=acc+d8;_S_F(_ADD_FLAGS(acc,d8,res));_S_A(res);}break;
      case 0x81:/*ADD C*/d8=_G_C();{uint8_t acc=_G_A();uint32_t res=acc+d8;_S_F(_ADD_FLAGS(acc,d8,res));_S_A(res);}break;
      case 0x82:/*ADD D*/d8=_G_D();{uint8_t acc=_G_A();uint32_t res=acc+d8;_S_F(_ADD_FLAGS(acc,d8,res));_S_A(res);}break;
      case 0x83:/*ADD E*/d8=_G_E();{uint8_t acc=_G_A();uint32_t res=acc+d8;_S_F(_ADD_FLAGS(acc,d8,res));_S_A(res);}break;
      case 0x84:/*ADD H*/d8=_G_H();{uint8_t acc=_G_A();uint32_t res=acc+d8;_S_F(_ADD_FLAGS(acc,d8,res));_S_A(res);}break;
      case 0x85:/*ADD L*/d8=_G_L();{uint8_t acc=_G_A();uint32_t res=acc+d8;_S_F(_ADD_FLAGS(acc,d8,res));_S_A(res);}break;
      case 0x86:/*ADD,(HL/IX+d/IY+d)*/_ADDR(addr,5);_MR(addr,d8);{uint8_t acc=_G_A();uint32_t res=acc+d8;_S_F(_ADD_FLAGS(acc,d8,res));_S_A(res);}break;
      case 0x87:/*ADD A*/d8=_G_A();{uint8_t acc=_G_A();uint32_t res=acc+d8;_S_F(_ADD_FLAGS(acc,d8,res));_S_A(res);}break;
      case 0x88:/*ADC B*/d8=_G_B();{uint8_t acc=_G_A();uint32_t res=acc+d8+(_G_F()&Z80_CF);_S_F(_ADD_FLAGS(acc,d8,res));_S_A(res);}break;
      case 0x89:/*ADC C*/d8=_G_C();{uint8_t acc=_G_A();uint32_t res=acc+d8+(_G_F()&Z80_CF);_S_F(_ADD_FLAGS(acc,d8,res));_S_A(res);}break;
      case 0x8a:/*ADC D*/d8=_G_D();{uint8_t acc=_G_A();uint32_t res=acc+d8+(_G_F()&Z80_CF);_S_F(_ADD_FLAGS(acc,d8,res));_S_A(res);}break;
      case 0x8b:/*ADC E*/d8=_G_E();{uint8_t acc=_G_A();uint32_t res=acc+d8+(_G_F()&Z80_CF);_S_F(_ADD_FLAGS(acc,d8,res));_S_A(res);}break;
      case 0x8c:/*ADC H*/d8=_G_H();{uint8_t acc=_G_A();uint32_t res=acc+d8+(_G_F()&Z80_CF);_S_F(_ADD_FLAGS(acc,d8,res));_S_A(res);}break;
      case 0x8d:/*ADC L*/d8=_G_L();{uint8_t acc=_G_A();uint32_t res=acc+d8+(_G_F()&Z80_CF);_S_F(_ADD_FLAGS(acc,d8,res));_S_A(res);}break;
      case 0x8e:/*ADC,(HL/IX+d/IY+d)*/_ADDR(addr,5);_MR(addr,d8);{uint8_t acc=_G_A();uint32_t res=acc+d8+(_G_F()&Z80_CF);_S_F(_ADD_FLAGS(acc,d8,res));_S_A(res);}break;
      case 0x8f:/*ADC A*/d8=_G_A();{uint8_t acc=_G_A();uint32_t res=acc+d8+(_G_F()&Z80_CF);_S_F(_ADD_FLAGS(acc,d8,res));_S_A(res);}break;
      case 0x90:/*SUB B*/d8=_G_B();{uint8_t acc=_G_A();uint32_t res=(uint32_t)((int)acc-(int)d8);_S_F(_SUB_FLAGS(acc,d8,res));_S_A(res);}break;
      case 0x91:/*SUB C*/d8=_G_C();{uint8_t acc=_G_A();uint32_t res=(uint32_t)((int)acc-(int)d8);_S_F(_SUB_FLAGS(acc,d8,res));_S_A(res);}break;
      case 0x92:/*SUB D*/d8=_G_D();{uint8_t acc=_G_A();uint32_t res=(uint32_t)((int)acc-(int)d8);_S_F(_SUB_FLAGS(acc,d8,res));_S_A(res);}break;
      case 0x93:/*SUB E*/d8=_G_E();{uint8_t acc=_G_A();uint32_t res=(uint32_t)((int)acc-(int)d8);_S_F(_SUB_FLAGS(acc,d8,res));_S_A(res);}break;
      case 0x94:/*SUB H*/d8=_G_H();{uint8_t acc=_G_A();uint32_t res=(uint32_t)((int)acc-(int)d8);_S_F(_SUB_FLAGS(acc,d8,res));_S_A(res);}break;
      case 0x95:/*SUB L*/d8=_G_L();{uint8_t acc=_G_A();uint32_t res=(uint32_t)((int)acc-(int)d8);_S_F(_SUB_FLAGS(acc,d8,res));_S_A(res);}break;
      case 0x96:/*SUB,(HL/IX+d/IY+d)*/_ADDR(addr,5);_MR(addr,d8);{uint8_t acc=_G_A();uint32_t res=(uint32_t)((int)acc-(int)d8);_S_F(_SUB_FLAGS(acc,d8,res));_S_A(res);}break;
      case 0x97:/*SUB A*/d8=_G_A();{uint8_t acc=_G_A();uint32_t res=(uint32_t)((int)acc-(int)d8);_S_F(_SUB_FLAGS(acc,d8,res));_S_A(res);}break;
      case 0x98:/*SBC B*/d8=_G_B();{uint8_t acc=_G_A();uint32_t res=(uint32_t)((int)acc-(int)d8-(_G_F()&Z80_CF));_S_F(_SUB_FLAGS(acc,d8,res));_S_A(res);}break;
      case 0x99:/*SBC C*/d8=_G_C();{uint8_t acc=_G_A();uint32_t res=(uint32_t)((int)acc-(int)d8-(_G_F()&Z80_CF));_S_F(_SUB_FLAGS(acc,d8,res));_S_A(res);}break;
      case 0x9a:/*SBC D*/d8=_G_D();{uint8_t acc=_G_A();uint32_t res=(uint32_t)((int)acc-(int)d8-(_G_F()&Z80_CF));_S_F(_SUB_FLAGS(acc,d8,res));_S_A(res);}break;
      case 0x9b:/*SBC E*/d8=_G_E();{uint8_t acc=_G_A();uint32_t res=(uint32_t)((int)acc-(int)d8-(_G_F()&Z80_CF));_S_F(_SUB_FLAGS(acc,d8,res));_S_A(res);}break;
      case 0x9c:/*SBC H*/d8=_G_H();{uint8_t acc=_G_A();uint32_t res=(uint32_t)((int)acc-(int)d8-(_G_F()&Z80_CF));_S_F(_SUB_FLAGS(acc,d8,res));_S_A(res);}break;
      case 0x9d:/*SBC L*/d8=_G_L();{uint8_t acc=_G_A();uint32_t res=(uint32_t)((int)acc-(int)d8-(_G_F()&Z80_CF));_S_F(_SUB_FLAGS(acc,d8,res));_S_A(res);}break;
      case 0x9e:/*SBC,(HL/IX+d/IY+d)*/_ADDR(addr,5);_MR(addr,d8);{uint8_t acc=_G_A();uint32_t res=(uint32_t)((int)acc-(int)d8-(_G_F()&Z80_CF));_S_F(_SUB_FLAGS(acc,d8,res));_S_A(res);}break;
      case 0x9f:/*SBC A*/d8=_G_A();{uint8_t acc=_G_A();uint32_t res=(uint32_t)((int)acc-(int)d8-(_G_F()&Z80_CF));_S_F(_SUB_FLAGS(acc,d8,res));_S_A(res);}break;
      case 0xa0:/*AND B*/d8=_G_B();{d8&=_G_A();_S_F(_z80_szp[d8]|Z80_HF);_S_A(d8);}break;
      case 0xa1:/*AND C*/d8=_G_C();{d8&=_G_A();_S_F(_z80_szp[d8]|Z80_HF);_S_A(d8);}break;
      case 0xa2:/*AND D*/d8=_G_D();{d8&=_G_A();_S_F(_z80_szp[d8]|Z80_HF);_S_A(d8);}break;
      case 0xa3:/*AND E*/d8=_G_E();{d8&=_G_A();_S_F(_z80_szp[d8]|Z80_HF);_S_A(d8);}break;
      case 0xa4:/*AND H*/d8=_G_H();{d8&=_G_A();_S_F(_z80_szp[d8]|Z80_HF);_S_A(d8);}break;
      case 0xa5:/*AND L*/d8=_G_L();{d8&=_G_A();_S_F(_z80_szp[d8]|Z80_HF);_S_A(d8);}break;
      case 0xa6:/*AND,(HL/IX+d/IY+d)*/_ADDR(addr,5);_MR(addr,d8);{d8&=_G_A();_S_F(_z80_szp[d8]|Z80_HF);_S_A(d8);}break;
      case 0xa7:/*AND A*/d8=_G_A();{d8&=_G_A();_S_F(_z80_szp[d8]|Z80_HF);_S_A(d8);}break;
      case 0xa8:/*XOR B*/d8=_G_B();{d8^=_G_A();_S_F(_z80_szp[d8]);_S_A(d8);}break;
      case 0xa9:/*XOR C*/d8=_G_C();{d8^=_G_A();_S_F(_z80_szp[d8]);_S_A(d8);}break;
      case 0xaa:/*XOR D*/d8=_G_D();{d8^=_G_A();_S_F(_z80_szp[d8]);_S_A(d8);}break;
      case 0xab:/*XOR E*/d8=_G_E();{d8^=_G_A();_S_F(_z80_szp[d8]);_S_A(d8);}break;
      case 0xac:/*XOR H*/d8=_G_H();{d8^=_G_A();_S_F(_z80_szp[d8]);_S_A(d8);}break;
      case 0xad:/*XOR L*/d8=_G_L();{d8^=_G_A();_S_F(_z80_szp[d8]);_S_A(d8);}break;
      case 0xae:/*XOR,(HL/IX+d/IY+d)*/_ADDR(addr,5);_MR(addr,d8);{d8^=_G_A();_S_F(_z80_szp[d8]);_S_A(d8);}break;
      case 0xaf:/*XOR A*/d8=_G_A();{d8^=_G_A();_S_F(_z80_szp[d8]);_S_A(d8);}break;
      case 0xb0:/*OR B*/d8=_G_B();{d8|=_G_A();_S_F(_z80_szp[d8]);_S_A(d8);}break;
      case 0xb1:/*OR C*/d8=_G_C();{d8|=_G_A();_S_F(_z80_szp[d8]);_S_A(d8);}break;
      case 0xb2:/*OR D*/d8=_G_D();{d8|=_G_A();_S_F(_z80_szp[d8]);_S_A(d8);}break;
      case 0xb3:/*OR E*/d8=_G_E();{d8|=_G_A();_S_F(_z80_szp[d8]);_S_A(d8);}break;
      case 0xb4:/*OR H*/d8=_G_H();{d8|=_G_A();_S_F(_z80_szp[d8]);_S_A(d8);}break;
      case 0xb5:/*OR L*/d8=_G_L();{d8|=_G_A();_S_F(_z80_szp[d8]);_S_A(d8);}break;
      case 0xb6:/*OR,(HL/IX+d/IY+d)*/_ADDR(addr,5);_MR(addr,d8);{d8|=_G_A();_S_F(_z80_szp[d8]);_S_A(d8);}break;
      case 0xb7:/*OR A*/d8=_G_A();{d8|=_G_A();_S_F(_z80_szp[d8]);_S_A(d8);}break;
      case 0xb8:/*CP B*/d8=_G_B();{uint8_t acc=_G_A();int32_t res=(uint32_t)((int)acc-(int)d8);_S_F(_CP_FLAGS(acc,d8,res));}break;
      case 0xb9:/*CP C*/d8=_G_C();{uint8_t acc=_G_A();int32_t res=(uint32_t)((int)acc-(int)d8);_S_F(_CP_FLAGS(acc,d8,res));}break;
      case 0xba:/*CP D*/d8=_G_D();{uint8_t acc=_G_A();int32_t res=(uint32_t)((int)acc-(int)d8);_S_F(_CP_FLAGS(acc,d8,res));}break;
      case 0xbb:/*CP E*/d8=_G_E();{uint8_t acc=_G_A();int32_t res=(uint32_t)((int)acc-(int)d8);_S_F(_CP_FLAGS(acc,d8,res));}break;
      case 0xbc:/*CP H*/d8=_G_H();{uint8_t acc=_G_A();int32_t res=(uint32_t)((int)acc-(int)d8);_S_F(_CP_FLAGS(acc,d8,res));}break;
      case 0xbd:/*CP L*/d8=_G_L();{uint8_t acc=_G_A();int32_t res=(uint32_t)((int)acc-(int)d8);_S_F(_CP_FLAGS(acc,d8,res));}break;
      case 0xbe:/*CP,(HL/IX+d/IY+d)*/_ADDR(addr,5);_MR(addr,d8);{uint8_t acc=_G_A();int32_t res=(uint32_t)((int)acc-(int)d8);_S_F(_CP_FLAGS(acc,d8,res));}break;
      case 0xbf:/*CP A*/d8=_G_A();{uint8_t acc=_G_A();int32_t res=(uint32_t)((int)acc-(int)d8);_S_F(_CP_FLAGS(acc,d8,res));}break;
      case 0xc0:/*RET NZ*/_T(1);if (!(_G_F()&Z80_ZF)){uint8_t w,z;d16=_G_SP();_MR(d16++,z);_MR(d16++,w);_S_SP(d16);pc=(w<<8)|z;_S_WZ(pc);}break;
      case 0xc1:/*POP BC*/addr=_G_SP();_MR(addr++,d8);d16=d8;_MR(addr++,d8);d16|=d8<<8;_S_BC(d16);_S_SP(addr);break;
      case 0xc2:/*JP NZ,nn*/_IMM16(addr);if(!(_G_F()&Z80_ZF)){pc=addr;}break;
      case 0xc3:/*JP nn*/_IMM16(pc);break;
      case 0xc4:/*CALL NZ,nn*/_IMM16(addr);if(!(_G_F()&Z80_ZF)){_T(1);uint16_t sp=_G_SP();_MW(--sp,pc>>8);_MW(--sp,pc);_S_SP(sp);pc=addr;}break;
      case 0xc5:/*PUSH BC*/_T(1);addr=_G_SP();d16=_G_BC();_MW(--addr,d16>>8);_MW(--addr,d16);_S_SP(addr);break;
      case 0xc6:/*ADD n*/_IMM8(d8);{uint8_t acc=_G_A();uint32_t res=acc+d8;_S_F(_ADD_FLAGS(acc,d8,res));_S_A(res);}break;
      case 0xc7:/*RST 0x0*/_T(1);d16= _G_SP();_MW(--d16, pc>>8);_MW(--d16, pc);_S_SP(d16);pc=0x0;_S_WZ(pc);break;
      case 0xc8:/*RET Z*/_T(1);if ((_G_F()&Z80_ZF)){uint8_t w,z;d16=_G_SP();_MR(d16++,z);_MR(d16++,w);_S_SP(d16);pc=(w<<8)|z;_S_WZ(pc);}break;
      case 0xc9:/*RET*/d16=_G_SP();_MR(d16++,d8);pc=d8;_MR(d16++,d8);pc|=d8<<8;_S_SP(d16);_S_WZ(pc);break;
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
      case 0xcc:/*CALL Z,nn*/_IMM16(addr);if((_G_F()&Z80_ZF)){_T(1);uint16_t sp=_G_SP();_MW(--sp,pc>>8);_MW(--sp,pc);_S_SP(sp);pc=addr;}break;
      case 0xcd:/*CALL nn*/_IMM16(addr);_T(1);d16=_G_SP();_MW(--d16,pc>>8);_MW(--d16,pc);_S_SP(d16);pc=addr;break;
      case 0xce:/*ADC n*/_IMM8(d8);{uint8_t acc=_G_A();uint32_t res=acc+d8+(_G_F()&Z80_CF);_S_F(_ADD_FLAGS(acc,d8,res));_S_A(res);}break;
      case 0xcf:/*RST 0x8*/_T(1);d16= _G_SP();_MW(--d16, pc>>8);_MW(--d16, pc);_S_SP(d16);pc=0x8;_S_WZ(pc);break;
      case 0xd0:/*RET NC*/_T(1);if (!(_G_F()&Z80_CF)){uint8_t w,z;d16=_G_SP();_MR(d16++,z);_MR(d16++,w);_S_SP(d16);pc=(w<<8)|z;_S_WZ(pc);}break;
      case 0xd1:/*POP DE*/addr=_G_SP();_MR(addr++,d8);d16=d8;_MR(addr++,d8);d16|=d8<<8;_S_DE(d16);_S_SP(addr);break;
      case 0xd2:/*JP NC,nn*/_IMM16(addr);if(!(_G_F()&Z80_CF)){pc=addr;}break;
      case 0xd3:/*OUT (n),A*/{_IMM8(d8);uint8_t a=_G_A();addr=(a<<8)|d8;_OUT(addr,a);_S_WZ((addr&0xFF00)|((addr+1)&0x00FF));}break;
      case 0xd4:/*CALL NC,nn*/_IMM16(addr);if(!(_G_F()&Z80_CF)){_T(1);uint16_t sp=_G_SP();_MW(--sp,pc>>8);_MW(--sp,pc);_S_SP(sp);pc=addr;}break;
      case 0xd5:/*PUSH DE*/_T(1);addr=_G_SP();d16=_G_DE();_MW(--addr,d16>>8);_MW(--addr,d16);_S_SP(addr);break;
      case 0xd6:/*SUB n*/_IMM8(d8);{uint8_t acc=_G_A();uint32_t res=(uint32_t)((int)acc-(int)d8);_S_F(_SUB_FLAGS(acc,d8,res));_S_A(res);}break;
      case 0xd7:/*RST 0x10*/_T(1);d16= _G_SP();_MW(--d16, pc>>8);_MW(--d16, pc);_S_SP(d16);pc=0x10;_S_WZ(pc);break;
      case 0xd8:/*RET C*/_T(1);if ((_G_F()&Z80_CF)){uint8_t w,z;d16=_G_SP();_MR(d16++,z);_MR(d16++,w);_S_SP(d16);pc=(w<<8)|z;_S_WZ(pc);}break;
      case 0xd9:/*EXX*/{r0=_z80_flush_r0(ws,r0,r2);const uint64_t rx=r3;r3=(r3&0xffff)|(r0&0xffffffffffff0000);r0=(r0&0xffff)|(rx&0xffffffffffff0000);ws=_z80_map_regs(r0, r1, r2);}break;
      case 0xda:/*JP C,nn*/_IMM16(addr);if((_G_F()&Z80_CF)){pc=addr;}break;
      case 0xdb:/*IN A,(n)*/{_IMM8(d8);uint8_t a=_G_A();addr=(a<<8)|d8;_IN(addr++,a);_S_A(a);_S_WZ(addr);}break;
      case 0xdc:/*CALL C,nn*/_IMM16(addr);if((_G_F()&Z80_CF)){_T(1);uint16_t sp=_G_SP();_MW(--sp,pc>>8);_MW(--sp,pc);_S_SP(sp);pc=addr;}break;
      case 0xdd:/*DD prefix*/map_bits|=_BIT_USE_IX;continue;break;
      case 0xde:/*SBC n*/_IMM8(d8);{uint8_t acc=_G_A();uint32_t res=(uint32_t)((int)acc-(int)d8-(_G_F()&Z80_CF));_S_F(_SUB_FLAGS(acc,d8,res));_S_A(res);}break;
      case 0xdf:/*RST 0x18*/_T(1);d16= _G_SP();_MW(--d16, pc>>8);_MW(--d16, pc);_S_SP(d16);pc=0x18;_S_WZ(pc);break;
      case 0xe0:/*RET PO*/_T(1);if (!(_G_F()&Z80_PF)){uint8_t w,z;d16=_G_SP();_MR(d16++,z);_MR(d16++,w);_S_SP(d16);pc=(w<<8)|z;_S_WZ(pc);}break;
      case 0xe1:/*POP HL*/addr=_G_SP();_MR(addr++,d8);d16=d8;_MR(addr++,d8);d16|=d8<<8;_S_HL(d16);_S_SP(addr);break;
      case 0xe2:/*JP PO,nn*/_IMM16(addr);if(!(_G_F()&Z80_PF)){pc=addr;}break;
      case 0xe3:/*EX (SP),HL*/{_T(3);addr=_G_SP();d16=_G_HL();uint8_t l,h;_MR(addr,l);_MR(addr+1,h);_MW(addr,d16);_MW(addr+1,d16>>8);d16=(h<<8)|l;_S_HL(d16);_S_WZ(d16);}break;
      case 0xe4:/*CALL PO,nn*/_IMM16(addr);if(!(_G_F()&Z80_PF)){_T(1);uint16_t sp=_G_SP();_MW(--sp,pc>>8);_MW(--sp,pc);_S_SP(sp);pc=addr;}break;
      case 0xe5:/*PUSH HL*/_T(1);addr=_G_SP();d16=_G_HL();_MW(--addr,d16>>8);_MW(--addr,d16);_S_SP(addr);break;
      case 0xe6:/*AND n*/_IMM8(d8);{d8&=_G_A();_S_F(_z80_szp[d8]|Z80_HF);_S_A(d8);}break;
      case 0xe7:/*RST 0x20*/_T(1);d16= _G_SP();_MW(--d16, pc>>8);_MW(--d16, pc);_S_SP(d16);pc=0x20;_S_WZ(pc);break;
      case 0xe8:/*RET PE*/_T(1);if ((_G_F()&Z80_PF)){uint8_t w,z;d16=_G_SP();_MR(d16++,z);_MR(d16++,w);_S_SP(d16);pc=(w<<8)|z;_S_WZ(pc);}break;
      case 0xe9:/*JP HL*/pc=_G_HL();break;
      case 0xea:/*JP PE,nn*/_IMM16(addr);if((_G_F()&Z80_PF)){pc=addr;}break;
      case 0xeb:/*EX DE,HL*/{r0=_z80_flush_r0(ws,r0,r2);uint16_t de=_G16(r0,_DE);uint16_t hl=_G16(r0,_HL);_S16(r0,_DE,hl);_S16(r0,_HL,de);ws=_z80_map_regs(r0,r1,r2);}break;
      case 0xec:/*CALL PE,nn*/_IMM16(addr);if((_G_F()&Z80_PF)){_T(1);uint16_t sp=_G_SP();_MW(--sp,pc>>8);_MW(--sp,pc);_S_SP(sp);pc=addr;}break;
      case 0xED: {
        _FETCH(op);
        switch(op) {
          case 0x40:/*IN B,(C)*/{addr=_G_BC();_IN(addr++,d8);_S_WZ(addr);uint8_t f=(_G_F()&Z80_CF)|_z80_szp[d8];_S8(ws,_F,f);_S_B(d8);}break;
          case 0x41:/*OUT (C),B*/addr=_G_BC();_OUT(addr++,_G_B());_S_WZ(addr);break;
          case 0x42:/*SBC HL,BC*/{uint16_t acc=_G_HL();_S_WZ(acc+1);d16=_G_BC();uint32_t r=acc-d16-(_G_F()&Z80_CF);uint8_t f=Z80_NF|(((d16^acc)&(acc^r)&0x8000)>>13);_S_HL(r);f|=((acc^r^d16)>>8) & Z80_HF;f|=(r>>16)&Z80_CF;f|=(r>>8)&(Z80_SF|Z80_YF|Z80_XF);f|=(r&0xFFFF)?0:Z80_ZF;_S_F(f);_T(7);}break;
          case 0x43:/*LD (nn),BC*/_IMM16(addr);d16=_G_BC();_MW(addr++,d16&0xFF);_MW(addr,d16>>8);_S_WZ(addr);break;
          case 0x44:/*NEG*/d8=_G_A();_S_A(0);{uint8_t acc=_G_A();uint32_t res=(uint32_t)((int)acc-(int)d8);_S_F(_SUB_FLAGS(acc,d8,res));_S_A(res);}break;
          case 0x45:/*RETN*/pins|=Z80_RETI;d16=_G_SP();_MR(d16++,d8);pc=d8;_MR(d16++,d8);pc|=d8<<8;_S_SP(d16);_S_WZ(pc);if (r2&_BIT_IFF2){r2|=_BIT_IFF1;}else{r2&=~_BIT_IFF1;}break;
          case 0x46:/*IM 0*/_S_IM(0);break;
          case 0x47:/*LD I,A*/_T(1);_S_I(_G_A());break;
          case 0x48:/*IN C,(C)*/{addr=_G_BC();_IN(addr++,d8);_S_WZ(addr);uint8_t f=(_G_F()&Z80_CF)|_z80_szp[d8];_S8(ws,_F,f);_S_C(d8);}break;
          case 0x49:/*OUT (C),C*/addr=_G_BC();_OUT(addr++,_G_C());_S_WZ(addr);break;
          case 0x4a:/*ADC HL,BC*/{uint16_t acc=_G_HL();_S_WZ(acc+1);d16=_G_BC();uint32_t r=acc+d16+(_G_F()&Z80_CF);_S_HL(r);uint8_t f=((d16^acc^0x8000)&(d16^r)&0x8000)>>13;f|=((acc^r^d16)>>8)&Z80_HF;f|=(r>>16)&Z80_CF;f|=(r>>8)&(Z80_SF|Z80_YF|Z80_XF);f|=(r&0xFFFF)?0:Z80_ZF;_S_F(f);_T(7);}break;
          case 0x4b:/*LD BC,(nn)*/_IMM16(addr);_MR(addr++,d8);d16=d8;_MR(addr,d8);d16|=d8<<8;_S_BC(d16);_S_WZ(addr);break;
          case 0x4c:/*NEG*/d8=_G_A();_S_A(0);{uint8_t acc=_G_A();uint32_t res=(uint32_t)((int)acc-(int)d8);_S_F(_SUB_FLAGS(acc,d8,res));_S_A(res);}break;
          case 0x4d:/*RETI*/pins|=Z80_RETI;d16=_G_SP();_MR(d16++,d8);pc=d8;_MR(d16++,d8);pc|=d8<<8;_S_SP(d16);_S_WZ(pc);if (r2&_BIT_IFF2){r2|=_BIT_IFF1;}else{r2&=~_BIT_IFF1;}break;
          case 0x4e:/*IM 0*/_S_IM(0);break;
          case 0x4f:/*LD R,A*/_T(1);_S_R(_G_A());break;
          case 0x50:/*IN D,(C)*/{addr=_G_BC();_IN(addr++,d8);_S_WZ(addr);uint8_t f=(_G_F()&Z80_CF)|_z80_szp[d8];_S8(ws,_F,f);_S_D(d8);}break;
          case 0x51:/*OUT (C),D*/addr=_G_BC();_OUT(addr++,_G_D());_S_WZ(addr);break;
          case 0x52:/*SBC HL,DE*/{uint16_t acc=_G_HL();_S_WZ(acc+1);d16=_G_DE();uint32_t r=acc-d16-(_G_F()&Z80_CF);uint8_t f=Z80_NF|(((d16^acc)&(acc^r)&0x8000)>>13);_S_HL(r);f|=((acc^r^d16)>>8) & Z80_HF;f|=(r>>16)&Z80_CF;f|=(r>>8)&(Z80_SF|Z80_YF|Z80_XF);f|=(r&0xFFFF)?0:Z80_ZF;_S_F(f);_T(7);}break;
          case 0x53:/*LD (nn),DE*/_IMM16(addr);d16=_G_DE();_MW(addr++,d16&0xFF);_MW(addr,d16>>8);_S_WZ(addr);break;
          case 0x54:/*NEG*/d8=_G_A();_S_A(0);{uint8_t acc=_G_A();uint32_t res=(uint32_t)((int)acc-(int)d8);_S_F(_SUB_FLAGS(acc,d8,res));_S_A(res);}break;
          case 0x55:/*RETN*/pins|=Z80_RETI;d16=_G_SP();_MR(d16++,d8);pc=d8;_MR(d16++,d8);pc|=d8<<8;_S_SP(d16);_S_WZ(pc);if (r2&_BIT_IFF2){r2|=_BIT_IFF1;}else{r2&=~_BIT_IFF1;}break;
          case 0x56:/*IM 1*/_S_IM(1);break;
          case 0x57:/*LD A,I*/_T(1);d8=_G_I();_S_A(d8);_S_F(_SZIFF2_FLAGS(d8));break;
          case 0x58:/*IN E,(C)*/{addr=_G_BC();_IN(addr++,d8);_S_WZ(addr);uint8_t f=(_G_F()&Z80_CF)|_z80_szp[d8];_S8(ws,_F,f);_S_E(d8);}break;
          case 0x59:/*OUT (C),E*/addr=_G_BC();_OUT(addr++,_G_E());_S_WZ(addr);break;
          case 0x5a:/*ADC HL,DE*/{uint16_t acc=_G_HL();_S_WZ(acc+1);d16=_G_DE();uint32_t r=acc+d16+(_G_F()&Z80_CF);_S_HL(r);uint8_t f=((d16^acc^0x8000)&(d16^r)&0x8000)>>13;f|=((acc^r^d16)>>8)&Z80_HF;f|=(r>>16)&Z80_CF;f|=(r>>8)&(Z80_SF|Z80_YF|Z80_XF);f|=(r&0xFFFF)?0:Z80_ZF;_S_F(f);_T(7);}break;
          case 0x5b:/*LD DE,(nn)*/_IMM16(addr);_MR(addr++,d8);d16=d8;_MR(addr,d8);d16|=d8<<8;_S_DE(d16);_S_WZ(addr);break;
          case 0x5c:/*NEG*/d8=_G_A();_S_A(0);{uint8_t acc=_G_A();uint32_t res=(uint32_t)((int)acc-(int)d8);_S_F(_SUB_FLAGS(acc,d8,res));_S_A(res);}break;
          case 0x5d:/*RETN*/pins|=Z80_RETI;d16=_G_SP();_MR(d16++,d8);pc=d8;_MR(d16++,d8);pc|=d8<<8;_S_SP(d16);_S_WZ(pc);if (r2&_BIT_IFF2){r2|=_BIT_IFF1;}else{r2&=~_BIT_IFF1;}break;
          case 0x5e:/*IM 2*/_S_IM(2);break;
          case 0x5f:/*LD A,R*/_T(1);d8=_G_R();_S_A(d8);_S_F(_SZIFF2_FLAGS(d8));break;
          case 0x60:/*IN H,(C)*/{addr=_G_BC();_IN(addr++,d8);_S_WZ(addr);uint8_t f=(_G_F()&Z80_CF)|_z80_szp[d8];_S8(ws,_F,f);_S_H(d8);}break;
          case 0x61:/*OUT (C),H*/addr=_G_BC();_OUT(addr++,_G_H());_S_WZ(addr);break;
          case 0x62:/*SBC HL,HL*/{uint16_t acc=_G_HL();_S_WZ(acc+1);d16=_G_HL();uint32_t r=acc-d16-(_G_F()&Z80_CF);uint8_t f=Z80_NF|(((d16^acc)&(acc^r)&0x8000)>>13);_S_HL(r);f|=((acc^r^d16)>>8) & Z80_HF;f|=(r>>16)&Z80_CF;f|=(r>>8)&(Z80_SF|Z80_YF|Z80_XF);f|=(r&0xFFFF)?0:Z80_ZF;_S_F(f);_T(7);}break;
          case 0x63:/*LD (nn),HL*/_IMM16(addr);d16=_G_HL();_MW(addr++,d16&0xFF);_MW(addr,d16>>8);_S_WZ(addr);break;
          case 0x64:/*NEG*/d8=_G_A();_S_A(0);{uint8_t acc=_G_A();uint32_t res=(uint32_t)((int)acc-(int)d8);_S_F(_SUB_FLAGS(acc,d8,res));_S_A(res);}break;
          case 0x65:/*RETN*/pins|=Z80_RETI;d16=_G_SP();_MR(d16++,d8);pc=d8;_MR(d16++,d8);pc|=d8<<8;_S_SP(d16);_S_WZ(pc);if (r2&_BIT_IFF2){r2|=_BIT_IFF1;}else{r2&=~_BIT_IFF1;}break;
          case 0x66:/*IM 0*/_S_IM(0);break;
          case 0x67:/*RRD*/{addr=_G_HL();uint8_t a=_G_A();_MR(addr,d8);uint8_t l=a&0x0F;a=(a&0xF0)|(d8&0x0F);_S_A(a);d8=(d8>>4)|(l<<4);_MW(addr++,d8);_S_WZ(addr);_S_F((_G_F()&Z80_CF)|_z80_szp[a]);_T(4);}break;
          case 0x68:/*IN L,(C)*/{addr=_G_BC();_IN(addr++,d8);_S_WZ(addr);uint8_t f=(_G_F()&Z80_CF)|_z80_szp[d8];_S8(ws,_F,f);_S_L(d8);}break;
          case 0x69:/*OUT (C),L*/addr=_G_BC();_OUT(addr++,_G_L());_S_WZ(addr);break;
          case 0x6a:/*ADC HL,HL*/{uint16_t acc=_G_HL();_S_WZ(acc+1);d16=_G_HL();uint32_t r=acc+d16+(_G_F()&Z80_CF);_S_HL(r);uint8_t f=((d16^acc^0x8000)&(d16^r)&0x8000)>>13;f|=((acc^r^d16)>>8)&Z80_HF;f|=(r>>16)&Z80_CF;f|=(r>>8)&(Z80_SF|Z80_YF|Z80_XF);f|=(r&0xFFFF)?0:Z80_ZF;_S_F(f);_T(7);}break;
          case 0x6b:/*LD HL,(nn)*/_IMM16(addr);_MR(addr++,d8);d16=d8;_MR(addr,d8);d16|=d8<<8;_S_HL(d16);_S_WZ(addr);break;
          case 0x6c:/*NEG*/d8=_G_A();_S_A(0);{uint8_t acc=_G_A();uint32_t res=(uint32_t)((int)acc-(int)d8);_S_F(_SUB_FLAGS(acc,d8,res));_S_A(res);}break;
          case 0x6d:/*RETN*/pins|=Z80_RETI;d16=_G_SP();_MR(d16++,d8);pc=d8;_MR(d16++,d8);pc|=d8<<8;_S_SP(d16);_S_WZ(pc);if (r2&_BIT_IFF2){r2|=_BIT_IFF1;}else{r2&=~_BIT_IFF1;}break;
          case 0x6e:/*IM 0*/_S_IM(0);break;
          case 0x6f:/*RLD*/{addr=_G_HL();uint8_t a=_G_A();_MR(addr,d8);uint8_t l=a&0x0F;a=(a&0xF0)|(d8>>4);_S_A(a);d8=(d8<<4)|l;_MW(addr++,d8);_S_WZ(addr);_S_F((_G_F()&Z80_CF)|_z80_szp[a]);_T(4);}break;
          case 0x70:/*IN HL,(C)*/{addr=_G_BC();_IN(addr++,d8);_S_WZ(addr);uint8_t f=(_G_F()&Z80_CF)|_z80_szp[d8];_S8(ws,_F,f);}break;
          case 0x71:/*OUT (C),HL*/addr=_G_BC();_OUT(addr++,0);_S_WZ(addr);break;
          case 0x72:/*SBC HL,SP*/{uint16_t acc=_G_HL();_S_WZ(acc+1);d16=_G_SP();uint32_t r=acc-d16-(_G_F()&Z80_CF);uint8_t f=Z80_NF|(((d16^acc)&(acc^r)&0x8000)>>13);_S_HL(r);f|=((acc^r^d16)>>8) & Z80_HF;f|=(r>>16)&Z80_CF;f|=(r>>8)&(Z80_SF|Z80_YF|Z80_XF);f|=(r&0xFFFF)?0:Z80_ZF;_S_F(f);_T(7);}break;
          case 0x73:/*LD (nn),SP*/_IMM16(addr);d16=_G_SP();_MW(addr++,d16&0xFF);_MW(addr,d16>>8);_S_WZ(addr);break;
          case 0x74:/*NEG*/d8=_G_A();_S_A(0);{uint8_t acc=_G_A();uint32_t res=(uint32_t)((int)acc-(int)d8);_S_F(_SUB_FLAGS(acc,d8,res));_S_A(res);}break;
          case 0x75:/*RETN*/pins|=Z80_RETI;d16=_G_SP();_MR(d16++,d8);pc=d8;_MR(d16++,d8);pc|=d8<<8;_S_SP(d16);_S_WZ(pc);if (r2&_BIT_IFF2){r2|=_BIT_IFF1;}else{r2&=~_BIT_IFF1;}break;
          case 0x76:/*IM 1*/_S_IM(1);break;
          case 0x77:/*NOP (ED)*/ break;
          case 0x78:/*IN A,(C)*/{addr=_G_BC();_IN(addr++,d8);_S_WZ(addr);uint8_t f=(_G_F()&Z80_CF)|_z80_szp[d8];_S8(ws,_F,f);_S_A(d8);}break;
          case 0x79:/*OUT (C),A*/addr=_G_BC();_OUT(addr++,_G_A());_S_WZ(addr);break;
          case 0x7a:/*ADC HL,SP*/{uint16_t acc=_G_HL();_S_WZ(acc+1);d16=_G_SP();uint32_t r=acc+d16+(_G_F()&Z80_CF);_S_HL(r);uint8_t f=((d16^acc^0x8000)&(d16^r)&0x8000)>>13;f|=((acc^r^d16)>>8)&Z80_HF;f|=(r>>16)&Z80_CF;f|=(r>>8)&(Z80_SF|Z80_YF|Z80_XF);f|=(r&0xFFFF)?0:Z80_ZF;_S_F(f);_T(7);}break;
          case 0x7b:/*LD SP,(nn)*/_IMM16(addr);_MR(addr++,d8);d16=d8;_MR(addr,d8);d16|=d8<<8;_S_SP(d16);_S_WZ(addr);break;
          case 0x7c:/*NEG*/d8=_G_A();_S_A(0);{uint8_t acc=_G_A();uint32_t res=(uint32_t)((int)acc-(int)d8);_S_F(_SUB_FLAGS(acc,d8,res));_S_A(res);}break;
          case 0x7d:/*RETN*/pins|=Z80_RETI;d16=_G_SP();_MR(d16++,d8);pc=d8;_MR(d16++,d8);pc|=d8<<8;_S_SP(d16);_S_WZ(pc);if (r2&_BIT_IFF2){r2|=_BIT_IFF1;}else{r2&=~_BIT_IFF1;}break;
          case 0x7e:/*IM 2*/_S_IM(2);break;
          case 0x7f:/*NOP (ED)*/ break;
          case 0xa0:/*LDI*/{uint16_t hl=_G_HL();uint16_t de=_G_DE();_MR(hl,d8);_MW(de,d8);hl++;de++;_S_HL(hl);_S_DE(de);_T(2);d8+=_G_A();uint8_t f=_G_F()&(Z80_SF|Z80_ZF|Z80_CF);if(d8&0x02){f|=Z80_YF;}if(d8&0x08){f|=Z80_XF;}uint16_t bc=_G_BC();bc--;_S_BC(bc);if(bc){f|=Z80_VF;}_S_F(f);}break;
          case 0xa1:/*CPI*/{uint16_t hl = _G_HL();_MR(hl,d8);uint16_t wz = _G_WZ();hl++;wz++;_S_WZ(wz);_S_HL(hl);_T(5);int r=((int)_G_A())-d8;uint8_t f=(_G_F()&Z80_CF)|Z80_NF|_SZ(r);if((r&0x0F)>(_G_A()&0x0F)){f|=Z80_HF;r--;}if(r&0x02){f|=Z80_YF;}if(r&0x08){f|=Z80_XF;}uint16_t bc=_G_BC();bc--;_S_BC(bc);if(bc){f|=Z80_VF;}_S8(ws,_F,f);}break;
          case 0xa2:/*INI*/{_T(1);addr=_G_BC();uint16_t hl=_G_HL();_IN(addr,d8);_MW(hl,d8);uint8_t b=_G_B();uint8_t c=_G_C();b--;addr++;hl++;c++;_S_B(b);_S_HL(hl);_S_WZ(addr);uint8_t f=(b?(b&Z80_SF):Z80_ZF)|(b&(Z80_XF|Z80_YF));if(d8&Z80_SF){f|=Z80_NF;}uint32_t t=(uint32_t)(c&0xFF)+d8;if(t&0x100){f|=Z80_HF|Z80_CF;}f|=_z80_szp[((uint8_t)(t&0x07))^b]&Z80_PF;_S_F(f);}break;
          case 0xa3:/*OUTI*/{_T(1);uint16_t hl=_G_HL();_MR(hl,d8);uint8_t b=_G_B();b--;_S_B(b);addr=_G_BC();_OUT(addr,d8);addr++; hl++;_S_HL(hl);_S_WZ(addr);uint8_t f=(b?(b&Z80_SF):Z80_ZF)|(b&(Z80_XF|Z80_YF));if(d8&Z80_SF){f|=Z80_NF;}uint32_t t=(uint32_t)_G_L()+(uint32_t)d8;if (t&0x0100){f|=Z80_HF|Z80_CF;}f|=_z80_szp[((uint8_t)(t&0x07))^b]&Z80_PF;_S_F(f);}break;
          case 0xa8:/*LDD*/{uint16_t hl=_G_HL();uint16_t de=_G_DE();_MR(hl,d8);_MW(de,d8);hl--;de--;_S_HL(hl);_S_DE(de);_T(2);d8+=_G_A();uint8_t f=_G_F()&(Z80_SF|Z80_ZF|Z80_CF);if(d8&0x02){f|=Z80_YF;}if(d8&0x08){f|=Z80_XF;}uint16_t bc=_G_BC();bc--;_S_BC(bc);if(bc){f|=Z80_VF;}_S_F(f);}break;
          case 0xa9:/*CPD*/{uint16_t hl = _G_HL();_MR(hl,d8);uint16_t wz = _G_WZ();hl--;wz--;_S_WZ(wz);_S_HL(hl);_T(5);int r=((int)_G_A())-d8;uint8_t f=(_G_F()&Z80_CF)|Z80_NF|_SZ(r);if((r&0x0F)>(_G_A()&0x0F)){f|=Z80_HF;r--;}if(r&0x02){f|=Z80_YF;}if(r&0x08){f|=Z80_XF;}uint16_t bc=_G_BC();bc--;_S_BC(bc);if(bc){f|=Z80_VF;}_S8(ws,_F,f);}break;
          case 0xaa:/*IND*/{_T(1);addr=_G_BC();uint16_t hl=_G_HL();_IN(addr,d8);_MW(hl,d8);uint8_t b=_G_B();uint8_t c=_G_C();b--;addr--;hl--;c--;_S_B(b);_S_HL(hl);_S_WZ(addr);uint8_t f=(b?(b&Z80_SF):Z80_ZF)|(b&(Z80_XF|Z80_YF));if(d8&Z80_SF){f|=Z80_NF;}uint32_t t=(uint32_t)(c&0xFF)+d8;if(t&0x100){f|=Z80_HF|Z80_CF;}f|=_z80_szp[((uint8_t)(t&0x07))^b]&Z80_PF;_S_F(f);}break;
          case 0xab:/*OUTD*/{_T(1);uint16_t hl=_G_HL();_MR(hl,d8);uint8_t b=_G_B();b--;_S_B(b);addr=_G_BC();_OUT(addr,d8);addr--;hl--;_S_HL(hl);_S_WZ(addr);uint8_t f=(b?(b&Z80_SF):Z80_ZF)|(b&(Z80_XF|Z80_YF));if(d8&Z80_SF){f|=Z80_NF;}uint32_t t=(uint32_t)_G_L()+(uint32_t)d8;if (t&0x0100){f|=Z80_HF|Z80_CF;}f|=_z80_szp[((uint8_t)(t&0x07))^b]&Z80_PF;_S_F(f);}break;
          case 0xb0:/*LDIR*/{uint16_t hl=_G_HL();uint16_t de=_G_DE();_MR(hl,d8);_MW(de,d8);hl++;de++;_S_HL(hl);_S_DE(de);_T(2);d8+=_G_A();uint8_t f=_G_F()&(Z80_SF|Z80_ZF|Z80_CF);if(d8&0x02){f|=Z80_YF;}if(d8&0x08){f|=Z80_XF;}uint16_t bc=_G_BC();bc--;_S_BC(bc);if(bc){f|=Z80_VF;}_S_F(f);if(bc){pc-=2;_S_WZ(pc+1);_T(5);}}break;
          case 0xb1:/*CPIR*/{uint16_t hl = _G_HL();_MR(hl,d8);uint16_t wz = _G_WZ();hl++;wz++;_S_WZ(wz);_S_HL(hl);_T(5);int r=((int)_G_A())-d8;uint8_t f=(_G_F()&Z80_CF)|Z80_NF|_SZ(r);if((r&0x0F)>(_G_A()&0x0F)){f|=Z80_HF;r--;}if(r&0x02){f|=Z80_YF;}if(r&0x08){f|=Z80_XF;}uint16_t bc=_G_BC();bc--;_S_BC(bc);if(bc){f|=Z80_VF;}_S8(ws,_F,f);if(bc&&!(f&Z80_ZF)){pc-=2;_S_WZ(pc+1);_T(5);}}break;
          case 0xb2:/*INIR*/{_T(1);addr=_G_BC();uint16_t hl=_G_HL();_IN(addr,d8);_MW(hl,d8);uint8_t b=_G_B();uint8_t c=_G_C();b--;addr++;hl++;c++;_S_B(b);_S_HL(hl);_S_WZ(addr);uint8_t f=(b?(b&Z80_SF):Z80_ZF)|(b&(Z80_XF|Z80_YF));if(d8&Z80_SF){f|=Z80_NF;}uint32_t t=(uint32_t)(c&0xFF)+d8;if(t&0x100){f|=Z80_HF|Z80_CF;}f|=_z80_szp[((uint8_t)(t&0x07))^b]&Z80_PF;_S_F(f);if(b){pc-=2;_T(5);}}break;
          case 0xb3:/*OTIR*/{_T(1);uint16_t hl=_G_HL();_MR(hl,d8);uint8_t b=_G_B();b--;_S_B(b);addr=_G_BC();_OUT(addr,d8);addr++; hl++;_S_HL(hl);_S_WZ(addr);uint8_t f=(b?(b&Z80_SF):Z80_ZF)|(b&(Z80_XF|Z80_YF));if(d8&Z80_SF){f|=Z80_NF;}uint32_t t=(uint32_t)_G_L()+(uint32_t)d8;if (t&0x0100){f|=Z80_HF|Z80_CF;}f|=_z80_szp[((uint8_t)(t&0x07))^b]&Z80_PF;_S_F(f);if(b){pc-=2;_T(5);}}break;
          case 0xb8:/*LDDR*/{uint16_t hl=_G_HL();uint16_t de=_G_DE();_MR(hl,d8);_MW(de,d8);hl--;de--;_S_HL(hl);_S_DE(de);_T(2);d8+=_G_A();uint8_t f=_G_F()&(Z80_SF|Z80_ZF|Z80_CF);if(d8&0x02){f|=Z80_YF;}if(d8&0x08){f|=Z80_XF;}uint16_t bc=_G_BC();bc--;_S_BC(bc);if(bc){f|=Z80_VF;}_S_F(f);if(bc){pc-=2;_S_WZ(pc+1);_T(5);}}break;
          case 0xb9:/*CPDR*/{uint16_t hl = _G_HL();_MR(hl,d8);uint16_t wz = _G_WZ();hl--;wz--;_S_WZ(wz);_S_HL(hl);_T(5);int r=((int)_G_A())-d8;uint8_t f=(_G_F()&Z80_CF)|Z80_NF|_SZ(r);if((r&0x0F)>(_G_A()&0x0F)){f|=Z80_HF;r--;}if(r&0x02){f|=Z80_YF;}if(r&0x08){f|=Z80_XF;}uint16_t bc=_G_BC();bc--;_S_BC(bc);if(bc){f|=Z80_VF;}_S8(ws,_F,f);if(bc&&!(f&Z80_ZF)){pc-=2;_S_WZ(pc+1);_T(5);}}break;
          case 0xba:/*INDR*/{_T(1);addr=_G_BC();uint16_t hl=_G_HL();_IN(addr,d8);_MW(hl,d8);uint8_t b=_G_B();uint8_t c=_G_C();b--;addr--;hl--;c--;_S_B(b);_S_HL(hl);_S_WZ(addr);uint8_t f=(b?(b&Z80_SF):Z80_ZF)|(b&(Z80_XF|Z80_YF));if(d8&Z80_SF){f|=Z80_NF;}uint32_t t=(uint32_t)(c&0xFF)+d8;if(t&0x100){f|=Z80_HF|Z80_CF;}f|=_z80_szp[((uint8_t)(t&0x07))^b]&Z80_PF;_S_F(f);if(b){pc-=2;_T(5);}}break;
          case 0xbb:/*OTDR*/{_T(1);uint16_t hl=_G_HL();_MR(hl,d8);uint8_t b=_G_B();b--;_S_B(b);addr=_G_BC();_OUT(addr,d8);addr--;hl--;_S_HL(hl);_S_WZ(addr);uint8_t f=(b?(b&Z80_SF):Z80_ZF)|(b&(Z80_XF|Z80_YF));if(d8&Z80_SF){f|=Z80_NF;}uint32_t t=(uint32_t)_G_L()+(uint32_t)d8;if (t&0x0100){f|=Z80_HF|Z80_CF;}f|=_z80_szp[((uint8_t)(t&0x07))^b]&Z80_PF;_S_F(f);if(b){pc-=2;_T(5);}}break;
          default: break;
        }
      }
      break;
      case 0xee:/*XOR n*/_IMM8(d8);{d8^=_G_A();_S_F(_z80_szp[d8]);_S_A(d8);}break;
      case 0xef:/*RST 0x28*/_T(1);d16= _G_SP();_MW(--d16, pc>>8);_MW(--d16, pc);_S_SP(d16);pc=0x28;_S_WZ(pc);break;
      case 0xf0:/*RET P*/_T(1);if (!(_G_F()&Z80_SF)){uint8_t w,z;d16=_G_SP();_MR(d16++,z);_MR(d16++,w);_S_SP(d16);pc=(w<<8)|z;_S_WZ(pc);}break;
      case 0xf1:/*POP FA*/addr=_G_SP();_MR(addr++,d8);d16=d8<<8;_MR(addr++,d8);d16|=d8;_S_FA(d16);_S_SP(addr);break;
      case 0xf2:/*JP P,nn*/_IMM16(addr);if(!(_G_F()&Z80_SF)){pc=addr;}break;
      case 0xf3:/*DI*/r2&=~(_BIT_IFF1|_BIT_IFF2);break;
      case 0xf4:/*CALL P,nn*/_IMM16(addr);if(!(_G_F()&Z80_SF)){_T(1);uint16_t sp=_G_SP();_MW(--sp,pc>>8);_MW(--sp,pc);_S_SP(sp);pc=addr;}break;
      case 0xf5:/*PUSH FA*/_T(1);addr=_G_SP();d16=_G_FA();_MW(--addr,d16);_MW(--addr,d16>>8);_S_SP(addr);break;
      case 0xf6:/*OR n*/_IMM8(d8);{d8|=_G_A();_S_F(_z80_szp[d8]);_S_A(d8);}break;
      case 0xf7:/*RST 0x30*/_T(1);d16= _G_SP();_MW(--d16, pc>>8);_MW(--d16, pc);_S_SP(d16);pc=0x30;_S_WZ(pc);break;
      case 0xf8:/*RET M*/_T(1);if ((_G_F()&Z80_SF)){uint8_t w,z;d16=_G_SP();_MR(d16++,z);_MR(d16++,w);_S_SP(d16);pc=(w<<8)|z;_S_WZ(pc);}break;
      case 0xf9:/*LD SP,HL*/_T(2);_S_SP(_G_HL());break;
      case 0xfa:/*JP M,nn*/_IMM16(addr);if((_G_F()&Z80_SF)){pc=addr;}break;
      case 0xfb:/*EI*/r2=(r2&~(_BIT_IFF1|_BIT_IFF2))|_BIT_EI;break;
      case 0xfc:/*CALL M,nn*/_IMM16(addr);if((_G_F()&Z80_SF)){_T(1);uint16_t sp=_G_SP();_MW(--sp,pc>>8);_MW(--sp,pc);_S_SP(sp);pc=addr;}break;
      case 0xfd:/*FD prefix*/map_bits|=_BIT_USE_IY;continue;break;
      case 0xfe:/*CP n*/_IMM8(d8);{uint8_t acc=_G_A();int32_t res=(uint32_t)((int)acc-(int)d8);_S_F(_CP_FLAGS(acc,d8,res));}break;
      case 0xff:/*RST 0x38*/_T(1);d16= _G_SP();_MW(--d16, pc>>8);_MW(--d16, pc);_S_SP(d16);pc=0x38;_S_WZ(pc);break;
      default: break;
    }
    bool nmi = 0 != ((pins & (pre_pins ^ pins)) & Z80_NMI);
    if (nmi || (((pins & (Z80_INT|Z80_BUSREQ))==Z80_INT) && (r2 & _BIT_IFF1))) {
      r2 &= ~_BIT_IFF1;
      if (pins & Z80_INT) {
        r2 &= ~_BIT_IFF2;
      }
      if (pins & Z80_HALT) {
        pins &= ~Z80_HALT;
        pc++;
      }
      _SA(pc);
      if (nmi) {
        _TWM(5,Z80_M1|Z80_MREQ|Z80_RD);_BUMPR();
        uint16_t sp = _G_SP();
        _MW(--sp,pc>>8);
        _MW(--sp,pc);
        _S_SP(sp);
        pc = 0x0066;
        _S_WZ(pc);
      }
      else {
        _TWM(4,Z80_M1|Z80_IORQ);
        const uint8_t int_vec = _GD();
        _BUMPR();
        _T(2);
        switch (_G_IM()) {
          case 0:
            break;
          case 1:
            {
              uint16_t sp = _G_SP();
              _MW(--sp,pc>>8);
              _MW(--sp,pc);
              _S_SP(sp);
              pc = 0x0038;
              _S_WZ(pc);
            }
            break;
          case 2:
            {
              uint16_t sp = _G_SP();
              _MW(--sp,pc>>8);
              _MW(--sp,pc);
              _S_SP(sp);
              addr = (_G_I()<<8) | (int_vec & 0xFE);
              uint8_t z,w;
              _MR(addr++,z);
              _MR(addr,w);
              pc = (w<<8)|z;
              _S_WZ(pc);
            }
            break;
        }
       }
    }
    map_bits &= ~(_BIT_USE_IX|_BIT_USE_IY);
    pins&=~Z80_INT;
    /* delay-enable interrupt flags */
    if (r2 & _BIT_EI) {
      r2 &= ~_BIT_EI;
      r2 |= (_BIT_IFF1 | _BIT_IFF2);
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
  _S_PC(pc);
  r0 = _z80_flush_r0(ws, r0, r2);
  r1 = _z80_flush_r1(ws, r1, r2);
  r2 = (r2 & ~_BITS_MAP_REGS) | map_bits;
  cpu->bc_de_hl_fa = r0;
  cpu->wz_ix_iy_sp = r1;
  cpu->im_ir_pc_bits = r2;
  cpu->bc_de_hl_fa_ = r3;
  cpu->pins = pins;
  cpu->trap_id = trap_id;
  return ticks;
}
