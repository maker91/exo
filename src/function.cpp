#include <iostream>

#include "function.h"

// register access
#define GET_RA(E, I)		(E->registers[GET_A(I)])
#define GET_RB(E, I)		(E->registers[GET_B(I)])
#define GET_RC(E, I)		(E->registers[GET_C(I)])
#define GET_RBC(E, I)		(E->registers[GET_B(I) - GET_C(I)])
	
#define SET_R(E, N, V)		(E->registers[N] = V)
#define SET_RA(E, I, V)		(void(E->registers[GET_A(I)] = V))
	
// constant access	
#define GET_K(N)			(k_store[N])
#define GET_KB(I)			(GET_K(GET_B(I)))
#define GET_KC(I)			(GET_K(GET_C(I)))
#define GET_KBC(I)			(GET_K(GET_B(I) - GET_C(I)))
	
// combined register and constant access
#define GET_RKB(E, I)		(IS_BK(I) ? GET_KB(I) : GET_RB(E, I))
#define GET_RKC(E, I)		(IS_CK(I) ? GET_KC(I) : GET_RC(E, I))
#define GET_RKBC(E, I)		((IS_BK(I) || IS_CK(I)) ? GET_KBC(I) : GET_RBC(E, I))
	
// stack access	
#define GET_S(E, N)			(E->stack.get_in_frame(N))
#define GET_SA(E, I)		(GET_S(GET_A(I)))
#define GET_SB(E, I)		(GET_S(GET_B(I)))
#define GET_SC(E, I)		(GET_S(GET_C(I)))
#define GET_SBC(E, I)		(GET_S(GET_B(I) - GET_C(I)))
	
#define PUSH(E, V)			(E->stack.push(V))
#define POP(E)				(E->stack.pop())
	
// stack push from register and constant
#define PUSH_RA(E, I)		(PUSH(GET_RA(E, I)))
#define PUSH_RB(E, I)		(PUSH(GET_RB(E, I)))
#define PUSH_RC(E, I)		(PUSH(GET_RC(E, I)))
#define PUSH_RBC(E, I)		(PUSH(GET_RBC(E, I)))
	
#define PUSH_KB(I)			(PUSH(GET_KA(I)))
#define PUSH_KC(I)			(PUSH(GET_KB(I)))
#define PUSH_KBC(I)			(PUSH(GET_KC(I)))
	
#define PUSH_RKB(E, I)		(PUSH(GET_RKB(E, I)))
#define PUSH_RKC(E, I)		(PUSH(GET_RKC(E, I)))
#define PUSH_RKBC(E, I)		(PUSH(GET_RKBC(E, I)))
	
// combined stack, register and constant access (-1 = use stack)
#define SET_SRA(E, I, V)	((GET_A(I) == 0xFF) ? PUSH(E, V) : SET_RA(E, I, V))
#define GET_SRA(E, I)		((GET_A(I) == 0xFF) ? POP(E) : GET_RA(E, I))
#define GET_SRKB(E, I)		((GET_B(I) == 0xFF) ? POP(E) : GET_RKB(E, I))
#define GET_SRKC(E, I)		((GET_C(I) == 0xFF) ? POP(E) : GET_RKC(E, I))
#define GET_SRKBC(E, I)		((GET_B(I) == 0xFF) ? POP(E) : GET_RKBC(E, I))


namespace exo {

	function::function(int pstart, int pnum)
		: param_start(pstart), num_params(pnum)
	{
		i_store.push_back(MAKE_ABC(opcodes::RTN, 1, 0, 0, 0, 0));
	}
	
	function::function(int pstart, int pnum, const std::vector<instruction> &s)
		: i_store(s), param_start(pstart), num_params(pnum)
	{

	}
	
	function::function(int pstart, int pnum, const std::vector<instruction> &s, const std::vector<value> &k)
		: k_store(k), i_store(s), param_start(pstart), num_params(pnum)
	{

	}
	
	function::~function() {
		
	}
	
	int function::call(state *E, int args) {
		exo::instruction *pc = &i_store[0];
		exo::value one = exo::value(1);
		exo::value nil = exo::value();

		// params are stored on the stack - 
		// they need to be moved to registers based on param_start, num_params and the number of args
		// actually passed in (args)
		while (args > num_params) {
			POP(E);
			args--;
		}

		for (int i = 1; i<=num_params; ++i) {
			if (i > args)
				SET_R(E, param_start + num_params - i, nil);
			else
				SET_R(E, param_start + num_params - i, POP(E));
		}
		
		while (true) {	
			exo::instruction I = *pc;
			exo::opcodes::opcode OP = GET_OP(I);
			
			/*
			std::cout << std::endl;
			std::cout << pc << ": " << opcode_name(OP) << "\t" << GET_A(I) << " " << IS_BK(I) << " " << GET_B(I) << " " << IS_CK(I) << " " << GET_C(I);
			std::cout << "\t(" << GET_A(I) << " " << GET_T(I) << " " << (int)GET_Bx(I) << ")" << std::endl;
			*/

			exo::value b, c;
			switch (OP) {
			case opcodes::NOOP:
				break;

			case opcodes::LOAD:
				SET_SRA(E, I, GET_SRKBC(E, I));
				break;

			case opcodes::BUILTIN:
				b = GET_SRKB(E, I);
				SET_SRA(E, I, E->builtins[b.to_string()]);
				break;
				
			case opcodes::JMP:
				pc += (int)GET_Bx(I) - 1;
				break;
				
			case opcodes::TEST:
				if (GET_SRA(E, I).to_boolean() == (exo::boolean)GET_T(I)) {
					pc += (int)GET_Bx(I) - 1;
				}
				break;
				
			case opcodes::RTN:
				return GET_A(I)-1;
				
			case opcodes::CALL:
				b = GET_RB(E, I);
				b.call(E, GET_C(I)-1, GET_A(I)-1);
				break;
				
			case opcodes::EQL:
				c = GET_SRKC(E, I);
				b = GET_SRKB(E, I);
				SET_SRA(E, I, b == c);
				break;
				
			case opcodes::LT:
				c = GET_SRKC(E, I);
				b = GET_SRKB(E, I);
				SET_SRA(E, I, b < c);
				break;
				
			case opcodes::LE:
				c = GET_SRKC(E, I);
				b = GET_SRKB(E, I);
				SET_SRA(E, I, b <= c);
				break;
				
			case opcodes::AND:
				c = GET_SRKC(E, I);
				b = GET_SRKB(E, I);
				SET_SRA(E, I, b.to_boolean() && c.to_boolean());
				break;
				
			case opcodes::OR:
				c = GET_SRKC(E, I);
				b = GET_SRKB(E, I);
				SET_SRA(E, I, b.to_boolean() || c.to_boolean());
				break;
				
			case opcodes::NOT:
				SET_SRA(E, I, !GET_SRKB(E, I).to_boolean());
				break;
				
			case opcodes::BAND:
				c = GET_SRKC(E, I);
				b = GET_SRKB(E, I);
				SET_SRA(E, I, b & c);
				break;
				
			case opcodes::BOR:
				c = GET_SRKC(E, I);
				b = GET_SRKB(E, I);
				SET_SRA(E, I, b | c);
				break;
				
			case opcodes::BXOR:
				c = GET_SRKC(E, I);
				b = GET_SRKB(E, I);
				SET_SRA(E, I, b ^ c);
				break;
				
			case opcodes::BNOT:
				SET_SRA(E, I, ~GET_SRKB(E, I));
				break;

			case opcodes::LSHIFT:
				c = GET_SRKC(E, I);
				b = GET_SRKB(E, I);
				SET_SRA(E, I, b << c);
				break;

			case opcodes::RSHIFT:
				c = GET_SRKC(E, I);
				b = GET_SRKB(E, I);
				SET_SRA(E, I, b >> c);
				break;
				
			case opcodes::ADD: 	
				c = GET_SRKC(E, I);
				b = GET_SRKB(E, I);
				SET_SRA(E, I, b + c);
				break;
					
			case opcodes::SUB: 
				c = GET_SRKC(E, I);
				b = GET_SRKB(E, I);
				SET_SRA(E, I, b - c);
				break;
				
			case opcodes::MUL: 
				c = GET_SRKC(E, I);
				b = GET_SRKB(E, I);
				SET_SRA(E, I, b * c);
				break;
				
			case opcodes::DIV: 
				c = GET_SRKC(E, I);
				b = GET_SRKB(E, I);
				SET_SRA(E, I, b / c);
				break;
				
			case opcodes::POW: 
				c = GET_SRKC(E, I);
				b = GET_SRKB(E, I);
				SET_SRA(E, I, b.pow(c));
				break;
				
			case opcodes::MOD: 
				c = GET_SRKC(E, I);
				b = GET_SRKB(E, I);
				SET_SRA(E, I, b % c);
				break;

			case opcodes::UNM:
				SET_SRA(E, I, -GET_SRKB(E, I));
				break;

			case opcodes::INCR:
				SET_SRA(E, I, GET_SRA(E, I) + one);
				break;

			case opcodes::DECR:
				SET_SRA(E, I, GET_SRA(E, I) - one);
				break;
				
			case opcodes::NEWLIST:
				{
					auto l = new list;
					for (int i=GET_B(I); i>=1; --i) {
						l->push_back(GET_S(E, -i));
					}
					for (int i=GET_B(I); i>=1; --i) {
						POP(E);
					}
					SET_SRA(E, I, l);
				}
				break;
				
			case opcodes::NEWMAP:
				{
					auto m = new map;
					for (int i=2*GET_B(I); i>=2; i-=2) {
						(*m)[GET_S(E, -i)] = GET_S(E, -i+1);
					}
					SET_SRA(E, I, m);
				}
				break;
				
			case opcodes::SETITEM:
				c = GET_SRKC(E, I);
				b = GET_SRKB(E, I);
				GET_SRA(E, I).set(b, c);
				break;
				
			case opcodes::GETITEM:
				c = GET_SRKC(E, I);
				b = GET_SRKB(E, I);
				SET_SRA(E, I, b.get(c));
				break;
				
			case opcodes::LEN:
				SET_SRA(E, I, GET_SRKB(E, I).size());
				break;
				
			case opcodes::CONCAT:
				c = GET_SRKC(E, I);
				b = GET_SRKB(E, I);
				SET_SRA(E, I, b.concat(c));
				break;
				
			default:
				break;
			}
			
			pc++;
			
			//E->print_stack();
			//E->print_registers();
		}
	}
}
