#ifndef UTIL_H
#define UTIL_H
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
bool add_bits(uint8_t *data, size_t len, size_t *index_byte, uint8_t *index_bit, uint32_t value, uint8_t bits);
#endif // UTIL_H
