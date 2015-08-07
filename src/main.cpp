#include <iostream>
#include <string>
#include <fstream>

#include "token.h"
#include "compiler.h"
#include "state.h"
#include "builtins.h"


int main(int argc, char **argv) {
	if (argc < 2) {
		std::cout << "usage: exo file" << std::endl;
		return 0;
	}
	
	try {
		std::string src;
		if (std::string(argv[1]) == "-c") {
			src = std::string(argv[2]);
		} else {
			std::ifstream ifs(argv[1]);
			src = std::string((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
		}

		// create state and register builtins
		exo::state E;
		exo::register_builtins(&E);
	
		// tokenise source
		exo::token_result res = exo::tokenise(src);
		
		// print symbols
		/*
		for (auto &symbol : res.symbols) {
			std::cout << "(" << symbol.tk << ") " << symbol.str << " [" << symbol.k << "]" << std::endl; 
		}
		std::cout << std::endl;
		*/
		
		// compile tokens
		exo::compiler c(std::begin(res.symbols), std::end(res.symbols), 
			res.constants, E.builtins);
		exo::function *func = c.compile();
		//std::cout << std::endl;

		// print constants
		/*
		std::cout << "K: " << std::endl;
		int i = 0;
		for (exo::value &v : res.constants) {
			std::cout << "[" << i << "]\t" << exo::type_name(v.get_type()) << ": " << v.to_string() << std::endl;
			i++;
		}
		std::cout << std::endl;
		*/
		
		// run script
		//std::cout << "output: " << std::endl;
		func->call(&E, 0);
	} catch (std::exception &e) {
		std::cout << "error: " << e.what() << std::endl;
	}

	return 0;
}
