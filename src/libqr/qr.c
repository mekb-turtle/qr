#include "qr.h"
#include <string.h>
#include <iconv.h>
#include "utf8.h"
#include "bit_buffer.h"
#include "qr_table.h"
#include "gf256.h"
#include "util.h"

#define ERR_ALLOC "Failed to allocate memory"

// numeric, alphanumeric, byte, kanji
static uint8_t mode_bits[4] = {1, 2, 4, 8};

bool qr_encode_utf8(struct qr *qr, struct qr_alloc alloc, const void *data__, enum qr_mode mode, uint8_t version, enum qr_ecl ecl, const char **error) {
	// pointers to simplify error handling
	uint8_t *kanji_data = NULL;
	struct qr_bit_buffer *new_data = NULL;

	qr_close(qr); // close any previous data

#define ERROR(msg)                          \
	{                                       \
		*error = msg;                       \
		FREE(kanji_data);                   \
		if (new_data) FREE(new_data->data); \
		qr_close(qr);                       \
		return false;                       \
	}

	if (!QR_ALLOC_VALID(alloc)) ERROR("Invalid allocator");

	memset(qr, 0, sizeof(*qr)); // zero out the struct, prevents potential UB later on

	qr->alloc = alloc;
	qr->mode = mode;
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
	if (qr->char_count > 0) {
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
	}

	if (!byte && !kanji && !numeric && !alphanumeric) {
		// no valid encoding found
		ERROR("No valid encoding mode found");
	}

	// figure out what encoding to use
	switch (qr->mode) {
		case QR_MODE_NUMERIC:
			if (!numeric) ERROR("Numeric encoding not possible");
			break;
		case QR_MODE_ALPHANUMERIC:
			if (!alphanumeric) ERROR("Alphanumeric encoding not possible");
			break;
		case QR_MODE_BYTE:
			if (!byte) ERROR("Byte encoding not possible");
			break;
		case QR_MODE_KANJI:
			if (!kanji) ERROR("Kanji encoding not possible");
			break;
		default:
			if (byte) qr->mode = QR_MODE_BYTE;
			if (kanji) qr->mode = QR_MODE_KANJI;
			if (alphanumeric) qr->mode = QR_MODE_ALPHANUMERIC;
			if (numeric) qr->mode = QR_MODE_NUMERIC;
			break;
	}

	if (qr->mode != QR_MODE_KANJI) {
		// free already allocated data
		FREE(kanji_data);
	}

	new_data = &qr->data; // short-hand

	if (qr->version > QR_MAX_VERSION) ERROR("Invalid version");

	// find the smallest version that can fit the data
	while (qr->version < QR_MIN_VERSION || character_capacity[4 * (qr->version - 1) + qr->ecl][qr->mode - 1] < qr->char_count) {
		++qr->version;
		if (qr->version > QR_MAX_VERSION) ERROR("Too much data");
	}

	// number of bits for character count indicator
	uint8_t count_bits;
	switch (qr->mode) {
		case QR_MODE_NUMERIC:
			count_bits = qr->version < 10 ? 10 : (qr->version < 27 ? 12 : 14);
			break;
		case QR_MODE_ALPHANUMERIC:
			count_bits = qr->version < 10 ? 9 : (qr->version < 27 ? 11 : 13);
			break;
		case QR_MODE_BYTE:
			count_bits = qr->version < 10 ? 8 : 16;
			break;
		case QR_MODE_KANJI:
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
	if (!bit_buffer_add_bits(new_data, value, bits)) ERROR("Failed to add bits: " LINE_STR);

	// add mode indicator
	ADD_BITS(mode_bits[qr->mode - 1], 4);

	// add character count indicator
	ADD_BITS(qr->char_count, count_bits);

	switch (qr->mode) {
		case QR_MODE_NUMERIC:
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

		case QR_MODE_ALPHANUMERIC:
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

		case QR_MODE_BYTE:
			for (size_t i = 0; data[i];) {
				uint32_t codepoint;
				uint8_t read = read_utf8(&data[i], &codepoint);
				if (!read) ERROR("Invalid UTF-8 sequence");
				i += read; // move to next codepoint

				ADD_BITS(codepoint, 8);
			}
			break;

		case QR_MODE_KANJI:
			if (!kanji_data) break; // empty data
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
			ERROR("Invalid encoding");
	}

#undef ERROR
#undef ADD_BITS
	return true;
}

bool qr_prepare_data(struct qr *qr, const char **error) {
#define ERROR(msg)           \
	{                        \
		*error = msg;        \
		FREE(qr->data.data); \
		qr_close(qr);        \
		return false;        \
	}
	if (!QR_VALID(qr)) ERROR("Invalid QR data");

#define ADD_BITS(value, bits) \
	if (!bit_buffer_add_bits(&qr->data, value, bits)) ERROR("Failed to add bits: " LINE_STR);

	const struct ec_row ec = error_correction[QR_ECL_NUM * (qr->version - 1) + qr->ecl];

	// cw = codeword
	uint16_t cw_total = ec.group1_cw * ec.group1_blocks + ec.group2_cw * ec.group2_blocks; // https://www.thonky.com/qr-code-tutorial/error-correction-table
	uint16_t cw_total_bits = cw_total * 8;                                                 // total number of bits in the data

	uint16_t data_bits = qr->data.byte_index * 8 + qr->data.bit_index;
	if (data_bits > cw_total_bits) ERROR("Too much data, this should never happen");
	uint16_t remainder = cw_total_bits - data_bits;
	if (remainder > 4) remainder = 4; // max 4 zeroes

	ADD_BITS(0, remainder); // add remaining zeroes

	// pad to nearest byte
	if (qr->data.bit_index != 0) {
		ADD_BITS(0, 8 - qr->data.bit_index);
	}

#undef ADD_BITS

	// sanity check
	if (qr->data.bit_index != 0) ERROR("Bit index is not 0, this should never happen");

	// add pad bytes until length is met
	for (uint8_t pulse = 1; qr->data.byte_index < cw_total; pulse ^= 1) {
		if (qr->data.byte_index >= qr->data.size) ERROR("Byte index overflow");
		// add 0xec 0x11 pattern
		((uint8_t *) qr->data.data)[qr->data.byte_index++] = pulse ? 0xec : 0x11;
	}

	// add error correction

	// hierarchy: groups -> blocks -> codewords (aka cw or bytes)
	size_t blocks_len = (size_t) ec.group1_blocks + ec.group2_blocks;

	size_t ec_blocks_len = blocks_len * ec.ec_per_block;
	uint8_t *ec_blocks = qr->alloc.malloc(ec_blocks_len);
	if (!ec_blocks) ERROR(ERR_ALLOC);
	memset(ec_blocks, 0, ec_blocks_len);

#undef ERROR
	FREE(qr->data_i.data);
	qr->data_i.data = NULL;
	// free ec_blocks if error
#define ERROR(msg)             \
	{                          \
		*error = msg;          \
		FREE(ec_blocks);       \
		FREE(qr->data.data);   \
		FREE(qr->data_i.data); \
		qr_close(qr);          \
		return false;          \
	}

	gf256_init();
	size_t j = 0;
#define ERR_POLY "Failed to calculate error correction data"
	uint8_t *buf = qr->data.data;
	for (uint8_t i = 0; i < ec.group1_blocks; ++i) {
		if (!gf256_poly_div(buf, ec.group1_cw, ec.ec_per_block, &ec_blocks[j])) ERROR(ERR_POLY);
		buf += ec.group1_cw;
		j += ec.ec_per_block;
	}
	for (uint8_t i = 0; i < ec.group2_blocks; ++i) {
		if (!gf256_poly_div(buf, ec.group2_cw, ec.ec_per_block, &ec_blocks[j])) ERROR(ERR_POLY);
		buf += ec.group2_cw;
		j += ec.ec_per_block;
	}

	// allocate memory for interleaved data
	qr->data_i.byte_index = 0;
	qr->data_i.bit_index = 0;
	qr->data_i.size = cw_total + ec_blocks_len + 1; // +1 for remainder bits
	qr->data_i.data = qr->alloc.malloc(qr->data_i.size);
	if (!qr->data_i.data) ERROR(ERR_ALLOC);

	buf = qr->data.data;

	// interleave data

// helper macro to add codewords to data_i
#define ADD_CW(cw)                                                                       \
	{                                                                                    \
		if (qr->data_i.byte_index >= qr->data_i.size) ERROR("Data overflow: " LINE_STR); \
		((uint8_t *) qr->data_i.data)[qr->data_i.byte_index++] = cw;                     \
	}
	j = 0;

	// add data codewords
	uint8_t *group1 = &buf[0];
	uint8_t *group2 = &buf[ec.group1_blocks * ec.group1_cw];
	for (size_t cw = 0; cw < MAX(ec.group1_cw, ec.group2_cw); ++cw) {
		// loop blocks from first group
		if (cw < ec.group1_cw)
			for (size_t block = 0; block < ec.group1_blocks; ++block) {
				ADD_CW(group1[cw + ec.group1_cw * block]);
			}
		// loop blocks from second group
		if (cw < ec.group2_cw)
			for (size_t block = 0; block < ec.group2_blocks; ++block) {
				ADD_CW(group2[cw + ec.group2_cw * block]);
			}
	}

	// add ec codewords
	group1 = &ec_blocks[0];
	group2 = &ec_blocks[ec.group1_blocks * ec.ec_per_block];
	for (size_t cw = 0; cw < ec.ec_per_block; ++cw) {
		// loop blocks from first group
		for (size_t block = 0; block < ec.group1_blocks; ++block) {
			ADD_CW(group1[cw + ec.ec_per_block * block]);
		}
		// loop blocks from second group
		for (size_t block = 0; block < ec.group2_blocks; ++block) {
			ADD_CW(group2[cw + ec.ec_per_block * block]);
		}
	}

	FREE(ec_blocks);

	// add remainder bits
	if (!bit_buffer_add_bits(&qr->data_i, 0, remainder_bits[qr->version - 1])) ERROR("Failed to add remainder bits");

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

void qr_close(struct qr *qr) {
	FREE(qr->data.data);
	FREE(qr->data_i.data);

	qr->data.byte_index = 0;
	qr->data.bit_index = 0;
	qr->data.size = 0;
	qr->data_i.byte_index = 0;
	qr->data_i.bit_index = 0;
	qr->data_i.size = 0;

	FREE(qr->output.data);

	qr->output.size = 0;
	qr->output.data_size = 0;
}
