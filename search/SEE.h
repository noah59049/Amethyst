#pragma once
#include "../Typedefs.h"
namespace see {
    constexpr const static int ONLY_KING_ATTACKING = 1048576;
    constexpr const static int TEXTBOOK_PIECE_VALUES[5] = {900,500,300,300,100};
    see_mask getSEE(int pieceTypeOnSquare, int myAttackersMask, int oppAttackersMask);
}