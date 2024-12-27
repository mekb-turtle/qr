#ifndef QR_H
#define QR_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define QR_MARGIN_DEFAULT (4)

// allocator functions
struct qr_alloc {
	void *(*malloc)(size_t);
	void *(*realloc)(void *, size_t);
	void (*free)(void *);
};

#define QR_ALLOC(malloc_, realloc_, free_) \
	((struct qr_alloc){.malloc = malloc_, .realloc = realloc_, .free = free_})

struct qr {
	// error correction level
	enum qr_ecl {
		ECL_LOW,
		ECL_MEDIUM,
		ECL_QUARTILE,
		ECL_HIGH
	} ecl;

	// QR code encoding
	enum qr_encoding {
		ENC_AUTO = 0,
		ENC_NUMERIC = 1,
		ENC_ALPHANUMERIC = 2,
		ENC_BYTE = 4,
		ENC_KANJI = 8
	} encoding;

	uint8_t version; // QR code version, 1-40, 0 for auto

	const void *data; // data encoded in the QR code (w/o error correction)
	size_t data_len;
	uint8_t data_bit_index; // current bit index in the data

	size_t char_count; // number of characters in the data

	struct qr_alloc alloc;

	void *free_data; // internal data created by qr_init that needs to be freed

	struct qr_output {
		void *data;      // one bit = one module
		uint8_t qr_size; // width/height of the QR code
		uint16_t data_size;
	} output;
};

// initialise QR code with UTF-8 encoded data
bool qr_init_utf8(struct qr *qr, struct qr_alloc alloc, const void *data, enum qr_encoding encoding, unsigned int version, enum qr_ecl ecl);

// generates QR code
bool qr_encode(struct qr *qr, const void *data, size_t len, int encoding, unsigned int module_size, unsigned int quiet_zone);

// reads a module from the QR code
bool qr_output_read(struct qr_output output, uint8_t x, uint8_t y);

// frees the QR code
void qr_close(struct qr *qr);
#endif // QR_H
