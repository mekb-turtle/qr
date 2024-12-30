#ifndef OUTPUT_H
#define OUTPUT_H
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "qr.h"

#define QUIET_ZONE_DEFAULT (4)

enum output_format {
	OUTPUT_TEXT = 0x00,
	OUTPUT_HTML = 0x01,
	OUTPUT_UNICODE = 0x02,
	OUTPUT_UNICODE2X = 0x03,
	OUTPUT_PNG = 0xf1,
	OUTPUT_BMP = 0xf2,
	OUTPUT_TGA = 0xf3,
	OUTPUT_HDR = 0xf4,
	OUTPUT_JPG = 0xf5,
	OUTPUT_FF = 0xf6,
	OUTPUT_IS_IMAGE = 0xf0
};
#define OUTPUT_HAS_COLOR(format) ((format) & OUTPUT_IS_IMAGE || (format) == OUTPUT_HTML)
#define OUTPUT_HAS_ALPHA(format) ((format) == OUTPUT_PNG || (format) == OUTPUT_TGA || (format) == OUTPUT_FF)
#define OUTPUT_HAS_FLOAT(format) ((format) == OUTPUT_HDR)

struct color {
	uint8_t r, g, b, a;
};

typedef uint8_t module_t;
#define MODULE_MAX (UINT8_MAX)
#define MODULE_MIN 0

struct output_options {
	enum output_format format;
	struct color fg, bg;
	module_t quiet_zone, module_size;
	bool invert;
};

bool write_output(FILE *fp, const struct qr *qr, struct output_options opt, const char **error);
#endif // OUTPUT_H
