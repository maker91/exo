#include "token.h"
#include "type.h"
#include "value.h"

#include <iostream>
#include <stdexcept>
#include <cctype>
#include <cstdlib>
#include <string>
#include <unordered_map>

namespace exo {

	token_result tokenise(const std::string &src) {
		std::vector<exo::value> K = {exo::value(), exo::value(true), exo::value(false)};
		std::vector<symbol> symbols;
		symbols.emplace_back(tokens::LBRACE, "{", 0);
		
		std::vector<char> src_v(std::begin(src), std::end(src));
		char *end = &src_v[src_v.size()-1]+1;
		
		unsigned line = 1;
		for (char *p=&src_v[0]; p!=end;) {
			while (std::isspace(*p)) { // skip whitespace
				if (*p == '\n')
					line++;
					
				++p;
			}
			
			if (*p == '\0')
				break;
		
			if (*p == '_' || std::isalpha(*p)) { // keyword or identifier
				std::vector<char> s;
				s.push_back(*p);
				++p;
				
				while (!std::isspace(*p) && p!=end) {
					if (!std::isalnum(*p) && *p != '_')
						break;
						
					s.push_back(*p);
					++p;
				}
				
				s.push_back('\0');
				std::string str(&s[0]);
				
				if (str == "true")
					symbols.emplace_back(tokens::BOOLEAN, str, line);
				else if (str == "false")
					symbols.emplace_back(tokens::BOOLEAN, str, line);
				else if (str == "nil")
					symbols.emplace_back(tokens::NIL, str, line);
				else if (str == "function")
					symbols.emplace_back(tokens::FUNCTION, str, line);
				else if (str == "return")
					symbols.emplace_back(tokens::RETURN, str, line);
				else if (str == "if")
					symbols.emplace_back(tokens::IF, str, line);
				else if (str == "else")
					symbols.emplace_back(tokens::ELSE, str, line);
				else if (str == "for")
					symbols.emplace_back(tokens::FOR, str, line);
				else if (str == "while")
					symbols.emplace_back(tokens::WHILE, str, line);
				else if (str == "do")
					symbols.emplace_back(tokens::DO, str, line);
				else if (str == "break")
					symbols.emplace_back(tokens::BREAK, str, line);
				else if (str == "continue")
					symbols.emplace_back(tokens::CONTINUE, str, line);
				else if (str == "outer")
					symbols.emplace_back(tokens::OUTER, str, line);
				else {
					value v(str);
					auto loc = std::find(K.begin(), K.end(), v);
					if (loc == K.end()) {
						K.push_back(v);
						loc = K.end() - 1;
					}
					auto pos = std::distance(K.begin(), loc);

					symbols.emplace_back(tokens::IDENTIFIER, str, line, pos);
				}
			} else if (*p == ',') {
				symbols.emplace_back(tokens::SEPARATOR, ",", line);
				++p;
			} else if (*p == '=') { // assignment or equals
				++p;
				if (*p == '=') {
					symbols.emplace_back(tokens::EQUAL, "==", line);
					++p;
				} else {
					symbols.emplace_back(tokens::ASSIGNMENT, "=", line);
				}
			} else if (*p == '[') {
				symbols.emplace_back(tokens::LINDEX, "[", line);
				++p;
			} else if (*p == ']') {
				symbols.emplace_back(tokens::RINDEX, "]", line);
				++p;
			} else if (*p == '{') {
				symbols.emplace_back(tokens::LBRACE, "{", line);
				++p;
			} else if (*p == '}') {
				symbols.emplace_back(tokens::RBRACE, "}", line);
				++p;
			} else if (*p == '(') {
				symbols.emplace_back(tokens::LPAREN, "(", line);
				++p;
			} else if (*p == ')') {
				symbols.emplace_back(tokens::RPAREN, ")", line);
				++p;
			} else if (*p == '.') { // access or concat or number
				++p;
				if (*p == '.') {
					symbols.emplace_back(tokens::CONCAT, "..", line);
					++p;
				} else if (std::isdigit(*p)) {
					--p;
					goto donumber;
				} else {
					symbols.emplace_back(tokens::ACCESS, ".", line);
				}
			} else if (*p == ':') {
				symbols.emplace_back(tokens::LABEL, ":", line);
				++p;
			} else if (*p == '+') {
				symbols.emplace_back(tokens::ADD, "+", line);
				++p;
			} else if (*p == '-') {
				symbols.emplace_back(tokens::SUB, "-", line);
				++p;
			} else if (*p == '*') {
				++p;
				if (*p == '*') {
					symbols.emplace_back(tokens::POW, "**", line);
					++p;
				} else {
					symbols.emplace_back(tokens::MUL, "*", line);
				}
			} else if (*p == '/') {
				symbols.emplace_back(tokens::DIV, "/", line);
				++p;
			} else if (*p == '^') {
				symbols.emplace_back(tokens::XOR, "^", line);
				++p;
			} else if (*p == '%') {
				symbols.emplace_back(tokens::MOD, "%", line);
				++p;
			} else if (*p == '#') {
				symbols.emplace_back(tokens::LEN, "#", line);
				++p;
			} else if (*p == '<') { // LT or LE or LSHIFT
				++p;
				if (*p == '=') {
					symbols.emplace_back(tokens::LE, "<=", line);
					++p;
				} else if (*p == '<') {
					symbols.emplace_back(tokens::LSHIFT, "<<", line);
					++p;
				} else {
					symbols.emplace_back(tokens::LT, "<", line);
				}
			} else if (*p == '>') { // GT or GE or RSHIFT
				++p;
				if (*p == '=') {
					symbols.emplace_back(tokens::GE, ">=", line);
					++p;
				} else if (*p == '>') {
					symbols.emplace_back(tokens::RSHIFT, ">>", line);
					++p;
				} else {
					symbols.emplace_back(tokens::GT, ">", line);
				}
			} else if (*p == '!') { // not or noteql
				++p;
				if (*p == '=') {
					symbols.emplace_back(tokens::NOTEQL, "!=", line);
					++p;
				} else {
					symbols.emplace_back(tokens::NOT, "!", line);
				}
			} else if (*p == '&') { // AND or BAND
				++p;
				if (*p == '&') {
					symbols.emplace_back(tokens::AND, "&&", line);
					++p;
				} else {
					symbols.emplace_back(tokens::BAND, "&", line);
				}
			} else if (*p == '|') { // OR or BOR
				++p;
				if (*p == '|') {
					symbols.emplace_back(tokens::OR, "||", line);
					++p;
				} else {
					symbols.emplace_back(tokens::BOR, "|", line);
				}
			} else if (*p == '~') {
				symbols.emplace_back(tokens::BNOT, "~", line);
				++p;
			} else if (*p == '@') {
				symbols.emplace_back(tokens::XOR, "@", line);
				++p;
			} else if (*p == '\'') {
				++p;
				char c = *p;
				++p;
				
				if (*p == '\'') {
					value v((byte)c);
					auto loc = std::find(K.begin(), K.end(), v);
					if (loc == K.end()) {
						K.push_back(v);
						loc = K.end() - 1;
					}
					auto pos = std::distance(K.begin(), loc);
						
					symbols.emplace_back(tokens::CONSTANT, std::string("")+c, line, pos);
					++p;
				} else {
					throw std::runtime_error(std::to_string(line) + ": unfinished char");
				}
			} else if (*p == '"') {
				char *start = ++p;
				size_t len = 0;
				
				for (; *p != '"'; ++p, ++len)
					if (*p == '\n' || p == end)
						throw std::runtime_error(std::to_string(line) + ": unfinished string");
				
				++p;
				std::string str(start, len);
				
				value v(str);
				auto loc = std::find(K.begin(), K.end(), v);
				if (loc == K.end()) {
					K.push_back(v);
					loc = K.end() - 1;
				}
				auto pos = std::distance(K.begin(), loc);

				symbols.emplace_back(tokens::CONSTANT, str, line, pos);
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
					
					value v(n);
					auto loc = std::find(K.begin(), K.end(), v);
					if (loc == K.end()) {
						K.push_back(v);
						loc = K.end() - 1;
					}
					auto pos = std::distance(K.begin(), loc);

					symbols.emplace_back(tokens::CONSTANT, std::string(&s[0]), line, pos);
				} else {
					integer i = std::strtol(&s[0], nullptr, 10);
					
					value v(i);
					auto loc = std::find(K.begin(), K.end(), v);
					if (loc == K.end()) {
						K.push_back(v);
						loc = K.end() - 1;
					}
					auto pos = std::distance(K.begin(), loc);

					symbols.emplace_back(tokens::CONSTANT, std::string(&s[0]), line, pos);
				}			
			} else {
				throw std::runtime_error(std::to_string(line) + ": unexpected symbol: " + std::to_string((int)*p));
			}
		}

		symbols.emplace_back(tokens::RBRACE, "}", line+1);
				
		token_result res;
		res.symbols = symbols;
		res.constants = K;
		return res;
	}
}
