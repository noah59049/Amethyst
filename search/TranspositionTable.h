#pragma once
#include <vector>
#include <optional>
#include "../Typedefs.h"
#include "../movegen/Flags.h"

struct TTValue {
    constexpr const static move_t TT_NULL_MOVE = 3120; //a4a4
    const eval_t lowerBoundEval = NAN;
    const eval_t upperBoundEval = NAN;
    const move_t hashMove = TT_NULL_MOVE;
    const zobrist_t zobristCode = 0;
    const int depth = -1;
    TTValue()=default;

    TTValue& operator = (TTValue other) {
        return *this;
    }

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
    void put (TTValue value);

    std::optional<TTValue> get(zobrist_t zobristCode, int depth);

}; // end class