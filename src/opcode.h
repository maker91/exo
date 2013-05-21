#pragma once

#include <cstdint>
#include "type.h"

/*
	instruction format:
		OP R[A] kR[B] kR[C]
		OOOOOO AAAAAAAA KBBBBBBBB KCCCCCCCC
		
		OP R[A] SBx
		OOOOOO AAAAAAAA SBBBBBBBB BBBBBBBB
*/

#define GET_OP(i) 	i>>26
#define GET_A(i)	(i>>18) & 0xFF
#define GET_B(i)	(i>>9) & 0xFF
#define GET_Bx(i)	i & 0xFFFF

#define IS_BK(i)	(i>>17) & 0x01
#define IS_CK(i)	(i>>8) & 0x01
#define IS_BxS(i)	(i>>16) & 0x01

#define MAKE_ABC(i, A, bk, B, ck, C)	((i&0x3F)<<26)&((A&0xFF)<<18)&((bk&0x01)<<17)&((B&0xFF)<<9)&((ck&0x01)<<8)&(C&0xFF)
#define MAKE_ABx(i, A, bs, B)			((i&0x3F)<<26)&((A&0xFF)<<18)&((bs&0x01)<<17)&(B&0xFFFF)			

namespace exo {
	typedef std::uint32_t instruction;

	namespace opcodes {
		
		enum opcode : byte {
			NOOP,
			LOADK,
			MOVE,
			JMP,
			RTN
		};
	}	
}
