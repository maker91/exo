#pragma once

#include <vector>
#include <map>

#include "token.h"
#include "function.h"

namespace exo {

	struct namespace_info {
		std::string name;
		std::map<std::string, int> L;
		std::map<std::string, namespace_info *> children;
		namespace_info *parent;

		namespace_info(const std::string &name) : name(name), parent(nullptr) {};

		std::string get_full_name() {
			std::string n = "";
			if (parent != nullptr)
				n += (parent->get_full_name() + "::");
			return (n + name);
		}
	};


	class compiler {
	private:
		std::vector<symbol>::iterator &p;
		std::vector<symbol>::iterator end;
		
		int next_register;
		
		namespace_info *N; //current namespace
		std::vector<instruction> I;
		std::vector<value> &K;
	
	public:
		compiler(std::vector<value> &constants, std::vector<symbol>::iterator &start, std::vector<symbol>::iterator end);
		compiler(std::vector<value> &constants, std::vector<symbol>::iterator &start, 
			std::vector<symbol>::iterator end, std::vector<std::string> params);
		function *compile();
		
	private:
		void consume(tokens::token, const std::string &);
	
		void do_block();
		void do_statement();
		void do_variable_assignment();
		void do_function_call(int, int);
		void do_while();
		void do_if();
		void do_for();
		int do_expression(int prec = 99);
		void do_sub_expression(int r, int prec);
		std::vector<std::string> do_identifier_list(tokens::token, const std::string &);
		void do_function();
		void do_function(int r);
		void do_return();
		int do_param_list(tokens::token, const std::string &);
		
		std::string do_identifier();
		
		int get_local(const std::string &, namespace_info *n);
		int get_local(const std::string &);

		namespace_info* lookup_namespace();
	};
}
