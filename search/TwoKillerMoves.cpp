#include "TwoKillerMoves.h"
void TwoKillerMoves::recordKillerMove(const uint16_t move) {
    if (firstKillerMove == NULL_KILLER_MOVE) {
        firstKillerMove = move;
    }
    else if (secondKillerMove == NULL_KILLER_MOVE) {
        secondKillerMove = move;
    }
    else if (move == firstKillerMove)
        firstKillerMoveCount++;
    else if (move == secondKillerMove) {
        secondKillerMoveCount++;
        if (secondKillerMoveCount > firstKillerMoveCount) {
            // swap first and second killer moves
            int8_t swap = secondKillerMoveCount;
            secondKillerMoveCount = firstKillerMoveCount;
            firstKillerMoveCount = swap;
            int swap2 = firstKillerMove;
            firstKillerMove = secondKillerMove;
            secondKillerMove = swap2;
        }
    }
    else {
        firstKillerMoveCount--;
        secondKillerMoveCount--;
        if (firstKillerMoveCount == 0) {
            firstKillerMove = move;
            firstKillerMoveCount = 1;
        } else if (secondKillerMoveCount == 0) {
            secondKillerMove = move;
            secondKillerMoveCount = 1;
        }
    } // end else
}

TwoKillerMoves::TwoKillerMoves() {
    firstKillerMove = NULL_KILLER_MOVE;
    firstKillerMoveCount = 0;
    secondKillerMove = NULL_KILLER_MOVE;
    secondKillerMoveCount = 0;
}

void TwoKillerMoves::clear() {
    firstKillerMove = NULL_KILLER_MOVE;
    firstKillerMoveCount = 0;
    secondKillerMove = NULL_KILLER_MOVE;
    secondKillerMoveCount = 0;
}