#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <locale.h>
#include <langinfo.h>
#include <unistd.h>
#include "../libqr/qr.h"
#include "arg.h"
#include "output.h"

#define eprintf(...) fprintf(stderr, __VA_ARGS__)

#define MODULE_SIZE_IMAGE_DEFAULT 8
#define MODULE_SIZE_TEXT_DEFAULT 1

void print_color_reason(enum parse_color_reason reason);

int main(int argc, char *argv[]) {
	const char *locale = setlocale(LC_ALL, "");
	if (!locale) {
		eprintf("Failed to set locale\n");
		return 1;
	}

	bool invert = false;
	enum output_format format;
	struct color fg, bg;
	module_t quiet_zone;
	module_t module_size;
	enum qr_ecl ecl;
	bool boost_ecl = true;
	uint8_t version;
	enum qr_mode encoding;
	bool force_terminal = false;
	uint8_t mask;

	char *opt_format = NULL,
	     *opt_background = NULL,
	     *opt_foreground = NULL,
	     *opt_quiet_zone = NULL,
	     *opt_module = NULL,
	     *opt_output = NULL,
	     *opt_ecl = NULL,
	     *opt_version = NULL,
	     *opt_encoding = NULL,
	     *opt_mask = NULL;

	bool invalid = false;
	int opt;

	// argument handling
	while ((opt = getopt_long(argc, argv, ":hVf:iB:F:q:m:o:e:Nv:E:SM:", (struct option[]){
	                                                                            {"help",         no_argument,       0, 'h'},
	                                                                            {"format",       required_argument, 0, 'f'},
	                                                                            {"invert",       no_argument,       0, 'i'},
	                                                                            {"background",   required_argument, 0, 'B'},
	                                                                            {"foreground",   required_argument, 0, 'F'},
	                                                                            {"quiet",        required_argument, 0, 'q'},
	                                                                            {"module",       required_argument, 0, 'm'},
	                                                                            {"output",       required_argument, 0, 'o'},
	                                                                            {"ecl",          required_argument, 0, 'e'},
	                                                                            {"no-boost-ecl", no_argument,       0, 'N'},
	                                                                            {"version",      required_argument, 0, 'v'},
	                                                                            {"encoding",     required_argument, 0, 'E'},
	                                                                            {"terminal",     no_argument,       0, 'S'},
	                                                                            {"mask",         required_argument, 0, 'M'},
	                                                                            {0,              0,                 0, 0  }
    },
	                          NULL)) != -1) {
		switch (opt) {
			case 'h':
				printf("Usage: %s [OPTION]... <TEXT>\n", PROJECT_NAME);
				printf("\
-h --help: Shows help text\n\
-V: Shows the version of the program\n\
\n\
-f --format <format>: Specify output format to use (default: text)\n\
  Values:\n\
    Image: png, bmp, tga, hdr, jpg, ff\n\
    Text: text, html, unicode, unicode2x\n\
-i --invert: Flip the colors of the output\n\
\n\
-B --background <color>\n\
-F --foreground <color>\n\
  Values: r,g,b or r,g,b,a\n\
\n\
-q --quiet <modules>: Margin around code in modules (default: %i)\n\
-m --module <pixels>: Size of each module in pixels/characters (default: %i for image, %i for text)\n\
\n\
-o --output: Specify the output file, - for stdin\n\
-S --terminal: Force output to terminal\n\
\n\
-e --ecl: Error correction level (default: low)\n\
  Values: low/medium/quartile/high\n\
-N --no-boost-ecl: Disable automatic error correction level boost\n\
-v --version <1-40|auto>: Minimum QR code version\n\
-E --encoding <encoding>: Encoding to use (default: auto)\n\
  Values: auto/numeric/alphanumeric/byte/kanji\n\
-M --mask <0-7|auto>: Force mask to use\n\
\n\
",
				       QUIET_ZONE_DEFAULT, MODULE_SIZE_IMAGE_DEFAULT, MODULE_SIZE_TEXT_DEFAULT);
				return 0;
			case 'V':
				printf("%s %s\n", PROJECT_NAME, PROJECT_VERSION);
#ifdef PROJECT_URL
				printf("See more at %s\n", PROJECT_URL);
#endif
				return 0;
			default:
				if (invalid) break;
				switch (opt) {
					case 'f':
						if (!optarg) invalid = true;
						opt_format = optarg;
						break;
					case 'i':
						invert = true;
						break;
					case 'B':
						if (!optarg) invalid = true;
						opt_background = optarg;
						break;
					case 'F':
						if (!optarg) invalid = true;
						opt_foreground = optarg;
						break;
					case 'q':
						if (!optarg) invalid = true;
						opt_quiet_zone = optarg;
						break;
					case 'm':
						if (!optarg) invalid = true;
						opt_module = optarg;
						break;
					case 'o':
						if (!optarg) invalid = true;
						opt_output = optarg;
						break;
					case 'e':
						if (!optarg) invalid = true;
						opt_ecl = optarg;
						break;
					case 'N':
						boost_ecl = false;
						break;
					case 'v':
						if (!optarg) invalid = true;
						opt_version = optarg;
						break;
					case 'E':
						if (!optarg) invalid = true;
						opt_encoding = optarg;
						break;
					case 'S':
						force_terminal = true;
						break;
					case 'M':
						if (!optarg) invalid = true;
						opt_mask = optarg;
						break;
					default:
						invalid = true;
						break;
				}
		}
	}

	if (optind == argc) {
		eprintf("No text specified\n");
		return 1;
	}
	if (optind != argc - 1 || invalid) {
	invalid_syntax:
		eprintf("Invalid usage, try --help\n");
		return 1;
	}

	bool is_stdout = !opt_output || strcmp(opt_output, "-") == 0;

	// parse all options

	if (opt_format) {
		if (!parse_output_format(opt_format, &format)) {
			eprintf("Invalid output format\n");
			return 1;
		}
	} else {
		format = OUTPUT_TEXT;
		if (!is_stdout) {
			// detect output format from file extension
			const char *ext = strrchr(opt_output, '.');
			if (!ext || !parse_output_format(ext + 1, &format)) {
				eprintf("No output format specified\n");
				return 1;
			}
		}
	}

	if (opt_module) {
		if (!parse_u8(opt_module, &module_size, NULL) || module_size < 1) {
			eprintf("Invalid module size, should be at least 1\n");
			return 1;
		}
	} else {
		module_size = format & OUTPUT_IS_IMAGE ? MODULE_SIZE_IMAGE_DEFAULT : MODULE_SIZE_TEXT_DEFAULT;
	}

	if (opt_quiet_zone) {
		if (!parse_u8(opt_quiet_zone, &quiet_zone, NULL)) {
			eprintf("Invalid quiet zone size\n");
			return 1;
		}
	} else {
		quiet_zone = QUIET_ZONE_DEFAULT;
	}

	if (opt_ecl) {
		if (!parse_ecl(opt_ecl, &ecl)) {
			eprintf("Invalid error correction level\n");
			return 1;
		}
	} else {
		ecl = QR_ECL_LOW;
	}

	if (!boost_ecl) {
		ecl |= QR_ECL_NO_BOOST;
	}

	version = QR_VERSION_AUTO;
	if (opt_version && !MATCH(opt_version, "auto")) {
		if (!parse_u8(opt_version, &version, NULL) || version < QR_VERSION_MIN || version > QR_VERSION_MAX) {
			eprintf("Invalid version, should be between 1-40 inclusive or 'auto'\n");
		}
	}

	mask = QR_MASK_AUTO;
	if (opt_mask && !MATCH(opt_mask, "auto")) {
		if (!parse_u8(opt_mask, &mask, NULL) || mask > QR_MASK_MAX) {
			eprintf("Invalid mask value, should be between 0-7 inclusive\n");
			return 1;
		}
	}

	enum parse_color_reason reason;

	reason = parse_color_fallback(opt_background, format, &bg, (struct color){255, 255, 255, 255});
	print_color_reason(reason);
	if (reason != COLOR_OK) return 1;

	reason = parse_color_fallback(opt_foreground, format, &fg, (struct color){0, 0, 0, 255});
	print_color_reason(reason);
	if (reason != COLOR_OK) return 1;

	if (opt_encoding) {
		if (!parse_encoding(opt_encoding, &encoding)) goto invalid_syntax;
	} else {
		encoding = QR_MODE_AUTO;
	}

	struct qr_alloc alloc = QR_ALLOC(malloc, realloc, free);

	int ret = 1;

	const char *str = argv[optind];

	const char *codeset = nl_langinfo(CODESET);
	if (strcmp(codeset, "UTF-8") != 0) {
		eprintf("Warning: Input encoding is not UTF-8, output may be incorrect\n");
		// TODO: convert to UTF-8
	}

	struct qr qr;

	FILE *fp = stdout;
	if (!is_stdout) {
		fp = fopen(opt_output, "wb");
		if (!fp) {
			eprintf("%s: %s\n", opt_output, strerror(errno));
			return 1;
		}
	}

	if (format & OUTPUT_IS_IMAGE && !force_terminal && isatty(fileno(fp))) {
		eprintf("Refusing to write image to terminal\n");
	close_exit:
		if (fp != stdout) fclose(fp);
		return ret;
	}

	memset(&qr, 0, sizeof(qr)); // zero out the struct

	const char *error = NULL;
#define PRINT_ERROR(msg) eprintf("%s%s%s\n", msg, error ? ": " : "", error ? error : "")

	if (!qr_encode_utf8(&qr, alloc, str, encoding, version, ecl, &error)) {
		PRINT_ERROR("Failed to encode data for QR code");
		goto close_exit;
	}

	if (!qr_prepare_data(&qr, &error)) {
		PRINT_ERROR("Failed to prepare QR code data");
		goto qr_exit;
	}

	if (!qr_render(&qr, &error, mask)) {
		PRINT_ERROR("Failed to render QR code");
	qr_exit:
		qr_close(&qr);
		goto close_exit;
	}

	struct output_options out_opt = {
	        .format = format,
	        .fg = fg,
	        .bg = bg,
	        .quiet_zone = quiet_zone,
	        .module_size = module_size,
	        .invert = invert};

	if (!write_output(fp, &qr, out_opt, &error)) {
		PRINT_ERROR("Failed to write output");
		goto qr_exit;
	}

	ret = 0; // success
	goto qr_exit;
}

void print_color_reason(enum parse_color_reason reason) {
	switch (reason) {
		case COLOR_SYNTAX:
			eprintf("Invalid color syntax\n");
			break;
		case COLOR_NO_COLOR:
			eprintf("Color not supported for this format\n");
			break;
		case COLOR_NO_ALPHA:
			eprintf("Alpha not supported for this format\n");
			break;
		case COLOR_OK:
			break;
	}
}
