#include "movegenerator.h"

move_t MoveGenerator::nextPseudolegalMove()  {
    if (stage == TT_MOVE) {
        stage = GOOD_TACTICALS;
        hasGenerated = false;
        if (board.isPseudolegal(ttMove))
            return ttMove;
    }

    if (stage == GOOD_TACTICALS) {
        if (!hasGenerated) {
            // We need to actually generate the good tacticals
            // 1. Generate moves
            board.getMoves(goodTacticals, TACTICAL_MOVES);
            // 2. Loop through moves, removing moves with bad SEE, the TT move, and scoring other moves
            for (int i = 0; i < goodTacticals.size; i++) {
                const move_t move = goodTacticals.at(i);
                if (move == ttMove) {
                    goodTacticals.moveList[i] = goodTacticals.pop_back();
                    i--;
                }
                else if (!board.isGoodSEE(move)) {
                    goodTacticals.moveList[i] = goodTacticals.pop_back();
                    i--;
                    quietsBadTacticals.push_back(move);
                }
                else {
                    goodTacticals.moveList[i] |= getMVVLVAScore(move) << 22;
                } // end else
            } // end for loop over goodTacticals.size
            badTacticalsCount = quietsBadTacticals.size;
            std::sort(goodTacticals.begin(), goodTacticals.end(), std::greater<>());
            hasGenerated = true;
        } // end if nextMoveIndex == 0
        if (nextMoveIndex == goodTacticals.size) {
            stage = QUIETS;
            hasGenerated = false;
            nextMoveIndex = badTacticalsCount;
        } // end if we are out of good tactical moves
        else {
            return goodTacticals.at(nextMoveIndex++);
        }
    } // end if stage == GOOD_TACTICALS
    if (stage == QUIETS) {
        if (!hasGenerated) {
            const auto quietBegin = quietsBadTacticals.end();
            board.getMoves(quietsBadTacticals, QUIET_MOVES);
            for (unsigned int i = badTacticalsCount; i < quietsBadTacticals.size; i++) {
                move_t move = quietsBadTacticals.at(i);
                if (move == ttMove) {
                    quietsBadTacticals.moveList[i] = quietsBadTacticals.pop_back();
                }
                else {
                    const auto historyScore = threadData.butterflyHistory[board.getSTM()][mvs::getFromTo(move)];
                    quietsBadTacticals.moveList[i] |= move_t(512 + historyScore) << 22;
                } // end else
            } // end for loop over i
            std::sort(quietBegin, quietsBadTacticals.end(), std::greater<>()); // TODO: use lazy selection sort instead
            hasGenerated = true;
        } // end if !hasGenerated
        if (nextMoveIndex == quietsBadTacticals.size) {
            stage = BAD_TACTICALS;
            quietsBadTacticals.trimToSize(badTacticalsCount);
            nextMoveIndex = 0;
        }
        else {
            return quietsBadTacticals.at(nextMoveIndex++);
        } // end else
    } // end if stage == QUIETS
    if (stage == BAD_TACTICALS) {
        if (nextMoveIndex == badTacticalsCount)
            return 0;
        else
            return quietsBadTacticals.at(nextMoveIndex++);
        // Bad tacticals were already generated in the generation of good tacticals
    } // end if stage == BAD_TACTICALS
    exit(1); // We shouldn't ever reach here
} // end nextPseudolegalMove method

MoveGenerator::MoveGenerator(const sg::ThreadData &threadData, const ChessBoard &board1, move_t ttMove) : threadData(threadData), board(board1) {
    this->ttMove = ttMove;
    stage = TT_MOVE;
    nextMoveIndex = 0;
    badTacticalsCount = 0;
    hasGenerated = false;
}

move_t MoveGenerator::nextMove() {
    move_t move;
    do {
        move = nextPseudolegalMove();
    }
    while (move != 0 and !board.isLegal(move));
    return move;
}