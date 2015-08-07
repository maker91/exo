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
	
	void compiler::do_block(std::vector<instruction> &I) {
		consume(tokens::LBRACE, "{");

        while (p != end && p->tk != tokens::RBRACE) {
            do_statement(I);
        }

        consume(tokens::RBRACE, "}");
	}

	int compiler::do_expression(std::vector<instruction> &I, int o, bool force_out, int prec) {
		/*
		int o is the output location of the expression result
		bool force_out specificies whether the output location should be forced to o
		int prec is the precedence of the previous expression part

		returns -1 if the result is on the stack, otherwise returns the register number
		*/
		int r = o; // intermediate result)
		switch (p->tk) {
		case tokens::CONSTANT:
			I.push_back(MAKE_ABC(opcodes::LOAD, o, 1, p->k, 0, 0));
			++p;
			break;
			
		case tokens::BOOLEAN:
			I.push_back(MAKE_ABC(opcodes::LOAD, o, 1, p->str == "true" ? 1 : 2, 0, 0));
			++p;
			break;
			
		case tokens::NIL:
			I.push_back(MAKE_ABC(opcodes::LOAD, o, 1, 0, 0, 0));
			++p;
			break;
			
		case tokens::IDENTIFIER:
			{
				std::string name = get_identifier();
				int l = get_local(name);
				
				if (l == -1) {
					// check if it is a builtin
					if (builtins.count(name))
						I.push_back(MAKE_ABC(opcodes::BUILTIN, o, 1, (p-1)->k, 0, 0));
					else
						throw std::runtime_error(std::to_string(p->line) + ": unknown identifier '" + name + "'");
				} else {
					r = l;
				}
			}
			break;

		case tokens::FUNCTION:
			do_anonymous_function(I, o);
			break;
			
		case tokens::LPAREN:
			consume(tokens::LPAREN, "(");
			o = do_expression(I, o, force_out);
			consume(tokens::RPAREN, ")");
			break;

		case tokens::LINDEX:  // list
			{
				consume(tokens::LINDEX, "[");
				int inits = 0;
				if (p->tk != tokens::RINDEX)
					inits = do_expression_list(I);
				consume(tokens::RINDEX, "]");
				I.push_back(MAKE_ABC(opcodes::NEWLIST, o, 0, inits, 0, 0));
			}
			break;

		case tokens::LBRACE:  // map
			{
				consume(tokens::LBRACE, "{");
				int inits = 0;
				if (p->tk != tokens::RBRACE)
					inits = do_pair_list(I);
				consume(tokens::RBRACE, "}");
				I.push_back(MAKE_ABC(opcodes::NEWMAP, o, 0, inits, 0, 0));
			}
			break;

		case tokens::NOT:
			{
				++p;
				int r = do_expression(I, -1, false, precedence(tokens::NOT));
				I.push_back(MAKE_ABC(opcodes::NOT, o, 0, r, 0, 0));
			}
			break;

		case tokens::BNOT:
			{
				++p;
				int r = do_expression(I, -1, false, precedence(tokens::BNOT));
				I.push_back(MAKE_ABC(opcodes::BNOT, o, 0, r, 0, 0));
			}
			break;

		case tokens::LEN:
			{
				++p;
				int r = do_expression(I, -1, false, precedence(tokens::LEN));
				I.push_back(MAKE_ABC(opcodes::LEN, o, 0, r, 0, 0));
			}
			break;

		case tokens::SUB:
			{
				++p;
				int r = do_expression(I, -1, false, precedence(tokens::UNM));
				I.push_back(MAKE_ABC(opcodes::UNM, o, 0, r, 0, 0));
			}
			break;

		
		default:
			throw std::runtime_error(std::to_string(p->line) + ": unexpected symbol '" + p->str + "'");
		}
		
		r = do_sub_expression(I, o, r, prec); // may result in 
		if (force_out && r!=o) {
			I.push_back(MAKE_ABC(opcodes::LOAD, o, 0, r, 0, 0));
			r = o;
		}
		return r;
	}

	int compiler::do_sub_expression(std::vector<instruction> &I, int o, int r, int prec)
	{
		/*
		int o is the location to store the result of the sub expression
			o will usually be -1, indicating to push it to the stack
		int r is the location of the intermediate value to use in the sub expression
			it will usually be -1, indicating its on the stack
		int prec is the precedence level of the last part of the expression
		*/
		int this_prec = precedence(p->tk);
		if (this_prec > prec)
			return r; // return the intermediate result

		int i;
		switch (p->tk) {
		case tokens::ADD:
			++p;
			i = do_expression(I, -1, false, this_prec);
			I.push_back(MAKE_ABC(opcodes::ADD, o, 0, r, 0, i));
			break;
			
		case tokens::SUB:
			++p;
			i = do_expression(I, -1, false, this_prec);
			I.push_back(MAKE_ABC(opcodes::SUB, o, 0, r, 0, i));
			break;
			
		case tokens::MUL:
			++p;
			i = do_expression(I, -1, false, this_prec);
			I.push_back(MAKE_ABC(opcodes::MUL, o, 0, r, 0, i));
			break;
			
		case tokens::DIV:
			++p;
			i = do_expression(I, -1, false, this_prec);			
			I.push_back(MAKE_ABC(opcodes::DIV, o, 0, r, 0, i));
			break;

		case tokens::MOD:
			++p;
			i = do_expression(I, -1, false, this_prec);			
			I.push_back(MAKE_ABC(opcodes::MOD, o, 0, r, 0, i));
			break;

		case tokens::POW:
			++p;
			i = do_expression(I, -1, false, this_prec);			
			I.push_back(MAKE_ABC(opcodes::POW, o, 0, r, 0, i));
			break;
			
		case tokens::LT:
			++p;
			i = do_expression(I, -1, false, this_prec);
			I.push_back(MAKE_ABC(opcodes::LT, o, 0, r, 0, i));
			break;
			
		case tokens::LE:
			++p;
			i = do_expression(I, -1, false, this_prec);
			I.push_back(MAKE_ABC(opcodes::LE, o, 0, r, 0, i));
			break;

		case tokens::GT:
			++p;
			i = do_expression(I, -1, false, this_prec);
			I.push_back(MAKE_ABC(opcodes::LT, o, 0, i, 0, r));
			break;
			
		case tokens::GE:
			++p;
			i = do_expression(I, -1, false, this_prec);
			I.push_back(MAKE_ABC(opcodes::LE, o, 0, i, 0, r));
			break;

		case tokens::AND:
			++p;
			i = do_expression(I, -1, false, this_prec);
			I.push_back(MAKE_ABC(opcodes::AND, o, 0, r, 0, i));
			break;

		case tokens::OR:
			++p;
			i = do_expression(I, -1, false, this_prec);
			I.push_back(MAKE_ABC(opcodes::OR, o, 0, r, 0, i));
			break;

		case tokens::BAND:
			++p;
			i = do_expression(I, -1, false, this_prec);
			I.push_back(MAKE_ABC(opcodes::BAND, o, 0, r, 0, i));
			break;

		case tokens::BOR:
			++p;
			i = do_expression(I, -1, false, this_prec);
			I.push_back(MAKE_ABC(opcodes::BOR, o, 0, r, 0, i));
			break;

		case tokens::XOR:
			++p;
			i = do_expression(I, -1, false, this_prec);
			I.push_back(MAKE_ABC(opcodes::BXOR, o, 0, r, 0, i));
			break;

		case tokens::LSHIFT:
			++p;
			i = do_expression(I, -1, false, this_prec);
			I.push_back(MAKE_ABC(opcodes::LSHIFT, o, 0, r, 0, i));
			break;

		case tokens::RSHIFT:
			++p;
			i = do_expression(I, -1, false, this_prec);
			I.push_back(MAKE_ABC(opcodes::RSHIFT, o, 0, r, 0, i));
			break;

		case tokens::EQUAL:
			++p;
			i = do_expression(I, -1, false, this_prec);
			I.push_back(MAKE_ABC(opcodes::EQL, o, 0, r, 0, i));
			break;

		case tokens::CONCAT:
			++p;
			i = do_expression(I, -1, false, this_prec);
			I.push_back(MAKE_ABC(opcodes::CONCAT, o, 0, r, 0, i));
			break;

		case tokens::LPAREN:  // function call
			// ensure that functions are stored in a register so they can be called properly
			// the register is only temporary
			if (r == -1) {
				r = next_register++;
				I.push_back(MAKE_ABC(opcodes::LOAD, r, 0, -1, 0, 0));
			}
			do_function_call(I, r, -1);
			// move the return values into the required registers
			if (o != -1)
				I.push_back(MAKE_ABC(opcodes::LOAD, o, 0, -1, 0, 0));
			break;

		case tokens::LINDEX:  // index
			consume(tokens::LINDEX, "[");
			i = do_expression(I, -1, false);
			I.push_back(MAKE_ABC(opcodes::GETITEM, o, 0, r, 0, i));
			consume(tokens::RINDEX, "]");
			break;
			
		default:
			return r;
		}

		return do_sub_expression(I, o, o, prec);
	}
	
	std::string compiler::get_identifier() {
		consume(tokens::IDENTIFIER, "name");
		return (p-1)->str;
	}
	
	int compiler::do_variable_assignment(std::vector<instruction> &I) {
		//std::vector<std::string> names = get_identifier_list();
		std::string name = get_identifier();
		consume(tokens::ASSIGNMENT, "=");
		
		int l = get_local(name, false);
		if (l == -1) {
			l = do_expression(I, next_register++, true);
			current_scope->L[name] = l;
		} else {
			do_expression(I, l, true);
		}

		return l;
	}

	int compiler::do_expression_list(std::vector<instruction> &I)
	{
		int count = 0;
		while (true) {
			if (p == end)
				throw std::runtime_error(std::to_string((p-1)->line) + ": unexpected eof near'" + (p-1)->str + "'");
				
			if (count > 0) {
				if (p->tk == tokens::SEPARATOR)
					consume(tokens::SEPARATOR, ",");
				else
					break;
			}
			
			count++;
			do_expression(I, -1, true); // output expression result to the stack
		}

		return count;
	}

	int compiler::do_pair_list(std::vector<instruction> &I)
	{
		int count = 0;
		while (true) {
			if (p == end)
				throw std::runtime_error(std::to_string((p-1)->line) + ": unexpected eof near'" + (p-1)->str + "'");
				
			if (count > 0) {
				if (p->tk == tokens::SEPARATOR)
					consume(tokens::SEPARATOR, ",");
				else
					break;
			}
			
			do_expression(I, -1, true);
			consume(tokens::LABEL, ":");
			do_expression(I, -1, true);
			count++;
		}

		return count;
	}
	
	void compiler::do_function_call(std::vector<instruction> &I, int f, int ret)
	{
		consume(tokens::LPAREN, "(");
		int args = 0;
		if (p->tk != tokens::RPAREN)
			args = do_expression_list(I);
		consume(tokens::RPAREN, ")");
		I.push_back(MAKE_ABC(opcodes::CALL, (ret+1), 0, f, 0, (args+1)));
	}

	void compiler::do_while(std::vector<instruction> &I) {
		++p;
		
		unsigned start_exp = I.size();

		int r = do_expression(I);
		
		unsigned start_loop = I.size();
		I.push_back(MAKE_ABC(opcodes::NOOP, 0, 0, 0, 0, 0)); // gets replaced by TEST instruction after while loop has been parsed
		
		do_statement(I);
		
		I.push_back(MAKE_ABx(opcodes::JMP, 0, 1, (I.size() - start_exp)));
		I[start_loop] = MAKE_AtBx(opcodes::TEST, r, 0, 0, (I.size() - start_loop));
	}

	void compiler::do_if(std::vector<instruction> &I) {
		++p;
		int r = do_expression(I);
		unsigned start_if = I.size();
		I.push_back(MAKE_ABC(opcodes::NOOP, 0, 0, 0, 0, 0)); // gets replaced by TEST instruction after if statement has been parsed
		do_statement(I);
		if (p->tk == tokens::ELSE) {
			unsigned start_else = I.size();
			I.push_back(MAKE_ABC(opcodes::NOOP, 0, 0, 0, 0, 0)); // replaced by a jump
			// replace the first NOOP with a TEST instruction
			I[start_if] = MAKE_AtBx(opcodes::TEST, r, 0, 0, (I.size() - start_if));
			consume(tokens::ELSE, "else");
			do_statement(I);
			// replace the second NOOP with a JMP instruction
			I[start_else] = MAKE_ABx(opcodes::JMP, 0, 0, I.size() - start_else);
		} else {
			// replace the NOOP with a TEST instruction
			I[start_if] = MAKE_AtBx(opcodes::TEST, r, 0, 0, (I.size() - start_if));
		}
	}

	void compiler::do_for(std::vector<instruction> &I) {
		consume(tokens::FOR, "for");

		bool parens = (p->tk == tokens::LPAREN);
		if (parens)
			consume(tokens::LPAREN, "(");

		int counter = do_variable_assignment(I);
		consume(tokens::SEPARATOR, ",");

		// load the limit into a register
		int limit = next_register++;
		do_expression(I, limit, true);

		if (parens)
			consume(tokens::RPAREN, ")");

		unsigned jump_back = I.size();
		I.push_back(MAKE_ABC(opcodes::LE, -1, 0, counter, 0, limit));

		unsigned jump_skip = I.size();
		I.push_back(MAKE_ABC(opcodes::NOOP, 0, 0, 0, 0, 0)); // gets replaced by TEST instruction after for loop has been parsed
		do_statement(I); // the block

		I.push_back(MAKE_ABC(opcodes::INCR, counter, 0, 0, 0, 0)); // increment the counter
		I.push_back(MAKE_ABx(opcodes::JMP, 0, 1, (I.size() - jump_back))); // jump back to the beginning of the loop
		I[jump_skip] = MAKE_AtBx(opcodes::TEST, -1, 0, 0, (I.size() - jump_skip));
	}

	void compiler::do_return(std::vector<instruction> &I) {
		consume(tokens::RETURN, "return");
		if (p->tk == tokens::RBRACE || p == end) {
			I.push_back(MAKE_ABC(opcodes::RTN, 1, 0, 0, 0, 0));
		} else {
			int args = do_expression_list(I);
			I.push_back(MAKE_ABC(opcodes::RTN, args+1, 0, 0, 0, 0));
		}
	}
	
	void compiler::do_statement(std::vector<instruction> &I) {
		switch (p->tk) {
		case tokens::RETURN:
			do_return(I);
			break;

		case tokens::LBRACE:
			do_block(I);
			break;
			
		case tokens::WHILE:
			do_while(I);			
			break;

		case tokens::FOR:
			do_for(I);
			break;
			
		case tokens::IF:
			do_if(I);
			break;

		case tokens::FUNCTION:
			if ((p+1)->tk == tokens::IDENTIFIER)
				do_function(I);
			else
				do_expression(I);
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

				do_variable_assignment(I);				
				current_scope = saved_current;
			}
			break;

		case tokens::IDENTIFIER:  // must be last
			if ((p+1)->tk == tokens::ASSIGNMENT) {
				// local variable assignment
				do_variable_assignment(I);
				break;
			} else if ((p+1)->tk == tokens::SEPARATOR) {
				// identifier list assignment
				break;
			}
			// fall through

		default:
			{
				int r = do_expression(I, -1, false, -1);  // force no sub expression
				if (p->tk == tokens::LINDEX) {
					consume(tokens::LINDEX, "[");
					int i = do_expression(I);
					consume(tokens::RINDEX, "]");
					if (p->tk == tokens::ASSIGNMENT) {
						consume(tokens::ASSIGNMENT, "=");
						int v = do_expression(I);
						I.push_back(MAKE_ABC(opcodes::SETITEM, r, 0, i, 0, v));
					} else {
						// do the sub expression
						I.push_back(MAKE_ABC(opcodes::GETITEM, r, 0, r, 0, i));
						do_sub_expression(I, -1, r, 99);
					}
				} else {
					// do the sub expression
					do_sub_expression(I, -1, r, 99);
				}
			}
		}
	}

	std::vector<std::string> compiler::get_identifier_list() 
	{
		std::vector<std::string> params;
		while (true) {
			if (p == end)
				throw std::runtime_error(std::to_string((p-1)->line) + ": unexpected eof near '" + (p-1)->str + "'");
				
			if (p->tk != tokens::SEPARATOR && p->tk != tokens::IDENTIFIER)
				break;
				
			if (!params.empty())
				consume(tokens::SEPARATOR, ",");
			
			params.push_back(get_identifier());
		}

		return params;
	}

	void compiler::do_function(std::vector<instruction> &I) {
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

		current_scope = current_scope->new_scope();
		K.emplace_back(compile(params));
		current_scope = current_scope->parent;

		// replace with NEWCLOSURE instruction when I figure out how
		// to do the goddamn things
		I.push_back(MAKE_ABC(opcodes::LOAD, r, 1, K.size() - 1, 0, 0));
	}

	void compiler::do_anonymous_function(std::vector<instruction> &I, int o) {
		consume(tokens::FUNCTION, "function");
		
		consume(tokens::LPAREN, "(");
		auto params = get_identifier_list();
		consume(tokens::RPAREN, ")");

		current_scope = current_scope->new_scope();
		K.emplace_back(compile(params));
		current_scope = current_scope->parent;

		// replace with NEWCLOSURE instruction when I figure out how
		// to do the goddamn things
		I.push_back(MAKE_ABC(opcodes::LOAD, o, 1, K.size() - 1, 0, 0));
	}

	function *compiler::compile(std::vector<std::string> params) {
		std::vector<instruction> I;

		int param_start = next_register;
		for (auto param : params)
			current_scope->L[param] = next_register++;

		consume(tokens::LBRACE, "{");
		while (p != end && p->tk != tokens::RBRACE)
            do_statement(I);
		consume(tokens::RBRACE, "}");
			
		I.push_back(MAKE_ABC(opcodes::RTN, 1, 0, 0, 0, 0));
		
		/*
		std::cout << "I: " << std::endl;
		for (instruction i : I) {
			opcodes::opcode op = GET_OP(i);
			
			std::cout << opcode_name(op) << "\t" << GET_A(i) << " " << IS_BK(i) << " " << GET_B(i) << " ";
			std::cout << IS_CK(i) << " " << GET_C(i);
			std::cout << "\t(" << GET_A(i) << " " << GET_T(i) << " " << (int)GET_Bx(i) << ")" << std::endl;
		}
		*/

		return new function(param_start, params.size(), I, K);
	}
}
