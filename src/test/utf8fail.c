#include <stdio.h>
#include "../libqr/utf8.h"
#include "test.h"

int main() {
	char *str[] = {
	        "\x80",
	        "\xe2\xa9",
	        "\xf0\x9f\x98\x00",
	        "\xf0\xe0",
	        "\xc0\xae",         // overlong
	        "\xe0\x80\xae",     // overlong
	        "\xf0\x80\x80\xae", // overlong
	        NULL};

	int ret = 1;
	for (int i = 0; str[i]; i++) {
		uint32_t actual;
		if (!(read_utf8((uint8_t *) str[i], &actual))) {
			EXPECTED_FAIL("read_utf8(\"%s\")", str[i]);
			continue;
		}

		UNEXPECTED_PASS_ACTUAL("read_utf8(\"%s\")", actual, FMT_HEX, str[i]);
		ret = 0;
	}
	return ret;
}
