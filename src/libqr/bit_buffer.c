#include "bit_buffer.h"
// #define DEBUG
#ifdef DEBUG
#include <stdio.h>
#include <inttypes.h>
#define eprintf(...) fprintf(stderr, __VA_ARGS__)
#endif

// add bits to buffer
// NOTE: this function does NOT write '0' bits to the buffer, clear the buffer with memset() before using it
bool bit_buffer_add_bits(struct qr_bit_buffer *buf, uint32_t value, uint8_t bits) {
#ifdef DEBUG
	eprintf("Adding %u bits to %p: ", bits, (void *) buf);
	for (uint8_t i = 0; i < bits; ++i) {
		// loop bits
		eprintf("%" PRIu8, (value >> (bits - i - 1)) & 1);
	}
	eprintf("\n");
#endif

	uint8_t *data = buf->data;

	if (bits == 0) return true;  // nothing to do
	if (bits > 32) return false; // invalid number of bits

	// check if we have enough space
	if (buf->byte_index >= buf->size) return false;

	uint8_t bits_left = 8 - buf->bit_index;

	// loop while there is not enough space in the current byte
	while (bits > bits_left) {
		uint8_t current_bits = value >> (bits - bits_left);
		data[buf->byte_index] |= current_bits; // fill up current byte

		++buf->byte_index; // move to next byte
		bits -= bits_left; // remaining bits

		buf->bit_index = 0; // reset bit index
		bits_left = 8;

		if (buf->byte_index >= buf->size) return false;
	}

	bits_left = 8 - buf->bit_index;

	// bits <= bits_left will always be true
	// enough space in current byte
	uint8_t new_bits = value & (0xff >> (8 - bits)); // mask out bits we don't need
	data[buf->byte_index] |= new_bits << (bits_left - bits);
	buf->bit_index += bits;
	if (buf->bit_index > 7) {
		buf->bit_index = 0;
		++buf->byte_index;
	}
	return true;
}
