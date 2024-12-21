#include "tt.h"
#include "uciopt.h"

TT::TT() {
    table = std::vector<TTEntry>((uciopt::HASH << 20) / sizeof(TTEntry));
}

TTEntry TT::get(const zobrist_t zobristCode) const {
    const size_t index = this->getIndex(zobristCode);
    const TTEntry entry = table[index];
    if (entry.zobristCode == zobristCode)
        return entry;
    else
        return {};
}

void TT::put(zobrist_t zobristCode, move_t ttMove, eval_t eval, ttflag_t ttFlag, depth_t depth) {
    table[getIndex(zobristCode)] = {zobristCode, ttMove, eval, ttFlag, depth};
}