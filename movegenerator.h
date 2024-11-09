#pragma once
#include <array>

#include "typedefs.h"
#include "chessboard.h"



class MovePicker {
private:
    std::array<move_t, 256> moves;
    const ChessBoard board;
public:
    explicit MovePicker(const ChessBoard &board);
    void add(move_t move, MovegenStage stage);
};
