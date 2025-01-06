#ifndef ARG_H
#define ARG_H
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "../libqr/qr.h"
#include "output.h"

#define MATCH(s1, s2) (strcasecmp(s1, s2) == 0)

#define PARSE_FUNC(name, type, min, max, signed) \
	bool parse_##name(const char *str, type *out, char **endptr);

#include "parse_int.h"

#undef PARSE_FUNC

extern const struct format_string {
	const char *name;
	enum output_format format;
} output_formats[], comments[];

bool parse_output_format(const char *str, enum output_format *format);

enum parse_color_reason {
	COLOR_SYNTAX,
	COLOR_OK,
	COLOR_NO_COLOR,
	COLOR_NO_ALPHA
} parse_color(const char *str, enum output_format format, struct color *color);
enum parse_color_reason parse_color_fallback(const char *str, enum output_format format, struct color *color, struct color fallback);

bool parse_ecl(const char *str, enum qr_ecl *ecl);
bool parse_encoding(const char *str, enum qr_mode *encoding);
#endif // ARG_H
