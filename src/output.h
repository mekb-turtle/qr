#ifndef OUTPUT_H
#define OUTPUT_H
#include <stdint.h>
#include <stdbool.h>
#include "qr.h"

enum output_format {
	OUTPUT_TEXT = 0x00,
	OUTPUT_HTML = 0x01,
	OUTPUT_UNICODE = 0x02,
	OUTPUT_UNICODE2X = 0x03,
	OUTPUT_ANSI = 0x04,
	OUTPUT_PNG = 0xf1,
	OUTPUT_JPEG = 0xf2,
	OUTPUT_GIF = 0xf3,
	OUTPUT_BMP = 0xf4,
	OUTPUT_FF = 0xf5,
	OUTPUT_IS_IMAGE = 0xf0
};
struct color {
	union {
		struct color_rgb {
			uint8_t r;
			uint8_t g;
			uint8_t b;
		} rgb;
		uint8_t ansi;
	};
};
struct color_pair {
	struct color bg, fg;
};
bool parse_color(const char *str, enum output_format format, struct color *color);
bool write_output(const char *filename, const struct qr_output *output, enum output_format format, struct color_pair colors);
#endif // OUTPUT_H
