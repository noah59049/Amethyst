#include "flags.h"
#include "corrhist.h"

PawnCorrhist::PawnCorrhist() {
    table = std::vector<eval_t>(1 << 20);
}

void PawnCorrhist::clear() {
    table = std::vector<eval_t>(1 << 20);
}

void PawnCorrhist::put(const zobrist_t pawnKey, const eval_t score, const eval_t staticEval, const side_t stm) {
    const int32_t diff = (stm == sides::WHITE) ? score - staticEval : staticEval - score;
    const auto index = getIndex(pawnKey);
    table[index] = (int32_t(table[index]) * 255 + diff) / 256;
}

eval_t PawnCorrhist::getCorrectedEval(const zobrist_t pawnKey, const eval_t staticEval, const side_t stm) {
    const auto index = getIndex(pawnKey);
    if (stm == sides::WHITE)
        return staticEval + table[index];
    else
        return staticEval - table[index];
}