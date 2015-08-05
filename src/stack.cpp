#include "stack.h"
#include "exception.h"
#include "type.h"

#include <iostream>

namespace exo {
	
	stack::stack()
		: top(0u)
	{
		frame.push(0u);
	}
	
	stack::~stack() {
	
	}
	
	unsigned stack::size() const {
		return top;
	}
	
	unsigned stack::frame_size() const {
		return top - frame.top();
	}
	
	void stack::push(const value &v) {
		set(top, v);
	}
	
	value &stack::pop() {
		value &r = get(-1);
		top--;
		return r; 
	}

	int stack::get_absolute(int p) {
		if (p < 0)
			p = top + p;
		else
			p = frame.top() + p;

		return p;
	}
	
	value &stack::get(int p) {
		if (p < 0)
			p = top + p;
	
		if (p >= (int)top || p < 0)
			throw out_of_bounds_error(p, top);
			
		return store[p];
	}
	
	value &stack::get_in_frame(int p) {
		p = get_absolute(p);

		if (p >= (int)top || p < (int)frame.top())
			throw out_of_bounds_error(p, top);

		return store[p];
	}
	
	void stack::set(int p, const value &v) {
		if (p < 0)
			p = top + p;
			
		if (p >= EXO_STACK_SIZE || p < 0)
			throw out_of_bounds_error(p, top);
			
		if (p >= (int)top)
			top = p+1;
			
		store[p] = v;
	}
	
	void stack::set_in_frame(int p, const value &v) {
		p = get_absolute(p);
	
		if (p >= EXO_STACK_SIZE || p < (int)frame.top())
			throw out_of_bounds_error(p, top);
			
		if (p >= (int)top)
			top = p+1;
			
		store[p] = v;
	}
	
	void stack::push_frame(unsigned args) {
		unsigned bottom = frame.top();
		int back = (int)top-(int)args < (int)bottom ? top-bottom : args;
		unsigned forward = args-back;
		
		frame.push(top - back);
		top += forward;
	}
	
	void stack::pop_frame(unsigned arets, unsigned erets) {
		unsigned bottom = frame.top();
		unsigned ret_start = top - arets;
		unsigned ret_end = ret_start + erets;
		
		unsigned m = ret_start - bottom;
		for (unsigned i=ret_start; i <= ret_end; ++i) {
			store[i - m] = store[i];
		}
		
		top = bottom + erets;
		frame.pop();
		
		if (frame.empty())
			frame.push(0u);
	}
	
	void stack::print_stack() const {
		for (unsigned i=0u; i<top; ++i) {
			if (i == frame.top())
				std::cout << "------- bottom: " << frame.top() << std::endl;
				
			const value &v = store[i];
			std::cout << type_name(v.get_type()) << ": " << v.to_string() << std::endl;
		}
		
		if (frame.top() == top)
			std::cout << "------- bottom: " << top << std::endl;
			
		std::cout << "------- top: " << top << std::endl;
	}
	
}
