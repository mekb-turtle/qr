#ifndef QR_TABLE_H
#define QR_TABLE_H
#include "qr.h"
#include <stdint.h>
// character capacity for each version, ECL, and mode
// https://www.thonky.com/qr-code-tutorial/character-capacities
extern const uint16_t character_capacity[QR_VERSION_NUM * QR_ECL_NUM][QR_MODE_NUM];
struct ec_row {
	// https://www.thonky.com/qr-code-tutorial/error-correction-table
	uint16_t ec_cw_per_block, group1_cw, group1_blocks, group2_cw, group2_blocks;
};
extern const struct ec_row error_correction[QR_VERSION_NUM * QR_ECL_NUM];
#endif // QR_TABLE_H
