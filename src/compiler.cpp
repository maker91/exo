#include <iostream>
#include <stdexcept>

#include "compiler.h"
#include "type.h"
#include "value.h"
#include "opcode.h"

namespace exo {
	
	compiler::compiler(std::vector<value> &constants,
		std::vector<symbol>::iterator &start, std::vector<symbol>::iterator end) 
		: N(new namespace_info("root")), K(constants), p(start), end(end)
	{

	}

	compiler::compiler(std::vector<value> &constants, std::vector<symbol>::iterator &start, 
		std::vector<symbol>::iterator end, std::vector<std::string> params)
		: N(new namespace_info("root")), K(constants), p(start), end(end)
	{
		int i = 0;
		for (std::string param : params)
			N->L[param] = i++;
		next_register = i;
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

	void compiler::do_expression(int r) {
		opcodes::opcode op = opcodes::MOVE;

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
				
				if (l == -1) { // no local variable, add a GETGLOBAL instruction
					I.push_back(MAKE_ABC(opcodes::GETGLOBAL, r, 1, (p-1)->k, 0, 0));
				} else if (r != l) {
					I.push_back(MAKE_ABC(op, r, 0, l, 0, 0));
				}
			}
			break;

		case tokens::FUNCTION:
			do_function(r);
			break;
			
		case tokens::LPAREN:
			consume(tokens::LPAREN, "(");
			do_expression(r);	
			consume(tokens::RPAREN, ")");
			break;

		case tokens::NOT:
			++p;
			do_expression(r);
			I.push_back(MAKE_ABC(opcodes::NOT, r, 0, r, 0, 0));
			break;
		
		default:
			throw std::runtime_error(std::to_string(p->line) + ": unexpected symbol '" + p->str + "'");
		}
		
		do_sub_expression(r, r+1);
	}

	void compiler::do_sub_expression(int r, int i) {
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

		case tokens::MOD:
			++p;
			do_expression(i);			
			I.push_back(MAKE_ABC(opcodes::MOD, r, 0, r, 0, i));
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

		case tokens::AND:
			++p;
			do_expression(i);
			I.push_back(MAKE_ABC(opcodes::AND, r, 0, i, 0, r));
			break;

		case tokens::OR:
			++p;
			do_expression(i);
			I.push_back(MAKE_ABC(opcodes::OR, r, 0, i, 0, r));
			break;

		case tokens::EQUAL:
			++p;
			do_expression(i);
			I.push_back(MAKE_ABC(opcodes::EQL, r, 0, r, 0, i));
			break;

		case tokens::CONCAT:
			++p;
			do_expression(i);
			I.push_back(MAKE_ABC(opcodes::CONCAT, r, 0, r, 0, i));
			break;

		case tokens::LPAREN:  // function call
			do_function_call(r, 0);
			break;
			
		default:
			return;
		}

		do_sub_expression(r, i+1);
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
	
	void compiler::do_variable_assignment() {
		namespace_info *root = lookup_namespace();	
		std::string name = do_identifier();
		consume(tokens::ASSIGNMENT, "=");
		
		if (!root->L.count(name)) {
			int l = next_register++;
			do_expression(l);
			root->L[name] = l;
		} else {
			int l = root->L[name];
			do_expression(l);
		}
	}

	int compiler::do_param_list(tokens::token end_tk, const std::string &end_str) {
		int args = 0;
		while (true) {
			if (p == end)
				throw std::runtime_error(std::to_string((p-1)->line) + ": expected '" + end_str + "' near eof");
				
			if (p->tk == end_tk)
				break;
				
			if (args > 0)
				consume(tokens::SEPARATOR, ",");
			
			int l = next_register;
			do_expression(l);
			I.push_back(MAKE_ABC(opcodes::PUSH, 0, 0, l, 0, 0));
			next_register = l;
			++args;
		}

		return args;
	}
	
	void compiler::do_function_call(int f, int ret) {
		consume(tokens::LPAREN, "(");
		int args = do_param_list(tokens::RPAREN, ")");
		consume(tokens::RPAREN, ")");
		I.push_back(MAKE_ABC(opcodes::CALL, (ret+1), 0, f, 0, (args+1)));
	}

	void compiler::do_while() {
		++p;
		
		unsigned start_exp = I.size();

		int r = next_register;
		do_expression(r);
		next_register = r;
		
		unsigned start_loop = I.size();
		I.push_back(MAKE_ABC(opcodes::NOOP, 0, 0, 0, 0, 0)); // gets replaced by TEST instruction after while loop has been parsed
		
		do_statement();
		
		I.push_back(MAKE_ABx(opcodes::JMP, 0, 1, (I.size() - start_exp)));
		I[start_loop] = MAKE_AtBx(opcodes::TEST, r, 0, 0, (I.size() - start_loop));
	}

	void compiler::do_if() {
		++p;
		int r = next_register;
		do_expression(next_register);
		next_register = r;
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
	}

	void compiler::do_for() {
		consume(tokens::FOR, "for");

		bool parens = (p->tk == tokens::LPAREN);
		if (parens)
			consume(tokens::LPAREN, "(");

		namespace_info *root = lookup_namespace();	
		std::string name = do_identifier();

		consume(tokens::ASSIGNMENT, "=");

		int l;
		if (!root->L.count(name)) {
			l = next_register++;
			root->L[name] = l;
		} else {
			l = root->L[name];
		}

		do_expression(l);  // the initial value

		consume(tokens::SEPARATOR, ",");
		int r = next_register++;
		do_expression(r); // the limit

		if (parens)
			consume(tokens::RPAREN, ")");

		unsigned start_for = I.size();
		int p = next_register++;
		I.push_back(MAKE_ABC(opcodes::LE, p, 0, l, 0, r));

		unsigned start_loop = I.size();
		I.push_back(MAKE_ABC(opcodes::NOOP, 0, 0, 0, 0, 0)); // gets replaced by TEST instruction after for loop has been parsed
		do_statement(); // the block

		I.push_back(MAKE_ABC(opcodes::INCR, l, 0, 0, 0, 0)); // increment the counter
		I.push_back(MAKE_ABx(opcodes::JMP, 0, 1, (I.size() - start_for))); // jump back to the beginning of the loop
		I[start_loop] = MAKE_AtBx(opcodes::TEST, p, 0, 0, (I.size() - start_loop));

		next_register = r;
	}

	void compiler::do_return() {
		consume(tokens::RETURN, "return");
		if (p->tk == tokens::RBRACE || p == end) {
			I.push_back(MAKE_ABC(opcodes::RTN, 1, 0, 0, 0, 0));
		} else {
			int args = do_param_list(tokens::RBRACE, "}");
			I.push_back(MAKE_ABC(opcodes::RTN, 1, args+1, 0, 0, 0));
		}
	}
	
	void compiler::do_statement() {
		switch (p->tk) {
		// local variable assignment
		case tokens::IDENTIFIER:
			if ((p+1)->tk == tokens::ASSIGNMENT) {
				do_variable_assignment();
			} else {
				int r = next_register++;
				do_expression(r);
				next_register = r;
			}
			break;

		case tokens::RETURN:
			do_return();
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
			do_while();			
			break;

		case tokens::FOR:
			do_for();
			break;
			
		case tokens::IF:
			do_if();
			break;

		case tokens::FUNCTION:
			do_function();
			break;

		default:
			{
				int r = next_register++;
				do_expression(r);
				next_register = r;
			}
		}
	}

	void compiler::do_function() {
		consume(tokens::FUNCTION, "function");
		namespace_info *root = lookup_namespace();	
		std::string name = do_identifier();

		int r;
		if (!root->L.count(name)) {
			r = next_register++;
			root->L[name] = r;
		} else {
			r = root->L[name];
		}

		consume(tokens::LPAREN, "(");
		std::vector<std::string> params;
		while (true) {
			if (p == end)
				throw std::runtime_error(std::to_string((p-1)->line) + ": expected ')' near eof");
				
			if (p->tk == tokens::RPAREN)
				break;
				
			if (!params.empty())
				consume(tokens::SEPARATOR, ",");
			
			params.push_back(do_identifier());
		}
		consume(tokens::RPAREN, ")");
		consume(tokens::LBRACE, "{");

		auto func_c = exo::compiler(K, p, end, params);
		K.emplace_back(func_c.compile());
		I.push_back(MAKE_ABC(opcodes::MOVE, r, 1, K.size() - 1, 0, 0));

		consume(tokens::RBRACE, "}");
	}

	void compiler::do_function(int r) {
		consume(tokens::FUNCTION, "function");
		consume(tokens::LPAREN, "(");
		std::vector<std::string> params;
		while (true) {
			if (p == end)
				throw std::runtime_error(std::to_string((p-1)->line) + ": expected ')' near eof");
				
			if (p->tk == tokens::RPAREN)
				break;
				
			if (!params.empty())
				consume(tokens::SEPARATOR, ",");
			
			params.push_back(do_identifier());
		}
		consume(tokens::RPAREN, ")");
		consume(tokens::LBRACE, "{");

		auto func_c = exo::compiler(K, p, end, params);
		K.emplace_back(func_c.compile());
		I.push_back(MAKE_ABC(opcodes::MOVE, r, 1, K.size() - 1, 0, 0));

		consume(tokens::RBRACE, "}");
	}

	function *compiler::compile() {
		do_statement();
		
		if (N->parent != nullptr)
			throw std::runtime_error("expected '}' to close namespace near eof");
			
		I.push_back(MAKE_ABC(opcodes::RTN, 1, 0, 0, 0, 0));
		
		std::cout << "I: " << std::endl;
		for (instruction i : I) {
			opcodes::opcode op = GET_OP(i);
			
			std::cout << opcode_name(op) << "\t" << GET_A(i) << " " << IS_BK(i) << " " << GET_B(i) << " ";
			std::cout << IS_CK(i) << " " << GET_C(i);
			std::cout << "\t(" << GET_A(i) << " " << GET_T(i) << " " << (int)GET_Bx(i) << ")" << std::endl;
		}
		
		return new function(I, K);
	}
}
