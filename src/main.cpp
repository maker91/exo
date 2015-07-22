#include <iostream>
#include <string>
#include <fstream>

#include "token.h"
#include "compiler.h"
#include "state.h"

int print(exo::state *E) {
	exo::value v = E->get(0);
	std::cout << v.to_string() << std::endl;
	
	return 0;
}

int main(int argc, char **argv) {
	if (argc < 2) {
		std::cout << "usage: exo file" << std::endl;
		return 0;
	}
	
	try {
		// load exo script
		std::ifstream ifs(argv[1]);
		std::string src((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
	
		// tokenise source
		exo::token_result res = exo::tokenise(src);
		
		// print symbols
		for (auto &symbol : res.symbols) {
			std::cout << "(" << symbol.tk << ") " << symbol.str << " [" << symbol.k << "]" << std::endl; 
		}
		std::cout << std::endl;
		
		// print constants
		std::cout << "K: " << std::endl;
		for (exo::value &v : res.constants) {
			std::cout << exo::type_name(v.get_type()) << ": " << v.to_string() << std::endl;
		}
		std::cout << std::endl;
		
		// compile tokens
		exo::compiler c(res);
		exo::function func = c.compile();
		std::cout << std::endl;
		
		// create state and register print function
		exo::state E;
		E.set_global("print", print);
		
		// run script
		std::cout << "output: " << std::endl;
		func.call(&E);
	} catch (std::exception &e) {
		std::cout << "error: " << e.what() << std::endl;
	}

	return 0;
}
