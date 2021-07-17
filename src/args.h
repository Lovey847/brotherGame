#ifndef ARGS_H
#define ARGS_H

#include "types.h"
#include "mem.h"
#include "str.h"

// Command line argument
struct args_arg_t {
	const char *name;
	const char *value;
	str_hash_t hash;
	u32 index;
};

// Command line arguments
class args_t {
private:
	mem_t &m_m;
	args_arg_t *m_args;
	
	char **m_argv;
	int m_argc;

public:
	args_t(int argc, char **argv, mem_t &m);
	~args_t();

	// Returns false if argument not found
	ubool check(str_hash_t arg) const;

	// Returns NULL if either:
	//   Argument not found
	//   Argument does not have associated value
	const char *val(str_hash_t arg) const;

	// Returns def if either:
	//   Argument not found
	//   Argument does not have associated value
	FINLINE const char *valDef(str_hash_t arg, const char *def) const {
		const char *ret = val(arg);
		return ret ? ret : def;
	}

	// More arguments then just program name
	FINLINE ubool active() const {return m_argc > 1;}
};

#endif //ARGS_H
