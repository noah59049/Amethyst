#include "searchglobals.h"

namespace sg {
    int softTimeLimit = 0;
    int hardTimeLimit = 0;
    int depthLimit = 100;
    perft_t nodesLimit = INT64_MAX;

    std::array<RepetitionTable, 2> repetitionTables{};

    TT GLOBAL_TT;
}

std::array<std::array<int, 64>, 16> initLMRTable() {
    std::array<std::array<int, 64>, 16> table = {};
    for (int depth = 1; depth < 16; depth++) {
        for (int movesSearched = 1; movesSearched < 64; movesSearched++) {
            int R = 0.75 + std::log(depth) * std::log(movesSearched) / 2.3;
            R = std::min(R, depth - 1);
            R = std::max(R, 1);
            table[depth][movesSearched] = R;
        } // end for loop over movesSearched
    } // end for loop over depth
    return table;
} // end initLMRTable function

const static std::array<std::array<int, 64>, 16> LMR_TABLE = initLMRTable();


int sg::getBaseLMR(int depth, int movesSearched) {
    return LMR_TABLE[std::min(depth,15)][std::min(movesSearched,63)];
}