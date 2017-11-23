// machine generated, do not edit!
static void _z80_op(z80* c) {
  void(*tick)(z80*) = c->tick;
  uint32_t ticks=c->ticks;
  uint8_t opcode;
  _FETCH(); switch (opcode) {
    case 0x0:   break; // NOP
    case 0x1: _IMM16(); c->BC=c->WZ; break; // LD BC,nn
    case 0x2: c->WZ=c->BC;_WR(c->WZ++,c->A);c->W=c->A; break; // LD (BC),A
    case 0x3: _T();_T();c->BC++; break; // INC BC
    case 0x4: c->B=_z80_inc(c,c->B); break; // INC B
    case 0x5: c->B=_z80_dec(c,c->B); break; // DEC B
    case 0x6: _RD(c->PC++,c->B); break; // LD B,n
    case 0x7: _z80_rlca(c); break; // RLCA
    case 0x8: _SWP16(c->AF,c->AF_); break; // EX AF,AF'
    case 0x9: c->HL=_z80_add16(c,c->HL,c->BC);_T();_T();_T();_T();_T();_T();_T(); break; // ADD HL,BC
    case 0xa: c->WZ=c->BC;_RD(c->WZ++,c->A); break; // LD A,(BC)
    case 0xb: _T();_T();c->BC--; break; // DEC BC
    case 0xc: c->C=_z80_inc(c,c->C); break; // INC C
    case 0xd: c->C=_z80_dec(c,c->C); break; // DEC C
    case 0xe: _RD(c->PC++,c->C); break; // LD C,n
    case 0xf: _z80_rrca(c); break; // RRCA
    case 0x10: ticks=_z80_djnz(c,tick,ticks); break; // DJNZ
    case 0x11: _IMM16(); c->DE=c->WZ; break; // LD DE,nn
    case 0x12: c->WZ=c->DE;_WR(c->WZ++,c->A);c->W=c->A; break; // LD (DE),A
    case 0x13: _T();_T();c->DE++; break; // INC DE
    case 0x14: c->D=_z80_inc(c,c->D); break; // INC D
    case 0x15: c->D=_z80_dec(c,c->D); break; // DEC D
    case 0x16: _RD(c->PC++,c->D); break; // LD D,n
    case 0x17: _z80_rla(c); break; // RLA
    case 0x18: ticks=_z80_jr(c,tick,ticks); break; // JR d
    case 0x19: c->HL=_z80_add16(c,c->HL,c->DE);_T();_T();_T();_T();_T();_T();_T(); break; // ADD HL,DE
    case 0x1a: c->WZ=c->DE;_RD(c->WZ++,c->A); break; // LD A,(DE)
    case 0x1b: _T();_T();c->DE--; break; // DEC DE
    case 0x1c: c->E=_z80_inc(c,c->E); break; // INC E
    case 0x1d: c->E=_z80_dec(c,c->E); break; // DEC E
    case 0x1e: _RD(c->PC++,c->E); break; // LD E,n
    case 0x1f: _z80_rra(c); break; // RRA
    case 0x20: ticks=_z80_jr_cc(c,!(c->F&Z80_ZF),tick,ticks); break; // JR NZ,d
    case 0x21: _IMM16(); c->HL=c->WZ; break; // LD HL,nn
    case 0x22: {_IMM16();_WR(c->WZ++,(uint8_t)c->HL);_WR(c->WZ,(uint8_t)(c->HL>>8));} break; // LD (nn),HL
    case 0x23: _T();_T();c->HL++; break; // INC HL
    case 0x24: c->H=_z80_inc(c,c->H); break; // INC H
    case 0x25: c->H=_z80_dec(c,c->H); break; // DEC H
    case 0x26: _RD(c->PC++,c->H); break; // LD H,n
    case 0x27: _z80_daa(c); break; // DAA
    case 0x28: ticks=_z80_jr_cc(c,(c->F&Z80_ZF),tick,ticks); break; // JR Z,d
    case 0x29: c->HL=_z80_add16(c,c->HL,c->HL);_T();_T();_T();_T();_T();_T();_T(); break; // ADD HL,HL
    case 0x2a: {_IMM16();uint8_t l;_RD(c->WZ++,l);uint8_t h;_RD(c->WZ,h);c->HL=(h<<8)|l;} break; // LD HL,(nn)
    case 0x2b: _T();_T();c->HL--; break; // DEC HL
    case 0x2c: c->L=_z80_inc(c,c->L); break; // INC L
    case 0x2d: c->L=_z80_dec(c,c->L); break; // DEC L
    case 0x2e: _RD(c->PC++,c->L); break; // LD L,n
    case 0x2f: _z80_cpl(c); break; // CPL
    case 0x30: ticks=_z80_jr_cc(c,!(c->F&Z80_CF),tick,ticks); break; // JR NC,d
    case 0x31: _IMM16(); c->SP=c->WZ; break; // LD SP,nn
    case 0x32: _IMM16();_WR(c->WZ++,c->A);c->W=c->A; break; // LD (nn),A
    case 0x33: _T();_T();c->SP++; break; // INC SP
    case 0x34: { uint16_t a=c->HL;_T();uint8_t v;_RD(a,v);_WR(a,_z80_inc(c,v)); } break; // INC (c->HL)
    case 0x35: { uint16_t a=c->HL;_T();uint8_t v;_RD(a,v);_WR(a,_z80_dec(c,v)); } break; // DEC (c->HL)
    case 0x36: { uint16_t a=c->HL;uint8_t v;_RD(c->PC++,v);_WR(a,v); } break; // LD (c->HL),n
    case 0x37: _z80_scf(c); break; // SCF
    case 0x38: ticks=_z80_jr_cc(c,(c->F&Z80_CF),tick,ticks); break; // JR C,d
    case 0x39: c->HL=_z80_add16(c,c->HL,c->SP);_T();_T();_T();_T();_T();_T();_T(); break; // ADD HL,SP
    case 0x3a: _IMM16();_RD(c->WZ++,c->A); break; // LD A,(nn)
    case 0x3b: _T();_T();c->SP--; break; // DEC SP
    case 0x3c: c->A=_z80_inc(c,c->A); break; // INC A
    case 0x3d: c->A=_z80_dec(c,c->A); break; // DEC A
    case 0x3e: _RD(c->PC++,c->A); break; // LD A,n
    case 0x3f: _z80_ccf(c); break; // CCF
    case 0x40: c->B=c->B; break; // LD B,B
    case 0x41: c->B=c->C; break; // LD B,C
    case 0x42: c->B=c->D; break; // LD B,D
    case 0x43: c->B=c->E; break; // LD B,E
    case 0x44: c->B=c->H; break; // LD B,H
    case 0x45: c->B=c->L; break; // LD B,L
    case 0x46: { uint16_t a=c->HL;_RD(a,c->B); }; break; // LD B,(c->HL)
    case 0x47: c->B=c->A; break; // LD B,A
    case 0x48: c->C=c->B; break; // LD C,B
    case 0x49: c->C=c->C; break; // LD C,C
    case 0x4a: c->C=c->D; break; // LD C,D
    case 0x4b: c->C=c->E; break; // LD C,E
    case 0x4c: c->C=c->H; break; // LD C,H
    case 0x4d: c->C=c->L; break; // LD C,L
    case 0x4e: { uint16_t a=c->HL;_RD(a,c->C); }; break; // LD C,(c->HL)
    case 0x4f: c->C=c->A; break; // LD C,A
    case 0x50: c->D=c->B; break; // LD D,B
    case 0x51: c->D=c->C; break; // LD D,C
    case 0x52: c->D=c->D; break; // LD D,D
    case 0x53: c->D=c->E; break; // LD D,E
    case 0x54: c->D=c->H; break; // LD D,H
    case 0x55: c->D=c->L; break; // LD D,L
    case 0x56: { uint16_t a=c->HL;_RD(a,c->D); }; break; // LD D,(c->HL)
    case 0x57: c->D=c->A; break; // LD D,A
    case 0x58: c->E=c->B; break; // LD E,B
    case 0x59: c->E=c->C; break; // LD E,C
    case 0x5a: c->E=c->D; break; // LD E,D
    case 0x5b: c->E=c->E; break; // LD E,E
    case 0x5c: c->E=c->H; break; // LD E,H
    case 0x5d: c->E=c->L; break; // LD E,L
    case 0x5e: { uint16_t a=c->HL;_RD(a,c->E); }; break; // LD E,(c->HL)
    case 0x5f: c->E=c->A; break; // LD E,A
    case 0x60: c->H=c->B; break; // LD H,B
    case 0x61: c->H=c->C; break; // LD H,C
    case 0x62: c->H=c->D; break; // LD H,D
    case 0x63: c->H=c->E; break; // LD H,E
    case 0x64: c->H=c->H; break; // LD H,H
    case 0x65: c->H=c->L; break; // LD H,L
    case 0x66: { uint16_t a=c->HL;_RD(a,c->H); }; break; // LD H,(c->HL)
    case 0x67: c->H=c->A; break; // LD H,A
    case 0x68: c->L=c->B; break; // LD L,B
    case 0x69: c->L=c->C; break; // LD L,C
    case 0x6a: c->L=c->D; break; // LD L,D
    case 0x6b: c->L=c->E; break; // LD L,E
    case 0x6c: c->L=c->H; break; // LD L,H
    case 0x6d: c->L=c->L; break; // LD L,L
    case 0x6e: { uint16_t a=c->HL;_RD(a,c->L); }; break; // LD L,(c->HL)
    case 0x6f: c->L=c->A; break; // LD L,A
    case 0x70: { uint16_t a=c->HL;_WR(a,c->B); }; break; // LD (c->HL),B
    case 0x71: { uint16_t a=c->HL;_WR(a,c->C); }; break; // LD (c->HL),C
    case 0x72: { uint16_t a=c->HL;_WR(a,c->D); }; break; // LD (c->HL),D
    case 0x73: { uint16_t a=c->HL;_WR(a,c->E); }; break; // LD (c->HL),E
    case 0x74: { uint16_t a=c->HL;_WR(a,c->H); }; break; // LD (c->HL),H
    case 0x75: { uint16_t a=c->HL;_WR(a,c->L); }; break; // LD (c->HL),L
    case 0x76: _z80_halt(c); break; // HALT
    case 0x77: { uint16_t a=c->HL;_WR(a,c->A); }; break; // LD (c->HL),A
    case 0x78: c->A=c->B; break; // LD A,B
    case 0x79: c->A=c->C; break; // LD A,C
    case 0x7a: c->A=c->D; break; // LD A,D
    case 0x7b: c->A=c->E; break; // LD A,E
    case 0x7c: c->A=c->H; break; // LD A,H
    case 0x7d: c->A=c->L; break; // LD A,L
    case 0x7e: { uint16_t a=c->HL;_RD(a,c->A); }; break; // LD A,(c->HL)
    case 0x7f: c->A=c->A; break; // LD A,A
    case 0x80: _z80_add(c,c->B); break; // ADD B
    case 0x81: _z80_add(c,c->C); break; // ADD C
    case 0x82: _z80_add(c,c->D); break; // ADD D
    case 0x83: _z80_add(c,c->E); break; // ADD E
    case 0x84: _z80_add(c,c->H); break; // ADD H
    case 0x85: _z80_add(c,c->L); break; // ADD L
    case 0x86: { uint16_t a=c->HL;uint8_t val;_RD(a,val);_z80_add(c,val); }; break; // ADD (c->HL)
    case 0x87: _z80_add(c,c->A); break; // ADD A
    case 0x88: _z80_adc(c,c->B); break; // ADC B
    case 0x89: _z80_adc(c,c->C); break; // ADC C
    case 0x8a: _z80_adc(c,c->D); break; // ADC D
    case 0x8b: _z80_adc(c,c->E); break; // ADC E
    case 0x8c: _z80_adc(c,c->H); break; // ADC H
    case 0x8d: _z80_adc(c,c->L); break; // ADC L
    case 0x8e: { uint16_t a=c->HL;uint8_t val;_RD(a,val);_z80_adc(c,val); }; break; // ADC (c->HL)
    case 0x8f: _z80_adc(c,c->A); break; // ADC A
    case 0x90: _z80_sub(c,c->B); break; // SUB B
    case 0x91: _z80_sub(c,c->C); break; // SUB C
    case 0x92: _z80_sub(c,c->D); break; // SUB D
    case 0x93: _z80_sub(c,c->E); break; // SUB E
    case 0x94: _z80_sub(c,c->H); break; // SUB H
    case 0x95: _z80_sub(c,c->L); break; // SUB L
    case 0x96: { uint16_t a=c->HL;uint8_t val;_RD(a,val);_z80_sub(c,val); }; break; // SUB (c->HL)
    case 0x97: _z80_sub(c,c->A); break; // SUB A
    case 0x98: _z80_sbc(c,c->B); break; // SBC B
    case 0x99: _z80_sbc(c,c->C); break; // SBC C
    case 0x9a: _z80_sbc(c,c->D); break; // SBC D
    case 0x9b: _z80_sbc(c,c->E); break; // SBC E
    case 0x9c: _z80_sbc(c,c->H); break; // SBC H
    case 0x9d: _z80_sbc(c,c->L); break; // SBC L
    case 0x9e: { uint16_t a=c->HL;uint8_t val;_RD(a,val);_z80_sbc(c,val); }; break; // SBC (c->HL)
    case 0x9f: _z80_sbc(c,c->A); break; // SBC A
    case 0xa0: _z80_and(c,c->B); break; // AND B
    case 0xa1: _z80_and(c,c->C); break; // AND C
    case 0xa2: _z80_and(c,c->D); break; // AND D
    case 0xa3: _z80_and(c,c->E); break; // AND E
    case 0xa4: _z80_and(c,c->H); break; // AND H
    case 0xa5: _z80_and(c,c->L); break; // AND L
    case 0xa6: { uint16_t a=c->HL;uint8_t val;_RD(a,val);_z80_and(c,val); }; break; // AND (c->HL)
    case 0xa7: _z80_and(c,c->A); break; // AND A
    case 0xa8: _z80_xor(c,c->B); break; // XOR B
    case 0xa9: _z80_xor(c,c->C); break; // XOR C
    case 0xaa: _z80_xor(c,c->D); break; // XOR D
    case 0xab: _z80_xor(c,c->E); break; // XOR E
    case 0xac: _z80_xor(c,c->H); break; // XOR H
    case 0xad: _z80_xor(c,c->L); break; // XOR L
    case 0xae: { uint16_t a=c->HL;uint8_t val;_RD(a,val);_z80_xor(c,val); }; break; // XOR (c->HL)
    case 0xaf: _z80_xor(c,c->A); break; // XOR A
    case 0xb0: _z80_or(c,c->B); break; // OR B
    case 0xb1: _z80_or(c,c->C); break; // OR C
    case 0xb2: _z80_or(c,c->D); break; // OR D
    case 0xb3: _z80_or(c,c->E); break; // OR E
    case 0xb4: _z80_or(c,c->H); break; // OR H
    case 0xb5: _z80_or(c,c->L); break; // OR L
    case 0xb6: { uint16_t a=c->HL;uint8_t val;_RD(a,val);_z80_or(c,val); }; break; // OR (c->HL)
    case 0xb7: _z80_or(c,c->A); break; // OR A
    case 0xb8: _z80_cp(c,c->B); break; // CP B
    case 0xb9: _z80_cp(c,c->C); break; // CP C
    case 0xba: _z80_cp(c,c->D); break; // CP D
    case 0xbb: _z80_cp(c,c->E); break; // CP E
    case 0xbc: _z80_cp(c,c->H); break; // CP H
    case 0xbd: _z80_cp(c,c->L); break; // CP L
    case 0xbe: { uint16_t a=c->HL;uint8_t val;_RD(a,val);_z80_cp(c,val); }; break; // CP (c->HL)
    case 0xbf: _z80_cp(c,c->A); break; // CP A
    case 0xc0: ticks=_z80_retcc(c,!(c->F&Z80_ZF),tick,ticks); break; // RET NZ
    case 0xc1: { uint8_t l;_RD(c->SP++,l);uint8_t h;_RD(c->SP++,h);c->BC=(h<<8)|l; } break; // POP BC
    case 0xc2: _IMM16(); if (!(c->F&Z80_ZF)) { c->PC=c->WZ; } break; // JP NZ,nn
    case 0xc3: _IMM16(); c->PC=c->WZ; break; // JP nn
    case 0xc4: ticks=_z80_callcc(c,!(c->F&Z80_ZF),tick,ticks); break; // CALL NZ,nn
    case 0xc5: _T();_WR(--c->SP,(uint8_t)(c->BC>>8)); _WR(--c->SP,(uint8_t)c->BC); break; // PUSH BC
    case 0xc6: {uint8_t v;_RD(c->PC++,v);_z80_add(c,v);} break; // ADD n
    case 0xc7: ticks=_z80_rst(c,0x0,tick,ticks); break; // RST 0x0
    case 0xc8: ticks=_z80_retcc(c,(c->F&Z80_ZF),tick,ticks); break; // RET Z
    case 0xc9: ticks=_z80_ret(c,tick,ticks); break; // RET
    case 0xca: _IMM16(); if ((c->F&Z80_ZF)) { c->PC=c->WZ; } break; // JP Z,nn
    case 0xcb:
      _FETCH(); switch (opcode) {
        case 0x0: c->B=_z80_rlc(c,c->B); break; // RLC B
        case 0x1: c->C=_z80_rlc(c,c->C); break; // RLC C
        case 0x2: c->D=_z80_rlc(c,c->D); break; // RLC D
        case 0x3: c->E=_z80_rlc(c,c->E); break; // RLC E
        case 0x4: c->H=_z80_rlc(c,c->H); break; // RLC H
        case 0x5: c->L=_z80_rlc(c,c->L); break; // RLC L
        case 0x6: { uint16_t a=c->HL;_T();uint8_t v;_RD(a,v);_WR(a,_z80_rlc(c,v)); } break; // RLC (c->HL)
        case 0x7: c->A=_z80_rlc(c,c->A); break; // RLC A
        case 0x8: c->B=_z80_rrc(c,c->B); break; // RRC B
        case 0x9: c->C=_z80_rrc(c,c->C); break; // RRC C
        case 0xa: c->D=_z80_rrc(c,c->D); break; // RRC D
        case 0xb: c->E=_z80_rrc(c,c->E); break; // RRC E
        case 0xc: c->H=_z80_rrc(c,c->H); break; // RRC H
        case 0xd: c->L=_z80_rrc(c,c->L); break; // RRC L
        case 0xe: { uint16_t a=c->HL;_T();uint8_t v;_RD(a,v);_WR(a,_z80_rrc(c,v)); } break; // RRC (c->HL)
        case 0xf: c->A=_z80_rrc(c,c->A); break; // RRC A
        case 0x10: c->B=_z80_rl(c,c->B); break; // RL B
        case 0x11: c->C=_z80_rl(c,c->C); break; // RL C
        case 0x12: c->D=_z80_rl(c,c->D); break; // RL D
        case 0x13: c->E=_z80_rl(c,c->E); break; // RL E
        case 0x14: c->H=_z80_rl(c,c->H); break; // RL H
        case 0x15: c->L=_z80_rl(c,c->L); break; // RL L
        case 0x16: { uint16_t a=c->HL;_T();uint8_t v;_RD(a,v);_WR(a,_z80_rl(c,v)); } break; // RL (c->HL)
        case 0x17: c->A=_z80_rl(c,c->A); break; // RL A
        case 0x18: c->B=_z80_rr(c,c->B); break; // RR B
        case 0x19: c->C=_z80_rr(c,c->C); break; // RR C
        case 0x1a: c->D=_z80_rr(c,c->D); break; // RR D
        case 0x1b: c->E=_z80_rr(c,c->E); break; // RR E
        case 0x1c: c->H=_z80_rr(c,c->H); break; // RR H
        case 0x1d: c->L=_z80_rr(c,c->L); break; // RR L
        case 0x1e: { uint16_t a=c->HL;_T();uint8_t v;_RD(a,v);_WR(a,_z80_rr(c,v)); } break; // RR (c->HL)
        case 0x1f: c->A=_z80_rr(c,c->A); break; // RR A
        case 0x20: c->B=_z80_sla(c,c->B); break; // SLA B
        case 0x21: c->C=_z80_sla(c,c->C); break; // SLA C
        case 0x22: c->D=_z80_sla(c,c->D); break; // SLA D
        case 0x23: c->E=_z80_sla(c,c->E); break; // SLA E
        case 0x24: c->H=_z80_sla(c,c->H); break; // SLA H
        case 0x25: c->L=_z80_sla(c,c->L); break; // SLA L
        case 0x26: { uint16_t a=c->HL;_T();uint8_t v;_RD(a,v);_WR(a,_z80_sla(c,v)); } break; // SLA (c->HL)
        case 0x27: c->A=_z80_sla(c,c->A); break; // SLA A
        case 0x28: c->B=_z80_sra(c,c->B); break; // SRA B
        case 0x29: c->C=_z80_sra(c,c->C); break; // SRA C
        case 0x2a: c->D=_z80_sra(c,c->D); break; // SRA D
        case 0x2b: c->E=_z80_sra(c,c->E); break; // SRA E
        case 0x2c: c->H=_z80_sra(c,c->H); break; // SRA H
        case 0x2d: c->L=_z80_sra(c,c->L); break; // SRA L
        case 0x2e: { uint16_t a=c->HL;_T();uint8_t v;_RD(a,v);_WR(a,_z80_sra(c,v)); } break; // SRA (c->HL)
        case 0x2f: c->A=_z80_sra(c,c->A); break; // SRA A
        case 0x30: c->B=_z80_sll(c,c->B); break; // SLL B
        case 0x31: c->C=_z80_sll(c,c->C); break; // SLL C
        case 0x32: c->D=_z80_sll(c,c->D); break; // SLL D
        case 0x33: c->E=_z80_sll(c,c->E); break; // SLL E
        case 0x34: c->H=_z80_sll(c,c->H); break; // SLL H
        case 0x35: c->L=_z80_sll(c,c->L); break; // SLL L
        case 0x36: { uint16_t a=c->HL;_T();uint8_t v;_RD(a,v);_WR(a,_z80_sll(c,v)); } break; // SLL (c->HL)
        case 0x37: c->A=_z80_sll(c,c->A); break; // SLL A
        case 0x38: c->B=_z80_srl(c,c->B); break; // SRL B
        case 0x39: c->C=_z80_srl(c,c->C); break; // SRL C
        case 0x3a: c->D=_z80_srl(c,c->D); break; // SRL D
        case 0x3b: c->E=_z80_srl(c,c->E); break; // SRL E
        case 0x3c: c->H=_z80_srl(c,c->H); break; // SRL H
        case 0x3d: c->L=_z80_srl(c,c->L); break; // SRL L
        case 0x3e: { uint16_t a=c->HL;_T();uint8_t v;_RD(a,v);_WR(a,_z80_srl(c,v)); } break; // SRL (c->HL)
        case 0x3f: c->A=_z80_srl(c,c->A); break; // SRL A
        case 0x40: _z80_bit(c,c->B,0x1); break; // BIT 0,B
        case 0x41: _z80_bit(c,c->C,0x1); break; // BIT 0,C
        case 0x42: _z80_bit(c,c->D,0x1); break; // BIT 0,D
        case 0x43: _z80_bit(c,c->E,0x1); break; // BIT 0,E
        case 0x44: _z80_bit(c,c->H,0x1); break; // BIT 0,H
        case 0x45: _z80_bit(c,c->L,0x1); break; // BIT 0,L
        case 0x46: { uint16_t a=c->HL;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x1); } break; // BIT 0,(c->HL)
        case 0x47: _z80_bit(c,c->A,0x1); break; // BIT 0,A
        case 0x48: _z80_bit(c,c->B,0x2); break; // BIT 1,B
        case 0x49: _z80_bit(c,c->C,0x2); break; // BIT 1,C
        case 0x4a: _z80_bit(c,c->D,0x2); break; // BIT 1,D
        case 0x4b: _z80_bit(c,c->E,0x2); break; // BIT 1,E
        case 0x4c: _z80_bit(c,c->H,0x2); break; // BIT 1,H
        case 0x4d: _z80_bit(c,c->L,0x2); break; // BIT 1,L
        case 0x4e: { uint16_t a=c->HL;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x2); } break; // BIT 1,(c->HL)
        case 0x4f: _z80_bit(c,c->A,0x2); break; // BIT 1,A
        case 0x50: _z80_bit(c,c->B,0x4); break; // BIT 2,B
        case 0x51: _z80_bit(c,c->C,0x4); break; // BIT 2,C
        case 0x52: _z80_bit(c,c->D,0x4); break; // BIT 2,D
        case 0x53: _z80_bit(c,c->E,0x4); break; // BIT 2,E
        case 0x54: _z80_bit(c,c->H,0x4); break; // BIT 2,H
        case 0x55: _z80_bit(c,c->L,0x4); break; // BIT 2,L
        case 0x56: { uint16_t a=c->HL;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x4); } break; // BIT 2,(c->HL)
        case 0x57: _z80_bit(c,c->A,0x4); break; // BIT 2,A
        case 0x58: _z80_bit(c,c->B,0x8); break; // BIT 3,B
        case 0x59: _z80_bit(c,c->C,0x8); break; // BIT 3,C
        case 0x5a: _z80_bit(c,c->D,0x8); break; // BIT 3,D
        case 0x5b: _z80_bit(c,c->E,0x8); break; // BIT 3,E
        case 0x5c: _z80_bit(c,c->H,0x8); break; // BIT 3,H
        case 0x5d: _z80_bit(c,c->L,0x8); break; // BIT 3,L
        case 0x5e: { uint16_t a=c->HL;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x8); } break; // BIT 3,(c->HL)
        case 0x5f: _z80_bit(c,c->A,0x8); break; // BIT 3,A
        case 0x60: _z80_bit(c,c->B,0x10); break; // BIT 4,B
        case 0x61: _z80_bit(c,c->C,0x10); break; // BIT 4,C
        case 0x62: _z80_bit(c,c->D,0x10); break; // BIT 4,D
        case 0x63: _z80_bit(c,c->E,0x10); break; // BIT 4,E
        case 0x64: _z80_bit(c,c->H,0x10); break; // BIT 4,H
        case 0x65: _z80_bit(c,c->L,0x10); break; // BIT 4,L
        case 0x66: { uint16_t a=c->HL;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x10); } break; // BIT 4,(c->HL)
        case 0x67: _z80_bit(c,c->A,0x10); break; // BIT 4,A
        case 0x68: _z80_bit(c,c->B,0x20); break; // BIT 5,B
        case 0x69: _z80_bit(c,c->C,0x20); break; // BIT 5,C
        case 0x6a: _z80_bit(c,c->D,0x20); break; // BIT 5,D
        case 0x6b: _z80_bit(c,c->E,0x20); break; // BIT 5,E
        case 0x6c: _z80_bit(c,c->H,0x20); break; // BIT 5,H
        case 0x6d: _z80_bit(c,c->L,0x20); break; // BIT 5,L
        case 0x6e: { uint16_t a=c->HL;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x20); } break; // BIT 5,(c->HL)
        case 0x6f: _z80_bit(c,c->A,0x20); break; // BIT 5,A
        case 0x70: _z80_bit(c,c->B,0x40); break; // BIT 6,B
        case 0x71: _z80_bit(c,c->C,0x40); break; // BIT 6,C
        case 0x72: _z80_bit(c,c->D,0x40); break; // BIT 6,D
        case 0x73: _z80_bit(c,c->E,0x40); break; // BIT 6,E
        case 0x74: _z80_bit(c,c->H,0x40); break; // BIT 6,H
        case 0x75: _z80_bit(c,c->L,0x40); break; // BIT 6,L
        case 0x76: { uint16_t a=c->HL;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x40); } break; // BIT 6,(c->HL)
        case 0x77: _z80_bit(c,c->A,0x40); break; // BIT 6,A
        case 0x78: _z80_bit(c,c->B,0x80); break; // BIT 7,B
        case 0x79: _z80_bit(c,c->C,0x80); break; // BIT 7,C
        case 0x7a: _z80_bit(c,c->D,0x80); break; // BIT 7,D
        case 0x7b: _z80_bit(c,c->E,0x80); break; // BIT 7,E
        case 0x7c: _z80_bit(c,c->H,0x80); break; // BIT 7,H
        case 0x7d: _z80_bit(c,c->L,0x80); break; // BIT 7,L
        case 0x7e: { uint16_t a=c->HL;_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x80); } break; // BIT 7,(c->HL)
        case 0x7f: _z80_bit(c,c->A,0x80); break; // BIT 7,A
        case 0x80: c->B&=~0x1; break; // RES 0,B
        case 0x81: c->C&=~0x1; break; // RES 0,C
        case 0x82: c->D&=~0x1; break; // RES 0,D
        case 0x83: c->E&=~0x1; break; // RES 0,E
        case 0x84: c->H&=~0x1; break; // RES 0,H
        case 0x85: c->L&=~0x1; break; // RES 0,L
        case 0x86: { uint16_t a=c->HL;_T();uint8_t v;_RD(a,v);_WR(a,v&~0x1); } break; // RES 0,(c->HL)
        case 0x87: c->A&=~0x1; break; // RES 0,A
        case 0x88: c->B&=~0x2; break; // RES 1,B
        case 0x89: c->C&=~0x2; break; // RES 1,C
        case 0x8a: c->D&=~0x2; break; // RES 1,D
        case 0x8b: c->E&=~0x2; break; // RES 1,E
        case 0x8c: c->H&=~0x2; break; // RES 1,H
        case 0x8d: c->L&=~0x2; break; // RES 1,L
        case 0x8e: { uint16_t a=c->HL;_T();uint8_t v;_RD(a,v);_WR(a,v&~0x2); } break; // RES 1,(c->HL)
        case 0x8f: c->A&=~0x2; break; // RES 1,A
        case 0x90: c->B&=~0x4; break; // RES 2,B
        case 0x91: c->C&=~0x4; break; // RES 2,C
        case 0x92: c->D&=~0x4; break; // RES 2,D
        case 0x93: c->E&=~0x4; break; // RES 2,E
        case 0x94: c->H&=~0x4; break; // RES 2,H
        case 0x95: c->L&=~0x4; break; // RES 2,L
        case 0x96: { uint16_t a=c->HL;_T();uint8_t v;_RD(a,v);_WR(a,v&~0x4); } break; // RES 2,(c->HL)
        case 0x97: c->A&=~0x4; break; // RES 2,A
        case 0x98: c->B&=~0x8; break; // RES 3,B
        case 0x99: c->C&=~0x8; break; // RES 3,C
        case 0x9a: c->D&=~0x8; break; // RES 3,D
        case 0x9b: c->E&=~0x8; break; // RES 3,E
        case 0x9c: c->H&=~0x8; break; // RES 3,H
        case 0x9d: c->L&=~0x8; break; // RES 3,L
        case 0x9e: { uint16_t a=c->HL;_T();uint8_t v;_RD(a,v);_WR(a,v&~0x8); } break; // RES 3,(c->HL)
        case 0x9f: c->A&=~0x8; break; // RES 3,A
        case 0xa0: c->B&=~0x10; break; // RES 4,B
        case 0xa1: c->C&=~0x10; break; // RES 4,C
        case 0xa2: c->D&=~0x10; break; // RES 4,D
        case 0xa3: c->E&=~0x10; break; // RES 4,E
        case 0xa4: c->H&=~0x10; break; // RES 4,H
        case 0xa5: c->L&=~0x10; break; // RES 4,L
        case 0xa6: { uint16_t a=c->HL;_T();uint8_t v;_RD(a,v);_WR(a,v&~0x10); } break; // RES 4,(c->HL)
        case 0xa7: c->A&=~0x10; break; // RES 4,A
        case 0xa8: c->B&=~0x20; break; // RES 5,B
        case 0xa9: c->C&=~0x20; break; // RES 5,C
        case 0xaa: c->D&=~0x20; break; // RES 5,D
        case 0xab: c->E&=~0x20; break; // RES 5,E
        case 0xac: c->H&=~0x20; break; // RES 5,H
        case 0xad: c->L&=~0x20; break; // RES 5,L
        case 0xae: { uint16_t a=c->HL;_T();uint8_t v;_RD(a,v);_WR(a,v&~0x20); } break; // RES 5,(c->HL)
        case 0xaf: c->A&=~0x20; break; // RES 5,A
        case 0xb0: c->B&=~0x40; break; // RES 6,B
        case 0xb1: c->C&=~0x40; break; // RES 6,C
        case 0xb2: c->D&=~0x40; break; // RES 6,D
        case 0xb3: c->E&=~0x40; break; // RES 6,E
        case 0xb4: c->H&=~0x40; break; // RES 6,H
        case 0xb5: c->L&=~0x40; break; // RES 6,L
        case 0xb6: { uint16_t a=c->HL;_T();uint8_t v;_RD(a,v);_WR(a,v&~0x40); } break; // RES 6,(c->HL)
        case 0xb7: c->A&=~0x40; break; // RES 6,A
        case 0xb8: c->B&=~0x80; break; // RES 7,B
        case 0xb9: c->C&=~0x80; break; // RES 7,C
        case 0xba: c->D&=~0x80; break; // RES 7,D
        case 0xbb: c->E&=~0x80; break; // RES 7,E
        case 0xbc: c->H&=~0x80; break; // RES 7,H
        case 0xbd: c->L&=~0x80; break; // RES 7,L
        case 0xbe: { uint16_t a=c->HL;_T();uint8_t v;_RD(a,v);_WR(a,v&~0x80); } break; // RES 7,(c->HL)
        case 0xbf: c->A&=~0x80; break; // RES 7,A
        case 0xc0: c->B|=0x1; break; // SET 0,B
        case 0xc1: c->C|=0x1; break; // SET 0,C
        case 0xc2: c->D|=0x1; break; // SET 0,D
        case 0xc3: c->E|=0x1; break; // SET 0,E
        case 0xc4: c->H|=0x1; break; // SET 0,H
        case 0xc5: c->L|=0x1; break; // SET 0,L
        case 0xc6: { uint16_t a=c->HL;_T();uint8_t v;_RD(a,v);_WR(a,v|0x1);} break; // SET 0,(c->HL)
        case 0xc7: c->A|=0x1; break; // SET 0,A
        case 0xc8: c->B|=0x2; break; // SET 1,B
        case 0xc9: c->C|=0x2; break; // SET 1,C
        case 0xca: c->D|=0x2; break; // SET 1,D
        case 0xcb: c->E|=0x2; break; // SET 1,E
        case 0xcc: c->H|=0x2; break; // SET 1,H
        case 0xcd: c->L|=0x2; break; // SET 1,L
        case 0xce: { uint16_t a=c->HL;_T();uint8_t v;_RD(a,v);_WR(a,v|0x2);} break; // SET 1,(c->HL)
        case 0xcf: c->A|=0x2; break; // SET 1,A
        case 0xd0: c->B|=0x4; break; // SET 2,B
        case 0xd1: c->C|=0x4; break; // SET 2,C
        case 0xd2: c->D|=0x4; break; // SET 2,D
        case 0xd3: c->E|=0x4; break; // SET 2,E
        case 0xd4: c->H|=0x4; break; // SET 2,H
        case 0xd5: c->L|=0x4; break; // SET 2,L
        case 0xd6: { uint16_t a=c->HL;_T();uint8_t v;_RD(a,v);_WR(a,v|0x4);} break; // SET 2,(c->HL)
        case 0xd7: c->A|=0x4; break; // SET 2,A
        case 0xd8: c->B|=0x8; break; // SET 3,B
        case 0xd9: c->C|=0x8; break; // SET 3,C
        case 0xda: c->D|=0x8; break; // SET 3,D
        case 0xdb: c->E|=0x8; break; // SET 3,E
        case 0xdc: c->H|=0x8; break; // SET 3,H
        case 0xdd: c->L|=0x8; break; // SET 3,L
        case 0xde: { uint16_t a=c->HL;_T();uint8_t v;_RD(a,v);_WR(a,v|0x8);} break; // SET 3,(c->HL)
        case 0xdf: c->A|=0x8; break; // SET 3,A
        case 0xe0: c->B|=0x10; break; // SET 4,B
        case 0xe1: c->C|=0x10; break; // SET 4,C
        case 0xe2: c->D|=0x10; break; // SET 4,D
        case 0xe3: c->E|=0x10; break; // SET 4,E
        case 0xe4: c->H|=0x10; break; // SET 4,H
        case 0xe5: c->L|=0x10; break; // SET 4,L
        case 0xe6: { uint16_t a=c->HL;_T();uint8_t v;_RD(a,v);_WR(a,v|0x10);} break; // SET 4,(c->HL)
        case 0xe7: c->A|=0x10; break; // SET 4,A
        case 0xe8: c->B|=0x20; break; // SET 5,B
        case 0xe9: c->C|=0x20; break; // SET 5,C
        case 0xea: c->D|=0x20; break; // SET 5,D
        case 0xeb: c->E|=0x20; break; // SET 5,E
        case 0xec: c->H|=0x20; break; // SET 5,H
        case 0xed: c->L|=0x20; break; // SET 5,L
        case 0xee: { uint16_t a=c->HL;_T();uint8_t v;_RD(a,v);_WR(a,v|0x20);} break; // SET 5,(c->HL)
        case 0xef: c->A|=0x20; break; // SET 5,A
        case 0xf0: c->B|=0x40; break; // SET 6,B
        case 0xf1: c->C|=0x40; break; // SET 6,C
        case 0xf2: c->D|=0x40; break; // SET 6,D
        case 0xf3: c->E|=0x40; break; // SET 6,E
        case 0xf4: c->H|=0x40; break; // SET 6,H
        case 0xf5: c->L|=0x40; break; // SET 6,L
        case 0xf6: { uint16_t a=c->HL;_T();uint8_t v;_RD(a,v);_WR(a,v|0x40);} break; // SET 6,(c->HL)
        case 0xf7: c->A|=0x40; break; // SET 6,A
        case 0xf8: c->B|=0x80; break; // SET 7,B
        case 0xf9: c->C|=0x80; break; // SET 7,C
        case 0xfa: c->D|=0x80; break; // SET 7,D
        case 0xfb: c->E|=0x80; break; // SET 7,E
        case 0xfc: c->H|=0x80; break; // SET 7,H
        case 0xfd: c->L|=0x80; break; // SET 7,L
        case 0xfe: { uint16_t a=c->HL;_T();uint8_t v;_RD(a,v);_WR(a,v|0x80);} break; // SET 7,(c->HL)
        case 0xff: c->A|=0x80; break; // SET 7,A
        default: break;
      }
      break;
    case 0xcc: ticks=_z80_callcc(c,(c->F&Z80_ZF),tick,ticks); break; // CALL Z,nn
    case 0xcd: ticks=_z80_call(c,tick,ticks); break; // CALL nn
    case 0xce: {uint8_t v;_RD(c->PC++,v);_z80_adc(c,v);} break; // ADC n
    case 0xcf: ticks=_z80_rst(c,0x8,tick,ticks); break; // RST 0x8
    case 0xd0: ticks=_z80_retcc(c,!(c->F&Z80_CF),tick,ticks); break; // RET NC
    case 0xd1: { uint8_t l;_RD(c->SP++,l);uint8_t h;_RD(c->SP++,h);c->DE=(h<<8)|l; } break; // POP DE
    case 0xd2: _IMM16(); if (!(c->F&Z80_CF)) { c->PC=c->WZ; } break; // JP NC,nn
    case 0xd3: {uint8_t v;_RD(c->PC++,v);c->WZ=((c->A<<8)|v);_OUT(c->WZ,c->A);c->Z++;} break; // OUT (n),A
    case 0xd4: ticks=_z80_callcc(c,!(c->F&Z80_CF),tick,ticks); break; // CALL NC,nn
    case 0xd5: _T();_WR(--c->SP,(uint8_t)(c->DE>>8)); _WR(--c->SP,(uint8_t)c->DE); break; // PUSH DE
    case 0xd6: {uint8_t v;_RD(c->PC++,v);_z80_sub(c,v);} break; // SUB n
    case 0xd7: ticks=_z80_rst(c,0x10,tick,ticks); break; // RST 0x10
    case 0xd8: ticks=_z80_retcc(c,(c->F&Z80_CF),tick,ticks); break; // RET C
    case 0xd9: _SWP16(c->BC,c->BC_);_SWP16(c->DE,c->DE_);_SWP16(c->HL,c->HL_); break; // EXX
    case 0xda: _IMM16(); if ((c->F&Z80_CF)) { c->PC=c->WZ; } break; // JP C,nn
    case 0xdb: {uint8_t v;_RD(c->PC++,v);c->WZ=((c->A<<8)|v);_IN(c->WZ++,c->A);} break; // IN A,(n)
    case 0xdc: ticks=_z80_callcc(c,(c->F&Z80_CF),tick,ticks); break; // CALL C,nn
    case 0xdd:
      _FETCH(); switch (opcode) {
        case 0x0:   break; // NOP
        case 0x1: _IMM16(); c->BC=c->WZ; break; // LD BC,nn
        case 0x2: c->WZ=c->BC;_WR(c->WZ++,c->A);c->W=c->A; break; // LD (BC),A
        case 0x3: _T();_T();c->BC++; break; // INC BC
        case 0x4: c->B=_z80_inc(c,c->B); break; // INC B
        case 0x5: c->B=_z80_dec(c,c->B); break; // DEC B
        case 0x6: _RD(c->PC++,c->B); break; // LD B,n
        case 0x7: _z80_rlca(c); break; // RLCA
        case 0x8: _SWP16(c->AF,c->AF_); break; // EX AF,AF'
        case 0x9: c->IX=_z80_add16(c,c->IX,c->BC);_T();_T();_T();_T();_T();_T();_T(); break; // ADD IX,BC
        case 0xa: c->WZ=c->BC;_RD(c->WZ++,c->A); break; // LD A,(BC)
        case 0xb: _T();_T();c->BC--; break; // DEC BC
        case 0xc: c->C=_z80_inc(c,c->C); break; // INC C
        case 0xd: c->C=_z80_dec(c,c->C); break; // DEC C
        case 0xe: _RD(c->PC++,c->C); break; // LD C,n
        case 0xf: _z80_rrca(c); break; // RRCA
        case 0x10: ticks=_z80_djnz(c,tick,ticks); break; // DJNZ
        case 0x11: _IMM16(); c->DE=c->WZ; break; // LD DE,nn
        case 0x12: c->WZ=c->DE;_WR(c->WZ++,c->A);c->W=c->A; break; // LD (DE),A
        case 0x13: _T();_T();c->DE++; break; // INC DE
        case 0x14: c->D=_z80_inc(c,c->D); break; // INC D
        case 0x15: c->D=_z80_dec(c,c->D); break; // DEC D
        case 0x16: _RD(c->PC++,c->D); break; // LD D,n
        case 0x17: _z80_rla(c); break; // RLA
        case 0x18: ticks=_z80_jr(c,tick,ticks); break; // JR d
        case 0x19: c->IX=_z80_add16(c,c->IX,c->DE);_T();_T();_T();_T();_T();_T();_T(); break; // ADD IX,DE
        case 0x1a: c->WZ=c->DE;_RD(c->WZ++,c->A); break; // LD A,(DE)
        case 0x1b: _T();_T();c->DE--; break; // DEC DE
        case 0x1c: c->E=_z80_inc(c,c->E); break; // INC E
        case 0x1d: c->E=_z80_dec(c,c->E); break; // DEC E
        case 0x1e: _RD(c->PC++,c->E); break; // LD E,n
        case 0x1f: _z80_rra(c); break; // RRA
        case 0x20: ticks=_z80_jr_cc(c,!(c->F&Z80_ZF),tick,ticks); break; // JR NZ,d
        case 0x21: _IMM16(); c->IX=c->WZ; break; // LD IX,nn
        case 0x22: {_IMM16();_WR(c->WZ++,(uint8_t)c->IX);_WR(c->WZ,(uint8_t)(c->IX>>8));} break; // LD (nn),IX
        case 0x23: _T();_T();c->IX++; break; // INC IX
        case 0x24: c->IXH=_z80_inc(c,c->IXH); break; // INC IXH
        case 0x25: c->IXH=_z80_dec(c,c->IXH); break; // DEC IXH
        case 0x26: _RD(c->PC++,c->IXH); break; // LD IXH,n
        case 0x27: _z80_daa(c); break; // DAA
        case 0x28: ticks=_z80_jr_cc(c,(c->F&Z80_ZF),tick,ticks); break; // JR Z,d
        case 0x29: c->IX=_z80_add16(c,c->IX,c->IX);_T();_T();_T();_T();_T();_T();_T(); break; // ADD IX,IX
        case 0x2a: {_IMM16();uint8_t l;_RD(c->WZ++,l);uint8_t h;_RD(c->WZ,h);c->IX=(h<<8)|l;} break; // LD IX,(nn)
        case 0x2b: _T();_T();c->IX--; break; // DEC IX
        case 0x2c: c->IXL=_z80_inc(c,c->IXL); break; // INC IXL
        case 0x2d: c->IXL=_z80_dec(c,c->IXL); break; // DEC IXL
        case 0x2e: _RD(c->PC++,c->IXL); break; // LD IXL,n
        case 0x2f: _z80_cpl(c); break; // CPL
        case 0x30: ticks=_z80_jr_cc(c,!(c->F&Z80_CF),tick,ticks); break; // JR NC,d
        case 0x31: _IMM16(); c->SP=c->WZ; break; // LD SP,nn
        case 0x32: _IMM16();_WR(c->WZ++,c->A);c->W=c->A; break; // LD (nn),A
        case 0x33: _T();_T();c->SP++; break; // INC SP
        case 0x34: { int8_t d;_RD(c->PC++,d);uint16_t a=c->WZ=c->IX+d;;_T();_T();_T();_T();_T();_T();uint8_t v;_RD(a,v);_WR(a,_z80_inc(c,v)); } break; // INC (c->IX+d)
        case 0x35: { int8_t d;_RD(c->PC++,d);uint16_t a=c->WZ=c->IX+d;;_T();_T();_T();_T();_T();_T();uint8_t v;_RD(a,v);_WR(a,_z80_dec(c,v)); } break; // DEC (c->IX+d)
        case 0x36: { int8_t d;_RD(c->PC++,d);uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(c->PC++,v);_WR(a,v); } break; // LD (c->IX+d),n
        case 0x37: _z80_scf(c); break; // SCF
        case 0x38: ticks=_z80_jr_cc(c,(c->F&Z80_CF),tick,ticks); break; // JR C,d
        case 0x39: c->IX=_z80_add16(c,c->IX,c->SP);_T();_T();_T();_T();_T();_T();_T(); break; // ADD IX,SP
        case 0x3a: _IMM16();_RD(c->WZ++,c->A); break; // LD A,(nn)
        case 0x3b: _T();_T();c->SP--; break; // DEC SP
        case 0x3c: c->A=_z80_inc(c,c->A); break; // INC A
        case 0x3d: c->A=_z80_dec(c,c->A); break; // DEC A
        case 0x3e: _RD(c->PC++,c->A); break; // LD A,n
        case 0x3f: _z80_ccf(c); break; // CCF
        case 0x40: c->B=c->B; break; // LD B,B
        case 0x41: c->B=c->C; break; // LD B,C
        case 0x42: c->B=c->D; break; // LD B,D
        case 0x43: c->B=c->E; break; // LD B,E
        case 0x44: c->B=c->IXH; break; // LD B,IXH
        case 0x45: c->B=c->IXL; break; // LD B,IXL
        case 0x46: { int8_t d;_RD(c->PC++,d);uint16_t a=c->WZ=c->IX+d;;_T();_T();_T();_T();_T();_RD(a,c->B); }; break; // LD B,(c->IX+d)
        case 0x47: c->B=c->A; break; // LD B,A
        case 0x48: c->C=c->B; break; // LD C,B
        case 0x49: c->C=c->C; break; // LD C,C
        case 0x4a: c->C=c->D; break; // LD C,D
        case 0x4b: c->C=c->E; break; // LD C,E
        case 0x4c: c->C=c->IXH; break; // LD C,IXH
        case 0x4d: c->C=c->IXL; break; // LD C,IXL
        case 0x4e: { int8_t d;_RD(c->PC++,d);uint16_t a=c->WZ=c->IX+d;;_T();_T();_T();_T();_T();_RD(a,c->C); }; break; // LD C,(c->IX+d)
        case 0x4f: c->C=c->A; break; // LD C,A
        case 0x50: c->D=c->B; break; // LD D,B
        case 0x51: c->D=c->C; break; // LD D,C
        case 0x52: c->D=c->D; break; // LD D,D
        case 0x53: c->D=c->E; break; // LD D,E
        case 0x54: c->D=c->IXH; break; // LD D,IXH
        case 0x55: c->D=c->IXL; break; // LD D,IXL
        case 0x56: { int8_t d;_RD(c->PC++,d);uint16_t a=c->WZ=c->IX+d;;_T();_T();_T();_T();_T();_RD(a,c->D); }; break; // LD D,(c->IX+d)
        case 0x57: c->D=c->A; break; // LD D,A
        case 0x58: c->E=c->B; break; // LD E,B
        case 0x59: c->E=c->C; break; // LD E,C
        case 0x5a: c->E=c->D; break; // LD E,D
        case 0x5b: c->E=c->E; break; // LD E,E
        case 0x5c: c->E=c->IXH; break; // LD E,IXH
        case 0x5d: c->E=c->IXL; break; // LD E,IXL
        case 0x5e: { int8_t d;_RD(c->PC++,d);uint16_t a=c->WZ=c->IX+d;;_T();_T();_T();_T();_T();_RD(a,c->E); }; break; // LD E,(c->IX+d)
        case 0x5f: c->E=c->A; break; // LD E,A
        case 0x60: c->IXH=c->B; break; // LD IXH,B
        case 0x61: c->IXH=c->C; break; // LD IXH,C
        case 0x62: c->IXH=c->D; break; // LD IXH,D
        case 0x63: c->IXH=c->E; break; // LD IXH,E
        case 0x64: c->IXH=c->IXH; break; // LD IXH,IXH
        case 0x65: c->IXH=c->IXL; break; // LD IXH,IXL
        case 0x66: { int8_t d;_RD(c->PC++,d);uint16_t a=c->WZ=c->IX+d;;_T();_T();_T();_T();_T();_RD(a,c->H); }; break; // LD H,(c->IX+d)
        case 0x67: c->IXH=c->A; break; // LD IXH,A
        case 0x68: c->IXL=c->B; break; // LD IXL,B
        case 0x69: c->IXL=c->C; break; // LD IXL,C
        case 0x6a: c->IXL=c->D; break; // LD IXL,D
        case 0x6b: c->IXL=c->E; break; // LD IXL,E
        case 0x6c: c->IXL=c->IXH; break; // LD IXL,IXH
        case 0x6d: c->IXL=c->IXL; break; // LD IXL,IXL
        case 0x6e: { int8_t d;_RD(c->PC++,d);uint16_t a=c->WZ=c->IX+d;;_T();_T();_T();_T();_T();_RD(a,c->L); }; break; // LD L,(c->IX+d)
        case 0x6f: c->IXL=c->A; break; // LD IXL,A
        case 0x70: { int8_t d;_RD(c->PC++,d);uint16_t a=c->WZ=c->IX+d;;_T();_T();_T();_T();_T();_WR(a,c->B); }; break; // LD (c->IX+d),B
        case 0x71: { int8_t d;_RD(c->PC++,d);uint16_t a=c->WZ=c->IX+d;;_T();_T();_T();_T();_T();_WR(a,c->C); }; break; // LD (c->IX+d),C
        case 0x72: { int8_t d;_RD(c->PC++,d);uint16_t a=c->WZ=c->IX+d;;_T();_T();_T();_T();_T();_WR(a,c->D); }; break; // LD (c->IX+d),D
        case 0x73: { int8_t d;_RD(c->PC++,d);uint16_t a=c->WZ=c->IX+d;;_T();_T();_T();_T();_T();_WR(a,c->E); }; break; // LD (c->IX+d),E
        case 0x74: { int8_t d;_RD(c->PC++,d);uint16_t a=c->WZ=c->IX+d;;_T();_T();_T();_T();_T();_WR(a,c->H); }; break; // LD (c->IX+d),H
        case 0x75: { int8_t d;_RD(c->PC++,d);uint16_t a=c->WZ=c->IX+d;;_T();_T();_T();_T();_T();_WR(a,c->L); }; break; // LD (c->IX+d),L
        case 0x76: _z80_halt(c); break; // HALT
        case 0x77: { int8_t d;_RD(c->PC++,d);uint16_t a=c->WZ=c->IX+d;;_T();_T();_T();_T();_T();_WR(a,c->A); }; break; // LD (c->IX+d),A
        case 0x78: c->A=c->B; break; // LD A,B
        case 0x79: c->A=c->C; break; // LD A,C
        case 0x7a: c->A=c->D; break; // LD A,D
        case 0x7b: c->A=c->E; break; // LD A,E
        case 0x7c: c->A=c->IXH; break; // LD A,IXH
        case 0x7d: c->A=c->IXL; break; // LD A,IXL
        case 0x7e: { int8_t d;_RD(c->PC++,d);uint16_t a=c->WZ=c->IX+d;;_T();_T();_T();_T();_T();_RD(a,c->A); }; break; // LD A,(c->IX+d)
        case 0x7f: c->A=c->A; break; // LD A,A
        case 0x80: _z80_add(c,c->B); break; // ADD B
        case 0x81: _z80_add(c,c->C); break; // ADD C
        case 0x82: _z80_add(c,c->D); break; // ADD D
        case 0x83: _z80_add(c,c->E); break; // ADD E
        case 0x84: _z80_add(c,c->IXH); break; // ADD IXH
        case 0x85: _z80_add(c,c->IXL); break; // ADD IXL
        case 0x86: { int8_t d;_RD(c->PC++,d);uint16_t a=c->WZ=c->IX+d;;_T();_T();_T();_T();_T();uint8_t val;_RD(a,val);_z80_add(c,val); }; break; // ADD (c->IX+d)
        case 0x87: _z80_add(c,c->A); break; // ADD A
        case 0x88: _z80_adc(c,c->B); break; // ADC B
        case 0x89: _z80_adc(c,c->C); break; // ADC C
        case 0x8a: _z80_adc(c,c->D); break; // ADC D
        case 0x8b: _z80_adc(c,c->E); break; // ADC E
        case 0x8c: _z80_adc(c,c->IXH); break; // ADC IXH
        case 0x8d: _z80_adc(c,c->IXL); break; // ADC IXL
        case 0x8e: { int8_t d;_RD(c->PC++,d);uint16_t a=c->WZ=c->IX+d;;_T();_T();_T();_T();_T();uint8_t val;_RD(a,val);_z80_adc(c,val); }; break; // ADC (c->IX+d)
        case 0x8f: _z80_adc(c,c->A); break; // ADC A
        case 0x90: _z80_sub(c,c->B); break; // SUB B
        case 0x91: _z80_sub(c,c->C); break; // SUB C
        case 0x92: _z80_sub(c,c->D); break; // SUB D
        case 0x93: _z80_sub(c,c->E); break; // SUB E
        case 0x94: _z80_sub(c,c->IXH); break; // SUB IXH
        case 0x95: _z80_sub(c,c->IXL); break; // SUB IXL
        case 0x96: { int8_t d;_RD(c->PC++,d);uint16_t a=c->WZ=c->IX+d;;_T();_T();_T();_T();_T();uint8_t val;_RD(a,val);_z80_sub(c,val); }; break; // SUB (c->IX+d)
        case 0x97: _z80_sub(c,c->A); break; // SUB A
        case 0x98: _z80_sbc(c,c->B); break; // SBC B
        case 0x99: _z80_sbc(c,c->C); break; // SBC C
        case 0x9a: _z80_sbc(c,c->D); break; // SBC D
        case 0x9b: _z80_sbc(c,c->E); break; // SBC E
        case 0x9c: _z80_sbc(c,c->IXH); break; // SBC IXH
        case 0x9d: _z80_sbc(c,c->IXL); break; // SBC IXL
        case 0x9e: { int8_t d;_RD(c->PC++,d);uint16_t a=c->WZ=c->IX+d;;_T();_T();_T();_T();_T();uint8_t val;_RD(a,val);_z80_sbc(c,val); }; break; // SBC (c->IX+d)
        case 0x9f: _z80_sbc(c,c->A); break; // SBC A
        case 0xa0: _z80_and(c,c->B); break; // AND B
        case 0xa1: _z80_and(c,c->C); break; // AND C
        case 0xa2: _z80_and(c,c->D); break; // AND D
        case 0xa3: _z80_and(c,c->E); break; // AND E
        case 0xa4: _z80_and(c,c->IXH); break; // AND IXH
        case 0xa5: _z80_and(c,c->IXL); break; // AND IXL
        case 0xa6: { int8_t d;_RD(c->PC++,d);uint16_t a=c->WZ=c->IX+d;;_T();_T();_T();_T();_T();uint8_t val;_RD(a,val);_z80_and(c,val); }; break; // AND (c->IX+d)
        case 0xa7: _z80_and(c,c->A); break; // AND A
        case 0xa8: _z80_xor(c,c->B); break; // XOR B
        case 0xa9: _z80_xor(c,c->C); break; // XOR C
        case 0xaa: _z80_xor(c,c->D); break; // XOR D
        case 0xab: _z80_xor(c,c->E); break; // XOR E
        case 0xac: _z80_xor(c,c->IXH); break; // XOR IXH
        case 0xad: _z80_xor(c,c->IXL); break; // XOR IXL
        case 0xae: { int8_t d;_RD(c->PC++,d);uint16_t a=c->WZ=c->IX+d;;_T();_T();_T();_T();_T();uint8_t val;_RD(a,val);_z80_xor(c,val); }; break; // XOR (c->IX+d)
        case 0xaf: _z80_xor(c,c->A); break; // XOR A
        case 0xb0: _z80_or(c,c->B); break; // OR B
        case 0xb1: _z80_or(c,c->C); break; // OR C
        case 0xb2: _z80_or(c,c->D); break; // OR D
        case 0xb3: _z80_or(c,c->E); break; // OR E
        case 0xb4: _z80_or(c,c->IXH); break; // OR IXH
        case 0xb5: _z80_or(c,c->IXL); break; // OR IXL
        case 0xb6: { int8_t d;_RD(c->PC++,d);uint16_t a=c->WZ=c->IX+d;;_T();_T();_T();_T();_T();uint8_t val;_RD(a,val);_z80_or(c,val); }; break; // OR (c->IX+d)
        case 0xb7: _z80_or(c,c->A); break; // OR A
        case 0xb8: _z80_cp(c,c->B); break; // CP B
        case 0xb9: _z80_cp(c,c->C); break; // CP C
        case 0xba: _z80_cp(c,c->D); break; // CP D
        case 0xbb: _z80_cp(c,c->E); break; // CP E
        case 0xbc: _z80_cp(c,c->IXH); break; // CP IXH
        case 0xbd: _z80_cp(c,c->IXL); break; // CP IXL
        case 0xbe: { int8_t d;_RD(c->PC++,d);uint16_t a=c->WZ=c->IX+d;;_T();_T();_T();_T();_T();uint8_t val;_RD(a,val);_z80_cp(c,val); }; break; // CP (c->IX+d)
        case 0xbf: _z80_cp(c,c->A); break; // CP A
        case 0xc0: ticks=_z80_retcc(c,!(c->F&Z80_ZF),tick,ticks); break; // RET NZ
        case 0xc1: { uint8_t l;_RD(c->SP++,l);uint8_t h;_RD(c->SP++,h);c->BC=(h<<8)|l; } break; // POP BC
        case 0xc2: _IMM16(); if (!(c->F&Z80_ZF)) { c->PC=c->WZ; } break; // JP NZ,nn
        case 0xc3: _IMM16(); c->PC=c->WZ; break; // JP nn
        case 0xc4: ticks=_z80_callcc(c,!(c->F&Z80_ZF),tick,ticks); break; // CALL NZ,nn
        case 0xc5: _T();_WR(--c->SP,(uint8_t)(c->BC>>8)); _WR(--c->SP,(uint8_t)c->BC); break; // PUSH BC
        case 0xc6: {uint8_t v;_RD(c->PC++,v);_z80_add(c,v);} break; // ADD n
        case 0xc7: ticks=_z80_rst(c,0x0,tick,ticks); break; // RST 0x0
        case 0xc8: ticks=_z80_retcc(c,(c->F&Z80_ZF),tick,ticks); break; // RET Z
        case 0xc9: ticks=_z80_ret(c,tick,ticks); break; // RET
        case 0xca: _IMM16(); if ((c->F&Z80_ZF)) { c->PC=c->WZ; } break; // JP Z,nn
        case 0xcb:
          { int8_t d; _RD(c->PC++, d);
          _FETCH(); switch (opcode) {
            case 0x0: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->B=_z80_rlc(c,v);_WR(a,c->B); } break; // RLC (c->IX+d),B
            case 0x1: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->C=_z80_rlc(c,v);_WR(a,c->C); } break; // RLC (c->IX+d),C
            case 0x2: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->D=_z80_rlc(c,v);_WR(a,c->D); } break; // RLC (c->IX+d),D
            case 0x3: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->E=_z80_rlc(c,v);_WR(a,c->E); } break; // RLC (c->IX+d),E
            case 0x4: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->H=_z80_rlc(c,v);_WR(a,c->H); } break; // RLC (c->IX+d),H
            case 0x5: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->L=_z80_rlc(c,v);_WR(a,c->L); } break; // RLC (c->IX+d),L
            case 0x6: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_WR(a,_z80_rlc(c,v)); } break; // RLC (c->IX+d)
            case 0x7: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->A=_z80_rlc(c,v);_WR(a,c->A); } break; // RLC (c->IX+d),A
            case 0x8: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->B=_z80_rrc(c,v);_WR(a,c->B); } break; // RRC (c->IX+d),B
            case 0x9: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->C=_z80_rrc(c,v);_WR(a,c->C); } break; // RRC (c->IX+d),C
            case 0xa: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->D=_z80_rrc(c,v);_WR(a,c->D); } break; // RRC (c->IX+d),D
            case 0xb: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->E=_z80_rrc(c,v);_WR(a,c->E); } break; // RRC (c->IX+d),E
            case 0xc: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->H=_z80_rrc(c,v);_WR(a,c->H); } break; // RRC (c->IX+d),H
            case 0xd: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->L=_z80_rrc(c,v);_WR(a,c->L); } break; // RRC (c->IX+d),L
            case 0xe: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_WR(a,_z80_rrc(c,v)); } break; // RRC (c->IX+d)
            case 0xf: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->A=_z80_rrc(c,v);_WR(a,c->A); } break; // RRC (c->IX+d),A
            case 0x10: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->B=_z80_rl(c,v);_WR(a,c->B); } break; // RL (c->IX+d),B
            case 0x11: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->C=_z80_rl(c,v);_WR(a,c->C); } break; // RL (c->IX+d),C
            case 0x12: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->D=_z80_rl(c,v);_WR(a,c->D); } break; // RL (c->IX+d),D
            case 0x13: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->E=_z80_rl(c,v);_WR(a,c->E); } break; // RL (c->IX+d),E
            case 0x14: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->H=_z80_rl(c,v);_WR(a,c->H); } break; // RL (c->IX+d),H
            case 0x15: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->L=_z80_rl(c,v);_WR(a,c->L); } break; // RL (c->IX+d),L
            case 0x16: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_WR(a,_z80_rl(c,v)); } break; // RL (c->IX+d)
            case 0x17: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->A=_z80_rl(c,v);_WR(a,c->A); } break; // RL (c->IX+d),A
            case 0x18: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->B=_z80_rr(c,v);_WR(a,c->B); } break; // RR (c->IX+d),B
            case 0x19: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->C=_z80_rr(c,v);_WR(a,c->C); } break; // RR (c->IX+d),C
            case 0x1a: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->D=_z80_rr(c,v);_WR(a,c->D); } break; // RR (c->IX+d),D
            case 0x1b: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->E=_z80_rr(c,v);_WR(a,c->E); } break; // RR (c->IX+d),E
            case 0x1c: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->H=_z80_rr(c,v);_WR(a,c->H); } break; // RR (c->IX+d),H
            case 0x1d: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->L=_z80_rr(c,v);_WR(a,c->L); } break; // RR (c->IX+d),L
            case 0x1e: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_WR(a,_z80_rr(c,v)); } break; // RR (c->IX+d)
            case 0x1f: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->A=_z80_rr(c,v);_WR(a,c->A); } break; // RR (c->IX+d),A
            case 0x20: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->B=_z80_sla(c,v);_WR(a,c->B); } break; // SLA (c->IX+d),B
            case 0x21: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->C=_z80_sla(c,v);_WR(a,c->C); } break; // SLA (c->IX+d),C
            case 0x22: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->D=_z80_sla(c,v);_WR(a,c->D); } break; // SLA (c->IX+d),D
            case 0x23: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->E=_z80_sla(c,v);_WR(a,c->E); } break; // SLA (c->IX+d),E
            case 0x24: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->H=_z80_sla(c,v);_WR(a,c->H); } break; // SLA (c->IX+d),H
            case 0x25: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->L=_z80_sla(c,v);_WR(a,c->L); } break; // SLA (c->IX+d),L
            case 0x26: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_WR(a,_z80_sla(c,v)); } break; // SLA (c->IX+d)
            case 0x27: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->A=_z80_sla(c,v);_WR(a,c->A); } break; // SLA (c->IX+d),A
            case 0x28: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->B=_z80_sra(c,v);_WR(a,c->B); } break; // SRA (c->IX+d),B
            case 0x29: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->C=_z80_sra(c,v);_WR(a,c->C); } break; // SRA (c->IX+d),C
            case 0x2a: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->D=_z80_sra(c,v);_WR(a,c->D); } break; // SRA (c->IX+d),D
            case 0x2b: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->E=_z80_sra(c,v);_WR(a,c->E); } break; // SRA (c->IX+d),E
            case 0x2c: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->H=_z80_sra(c,v);_WR(a,c->H); } break; // SRA (c->IX+d),H
            case 0x2d: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->L=_z80_sra(c,v);_WR(a,c->L); } break; // SRA (c->IX+d),L
            case 0x2e: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_WR(a,_z80_sra(c,v)); } break; // SRA (c->IX+d)
            case 0x2f: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->A=_z80_sra(c,v);_WR(a,c->A); } break; // SRA (c->IX+d),A
            case 0x30: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->B=_z80_sll(c,v);_WR(a,c->B); } break; // SLL (c->IX+d),B
            case 0x31: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->C=_z80_sll(c,v);_WR(a,c->C); } break; // SLL (c->IX+d),C
            case 0x32: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->D=_z80_sll(c,v);_WR(a,c->D); } break; // SLL (c->IX+d),D
            case 0x33: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->E=_z80_sll(c,v);_WR(a,c->E); } break; // SLL (c->IX+d),E
            case 0x34: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->H=_z80_sll(c,v);_WR(a,c->H); } break; // SLL (c->IX+d),H
            case 0x35: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->L=_z80_sll(c,v);_WR(a,c->L); } break; // SLL (c->IX+d),L
            case 0x36: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_WR(a,_z80_sll(c,v)); } break; // SLL (c->IX+d)
            case 0x37: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->A=_z80_sll(c,v);_WR(a,c->A); } break; // SLL (c->IX+d),A
            case 0x38: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->B=_z80_srl(c,v);_WR(a,c->B); } break; // SRL (c->IX+d),B
            case 0x39: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->C=_z80_srl(c,v);_WR(a,c->C); } break; // SRL (c->IX+d),C
            case 0x3a: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->D=_z80_srl(c,v);_WR(a,c->D); } break; // SRL (c->IX+d),D
            case 0x3b: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->E=_z80_srl(c,v);_WR(a,c->E); } break; // SRL (c->IX+d),E
            case 0x3c: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->H=_z80_srl(c,v);_WR(a,c->H); } break; // SRL (c->IX+d),H
            case 0x3d: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->L=_z80_srl(c,v);_WR(a,c->L); } break; // SRL (c->IX+d),L
            case 0x3e: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_WR(a,_z80_srl(c,v)); } break; // SRL (c->IX+d)
            case 0x3f: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->A=_z80_srl(c,v);_WR(a,c->A); } break; // SRL (c->IX+d),A
            case 0x40: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x1); } break; // BIT 0,(c->IX+d)
            case 0x41: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x1); } break; // BIT 0,(c->IX+d)
            case 0x42: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x1); } break; // BIT 0,(c->IX+d)
            case 0x43: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x1); } break; // BIT 0,(c->IX+d)
            case 0x44: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x1); } break; // BIT 0,(c->IX+d)
            case 0x45: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x1); } break; // BIT 0,(c->IX+d)
            case 0x46: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x1); } break; // BIT 0,(c->IX+d)
            case 0x47: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x1); } break; // BIT 0,(c->IX+d)
            case 0x48: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x2); } break; // BIT 1,(c->IX+d)
            case 0x49: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x2); } break; // BIT 1,(c->IX+d)
            case 0x4a: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x2); } break; // BIT 1,(c->IX+d)
            case 0x4b: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x2); } break; // BIT 1,(c->IX+d)
            case 0x4c: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x2); } break; // BIT 1,(c->IX+d)
            case 0x4d: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x2); } break; // BIT 1,(c->IX+d)
            case 0x4e: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x2); } break; // BIT 1,(c->IX+d)
            case 0x4f: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x2); } break; // BIT 1,(c->IX+d)
            case 0x50: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x4); } break; // BIT 2,(c->IX+d)
            case 0x51: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x4); } break; // BIT 2,(c->IX+d)
            case 0x52: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x4); } break; // BIT 2,(c->IX+d)
            case 0x53: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x4); } break; // BIT 2,(c->IX+d)
            case 0x54: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x4); } break; // BIT 2,(c->IX+d)
            case 0x55: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x4); } break; // BIT 2,(c->IX+d)
            case 0x56: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x4); } break; // BIT 2,(c->IX+d)
            case 0x57: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x4); } break; // BIT 2,(c->IX+d)
            case 0x58: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x8); } break; // BIT 3,(c->IX+d)
            case 0x59: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x8); } break; // BIT 3,(c->IX+d)
            case 0x5a: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x8); } break; // BIT 3,(c->IX+d)
            case 0x5b: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x8); } break; // BIT 3,(c->IX+d)
            case 0x5c: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x8); } break; // BIT 3,(c->IX+d)
            case 0x5d: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x8); } break; // BIT 3,(c->IX+d)
            case 0x5e: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x8); } break; // BIT 3,(c->IX+d)
            case 0x5f: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x8); } break; // BIT 3,(c->IX+d)
            case 0x60: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x10); } break; // BIT 4,(c->IX+d)
            case 0x61: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x10); } break; // BIT 4,(c->IX+d)
            case 0x62: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x10); } break; // BIT 4,(c->IX+d)
            case 0x63: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x10); } break; // BIT 4,(c->IX+d)
            case 0x64: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x10); } break; // BIT 4,(c->IX+d)
            case 0x65: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x10); } break; // BIT 4,(c->IX+d)
            case 0x66: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x10); } break; // BIT 4,(c->IX+d)
            case 0x67: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x10); } break; // BIT 4,(c->IX+d)
            case 0x68: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x20); } break; // BIT 5,(c->IX+d)
            case 0x69: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x20); } break; // BIT 5,(c->IX+d)
            case 0x6a: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x20); } break; // BIT 5,(c->IX+d)
            case 0x6b: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x20); } break; // BIT 5,(c->IX+d)
            case 0x6c: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x20); } break; // BIT 5,(c->IX+d)
            case 0x6d: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x20); } break; // BIT 5,(c->IX+d)
            case 0x6e: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x20); } break; // BIT 5,(c->IX+d)
            case 0x6f: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x20); } break; // BIT 5,(c->IX+d)
            case 0x70: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x40); } break; // BIT 6,(c->IX+d)
            case 0x71: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x40); } break; // BIT 6,(c->IX+d)
            case 0x72: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x40); } break; // BIT 6,(c->IX+d)
            case 0x73: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x40); } break; // BIT 6,(c->IX+d)
            case 0x74: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x40); } break; // BIT 6,(c->IX+d)
            case 0x75: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x40); } break; // BIT 6,(c->IX+d)
            case 0x76: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x40); } break; // BIT 6,(c->IX+d)
            case 0x77: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x40); } break; // BIT 6,(c->IX+d)
            case 0x78: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x80); } break; // BIT 7,(c->IX+d)
            case 0x79: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x80); } break; // BIT 7,(c->IX+d)
            case 0x7a: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x80); } break; // BIT 7,(c->IX+d)
            case 0x7b: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x80); } break; // BIT 7,(c->IX+d)
            case 0x7c: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x80); } break; // BIT 7,(c->IX+d)
            case 0x7d: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x80); } break; // BIT 7,(c->IX+d)
            case 0x7e: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x80); } break; // BIT 7,(c->IX+d)
            case 0x7f: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x80); } break; // BIT 7,(c->IX+d)
            case 0x80: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->B=v&~0x1;_WR(a,c->B); } break; // RES 0,(c->IX+d),B
            case 0x81: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->C=v&~0x1;_WR(a,c->C); } break; // RES 0,(c->IX+d),C
            case 0x82: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->D=v&~0x1;_WR(a,c->D); } break; // RES 0,(c->IX+d),D
            case 0x83: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->E=v&~0x1;_WR(a,c->E); } break; // RES 0,(c->IX+d),E
            case 0x84: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->H=v&~0x1;_WR(a,c->H); } break; // RES 0,(c->IX+d),H
            case 0x85: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->L=v&~0x1;_WR(a,c->L); } break; // RES 0,(c->IX+d),L
            case 0x86: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_WR(a,v&~0x1); } break; // RES 0,(c->IX+d)
            case 0x87: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->A=v&~0x1;_WR(a,c->A); } break; // RES 0,(c->IX+d),A
            case 0x88: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->B=v&~0x2;_WR(a,c->B); } break; // RES 1,(c->IX+d),B
            case 0x89: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->C=v&~0x2;_WR(a,c->C); } break; // RES 1,(c->IX+d),C
            case 0x8a: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->D=v&~0x2;_WR(a,c->D); } break; // RES 1,(c->IX+d),D
            case 0x8b: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->E=v&~0x2;_WR(a,c->E); } break; // RES 1,(c->IX+d),E
            case 0x8c: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->H=v&~0x2;_WR(a,c->H); } break; // RES 1,(c->IX+d),H
            case 0x8d: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->L=v&~0x2;_WR(a,c->L); } break; // RES 1,(c->IX+d),L
            case 0x8e: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_WR(a,v&~0x2); } break; // RES 1,(c->IX+d)
            case 0x8f: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->A=v&~0x2;_WR(a,c->A); } break; // RES 1,(c->IX+d),A
            case 0x90: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->B=v&~0x4;_WR(a,c->B); } break; // RES 2,(c->IX+d),B
            case 0x91: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->C=v&~0x4;_WR(a,c->C); } break; // RES 2,(c->IX+d),C
            case 0x92: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->D=v&~0x4;_WR(a,c->D); } break; // RES 2,(c->IX+d),D
            case 0x93: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->E=v&~0x4;_WR(a,c->E); } break; // RES 2,(c->IX+d),E
            case 0x94: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->H=v&~0x4;_WR(a,c->H); } break; // RES 2,(c->IX+d),H
            case 0x95: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->L=v&~0x4;_WR(a,c->L); } break; // RES 2,(c->IX+d),L
            case 0x96: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_WR(a,v&~0x4); } break; // RES 2,(c->IX+d)
            case 0x97: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->A=v&~0x4;_WR(a,c->A); } break; // RES 2,(c->IX+d),A
            case 0x98: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->B=v&~0x8;_WR(a,c->B); } break; // RES 3,(c->IX+d),B
            case 0x99: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->C=v&~0x8;_WR(a,c->C); } break; // RES 3,(c->IX+d),C
            case 0x9a: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->D=v&~0x8;_WR(a,c->D); } break; // RES 3,(c->IX+d),D
            case 0x9b: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->E=v&~0x8;_WR(a,c->E); } break; // RES 3,(c->IX+d),E
            case 0x9c: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->H=v&~0x8;_WR(a,c->H); } break; // RES 3,(c->IX+d),H
            case 0x9d: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->L=v&~0x8;_WR(a,c->L); } break; // RES 3,(c->IX+d),L
            case 0x9e: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_WR(a,v&~0x8); } break; // RES 3,(c->IX+d)
            case 0x9f: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->A=v&~0x8;_WR(a,c->A); } break; // RES 3,(c->IX+d),A
            case 0xa0: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->B=v&~0x10;_WR(a,c->B); } break; // RES 4,(c->IX+d),B
            case 0xa1: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->C=v&~0x10;_WR(a,c->C); } break; // RES 4,(c->IX+d),C
            case 0xa2: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->D=v&~0x10;_WR(a,c->D); } break; // RES 4,(c->IX+d),D
            case 0xa3: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->E=v&~0x10;_WR(a,c->E); } break; // RES 4,(c->IX+d),E
            case 0xa4: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->H=v&~0x10;_WR(a,c->H); } break; // RES 4,(c->IX+d),H
            case 0xa5: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->L=v&~0x10;_WR(a,c->L); } break; // RES 4,(c->IX+d),L
            case 0xa6: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_WR(a,v&~0x10); } break; // RES 4,(c->IX+d)
            case 0xa7: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->A=v&~0x10;_WR(a,c->A); } break; // RES 4,(c->IX+d),A
            case 0xa8: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->B=v&~0x20;_WR(a,c->B); } break; // RES 5,(c->IX+d),B
            case 0xa9: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->C=v&~0x20;_WR(a,c->C); } break; // RES 5,(c->IX+d),C
            case 0xaa: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->D=v&~0x20;_WR(a,c->D); } break; // RES 5,(c->IX+d),D
            case 0xab: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->E=v&~0x20;_WR(a,c->E); } break; // RES 5,(c->IX+d),E
            case 0xac: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->H=v&~0x20;_WR(a,c->H); } break; // RES 5,(c->IX+d),H
            case 0xad: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->L=v&~0x20;_WR(a,c->L); } break; // RES 5,(c->IX+d),L
            case 0xae: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_WR(a,v&~0x20); } break; // RES 5,(c->IX+d)
            case 0xaf: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->A=v&~0x20;_WR(a,c->A); } break; // RES 5,(c->IX+d),A
            case 0xb0: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->B=v&~0x40;_WR(a,c->B); } break; // RES 6,(c->IX+d),B
            case 0xb1: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->C=v&~0x40;_WR(a,c->C); } break; // RES 6,(c->IX+d),C
            case 0xb2: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->D=v&~0x40;_WR(a,c->D); } break; // RES 6,(c->IX+d),D
            case 0xb3: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->E=v&~0x40;_WR(a,c->E); } break; // RES 6,(c->IX+d),E
            case 0xb4: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->H=v&~0x40;_WR(a,c->H); } break; // RES 6,(c->IX+d),H
            case 0xb5: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->L=v&~0x40;_WR(a,c->L); } break; // RES 6,(c->IX+d),L
            case 0xb6: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_WR(a,v&~0x40); } break; // RES 6,(c->IX+d)
            case 0xb7: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->A=v&~0x40;_WR(a,c->A); } break; // RES 6,(c->IX+d),A
            case 0xb8: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->B=v&~0x80;_WR(a,c->B); } break; // RES 7,(c->IX+d),B
            case 0xb9: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->C=v&~0x80;_WR(a,c->C); } break; // RES 7,(c->IX+d),C
            case 0xba: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->D=v&~0x80;_WR(a,c->D); } break; // RES 7,(c->IX+d),D
            case 0xbb: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->E=v&~0x80;_WR(a,c->E); } break; // RES 7,(c->IX+d),E
            case 0xbc: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->H=v&~0x80;_WR(a,c->H); } break; // RES 7,(c->IX+d),H
            case 0xbd: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->L=v&~0x80;_WR(a,c->L); } break; // RES 7,(c->IX+d),L
            case 0xbe: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_WR(a,v&~0x80); } break; // RES 7,(c->IX+d)
            case 0xbf: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->A=v&~0x80;_WR(a,c->A); } break; // RES 7,(c->IX+d),A
            case 0xc0: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->B=v|0x1;_WR(a,c->B);} break; // SET 0,(c->IX+d),B
            case 0xc1: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->C=v|0x1;_WR(a,c->C);} break; // SET 0,(c->IX+d),C
            case 0xc2: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->D=v|0x1;_WR(a,c->D);} break; // SET 0,(c->IX+d),D
            case 0xc3: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->E=v|0x1;_WR(a,c->E);} break; // SET 0,(c->IX+d),E
            case 0xc4: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->H=v|0x1;_WR(a,c->IXH);} break; // SET 0,(c->IX+d),H
            case 0xc5: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->L=v|0x1;_WR(a,c->IXL);} break; // SET 0,(c->IX+d),L
            case 0xc6: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_WR(a,v|0x1);} break; // SET 0,(c->IX+d)
            case 0xc7: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->A=v|0x1;_WR(a,c->A);} break; // SET 0,(c->IX+d),A
            case 0xc8: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->B=v|0x2;_WR(a,c->B);} break; // SET 1,(c->IX+d),B
            case 0xc9: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->C=v|0x2;_WR(a,c->C);} break; // SET 1,(c->IX+d),C
            case 0xca: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->D=v|0x2;_WR(a,c->D);} break; // SET 1,(c->IX+d),D
            case 0xcb: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->E=v|0x2;_WR(a,c->E);} break; // SET 1,(c->IX+d),E
            case 0xcc: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->H=v|0x2;_WR(a,c->IXH);} break; // SET 1,(c->IX+d),H
            case 0xcd: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->L=v|0x2;_WR(a,c->IXL);} break; // SET 1,(c->IX+d),L
            case 0xce: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_WR(a,v|0x2);} break; // SET 1,(c->IX+d)
            case 0xcf: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->A=v|0x2;_WR(a,c->A);} break; // SET 1,(c->IX+d),A
            case 0xd0: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->B=v|0x4;_WR(a,c->B);} break; // SET 2,(c->IX+d),B
            case 0xd1: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->C=v|0x4;_WR(a,c->C);} break; // SET 2,(c->IX+d),C
            case 0xd2: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->D=v|0x4;_WR(a,c->D);} break; // SET 2,(c->IX+d),D
            case 0xd3: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->E=v|0x4;_WR(a,c->E);} break; // SET 2,(c->IX+d),E
            case 0xd4: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->H=v|0x4;_WR(a,c->IXH);} break; // SET 2,(c->IX+d),H
            case 0xd5: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->L=v|0x4;_WR(a,c->IXL);} break; // SET 2,(c->IX+d),L
            case 0xd6: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_WR(a,v|0x4);} break; // SET 2,(c->IX+d)
            case 0xd7: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->A=v|0x4;_WR(a,c->A);} break; // SET 2,(c->IX+d),A
            case 0xd8: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->B=v|0x8;_WR(a,c->B);} break; // SET 3,(c->IX+d),B
            case 0xd9: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->C=v|0x8;_WR(a,c->C);} break; // SET 3,(c->IX+d),C
            case 0xda: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->D=v|0x8;_WR(a,c->D);} break; // SET 3,(c->IX+d),D
            case 0xdb: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->E=v|0x8;_WR(a,c->E);} break; // SET 3,(c->IX+d),E
            case 0xdc: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->H=v|0x8;_WR(a,c->IXH);} break; // SET 3,(c->IX+d),H
            case 0xdd: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->L=v|0x8;_WR(a,c->IXL);} break; // SET 3,(c->IX+d),L
            case 0xde: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_WR(a,v|0x8);} break; // SET 3,(c->IX+d)
            case 0xdf: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->A=v|0x8;_WR(a,c->A);} break; // SET 3,(c->IX+d),A
            case 0xe0: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->B=v|0x10;_WR(a,c->B);} break; // SET 4,(c->IX+d),B
            case 0xe1: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->C=v|0x10;_WR(a,c->C);} break; // SET 4,(c->IX+d),C
            case 0xe2: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->D=v|0x10;_WR(a,c->D);} break; // SET 4,(c->IX+d),D
            case 0xe3: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->E=v|0x10;_WR(a,c->E);} break; // SET 4,(c->IX+d),E
            case 0xe4: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->H=v|0x10;_WR(a,c->IXH);} break; // SET 4,(c->IX+d),H
            case 0xe5: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->L=v|0x10;_WR(a,c->IXL);} break; // SET 4,(c->IX+d),L
            case 0xe6: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_WR(a,v|0x10);} break; // SET 4,(c->IX+d)
            case 0xe7: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->A=v|0x10;_WR(a,c->A);} break; // SET 4,(c->IX+d),A
            case 0xe8: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->B=v|0x20;_WR(a,c->B);} break; // SET 5,(c->IX+d),B
            case 0xe9: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->C=v|0x20;_WR(a,c->C);} break; // SET 5,(c->IX+d),C
            case 0xea: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->D=v|0x20;_WR(a,c->D);} break; // SET 5,(c->IX+d),D
            case 0xeb: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->E=v|0x20;_WR(a,c->E);} break; // SET 5,(c->IX+d),E
            case 0xec: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->H=v|0x20;_WR(a,c->IXH);} break; // SET 5,(c->IX+d),H
            case 0xed: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->L=v|0x20;_WR(a,c->IXL);} break; // SET 5,(c->IX+d),L
            case 0xee: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_WR(a,v|0x20);} break; // SET 5,(c->IX+d)
            case 0xef: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->A=v|0x20;_WR(a,c->A);} break; // SET 5,(c->IX+d),A
            case 0xf0: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->B=v|0x40;_WR(a,c->B);} break; // SET 6,(c->IX+d),B
            case 0xf1: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->C=v|0x40;_WR(a,c->C);} break; // SET 6,(c->IX+d),C
            case 0xf2: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->D=v|0x40;_WR(a,c->D);} break; // SET 6,(c->IX+d),D
            case 0xf3: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->E=v|0x40;_WR(a,c->E);} break; // SET 6,(c->IX+d),E
            case 0xf4: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->H=v|0x40;_WR(a,c->IXH);} break; // SET 6,(c->IX+d),H
            case 0xf5: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->L=v|0x40;_WR(a,c->IXL);} break; // SET 6,(c->IX+d),L
            case 0xf6: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_WR(a,v|0x40);} break; // SET 6,(c->IX+d)
            case 0xf7: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->A=v|0x40;_WR(a,c->A);} break; // SET 6,(c->IX+d),A
            case 0xf8: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->B=v|0x80;_WR(a,c->B);} break; // SET 7,(c->IX+d),B
            case 0xf9: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->C=v|0x80;_WR(a,c->C);} break; // SET 7,(c->IX+d),C
            case 0xfa: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->D=v|0x80;_WR(a,c->D);} break; // SET 7,(c->IX+d),D
            case 0xfb: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->E=v|0x80;_WR(a,c->E);} break; // SET 7,(c->IX+d),E
            case 0xfc: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->H=v|0x80;_WR(a,c->IXH);} break; // SET 7,(c->IX+d),H
            case 0xfd: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->L=v|0x80;_WR(a,c->IXL);} break; // SET 7,(c->IX+d),L
            case 0xfe: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);_WR(a,v|0x80);} break; // SET 7,(c->IX+d)
            case 0xff: { uint16_t a=c->WZ=c->IX+d;;_T();_T();uint8_t v;_RD(a,v);c->A=v|0x80;_WR(a,c->A);} break; // SET 7,(c->IX+d),A
            default: break;
          }
          break;
          }
        case 0xcc: ticks=_z80_callcc(c,(c->F&Z80_ZF),tick,ticks); break; // CALL Z,nn
        case 0xcd: ticks=_z80_call(c,tick,ticks); break; // CALL nn
        case 0xce: {uint8_t v;_RD(c->PC++,v);_z80_adc(c,v);} break; // ADC n
        case 0xcf: ticks=_z80_rst(c,0x8,tick,ticks); break; // RST 0x8
        case 0xd0: ticks=_z80_retcc(c,!(c->F&Z80_CF),tick,ticks); break; // RET NC
        case 0xd1: { uint8_t l;_RD(c->SP++,l);uint8_t h;_RD(c->SP++,h);c->DE=(h<<8)|l; } break; // POP DE
        case 0xd2: _IMM16(); if (!(c->F&Z80_CF)) { c->PC=c->WZ; } break; // JP NC,nn
        case 0xd3: {uint8_t v;_RD(c->PC++,v);c->WZ=((c->A<<8)|v);_OUT(c->WZ,c->A);c->Z++;} break; // OUT (n),A
        case 0xd4: ticks=_z80_callcc(c,!(c->F&Z80_CF),tick,ticks); break; // CALL NC,nn
        case 0xd5: _T();_WR(--c->SP,(uint8_t)(c->DE>>8)); _WR(--c->SP,(uint8_t)c->DE); break; // PUSH DE
        case 0xd6: {uint8_t v;_RD(c->PC++,v);_z80_sub(c,v);} break; // SUB n
        case 0xd7: ticks=_z80_rst(c,0x10,tick,ticks); break; // RST 0x10
        case 0xd8: ticks=_z80_retcc(c,(c->F&Z80_CF),tick,ticks); break; // RET C
        case 0xd9: _SWP16(c->BC,c->BC_);_SWP16(c->DE,c->DE_);_SWP16(c->HL,c->HL_); break; // EXX
        case 0xda: _IMM16(); if ((c->F&Z80_CF)) { c->PC=c->WZ; } break; // JP C,nn
        case 0xdb: {uint8_t v;_RD(c->PC++,v);c->WZ=((c->A<<8)|v);_IN(c->WZ++,c->A);} break; // IN A,(n)
        case 0xdc: ticks=_z80_callcc(c,(c->F&Z80_CF),tick,ticks); break; // CALL C,nn
        case 0xde: {uint8_t v;_RD(c->PC++,v);_z80_sbc(c,v);} break; // SBC n
        case 0xdf: ticks=_z80_rst(c,0x18,tick,ticks); break; // RST 0x18
        case 0xe0: ticks=_z80_retcc(c,!(c->F&Z80_PF),tick,ticks); break; // RET PO
        case 0xe1: { uint8_t l;_RD(c->SP++,l);uint8_t h;_RD(c->SP++,h);c->IX=(h<<8)|l; } break; // POP IX
        case 0xe2: _IMM16(); if (!(c->F&Z80_PF)) { c->PC=c->WZ; } break; // JP PO,nn
        case 0xe3: _T();_RD(c->SP,c->Z);_RD(c->SP+1,c->W);_WR(c->SP,(uint8_t)c->IX);_WR(c->SP+1,(uint8_t)(c->IX>>8));c->IX=c->WZ;_T();_T(); break; // EX (SP),IX
        case 0xe4: ticks=_z80_callcc(c,!(c->F&Z80_PF),tick,ticks); break; // CALL PO,nn
        case 0xe5: _T();_WR(--c->SP,(uint8_t)(c->IX>>8)); _WR(--c->SP,(uint8_t)c->IX); break; // PUSH IX
        case 0xe6: {uint8_t v;_RD(c->PC++,v);_z80_and(c,v);} break; // AND n
        case 0xe7: ticks=_z80_rst(c,0x20,tick,ticks); break; // RST 0x20
        case 0xe8: ticks=_z80_retcc(c,(c->F&Z80_PF),tick,ticks); break; // RET PE
        case 0xe9: c->PC=c->IX; break; // JP IX
        case 0xea: _IMM16(); if ((c->F&Z80_PF)) { c->PC=c->WZ; } break; // JP PE,nn
        case 0xeb: _SWP16(c->DE,c->HL); break; // EX DE,HL
        case 0xec: ticks=_z80_callcc(c,(c->F&Z80_PF),tick,ticks); break; // CALL PE,nn
        case 0xee: {uint8_t v;_RD(c->PC++,v);_z80_xor(c,v);} break; // XOR n
        case 0xef: ticks=_z80_rst(c,0x28,tick,ticks); break; // RST 0x28
        case 0xf0: ticks=_z80_retcc(c,!(c->F&Z80_SF),tick,ticks); break; // RET P
        case 0xf1: { uint8_t l;_RD(c->SP++,l);uint8_t h;_RD(c->SP++,h);c->AF=(h<<8)|l; } break; // POP AF
        case 0xf2: _IMM16(); if (!(c->F&Z80_SF)) { c->PC=c->WZ; } break; // JP P,nn
        case 0xf3: _z80_di(c); break; // DI
        case 0xf4: ticks=_z80_callcc(c,!(c->F&Z80_SF),tick,ticks); break; // CALL P,nn
        case 0xf5: _T();_WR(--c->SP,(uint8_t)(c->AF>>8)); _WR(--c->SP,(uint8_t)c->AF); break; // PUSH AF
        case 0xf6: {uint8_t v;_RD(c->PC++,v);_z80_or(c,v);} break; // OR n
        case 0xf7: ticks=_z80_rst(c,0x30,tick,ticks); break; // RST 0x30
        case 0xf8: ticks=_z80_retcc(c,(c->F&Z80_SF),tick,ticks); break; // RET M
        case 0xf9: _T();_T();c->SP=c->IX; break; // LD SP,IX
        case 0xfa: _IMM16(); if ((c->F&Z80_SF)) { c->PC=c->WZ; } break; // JP M,nn
        case 0xfb: _z80_ei(c); break; // EI
        case 0xfc: ticks=_z80_callcc(c,(c->F&Z80_SF),tick,ticks); break; // CALL M,nn
        case 0xfe: {uint8_t v;_RD(c->PC++,v);_z80_cp(c,v);} break; // CP n
        case 0xff: ticks=_z80_rst(c,0x38,tick,ticks); break; // RST 0x38
        default: break;
      }
      break;
    case 0xde: {uint8_t v;_RD(c->PC++,v);_z80_sbc(c,v);} break; // SBC n
    case 0xdf: ticks=_z80_rst(c,0x18,tick,ticks); break; // RST 0x18
    case 0xe0: ticks=_z80_retcc(c,!(c->F&Z80_PF),tick,ticks); break; // RET PO
    case 0xe1: { uint8_t l;_RD(c->SP++,l);uint8_t h;_RD(c->SP++,h);c->HL=(h<<8)|l; } break; // POP HL
    case 0xe2: _IMM16(); if (!(c->F&Z80_PF)) { c->PC=c->WZ; } break; // JP PO,nn
    case 0xe3: _T();_RD(c->SP,c->Z);_RD(c->SP+1,c->W);_WR(c->SP,(uint8_t)c->HL);_WR(c->SP+1,(uint8_t)(c->HL>>8));c->HL=c->WZ;_T();_T(); break; // EX (SP),HL
    case 0xe4: ticks=_z80_callcc(c,!(c->F&Z80_PF),tick,ticks); break; // CALL PO,nn
    case 0xe5: _T();_WR(--c->SP,(uint8_t)(c->HL>>8)); _WR(--c->SP,(uint8_t)c->HL); break; // PUSH HL
    case 0xe6: {uint8_t v;_RD(c->PC++,v);_z80_and(c,v);} break; // AND n
    case 0xe7: ticks=_z80_rst(c,0x20,tick,ticks); break; // RST 0x20
    case 0xe8: ticks=_z80_retcc(c,(c->F&Z80_PF),tick,ticks); break; // RET PE
    case 0xe9: c->PC=c->HL; break; // JP HL
    case 0xea: _IMM16(); if ((c->F&Z80_PF)) { c->PC=c->WZ; } break; // JP PE,nn
    case 0xeb: _SWP16(c->DE,c->HL); break; // EX DE,HL
    case 0xec: ticks=_z80_callcc(c,(c->F&Z80_PF),tick,ticks); break; // CALL PE,nn
    case 0xed:
      _FETCH(); switch (opcode) {
        case 0x40: c->WZ=c->BC;_IN(c->WZ++,c->B);c->F=c->szp[c->B]|(c->F&Z80_CF); break; // IN B,(C)
        case 0x41: c->WZ=c->BC;_OUT(c->WZ++,c->B); break; // OUT (C),B
        case 0x42: c->HL=_z80_sbc16(c,c->HL,c->BC);_T();_T();_T();_T();_T();_T();_T(); break; // SBC HL,BC
        case 0x43: _IMM16();_WR(c->WZ++,(uint8_t)c->BC);_WR(c->WZ,(uint8_t)(c->BC>>8)); break; // LD (nn),BC
        case 0x44: _z80_neg(c); break; // NEG
        case 0x46: c->IM=0; break; // IM 0
        case 0x47: _T(); c->I=c->A; break; // LD I,A
        case 0x48: c->WZ=c->BC;_IN(c->WZ++,c->C);c->F=c->szp[c->C]|(c->F&Z80_CF); break; // IN C,(C)
        case 0x49: c->WZ=c->BC;_OUT(c->WZ++,c->C); break; // OUT (C),C
        case 0x4a: c->HL=_z80_adc16(c,c->HL,c->BC);_T();_T();_T();_T();_T();_T();_T(); break; // ADC HL,BC
        case 0x4b: {_IMM16();uint8_t l;_RD(c->WZ++,l);uint8_t h;_RD(c->WZ,h);c->BC=(h<<8)|l;} break; // LD BC,(nn)
        case 0x4c: _z80_neg(c); break; // NEG
        case 0x4d: _z80_reti(c); break; // RETI
        case 0x4e: c->IM=0; break; // IM 0
        case 0x4f: _T(); c->R=c->A; break; // LD R,A
        case 0x50: c->WZ=c->BC;_IN(c->WZ++,c->D);c->F=c->szp[c->D]|(c->F&Z80_CF); break; // IN D,(C)
        case 0x51: c->WZ=c->BC;_OUT(c->WZ++,c->D); break; // OUT (C),D
        case 0x52: c->HL=_z80_sbc16(c,c->HL,c->DE);_T();_T();_T();_T();_T();_T();_T(); break; // SBC HL,DE
        case 0x53: _IMM16();_WR(c->WZ++,(uint8_t)c->DE);_WR(c->WZ,(uint8_t)(c->DE>>8)); break; // LD (nn),DE
        case 0x54: _z80_neg(c); break; // NEG
        case 0x56: c->IM=1; break; // IM 1
        case 0x57: _T(); c->A=c->I; c->F=_z80_sziff2(c,c->I)|(c->F&Z80_CF); break; // LD A,I
        case 0x58: c->WZ=c->BC;_IN(c->WZ++,c->E);c->F=c->szp[c->E]|(c->F&Z80_CF); break; // IN E,(C)
        case 0x59: c->WZ=c->BC;_OUT(c->WZ++,c->E); break; // OUT (C),E
        case 0x5a: c->HL=_z80_adc16(c,c->HL,c->DE);_T();_T();_T();_T();_T();_T();_T(); break; // ADC HL,DE
        case 0x5b: {_IMM16();uint8_t l;_RD(c->WZ++,l);uint8_t h;_RD(c->WZ,h);c->DE=(h<<8)|l;} break; // LD DE,(nn)
        case 0x5c: _z80_neg(c); break; // NEG
        case 0x5e: c->IM=2; break; // IM 2
        case 0x5f: _T(); c->A=c->R; c->F=_z80_sziff2(c,c->R)|(c->F&Z80_CF); break; // LD A,R
        case 0x60: c->WZ=c->BC;_IN(c->WZ++,c->H);c->F=c->szp[c->H]|(c->F&Z80_CF); break; // IN H,(C)
        case 0x61: c->WZ=c->BC;_OUT(c->WZ++,c->H); break; // OUT (C),H
        case 0x62: c->HL=_z80_sbc16(c,c->HL,c->HL);_T();_T();_T();_T();_T();_T();_T(); break; // SBC HL,HL
        case 0x63: _IMM16();_WR(c->WZ++,(uint8_t)c->HL);_WR(c->WZ,(uint8_t)(c->HL>>8)); break; // LD (nn),HL
        case 0x64: _z80_neg(c); break; // NEG
        case 0x66: c->IM=0; break; // IM 0
        case 0x67: ticks=_z80_rrd(c,tick,ticks); break; // RRD
        case 0x68: c->WZ=c->BC;_IN(c->WZ++,c->L);c->F=c->szp[c->L]|(c->F&Z80_CF); break; // IN L,(C)
        case 0x69: c->WZ=c->BC;_OUT(c->WZ++,c->L); break; // OUT (C),L
        case 0x6a: c->HL=_z80_adc16(c,c->HL,c->HL);_T();_T();_T();_T();_T();_T();_T(); break; // ADC HL,HL
        case 0x6b: {_IMM16();uint8_t l;_RD(c->WZ++,l);uint8_t h;_RD(c->WZ,h);c->HL=(h<<8)|l;} break; // LD HL,(nn)
        case 0x6c: _z80_neg(c); break; // NEG
        case 0x6e: c->IM=0; break; // IM 0
        case 0x6f: ticks=_z80_rld(c,tick,ticks); break; // RLD
        case 0x70: c->WZ=c->BC;uint8_t v; _IN(c->WZ++,v);c->F=c->szp[v]|(c->F&Z80_CF); break; // IN (C)
        case 0x71: c->WZ=c->BC;_OUT(c->WZ++,0); break; // None
        case 0x72: c->HL=_z80_sbc16(c,c->HL,c->SP);_T();_T();_T();_T();_T();_T();_T(); break; // SBC HL,SP
        case 0x73: _IMM16();_WR(c->WZ++,(uint8_t)c->SP);_WR(c->WZ,(uint8_t)(c->SP>>8)); break; // LD (nn),SP
        case 0x74: _z80_neg(c); break; // NEG
        case 0x76: c->IM=1; break; // IM 1
        case 0x77:   break; // NOP (ED)
        case 0x78: c->WZ=c->BC;_IN(c->WZ++,c->A);c->F=c->szp[c->A]|(c->F&Z80_CF); break; // IN A,(C)
        case 0x79: c->WZ=c->BC;_OUT(c->WZ++,c->A); break; // OUT (C),A
        case 0x7a: c->HL=_z80_adc16(c,c->HL,c->SP);_T();_T();_T();_T();_T();_T();_T(); break; // ADC HL,SP
        case 0x7b: {_IMM16();uint8_t l;_RD(c->WZ++,l);uint8_t h;_RD(c->WZ,h);c->SP=(h<<8)|l;} break; // LD SP,(nn)
        case 0x7c: _z80_neg(c); break; // NEG
        case 0x7e: c->IM=2; break; // IM 2
        case 0x7f:   break; // NOP (ED)
        case 0xa0: ticks=_z80_ldi(c,tick,ticks); break; // LDI
        case 0xa1: ticks=_z80_cpi(c,tick,ticks); break; // CPI
        case 0xa2: ticks=_z80_ini(c,tick,ticks); break; // INI
        case 0xa3: ticks=_z80_outi(c,tick,ticks); break; // OUTI
        case 0xa8: ticks=_z80_ldd(c,tick,ticks); break; // LDD
        case 0xa9: ticks=_z80_cpd(c,tick,ticks); break; // CPD
        case 0xaa: ticks=_z80_ind(c,tick,ticks); break; // IND
        case 0xab: ticks=_z80_outd(c,tick,ticks); break; // OUTD
        case 0xb0: ticks=_z80_ldir(c,tick,ticks); break; // LDIR
        case 0xb1: ticks=_z80_cpir(c,tick,ticks); break; // CPIR
        case 0xb2: ticks=_z80_inir(c,tick,ticks); break; // INIR
        case 0xb3: ticks=_z80_otir(c,tick,ticks); break; // OTIR
        case 0xb8: ticks=_z80_lddr(c,tick,ticks); break; // LDDR
        case 0xb9: ticks=_z80_cpdr(c,tick,ticks); break; // CPDR
        case 0xba: ticks=_z80_indr(c,tick,ticks); break; // INDR
        case 0xbb: ticks=_z80_otdr(c,tick,ticks); break; // OTDR
        default: break;
      }
      break;
    case 0xee: {uint8_t v;_RD(c->PC++,v);_z80_xor(c,v);} break; // XOR n
    case 0xef: ticks=_z80_rst(c,0x28,tick,ticks); break; // RST 0x28
    case 0xf0: ticks=_z80_retcc(c,!(c->F&Z80_SF),tick,ticks); break; // RET P
    case 0xf1: { uint8_t l;_RD(c->SP++,l);uint8_t h;_RD(c->SP++,h);c->AF=(h<<8)|l; } break; // POP AF
    case 0xf2: _IMM16(); if (!(c->F&Z80_SF)) { c->PC=c->WZ; } break; // JP P,nn
    case 0xf3: _z80_di(c); break; // DI
    case 0xf4: ticks=_z80_callcc(c,!(c->F&Z80_SF),tick,ticks); break; // CALL P,nn
    case 0xf5: _T();_WR(--c->SP,(uint8_t)(c->AF>>8)); _WR(--c->SP,(uint8_t)c->AF); break; // PUSH AF
    case 0xf6: {uint8_t v;_RD(c->PC++,v);_z80_or(c,v);} break; // OR n
    case 0xf7: ticks=_z80_rst(c,0x30,tick,ticks); break; // RST 0x30
    case 0xf8: ticks=_z80_retcc(c,(c->F&Z80_SF),tick,ticks); break; // RET M
    case 0xf9: _T();_T();c->SP=c->HL; break; // LD SP,HL
    case 0xfa: _IMM16(); if ((c->F&Z80_SF)) { c->PC=c->WZ; } break; // JP M,nn
    case 0xfb: _z80_ei(c); break; // EI
    case 0xfc: ticks=_z80_callcc(c,(c->F&Z80_SF),tick,ticks); break; // CALL M,nn
    case 0xfd:
      _FETCH(); switch (opcode) {
        case 0x0:   break; // NOP
        case 0x1: _IMM16(); c->BC=c->WZ; break; // LD BC,nn
        case 0x2: c->WZ=c->BC;_WR(c->WZ++,c->A);c->W=c->A; break; // LD (BC),A
        case 0x3: _T();_T();c->BC++; break; // INC BC
        case 0x4: c->B=_z80_inc(c,c->B); break; // INC B
        case 0x5: c->B=_z80_dec(c,c->B); break; // DEC B
        case 0x6: _RD(c->PC++,c->B); break; // LD B,n
        case 0x7: _z80_rlca(c); break; // RLCA
        case 0x8: _SWP16(c->AF,c->AF_); break; // EX AF,AF'
        case 0x9: c->IY=_z80_add16(c,c->IY,c->BC);_T();_T();_T();_T();_T();_T();_T(); break; // ADD IY,BC
        case 0xa: c->WZ=c->BC;_RD(c->WZ++,c->A); break; // LD A,(BC)
        case 0xb: _T();_T();c->BC--; break; // DEC BC
        case 0xc: c->C=_z80_inc(c,c->C); break; // INC C
        case 0xd: c->C=_z80_dec(c,c->C); break; // DEC C
        case 0xe: _RD(c->PC++,c->C); break; // LD C,n
        case 0xf: _z80_rrca(c); break; // RRCA
        case 0x10: ticks=_z80_djnz(c,tick,ticks); break; // DJNZ
        case 0x11: _IMM16(); c->DE=c->WZ; break; // LD DE,nn
        case 0x12: c->WZ=c->DE;_WR(c->WZ++,c->A);c->W=c->A; break; // LD (DE),A
        case 0x13: _T();_T();c->DE++; break; // INC DE
        case 0x14: c->D=_z80_inc(c,c->D); break; // INC D
        case 0x15: c->D=_z80_dec(c,c->D); break; // DEC D
        case 0x16: _RD(c->PC++,c->D); break; // LD D,n
        case 0x17: _z80_rla(c); break; // RLA
        case 0x18: ticks=_z80_jr(c,tick,ticks); break; // JR d
        case 0x19: c->IY=_z80_add16(c,c->IY,c->DE);_T();_T();_T();_T();_T();_T();_T(); break; // ADD IY,DE
        case 0x1a: c->WZ=c->DE;_RD(c->WZ++,c->A); break; // LD A,(DE)
        case 0x1b: _T();_T();c->DE--; break; // DEC DE
        case 0x1c: c->E=_z80_inc(c,c->E); break; // INC E
        case 0x1d: c->E=_z80_dec(c,c->E); break; // DEC E
        case 0x1e: _RD(c->PC++,c->E); break; // LD E,n
        case 0x1f: _z80_rra(c); break; // RRA
        case 0x20: ticks=_z80_jr_cc(c,!(c->F&Z80_ZF),tick,ticks); break; // JR NZ,d
        case 0x21: _IMM16(); c->IY=c->WZ; break; // LD IY,nn
        case 0x22: {_IMM16();_WR(c->WZ++,(uint8_t)c->IY);_WR(c->WZ,(uint8_t)(c->IY>>8));} break; // LD (nn),IY
        case 0x23: _T();_T();c->IY++; break; // INC IY
        case 0x24: c->IYH=_z80_inc(c,c->IYH); break; // INC IYH
        case 0x25: c->IYH=_z80_dec(c,c->IYH); break; // DEC IYH
        case 0x26: _RD(c->PC++,c->IYH); break; // LD IYH,n
        case 0x27: _z80_daa(c); break; // DAA
        case 0x28: ticks=_z80_jr_cc(c,(c->F&Z80_ZF),tick,ticks); break; // JR Z,d
        case 0x29: c->IY=_z80_add16(c,c->IY,c->IY);_T();_T();_T();_T();_T();_T();_T(); break; // ADD IY,IY
        case 0x2a: {_IMM16();uint8_t l;_RD(c->WZ++,l);uint8_t h;_RD(c->WZ,h);c->IY=(h<<8)|l;} break; // LD IY,(nn)
        case 0x2b: _T();_T();c->IY--; break; // DEC IY
        case 0x2c: c->IYL=_z80_inc(c,c->IYL); break; // INC IYL
        case 0x2d: c->IYL=_z80_dec(c,c->IYL); break; // DEC IYL
        case 0x2e: _RD(c->PC++,c->IYL); break; // LD IYL,n
        case 0x2f: _z80_cpl(c); break; // CPL
        case 0x30: ticks=_z80_jr_cc(c,!(c->F&Z80_CF),tick,ticks); break; // JR NC,d
        case 0x31: _IMM16(); c->SP=c->WZ; break; // LD SP,nn
        case 0x32: _IMM16();_WR(c->WZ++,c->A);c->W=c->A; break; // LD (nn),A
        case 0x33: _T();_T();c->SP++; break; // INC SP
        case 0x34: { int8_t d;_RD(c->PC++,d);uint16_t a=c->WZ=c->IY+d;;_T();_T();_T();_T();_T();_T();uint8_t v;_RD(a,v);_WR(a,_z80_inc(c,v)); } break; // INC (c->IY+d)
        case 0x35: { int8_t d;_RD(c->PC++,d);uint16_t a=c->WZ=c->IY+d;;_T();_T();_T();_T();_T();_T();uint8_t v;_RD(a,v);_WR(a,_z80_dec(c,v)); } break; // DEC (c->IY+d)
        case 0x36: { int8_t d;_RD(c->PC++,d);uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(c->PC++,v);_WR(a,v); } break; // LD (c->IY+d),n
        case 0x37: _z80_scf(c); break; // SCF
        case 0x38: ticks=_z80_jr_cc(c,(c->F&Z80_CF),tick,ticks); break; // JR C,d
        case 0x39: c->IY=_z80_add16(c,c->IY,c->SP);_T();_T();_T();_T();_T();_T();_T(); break; // ADD IY,SP
        case 0x3a: _IMM16();_RD(c->WZ++,c->A); break; // LD A,(nn)
        case 0x3b: _T();_T();c->SP--; break; // DEC SP
        case 0x3c: c->A=_z80_inc(c,c->A); break; // INC A
        case 0x3d: c->A=_z80_dec(c,c->A); break; // DEC A
        case 0x3e: _RD(c->PC++,c->A); break; // LD A,n
        case 0x3f: _z80_ccf(c); break; // CCF
        case 0x40: c->B=c->B; break; // LD B,B
        case 0x41: c->B=c->C; break; // LD B,C
        case 0x42: c->B=c->D; break; // LD B,D
        case 0x43: c->B=c->E; break; // LD B,E
        case 0x44: c->B=c->IYH; break; // LD B,IYH
        case 0x45: c->B=c->IYL; break; // LD B,IYL
        case 0x46: { int8_t d;_RD(c->PC++,d);uint16_t a=c->WZ=c->IY+d;;_T();_T();_T();_T();_T();_RD(a,c->B); }; break; // LD B,(c->IY+d)
        case 0x47: c->B=c->A; break; // LD B,A
        case 0x48: c->C=c->B; break; // LD C,B
        case 0x49: c->C=c->C; break; // LD C,C
        case 0x4a: c->C=c->D; break; // LD C,D
        case 0x4b: c->C=c->E; break; // LD C,E
        case 0x4c: c->C=c->IYH; break; // LD C,IYH
        case 0x4d: c->C=c->IYL; break; // LD C,IYL
        case 0x4e: { int8_t d;_RD(c->PC++,d);uint16_t a=c->WZ=c->IY+d;;_T();_T();_T();_T();_T();_RD(a,c->C); }; break; // LD C,(c->IY+d)
        case 0x4f: c->C=c->A; break; // LD C,A
        case 0x50: c->D=c->B; break; // LD D,B
        case 0x51: c->D=c->C; break; // LD D,C
        case 0x52: c->D=c->D; break; // LD D,D
        case 0x53: c->D=c->E; break; // LD D,E
        case 0x54: c->D=c->IYH; break; // LD D,IYH
        case 0x55: c->D=c->IYL; break; // LD D,IYL
        case 0x56: { int8_t d;_RD(c->PC++,d);uint16_t a=c->WZ=c->IY+d;;_T();_T();_T();_T();_T();_RD(a,c->D); }; break; // LD D,(c->IY+d)
        case 0x57: c->D=c->A; break; // LD D,A
        case 0x58: c->E=c->B; break; // LD E,B
        case 0x59: c->E=c->C; break; // LD E,C
        case 0x5a: c->E=c->D; break; // LD E,D
        case 0x5b: c->E=c->E; break; // LD E,E
        case 0x5c: c->E=c->IYH; break; // LD E,IYH
        case 0x5d: c->E=c->IYL; break; // LD E,IYL
        case 0x5e: { int8_t d;_RD(c->PC++,d);uint16_t a=c->WZ=c->IY+d;;_T();_T();_T();_T();_T();_RD(a,c->E); }; break; // LD E,(c->IY+d)
        case 0x5f: c->E=c->A; break; // LD E,A
        case 0x60: c->IYH=c->B; break; // LD IYH,B
        case 0x61: c->IYH=c->C; break; // LD IYH,C
        case 0x62: c->IYH=c->D; break; // LD IYH,D
        case 0x63: c->IYH=c->E; break; // LD IYH,E
        case 0x64: c->IYH=c->IYH; break; // LD IYH,IYH
        case 0x65: c->IYH=c->IYL; break; // LD IYH,IYL
        case 0x66: { int8_t d;_RD(c->PC++,d);uint16_t a=c->WZ=c->IY+d;;_T();_T();_T();_T();_T();_RD(a,c->H); }; break; // LD H,(c->IY+d)
        case 0x67: c->IYH=c->A; break; // LD IYH,A
        case 0x68: c->IYL=c->B; break; // LD IYL,B
        case 0x69: c->IYL=c->C; break; // LD IYL,C
        case 0x6a: c->IYL=c->D; break; // LD IYL,D
        case 0x6b: c->IYL=c->E; break; // LD IYL,E
        case 0x6c: c->IYL=c->IYH; break; // LD IYL,IYH
        case 0x6d: c->IYL=c->IYL; break; // LD IYL,IYL
        case 0x6e: { int8_t d;_RD(c->PC++,d);uint16_t a=c->WZ=c->IY+d;;_T();_T();_T();_T();_T();_RD(a,c->L); }; break; // LD L,(c->IY+d)
        case 0x6f: c->IYL=c->A; break; // LD IYL,A
        case 0x70: { int8_t d;_RD(c->PC++,d);uint16_t a=c->WZ=c->IY+d;;_T();_T();_T();_T();_T();_WR(a,c->B); }; break; // LD (c->IY+d),B
        case 0x71: { int8_t d;_RD(c->PC++,d);uint16_t a=c->WZ=c->IY+d;;_T();_T();_T();_T();_T();_WR(a,c->C); }; break; // LD (c->IY+d),C
        case 0x72: { int8_t d;_RD(c->PC++,d);uint16_t a=c->WZ=c->IY+d;;_T();_T();_T();_T();_T();_WR(a,c->D); }; break; // LD (c->IY+d),D
        case 0x73: { int8_t d;_RD(c->PC++,d);uint16_t a=c->WZ=c->IY+d;;_T();_T();_T();_T();_T();_WR(a,c->E); }; break; // LD (c->IY+d),E
        case 0x74: { int8_t d;_RD(c->PC++,d);uint16_t a=c->WZ=c->IY+d;;_T();_T();_T();_T();_T();_WR(a,c->H); }; break; // LD (c->IY+d),H
        case 0x75: { int8_t d;_RD(c->PC++,d);uint16_t a=c->WZ=c->IY+d;;_T();_T();_T();_T();_T();_WR(a,c->L); }; break; // LD (c->IY+d),L
        case 0x76: _z80_halt(c); break; // HALT
        case 0x77: { int8_t d;_RD(c->PC++,d);uint16_t a=c->WZ=c->IY+d;;_T();_T();_T();_T();_T();_WR(a,c->A); }; break; // LD (c->IY+d),A
        case 0x78: c->A=c->B; break; // LD A,B
        case 0x79: c->A=c->C; break; // LD A,C
        case 0x7a: c->A=c->D; break; // LD A,D
        case 0x7b: c->A=c->E; break; // LD A,E
        case 0x7c: c->A=c->IYH; break; // LD A,IYH
        case 0x7d: c->A=c->IYL; break; // LD A,IYL
        case 0x7e: { int8_t d;_RD(c->PC++,d);uint16_t a=c->WZ=c->IY+d;;_T();_T();_T();_T();_T();_RD(a,c->A); }; break; // LD A,(c->IY+d)
        case 0x7f: c->A=c->A; break; // LD A,A
        case 0x80: _z80_add(c,c->B); break; // ADD B
        case 0x81: _z80_add(c,c->C); break; // ADD C
        case 0x82: _z80_add(c,c->D); break; // ADD D
        case 0x83: _z80_add(c,c->E); break; // ADD E
        case 0x84: _z80_add(c,c->IYH); break; // ADD IYH
        case 0x85: _z80_add(c,c->IYL); break; // ADD IYL
        case 0x86: { int8_t d;_RD(c->PC++,d);uint16_t a=c->WZ=c->IY+d;;_T();_T();_T();_T();_T();uint8_t val;_RD(a,val);_z80_add(c,val); }; break; // ADD (c->IY+d)
        case 0x87: _z80_add(c,c->A); break; // ADD A
        case 0x88: _z80_adc(c,c->B); break; // ADC B
        case 0x89: _z80_adc(c,c->C); break; // ADC C
        case 0x8a: _z80_adc(c,c->D); break; // ADC D
        case 0x8b: _z80_adc(c,c->E); break; // ADC E
        case 0x8c: _z80_adc(c,c->IYH); break; // ADC IYH
        case 0x8d: _z80_adc(c,c->IYL); break; // ADC IYL
        case 0x8e: { int8_t d;_RD(c->PC++,d);uint16_t a=c->WZ=c->IY+d;;_T();_T();_T();_T();_T();uint8_t val;_RD(a,val);_z80_adc(c,val); }; break; // ADC (c->IY+d)
        case 0x8f: _z80_adc(c,c->A); break; // ADC A
        case 0x90: _z80_sub(c,c->B); break; // SUB B
        case 0x91: _z80_sub(c,c->C); break; // SUB C
        case 0x92: _z80_sub(c,c->D); break; // SUB D
        case 0x93: _z80_sub(c,c->E); break; // SUB E
        case 0x94: _z80_sub(c,c->IYH); break; // SUB IYH
        case 0x95: _z80_sub(c,c->IYL); break; // SUB IYL
        case 0x96: { int8_t d;_RD(c->PC++,d);uint16_t a=c->WZ=c->IY+d;;_T();_T();_T();_T();_T();uint8_t val;_RD(a,val);_z80_sub(c,val); }; break; // SUB (c->IY+d)
        case 0x97: _z80_sub(c,c->A); break; // SUB A
        case 0x98: _z80_sbc(c,c->B); break; // SBC B
        case 0x99: _z80_sbc(c,c->C); break; // SBC C
        case 0x9a: _z80_sbc(c,c->D); break; // SBC D
        case 0x9b: _z80_sbc(c,c->E); break; // SBC E
        case 0x9c: _z80_sbc(c,c->IYH); break; // SBC IYH
        case 0x9d: _z80_sbc(c,c->IYL); break; // SBC IYL
        case 0x9e: { int8_t d;_RD(c->PC++,d);uint16_t a=c->WZ=c->IY+d;;_T();_T();_T();_T();_T();uint8_t val;_RD(a,val);_z80_sbc(c,val); }; break; // SBC (c->IY+d)
        case 0x9f: _z80_sbc(c,c->A); break; // SBC A
        case 0xa0: _z80_and(c,c->B); break; // AND B
        case 0xa1: _z80_and(c,c->C); break; // AND C
        case 0xa2: _z80_and(c,c->D); break; // AND D
        case 0xa3: _z80_and(c,c->E); break; // AND E
        case 0xa4: _z80_and(c,c->IYH); break; // AND IYH
        case 0xa5: _z80_and(c,c->IYL); break; // AND IYL
        case 0xa6: { int8_t d;_RD(c->PC++,d);uint16_t a=c->WZ=c->IY+d;;_T();_T();_T();_T();_T();uint8_t val;_RD(a,val);_z80_and(c,val); }; break; // AND (c->IY+d)
        case 0xa7: _z80_and(c,c->A); break; // AND A
        case 0xa8: _z80_xor(c,c->B); break; // XOR B
        case 0xa9: _z80_xor(c,c->C); break; // XOR C
        case 0xaa: _z80_xor(c,c->D); break; // XOR D
        case 0xab: _z80_xor(c,c->E); break; // XOR E
        case 0xac: _z80_xor(c,c->IYH); break; // XOR IYH
        case 0xad: _z80_xor(c,c->IYL); break; // XOR IYL
        case 0xae: { int8_t d;_RD(c->PC++,d);uint16_t a=c->WZ=c->IY+d;;_T();_T();_T();_T();_T();uint8_t val;_RD(a,val);_z80_xor(c,val); }; break; // XOR (c->IY+d)
        case 0xaf: _z80_xor(c,c->A); break; // XOR A
        case 0xb0: _z80_or(c,c->B); break; // OR B
        case 0xb1: _z80_or(c,c->C); break; // OR C
        case 0xb2: _z80_or(c,c->D); break; // OR D
        case 0xb3: _z80_or(c,c->E); break; // OR E
        case 0xb4: _z80_or(c,c->IYH); break; // OR IYH
        case 0xb5: _z80_or(c,c->IYL); break; // OR IYL
        case 0xb6: { int8_t d;_RD(c->PC++,d);uint16_t a=c->WZ=c->IY+d;;_T();_T();_T();_T();_T();uint8_t val;_RD(a,val);_z80_or(c,val); }; break; // OR (c->IY+d)
        case 0xb7: _z80_or(c,c->A); break; // OR A
        case 0xb8: _z80_cp(c,c->B); break; // CP B
        case 0xb9: _z80_cp(c,c->C); break; // CP C
        case 0xba: _z80_cp(c,c->D); break; // CP D
        case 0xbb: _z80_cp(c,c->E); break; // CP E
        case 0xbc: _z80_cp(c,c->IYH); break; // CP IYH
        case 0xbd: _z80_cp(c,c->IYL); break; // CP IYL
        case 0xbe: { int8_t d;_RD(c->PC++,d);uint16_t a=c->WZ=c->IY+d;;_T();_T();_T();_T();_T();uint8_t val;_RD(a,val);_z80_cp(c,val); }; break; // CP (c->IY+d)
        case 0xbf: _z80_cp(c,c->A); break; // CP A
        case 0xc0: ticks=_z80_retcc(c,!(c->F&Z80_ZF),tick,ticks); break; // RET NZ
        case 0xc1: { uint8_t l;_RD(c->SP++,l);uint8_t h;_RD(c->SP++,h);c->BC=(h<<8)|l; } break; // POP BC
        case 0xc2: _IMM16(); if (!(c->F&Z80_ZF)) { c->PC=c->WZ; } break; // JP NZ,nn
        case 0xc3: _IMM16(); c->PC=c->WZ; break; // JP nn
        case 0xc4: ticks=_z80_callcc(c,!(c->F&Z80_ZF),tick,ticks); break; // CALL NZ,nn
        case 0xc5: _T();_WR(--c->SP,(uint8_t)(c->BC>>8)); _WR(--c->SP,(uint8_t)c->BC); break; // PUSH BC
        case 0xc6: {uint8_t v;_RD(c->PC++,v);_z80_add(c,v);} break; // ADD n
        case 0xc7: ticks=_z80_rst(c,0x0,tick,ticks); break; // RST 0x0
        case 0xc8: ticks=_z80_retcc(c,(c->F&Z80_ZF),tick,ticks); break; // RET Z
        case 0xc9: ticks=_z80_ret(c,tick,ticks); break; // RET
        case 0xca: _IMM16(); if ((c->F&Z80_ZF)) { c->PC=c->WZ; } break; // JP Z,nn
        case 0xcb:
          { int8_t d; _RD(c->PC++, d);
          _FETCH(); switch (opcode) {
            case 0x0: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->B=_z80_rlc(c,v);_WR(a,c->B); } break; // RLC (c->IY+d),B
            case 0x1: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->C=_z80_rlc(c,v);_WR(a,c->C); } break; // RLC (c->IY+d),C
            case 0x2: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->D=_z80_rlc(c,v);_WR(a,c->D); } break; // RLC (c->IY+d),D
            case 0x3: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->E=_z80_rlc(c,v);_WR(a,c->E); } break; // RLC (c->IY+d),E
            case 0x4: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->H=_z80_rlc(c,v);_WR(a,c->H); } break; // RLC (c->IY+d),H
            case 0x5: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->L=_z80_rlc(c,v);_WR(a,c->L); } break; // RLC (c->IY+d),L
            case 0x6: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_WR(a,_z80_rlc(c,v)); } break; // RLC (c->IY+d)
            case 0x7: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->A=_z80_rlc(c,v);_WR(a,c->A); } break; // RLC (c->IY+d),A
            case 0x8: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->B=_z80_rrc(c,v);_WR(a,c->B); } break; // RRC (c->IY+d),B
            case 0x9: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->C=_z80_rrc(c,v);_WR(a,c->C); } break; // RRC (c->IY+d),C
            case 0xa: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->D=_z80_rrc(c,v);_WR(a,c->D); } break; // RRC (c->IY+d),D
            case 0xb: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->E=_z80_rrc(c,v);_WR(a,c->E); } break; // RRC (c->IY+d),E
            case 0xc: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->H=_z80_rrc(c,v);_WR(a,c->H); } break; // RRC (c->IY+d),H
            case 0xd: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->L=_z80_rrc(c,v);_WR(a,c->L); } break; // RRC (c->IY+d),L
            case 0xe: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_WR(a,_z80_rrc(c,v)); } break; // RRC (c->IY+d)
            case 0xf: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->A=_z80_rrc(c,v);_WR(a,c->A); } break; // RRC (c->IY+d),A
            case 0x10: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->B=_z80_rl(c,v);_WR(a,c->B); } break; // RL (c->IY+d),B
            case 0x11: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->C=_z80_rl(c,v);_WR(a,c->C); } break; // RL (c->IY+d),C
            case 0x12: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->D=_z80_rl(c,v);_WR(a,c->D); } break; // RL (c->IY+d),D
            case 0x13: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->E=_z80_rl(c,v);_WR(a,c->E); } break; // RL (c->IY+d),E
            case 0x14: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->H=_z80_rl(c,v);_WR(a,c->H); } break; // RL (c->IY+d),H
            case 0x15: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->L=_z80_rl(c,v);_WR(a,c->L); } break; // RL (c->IY+d),L
            case 0x16: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_WR(a,_z80_rl(c,v)); } break; // RL (c->IY+d)
            case 0x17: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->A=_z80_rl(c,v);_WR(a,c->A); } break; // RL (c->IY+d),A
            case 0x18: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->B=_z80_rr(c,v);_WR(a,c->B); } break; // RR (c->IY+d),B
            case 0x19: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->C=_z80_rr(c,v);_WR(a,c->C); } break; // RR (c->IY+d),C
            case 0x1a: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->D=_z80_rr(c,v);_WR(a,c->D); } break; // RR (c->IY+d),D
            case 0x1b: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->E=_z80_rr(c,v);_WR(a,c->E); } break; // RR (c->IY+d),E
            case 0x1c: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->H=_z80_rr(c,v);_WR(a,c->H); } break; // RR (c->IY+d),H
            case 0x1d: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->L=_z80_rr(c,v);_WR(a,c->L); } break; // RR (c->IY+d),L
            case 0x1e: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_WR(a,_z80_rr(c,v)); } break; // RR (c->IY+d)
            case 0x1f: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->A=_z80_rr(c,v);_WR(a,c->A); } break; // RR (c->IY+d),A
            case 0x20: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->B=_z80_sla(c,v);_WR(a,c->B); } break; // SLA (c->IY+d),B
            case 0x21: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->C=_z80_sla(c,v);_WR(a,c->C); } break; // SLA (c->IY+d),C
            case 0x22: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->D=_z80_sla(c,v);_WR(a,c->D); } break; // SLA (c->IY+d),D
            case 0x23: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->E=_z80_sla(c,v);_WR(a,c->E); } break; // SLA (c->IY+d),E
            case 0x24: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->H=_z80_sla(c,v);_WR(a,c->H); } break; // SLA (c->IY+d),H
            case 0x25: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->L=_z80_sla(c,v);_WR(a,c->L); } break; // SLA (c->IY+d),L
            case 0x26: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_WR(a,_z80_sla(c,v)); } break; // SLA (c->IY+d)
            case 0x27: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->A=_z80_sla(c,v);_WR(a,c->A); } break; // SLA (c->IY+d),A
            case 0x28: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->B=_z80_sra(c,v);_WR(a,c->B); } break; // SRA (c->IY+d),B
            case 0x29: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->C=_z80_sra(c,v);_WR(a,c->C); } break; // SRA (c->IY+d),C
            case 0x2a: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->D=_z80_sra(c,v);_WR(a,c->D); } break; // SRA (c->IY+d),D
            case 0x2b: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->E=_z80_sra(c,v);_WR(a,c->E); } break; // SRA (c->IY+d),E
            case 0x2c: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->H=_z80_sra(c,v);_WR(a,c->H); } break; // SRA (c->IY+d),H
            case 0x2d: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->L=_z80_sra(c,v);_WR(a,c->L); } break; // SRA (c->IY+d),L
            case 0x2e: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_WR(a,_z80_sra(c,v)); } break; // SRA (c->IY+d)
            case 0x2f: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->A=_z80_sra(c,v);_WR(a,c->A); } break; // SRA (c->IY+d),A
            case 0x30: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->B=_z80_sll(c,v);_WR(a,c->B); } break; // SLL (c->IY+d),B
            case 0x31: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->C=_z80_sll(c,v);_WR(a,c->C); } break; // SLL (c->IY+d),C
            case 0x32: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->D=_z80_sll(c,v);_WR(a,c->D); } break; // SLL (c->IY+d),D
            case 0x33: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->E=_z80_sll(c,v);_WR(a,c->E); } break; // SLL (c->IY+d),E
            case 0x34: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->H=_z80_sll(c,v);_WR(a,c->H); } break; // SLL (c->IY+d),H
            case 0x35: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->L=_z80_sll(c,v);_WR(a,c->L); } break; // SLL (c->IY+d),L
            case 0x36: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_WR(a,_z80_sll(c,v)); } break; // SLL (c->IY+d)
            case 0x37: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->A=_z80_sll(c,v);_WR(a,c->A); } break; // SLL (c->IY+d),A
            case 0x38: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->B=_z80_srl(c,v);_WR(a,c->B); } break; // SRL (c->IY+d),B
            case 0x39: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->C=_z80_srl(c,v);_WR(a,c->C); } break; // SRL (c->IY+d),C
            case 0x3a: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->D=_z80_srl(c,v);_WR(a,c->D); } break; // SRL (c->IY+d),D
            case 0x3b: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->E=_z80_srl(c,v);_WR(a,c->E); } break; // SRL (c->IY+d),E
            case 0x3c: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->H=_z80_srl(c,v);_WR(a,c->H); } break; // SRL (c->IY+d),H
            case 0x3d: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->L=_z80_srl(c,v);_WR(a,c->L); } break; // SRL (c->IY+d),L
            case 0x3e: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_WR(a,_z80_srl(c,v)); } break; // SRL (c->IY+d)
            case 0x3f: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->A=_z80_srl(c,v);_WR(a,c->A); } break; // SRL (c->IY+d),A
            case 0x40: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x1); } break; // BIT 0,(c->IY+d)
            case 0x41: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x1); } break; // BIT 0,(c->IY+d)
            case 0x42: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x1); } break; // BIT 0,(c->IY+d)
            case 0x43: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x1); } break; // BIT 0,(c->IY+d)
            case 0x44: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x1); } break; // BIT 0,(c->IY+d)
            case 0x45: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x1); } break; // BIT 0,(c->IY+d)
            case 0x46: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x1); } break; // BIT 0,(c->IY+d)
            case 0x47: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x1); } break; // BIT 0,(c->IY+d)
            case 0x48: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x2); } break; // BIT 1,(c->IY+d)
            case 0x49: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x2); } break; // BIT 1,(c->IY+d)
            case 0x4a: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x2); } break; // BIT 1,(c->IY+d)
            case 0x4b: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x2); } break; // BIT 1,(c->IY+d)
            case 0x4c: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x2); } break; // BIT 1,(c->IY+d)
            case 0x4d: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x2); } break; // BIT 1,(c->IY+d)
            case 0x4e: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x2); } break; // BIT 1,(c->IY+d)
            case 0x4f: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x2); } break; // BIT 1,(c->IY+d)
            case 0x50: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x4); } break; // BIT 2,(c->IY+d)
            case 0x51: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x4); } break; // BIT 2,(c->IY+d)
            case 0x52: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x4); } break; // BIT 2,(c->IY+d)
            case 0x53: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x4); } break; // BIT 2,(c->IY+d)
            case 0x54: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x4); } break; // BIT 2,(c->IY+d)
            case 0x55: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x4); } break; // BIT 2,(c->IY+d)
            case 0x56: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x4); } break; // BIT 2,(c->IY+d)
            case 0x57: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x4); } break; // BIT 2,(c->IY+d)
            case 0x58: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x8); } break; // BIT 3,(c->IY+d)
            case 0x59: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x8); } break; // BIT 3,(c->IY+d)
            case 0x5a: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x8); } break; // BIT 3,(c->IY+d)
            case 0x5b: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x8); } break; // BIT 3,(c->IY+d)
            case 0x5c: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x8); } break; // BIT 3,(c->IY+d)
            case 0x5d: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x8); } break; // BIT 3,(c->IY+d)
            case 0x5e: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x8); } break; // BIT 3,(c->IY+d)
            case 0x5f: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x8); } break; // BIT 3,(c->IY+d)
            case 0x60: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x10); } break; // BIT 4,(c->IY+d)
            case 0x61: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x10); } break; // BIT 4,(c->IY+d)
            case 0x62: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x10); } break; // BIT 4,(c->IY+d)
            case 0x63: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x10); } break; // BIT 4,(c->IY+d)
            case 0x64: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x10); } break; // BIT 4,(c->IY+d)
            case 0x65: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x10); } break; // BIT 4,(c->IY+d)
            case 0x66: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x10); } break; // BIT 4,(c->IY+d)
            case 0x67: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x10); } break; // BIT 4,(c->IY+d)
            case 0x68: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x20); } break; // BIT 5,(c->IY+d)
            case 0x69: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x20); } break; // BIT 5,(c->IY+d)
            case 0x6a: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x20); } break; // BIT 5,(c->IY+d)
            case 0x6b: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x20); } break; // BIT 5,(c->IY+d)
            case 0x6c: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x20); } break; // BIT 5,(c->IY+d)
            case 0x6d: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x20); } break; // BIT 5,(c->IY+d)
            case 0x6e: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x20); } break; // BIT 5,(c->IY+d)
            case 0x6f: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x20); } break; // BIT 5,(c->IY+d)
            case 0x70: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x40); } break; // BIT 6,(c->IY+d)
            case 0x71: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x40); } break; // BIT 6,(c->IY+d)
            case 0x72: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x40); } break; // BIT 6,(c->IY+d)
            case 0x73: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x40); } break; // BIT 6,(c->IY+d)
            case 0x74: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x40); } break; // BIT 6,(c->IY+d)
            case 0x75: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x40); } break; // BIT 6,(c->IY+d)
            case 0x76: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x40); } break; // BIT 6,(c->IY+d)
            case 0x77: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x40); } break; // BIT 6,(c->IY+d)
            case 0x78: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x80); } break; // BIT 7,(c->IY+d)
            case 0x79: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x80); } break; // BIT 7,(c->IY+d)
            case 0x7a: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x80); } break; // BIT 7,(c->IY+d)
            case 0x7b: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x80); } break; // BIT 7,(c->IY+d)
            case 0x7c: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x80); } break; // BIT 7,(c->IY+d)
            case 0x7d: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x80); } break; // BIT 7,(c->IY+d)
            case 0x7e: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x80); } break; // BIT 7,(c->IY+d)
            case 0x7f: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_z80_ibit(c,v,0x80); } break; // BIT 7,(c->IY+d)
            case 0x80: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->B=v&~0x1;_WR(a,c->B); } break; // RES 0,(c->IY+d),B
            case 0x81: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->C=v&~0x1;_WR(a,c->C); } break; // RES 0,(c->IY+d),C
            case 0x82: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->D=v&~0x1;_WR(a,c->D); } break; // RES 0,(c->IY+d),D
            case 0x83: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->E=v&~0x1;_WR(a,c->E); } break; // RES 0,(c->IY+d),E
            case 0x84: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->H=v&~0x1;_WR(a,c->H); } break; // RES 0,(c->IY+d),H
            case 0x85: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->L=v&~0x1;_WR(a,c->L); } break; // RES 0,(c->IY+d),L
            case 0x86: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_WR(a,v&~0x1); } break; // RES 0,(c->IY+d)
            case 0x87: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->A=v&~0x1;_WR(a,c->A); } break; // RES 0,(c->IY+d),A
            case 0x88: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->B=v&~0x2;_WR(a,c->B); } break; // RES 1,(c->IY+d),B
            case 0x89: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->C=v&~0x2;_WR(a,c->C); } break; // RES 1,(c->IY+d),C
            case 0x8a: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->D=v&~0x2;_WR(a,c->D); } break; // RES 1,(c->IY+d),D
            case 0x8b: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->E=v&~0x2;_WR(a,c->E); } break; // RES 1,(c->IY+d),E
            case 0x8c: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->H=v&~0x2;_WR(a,c->H); } break; // RES 1,(c->IY+d),H
            case 0x8d: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->L=v&~0x2;_WR(a,c->L); } break; // RES 1,(c->IY+d),L
            case 0x8e: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_WR(a,v&~0x2); } break; // RES 1,(c->IY+d)
            case 0x8f: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->A=v&~0x2;_WR(a,c->A); } break; // RES 1,(c->IY+d),A
            case 0x90: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->B=v&~0x4;_WR(a,c->B); } break; // RES 2,(c->IY+d),B
            case 0x91: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->C=v&~0x4;_WR(a,c->C); } break; // RES 2,(c->IY+d),C
            case 0x92: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->D=v&~0x4;_WR(a,c->D); } break; // RES 2,(c->IY+d),D
            case 0x93: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->E=v&~0x4;_WR(a,c->E); } break; // RES 2,(c->IY+d),E
            case 0x94: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->H=v&~0x4;_WR(a,c->H); } break; // RES 2,(c->IY+d),H
            case 0x95: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->L=v&~0x4;_WR(a,c->L); } break; // RES 2,(c->IY+d),L
            case 0x96: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_WR(a,v&~0x4); } break; // RES 2,(c->IY+d)
            case 0x97: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->A=v&~0x4;_WR(a,c->A); } break; // RES 2,(c->IY+d),A
            case 0x98: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->B=v&~0x8;_WR(a,c->B); } break; // RES 3,(c->IY+d),B
            case 0x99: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->C=v&~0x8;_WR(a,c->C); } break; // RES 3,(c->IY+d),C
            case 0x9a: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->D=v&~0x8;_WR(a,c->D); } break; // RES 3,(c->IY+d),D
            case 0x9b: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->E=v&~0x8;_WR(a,c->E); } break; // RES 3,(c->IY+d),E
            case 0x9c: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->H=v&~0x8;_WR(a,c->H); } break; // RES 3,(c->IY+d),H
            case 0x9d: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->L=v&~0x8;_WR(a,c->L); } break; // RES 3,(c->IY+d),L
            case 0x9e: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_WR(a,v&~0x8); } break; // RES 3,(c->IY+d)
            case 0x9f: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->A=v&~0x8;_WR(a,c->A); } break; // RES 3,(c->IY+d),A
            case 0xa0: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->B=v&~0x10;_WR(a,c->B); } break; // RES 4,(c->IY+d),B
            case 0xa1: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->C=v&~0x10;_WR(a,c->C); } break; // RES 4,(c->IY+d),C
            case 0xa2: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->D=v&~0x10;_WR(a,c->D); } break; // RES 4,(c->IY+d),D
            case 0xa3: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->E=v&~0x10;_WR(a,c->E); } break; // RES 4,(c->IY+d),E
            case 0xa4: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->H=v&~0x10;_WR(a,c->H); } break; // RES 4,(c->IY+d),H
            case 0xa5: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->L=v&~0x10;_WR(a,c->L); } break; // RES 4,(c->IY+d),L
            case 0xa6: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_WR(a,v&~0x10); } break; // RES 4,(c->IY+d)
            case 0xa7: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->A=v&~0x10;_WR(a,c->A); } break; // RES 4,(c->IY+d),A
            case 0xa8: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->B=v&~0x20;_WR(a,c->B); } break; // RES 5,(c->IY+d),B
            case 0xa9: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->C=v&~0x20;_WR(a,c->C); } break; // RES 5,(c->IY+d),C
            case 0xaa: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->D=v&~0x20;_WR(a,c->D); } break; // RES 5,(c->IY+d),D
            case 0xab: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->E=v&~0x20;_WR(a,c->E); } break; // RES 5,(c->IY+d),E
            case 0xac: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->H=v&~0x20;_WR(a,c->H); } break; // RES 5,(c->IY+d),H
            case 0xad: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->L=v&~0x20;_WR(a,c->L); } break; // RES 5,(c->IY+d),L
            case 0xae: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_WR(a,v&~0x20); } break; // RES 5,(c->IY+d)
            case 0xaf: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->A=v&~0x20;_WR(a,c->A); } break; // RES 5,(c->IY+d),A
            case 0xb0: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->B=v&~0x40;_WR(a,c->B); } break; // RES 6,(c->IY+d),B
            case 0xb1: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->C=v&~0x40;_WR(a,c->C); } break; // RES 6,(c->IY+d),C
            case 0xb2: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->D=v&~0x40;_WR(a,c->D); } break; // RES 6,(c->IY+d),D
            case 0xb3: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->E=v&~0x40;_WR(a,c->E); } break; // RES 6,(c->IY+d),E
            case 0xb4: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->H=v&~0x40;_WR(a,c->H); } break; // RES 6,(c->IY+d),H
            case 0xb5: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->L=v&~0x40;_WR(a,c->L); } break; // RES 6,(c->IY+d),L
            case 0xb6: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_WR(a,v&~0x40); } break; // RES 6,(c->IY+d)
            case 0xb7: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->A=v&~0x40;_WR(a,c->A); } break; // RES 6,(c->IY+d),A
            case 0xb8: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->B=v&~0x80;_WR(a,c->B); } break; // RES 7,(c->IY+d),B
            case 0xb9: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->C=v&~0x80;_WR(a,c->C); } break; // RES 7,(c->IY+d),C
            case 0xba: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->D=v&~0x80;_WR(a,c->D); } break; // RES 7,(c->IY+d),D
            case 0xbb: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->E=v&~0x80;_WR(a,c->E); } break; // RES 7,(c->IY+d),E
            case 0xbc: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->H=v&~0x80;_WR(a,c->H); } break; // RES 7,(c->IY+d),H
            case 0xbd: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->L=v&~0x80;_WR(a,c->L); } break; // RES 7,(c->IY+d),L
            case 0xbe: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_WR(a,v&~0x80); } break; // RES 7,(c->IY+d)
            case 0xbf: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->A=v&~0x80;_WR(a,c->A); } break; // RES 7,(c->IY+d),A
            case 0xc0: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->B=v|0x1;_WR(a,c->B);} break; // SET 0,(c->IY+d),B
            case 0xc1: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->C=v|0x1;_WR(a,c->C);} break; // SET 0,(c->IY+d),C
            case 0xc2: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->D=v|0x1;_WR(a,c->D);} break; // SET 0,(c->IY+d),D
            case 0xc3: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->E=v|0x1;_WR(a,c->E);} break; // SET 0,(c->IY+d),E
            case 0xc4: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->H=v|0x1;_WR(a,c->IYH);} break; // SET 0,(c->IY+d),H
            case 0xc5: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->L=v|0x1;_WR(a,c->IYL);} break; // SET 0,(c->IY+d),L
            case 0xc6: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_WR(a,v|0x1);} break; // SET 0,(c->IY+d)
            case 0xc7: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->A=v|0x1;_WR(a,c->A);} break; // SET 0,(c->IY+d),A
            case 0xc8: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->B=v|0x2;_WR(a,c->B);} break; // SET 1,(c->IY+d),B
            case 0xc9: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->C=v|0x2;_WR(a,c->C);} break; // SET 1,(c->IY+d),C
            case 0xca: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->D=v|0x2;_WR(a,c->D);} break; // SET 1,(c->IY+d),D
            case 0xcb: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->E=v|0x2;_WR(a,c->E);} break; // SET 1,(c->IY+d),E
            case 0xcc: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->H=v|0x2;_WR(a,c->IYH);} break; // SET 1,(c->IY+d),H
            case 0xcd: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->L=v|0x2;_WR(a,c->IYL);} break; // SET 1,(c->IY+d),L
            case 0xce: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_WR(a,v|0x2);} break; // SET 1,(c->IY+d)
            case 0xcf: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->A=v|0x2;_WR(a,c->A);} break; // SET 1,(c->IY+d),A
            case 0xd0: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->B=v|0x4;_WR(a,c->B);} break; // SET 2,(c->IY+d),B
            case 0xd1: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->C=v|0x4;_WR(a,c->C);} break; // SET 2,(c->IY+d),C
            case 0xd2: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->D=v|0x4;_WR(a,c->D);} break; // SET 2,(c->IY+d),D
            case 0xd3: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->E=v|0x4;_WR(a,c->E);} break; // SET 2,(c->IY+d),E
            case 0xd4: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->H=v|0x4;_WR(a,c->IYH);} break; // SET 2,(c->IY+d),H
            case 0xd5: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->L=v|0x4;_WR(a,c->IYL);} break; // SET 2,(c->IY+d),L
            case 0xd6: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_WR(a,v|0x4);} break; // SET 2,(c->IY+d)
            case 0xd7: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->A=v|0x4;_WR(a,c->A);} break; // SET 2,(c->IY+d),A
            case 0xd8: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->B=v|0x8;_WR(a,c->B);} break; // SET 3,(c->IY+d),B
            case 0xd9: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->C=v|0x8;_WR(a,c->C);} break; // SET 3,(c->IY+d),C
            case 0xda: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->D=v|0x8;_WR(a,c->D);} break; // SET 3,(c->IY+d),D
            case 0xdb: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->E=v|0x8;_WR(a,c->E);} break; // SET 3,(c->IY+d),E
            case 0xdc: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->H=v|0x8;_WR(a,c->IYH);} break; // SET 3,(c->IY+d),H
            case 0xdd: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->L=v|0x8;_WR(a,c->IYL);} break; // SET 3,(c->IY+d),L
            case 0xde: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_WR(a,v|0x8);} break; // SET 3,(c->IY+d)
            case 0xdf: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->A=v|0x8;_WR(a,c->A);} break; // SET 3,(c->IY+d),A
            case 0xe0: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->B=v|0x10;_WR(a,c->B);} break; // SET 4,(c->IY+d),B
            case 0xe1: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->C=v|0x10;_WR(a,c->C);} break; // SET 4,(c->IY+d),C
            case 0xe2: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->D=v|0x10;_WR(a,c->D);} break; // SET 4,(c->IY+d),D
            case 0xe3: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->E=v|0x10;_WR(a,c->E);} break; // SET 4,(c->IY+d),E
            case 0xe4: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->H=v|0x10;_WR(a,c->IYH);} break; // SET 4,(c->IY+d),H
            case 0xe5: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->L=v|0x10;_WR(a,c->IYL);} break; // SET 4,(c->IY+d),L
            case 0xe6: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_WR(a,v|0x10);} break; // SET 4,(c->IY+d)
            case 0xe7: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->A=v|0x10;_WR(a,c->A);} break; // SET 4,(c->IY+d),A
            case 0xe8: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->B=v|0x20;_WR(a,c->B);} break; // SET 5,(c->IY+d),B
            case 0xe9: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->C=v|0x20;_WR(a,c->C);} break; // SET 5,(c->IY+d),C
            case 0xea: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->D=v|0x20;_WR(a,c->D);} break; // SET 5,(c->IY+d),D
            case 0xeb: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->E=v|0x20;_WR(a,c->E);} break; // SET 5,(c->IY+d),E
            case 0xec: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->H=v|0x20;_WR(a,c->IYH);} break; // SET 5,(c->IY+d),H
            case 0xed: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->L=v|0x20;_WR(a,c->IYL);} break; // SET 5,(c->IY+d),L
            case 0xee: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_WR(a,v|0x20);} break; // SET 5,(c->IY+d)
            case 0xef: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->A=v|0x20;_WR(a,c->A);} break; // SET 5,(c->IY+d),A
            case 0xf0: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->B=v|0x40;_WR(a,c->B);} break; // SET 6,(c->IY+d),B
            case 0xf1: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->C=v|0x40;_WR(a,c->C);} break; // SET 6,(c->IY+d),C
            case 0xf2: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->D=v|0x40;_WR(a,c->D);} break; // SET 6,(c->IY+d),D
            case 0xf3: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->E=v|0x40;_WR(a,c->E);} break; // SET 6,(c->IY+d),E
            case 0xf4: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->H=v|0x40;_WR(a,c->IYH);} break; // SET 6,(c->IY+d),H
            case 0xf5: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->L=v|0x40;_WR(a,c->IYL);} break; // SET 6,(c->IY+d),L
            case 0xf6: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_WR(a,v|0x40);} break; // SET 6,(c->IY+d)
            case 0xf7: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->A=v|0x40;_WR(a,c->A);} break; // SET 6,(c->IY+d),A
            case 0xf8: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->B=v|0x80;_WR(a,c->B);} break; // SET 7,(c->IY+d),B
            case 0xf9: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->C=v|0x80;_WR(a,c->C);} break; // SET 7,(c->IY+d),C
            case 0xfa: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->D=v|0x80;_WR(a,c->D);} break; // SET 7,(c->IY+d),D
            case 0xfb: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->E=v|0x80;_WR(a,c->E);} break; // SET 7,(c->IY+d),E
            case 0xfc: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->H=v|0x80;_WR(a,c->IYH);} break; // SET 7,(c->IY+d),H
            case 0xfd: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->L=v|0x80;_WR(a,c->IYL);} break; // SET 7,(c->IY+d),L
            case 0xfe: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);_WR(a,v|0x80);} break; // SET 7,(c->IY+d)
            case 0xff: { uint16_t a=c->WZ=c->IY+d;;_T();_T();uint8_t v;_RD(a,v);c->A=v|0x80;_WR(a,c->A);} break; // SET 7,(c->IY+d),A
            default: break;
          }
          break;
          }
        case 0xcc: ticks=_z80_callcc(c,(c->F&Z80_ZF),tick,ticks); break; // CALL Z,nn
        case 0xcd: ticks=_z80_call(c,tick,ticks); break; // CALL nn
        case 0xce: {uint8_t v;_RD(c->PC++,v);_z80_adc(c,v);} break; // ADC n
        case 0xcf: ticks=_z80_rst(c,0x8,tick,ticks); break; // RST 0x8
        case 0xd0: ticks=_z80_retcc(c,!(c->F&Z80_CF),tick,ticks); break; // RET NC
        case 0xd1: { uint8_t l;_RD(c->SP++,l);uint8_t h;_RD(c->SP++,h);c->DE=(h<<8)|l; } break; // POP DE
        case 0xd2: _IMM16(); if (!(c->F&Z80_CF)) { c->PC=c->WZ; } break; // JP NC,nn
        case 0xd3: {uint8_t v;_RD(c->PC++,v);c->WZ=((c->A<<8)|v);_OUT(c->WZ,c->A);c->Z++;} break; // OUT (n),A
        case 0xd4: ticks=_z80_callcc(c,!(c->F&Z80_CF),tick,ticks); break; // CALL NC,nn
        case 0xd5: _T();_WR(--c->SP,(uint8_t)(c->DE>>8)); _WR(--c->SP,(uint8_t)c->DE); break; // PUSH DE
        case 0xd6: {uint8_t v;_RD(c->PC++,v);_z80_sub(c,v);} break; // SUB n
        case 0xd7: ticks=_z80_rst(c,0x10,tick,ticks); break; // RST 0x10
        case 0xd8: ticks=_z80_retcc(c,(c->F&Z80_CF),tick,ticks); break; // RET C
        case 0xd9: _SWP16(c->BC,c->BC_);_SWP16(c->DE,c->DE_);_SWP16(c->HL,c->HL_); break; // EXX
        case 0xda: _IMM16(); if ((c->F&Z80_CF)) { c->PC=c->WZ; } break; // JP C,nn
        case 0xdb: {uint8_t v;_RD(c->PC++,v);c->WZ=((c->A<<8)|v);_IN(c->WZ++,c->A);} break; // IN A,(n)
        case 0xdc: ticks=_z80_callcc(c,(c->F&Z80_CF),tick,ticks); break; // CALL C,nn
        case 0xde: {uint8_t v;_RD(c->PC++,v);_z80_sbc(c,v);} break; // SBC n
        case 0xdf: ticks=_z80_rst(c,0x18,tick,ticks); break; // RST 0x18
        case 0xe0: ticks=_z80_retcc(c,!(c->F&Z80_PF),tick,ticks); break; // RET PO
        case 0xe1: { uint8_t l;_RD(c->SP++,l);uint8_t h;_RD(c->SP++,h);c->IY=(h<<8)|l; } break; // POP IY
        case 0xe2: _IMM16(); if (!(c->F&Z80_PF)) { c->PC=c->WZ; } break; // JP PO,nn
        case 0xe3: _T();_RD(c->SP,c->Z);_RD(c->SP+1,c->W);_WR(c->SP,(uint8_t)c->IY);_WR(c->SP+1,(uint8_t)(c->IY>>8));c->IY=c->WZ;_T();_T(); break; // EX (SP),IY
        case 0xe4: ticks=_z80_callcc(c,!(c->F&Z80_PF),tick,ticks); break; // CALL PO,nn
        case 0xe5: _T();_WR(--c->SP,(uint8_t)(c->IY>>8)); _WR(--c->SP,(uint8_t)c->IY); break; // PUSH IY
        case 0xe6: {uint8_t v;_RD(c->PC++,v);_z80_and(c,v);} break; // AND n
        case 0xe7: ticks=_z80_rst(c,0x20,tick,ticks); break; // RST 0x20
        case 0xe8: ticks=_z80_retcc(c,(c->F&Z80_PF),tick,ticks); break; // RET PE
        case 0xe9: c->PC=c->IY; break; // JP IY
        case 0xea: _IMM16(); if ((c->F&Z80_PF)) { c->PC=c->WZ; } break; // JP PE,nn
        case 0xeb: _SWP16(c->DE,c->HL); break; // EX DE,HL
        case 0xec: ticks=_z80_callcc(c,(c->F&Z80_PF),tick,ticks); break; // CALL PE,nn
        case 0xee: {uint8_t v;_RD(c->PC++,v);_z80_xor(c,v);} break; // XOR n
        case 0xef: ticks=_z80_rst(c,0x28,tick,ticks); break; // RST 0x28
        case 0xf0: ticks=_z80_retcc(c,!(c->F&Z80_SF),tick,ticks); break; // RET P
        case 0xf1: { uint8_t l;_RD(c->SP++,l);uint8_t h;_RD(c->SP++,h);c->AF=(h<<8)|l; } break; // POP AF
        case 0xf2: _IMM16(); if (!(c->F&Z80_SF)) { c->PC=c->WZ; } break; // JP P,nn
        case 0xf3: _z80_di(c); break; // DI
        case 0xf4: ticks=_z80_callcc(c,!(c->F&Z80_SF),tick,ticks); break; // CALL P,nn
        case 0xf5: _T();_WR(--c->SP,(uint8_t)(c->AF>>8)); _WR(--c->SP,(uint8_t)c->AF); break; // PUSH AF
        case 0xf6: {uint8_t v;_RD(c->PC++,v);_z80_or(c,v);} break; // OR n
        case 0xf7: ticks=_z80_rst(c,0x30,tick,ticks); break; // RST 0x30
        case 0xf8: ticks=_z80_retcc(c,(c->F&Z80_SF),tick,ticks); break; // RET M
        case 0xf9: _T();_T();c->SP=c->IY; break; // LD SP,IY
        case 0xfa: _IMM16(); if ((c->F&Z80_SF)) { c->PC=c->WZ; } break; // JP M,nn
        case 0xfb: _z80_ei(c); break; // EI
        case 0xfc: ticks=_z80_callcc(c,(c->F&Z80_SF),tick,ticks); break; // CALL M,nn
        case 0xfe: {uint8_t v;_RD(c->PC++,v);_z80_cp(c,v);} break; // CP n
        case 0xff: ticks=_z80_rst(c,0x38,tick,ticks); break; // RST 0x38
        default: break;
      }
      break;
    case 0xfe: {uint8_t v;_RD(c->PC++,v);_z80_cp(c,v);} break; // CP n
    case 0xff: ticks=_z80_rst(c,0x38,tick,ticks); break; // RST 0x38
    default: break;
  }
  c->ticks=ticks;
}
