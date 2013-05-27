#include "exception.h"
#include "type.h"

namespace exo {

	error::error(std::string s) noexcept
		: str(s) {}
		
	const char *error::what()  const noexcept {
		return str.c_str();
	}
	
	
	/*
		exo exceptions
	*/
	conversion_error::conversion_error(type a, type b) noexcept
		: error(std::string("attempt to convert ") + type_name(a) + " to " + type_name(b)) {}
		
	out_of_bounds_error::out_of_bounds_error(unsigned i, unsigned s) noexcept
		: error(std::string("index out of bounds (" + std::to_string(i) + ", " + std::to_string(s) + ")")) {}
		
	invalid_binop_error::invalid_binop_error(type a, type b) noexcept
		: error(std::string("attempt to perform binary operation on ") + type_name(a) + " and " + type_name(b)) {}
		
	invalid_unop_error::invalid_unop_error(type a) noexcept
		: error(std::string("attempt to perform unary operation on ") + type_name(a)) {}
		
	invalid_call_error::invalid_call_error(type a) noexcept
		: error(std::string("attempt to call a " + type_name(a) + " value")) {}
		
	invalid_comparison_error::invalid_comparison_error(type a, type b) noexcept
		: error(std::string("attempt to compare ") + type_name(a) + " with " + type_name(b)) {}
		
	invalid_length_error::invalid_length_error(type t) noexcept
		: error(std::string("attempt to get length of a ") + type_name(t) + " value") {}
		
	invalid_index_error::invalid_index_error(type c, type i) noexcept
		: error(std::string("attempt to index ") + type_name(c) + " with a " + type_name(i) + " value") {}
		
	invalid_concat_error::invalid_concat_error() noexcept
		: error(std::string("attempt to concat a nil value")) {}
}
