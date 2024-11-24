#pragma once

#include "searchglobals.h"
#include "chessboard.h"

eval_t negamax(sg::ThreadData& threadData, const ChessBoard& board, depth_t depth, depth_t ply, eval_t alpha, eval_t beta);

sg::ThreadData rootSearch(ChessBoard board);