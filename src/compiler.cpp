#include "compiler.h"
#include "type.h"
#include "value.h"
#include "opcode.h"

#include <iostream>

namespace exo {
	
	compiler::compiler(const std::vector<symbol> &src) 
		: p(&src[0]), end(&src[src.size()-1]+1), next_register(0), next_constant(0)
	{
		
	}
	
	int compiler::do_expression(int r) {
		int k = 0;
	
		switch (p->tk) {
		case tokens::CONSTANT:
			K.push_back(p->k);
			k = K.size()-1;

			I.push_back(MAKE_ABC(opcodes::LOADK, r, 1, k, 0, 0));
			
			++p;
			break;
			
		case tokens::BOOLEAN:
			I.push_back(MAKE_ABC(opcodes::LOADBOOL, r, 0, p->k.to_boolean() ? 1 : 0, 0, 0));
			
			++p;
			break;
		
		default:
			throw std::runtime_error(std::to_string(p->line) + ": unexpected symbol '" + p->str + "'");
		}
		
		return r;
	}

	void compiler::do_global() {
		if (p == end-1)
			throw std::runtime_error(std::to_string(p->line) + ": expected symbol near 'global'");
			
		++p;
		
		if (p->tk == tokens::FUNCTION) {
			// do parameters
			// do block
			throw std::runtime_error("global function definition not implemented");
		} else if (p->tk == tokens::IDENTIFIER) {
			value key = p->k;
			K.push_back(key);
			int k = K.size()-1;
			++p;
			
			if (p->tk == tokens::ASSIGNMENT) {
				++p;
				
				int r = do_expression(next_register++);
				I.push_back(MAKE_ABC(opcodes::SETGLOBAL, 0, 1, k, 0, r));
			} else {
				throw std::runtime_error(std::to_string((p-1)->line) + ": global variables must be initialised");
			}
		} else {
			throw std::runtime_error(std::to_string(p->line) + ": unexpected symbol '" + p->str + "' near 'global'");
		}
	}
	
	void compiler::do_local() {
		std::string name = p->str;
		++p;
		
		if (p->tk == tokens::ASSIGNMENT) {
			++p;
			int l = next_register++;
			L[name] = l;
			
			int r = do_expression(l);
			if (r!=l)
				I.push_back(MAKE_ABC(opcodes::MOVE, r, 0, l, 0, 0));
		} else {
			throw std::runtime_error(std::to_string((p-1)->line) + ": use the decl keywod to declare uninitialised local variables");
		}
	}
	
	void compiler::do_statement() {
		switch (p->tk) {
		// global variable definition
		case tokens::GLOBAL:
			do_global();
			break;
		
		// assignment, function call
		case tokens::IDENTIFIER:
			if (p == end-1)
				throw std::runtime_error(std::to_string(p->line) + ": expected symbol near '" + p->str + "'");
		
			switch ((p+1)->tk) {
			case tokens::ASSIGNMENT:
				do_local();
				break;
				
			default:
				throw std::runtime_error(std::to_string(p->line) + ": unexpected symbol '" + p->str + "'");
			}
			break;
			
		default:
			throw std::runtime_error(std::to_string(p->line) + ": unexpected symbol '" + p->str + "'");
		}
	}

	function compiler::compile() {
		while (p != end) {
			do_statement();
		}
		
		std::cout << "K: " << std::endl;
		for (value &v : K) {
			std::cout << type_name(v.get_type()) << ": " << v.to_string() << std::endl;
		}
		
		std::cout << std::endl;
		
		std::cout << "I: " << std::endl;
		for (instruction i : I) {
			std::cout << GET_OP(i) << " " << GET_A(i) << " " << IS_BK(i) << " " << GET_B(i) << " " << IS_CK(i) << " " << GET_C(i) << std::endl;
		}
		
		return function(I, K);
	}
}