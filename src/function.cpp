#include "function.h"

#include <iostream>
	
#define GET_RA(I)	(E->get(GET_A(I)))
#define GET_RB(I)	(E->get(GET_B(I)))
#define GET_RC(I)	(E->get(GET_C(I)))

#define GET_KB(I)	(k_store[GET_B(I)])
#define GET_KC(I)	(k_store[GET_C(I)])

#define GET_RKB(I)	(IS_BK(I) ? GET_KB(I) : GET_RB(I))
#define GET_RKC(I)	(IS_CK(I) ? GET_KC(I) : GET_RC(I))

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
				if (GET_RA(I).to_boolean() == (exo::boolean)GET_T(I))
					pc += (int)GET_Bx(I) - 1;
				break;
				
			case opcodes::LOADK:
				E->set(GET_A(I), k_store[GET_B(I)]);
				break;
				
			case opcodes::LOADBOOL:
				E->set(GET_A(I), (exo::boolean)GET_B(I));
				break;
				
			case opcodes::LOADNIL:
				E->set(GET_A(I), value());
				break;
				
			case opcodes::MOVE:
				E->set(GET_B(I), GET_RA(I));
				break;
				
			case opcodes::RTN:
				return GET_A(I)-1;
				
			case opcodes::CALL:
				GET_RB(I).call(E, GET_B(I)-1, GET_C(I)-1);
				break;
				
			case opcodes::EQL:
				E->set(GET_A(I), GET_RKB(I) == GET_RKC(I));
				break;
				
			case opcodes::LT:
				E->set(GET_A(I), GET_RKB(I) < GET_RKC(I));
				break;
				
			case opcodes::LE:
				E->set(GET_A(I), GET_RKB(I) <= GET_RKC(I));
				break;
				
			case opcodes::ADD: 	
				E->set(GET_A(I), GET_RKB(I) + GET_RKC(I));
				break;
					
			case opcodes::SUB: 
				E->set(GET_A(I), GET_RKB(I) - GET_RKC(I));
				break;
				
			case opcodes::MUL: 
				E->set(GET_A(I), GET_RKB(I) * GET_RKC(I));
				break;
				
			case opcodes::DIV: 
				E->set(GET_A(I), GET_RKB(I) / GET_RKC(I));
				break;
			}
			
			pc++;
		}
	}
}
