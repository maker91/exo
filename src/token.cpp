#include "token.h"
#include "type.h"

#include <iostream>
#include <stdexcept>
#include <cctype>
#include <cstdlib>

namespace exo {

	std::vector<symbol> tokenise(const std::string &src) {
		std::vector<symbol> symbols;
		std::vector<char> src_v(std::begin(src), std::end(src));
		char *end = &src_v[src_v.size()-1]+1;
		
		unsigned line = 1;
		for (char *p=&src_v[0]; p!=end;) {
			while (std::isspace(*p)) { // skip whitespace
				if (*p == '\n')
					line++;
					
				++p;
			}
		
			if (*p == '_' || std::isalpha(*p)) { // keyword or identifier
				std::vector<char> s;
				s.push_back(*p);
				++p;
				
				while (!std::isspace(*p) && p!=end) {
					if (!std::isalnum(*p))
						break;
						
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
			} else if (*p == ',') {
				symbols.emplace_back(tokens::SEPARATOR, ",");
				++p;
			} else if (*p == '=') { // assignment or equals
				++p;
				if (*p == '=') {
					symbols.emplace_back(tokens::EQUAL, "==");
					++p;
				} else {
					symbols.emplace_back(tokens::ASSIGNMENT, "=");
				}
			} else if (*p == '[') {
				symbols.emplace_back(tokens::LINDEX, "[");
				++p;
			} else if (*p == ']') {
				symbols.emplace_back(tokens::RINDEX, "]");
				++p;
			} else if (*p == '{') {
				symbols.emplace_back(tokens::LBRACE, "{");
				++p;
			} else if (*p == '}') {
				symbols.emplace_back(tokens::RBRACE, "}");
				++p;
			} else if (*p == '(') {
				symbols.emplace_back(tokens::LPAREN, "(");
				++p;
			} else if (*p == ')') {
				symbols.emplace_back(tokens::RPAREN, ")");
				++p;
			} else if (*p == '.') { // access or concat or number
				++p;
				if (*p == '.') {
					symbols.emplace_back(tokens::CONCAT, "..");
					++p;
				} else if (std::isdigit(*p)) {
					--p;
					goto donumber;
				} else {
					symbols.emplace_back(tokens::ACCESS, ".");
				}
			} else if (*p == ':') { // namespace?
				++p;
				if (*p == ':') {
					symbols.emplace_back(tokens::NAMESPACE, "::");
					++p;
				} else {
					throw std::runtime_error(std::to_string(line) + ": unexpected symbol near ':'");
				}
			} else if (*p == '+') {
				symbols.emplace_back(tokens::ADD, "+");
				++p;
			} else if (*p == '-') {
				symbols.emplace_back(tokens::SUB, "-");
				++p;
			} else if (*p == '*') {
				symbols.emplace_back(tokens::MUL, "*");
				++p;
			} else if (*p == '/') {
				symbols.emplace_back(tokens::DIV, "/");
				++p;
			} else if (*p == '^') {
				symbols.emplace_back(tokens::POW, "^");
				++p;
			} else if (*p == '%') {
				symbols.emplace_back(tokens::MOD, "%");
				++p;
			} else if (*p == '#') {
				symbols.emplace_back(tokens::LEN, "#");
				++p;
			} else if (*p == '<') { // LT or LE
				++p;
				if (*p == '=') {
					symbols.emplace_back(tokens::LE, "<=");
					++p;
				} else {
					symbols.emplace_back(tokens::LT, "<");
				}
			} else if (*p == '>') { // GT or GE
				++p;
				if (*p == '=') {
					symbols.emplace_back(tokens::GE, ">=");
					++p;
				} else {
					symbols.emplace_back(tokens::GT, ">");
				}
			} else if (*p == '!') { // not or noteql
				++p;
				if (*p == '=') {
					symbols.emplace_back(tokens::NOTEQL, "!=");
					++p;
				} else {
					symbols.emplace_back(tokens::NOT, "!");
				}
			} else if (*p == '&') { // AND or BAND
				++p;
				if (*p == '&') {
					symbols.emplace_back(tokens::AND, "&&");
					++p;
				} else {
					symbols.emplace_back(tokens::BAND, "&");
				}
			} else if (*p == '|') { // OR or BOR
				++p;
				if (*p == '|') {
					symbols.emplace_back(tokens::OR, "||");
					++p;
				} else {
					symbols.emplace_back(tokens::BOR, "|");
				}
			} else if (*p == '~') {
				symbols.emplace_back(tokens::BNOT, "~");
				++p;
			} else if (*p == '@') {
				symbols.emplace_back(tokens::XOR, "@");
				++p;
			} else if (*p == '\'') {
				++p;
				char c = *p;
				++p;
				
				if (*p == '\'') {
					symbols.emplace_back(tokens::CHAR, std::string("")+c);
					++p;
				} else {
					throw std::runtime_error(std::to_string(line) + ": unfinished char");
				}
			} else if (*p == '"') {
				++p;
				
				std::vector<char> s;
				while (*p != '"') {
					if (*p == '\n' || p == end)
						throw std::runtime_error(std::to_string(line) + ": unfinished string");
						
					s.push_back(*p);
					++p;
				}
				
				++p;
				s.push_back('\0');
				std::string str(std::begin(s), std::end(s));
				symbols.emplace_back(tokens::STRING, str);
			} else if (std::isdigit(*p)) {
			donumber:
				std::vector<char> s;
				s.push_back(*p);
				
				bool dp = *p == '.';
				bool e = false;
				
				++p;
				while (!std::isspace(*p) && p != end) {
					if (*p == '.') {
						if (dp)
							throw std::runtime_error(std::to_string(line) + ": malformed number");
						
						dp = true;
					} else if (*p == 'e' || *p == 'E') {
						if (e)
							throw std::runtime_error(std::to_string(line) + ": malformed number");
							
						e = true;
					} else if (!std::isdigit(*p)) {
						break;
					}
					
					s.push_back(*p);
					++p;
				}
				
				if (s.back() == '.' || s.back() == 'E' || s.back() == 'e')
					throw std::runtime_error(std::to_string(line) + ": malformed number");
				
				s.push_back('\0');
				if (dp || e) {
					number n = std::strtod(&s[0], nullptr);
					std::cout << n << std::endl;
					symbols.emplace_back(tokens::NUMBER, std::string(&s[0]));
				} else {
					integer i = std::strtol(&s[0], nullptr, 10);
					std::cout << i << std::endl;
					symbols.emplace_back(tokens::INTEGER, std::string(&s[0]));
				}			
			} else {
				throw std::runtime_error(std::to_string(line) + ": unexpected symbol");
			}
		}
		
		return symbols;
	}
}
