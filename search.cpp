#include "search.h"

#include "moveorder.h"
#include "movegenerator.h"

#include <iostream>
#include <exception>
#include <chrono>
#include <functional>
#include <cmath>
#include <algorithm>

class SearchCancelledException : std::exception {

};

eval_t qsearch(sg::ThreadData& threadData, const ChessBoard& board, const depth_t ply, eval_t alpha, const eval_t beta, const move_t lastMove) {
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

    // Step 3: Check stand-pat
    const eval_t staticEval = board.getEval();
    eval_t bestScore = staticEval;
    if (bestScore >= beta)
        return bestScore;
    if (bestScore > alpha)
        alpha = bestScore;

    // Step 4: Get the move list, sorted by MVV-LVA
    MoveList moves;
    board.getMoves(moves, TACTICAL_MOVES);
    scoreMovesByMVVLVA(moves);
    std::sort(moves.begin(), moves.end(), std::greater<>());

    // Step 5: Search all the moves
    for (move_t move : moves) {
        if (board.isLegal(move) and board.isGoodSEE(move)) {
            ChessBoard newBoard = board;
            newBoard.makemove(move);
            eval_t newScore = -qsearch(threadData, newBoard, ply + 1, -beta, -alpha, move);
            if (newScore > bestScore) {
                bestScore = newScore;
                if (newScore > alpha) {
                    alpha = newScore;
                    if (newScore >= beta) {
                        break;
                    } // end if newScore >= beta
                } // end if newScore > alpha
            } // end if newScore > bestScore
        } // end if board.isLegal(move)
    } // end for loop over moves

    return bestScore;
}

eval_t negamax(sg::ThreadData& threadData, const ChessBoard& board, depth_t depth, const depth_t ply, eval_t alpha, const eval_t beta, const move_t lastMove) {
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

    // Step 3: Initialize certain useful variables for search
    const bool isRoot = ply == 0;
    const bool is50mrDraw = board.getHalfmove() >= 100;
    const bool inCheck = board.isInCheck();
    const side_t stm = board.getSTM();
    const zobrist_t zobristCode = board.getZobristCode();

    // Step 4: Check for game end conditions
    // Annoyingly, if there have been 50 moves since a capture or pawn move, and you are in checkmate, it's not a draw.
    if (is50mrDraw and !inCheck)
        return 0;
    if (!isRoot and sg::repetitionTables[stm].isRepeated(zobristCode))
        return 0;

    // Step 5: Probe the TT
    const TTEntry ttEntry = sg::GLOBAL_TT.get(zobristCode);
    move_t ttMove = ttEntry.ttMove;
    eval_t ttScore = ttEntry.score;
    ttflag_t ttFlag = ttEntry.ttFlag;
    depth_t ttDepth = ttEntry.depth;

    // Step 6: Check for TT cutoffs
    if (!isRoot and ttDepth >= depth) {
        if (ttFlag == ttflags::EXACT)
            return ttScore;
        if (ttFlag == ttflags::UPPER_BOUND and ttScore <= alpha)
            return ttScore;
        if (ttFlag == ttflags::LOWER_BOUND and ttScore >= beta)
            return ttScore;
    }

    // Internal iterative reductions
    if (!isRoot and ttMove == 0 and depth > 3)
        depth--;

    // Step 7: Check if depth is 0 or less
    if (depth <= 0)
        return qsearch(threadData, board, ply, alpha, beta, lastMove);

    // Step 8: Try RFP
    const eval_t staticEval = board.getEval();
    if (!inCheck and depth <= 5 and staticEval - 100 * depth >= beta)
        return beta;

    // Step 9: Try NMP
    if (!isRoot and !sg::isMateScore(beta) and board.canTryNMP()) {
        const depth_t R = 4 + depth / 5;
        ChessBoard nmBoard = board;
        nmBoard.makeNullMove();
        const eval_t nmScore = -negamax(threadData, nmBoard, depth - R, ply + 1, -beta, -beta + 1, 1);
        if (nmScore >= beta) {
            return nmScore;
        }
    }

    // Step 10: Initialize variables for moves searched through
    MoveList movesTried;
    MoveGenerator generator(threadData, board, ttMove);
    eval_t newScore;
    eval_t bestScore = sg::SCORE_MIN;
    move_t bestMove = 0;
    int moveCount = 0;
    bool improvedAlpha = false;

    // Step 11: Overwrite the current entry of the search stack
    threadData.searchStack[ply].zobristCode = zobristCode;
    threadData.searchStack[ply].move = lastMove;
    threadData.searchStack[ply].staticEval = staticEval;

    // Step 13: Search all the moves
    while (move_t move = generator.nextMove()) {
        // We first have to handle some annoying edge cases
        if (isRoot and depth == 1 and moveCount == 0)
            threadData.rootBestMove = move; // This is to make sure there is always a root best move
        if (is50mrDraw)
            return 0;

        ChessBoard newBoard = board;
        newBoard.makemove(move);
        movesTried.push_back(move);
        moveCount++;

        int R = 1;
        bool doReducedSearch = depth > 2 and moveCount > 1;
        bool doZWS = moveCount > 1;
        bool doFullSearch = !doZWS;
        if (doReducedSearch) {
            R = sg::getBaseLMR(depth, moveCount);
            R = std::min(R, depth - 1);
            if (R <= 1)
                doReducedSearch = false;
        }

        if (doReducedSearch) {
            newScore = -negamax(threadData, newBoard, depth - R, ply + 1, -alpha - 1, -alpha, move);
            if (newScore <= alpha) {
                doZWS = false;
                doFullSearch = false;
            }
        }
        if (doZWS) {
            newScore = -negamax(threadData, newBoard, depth - 1, ply + 1, -alpha - 1, -alpha, move);
            if (alpha < newScore and newScore < beta)
                doFullSearch = true;
        }
        if (doFullSearch) {
            newScore = -negamax(threadData, newBoard, depth - 1, ply + 1, -beta, -alpha, move);
        }

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

        // Late move pruning
        if (moveCount >= 8 * depth)
            break;
    } // end for loop over moves

    // Step 14: Deal with checkmates and stalemates
    if (moveCount == 0) {
        bestScore = inCheck ? -sg::SCORE_MATE : 0;
    }

    // Step 15: Update history in case of a beta cutoff from a quiet move
    if (bestScore >= beta) {
        const history_t bonus = std::clamp(depth * depth, -512, 511);
        // Step 15A: bonus to cutoff move if it's quiet
        if (mvs::isQuiet(bestMove)) {
            const auto fromTo = mvs::getFromTo(bestMove);
            threadData.butterflyHistory[stm][fromTo] += bonus - bonus * threadData.butterflyHistory[stm][fromTo] / 511;
        } // end if best move is quiet
        
        // Step 15B: malus to all moves before this that didn't cause a cutoff
        for (move_t move : movesTried) {
            if (mvs::isQuiet(move) and move != bestMove) {
                const auto fromTo = mvs::getFromTo(move);
                threadData.butterflyHistory[stm][fromTo] -= bonus + bonus * threadData.butterflyHistory[stm][fromTo] / 512;
            } // end if move is quiet and is not the best move
        } // end for loop over moves tried
    } // end if bestScore >= beta

    // Step 16: Put something in the TT
    const ttflag_t flagForTT = bestScore >= beta ? ttflags::LOWER_BOUND : (improvedAlpha ? ttflags::EXACT : ttflags::UPPER_BOUND);
    const move_t bestMoveForTT = improvedAlpha ? bestMove : 0;
    sg::GLOBAL_TT.put(zobristCode, bestMoveForTT, bestScore, flagForTT, depth);

    // Step 17: Return the score
    return bestScore;
}

sg::ThreadData rootSearch(const ChessBoard board) {
    // Step 1: Initialize thread data
    sg::ThreadData rootThreadData;
    eval_t score = board.getEval();
    std::string rootBestMove;
    bool cancelled = false;

    // Step 2: Iterative deepening search
    for (depth_t depth = 1; depth <= sg::depthLimit and !cancelled; depth++) {
        // Step 2.1: Do the search
        try {
            score = negamax(rootThreadData, board, depth_t(depth), depth_t(0), sg::SCORE_MIN, sg::SCORE_MAX, 0);
        }
        catch (const SearchCancelledException& e) {
            cancelled = true;
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
