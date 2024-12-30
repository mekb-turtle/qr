// to be called by arg.c/arg.h

#include <stddef.h>
#include <limits.h>
#include <stdint.h>

PARSE_FUNC(size, size_t, 0, SIZE_MAX, false)

PARSE_FUNC(u64, uint64_t, 0, UINT64_MAX, false)
PARSE_FUNC(i64, int64_t, INT64_MIN, INT64_MAX, true)
PARSE_FUNC(u32, uint32_t, 0, UINT32_MAX, false)
PARSE_FUNC(i32, int32_t, INT32_MIN, INT32_MAX, true)
PARSE_FUNC(u16, uint16_t, 0, UINT16_MAX, false)
PARSE_FUNC(i16, int16_t, INT16_MIN, INT16_MAX, true)
PARSE_FUNC(u8, uint8_t, 0, UINT8_MAX, false)
PARSE_FUNC(i8, int8_t, INT8_MIN, INT8_MAX, true)
