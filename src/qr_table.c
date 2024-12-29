#include "qr_table.h"
const uint16_t character_capacity[QR_MAX_VERSION * 4][4] = {
        {41,   25,   17,   10  }, // Low, version 1
        {77,   47,   32,   20  }, // Low, version 2
        {127,  77,   53,   32  }, // Low, version 3
        {187,  114,  78,   48  }, // Low, version 4
        {255,  154,  106,  65  }, // Low, version 5
        {322,  195,  134,  82  }, // Low, version 6
        {370,  224,  154,  95  }, // Low, version 7
        {461,  279,  192,  118 }, // Low, version 8
        {552,  335,  230,  141 }, // Low, version 9
        {652,  395,  271,  167 }, // Low, version 10
        {772,  468,  321,  198 }, // Low, version 11
        {883,  535,  367,  226 }, // Low, version 12
        {1022, 619,  425,  262 }, // Low, version 13
        {1101, 667,  458,  282 }, // Low, version 14
        {1250, 758,  520,  320 }, // Low, version 15
        {1408, 854,  586,  361 }, // Low, version 16
        {1548, 938,  644,  397 }, // Low, version 17
        {1725, 1046, 718,  442 }, // Low, version 18
        {1903, 1153, 792,  488 }, // Low, version 19
        {2061, 1249, 858,  528 }, // Low, version 20
        {2232, 1352, 929,  572 }, // Low, version 21
        {2409, 1460, 1003, 618 }, // Low, version 22
        {2620, 1588, 1091, 672 }, // Low, version 23
        {2812, 1704, 1171, 721 }, // Low, version 24
        {3057, 1853, 1273, 784 }, // Low, version 25
        {3283, 1990, 1367, 842 }, // Low, version 26
        {3517, 2132, 1465, 902 }, // Low, version 27
        {3669, 2223, 1528, 940 }, // Low, version 28
        {3909, 2369, 1628, 1002}, // Low, version 29
        {4158, 2520, 1732, 1066}, // Low, version 30
        {4417, 2677, 1840, 1132}, // Low, version 31
        {4686, 2840, 1952, 1201}, // Low, version 32
        {4965, 3009, 2068, 1273}, // Low, version 33
        {5253, 3183, 2188, 1347}, // Low, version 34
        {5529, 3351, 2303, 1417}, // Low, version 35
        {5836, 3537, 2431, 1496}, // Low, version 36
        {6153, 3729, 2563, 1577}, // Low, version 37
        {6479, 3927, 2699, 1661}, // Low, version 38
        {6743, 4087, 2809, 1729}, // Low, version 39
        {7089, 4296, 2953, 1817}, // Low, version 40
        {34,   20,   14,   8   }, // Medium, version 1
        {63,   38,   26,   16  }, // Medium, version 2
        {101,  61,   42,   26  }, // Medium, version 3
        {149,  90,   62,   38  }, // Medium, version 4
        {202,  122,  84,   52  }, // Medium, version 5
        {255,  154,  106,  65  }, // Medium, version 6
        {293,  178,  122,  75  }, // Medium, version 7
        {365,  221,  152,  93  }, // Medium, version 8
        {432,  262,  180,  111 }, // Medium, version 9
        {513,  311,  213,  131 }, // Medium, version 10
        {604,  366,  251,  155 }, // Medium, version 11
        {691,  419,  287,  177 }, // Medium, version 12
        {796,  483,  331,  204 }, // Medium, version 13
        {871,  528,  362,  223 }, // Medium, version 14
        {991,  600,  412,  254 }, // Medium, version 15
        {1082, 656,  450,  277 }, // Medium, version 16
        {1212, 734,  504,  310 }, // Medium, version 17
        {1346, 816,  560,  345 }, // Medium, version 18
        {1500, 909,  624,  384 }, // Medium, version 19
        {1600, 970,  666,  410 }, // Medium, version 20
        {1708, 1035, 711,  438 }, // Medium, version 21
        {1872, 1134, 779,  480 }, // Medium, version 22
        {2059, 1248, 857,  528 }, // Medium, version 23
        {2188, 1326, 911,  561 }, // Medium, version 24
        {2395, 1451, 997,  614 }, // Medium, version 25
        {2544, 1542, 1059, 652 }, // Medium, version 26
        {2701, 1637, 1125, 692 }, // Medium, version 27
        {2857, 1732, 1190, 732 }, // Medium, version 28
        {3035, 1839, 1264, 778 }, // Medium, version 29
        {3289, 1994, 1370, 843 }, // Medium, version 30
        {3486, 2113, 1452, 894 }, // Medium, version 31
        {3693, 2238, 1538, 947 }, // Medium, version 32
        {3909, 2369, 1628, 1002}, // Medium, version 33
        {4134, 2506, 1722, 1060}, // Medium, version 34
        {4343, 2632, 1809, 1113}, // Medium, version 35
        {4588, 2780, 1911, 1176}, // Medium, version 36
        {4775, 2894, 1989, 1224}, // Medium, version 37
        {5039, 3054, 2099, 1292}, // Medium, version 38
        {5313, 3220, 2213, 1362}, // Medium, version 39
        {5596, 3391, 2331, 1435}, // Medium, version 40
        {27,   16,   11,   7   }, // Quartile, version 1
        {48,   29,   20,   12  }, // Quartile, version 2
        {77,   47,   32,   20  }, // Quartile, version 3
        {111,  67,   46,   28  }, // Quartile, version 4
        {144,  87,   60,   37  }, // Quartile, version 5
        {178,  108,  74,   45  }, // Quartile, version 6
        {207,  125,  86,   53  }, // Quartile, version 7
        {259,  157,  108,  66  }, // Quartile, version 8
        {312,  189,  130,  80  }, // Quartile, version 9
        {364,  221,  151,  93  }, // Quartile, version 10
        {427,  259,  177,  109 }, // Quartile, version 11
        {489,  296,  203,  125 }, // Quartile, version 12
        {580,  352,  241,  149 }, // Quartile, version 13
        {621,  376,  258,  159 }, // Quartile, version 14
        {703,  426,  292,  180 }, // Quartile, version 15
        {775,  470,  322,  198 }, // Quartile, version 16
        {876,  531,  364,  224 }, // Quartile, version 17
        {948,  574,  394,  243 }, // Quartile, version 18
        {1063, 644,  442,  272 }, // Quartile, version 19
        {1159, 702,  482,  297 }, // Quartile, version 20
        {1224, 742,  509,  314 }, // Quartile, version 21
        {1358, 823,  565,  348 }, // Quartile, version 22
        {1468, 890,  611,  376 }, // Quartile, version 23
        {1588, 963,  661,  407 }, // Quartile, version 24
        {1718, 1041, 715,  440 }, // Quartile, version 25
        {1804, 1094, 751,  462 }, // Quartile, version 26
        {1933, 1172, 805,  496 }, // Quartile, version 27
        {2085, 1263, 868,  534 }, // Quartile, version 28
        {2181, 1322, 908,  559 }, // Quartile, version 29
        {2358, 1429, 982,  604 }, // Quartile, version 30
        {2473, 1499, 1030, 634 }, // Quartile, version 31
        {2670, 1618, 1112, 684 }, // Quartile, version 32
        {2805, 1700, 1168, 719 }, // Quartile, version 33
        {2949, 1787, 1228, 756 }, // Quartile, version 34
        {3081, 1867, 1283, 790 }, // Quartile, version 35
        {3244, 1966, 1351, 832 }, // Quartile, version 36
        {3417, 2071, 1423, 876 }, // Quartile, version 37
        {3599, 2181, 1499, 923 }, // Quartile, version 38
        {3791, 2298, 1579, 972 }, // Quartile, version 39
        {3993, 2420, 1663, 1024}, // Quartile, version 40
        {17,   10,   7,    4   }, // High, version 1
        {34,   20,   14,   8   }, // High, version 2
        {58,   35,   24,   15  }, // High, version 3
        {82,   50,   34,   21  }, // High, version 4
        {106,  64,   44,   27  }, // High, version 5
        {139,  84,   58,   36  }, // High, version 6
        {154,  93,   64,   39  }, // High, version 7
        {202,  122,  84,   52  }, // High, version 8
        {235,  143,  98,   60  }, // High, version 9
        {288,  174,  119,  74  }, // High, version 10
        {331,  200,  137,  85  }, // High, version 11
        {374,  227,  155,  96  }, // High, version 12
        {427,  259,  177,  109 }, // High, version 13
        {468,  283,  194,  120 }, // High, version 14
        {530,  321,  220,  136 }, // High, version 15
        {602,  365,  250,  154 }, // High, version 16
        {674,  408,  280,  173 }, // High, version 17
        {746,  452,  310,  191 }, // High, version 18
        {813,  493,  338,  208 }, // High, version 19
        {919,  557,  382,  235 }, // High, version 20
        {969,  587,  403,  248 }, // High, version 21
        {1056, 640,  439,  270 }, // High, version 22
        {1108, 672,  461,  284 }, // High, version 23
        {1228, 744,  511,  315 }, // High, version 24
        {1286, 779,  535,  330 }, // High, version 25
        {1425, 864,  593,  365 }, // High, version 26
        {1501, 910,  625,  385 }, // High, version 27
        {1581, 958,  658,  405 }, // High, version 28
        {1677, 1016, 698,  430 }, // High, version 29
        {1782, 1080, 742,  457 }, // High, version 30
        {1897, 1150, 790,  486 }, // High, version 31
        {2022, 1226, 842,  518 }, // High, version 32
        {2157, 1307, 898,  553 }, // High, version 33
        {2301, 1394, 958,  590 }, // High, version 34
        {2361, 1431, 983,  605 }, // High, version 35
        {2524, 1530, 1051, 647 }, // High, version 36
        {2625, 1591, 1093, 673 }, // High, version 37
        {2735, 1658, 1139, 701 }, // High, version 38
        {2927, 1774, 1219, 750 }, // High, version 39
        {3057, 1852, 1273, 784 }, // High, version 40
};
