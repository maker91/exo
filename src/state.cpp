#include "state.h"

namespace exo {
	void state::push(const value &v) {
		stack.push(v);
	}
	
	void state::pop() {
		stack.pop();
	}	
	
	value state::get(int i) {
		return stack.get_in_frame(i);
	}
}
