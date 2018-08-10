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
  do {
    _OFF(Z80_INT);
    /* delay-enable interrupt flags */
    if (r2 & _BIT_EI) {
      r2 &= ~_BIT_EI;
      r2 |= (_BIT_IFF1 | _BIT_IFF2);
    }
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
      case 0x6:/*LD B,n*/_IMM8(d8);_S_B(d8);break;
      case 0xa:/*LD A,(BC)*/addr=_G_BC();_MR(addr++,d8);_S_A(d8);_S_WZ(addr);break;
      case 0xb:/*DEC BC*/_T(2);_S_BC(_G_BC()-1);break;
      case 0xe:/*LD C,n*/_IMM8(d8);_S_C(d8);break;
      case 0x11:/*LD DE,nn*/_IMM16(d16);_S_DE(d16);break;
      case 0x12:/*LD (DE),A*/addr=_G_DE();d8=_G_A();_MW(addr++,d8);_S_WZ((d8<<8)|(addr&0x00FF));break;
      case 0x13:/*INC DE*/_T(2);_S_DE(_G_DE()+1);break;
      case 0x16:/*LD D,n*/_IMM8(d8);_S_D(d8);break;
      case 0x1a:/*LD A,(DE)*/addr=_G_DE();_MR(addr++,d8);_S_A(d8);_S_WZ(addr);break;
      case 0x1b:/*DEC DE*/_T(2);_S_DE(_G_DE()-1);break;
      case 0x1e:/*LD E,n*/_IMM8(d8);_S_E(d8);break;
      case 0x21:/*LD HL,nn*/_IMM16(d16);_S_HL(d16);break;
      case 0x22:/*LD (nn),HL*/_IMM16(addr);_MW(addr++,_G_L());_MW(addr,_G_H());_S_WZ(addr);break;
      case 0x23:/*INC HL*/_T(2);_S_HL(_G_HL()+1);break;
      case 0x26:/*LD H,n*/_IMM8(d8);_S_H(d8);break;
      case 0x2a:/*LD HL,(nn)*/_IMM16(addr);_MR(addr++,d8);_S_L(d8);_MR(addr,d8);_S_H(d8);_S_WZ(addr);break;
      case 0x2b:/*DEC HL*/_T(2);_S_HL(_G_HL()-1);break;
      case 0x2e:/*LD L,n*/_IMM8(d8);_S_L(d8);break;
      case 0x31:/*LD SP,nn*/_IMM16(d16);_S_SP(d16);break;
      case 0x32:/*LD (nn),A*/_IMM16(addr);d8=_G_A();_MW(addr++,d8);_S_WZ((d8<<8)|(addr&0x00FF));break;
      case 0x33:/*INC SP*/_T(2);_S_SP(_G_SP()+1);break;
      case 0x36:/*LD (HL/IX+d/IY+d),n*/_ADDR(addr,2);_IMM8(d8);_MW(addr,d8);break;
      case 0x3a:/*LD A,(nn)*/_IMM16(addr);_MR(addr++,d8);_S_A(d8);_S_WZ(addr);break;
      case 0x3b:/*DEC SP*/_T(2);_S_SP(_G_SP()-1);break;
      case 0x3e:/*LD A,n*/_IMM8(d8);_S_A(d8);break;
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
      case 0x76:/*HALT*/_ON(Z80_HALT);pc--;break;
      case 0x77:/*LD (HL/IX+d/IY+d),A*/d8=_G_A();_ADDR(addr,5);_MW(addr,d8);break;
      case 0x78:/*LD A,B*/_S_A(_G_B());break;
      case 0x79:/*LD A,C*/_S_A(_G_C());break;
      case 0x7a:/*LD A,D*/_S_A(_G_D());break;
      case 0x7b:/*LD A,E*/_S_A(_G_E());break;
      case 0x7c:/*LD A,H*/_S_A(_G_H());break;
      case 0x7d:/*LD A,L*/_S_A(_G_L());break;
      case 0x7e:/*LD A,(HL/IX+d/IY+d)*/_ADDR(addr,5);_MR(addr,d8);_S_A(d8);break;
      case 0x7f:/*LD A,A*/_S_A(_G_A());break;
      case 0xc1:/*POP BC*/addr=_G_SP();_MR(addr++,d8);d16=d8;_MR(addr++,d8);d16|=d8<<8;_S_BC(d16);_S_SP(addr);break;
      case 0xc5:/*PUSH BC*/_T(1);addr=_G_SP();d16=_G_BC();_MW(--addr,d16>>8);_MW(--addr,d16);_S_SP(addr);break;
      case 0xc9:/*RET*/d16=_G_SP();_MR(d16++,d8);pc=d8<<8;_MR(d16++,d8);pc|=d8;_S_SP(d16);_S_WZ(pc);break;
      case 0xcd:/*CALL nn*/_IMM16(addr);_T(1);d16=_G_SP();_MW(--d16,pc>>8);_MW(--d16,pc);_S_SP(d16);pc=addr;break;
      case 0xd1:/*POP DE*/addr=_G_SP();_MR(addr++,d8);d16=d8;_MR(addr++,d8);d16|=d8<<8;_S_DE(d16);_S_SP(addr);break;
      case 0xd5:/*PUSH DE*/_T(1);addr=_G_SP();d16=_G_DE();_MW(--addr,d16>>8);_MW(--addr,d16);_S_SP(addr);break;
      case 0xd9:/*EXX*/{r0=_z80_flush_r0(ws,r0,r2);const uint64_t rx=r3;r3=(r3&0xffff)|(r0&0xffffffffffff0000);r0=(r0&0xffff)|(rx&0xffffffffffff0000);ws=_z80_map_regs(r0, r1, r2);}break;
      case 0xdd:/*DD prefix*/map_bits|=_BIT_USE_IX;continue;break;
      case 0xe1:/*POP HL*/addr=_G_SP();_MR(addr++,d8);d16=d8;_MR(addr++,d8);d16|=d8<<8;_S_HL(d16);_S_SP(addr);break;
      case 0xe5:/*PUSH HL*/_T(1);addr=_G_SP();d16=_G_HL();_MW(--addr,d16>>8);_MW(--addr,d16);_S_SP(addr);break;
      case 0xe9:/*JP HL*/pc=_G_HL();break;
      case 0xED: {
        _FETCH(op);
        switch(op) {
          case 0x43:/*LD (nn),BC*/_IMM16(addr);d16=_G_BC();_MW(addr++,d16&0xFF);_MW(addr,d16>>8);_S_WZ(addr);break;
          case 0x4b:/*LD BC,(nn)*/_IMM16(addr);_MR(addr++,d8);d16=d8;_MR(addr,d8);d16|=d8<<8;_S_BC(d16);_S_WZ(addr);break;
          case 0x53:/*LD (nn),DE*/_IMM16(addr);d16=_G_DE();_MW(addr++,d16&0xFF);_MW(addr,d16>>8);_S_WZ(addr);break;
          case 0x5b:/*LD DE,(nn)*/_IMM16(addr);_MR(addr++,d8);d16=d8;_MR(addr,d8);d16|=d8<<8;_S_DE(d16);_S_WZ(addr);break;
          case 0x63:/*LD (nn),HL*/_IMM16(addr);d16=_G_HL();_MW(addr++,d16&0xFF);_MW(addr,d16>>8);_S_WZ(addr);break;
          case 0x6b:/*LD HL,(nn)*/_IMM16(addr);_MR(addr++,d8);d16=d8;_MR(addr,d8);d16|=d8<<8;_S_HL(d16);_S_WZ(addr);break;
          case 0x73:/*LD (nn),SP*/_IMM16(addr);d16=_G_SP();_MW(addr++,d16&0xFF);_MW(addr,d16>>8);_S_WZ(addr);break;
          case 0x7b:/*LD SP,(nn)*/_IMM16(addr);_MR(addr++,d8);d16=d8;_MR(addr,d8);d16|=d8<<8;_S_SP(d16);_S_WZ(addr);break;
          default: break;
        }
      }
      break;
      case 0xf1:/*POP FA*/addr=_G_SP();_MR(addr++,d8);d16=d8<<8;_MR(addr++,d8);d16|=d8;_S_FA(d16);_S_SP(addr);break;
      case 0xf5:/*PUSH FA*/_T(1);addr=_G_SP();d16=_G_FA();_MW(--addr,d16);_MW(--addr,d16>>8);_S_SP(addr);break;
      case 0xf9:/*LD SP,HL*/_T(2);_S_SP(_G_HL());break;
      case 0xfd:/*FD prefix*/map_bits|=_BIT_USE_IY;continue;break;
      default: break;
    }
    if (((pins & (Z80_INT|Z80_BUSREQ))==Z80_INT) && (r2 & _BIT_IFF1)) {
      r2 &= ~(_BIT_IFF1|_BIT_IFF2);
      if (pins & Z80_HALT) {
        pins &= ~Z80_HALT;
        pc++;
      }
      _ON(Z80_M1|Z80_IORQ);
      _SA(pc);
      _TW(4);
      const uint8_t int_vec = _GD();
      _OFF(Z80_M1|Z80_IORQ);
      _BUMPR();
      _T(2);
      uint16_t sp = _G_SP();
      _MW(--sp,pc>>8);
      _MW(--sp,pc);
      _S_SP(sp);
      switch (_G_IM()) {
        case 0:
          break;
        case 1:
          pc = 0x0038;
          break;
        case 2:
          {
            addr = _G_I() | (int_vec & 0xFE);
            uint8_t z,w;
            _MR(addr++,z);
            _MR(addr,w);
            pc = (w<<8)|z;
          }
          break;
      }
      _S_WZ(pc);
    }
    map_bits &= ~(_BIT_USE_IX|_BIT_USE_IY);
    if (trap_addr != 0xFFFFFFFFFFFFFFFF) {
      uint64_t ta = trap_addr;
      for (int i = 0; i < Z80_MAX_NUM_TRAPS; i++) {
        ta >>= 16;
        if (((ta & 0xFFFF) == pc) && (pc != 0xFFFF)) {
          trap_id = i;
          break;
        }
      }
    }
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
