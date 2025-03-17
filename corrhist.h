#pragma once

#include "typedefs.h"
#include <vector>

class PawnCorrhist {
private:
    std::vector<eval_t> table;

    [[nodiscard]] inline size_t getIndex(zobrist_t pawnKey) const {
        using u128 = unsigned __int128;
        return (u128(pawnKey) * u128(table.size())) >> 64;
    }

public:
    PawnCorrhist();

    void clear();

    void put(zobrist_t pawnKey, eval_t score, eval_t staticEval, side_t stm);

    eval_t getCorrectedEval(zobrist_t pawnKey, eval_t staticEval, side_t stm);
};