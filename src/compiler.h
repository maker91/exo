#pragma once

#include <vector>
#include <map>

#include "token.h"
#include "function.h"

namespace exo {

	class compiler {
	private:
		const symbol *p;
		const symbol *end;
		
		int next_register;
		
		std::vector<instruction> 	I;
		std::vector<value> 			K;
		std::vector<std::string>	N;
		std::map<std::string, int>	L;
	
	public:
		compiler(const token_result &);
		function compile();
		
	private:
		void consume(tokens::token, const std::string &);
	
		void do_block();
		void do_statement();
		void do_global();
		void do_local();
		void do_function(int, int);
		
		void do_expression(int);
		
		std::string do_name(bool);
		std::string do_identifier();
		
		int get_local(const std::string &);
		void get_global(int, int);
	};
}