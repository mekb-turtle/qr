#ifndef LIBQR_QR_H
#define LIBQR_QR_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limits.h>

#ifdef __cplusplus
extern "C" {
#endif

#define QR_VERSION_MIN (1)
#define QR_VERSION_MAX (40)
#define QR_VERSION_AUTO (0)

#define QR_MASK_MIN (0)
#define QR_MASK_MAX (7)
#define QR_MASK_AUTO (255)

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
#define PRIuQR PRIu32
#define PRIxQR PRIx32
#define PRIXQR PRIX32
#define PRIoQR PRIo32

struct qr_pos {
	qr_t x, y;
};

struct qr_bit_buffer {
	void *data;
	size_t size;
	size_t byte_index;
	uint8_t bit_index;
};

struct qr {
	// error correction level
	enum qr_ecl {
		QR_ECL_LOW = 0,
		QR_ECL_MEDIUM = 1,
		QR_ECL_QUARTILE = 2,
		QR_ECL_HIGH = 3,
		// flags:
		QR_ECL_ALL_MASK = 0x03,
		QR_ECL_NO_BOOST = 0x04
	} ecl;

	// encoding mode
	enum qr_mode {
		QR_MODE_AUTO = 0,
		QR_MODE_NUMERIC = 1,
		QR_MODE_ALPHANUMERIC = 2,
		QR_MODE_BYTE = 3,
		QR_MODE_KANJI = 4
	} mode;

	uint8_t version; // QR code version, 1-40, 0 = auto

	struct qr_bit_buffer data;   // data encoded in the QR code
	struct qr_bit_buffer data_i; // interleaved data, ready for rendering into modules

	size_t char_count; // number of characters in the data

	struct qr_alloc alloc;

	struct qr_bitmap {
		void *data;       // one bit = one module
		qr_t size;        // width/height of the QR code matrix
		size_t data_size; // size of the data buffer
	} output;
};

// the general order of functions is:
// qr_init -> qr_encode_* -> qr_prepare_data -> qr_render -> qr_bitmap_read[] -> qr_close

// encode QR code with UTF-8 encoded data
// NOTE: *qr MUST be zeroed out at least once before calling this function
bool qr_encode_utf8(struct qr *qr, struct qr_alloc alloc, const void *data, enum qr_mode mode, uint8_t version, enum qr_ecl ecl, const char **error);

// prepares data for rendering
bool qr_prepare_data(struct qr *qr, const char **error);

// renders QR code into modules
bool qr_render(struct qr *qr, const char **error, uint8_t mask);

// read/write a module from the QR code
bool qr_bitmap_write(struct qr_bitmap *output, struct qr_pos pos, bool value);
bool qr_bitmap_read(struct qr_bitmap output, struct qr_pos pos);

// frees the QR code
void qr_close(struct qr *qr);

#ifdef __cplusplus
}
#endif
#endif // LIBQR_QR_H
