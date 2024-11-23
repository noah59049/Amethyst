#pragma once

#include "searchglobals.h"
#include "chessboard.h"

void rootSearch(ChessBoard board);

eval_t negamax(const sg::ThreadData& threadData, const ChessBoard& board, depth_t depth, depth_t ply);