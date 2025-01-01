#ifndef LIBQR_GF256_H
#define LIBQR_GF256_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#define TO_VALUE(alpha) (exp_table[alpha % 255])
#define TO_ALPHA(value) (log_table[value % 255])
#define GF256_MIN_ERROR_CW (7)
#define GF256_MAX_ERROR_CW (30)
extern uint8_t log_table[255];
extern uint8_t exp_table[255];
uint8_t gf256_mul(uint8_t a, uint8_t b);
uint8_t gf256_div(uint8_t dividend, uint8_t divisor);
uint8_t gf256_add(uint8_t a, uint8_t b);
// size of out == error_cw
bool gf256_encode(uint8_t *message, size_t message_len, uint8_t error_cw, uint8_t *out);
void gf256_init();
#endif // LIBQR_GF256_H
