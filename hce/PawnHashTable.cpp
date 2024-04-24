#include "PawnHashTable.h"

constexpr uint64_t magic1 = 0b0100001011100010000010110110110100010001110000010010111101100101ULL;
constexpr uint64_t magic2 = 0b1010011111011101001110011000100101111001000101000110101101011000ULL;

uint64_t getPawnHashIndex (bitboard_t whitePawns, bitboard_t blackPawns) {
    return (whitePawns * magic1 ^ blackPawns * magic2) >> 47;
}

PawnHashTable::PawnHashTable() {
    this->entries = std::vector<PawnHashValue>(131072);
}

void PawnHashTable::put(bitboard_t whitePawns, bitboard_t blackPawns, packed_eval_t packedEval) {
    this->entries[getPawnHashIndex(whitePawns,blackPawns)] =
            {whitePawns,blackPawns,packedEval,true};
}

std::pair<bitboard_t,bool> PawnHashTable::get(bitboard_t whitePawns, bitboard_t blackPawns) const {
    auto pawnHashIndex = getPawnHashIndex(whitePawns,blackPawns);
    if (this->entries[pawnHashIndex].isFilled and
                this->entries[pawnHashIndex].whitePawns == whitePawns and
                this->entries[pawnHashIndex].blackPawns == blackPawns)
        return {this->entries[pawnHashIndex].packedEval,this->entries[pawnHashIndex].isFilled};
    else
        return {0,false};
}