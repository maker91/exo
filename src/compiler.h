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
		const symbol *p;
		const symbol *end;
		
		int next_register;
		
		namespace_info *N; //current namespace
		std::vector<instruction> I;
		std::vector<value> 			K;
	
	public:
		compiler(const token_result &);
		function compile();
		
	private:
		void consume(tokens::token, const std::string &);
	
		void do_block();
		void do_statement();
		void do_local();
		void do_function(int, int);
		
		void do_expression(int r, bool push = false);
		
		std::string do_identifier();
		
		int get_local(const std::string &, namespace_info *n);
		int get_local(const std::string &);

		namespace_info* lookup_namespace();
	};
}
