#include "function.h"

namespace exo {

	function::function() {
		i_store.push_back(opcodes::RTN << 2);
		pc = &i_store[0];
	}
	
	function::function(const std::vector<instruction> &s)
		: i_store(s)
	{
		function();
	}
	
	function::~function() {
	
	}
	
	int function::call(state *E) {
		while (true) {	
			switch (*pc) {
			case opcodes::NOOP:
				break;
				
			case opcodes::RTN:
				return 0;
				
			case opcodes::JMP:
				pc += GET_Bx(*pc);
				break;
			}
			
			pc++;
		}
	}
}
