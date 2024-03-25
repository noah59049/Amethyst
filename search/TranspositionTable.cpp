//
// Created by Noah Holbrook on 3/22/24.
//

#include "TranspositionTable.h"

void TranspositionTable::extendToDepth(int depth) {
    while (tables.size() <= depth) { // we want to extend so that it has indices 1 through depth
        std::unordered_map<zobrist_t,TTValue> newTable;
        newTable.max_load_factor(LOAD_FACTOR);
        tables.push_back(newTable);
    }
} // end extendToDepth

void TranspositionTable::put (const zobrist_t zobristCode, const int depth, const TTValue& value) {
    extendToDepth(depth);
    if (tables[depth].size() < MAX_SIZE_PER_DEPTH)
        tables[depth][zobristCode] = value;
}

std::optional<TTValue> TranspositionTable::get(const zobrist_t zobristCode, const int depth) {
    extendToDepth(depth);
    auto myIterator = tables[depth].find(zobristCode); // actual type is std::unordered_map<zobrist_t, TTValue>::iterator
    if (myIterator == tables[depth].end())
        return std::nullopt;
    else
        return {myIterator->second};
} // end get method