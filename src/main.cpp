#include <iostream>
#include <string>

#include "value.h"
#include "state.h"
#include "opcode.h"
#include "function.h"

int main() {
	exo::state E;
	
	E.stack.print_stack();
	std::cout << std::endl;

	try {
		exo::function f({
			MAKE_ABC(exo::opcodes::LOADK, 0, 0, 0, 0, 0),
			MAKE_AtBx(exo::opcodes::TEST, 0, 0, 0, 3),
			MAKE_ABC(exo::opcodes::LOADK, 0, 0, 1, 0, 0),
			MAKE_ABx(exo::opcodes::JMP, 0, 0, 2),
			MAKE_ABC(exo::opcodes::LOADK, 0, 0, 2, 0, 0),
			MAKE_ABC(exo::opcodes::RTN, 1, 0, 0, 0, 0)
		}, {true, "TRUE", "FALSE"});
		
		f.call(&E);
	} catch (std::exception &e) {
		std::cout << "error: " << e.what() << std::endl;
	}
	
	std::cout << std::endl;
	E.stack.print_stack();

	return 0;
}
