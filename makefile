CC			= g++
CFLAGS		= -stdlib=libc++ -std=c++11 -Wall -Wextra -pedantic -g
LDFLAGS		= 
SOURCES		= src/main.cpp src/compiler.cpp src/exception.cpp src/function.cpp src/stack.cpp src/state.cpp src/token.cpp src/value.cpp
OBJECTS		= $(patsubst %.cpp, %.o, $(patsubst src/%, obj/%, ${SOURCES}))
EXECUTABLE	= bin/exo

all: $(EXECUTABLE)
	@echo "done!"
	
$(EXECUTABLE): bin obj $(OBJECTS)
	@echo "buiding $(EXECUTABLE)"
	@$(CC) $(LDFLAGS) $(OBJECTS) -o $@
	
obj/%.o : src/%.cpp
	@echo "compiling $<"
	@$(CC) $(CFLAGS) -c $< -o $@
	
obj:
	@mkdir obj
	
bin:
	@mkdir bin
	
clean:
	@echo "cleaning project"
	@rm -rf obj/
	@echo "done!"
