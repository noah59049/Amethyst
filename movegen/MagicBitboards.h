#pragma once
#include "../Typedefs.h"

bitboard getMagicKingAttackedSquares (int startingSquare);
bitboard getMagicKnightAttackedSquares(int startingSquare);

bitboard getMagicBishopAttackedSquares (int startingSquare, bitboard allPieces);
bitboard getMagicRookAttackedSquares (int startingSquare, bitboard allPieces);

inline bitboard getMagicQueenAttackedSquares (const int startingSquare, const bitboard allPieces) {
    return getMagicRookAttackedSquares(startingSquare, allPieces) | getMagicBishopAttackedSquares(startingSquare,allPieces);
}

bitboard getWhitePawnAttackedSquares(int startingSquare);
bitboard getBlackPawnAttackedSquares(int startingSquare);

bitboard getMagicWhiteAttackedSquares (int pieceType, int startingSquare, bitboard allPieces);
bitboard getMagicBlackAttackedSquares (int pieceType, int startingSquare, bitboard allPieces);

bitboard getEmptyBoardMagicBishopAttackedSquares (int startingSquare);
bitboard getEmptyBoardMagicRookAttackedSquares (int startingSquare);