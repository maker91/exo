#include "token.h"

#include <stdexcept>
#include <cctype>

namespace exo {

	std::vector<symbol> tokenise(const std::string &src) {
		std::vector<symbol> symbols;
		std::vector<char> src_v(std::begin(src), std::end(src));
		char *end = &src_v[src_v.size()-1]+1;
		
		for (char *p=&src_v[0]; p!=end;) {
			while (std::isspace(*p)) { // skip whitespace
				++p;
			}
		
			if (*p == '_' || std::isalnum(*p)) { // keyword or identifier
				std::vector<char> s;
				s.push_back(*p);
				++p;
				
				while (!std::isspace(*p) && p!=end) {
					if (!std::isalnum(*p))
						throw std::runtime_error("unexpected symbol");
						
					s.push_back(*p);
					++p;
				}
				
				s.push_back('\0');
				std::string str(&s[0]);
				
				if (str == "global")
					symbols.emplace_back(tokens::GLOBAL, str);
				else if (str == "true" || str == "false")
					symbols.emplace_back(tokens::BOOLEAN, str);
				else if (str == "function")
					symbols.emplace_back(tokens::FUNCTION, str);
				else if (str == "return")
					symbols.emplace_back(tokens::RETURN, str);
				else if (str == "if")
					symbols.emplace_back(tokens::IF, str);
				else if (str == "else")
					symbols.emplace_back(tokens::ELSE, str);
				else if (str == "for")
					symbols.emplace_back(tokens::FOR, str);
				else if (str == "while")
					symbols.emplace_back(tokens::WHILE, str);
				else if (str == "do")
					symbols.emplace_back(tokens::DO, str);
				else if (str == "break")
					symbols.emplace_back(tokens::BREAK, str);
				else if (str == "continue")
					symbols.emplace_back(tokens::CONTINUE, str);
				else
					symbols.emplace_back(tokens::IDENTIFIER, str);
			}
		}
		
		return symbols;
	}
}
