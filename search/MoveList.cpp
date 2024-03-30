#include "MoveList.h"
void MoveList::push_back(move_t move) {
    assert(size < 218);
    moveList[size] = move;
    size++;
}

void MoveList::sortMoves(const ChessBoard &board, move_t hashMove, const TwoKillerMoves &killerMoves,
                         const QuietHistory &quietHistory) {
    // Step 1: Put the hash move first
    for (unsigned int i = 0; i < size; i++) {
        if (moveList[i] == hashMove) {
            move_t temp = moveList[0];
            moveList[0] = moveList[i];
            moveList[i] = temp;
            break;
        }
    }
    // Step 2: Put SEE>=0 captures in the front and SEE<0 captures in the back
    unsigned int sortedIndex = moveList[0] == hashMove; // which should be true EVERY SINGLE TIME unless we have a zobrist code collision
    unsigned int backIndex = size - 1;
    for (unsigned int i = sortedIndex; i <= backIndex; i++) {
        if (isCapture(moveList[i])) {
            if (board.getCaptureSEE(moveList[i]) >= 0) {
                // the capture has positive or zero SEE
                move_t temp = moveList[sortedIndex];
                moveList[sortedIndex] = moveList[i];
                moveList[i] = temp;
                sortedIndex++;
            }
            else {
                // the capture has negative SEE
                move_t temp = moveList[backIndex];
                moveList[backIndex] = moveList[i];
                moveList[i] = temp;
                backIndex--;
            }
        } // end if it's a capture
    } // end for loop over i

    // Step 3: Put killers in front
    for (unsigned int i = sortedIndex; i <= backIndex; i++) {
        if (moveList[i] == killerMoves.getFirstKillerMove()) {
            move_t temp = moveList[sortedIndex];
            moveList[sortedIndex] = moveList[i];
            moveList[i] = temp;
            sortedIndex++;
        }
        else if (moveList[i] == killerMoves.getSecondKillerMove()) {
            move_t temp = moveList[sortedIndex];
            moveList[sortedIndex] = moveList[i];
            moveList[i] = temp;
            sortedIndex++;
        }
    }

    // Step 4: Make sure the killers are in the right order
    if (moveList[sortedIndex-1] == killerMoves.getFirstKillerMove() and moveList[sortedIndex-2] == killerMoves.getSecondKillerMove()) {
        moveList[sortedIndex-2] = killerMoves.getFirstKillerMove();
        moveList[sortedIndex-1] = killerMoves.getSecondKillerMove();
    }

    // Step 5: Sort quiets by history heuristic
    quietHistory.sortMovesByCutoffs(moveList,sortedIndex,backIndex);
}