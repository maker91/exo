#pragma once

#include <vector>
#include <map>
#include <stack>

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
		std::map<std::string, int>	L;
		std::stack<std::string>		N;
	
	public:
		compiler(const std::vector<symbol> &);
		function compile();
		
	private:
		void consume(tokens::token, const std::string &);
	
		void do_block();
		void do_statement();
		int do_expression(int);
		void do_global();
		void do_local(const std::string &);
		void do_function(int, int);
		std::string do_identifier();
	};
}