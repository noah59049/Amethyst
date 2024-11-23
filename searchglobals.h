#pragma once

#include "typedefs.h"

namespace sg {
    constexpr eval_t SCORE_MIN = -32767;
    constexpr eval_t SCORE_MATE = 32700;

    struct ThreadData {
        perft_t nodes;
        move_t rootBestMove;
    };

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