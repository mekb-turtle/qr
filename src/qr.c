#include "qr.h"
#include <string.h>
#include <iconv.h>
#include "utf8.h"
#include "util.h"
#include "qr_table.h"

//TODO: remove
#include <stdio.h>

#define FREE(ptr)                    \
	{                                \
		if (qr->alloc.free && ptr) { \
			qr->alloc.free(ptr);     \
		}                            \
		ptr = NULL;                  \
	}

static bool qr_post_encode(struct qr *qr);

static uint8_t encoding_bits[4] = {1, 2, 4, 8};

bool qr_init_utf8(struct qr *qr, struct qr_alloc alloc, const void *data__, enum qr_encoding encoding, uint8_t version, enum qr_ecl ecl) {
	if (!alloc.malloc) return false;
	qr_close(qr);
	qr->alloc = alloc;
	qr->encoding = encoding;
	qr->version = version;
	qr->ecl = ecl;

	const uint8_t *data = data__;
	if (!data) return false;

	// arguments should not be used after this point

	bool numeric = true, alphanumeric = true, byte = true, kanji = true;

	const char *const numeric_chars = "0123456789";
	const char *const alphanumeric_chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ $%*+-./:";

	// count number of codepoints and bytes
	qr->char_count = 0;
	size_t num_bytes = 0;

	// scan codepoints in UTF-8
	for (size_t i = 0; data[i];) {
		uint32_t codepoint;
		uint8_t read = read_utf8(&data[i], &codepoint);
		if (!read) return false; // invalid UTF-8 sequence
		i += read;               // move to next codepoint
		num_bytes += read;
		++qr->char_count;

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

	size_t kanji_size = qr->char_count * 2; // worst case scenario but we can realloc later
	uint8_t *kanji_data = qr->alloc.malloc(kanji_size);
	if (!kanji_data) {
		iconv_close(cd);
		return false;
	} else {
		// initialise variables for iconv
		const uint8_t *data_tmp = data;
		const uint8_t *kanji_data_tmp = kanji_data;
		size_t in_size = num_bytes, out_size = kanji_size;
		size_t iconv_ret = iconv(cd, (char **) &data_tmp, &in_size, (char **) &kanji_data_tmp, &out_size);
		if (iconv_ret == (size_t) -1 || kanji_data_tmp < kanji_data) {
			FREE(kanji_data);
			kanji = false; // not possible
		} else {
			kanji_size = kanji_data_tmp - kanji_data; // iconv increments the pointer, so we can calculate the new size using this
			if (qr->alloc.realloc)
				kanji_data = qr->alloc.realloc(kanji_data, kanji_size); // shrink to fit
		}
	}
	iconv_close(cd);

	if (kanji) {
		// verify the data is entirely double-byte
		for (size_t i = 0; i < kanji_size; i += 2) {
			// check first byte
			if (kanji_data[i] < 0x81 || (kanji_data[i] > 0x9f && kanji_data[i] < 0xe0) || kanji_data[i] > 0xef) {
			invalid_kanji:
				FREE(kanji_data);
				kanji = false;
				break;
			}

			// check second byte
			if ((kanji_data[i] & 1) == 0) { // even
				if (kanji_data[i + 1] < 0x9f || kanji_data[i + 1] > 0xfc) goto invalid_kanji;
			} else { // odd
				if (kanji_data[i + 1] < 0x40 || kanji_data[i + 1] > 0x9e || kanji_data[i + 1] == 0x7f) goto invalid_kanji;
			}
		}
	}

	if (!byte && !kanji && !numeric && !alphanumeric) {
		// no valid encoding found
	failure:
		FREE(kanji_data);
		return false;
	}

	// figure out what encoding to use
	switch (qr->encoding) {
		case ENC_NUMERIC:
			if (!numeric) goto failure;
			break;
		case ENC_ALPHANUMERIC:
			if (!alphanumeric) goto failure;
			break;
		case ENC_BYTE:
			if (!byte) goto failure;
			break;
		case ENC_KANJI:
			if (!kanji) goto failure;
			break;
		default:
			if (byte) qr->encoding = ENC_BYTE;
			if (kanji) qr->encoding = ENC_KANJI;
			if (alphanumeric) qr->encoding = ENC_ALPHANUMERIC;
			if (numeric) qr->encoding = ENC_NUMERIC;
			break;
	}

	if (qr->encoding != ENC_KANJI) {
		// free already allocated data
		FREE(kanji_data);
	}

	struct bit_buffer *new_data = &qr->data; // short-hand

	uint8_t table_mode_index;
	switch (qr->encoding) {
		case ENC_NUMERIC:
			table_mode_index = 0;
			break;
		case ENC_ALPHANUMERIC:
			table_mode_index = 1;
			break;
		case ENC_BYTE:
			table_mode_index = 2;
			break;
		case ENC_KANJI:
			table_mode_index = 3;
			break;
		default:
			return false;
	}

	if (qr->version > QR_MAX_VERSION) goto failure; // returns false, but we need to free kanji_data first

	// find the smallest version that can fit the data
	while (qr->version < QR_MIN_VERSION || character_capacity[4 * (qr->version - 1) + qr->ecl][table_mode_index] < qr->char_count) {
		++qr->version;
		if (qr->version > QR_MAX_VERSION) goto failure;
	}

	// number of bits for character count indicator
	uint8_t count_bits;
	switch (qr->encoding) {
		case ENC_NUMERIC:
			count_bits = qr->version < 10 ? 10 : (qr->version < 27 ? 12 : 14);
			break;
		case ENC_ALPHANUMERIC:
			count_bits = qr->version < 10 ? 9 : (qr->version < 27 ? 11 : 13);
			break;
		case ENC_BYTE:
			count_bits = qr->version < 10 ? 8 : 16;
			break;
		case ENC_KANJI:
			count_bits = qr->version < 10 ? 8 : (qr->version < 27 ? 10 : 12);
			break;
		default:
			return false;
	}

	// approximate size of the data
	new_data->size = QR_SIZE(qr->version);
	new_data->size *= new_data->size;
	new_data->size = (new_data->size + 7) / 8; // round up to nearest byte
	new_data->data = qr->alloc.malloc(new_data->size);
	if (!new_data->data) goto failure;

		// helper macro to add bits to new_data
#define ADD_BITS(value, bits) add_bits(new_data, value, bits)

	// add mode indicator
	if (!ADD_BITS(encoding_bits[qr->encoding], 4)) {
	discard_data:
		FREE(new_data);
		return false;
	}

	// add character count indicator
	if (!ADD_BITS(qr->char_count, count_bits)) goto discard_data;

	switch (qr->encoding) {
		case ENC_NUMERIC:
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
			for (size_t i = 0; data[i];) {
				uint32_t codepoint;
				uint8_t read = read_utf8(&data[i], &codepoint);
				if (!read) return false; // invalid UTF-8 sequence
				i += read;               // move to next codepoint

				if (!ADD_BITS(codepoint, 8)) goto discard_data;
			}
			break;

		case ENC_KANJI:
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
			return false;
			break;

		default:
			return false;
	}

	return qr_post_encode(qr);
}

static bool qr_post_encode(struct qr *qr) {
	// sanity checks
	if (qr->encoding != ENC_NUMERIC && qr->encoding != ENC_ALPHANUMERIC && qr->encoding != ENC_BYTE && qr->encoding != ENC_KANJI) return false;

	// TODO: add terminator and padding
	// TODO: add error correction
	// TODO: add remainder bits
	// TODO: render QR code

	return true;
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

bool qr_render(struct qr *qr) {
	if (qr->output.data) {
		// free already allocated data
		if (qr->alloc.free) qr->alloc.free(qr->output.data);
		qr->output.data = NULL;
	}

	// allocate memory for output
	qr->output.qr_size = QR_SIZE(qr->version);
	qr->output.data_size = (qr->output.qr_size * qr->output.qr_size + 7) / 8; // 1 bit per 
	// max size is 248.4 MiB
	qr->output.data = qr->alloc.malloc(qr->output.data_size);
	if (!qr->output.data) return false;

	// clear output
	memset(qr->output.data, 0, qr->output.data_size);

	// test
	qr_output_write(&qr->output, 0, 0, true);
	qr_output_write(&qr->output, 0, qr->output.qr_size - 1, true);
	qr_output_write(&qr->output, qr->output.qr_size - 1, 0, true);

	return false;
}

void qr_close(struct qr *qr) {
	// free internal data
	if (qr->free_data && qr->alloc.free) qr->alloc.free(qr->free_data);
	if (qr->data.data == qr->free_data) qr->data.data = NULL;
	qr->free_data = NULL;

	// free output data
	if (qr->output.data && qr->alloc.free) qr->alloc.free(qr->output.data);
	qr->output.data = NULL;
}
