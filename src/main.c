#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdbool.h>
#include <string.h>
#include <locale.h>
#include "qr.h"
#include "arg.h"
#include "output.h"
#define eprintf(...) fprintf(stderr, __VA_ARGS__)

struct options {
	enum output_format format;
	struct color_pair colors;
	unsigned int quiet;
	unsigned int module_size;
	enum qr_ecl ecl;
	unsigned int version;
	enum qr_encoding encoding;
};

int main(int argc, char *argv[]) {
	bool format_set = false;
	struct options options;
	memset(&options, 0, sizeof(options));

	char *opt_format = NULL,
	     *opt_background = NULL,
	     *opt_foreground = NULL,
	     *opt_quiet = NULL,
	     *opt_module = NULL,
	     *opt_output = NULL,
	     *opt_ecl = NULL,
	     *opt_version = NULL,
	     *opt_encoding = NULL;

	bool invalid = false;
	int opt;

	// argument handling
	while ((opt = getopt_long(argc, argv, ":hVf:B:F:q:m:o:e:v:E:", (struct option[]){
	                                                                       {"help",       no_argument,       0, 'h'},
	                                                                       {"format",     required_argument, 0, 'f'},
	                                                                       {"background", required_argument, 0, 'B'},
	                                                                       {"foreground", required_argument, 0, 'F'},
	                                                                       {"quiet",      required_argument, 0, 'q'},
	                                                                       {"module",     required_argument, 0, 'm'},
	                                                                       {"output",     required_argument, 0, 'o'},
	                                                                       {"ecl",        required_argument, 0, 'e'},
	                                                                       {"version",    required_argument, 0, 'v'},
	                                                                       {"encoding",   required_argument, 0, 'E'},
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
    Image: png, jpeg, gif, bmp, ff\n\
    Text: text, html, unicode, unicode2x, ansi\n\
\n\
-B --background <color>\n\
-F --foreground <color>\n\
  Values: r,g,b for image, 0-255 for ansi\n\
\n\
-q --quiet <modules>: Margin around code (default: %i)\n\
-m --module <pixels>: Size of each module in pixels/characters (default: 8 for image, 1 for text)\n\
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
				       QR_MARGIN_DEFAULT);
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
						opt_format = optarg;
						break;
					case 'B':
						opt_background = optarg;
						break;
					case 'F':
						opt_foreground = optarg;
						break;
					case 'q':
						opt_quiet = optarg;
						break;
					case 'm':
						opt_module = optarg;
						break;
					case 'o':
						opt_output = optarg;
						break;
					case 'e':
						opt_ecl = optarg;
						break;
					case 'v':
						opt_version = optarg;
						break;
					case 'E':
						opt_encoding = optarg;
						break;
					default:
						invalid = true;
						break;
				}
		}
	}

	if (!opt_format) invalid = true;

	// parse all options

	if (!parse_output_format(opt_format, &options.format)) invalid = true;

	if (opt_quiet) {
		if (!parse_uint(opt_quiet, &options.quiet, NULL)) invalid = true;
	} else {
		options.quiet = QR_MARGIN_DEFAULT;
	}

	if (opt_module) {
		if (!parse_uint(opt_module, &options.module_size, NULL)) invalid = true;
		if (options.module_size < 1) invalid = true;
	} else {
		options.module_size = 1;
	}

	if (opt_ecl) {
		if (!parse_ecl(opt_ecl, &options.ecl)) invalid = true;
	} else {
		options.ecl = ECL_LOW;
	}

	options.version = 0;
	if (opt_version) {
		if (strcmp(opt_version, "auto") != 0) {
			if (!parse_uint(opt_version, &options.version, NULL)) invalid = true;
			if (options.version > 40) invalid = true;
		}
	}

	if (!parse_color_fallback(opt_background, options.format, &options.colors.bg, (struct color_rgb){0}, 0)) invalid = true;

	if (!parse_color_fallback(opt_foreground, options.format, &options.colors.fg, (struct color_rgb){255, 255, 255}, 15)) invalid = true;

	if (opt_encoding) {
		if (!parse_encoding(opt_encoding, &options.encoding)) invalid = true;
	} else {
		options.encoding = ENC_AUTO;
	}

	if (optind != argc - 1 || invalid) {
		eprintf("Invalid usage, try --help\n");
		return 1;
	}

	const char *locale = setlocale(LC_ALL, NULL);
	printf("Locale: %s\n", locale ? locale : "none");

	const char *str = argv[argc];

	return 0;
}
