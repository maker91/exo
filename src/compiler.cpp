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
	
	void compiler::consume(tokens::token t, const std::string &s) {
		if (p == end-1)
			throw std::runtime_error(std::to_string(p->line) + ": expected " + s + " near eof");
	
		if (p->tk != t)
			throw std::runtime_error(std::to_string(p->line) + ": expected " + s + " near " + p->str);
			
		++p;
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
			
		case tokens::NIL:
			I.push_back(MAKE_ABC(opcodes::LOADNIL, r, 0, 0, 0, 0));
			++p;
			break;
		
		default:
			throw std::runtime_error(std::to_string(p->line) + ": unexpected symbol '" + p->str + "'");
		}
		
		return r;
	}

	void compiler::do_global() {
		consume(tokens::GLOBAL, "global");
		consume(tokens::RESOLUTION, "::");
		consume(tokens::IDENTIFIER, "name");
		
		value key = (p-1)->k;
		K.push_back(key);
		int k = K.size()-1;
		
		switch (p->tk) {
		case tokens::ASSIGNMENT:
			{
				++p;
				
				int r = do_expression(next_register++);
				I.push_back(MAKE_ABC(opcodes::SETGLOBAL, 0, 1, k, 0, r));
				break;
			}
			
		case tokens::LPAREN:
			{
				int r = next_register++;
				I.push_back(MAKE_ABC(opcodes::GETGLOBAL, r, 1, k, 0, 0));
				do_function(r, 0);
				break;
			}
			
		default:
			throw std::runtime_error(std::to_string((p-1)->line) + ": global variables must be initialised");
		}
	}
	
	void compiler::do_local() {
		if (p == end-1)
			throw std::runtime_error(std::to_string(p->line) + ": expected symbol near '" + p->str + "'");
	
		std::string name = p->str;
		++p;
		
		switch (p->tk) {
		case tokens::ASSIGNMENT:
			{
				++p;
				int l = next_register++;
				L[name] = l;
				
				int r = do_expression(l);
				if (r!=l)
					I.push_back(MAKE_ABC(opcodes::MOVE, r, 0, l, 0, 0));
					
				break;
			}
			
		case tokens::LPAREN:
			{
				std::string name = p->str;
				if (L.count(name) != 1)
					throw std::runtime_error(std::to_string(p->line) + ": " + name + " is not a variable!");
					
				++p;
					
				int r = L[name];
				do_function(r, 0);
				break;
			}
		
		default:
			throw std::runtime_error(std::to_string((p-1)->line) + "unexpected symbol near '" + name + "'");
		}
	}
	
	void compiler::do_function(int f, int ret) {
		consume(tokens::LPAREN, "(");
		
		int args = 0;
		while (true) {
			if (p == end)
				throw std::runtime_error(std::to_string((p-1)->line) + ": expected ')' near eof");
				
			if (p->tk == tokens::RPAREN)
				break;
				
			if (args > 0)
				consume(tokens::SEPARATOR, ",");
				
			int l = next_register++;
			int r = do_expression(l);
			if (r!=l)
				I.push_back(MAKE_ABC(opcodes::MOVE, r, 0, l, 0, 0));
				
			++args;
		}
		
		++p;
		I.push_back(MAKE_ABC(opcodes::CALL, (ret+1), 0, f, 0, (args+1)));
	}
	
	void compiler::do_statement() {
		switch (p->tk) {
		// global variable definition
		case tokens::GLOBAL:
			do_global();
			break;
		
		// assignment, function call
		case tokens::IDENTIFIER:
			do_local();
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