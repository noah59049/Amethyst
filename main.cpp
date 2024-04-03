#include "UCI.h"
#include "search/Negamax.h"
#include "search/RepetitionTable.h"
#include "search/MoveOrder.h"
#include <iostream>
int main() {
    ChessBoard board = ChessBoard::boardFromFENNotation("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -");
    eval_t eval = board.getNegaStaticEval();
    move_t bestMoveFromPrevious = WHITE_LONG_CASTLE;
    RepetitionTable repetitionTable;
    int depth;
    bool* isSearchCancelled = new bool(false);
    //std::thread* sleepThread = new std::thread(setPointerTrueLater,fut,isSearchCancelled,ms);
    search::NegamaxData data(isSearchCancelled,repetitionTable,1);
    for (depth = 1; depth < 10; depth++) {
        try {
            u_int16_t bestMove = bestMoveFromPrevious;
            data.extendKillersToDepth(depth);
            search::getNegamaxBestMoveAndEval(board,depth,data,eval,bestMove,eval);
            if (bestMove != SEARCH_FAILED_MOVE_CODE)
                bestMoveFromPrevious = bestMove;
        }
        catch (const search::SearchCancelledException& e) {
            break;
        }
    } // end for loop over depth

    bestMoveFromPrevious = board.getMoveFromPureAlgebraicNotation("f3h3");
    std::cout << board.moveToSAN(bestMoveFromPrevious) << std::endl;

    TwoKillerMoves killerMoves;
    killerMoves.recordKillerMove(WHITE_LONG_CASTLE);
    killerMoves.recordKillerMove(WHITE_SHORT_CASTLE);
    killerMoves.recordKillerMove(WHITE_SHORT_CASTLE);
    killerMoves.recordKillerMove(WHITE_SHORT_CASTLE);
    MoveList legalMoves;
    board.getLegalMoves(legalMoves);
    sortMoves(legalMoves,board,bestMoveFromPrevious,killerMoves,data.whiteHHB);
    for (move_t move : legalMoves) {
        std::cout << board.moveToSAN(move) << " " << data.whiteHHB.lookupMoveCutoffCount(move) << std::endl;
    }
//    uciLoop();
    return 0;
}
