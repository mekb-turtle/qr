#include "arg.h"
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>

typedef long long int ll;
typedef unsigned long long int ull;

static bool parse_internal(const char *str, ull *out, char **endptr, bool is_signed, ull min, ull max) {
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
	while (**endptr) {
		if (!isspace(**endptr)) break;
		++*endptr;
	}

	if (own_endptr && *endptr_) return false; // not all characters were consumed

	// check if the result is in the valid range
	if (is_signed) {
		if ((ll) result < (ll) min || (ll) result > (ll) max) return false;
	} else {
		if (result < min || result > max) return false;
	}

	*out = result;
	return true;
}

// avoid reusing code for similar data types

#define PARSE_FUNC(name, type, min, max, signed)                                  \
	bool parse_##name(const char *str, type *out, char **endptr) {                \
		ull value;                                                                \
		if (!parse_internal(str, &value, endptr, signed, min, max)) return false; \
		*out = value;                                                             \
		return true;                                                              \
	}

#include "parse_int.h"

#undef PARSE_FUNC

bool parse_output_format(const char *str, enum output_format *format) {
	if (!str || !*str) return false;
	if (MATCH(str, "text") || MATCH(str, "txt") || MATCH(str, "t")) {
		*format = OUTPUT_TEXT;
	} else if (MATCH(str, "html") || MATCH(str, "h") || MATCH(str, "htm")) {
		*format = OUTPUT_HTML;
	} else if (MATCH(str, "unicode") || MATCH(str, "u")) {
		*format = OUTPUT_UNICODE;
	} else if (MATCH(str, "unicode2x") || MATCH(str, "u2x")) {
		*format = OUTPUT_UNICODE2X;
	} else if (MATCH(str, "png")) {
		*format = OUTPUT_PNG;
	} else if (MATCH(str, "bmp")) {
		*format = OUTPUT_BMP;
	} else if (MATCH(str, "tga") || MATCH(str, "targa")) {
		*format = OUTPUT_TGA;
	} else if (MATCH(str, "hdr")) {
		*format = OUTPUT_HDR;
	} else if (MATCH(str, "jpg") || MATCH(str, "jpeg")) {
		*format = OUTPUT_JPG;
	} else if (MATCH(str, "ff") || MATCH(str, "farbfeld")) {
		*format = OUTPUT_FF;
	} else {
		return false;
	}
	return true;
}

enum parse_color_reason parse_color_fallback(const char *str, enum output_format format, struct color *color, struct color fallback) {
	if (str) return parse_color(str, format, color);

	// use fallback color if no color was specified
	if (OUTPUT_HAS_COLOR(format)) *color = fallback;
	return COLOR_OK;
}

enum parse_color_reason parse_color(const char *str, enum output_format format, struct color *out) {
	if (!str || !*str) return COLOR_SYNTAX;
	char *endptr = NULL;

	struct color color;
	bool alpha_set = false;

	if (!parse_u8(str, &color.r, &endptr)) return COLOR_SYNTAX;
	if (*endptr != ',') return COLOR_SYNTAX; // missing comma

	if (!parse_u8(endptr + 1, &color.g, &endptr)) return COLOR_SYNTAX;
	if (*endptr != ',') return COLOR_SYNTAX; // missing comma

	if (!parse_u8(endptr + 1, &color.b, &endptr)) return COLOR_SYNTAX;

	if (*endptr == ',') {
		if (!parse_u8(endptr + 1, &color.a, &endptr)) return COLOR_SYNTAX;
		alpha_set = true;
	} else {
		color.a = 0xFF;
	}

	if (*endptr) return COLOR_SYNTAX; // not all characters were consumed

	if (!OUTPUT_HAS_COLOR(format)) return COLOR_NO_COLOR;              // only image formats have colors
	if (alpha_set && !OUTPUT_HAS_ALPHA(format)) return COLOR_NO_ALPHA; // alpha is not supported for this format

	*out = color;

	return COLOR_OK;
}

bool parse_ecl(const char *str, enum qr_ecl *ecl) {
	if (!str) return false;

	if (MATCH(str, "l") || MATCH(str, "low")) {
		*ecl = QR_ECL_LOW;
	} else if (MATCH(str, "m") || MATCH(str, "medium")) {
		*ecl = QR_ECL_MEDIUM;
	} else if (MATCH(str, "q") || MATCH(str, "quartile")) {
		*ecl = QR_ECL_QUARTILE;
	} else if (MATCH(str, "h") || MATCH(str, "high")) {
		*ecl = QR_ECL_HIGH;
	} else {
		return false;
	}

	return true;
}

bool parse_encoding(const char *str, enum qr_mode *encoding) {
	if (!str) return false;

	if (MATCH(str, "auto")) {
		*encoding = QR_MODE_AUTO;
	} else if (MATCH(str, "numeric") || MATCH(str, "n")) {
		*encoding = QR_MODE_NUMERIC;
	} else if (MATCH(str, "alphanumeric") || MATCH(str, "a")) {
		*encoding = QR_MODE_ALPHANUMERIC;
	} else if (MATCH(str, "byte") || MATCH(str, "b")) {
		*encoding = QR_MODE_BYTE;
	} else if (MATCH(str, "kanji") || MATCH(str, "k")) {
		*encoding = QR_MODE_KANJI;
	} else {
		return false;
	}

	return true;
}
