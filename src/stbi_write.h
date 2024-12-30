#include <stddef.h>
#include "qr.h"

#define STBIW_MALLOC(sz) output_alloc.malloc(sz)
#define STBIW_REALLOC(p, newsz) output_alloc.realloc(p, newsz)
#define STBIW_FREE(p) output_alloc.free(p)

// define custom alloc functions for stb_image_write
extern struct qr_alloc output_alloc;

#include "../submodule/stb/stb_image_write.h"
