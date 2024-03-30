#include "UCI.h"
#include "movegen/MoveList.h"
#include "movegen/ChessBoard.h"
#include <iostream>
#include <vector>
#include "search/MoveOrder.h"
#include "search/Negamax.h"
using namespace std;
const vector<vector<int>> EXPECTED_PERFT_COUNTS = {
        {}, // position 0
        {1,20,400,8902,197281,4865609,119060324}, // position 1
        {1,48,2039,97862,4085603,193690690}, // position 2
        {1,14,191,2812,43238,674624,11030083,178633661}, // position 3
        {1,6,264,9467,422333,15833292}, // position 4
        {1,44,1486,62379,2103487,89941194}, // position 5
        {1,46,2079,89890,3894594,164075551} // position 6
};
int main() {
//    ChessBoard board = ChessBoard::boardFromFENNotation("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -");
//    for (int depth = 0; depth < 6; depth++) {
//        auto start = std::chrono::high_resolution_clock::now();
//        int numLegalMoves = board.perft(depth);
//        int numExpectedLegalMoves = EXPECTED_PERFT_COUNTS.at(2).at(depth);
//        auto end = std::chrono::high_resolution_clock::now();
//        auto duration = std::chrono::duration_cast<chrono::milliseconds>(end - start);
//        long ms = duration.count();
//        if (numLegalMoves == numExpectedLegalMoves)
//            cout << numLegalMoves << " legal moves at depth " << depth << " in " << ms << " milliseconds. Test succeeded." << endl;
//        else
//            cout << numLegalMoves << " legal moves at depth " << depth << ". Test failed. Expected " << numExpectedLegalMoves << ". Elapsed time: " << ms << " milliseconds." << endl;
//    }

//    float negaEval = board.getNegaStaticEval();
//    for (int depth = 1; depth <= 6; depth++) {
//        using namespace search;
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
//
//    MoveList legalMoves;
//    board.getLegalMoves1(legalMoves);
//    auto killerMoves = TwoKillerMoves();
//    killerMoves.recordKillerMove(WHITE_SHORT_CASTLE);
//
//    sortMoves(legalMoves,board,WHITE_LONG_CASTLE,killerMoves,QuietHistory());
//    for (move_t move : legalMoves)
//        cout << board.moveToSAN(move) << endl;
    uciLoop();
    return 0;
}
