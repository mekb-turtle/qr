#include <stdio.h>
#include "../src/utf8.h"
int main() {
	char *str[] = {
	        "\x80",
	        "\xe2\xa9",
	        "\xf0\x9f\x98\x00",
	        "\xf0\xe0",
			"\xc0\xae", // overlong
			"\xe0\x80\xae", // overlong
			"\xf0\x80\x80\xae", // overlong
	        NULL};

	int ret = 1;
	for (int i = 0; str[i]; i++) {
		uint32_t actual;
		if (!(read_utf8((uint8_t *) str[i], &actual))) {
			printf("read_utf8(\"%s\") failed as expected\n", str[i]);
			continue;
		}
		printf("Error: read_utf8(\"%s\") = 0x%X, expected failure\n",
		       str[i], actual);
		ret = 0;
	}
	return ret;
}
