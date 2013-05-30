#pragma once

#include "token.h"
#include "function.h"

namespace exo {

	class compiler {
	private:
		const symbol *p;
		const symbol *end;
		
		int next_register;
		int next_constant;
		
		std::vector<instruction> 	I;
		std::vector<value> 			K;
	
	public:
		compiler(const std::vector<symbol> &);
		function compile();
		
	private:
		void do_block();
		void do_statement();
		int do_expression();
		void do_global();
	};
}