#ifndef LIBQR_QR_TABLE_H
#define LIBQR_QR_TABLE_H
#include "qr.h"
#include <stdint.h>
// character capacity for each version, ECL, and mode
// https://www.thonky.com/qr-code-tutorial/character-capacities
extern const uint16_t character_capacity[QR_VERSION_NUM * QR_ECL_NUM][QR_MODE_NUM];
struct ec_row {
	// https://www.thonky.com/qr-code-tutorial/error-correction-table
	uint16_t ec_per_block, group1_blocks, group1_cw, group2_blocks, group2_cw;
};
extern const struct ec_row error_correction[QR_VERSION_NUM * QR_ECL_NUM];
extern const uint8_t *generator_polynomials[];
extern const uint8_t remainder_bits[QR_VERSION_NUM];
#endif // LIBQR_QR_TABLE_H
