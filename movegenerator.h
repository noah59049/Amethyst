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
    MoveList quietsBadTacticals; // This stores the bad tacticals at the front, and quiets after that
    ChessBoard board;
    MovegenStage stage;
    size_t nextMoveIndex;
    size_t badTacticalsCount;
    bool hasGenerated;
    move_t ttMove;

    move_t nextPseudolegalMove();
public:
    explicit MoveGenerator(const sg::ThreadData &threadData, const ChessBoard &board1, move_t ttMove);
    move_t nextMove();
}; // end class
