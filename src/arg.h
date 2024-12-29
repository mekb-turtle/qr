#ifndef ARG_H
#define ARG_H
#include <stdint.h>
#include <stdbool.h>
#include "qr.h"
#include "output.h"

#define PARSE_FUNC(name, type, min, max, signed) \
	bool parse_##name(const char *str, type *out, char **endptr);

#include "parse_int.h"

#undef PARSE_FUNC

bool parse_output_format(const char *str, enum output_format *format);

bool parse_color(const char *str, enum output_format format, struct color *color);
bool parse_color_fallback(const char *str, enum output_format format, struct color *color, struct color fallback);

bool parse_ecl(const char *str, enum qr_ecl *ecl);
bool parse_encoding(const char *str, enum qr_encoding *encoding);
#endif // ARG_H
