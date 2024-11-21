#include "search.h"

#include <iostream>

void rootSearch(ChessBoard board) {
    // In the future we will do the iterative deepening search with aspiration windows
    // For now, just return the first move

    MoveList moves = board.getPseudoLegalMoves();
    for (move_t move : moves) {
        if (board.isLegal(move)) {
            std::cout << "bestmove " << moveToLAN(move) << std::endl;
            break;
        }
    }

    // There are no legal moves
    std::cout << "bestmove 0000" << std::endl;
}

eval_t negamax(const ChessBoard& board, eval_t alpha, eval_t beta) {
    // TODO: return something from here
}