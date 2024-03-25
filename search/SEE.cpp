#include "SEE.h"
#include "../movegen/Logarithm.h"
// an attackers mask goes like this
// each 4 bits records the number of those player's pieces attacking a square

// KKKKQQQQRRRRBBBBNNNNPPPP

inline int max(int a, int b) {
    if (a > b)
        return a;
    else
        return b;
}

see_mask see::getSEE(int pieceTypeOnSquare, see_mask myAttackersMask, see_mask oppAttackersMask) {
    if (myAttackersMask == 0 or myAttackersMask == ONLY_KING_ATTACKING and oppAttackersMask != 0) // we cannot take the piece
        return 0;
    else if (oppAttackersMask == 0)
        return TEXTBOOK_PIECE_VALUES[pieceTypeOnSquare];
    int smallestPieceTypeAttacking = log2ll(myAttackersMask & -myAttackersMask) >> 2;
    return max(0,TEXTBOOK_PIECE_VALUES[pieceTypeOnSquare] - getSEE(4 - smallestPieceTypeAttacking,oppAttackersMask,myAttackersMask - (1 << (smallestPieceTypeAttacking << 2))));
}
// We should maybe memoize this later.