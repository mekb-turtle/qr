#ifndef QR_TABLE_H
#define QR_TABLE_H
#include "qr.h"
#include <stdint.h>
// character capacity for each version, ECL, and mode
extern const uint16_t character_capacity[QR_MAX_VERSION * QR_ECL_NUM][4];
#endif // QR_TABLE_H
