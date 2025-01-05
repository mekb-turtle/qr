#include <stdio.h>
#include <stdint.h>
#include "../libqr/qr_render.h"
#include "test.h"
#include <stdlib.h>
#include <string.h>

static bool expected[][15] = {
        // https://www.thonky.com/qr-code-tutorial/format-version-tables
        {1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0},
        {1, 1, 1, 0, 0, 1, 0, 1, 1, 1, 1, 0, 0, 1, 1},
        {1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0},
        {1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 1},
        {1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 1},
        {1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0},
        {1, 1, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1},
        {1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 0},
        {1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0},
        {1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
        {1, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0},
        {1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1},
        {1, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0},
        {1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 0, 1, 1, 1},
        {1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0},
        {0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1},
        {0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0},
        {0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1},
        {0, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0},
        {0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0},
        {0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1},
        {0, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0},
        {0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1},
        {0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1},
        {0, 0, 1, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0},
        {0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1},
        {0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0},
        {0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 0, 0, 0, 1, 0},
        {0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1},
        {0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0},
        {0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1}
};

int main() {
	int ret = 0;
	bool out[15];
	for (uint8_t ecl = 0; ecl < 4; ++ecl)
		for (uint8_t mask = 0; mask < 8; ++mask) {
			memset(out, 0, sizeof(out));
			printf("ecl %" PRIu8 " mask %" PRIu8 ":\n", ecl, mask);
			get_format_information(ecl, mask, out);
			size_t i = ecl * 8 + mask;
			for (uint8_t bit = 0; bit < 15; bit++) {
				ASSERT(out[bit], ==, expected[i][bit], FMT_HEX, ret = 1);
			}
		}
	return ret;
}
