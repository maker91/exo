#include <iostream>
#include <stdexcept>

#include "compiler.h"
#include "type.h"
#include "value.h"
#include "opcode.h"

namespace exo {
	
	compiler::compiler(const token_result &src) 
		: p(&src.symbols[0]), end(&src.symbols[src.symbols.size()-1]+1), next_register(0),
		K(src.constants), N(new namespace_info("root"))
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
		return get_local(name, N);
	}
	
	int compiler::get_local(const std::string &name, namespace_info *n) {
		while (n != nullptr) {
			if (n->L.count(name) == 1)
				return n->L[name];
			n = n->parent;
		}

		return -1;
	}
	
	void compiler::do_block() {
		consume(tokens::LBRACE, "{");
        while (p != end && p->tk != tokens::RBRACE) {
                do_statement();
        }
        consume(tokens::RBRACE, "}");
	}

	void compiler::do_expression(int r, bool push) {
		opcodes::opcode op = (push ? opcodes::PUSH : opcodes::MOVE);

		switch (p->tk) {
		case tokens::CONSTANT:
			I.push_back(MAKE_ABC(op, r, 1, p->k, 0, 0));
			++p;
			break;
			
		case tokens::BOOLEAN:
			I.push_back(MAKE_ABC(op, r, 1, p->str == "true" ? 1 : 2, 0, 0));
			++p;
			break;
			
		case tokens::NIL:
			I.push_back(MAKE_ABC(op, r, 1, 0, 0, 0));
			++p;
			break;
			
		case tokens::IDENTIFIER:
			{
				namespace_info *root = lookup_namespace();
				std::string name = do_identifier();
				int l = get_local(name, root);
				std::cout << name << ": " << l << std::endl;
				
				if (l == -1) { // no local variable, add a GETGLOBAL instruction
					I.push_back(MAKE_ABC(opcodes::GETGLOBAL, r, 1, (p-1)->k, 0, 0));
				} else if (r != l) {
					I.push_back(MAKE_ABC(op, r, 0, l, 0, 0));
				}
				break;
			}
			
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
		
		int i = r + 1;
		switch (p->tk) {
		case tokens::ADD:
			++p;
			do_expression(i);
			I.push_back(MAKE_ABC(opcodes::ADD, r, 0, r, 0, i));
			break;
			
		case tokens::SUB:
			++p;
			do_expression(i);
			I.push_back(MAKE_ABC(opcodes::SUB, r, 0, r, 0, i));
			break;
			
		case tokens::MUL:
			++p;
			do_expression(i);
			I.push_back(MAKE_ABC(opcodes::MUL, r, 0, r, 0, i));
			break;
			
		case tokens::DIV:
			++p;
			do_expression(i);			
			I.push_back(MAKE_ABC(opcodes::DIV, r, 0, r, 0, i));
			break;
			
		case tokens::LT:
			++p;
			do_expression(i);
			I.push_back(MAKE_ABC(opcodes::LT, r, 0, r, 0, i));
			break;
			
		case tokens::LE:
			++p;
			do_expression(i);
			I.push_back(MAKE_ABC(opcodes::LE, r, 0, r, 0, i));
			break;

		case tokens::GT:
			++p;
			do_expression(i);
			I.push_back(MAKE_ABC(opcodes::LT, r, 0, i, 0, r));
			break;
			
		case tokens::GE:
			++p;
			do_expression(i);
			I.push_back(MAKE_ABC(opcodes::LE, r, 0, i, 0, r));
			break;

		case tokens::EQUAL:
			++p;
			do_expression(i);
			I.push_back(MAKE_ABC(opcodes::EQL, r, 0, r, 0, i));
			break;
			
		default:
			break;
		}
	}
	
	std::string compiler::do_identifier() {
		consume(tokens::IDENTIFIER, "name");
		return (p-1)->str;
	}

	namespace_info *compiler::lookup_namespace() {
		std::string str = do_identifier();
		namespace_info *n = N;

		while (p != end && p->tk == tokens::RESOLUTION) {
			if (n->children.count(str) != 1)
				throw std::runtime_error(std::to_string(p->line) + ": '" + str + "' is not a namespace!");
			n = n->children[str];
			
			consume(tokens::RESOLUTION, "::");
			str = do_identifier();
		}
		
		--p;
		return n;
	}
	
	void compiler::do_local() {
		namespace_info *root = lookup_namespace();	
		std::string name = do_identifier();
		int l;

		switch (p->tk) {
		case tokens::ASSIGNMENT:
			{
				consume(tokens::ASSIGNMENT, "=");
				
				if (!root->L.count(name)) {
					int l = next_register++;
					do_expression(l);
					root->L[name] = l;
				} else {
					int l = root->L[name];
					do_expression(l);
				}
				break;
			}
			
		case tokens::LPAREN:
			l = get_local(name, root);
			if (l == -1) {
				l = next_register++;
				I.push_back(MAKE_ABC(opcodes::GETGLOBAL, l, 1, (p-1)->k, 0, 0));

				do_function(l, 0);
				next_register = l;
			} else {
				do_function(l, 0);
			}
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
				
			do_expression(-1, true);
			++args;
		}
		
		++p;
		I.push_back(MAKE_ABC(opcodes::CALL, (ret+1), 0, f, 0, (args+1)));
	}
	
	void compiler::do_statement() {
		switch (p->tk) {
		// local variable access
		case tokens::IDENTIFIER:
			do_local();
			break;

		case tokens::RETURN:
			consume(tokens::RETURN, "return");
			I.push_back(MAKE_ABC(opcodes::RTN, 1, 0, 0, 0, 0));
			break;
			
		case tokens::NAMESPACE:
			consume(tokens::NAMESPACE, "namespace");
			{
				std::string name = do_identifier();
				namespace_info *n = new namespace_info(name);
				N->children[name] = n;
				n->parent = N;
				N = n;
			}
			do_block();
			N = N->parent;
			break;

		case tokens::LBRACE:
			do_block();
			break;
			
		case tokens::WHILE:
			{
				++p;
				consume(tokens::LPAREN, "(");
				
				unsigned start_exp = I.size();

				int r = next_register;
				do_expression(r);
				next_register = r;
				
				consume(tokens::RPAREN, ")");
				
				unsigned start_loop = I.size();
				I.push_back(MAKE_ABC(opcodes::NOOP, 0, 0, 0, 0, 0)); // gets replaced by TEST instruction after while loop has been parsed
				
				do_statement();
				
				I.push_back(MAKE_ABx(opcodes::JMP, 0, 1, (I.size() - start_exp)));
				I[start_loop] = MAKE_AtBx(opcodes::TEST, r, 0, 0, (I.size() - start_loop));
				
				break;
			}
			
		case tokens::IF:
			{
				++p;

				consume(tokens::LPAREN, "(");
				int r = next_register;
				do_expression(next_register);
				next_register = r;
				consume(tokens::RPAREN, ")");

				unsigned start_if = I.size();
				I.push_back(MAKE_ABC(opcodes::NOOP, 0, 0, 0, 0, 0)); // gets replaced by TEST instruction after if statement has been parsed

				do_statement();

				if (p->tk == tokens::ELSE) {
					unsigned start_else = I.size();
					I.push_back(MAKE_ABC(opcodes::NOOP, 0, 0, 0, 0, 0)); // replaced by a jump

					// replace the first NOOP with a TEST instruction
					I[start_if] = MAKE_AtBx(opcodes::TEST, r, 0, 0, (I.size() - start_if));

					consume(tokens::ELSE, "else");
					do_statement();

					// replace the second NOOP with a JMP instruction
					I[start_else] = MAKE_ABx(opcodes::JMP, 0, 0, I.size() - start_else);
				} else {
					// replace the NOOP with a TEST instruction
					I[start_if] = MAKE_AtBx(opcodes::TEST, r, 0, 0, (I.size() - start_if));
				}

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
		
		if (N->parent != nullptr)
			throw std::runtime_error("expected '}' to close namespace near eof");
			
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
