#include <stdio.h>
#include <string.h>
#include "../src/util.h"
#include "test.h"

int main() {
	int ret = 1;

	struct bit_buffer buf = {.size = 100, .byte_index = 0, .bit_index = 0};
	uint8_t data_[buf.size];
	buf.data = data_;
	memset(buf.data, 0, buf.size);

	buf.size = 1;

	uint32_t value = 0x135;
	uint8_t bits = 9;

	// not enough space in buffer
	if (add_bits(&buf, value, bits)) {
		UNEXPECTED_PASS("add_bits");
		ret = 0;
	}
	EXPECTED_FAIL("add_bits");

	buf.size = 0;
	buf.byte_index = 0;
	buf.bit_index = 0;
	memset(buf.data, 0, buf.size);

	value = 0x55;
	bits = 7;

	if (add_bits(&buf, value, bits)) {
		UNEXPECTED_PASS("add_bits");
		ret = 0;
	}
	EXPECTED_FAIL("add_bits");

	return ret;
}
