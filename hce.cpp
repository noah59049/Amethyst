#include "hce.h"
#include "logarithm.h"
#include "attacks.h"

namespace hce {
    /********************************\
    |*** Part 1: Helper functions ***|   
    \********************************/
    constexpr phase_t PHASE_PIECE_VALUES[6] = {0,1,1,2,4,0};
    constexpr phase_t MAX_PHASE = 24;

    constexpr packed_eval_t S(int16_t mg, int16_t eg) {
        return (uint32_t(uint16_t(mg)) << 16) + eg;
    }

    // PSTs are bucketed by the friendly king
    // There are only two buckets:
    // queenside and kingside
    constexpr auto NUM_KING_BUCKETS = 2;

    inline auto getFriendlyKingBucket(square_t kingSquare, side_t side) {
        return kingSquare / 32;
    }

    /****************************\
    |*** Part 2: Eval weights ***|   
    \****************************/

    constexpr packed_eval_t material[6] = {S(163, 339), S(470, 600), S(489, 584), S(584, 1107), S(996, 2141), S(-1, 1)};

    constexpr packed_eval_t mobility[6] = {S(-13, -25), S(1, 5), S(9, 12), S(8, 6), S(3, 10), S(-19, 1)};
    // hash=2c7dcade45139c90877f774d2aff1c1c5eaccadf6c97b08f426dda8fda187db1

    // You can visualize these PSTs by pretending they are squares on a board
    // And each entry is S(mg, eg) for a black piece on that square
    constexpr std::array<std::array<std::array<packed_eval_t, 64>, 6>, 2> psts = {{
          {{
                   {
                       S(   8,  10), S(   8,  10), S(   8,  10), S(   8,  10), S(   8,  10), S(   8,  10), S(   8,  10), S(   8,  10), 
                       S(   3, -92), S(  85, -77), S(  36, -90), S( -29, -59), S( -65, -53), S( -58, -62), S( -89, -36), S(-148, -55), 
                       S(  12,-104), S(  40, -79), S(  26,-100), S( -40, -67), S( -27, -86), S( -59, -85), S( -90, -68), S(-149, -83), 
                       S( -10, -69), S(  33, -52), S(  21, -85), S(   5, -76), S(  -9, -86), S( -38, -87), S( -54, -65), S(-135, -85), 
                       S(   8,  -7), S(  44,   5), S(  26, -34), S( -14, -44), S(  14, -68), S(  -4, -77), S( -38, -40), S(-113, -50), 
                       S(  25, 147), S(  43, 197), S(  19, 152), S(  36,  92), S(  41,  69), S(  43,  19), S(  -6,  79), S(-102,  59), 
                       S(  65, 254), S( 135, 180), S(  91, 275), S(  93, 189), S(  49, 168), S(  36, 127), S(   1, 162), S( -77, 178), 
                       S(   8,  10), S(   8,  10), S(   8,  10), S(   8,  10), S(   8,  10), S(   8,  10), S(   8,  10), S(   8,  10), 
                   },
                   {
                       S( -67, -40), S( -88, -18), S( -55,  -9), S( -40,  -5), S( -52, -21), S( -53, -29), S( -65, -33), S( -31, -45), 
                       S( -53, -22), S( -51,   0), S( -21,   7), S(  -4,  17), S( -13,   6), S(  -8,   3), S( -20, -21), S( -58, -26), 
                       S( -48, -11), S( -28,   2), S(  34, -10), S(  27,  40), S(  22,  37), S( -12,  15), S( -22, -13), S( -45, -13), 
                       S( -12,  -7), S( -11,  18), S(  36,  37), S(  37,  43), S(  69,  58), S(  24,  40), S(  38,  27), S( -17, -14), 
                       S( -11,  -3), S(  33,  24), S(  71,  45), S( 131,  29), S(  78,  47), S(  83,  53), S(  36,  23), S(  21,   7), 
                       S( -26,  -4), S(  33,   1), S(  57,  20), S(  93,  28), S( 112,  31), S(  58,  21), S(  62,   1), S(  -8,  -6), 
                       S( -41, -38), S(   6, -10), S(  28, -11), S(  25,  15), S(  34,  -9), S(  59,  -2), S(  -7,  16), S(  18, -22), 
                       S(-145,-124), S(  -7, -53), S( -14, -18), S( -11, -34), S(  -1,  -3), S( -18, -29), S(   6, -16), S( -70, -76), 
                   },
                   {
                       S( -5,  0), S(-20, 20), S(-22, 19), S(-10, 12), S( -6,  3), S( -7,  5), S(-16, 11), S(-21,-22), 
                       S(-14, 25), S( 61,-13), S( 46,-12), S( 12,  3), S( -3,  8), S(  8, -6), S(-16,  7), S(-18,-20), 
                       S(  7, 14), S( 11,-12), S( 42, -6), S( 23, -4), S( 18,  6), S( -9, 11), S(-14,  2), S(-28,-12), 
                       S( -9,  3), S( 15,  8), S( 13,  1), S( 58, -7), S( 28, -8), S( -9, 17), S( -9, -5), S(-12, 14), 
                       S( -6, 11), S(  6, 13), S( 16,  8), S( 45, -4), S( 48, -6), S( 32, -9), S(  3, 22), S( 13,  2), 
                       S(-30, 40), S( 16, 11), S(  2, 11), S( 49, -5), S( 15,-19), S( 46,  4), S( 41, -2), S( 31, 13), 
                       S(-65,-17), S(-37,  5), S( -8, -2), S(-11,-20), S(-13, -4), S( 17, -9), S( 29,  1), S(  2, -4), 
                       S(-37,-18), S(-26,  3), S(-23,  5), S(-38, 12), S(-28, 10), S(-54, -4), S(-19, -2), S(-37,  2), 
                   },
                   {
                       S(-92, -1), S(-41, -1), S(-19,  7), S(  7,-10), S( 14,-24), S(  1,-36), S( 11,-49), S(-26,-22), 
                       S(-26,-37), S(-32,-20), S(-14, -7), S(-23,  1), S(-29,-12), S(-23,-21), S(-18,-30), S(-53,-34), 
                       S(-22,-14), S(-34, -2), S( -9,-10), S(-28,-11), S(-43,  0), S(-34,-21), S(-28,-37), S(-56,-33), 
                       S(-29,-14), S(-16, -2), S(-28,  7), S(-19,  5), S(-32,  6), S(-50, -3), S(-32,-13), S(-32,-12), 
                       S(  1,  0), S( -2,  5), S( -9, 21), S( -7, 16), S(-12, -5), S( -1,-13), S(  5,-15), S(  1, -7), 
                       S(  6, -5), S( 15, 13), S( 22, 12), S( 21, -4), S( 15, -1), S(  8,  6), S( 51, -8), S( 45, -8), 
                       S( 28, -4), S( 26, 23), S( 41, 30), S( 56, 17), S( 44, 16), S( 41,  4), S( 41,  6), S( 78, -4), 
                       S( 10,  0), S(-21, 24), S( 15, 38), S( 17, 20), S( -3, 24), S(  0,  9), S( 22,  7), S( 30, 20), 
                   },
                   {
                       S(-53,-48), S(-76,-44), S(-36,-37), S(-28,-42), S( -8,-33), S(-13,-53), S(-25,-18), S(-25,-38), 
                       S(-51,-28), S(-31,-54), S( 13,-54), S( 19,-10), S(  8,-27), S(-11,-10), S( -2,-36), S( 31,-38), 
                       S(-41,-18), S(-24, 10), S( -5,  4), S( -8,  0), S(  3,-13), S(-20,  2), S( -6,-19), S( -5,  0), 
                       S(-20,-22), S(-14,  0), S( -9, -3), S( 29, 21), S(  8, -6), S(-14,  3), S( -9,-10), S(  7, 15), 
                       S(-22,  1), S(-27, 15), S(-11,  7), S(  2, 18), S( 23, 34), S( 19,  2), S( 14, 10), S( 44,  1), 
                       S(-26, -3), S(-12, -4), S(-18, 33), S( 25, 24), S( 40, 26), S( 59, 20), S( 83,-21), S( 91,  8), 
                       S(-24,-29), S(-39, -8), S(-20, 35), S(-18, 29), S(-10, 43), S( 24, -1), S( 29,-21), S( 51,  2), 
                       S(-65,-97), S(-32,-60), S(  0,-27), S( -6, -7), S( -2,-14), S(-15,-27), S(-24,-44), S(-12,-34), 
                   },
                   {
                       S(  53,-135), S( 121,-119), S(  72, -81), S(  -4, -78), S(   0,  -1), S(   0,  -1), S(   0,  -1), S(   0,  -1), 
                       S(  83, -79), S(  98, -58), S(  68, -37), S(  47, -29), S(   0,  -1), S(   0,  -1), S(   0,  -1), S(   0,  -1), 
                       S(  -9, -74), S(  34, -39), S( -21,  -8), S( -44,  18), S(   0,  -1), S(   0,  -1), S(   0,  -1), S(   0,  -1), 
                       S( -35, -68), S( -18, -20), S( -31,  18), S( -69,  49), S(   0,  -1), S(   0,  -1), S(   0,  -1), S(   0,  -1), 
                       S( -13, -51), S(  13,   2), S(   3,  39), S( -52,  62), S(   0,  -1), S(   0,  -1), S(   0,  -1), S(   0,  -1), 
                       S( -25, -35), S(  64,  19), S(  29,  39), S(  31,  57), S(   0,  -1), S(   0,  -1), S(   0,  -1), S(   0,  -1), 
                       S( -17, -52), S(  31,   5), S(  21,  23), S(  45,  19), S(   0,  -1), S(   0,  -1), S(   0,  -1), S(   0,  -1), 
                       S( -20,-116), S(  -4, -72), S(  10, -43), S(  -4, -16), S(   0,  -1), S(   0,  -1), S(   0,  -1), S(   0,  -1), 
                   },
          }},
          {{
                   {
                       S(   8,  10), S(   8,  10), S(   8,  10), S(   8,  10), S(   8,  10), S(   8,  10), S(   8,  10), S(   8,  10), 
                       S( -92, -71), S( -47, -51), S( -58, -73), S( -63, -59), S( -28, -44), S(   4, -74), S(  61, -87), S( -38,-131), 
                       S(-100, -87), S( -58, -73), S( -50, -92), S( -34, -74), S( -12, -86), S( -18, -91), S(  29, -97), S( -15,-134), 
                       S( -98, -74), S( -39, -62), S( -37, -90), S(  -4, -98), S(   4,-100), S(  -3, -99), S(  10, -78), S( -37,-115), 
                       S( -79, -30), S( -23, -34), S( -21, -63), S(  -6, -77), S(  33, -91), S(  15, -76), S(  29, -48), S( -14, -70), 
                       S( -50,  86), S( -10, 122), S(  48,  72), S(  53,  51), S(  65,  31), S(  99,  23), S(  70, 110), S( -18,  82), 
                       S(  80, 177), S( 109, 181), S( 111, 183), S( 125, 134), S( 112, 130), S(  89, 160), S(  -1, 196), S( -38, 201), 
                       S(   8,  10), S(   8,  10), S(   8,  10), S(   8,  10), S(   8,  10), S(   8,  10), S(   8,  10), S(   8,  10), 
                   },
                   {
                       S(-112, -47), S( -53, -55), S( -66, -26), S( -37,  -7), S( -35,  -3), S( -35, -17), S( -46, -34), S(-100, -30), 
                       S( -79, -29), S( -59,  -7), S( -32,  10), S(  -6,  13), S(  -1,  10), S(  -5,  -1), S( -36, -11), S( -43,  -4), 
                       S( -74, -28), S( -36,   1), S(  -8,  10), S(  14,  47), S(  34,  35), S(  13,   0), S(   3, -12), S( -35, -19), 
                       S( -42,   7), S( -12,  21), S(  19,  54), S(  26,  48), S(  47,  59), S(  30,  31), S(  27,  19), S( -15,  -6), 
                       S( -21,  13), S(   7,  29), S(  45,  48), S(  86,  58), S(  57,  54), S( 107,  45), S(  33,  42), S(  50,  -4), 
                       S( -17,  -4), S(  38,  15), S(  85,  35), S(  93,  32), S( 148,   9), S( 154,   9), S(  87,   0), S(  44, -23), 
                       S( -47, -23), S(  -7,   3), S(  21,  16), S(  46,  21), S(  25,   7), S( 122, -13), S(   7, -16), S(  22, -52), 
                       S(-216,-105), S( -65, -30), S( -52, -15), S( -34, -12), S( -11,  -3), S( -56, -43), S( -32, -38), S(-120,-125), 
                   },
                   {
                       S( -16, -20), S(  22,   3), S(   5, -17), S( -18,   2), S(  -9,   4), S( -28,  22), S(   2, -21), S(  -9, -40), 
                       S(   7,  -5), S(  15, -14), S(  20, -20), S( -13,  10), S(   9,   5), S(  18,  -5), S(  43,  -4), S(   1, -22), 
                       S( -13,   6), S(  -6,   4), S(   5,   9), S(   0,   6), S(   5,  11), S(  10,   0), S(  -2, -10), S(  23,  -9), 
                       S( -22,  -1), S( -22,  10), S( -10,  15), S(  32,   0), S(  30,  -9), S(  -6,  -2), S( -14,   3), S(   1, -17), 
                       S( -22,  12), S(  -1,  11), S(  20,  -2), S(  48,   5), S(  40,  -8), S(  26,   4), S(   1,   3), S( -12,  14), 
                       S(  -6,  16), S(  22,   0), S(  21,  -5), S(  48, -22), S(  25,  -2), S(  73,  -4), S(  51,   4), S(  38,  19), 
                       S( -33, -15), S(  14, -11), S( -12,   0), S( -28,   2), S(   2,  -9), S(   2, -14), S(  -2,  -1), S( -10, -20), 
                       S( -36,   2), S( -49,  11), S( -53,  -2), S(-112,  16), S( -66,   0), S( -58,  -1), S( -20, -13), S( -54, -29), 
                   },
                   {
                       S(-16, -5), S(-18, -5), S(  6,  1), S( 21, -5), S( 28,-15), S(  2, -8), S(  9,-18), S( -2,-46), 
                       S(-58,  0), S(-46,  3), S(-12, -1), S( -7,  0), S(  4,-16), S(  2,-23), S( 40,-47), S(-12,-27), 
                       S(-64,  6), S(-64,  0), S(-45,  3), S(-31,  8), S(-19,-11), S(-16,-25), S( 28,-55), S(  6,-48), 
                       S(-64, 14), S(-68, 18), S(-49, 19), S(-34,  9), S(-20,  1), S(-32, -3), S( -2,-12), S(-16,-21), 
                       S(-46, 28), S(-36, 21), S(-31, 32), S(-12, 16), S( -2,-10), S( 13, -9), S( 13, -7), S( 29,-14), 
                       S(-31, 25), S(  4, 24), S(  4, 26), S( 13, 17), S( 59,  0), S( 70,-15), S(106, -6), S( 81,-24), 
                       S( -2, 24), S( -1, 41), S( 32, 49), S( 59, 34), S( 43, 31), S(109,  5), S( 78, 10), S(109,-20), 
                       S( 13, 22), S( 15, 24), S(  5, 43), S(  4, 37), S( 43, 17), S( 31, 31), S( 16, 35), S( 42, 13), 
                   },
                   {
                       S(-25,-32), S(-15,-34), S(  6,-37), S( 21,-15), S( 14,-44), S(-37,-26), S(-18,-45), S(-25,-29), 
                       S(-10,-32), S( -7,-20), S( 11,-28), S( 12,-20), S( 12,-10), S( 23,-52), S( 20,-69), S(  2,-56), 
                       S(-21,-33), S(-17, -4), S(-15,  9), S(-15, 13), S(-17, 22), S( -1, 18), S(  9,  3), S(  6, -6), 
                       S(-22,-11), S(-29,  9), S(-29, 24), S(-14, 39), S( -6, 40), S(-12, 48), S( -1, 47), S( 17, 34), 
                       S(-33, -7), S(-21,  3), S( -8,  4), S(-13, 44), S( -8, 62), S( 13, 70), S( 14, 80), S( 26, 67), 
                       S(  4,-21), S( -9, -9), S(  6, 20), S( 15, 42), S( 32, 72), S(109, 52), S(103, 35), S( 85, 58), 
                       S( -9,-20), S(-30, 11), S(-16, 43), S(-13, 53), S(  1, 88), S( 60, 65), S( 32, 43), S( 93, 33), 
                       S(-59, -9), S(-49,  4), S( -9, 24), S( 11, 26), S( 39, 32), S( 43, 32), S( 63,-11), S( 19,-22), 
                   },
                   {
                       S(   0,  -1), S(   0,  -1), S(   0,  -1), S(   0,  -1), S(  36, -51), S( -29, -21), S( 103, -67), S(  84,-125), 
                       S(   0,  -1), S(   0,  -1), S(   0,  -1), S(   0,  -1), S( -18,  36), S(  12,  21), S(  95, -13), S(  70, -46), 
                       S(   0,  -1), S(   0,  -1), S(   0,  -1), S(   0,  -1), S( -83,  71), S( -81,  52), S( -14,  12), S( -70,  -7), 
                       S(   0,  -1), S(   0,  -1), S(   0,  -1), S(   0,  -1), S(-143, 100), S( -89,  73), S( -83,  45), S(-137,   9), 
                       S(   0,  -1), S(   0,  -1), S(   0,  -1), S(   0,  -1), S( -76, 101), S( -48,  94), S( -35,  71), S( -93,  21), 
                       S(   0,  -1), S(   0,  -1), S(   0,  -1), S(   0,  -1), S(   2,  95), S(  42,  96), S(  28,  89), S(  -7,  27), 
                       S(   0,  -1), S(   0,  -1), S(   0,  -1), S(   0,  -1), S(  25,  60), S(  20,  76), S(  32,  70), S(   1,  12), 
                       S(   0,  -1), S(   0,  -1), S(   0,  -1), S(   0,  -1), S(   0, -17), S(   7,  -5), S(  11,   1), S(  -1, -87), 
                   },
          }},
        }}; // hash=2c7dcade45139c90877f774d2aff1c1c5eaccadf6c97b08f426dda8fda187db1
    /********************************\
    |*** Part 3: Flipping squares ***|   
    \********************************/

    // The PSTs above are designed for readability, so you can see the 8x8 thing and look at the weights as if they are squares on a chessboard
    // However Amethyst uses a1=0, a8=7, h1=56, h8=63
    // This means that we need to mirror the PSTs from part 2 across the a1-h8 diagonal before we index them
    // 
    constexpr square_t flipSquare(square_t square) {
        // Returns the square reflected across the a1-h8 diagonal
        return square >> 3 | (square & 7) << 3;
    }

    constexpr auto transformPSTs(const std::array<std::array<std::array<packed_eval_t, 64>, 6>, NUM_KING_BUCKETS>& psts_) {
        std::array<std::array<std::array<packed_eval_t, 64>, 6>, NUM_KING_BUCKETS> result = {};
        for (auto bucket = 0; bucket < NUM_KING_BUCKETS; bucket++) {
            for (piece_t piece = 0; piece <= pcs::KING; piece++) {
                for (square_t square = 0; square < 64; square++) {
                    result[bucket][piece][square] = psts_[bucket][piece][flipSquare(square)] + material[piece];
                }
            }
        }

        return result;
    }

    constexpr auto real_psts = transformPSTs(psts);

    /*****************************\
    |*** Part 4: Eval function ***|   
    \*****************************/
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

