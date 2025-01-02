#ifndef LIBQR_UTIL_H
#define LIBQR_UTIL_H

#define QR_ECL_NUM (4)
#define QR_MODE_NUM (4)
#define QR_VERSION_NUM (QR_MAX_VERSION - QR_MIN_VERSION + 1)

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define DIFF(a, b) ((a) > (b) ? (a) - (b) : (b) - (a))

#define STR_(x) #x
#define STR(x) STR_(x)
#define LINE_STR "Line " STR(__LINE__)

#define ERR_ALLOC "Failed to allocate memory"

// validity checks
#define QR_ECL_VALID(ecl) ((ecl) == QR_ECL_LOW || (ecl) == QR_ECL_MEDIUM || (ecl) == QR_ECL_QUARTILE || (ecl) == QR_ECL_HIGH)
#define QR_MODE_VALID(mode) ((mode) == QR_MODE_NUMERIC || (mode) == QR_MODE_ALPHANUMERIC || (mode) == QR_MODE_BYTE || (mode) == QR_MODE_KANJI)
#define QR_VERSION_VALID(version) ((version) >= QR_MIN_VERSION && (version) <= QR_MAX_VERSION)
#define QR_ALLOC_VALID(alloc) ((alloc).malloc && (alloc).realloc && (alloc).free)
#define QR_VALID(qr) ((qr) && qr->data.data && QR_ECL_VALID(qr->ecl) && QR_MODE_VALID(qr->mode) && QR_VERSION_VALID(qr->version) && QR_ALLOC_VALID(qr->alloc))

// free memory using qr->alloc and set pointer to NULL
#define FREE(ptr)                    \
	{                                \
		if (qr->alloc.free && ptr) { \
			qr->alloc.free(ptr);     \
		}                            \
		ptr = NULL;                  \
	}

#endif // LIBQR_UTIL_H
