// machine generated, do not edit!
void _z80_op(z80* c) {
  switch (_z80_fetch(c)) {
    case 0x0:   return; // NOP
    case 0x1: _IMM16(); c->BC=c->WZ; return; // LD BC,nn
    case 0x2: c->WZ=c->BC;_WR(c->WZ++,c->A);c->W=c->A; return; // LD (BC),A
    case 0x3: c->BC++; return; // INC BC
    case 0x4: c->B=_z80_inc(c->B); return; // INC B
    case 0x5: c->B=_z80_dec(c->B); return; // DEC B
    case 0x6: c->B=_RD(c->PC++); return; // LD B,n
    case 0x7: _z80_rlca(c); return; // RLCA
    case 0x8: _SWP16(c->FA)c->FA_)); return; // EX AF,AF'
    case 0x9: c->HL=_z80_add16(c,c->HL,c->BC); return; // ADD HL,BC
    case 0xa: c->WZ=c->BC;c->A=_RD(c->WZ++); return; // LD A,(BC)
    case 0xb: c->BC--; return; // DEC BC
    case 0xc: c->C=_z80_inc(c->C); return; // INC C
    case 0xd: c->C=_z80_dec(c->C); return; // DEC C
    case 0xe: c->C=_RD(c->PC++); return; // LD C,n
    case 0xf: _z80_rrca(c); return; // RRCA
    case 0x10: _z80_djnz(c); return; // DJNZ
    case 0x11: _IMM16(); c->DE=c->WZ; return; // LD DE,nn
    case 0x12: c->WZ=c->DE;_WR(c->WZ++,c->A);c->W=c->A; return; // LD (DE),A
    case 0x13: c->DE++; return; // INC DE
    case 0x14: c->D=_z80_inc(c->D); return; // INC D
    case 0x15: c->D=_z80_dec(c->D); return; // DEC D
    case 0x16: c->D=_RD(c->PC++); return; // LD D,n
    case 0x17: _z80_rla(c); return; // RLA
    case 0x18: _z80_jr(c); return; // JR d
    case 0x19: c->HL=_z80_add16(c,c->HL,c->DE); return; // ADD HL,DE
    case 0x1a: c->WZ=c->DE;c->A=_RD(c->WZ++); return; // LD A,(DE)
    case 0x1b: c->DE--; return; // DEC DE
    case 0x1c: c->E=_z80_inc(c->E); return; // INC E
    case 0x1d: c->E=_z80_dec(c->E); return; // DEC E
    case 0x1e: c->E=_RD(c->PC++); return; // LD E,n
    case 0x1f: _z80_rra(c); return; // RRA
    case 0x20: _z80_jr_cc(c,!(c->F)&Z80_ZF)) return; // JR NZ,d
    case 0x21: _IMM16(); c->HL=c->WZ; return; // LD HL,nn
    case 0x22: _IMM16();_WR(c->WZ++,c->HL);_WR(c->WZ,c->HL>>8); return; // LD (nn),HL
    case 0x23: c->HL++; return; // INC HL
    case 0x24: c->H=_z80_inc(c->H); return; // INC H
    case 0x25: c->H=_z80_dec(c->H); return; // DEC H
    case 0x26: c->H=_RD(c->PC++); return; // LD H,n
    case 0x27: _z80_daa(c); return; // DAA
    case 0x28: _z80_jr_cc(c,(c->F)&Z80_ZF)) return; // JR Z,d
    case 0x29: c->HL=_z80_add16(c,c->HL,c->HL); return; // ADD HL,HL
    case 0x2a: _IMM16();uint8_t l=_RD(c->WZ++);uint8_t h=_RD(c->WZ);c->HL=(h<<8)|l; return; // LD HL,(nn)
    case 0x2b: c->HL--; return; // DEC HL
    case 0x2c: c->L=_z80_inc(c->L); return; // INC L
    case 0x2d: c->L=_z80_dec(c->L); return; // DEC L
    case 0x2e: c->L=_RD(c->PC++); return; // LD L,n
    case 0x2f: _z80_cpl(c); return; // CPL
    case 0x30: _z80_jr_cc(c,!(c->F&Z80_CF)) return; // JR NC,d
    case 0x31: _IMM16(); c->SP=c->WZ; return; // LD SP,nn
    case 0x32: _IMM16();_WR(c->WZ++,c->A);c->W=c->A; return; // LD (nn),A
    case 0x33: c->SP++; return; // INC SP
    case 0x34: { uint16_t a=HL; _WR(a,_z80_inc(_RD(a))); } return; // INC (HL)
    case 0x35: { uint16_t a=HL; _WR(a,_z80_dec(_RD(a))); } return; // DEC (HL)
    case 0x36: { uint16_t a=HL; _WR(a,_RD(c->PC++)); } return; // LD (HL),n
    case 0x37: _z80_scf(c); return; // SCF
    case 0x38: _z80_jr_cc(c,(c->F&Z80_CF)) return; // JR C,d
    case 0x39: c->HL=_z80_add16(c,c->HL,c->SP); return; // ADD HL,SP
    case 0x3a: _IMM16();c->A=_RD(c->WZ++); return; // LD A,(nn)
    case 0x3b: c->SP--; return; // DEC SP
    case 0x3c: c->A=_z80_inc(c->A); return; // INC A
    case 0x3d: c->A=_z80_dec(c->A); return; // DEC A
    case 0x3e: c->A=_RD(c->PC++); return; // LD A,n
    case 0x3f: _z80_ccf(c); return; // CCF
    case 0x40: c->B=c->B; return; // LD B,B
    case 0x41: c->B=c->C; return; // LD B,C
    case 0x42: c->B=c->D; return; // LD B,D
    case 0x43: c->B=c->E; return; // LD B,E
    case 0x44: c->B=c->H; return; // LD B,H
    case 0x45: c->B=c->L; return; // LD B,L
    case 0x46: { uint16_t a=HL; c->B=_RD(a); }; return; // LD B,(HL)
    case 0x47: c->B=c->A; return; // LD B,A
    case 0x48: c->C=c->B; return; // LD C,B
    case 0x49: c->C=c->C; return; // LD C,C
    case 0x4a: c->C=c->D; return; // LD C,D
    case 0x4b: c->C=c->E; return; // LD C,E
    case 0x4c: c->C=c->H; return; // LD C,H
    case 0x4d: c->C=c->L; return; // LD C,L
    case 0x4e: { uint16_t a=HL; c->C=_RD(a); }; return; // LD C,(HL)
    case 0x4f: c->C=c->A; return; // LD C,A
    case 0x50: c->D=c->B; return; // LD D,B
    case 0x51: c->D=c->C; return; // LD D,C
    case 0x52: c->D=c->D; return; // LD D,D
    case 0x53: c->D=c->E; return; // LD D,E
    case 0x54: c->D=c->H; return; // LD D,H
    case 0x55: c->D=c->L; return; // LD D,L
    case 0x56: { uint16_t a=HL; c->D=_RD(a); }; return; // LD D,(HL)
    case 0x57: c->D=c->A; return; // LD D,A
    case 0x58: c->E=c->B; return; // LD E,B
    case 0x59: c->E=c->C; return; // LD E,C
    case 0x5a: c->E=c->D; return; // LD E,D
    case 0x5b: c->E=c->E; return; // LD E,E
    case 0x5c: c->E=c->H; return; // LD E,H
    case 0x5d: c->E=c->L; return; // LD E,L
    case 0x5e: { uint16_t a=HL; c->E=_RD(a); }; return; // LD E,(HL)
    case 0x5f: c->E=c->A; return; // LD E,A
    case 0x60: c->H=c->B; return; // LD H,B
    case 0x61: c->H=c->C; return; // LD H,C
    case 0x62: c->H=c->D; return; // LD H,D
    case 0x63: c->H=c->E; return; // LD H,E
    case 0x64: c->H=c->H; return; // LD H,H
    case 0x65: c->H=c->L; return; // LD H,L
    case 0x66: { uint16_t a=HL; c->H=_RD(a); }; return; // LD H,(HL)
    case 0x67: c->H=c->A; return; // LD H,A
    case 0x68: c->L=c->B; return; // LD L,B
    case 0x69: c->L=c->C; return; // LD L,C
    case 0x6a: c->L=c->D; return; // LD L,D
    case 0x6b: c->L=c->E; return; // LD L,E
    case 0x6c: c->L=c->H; return; // LD L,H
    case 0x6d: c->L=c->L; return; // LD L,L
    case 0x6e: { uint16_t a=HL; c->L=_RD(a); }; return; // LD L,(HL)
    case 0x6f: c->L=c->A; return; // LD L,A
    case 0x70: { uint16_t a=HL; _WR(a,c->B); }; return; // LD (HL),B
    case 0x71: { uint16_t a=HL; _WR(a,c->C); }; return; // LD (HL),C
    case 0x72: { uint16_t a=HL; _WR(a,c->D); }; return; // LD (HL),D
    case 0x73: { uint16_t a=HL; _WR(a,c->E); }; return; // LD (HL),E
    case 0x74: { uint16_t a=HL; _WR(a,c->H); }; return; // LD (HL),H
    case 0x75: { uint16_t a=HL; _WR(a,c->L); }; return; // LD (HL),L
    case 0x76: _z80_halt(c); return; // HALT
    case 0x77: { uint16_t a=HL; _WR(a,c->A); }; return; // LD (HL),A
    case 0x78: c->A=c->B; return; // LD A,B
    case 0x79: c->A=c->C; return; // LD A,C
    case 0x7a: c->A=c->D; return; // LD A,D
    case 0x7b: c->A=c->E; return; // LD A,E
    case 0x7c: c->A=c->H; return; // LD A,H
    case 0x7d: c->A=c->L; return; // LD A,L
    case 0x7e: { uint16_t a=HL; c->A=_RD(a); }; return; // LD A,(HL)
    case 0x7f: c->A=c->A; return; // LD A,A
    case 0x80: _z80_add(c->B); return; // ADD B
    case 0x81: _z80_add(c->C); return; // ADD C
    case 0x82: _z80_add(c->D); return; // ADD D
    case 0x83: _z80_add(c->E); return; // ADD E
    case 0x84: _z80_add(c->H); return; // ADD H
    case 0x85: _z80_add(c->L); return; // ADD L
    case 0x86: { uint16_t a=HL; _z80_add(_RD(a)); }; return; // ADD (HL)
    case 0x87: _z80_add(c->A); return; // ADD A
    case 0x88: _z80_adc(c->B); return; // ADC B
    case 0x89: _z80_adc(c->C); return; // ADC C
    case 0x8a: _z80_adc(c->D); return; // ADC D
    case 0x8b: _z80_adc(c->E); return; // ADC E
    case 0x8c: _z80_adc(c->H); return; // ADC H
    case 0x8d: _z80_adc(c->L); return; // ADC L
    case 0x8e: { uint16_t a=HL; _z80_adc(_RD(a)); }; return; // ADC (HL)
    case 0x8f: _z80_adc(c->A); return; // ADC A
    case 0x90: _z80_sub(c->B); return; // SUB B
    case 0x91: _z80_sub(c->C); return; // SUB C
    case 0x92: _z80_sub(c->D); return; // SUB D
    case 0x93: _z80_sub(c->E); return; // SUB E
    case 0x94: _z80_sub(c->H); return; // SUB H
    case 0x95: _z80_sub(c->L); return; // SUB L
    case 0x96: { uint16_t a=HL; _z80_sub(_RD(a)); }; return; // SUB (HL)
    case 0x97: _z80_sub(c->A); return; // SUB A
    case 0x98: _z80_sbc(c->B); return; // SBC B
    case 0x99: _z80_sbc(c->C); return; // SBC C
    case 0x9a: _z80_sbc(c->D); return; // SBC D
    case 0x9b: _z80_sbc(c->E); return; // SBC E
    case 0x9c: _z80_sbc(c->H); return; // SBC H
    case 0x9d: _z80_sbc(c->L); return; // SBC L
    case 0x9e: { uint16_t a=HL; _z80_sbc(_RD(a)); }; return; // SBC (HL)
    case 0x9f: _z80_sbc(c->A); return; // SBC A
    case 0xa0: _z80_and(c->B); return; // AND B
    case 0xa1: _z80_and(c->C); return; // AND C
    case 0xa2: _z80_and(c->D); return; // AND D
    case 0xa3: _z80_and(c->E); return; // AND E
    case 0xa4: _z80_and(c->H); return; // AND H
    case 0xa5: _z80_and(c->L); return; // AND L
    case 0xa6: { uint16_t a=HL; _z80_and(_RD(a)); }; return; // AND (HL)
    case 0xa7: _z80_and(c->A); return; // AND A
    case 0xa8: _z80_xor(c->B); return; // XOR B
    case 0xa9: _z80_xor(c->C); return; // XOR C
    case 0xaa: _z80_xor(c->D); return; // XOR D
    case 0xab: _z80_xor(c->E); return; // XOR E
    case 0xac: _z80_xor(c->H); return; // XOR H
    case 0xad: _z80_xor(c->L); return; // XOR L
    case 0xae: { uint16_t a=HL; _z80_xor(_RD(a)); }; return; // XOR (HL)
    case 0xaf: _z80_xor(c->A); return; // XOR A
    case 0xb0: _z80_or(c->B); return; // OR B
    case 0xb1: _z80_or(c->C); return; // OR C
    case 0xb2: _z80_or(c->D); return; // OR D
    case 0xb3: _z80_or(c->E); return; // OR E
    case 0xb4: _z80_or(c->H); return; // OR H
    case 0xb5: _z80_or(c->L); return; // OR L
    case 0xb6: { uint16_t a=HL; _z80_or(_RD(a)); }; return; // OR (HL)
    case 0xb7: _z80_or(c->A); return; // OR A
    case 0xb8: _z80_cp(c->B); return; // CP B
    case 0xb9: _z80_cp(c->C); return; // CP C
    case 0xba: _z80_cp(c->D); return; // CP D
    case 0xbb: _z80_cp(c->E); return; // CP E
    case 0xbc: _z80_cp(c->H); return; // CP H
    case 0xbd: _z80_cp(c->L); return; // CP L
    case 0xbe: { uint16_t a=HL; _z80_cp(_RD(a)); }; return; // CP (HL)
    case 0xbf: _z80_cp(c->A); return; // CP A
    case 0xc0: _z80_retcc(c,!(c->F)&Z80_ZF)); return; // RET NZ
    case 0xc1: uint8_t l=_RD(c->SP++);uint8_t h=_RD(c->SP++);c->BC=(h<<8)|l; return; // POP BC
    case 0xc2: _IMM16(); if (!(c->F)&Z80_ZF)) { c->PC=c->WZ; } return; // JP NZ,nn
    case 0xc3: _IMM16(); c->PC=c->WZ; return; // JP nn
    case 0xc4: _z80_callcc(c, !(c->F)&Z80_ZF)); return; // CALL NZ,nn
    case 0xc5: _WR(--c->SP,BC<<8); _WR(--c->SP,BC); return; // PUSH BC
    case 0xc6: _z80_add(_RD(c->PC++)); return; // ADD n
    case 0xc7: _z80_rst(0x0); return; // RST 0x0
    case 0xc8: _z80_retcc(c,(c->F)&Z80_ZF)); return; // RET Z
    case 0xc9: c->Z=_RD(c->SP++);c->W=_RD(c->SP++);c->PC=c->WZ; return; // RET
    case 0xca: _IMM16(); if ((c->F)&Z80_ZF)) { c->PC=c->WZ; } return; // JP Z,nn
    case 0xcb:
      switch (_z80_fetch(c)) {
        case 0x0: c->B=_z80_rlc(c->B); return; // RLC B
        case 0x1: c->C=_z80_rlc(c->C); return; // RLC C
        case 0x2: c->D=_z80_rlc(c->D); return; // RLC D
        case 0x3: c->E=_z80_rlc(c->E); return; // RLC E
        case 0x4: c->H=_z80_rlc(c->H); return; // RLC H
        case 0x5: c->L=_z80_rlc(c->L); return; // RLC L
        case 0x6: { uint16_t a=HL; _WR(a,_z80_rlc(_RD(a))); } return; // RLC (HL)
        case 0x7: c->A=_z80_rlc(c->A); return; // RLC A
        case 0x8: c->B=_z80_rrc(c->B); return; // RRC B
        case 0x9: c->C=_z80_rrc(c->C); return; // RRC C
        case 0xa: c->D=_z80_rrc(c->D); return; // RRC D
        case 0xb: c->E=_z80_rrc(c->E); return; // RRC E
        case 0xc: c->H=_z80_rrc(c->H); return; // RRC H
        case 0xd: c->L=_z80_rrc(c->L); return; // RRC L
        case 0xe: { uint16_t a=HL; _WR(a,_z80_rrc(_RD(a))); } return; // RRC (HL)
        case 0xf: c->A=_z80_rrc(c->A); return; // RRC A
        case 0x10: c->B=_z80_rl(c->B); return; // RL B
        case 0x11: c->C=_z80_rl(c->C); return; // RL C
        case 0x12: c->D=_z80_rl(c->D); return; // RL D
        case 0x13: c->E=_z80_rl(c->E); return; // RL E
        case 0x14: c->H=_z80_rl(c->H); return; // RL H
        case 0x15: c->L=_z80_rl(c->L); return; // RL L
        case 0x16: { uint16_t a=HL; _WR(a,_z80_rl(_RD(a))); } return; // RL (HL)
        case 0x17: c->A=_z80_rl(c->A); return; // RL A
        case 0x18: c->B=_z80_rr(c->B); return; // RR B
        case 0x19: c->C=_z80_rr(c->C); return; // RR C
        case 0x1a: c->D=_z80_rr(c->D); return; // RR D
        case 0x1b: c->E=_z80_rr(c->E); return; // RR E
        case 0x1c: c->H=_z80_rr(c->H); return; // RR H
        case 0x1d: c->L=_z80_rr(c->L); return; // RR L
        case 0x1e: { uint16_t a=HL; _WR(a,_z80_rr(_RD(a))); } return; // RR (HL)
        case 0x1f: c->A=_z80_rr(c->A); return; // RR A
        case 0x20: c->B=_z80_sla(c->B); return; // SLA B
        case 0x21: c->C=_z80_sla(c->C); return; // SLA C
        case 0x22: c->D=_z80_sla(c->D); return; // SLA D
        case 0x23: c->E=_z80_sla(c->E); return; // SLA E
        case 0x24: c->H=_z80_sla(c->H); return; // SLA H
        case 0x25: c->L=_z80_sla(c->L); return; // SLA L
        case 0x26: { uint16_t a=HL; _WR(a,_z80_sla(_RD(a))); } return; // SLA (HL)
        case 0x27: c->A=_z80_sla(c->A); return; // SLA A
        case 0x28: c->B=_z80_sra(c->B); return; // SRA B
        case 0x29: c->C=_z80_sra(c->C); return; // SRA C
        case 0x2a: c->D=_z80_sra(c->D); return; // SRA D
        case 0x2b: c->E=_z80_sra(c->E); return; // SRA E
        case 0x2c: c->H=_z80_sra(c->H); return; // SRA H
        case 0x2d: c->L=_z80_sra(c->L); return; // SRA L
        case 0x2e: { uint16_t a=HL; _WR(a,_z80_sra(_RD(a))); } return; // SRA (HL)
        case 0x2f: c->A=_z80_sra(c->A); return; // SRA A
        case 0x30: c->B=_z80_sll(c->B); return; // SLL B
        case 0x31: c->C=_z80_sll(c->C); return; // SLL C
        case 0x32: c->D=_z80_sll(c->D); return; // SLL D
        case 0x33: c->E=_z80_sll(c->E); return; // SLL E
        case 0x34: c->H=_z80_sll(c->H); return; // SLL H
        case 0x35: c->L=_z80_sll(c->L); return; // SLL L
        case 0x36: { uint16_t a=HL; _WR(a,_z80_sll(_RD(a))); } return; // SLL (HL)
        case 0x37: c->A=_z80_sll(c->A); return; // SLL A
        case 0x38: c->B=_z80_srl(c->B); return; // SRL B
        case 0x39: c->C=_z80_srl(c->C); return; // SRL C
        case 0x3a: c->D=_z80_srl(c->D); return; // SRL D
        case 0x3b: c->E=_z80_srl(c->E); return; // SRL E
        case 0x3c: c->H=_z80_srl(c->H); return; // SRL H
        case 0x3d: c->L=_z80_srl(c->L); return; // SRL L
        case 0x3e: { uint16_t a=HL; _WR(a,_z80_srl(_RD(a))); } return; // SRL (HL)
        case 0x3f: c->A=_z80_srl(c->A); return; // SRL A
        case 0x40: _z80_bit(c,c->B,0x1); return; // BIT 0,B
        case 0x41: _z80_bit(c,c->C,0x1); return; // BIT 0,C
        case 0x42: _z80_bit(c,c->D,0x1); return; // BIT 0,D
        case 0x43: _z80_bit(c,c->E,0x1); return; // BIT 0,E
        case 0x44: _z80_bit(c,c->H,0x1); return; // BIT 0,H
        case 0x45: _z80_bit(c,c->L,0x1); return; // BIT 0,L
        case 0x46: { uint16_t a=HL; _z80_ibit(c,_RD(a),0x1); } return; // BIT 0,(HL)
        case 0x47: _z80_bit(c,c->A,0x1); return; // BIT 0,A
        case 0x48: _z80_bit(c,c->B,0x2); return; // BIT 1,B
        case 0x49: _z80_bit(c,c->C,0x2); return; // BIT 1,C
        case 0x4a: _z80_bit(c,c->D,0x2); return; // BIT 1,D
        case 0x4b: _z80_bit(c,c->E,0x2); return; // BIT 1,E
        case 0x4c: _z80_bit(c,c->H,0x2); return; // BIT 1,H
        case 0x4d: _z80_bit(c,c->L,0x2); return; // BIT 1,L
        case 0x4e: { uint16_t a=HL; _z80_ibit(c,_RD(a),0x2); } return; // BIT 1,(HL)
        case 0x4f: _z80_bit(c,c->A,0x2); return; // BIT 1,A
        case 0x50: _z80_bit(c,c->B,0x4); return; // BIT 2,B
        case 0x51: _z80_bit(c,c->C,0x4); return; // BIT 2,C
        case 0x52: _z80_bit(c,c->D,0x4); return; // BIT 2,D
        case 0x53: _z80_bit(c,c->E,0x4); return; // BIT 2,E
        case 0x54: _z80_bit(c,c->H,0x4); return; // BIT 2,H
        case 0x55: _z80_bit(c,c->L,0x4); return; // BIT 2,L
        case 0x56: { uint16_t a=HL; _z80_ibit(c,_RD(a),0x4); } return; // BIT 2,(HL)
        case 0x57: _z80_bit(c,c->A,0x4); return; // BIT 2,A
        case 0x58: _z80_bit(c,c->B,0x8); return; // BIT 3,B
        case 0x59: _z80_bit(c,c->C,0x8); return; // BIT 3,C
        case 0x5a: _z80_bit(c,c->D,0x8); return; // BIT 3,D
        case 0x5b: _z80_bit(c,c->E,0x8); return; // BIT 3,E
        case 0x5c: _z80_bit(c,c->H,0x8); return; // BIT 3,H
        case 0x5d: _z80_bit(c,c->L,0x8); return; // BIT 3,L
        case 0x5e: { uint16_t a=HL; _z80_ibit(c,_RD(a),0x8); } return; // BIT 3,(HL)
        case 0x5f: _z80_bit(c,c->A,0x8); return; // BIT 3,A
        case 0x60: _z80_bit(c,c->B,0x10); return; // BIT 4,B
        case 0x61: _z80_bit(c,c->C,0x10); return; // BIT 4,C
        case 0x62: _z80_bit(c,c->D,0x10); return; // BIT 4,D
        case 0x63: _z80_bit(c,c->E,0x10); return; // BIT 4,E
        case 0x64: _z80_bit(c,c->H,0x10); return; // BIT 4,H
        case 0x65: _z80_bit(c,c->L,0x10); return; // BIT 4,L
        case 0x66: { uint16_t a=HL; _z80_ibit(c,_RD(a),0x10); } return; // BIT 4,(HL)
        case 0x67: _z80_bit(c,c->A,0x10); return; // BIT 4,A
        case 0x68: _z80_bit(c,c->B,0x20); return; // BIT 5,B
        case 0x69: _z80_bit(c,c->C,0x20); return; // BIT 5,C
        case 0x6a: _z80_bit(c,c->D,0x20); return; // BIT 5,D
        case 0x6b: _z80_bit(c,c->E,0x20); return; // BIT 5,E
        case 0x6c: _z80_bit(c,c->H,0x20); return; // BIT 5,H
        case 0x6d: _z80_bit(c,c->L,0x20); return; // BIT 5,L
        case 0x6e: { uint16_t a=HL; _z80_ibit(c,_RD(a),0x20); } return; // BIT 5,(HL)
        case 0x6f: _z80_bit(c,c->A,0x20); return; // BIT 5,A
        case 0x70: _z80_bit(c,c->B,0x40); return; // BIT 6,B
        case 0x71: _z80_bit(c,c->C,0x40); return; // BIT 6,C
        case 0x72: _z80_bit(c,c->D,0x40); return; // BIT 6,D
        case 0x73: _z80_bit(c,c->E,0x40); return; // BIT 6,E
        case 0x74: _z80_bit(c,c->H,0x40); return; // BIT 6,H
        case 0x75: _z80_bit(c,c->L,0x40); return; // BIT 6,L
        case 0x76: { uint16_t a=HL; _z80_ibit(c,_RD(a),0x40); } return; // BIT 6,(HL)
        case 0x77: _z80_bit(c,c->A,0x40); return; // BIT 6,A
        case 0x78: _z80_bit(c,c->B,0x80); return; // BIT 7,B
        case 0x79: _z80_bit(c,c->C,0x80); return; // BIT 7,C
        case 0x7a: _z80_bit(c,c->D,0x80); return; // BIT 7,D
        case 0x7b: _z80_bit(c,c->E,0x80); return; // BIT 7,E
        case 0x7c: _z80_bit(c,c->H,0x80); return; // BIT 7,H
        case 0x7d: _z80_bit(c,c->L,0x80); return; // BIT 7,L
        case 0x7e: { uint16_t a=HL; _z80_ibit(c,_RD(a),0x80); } return; // BIT 7,(HL)
        case 0x7f: _z80_bit(c,c->A,0x80); return; // BIT 7,A
        case 0x80: c->B&=~0x1; return; // RES 0,B
        case 0x81: c->C&=~0x1; return; // RES 0,C
        case 0x82: c->D&=~0x1; return; // RES 0,D
        case 0x83: c->E&=~0x1; return; // RES 0,E
        case 0x84: c->H&=~0x1; return; // RES 0,H
        case 0x85: c->L&=~0x1; return; // RES 0,L
        case 0x86: { uint16_t a=HL; _WR(a,_RD(a)&~0x1); } return; // RES 0,(HL)
        case 0x87: c->A&=~0x1; return; // RES 0,A
        case 0x88: c->B&=~0x2; return; // RES 1,B
        case 0x89: c->C&=~0x2; return; // RES 1,C
        case 0x8a: c->D&=~0x2; return; // RES 1,D
        case 0x8b: c->E&=~0x2; return; // RES 1,E
        case 0x8c: c->H&=~0x2; return; // RES 1,H
        case 0x8d: c->L&=~0x2; return; // RES 1,L
        case 0x8e: { uint16_t a=HL; _WR(a,_RD(a)&~0x2); } return; // RES 1,(HL)
        case 0x8f: c->A&=~0x2; return; // RES 1,A
        case 0x90: c->B&=~0x4; return; // RES 2,B
        case 0x91: c->C&=~0x4; return; // RES 2,C
        case 0x92: c->D&=~0x4; return; // RES 2,D
        case 0x93: c->E&=~0x4; return; // RES 2,E
        case 0x94: c->H&=~0x4; return; // RES 2,H
        case 0x95: c->L&=~0x4; return; // RES 2,L
        case 0x96: { uint16_t a=HL; _WR(a,_RD(a)&~0x4); } return; // RES 2,(HL)
        case 0x97: c->A&=~0x4; return; // RES 2,A
        case 0x98: c->B&=~0x8; return; // RES 3,B
        case 0x99: c->C&=~0x8; return; // RES 3,C
        case 0x9a: c->D&=~0x8; return; // RES 3,D
        case 0x9b: c->E&=~0x8; return; // RES 3,E
        case 0x9c: c->H&=~0x8; return; // RES 3,H
        case 0x9d: c->L&=~0x8; return; // RES 3,L
        case 0x9e: { uint16_t a=HL; _WR(a,_RD(a)&~0x8); } return; // RES 3,(HL)
        case 0x9f: c->A&=~0x8; return; // RES 3,A
        case 0xa0: c->B&=~0x10; return; // RES 4,B
        case 0xa1: c->C&=~0x10; return; // RES 4,C
        case 0xa2: c->D&=~0x10; return; // RES 4,D
        case 0xa3: c->E&=~0x10; return; // RES 4,E
        case 0xa4: c->H&=~0x10; return; // RES 4,H
        case 0xa5: c->L&=~0x10; return; // RES 4,L
        case 0xa6: { uint16_t a=HL; _WR(a,_RD(a)&~0x10); } return; // RES 4,(HL)
        case 0xa7: c->A&=~0x10; return; // RES 4,A
        case 0xa8: c->B&=~0x20; return; // RES 5,B
        case 0xa9: c->C&=~0x20; return; // RES 5,C
        case 0xaa: c->D&=~0x20; return; // RES 5,D
        case 0xab: c->E&=~0x20; return; // RES 5,E
        case 0xac: c->H&=~0x20; return; // RES 5,H
        case 0xad: c->L&=~0x20; return; // RES 5,L
        case 0xae: { uint16_t a=HL; _WR(a,_RD(a)&~0x20); } return; // RES 5,(HL)
        case 0xaf: c->A&=~0x20; return; // RES 5,A
        case 0xb0: c->B&=~0x40; return; // RES 6,B
        case 0xb1: c->C&=~0x40; return; // RES 6,C
        case 0xb2: c->D&=~0x40; return; // RES 6,D
        case 0xb3: c->E&=~0x40; return; // RES 6,E
        case 0xb4: c->H&=~0x40; return; // RES 6,H
        case 0xb5: c->L&=~0x40; return; // RES 6,L
        case 0xb6: { uint16_t a=HL; _WR(a,_RD(a)&~0x40); } return; // RES 6,(HL)
        case 0xb7: c->A&=~0x40; return; // RES 6,A
        case 0xb8: c->B&=~0x80; return; // RES 7,B
        case 0xb9: c->C&=~0x80; return; // RES 7,C
        case 0xba: c->D&=~0x80; return; // RES 7,D
        case 0xbb: c->E&=~0x80; return; // RES 7,E
        case 0xbc: c->H&=~0x80; return; // RES 7,H
        case 0xbd: c->L&=~0x80; return; // RES 7,L
        case 0xbe: { uint16_t a=HL; _WR(a,_RD(a)&~0x80); } return; // RES 7,(HL)
        case 0xbf: c->A&=~0x80; return; // RES 7,A
        case 0xc0: c->B|=0x1; return; // SET 0,B
        case 0xc1: c->C|=0x1; return; // SET 0,C
        case 0xc2: c->D|=0x1; return; // SET 0,D
        case 0xc3: c->E|=0x1; return; // SET 0,E
        case 0xc4: c->H|=0x1; return; // SET 0,H
        case 0xc5: c->L|=0x1; return; // SET 0,L
        case 0xc6: { uint16_t a=HL; _WR(a,_RD(a)|0x1);} return; // SET 0,(HL)
        case 0xc7: c->A|=0x1; return; // SET 0,A
        case 0xc8: c->B|=0x2; return; // SET 1,B
        case 0xc9: c->C|=0x2; return; // SET 1,C
        case 0xca: c->D|=0x2; return; // SET 1,D
        case 0xcb: c->E|=0x2; return; // SET 1,E
        case 0xcc: c->H|=0x2; return; // SET 1,H
        case 0xcd: c->L|=0x2; return; // SET 1,L
        case 0xce: { uint16_t a=HL; _WR(a,_RD(a)|0x2);} return; // SET 1,(HL)
        case 0xcf: c->A|=0x2; return; // SET 1,A
        case 0xd0: c->B|=0x4; return; // SET 2,B
        case 0xd1: c->C|=0x4; return; // SET 2,C
        case 0xd2: c->D|=0x4; return; // SET 2,D
        case 0xd3: c->E|=0x4; return; // SET 2,E
        case 0xd4: c->H|=0x4; return; // SET 2,H
        case 0xd5: c->L|=0x4; return; // SET 2,L
        case 0xd6: { uint16_t a=HL; _WR(a,_RD(a)|0x4);} return; // SET 2,(HL)
        case 0xd7: c->A|=0x4; return; // SET 2,A
        case 0xd8: c->B|=0x8; return; // SET 3,B
        case 0xd9: c->C|=0x8; return; // SET 3,C
        case 0xda: c->D|=0x8; return; // SET 3,D
        case 0xdb: c->E|=0x8; return; // SET 3,E
        case 0xdc: c->H|=0x8; return; // SET 3,H
        case 0xdd: c->L|=0x8; return; // SET 3,L
        case 0xde: { uint16_t a=HL; _WR(a,_RD(a)|0x8);} return; // SET 3,(HL)
        case 0xdf: c->A|=0x8; return; // SET 3,A
        case 0xe0: c->B|=0x10; return; // SET 4,B
        case 0xe1: c->C|=0x10; return; // SET 4,C
        case 0xe2: c->D|=0x10; return; // SET 4,D
        case 0xe3: c->E|=0x10; return; // SET 4,E
        case 0xe4: c->H|=0x10; return; // SET 4,H
        case 0xe5: c->L|=0x10; return; // SET 4,L
        case 0xe6: { uint16_t a=HL; _WR(a,_RD(a)|0x10);} return; // SET 4,(HL)
        case 0xe7: c->A|=0x10; return; // SET 4,A
        case 0xe8: c->B|=0x20; return; // SET 5,B
        case 0xe9: c->C|=0x20; return; // SET 5,C
        case 0xea: c->D|=0x20; return; // SET 5,D
        case 0xeb: c->E|=0x20; return; // SET 5,E
        case 0xec: c->H|=0x20; return; // SET 5,H
        case 0xed: c->L|=0x20; return; // SET 5,L
        case 0xee: { uint16_t a=HL; _WR(a,_RD(a)|0x20);} return; // SET 5,(HL)
        case 0xef: c->A|=0x20; return; // SET 5,A
        case 0xf0: c->B|=0x40; return; // SET 6,B
        case 0xf1: c->C|=0x40; return; // SET 6,C
        case 0xf2: c->D|=0x40; return; // SET 6,D
        case 0xf3: c->E|=0x40; return; // SET 6,E
        case 0xf4: c->H|=0x40; return; // SET 6,H
        case 0xf5: c->L|=0x40; return; // SET 6,L
        case 0xf6: { uint16_t a=HL; _WR(a,_RD(a)|0x40);} return; // SET 6,(HL)
        case 0xf7: c->A|=0x40; return; // SET 6,A
        case 0xf8: c->B|=0x80; return; // SET 7,B
        case 0xf9: c->C|=0x80; return; // SET 7,C
        case 0xfa: c->D|=0x80; return; // SET 7,D
        case 0xfb: c->E|=0x80; return; // SET 7,E
        case 0xfc: c->H|=0x80; return; // SET 7,H
        case 0xfd: c->L|=0x80; return; // SET 7,L
        case 0xfe: { uint16_t a=HL; _WR(a,_RD(a)|0x80);} return; // SET 7,(HL)
        case 0xff: c->A|=0x80; return; // SET 7,A
        default: return _INVALID_OPCODE(2);
      }
      break;
    case 0xcc: _z80_callcc(c, (c->F)&Z80_ZF)); return; // CALL Z,nn
    case 0xcd: _z80_call(c); return; // CALL nn
    case 0xce: _z80_adc(_RD(c->PC++)); return; // ADC n
    case 0xcf: _z80_rst(0x8); return; // RST 0x8
    case 0xd0: _z80_retcc(c,!(c->F&Z80_CF)); return; // RET NC
    case 0xd1: uint8_t l=_RD(c->SP++);uint8_t h=_RD(c->SP++);c->DE=(h<<8)|l; return; // POP DE
    case 0xd2: _IMM16(); if (!(c->F&Z80_CF)) { c->PC=c->WZ; } return; // JP NC,nn
    case 0xd3: _z80_out(c, (c->A<<8)|_RD(c->PC++), c->A); return; // OUT (n),A
    case 0xd4: _z80_callcc(c, !(c->F&Z80_CF)); return; // CALL NC,nn
    case 0xd5: _WR(--c->SP,DE<<8); _WR(--c->SP,DE); return; // PUSH DE
    case 0xd6: _z80_sub(_RD(c->PC++)); return; // SUB n
    case 0xd7: _z80_rst(0x10); return; // RST 0x10
    case 0xd8: _z80_retcc(c,(c->F&Z80_CF)); return; // RET C
    case 0xd9: _SWP16(c->BC,c->BC_); _SWP16(c->DE,c->DE_); SWP16(c->HL,c->HL_); SWP16(c->WZ,c->WZ_); return; // EXX
    case 0xda: _IMM16(); if ((c->F&Z80_CF)) { c->PC=c->WZ; } return; // JP C,nn
    case 0xdb: c->A=_z80_in(c, (c->A<<8)|_RD(c->PC++)); return; // IN A,(n)
    case 0xdc: _z80_callcc(c, (c->F&Z80_CF)); return; // CALL C,nn
    case 0xdd:
      switch (_z80_fetch(c)) {
        case 0x0:   return; // NOP
        case 0x1: _IMM16(); c->BC=c->WZ; return; // LD BC,nn
        case 0x2: c->WZ=c->BC;_WR(c->WZ++,c->A);c->W=c->A; return; // LD (BC),A
        case 0x3: c->BC++; return; // INC BC
        case 0x4: c->B=_z80_inc(c->B); return; // INC B
        case 0x5: c->B=_z80_dec(c->B); return; // DEC B
        case 0x6: c->B=_RD(c->PC++); return; // LD B,n
        case 0x7: _z80_rlca(c); return; // RLCA
        case 0x8: _SWP16(c->FA)c->FA_)); return; // EX AF,AF'
        case 0x9: c->IX=_z80_add16(c,c->IX,c->BC); return; // ADD IX,BC
        case 0xa: c->WZ=c->BC;c->A=_RD(c->WZ++); return; // LD A,(BC)
        case 0xb: c->BC--; return; // DEC BC
        case 0xc: c->C=_z80_inc(c->C); return; // INC C
        case 0xd: c->C=_z80_dec(c->C); return; // DEC C
        case 0xe: c->C=_RD(c->PC++); return; // LD C,n
        case 0xf: _z80_rrca(c); return; // RRCA
        case 0x10: _z80_djnz(c); return; // DJNZ
        case 0x11: _IMM16(); c->DE=c->WZ; return; // LD DE,nn
        case 0x12: c->WZ=c->DE;_WR(c->WZ++,c->A);c->W=c->A; return; // LD (DE),A
        case 0x13: c->DE++; return; // INC DE
        case 0x14: c->D=_z80_inc(c->D); return; // INC D
        case 0x15: c->D=_z80_dec(c->D); return; // DEC D
        case 0x16: c->D=_RD(c->PC++); return; // LD D,n
        case 0x17: _z80_rla(c); return; // RLA
        case 0x18: _z80_jr(c); return; // JR d
        case 0x19: c->IX=_z80_add16(c,c->IX,c->DE); return; // ADD IX,DE
        case 0x1a: c->WZ=c->DE;c->A=_RD(c->WZ++); return; // LD A,(DE)
        case 0x1b: c->DE--; return; // DEC DE
        case 0x1c: c->E=_z80_inc(c->E); return; // INC E
        case 0x1d: c->E=_z80_dec(c->E); return; // DEC E
        case 0x1e: c->E=_RD(c->PC++); return; // LD E,n
        case 0x1f: _z80_rra(c); return; // RRA
        case 0x20: _z80_jr_cc(c,!(c->F)&Z80_ZF)) return; // JR NZ,d
        case 0x21: _IMM16(); c->IX=c->WZ; return; // LD IX,nn
        case 0x22: _IMM16();_WR(c->WZ++,c->IX);_WR(c->WZ,c->IX>>8); return; // LD (nn),IX
        case 0x23: c->IX++; return; // INC IX
        case 0x24: c->IXH=_z80_inc(c->IXH); return; // INC IXH
        case 0x25: c->IXH=_z80_dec(c->IXH); return; // DEC IXH
        case 0x26: c->IXH=_RD(c->PC++); return; // LD IXH,n
        case 0x27: _z80_daa(c); return; // DAA
        case 0x28: _z80_jr_cc(c,(c->F)&Z80_ZF)) return; // JR Z,d
        case 0x29: c->IX=_z80_add16(c,c->IX,c->IX); return; // ADD IX,IX
        case 0x2a: _IMM16();uint8_t l=_RD(c->WZ++);uint8_t h=_RD(c->WZ);c->IX=(h<<8)|l; return; // LD IX,(nn)
        case 0x2b: c->IX--; return; // DEC IX
        case 0x2c: c->IXL=_z80_inc(c->IXL); return; // INC IXL
        case 0x2d: c->IXL=_z80_dec(c->IXL); return; // DEC IXL
        case 0x2e: c->IXL=_RD(c->PC++); return; // LD IXL,n
        case 0x2f: _z80_cpl(c); return; // CPL
        case 0x30: _z80_jr_cc(c,!(c->F&Z80_CF)) return; // JR NC,d
        case 0x31: _IMM16(); c->SP=c->WZ; return; // LD SP,nn
        case 0x32: _IMM16();_WR(c->WZ++,c->A);c->W=c->A; return; // LD (nn),A
        case 0x33: c->SP++; return; // INC SP
        case 0x34: { uint16_t a=c->WZ=c->IX+_RD(c->PC++); _WR(a,_z80_inc(_RD(a))); } return; // INC (IX+d)
        case 0x35: { uint16_t a=c->WZ=c->IX+_RD(c->PC++); _WR(a,_z80_dec(_RD(a))); } return; // DEC (IX+d)
        case 0x36: { uint16_t a=c->WZ=c->IX+_RD(c->PC++); _WR(a,_RD(c->PC++)); } return; // LD (IX+d),n
        case 0x37: _z80_scf(c); return; // SCF
        case 0x38: _z80_jr_cc(c,(c->F&Z80_CF)) return; // JR C,d
        case 0x39: c->IX=_z80_add16(c,c->IX,c->SP); return; // ADD IX,SP
        case 0x3a: _IMM16();c->A=_RD(c->WZ++); return; // LD A,(nn)
        case 0x3b: c->SP--; return; // DEC SP
        case 0x3c: c->A=_z80_inc(c->A); return; // INC A
        case 0x3d: c->A=_z80_dec(c->A); return; // DEC A
        case 0x3e: c->A=_RD(c->PC++); return; // LD A,n
        case 0x3f: _z80_ccf(c); return; // CCF
        case 0x40: c->B=c->B; return; // LD B,B
        case 0x41: c->B=c->C; return; // LD B,C
        case 0x42: c->B=c->D; return; // LD B,D
        case 0x43: c->B=c->E; return; // LD B,E
        case 0x44: c->B=c->IXH; return; // LD B,IXH
        case 0x45: c->B=c->IXL; return; // LD B,IXL
        case 0x46: { uint16_t a=c->WZ=c->IX+_RD(c->PC++); c->B=_RD(a); }; return; // LD B,(IX+d)
        case 0x47: c->B=c->A; return; // LD B,A
        case 0x48: c->C=c->B; return; // LD C,B
        case 0x49: c->C=c->C; return; // LD C,C
        case 0x4a: c->C=c->D; return; // LD C,D
        case 0x4b: c->C=c->E; return; // LD C,E
        case 0x4c: c->C=c->IXH; return; // LD C,IXH
        case 0x4d: c->C=c->IXL; return; // LD C,IXL
        case 0x4e: { uint16_t a=c->WZ=c->IX+_RD(c->PC++); c->C=_RD(a); }; return; // LD C,(IX+d)
        case 0x4f: c->C=c->A; return; // LD C,A
        case 0x50: c->D=c->B; return; // LD D,B
        case 0x51: c->D=c->C; return; // LD D,C
        case 0x52: c->D=c->D; return; // LD D,D
        case 0x53: c->D=c->E; return; // LD D,E
        case 0x54: c->D=c->IXH; return; // LD D,IXH
        case 0x55: c->D=c->IXL; return; // LD D,IXL
        case 0x56: { uint16_t a=c->WZ=c->IX+_RD(c->PC++); c->D=_RD(a); }; return; // LD D,(IX+d)
        case 0x57: c->D=c->A; return; // LD D,A
        case 0x58: c->E=c->B; return; // LD E,B
        case 0x59: c->E=c->C; return; // LD E,C
        case 0x5a: c->E=c->D; return; // LD E,D
        case 0x5b: c->E=c->E; return; // LD E,E
        case 0x5c: c->E=c->IXH; return; // LD E,IXH
        case 0x5d: c->E=c->IXL; return; // LD E,IXL
        case 0x5e: { uint16_t a=c->WZ=c->IX+_RD(c->PC++); c->E=_RD(a); }; return; // LD E,(IX+d)
        case 0x5f: c->E=c->A; return; // LD E,A
        case 0x60: c->IXH=c->B; return; // LD IXH,B
        case 0x61: c->IXH=c->C; return; // LD IXH,C
        case 0x62: c->IXH=c->D; return; // LD IXH,D
        case 0x63: c->IXH=c->E; return; // LD IXH,E
        case 0x64: c->IXH=c->IXH; return; // LD IXH,IXH
        case 0x65: c->IXH=c->IXL; return; // LD IXH,IXL
        case 0x66: { uint16_t a=c->WZ=c->IX+_RD(c->PC++); c->H=_RD(a); }; return; // LD H,(IX+d)
        case 0x67: c->IXH=c->A; return; // LD IXH,A
        case 0x68: c->IXL=c->B; return; // LD IXL,B
        case 0x69: c->IXL=c->C; return; // LD IXL,C
        case 0x6a: c->IXL=c->D; return; // LD IXL,D
        case 0x6b: c->IXL=c->E; return; // LD IXL,E
        case 0x6c: c->IXL=c->IXH; return; // LD IXL,IXH
        case 0x6d: c->IXL=c->IXL; return; // LD IXL,IXL
        case 0x6e: { uint16_t a=c->WZ=c->IX+_RD(c->PC++); c->L=_RD(a); }; return; // LD L,(IX+d)
        case 0x6f: c->IXL=c->A; return; // LD IXL,A
        case 0x70: { uint16_t a=c->WZ=c->IX+_RD(c->PC++); _WR(a,c->B); }; return; // LD (IX+d),B
        case 0x71: { uint16_t a=c->WZ=c->IX+_RD(c->PC++); _WR(a,c->C); }; return; // LD (IX+d),C
        case 0x72: { uint16_t a=c->WZ=c->IX+_RD(c->PC++); _WR(a,c->D); }; return; // LD (IX+d),D
        case 0x73: { uint16_t a=c->WZ=c->IX+_RD(c->PC++); _WR(a,c->E); }; return; // LD (IX+d),E
        case 0x74: { uint16_t a=c->WZ=c->IX+_RD(c->PC++); _WR(a,c->H); }; return; // LD (IX+d),H
        case 0x75: { uint16_t a=c->WZ=c->IX+_RD(c->PC++); _WR(a,c->L); }; return; // LD (IX+d),L
        case 0x76: _z80_halt(c); return; // HALT
        case 0x77: { uint16_t a=c->WZ=c->IX+_RD(c->PC++); _WR(a,c->A); }; return; // LD (IX+d),A
        case 0x78: c->A=c->B; return; // LD A,B
        case 0x79: c->A=c->C; return; // LD A,C
        case 0x7a: c->A=c->D; return; // LD A,D
        case 0x7b: c->A=c->E; return; // LD A,E
        case 0x7c: c->A=c->IXH; return; // LD A,IXH
        case 0x7d: c->A=c->IXL; return; // LD A,IXL
        case 0x7e: { uint16_t a=c->WZ=c->IX+_RD(c->PC++); c->A=_RD(a); }; return; // LD A,(IX+d)
        case 0x7f: c->A=c->A; return; // LD A,A
        case 0x80: _z80_add(c->B); return; // ADD B
        case 0x81: _z80_add(c->C); return; // ADD C
        case 0x82: _z80_add(c->D); return; // ADD D
        case 0x83: _z80_add(c->E); return; // ADD E
        case 0x84: _z80_add(c->IXH); return; // ADD IXH
        case 0x85: _z80_add(c->IXL); return; // ADD IXL
        case 0x86: { uint16_t a=c->WZ=c->IX+_RD(c->PC++); _z80_add(_RD(a)); }; return; // ADD (IX+d)
        case 0x87: _z80_add(c->A); return; // ADD A
        case 0x88: _z80_adc(c->B); return; // ADC B
        case 0x89: _z80_adc(c->C); return; // ADC C
        case 0x8a: _z80_adc(c->D); return; // ADC D
        case 0x8b: _z80_adc(c->E); return; // ADC E
        case 0x8c: _z80_adc(c->IXH); return; // ADC IXH
        case 0x8d: _z80_adc(c->IXL); return; // ADC IXL
        case 0x8e: { uint16_t a=c->WZ=c->IX+_RD(c->PC++); _z80_adc(_RD(a)); }; return; // ADC (IX+d)
        case 0x8f: _z80_adc(c->A); return; // ADC A
        case 0x90: _z80_sub(c->B); return; // SUB B
        case 0x91: _z80_sub(c->C); return; // SUB C
        case 0x92: _z80_sub(c->D); return; // SUB D
        case 0x93: _z80_sub(c->E); return; // SUB E
        case 0x94: _z80_sub(c->IXH); return; // SUB IXH
        case 0x95: _z80_sub(c->IXL); return; // SUB IXL
        case 0x96: { uint16_t a=c->WZ=c->IX+_RD(c->PC++); _z80_sub(_RD(a)); }; return; // SUB (IX+d)
        case 0x97: _z80_sub(c->A); return; // SUB A
        case 0x98: _z80_sbc(c->B); return; // SBC B
        case 0x99: _z80_sbc(c->C); return; // SBC C
        case 0x9a: _z80_sbc(c->D); return; // SBC D
        case 0x9b: _z80_sbc(c->E); return; // SBC E
        case 0x9c: _z80_sbc(c->IXH); return; // SBC IXH
        case 0x9d: _z80_sbc(c->IXL); return; // SBC IXL
        case 0x9e: { uint16_t a=c->WZ=c->IX+_RD(c->PC++); _z80_sbc(_RD(a)); }; return; // SBC (IX+d)
        case 0x9f: _z80_sbc(c->A); return; // SBC A
        case 0xa0: _z80_and(c->B); return; // AND B
        case 0xa1: _z80_and(c->C); return; // AND C
        case 0xa2: _z80_and(c->D); return; // AND D
        case 0xa3: _z80_and(c->E); return; // AND E
        case 0xa4: _z80_and(c->IXH); return; // AND IXH
        case 0xa5: _z80_and(c->IXL); return; // AND IXL
        case 0xa6: { uint16_t a=c->WZ=c->IX+_RD(c->PC++); _z80_and(_RD(a)); }; return; // AND (IX+d)
        case 0xa7: _z80_and(c->A); return; // AND A
        case 0xa8: _z80_xor(c->B); return; // XOR B
        case 0xa9: _z80_xor(c->C); return; // XOR C
        case 0xaa: _z80_xor(c->D); return; // XOR D
        case 0xab: _z80_xor(c->E); return; // XOR E
        case 0xac: _z80_xor(c->IXH); return; // XOR IXH
        case 0xad: _z80_xor(c->IXL); return; // XOR IXL
        case 0xae: { uint16_t a=c->WZ=c->IX+_RD(c->PC++); _z80_xor(_RD(a)); }; return; // XOR (IX+d)
        case 0xaf: _z80_xor(c->A); return; // XOR A
        case 0xb0: _z80_or(c->B); return; // OR B
        case 0xb1: _z80_or(c->C); return; // OR C
        case 0xb2: _z80_or(c->D); return; // OR D
        case 0xb3: _z80_or(c->E); return; // OR E
        case 0xb4: _z80_or(c->IXH); return; // OR IXH
        case 0xb5: _z80_or(c->IXL); return; // OR IXL
        case 0xb6: { uint16_t a=c->WZ=c->IX+_RD(c->PC++); _z80_or(_RD(a)); }; return; // OR (IX+d)
        case 0xb7: _z80_or(c->A); return; // OR A
        case 0xb8: _z80_cp(c->B); return; // CP B
        case 0xb9: _z80_cp(c->C); return; // CP C
        case 0xba: _z80_cp(c->D); return; // CP D
        case 0xbb: _z80_cp(c->E); return; // CP E
        case 0xbc: _z80_cp(c->IXH); return; // CP IXH
        case 0xbd: _z80_cp(c->IXL); return; // CP IXL
        case 0xbe: { uint16_t a=c->WZ=c->IX+_RD(c->PC++); _z80_cp(_RD(a)); }; return; // CP (IX+d)
        case 0xbf: _z80_cp(c->A); return; // CP A
        case 0xc0: _z80_retcc(c,!(c->F)&Z80_ZF)); return; // RET NZ
        case 0xc1: uint8_t l=_RD(c->SP++);uint8_t h=_RD(c->SP++);c->BC=(h<<8)|l; return; // POP BC
        case 0xc2: _IMM16(); if (!(c->F)&Z80_ZF)) { c->PC=c->WZ; } return; // JP NZ,nn
        case 0xc3: _IMM16(); c->PC=c->WZ; return; // JP nn
        case 0xc4: _z80_callcc(c, !(c->F)&Z80_ZF)); return; // CALL NZ,nn
        case 0xc5: _WR(--c->SP,BC<<8); _WR(--c->SP,BC); return; // PUSH BC
        case 0xc6: _z80_add(_RD(c->PC++)); return; // ADD n
        case 0xc7: _z80_rst(0x0); return; // RST 0x0
        case 0xc8: _z80_retcc(c,(c->F)&Z80_ZF)); return; // RET Z
        case 0xc9: c->Z=_RD(c->SP++);c->W=_RD(c->SP++);c->PC=c->WZ; return; // RET
        case 0xca: _IMM16(); if ((c->F)&Z80_ZF)) { c->PC=c->WZ; } return; // JP Z,nn
        case 0xcb:
          { const int8_t d = _READ(_REG(PC++));
          switch (_z80_fetch(c)) {
            case 0x0: { uint16_t a=c->WZ=IX+d;; c->B=_z80_rlc(_RD(a)); _WR(a,c->B); } return; // RLC (IX+d),B
            case 0x1: { uint16_t a=c->WZ=IX+d;; c->C=_z80_rlc(_RD(a)); _WR(a,c->C); } return; // RLC (IX+d),C
            case 0x2: { uint16_t a=c->WZ=IX+d;; c->D=_z80_rlc(_RD(a)); _WR(a,c->D); } return; // RLC (IX+d),D
            case 0x3: { uint16_t a=c->WZ=IX+d;; c->E=_z80_rlc(_RD(a)); _WR(a,c->E); } return; // RLC (IX+d),E
            case 0x4: { uint16_t a=c->WZ=IX+d;; c->H=_z80_rlc(_RD(a)); _WR(a,c->H); } return; // RLC (IX+d),H
            case 0x5: { uint16_t a=c->WZ=IX+d;; c->L=_z80_rlc(_RD(a)); _WR(a,c->L); } return; // RLC (IX+d),L
            case 0x6: { uint16_t a=c->WZ=IX+d;; _WR(a,_z80_rlc(_RD(a))); } return; // RLC (IX+d)
            case 0x7: { uint16_t a=c->WZ=IX+d;; c->A=_z80_rlc(_RD(a)); _WR(a,c->A); } return; // RLC (IX+d),A
            case 0x8: { uint16_t a=c->WZ=IX+d;; c->B=_z80_rrc(_RD(a)); _WR(a,c->B); } return; // RRC (IX+d),B
            case 0x9: { uint16_t a=c->WZ=IX+d;; c->C=_z80_rrc(_RD(a)); _WR(a,c->C); } return; // RRC (IX+d),C
            case 0xa: { uint16_t a=c->WZ=IX+d;; c->D=_z80_rrc(_RD(a)); _WR(a,c->D); } return; // RRC (IX+d),D
            case 0xb: { uint16_t a=c->WZ=IX+d;; c->E=_z80_rrc(_RD(a)); _WR(a,c->E); } return; // RRC (IX+d),E
            case 0xc: { uint16_t a=c->WZ=IX+d;; c->H=_z80_rrc(_RD(a)); _WR(a,c->H); } return; // RRC (IX+d),H
            case 0xd: { uint16_t a=c->WZ=IX+d;; c->L=_z80_rrc(_RD(a)); _WR(a,c->L); } return; // RRC (IX+d),L
            case 0xe: { uint16_t a=c->WZ=IX+d;; _WR(a,_z80_rrc(_RD(a))); } return; // RRC (IX+d)
            case 0xf: { uint16_t a=c->WZ=IX+d;; c->A=_z80_rrc(_RD(a)); _WR(a,c->A); } return; // RRC (IX+d),A
            case 0x10: { uint16_t a=c->WZ=IX+d;; c->B=_z80_rl(_RD(a)); _WR(a,c->B); } return; // RL (IX+d),B
            case 0x11: { uint16_t a=c->WZ=IX+d;; c->C=_z80_rl(_RD(a)); _WR(a,c->C); } return; // RL (IX+d),C
            case 0x12: { uint16_t a=c->WZ=IX+d;; c->D=_z80_rl(_RD(a)); _WR(a,c->D); } return; // RL (IX+d),D
            case 0x13: { uint16_t a=c->WZ=IX+d;; c->E=_z80_rl(_RD(a)); _WR(a,c->E); } return; // RL (IX+d),E
            case 0x14: { uint16_t a=c->WZ=IX+d;; c->H=_z80_rl(_RD(a)); _WR(a,c->H); } return; // RL (IX+d),H
            case 0x15: { uint16_t a=c->WZ=IX+d;; c->L=_z80_rl(_RD(a)); _WR(a,c->L); } return; // RL (IX+d),L
            case 0x16: { uint16_t a=c->WZ=IX+d;; _WR(a,_z80_rl(_RD(a))); } return; // RL (IX+d)
            case 0x17: { uint16_t a=c->WZ=IX+d;; c->A=_z80_rl(_RD(a)); _WR(a,c->A); } return; // RL (IX+d),A
            case 0x18: { uint16_t a=c->WZ=IX+d;; c->B=_z80_rr(_RD(a)); _WR(a,c->B); } return; // RR (IX+d),B
            case 0x19: { uint16_t a=c->WZ=IX+d;; c->C=_z80_rr(_RD(a)); _WR(a,c->C); } return; // RR (IX+d),C
            case 0x1a: { uint16_t a=c->WZ=IX+d;; c->D=_z80_rr(_RD(a)); _WR(a,c->D); } return; // RR (IX+d),D
            case 0x1b: { uint16_t a=c->WZ=IX+d;; c->E=_z80_rr(_RD(a)); _WR(a,c->E); } return; // RR (IX+d),E
            case 0x1c: { uint16_t a=c->WZ=IX+d;; c->H=_z80_rr(_RD(a)); _WR(a,c->H); } return; // RR (IX+d),H
            case 0x1d: { uint16_t a=c->WZ=IX+d;; c->L=_z80_rr(_RD(a)); _WR(a,c->L); } return; // RR (IX+d),L
            case 0x1e: { uint16_t a=c->WZ=IX+d;; _WR(a,_z80_rr(_RD(a))); } return; // RR (IX+d)
            case 0x1f: { uint16_t a=c->WZ=IX+d;; c->A=_z80_rr(_RD(a)); _WR(a,c->A); } return; // RR (IX+d),A
            case 0x20: { uint16_t a=c->WZ=IX+d;; c->B=_z80_sla(_RD(a)); _WR(a,c->B); } return; // SLA (IX+d),B
            case 0x21: { uint16_t a=c->WZ=IX+d;; c->C=_z80_sla(_RD(a)); _WR(a,c->C); } return; // SLA (IX+d),C
            case 0x22: { uint16_t a=c->WZ=IX+d;; c->D=_z80_sla(_RD(a)); _WR(a,c->D); } return; // SLA (IX+d),D
            case 0x23: { uint16_t a=c->WZ=IX+d;; c->E=_z80_sla(_RD(a)); _WR(a,c->E); } return; // SLA (IX+d),E
            case 0x24: { uint16_t a=c->WZ=IX+d;; c->H=_z80_sla(_RD(a)); _WR(a,c->H); } return; // SLA (IX+d),H
            case 0x25: { uint16_t a=c->WZ=IX+d;; c->L=_z80_sla(_RD(a)); _WR(a,c->L); } return; // SLA (IX+d),L
            case 0x26: { uint16_t a=c->WZ=IX+d;; _WR(a,_z80_sla(_RD(a))); } return; // SLA (IX+d)
            case 0x27: { uint16_t a=c->WZ=IX+d;; c->A=_z80_sla(_RD(a)); _WR(a,c->A); } return; // SLA (IX+d),A
            case 0x28: { uint16_t a=c->WZ=IX+d;; c->B=_z80_sra(_RD(a)); _WR(a,c->B); } return; // SRA (IX+d),B
            case 0x29: { uint16_t a=c->WZ=IX+d;; c->C=_z80_sra(_RD(a)); _WR(a,c->C); } return; // SRA (IX+d),C
            case 0x2a: { uint16_t a=c->WZ=IX+d;; c->D=_z80_sra(_RD(a)); _WR(a,c->D); } return; // SRA (IX+d),D
            case 0x2b: { uint16_t a=c->WZ=IX+d;; c->E=_z80_sra(_RD(a)); _WR(a,c->E); } return; // SRA (IX+d),E
            case 0x2c: { uint16_t a=c->WZ=IX+d;; c->H=_z80_sra(_RD(a)); _WR(a,c->H); } return; // SRA (IX+d),H
            case 0x2d: { uint16_t a=c->WZ=IX+d;; c->L=_z80_sra(_RD(a)); _WR(a,c->L); } return; // SRA (IX+d),L
            case 0x2e: { uint16_t a=c->WZ=IX+d;; _WR(a,_z80_sra(_RD(a))); } return; // SRA (IX+d)
            case 0x2f: { uint16_t a=c->WZ=IX+d;; c->A=_z80_sra(_RD(a)); _WR(a,c->A); } return; // SRA (IX+d),A
            case 0x30: { uint16_t a=c->WZ=IX+d;; c->B=_z80_sll(_RD(a)); _WR(a,c->B); } return; // SLL (IX+d),B
            case 0x31: { uint16_t a=c->WZ=IX+d;; c->C=_z80_sll(_RD(a)); _WR(a,c->C); } return; // SLL (IX+d),C
            case 0x32: { uint16_t a=c->WZ=IX+d;; c->D=_z80_sll(_RD(a)); _WR(a,c->D); } return; // SLL (IX+d),D
            case 0x33: { uint16_t a=c->WZ=IX+d;; c->E=_z80_sll(_RD(a)); _WR(a,c->E); } return; // SLL (IX+d),E
            case 0x34: { uint16_t a=c->WZ=IX+d;; c->H=_z80_sll(_RD(a)); _WR(a,c->H); } return; // SLL (IX+d),H
            case 0x35: { uint16_t a=c->WZ=IX+d;; c->L=_z80_sll(_RD(a)); _WR(a,c->L); } return; // SLL (IX+d),L
            case 0x36: { uint16_t a=c->WZ=IX+d;; _WR(a,_z80_sll(_RD(a))); } return; // SLL (IX+d)
            case 0x37: { uint16_t a=c->WZ=IX+d;; c->A=_z80_sll(_RD(a)); _WR(a,c->A); } return; // SLL (IX+d),A
            case 0x38: { uint16_t a=c->WZ=IX+d;; c->B=_z80_srl(_RD(a)); _WR(a,c->B); } return; // SRL (IX+d),B
            case 0x39: { uint16_t a=c->WZ=IX+d;; c->C=_z80_srl(_RD(a)); _WR(a,c->C); } return; // SRL (IX+d),C
            case 0x3a: { uint16_t a=c->WZ=IX+d;; c->D=_z80_srl(_RD(a)); _WR(a,c->D); } return; // SRL (IX+d),D
            case 0x3b: { uint16_t a=c->WZ=IX+d;; c->E=_z80_srl(_RD(a)); _WR(a,c->E); } return; // SRL (IX+d),E
            case 0x3c: { uint16_t a=c->WZ=IX+d;; c->H=_z80_srl(_RD(a)); _WR(a,c->H); } return; // SRL (IX+d),H
            case 0x3d: { uint16_t a=c->WZ=IX+d;; c->L=_z80_srl(_RD(a)); _WR(a,c->L); } return; // SRL (IX+d),L
            case 0x3e: { uint16_t a=c->WZ=IX+d;; _WR(a,_z80_srl(_RD(a))); } return; // SRL (IX+d)
            case 0x3f: { uint16_t a=c->WZ=IX+d;; c->A=_z80_srl(_RD(a)); _WR(a,c->A); } return; // SRL (IX+d),A
            case 0x40: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x1); } return; // BIT 0,(IX+d)
            case 0x41: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x1); } return; // BIT 0,(IX+d)
            case 0x42: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x1); } return; // BIT 0,(IX+d)
            case 0x43: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x1); } return; // BIT 0,(IX+d)
            case 0x44: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x1); } return; // BIT 0,(IX+d)
            case 0x45: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x1); } return; // BIT 0,(IX+d)
            case 0x46: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x1); } return; // BIT 0,(IX+d)
            case 0x47: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x1); } return; // BIT 0,(IX+d)
            case 0x48: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x2); } return; // BIT 1,(IX+d)
            case 0x49: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x2); } return; // BIT 1,(IX+d)
            case 0x4a: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x2); } return; // BIT 1,(IX+d)
            case 0x4b: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x2); } return; // BIT 1,(IX+d)
            case 0x4c: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x2); } return; // BIT 1,(IX+d)
            case 0x4d: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x2); } return; // BIT 1,(IX+d)
            case 0x4e: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x2); } return; // BIT 1,(IX+d)
            case 0x4f: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x2); } return; // BIT 1,(IX+d)
            case 0x50: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x4); } return; // BIT 2,(IX+d)
            case 0x51: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x4); } return; // BIT 2,(IX+d)
            case 0x52: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x4); } return; // BIT 2,(IX+d)
            case 0x53: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x4); } return; // BIT 2,(IX+d)
            case 0x54: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x4); } return; // BIT 2,(IX+d)
            case 0x55: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x4); } return; // BIT 2,(IX+d)
            case 0x56: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x4); } return; // BIT 2,(IX+d)
            case 0x57: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x4); } return; // BIT 2,(IX+d)
            case 0x58: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x8); } return; // BIT 3,(IX+d)
            case 0x59: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x8); } return; // BIT 3,(IX+d)
            case 0x5a: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x8); } return; // BIT 3,(IX+d)
            case 0x5b: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x8); } return; // BIT 3,(IX+d)
            case 0x5c: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x8); } return; // BIT 3,(IX+d)
            case 0x5d: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x8); } return; // BIT 3,(IX+d)
            case 0x5e: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x8); } return; // BIT 3,(IX+d)
            case 0x5f: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x8); } return; // BIT 3,(IX+d)
            case 0x60: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x10); } return; // BIT 4,(IX+d)
            case 0x61: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x10); } return; // BIT 4,(IX+d)
            case 0x62: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x10); } return; // BIT 4,(IX+d)
            case 0x63: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x10); } return; // BIT 4,(IX+d)
            case 0x64: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x10); } return; // BIT 4,(IX+d)
            case 0x65: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x10); } return; // BIT 4,(IX+d)
            case 0x66: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x10); } return; // BIT 4,(IX+d)
            case 0x67: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x10); } return; // BIT 4,(IX+d)
            case 0x68: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x20); } return; // BIT 5,(IX+d)
            case 0x69: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x20); } return; // BIT 5,(IX+d)
            case 0x6a: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x20); } return; // BIT 5,(IX+d)
            case 0x6b: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x20); } return; // BIT 5,(IX+d)
            case 0x6c: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x20); } return; // BIT 5,(IX+d)
            case 0x6d: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x20); } return; // BIT 5,(IX+d)
            case 0x6e: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x20); } return; // BIT 5,(IX+d)
            case 0x6f: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x20); } return; // BIT 5,(IX+d)
            case 0x70: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x40); } return; // BIT 6,(IX+d)
            case 0x71: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x40); } return; // BIT 6,(IX+d)
            case 0x72: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x40); } return; // BIT 6,(IX+d)
            case 0x73: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x40); } return; // BIT 6,(IX+d)
            case 0x74: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x40); } return; // BIT 6,(IX+d)
            case 0x75: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x40); } return; // BIT 6,(IX+d)
            case 0x76: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x40); } return; // BIT 6,(IX+d)
            case 0x77: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x40); } return; // BIT 6,(IX+d)
            case 0x78: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x80); } return; // BIT 7,(IX+d)
            case 0x79: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x80); } return; // BIT 7,(IX+d)
            case 0x7a: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x80); } return; // BIT 7,(IX+d)
            case 0x7b: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x80); } return; // BIT 7,(IX+d)
            case 0x7c: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x80); } return; // BIT 7,(IX+d)
            case 0x7d: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x80); } return; // BIT 7,(IX+d)
            case 0x7e: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x80); } return; // BIT 7,(IX+d)
            case 0x7f: { uint16_t a=c->WZ=IX+d;; _z80_ibit(c,_RD(a),0x80); } return; // BIT 7,(IX+d)
            case 0x80: { uint16_t a=c->WZ=IX+d;; c->B=_RD(a)&~0x1; _WR(a,c->B); } return; // RES 0,(IX+d),B
            case 0x81: { uint16_t a=c->WZ=IX+d;; c->C=_RD(a)&~0x1; _WR(a,c->C); } return; // RES 0,(IX+d),C
            case 0x82: { uint16_t a=c->WZ=IX+d;; c->D=_RD(a)&~0x1; _WR(a,c->D); } return; // RES 0,(IX+d),D
            case 0x83: { uint16_t a=c->WZ=IX+d;; c->E=_RD(a)&~0x1; _WR(a,c->E); } return; // RES 0,(IX+d),E
            case 0x84: { uint16_t a=c->WZ=IX+d;; c->H=_RD(a)&~0x1; _WR(a,c->H); } return; // RES 0,(IX+d),H
            case 0x85: { uint16_t a=c->WZ=IX+d;; c->L=_RD(a)&~0x1; _WR(a,c->L); } return; // RES 0,(IX+d),L
            case 0x86: { uint16_t a=c->WZ=IX+d;; _WR(a,_RD(a)&~0x1); } return; // RES 0,(IX+d)
            case 0x87: { uint16_t a=c->WZ=IX+d;; c->A=_RD(a)&~0x1; _WR(a,c->A); } return; // RES 0,(IX+d),A
            case 0x88: { uint16_t a=c->WZ=IX+d;; c->B=_RD(a)&~0x2; _WR(a,c->B); } return; // RES 1,(IX+d),B
            case 0x89: { uint16_t a=c->WZ=IX+d;; c->C=_RD(a)&~0x2; _WR(a,c->C); } return; // RES 1,(IX+d),C
            case 0x8a: { uint16_t a=c->WZ=IX+d;; c->D=_RD(a)&~0x2; _WR(a,c->D); } return; // RES 1,(IX+d),D
            case 0x8b: { uint16_t a=c->WZ=IX+d;; c->E=_RD(a)&~0x2; _WR(a,c->E); } return; // RES 1,(IX+d),E
            case 0x8c: { uint16_t a=c->WZ=IX+d;; c->H=_RD(a)&~0x2; _WR(a,c->H); } return; // RES 1,(IX+d),H
            case 0x8d: { uint16_t a=c->WZ=IX+d;; c->L=_RD(a)&~0x2; _WR(a,c->L); } return; // RES 1,(IX+d),L
            case 0x8e: { uint16_t a=c->WZ=IX+d;; _WR(a,_RD(a)&~0x2); } return; // RES 1,(IX+d)
            case 0x8f: { uint16_t a=c->WZ=IX+d;; c->A=_RD(a)&~0x2; _WR(a,c->A); } return; // RES 1,(IX+d),A
            case 0x90: { uint16_t a=c->WZ=IX+d;; c->B=_RD(a)&~0x4; _WR(a,c->B); } return; // RES 2,(IX+d),B
            case 0x91: { uint16_t a=c->WZ=IX+d;; c->C=_RD(a)&~0x4; _WR(a,c->C); } return; // RES 2,(IX+d),C
            case 0x92: { uint16_t a=c->WZ=IX+d;; c->D=_RD(a)&~0x4; _WR(a,c->D); } return; // RES 2,(IX+d),D
            case 0x93: { uint16_t a=c->WZ=IX+d;; c->E=_RD(a)&~0x4; _WR(a,c->E); } return; // RES 2,(IX+d),E
            case 0x94: { uint16_t a=c->WZ=IX+d;; c->H=_RD(a)&~0x4; _WR(a,c->H); } return; // RES 2,(IX+d),H
            case 0x95: { uint16_t a=c->WZ=IX+d;; c->L=_RD(a)&~0x4; _WR(a,c->L); } return; // RES 2,(IX+d),L
            case 0x96: { uint16_t a=c->WZ=IX+d;; _WR(a,_RD(a)&~0x4); } return; // RES 2,(IX+d)
            case 0x97: { uint16_t a=c->WZ=IX+d;; c->A=_RD(a)&~0x4; _WR(a,c->A); } return; // RES 2,(IX+d),A
            case 0x98: { uint16_t a=c->WZ=IX+d;; c->B=_RD(a)&~0x8; _WR(a,c->B); } return; // RES 3,(IX+d),B
            case 0x99: { uint16_t a=c->WZ=IX+d;; c->C=_RD(a)&~0x8; _WR(a,c->C); } return; // RES 3,(IX+d),C
            case 0x9a: { uint16_t a=c->WZ=IX+d;; c->D=_RD(a)&~0x8; _WR(a,c->D); } return; // RES 3,(IX+d),D
            case 0x9b: { uint16_t a=c->WZ=IX+d;; c->E=_RD(a)&~0x8; _WR(a,c->E); } return; // RES 3,(IX+d),E
            case 0x9c: { uint16_t a=c->WZ=IX+d;; c->H=_RD(a)&~0x8; _WR(a,c->H); } return; // RES 3,(IX+d),H
            case 0x9d: { uint16_t a=c->WZ=IX+d;; c->L=_RD(a)&~0x8; _WR(a,c->L); } return; // RES 3,(IX+d),L
            case 0x9e: { uint16_t a=c->WZ=IX+d;; _WR(a,_RD(a)&~0x8); } return; // RES 3,(IX+d)
            case 0x9f: { uint16_t a=c->WZ=IX+d;; c->A=_RD(a)&~0x8; _WR(a,c->A); } return; // RES 3,(IX+d),A
            case 0xa0: { uint16_t a=c->WZ=IX+d;; c->B=_RD(a)&~0x10; _WR(a,c->B); } return; // RES 4,(IX+d),B
            case 0xa1: { uint16_t a=c->WZ=IX+d;; c->C=_RD(a)&~0x10; _WR(a,c->C); } return; // RES 4,(IX+d),C
            case 0xa2: { uint16_t a=c->WZ=IX+d;; c->D=_RD(a)&~0x10; _WR(a,c->D); } return; // RES 4,(IX+d),D
            case 0xa3: { uint16_t a=c->WZ=IX+d;; c->E=_RD(a)&~0x10; _WR(a,c->E); } return; // RES 4,(IX+d),E
            case 0xa4: { uint16_t a=c->WZ=IX+d;; c->H=_RD(a)&~0x10; _WR(a,c->H); } return; // RES 4,(IX+d),H
            case 0xa5: { uint16_t a=c->WZ=IX+d;; c->L=_RD(a)&~0x10; _WR(a,c->L); } return; // RES 4,(IX+d),L
            case 0xa6: { uint16_t a=c->WZ=IX+d;; _WR(a,_RD(a)&~0x10); } return; // RES 4,(IX+d)
            case 0xa7: { uint16_t a=c->WZ=IX+d;; c->A=_RD(a)&~0x10; _WR(a,c->A); } return; // RES 4,(IX+d),A
            case 0xa8: { uint16_t a=c->WZ=IX+d;; c->B=_RD(a)&~0x20; _WR(a,c->B); } return; // RES 5,(IX+d),B
            case 0xa9: { uint16_t a=c->WZ=IX+d;; c->C=_RD(a)&~0x20; _WR(a,c->C); } return; // RES 5,(IX+d),C
            case 0xaa: { uint16_t a=c->WZ=IX+d;; c->D=_RD(a)&~0x20; _WR(a,c->D); } return; // RES 5,(IX+d),D
            case 0xab: { uint16_t a=c->WZ=IX+d;; c->E=_RD(a)&~0x20; _WR(a,c->E); } return; // RES 5,(IX+d),E
            case 0xac: { uint16_t a=c->WZ=IX+d;; c->H=_RD(a)&~0x20; _WR(a,c->H); } return; // RES 5,(IX+d),H
            case 0xad: { uint16_t a=c->WZ=IX+d;; c->L=_RD(a)&~0x20; _WR(a,c->L); } return; // RES 5,(IX+d),L
            case 0xae: { uint16_t a=c->WZ=IX+d;; _WR(a,_RD(a)&~0x20); } return; // RES 5,(IX+d)
            case 0xaf: { uint16_t a=c->WZ=IX+d;; c->A=_RD(a)&~0x20; _WR(a,c->A); } return; // RES 5,(IX+d),A
            case 0xb0: { uint16_t a=c->WZ=IX+d;; c->B=_RD(a)&~0x40; _WR(a,c->B); } return; // RES 6,(IX+d),B
            case 0xb1: { uint16_t a=c->WZ=IX+d;; c->C=_RD(a)&~0x40; _WR(a,c->C); } return; // RES 6,(IX+d),C
            case 0xb2: { uint16_t a=c->WZ=IX+d;; c->D=_RD(a)&~0x40; _WR(a,c->D); } return; // RES 6,(IX+d),D
            case 0xb3: { uint16_t a=c->WZ=IX+d;; c->E=_RD(a)&~0x40; _WR(a,c->E); } return; // RES 6,(IX+d),E
            case 0xb4: { uint16_t a=c->WZ=IX+d;; c->H=_RD(a)&~0x40; _WR(a,c->H); } return; // RES 6,(IX+d),H
            case 0xb5: { uint16_t a=c->WZ=IX+d;; c->L=_RD(a)&~0x40; _WR(a,c->L); } return; // RES 6,(IX+d),L
            case 0xb6: { uint16_t a=c->WZ=IX+d;; _WR(a,_RD(a)&~0x40); } return; // RES 6,(IX+d)
            case 0xb7: { uint16_t a=c->WZ=IX+d;; c->A=_RD(a)&~0x40; _WR(a,c->A); } return; // RES 6,(IX+d),A
            case 0xb8: { uint16_t a=c->WZ=IX+d;; c->B=_RD(a)&~0x80; _WR(a,c->B); } return; // RES 7,(IX+d),B
            case 0xb9: { uint16_t a=c->WZ=IX+d;; c->C=_RD(a)&~0x80; _WR(a,c->C); } return; // RES 7,(IX+d),C
            case 0xba: { uint16_t a=c->WZ=IX+d;; c->D=_RD(a)&~0x80; _WR(a,c->D); } return; // RES 7,(IX+d),D
            case 0xbb: { uint16_t a=c->WZ=IX+d;; c->E=_RD(a)&~0x80; _WR(a,c->E); } return; // RES 7,(IX+d),E
            case 0xbc: { uint16_t a=c->WZ=IX+d;; c->H=_RD(a)&~0x80; _WR(a,c->H); } return; // RES 7,(IX+d),H
            case 0xbd: { uint16_t a=c->WZ=IX+d;; c->L=_RD(a)&~0x80; _WR(a,c->L); } return; // RES 7,(IX+d),L
            case 0xbe: { uint16_t a=c->WZ=IX+d;; _WR(a,_RD(a)&~0x80); } return; // RES 7,(IX+d)
            case 0xbf: { uint16_t a=c->WZ=IX+d;; c->A=_RD(a)&~0x80; _WR(a,c->A); } return; // RES 7,(IX+d),A
            case 0xc0: { uint16_t a=c->WZ=IX+d;; c->B=_RD(a)|0x1; _WR(a,c->B);} return; // SET 0,(IX+d),B
            case 0xc1: { uint16_t a=c->WZ=IX+d;; c->C=_RD(a)|0x1; _WR(a,c->C);} return; // SET 0,(IX+d),C
            case 0xc2: { uint16_t a=c->WZ=IX+d;; c->D=_RD(a)|0x1; _WR(a,c->D);} return; // SET 0,(IX+d),D
            case 0xc3: { uint16_t a=c->WZ=IX+d;; c->E=_RD(a)|0x1; _WR(a,c->E);} return; // SET 0,(IX+d),E
            case 0xc4: { uint16_t a=c->WZ=IX+d;; c->H=_RD(a)|0x1; _WR(a,c->IXH);} return; // SET 0,(IX+d),H
            case 0xc5: { uint16_t a=c->WZ=IX+d;; c->L=_RD(a)|0x1; _WR(a,c->IXL);} return; // SET 0,(IX+d),L
            case 0xc6: { uint16_t a=c->WZ=IX+d;; _WR(a,_RD(a)|0x1);} return; // SET 0,(IX+d)
            case 0xc7: { uint16_t a=c->WZ=IX+d;; c->A=_RD(a)|0x1; _WR(a,c->A);} return; // SET 0,(IX+d),A
            case 0xc8: { uint16_t a=c->WZ=IX+d;; c->B=_RD(a)|0x2; _WR(a,c->B);} return; // SET 1,(IX+d),B
            case 0xc9: { uint16_t a=c->WZ=IX+d;; c->C=_RD(a)|0x2; _WR(a,c->C);} return; // SET 1,(IX+d),C
            case 0xca: { uint16_t a=c->WZ=IX+d;; c->D=_RD(a)|0x2; _WR(a,c->D);} return; // SET 1,(IX+d),D
            case 0xcb: { uint16_t a=c->WZ=IX+d;; c->E=_RD(a)|0x2; _WR(a,c->E);} return; // SET 1,(IX+d),E
            case 0xcc: { uint16_t a=c->WZ=IX+d;; c->H=_RD(a)|0x2; _WR(a,c->IXH);} return; // SET 1,(IX+d),H
            case 0xcd: { uint16_t a=c->WZ=IX+d;; c->L=_RD(a)|0x2; _WR(a,c->IXL);} return; // SET 1,(IX+d),L
            case 0xce: { uint16_t a=c->WZ=IX+d;; _WR(a,_RD(a)|0x2);} return; // SET 1,(IX+d)
            case 0xcf: { uint16_t a=c->WZ=IX+d;; c->A=_RD(a)|0x2; _WR(a,c->A);} return; // SET 1,(IX+d),A
            case 0xd0: { uint16_t a=c->WZ=IX+d;; c->B=_RD(a)|0x4; _WR(a,c->B);} return; // SET 2,(IX+d),B
            case 0xd1: { uint16_t a=c->WZ=IX+d;; c->C=_RD(a)|0x4; _WR(a,c->C);} return; // SET 2,(IX+d),C
            case 0xd2: { uint16_t a=c->WZ=IX+d;; c->D=_RD(a)|0x4; _WR(a,c->D);} return; // SET 2,(IX+d),D
            case 0xd3: { uint16_t a=c->WZ=IX+d;; c->E=_RD(a)|0x4; _WR(a,c->E);} return; // SET 2,(IX+d),E
            case 0xd4: { uint16_t a=c->WZ=IX+d;; c->H=_RD(a)|0x4; _WR(a,c->IXH);} return; // SET 2,(IX+d),H
            case 0xd5: { uint16_t a=c->WZ=IX+d;; c->L=_RD(a)|0x4; _WR(a,c->IXL);} return; // SET 2,(IX+d),L
            case 0xd6: { uint16_t a=c->WZ=IX+d;; _WR(a,_RD(a)|0x4);} return; // SET 2,(IX+d)
            case 0xd7: { uint16_t a=c->WZ=IX+d;; c->A=_RD(a)|0x4; _WR(a,c->A);} return; // SET 2,(IX+d),A
            case 0xd8: { uint16_t a=c->WZ=IX+d;; c->B=_RD(a)|0x8; _WR(a,c->B);} return; // SET 3,(IX+d),B
            case 0xd9: { uint16_t a=c->WZ=IX+d;; c->C=_RD(a)|0x8; _WR(a,c->C);} return; // SET 3,(IX+d),C
            case 0xda: { uint16_t a=c->WZ=IX+d;; c->D=_RD(a)|0x8; _WR(a,c->D);} return; // SET 3,(IX+d),D
            case 0xdb: { uint16_t a=c->WZ=IX+d;; c->E=_RD(a)|0x8; _WR(a,c->E);} return; // SET 3,(IX+d),E
            case 0xdc: { uint16_t a=c->WZ=IX+d;; c->H=_RD(a)|0x8; _WR(a,c->IXH);} return; // SET 3,(IX+d),H
            case 0xdd: { uint16_t a=c->WZ=IX+d;; c->L=_RD(a)|0x8; _WR(a,c->IXL);} return; // SET 3,(IX+d),L
            case 0xde: { uint16_t a=c->WZ=IX+d;; _WR(a,_RD(a)|0x8);} return; // SET 3,(IX+d)
            case 0xdf: { uint16_t a=c->WZ=IX+d;; c->A=_RD(a)|0x8; _WR(a,c->A);} return; // SET 3,(IX+d),A
            case 0xe0: { uint16_t a=c->WZ=IX+d;; c->B=_RD(a)|0x10; _WR(a,c->B);} return; // SET 4,(IX+d),B
            case 0xe1: { uint16_t a=c->WZ=IX+d;; c->C=_RD(a)|0x10; _WR(a,c->C);} return; // SET 4,(IX+d),C
            case 0xe2: { uint16_t a=c->WZ=IX+d;; c->D=_RD(a)|0x10; _WR(a,c->D);} return; // SET 4,(IX+d),D
            case 0xe3: { uint16_t a=c->WZ=IX+d;; c->E=_RD(a)|0x10; _WR(a,c->E);} return; // SET 4,(IX+d),E
            case 0xe4: { uint16_t a=c->WZ=IX+d;; c->H=_RD(a)|0x10; _WR(a,c->IXH);} return; // SET 4,(IX+d),H
            case 0xe5: { uint16_t a=c->WZ=IX+d;; c->L=_RD(a)|0x10; _WR(a,c->IXL);} return; // SET 4,(IX+d),L
            case 0xe6: { uint16_t a=c->WZ=IX+d;; _WR(a,_RD(a)|0x10);} return; // SET 4,(IX+d)
            case 0xe7: { uint16_t a=c->WZ=IX+d;; c->A=_RD(a)|0x10; _WR(a,c->A);} return; // SET 4,(IX+d),A
            case 0xe8: { uint16_t a=c->WZ=IX+d;; c->B=_RD(a)|0x20; _WR(a,c->B);} return; // SET 5,(IX+d),B
            case 0xe9: { uint16_t a=c->WZ=IX+d;; c->C=_RD(a)|0x20; _WR(a,c->C);} return; // SET 5,(IX+d),C
            case 0xea: { uint16_t a=c->WZ=IX+d;; c->D=_RD(a)|0x20; _WR(a,c->D);} return; // SET 5,(IX+d),D
            case 0xeb: { uint16_t a=c->WZ=IX+d;; c->E=_RD(a)|0x20; _WR(a,c->E);} return; // SET 5,(IX+d),E
            case 0xec: { uint16_t a=c->WZ=IX+d;; c->H=_RD(a)|0x20; _WR(a,c->IXH);} return; // SET 5,(IX+d),H
            case 0xed: { uint16_t a=c->WZ=IX+d;; c->L=_RD(a)|0x20; _WR(a,c->IXL);} return; // SET 5,(IX+d),L
            case 0xee: { uint16_t a=c->WZ=IX+d;; _WR(a,_RD(a)|0x20);} return; // SET 5,(IX+d)
            case 0xef: { uint16_t a=c->WZ=IX+d;; c->A=_RD(a)|0x20; _WR(a,c->A);} return; // SET 5,(IX+d),A
            case 0xf0: { uint16_t a=c->WZ=IX+d;; c->B=_RD(a)|0x40; _WR(a,c->B);} return; // SET 6,(IX+d),B
            case 0xf1: { uint16_t a=c->WZ=IX+d;; c->C=_RD(a)|0x40; _WR(a,c->C);} return; // SET 6,(IX+d),C
            case 0xf2: { uint16_t a=c->WZ=IX+d;; c->D=_RD(a)|0x40; _WR(a,c->D);} return; // SET 6,(IX+d),D
            case 0xf3: { uint16_t a=c->WZ=IX+d;; c->E=_RD(a)|0x40; _WR(a,c->E);} return; // SET 6,(IX+d),E
            case 0xf4: { uint16_t a=c->WZ=IX+d;; c->H=_RD(a)|0x40; _WR(a,c->IXH);} return; // SET 6,(IX+d),H
            case 0xf5: { uint16_t a=c->WZ=IX+d;; c->L=_RD(a)|0x40; _WR(a,c->IXL);} return; // SET 6,(IX+d),L
            case 0xf6: { uint16_t a=c->WZ=IX+d;; _WR(a,_RD(a)|0x40);} return; // SET 6,(IX+d)
            case 0xf7: { uint16_t a=c->WZ=IX+d;; c->A=_RD(a)|0x40; _WR(a,c->A);} return; // SET 6,(IX+d),A
            case 0xf8: { uint16_t a=c->WZ=IX+d;; c->B=_RD(a)|0x80; _WR(a,c->B);} return; // SET 7,(IX+d),B
            case 0xf9: { uint16_t a=c->WZ=IX+d;; c->C=_RD(a)|0x80; _WR(a,c->C);} return; // SET 7,(IX+d),C
            case 0xfa: { uint16_t a=c->WZ=IX+d;; c->D=_RD(a)|0x80; _WR(a,c->D);} return; // SET 7,(IX+d),D
            case 0xfb: { uint16_t a=c->WZ=IX+d;; c->E=_RD(a)|0x80; _WR(a,c->E);} return; // SET 7,(IX+d),E
            case 0xfc: { uint16_t a=c->WZ=IX+d;; c->H=_RD(a)|0x80; _WR(a,c->IXH);} return; // SET 7,(IX+d),H
            case 0xfd: { uint16_t a=c->WZ=IX+d;; c->L=_RD(a)|0x80; _WR(a,c->IXL);} return; // SET 7,(IX+d),L
            case 0xfe: { uint16_t a=c->WZ=IX+d;; _WR(a,_RD(a)|0x80);} return; // SET 7,(IX+d)
            case 0xff: { uint16_t a=c->WZ=IX+d;; c->A=_RD(a)|0x80; _WR(a,c->A);} return; // SET 7,(IX+d),A
            default: return _INVALID_OPCODE(4);
          }
          break;
          }
        case 0xcc: _z80_callcc(c, (c->F)&Z80_ZF)); return; // CALL Z,nn
        case 0xcd: _z80_call(c); return; // CALL nn
        case 0xce: _z80_adc(_RD(c->PC++)); return; // ADC n
        case 0xcf: _z80_rst(0x8); return; // RST 0x8
        case 0xd0: _z80_retcc(c,!(c->F&Z80_CF)); return; // RET NC
        case 0xd1: uint8_t l=_RD(c->SP++);uint8_t h=_RD(c->SP++);c->DE=(h<<8)|l; return; // POP DE
        case 0xd2: _IMM16(); if (!(c->F&Z80_CF)) { c->PC=c->WZ; } return; // JP NC,nn
        case 0xd3: _z80_out(c, (c->A<<8)|_RD(c->PC++), c->A); return; // OUT (n),A
        case 0xd4: _z80_callcc(c, !(c->F&Z80_CF)); return; // CALL NC,nn
        case 0xd5: _WR(--c->SP,DE<<8); _WR(--c->SP,DE); return; // PUSH DE
        case 0xd6: _z80_sub(_RD(c->PC++)); return; // SUB n
        case 0xd7: _z80_rst(0x10); return; // RST 0x10
        case 0xd8: _z80_retcc(c,(c->F&Z80_CF)); return; // RET C
        case 0xd9: _SWP16(c->BC,c->BC_); _SWP16(c->DE,c->DE_); SWP16(c->HL,c->HL_); SWP16(c->WZ,c->WZ_); return; // EXX
        case 0xda: _IMM16(); if ((c->F&Z80_CF)) { c->PC=c->WZ; } return; // JP C,nn
        case 0xdb: c->A=_z80_in(c, (c->A<<8)|_RD(c->PC++)); return; // IN A,(n)
        case 0xdc: _z80_callcc(c, (c->F&Z80_CF)); return; // CALL C,nn
        case 0xde: _z80_sbc(_RD(c->PC++)); return; // SBC n
        case 0xdf: _z80_rst(0x18); return; // RST 0x18
        case 0xe0: _z80_retcc(c,!(c->F&Z80_PF)); return; // RET PO
        case 0xe1: uint8_t l=_RD(c->SP++);uint8_t h=_RD(c->SP++);c->IX=(h<<8)|l; return; // POP IX
        case 0xe2: _IMM16(); if (!(c->F&Z80_PF)) { c->PC=c->WZ; } return; // JP PO,nn
        case 0xe3: c->IX=_z80_exsp(c,c->IX); return; // EX (SP),IX
        case 0xe4: _z80_callcc(c, !(c->F&Z80_PF)); return; // CALL PO,nn
        case 0xe5: _WR(--c->SP,IX<<8); _WR(--c->SP,IX); return; // PUSH IX
        case 0xe6: _z80_and(_RD(c->PC++)); return; // AND n
        case 0xe7: _z80_rst(0x20); return; // RST 0x20
        case 0xe8: _z80_retcc(c,(c->F&Z80_PF)); return; // RET PE
        case 0xe9: c->PC=c->IX; return; // JP IX
        case 0xea: _IMM16(); if ((c->F&Z80_PF)) { c->PC=c->WZ; } return; // JP PE,nn
        case 0xeb: _SWP16(c->DE,c->HL); return; // EX DE,HL
        case 0xec: _z80_callcc(c, (c->F&Z80_PF)); return; // CALL PE,nn
        case 0xee: _z80_xor(_RD(c->PC++)); return; // XOR n
        case 0xef: _z80_rst(0x28); return; // RST 0x28
        case 0xf0: _z80_retcc(c,!(c->F&Z80_SF)); return; // RET P
        case 0xf1: uint8_t l=_RD(c->SP++);uint8_t h=_RD(c->SP++);c->AF=(h<<8)|l; return; // POP AF
        case 0xf2: _IMM16(); if (!(c->F&Z80_SF)) { c->PC=c->WZ; } return; // JP P,nn
        case 0xf3: _z80_di(); return; // DI
        case 0xf4: _z80_callcc(c, !(c->F&Z80_SF)); return; // CALL P,nn
        case 0xf5: _WR(--c->SP,AF<<8); _WR(--c->SP,AF); return; // PUSH AF
        case 0xf6: _z80_or(_RD(c->PC++)); return; // OR n
        case 0xf7: _z80_rst(0x30); return; // RST 0x30
        case 0xf8: _z80_retcc(c,(c->F&Z80_SF)); return; // RET M
        case 0xf9: _T(2);c->SP=c->IX; return; // LD SP,IX
        case 0xfa: _IMM16(); if ((c->F&Z80_SF)) { c->PC=c->WZ; } return; // JP M,nn
        case 0xfb: _z80_ei(); return; // EI
        case 0xfc: _z80_callcc(c, (c->F&Z80_SF)); return; // CALL M,nn
        case 0xfe: _z80_cp(_RD(c->PC++)); return; // CP n
        case 0xff: _z80_rst(0x38); return; // RST 0x38
        default: return _INVALID_OPCODE(2);
      }
      break;
    case 0xde: _z80_sbc(_RD(c->PC++)); return; // SBC n
    case 0xdf: _z80_rst(0x18); return; // RST 0x18
    case 0xe0: _z80_retcc(c,!(c->F&Z80_PF)); return; // RET PO
    case 0xe1: uint8_t l=_RD(c->SP++);uint8_t h=_RD(c->SP++);c->HL=(h<<8)|l; return; // POP HL
    case 0xe2: _IMM16(); if (!(c->F&Z80_PF)) { c->PC=c->WZ; } return; // JP PO,nn
    case 0xe3: c->HL=_z80_exsp(c,c->HL); return; // EX (SP),HL
    case 0xe4: _z80_callcc(c, !(c->F&Z80_PF)); return; // CALL PO,nn
    case 0xe5: _WR(--c->SP,HL<<8); _WR(--c->SP,HL); return; // PUSH HL
    case 0xe6: _z80_and(_RD(c->PC++)); return; // AND n
    case 0xe7: _z80_rst(0x20); return; // RST 0x20
    case 0xe8: _z80_retcc(c,(c->F&Z80_PF)); return; // RET PE
    case 0xe9: c->PC=c->HL; return; // JP HL
    case 0xea: _IMM16(); if ((c->F&Z80_PF)) { c->PC=c->WZ; } return; // JP PE,nn
    case 0xeb: _SWP16(c->DE,c->HL); return; // EX DE,HL
    case 0xec: _z80_callcc(c, (c->F&Z80_PF)); return; // CALL PE,nn
    case 0xed:
      switch (_z80_fetch(c)) {
        case 0x40: c->B=_z80_in(c, c->BC); c->F=c->szp[c->B]|(c->F&Z80_CF); return; // IN B,(C)
        case 0x41: _z80_out(c,c->BC,c->B); return; // OUT (C),B
        case 0x42: c->HL=_z80_sbc16(c->HL,c->BC); return; // SBC HL,BC
        case 0x43: _IMM16();_WR(c->WZ++,BC);_WR(c->WZ,BC>>8); return; // LD (nn),BC
        case 0x44: _z80_neg(c); return; // NEG
        case 0x46: c->IM=0; return; // IM 0
        case 0x47: c->I=c->A; return; // LD I,A
        case 0x48: c->C=_z80_in(c, c->BC); c->F=c->szp[c->C]|(c->F&Z80_CF); return; // IN C,(C)
        case 0x49: _z80_out(c,c->BC,c->C); return; // OUT (C),C
        case 0x4a: c->HL=_z80_adc16(c->HL,c->BC); return; // ADC HL,BC
        case 0x4b: _IMM16();uint8_t l=_RD(c->WZ++);uint8_t h=_RD(c->WZ);c->BC=(h<<8)|l; return; // LD BC,(nn)
        case 0x4c: _z80_neg(c); return; // NEG
        case 0x4d: _z80_reti(c); return; // RETI
        case 0x4e: c->IM=0; return; // IM 0
        case 0x4f: c->R=c->A; return; // LD R,A
        case 0x50: c->D=_z80_in(c, c->BC); c->F=c->szp[c->D]|(c->F&Z80_CF); return; // IN D,(C)
        case 0x51: _z80_out(c,c->BC,c->D); return; // OUT (C),D
        case 0x52: c->HL=_z80_sbc16(c->HL,c->DE); return; // SBC HL,DE
        case 0x53: _IMM16();_WR(c->WZ++,DE);_WR(c->WZ,DE>>8); return; // LD (nn),DE
        case 0x54: _z80_neg(c); return; // NEG
        case 0x56: c->IM=1; return; // IM 1
        case 0x57: c->A=c->I; c->F=_z80_sziff2(c->I,c->IFF2)|(c->F&Z80_CF); return; // LD A,I
        case 0x58: c->E=_z80_in(c, c->BC); c->F=c->szp[c->E]|(c->F&Z80_CF); return; // IN E,(C)
        case 0x59: _z80_out(c,c->BC,c->E); return; // OUT (C),E
        case 0x5a: c->HL=_z80_adc16(c->HL,c->DE); return; // ADC HL,DE
        case 0x5b: _IMM16();uint8_t l=_RD(c->WZ++);uint8_t h=_RD(c->WZ);c->DE=(h<<8)|l; return; // LD DE,(nn)
        case 0x5c: _z80_neg(c); return; // NEG
        case 0x5e: c->IM=2; return; // IM 2
        case 0x5f: c->A=c->R; c->F=_z80_sziff2(c->R,c->IFF2)|(c->F&Z80_CF); return; // LD A,R
        case 0x60: c->H=_z80_in(c, c->BC); c->F=c->szp[c->H]|(c->F&Z80_CF); return; // IN H,(C)
        case 0x61: _z80_out(c,c->BC,c->H); return; // OUT (C),H
        case 0x62: c->HL=_z80_sbc16(c->HL,c->HL); return; // SBC HL,HL
        case 0x63: _IMM16();_WR(c->WZ++,HL);_WR(c->WZ,HL>>8); return; // LD (nn),HL
        case 0x64: _z80_neg(c); return; // NEG
        case 0x66: c->IM=0; return; // IM 0
        case 0x67: _z80_rrd(c); return; // RRD
        case 0x68: c->L=_z80_in(c, c->BC); c->F=c->szp[c->L]|(c->F&Z80_CF); return; // IN L,(C)
        case 0x69: _z80_out(c,c->BC,c->L); return; // OUT (C),L
        case 0x6a: c->HL=_z80_adc16(c->HL,c->HL); return; // ADC HL,HL
        case 0x6b: _IMM16();uint8_t l=_RD(c->WZ++);uint8_t h=_RD(c->WZ);c->HL=(h<<8)|l; return; // LD HL,(nn)
        case 0x6c: _z80_neg(c); return; // NEG
        case 0x6e: c->IM=0; return; // IM 0
        case 0x6f: _z80_rld(c); return; // RLD
        case 0x70: c->F=c->szp[_z80_in(c, c->BC)]|(c->F&Z80_CF); return; // IN (C)
        case 0x71: _z80_out(c,c->BC,0); return; // None
        case 0x72: c->HL=_z80_sbc16(c->HL,c->SP); return; // SBC HL,SP
        case 0x73: _IMM16();_WR(c->WZ++,SP);_WR(c->WZ,SP>>8); return; // LD (nn),SP
        case 0x74: _z80_neg(c); return; // NEG
        case 0x76: c->IM=1; return; // IM 1
        case 0x77:   return; // NOP (ED)
        case 0x78: c->A=_z80_in(c, c->BC); c->F=c->szp[c->A]|(c->F&Z80_CF); return; // IN A,(C)
        case 0x79: _z80_out(c,c->BC,c->A); return; // OUT (C),A
        case 0x7a: c->HL=_z80_adc16(c->HL,c->SP); return; // ADC HL,SP
        case 0x7b: _IMM16();uint8_t l=_RD(c->WZ++);uint8_t h=_RD(c->WZ);c->SP=(h<<8)|l; return; // LD SP,(nn)
        case 0x7c: _z80_neg(c); return; // NEG
        case 0x7e: c->IM=2; return; // IM 2
        case 0x7f:   return; // NOP (ED)
        case 0xa0: _z80_ldi(c); return; // LDI
        case 0xa1: _z80_cpi(c); return; // CPI
        case 0xa2: _z80_ini(c); return; // INI
        case 0xa3: _z80_outi(c); return; // OUTI
        case 0xa8: _z80_ldd(c); return; // LDD
        case 0xa9: _z80_cpd(c); return; // CPD
        case 0xaa: _z80_ind(c); return; // IND
        case 0xab: _z80_outd(c); return; // OUTD
        case 0xb0: _z80_ldir(c); return; // LDIR
        case 0xb1: _z80_cpir(c); return; // CPIR
        case 0xb2: _z80_inir(c); return; // INIR
        case 0xb3: _z80_otir(c); return; // OTIR
        case 0xb8: _z80_lddr(c); return; // LDDR
        case 0xb9: _z80_cpdr(c); return; // CPDR
        case 0xba: _z80_indr(c); return; // INDR
        case 0xbb: _z80_otdr(c); return; // OTDR
        default: return _INVALID_OPCODE(2);
      }
      break;
    case 0xee: _z80_xor(_RD(c->PC++)); return; // XOR n
    case 0xef: _z80_rst(0x28); return; // RST 0x28
    case 0xf0: _z80_retcc(c,!(c->F&Z80_SF)); return; // RET P
    case 0xf1: uint8_t l=_RD(c->SP++);uint8_t h=_RD(c->SP++);c->AF=(h<<8)|l; return; // POP AF
    case 0xf2: _IMM16(); if (!(c->F&Z80_SF)) { c->PC=c->WZ; } return; // JP P,nn
    case 0xf3: _z80_di(); return; // DI
    case 0xf4: _z80_callcc(c, !(c->F&Z80_SF)); return; // CALL P,nn
    case 0xf5: _WR(--c->SP,AF<<8); _WR(--c->SP,AF); return; // PUSH AF
    case 0xf6: _z80_or(_RD(c->PC++)); return; // OR n
    case 0xf7: _z80_rst(0x30); return; // RST 0x30
    case 0xf8: _z80_retcc(c,(c->F&Z80_SF)); return; // RET M
    case 0xf9: _T(2);c->SP=c->HL; return; // LD SP,HL
    case 0xfa: _IMM16(); if ((c->F&Z80_SF)) { c->PC=c->WZ; } return; // JP M,nn
    case 0xfb: _z80_ei(); return; // EI
    case 0xfc: _z80_callcc(c, (c->F&Z80_SF)); return; // CALL M,nn
    case 0xfd:
      switch (_z80_fetch(c)) {
        case 0x0:   return; // NOP
        case 0x1: _IMM16(); c->BC=c->WZ; return; // LD BC,nn
        case 0x2: c->WZ=c->BC;_WR(c->WZ++,c->A);c->W=c->A; return; // LD (BC),A
        case 0x3: c->BC++; return; // INC BC
        case 0x4: c->B=_z80_inc(c->B); return; // INC B
        case 0x5: c->B=_z80_dec(c->B); return; // DEC B
        case 0x6: c->B=_RD(c->PC++); return; // LD B,n
        case 0x7: _z80_rlca(c); return; // RLCA
        case 0x8: _SWP16(c->FA)c->FA_)); return; // EX AF,AF'
        case 0x9: c->IY=_z80_add16(c,c->IY,c->BC); return; // ADD IY,BC
        case 0xa: c->WZ=c->BC;c->A=_RD(c->WZ++); return; // LD A,(BC)
        case 0xb: c->BC--; return; // DEC BC
        case 0xc: c->C=_z80_inc(c->C); return; // INC C
        case 0xd: c->C=_z80_dec(c->C); return; // DEC C
        case 0xe: c->C=_RD(c->PC++); return; // LD C,n
        case 0xf: _z80_rrca(c); return; // RRCA
        case 0x10: _z80_djnz(c); return; // DJNZ
        case 0x11: _IMM16(); c->DE=c->WZ; return; // LD DE,nn
        case 0x12: c->WZ=c->DE;_WR(c->WZ++,c->A);c->W=c->A; return; // LD (DE),A
        case 0x13: c->DE++; return; // INC DE
        case 0x14: c->D=_z80_inc(c->D); return; // INC D
        case 0x15: c->D=_z80_dec(c->D); return; // DEC D
        case 0x16: c->D=_RD(c->PC++); return; // LD D,n
        case 0x17: _z80_rla(c); return; // RLA
        case 0x18: _z80_jr(c); return; // JR d
        case 0x19: c->IY=_z80_add16(c,c->IY,c->DE); return; // ADD IY,DE
        case 0x1a: c->WZ=c->DE;c->A=_RD(c->WZ++); return; // LD A,(DE)
        case 0x1b: c->DE--; return; // DEC DE
        case 0x1c: c->E=_z80_inc(c->E); return; // INC E
        case 0x1d: c->E=_z80_dec(c->E); return; // DEC E
        case 0x1e: c->E=_RD(c->PC++); return; // LD E,n
        case 0x1f: _z80_rra(c); return; // RRA
        case 0x20: _z80_jr_cc(c,!(c->F)&Z80_ZF)) return; // JR NZ,d
        case 0x21: _IMM16(); c->IY=c->WZ; return; // LD IY,nn
        case 0x22: _IMM16();_WR(c->WZ++,c->IY);_WR(c->WZ,c->IY>>8); return; // LD (nn),IY
        case 0x23: c->IY++; return; // INC IY
        case 0x24: c->IYH=_z80_inc(c->IYH); return; // INC IYH
        case 0x25: c->IYH=_z80_dec(c->IYH); return; // DEC IYH
        case 0x26: c->IYH=_RD(c->PC++); return; // LD IYH,n
        case 0x27: _z80_daa(c); return; // DAA
        case 0x28: _z80_jr_cc(c,(c->F)&Z80_ZF)) return; // JR Z,d
        case 0x29: c->IY=_z80_add16(c,c->IY,c->IY); return; // ADD IY,IY
        case 0x2a: _IMM16();uint8_t l=_RD(c->WZ++);uint8_t h=_RD(c->WZ);c->IY=(h<<8)|l; return; // LD IY,(nn)
        case 0x2b: c->IY--; return; // DEC IY
        case 0x2c: c->IYL=_z80_inc(c->IYL); return; // INC IYL
        case 0x2d: c->IYL=_z80_dec(c->IYL); return; // DEC IYL
        case 0x2e: c->IYL=_RD(c->PC++); return; // LD IYL,n
        case 0x2f: _z80_cpl(c); return; // CPL
        case 0x30: _z80_jr_cc(c,!(c->F&Z80_CF)) return; // JR NC,d
        case 0x31: _IMM16(); c->SP=c->WZ; return; // LD SP,nn
        case 0x32: _IMM16();_WR(c->WZ++,c->A);c->W=c->A; return; // LD (nn),A
        case 0x33: c->SP++; return; // INC SP
        case 0x34: { uint16_t a=c->WZ=c->IY+_RD(c->PC++); _WR(a,_z80_inc(_RD(a))); } return; // INC (IY+d)
        case 0x35: { uint16_t a=c->WZ=c->IY+_RD(c->PC++); _WR(a,_z80_dec(_RD(a))); } return; // DEC (IY+d)
        case 0x36: { uint16_t a=c->WZ=c->IY+_RD(c->PC++); _WR(a,_RD(c->PC++)); } return; // LD (IY+d),n
        case 0x37: _z80_scf(c); return; // SCF
        case 0x38: _z80_jr_cc(c,(c->F&Z80_CF)) return; // JR C,d
        case 0x39: c->IY=_z80_add16(c,c->IY,c->SP); return; // ADD IY,SP
        case 0x3a: _IMM16();c->A=_RD(c->WZ++); return; // LD A,(nn)
        case 0x3b: c->SP--; return; // DEC SP
        case 0x3c: c->A=_z80_inc(c->A); return; // INC A
        case 0x3d: c->A=_z80_dec(c->A); return; // DEC A
        case 0x3e: c->A=_RD(c->PC++); return; // LD A,n
        case 0x3f: _z80_ccf(c); return; // CCF
        case 0x40: c->B=c->B; return; // LD B,B
        case 0x41: c->B=c->C; return; // LD B,C
        case 0x42: c->B=c->D; return; // LD B,D
        case 0x43: c->B=c->E; return; // LD B,E
        case 0x44: c->B=c->IYH; return; // LD B,IYH
        case 0x45: c->B=c->IYL; return; // LD B,IYL
        case 0x46: { uint16_t a=c->WZ=c->IY+_RD(c->PC++); c->B=_RD(a); }; return; // LD B,(IY+d)
        case 0x47: c->B=c->A; return; // LD B,A
        case 0x48: c->C=c->B; return; // LD C,B
        case 0x49: c->C=c->C; return; // LD C,C
        case 0x4a: c->C=c->D; return; // LD C,D
        case 0x4b: c->C=c->E; return; // LD C,E
        case 0x4c: c->C=c->IYH; return; // LD C,IYH
        case 0x4d: c->C=c->IYL; return; // LD C,IYL
        case 0x4e: { uint16_t a=c->WZ=c->IY+_RD(c->PC++); c->C=_RD(a); }; return; // LD C,(IY+d)
        case 0x4f: c->C=c->A; return; // LD C,A
        case 0x50: c->D=c->B; return; // LD D,B
        case 0x51: c->D=c->C; return; // LD D,C
        case 0x52: c->D=c->D; return; // LD D,D
        case 0x53: c->D=c->E; return; // LD D,E
        case 0x54: c->D=c->IYH; return; // LD D,IYH
        case 0x55: c->D=c->IYL; return; // LD D,IYL
        case 0x56: { uint16_t a=c->WZ=c->IY+_RD(c->PC++); c->D=_RD(a); }; return; // LD D,(IY+d)
        case 0x57: c->D=c->A; return; // LD D,A
        case 0x58: c->E=c->B; return; // LD E,B
        case 0x59: c->E=c->C; return; // LD E,C
        case 0x5a: c->E=c->D; return; // LD E,D
        case 0x5b: c->E=c->E; return; // LD E,E
        case 0x5c: c->E=c->IYH; return; // LD E,IYH
        case 0x5d: c->E=c->IYL; return; // LD E,IYL
        case 0x5e: { uint16_t a=c->WZ=c->IY+_RD(c->PC++); c->E=_RD(a); }; return; // LD E,(IY+d)
        case 0x5f: c->E=c->A; return; // LD E,A
        case 0x60: c->IYH=c->B; return; // LD IYH,B
        case 0x61: c->IYH=c->C; return; // LD IYH,C
        case 0x62: c->IYH=c->D; return; // LD IYH,D
        case 0x63: c->IYH=c->E; return; // LD IYH,E
        case 0x64: c->IYH=c->IYH; return; // LD IYH,IYH
        case 0x65: c->IYH=c->IYL; return; // LD IYH,IYL
        case 0x66: { uint16_t a=c->WZ=c->IY+_RD(c->PC++); c->H=_RD(a); }; return; // LD H,(IY+d)
        case 0x67: c->IYH=c->A; return; // LD IYH,A
        case 0x68: c->IYL=c->B; return; // LD IYL,B
        case 0x69: c->IYL=c->C; return; // LD IYL,C
        case 0x6a: c->IYL=c->D; return; // LD IYL,D
        case 0x6b: c->IYL=c->E; return; // LD IYL,E
        case 0x6c: c->IYL=c->IYH; return; // LD IYL,IYH
        case 0x6d: c->IYL=c->IYL; return; // LD IYL,IYL
        case 0x6e: { uint16_t a=c->WZ=c->IY+_RD(c->PC++); c->L=_RD(a); }; return; // LD L,(IY+d)
        case 0x6f: c->IYL=c->A; return; // LD IYL,A
        case 0x70: { uint16_t a=c->WZ=c->IY+_RD(c->PC++); _WR(a,c->B); }; return; // LD (IY+d),B
        case 0x71: { uint16_t a=c->WZ=c->IY+_RD(c->PC++); _WR(a,c->C); }; return; // LD (IY+d),C
        case 0x72: { uint16_t a=c->WZ=c->IY+_RD(c->PC++); _WR(a,c->D); }; return; // LD (IY+d),D
        case 0x73: { uint16_t a=c->WZ=c->IY+_RD(c->PC++); _WR(a,c->E); }; return; // LD (IY+d),E
        case 0x74: { uint16_t a=c->WZ=c->IY+_RD(c->PC++); _WR(a,c->H); }; return; // LD (IY+d),H
        case 0x75: { uint16_t a=c->WZ=c->IY+_RD(c->PC++); _WR(a,c->L); }; return; // LD (IY+d),L
        case 0x76: _z80_halt(c); return; // HALT
        case 0x77: { uint16_t a=c->WZ=c->IY+_RD(c->PC++); _WR(a,c->A); }; return; // LD (IY+d),A
        case 0x78: c->A=c->B; return; // LD A,B
        case 0x79: c->A=c->C; return; // LD A,C
        case 0x7a: c->A=c->D; return; // LD A,D
        case 0x7b: c->A=c->E; return; // LD A,E
        case 0x7c: c->A=c->IYH; return; // LD A,IYH
        case 0x7d: c->A=c->IYL; return; // LD A,IYL
        case 0x7e: { uint16_t a=c->WZ=c->IY+_RD(c->PC++); c->A=_RD(a); }; return; // LD A,(IY+d)
        case 0x7f: c->A=c->A; return; // LD A,A
        case 0x80: _z80_add(c->B); return; // ADD B
        case 0x81: _z80_add(c->C); return; // ADD C
        case 0x82: _z80_add(c->D); return; // ADD D
        case 0x83: _z80_add(c->E); return; // ADD E
        case 0x84: _z80_add(c->IYH); return; // ADD IYH
        case 0x85: _z80_add(c->IYL); return; // ADD IYL
        case 0x86: { uint16_t a=c->WZ=c->IY+_RD(c->PC++); _z80_add(_RD(a)); }; return; // ADD (IY+d)
        case 0x87: _z80_add(c->A); return; // ADD A
        case 0x88: _z80_adc(c->B); return; // ADC B
        case 0x89: _z80_adc(c->C); return; // ADC C
        case 0x8a: _z80_adc(c->D); return; // ADC D
        case 0x8b: _z80_adc(c->E); return; // ADC E
        case 0x8c: _z80_adc(c->IYH); return; // ADC IYH
        case 0x8d: _z80_adc(c->IYL); return; // ADC IYL
        case 0x8e: { uint16_t a=c->WZ=c->IY+_RD(c->PC++); _z80_adc(_RD(a)); }; return; // ADC (IY+d)
        case 0x8f: _z80_adc(c->A); return; // ADC A
        case 0x90: _z80_sub(c->B); return; // SUB B
        case 0x91: _z80_sub(c->C); return; // SUB C
        case 0x92: _z80_sub(c->D); return; // SUB D
        case 0x93: _z80_sub(c->E); return; // SUB E
        case 0x94: _z80_sub(c->IYH); return; // SUB IYH
        case 0x95: _z80_sub(c->IYL); return; // SUB IYL
        case 0x96: { uint16_t a=c->WZ=c->IY+_RD(c->PC++); _z80_sub(_RD(a)); }; return; // SUB (IY+d)
        case 0x97: _z80_sub(c->A); return; // SUB A
        case 0x98: _z80_sbc(c->B); return; // SBC B
        case 0x99: _z80_sbc(c->C); return; // SBC C
        case 0x9a: _z80_sbc(c->D); return; // SBC D
        case 0x9b: _z80_sbc(c->E); return; // SBC E
        case 0x9c: _z80_sbc(c->IYH); return; // SBC IYH
        case 0x9d: _z80_sbc(c->IYL); return; // SBC IYL
        case 0x9e: { uint16_t a=c->WZ=c->IY+_RD(c->PC++); _z80_sbc(_RD(a)); }; return; // SBC (IY+d)
        case 0x9f: _z80_sbc(c->A); return; // SBC A
        case 0xa0: _z80_and(c->B); return; // AND B
        case 0xa1: _z80_and(c->C); return; // AND C
        case 0xa2: _z80_and(c->D); return; // AND D
        case 0xa3: _z80_and(c->E); return; // AND E
        case 0xa4: _z80_and(c->IYH); return; // AND IYH
        case 0xa5: _z80_and(c->IYL); return; // AND IYL
        case 0xa6: { uint16_t a=c->WZ=c->IY+_RD(c->PC++); _z80_and(_RD(a)); }; return; // AND (IY+d)
        case 0xa7: _z80_and(c->A); return; // AND A
        case 0xa8: _z80_xor(c->B); return; // XOR B
        case 0xa9: _z80_xor(c->C); return; // XOR C
        case 0xaa: _z80_xor(c->D); return; // XOR D
        case 0xab: _z80_xor(c->E); return; // XOR E
        case 0xac: _z80_xor(c->IYH); return; // XOR IYH
        case 0xad: _z80_xor(c->IYL); return; // XOR IYL
        case 0xae: { uint16_t a=c->WZ=c->IY+_RD(c->PC++); _z80_xor(_RD(a)); }; return; // XOR (IY+d)
        case 0xaf: _z80_xor(c->A); return; // XOR A
        case 0xb0: _z80_or(c->B); return; // OR B
        case 0xb1: _z80_or(c->C); return; // OR C
        case 0xb2: _z80_or(c->D); return; // OR D
        case 0xb3: _z80_or(c->E); return; // OR E
        case 0xb4: _z80_or(c->IYH); return; // OR IYH
        case 0xb5: _z80_or(c->IYL); return; // OR IYL
        case 0xb6: { uint16_t a=c->WZ=c->IY+_RD(c->PC++); _z80_or(_RD(a)); }; return; // OR (IY+d)
        case 0xb7: _z80_or(c->A); return; // OR A
        case 0xb8: _z80_cp(c->B); return; // CP B
        case 0xb9: _z80_cp(c->C); return; // CP C
        case 0xba: _z80_cp(c->D); return; // CP D
        case 0xbb: _z80_cp(c->E); return; // CP E
        case 0xbc: _z80_cp(c->IYH); return; // CP IYH
        case 0xbd: _z80_cp(c->IYL); return; // CP IYL
        case 0xbe: { uint16_t a=c->WZ=c->IY+_RD(c->PC++); _z80_cp(_RD(a)); }; return; // CP (IY+d)
        case 0xbf: _z80_cp(c->A); return; // CP A
        case 0xc0: _z80_retcc(c,!(c->F)&Z80_ZF)); return; // RET NZ
        case 0xc1: uint8_t l=_RD(c->SP++);uint8_t h=_RD(c->SP++);c->BC=(h<<8)|l; return; // POP BC
        case 0xc2: _IMM16(); if (!(c->F)&Z80_ZF)) { c->PC=c->WZ; } return; // JP NZ,nn
        case 0xc3: _IMM16(); c->PC=c->WZ; return; // JP nn
        case 0xc4: _z80_callcc(c, !(c->F)&Z80_ZF)); return; // CALL NZ,nn
        case 0xc5: _WR(--c->SP,BC<<8); _WR(--c->SP,BC); return; // PUSH BC
        case 0xc6: _z80_add(_RD(c->PC++)); return; // ADD n
        case 0xc7: _z80_rst(0x0); return; // RST 0x0
        case 0xc8: _z80_retcc(c,(c->F)&Z80_ZF)); return; // RET Z
        case 0xc9: c->Z=_RD(c->SP++);c->W=_RD(c->SP++);c->PC=c->WZ; return; // RET
        case 0xca: _IMM16(); if ((c->F)&Z80_ZF)) { c->PC=c->WZ; } return; // JP Z,nn
        case 0xcb:
          { const int8_t d = _READ(_REG(PC++));
          switch (_z80_fetch(c)) {
            case 0x0: { uint16_t a=c->WZ=IY+d;; c->B=_z80_rlc(_RD(a)); _WR(a,c->B); } return; // RLC (IY+d),B
            case 0x1: { uint16_t a=c->WZ=IY+d;; c->C=_z80_rlc(_RD(a)); _WR(a,c->C); } return; // RLC (IY+d),C
            case 0x2: { uint16_t a=c->WZ=IY+d;; c->D=_z80_rlc(_RD(a)); _WR(a,c->D); } return; // RLC (IY+d),D
            case 0x3: { uint16_t a=c->WZ=IY+d;; c->E=_z80_rlc(_RD(a)); _WR(a,c->E); } return; // RLC (IY+d),E
            case 0x4: { uint16_t a=c->WZ=IY+d;; c->H=_z80_rlc(_RD(a)); _WR(a,c->H); } return; // RLC (IY+d),H
            case 0x5: { uint16_t a=c->WZ=IY+d;; c->L=_z80_rlc(_RD(a)); _WR(a,c->L); } return; // RLC (IY+d),L
            case 0x6: { uint16_t a=c->WZ=IY+d;; _WR(a,_z80_rlc(_RD(a))); } return; // RLC (IY+d)
            case 0x7: { uint16_t a=c->WZ=IY+d;; c->A=_z80_rlc(_RD(a)); _WR(a,c->A); } return; // RLC (IY+d),A
            case 0x8: { uint16_t a=c->WZ=IY+d;; c->B=_z80_rrc(_RD(a)); _WR(a,c->B); } return; // RRC (IY+d),B
            case 0x9: { uint16_t a=c->WZ=IY+d;; c->C=_z80_rrc(_RD(a)); _WR(a,c->C); } return; // RRC (IY+d),C
            case 0xa: { uint16_t a=c->WZ=IY+d;; c->D=_z80_rrc(_RD(a)); _WR(a,c->D); } return; // RRC (IY+d),D
            case 0xb: { uint16_t a=c->WZ=IY+d;; c->E=_z80_rrc(_RD(a)); _WR(a,c->E); } return; // RRC (IY+d),E
            case 0xc: { uint16_t a=c->WZ=IY+d;; c->H=_z80_rrc(_RD(a)); _WR(a,c->H); } return; // RRC (IY+d),H
            case 0xd: { uint16_t a=c->WZ=IY+d;; c->L=_z80_rrc(_RD(a)); _WR(a,c->L); } return; // RRC (IY+d),L
            case 0xe: { uint16_t a=c->WZ=IY+d;; _WR(a,_z80_rrc(_RD(a))); } return; // RRC (IY+d)
            case 0xf: { uint16_t a=c->WZ=IY+d;; c->A=_z80_rrc(_RD(a)); _WR(a,c->A); } return; // RRC (IY+d),A
            case 0x10: { uint16_t a=c->WZ=IY+d;; c->B=_z80_rl(_RD(a)); _WR(a,c->B); } return; // RL (IY+d),B
            case 0x11: { uint16_t a=c->WZ=IY+d;; c->C=_z80_rl(_RD(a)); _WR(a,c->C); } return; // RL (IY+d),C
            case 0x12: { uint16_t a=c->WZ=IY+d;; c->D=_z80_rl(_RD(a)); _WR(a,c->D); } return; // RL (IY+d),D
            case 0x13: { uint16_t a=c->WZ=IY+d;; c->E=_z80_rl(_RD(a)); _WR(a,c->E); } return; // RL (IY+d),E
            case 0x14: { uint16_t a=c->WZ=IY+d;; c->H=_z80_rl(_RD(a)); _WR(a,c->H); } return; // RL (IY+d),H
            case 0x15: { uint16_t a=c->WZ=IY+d;; c->L=_z80_rl(_RD(a)); _WR(a,c->L); } return; // RL (IY+d),L
            case 0x16: { uint16_t a=c->WZ=IY+d;; _WR(a,_z80_rl(_RD(a))); } return; // RL (IY+d)
            case 0x17: { uint16_t a=c->WZ=IY+d;; c->A=_z80_rl(_RD(a)); _WR(a,c->A); } return; // RL (IY+d),A
            case 0x18: { uint16_t a=c->WZ=IY+d;; c->B=_z80_rr(_RD(a)); _WR(a,c->B); } return; // RR (IY+d),B
            case 0x19: { uint16_t a=c->WZ=IY+d;; c->C=_z80_rr(_RD(a)); _WR(a,c->C); } return; // RR (IY+d),C
            case 0x1a: { uint16_t a=c->WZ=IY+d;; c->D=_z80_rr(_RD(a)); _WR(a,c->D); } return; // RR (IY+d),D
            case 0x1b: { uint16_t a=c->WZ=IY+d;; c->E=_z80_rr(_RD(a)); _WR(a,c->E); } return; // RR (IY+d),E
            case 0x1c: { uint16_t a=c->WZ=IY+d;; c->H=_z80_rr(_RD(a)); _WR(a,c->H); } return; // RR (IY+d),H
            case 0x1d: { uint16_t a=c->WZ=IY+d;; c->L=_z80_rr(_RD(a)); _WR(a,c->L); } return; // RR (IY+d),L
            case 0x1e: { uint16_t a=c->WZ=IY+d;; _WR(a,_z80_rr(_RD(a))); } return; // RR (IY+d)
            case 0x1f: { uint16_t a=c->WZ=IY+d;; c->A=_z80_rr(_RD(a)); _WR(a,c->A); } return; // RR (IY+d),A
            case 0x20: { uint16_t a=c->WZ=IY+d;; c->B=_z80_sla(_RD(a)); _WR(a,c->B); } return; // SLA (IY+d),B
            case 0x21: { uint16_t a=c->WZ=IY+d;; c->C=_z80_sla(_RD(a)); _WR(a,c->C); } return; // SLA (IY+d),C
            case 0x22: { uint16_t a=c->WZ=IY+d;; c->D=_z80_sla(_RD(a)); _WR(a,c->D); } return; // SLA (IY+d),D
            case 0x23: { uint16_t a=c->WZ=IY+d;; c->E=_z80_sla(_RD(a)); _WR(a,c->E); } return; // SLA (IY+d),E
            case 0x24: { uint16_t a=c->WZ=IY+d;; c->H=_z80_sla(_RD(a)); _WR(a,c->H); } return; // SLA (IY+d),H
            case 0x25: { uint16_t a=c->WZ=IY+d;; c->L=_z80_sla(_RD(a)); _WR(a,c->L); } return; // SLA (IY+d),L
            case 0x26: { uint16_t a=c->WZ=IY+d;; _WR(a,_z80_sla(_RD(a))); } return; // SLA (IY+d)
            case 0x27: { uint16_t a=c->WZ=IY+d;; c->A=_z80_sla(_RD(a)); _WR(a,c->A); } return; // SLA (IY+d),A
            case 0x28: { uint16_t a=c->WZ=IY+d;; c->B=_z80_sra(_RD(a)); _WR(a,c->B); } return; // SRA (IY+d),B
            case 0x29: { uint16_t a=c->WZ=IY+d;; c->C=_z80_sra(_RD(a)); _WR(a,c->C); } return; // SRA (IY+d),C
            case 0x2a: { uint16_t a=c->WZ=IY+d;; c->D=_z80_sra(_RD(a)); _WR(a,c->D); } return; // SRA (IY+d),D
            case 0x2b: { uint16_t a=c->WZ=IY+d;; c->E=_z80_sra(_RD(a)); _WR(a,c->E); } return; // SRA (IY+d),E
            case 0x2c: { uint16_t a=c->WZ=IY+d;; c->H=_z80_sra(_RD(a)); _WR(a,c->H); } return; // SRA (IY+d),H
            case 0x2d: { uint16_t a=c->WZ=IY+d;; c->L=_z80_sra(_RD(a)); _WR(a,c->L); } return; // SRA (IY+d),L
            case 0x2e: { uint16_t a=c->WZ=IY+d;; _WR(a,_z80_sra(_RD(a))); } return; // SRA (IY+d)
            case 0x2f: { uint16_t a=c->WZ=IY+d;; c->A=_z80_sra(_RD(a)); _WR(a,c->A); } return; // SRA (IY+d),A
            case 0x30: { uint16_t a=c->WZ=IY+d;; c->B=_z80_sll(_RD(a)); _WR(a,c->B); } return; // SLL (IY+d),B
            case 0x31: { uint16_t a=c->WZ=IY+d;; c->C=_z80_sll(_RD(a)); _WR(a,c->C); } return; // SLL (IY+d),C
            case 0x32: { uint16_t a=c->WZ=IY+d;; c->D=_z80_sll(_RD(a)); _WR(a,c->D); } return; // SLL (IY+d),D
            case 0x33: { uint16_t a=c->WZ=IY+d;; c->E=_z80_sll(_RD(a)); _WR(a,c->E); } return; // SLL (IY+d),E
            case 0x34: { uint16_t a=c->WZ=IY+d;; c->H=_z80_sll(_RD(a)); _WR(a,c->H); } return; // SLL (IY+d),H
            case 0x35: { uint16_t a=c->WZ=IY+d;; c->L=_z80_sll(_RD(a)); _WR(a,c->L); } return; // SLL (IY+d),L
            case 0x36: { uint16_t a=c->WZ=IY+d;; _WR(a,_z80_sll(_RD(a))); } return; // SLL (IY+d)
            case 0x37: { uint16_t a=c->WZ=IY+d;; c->A=_z80_sll(_RD(a)); _WR(a,c->A); } return; // SLL (IY+d),A
            case 0x38: { uint16_t a=c->WZ=IY+d;; c->B=_z80_srl(_RD(a)); _WR(a,c->B); } return; // SRL (IY+d),B
            case 0x39: { uint16_t a=c->WZ=IY+d;; c->C=_z80_srl(_RD(a)); _WR(a,c->C); } return; // SRL (IY+d),C
            case 0x3a: { uint16_t a=c->WZ=IY+d;; c->D=_z80_srl(_RD(a)); _WR(a,c->D); } return; // SRL (IY+d),D
            case 0x3b: { uint16_t a=c->WZ=IY+d;; c->E=_z80_srl(_RD(a)); _WR(a,c->E); } return; // SRL (IY+d),E
            case 0x3c: { uint16_t a=c->WZ=IY+d;; c->H=_z80_srl(_RD(a)); _WR(a,c->H); } return; // SRL (IY+d),H
            case 0x3d: { uint16_t a=c->WZ=IY+d;; c->L=_z80_srl(_RD(a)); _WR(a,c->L); } return; // SRL (IY+d),L
            case 0x3e: { uint16_t a=c->WZ=IY+d;; _WR(a,_z80_srl(_RD(a))); } return; // SRL (IY+d)
            case 0x3f: { uint16_t a=c->WZ=IY+d;; c->A=_z80_srl(_RD(a)); _WR(a,c->A); } return; // SRL (IY+d),A
            case 0x40: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x1); } return; // BIT 0,(IY+d)
            case 0x41: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x1); } return; // BIT 0,(IY+d)
            case 0x42: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x1); } return; // BIT 0,(IY+d)
            case 0x43: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x1); } return; // BIT 0,(IY+d)
            case 0x44: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x1); } return; // BIT 0,(IY+d)
            case 0x45: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x1); } return; // BIT 0,(IY+d)
            case 0x46: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x1); } return; // BIT 0,(IY+d)
            case 0x47: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x1); } return; // BIT 0,(IY+d)
            case 0x48: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x2); } return; // BIT 1,(IY+d)
            case 0x49: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x2); } return; // BIT 1,(IY+d)
            case 0x4a: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x2); } return; // BIT 1,(IY+d)
            case 0x4b: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x2); } return; // BIT 1,(IY+d)
            case 0x4c: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x2); } return; // BIT 1,(IY+d)
            case 0x4d: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x2); } return; // BIT 1,(IY+d)
            case 0x4e: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x2); } return; // BIT 1,(IY+d)
            case 0x4f: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x2); } return; // BIT 1,(IY+d)
            case 0x50: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x4); } return; // BIT 2,(IY+d)
            case 0x51: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x4); } return; // BIT 2,(IY+d)
            case 0x52: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x4); } return; // BIT 2,(IY+d)
            case 0x53: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x4); } return; // BIT 2,(IY+d)
            case 0x54: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x4); } return; // BIT 2,(IY+d)
            case 0x55: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x4); } return; // BIT 2,(IY+d)
            case 0x56: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x4); } return; // BIT 2,(IY+d)
            case 0x57: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x4); } return; // BIT 2,(IY+d)
            case 0x58: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x8); } return; // BIT 3,(IY+d)
            case 0x59: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x8); } return; // BIT 3,(IY+d)
            case 0x5a: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x8); } return; // BIT 3,(IY+d)
            case 0x5b: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x8); } return; // BIT 3,(IY+d)
            case 0x5c: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x8); } return; // BIT 3,(IY+d)
            case 0x5d: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x8); } return; // BIT 3,(IY+d)
            case 0x5e: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x8); } return; // BIT 3,(IY+d)
            case 0x5f: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x8); } return; // BIT 3,(IY+d)
            case 0x60: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x10); } return; // BIT 4,(IY+d)
            case 0x61: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x10); } return; // BIT 4,(IY+d)
            case 0x62: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x10); } return; // BIT 4,(IY+d)
            case 0x63: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x10); } return; // BIT 4,(IY+d)
            case 0x64: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x10); } return; // BIT 4,(IY+d)
            case 0x65: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x10); } return; // BIT 4,(IY+d)
            case 0x66: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x10); } return; // BIT 4,(IY+d)
            case 0x67: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x10); } return; // BIT 4,(IY+d)
            case 0x68: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x20); } return; // BIT 5,(IY+d)
            case 0x69: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x20); } return; // BIT 5,(IY+d)
            case 0x6a: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x20); } return; // BIT 5,(IY+d)
            case 0x6b: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x20); } return; // BIT 5,(IY+d)
            case 0x6c: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x20); } return; // BIT 5,(IY+d)
            case 0x6d: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x20); } return; // BIT 5,(IY+d)
            case 0x6e: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x20); } return; // BIT 5,(IY+d)
            case 0x6f: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x20); } return; // BIT 5,(IY+d)
            case 0x70: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x40); } return; // BIT 6,(IY+d)
            case 0x71: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x40); } return; // BIT 6,(IY+d)
            case 0x72: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x40); } return; // BIT 6,(IY+d)
            case 0x73: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x40); } return; // BIT 6,(IY+d)
            case 0x74: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x40); } return; // BIT 6,(IY+d)
            case 0x75: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x40); } return; // BIT 6,(IY+d)
            case 0x76: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x40); } return; // BIT 6,(IY+d)
            case 0x77: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x40); } return; // BIT 6,(IY+d)
            case 0x78: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x80); } return; // BIT 7,(IY+d)
            case 0x79: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x80); } return; // BIT 7,(IY+d)
            case 0x7a: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x80); } return; // BIT 7,(IY+d)
            case 0x7b: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x80); } return; // BIT 7,(IY+d)
            case 0x7c: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x80); } return; // BIT 7,(IY+d)
            case 0x7d: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x80); } return; // BIT 7,(IY+d)
            case 0x7e: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x80); } return; // BIT 7,(IY+d)
            case 0x7f: { uint16_t a=c->WZ=IY+d;; _z80_ibit(c,_RD(a),0x80); } return; // BIT 7,(IY+d)
            case 0x80: { uint16_t a=c->WZ=IY+d;; c->B=_RD(a)&~0x1; _WR(a,c->B); } return; // RES 0,(IY+d),B
            case 0x81: { uint16_t a=c->WZ=IY+d;; c->C=_RD(a)&~0x1; _WR(a,c->C); } return; // RES 0,(IY+d),C
            case 0x82: { uint16_t a=c->WZ=IY+d;; c->D=_RD(a)&~0x1; _WR(a,c->D); } return; // RES 0,(IY+d),D
            case 0x83: { uint16_t a=c->WZ=IY+d;; c->E=_RD(a)&~0x1; _WR(a,c->E); } return; // RES 0,(IY+d),E
            case 0x84: { uint16_t a=c->WZ=IY+d;; c->H=_RD(a)&~0x1; _WR(a,c->H); } return; // RES 0,(IY+d),H
            case 0x85: { uint16_t a=c->WZ=IY+d;; c->L=_RD(a)&~0x1; _WR(a,c->L); } return; // RES 0,(IY+d),L
            case 0x86: { uint16_t a=c->WZ=IY+d;; _WR(a,_RD(a)&~0x1); } return; // RES 0,(IY+d)
            case 0x87: { uint16_t a=c->WZ=IY+d;; c->A=_RD(a)&~0x1; _WR(a,c->A); } return; // RES 0,(IY+d),A
            case 0x88: { uint16_t a=c->WZ=IY+d;; c->B=_RD(a)&~0x2; _WR(a,c->B); } return; // RES 1,(IY+d),B
            case 0x89: { uint16_t a=c->WZ=IY+d;; c->C=_RD(a)&~0x2; _WR(a,c->C); } return; // RES 1,(IY+d),C
            case 0x8a: { uint16_t a=c->WZ=IY+d;; c->D=_RD(a)&~0x2; _WR(a,c->D); } return; // RES 1,(IY+d),D
            case 0x8b: { uint16_t a=c->WZ=IY+d;; c->E=_RD(a)&~0x2; _WR(a,c->E); } return; // RES 1,(IY+d),E
            case 0x8c: { uint16_t a=c->WZ=IY+d;; c->H=_RD(a)&~0x2; _WR(a,c->H); } return; // RES 1,(IY+d),H
            case 0x8d: { uint16_t a=c->WZ=IY+d;; c->L=_RD(a)&~0x2; _WR(a,c->L); } return; // RES 1,(IY+d),L
            case 0x8e: { uint16_t a=c->WZ=IY+d;; _WR(a,_RD(a)&~0x2); } return; // RES 1,(IY+d)
            case 0x8f: { uint16_t a=c->WZ=IY+d;; c->A=_RD(a)&~0x2; _WR(a,c->A); } return; // RES 1,(IY+d),A
            case 0x90: { uint16_t a=c->WZ=IY+d;; c->B=_RD(a)&~0x4; _WR(a,c->B); } return; // RES 2,(IY+d),B
            case 0x91: { uint16_t a=c->WZ=IY+d;; c->C=_RD(a)&~0x4; _WR(a,c->C); } return; // RES 2,(IY+d),C
            case 0x92: { uint16_t a=c->WZ=IY+d;; c->D=_RD(a)&~0x4; _WR(a,c->D); } return; // RES 2,(IY+d),D
            case 0x93: { uint16_t a=c->WZ=IY+d;; c->E=_RD(a)&~0x4; _WR(a,c->E); } return; // RES 2,(IY+d),E
            case 0x94: { uint16_t a=c->WZ=IY+d;; c->H=_RD(a)&~0x4; _WR(a,c->H); } return; // RES 2,(IY+d),H
            case 0x95: { uint16_t a=c->WZ=IY+d;; c->L=_RD(a)&~0x4; _WR(a,c->L); } return; // RES 2,(IY+d),L
            case 0x96: { uint16_t a=c->WZ=IY+d;; _WR(a,_RD(a)&~0x4); } return; // RES 2,(IY+d)
            case 0x97: { uint16_t a=c->WZ=IY+d;; c->A=_RD(a)&~0x4; _WR(a,c->A); } return; // RES 2,(IY+d),A
            case 0x98: { uint16_t a=c->WZ=IY+d;; c->B=_RD(a)&~0x8; _WR(a,c->B); } return; // RES 3,(IY+d),B
            case 0x99: { uint16_t a=c->WZ=IY+d;; c->C=_RD(a)&~0x8; _WR(a,c->C); } return; // RES 3,(IY+d),C
            case 0x9a: { uint16_t a=c->WZ=IY+d;; c->D=_RD(a)&~0x8; _WR(a,c->D); } return; // RES 3,(IY+d),D
            case 0x9b: { uint16_t a=c->WZ=IY+d;; c->E=_RD(a)&~0x8; _WR(a,c->E); } return; // RES 3,(IY+d),E
            case 0x9c: { uint16_t a=c->WZ=IY+d;; c->H=_RD(a)&~0x8; _WR(a,c->H); } return; // RES 3,(IY+d),H
            case 0x9d: { uint16_t a=c->WZ=IY+d;; c->L=_RD(a)&~0x8; _WR(a,c->L); } return; // RES 3,(IY+d),L
            case 0x9e: { uint16_t a=c->WZ=IY+d;; _WR(a,_RD(a)&~0x8); } return; // RES 3,(IY+d)
            case 0x9f: { uint16_t a=c->WZ=IY+d;; c->A=_RD(a)&~0x8; _WR(a,c->A); } return; // RES 3,(IY+d),A
            case 0xa0: { uint16_t a=c->WZ=IY+d;; c->B=_RD(a)&~0x10; _WR(a,c->B); } return; // RES 4,(IY+d),B
            case 0xa1: { uint16_t a=c->WZ=IY+d;; c->C=_RD(a)&~0x10; _WR(a,c->C); } return; // RES 4,(IY+d),C
            case 0xa2: { uint16_t a=c->WZ=IY+d;; c->D=_RD(a)&~0x10; _WR(a,c->D); } return; // RES 4,(IY+d),D
            case 0xa3: { uint16_t a=c->WZ=IY+d;; c->E=_RD(a)&~0x10; _WR(a,c->E); } return; // RES 4,(IY+d),E
            case 0xa4: { uint16_t a=c->WZ=IY+d;; c->H=_RD(a)&~0x10; _WR(a,c->H); } return; // RES 4,(IY+d),H
            case 0xa5: { uint16_t a=c->WZ=IY+d;; c->L=_RD(a)&~0x10; _WR(a,c->L); } return; // RES 4,(IY+d),L
            case 0xa6: { uint16_t a=c->WZ=IY+d;; _WR(a,_RD(a)&~0x10); } return; // RES 4,(IY+d)
            case 0xa7: { uint16_t a=c->WZ=IY+d;; c->A=_RD(a)&~0x10; _WR(a,c->A); } return; // RES 4,(IY+d),A
            case 0xa8: { uint16_t a=c->WZ=IY+d;; c->B=_RD(a)&~0x20; _WR(a,c->B); } return; // RES 5,(IY+d),B
            case 0xa9: { uint16_t a=c->WZ=IY+d;; c->C=_RD(a)&~0x20; _WR(a,c->C); } return; // RES 5,(IY+d),C
            case 0xaa: { uint16_t a=c->WZ=IY+d;; c->D=_RD(a)&~0x20; _WR(a,c->D); } return; // RES 5,(IY+d),D
            case 0xab: { uint16_t a=c->WZ=IY+d;; c->E=_RD(a)&~0x20; _WR(a,c->E); } return; // RES 5,(IY+d),E
            case 0xac: { uint16_t a=c->WZ=IY+d;; c->H=_RD(a)&~0x20; _WR(a,c->H); } return; // RES 5,(IY+d),H
            case 0xad: { uint16_t a=c->WZ=IY+d;; c->L=_RD(a)&~0x20; _WR(a,c->L); } return; // RES 5,(IY+d),L
            case 0xae: { uint16_t a=c->WZ=IY+d;; _WR(a,_RD(a)&~0x20); } return; // RES 5,(IY+d)
            case 0xaf: { uint16_t a=c->WZ=IY+d;; c->A=_RD(a)&~0x20; _WR(a,c->A); } return; // RES 5,(IY+d),A
            case 0xb0: { uint16_t a=c->WZ=IY+d;; c->B=_RD(a)&~0x40; _WR(a,c->B); } return; // RES 6,(IY+d),B
            case 0xb1: { uint16_t a=c->WZ=IY+d;; c->C=_RD(a)&~0x40; _WR(a,c->C); } return; // RES 6,(IY+d),C
            case 0xb2: { uint16_t a=c->WZ=IY+d;; c->D=_RD(a)&~0x40; _WR(a,c->D); } return; // RES 6,(IY+d),D
            case 0xb3: { uint16_t a=c->WZ=IY+d;; c->E=_RD(a)&~0x40; _WR(a,c->E); } return; // RES 6,(IY+d),E
            case 0xb4: { uint16_t a=c->WZ=IY+d;; c->H=_RD(a)&~0x40; _WR(a,c->H); } return; // RES 6,(IY+d),H
            case 0xb5: { uint16_t a=c->WZ=IY+d;; c->L=_RD(a)&~0x40; _WR(a,c->L); } return; // RES 6,(IY+d),L
            case 0xb6: { uint16_t a=c->WZ=IY+d;; _WR(a,_RD(a)&~0x40); } return; // RES 6,(IY+d)
            case 0xb7: { uint16_t a=c->WZ=IY+d;; c->A=_RD(a)&~0x40; _WR(a,c->A); } return; // RES 6,(IY+d),A
            case 0xb8: { uint16_t a=c->WZ=IY+d;; c->B=_RD(a)&~0x80; _WR(a,c->B); } return; // RES 7,(IY+d),B
            case 0xb9: { uint16_t a=c->WZ=IY+d;; c->C=_RD(a)&~0x80; _WR(a,c->C); } return; // RES 7,(IY+d),C
            case 0xba: { uint16_t a=c->WZ=IY+d;; c->D=_RD(a)&~0x80; _WR(a,c->D); } return; // RES 7,(IY+d),D
            case 0xbb: { uint16_t a=c->WZ=IY+d;; c->E=_RD(a)&~0x80; _WR(a,c->E); } return; // RES 7,(IY+d),E
            case 0xbc: { uint16_t a=c->WZ=IY+d;; c->H=_RD(a)&~0x80; _WR(a,c->H); } return; // RES 7,(IY+d),H
            case 0xbd: { uint16_t a=c->WZ=IY+d;; c->L=_RD(a)&~0x80; _WR(a,c->L); } return; // RES 7,(IY+d),L
            case 0xbe: { uint16_t a=c->WZ=IY+d;; _WR(a,_RD(a)&~0x80); } return; // RES 7,(IY+d)
            case 0xbf: { uint16_t a=c->WZ=IY+d;; c->A=_RD(a)&~0x80; _WR(a,c->A); } return; // RES 7,(IY+d),A
            case 0xc0: { uint16_t a=c->WZ=IY+d;; c->B=_RD(a)|0x1; _WR(a,c->B);} return; // SET 0,(IY+d),B
            case 0xc1: { uint16_t a=c->WZ=IY+d;; c->C=_RD(a)|0x1; _WR(a,c->C);} return; // SET 0,(IY+d),C
            case 0xc2: { uint16_t a=c->WZ=IY+d;; c->D=_RD(a)|0x1; _WR(a,c->D);} return; // SET 0,(IY+d),D
            case 0xc3: { uint16_t a=c->WZ=IY+d;; c->E=_RD(a)|0x1; _WR(a,c->E);} return; // SET 0,(IY+d),E
            case 0xc4: { uint16_t a=c->WZ=IY+d;; c->H=_RD(a)|0x1; _WR(a,c->IYH);} return; // SET 0,(IY+d),H
            case 0xc5: { uint16_t a=c->WZ=IY+d;; c->L=_RD(a)|0x1; _WR(a,c->IYL);} return; // SET 0,(IY+d),L
            case 0xc6: { uint16_t a=c->WZ=IY+d;; _WR(a,_RD(a)|0x1);} return; // SET 0,(IY+d)
            case 0xc7: { uint16_t a=c->WZ=IY+d;; c->A=_RD(a)|0x1; _WR(a,c->A);} return; // SET 0,(IY+d),A
            case 0xc8: { uint16_t a=c->WZ=IY+d;; c->B=_RD(a)|0x2; _WR(a,c->B);} return; // SET 1,(IY+d),B
            case 0xc9: { uint16_t a=c->WZ=IY+d;; c->C=_RD(a)|0x2; _WR(a,c->C);} return; // SET 1,(IY+d),C
            case 0xca: { uint16_t a=c->WZ=IY+d;; c->D=_RD(a)|0x2; _WR(a,c->D);} return; // SET 1,(IY+d),D
            case 0xcb: { uint16_t a=c->WZ=IY+d;; c->E=_RD(a)|0x2; _WR(a,c->E);} return; // SET 1,(IY+d),E
            case 0xcc: { uint16_t a=c->WZ=IY+d;; c->H=_RD(a)|0x2; _WR(a,c->IYH);} return; // SET 1,(IY+d),H
            case 0xcd: { uint16_t a=c->WZ=IY+d;; c->L=_RD(a)|0x2; _WR(a,c->IYL);} return; // SET 1,(IY+d),L
            case 0xce: { uint16_t a=c->WZ=IY+d;; _WR(a,_RD(a)|0x2);} return; // SET 1,(IY+d)
            case 0xcf: { uint16_t a=c->WZ=IY+d;; c->A=_RD(a)|0x2; _WR(a,c->A);} return; // SET 1,(IY+d),A
            case 0xd0: { uint16_t a=c->WZ=IY+d;; c->B=_RD(a)|0x4; _WR(a,c->B);} return; // SET 2,(IY+d),B
            case 0xd1: { uint16_t a=c->WZ=IY+d;; c->C=_RD(a)|0x4; _WR(a,c->C);} return; // SET 2,(IY+d),C
            case 0xd2: { uint16_t a=c->WZ=IY+d;; c->D=_RD(a)|0x4; _WR(a,c->D);} return; // SET 2,(IY+d),D
            case 0xd3: { uint16_t a=c->WZ=IY+d;; c->E=_RD(a)|0x4; _WR(a,c->E);} return; // SET 2,(IY+d),E
            case 0xd4: { uint16_t a=c->WZ=IY+d;; c->H=_RD(a)|0x4; _WR(a,c->IYH);} return; // SET 2,(IY+d),H
            case 0xd5: { uint16_t a=c->WZ=IY+d;; c->L=_RD(a)|0x4; _WR(a,c->IYL);} return; // SET 2,(IY+d),L
            case 0xd6: { uint16_t a=c->WZ=IY+d;; _WR(a,_RD(a)|0x4);} return; // SET 2,(IY+d)
            case 0xd7: { uint16_t a=c->WZ=IY+d;; c->A=_RD(a)|0x4; _WR(a,c->A);} return; // SET 2,(IY+d),A
            case 0xd8: { uint16_t a=c->WZ=IY+d;; c->B=_RD(a)|0x8; _WR(a,c->B);} return; // SET 3,(IY+d),B
            case 0xd9: { uint16_t a=c->WZ=IY+d;; c->C=_RD(a)|0x8; _WR(a,c->C);} return; // SET 3,(IY+d),C
            case 0xda: { uint16_t a=c->WZ=IY+d;; c->D=_RD(a)|0x8; _WR(a,c->D);} return; // SET 3,(IY+d),D
            case 0xdb: { uint16_t a=c->WZ=IY+d;; c->E=_RD(a)|0x8; _WR(a,c->E);} return; // SET 3,(IY+d),E
            case 0xdc: { uint16_t a=c->WZ=IY+d;; c->H=_RD(a)|0x8; _WR(a,c->IYH);} return; // SET 3,(IY+d),H
            case 0xdd: { uint16_t a=c->WZ=IY+d;; c->L=_RD(a)|0x8; _WR(a,c->IYL);} return; // SET 3,(IY+d),L
            case 0xde: { uint16_t a=c->WZ=IY+d;; _WR(a,_RD(a)|0x8);} return; // SET 3,(IY+d)
            case 0xdf: { uint16_t a=c->WZ=IY+d;; c->A=_RD(a)|0x8; _WR(a,c->A);} return; // SET 3,(IY+d),A
            case 0xe0: { uint16_t a=c->WZ=IY+d;; c->B=_RD(a)|0x10; _WR(a,c->B);} return; // SET 4,(IY+d),B
            case 0xe1: { uint16_t a=c->WZ=IY+d;; c->C=_RD(a)|0x10; _WR(a,c->C);} return; // SET 4,(IY+d),C
            case 0xe2: { uint16_t a=c->WZ=IY+d;; c->D=_RD(a)|0x10; _WR(a,c->D);} return; // SET 4,(IY+d),D
            case 0xe3: { uint16_t a=c->WZ=IY+d;; c->E=_RD(a)|0x10; _WR(a,c->E);} return; // SET 4,(IY+d),E
            case 0xe4: { uint16_t a=c->WZ=IY+d;; c->H=_RD(a)|0x10; _WR(a,c->IYH);} return; // SET 4,(IY+d),H
            case 0xe5: { uint16_t a=c->WZ=IY+d;; c->L=_RD(a)|0x10; _WR(a,c->IYL);} return; // SET 4,(IY+d),L
            case 0xe6: { uint16_t a=c->WZ=IY+d;; _WR(a,_RD(a)|0x10);} return; // SET 4,(IY+d)
            case 0xe7: { uint16_t a=c->WZ=IY+d;; c->A=_RD(a)|0x10; _WR(a,c->A);} return; // SET 4,(IY+d),A
            case 0xe8: { uint16_t a=c->WZ=IY+d;; c->B=_RD(a)|0x20; _WR(a,c->B);} return; // SET 5,(IY+d),B
            case 0xe9: { uint16_t a=c->WZ=IY+d;; c->C=_RD(a)|0x20; _WR(a,c->C);} return; // SET 5,(IY+d),C
            case 0xea: { uint16_t a=c->WZ=IY+d;; c->D=_RD(a)|0x20; _WR(a,c->D);} return; // SET 5,(IY+d),D
            case 0xeb: { uint16_t a=c->WZ=IY+d;; c->E=_RD(a)|0x20; _WR(a,c->E);} return; // SET 5,(IY+d),E
            case 0xec: { uint16_t a=c->WZ=IY+d;; c->H=_RD(a)|0x20; _WR(a,c->IYH);} return; // SET 5,(IY+d),H
            case 0xed: { uint16_t a=c->WZ=IY+d;; c->L=_RD(a)|0x20; _WR(a,c->IYL);} return; // SET 5,(IY+d),L
            case 0xee: { uint16_t a=c->WZ=IY+d;; _WR(a,_RD(a)|0x20);} return; // SET 5,(IY+d)
            case 0xef: { uint16_t a=c->WZ=IY+d;; c->A=_RD(a)|0x20; _WR(a,c->A);} return; // SET 5,(IY+d),A
            case 0xf0: { uint16_t a=c->WZ=IY+d;; c->B=_RD(a)|0x40; _WR(a,c->B);} return; // SET 6,(IY+d),B
            case 0xf1: { uint16_t a=c->WZ=IY+d;; c->C=_RD(a)|0x40; _WR(a,c->C);} return; // SET 6,(IY+d),C
            case 0xf2: { uint16_t a=c->WZ=IY+d;; c->D=_RD(a)|0x40; _WR(a,c->D);} return; // SET 6,(IY+d),D
            case 0xf3: { uint16_t a=c->WZ=IY+d;; c->E=_RD(a)|0x40; _WR(a,c->E);} return; // SET 6,(IY+d),E
            case 0xf4: { uint16_t a=c->WZ=IY+d;; c->H=_RD(a)|0x40; _WR(a,c->IYH);} return; // SET 6,(IY+d),H
            case 0xf5: { uint16_t a=c->WZ=IY+d;; c->L=_RD(a)|0x40; _WR(a,c->IYL);} return; // SET 6,(IY+d),L
            case 0xf6: { uint16_t a=c->WZ=IY+d;; _WR(a,_RD(a)|0x40);} return; // SET 6,(IY+d)
            case 0xf7: { uint16_t a=c->WZ=IY+d;; c->A=_RD(a)|0x40; _WR(a,c->A);} return; // SET 6,(IY+d),A
            case 0xf8: { uint16_t a=c->WZ=IY+d;; c->B=_RD(a)|0x80; _WR(a,c->B);} return; // SET 7,(IY+d),B
            case 0xf9: { uint16_t a=c->WZ=IY+d;; c->C=_RD(a)|0x80; _WR(a,c->C);} return; // SET 7,(IY+d),C
            case 0xfa: { uint16_t a=c->WZ=IY+d;; c->D=_RD(a)|0x80; _WR(a,c->D);} return; // SET 7,(IY+d),D
            case 0xfb: { uint16_t a=c->WZ=IY+d;; c->E=_RD(a)|0x80; _WR(a,c->E);} return; // SET 7,(IY+d),E
            case 0xfc: { uint16_t a=c->WZ=IY+d;; c->H=_RD(a)|0x80; _WR(a,c->IYH);} return; // SET 7,(IY+d),H
            case 0xfd: { uint16_t a=c->WZ=IY+d;; c->L=_RD(a)|0x80; _WR(a,c->IYL);} return; // SET 7,(IY+d),L
            case 0xfe: { uint16_t a=c->WZ=IY+d;; _WR(a,_RD(a)|0x80);} return; // SET 7,(IY+d)
            case 0xff: { uint16_t a=c->WZ=IY+d;; c->A=_RD(a)|0x80; _WR(a,c->A);} return; // SET 7,(IY+d),A
            default: return _INVALID_OPCODE(4);
          }
          break;
          }
        case 0xcc: _z80_callcc(c, (c->F)&Z80_ZF)); return; // CALL Z,nn
        case 0xcd: _z80_call(c); return; // CALL nn
        case 0xce: _z80_adc(_RD(c->PC++)); return; // ADC n
        case 0xcf: _z80_rst(0x8); return; // RST 0x8
        case 0xd0: _z80_retcc(c,!(c->F&Z80_CF)); return; // RET NC
        case 0xd1: uint8_t l=_RD(c->SP++);uint8_t h=_RD(c->SP++);c->DE=(h<<8)|l; return; // POP DE
        case 0xd2: _IMM16(); if (!(c->F&Z80_CF)) { c->PC=c->WZ; } return; // JP NC,nn
        case 0xd3: _z80_out(c, (c->A<<8)|_RD(c->PC++), c->A); return; // OUT (n),A
        case 0xd4: _z80_callcc(c, !(c->F&Z80_CF)); return; // CALL NC,nn
        case 0xd5: _WR(--c->SP,DE<<8); _WR(--c->SP,DE); return; // PUSH DE
        case 0xd6: _z80_sub(_RD(c->PC++)); return; // SUB n
        case 0xd7: _z80_rst(0x10); return; // RST 0x10
        case 0xd8: _z80_retcc(c,(c->F&Z80_CF)); return; // RET C
        case 0xd9: _SWP16(c->BC,c->BC_); _SWP16(c->DE,c->DE_); SWP16(c->HL,c->HL_); SWP16(c->WZ,c->WZ_); return; // EXX
        case 0xda: _IMM16(); if ((c->F&Z80_CF)) { c->PC=c->WZ; } return; // JP C,nn
        case 0xdb: c->A=_z80_in(c, (c->A<<8)|_RD(c->PC++)); return; // IN A,(n)
        case 0xdc: _z80_callcc(c, (c->F&Z80_CF)); return; // CALL C,nn
        case 0xde: _z80_sbc(_RD(c->PC++)); return; // SBC n
        case 0xdf: _z80_rst(0x18); return; // RST 0x18
        case 0xe0: _z80_retcc(c,!(c->F&Z80_PF)); return; // RET PO
        case 0xe1: uint8_t l=_RD(c->SP++);uint8_t h=_RD(c->SP++);c->IY=(h<<8)|l; return; // POP IY
        case 0xe2: _IMM16(); if (!(c->F&Z80_PF)) { c->PC=c->WZ; } return; // JP PO,nn
        case 0xe3: c->IY=_z80_exsp(c,c->IY); return; // EX (SP),IY
        case 0xe4: _z80_callcc(c, !(c->F&Z80_PF)); return; // CALL PO,nn
        case 0xe5: _WR(--c->SP,IY<<8); _WR(--c->SP,IY); return; // PUSH IY
        case 0xe6: _z80_and(_RD(c->PC++)); return; // AND n
        case 0xe7: _z80_rst(0x20); return; // RST 0x20
        case 0xe8: _z80_retcc(c,(c->F&Z80_PF)); return; // RET PE
        case 0xe9: c->PC=c->IY; return; // JP IY
        case 0xea: _IMM16(); if ((c->F&Z80_PF)) { c->PC=c->WZ; } return; // JP PE,nn
        case 0xeb: _SWP16(c->DE,c->HL); return; // EX DE,HL
        case 0xec: _z80_callcc(c, (c->F&Z80_PF)); return; // CALL PE,nn
        case 0xee: _z80_xor(_RD(c->PC++)); return; // XOR n
        case 0xef: _z80_rst(0x28); return; // RST 0x28
        case 0xf0: _z80_retcc(c,!(c->F&Z80_SF)); return; // RET P
        case 0xf1: uint8_t l=_RD(c->SP++);uint8_t h=_RD(c->SP++);c->AF=(h<<8)|l; return; // POP AF
        case 0xf2: _IMM16(); if (!(c->F&Z80_SF)) { c->PC=c->WZ; } return; // JP P,nn
        case 0xf3: _z80_di(); return; // DI
        case 0xf4: _z80_callcc(c, !(c->F&Z80_SF)); return; // CALL P,nn
        case 0xf5: _WR(--c->SP,AF<<8); _WR(--c->SP,AF); return; // PUSH AF
        case 0xf6: _z80_or(_RD(c->PC++)); return; // OR n
        case 0xf7: _z80_rst(0x30); return; // RST 0x30
        case 0xf8: _z80_retcc(c,(c->F&Z80_SF)); return; // RET M
        case 0xf9: _T(2);c->SP=c->IY; return; // LD SP,IY
        case 0xfa: _IMM16(); if ((c->F&Z80_SF)) { c->PC=c->WZ; } return; // JP M,nn
        case 0xfb: _z80_ei(); return; // EI
        case 0xfc: _z80_callcc(c, (c->F&Z80_SF)); return; // CALL M,nn
        case 0xfe: _z80_cp(_RD(c->PC++)); return; // CP n
        case 0xff: _z80_rst(0x38); return; // RST 0x38
        default: return _INVALID_OPCODE(2);
      }
      break;
    case 0xfe: _z80_cp(_RD(c->PC++)); return; // CP n
    case 0xff: _z80_rst(0x38); return; // RST 0x38
    default: return _INVALID_OPCODE(1);
  }
}
