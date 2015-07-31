#include <iostream>
#include <string>
#include <fstream>

#include "token.h"
#include "compiler.h"
#include "state.h"

int print(exo::state *E, int args) {
	for (int i=0; i<args; ++i) {
		if (i) 
			std::cout << "\t";
		exo::value v = E->get(i);
		std::cout << v.to_string();
	}
	std::cout << std::endl;
	
	return 0;
}

int getline(exo::state *E, int args) {
	if (args)
		std::cout << E->get(0).to_string() << " ";
	std::string s;
	std::getline(std::cin, s);
	E->push(s);
	return 1;
}

int to_integer(exo::state *E, int args) {
	E->push(E->get(0).to_integer());
	return 1;
}

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
	
		// tokenise source
		exo::token_result res = exo::tokenise(src);
		
		// print symbols
		/*for (auto &symbol : res.symbols) {
			std::cout << "(" << symbol.tk << ") " << symbol.str << " [" << symbol.k << "]" << std::endl; 
		}
		std::cout << std::endl;*/
		
		// compile tokens
		std::vector<exo::symbol>::iterator start = std::begin(res.symbols);
		exo::compiler c(res.constants, start, std::end(res.symbols));
		exo::function *func = c.compile();
		//std::cout << std::endl;

		// print constants
		/*std::cout << "K: " << std::endl;
		for (exo::value &v : res.constants) {
			std::cout << exo::type_name(v.get_type()) << ": " << v.to_string() << std::endl;
		}
		std::cout << std::endl;*/
		
		// create state and register global definitions
		exo::state E;
		E.set_global("print", print);
		E.set_global("getline", getline);
		E.set_global("int", to_integer);
		
		// run script
		//std::cout << "output: " << std::endl;
		func->call(&E);
	} catch (std::exception &e) {
		std::cout << "error: " << e.what() << std::endl;
	}

	return 0;
}
