#include <stddef.h>

#define STBIW_MALLOC(sz) output_malloc(sz)
#define STBIW_REALLOC(p, newsz) output_realloc(p, newsz)
#define STBIW_FREE(p) output_free(p)

// define custom alloc functions for stb_image_write
extern void *(*output_malloc)(size_t size);
extern void *(*output_realloc)(void *ptr, size_t size);
extern void (*output_free)(void *ptr);

#define STBI_WRITE_NO_STDIO
#include "../submodule/stb/stb_image_write.h"
