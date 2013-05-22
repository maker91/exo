#include <iostream>
#include <string>

#include "value.h"
#include "state.h"
#include "opcode.h"
#include "function.h"

int main() {
	exo::state E;
	E.push(1);
	
	E.stack.print_stack();
	std::cout << std::endl;

	try {
		exo::function f({
			MAKE_ABx(exo::opcodes::JZR, 0, 0, 1),
			MAKE_ABC(exo::opcodes::LOADK, 1, 0, 0, 0, 0),
			MAKE_ABC(exo::opcodes::RTN, 0, 0, 0, 0, 0)
		}, {
			"constant"
		});
		
		f.call(&E);
	} catch (std::exception &e) {
		std::cout << "error: " << e.what() << std::endl;
	}
	
	std::cout << std::endl;
	E.stack.print_stack();

	return 0;
}
