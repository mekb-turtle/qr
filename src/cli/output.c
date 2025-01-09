#include "output.h"
#include "arg.h"
#include <stddef.h>
#include <string.h>
#include "stbi_write.h"
#include "endian.h"
#include <inttypes.h>
#include "../libqr/qr.h"

#define ERR_ALLOC "Failed to allocate memory"

#define FREE(ptr)                    \
	{                                \
		if (qr->alloc.free && ptr) { \
			qr->alloc.free(ptr);     \
		}                            \
		ptr = NULL;                  \
	}

static void write_data(void *context, void *data, int size) {
	fwrite(data, 1, size, (FILE *) context);
}

static void write_bit(FILE *fp, bool bit, uint8_t *byte, uint8_t *bit_index, bool from_left) {
	if (from_left) {
		if (bit) *byte |= 0x80 >> *bit_index;
	} else {
		if (bit) *byte |= 1 << *bit_index;
	}
	(*bit_index)++;
	if (*bit_index == 8) {
		fputc(*byte, fp);
		*byte = 0;
		*bit_index = 0;
	}
}

// loops over bitmap, calling callback for each pixel
// callback should write module_size pixels
static bool loop_bitmap(struct qr_bitmap bitmap, struct output_options opt, bool (*callback)(bool bit, bool quiet, struct qr_pos pos, FILE *fp, void *data), FILE *fp, void *data) {
	bool bg = opt.invert, fg = !bg;

	struct qr_pos pos, repeat;

#define LOOP(var, max) \
	for (var = 0; var < max; var++)

#define V_QUIET_LOOP()                                                \
	LOOP(pos.y, opt.quiet_zone) {                                     \
		LOOP(repeat.y, opt.module_size) {                             \
			LOOP(pos.x, opt.quiet_zone * 2 + bitmap.size) {           \
				if (!callback(bg, true, pos, fp, data)) return false; \
			}                                                         \
		}                                                             \
	}

	V_QUIET_LOOP(); // loop top quiet zone

	LOOP(pos.y, bitmap.size) { // loop bitmap
		LOOP(repeat.y, opt.module_size) {
			LOOP(pos.x, opt.quiet_zone) { // loop left quiet zone
				if (!callback(bg, true, pos, fp, data)) return false;
			}

			LOOP(pos.x, bitmap.size) { // loop bitmap
				if (!callback(qr_bitmap_read(bitmap, pos) ? fg : bg, false, pos, fp, data)) return false;
			}

			LOOP(pos.x, opt.quiet_zone) { // loop right quiet zone
				if (!callback(bg, true, pos, fp, data)) return false;
			}
		}
	}

	V_QUIET_LOOP(); // loop bottom quiet zone

#undef LOOP
#undef V_QUIET_LOOP

	return true;
}

static bool output_bitmap_read(struct qr_bitmap bitmap, struct qr_pos pos, struct output_options opt) {
	bool bg = opt.invert, fg = !bg;
	qr_t quiet_size = opt.quiet_zone * opt.module_size;
	qr_t bitmap_size = bitmap.size * opt.module_size;

	// check if in quiet zone
	if (pos.x < quiet_size || pos.y < quiet_size) return bg;
	pos.x -= quiet_size;
	pos.y -= quiet_size;

	if (pos.x >= bitmap_size || pos.y >= bitmap_size) return bg;

	pos.x /= opt.module_size;
	pos.y /= opt.module_size;
	return qr_bitmap_read(bitmap, pos) ? fg : bg;
}

struct data_raw_opt {
	uint8_t byte, bit_index;
	const struct output_options *opt;
	uint8_t *modules_fg, *modules_bg;
};

static bool write_data_raw(bool bit, bool quiet, struct qr_pos pos, FILE *fp, void *data_) {
	(void) quiet;
	(void) pos;
	struct data_raw_opt *data = (struct data_raw_opt *) data_;
	qr_t times = data->opt->module_size;
	switch (data->opt->format) {
		case OUTPUT_RAW_BIT_LEFT:
			for (size_t i = 0; i < times; i++)
				write_bit(fp, bit, &data->byte, &data->bit_index, false);
			return true;
		case OUTPUT_RAW_BIT_RIGHT:
			for (size_t i = 0; i < times; i++)
				write_bit(fp, bit, &data->byte, &data->bit_index, true);
			return true;
		case OUTPUT_RAW_BYTE:
			// optimize for writing bytes
			fwrite(bit ? data->modules_fg : data->modules_bg, 1, times, fp);
			return true;
		default:
			return false;
	}
}

struct data_ff_opt {
	uint8_t *modules_fg, *modules_bg;
	qr_t module_size;
};

static bool write_data_ff(bool bit, bool quiet, struct qr_pos pos, FILE *fp, void *data_) {
	(void) quiet;
	(void) pos;
	struct data_ff_opt *data = (struct data_ff_opt *) data_;
	// farbfeld uses BE 16-bit values for each channel
	fwrite(bit ? data->modules_fg : data->modules_bg, 1, data->module_size * 8, fp);
	return true;
}

bool write_output(FILE *fp, const struct qr *qr, struct output_options opt, const char **error) {
	if (opt.format == OUTPUT_UNICODE2X && opt.module_size % 2 == 0) {
		// save space
		opt.module_size /= 2;
		opt.format = OUTPUT_UNICODE;
	}

	if (opt.format == OUTPUT_HTML)
		opt.module_size = 1; // HTML table is scaled to viewport width, so module size is useless

#define ERROR(msg)    \
	{                 \
		*error = msg; \
		return false; \
	}

	// check for overflow
	if ((qr_t) opt.quiet_zone > QR_MAX / 2) ERROR("Quiet zone too large");
	qr_t size = 2 * (qr_t) opt.quiet_zone;
	if ((qr_t) qr->output.size > QR_MAX - size) ERROR("Bitmap too large");
	size += (qr_t) qr->output.size;
	if ((qr_t) opt.module_size > QR_MAX / size) ERROR("Module size too large");
	size *= (qr_t) opt.module_size;
	if ((qr_t) size > QR_MAX / size) ERROR("Bitmap too large");

	// prevent stupidly large bitmaps (>1 MiB) from being created
	if (QR_DATA_SIZE(size) > 0x200000) ERROR("Bitmap too large, try reducing the module size");

	if (opt.invert && OUTPUT_HAS_COLOR(opt.format)) {
		// swapping colors is more efficient
		opt.invert = false;
		struct color tmp = opt.fg;
		opt.fg = opt.bg;
		opt.bg = tmp;
	}

	if (OUTPUT_IS_BINARY(opt.format)) {
#define WRITE(data, len) fwrite((data), 1, (len), fp)
#define PRINT(...) fprintf(fp, __VA_ARGS__)
		// output image
		switch (opt.format) {
			case OUTPUT_RAW_BIT_LEFT:
			case OUTPUT_RAW_BIT_RIGHT:
			case OUTPUT_RAW_BYTE: {
				struct data_raw_opt data = {0, 0, &opt, NULL, NULL};

				if (opt.format == OUTPUT_RAW_BYTE) {
					// buffer for one row of a module
					data.modules_fg = qr->alloc.malloc(opt.module_size);
					if (!data.modules_fg) ERROR(ERR_ALLOC);
					data.modules_bg = qr->alloc.malloc(opt.module_size);
					if (!data.modules_bg) {
						FREE(data.modules_fg);
						ERROR(ERR_ALLOC);
					}

					memset(data.modules_fg, 0x00, opt.module_size);
					memset(data.modules_bg, 0xff, opt.module_size);
				}

				loop_bitmap(qr->output, opt, write_data_raw, fp, &data);
				if (opt.format == OUTPUT_RAW_BYTE) break;

				// write remaining bits
				while (data.bit_index > 0) write_bit(fp, 0, &data.byte, &data.bit_index, opt.format == OUTPUT_RAW_BIT_RIGHT);
				FREE(data.modules_fg);
				FREE(data.modules_bg);
				break;
			}

			case OUTPUT_FARBFELD: {
				// stbi doesn't support this format
				// write it ourselves
				// see https://tools.suckless.org/farbfeld/
				const uint8_t magic_ff[8] = {102, 97, 114, 98, 102, 101, 108, 100};
				WRITE(magic_ff, 8); // magic, "farbfeld"
				                    // using bytes since not all compilers use UTF-8
				const uint32_t size_ff = TO_BE32((uint32_t) size);
				WRITE(&size_ff, sizeof(size_ff)); // width
				WRITE(&size_ff, sizeof(size_ff)); // height

				struct data_ff_opt data = {NULL, NULL, opt.module_size};

				// buffer for one row of a module
				data.modules_fg = qr->alloc.malloc(opt.module_size * 8);
				if (!data.modules_fg) ERROR(ERR_ALLOC);
				data.modules_bg = qr->alloc.malloc(opt.module_size * 8);
				if (!data.modules_bg) {
					FREE(data.modules_fg);
					ERROR(ERR_ALLOC);
				}

				// prepare colors
				for (size_t i = 0; i < opt.module_size * 8; i += 8) {
					data.modules_fg[i + 0] = opt.fg.r;
					data.modules_fg[i + 1] = opt.fg.r;
					data.modules_fg[i + 2] = opt.fg.g;
					data.modules_fg[i + 3] = opt.fg.g;
					data.modules_fg[i + 4] = opt.fg.b;
					data.modules_fg[i + 5] = opt.fg.b;
					data.modules_fg[i + 6] = opt.fg.a;
					data.modules_fg[i + 7] = opt.fg.a;

					data.modules_bg[i + 0] = opt.bg.r;
					data.modules_bg[i + 1] = opt.bg.r;
					data.modules_bg[i + 2] = opt.bg.g;
					data.modules_bg[i + 3] = opt.bg.g;
					data.modules_bg[i + 4] = opt.bg.b;
					data.modules_bg[i + 5] = opt.bg.b;
					data.modules_bg[i + 6] = opt.bg.a;
					data.modules_bg[i + 7] = opt.bg.a;
				}

				loop_bitmap(qr->output, opt, write_data_ff, fp, &data);
				FREE(data.modules_fg);
				FREE(data.modules_bg);
				break;
			}

			default: {
				// create image for use with stbi
				bool is_float = OUTPUT_HAS_FLOAT(opt.format);

				uint8_t bytes_per_channel = is_float ? sizeof(float) : 1;
#define CHANNELS (4)
				uint8_t bytes_per_pixel = bytes_per_channel * CHANNELS;

				if (size > SIZE_MAX / bytes_per_pixel) ERROR("Bitmap too large");
				size_t image_stride = (size_t) size * bytes_per_pixel;
				if (image_stride > SIZE_MAX / size) ERROR("Bitmap too large");
				size_t image_size = size * image_stride;
				uint8_t *image = qr->alloc.malloc(image_size);
				if (!image) ERROR(ERR_ALLOC);
				float *imagef = (float *) image;
				memset(image, 0, image_size);

				// prepare colors
				uint8_t fg[CHANNELS] = {opt.fg.r, opt.fg.g, opt.fg.b, opt.fg.a},
				        bg[CHANNELS] = {opt.bg.r, opt.bg.g, opt.bg.b, opt.bg.a};
				float fgf[CHANNELS] = {opt.fg.r / 255.0f, opt.fg.g / 255.0f, opt.fg.b / 255.0f, opt.fg.a / 255.0f},
				      bgf[CHANNELS] = {opt.bg.r / 255.0f, opt.bg.g / 255.0f, opt.bg.b / 255.0f, opt.bg.a / 255.0f};

				// copy image data to new buffer
				struct qr_pos pos;
				size_t i;
				for (i = 0, pos.y = 0; pos.y < size; ++pos.y)
					for (pos.x = 0; pos.x < size; ++pos.x, i += CHANNELS) {
						bool bit = output_bitmap_read(qr->output, pos, opt);
						if (is_float)
							memcpy(imagef + i, bit ? fgf : bgf, sizeof(fgf));
						else
							memcpy(image + i, bit ? fg : bg, sizeof(fg));
					}

				// set alloc functions for stbi
				output_alloc = qr->alloc;
				switch (opt.format) {
					case OUTPUT_PNG:
						stbi_write_png_to_func(write_data, fp, size, size, CHANNELS, image, image_stride);
						break;
					case OUTPUT_BMP:
						stbi_write_bmp_to_func(write_data, fp, size, size, CHANNELS, image);
						break;
					case OUTPUT_TGA:
						stbi_write_tga_to_func(write_data, fp, size, size, CHANNELS, image);
						break;
					case OUTPUT_HDR:
						stbi_write_hdr_to_func(write_data, fp, size, size, CHANNELS, imagef);
						break;
					case OUTPUT_JPG:
						stbi_write_jpg_to_func(write_data, fp, size, size, CHANNELS, image, 100);
						break;
					default:
						FREE(image);
						ERROR("Invalid image format");
				}
				FREE(image);
				break;
			}
		}
	} else {
		// output text
		if (opt.format == OUTPUT_HTML) {
			// HTML header
			float perc = 100.0 / size;
			PRINT("<!DOCTYPE html><html><head><title>QR Code</title><style>"

			      "body{"
			      "background-color:rgb(%" PRIu8 ",%" PRIu8 ",%" PRIu8 ");" // set page colors
			      "color:rgb(%" PRIu8 ",%" PRIu8 ",%" PRIu8 ");"
			      "margin:0;overflow-x:hidden;}", // remove margin and h scroll
			      opt.bg.r, opt.bg.g, opt.bg.b, opt.fg.r, opt.fg.g, opt.fg.b);
			PRINT("table{border-collapse:collapse;}"       // remove cell spacing
			      "td{width:%fvw;height:%fvw;"             // set cell size to fraction of viewport width
			      "aspect-ratio:1/1;padding:0;margin:0;}", // square cells
			      perc, perc);                             // scale to fit

			PRINT("td.a{background-color:rgb(%" PRIu8 ",%" PRIu8 ",%" PRIu8 ");}", opt.fg.r, opt.fg.g, opt.fg.b); // set fg color
			PRINT("td.b{background-color:rgb(%" PRIu8 ",%" PRIu8 ",%" PRIu8 ");}", opt.bg.r, opt.bg.g, opt.bg.b); // set bg color

			PRINT("</style>"
			      "<meta charset=\"UTF-8\">"
			      "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
			      "<meta name=\"darkreader-lock\">" // prevent Dark Reader from messing with colors
			      "</head><body>"
			      "<table>");
		}
		// loop pixels

		struct qr_pos pos;
		for (pos.y = 0; pos.y < size; pos.y++) {
			if (opt.format == OUTPUT_HTML)
				PRINT("<tr>");

			for (pos.x = 0; pos.x < size; pos.x++) {
				bool bit = output_bitmap_read(qr->output, pos, opt);
				switch (opt.format) {
					case OUTPUT_TEXT:
						PRINT(bit ? "##" : "  ");
						break;
					case OUTPUT_CSV:
						if (pos.x > 0) PRINT(",");
						PRINT("%c", bit ? '1' : '0');
						break;
					case OUTPUT_HTML:
						PRINT("<td class=\"%c\"></td>", bit ? 'a' : 'b');
						break;
					case OUTPUT_UNICODE:
						// full block or space
						PRINT(bit ? "\u2588\u2588" : "  ");
						break;
					case OUTPUT_UNICODE2X:;
						bool bit2 = output_bitmap_read(qr->output, QR_POS(pos.x, pos.y + 1), opt);
						PRINT(bit ? (bit2 ? "\u2588" : "\u2580") : (bit2 ? "\u2584" : " "));
						break;
					default:
						ERROR("Invalid text format");
				}
			}

			// newline
			if (opt.format == OUTPUT_HTML)
				PRINT("</tr>");
			else
				PRINT("\n");

			if (opt.format == OUTPUT_UNICODE2X) ++pos.y; // skip every other line
		}
		if (opt.format == OUTPUT_HTML) {
			// HTML footer
			PRINT("</table>");
			PRINT("</body></html>\n");
		}
	}

	return true;
}
