#include "output.h"
#include "arg.h"
#include <stddef.h>
#include <string.h>
#include "qr.h"
#include "stbi_write.h"

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
#define ERROR(msg)    \
	{                 \
		*error = msg; \
		return false; \
	}

	struct qr_output img;
	// check for overflow
	img.size = 2 * (qr_t) opt.quiet_zone;
	if ((qr_t) qr->output.size > QR_MAX - img.size) ERROR("Output too large");
	img.size += (qr_t) qr->output.size;
	if ((qr_t) opt.module_size > QR_MAX / img.size) ERROR("Module size too large");
	img.size *= (qr_t) opt.module_size;

	// construct larger output before encoding
	img.data_size = QR_DATA_SIZE(img.size);
	img.data = qr->alloc.malloc(img.size * img.size);
	if (!img.data) ERROR(ERR_ALLOC);

	if (opt.invert && OUTPUT_HAS_COLOR(opt.format)) {
		// swapping colors is more efficient
		opt.invert = false;
		struct color tmp = opt.fg;
		opt.fg = opt.bg;
		opt.bg = tmp;
	}

	// clear output
	memset(img.data, opt.invert ? 0xFF : 0x00, img.data_size);

	// copy data
	struct qr_pos pos;
	for (pos.y = 0; pos.y < qr->output.size; pos.y++)
		for (pos.x = 0; pos.x < qr->output.size; pos.x++) {
			// skip if false and not inverted, or true and inverted
			if (qr_output_read(qr->output, pos) == false) continue;

			// write the square
			struct qr_pos off = QR_POS((pos.x + opt.quiet_zone) * opt.module_size, (pos.y + opt.quiet_zone) * opt.module_size);
			struct qr_pos rel;
			for (rel.y = 0; rel.y < opt.module_size; rel.y++)
				for (rel.x = 0; rel.x < opt.module_size; rel.x++) {
					qr_output_write(&img, QR_POS(off.x + rel.x, off.y + rel.y), !opt.invert);
				}
		}

	if (opt.format & OUTPUT_IS_IMAGE) {
#define WRITE(data, len) fwrite(data, 1, len, fp)
		// output image
		// set alloc functions for stbi
		output_alloc = qr->alloc;
		switch (opt.format) {
			case OUTPUT_PNG:
				break;
			case OUTPUT_BMP:
				break;
			case OUTPUT_TGA:
				break;
			case OUTPUT_HDR:
				break;
			case OUTPUT_JPG:
				break;
			case OUTPUT_FF:
				// stbi doesn't support this format
				// write it ourselves
				WRITE("farbfeld", 8); // magic
				const uint32_t size = TO_BE32((uint32_t) img.size);
				WRITE(&size, sizeof(size)); // width
				WRITE(&size, sizeof(size)); // height
				for (pos.y = 0; pos.y < img.size; pos.y++)
					for (pos.x = 0; pos.x < img.size; pos.x++) {
						struct color c = qr_output_read(img, pos) ? opt.fg : opt.bg;
						// farbfeld uses BE 16-bit values for each channel
						WRITE(((uint8_t[]){c.r, c.r, c.g, c.g, c.b, c.b, c.a, c.a}), 8);
					}
				break;
			default:
				FREE(img.data);
				ERROR("Invalid image format");
		}
	} else {
#define PRINT(...) fprintf(fp, __VA_ARGS__)
		// output text
		if (opt.format == OUTPUT_HTML) {
			// HTML header
			float perc = 100.0 / img.size;
			perc = 20;
			PRINT("<!DOCTYPE html><html><head><title>QR Code</title><style>\n");
			PRINT("body {background-color:rgb(%u,%u,%u); color:rgb(%u,%u,%u); margin:0;}\n", opt.bg.r, opt.bg.g, opt.bg.b, opt.fg.r, opt.fg.g, opt.fg.b);
			PRINT("table {border-collapse:collapse;}\n");
			PRINT("td {width:%fpx;height:%fpx;}\n", perc, perc); // scale to fit
			PRINT("td.a {background-color:rgb(%u,%u,%u);}\n", opt.fg.r, opt.fg.g, opt.fg.b);
			PRINT("td.b {background-color:rgb(%u,%u,%u);}\n", opt.bg.r, opt.bg.g, opt.bg.b);
			PRINT("</style>\n");
			PRINT("<meta charset=\"UTF-8\">\n");
			PRINT("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n");
			PRINT("<meta name=\"darkreader-lock\">");
			PRINT("</head><body>\n");
			PRINT("<table>\n");
		}
		// loop pixels

		for (pos.y = 0; pos.y < img.size; pos.y++) {
			if (opt.format == OUTPUT_HTML)
				PRINT("<tr>");

			for (pos.x = 0; pos.x < img.size; pos.x++) {
				bool bit = qr_output_read(img, pos);
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
						bool bit2 = qr_output_read(img, QR_POS(pos.x, pos.y + 1));
						PRINT(bit ? (bit2 ? "\u2588" : "\u2580") : (bit2 ? "\u2584" : " "));
						break;
					default:
						ERROR("Invalid image format");
				}
			}

			// newline
			if (opt.format == OUTPUT_HTML)
				PRINT("</tr>\n");
			else
				PRINT("\n");

			if (opt.format == OUTPUT_UNICODE2X) ++pos.y; // skip every other line
		}
		if (opt.format == OUTPUT_HTML) {
			// HTML footer
			PRINT("</table>\n");
			PRINT("</body></html>\n");
		}
	}

	FREE(img.data);

	return true;
}
