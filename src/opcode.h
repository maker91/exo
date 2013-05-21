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
