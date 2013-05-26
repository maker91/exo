#include "value.h"
#include "exception.h"
#include "state.h"

#include <iostream>

namespace exo {
	/*
		attempt to promote arg a or b to the same type
	*/
	void promote(const value &a, const value &b, value &ra, value &rb) {
		switch (a.get_type()) {
		case NUMBER:
			switch (b.get_type()) {
			case NUMBER:
			case INTEGER:
			case BYTE:
				ra = a;
				rb = b.to_number();
				break;
			
			case STRING:
				ra = a;
				try {
					rb = b.to_number();
				} catch (conversion_error &) {
					throw invalid_binop_error(NUMBER, STRING);
				}
				break;
				
			default:
				throw invalid_binop_error(NUMBER, b.get_type());	
			}
			break;
			
		case INTEGER:
			switch (b.get_type()) {
			case NUMBER:
				ra = a.to_number();
				rb = b;
				break;
				
			case INTEGER:
			case BYTE:
				ra = a;
				rb = b.to_integer();
				break;			
			
			case STRING:
				ra = a.to_number();
				try {
					rb = b.to_number();
				} catch (conversion_error &) {
					throw invalid_binop_error(INTEGER, STRING);
				}
				break;
				
			default:
				throw invalid_binop_error(INTEGER, b.get_type());
			}
			break;
			
		case BYTE:
			switch (b.get_type()) {
			case NUMBER:
				ra = a.to_number();
				rb = b;
				break;
			
			case INTEGER:
				ra = a.to_integer();
				rb = b;
				break;
				
			case BYTE:
				ra = a;
				rb = b;
				break;
			
			case STRING:
				ra = a.to_number();
				try {
					rb = b.to_number();
				} catch (conversion_error &) {
					throw invalid_binop_error(BYTE, STRING);
				}
				break;
				
			default:
				throw invalid_binop_error(BYTE, b.get_type());
			}
			break;
			
		case STRING:
			switch (b.get_type()) {
			case NUMBER:				
			case INTEGER:				
			case BYTE:
				try {
					ra = a.to_number();
				} catch (conversion_error &) {
					throw invalid_binop_error(STRING, b.get_type());
				}
				rb = b.to_number();
				break;
				
			case STRING:
				try {
					ra = a.to_number();
				} catch (conversion_error &) {
					throw invalid_binop_error(STRING, STRING);
				}
				
				try {
					rb = b.to_number();
				} catch (conversion_error &) {
					throw invalid_binop_error(STRING, STRING);
				}
				break;
				
			default:
				throw invalid_binop_error(STRING, b.get_type());
			}
			break;
			
		default:
			throw invalid_binop_error(a.get_type(), b.get_type());
		}
	}

	value::value()
		: type(NIL) {}
		
	value::value(const value &o)
		: type(o.type), data(o.data) {}
		
	value &value::operator=(const value &o) {
		type = o.type;
		data = o.data;
		
		return *this;
	}
		
	value::value(number n)
		: type(NUMBER), u_num(n) {}
		
	value::value(integer i)
		: type(INTEGER), u_int(i) {}
		
	value::value(byte b)
		: type(BYTE), u_byte(b) {}
		
	value::value(boolean b)
		: type(BOOLEAN), u_bool(b) {}
		
	value::value(string s)
		: type(STRING), u_string(s) {}
		
	value::value(const char *s)
		: value(string(s)) {}
		
	value::value(list *l)
		: type(LIST), u_list(l) {}
		
	value::value(map *m)
		: type(MAP), u_map(m) {}
		
	value::value(nfunction f)
		: type(NFUNCTION), u_nfunc(f) {}
		
	value::~value() {
		// delete map or list if it is the last reference?
	}
	
	type value::get_type() const {
		return type;
	}
	
	number value::to_number() const {
		switch (type) {
		case NUMBER:
			return u_num;
			
		case INTEGER:
			return (number)u_int;
		
		case BYTE:
			return (number)u_byte;
			
		case STRING:
		 	try {
		 		std::size_t p;
		 		number d = std::stod(u_string, &p);
		 		
		 		if (p < u_string.size())
		 			throw conversion_error(type, NUMBER);
		 			
		 		return d;
		 	} catch (std::exception &) {
		 		throw conversion_error(type, NUMBER);
		 	}
		 	
		 default:
		 	throw conversion_error(type, NUMBER);
		}
	}
	
	integer value::to_integer() const {
		switch (type) {
		case NUMBER:
			return (integer)u_num;
			
		case INTEGER:
			return u_int;
		
		case BYTE:
			return (integer)u_byte;
			
		case STRING:
		 	try {
		 		std::size_t p;
		 		number d = std::stoi(u_string, &p);
		 		
		 		if (p < u_string.size())
		 			throw conversion_error(type, NUMBER);
		 			
		 		return d;
		 	} catch (std::exception &) {
		 		throw conversion_error(type, INTEGER);
		 	}
		 	
		 default:
		 	throw conversion_error(type, INTEGER);
		}
	}
	
	byte value::to_byte() const {
		switch (type) {
		case NUMBER:
			return (byte)u_num;
			
		case INTEGER:
			return (byte)u_int;
		
		case BYTE:
			return u_byte;
			
		case STRING:
		 	try {
		 		std::size_t p;
		 		number d = (byte)std::stoi(u_string, &p);
		 		
		 		if (p < u_string.size())
		 			throw conversion_error(type, NUMBER);
		 			
		 		return d;
		 	} catch (std::exception &) {
		 		throw conversion_error(type, BYTE);
		 	}
		 	
		 default:
		 	throw conversion_error(type, BYTE);
		}
	}
	
	boolean value::to_boolean() const {
		switch (type) {
		case NUMBER:		
		case INTEGER:					
		case BYTE:		
		case LIST:
		case MAP:			
		case STRING:
			return true;
			
		case BOOLEAN:
			return u_bool;
			
		default:
			return false;
		}
	}
	
	string value::to_string() const {
		switch (type) {
		case NUMBER:
			return std::to_string(u_num);
			
		case INTEGER: 
			return std::to_string(u_int);
			
		case BYTE:
			return std::to_string(u_byte);	
			
		case BOOLEAN:
			return u_bool ? "true" : "false";
			
		case LIST:			
		case MAP:
			return type_name(type);
			
		case STRING:
			return u_string;
			
		default:
			return "nil";
		}
	}
	
	int value::call(state *s, int args, int rets) {
		if (type != NFUNCTION)
			throw invalid_call_error(type);
		
		args = args < 0 ? s->stack.frame_size() : args;
		s->stack.push_frame(args);
		
		int act_r = u_nfunc(s);
		
		act_r = act_r < 0 ? s->stack.frame_size() - args : act_r;
		rets = rets < 0 ? act_r : rets;
		s->stack.pop_frame(act_r, rets);
		
		return act_r;
	}
	
	bool value::operator==(const value &o) const {
		if (type != o.type)
			return false;
			
		switch (type) {
		case STRING:
			return u_string == o.u_string;
			
		default:
			return data == o.data;	
		}	
	}
	
	bool value::operator<(const value &o) const {
		switch (type) {
		case STRING:
			return u_string < o.u_string;
			
		default:
			return data < o.data;
		}	
	}
	
	bool value::operator<=(const value &o) const {
		return (*this < o)||(*this == o);
	}
	
	value value::get(const value &o) const {
		switch (type) {
		case LIST:
			switch (o.type) {
			case INTEGER:		
			case BYTE:
				if ((unsigned)o.to_integer() >= u_list->size())
					return value();
				
				return (*u_list)[(unsigned)o.to_integer()];
				break;
				
			default:
				throw invalid_index_error(LIST, o.type);
			}
			break;
			
		case MAP:
			switch (o.type) {
			case NIL:
				throw invalid_index_error(MAP, NIL);
				
			default:
				return (*u_map)[o];
			}
			break;
			
		case STRING:
			switch (o.type) {
			case INTEGER:		
			case BYTE:
				return u_string[o.to_integer()];
				break;
				
			default:
				throw invalid_index_error(LIST, o.type);
			}
			break;
			
		default:
			throw invalid_index_error(type, o.type);
		}
	}
	
	void value::set(const value &k, const value &v) {
		switch (type) {
		case LIST:
			switch (k.type) {
			case INTEGER:		
			case BYTE:
				if ((unsigned)k.to_integer() >= u_list->size()) {
					u_list->resize(k.to_integer()+1, value());
				}
			
				(*u_list)[(unsigned)k.to_integer()] = v;
				break;
				
			default:
				throw invalid_index_error(LIST, k.type);
			}
			break;
			
		case MAP:
			switch (k.type) {
			case NIL:
				throw invalid_index_error(MAP, NIL);
				
			default:
				(*u_map)[k] = v;
			}
			break;
			
		default:
			throw invalid_index_error(type, k.type);
		}
	}
	
	integer value::size() const {
		switch (type) {
		case LIST:
			return u_list->size();
			
		case MAP:
			return u_map->size();
			
		case STRING:
			return u_string.size();
		
		default:
			throw error(std::string("attempt to get size of ") + type_name(type)); 
		}	
	}
	
	value value::operator+(const value &o) const {
		value a, b;
		promote(*this, o, a, b);
		
		if (a.get_type() == NIL || b.get_type() == NIL)
			throw invalid_binop_error(a.get_type(), b.get_type());
			
		switch (a.get_type()) {
		case NUMBER:
			return a.u_num + b.u_num;
		case INTEGER:
			return a.u_int + b.u_int;
		case BYTE:
			return a.u_byte + b.u_byte;
		default:
			throw invalid_binop_error(a.get_type(), b.get_type());
		}
	}
	
	value value::operator-(const value &o) const {
		value a, b;
		promote(*this, o, a, b);
		
		if (a.get_type() == NIL || b.get_type() == NIL)
			throw invalid_binop_error(a.get_type(), b.get_type());
			
		switch (a.get_type()) {
		case NUMBER:
			return a.u_num - b.u_num;
		case INTEGER:
			return a.u_int - b.u_int;
		case BYTE:
			return a.u_byte - b.u_byte;
		default:
			throw invalid_binop_error(a.get_type(), b.get_type());
		}
	}
	
	value value::operator*(const value &o) const {
		value a, b;
		promote(*this, o, a, b);
		
		if (a.get_type() == NIL || b.get_type() == NIL)
			throw invalid_binop_error(a.get_type(), b.get_type());
			
		switch (a.get_type()) {
		case NUMBER:
			return a.u_num * b.u_num;
		case INTEGER:
			return a.u_int * b.u_int;
		case BYTE:
			return a.u_byte * b.u_byte;
		default:
			throw invalid_binop_error(a.get_type(), b.get_type());
		}
	}
	
	value value::operator/(const value &o) const {
		value a, b;
		promote(*this, o, a, b);
		
		if (a.get_type() == NIL || b.get_type() == NIL)
			throw invalid_binop_error(a.get_type(), b.get_type());
			
		switch (a.get_type()) {
		case NUMBER:
			return a.u_num / b.u_num;
		case INTEGER:
			return a.u_int / b.u_int;
		case BYTE:
			return a.u_byte / b.u_byte;
		default:
			throw invalid_binop_error(a.get_type(), b.get_type());
		}
	}
}
