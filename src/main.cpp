#include <iostream>

#include "token.h"
#include "compiler.h"

int main() {
	try {
		std::vector<exo::symbol> symbols = exo::tokenise("global test = 10");
		
		for (auto &symbol : symbols) {
			std::cout << "(" << symbol.tk << ") " << symbol.str << std::endl; 
		}
		std::cout << std::endl;
		
		exo::compiler c(symbols);
		exo::function func = c.compile();
	} catch (std::exception &e) {
		std::cout << "error: " << e.what() << std::endl;
	}

	return 0;
}