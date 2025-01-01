#include <stdio.h>
#include "../libqr/utf8.h"
#include "test.h"

int main() {
	struct {
		char *utf8;
		uint32_t codepoint;
	} str[] = {
	        {"\x00",             0x00   },
	        {"\x7F",             0x7F   },
	        {"\xc3\xbf",         0xFF   },
	        {"\xc8\xb6",         0x0236 },
	        {"\xea\xaf\x8d",     0xABCD },
	        {"\xf0\x9f\x98\x80", 0x1F600},
	        {"\xf0\x9f\x98\x81", 0x1F601},
	        {"\xf0\x9f\x98\x82", 0x1F602},
	        {"\u1F60",           0x1F60 },
	        {"\u0226",           0x0226 },
	        {"\uABCD",           0xABCD },
	        {"\u1127",           0x1127 },
	        {NULL,               0      }
    };

	int ret = 0;
	for (int i = 0; str[i].utf8; i++) {
		uint32_t actual, expected = str[i].codepoint;
		if (!(read_utf8((uint8_t *) str[i].utf8, &actual))) {
			FAIL_VAL("read_utf8(\"%s\")", expected, FMT_HEX, str[i].utf8);
			ret = 1;
			continue;
		}

		ASSERT(actual, ==, expected, FMT_HEX, ret = 1);
	}
	return ret;
}
