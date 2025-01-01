#pragma once
#include <string>

#include "typedefs.h"

namespace pcs {
    constexpr piece_t PAWN = 0;
    constexpr piece_t KNIGHT = 1;
    constexpr piece_t BISHOP = 2;
    constexpr piece_t ROOK = 3;
    constexpr piece_t QUEEN = 4;
    constexpr piece_t KING = 5;
}

namespace sides {
    constexpr side_t WHITE = 0;
    constexpr side_t BLACK = 1;
    // Ciekce and alion02 both say this is better than doing it the other way
}

namespace squares {
    inline square_t getRank(square_t square) {
        return square & 7;
    }

    inline square_t getFile(square_t square) {
        return square >> 3;
    }

    inline square_t squareFromFileRank(square_t file, square_t rank) {
        return file * 8 + rank;
    }

    constexpr square_t a1 = 0;
    constexpr square_t a2 = 1;
    constexpr square_t a3 = 2;
    constexpr square_t a4 = 3;
    constexpr square_t a5 = 4;
    constexpr square_t a6 = 5;
    constexpr square_t a7 = 6;
    constexpr square_t a8 = 7;
    constexpr square_t b1 = 8;
    constexpr square_t b2 = 9;
    constexpr square_t b3 = 10;
    constexpr square_t b4 = 11;
    constexpr square_t b5 = 12;
    constexpr square_t b6 = 13;
    constexpr square_t b7 = 14;
    constexpr square_t b8 = 15;
    constexpr square_t c1 = 16;
    constexpr square_t c2 = 17;
    constexpr square_t c3 = 18;
    constexpr square_t c4 = 19;
    constexpr square_t c5 = 20;
    constexpr square_t c6 = 21;
    constexpr square_t c7 = 22;
    constexpr square_t c8 = 23;
    constexpr square_t d1 = 24;
    constexpr square_t d2 = 25;
    constexpr square_t d3 = 26;
    constexpr square_t d4 = 27;
    constexpr square_t d5 = 28;
    constexpr square_t d6 = 29;
    constexpr square_t d7 = 30;
    constexpr square_t d8 = 31;
    constexpr square_t e1 = 32;
    constexpr square_t e2 = 33;
    constexpr square_t e3 = 34;
    constexpr square_t e4 = 35;
    constexpr square_t e5 = 36;
    constexpr square_t e6 = 37;
    constexpr square_t e7 = 38;
    constexpr square_t e8 = 39;
    constexpr square_t f1 = 40;
    constexpr square_t f2 = 41;
    constexpr square_t f3 = 42;
    constexpr square_t f4 = 43;
    constexpr square_t f5 = 44;
    constexpr square_t f6 = 45;
    constexpr square_t f7 = 46;
    constexpr square_t f8 = 47;
    constexpr square_t g1 = 48;
    constexpr square_t g2 = 49;
    constexpr square_t g3 = 50;
    constexpr square_t g4 = 51;
    constexpr square_t g5 = 52;
    constexpr square_t g6 = 53;
    constexpr square_t g7 = 54;
    constexpr square_t g8 = 55;
    constexpr square_t h1 = 56;
    constexpr square_t h2 = 57;
    constexpr square_t h3 = 58;
    constexpr square_t h4 = 59;
    constexpr square_t h5 = 60;
    constexpr square_t h6 = 61;
    constexpr square_t h7 = 62;
    constexpr square_t h8 = 63;
}

namespace flags {
    constexpr move_t QUIET_FLAG = 0;
    constexpr move_t DOUBLE_PAWN_PUSH_FLAG = 1;
    constexpr move_t SHORT_CASTLE_FLAG = 2;
    constexpr move_t LONG_CASTLE_FLAG = 3;

    constexpr move_t CAPTURE_FLAG = 4;
    constexpr move_t EN_PASSANT_FLAG = 5;

    constexpr move_t KNIGHT_PROMO_FLAG = 8;
    constexpr move_t BISHOP_PROMO_FLAG = 9;
    constexpr move_t ROOK_PROMO_FLAG = 10;
    constexpr move_t QUEEN_PROMO_FLAG = 11;

    constexpr move_t KNIGHT_CAP_PROMO_FLAG = 12;
    constexpr move_t BISHOP_CAP_PROMO_FLAG = 13;
    constexpr move_t ROOK_CAP_PROMO_FLAG = 14;
    constexpr move_t QUEEN_CAP_PROMO_FLAG = 15;
}

namespace mvs {
    inline move_t getFrom(move_t move) {
        return move & 63;
    }

    inline move_t getTo(move_t move) {
        return move >> 6 & 63;
    }

    inline move_t getFromTo(move_t move) {
        return move & 4095;
    }

    inline move_t getFlag(move_t move) {
        return move >> 12 & 15;
    }

    inline piece_t getPiece(move_t move) {
        return move >> 16 & 7;
    }

    inline piece_t getCapturedPiece(move_t move) {
        return move >> 19 & 7;
    }

    inline piece_t getPromotedPiece(move_t move) {
        piece_t result = (move >> 12 & 3) + 1;
        move_t flag = getFlag(move);

        return result;
    }



    inline bool isQuiet(move_t move) {
        return (move & 0xc000) == 0;
    }

    inline bool isTactical(move_t move) {
        return move & 0xc000;
    }

    inline bool isCapture(move_t move) {
        return move & 0x4000;
    }

    inline bool isPromotion(move_t move) {
        return move & 0x8000;
    }

    inline bool isEP(move_t move) {
        return (move & 0xf000) == 0x5000;
    }

    inline bool isDoublePawnPush(move_t move) {
        return (move & 0xf000) == 0x1000;
    }

    inline bool isShortCastle(move_t move) {
        return getFlag(move) == flags::SHORT_CASTLE_FLAG;
        // Could be made faster, but probably not worth it
    }

    inline bool isLongCastle(move_t move) {
        return getFlag(move) == flags::LONG_CASTLE_FLAG;
    }

    inline bool isCastle(move_t move) {
        return isShortCastle(move) or isLongCastle(move);
    }

    inline bool isIrreversible(move_t move) {
        return isCapture(move) or getPiece(move) == pcs::PAWN;
    }



    inline move_t constructMove(move_t from, move_t to, move_t flag, piece_t piece, piece_t capturedPiece) {
        return from | to << 6 | flag << 12 | move_t(piece) << 16 | move_t(capturedPiece) << 19;
    }

    inline move_t constructShortCastle(side_t side) {
        return constructMove(squares::e1 + 7 * side, squares::g1 + 7 * side, flags::SHORT_CASTLE_FLAG, pcs::KING, 0);
    }

    inline move_t constructLongCastle(side_t side) {
        return constructMove(squares::e1 + 7 * side, squares::c1 + 7 * side, flags::LONG_CASTLE_FLAG, pcs::KING, 0);
    }

    // Turns e1g1 into e1f1
    // Turns e1c1 into e1d1
    // Turns e8g8 into e8f8
    // Turns e8c8 into e8d8
    // This is necessary for pseudolegal movegen
    // Castling is legal if the king can't be taken AND the square the rook moves to is not under attack
    // "the square the rook moves to is not under attack" is the same as "if the king moves 1 square over, it isn't under attack
    inline move_t castleToKingSlide(move_t move) {
        square_t from = getFrom(move);
        square_t to = getTo(move);
        return constructMove(from, (from + to) / 2, flags::QUIET_FLAG, pcs::KING, 0);
    }
}

namespace rights {
    constexpr uint8_t EP_POSSIBLE = 8;
    constexpr uint8_t WHITE_SC = 16;
    constexpr uint8_t WHITE_LC = 32;
    constexpr uint8_t BLACK_SC = 64;
    constexpr uint8_t BLACK_LC = 128;

    inline void removeEPRights(uint8_t& rightsMask) {
        rightsMask &= 0xf0;
    }
    inline void addEPRights(uint8_t& rightsMask, square_t file) {
        rightsMask |= EP_POSSIBLE | file;
    }
    inline bool isEPPossible(uint8_t mask) {
        return mask & EP_POSSIBLE;
    }
    inline uint8_t extractEPRights(uint8_t mask) {
        return mask & 7;
    }

    inline bool canWhiteCastleShort(uint8_t mask) {
        return mask & WHITE_SC;
    }
    inline bool canWhiteCastleLong(uint8_t mask) {
        return mask & WHITE_LC;
    }
    inline bool canBlackCastleShort(uint8_t mask) {
        return mask & BLACK_SC;
    }
    inline bool canBlackCastleLong(uint8_t mask) {
        return mask & BLACK_LC;
    }

    // ALERT: These assume white = 0, black = 1
    inline bool canSideCastleShort(side_t side, uint8_t mask) {
        return mask & WHITE_SC << side << side;
    }

    inline bool canSideCastleLong(side_t side, uint8_t mask) {
        return mask & WHITE_LC << side << side;
    }



    constexpr uint8_t CASTLING_RIGHTS_PRESERVED[64] = {
            0b11011111, // a1
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b01111111, // a8
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11001111, // e1
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b00111111, // e8
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11101111, // h1
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b10111111, // h8
    };
}

enum MovegenStage {
    TACTICALS,
    QUIETS
};

namespace outcomes {
    // I don't think I will ever use these, but they are here
    constexpr outcome_t ONGOING = 0;
    constexpr outcome_t STALEMATE = 1;
    constexpr outcome_t INSUFFICIENT_MATERIAL = 2;
    constexpr outcome_t THREE_MOVE_REP = 3;
    constexpr outcome_t FIFTY_MOVES = 4;
    constexpr outcome_t WHITE_MATES = 6;
    constexpr outcome_t BLACK_MATES = 7;
}