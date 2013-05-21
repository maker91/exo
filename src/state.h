#pragma once

#include "stack.h"
#include "value.h"

namespace exo {

	class state {
		friend class value;
	
	public:
		exo::stack stack;
		
	public:
		void push(const value &);
		void pop();
		
		value get(int);
	};
}
