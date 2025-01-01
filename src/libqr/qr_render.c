#include "qr.h"
#include "util.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

struct qr_render {
	struct qr_bitmap *bitmap, *drawn;
};

static bool write(struct qr_render render, struct qr_pos pos, bool value) {
	if (!qr_bitmap_write(render.drawn, pos, true)) return false;
	if (!qr_bitmap_write(render.bitmap, pos, value)) return false;
	return true;
}

static void write_finder(struct qr_render render, struct qr_pos pos) {
	for (qr_t y = 0; y < 9; y++) {
		if (pos.y + y == 0) continue; // prevent underflow
		for (qr_t x = 0; x < 9; x++) {
			if (pos.x + x == 0) continue; // prevent underflow
			// there is probably a more efficient way to do this
			uint8_t chebyshev = MAX(DIFF(x, 4), DIFF(y, 4));
			bool fill = chebyshev < 2 || chebyshev == 3; // see the squares in the 3 corners
			struct qr_pos p = QR_POS(pos.x + x - 1, pos.y + y - 1);
			write(render, p, fill); // not concerned with return value
		}
	}
}

static void write_alignment(struct qr_render render, struct qr_pos pos) {
	for (qr_t y = 0; y < 5; y++) {
		for (qr_t x = 0; x < 5; x++) {
			uint8_t chebyshev = MAX(DIFF(x, 2), DIFF(y, 2));
			bool fill = chebyshev != 1;
			struct qr_pos p = QR_POS(pos.x + x - 2, pos.y + y - 2);
			write(render, p, fill);
		}
	}
}

// https://stackoverflow.com/a/73349304
uint8_t get_alignment_locations(uint8_t version, uint8_t *out) {
	if (version < 2 || version > 40) return 0;
	uint8_t intervals = (version / 7) + 1;                        // number of gaps between alignment patterns
	uint8_t distance = 4 * version + 4;                           // distance between first and last alignment pattern
	uint8_t step = ((float) distance / (float) intervals) + 0.5f; // round equal spacing to nearest integer
	step += step % 2;                                             // round step to next even number
	out[0] = 6;                                                   // first coordinate is always 6 (can't be calculated with step)
	for (uint8_t i = 1; i <= intervals; i++) {
		out[i] = 6 + distance - step * (intervals - i); // start right/bottom and go left/up by step*k
	}
	return intervals + 1;
}

bool qr_render(struct qr *qr, const char **error) {
	if (!QR_ENSURE_ALLOC(qr)) {
		*error = "Invalid allocator";
		return false;
	}

	if (qr->output.data) {
		// free already allocated data
		if (qr->alloc.free) qr->alloc.free(qr->output.data);
		qr->output.data = NULL;
	}

	struct qr_bitmap qr_drawn = {0};
	struct qr_render render = {&qr->output, &qr_drawn};

#define ERROR(msg)             \
	{                          \
		*error = msg;          \
		FREE(qr_drawn.data);   \
		FREE(qr->output.data); \
		return false;          \
	}

	// allocate memory for output
	qr->output.size = QR_SIZE(qr->version);
	qr->output.data_size = QR_DATA_SIZE(qr->output.size); // 1 bit per module
	qr->output.data = qr->alloc.malloc(qr->output.data_size);
	if (!qr->output.data) ERROR(ERR_ALLOC);

	// clear output
	memset(qr->output.data, 0, qr->output.data_size);

	// create a mask of what modules have been written
	qr_drawn.size = qr->output.size;
	qr_drawn.data_size = qr->output.data_size;
	qr_drawn.data = qr->alloc.malloc(qr_drawn.data_size);
	if (!qr_drawn.data) ERROR(ERR_ALLOC);

	// write finder patterns
	write_finder(render, QR_POS(0, 0));                   // top-left
	write_finder(render, QR_POS(qr->output.size - 7, 0)); // top-right
	write_finder(render, QR_POS(0, qr->output.size - 7)); // bottom-left

	// write timing patterns
	for (qr_t i = 8; i < qr->output.size - 8; i++) {
		write(render, QR_POS(i, 6), i % 2 == 0); // horizontal
		write(render, QR_POS(6, i), i % 2 == 0); // vertical
	}

	// write alignment patterns
	uint8_t alignment[7];
	uint8_t alignment_len = get_alignment_locations(qr->version, alignment);
	for (uint8_t i = 0; i < alignment_len; i++) {
		for (uint8_t j = 0; j < alignment_len; j++) {
			struct qr_pos pos = QR_POS(alignment[i], alignment[j]);

			// skip overwriting finder pattern
			if (pos.x == 6 && pos.y == 6) continue;
			if (pos.x == 6 && pos.y == qr->output.size - 7) continue;
			if (pos.x == qr->output.size - 7 && pos.y == 6) continue;

			write_alignment(render, pos);
		}
	}

	// write dark module
	write(render, QR_POS(8, qr->output.size - 8), true);

	// write format information

	// write version information

	// write data modules

	// apply best mask

	// visualise written modules
	// memcpy(qr->output.data, qr_drawn.data, qr->output.data_size);

	// free drawn mask
	FREE(qr_drawn.data);

	return true;
}
