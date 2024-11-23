#include "search.h"

#include <iostream>

void rootSearch(ChessBoard board) {
    // Step 1: Initialize thread data
    sg::ThreadData rootThreadData;

    // Step 2: Fixed depth search to depth 3
    eval_t score = negamax(rootThreadData, board, 3, 0);

    // Step 3: Printout stuff
    std::string rootBestMove = moveToLAN(rootThreadData.rootBestMove);
    std::cout << "info depth 3 nodes " << rootThreadData.nodes << " score cp " << " pv " << rootBestMove << std::endl;
    std::cout << "bestmove " << rootBestMove << std::endl;
}

eval_t negamax(sg::ThreadData& threadData, const ChessBoard& board, depth_t depth, depth_t ply) {
    // Step 1: Increment nodes
    threadData.nodes++;

    // Step 2: Initialize certain useful booleans
    const bool isRoot = ply == 0;
    const bool is50mrDraw = board.getHalfmove() >= 100;
    const bool inCheck = board.isInCheck();

    // Step 3: Check for game end conditions
    // Annoyingly, if there is have been 50 moves since a capture or pawn move and you are in checkmate, it's not a draw.
    if (is50mrDraw and !inCheck)
        return 0;
    if (depth == 0)
        return board.getEval();

    // Step 4: Initialize variables for moves searched through
    MoveList moves = board.getPseudoLegalMoves();
    eval_t bestScore = sg::SCORE_MIN;
    move_t bestMove = 0;
    int movesSearched = 0;

    // Step 5: Search all the moves
    for (move_t move : moves) {
        if (board.isLegal(move)) {
            if (is50mrDraw)
                return 0;
            ChessBoard newBoard = board;
            newBoard.makemove(move);
            movesSearched++;
            eval_t newScore = -negamax(threadData, board, depth - 1, ply + 1);
            if (newScore > bestScore) {
                bestScore = newScore;
                bestMove = move;
                if (isRoot)
                    threadData.rootBestMove = move;
            } // end if newScore > bestScore
        } // end if move is legal
    } // end for loop over moves

    // Step 6: Deal with checkmates and stalemates
    if (movesSearched == 0) {
        return inCheck ? -sg::SCORE_MATE : 0;
    }

    // Step 7: Return the score
    return bestScore;
}