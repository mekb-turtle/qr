#include "stbi_write.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../submodule/stb/stb_image_write.h"
void *(*output_malloc)(size_t size) = NULL;
void *(*output_realloc)(void *ptr, size_t size) = NULL;
void (*output_free)(void *ptr) = NULL;
