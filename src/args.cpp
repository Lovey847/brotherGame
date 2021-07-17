#include "types.h"
#include "mem.h"
#include "str.h"
#include "args.h"

#include <cstring>

// Go through string, replace first equals with NULL and return pointer to
// character after equals
// Return NULL if no equals found
static char *findValue(char *str) {
	char *p;
	for (p = str; *p != '='; ++p) {
		if (*p == 0) return NULL;
	}

	*p = 0;
	return p+1;
}

args_t::args_t(int argc, char **argv, mem_t &m) : m_m(m) {
	m_argc = argc;
	m_argv = argv;
	
	// Allocate null-terminated list of arguments
	// argv[0] == program name

	// If there's no arguments, don't do anything
	if (active()) {
		m_args = (args_arg_t*)m.alloc(sizeof(args_arg_t)*argc - sizeof(args_arg_t));

		// Load arguments in
		args_arg_t *arg = m_args;
		while (--argc) {
			arg->name = m_argv[argc];
			arg->value = findValue(m_argv[argc]);
			arg->hash = str_hashR(m_argv[argc]);
			arg->index = arg-m_args;

			++arg;
		}

		// Add NULL terminator
		arg->name = NULL;
	}
}

args_t::~args_t() {
	if (active()) m_m.free(m_args);
}

ubool args_t::check(str_hash_t arg) const {
	if (!active()) return false;

	for (const args_arg_t *i = m_args; i->name; ++i) {
		if (i->hash == arg) return true;
	}

	return false;
}

const char *args_t::val(str_hash_t arg) const {
	if (!active()) return NULL;
	
	for (const args_arg_t *i = m_args; i->name; ++i) {
		if (i->hash == arg) return i->value;
	}

	return NULL;
}
