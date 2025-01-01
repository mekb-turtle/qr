#include "util.h"
// #define DEBUG
#ifdef DEBUG
#include <stdio.h>
#define eprintf(...) fprintf(stderr, __VA_ARGS__)
#endif

// add bits to buffer
bool add_bits(struct bit_buffer *buf, uint32_t value, uint8_t bits) {
#ifdef DEBUG
	eprintf("Adding %u bits to %p: ", bits, (void *) buf);
	for (uint8_t i = 0; i < bits; ++i) {
		// loop bits
		eprintf("%d", (value >> (bits - i - 1)) & 1);
	}
	eprintf("\n");
#endif

	uint8_t *data = buf->data;

	if (bits == 0 || bits > 32) return false; // invalid number of bits

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

bool is_little_endian() {
	// 0x0100 if LE, 0x0001 if BE, casting to u8 gives first byte
	uint16_t val = 1;
	return *(uint8_t *) &val;
}

uint16_t endian16_swap(uint16_t val) {
	return ((0xFF00 & val) >> 010) | ((0x00FF & val) << 010);
}

uint32_t endian32_swap(uint32_t val) {
	return ((0xFF000000 & val) >> 030) |
	       ((0x00FF0000 & val) >> 010) |
	       ((0x0000FF00 & val) << 010) |
	       ((0x000000FF & val) << 030);
}
