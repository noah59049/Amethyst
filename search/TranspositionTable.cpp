#include "TranspositionTable.h"

void TranspositionTable::put (const TTValue& value) {
    size_t bucketIndex = value.zobristCode % buckets.size();
    if (buckets.at(bucketIndex).isNull() or // always replace null values
            buckets.at(bucketIndex).depth < value.depth or // replace lower depth with higher depth
            (buckets.at(bucketIndex).depth == value.depth and // also replace equal depth
            (!buckets.at(bucketIndex).isExact() or value.isExact()))) { // as long as it's not replacing exact with inexact
        buckets[bucketIndex].lowerBoundEval = value.lowerBoundEval;
        buckets[bucketIndex].upperBoundEval = value.upperBoundEval;
        buckets[bucketIndex].hashMove = value.hashMove;
        buckets[bucketIndex].zobristCode = value.zobristCode;
        buckets[bucketIndex].depth = value.depth;
    }
}

std::optional<TTValue> TranspositionTable::get(const zobrist_t zobristCode, const int depth) {
    size_t bucketIndex = zobristCode % buckets.size();
    if (buckets.at(bucketIndex).zobristCode == zobristCode and buckets.at(bucketIndex).depth >= depth) {
        return buckets[bucketIndex];
    }
    else
        return std::nullopt;
}


// For every zobrist code, it hashes to a bucket
// When we do the get method: check that the zobrist code is actually the zobrist code we want
// And check that the depth is >= the depth that we want
// If it is, then get the result from the TT

// If we put something in the TT
// Then we do our replacement strategy