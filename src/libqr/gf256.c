#include "gf256.h"
#include <string.h>
#include "qr_table.h"

uint8_t log_table[255] = {0};
uint8_t exp_table[255] = {0};

uint8_t gf256_mul(uint8_t a, uint8_t b) {
	// log(a) + log(b) = log(a * b)
	// convert to alpha
	a = TO_ALPHA(a);
	b = TO_ALPHA(b);
	// multiply
	a = (a + b) % 255;
	// convert back to value
	a = TO_VALUE(a);
	return a;
}

uint8_t gf256_div(uint8_t dividend, uint8_t divisor) {
	if (dividend == 0 || divisor == 0) return 0;
	// log(a) - log(b) = log(a / b)
	// convert to alpha
	dividend = TO_ALPHA(dividend);
	divisor = TO_ALPHA(divisor);
	// divide
	dividend = (dividend + 255 - divisor) % 255;
	// convert back to value
	dividend = TO_VALUE(dividend);
	return dividend;
}

uint8_t gf256_add(uint8_t a, uint8_t b) {
	return a ^ b; // xor
}

bool gf256_poly_div(uint8_t *message, size_t message_len, uint8_t error_cw, uint8_t *out) {
	if (error_cw > GF256_MAX_ERROR_CW || error_cw < GF256_MIN_ERROR_CW) return false;

	const uint8_t *generator = generator_polynomials[error_cw];
	if (!generator) return false;

	size_t len = message_len + error_cw;

	uint8_t msg[len]; // multiply by x^error_cw
	memset(msg, 0, len);
	memcpy(msg, message, message_len);

	// although generator polynomial needs to have the same degree as error_cw, we don't need to add zeroes since they're not read
	uint8_t gen[error_cw + 1];
	for (size_t i = 0; i <= error_cw; ++i)
		gen[i] = TO_VALUE(generator[i]); // convert to actual value

	for (size_t i = 0; i < message_len; ++i) {
		uint8_t lead = msg[i];
		if (lead == 0) continue;

		for (size_t j = 0; j <= error_cw; ++j) {
			// multiply generator polynomial by lead term of message
			uint8_t product = gf256_mul(lead, gen[j]);
			// subtract with message
			msg[i + j] = gf256_add(msg[i + j], product);
		}

		// msg[i] == 0, we can discard this term now with ++i
	}

	// return result
	memcpy(out, &msg[message_len], error_cw);
	return true;
}

void gf256_init() {
	if (exp_table[0] != 0) return;
	// initialise log and exponent tables
	uint16_t x = 1;
	for (uint8_t i = 0; i < 255; i++) {
		exp_table[i] = x;
		log_table[x] = i;
		x *= 2;
		if (x >= 256) { // reduce
			x ^= 0x11d;
		}
	}
}
