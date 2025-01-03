#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "../../libqr/qr.h"
#include "../../libqr/util.h"
#include "../test.h"

void output_module(bool actual, bool expected) {
	// output the color of the module
	printf("\x1b[48;5;%cm", actual ? '0' : '7');
	if (actual == expected) {
		printf(" ");
	} else {
		// output an X if the values don't match
		printf("\x1b[38;5;1mX");
	}
	printf("\x1b[0m");
}

bool qr_bitmap_compare(struct qr_bitmap actual, struct qr_bitmap expected) {
	if (actual.size != expected.size) {
		printf("mismatched sizes: %d (A) vs %d (E)\n", actual.size, expected.size);
		return false;
	}

	bool exact_match = true;
	struct qr_pos pos;
	bool fits = actual.size < 80;

	printf("Actual");
	if (fits) printf("%*sExpected", actual.size < 4 ? 0 : actual.size - 4, "");
	for (pos.y = 0; pos.y < actual.size; ++pos.y) {
		printf("\n");
		for (pos.x = 0; pos.x < actual.size; ++pos.x) {
			bool a_val = qr_bitmap_read(actual, pos);
			bool e_val = qr_bitmap_read(expected, pos);
			if (a_val != e_val) exact_match = false;
			output_module(a_val, e_val);
		}
		if (!fits) continue;
		printf("  ");
		for (pos.x = 0; pos.x < actual.size; ++pos.x) {
			bool e_val = qr_bitmap_read(expected, pos);
			output_module(e_val, e_val);
		}
	}

	printf("\n");
	if (!fits) {
		printf("Expected\n");
		// doesn't fit on the screen if its side-by-side
		for (pos.y = 0; pos.y < actual.size; ++pos.y) {
			for (pos.x = 0; pos.x < actual.size; ++pos.x) {
				bool e_val = qr_bitmap_read(expected, pos);
				output_module(e_val, e_val);
			}
			printf("\n");
		}
	}

	return exact_match;
}

int main() {
	// find the qr_output.bin file
	const char *filename = __FILE__;
	char bin_filename[strlen(filename) + 8];
	strcpy(bin_filename, filename);
	char *ext = strrchr(bin_filename, '.');
	if (ext)
		strcpy(ext, ".bin");
	else
		strcat(bin_filename, ".bin");

	FILE *file = fopen(bin_filename, "rb");
	if (!file) {
		FAIL("fopen");
		return 77; // skip test
	}

	struct qr_alloc alloc = QR_ALLOC(malloc, realloc, free);
	const char *error = NULL;
	struct qr qr;
	memset(&qr, 0, sizeof(qr));

	int ret = 0;
	for (uint8_t version = 1; version <= 40; ++version) {
		// parse test data
		struct qr_bitmap test_bitmap;
		char c;
		size_t i = 0;
		test_bitmap.size = 0;
		for (; (c = fgetc(file)) != EOF; ++i) {
			printf("read: %c\n", c);
			if (c >= '0' && c <= '9') {
				test_bitmap.size *= 10;
				test_bitmap.size += c - '0';
				continue;
			}
			if (c == ' ') break;
			FAIL("unexpected character: %c", c);
			return 1;
		}

		bool match = true;
		ASSERT(test_bitmap.size, ==, (qr_t) QR_SIZE(version), FMT_INT, match = false);

		test_bitmap.data_size = QR_DATA_SIZE(test_bitmap.size);
		test_bitmap.data = alloc.malloc(test_bitmap.data_size);
		if (!test_bitmap.data) {
			FAIL("malloc");
			return 1;
		}

		if (fread(test_bitmap.data, 1, test_bitmap.data_size, file) != test_bitmap.data_size) {
			FAIL("fread");
			alloc.free(test_bitmap.data);
			return 1;
		}

		if (!match) {
			// skip this version since the sizes don't match
			ret = 1;
			continue;
		}

		qr_close(&qr);
		bool result = qr_encode_utf8(&qr, alloc, "test string...", QR_MODE_AUTO, version, QR_ECL_LOW, &error);
		if (!result) {
			ret = 1;
			alloc.free(test_bitmap.data);
			FAIL("qr_encode_utf8: %s", error);
			if (error) printf("error: %s\n", error);
			continue;
		}
		ASSERT(qr.ecl, ==, QR_ECL_LOW, FMT_INT, result = false);
		ASSERT(qr.mode, ==, QR_MODE_BYTE, FMT_INT, result = false);
		ASSERT(qr.version, ==, version, FMT_INT, result = false);
		ASSERT(qr.char_count, ==, 14, FMT_INT, result = false);
		if (!result) {
			ret = 1;
			continue;
		}
		if (!qr_prepare_data(&qr, &error)) {
			FAIL("qr_prepare_data: %s", error);
			ret = 1;
			continue;
		}
		if (!qr_render(&qr, &error, QR_MASK_AUTO)) {
			FAIL("qr_generate: %s", error);
			ret = 1;
			continue;
		}
		if (!qr_bitmap_compare(qr.output, test_bitmap)) {
			FAIL("qr_bitmap_compare");
			ret = 1;
			continue;
		}
	}
	qr_close(&qr);

	return ret;
}
