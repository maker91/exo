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
#define GET_Bx(i)	(((i>>16) & 0x01)==0x01 ? -(i & 0xFFFF) : (i & 0xFFFF))
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
			
			LOADK,		// R[A] = K[B]
			LOADBOOL,	// R[A] = Bool(B)
			LOADNIL,	// R[A] = nil
			MOVE,		// R[B] = R[A]
			
			JMP,		// pc += Bx
			TEST,		// if R[A] == Bool(T) then pc += Bx
			RTN,		// return R[top - A-1]...R[top] 							(A-1==-1 -> return whole stack minus parameters)
			CALL,		// R[top]...R[top + A-1] = R[B](R[top - C-1]...R[top]) 		(A-1==-1 -> accept all returns) (C-1==-1 -> pass whole stack as parameters)
			
			EQL,		// R[A] = RK[B]==RK[C]
			LT,			// R[A] = RK[B]<RK[C]
			LE,			// R[A] = RK[B]<=RK[C]
			
			ADD,		// R[A] = RK[B] + RK[C]
			SUB,		// R[A] = RK[B] + RK[C]
			MUL,		// R[A] = RK[B] + RK[C]
			DIV,		// R[A] = RK[B] + RK[C]
			
			NEWLIST,	// R[A] = list
			NEWMAP,		// R[A] = map
			SET,		// R[A][RK[B]] = RK[C]
			GET,		// R[A] = R[B][RK[C]]
		};
	}	
}
