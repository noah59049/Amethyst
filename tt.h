#pragma once
#include "typedefs.h"
#include "searchglobals.h"

#include <vector>

namespace ttflags {
    ttflag_t EMPTY = 0;
    ttflag_t UPPER_BOUND = 1;
    ttflag_t LOWER_BOUND = 2;
    ttflag_t EXACT = 3;

}

struct TTEntry {
    zobrist_t zobristCode = 0;
    move_t ttMove = 0;
    eval_t eval = 0;
    ttflag_t ttFlag = 0;
    depth_t depth = 0;

    [[nodiscard]] inline bool isNotNull() const {
        return ttFlag != ttflags::EMPTY;
    }
};

class TT {
private:
    std::vector<TTEntry> table;

    [[nodiscard]] inline size_t getIndex(zobrist_t zobristCode) const {
        using u128 = unsigned __int128;
        return (u128(zobristCode) * u128(table.size())) >> 64;
    }

public:
    TT();

    [[nodiscard]] TTEntry get(zobrist_t zobristCode) const;

    void put(zobrist_t zobristCode, move_t ttMove, eval_t eval, ttflag_t ttFlag, depth_t depth);
};