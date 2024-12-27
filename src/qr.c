#include "qr.h"
#include <string.h>
#include <iconv.h>
#include "utf8.h"

// add bits to data buffer
static bool add_bits(uint8_t **data, size_t *len, size_t *index_byte, uint8_t *index_bit, uint32_t value, uint8_t bits) {
	//TODO
}

#define FREE(ptr)                    \
	{                                \
		if (qr->alloc.free && ptr) { \
			qr->alloc.free(ptr);     \
		}                            \
		ptr = NULL;                  \
	}

// pre-encoded data
static bool qr_init_raw(struct qr *qr, struct qr_alloc alloc, const void *data, size_t len, enum qr_encoding encoding, unsigned int version, enum qr_ecl ecl) {
	if (!alloc.malloc) return false;
	qr->alloc = alloc;

	if (!data || !len) return false;

	// sanity checks
	if (ecl != ECL_LOW && ecl != ECL_MEDIUM && ecl != ECL_QUARTILE && ecl != ECL_HIGH) return false;
	if (version < 1 || version > 40) return false;
	if (encoding != ENC_NUMERIC && encoding != ENC_ALPHANUMERIC && encoding != ENC_BYTE && encoding != ENC_KANJI) return false;

	qr->data = data;
	qr->data_len = len;
	qr->encoding = encoding;
	qr->version = version;
	qr->ecl = ecl;

	return true;
}

bool qr_init_utf8(struct qr *qr, struct qr_alloc alloc, const void *data_, enum qr_encoding encoding, unsigned int version, enum qr_ecl ecl) {
	if (!alloc.malloc) return false;
	qr->alloc = alloc;

	const uint8_t *data = data_;
	if (!data_ || !*data) return false;

	bool numeric = true, alphanumeric = true, byte = true, kanji = true;

	const char *const numeric_chars = "0123456789";
	const char *const alphanumeric_chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ $%*+-./:";

	size_t num_codepoints = 0, num_bytes = 0;

	// scan codepoints in UTF-8
	for (size_t i = 0; data[i];) {
		uint32_t codepoint;
		uint8_t read = read_utf8(&data[i], &codepoint);
		if (!read) return false; // invalid UTF-8 sequence
		i += read;               // move to next codepoint
		num_bytes += read;
		++num_codepoints;

		if (numeric)
			numeric = strchr(numeric_chars, *data) != NULL;
		if (alphanumeric)
			alphanumeric = strchr(alphanumeric_chars, *data) != NULL;
		if (byte)
			byte = codepoint < 0x100; // IEC 8859-1 is every codepoint from 0x00 to 0xFF
	}


	// use iconv to detect if kanji mode can be used
	iconv_t cd = iconv_open("SHIFT_JIS//TRANSLIT", "UTF-8"); // enable transliteration
	if (cd == (iconv_t) -1) return false;                    // failed to open iconv

	size_t kanji_size = num_codepoints * 2; // worst case scenario but we can realloc later
	uint8_t *kanji_data = qr->alloc.malloc(kanji_size);
	if (!kanji_data) {
		iconv_close(cd);
		return false;
	} else {
		// initialise variables for iconv
		const uint8_t *data_ = data;
		const uint8_t *kanji_data_ = kanji_data;
		size_t in_size = num_bytes, out_size = kanji_size;
		size_t iconv_ret = iconv(cd, (char **) &data_, &in_size, (char **) &kanji_data_, &out_size);
		if (iconv_ret == (size_t) -1 || kanji_data_ < kanji_data) {
			FREE(kanji_data);
			kanji = false; // not possible
		}
		kanji_size = kanji_data_ - kanji_data; // iconv increments the pointer, so we can calculate the new size using this
		if (qr->alloc.realloc)
			kanji_data = qr->alloc.realloc(kanji_data, kanji_size); // shrink to fit
	}
	iconv_close(cd);

	if (kanji) {
		// verify the data is entirely double-byte
		for (size_t i = 0; i < kanji_size; i += 2) {
			if (kanji_data[i] < 0x81 || (kanji_data[i] > 0x9f && kanji_data[i] < 0xe0) || kanji_data[i] > 0xef) {
			invalid_kanji:
				kanji = false;
				break;
			}
			if ((kanji_data[i] & 1) == 0) { // even
				if (kanji_data[i + 1] < 0x9f || kanji_data[i + 1] > 0xfc) goto invalid_kanji;
			} else { // odd
				if (kanji_data[i + 1] < 0x40 || kanji_data[i + 1] > 0x9e || kanji_data[i + 1] == 0x7f) goto invalid_kanji;
			}
		}
	}

	if (!byte && !kanji && !numeric && !alphanumeric) {
		// no valid encoding found
	invalid_encoding:
		FREE(kanji_data);
		return false;
	}
	// figure out what encoding to use
	switch (encoding) {
		case ENC_NUMERIC:
			if (!numeric) goto invalid_encoding;
			break;
		case ENC_ALPHANUMERIC:
			if (!alphanumeric) goto invalid_encoding;
			break;
		case ENC_BYTE:
			if (!byte) goto invalid_encoding;
			break;
		case ENC_KANJI:
			if (!kanji) goto invalid_encoding;
			break;
		default:
			if (byte) encoding = ENC_BYTE;
			if (kanji) encoding = ENC_KANJI;
			if (alphanumeric) encoding = ENC_ALPHANUMERIC;
			if (numeric) encoding = ENC_NUMERIC;
			break;
	}

	if (encoding != ENC_KANJI) {
		// free already allocated data
		FREE(kanji_data);
	}

	size_t new_data_len = 1;
	uint8_t *new_data;
	size_t index_byte = 0;
	uint8_t index_bit = 0;

#define ADD_BITS(value, bits) add_bits(&new_data, &new_data_len, &index_byte, &index_bit, value, bits)

	// TODO: auto detect version to use
	if (version == 0) version = 40;

	uint8_t count_bits;
	switch (encoding) {
		case ENC_NUMERIC:
			new_data_len += (num_codepoints + 2) / 3 * 10;
			count_bits = version < 10 ? 10 : (version < 27 ? 12 : 14);
			break;
		case ENC_ALPHANUMERIC:
			new_data_len += (num_codepoints + 1) / 2 * 11;
			count_bits = version < 10 ? 9 : (version < 27 ? 11 : 13);
			break;
		case ENC_BYTE:
			new_data_len += num_codepoints;
			count_bits = version < 10 ? 8 : 16;
			break;
		case ENC_KANJI:
			new_data_len += kanji_size / 2 * 13;
			count_bits = version < 10 ? 8 : (version < 27 ? 10 : 12);
			break;
		default:
			return false;
	}

	new_data = qr->alloc.malloc(new_data_len);
	if (!new_data) return false;
	
	// add mode indicator
	if (!ADD_BITS(encoding, 4)) {
	discard_data:
		FREE(new_data);
		return false;
	}

	// add character count indicator
	if (!ADD_BITS(num_codepoints, count_bits)) goto discard_data;

	switch (encoding) {
		case ENC_NUMERIC:
			// https://www.thonky.com/qr-code-tutorial/numeric-mode-encoding
			for (size_t i = 0; data[i];) {
				uint16_t value = 0;
				uint8_t bytes;
				// add up to 3 characters
				for (bytes = 0; bytes < 3 && data[i]; ++bytes) {
					value *= 10;
					value += data[i++] - '0'; // get digit
				}
				uint8_t bits = bytes * 3 + 1; // 4, 7, 10
				if (!ADD_BITS(value, bits)) goto discard_data;
			}
			break;

		case ENC_ALPHANUMERIC:
			// https://www.thonky.com/qr-code-tutorial/alphanumeric-mode-encoding
			for (size_t i = 0; data[i];) {
				uint16_t value = 0;
				uint8_t bytes;
				// add up to 2 characters
				for (bytes = 0; bytes < 2 && data[i]; ++bytes) {
					value *= 45;
					char *c_ = strchr(alphanumeric_chars, data[i]);
					if (!c_) {
						FREE(new_data);
						return false;
					}
					uint8_t c = c_ - alphanumeric_chars;
					value += c;
				}
				uint8_t bits = bytes * 5 + 1; // 6, 11
				if (!ADD_BITS(value, bits)) goto discard_data;
			}
			break;

		case ENC_BYTE:
			// https://www.thonky.com/qr-code-tutorial/byte-mode-encoding
			for (size_t i = 0; data[i];) {
				uint32_t codepoint;
				uint8_t read = read_utf8(&data[i], &codepoint);
				if (!read) return false; // invalid UTF-8 sequence
				i += read;               // move to next codepoint

				if (!ADD_BITS(codepoint, 8)) goto discard_data;
			}
			break;

		case ENC_KANJI:
			// https://www.thonky.com/qr-code-tutorial/kanji-mode-encoding
			for (size_t i = 0; kanji_data[i]; i += 2) {
				uint16_t word = kanji_data[i] << 8 | kanji_data[i + 1];
				if (word >= 0xc140)
					word -= 0xc140;
				else
					word -= 0x8140;
				uint8_t msb = word >> 8;
				uint8_t lsb = word & 0xff;
				uint16_t value = (uint16_t) msb * 0xc0 + lsb;
				if (!ADD_BITS(value, 13)) {
					FREE(kanji_data);
					goto discard_data;
				}
			}
			FREE(kanji_data);
			// TODO
			return false;
			break;

		default:
			return false;
	}

	// TODO: add terminator and padding
	// TODO: add error correction
	// TODO: add remainder bits
	// TODO: render QR code

	return qr_init_raw(qr, alloc, new_data, new_data_len, encoding, version, ecl);
}

#undef FREE

static bool qr_output_offset(struct qr_output output, uint8_t x, uint8_t y, uint16_t *byte, uint8_t *bit) {
	// check if x and y are within bounds
	if (x >= output.qr_size || y >= output.qr_size) return false;

	// calculate index of bit
	uint16_t index = y * (uint16_t) output.qr_size + x;
	if (bit) *bit = 1 << (index % 8);
	if (byte) *byte = index / 8;
	return true;
}

static bool qr_output_write(struct qr_output *output, uint8_t x, uint8_t y, bool value) {
	uint16_t byte;
	uint8_t bit;
	if (!qr_output_offset(*output, x, y, &byte, &bit)) return false;

	uint8_t *data = output->data; // can't use void* for bit manipulation

	// set bit
	if (value)
		data[byte] |= bit; // true
	else
		data[byte] &= ~bit; // false

	return true;
}

bool qr_output_read(struct qr_output output, uint8_t x, uint8_t y) {
	uint16_t byte;
	uint8_t bit;
	if (!qr_output_offset(output, x, y, &byte, &bit)) return false;

	// get bit
	return ((const uint8_t *) output.data)[byte] & bit;
}

bool qr_encode(struct qr *qr, const void *data, size_t len, int encoding, unsigned int module_size, unsigned int quiet_zone) {
	if (qr->output.data) {
		// free already allocated data
		if (qr->alloc.free) qr->alloc.free(qr->output.data);
		qr->output.data = NULL;
	}

	if (module_size < 1) return false;

	// allocate memory for output
	qr->output.qr_size = 21 + 4 * qr->version;
	qr->output.data_size = (qr->output.qr_size * qr->output.qr_size + 7) / 8; // 1 bit per module
	qr->output.data = qr->alloc.malloc(qr->output.data_size);
	if (!qr->output.data) return false;

	// clear output
	memset(qr->output.data, 0, qr->output.data_size);

	// start writing modules
	// TODO

	return false;
}

void qr_close(struct qr *qr) {
	// free internal data
	if (qr->free_data && qr->alloc.free) qr->alloc.free(qr->free_data);
	if (qr->data == qr->free_data) qr->data = NULL;
	qr->free_data = NULL;

	// free output data
	if (qr->output.data && qr->alloc.free) qr->alloc.free(qr->output.data);
	qr->output.data = NULL;
}
