#pragma once

#include "type.h"

namespace exo {
	class state;
	class function;

	class value {
	private:
		exo::type 	type;
		
		union {
			std::size_t data;
		
			number 		u_num;
			integer		u_int;
			byte		u_byte;
			boolean		u_bool;
			string		u_string;
			list		*u_list;
			map			*u_map;
			function	*u_func;
			nfunction	u_nfunc;
		};
		
	public:
		value();
		value(const value &);
		value &operator=(const value &);
		
		value(number);
		value(integer);
		value(byte);
		value(boolean);
		value(string);
		value(const char *);
		value(list *);
		value(map *);
		value(function *);
		value(nfunction);
	
		~value();
		
		exo::type get_type() const;
		std::size_t get_data() const;
		std::size_t hash() const;
		
		number 	to_number() const;
		integer to_integer() const;
		byte 	to_byte() const;
		boolean to_boolean() const;
		string 	to_string() const;
		
		int call(state *, int, int);
		
		value get(const value &) const;
		void set(const value &, const value &);
		
		integer size() const;
		value pow(const value &) const;
		value concat(const value &) const;
		
		value operator+(const value &) const;
		value operator-(const value &) const;
		value operator*(const value &) const;
		value operator/(const value &) const;
		value operator%(const value &) const;
		value operator&(const value &) const;
		value operator|(const value &) const;
		value operator^(const value &) const;
		value operator~() const;
		
		bool operator==(const value &) const;
		bool operator<(const value &) const;
		bool operator<=(const value &) const;
	};
	
	
	/*
		binary operand promotion
	*/
	void promote(const value &a, const value &b, value &ra, value &rb);
}

/*
	value hash specialization
*/
namespace std {
	template <> struct hash<exo::value> {
		size_t operator()(const exo::value &o) const {
			return o.hash();
		}
	};
}