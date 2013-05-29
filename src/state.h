#pragma once

#include "stack.h"
#include "value.h"

namespace exo {

	class state {
		friend class value;
	
	// make this private
	public:
		exo::stack 	stack;
		exo::map	globals;
		
	public:
		void push(const value &);
		void pop();
		
		void set(int, const value &);
		value get(int);
		
		void set_global(const value &, const value &);
		value get_global(const value &);
	};
}
