#pragma once

#include <chrono>
#include <climits>
#include <array>

#include "typedefs.h"


namespace sg {
    constexpr eval_t SCORE_MIN = -32767;
    constexpr eval_t SCORE_MAX = 32767;
    constexpr eval_t SCORE_MATE = 32700;

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
    };

    // softTimeLimit and hardTimeLimit are measured in milliseconds
    extern int softTimeLimit;
    extern int hardTimeLimit;
    extern int depthLimit;
    extern perft_t nodesLimit;

    // Insert TT

}

namespace uciopt {
    constexpr int HASH_MIN = 1;
    constexpr int HASH_DEFAULT = 16;
    constexpr int HASH_MAX = 1024;
    static int HASH = HASH_DEFAULT;

    constexpr int THREADS_MIN = 1;
    constexpr int THREADS_DEFAULT = 1;
    constexpr int THREADS_MAX = 1;
    static int THREADS = 1;
}

namespace spsa {
    inline int calcSoftTimeLimit(int time, int inc) {
        return time / 20 + inc / 2;
    }
    inline int calcHardTimeLimit(int time, int inc) {
        return time / 3;
    }
}