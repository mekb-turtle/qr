#ifndef LIBQR_QR_RENDER_H
#define LIBQR_QR_RENDER_H
#include "qr.h"
void get_format_information(uint8_t ecl, uint8_t mask, bool *out); // 2 bytes
void get_version_information(uint8_t version, bool *out);          // 3 bytes
uint8_t get_alignment_locations(uint8_t version, uint8_t *out);
uint16_t calculate_penalty(struct qr_bitmap bitmap, uint16_t *run_pen, uint16_t *box_pen, uint16_t *find_pen, uint16_t *dark_pen);
#endif // LIBQR_QR_RENDER_H
