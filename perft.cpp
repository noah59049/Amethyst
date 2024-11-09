#include "perft.h"

perft_t perft(const ChessBoard& board, depth_t depth) {
    board.areBitboardsCorrect();

    if (depth <= 0)
        return 1;
    MoveList moves = board.getPseudoLegalMoves();

    perft_t count = 0;
    for (move_t move : moves) {
        ChessBoard newBoard = board;
        newBoard.makemove(move);
        bool canKingBeTaken;
        if (mvs::isCastle(move)) {
            ChessBoard slideBoard = board;
            slideBoard.makemove(mvs::castleToKingSlide(move));
            canKingBeTaken = slideBoard.canTheKingBeTaken() or newBoard.canTheKingBeTaken();
        }
        else
            canKingBeTaken = newBoard.canTheKingBeTaken();
        if (!canKingBeTaken)
             count += perft(newBoard, depth_t(depth - 1));
    }

    return count;
}

