#include <iostream>

#include "function.h"
	
#define GET_RA(E, I)	(E->get(GET_A(I)))
#define GET_RB(E, I)	(E->get(GET_B(I)))
#define GET_RC(E, I)	(E->get(GET_C(I)))

#define GET_KB(I)		(k_store[GET_B(I)])
#define GET_KC(I)		(k_store[GET_C(I)])

#define GET_RKB(E, I)	(IS_BK(I) ? GET_KB(I) : GET_RB(E, I))
#define GET_RKC(E, I)	(IS_CK(I) ? GET_KC(I) : GET_RC(E, I))

#define SET_R(E, N, V)	(E->set(N, V))

namespace exo {

	function::function() {
		i_store.push_back(MAKE_ABC(opcodes::RTN, 1, 0, 0, 0, 0));
	}
	
	function::function(const std::vector<instruction> &s)
		: i_store(s)
	{
		function();
	}
	
	function::function(const std::vector<instruction> &s, const std::vector<value> &k)
		: i_store(s), k_store(k)
	{
		function();
	}
	
	function::~function() {
	
	}
	
	int function::call(state *E) {
		exo::instruction *pc = &i_store[0];
		
		while (true) {	
			exo::instruction I = *pc;
			exo::opcodes::opcode OP = GET_OP(I);
		
			switch (OP) {
			case opcodes::NOOP:
				break;
				
			case opcodes::JMP:
				pc += (int)GET_Bx(I) - 1;
				break;
				
			case opcodes::TEST:
				if (GET_RA(E, I).to_boolean() == (exo::boolean)GET_T(I))
					pc += (int)GET_Bx(I) - 1;
				break;
				
			case opcodes::LOADK:
				SET_R(E, GET_A(I), GET_KB(I));
				break;
				
			case opcodes::LOADBOOL:
				SET_R(E, GET_A(I), (exo::boolean)GET_B(I));
				break;
				
			case opcodes::LOADNIL:
				SET_R(E, GET_A(I), value());
				break;
				
			case opcodes::MOVE:
				SET_R(E, GET_B(I), GET_RA(E, I));
				break;
				
			case opcodes::RTN:
				return GET_A(I)-1;
				
			case opcodes::CALL:
				GET_RB(E, I).call(E, GET_B(I)-1, GET_C(I)-1);
				break;
				
			case opcodes::EQL:
				SET_R(E, GET_A(I), GET_RKB(E, I) == GET_RKC(E, I));
				break;
				
			case opcodes::LT:
				SET_R(E, GET_A(I), GET_RKB(E, I) < GET_RKC(E, I));
				break;
				
			case opcodes::LE:
				SET_R(E, GET_A(I), GET_RKB(E, I) <= GET_RKC(E, I));
				break;
				
			case opcodes::ADD: 	
				SET_R(E, GET_A(I), GET_RKB(E, I) + GET_RKC(E, I));
				break;
					
			case opcodes::SUB: 
				SET_R(E, GET_A(I), GET_RKB(E, I) - GET_RKC(E, I));
				break;
				
			case opcodes::MUL: 
				SET_R(E, GET_A(I), GET_RKB(E, I) * GET_RKC(E, I));
				break;
				
			case opcodes::DIV: 
				SET_R(E, GET_A(I), GET_RKB(E, I) / GET_RKC(E, I));
				break;
				
			case opcodes::NEWLIST:
				SET_R(E, GET_A(I), new list);
				break;
				
			case opcodes::NEWMAP:
				SET_R(E, GET_A(I), new map);
				break;
				
			case opcodes::SET:
				GET_RA(E, I).set(GET_RKB(E, I), GET_RKC(E, I));
				break;
				
			case opcodes::GET:
				SET_R(E, GET_A(I), GET_RKB(E, I).get(GET_RKC(E, I)));
				break;
			}
			
			pc++;
		}
	}
}
