#include "arg.h"
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>

typedef long long int ll;
typedef unsigned long long int ull;

// avoid reusing code
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
	while (*endptr) {
		if (!isspace(*endptr)) break;
		++endptr;
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

bool parse_ullong(const char *str, unsigned long long int *out, char **endptr) {
	ull value;
	if (!parse_internal(str, &value, endptr, true, 0, ULLONG_MAX)) return false;
	*out = value;
	return true;
}

bool parse_llong(const char *str, long long int *out, char **endptr) {
	ull value;
	if (!parse_internal(str, &value, endptr, true, LLONG_MIN, LLONG_MAX)) return false;
	*out = value;
	return true;
}

bool parse_ulong(const char *str, unsigned long int *out, char **endptr) {
	ull value;
	if (!parse_internal(str, &value, endptr, false, 0, ULONG_MAX)) return false;
	*out = value;
	return true;
}

bool parse_long(const char *str, long int *out, char **endptr) {
	ull value;
	if (!parse_internal(str, &value, endptr, true, LONG_MIN, LONG_MAX)) return false;
	*out = value;
	return true;
}

bool parse_uint(const char *str, unsigned int *out, char **endptr) {
	ull value;
	if (!parse_internal(str, &value, endptr, false, 0, UINT_MAX)) return false;
	*out = value;
	return true;
}

bool parse_int(const char *str, int *out, char **endptr) {
	ull value;
	if (!parse_internal(str, &value, endptr, true, INT_MIN, INT_MAX)) return false;
	*out = value;
	return true;
}

#define MATCH(s1, s2) (strcasecmp(s1, s2) == 0)

bool parse_output_format(const char *str, enum output_format *format) {
	if (!str || !*str) return false;
	if (MATCH(str, "text") || MATCH(str, "txt") || MATCH(str, "t")) {
		*format = OUTPUT_TEXT;
	} else if (MATCH(str, "html") || MATCH(str, "h")) {
		*format = OUTPUT_HTML;
	} else if (MATCH(str, "unicode") || MATCH(str, "u")) {
		*format = OUTPUT_UNICODE;
	} else if (MATCH(str, "unicode2x") || MATCH(str, "u2x")) {
		*format = OUTPUT_UNICODE2X;
	} else if (MATCH(str, "ansi")) {
		*format = OUTPUT_ANSI;
	} else if (MATCH(str, "png")) {
		*format = OUTPUT_PNG;
	} else if (MATCH(str, "jpeg")) {
		*format = OUTPUT_JPEG;
	} else if (MATCH(str, "gif")) {
		*format = OUTPUT_GIF;
	} else if (MATCH(str, "bmp")) {
		*format = OUTPUT_BMP;
	} else if (MATCH(str, "ff") || MATCH(str, "farbfeld")) {
		*format = OUTPUT_FF;
	} else {
		return false;
	}
	return true;
}

bool parse_color_fallback(const char *str, enum output_format format, struct color *color, struct color_rgb fallback_rgb, uint8_t fallback_ansi) {
	if (str) return parse_color(str, format, color);
	if (format == OUTPUT_ANSI) {
		color->ansi = fallback_ansi;
	} else {
		color->rgb = fallback_rgb;
	}
	return true;
}

bool parse_color(const char *str, enum output_format format, struct color *out) {
	if (!str || !*str) return false;
	if (!(format & OUTPUT_IS_IMAGE) && format != OUTPUT_ANSI) return false; // no need
	char *endptr = NULL;
	struct color color = {0};

	unsigned long long int r = 0, g = 0, b = 0;

	if (!parse_ullong(str, &r, &endptr)) return false;
	if (format != OUTPUT_ANSI) {
		if (*endptr != ',') return false; // missing comma

		if (!parse_ullong(endptr + 1, &g, &endptr)) return false;
		if (*endptr != ',') return false; // missing comma

		if (!parse_ullong(endptr + 1, &b, &endptr)) return false;
	}
	if (*endptr) return false; // not all characters were consumed

	if (r > 255 || g > 255 || b > 255) return false; // invalid color

	color.rgb.r = r;
	color.rgb.g = g;
	color.rgb.b = b;
	*out = color;

	return true;
}

bool parse_ecl(const char *str, enum qr_ecl *ecl) {
	if (!str) return false;

	if (MATCH(str, "l") || MATCH(str, "low")) {
		*ecl = ECL_LOW;
	} else if (MATCH(str, "m") || MATCH(str, "medium")) {
		*ecl = ECL_MEDIUM;
	} else if (MATCH(str, "q") || MATCH(str, "quartile")) {
		*ecl = ECL_QUARTILE;
	} else if (MATCH(str, "h") || MATCH(str, "high")) {
		*ecl = ECL_HIGH;
	} else {
		return false;
	}

	return true;
}

bool parse_encoding(const char *str, enum qr_encoding *encoding) {
	if (!str) return false;

	if (MATCH(str, "auto")) {
		*encoding = ENC_AUTO;
	} else if (MATCH(str, "numeric") || MATCH(str, "n")) {
		*encoding = ENC_NUMERIC;
	} else if (MATCH(str, "alphanumeric") || MATCH(str, "a")) {
		*encoding = ENC_ALPHANUMERIC;
	} else if (MATCH(str, "byte") || MATCH(str, "b")) {
		*encoding = ENC_BYTE;
	} else if (MATCH(str, "kanji") || MATCH(str, "k")) {
		*encoding = ENC_KANJI;
	} else {
		return false;
	}

	return true;
}
