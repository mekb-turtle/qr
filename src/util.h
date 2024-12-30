#ifndef UTIL_H
#define UTIL_H
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

struct bit_buffer {
	void *data;
	size_t size;
	size_t byte_index;
	uint8_t bit_index;
};

bool add_bits(struct bit_buffer *buf, uint32_t value, uint8_t bits);

#define MATCH(s1, s2) (strcasecmp(s1, s2) == 0)
#endif // UTIL_H
