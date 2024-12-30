#pragma once

#include <chrono>
#include <climits>
#include <array>

#include "typedefs.h"
#include "uciopt.h"
#include "repetitiontable.h"
#include "tt.h"

namespace sg {
    constexpr eval_t SCORE_MIN = -32767;
    constexpr eval_t SCORE_MAX = 32767;
    constexpr eval_t SCORE_MATE = 32700;

    inline bool isMateScore(eval_t score) {
        return score > 32000 or score < -32000;
    }

    struct SearchStackEntry {
        zobrist_t zobristCode = 0;
        eval_t staticEval = 0;
        move_t move = 0; // this is the move that lead to the position
    };

    struct ThreadData {
        perft_t nodes = 0;
        move_t rootBestMove = 0;
        std::chrono::time_point<std::chrono::high_resolution_clock> searchStartTime = std::chrono::high_resolution_clock::now();
        std::array<SearchStackEntry, 128> searchStack{};
        std::array<std::array<history_t, 4096>, 2> butterflyHistory{};
    };

    // softTimeLimit and hardTimeLimit are measured in milliseconds
    extern int softTimeLimit;
    extern int hardTimeLimit;
    extern int depthLimit;
    extern perft_t nodesLimit;

    extern std::array<RepetitionTable, 2> repetitionTables;

    extern TT GLOBAL_TT;

    int getBaseLMR(int depth, int movesSearched);
}

namespace spsa {
    inline int calcSoftTimeLimit(int time, int inc) {
        return time / 20 + inc / 2;
    }
    inline int calcHardTimeLimit(int time, int inc) {
        return time / 3;
    }
}