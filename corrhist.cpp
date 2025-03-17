#include "flags.h"
#include "corrhist.h"

// Corrections are stored as an exponentially weighted moving average
// The value stored in the table is 256 times the average diff for that pawn hash
// This is because otherwise everything would just round off to 0 all the time

PawnCorrhist::PawnCorrhist() {
    table = std::vector<int32_t>(1 << 20);
}

void PawnCorrhist::clear() {
    table = std::vector<int32_t>(1 << 20);
}

void PawnCorrhist::put(const zobrist_t pawnKey, const eval_t score, const eval_t staticEval, const side_t stm) {
    int32_t diff = score - staticEval;
    if (stm != sides::WHITE)
        diff = -diff;
    const auto index = getIndex(pawnKey);
    table[index] = table[index] * 255 / 256 + diff;
}

eval_t PawnCorrhist::getCorrectedEval(const zobrist_t pawnKey, const eval_t staticEval, const side_t stm) {
    const auto index = getIndex(pawnKey);
    if (stm == sides::WHITE)
        return staticEval + table[index] / 256;
    else
        return staticEval - table[index] / 256;
}