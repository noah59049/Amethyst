#pragma once

#include "searchglobals.h"
#include "chessboard.h"

eval_t negamax(sg::ThreadData& threadData, const ChessBoard& board, depth_t depth, depth_t ply);

sg::ThreadData rootSearch(ChessBoard board);