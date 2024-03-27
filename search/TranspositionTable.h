#pragma once
#include <unordered_map>
#include <vector>
#include <optional>
#include "../Typedefs.h"
struct TTValue {
    float lowerBoundEval;
    float upperBoundEval;
    move_t hashMove;
};

class TranspositionTable {
private:
    constexpr const static float LOAD_FACTOR = 0.45;
    constexpr const static int MAX_SIZE_PER_DEPTH = 1000000;

    std::vector<std::unordered_map<zobrist_t,TTValue>> tables;

    void extendToDepth(int depth);

public:
    void put (zobrist_t zobristCode, int depth, const TTValue& value);

    std::optional<TTValue> get(zobrist_t zobristCode, int depth);

}; // end class