#include "utf8.h"
uint8_t read_utf8(const uint8_t *str, uint32_t *codepoint) {
	if (!str || !codepoint) return false;

	if ((str[0] & 0x80) == 0x00) { // ASCII
		*codepoint = str[0];
		return 1;
	}

	if ((str[1] & 0xC0) != 0x80) return 0; // invalid continuation byte

	if ((str[0] & 0xE0) == 0xC0) { // 2-byte sequence
		*codepoint = (str[0] & 0x1F) << 6;
		*codepoint |= str[1] & 0x3F;
		if (*codepoint < 0x80) return 0; // overlong encoding
		return 2;
	}

	if ((str[2] & 0xC0) != 0x80) return 0;

	if ((str[0] & 0xF0) == 0xE0) { // 3-byte sequence
		*codepoint = (str[0] & 0x0F) << 12;
		*codepoint |= (str[1] & 0x3F) << 6;
		*codepoint |= str[2] & 0x3F;
		if (*codepoint < 0x800) return 0; // overlong encoding
		return 3;
	}

	if ((str[3] & 0xC0) != 0x80) return 0;

	if ((str[0] & 0xF8) == 0xF0) { // 4-byte sequence
		*codepoint = (str[0] & 0x07) << 18;
		*codepoint |= (str[1] & 0x3F) << 12;
		*codepoint |= (str[2] & 0x3F) << 6;
		*codepoint |= str[3] & 0x3F;
		if (*codepoint < 0x10000) return 0; // overlong encoding
		return 4;
	}
	return 0; // invalid sequence
}
