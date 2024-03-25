#pragma once
#include <vector>
#include "../Typedefs.h"


class QuietHistory {
private:
    int cutoffCounts[4096]{};
    inline int lookupMoveCutoffCount (uint16_t move) {
        return cutoffCounts[move >> 4];
    }

    void sortMovesByCutoffs(std::vector<uint16_t> &vec, int startIndex, int endIndex);
public:
    QuietHistory();
    void recordKillerMove (uint16_t move);

    void sortMoves(std::vector<uint16_t>& quietMoves);

};
