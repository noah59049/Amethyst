#pragma once

#include <chrono>
#include "typedefs.h"


namespace sg {
    constexpr eval_t SCORE_MIN = -32767;
    constexpr eval_t SCORE_MATE = 32700;

    struct ThreadData {
        perft_t nodes = 0;
        move_t rootBestMove = 0;
        std::chrono::time_point<std::chrono::high_resolution_clock> searchStartTime = std::chrono::high_resolution_clock::now();
    };

    // softTimeLimit and hardTimeLimit are measured in milliseconds
    static int softTimeLimit = 0;
    static int hardTimeLimit = 0;

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