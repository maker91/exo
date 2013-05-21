#pragma once

#include "stack.h"
#include "value.h"

namespace exo {

	class state {
		friend class value;
	
	// make this private
	public:
		exo::stack stack;
		
	public:
		void push(const value &);
		void pop();
		
		void set(int, const value &);
		value get(int);
	};
}
