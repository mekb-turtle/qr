#include "qr.h"
#include "util.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <stdio.h> // for debugging, TODO: remove

struct qr_render {
	struct qr_bitmap *bitmap, *mask;
};

struct qr_rect {
	union {
		struct qr_pos pos;
		struct {
			qr_t x, y;
		};
	};
	union {
		struct qr_pos size;
		struct {
			qr_t w, h;
		};
	};
};

static bool write(struct qr_render render, struct qr_pos pos, bool value, bool override) {
	if (qr_bitmap_read(*render.mask, pos) && !override) return false;
	if (!qr_bitmap_write(render.mask, pos, true)) return false;
	if (!qr_bitmap_write(render.bitmap, pos, value)) return false;
	return true;
}

static void write_rect(struct qr_render render, struct qr_rect rect, bool value, bool override) {
	for (qr_t y = 0; y < rect.h; y++) {
		for (qr_t x = 0; x < rect.w; x++) {
			struct qr_pos pos = QR_POS(rect.x + x, rect.y + y);
			write(render, pos, value, override);
		}
	}
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
			write(render, p, fill, true); // not concerned with return value
		}
	}
}

static void write_alignment(struct qr_render render, struct qr_pos pos) {
	for (qr_t y = 0; y < 5; y++) {
		for (qr_t x = 0; x < 5; x++) {
			uint8_t chebyshev = MAX(DIFF(x, 2), DIFF(y, 2));
			bool fill = chebyshev != 1;
			struct qr_pos p = QR_POS(pos.x + x - 2, pos.y + y - 2);
			write(render, p, fill, true);
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

static bool fetch_bit(struct qr *qr, size_t *byte, uint8_t *bit, bool *out) {
	// don't read past end of data
	if (*byte > qr->data_i.byte_index) return false;
	if (*byte == qr->data_i.byte_index && *bit >= qr->data_i.bit_index) return false;

	// read bit
	*out = ((const uint8_t *) qr->data_i.data)[*byte] & (1 << *bit);

	// move to next bit
	++*bit;
	if (*bit >= 8) {
		*bit = 0;
		(*byte)++;
	}

	return true;
}

#define QR_RECT(x_, y_, w_, h_) ((struct qr_rect){.x = x_, .y = y_, .w = w_, .h = h_})

uint16_t calculate_penalty(struct qr_render render) {
	(void) render;
	// TODO
	return 1;
}

void apply_mask(struct qr_render render, uint8_t i) {
	struct qr_pos pos;
	for (pos.x = 0; pos.x < render.bitmap->size; ++pos.x)
		for (pos.y = 0; pos.y < render.bitmap->size; ++pos.y) {
			if (qr_bitmap_read(*render.mask, pos)) continue; // skip reserved modules
			size_t product = pos.x * (size_t) pos.y;
			bool invert = false;
			// clang-format off
			switch (i) {
				case 0: invert = ((pos.x + pos.y) % 2)                     == 0; break;
				case 1: invert = (pos.y % 2)                               == 0; break;
				case 2: invert = (pos.x % 3)                               == 0; break;
				case 3: invert = ((pos.x + pos.y) % 3)                     == 0; break;
				case 4: invert = ((pos.x / 3 + pos.y / 2) % 2)             == 0; break;
				case 5: invert = (product % 2 + product % 3)               == 0; break;
				case 6: invert = ((product % 2 + product % 3) % 2)         == 0; break;
				case 7: invert = (((pos.x + pos.y) % 2 + product % 3) % 2) == 0; break;
				default: return;
			}
			// clang-format on
			if (!invert) continue;
			// invert module
			bool value = !qr_bitmap_read(*render.bitmap, pos);
			qr_bitmap_write(render.bitmap, pos, value);
		}
}

bool qr_render(struct qr *qr, const char **error, uint8_t mask) {
	struct qr_bitmap qr_drawn = {0};
	struct qr_render render = {&qr->output, &qr_drawn};

#define ERROR(msg)             \
	{                          \
		*error = msg;          \
		FREE(qr_drawn.data);   \
		FREE(qr->output.data); \
		qr_close(qr);          \
		return false;          \
	}

	if (!(QR_VALID(qr) && qr->data_i.data)) ERROR("Invalid QR data");

	if (qr->output.data) {
		// free already allocated data
		if (qr->alloc.free) qr->alloc.free(qr->output.data);
		qr->output.data = NULL;
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
	memset(qr_drawn.data, 0, qr_drawn.data_size);

	// write finder patterns
	write_finder(render, QR_POS(0, 0));                   // top-left
	write_finder(render, QR_POS(qr->output.size - 7, 0)); // top-right
	write_finder(render, QR_POS(0, qr->output.size - 7)); // bottom-left

	// write timing patterns
	for (qr_t i = 8; i < qr->output.size - 8; i++) {
		write(render, QR_POS(i, 6), i % 2 == 0, true); // horizontal
		write(render, QR_POS(6, i), i % 2 == 0, true); // vertical
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
	write(render, QR_POS(8, qr->output.size - 8), true, true);

	// reserve space for format information
	struct qr_rect reserved[6] = {
	        QR_RECT(0, 8, 9, 1),
	        QR_RECT(8, 0, 1, 8),
	        QR_RECT(qr->output.size - 8, 8, 8, 1),
	        QR_RECT(8, qr->output.size - 8, 1, 8),
	        QR_RECT(0, 0, 0, 0),
	        QR_RECT(0, 0, 0, 0)};
	if (qr->version >= 7) {
		// reserve space for version information
		reserved[4] = QR_RECT(qr->output.size - 11, 0, 3, 6);
		reserved[5] = QR_RECT(0, qr->output.size - 11, 6, 3);
	}
	for (uint8_t i = 0; i < 6; i++) write_rect(render, reserved[i], false, false);

	// write data modules
	bool up = true, alt = false; // direction indicators
	struct qr_pos pos = QR_POS(qr->output.size - 1, qr->output.size - 1);

	size_t byte_index = 0;
	uint8_t bit_index = 0;
	while (true) {
		// get if module is already written
		bool drawn = qr_bitmap_read(qr_drawn, pos);

		if (!drawn) {
			bool bit;
			if (!fetch_bit(qr, &byte_index, &bit_index, &bit)) ERROR("Failed to fetch bit");

			// write module
			// TODO: fix valgrind errors
			if (!write(render, pos, bit, false)) break;
		}

		// move to next module, zigzag pattern
		if (up) {
			// https://www.thonky.com/qr-code-tutorial/upward.png
			if (!alt) {
				--pos.x;             // move left
			} else if (pos.y == 0) { // hit top row
				// move left and start moving downwards
				--pos.x;
				if (pos.x == 6) --pos.x; // skip over timing pattern
				up = false;
			} else {
				// move right and up
				++pos.x;
				--pos.y;
			}
		} else {
			// https://www.thonky.com/qr-code-tutorial/downward.png
			if (!alt) {
				--pos.x;                               // move left
			} else if (pos.y == qr->output.size - 1) { // hit bottom row
				// move left and start moving upwards
				if (pos.x == 0) break; // done
				--pos.x;
				up = true;
			} else {
				// move right and down
				++pos.x;
				++pos.y;
			}
		}
		alt = !alt; // alternate back and forth
	}

	if (mask == QR_MASK_AUTO) {
		uint16_t penalty = 0;

		// apply best mask
		for (uint8_t i = 0; i < 8; i++) {
			apply_mask(render, i);
			uint16_t new_penalty = calculate_penalty(render);
			if (i == 0 || new_penalty < penalty) {
				penalty = new_penalty;
				mask = i;
			}
			apply_mask(render, i); // XOR twice will undo the mask
		}
	} else {
		mask &= 7; // trim to 0-7
		printf("Forcing mask %d\n", mask);
	}
	apply_mask(render, mask);

	// write format information

	// write version information

	// debug: visualise written modules
	// memcpy(qr->output.data, qr_drawn.data, qr->output.data_size);

	// free drawn mask
	FREE(qr_drawn.data);

	return true;
#undef ERROR
}
