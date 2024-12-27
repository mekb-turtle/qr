#include "arg.h"
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <ctype.h>

typedef unsigned long long int ull;

// avoid reusing code
static bool parse_internal(const char *str, ull *out, char **endptr, bool is_signed) {
	if (!str || !*str) return false;

	int errno_ = errno;
	errno = 0;

	bool own_endptr = false;
	char *endptr_;
	if (!endptr) {
		// do own our logic if endptr is NULL
		endptr = &endptr_;
		own_endptr = true;
	}

	ull result = is_signed ? (ull) strtoll(str, endptr, 0) : strtoull(str, endptr, 0);

	if (errno) return false;
	errno = errno_;

	// skip whitespace
	while (*endptr) {
		if (!isspace(*endptr)) break;
		++endptr;
	}

	if (own_endptr && *endptr_) return false; // not all characters were consumed

	*out = result;
	return true;
}

bool parse_ull(const char *str, ull *out, char **endptr) {
	return parse_internal(str, out, endptr, true);
}

bool parse_ll(const char *str, long long int *out, char **endptr) {
	ull result;
	if (!parse_internal(str, &result, endptr, false)) return false;

	if (result > LLONG_MAX) return false;
	*out = result;
	return true;
}
