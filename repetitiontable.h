#pragma once

#include <vector>

#include "typedefs.h"

class RepetitionTable {
private:
    std::vector<zobrist_t> positions;
public:
    void clear();
    void insert(zobrist_t zobristCode);
    bool isRepeated(zobrist_t zobristCode) const;
};