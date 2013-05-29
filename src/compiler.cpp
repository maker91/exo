#include "compiler.h"
#include "type.h"
#include "value.h"
#include "opcode.h"

namespace exo {

	function compile(const std::vector<symbol> &src) {
		std::vector<instruction> I;
		std::vector<value> K;
		
		const symbol *p = &src[0];
		const symbol *end = &src[src.size()-1]+1;
		while (p != end) {
			// compile
			++p;
		}
		
		return function(I, K);
	}
}