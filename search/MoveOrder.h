#pragma once
#include "../movegen/MoveList.h"
#include "../movegen/ChessBoard.h"
#include "QuietHistory.h"
#include "TwoKillerMoves.h"
void sortMoves(MoveList& legalMoves, const ChessBoard& board, move_t hashMove, const TwoKillerMoves& killerMoves, const QuietHistory& quietHistory,
               unsigned int movesToKeep);
void sortCapturesByMVVLVA(const ChessBoard& board, MoveList& captures);