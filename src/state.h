#pragma once

#include <stack>

#include "stack.h"
#include "value.h"
#include "config.h"

namespace exo {

	class state {
		friend class value;
	
	// make this private
	public:
		exo::value	registers[EXO_NUM_REGISTERS];
		exo::stack 	stack;
		exo::map	builtins;
		
	public:
		void register_builtin(const std::string &name, const exo::value &builtin);
		void print_stack();
		void print_registers();
	};
}
