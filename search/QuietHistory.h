#pragma once
#include <vector>
#include "../Typedefs.h"


class QuietHistory {
private:
    int cutoffCounts[4096]{};
    inline int lookupMoveCutoffCount (move_t move) {
        return cutoffCounts[move >> 4];
    }

    void sortMovesByCutoffs(std::vector<move_t> &vec, int startIndex, int endIndex);
public:
    QuietHistory();
    void recordKillerMove (move_t move);

    void sortMoves(std::vector<move_t>& quietMoves);

};
