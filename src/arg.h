#ifndef ARG_H
#define ARG_H
#include <stdint.h>
#include <stdbool.h>
bool parse_ll(const char *str, long long int *out, char **endptr);
bool parse_ull(const char *str, unsigned long long int *out, char **endptr);
#endif // ARG_H
