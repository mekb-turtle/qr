#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "../libqr/qr_render.h"
#include "test.h"

static uint8_t expected[40][8] = {
        {0},
        {2, 6, 18},
        {2, 6, 22},
        {2, 6, 26},
        {2, 6, 30},
        {2, 6, 34},
        {3, 6, 22, 38},
        {3, 6, 24, 42},
        {3, 6, 26, 46},
        {3, 6, 28, 50},
        {3, 6, 30, 54},
        {3, 6, 32, 58},
        {3, 6, 34, 62},
        {4, 6, 26, 46, 66},
        {4, 6, 26, 48, 70},
        {4, 6, 26, 50, 74},
        {4, 6, 30, 54, 78},
        {4, 6, 30, 56, 82},
        {4, 6, 30, 58, 86},
        {4, 6, 34, 62, 90},
        {5, 6, 28, 50, 72, 94},
        {5, 6, 26, 50, 74, 98},
        {5, 6, 30, 54, 78, 102},
        {5, 6, 28, 54, 80, 106},
        {5, 6, 32, 58, 84, 110},
        {5, 6, 30, 58, 86, 114},
        {5, 6, 34, 62, 90, 118},
        {6, 6, 26, 50, 74, 98, 122},
        {6, 6, 30, 54, 78, 102, 126},
        {6, 6, 26, 52, 78, 104, 130},
        {6, 6, 30, 56, 82, 108, 134},
        {6, 6, 34, 60, 86, 112, 138},
        {6, 6, 30, 58, 86, 114, 142},
        {6, 6, 34, 62, 90, 118, 146},
        {7, 6, 30, 54, 78, 102, 126, 150},
        {7, 6, 24, 50, 76, 102, 128, 154},
        {7, 6, 28, 54, 80, 106, 132, 158},
        {7, 6, 32, 58, 84, 110, 136, 162},
        {7, 6, 26, 54, 82, 110, 138, 166},
        {7, 6, 30, 58, 86, 114, 142, 170}
};

int main() {
	int ret = 0;

	for (uint8_t version = 1; version <= 40; version++) {
		printf("version %" PRIu8 ":\n", version);
		uint8_t out[7];
		uint8_t result = get_alignment_locations(version, out);
		uint8_t *exp = expected[version - 1];
		ASSERT(result, ==, exp[0], FMT_INT, ret = 1);
		if (result != exp[0]) continue;
		for (uint8_t j = 0; j < result; j++) {
			ASSERT(out[j], ==, exp[j + 1], FMT_INT, ret = 1);
		}
	}

	return ret;
}
