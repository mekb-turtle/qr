#ifndef LIBQR_UTF8_H
#define LIBQR_UTF8_H
#include <stdint.h>
#include <stdbool.h>
uint8_t read_utf8(const uint8_t *str, uint32_t *codepoint);
#endif // LIBQR_UTF8_H
