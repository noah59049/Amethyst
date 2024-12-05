#include "search.h"

#include "moveorder.h"

#include <iostream>
#include <exception>
#include <chrono>
#include <functional>

class SearchCancelledException : std::exception {

};

sg::ThreadData rootSearch(const ChessBoard board) {
    // Step 1: Initialize thread data
    sg::ThreadData rootThreadData;
    eval_t score = board.getEval();
    std::string rootBestMove;

    // Step 2: Iterative deepening search
    for (depth_t depth = 1; depth <= sg::depthLimit; depth++) {
        // Step 2.1: Do the search
        try {
            score = negamax(rootThreadData, board, depth_t(depth), depth_t(0), sg::SCORE_MIN, sg::SCORE_MAX, 0);
        }
        catch (const SearchCancelledException& e) {
            break;
        }

        // Step 2.2: Get elapsed time
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto msElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - rootThreadData.searchStartTime).count();

        // Step 2.3: Print out stuff
        rootBestMove = moveToLAN(rootThreadData.rootBestMove);

        std::cout << "info depth " << int(depth) << " nodes " << rootThreadData.nodes << " time " << msElapsed << " score cp " << score << " pv " << rootBestMove << std::endl;

        // Step 2.4: Check for soft time/depth/nodes limit
        if (msElapsed > sg::softTimeLimit or rootThreadData.nodes >= sg::nodesLimit)
            break;

    }

    // Step 3: Print out bestmove
    std::cout << "bestmove " << rootBestMove << std::endl;

    // Step 4: Return the thread data
    return rootThreadData;
}

eval_t negamax(sg::ThreadData& threadData, const ChessBoard& board, depth_t depth, depth_t ply, eval_t alpha, eval_t beta, move_t lastMove) {
    // Step 1: Increment nodes
    threadData.nodes++;

    // Step 2: Check for hard time limit
    if (threadData.nodes % 1024 == 0) {
        auto now = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - threadData.searchStartTime);
        auto msElapsed = duration.count();
        if (msElapsed >= sg::hardTimeLimit)
            throw SearchCancelledException();
    }

    // Step 3: Initialize certain useful booleans
    const bool isRoot = ply == 0;
    const bool is50mrDraw = board.getHalfmove() >= 100;
    const bool inCheck = board.isInCheck();

    // Step 4: Check for game end conditions
    // Annoyingly, if there have been 50 moves since a capture or pawn move, and you are in checkmate, it's not a draw.
    if (is50mrDraw and !inCheck)
        return 0;
    if (depth == 0)
        return board.getEval();
    if (!isRoot and sg::repetitionTables[board.getSTM()].isRepeated(board.getZobristCode()))
        return 0;

    // Step 5: Initialize variables for moves searched through
    eval_t staticEval = board.getEval();
    MoveList rawMoves = board.getPseudoLegalMoves();
    eval_t bestScore = sg::SCORE_MIN;
    move_t bestMove = 0;
    int movesSearched = 0;
    bool improvedAlpha = false;

    // Step 6: Overwrite the current entry of the search stack
    threadData.searchStack[ply].zobristCode = board.getZobristCode();
    threadData.searchStack[ply].move = lastMove;
    threadData.searchStack[ply].staticEval = staticEval;

    // Step 7: Sort moves according to: tactical moves, then quiets
    // This could be done more efficiently with staged movegen
    MoveList moves;
    for (move_t move : rawMoves) {
        if (mvs::isTactical(move))
            moves.push_back(move);
    }

    scoreMovesByMVVLVA(moves);
    std::sort(moves.begin(), moves.end(), std::greater<move_t>());


    for (move_t move : rawMoves) {
        if (mvs::isQuiet(move))
            moves.push_back(move);
    }

    // Step 8: Search all the moves
    for (move_t move : moves) {
        if (board.isLegal(move)) {
            if (is50mrDraw)
                return 0;
            ChessBoard newBoard = board;
            newBoard.makemove(move);
            movesSearched++;
            eval_t newScore = -negamax(threadData, newBoard, depth - 1, ply + 1, -beta, -alpha, move);
            if (newScore > bestScore) {
                bestScore = newScore;
                bestMove = move;
                if (newScore > alpha) {
                    improvedAlpha = true;
                    alpha = newScore;
                    if (isRoot)
                        threadData.rootBestMove = move;
                    if (newScore >= beta)
                        break;
                } // end if newScore > alpha
            } // end if newScore > bestScore
        } // end if move is legal
    } // end for loop over moves

    // Step 9: Deal with checkmates and stalemates
    if (movesSearched == 0) {
        return inCheck ? -sg::SCORE_MATE : 0;
    }

    // Step 10: Return the score
    return bestScore;
}