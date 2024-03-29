#pragma once
#include <array>
#include "../Typedefs.h"
#include "../movegen/ChessBoard.h"
#include "QuietHistory.h"
#include "TwoKillerMoves.h"
class MoveList {
public:
    std::array<move_t,218> moveList;
    unsigned int size = 0;
    void push_back(move_t move);
    void sortMoves(const ChessBoard& board, move_t hashMove, const TwoKillerMoves& killerMoves, const QuietHistory& quietHistory);
};