#include "MoveOrder.h"

// WARNING! If search is multithreaded, this approach will completely break!
static int16_t historyTempValues[4096]{};

void updateHistoryTempValues (std::array<move_t,218> &vec, int startIndex, int endIndex,
                         const ChessBoard& board, const QuietHistory& quietHistory, const Conthist& conthist, const std::vector<uint16_t>& conthistStack) {
    for (int i = startIndex; i <= endIndex; i++) {
        historyTempValues[vec[i] >> 4] = 2 * quietHistory.lookupMoveCutoffCount(vec[i]);
        if (conthistStack.size() >= 1)
            historyTempValues[vec[i] >> 4] += conthist.getCutoffCount(conthistStack.at(conthistStack.size() - 1), board, vec[i]);
    }
}

void sortMovesByCutoffs (std::array<move_t,218> &vec, int startIndex, int endIndex,
                         const ChessBoard& board, const QuietHistory& quietHistory, const Conthist& conthist, const std::vector<uint16_t>& conthistStack) {
    if (startIndex >= endIndex) {
        return;
    }

    // Choose a partition element
    move_t partition = vec[startIndex];

    // Loop through vec from startIndex to endIndex
    // Keep track of where the > partition elements start
    int i;
    int largerElementIndex = startIndex+1;
    move_t temp;
    for (i = startIndex+1; i <= endIndex; ++i) {
        if (historyTempValues[vec[i] >> 4] >= historyTempValues[partition >> 4]) {
            // Swap the larger/equal item to the left of the larger items
            // This is modified so the moves are going from highest to lowest reward, instead of lowest to highest.
            temp = vec[i];
            vec[i] = vec[largerElementIndex];
            vec[largerElementIndex] = temp;
            // Update largerElementIndex
            ++largerElementIndex;
        }
    }
    // Swap the partition element into place
    temp = vec[startIndex];
    vec[startIndex] = vec[largerElementIndex-1];
    vec[largerElementIndex-1] = temp;

    // Uncomment this line if you want to see each iteration
    //printVec(vec);

    // Recursive calls for two halves
    sortMovesByCutoffs(vec, startIndex, largerElementIndex-2,
                       board, quietHistory, conthist, conthistStack);
    sortMovesByCutoffs(vec, largerElementIndex, endIndex,
                       board, quietHistory, conthist, conthistStack);
}

void sortMoves(MoveList& legalMoves, const ChessBoard &board, move_t hashMove, const TwoKillerMoves &killerMoves,
               const QuietHistory &quietHistory, const Conthist& conthist, const std::vector<uint16_t>& conthistStack) {
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
//    quietHistory.sortMovesByCutoffs(legalMoves.moveList,sortedIndex,backIndex);
    updateHistoryTempValues(legalMoves.moveList,sortedIndex,backIndex,
                                 board,quietHistory,conthist,conthistStack);
    sortMovesByCutoffs(legalMoves.moveList,sortedIndex,backIndex,
                       board,quietHistory,conthist,conthistStack);
}