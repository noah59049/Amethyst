cmake_minimum_required(VERSION 3.26)
project(amethyst_chess3)

set(CMAKE_CXX_STANDARD 20)

add_executable(amethyst_chess3 main.cpp
        attacks.cpp
        chessboard.cpp
        movegenerator.cpp
        perft.cpp
        uci.cpp
        search.cpp
        searchglobals.cpp
        bench.cpp
        moveorder.cpp
        moveorder.h
        repetitiontable.cpp
        repetitiontable.h
        tt.cpp
        tt.h
        uciopt.cpp
        uciopt.h
        hce.cpp
)

add_executable(amethyst_chess3_test tests.cpp
        attacks.cpp
        chessboard.cpp
        movegenerator.cpp
        perft.cpp
        uci.cpp
        search.cpp
        searchglobals.cpp
        bench.cpp
        moveorder.cpp
        moveorder.h
        repetitiontable.cpp
        repetitiontable.h
        uciopt.cpp
        uciopt.h
        tt.h
        tt.cpp
        hce.cpp
)