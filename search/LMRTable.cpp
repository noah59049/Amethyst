#include "LMRTable.h"

#include <cmath>
#include <array>

inline float calculateReduction(unsigned int depth, unsigned int movesSearched) {
    // Goals: Do not reduce AT ALL if movesSearched is 0
    // At depth 3, 4, or 5, start reducing after like 8 moves
    // Double reduce after
    // At depth 8, reduce after 4 moves

    float f = -4.00F + 1.44F * logf(float(depth))  + 1.44F * logf(float(movesSearched));
    return f < 0 ? 0 : f;
}

std::array<std::array<float,64>,32> getLMRTable() {
    std::array<std::array<float,64>,32> lmrTable{};
    for (unsigned int depth = 0; depth < 32; depth++) {
        for (unsigned int movesSearched = 0; movesSearched < 64; movesSearched++) {
            lmrTable[depth][movesSearched] = calculateReduction(depth,movesSearched);
            if (movesSearched == 0)
                lmrTable[depth][movesSearched] = 0;
        }
    }
    return lmrTable;
}

const static std::array<std::array<float,64>,32> LMR_TABLE = getLMRTable();

float lookupReduction(unsigned int depth, unsigned int movesSearched) {
    if (depth >= 32)
        depth = 31;
    if (movesSearched >= 64)
        movesSearched = 63;
    return LMR_TABLE[depth][movesSearched];
}