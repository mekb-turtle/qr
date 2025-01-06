#ifndef TEST_H
#define TEST_H
#include <stdio.h>
#include <inttypes.h>
#define STRINGIFY(x) #x
#define TO_STRING(x) STRINGIFY(x)
#define LOC (__FILE__ ":" TO_STRING(__LINE__) ": ")

// number format specifiers
#define NUM(x) ((size_t) (x))

// success messages
#define SUCCESS(x, ...) printf("%sSuccess: " x "\n", LOC __VA_OPT__(, ) __VA_ARGS__)
#define EXPECTED_FAIL(x, ...) printf("%sExpected failure: " x "\n", LOC __VA_OPT__(, ) __VA_ARGS__)

// failure messages
#define FAIL(x, ...) printf("%sFail: " x "\n", LOC __VA_OPT__(, ) __VA_ARGS__)
#define FAIL_VAL(x, y, format, ...) printf("%sFail: " x ", expected" format "\n", LOC __VA_OPT__(, ) __VA_ARGS__, NUM(y))
#define UNEXPECTED_PASS(x, ...) printf("%sUnexpected pass: " x "\n", LOC __VA_OPT__(, ) __VA_ARGS__)
#define UNEXPECTED_PASS_ACTUAL(x, y, format, ...) printf("%sUnexpected pass: " x ", got " format "\n", LOC __VA_OPT__(, ) __VA_ARGS__, NUM(y))

// prints error if operator does not hold true
// cast to size_t to avoid format warnings
#define ASSERT(actual, op, expected, format, failure)                              \
	{                                                                              \
		if ((actual) op(expected)) {                                               \
			printf("%sAssertion passed: ", LOC);                                   \
			printf(format " %s " format " ... ", NUM(actual), #op, NUM(expected)); \
			printf("%s %s %s\n", #actual, #op, #expected);                         \
		} else {                                                                   \
			printf("%sAssertion failed: ", LOC);                                   \
			printf(format " %s " format " ... ", NUM(actual), #op, NUM(expected)); \
			printf("%s %s %s\n", #actual, #op, #expected);                         \
			failure;                                                               \
		}                                                                          \
	}

// custom format specifiers
#define FMT_PRE "%"
#define FMT_CHAR "#"
#define FMT_DEC FMT_PRE PRIdPTR
#define FMT_INT FMT_PRE PRIiPTR
#define FMT_UINT FMT_PRE PRIuPTR
#define FMT_HEX FMT_PRE FMT_CHAR PRIxPTR
#define FMT_HEX_UPPER FMT_PRE FMT_CHAR PRIXPTR
#define FMT_OCTAL FMT_PRE FMT_CHAR PRIoPTR

#define ASSERT_TRUE(actual, failure) ASSERT(actual, ==, true, FMT_INT, failure)
#define ASSERT_FALSE(actual, failure) ASSERT(actual, ==, false, FMT_INT, failure)
#endif // TEST_H
