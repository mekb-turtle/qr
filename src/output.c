#include "output.h"
#include "arg.h"
#include <stddef.h>

bool parse_color(const char *str, enum output_format format, struct color *out) {
	if (!(format & OUTPUT_IS_IMAGE) && format != OUTPUT_ANSI) return false; // no need
	char *endptr = NULL;
	struct color color = {0};

	unsigned long long int r = 0, g = 0, b = 0;

	if (!parse_ull(str, &r, &endptr)) return false;
	if (format != OUTPUT_ANSI) {
		if (*endptr != ',') return false; // missing comma

		if (!parse_ull(endptr + 1, &g, &endptr)) return false;
		if (*endptr != ',') return false; // missing comma

		if (!parse_ull(endptr + 1, &b, &endptr)) return false;
	}
	if (*endptr) return false; // not all characters were consumed

	if (r > 255 || g > 255 || b > 255) return false; // invalid color

	color.rgb.r = r;
	color.rgb.g = g;
	color.rgb.b = b;
	*out = color;

	return true;
}

bool write_output(const char *filename, const struct qr_output *output, enum output_format format, struct color_pair colors) {
	(void) filename;
	(void) output;
	(void) format;
	(void) colors;
	return false;
}
