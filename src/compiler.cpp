#include <iostream>
#include <stdexcept>

#include "compiler.h"
#include "type.h"
#include "value.h"
#include "opcode.h"

namespace exo {
	
	compiler::compiler(const token_result &src) 
		: p(&src.symbols[0]), end(&src.symbols[src.symbols.size()-1]+1), next_register(0),
		K(src.constants)
	{
		
	}
	
	void compiler::consume(tokens::token t, const std::string &s) {
		if (p == end)
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
	
	void compiler::get_global(int k, int r) {
		I.push_back(MAKE_ABC(opcodes::GETGLOBAL, r, 1, k, 0, 0));
	}
	
	void compiler::do_expression(int r) {
		switch (p->tk) {
		case tokens::CONSTANT:
			I.push_back(MAKE_ABC(opcodes::LOADK, r, 1, p->k, 0, 0));
			++p;
			break;
			
		case tokens::BOOLEAN:
			I.push_back(MAKE_ABC(opcodes::LOADBOOL, r, 0, p->str == "true" ? 1 : 0, 0, 0));
			++p;
			break;
			
		case tokens::NIL:
			I.push_back(MAKE_ABC(opcodes::LOADNIL, r, 0, 0, 0, 0));
			++p;
			break;
			
		case tokens::IDENTIFIER:
			{
				int l = get_local(do_name(true));
				
				if (r != l)
					I.push_back(MAKE_ABC(opcodes::MOVE, r, 0, l, 0, 0));
				break;
			}
			
		case tokens::GLOBAL:
			consume(tokens::GLOBAL, "global");
			consume(tokens::RESOLUTION, "::");
			consume(tokens::IDENTIFIER, "name");
			
			get_global((p-1)->k, r);
			break;
			
		case tokens::LPAREN:
			{
				++p;
				
				do_expression(r);	
				consume(tokens::RPAREN, ")");
				break;
			}
		
		default:
			throw std::runtime_error(std::to_string(p->line) + ": unexpected symbol '" + p->str + "'");
		}
		
		int i;
		switch (p->tk) {
		case tokens::ADD:
			++p;
			
			i = next_register++;
			do_expression(i);
			
			I.push_back(MAKE_ABC(opcodes::ADD, r, 0, r, 0, i));
			break;
			
		case tokens::SUB:
			++p;
			
			i = next_register++;
			do_expression(i);
			
			I.push_back(MAKE_ABC(opcodes::SUB, r, 0, r, 0, i));
			break;
			
		case tokens::MUL:
			++p;
			
			i = next_register++;
			do_expression(i);
			
			I.push_back(MAKE_ABC(opcodes::MUL, r, 0, r, 0, i));
			break;
			
		case tokens::DIV:
			++p;
			
			i = next_register++;
			do_expression(i);
			
			I.push_back(MAKE_ABC(opcodes::DIV, r, 0, r, 0, i));
			break;
			
		case tokens::LT:
			++p;
			
			i = next_register++;
			do_expression(i);
			
			I.push_back(MAKE_ABC(opcodes::LT, r, 0, r, 0, i));
			break;
			
		case tokens::LE:
			++p;
		
			i = next_register++;
			do_expression(i);
			
			I.push_back(MAKE_ABC(opcodes::LE, r, 0, r, 0, i));
			break;
			
		default:
			break;
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
		
		return r;
	}
	
	void compiler::do_global() {
		consume(tokens::GLOBAL, "global");
		consume(tokens::RESOLUTION, "::");
		consume(tokens::IDENTIFIER, "name");
		
		int k = (p-1)->k;
		
		switch (p->tk) {
		case tokens::ASSIGNMENT:
			{
				++p;
				
				int r = next_register++;
				do_expression(r);				
				I.push_back(MAKE_ABC(opcodes::SETGLOBAL, 0, 1, k, 0, r));
				break;
			}
			
		case tokens::LPAREN:
			{
				int r = next_register++;
				get_global(k, r);
				do_function(r, 0);
				break;
			}
			
		default:
			throw std::runtime_error(std::to_string((p-1)->line) + ": unexpected symbol '" + p->str + "' near '" + (p-1)->str + "'");
		}
	}
	
	void compiler::do_local() {
		std::string name = do_name(true);
	
		switch (p->tk) {
		case tokens::ASSIGNMENT:
			{
				++p;
				
				int l;
				if (!L.count(name)) {
					l = next_register++;
					L[name] = l;
				} else {
					l = L[name];
				}
				
				do_expression(l);
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
				
			int r = next_register++;
			do_expression(r);				
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
			
		case tokens::DECL:
			++p;
			L[do_name(true)] = next_register++;
			break;
			
		case tokens::WHILE:
			{
				++p;
				consume(tokens::LPAREN, "(");
				
				unsigned start_exp = I.size();
				int r = next_register++;
				do_expression(r);
				
				consume(tokens::RPAREN, ")");
				consume(tokens::LBRACE, "{");
				
				unsigned start_loop = I.size();
				I.push_back(MAKE_ABC(opcodes::NOOP, 0, 0, 0, 0, 0)); // gets replaced by TEST instruction after while loop has been parsed
				
				while (p != end && p->tk != tokens::RBRACE)
					do_statement();
				
				I.push_back(MAKE_ABC());
				I.push_back(MAKE_ABx(opcodes::JMP, 0, 1, (I.size() - start_exp)));
				I[start_loop] = MAKE_AtBx(opcodes::TEST, r, 0, 0, (I.size() - start_loop));
				
				consume(tokens::RBRACE, "}");
				break;
			}
			
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
		
		std::cout << "I: " << std::endl;
		for (instruction i : I) {
			opcodes::opcode op = GET_OP(i);
			
			std::cout << opcode_name(op) << "\t" << GET_A(i) << " " << IS_BK(i) << " " << GET_B(i) << " " << IS_CK(i) << " " << GET_C(i);
			std::cout << "\t(" << GET_A(i) << " " << GET_T(i) << " " << (int)GET_Bx(i) << ")" << std::endl;
		}
		
		return function(I, K);
	}
}
