#include "Negamax.h"
#include "MoveOrder.h"
#include "../hce/Eval.h"
#include <iostream>
using namespace std;

eval_t search::getNegaQuiescenceEval(ChessBoard &board, eval_t alpha, eval_t beta) {
    if (board.isInCheck())
        board.updateMates();
    if (board.hasGameEnded())
        return board.getNegaStaticEval();

    eval_t bestscore = board.getNegaStaticEval();
    alpha = max(alpha, bestscore);
    if (bestscore >= beta) {
        return bestscore;
    }

    MoveList captures;
    board.getNonnegativeSEECapturesOnly(captures);
    if (captures.size > 1)
        sortCapturesByMVVLVA(board,captures);

    eval_t newscore;
    for (move_t move: captures) {
        ChessBoard newBoard = board;
        newBoard.makemoveLazy(move);
        newscore = -getNegaQuiescenceEval(newBoard, -beta, -alpha);
        if (newscore > bestscore) {
            bestscore = newscore;
            if (newscore > alpha) {
                alpha = newscore;
                if (newscore >= beta)
                    return newscore;
            }
        }
    }

    return bestscore;
}

eval_t search::getNegamaxEval(ChessBoard &board, int depth, eval_t alpha, const eval_t beta, search::NegamaxData& data) {
    if (board.hasGameEnded())
        return board.getNegaStaticEval();
    if (data.repetitionTable.count(board) >= 2) // If we go there a 3rd time, it's a draw.
        return 0;

    // Check move extension
    if (board.isInCheck())
        depth++;

    const bool pvNode = beta - alpha > 1;

    eval_t ttLowerBound = MIN_EVAL;
    eval_t ttUpperBound = MAX_EVAL;
    // ttLowerBound and ttUpperBound are used to replace staticEval, and have no depth minimum

    move_t hashMove = SEARCH_FAILED_MOVE_CODE;
    // Check if our result is in the transposition table, plus add hash move if it is
    std::optional<TTValue> ttHashValue = data.transpositionTable.get(board.getZobristCode(),depth);
    if (ttHashValue != std::nullopt) { // our thing was in the transposition table! Yay!
        TTValue ttValue = ttHashValue.value();
        if (ttValue.lowerBoundEval == ttValue.upperBoundEval)
            return ttValue.lowerBoundEval;
        else if (ttValue.lowerBoundEval >= beta)
            return ttValue.lowerBoundEval;
        else if (ttValue.upperBoundEval <= alpha)
            return ttValue.upperBoundEval;
        else if (ttValue.hashMove != SEARCH_FAILED_MOVE_CODE) // We add the hash move
            hashMove = ttValue.hashMove;

        ttLowerBound = ttValue.lowerBoundEval;
        ttUpperBound = ttValue.upperBoundEval;
    }
    else {
        // Look for the best move from the TT at lower depths
        std::optional<TTValue> prevHashValue = data.transpositionTable.get(board.getZobristCode(),0);
        if (prevHashValue != std::nullopt) {

            // Get a TT move
            if (prevHashValue->hashMove != SEARCH_FAILED_MOVE_CODE and prevHashValue->depth >= depth - IID_DEPTH_DECREASE)
                hashMove = prevHashValue->hashMove;

            // Set TT bounds for replacing static eval
            ttLowerBound = prevHashValue->lowerBoundEval;
            ttUpperBound = prevHashValue->upperBoundEval;
        } // end if prevHashValue != nullopt
    } // end else (we don't have a TT value from this depth)

    if (depth <= 0)
        return getNegaQuiescenceEval(board, alpha, beta);
    if (*data.isCancelled)
        throw SearchCancelledException();

    // Null move pruning
    eval_t staticEval = board.getNegaStaticEval();
    staticEval = std::clamp(staticEval,ttLowerBound,ttUpperBound);

    // Reverse futility pruning
    if (!board.isInCheck() and depth <= MAX_RFP_DEPTH and staticEval - RFP_MARGIN * eval_t(depth) >= beta)
        return staticEval - RFP_MARGIN * eval_t(depth);

    if (board.canMakeNullMove()) {

        const int nmReduction = 2 + depth / 3; // TODO: Turn these into constants
        ChessBoard nmBoard = board;
        nmBoard.makeNullMove();
        if (staticEval >= beta and depth >= MIN_NMP_DEPTH and -getNegamaxEval(nmBoard, max(0,depth - nmReduction),-beta, -beta + 1, data) >= beta) {
            // Verification search at high depth to avoid zugzwang
            if (depth < 9 or getNegamaxEval(board, depth - nmReduction, beta - 1 , beta, data) >= beta) {
                // We know we caused a beta cutoff, but we don't know what the best move is
                data.transpositionTable.put({beta,MAX_EVAL,SEARCH_FAILED_MOVE_CODE,board.getZobristCode(),depth});
                return beta; // TODO: Fail-soft here
            }
        }
    }

    // clear the killer moves from irrelevant positions
    if (depth > KILLER_MAX_COUSIN_LEVEL)
        data.killerMoves[depth - KILLER_MAX_COUSIN_LEVEL].clear();

    MoveList legalMoves;
    board.getLegalMoves(legalMoves);

    // Checkmate and stalemate detection
    if (legalMoves.size == 0)
        return board.isInCheck() ? -hce::MATE_VALUE : 0;

    // Set up late move pruning
    const unsigned int movesToKeep = !pvNode and depth <= MAX_LMP_DEPTH ? depth * LMP_MOVECOUNT : 234;

    sortMoves(legalMoves,board,hashMove,data.killerMoves.at(depth),
              board.getIsItWhiteToMove() ? data.whiteQuietHistory : data.blackQuietHistory,
              movesToKeep);

    legalMoves.trimToSize(movesToKeep);

    const unsigned int numMovesToNotReduce = depth >= SECOND_LMR_DEPTH ?
                                             SECOND_QUIETS_TO_NOT_REDUCE : QUIETS_TO_NOT_REDUCE;

    unsigned int numMovesSearched = 0;

    eval_t newscore;
    eval_t bestscore = FAIL_SOFT_MIN_SCORE;
    move_t bestmove = SEARCH_FAILED_MOVE_CODE;
    bool improvedAlpha = false;
    for (move_t move: legalMoves) {
        ChessBoard newBoard = board;
        newBoard.makemoveLazy(move);
        numMovesSearched++;

        // Late move reductions
        int reduction = 0;
        if (depth >= MIN_LMR_DEPTH and numMovesSearched > numMovesToNotReduce) {
            reduction = 1;
            if (numMovesSearched > 3 * numMovesToNotReduce)
                reduction = 2;
        }
        if (depth >= MIN_FR_DEPTH and move != hashMove and !isCapture(move) and alpha - staticEval > FR_MARGIN) {
            reduction += 1;
        }
        if (depth >= MIN_SEE_REDUCTION_DEPTH and move != hashMove and board.getSEE(move) < 0) {
            reduction += 1;
        }

        if (reduction > 0) {
            eval_t reducedScore = -getNegamaxEval(newBoard, depth - reduction - 1, -alpha - 1, -alpha, data);
            if (reducedScore <= alpha) {
                bestscore = max(bestscore, reducedScore);
                continue;
            }
            newscore = -getNegamaxEval(newBoard, depth - 1, -beta, -alpha, data);
        }
        else if (move != legalMoves.at(0)) { // PVS
            newscore = -search::getNegamaxEval(newBoard, depth - 1, -alpha - 1, -alpha, data);
            if (alpha < newscore and newscore < beta) // Search with full if score is better than alpha but worse than beta
                newscore = -search::getNegamaxEval(newBoard, depth - 1, -beta, -newscore, data);
        }
        else { // Search with full window for first move
            newscore = -search::getNegamaxEval(newBoard, depth - 1, -beta, -alpha, data);
        }

        if (newscore > bestscore) {
            bestscore = newscore;
            bestmove = move;
        }
        if (newscore > alpha) {
            improvedAlpha = true;
            alpha = newscore;
            if (alpha >= beta) { // We caused a beta cutoff
                if (!isCapture(move)) {
                    // Record a killer move
                    data.killerMoves[depth].recordKillerMove(move);
                    // Record a move for history heuristic
                    if (board.getIsItWhiteToMove())
                        data.whiteQuietHistory.recordKillerMove(move, legalMoves, depth * depth);
                    else
                        data.blackQuietHistory.recordKillerMove(move, legalMoves, depth * depth);
                }
                data.transpositionTable.put({newscore,MAX_EVAL,move,board.getZobristCode(),depth});
                return newscore;
            } // end if alpha > beta
        } // end if newscore > alpha
    } // end for loop over moves
    if (improvedAlpha) { // This is a PV node, and score is exact
        data.transpositionTable.put({alpha,alpha,bestmove,board.getZobristCode(),depth});
    }
    else { // None of the moves improved alpha. The score is an upper bound
        // Change implemented in Amethyst 43: At an All-Node, we don't add the "best move" to the transposition table because that's just noise
        data.transpositionTable.put({MIN_EVAL,bestscore,SEARCH_FAILED_MOVE_CODE,board.getZobristCode(),depth});
    }
    return bestscore;
}

void search::getNegamaxBestMoveAndEval(ChessBoard &board, const int depth, NegamaxData& data, const eval_t aspirationWindowCenter,
                                       move_t &bestMove, eval_t &eval) {

    // Get TT move from depth - 1
    move_t hashMove = SEARCH_FAILED_MOVE_CODE;
    std::optional<TTValue> prevHashValue = data.transpositionTable.get(board.getZobristCode(),depth - 1);
    if (prevHashValue != std::nullopt and prevHashValue->hashMove != SEARCH_FAILED_MOVE_CODE) {
        hashMove = prevHashValue->hashMove;
    }

    MoveList legalMoves;
    board.getLegalMoves(legalMoves);
    sortMoves(legalMoves,board,hashMove,data.killerMoves.at(depth),
              board.getIsItWhiteToMove() ? data.whiteQuietHistory : data.blackQuietHistory, 234);

    eval_t startAlpha = aspirationWindowCenter - ASPIRATION_WINDOW_RADIUS;
    eval_t beta = aspirationWindowCenter + ASPIRATION_WINDOW_RADIUS;

    eval_t newscore;
    eval_t alpha = startAlpha;
    move_t bestmove = SEARCH_FAILED_MOVE_CODE;
    while (alpha <= startAlpha or alpha >= beta) { // in other words, we leave this loop once we score inside the aspiration window.
        for (move_t move: legalMoves) {
            ChessBoard newBoard = board;
            newBoard.makemove(move);
            if (move != legalMoves.at(0)) { // PVS
                newscore = -search::getNegamaxEval(newBoard, depth - 1, -alpha - 1, -alpha, data);
                if (alpha < newscore and newscore < beta) { // Search with full if score is better than alpha but worse than beta
                    bestMove = bestmove = move; // This seemingly inconsequential line is worth 32 Elo
                    newscore = -search::getNegamaxEval(newBoard, depth - 1, -beta, -newscore, data);
                }
            }
            else { // Search with full window for first move
                newscore = -search::getNegamaxEval(newBoard, depth - 1, -beta, -alpha, data);
            }
            if (newscore > alpha) {
                alpha = newscore;
                bestMove = bestmove = move;
                if (alpha >= beta) {
                    break;
                }
            }
        }
        if (startAlpha < alpha and alpha < beta) {
            bestMove = bestmove;
            eval = alpha;
            data.transpositionTable.put({alpha,alpha,bestmove,board.getZobristCode(),depth});
        }
        else {
            // Search returned an evaluation outside the aspiration window
            // We need to redo the search with a wider window.
            startAlpha = -FULL_WINDOW_VALUE;
            beta = FULL_WINDOW_VALUE;
            alpha = startAlpha;
        } // end else
    } // end while
} // end getNegamaxBestMoveAndEval

void search::timeSearchFromFEN (const string& fenNotation, int maxDepth) {
    ChessBoard board = ChessBoard::boardFromFENNotation(fenNotation);
    eval_t negaEval = 0;
    for (int depth = 1; depth <= maxDepth; depth++) {
        auto start = std::chrono::high_resolution_clock::now();
        bool* isCancelled = new bool(false);
        RepetitionTable repetitionTable;
        NegamaxData data(isCancelled,repetitionTable,depth);
        move_t bestMove = SEARCH_FAILED_MOVE_CODE;
        getNegamaxBestMoveAndEval(board,depth,data,negaEval,bestMove,negaEval);
        string readableBestMove = board.moveToSAN(bestMove);
        delete isCancelled;
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<chrono::milliseconds>(end - start);
        long ms = duration.count();
        cout << "Nega eval is " << negaEval << " and best move is " << readableBestMove << " at depth " << depth << " in " << ms << " milliseconds." << endl;
    } // end for loop
} // end timeSearchFromTraxler

void search::timeSearchFromTraxler () {
    timeSearchFromFEN("r1bqk2r/pppp1Npp/2n2n2/4p3/2B1P3/8/PPPP1KPP/RNBQ3R b kq - 0 6",7);
} // end timeSearchFromTraxler
