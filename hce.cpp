#include "hce.h"
#include "logarithm.h"
#include "attacks.h"

namespace hce {
    constexpr phase_t PHASE_PIECE_VALUES[6] = {0,1,1,2,4,0};
    constexpr phase_t MAX_PHASE = 24;

    constexpr packed_eval_t S(int16_t mg, int16_t eg) {
        return (uint32_t(uint16_t(mg)) << 16) + eg;
    }

    constexpr packed_eval_t mobility[6] = {S(-13, -25), S(0, 2), S(9, 12), S(8, 6), S(2, 12), S(-17, -3)};

    constexpr auto NUM_KING_BUCKETS = 2;

    inline auto getFriendlyKingBucket(square_t kingSquare, side_t side) {
        return kingSquare / 32;
    }

    // PSTs are bucketed by the friendly king
    // There are only two buckets:
    // queenside and kingside

    // Note that these PSTs are not the actual PSTs that we will use
    // They are the PSTs output from python-chess, which uses a1 = 0, h1 = 7, a8 = 56, h8 = 63
    // I use a1 = 0, a8 = 7, h1 = 56, h8 = 63
    // Which is reflected across the a1-h8 diagonal from how python-chess does it
    // Anyway, you can visualize these PSTs by pretending they are squares on a board
    // And each entry is S(mg, eg) for a black piece on that square
    constexpr std::array<std::array<std::array<packed_eval_t, 64>, 6>, 2> psts = {{
                                                                                          {{
                                                                                                   // Friendly king on the queenside
                                                                                                   {
                                                                                                           S(  0,  0), S(  0,  0), S(  0,  0), S(  0,  0), S(  0,  0), S(  0,  0), S(  0,  0), S(  0,  0),
                                                                                                           S(161,247), S(246,260), S(197,245), S(137,264), S( 93,284), S(101,277), S( 68,305), S(  4,288),
                                                                                                           S(172,236), S(201,259), S(189,235), S(122,270), S(135,251), S( 99,256), S( 66,273), S(  2,260),
                                                                                                           S(149,270), S(198,286), S(184,252), S(166,261), S(153,251), S(120,254), S(103,277), S( 16,258),
                                                                                                           S(168,333), S(212,342), S(192,303), S(148,294), S(177,269), S(155,263), S(118,301), S( 38,294),
                                                                                                           S(187,492), S(207,548), S(172,502), S(195,436), S(205,413), S(207,360), S(136,434), S( 39,408),
                                                                                                           S(229,618), S(508,506), S(278,666), S(308,545), S(202,570), S(184,511), S( 94,576), S(-22,592),
                                                                                                           S(  0,  0), S(  0,  0), S(  0,  0), S(  0,  0), S(  0,  0), S(  0,  0), S(  0,  0), S(  0,  0),
                                                                                                   },
                                                                                                   {
                                                                                                           S(324,552), S(367,580), S(382,600), S(395,605), S(387,586), S(392,577), S(394,563), S(316,499),
                                                                                                           S(401,566), S(354,626), S(439,612), S(459,622), S(450,611), S(445,615), S(425,583), S(394,557),
                                                                                                           S(408,592), S(434,608), S(498,597), S(491,653), S(489,648), S(453,626), S(441,593), S(414,590),
                                                                                                           S(450,593), S(466,617), S(508,647), S(502,656), S(537,669), S(490,653), S(514,634), S(446,581),
                                                                                                           S(462,588), S(505,627), S(549,654), S(613,632), S(551,658), S(564,666), S(504,625), S(486,610),
                                                                                                           S(407,601), S(515,600), S(548,624), S(595,628), S(615,634), S(608,597), S(588,588), S(418,608),
                                                                                                           S(381,557), S(452,596), S(511,583), S(573,596), S(509,595), S(539,606), S(447,629), S(545,544),
                                                                                                           S(211,426), S(252,530), S(347,610), S(383,567), S(397,627), S(299,598), S(352,570), S(178,488),
                                                                                                   },
                                                                                                   {
                                                                                                           S(476,576), S(436,612), S(455,598), S(444,598), S(469,584), S(469,583), S(453,598), S(451,550),
                                                                                                           S(454,618), S(540,561), S(529,559), S(488,578), S(471,587), S(488,571), S(461,583), S(450,564),
                                                                                                           S(479,598), S(487,561), S(520,570), S(498,575), S(495,584), S(463,591), S(460,583), S(442,571),
                                                                                                           S(454,587), S(498,582), S(488,579), S(541,570), S(511,567), S(467,596), S(458,580), S(459,602),
                                                                                                           S(462,595), S(479,596), S(503,584), S(521,577), S(531,572), S(513,570), S(477,605), S(489,582),
                                                                                                           S(421,636), S(521,579), S(482,591), S(542,566), S(490,560), S(547,577), S(527,574), S(508,593),
                                                                                                           S(364,576), S(401,596), S(472,571), S(419,566), S(397,594), S(525,553), S(531,574), S(486,569),
                                                                                                           S(345,584), S(341,605), S(364,607), S(274,634), S(300,625), S(194,621), S(421,583), S(315,625),
                                                                                                   },
                                                                                                   {
                                                                                                           S( 460,1097), S( 504,1099), S( 528,1104), S( 555,1086), S( 564,1070), S( 551,1056), S( 560,1044), S( 524,1073),
                                                                                                           S( 515,1054), S( 493,1076), S( 535,1083), S( 523,1097), S( 516,1081), S( 515,1078), S( 531,1061), S( 489,1061),
                                                                                                           S( 530,1073), S( 486,1103), S( 521,1089), S( 516,1084), S( 496,1098), S( 509,1074), S( 518,1054), S( 485,1061),
                                                                                                           S( 478,1093), S( 498,1104), S( 506,1107), S( 528,1101), S( 484,1116), S( 457,1108), S( 502,1087), S( 503,1087),
                                                                                                           S( 510,1105), S( 493,1118), S( 525,1122), S( 533,1116), S( 519,1098), S( 569,1071), S( 563,1076), S( 539,1093),
                                                                                                           S( 518,1101), S( 537,1117), S( 560,1112), S( 583,1082), S( 593,1079), S( 566,1097), S( 640,1073), S( 620,1075),
                                                                                                           S( 598,1082), S( 606,1107), S( 622,1111), S( 653,1092), S( 641,1093), S( 648,1078), S( 598,1100), S( 660,1079),
                                                                                                           S( 596,1088), S( 410,1150), S( 591,1128), S( 574,1110), S( 505,1138), S( 550,1107), S( 632,1085), S( 570,1122),
                                                                                                   },
                                                                                                   {
                                                                                                           S(1029,1714), S( 941,1901), S( 995,1931), S(1017,1912), S(1041,1919), S(1039,1889), S(1006,1964), S(1045,1849),
                                                                                                           S( 970,1912), S(1015,1890), S(1050,1916), S(1054,1977), S(1043,1957), S(1028,1964), S(1042,1918), S(1097,1871),
                                                                                                           S( 975,2000), S(1002,2023), S(1023,2007), S(1023,1997), S(1040,1976), S(1010,2005), S(1036,1947), S(1016,2029),
                                                                                                           S(1017,1950), S(1009,2015), S(1023,1992), S(1065,2017), S(1050,1971), S(1019,1999), S(1025,1983), S(1031,2030),
                                                                                                           S(1001,2015), S( 983,2052), S( 999,2042), S(1023,2041), S(1044,2068), S(1046,2012), S(1044,2023), S(1084,1995),
                                                                                                           S( 982,2026), S( 994,2025), S( 975,2100), S(1052,2053), S(1065,2050), S(1116,2017), S(1173,1914), S(1147,1980),
                                                                                                           S(1015,1960), S( 967,2014), S( 965,2097), S( 922,2131), S( 932,2154), S(1068,1995), S(1089,1953), S(1108,1990),
                                                                                                           S( 956,1882), S(1078,1861), S(1170,1872), S(1111,1939), S(1169,1890), S(1229,1815), S(1177,1802), S(1037,1940),
                                                                                                   },
                                                                                                   {
                                                                                                           S( 189,-163), S( 247,-134), S( 198, -94), S( 118, -89), S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0),
                                                                                                           S( 218, -96), S( 221, -61), S( 187, -37), S( 162, -27), S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0),
                                                                                                           S(  97, -84), S( 153, -40), S(  79,  -3), S(  62,  23), S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0),
                                                                                                           S(  42, -74), S( 115, -23), S(  33,  29), S(  -2,  59), S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0),
                                                                                                           S(  44, -55), S(  88,  11), S(  69,  51), S( -52,  84), S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0),
                                                                                                           S(-108, -16), S( 257,  11), S( 137,  46), S( 132,  64), S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0),
                                                                                                           S(  39, -58), S( 179,   4), S(  68,  37), S( 255,   5), S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0),
                                                                                                           S( 561,-299), S( 463,-185), S( 536,-157), S( 352, -85), S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0),
                                                                                                   },
                                                                                           }},
                                                                                          {{
                                                                                                   // Friendly king on the kingside
                                                                                                   {
                                                                                                           S(  0,  0), S(  0,  0), S(  0,  0), S(  0,  0), S(  0,  0), S(  0,  0), S(  0,  0), S(  0,  0),
                                                                                                           S( 73,267), S(119,287), S(107,266), S(100,276), S(137,293), S(170,260), S(228,247), S(126,204),
                                                                                                           S( 64,250), S(107,265), S(115,247), S(131,263), S(154,250), S(147,245), S(195,240), S(149,203),
                                                                                                           S( 67,263), S(127,276), S(128,248), S(160,239), S(169,237), S(162,238), S(177,258), S(128,221),
                                                                                                           S( 86,308), S(142,304), S(145,274), S(159,261), S(198,246), S(181,261), S(195,287), S(151,266),
                                                                                                           S(113,427), S(155,466), S(217,410), S(221,391), S(236,368), S(273,357), S(245,445), S(145,424),
                                                                                                           S(263,526), S(291,534), S(290,530), S(320,478), S(309,474), S(265,510), S(133,582), S( 72,581),
                                                                                                           S(  0,  0), S(  0,  0), S(  0,  0), S(  0,  0), S(  0,  0), S(  0,  0), S(  0,  0), S(  0,  0),
                                                                                                   },
                                                                                                   {
                                                                                                           S(323,556), S(425,553), S(406,592), S(440,605), S(443,609), S(445,591), S(433,571), S(359,572),
                                                                                                           S(397,577), S(416,612), S(448,630), S(475,632), S(480,627), S(477,615), S(446,598), S(436,607),
                                                                                                           S(406,587), S(445,621), S(476,633), S(497,671), S(519,657), S(495,622), S(485,606), S(444,591),
                                                                                                           S(438,622), S(469,643), S(504,679), S(511,675), S(532,683), S(516,654), S(511,637), S(466,606),
                                                                                                           S(457,631), S(491,650), S(532,673), S(575,683), S(544,681), S(595,669), S(518,661), S(536,607),
                                                                                                           S(460,611), S(525,634), S(578,657), S(587,655), S(648,628), S(671,619), S(585,612), S(538,582),
                                                                                                           S(423,592), S(474,619), S(506,636), S(553,630), S(517,622), S(635,588), S(485,601), S(514,547),
                                                                                                           S(211,499), S(246,615), S(338,628), S(378,624), S(464,612), S(360,578), S(330,580), S(329,431),
                                                                                                   },
                                                                                                   {
                                                                                                           S(478,572), S(517,594), S(499,573), S(476,594), S(485,595), S(468,610), S(499,558), S(490,541),
                                                                                                           S(500,590), S(509,578), S(514,570), S(481,602), S(504,596), S(514,582), S(539,584), S(498,562),
                                                                                                           S(481,597), S(490,596), S(497,604), S(494,599), S(499,603), S(503,592), S(492,581), S(519,580),
                                                                                                           S(475,591), S(469,605), S(483,609), S(527,593), S(525,582), S(488,591), S(480,596), S(497,571),
                                                                                                           S(470,606), S(493,604), S(516,590), S(544,598), S(535,584), S(521,598), S(497,594), S(484,605),
                                                                                                           S(490,609), S(517,593), S(518,584), S(543,572), S(524,587), S(579,583), S(553,596), S(534,610),
                                                                                                           S(457,578), S(506,584), S(479,594), S(452,596), S(499,582), S(499,576), S(492,592), S(485,568),
                                                                                                           S(437,603), S(404,612), S(379,608), S(277,640), S(352,620), S(383,603), S(414,594), S(408,574),
                                                                                                   },
                                                                                                   {
                                                                                                           S( 578,1115), S( 577,1116), S( 600,1123), S( 615,1116), S( 623,1105), S( 598,1107), S( 605,1099), S( 593,1072),
                                                                                                           S( 535,1121), S( 544,1126), S( 582,1120), S( 587,1119), S( 599,1101), S( 599,1092), S( 641,1064), S( 584,1087),
                                                                                                           S( 526,1128), S( 528,1121), S( 545,1126), S( 562,1128), S( 577,1107), S( 579,1093), S( 627,1055), S( 605,1067),
                                                                                                           S( 526,1136), S( 518,1142), S( 540,1143), S( 555,1133), S( 574,1122), S( 562,1116), S( 595,1105), S( 577,1097),
                                                                                                           S( 542,1151), S( 555,1144), S( 561,1155), S( 581,1138), S( 594,1110), S( 611,1109), S( 610,1112), S( 633,1101),
                                                                                                           S( 561,1147), S( 600,1144), S( 598,1148), S( 606,1138), S( 662,1117), S( 681,1097), S( 761,1092), S( 706,1083),
                                                                                                           S( 591,1146), S( 593,1162), S( 626,1170), S( 658,1153), S( 641,1151), S( 726,1115), S( 694,1122), S( 749,1085),
                                                                                                           S( 611,1141), S( 607,1145), S( 597,1165), S( 593,1162), S( 647,1135), S( 637,1151), S( 583,1167), S( 716,1105),
                                                                                                   },
                                                                                                   {
                                                                                                           S(1050,2059), S(1058,2055), S(1081,2049), S(1093,2082), S(1087,2047), S(1039,2064), S(1092,1961), S(1064,2006),
                                                                                                           S(1066,2057), S(1068,2071), S(1086,2064), S(1086,2073), S(1086,2084), S(1100,2032), S(1106,1993), S(1111,1933),
                                                                                                           S(1054,2053), S(1058,2090), S(1059,2102), S(1059,2104), S(1057,2115), S(1071,2113), S(1083,2094), S(1080,2091),
                                                                                                           S(1053,2080), S(1042,2106), S(1044,2119), S(1059,2132), S(1068,2136), S(1060,2148), S(1071,2148), S(1089,2140),
                                                                                                           S(1039,2094), S(1053,2097), S(1066,2096), S(1061,2138), S(1061,2164), S(1083,2176), S(1080,2195), S(1097,2179),
                                                                                                           S(1078,2071), S(1069,2080), S(1081,2115), S(1087,2142), S(1103,2180), S(1200,2135), S(1191,2119), S(1160,2162),
                                                                                                           S(1066,2076), S(1042,2109), S(1049,2152), S(1040,2178), S(1046,2225), S(1134,2167), S(1103,2158), S(1179,2126),
                                                                                                           S( 988,2111), S( 984,2133), S(1057,2136), S(1103,2114), S(1145,2115), S(1142,2122), S(1265,1984), S(1115,2048),
                                                                                                   },
                                                                                                   {
                                                                                                           S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0), S( -86, -42), S(-150, -12), S( -18, -61), S( -33,-126),
                                                                                                           S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0), S(-145,  57), S(-114,  40), S( -30,   4), S( -49, -40),
                                                                                                           S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0), S(-222,  94), S(-217,  73), S(-143,  31), S(-194,   2),
                                                                                                           S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0), S(-336, 132), S(-244,  98), S(-245,  72), S(-307,  28),
                                                                                                           S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0), S(-307, 140), S(-243, 128), S(-211, 100), S(-321,  50),
                                                                                                           S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0), S( -95, 114), S(  47,  98), S(   3,  96), S( -29,  20),
                                                                                                           S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0), S(   5,  67), S( -40,  86), S(  30,  74), S(-299,  53),
                                                                                                           S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0), S(-447,  50), S(-394,  59), S(-333,  52), S( -52,-187),
                                                                                                   },
                                                                                           }},
                                                                                  }};

    constexpr square_t flipSquare(square_t square) {
        return square >> 3 | (square & 7) << 3;
    }

    constexpr auto transformPSTs(const std::array<std::array<std::array<packed_eval_t, 64>, 6>, NUM_KING_BUCKETS>& psts_) {
        std::array<std::array<std::array<packed_eval_t, 64>, 6>, NUM_KING_BUCKETS> result = {};
        for (auto bucket = 0; bucket < NUM_KING_BUCKETS; bucket++) {
            for (piece_t piece = 0; piece <= pcs::KING; piece++) {
                for (square_t square = 0; square < 64; square++) {
                    result[bucket][piece][square] = psts_[bucket][piece][flipSquare(square)];
                }
            }
        }

        return result;
    }

    constexpr auto real_psts = transformPSTs(psts);

    inline eval_t evalFromPacked(packed_eval_t packed, phase_t phase) {
        int32_t mg = int32_t(int16_t(uint16_t((packed + (1U << 15)) >> 16)));
        int32_t eg = int32_t(int16_t(uint16_t(packed)));
        if (phase >= MAX_PHASE)
            return mg;
        else
            return (mg * phase + eg * (MAX_PHASE - phase)) / MAX_PHASE;
    }

    eval_t getStaticEval(const ChessBoard& board) {
        // Step 1: initialize eval and phase
        packed_eval_t packedEval = 0;
        phase_t phase = 0;
        const bitboard_t allPieces = board.getSideBB(sides::WHITE) | board.getSideBB(sides::BLACK);

        // Step 2: Loop through all the piece types, adding up their PSTs and mobility evals
        for (piece_t piece = pcs::PAWN; piece <= pcs::KING; piece++) {
            for (side_t side = 0; side < 2; side++) {
                const square_t kingSquare = log2ll(board.getPieceBB(pcs::KING) & board.getSideBB(side));
                const auto kingBucket = hce::getFriendlyKingBucket(kingSquare, side);
                const bitboard_t notFriendlyPieces = ~board.getSideBB(side);
                const eval_t multiplier = side == sides::WHITE ? 1 : -1;
                bitboard_t remainingPieces = board.getPieceBB(piece) & board.getSideBB(side);
                phase += hce::PHASE_PIECE_VALUES[piece] * std::popcount(remainingPieces);
                bitboard_t squareBB;
                square_t square;
                while (remainingPieces) {
                    squareBB = remainingPieces & -remainingPieces;
                    remainingPieces -= squareBB;
                    square = log2ll(squareBB);

                    // PSTs
                    packedEval += hce::real_psts[kingBucket][piece][square ^ (7 * side)] * multiplier;
                    // Mobility
                    const bitboard_t attacks = getAttackedSquares(square, piece, allPieces, side);
                    packedEval += hce::mobility[piece] * std::popcount(attacks & notFriendlyPieces) * multiplier;

                } // end while remainingPieces
            } // end for loop over side
        } // end for loop over piece type

        // Step 3: Unpack the eval
        eval_t whiteRelativeEval = hce::evalFromPacked(packedEval, phase);

        // Step 4: Return the eval from the perspective of stm
        return (board.getSTM() == sides::WHITE) ? whiteRelativeEval : -whiteRelativeEval;
    }
}

