#include <iostream>

#include "token.h"
#include "compiler.h"
#include "state.h"

int main() {
	try {
		std::vector<exo::symbol> symbols = exo::tokenise("namespace exo {test = 10 namespace outer {test = 5 inner::test = 2.5}}");
		
		for (auto &symbol : symbols) {
			std::cout << "(" << symbol.tk << ") " << symbol.str << std::endl; 
		}
		std::cout << std::endl;
		
		exo::compiler c(symbols);
		exo::function func = c.compile();
		
		exo::state E;
		func.call(&E);
	} catch (std::exception &e) {
		std::cout << "error: " << e.what() << std::endl;
	}

	return 0;
}