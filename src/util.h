#ifndef UTIL_H
#define UTIL_H
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

struct bit_buffer {
	void *data;
	size_t size;
	size_t byte_index;
	uint8_t bit_index;
};

bool add_bits(struct bit_buffer *buf, uint32_t value, uint8_t bits);

bool is_little_endian();
uint16_t endian16_swap(uint16_t val);
uint32_t endian32_swap(uint32_t val);

#define TO_BE32(val) (is_little_endian() ? endian32_swap(val) : val)
#define TO_BE16(val) (is_little_endian() ? endian16_swap(val) : val)
#define TO_LE32(val) (is_little_endian() ? val : endian32_swap(val))
#define TO_LE16(val) (is_little_endian() ? val : endian16_swap(val)

#define MATCH(s1, s2) (strcasecmp(s1, s2) == 0)

// common error messages
#define ERR_ALLOC "Failed to allocate memory"

#define eprintf(...) fprintf(stderr, __VA_ARGS__)
#endif // UTIL_H
