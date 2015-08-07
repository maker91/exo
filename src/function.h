#pragma once

#include <vector>
#include "opcode.h"
#include "state.h"
#include "value.h"

namespace exo {
	
	class function {
	private:
		std::vector<instruction> 	i_store;
		std::vector<value>			k_store;
		int 						param_start;
		int 						num_params;
		
	public:
		function(int pstart, int pnum);
		function(int pstart, int pnum, const std::vector<instruction> &);
		function(int pstart, int pnum, const std::vector<instruction> &, const std::vector<value> &);
		
		~function();
		
		int call(state *E, int args);
	};
}
