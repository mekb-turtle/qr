#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "../libqr/qr.h"
#include "test.h"

int main() {
	struct qr_alloc alloc = QR_ALLOC(malloc, realloc, free);
	const char *error = NULL;
	struct qr qr;
	memset(&qr, 0, sizeof(qr));
	bool ret = qr_encode_utf8(&qr, alloc, "HELLO WORLD", ENC_AUTO, 0, ECL_LOW, &error);
	if (!ret) {
		FAIL("qr_encode_utf8");
		if (error) printf("error: %s\n", error);
	} else {
		ASSERT(qr.ecl, ==, ECL_LOW, FMT_INT, ret = false);
		ASSERT(qr.encoding, ==, ENC_ALPHANUMERIC, FMT_INT, ret = false);
		ASSERT(qr.version, ==, 1, FMT_INT, ret = false);
		ASSERT(qr.char_count, ==, 11, FMT_INT, ret = false);
	}

	qr_close(&qr);

	// test with a larger string
	ret = qr_encode_utf8(&qr, alloc, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz", ENC_AUTO, 5, ECL_MEDIUM, &error);
	if (!ret) {
		FAIL("qr_encode_utf8");
		if (error) printf("error: %s\n", error);
	} else {
		ASSERT(qr.ecl, ==, ECL_MEDIUM, FMT_INT, ret = false);
		ASSERT(qr.encoding, ==, ENC_BYTE, FMT_INT, ret = false);
		ASSERT(qr.version, ==, 10, FMT_INT, ret = false);
		ASSERT(qr.char_count, ==, 182, FMT_INT, ret = false);
	}

	qr_close(&qr);

	// test with numeric encoding at a higher verion
	ret = qr_encode_utf8(&qr, alloc, "1234567890", ENC_AUTO, 30, ECL_HIGH, &error);
	if (!ret) {
		FAIL("qr_encode_utf8");
		if (error) printf("error: %s\n", error);
	} else {
		ASSERT(qr.ecl, ==, ECL_HIGH, FMT_INT, ret = false);
		ASSERT(qr.encoding, ==, ENC_NUMERIC, FMT_INT, ret = false);
		ASSERT(qr.version, ==, 30, FMT_INT, ret = false);
		ASSERT(qr.char_count, ==, 10, FMT_INT, ret = false);
	}

	// test with a string that requires kanji encoding
	ret = qr_encode_utf8(&qr, alloc, "ぁあぃいぅうぇえぉおかがきぎく", ENC_AUTO, 0, ECL_HIGH, &error);

	if (!ret) {
		FAIL("qr_encode_utf8");
		if (error) printf("error: %s\n", error);
	} else {
		ASSERT(qr.ecl, ==, ECL_HIGH, FMT_INT, ret = false);
		ASSERT(qr.encoding, ==, ENC_KANJI, FMT_INT, ret = false);
		ASSERT(qr.version, ==, 3, FMT_INT, ret = false);
		ASSERT(qr.char_count, ==, 15, FMT_INT, ret = false);
	}

	qr_close(&qr);

	return ret;
}
