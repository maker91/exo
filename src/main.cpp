#include <iostream>
#include <string>
#include <fstream>

#include "token.h"
#include "compiler.h"
#include "state.h"

int main(int argc, char **argv) {
	if (argc < 2) {
		std::cout << "usage: exo file" << std::endl;
		return 0;
	}

	try {
		std::ifstream ifs(argv[1]);
		std::string src((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
	
		std::vector<exo::symbol> symbols = exo::tokenise(src);
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