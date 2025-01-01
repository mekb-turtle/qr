#ifndef ENDIAN_H
#define ENDIAN_H
#include <stdint.h>
#include <stdbool.h>
	
bool is_little_endian();
uint16_t endian16_swap(uint16_t val);
uint32_t endian32_swap(uint32_t val);

#define TO_BE32(val) (is_little_endian() ? endian32_swap(val) : val)
#define TO_BE16(val) (is_little_endian() ? endian16_swap(val) : val)
#define TO_LE32(val) (is_little_endian() ? val : endian32_swap(val))
#define TO_LE16(val) (is_little_endian() ? val : endian16_swap(val)
#endif // ENDIAN_H
