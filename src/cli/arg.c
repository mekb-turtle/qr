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

// output formats for parsing
#define NAME(...) \
	((const char *[]){__VA_ARGS__, NULL})
const struct format_string formats[] = {
        {OUTPUT_TEXT, NAME("text", "txt", "t"), "Text ('#'/' ')"},
        {OUTPUT_HTML, NAME("html", "h", "htm"), "HTML table"},
        {OUTPUT_CSV, NAME("csv", "c"), "CSV file (1/0)"},
        {OUTPUT_UNICODE, NAME("unicode", "u", "block"), "Block characters"},
        {OUTPUT_UNICODE2X, NAME("unicode2x", "unicode2", "u2x", "u2", "block2x", "block2"), "Block characters (2x2)"},

        {OUTPUT_RAW_BYTE, NAME("raw_byte", "raw", "r"), "Raw bytes"},
        {OUTPUT_RAW_BIT_LEFT, NAME("raw_bit_left", "rawbitleft", "rbl"), "Raw bits (from left, no pad)"},
        {OUTPUT_RAW_BIT_RIGHT, NAME("raw_bit_right", "rawbitright", "rbr"), "Raw bits (from right, no pad)"},

        {OUTPUT_PNG, NAME("png"), "PNG image"},
        {OUTPUT_BMP, NAME("bmp"), "BMP image"},
        {OUTPUT_TGA, NAME("tga", "targa"), "TGA image"},
        {OUTPUT_HDR, NAME("hdr"), "HDR image"},
        {OUTPUT_JPG, NAME("jpg", "jpeg"), "JPEG image"},
        {OUTPUT_FARBFELD, NAME("ff", "farbfeld"), "Farbfeld image"},
        {0, NULL, NULL},
};
#undef NAME

bool parse_output_format(const char *str, enum output_format *format) {
	if (!str || !*str) return false;
	for (const struct format_string *fmt = formats; fmt->arg_names; ++fmt) {
		for (const char *const *name = fmt->arg_names; *name; ++name) {
			if (MATCH(str, *name)) {
				*format = fmt->format;
				return true;
			}
		}
	}
	return false;
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
