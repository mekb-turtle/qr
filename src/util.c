#include "util.h"
// add bits to data buffer
bool add_bits(uint8_t *data, size_t len, size_t *index_byte, uint8_t *index_bit, uint32_t value, uint8_t bits) {
	if (bits == 0 || bits > 32) return false; // invalid number of bits

	// check if we have enough space
	if (*index_byte >= len) return false;

	uint8_t bits_left = 8 - *index_bit;

	// loop while there is not enough space in the current byte
	while (bits > bits_left) {
		uint8_t current_bits = value >> (bits - bits_left);
		data[*index_byte] |= current_bits; // fill up current byte

		++*index_byte;     // move to next byte
		bits -= bits_left; // remaining bits

		*index_bit = 0; // reset bit index
		bits_left = 8;
	}

	bits_left = 8 - *index_bit;

	// bits <= bits_left will always be true
	// enough space in current byte
	uint8_t new_bits = value & (0xff >> (8 - bits)); // mask out bits we don't need
	data[*index_byte] |= new_bits << (bits_left - bits);
	*index_bit += bits;
	if (*index_bit > 7) {
		*index_bit = 0;
		++*index_byte;
	}
	return true;
}
