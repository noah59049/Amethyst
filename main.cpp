#include "UCI.h"
#include "search/MoveList.h"
#include "movegen/ChessBoard.h"
#include <iostream>
#include <vector>
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
    ChessBoard board = ChessBoard::boardFromFENNotation("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -");
    for (int depth = 0; depth < 6; depth++) {
        auto start = std::chrono::high_resolution_clock::now();
        int numLegalMoves = board.perft(depth);
        int numExpectedLegalMoves = EXPECTED_PERFT_COUNTS.at(2).at(depth);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<chrono::milliseconds>(end - start);
        long ms = duration.count();
        if (numLegalMoves == numExpectedLegalMoves)
            cout << numLegalMoves << " legal moves at depth " << depth << " in " << ms << " milliseconds. Test succeeded." << endl;
        else
            cout << numLegalMoves << " legal moves at depth " << depth << ". Test failed. Expected " << numExpectedLegalMoves << ". Elapsed time: " << ms << " milliseconds." << endl;
    }
//    uciLoop();
    return 0;
}
