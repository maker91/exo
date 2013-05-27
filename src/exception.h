#pragma once

#include <exception>
#include "type.h"

namespace exo {
	/*
		base class for all Exo runtime errors
	*/
	class error : public std::exception {
	protected:
		std::string str;
	
	public:
		error(std::string) noexcept;
		virtual const char *what() const noexcept;
	};
	
	
	/*
		invalid conversion error
	*/
	class conversion_error : public error {
	public:
		conversion_error(type, type) noexcept;	
	};
	
	
	/*
		out of bounds error
	*/
	class out_of_bounds_error : public error {
	public:
		out_of_bounds_error(unsigned, unsigned) noexcept;
	};
	
	
	/*
		invalid operations
	*/
	class invalid_binop_error : public error {
	public:
		invalid_binop_error(type, type) noexcept;
	};
	
	class invalid_call_error : public error {
	public:
		invalid_call_error(type) noexcept;
	};
	
	class invalid_comparison_error : public error {
	public:
		invalid_comparison_error(type, type) noexcept;
	};
	
	
	/*
		invalid index error
	*/
	class invalid_index_error : public error {
	public:
		invalid_index_error(type, type) noexcept;
	};
}
