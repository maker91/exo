#include "state.h"

namespace exo {
	void state::push(const value &v) {
		stack.push(v);
	}
	
	void state::pop() {
		stack.pop();
	}	
	
	void state::set(int i, const value &v) {
		stack.set_in_frame(i, v);
	}
	
	value state::get(int i) {
		return stack.get_in_frame(i);
	}
	
	void state::set_global(const value &k, const value &v) {
		if (v.get_type() == NIL) {
			globals.erase(k);
			return;
		}
		
		globals[k] = v;
	}
	
	value state::get_global(const value &k) {
		return globals[k];
	}
}
