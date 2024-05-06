#pragma once
#include "../Typedefs.h"

bitboard_t getMagicKingAttackedSquares (int startingSquare);
bitboard_t getMagicKnightAttackedSquares(int startingSquare);

bitboard_t getMagicBishopAttackedSquares (int startingSquare, bitboard_t allPieces);
bitboard_t getMagicRookAttackedSquares (int startingSquare, bitboard_t allPieces);

//inline bitboard_t getMagicQueenAttackedSquares (const int startingSquare, const bitboard_t allPieces) {
//    return getMagicRookAttackedSquares(startingSquare, allPieces) | getMagicBishopAttackedSquares(startingSquare,allPieces);
//}

bitboard_t getWhitePawnAttackedSquares(int startingSquare);
bitboard_t getBlackPawnAttackedSquares(int startingSquare);

bitboard_t getMagicWhiteAttackedSquares (int pieceType, int startingSquare, bitboard_t allPieces);
bitboard_t getMagicBlackAttackedSquares (int pieceType, int startingSquare, bitboard_t allPieces);

bitboard_t getEmptyBoardMagicBishopAttackedSquares (int startingSquare);
bitboard_t getEmptyBoardMagicRookAttackedSquares (int startingSquare);