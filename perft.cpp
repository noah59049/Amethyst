#include "perft.h"
#include <iostream>
#include <unordered_set>

std::unordered_set<move_t> allPseudolegalMoves; // This is the set of all pseudolegal moves in all positions everywhere
constexpr bool doPseudolegalCheck = true;

perft_t perft(const ChessBoard& board, depth_t depth) {
    board.areBitboardsCorrect();

    if (depth <= 0)
        return 1;
    MoveList moves = board.getPseudoLegalMoves();

    if constexpr (doPseudolegalCheck) {
        // For every move that was pseudolegal in any position
        // If board.isPseudolegal(move), then make sure it is in the move list
        for (move_t move : allPseudolegalMoves) {
            if (board.isPseudolegal(move)) {
                bool found = false;
                for (move_t move1 : moves) {
                    if (move == move1)
                        found = true;
                }
                if (!found) {
                    std::cout << "FAILED pseudolegal check in perft" << std::endl;
                    std::cout << "FEN is " << board.toFEN() << std::endl;
                    std::cout << moveToLAN(move) << " returns true for isPseudolegal but is not in the move list" << std::endl;
                    exit(1);
                } // end if !found
            } // end if move is pseudolegal
        } // end for loop over all pseudolegal moves
    } // end if constexpr doPseudolegal check

    perft_t count = 0;
    for (move_t move : moves) {
        if (!board.isPseudolegal(move)) {
            std::cout << "FAILED isPseudolegal test: move " << moveToLAN(move) << "was in move list but failed isPseudolegal test" << std::endl;
            exit(1);
        }
        else if constexpr (doPseudolegalCheck) {
            allPseudolegalMoves.insert(move);
        }

        if (board.isLegal(move)) {
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
            ChessBoard newBoard = board;
            newBoard.makemove(move);
            count += perft(newBoard, depth_t(depth - 1));
        }
    }

    return count;
}

