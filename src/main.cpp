#include <iostream>

#include "token.h"

int main() {
	try {
		std::vector<exo::symbol> symbols = exo::tokenise("global test = 10");
		
		for (auto &symbol : symbols) {
			std::cout << "(" << symbol.tk << ") " << symbol.str << std::endl; 
		}
	} catch (std::exception &e) {
		std::cout << "error: " << e.what() << std::endl;
	}

	return 0;
}