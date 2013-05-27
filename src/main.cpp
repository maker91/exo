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
			MAKE_ABC(exo::opcodes::NEWMAP, 0, 0, 0, 0, 0),
			MAKE_ABC(exo::opcodes::LEN, 1, 0, 0, 0, 0),
			MAKE_ABC(exo::opcodes::SET, 0, 1, 0, 1, 1),
			MAKE_ABC(exo::opcodes::LEN, 2, 0, 0, 0, 0),
			MAKE_ABC(exo::opcodes::GET, 3, 0, 0, 1, 0),
			MAKE_ABC(exo::opcodes::RTN, 1, 0, 0, 0, 0),
		}, {
			"key", "value"
		});
		
		f.call(&E);
	} catch (std::exception &e) {
		std::cout << "error: " << e.what() << std::endl;
	}
	
	std::cout << std::endl;
	E.stack.print_stack();

	return 0;
}
