#pragma once
#include <vector>
#include <array>
#include "../Typedefs.h"


class QuietHistory {
private:
    int cutoffCounts[4096]{};
    [[nodiscard]] inline int lookupMoveCutoffCount (const move_t move) const {
        return cutoffCounts[move >> 4];
    }

    void sortMovesByCutoffs(std::vector<move_t> &vec, unsigned int startIndex, unsigned int endIndex) const;
public:
    QuietHistory();
    void recordKillerMove (move_t move);

    void sortMoves(std::vector<move_t>& quietMoves) const;
    void sortMovesByCutoffs(std::array<move_t,218>& array, unsigned int startIndex, unsigned int endIndex) const;

};
