#pragma once

#include <stack>

#include "stack.h"
#include "value.h"

namespace exo {

	class state {
		friend class value;
	
	// make this private
	public:
		exo::stack 	stack;
		exo::map	builtins;
		
	public:
		void push(const value &);
		void pop();
		
		void set(int, const value &);
		value get(int);

		void push_scope(int i);
		void pop_scope();

		void register_builtin(const std::string &name, const exo::value &builtin);
	};
}
