#include "repetitiontable.h"

#include <ranges>

void RepetitionTable::clear() {
    positions.clear();
}

void RepetitionTable::insert(zobrist_t zobristCode) {
    positions.push_back(zobristCode);
}

bool RepetitionTable::isRepeated(zobrist_t zobristCode) const {
    for (int i = positions.size() - 1; i >= 0; i--) {
        if (positions[i] == zobristCode)
            return true;
    }
    return false;
}