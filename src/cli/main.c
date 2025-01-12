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

	bool invert = false, boost_ecl = true, force_terminal = false;
	enum output_format format;
	struct color fg, bg;
	qr_t quiet_zone, module_size;
	enum qr_ecl ecl;
	uint8_t version;
	enum qr_mode encoding;
	uint8_t mask;

	// store all options as strings for later use
	// this is to avoid having to parse them multiple times if they are specified multiple times
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
	while ((opt = getopt_long(argc, argv, ":hVf:liB:F:q:m:o:e:Nv:E:SM:", (struct option[]){
	                                                                             {"help",         no_argument,       0, 'h'},
	                                                                             {"format",       required_argument, 0, 'f'},
	                                                                             {"formats",      no_argument,       0, 'l'},
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
-l --formats: Shows a list of supported formats\n\
\n\
-f --format <format>: Specify output format to use (default: text)\n\
-i --invert: Flip the colors of the output\n\
\n\
-B --background <r,g,b|r,g,b,a>\n\
-F --foreground <r,g,b|r,g,b,a>\n\
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
",
				       QUIET_ZONE_DEFAULT, MODULE_SIZE_IMAGE_DEFAULT, MODULE_SIZE_TEXT_DEFAULT);
				return 0;
			case 'V':
				printf("%s %s\n", PROJECT_NAME, PROJECT_VERSION);
#ifdef PROJECT_URL
				printf("See more at %s\n", PROJECT_URL);
#endif
				return 0;
			case 'l':
				printf("Supported formats:\n");
				const struct format_string *prev = NULL, *fmt;

				// find longest pretty name
				int max_pretty_name = 0;
				for (const struct format_string *fmt = formats; fmt->arg_names; ++fmt) {
					if (!fmt->pretty_name) continue;
					size_t len = strlen(fmt->pretty_name);
					if (len > INT_MAX) len = INT_MAX;
					if ((int) len > max_pretty_name) max_pretty_name = len;
				}

				printf("Text:\n");
				for (fmt = formats; fmt->arg_names; prev = fmt, ++fmt) {
					if (prev) {
						// print header for new format type
						if (OUTPUT_IS_BINARY(fmt->format) && !OUTPUT_IS_BINARY(prev->format)) printf("Binary:\n");
						if (OUTPUT_IS_IMAGE(fmt->format) && !OUTPUT_IS_IMAGE(prev->format)) printf("Image:\n");
					}

					printf(" - ");

					// print pretty name
					if (fmt->pretty_name) printf("%-*s : ", max_pretty_name, fmt->pretty_name);

					// print arg names
					for (const char *const *arg = fmt->arg_names; *arg; ++arg) {
						if (arg != fmt->arg_names) printf(", ");
						printf("%s", *arg);
					}
					printf("\n");
				}
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

	if (optind != argc - 1 || invalid) {
		eprintf("Invalid usage, try --help\n");
		return 1;
	}

	if (optind == argc) {
		eprintf("No text specified\n");
		invalid = true;
	}

	bool is_stdout = !opt_output || strcmp(opt_output, "-") == 0;
	bool format_invalid = false;

	// parse all options

	if (opt_format) {
		if (!parse_output_format(opt_format, &format)) {
			eprintf("Invalid output format\n");
			format_invalid = invalid = true;
		}
	} else {
		format = OUTPUT_TEXT;
		if (!is_stdout) {
			// detect output format from file extension
			const char *ext = strrchr(opt_output, '.');
			if (!ext || !parse_output_format(ext + 1, &format)) {
				eprintf("No output format specified\n");
				format_invalid = invalid = true;
			}
		}
	}

	if (opt_module) {
		if (!parse_u32(opt_module, &module_size, NULL) || module_size < 1) {
			eprintf("Invalid module size, should be at least 1\n");
			invalid = true;
		}
	} else {
		module_size = OUTPUT_IS_IMAGE(format) ? MODULE_SIZE_IMAGE_DEFAULT : MODULE_SIZE_TEXT_DEFAULT;
	}

	if (opt_quiet_zone) {
		if (!parse_u32(opt_quiet_zone, &quiet_zone, NULL)) {
			eprintf("Invalid quiet zone size\n");
			invalid = true;
		}
	} else
		quiet_zone = QUIET_ZONE_DEFAULT;

	if (opt_ecl) {
		if (!parse_ecl(opt_ecl, &ecl)) {
			eprintf("Invalid error correction level\n");
			invalid = true;
		}
	} else
		ecl = QR_ECL_MEDIUM;

	if (boost_ecl) ecl |= QR_ECL_BOOST;

	version = QR_VERSION_AUTO;
	if (opt_version && !MATCH(opt_version, "auto")) {
		if (!parse_u8(opt_version, &version, NULL) || version < QR_VERSION_MIN || version > QR_VERSION_MAX) {
			eprintf("Invalid version, should be between 1-40 inclusive or 'auto'\n");
			invalid = true;
		}
	}

	mask = QR_MASK_AUTO;
	if (opt_mask && !MATCH(opt_mask, "auto")) {
		if (!parse_u8(opt_mask, &mask, NULL) || mask > QR_MASK_MAX) {
			eprintf("Invalid mask value, should be between 0-7 inclusive\n");
			invalid = true;
		}
	}

	enum parse_color_reason reason;

	if (!format_invalid) {
		reason = parse_color_fallback(opt_background, format, &bg, (struct color){255, 255, 255, 255});
		print_color_reason(reason);
		if (reason != COLOR_OK) invalid = true;

		reason = parse_color_fallback(opt_foreground, format, &fg, (struct color){0, 0, 0, 255});
		print_color_reason(reason);
		if (reason != COLOR_OK) invalid = true;
	}

	if (opt_encoding) {
		if (!parse_encoding(opt_encoding, &encoding)) {
			eprintf("Invalid encoding\n");
			invalid = true;
		}
	} else {
		encoding = QR_MODE_AUTO;
	}

	if (invalid) return 1;

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

	if (OUTPUT_IS_BINARY(format) && !force_terminal && isatty(fileno(fp))) {
		eprintf("Refusing to write binary file to terminal\n");
		eprintf("Use --terminal to override\n");
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

	if (!qr_encode_prepare(&qr, &error)) {
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
			eprintf("Color is not supported for this format\n");
			eprintf("Images and HTML formats support colors\n");
			break;
		case COLOR_NO_ALPHA:
			eprintf("Alpha channel is not supported for this format\n");
			break;
		case COLOR_OK:
			break;
	}
}
