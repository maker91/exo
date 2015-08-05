#pragma once

#include "value.h"

#include <vector>
#include <string>

namespace exo {
	namespace tokens {
	
		enum token {
			INVALID,
			IDENTIFIER,		// [_a-zA-Z][_0-9a-zA-Z]+
			SEPARATOR,		// ,
			ASSIGNMENT,		// =
			LABEL,			// :
			
			BOOLEAN,
			CONSTANT,
			NIL,
			
			FUNCTION,		// function
			RETURN,			// return
			
			LINDEX,			// [
			RINDEX,			// ]
			LBRACE,			// {
			RBRACE,			// }
			LPAREN,			// (
			RPAREN,			// )
			
			ACCESS,			// .
			
			ADD,			// +
			SUB,			// -
			MUL,			// *
			DIV,			// /
			POW,			// **
			MOD,			// %
			LEN,			// #
			CONCAT,			// ..
			
			EQUAL,			// ==
			LT,				// <
			LE,				// <=
			GT,				// >
			GE,				// >=
			NOTEQL,			// !=
			
			NOT,			// !
			AND,			// &&
			OR,				// ||

			UNM,			// placeholder for precedence
			UNP,			// placeholder for precedence
			
			BNOT,			// ~
			BAND,			// &
			BOR,			// |
			XOR,			// ^
			LSHIFT,			// >>
			RSHIFT,			// <<
			
			IF, 			// if
			ELSE,			// else
			FOR,			// for
			WHILE,			// while
			DO,				// do
			BREAK,			// break
			CONTINUE,		// continue
			OUTER			// outer
		};
	}
	
	struct symbol {
		tokens::token 	tk;
		std::string		str;
		
		int				k;
		
		unsigned int	line;
		unsigned int	column;
		
		symbol(tokens::token t, const std::string &s, unsigned int l, int k=-1)
			: tk(t), str(s), k(k), line(l), column(0) {}
	};
	
	struct token_result {
		std::vector<symbol> symbols;
		std::vector<value>	constants;
	};
	
	token_result tokenise(const std::string &);

	inline int precedence(tokens::token tk) {
		switch(tk) {
		// 2
		case tokens::LPAREN:
		case tokens::LINDEX:
		case tokens::ACCESS:
			return 2;

		// 3
		case tokens::UNM:
		case tokens::UNP:
		case tokens::NOT:
		case tokens::BNOT:
		case tokens::LEN:
			return 3;

		// 5
		case tokens::MUL:
		case tokens::DIV:
		case tokens::MOD:
			return 5;

		// 6
		case tokens::ADD:
		case tokens::SUB:
			return 6;

		// 7
		case tokens::LSHIFT:
		case tokens::RSHIFT:
			return 7;

		// 8
		case tokens::LT:
		case tokens::LE:
		case tokens::GT:
		case tokens::GE:
			return 8;

		// 9
		case tokens::EQUAL:
		case tokens::NOTEQL:
			return 9;

		// 10
		case tokens::BAND:
			return 10;

		// 11
		case tokens::XOR:
			return 11;

		// 12
		case tokens::BOR:
			return 12;

		// 13
		case tokens::AND:
			return 13;

		// 14
		case tokens::OR:
			return 14;

		default:
			return 99;
		}
	}
}
