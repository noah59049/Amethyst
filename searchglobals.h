#pragma once

#include "typedefs.h"

namespace sg {
    struct ThreadData {
        move_t rootBestMove;
    };

    static int timeLeft;
    static int increment;

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