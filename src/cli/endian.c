#include "endian.h"

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
