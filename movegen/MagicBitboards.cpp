#include "MagicBitboards.h"
using namespace bitmasks;
using std::vector;

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////   Part 1: Calculating attacked squares    /////////////////////
/////////////////////////////////////////////////////////////////////////////////////

bitboard_t getKingAttackedSquares (const int square) {
    const int rank = square % 8;
    const int file = square / 8;

    bitboard_t attackedSquares = 0ULL;

    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            if (dx == 0 and dy == 0)
                continue;
            int newRank = rank + dy;
            int newFile = file + dx;
            if (0 <= newRank and newRank <= 7 && 0 <= newFile and newFile <= 7)
                attackedSquares |= 1ULL << (8 * newFile + newRank);
        }
    }
    return attackedSquares;
}

bitboard_t getKnightAttackedSquares (const int square) {
    const bitboard_t startSquare = 1ULL << square;
    const int rank = square % 8;
    const int file = square / 8;

    bitboard_t attackedSquares = 0ULL;

    // Check forwards moves (from white's perspective)
    if (rank < 7) {
        // Check moves that are 2 sideways and 1 forwards
        if (file > 1) // we can go Nc2-a3
            attackedSquares |= startSquare >> 15;
        if (file < 6) // We can go Nf2-h3
            attackedSquares |= startSquare << 17;

        if (rank < 6) {
            // Check moves that are 2 forwards and 1 sideways
            if (file > 0)
                attackedSquares |= startSquare >> 6;
            if (file < 7)
                attackedSquares |= startSquare << 10;
        }
    }

    if (rank > 0) {
        // Check moves that are 2 sideways and 1 backwards
        if (file > 1) // We can go Nc2-a1
            attackedSquares |= startSquare >> 17;
        if (file < 6)
            attackedSquares |= startSquare << 15;

        if (rank > 1) {
            // check moves that are 2 backwards and 1 sideways
            if (file > 0) // we can go Nb3-a1
                attackedSquares |= startSquare >> 10;
            if (file < 7) // we can go Ng3-h1
                attackedSquares |= startSquare << 6;
        }
    }

    return attackedSquares;
}

bitboard_t getBishopAttackedSquares (const int square) {
    const int rank = square % 8;
    const int file = square / 8;

    bitboard_t attackedSquares = 0ULL;

    for (int dx : {-1,1}) {
        for (int dy : {-1,1}) {
            int x = file + dx;
            int y = rank + dy;
            while (0 <= x and x <= 7 && 0 <= y and y <= 7) {
                attackedSquares |= 1ULL << (8 * x + y);
                x += dx;
                y += dy;
            }
        }
    }

    return attackedSquares;
}

bitboard_t getRookPotentialBlockers (int square) {
    const int rank = square % 8;
    const int file = square / 8;

    const bitboard_t fileBlockers = A_FILE << (file * 8) & INNER_RANKS;
    const bitboard_t rankBlockers = (FIRST_RANK << rank) & INNER_FILES;

    return (fileBlockers | rankBlockers) & ~(1ULL << square);
}

bitboard_t getBishopPotentialBlockers (int square) {
    return getBishopAttackedSquares(square) & INNER_36;
}

bitboard_t getBishopLegalMoves (int square, bitboard_t blockers) {
    //assert(((blockers & getBishopPotentialBlockers(square)) == blockers));
    blockers &= getBishopPotentialBlockers(square);
    const int rank = square % 8;
    const int file = square / 8;

    bitboard_t attackedSquares = 0ULL;

    for (int dx : {-1,1}) {
        for (int dy : {-1,1}) {
            int x = file + dx;
            int y = rank + dy;
            while (0 <= x and x <= 7 && 0 <= y and y <= 7) {
                attackedSquares |= 1ULL << (8 * x + y);
                if (blockers & (1ULL << (8 * x + y)))
                    break;
                x += dx;
                y += dy;
            }
        }
    }

    return attackedSquares;
}

bitboard_t getRookLegalMoves (int square, bitboard_t blockers) {
    //assert(((blockers & getRookPotentialBlockers(square)) == blockers));
    blockers &= getRookPotentialBlockers(square);
    const int rank = square % 8;
    const int file = square / 8;

    bitboard_t attackedSquares = 0ULL;

    int DXS[4] = {0,0,-1,1};
    int DYS[4] = {1,-1,0,0};

    for (int i = 0; i < 4; i++) {
        int dx = DXS[i];
        int dy = DYS[i];
        int x = file + dx;
        int y = rank + dy;
        while (0 <= x and x <= 7 && 0 <= y and y <= 7) {
            attackedSquares |= 1ULL << (8 * x + y);
            if (blockers & (1ULL << (8 * x + y)))
                break;
            x += dx;
            y += dy;
        }
    }

    return attackedSquares;
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////       Part 3: Attack square tables        /////////////////////
/////////////////////////////////////////////////////////////////////////////////////

vector<bitboard_t> getKingAttackedSquaresTable () {
    vector<bitboard_t> kingAttackedSquaresTable(64);
    for (int square = 0; square < 64; square++) {
        kingAttackedSquaresTable[square] = getKingAttackedSquares(square);
    }
    return kingAttackedSquaresTable;
}

vector<bitboard_t> getKnightAttackedSquaresTable () {
    vector<bitboard_t> knightAttackedSquaresTable(64);
    for (int square = 0; square < 64; square++) {
        knightAttackedSquaresTable[square] = getKnightAttackedSquares(square);
    }
    return knightAttackedSquaresTable;
}

vector<bitboard_t> getAllSubBitsOf (bitboard_t original) {
    // This function has been bug tested and is correct
    // Having good variable names for this function is almost impossible because it just has so many things that mean different things.
    // Step 1: Find the 1 bits
    int bitCount = 0;
    vector<int> oneBits;
    for (int i = 0; i < 64; i++) {
        if (((original >> i) & 1ULL) == 1ULL) {
            bitCount++;
            oneBits.push_back(i);
        }
    }
    bitboard_t remainingBits = original;
    int index = 0;
    while (remainingBits != 0ULL) {
        int smallestOneBit = log2ll(remainingBits & -remainingBits);
        oneBits[index] = smallestOneBit;
        index++;
        remainingBits -= 1L << smallestOneBit;
    }

    // Step 2: Construct all possibilities
    vector<bitboard_t> possibilities(1 << bitCount);
    for (index = 0; index < possibilities.size(); index++) {
        // For every 1 bit in index, OR the thing from oneBits
        bitboard_t subBits = 0ULL;
        for (int bit = 0; bit < bitCount; bit++) {
            if (((index >> bit) & 1) == 1)
                subBits |= 1L << oneBits[bit];
        } // end inner for loop
        possibilities[index] = subBits;
    } // end outer for loop
    return possibilities;
} // end getAllSubBitsOf

vector<bitboard_t> getRookMagicBitboardTable(int startSquare, bitboard_t magicNumber, int numBits) {
    //bitboard_t numBits = bitCount(getRookPotentialBlockers(startSquare));
    vector<bitboard_t> magicBitboardTable(1 << numBits);

    for (bitboard_t blockersMask : getAllSubBitsOf(getRookPotentialBlockers(startSquare))) {
        bitboard_t result = getRookLegalMoves(startSquare,blockersMask);
        bitboard_t bucket = (blockersMask * magicNumber) >> (64 - numBits);
        assert(magicBitboardTable[bucket] == 0ULL or magicBitboardTable[bucket] == result);
        magicBitboardTable[bucket] = result;
    }

    return magicBitboardTable;
}

vector<bitboard_t> getBishopMagicBitboardTable(int startSquare, bitboard_t magicNumber, int numBits) {
    vector<bitboard_t> magicBitboardTable(1 << numBits);

    for (bitboard_t blockersMask : getAllSubBitsOf(getBishopPotentialBlockers(startSquare))) {
        bitboard_t result = getBishopLegalMoves(startSquare,blockersMask);
        bitboard_t bucket = (blockersMask * magicNumber) >> (64 - numBits);
        assert(magicBitboardTable[bucket] == 0ULL or magicBitboardTable[bucket] == result);
        magicBitboardTable[bucket] = result;
    }

    return magicBitboardTable;
}

// A 2D vector is not the fastest way to do this, but I will optimize it later.
vector<vector<bitboard_t>> getRookMagicBitboardTable() {
    vector<vector<bitboard_t>> table(64);
    for (int square = 0; square < 64; square++) {
        int bits = rook_rellevant_bits[square];
        bitboard_t magic = rook_magics[square];
        table[square] = getRookMagicBitboardTable(square,magic,bits);
    }
    return table;
}

vector<vector<bitboard_t>> getBishopMagicBitboardTable() {
    vector<vector<bitboard_t>> table(64);
    for (int square = 0; square < 64; square++) {
        int bits = bishop_rellevant_bits[square];
        bitboard_t magic = bishop_magics[square];
        table[square] = getBishopMagicBitboardTable(square,magic,bits);
    }
    return table;
}