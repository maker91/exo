#pragma once

#include <stack>
#include "config.h"
#include "value.h"

namespace exo {
	class stack {
	private:
		value store[EXO_STACK_SIZE/sizeof(value)];
		unsigned top;
		std::stack<unsigned> frame;
	
	public:
		stack();
		stack(const stack &)=delete;
		stack(stack &&)=delete;
		
		~stack();
		
		unsigned size() const;
		unsigned frame_size() const;
		
		void push(const value &);
		void pop();
		
		value &get(int p);
		value &get_in_frame(int p);
		
		void set(int p, const value &);
		void set_in_frame(int p, const value &);
		
		void push_frame(unsigned args = 0u);
		void pop_frame(unsigned args = 0u, unsigned rets = 0u);
		
		/*
			debug purposes
		*/
		void print_stack() const;
	};
}
