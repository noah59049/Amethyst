#pragma once
#include <vector>
#include <optional>
#include "../Typedefs.h"
#include "../movegen/Flags.h"

struct TTValue {
    constexpr const static move_t TT_NULL_MOVE = 3120; //a4a4
    ttbound_t lowerBoundEval = 0;
    ttbound_t upperBoundEval = 0;
    move_t hashMove = TT_NULL_MOVE;
    zobrist_t zobristCode = 0;
    depth_t depth = -1;
    TTValue()=default;

    [[nodiscard]] inline bool isNull() const {
        return zobristCode == 0 and hashMove == TT_NULL_MOVE;
    }

    [[nodiscard]] inline bool isExact() const {
        return lowerBoundEval == upperBoundEval;
    }
};

class TranspositionTable {
private:
    std::vector<TTValue> buckets;

public:
    explicit TranspositionTable(int numBuckets) {
        buckets = std::vector<TTValue>(numBuckets);
    }
    void put (const TTValue& value);

    std::optional<TTValue> get(zobrist_t zobristCode, int depth);

}; // end class