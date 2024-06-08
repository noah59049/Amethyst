#pragma once
#include "../Typedefs.h"
#include "../movegen/Flags.h"

inline eval_t getDelta(move_t move) {
    switch (getFlag(move)) {
        case CAPTURE_PAWN_FLAG: return 300;

        case CAPTURE_KNIGHT_FLAG:
        case CAPTURE_BISHOP_FLAG:
            return 500;

        case CAPTURE_ROOK_FLAG: return 700;
        case CAPTURE_QUEEN_FLAG: return 1100;
        case EN_PASSANT_FLAG: return 300;

        case NORMAL_MOVE_FLAG:
        case PAWN_PUSH_TWO_SQUARES_FLAG:
            return 200;

        case PROMOTE_TO_QUEEN_FLAG: return 2000;

        case PROMOTE_TO_ROOK_FLAG:
        case PROMOTE_TO_BISHOP_FLAG:
        case PROMOTE_TO_KNIGHT_FLAG:
            return 0;
        default: return 2000; // This should never happen, but if it does, return a very high delta
    }
}