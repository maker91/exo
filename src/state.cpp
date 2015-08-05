#include "state.h"
#include <iostream>

namespace exo {
	void state::register_builtin(const std::string &name, const exo::value &builtin) {
		builtins[name] = builtin;
	}

	void state::print_stack() {
		std::cout << "======= STACK:" << std::endl;
		stack.print_stack();
	}

	void state::print_registers() {
		std::cout << "======= REGISTERS:" << std::endl;
		for (int i=0; i<EXO_NUM_REGISTERS; ++i) {
			const exo::value &v = registers[i];
			if (v.get_type() == exo::type::NIL)
				break;

			std::cout << "[" << i << "]\t" << v.to_string() << std::endl;
		}
	}
}
