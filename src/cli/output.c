#include "output.h"
#include "arg.h"
#include <stddef.h>
#include <string.h>
#include "stbi_write.h"
#include "endian.h"
#include <inttypes.h>

#define ERR_ALLOC "Failed to allocate memory"

#define FREE(ptr)                    \
	{                                \
		if (qr->alloc.free && ptr) { \
			qr->alloc.free(ptr);     \
		}                            \
		ptr = NULL;                  \
	}

void write_data(void *context, void *data, int size) {
	fwrite(data, 1, size, (FILE *) context);
}

bool write_output(FILE *fp, const struct qr *qr, struct output_options opt, const char **error) {
	if (opt.format == OUTPUT_UNICODE2X && opt.module_size % 2 == 0) {
		// save space
		opt.module_size /= 2;
		opt.format = OUTPUT_UNICODE;
	}

	struct qr_bitmap bitmap;
	bitmap.data = NULL;
	uint8_t *image = NULL;

#define ERROR(msg)         \
	{                      \
		*error = msg;      \
		FREE(bitmap.data); \
		FREE(image);       \
		return false;      \
	}

	// check for overflow
	bitmap.size = 2 * (qr_t) opt.quiet_zone;
	if ((qr_t) qr->output.size > QR_MAX - bitmap.size) ERROR("Bitmap too large");
	bitmap.size += (qr_t) qr->output.size;
	if ((qr_t) opt.module_size > QR_MAX / bitmap.size) ERROR("Module size too large");
	bitmap.size *= (qr_t) opt.module_size;
	if ((qr_t) bitmap.size > QR_MAX / bitmap.size * 8) ERROR("Bitmap too large");

	// construct larger output before encoding
	bitmap.data_size = QR_DATA_SIZE(bitmap.size);
	bitmap.data = qr->alloc.malloc(bitmap.data_size);
	if (!bitmap.data) ERROR(ERR_ALLOC);

	if (opt.invert && OUTPUT_HAS_COLOR(opt.format)) {
		// swapping colors is more efficient
		opt.invert = false;
		struct color tmp = opt.fg;
		opt.fg = opt.bg;
		opt.bg = tmp;
	}

	// clear output
	memset(bitmap.data, opt.invert ? 0xFF : 0x00, bitmap.data_size);

	// copy data
	struct qr_pos pos;
	for (pos.y = 0; pos.y < qr->output.size; pos.y++)
		for (pos.x = 0; pos.x < qr->output.size; pos.x++) {
			// skip if false and not inverted, or true and inverted
			if (qr_bitmap_read(qr->output, pos) == false) continue;

			// write the square
			struct qr_pos off = QR_POS((pos.x + opt.quiet_zone) * opt.module_size, (pos.y + opt.quiet_zone) * opt.module_size);
			struct qr_pos rel;
			for (rel.y = 0; rel.y < opt.module_size; rel.y++)
				for (rel.x = 0; rel.x < opt.module_size; rel.x++) {
					qr_bitmap_write(&bitmap, QR_POS(off.x + rel.x, off.y + rel.y), !opt.invert);
				}
		}

	if (opt.format & OUTPUT_IS_IMAGE) {
#define WRITE(data, len) fwrite(data, 1, len, fp)
		// output image
		if (opt.format == OUTPUT_FF) {
			// stbi doesn't support this format
			// write it ourselves
			// see https://tools.suckless.org/farbfeld/
			WRITE("farbfeld", 8); // magic
			const uint32_t size = TO_BE32((uint32_t) bitmap.size);
			WRITE(&size, sizeof(size)); // width
			WRITE(&size, sizeof(size)); // height
			for (pos.y = 0; pos.y < bitmap.size; pos.y++)
				for (pos.x = 0; pos.x < bitmap.size; pos.x++) {
					struct color c = qr_bitmap_read(bitmap, pos) ? opt.fg : opt.bg;
					// farbfeld uses BE 16-bit values for each channel
					WRITE(((uint8_t[]){c.r, c.r, c.g, c.g, c.b, c.b, c.a, c.a}), 8);
				}
		} else {
			// create image for use with stbi
			bool is_float = OUTPUT_HAS_FLOAT(opt.format);

			uint8_t bytes_per_channel = is_float ? sizeof(float) : 1;
			const uint8_t channels = 4;
			uint8_t bytes_per_pixel = bytes_per_channel * channels;

			if (bitmap.size > SIZE_MAX / bytes_per_pixel) ERROR("Bitmap too large");
			size_t image_stride = (size_t) bitmap.size * bytes_per_pixel;
			if (image_stride > SIZE_MAX / bitmap.size) ERROR("Bitmap too large");
			size_t image_size = bitmap.size * image_stride;
			image = qr->alloc.malloc(image_size);
			float *imagef = (float *) image;
			if (!image) ERROR(ERR_ALLOC);

			// copy image data to new buffer
			size_t i;
			for (i = 0, pos.y = 0; pos.y < bitmap.size; pos.y++)
				for (pos.x = 0; pos.x < bitmap.size; pos.x++, i += channels) {
					struct color c = qr_bitmap_read(bitmap, pos) ? opt.fg : opt.bg;
					if (is_float) {
						imagef[i + 0] = c.r / 255.0f;
						imagef[i + 1] = c.g / 255.0f;
						imagef[i + 2] = c.b / 255.0f;
						imagef[i + 3] = c.a / 255.0f;
					} else {
						image[i + 0] = c.r;
						image[i + 1] = c.g;
						image[i + 2] = c.b;
						image[i + 3] = c.a;
					}
				}
			FREE(bitmap.data);

			// set alloc functions for stbi
			output_alloc = qr->alloc;
			switch (opt.format) {
				case OUTPUT_PNG:
					stbi_write_png_to_func(write_data, fp, bitmap.size, bitmap.size, channels, image, image_stride);
					break;
				case OUTPUT_BMP:
					stbi_write_bmp_to_func(write_data, fp, bitmap.size, bitmap.size, channels, image);
					break;
				case OUTPUT_TGA:
					stbi_write_tga_to_func(write_data, fp, bitmap.size, bitmap.size, channels, image);
					break;
				case OUTPUT_HDR:
					stbi_write_hdr_to_func(write_data, fp, bitmap.size, bitmap.size, channels, (float *) image);
					break;
				case OUTPUT_JPG:
					stbi_write_jpg_to_func(write_data, fp, bitmap.size, bitmap.size, channels, image, 100);
					break;
				default:
					ERROR("Invalid image format");
			}
			FREE(image);
		}
	} else {
#define PRINT(...) fprintf(fp, __VA_ARGS__)
		// output text
		if (opt.format == OUTPUT_HTML) {
			// HTML header
			float perc = 100.0 / bitmap.size;
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

		for (pos.y = 0; pos.y < bitmap.size; pos.y++) {
			if (opt.format == OUTPUT_HTML)
				PRINT("<tr>");

			for (pos.x = 0; pos.x < bitmap.size; pos.x++) {
				bool bit = qr_bitmap_read(bitmap, pos);
				switch (opt.format) {
					case OUTPUT_TEXT:
						PRINT(bit ? "##" : "  ");
						break;
					case OUTPUT_HTML:
						PRINT("<td class=\"%c\"></td>", bit ? 'a' : 'b');
						break;
					case OUTPUT_UNICODE:
						// full block or space
						PRINT(bit ? "\u2588\u2588" : "  ");
						break;
					case OUTPUT_UNICODE2X:;
						bool bit2 = qr_bitmap_read(bitmap, QR_POS(pos.x, pos.y + 1));
						PRINT(bit ? (bit2 ? "\u2588" : "\u2580") : (bit2 ? "\u2584" : " "));
						break;
					default:
						ERROR("Invalid image format");
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

	FREE(bitmap.data);

	return true;
}
