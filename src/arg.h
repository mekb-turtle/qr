#ifndef ARG_H
#define ARG_H
#include <stdint.h>
#include <stdbool.h>
#include "qr.h"
#include "output.h"
bool parse_ullong(const char *str, unsigned long long int *out, char **endptr);
bool parse_llong(const char *str, long long int *out, char **endptr);
bool parse_ulong(const char *str, unsigned long int *out, char **endptr);
bool parse_long(const char *str, long int *out, char **endptr);
bool parse_uint(const char *str, unsigned int *out, char **endptr);
bool parse_int(const char *str, int *out, char **endptr);

bool parse_output_format(const char *str, enum output_format *format);

bool parse_color(const char *str, enum output_format format, struct color *color);
bool parse_color_fallback(const char *str, enum output_format format, struct color *color, struct color_rgb fallback_rgb, uint8_t fallback_ansi);

bool parse_ecl(const char *str, enum qr_ecl *ecl);
bool parse_encoding(const char *str, enum qr_encoding *encoding);
#endif // ARG_H
