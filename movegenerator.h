#pragma once
#include <array>
#include <functional>

#include "typedefs.h"
#include "chessboard.h"
#include "moveorder.h"
#include "searchglobals.h"


class MoveGenerator {
private:
    const sg::ThreadData& threadData;
    MoveList goodTacticals;
    MoveList quiets;
    ChessBoard board;
    MovegenStage stage;
    size_t nextMoveIndex;
    size_t badTacticalsCount;
    bool hasGenerated;
    move_t ttMove;

    move_t nextPseudolegalMove() {
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
                    }
                    else if (!board.isGoodSEE(move)) {
                        goodTacticals.moveList[i] = goodTacticals.pop_back();
                        quiets.push_back(move);
                    }
                    else {
                        goodTacticals.moveList[i] = getMVVLVAScore(move);
                    } // end else
                } // end for loop over goodTacticals.size
                badTacticalsCount = quiets.size;
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
                const auto quietBegin = quiets.end();
                board.getMoves(quiets, QUIET_MOVES);
                for (unsigned int i = badTacticalsCount; i < quiets.size; i++) {
                    move_t move = quiets.at(i);
                    if (move == ttMove) {
                        quiets.moveList[i] = quiets.pop_back();
                    }
                    else {
                        const auto historyScore = threadData.butterflyHistory[board.getSTM()][mvs::getFromTo(move)];
                        quiets.moveList[i] |= move_t(512 + historyScore) << 22;
                    } // end else
                } // end for loop over i
                std::sort(quietBegin, quiets.end(), std::greater<>()); // TODO: use lazy selection sort instead
            } // end if !hasGenerated
            if (nextMoveIndex == quiets.size) {
                stage = BAD_TACTICALS;
                quiets.trimToSize(badTacticalsCount);
                nextMoveIndex = 0;
            }
            else {
                return quiets.at(nextMoveIndex++);
            } // end else
        } // end if stage == QUIETS
        if (stage == BAD_TACTICALS) {
            if (nextMoveIndex == badTacticalsCount)
                return 0;
            else
                return quiets.at(nextMoveIndex++);
            // Bad tacticals were already generated in the generation of good tacticals
        } // end if stage == BAD_TACTICALS
        exit(1); // We shouldn't ever reach here
    } // end nextMove method
public:
    explicit MoveGenerator(const sg::ThreadData &threadData, const ChessBoard &board1, move_t ttMove) : threadData(threadData), board(board1) {
        this->ttMove = ttMove;
        stage = TT_MOVE;
        nextMoveIndex = 0;
        badTacticalsCount = 0;
        hasGenerated = false;
    }

    move_t nextMove() {
        move_t move;
        do {
            move = nextPseudolegalMove();
        }
        while (move != 0 and !board.isLegal(move));
        return move;
    }
}; // end class
