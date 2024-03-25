#include "MagicBitboards.h"
#include "Bitmasks.h"
#include "Logarithm.h"
#include "Flags.h"
#include <vector>
using namespace bitmasks;
using namespace std;

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

bitboard_t getRookAttackedSquares (const int square) {
    const int rank = square % 8;
    const int file = square / 8;

    return (A_FILE << (file * 8) | FIRST_RANK << rank) - (1ULL << square);
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
/////////////////////            Part 2: Magic numbers          /////////////////////
/////////////////////////////////////////////////////////////////////////////////////

// All of these are copied from
//https://github.com/maksimKorzh/chess_programming/blob/master/src/magics/magics.c

// Except for the rook magics for squares 49 through 60 and 62 through 63
// Those are N-1 magics, so the magic numbers and relevant occupancy bits have been changed.
// Those are copied from https://www.chessprogramming.org/Best_Magics_so_far

// The rook magic bitboard_t tables take 713 KB of memory.
// The bishop magic bitboard_t tables take 42 KB of memory.

// rook rellevant occupancy bits
constexpr const static int rook_rellevant_bits[64] = {
        12, 11, 11, 11, 11, 11, 11, 12,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 9, 9, 9, 9, 9, 9, 10,
        11, 10, 10, 10, 10, 11, 10, 11
};

// bishop rellevant occupancy bits
constexpr const static int bishop_rellevant_bits[64] = {
        6, 5, 5, 5, 5, 5, 5, 6,
        5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 7, 7, 7, 7, 5, 5,
        5, 5, 7, 9, 9, 7, 5, 5,
        5, 5, 7, 9, 9, 7, 5, 5,
        5, 5, 7, 7, 7, 7, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5,
        6, 5, 5, 5, 5, 5, 5, 6
};

constexpr const static int rook_shifts[64] = {
        52, 53, 53, 53, 53, 53, 53, 52,
        53, 54, 54, 54, 54, 54, 54, 53,
        53, 54, 54, 54, 54, 54, 54, 53,
        53, 54, 54, 54, 54, 54, 54, 53,
        53, 54, 54, 54, 54, 54, 54, 53,
        53, 54, 54, 54, 54, 54, 54, 53,
        53, 55, 55, 55, 55, 55, 55, 54,
        53, 54, 54, 54, 54, 53, 54, 53
};

constexpr const static int bishop_shifts[64] = {
        58, 59, 59, 59, 59, 59, 59, 58,
        59, 59, 59, 59, 59, 59, 59, 59,
        59, 59, 57, 57, 57, 57, 59, 59,
        59, 59, 57, 55, 55, 57, 59, 59,
        59, 59, 57, 55, 55, 57, 59, 59,
        59, 59, 57, 57, 57, 57, 59, 59,
        59, 59, 59, 59, 59, 59, 59, 59,
        58, 59, 59, 59, 59, 59, 59, 58
};

constexpr const bitboard_t rook_magics[64] = {
        0xa8002c000108020ULL,
        0x6c00049b0002001ULL,
        0x100200010090040ULL,
        0x2480041000800801ULL,
        0x280028004000800ULL,
        0x900410008040022ULL,
        0x280020001001080ULL,
        0x2880002041000080ULL,
        0xa000800080400034ULL,
        0x4808020004000ULL,
        0x2290802004801000ULL,
        0x411000d00100020ULL,
        0x402800800040080ULL,
        0xb000401004208ULL,
        0x2409000100040200ULL,
        0x1002100004082ULL,
        0x22878001e24000ULL,
        0x1090810021004010ULL,
        0x801030040200012ULL,
        0x500808008001000ULL,
        0xa08018014000880ULL,
        0x8000808004000200ULL,
        0x201008080010200ULL,
        0x801020000441091ULL,
        0x800080204005ULL,
        0x1040200040100048ULL,
        0x120200402082ULL,
        0xd14880480100080ULL,
        0x12040280080080ULL,
        0x100040080020080ULL,
        0x9020010080800200ULL,
        0x813241200148449ULL,
        0x491604001800080ULL,
        0x100401000402001ULL,
        0x4820010021001040ULL,
        0x400402202000812ULL,
        0x209009005000802ULL,
        0x810800601800400ULL,
        0x4301083214000150ULL,
        0x204026458e001401ULL,
        0x40204000808000ULL,
        0x8001008040010020ULL,
        0x8410820820420010ULL,
        0x1003001000090020ULL,
        0x804040008008080ULL,
        0x12000810020004ULL,
        0x1000100200040208ULL,
        0x430000a044020001ULL,
        0x280009023410300ULL,
        0x48FFFE99FECFAA00,
        0x497FFFADFF9C2E00,
        0x613FFFDDFFCE9200,
        0xffffffe9ffe7ce00,
        0xfffffff5fff3e600,
        0x0003ff95e5e6a4c0,
        0x510FFFF5F63C96A0,
        0xEBFFFFB9FF9FC526,
        0x61FFFEDDFEEDAEAE,
        0x53BFFFEDFFDEB1A2,
        0x127FFFB9FFDFB5F6,
        0x411FFFDDFFDBF4D6,
        0x1004400080a13ULL,
        0x0003ffef27eebe74,
        0x7645FFFECBFEA79E,
};

// bishop magic number
constexpr const bitboard_t bishop_magics[64] = {
        0x89a1121896040240ULL,
        0x2004844802002010ULL,
        0x2068080051921000ULL,
        0x62880a0220200808ULL,
        0x4042004000000ULL,
        0x100822020200011ULL,
        0xc00444222012000aULL,
        0x28808801216001ULL,
        0x400492088408100ULL,
        0x201c401040c0084ULL,
        0x840800910a0010ULL,
        0x82080240060ULL,
        0x2000840504006000ULL,
        0x30010c4108405004ULL,
        0x1008005410080802ULL,
        0x8144042209100900ULL,
        0x208081020014400ULL,
        0x4800201208ca00ULL,
        0xf18140408012008ULL,
        0x1004002802102001ULL,
        0x841000820080811ULL,
        0x40200200a42008ULL,
        0x800054042000ULL,
        0x88010400410c9000ULL,
        0x520040470104290ULL,
        0x1004040051500081ULL,
        0x2002081833080021ULL,
        0x400c00c010142ULL,
        0x941408200c002000ULL,
        0x658810000806011ULL,
        0x188071040440a00ULL,
        0x4800404002011c00ULL,
        0x104442040404200ULL,
        0x511080202091021ULL,
        0x4022401120400ULL,
        0x80c0040400080120ULL,
        0x8040010040820802ULL,
        0x480810700020090ULL,
        0x102008e00040242ULL,
        0x809005202050100ULL,
        0x8002024220104080ULL,
        0x431008804142000ULL,
        0x19001802081400ULL,
        0x200014208040080ULL,
        0x3308082008200100ULL,
        0x41010500040c020ULL,
        0x4012020c04210308ULL,
        0x208220a202004080ULL,
        0x111040120082000ULL,
        0x6803040141280a00ULL,
        0x2101004202410000ULL,
        0x8200000041108022ULL,
        0x21082088000ULL,
        0x2410204010040ULL,
        0x40100400809000ULL,
        0x822088220820214ULL,
        0x40808090012004ULL,
        0x910224040218c9ULL,
        0x402814422015008ULL,
        0x90014004842410ULL,
        0x1000042304105ULL,
        0x10008830412a00ULL,
        0x2520081090008908ULL,
        0x40102000a0a60140ULL,
};

constexpr const bitboard_t ROOK_RELEVANT_BLOCKERS[64] = {282578800148862,
                                                       565157600297596,
                                                       1130315200595066,
                                                       2260630401190006,
                                                       4521260802379886,
                                                       9042521604759646,
                                                       18085043209519166,
                                                       36170086419038334,
                                                       282578800180736,
                                                       565157600328704,
                                                       1130315200625152,
                                                       2260630401218048,
                                                       4521260802403840,
                                                       9042521604775424,
                                                       18085043209518592,
                                                       36170086419037696,
                                                       282578808340736,
                                                       565157608292864,
                                                       1130315208328192,
                                                       2260630408398848,
                                                       4521260808540160,
                                                       9042521608822784,
                                                       18085043209388032,
                                                       36170086418907136,
                                                       282580897300736,
                                                       565159647117824,
                                                       1130317180306432,
                                                       2260632246683648,
                                                       4521262379438080,
                                                       9042522644946944,
                                                       18085043175964672,
                                                       36170086385483776,
                                                       283115671060736,
                                                       565681586307584,
                                                       1130822006735872,
                                                       2261102847592448,
                                                       4521664529305600,
                                                       9042787892731904,
                                                       18085034619584512,
                                                       36170077829103616,
                                                       420017753620736,
                                                       699298018886144,
                                                       1260057572672512,
                                                       2381576680245248,
                                                       4624614895390720,
                                                       9110691325681664,
                                                       18082844186263552,
                                                       36167887395782656,
                                                       35466950888980736,
                                                       34905104758997504,
                                                       34344362452452352,
                                                       33222877839362048,
                                                       30979908613181440,
                                                       26493970160820224,
                                                       17522093256097792,
                                                       35607136465616896,
                                                       9079539427579068672,
                                                       8935706818303361536,
                                                       8792156787827803136,
                                                       8505056726876686336,
                                                       7930856604974452736,
                                                       6782456361169985536,
                                                       4485655873561051136,
                                                       9115426935197958144};

constexpr const bitboard_t BISHOP_RELEVANT_BLOCKERS[64] = {18049651735527936,
                                                         70506452091904,
                                                         275415828992,
                                                         1075975168,
                                                         38021120,
                                                         8657588224,
                                                         2216338399232,
                                                         567382630219776,
                                                         9024825867763712,
                                                         18049651735527424,
                                                         70506452221952,
                                                         275449643008,
                                                         9733406720,
                                                         2216342585344,
                                                         567382630203392,
                                                         1134765260406784,
                                                         4512412933816832,
                                                         9024825867633664,
                                                         18049651768822272,
                                                         70515108615168,
                                                         2491752130560,
                                                         567383701868544,
                                                         1134765256220672,
                                                         2269530512441344,
                                                         2256206450263040,
                                                         4512412900526080,
                                                         9024834391117824,
                                                         18051867805491712,
                                                         637888545440768,
                                                         1135039602493440,
                                                         2269529440784384,
                                                         4539058881568768,
                                                         1128098963916800,
                                                         2256197927833600,
                                                         4514594912477184,
                                                         9592139778506752,
                                                         19184279556981248,
                                                         2339762086609920,
                                                         4538784537380864,
                                                         9077569074761728,
                                                         562958610993152,
                                                         1125917221986304,
                                                         2814792987328512,
                                                         5629586008178688,
                                                         11259172008099840,
                                                         22518341868716544,
                                                         9007336962655232,
                                                         18014673925310464,
                                                         2216338399232,
                                                         4432676798464,
                                                         11064376819712,
                                                         22137335185408,
                                                         44272556441600,
                                                         87995357200384,
                                                         35253226045952,
                                                         70506452091904,
                                                         567382630219776,
                                                         1134765260406784,
                                                         2832480465846272,
                                                         5667157807464448,
                                                         11333774449049600,
                                                         22526811443298304,
                                                         9024825867763712,
                                                         18049651735527936};

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

int bitCount (const bitboard_t number) {
    // Could be made faster with __builtin_popcountll, but I don't want to rewrite legacy code.
    int bitCount = 0;
    for (int i = 0; i < 64; i++) {
        if (((number >> i) & 1ULL) == 1ULL) {
            bitCount++;
        }
    }
    return bitCount;
}

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

const vector<vector<bitboard_t>> ROOK_MAGIC_BITBOARD_TABLE = getRookMagicBitboardTable();
const vector<vector<bitboard_t>> BISHOP_MAGIC_BITBOARD_TABLE = getBishopMagicBitboardTable();
const vector<bitboard_t> KING_ATTACKED_SQUARES_TABLE = getKingAttackedSquaresTable();
const vector<bitboard_t> KNIGHT_ATTACKED_SQUARES_TABLE = getKnightAttackedSquaresTable();

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////           Part 4: Magic lookups           /////////////////////
/////////////////////////////////////////////////////////////////////////////////////

bitboard_t getMagicKingAttackedSquares (const int startingSquare) {
    return KING_ATTACKED_SQUARES_TABLE[startingSquare];
}

inline bitboard_t getMagicKnightAttackedSquares(const int startingSquare) {
    return KNIGHT_ATTACKED_SQUARES_TABLE[startingSquare];
}

inline bitboard_t getMagicBishopAttackedSquares (const int startingSquare, bitboard_t allPieces) {
    allPieces &= BISHOP_RELEVANT_BLOCKERS[startingSquare];
    return BISHOP_MAGIC_BITBOARD_TABLE[startingSquare][(allPieces * bishop_magics[startingSquare]) >> bishop_shifts[startingSquare]];
}

inline bitboard_t getMagicRookAttackedSquares (const int startingSquare, bitboard_t allPieces) {
    allPieces &= ROOK_RELEVANT_BLOCKERS[startingSquare];
    return ROOK_MAGIC_BITBOARD_TABLE[startingSquare][(allPieces * rook_magics[startingSquare]) >> rook_shifts[startingSquare]];
}

inline bitboard_t calcMagicQueenAttackedSquares (const int startingSquare, const bitboard_t allPieces) {
    return getMagicRookAttackedSquares(startingSquare, allPieces) | getMagicBishopAttackedSquares(startingSquare,allPieces);
}

bitboard_t getWhitePawnAttackedSquares(const int startingSquare) {
    return ((512ULL << startingSquare) & NOT_A_FILE) | (((1ULL << (startingSquare)) >> 7) & NOT_H_FILE);
}

bitboard_t getBlackPawnAttackedSquares(const int startingSquare) {
    return ((128ULL << startingSquare) & NOT_A_FILE) | (((1ULL << (startingSquare)) >> 9) & NOT_H_FILE);
}

bitboard_t getMagicWhiteAttackedSquares (const int pieceType, const int startingSquare, const bitboard_t allPieces) {
    switch(pieceType) {
        case QUEEN_CODE: return calcMagicQueenAttackedSquares(startingSquare,allPieces);
        case ROOK_CODE: return getMagicRookAttackedSquares(startingSquare,allPieces);
        case BISHOP_CODE: return getMagicBishopAttackedSquares(startingSquare,allPieces);
        case KNIGHT_CODE: return getMagicKnightAttackedSquares(startingSquare);
        case PAWN_CODE: return getWhitePawnAttackedSquares(startingSquare);
        default: assert(false);
    }
}

bitboard_t getMagicBlackAttackedSquares (const int pieceType, const int startingSquare, const bitboard_t allPieces) {
    switch(pieceType) {
        case QUEEN_CODE: return calcMagicQueenAttackedSquares(startingSquare,allPieces);
        case ROOK_CODE: return getMagicRookAttackedSquares(startingSquare,allPieces);
        case BISHOP_CODE: return getMagicBishopAttackedSquares(startingSquare,allPieces);
        case KNIGHT_CODE: return getMagicKnightAttackedSquares(startingSquare);
        case PAWN_CODE: return getBlackPawnAttackedSquares(startingSquare);
        default: assert(false);
    }
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////     Part 5: Empty board magic lookups     /////////////////////
/////////////////////////////////////////////////////////////////////////////////////

constexpr const static unsigned long EMPTY_BISHOP_ATTACKED_SQUARES[64] = {9241421688590303744ULL, 36099303471056128ULL, 141012904249856ULL, 550848566272ULL, 6480472064ULL, 1108177604608ULL, 283691315142656ULL, 72624976668147712ULL, 4620710844295151618ULL, 9241421688590368773ULL, 36099303487963146ULL, 141017232965652ULL, 1659000848424ULL, 283693466779728ULL, 72624976676520096ULL, 145249953336262720ULL, 2310355422147510788ULL, 4620710844311799048ULL, 9241421692918565393ULL, 36100411639206946ULL, 424704217196612ULL, 72625527495610504ULL, 145249955479592976ULL, 290499906664153120ULL, 1155177711057110024ULL, 2310355426409252880ULL, 4620711952330133792ULL, 9241705379636978241ULL, 108724279602332802ULL, 145390965166737412ULL, 290500455356698632ULL, 580999811184992272ULL, 577588851267340304ULL, 1155178802063085600ULL, 2310639079102947392ULL, 4693335752243822976ULL, 9386671504487645697ULL, 326598935265674242ULL, 581140276476643332ULL, 1161999073681608712ULL, 288793334762704928ULL, 577868148797087808ULL, 1227793891648880768ULL, 2455587783297826816ULL, 4911175566595588352ULL, 9822351133174399489ULL, 1197958188344280066ULL, 2323857683139004420ULL, 144117404414255168ULL, 360293502378066048ULL, 720587009051099136ULL, 1441174018118909952ULL, 2882348036221108224ULL, 5764696068147249408ULL, 11529391036782871041ULL, 4611756524879479810ULL, 567382630219904ULL, 1416240237150208ULL, 2833579985862656ULL, 5667164249915392ULL, 11334324221640704ULL, 22667548931719168ULL, 45053622886727936ULL, 18049651735527937ULL};
constexpr const static unsigned long EMPTY_ROOK_ATTACKED_SQUARES[64] = {72340172838076926ULL, 144680345676153597ULL, 289360691352306939ULL, 578721382704613623ULL, 1157442765409226991ULL, 2314885530818453727ULL, 4629771061636907199ULL, 9259542123273814143ULL, 72340172838141441ULL, 144680345676217602ULL, 289360691352369924ULL, 578721382704674568ULL, 1157442765409283856ULL, 2314885530818502432ULL, 4629771061636939584ULL, 9259542123273813888ULL, 72340172854657281ULL, 144680345692602882ULL, 289360691368494084ULL, 578721382720276488ULL, 1157442765423841296ULL, 2314885530830970912ULL, 4629771061645230144ULL, 9259542123273748608ULL, 72340177082712321ULL, 144680349887234562ULL, 289360695496279044ULL, 578721386714368008ULL, 1157442769150545936ULL, 2314885534022901792ULL, 4629771063767613504ULL, 9259542123257036928ULL, 72341259464802561ULL, 144681423712944642ULL, 289361752209228804ULL, 578722409201797128ULL, 1157443723186933776ULL, 2314886351157207072ULL, 4629771607097753664ULL, 9259542118978846848ULL, 72618349279904001ULL, 144956323094725122ULL, 289632270724367364ULL, 578984165983651848ULL, 1157687956502220816ULL, 2315095537539358752ULL, 4629910699613634624ULL, 9259541023762186368ULL, 143553341945872641ULL, 215330564830528002ULL, 358885010599838724ULL, 645993902138460168ULL, 1220211685215703056ULL, 2368647251370188832ULL, 4665518383679160384ULL, 9259260648297103488ULL, 18302911464433844481ULL, 18231136449196065282ULL, 18087586418720506884ULL, 17800486357769390088ULL, 17226286235867156496ULL, 16077885992062689312ULL, 13781085504453754944ULL, 9187484529235886208ULL};
bitboard_t getEmptyBoardMagicBishopAttackedSquares (const int startingSquare) {
    return EMPTY_BISHOP_ATTACKED_SQUARES[startingSquare];
}
bitboard_t getEmptyBoardMagicRookAttackedSquares (const int startingSquare) {
    return EMPTY_ROOK_ATTACKED_SQUARES[startingSquare];
}