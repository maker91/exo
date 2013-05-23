#include "function.h"

#include <iostream>

namespace exo {

	function::function() {
		i_store.push_back(opcodes::RTN << 26);
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
				
			case opcodes::RTN:
				return GET_A(I);
				
			case opcodes::JMP:
				pc += (int)GET_Bx(I) - 1;
				break;
				
			case opcodes::JZR:
				if (E->get(GET_A(I)).to_integer() == 0)
					pc += (int)GET_Bx(I) - 1;
				break;
				
			case opcodes::LOADK:
				E->set(GET_A(I), k_store[GET_B(I)]);
				break;
				
			case opcodes::MOVE:
				E->set(GET_B(I), E->get(GET_A(I)));
				break;
				
			case opcodes::ADD: 
				{
					value b;
					if (IS_BK(I))
						b = k_store[GET_B(I)];
					else
						b = E->get(GET_B(I));
						
					value c;
					if (IS_CK(I))
						c = k_store[GET_C(I)];
					else
						c = E->get(GET_C(I));
						
					E->set(GET_A(I), b+c);
					
					break;
				}
			}
			
			pc++;
		}
	}
}
