#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <string>

namespace exo {
	class value;
	class state;

	typedef double								number;
	typedef std::int32_t 						integer;
	typedef std::int8_t							byte;
	typedef bool								boolean;
	typedef std::string							string;
	typedef std::vector<value> 					list;
	typedef std::unordered_map<value, value> 	map;
	
	typedef int (*nfunction)(state *, int);
	
	enum type : byte {
		NIL,
		NUMBER,
		INTEGER,
		BYTE,
		BOOLEAN,
		STRING,
		LIST,
		MAP,
		NFUNCTION,
		FUNCTION
	};
	
	inline string type_name(type t) {
		switch (t) {
			case NUMBER:	return "number";
			case INTEGER:	return "integer";
			case BYTE:		return "byte";
			case BOOLEAN:	return "boolean";
			case STRING:	return "string";
			case LIST:		return "list";
			case MAP:		return "map";
			case NFUNCTION:	return "nfunction";
			case FUNCTION:	return "function";
			
			case NIL:
			default:		return "nil";
		};
	}
}
