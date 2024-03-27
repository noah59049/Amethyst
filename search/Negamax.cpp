#include "Negamax.h"
#include <iostream>
using namespace std;

float search::getNegaQuiescenceEval(const ChessBoard &board, float alpha, float beta) {
    if (board.hasGameEnded())
        return board.getNegaStaticEval();

    alpha = max(alpha, board.getNegaStaticEval());
    if (alpha >= beta)
        return beta;

    vector<move_t> captures;
    captures.reserve(5);

    board.getNonnegativeSEECapturesOnly(captures);

    float newscore;
    for (move_t move: captures) {
        ChessBoard newBoard = board;
        newBoard.makemove(move);
        newscore = -getNegaQuiescenceEval(newBoard, -beta, -alpha);
        if (newscore >= beta)
            return beta;
        if (newscore > alpha)
            alpha = newscore;
    }

    return alpha;
}

float search::getNegamaxEval(const ChessBoard &board, int depth, float alpha, const float beta, search::NegamaxData& data) {
    if (board.hasGameEnded())
        return board.getNegaStaticEval();
    if (data.repetitionTable.count(board) >= 2) // If we go there a 3rd time, it's a draw.
        return 0;

    // Check move extension
    if (board.isInCheck())
        depth++;

    vector<move_t> winningEqualCaptures;
    vector<move_t> losingCaptures;
    vector<move_t> nonCaptures;

    // Check if our result is in the transposition table, plus add hash move if it is
    std::optional<TTValue> ttHashValue = data.transpositionTable.get(board.getZobristCode(),depth);
    if (ttHashValue != std::nullopt) { // our thing was in the transposition table! Yay!
        TTValue ttValue = ttHashValue.value();
        if (ttValue.lowerBoundEval == ttValue.upperBoundEval)
            return ttValue.lowerBoundEval;
        else if (ttValue.lowerBoundEval >= beta)
            return beta;
        else if (ttValue.upperBoundEval <= alpha)
            return alpha;
        else if (ttValue.hashMove != SEARCH_FAILED_MOVE_CODE) // We add the hash move
            winningEqualCaptures.push_back(ttValue.hashMove);
    }
        // Otherwise, we do IID
    else {
        for (int depthDecrease = 1; depthDecrease <= IID_DEPTH_DECREASE; depthDecrease++) {
            if (depth <= depthDecrease)
                break;
            std::optional<TTValue> prevHashValue = data.transpositionTable.get(board.getZobristCode(),depth - depthDecrease);
            if (prevHashValue != std::nullopt and prevHashValue->hashMove != SEARCH_FAILED_MOVE_CODE) {
                winningEqualCaptures.push_back(prevHashValue->hashMove);
                break;
            }
        }
    }

    if (depth == 0)
        return search::getNegaQuiescenceEval(board, alpha, beta);
    if (*data.isCancelled)
        throw search::SearchCancelledException();

    // Null move pruning (technically null move reductions)
    if (board.canMakeNullMove()) {
        ChessBoard nmBoard = board;
        nmBoard.makeNullMove();
        if (-search::getNegamaxEval(nmBoard, max(0,depth - NMP_REDUCTION),-beta - ZERO_WINDOW_RADIUS, -beta + ZERO_WINDOW_RADIUS, data) > beta) {
            // To guard against zugzwang, we do a search to depth - 4 without a null move, and if THAT causes a beta cutoff, then we return beta.
            if (search::getNegamaxEval(board, max(0,depth - 4), beta - ZERO_WINDOW_RADIUS, beta + ZERO_WINDOW_RADIUS, data) > beta) {
                // We know we caused a beta cutoff, but we don't know what the best move is
                data.transpositionTable.put(board.getZobristCode(),depth,{beta,INFINITY,SEARCH_FAILED_MOVE_CODE});
                return beta;
            }
        }
    }

    // clear the killer moves from irrelevant positions
    if (depth > KILLER_MAX_COUSIN_LEVEL)
        data.killerMoves[depth - KILLER_MAX_COUSIN_LEVEL].clear();

    winningEqualCaptures.reserve(52);
    losingCaptures.reserve(8);
    nonCaptures.reserve(50);
    board.getLegalMoves(winningEqualCaptures, losingCaptures, nonCaptures);

    if (board.getIsItWhiteToMove())
        data.whiteHHB.sortMoves(nonCaptures);
    else
        data.blackHHB.sortMoves(nonCaptures);

    bool firstKillerMoveIsLegal = false;
    bool secondKillerMoveIsLegal = false;
    const move_t firstKillerMove = data.killerMoves[depth].getFirstKillerMove();
    const move_t secondKillerMove = data.killerMoves[depth].getSecondKillerMove();
    for (const move_t move : nonCaptures) {
        if (move == firstKillerMove) {
            firstKillerMoveIsLegal = true;
        }
        else if (move == secondKillerMove) {
            secondKillerMoveIsLegal = true;
        }
    }

    if (firstKillerMoveIsLegal)
        winningEqualCaptures.push_back(firstKillerMove);
    if (secondKillerMoveIsLegal)
        winningEqualCaptures.push_back(secondKillerMove);

    const unsigned int numMovesToNotReduce = winningEqualCaptures.size() + QUIETS_TO_NOT_REDUCE;
    unsigned int numMovesSearched = 0;

    winningEqualCaptures.insert(winningEqualCaptures.end(), nonCaptures.begin(), nonCaptures.end());
    winningEqualCaptures.insert(winningEqualCaptures.end(), losingCaptures.begin(), losingCaptures.end());

    float newscore;
    float bestscore = -INFINITY;
    move_t bestmove = SEARCH_FAILED_MOVE_CODE;
    bool improvedAlpha = false;
    for (move_t move: winningEqualCaptures) {
        ChessBoard newBoard = board;
        newBoard.makemove(move);
        numMovesSearched++;

        // Late move reductions
        if (depth >= MIN_LMR_DEPTH and numMovesSearched > numMovesToNotReduce) {
            float reducedScore = -search::getNegamaxEval(newBoard, depth - 2, -alpha - ZERO_WINDOW_RADIUS, -alpha + ZERO_WINDOW_RADIUS, data);
            if (reducedScore < alpha)
                continue;
        }
        newscore = -search::getNegamaxEval(newBoard, depth - 1, -beta, -alpha, data);

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
                        data.whiteHHB.recordKillerMove(move);
                    else
                        data.blackHHB.recordKillerMove(move);
                }
                data.transpositionTable.put(board.getZobristCode(),depth,{beta,INFINITY,move});
                return beta;
            } // end if alpha > beta
        } // end if newscore > alpha
    } // end for loop over moves
    if (improvedAlpha) { // This is a PV node, and score is exact
        data.transpositionTable.put(board.getZobristCode(),depth,{alpha,alpha,bestmove});
    }
    else { // None of the moves improved alpha. The score is an upper bound
        // Change implemented in Amethyst 43: At an All-Node, we don't add the "best move" to the transposition table because that's just noise
        data.transpositionTable.put(board.getZobristCode(),depth,{-INFINITY,alpha,SEARCH_FAILED_MOVE_CODE});
    }
    return alpha;
}

void search::getNegamaxBestMoveAndEval(const ChessBoard &board, const int depth, NegamaxData& data, const float aspirationWindowCenter,
                               move_t &bestMove, float &eval) {
    if (*data.isCancelled)
        throw SearchCancelledException();

    vector<move_t> winningEqualCaptures;
    vector<move_t> losingCaptures;
    vector<move_t> nonCaptures;
    board.getLegalMoves(winningEqualCaptures, losingCaptures, nonCaptures);

    // Internal iterative deepening
    for (int depthDecrease = 1; depthDecrease <= IID_DEPTH_DECREASE; depthDecrease++) {
        if (depth <= depthDecrease)
            break;
        std::optional<TTValue> prevHashValue = data.transpositionTable.get(board.getZobristCode(),depth - depthDecrease);
        if (prevHashValue != std::nullopt) {
            winningEqualCaptures.push_back(prevHashValue->hashMove);
            break;
        }
    }

    if (board.getIsItWhiteToMove())
        data.whiteHHB.sortMoves(nonCaptures);
    else
        data.blackHHB.sortMoves(nonCaptures);


    winningEqualCaptures.insert(winningEqualCaptures.end(), nonCaptures.begin(), nonCaptures.end());
    winningEqualCaptures.insert(winningEqualCaptures.end(), losingCaptures.begin(), losingCaptures.end());

    float startAlpha = aspirationWindowCenter - ASPIRATION_WINDOW_RADIUS;
    float beta = aspirationWindowCenter + ASPIRATION_WINDOW_RADIUS;

    float newscore;
    float alpha = startAlpha;
    move_t bestmove = SEARCH_FAILED_MOVE_CODE;
    while (alpha <= startAlpha or alpha >= beta) { // in other words, we leave this loop once we score inside the aspiration window.
        for (move_t move: winningEqualCaptures) {
            ChessBoard newBoard = board;
            newBoard.makemove(move);
            newscore = -search::getNegamaxEval(newBoard, depth - 1, -beta, -alpha, data);
            if (newscore > alpha) {
                alpha = newscore;
                bestmove = move;
                if (alpha >= beta) {
                    break;
                }
            }
        }
        if (startAlpha < alpha and alpha < beta) {
            bestMove = bestmove;
            eval = alpha;
            data.transpositionTable.put(board.getZobristCode(),depth,{alpha,alpha,bestmove});
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
    float negaEval = 0;
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
