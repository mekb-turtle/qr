#include "qr.h"
#include <string.h>
#include <iconv.h>
#include "utf8.h"
#include "util.h"
#include "qr_table.h"
#include "gf256.h"

#define ERR_ALLOC "Failed to allocate memory"

#define FREE(ptr)                    \
	{                                \
		if (qr->alloc.free && ptr) { \
			qr->alloc.free(ptr);     \
		}                            \
		ptr = NULL;                  \
	}

static bool qr_post_encode(struct qr *qr, const char **error);

// numeric, alphanumeric, byte, kanji
static uint8_t encoding_bits[4] = {1, 2, 4, 8};

static bool qr_ensure_alloc(struct qr *qr) {
	if (!qr->alloc.malloc) return false;
	if (!qr->alloc.realloc) return false;
	if (!qr->alloc.free) return false;
	return true;
}

bool qr_encode_utf8(struct qr *qr, struct qr_alloc alloc, const void *data__, enum qr_encoding encoding, uint8_t version, enum qr_ecl ecl, const char **error) {
	// pointers to simplify error handling
	uint8_t *kanji_data = NULL;
	struct bit_buffer *new_data = NULL;
#define ERROR(msg)                          \
	{                                       \
		*error = msg;                       \
		FREE(kanji_data);                   \
		if (new_data) FREE(new_data->data); \
		return false;                       \
	}

	qr_close(qr);

	qr->alloc = alloc;
	if (!qr_ensure_alloc(qr)) ERROR("Invalid allocator");

	qr->encoding = encoding;
	qr->version = version;
	qr->ecl = ecl;

	const uint8_t *data = data__;
	if (!data) ERROR("Data is NULL");

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
		if (!read) ERROR("Invalid UTF-8 sequence");
		i += read; // move to next codepoint
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
	if (cd == (iconv_t) -1) ERROR("Failed to open iconv");

	size_t kanji_size = qr->char_count * 2; // worst case scenario but we can realloc later
	kanji_data = qr->alloc.malloc(kanji_size);
	if (!kanji_data) {
		iconv_close(cd);
		ERROR(ERR_ALLOC);
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
		// verify the data is entirely double-byte kanji characters
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
		ERROR("No valid encoding found");
	}

	// figure out what encoding to use
	switch (qr->encoding) {
		case ENC_NUMERIC:
			if (!numeric) ERROR("Numeric encoding not possible");
			break;
		case ENC_ALPHANUMERIC:
			if (!alphanumeric) ERROR("Alphanumeric encoding not possible");
			break;
		case ENC_BYTE:
			if (!byte) ERROR("Byte encoding not possible");
			break;
		case ENC_KANJI:
			if (!kanji) ERROR("Kanji encoding not possible");
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

	new_data = &qr->data; // short-hand

	if (qr->version > QR_MAX_VERSION) ERROR("Invalid version");

	// find the smallest version that can fit the data
	while (qr->version < QR_MIN_VERSION || character_capacity[4 * (qr->version - 1) + qr->ecl][qr->encoding] < qr->char_count) {
		++qr->version;
		if (qr->version > QR_MAX_VERSION) ERROR("Too much data");
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
			ERROR("Invalid encoding");
	}

	// approximate size of the data
	new_data->size = QR_SIZE(qr->version);
	new_data->size *= new_data->size;
	new_data->size = (new_data->size + 7) / 8; // round up to nearest byte
	new_data->data = qr->alloc.malloc(new_data->size);
	if (!new_data->data) ERROR(ERR_ALLOC);

		// helper macro to add bits to new_data
#define ADD_BITS(value, bits) \
	if (!add_bits(new_data, value, bits)) ERROR("Failed to add bits");

	// add mode indicator
	ADD_BITS(encoding_bits[qr->encoding], 4);

	// add character count indicator
	ADD_BITS(qr->char_count, count_bits);

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
				ADD_BITS(value, bits);
			}
			break;

		case ENC_ALPHANUMERIC:
			for (size_t i = 0; data[i];) {
				uint16_t value = 0;
				uint8_t bytes;
				// add up to 2 characters
				for (bytes = 0; bytes < 2 && data[i]; ++bytes) {
					value *= 45;
					// this will never be NULL because we already checked for alphanumeric characters
					uint8_t c = strchr(alphanumeric_chars, data[i++]) - alphanumeric_chars;
					value += c;
				}
				uint8_t bits = bytes * 5 + 1; // 6, 11
				ADD_BITS(value, bits);
			}
			break;

		case ENC_BYTE:
			for (size_t i = 0; data[i];) {
				uint32_t codepoint;
				uint8_t read = read_utf8(&data[i], &codepoint);
				if (!read) ERROR("Invalid UTF-8 sequence");
				i += read; // move to next codepoint

				ADD_BITS(codepoint, 8);
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
				ADD_BITS(value, 13);
			}
			FREE(kanji_data);
			break;

		default:
			return false;
	}

#undef ERROR
#undef ADD_BITS
	return qr_post_encode(qr, error);
}

static bool qr_post_encode(struct qr *qr, const char **error) {
	struct bit_buffer *data = &qr->data;
#define ERROR(msg)                  \
	{                               \
		*error = msg;               \
		if (data) FREE(data->data); \
		return false;               \
	}
#define ADD_BITS(value, bits) \
	if (!add_bits(data, value, bits)) ERROR("Failed to add bits");

	const struct ec_row ec = error_correction[QR_ECL_NUM * (qr->version - 1) + qr->ecl];

	uint16_t cw_total = ec.group1_cw * ec.group1_blocks + ec.group2_cw * ec.group2_blocks; // https://www.thonky.com/qr-code-tutorial/error-correction-table
	uint16_t cw_total_bits = cw_total * 8;                                                 // total number of bits in the data

	uint16_t data_bits = data->byte_index * 8 + data->bit_index;
	if (data_bits > cw_total_bits) ERROR("Too much data, this should never happen");
	uint16_t remainder = cw_total_bits - data_bits;
	if (remainder > 4) remainder = 4; // max 4 zeroes

	ADD_BITS(0, remainder); // add remaining zeroes

	// pad to nearest byte
	if (data->bit_index != 0) {
		ADD_BITS(0, 8 - data->bit_index);
	}

	// sanity check
	if (data->bit_index != 0) ERROR("Bit index is not 0, this should never happen");

		// from now on, we are only adding full bytes
#define ADD_BYTE(byte)                                          \
	if (data->byte_index >= data->size) ERROR("Data overflow"); \
	((uint8_t *) data->data)[data->byte_index++] = byte;

	// add pad bytes until length is met
	for (uint8_t pulse = 1; data->byte_index < cw_total; pulse ^= 1) {
		// add 0xec 0x11 pattern
		ADD_BYTE(pulse ? 0xec : 0x11);
	}

	// add error correction
	uint8_t *ec_data = data->data;
	for (uint8_t i = 0; i < ec.group1_blocks; ++i) {
		// ADD_POLY(ec_data, ec.group1_cw);
		ec_data += ec.group1_cw;
	}
	for (uint8_t i = 0; i < ec.group2_blocks; ++i) {
		// ADD_POLY(ec_data, ec.group2_cw);
		ec_data += ec.group2_cw;
	}
	gf256_init();

	// TODO: add error correction
	// TODO: render bits into matrix
	// TODO: mask the matrix
	// TODO: add format and version information

	// TODO: tests

	(void) error;
	return true;
}

#undef ERROR
#undef ADD_BITS

static bool qr_bitmap_offset(struct qr_bitmap output, struct qr_pos pos, size_t *byte, uint8_t *bit) {
	// check if x and y are within bounds
	if (pos.x >= output.size || pos.y >= output.size) return false;

	// calculate index of bit
	size_t index = pos.y * (size_t) output.size + pos.x;
	if (bit) *bit = 1 << (index % 8);
	if (byte) *byte = index / 8;
	return true;
}

bool qr_bitmap_write(struct qr_bitmap *output, struct qr_pos pos, bool value) {
	size_t byte;
	uint8_t bit;
	if (!qr_bitmap_offset(*output, pos, &byte, &bit)) return false;

	uint8_t *data = output->data; // can't use void* for bit manipulation

	// set bit
	if (value)
		data[byte] |= bit; // true
	else
		data[byte] &= ~bit; // false

	return true;
}

bool qr_bitmap_read(struct qr_bitmap output, struct qr_pos pos) {
	size_t byte;
	uint8_t bit;
	if (!qr_bitmap_offset(output, pos, &byte, &bit)) return false;

	// get bit
	return ((const uint8_t *) output.data)[byte] & bit;
}

bool qr_render(struct qr *qr, const char **error) {
	if (!qr_ensure_alloc(qr)) {
		*error = "Invalid allocator";
		return false;
	}

	if (qr->output.data) {
		// free already allocated data
		if (qr->alloc.free) qr->alloc.free(qr->output.data);
		qr->output.data = NULL;
	}

#define ERROR(msg)             \
	{                          \
		*error = msg;          \
		FREE(qr->output.data); \
		return false;          \
	}

	// allocate memory for output
	qr->output.size = QR_SIZE(qr->version);
	qr->output.data_size = QR_DATA_SIZE(qr->output.size); // 1 bit per module
	// max size is 3917 bytes
	qr->output.data = qr->alloc.malloc(qr->output.data_size);
	if (!qr->output.data) return false;

	// clear output
	memset(qr->output.data, 0, qr->output.data_size);

	// test
	qr_bitmap_write(&qr->output, QR_POS(0, 0), true);
	qr_bitmap_write(&qr->output, QR_POS(0, qr->output.size - 1), true);
	qr_bitmap_write(&qr->output, QR_POS(qr->output.size - 1, 0), true);

	return true;
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