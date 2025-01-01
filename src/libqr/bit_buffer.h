#ifndef LIBQR_BIT_BUFFER_H
#define LIBQR_BIT_BUFFER_H
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

struct bit_buffer {
	void *data;
	size_t size;
	size_t byte_index;
	uint8_t bit_index;
};

bool bit_buffer_add_bits(struct bit_buffer *buf, uint32_t value, uint8_t bits);
#endif // LIBQR_BIT_BUFFER_H
