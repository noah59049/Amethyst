#pragma once
#include "../Typedefs.h"
#include <vector>

struct PawnHashValue {
    bitboard_t whitePawns;
    bitboard_t blackPawns;
    packed_eval_t packedEval;
    bool isFilled = false;
};

class PawnHashTable {
private:
    std::vector<PawnHashValue> entries;
public:
    PawnHashTable();
    void put (bitboard_t whitePawns, bitboard_t blackPawns, packed_eval_t packedEval);
    [[nodiscard]] std::pair<packed_eval_t,bool> get (bitboard_t whitePawns, bitboard_t blackPawns) const;
};

static PawnHashTable GLOBAL_PAWN_HASH_TABLE = PawnHashTable();