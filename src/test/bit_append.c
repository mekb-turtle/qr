#include <stdio.h>
#include <string.h>
#include "../libqr/util.h"
#include "test.h"

static void print_bits(uint8_t *data, size_t len) {
	for (size_t i = 0; i < len; i++) {
		if (i > 0) printf(" ");
		for (size_t j = 0; j < 8; j++) {
			// print bit
			printf("%d", (data[i] >> (7 - j)) & 1);
		}
	}
	printf("\n");
}

static bool check_buf(struct bit_buffer buf, size_t expected_bit_count, uint8_t *expected_data, size_t len) {
	print_bits(buf.data, 3);

	bool ret = true;

	ASSERT(buf.byte_index, ==, expected_bit_count / 8, FMT_INT, ret = false);
	ASSERT(buf.bit_index, ==, expected_bit_count % 8, FMT_INT, ret = false);

	for (size_t i = 0; i < len; i++) {
		ASSERT(((uint8_t *) buf.data)[i], ==, expected_data[i], FMT_HEX, ret = false);
	}

	return ret;
}

int main() {
	int ret = 0;

	struct bit_buffer buf = {.size = 100, .byte_index = 0, .bit_index = 0};
	uint8_t data_[buf.size];
	buf.data = data_;
	memset(buf.data, 0, buf.size);

	uint32_t value = 0x35;
	uint8_t bits = 7;

#define ADD_BITS                            \
	{                                       \
		if (!add_bits(&buf, value, bits)) { \
			FAIL("add_bits");               \
			ret = 1;                        \
		}                                   \
	}

	ADD_BITS;

	if (!check_buf(buf, 7, (uint8_t[]){0x6a, 0, 0}, 3)) ret = 1;

	value = 0x55;
	bits = 7;

	ADD_BITS;

	if (!check_buf(buf, 14, (uint8_t[]){0x6b, 0x54, 0}, 3)) ret = 1;

	value = 0;
	bits = 1;

	ADD_BITS;

	if (!check_buf(buf, 15, (uint8_t[]){0x6b, 0x54, 0}, 3)) ret = 1;

	value = 1;
	bits = 1;

	ADD_BITS;

	if (!check_buf(buf, 16, (uint8_t[]){0x6b, 0x55, 0}, 3)) ret = 1;

	return ret;
}
