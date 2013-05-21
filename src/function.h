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
		
	public:
		function();
		function(const std::vector<instruction> &);
		function(const std::vector<instruction> &, const std::vector<value> &);
		
		~function();
		
		function(const function &)=delete;
		function(function &&)=delete;
		const function &operator=(const function &)=delete;
		
		int call(state *);
	};
}
