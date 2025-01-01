#ifndef LIBQR_UTIL_H
#define LIBQR_UTIL_H

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define DIFF(a, b) ((a) > (b) ? (a) - (b) : (b) - (a))

#define STR_(x) #x
#define STR(x) STR_(x)
#define LINE_STR "Line " STR(__LINE__)

#define ERR_ALLOC "Failed to allocate memory"

#define QR_ENSURE_ALLOC(qr) (qr->alloc.malloc && qr->alloc.realloc && qr->alloc.free)

// free memory using qr->alloc and set pointer to NULL
#define FREE(ptr)                    \
	{                                \
		if (qr->alloc.free && ptr) { \
			qr->alloc.free(ptr);     \
		}                            \
		ptr = NULL;                  \
	}

#endif // LIBQR_UTIL_H
