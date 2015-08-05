#include "builtins.h"
#include <iostream>

int print(exo::state *E, int args) {
    for (int i=0; i<args; ++i) {
        if (i) 
            std::cout << "\t";
        exo::value v = E->stack.get_in_frame(i);
        std::cout << v.to_string();
    }
    std::cout << std::endl;
    
    return 0;
}

namespace exo {
    void register_builtins(exo::state *E) {
        E->register_builtin("print", print);
    }
}