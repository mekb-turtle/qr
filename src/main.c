#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <locale.h>
#include <langinfo.h>
#include <unistd.h>
#include "qr.h"
#include "arg.h"
#include "output.h"
#include "util.h"

struct options {
	bool invert;
	enum output_format format;
	struct color fg, bg;
	module_t quiet_zone;
	module_t module_size;
	enum qr_ecl ecl;
	uint8_t version;
	enum qr_encoding encoding;
};

#define MODULE_SIZE_IMAGE_DEFAULT 8
#define MODULE_SIZE_TEXT_DEFAULT 1

void print_color_reason(enum parse_color_reason reason);

int main(int argc, char *argv[]) {
	const char *locale = setlocale(LC_ALL, "");
	if (!locale) {
		eprintf("Failed to set locale\n");
		return 1;
	}

	struct options options;
	memset(&options, 0, sizeof(options));

	char *opt_format = NULL,
	     *opt_background = NULL,
	     *opt_foreground = NULL,
	     *opt_quiet_zone = NULL,
	     *opt_module = NULL,
	     *opt_output = NULL,
	     *opt_ecl = NULL,
	     *opt_version = NULL,
	     *opt_encoding = NULL;

	bool force_terminal = false;

	bool invalid = false;
	int opt;

	// argument handling
	while ((opt = getopt_long(argc, argv, ":hVf:iB:F:q:m:o:e:v:E:S", (struct option[]){
	                                                                         {"help",       no_argument,       0, 'h'},
	                                                                         {"format",     required_argument, 0, 'f'},
	                                                                         {"invert",     no_argument,       0, 'i'},
	                                                                         {"background", required_argument, 0, 'B'},
	                                                                         {"foreground", required_argument, 0, 'F'},
	                                                                         {"quiet",      required_argument, 0, 'q'},
	                                                                         {"module",     required_argument, 0, 'm'},
	                                                                         {"output",     required_argument, 0, 'o'},
	                                                                         {"ecl",        required_argument, 0, 'e'},
	                                                                         {"version",    required_argument, 0, 'v'},
	                                                                         {"encoding",   required_argument, 0, 'E'},
	                                                                         {"terminal",   no_argument,       0, 'S'},
	                                                                         {0,            0,                 0, 0  }
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
\n\
-e --ecl: Error correction level (default: low)\n\
  Values: low/medium/quartile/high\n\
-v --version <1-40|auto>: QR code version (default: auto)\n\
-E --encoding <encoding>: Encoding to use (default: auto)\n\
  Values: auto/numeric/alphanumeric/byte/kanji\n\
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
						options.invert = true;
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
	if (optind != argc - 1 && invalid) {
	invalid_syntax:
		eprintf("Invalid usage, try --help\n");
		return 1;
	}

	// parse all options

	if (opt_format) {
		if (!parse_output_format(opt_format, &options.format)) goto invalid_syntax;
	} else {
		options.format = OUTPUT_TEXT;
	}

	if (opt_module) {
		if (!parse_u8(opt_module, &options.module_size, NULL)) goto invalid_syntax;
		if (options.module_size < 1) goto invalid_syntax;
	} else {
		options.module_size = options.format & OUTPUT_IS_IMAGE ? MODULE_SIZE_IMAGE_DEFAULT : MODULE_SIZE_TEXT_DEFAULT;
	}

	if (opt_quiet_zone) {
		if (!parse_u8(opt_quiet_zone, &options.quiet_zone, NULL)) goto invalid_syntax;
	} else {
		options.quiet_zone = QUIET_ZONE_DEFAULT;
	}

	if (opt_ecl) {
		if (!parse_ecl(opt_ecl, &options.ecl)) goto invalid_syntax;
	} else {
		options.ecl = ECL_LOW;
	}

	options.version = 0;
	if (opt_version) {
		if (strcasecmp(opt_version, "auto") != 0) {
			if (!parse_u8(opt_version, &options.version, NULL)) goto invalid_syntax;
			if (options.version > 40) goto invalid_syntax;
		}
	}

	enum parse_color_reason reason;

	reason = parse_color_fallback(opt_background, options.format, &options.bg, (struct color){255, 255, 255, 255});
	print_color_reason(reason);
	if (reason != COLOR_OK) return 1;

	reason = parse_color_fallback(opt_foreground, options.format, &options.fg, (struct color){0, 0, 0, 255});
	print_color_reason(reason);
	if (reason != COLOR_OK) return 1;

	if (opt_encoding) {
		if (!parse_encoding(opt_encoding, &options.encoding)) goto invalid_syntax;
	} else {
		options.encoding = ENC_AUTO;
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
	if (opt_output && strcmp(opt_output, "-") != 0) {
		fp = fopen(opt_output, "wb");
		if (!fp) {
			eprintf("%s: %s\n", opt_output, strerror(errno));
			return 1;
		}
	}

	if (options.format & OUTPUT_IS_IMAGE && !force_terminal && isatty(fileno(fp))) {
		eprintf("Refusing to write image to terminal\n");
	close_exit:
		if (fp != stdout) fclose(fp);
		return ret;
	}

	memset(&qr, 0, sizeof(qr)); // zero out the struct

	const char *error = NULL;
#define PRINT_ERROR(msg) eprintf("%s%s%s\n", msg, error ? ": " : "", error ? error : "")

	if (!qr_encode_utf8(&qr, alloc, str, options.encoding, options.version, options.ecl, &error)) {
		PRINT_ERROR("Failed to encode QR code");
		goto close_exit;
	}

	if (!qr_render(&qr, &error)) {
		PRINT_ERROR("Failed to render QR code");
	qr_exit:
		qr_close(&qr);
		goto close_exit;
	}

	struct output_options out_opt = {
	        .format = options.format,
	        .fg = options.fg,
	        .bg = options.bg,
	        .quiet_zone = options.quiet_zone,
	        .module_size = options.module_size,
	        .invert = options.invert};

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
