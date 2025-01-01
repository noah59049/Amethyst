#pragma once
#include <array>

#include "typedefs.h"
#include "chessboard.h"
#include "moveorder.h"



class MoveGenerator {
private:
    MoveList goodTacticals;
    MoveList quiets;
    ChessBoard board;
    size_t nextMoveIndex;
    MovegenStage stage;
    move_t ttMove;

    move_t nextPseudolegalMove() {
        if (stage == TT_MOVE) {
            stage = GOOD_TACTICALS;
            if (board.isPseudolegal(ttMove))
                return ttMove;
            else
                return this->nextPseudolegalMove();
        }
        else if (stage == GOOD_TACTICALS) {
            if (nextMoveIndex == 0) {

            }
        }
    } // end nextMove method
public:
    explicit MoveGenerator(const ChessBoard &board1, move_t ttMove) : board(board1) {
        this->ttMove = ttMove;
        nextMoveIndex = 0;
        stage = TACTICALS;
        moves = board.getMoves(stage);
        scoreMovesByMVVLVA(moves);
        std::sort(moves.begin(), moves.end(), std::greater<>());
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
