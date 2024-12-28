#include <stdio.h>
#include <string.h>
#include "../src/util.h"
#include "test.h"

static bool check_index_byte(size_t actual, size_t expected) {
	ASSERT(actual, ==, expected, FMT_INT, return false);
	return true;
}

static bool check_index_bit(uint8_t actual, uint8_t expected) {
	ASSERT(actual, ==, expected, FMT_INT, return false);
	return true;
}

static bool check_bytes(uint8_t *actual, uint8_t *expected, size_t len) {
	for (size_t i = 0; i < len; i++) {
		ASSERT(actual[i], ==, expected[i], FMT_HEX, return false);
	}
	return true;
}

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

int main() {
	int ret = 0;

	size_t len = 100;
	uint8_t data[len];
	memset(data, 0, len);

	size_t index_byte = 0;
	uint8_t index_bit = 0;

	uint32_t value = 0x35;
	uint8_t bits = 7;

	add_bits(data, len, &index_byte, &index_bit, value, bits);

	print_bits(data, 3);

	if (!check_index_byte(index_byte, 0)) ret = 1;
	if (!check_index_bit(index_bit, 7)) ret = 1;
	if (!check_bytes(data, (uint8_t[]){0x6a, 0, 0}, 3)) ret = 1;

	value = 0x55;
	bits = 7;

	add_bits(data, len, &index_byte, &index_bit, value, bits);

	print_bits(data, 3);

	// 14 bits
	if (!check_index_byte(index_byte, 1)) ret = 1;
	if (!check_index_bit(index_bit, 6)) ret = 1;
	if (!check_bytes(data, (uint8_t[]){0x6b, 0x54, 0}, 3)) ret = 1;

	value = 0;
	bits = 1;

	add_bits(data, len, &index_byte, &index_bit, value, bits);

	print_bits(data, 3);

	// 15 bits
	if (!check_index_byte(index_byte, 1)) ret = 1;
	if (!check_index_bit(index_bit, 7)) ret = 1;
	if (!check_bytes(data, (uint8_t[]){0x6b, 0x54, 0}, 3)) ret = 1;

	value = 1;
	bits = 1;

	add_bits(data, len, &index_byte, &index_bit, value, bits);

	print_bits(data, 3);

	// 16 bits
	if (!check_index_byte(index_byte, 2)) ret = 1;
	if (!check_index_bit(index_bit, 0)) ret = 1;
	if (!check_bytes(data, (uint8_t[]){0x6b, 0x55, 0}, 3)) ret = 1;

	return ret;
}
