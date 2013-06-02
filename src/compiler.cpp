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
	
	int compiler::get_local(const std::string &name) {
		if (L.count(name) != 1)
			throw std::runtime_error(std::to_string(p->line) + ": " + name + " is not a variable!");
					
		return L[name];
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
			
		case tokens::IDENTIFIER:
			r = get_local(do_name(true));
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
			throw std::runtime_error(std::to_string((p-1)->line) + ": unexpected symbol '" + p->str + "' near '" + (p-1)->str + "'");
		}
	}
	
	std::string compiler::do_identifier() {
		consume(tokens::IDENTIFIER, "name");
		return (p-1)->str;
	}
	
	std::string compiler::do_name(bool resolve) {
		std::string r = "";
		
		if (resolve) {
			for (std::string &n : N)
				r += (n + "::");
		}
		
		r += do_identifier();
		while (p != end && p->tk == tokens::RESOLUTION) {
			consume(tokens::RESOLUTION, "::");
			r += ("::" + do_identifier());
		}
		
		std::cout << "name: " << r << std::endl;
		
		return r;
	}
	
	void compiler::do_local() {
		std::string name = do_name(true);
	
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
			do_function(get_local(name), 0);
			break;
		
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
		// global variable access
		case tokens::GLOBAL:
			do_global();
			break;
		
		// local variable access
		case tokens::IDENTIFIER:
			do_local();
			break;
			
		case tokens::NAMESPACE:
			++p;
			N.push_back(do_identifier());
			consume(tokens::LBRACE, "{");
			break;
			
		case tokens::RBRACE:
			++p;
			
			if (N.empty())
				throw std::runtime_error(std::to_string(p->line) + ": unexpected '}' near '" + (p-1)->str + "'");
				
			N.pop_back();
			break;
			
		default:
			throw std::runtime_error(std::to_string(p->line) + ": unexpected symbol '" + p->str + "'");
		}
	}

	function compiler::compile() {
		while (p != end) {
			do_statement();
		}
		
		if (!N.empty())
			throw std::runtime_error("expected '}' to close namespace '" + N.back() + " near eof");
			
		I.push_back(MAKE_ABC(opcodes::RTN, 1, 0, 0, 0, 0));
		
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