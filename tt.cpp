#include "tt.h"
#include "uciopt.h"

TT::TT() {
    table = std::vector<TTEntry>((uciopt::HASH << 20) / sizeof(TTEntry));
}

void TT::clear() {
    table = std::vector<TTEntry>((uciopt::HASH << 20) / sizeof(TTEntry));
}

TTEntry TT::get(const zobrist_t zobristCode) const {
    const size_t index = this->getIndex(zobristCode);
    const TTEntry entry = table[index];
    if (entry.isNotNull() and entry.zobristCode == zobristCode)
        return entry;
    else
        return {};
}

void TT::put(zobrist_t zobristCode, move_t ttMove, eval_t score, ttflag_t ttFlag, depth_t depth) {
    const size_t index = getIndex(zobristCode);
    ttMove &= 0x3fffff;
    if (ttMove == 0 and zobristCode == table[index].zobristCode)
        ttMove = table[index].ttMove;
    table[getIndex(zobristCode)] = {zobristCode, ttMove, score, ttFlag, depth};
}