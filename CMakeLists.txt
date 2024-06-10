cmake_minimum_required(VERSION 3.26)
project(amethyst_chess2)

set(CMAKE_CXX_STANDARD 17)

add_executable(amethyst_chess2 main.cpp
        movegen/Bitmasks.h
        Typedefs.h
        movegen/MagicBitboards.cpp
        movegen/MagicBitboards.h
        movegen/Logarithm.h
        movegen/Flags.h
        search/Zobrist.h
        movegen/ChessBoard.cpp
        movegen/ChessBoard.h
        search/SEE.cpp
        search/SEE.h
        hce/Eval.h
        search/TranspositionTable.cpp
        search/TranspositionTable.h
        search/TwoKillerMoves.cpp
        search/TwoKillerMoves.h
        search/QuietHistory.cpp
        search/QuietHistory.h
        search/RepetitionTable.h
        search/Negamax.cpp
        search/Negamax.h
        UCI.cpp
        UCI.h
        ChessGame.h
        movegen/MoveList.h
        search/MoveOrder.cpp
        search/MoveOrder.h
        search/Conthist.cpp
        search/Conthist.h
        hce/PawnHashTable.cpp
        hce/PawnHashTable.h
        search/DeltaPruning.h
        search/Bench.cpp
        search/Bench.h
)
