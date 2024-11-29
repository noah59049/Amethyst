#include "searchglobals.h"

namespace sg {
    int softTimeLimit = 0;
    int hardTimeLimit = 0;
    int depthLimit = 100;
    perft_t nodesLimit = INT64_MAX;

    std::array<RepetitionTable, 2> repetitionTables{};
}