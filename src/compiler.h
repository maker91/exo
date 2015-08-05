#pragma once

#include <vector>
#include <map>

#include "token.h"
#include "function.h"

namespace exo {
	struct lexical_scope {
		std::map<std::string, int> L;
		std::vector<lexical_scope> children;
		lexical_scope *parent;

		lexical_scope() : parent(nullptr) {}
		lexical_scope(lexical_scope *p) : parent(p) {}

		lexical_scope *new_scope() {
			children.emplace_back(this);
			return &children.back();
		}
	};

	class compiler {
	private:
		std::vector<symbol>::iterator p;
		std::vector<symbol>::iterator end;
		int next_register;
		
		exo::map builtins;
		std::vector<value> &K;

		lexical_scope root;
		lexical_scope *current_scope;
	
	public:
		compiler(std::vector<symbol>::iterator start, std::vector<symbol>::iterator end, 
			std::vector<value> &constants, exo::map builtins);
		function *compile(std::vector<std::string> params = {});
		
	private:
		void consume(tokens::token, const std::string &);
	
		void do_block(std::vector<instruction> &I);
		void do_statement(std::vector<instruction> &I);
		int do_variable_assignment(std::vector<instruction> &I);
		void do_function_call(std::vector<instruction> &I, int, int);
		void do_while(std::vector<instruction> &I);
		void do_if(std::vector<instruction> &I);
		void do_for(std::vector<instruction> &I);
		int do_expression(std::vector<instruction> &I, int o=-1, bool force_out=false, int prec = 99);
		int do_sub_expression(std::vector<instruction> &I, int o, int r, int prec);
		void do_function(std::vector<instruction> &I);
		void do_anonymous_function(std::vector<instruction> &I);
		void do_return(std::vector<instruction> &I);
		int do_expression_list(std::vector<instruction> &I);
		int do_pair_list(std::vector<instruction> &I);
		
		std::string get_identifier();
		std::vector<std::string> get_identifier_list();
		int get_local(const std::string &name, bool recursive = true);
	};
}
