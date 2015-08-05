#include "value.h"
#include "exception.h"
#include "state.h"
#include "function.h"

#include <iostream>
#include <cstdio>
#include <cmath>
#include <stdexcept>

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

			case BOOLEAN:
				ra = a.to_boolean();
				rb = b;
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

			case BOOLEAN:
				ra = a.to_boolean();
				rb = b;
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
				
			case BOOLEAN:
				ra = a.to_boolean();
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
			
		case BOOLEAN:
			switch (b.get_type()) {
				case NUMBER:
				case INTEGER:
				case BYTE:
				case BOOLEAN:
				case STRING:
					ra = a;
					rb = b.to_boolean();
					break;
					
				default:
					throw invalid_binop_error(BOOLEAN, b.get_type());
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
				
			case BOOLEAN:
				ra = a.to_boolean();
				rb = b;
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

		case LIST:
			switch (b.get_type()) {
			case LIST:
				ra = a;
				rb = b;
				break;

			default:
				throw invalid_binop_error(LIST, b.get_type());
			}
			break;
			
		default:
			throw invalid_binop_error(a.get_type(), b.get_type());
		}
	}

	value::value()
		: type(NIL) {}
		
	value::value(const value &o)
		: type(o.type), data(o.data)
	{
		if (o.type == STRING)
			u_string = o.u_string;
	}
		
	value &value::operator=(const value &o) {
		type = o.type;
		data = o.data;

		if (type == STRING)
			u_string = o.u_string;
		
		return *this;
	}
		
	value::value(number n)
		: type(NUMBER), data((size_t)n) {}
		
	value::value(integer i)
		: type(INTEGER), data((size_t)i) {}
		
	value::value(byte b)
		: type(BYTE), data((size_t)b) {}
		
	value::value(boolean b)
		: type(BOOLEAN), data((size_t)b) {}
		
	value::value(string s)
		: type(STRING), u_string(s) {}
		
	value::value(const char *s)
		: value(string(s)) {}
		
	value::value(list *l)
		: type(LIST), data((size_t)l) {}
		
	value::value(map *m)
		: type(MAP), data((size_t)m) {}
		
	value::value(function *f)
		: type(FUNCTION), data((size_t)f) {}
		
	value::value(nfunction f)
		: type(NFUNCTION), data((size_t)f) {}
		
	value::~value() {
		// delete map, list or function if it is the last reference?
	}
	
	type value::get_type() const {
		return type;
	}
	
	std::size_t value::get_data() const {
		return data;
	}

	std::size_t value::hash() const {
		if (type == exo::STRING)
			return std::hash<std::string>()(u_string);
		else
			return std::hash<size_t>()((size_t(type)<<((sizeof(size_t)-1)*8)) | data);
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
		case LIST:
		case MAP:
		case NFUNCTION:
		case FUNCTION:
			return true;

		case STRING:
			return !u_string.empty();

		case NUMBER:
			return u_num != 0.0;

		case INTEGER:
			return u_int != 0;

		case BYTE:
			return u_byte != 0;
			
		case BOOLEAN:
			return u_bool;
			
		default:
			return false;
		}
	}
	
	string value::to_string() const {
		switch (type) {
		case STRING:
			return u_string;
			
		default:
			return repr();
		}
	}

	string value::repr() const {
		switch (type) {
		case NUMBER:
			return std::to_string(u_num);
			
		case INTEGER: 
			return std::to_string(u_int);
			
		case BYTE:
			{
				char s[2];
				s[0] = u_byte;
				s[1] = '\0';
				return std::string(s);
			}
			
		case BOOLEAN:
			return u_bool ? "true" : "false";
			
		case LIST:
			{
				std::string s;
				s += "[";
				bool first = true;
				for (const value &v : *u_list) {
					if (!first)
						s = s + ", ";
					else
						first = false;
					s = s + v.repr();
				}
				s += "]";
				return s;
			}
		
		case MAP:
			{
				std::string s;
				s += "{";
				bool first = true;
				for (const auto &v : *u_map) {
					if (!first)
						s = s + ", ";
					else
						first = false;
					s = s + v.first.repr() + ": " + v.second.repr();
				}
				s += "}";
				return s;
			}
			
		case STRING:
			return "\"" + u_string + "\"";
			
		case FUNCTION:
			{
				char c[20];
				snprintf(c, 20, "0x%p", (void *)u_func);
				return std::string(c);
			}
			
		case NFUNCTION:
			return "native function";
			
		default:
			return "nil";
		}
	}
	
	int value::call(state *s, int args, int rets) {
		if (type != NFUNCTION && type != FUNCTION)
			throw invalid_call_error(type);
		
		args = args < 0 ? s->stack.frame_size() : args;
		s->stack.push_frame(args);
		
		int act_r;
		if (type == NFUNCTION) {
			act_r = u_nfunc(s, args);
		} else {
			act_r = u_func->call(s);
		}
		
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
		value ra, rb;
		try {
			promote(*this, o, ra, rb);
		} catch (invalid_binop_error &err) {
			throw std::runtime_error(std::string("attempt to compare ") + type_name(type) + " and " + type_name(o.type));
		}
	
		switch (ra.type) {
		case NUMBER:
			return ra.u_num < rb.u_num;
		
		case INTEGER:
			return ra.u_int < rb.u_int;
		
		case BYTE:
			return ra.u_byte < rb.u_byte;
		
		case STRING:
			return ra.u_string < rb.u_string;
			
		default:
			return false;
		}
	}
	
	bool value::operator<=(const value &o) const {
		return ((*this < o)||(*this == o));
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
				return string(&u_string[o.to_integer()], 1);
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
			throw invalid_length_error(type); 
		}	
	}
	
	value value::pow(const value &o) const {
		value a, b;
		promote(*this, o, a, b);
		
		if (a.get_type() == NIL || b.get_type() == NIL)
			throw invalid_binop_error(a.get_type(), b.get_type());
			
		switch (a.get_type()) {
		case NUMBER:
			return std::pow(a.u_num, b.u_num);
		case INTEGER:
			return std::pow(a.u_int, b.u_int);
		case BYTE:
			return std::pow(a.u_byte, b.u_byte);
		default:
			throw invalid_binop_error(a.get_type(), b.get_type());
		}
	}
	
	value value::concat(const value &o) const {
		if (type == NIL || o.type == NIL)
			throw invalid_concat_error();
		
		return exo::value(to_string() + o.to_string());
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
		case LIST:
			{
				list *l = new list();
				l->reserve(a.u_list->size() + b.u_list->size());
				l->insert(l->end(), a.u_list->begin(), a.u_list->end());
				l->insert(l->end(), b.u_list->begin(), b.u_list->end());
				return l;
			}
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

	value value::operator-() const {	
		switch (get_type()) {
		case NUMBER:
			return -u_num;
		case INTEGER:
			return -u_int;
		case BYTE:
			return -u_byte;
		default:
			throw invalid_unop_error(get_type());
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
	
	value value::operator%(const value &o) const {
		value a, b;
		promote(*this, o, a, b);
		
		if (a.get_type() == NIL || b.get_type() == NIL)
			throw invalid_binop_error(a.get_type(), b.get_type());
			
		switch (a.get_type()) {
		case NUMBER:
			return std::fmod(a.u_num, b.u_num);
		case INTEGER:
			return a.u_int%b.u_int;
		case BYTE:
			return a.u_byte%b.u_byte;
		default:
			throw invalid_binop_error(a.get_type(), b.get_type());
		}
	}
	
	value value::operator&(const value &o) const {
		value a, b;
		promote(*this, o, a, b);
		
		if (a.get_type() == NIL || b.get_type() == NIL)
			throw invalid_binop_error(a.get_type(), b.get_type());
			
		switch (a.get_type()) {
		case INTEGER:
			return a.u_int & b.u_int;
		case BYTE:
			return a.u_byte & b.u_byte;
		default:
			throw invalid_binop_error(a.get_type(), b.get_type());
		}
	}
	
	value value::operator|(const value &o) const {
		value a, b;
		promote(*this, o, a, b);
		
		if (a.get_type() == NIL || b.get_type() == NIL)
			throw invalid_binop_error(a.get_type(), b.get_type());
			
		switch (a.get_type()) {
		case INTEGER:
			return a.u_int | b.u_int;
		case BYTE:
			return a.u_byte | b.u_byte;
		default:
			throw invalid_binop_error(a.get_type(), b.get_type());
		}
	}
	
	value value::operator^(const value &o) const {
		value a, b;
		promote(*this, o, a, b);
		
		if (a.get_type() == NIL || b.get_type() == NIL)
			throw invalid_binop_error(a.get_type(), b.get_type());
			
		switch (a.get_type()) {
		case INTEGER:
			return a.u_int ^ b.u_int;
		case BYTE:
			return a.u_byte ^ b.u_byte;
		default:
			throw invalid_binop_error(a.get_type(), b.get_type());
		}
	}

	value value::operator<<(const value &o) const {
		value a, b;
		promote(*this, o, a, b);
		
		if (a.get_type() == NIL || b.get_type() == NIL)
			throw invalid_binop_error(a.get_type(), b.get_type());
			
		switch (a.get_type()) {
		case INTEGER:
			return a.u_int << b.u_int;
		case BYTE:
			return a.u_byte << b.u_byte;
		default:
			throw invalid_binop_error(a.get_type(), b.get_type());
		}
	}

	value value::operator>>(const value &o) const {
		value a, b;
		promote(*this, o, a, b);
		
		if (a.get_type() == NIL || b.get_type() == NIL)
			throw invalid_binop_error(a.get_type(), b.get_type());
			
		switch (a.get_type()) {
		case INTEGER:
			return a.u_int << b.u_int;
		case BYTE:
			return a.u_byte << b.u_byte;
		default:
			throw invalid_binop_error(a.get_type(), b.get_type());
		}
	}
	
	value value::operator~() const {
		switch (type) {
		case INTEGER:
			return ~u_int;
		case BYTE:
			return ~u_byte;
		default:
			throw invalid_unop_error(type);
		}
	}	
}
