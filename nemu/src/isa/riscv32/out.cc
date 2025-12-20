// Auto-generated code for RISC-V instructions

#include <stdint.h>

#include "IDLHlper.hpp"
#include "encoding.out.h" 

#define INST() (instruction)
#define INST_IS(name) (INST() & MASK_##name) == MATCH_##name

#define InstAt(bit) Bits<1>(((instruction) >> (bit)) & 0x1)
#define InstRng(high, low) Bits<(high) - (low) + 1>(selbits((instruction), (high), (low)))

#define _UNUSED_INTVAL_ 0

// -4 will be added at the end of execute_instruction
#define jump_halfword(addr) do{ *pc = (addr) - 4; }while(0)
#define jump(addr) do{ *pc = (addr) - 4; }while(0)

#define GOOD_END() do { *pc += 4; return EXEC_SUCCESS; } while(0)

#define EXEC_SUCCESS 0
#define EXEC_NOMATCH -1

// return 0 if instruction matched and executed
extern "C" int execute_instruction(word_t instruction, word_t* pc, word_t* regs) {
    auto X = regs;
    Bits<5> xs2 = Bits<5>(InstRng(24, 20));
    Bits<5> xs1 = Bits<5>(InstRng(19, 15));
    Bits<5> xd  = Bits<5>(InstRng(11, 7));

	if (INST_IS(ADDW)) { 
		// variables:
		// operation:
		XReg operand1 = sext(X[xs1], 32);
		XReg operand2 = sext(X[xs2], 32);
		X[xd] = sext(operand1 + operand2, 32);
		GOOD_END();
	}
	if (INST_IS(SLTI)) { 
		// variables:
		Bits<12> imm = Bits<12>(InstRng(31, 20));
		// operation:
		X[xd] = (as_signed(X[xs1]) < as_signed(imm)) ? 1 : 0;
		GOOD_END();
	}
	if (INST_IS(SUBW)) { 
		// variables:
		// operation:
		Bits<32> t0 = X[xs1] * Rng(31, 0);
		Bits<32> t1 = X[xs2] * Rng(31, 0);
		X[xd] = sext(t0 - t1, 32);
		GOOD_END();
	}
	if (INST_IS(SRAW)) { 
		// variables:
		// operation:
		XReg operand1 = sext(X[xs1], 32);
		
		X[xd] = sext(operand1 SRA X[xs2] * Rng(4, 0), 32);
		GOOD_END();
	}
	if (INST_IS(LWU)) { 
		// variables:
		Bits<12> imm = Bits<12>(InstRng(31, 20));
		// operation:
		XReg virtual_address = X[xs1] + as_signed(imm);
		
		X[xd] = read_memory<32>(virtual_address, _UNUSED_INTVAL_);
		GOOD_END();
	}
	if (INST_IS(AND)) { 
		// variables:
		// operation:
		X[xd] = X[xs1] & X[xs2];
		GOOD_END();
	}
	if (INST_IS(SLL)) { 
		// variables:
		// operation:
		if (xlen() == 64) {
		  X[xd] = X[xs1] << X[xs2] * Rng(5, 0);
		} else {
		  X[xd] = X[xs1] << X[xs2] * Rng(4, 0);
		}
		GOOD_END();
	}
	if (INST_IS(SRLIW)) { 
		// variables:
		Bits<5> shamt = Bits<5>(InstRng(24, 20));
		// operation:
		// #  shamt is between 0-31
		XReg operand = X[xs1] * Rng(31, 0);
		
		X[xd] = sext(operand >> shamt, 32);
		GOOD_END();
	}
	if (INST_IS(LB)) { 
		// variables:
		Bits<12> imm = Bits<12>(InstRng(31, 20));
		// operation:
		XReg virtual_address = X[xs1] + as_signed(imm);
		
		X[xd] = sext(read_memory<8>(virtual_address, _UNUSED_INTVAL_), 8);
		GOOD_END();
	}
	if (INST_IS(XORI)) { 
		// variables:
		Bits<12> imm = Bits<12>(InstRng(31, 20));
		// operation:
		X[xd] = X[xs1] ^ as_signed(imm);
		GOOD_END();
	}
	if (INST_IS(SH)) { 
		// variables:
		Bits<12> imm = Bits<12>(Concat{InstRng(31, 25), InstRng(11, 7)});
		// operation:
		XReg virtual_address = X[xs1] + as_signed(imm);
		
		write_memory<16>(virtual_address, X[xs2] * Rng(15, 0), _UNUSED_INTVAL_);
		GOOD_END();
	}
	if (INST_IS(SLLIW)) { 
		// variables:
		Bits<5> shamt = Bits<5>(InstRng(24, 20));
		// operation:
		// #  shamt is between 0-32
		X[xd] = sext(X[xs1] << shamt, 32);
		GOOD_END();
	}
	if (INST_IS(SB)) { 
		// variables:
		Bits<12> imm = Bits<12>(Concat{InstRng(31, 25), InstRng(11, 7)});
		// operation:
		XReg virtual_address = X[xs1] + as_signed(imm);
		
		write_memory<8>(virtual_address, X[xs2] * Rng(7, 0), _UNUSED_INTVAL_);
		GOOD_END();
	}
	if (INST_IS(BLT)) { 
		// variables:
		XReg imm = Bits<12>(Concat{InstAt(31), InstAt(7), InstRng(30, 25), InstRng(11, 8)})
			.sign_extend()
			.left_shift(1);
		// operation:
		XReg lhs = X[xs1];
		XReg rhs = X[xs2];
		
		if (as_signed(lhs) < as_signed(rhs)) {
		  jump_halfword(*pc + as_signed(imm));
		}
		GOOD_END();
	}
	if (INST_IS(XOR)) { 
		// variables:
		// operation:
		X[xd] = X[xs1] ^ X[xs2];
		GOOD_END();
	}
	if (INST_IS(SRAI)) { 
		// variables:
		Bits<5> shamt = Bits<5>(InstRng(24, 20));
		// operation:
		// #  shamt is between 0-63
		X[xd] = X[xs1] SRA shamt;
		GOOD_END();
	}
	if (INST_IS(LUI)) { 
		// variables:
		Bits<20> imm = Bits<20>(InstRng(31, 12))
			.left_shift(12);
		// operation:
		X[xd] = as_signed(imm);
		GOOD_END();
	}
	if (INST_IS(SRLW)) { 
		// variables:
		// operation:
		X[xd] = sext(X[xs1] * Rng(31, 0) >> X[xs2] * Rng(4, 0), 32);
		GOOD_END();
	}
	if (INST_IS(ORI)) { 
		// variables:
		Bits<12> imm = Bits<12>(InstRng(31, 20));
		// operation:
		X[xd] = X[xs1] | as_signed(imm);
		GOOD_END();
	}
	if (INST_IS(SLTU)) { 
		// variables:
		// operation:
		X[xd] = (X[xs1] < X[xs2]) ? 1 : 0;
		GOOD_END();
	}
	if (INST_IS(OR)) { 
		// variables:
		// operation:
		X[xd] = X[xs1] | X[xs2];
		GOOD_END();
	}
	if (INST_IS(JALR)) { 
		// variables:
		Bits<12> imm = Bits<12>(InstRng(31, 20));
		// operation:
		XReg addr = (X[xs1] + as_signed(imm)) & ~Bits<MXLEN>(1);
		XReg returnaddr;
		returnaddr = *pc + 4;
		
		X[xd] = returnaddr;
		jump(addr);
		GOOD_END();
	}
	if (INST_IS(LH)) { 
		// variables:
		Bits<12> imm = Bits<12>(InstRng(31, 20));
		// operation:
		XReg virtual_address = X[xs1] + as_signed(imm);
		
		X[xd] = sext(read_memory<16>(virtual_address, _UNUSED_INTVAL_), 16);
		GOOD_END();
	}
	if (INST_IS(SLTIU)) { 
		// variables:
		Bits<12> imm = Bits<12>(InstRng(31, 20));
		// operation:
		Bits<MXLEN> sign_extend_imm = as_signed(imm);
		X[xd] = (X[xs1] < sign_extend_imm) ? 1 : 0;
		GOOD_END();
	}
	if (INST_IS(BNE)) { 
		// variables:
		XReg imm = Bits<12>(Concat{InstAt(31), InstAt(7), InstRng(30, 25), InstRng(11, 8)})
			.sign_extend()
			.left_shift(1);
		// operation:
		XReg lhs = X[xs1];
		XReg rhs = X[xs2];
		
		if (lhs != rhs) {
		  jump_halfword(*pc + as_signed(imm));
		}
		GOOD_END();
	}
	if (INST_IS(ADDI)) { 
		// variables:
		Bits<12> imm = Bits<12>(InstRng(31, 20));
		// operation:
		X[xd] = X[xs1] + as_signed(imm);
		GOOD_END();
	}
	if (INST_IS(SLLI)) { 
		// variables:
		Bits<5> shamt = Bits<5>(InstRng(24, 20));
		// operation:
		// #  shamt is between 0-(XLEN-1)
		X[xd] = X[xs1] << shamt;
		GOOD_END();
	}
	if (INST_IS(LHU)) { 
		// variables:
		Bits<12> imm = Bits<12>(InstRng(31, 20));
		// operation:
		XReg virtual_address = X[xs1] + as_signed(imm);
		
		X[xd] = read_memory<16>(virtual_address, _UNUSED_INTVAL_);
		GOOD_END();
	}
	if (INST_IS(SUB)) { 
		// variables:
		// operation:
		XReg t0 = X[xs1];
		XReg t1 = X[xs2];
		X[xd] = t0 - t1;
		GOOD_END();
	}
	if (INST_IS(SLLW)) { 
		// variables:
		// operation:
		X[xd] = sext(X[xs1] << X[xs2] * Rng(4, 0), 32);
		GOOD_END();
	}
	if (INST_IS(BGEU)) { 
		// variables:
		Bits<12> imm = Bits<12>(Concat{InstAt(31), InstAt(7), InstRng(30, 25), InstRng(11, 8)})
			.left_shift(1);
		// operation:
		XReg lhs = X[xs1];
		XReg rhs = X[xs2];
		
		if (lhs >= rhs) {
		  jump_halfword(*pc + as_signed(imm));
		}
		GOOD_END();
	}
	if (INST_IS(AUIPC)) { 
		// variables:
		Bits<20> imm = Bits<20>(InstRng(31, 12))
			.left_shift(12);
		// operation:
		X[xd] = *pc + as_signed(imm);
		GOOD_END();
	}
	if (INST_IS(SRL)) { 
		// variables:
		// operation:
		if (xlen() == 64) {
		  X[xd] = X[xs1] >> X[xs2] * Rng(5, 0);
		} else {
		  X[xd] = X[xs1] >> X[xs2] * Rng(4, 0);
		}
		GOOD_END();
	}
	if (INST_IS(SRA)) { 
		// variables:
		// operation:
		if (xlen() == 64) {
		  X[xd] = X[xs1] SRA X[xs2] * Rng(5, 0);
		} else {
		  X[xd] = X[xs1] SRA X[xs2] * Rng(4, 0);
		}
		GOOD_END();
	}
	if (INST_IS(ADD)) { 
		// variables:
		// operation:
		X[xd] = X[xs1] + X[xs2];
		GOOD_END();
	}
	if (INST_IS(SLT)) { 
		// variables:
		// operation:
		XReg src1 = X[xs1];
		XReg src2 = X[xs2];
		
		X[xd] = (as_signed(src1) < as_signed(src2)) ? 1 : 0;
		GOOD_END();
	}
	if (INST_IS(BLTU)) { 
		// variables:
		Bits<12> imm = Bits<12>(Concat{InstAt(31), InstAt(7), InstRng(30, 25), InstRng(11, 8)})
			.left_shift(1);
		// operation:
		XReg lhs = X[xs1];
		XReg rhs = X[xs2];
		
		if (lhs < rhs) {
		  jump_halfword(*pc + as_signed(imm));
		}
		GOOD_END();
	}
	if (INST_IS(SRAIW)) { 
		// variables:
		Bits<5> shamt = Bits<5>(InstRng(24, 20));
		// operation:
		// #  shamt is between 0-32
		XReg operand = sext(X[xs1], 32);
		X[xd] = sext(operand SRA shamt, 32);
		GOOD_END();
	}
	if (INST_IS(BGE)) { 
		// variables:
		XReg imm = Bits<12>(Concat{InstAt(31), InstAt(7), InstRng(30, 25), InstRng(11, 8)})
			.sign_extend()
			.left_shift(1);
		// operation:
		XReg lhs = X[xs1];
		XReg rhs = X[xs2];
		
		if (as_signed(lhs) >= as_signed(rhs)) {
		  jump_halfword(*pc + as_signed(imm));
		}
		GOOD_END();
	}
	if (INST_IS(SRLI)) { 
		// variables:
		Bits<5> shamt = Bits<5>(InstRng(24, 20));
		// operation:
		// #  shamt is between 0-63
		X[xd] = X[xs1] >> shamt;
		GOOD_END();
	}
	if (INST_IS(BEQ)) { 
		// variables:
		XReg imm = Bits<12>(Concat{InstAt(31), InstAt(7), InstRng(30, 25), InstRng(11, 8)})
			.sign_extend()
			.left_shift(1);
		// operation:
		XReg lhs = X[xs1];
		XReg rhs = X[xs2];
		
		if (lhs == rhs) {
		  jump_halfword(*pc + as_signed(imm));
		}
		GOOD_END();
	}
	if (INST_IS(SW)) { 
		// variables:
		Bits<12> imm = Bits<12>(Concat{InstRng(31, 25), InstRng(11, 7)});
		// operation:
		XReg virtual_address = X[xs1] + as_signed(imm);

		std::cout << "XS2: " << std::hex << X[xs2] << std::dec << std::endl;
		
		write_memory<32>(virtual_address, X[xs2] * Rng(31, 0), _UNUSED_INTVAL_);
		GOOD_END();
	}
	if (INST_IS(LBU)) { 
		// variables:
		Bits<12> imm = Bits<12>(InstRng(31, 20));
		// operation:
		XReg virtual_address = X[xs1] + as_signed(imm);
		
		X[xd] = read_memory<8>(virtual_address, _UNUSED_INTVAL_);
		GOOD_END();
	}
	if (INST_IS(ANDI)) { 
		// variables:
		Bits<12> imm = Bits<12>(InstRng(31, 20));
		// operation:
		X[xd] = X[xs1] & as_signed(imm);
		GOOD_END();
	}
	if (INST_IS(JAL)) { 
		// variables:
		XReg imm = Bits<20>(Concat{InstAt(31), InstRng(19, 12), InstAt(20), InstRng(30, 21)})
			.sign_extend()
			.left_shift(1);
		// operation:
		XReg return_addr = *pc + 4;
		
		X[xd] = return_addr;
		jump_halfword(*pc + as_signed(imm));
		GOOD_END();
	}
	if (INST_IS(LW)) { 
		// variables:
		Bits<12> imm = Bits<12>(InstRng(31, 20));
		// operation:
		XReg virtual_address = X[xs1] + as_signed(imm);
		
		X[xd] = sext(read_memory<32>(virtual_address, _UNUSED_INTVAL_), 32);
		GOOD_END();
	}
	if (INST_IS(MULH)) { 
		// variables:
		// operation:
		
		
		// #  enlarge and sign extend the sources
		Bits<1> xs1_sign_bit = X[xs1] * At(xlen()-1);
		Bits<MXLEN WIDE_MUL 2> src1 = Concat{{Repl<xlen()>{xs1_sign_bit}}, X[xs1]};
		
		Bits<1> xs2_sign_bit = X[xs2] * At(xlen()-1);
		Bits<MXLEN WIDE_MUL 2> src2 = Concat{{Repl<xlen()>{xs2_sign_bit}}, X[xs2]};
		
		// #  grab the high half of the result, and put it in xd
		X[xd] = (src1 * src2) * Rng((xlen()*Bits<8>(2))-1, xlen());
		GOOD_END();
	}
	if (INST_IS(DIVUW)) { 
		// variables:
		// operation:
		
		
		Bits<32> src1 = X[xs1] * Rng(31, 0);
		Bits<32> src2 = X[xs2] * Rng(31, 0);
		
		if (src2 == 0) {
		  // #  division by zero. Since RISC-V does not have arithmetic exceptions, the result is defined
		  // #  to be the largest 32-bit unsigned value (sign extended to 64-bits)
		  X[xd] = Concat{Repl<64>{Bits<1>(1)}};
		
		} else {
		
		  Bits<32> result = src1 / src2;
		  Bits<1> sign_bit = result[31];
		
		  X[xd] = Concat{{Repl<32>{sign_bit}}, result};
		}
		GOOD_END();
	}
	if (INST_IS(DIVW)) { 
		// variables:
		// operation:
		
		
		Bits<32> src1 = X[xs1] * Rng(31, 0);
		Bits<32> src2 = X[xs2] * Rng(31, 0);
		
		if (src2 == 0) {
		  // #  division by zero. Since RISC-V does not have arithmetic exceptions, the result is defined
		  // #  to be -1
		  X[xd] = Concat{Repl<MXLEN>{Bits<1>(1)}};
		
		} else if ((src1 == Bits<32>(0x80000000)) && (src2 == Bits<32>(0xFFFFFFFF))) {
		   // # INT_MIN / -1 = INT_MIN
		   // # signed overflow. Since RISC-V does not have arithmetic exceptions, the result is defined
		   // # to be the most negative number (-2^(31))
		  X[xd] = sext(Bits<32>(0x80000000), 32);
		
		} else {
		  // #  no special case, just divide
		  X[xd] = sext(as_signed(src1) / as_signed(src2), 32);
		}
		GOOD_END();
	}
	if (INST_IS(MULW)) { 
		// variables:
		// operation:
		
		
		Bits<32> src1 = X[xs1] * Rng(31, 0);
		Bits<32> src2 = X[xs2] * Rng(31, 0);
		
		Bits<32> result = src1 * src2;
		Bits<1> sign_bit = result[31];
		
		// #  return the sign-extended result
		X[xd] = Concat{{Repl<32>{sign_bit}}, result};
		GOOD_END();
	}
	if (INST_IS(REMUW)) { 
		// variables:
		// operation:
		
		
		Bits<32> src1 = X[xs1] * Rng(31, 0);
		Bits<32> src2 = X[xs2] * Rng(31, 0);
		
		if (src2 == 0) {
		  // #  division by zero. Since RISC-V does not have arithmetic exceptions, the result is defined
		  // #  to be the dividend
		  Bits<1> sign_bit = src1[31];
		  X[xd] = Concat{{Repl<32>{sign_bit}}, src1};
		
		} else {
		  // #  no special case
		
		  Bits<32> result = src1 % src2;
		
		  Bits<1> sign_bit = result[31];
		
		  X[xd] = Concat{{Repl<32>{sign_bit}}, result};
		}
		GOOD_END();
	}
	if (INST_IS(MULHU)) { 
		// variables:
		// operation:
		
		
		// #  enlarge and zero-extend the sources
		Bits<MXLEN*Bits<8>(2)> src1 = Concat{{Repl<MXLEN>{Bits<1>(0)}}, X[xs1]};
		Bits<MXLEN*Bits<8>(2)> src2 = Concat{{Repl<MXLEN>{Bits<1>(0)}}, X[xs2]};
		
		X[xd] = (src1 * src2) * Rng((MXLEN*Bits<8>(2))-1, MXLEN);
		GOOD_END();
	}
	if (INST_IS(REMW)) { 
		// variables:
		// operation:
		
		
		Bits<32> src1 = X[xs1] * Rng(31, 0);
		Bits<32> src2 = X[xs2] * Rng(31, 0);
		
		if (src2 == 0) {
		  // #  division by zero. Since RISC-V does not have arithmetic exceptions, the result is defined
		  // #  to be the dividend, sign extended to into the 64-bit register
		  Bits<1> sign_bit = src1[31];
		  X[xd] = Concat{{Repl<32>{sign_bit}}, src1};
		
		} else if ((src1 == Bits<32>(0x80000000)) && (src2 == Bits<32>(0xFFFFFFFF))) {
		  // #  signed overflow. Since RISC-V does not have arithmetic exceptions, the result is defined
		  // #  to be zero
		  X[xd] = 0;
		
		} else {
		  // #  no special case
		  Bits<32> result = as_signed(src1) % as_signed(src2);
		  Bits<1> sign_bit = result[31];
		
		  X[xd] = Concat{{Repl<32>{sign_bit}}, result};
		}
		GOOD_END();
	}
	if (INST_IS(MUL)) { 
		// variables:
		// operation:
		
		
		XReg src1 = X[xs1];
		XReg src2 = X[xs2];
		
		X[xd] = (src1 * src2) * Rng(MXLEN-1, 0);
		GOOD_END();
	}
	if (INST_IS(DIVU)) { 
		// variables:
		// operation:
		
		
		XReg src1 = X[xs1];
		XReg src2 = X[xs2];
		
		if (src2 == 0) {
		  // #  division by zero. Since RISC-V does not have arithmetic exceptions, the result is defined
		  // #  to be -1
		  X[xd] = Concat{Repl<MXLEN>{Bits<1>(1)}};
		} else {
		  X[xd] = src1 / src2;
		}
		GOOD_END();
	}
	if (INST_IS(DIV)) { 
		// variables:
		// operation:
		
		
		XReg src1 = X[xs1];
		XReg src2 = X[xs2];
		
		// #  smallest signed value
		XReg signed_min = (xlen() == 32) ? as_signed(Concat{Bits<1>(1), {Repl<31>{Bits<1>(0)}}}) : 1ll * Concat{Bits<1>(1), {Repl<63>{Bits<1>(0)}}};
		
		if (src2 == 0) {
		  // #  division by zero. Since RISC-V does not have arithmetic exceptions, the result is defined
		  // #  to be -1
		  X[xd] = Concat{Repl<MXLEN>{Bits<1>(1)}};
		
		} else if ((src1 == signed_min) && (src2 == Concat{Repl<MXLEN>{Bits<1>(1)}})) {
		  // #  signed overflow. Since RISC-V does not have arithmetic exceptions, the result is defined
		  // #  to be the most negative number (-2^(MXLEN-1))
		  X[xd] = signed_min;
		
		} else {
		  // #  no special case, just divide
		  X[xd] = as_signed(src1) / as_signed(src2);
		}
		GOOD_END();
	}
	if (INST_IS(MULHSU)) { 
		// variables:
		// operation:
		
		
		// #  enlarge and extend the sources
		Bits<1> xs1_sign_bit = X[xs1] * At(MXLEN-1);
		Bits<MXLEN*Bits<8>(2)> src1 = Concat{{Repl<MXLEN>{xs1_sign_bit}}, X[xs1]};
		Bits<MXLEN*Bits<8>(2)> src2 = Concat{{Repl<MXLEN>{Bits<1>(0)}}, X[xs2]};
		
		X[xd] = (src1 * src2) * Rng((MXLEN*Bits<8>(2))-1, MXLEN);
		GOOD_END();
	}
	if (INST_IS(REMU)) { 
		// variables:
		// operation:
		
		
		XReg src1 = X[xs1];
		XReg src2 = X[xs2];
		
		if (src2 == 0) {
		  // #  division by zero. Since RISC-V does not have arithmetic exceptions, the result is defined
		  // #  to be the dividend
		  X[xd] = src1;
		} else {
		  X[xd] = src1 % src2;
		}
		GOOD_END();
	}
	if (INST_IS(REM)) { 
		// variables:
		// operation:
		
		
		XReg src1 = X[xs1];
		XReg src2 = X[xs2];
		
		if (src2 == 0) {
		  // #  division by zero. Since RISC-V does not have arithmetic exceptions, the result is defined
		  // #  to be the dividend
		  X[xd] = src1;
		
		} else if ((src1 == Concat{Bits<1>(1), {Repl<MXLEN-1>{Bits<1>(0)}}}) && (src2 == Concat{Repl<MXLEN>{Bits<1>(1)}})) {
		  // #  signed overflow. Since RISC-V does not have arithmetic exceptions, the result is defined
		  // #  to be zero
		  X[xd] = 0;
		
		} else {
		  X[xd] = as_signed(src1) % as_signed(src2);
		}
		GOOD_END();
	}

	return EXEC_NOMATCH;
}
