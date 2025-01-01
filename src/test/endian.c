#include <stdio.h>
#include <stdint.h>
#include "../cli/endian.h"
#include "test.h"

int main() {
	struct {
		uint32_t from, expected;
	} val32[] = {
	        {0x12345678, 0x78563412},
	        {0x87654321, 0x21436587},
	        {0x00000000, 0x00000000},
	        {0xFFFFFFFF, 0xFFFFFFFF},
	        {0x00000001, 0x01000000},
	        {0x01000000, 0x00000001},
	};

	struct {
		uint16_t from, expected;
	} val16[] = {
	        {0x1234, 0x3412},
	        {0x4321, 0x2143},
	        {0x0000, 0x0000},
	        {0xFFFF, 0xFFFF},
	        {0x0001, 0x0100},
	        {0x0100, 0x0001},
	};

	int ret = 0;
#define LEN(val) (sizeof(val) / sizeof(val[0]))
	for (size_t i = 0; i < LEN(val32); i++) {
		ASSERT(endian32_swap(val32[i].from), ==, val32[i].expected, FMT_HEX, ret = 1);
	}
	for (size_t i = 0; i < LEN(val16); i++) {
		ASSERT(endian16_swap(val16[i].from), ==, val16[i].expected, FMT_HEX, ret = 1);
	}

	printf("current system is %s-endian\n", is_little_endian() ? "little" : "big");
	return ret;
}
