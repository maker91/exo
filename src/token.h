#pragma once

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
			
			NUMBER,			// [-+]?[0-9]*.?[0-9]+[eE]?[0-9]+
			INTEGER,		// [+-]?[0-9]+[eE]?[0-9]+
			STRING,			// ".+"
			CHAR,			// '.'
			BOOLEAN,		// true/false
			FUNCTION,		// function
			
			RETURN,			// return
			
			LINDEX,			// [
			RINDEX,			// ]
			LBRACE,			// {
			RBRACE,			// }
			LPAREN,			// (
			RPAREN,			// )
			
			OBJACCESS,		// .
			NAMESPACE,		// ::
			
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
			BXOR,			// @
			
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
		unsigned int	line;
		unsigned int	column;
		
		symbol(tokens::token t, const std::string &s)
			: tk(t), str(s), line(0), column(0) {}
	};
	
	std::vector<symbol> tokenise(const std::string &);
}
