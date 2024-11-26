#include "moveorder.h"

#include "flags.h"
#include "movelist.h"

#include <algorithm>

uint32_t getMVVLVAScore(move_t move) {
    if (mvs::isPromotion(move)) {
        if (mvs::getPromotedPiece(move) != pcs::QUEEN)
            return 0;

        else if (mvs::isCapture(move)) {
            return 10 * mvs::getCapturedPiece(move) + 10 * pcs::QUEEN - pcs::PAWN;
        }
        else {
            return 10 * pcs::QUEEN - pcs::PAWN;
        }
    }
    return 10 * mvs::getCapturedPiece(move) + 10 - mvs::getPiece(move);
}

// We assume all the moves in here are captures
void scoreMovesByMVVLVA(MoveList& moves) {
    for (move_t& move : moves)
        move |= getMVVLVAScore(move) << 22;
    // Note that we add mvvlvaScore << 22 because that doesn't affect any of the information we have in a move
    // And using the high bits allows us to sort moves by raw value
}