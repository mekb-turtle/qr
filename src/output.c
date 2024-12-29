#include "output.h"
#include "arg.h"
#include <stddef.h>
#include <string.h>

bool write_output(const char *filename, const struct qr_output *output, enum output_format format, struct color_pair colors) {
	(void) filename;
	(void) output;
	(void) format;
	(void) colors;
	return false;
}
