#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "../libqr/qr.h"
#include "../libqr/qr_render.h"
#include "test.h"

int main() {
	int ret = 0;
	struct qr_alloc alloc = QR_ALLOC(malloc, realloc, free);
	const char *error = NULL;
	struct qr qr;
	memset(&qr, 0, sizeof(qr));

	bool result = qr_encode_utf8(&qr, alloc, "HELLO WORLD", QR_MODE_AUTO, QR_VERSION_AUTO, QR_ECL_QUARTILE, &error);
	if (!result) {
		FAIL("qr_encode_utf8");
		if (error) printf("error: %s\n", error);
	} else {
		bool match = true;
		ASSERT(qr.ecl, ==, QR_ECL_QUARTILE, FMT_INT, match = false);
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
				for (uint8_t mask = 0; mask < 8; ++mask) {
					result = qr_render(&qr, &error, mask);
					if (!result) {
						FAIL("qr_generate");
						if (error) printf("error: %s\n", error);
						ret = 1;
					}
					// for some reason, the test data has these bits flipped
					// https://www.thonky.com/qr-code-tutorial/data-masking#choose-the-lowest-penalty-score-for-the-eight-mask-patterns
#define XOR(x, y) qr_bitmap_write(&qr.output, QR_POS(x, y), !qr_bitmap_read(qr.output, QR_POS(x, y)))
					XOR(11, 20);
					XOR(12, 20);
					XOR(12, 19);
					XOR(12, 18);
					XOR(11, 18);
#undef XOR
					uint8_t expected[8][4] = {
					        {180, 90,  80,  0},
					        {172, 129, 120, 0},
					        {206, 141, 160, 0},
					        {180, 141, 120, 0},
					        {195, 138, 200, 0},
					        {189, 156, 200, 0},
					        {171, 102, 80,  0},
					        {197, 123, 200, 0},
					};
					uint16_t penalties[4];
					calculate_penalty(qr.output, &penalties[0], &penalties[1], &penalties[2], &penalties[3]);
					printf("mask %" PRIu8 ":\n", mask);
					for (uint8_t i = 0; i < 4; ++i) {
						ASSERT(penalties[i], ==, expected[mask][i], FMT_INT, ret = 1);
					}
					printf("---\n");
				}
			}
		}
	}

	qr_close(&qr);

	return ret;
}
