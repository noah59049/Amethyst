#pragma once
#include "../Typedefs.h"
class FastLogarithm {
private:
    constexpr const static bitboard magic = 0x07EDD5E59A4E28C2ULL;
    constexpr const static int lookupTable[64] = {63, 0, 58, 1, 59, 47, 53, 2, 60, 39, 48, 27, 54, 33, 42, 3, 61, 51, 37, 40, 49, 18, 28, 20, 55, 30, 34, 11, 43, 14, 22, 4, 62, 57, 46, 52, 38, 26, 32, 41, 50, 36, 17, 19, 29, 10, 13, 21, 56, 45, 25, 31, 35, 16, 9, 12, 44, 24, 15, 8, 23, 7, 6, 5};
public:
    inline static int log2ll(const bitboard value) {
        return lookupTable[value * magic >> 58];
    }
};

inline int log2ll(const bitboard value) {
    return FastLogarithm::log2ll(value);
}