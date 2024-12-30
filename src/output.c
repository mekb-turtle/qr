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

bool write_output(FILE *fp, const struct qr *qr, struct output_options opt) {
	// check for overflow
	if (qr->output.size > QR_MAX / opt.module_size) return false;
	if (qr->output.size * opt.module_size > QR_MAX - 2 * opt.quiet_zone) return false;

	// construct larger output before encoding
	struct qr_output img;
	img.size = qr->output.size * opt.module_size + 2 * opt.quiet_zone;
	img.data_size = QR_DATA_SIZE(img.size);
	img.data = qr->alloc.malloc(img.size * img.size);
	if (!img.data) return false;

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
	qr_pos pos;
	for (pos.y = 0; pos.y < qr->output.size; pos.y++)
		for (pos.x = 0; pos.x < qr->output.size; pos.x++) {
			// skip if false and not inverted, or true and inverted
			if (qr_output_read(qr->output, pos) == false) continue;

			// write the square
			qr_pos off = QR_POS(pos.x * opt.module_size + opt.quiet_zone, pos.y * opt.module_size + opt.quiet_zone);
			qr_pos rel;
			for (rel.y = 0; rel.y < opt.module_size; rel.y++)
				for (rel.x = 0; rel.x < opt.module_size; rel.x++) {
					qr_output_write(&img, QR_POS(off.x + rel.x, off.y + rel.y), !opt.invert);
				}
		}


#define WRITE(...) fprintf(fp, __VA_ARGS__)

	if (opt.format & OUTPUT_IS_IMAGE) {

	} else {
		// output text
		if (opt.format == OUTPUT_HTML) {
			// HTML header
			float perc = 100.0 / img.size;
			perc = 20;
			WRITE("<!DOCTYPE html><html><head><title>QR Code</title><style>\n");
			WRITE("body {background-color:rgb(%u,%u,%u); color:rgb(%u,%u,%u); margin:0;}\n", opt.bg.r, opt.bg.g, opt.bg.b, opt.fg.r, opt.fg.g, opt.fg.b);
			WRITE("table {border-collapse:collapse;}\n");
			WRITE("td {width:%fpx;height:%fpx;}\n", perc, perc); // scale to fit
			WRITE("td.a {background-color:rgb(%u,%u,%u);}\n", opt.fg.r, opt.fg.g, opt.fg.b);
			WRITE("td.b {background-color:rgb(%u,%u,%u);}\n", opt.bg.r, opt.bg.g, opt.bg.b);
			WRITE("</style>\n");
			WRITE("<meta charset=\"UTF-8\">\n");
			WRITE("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n");
			WRITE("<meta name=\"darkreader-lock\">");
			WRITE("</head><body>\n");
			WRITE("<table>\n");
		}
		// loop pixels

		for (pos.y = 0; pos.y < img.size; pos.y++) {
			if (opt.format == OUTPUT_HTML)
				WRITE("<tr>");

			for (pos.x = 0; pos.x < img.size; pos.x++) {
				bool bit = qr_output_read(img, pos);
				switch (opt.format) {
					case OUTPUT_TEXT:
						WRITE(bit ? "##" : "  ");
						break;
					case OUTPUT_HTML:
						WRITE("<td class=\"%c\"></td>", bit ? 'a' : 'b');
						break;
					case OUTPUT_UNICODE:
						// full block or space
						WRITE(bit ? "\u2588\u2588" : "  ");
						break;
					case OUTPUT_UNICODE2X:;
						bool bit2 = qr_output_read(img, QR_POS(pos.x, pos.y + 1));
						WRITE(bit ? (bit2 ? "\u2588" : "\u2580") : (bit2 ? "\u2584" : " "));
						break;
					default:
						break;
				}
			}

			// newline
			if (opt.format == OUTPUT_HTML)
				WRITE("</tr>\n");
			else
				WRITE("\n");

			if (opt.format == OUTPUT_UNICODE2X) ++pos.y; // skip every other line
		}
		if (opt.format == OUTPUT_HTML) {
			// HTML footer
			WRITE("</table>\n");
			WRITE("</body></html>\n");
		}
	}

	FREE(img.data);

	return true;
}
