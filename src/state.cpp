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

		int last;
		for (last=EXO_NUM_REGISTERS-1; last>=0 && registers[last].get_type() == exo::type::NIL; last--);

		for (int i=0; i<=last; ++i) {
			std::cout << "[" << i << "]\t" << registers[i].to_string() << std::endl;
		}
	}
}
