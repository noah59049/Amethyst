#include "UCI.h"
#include "search/Negamax.h"
#include <iostream>
#include "search/MoveOrder.h"
int main() {
    ChessBoard board = ChessBoard::boardFromFENNotation("r1bqk2r/pppp1Npp/2n2n2/4p3/2B1P3/8/PPPP1KPP/RNBQ3R b kq - 0 6");
    eval_t negaEval = 0;
    bool* isCancelled = new bool(false);
    RepetitionTable repetitionTable;
    search::NegamaxData data(isCancelled,repetitionTable,10);
    for (int depth = 1; depth <= 7; depth++) {
        auto start = std::chrono::high_resolution_clock::now();

        move_t bestMove = SEARCH_FAILED_MOVE_CODE;
        getNegamaxBestMoveAndEval(board,depth,data,negaEval,bestMove,negaEval);
        std::string readableBestMove = board.moveToSAN(bestMove);

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        long ms = duration.count();
        std::cout << "Nega eval is " << negaEval << " and best move is " << readableBestMove << " at depth " << depth << " in " << ms << " milliseconds." << std::endl;
    } // end for loop
    delete isCancelled;

    MoveList legalMoves;
    board.getLegalMoves(legalMoves);
    sortMoves(legalMoves,board,SEARCH_FAILED_MOVE_CODE,TwoKillerMoves(),data.blackQuietHistory,*data.conthist,data.conthistPrevStack);

    for (move_t move : legalMoves) {
        std::cout << board.moveToSAN(move) << std::endl;
    }
    uciLoop();
    return 0;
}
