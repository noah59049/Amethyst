#include "MoveOrder.h"
#include <iostream>
void sortMoves(MoveList& legalMoves, const ChessBoard &board, move_t hashMove, const TwoKillerMoves &killerMoves,
                         const QuietHistory &quietHistory) {
    // Step 1: Put the hash move first
    for (unsigned int i = 0; i < legalMoves.size; i++) {
        if (legalMoves.moveList[i] == hashMove) {
            move_t temp = legalMoves.moveList[0];
            legalMoves.moveList[0] = legalMoves.moveList[i];
            legalMoves.moveList[i] = temp;
            break;
        }
    }
    // Step 2: Put SEE>=0 captures in the front and SEE<0 captures in the back
    unsigned int sortedIndex = legalMoves.moveList[0] == hashMove; // which should be true EVERY SINGLE TIME unless we have a zobrist code collision
    unsigned int backIndex = legalMoves.size - 1;
    for (unsigned int i = sortedIndex; i <= backIndex; i++) {
        if (isCapture(legalMoves.moveList[i])) {
            if (board.getCaptureSEE(legalMoves.moveList[i]) >= 0) {
                // the capture has positive or zero SEE
                move_t temp = legalMoves.moveList[sortedIndex];
                legalMoves.moveList[sortedIndex] = legalMoves.moveList[i];
                legalMoves.moveList[i] = temp;
                sortedIndex++;
            }
            else {
                // the capture has negative SEE
                std::cout << "capture " << board.moveToSAN(legalMoves.moveList[i]) << "has negative SEE" << std::endl;
                move_t temp = legalMoves.moveList[backIndex];
                legalMoves.moveList[backIndex] = legalMoves.moveList[i];
                legalMoves.moveList[i] = temp;
                backIndex--;
            }
        } // end if it's a capture
    } // end for loop over i

    // Step 3: Put killers in front
    for (unsigned int i = sortedIndex; i <= backIndex; i++) {
        if (legalMoves.moveList[i] == killerMoves.getFirstKillerMove()) {
            move_t temp = legalMoves.moveList[sortedIndex];
            legalMoves.moveList[sortedIndex] = legalMoves.moveList[i];
            legalMoves.moveList[i] = temp;
            sortedIndex++;
        }
        else if (legalMoves.moveList[i] == killerMoves.getSecondKillerMove()) {
            move_t temp = legalMoves.moveList[sortedIndex];
            legalMoves.moveList[sortedIndex] = legalMoves.moveList[i];
            legalMoves.moveList[i] = temp;
            sortedIndex++;
        }
    }

    // Step 4: Make sure the killers are in the right order
    if (legalMoves.moveList[sortedIndex-1] == killerMoves.getFirstKillerMove() and legalMoves.moveList[sortedIndex-2] == killerMoves.getSecondKillerMove()) {
        legalMoves.moveList[sortedIndex-2] = killerMoves.getFirstKillerMove();
        legalMoves.moveList[sortedIndex-1] = killerMoves.getSecondKillerMove();
    }

    // Step 5: Sort quiets by history heuristic
    quietHistory.sortMovesByCutoffs(legalMoves.moveList,sortedIndex,backIndex);
}