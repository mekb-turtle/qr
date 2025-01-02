#ifndef LIBQR_BIT_BUFFER_H
#define LIBQR_BIT_BUFFER_H
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "qr.h"

bool bit_buffer_add_bits(struct qr_bit_buffer *buf, uint32_t value, uint8_t bits);
#endif // LIBQR_BIT_BUFFER_H
