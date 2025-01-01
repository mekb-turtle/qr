#ifndef LIBQR_QR_H
#define LIBQR_QR_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limits.h>
#include "bit_buffer.h"

#define QR_MIN_VERSION (1)
#define QR_MAX_VERSION (40)

#define QR_VERSION_NUM (QR_MAX_VERSION - QR_MIN_VERSION + 1)
#define QR_ECL_NUM (4)
#define QR_MODE_NUM (4)

#define QR_SIZE(version) (17 + 4 * version)
#define QR_DATA_SIZE(size) ((size * size + 7) / 8)

// allocator functions
struct qr_alloc {
	void *(*malloc)(size_t);
	void *(*realloc)(void *, size_t);
	void (*free)(void *);
};

#define QR_ALLOC(malloc_, realloc_, free_) \
	((struct qr_alloc){.malloc = malloc_, .realloc = realloc_, .free = free_})
#define QR_POS(x_, y_) ((struct qr_pos){.x = x_, .y = y_})

typedef uint32_t qr_t;
#define QR_MIN (0)
#define QR_MAX (UINT32_MAX)

struct qr_pos {
	qr_t x, y;
};

struct qr {
	// error correction level
	enum qr_ecl {
		ECL_LOW = 0,
		ECL_MEDIUM = 1,
		ECL_QUARTILE = 2,
		ECL_HIGH = 3
	} ecl;

	// QR code encoding
	enum qr_encoding {
		ENC_AUTO = 0,
		ENC_NUMERIC = 1,
		ENC_ALPHANUMERIC = 2,
		ENC_BYTE = 3,
		ENC_KANJI = 4
	} encoding;

	uint8_t version; // QR code version, 1-40

	struct bit_buffer data;   // data encoded in the QR code
	struct bit_buffer data_i; // interleaved data, ready for rendering into modules

	size_t char_count; // number of characters in the data

	struct qr_alloc alloc;

	struct qr_bitmap {
		void *data;       // one bit = one module
		qr_t size;        // width/height of the QR code matrix
		size_t data_size; // size of the data buffer
	} output;
};

// encode QR code with UTF-8 encoded data
// qr MUST be zeroed out at least once before calling this function
bool qr_encode_utf8(struct qr *qr, struct qr_alloc alloc, const void *data, enum qr_encoding encoding, uint8_t version, enum qr_ecl ecl, const char **error);

// renders QR code into modules
bool qr_render(struct qr *qr, const char **error);

// read/write a module from the QR code
bool qr_bitmap_write(struct qr_bitmap *output, struct qr_pos pos, bool value);
bool qr_bitmap_read(struct qr_bitmap output, struct qr_pos pos);

// frees the QR code
void qr_close(struct qr *qr);
#endif // LIBQR_QR_H
