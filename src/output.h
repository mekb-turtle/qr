#ifndef OUTPUT_H
#define OUTPUT_H
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "qr.h"

enum output_format {
	OUTPUT_TEXT = 0x00,
	OUTPUT_HTML = 0x01,
	OUTPUT_UNICODE = 0x02,
	OUTPUT_UNICODE2X = 0x03,
	OUTPUT_PNG = 0xf1,
	OUTPUT_JPEG = 0xf2,
	OUTPUT_GIF = 0xf3,
	OUTPUT_BMP = 0xf4,
	OUTPUT_FF = 0xf5,
	OUTPUT_IS_IMAGE = 0xf0
};
struct color {
	uint8_t r, g, b;
};
struct output_info {
	enum output_format format;
	struct color fg, bg;
	uint8_t quiet_zone, module_size;
};

#define OUTPUT_INFO(format_, fg_, bg_, quiet_zone_, module_size_) \
	((struct output_info){.format = format_, .fg = fg_, .bg = bg_, .quiet_zone = quiet_zone_, .module_size = module_size_})

bool write_output(FILE *fp, const struct qr_output *output, struct output_info info);
#endif // OUTPUT_H
