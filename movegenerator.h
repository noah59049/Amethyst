#pragma once
#include <array>

#include "typedefs.h"
#include "chessboard.h"
#include "moveorder.h"



class MoveGenerator {
private:
    MoveList moves;
    ChessBoard board;
    size_t nextMoveIndex;
    MovegenStage stage;

    move_t nextPseudolegalMove() {
        if (nextMoveIndex == moves.size) {
            switch (stage) {
                case TACTICALS: {
                    nextMoveIndex = 0;
                    stage = QUIETS;
                    moves = board.getMoves(stage);
                    return nextPseudolegalMove();
                }
                case QUIETS: {
                    return 0;
                } // end case QUIETS
            } // end switch
        } // end if nextMoveIndex == moves.size
        else {
            return moves.at(nextMoveIndex++);
        }
    } // end nextMove method
public:
    explicit MovePicker(const ChessBoard &board1) : board(board1) {
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
