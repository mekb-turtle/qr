#include "qr_table.h"
const uint16_t character_capacity[QR_VERSION_NUM * QR_ECL_NUM][QR_MODE_NUM] = {
        {41,   25,   17,   10  },
        {34,   20,   14,   8   },
        {27,   16,   11,   7   },
        {17,   10,   7,    4   },
        {77,   47,   32,   20  },
        {63,   38,   26,   16  },
        {48,   29,   20,   12  },
        {34,   20,   14,   8   },
        {127,  77,   53,   32  },
        {101,  61,   42,   26  },
        {77,   47,   32,   20  },
        {58,   35,   24,   15  },
        {187,  114,  78,   48  },
        {149,  90,   62,   38  },
        {111,  67,   46,   28  },
        {82,   50,   34,   21  },
        {255,  154,  106,  65  },
        {202,  122,  84,   52  },
        {144,  87,   60,   37  },
        {106,  64,   44,   27  },
        {322,  195,  134,  82  },
        {255,  154,  106,  65  },
        {178,  108,  74,   45  },
        {139,  84,   58,   36  },
        {370,  224,  154,  95  },
        {293,  178,  122,  75  },
        {207,  125,  86,   53  },
        {154,  93,   64,   39  },
        {461,  279,  192,  118 },
        {365,  221,  152,  93  },
        {259,  157,  108,  66  },
        {202,  122,  84,   52  },
        {552,  335,  230,  141 },
        {432,  262,  180,  111 },
        {312,  189,  130,  80  },
        {235,  143,  98,   60  },
        {652,  395,  271,  167 },
        {513,  311,  213,  131 },
        {364,  221,  151,  93  },
        {288,  174,  119,  74  },
        {772,  468,  321,  198 },
        {604,  366,  251,  155 },
        {427,  259,  177,  109 },
        {331,  200,  137,  85  },
        {883,  535,  367,  226 },
        {691,  419,  287,  177 },
        {489,  296,  203,  125 },
        {374,  227,  155,  96  },
        {1022, 619,  425,  262 },
        {796,  483,  331,  204 },
        {580,  352,  241,  149 },
        {427,  259,  177,  109 },
        {1101, 667,  458,  282 },
        {871,  528,  362,  223 },
        {621,  376,  258,  159 },
        {468,  283,  194,  120 },
        {1250, 758,  520,  320 },
        {991,  600,  412,  254 },
        {703,  426,  292,  180 },
        {530,  321,  220,  136 },
        {1408, 854,  586,  361 },
        {1082, 656,  450,  277 },
        {775,  470,  322,  198 },
        {602,  365,  250,  154 },
        {1548, 938,  644,  397 },
        {1212, 734,  504,  310 },
        {876,  531,  364,  224 },
        {674,  408,  280,  173 },
        {1725, 1046, 718,  442 },
        {1346, 816,  560,  345 },
        {948,  574,  394,  243 },
        {746,  452,  310,  191 },
        {1903, 1153, 792,  488 },
        {1500, 909,  624,  384 },
        {1063, 644,  442,  272 },
        {813,  493,  338,  208 },
        {2061, 1249, 858,  528 },
        {1600, 970,  666,  410 },
        {1159, 702,  482,  297 },
        {919,  557,  382,  235 },
        {2232, 1352, 929,  572 },
        {1708, 1035, 711,  438 },
        {1224, 742,  509,  314 },
        {969,  587,  403,  248 },
        {2409, 1460, 1003, 618 },
        {1872, 1134, 779,  480 },
        {1358, 823,  565,  348 },
        {1056, 640,  439,  270 },
        {2620, 1588, 1091, 672 },
        {2059, 1248, 857,  528 },
        {1468, 890,  611,  376 },
        {1108, 672,  461,  284 },
        {2812, 1704, 1171, 721 },
        {2188, 1326, 911,  561 },
        {1588, 963,  661,  407 },
        {1228, 744,  511,  315 },
        {3057, 1853, 1273, 784 },
        {2395, 1451, 997,  614 },
        {1718, 1041, 715,  440 },
        {1286, 779,  535,  330 },
        {3283, 1990, 1367, 842 },
        {2544, 1542, 1059, 652 },
        {1804, 1094, 751,  462 },
        {1425, 864,  593,  365 },
        {3517, 2132, 1465, 902 },
        {2701, 1637, 1125, 692 },
        {1933, 1172, 805,  496 },
        {1501, 910,  625,  385 },
        {3669, 2223, 1528, 940 },
        {2857, 1732, 1190, 732 },
        {2085, 1263, 868,  534 },
        {1581, 958,  658,  405 },
        {3909, 2369, 1628, 1002},
        {3035, 1839, 1264, 778 },
        {2181, 1322, 908,  559 },
        {1677, 1016, 698,  430 },
        {4158, 2520, 1732, 1066},
        {3289, 1994, 1370, 843 },
        {2358, 1429, 982,  604 },
        {1782, 1080, 742,  457 },
        {4417, 2677, 1840, 1132},
        {3486, 2113, 1452, 894 },
        {2473, 1499, 1030, 634 },
        {1897, 1150, 790,  486 },
        {4686, 2840, 1952, 1201},
        {3693, 2238, 1538, 947 },
        {2670, 1618, 1112, 684 },
        {2022, 1226, 842,  518 },
        {4965, 3009, 2068, 1273},
        {3909, 2369, 1628, 1002},
        {2805, 1700, 1168, 719 },
        {2157, 1307, 898,  553 },
        {5253, 3183, 2188, 1347},
        {4134, 2506, 1722, 1060},
        {2949, 1787, 1228, 756 },
        {2301, 1394, 958,  590 },
        {5529, 3351, 2303, 1417},
        {4343, 2632, 1809, 1113},
        {3081, 1867, 1283, 790 },
        {2361, 1431, 983,  605 },
        {5836, 3537, 2431, 1496},
        {4588, 2780, 1911, 1176},
        {3244, 1966, 1351, 832 },
        {2524, 1530, 1051, 647 },
        {6153, 3729, 2563, 1577},
        {4775, 2894, 1989, 1224},
        {3417, 2071, 1423, 876 },
        {2625, 1591, 1093, 673 },
        {6479, 3927, 2699, 1661},
        {5039, 3054, 2099, 1292},
        {3599, 2181, 1499, 923 },
        {2735, 1658, 1139, 701 },
        {6743, 4087, 2809, 1729},
        {5313, 3220, 2213, 1362},
        {3791, 2298, 1579, 972 },
        {2927, 1774, 1219, 750 },
        {7089, 4296, 2953, 1817},
        {5596, 3391, 2331, 1435},
        {3993, 2420, 1663, 1024},
        {3057, 1852, 1273, 784 },
};
const struct ec_row error_correction[QR_VERSION_NUM * QR_ECL_NUM] = {
        {7,  1,  19,  0,  0  },
        {10, 1,  16,  0,  0  },
        {13, 1,  13,  0,  0  },
        {17, 1,  9,   0,  0  },
        {10, 1,  34,  0,  0  },
        {16, 1,  28,  0,  0  },
        {22, 1,  22,  0,  0  },
        {28, 1,  16,  0,  0  },
        {15, 1,  55,  0,  0  },
        {26, 1,  44,  0,  0  },
        {18, 2,  17,  0,  0  },
        {22, 2,  13,  0,  0  },
        {20, 1,  80,  0,  0  },
        {18, 2,  32,  0,  0  },
        {26, 2,  24,  0,  0  },
        {16, 4,  9,   0,  0  },
        {26, 1,  108, 0,  0  },
        {24, 2,  43,  0,  0  },
        {18, 2,  15,  2,  16 },
        {22, 2,  11,  2,  12 },
        {18, 2,  68,  0,  0  },
        {16, 4,  27,  0,  0  },
        {24, 4,  19,  0,  0  },
        {28, 4,  15,  0,  0  },
        {20, 2,  78,  0,  0  },
        {18, 4,  31,  0,  0  },
        {18, 2,  14,  4,  15 },
        {26, 4,  13,  1,  14 },
        {24, 2,  97,  0,  0  },
        {22, 2,  38,  2,  39 },
        {22, 4,  18,  2,  19 },
        {26, 4,  14,  2,  15 },
        {30, 2,  116, 0,  0  },
        {22, 3,  36,  2,  37 },
        {20, 4,  16,  4,  17 },
        {24, 4,  12,  4,  13 },
        {18, 2,  68,  2,  69 },
        {26, 4,  43,  1,  44 },
        {24, 6,  19,  2,  20 },
        {28, 6,  15,  2,  16 },
        {20, 4,  81,  0,  0  },
        {30, 1,  50,  4,  51 },
        {28, 4,  22,  4,  23 },
        {24, 3,  12,  8,  13 },
        {24, 2,  92,  2,  93 },
        {22, 6,  36,  2,  37 },
        {26, 4,  20,  6,  21 },
        {28, 7,  14,  4,  15 },
        {26, 4,  107, 0,  0  },
        {22, 8,  37,  1,  38 },
        {24, 8,  20,  4,  21 },
        {22, 12, 11,  4,  12 },
        {30, 3,  115, 1,  116},
        {24, 4,  40,  5,  41 },
        {20, 11, 16,  5,  17 },
        {24, 11, 12,  5,  13 },
        {22, 5,  87,  1,  88 },
        {24, 5,  41,  5,  42 },
        {30, 5,  24,  7,  25 },
        {24, 11, 12,  7,  13 },
        {24, 5,  98,  1,  99 },
        {28, 7,  45,  3,  46 },
        {24, 15, 19,  2,  20 },
        {30, 3,  15,  13, 16 },
        {28, 1,  107, 5,  108},
        {28, 10, 46,  1,  47 },
        {28, 1,  22,  15, 23 },
        {28, 2,  14,  17, 15 },
        {30, 5,  120, 1,  121},
        {26, 9,  43,  4,  44 },
        {28, 17, 22,  1,  23 },
        {28, 2,  14,  19, 15 },
        {28, 3,  113, 4,  114},
        {26, 3,  44,  11, 45 },
        {26, 17, 21,  4,  22 },
        {26, 9,  13,  16, 14 },
        {28, 3,  107, 5,  108},
        {26, 3,  41,  13, 42 },
        {30, 15, 24,  5,  25 },
        {28, 15, 15,  10, 16 },
        {28, 4,  116, 4,  117},
        {26, 17, 42,  0,  0  },
        {28, 17, 22,  6,  23 },
        {30, 19, 16,  6,  17 },
        {28, 2,  111, 7,  112},
        {28, 17, 46,  0,  0  },
        {30, 7,  24,  16, 25 },
        {24, 34, 13,  0,  0  },
        {30, 4,  121, 5,  122},
        {28, 4,  47,  14, 48 },
        {30, 11, 24,  14, 25 },
        {30, 16, 15,  14, 16 },
        {30, 6,  117, 4,  118},
        {28, 6,  45,  14, 46 },
        {30, 11, 24,  16, 25 },
        {30, 30, 16,  2,  17 },
        {26, 8,  106, 4,  107},
        {28, 8,  47,  13, 48 },
        {30, 7,  24,  22, 25 },
        {30, 22, 15,  13, 16 },
        {28, 10, 114, 2,  115},
        {28, 19, 46,  4,  47 },
        {28, 28, 22,  6,  23 },
        {30, 33, 16,  4,  17 },
        {30, 8,  122, 4,  123},
        {28, 22, 45,  3,  46 },
        {30, 8,  23,  26, 24 },
        {30, 12, 15,  28, 16 },
        {30, 3,  117, 10, 118},
        {28, 3,  45,  23, 46 },
        {30, 4,  24,  31, 25 },
        {30, 11, 15,  31, 16 },
        {30, 7,  116, 7,  117},
        {28, 21, 45,  7,  46 },
        {30, 1,  23,  37, 24 },
        {30, 19, 15,  26, 16 },
        {30, 5,  115, 10, 116},
        {28, 19, 47,  10, 48 },
        {30, 15, 24,  25, 25 },
        {30, 23, 15,  25, 16 },
        {30, 13, 115, 3,  116},
        {28, 2,  46,  29, 47 },
        {30, 42, 24,  1,  25 },
        {30, 23, 15,  28, 16 },
        {30, 17, 115, 0,  0  },
        {28, 10, 46,  23, 47 },
        {30, 10, 24,  35, 25 },
        {30, 19, 15,  35, 16 },
        {30, 17, 115, 1,  116},
        {28, 14, 46,  21, 47 },
        {30, 29, 24,  19, 25 },
        {30, 11, 15,  46, 16 },
        {30, 13, 115, 6,  116},
        {28, 14, 46,  23, 47 },
        {30, 44, 24,  7,  25 },
        {30, 59, 16,  1,  17 },
        {30, 12, 121, 7,  122},
        {28, 12, 47,  26, 48 },
        {30, 39, 24,  14, 25 },
        {30, 22, 15,  41, 16 },
        {30, 6,  121, 14, 122},
        {28, 6,  47,  34, 48 },
        {30, 46, 24,  10, 25 },
        {30, 2,  15,  64, 16 },
        {30, 17, 122, 4,  123},
        {28, 29, 46,  14, 47 },
        {30, 49, 24,  10, 25 },
        {30, 24, 15,  46, 16 },
        {30, 4,  122, 18, 123},
        {28, 13, 46,  32, 47 },
        {30, 48, 24,  14, 25 },
        {30, 42, 15,  32, 16 },
        {30, 20, 117, 4,  118},
        {28, 40, 47,  7,  48 },
        {30, 43, 24,  22, 25 },
        {30, 10, 15,  67, 16 },
        {30, 19, 118, 6,  119},
        {28, 18, 47,  31, 48 },
        {30, 34, 24,  34, 25 },
        {30, 20, 15,  61, 16 },
};
const uint8_t *generator_polynomials[] = {
        [7] = (uint8_t[]){0, 87, 229, 146, 149, 238, 102, 21},
        [10] = (uint8_t[]){0, 251, 67, 46, 61, 118, 70, 64, 94, 32, 45},
        [13] = (uint8_t[]){0, 74, 152, 176, 100, 86, 100, 106, 104, 130, 218, 206, 140, 78},
        [15] = (uint8_t[]){0, 8, 183, 61, 91, 202, 37, 51, 58, 58, 237, 140, 124, 5, 99, 105},
        [16] = (uint8_t[]){0, 120, 104, 107, 109, 102, 161, 76, 3, 91, 191, 147, 169, 182, 194, 225, 120},
        [17] = (uint8_t[]){0, 43, 139, 206, 78, 43, 239, 123, 206, 214, 147, 24, 99, 150, 39, 243, 163, 136},
        [18] = (uint8_t[]){0, 215, 234, 158, 94, 184, 97, 118, 170, 79, 187, 152, 148, 252, 179, 5, 98, 96, 153},
        [20] = (uint8_t[]){0, 17, 60, 79, 50, 61, 163, 26, 187, 202, 180, 221, 225, 83, 239, 156, 164, 212, 212, 188, 190},
        [22] = (uint8_t[]){0, 210, 171, 247, 242, 93, 230, 14, 109, 221, 53, 200, 74, 8, 172, 98, 80, 219, 134, 160, 105, 165, 231},
        [24] = (uint8_t[]){0, 229, 121, 135, 48, 211, 117, 251, 126, 159, 180, 169, 152, 192, 226, 228, 218, 111, 0, 117, 232, 87, 96, 227, 21},
        [26] = (uint8_t[]){0, 173, 125, 158, 2, 103, 182, 118, 17, 145, 201, 111, 28, 165, 53, 161, 21, 245, 142, 13, 102, 48, 227, 153, 145, 218, 70},
        [28] = (uint8_t[]){0, 168, 223, 200, 104, 224, 234, 108, 180, 110, 190, 195, 147, 205, 27, 232, 201, 21, 43, 245, 87, 42, 195, 212, 119, 242, 37, 9, 123},
        [30] = (uint8_t[]){0, 41, 173, 145, 152, 216, 31, 179, 182, 50, 48, 110, 86, 239, 96, 222, 125, 42, 173, 226, 193, 224, 130, 156, 37, 251, 216, 238, 40, 192, 180},
};