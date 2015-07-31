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
#define GET_Bx(i)	(((i>>16) & 0x01)==0x01 ? -GET_uBx(i) : GET_uBx(i))
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
		enum opcode {
			NOOP,		// Do nothing
			
			MOVE,		// R[A] = RK[B - C]
			PUSH,		// R[top] = RK[B - C]
			
			JMP,		// pc += Bx
			TEST,		// if R[A] == Bool(T) then pc += Bx
			RTN,		// return R[top - A-1]...R[top] 							(A-1==-1 -> return whole stack minus parameters)
			CALL,		// R[top]...R[top + A-1] = R[B](R[top - C-1]...R[top]) 		(A-1==-1 -> accept all returns) (C-1==-1 -> pass whole stack as parameters)
			
			EQL,		// R[A] = RK[B]==RK[C]
			LT,			// R[A] = RK[B]<RK[C]
			LE,			// R[A] = RK[B]<=RK[C]
			
			AND,		// R[A] = RK[B] && RK[C]
			OR,			// R[A] = RK[B] || RK[C]
			NOT,		// R[A] = !RK[B]
			
			BAND,		// R[A] = RK[B] & RK[C]
			BOR,		// R[A] = RK[B] | RK[C]
			BXOR,		// R[A] = RK[B] @ RK[C]
			BNOT,		// R[A] = ~RK[B]

			LSHIFT,		// R[A] = RK[B] << RK[C]
			RSHIFT,		// R[A] = RK[B] << RK[C]
			
			ADD,		// R[A] = RK[B] + RK[C]
			SUB,		// R[A] = RK[B] + RK[C]
			MUL,		// R[A] = RK[B] + RK[C]
			DIV,		// R[A] = RK[B] + RK[C]
			POW,		// R[A] = RK[B] ^ RK[C]
			MOD,		// R[A] = RK[B] % RK[C]
			UNM,		// R[A] = -RK[B]

			INCR,		// R[A] = R[A] + 1
			DECR,		// R[A] = R[A] - 1
			
			NEWLIST,	// R[A] = list(R[top - B] .. R[top])
			NEWMAP,		// R[A] = map
			SET,		// R[A][RK[B]] = RK[C]
			GET,		// R[A] = R[B][RK[C]]
			
			LEN,		// R[A] = length(RK[B])
			CONCAT,		// R[A] = RK[B] .. RK[C]
			
			SETGLOBAL,	// _G[RK[B]] = RK[C]
			GETGLOBAL,	// R[A] = _G[RK[B]]
			
			COUNT		// opcode count
		};
	}	
	
	inline string opcode_name(opcodes::opcode o) {
		switch (o) {
			case opcodes::NOOP:			return "NOOP";
			case opcodes::MOVE:			return "MOVE";
			case opcodes::PUSH:			return "PUSH";
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
			case opcodes::SET:			return "SET";
			case opcodes::GET:			return "GET";
			case opcodes::LEN:			return "LEN";
			case opcodes::CONCAT:		return "CONCAT";
			case opcodes::SETGLOBAL:	return "SETGLOB";
			case opcodes::GETGLOBAL:	return "GETGLOB";
			
			case opcodes::COUNT:
			default:					return "INVALID";
		}
	}
}
