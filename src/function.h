#pragma once

#include <vector>
#include "opcode.h"
#include "state.h"

namespace exo {
	
	class function {
	private:
		std::vector<instruction> 	i_store;
		std::vector<value>			k_store;
		
		instruction *pc;
		
	public:
		function();
		function(const std::vector<instruction> &);
		
		~function();
		
		function(const function &)=delete;
		function(function &&)=delete;
		const function &operator=(const function &)=delete;
		
		int call(state *);
	};
}
