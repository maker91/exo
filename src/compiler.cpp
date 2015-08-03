#include <iostream>
#include <stdexcept>

#include "compiler.h"
#include "type.h"
#include "value.h"
#include "opcode.h"

namespace exo {
	
	compiler::compiler(std::vector<symbol>::iterator start, std::vector<symbol>::iterator end,
		std::vector<value> &constants, exo::map builtins) 
		: K(constants), p(start), end(end), current_scope(&root), builtins(builtins)
	{

	}
	
	void compiler::consume(tokens::token t, const std::string &s) {
		if (p == end)
			throw std::runtime_error(std::to_string(p->line) + ": expected " + s + " near eof");
	
		if (p->tk != t)
			throw std::runtime_error(std::to_string(p->line) + ": expected " + s + " near " + p->str);
			
		++p;
	}

	int compiler::get_local(const std::string &name, bool recurse) {
		lexical_scope *s = current_scope;
		while (s) {
			if (s->L.count(name))
				return s->L[name];
			if (!recurse)
				break;
			s = s->parent;
		}
		return -1;
	}
	
	void compiler::do_block(std::vector<instruction> &I, int &next_register) {
		consume(tokens::LBRACE, "{");
		current_scope = current_scope->new_scope();

        while (p != end && p->tk != tokens::RBRACE) {
            do_statement(I, next_register);
        }

        current_scope = current_scope->parent;
        consume(tokens::RBRACE, "}");
	}

	int compiler::do_expression(std::vector<instruction> &I, int &next_register, int prec) {
		opcodes::opcode op = opcodes::MOVE;
		int r = next_register++;

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
				std::string name = get_identifier();
				int l = get_local(name);
				
				if (l == -1) {
					// check if it is a builtin
					if (builtins.count(name)) {
						if (!B.count(name)) {
							K.push_back(builtins[name]);
							B[name] = K.size() - 1;
						}

						I.push_back(MAKE_ABC(op, r, 1, B[name], 0, 0));
					} else {
						throw std::runtime_error(std::to_string(p->line) + ": unknown identifier '" + name + "'");
					}
				} else {
					I.push_back(MAKE_ABC(op, r, 0, l, 0, 0));
				}
			}
			break;

		case tokens::FUNCTION:
			do_function(I, next_register, r);
			break;
			
		case tokens::LPAREN:
			consume(tokens::LPAREN, "(");
			next_register = r;
			do_expression(I, next_register);	
			consume(tokens::RPAREN, ")");
			break;

		case tokens::LINDEX:  // list
			{
				consume(tokens::LINDEX, "[");
				int inits = do_expression_list(I, next_register, tokens::RINDEX, "]");
				consume(tokens::RINDEX, "]");
				I.push_back(MAKE_ABC(opcodes::NEWLIST, r, 0, inits, 0, 0));
			}
			break;

		case tokens::LBRACE:  // map
			{
				consume(tokens::LBRACE, "{");
				int inits = do_pair_list(I, next_register, tokens::RBRACE, "}");
				consume(tokens::RBRACE, "}");
				I.push_back(MAKE_ABC(opcodes::NEWMAP, r, 0, inits, 0, 0));
			}
			break;

		case tokens::NOT:
			{
				++p;
				next_register = r;
				do_expression(I, next_register, precedence(tokens::NOT));
				I.push_back(MAKE_ABC(opcodes::NOT, r, 0, r, 0, 0));
			}
			break;

		case tokens::BNOT:
			{
				++p;
				next_register = r;
				do_expression(I, next_register, precedence(tokens::BNOT));
				I.push_back(MAKE_ABC(opcodes::BNOT, r, 0, r, 0, 0));
			}
			break;

		case tokens::LEN:
			{
				++p;
				next_register = r;
				do_expression(I, next_register, precedence(tokens::LEN));
				I.push_back(MAKE_ABC(opcodes::LEN, r, 0, r, 0, 0));
			}
			break;

		case tokens::SUB:
			{
				++p;
				next_register = r;
				do_expression(I, next_register, precedence(tokens::UNM));
				I.push_back(MAKE_ABC(opcodes::UNM, r, 0, r, 0, 0));
			}
			break;

		
		default:
			throw std::runtime_error(std::to_string(p->line) + ": unexpected symbol '" + p->str + "'");
		}
		
		do_sub_expression(I, next_register, r, prec);
		next_register = r+1;
		return r;
	}

	void compiler::do_sub_expression(std::vector<instruction> &I, int &next_register,
		int r, int prec)
	{
		int this_prec = precedence(p->tk);
		if (this_prec > prec)
			return;

		int i;
		switch (p->tk) {
		case tokens::ADD:
			++p;
			i = do_expression(I, next_register, this_prec);
			I.push_back(MAKE_ABC(opcodes::ADD, r, 0, r, 0, i));
			break;
			
		case tokens::SUB:
			++p;
			i = do_expression(I, next_register, this_prec);
			I.push_back(MAKE_ABC(opcodes::SUB, r, 0, r, 0, i));
			break;
			
		case tokens::MUL:
			++p;
			i = do_expression(I, next_register, this_prec);
			I.push_back(MAKE_ABC(opcodes::MUL, r, 0, r, 0, i));
			break;
			
		case tokens::DIV:
			++p;
			i = do_expression(I, next_register, this_prec);			
			I.push_back(MAKE_ABC(opcodes::DIV, r, 0, r, 0, i));
			break;

		case tokens::MOD:
			++p;
			i = do_expression(I, next_register, this_prec);			
			I.push_back(MAKE_ABC(opcodes::MOD, r, 0, r, 0, i));
			break;

		case tokens::POW:
			++p;
			i = do_expression(I, next_register, this_prec);			
			I.push_back(MAKE_ABC(opcodes::POW, r, 0, r, 0, i));
			break;
			
		case tokens::LT:
			++p;
			i = do_expression(I, next_register, this_prec);
			I.push_back(MAKE_ABC(opcodes::LT, r, 0, r, 0, i));
			break;
			
		case tokens::LE:
			++p;
			i = do_expression(I, next_register, this_prec);
			I.push_back(MAKE_ABC(opcodes::LE, r, 0, r, 0, i));
			break;

		case tokens::GT:
			++p;
			i = do_expression(I, next_register, this_prec);
			I.push_back(MAKE_ABC(opcodes::LT, r, 0, i, 0, r));
			break;
			
		case tokens::GE:
			++p;
			i = do_expression(I, next_register, this_prec);
			I.push_back(MAKE_ABC(opcodes::LE, r, 0, i, 0, r));
			break;

		case tokens::AND:
			++p;
			i = do_expression(I, next_register, this_prec);
			I.push_back(MAKE_ABC(opcodes::AND, r, 0, i, 0, r));
			break;

		case tokens::OR:
			++p;
			i = do_expression(I, next_register, this_prec);
			I.push_back(MAKE_ABC(opcodes::OR, r, 0, i, 0, r));
			break;

		case tokens::BAND:
			++p;
			i = do_expression(I, next_register, this_prec);
			I.push_back(MAKE_ABC(opcodes::BAND, r, 0, r, 0, i));
			break;

		case tokens::BOR:
			++p;
			i = do_expression(I, next_register, this_prec);
			I.push_back(MAKE_ABC(opcodes::BOR, r, 0, r, 0, i));
			break;

		case tokens::XOR:
			++p;
			i = do_expression(I, next_register, this_prec);
			I.push_back(MAKE_ABC(opcodes::BXOR, r, 0, r, 0, i));
			break;

		case tokens::LSHIFT:
			++p;
			i = do_expression(I, next_register, this_prec);
			I.push_back(MAKE_ABC(opcodes::LSHIFT, r, 0, r, 0, i));
			break;

		case tokens::RSHIFT:
			++p;
			i = do_expression(I, next_register, this_prec);
			I.push_back(MAKE_ABC(opcodes::RSHIFT, r, 0, r, 0, i));
			break;

		case tokens::EQUAL:
			++p;
			i = do_expression(I, next_register, this_prec);
			I.push_back(MAKE_ABC(opcodes::EQL, r, 0, r, 0, i));
			break;

		case tokens::CONCAT:
			++p;
			i = do_expression(I, next_register, this_prec);
			I.push_back(MAKE_ABC(opcodes::CONCAT, r, 0, r, 0, i));
			break;

		case tokens::LPAREN:  // function call
			do_function_call(I, next_register, r, -1);
			I.push_back(MAKE_ABC(opcodes::MOVE, r, 0, 0, 0, 1)); // move the top (return value) to r
			break;

		case tokens::LINDEX:  // index
			++p;
			i = do_expression(I, next_register, this_prec);
			I.push_back(MAKE_ABC(opcodes::GET, r, 0, r, 0, i));
			consume(tokens::RINDEX, "]");
			break;
			
		default:
			return;
		}

		do_sub_expression(I, next_register, r, prec);
	}
	
	std::string compiler::get_identifier() {
		consume(tokens::IDENTIFIER, "name");
		return (p-1)->str;
	}
	
	int compiler::do_variable_assignment(std::vector<instruction> &I, int &next_register) {
		std::vector<std::string> names = get_identifier_list();
		consume(tokens::ASSIGNMENT, "=");
		
		int l = get_local(names[0], false);
		if (l == -1) {
			l = do_expression(I, next_register);
			current_scope->L[names[0]] = l;
		} else {
			int n = do_expression(I, next_register);
			I.push_back(MAKE_ABC(opcodes::MOVE, l, 0, n, 0, 0));
		}

		return l;
	}

	int compiler::do_expression_list(std::vector<instruction> &I, int &next_register,
		tokens::token end_tk, const std::string &end_str)
	{
		std::vector<int> params;
		while (true) {
			if (p == end)
				throw std::runtime_error(std::to_string((p-1)->line) + ": expected '" + end_str + "' near eof");
				
			if (p->tk == end_tk)
				break;
				
			if (!params.empty())
				consume(tokens::SEPARATOR, ",");
			
			int l = do_expression(I, next_register);
			params.push_back(l);
		}

		for (int param : params)
			I.push_back(MAKE_ABC(opcodes::PUSH, 0, 0, param, 0, 0));

		return params.size();
	}

	int compiler::do_pair_list(std::vector<instruction> &I, int &next_register,
		tokens::token end_tk, const std::string &end_str)
	{
		std::vector<std::pair<int, int>> mappings;
		while (true) {
			if (p == end)
				throw std::runtime_error(std::to_string((p-1)->line) + ": expected '" + end_str + "' near eof");
				
			if (p->tk == end_tk)
				break;
				
			if (!mappings.empty() > 0)
				consume(tokens::SEPARATOR, ",");
			
			int k = do_expression(I, next_register);
			consume(tokens::LABEL, ":");
			int v = do_expression(I, next_register);
			mappings.emplace_back(k, v);
		}

		for (auto &mapping : mappings) {
			I.push_back(MAKE_ABC(opcodes::PUSH, 0, 0, mapping.first, 0, 0));
			I.push_back(MAKE_ABC(opcodes::PUSH, 0, 0, mapping.second, 0, 0));
		}

		return mappings.size();
	}
	
	void compiler::do_function_call(std::vector<instruction> &I, int &next_register,
		int f, int ret)
	{
		consume(tokens::LPAREN, "(");
		int args = do_expression_list(I, next_register, tokens::RPAREN, ")");
		consume(tokens::RPAREN, ")");
		I.push_back(MAKE_ABC(opcodes::CALL, (ret+1), 0, f, 0, (args+1)));
	}

	void compiler::do_while(std::vector<instruction> &I, int &next_register) {
		++p;
		
		unsigned start_exp = I.size();

		int r = do_expression(I, next_register);
		
		unsigned start_loop = I.size();
		I.push_back(MAKE_ABC(opcodes::NOOP, 0, 0, 0, 0, 0)); // gets replaced by TEST instruction after while loop has been parsed
		
		do_statement(I, next_register);
		
		I.push_back(MAKE_ABx(opcodes::JMP, 0, 1, (I.size() - start_exp)));
		I[start_loop] = MAKE_AtBx(opcodes::TEST, r, 0, 0, (I.size() - start_loop));
	}

	void compiler::do_if(std::vector<instruction> &I, int &next_register) {
		++p;
		int r = do_expression(I, next_register);
		unsigned start_if = I.size();
		I.push_back(MAKE_ABC(opcodes::NOOP, 0, 0, 0, 0, 0)); // gets replaced by TEST instruction after if statement has been parsed
		do_statement(I, next_register);
		if (p->tk == tokens::ELSE) {
			unsigned start_else = I.size();
			I.push_back(MAKE_ABC(opcodes::NOOP, 0, 0, 0, 0, 0)); // replaced by a jump
			// replace the first NOOP with a TEST instruction
			I[start_if] = MAKE_AtBx(opcodes::TEST, r, 0, 0, (I.size() - start_if));
			consume(tokens::ELSE, "else");
			do_statement(I, next_register);
			// replace the second NOOP with a JMP instruction
			I[start_else] = MAKE_ABx(opcodes::JMP, 0, 0, I.size() - start_else);
		} else {
			// replace the NOOP with a TEST instruction
			I[start_if] = MAKE_AtBx(opcodes::TEST, r, 0, 0, (I.size() - start_if));
		}
	}

	void compiler::do_for(std::vector<instruction> &I, int &next_register) {
		consume(tokens::FOR, "for");

		bool parens = (p->tk == tokens::LPAREN);
		if (parens)
			consume(tokens::LPAREN, "(");

		int l = do_variable_assignment(I, next_register);

		consume(tokens::SEPARATOR, ",");
		int r = do_expression(I, next_register); // the limit

		if (parens)
			consume(tokens::RPAREN, ")");

		unsigned start_for = I.size();
		int p = next_register++;
		I.push_back(MAKE_ABC(opcodes::LE, p, 0, l, 0, r));

		unsigned start_loop = I.size();
		I.push_back(MAKE_ABC(opcodes::NOOP, 0, 0, 0, 0, 0)); // gets replaced by TEST instruction after for loop has been parsed
		do_statement(I, next_register); // the block

		I.push_back(MAKE_ABC(opcodes::INCR, l, 0, 0, 0, 0)); // increment the counter
		I.push_back(MAKE_ABx(opcodes::JMP, 0, 1, (I.size() - start_for))); // jump back to the beginning of the loop
		I[start_loop] = MAKE_AtBx(opcodes::TEST, p, 0, 0, (I.size() - start_loop));
	}

	void compiler::do_return(std::vector<instruction> &I, int &next_register) {
		consume(tokens::RETURN, "return");
		if (p->tk == tokens::RBRACE || p == end) {
			I.push_back(MAKE_ABC(opcodes::RTN, 1, 0, 0, 0, 0));
		} else {
			int args = do_expression_list(I, next_register, tokens::RBRACE, "}");
			I.push_back(MAKE_ABC(opcodes::RTN, args+1, 0, 0, 0, 0));
		}
	}
	
	void compiler::do_statement(std::vector<instruction> &I, int &next_register) {
		switch (p->tk) {
		case tokens::RETURN:
			do_return(I, next_register);
			break;

		case tokens::LBRACE:
			do_block(I, next_register);
			break;
			
		case tokens::WHILE:
			do_while(I, next_register);			
			break;

		case tokens::FOR:
			do_for(I, next_register);
			break;
			
		case tokens::IF:
			do_if(I, next_register);
			break;

		case tokens::FUNCTION:
			do_function(I, next_register);
			break;

		case tokens::OUTER:
			{
				lexical_scope *saved_current = current_scope;
				while (p->tk == tokens::OUTER) {
					consume(tokens::OUTER, "outer");
					current_scope = current_scope->parent;
					if (!current_scope)
						throw std::runtime_error(std::to_string(p->line) + ": already at outer-most scope.");
				}

				do_variable_assignment(I, next_register);				
				current_scope = saved_current;
			}
			break;

		case tokens::IDENTIFIER:  // must be last
			if ((p+1)->tk == tokens::ASSIGNMENT) {
				// local variable assignment
				do_variable_assignment(I, next_register);
				break;
			} else if ((p+1)->tk == tokens::SEPARATOR) {
				// identifier list assignment

			}
			// fall through

		default:
			{
				int r = do_expression(I, next_register, -1);  // force no sub expression
				if (p->tk == tokens::LINDEX) {
					consume(tokens::LINDEX, "[");
					int i = do_expression(I, next_register);
					consume(tokens::RINDEX, "]");
					consume(tokens::ASSIGNMENT, "=");
					int v = do_expression(I, next_register);
					I.push_back(MAKE_ABC(opcodes::SET, r, 0, i, 0, v));
				} else {
					// do the sub expression
					do_sub_expression(I, next_register, r, 99);
				}
				next_register = r;
			}
		}
	}

	std::vector<std::string> compiler::get_identifier_list() 
	{
		std::vector<std::string> params;
		while (true) {
			if (p == end)
				throw std::runtime_error(std::to_string((p-1)->line) + ": unexpected eof near '" + (p-1)->str + "'");
				
			if (p->tk != tokens::SEPARATOR || p->tk != tokens::IDENTIFIER)
				break;
				
			if (!params.empty())
				consume(tokens::SEPARATOR, ",");
			
			params.push_back(get_identifier());
		}

		return params;
	}

	void compiler::do_function(std::vector<instruction> &I, int &next_register) {
		consume(tokens::FUNCTION, "function");
		std::string name = get_identifier();

		int r = get_local(name);
		if (r == -1) {
			r = next_register++;
			current_scope->L[name] = r;
		}

		consume(tokens::LPAREN, "(");
		auto params = get_identifier_list();
		consume(tokens::RPAREN, ")");

		K.emplace_back(compile(params));
		I.push_back(MAKE_ABC(opcodes::MOVE, r, 1, K.size() - 1, 0, 0));
	}

	void compiler::do_function(std::vector<instruction> &I, int &next_register, int r) {
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
			
			params.push_back(get_identifier());
		}
		consume(tokens::RPAREN, ")");

		current_scope = current_scope->new_scope();
		K.emplace_back(compile(params));
		current_scope = current_scope->parent;

		// replace with NEWCLOSURE instruction when I figure out how
		// to do the goddamn things
		I.push_back(MAKE_ABC(opcodes::MOVE, r, 1, K.size() - 1, 0, 0));
	}

	function *compiler::compile(std::vector<std::string> params) {
		std::vector<instruction> I;
		int next_register = 0;

		for (auto param : params)
			current_scope->L[param] = next_register++;

		consume(tokens::LBRACE, "{");
		while (p != end && p->tk != tokens::RBRACE)
            do_statement(I, next_register);
		consume(tokens::RBRACE, "}");
			
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
