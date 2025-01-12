#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "../libqr/qr.h"
#include "../libqr/util.h"
#include "test.h"

int main() {
	int ret = 0;
	struct qr_alloc alloc = QR_ALLOC(malloc, realloc, free);
	const char *error = NULL;
	struct qr qr;
	memset(&qr, 0, sizeof(qr));
	bool result = qr_encode_utf8(&qr, alloc, "HELLO WORLD", QR_MODE_AUTO, QR_VERSION_AUTO, QR_ECL_LOW, &error);
	if (!result) {
		FAIL("qr_encode_utf8");
		if (error) printf("error: %s\n", error);
	} else {
		bool match = true;
		ASSERT(QR_ECL(qr.ecl), ==, QR_ECL_LOW, FMT_INT, match = false);
		ASSERT(qr.mode, ==, QR_MODE_ALPHANUMERIC, FMT_INT, match = false);
		ASSERT(qr.version, ==, 1, FMT_INT, match = false);
		ASSERT(qr.char_count, ==, 11, FMT_INT, match = false);
		if (!match) ret = 1;
		else {
			result = qr_encode_prepare(&qr, &error);
			if (!result) {
				FAIL("qr_encode_prepare");
				if (error) printf("error: %s\n", error);
				ret = 1;
			} else {
				// thanks to https://www.nayuki.io/page/creating-a-qr-code-step-by-step
				uint8_t expected[] = {0x20, 0x5B, 0x0B, 0x78, 0xD1, 0x72, 0xDC, 0x4D, 0x43, 0x40, 0xEC, 0x11, 0xEC, 0x11, 0xEC, 0x11, 0xEC, 0x11, 0xEC};
#define LEN(x) (sizeof(x) / sizeof(x[0]))
				ASSERT(qr.data.byte_index, ==, LEN(expected), FMT_INT, ret = 1);
				if (qr.data.byte_index == LEN(expected)) {
					for (size_t i = 0; i < qr.data.byte_index; i++) {
						ASSERT(((uint8_t *) qr.data.data)[i], ==, expected[i], FMT_HEX, ret = 1);
					}
				}
				uint8_t expected_i[] = {0x20, 0x5B, 0x0B, 0x78, 0xD1, 0x72, 0xDC, 0x4D, 0x43, 0x40, 0xEC, 0x11, 0xEC, 0x11, 0xEC, 0x11, 0xEC, 0x11, 0xEC, 0xD1, 0xEF, 0xC4, 0xCF, 0x4E, 0xC3, 0x6D};
				ASSERT(qr.data_i.byte_index, ==, LEN(expected_i), FMT_INT, ret = 1);
				if (qr.data_i.byte_index == LEN(expected_i)) {
					for (size_t i = 0; i < qr.data_i.byte_index; i++) {
						ASSERT(((uint8_t *) qr.data_i.data)[i], ==, expected_i[i], FMT_HEX, ret = 1);
					}
				}
			}
		}
	}

	qr_close(&qr);

	// test with a larger string
	result = qr_encode_utf8(&qr, alloc, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz", QR_MODE_AUTO, 5, QR_ECL_MEDIUM, &error);
	if (!result) {
		FAIL("qr_encode_utf8");
		if (error) printf("error: %s\n", error);
	} else {
		ASSERT(QR_ECL(qr.ecl), ==, QR_ECL_MEDIUM, FMT_INT, ret = 1);
		ASSERT(qr.mode, ==, QR_MODE_BYTE, FMT_INT, ret = 1);
		ASSERT(qr.version, ==, 10, FMT_INT, ret = 1);
		ASSERT(qr.char_count, ==, 182, FMT_INT, ret = 1);
	}

	qr_close(&qr);

	// test with numeric mode at a higher verion
	result = qr_encode_utf8(&qr, alloc, "1234567890", QR_MODE_AUTO, 30, QR_ECL_HIGH, &error);
	if (!result) {
		FAIL("qr_encode_utf8");
		if (error) printf("error: %s\n", error);
	} else {
		ASSERT(qr.ecl, ==, QR_ECL_HIGH, FMT_INT, ret = 1);
		ASSERT(qr.mode, ==, QR_MODE_NUMERIC, FMT_INT, ret = 1);
		ASSERT(qr.version, ==, 30, FMT_INT, ret = 1);
		ASSERT(qr.char_count, ==, 10, FMT_INT, ret = 1);
	}

	qr_close(&qr);

	// test with a string that requires kanji mode
	result = qr_encode_utf8(&qr, alloc, "ぁあぃいぅうぇえぉおかがきぎく", QR_MODE_AUTO, QR_VERSION_AUTO, QR_ECL_HIGH, &error);

	if (!result) {
		FAIL("qr_encode_utf8");
		if (error) printf("error: %s\n", error);
	} else {
		ASSERT(qr.ecl, ==, QR_ECL_HIGH, FMT_INT, ret = 1);
		ASSERT(qr.mode, ==, QR_MODE_KANJI, FMT_INT, ret = 1);
		ASSERT(qr.version, ==, 3, FMT_INT, ret = 1);
		ASSERT(qr.char_count, ==, 15, FMT_INT, ret = 1);
	}

	qr_close(&qr);

	// test empty strings
	result = qr_encode_utf8(&qr, alloc, "", QR_MODE_AUTO, 0, QR_ECL_HIGH, &error);

	if (!result) {
		FAIL("qr_encode_utf8");
		if (error) printf("error: %s\n", error);
	} else {
		ASSERT(qr.ecl, ==, QR_ECL_HIGH, FMT_INT, ret = 1);
		ASSERT(qr.version, ==, 1, FMT_INT, ret = 1);
		ASSERT(qr.char_count, ==, 0, FMT_INT, ret = 1);
	}

	qr_close(&qr);

	result = qr_encode_utf8(&qr, alloc, "", QR_MODE_NUMERIC, QR_VERSION_AUTO, QR_ECL_HIGH, &error);

	if (!result) {
		FAIL("qr_encode_utf8");
		if (error) printf("error: %s\n", error);
	} else {
		ASSERT(qr.ecl, ==, QR_ECL_HIGH, FMT_INT, ret = 1);
		ASSERT(qr.mode, ==, QR_MODE_NUMERIC, FMT_INT, ret = 1);
		ASSERT(qr.version, ==, 1, FMT_INT, ret = 1);
		ASSERT(qr.char_count, ==, 0, FMT_INT, ret = 1);
	}

	qr_close(&qr);

	result = qr_encode_utf8(&qr, alloc, "", QR_MODE_ALPHANUMERIC, QR_VERSION_AUTO, QR_ECL_HIGH, &error);

	if (!result) {
		FAIL("qr_encode_utf8");
		if (error) printf("error: %s\n", error);
	} else {
		ASSERT(qr.ecl, ==, QR_ECL_HIGH, FMT_INT, ret = 1);
		ASSERT(qr.mode, ==, QR_MODE_ALPHANUMERIC, FMT_INT, ret = 1);
		ASSERT(qr.version, ==, 1, FMT_INT, ret = 1);
		ASSERT(qr.char_count, ==, 0, FMT_INT, ret = 1);
	}

	qr_close(&qr);

	result = qr_encode_utf8(&qr, alloc, "", QR_MODE_BYTE, QR_VERSION_AUTO, QR_ECL_HIGH, &error);

	if (!result) {
		FAIL("qr_encode_utf8");
		if (error) printf("error: %s\n", error);
	} else {
		ASSERT(qr.ecl, ==, QR_ECL_HIGH, FMT_INT, ret = 1);
		ASSERT(qr.mode, ==, QR_MODE_BYTE, FMT_INT, ret = 1);
		ASSERT(qr.version, ==, 1, FMT_INT, ret = 1);
		ASSERT(qr.char_count, ==, 0, FMT_INT, ret = 1);
	}

	qr_close(&qr);

	result = qr_encode_utf8(&qr, alloc, "", QR_MODE_KANJI, QR_VERSION_AUTO, QR_ECL_HIGH, &error);

	if (!result) {
		FAIL("qr_encode_utf8");
		if (error) printf("error: %s\n", error);
	} else {
		ASSERT(qr.ecl, ==, QR_ECL_HIGH, FMT_INT, ret = 1);
		ASSERT(qr.mode, ==, QR_MODE_KANJI, FMT_INT, ret = 1);
		ASSERT(qr.version, ==, 1, FMT_INT, ret = 1);
		ASSERT(qr.char_count, ==, 0, FMT_INT, ret = 1);
	}

	qr_close(&qr);

	// test with very long string
	char *str = alloc.malloc(920);
	if (!str) {
		FAIL("malloc");
		return 1;
	}
	for (size_t i = 0, c = 0; i < 919; i++, c %= 10) str[i] = '0' + (c++);
	str[919] = '\0';

	result = qr_encode_utf8(&qr, alloc, str, QR_MODE_AUTO, 5, QR_ECL_HIGH, &error);

	if (!result) {
		FAIL("qr_encode_utf8");
		if (error) printf("error: %s\n", error);
	} else {
		bool match = true;
		ASSERT(qr.ecl, ==, QR_ECL_HIGH, FMT_INT, match = 0);
		ASSERT(qr.mode, ==, QR_MODE_NUMERIC, FMT_INT, match = 0);
		ASSERT(qr.version, ==, 20, FMT_INT, match = 0);
		ASSERT(qr.char_count, ==, 919, FMT_INT, match = 0);
		if (!match) ret = 1;
		else {
			result = qr_encode_prepare(&qr, &error);
			if (!result) {
				FAIL("qr_encode_prepare");
				if (error) printf("error: %s\n", error);
				ret = 1;
			} else {
				uint8_t expected_i[] = {
#include "qr_encode_data.h"
				};
				ASSERT(qr.data_i.byte_index, ==, LEN(expected_i), FMT_INT, ret = 1);
				if (qr.data_i.byte_index == LEN(expected_i)) {
					for (size_t i = 0; i < qr.data_i.byte_index; i++) {
						printf("i: %zu: ", i);
						ASSERT(((uint8_t *) qr.data_i.data)[i], ==, expected_i[i], FMT_HEX, ret = 1);
					}
				}
			}
		}
	}


	return ret;
}
