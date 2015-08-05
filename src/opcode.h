#pragma once

#include <cstdint>
#include "type.h"

/*
	instruction format:
		ABC:
			OP R[A] KR[B] KR[C]
			OOOOOO AAAAAAAA KBBBBBBBB KCCCCCCCC
		
		ABx:
			OP R[A] SBx
			OOOOOO AAAAAAAA T SBBBBBBBB BBBBBBBB
			
		O = opcode
		A = operand A
		B = operand B
		C = operand C
		K = constant?
		T = test for false or true?
		S = signed?
*/

#define GET_OP(i) 	exo::opcodes::opcode((i>>26) & 0xFF)
#define GET_A(i)	((i>>18) & 0xFF)
#define GET_B(i)	((i>>9) & 0xFF)
#define GET_uBx(i)	(i & 0xFFFF)
#define GET_Bx(i)	(IS_BxS(i) ? -GET_uBx(i) : GET_uBx(i))
#define GET_C(i)	(i & 0xFF)
#define GET_T(i)	((i>>17) & 0x01)

#define IS_BK(i)	((i>>17) & 0x01)
#define IS_CK(i)	((i>>8) & 0x01)
#define IS_BxS(i)	((i>>16) & 0x01)

#define MAKE_ABC(i, A, bk, B, ck, C)	(((i&0x3F)<<26)|((A&0xFF)<<18)|((bk&0x01)<<17)|((B&0xFF)<<9)|((ck&0x01)<<8)|(C&0xFF))
#define MAKE_ABx(i, A, bs, B)			(((i&0x3F)<<26)|((A&0xFF)<<18)|((bs&0x01)<<16)|(B&0xFFFF))
#define MAKE_AtBx(i, A, T, bs, B)		MAKE_ABx(i, A, bs, B)|((T&0x01)<<17)			

namespace exo {
	typedef std::uint32_t instruction;

	namespace opcodes {
		// S = stack, R = registers, K = constants
		// SR = use push/pop or register based on instruction
		// RK = use registers or constants based on instruction
		// SRK = use pop or register or constant based on instruction
		enum opcode {
			NOOP,		// Do nothing

			LOAD,		// SR[A] = SRK[B - C]
			BUILTIN,	// SR[A] = builtins[SRK[B]]
			
			JMP,		// pc += Bx
			TEST,		// if SR[A] == Bool(T) then pc += Bx
			RTN,		// return A-1 values from top of stack. if A-1==-1 -> return whole stack
			CALL,		// push A-1 returns of R[B](C args from the top of the stack) if A-1==-1 -> accept all returns.
			
			EQL,		// SR[A] = SRK[B] == SRK[C]
			LT,			// SR[A] = SRK[B] < SRK[C]
			LE,			// SR[A] = SRK[B] <= SRK[C]
			
			AND,		// SR[A] = SRK[B] && SRK[C]
			OR,			// SR[A] = SRK[B] || SRK[C]
			NOT,		// SR[A] = !SRK[B]
			
			BAND,		// SR[A] = SRK[B] & SRK[C]
			BOR,		// SR[A] = SRK[B] | SRK[C]
			BXOR,		// SR[A] = SRK[B] ^ SRK[C]
			BNOT,		// SR[A] = ~SRK[B]

			LSHIFT,		// SR[A] = SRK[B] << SRK[C]
			RSHIFT,		// SR[A] = SRK[B] >> SRK[C]
			
			ADD,		// SR[A] = SRK[B] + SRK[C]
			SUB,		// SR[A] = SRK[B] - SRK[C]
			MUL,		// SR[A] = SRK[B] * SRK[C]
			DIV,		// SR[A] = SRK[B] / SRK[C]
			POW,		// SR[A] = SRK[B] ** SRK[C]
			MOD,		// SR[A] = SRK[B] % SRK[C]
			UNM,		// SR[A] = -SRK[B]

			INCR,		// SR[A] = SR[A] + 1
			DECR,		// SR[A] = SR[A] - 1
			
			NEWLIST,	// SR[A] = list(S[-B] .. S[-1])
			NEWMAP,		// SR[A] = map(S[-B]: S[-B+1] .. S[-2]: S[-1])
			SETITEM,	// SR[A][SRK[B]] = SRK[C]
			GETITEM,	// SR[A] = SR[B][SRK[C]]
			
			LEN,		// SR[A] = length(SRK[B])
			CONCAT,		// SR[A] = concat(SRK[A], SRK[C])
			
			COUNT		// opcode count
		};
	}	
	
	inline string opcode_name(opcodes::opcode o) {
		switch (o) {
			case opcodes::NOOP:			return "NOOP";
			case opcodes::LOAD:			return "LOAD";
			case opcodes::BUILTIN:		return "BUILTIN";
			case opcodes::JMP:			return "JMP";
			case opcodes::TEST:			return "TEST";
			case opcodes::RTN:			return "RTN";
			case opcodes::CALL:			return "CALL";
			case opcodes::EQL:			return "EQL";
			case opcodes::LT:			return "LT";
			case opcodes::LE:			return "LE";
			case opcodes::AND:			return "AND";
			case opcodes::OR:			return "OR";
			case opcodes::NOT:			return "NOT";
			case opcodes::BAND:			return "BAND";
			case opcodes::BOR:			return "BOR";
			case opcodes::BXOR:			return "BXOR";
			case opcodes::BNOT:			return "BNOT";
			case opcodes::LSHIFT:		return "LSHIFT";
			case opcodes::RSHIFT:		return "RSHIFT";
			case opcodes::ADD:			return "ADD";
			case opcodes::SUB:			return "SUB";
			case opcodes::MUL:			return "MUL";
			case opcodes::DIV:			return "DIV";
			case opcodes::POW:			return "POW";
			case opcodes::MOD:			return "MOD";
			case opcodes::UNM:			return "UNM";
			case opcodes::INCR:			return "INCR";
			case opcodes::DECR:			return "DECR";
			case opcodes::NEWLIST:		return "NEWLIST";
			case opcodes::NEWMAP:		return "NEWMAP";
			case opcodes::SETITEM:		return "SETITEM";
			case opcodes::GETITEM:		return "GETITEM";
			case opcodes::LEN:			return "LEN";
			case opcodes::CONCAT:		return "CONCAT";
			
			case opcodes::COUNT:
			default:					return "INVALID";
		}
	}
}
