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
const struct format_string output_formats[] = {
        {"text",      OUTPUT_TEXT     },
        {"txt",       OUTPUT_TEXT     },
        {"t",         OUTPUT_TEXT     },
        {"html",      OUTPUT_HTML     },
        {"h",         OUTPUT_HTML     },
        {"htm",       OUTPUT_HTML     },
        {"csv",       OUTPUT_CSV      },
        {"c",         OUTPUT_CSV      },
        {"unicode",   OUTPUT_UNICODE  },
        {"u",         OUTPUT_UNICODE  },
        {"unicode2x", OUTPUT_UNICODE2X},
        {"unicode2",  OUTPUT_UNICODE2X},
        {"u2x",       OUTPUT_UNICODE2X},
        {"u2",        OUTPUT_UNICODE2X},

        {"raw_byte",  OUTPUT_RAW_BYTE },
        {"raw",       OUTPUT_RAW_BYTE },
        {"r",         OUTPUT_RAW_BYTE },
        {"raw_bit",   OUTPUT_RAW_BIT  },
        {"rawbit",    OUTPUT_RAW_BIT  },
        {"rb",        OUTPUT_RAW_BIT  },

        {"png",       OUTPUT_PNG      },
        {"bmp",       OUTPUT_BMP      },
        {"tga",       OUTPUT_TGA      },
        {"targa",     OUTPUT_TGA      },
        {"hdr",       OUTPUT_HDR      },
        {"jpg",       OUTPUT_JPG      },
        {"jpeg",      OUTPUT_JPG      },
        {"ff",        OUTPUT_FF       },
        {"farbfeld",  OUTPUT_FF       },
        {NULL,        0               }
};

const struct format_string comments[] = {
        {"'#' (hash) and ' ' (space)",                OUTPUT_TEXT     },
        {"Uses HTML tables",                          OUTPUT_HTML     },
        {"Comma-separated values",                    OUTPUT_CSV      },
        {"1 character per module",                    OUTPUT_UNICODE  },
        {"1 character per 2x2 modules",               OUTPUT_UNICODE2X},
        {"8 bits per module",                         OUTPUT_RAW_BYTE },
        {"1 bit per module (left-most module = MSb)", OUTPUT_RAW_BIT  },
        {NULL,                                        0               }
};

bool parse_output_format(const char *str, enum output_format *format) {
	if (!str || !*str) return false;
	for (const struct format_string *fmt = output_formats; fmt->name; ++fmt) {
		if (MATCH(str, fmt->name)) {
			*format = fmt->format;
			return true;
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
