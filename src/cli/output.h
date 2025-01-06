#ifndef OUTPUT_H
#define OUTPUT_H
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "../libqr/qr.h"

#define QUIET_ZONE_DEFAULT (4)

enum output_format {
	OUTPUT_FLAG_BINARY = 0x40,
	OUTPUT_FLAG_IMAGE = 0x80 | OUTPUT_FLAG_BINARY,

	OUTPUT_TEXT = 0,
	OUTPUT_UNICODE = 1,
	OUTPUT_UNICODE2X = 2,
	OUTPUT_HTML = 3,
	OUTPUT_CSV = 4,

	OUTPUT_RAW_BIT = 0 | OUTPUT_FLAG_BINARY,
	OUTPUT_RAW_BYTE = 1 | OUTPUT_FLAG_BINARY,

	OUTPUT_PNG = 1 | OUTPUT_FLAG_IMAGE,
	OUTPUT_BMP = 2 | OUTPUT_FLAG_IMAGE,
	OUTPUT_TGA = 3 | OUTPUT_FLAG_IMAGE,
	OUTPUT_HDR = 4 | OUTPUT_FLAG_IMAGE,
	OUTPUT_JPG = 5 | OUTPUT_FLAG_IMAGE,
	OUTPUT_FF = 6 | OUTPUT_FLAG_IMAGE,
};

#define OUTPUT_IS_IMAGE(format) (((format) & OUTPUT_FLAG_IMAGE) == OUTPUT_FLAG_IMAGE)
#define OUTPUT_IS_BINARY(format) (((format) & OUTPUT_FLAG_BINARY) == OUTPUT_FLAG_BINARY)

#define OUTPUT_HAS_COLOR(format) (OUTPUT_IS_IMAGE(format) || (format) == OUTPUT_HTML)
#define OUTPUT_HAS_ALPHA(format) ((format) == OUTPUT_PNG || (format) == OUTPUT_TGA || (format) == OUTPUT_FF)
#define OUTPUT_HAS_FLOAT(format) ((format) == OUTPUT_HDR)

struct color {
	uint8_t r, g, b, a;
};

struct output_options {
	enum output_format format;
	struct color fg, bg;
	qr_t quiet_zone, module_size;
	bool invert;
};

bool write_output(FILE *fp, const struct qr *qr, struct output_options opt, const char **error);
#endif // OUTPUT_H
