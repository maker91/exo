release:
	g++ -Wall -Wextra -pedantic -std=c++11 -Ofast -o"bin/exo" src/*.cpp
	
debug:
	g++ -Wall -Wextra -pedantic -std=c++11 -g -o"bin/exo" src/*.cpp
	
profile:
	g++ -Wall -Wextra -pedantic -std=c++11 -g -pg -o"bin/exo" src/*.cpp