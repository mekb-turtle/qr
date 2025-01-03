#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "../libqr/qr.h"
#include "test.h"

int main() {
	int ret = 0;
	struct qr_alloc alloc = QR_ALLOC(malloc, realloc, free);
	const char *error = NULL;
	struct qr qr;
	memset(&qr, 0, sizeof(qr));
	bool res = qr_encode_utf8(&qr, alloc, "HELLO WORLD", QR_MODE_AUTO, 0, QR_ECL_LOW, &error);
	if (!res) {
		FAIL("qr_encode_utf8");
		if (error) printf("error: %s\n", error);
	} else {
		ASSERT(qr.ecl, ==, QR_ECL_LOW, FMT_INT, ret = 1);
		ASSERT(qr.mode, ==, QR_MODE_ALPHANUMERIC, FMT_INT, ret = 1);
		ASSERT(qr.version, ==, 1, FMT_INT, ret = 1);
		ASSERT(qr.char_count, ==, 11, FMT_INT, ret = 1);
	}

	qr_close(&qr);

	// test with a larger string
	res = qr_encode_utf8(&qr, alloc, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz", QR_MODE_AUTO, 5, QR_ECL_MEDIUM, &error);
	if (!res) {
		FAIL("qr_encode_utf8");
		if (error) printf("error: %s\n", error);
	} else {
		ASSERT(qr.ecl, ==, QR_ECL_MEDIUM, FMT_INT, ret = 1);
		ASSERT(qr.mode, ==, QR_MODE_BYTE, FMT_INT, ret = 1);
		ASSERT(qr.version, ==, 10, FMT_INT, ret = 1);
		ASSERT(qr.char_count, ==, 182, FMT_INT, ret = 1);
	}

	qr_close(&qr);

	// test with numeric mode at a higher verion
	res = qr_encode_utf8(&qr, alloc, "1234567890", QR_MODE_AUTO, 30, QR_ECL_HIGH, &error);
	if (!res) {
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
	res = qr_encode_utf8(&qr, alloc, "ぁあぃいぅうぇえぉおかがきぎく", QR_MODE_AUTO, 0, QR_ECL_HIGH, &error);

	if (!res) {
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
	res = qr_encode_utf8(&qr, alloc, "", QR_MODE_AUTO, 0, QR_ECL_HIGH, &error);

	if (!res) {
		FAIL("qr_encode_utf8");
		if (error) printf("error: %s\n", error);
	} else {
		ASSERT(qr.ecl, ==, QR_ECL_HIGH, FMT_INT, ret = 1);
		ASSERT(qr.version, ==, 1, FMT_INT, ret = 1);
		ASSERT(qr.char_count, ==, 0, FMT_INT, ret = 1);
	}

	qr_close(&qr);

	res = qr_encode_utf8(&qr, alloc, "", QR_MODE_NUMERIC, 0, QR_ECL_HIGH, &error);

	if (!res) {
		FAIL("qr_encode_utf8");
		if (error) printf("error: %s\n", error);
	} else {
		ASSERT(qr.ecl, ==, QR_ECL_HIGH, FMT_INT, ret = 1);
		ASSERT(qr.mode, ==, QR_MODE_NUMERIC, FMT_INT, ret = 1);
		ASSERT(qr.version, ==, 1, FMT_INT, ret = 1);
		ASSERT(qr.char_count, ==, 0, FMT_INT, ret = 1);
	}

	qr_close(&qr);

	res = qr_encode_utf8(&qr, alloc, "", QR_MODE_ALPHANUMERIC, 0, QR_ECL_HIGH, &error);

	if (!res) {
		FAIL("qr_encode_utf8");
		if (error) printf("error: %s\n", error);
	} else {
		ASSERT(qr.ecl, ==, QR_ECL_HIGH, FMT_INT, ret = 1);
		ASSERT(qr.mode, ==, QR_MODE_ALPHANUMERIC, FMT_INT, ret = 1);
		ASSERT(qr.version, ==, 1, FMT_INT, ret = 1);
		ASSERT(qr.char_count, ==, 0, FMT_INT, ret = 1);
	}

	qr_close(&qr);

	res = qr_encode_utf8(&qr, alloc, "", QR_MODE_BYTE, 0, QR_ECL_HIGH, &error);

	if (!res) {
		FAIL("qr_encode_utf8");
		if (error) printf("error: %s\n", error);
	} else {
		ASSERT(qr.ecl, ==, QR_ECL_HIGH, FMT_INT, ret = 1);
		ASSERT(qr.mode, ==, QR_MODE_BYTE, FMT_INT, ret = 1);
		ASSERT(qr.version, ==, 1, FMT_INT, ret = 1);
		ASSERT(qr.char_count, ==, 0, FMT_INT, ret = 1);
	}

	qr_close(&qr);

	res = qr_encode_utf8(&qr, alloc, "", QR_MODE_KANJI, 0, QR_ECL_HIGH, &error);

	if (!res) {
		FAIL("qr_encode_utf8");
		if (error) printf("error: %s\n", error);
	} else {
		ASSERT(qr.ecl, ==, QR_ECL_HIGH, FMT_INT, ret = 1);
		ASSERT(qr.mode, ==, QR_MODE_KANJI, FMT_INT, ret = 1);
		ASSERT(qr.version, ==, 1, FMT_INT, ret = 1);
		ASSERT(qr.char_count, ==, 0, FMT_INT, ret = 1);
	}

	qr_close(&qr);

	return ret;
}
