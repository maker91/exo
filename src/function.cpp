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
			
			std::cout << I << " (" << OP << ")" << std::endl;
		
			switch (OP) {
			case opcodes::NOOP:
				break;
				
			case opcodes::RTN:
				return GET_A(I);
				
			case opcodes::JMP:
				pc += GET_Bx(I);
				break;
				
			case opcodes::LOADK:
				E->set(GET_A(I), k_store[GET_B(I)]);
				break;
				
			case opcodes::MOVE:
				E->set(GET_B(I), E->get(GET_A(I)));
				break;
			}
			
			pc++;
		}
	}
}
