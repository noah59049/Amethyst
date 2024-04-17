#pragma once
#include <vector>
#include <array>
#include "../movegen/MoveList.h"
#include "../Typedefs.h"


class QuietHistory {
private:
    int cutoffCounts[4096]{};

    void sortMovesByCutoffs(std::vector<move_t> &vec, int startIndex, int endIndex) const;
public:
    QuietHistory();

    void clear();

    [[nodiscard]] inline int lookupMoveCutoffCount (move_t move) const {
        return cutoffCounts[move >> 4];
    }

    void recordKillerMove (move_t move, MoveList& moveList, int weight);

    void sortMoves(std::vector<move_t>& quietMoves) const;

    void sortMovesByCutoffs(std::array<move_t,218> &vec, int startIndex, int endIndex) const;
};
