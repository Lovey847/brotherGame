// Testing utilities

#ifndef TEST_H
#define TEST_H

#include <csignal>
#include <cstdlib>
#include <cstring>

#define TEST_ASSERT(cond) (std::raise(SGABRT);}

// Return line number failed test is on
#define TEST(cond) {if (!(cond)) {return __LINE__;}}

#define TEST_EQUALS(exp, val) TEST((exp) == (val))
#define TEST_NEQUALS(exp, val) TEST((exp) != (val))
#define TEST_LESS(exp, val) TEST((exp) < (val))
#define TEST_LESSEQ(exp, val) TEST((exp) <= (val))
#define TEST_GTR(exp, val) TEST((exp) > (val))
#define TEST_GTREQ(exp, val) TEST((exp) >= (val))
#define TEST_TRUE(exp) TEST_NEQUALS(exp, 0)
#define TEST_FALSE(exp) TEST_EQUALS(exp, 0)
#define TEST_ZERO(exp) TEST_FALSE(exp)

#define TEST_MEM(addr, addr2, len) TEST_ZERO(memcmp((addr), (addr2), (len)))

#endif //TEST_H
