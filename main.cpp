#include "UCI.h"
#include "movegen/MoveList.h"
#include "movegen/ChessBoard.h"
#include <iostream>
int main() {
    ChessBoard board = ChessBoard::boardFromFENNotation("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -");
    MoveList moveList;
    board.getLegalMoves1(moveList);
    for (int i = 0; i < moveList.size; i++) {
        std::cout << board.moveToSAN(moveList.moveList[i]) << std::endl;
    }
//    uciLoop();
    return 0;
}
