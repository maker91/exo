#include <iostream>
#include <string>

#include "value.h"
#include "state.h"
#include "opcode.h"
#include "function.h"

int main() {
	exo::state E;
	E.push("first");
	E.push("second");
	
	E.stack.print_stack();
	std::cout << std::endl;

	try {
		exo::function f({
			MAKE_ABC(exo::opcodes::MOVE, 1, 0, 0, 0, 0),
			MAKE_ABC(exo::opcodes::RTN, 0, 0, 0, 0, 0)
		});
		
		f.call(&E);
	} catch (std::exception &e) {
		std::cout << "error: " << e.what() << std::endl;
	}
	
	E.stack.print_stack();

	return 0;
}
