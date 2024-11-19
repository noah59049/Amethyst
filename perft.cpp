#include "perft.h"
#include <iostream>

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
        if (!canKingBeTaken) {
            // Here we do checks that the move is the same when we convert it to LAN and back
            std::string move1 = moveToLAN(move);
            move_t move2 = board.parseLANMove(move1);
            std::string move3 = moveToLAN(move2);
            if (move != move2) {
                std::cout << "FAILED perft LAN move test: move != move2" << std::endl;
                std::cout << "FEN is " << board.toFEN() << std::endl;
                std::cout << "move is " << move << " and move2 is " << move2 << std::endl;
                std::cout << std::endl;
            }
            else if (move1 != move3) {
                std::cout << "FAILED perft LAN move test: move1 != move3" << std::endl;
                std::cout << "FEN is " << board.toFEN() << std::endl;
                std::cout << "move1 is " << move1 << " and move3 is " << move3 << std::endl;
                std::cout << std::endl;
            }

            count += perft(newBoard, depth_t(depth - 1));
        }
    }

    return count;
}

