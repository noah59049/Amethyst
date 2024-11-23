#include "search.h"

#include <iostream>

void rootSearch(ChessBoard board) {
    // In the future we will do the iterative deepening search with aspiration windows
    // For now, just return the first move

    MoveList moves = board.getPseudoLegalMoves();
    for (move_t move : moves) {
        if (board.isLegal(move)) {
            std::cout << "bestmove " << moveToLAN(move) << std::endl;
            return;
        }
    }

    // There are no legal moves
    std::cout << "bestmove 0000" << std::endl;
}

eval_t negamax(sg::ThreadData& threadData, const ChessBoard& board, depth_t depth, depth_t ply) {
    threadData.nodes++;

    const bool isRoot = ply == 0;
    const bool is50mrDraw = board.getHalfmove() >= 100;
    const bool inCheck = board.isInCheck();

    if (is50mrDraw and !inCheck)
        return 0;

    if (depth == 0)
        return board.getEval();

    MoveList moves = board.getPseudoLegalMoves();
    eval_t bestScore = sg::SCORE_MIN;
    move_t bestMove = 0;

    int movesSearched = 0;
    for (move_t move : moves) {
        if (board.isLegal(move)) {
            if (is50mrDraw)
                return 0;
            ChessBoard newBoard = board;
            newBoard.makemove(move);
            movesSearched++;
            eval_t newScore = -negamax(threadData, board, depth - 1, ply);
            if (newScore > bestScore) {
                bestScore = newScore;
                bestMove = move;
                if (isRoot)
                    threadData.rootBestMove = move;
            } // end if newScore > bestScore
        } // end if move is legal
    } // end for loop over moves

    if (movesSearched == 0) {
        return inCheck ? -sg::SCORE_MATE : 0;
    }

    return bestScore;
}