#pragma once

#include "value.h"

#include <vector>
#include <string>

namespace exo {
	namespace tokens {
	
		enum token {
			INVALID,
			GLOBAL,			// global
			IDENTIFIER,		// [_a-zA-Z][_0-9a-zA-Z]+
			SEPARATOR,		// ,
			ASSIGNMENT,		// =
			
			BOOLEAN,
			CONSTANT,
			
			FUNCTION,		// function
			RETURN,			// return
			NAMESPACE,		// namespace
			
			LINDEX,			// [
			RINDEX,			// ]
			LBRACE,			// {
			RBRACE,			// }
			LPAREN,			// (
			RPAREN,			// )
			
			ACCESS,			// .
			RESOLUTION,		// ::
			
			ADD,			// +
			SUB,			// -
			MUL,			// *
			DIV,			// /
			POW,			// ^
			MOD,			// %
			LEN,			// #
			CONCAT,			// ..
			
			UNM,			// -
			
			EQUAL,			// ==
			LT,				// <
			LE,				// <=
			GT,				// >
			GE,				// >=
			NOTEQL,			// !=
			
			NOT,			// !
			AND,			// &&
			OR,				// ||
			
			BNOT,			// ~
			BAND,			// &
			BOR,			// |
			XOR,			// @
			
			IF, 			// if
			ELSE,			// else
			FOR,			// for
			WHILE,			// while
			DO,				// do
			BREAK,			// break
			CONTINUE,		// continue
		};
	}
	
	struct symbol {
		tokens::token 	tk;
		std::string		str;
		
		value			k;
		
		unsigned int	line;
		unsigned int	column;
		
		symbol(tokens::token t, const std::string &s, value k=value())
			: tk(t), str(s), k(k), line(0), column(0) {}
	};
	
	std::vector<symbol> tokenise(const std::string &);
}
