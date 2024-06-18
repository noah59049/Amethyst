#include "MoveOrder.h"
void sortMoves(MoveList& legalMoves, const ChessBoard &board, move_t hashMove, const TwoKillerMoves &killerMoves,
               const QuietHistory &quietHistory, unsigned int movesToKeep) {
    // Step 0: Do nothing if there are 1 or fewer legal moves
    if (legalMoves.size < 2)
        return;
    // Step 1: Put the hash move first
    for (unsigned int i = 0; i < legalMoves.size; i++) {
        if (legalMoves.at(i) == hashMove) {
            move_t temp = legalMoves.at(0);
            legalMoves.moveList[0] = legalMoves.at(i);
            legalMoves.moveList[i] = temp;
            break;
        }
    }
    // Step 2: Put SEE>=0 captures in the front and SEE<0 captures in the back
    unsigned int sortedIndex = legalMoves.at(0) == hashMove; // which should be true EVERY SINGLE TIME unless we have a zobrist code collision
    unsigned int backIndex = legalMoves.size - 1;
    for (unsigned int i = sortedIndex; i <= backIndex; i++) {
        if (isCapture(legalMoves.at(i))) {
            if (board.getCaptureSEE(legalMoves.at(i)) >= 0) {
                // the capture has positive or zero SEE
                move_t temp = legalMoves.moveList[sortedIndex];
                legalMoves.moveList[sortedIndex] = legalMoves.at(i);
                legalMoves.moveList[i] = temp;
                sortedIndex++;
            }
            else {
                // the capture has negative SEE
                move_t temp = legalMoves.at(backIndex);
                legalMoves.moveList[backIndex] = legalMoves.at(i);
                legalMoves.moveList[i] = temp;
                backIndex--;
            }
        } // end if it's a capture
    } // end for loop over i

    // Step 3: Put killers in front
    for (unsigned int i = sortedIndex; i <= backIndex; i++) {
        if (legalMoves.at(i) == killerMoves.getFirstKillerMove()) {
            move_t temp = legalMoves.at(sortedIndex);
            legalMoves.moveList[sortedIndex] = legalMoves.at(i);
            legalMoves.moveList[i] = temp;
            sortedIndex++;
        }
        else if (legalMoves.at(i) == killerMoves.getSecondKillerMove()) {
            move_t temp = legalMoves.at(sortedIndex);
            legalMoves.moveList[sortedIndex] = legalMoves.at(i);
            legalMoves.moveList[i] = temp;
            sortedIndex++;
        }
    }

    // Step 4: Make sure the killers are in the right order
    if (sortedIndex >= 2 and legalMoves.at(sortedIndex-1) == killerMoves.getFirstKillerMove() and legalMoves.at(sortedIndex-2) == killerMoves.getSecondKillerMove()) {
        legalMoves.moveList[sortedIndex-2] = killerMoves.getFirstKillerMove();
        legalMoves.moveList[sortedIndex-1] = killerMoves.getSecondKillerMove();
    }

    // Step 5: Sort quiets by history heuristic
    quietHistory.sortMovesByCutoffs(legalMoves.moveList,sortedIndex,backIndex, movesToKeep);
}

inline int8_t getCaptureMVVLVAScore(const ChessBoard& board, move_t capture) {
    int8_t mvvlvaScore;
    switch(getFlag(capture)) {
        case CAPTURE_QUEEN_FLAG: mvvlvaScore = 90; break;
        case CAPTURE_ROOK_FLAG: mvvlvaScore = 50; break;
        case CAPTURE_BISHOP_FLAG: mvvlvaScore = 35; break;
        case CAPTURE_KNIGHT_FLAG: mvvlvaScore = 32; break;
        case CAPTURE_PAWN_FLAG: mvvlvaScore = 10; break;
        case PROMOTE_TO_QUEEN_FLAG: return 120;
        case EN_PASSANT_FLAG: return 0;
        default: mvvlvaScore = 0; break;
    }
    switch(board.getMovingPiece(capture)) {
        case KING: mvvlvaScore -= 0; break;
        case QUEEN: mvvlvaScore -= 90; break;
        case ROOK: mvvlvaScore -= 50; break;
        case BISHOP: mvvlvaScore -= 35; break;
        case KNIGHT: mvvlvaScore -= 32; break;
        case PAWN: mvvlvaScore -= 10; break;
    }
    return mvvlvaScore;
}

void sortCapturesByMVVLVA(const ChessBoard& board, MoveList& captures) {
    std::array<int8_t,4096> moveScores;
    for (move_t move : captures) {
        moveScores[move >> 4] = getCaptureMVVLVAScore(board,move);
    }

    for (int newIndex = 1; newIndex < captures.size; newIndex++) {
        move_t thisMove = captures.at(newIndex);
        int8_t thisMoveScore = moveScores[thisMove >> 4];
        int insertIndex;
        for (insertIndex = newIndex - 1; insertIndex >= 0; insertIndex--) {
            move_t newMove = captures.at(insertIndex);
            int8_t newMoveScore = moveScores[newMove >> 4];
            if (thisMoveScore > newMoveScore)
                captures.moveList[insertIndex + 1] = captures.at(insertIndex);
            else
                break;
        }
        captures.moveList[insertIndex + 1] = thisMove;
    }
}