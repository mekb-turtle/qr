#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdbool.h>
#include "qr.h"
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
				invalid = true;
				break;
		}
	}

	if (optind != argc - 1 || invalid) {
		eprintf("Invalid usage, try --help\n");
		return 1;
	}

	const char *str = argv[argc];

	return 0;
}
