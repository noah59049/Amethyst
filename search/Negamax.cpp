#include "Negamax.h"
#include "MoveOrder.h"
#include "../hce/Eval.h"
using namespace std;

eval_t search::getNegaQuiescenceEval(ChessBoard &board, eval_t alpha, eval_t beta) {
    if (board.isInCheck())
        board.updateMates();
    if (board.hasGameEnded())
        return board.getNegaStaticEval();

    alpha = max(alpha, board.getNegaStaticEval());
    if (alpha >= beta) {
        return beta;
    }

    MoveList captures;
    board.getNonnegativeSEECapturesOnly(captures);

    eval_t newscore;
    for (move_t move: captures) {
        ChessBoard newBoard = board;
        newBoard.makemoveLazy(move);
        newscore = -getNegaQuiescenceEval(newBoard, -beta, -alpha);
        if (newscore >= beta)
            return beta;
        if (newscore > alpha)
            alpha = newscore;
    }

    return alpha;
}

eval_t search::getNegamaxEval(ChessBoard &board, int depth, eval_t alpha, const eval_t beta, bool* isCancelled) {
    if (board.hasGameEnded())
        return board.getNegaStaticEval();
    if (repetitionTable.count(board) >= 2) // If we go there a 3rd time, it's a draw.
        return 0;

    // Check move extension
    if (board.isInCheck())
        depth++;


    move_t hashMove = SEARCH_FAILED_MOVE_CODE;
    // Check if our result is in the transposition table, plus add hash move if it is
    std::optional<TTValue> ttHashValue = transpositionTable.get(board.getZobristCode(),depth);
    if (ttHashValue != std::nullopt) { // our thing was in the transposition table! Yay!
        TTValue ttValue = ttHashValue.value();
        if (ttValue.lowerBoundEval == ttValue.upperBoundEval)
            return ttValue.lowerBoundEval;
        else if (ttValue.lowerBoundEval >= beta)
            return beta;
        else if (ttValue.upperBoundEval <= alpha)
            return alpha;
        else if (ttValue.hashMove != SEARCH_FAILED_MOVE_CODE) // We add the hash move
            hashMove = ttValue.hashMove;
    }
        // Otherwise, we do IID
    else {
        for (int depthDecrease = 1; depthDecrease <= IID_DEPTH_DECREASE; depthDecrease++) {
            if (depth <= depthDecrease)
                break;
            std::optional<TTValue> prevHashValue = transpositionTable.get(board.getZobristCode(),depth - depthDecrease);
            if (prevHashValue != std::nullopt and prevHashValue->hashMove != SEARCH_FAILED_MOVE_CODE) {
                hashMove = prevHashValue->hashMove;
                break;
            }
        }
    }

    if (depth == 0)
        return getNegaQuiescenceEval(board, alpha, beta);
    if (*isCancelled)
        throw SearchCancelledException();

    // Null move pruning (technically null move reductions)
    if (board.canMakeNullMove()) {
        eval_t staticEval = board.getNegaStaticEval();
        // Reverse futility pruning
        if (depth <= MAX_RFP_DEPTH and staticEval - RFP_MARGIN * eval_t(depth) >= beta)
            return beta;

        ChessBoard nmBoard = board;
        nmBoard.makeNullMove();
        if (-getNegamaxEval(nmBoard, max(0,depth - NMP_REDUCTION),-beta - 1, -beta, isCancelled) > beta) {
            // To guard against zugzwang, we do a search to depth - 4 without a null move, and if THAT causes a beta cutoff, then we return beta.
            if (getNegamaxEval(board, max(0,depth - 4), beta , beta + 1, isCancelled) > beta) {
                // We know we caused a beta cutoff, but we don't know what the best move is
                transpositionTable.put({beta,MAX_EVAL,SEARCH_FAILED_MOVE_CODE,board.getZobristCode(),depth});
                return beta;
            }
        }
    }

    // clear the killer moves from irrelevant positions
    if (depth > KILLER_MAX_COUSIN_LEVEL)
        killerMoves[depth - KILLER_MAX_COUSIN_LEVEL].clear();

    MoveList legalMoves;
    board.getLegalMoves(legalMoves);

    // Checkmate and stalemate detection
    if (legalMoves.size == 0)
        return board.isInCheck() ? -hce::MATE_VALUE : 0;

    // Move ordering
    if (board.getIsItWhiteToMove())
        sortMoves(legalMoves,board,hashMove,killerMoves.at(depth),whiteQuietHistory);
    else
        sortMoves(legalMoves,board,hashMove,killerMoves.at(depth),blackQuietHistory);

    // Set up LMR
    const unsigned int numMovesToNotReduce = QUIETS_TO_NOT_REDUCE;
    unsigned int numMovesSearched = 0;

    // Late move pruning
    if (depth <= MAX_LMP_DEPTH) {
        legalMoves.trimToSize(LMP_MOVECOUNT * depth);
    }

    eval_t newscore;
    eval_t bestscore = MIN_EVAL;
    move_t bestmove = SEARCH_FAILED_MOVE_CODE;
    bool improvedAlpha = false;
    for (move_t move: legalMoves) {
        ChessBoard newBoard = board;
        newBoard.makemoveLazy(move);
        numMovesSearched++;

        // Late move reductions
        if (depth >= MIN_LMR_DEPTH and numMovesSearched > numMovesToNotReduce) {
            eval_t reducedScore = -getNegamaxEval(newBoard, depth - 2, -alpha - 1, -alpha, isCancelled);
            if (reducedScore <= alpha)
                continue;
        }
        newscore = -getNegamaxEval(newBoard, depth - 1, -beta, -alpha, isCancelled);

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
                    killerMoves[depth].recordKillerMove(move);
                    // Record a move for history heuristic
                    if (board.getIsItWhiteToMove())
                        whiteQuietHistory.recordKillerMove(move, legalMoves, depth * depth);
                    else
                        blackQuietHistory.recordKillerMove(move, legalMoves, depth * depth);
                }
                transpositionTable.put({beta,MAX_EVAL,move,board.getZobristCode(),depth});
                return beta;
            } // end if alpha > beta
        } // end if newscore > alpha
    } // end for loop over moves
    if (improvedAlpha) { // This is a PV node, and score is exact
        transpositionTable.put({alpha,alpha,bestmove,board.getZobristCode(),depth});
    }
    else { // None of the moves improved alpha. The score is an upper bound
        // Change implemented in Amethyst 43: At an All-Node, we don't add the "best move" to the transposition table because that's just noise
        transpositionTable.put({MIN_EVAL,alpha,SEARCH_FAILED_MOVE_CODE,board.getZobristCode(),depth});
    }
    return alpha;
}

void search::getNegamaxBestMoveAndEval(ChessBoard &board, const int depth, bool* isCancelled, const eval_t aspirationWindowCenter,
                               move_t &bestMove, eval_t &eval) {

    // Internal iterative deepening
    move_t hashMove = SEARCH_FAILED_MOVE_CODE;
    for (int depthDecrease = 1; depthDecrease <= IID_DEPTH_DECREASE; depthDecrease++) {
        if (depth <= depthDecrease)
            break;
        std::optional<TTValue> prevHashValue = transpositionTable.get(board.getZobristCode(),depth - depthDecrease);
        if (prevHashValue != std::nullopt and prevHashValue->hashMove != SEARCH_FAILED_MOVE_CODE) {
            hashMove = prevHashValue->hashMove;
            break;
        }
    }

    MoveList legalMoves;
    board.getLegalMoves(legalMoves);
    sortMoves(legalMoves,board,hashMove,TwoKillerMoves(),board.getIsItWhiteToMove() ? whiteQuietHistory : blackQuietHistory);

    eval_t startAlpha = aspirationWindowCenter - ASPIRATION_WINDOW_RADIUS;
    eval_t beta = aspirationWindowCenter + ASPIRATION_WINDOW_RADIUS;

    eval_t newscore;
    eval_t alpha = startAlpha;
    move_t bestmove = SEARCH_FAILED_MOVE_CODE;
    while (alpha <= startAlpha or alpha >= beta) { // in other words, we leave this loop once we score inside the aspiration window.
        for (move_t move: legalMoves) {
            ChessBoard newBoard = board;
            newBoard.makemove(move);
            newscore = -search::getNegamaxEval(newBoard, depth - 1, -beta, -alpha, isCancelled);
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
            transpositionTable.put({alpha,alpha,bestmove,board.getZobristCode(),depth});
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

//void search::timeSearchFromFEN (const string& fenNotation, int maxDepth) {
//    ChessBoard board = ChessBoard::boardFromFENNotation(fenNotation);
//    eval_t negaEval = 0;
//    for (int depth = 1; depth <= maxDepth; depth++) {
//        auto start = std::chrono::high_resolution_clock::now();
//        bool* isCancelled = new bool(false);
//        RepetitionTable repetitionTable;
//        NegamaxData data(isCancelled,repetitionTable,depth);
//        move_t bestMove = SEARCH_FAILED_MOVE_CODE;
//        getNegamaxBestMoveAndEval(board,depth,data,negaEval,bestMove,negaEval);
//        string readableBestMove = board.moveToSAN(bestMove);
//        delete isCancelled;
//        auto end = std::chrono::high_resolution_clock::now();
//        auto duration = std::chrono::duration_cast<chrono::milliseconds>(end - start);
//        long ms = duration.count();
//        cout << "Nega eval is " << negaEval << " and best move is " << readableBestMove << " at depth " << depth << " in " << ms << " milliseconds." << endl;
//    } // end for loop
//} // end timeSearchFromTraxler
//
//void search::timeSearchFromTraxler () {
//    timeSearchFromFEN("r1bqk2r/pppp1Npp/2n2n2/4p3/2B1P3/8/PPPP1KPP/RNBQ3R b kq - 0 6",7);
//} // end timeSearchFromTraxler
