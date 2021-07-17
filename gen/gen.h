/*
 * Main header for generators, contains helper functions for the generator and
 * the generator entry point declaration
 */
#ifndef GEN_H
#define GEN_H

#include "gen.h"

#include "types.h"

#include "mem.h"
#include "file.h"
#include "log.h"

// Throws log_except_t on error
// m: Memory pool provided by platform layer
// f: File system interface provided by platform layer
// txt: Text file name
// output: Output directory (contains slash after final directory name)
void gen_main(mem_t &m, file_system_t &f, const char *txt, const char *output);

#endif //GEN_H
